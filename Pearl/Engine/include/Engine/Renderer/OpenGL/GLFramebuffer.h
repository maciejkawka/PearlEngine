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

		void         Resize(size_t width, size_t height) override;
		virtual void ClearAttachmentColor(unsigned int p_attachemntIndex, const Core::Color& p_color) override;

		virtual Resources::TexturePtr GetTexturePtr(unsigned int p_index = 0) override;
		virtual Resources::TexturePtr GetDepthTexturePtr() override;

	private:
		void UpdateFamebuffer();
		void UpdateColorTextures();
		void CreateCubemapAttachment(int p_attachmentIndex);
		void CreateTextureAttachment(int p_attachmentIndex);
		void UpdateDepthTexture();

		void DeleteTextures();
	};
}