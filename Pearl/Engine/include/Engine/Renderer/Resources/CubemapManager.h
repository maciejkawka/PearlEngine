#pragma once
#include"Core/Resources/ResourceManager.h"

namespace PrRenderer::Resources {

	class CubemapManager : public PrCore::Resources::ResourceManager {
	public:
		CubemapManager();

	protected:
		PrCore::Resources::Resource* CreateImpl(const std::string& p_name) override;
	};
}