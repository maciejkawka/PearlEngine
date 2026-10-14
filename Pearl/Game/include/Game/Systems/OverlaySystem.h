#pragma once

#include "Components/MainComponents.h"

#include "Engine/Core/ECS/BaseSystem.h"
#include "Engine/Core/Events/Event.h"

namespace ChessGame {

	// Animates the selected piece overlay and colors square overlays by the move type
	class OverlaySystem : public PrCore::BaseSystem {
	public:
		OverlaySystem() = default;
		~OverlaySystem() = default;

		void OnCreate() override;
		void OnUpdate(float p_dt) override;

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override {}
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override {}

	private:
		void OnOverlayAdded(PrCore::EventPtr p_event);
		void OnOverlayRemoved(PrCore::EventPtr p_event);
	};
}
