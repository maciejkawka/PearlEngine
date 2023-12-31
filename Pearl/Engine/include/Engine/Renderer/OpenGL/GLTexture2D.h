#pragma once
#include"Renderer/Resources/Texture2D.h"

namespace PrRenderer::OpenGL {

	class GLTexture2D : public Resources::Texture2D {
	public:
		GLTexture2D();
		GLTexture2D(RendererID p_rendererID, size_t p_width, size_t p_height, Resources::TextureFormat p_format);

		//Constructor for managed resource
		GLTexture2D(const std::string& p_name, PrCore::Resources::ResourceHandle p_handle) :
			Texture2D(p_name, p_handle)
		{}

		~GLTexture2D() override;

		void Bind(unsigned int p_slot = 0) override;
		void Unbind(unsigned int p_slot = 0) override;

		void GenerateMipMaps() override;
		void Apply() override;

		//To implent in future
		virtual void IsReadable(bool p_readable) override;
		
		virtual void SetMinFiltering(Resources::TextureFiltering p_minfiltering) override;
		virtual void SetMagFiltering(Resources::TextureFiltering p_magfiltering) override;
		virtual void SetWrapModeU(Resources::TextureWrapMode p_wrapU) override;
		virtual void SetWrapModeV(Resources::TextureWrapMode p_wrapV) override;

		virtual void SetData(void* p_data) override;
	protected:
		void LoadUnitTexture(Core::Color p_unitColor) override;

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