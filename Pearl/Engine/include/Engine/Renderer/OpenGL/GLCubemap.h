#pragma once
#include"Renderer/Resources/Cubemap.h"

namespace PrRenderer::OpenGL {

	class GLCubemap: public Resources::Cubemap{
	public:
		GLCubemap() = default;

		//Constructor for managed resource
		GLCubemap(std::string p_name, PrCore::Resources::ResourceHandle p_handle);

		void Bind(unsigned int p_slot = 0) override;
		void Unbind(unsigned int p_slot = 0) override;
		
		virtual void SetMinFiltering(Resources::TextureFiltering p_minfiltering) override;
		virtual void SetMagFiltering(Resources::TextureFiltering p_magfiltering) override;
		virtual void SetWrapModeU(Resources::TextureWrapMode p_wrapU) override;
		virtual void SetWrapModeV(Resources::TextureWrapMode p_wrapV) override;

	private:
		void PreLoadImpl() override;
		bool LoadImpl() override;
		void PostLoadImpl() override;

		void PreUnloadImpl() override;
		bool UnloadImpl() override;
		void PostUnloadImpl() override;

		void LoadDefault() override;

		void CalculateSize() override;

		unsigned char* ReadRawData(const std::string& p_name, bool p_first);
		bool LoadTexturesNames();

	protected:
		Resources::TextureWrapMode m_wrapR;
	};
	
}
