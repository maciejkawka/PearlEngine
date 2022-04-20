#include"Core/Common/pearl_pch.h"

#include"Core/Resources/ResourceLoader.h"

#include"Renderer/Resources/MaterialManager.h"
#include"Renderer/Resources/ShaderManager.h"
#include"Renderer/Resources/TextureManager.h"
#include"Renderer/Resources/MeshManager.h"

#include"Renderer/Resources/Material.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Texture2D.h"
#include"Renderer/Resources/Shader.h"

using namespace PrCore::Resources;

ResourceLoader& ResourceLoader::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new ResourceLoader();

	return *s_instance;
}

void ResourceLoader::Terminate()
{
	if (s_instance)
		delete s_instance;
}

ResourceLoader::ResourceLoader()
{
	m_materialManager = new PrRenderer::Resources::MaterialManager();
	m_meshManager = new PrRenderer::Resources::MeshManager();
	m_textureManager = new PrRenderer::Resources::TextureManager();
	m_shaderManager = new PrRenderer::Resources::ShaderManager();
}

ResourceLoader::~ResourceLoader()
{
	delete m_materialManager;
	delete m_meshManager;
	delete m_textureManager;
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

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Shader).hash_code())
		return std::static_pointer_cast<T>(m_shaderManager->Load(p_name));

	return std::shared_ptr<T>();
}

template std::shared_ptr<PrRenderer::Resources::Material> ResourceLoader::LoadResource<PrRenderer::Resources::Material>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Mesh> ResourceLoader::LoadResource<PrRenderer::Resources::Mesh>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Texture> ResourceLoader::LoadResource<PrRenderer::Resources::Texture>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Texture2D> ResourceLoader::LoadResource<PrRenderer::Resources::Texture2D>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Shader> ResourceLoader::LoadResource<PrRenderer::Resources::Shader>(const std::string& p_name);

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

	if (typeid(T).hash_code() == typeid(PrRenderer::Resources::Shader).hash_code())
		return std::static_pointer_cast<T>(m_shaderManager->GetResource(p_name));

	return std::shared_ptr<T>();
}

template std::shared_ptr<PrRenderer::Resources::Material> ResourceLoader::GetResource<PrRenderer::Resources::Material>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Mesh> ResourceLoader::GetResource<PrRenderer::Resources::Mesh>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Texture> ResourceLoader::GetResource<PrRenderer::Resources::Texture>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Texture2D> ResourceLoader::GetResource<PrRenderer::Resources::Texture2D>(const std::string& p_name);
template std::shared_ptr<PrRenderer::Resources::Shader> ResourceLoader::GetResource<PrRenderer::Resources::Shader>(const std::string& p_name);


