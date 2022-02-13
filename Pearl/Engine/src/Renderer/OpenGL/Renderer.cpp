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

PrRenderer::Core::Renderer::~Renderer()
{
	delete PrRenderer::Core::Camera::GetMainCamera();
}

void Renderer::Test()
{
	float vertecies[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
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

	Resources::Texture2DPtr texture = std::static_pointer_cast<Resources::Texture2D>(PrRenderer::Resources::TextureManager::GetInstance().Load("brick.jpg"));
	texture->Bind();
	shader->SetUniformInt("u_tex", 0);

	auto camera = new PrRenderer::Core::Camera(PrRenderer::Core::CameraType::Perspective);
	camera->SetSize(1.0f);
	PrRenderer::Core::Camera::SetMainCamera(camera);
}


void Renderer::Draw()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	Resources::ShaderPtr shader = std::static_pointer_cast<Resources::Shader>(PrRenderer::Resources::ShaderManager::GetInstance().GetResource("CameraShader.shader"));
	auto sinR = PrCore::Math::sin(m_clock.GetRealTime()*2) * 0.5f + 1.0f;
	auto sinG = PrCore::Math::sin(m_clock.GetRealTime()*2 + 2.0f) * 0.5f + 1.0f;
	auto sinB = PrCore::Math::sin(m_clock.GetRealTime()*2 + 4.0f) * 0.5f + 1.0f;
	
	auto roll = m_clock.GetRealTime()* 1.0f;
	auto positionZ = PrCore::Math::sin(m_clock.GetRealTime()/2) * 8.0f + 10.0f;
	//camera->SetRotation({ 0,roll,roll });
	
	auto camera = PrRenderer::Core::Camera::GetMainCamera();
	camera->RecalculateMatrices();
	auto MVP = camera->GetCameraMatrix();

	shader->SetUniformMat4("u_MVP", MVP);

	vertexArray->Bind();
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	MVP = MVP * PrCore::Math::translate(PrCore::Math::mat4(1.0f), PrCore::Math::vec3(2, 0, 0));
	shader->SetUniformMat4("u_MVP", MVP);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	m_clock.Tick();
}
