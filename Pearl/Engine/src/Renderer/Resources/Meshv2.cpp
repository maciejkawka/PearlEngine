#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/Meshv2.h"

#include "Renderer/Buffers/IndexBuffer.h"
#include "Renderer/Core/RendererAPI.h"
#include "Renderer/OpenGL/GLMeshv2.h"

#include"Renderer/Buffers/VertexArray.h"
#include "Renderer/Buffers/VertexBuffer.h"

using namespace PrRenderer::Resources;

void Meshv2::SetVertices(std::vector<PrCore::Math::vec3>&& p_vertices)
{
	m_vertices = std::move(p_vertices);
	m_verticesCount = p_vertices.size();

	m_stateChanged = true;
}

void Meshv2::SetIndices(std::vector<unsigned int>&& p_indices)
{
	m_indices = std::move(p_indices);
	m_indicesCount = p_indices.size();

	m_stateChanged = true;
}

void Meshv2::SetColors(std::vector<Core::Color>&& p_colors)
{
	if (p_colors.size() != m_verticesCount)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong colors length", m_name);
		return;
	}

	m_colors = std::move(p_colors);
	m_stateChanged = true;
}

void Meshv2::SetNormals(std::vector<PrCore::Math::vec3>&& p_normals)
{
	if (p_normals.size() != m_verticesCount)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong normals length", m_name);
		return;
	}

	m_normals = std::move(p_normals);
	m_stateChanged = true;
}

void Meshv2::SetTangents(std::vector<PrCore::Math::vec4>&& p_tangents)
{
	if (p_tangents.size() != m_verticesCount)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong tangents length", m_name);
		return;
	}

	m_tangents = std::move(p_tangents);
	m_stateChanged = true;
}

void Meshv2::SetUVs(unsigned int p_UVSet, std::vector<PrCore::Math::vec2>&& p_UVs)
{
	if (p_UVSet > m_maxUVs)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong UVSet", m_name);
		return;
	}

	if (p_UVs.size() != m_verticesCount)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong UVs length", m_name);
		return;
	}

	m_UVs[p_UVSet] = std::move(p_UVs);
	m_stateChanged = true;
}

Meshv2Ptr Meshv2::Create()
{
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: return std::make_shared<OpenGL::GLMeshv2>();

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		return nullptr;
	}
	}
}

std::vector<PrCore::Math::vec4> Meshv2::CalculateTangents()
{
	std::vector<PrCore::Math::vec4> tangents(m_verticesCount);

	if (m_vertices.empty())
		return tangents;

	if (m_UVs[0].empty())
		return tangents;

	for (int i = 0; i < m_indicesCount; i += 3)
	{
		auto tangentA = PrCore::Math::normalize(GenerateTangent(i, i + 1, i + 2));
		auto tangentB = PrCore::Math::normalize(GenerateTangent(i + 1, i + 2, i));
		auto tangentC = PrCore::Math::normalize(GenerateTangent(i + 2, i, i + 1));

		tangents[m_indices[i + 0]] = tangentA;
		tangents[m_indices[i + 1]] = tangentB;
		tangents[m_indices[i + 2]] = tangentC;
	}

	return tangents;
}

std::vector<PrCore::Math::vec3> Meshv2::CalculateNormals()
{
	std::vector<PrCore::Math::vec3> normals(m_verticesCount);

	if (m_vertices.empty())
		return normals;

	for (int i = 0; i < m_indicesCount; i += 3)
	{
		auto A = m_vertices[m_indices[i + 0]];
		auto B = m_vertices[m_indices[i + 1]];
		auto C = m_vertices[m_indices[i + 2]];

		auto AC = C - A;
		auto AB = B - A;

		auto BA = A - B;
		auto BC = C - B;

		auto CA = A - C;
		auto CB = B - C;

		auto Anormal = PrCore::Math::normalize(PrCore::Math::cross(AB, AC));
		auto Bnormal = PrCore::Math::normalize(PrCore::Math::cross(BC, BA));
		auto Cnormal = PrCore::Math::normalize(PrCore::Math::cross(CA, CB));

		normals[m_indices[i + 0]] = Anormal;
		normals[m_indices[i + 1]] = Bnormal;
		normals[m_indices[i + 2]] = Cnormal;
	}

	return normals;
}

PrCore::Math::vec4 Meshv2::GenerateTangent(int a, int b, int c)
{
	auto A = m_vertices[m_indices[a]];
	auto B = m_vertices[m_indices[b]];
	auto C = m_vertices[m_indices[c]];

	auto Auv = m_UVs[0][m_indices[a]];
	auto Buv = m_UVs[0][m_indices[b]];
	auto Cuv = m_UVs[0][m_indices[c]];

	auto AC = C - A;
	auto AB = B - A;

	auto ACuv = Cuv - Auv;
	auto ABuv = Buv - Auv;


	PrCore::Math::mat2 texMatrix(ABuv, ACuv);
	texMatrix = PrCore::Math::inverse(texMatrix);

	PrCore::Math::vec3 tangent;
	PrCore::Math::vec3 bitangent;
	PrCore::Math::vec3 normal;

	tangent = AB * texMatrix[0].x + AC * texMatrix[0].y;
	bitangent = AB * texMatrix[1].x + AC * texMatrix[1].y;

	normal = PrCore::Math::cross(AB, AC);
	auto biCross = PrCore::Math::cross(tangent, normal);

	float handedness = 1.0f;
	if (PrCore::Math::dot(biCross, bitangent) < 0.0f)
		handedness = -1.0f;

	return PrCore::Math::vec4(tangent, handedness);
}

bool Meshv2::ValidateBuffers()
{
	if (!m_colors.empty() && m_colors.size() != m_verticesCount)
		return false;

	if (!m_normals.empty() && m_normals.size() != m_verticesCount)
		return false;

	if (!m_tangents.empty() && m_tangents.size() != m_verticesCount)
		return false;

	for (int i = 0; i < m_maxUVs; i++)
	{
		if (!m_UVs[i].empty() && m_UVs[i].size() != m_verticesCount)
			return false;
	}

	return true;
}
