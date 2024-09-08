#include "Core/Utils/Assert.h"

namespace PrCore::Resources {

	template<class T>
	Resourcev2<T> ResourceSystem::Load(const std::string& p_path, std::shared_ptr<IResourceDataLoader> p_loader /*= nullptr*/)
	{
		auto resourceDesc = GetResourceDatabase<T>()->Load(p_path, p_loader);
		return Resourcev2<T>(resourceDesc);
	}
	
	template<class T>
	Resourcev2<T> ResourceSystem::Load(ResourceID p_id, std::shared_ptr<IResourceDataLoader> p_loader /*= nullptr*/)
	{
		auto resourceDesc = GetResourceDatabase<T>()->Load(p_id, p_loader);
		return Resourcev2<T>(resourceDesc);
	}

	template<class T>
	void ResourceSystem::Unload(const std::string& p_path)
	{
		GetResourceDatabase<T>()->Unload(p_path);
	}

	template<class T>
	void ResourceSystem::Unload(ResourceID p_id)
	{
		GetResourceDatabase<T>()->Unload(p_id);
	}

	template<class T>
	void ResourceSystem::UnloadAll()
	{
		GetResourceDatabase<T>()->UnloadAll();
	}

	template<class T>
	Resourcev2<T> ResourceSystem::Get(ResourceID p_id)
	{
		auto resourceDesc = GetResourceDatabase<T>()->Get(p_id);
		return Resourcev2<T>(resourceDesc);
	}	
	
	template<class T>
	Resourcev2<T> ResourceSystem::Get(const std::string& p_path)
	{
		auto resourceDesc = GetResourceDatabase<T>()->Get(p_path);
		return Resourcev2<T>(resourceDesc);
	}

	template<class T>
	Resourcev2<T> ResourceSystem::GetMetadata(ResourceID p_id)
	{
		auto resourceDesc = GetResourceDatabase<T>()->GetMetadata(p_id);
		return Resourcev2<T>(resourceDesc);
	}

	template<class T>
	Resourcev2<T> ResourceSystem::GetMetadata(const std::string& p_path)
	{
		auto resourceDesc = GetResourceDatabase<T>()->GetMetadata(p_path);
		return Resourcev2<T>(resourceDesc);
	}

	template<class T>
	Resourcev2<T> ResourceSystem::Register(IResourceDataPtr p_resourceData)
	{
		auto resourceDesc = GetResourceDatabase<T>()->Register(p_resourceData);
		return Resourcev2<T>(resourceDesc);
	}

	template<class T>
	Resourcev2<T> ResourceSystem::Register(const std::string& p_path)
	{
		auto resourceDesc = GetResourceDatabase<T>()->Register(p_path);
		return Resourcev2<T>(resourceDesc);
	}

	template<class T>
	bool ResourceSystem::Remove(ResourceID p_id)
	{
		 return GetResourceDatabase<T>()->Remove(p_id);
	}

	template<class T>
	void ResourceSystem::RemoveAll()
	{
		GetResourceDatabase<T>()->RemoveAll();
	}

	template<class T>
	void ResourceSystem::ForEachResource(ResourceVisitor p_visitor)
	{
		GetResourceDatabase<T>()->ForEachResource(p_visitor);
	}

	template<class T>
	bool ResourceSystem::SaveToFile(ResourceID p_sourceId, const std::string& p_path)
	{
		return GetResourceDatabase<T>()->SaveToFile(p_sourceId, p_path);
	}

	template<class T>
	Resourcev2<T> ResourceSystem::SaveToFileAndLoad(ResourceID p_sourceId, const std::string& p_path)
	{
		auto resourceDesc = GetResourceDatabase<T>()->SaveToFileAndLoad(p_sourceId, p_path);
		return Resourcev2<T>(resourceDesc);
	}

	template<class T>
	std::shared_ptr<T> ResourceSystem::Copy(ResourceID p_sourceId)
	{
		auto resourceData = GetResourceDatabase<T>()->Get(p_sourceId)->data;
		return std::make_shared<T>(*std::static_pointer_cast<T>(resourceData));
	}

	template<class T>
	std::shared_ptr<T> ResourceSystem::Copy(const std::string& p_path)
	{
		auto resourceData = GetResourceDatabase<T>()->Get(p_path)->data;
		return std::make_shared<T>(*std::static_pointer_cast<T>(resourceData));
	}

	template<class T>
	size_t ResourceSystem::GetMemoryUsage() const
	{
		return GetResourceDatabase<T>()->GetMemoryUsage();
	}

	template<class T>
	void ResourceSystem::SetMemoryBudget(size_t p_budget)
	{
		GetResourceDatabase<T>()->SetMemoryBudget(p_budget);
	}

	template<class T>
	size_t ResourceSystem::GetMemoryBudget() const
	{
		return GetResourceDatabase<T>()->GetMemoryBudget();
	}

	template<class T>
	void ResourceSystem::RegisterLoader(const std::string& p_fileExtension, IResourceDataLoader* p_loader)
	{
		GetResourceDatabase<T>()->RegisterLoader(p_fileExtension, p_loader);
	}

	template<class T>
	void ResourceSystem::UnregisterLoader(const std::string& p_fileExtension)
	{
		GetResourceDatabase<T>()->UnregisterLoader(p_fileExtension);
	}

	template<class T>
	void ResourceSystem::UnregisterAllLoaders()
	{
		GetResourceDatabase<T>()->UnregisterAllLoaders();
	}

	template<class T>
	std::unique_ptr<IResourceDatabase>& ResourceSystem::GetResourceDatabase()
	{
		auto database = m_resourceDatabaseTypes.find(typeid(T).hash_code());
		PR_ASSERT(database != m_resourceDatabaseTypes.end(), "Resource database is not registered.");
		
		return database->second;
	}

	template<class T>
	void ResourceSystem::RegisterDatabase(std::unique_ptr<IResourceDatabase> p_database)
	{
		auto databaseIt = m_resourceDatabaseTypes.find(typeid(T).hash_code());
		PR_ASSERT(databaseIt != m_resourceDatabaseTypes.end(), "Database already registered.");

		m_resourceDatabaseTypes.insert({ typeid(T).hash_code(), std::move(p_database) });
	}

	template<class T>
	void ResourceSystem::UnregisterDatabase()
	{
		auto databaseIt = m_resourceDatabaseTypes.find(typeid(T).hash_code());
		PR_ASSERT(databaseIt == m_resourceDatabaseTypes.end(), "Database is not registered.");

		m_resourceDatabaseTypes.erase(databaseIt);
	}
}