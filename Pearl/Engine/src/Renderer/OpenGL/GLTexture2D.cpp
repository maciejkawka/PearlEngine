#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLTexture2D.h"

#include"glad/glad.h"
#define STB_IMAGE_IMPLEMENTATION //Dirty as fuck :/
#include"stb/stb_image.h"

#include"Core/Filesystem/FileSystem.h"

using namespace PrRenderer::OpenGL;

void GLTexture2D::Bind(unsigned int p_slot)
{
	glActiveTexture(GL_TEXTURE0 + p_slot);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

void GLTexture2D::Unbind()
{
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
		case PrRenderer::Resources::TextureFormat::Alpha8:
		case PrRenderer::Resources::TextureFormat::R8:
			m_size += m_width * m_height * 8;
			break;
		case PrRenderer::Resources::TextureFormat::GrayAlpha8:
		case PrRenderer::Resources::TextureFormat::RG8:
			m_size += m_width * m_height * 2 * 8;
			break;
		case PrRenderer::Resources::TextureFormat::RGB8:
			m_size += m_width * m_height * 3 * 8;
			break;
		case PrRenderer::Resources::TextureFormat::RGBA8:
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
		m_format = PrRenderer::Resources::TextureFormat::Gray8;
		break;
	case 2:
		m_format = PrRenderer::Resources::TextureFormat::GrayAlpha8;
		break;
	case 3:
		m_format = PrRenderer::Resources::TextureFormat::RGB8;
		break;
	case 4:
		m_format = PrRenderer::Resources::TextureFormat::RGBA8;
		break;
	default:
		PRLOG_WARN("Cannot specify texture {} channel format", m_name);
		break;
	}

	m_height = heigth;
	m_width = width;

	return data;
}

unsigned int PrRenderer::OpenGL::TextureFormatToGL(Resources::TextureFormat p_format)
{
	switch (p_format)
	{
	case Resources::TextureFormat::RGB8:
		return GL_RGB;
		break;
	case Resources::TextureFormat::RGBA8:
		return GL_RGBA;
		break;
	case Resources::TextureFormat::Alpha8:
		return GL_DEPTH_COMPONENT;
		break;
	case Resources::TextureFormat::GrayAlpha8:
		return GL_RG;
		break;
	default:
		return 0;
		break;
	}
}

unsigned int PrRenderer::OpenGL::TextureWrapToGL(PrRenderer::Resources::TextureWrapMode p_wrap)
{
	switch (p_wrap)
	{
	case Resources::TextureWrapMode::Clamp:
		return GL_CLAMP_TO_EDGE;
		break;
	case Resources::TextureWrapMode::Repeat:
		return GL_REPEAT;
		break;
	case Resources::TextureWrapMode::Mirror:
		return GL_MIRRORED_REPEAT;
		break;
	default:
		return GL_REPEAT; //If not recognise return default
		break;
	}
}

unsigned int PrRenderer::OpenGL::TextureFilterToGL(PrRenderer::Resources::TextureFiltering p_filter)
{
	switch (p_filter)
	{
	case Resources::TextureFiltering::Linear:
		return GL_LINEAR;
		break;
	case Resources::TextureFiltering::Nearest:
		return GL_NEAREST;
		break;
	case Resources::TextureFiltering::LinearMipMapLinear:
		return GL_LINEAR_MIPMAP_LINEAR;
		break;
	case Resources::TextureFiltering::LinearMipmapNearest:
		return GL_LINEAR_MIPMAP_NEAREST;
		break;
	case Resources::TextureFiltering::NearestMipmapLinear:
		return GL_NEAREST_MIPMAP_LINEAR;
		break;
	case Resources::TextureFiltering::NearestMipmapNearest:
		return GL_NEAREST_MIPMAP_NEAREST;
		break;
	default:
		return GL_LINEAR; //If not recognise return default
		break;
	}
}
