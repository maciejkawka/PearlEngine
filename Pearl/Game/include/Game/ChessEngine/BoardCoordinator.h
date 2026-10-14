#pragma once

#include "ChessEngine/ChessSystem.h"
#include "Components/MainComponents.h"
#include "GameConstants.h"

#include "Engine/Core/ECS/EntityViewer.h"
#include "Engine/Core/Utils/Singleton.h"

#include <vector>

namespace ChessGame {

	enum class GameRules
	{
		SinglePlayer,
		Multiplayer
	};

	enum class GameState
	{
		Playing,
		GameOver
	};

	// Bridges ChessSystem with the scene.
	// ChessSystem is the only source of truth for the board state,
	// piece entities are synchronized with it when the board is dirty.
	//
	// Events fired (see ChessEvents.h):
	//   MoveMadeEvent     - after a move is applied, piece entities are synchronized on the next UpdatePieces()
	//   BoardChangedEvent - after Reset() or Load() replaced the board
	//   GameOverEvent     - once when the game ends
	class BoardCoordinator : public PrCore::Utils::Singleton<BoardCoordinator> {
	public:
		BoardCoordinator() = delete;
		BoardCoordinator(PrCore::EntityViewer* p_entityViewer);

		// Moves
		bool              TryMovePiece(Square p_from, Square p_to, PieceType p_promotion = PieceType::Queen);
		void              GenerateMoves();
		const LegalMoves& GetLegalMoves() const;

		// Square overlays
		void ShowPossibleMovesFor(Square p_from);
		void DisablePossibleMoves();

		// Synchronizes piece entities with the board state and updates the game state
		void SetBoardDirty();
		void UpdatePieces();

		// Game state
		Color     SideToMove() const;
		bool      IsCheck() const;
		GameState GetGameState() const;
		bool      CanSelectPiece(Color p_pieceColor) const;

		// Save, load and reset the game
		void Save();
		void Load();
		void Reset();

		void      SetGameRules(GameRules p_rules);
		GameRules GetGameRules() const;
		void      SetPlayerColor(Color p_color);
		Color     GetPlayerColor() const;

		// Helpers
		PrCore::Math::vec3 GetSquarePos(Square p_square) const;

	private:
		void SyncPiecesWithBoard();
		void PlacePiece(PrCore::Entity p_entity, PieceComponent* p_piece, Square p_square);
		void UpdateGameState();
		void OnBoardReplaced();

		using PiecesArray =  std::array<PieceComponent*, BoardSquaresCount>;
		using SquaresArray = std::array<SquareComponent*, BoardSquaresCount>;

		PiecesArray  m_pieces;
		SquaresArray m_squares;
		LegalMoves   m_legalMoves;
		GameRules    m_gameRules = GameRules::Multiplayer;
		GameState    m_gameState = GameState::Playing;
		Color        m_playerColor = Color::White;
		bool         m_isDirty = false;

		ChessSystem           m_system;
		PrCore::EntityViewer* m_entityViewer;

		// Board geometry, square world positions are not available before the first hierarchy update
		float              m_squareStep = 0.25135f;
		PrCore::Math::vec3 m_A1Pos = { 0.801838f , PieceHeight, 0.316983f };
	};
}
