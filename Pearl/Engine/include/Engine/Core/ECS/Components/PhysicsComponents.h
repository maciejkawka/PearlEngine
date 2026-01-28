#pragma once

#include "Core/ECS/BaseComponent.h"
#include "Core/Resources/ResourceSystem.h"

#include "Physics/Core/PhysicsSystem.h"
#include "Physics/Actor/IRigidBody.h"
#include "Physics/Shape/IGeometry.h"

namespace PrCore {

	class RigidBodyDynamicComponent : public BaseComponent {
	public:
		PrPhysics::IRigidBodyDynamicPtr rigidBody;

		void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			// Basic
			p_serialized["type"] = rigidBody->GetType();
			p_serialized["flags"] = rigidBody->GetRigidbodyFlags();
			p_serialized["actorFlags"] = rigidBody->GetActorFlag();
			p_serialized["mass"] = rigidBody->GetMass();

			// Shape
			Utils::JSON::json jsonShape;
			auto shape = rigidBody->GetShape();
			jsonShape["flags"] = shape->GetFlags();

			// Geometry
			Utils::JSON::json jsonGeo;
			auto geometery = shape->GetGeometry();
			jsonGeo["type"] = geometery->GetType();

			PrPhysics::GeometeryVisitor visitor
			{
				[&](const PrPhysics::SphereGeometry* p_geometery) { jsonGeo["radius"] = p_geometery->radius; },
				[&](const PrPhysics::PlaneGeometry* p_geometery) {},
				[&](const PrPhysics::CapsuleGeometry* p_geometery) { jsonGeo["radius"] = p_geometery->radius; jsonGeo["halfHeight"] = p_geometery->halfHeight; },
				[&](const PrPhysics::BoxGeometery* p_geometery) { jsonGeo["halfExtents"] = Utils::JSONParser::ParseVec3(p_geometery->halfExtents); },
				[&](const PrPhysics::ConvexGeometry* p_geometery) {
					if (p_geometery->convexMeshHandle.GetOrigin() == ResourceOrigin::File)
					{
						jsonGeo["scale"] = Utils::JSONParser::ParseVec3(p_geometery->scale);
						jsonGeo["path"] = p_geometery->convexMeshHandle.GetPath();
					}
					else
					{
						PRLOG_WARN("Cannot seriallize covex mesh. It was created in memory. This is not supported yet. RigidBody - name: {0}", rigidBody->GetName());
					}
				},
				[&](const PrPhysics::TriangleGeometery* p_geometery) {
					PRLOG_WARN("Cannot seriallize triangle mesh in dynamic rigidbody name: {0}", rigidBody->GetName());
				}
			};
			geometery->Accept(visitor);
			jsonShape["geometery"] = jsonGeo;

			//Material
			Utils::JSON::json jsonMat;
			auto material = shape->GetMaterial();
			jsonMat["dumping"] = material.dumping;
			jsonMat["dynamicFriction"] = material.dynamicFriction;
			jsonMat["flag"] = material.flag;
			jsonMat["restitution"] = material.restitution;
			jsonMat["staticFriction"] = material.staticFriction;

			jsonShape["material"] = jsonMat;

			p_serialized["shape"] = jsonShape;
		}

		void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			// Basic
			rigidBody->SetActorFlags(p_deserialized["actorFlags"]);
			rigidBody->SetRigidBodyFlags(p_deserialized["flags"]);
			rigidBody->SetMass(p_deserialized["mass"]);

			//Shape
			auto jsonShape = p_deserialized["shape"];
			
			// Material
			auto mat = jsonShape["material"];
			PrPhysics::Material material;
			material.dumping = mat["dumping"];
			material.dynamicFriction = mat["dynamicFriction"];
			material.flag = mat["flag"];
			material.restitution = mat["restitution"];
			material.staticFriction = mat["staticFriction"];

			// Geometry
			std::unique_ptr<PrPhysics::IGeometry> geometery = nullptr;
			auto geom = jsonShape["geometery"];
			PrPhysics::GeometryType type = geom["type"];
			switch (type)
			{
			case PrPhysics::GeometryType::Sphere:
			{
				auto sphere = std::make_unique<PrPhysics::SphereGeometry>();
				sphere->radius = geom["radius"];
				geometery = std::move(sphere);
			}
			break;
			case PrPhysics::GeometryType::Plane:
				geometery = std::make_unique<PrPhysics::PlaneGeometry>();
				break;
			case PrPhysics::GeometryType::Capsule:
			{
				auto capsule = std::make_unique<PrPhysics::CapsuleGeometry>();
				capsule->halfHeight = geom["halfHeight"];
				capsule->radius = geom["radius"];
				geometery = std::move(capsule);
			}
			break;
			case PrPhysics::GeometryType::Box:
			{
				auto box = std::make_unique<PrPhysics::BoxGeometery>();
				box->halfExtents = Utils::JSONParser::ToVec3(geom["halfExtents"]);
				geometery = std::move(box);
			}
			break;
			case PrPhysics::GeometryType::Convex:
			{
				if (geom.contains("path"))
				{
					auto convex = std::make_unique<PrPhysics::ConvexGeometry>();
					convex->convexMeshHandle = PrSystems::Get<ResourceSystem>()->Load<PrPhysics::IConvexMesh>(static_cast<std::string>(geom["path"]));
					convex->scale = PrCore::Utils::JSONParser::ToVec3(geom["scale"]);
					geometery = std::move(convex);
				}
				else
				{
					PRLOG_WARN("Could not deserialize convex geometery. Defaulting to sphere radius of 1.");
					auto sphere = std::make_unique<PrPhysics::SphereGeometry>();
					sphere->radius = 1.0f;
					geometery = std::move(sphere);
				}
			}
			break;
			case PrPhysics::GeometryType::Triangle:
			{
				PRLOG_WARN("Cannot deserialize traignle mesh in dynamic rigidbody!");
			}
			break;
			default:
				PR_ASSERT(false, "Could not deserialize geometery!");
				break;
			};

			auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(*geometery, material);
			shape->SetFlags(jsonShape["flags"]);
			rigidBody->AttachShape(shape);
		}
	};

	class RigidBodyStaticComponent : public BaseComponent {
	public:
		PrPhysics::IRigidStaticPtr rigidBody;

		void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			// Basic
			p_serialized["type"] = rigidBody->GetType();
			p_serialized["actorFlags"] = rigidBody->GetActorFlag();

			// Shape
			Utils::JSON::json jsonShape;
			auto shape = rigidBody->GetShape();
			jsonShape["flags"] = shape->GetFlags();

			// Geometry
			Utils::JSON::json jsonGeo;
			auto geometery = shape->GetGeometry();
			jsonGeo["type"] = geometery->GetType();

			PrPhysics::GeometeryVisitor visitor
			{
				[&](const PrPhysics::SphereGeometry* p_geometery) { jsonGeo["radius"] = p_geometery->radius; },
				[&](const PrPhysics::PlaneGeometry* p_geometery) {},
				[&](const PrPhysics::CapsuleGeometry* p_geometery) { jsonGeo["radius"] = p_geometery->radius; jsonGeo["halfHeight"] = p_geometery->halfHeight; },
				[&](const PrPhysics::BoxGeometery* p_geometery) { jsonGeo["halfExtents"] = Utils::JSONParser::ParseVec3(p_geometery->halfExtents); },
				[&](const PrPhysics::ConvexGeometry* p_geometery) {
					if (p_geometery->convexMeshHandle.GetOrigin() == ResourceOrigin::File)
					{
						jsonGeo["scale"] = Utils::JSONParser::ParseVec3(p_geometery->scale);
						jsonGeo["path"] = p_geometery->convexMeshHandle.GetPath();
					}
					else
					{
						PRLOG_WARN("Cannot seriallize covex mesh. It was created in memory. This is not supported yet. RigidBody - name: {0}", rigidBody->GetName());
					}
				},
				[&](const PrPhysics::TriangleGeometery* p_geometery) {
					if (p_geometery->triangleMeshHandle.GetOrigin() == ResourceOrigin::File)
					{
						jsonGeo["scale"] = Utils::JSONParser::ParseVec3(p_geometery->scale);
						jsonGeo["path"] = p_geometery->triangleMeshHandle.GetPath();
					}
					else
					{
						PRLOG_WARN("Cannot seriallize triangle mesh. It was created in memory. This is not supported yet. RigidBody - name: {0}", rigidBody->GetName());
					}
				}
			};
			geometery->Accept(visitor);
			jsonShape["geometery"] = jsonGeo;

			//Material
			Utils::JSON::json jsonMat;
			auto material = shape->GetMaterial();
			jsonMat["dumping"] = material.dumping;
			jsonMat["dynamicFriction"] = material.dynamicFriction;
			jsonMat["flag"] = material.flag;
			jsonMat["restitution"] = material.restitution;
			jsonMat["staticFriction"] = material.staticFriction;

			jsonShape["material"] = jsonMat;

			p_serialized["shape"] = jsonShape;
		}

		void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			// Basic
			rigidBody->SetActorFlags(p_deserialized["actorFlags"]);

			// Shape
			auto jsonShape = p_deserialized["shape"];

			// Material
			auto mat = jsonShape["material"];
			PrPhysics::Material material;
			material.dumping = mat["dumping"];
			material.dynamicFriction = mat["dynamicFriction"];
			material.flag = mat["flag"];
			material.restitution = mat["restitution"];
			material.staticFriction = mat["staticFriction"];

			// Geometry
			std::unique_ptr<PrPhysics::IGeometry> geometery = nullptr;
			auto geom = jsonShape["geometery"];
			PrPhysics::GeometryType type = geom["type"];
			switch (type)
			{
			case PrPhysics::GeometryType::Sphere:
			{
				auto sphere = std::make_unique<PrPhysics::SphereGeometry>();
				sphere->radius = geom["radius"];
				geometery = std::move(sphere);
			}
			break;
			case PrPhysics::GeometryType::Plane:
				geometery = std::make_unique<PrPhysics::PlaneGeometry>();
				break;
			case PrPhysics::GeometryType::Capsule:
			{
				auto capsule = std::make_unique<PrPhysics::CapsuleGeometry>();
				capsule->halfHeight = geom["halfHeight"];
				capsule->radius = geom["radius"];
				geometery = std::move(capsule);
			}
			break;
			case PrPhysics::GeometryType::Box:
			{
				auto box = std::make_unique<PrPhysics::BoxGeometery>();
				box->halfExtents = Utils::JSONParser::ToVec3(geom["halfExtents"]);
				geometery = std::move(box);
			}
			break;
			case PrPhysics::GeometryType::Convex:
			{
				if (geom.contains("path"))
				{
					auto convex = std::make_unique<PrPhysics::ConvexGeometry>();
					convex->convexMeshHandle = PrSystems::Get<ResourceSystem>()->Load<PrPhysics::IConvexMesh>(static_cast<std::string>(geom["path"]));
					convex->scale = PrCore::Utils::JSONParser::ToVec3(geom["scale"]);
					geometery = std::move(convex);
				}
				else
				{
					PRLOG_WARN("Could not deserialize convex geometery. Defaulting to sphere radius of 1.");
					auto sphere = std::make_unique<PrPhysics::SphereGeometry>();
					sphere->radius = 1.0f;
					geometery = std::move(sphere);
				}
			}
			break;
			case PrPhysics::GeometryType::Triangle:
			{
				if (geom.contains("path"))
				{
					auto triangle = std::make_unique<PrPhysics::TriangleGeometery>();
					triangle->triangleMeshHandle = PrSystems::Get<ResourceSystem>()->Load<PrPhysics::ITriangleMesh>(static_cast<std::string>(geom["path"]));
					triangle->scale = PrCore::Utils::JSONParser::ToVec3(geom["scale"]);
					geometery = std::move(triangle);
				}
				else
				{
					PRLOG_WARN("Could not deserialize triangle geometery. Defaulting to sphere radius of 1.");
					auto sphere = std::make_unique<PrPhysics::SphereGeometry>();
					sphere->radius = 1.0f;
					geometery = std::move(sphere);
				}
			}
			break;
			default:
				PR_ASSERT(false, "Could not deserialize geometery!");
				break;
			};

			auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(*geometery, material);
			shape->SetFlags(jsonShape["flags"]);
			rigidBody->AttachShape(shape);
		}
	};
}