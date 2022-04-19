#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/Material.h"
#include"Renderer/Resources/ShaderManager.h"
#include"Renderer/Resources/TextureManager.h"

#include"Core/Filesystem/FileSystem.h"

//Const Pearl engine shader uniforms 
#define TEXOFFSET_UNIFORM "_offset"
#define TEXSCALE_UNIFORM "_scale"
#define COLOR_UNIFORM "_color"

using namespace PrRenderer::Resources;
using namespace PrCore::Utils;

Material::Material(const std::string& p_name, PrCore::Resources::ResourceID p_ID):
	Resources(p_name, p_ID),
	m_renderType(RenderType::Opaque),
	m_renderOrder(0.f)
{}

Material::Material(ShaderPtr p_shader):
	Resources("Material_"+ p_shader->GetName())
{	
	m_shader = p_shader;
	m_uniforms = m_shader->GetAllUniforms();

	m_renderType = RenderType::Opaque;
	m_renderOrder = 0;

	for (auto& unformPair : m_uniforms)
	{
		auto& uniformName = unformPair.first;
		auto& uniform = unformPair.second;

		if (uniform.type == UniformType::Texture2D)
			m_textures[uniformName] = TexturePtr();
	}
}

Material::Material(const Material& p_material):
	Resources("Material_" + p_material.m_shader->GetName())
{
	m_shader = p_material.m_shader;
	m_uniforms = p_material.m_uniforms;

	m_renderType = RenderType::Opaque;
	m_renderOrder = 0;

	m_textures = p_material.m_textures;
}

void Material::SetColor(const PrRenderer::Core::Color& p_color)
{
	auto find = m_uniforms.find(COLOR_UNIFORM);
	if (find != m_uniforms.end())
		find->second.value = std::make_any<PrCore::Math::vec4>(p_color);
}

const PrRenderer::Core::Color& Material::GetColor()
{
	auto find = m_uniforms.find(COLOR_UNIFORM);

	if (find != m_uniforms.end())
	{
		auto vec4 = std::any_cast<PrCore::Math::vec4>(find->second.value);
		return PrRenderer::Core::Color(vec4);
	}

	PRLOG_WARN("Renderer: Material {0}, missing uniform {1}", m_name, COLOR_UNIFORM);
	return PrRenderer::Core::Color();
}

void Material::Bind()
{
	//Bind shader
	m_shader->Bind();

	//Bind Textures
	int texSlot = 0;
	for (auto& texture : m_textures)
	{
		texture.second->Bind(texSlot);
		m_shader->SetUniformInt(texture.first, texSlot);
		texSlot++;

		//Set Texture Usage Flag
		if(HasProperty("use" + texture.first))
			SetProperty("use" + texture.first, true);
	}

	//Bind Properties
	for (auto& uniform : m_uniforms)
	{
		auto& uniformValue = uniform.second;
		const auto& uniformName = uniform.first;

		if (!uniformValue.value.has_value())
			continue;

		switch (uniformValue.type)
		{
		case UniformType::Int:
			m_shader->SetUniformInt(uniformName, std::any_cast<int>(uniformValue.value));
			break;
		case UniformType::Bool:
			m_shader->SetUniformBool(uniformName, std::any_cast<bool>(uniformValue.value));
			break;
		case UniformType::Int_Array:
			m_shader->SetUniformIntArray(uniformName, std::any_cast<std::vector<int>>(uniformValue.value).data(), uniformValue.size);
			break;
		case UniformType::Float:
			m_shader->SetUniformFloat(uniformName, std::any_cast<float>(uniformValue.value));
			break;
		case UniformType::Float_Array:
			m_shader->SetUniformFloatArray(uniformName, std::any_cast<std::vector<float>>(uniformValue.value).data(), uniformValue.size);
			break;
		case UniformType::Float_Vec2:
			m_shader->SetUniformVec2(uniformName, std::any_cast<PrCore::Math::vec2>(uniformValue.value));
			break;
		case UniformType::Float_Vec2_Array:
			m_shader->SetUniformVec2Array(uniformName, std::any_cast<std::vector<PrCore::Math::vec2>>(uniformValue.value).data(), uniformValue.size);
			break;
		case UniformType::Float_Vec3:
			m_shader->SetUniformVec3(uniformName, std::any_cast<PrCore::Math::vec3>(uniformValue.value));
			break;
		case UniformType::Float_Vec3_Array:
			m_shader->SetUniformVec3Array(uniformName, std::any_cast<std::vector<PrCore::Math::vec3>>(uniformValue.value).data(), uniformValue.size);
			break;
		case UniformType::Float_Vec4:
			m_shader->SetUniformVec4(uniformName, std::any_cast<PrCore::Math::vec4>(uniformValue.value));
			break;
		case UniformType::Float_Vec4_Array:
			m_shader->SetUniformVec4Array(uniformName, std::any_cast<std::vector<PrCore::Math::vec4>>(uniformValue.value).data(), uniformValue.size);
			break;
		case UniformType::Float_Mat3:
			m_shader->SetUniformMat3(uniformName, std::any_cast<PrCore::Math::mat3>(uniformValue.value));
			break;
		case UniformType::Float_Mat3_Array:
			m_shader->SetUniformMat3Array(uniformName, std::any_cast<std::vector<PrCore::Math::mat3>>(uniformValue.value).data(), uniformValue.size);
			break;
		case UniformType::Float_Mat4:
			m_shader->SetUniformMat4(uniformName, std::any_cast<PrCore::Math::mat4>(uniformValue.value));
			break;
		case UniformType::Float_Mat4_Array:
			m_shader->SetUniformMat4Array(uniformName, std::any_cast<std::vector<PrCore::Math::mat4>>(uniformValue.value).data(), uniformValue.size);
			break;
		}
	}
}

void PrRenderer::Resources::Material::Unbind()
{
	m_shader->Unbind();

	unsigned int texSlot = 0;
	for (auto& texture : m_textures)
		texture.second->Unbind(texSlot++);
	
}

void Material::SetTexture(const std::string& p_name, TexturePtr p_texture)
{
	auto find = m_textures.find(p_name);
	if (find != m_textures.end())
	{
		find->second = p_texture;

		if (p_texture != nullptr)
			SetProperty("use" + p_name, true);
		else
			SetProperty("use" + p_name, false);
	}

}

TexturePtr Material::GetTexture(const std::string& p_name)
{
	auto find = m_textures.find(p_name);
	
	if (find != m_textures.end())
		return find->second;

	PRLOG_WARN("Renderer: Material {0}, missing texture {1}", m_name, p_name);
	return TexturePtr();
}

void Material::SetTexScale(const std::string& p_name, const PrCore::Math::vec2& p_value)
{
	auto find = m_uniforms.find(p_name + TEXSCALE_UNIFORM);
	if (find != m_uniforms.end())
		find->second.value = std::make_any<PrCore::Math::vec2>(p_value);
}

void Material::SetTexOffset(const std::string& p_name, const PrCore::Math::vec2& p_value)
{
	auto find = m_uniforms.find(p_name + TEXOFFSET_UNIFORM);
	if (find != m_uniforms.end())
		find->second.value = std::make_any<PrCore::Math::vec2>(p_value);
}

PrCore::Math::vec2 Material::GetTexScale(const std::string& p_name)
{
	auto find = m_uniforms.find(p_name + TEXSCALE_UNIFORM);

	if (find != m_uniforms.end())
		return std::any_cast<PrCore::Math::vec2>(find->second.value);

	PRLOG_WARN("Renderer: Material {0}, missing uniform {1}", m_name, p_name);
	return PrCore::Math::vec2();
}

PrCore::Math::vec2 Material::GetTexOffset(const std::string& p_name)
{
	auto find = m_uniforms.find(p_name + TEXOFFSET_UNIFORM);

	if (find != m_uniforms.end())
		return std::any_cast<PrCore::Math::vec2>(find->second.value);

	PRLOG_WARN("Renderer: Material {0}, missing uniform {1}", m_name, p_name);
	return PrCore::Math::vec2();
}

bool PrRenderer::Resources::Material::HasProperty(const std::string& p_name)
{
	return m_uniforms.find(p_name) != m_uniforms.end();
}

void Material::PreLoadImpl()
{
}

bool PrRenderer::Resources::Material::LoadImpl()
{
	auto jsonFile = ReadFile();
	if (jsonFile == nullptr)
		return false;

	auto result = PopulateBasedOnShader(jsonFile);

	return true;
}

void Material::PostLoadImpl()
{
}

void Material::PreUnloadImpl()
{
}

bool Material::UnloadImpl()
{
	m_shader.reset();
	m_textures.clear();
	m_uniforms.clear();
	m_renderOrder = 0;
	m_renderType = RenderType::Opaque;

	return true;
}

void Material::PostUnloadImpl()
{
}

void Material::CalculateSize()
{
}

bool Material::PopulateBasedOnShader(PrCore::Utils::JSON::json& p_json)
{
	float version = p_json["materialVersion"];
	if (version != m_materialVersion)
	{
		PRLOG_ERROR("Renderer: Material {0} wrong version", m_name);
		return false;
	}

	std::string shader = p_json["shader"];
	m_shader = std::static_pointer_cast<Shader>(PrRenderer::Resources::ShaderManager::GetInstance().Load(shader));
	if (m_shader == nullptr)
	{
		PRLOG_ERROR("Renderer: Material {0}, shader {1} cannot be find", m_name, shader);
		return false;
	}

	m_renderOrder = p_json["RenderOrder"];
	m_renderType = p_json["RenderType"];

	m_uniforms = m_shader->GetAllUniforms();

	for (auto& uniformPair : m_uniforms)
	{
		const auto& uniformName = uniformPair.first;
		auto& uniform = uniformPair.second;

		switch (uniform.type)
		{
		case UniformType::Int:
		{
			auto uniformExist = false;
			auto intProperties = p_json.find("intproperties");
			if (intProperties != p_json.end())
			{
				auto& value = intProperties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
					{
						uniform.value = std::make_any<int>(property.value()["value"]);
						uniformExist = true;
					}
				}
			}

			break;
		}
		case UniformType::Bool:
		{
			auto uniformExist = false;
			auto intProperties = p_json.find("boolproperties");
			if (intProperties != p_json.end())
			{
				auto& value = intProperties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
					{
						uniform.value = std::make_any<int>(property.value()["value"]);
						uniformExist = true;
					}
				}
			}

			break;
		}
		case UniformType::Float:
		{
			auto uniformExist = false;
			auto floatProperties = p_json.find("floatproperties");
			if (floatProperties != p_json.end())
			{
				auto& value = floatProperties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
					{
						uniform.value = std::make_any<float>(property.value()["value"]);
						uniformExist = true;
					}
				}
			}

			break;
		}
		case UniformType::Float_Vec2:
		{
			auto uniformExist = false;
			auto float2Properties = p_json.find("float2properties");
			if (float2Properties != p_json.end())
			{
				auto& value = float2Properties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
					{
						uniform.value = std::make_any<PrCore::Math::vec2>(PrCore::Utils::JSONParser::ToVec2(property.value()["value"]));
						uniformExist = true;
					}
				}
			}

			//Check if this is a texture property
			auto textures = p_json.find("textures");
			if (textures != p_json.end())
			{
				auto& value = textures.value();
				for (auto& texture : value.items())
				{
					if ((std::string)texture.value()["texName"] + TEXOFFSET_UNIFORM == uniformName)
					{
						uniform.value = std::make_any<PrCore::Math::vec2>(PrCore::Utils::JSONParser::ToVec2(texture.value()["texOffset"]));
						uniformExist = true;
					}
					else if ((std::string)texture.value()["texName"] + TEXSCALE_UNIFORM == uniformName)
					{
						uniform.value = std::make_any<PrCore::Math::vec2>(PrCore::Utils::JSONParser::ToVec2(texture.value()["texScale"]));
						uniformExist = true;
					}
				}
			}

			break;
		}
		case UniformType::Float_Vec3:
		{
			auto uniformExist = false;
			auto float3Properties = p_json.find("float3properties");
			if (float3Properties != p_json.end())
			{
				auto& value = float3Properties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
					{
						uniform.value = std::make_any<PrCore::Math::vec3>(PrCore::Utils::JSONParser::ToVec3(property.value()["value"]));
						uniformExist = true;
					}
				}
			}

			break;
		}
		case UniformType::Float_Vec4:
		{
			auto uniformExist = false;
			auto float4Properties = p_json.find("float4properties");
			if (float4Properties != p_json.end())
			{
				auto& value = float4Properties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
					{
						uniform.value = std::make_any<PrCore::Math::vec4>(PrCore::Utils::JSONParser::ToVec4(property.value()["value"]));
						uniformExist = true;
					}
				}
			}

			if (uniformName == COLOR_UNIFORM)
			{
				uniform.value = std::make_any<PrCore::Math::vec4>(PrCore::Utils::JSONParser::ToColor(p_json["color"]));
				uniformExist = true;
			}

			break;
		}
		case UniformType::Float_Mat3:
		{
			auto uniformExist = false;
			auto mat3Properties = p_json.find("mat3properties");
			if (mat3Properties != p_json.end())
			{
				auto value = mat3Properties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
					{
						uniform.value = std::make_any<PrCore::Math::mat3>(PrCore::Utils::JSONParser::ToMat3(property.value()["value"]));
						uniformExist = true;
					}
				}
			}

			break;
		}
		case UniformType::Float_Mat4:
		{
			auto uniformExist = false;
			auto mat4Properties = p_json.find("mat4properties");
			if (mat4Properties != p_json.end())
			{
				auto& value = mat4Properties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
					{
						uniform.value = std::make_any<PrCore::Math::mat4>(PrCore::Utils::JSONParser::ToMat4(property.value()["value"]));
						uniformExist = true;
					}
				}
			}

			break;
		}
		case UniformType::Texture2D:
		{
			auto uniformExist = false;
			auto textures = p_json.find("textures");
			if (textures != p_json.end())
			{
				auto& value = textures.value();
				for (auto& texture : value.items())
				{
					if(texture.value()["texName"] == uniformName)
					{
						auto textureName = texture.value()["texName"];
						auto texturePath = texture.value()["texPath"];

						auto textureResource = std::static_pointer_cast<Texture>(PrRenderer::Resources::TextureManager::GetInstance().Load(texturePath));
						m_textures[textureName] = textureResource;
						uniformExist = true;
					}
				}
			}

			break;
		}
		case UniformType::Cubemap:
		case UniformType::Texture3D:
			break;
		default:
			PRLOG_WARN("Renderer: Material {0} Uniform {1} type not supported", m_name, uniformName);
			break;
		}
	}
}

PrCore::Utils::JSON::json Material::ReadFile()
{
	std::string dir = MATERIAL_DIR;
	dir += ("/" + m_name);
	PrCore::Filesystem::FileStreamPtr file = PrCore::Filesystem::FileSystem::GetInstance().OpenFileStream(dir.c_str());
	if (file == nullptr)
		return nullptr;

	char* data = new char[file->GetSize()];
	file->Read(data);

	std::vector<uint8_t> dataVector;

	for (auto i = 0; i < file->GetSize(); i++)
		dataVector.push_back(*(data + i));
	delete[] data;

	return JSON::json::parse(dataVector);
}


