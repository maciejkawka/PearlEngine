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

		PrCore::Entity                          CreatePieceEntity(PieceType p_type, Color p_color);

	private:
		std::string GetName(PieceType p_type, Color p_color);
	};
}