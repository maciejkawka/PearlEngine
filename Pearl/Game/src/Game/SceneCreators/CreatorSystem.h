#pragma once

#include "SceneCreators/SceneCreator.h"
#include "ChessEngine/BoardCoordinator.h"

#include "Engine/Core/ECS/BaseSystem.h"


namespace ChessGame {

	class CreatorSystem : public PrCore::BaseSystem {
	public:
		void OnCreate() override
		{
			SceneCreator creator{ &m_entityViewer };

			creator.CreateMesh();
			creator.CreatePhysics();
			creator.CreateAudio();

			//BoardCoordinator coorindator{ &m_entityViewer };
			//coorindator.MovePiece(Square::A1, Square::A4);
			//coorindator.MovePiece(Square::B1, Square::A4);
		}

		void OnUpdate(float p_dt) override
		{
		}

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override {}
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override  {}
	};
}