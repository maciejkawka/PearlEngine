#pragma once

#include "Core/Math/Math.h"

namespace PrPhysics {

	enum class GeometryType
	{
		Sphere,
		Plane,
		Capsule,
		Box
	};

	struct IGeometry 
	{
		GeometryType GetType() const { return type; }

	protected:
		GeometryType type;
	};

	struct SphereGeometry : public IGeometry 
	{
		SphereGeometry()
		{
			type = GeometryType::Sphere;
		}

		SphereGeometry(float p_radius)
		{
			radius = p_radius;
			type = GeometryType::Sphere;
		}

		float radius = 1.0f;
	};

	struct PlaneGeometry : public IGeometry 
	{
		PlaneGeometry()
		{
			type = GeometryType::Plane;
		}
	};

	struct CapsuleGeometry : public IGeometry 
	{
		CapsuleGeometry()
		{
			type = GeometryType::Capsule;
		}

		float radius = 1.0f;
		float halfHeight = 1.0f;
	};

	struct BoxGeometery : public IGeometry 
	{
		BoxGeometery()
		{
			type = GeometryType::Box;
		}

		BoxGeometery(float p_x, float p_y, float p_z)
		{
			halfExtents.x = p_x;
			halfExtents.y = p_y;
			halfExtents.z = p_z;
			type = GeometryType::Box;
		}

		BoxGeometery(PrCore::Math::vec3 p_halfExtents)
		{
			halfExtents = p_halfExtents;
			type = GeometryType::Box;
		}

		PrCore::Math::vec3 halfExtents = PrCore::Math::vec3{ 1.0f };
	};
}