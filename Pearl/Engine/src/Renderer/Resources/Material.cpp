#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/Material.h"
#include"Renderer/Resources/ShaderManager.h"
#include"Renderer/Resources/TextureManager.h"

#include"Core/Filesystem/FileSystem.h"



using namespace PrRenderer::Resources;
using namespace PrCore::Utils;

Material::Material(const std::string& p_name, PrCore::Resources::ResourceManager* p_creator, PrCore::Resources::ResourceID p_ID):
	Resources(p_name, p_creator, p_ID),
	m_color(nullptr),
	m_renderType(RenderType::Opaque),
	m_renderOrder(0.f)
{}

Material::~Material()
{
}

void PrRenderer::Resources::Material::Bind()
{
	//Bind shader
	m_shader->Bind();

	//Bind color, color is the specific uniform that is explisitly set in the material it 
	//change diffuse color always do not use that to other things 
	if (m_color)
		m_shader->SetUniformVec4("color", *m_color);

	//Bind Textures
	int texSlot = 0;
	for (auto texture : m_textures)
	{
		auto textureValue = texture.second;
		m_shader->SetUniformVec2(textureValue.name + "_Scale", textureValue.scale);
		m_shader->SetUniformVec2(textureValue.name + "_Offset", textureValue.offset);

		textureValue.resource->Bind(texSlot);
		texSlot++;
	}

	//Bind Properties
	for (auto uniform : m_unforms)
	{
		auto uniformValue = uniform.second;

		if (!uniformValue.value.has_value())
			continue;

		if(uniformValue.name == "color")
			continue;

		switch (uniformValue.type)
		{
		case UniformType::Float:
			m_shader->SetUniformFloat(uniformValue.name, std::any_cast<float>(uniformValue.value));
			break;
		case UniformType::Float_Vec2:
			m_shader->SetUniformVec2(uniformValue.name, std::any_cast<PrCore::Math::vec2>(uniformValue.value));
			break;
		case UniformType::Float_Vec3:
			m_shader->SetUniformVec3(uniformValue.name, std::any_cast<PrCore::Math::vec3>(uniformValue.value));
			break;
		case UniformType::Float_Vec4:
			m_shader->SetUniformVec4(uniformValue.name, std::any_cast<PrCore::Math::vec4>(uniformValue.value));
			break;
		case UniformType::Float_Mat3:
			m_shader->SetUniformMat3(uniformValue.name, std::any_cast<PrCore::Math::mat3>(uniformValue.value));
			break;
		case UniformType::Float_Mat4:
			m_shader->SetUniformMat4(uniformValue.name, std::any_cast<PrCore::Math::mat4>(uniformValue.value));
			break;
		}
	}
}

void PrRenderer::Resources::Material::Unbind()
{
	m_shader->Unbind();
}

void Material::SetTexture(const std::string& p_name, TexturePtr p_texture)
{
	auto find = m_textures.find(p_name);
	if(find != m_textures.end())
		find->second.resource = p_texture;
}

TexturePtr Material::GetTexture(const std::string& p_name)
{
	auto find = m_textures.find(p_name);
	
	if (find != m_textures.end())
		return find->second.resource;

	return TexturePtr();
}

void Material::SetTexScale(const std::string& p_name, const PrCore::Math::vec2& p_value)
{
	auto find = m_textures.find(p_name);
	if (find != m_textures.end())
		find->second.scale = p_value;
}

void Material::SetTexOffset(const std::string& p_name, const PrCore::Math::vec2& p_value)
{
	auto find = m_textures.find(p_name);
	if (find != m_textures.end())
		find->second.scale = p_value;
}

PrCore::Math::vec2 Material::GetTexScale(const std::string& p_name)
{
	auto find = m_textures.find(p_name);

	if (find != m_textures.end())
		return find->second.scale;

	return PrCore::Math::vec2();
}

PrCore::Math::vec2 Material::GetTexOffset(const std::string& p_name)
{
	auto find = m_textures.find(p_name);

	if (find != m_textures.end())
		return find->second.offset;

	return PrCore::Math::vec2();
}

bool PrRenderer::Resources::Material::HasProperty(const std::string& p_name)
{
	return m_unforms.find(p_name) != m_unforms.end();
}

void Material::PreLoadImpl()
{
}

bool PrRenderer::Resources::Material::LoadImpl()
{
	auto jsonFile = ReadFile();
	if (jsonFile == nullptr)
		return false;

	auto result = PopulateMaterial(jsonFile);

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
	m_unforms.clear();
	if (m_color)
		delete m_color;
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

bool Material::PopulateMaterial(JSON::json& p_json)
{
	float version = p_json["materialVersion"];
	if (version != m_materialVersion)
	{
		PRLOG_ERROR("Renderer: Material {0} wrong version", m_name);
		return false;
	}
	
	std::string shader = p_json["shader"];
	m_shader = std::static_pointer_cast<Shader>(PrRenderer::Resources::ShaderManager::GetInstance().Load(shader));

	m_renderOrder = p_json["RenderOrder"];
	m_renderType = p_json["RenderType"];

	if (p_json.contains("color"))
		m_color = new Core::Color(JSONParser::ToColor(p_json["color"]));

	m_unforms = m_shader->GetAllUniforms();
	int textureCount = 0;
	for (auto uniformPair : m_unforms)
	{
		auto uniform = uniformPair.second;
		std::string propertiesList;
		switch (uniform.type)
		{
		case UniformType::Int:
		{
			auto properties = p_json["intproperties"];
			for (auto uniformProperty : properties)
			{
				std::string name = uniformProperty["name"];
				if (name == uniform.name)
					uniform.value = std::make_any<int>(uniformProperty["value"]);
			}
			break;
		}
		case UniformType::Float:
		{
			auto properties = p_json["floatproperties"];
			for (auto uniformProperty : properties)
			{
				std::string name = uniformProperty["name"];
				if (name == uniform.name)
				uniform.value = std::make_any<float>(uniformProperty["value"]);
			}
			break;
		}
		case UniformType::Float_Vec2:
		{
			auto properties = p_json["float2properties"];
			for (auto uniformProperty : properties)
			{
				std::string name = uniformProperty["name"];
				if (name == uniform.name)
				{
					auto vec2 = PrCore::Utils::JSONParser::ToVec2(uniformProperty["value"]);
					uniform.value = std::make_any<PrCore::Math::vec2>(vec2);
				}
			}
			break;
		}
		case UniformType::Float_Vec3:
		{
			auto properties = p_json["float3properties"];
			for (auto uniformProperty : properties)
			{
				std::string name = uniformProperty["name"];
				if (name == uniform.name)
				{
					auto vec3 = PrCore::Utils::JSONParser::ToVec3(uniformProperty["value"]);
					uniform.value = std::make_any<PrCore::Math::vec3>(vec3);
				}
			}
			break;
		}
		case UniformType::Float_Vec4:
		{
			auto properties = p_json["float4properties"];
			for (auto uniformProperty : properties)
			{
				std::string name = uniformProperty["name"];
				if (name == uniform.name)
				{
					auto vec4 = PrCore::Utils::JSONParser::ToVec4(uniformProperty["value"]);
					uniform.value = std::make_any<PrCore::Math::vec4>(vec4);
				}
			}
			break;
		}
		case UniformType::Float_Mat3:
		{
			auto properties = p_json["mat3properties"];
			for (auto uniformProperty : properties)
			{
				std::string name = uniformProperty["name"];
				if (name == uniform.name)
				{
					auto mat3 = PrCore::Utils::JSONParser::ToMat3(uniformProperty["value"]);
					uniform.value = std::make_any<PrCore::Math::mat3>(mat3);
				}
			}
			break;
		}
		case UniformType::Float_Mat4:
		{
			auto properties = p_json["mat4properties"];
			for (auto uniformProperty : properties)
			{
				std::string name = uniformProperty["name"];
				if (name == uniform.name)
				{
					auto mat4 = PrCore::Utils::JSONParser::ToMat4(uniformProperty["value"]);
					uniform.value = std::make_any<PrCore::Math::mat4>(mat4);
				}
			}
			break;
		}
		case UniformType::Cubemap:
		case UniformType::Texture2D:
		case UniformType::Texture3D:
			textureCount++;
			break;

		default:
			PRLOG_ERROR("Renderer: Material {0} Uniform {1} type not supported", m_name, uniform.name);
			return false;
			break;
		}
	}

	
	auto texVector = p_json["textures"];
	if (texVector.size() != textureCount)
	{
		PRLOG_ERROR("Renderer: Material {0} Texture number do not mach shader texture number", m_name);
		return false;
	}

	for (auto it = texVector.cbegin(); it != texVector.cend(); it++)
	{
		auto texture = it.value();

		TextureData texData;
		texData.name = texture["texName"];
		texData.scale = JSONParser::ToVec2(texture["texScale"]);
		texData.offset = JSONParser::ToVec2(texture["texOffset"]);

		auto texPath = texture["texPath"];
		texData.resource = std::static_pointer_cast<Texture>(PrRenderer::Resources::TextureManager::GetInstance().Load(texPath));

		m_textures[texData.name] = texData;
	}

	return true;
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
