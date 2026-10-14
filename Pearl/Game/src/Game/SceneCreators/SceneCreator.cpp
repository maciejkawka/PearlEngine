#include "SceneCreators/SceneCreator.h"
#include "GameConstants.h"
#include "Systems/MainCamera.h"


#include "Engine/Core/ECS/Components/PhysicsComponents.h"
#include "Engine/Core/ECS/Components/RendererComponents.h"
#include "Engine/Core/ECS/Components/TransformComponent.h"
#include "Engine/Renderer/Core/IRenderFrontend.h"
#include "Engine/Physics/Core/PhysicsSystem.h"

namespace ChessGame {

	SceneCreator::SceneCreator(PrCore::EntityViewer* p_entityViewer) :
		m_entityViewer(p_entityViewer)
	{
	}

	void SceneCreator::Build()
	{
		CreateCamera();
		SetupOverlays();
		CreateSquarePhysics();
	}

	void SceneCreator::CreateCamera()
	{
		auto mainCamera = m_entityViewer->CreateEntity(std::string{ MainCameraName });
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

	void SceneCreator::SetupOverlays()
	{
		// Overlays are transparent highlights, each gets own material instance to change its color separately
		auto sharedMaterial = PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Material>(std::string{ OverlayMaterialPath });

		for (auto [entity, nameComponent] : m_entityViewer->EntitesWithComponents<PrCore::NameComponent>())
		{
			if (nameComponent->name.find(OverlayName) == std::string::npos)
				continue;

			entity.GetComponent<PrCore::TagComponent>()->tag = OverlayTag;

			if (!entity.HasComponent<PrCore::MeshRendererComponent>())
				continue;

			auto meshRenderer = entity.GetComponent<PrCore::MeshRendererComponent>();
			meshRenderer->shadowCaster = false;

			auto matInstance = std::make_shared<PrRenderer::Material>(*sharedMaterial.GetData());
			for (auto& material : meshRenderer->materials)
			{
				material = matInstance;
			}
		}
	}

	void SceneCreator::CreateSquarePhysics()
	{
		// Only square overlays need colliders, scene pieces are replaced by the PiecesFactory pieces
		for (auto [entity, meshComponent, transformComponent] : m_entityViewer->EntitesWithComponents<PrCore::MeshRendererComponent, PrCore::TransformComponent>())
		{
			const auto& name = entity.GetComponent<PrCore::NameComponent>()->name;
			if (name.find(SquareOverlayPrefix) != std::string::npos)
			{
				AttachConvexShape(entity, meshComponent->mesh, transformComponent->GetScale());
			}
		}
	}

	void SceneCreator::AttachConvexShape(PrCore::Entity p_entity, const PrRenderer::MeshHandle& p_mesh, const PrCore::Math::vec3& p_scale)
	{
		const auto& name = p_entity.GetComponent<PrCore::NameComponent>()->name;
		auto staticRigidbody = p_entity.AddComponent<PrCore::RigidBodyStaticComponent>();

		// Convex meshes are cached per name prefix to share them between the same overlays
		auto cacheKey = name.substr(0, name.find("_"));

		PrPhysics::IConvexMeshPtr convexMesh;
		auto it = m_convexMeshes.find(cacheKey);
		if (it != m_convexMeshes.end())
		{
			convexMesh = PrSystems::Get<PrCore::ResourceSystem>()->Get<PrPhysics::IConvexMesh>(it->second).GetData();
		}
		else
		{
			convexMesh = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateConvexMesh(p_mesh.GetData());
			convexMesh->SetName(name);
			auto resourceHandle = PrSystems::Get<PrCore::ResourceSystem>()->Register<PrPhysics::IConvexMesh>(convexMesh);
			m_convexMeshes[cacheKey] = resourceHandle.GetID();
		}

		PrPhysics::Material material;
		auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(PrPhysics::ConvexGeometry{ convexMesh, p_scale }, material);
		staticRigidbody->rigidBody->AttachShape(shape);
	}
}
