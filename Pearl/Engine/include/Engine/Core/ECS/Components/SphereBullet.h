#pragma once

#include "Core/ECS/BaseComponent.h"

namespace PrCore::ECS {

	class SphereBullet : public BaseComponent {
	public:
		SphereBullet() = default;
		virtual ~SphereBullet() = default;

		virtual void OnSerialize(Utils::JSON::json& p_serialized) {}

		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) {}

		float time = 0.0f;
		float maxTime = 5.0f;
	};
}