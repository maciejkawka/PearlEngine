#pragma once

namespace PrRenderer::Core {

	class Renderer {
	public:

		Renderer(){}

		void Test();
		void Draw();

	private:
		unsigned int vao;
		unsigned int ebo;
		unsigned int shaderProgram;
	};
}