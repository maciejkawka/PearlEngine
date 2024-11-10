#include "Editor/Assets/FBX/FbxResourceLoader.h"
#include "Editor/Assets/FBX/FBXResource.h"

#include "Core/Filesystem/FileSystem.h"
#include "Core/Resources/ResourceSystem.h"
#include "Core/Utils/PathUtils.h"

#include "Renderer/Resources/Mesh.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Texture.h"

#include <set>
#include "ofbx.h"

using namespace PrEditor::Assets;
using namespace PrRenderer::Resources;


struct Vec2Compare {
	bool operator()(const glm::vec2& a, const glm::vec2& b) const {
		if (a.x != b.x) return a.x < b.x;  // Compare x first
		return a.y < b.y;                  // If x is equal, compare y
	}
};

//Hash Function for map
template <typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
{
	seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
	(hash_combine(seed, rest), ...);
}

struct VertexProxy
{
	PrCore::Math::vec3 pos;
	PrCore::Math::vec3 normal;
	PrCore::Math::vec2 uv;
	PrRenderer::Core::Color color;

	VertexProxy()
	{
		pos = PrCore::Math::vec3(0.f);
		normal = PrCore::Math::vec3(0.f);
		uv = PrCore::Math::vec2(0.f);
		color = PrRenderer::Core::Color::Black;
	}

	bool operator==(const VertexProxy& vertex) const
	{
		return vertex.uv == this->uv && vertex.pos == this->pos &&
			vertex.normal == this->normal && vertex.color == this->color;
	}
};

namespace std {
	template<> struct hash<VertexProxy> {
		size_t operator()(VertexProxy const& vertex) const {
			size_t seed = 0;
			hash_combine(seed, vertex.pos, (PrCore::Math::vec3)vertex.color, vertex.uv, vertex.normal);
			return seed;
		}
	};
}

PrCore::Math::vec4 ColorToVec4(const ofbx::Color& p_color)
{
	return { p_color.r, p_color.g, p_color.b, 1.0f };
}

PrCore::Math::vec4 ToVec4(const ofbx::Vec4& p_vec4)
{
	return { p_vec4.x, p_vec4.y, p_vec4.z, p_vec4.w };
}

PrCore::Math::vec3 ToVec3(const ofbx::Vec3& p_vec3)
{
	return { p_vec3.x, p_vec3.y, p_vec3.z };
}

PrCore::Math::vec3 ToVec3(const ofbx::DVec3& p_vec3)
{
	return { p_vec3.x, p_vec3.y, p_vec3.z };
}

PrCore::Math::vec2 ToVec2(const ofbx::Vec2& p_vec2)
{
	return { p_vec2.x, p_vec2.y };
}

glm::mat4 ToMat4(const ofbx::DMatrix& openFBXMatrix) {

	const double* m = openFBXMatrix.m;
	glm::mat4 mat(
		static_cast<float>(m[0]), static_cast<float>(m[1]), static_cast<float>(m[2]), static_cast<float>(m[3]),
		static_cast<float>(m[4]), static_cast<float>(m[5]), static_cast<float>(m[6]), static_cast<float>(m[7]),
		static_cast<float>(m[8]), static_cast<float>(m[9]), static_cast<float>(m[10]), static_cast<float>(m[11]),
		static_cast<float>(m[12]), static_cast<float>(m[13]), static_cast<float>(m[14]), static_cast<float>(m[15])
	);

	return mat;
}

PrRenderer::Resources::LightType ToPearlLightType(ofbx::Light::LightType p_lightType)
{
	switch (p_lightType)
	{
	case ofbx::Light::LightType::POINT:
		return LightType::Point;
		break;
	case ofbx::Light::LightType::DIRECTIONAL:
		return LightType::Directional;
		break;
	case ofbx::Light::LightType::SPOT:
		return LightType::Spot;
		break;
	default:
		return LightType::Point;
		break;
	}
}

////////////////////////////////////////////////////
struct FbxLoaderHelper
{
	std::unordered_map<uint64_t, PrRenderer::Resources::MaterialHandle> materialMap;
	std::unordered_map<uint64_t, PrRenderer::Resources::MeshHandle>     meshMap;
	std::unordered_map<uint64_t, PrRenderer::Resources::TextureHandle>  textureMap;

	PrRenderer::Resources::TextureHandle GetOrCreateTexture(const ofbx::Texture* p_texture);
	PrRenderer::Resources::MaterialHandle GetOrCreateMaterial(const ofbx::Material* p_material);
	PrRenderer::Resources::MeshHandle GetOrCreateMesh(const ofbx::Mesh* p_mesh);

	void CreateEntityHierarchy(const ofbx::Object* p_objectNode, FbxEntityNode* p_EntityNode, int depth = 0);
};

PrRenderer::Resources::TextureHandle FbxLoaderHelper::GetOrCreateTexture(const ofbx::Texture* p_texture)
{
	PR_ASSERT(p_texture);

	auto it = textureMap.find(p_texture->id);
	if (it != textureMap.end())
		return it->second;

	char path[MAX_PATH];
	p_texture->getFileName().toString(path);
	auto textureHandle = PrCore::Resources::ResourceSystem::GetInstance().Load<Texture>(path);
	textureMap.insert({ p_texture->id, textureHandle });

	if (textureHandle.GetState() == PrCore::Resources::ResourceState::Corrupted)
		PRLOG_WARN("Texture {} is Corrupted!", std::string(path).c_str());

	return textureHandle;
}

PrRenderer::Resources::MaterialHandle FbxLoaderHelper::GetOrCreateMaterial(const ofbx::Material* p_material)
{
	PR_ASSERT(p_material);

	auto it = materialMap.find(p_material->id);
	if (it != materialMap.end())
		return it->second;

	// Material not created yet, create one

	// By default FBX files are loaded with StandarLit shader
	auto shaderRes = PrCore::Resources::ResourceSystem::GetInstance().Load<Shader>("Deferred/StandardLit.shader");
	PR_ASSERT(shaderRes != nullptr, "Cannot load standard shader!");

	auto materialData = std::make_shared<Material>(shaderRes.GetData());
	materialData->SetName(p_material->name);

	// Values
	materialData->SetProperty<PrCore::Math::vec4>("albedoValue", ColorToVec4(p_material->getDiffuseColor()));
	materialData->SetProperty<float>("metallicValue", PrCore::Math::max(0.0f, (float)p_material->getSpecularFactor()));
	materialData->SetProperty<float>("roughnessValue", 1.0f - PrCore::Math::max(0.0f, (float)p_material->getShininess()));
	materialData->SetProperty<float>("aoValue", PrCore::Math::max(0.0f, (float)p_material->getAmbientFactor()));
	materialData->SetProperty<float>("emissionInt", PrCore::Math::max(0.0f, (float)p_material->getEmissiveFactor()));
	materialData->SetProperty<PrCore::Math::vec3>("emissionColor", PrCore::Math::vec3{ ColorToVec4(p_material->getDiffuseColor()) });

	// Textures

	// Conversion Assumtions
	//   FBX ------ PBR
	// DIFFUSE   -> DIFFUSE
	// NORMAL    -> NORMAL
	// SPECULAR  -> METALLIC
	// SHININESS -> ROUGHNESS (Texture has to prpeared, factor is calculated 1 - value)
	// EMISSIVE  -> EMISSION
	// AMBIENT   -> AO

	auto texPtr = p_material->getTexture(ofbx::Texture::DIFFUSE);
	if (texPtr != nullptr)
	{
		auto texture = GetOrCreateTexture(texPtr);
		if (texture != nullptr)
		{
			materialData->SetTexture("albedoMap", texture.GetData());
		}
	}
	
	texPtr = p_material->getTexture(ofbx::Texture::NORMAL);
	materialData->SetProperty("normalMapping", texPtr != nullptr);
	if (texPtr != nullptr)
	{
		auto texture = GetOrCreateTexture(texPtr);
		if (texture != nullptr)
		{
			materialData->SetTexture("normalMap", texture.GetData());
		}

	}

	texPtr = p_material->getTexture(ofbx::Texture::SPECULAR);
	if (texPtr != nullptr)
	{
		auto texture = GetOrCreateTexture(texPtr);
		if (texture != nullptr)
		{
			materialData->SetTexture("metallicMap", texture.GetData());
		}
	}

	texPtr = p_material->getTexture(ofbx::Texture::SHININESS);
	if (texPtr != nullptr)
	{
		auto texture = GetOrCreateTexture(texPtr);
		if (texture != nullptr)
		{
			materialData->SetTexture("roughnessMap", texture.GetData());
		}
	}

	texPtr = p_material->getTexture(ofbx::Texture::EMISSIVE);
	if (texPtr != nullptr)
	{
		auto texture = GetOrCreateTexture(texPtr);
		if (texture != nullptr)
		{
			materialData->SetTexture("emissionMap", texture.GetData());
		}
	}

	texPtr = p_material->getTexture(ofbx::Texture::AMBIENT);
	if (texPtr != nullptr)
	{
		auto texture = GetOrCreateTexture(texPtr);
		if (texture != nullptr)
		{
			materialData->SetTexture("aoMap", texture.GetData());
		}
	}

	auto materialHandle = PrCore::Resources::ResourceSystem::GetInstance().Register<Material>(materialData);
	materialMap.insert({ p_material->id, materialHandle });

	return materialHandle;
}

PrRenderer::Resources::MeshHandle FbxLoaderHelper::GetOrCreateMesh(const ofbx::Mesh* p_mesh)
{
	PR_ASSERT(p_mesh);

	auto it = meshMap.find(p_mesh->id);
	if (it != meshMap.end())
		return it->second;

	auto meshData = Mesh::Create();
	meshData->SetName(p_mesh->name);

	std::vector<PrCore::Math::vec3> verticesVec;
	std::vector<PrCore::Math::vec3> normalsVec;
	std::vector<unsigned int> indicesVec;
	std::vector<PrCore::Math::vec2> UVsVec;

	const ofbx::GeometryData& geom = p_mesh->getGeometryData();
	const ofbx::Vec3Attributes positions = geom.getPositions();
	const ofbx::Vec3Attributes normals = geom.getNormals();
	const ofbx::Vec2Attributes uvs = geom.getUVs(0);

	// Mesh vertices might be translated by geompetry matrix
	// Transpose vertices positions, this might be probelm with normals but fuck it for now
	auto geompetryMatrix = ToMat4(p_mesh->getGeometricMatrix());

	std::unordered_map<VertexProxy, unsigned int> vertexMap;

	for (int partition_idx = 0; partition_idx < geom.getPartitionCount(); ++partition_idx)
	{
		const ofbx::GeometryPartition& partition = geom.getPartition(partition_idx);
		for (int polygon_idx = 0; polygon_idx < partition.polygon_count; ++polygon_idx)
		{
			const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygon_idx];

			int tri_indices[32];
			auto indices = ofbx::triangulate(geom, polygon, tri_indices);

			for (int i = 0; i < indices; i++)
			{
				VertexProxy vert;

				vert.pos = geompetryMatrix * PrCore::Math::vec4(ToVec3(positions.get(tri_indices[i])), 1.0f);

				if (normals.values != nullptr)
				{
					vert.normal = ToVec3(normals.get(tri_indices[i]));
				}

				if (uvs.values != nullptr)
				{
					vert.uv = ToVec2(uvs.get(tri_indices[i]));
				}

				if (vertexMap.find(vert) == vertexMap.end())
				{
					verticesVec.push_back(vert.pos);

					if (normals.values != nullptr)
					{
						normalsVec.push_back(vert.normal);
					}

					if (uvs.values != nullptr)
					{
						UVsVec.push_back(vert.uv);
					}

					vertexMap[vert] = (unsigned int)verticesVec.size() - 1;
				}

				indicesVec.push_back(vertexMap[vert]);
			}
		}
	}

	meshData->SetVertices(std::move(verticesVec));
	meshData->SetNormals(std::move(normalsVec));
	meshData->SetUVs(0, std::move(UVsVec));
	meshData->SetIndices(std::move(indicesVec));

	meshData->UpdateBuffers();
	if (!meshData->ValidateBuffers())
	{
		PRLOG_ERROR("Renderer: Mesh {0} invalid", meshData->GetName());
		return PrRenderer::Resources::MeshHandle{};;
	}

	auto meshHandle = PrCore::Resources::ResourceSystem::GetInstance().Register<Mesh>(meshData);
	meshMap.insert({ p_mesh->id, meshHandle });
	return meshHandle;
}

void FbxLoaderHelper::CreateEntityHierarchy(const ofbx::Object* p_objectNode, FbxEntityNode* p_EntityNode, int depth)
{
	if (p_objectNode == nullptr)
		return;

	if (depth == 20)
		return;

	PR_ASSERT(p_EntityNode);
	switch (p_objectNode->getType())
	{
	case ofbx::Object::Type::ROOT:
	{
		// Set root
		auto fbxEntity = new FbxEntity();
		fbxEntity->name = p_objectNode->name;
		fbxEntity->position = ToVec3(p_objectNode->getLocalTranslation());
		fbxEntity->scale = ToVec3(p_objectNode->getLocalScaling());
		fbxEntity->rotation = ToVec3(p_objectNode->getLocalRotation());

		p_EntityNode->nodePath = p_objectNode->name;
		p_EntityNode->entity = fbxEntity;
	}
	break;

	case ofbx::Object::Type::MESH: 
	{
		// Create entity and set mesh
		// We asume that mesh node indicates a new entity create
		auto fbxEntity = new FbxEntity();
		fbxEntity->name = p_objectNode->name;
		fbxEntity->position = ToVec3(p_objectNode->getLocalTranslation());
		fbxEntity->scale = ToVec3(p_objectNode->getLocalScaling());
		fbxEntity->rotation = PrCore::Math::quat{ PrCore::Math::radians(ToVec3(p_objectNode->getPreRotation())) }
			*PrCore::Math::quat{ PrCore::Math::radians(ToVec3(p_objectNode->getLocalRotation())) }
		*PrCore::Math::quat{ PrCore::Math::radians(ToVec3(p_objectNode->getPostRotation())) };

		fbxEntity->mesh = GetOrCreateMesh(static_cast<const ofbx::Mesh*>(p_objectNode));

		auto entityNode = new FbxEntityNode();
		entityNode->entity = fbxEntity;
		entityNode->parent = p_EntityNode;
		entityNode->nodePath = PrCore::PathUtils::MakePath(p_EntityNode->nodePath, p_objectNode->name);

		p_EntityNode->children.push_back(entityNode);
		p_EntityNode = entityNode;

		// Gather materials for the mesh
		const auto mesh = static_cast<const ofbx::Mesh*>(p_objectNode);
		const auto& paritionData = mesh->getGeometry()->getGeometryData();
		if (paritionData.getMaterialsCount() != 0)
		{
			// Get material per partition if present
			for (int i = 0; i < paritionData.getMaterialsCount(); ++i)
			{

				auto material = mesh->getMaterial(paritionData.getPartitionMaterialIndex(i));
				p_EntityNode->entity->materials.push_back(GetOrCreateMaterial(material));
			}
		}
		else
		{
			// Get material from mesh
			for (int i = 0; i < mesh->getMaterialCount(); ++i)
			{
				auto material = mesh->getMaterial(i);
				p_EntityNode->entity->materials.push_back(GetOrCreateMaterial(material));
			}
		}
	}
	break;

	case ofbx::Object::Type::LIGHT:
	{
		// Set light to the entity created upon mesh detection
		auto light = std::make_shared<Light>();
		auto lightNode = static_cast<const ofbx::Light*>(p_objectNode);

		light->SetType(ToPearlLightType(lightNode->getLightType()));
		light->SetColor(ColorToVec4(lightNode->getColor()));
		light->SetInnerCone(lightNode->getInnerAngle());
		light->SetOutterCone(lightNode->getOuterAngle());
		light->SetRange(lightNode->getFarAttenuationEnd());

		p_EntityNode->entity->light = light;
	}
	break;

	case ofbx::Object::Type::NULL_NODE:
	{
		// Create empty entity in the hierarchy
		auto fbxEntity = new FbxEntity();
		fbxEntity->name = p_objectNode->name;
		fbxEntity->position = ToVec3(p_objectNode->getLocalTranslation());
		fbxEntity->position.x = -fbxEntity->position.x;
		fbxEntity->scale = ToVec3(p_objectNode->getLocalScaling());
		fbxEntity->rotation = PrCore::Math::quat{ PrCore::Math::radians(ToVec3(p_objectNode->getPreRotation())) }
			*PrCore::Math::quat{ PrCore::Math::radians(ToVec3(p_objectNode->getLocalRotation())) }
			*PrCore::Math::quat{ PrCore::Math::radians(ToVec3(p_objectNode->getPostRotation())) };

		auto entityNode = new FbxEntityNode();
		entityNode->entity = fbxEntity;
		entityNode->parent = p_EntityNode;
		entityNode->nodePath = PrCore::PathUtils::MakePath(p_EntityNode->nodePath, p_objectNode->name);

		p_EntityNode->children.push_back(entityNode);
		p_EntityNode = entityNode;
	}
	break;
	}


	// Loop children
	int i = 0;
	while (ofbx::Object* child = p_objectNode->resolveObjectLink(i))
	{
		CreateEntityHierarchy(child, p_EntityNode, depth + 1);
		i++;
	}
}
////////////////////////////////////////////////////


PrCore::Resources::IResourceDataPtr FbxResourceLoader::LoadResource(const std::string& p_path)
{
	auto file = PrCore::Filesystem::FileSystem::GetInstance().OpenFileStream(p_path.c_str());

	if (file == nullptr)
		return nullptr;

	size_t size = file->Tell();
	char* data = new char[file->GetSize()];
	file->Read(data);

	ofbx::LoadFlags flags =
//		ofbx::LoadFlags::IGNORE_MODELS |
		ofbx::LoadFlags::IGNORE_BLEND_SHAPES |
		ofbx::LoadFlags::IGNORE_CAMERAS |
//		ofbx::LoadFlags::IGNORE_LIGHTS |
//		ofbx::LoadFlags::IGNORE_TEXTURES |
		ofbx::LoadFlags::IGNORE_SKIN |
		ofbx::LoadFlags::IGNORE_BONES |
		ofbx::LoadFlags::IGNORE_PIVOTS |
//		ofbx::LoadFlags::IGNORE_MATERIALS |
		ofbx::LoadFlags::IGNORE_POSES |
		ofbx::LoadFlags::IGNORE_VIDEOS |
		ofbx::LoadFlags::IGNORE_LIMBS |
//		ofbx::LoadFlags::IGNORE_MESHES |
//		ofbx::LoadFlags::IGNORE_GEOMETRY |
		ofbx::LoadFlags::IGNORE_ANIMATIONS;

	ofbx::IScene* scene = ofbx::load(reinterpret_cast<ofbx::u8*>(data), file->GetSize(), static_cast<ofbx::u16>(flags));

	const ofbx::GlobalSettings* settings = scene->getGlobalSettings();

	FbxLoaderHelper helper;
	auto root = new FbxEntityNode();
	auto fbxEntityGraph = std::make_unique<FbxEntityGraph>(root);

	helper.CreateEntityHierarchy(scene->getRoot(), root, 0);
	root->entity->name = PrCore::PathUtils::GetFilename(p_path);

	scene->destroy();
	delete[] data;

	// Convert maps to vectors
	std::vector<MaterialHandle> materialVec;
	materialVec.reserve(helper.materialMap.size());
	std::for_each(helper.materialMap.begin(), helper.materialMap.end(), [&materialVec](auto&& pair) {
		materialVec.push_back(pair.second);
		});

	std::vector<MeshHandle> meshVec;
	materialVec.reserve(helper.meshMap.size());
	std::for_each(helper.meshMap.begin(), helper.meshMap.end(), [&meshVec](auto&& pair) {
		meshVec.push_back(pair.second);
		});

	std::vector<TextureHandle> textureVec;
	materialVec.reserve(helper.textureMap.size());
	std::for_each(helper.textureMap.begin(), helper.textureMap.end(), [&textureVec](auto&& pair) {
		textureVec.push_back(pair.second);
		});

	auto fbxData = std::make_shared<FbxResource>(std::move(fbxEntityGraph), std::move(materialVec), std::move(meshVec), std::move(textureVec));
	return fbxData;
}

void FbxResourceLoader::UnloadResource(PrCore::Resources::IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
}

bool FbxResourceLoader::SaveResourceOnDisc(PrCore::Resources::IResourceDataPtr p_resourceData, const std::string& p_path)
{
	return false;
}
