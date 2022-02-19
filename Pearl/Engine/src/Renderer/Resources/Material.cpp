#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/Material.h"
#include"Renderer/Resources/ShaderManager.h"
#include"Renderer/Resources/TextureManager.h"

#include"Core/Filesystem/FileSystem.h"



using namespace PrRenderer::Resources;
using namespace PrCore::Utils;

Material::Material(const std::string& p_name, PrCore::Resources::ResourceManager* p_creator, PrCore::Resources::ResourceID p_ID):
	Resources(p_name, p_creator, p_ID),
	m_color(Core::Color::White),
	m_renderType(RenderType::Opaque),
	m_renderOrder(0.f)
{}

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
	m_color = PrRenderer::Core::Color::White;
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

	m_color = JSONParser::ToColor(p_json["color"]);

	m_unforms = m_shader->GetAllUniforms();
	int textureCount = 0;
	for (auto uniform : m_unforms)
	{
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
		texData.scale = JSONParser::ToVec3(texture["texScale"]);
		texData.offset = JSONParser::ToVec2(texture["texOffset"]);

		auto texPath = texture["texPath"];
		texData.resource = std::static_pointer_cast<Texture>(PrRenderer::Resources::TextureManager::GetInstance().Load(texPath));

		m_textures.push_back(texData);
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
