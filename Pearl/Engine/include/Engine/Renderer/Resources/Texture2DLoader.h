#pragma once

#include "Core/Resources/IResourceDataLoader.h"

namespace PrRenderer::Resources {

	using namespace PrCore::Resources;

	class Texture2DLoader : public PrCore::Resources::IResourceDataLoader {
	public:

		enum Texture2DFlag 
		{
			FlagDesiredChannelsGrayScale = 1,
			FlagDesiredChannelsRGB = 2,
			FlagDesiredChannelsRGBA = 4
		};

		Texture2DLoader() = default;

		IResourceDataPtr LoadResource(const std::string& p_path) override;

		IResourceDataPtr LoadFromMemoryResource(const void* p_buffer, size_t p_size, int p_flags = 0);

		void UnloadResource(IResourceDataPtr p_resourceData) override;

		bool SaveResourceOnDisc(IResourceDataPtr p_resourceData, const std::string& p_path) override;
	};
}
