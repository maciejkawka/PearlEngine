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
	DeleteTextures();
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
	if (p_attachment > m_colorTextureAttachments.size() || p_textureTarget > 5 || p_textureTarget < 0 || m_colorTextureAttachments.empty())
	{
		PRLOG_ERROR("Framebuffer {0}: wrong SetLevelOfTexture", m_ID);
		return;
	}

	if (!m_settings.mipMaped)
	{
		p_mipLevel = 0;
		PRLOG_WARN("Framebuffer {0}: framebuffr is not mipmaped", m_ID);
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + p_attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + p_textureTarget, m_colorTextureIDs[p_attachment], p_mipLevel);

	//check if texture has size specified if yes use this one is not use the global one
	if (m_colorTextureAttachments[p_attachment].width != 0 && m_colorTextureAttachments[p_attachment].height)
		glViewport(0, 0, m_colorTextureAttachments[p_attachment].width / PrCore::Math::pow(2, p_mipLevel), m_colorTextureAttachments[p_attachment].height / PrCore::Math::pow(2, p_mipLevel));
	else
		glViewport(0, 0, m_settings.globalWidth / PrCore::Math::pow(2, p_mipLevel), m_settings.globalHeight / PrCore::Math::pow(2, p_mipLevel));
}

void GLFramebuffer::Resize(size_t width, size_t height)
{
	if (width == 0 || height == 0)
		return;

	m_settings.globalWidth = width;
	m_settings.globalHeight = height;

	UpdateFamebuffer();
}

void GLFramebuffer::ClearAttachmentColor(unsigned int p_attachment, PrRenderer::Core::Color p_color)
{
	if(p_attachment < m_colorTextureIDs.size());

	auto format = TextureFormatToGL(m_colorTextureAttachments[p_attachment].format);
	glClearTexImage(m_colorTextureIDs[p_attachment], 0,
		format, GL_FLOAT, &p_color);
}

PrRenderer::Resources::TexturePtr GLFramebuffer::GetTexturePtr(unsigned int p_index)
{
	if (p_index >= m_colorTextureAttachments.size())
		return Resources::TexturePtr();

	//Generate texturePtr and mark texture Id as untracked, this texture
	//will not be deleted when framebuffer is deleted
	GenerateTexture(p_index);
	auto element = m_trackedAttachments.find(m_colorTextureIDs[p_index]);
	m_trackedAttachments.erase(element);

	return m_colorTextures[p_index];
}

PrRenderer::RendererID GLFramebuffer::GetTextureID(unsigned int p_index)
{
	if (p_index >= m_colorTextureAttachments.size())
		return 0;

	//Take texture ID and mark texture ID as untracked, this texture
	//will not be deleted when framebuffer is deleted
	auto id = m_colorTextureIDs[p_index];
	auto element = m_trackedAttachments.find(m_colorTextureIDs[p_index]);
	m_trackedAttachments.erase(element);

	return id;
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
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	m_colorTextures.resize(m_colorTextureAttachments.size());
}

void GLFramebuffer::CreateCubemapAttachment(int p_attachmentIndex)
{
	const auto& colorTexAttachments = m_settings.colorTextureAttachments.textures;
	const auto& attachment = colorTexAttachments[p_attachmentIndex];

	//Create cube texture
	unsigned int textureID;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	m_colorTextureIDs.push_back(textureID);
	m_trackedAttachments.insert(textureID);

	if (m_settings.globalWidth != m_settings.globalHeight)
		m_settings.globalHeight = m_settings.globalWidth;

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

	for (int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, TextureFormatToInternalGL(attachment.format), width, height,
			0, TextureFormatToGL(attachment.format), TextureFormatToDataTypeGL(attachment.format), nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, TextureWrapToGL(attachment.wrapModeU));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, TextureWrapToGL(attachment.wrapModeV));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, TextureWrapToGL(attachment.wrapModeR));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(attachment.filteringMin));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(attachment.filteringMag));
	if(m_settings.mipMaped)
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, textureID, 0);

	m_colorTextureAttachments.push_back(attachment);
}

void GLFramebuffer::CreateTextureAttachment(int p_attachmentIndex)
{
	const auto& colorTexAttachments = m_settings.colorTextureAttachments.textures;
	const auto& attachment = colorTexAttachments[p_attachmentIndex];

	//Create 2D texture
	unsigned int textureID;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	m_colorTextureIDs.push_back(textureID);
	m_trackedAttachments.insert(textureID);

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

	glTexImage2D(GL_TEXTURE_2D, 0, TextureFormatToInternalGL(attachment.format), width, height,
		0, TextureFormatToGL(attachment.format), TextureFormatToDataTypeGL(attachment.format), nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrapToGL(attachment.wrapModeU));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrapToGL(attachment.wrapModeV));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(attachment.filteringMin));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(attachment.filteringMag));
	if (m_settings.mipMaped)
		glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

	m_colorTextureAttachments.push_back(attachment);
}


void GLFramebuffer::UpdateDepthTexture()
{
	const auto& attachment = m_settings.depthStencilAttachment;

	//Create texture2D
	glCreateTextures(GL_TEXTURE_2D, 1, &m_depthTextureID);
	glBindTexture(GL_TEXTURE_2D, m_depthTextureID);

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

	glTexStorage2D(GL_TEXTURE_2D, 1, TextureFormatToInternalGL(attachment.format), width, width);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrapToGL(attachment.wrapModeU));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrapToGL(attachment.wrapModeV));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(attachment.filteringMin));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(attachment.filteringMag));
	if (m_settings.mipMaped)
		glGenerateMipmap(GL_TEXTURE_2D);
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

	//Use global size if the attachment does not have set
	size_t width;
	size_t height;
	if (textureSettings.width == 0 && textureSettings.height == 0)
	{
		width = m_settings.globalWidth;
		height = m_settings.globalHeight;
	}
	else
	{
		width = textureSettings.width;
		height = textureSettings.height;
	}

	Resources::TexturePtr texture;
	if (textureSettings.cubeTexture)
	{
		auto cubemap = std::make_shared<OpenGL::GLCubemap>(m_colorTextureIDs[p_index], width, height, textureSettings.format);
		cubemap->SetWrapModeR(textureSettings.wrapModeR);
		texture = cubemap;
	}
	else
		texture = std::make_shared<OpenGL::GLTexture2D>(m_colorTextureIDs[p_index], width, height, textureSettings.format);

	texture->SetMagFiltering(textureSettings.filteringMag);
	texture->SetMinFiltering(textureSettings.filteringMin);

	texture->SetWrapModeU(textureSettings.wrapModeU);
	texture->SetWrapModeV(textureSettings.wrapModeV);

	m_colorTextures[p_index] = texture;
}

void GLFramebuffer::DeleteTextures()
{
	//Delete only textures that were not taken from the framebuffer
	for (int i = 0; i < m_colorTextureIDs.size(); i++)
	{
		if (m_trackedAttachments.find(m_colorTextureIDs[i]) != m_trackedAttachments.end())
			glDeleteTextures(1, &m_colorTextureIDs[i]);
	}

	glDeleteTextures(1, &m_depthTextureID);

	//Clear all vectors
	m_colorTextureAttachments.clear();
	m_depthStencilTextureAttachment.format = Resources::TextureFormat::None;
	m_trackedAttachments.clear();
	m_colorTextures.clear();
	m_colorTextureIDs.clear();
	m_depthTextureID = 0;
}
