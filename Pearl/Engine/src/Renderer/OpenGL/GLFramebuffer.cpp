#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLFramebuffer.h"
#include"Renderer/OpenGL/GLTexture2D.h"
#include"Renderer/OpenGL/GLUtils.h"

#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

GLFramebuffer::GLFramebuffer(const Buffers::FramebufferSettings& p_settings)
{
	m_settings = p_settings;

	UpdateFamebuffer();
}

GLFramebuffer::~GLFramebuffer()
{
	glDeleteFramebuffers(1, &m_ID);
	glDeleteTextures(m_colorTextureIDs.size(), m_colorTextureIDs.data());
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
		const auto& attachment = colorTexAttachments[i];

		unsigned int textureID;
		glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
		m_colorTextureIDs.push_back(textureID);
		
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, TextureFormatToInternalGL(attachment.format), m_settings.width, m_settings.height,
				0, TextureFormatToGL(attachment.format), GL_UNSIGNED_INT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrapToGL(attachment.wrapModeU));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrapToGL(attachment.wrapModeV));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(attachment.filteringMin));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(attachment.filteringMag));
		
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureID, 0);
		m_colorTextureAttachments.push_back(attachment);
	}

	std::vector<GLenum> attachments;
	for (int i = 0; i < m_colorTextureAttachments.size(); i++)
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	glDrawBuffers(attachments.size(), attachments.data());
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
