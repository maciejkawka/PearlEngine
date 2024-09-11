#include "Core/Common/pearl_pch.h"

#include "Renderer/OpenGL/GLMeshv2.h"

#include"Renderer/OpenGl/GLVertexArray.h"
#include"Renderer/OpenGl/GLVertexBuffer.h"
#include"Renderer/OpenGl/GLIndexBuffer.h"
#include"Renderer/Core/Defines.h"

using namespace PrRenderer::OpenGL;

void GLMeshv2::Bind()
{
	if (m_stateChanged)
		UpdateBuffers();
	m_VA->Bind();
}

void GLMeshv2::Unbind()
{
	m_VA->Unbind();
}

void GLMeshv2::RecalculateNormals()
{
	m_normals.clear();
	m_normals = CalculateNormals();
}

void GLMeshv2::RecalculateTangents()
{
	m_tangents.clear();
	m_tangents = CalculateTangents();
}

void GLMeshv2::UpdateBuffers()
{
	auto vertexBuffer = Buffers::VertexBuffer::Create();
	auto indexBuffer = Buffers::IndexBuffer::Create();
	Buffers::BufferLayout bufferLayout;

	//Vertex
	bufferLayout.AddElementBuffer({ "Vertex", Buffers::ShaderDataType::Float3 });

	//Normals
	if (m_normals.empty() && m_normals.size() > 2)
		m_normals = CalculateNormals();
	bufferLayout.AddElementBuffer({ "Normals", Buffers::ShaderDataType::Float3 });

	//Tangents
	if (m_tangents.empty() && m_normals.size() > 2)
		m_tangents = CalculateTangents();
	bufferLayout.AddElementBuffer({ "Tangents", Buffers::ShaderDataType::Float4 });

	//UVs
	for (int i = 0; i < m_maxUVs; i++)
		if (!m_UVs[i].empty())
			bufferLayout.AddElementBuffer({ ("UV" + std::to_string(i)), Buffers::ShaderDataType::Float2 });

	//Colors
	if (!m_colors.empty())
		bufferLayout.AddElementBuffer({ "Color", Buffers::ShaderDataType::Float4 });

	//Create VertexBuffer
	size_t bufferSize = bufferLayout.GetFloatStride() * m_verticesCount;
	std::vector<float> bufferVector;

	for (int i = 0; i < m_verticesCount; i++)
	{
		bufferVector.push_back(m_vertices[i].x);
		bufferVector.push_back(m_vertices[i].y);
		bufferVector.push_back(m_vertices[i].z);

		if (!m_normals.empty())
		{
			bufferVector.push_back(m_normals[i].x);
			bufferVector.push_back(m_normals[i].y);
			bufferVector.push_back(m_normals[i].z);
		}

		if (!m_tangents.empty())
		{
			bufferVector.push_back(m_tangents[i].x);
			bufferVector.push_back(m_tangents[i].y);
			bufferVector.push_back(m_tangents[i].z);
			bufferVector.push_back(m_tangents[i].w);
		}

		for (int j = 0; j < m_maxUVs; j++)
		{
			if (!m_UVs[j].empty())
			{
				bufferVector.push_back(m_UVs[j][i].x);
				bufferVector.push_back(m_UVs[j][i].y);
			}
		}


		if (!m_colors.empty())
		{
			bufferVector.push_back(m_colors[i].r);
			bufferVector.push_back(m_colors[i].g);
			bufferVector.push_back(m_colors[i].b);
			bufferVector.push_back(m_colors[i].a);
		}
	}

	//Pass Vertices to GPU
	vertexBuffer->SetBufferLayout(bufferLayout);
	vertexBuffer->SetData(static_cast<void*>(bufferVector.data()), bufferVector.size());
	indexBuffer->SetIndeces(static_cast<void*>(m_indices.data()), m_indicesCount);

	m_VA.reset(new GLVertexArray());
	m_VA->SetIndexBuffer(indexBuffer);
	m_VA->SetVertexBuffer(vertexBuffer);

	m_boxVolume = Core::BoxVolume(m_vertices);

	m_stateChanged = false;
}

size_t GLMeshv2::GetByteSize() const
{
	size_t size = sizeof(GLMeshv2);

	size += sizeof(unsigned int) * m_indices.size();
	size += sizeof(PrCore::Math::vec3) * m_vertices.size();
	size += sizeof(PrCore::Math::vec3) * m_normals.size();
	size += sizeof(PrCore::Math::vec3) * m_tangents.size();
	size += sizeof(PrCore::Math::vec4) * m_colors.size();

	for (int i = 0; i < m_maxUVs; i++)
		size += sizeof(PrCore::Math::vec2) * m_UVs[i].size();

	return size;
}
