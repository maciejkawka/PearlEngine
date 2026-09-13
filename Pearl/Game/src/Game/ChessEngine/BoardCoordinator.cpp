#include "ChessEngine/BoardCoordinator.h"
#include "SceneCreators/PiecesFactory.h"

#include "Engine/Core/ECS/Components/TransformComponent.h"
#include "Engine/Core/ECS/Components/RendererComponents.h"
#include "Engine/Core/ECS/Components/CoreComponents.h"
#include "Engine/Core/File/FileSystem.h"
#include "Engine/Core/Utils/ILogger.h"

#include <string>

namespace ChessGame {

	constexpr std::string_view SaveFilePath = "save.chess";

	BoardCoordinator::BoardCoordinator(PrCore::EntityViewer* p_entityViewer)
	{
		m_system = ChessSystem{};
		m_entityViewer = p_entityViewer;

		std::fill(m_pieces.begin(), m_pieces.end(), nullptr);

		int collectedSquared = 0;
		int collectedPieces = 0;
		for (auto [entity, nameComponent, transformComponent] : p_entityViewer->EntitesWithComponents<PrCore::NameComponent, PrCore::TransformComponent>())
		{
			std::string_view name = nameComponent->name;

			// Collect Square Overlays
			if (name.find("Square_Overlay_") != name.npos)
			{
				collectedSquared++;

				int squarePos = name.find_last_of('_');
				std::string_view squareName = name.substr(squarePos + 1);

				int squareIndex = squareName[0] - 'A' + 8 * (squareName[1] - '1');
				m_squares[squareIndex] = entity.AddComponent<SquareComponent>();
				m_squares[squareIndex]->square = static_cast<Square>(squareIndex);
			}

			// Collect Pieces
			if (name.find("Piece_") != name.npos)
			{
				auto pieceName = name.substr(6);

				PieceComponent* pieceComponent = entity.AddComponent<PieceComponent>();
				pieceComponent->square = GetSquare(transformComponent->GetPosition());
				pieceComponent->type = PieceNameToType(pieceName);
				pieceComponent->color = PieceNameToColor(pieceName);
				m_pieces[static_cast<int>(pieceComponent->square)] = pieceComponent;

				collectedPieces++;
			}
		}

		// Validation
		PR_ASSERT(collectedSquared == 64, "Square Overlay not scanned properly!");
		PR_ASSERT(collectedPieces == 32, "Pieces not scanned properly!");

		// Reset and update board
		Reset();
		UpdatePieces();
	}

	bool BoardCoordinator::TryMovePiece(Square from, Square to)
	{
		if (from == to)
			return false;

		for (auto move : m_legalMoves)
		{
			if (move.fromSquare == from && move.toSquare == to)
			{
				if (m_system.IsCapture(move))
				{
					m_pieces[static_cast<int>(to)]->square = Square::NoSquare;
				}

				m_pieces[static_cast<int>(from)]->square = to;
				std::swap(m_pieces[static_cast<int>(from)], m_pieces[static_cast<int>(to)]);
				m_system.MakeMove(move);

				return true;
			}
		}

		return false;
	}

	void BoardCoordinator::GenerateMoves()
	{
		m_legalMoves.clear();
		m_system.GetLegalMoves(m_legalMoves);
	}

	void BoardCoordinator::GetMoves(LegalMoves& p_legalMoves)
	{
		p_legalMoves = m_legalMoves;
	}

	void BoardCoordinator::ShowPossibleMovesFor(Square from)
	{
		for (auto& move : m_legalMoves)
		{
			if (move.fromSquare == from)
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
			for (auto [entity, pieceComponent, transform, mesh] : m_entityViewer->EntitesWithComponents<PieceComponent, PrCore::TransformComponent, PrCore::MeshRendererComponent>())
			{
				if (pieceComponent->square == Square::NoSquare)
				{
					entity.Destroy();
				}
				else
				{
					transform->SetPosition(GetSquarePos(pieceComponent->square));
				}
			}

			for (auto& piece : m_pieces)
			{
				if (piece && piece->square == Square::NoSquare)
				{
					piece = nullptr;
				}
			}

			m_isDirty = false;
		}
	}

	void BoardCoordinator::UpdateGameState()
	{

	}

	Color BoardCoordinator::SideToMove()
	{
		return m_system.SideToMove();
	}

	void BoardCoordinator::Save()
	{
		std::string fen = m_system.SaveBoard();
		auto filehandle = PrSystems::Get<PrCore::FileSystem>()->FileOpen(SaveFilePath, PrCore::FileOpenMode::Write);
		PrSystems::Get<PrCore::FileSystem>()->FileWrite(filehandle, fen.data(), fen.length());
		PrSystems::Get<PrCore::FileSystem>()->FileClose(filehandle);
	}

	void BoardCoordinator::Load()
	{
		// Destory all pieces
		for (auto [entity, pieceComponent] : m_entityViewer->EntitesWithComponents<PieceComponent>())
		{
			pieceComponent->square = Square::NoSquare;
			entity.Destroy();
		}

		auto filehandle = PrSystems::Get<PrCore::FileSystem>()->FileOpen(SaveFilePath, PrCore::FileOpenMode::Read);
		if (filehandle)
		{
			std::string fen;
			fen.resize(PrSystems::Get<PrCore::FileSystem>()->FileSize(filehandle));
			PrSystems::Get<PrCore::FileSystem>()->FileRead(filehandle, fen.data(), fen.length());
			PrSystems::Get<PrCore::FileSystem>()->FileClose(filehandle);

			PiecesFactory factory;
			std::fill(m_pieces.begin(), m_pieces.end(), nullptr);
			m_system.LoadBoard(fen);

			const auto pieces = m_system.GetAllPieces();
			for (const auto& piece : pieces)
			{
				if (piece.type != PieceType::Invalid)
				{
					auto entity = factory.CreatePieceEntity(piece.type, piece.color);
					auto pieceComponent = entity.GetComponent<PieceComponent>();
					pieceComponent->square = piece.square;
					pieceComponent->color = piece.color;
					pieceComponent->type = piece.type;

					m_pieces[static_cast<int>(pieceComponent->square)] = pieceComponent;
				}
			}
		}

		SetBoardDirty();
	}

	void BoardCoordinator::Reset()
	{
		// Destory all pieces
		for (auto [entity, pieceComponent] : m_entityViewer->EntitesWithComponents<PieceComponent>())
		{
			entity.Destroy();
			pieceComponent->square = Square::NoSquare;
		}

		PiecesFactory factory;
		std::fill(m_pieces.begin(), m_pieces.end(), nullptr);
		m_system = ChessSystem{};

		auto pieces = m_system.GetAllPieces();
		for (auto& piece : pieces)
		{
			if (piece.type != PieceType::Invalid)
			{
				auto entity = factory.CreatePieceEntity(piece.type, piece.color);
				auto pieceComponent = entity.GetComponent<PieceComponent>();
				pieceComponent->square = piece.square;
				pieceComponent->color = piece.color;
				pieceComponent->type = piece.type;

				m_pieces[static_cast<int>(pieceComponent->square)] = pieceComponent;
			}
		}

		SetBoardDirty();
	}

	void BoardCoordinator::SetGameRules(GameRules rules)
	{
		m_gameRules = rules;
	}

	GameRules BoardCoordinator::GetGameRules()
	{
		return m_gameRules;
	}

	PrCore::Math::vec3 BoardCoordinator::GetSquarePos(Square p_square)
	{
		int zOffset = static_cast<int>(p_square) % 8;
		int xOffset = (static_cast<int>(p_square) - zOffset) / 8;


		auto returnPos = m_A1Pos;
		returnPos.x -= xOffset * m_squareStep;
		returnPos.z -= zOffset * m_squareStep;

		return returnPos;
	}

	Square BoardCoordinator::GetSquare(PrCore::Math::vec3 p_pos)
	{
		float x = p_pos.x;
		float z = p_pos.z;

		float xOffset = PrCore::Math::abs((m_A1Pos.x - x) / m_squareStep) + 0.2f;
		float zOffset = PrCore::Math::abs((m_A1Pos.z - z) / m_squareStep) + 0.2f;

		int squareIndex = static_cast<int>(xOffset) * 8 + static_cast<int>(zOffset);
		return static_cast<Square>(squareIndex);
	}

	Color BoardCoordinator::PieceNameToColor(std::string_view p_name)
	{
		int pos = p_name.find_first_of('_');
		auto pieceSide = p_name.substr(pos + 1);
		return pieceSide[0] == 'B' ? Color::Black : Color::White;
	}

	PieceType BoardCoordinator::PieceNameToType(std::string_view p_name)
	{
		int pos = p_name.find_first_of('_');

		auto pieceName = p_name.substr(0, pos);

		PieceType pieceType;
		if (pieceName == "Pawn")
			pieceType = PieceType::Pawn;
		else if (pieceName == "Knight")
			pieceType = PieceType::Knight;
		else if (pieceName == "Bishop")
			pieceType = PieceType::Bishop;
		else if (pieceName == "Castle")
			pieceType = PieceType::Rook;
		else if (pieceName == "Queen")
			pieceType = PieceType::Queen;
		else if (pieceName == "King")
			pieceType = PieceType::King;

		return pieceType;
	}
}