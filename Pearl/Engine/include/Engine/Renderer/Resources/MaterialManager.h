#pragma once
#include"Core/Resources/ResourceManager.h"

namespace PrRenderer::Resources {

	class MaterialManager : public PrCore::Resources::ResourceManager {
	public:
		MaterialManager();

	protected:
		PrCore::Resources::Resource* CreateImpl(const std::string& p_name) override;
	};
}