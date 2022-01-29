#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/Renderer.h"
#include"Renderer/OpenGL/GLVertexBuffer.h"
#include"Renderer/OpenGL/GLIndexBuffer.h"
#include"Renderer/OpenGL/GLVertexArray.h"

#include"Renderer/Resources/ShaderManager.h"
#include"Renderer/Resources/Shader.h"

#include"glad/glad.h"

using namespace PrRenderer::Core;

void Renderer::Test()
{
	float vertecies[] = {
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f,  // top right
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,// bottom left
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f  // top left 
	};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	vertexArray = std::make_shared<OpenGL::GLVertexArray>();

	IndexBufferPtr indexBuffer = std::make_shared<OpenGL::GLIndexBuffer>(indices, 6);
	vertexArray->SetIndexBuffer(indexBuffer);


	Buffers::BufferLayout layout = {
		{ "Vertex", Buffers::ShaderDataType::Float3},
		{ "Colour", Buffers::ShaderDataType::Float3}
	};

	VertexBufferPtr vertexBuffer = std::make_shared<OpenGL::GLVertexBuffer>(vertecies, sizeof(vertecies));
	vertexBuffer->SetBufferLayout(layout);

	vertexArray->SetVertexBuffer(vertexBuffer);

	Resources::ShaderPtr shader = std::static_pointer_cast<Resources::Shader>(PrRenderer::Resources::ShaderManager::GetInstance().Load("BasicShader.txt"));
	shader->Bind();
}


void Renderer::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	vertexArray->Bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
