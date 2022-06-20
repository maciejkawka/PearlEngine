#pragma once 
#include"Renderer/Buffers/Framebuffer.h"

namespace PrRenderer::OpenGL {

	class GLFramebuffer : public Buffers::Framebufffer {
	public:
		GLFramebuffer(const Buffers::FramebufferSettings& p_settings);
		~GLFramebuffer();

		void Bind() override;
		void Unbind() override;
		void SetLevelofTexture(int p_attachment, int p_textureLevel) override;

		void Resize(size_t width, size_t height) override;
		virtual void ClearAttachmentColor(unsigned int p_attachment, int p_value) override;

		virtual Resources::TexturePtr GetTexturePtr(unsigned int p_index = 0) override;
		virtual RendererID GetTextureID(unsigned int p_index = 0) override;

	private:
		void UpdateFamebuffer();
		void UpdateColorTextures();
		void CreateCubemapAttachment(int p_attachmentIndex);
		void CreateTextureAttachment(int p_attachmentIndex);
		void UpdateDepthTexture();

		void GenerateTexture(unsigned int p_index);

		std::vector<Buffers::FramebufferTexture> m_colorTextureAttachments;
		std::vector<PrRenderer::Resources::TexturePtr> m_colorTextures;
		Buffers::FramebufferTexture m_depthStencilTextureAttachment;
	};

}