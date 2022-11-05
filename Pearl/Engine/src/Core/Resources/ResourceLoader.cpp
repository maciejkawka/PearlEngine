#include"Core/Common/pearl_pch.h"

#include"Core/Resources/ResourceLoader.h"

#include"Renderer/Resources/MaterialManager.h"
#include"Renderer/Resources/ShaderManager.h"
#include"Renderer/Resources/TextureManager.h"
#include"Renderer/Resources/CubemapManager.h"
#include"Renderer/Resources/MeshManager.h"

#include"Renderer/Resources/Material.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Texture2D.h"
#include"Renderer/Resources/Cubemap.h"
#include"Renderer/Resources/Shader.h"

using namespace PrCore::Resources;

ResourceLoader::ResourceLoader()
{
	m_materialManager = new PrRenderer::Resources::MaterialManager();
	m_meshManager = new PrRenderer::Resources::MeshManager();
	m_textureManager = new PrRenderer::Resources::TextureManager();
	m_cubemapManager = new PrRenderer::Resources::CubemapManager();
	m_shaderManager = new PrRenderer::Resources::ShaderManager();
}

ResourceLoader::~ResourceLoader()
{
	delete m_materialManager;
	delete m_meshManager;
	delete m_textureManager;
	delete m_cubemapManager;
	delete m_shaderManager;
}

template <class T>
std::shared_ptr<T> ResourceLoader::LoadResource(const std::string& p_name)
{
	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Material).hash_code())
		return std::static_pointer_cast<T>(m_materialManager->Load(p_name));

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Mesh).hash_code())
		return std::static_pointer_cast<T>(m_meshManager->Load(p_name));

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture).hash_code() ||
		typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture2D).hash_code())
		return std::static_pointer_cast<T>(m_textureManager->Load(p_name));

	if(typeid(T).hash_code() == typeid(PrRenderer::Resources::Cubemap).hash_code())
		return std::static_pointer_cast<T>(m_cubemapManager->Load(p_name));

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Shader).hash_code())
		return std::static_pointer_cast<T>(m_shaderManager->Load(p_name));

	return std::shared_ptr<T>();
}

template std::shared_ptr<PrRenderer::Resources::Material> ResourceLoader::LoadResource<PrRenderer::Resources::Material>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Mesh> ResourceLoader::LoadResource<PrRenderer::Resources::Mesh>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Texture> ResourceLoader::LoadResource<PrRenderer::Resources::Texture>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Texture2D> ResourceLoader::LoadResource<PrRenderer::Resources::Texture2D>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Cubemap> ResourceLoader::LoadResource<PrRenderer::Resources::Cubemap>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Shader> ResourceLoader::LoadResource<PrRenderer::Resources::Shader>(const std::string& p_name);


template<class T>
void ResourceLoader::UnloadResource(const std::string& p_name)
{
	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Material).hash_code())
		m_materialManager->Unload(p_name);

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Mesh).hash_code())
		m_meshManager->Unload(p_name);

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture).hash_code() ||
		typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture2D).hash_code())
		m_textureManager->Unload(p_name);

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Cubemap).hash_code())
		m_cubemapManager->Unload(p_name);

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Shader).hash_code())
		m_shaderManager->Unload(p_name);
}

template void ResourceLoader::UnloadResource<PrRenderer::Resources::Material>(const std::string& p_name);
template void ResourceLoader::UnloadResource<PrRenderer::Resources::Mesh>(const std::string& p_name);
template void ResourceLoader::UnloadResource<PrRenderer::Resources::Texture>(const std::string& p_name);
template void ResourceLoader::UnloadResource<PrRenderer::Resources::Texture2D>(const std::string& p_name);
template void ResourceLoader::UnloadResource<PrRenderer::Resources::Cubemap>(const std::string& p_name);
template void ResourceLoader::UnloadResource<PrRenderer::Resources::Shader>(const std::string& p_name);


template<class T>
void ResourceLoader::UnloadAllResources()
{
	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Material).hash_code())
		m_materialManager->UnloadAll();

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Mesh).hash_code())
		m_meshManager->UnloadAll();

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture).hash_code() ||
		typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture2D).hash_code())
		m_textureManager->UnloadAll();

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Cubemap).hash_code())
		m_cubemapManager->UnloadAll();

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Shader).hash_code())
		m_shaderManager->UnloadAll();

	if (typeid(T).hash_code() == typeid(int).hash_code())
	{
		m_materialManager->UnloadAll();
		m_meshManager->UnloadAll();
		m_textureManager->UnloadAll();
		m_shaderManager->UnloadAll();
	}
}

template void ResourceLoader::UnloadAllResources<PrRenderer::Resources::Material>();
template void ResourceLoader::UnloadAllResources<PrRenderer::Resources::Mesh>();
template void ResourceLoader::UnloadAllResources<PrRenderer::Resources::Texture>();
template void ResourceLoader::UnloadAllResources<PrRenderer::Resources::Texture2D>();
template void ResourceLoader::UnloadAllResources<PrRenderer::Resources::Cubemap>();
template void ResourceLoader::UnloadAllResources<PrRenderer::Resources::Shader>();
template void ResourceLoader::UnloadAllResources<int>();


template<class T>
std::shared_ptr<T> ResourceLoader::GetResource(const std::string& p_name)
{
	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Material).hash_code())
		return std::static_pointer_cast<T>(m_materialManager->GetResource(p_name));

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Mesh).hash_code())
		return std::static_pointer_cast<T>(m_meshManager->GetResource(p_name));

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture).hash_code() ||
		typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture2D).hash_code())
		return std::static_pointer_cast<T>(m_textureManager->GetResource(p_name));

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Cubemap).hash_code())
		return std::static_pointer_cast<T>(m_cubemapManager->GetResource(p_name));

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Shader).hash_code())
		return std::static_pointer_cast<T>(m_shaderManager->GetResource(p_name));

	return std::shared_ptr<T>();
}

template std::shared_ptr<PrRenderer::Resources::Material> ResourceLoader::GetResource<PrRenderer::Resources::Material>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Mesh> ResourceLoader::GetResource<PrRenderer::Resources::Mesh>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Texture> ResourceLoader::GetResource<PrRenderer::Resources::Texture>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Texture2D> ResourceLoader::GetResource<PrRenderer::Resources::Texture2D>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Cubemap> ResourceLoader::GetResource<PrRenderer::Resources::Cubemap>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Shader> ResourceLoader::GetResource<PrRenderer::Resources::Shader>(const std::string& p_name);


template<class T>
void ResourceLoader::DeleteResource(const std::string& p_name)
{
	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Material).hash_code())
		m_materialManager->Delete(p_name);

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Mesh).hash_code())
		m_meshManager->Delete(p_name);

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture).hash_code() ||
		typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture2D).hash_code())
		m_textureManager->Delete(p_name);

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Cubemap).hash_code())
		m_cubemapManager->Delete(p_name);

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Shader).hash_code())
		m_shaderManager->Delete(p_name);
}

template void ResourceLoader::DeleteResource<PrRenderer::Resources::Material>(const std::string& p_name);
template void ResourceLoader::DeleteResource<PrRenderer::Resources::Mesh>(const std::string& p_name);
template void ResourceLoader::DeleteResource<PrRenderer::Resources::Texture>(const std::string& p_name);
template void ResourceLoader::DeleteResource<PrRenderer::Resources::Texture2D>(const std::string& p_name);
template void ResourceLoader::DeleteResource<PrRenderer::Resources::Cubemap>(const std::string& p_name);
template void ResourceLoader::DeleteResource<PrRenderer::Resources::Shader>(const std::string& p_name);


template<class T>
void ResourceLoader::DeleteAllResources()
{
	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Material).hash_code())
		m_materialManager->DeleteAll();

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Mesh).hash_code())
		m_meshManager->DeleteAll();

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture).hash_code() ||
		typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture2D).hash_code())
		m_textureManager->DeleteAll();

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Cubemap).hash_code())
		m_cubemapManager->DeleteAll();

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Shader).hash_code())
		m_shaderManager->DeleteAll();

	if (typeid(T).hash_code() == typeid(int).hash_code())
	{
		m_materialManager->DeleteAll();
		m_meshManager->DeleteAll();
		m_textureManager->DeleteAll();
		m_shaderManager->DeleteAll();
	}
}

template void ResourceLoader::DeleteAllResources<PrRenderer::Resources::Material>();
template void ResourceLoader::DeleteAllResources<PrRenderer::Resources::Mesh>();
template void ResourceLoader::DeleteAllResources<PrRenderer::Resources::Texture>();
template void ResourceLoader::DeleteAllResources<PrRenderer::Resources::Texture2D>();
template void ResourceLoader::DeleteAllResources<PrRenderer::Resources::Cubemap>();
template void ResourceLoader::DeleteAllResources<PrRenderer::Resources::Shader>();
template void ResourceLoader::DeleteAllResources<int>();

template<class T>
const ResourceManager* ResourceLoader::GetResourceManager()
{
	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Material).hash_code())
		return m_materialManager;

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Mesh).hash_code())
		return m_meshManager;

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture).hash_code() ||
		typeid(T).hash_code() == typeid(PrRenderer::Resources::Texture2D).hash_code())
		return m_textureManager;

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Cubemap).hash_code())
		return m_cubemapManager;

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Shader).hash_code())
		return m_shaderManager;

	return nullptr;
}

template const ResourceManager* ResourceLoader::GetResourceManager<PrRenderer::Resources::Material>();
template const ResourceManager* ResourceLoader::GetResourceManager<PrRenderer::Resources::Mesh>();
template const ResourceManager* ResourceLoader::GetResourceManager<PrRenderer::Resources::Texture>();
template const ResourceManager* ResourceLoader::GetResourceManager<PrRenderer::Resources::Texture2D>();
template const ResourceManager* ResourceLoader::GetResourceManager<PrRenderer::Resources::Cubemap>();
template const ResourceManager* ResourceLoader::GetResourceManager<PrRenderer::Resources::Shader>();
