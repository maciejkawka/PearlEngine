#include"Core/Common/pearl_pch.h"

#include"Core/Resources/Resource.h"
#include"Core/Resources/ResourceManager.h"
#include"Core/Events/ResourceEvents.h"
#include"Core/Events/EventManager.h"

using namespace PrCore::Resources;

PrCore::Resources::Resource::Resource(const std::string& p_name):
	m_name(p_name),
	m_handle(UINT_MAX),
	m_size(0),
	m_state(ResourceStatus::Unmanaged)
{}

Resource::Resource(const std::string& p_name, ResourceHandle p_ID) :
	m_name(p_name),
	m_handle(p_ID),
	m_state(ResourceStatus::Unloaded),
	m_size(0)
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
		LoadDefault();
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
	PrCore::Events::EventPtr event = 
		std::make_shared<PrCore::Events::ResourceUnloadedEvent>(m_name, m_size, m_handle);
	PrCore::Events::EventManager::GetInstance().QueueEvent(event);
}

void Resource::FireLoadedEvent()
{
	PrCore::Events::EventPtr event =
		std::make_shared<PrCore::Events::ResourceLoadedEvent>(m_name, m_size, m_handle);
	PrCore::Events::EventManager::GetInstance().QueueEvent(event);
}

void Resource::FireCorruptedEvent()
{
	PrCore::Events::EventPtr event =
		std::make_shared<PrCore::Events::ResourceCorruptedEvent>(m_name, m_size, m_handle);
	PrCore::Events::EventManager::GetInstance().QueueEvent(event);
}
