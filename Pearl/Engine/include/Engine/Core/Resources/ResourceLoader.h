#pragma once
#include"Core/Utils/Singleton.h"

#include<memory>
#include<string>

namespace PrRenderer::Resources {
	class MaterialManager;
	class MeshManager;
	class TextureManager;
	class ShaderManager;
	class CubemapManager;
}

namespace PrCore::Resources {

	class ResourceManager;

	class ResourceLoader: public Utils::Singleton<ResourceLoader> {
	public:
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
		~ResourceLoader();

		PrRenderer::Resources::MaterialManager* m_materialManager;
		PrRenderer::Resources::MeshManager* m_meshManager;
		PrRenderer::Resources::TextureManager* m_textureManager;
		PrRenderer::Resources::CubemapManager* m_cubemapManager;
		PrRenderer::Resources::ShaderManager* m_shaderManager;
		
		friend Singleton<ResourceLoader>;
	};


}