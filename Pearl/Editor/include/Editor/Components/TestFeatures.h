#pragma once

#include"Renderer/Core/Renderer3D.h"

namespace PrEditor::Components {

	class TestFeatures {
	public:
		TestFeatures(PrRenderer::Core::Renderer3D* p_renderer3D);

		~TestFeatures();

		void Update(float p_deltaTime);

	private:
		PrRenderer::Core::Renderer3D* m_renderer3D;
		PrRenderer::Resources::MeshPtr m_mesh;
		float dir = 0.0f;
	};
}