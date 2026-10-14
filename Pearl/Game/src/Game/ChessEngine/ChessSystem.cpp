#include "ChessEngine/ChessSystem.h"

namespace ChessGame {

	// Conversions between game and chess-library types
	Square ToSquare(chess::Square p_square)
	{
		return static_cast<Square>(p_square.index());
	}

	PieceType ToPieceType(chess::PieceType p_pieceType)
	{
		switch (p_pieceType.internal())
		{
		case chess::PieceType::PAWN:   return PieceType::Pawn;
		case chess::PieceType::KNIGHT: return PieceType::Knight;
		case chess::PieceType::BISHOP: return PieceType::Bishop;
		case chess::PieceType::ROOK:   return PieceType::Rook;
		case chess::PieceType::QUEEN:  return PieceType::Queen;
		case chess::PieceType::KING:   return PieceType::King;
		default:                       return PieceType::Invalid;
		}
	}

	Color ToColor(chess::Color p_color)
	{
		return p_color == chess::Color::WHITE ? Color::White : Color::Black;
	}

	chess::Square ToChessSquare(Square p_square)
	{
		return chess::Square{ static_cast<int>(p_square) };
	}

	chess::PieceType ToChessPieceType(PieceType p_pieceType)
	{
		switch (p_pieceType)
		{
		case PieceType::Pawn:   return chess::PieceType::PAWN;
		case PieceType::Knight: return chess::PieceType::KNIGHT;
		case PieceType::Bishop: return chess::PieceType::BISHOP;
		case PieceType::Rook:   return chess::PieceType::ROOK;
		case PieceType::Queen:  return chess::PieceType::QUEEN;
		case PieceType::King:   return chess::PieceType::KING;
		default:                return chess::PieceType::NONE;
		}
	}

	ChessSystem::ChessSystem() :
		m_board()
	{
	}

	void ChessSystem::MakeMove(const Move& p_move)
	{
		const auto from = ToChessSquare(p_move.fromSquare);
		const auto to = ToChessSquare(p_move.toSquare);

		// Promotion piece type is encoded only for promotion moves
		chess::Move move;
		switch (p_move.moveType)
		{
		case MoveType::Promotion:
			move = chess::Move::make<chess::Move::PROMOTION>(from, to, ToChessPieceType(p_move.promotedTo));
			break;
		case MoveType::Castling:
			move = chess::Move::make<chess::Move::CASTLING>(from, to);
			break;
		case MoveType::EnPassant:
			move = chess::Move::make<chess::Move::ENPASSANT>(from, to);
			break;
		default:
			move = chess::Move::make(from, to);
			break;
		}

		m_board.makeMove(move);
	}

	void ChessSystem::GetLegalMoves(LegalMoves& p_outMoves) const
	{
		chess::Movelist moveList;
		chess::movegen::legalmoves(moveList, m_board);

		for (auto& legalMove : moveList)
		{
			Move move;
			move.fromSquare = ToSquare(legalMove.from());
			move.toSquare = ToSquare(legalMove.to());

			if (legalMove.typeOf() == chess::Move::PROMOTION)
			{
				move.moveType = MoveType::Promotion;
				move.promotedTo = ToPieceType(legalMove.promotionType());
			}
			else if (legalMove.typeOf() == chess::Move::CASTLING)
			{
				move.moveType = MoveType::Castling;
			}
			else if (legalMove.typeOf() == chess::Move::ENPASSANT)
			{
				move.moveType = MoveType::EnPassant;
			}
			else if (m_board.isCapture(legalMove))
			{
				move.moveType = MoveType::Capture;
			}
			else
			{
				move.moveType = MoveType::Normal;
			}

			p_outMoves.push_back(std::move(move));
		}
	}

	Color ChessSystem::SideToMove() const
	{
		return ToColor(m_board.sideToMove());
	}

	bool ChessSystem::IsGameOver() const
	{
		return m_board.isGameOver().first != chess::GameResultReason::NONE;
	}

	GameOverType ChessSystem::GetGameOverType() const
	{
		switch (m_board.isGameOver().second)
		{
		case chess::GameResult::WIN:  return GameOverType::Win;
		case chess::GameResult::LOSE: return GameOverType::Lose;
		case chess::GameResult::DRAW: return GameOverType::Draw;
		default:                      return GameOverType::None;
		}
	}

	GameOverResult ChessSystem::GetGameOverResult() const
	{
		switch (m_board.isGameOver().first)
		{
		case chess::GameResultReason::CHECKMATE:             return GameOverResult::Checkmate;
		case chess::GameResultReason::STALEMATE:             return GameOverResult::Stalemate;
		case chess::GameResultReason::INSUFFICIENT_MATERIAL: return GameOverResult::InsufficientMaterial;
		case chess::GameResultReason::FIFTY_MOVE_RULE:       return GameOverResult::FiftyMoveRule;
		case chess::GameResultReason::THREEFOLD_REPETITION:  return GameOverResult::ThreefoldRepetition;
		default:                                             return GameOverResult::None;
		}
	}

	bool ChessSystem::IsCheck() const
	{
		return m_board.inCheck();
	}

	Piece ChessSystem::PieceAt(Square p_square) const
	{
		chess::Piece chessPiece = m_board.at(ToChessSquare(p_square));

		Piece piece;
		if (chessPiece.type() != chess::PieceType::NONE)
		{
			piece.color = ToColor(chessPiece.color());
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

	std::string ChessSystem::SaveBoard() const
	{
		return m_board.getFen();
	}

	std::array<Piece, 64> ChessSystem::GetAllPieces() const
	{
		std::array<Piece, 64> pieces;
		for (int i = 0; i < 64; i++)
			pieces[i] = PieceAt(static_cast<Square>(i));

		return pieces;
	}
}
