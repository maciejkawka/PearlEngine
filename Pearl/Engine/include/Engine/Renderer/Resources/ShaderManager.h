#include"Core/Resources/ResourceManager.h"

namespace PrRenderer::Resources {

	class ShaderManager: public PrCore::Resources::ResourceManager {
	public:
		static ShaderManager& GetInstance();
		static void Terminate();

	protected:
		PrCore::Resources::Resources* CreateImpl(const std::string& p_name) override;
		void DeleteImpl(PrCore::Resources::ResourcePtr& p_resource) override;

	private:
		ShaderManager();

		
		static ShaderManager* m_instance;
	};
}