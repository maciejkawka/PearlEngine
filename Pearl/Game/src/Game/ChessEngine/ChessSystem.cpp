#include "ChessEngine/ChessSystem.h"


namespace ChessGame {

	Square ToSquare(chess::Square p_square)
	{
		return static_cast<Square>(p_square.index());
	}

	PieceType ToPieceType(chess::PieceType p_pieceType)
	{
		int undelying = static_cast<int>(p_pieceType);
		return static_cast<PieceType>(undelying);
	}

	chess::Square ToChessSquare(Square p_square)
	{
		return chess::Square{ static_cast<int>(p_square) };
	}

	chess::PieceType ToChessPieceType(PieceType p_pieceType)
	{
		int underlying = static_cast<int>(p_pieceType);
		return static_cast<chess::PieceType::underlying>(underlying);
	}

	ChessSystem::ChessSystem() :
		m_board()
	{
	}

	void ChessSystem::MakeMove(Move p_move)
	{
		chess::Move move = chess::Move::make(
			ToChessSquare(p_move.fromSquare),
			ToChessSquare(p_move.toSquare),
			ToChessPieceType(p_move.pieceType)
		);

		m_board.makeMove(move);
	}

	void ChessSystem::GetLegalMoves(LegalMoves& p_outMoves)
	{
		chess::Movelist moveList;
		chess::movegen::legalmoves(moveList, m_board);


		for (auto& legalMove : moveList)
		{
			Move move;
			move.fromSquare = ToSquare(legalMove.from());
			move.toSquare = ToSquare(legalMove.to());
			move.pieceType = ToPieceType(legalMove.promotionType());

			if (m_board.isCapture(legalMove))
				move.moveType = MoveType::Capture;
			else if (legalMove.typeOf() == chess::Move::PROMOTION)
				move.moveType = MoveType::Promotion;
			else
				move.moveType = MoveType::Normal;

			p_outMoves.push_back(std::move(move));
		}
	}

	Color ChessSystem::SideToMove()
	{
		auto color = static_cast<int>(m_board.sideToMove());
		return static_cast<Color>(color);
	}

	bool ChessSystem::IsGameOver()
	{
		return m_board.isGameOver().first != chess::GameResultReason::NONE;
	}

	GameOverType ChessSystem::GetGameOverType()
	{
		auto underlying = static_cast<int>(m_board.isGameOver().first);
		return static_cast<GameOverType>(underlying);
	}

	GameOverResult ChessSystem::GetGameOverResult()
	{
		auto underlying = static_cast<int>(m_board.isGameOver().second);
		return static_cast<GameOverResult>(underlying);
	}

	bool ChessSystem::IsCheck()
	{
		return m_board.inCheck();
	}

	bool ChessSystem::IsCapture(Move p_move)
	{
		chess::Move move = chess::Move::make(
			ToChessSquare(p_move.fromSquare),
			ToChessSquare(p_move.toSquare),
			ToChessPieceType(p_move.pieceType)
		);

		return m_board.isCapture(move);
	}

	Piece ChessSystem::PieceAt(Square p_square)
	{
		chess::Piece chessPiece = m_board.at(ToChessSquare(p_square));

		Piece piece;
		piece.type = PieceType::Invalid;
		piece.square = Square::NoSquare;

		if (chessPiece.type() != chess::PieceType::NONE)
		{
			auto color = static_cast<int>(chessPiece.color());
			piece.color = static_cast<Color>(color);

			piece.type = ToPieceType(chessPiece.type());
			piece.square = p_square;
		}

		return piece;
	}

	void ChessSystem::ResetBoard()
	{
		m_board = chess::Board();
	}

	void ChessSystem::LoadBoard(std::string_view p_fen)
	{
		m_board = m_board.fromFen(p_fen);
	}

	std::string ChessSystem::SaveBoard()
	{
		return m_board.getFen();
	}

	Square ChessSystem::GetPiecePos(PieceType type, Color color)
	{
		return Square::NoSquare;
	}

	std::array<Piece, 64> ChessSystem::GetAllPieces()
	{
		std::array<Piece, 64> pieces;

		for (int i = 0; i < 64; i++)
		{
			chess::Piece chessPiece = m_board.at(chess::Square{ i });

			Piece piece;
			piece.type = PieceType::Invalid;

			if (chessPiece.type() != chess::PieceType::NONE)
			{
				auto color = static_cast<int>(chessPiece.color());
				piece.color = static_cast<Color>(color);

				piece.type = ToPieceType(chessPiece.type());
				piece.square = static_cast<Square>(i);
			}

			pieces[i] = std::move(piece);
		}

		return pieces;
	}
}
