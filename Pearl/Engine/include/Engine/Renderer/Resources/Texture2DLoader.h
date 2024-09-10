#pragma once

#include "Core/Resources/IResourceDataLoader.h"

namespace PrRenderer::Resources {

	using namespace PrCore::Resources;

	class Texture2DLoader : public PrCore::Resources::IResourceDataLoader {
	public:
		Texture2DLoader() = default;

		IResourceDataPtr LoadResource(const std::string& p_path) override;

		void UnloadResource(IResourceDataPtr p_resourceData) override;

		bool SaveResourceOnDisc(IResourceDataPtr p_resourceData, const std::string& p_path) override;
	};
}
