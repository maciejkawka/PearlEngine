#pragma once

#include"Renderer/Resources/Mesh.h"

namespace PrRenderer::OpenGL {

	class GLMesh : public Resources::Mesh {
	public:
		void Bind() override;
		void Unbind() override;

		void RecalculateNormals() override;
		void RecalculateTangents() override;

	private:
		void UpdateBuffers() override;
	};
}
