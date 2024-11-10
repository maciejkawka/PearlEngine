#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/MeshOBJLoader.h"
#include "Renderer/Resources/Mesh.h"

#include "Core/Filesystem/FileSystem.h"
#include "Core/Utils/PathUtils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include"tinyObj/tiny_obj_loader.h"

using namespace PrRenderer::Resources;

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

PrCore::Resources::IResourceDataPtr MeshOBJLoader::LoadResource(const std::string& p_path)
{
	std::string resourcePath = PrCore::PathUtils::MakePath(MESH_DIR, p_path);
	PrCore::Filesystem::FileStreamPtr file = PrCore::Filesystem::FileSystem::GetInstance().OpenFileStream(resourcePath.c_str());
	if (file == nullptr)
		return nullptr;

	char* data = new char[file->GetSize()];
	file->Read(data);
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
	Mesh::UVArray UVs;

	for (const auto& shape : shapes)
	{
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
	}

	MeshPtr mesh = Mesh::Create();
	if (!indices.empty())
		mesh->SetIndices(std::move(indices));
	if (!vertices.empty())
		mesh->SetVertices(std::move(vertices));
	if(!normals.empty())
		mesh->SetNormals(std::move(normals));
	if (!colors.empty())
		mesh->SetColors(std::move(colors));
	if(!UVs[0].empty())
		mesh->SetUVs(0, std::move(UVs[0]));

	mesh->UpdateBuffers();
	if (!mesh->ValidateBuffers())
	{
		PRLOG_ERROR("Renderer: Mesh {0} invalid", mesh->GetName());
		return nullptr;
	}

	return mesh;
}

void MeshOBJLoader::UnloadResource(PrCore::Resources::IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
	p_resourceData = nullptr;
}

bool MeshOBJLoader::SaveResourceOnDisc(PrCore::Resources::IResourceDataPtr p_resourceData, const std::string& p_path)
{
	return false;
}
