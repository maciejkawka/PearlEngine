#include"Core/Common/pearl_pch.h"

#include "Renderer/Resources/Material.h"

#include "Renderer/Resources/Texture2D.h"
#include "Renderer/Resources/Cubemap.h"

#include "Core/Resources/ResourceSystem.h"

//Const Pearl engine shader uniforms 
#define TEXOFFSET_UNIFORM "albedoMap_offset"
#define TEXSCALE_UNIFORM "albedoMap_scale"
#define COLOR_UNIFORM "albedoValue"

using namespace PrRenderer;
using namespace PrCore::Utils;

Material::Material(ShaderHandle p_shader)
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
			m_textures[uniformName] = TextureHandle{ Texture2D::CreateUnitTex(PrRenderer::Core::Color::Black) };
		if (uniform.type == UniformType::Cubemap)
			m_cubemaps[uniformName] = CubemapHandle{ Cubemap::CreateUnitTex(Core::Color::Black) };
	}

	if (blackTexture == nullptr)
		blackTexture = Texture2D::CreateUnitTex(Core::Color::Black);
	if (blackCubemap == nullptr)
		blackCubemap = Cubemap::CreateUnitTex(Core::Color::Black);
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
	if (blackCubemap == nullptr)
		blackCubemap = Cubemap::CreateUnitTex(Core::Color::Black);
}

Material::Material() :
	m_shader(ShaderHandle{}),
	m_renderType(RenderType::Opaque),
	m_renderOrder(0)
{
	if (blackTexture == nullptr)
		blackTexture = Texture2D::CreateUnitTex(Core::Color::Black);
	if (blackCubemap == nullptr)
		blackCubemap = Cubemap::CreateUnitTex(Core::Color::Black);
}

void Material::SetColor(const Core::Color& p_color)
{
	SetProperty<PrCore::Math::vec4>(COLOR_UNIFORM, p_color);
}

const PrRenderer::Core::Color& Material::GetColor() const
{
	return static_cast<const PrRenderer::Core::Color&>(GetProperty<PrCore::Math::vec4>(COLOR_UNIFORM));
}

void Material::Bind()
{
	//Bind shader
	m_shader->Bind();

	//Bind Textures
	int texSlot = 0;
	for (auto& texture : m_textures)
	{
		const auto& name = texture.first;
		auto& tex = texture.second;
		if (tex.IsValid())
		{
			tex->Bind(texSlot);
			m_shader->SetUniformInt(name, texSlot);
			texSlot++;
		}
		else
		{
			// Pipeline expects to bind black texture is not assigned
			blackTexture->Bind(texSlot);
			m_shader->SetUniformInt(name, texSlot);
			texSlot++;
		}
	}

	// Bind cubemaps
	for (auto& cubemap : m_cubemaps)
	{
		const auto& name = cubemap.first;
		auto& tex = cubemap.second;
		if (tex.IsValid())
		{
			tex->Bind(texSlot);
			m_shader->SetUniformInt(name, texSlot);
			texSlot++;
		}
		else
		{
			// Pipeline expects to bind black texture is not assigned
			blackTexture->Bind(texSlot);
			m_shader->SetUniformInt(name, texSlot);
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
		if (texture.second.IsValid())
			texture.second->Unbind(texSlot++);
	}

	for (auto& cubemap : m_cubemaps)
	{
		if (cubemap.second.IsValid())
			cubemap.second->Unbind(texSlot++);
	}
}

void Material::SetTexture(const std::string& p_name, TextureHandle p_texture)
{
	auto itTex = m_textures.find(p_name);
	if (itTex != m_textures.end())
	{
		itTex->second = p_texture;
	}
}

void Material::SetCubemap(const std::string& p_name, CubemapHandle p_cubemap)
{
	auto itCubemap = m_cubemaps.find(p_name);
	if (itCubemap != m_cubemaps.end())
	{
		itCubemap->second = p_cubemap;
	}
}

TexturePtr Material::GetTexture(const std::string& p_name)
{
	auto itTex = m_textures.find(p_name);
	if (itTex != m_textures.end())
		return itTex->second.GetData();

	auto itCubemap = m_cubemaps.find(p_name);
	if (itCubemap != m_cubemaps.end())
		return itCubemap->second.GetData();

	PRLOG_WARN("Renderer: Material {0}, missing texture {1}", m_name, p_name);
	return TexturePtr();
}

void Material::SetTexScale(const PrCore::Math::vec2& p_value)
{
	auto find = m_uniforms.find(TEXSCALE_UNIFORM);
	if (find != m_uniforms.end())
		find->second.value = std::make_any<PrCore::Math::vec2>(p_value);
}

void Material::SetTexOffset(const PrCore::Math::vec2& p_value)
{
	auto find = m_uniforms.find(TEXOFFSET_UNIFORM);
	if (find != m_uniforms.end())
		find->second.value = std::make_any<PrCore::Math::vec2>(p_value);
}

PrCore::Math::vec2 Material::GetTexScale() const
{
	auto find = m_uniforms.find(TEXSCALE_UNIFORM);

	if (find != m_uniforms.end())
		return std::any_cast<PrCore::Math::vec2>(find->second.value);

	PRLOG_WARN("Renderer: Material {0}, missing uniform {1}", m_name, TEXSCALE_UNIFORM);
	return PrCore::Math::vec2();
}

PrCore::Math::vec2 Material::GetTexOffset() const
{
	auto find = m_uniforms.find(TEXOFFSET_UNIFORM);

	if (find != m_uniforms.end())
		return std::any_cast<PrCore::Math::vec2>(find->second.value);

	PRLOG_WARN("Renderer: Material {0}, missing uniform {1}", m_name, TEXOFFSET_UNIFORM);
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

size_t Material::GetByteSize() const
{
	return sizeof(Material);
}

void Material::OnSerialize(PrCore::Utils::JSON::json& p_serialized)
{
	p_serialized[SerGuide::version] = 1;
	p_serialized[Material::SerGuide::name] = m_name; // Debug only

	PR_ASSERT(m_shader.IsValid(), "Shader is invalid! That's bad!");
	if (m_shader.GetOrigin() == PrCore::ResourceOrigin::File)
	{
		p_serialized[SerGuide::shader] = m_shader.GetPath();
	}

	p_serialized[SerGuide::renderOrder] = m_renderOrder;
	p_serialized[SerGuide::renderType] = m_renderType;

	PrCore::Utils::JSON::json uniformArray;
	for (auto& uniformPair : m_uniforms)
	{
		const auto& uniformName = uniformPair.first;
		auto& uniform = uniformPair.second;

		// Skip Pipeline Uniforms
		if (uniformName.find("PIPELINE_") != uniformName.npos || uniformName.find("PBR_") != uniformName.npos)
			continue;

		// Uniforms
		PrCore::Utils::JSON::json entry;
		entry[SerGuide::Uniform::typeId] = uniform.type;
		switch (uniform.type)
		{
		case UniformType::Int:
		{
			entry[SerGuide::Uniform::name] = uniformName;
			entry[SerGuide::Uniform::value] = std::any_cast<int>(uniform.value);
			entry[SerGuide::Uniform::type] = "int";
			uniformArray.push_back(entry);

			break;
		}
		case UniformType::Bool:
		{
			entry[SerGuide::Uniform::name] = uniformName;
			entry[SerGuide::Uniform::value] = std::any_cast<bool>(uniform.value);
			entry[SerGuide::Uniform::type] = "bool";
			uniformArray.push_back(entry);
			break;
		}
		case UniformType::Float:
		{
			entry[SerGuide::Uniform::name] = uniformName;
			entry[SerGuide::Uniform::value] = std::any_cast<float>(uniform.value);
			entry[SerGuide::Uniform::type] = "float";
			uniformArray.push_back(entry);
			break;
		}
		case UniformType::Float_Vec2:
		{
			entry[SerGuide::Uniform::name] = uniformName;
			entry[SerGuide::Uniform::value] = PrCore::Utils::JSONParser::ParseVec2(std::any_cast<PrCore::Math::vec2>(uniform.value));
			entry[SerGuide::Uniform::type] = "float2";
			uniformArray.push_back(entry);
			break;
		}
		case UniformType::Float_Vec3:
		{
			entry[SerGuide::Uniform::name] = uniformName;
			entry[SerGuide::Uniform::value] = PrCore::Utils::JSONParser::ParseVec3(std::any_cast<PrCore::Math::vec3>(uniform.value));
			entry[SerGuide::Uniform::type] = "float3";
			uniformArray.push_back(entry);
			break;
		}
		case UniformType::Float_Vec4:
		{
			entry[SerGuide::Uniform::name] = uniformName;
			entry[SerGuide::Uniform::value] = PrCore::Utils::JSONParser::ParseVec4(std::any_cast<PrCore::Math::vec4>(uniform.value));
			entry[SerGuide::Uniform::type] = "float4";
			uniformArray.push_back(entry);
			break;
		}
		case UniformType::Float_Mat3:
		{
			entry[SerGuide::Uniform::name] = uniformName;
			entry[SerGuide::Uniform::value] = PrCore::Utils::JSONParser::ParseMat3(std::any_cast<PrCore::Math::mat3>(uniform.value));
			entry[SerGuide::Uniform::type] = "mat3";
			uniformArray.push_back(entry);
			break;
		}
		case UniformType::Float_Mat4:
		{
			entry[SerGuide::Uniform::name] = uniformName;
			entry[SerGuide::Uniform::value] = PrCore::Utils::JSONParser::ParseMat4(std::any_cast<PrCore::Math::mat4>(uniform.value));
			entry[SerGuide::Uniform::type] = "mat4";
			uniformArray.push_back(entry);
			break;
		}
		case UniformType::Texture2D:
		case UniformType::Texture3D:
		case UniformType::Cubemap:
			break;
		default:
			PRLOG_WARN("Renderer: Material {0} Uniform {1} type not supported", m_name, uniformName);
			break;
		}
	}
	p_serialized[SerGuide::uniforms] = uniformArray;


	PrCore::Utils::JSON::json textureArray;
	for (auto& tex : m_textures)
	{
		if (tex.second != nullptr && tex.second.GetOrigin() == PrCore::ResourceOrigin::File)
		{
			PrCore::Utils::JSON::json texture;
			texture[SerGuide::Texture::name] = tex.first;
			texture[SerGuide::Texture::path] = tex.second.GetPath();
			textureArray.push_back(texture);
		}
	}
	p_serialized[SerGuide::textures] = textureArray;


	PrCore::Utils::JSON::json cubemapArray;
	for (auto& tex : m_cubemaps)
	{
		if (tex.second.GetOrigin() == PrCore::ResourceOrigin::File)
		{
			PrCore::Utils::JSON::json cubemap;
			cubemap[SerGuide::Texture::name] = tex.first;
			cubemap[SerGuide::Texture::path] = tex.second.GetPath();
			cubemapArray.push_back(cubemap);
		}
	}
	p_serialized[SerGuide::cubemaps] = cubemapArray;
}

void Material::OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized)
{
	// Reset
	m_shader.Invalidate();
	m_textures.clear();
	m_uniforms.clear();
	m_renderType = RenderType::Opaque;
	m_renderOrder = 0;

	if (p_deserialized[SerGuide::version] != m_materialVersion)
	{
		PR_ASSERT(false, "Cannot load material. Wrong version!");
		return;
	}

	std::string shader = p_deserialized[SerGuide::shader];
	m_shader = PrSystems::Get<PrCore::ResourceSystem>()->Load<Shader>(shader);
	if (m_shader == nullptr)
	{
		PRLOG_WARN("Renderer: Material {0}, shader {1} cannot be found. Default standard unlit", m_name, shader);
		m_shader = PrSystems::Get<PrCore::ResourceSystem>()->Load<Shader>("standard_unlit.shader");
	}

	m_renderOrder = p_deserialized[SerGuide::renderOrder];
	m_renderType = p_deserialized[SerGuide::renderType];

	auto jsonUniforms = p_deserialized[SerGuide::uniforms];
	m_uniforms = m_shader->GetAllUniforms();
	for (auto& uniform : jsonUniforms)
	{
		const std::string name = uniform[SerGuide::Uniform::name];
		auto it = m_uniforms.find(name);
		if (it == m_uniforms.end())
			continue;

		const UniformType type = uniform[SerGuide::Uniform::typeId];
		switch (type)
		{
		case UniformType::Int:
			SetProperty<int>(name, uniform[SerGuide::Uniform::value]);
			break;
		case UniformType::Float:
			SetProperty<float>(name, uniform[SerGuide::Uniform::value]);
			break;
		case UniformType::Float_Vec2:
			SetProperty<PrCore::Math::vec2>(name, PrCore::Utils::JSONParser::ToVec2(uniform[SerGuide::Uniform::value]));
			break;
		case UniformType::Float_Vec3:
			SetProperty<PrCore::Math::vec3>(name, PrCore::Utils::JSONParser::ToVec3(uniform[SerGuide::Uniform::value]));
			break;
		case UniformType::Float_Vec4:
			SetProperty<PrCore::Math::vec4>(name, PrCore::Utils::JSONParser::ToVec4(uniform[SerGuide::Uniform::value]));
			break;
		case UniformType::Float_Mat4:
			SetProperty<PrCore::Math::mat4>(name, PrCore::Utils::JSONParser::ToMat4(uniform[SerGuide::Uniform::value]));
			break;
		case UniformType::Float_Mat3:
			SetProperty<PrCore::Math::mat3>(name, PrCore::Utils::JSONParser::ToMat3(uniform[SerGuide::Uniform::value]));
			break;
		case UniformType::Bool:
			SetProperty<bool>(name, uniform[SerGuide::Uniform::value]);
			break;
		case UniformType::Texture2D:
		case UniformType::Texture3D:
		case UniformType::Cubemap:
			break;
		case UniformType::None:
		default:
			PRLOG_WARN("Renderer: Material {0} Uniform {1} type not supported", m_name, name);
			break;
		}
	}

	// Fill texture vectors
	for (auto& uniform : m_uniforms)
	{
		if (uniform.second.type == UniformType::Texture2D)
		{
			TextureHandle invalidTex{ Texture2D::CreateUnitTex(PrRenderer::Core::Color::Black) };
			m_textures.insert({ uniform.first, invalidTex });
		}
		else if (uniform.second.type == UniformType::Cubemap)
		{
			CubemapHandle invalidTex{ Cubemap::CreateUnitTex(PrRenderer::Core::Color::Black) };
			m_cubemaps.insert({ uniform.first, invalidTex });
		}
	}

	const auto& texArray = p_deserialized[SerGuide::textures];
	for (auto& texture : texArray)
	{
		const std::string name = texture[SerGuide::Texture::name];
		auto tex = PrSystems::Get<PrCore::ResourceSystem>()->Load<Texture>(static_cast<std::string>(texture[SerGuide::Texture::path]));
		if (tex.IsValid())
		{
			m_textures[name] = tex;
		}
	}

	const auto& cubeArray = p_deserialized[SerGuide::cubemaps];
	for (auto& cubemap : cubeArray)
	{
		const std::string name = cubemap[SerGuide::Texture::name];
		auto tex = PrSystems::Get<PrCore::ResourceSystem>()->Load<Cubemap>(static_cast<std::string>(cubemap[SerGuide::Texture::path]));
		if (tex.IsValid())
		{
			m_cubemaps[name] = tex;
		}
	}
}

