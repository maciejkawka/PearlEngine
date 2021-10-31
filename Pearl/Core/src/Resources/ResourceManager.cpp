#include"Core/Common/pearl_pch.h"

#include"Core/Resources/ResourceManager.h"

using namespace PrCore::Resources;

unsigned int ResourceManager::s_nextResourceID = 0;

ResourceManager::ResourceManager() :
	m_memoryUsage(0)
{
	m_maxMemory = UINT_MAX;
}

ResourceManager::~ResourceManager()
{
	DeleteAll();
}

ResourcePtr ResourceManager::CreateResource(const std::string& p_name)
{
	if (p_name.empty())
		return ResourcePtr();

	auto findResource = m_resourceName.find(p_name);
	if (m_resourceName.find(p_name) != m_resourceName.end())
		return findResource->second;

	auto resource = ResourcePtr(CreateImpl(p_name));

	m_resourceID.emplace(resource->GetID(), resource);
	m_resourceName.emplace(p_name, resource);
	m_resources.push_back(resource);

	return resource;
}

void ResourceManager::FireCacheMiss(CacheMiss p_cacheMiss)
{
	int i = 0;
}

ResourcePtr ResourceManager::Load(const std::string& p_name)
{
	if (p_name.empty())
		return ResourcePtr();

	ResourcePtr resource = ResourceByName(p_name);
	if (resource->IsLoaded())
		return resource;

	if (!resource)
		resource = CreateResource(p_name);

	resource->Load();

	m_memoryUsage += resource->GetSize();

	m_LRU.UpdateLRU(resource->GetID());
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
	if (p_name.empty())
		return;

	ResourcePtr resource = ResourceByName(p_name);
	if (resource != nullptr && resource->IsLoaded())
	{
		m_memoryUsage -= resource->GetSize();
		resource->Unload();
		m_LRU.RemoveResource(resource->GetID());
	}
}

void ResourceManager::UnloadAll()
{
	for (ResourcePtr resource : m_resources)
	{
		if (resource->IsLoaded())
		{
			m_memoryUsage -= resource->GetSize();
			resource->Unload();
		}
	}

	m_LRU.Clear();
}

void ResourceManager::Delete(const std::string& p_name)
{
	if (p_name.empty())
		return;

	ResourcePtr resource = ResourceByName(p_name);
	if (resource == nullptr)
		return;

	if (resource->IsLoaded())
		Unload(p_name);

	DeleteImpl(resource);

	m_resourceID.erase(resource->GetID());
	m_resourceName.erase(p_name);
	m_resources.remove(resource);
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


	m_LRU.UpdateLRU(resource->GetID());
	return resource;
}

void ResourceManager::OnResourceLoaded(ResourceSize p_size)
{
	//m_memoryUsage += p_size;
}

void ResourceManager::OnResourceUnloaded(ResourceSize p_size)
{
	/*m_memoryUsage -= p_size;
	if (m_memoryUsage <= 0)
		std::cout << "ELO";*/
}

ResourceID ResourceManager::ResNameToID(const std::string& p_name)
{
	return ResourceByName(p_name)->GetID();
}

std::string ResourceManager::ResIDToName(ResourceID p_ID)
{
	return ResourceByID(p_ID)->GetName();
}

ResourcePtr ResourceManager::ResourceByName(const std::string& p_name)
{
	return m_resourceName.find(p_name)->second;
}

ResourcePtr ResourceManager::ResourceByID(ResourceID p_ID)
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

void ResourceManager::LRU::UpdateLRU(ResourceID p_ID)
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

void ResourceManager::LRU::RemoveResource(ResourceID p_ID)
{
	auto findResource = m_lruMap.find(p_ID);
	if (findResource == m_lruMap.end())
		return;
	m_LRU.erase(findResource->second);
	m_lruMap.erase(findResource);
}

ResourceID ResourceManager::LRU::PopLeastUsed()
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
