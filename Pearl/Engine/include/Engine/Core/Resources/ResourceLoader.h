#pragma once

#include<memory>
#include<string>

namespace PrRenderer::Resources {
	class MaterialManager;
	class MeshManager;
	class TextureManager;
	class ShaderManager;
}

namespace PrCore::Resources {

	class ResourceManager;

	class ResourceLoader {
	public:
		static ResourceLoader& GetInstance();
		static void Terminate();

		template<class T>
		std::shared_ptr<T> LoadResource(const std::string& p_name);

		template<class T>
		void UnloadResource(const std::string& p_name);
		
		template<class T = int>
		void UnloadAllResources();
		
		template<class T>
		std::shared_ptr<T> GetResource(const std::string& p_name);

		template<class T>
		void DeleteResource(const std::string& p_name);

		template<class T = int>
		void DeleteAllResources();
		
		template<class T>
		const ResourceManager* GetResourceManager();

	private:
		ResourceLoader();
		ResourceLoader(ResourceManager&) = delete;
		ResourceLoader(ResourceManager&&) = delete;
		~ResourceLoader();

		PrRenderer::Resources::MaterialManager* m_materialManager;
		PrRenderer::Resources::MeshManager* m_meshManager;
		PrRenderer::Resources::TextureManager* m_textureManager;
		PrRenderer::Resources::ShaderManager* m_shaderManager;

		inline static ResourceLoader* s_instance = nullptr;;
	};


}