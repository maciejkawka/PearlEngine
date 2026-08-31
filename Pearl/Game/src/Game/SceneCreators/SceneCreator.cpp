#include "SceneCreators/SceneCreator.h"

#include "ChessEngine/BoardCoordinator.h"
#include "Systems/MainCamera.h"

#include "Engine/Core/ECS/Components/PhysicsComponents.h"
#include "Engine/Renderer/Core/IRenderFrontend.h"
#include "Engine/Physics/Core/PhysicsSystem.h"

namespace ChessGame {

	SceneCreator::SceneCreator(PrCore::EntityViewer* p_entityViewer)
	{
		m_entityViewer = p_entityViewer;

		CreateEntities();
		ProcessEntities();
	}

	void SceneCreator::CreateEntities()
	{
		{
			auto mainCamera = m_entityViewer->CreateEntity("MainCamera");
			mainCamera.AddComponent<PrCore::TransformComponent>();
			mainCamera.AddComponent<MainCamera>()->camera = PrSystems::Get<PrRenderer::IRenderFrontend>()->GetCamera();

			auto staticRigidbody = mainCamera.AddComponent<PrCore::RigidBodyStaticComponent>();

			PrPhysics::BoxGeometery box;
			PrPhysics::Material material;

			box.halfExtents = PrCore::Math::vec3(0.1f);
			auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(box, material);
			shape->SetFlags(PrPhysics::ShapeFlags::Trigger);

			staticRigidbody->rigidBody->AttachShape(shape);
		}
	}

	void SceneCreator::ProcessEntities()
	{
		for (auto [entity] : m_entityViewer->AllEntities())
		{
			if (entity.GetComponent<PrCore::NameComponent>()->name.find("Overlay") != std::string::npos)
			{
				m_chessOverlayEntities.push_back(entity);
			}
		}

		for (auto [entity] : m_entityViewer->AllEntities())
		{
			if (entity.GetComponent<PrCore::NameComponent>()->name.find("Overlay") != std::string::npos)
			{
				entity.GetComponent<PrCore::TagComponent>()->tag == "Overlay";
			}
		}
	}

	void SceneCreator::CreateMesh()
	{
		auto pScene = PrSystems::Get<PrCore::SceneManager>();
		for (auto [entity] : m_entityViewer->AllEntities())
		{
			if (entity.GetComponent<PrCore::NameComponent>()->name.find("Overlay") != std::string::npos)
			{
				auto meshRenderer = entity.GetComponent<PrCore::MeshRendererComponent>();

				meshRenderer->shadowCaster = false;

				auto sharedMaterial = PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Material>("Chess/materials/overlay.mat");

				auto matInstance = std::make_shared<PrRenderer::Material>(*sharedMaterial.GetData());
				for (int i = 0; i < meshRenderer->materials.size(); i++)
				{
					meshRenderer->materials[i] = matInstance;
				}
			}
		}
	}

	void SceneCreator::CreatePhysics()
	{
		for (auto [entity, meshComponent, transformComponent] : m_entityViewer->EntitesWithComponents<PrCore::MeshRendererComponent, PrCore::TransformComponent>())
		{
			auto name = entity.GetComponent<PrCore::NameComponent>()->name;
			if (name.find("Overlay") != std::string::npos)
			{
				if (name.find("Square") != std::string::npos)
				{
					auto staticRigidbody = entity.AddComponent<PrCore::RigidBodyStaticComponent>();

					auto scale = transformComponent->GetScale();
					PrPhysics::Material material;
					auto it = m_convexMeshes.find(name.substr(0, name.find("_")));
					if (it != m_convexMeshes.end())
					{
						auto convexMesh = PrSystems::Get<PrCore::ResourceSystem>()->Get<PrPhysics::IConvexMesh>(it->second);
						auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(PrPhysics::ConvexGeometry{ convexMesh, scale }, material);
						staticRigidbody->rigidBody->AttachShape(shape);
					}
					else
					{
						auto convexMesh = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateConvexMesh(meshComponent->mesh.GetData());
						auto resourceHandle = PrSystems::Get<PrCore::ResourceSystem>()->Register<PrPhysics::IConvexMesh>(convexMesh);
						m_convexMeshes[name.substr(0, name.find("_"))] = resourceHandle.GetID();

						auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(PrPhysics::ConvexGeometry{ convexMesh, scale }, material);
						staticRigidbody->rigidBody->AttachShape(shape);
					}
				}
				else
				{
					auto staticRigidbody = entity.AddComponent<PrCore::RigidBodyStaticComponent>();
					auto parentEntity = entity.GetComponent<PrCore::ParentComponent>()->parent;

					auto scale = parentEntity.GetComponent<PrCore::TransformComponent>()->GetScale();
					PrPhysics::Material material;
					auto it = m_convexMeshes.find(name.substr(0, name.find("_")));
					if (it != m_convexMeshes.end())
					{
						auto convexMesh = PrSystems::Get<PrCore::ResourceSystem>()->Get<PrPhysics::IConvexMesh>(it->second);
						auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(PrPhysics::ConvexGeometry{ convexMesh, scale }, material);
						staticRigidbody->rigidBody->AttachShape(shape);
					}
					else
					{
						auto convexMesh = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateConvexMesh(meshComponent->mesh.GetData());
						auto resourceHandle = PrSystems::Get<PrCore::ResourceSystem>()->Register<PrPhysics::IConvexMesh>(convexMesh);
						m_convexMeshes[name.substr(0, name.find("_"))] = resourceHandle.GetID();

						auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(PrPhysics::ConvexGeometry{ convexMesh, scale }, material);
						staticRigidbody->rigidBody->AttachShape(shape);
					}
				}
			}
		}
	}

	void SceneCreator::CreateAudio()
	{

	}
}