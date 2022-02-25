#include"Core/Common/pearl_pch.h"
#include"Renderer/Resources/Mesh.h"

using namespace PrRenderer::Resources;

void Mesh::SetVertices(std::shared_ptr<float[]> p_vertices, unsigned int p_count)
{
	m_vertices = p_vertices;
	m_verticesCount = p_count;

	m_stateChanged = true;
}

void Mesh::SetIndices(std::shared_ptr<float[]> p_indices, unsigned int p_count)
{
	m_indices = p_indices;
	m_indicesCount = p_count;

	m_stateChanged = true;
}

void Mesh::SetColors(std::shared_ptr<float[]> p_colors, unsigned int p_count)
{
	if (p_count != m_verticesCount)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong colors length", m_name);
		return;
	}

	m_colors = p_colors;
	m_stateChanged = true;
}

void Mesh::SetNormals(std::shared_ptr<float[]> p_normals, unsigned int p_count)
{
	if (p_count != m_verticesCount)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong normals length", m_name);
		return;
	}

	m_normals = p_normals;
	m_stateChanged = true;
}

void Mesh::SetTangents(std::shared_ptr<float[]> p_tangents, unsigned int p_count)
{
	if (p_count != m_verticesCount)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong tangents length", m_name);
		return;
	}

	m_tangents = p_tangents;
	m_stateChanged = true;
}

void Mesh::SetUVs(unsigned int p_UVSet, std::shared_ptr<float[]> p_UVs, unsigned int p_count)
{
	if (p_UVSet > m_maxUVs)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong UVSet", m_name);
		return;
	}

	if (p_count != m_verticesCount)
	{
		PRLOG_WARN("Renderer: Mesh {0} wrong UVs length", m_name);
		return;
	}

	m_UVs[p_UVSet] = p_UVs;
	m_stateChanged = true;
}
