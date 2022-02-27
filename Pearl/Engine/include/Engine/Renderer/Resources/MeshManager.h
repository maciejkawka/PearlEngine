#pragma once
#include"Core/Resources/ResourceManager.h"

namespace PrRenderer::Resources {

	class MeshManager : public PrCore::Resources::ResourceManager {
	public:
		static MeshManager& GetInstance();
		static void Terminate();

	protected:
		PrCore::Resources::Resources* CreateImpl(const std::string& p_name) override;

	private:
		MeshManager();


		static MeshManager* s_instance;
	};
}