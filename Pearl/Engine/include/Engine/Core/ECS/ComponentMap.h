#pragma once
#include "Core/ECS/SystemManager.h"
#include "Core/ECS/Components.h"

namespace PrCore::ECS {
	
	//Component Map to be update for each component
	//This Map need to be tracked all time
	BaseComponent* DeduceComponentTypeByString(Entity p_entity, std::string p_typeName)
	{
		BaseComponent* component = nullptr;

		if		(p_typeName == typeid(NameComponent).name())			component = p_entity.AddComponent<NameComponent>();
		else if (p_typeName == typeid(UUIDComponent).name())			component = p_entity.AddComponent<UUIDComponent>();
		else if (p_typeName == typeid(TagComponent).name())				component = p_entity.AddComponent<TagComponent>();
		else if (p_typeName == typeid(TransformComponent).name())		component = p_entity.AddComponent<TransformComponent>();
		else if (p_typeName == typeid(ParentComponent).name())			component = p_entity.AddComponent<ParentComponent>();
		else if (p_typeName == typeid(ToDestoryTag).name())			component = p_entity.AddComponent<ToDestoryTag>();
		else if (p_typeName == typeid(TimeComponent).name())			component = p_entity.AddComponent<TimeComponent>();
		else														PR_ASSERT(true, "Component type invalid" + p_typeName);

		return component;
	}
}