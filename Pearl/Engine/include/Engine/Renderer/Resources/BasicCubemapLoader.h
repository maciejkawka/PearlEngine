#pragma once

#include "Core/Resources/IResourceDataLoader.h"
#include "Renderer/Resources/Texture2D.h"

namespace PrRenderer {

	using namespace PrCore;

	class BasicCubemapLoader : public PrCore::IResourceDataLoader {
	public:
		IResourceDataPtr LoadResource(const std::string& p_path) override;

		void UnloadResource(IResourceDataPtr p_resourceData) override;

		bool SaveResourceOnDisc(IResourceDataPtr p_resourceData, const std::string& p_path) override;

	private:
		unsigned char* LoadTexture(const std::string& p_path, int& p_width, int& p_height, PrRenderer::TextureFormat& p_format);
	};
}
