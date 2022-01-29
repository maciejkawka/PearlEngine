#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLShader.h"

#include"Core/Filesystem/FileSystem.h"
#include"Core/Filesystem/FileStream.h"

#include"glad/glad.h"

using namespace PrRenderer::OpenGL;
using namespace PrCore::Filesystem;

GLShader::GLShader(const std::string& p_name, PrCore::Resources::ResourceManager* p_creator, PrCore::Resources::ResourceID p_ID):
	Shader(p_name, p_creator, p_ID)
{}

GLShader::~GLShader()
{}

void PrRenderer::OpenGL::GLShader::Bind()
{
	glUseProgram(m_ID);
}

void PrRenderer::OpenGL::GLShader::Unbind()
{
	glUseProgram(0);
}

void PrRenderer::OpenGL::GLShader::SetUniformFloat(const std::string& p_name, float p_value)
{
}

void PrRenderer::OpenGL::GLShader::SetUniformInt(const std::string& p_name, int p_value)
{
}

void PrRenderer::OpenGL::GLShader::SetUniformMat4(const std::string& p_name, const PrCore::Math::mat4& p_value)
{
}

void PrRenderer::OpenGL::GLShader::SetUniformMat3(const std::string& p_name, const PrCore::Math::mat3 p_value)
{
}

void PrRenderer::OpenGL::GLShader::SetUniformVec4(const std::string& p_name, const PrCore::Math::vec4& p_value)
{
}

void PrRenderer::OpenGL::GLShader::SetUniformVec3(const std::string& p_name, const PrCore::Math::vec3& p_value)
{
}

void PrRenderer::OpenGL::GLShader::SetUniformVec2(const std::string& p_name, const PrCore::Math::vec2& p_value)
{
}

void PrRenderer::OpenGL::GLShader::GetUniformFloat(const std::string& p_name)
{
}

void PrRenderer::OpenGL::GLShader::GetUniformInt(const std::string& p_name)
{
}

void PrRenderer::OpenGL::GLShader::GetUniformMat4(const std::string& p_name)
{
}

void PrRenderer::OpenGL::GLShader::GetUniformMat3(const std::string& p_name)
{
}

void PrRenderer::OpenGL::GLShader::GetUniformVec4(const std::string& p_name)
{
}

void PrRenderer::OpenGL::GLShader::GetUniformVec3(const std::string& p_name)
{
}

void PrRenderer::OpenGL::GLShader::GetUniformVec2(const std::string& p_name)
{
}

bool PrRenderer::OpenGL::GLShader::Compile()
{
	if (m_ID != 0)
		return true;

	RendererID vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	auto sourceVertex = m_vertexShader.c_str();
	glShaderSource(vertexShader, 1, &sourceVertex, NULL);
	glCompileShader(vertexShader);

	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		PRLOG_ERROR("Renderer: VertexShader " + m_name + " Error: " + infoLog);
		return false;
	}

	RendererID fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	auto sourceFragment = m_fragmentShader.c_str();
	glShaderSource(fragmentShader, 1, &sourceFragment, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		PRLOG_ERROR("Renderer: FragmentShader " + m_name + " Error: " + infoLog);
		return false;
	}

	m_ID = glCreateProgram();

	glAttachShader(m_ID, vertexShader);
	glAttachShader(m_ID, fragmentShader);
	glLinkProgram(m_ID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return true;
}

void PrRenderer::OpenGL::GLShader::ScanUniforms()
{
}

void GLShader::PreLoadImpl()
{
	std::string dir = SHADER_DIR;
	dir += ("/" + m_name);

	FileStreamPtr file = FileSystem::GetInstance().OpenFileStream(dir.c_str());
	std::string shader;
	shader.resize(file->GetSize());
	file->Read(&shader[0]);

	std::string vertexShaderKeyword = "#vertex";
	std::string fragmentShaderKeyword = "#fragment";

	auto fragmentShaderPos = shader.find(fragmentShaderKeyword);
	
	m_vertexShader = shader.substr(vertexShaderKeyword.length(), fragmentShaderPos - vertexShaderKeyword.length());
	m_fragmentShader = shader.substr(fragmentShaderPos + fragmentShaderKeyword.length());
}

bool GLShader::LoadImpl()
{
	if (m_vertexShader.empty() || m_fragmentShader.empty())
		return false;

	auto result = Compile();

	return result;
}

void PrRenderer::OpenGL::GLShader::PostLoadImpl()
{
	ScanUniforms();
}

void PrRenderer::OpenGL::GLShader::PreUnloadImpl()
{

}

bool PrRenderer::OpenGL::GLShader::UnloadImpl()
{
	glDeleteProgram(m_ID);
	m_ID = 0;
	return true;
}

void PrRenderer::OpenGL::GLShader::PostUnloadImpl()
{
}

void PrRenderer::OpenGL::GLShader::CalculateSize()
{
	m_size =
		sizeof(m_vertexShader) +
		sizeof(m_fragmentShader) +
		sizeof(m_uniforms) +
		sizeof(m_uniformLocation)+
		sizeof(m_name);
}
