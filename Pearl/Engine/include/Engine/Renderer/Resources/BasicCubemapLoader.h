#pragma once

#include "Core/Resources/IResourceDataLoader.h"
#include "Renderer/Resources/Texture2Dv2.h"

namespace PrRenderer::Resources {

	using namespace PrCore::Resources;

	class BasicCubemapLoader : public PrCore::Resources::IResourceDataLoader {
	public:
		IResourceDataPtr LoadResource(const std::string& p_path) override;

		void UnloadResource(IResourceDataPtr p_resourceData) override;
	
		bool SaveResourceOnDisc(IResourceDataPtr p_resourceData, const std::string& p_path) override;

	private:
		unsigned char*   LoadTexture(const std::string& p_path, int& p_width, int& p_height, PrRenderer::Resources::TextureFormat& p_format);
	};
}
