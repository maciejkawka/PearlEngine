#pragma once

#include "ChessEngine/ChessSystem.h"

#include "Engine/Core/ECS/BaseSystem.h"
#include "Engine/Core/ECS/Components/RendererComponents.h"
#include "Engine/Core/ECS/Components/PhysicsComponents.h"

#include "Engine/Core/Events/EventManager.h"
#include "Engine/Core/Events/ECSEvents.h"

namespace ChessGame
{
	class PieceComponent : PrCore::BaseComponent {
	public:

		PieceType  type;
		Color      color;
		Square     square;
	};

	class OverlayComponent : public PrCore::BaseComponent
	{
	public:
		float                time = 0.0f;
		PieceComponent*      pieceComponent;
		PrPhysics::IShapePtr shapeCache = nullptr;
	};

	class SquareComponent : public PrCore::BaseComponent
	{
	public:
		bool     showOverlay = false;
		MoveType moveType = MoveType::Normal;
		Square   square;
	};
}