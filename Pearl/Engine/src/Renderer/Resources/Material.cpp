#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/Material.h"

#include"Core/Resources/ResourceSystem.h"
#include"Renderer/Resources/Texture2D.h"
#include"Renderer/Resources/Cubemap.h"

//Const Pearl engine shader uniforms 
#define TEXOFFSET_UNIFORM "_offset"
#define TEXSCALE_UNIFORM "_scale"
#define COLOR_UNIFORM "_color"

using namespace PrRenderer::Resources;
using namespace PrCore::Utils;

Material::Material(ShaderPtr p_shader)
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
			m_textures[uniformName] = Texture2D::CreateUnitTex(PrRenderer::Core::Color::Black);
		if(uniform.type == UniformType::Cubemap)
			m_textures[uniformName] = nullptr;
	}

	if (blackTexture == nullptr)
		blackTexture = Texture2D::CreateUnitTex(Core::Color::Black);
}

Material::Material(const Material& p_material)
{
	m_shader = p_material.m_shader;
	m_uniforms = p_material.m_uniforms;

	m_renderType = p_material.GetRenderType();
	m_renderOrder = p_material.GetRenderOrder();

	m_textures = p_material.m_textures;

	if (blackTexture == nullptr)
		blackTexture = Texture2D::CreateUnitTex(Core::Color::Black);
}

Material::Material(JSON::json& p_seralizedMat)
{
	bool result = PopulateBasedOnShader(p_seralizedMat);
	PR_ASSERT(result, "Cannot load material file corrupted");

	if (blackTexture == nullptr)
		blackTexture = Texture2D::CreateUnitTex(Core::Color::Black);
}

void Material::SetColor(const Core::Color& p_color)
{
	auto find = m_uniforms.find(COLOR_UNIFORM);
	if (find != m_uniforms.end())
		find->second.value = std::make_any<PrCore::Math::vec4>(p_color);
}

const PrRenderer::Core::Color& Material::GetColor() const
{
	auto find = m_uniforms.find(COLOR_UNIFORM);

	if (find != m_uniforms.end())
	{
		auto vec4 = std::any_cast<PrCore::Math::vec4>(find->second.value);
		return Core::Color(vec4);
	}

	PRLOG_WARN("Renderer: Material {0}, missing uniform {1}", m_name, COLOR_UNIFORM);
	return Core::Color();
}

void Material::Bind()
{
	//Bind shader
	m_shader->Bind();

	//Bind Textures
	int texSlot = 0;
	for (auto& texture : m_textures)
	{
		if (texture.second)
		{
			texture.second->Bind(texSlot);
			m_shader->SetUniformInt(texture.first, texSlot);
			texSlot++;
		}
		else
		{
			// Pipeline expects to bind black texture is not assigned
			blackTexture->Bind(texSlot);
			m_shader->SetUniformInt(texture.first, texSlot);
			texSlot++;
		}
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

void Material::Unbind()
{
	m_shader->Unbind();

	unsigned int texSlot = 0;
	for (auto& texture : m_textures)
	{
		if(texture.second)
			texture.second->Unbind(texSlot++);
	}

}

void Material::SetTexture(const std::string& p_name, TexturePtr p_texture)
{
	auto find = m_textures.find(p_name);
	if (find != m_textures.end())
	{
		find->second = p_texture;
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

PrCore::Math::vec2 Material::GetTexScale(const std::string& p_name) const
{
	auto find = m_uniforms.find(p_name + TEXSCALE_UNIFORM);

	if (find != m_uniforms.end())
		return std::any_cast<PrCore::Math::vec2>(find->second.value);

	PRLOG_WARN("Renderer: Material {0}, missing uniform {1}", m_name, p_name);
	return PrCore::Math::vec2();
}

PrCore::Math::vec2 Material::GetTexOffset(const std::string& p_name) const
{
	auto find = m_uniforms.find(p_name + TEXOFFSET_UNIFORM);

	if (find != m_uniforms.end())
		return std::any_cast<PrCore::Math::vec2>(find->second.value);

	PRLOG_WARN("Renderer: Material {0}, missing uniform {1}", m_name, p_name);
	return PrCore::Math::vec2();
}

bool Material::HasProperty(const std::string& p_name) const
{
	return m_uniforms.find(p_name) != m_uniforms.end();
}

void Material::CopyPropertiesFrom(const Material& p_material)
{
	m_renderType = p_material.GetRenderType();
	m_renderOrder = p_material.GetRenderOrder();
	m_textures = p_material.m_textures;

	for (auto& uniformObject : p_material.m_uniforms)
	{
		auto& uniformName = uniformObject.first;
		auto& unform = uniformObject.second;

		auto find = m_uniforms.find(uniformName);
		if (find != m_uniforms.end() && find->second.type == unform.type)
			find->second.value = unform.value;
	}
}

bool Material::PopulateBasedOnShader(JSON::json& p_json)
{
	float version = p_json["materialVersion"];
	if (version != m_materialVersion)
	{
		PRLOG_ERROR("Renderer: Material {0} wrong version", m_name);
		return false;
	}

	// Change in the future
	std::string shader = p_json["shader"];
	m_shader = PrCore::Resources::ResourceSystem::GetInstance().Load<Shader>(shader).GetData();
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
			//Set default value
			uniform.value = std::make_any<int>(0);

			auto intProperties = p_json.find("intproperties");
			if (intProperties != p_json.end())
			{
				auto& value = intProperties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
						uniform.value = std::make_any<int>(property.value()["value"]);
				}
			}

			break;
		}
		case UniformType::Bool:
		{
			//Set default value
			uniform.value = std::make_any<bool>(false);

			auto boolProperies = p_json.find("boolproperties");
			if (boolProperies != p_json.end())
			{
				auto& value = boolProperies.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
						uniform.value = std::make_any<bool>(property.value()["value"]);
				}
			}

			break;
		}
		case UniformType::Float:
		{
			//Set default value
			uniform.value = std::make_any<float>(0.0f);

			auto floatProperties = p_json.find("floatproperties");
			if (floatProperties != p_json.end())
			{
				auto& value = floatProperties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
						uniform.value = std::make_any<float>(property.value()["value"]);
				}
			}

			break;
		}
		case UniformType::Float_Vec2:
		{
			//Set default value
			uniform.value = std::make_any<PrCore::Math::vec2>(PrCore::Math::vec2(0.0f));

			auto float2Properties = p_json.find("float2properties");
			if (float2Properties != p_json.end())
			{
				auto& value = float2Properties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
						uniform.value = std::make_any<PrCore::Math::vec2>(PrCore::Utils::JSONParser::ToVec2(property.value()["value"]));
				}
			}

			//Check if this is a texture property temporary solution scale and offset works for all textures
			auto textures = p_json.find("textures");
			if (textures != p_json.end())
			{
				auto& value = textures.value();
				for (auto& texture : value.items())
				{
					auto fieldName = (std::string)texture.value()["texName"];
					if (fieldName + TEXOFFSET_UNIFORM == uniformName)
						uniform.value = std::make_any<PrCore::Math::vec2>(PrCore::Utils::JSONParser::ToVec2(texture.value()["texOffset"]));


					else if (fieldName + TEXSCALE_UNIFORM == uniformName)
						uniform.value = std::make_any<PrCore::Math::vec2>(PrCore::Utils::JSONParser::ToVec2(texture.value()["texScale"]));
				}
			}

			break;
		}
		case UniformType::Float_Vec3:
		{
			//Set default value
			uniform.value = std::make_any<PrCore::Math::vec3>(PrCore::Math::vec3(0.0f));

			auto float3Properties = p_json.find("float3properties");
			if (float3Properties != p_json.end())
			{
				auto& value = float3Properties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
						uniform.value = std::make_any<PrCore::Math::vec3>(PrCore::Utils::JSONParser::ToVec3(property.value()["value"]));
				}
			}

			break;
		}
		case UniformType::Float_Vec4:
		{
			//Set default value
			uniform.value = std::make_any<PrCore::Math::vec4>(PrCore::Math::vec4(0.0f));

			auto float4Properties = p_json.find("float4properties");
			if (float4Properties != p_json.end())
			{
				auto& value = float4Properties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
						uniform.value = std::make_any<PrCore::Math::vec4>(PrCore::Utils::JSONParser::ToVec4(property.value()["value"]));
				}
			}

			if (uniformName == COLOR_UNIFORM)
				uniform.value = std::make_any<PrCore::Math::vec4>(PrCore::Utils::JSONParser::ToColor(p_json["color"]));

			break;
		}
		case UniformType::Float_Mat3:
		{
			//Set default value
			uniform.value = std::make_any<PrCore::Math::mat3>(PrCore::Math::mat3(0.0f));

			auto mat3Properties = p_json.find("mat3properties");
			if (mat3Properties != p_json.end())
			{
				auto value = mat3Properties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
						uniform.value = std::make_any<PrCore::Math::mat3>(PrCore::Utils::JSONParser::ToMat3(property.value()["value"]));
				}
			}

			break;
		}
		case UniformType::Float_Mat4:
		{
			//Set default value
			uniform.value = std::make_any<PrCore::Math::mat4>(PrCore::Math::mat4(0.0f));

			auto mat4Properties = p_json.find("mat4properties");
			if (mat4Properties != p_json.end())
			{
				auto& value = mat4Properties.value();
				for (auto& property : value.items())
				{
					if (property.value()["name"] == uniformName)
						uniform.value = std::make_any<PrCore::Math::mat4>(PrCore::Utils::JSONParser::ToMat4(property.value()["value"]));
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
					if (texture.value()["texName"] == uniformName)
					{
						auto texturePath = texture.value()["texPath"];

						auto textureResource = PrCore::Resources::ResourceSystem::GetInstance().Load<Texture>(static_cast<std::string>(texturePath)).GetData();
						if (textureResource != nullptr)
						{
							m_textures[uniformName] = textureResource;
							uniformExist = true;
						}
					}
				}
			}
			if (!uniformExist)
				m_textures[uniformName] = Texture2D::CreateUnitTex(PrRenderer::Core::Color::Black);

			break;
		}
		case UniformType::Cubemap:
		{
			auto uniformExist = false;
			auto cubemaps = p_json.find("cubemap");
			if (cubemaps != p_json.end())
			{
				auto& value = cubemaps.value();
				for (auto& cubemap : value.items())
				{
					if (cubemap.value()["texName"] == uniformName)
					{
						auto cubemapPath = cubemap.value()["texPath"];

						auto textureResource = PrCore::Resources::ResourceSystem::GetInstance().Load<Cubemap>(static_cast<std::string>(cubemapPath));
						if (textureResource != nullptr)
						{
							m_textures[uniformName] = textureResource.GetData();
							uniformExist = true;
						}
					}
				}
			}
			if (!uniformExist)
				m_textures[uniformName] = Cubemap::CreateUnitTex(PrRenderer::Core::Color::Black);

			break;
		}
		case UniformType::Texture3D:
			break;
		default:
			PRLOG_WARN("Renderer: Material {0} Uniform {1} type not supported", m_name, uniformName);
			break;
		}
	}

	return true;
}

size_t Material::GetByteSize() const
{
	return sizeof(Material);
}
