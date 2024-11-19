#include "Editor/Assets/FBX/FbxResourceLoader.h"
#include "Editor/Assets/FBX/FBXResource.h"

#include "Core/Filesystem/FileSystem.h"
#include "Core/Resources/ResourceSystem.h"
#include "Core/Utils/PathUtils.h"

#include "Renderer/Resources/Mesh.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <set>

using namespace PrEditor::Assets;
using namespace PrRenderer::Resources;


PrRenderer::Core::Color ToColor(const aiColor4D& p_vec4)
{
	return { p_vec4.r, p_vec4.g, p_vec4.g, p_vec4.a };
}

PrRenderer::Core::Color ToColor(const aiColor3D& p_vec4)
{
	return { p_vec4.r, p_vec4.g, p_vec4.g, 1.0f };
}

PrCore::Math::vec3 ToVec3(const aiVector3D& p_vec3)
{
	return { p_vec3.x, p_vec3.y, p_vec3.z };
}

PrCore::Math::quat ToQuat(const aiQuaternion& p_quat)
{
	return { p_quat.w, p_quat.x, p_quat.y, p_quat.z };
}

glm::mat4 ToMat4(const aiMatrix4x4& p_mat) {

	glm::mat4 retMat;

	retMat[0][0] = p_mat.a1; retMat[1][0] = p_mat.a2; retMat[2][0] = p_mat.a3; retMat[3][0] = p_mat.a4;
	retMat[0][1] = p_mat.b1; retMat[1][1] = p_mat.b2; retMat[2][1] = p_mat.b3; retMat[3][1] = p_mat.b4;
	retMat[0][2] = p_mat.c1; retMat[1][2] = p_mat.c2; retMat[2][2] = p_mat.c3; retMat[3][2] = p_mat.c4;
	retMat[0][3] = p_mat.d1; retMat[1][3] = p_mat.d2; retMat[2][3] = p_mat.d3; retMat[3][3] = p_mat.d4;

	return retMat;
}

LightType ToLightType(aiLightSourceType p_sourceType)
{
	switch (p_sourceType)
	{
	case aiLightSource_DIRECTIONAL:
		return LightType::Directional;
		break;
	case aiLightSource_POINT:
		return LightType::Point;
		break;
	case aiLightSource_SPOT:
		return LightType::Spot;
		break;
	case aiLightSource_AMBIENT:
	case aiLightSource_AREA:
	case _aiLightSource_Force32Bit:
	case aiLightSource_UNDEFINED:
		return LightType::Unknown;
	}
}

////////////////////////////////////////////////////
struct FbxLoaderHelper
{
	std::unordered_map<uint64_t, PrRenderer::Resources::MaterialHandle> materialMap;
	std::unordered_map<uint64_t, PrRenderer::Resources::MeshHandle>     meshMap;

	std::vector<const aiMesh*>     meshes;
	std::vector<const aiMaterial*> materials;
	std::vector<const aiLight*>    lights;

	PrRenderer::Resources::MaterialHandle GetOrCreateMaterial(const aiMaterial* p_material);
	PrRenderer::Resources::MeshHandle     GetOrCreateMesh(const aiNode* p_mesh);
	LightPtr                              CreateLight(const aiLight* p_light);

	void GatherMaterials(const aiScene* p_scene);
	void GatherMeshes(const aiScene* p_scene);
	void GatherLights(const aiScene* p_scene);

	void CreateEntityGraph(const aiNode* p_objectNode, FbxEntityNode* p_EntityNode);
	void CreateEntityGraphRecursive(const aiNode* p_objectNode, FbxEntityNode* p_EntityNode, int depth = 0);
};

PrRenderer::Resources::MaterialHandle FbxLoaderHelper::GetOrCreateMaterial(const aiMaterial* p_material)
{
	PR_ASSERT(p_material);

	// Check if already created
	auto it = materialMap.find((uint64_t)p_material);
	if (it != materialMap.end())
		return it->second;

	auto shaderRes = PrCore::Resources::ResourceSystem::GetInstance().Load<Shader>("Deferred/StandardLit.shader");
	PR_ASSERT(shaderRes != nullptr, "Cannot load standard shader!");

	auto materialData = std::make_shared<Material>(shaderRes.GetData());
	materialData->SetName(p_material->GetName().C_Str());

	aiString texPath;
	materialData->SetProperty("albedoValue", PrCore::Math::vec4{ 0.5f });
	if (AI_SUCCESS == p_material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
	{
		auto tex = PrCore::Resources::ResourceSystem::GetInstance().Load<Texture>(texPath.C_Str());
		if (tex.IsValid())
		{
			materialData->SetTexture("albedoMap", tex.GetData());
			materialData->SetProperty("albedoValue", PrCore::Math::vec4{ 0.0f });
		}
	}

	materialData->SetProperty("normalMapping", false);
	if (AI_SUCCESS == p_material->GetTexture(aiTextureType_NORMALS, 0, &texPath))
	{
		auto tex = PrCore::Resources::ResourceSystem::GetInstance().Load<Texture>(texPath.C_Str());
		if (tex.IsValid())
		{
			materialData->SetProperty("normalMapping", true);
			materialData->SetTexture("normalMap", tex.GetData());
		}
	}

	materialData->SetProperty("roughnessValue", 0.5f);
	if (AI_SUCCESS == p_material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texPath))
	{
		auto tex = PrCore::Resources::ResourceSystem::GetInstance().Load<Texture>(texPath.C_Str());
		if (tex.IsValid())
		{
			materialData->SetTexture("roughnessMap", tex.GetData());
			materialData->SetProperty("roughnessValue", 0.0f);
		}
	}

	materialData->SetProperty("metallicValue", 0.3f);
	if (AI_SUCCESS == p_material->GetTexture(aiTextureType_METALNESS, 0, &texPath))
	{
		auto tex = PrCore::Resources::ResourceSystem::GetInstance().Load<Texture>(texPath.C_Str());
		if (tex.IsValid())
		{
			materialData->SetTexture("metallicMap", tex.GetData());
			materialData->SetProperty("metallicValue", 0.0f);
		}
	}

	materialData->SetProperty("aoValue", 1.0f);
	if (AI_SUCCESS == p_material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texPath))
	{
		auto tex = PrCore::Resources::ResourceSystem::GetInstance().Load<Texture>(texPath.C_Str());
		if (tex.IsValid())
		{
			materialData->SetTexture("aoMap", tex.GetData());
			materialData->SetProperty("aoValue", 0.0f);
		}
	}


	materialData->SetProperty("emissionInt", 0.0f);
	materialData->SetProperty("emissionColor", PrCore::Math::vec3{ 0.0f });
	if (AI_SUCCESS == p_material->GetTexture(aiTextureType_EMISSION_COLOR, 0, &texPath))
	{
		auto tex = PrCore::Resources::ResourceSystem::GetInstance().Load<Texture>(texPath.C_Str());
		if (tex.IsValid())
		{
			materialData->SetTexture("emissionMap", tex.GetData());
			materialData->SetProperty("emissionInt", 5.0f);
		}
	}


	auto materialHandle = PrCore::Resources::ResourceSystem::GetInstance().Register<Material>(materialData);
	materialMap.insert({ (uint64_t)p_material, materialHandle });
	return materialHandle;
}

PrRenderer::Resources::MeshHandle FbxLoaderHelper::GetOrCreateMesh(const aiNode* p_mesh)
{
	PR_ASSERT(p_mesh);

	// Check if already created
	auto it = meshMap.find((uint64_t)p_mesh);
	if (it != meshMap.end())
		return it->second;

	auto meshData = Mesh::Create();
	meshData->SetName(p_mesh->mName.C_Str());

	std::vector<PrCore::Math::vec3>      verticesVec;
	std::vector<PrCore::Math::vec3>      normalsVec;
	std::vector<unsigned int>            indicesVec;
	std::vector<PrCore::Math::vec2>      UVsVec;
	//std::vector<PrRenderer::Core::Color> colorsVec;

	unsigned int indicesOffset = 0;
	for (int i = 0; i < p_mesh->mNumMeshes; ++i)
	{
		auto mesh = meshes[p_mesh->mMeshes[i]];
		for (int vertId = 0; vertId < mesh->mNumVertices; ++vertId)
		{
			if (mesh->HasPositions())
			{
				verticesVec.push_back(ToVec3(mesh->mVertices[vertId]));
			}

			if (mesh->HasNormals())
			{
				normalsVec.push_back(ToVec3(mesh->mNormals[vertId]));
			}

			if (mesh->HasTextureCoords(0))
			{
				UVsVec.push_back(ToVec3(mesh->mTextureCoords[0][vertId]));
			}

			// Do not use colors for now
			//if (mesh->HasVertexColors(0))
			//{
			//	colorsVec.push_back(PrRenderer::Core::Color::White);
			//}
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace& face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j)
			{
				indicesVec.push_back(face.mIndices[j] + indicesOffset);
			}
		}

		indicesOffset = verticesVec.size();
	}

	meshData->SetVertices(std::move(verticesVec));
	meshData->SetNormals(std::move(normalsVec));
	meshData->SetUVs(0, std::move(UVsVec));
	meshData->SetIndices(std::move(indicesVec));
	//meshData->SetColors(std::move(colorsVec));

	meshData->UpdateBuffers();
	if (!meshData->ValidateBuffers())
	{
		PRLOG_ERROR("Renderer: Mesh {0} invalid", meshData->GetName());
		return PrRenderer::Resources::MeshHandle{};;
	}

	auto meshHandle = PrCore::Resources::ResourceSystem::GetInstance().Register<Mesh>(meshData);
	meshMap.insert({ (uint64_t)p_mesh, meshHandle });

	return meshHandle;
}

void FbxLoaderHelper::GatherMaterials(const aiScene* p_scene)
{
	for (int i = 0; i < p_scene->mNumMaterials; i++)
	{
		materials.push_back(p_scene->mMaterials[i]);
	}
}

void FbxLoaderHelper::GatherMeshes(const aiScene* p_scene)
{
	for (int i = 0; i < p_scene->mNumMeshes; i++)
	{
		meshes.push_back(p_scene->mMeshes[i]);
	}
}

void FbxLoaderHelper::GatherLights(const aiScene* p_scene)
{
	for (int i = 0; i < p_scene->mNumLights; i++)
	{
		lights.push_back(p_scene->mLights[i]);
	}
}

void FbxLoaderHelper::CreateEntityGraph(const aiNode* p_node, FbxEntityNode* p_EntityNode)
{
	FbxEntity* fbxEntity = new FbxEntity();
	aiVector3D transform;
	aiQuaternion rotation;
	aiVector3D scale;
	p_node->mTransformation.Decompose(scale, rotation, transform);
	fbxEntity->position = ToVec3(transform);
	fbxEntity->rotation = ToQuat(rotation);
	fbxEntity->scale = ToVec3(scale);

	p_EntityNode->entity = fbxEntity;

	for (int i = 0; i < p_node->mNumChildren; i++)
	{
		CreateEntityGraphRecursive(p_node->mChildren[i], p_EntityNode, 1);
	}
}

void FbxLoaderHelper::CreateEntityGraphRecursive(const aiNode* p_node, FbxEntityNode* p_EntityNode, int p_depth)
{
	PR_ASSERT(p_depth < FbxEntityGraph::s_maxDepth, "Looks like the FBX file has graph is too deep");

	if (p_EntityNode == nullptr)
		return;

	if (p_node == nullptr)
		return;

	//CreateEntity
	FbxEntity* fbxEntity = new FbxEntity();
	fbxEntity->name = p_node->mName.C_Str();

	aiVector3D transform;
	aiQuaternion rotation;
	aiVector3D scale;
	p_node->mTransformation.Decompose(scale, rotation, transform);
	fbxEntity->position = ToVec3(transform);
	fbxEntity->rotation = ToQuat(rotation);
	fbxEntity->scale = ToVec3(scale);

	// Get mesh
	if (p_node->mNumMeshes > 0)
	{
		fbxEntity->mesh = GetOrCreateMesh(p_node);
	}

	// Get material
	for (int i = 0; i < p_node->mNumMeshes; i++)
	{
		auto meshIndex = p_node->mMeshes[i];
		auto materialIndex = meshes[meshIndex]->mMaterialIndex;
		auto material = materials[materialIndex];

		fbxEntity->materials.push_back(GetOrCreateMaterial(material));
	}

	// Get Light
	auto it = std::find_if(lights.begin(), lights.end(), [&nodeName = p_node->mName](const aiLight* p_light)
	{
		return nodeName == p_light->mName;
	});

	if (it != lights.end())
	{
		fbxEntity->light = CreateLight(*it);
	}

	//Create node
	FbxEntityNode* fbxNode = new FbxEntityNode();
	fbxNode->parent = p_EntityNode;
	fbxNode->nodePath = PrCore::PathUtils::MakePath(p_EntityNode->nodePath, p_node->mName.C_Str());
	fbxNode->entity = fbxEntity;

	p_EntityNode->children.push_back(fbxNode);
	p_depth++;
	for (int i = 0; i < p_node->mNumChildren; i++)
	{
		CreateEntityGraphRecursive(p_node->mChildren[i], fbxNode, p_depth);
	}
}

LightPtr FbxLoaderHelper::CreateLight(const aiLight* p_light)
{
	PR_ASSERT(p_light);

	auto type = ToLightType(p_light->mType);
	if (type == LightType::Unknown)
		return nullptr;


	auto light = std::make_shared<Light>();
	light->SetType(type);
	light->SetColor(ToColor(p_light->mColorDiffuse));
	light->SetInnerCone(PrCore::Math::degrees(p_light->mAngleInnerCone));
	light->SetOutterCone(PrCore::Math::degrees(p_light->mAngleOuterCone));

	// Default
	light->SetRange(10.0f);

	return light;
}

PrCore::Resources::IResourceDataPtr FbxResourceLoader::LoadResource(const std::string& p_path)
{
	auto file = PrCore::Filesystem::FileSystem::GetInstance().OpenFileStream(p_path.c_str());

	if (file == nullptr)
		return nullptr;

	size_t size = file->Tell();
	char* data = new char[file->GetSize()];
	file->Read(data);

	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	auto scene = importer.ReadFileFromMemory(data, file->GetSize(), aiProcess_Triangulate | aiProcess_ImproveCacheLocality | aiProcess_GenSmoothNormals, PrCore::PathUtils::GetExtension(p_path).c_str());
	delete[] data;

	FbxLoaderHelper helper;
	helper.GatherMeshes(scene);
	helper.GatherMaterials(scene);
	helper.GatherLights(scene);

	auto root = new FbxEntityNode();
	root->nodePath = PrCore::PathUtils::GetFilenameInPlace(p_path);
	helper.CreateEntityGraph(scene->mRootNode, root);
	root->entity->name = root->nodePath;

	auto fbxEntityGraph = std::make_unique<FbxEntityGraph>(root);

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
	
	auto fbxData = std::make_shared<FbxResource>(std::move(fbxEntityGraph), std::move(materialVec), std::move(meshVec));
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
