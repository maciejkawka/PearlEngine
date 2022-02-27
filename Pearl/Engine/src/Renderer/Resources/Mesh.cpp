#include"Core/Common/pearl_pch.h"
#include"Renderer/Resources/Mesh.h"

using namespace PrRenderer::Resources;

void Mesh::SetVertices(const std::vector<PrCore::Math::vec3>& p_vertices)
{
	m_vertices = p_vertices;
	m_verticesCount = p_vertices.size();

	m_stateChanged = true;
}

void Mesh::SetIndices(const std::vector<unsigned int>& p_indices)
{
	m_indices = p_indices;
	m_indicesCount = p_indices.size();

	m_stateChanged = true;
}

void Mesh::SetColors(const std::vector<PrRenderer::Core::Color>& p_colors)
{
	if (p_colors.size() != m_verticesCount)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong colors length", m_name);
		return;
	}

	m_colors = p_colors;
	m_stateChanged = true;
}

void Mesh::SetNormals(const std::vector<PrCore::Math::vec3>& p_normals)
{
	if (p_normals.size() != m_verticesCount)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong normals length", m_name);
		return;
	}

	m_normals = p_normals;
	m_stateChanged = true;
}

void Mesh::SetTangents(const std::vector<PrCore::Math::vec3>& p_tangents)
{
	if (p_tangents.size() != m_verticesCount)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong tangents length", m_name);
		return;
	}

	m_tangents = p_tangents;
	m_stateChanged = true;
}

void Mesh::SetUVs(unsigned int p_UVSet, const std::vector<PrCore::Math::vec2>& p_UVs)
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

	m_UVs[p_UVSet] = p_UVs;
	m_stateChanged = true;
}
