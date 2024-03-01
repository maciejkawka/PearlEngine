#pragma once
#include"Core/ECS/SystemManager.h"
#include"Core/ECS/Systems.h"

namespace PrCore::ECS {

	void DeduceSystemTypeByString(SystemManager* p_systemManager, std::string p_typeName)
	{
		BaseSystem* system = nullptr;

		if (p_typeName == typeid(MeshRendererSystem).name())				p_systemManager->RegisterSystem<MeshRendererSystem>();
		else if (p_typeName == typeid(HierarchyTransform).name())			p_systemManager->RegisterSystem<HierarchyTransform>();
		else if (p_typeName == typeid(TestSystem).name())					p_systemManager->RegisterSystem<TestSystem>();
		else if (p_typeName == typeid(RenderStressTest).name())				p_systemManager->RegisterSystem<RenderStressTest>();
		else																PR_ASSERT(true, "System type is invalid" + p_typeName);
	}
}