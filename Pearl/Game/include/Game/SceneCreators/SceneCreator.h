#pragma once

#include "Engine/Core/ECS/ECS.h"
#include "Engine/Core/ECS/EntityViewer.h"

#include <map>
#include <string>

namespace ChessGame {

	// Prepares the chess scene loaded from the model for the game
	class SceneCreator {
	public:
		explicit SceneCreator(PrCore::EntityViewer* p_entityViewer);

		// Creates game entities and sets up the scene overlays
		void Build();

	private:
		void CreateCamera();
		void SetupOverlays();
		void CreateSquarePhysics();

		void AttachConvexShape(PrCore::Entity p_entity, const PrRenderer::MeshHandle& p_mesh, const PrCore::Math::vec3& p_scale);

		PrCore::EntityViewer* m_entityViewer;

		std::map<std::string, PrCore::ResourceID> m_convexMeshes;
	};
}
