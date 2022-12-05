#include"Core/Common/pearl_pch.h"

#include"Core/Resources/ResourceManager.h"
#include"Core/Events/EventManager.h"
#include"Core/Events/ResourceEvents.h"
#include"Core/Utils/Assert.h"

using namespace PrCore::Resources;

unsigned int ResourceManager::s_nextResourceID = 0;

ResourceManager::ResourceManager() :
	m_memoryUsage(0)
{
	m_maxMemory = UINT_MAX;

	using namespace Events;
	EventListener loadedListener;
	loadedListener.connect<&ResourceManager::OnResourceLoaded>();
	EventManager::GetInstance().AddListener(loadedListener, ResourceLoadedEvent::s_type);

	EventListener unloadedListener;
	unloadedListener.connect<&ResourceManager::OnResourceUnloaded>();
	EventManager::GetInstance().AddListener(unloadedListener, ResourceUnloadedEvent::s_type);

	EventListener corruptedListener;
	corruptedListener.connect<&ResourceManager::OnResourceCorrupted>();
	EventManager::GetInstance().AddListener(corruptedListener, ResourceCorruptedEvent::s_type);
}

ResourceManager::~ResourceManager()
{
	DeleteAll();
}

ResourcePtr ResourceManager::CreateResource(const std::string& p_name)
{
	PR_ASSERT(!p_name.empty(), "Resource name is empty");

	auto findResource = m_resourceName.find(p_name);
	if (m_resourceName.find(p_name) != m_resourceName.end())
		return findResource->second;

	auto resource = ResourcePtr(CreateImpl(p_name));

	m_resourceID.emplace(resource->GetHandle(), resource);
	m_resourceName.emplace(p_name, resource);
	m_resources.push_back(resource);

	return resource;
}

void ResourceManager::DeleteImpl(ResourcePtr& p_resource)
{
	p_resource.reset();
}

void ResourceManager::FireCacheMiss(CacheMiss p_cacheMiss)
{
	Events::EventPtr event = std::make_shared<Events::CacheMissEvent>(p_cacheMiss);
	Events::EventManager::GetInstance().QueueEvent(event);
}

ResourcePtr ResourceManager::Load(const std::string& p_name)
{
	PR_ASSERT(!p_name.empty(), "Resource name is empty");

	ResourcePtr resource = ResourceByName(p_name);

	if (!resource)
		resource = CreateResource(p_name);

	if (resource->IsLoaded())
		return resource;

	resource->Load();
	UpdateMemoryUsage();

	m_LRU.UpdateLRU(resource->GetHandle());
	MemoryCheck();

	return resource;
}

ResourcePtr ResourceManager::Reload(const std::string& p_name)
{
	Unload(p_name);
	return Load(p_name);
}

void ResourceManager::Unload(const std::string& p_name)
{
	PR_ASSERT(!p_name.empty(), "Resource name is empty");

	ResourcePtr resource = ResourceByName(p_name);
	if (resource != nullptr && resource->IsLoaded())
	{
		resource->Unload();
		UpdateMemoryUsage();
		m_LRU.RemoveResource(resource->GetHandle());
	}
}

void ResourceManager::UnloadAll()
{
	for (ResourcePtr resource : m_resources)
	{
		if (resource->IsLoaded())
		{
			resource->Unload();
		}
	}

	UpdateMemoryUsage();
	m_LRU.Clear();
}

void ResourceManager::Delete(const std::string& p_name)
{
	PR_ASSERT(!p_name.empty(), "Resource name is empty");

	ResourcePtr resource = ResourceByName(p_name);
	if (resource == nullptr)
		return;

	if (resource->IsLoaded())
		Unload(p_name);

	m_resourceID.erase(resource->GetHandle());
	m_resourceName.erase(p_name);
	m_resources.remove(resource);

	DeleteImpl(resource);
}

void ResourceManager::DeleteAll()
{
	UnloadAll();

	for (ResourcePtr resource : m_resources)
		DeleteImpl(resource);

	m_resourceID.clear();
	m_resourceName.clear();
	m_resources.clear();
	m_LRU.Clear();
}

void ResourceManager::SetMaxMemory(size_t p_maxMemory)
{
	m_maxMemory = p_maxMemory;
}

ResourcePtr ResourceManager::GetResource(const std::string& p_name)
{
	auto findResource = m_resourceName.find(p_name);

	ResourcePtr resource;
	if (findResource == m_resourceName.end())
		resource = CreateResource(p_name);
	else
		resource = findResource->second;

	if (resource->IsUnloaded())
	{
		FireCacheMiss(CacheMiss::Compulsory);
		Load(p_name);
	}

	if (resource->IsCorrupted())
	{
		Unload(resource->GetName());
		return ResourcePtr();
	}


	m_LRU.UpdateLRU(resource->GetHandle());
	return resource;
}

void ResourceManager::OnResourceLoaded(Events::EventPtr p_event)
{
	auto event = std::static_pointer_cast<Events::ResourceLoadedEvent>(p_event);
	PRLOG_INFO("Resource handle {0} Name {1} loaded", event->m_ID, event->m_name);
}

void ResourceManager::OnResourceUnloaded(Events::EventPtr p_event)
{
	auto event = std::static_pointer_cast<Events::ResourceUnloadedEvent>(p_event);
	PRLOG_INFO("Resource handle {0} Name {1} unloaded", event->m_ID, event->m_name);
}

void ResourceManager::OnResourceCorrupted(Events::EventPtr p_event)
{
	auto event = std::static_pointer_cast<PrCore::Events::ResourceCorruptedEvent>(p_event);
	PRLOG_ERROR("Resource handle {0} Name {1} corrupted", event->m_ID, event->m_name);
}

ResourceHandle ResourceManager::ResNameToID(const std::string& p_name)
{
	return ResourceByName(p_name)->GetHandle();
}

std::string ResourceManager::ResIDToName(ResourceHandle p_ID)
{
	return ResourceByID(p_ID)->GetName();
}

ResourcePtr ResourceManager::ResourceByName(const std::string& p_name)
{
	auto resource = m_resourceName.find(p_name);

	if (resource == m_resourceName.end())
		return ResourcePtr();
	return resource->second;
}

ResourcePtr ResourceManager::ResourceByID(ResourceHandle p_ID)
{
	return m_resourceID.find(p_ID)->second;
}

void ResourceManager::MemoryCheck()
{
	if (m_memoryUsage >= m_maxMemory)
	{
		FireCacheMiss(CacheMiss::Capacity);
		while (m_memoryUsage >= m_maxMemory && !m_LRU.IsEmpty())
		{
			auto resourceID = m_LRU.PopLeastUsed();
			Unload(ResIDToName(resourceID));
		}
	}
}

void ResourceManager::UpdateMemoryUsage()
{
	size_t actualMemoryUsage = 0;
	for (auto rsource : m_resources)
		actualMemoryUsage += rsource->GetSize();

	m_memoryUsage = actualMemoryUsage;
}

void ResourceManager::LRU::UpdateLRU(ResourceHandle p_ID)
{
	auto findResource = m_lruMap.find(p_ID);
	if (findResource == m_lruMap.end())
	{
		m_LRU.emplace_front(p_ID);
		m_lruMap.emplace(p_ID, m_LRU.begin());
	}
	else
	{
		m_LRU.erase(findResource->second);
		m_LRU.emplace_front(p_ID);
		m_lruMap[p_ID] = m_LRU.begin();
	}
}

void ResourceManager::LRU::RemoveResource(ResourceHandle p_ID)
{
	auto findResource = m_lruMap.find(p_ID);
	if (findResource == m_lruMap.end())
		return;
	m_LRU.erase(findResource->second);
	m_lruMap.erase(findResource);
}

ResourceHandle ResourceManager::LRU::PopLeastUsed()
{
	if (m_LRU.empty())
		return 0;

	auto resourceID = m_LRU.back();
	m_lruMap.erase(resourceID);
	m_LRU.pop_back();
	return resourceID;
}

void ResourceManager::LRU::Clear()
{
	m_LRU.clear();
	m_lruMap.clear();
}

bool ResourceManager::LRU::IsEmpty()
{
	return m_LRU.empty();
}
