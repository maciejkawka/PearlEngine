#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/MeshOBJLoader.h"
#include "Renderer/Resources/Mesh.h"

#include "Core/File/FileSystem.h"
#include "Core/Utils/PathUtils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include"tinyObj/tiny_obj_loader.h"

#include <assimp/Exporter.hpp>
#include <assimp/scene.h>

using namespace PrRenderer;

//Hash Function for map
template <typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
{
	seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
	(hash_combine(seed, rest), ...);
}

struct Vertex
{
	PrCore::Math::vec3 pos;
	PrCore::Math::vec3 normal;
	PrCore::Math::vec2 uv;
	PrRenderer::Core::Color color;

	Vertex()
	{
		pos = PrCore::Math::vec3(0.f);
		normal = PrCore::Math::vec3(0.f);
		uv = PrCore::Math::vec2(0.f);
		color = PrRenderer::Core::Color::Black;
	}

	bool operator==(const Vertex& vertex) const
	{
		return vertex.uv == this->uv && vertex.pos == this->pos &&
			vertex.normal == this->normal && vertex.color == this->color;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			size_t seed = 0;
			hash_combine(seed, vertex.pos, (PrCore::Math::vec3)vertex.color, vertex.uv, vertex.normal);
			return seed;
		}
	};
}

PrCore::IResourceDataPtr MeshOBJLoader::LoadResource(const std::string& p_path)
{
	auto file = PrSystems::Get<PrCore::FileSystem>()->OpenFileWrapper(p_path);
	if (file == nullptr)
		return nullptr;

	char* data = new char[file->GetSize()];
	file->Read(data, file->GetSize());
	std::string objFile = std::string(data, file->GetSize());
	delete[] data;

	tinyobj::ObjReader reader;

	if (!reader.ParseFromString(objFile, std::string(), tinyobj::ObjReaderConfig()))
	{
		PRLOG_ERROR("Renderer: Cannot open mesh file {0}", p_path);
		return nullptr;
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();

	std::unordered_map<Vertex, unsigned int> vertexMap;
	std::vector<unsigned int> indices;
	std::vector<PrCore::Math::vec3> vertices;
	std::vector<PrCore::Math::vec3> normals;
	std::vector<PrCore::Math::vec4> tangents;
	std::vector<Core::Color> colors;
	std::vector<SubMesh> subMeshes;
	Mesh::UVArray UVs;

	size_t expectedVertSize = attrib.vertices.size() / 3;
	vertexMap.reserve(expectedVertSize);
	indices.reserve(expectedVertSize);
	vertices.reserve(expectedVertSize);
	normals.reserve(expectedVertSize);
	tangents.reserve(expectedVertSize);
	colors.reserve(expectedVertSize);
	UVs[0].reserve(expectedVertSize);

	for (const auto& shape : shapes)
	{
		SubMesh submesh;
		submesh.firstIndex = indices.size();

		for (const auto& face : shape.mesh.num_face_vertices)
		{
			if (face != 3)
			{
				PRLOG_ERROR("Renderer: PearlEngine supports only triangle mesh error in mesh {0}", p_path);
				return nullptr;
			}
		}

		for (const auto& index : shape.mesh.indices)
		{
			Vertex vert;

			PrCore::Math::vec3 pos(0.f);
			pos.x = attrib.vertices[3 * index.vertex_index + 0];
			pos.y = attrib.vertices[3 * index.vertex_index + 1];
			pos.z = attrib.vertices[3 * index.vertex_index + 2];
			vert.pos = pos;

			if (!attrib.texcoords.empty())
			{
				PrCore::Math::vec2 uvs(0.f);
				uvs.x = attrib.texcoords[2 * index.texcoord_index + 0];
				uvs.y = attrib.texcoords[2 * index.texcoord_index + 1];
				vert.uv = uvs;
			}

			if (!attrib.normals.empty())
			{
				PrCore::Math::vec3 normal(0.f);
				normal.x = attrib.normals[3 * index.normal_index + 0];
				normal.y = attrib.normals[3 * index.normal_index + 1];
				normal.z = attrib.normals[3 * index.normal_index + 2];
				vert.normal = normal;
			}

			PrRenderer::Core::Color color(PrRenderer::Core::Color::White);
			if (!attrib.colors.empty())
			{
				color.r = attrib.colors[3 * index.vertex_index + 0];
				color.g = attrib.colors[3 * index.vertex_index + 1];
				color.b = attrib.colors[3 * index.vertex_index + 2];
			}
			vert.color = color;

			if (vertexMap.find(vert) == vertexMap.end())
			{
				vertices.push_back(vert.pos);

				if (!attrib.texcoords.empty())
					UVs[0].push_back(vert.uv);

				if (!attrib.normals.empty())
					normals.push_back(vert.normal);

				if (!attrib.colors.empty())
					colors.push_back(vert.color);

				vertexMap[vert] = (unsigned int)vertices.size() - 1;
			}

			indices.push_back(vertexMap[vert]);
		}

		submesh.indicesCount = indices.size() - submesh.firstIndex;
		subMeshes.push_back(submesh);
	}

	MeshPtr mesh = Mesh::Create();
	mesh->SetSubmeshes(subMeshes);
	if (!indices.empty())
		mesh->SetIndices(std::move(indices));
	if (!vertices.empty())
		mesh->SetVertices(std::move(vertices));
	if (!normals.empty())
		mesh->SetNormals(std::move(normals));
	if (!colors.empty())
		mesh->SetColors(std::move(colors));
	if (!UVs[0].empty())
		mesh->SetUVs(0, std::move(UVs[0]));

	mesh->UpdateBuffers();
	if (!mesh->ValidateBuffers())
	{
		PRLOG_ERROR("Renderer: Mesh {0} invalid", mesh->GetName());
		return nullptr;
	}

	return mesh;
}

void MeshOBJLoader::UnloadResource(PrCore::IResourceDataPtr p_resourceData)
{
	PR_ASSERT(p_resourceData, "Pointer invalid!");

	p_resourceData.reset();
	p_resourceData = nullptr;
}

bool MeshOBJLoader::SaveResourceOnDisc(PrCore::IResourceDataPtr p_resourceData, const std::string& p_path)
{
	PR_ASSERT(p_resourceData, "Pointer invalid!");

	auto meshPtr = std::static_pointer_cast<Mesh>(p_resourceData);

	// convert vertices
	auto vertices = meshPtr->GetVertices();
	auto vertSize = meshPtr->GetVerticesCount();
	aiVector3D* aiVerts = new aiVector3D[vertSize];
	for (size_t i = 0; i < vertSize; i++)
	{
		aiVerts[i] = { vertices[i].x, vertices[i].y, vertices[i].z };
	}

	// convert normals
	auto normals = meshPtr->GetNormals();
	aiVector3D* aiNormals = new aiVector3D[vertSize];
	for (size_t i = 0; i < vertSize; i++)
	{
		aiNormals[i] = { normals[i].x, normals[i].y, normals[i].z };
	}

	// convert UVs
	auto uvs = meshPtr->GetUVs();
	aiVector3D* aiUvs = new aiVector3D[vertSize];
	for (size_t i = 0; i < vertSize; i++)
	{
		aiUvs[i] = { uvs[i].x, uvs[i].y, 0.0f };
	}

	auto indicies = meshPtr->GetIndices();

	// convert default submesh
	size_t submeshCount = meshPtr->GetSubmeshCount();
	auto subMeshes = meshPtr->GetSubmeshes();
	aiMesh** aiMeshes = new aiMesh * [submeshCount];
	{
		auto aiDefaultMesh = new aiMesh();

		aiDefaultMesh->mNumVertices = vertSize;
		aiDefaultMesh->mVertices = aiVerts;

		aiDefaultMesh->mNormals = aiNormals;

		aiDefaultMesh->mNumUVComponents[0] = 2;
		aiDefaultMesh->mTextureCoords[0] = aiUvs;

		// convert faces
		aiDefaultMesh->mNumFaces = subMeshes[0].indicesCount / 3;
		aiFace* aiFaces = new aiFace[aiDefaultMesh->mNumFaces];
		for (unsigned int j = 0; j < aiDefaultMesh->mNumFaces; j++)
		{
			aiFace face;
			face.mNumIndices = 3;
			face.mIndices = new unsigned int[3];
			face.mIndices[0] = indicies[j * 3];
			face.mIndices[1] = indicies[j * 3 + 1];
			face.mIndices[2] = indicies[j * 3 + 2];
			aiFaces[j] = face;
		}

		aiDefaultMesh->mFaces = aiFaces;
		aiDefaultMesh->mName = std::string{ "submesh_0" };
		aiDefaultMesh->mMaterialIndex = 0;
		aiMeshes[0] = aiDefaultMesh;
	}

	// convert the rest of submeshes
	// copy vertices from the default submesh
	for (int i = 1; i < submeshCount; i++)
	{
		auto aiMeshPtr = new aiMesh();

		// Assign copies, assimp take the memory ownership so cannot reuse memory :/
		aiMeshPtr->mNumVertices = vertSize;

		size_t copySize = vertSize * sizeof(aiVector3D);
		aiVector3D* aiVertsCopy = new aiVector3D[vertSize];
		memcpy(aiVertsCopy, aiVerts, copySize);
		aiMeshPtr->mVertices = aiVertsCopy;

		aiVector3D* aiNormalsCopy = new aiVector3D[vertSize];
		memcpy(aiNormalsCopy, aiNormals, copySize);
		aiMeshPtr->mNormals = aiNormalsCopy;

		aiVector3D* aiUvsCopy = new aiVector3D[vertSize];
		memcpy(aiUvsCopy, aiUvs, copySize);
		aiMeshPtr->mNumUVComponents[0] = 2;
		aiMeshPtr->mTextureCoords[0] = aiUvsCopy;

		//convert faces
		const auto firxtIndex = subMeshes[i].firstIndex;
		aiMeshPtr->mNumFaces = subMeshes[i].indicesCount / 3;
		aiFace* aiFaces = new aiFace[aiMeshPtr->mNumFaces];
		for (unsigned int j = 0; j < aiMeshPtr->mNumFaces; j++)
		{
			aiFace face;
			face.mNumIndices = 3;
			face.mIndices = new unsigned int[3];
			face.mIndices[0] = indicies[j * 3 + firxtIndex];
			face.mIndices[1] = indicies[j * 3 + 1 + firxtIndex];
			face.mIndices[2] = indicies[j * 3 + 2 + firxtIndex];
			aiFaces[j] = face;
		}

		aiMeshPtr->mFaces = aiFaces;
		aiMeshPtr->mName = std::string{ "submesh_" + PrCore::StringUtils::ToString(i) };
		aiMeshPtr->mMaterialIndex = i;
		aiMeshes[i] = aiMeshPtr;
	}

	// Create a dummy scene to export
	aiScene aiScene;
	aiScene.mNumMeshes = submeshCount;
	aiScene.mMeshes = aiMeshes;
	aiScene.mNumMaterials = submeshCount;
	aiScene.mMaterials = new aiMaterial * [submeshCount];

	aiScene.mRootNode = new aiNode();
	aiScene.mRootNode->mNumMeshes = submeshCount;
	aiScene.mRootNode->mMeshes = new unsigned int[submeshCount];
	for (int i = 0; i < submeshCount; i++)
	{
		auto* aiMat = new aiMaterial();
		aiString matName{ "mat_submesh_" + PrCore::StringUtils::ToString(i) };
		aiMat->AddProperty(&matName, AI_MATKEY_NAME);

		aiScene.mMaterials[i] = aiMat;
		aiScene.mRootNode->mMeshes[i] = i;
	}

	Assimp::Exporter exporter;
	auto blob = exporter.ExportToBlob(&aiScene, "obj");
	if (blob == nullptr)
		return false;

	auto pFileSystem = PrSystems::Get<PrCore::FileSystem>();
	auto file = pFileSystem->FileOpen(p_path, PrCore::FileOpenMode::Write);
	if (file == nullptr)
		return false;

	pFileSystem->FileWrite(file, blob->data, blob->size);
	pFileSystem->FileClose(file);

	return true;
}
