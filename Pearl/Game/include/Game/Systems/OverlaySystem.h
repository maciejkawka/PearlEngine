#pragma once

#include "Systems/MainComponents.h"

#include "Engine/Core/ECS/BaseSystem.h"
#include "Engine/Core/Events/Event.h"

namespace ChessGame {

	// Animates piece and square overlays
	// Piece overlay: selected piece pulses, king in check flashes red, kings glow on game over
	// Square overlay: possible move of the selected piece, otherwise the fading last move
	class OverlaySystem : public PrCore::BaseSystem {
	public:
		OverlaySystem() = default;
		~OverlaySystem() = default;

		void OnCreate() override;
		void OnUpdate(float p_dt) override;

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override {}
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override {}

	private:
		float m_time = 0.0f;

		void OnOverlayAdded(PrCore::EventPtr p_event);
		void OnOverlayRemoved(PrCore::EventPtr p_event);
	};
}
