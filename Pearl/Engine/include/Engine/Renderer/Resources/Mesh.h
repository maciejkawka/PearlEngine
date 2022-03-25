#pragma once
#include"Core/Resources/Resource.h"

#include"Renderer/Buffers//VertexArray.h"

#include<array>

namespace PrRenderer::Resources {

	class Mesh : public PrCore::Resources::Resources {

		typedef std::array<std::vector<PrCore::Math::vec2>, 8> UVArray;

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

		inline std::shared_ptr<Buffers::VertexArray> GetVertexArray() { return m_VA; }

		inline std::vector<PrCore::Math::vec3> GetVertices() { return m_vertices; }
		inline unsigned int GetVerticesCount() { return m_verticesCount; }

		inline std::vector<unsigned int> GetIndices() { return m_indices; }
		inline size_t GetIndicesCount() { return m_indicesCount; }

		inline std::vector<PrRenderer::Core::Color> GetColors() { return m_colors; }
		inline std::vector<PrCore::Math::vec3> GetNormals() { return m_normals; }
		inline std::vector<PrCore::Math::vec3> GetTangents() { return m_tangents; }

		void SetVertices(const std::vector<PrCore::Math::vec3>& p_vertices);
		void SetIndices(const std::vector<unsigned int>& p_indices);
		void SetColors(const std::vector<PrRenderer::Core::Color>& p_colors);
		void SetNormals(const std::vector<PrCore::Math::vec3>& p_normals);
		void SetTangents(const std::vector<PrCore::Math::vec3>& p_tangents);

		void SetUVs(unsigned int p_UVSet, const std::vector<PrCore::Math::vec2>& p_UVs);

	protected:	
		std::vector<unsigned int>				m_indices;
		size_t									m_indicesCount;

		std::vector<PrCore::Math::vec3>		m_vertices;
		size_t									m_verticesCount;
		std::vector<PrRenderer::Core::Color>	m_colors;
		std::vector<PrCore::Math::vec3>		m_normals;
		std::vector<PrCore::Math::vec3>		m_tangents; //NOT USED YET

		UVArray m_UVs;
		size_t m_maxUVs = 8;

		bool m_stateChanged;

		std::shared_ptr<Buffers::VertexArray> m_VA;
	};

	typedef std::shared_ptr<Mesh> MeshPtr;
}