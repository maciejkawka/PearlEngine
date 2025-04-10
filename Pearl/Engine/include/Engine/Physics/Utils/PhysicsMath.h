#pragma once

#include "Core/Math/Math.h"

#include <memory>

namespace PrPhysics {

	struct Bounds3 {
		Bounds3() :
			min(PrCore::Math::vec3{ 0.0f }),
			max(PrCore::Math::vec3{ 0.0f })
		{}

		Bounds3(const PrCore::Math::vec3& p_min, const PrCore::Math::vec3& p_max) :
			min(p_min),
			max(p_max)
		{}

		PrCore::Math::vec3 min;
		PrCore::Math::vec3 max;
	};

	struct Transform {
		Transform() :
			rotation(PrCore::Math::identity<PrCore::Math::quat>()),
			position(PrCore::Math::vec3{ 0 })
		{}

		Transform(const PrCore::Math::quat& p_rotation, const PrCore::Math::vec3& p_position) :
			rotation(p_rotation),
			position(p_position)
		{}

		PrCore::Math::quat  rotation;
		PrCore::Math::vec3	position;
	};
}