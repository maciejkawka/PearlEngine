#include "SceneCreators/PiecesFactory.h"

#include "GameConstants.h"

#include "Engine/Core/ECS/SceneManager.h"
#include "Engine/Core/ECS/Components/TransformComponent.h"
#include "Engine/Core/Resources/ResourceSystem.h"
#include "Engine/Physics/Core/PhysicsSystem.h"

namespace ChessGame {

	namespace {
		// Piece meshes are exported lying on the side and in model units
		const PrCore::Math::quat PieceRotation{ 0.707f, 0.0f, -0.707f, 0.0f };
		const PrCore::Math::vec3 PieceScale{ 4.00753f, 4.00753f, 4.00753f };
	}

	PrRenderer::MeshPtr PiecesFactory::GetMesh(PieceType p_type, Color p_color)
	{
		std::string name{ MeshPath };
		name.append(GetName(p_type, p_color));
		name.append("_Shared.obj");

		auto meshPtr = PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Mesh>(name);
		PR_ASSERT(meshPtr.IsValid());

		return meshPtr.GetData();
	}

	PrRenderer::MeshPtr PiecesFactory::GetMeshOverlay(PieceType p_type, Color p_color)
	{
		std::string name{ MeshPath };
		name.append(GetName(p_type, p_color));
		name.append("_Shared_Overlay.obj");

		auto meshPtr = PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Mesh>(name);
		PR_ASSERT(meshPtr.IsValid());

		return meshPtr.GetData();
	}

	std::vector<PrRenderer::MaterialHandle> PiecesFactory::GetMaterials(PieceType p_type, Color p_color)
	{
		std::string name{ MaterialsPath };
		name.append(GetName(p_type, p_color));

		std::vector<PrRenderer::MaterialHandle> materials;
		if (p_type == PieceType::Pawn)
		{
			std::string top = name + "_Top.mat";
			std::string body = name + "_Body.mat";
			materials.push_back(PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Material>(top));
			materials.push_back(PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Material>(body));

			PR_ASSERT(materials[0].IsValid());
			PR_ASSERT(materials[1].IsValid());
		}
		else 
		{
			materials.push_back(PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Material>(name.append(".mat")));
			PR_ASSERT(materials[0].IsValid());
		}

		return  materials;
	}

	PrPhysics::IConvexMeshPtr PiecesFactory::GetConvexMesh(PieceType p_type, Color p_color)
	{
		std::string name{ PhysicsPath };
		name.append(GetName(p_type, p_color));
		name.append(".physc");

		auto shapePtr = PrSystems::Get<PrCore::ResourceSystem>()->Load<PrPhysics::IConvexMesh>(name);
		PR_ASSERT(shapePtr.IsValid());

		return shapePtr.GetData();
	}

	PrCore::Entity PiecesFactory::CreatePieceEntity(PieceType p_type, Color p_color)
	{
		auto scene = PrSystems::Get<PrCore::SceneManager>()->GetActiveScene();

		// Parent piece
		PrCore::Entity entity = scene->CreateEntity(GetName(p_type, p_color));

		// White knights face the opposite direction
		PrCore::Math::quat rotation = PieceRotation;
		if (p_type == PieceType::Knight && p_color == Color::White)
			rotation.w = -rotation.w;

		auto transform = entity.AddComponent<PrCore::TransformComponent>();
		transform->SetLocalScale(PieceScale);
		transform->SetRotation(rotation);

		auto pieceComponent = entity.AddComponent<PieceComponent>();
		pieceComponent->color = p_color;
		pieceComponent->type = p_type;
		pieceComponent->square = Square::NoSquare;

		auto mesh = entity.AddComponent<PrCore::MeshRendererComponent>();
		mesh->mesh = GetMesh(p_type, p_color);
		mesh->shadowMesh = mesh->mesh;
		mesh->materials = GetMaterials(p_type, p_color);

		// Overlay
		PrCore::Entity overlayEntity = scene->CreateEntity(GetName(p_type, p_color).append("_Overlay"));
		overlayEntity.AddComponent<PrCore::ParentComponent>()->SetParent(entity);

		overlayEntity.AddComponent<PrCore::TransformComponent>();

		auto meshOverlay = overlayEntity.AddComponent<PrCore::MeshRendererComponent>();
		meshOverlay->mesh = GetMeshOverlay(p_type, p_color);
		meshOverlay->shadowMesh = meshOverlay->mesh;
		meshOverlay->shadowCaster = false;

		PrRenderer::MaterialPtr material = PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Material>(std::string{ OverlayMaterialPath }).GetData();
		meshOverlay->mainMaterial = std::make_shared<PrRenderer::Material>(*material);
		if (p_type == PieceType::Pawn)
			meshOverlay->materials.push_back(meshOverlay->mainMaterial);

		auto rigidbody = overlayEntity.AddComponent<PrCore::RigidBodyStaticComponent>();
		auto convexMesh = GetConvexMesh(p_type, p_color);
		PrPhysics::Material physMat;
		auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(PrPhysics::ConvexGeometry{ convexMesh, PieceScale }, physMat);
		rigidbody->rigidBody->AttachShape(shape);

		return entity;
	}

	std::string PiecesFactory::GetName(PieceType p_type, Color p_color)
	{
		// Names match the asset file names, rook assets are named Castle
		std::string name;
		switch (p_type)
		{
		case PieceType::Pawn:   name = "Pawn";   break;
		case PieceType::Knight: name = "Knight"; break;
		case PieceType::Bishop: name = "Bishop"; break;
		case PieceType::Rook:   name = "Castle"; break;
		case PieceType::Queen:  name = "Queen";  break;
		case PieceType::King:   name = "King";   break;
		default:
			PR_ASSERT(false, "Invalid piece type");
			break;
		}

		name.append(p_color == Color::Black ? "_B" : "_W");
		return name;
	}
}