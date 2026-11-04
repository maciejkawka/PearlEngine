#include "Systems/GameOverSystem.h"

#include "ChessEngine/ChessEvents.h"
#include "Systems/MainComponents.h"

#include "Engine/Core/ECS/Components/CoreComponents.h"
#include "Engine/Core/ECS/Components/TransformComponent.h"
#include "Engine/Core/Events/EventManager.h"

namespace ChessGame {

	namespace {
		constexpr float ToppleDuration = 1.0f;   // Seconds
		constexpr float ToppleAngle    = 90.0f;  // Degrees
		constexpr float ToppleLift     = 0.03f;  // Keeps the lying king above the board

		// Board ranks go along the x axis, the king falls along the files
		const PrCore::Math::vec3 ToppleAxis{ 1.0f, 0.0f, 0.0f };

		Color Opposite(Color p_color)
		{
			return p_color == Color::White ? Color::Black : Color::White;
		}
	}

	void GameOverSystem::OnCreate()
	{
		auto eventManager = PrSystems::Get<PrCore::EventManager>();

		PrCore::EventListener gameOverListener;
		gameOverListener.Connect<&GameOverSystem::OnGameOver>(this);
		eventManager->AddListener(gameOverListener, GameOverEvent::s_type);

		PrCore::EventListener boardChangedListener;
		boardChangedListener.Connect<&GameOverSystem::OnBoardChanged>(this);
		eventManager->AddListener(boardChangedListener, BoardChangedEvent::s_type);
	}

	void GameOverSystem::OnUpdate(float p_dt)
	{
		if (!m_toppledKing.IsValid() || m_toppleTime >= ToppleDuration)
			return;

		m_toppleTime = PrCore::Math::min(m_toppleTime + p_dt, ToppleDuration);

		// Smooth step easing
		const float t = m_toppleTime / ToppleDuration;
		const float eased = t * t * (3.0f - 2.0f * t);

		const auto toppledRotation = PrCore::Math::angleAxis(PrCore::Math::radians(ToppleAngle), ToppleAxis) * m_toppleStartRotation;

		auto transform = m_toppledKing.GetComponent<PrCore::TransformComponent>();
		transform->SetRotation(PrCore::Math::slerp(m_toppleStartRotation, toppledRotation, eased));
		transform->SetPosition(m_toppleStartPosition + PrCore::Math::vec3{ 0.0f, ToppleLift * eased, 0.0f });
	}

	void GameOverSystem::OnGameOver(PrCore::EventPtr p_event)
	{
		auto gameOverEvent = std::static_pointer_cast<GameOverEvent>(p_event);
		if (gameOverEvent->m_type != GameOverType::Win)
			return;

		m_toppledKing = FindKing(Opposite(gameOverEvent->m_winner));
		if (!m_toppledKing.IsValid())
			return;

		auto transform = m_toppledKing.GetComponent<PrCore::TransformComponent>();
		m_toppleStartRotation = transform->GetRotation();
		m_toppleStartPosition = transform->GetPosition();
		m_toppleTime = 0.0f;
	}

	void GameOverSystem::OnBoardChanged(PrCore::EventPtr p_event)
	{
		// Board synchronization restores the king position and rotation
		m_toppledKing = PrCore::Entity{};
		m_toppleTime = 0.0f;
	}

	PrCore::Entity GameOverSystem::FindKing(Color p_color)
	{
		for (auto [entity, piece] : m_entityViewer.EntitesWithComponents<PieceComponent>())
		{
			if (piece->type == PieceType::King && piece->color == p_color && !entity.HasComponent<PrCore::ToDestoryTag>())
				return entity;
		}

		return PrCore::Entity{};
	}
}
