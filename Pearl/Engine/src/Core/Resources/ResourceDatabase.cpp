#include "Core/Common/pearl_pch.h"

#include "Core/Resources/ResourceDatabase.h"
#include "Core/Resources/IResourceDataLoader.h"
#include "Core/Utils/PathUtils.h"
#include "Core/Utils/StringUtils.h"

#include "Core/Events/EventManager.h"
#include "Core/Events/ResourceEvents.h"

#include <algorithm>

using namespace PrCore::Resources;

ResourceDatabase::~ResourceDatabase()
{
	RemoveAll();
	UnregisterAllLoaders();
}

ResourceDescPtr ResourceDatabase::Load(const std::string& p_path, std::shared_ptr<IResourceDataLoader> p_loader /*= nullptr*/)
{
	PR_ASSERT(!p_path.empty(), "Resource path is empty");
	auto lowerPath = PrCore::StringUtils::ToLower(p_path);

	auto resourceDesc = ResourceByPath(lowerPath);
	if (resourceDesc == nullptr)
		resourceDesc = RegisterFileResourcePrivate(lowerPath);
	
	if (resourceDesc->state == ResourceState::Unloaded || resourceDesc->state == ResourceState::Registered)
		LoadResourcePrivate(resourceDesc, p_loader);
	
	return resourceDesc;
}

ResourceDescPtr ResourceDatabase::Load(ResourceID p_id, std::shared_ptr<IResourceDataLoader> p_loader /*= nullptr*/)
{
	PR_ASSERT(p_id != InvalidID, "ResourceID is invalid.");

	auto resourceDesc = ResourceByID(p_id);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot load resource with ID {0}. ID does not exist.", p_id);
		return nullptr;
	}

	if (resourceDesc->origin != ResourceOrigin::File)
	{
		PRLOG_WARN("Cannot load resource with ID {0}. Only file resources can be loaded. Returning memory resource directly.", p_id);
		return resourceDesc;
	}

	if (resourceDesc->state == ResourceState::Corrupted)
		PRLOG_WARN("Cannot load resource with ID {0} and path \"{1}\".Resource is corrupted.", p_id, resourceDesc->filePath);

	if (resourceDesc->state == ResourceState::Unloaded || resourceDesc->state == ResourceState::Registered)
		LoadResourcePrivate(resourceDesc, p_loader);
	
	return resourceDesc;
}

void ResourceDatabase::Unload(const std::string& p_path)
{
	PR_ASSERT(!p_path.empty(), "Resource path is empty");
	auto lowerPath = PrCore::StringUtils::ToLower(p_path);

	auto resourceDesc = ResourceByPath(lowerPath);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot unload resource with path \"{0}\". Resource does not exist.", lowerPath);
		return;
	}

	if (resourceDesc->origin != ResourceOrigin::File)
	{
		PRLOG_WARN("Cannot unload resource with ID {0}, path \"{1}\". Only file resources can be loaded and unloaded. Use ResourceDatabase::Remove() to delete the memory resource.", resourceDesc->id, resourceDesc->filePath);
		return;
	}

	if (resourceDesc->state == ResourceState::Corrupted)
		PRLOG_WARN("Cannot unload resource with ID {0} path \"{1}\". Resource is corrupted.", resourceDesc->id, resourceDesc->filePath);

	if (resourceDesc->state == ResourceState::Loaded)
		UnloadResourcePrivate(resourceDesc);
}

void ResourceDatabase::Unload(ResourceID p_id)
{
	PR_ASSERT(p_id != InvalidID, "ResourceID is invalid.");

	auto resourceDesc = ResourceByID(p_id);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot unload resource with ID {0}. ID does not exist.", p_id);
		return;
	}

	if (resourceDesc->origin != ResourceOrigin::File)
	{
		PRLOG_WARN("Cannot unload resource with ID {0}. Only file resources can be loaded and unloaded, this is memory source. Use ResourceDatabase::Remove() to delete the memory resource.", p_id);
		return;
	}

	if (resourceDesc->state == ResourceState::Corrupted)
		PRLOG_WARN("Cannot unload resource with ID {0} path \"{1}\". Resource is corrupted.", p_id, resourceDesc->filePath);

	if (resourceDesc->state == ResourceState::Loaded)
		UnloadResourcePrivate(resourceDesc);
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

	if (resourceDesc->origin == ResourceOrigin::File && (resourceDesc->state == ResourceState::Unloaded || resourceDesc->state == ResourceState::Registered))
	{
		LoadResourcePrivate(resourceDesc);
		FireCacheMiss(resourceDesc->id, resourceDesc->filePath);
	}

	return resourceDesc;
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::Get(const std::string& p_path)
{
	PR_ASSERT(!p_path.empty(), "Resource path is empty");
	auto lowerPath = PrCore::StringUtils::ToLower(p_path);

	auto resourceDesc = ResourceByPath(lowerPath);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot load resource with path \"{0}\". Resource is not registered.", lowerPath);
		return nullptr;
	}

	if (resourceDesc->origin == ResourceOrigin::File && (resourceDesc->state == ResourceState::Unloaded || resourceDesc->state == ResourceState::Registered))
	{
		LoadResourcePrivate(resourceDesc);
		FireCacheMiss(resourceDesc->id, resourceDesc->filePath);
	}

	return resourceDesc;
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::Register(IResourceDataPtr p_resourceData)
{
	return RegisterMemoryResourcePrivate(p_resourceData);
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::Register(const std::string& p_path)
{
	PR_ASSERT(!p_path.empty(), "Resource path is empty");
	auto lowerPath = PrCore::StringUtils::ToLower(p_path);

	auto resourceDesc = ResourceByPath(lowerPath);
	if (resourceDesc)
	{
		PRLOG_WARN("Resource with path \"{0}\" already registered. Returning already registered", lowerPath);
		return resourceDesc;
	}

	return RegisterFileResourcePrivate(lowerPath);
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

	if (resourceDesc->origin == ResourceOrigin::File && resourceDesc->state == ResourceState::Loaded)
	{
		UnloadResourcePrivate(resourceDesc);
		FireUnloadedEvent(resourceDesc->id, resourceDesc->filePath);
	}

	if (resourceDesc->origin == ResourceOrigin::Memory)
		m_memoryUsage -= resourceDesc->size;

	m_resourcesID.erase(resourceDesc->id);
	if(resourceDesc->origin == ResourceOrigin::File && !resourceDesc->filePath.empty())
		m_resourcesPaths.erase(resourceDesc->filePath);

	// Remove custom loader for the removed resource
	if(auto customLoaderIt = m_customLoaders.find(resourceDesc->id); customLoaderIt != m_customLoaders.end())
		m_customLoaders.erase(customLoaderIt);

	resourceDesc->id = InvalidID;
	resourceDesc->data = nullptr;
	resourceDesc->size = 0;
	resourceDesc->state = ResourceState::Unmanaged;

	return true;
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::RegisterFileResourcePrivate(const std::string& p_path)
{
	PR_ASSERT(!p_path.empty(), "Resource path is empty");

	auto resourceDesc = std::make_shared<ResourceDesc>();
	resourceDesc->id = Utils::UUIDGenerator().Generate();
	resourceDesc->filePath = p_path;
	resourceDesc->state = ResourceState::Registered;
	resourceDesc->origin = ResourceOrigin::File;
	resourceDesc->data = nullptr;
	resourceDesc->size = 0;

	m_resourcesID.emplace(resourceDesc->id, resourceDesc);
	m_resourcesPaths.emplace(resourceDesc->filePath, resourceDesc);

	return resourceDesc;
}

PrCore::Resources::ResourceDescPtr PrCore::Resources::ResourceDatabase::RegisterMemoryResourcePrivate(const IResourceDataPtr& p_resourceData)
{
	PR_ASSERT(p_resourceData, "Resource data is nullptr");

	auto resourceDesc = std::make_shared<ResourceDesc>();
	resourceDesc->id = Utils::UUIDGenerator().Generate();
	resourceDesc->filePath = "";
	resourceDesc->state = ResourceState::Registered;
	resourceDesc->origin = ResourceOrigin::Memory;
	resourceDesc->data = p_resourceData;
	resourceDesc->size = p_resourceData->GetByteSize();

	m_memoryUsage += resourceDesc->size;
	CheckMemoryBudget(resourceDesc);

	m_resourcesID.emplace(resourceDesc->id, resourceDesc);

	return resourceDesc;
}

bool PrCore::Resources::ResourceDatabase::LoadResourcePrivate(const ResourceDescPtr& p_resourceDesc, std::shared_ptr<IResourceDataLoader> p_loader /*= nullptr*/)
{
	PR_ASSERT(p_resourceDesc, "Resource is nullptr");
	PR_ASSERT(!p_resourceDesc->filePath.empty(), "Resource path is empty");

	const std::string& path = p_resourceDesc->filePath;
	IResourceDataPtr resourceData = nullptr;
	
	// Check if custom loader was passed to use
	if (p_loader)
	{
		//Use custom loader and insert it into the map, replace if another custom loader was used previously. 
		resourceData = p_loader->LoadResource(path);
		m_customLoaders.insert_or_assign(p_resourceDesc->id, p_loader);
	}
	else
	{
		auto customLoader = m_customLoaders.find(p_resourceDesc->id);
		if (customLoader != m_customLoaders.end())
		{
			// Use a custom loader if assigned
			resourceData = customLoader->second->LoadResource(path);
		}
		else
		{
			// Use default loader for extension
			auto loaderIt = m_loaders.find(PathUtils::GetExtension(path));
			PR_ASSERT(loaderIt != m_loaders.end(), "Cannot load resource. Resource loader not registered.");

			if (loaderIt != m_loaders.end())
				resourceData = loaderIt->second->LoadResource(path);
		}
	}

	if (resourceData == nullptr)
	{
		PRLOG_WARN("Cannot load resource path \"{0}\" with ID {1}", path, p_resourceDesc->id);
		FireCorruptedEvent(p_resourceDesc->id, path);

		p_resourceDesc->data = nullptr;
		p_resourceDesc->size = 0;
		p_resourceDesc->state = ResourceState::Corrupted;
		p_resourceDesc->origin = ResourceOrigin::File;

		return false;
	}

	// Set resource data name for logging purpose
	resourceData->SetName(path);

	p_resourceDesc->data = resourceData;
	p_resourceDesc->size = resourceData->GetByteSize();
	p_resourceDesc->state = ResourceState::Loaded;
	p_resourceDesc->origin = ResourceOrigin::File;

	PRLOG_INFO("Loaded resource path \"{0}\" with UUID {1}", path, p_resourceDesc->id);
	FireLoadedEvent(p_resourceDesc->id, p_resourceDesc->filePath);
	
	m_memoryUsage += p_resourceDesc->size;
	CheckMemoryBudget(p_resourceDesc);

	return true;
}

PrCore::Resources::IResourceDataLoader* PrCore::Resources::ResourceDatabase::GetLoader(const std::string& p_fileExtension)
{
	PR_ASSERT(!p_fileExtension.empty(), "File extension is empty");
	auto lowerExt = PrCore::StringUtils::ToLower(p_fileExtension);

	auto it = m_loaders.find(lowerExt);
	if (it == m_loaders.end())
		return nullptr;

	// Return raw ptr, possible risk of deleting the ptr by the client.
	return it->second.get();
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::SaveToFileAndLoad(ResourceID p_sourceId, const std::string& p_path)
{
	PR_ASSERT(!p_path.empty(), "File path is empty");
	auto lowerPath = PrCore::StringUtils::ToLower(p_path);

	// Save to File
	bool success = SaveToFile(p_sourceId, lowerPath);

	if (!success)
	{
		PRLOG_WARN("Resource id {0} cannot be saved to file. Skipping loading.", p_sourceId);
		return nullptr;
	}

	auto customLoaderIt = m_customLoaders.find(p_sourceId);
	auto customLoader = customLoaderIt != m_customLoaders.end() ? customLoaderIt->second : nullptr;
	return Load(lowerPath, customLoader);
}

bool ResourceDatabase::SaveToFile(ResourceID p_sourceId, const std::string& p_path)
{
	PR_ASSERT(!p_path.empty(), "File path is empty");
	auto lowerPath = PrCore::StringUtils::ToLower(p_path);

	// Get Load if necessary
	auto resourceDesc = Get(p_sourceId);
	if (resourceDesc == nullptr)
		return false;

	// Get loader
	bool success = false;
	auto customLoaderIt = m_customLoaders.find(resourceDesc->id);
	if (customLoaderIt != m_customLoaders.end())
	{
		success = customLoaderIt->second->SaveResourceOnDisc(resourceDesc->data, lowerPath);
	}
	else
	{
		auto loaderIt = m_loaders.find(PathUtils::GetExtension(lowerPath));
		PR_ASSERT(loaderIt != m_loaders.end(), "Cannot unload resource. Resource loader not registered.");

		if (loaderIt != m_loaders.end())
			success = loaderIt->second->SaveResourceOnDisc(resourceDesc->data, lowerPath);
	}
	
	return success;
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

PrCore::Resources::ResourceDescPtr ResourceDatabase::GetMetadata(const std::string& p_path)
{
	PR_ASSERT(!p_path.empty(), "Resource path is empty");
	auto lowerPath = PrCore::StringUtils::ToLower(p_path);

	auto resourceDesc = ResourceByPath(lowerPath);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot unload resource with path \"{0}\". Resource is not registered.", lowerPath);
		return nullptr;
	}

	return resourceDesc;
}

PrCore::Resources::ResourceDescPtr ResourceDatabase::GetMetadata(ResourceID p_id)
{
	PR_ASSERT(p_id != InvalidID, "ResourceID is invalid.");

	auto resourceDesc = ResourceByID(p_id);
	if (resourceDesc == nullptr)
	{
		PRLOG_WARN("Cannot load resource with ID {0}. Resource is not registered.", p_id);
		return nullptr;
	}

	return resourceDesc;
}

void ResourceDatabase::CheckMemoryBudget(ResourceDescPtr p_resource)
{
	if (m_memoryUsage > m_memoryBudget)
		FireBudgetExceeded(p_resource->id, p_resource->filePath, m_memoryUsage, m_memoryBudget);
}

void ResourceDatabase::UnloadAll()
{
	for (auto& [_, resourceDesc] : m_resourcesPaths)
	{
		if (resourceDesc->origin == ResourceOrigin::File && resourceDesc->state == ResourceState::Loaded)
			UnloadResourcePrivate(resourceDesc);
	}
}

void ResourceDatabase::UnregisterLoader(const std::string& p_fileExtension)
{
	PR_ASSERT(!p_fileExtension.empty(), "File extension is empty");
	auto lowerExt = PrCore::StringUtils::ToLower(p_fileExtension);

	auto it = m_loaders.find(lowerExt);

	if (it == m_loaders.end())
	{
		PRLOG_WARN("Loader with extension \"{0}\" is not registered. Cannot unregister the loader.", lowerExt);
		return;
	}

	m_loaders.erase(it);
}

void ResourceDatabase::RegisterLoader(const std::string& p_fileExtension, std::unique_ptr<IResourceDataLoader> p_loader)
{
	PR_ASSERT(p_loader != nullptr, "Loader is nullptr");
	PR_ASSERT(!p_fileExtension.empty(), "File extension is empty");

	if (m_loaders.find(p_fileExtension) != m_loaders.end())
		PRLOG_WARN("Loader with extension \"{0}\" already registered. Replacing the loader.", p_fileExtension);

	m_loaders.emplace(p_fileExtension, std::move(p_loader));
}

void ResourceDatabase::UnregisterAllLoaders()
{
	m_loaders.clear();
}

void PrCore::Resources::ResourceDatabase::UnloadResourcePrivate(const ResourceDescPtr& p_resourceDesc)
{
	PR_ASSERT(p_resourceDesc, "Resource is nullptr");
	PR_ASSERT(!p_resourceDesc->filePath.empty(), "Resource path is empty");

	const std::string& path = p_resourceDesc->filePath;
	auto customLoaderIt = m_customLoaders.find(p_resourceDesc->id);
	if (customLoaderIt != m_customLoaders.end())
	{
		// Check if resource was loaded with custom loader
		customLoaderIt->second->UnloadResource(p_resourceDesc->data);
	}
	else
	{
		// Use default loader for extension
		auto loaderIt = m_loaders.find(PathUtils::GetExtension(path));
		PR_ASSERT(loaderIt != m_loaders.end(), "Cannot unload resource. Resource loader not registered.");

		if (loaderIt != m_loaders.end())
			loaderIt->second->UnloadResource(p_resourceDesc->data);
	}

	m_memoryUsage -= p_resourceDesc->size;

	p_resourceDesc->data = nullptr;
	p_resourceDesc->size = 0;
	p_resourceDesc->state = ResourceState::Unloaded;

	PRLOG_INFO("Unloaded resource path \"{0}\" with UUID {1}", path, p_resourceDesc->id);
	FireUnloadedEvent(p_resourceDesc->id, path);
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
	Events::EventPtr event =
		std::make_shared<Events::BudgetExceededv2>(p_id, p_path, p_usage, p_budget);
	Events::EventManager::GetInstance().QueueEvent(event);
}

void ResourceDatabase::FireCacheMiss(ResourceID p_id, const std::string& p_path)
{
	Events::EventPtr event =
		std::make_shared<Events::CacheMissEventv2>(p_id, p_path);
	Events::EventManager::GetInstance().QueueEvent(event);
}

void ResourceDatabase::FireCorruptedEvent(ResourceID p_id, const std::string& p_path)
{
	Events::EventPtr event =
		std::make_shared<Events::ResourceCorruptedEventv2>(p_id, p_path);
	Events::EventManager::GetInstance().QueueEvent(event);
}

void ResourceDatabase::FireLoadedEvent(ResourceID p_id, const std::string& p_path)
{
	Events::EventPtr event =
		std::make_shared<Events::ResourceLoadedEventv2>(p_id, p_path);
	Events::EventManager::GetInstance().QueueEvent(event);
}

void ResourceDatabase::FireUnloadedEvent(ResourceID p_id, const std::string& p_path)
{
	Events::EventPtr event =
		std::make_shared<Events::ResourceUnloadedEventv2>(p_id, p_path);
	Events::EventManager::GetInstance().QueueEvent(event);
}
