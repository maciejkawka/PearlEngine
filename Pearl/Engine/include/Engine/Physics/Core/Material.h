#pragma once

namespace PrPhysics {

	enum class MaterialFlag {
		DisableFriction = 1 << 0,
		StrongFriction = 1 << 1,
		ImprovedPathFriction = 1 << 2,
		Reserved = 1 << 3,
		AccelerationSpring = 1 << 4
	};
	DEFINE_ENUM_FLAG_OPERATORS(MaterialFlag);

	struct Material 
	{
		float dynamicFriction = 0.0f;
		float staticFriction  = 0.0f;
		float restitution     = 0.0f;
		float dumping         = 0.0f;
		MaterialFlag flag     = MaterialFlag::ImprovedPathFriction;
	};
}