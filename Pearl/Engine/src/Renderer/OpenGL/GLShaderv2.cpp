#include "Core/Common/pearl_pch.h"

#include "Renderer/OpenGL/GLShaderv2.h"
#include "Core/Utils/StringUtils.h"

#include<any>
#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

GLShaderv2::GLShaderv2(const std::string& p_vertexShader, const std::string& p_fragmentShader) :
	Shaderv2(p_vertexShader, p_fragmentShader)
{
}

GLShaderv2::GLShaderv2(const std::string& p_vertexShader, const std::string& p_fragmentShader, const std::string& p_geometeryShader) :
	Shaderv2(p_vertexShader, p_fragmentShader, m_geometryShader)
{
}

void GLShaderv2::Bind()
{
	glUseProgram(m_ID);
}

void GLShaderv2::Unbind()
{
	glUseProgram(0);
}

void GLShaderv2::SetUniformFloat(const std::string& p_name, float p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniform1f(location, p_value);
}

void GLShaderv2::SetUniformInt(const std::string& p_name, int p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniform1i(location, p_value);
}

void GLShaderv2::SetUniformBool(const std::string& p_name, bool p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniform1i(location, p_value);
}

void GLShaderv2::SetUniformMat4(const std::string& p_name, const PrCore::Math::mat4& p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniformMatrix4fv(location, 1, GL_FALSE, &p_value[0].x);
}

void GLShaderv2::SetUniformMat3(const std::string& p_name, const PrCore::Math::mat3 p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniformMatrix3fv(location, 1, GL_FALSE, &p_value[0].x);
}

void GLShaderv2::SetUniformVec4(const std::string& p_name, const PrCore::Math::vec4& p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniform4f(location, p_value.x, p_value.y, p_value.z, p_value.w);
}

void GLShaderv2::SetUniformVec3(const std::string& p_name, const PrCore::Math::vec3& p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniform3f(location, p_value.x, p_value.y, p_value.z);
}

void GLShaderv2::SetUniformVec2(const std::string& p_name, const PrCore::Math::vec2& p_value)
{
	auto location = GetUniformLocation(p_name);
	glUniform2f(location, p_value.x, p_value.y);
}

void GLShaderv2::SetUniformFloatArray(const std::string& p_name, const float* p_value, unsigned int p_count)
{
	auto location = GetUniformLocation(p_name);
	glUniform1fv(location, p_count, p_value);
}

void GLShaderv2::SetUniformIntArray(const std::string& p_name, const int* p_value, unsigned int p_count)
{
	auto location = GetUniformLocation(p_name);
	glUniform1iv(location, p_count, p_value);
}

void GLShaderv2::SetUniformMat4Array(const std::string& p_name, const PrCore::Math::mat4* p_value, unsigned int p_count)
{
	auto location = GetUniformLocation(p_name);
	glUniformMatrix4fv(location, p_count, GL_FALSE, &p_value[0][0].x);
}

void GLShaderv2::SetUniformMat3Array(const std::string& p_name, const PrCore::Math::mat3* p_value, unsigned int p_count)
{
	auto location = GetUniformLocation(p_name);
	glUniformMatrix3fv(location, p_count, GL_FALSE, &p_value[0][0].x);
}

void GLShaderv2::SetUniformVec4Array(const std::string& p_name, const PrCore::Math::vec4* p_value, unsigned int p_count)
{
	auto location = GetUniformLocation(p_name);
	glUniform4fv(location, p_count, &p_value->x);
}

void GLShaderv2::SetUniformVec3Array(const std::string& p_name, const PrCore::Math::vec3* p_value, unsigned int p_count)
{
	auto location = GetUniformLocation(p_name);
	glUniform3fv(location, p_count, &p_value->x);
}

void GLShaderv2::SetUniformVec2Array(const std::string& p_name, const PrCore::Math::vec2* p_value, unsigned int p_count)
{
	auto location = GetUniformLocation(p_name);
	glUniform2fv(location, p_count, &p_value->x);
}

float GLShaderv2::GetUniformFloat(const std::string& p_name)
{
	GLfloat value;
	auto location = GetUniformLocation(p_name);
	glGetUniformfv(m_ID, location, &value);

	return (float)value;
}

int GLShaderv2::GetUniformInt(const std::string& p_name)
{
	GLint value;
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, &value);

	return (int)value;
}

bool GLShaderv2::GetUniformBool(const std::string& p_name)
{
	GLint value;
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, &value);

	return (bool)value;
}

PrCore::Math::mat4 GLShaderv2::GetUniformMat4(const std::string& p_name)
{
	GLint value[16];
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, value);

	return PrCore::Math::make_mat4(value);
}

PrCore::Math::mat3 GLShaderv2::GetUniformMat3(const std::string& p_name)
{
	GLint value[12];
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, value);

	return PrCore::Math::make_mat3(value);
}

PrCore::Math::vec4 GLShaderv2::GetUniformVec4(const std::string& p_name)
{
	GLint value[4];
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, value);

	return PrCore::Math::make_vec4(value);
}

PrCore::Math::vec3 GLShaderv2::GetUniformVec3(const std::string& p_name)
{
	GLint value[3];
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, value);

	return PrCore::Math::make_vec3(value);
}

PrCore::Math::vec2 GLShaderv2::GetUniformVec2(const std::string& p_name)
{
	GLint value[2];
	auto location = GetUniformLocation(p_name);
	glGetUniformiv(m_ID, location, value);

	return PrCore::Math::make_vec2(value);
}

bool GLShaderv2::Compile()
{
	if (m_ID != 0)
	{
		PRLOG_WARN("Shader already compiled! Shader ID {0}" m_ID);
		return true;
	}

	RendererID vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	auto sourceVertex = m_vertexShader.c_str();
	glShaderSource(vertexShader, 1, &sourceVertex, NULL);
	glCompileShader(vertexShader);

	int  success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		PRLOG_ERROR("Renderer: VertexShader " + m_name + " Error: " + infoLog);
		return false;
	}

	RendererID geometryShader = 0;
	if (!m_geometryShader.empty())
	{
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);

		auto sourceGeometry = m_geometryShader.c_str();
		glShaderSource(geometryShader, 1, &sourceGeometry, NULL);
		glCompileShader(geometryShader);

		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);

		if (!success)
		{
			char infoLog[512];
			glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
			PRLOG_ERROR("Renderer: GeometryShader " + m_name + " Error: " + infoLog);
			return false;
		}
	}

	RendererID fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	auto sourceFragment = m_fragmentShader.c_str();
	glShaderSource(fragmentShader, 1, &sourceFragment, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		PRLOG_ERROR("Renderer: FragmentShader " + m_name + " Error: " + infoLog);
		return false;
	}

	m_ID = glCreateProgram();

	glAttachShader(m_ID, vertexShader);
	glAttachShader(m_ID, fragmentShader);
	if (geometryShader)
		glAttachShader(m_ID, geometryShader);
	glLinkProgram(m_ID);

	glGetShaderiv(m_ID, GL_LINK_STATUS, &success);

	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(m_ID, 512, NULL, infoLog);
		PRLOG_ERROR("Renderer: Shader Linking " + m_name + " Error: " + infoLog);
		return false;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(geometryShader);

	ScanUniforms();

	return true;
}

void GLShaderv2::ScanUniforms()
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

		PrCore::StringUtils::ResizeToFitContains(uniformName);

		Resources::UniformType prUniformType = Resources::UniformType::None;
		std::any uniformValue;

		switch (uniformType)
		{
		case GL_INT:
			if (uniformSize == 1)
			{
				prUniformType = Resources::UniformType::Int;
				uniformValue = std::make_any<int>(GetUniformInt(uniformName));
			}
			else
				prUniformType = Resources::UniformType::Int_Array;
			break;
		case GL_BOOL:
			prUniformType = Resources::UniformType::Bool;
			uniformValue = std::make_any<bool>(GetUniformInt(uniformName));

			break;
		case GL_FLOAT:
			if (uniformSize == 1)
			{
				prUniformType = Resources::UniformType::Float;
				uniformValue = std::make_any<float>(GetUniformFloat(uniformName));
			}
			else
				prUniformType = Resources::UniformType::Float_Array;
			break;
		case GL_FLOAT_VEC2:
			if (uniformSize == 1)
			{
				prUniformType = Resources::UniformType::Float_Vec2;
				uniformValue = std::make_any<PrCore::Math::vec2>(GetUniformVec2(uniformName));
			}
			else
				prUniformType = Resources::UniformType::Float_Vec2_Array;
			break;
		case GL_FLOAT_VEC3:
			if (uniformSize == 1)
			{
				prUniformType = Resources::UniformType::Float_Vec3;
				uniformValue = std::make_any<PrCore::Math::vec3>(GetUniformVec3(uniformName));
			}
			else
				prUniformType = Resources::UniformType::Float_Vec3_Array;
			break;
		case GL_FLOAT_VEC4:
			if (uniformSize == 1)
			{
				prUniformType = Resources::UniformType::Float_Vec4;
				uniformValue = std::make_any<PrCore::Math::vec4>(GetUniformVec4(uniformName));
			}
			else
				prUniformType = Resources::UniformType::Float_Vec4_Array;
			break;
		case GL_FLOAT_MAT4:
			if (uniformSize == 1)
			{
				prUniformType = Resources::UniformType::Float_Mat4;
				uniformValue = std::make_any<PrCore::Math::mat4>(GetUniformMat4(uniformName));
			}
			else
				prUniformType = Resources::UniformType::Float_Mat4_Array;
			break;
		case GL_FLOAT_MAT3:
			if (uniformSize == 1)
			{
				prUniformType = Resources::UniformType::Float_Mat3;
				uniformValue = std::make_any<PrCore::Math::mat3>(GetUniformMat3(uniformName));
			}
			else
				prUniformType = Resources::UniformType::Float_Mat3_Array;
			break;
		case GL_SAMPLER_2D:
			prUniformType = Resources::UniformType::Texture2D;
			break;
		case GL_SAMPLER_3D:
			prUniformType = Resources::UniformType::Texture3D;
			break;
		case GL_SAMPLER_CUBE:
			prUniformType = Resources::UniformType::Cubemap;
			break;
		default:
		{
			PRLOG_INFO("Renderer Shader name:{0} Uniform {1} is not supported", m_name, uniformName);
			continue;
			break;
		}
		}

		Resources::Uniform uniform{
			prUniformType,
			(unsigned int)uniformSize,
			uniformValue
		};

		m_uniforms[uniformName] = uniform;
	}
}

int GLShaderv2::GetUniformLocation(const std::string& p_name)
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

size_t GLShaderv2::GetByteSize() const
{
	return sizeof(GLShaderv2) + m_vertexShader.size() + m_fragmentShader.size() + m_geometryShader.size();
}
