#pragma once
#include "Core/ECS/BaseSystem.h"
#include "Core/ECS/Components.h"
#include "Core/Input/InputManager.h"
#include "Core/ECS/SceneManager.h"
#include "Core/ECS/Scene.h"

namespace PrCore::ECS {

	class Increase : public BaseSystem {
	public:
		void OnUpdate(float p_dt) override
		{
			for (auto entity : m_entityViewer.EntitesWithComponents<TimeComponent>())
			{
				auto time = entity.GetComponent<TimeComponent>()->time;
				entity.GetComponent<TimeComponent>()->time = time + p_dt;
				//PRLOG_INFO("System is for ID: {0} is {1}", entity.GetID().GetIndex(), entity.GetComponent<TimeComponent>()->time);

				if (entity.GetComponent<NameComponent>()->name == "ELO2")
					if (Input::InputManager::IsKeyPressed(Input::PrKey::G))
						SceneManager::GetInstance().CreateScene("TESTSCENE")->DestoryEntity(entity);
			}
		}
	};
}