#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLTexture2D.h"
#include"Renderer/OpenGL/GLUtils.h"

#include"glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION //Dirty as fuck :/
#include"stb/stb_image.h"

#include"Core/Filesystem/FileSystem.h"

using namespace PrRenderer::OpenGL;

GLTexture2D::GLTexture2D(RendererID p_rendererID, size_t p_width, size_t p_height, PrRenderer::Resources::TextureFormat p_format)
{
	m_ID = p_rendererID;
	m_width = p_width;
	m_height = p_height;

	m_format = p_format;
}

void GLTexture2D::Bind(unsigned int p_slot)
{
	glActiveTexture(GL_TEXTURE0 + p_slot);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

void GLTexture2D::Unbind(unsigned int p_slot)
{
	glActiveTexture(GL_TEXTURE0 + p_slot);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture2D::IsMipMapped(bool p_mipmap)
{
}

void GLTexture2D::IsReadable(bool p_readable)
{
}

void GLTexture2D::SetMinFiltering(PrRenderer::Resources::TextureFiltering p_minfiltering)
{
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(p_minfiltering));
	m_minFiltering = p_minfiltering;
}

void GLTexture2D::SetMagFiltering(PrRenderer::Resources::TextureFiltering p_magfiltering)
{
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(p_magfiltering));
	m_magFiltering = p_magfiltering;
}

void GLTexture2D::SetWrapModeU(PrRenderer::Resources::TextureWrapMode p_wrapU)
{
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrapToGL(p_wrapU));
	m_wrapU = p_wrapU;
}

void GLTexture2D::SetWrapModeV(PrRenderer::Resources::TextureWrapMode p_wrapV)
{
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrapToGL(p_wrapV));
	m_wrapV = p_wrapV;
}

void GLTexture2D::PreLoadImpl()
{
}

bool GLTexture2D::LoadImpl()
{
	unsigned char* rawImage = ReadRawData();
	if (!rawImage)
		return false;

	unsigned int format = TextureFormatToGL(m_format);
	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, rawImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrapToGL(m_wrapU));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrapToGL(m_wrapV));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(m_minFiltering));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(m_magFiltering));

	if(m_mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);
	
	if (!m_readable)
		stbi_image_free(rawImage);
	else
		m_rawData = rawImage;
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

void GLTexture2D::PostLoadImpl()
{
}

void GLTexture2D::PreUnloadImpl()
{
}

bool PrRenderer::OpenGL::GLTexture2D::UnloadImpl()
{
	if(m_readable)
		stbi_image_free(m_rawData);

	glDeleteTextures(1, &m_ID);

	return true;
}

void GLTexture2D::PostUnloadImpl()
{
}

void GLTexture2D::LoadDefault()
{
	unsigned char* rawImage = new unsigned char[4];

	//Pink color
	rawImage[0] = 255;
	rawImage[1] = 20;
	rawImage[2] = 147;
	rawImage[3] = 255;

	unsigned int format = TextureFormatToGL(Resources::TextureFormat::RGB24);
	int height = 1;
	int width = 1;

	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, rawImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrapToGL(m_wrapU));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrapToGL(m_wrapV));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(m_minFiltering));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(m_magFiltering));

	if (m_mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	delete[] rawImage;
	m_readable = false;

	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture2D::CalculateSize()
{
	m_size = sizeof(m_ID) +
			sizeof(m_height) +
			sizeof(m_width) +
			sizeof(m_readable) +
			sizeof(m_mipmap) +
			sizeof(m_format) +
			sizeof(m_minFiltering) +
			sizeof(m_magFiltering) +
			sizeof(m_wrapU) +
			sizeof(m_wrapV);
	
	if (m_readable)
	{
		switch (m_format)
		{
		case PrRenderer::Resources::TextureFormat::R8:
			m_size += m_width * m_height * 8;
			break;
		case PrRenderer::Resources::TextureFormat::RG16:
			m_size += m_width * m_height * 2 * 8;
			break;
		case PrRenderer::Resources::TextureFormat::RGB24:
			m_size += m_width * m_height * 3 * 8;
			break;
		case PrRenderer::Resources::TextureFormat::RGBA32:
			m_size += m_width * m_height * 4 * 8;
			break;
		default:
			break;
		}
	}
}

unsigned char* GLTexture2D::ReadRawData()
{
	int width = 0;
	int heigth = 0;
	int channelsNumber = 0;

	//Change that in future
	std::string dir = ROOT_DIR;
	dir += "/";
	dir +=TEXTURE_DIR;
	dir += "/" + m_name;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(dir.c_str(), &width, &heigth, &channelsNumber, 0);

	if (!data)
		return nullptr;

	switch (channelsNumber)
	{
	case 1:
		m_format = PrRenderer::Resources::TextureFormat::R8;
		break;
	case 2:
		m_format = PrRenderer::Resources::TextureFormat::RG16;
		break;
	case 3:
		m_format = PrRenderer::Resources::TextureFormat::RGB24;
		break;
	case 4:
		m_format = PrRenderer::Resources::TextureFormat::RGBA32;
		break;
	default:
		PRLOG_WARN("Cannot specify texture {} channel format", m_name);
		break;
	}

	m_height = heigth;
	m_width = width;

	return data;
}


