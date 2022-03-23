#pragma once
#include"Renderer/Resources/Texture2D.h"

namespace PrRenderer::OpenGL {

	class GLTexture2D : public PrRenderer::Resources::Texture2D {
	public:
		GLTexture2D(const std::string& p_name, PrCore::Resources::ResourceID p_ID) :
			Texture2D(p_name, p_ID)
		{}

		void Bind(unsigned int p_slot = 0) override;
		void Unbind() override;

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

		void CalculateSize() override;

		unsigned char* ReadRawData();
	};
}