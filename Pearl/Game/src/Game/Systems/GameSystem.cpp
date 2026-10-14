#include "GameConstants.h"
#include "Systems/GameSystem.h"
#include "ChessEngine/BoardCoordinator.h"

#include "Engine/Core/Input/InputManager.h"

namespace ChessGame {

	void GameSystem::OnCreate()
	{
		// BoardCoordinator resets and synchronizes the board in the constructor
		BoardCoordinator::Init(&m_entityViewer);
		m_coordinator = BoardCoordinator::GetInstancePtr();
	}

	void GameSystem::OnUpdate(float p_dt)
	{
		auto input = PrSystems::Get<PrCore::InputManager>();

		if (input->IsKeyPressed(ResetGameKey))
			m_coordinator->Reset();

		if (input->IsKeyPressed(SaveGameKey))
			m_coordinator->Save();

		if (input->IsKeyPressed(LoadGameKey))
			m_coordinator->Load();

		m_coordinator->UpdatePieces();
	}
}
