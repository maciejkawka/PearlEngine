#pragma once
#include"Core/Resources/Resource.h"

#include"Renderer/Buffers//VertexArray.h"

#include<array>

namespace PrRenderer::Resources {

	class Mesh : public PrCore::Resources::Resources {

		typedef std::array<std::shared_ptr<float[]>, 8> UVArray;

	public:
		Mesh(const std::string& p_name, PrCore::Resources::ResourceID p_ID) :
			Resources(p_name, p_ID),
			m_indicesCount(0),
			m_verticesCount(0),
			m_stateChanged(false)
		{}
		
		Mesh() :
			Resources("Mesh"),
			m_indicesCount(0),
			m_verticesCount(0),
			m_stateChanged(false)
		{}

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		inline std::shared_ptr<float[]> GetVertices() { return m_vertices; }
		inline unsigned int GetVerticesCount() { return m_verticesCount; }

		inline std::shared_ptr<float[]> GetIndices() { return m_indices; }
		inline unsigned int GetIndicesCount() { return m_indicesCount; }

		inline std::shared_ptr<float[]> GetColors() { return m_colors; }
		inline std::shared_ptr<float[]> GetNormals() { return m_normals; }
		inline std::shared_ptr<float[]> GetTangents() { return m_tangents; }

		void SetVertices(std::shared_ptr<float[]> p_vertices, unsigned int p_count);
		void SetIndices(std::shared_ptr<float[]> p_indices, unsigned int p_count);
		void SetColors(std::shared_ptr<float[]> p_colors, unsigned int p_count);
		void SetNormals(std::shared_ptr<float[]> p_normals, unsigned int p_count);
		void SetTangents(std::shared_ptr<float[]> p_tangents, unsigned int p_count);

		void SetUVs(unsigned int p_UVSet, std::shared_ptr<float[]> p_UVs, unsigned int p_count);

	private:		
		std::shared_ptr<float[]>	m_indices;
		unsigned int				m_indicesCount;

		std::shared_ptr<float[]>	m_vertices;
		unsigned int				m_verticesCount;
		std::shared_ptr<float[]>	m_colors;
		std::shared_ptr<float[]>	m_normals;
		std::shared_ptr<float[]>	m_tangents;

		UVArray m_UVs;
		const unsigned int m_maxUVs = 8;

		bool m_stateChanged;

		std::unique_ptr<Buffers::VertexArray> m_VA;
	};
}