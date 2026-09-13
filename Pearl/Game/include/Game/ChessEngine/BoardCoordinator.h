#pragma once

#include "ChessEngine/ChessSystem.h"
#include "Components/MainComponents.h"

#include "Engine/Core/ECS/EntityViewer.h"
#include "Engine/Core/Utils/Singleton.h"

#include <vector>

namespace ChessGame {

	enum GameRules
	{
		SinglePlayer,
		Multiplayer
	};

	class BoardCoordinator : public PrCore::Utils::Singleton<BoardCoordinator> {
	public:
		BoardCoordinator() = delete;
		BoardCoordinator(PrCore::EntityViewer* p_entityViewer);

		bool TryMovePiece(Square from, Square to);

		void GenerateMoves();
		void GetMoves(LegalMoves& p_legalMoves);

		void ShowPossibleMovesFor(Square from);
		void DisablePossibleMoves();

		// Updates all pieces, positions capture ect.
		void SetBoardDirty();
		void UpdatePieces();
		void UpdateGameState();

		Color SideToMove();

		void Save();
		void Load();
		void Reset();

		void      SetGameRules(GameRules rules);
		GameRules GetGameRules();
		void      SetPlayerColor(Color color);
		Color     GetPlayerColor();

		// Helpers
		PrCore::Math::vec3 GetSquarePos(Square p_square);
		Square             GetSquare(PrCore::Math::vec3 p_pos);
		PieceType          PieceNameToType(std::string_view p_name);
		Color              PieceNameToColor(std::string_view p_name);

	private:
		using PiecesArray =  std::array<PieceComponent*, 64>;
		using SquaresArray = std::array<SquareComponent*, 64>;

		PiecesArray  m_pieces;
		SquaresArray m_squares;
		LegalMoves   m_legalMoves;
		GameRules    m_gameRules;
		bool         m_isDirty;

		ChessSystem           m_system;
		PrCore::EntityViewer* m_entityViewer;

		// Helper constants
		float              m_squareStep = 0.25135f;
		PrCore::Math::vec3 m_A1Pos = { 0.801838f , 0.064655f, 0.316983f };
		PrCore::Math::vec3 m_gravePos = { 10.0f , 10.0f, 10.0f };
	};
}