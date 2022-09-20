#pragma once
#include"Core/ECS/SystemManager.h"
#include"Core/ECS/BaseSystem.h"
#include"Core/ECS/Systems.h"

namespace PrCore::ECS {

	void DeduceSystemTypeByString(SystemManager* p_systemManager, std::string p_typeName)
	{
		BaseSystem* system = nullptr;

		if (p_typeName == typeid(Increase).name())					p_systemManager->RegisterSystem<Increase>();
		//else if (p_typeName == typeid(Increase).name())			p_systemManager->RegisterSystem<Increase>();
		else														PR_ASSERT(true, "System type is invalid" + p_typeName);
	}
}