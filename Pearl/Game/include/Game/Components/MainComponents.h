#pragma once

#include "ChessEngine/ChessSystem.h"

#include "Engine/Core/ECS/BaseComponent.h"
#include "Engine/Core/ECS/Components/RendererComponents.h"
#include "Engine/Core/ECS/Components/PhysicsComponents.h"

namespace ChessGame
{
	// Chess piece placed on the board, added to the piece root entity
	class PieceComponent : public PrCore::BaseComponent {
	public:
		PieceType  type = PieceType::Invalid;
		Color      color = Color::White;
		Square     square = Square::NoSquare;
	};

	// Highlight of the selected piece, added to the piece overlay entity
	// The overlay physics shape is detached while the component exists
	class OverlayComponent : public PrCore::BaseComponent
	{
	public:
		float                time = 0.0f;
		PrPhysics::IShapePtr shapeCache = nullptr;
	};

	// Board square overlay showing possible moves
	class SquareComponent : public PrCore::BaseComponent
	{
	public:
		bool     showOverlay = false;
		MoveType moveType = MoveType::Normal;
		Square   square = Square::NoSquare;
	};
}
