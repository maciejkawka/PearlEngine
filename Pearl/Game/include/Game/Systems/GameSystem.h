#pragma once

#include "ChessEngine/BoardCoordinator.h"

#include "Engine/Core/ECS/BaseSystem.h"

namespace ChessGame {
	class GameSystem : public PrCore::BaseSystem
	{
	public:
		void OnCreate() 
		{
			BoardCoordinator::Init(&m_entityViewer);
			m_coordinator = BoardCoordinator::GetInstancePtr();
			m_coordinator->SetBoardDirty();
		}

		void OnUpdate(float p_dt)
		{
			if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::R))
			{
				m_coordinator->Reset();
			}

			if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::P))
			{
				m_coordinator->Save();
			}

			if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::L))
			{
				m_coordinator->Load();
			}

			m_coordinator->UpdatePieces();
			m_coordinator->UpdateGameState();
		}

	private:
		enum GameState
		{
			Menu,
			InGame,
			GameOver
		};

		BoardCoordinator* m_coordinator;

	};
}