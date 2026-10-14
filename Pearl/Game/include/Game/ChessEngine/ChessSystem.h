#pragma once

#include "ChessLib.hpp"

#include "Core/ECS/ECS.h"

#include <string_view>

namespace ChessGame {
	enum class PieceType {
		Pawn,
		Knight,
		Bishop,
		Rook,
		Queen,
		King,
		Invalid
	};

	enum class Color
	{
		White,
		Black
	};

	enum class Square {
		A1, B1, C1, D1, E1, F1, G1, H1,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A8, B8, C8, D8, E8, F8, G8, H8,
		NoSquare
	};

	enum class MoveType {
		Normal,
		Capture,
		Promotion,
		Castling,
		EnPassant
	};

	enum class GameOverResult
	{
		Checkmate,
		Stalemate,
		InsufficientMaterial,
		FiftyMoveRule,
		ThreefoldRepetition,
		None
	};

	enum class GameOverType
	{
		Win,
		Lose,
		Draw,
		None
	};

	struct Move 
	{
		Square    toSquare = Square::NoSquare;
		Square    fromSquare = Square::NoSquare;
		MoveType  moveType = MoveType::Normal;
		PieceType promotedTo = PieceType::Queen; // Used only by MoveType::Promotion
	};

	struct Piece 
	{
		PieceType type = PieceType::Invalid;
		Color     color = Color::White;
		Square    square = Square::NoSquare;
	};

	using LegalMoves = std::vector<Move>;

	// Thin wrapper over chess-library, translates game types to the library types
	class ChessSystem {
	public:
		ChessSystem();

		void MakeMove(const Move& p_move);
		void GetLegalMoves(LegalMoves& p_outMoves) const;

		Color SideToMove() const;

		bool           IsGameOver() const;
		GameOverType   GetGameOverType() const;   // From the side to move perspective
		GameOverResult GetGameOverResult() const;

		bool           IsCheck() const;

		Piece          PieceAt(Square p_square) const;

		void           ResetBoard();
		void           LoadBoard(std::string_view p_fen);
		std::string    SaveBoard() const;

		std::array<Piece, 64> GetAllPieces() const;

	private:
		chess::Board m_board;
	};
}