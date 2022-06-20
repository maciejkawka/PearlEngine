#include"Core/Common/pearl_pch.h"

#include"Core/Windowing/Window.h"

#include"Renderer/OpenGL/GLFramebuffer.h"
#include"Renderer/OpenGL/GLTexture2D.h"
#include"Renderer/OpenGL/GLCubemap.h"
#include"Renderer/OpenGL/GLUtils.h"

#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

GLFramebuffer::GLFramebuffer(const Buffers::FramebufferSettings& p_settings):
	Framebufffer()
{
	m_settings = p_settings;
	UpdateFamebuffer();
}

GLFramebuffer::~GLFramebuffer()
{
	glDeleteFramebuffers(1, &m_ID);

	for (int i = 0; i < m_colorTextureIDs.size(); i++)
	{
		if(m_colorTextureIDs[i] != 0)
			glDeleteTextures(1, &m_colorTextureIDs[i]);
	}
	
	glDeleteTextures(1, &m_depthTextureID);
}

void GLFramebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
	glViewport(0, 0, m_settings.width, m_settings.height);
}


void GLFramebuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	auto& window = PrCore::Windowing::Window::GetMainWindow();
	
	glViewport(0, 0, window.GetWidth(), window.GetHeight());
}

void GLFramebuffer::SetLevelofTexture(int p_attachment, int p_textureLevel)
{
	if (p_attachment > m_colorTextureAttachments.size() || p_textureLevel > 5 || p_textureLevel < 0 || m_colorTextureAttachments.empty())
	{
		PRLOG_ERROR("Framebuffer {0}: wrong SetLevelOfTexture", m_ID);
		return;
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + p_attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + p_textureLevel, m_colorTextureIDs[p_attachment], 0);
}

void GLFramebuffer::Resize(size_t width, size_t height)
{
	if (width == 0 || height == 0)
		return;

	m_settings.width = width;
	m_settings.height = height;

	UpdateFamebuffer();
}

void PrRenderer::OpenGL::GLFramebuffer::ClearAttachmentColor(unsigned int p_attachment, int p_value)
{
	if(p_attachment < m_colorTextureIDs.size());

	auto format = TextureFormatToGL(m_colorTextureAttachments[p_attachment].format);
	glClearTexImage(m_colorTextureIDs[p_attachment], 0,
		format, GL_INT, &p_value);
}

PrRenderer::Resources::TexturePtr GLFramebuffer::GetTexturePtr(unsigned int p_index)
{
	if (p_index >= m_colorTextureAttachments.size())
		return PrRenderer::Resources::TexturePtr();
	
	GenerateTexture(p_index);

	return m_colorTextures[p_index];
}

PrRenderer::RendererID GLFramebuffer::GetTextureID(unsigned int p_index)
{
	if (p_index >= m_colorTextureAttachments.size())
		return 0;

	auto id = m_colorTextureIDs[p_index];

	//ID set to zero to show that resposibility to delete is moved to outside the frambuffer
	m_colorTextureIDs[p_index] = 0;

	return id;
}

void PrRenderer::OpenGL::GLFramebuffer::UpdateFamebuffer()
{
	if (m_ID)
	{
		glDeleteFramebuffers(1, &m_ID);

		glDeleteTextures(m_colorTextureIDs.size(), m_colorTextureIDs.data());
		m_colorTextureIDs.clear();
		m_colorTextureAttachments.clear();
		m_depthStencilTextureAttachment.format = Resources::TextureFormat::None;
	}

	glCreateFramebuffers(1, &m_ID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ID);

	if(!m_settings.colorTextureAttachments.textures.empty())
		UpdateColorTextures();

	if (m_depthStencilTextureAttachment.format != Resources::TextureFormat::None)
		UpdateDepthTexture();

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		PRLOG_ERROR("Framebuffer ID: {0} is invalid", m_ID);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void GLFramebuffer::UpdateColorTextures()
{
	auto colorTexAttachments = m_settings.colorTextureAttachments.textures;
	
	for (int i = 0; i < colorTexAttachments.size(); i++)
	{
		if (colorTexAttachments[i].cubeTexture)
			CreateCubemapAttachment(i);
		else
			CreateTextureAttachment(i);
	}

	std::vector<GLenum> attachments;
	for (int i = 0; i < m_colorTextureAttachments.size(); i++)
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	glDrawBuffers(attachments.size(), attachments.data());

	m_colorTextures.resize(m_colorTextureAttachments.size());
}

void GLFramebuffer::CreateCubemapAttachment(int p_attachmentIndex)
{
	auto colorTexAttachments = m_settings.colorTextureAttachments.textures;
	const auto& attachment = colorTexAttachments[p_attachmentIndex];

	//Create cube texture
	unsigned int textureID;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	m_colorTextureIDs.push_back(textureID);

	if (m_settings.width != m_settings.height)
		m_settings.height = m_settings.width;

	for (int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, TextureFormatToInternalGL(attachment.format), m_settings.width, m_settings.height,
			0, TextureFormatToGL(attachment.format), TextureFormatToDataTypeGL(attachment.format), NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, TextureWrapToGL(attachment.wrapModeU));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, TextureWrapToGL(attachment.wrapModeV));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, TextureWrapToGL(attachment.wrapModeR));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(attachment.filteringMin));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(attachment.filteringMag));

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 1, GL_TEXTURE_CUBE_MAP_POSITIVE_X, textureID, 0);

	m_colorTextureAttachments.push_back(attachment);
}

void GLFramebuffer::CreateTextureAttachment(int p_attachmentIndex)
{
	auto colorTexAttachments = m_settings.colorTextureAttachments.textures;
	const auto& attachment = colorTexAttachments[p_attachmentIndex];

	//Create 2D texture
	unsigned int textureID;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	m_colorTextureIDs.push_back(textureID);

	
	glTexImage2D(GL_TEXTURE_2D, 0, TextureFormatToInternalGL(attachment.format), m_settings.width, m_settings.height,
		0, TextureFormatToGL(attachment.format), TextureFormatToDataTypeGL(attachment.format), NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrapToGL(attachment.wrapModeU));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrapToGL(attachment.wrapModeV));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(attachment.filteringMin));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(attachment.filteringMag));

	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + p_attachmentIndex, GL_TEXTURE_2D, textureID, 0);

	m_colorTextureAttachments.push_back(attachment);
}


void GLFramebuffer::UpdateDepthTexture()
{
	auto attachment = m_settings.depthStencilAttachment;

	glCreateTextures(GL_TEXTURE_2D, 1, &m_depthTextureID);
	glBindTexture(GL_TEXTURE_2D, m_depthTextureID);
	glTexStorage2D(GL_TEXTURE_2D, 1, TextureFormatToInternalGL(attachment.format), m_settings.width, m_settings.height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrapToGL(attachment.wrapModeU));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrapToGL(attachment.wrapModeV));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(attachment.filteringMin));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(attachment.filteringMag));

	glBindTexture(GL_TEXTURE_2D, 0);

	switch (attachment.format)
	{
	case Resources::TextureFormat::Depth24Stencil8:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureID, 0);
		break;
	case Resources::TextureFormat::Depth16:
	case Resources::TextureFormat::Depth24:
	case Resources::TextureFormat::Depth32:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureID, 0);
		break;
	case Resources::TextureFormat::Stencil8:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureID, 0);
	default:
		PRLOG_ERROR("Framebuffer ID:{0} wrong depth format", m_ID);
		break;
	}
}

void GLFramebuffer::GenerateTexture(unsigned int p_index)
{
	const auto& textureSettings = m_colorTextureAttachments[p_index];
	
	Resources::TexturePtr texture;
	if (textureSettings.cubeTexture)
	{
		auto cubemap = std::make_shared<OpenGL::GLCubemap>(m_colorTextureIDs[p_index], m_settings.width, m_settings.height, textureSettings.format);
		cubemap->SetWrapModeR(textureSettings.wrapModeR);
		texture = cubemap;
	}
	else
		texture = std::make_shared<OpenGL::GLTexture2D>(m_colorTextureIDs[p_index], m_settings.width, m_settings.height, textureSettings.format);

	texture->SetMagFiltering(textureSettings.filteringMag);
	texture->SetMinFiltering(textureSettings.filteringMin);

	texture->SetWrapModeU(textureSettings.wrapModeU);
	texture->SetWrapModeV(textureSettings.wrapModeV);

	m_colorTextures[p_index] = texture;
	
	//ID set to zero to show that resposibility to delete is moved to outside the frambuffer
	m_colorTextureIDs[p_index] = 0;
}