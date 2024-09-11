#pragma once

#include "Core/Resources/IResourceDataLoader.h"

namespace PrRenderer::Resources {

	class ShaderLoader : public PrCore::Resources::IResourceDataLoader {
	public:
		PrCore::Resources::IResourceDataPtr LoadResource(const std::string& p_path) override;

		void UnloadResource(PrCore::Resources::IResourceDataPtr p_resourceData) override;

		bool SaveResourceOnDisc(PrCore::Resources::IResourceDataPtr p_resourceData, const std::string& p_path) override;
	};
}
