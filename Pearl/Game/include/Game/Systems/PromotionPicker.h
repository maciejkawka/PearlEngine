#pragma once

#include "ChessEngine/ChessSystem.h"

#include "Engine/Core/ECS/EntityManager.h"
#include "Engine/Core/Math/Math.h"

#include <array>

namespace ChessGame {

	// Lets the player choose the promotion piece.
	// Shows a preview piece over the promotion square, the choice is cycled and confirmed by the owner.
	class PromotionPicker {
	public:
		void Start(Square p_from, Square p_to, Color p_color, const PrCore::Math::vec3& p_previewPosition);
		void Cancel();

		void Next();
		void Previous();

		bool      IsActive() const { return m_isActive; }
		Square    GetFromSquare() const { return m_from; }
		Square    GetToSquare() const { return m_to; }
		PieceType GetChoice() const;

	private:
		void ShowPreview();
		void DestroyPreview();

		static constexpr std::array<PieceType, 4> s_choices{ PieceType::Queen, PieceType::Knight, PieceType::Rook, PieceType::Bishop };

		bool               m_isActive = false;
		size_t             m_choiceIndex = 0;
		Square             m_from = Square::NoSquare;
		Square             m_to = Square::NoSquare;
		Color              m_color = Color::White;
		PrCore::Math::vec3 m_previewPosition{ 0.0f };
		PrCore::Entity     m_preview;
	};
}
