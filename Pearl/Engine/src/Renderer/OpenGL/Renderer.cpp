#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/Renderer.h"
#include"Renderer/Core/Camera.h"
#include"Renderer/OpenGL/GLVertexBuffer.h"
#include"Renderer/OpenGL/GLIndexBuffer.h"
#include"Renderer/OpenGL/GLVertexArray.h"

#include"Renderer/Resources/ShaderManager.h"
#include"Renderer/Resources/TextureManager.h"
#include"Renderer/Resources/Shader.h"
#include"Renderer/Resources/Texture2D.h"

#include"glad/glad.h"

using namespace PrRenderer::Core;

void Renderer::Test()
{
	float vertecies[] = {
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.5f, 1.5f, // top right
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.5f, 0.5f,  // bottom right
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.5f, 0.5f,// bottom left
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 1.5f  // top left 
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
		{ "Colour", Buffers::ShaderDataType::Float3},
		{ "UVs", Buffers::ShaderDataType::Float2}
	};

	VertexBufferPtr vertexBuffer = std::make_shared<OpenGL::GLVertexBuffer>(vertecies, sizeof(vertecies));
	vertexBuffer->SetBufferLayout(layout);

	vertexArray->SetVertexBuffer(vertexBuffer);

	Resources::ShaderPtr shader = std::static_pointer_cast<Resources::Shader>(PrRenderer::Resources::ShaderManager::GetInstance().Load("TextureShader.shader"));
	Resources::ShaderPtr shader2 = std::static_pointer_cast<Resources::Shader>(PrRenderer::Resources::ShaderManager::GetInstance().Load("SinColour1.shader"));
	Resources::ShaderPtr shader3 = std::static_pointer_cast<Resources::Shader>(PrRenderer::Resources::ShaderManager::GetInstance().Load("BasicShader.shader"));
	Resources::ShaderPtr shader4 = std::static_pointer_cast<Resources::Shader>(PrRenderer::Resources::ShaderManager::GetInstance().Load("CameraShader.shader"));
	shader4->Bind();

	Resources::Texture2DPtr texture = std::static_pointer_cast<Resources::Texture2D>(PrRenderer::Resources::TextureManager::GetInstance().Load("IMG_6572.jpg"));
	texture->Bind();
	shader->SetUniformInt("u_tex", 0);

	auto camera = new PrRenderer::Core::Camera(PrRenderer::Core::CameraType::Perspective);
	camera->SetSize(1.0f);
	PrRenderer::Core::Camera::SetMainCamera(camera);
}


void Renderer::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	Resources::ShaderPtr shader = std::static_pointer_cast<Resources::Shader>(PrRenderer::Resources::ShaderManager::GetInstance().GetResource("CameraShader.shader"));
	auto sinR = PrCore::Math::sin(m_clock.GetRealTime()*2) * 0.5f + 1.0f;
	auto sinG = PrCore::Math::sin(m_clock.GetRealTime()*2 + 2.0f) * 0.5f + 1.0f;
	auto sinB = PrCore::Math::sin(m_clock.GetRealTime()*2 + 4.0f) * 0.5f + 1.0f;
	
	auto roll = m_clock.GetRealTime()* 1.0f;
	auto positionZ = PrCore::Math::sin(m_clock.GetRealTime()/2) * 15.0f + 18.0f;
	auto camera = PrRenderer::Core::Camera::GetMainCamera();
	camera->SetPosition({0,0,positionZ });
	camera->SetRotation({ 0,0,roll });
	camera->RecalculateMatrices();
	auto& MVP = camera->GetCameraMatrix();

	shader->SetUniformMat4("u_MVP", MVP);

	vertexArray->Bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	m_clock.Tick();
}
