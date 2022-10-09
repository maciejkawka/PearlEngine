#pragma once

#include"Renderer/Core/Renderer3D.h"

namespace PrEditor::Components {

	class TestFeatures {
	public:
		TestFeatures();

		~TestFeatures();

		void Update(float p_deltaTime);

	private:
		PrRenderer::Resources::MeshPtr m_mesh;
		float dir = 0.0f;
	};
}