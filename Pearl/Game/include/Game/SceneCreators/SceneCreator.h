#pragma once

#include "Engine/Core/ECS/ECS.h"
#include "Engine/Core/ECS/EntityViewer.h"

#include <vector>

namespace ChessGame {

	class SceneCreator {
	public:
		SceneCreator(PrCore::EntityViewer* p_entityViewer);

		void ProcessEntities();
		void CreateEntities();
		void CreatePhysics();
		void CreateMesh();
		void CreateAudio();
		

	private:
		std::vector<PrCore::Entity> m_chessEntities;
		std::vector<PrCore::Entity> m_chessOverlayEntities;
		PrCore::EntityViewer*       m_entityViewer;

		std::map<std::string, PrCore::ResourceID> m_convexMeshes;
	};
}