#pragma once
#include"Core/Resources/ResourceManager.h"

namespace PrRenderer::Resources {

	class TextureManager : public PrCore::Resources::ResourceManager {
	public:
		TextureManager();

	protected:
		PrCore::Resources::Resource* CreateImpl(const std::string& p_name) override;
	};
}