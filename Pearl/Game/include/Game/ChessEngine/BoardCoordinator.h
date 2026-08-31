#pragma once

#include "Engine/Core/ECS/EntityViewer.h"

namespace ChessGame {

	enum class PieceType {
		WhitePawn,
		WhiteKnight,
		WhiteBishop,
		WhiteRook,
		WhiteQueen,
		WhiteKing,
		BlackPawn,
		BlackKnight,
		BlackBishop,
		BlackRook,
		BlackQueen,
		BlackKing,
		Invalid
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

	struct LegalMove
	{
		Square   square = Square::NoSquare;
		MoveType isCapture = MoveType::Normal;
	};

	struct Piece {
		PieceType type = PieceType::Invalid;
		Square    square = Square::NoSquare;

		PrCore::Entity pieceEntity = PrCore::Entity{};
		PrCore::Entity pieceOverlayEntity = PrCore::Entity{};
	};

	using LegalMoves = std::vector<LegalMove>;

	class BoardCoordinator {
	public:
		BoardCoordinator() = default;
		BoardCoordinator(PrCore::EntityViewer* p_entityViewer);

		void ResetBoard();
		void MovePiece(Square p_moveFrom, Square p_moveTo);

		void CaputePieceOnSquare(Square p_square);

		// Helpers
		PrCore::Math::vec3 GetSquarePos(Square p_square);
		Square             GetSquare(PrCore::Math::vec3 p_pos);
		PieceType          PieceNameToType(std::string_view p_name);
		Square             GetSqureByEntity(PrCore::Entity p_entity);

	private:
		using PieceBySquare = std::array<Piece*, 64>;
		using PiecesVec      = std::vector<Piece>;
		using SquareOverlays = std::vector<PrCore::Entity>;

		PieceBySquare  m_piecesBySquare;
		PiecesVec      m_pieces;
		SquareOverlays m_squareOverlays;

		float m_squareStep = 0.25135f;
		PrCore::Math::vec3 m_A1Pos = { 0.801838f , 0.064655f, 0.316983f };
	};
}