#pragma once
#include"Renderer/Core/Defines.h"

#include"Core/Utils/Clock.h"
namespace PrRenderer::Core {

	class Renderer {
	public:

		Renderer(){}
		~Renderer();

		void Test();
		void Draw();

	private:
		unsigned int shaderProgram;
		VertexArrayPtr vertexArray;
		PrCore::Utils::Clock m_clock;
	};
}