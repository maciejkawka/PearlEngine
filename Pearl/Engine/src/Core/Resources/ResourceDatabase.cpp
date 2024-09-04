#include "Core/Common/pearl_pch.h"

#include "Core/Resources/ResourceDatabase.h"
#include "Core/Resources/IResourceDataLoader.h"
#include "Core/Utils/PathUtils.h"

#include "Core/Events/EventManager.h"
#include "Core/Events/ResourceEvents.h"

#include <algorithm>

using namespace PrCore::Resources;

ResourceDescPtr ResourceDatabase::Load(const std::string& p_path, IResourceDataLoader* p_loader /*= nullptr*/)
{
	PR_ASSERT(!p_path.empty(), "Resource path is empty");

	auto resourceDesc = ResourceByPath(p_path);
	if (resourceDesc == nullptr)
		resourceDesc = RegisterFileResourcePrivate(p_path);
	
	if (resourceDesc->GetState() == ResourceState::Unloaded || resourceDesc->GetState() == ResourceState::Registered)
		LoadResourcePrivate(resourceDesc, p_loader);
	
	return resourceDesc;
}

ResourceDescPtr ResourceDatabase::Load(ResourceID p_id, IResourceDataLoader* p_loader /*= nullptr*/)
{
	PR_ASSERT(p_id != InvalidID, "ResourceID is invalid.");

	auto resourceDesc = ResourceByID(p_id);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot load resource with ID {0}. ID does not exist.", p_id);
		return nullptr;
	}

	if (resourceDesc->m_source != ResourceSource::FromFile)
	{
		PRLOG_WARN("Cannot load resource with ID {0}. Only file resources can be loaded. Returning memory resource directly.", p_id);
		return resourceDesc;
	}

	if (resourceDesc->m_state == ResourceState::Corrupted)
		PRLOG_WARN("Cannot load resource with ID {0} and path {1}. Resource is corrupted.", p_id, resourceDesc->m_path);

	if (resourceDesc->GetState() == ResourceState::Unloaded || resourceDesc->GetState() == ResourceState::Registered)
		LoadResourcePrivate(resourceDesc, p_loader);
	
	return resourceDesc;
}

void ResourceDatabase::Unload(const std::string& p_path, IResourceDataLoader* p_loader /*= nullptr*/)
{
	PR_ASSERT(!p_path.empty(), "Resource path is empty");

	auto resourceDesc = ResourceByPath(p_path);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot unload resource with path {0}. Resource does not exist.", p_path);
		return;
	}

	if (resourceDesc->m_source != ResourceSource::FromFile)
	{
		PRLOG_WARN("Cannot unload resource with ID {0}, path {1}. Only file resources can be loaded and unloaded. Use ResourceDatabase::Remove() to delete the memory resource.", p_id, p_path);
		return;
	}

	if (resourceDesc->m_state == ResourceState::Corrupted)
		PRLOG_WARN("Cannot unload resource with ID {0} path {1}. Resource is corrupted.", p_id, resourceDesc->m_path);

	if (resourceDesc->GetState() == ResourceState::Loaded)
		UnloadResourcePrivate(resourceDesc, p_loader);
}

void ResourceDatabase::Unload(ResourceID p_id, IResourceDataLoader* p_loader /*= nullptr*/)
{
	PR_ASSERT(p_id != InvalidID, "ResourceID is invalid.");

	auto resourceDesc = ResourceByID(p_id);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot unload resource with ID {0}. ID does not exist.", p_id);
		return;
	}

	if (resourceDesc->m_source != ResourceSource::FromFile)
	{
		PRLOG_WARN("Cannot unload resource with ID {0}. Only file resources can be loaded and unloaded, this is memory source. Use ResourceDatabase::Remove() to delete the memory resource.", p_id);
		return;
	}

	if (resourceDesc->m_state == ResourceState::Corrupted)
		PRLOG_WARN("Cannot unload resource with ID {0} path {1}. Resource is corrupted.", p_id, resourceDesc->m_path);

	if (resourceDesc->GetState() == ResourceState::Loaded)
		UnloadResourcePrivate(resourceDesc, p_loader);
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::Get(ResourceID p_id)
{
	PR_ASSERT(p_id != InvalidID, "ResourceID is invalid.");

	auto resourceDesc = ResourceByID(p_id);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot load resource with ID {0}. Resource is not registered.", p_id);
		return nullptr;
	}

	if (resourceDesc->m_source == ResourceSource::FromFile && (resourceDesc->m_state == ResourceState::Unloaded || resourceDesc->m_state == ResourceState::Registered))
	{
		LoadResourcePrivate(resourceDesc);
		FireCacheMiss(resourceDesc->m_ID, resourceDesc->m_path);
	}

	return resourceDesc;
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::Get(const std::string& p_path)
{
	PR_ASSERT(!p_path.empty(), "Resource path is empty");

	auto resourceDesc = ResourceByPath(p_path);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot load resource with path {0}. Resource is not registered.", p_path);
		return nullptr;
	}

	if (resourceDesc->m_source == ResourceSource::FromFile && (resourceDesc->m_state == ResourceState::Unloaded || resourceDesc->m_state == ResourceState::Registered))
	{
		LoadResourcePrivate(resourceDesc);
		FireCacheMiss(resourceDesc->m_ID, resourceDesc->m_path);
	}

	return resourceDesc;
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::Register(IResourceDataPtr p_resourceData, size_t p_size /*= 0*/)
{
	return RegisterMemoryResourcePrivate(p_resourceData, p_size);
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::Register(const std::string& p_path)
{
	return RegisterFileResourcePrivate(p_path);
}


bool ResourceDatabase::Remove(ResourceID p_id)
{
	PR_ASSERT(p_id != InvalidID, "ResourceID is invalid.");

	auto resourceDesc = ResourceByID(p_id);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot remove resource with ID {0}. Resource is not registered.", p_id);
		return false;
	}

	if (resourceDesc->m_source == ResourceSource::FromFile && resourceDesc->m_state == ResourceState::Loaded)
	{
		UnloadResourcePrivate(resourceDesc);
		FireUnloadedEvent(resourceDesc->m_ID, resourceDesc->m_path);
	}

	if (resourceDesc->m_source == ResourceSource::FromMemory)
		m_memoryUsage -= resourceDesc->m_size;

	m_resourcesID.erase(resourceDesc->m_ID);
	if(resourceDesc->m_source == ResourceSource::FromFile && !resourceDesc->m_path.empty())
		m_resourcesPaths.erase(resourceDesc->m_path);

	resourceDesc->m_ID = InvalidID;
	resourceDesc->m_data = nullptr;
	resourceDesc->m_size = 0;
	resourceDesc->m_state = ResourceState::Unmanaged;

	return true;
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::RegisterFileResourcePrivate(const std::string& p_path)
{
	PR_ASSERT(!p_path.empty(), "Resource path is empty");

	auto resourceDesc = std::make_shared<ResourceDesc>();
	resourceDesc->m_ID = Utils::UUIDGenerator().Generate();
	resourceDesc->m_path = p_path;
	resourceDesc->m_state = ResourceState::Registered;
	resourceDesc->m_source = ResourceSource::FromFile;
	resourceDesc->m_data = nullptr;
	resourceDesc->m_size = 0;

	m_resourcesID.emplace(resourceDesc->GetID(), resourceDesc);
	m_resourcesPaths.emplace(resourceDesc->GetPath(), resourceDesc);

	return resourceDesc;
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::RegisterMemoryResourcePrivate(IResourceDataPtr p_resourceData, size_t p_size)
{
	PR_ASSERT(p_resourceData, "Resource data is nullptr");

	auto resourceDesc = std::make_shared<ResourceDesc>();
	resourceDesc->m_ID = Utils::UUIDGenerator().Generate();
	resourceDesc->m_path = "";
	resourceDesc->m_state = ResourceState::Registered;
	resourceDesc->m_source = ResourceSource::FromMemory;
	resourceDesc->m_data = p_resourceData;
	resourceDesc->m_size = p_size;

	m_memoryUsage += p_size;
	CheckMemoryBudget(resourceDesc);

	m_resourcesID.emplace(resourceDesc->GetID(), resourceDesc);

	return resourceDesc;
}

bool PrCore::Resources::ResourceDatabase::LoadResourcePrivate(ResourceDescPtr p_resourceDesc, IResourceDataLoader* p_loader /*= nullptr*/)
{
	PR_ASSERT(p_resourceDesc, "Resource is nullptr");
	PR_ASSERT(!p_resourceDesc->m_path.empty(), "Resource path is empty");

	const std::string& path = p_resourceDesc->m_path;
	IResourceDataPtr resourceData = nullptr;
	IResourceDataLoader* loader = nullptr;
	size_t dataSize = 0;
	
	// Choose correct loader
	if (p_loader)
	{
		loader = p_loader;
		m_customLoaders.emplace(p_resourceDesc->m_ID, loader);
	}
	else
	{
		auto loaderIt = m_loaders.find(PathUtils::GetExtension(path));
		PR_ASSERT(loaderIt != m_loaders.end(), "Cannot load resource. Resource loader not registered.");

		if (loaderIt != m_loaders.end())
			loader = loaderIt->second;
	}

	// Load resource
	resourceData = loader->LoadResource(path, dataSize);

	if (resourceData == nullptr)
	{
		PRLOG_WARN("Cannot load resource path: {0} with ID {1}", path, path, p_resourceDesc->GetID());
		FireCorruptedEvent(p_resourceDesc->m_ID, path);

		p_resourceDesc->m_data = nullptr;
		p_resourceDesc->m_size = 0;
		p_resourceDesc->m_state = ResourceState::Corrupted;
		p_resourceDesc->m_source = ResourceSource::FromFile;

		return false;
	}

	p_resourceDesc->m_data = resourceData;
	p_resourceDesc->m_size = dataSize;
	p_resourceDesc->m_state = ResourceState::Loaded;
	p_resourceDesc->m_source = ResourceSource::FromFile;

	PRLOG_INFO("Loaded resource path: {0} with UUID {1}", path, p_resourceDesc->GetID());
	FireLoadedEvent(p_resourceDesc->m_ID, p_resourceDesc->m_path);
	
	m_memoryUsage += dataSize;
	CheckMemoryBudget(p_resourceDesc);

	return true;
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::SaveToFileAndLoad(ResourceID p_sourceId, const std::string& p_path)
{
	// Save to File
	bool success = SaveToFile(p_sourceId, p_path);

	if (!success)
	{
		PRLOG_WARN("Resource id {0} cannot be saved to file. Skipping loading.", p_sourceId);
		return nullptr;
	}

	auto customLoaderIt = m_customLoaders.find(p_sourceId);
	auto customLoader = customLoaderIt != m_customLoaders.end() ? customLoaderIt->second : nullptr;
	return Load(p_path, customLoader);
}

bool ResourceDatabase::SaveToFile(ResourceID p_sourceId, const std::string& p_path)
{
	// Get Load if necessary
	auto resourceDesc = Get(p_sourceId);
	if (resourceDesc == nullptr)
		return false;

	// Get loader
	IResourceDataLoader* loader = nullptr;
	auto customLoaderIt = m_customLoaders.find(resourceDesc->m_ID);
	if (customLoaderIt != m_customLoaders.end())
	{
		loader = customLoaderIt->second;
	}
	else
	{
		auto loaderIt = m_loaders.find(PathUtils::GetExtension(p_path));
		PR_ASSERT(loaderIt != m_loaders.end(), "Cannot unload resource. Resource loader not registered.");

		if (loaderIt != m_loaders.end())
			loader = loaderIt->second;
	}

	// Save to file
	return loader->SaveResourceOnDisc(resourceDesc->m_data, p_path);
}

void ResourceDatabase::ForEachResource(ResourceVisitor p_visitor)
{
	for (auto [_, resourceDesc] : m_resourcesID)
		p_visitor(resourceDesc);
}

void ResourceDatabase::RemoveAll()
{
	std::vector<ResourceID> tempIDs;
	tempIDs.reserve(m_resourcesID.size());

	for (auto& [id, _] : m_resourcesID)
		tempIDs.push_back(id);
	
	for (auto id : tempIDs)
		Remove(id);
}

PrCore::Resources::ResourceDesc ResourceDatabase::GetMetadata(const std::string& p_path)
{
	PR_ASSERT(!p_path.empty(), "Resource path is empty");

	auto resourceDesc = ResourceByPath(p_path);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot unload resource with path {0}. Resource is not registered.", p_path);
		return ResourceDesc();
	}

	// Pass only metadata
	auto metadata = *resourceDesc;
	metadata.m_data = nullptr;
	return metadata;
}

PrCore::Resources::ResourceDesc ResourceDatabase::GetMetadata(ResourceID p_id)
{
	PR_ASSERT(p_id != InvalidID, "ResourceID is invalid.");

	auto resourceDesc = ResourceByID(p_id);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot load resource with ID {0}. Resource is not registered.", p_id);
		return ResourceDesc();
	}

	// Pass only metadata
	auto metadata = *resourceDesc;
	metadata.m_data = nullptr;
	return metadata;
}

void ResourceDatabase::CheckMemoryBudget(ResourceDescPtr p_resource)
{
	if (m_memoryUsage > m_memoryBudget)
		FireBudgetExceeded(p_resource->m_ID, p_resource->m_path, m_memoryUsage, m_memoryBudget);
}

void ResourceDatabase::UnloadAll()
{
	for (auto& [_, resourceDesc] : m_resourcesPaths)
	{
		if (resourceDesc->GetSource() == ResourceSource::FromFile && resourceDesc->GetState() == ResourceState::Loaded)
			UnloadResourcePrivate(resourceDesc);
	}
}

void ResourceDatabase::UnregisterLoader(const std::string& p_fileExtension)
{
	PR_ASSERT(!p_fileExtension.empty(), "File extension is empty");

	auto it = m_loaders.find(p_fileExtension);

	if (it == m_loaders.end())
	{
		PRLOG_WARN("Loader with extension {0} is not registered. Cannot unregister the loader.", p_fileExtension);
		return;
	}

	m_loaders.erase(it);
}

void ResourceDatabase::RegisterLoader(const std::string& p_fileExtension, IResourceDataLoader* p_loader)
{
	PR_ASSERT(p_loader != nullptr, "Loader is nullptr");
	PR_ASSERT(!p_fileExtension.empty(), "File extension is empty");

	if (m_loaders.find(p_fileExtension) != m_loaders.end())
		PRLOG_WARN("Loader with extension {0} already registered. Replacing the loader.", p_fileExtension);

	m_loaders.emplace(p_fileExtension, p_loader);
}

void ResourceDatabase::UnregisterAllLoaders()
{
	m_loaders.clear();
}

void ResourceDatabase::UnloadResourcePrivate(ResourceDescPtr p_resourceDesc, IResourceDataLoader* p_loader /*= nullptr*/)
{
	PR_ASSERT(p_resourceDesc, "Resource is nullptr");
	PR_ASSERT(!p_resourceDesc->m_path.empty(), "Resource path is empty");

	const std::string& path = p_resourceDesc->m_path;
	IResourceDataLoader* loader = nullptr;
	if (p_loader)
		loader = p_loader;
	else
	{
		auto customLoaderIt = m_customLoaders.find(p_resourceDesc->m_ID);
		if (customLoaderIt != m_customLoaders.end())
		{
			loader = customLoaderIt->second;
			m_customLoaders.erase(customLoaderIt);
		}
		else
		{
			auto loaderIt = m_loaders.find(PathUtils::GetExtension(path));
			PR_ASSERT(loaderIt != m_loaders.end(), "Cannot unload resource. Resource loader not registered.");

			if (loaderIt != m_loaders.end())
				loader = loaderIt->second;
		}
	}

	loader->UnloadResource(p_resourceDesc->m_data);
	size_t dataSize = p_resourceDesc->m_size;

	p_resourceDesc->m_data = nullptr;
	p_resourceDesc->m_size = 0;
	p_resourceDesc->m_state = ResourceState::Unloaded;

	PRLOG_INFO("Unloaded resource path: {0} with UUID {1}", path, p_resourceDesc->GetID());
	FireUnloadedEvent(p_resourceDesc->GetID(), path);

	m_memoryUsage -= dataSize;
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::ResourceByID(ResourceID p_id)
{
	auto find = m_resourcesID.find(p_id);

	if (find != m_resourcesID.end())
		return find->second;

	return nullptr;
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::ResourceByPath(const std::string& p_path)
{
	auto find = m_resourcesPaths.find(p_path);

	if (find != m_resourcesPaths.end())
		return find->second;

	return nullptr;
}

void ResourceDatabase::FireBudgetExceeded(ResourceID p_id, const std::string& p_path, size_t p_usage, size_t p_budget)
{
	//Events::EventPtr event =
	//	std::make_shared<Events::BudgetExceededv2>(p_id, p_path, p_usage, p_budget);
	//Events::EventManager::GetInstance().QueueEvent(event);
}

void ResourceDatabase::FireCacheMiss(ResourceID p_id, const std::string& p_path)
{
	//Events::EventPtr event =
	//	std::make_shared<Events::CacheMissEventv2>(p_id, p_path);
	//Events::EventManager::GetInstance().QueueEvent(event);
}

void ResourceDatabase::FireCorruptedEvent(ResourceID p_id, const std::string& p_path)
{
	//Events::EventPtr event =
	//	std::make_shared<Events::ResourceCorruptedEventv2>(p_id, p_path);
	//Events::EventManager::GetInstance().QueueEvent(event);
}

void ResourceDatabase::FireLoadedEvent(ResourceID p_id, const std::string& p_path)
{
	//Events::EventPtr event =
	//	std::make_shared<Events::ResourceLoadedEventv2>(p_id, p_path);
	//Events::EventManager::GetInstance().QueueEvent(event);
}

void ResourceDatabase::FireUnloadedEvent(ResourceID p_id, const std::string& p_path)
{
	//Events::EventPtr event =
	//	std::make_shared<Events::ResourceUnloadedEventv2>(p_id, p_path);
	//Events::EventManager::GetInstance().QueueEvent(event);
}
