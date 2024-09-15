#pragma once 
#include"Renderer/Buffers/Framebuffer.h"

#include<set>
namespace PrRenderer::OpenGL {

	class GLFramebuffer : public Buffers::Framebufffer {
	public:
		GLFramebuffer(const Buffers::FramebufferSettings& p_settings);
		~GLFramebuffer() override;

		void Bind() override;
		void Unbind() override;
		void SetAttachmentDetails(int p_attachment, int p_textureTarget, int p_mipLevel = 0) override;

		void Resize(size_t width, size_t height) override;
		virtual void ClearAttachmentColor(unsigned int p_attachment, Core::Color p_color) override;

		virtual Resources::Texturev2Ptr GetTexturePtr(unsigned int p_index = 0) override;
		virtual Resources::Texturev2Ptr GetDepthTexturePtr() override;

		//Unprotected function call only if you know what to do with the texture
		//Texture has to be deleted manually
		virtual RendererID GetTextureID(unsigned int p_index = 0) override;

	private:
		void UpdateFamebuffer();
		void UpdateColorTextures();
		void CreateCubemapAttachment(int p_attachmentIndex);
		void CreateTextureAttachment(int p_attachmentIndex);
		void UpdateDepthTexture();

		void GenerateTexture(unsigned int p_index);
		void GenerateDepthTexture();

		void DeleteTextures();

		std::vector<Buffers::FramebufferTexture> m_colorTextureAttachments;
		std::set<RendererID> m_trackedAttachments;
		std::vector<Resources::Texturev2Ptr> m_colorTextures;
		Resources::Texturev2Ptr m_depthTexture;
		Buffers::FramebufferTexture m_depthStencilTextureAttachment;
	};

}