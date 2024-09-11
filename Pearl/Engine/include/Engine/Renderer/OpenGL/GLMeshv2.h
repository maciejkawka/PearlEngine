#pragma once

#include"Renderer/Resources/Meshv2.h"

namespace PrRenderer::OpenGL {

	class GLMeshv2 : public Resources::Meshv2 {
	public:
		void Bind() override;
		void Unbind() override;

		void RecalculateNormals() override;
		void RecalculateTangents() override;

		size_t GetByteSize() const override;

	private:
		void UpdateBuffers() override;
	};
}
