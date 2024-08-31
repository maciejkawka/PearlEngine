#pragma once
#include "Scene.h"
#include"Core/ECS/SystemManager.h"

namespace PrCore::ECS {

	template<class System>
	void Scene::RegisterSystem()
	{
		m_systemManager->RegisterSystem<System>();
	}

	template<class System>
	void Scene::SetActiveSystem(bool p_isActive)
	{
		m_systemManager->SetActiveSystem<System>(p_isActive);
	}

	template<class System>
	bool Scene::IsActiveSystem() const
	{
		return m_systemManager->IsActiveSystem<System>();
	}
}
