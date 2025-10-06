#pragma once

#include "Core/Math/Math.h"
#include "Physics/Shape/IConvexMesh.h"
#include "Physics/Shape/ITriangleMesh.h"

#include <functional>

namespace PrPhysics {

	class IConvexMesh;

	struct SphereGeometry;
	struct PlaneGeometry;
	struct CapsuleGeometry;
	struct BoxGeometery;
	struct ConvexGeometry;
	struct TriangleGeometery;

	struct GeometeryVisitor
	{
		std::function<void(const SphereGeometry*)>    onSphere;
		std::function<void(const PlaneGeometry*)>     onPlane;
		std::function<void(const CapsuleGeometry*)>   onCapcule;
		std::function<void(const BoxGeometery*)>      onBox;
		std::function<void(const ConvexGeometry*)>    onConvex;
		std::function<void(const TriangleGeometery*)> onTriangle;

		void visit(const SphereGeometry* p_geom)
		{
			if (onSphere) onSphere(p_geom);
		}

		void visit(const PlaneGeometry* p_geom)
		{
			if (onPlane) onPlane(p_geom);
		}

		void visit(const CapsuleGeometry* p_geom)
		{
			if (onCapcule) onCapcule(p_geom);
		}

		void visit(const BoxGeometery* p_geom)
		{
			if (onBox) onBox(p_geom);
		}

		void visit(const ConvexGeometry* p_geom)
		{
			if (onConvex) onConvex(p_geom);
		}

		void visit(const TriangleGeometery* p_geom)
		{
			if (onTriangle) onTriangle(p_geom);
		}
	};

	enum class GeometryType
	{
		Sphere,
		Plane,
		Capsule,
		Box,
		Convex,
		Triangle
	};

	struct IGeometry
	{
		virtual ~IGeometry() = default;

		GeometryType GetType() const { return type; }
		virtual void Accept(GeometeryVisitor& p_visitor) const = 0;

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

		void Accept(GeometeryVisitor& p_visitor) const override
		{
			p_visitor.visit(this);
		}

		float radius = 1.0f;
	};

	struct PlaneGeometry : public IGeometry
	{
		PlaneGeometry()
		{
			type = GeometryType::Plane;
		}

		void Accept(GeometeryVisitor& p_visitor) const override
		{
			p_visitor.visit(this);
		}
	};

	struct CapsuleGeometry : public IGeometry
	{
		CapsuleGeometry()
		{
			type = GeometryType::Capsule;
		}

		CapsuleGeometry(float p_radius, float p_halfHeight)
		{
			type = GeometryType::Capsule;
			radius = p_radius;
			halfHeight = p_halfHeight;
		}

		void Accept(GeometeryVisitor& p_visitor) const override
		{
			p_visitor.visit(this);
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

		void Accept(GeometeryVisitor& p_visitor) const override
		{
			p_visitor.visit(this);
		}

		PrCore::Math::vec3 halfExtents = PrCore::Math::vec3{ 1.0f };
	};

	struct ConvexGeometry : public IGeometry
	{
		ConvexGeometry()
		{
			type = GeometryType::Convex;
			scale = PrCore::Math::vec3{ 1.0f };
		}

		ConvexGeometry(IConvexMeshHandle p_convexMeshHandle, const PrCore::Math::vec3& p_scale = PrCore::Math::vec3(1.0f))
		{
			type = GeometryType::Convex;
			convexMeshHandle = p_convexMeshHandle;
			scale = p_scale;
		}

		void Accept(GeometeryVisitor& p_visitor) const override
		{
			p_visitor.visit(this);
		}

		PrCore::Math::vec3 scale;
		IConvexMeshHandle  convexMeshHandle;
	};

	struct TriangleGeometery : public IGeometry
	{
		TriangleGeometery()
		{
			type = GeometryType::Triangle;
			scale = PrCore::Math::vec3{ 1.0f };
		}

		TriangleGeometery(ITriangleMeshHandle p_triangle, const PrCore::Math::vec3& p_scale = PrCore::Math::vec3(1.0f))
		{
			type = GeometryType::Triangle;
			triangleMeshHandle = p_triangle;
			scale = p_scale;
		}

		void Accept(GeometeryVisitor& p_visitor) const override
		{
			p_visitor.visit(this);
		}

		PrCore::Math::vec3    scale;
		ITriangleMeshHandle   triangleMeshHandle;
	};
}