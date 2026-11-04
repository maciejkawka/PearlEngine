#pragma once

#include "Components/MainComponents.h"

#include "Engine/Physics/Shape/IConvexMesh.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"

#include <string>
#include <vector>

namespace ChessGame {
	
	// Creates piece entities, the piece root entity with a mesh and a child overlay entity with a collider
	class PiecesFactory	{
	public:
		PrRenderer::MeshPtr                     GetMesh(PieceType p_type, Color p_color);
		PrRenderer::MeshPtr                     GetMeshOverlay(PieceType p_type, Color p_color);
		std::vector<PrRenderer::MaterialHandle> GetMaterials(PieceType p_type, Color p_color);
		PrPhysics::IConvexMeshPtr               GetConvexMesh(PieceType p_type, Color p_color);

		// Board piece, synchronized by BoardCoordinator and selectable with the mouse
		PrCore::Entity                          CreatePieceEntity(PieceType p_type, Color p_color);

		// Visual only piece, no PieceComponent and no collider, ignored by the board and raycasts
		PrCore::Entity                          CreatePreviewEntity(PieceType p_type, Color p_color);

		// Standing piece rotation on the board
		static PrCore::Math::quat               GetPieceRotation(PieceType p_type, Color p_color);

	private:
		PrCore::Entity CreatePieceVisual(PieceType p_type, Color p_color, const std::string& p_name);
		std::string    GetName(PieceType p_type, Color p_color);
	};
}