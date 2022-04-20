#pragma once
#include"Core/Resources/ResourceManager.h"

namespace PrRenderer::Resources {

	class ShaderManager: public PrCore::Resources::ResourceManager {
	public:
		ShaderManager();

	protected:
		PrCore::Resources::Resource* CreateImpl(const std::string& p_name) override;
	};
}