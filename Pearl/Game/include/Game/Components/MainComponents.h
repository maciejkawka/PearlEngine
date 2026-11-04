#pragma once

#include "ChessEngine/ChessSystem.h"

#include "Engine/Core/ECS/BaseComponent.h"
#include "Engine/Core/ECS/Components/RendererComponents.h"
#include "Engine/Core/ECS/Components/PhysicsComponents.h"

namespace ChessGame
{
	// Piece overlay highlight, set by BoardCoordinator and rendered by OverlaySystem
	enum class PieceHighlight
	{
		None,
		Check,   // King in check, flashes red
		Winner,  // Winning king, glows gold
		Draw     // Both kings on draw, glow blue
	};

	// Chess piece placed on the board, added to the piece root entity
	struct PieceComponent : public PrCore::BaseComponent
	{
		PieceType      type = PieceType::Invalid;
		Color          color = Color::White;
		Square         square = Square::NoSquare;
		PieceHighlight highlight = PieceHighlight::None;
	};

	// Highlight of the selected piece, added to the piece overlay entity
	// The overlay physics shape is detached while the component exists
	struct OverlayComponent : public PrCore::BaseComponent
	{
		float                time = 0.0f;
		PrPhysics::IShapePtr shapeCache = nullptr;
	};

	// Board square overlay showing possible moves and the last move
	// Highlights are set by BoardCoordinator and rendered by OverlaySystem
	struct SquareComponent : public PrCore::BaseComponent
	{
		bool     showOverlay = false;   // Possible move of the selected piece
		MoveType moveType = MoveType::Normal;
		bool     isLastMove = false;    // Source or destination of the last move
		float    lastMoveTime = 0.0f;   // Seconds since the last move highlight was set, drives the fade
		Square   square = Square::NoSquare;
	};
}
