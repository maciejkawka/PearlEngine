#pragma once
#include"Core/Resources/ResourceManager.h"

namespace PrRenderer::Resources {

	class TextureManager : public PrCore::Resources::ResourceManager {
	public:
		static TextureManager& GetInstance();
		static void Terminate();

	protected:
		PrCore::Resources::Resource* CreateImpl(const std::string& p_name) override;

	private:
		TextureManager();


		static TextureManager* m_instance;
	};
}
