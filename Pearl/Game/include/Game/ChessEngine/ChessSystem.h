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
		Promotion
	};

	enum class GameOverResult
	{
		Checkmate,
		Stalemate,
		InsuddicientMaterial,
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
		Square    toSquare;
		Square    fromSquare;
		MoveType  moveType;
		PieceType pieceType;
	};

	struct Piece 
	{
		PieceType type;
		Color     color;
		Square    square;
	};

	using LegalMoves = std::vector<Move>;

	class ChessSystem {
	public:
		ChessSystem();

		void MakeMove(Move p_move);
		void GetLegalMoves(LegalMoves& p_outMoves);

		Color SideToMove();

		bool           IsGameOver();
		GameOverType   GetGameOverType();
		GameOverResult GetGameOverResult();

		bool           IsCheck();
		bool           IsCapture(Move p_move);

		Piece          PieceAt(Square p_square);

		void           ResetBoard();
		void           LoadBoard(std::string_view p_fen);
		std::string    SaveBoard();

		Square                GetPiecePos(PieceType type, Color color);
		std::array<Piece, 64> GetAllPieces();

	private:
		chess::Board m_board;
	};
}