#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLCubemap.h"
#include"Renderer/OpenGL/GLUtils.h"

#include"glad/glad.h"
#include"stb/stb_image.h"

#include"Core/Filesystem/FileSystem.h"
#include"Core/Utils/JSONParser.h"

using namespace PrRenderer::OpenGL;

GLCubemap::GLCubemap(std::string p_name, PrCore::Resources::ResourceHandle p_handle):
	Cubemap(p_name, p_handle)
{
}

void GLCubemap::Bind(unsigned int p_slot)
{
	glActiveTexture(GL_TEXTURE0 + p_slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
}

void GLCubemap::Unbind(unsigned int p_slot)
{
	glActiveTexture(GL_TEXTURE0 + p_slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GLCubemap::SetMinFiltering(PrRenderer::Resources::TextureFiltering p_minfiltering)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(p_minfiltering));
	m_minFiltering = p_minfiltering;
}

void GLCubemap::SetMagFiltering(PrRenderer::Resources::TextureFiltering p_magfiltering)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(p_magfiltering));
	m_magFiltering = p_magfiltering;
}

void GLCubemap::SetWrapModeU(PrRenderer::Resources::TextureWrapMode p_wrapU)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, TextureWrapToGL(p_wrapU));
	m_wrapU = p_wrapU;
}

void GLCubemap::SetWrapModeV(PrRenderer::Resources::TextureWrapMode p_wrapV)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, TextureWrapToGL(p_wrapV));
	m_wrapV = p_wrapV;
}

void GLCubemap::PreLoadImpl()
{
}

bool GLCubemap::LoadImpl()
{
	if(m_readable)
		m_rawDataArray = new unsigned char* [6];

	LoadTexturesNames();

	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

	for(int i=0;i<6;i++)
	{
		unsigned char* rawImage = ReadRawData(m_facesNames[i], i == 0);
		if (rawImage == nullptr)
			return false;

		unsigned int format = TextureFormatToGL(m_format);
		unsigned int internalFormat = TextureFormatToInternalGL(m_format);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, rawImage);

		if (!m_readable)
			stbi_image_free(rawImage);
		else
			m_rawDataArray[i] = rawImage;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, TextureWrapToGL(m_wrapU));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, TextureWrapToGL(m_wrapV));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, TextureWrapToGL(m_wrapR));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(m_minFiltering));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(m_magFiltering));

	if (m_mipmap)
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return true;
}

void GLCubemap::PostLoadImpl()
{
}

void GLCubemap::PreUnloadImpl()
{
}

bool GLCubemap::UnloadImpl()
{
	if (m_readable)
		delete[] m_rawDataArray;

	glDeleteTextures(1, &m_ID);

	return true;
}

void GLCubemap::PostUnloadImpl()
{
}

void GLCubemap::LoadDefault()
{
	unsigned char* rawImage = new unsigned char[4];

	//Pink color
	rawImage[0] = 255;
	rawImage[1] = 20;
	rawImage[2] = 147;
	rawImage[3] = 255;

	unsigned int format = TextureFormatToGL(Resources::TextureFormat::RGB24);
	unsigned int internalFormat = TextureFormatToInternalGL(Resources::TextureFormat::RGB24);
	int height = 1;
	int width = 1;

	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
	for(int i=0;i<6;i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, rawImage);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, TextureWrapToGL(m_wrapU));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, TextureWrapToGL(m_wrapV));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(m_minFiltering));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(m_magFiltering));

	if (m_mipmap)
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	delete[] rawImage;
	m_readable = false;

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GLCubemap::CalculateSize()
{
}

unsigned char* GLCubemap::ReadRawData(const std::string& p_name, bool p_first)
{
	/*if (m_name == "Empty")
	{
		PRLOG_WARN("Cubamap {0} the face is not specify", m_name);
		unsigned char* rawImage = new unsigned char[4];

		//Pink color
		rawImage[0] = 255;
		rawImage[1] = 20;
		rawImage[2] = 147;
		rawImage[3] = 255;

		return rawImage;
	}*/

	int width = 0;
	int heigth = 0;
	int channelsNumber = 0;

	//Change that in future
	std::string dir = ROOT_DIR;
	dir += "/";
	dir += TEXTURE_DIR;
	dir += "/" + p_name;

	stbi_set_flip_vertically_on_load(false);
	unsigned char* data = stbi_load(dir.c_str(), &width, &heigth, &channelsNumber, 0);

	if (!data)
		return nullptr;

	Resources::TextureFormat format = Resources::TextureFormat::None;
	switch (channelsNumber)
	{
	case 1:
		format = PrRenderer::Resources::TextureFormat::R8;
		break;
	case 2:
		format = PrRenderer::Resources::TextureFormat::RG16;
		break;
	case 3:
		format = PrRenderer::Resources::TextureFormat::RGB24;
		break;
	case 4:
		format = PrRenderer::Resources::TextureFormat::RGBA32;
		break;
	default:
		PRLOG_WARN("Cannot specify texture {} channel format", m_name);
		break;
	}

	if (p_first)
	{
		m_format = format;
		m_height = heigth;
		m_width = width;
	}
	else
	{
		if (m_format != format)
			PRLOG_WARN("Cubamap {0} face {1} has different format than first face. Using default first face format",
				m_name, p_name);

		if (m_height != heigth)
			PRLOG_WARN("Cubamap {0} face {1} has different height than first face. Using default first face height",
				m_name, p_name);

		if (m_width != width)
			PRLOG_WARN("Cubamap {0} face {1} has different width than first face. Using default first face width",
				m_name, p_name);
	}

	return data;
}

bool GLCubemap::LoadTexturesNames()
{
	std::string dir = TEXTURE_DIR;
	dir += ("/" + m_name);
	PrCore::Filesystem::FileStreamPtr file = PrCore::Filesystem::FileSystem::GetInstance().OpenFileStream(dir.c_str());
	if (file == nullptr)
		return false;

	char* data = new char[file->GetSize()];
	file->Read(data);

	std::vector<uint8_t> dataVector;

	for (auto i = 0; i < file->GetSize(); i++)
		dataVector.push_back(*(data + i));
	delete[] data;

	auto json = PrCore::Utils::JSON::json::parse(dataVector);


	m_facesNames.push_back(json["right"]);
	m_facesNames.push_back(json["left"]);
	m_facesNames.push_back(json["top"]);
	m_facesNames.push_back(json["bottom"]);
	m_facesNames.push_back(json["front"]);
	m_facesNames.push_back(json["back"]);
	

}
