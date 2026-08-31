#include "ChessEngine/BoardCoordinator.h"

#include "Engine/Core/Utils/ILogger.h"

#include "Engine/Core/ECS/Components/TransformComponent.h"
#include "Engine/Core/ECS/Components/RendererComponents.h"
#include "Engine/Core/ECS/Components/CoreComponents.h"

namespace ChessGame {

	BoardCoordinator::BoardCoordinator(PrCore::EntityViewer* p_entityViewer)
	{
		m_squareOverlays.resize(64);
		m_pieces.reserve(32);
		std::fill(m_piecesBySquare.begin(), m_piecesBySquare.end(), nullptr);

		for (auto [entity, nameComponent, transformComponent] : p_entityViewer->EntitesWithComponents<PrCore::NameComponent, PrCore::TransformComponent>())
		{
			std::string_view name = nameComponent->name;
			
			// Collect Square Overlays
			if (name.find("Square_Overlay_") != name.npos)
			{
				int squarePos = name.find_last_of('_');
				std::string_view squareName = name.substr(squarePos + 1);

				int squareIndex = squareName[0] - 'A' + 8 * (squareName[1] - '1');
				m_squareOverlays[squareIndex] = entity;
			}

			// Collect Pieces
			if (name.find("Piece_") != name.npos)
			{
				auto pieceName = name.substr(6);
				std::string overlayName{ pieceName };
				overlayName += "_Overlay";

				Piece piece;
				piece.square = GetSquare(transformComponent->GetPosition());
				piece.type = PieceNameToType(pieceName);
				piece.pieceEntity = entity;
				piece.pieceOverlayEntity = p_entityViewer->GetEntityByName(overlayName);

				m_pieces.push_back(piece);
				m_piecesBySquare[static_cast<int>(piece.square)] = &(m_pieces.back());
			}
		}

		// Validation
		PR_ASSERT(m_squareOverlays.size() == 64, "Square Overlay not scanned properly!");
		PR_ASSERT(m_pieces.size() == 32, "Pieces not scanned properly!");
	}

	void BoardCoordinator::MovePiece(Square p_moveFrom, Square p_moveTo)
	{

		auto fromPiece = m_piecesBySquare[static_cast<int>(p_moveFrom)];
		auto toPiece = m_piecesBySquare[static_cast<int>(p_moveTo)];

		fromPiece->pieceEntity.GetComponent<PrCore::TransformComponent>()->SetPosition(GetSquarePos(p_moveTo));
		fromPiece->square = p_moveTo;

		if (toPiece)
		{
			toPiece->pieceEntity.Destroy();
		}

		m_piecesBySquare[static_cast<int>(p_moveTo)] = fromPiece;
		m_piecesBySquare[static_cast<int>(p_moveFrom)] = nullptr;
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

	Square BoardCoordinator::GetSqureByEntity(PrCore::Entity p_entity)
	{
		auto find = std::find_if(m_pieces.begin(), m_pieces.end(), [=](Piece& piece) {
			return  piece.pieceEntity == p_entity;
			});

		if (find == m_pieces.end())
			return Square::NoSquare;

		return find->square;
	}

	PieceType BoardCoordinator::PieceNameToType(std::string_view p_name)
	{
		int pos = p_name.find_first_of('_');

		auto pieceName = p_name.substr(0, pos);
		auto pieceSide = p_name.substr(pos + 1);
		int sideOffest = pieceSide[0] == 'B' ? static_cast<int>(PieceType::BlackPawn) : 0;

		PieceType pieceType;
		if (pieceName == "Pawn")
			pieceType = PieceType::WhitePawn;
		else if (pieceName == "Knight")
			pieceType = PieceType::WhiteKnight;
		else if (pieceName == "Bishop")
			pieceType = PieceType::WhiteBishop;
		else if (pieceName == "Castle")
			pieceType = PieceType::WhiteRook;
		else if (pieceName == "Queen")
			pieceType = PieceType::WhiteQueen;
		else if (pieceName == "King")
			pieceType = PieceType::WhiteKing;


		return static_cast<PieceType>(static_cast<int>(pieceType) + sideOffest);
	}
}