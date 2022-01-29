#include"Core/Common/pearl_pch.h"

#include"Core/Resources/Resource.h"
#include"Core/Resources/ResourceManager.h"
#include"Core/Events/ResourceEvents.h"
#include"Core/Events/EventManager.h"

using namespace PrCore::Resources;

Resources::Resources(const std::string& p_name, ResourceManager* p_creator, ResourceID p_ID) :
	m_name(p_name),
	m_creator(p_creator),
	m_ID(p_ID),
	m_state(ResourceStatus::Unloaded),
	m_size(0)
{}

void Resources::Load()
{
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
		m_state = ResourceStatus::Corrupted;
}

void Resources::Unload()
{
	m_state = ResourceStatus::Unloading;

	PreUnloadImpl();
	auto result = UnloadImpl();
	if (result)
	{
		FireUnloadedEvent();
		m_state = ResourceStatus::Unloaded;
		PostUnloadImpl();
	}
	else
		m_state = ResourceStatus::Corrupted;
}

void Resources::FireUnloadedEvent()
{
	PrCore::Events::EventPtr event = 
		std::make_shared<PrCore::Events::ResourceUnloadedEvent>(m_name, m_size, m_ID);
	PrCore::Events::EventManager::GetInstance().QueueEvent(event);
}

void Resources::FireLoadedEvent()
{
	PrCore::Events::EventPtr event =
		std::make_shared<PrCore::Events::ResourceLoadedEvent>(m_name, m_size, m_ID);
	PrCore::Events::EventManager::GetInstance().QueueEvent(event);
}

void Resources::FireCorruptedEvent()
{
	PrCore::Events::EventPtr event =
		std::make_shared<PrCore::Events::ResourceCorruptedEvent>(m_name, m_size, m_ID);
	PrCore::Events::EventManager::GetInstance().QueueEvent(event);
}
