#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLShader.h"

#include"Core/Filesystem/FileSystem.h"
#include"Core/Filesystem/FileStream.h"
#include"Core/Utils/StringUtils.h"

#include<any>
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
	auto location = GetUniformLocation(p_name);
	glUniform1f(location, p_value);
}

void PrRenderer::OpenGL::GLShader::SetUniformInt(const std::string& p_name, int p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniform1i(location, p_value);
}

void PrRenderer::OpenGL::GLShader::SetUniformMat4(const std::string& p_name, const PrCore::Math::mat4& p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniformMatrix4fv(location, 1, GL_FALSE, &p_value[0].x);
}

void PrRenderer::OpenGL::GLShader::SetUniformMat3(const std::string& p_name, const PrCore::Math::mat3 p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniformMatrix3fv(location, 1, GL_FALSE, &p_value[0].x);
}

void PrRenderer::OpenGL::GLShader::SetUniformVec4(const std::string& p_name, const PrCore::Math::vec4& p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniform4f(location, p_value.x, p_value.y, p_value.z, p_value.w);
}

void PrRenderer::OpenGL::GLShader::SetUniformVec3(const std::string& p_name, const PrCore::Math::vec3& p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniform3f(location, p_value.x, p_value.y, p_value.z);
}

void PrRenderer::OpenGL::GLShader::SetUniformVec2(const std::string& p_name, const PrCore::Math::vec2& p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniform2f(location, p_value.x, p_value.y);
}

float PrRenderer::OpenGL::GLShader::GetUniformFloat(const std::string& p_name)
{
	GLfloat value;
	auto location = GetUniformLocation(p_name);
	glGetUniformfv(m_ID, location, &value);

	return (float)value;
}

int PrRenderer::OpenGL::GLShader::GetUniformInt(const std::string& p_name)
{
	GLint value;
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, &value);

	return (int)value;
}

PrCore::Math::mat4 PrRenderer::OpenGL::GLShader::GetUniformMat4(const std::string& p_name)
{
	GLint value[16];
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, value);

	return PrCore::Math::make_mat4(value);
}

PrCore::Math::mat3 PrRenderer::OpenGL::GLShader::GetUniformMat3(const std::string& p_name)
{
	GLint value[12];
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, value);

	return PrCore::Math::make_mat3(value);
}

PrCore::Math::vec4 PrRenderer::OpenGL::GLShader::GetUniformVec4(const std::string& p_name)
{
	GLint value[4];
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, value);

	return PrCore::Math::make_vec4(value);
}

PrCore::Math::vec3 PrRenderer::OpenGL::GLShader::GetUniformVec3(const std::string& p_name)
{
	GLint value[3];
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, value);

	return PrCore::Math::make_vec3(value);
}

PrCore::Math::vec2 PrRenderer::OpenGL::GLShader::GetUniformVec2(const std::string& p_name)
{
	GLint value[2];
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, value);

	return PrCore::Math::make_vec2(value);
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
		std::string error(infoLog);
		PRLOG_ERROR("Renderer: FragmentShader " + m_name + " Error: " + error);
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
	m_uniformLocation.clear();
	m_uniforms.clear();

	GLint uniformNumber = 0;
	glGetProgramiv(m_ID, GL_ACTIVE_UNIFORMS, &uniformNumber);

	GLint uniformSize;
	GLenum uniformType;
	GLint uniformLength;
	GLint maxNameSize = 64;
	std::string uniformName;

	for (int i = 0; i < uniformNumber; i++)
	{
		uniformName.clear();
		uniformName.resize(maxNameSize);
		glGetActiveUniform(m_ID, i, maxNameSize, &uniformLength, &uniformSize, &uniformType, &uniformName[0]);

		PrRenderer::Resources::UniformType prUniformType = PrRenderer::Resources::UniformType::None;

		switch (uniformType)
		{
			case GL_INT:
				prUniformType = PrRenderer::Resources::UniformType::Int;
				break;
			case GL_FLOAT:
				prUniformType = PrRenderer::Resources::UniformType::Float;
				break;
			case GL_FLOAT_VEC2:
				prUniformType = PrRenderer::Resources::UniformType::Float_Vec2;
				break;
			case GL_FLOAT_VEC3:
				prUniformType = PrRenderer::Resources::UniformType::Float_Vec3;
				break;
			case GL_FLOAT_VEC4:
				prUniformType = PrRenderer::Resources::UniformType::Float_Vec4;
				break;
			case GL_FLOAT_MAT4:
				prUniformType = PrRenderer::Resources::UniformType::Float_Mat4;
				break;
			case GL_FLOAT_MAT3:
				prUniformType = PrRenderer::Resources::UniformType::Float_Mat3;
				break;
			case GL_SAMPLER_2D:
				prUniformType = PrRenderer::Resources::UniformType::Texture2D;
				break;
			case GL_SAMPLER_3D:
				prUniformType = PrRenderer::Resources::UniformType::Texture3D;
				break;
			case GL_TEXTURE_CUBE_MAP:
				prUniformType = PrRenderer::Resources::UniformType::Cubemap;
				break;
			default:
			{
				PRLOG_INFO("Renderer Shader name:{0} Uniform {1} is not supported", m_name, uniformName);
				continue;
				break;
			}
		}

		PrCore::Utils::StringUtils::ResizeToFitContains(uniformName);

		PrRenderer::Resources::Uniform uniform{
			uniformName,
			prUniformType,
			GetUniformLocation(uniformName)
		};

		m_uniforms[uniform.name] = uniform;
	}
}

int PrRenderer::OpenGL::GLShader::GetUniformLocation(const std::string& p_name)
{
	//Due to different string size sometimes there can be more than one key with the same value
	//to fix in future
	auto location = m_uniformLocation.find(p_name);
	if (location != m_uniformLocation.end())
		return location->second;

	auto glLocation = glGetUniformLocation(m_ID, p_name.c_str());

	m_uniformLocation[p_name] = glLocation;
	return glLocation;
}

void GLShader::PreLoadImpl()
{
	//Load from file
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

	m_uniformLocation.clear();
	m_uniforms.clear();
	m_vertexShader.clear();
	m_fragmentShader.clear();

	return true;
}

void PrRenderer::OpenGL::GLShader::PostUnloadImpl()
{
}

void PrRenderer::OpenGL::GLShader::CalculateSize()
{
	m_size =
		sizeof(m_vertexShader) + sizeof(char) * m_vertexShader.length() +
		sizeof(m_fragmentShader) + sizeof(char) * m_fragmentShader.length() +
		sizeof(m_uniforms) + m_uniformLocation.size() * sizeof(decltype(m_uniforms)::value_type) +
		sizeof(m_uniformLocation) + m_uniformLocation.size() * (sizeof(decltype(m_uniformLocation)::key_type) + sizeof(decltype(m_uniformLocation)::mapped_type)) +
		sizeof(m_name) + sizeof(char) * m_name.length();
}
