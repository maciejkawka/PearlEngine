#pragma once
#include"Core/ECS/BaseComponent.h"

//Include Components
#include"Core/ECS/Components/CoreComponents.h"
#include"Core/ECS/Components/RendererComponents.h"
#include"Core/ECS/Components/TransformComponent.h"

namespace PrCore::ECS {

	class TimeComponent : public BaseComponent
	{
	public:
		float time = 0;
	};
}
