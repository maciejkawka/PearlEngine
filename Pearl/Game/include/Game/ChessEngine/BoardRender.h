#pragma once

#include "Engine/Core/ECS/BaseSystem.h"

namespace ChessGame {

	class BoardRender
	{
	public:
		void ShowMoveHollow(PrCore::Math::vec3 p_pos);
		void DisableMoveHollow();

		void Select(PrCore::Entity p_entity);
		void Deselect();

	private:
		PrCore::Entity      m_selected;
		PrCore::Entity      m_selectedOverlay;
		PrCore::Math::vec3  m_selectedOrigin;
	};
}