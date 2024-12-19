#include "Core/Common/pearl_pch.h"
		 
#include "Core/Windowing/Window.h"
		 
#include "Renderer/OpenGL/GLFramebuffer.h"
#include "Renderer/OpenGL/GLTexture2D.h"
#include "Renderer/OpenGL/GLCubemap.h"
#include "Renderer/OpenGL/GLUtils.h"
		 
#include "glad/glad.h"

using namespace PrRenderer::OpenGL;

GLFramebuffer::GLFramebuffer(const Buffers::FramebufferSettings& p_settings):
	Framebufffer()
{
	m_settings = p_settings;
	UpdateFamebuffer();
}

GLFramebuffer::~GLFramebuffer()
{
	DeleteTextures();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &m_ID);
}

void GLFramebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
	
	//If global settings set use global size if not the SetLevelofTexture need to be called
	if(m_settings.globalWidth != 0 && m_settings.globalHeight != 0) 
		glViewport(0, 0, m_settings.globalWidth, m_settings.globalHeight);
}


void GLFramebuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	auto& window = PrCore::Windowing::Window::GetMainWindow();
	glViewport(0, 0, window.GetWidth(), window.GetHeight());
}

void GLFramebuffer::SetAttachmentDetails(int p_attachment, int p_textureTarget, int p_mipLevel)
{
	if (p_attachment > m_colorTextures.size() || p_textureTarget > 5 || p_textureTarget < 0 || m_colorTextures.empty())
	{
		PRLOG_ERROR("Framebuffer {0}: wrong SetLevelOfTexture", m_ID);
		return;
	}

	if (p_mipLevel > 0 && !m_settings.mipMaped)
	{
		p_mipLevel = 0;
		PRLOG_WARN("Framebuffer {0}: framebuffr is not mipmaped", m_ID);
	}

	auto texture = m_colorTextures[p_attachment];
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + p_attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + p_textureTarget, texture->GetRendererID(), p_mipLevel);
	glViewport(0, 0, texture->GetWidth() / PrCore::Math::pow(2, p_mipLevel), texture->GetHeight() / PrCore::Math::pow(2, p_mipLevel));
}

void GLFramebuffer::Resize(size_t width, size_t height)
{
	if (width == 0 || height == 0)
		return;

	m_settings.globalWidth = width;
	m_settings.globalHeight = height;

	UpdateFamebuffer();
}

void GLFramebuffer::ClearAttachmentColor(unsigned int p_attachemntIndex, const Core::Color& p_color)
{
	PR_ASSERT(p_attachemntIndex < m_colorTextures.size(), "Framebuffer attachemnt index over the size" + this->m_ID);
	m_colorTextures[p_attachemntIndex]->ClearWithColor(p_color);
}

PrRenderer::Resources::TexturePtr GLFramebuffer::GetTexturePtr(unsigned int p_index)
{
	PR_ASSERT(p_index < m_colorTextures.size(), "Framebuffer attachemnt index over the size" + this->m_ID);
	return m_colorTextures[p_index];
}

PrRenderer::Resources::TexturePtr PrRenderer::OpenGL::GLFramebuffer::GetDepthTexturePtr()
{
	return m_depthTexture;
}

void GLFramebuffer::UpdateFamebuffer()
{
	if (m_ID)
	{
		glDeleteFramebuffers(1, &m_ID);
		DeleteTextures();
	}

	glCreateFramebuffers(1, &m_ID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ID);

	if (!m_settings.colorTextureAttachments.textures.empty())
		UpdateColorTextures();

	if (m_settings.depthStencilAttachment.format != Resources::TextureFormat::None)
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
	for (int i = 0; i < m_colorTextures.size(); i++)
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	glDrawBuffers(attachments.size(), attachments.data());
}

void GLFramebuffer::CreateCubemapAttachment(int p_attachmentIndex)
{
	const auto& colorTexAttachments = m_settings.colorTextureAttachments.textures;
	const auto& attachment = colorTexAttachments[p_attachmentIndex];

	//Use global size if the attachment does not have set
	size_t width;
	size_t height;
	if (attachment.width == 0 && attachment.height == 0)
	{
		width = m_settings.globalWidth;
		height = m_settings.globalHeight;
	}
	else
	{
		width = attachment.width;
		height = attachment.height;
	}

	// Create cubetexture ptr
	auto cubemap = Resources::Cubemap::Create();
	cubemap->SetFormat(attachment.format);
	cubemap->SetWidth(width);
	cubemap->SetHeight(height);

	cubemap->SetWrapModeU(attachment.wrapModeU);
	cubemap->SetWrapModeV(attachment.wrapModeV);
	cubemap->SetWrapModeR(attachment.wrapModeR);

	cubemap->SetMinFiltering(attachment.filteringMin);
	cubemap->SetMagFiltering(attachment.filteringMag);

	cubemap->SetMipMap(m_settings.mipMaped);
	cubemap->Apply();

	m_colorTextures.push_back(cubemap);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubemap->GetRendererID(), 0);
}

void GLFramebuffer::CreateTextureAttachment(int p_attachmentIndex)
{
	const auto& colorTexAttachments = m_settings.colorTextureAttachments.textures;
	const auto& attachment = colorTexAttachments[p_attachmentIndex];

	//Use global size if the attachment does not have set
	size_t width;
	size_t height;
	if (attachment.width == 0 && attachment.height == 0)
	{
		width = m_settings.globalWidth;
		height = m_settings.globalHeight;
	}
	else
	{
		width = attachment.width;
		height = attachment.height;
	}

	// Create Texture2DPtr
	auto texture = Resources::Texture2D::Create();
	texture->SetFormat(attachment.format);
	texture->SetWidth(width);
	texture->SetHeight(height);

	texture->SetWrapModeU(attachment.wrapModeU);
	texture->SetWrapModeV(attachment.wrapModeV);

	texture->SetMinFiltering(attachment.filteringMin);
	texture->SetMagFiltering(attachment.filteringMag);

	texture->SetMipMap(m_settings.mipMaped);
	texture->Apply();

	m_colorTextures.push_back(texture);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + p_attachmentIndex, GL_TEXTURE_2D, texture->GetRendererID(), 0);
}

void GLFramebuffer::UpdateDepthTexture()
{
	const auto& attachment = m_settings.depthStencilAttachment;

	size_t width;
	size_t height;
	if (attachment.width == 0 && attachment.height == 0)
	{
		width = m_settings.globalWidth;
		height = m_settings.globalHeight;
	}
	else
	{
		width = attachment.width;
		height = attachment.height;
	}

	// Create Texture2DPtr
	auto texture = Resources::Texture2D::Create();
	texture->SetFormat(attachment.format);
	texture->SetWidth(width);
	texture->SetHeight(height);

	texture->SetWrapModeU(attachment.wrapModeU);
	texture->SetWrapModeV(attachment.wrapModeV);

	texture->SetMinFiltering(attachment.filteringMin);
	texture->SetMagFiltering(attachment.filteringMag);

	texture->SetMipMap(m_settings.mipMaped);
	texture->Apply();

	switch (attachment.format)
	{
	case Resources::TextureFormat::Depth24Stencil8:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture->GetRendererID(), 0);
		break;
	case Resources::TextureFormat::Depth16:
	case Resources::TextureFormat::Depth24:
	case Resources::TextureFormat::Depth32:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->GetRendererID(), 0);
		break;
	case Resources::TextureFormat::Stencil8:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture->GetRendererID(), 0);
	default:
		PRLOG_ERROR("Framebuffer ID:{0} wrong depth format", m_ID);
		break;
	}

	m_depthTexture = texture;
}

void GLFramebuffer::DeleteTextures()
{
	// Release the textures ownership
	for (auto& texture : m_colorTextures)
	{
		texture.reset();
		texture = nullptr;
	}

	m_depthTexture.reset();
	m_depthTexture = nullptr;
}