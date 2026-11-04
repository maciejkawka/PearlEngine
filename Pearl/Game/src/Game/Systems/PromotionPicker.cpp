#include "Systems/PromotionPicker.h"

#include "SceneCreators/PiecesFactory.h"

#include "Engine/Core/ECS/Components/TransformComponent.h"
#include "Engine/Core/Utils/ILogger.h"

namespace ChessGame {

	namespace {
		const char* ToString(PieceType p_type)
		{
			switch (p_type)
			{
			case PieceType::Queen:  return "Queen";
			case PieceType::Knight: return "Knight";
			case PieceType::Rook:   return "Rook";
			case PieceType::Bishop: return "Bishop";
			default:                return "Invalid";
			}
		}
	}

	void PromotionPicker::Start(Square p_from, Square p_to, Color p_color, const PrCore::Math::vec3& p_previewPosition)
	{
		Cancel();

		m_isActive = true;
		m_choiceIndex = 0;
		m_from = p_from;
		m_to = p_to;
		m_color = p_color;
		m_previewPosition = p_previewPosition;

		ShowPreview();
		PRLOG_INFO("Choose promotion: Left/Right or Tab to change, Enter or left click to confirm, Esc, G or right click to cancel");
	}

	void PromotionPicker::Cancel()
	{
		DestroyPreview();

		m_isActive = false;
		m_from = Square::NoSquare;
		m_to = Square::NoSquare;
	}

	void PromotionPicker::Next()
	{
		if (!m_isActive)
			return;

		m_choiceIndex = (m_choiceIndex + 1) % s_choices.size();
		ShowPreview();
	}

	void PromotionPicker::Previous()
	{
		if (!m_isActive)
			return;

		m_choiceIndex = (m_choiceIndex + s_choices.size() - 1) % s_choices.size();
		ShowPreview();
	}

	PieceType PromotionPicker::GetChoice() const
	{
		return s_choices[m_choiceIndex];
	}

	void PromotionPicker::ShowPreview()
	{
		DestroyPreview();

		PiecesFactory factory;
		m_preview = factory.CreatePreviewEntity(GetChoice(), m_color);
		m_preview.GetComponent<PrCore::TransformComponent>()->SetPosition(m_previewPosition);

		PRLOG_INFO("Promotion choice: {}", ToString(GetChoice()));
	}

	void PromotionPicker::DestroyPreview()
	{
		if (m_preview.IsValid())
			m_preview.Destroy();

		m_preview = PrCore::Entity{};
	}
}
