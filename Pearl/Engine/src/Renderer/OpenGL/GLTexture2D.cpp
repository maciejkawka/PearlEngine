#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLTexture2D.h"
#include"Renderer/OpenGL/GLUtils.h"

#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

GLTexture2D::GLTexture2D()
{
	glGenTextures(1, &m_ID);
}

GLTexture2D::GLTexture2D(RendererID p_id, size_t p_width, size_t p_height, Resources::TextureFormat p_format)
{
	m_ID = p_id;
	m_width = p_width;
	m_height = p_height;
	m_format = p_format;
}

GLTexture2D::~GLTexture2D()
{
	if (m_ID != 0)
		glDeleteTextures(1, &m_ID);
}

void GLTexture2D::Bind(unsigned int p_slot /*= 0*/)
{
	glActiveTexture(GL_TEXTURE0 + p_slot);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

void GLTexture2D::Unbind(unsigned int p_slot /*= 0*/)
{
	glActiveTexture(GL_TEXTURE0 + p_slot);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture2D::GenerateMipMaps()
{
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture2D::Apply()
{
	PR_ASSERT(m_ID != 0, "Texture ID is not generated");

	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrapToGL(m_wrapU));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrapToGL(m_wrapV));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(m_minFiltering));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(m_magFiltering));

	if (m_rawData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, TextureFormatToInternalGL(m_format), m_width, m_height, 0, TextureFormatToGL(m_format), TextureFormatToDataTypeGL(m_format), m_rawData);
		CalculateSize();
	}

	if (m_mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture2D::CalculateSize()
{
	PR_ASSERT(m_rawData != nullptr, "Texture data is not loaded");

	m_size = sizeof(GLTexture2D);

	switch (m_format)
	{
	case Resources::TextureFormat::R8:
		m_size += m_width * m_height * 8;
		break;
	case Resources::TextureFormat::RG16:
		m_size += m_width * m_height * 2 * 8;
		break;
	case Resources::TextureFormat::RGB24:
		m_size += m_width * m_height * 3 * 8;
		break;
	case Resources::TextureFormat::RGBA32:
		m_size += m_width * m_height * 4 * 8;
		break;
	default:
		break;
	}
}

size_t GLTexture2D::GetByteSize() const
{
	return m_size;
}
