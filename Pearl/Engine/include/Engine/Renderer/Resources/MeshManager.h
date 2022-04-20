#pragma once
#include"Core/Resources/ResourceManager.h"

namespace PrRenderer::Resources {

	class MeshManager : public PrCore::Resources::ResourceManager {
	public:
		MeshManager();

	protected:
		PrCore::Resources::Resource* CreateImpl(const std::string& p_name) override;
	};
}