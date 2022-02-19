#pragma once
#include"Core/Resources/ResourceManager.h"

namespace PrRenderer::Resources {

	class MaterialManager : public PrCore::Resources::ResourceManager {
	public:
		static MaterialManager& GetInstance();
		static void Terminate();

	protected:
		PrCore::Resources::Resources* CreateImpl(const std::string& p_name) override;

	private:
		MaterialManager();


		static MaterialManager* m_instance;
	};
}