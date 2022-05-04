#pragma once
#include"Renderer/Resources/Texture2D.h"

namespace PrRenderer::OpenGL {

	class GLTexture2D : public PrRenderer::Resources::Texture2D {
	public:
		GLTexture2D() = default;

		GLTexture2D(RendererID p_rendererID, size_t p_width, size_t p_height, PrRenderer::Resources::TextureFormat p_format);

		//Constructor for managed resource
		GLTexture2D(const std::string& p_name, PrCore::Resources::ResourceHandle p_handle) :
			Texture2D(p_name, p_handle)
		{}

		void Bind(unsigned int p_slot = 0) override;
		void Unbind(unsigned int p_slot = 0) override;

		//To implent in future
		virtual void IsMipMapped(bool p_mipmap) override;
		virtual void IsReadable(bool p_readable) override;
		
		virtual void SetMinFiltering(PrRenderer::Resources::TextureFiltering p_minfiltering) override;
		virtual void SetMagFiltering(PrRenderer::Resources::TextureFiltering p_magfiltering) override;
		virtual void SetWrapModeU(PrRenderer::Resources::TextureWrapMode p_wrapU) override;
		virtual void SetWrapModeV(PrRenderer::Resources::TextureWrapMode p_wrapV) override;

	private:
		void PreLoadImpl() override;
		bool LoadImpl() override;
		void PostLoadImpl() override;

		void PreUnloadImpl() override;
		bool UnloadImpl() override;
		void PostUnloadImpl() override;

		void LoadDefault() override;

		void CalculateSize() override;

		unsigned char* ReadRawData();
	};
}