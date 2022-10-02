#pragma once
#include "Core/ECS/BaseSystem.h"


namespace PrCore::ECS {

	class HierarchyTransform: public BaseSystem {
	public:
		void OnCreate() override;
		void OnUpdate(float p_dt) override;
	};
}