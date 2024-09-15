#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/Shader.h"
#include "Renderer/OpenGL/GLShader.h"

#include "Renderer/Core/RendererAPI.h"


using namespace PrRenderer::Resources;

PrRenderer::Resources::Shader::Shader(const std::string& p_vertexShader, const std::string& p_fragmentShader):
	m_vertexShader(p_vertexShader),
	m_fragmentShader(p_fragmentShader),
	m_geometryShader(""),
	m_ID(0)
{
}

Shader::Shader(const std::string& p_vertexShader, const std::string& p_fragmentShader, const std::string& p_geometeryShader):
	m_vertexShader(p_vertexShader),
	m_fragmentShader(p_fragmentShader),
	m_geometryShader(p_geometeryShader),
	m_ID(0)
{
}

std::shared_ptr<Shader> Shader::Create(const std::string& p_vertexShader, const std::string& p_fragmentShader)
{
	ShaderPtr shader = nullptr;
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: shader = std::make_shared<OpenGL::GLShader>(p_vertexShader, p_fragmentShader); break;

	default:
	{
		PR_ASSERT(false, "No such Graphics API");
		break;
	}
	}

	return shader;
}

std::shared_ptr<Shader> Shader::Create(const std::string& p_vertexShader, const std::string& p_fragmentShader, const std::string& p_geometeryShader)
{
	ShaderPtr shader = nullptr;
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: shader = std::make_shared<OpenGL::GLShader>(p_vertexShader, p_fragmentShader, p_geometeryShader); break;

	default:
	{
		PR_ASSERT(false, "No such Graphics API");
		break;
	}
	}

	return shader;
}
