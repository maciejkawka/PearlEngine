#pragma once

#include "Renderer/Resources/Texture2Dv2.h"
#include "Core/Resources/IResourceDataLoader.h"

namespace PrRenderer::Resources {

	using namespace PrCore::Resources;

	class CubemapLoader : public PrCore::Resources::IResourceDataLoader {
	public:
		IResourceDataPtr LoadResource(const std::string& p_path) override;

		void UnloadResource(IResourceDataPtr p_resourceData) override;
	
		bool SaveResourceOnDisc(IResourceDataPtr p_resourceData, const std::string& p_path) override;

	private:
		IResourceDataPtr LoadSixSided(const std::string& p_path);
		IResourceDataPtr LoadHDR(const std::string& p_path);
		unsigned char*   LoadTexture(const std::string& p_path, int& p_width, int& p_height, PrRenderer::Resources::TextureFormat& p_format);
	};
}
