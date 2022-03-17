#pragma once 
#include"Renderer/Buffers/Framebuffer.h"

namespace PrRenderer::OpenGL {

	class GLFramebuffer : public Buffers::Framebufffer {
	public:
		GLFramebuffer(const Buffers::FramebufferSettings& p_settings);
		~GLFramebuffer();

		void Bind() override;
		void Unbind() override;

		void Resize(size_t width, size_t height) override;
		virtual void ClearAttachmentColor(unsigned int p_attachment, int p_value) override;

	private:
		void UpdateFamebuffer();
		void UpdateColorTextures();
		void UpdateDepthTexture();

		std::vector<Buffers::FramebufferTexture> m_colorTextureAttachments;
		Buffers::FramebufferTexture m_depthStencilTextureAttachment;
	};

}