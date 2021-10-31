#include"Core/Common/pearl_pch.h"

#include"Core/Resources/Resource.h"
#include"Core/Resources/ResourceManager.h"

using namespace PrCore::Resources;

Resource::Resource(const std::string& p_name, ResourceManager* p_creator, ResourceID p_ID) :
	m_name(p_name),
	m_creator(p_creator),
	m_ID(p_ID),
	m_state(ResourceStatus::Unloaded),
	m_size(0)
{}

void Resource::Load()
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

void Resource::Unload()
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

void Resource::FireUnloadedEvent()
{
	m_creator->OnResourceUnloaded(m_size);
}

void Resource::FireLoadedEvent()
{
	m_creator->OnResourceLoaded(m_size);
}

void Resource::FireCorruptedEvent()
{

}
