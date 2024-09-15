#include "Core/Utils/Assert.h"
#include "Core/Resources/IResource.h"

namespace PrCore::Resources {

	template<class T>
	ResourceHandle<T> ResourceSystem::Load(const std::string& p_path, std::shared_ptr<IResourceDataLoader> p_loader /*= nullptr*/)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto resourceDesc = GetResourceDatabase<T>()->Load(p_path, p_loader);
		return ResourceHandle<T>(resourceDesc);
	}
	
	template<class T>
	ResourceHandle<T> ResourceSystem::Load(ResourceID p_id, std::shared_ptr<IResourceDataLoader> p_loader /*= nullptr*/)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto resourceDesc = GetResourceDatabase<T>()->Load(p_id, p_loader);
		return ResourceHandle<T>(resourceDesc);
	}

	template<class T>
	void ResourceSystem::Unload(const std::string& p_path)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		GetResourceDatabase<T>()->Unload(p_path);
	}

	template<class T>
	void ResourceSystem::Unload(ResourceID p_id)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		GetResourceDatabase<T>()->Unload(p_id);
	}

	template<class T>
	void ResourceSystem::UnloadAll()
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		GetResourceDatabase<T>()->UnloadAll();
	}

	template<class T>
	ResourceHandle<T> ResourceSystem::Get(ResourceID p_id)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto resourceDesc = GetResourceDatabase<T>()->Get(p_id);
		return ResourceHandle<T>(resourceDesc);
	}	
	
	template<class T>
	ResourceHandle<T> ResourceSystem::Get(const std::string& p_path)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto resourceDesc = GetResourceDatabase<T>()->Get(p_path);
		return ResourceHandle<T>(resourceDesc);
	}

	template<class T>
	ResourceHandle<T> ResourceSystem::GetMetadata(ResourceID p_id)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto resourceDesc = GetResourceDatabase<T>()->GetMetadata(p_id);
		return ResourceHandle<T>(resourceDesc);
	}

	template<class T>
	ResourceHandle<T> ResourceSystem::GetMetadata(const std::string& p_path)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto resourceDesc = GetResourceDatabase<T>()->GetMetadata(p_path);
		return ResourceHandle<T>(resourceDesc);
	}

	template<class T>
	ResourceHandle<T> ResourceSystem::Register(IResourceDataPtr p_resourceData)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto resourceDesc = GetResourceDatabase<T>()->Register(p_resourceData);
		return ResourceHandle<T>(resourceDesc);
	}

	template<class T>
	ResourceHandle<T> ResourceSystem::Register(const std::string& p_path)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto resourceDesc = GetResourceDatabase<T>()->Register(p_path);
		return ResourceHandle<T>(resourceDesc);
	}

	template<class T>
	bool ResourceSystem::Remove(ResourceID p_id)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		return GetResourceDatabase<T>()->Remove(p_id);
	}

	template<class T>
	bool ResourceSystem::Remove(const std::string& p_path)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		ResourceID id = GetResourceDatabase<T>()->GetMetadata(p_path)->id;
		return GetResourceDatabase<T>()->Remove(id);
	}

	template<class T>
	void ResourceSystem::RemoveAll()
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		GetResourceDatabase<T>()->RemoveAll();
	}

	template<class T>
	void ResourceSystem::ForEachResource(ResourceVisitor p_visitor)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		GetResourceDatabase<T>()->ForEachResource(p_visitor);
	}

	template<class T>
	bool ResourceSystem::SaveToFile(ResourceID p_sourceId, const std::string& p_path)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		return GetResourceDatabase<T>()->SaveToFile(p_sourceId, p_path);
	}

	template<class T>
	ResourceHandle<T> ResourceSystem::SaveToFileAndLoad(ResourceID p_sourceId, const std::string& p_path)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto resourceDesc = GetResourceDatabase<T>()->SaveToFileAndLoad(p_sourceId, p_path);
		return ResourceHandle<T>(resourceDesc);
	}

	template<class T>
	std::shared_ptr<T> ResourceSystem::Copy(ResourceID p_sourceId)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto resourceData = GetResourceDatabase<T>()->Get(p_sourceId)->data;
		return std::make_shared<T>(*std::static_pointer_cast<T>(resourceData));
	}

	template<class T>
	std::shared_ptr<T> ResourceSystem::Copy(const std::string& p_path)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto resourceData = GetResourceDatabase<T>()->Get(p_path)->data;
		return std::make_shared<T>(*std::static_pointer_cast<T>(resourceData));
	}

	template<class T>
	size_t ResourceSystem::GetMemoryUsage() const
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		return GetResourceDatabase<T>()->GetMemoryUsage();
	}

	template<class T>
	void ResourceSystem::SetMemoryBudget(size_t p_budget)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		GetResourceDatabase<T>()->SetMemoryBudget(p_budget);
	}

	template<class T>
	size_t ResourceSystem::GetMemoryBudget() const
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		return GetResourceDatabase<T>()->GetMemoryBudget();
	}

	template<class T>
	void ResourceSystem::RegisterLoader(const std::string& p_fileExtension, IResourceDataLoader* p_loader)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		GetResourceDatabase<T>()->RegisterLoader(p_fileExtension, p_loader);
	}

	template<class T>
	void ResourceSystem::UnregisterLoader(const std::string& p_fileExtension)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		GetResourceDatabase<T>()->UnregisterLoader(p_fileExtension);
	}

	template<class T>
	void ResourceSystem::UnregisterAllLoaders()
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		GetResourceDatabase<T>()->UnregisterAllLoaders();
	}

	template<class T>
	std::unique_ptr<IResourceDatabase>& ResourceSystem::GetResourceDatabase()
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto database = m_resourceDatabaseTypes.find(typeid(T).hash_code());
		PR_ASSERT(database != m_resourceDatabaseTypes.end(), "Resource database is not registered. Resource Type: " + std::string(typeid(T).name()));
		
		return database->second;
	}

	template<class T>
	void ResourceSystem::RegisterDatabase(std::unique_ptr<IResourceDatabase> p_database)
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto databaseIt = m_resourceDatabaseTypes.find(typeid(T).hash_code());
		PR_ASSERT(databaseIt == m_resourceDatabaseTypes.end(), "Database already registered. Resource Type: " + std::string(typeid(T).name()));

		m_resourceDatabaseTypes.insert({ typeid(T).hash_code(), std::move(p_database) });
	}

	template<class T>
	void ResourceSystem::UnregisterDatabase()
	{
		static_assert(std::is_base_of<IResourceData, T>::value, "T has to be base of IResourceData.");
		auto databaseIt = m_resourceDatabaseTypes.find(typeid(T).hash_code());
		PR_ASSERT(databaseIt != m_resourceDatabaseTypes.end(), "Database is not registered. Resource Type: " + std::string(typeid(T).name()));

		m_resourceDatabaseTypes.erase(databaseIt);
	}
}