#include "ChessEngine/BoardRender.h"

#include "Engine/Core/ECS/Components/RendererComponents.h"
#include "Engine/Core/ECS/Components/TransformComponent.h"
#include "Engine/Core/ECS/Components/PhysicsComponents.h"

namespace ChessGame {
	void BoardRender::ShowMoveHollow(PrCore::Math::vec3 p_pos)
	{
		auto transform = m_selected.GetComponent<PrCore::TransformComponent>();
		p_pos.y = 0.1f;
		transform->SetPosition(p_pos);
	}

	void BoardRender::DisableMoveHollow()
	{
		auto transform = m_selected.GetComponent<PrCore::TransformComponent>();
		transform->SetPosition(m_selectedOrigin);
	}

	void BoardRender::Select(PrCore::Entity p_entity)
	{
		m_selected = p_entity.GetComponent<PrCore::ParentComponent>()->GetParent();
		m_selectedOrigin = m_selected.GetComponent<PrCore::TransformComponent>()->GetPosition();
	}

	void BoardRender::Deselect()
	{
		DisableMoveHollow();
		m_selected = PrCore::Entity{};
	}
}