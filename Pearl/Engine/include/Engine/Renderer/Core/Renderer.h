#pragma once
#include"Renderer/Core/Defines.h"

namespace PrRenderer::Core {

	class Renderer {
	public:

		Renderer(){}

		void Test();
		void Draw();

	private:
		unsigned int shaderProgram;
		VertexArrayPtr vertexArray;
	};
}