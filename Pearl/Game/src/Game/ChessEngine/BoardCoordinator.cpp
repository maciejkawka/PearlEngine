#include "ChessEngine/BoardCoordinator.h"
#include "ChessEngine/ChessEvents.h"
#include "SceneCreators/PiecesFactory.h"

#include "Engine/Core/ECS/Components/TransformComponent.h"
#include "Engine/Core/ECS/Components/RendererComponents.h"
#include "Engine/Core/ECS/Components/CoreComponents.h"
#include "Engine/Core/Events/EventManager.h"
#include "Engine/Core/File/FileSystem.h"
#include "Engine/Core/Utils/ILogger.h"

#include <string>
#include <algorithm>

namespace ChessGame {

	const char* ToString(GameOverResult p_result)
	{
		switch (p_result)
		{
		case GameOverResult::Checkmate:            return "checkmate";
		case GameOverResult::Stalemate:            return "stalemate";
		case GameOverResult::InsufficientMaterial: return "insufficient material";
		case GameOverResult::FiftyMoveRule:        return "fifty move rule";
		case GameOverResult::ThreefoldRepetition:  return "threefold repetition";
		default:                                   return "unknown";
		}
	}

	const char* ToString(Color p_color)
	{
		return p_color == Color::White ? "White" : "Black";
	}

	Color Opposite(Color p_color)
	{
		return p_color == Color::White ? Color::Black : Color::White;
	}

	void Fire(PrCore::EventPtr p_event)
	{
		PrSystems::Get<PrCore::EventManager>()->FireEvent(p_event);
	}

	BoardCoordinator::BoardCoordinator(PrCore::EntityViewer* p_entityViewer) :
		m_entityViewer(p_entityViewer)
	{
		std::fill(m_pieces.begin(), m_pieces.end(), nullptr);
		std::fill(m_squares.begin(), m_squares.end(), nullptr);

		int collectedSquares = 0;
		for (auto [entity, nameComponent] : p_entityViewer->EntitesWithComponents<PrCore::NameComponent>())
		{
			std::string_view name = nameComponent->name;

			// Collect Square Overlays, names end with the square e.g. Square_Overlay_A1
			if (name.find(SquareOverlayPrefix) != name.npos)
			{
				auto squarePos = name.find_last_of('_');
				std::string_view squareName = name.substr(squarePos + 1);

				bool isValidName = squareName.size() == 2 &&
					squareName[0] >= 'A' && squareName[0] <= 'H' &&
					squareName[1] >= '1' && squareName[1] <= '8';

				PR_ASSERT(isValidName, "Invalid square overlay name " + std::string(name));
				if (isValidName)
				{
					int squareIndex = squareName[0] - 'A' + BoardFilesCount * (squareName[1] - '1');
					PR_ASSERT(m_squares[squareIndex] == nullptr, "Duplicated square overlay " + std::string(name));

					m_squares[squareIndex] = entity.AddComponent<SquareComponent>();
					m_squares[squareIndex]->square = static_cast<Square>(squareIndex);
					collectedSquares++;
				}
			}
			// Pieces placed in the scene are replaced by the pieces spawned from the board state
			else if (name.find(ScenePiecePrefix) != name.npos)
			{
				entity.Destroy();
			}
		}

		// Validation
		PR_ASSERT(collectedSquares == BoardSquaresCount, "Square Overlay not scanned properly!");

		// Reset and update board
		Reset();
		UpdatePieces();
	}

	bool BoardCoordinator::TryMovePiece(Square p_from, Square p_to, PieceType p_promotion)
	{
		if (p_from == p_to || m_gameState != GameState::Playing)
			return false;

		for (const auto& move : m_legalMoves)
		{
			if (move.fromSquare != p_from || move.toSquare != p_to)
				continue;

			// Each promotion target square has a move per piece type
			if (move.moveType == MoveType::Promotion && move.promotedTo != p_promotion)
				continue;

			const Move appliedMove = move;
			const Color movedSide = m_system.SideToMove();

			m_system.MakeMove(appliedMove);
			m_legalMoves.clear();
			SetBoardDirty();

			Fire(std::make_shared<MoveMadeEvent>(appliedMove, movedSide));
			return true;
		}

		return false;
	}

	void BoardCoordinator::GenerateMoves()
	{
		m_legalMoves.clear();
		m_system.GetLegalMoves(m_legalMoves);
	}

	const LegalMoves& BoardCoordinator::GetLegalMoves() const
	{
		return m_legalMoves;
	}

	void BoardCoordinator::ShowPossibleMovesFor(Square p_from)
	{
		for (const auto& move : m_legalMoves)
		{
			if (move.fromSquare == p_from)
			{
				SquareComponent* squareComponent = m_squares[static_cast<int>(move.toSquare)];
				squareComponent->showOverlay = true;
				squareComponent->moveType = move.moveType;
			}
		}
	}

	void BoardCoordinator::DisablePossibleMoves()
	{
		for (SquareComponent* squareOverlay : m_squares)
		{
			squareOverlay->showOverlay = false;
		}
	}

	void BoardCoordinator::SetBoardDirty()
	{
		m_isDirty = true;
	}

	void BoardCoordinator::UpdatePieces()
	{
		if (m_isDirty)
		{
			SyncPiecesWithBoard();
			DisablePossibleMoves();
			UpdateGameState();
			m_isDirty = false;
		}
	}

	void BoardCoordinator::SyncPiecesWithBoard()
	{
		struct PieceEntity
		{
			PrCore::Entity  entity;
			PieceComponent* piece;
		};

		const auto boardPieces = m_system.GetAllPieces();
		std::array<bool, BoardSquaresCount> filledSquares{};
		std::fill(m_pieces.begin(), m_pieces.end(), nullptr);

		auto isSamePiece = [](const Piece& p_boardPiece, const PieceComponent* p_piece)
		{
			return p_boardPiece.type == p_piece->type && p_boardPiece.color == p_piece->color;
		};

		// Gather all alive piece entities
		std::vector<PieceEntity> unmatched;
		for (auto [entity, pieceComponent] : m_entityViewer->EntitesWithComponents<PieceComponent>())
		{
			if (!entity.HasComponent<PrCore::ToDestoryTag>())
				unmatched.push_back({ entity, pieceComponent });
		}

		// Keep pieces that already stand on the correct square
		for (auto it = unmatched.begin(); it != unmatched.end();)
		{
			if (it->piece->square != Square::NoSquare)
			{
				int squareIndex = static_cast<int>(it->piece->square);
				if (!filledSquares[squareIndex] && isSamePiece(boardPieces[squareIndex], it->piece))
				{
					PlacePiece(it->entity, it->piece, it->piece->square);
					filledSquares[squareIndex] = true;
					it = unmatched.erase(it);
					continue;
				}
			}

			++it;
		}

		// Move pieces to their new squares, covers normal moves and castling
		for (int i = 0; i < BoardSquaresCount; i++)
		{
			if (filledSquares[i] || boardPieces[i].type == PieceType::Invalid)
				continue;

			auto it = std::find_if(unmatched.begin(), unmatched.end(), [&](const PieceEntity& p_pieceEntity)
			{
				return isSamePiece(boardPieces[i], p_pieceEntity.piece);
			});

			if (it != unmatched.end())
			{
				PlacePiece(it->entity, it->piece, static_cast<Square>(i));
				filledSquares[i] = true;
				unmatched.erase(it);
			}
		}

		// Destroy pieces that left the board, covers captures, en passant and promoted pawns
		for (auto& pieceEntity : unmatched)
		{
			pieceEntity.piece->square = Square::NoSquare;
			pieceEntity.entity.Destroy();
		}

		// Spawn missing pieces, covers promotions and the new board
		PiecesFactory factory;
		for (int i = 0; i < BoardSquaresCount; i++)
		{
			if (filledSquares[i] || boardPieces[i].type == PieceType::Invalid)
				continue;

			auto entity = factory.CreatePieceEntity(boardPieces[i].type, boardPieces[i].color);
			PlacePiece(entity, entity.GetComponent<PieceComponent>(), static_cast<Square>(i));
		}
	}

	void BoardCoordinator::PlacePiece(PrCore::Entity p_entity, PieceComponent* p_piece, Square p_square)
	{
		p_piece->square = p_square;
		m_pieces[static_cast<int>(p_square)] = p_piece;
		p_entity.GetComponent<PrCore::TransformComponent>()->SetPosition(GetSquarePos(p_square));
	}

	void BoardCoordinator::UpdateGameState()
	{
		if (m_gameState == GameState::GameOver || !m_system.IsGameOver())
			return;

		m_gameState = GameState::GameOver;

		const GameOverResult result = m_system.GetGameOverResult();
		const GameOverType sideToMoveResult = m_system.GetGameOverType();

		// Library reports the result from the side to move perspective
		const Color sideToMove = m_system.SideToMove();
		GameOverType type = GameOverType::Draw;
		Color winner = sideToMove;
		if (sideToMoveResult == GameOverType::Lose)
		{
			type = GameOverType::Win;
			winner = Opposite(sideToMove);
		}
		else if (sideToMoveResult == GameOverType::Win)
		{
			type = GameOverType::Win;
		}

		if (type == GameOverType::Win)
		{
			PRLOG_INFO("Game over, {} wins by {}", ToString(winner), ToString(result));
		}
		else
		{
			PRLOG_INFO("Game over, draw by {}", ToString(result));
		}

		Fire(std::make_shared<GameOverEvent>(result, type, winner));
	}

	Color BoardCoordinator::SideToMove() const
	{
		return m_system.SideToMove();
	}

	bool BoardCoordinator::IsCheck() const
	{
		return m_system.IsCheck();
	}

	GameState BoardCoordinator::GetGameState() const
	{
		return m_gameState;
	}

	bool BoardCoordinator::CanSelectPiece(Color p_pieceColor) const
	{
		// Extend here with player color and AI turn rules
		return m_gameState == GameState::Playing && p_pieceColor == m_system.SideToMove();
	}

	void BoardCoordinator::Save()
	{
		auto fileSystem = PrSystems::Get<PrCore::FileSystem>();
		auto filehandle = fileSystem->FileOpen(SaveFilePath, PrCore::FileOpenMode::Write);
		if (!filehandle)
		{
			PRLOG_WARN("Cannot open save file {}, save skipped", SaveFilePath);
			return;
		}

		std::string fen = m_system.SaveBoard();
		fileSystem->FileWrite(filehandle, fen.data(), fen.length());
		fileSystem->FileClose(filehandle);

		PRLOG_INFO("Game saved: {}", fen);
	}

	void BoardCoordinator::Load()
	{
		// Read the save file first, keep the current game if there is nothing to load
		auto fileSystem = PrSystems::Get<PrCore::FileSystem>();
		auto filehandle = fileSystem->FileOpen(SaveFilePath, PrCore::FileOpenMode::Read);
		if (!filehandle)
		{
			PRLOG_WARN("Cannot open save file {}, load skipped", SaveFilePath);
			return;
		}

		std::string fen;
		fen.resize(fileSystem->FileSize(filehandle));
		fileSystem->FileRead(filehandle, fen.data(), fen.length());
		fileSystem->FileClose(filehandle);

		// Trim whitespaces, a manually edited save file can end with a new line
		const char* whitespaces = " \t\r\n";
		auto first = fen.find_first_not_of(whitespaces);
		if (first == std::string::npos)
		{
			PRLOG_WARN("Save file {} is empty, load skipped", SaveFilePath);
			return;
		}
		fen = fen.substr(first, fen.find_last_not_of(whitespaces) - first + 1);

		m_system.LoadBoard(fen);
		OnBoardReplaced();

		PRLOG_INFO("Game loaded: {}", fen);
	}

	void BoardCoordinator::Reset()
	{
		m_system.ResetBoard();
		OnBoardReplaced();
	}

	void BoardCoordinator::OnBoardReplaced()
	{
		m_legalMoves.clear();
		m_gameState = GameState::Playing;
		SetBoardDirty();

		Fire(std::make_shared<BoardChangedEvent>());
	}

	void BoardCoordinator::SetGameRules(GameRules p_rules)
	{
		m_gameRules = p_rules;
	}

	GameRules BoardCoordinator::GetGameRules() const
	{
		return m_gameRules;
	}

	void BoardCoordinator::SetPlayerColor(Color p_color)
	{
		m_playerColor = p_color;
	}

	Color BoardCoordinator::GetPlayerColor() const
	{
		return m_playerColor;
	}

	PrCore::Math::vec3 BoardCoordinator::GetSquarePos(Square p_square) const
	{
		int zOffset = static_cast<int>(p_square) % BoardFilesCount;
		int xOffset = (static_cast<int>(p_square) - zOffset) / BoardFilesCount;

		auto returnPos = m_A1Pos;
		returnPos.x -= xOffset * m_squareStep;
		returnPos.z -= zOffset * m_squareStep;

		return returnPos;
	}
}
