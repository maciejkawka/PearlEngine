#include "Core/Common/pearl_pch.h"

#include "Renderer/OpenGL/GLCubemapv2.h"
#include"Renderer/OpenGL/GLUtils.h"

#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

GLCubemapv2::GLCubemapv2()
{
	glGenTextures(1, &m_ID);
}

GLCubemapv2::GLCubemapv2(RendererID p_id, size_t p_width, size_t p_height, Resources::TextureFormat p_format)
{
	m_ID = p_id;
	m_width = p_width;
	m_height = p_height;
	m_format = p_format;
}

GLCubemapv2::~GLCubemapv2()
{
	if (m_ID != 0)
		glDeleteTextures(1, &m_ID);
}

void GLCubemapv2::Unbind(unsigned int p_slot /*= 0*/)
{
	glActiveTexture(GL_TEXTURE0 + p_slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GLCubemapv2::Bind(unsigned int p_slot /*= 0*/)
{
	glActiveTexture(GL_TEXTURE0 + p_slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
}

size_t GLCubemapv2::GetByteSize() const
{
	return m_size;
}

void GLCubemapv2::CalculateSize()
{
	m_size = sizeof(GLCubemapv2);

	switch (m_format)
	{
	case Resources::TextureFormat::R8:
		m_size += m_width * m_height * 8 * 6;
		break;
	case Resources::TextureFormat::RG16:
		m_size += m_width * m_height * 2 * 8 * 6;
		break;
	case Resources::TextureFormat::RGB24:
		m_size += m_width * m_height * 3 * 8 * 6;
		break;
	case Resources::TextureFormat::RGBA32:
		m_size += m_width * m_height * 4 * 8 * 6;
		break;
	default:
		break;
	}
}

void GLCubemapv2::Apply()
{
	PR_ASSERT(m_ID != 0, "Texture ID is not generated");

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, TextureWrapToGL(m_wrapU));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, TextureWrapToGL(m_wrapV));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, TextureWrapToGL(m_wrapR));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(m_minFiltering));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(m_magFiltering));

	if (m_rawDataArray)
	{
		for (int i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, TextureFormatToInternalGL(m_format), m_width, m_height, 0,
				TextureFormatToGL(m_format), TextureFormatToDataTypeGL(m_format), m_rawDataArray[i]);
		CalculateSize();
	}

	if (m_mipmap)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GLCubemapv2::GenerateMipMaps()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}