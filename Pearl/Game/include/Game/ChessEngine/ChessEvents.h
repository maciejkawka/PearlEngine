#pragma once

#include "ChessEngine/ChessSystem.h"

#include "Engine/Core/Events/Event.h"

namespace ChessGame {

	// Fired by BoardCoordinator after a legal move is applied to the board
	class MoveMadeEvent : public PrCore::Event {
	public:
		MoveMadeEvent(const Move& p_move, Color p_movedSide) :
			m_move(p_move),
			m_movedSide(p_movedSide)
		{}

		Move  m_move;
		Color m_movedSide;

		DEFINE_EVENT_GUID(0x7c2a91e4);
	};

	// Fired by BoardCoordinator when the whole board state is replaced, reset or load
	class BoardChangedEvent : public PrCore::Event {
	public:
		BoardChangedEvent() = default;

		DEFINE_EVENT_GUID(0x3e5bd017);
	};

	// Fired by BoardCoordinator once when the game ends
	class GameOverEvent : public PrCore::Event {
	public:
		GameOverEvent(GameOverResult p_result, GameOverType p_type, Color p_winner) :
			m_result(p_result),
			m_type(p_type),
			m_winner(p_winner)
		{}

		GameOverResult m_result;
		GameOverType   m_type;   // Draw or Win, Win means m_winner won
		Color          m_winner; // Valid only for GameOverType::Win

		DEFINE_EVENT_GUID(0xa41f6c58);
	};
}
