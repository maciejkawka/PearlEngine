#include"Core/Common/pearl_pch.h"

#include"Core/Resources/Resource.h"
#include"Core/Events/ResourceEvents.h"
#include"Core/Events/EventManager.h"

using namespace PrCore::Resources;

Resource::Resource(const std::string& p_name):
	m_name(p_name),
	m_handle(UINT_MAX),
	m_size(0),
	m_state(ResourceStatus::Unmanaged),
	m_nameHash(std::hash<std::string>{}(m_name))
{}

Resource::Resource(const std::string& p_name, ResourceHandle p_ID) :
	m_name(p_name),
	m_handle(p_ID),
	m_state(ResourceStatus::Unloaded),
	m_size(0),
	m_nameHash(std::hash<std::string>{}(m_name))
{}

void Resource::Load()
{
	if (m_state == ResourceStatus::Unmanaged)
		return;

	if (IsLoaded())
		return;

	m_state = ResourceStatus::Loading;

	PreLoadImpl();
	auto result = LoadImpl();
	if (result)
	{
		FireLoadedEvent();
		m_state = ResourceStatus::Loaded;
		PostLoadImpl();
		CalculateSize();
	}
	else
	{
		FireCorruptedEvent();
		m_state = ResourceStatus::Corrupted;
		LoadCorruptedResource();
	}
}

void Resource::Unload()
{
	if (m_state == ResourceStatus::Unmanaged)
		return;

	m_state = ResourceStatus::Unloading;

	PreUnloadImpl();
	auto result = UnloadImpl();
	if (result)
	{
		FireUnloadedEvent();
		m_state = ResourceStatus::Unloaded;
		PostUnloadImpl();
		CalculateSize();
	}
	else
		m_state = ResourceStatus::Corrupted;
}

void Resource::FireUnloadedEvent()
{
	Events::EventPtr event = 
		std::make_shared<Events::ResourceUnloadedEvent>(m_name, m_size, m_handle);
	Events::EventManager::GetInstance().QueueEvent(event);
}

void Resource::FireLoadedEvent()
{
	Events::EventPtr event =
		std::make_shared<Events::ResourceLoadedEvent>(m_name, m_size, m_handle);
	Events::EventManager::GetInstance().QueueEvent(event);
}

void Resource::FireCorruptedEvent()
{
	Events::EventPtr event =
		std::make_shared<Events::ResourceCorruptedEvent>(m_name, m_size, m_handle);
	Events::EventManager::GetInstance().QueueEvent(event);
}
