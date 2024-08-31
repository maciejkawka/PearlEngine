#pragma once
#include"Core/Resources/Resource.h"

#include"Renderer/Core/Defines.h"
#include"Renderer/Core/Color.h"
#include"Renderer/Buffers/VertexArray.h"
#include"Renderer/Core/BoundingVolume.h"

#include<array>

#define MAX_UVs 8

namespace PrRenderer::Resources {

	class Mesh;
	typedef std::shared_ptr<Mesh> MeshPtr;

	enum PrimitiveType
	{
		Cube,
		Sphere,
		Capsule,
		Cylinder,
		Plane,
		Quad,
		Line
	};

	class Mesh : public PrCore::Resources::Resource {

		typedef std::array<std::vector<PrCore::Math::vec2>, 8> UVArray;

	public:
		Mesh() :
			Resource("Mesh"),
			m_indicesCount(0),
			m_verticesCount(0),
			m_stateChanged(false)
		{}

		//Constructor for managed resource
		Mesh(const std::string& p_name, PrCore::Resources::ResourceHandle p_handle) :
			Resource(p_name, p_handle),
			m_indicesCount(0),
			m_verticesCount(0),
			m_stateChanged(false)
		{}
		

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		inline std::shared_ptr<Buffers::VertexArray> GetVertexArray() { return m_VA; }

		inline const std::vector<PrCore::Math::vec3>& GetVertices() { return m_vertices; }
		inline size_t GetVerticesCount() const { return m_verticesCount; }

		inline std::vector<unsigned int> GetIndices() { return m_indices; }
		inline size_t GetIndicesCount() const { return m_indicesCount; }

		inline std::vector<Core::Color> GetColors() const { return m_colors; }
		inline std::vector<PrCore::Math::vec3> GetNormals() const { return m_normals; }
		inline std::vector<PrCore::Math::vec4> GetTangents() const { return m_tangents; }

		inline const Core::BoxVolume& GetBoxVolume() { return m_boxVolume; }

		void SetVertices(const std::vector<PrCore::Math::vec3>& p_vertices);
		void SetIndices(const std::vector<unsigned int>& p_indices);
		void SetColors(const std::vector<Core::Color>& p_colors);
		void SetNormals(const std::vector<PrCore::Math::vec3>& p_normals);
		void SetTangents(const std::vector<PrCore::Math::vec4>& p_tangents);

		void SetUVs(unsigned int p_UVSet, const std::vector<PrCore::Math::vec2>& p_UVs);

		virtual void RecalculateNormals() = 0;
		virtual void RecalculateTangents() = 0;

		static MeshPtr Create();

		// Returns a shared primitive, do not edit the vertices
		// Copy the mesh first in order edit
		static MeshPtr CreatePrimitive(PrimitiveType p_primitiveType);

	protected:
		virtual void UpdateBuffers() = 0;

		std::vector<PrCore::Math::vec4> CalculateTangents();
		std::vector<PrCore::Math::vec3> CalculateNormals();
		PrCore::Math::vec4 GenerateTangent(int a, int b, int c);
		bool ValidateBuffers();

		std::vector<unsigned int>				m_indices;
		size_t									m_indicesCount;

		std::vector<PrCore::Math::vec3>			m_vertices;
		size_t									m_verticesCount;
		std::vector<Core::Color>	m_colors;
		std::vector<PrCore::Math::vec3>			m_normals;
		std::vector<PrCore::Math::vec4>			m_tangents;

		UVArray									m_UVs;
		size_t									m_maxUVs = MAX_UVs;

		bool									m_stateChanged;

		std::shared_ptr<Buffers::VertexArray>   m_VA;

		Core::BoxVolume                         m_boxVolume;

	private:
		static MeshPtr CreateCube();
		static MeshPtr CreateSphere();
		static MeshPtr CreateCapsule();
		static MeshPtr CreateCylinder();
		static MeshPtr CreatePlane();
		static MeshPtr CreateQuad();
		static MeshPtr CreateLine();
	};
}