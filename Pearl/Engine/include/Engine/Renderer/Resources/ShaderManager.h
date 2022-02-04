#pragma once
#include"Core/Resources/ResourceManager.h"

namespace PrRenderer::Resources {

	class ShaderManager: public PrCore::Resources::ResourceManager {
	public:
		static ShaderManager& GetInstance();
		static void Terminate();

	protected:
		PrCore::Resources::Resources* CreateImpl(const std::string& p_name) override;

	private:
		ShaderManager();

		
		static ShaderManager* m_instance;
	};
}