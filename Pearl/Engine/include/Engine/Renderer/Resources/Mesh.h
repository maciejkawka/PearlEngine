#pragma once

#include "Core/Resources/IResource.h"

#include "Renderer/Core/Defines.h"
#include "Renderer/Core/Color.h"
#include "Renderer/Buffers/VertexArray.h"
#include "Renderer/Core/BoundingVolume.h"

#include <array>

//Remove later
#include "Renderer/Resources/Mesh.h"

#define MAX_UVs 8

namespace PrRenderer::Resources {

	class Mesh;
	using Meshv2Ptr = std::shared_ptr<Mesh>;

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

	class Mesh : public PrCore::Resources::IResourceData {
	public:
		using UVArray = std::array<std::vector<PrCore::Math::vec2>, 8>;

		Mesh() :
			m_indicesCount(0),
			m_verticesCount(0),
			m_stateChanged(false)
		{}

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		inline const std::shared_ptr<Buffers::VertexArray>& GetVertexArray() const { return m_VA; }

		inline const std::vector<PrCore::Math::vec3>& GetVertices() const { return m_vertices; }
		inline size_t GetVerticesCount() const { return m_verticesCount; }

		inline const std::vector<unsigned int>& GetIndices() const { return m_indices; }
		inline size_t GetIndicesCount() const { return m_indicesCount; }

		inline const std::vector<Core::Color>& GetColors() const { return m_colors; }
		inline const std::vector<PrCore::Math::vec3>& GetNormals() const { return m_normals; }
		inline const std::vector<PrCore::Math::vec4>& GetTangents() const { return m_tangents; }

		inline const Core::BoxVolume& GetBoxVolume() { return m_boxVolume; }

		void SetVertices(std::vector<PrCore::Math::vec3>&& p_vertices);
		void SetIndices(std::vector<unsigned int>&& p_indices);
		void SetColors(std::vector<Core::Color>&& p_colors);
		void SetNormals(std::vector<PrCore::Math::vec3>&& p_normals);
		void SetTangents(std::vector<PrCore::Math::vec4>&& p_tangents);

		void SetUVs(unsigned int p_UVSet, std::vector<PrCore::Math::vec2>&& p_UVs);

		virtual void RecalculateNormals() = 0;
		virtual void RecalculateTangents() = 0;

		virtual void UpdateBuffers() = 0;
		bool ValidateBuffers();

		// Factories
		static Meshv2Ptr Create();

		// Returns a shared primitive, do not edit the vertices
		// Copy the mesh first in order edit
		static Meshv2Ptr CreatePrimitive(PrimitiveType p_primitiveType);

		size_t GetByteSize() const override;

	protected:
		std::vector<PrCore::Math::vec4> CalculateTangents();
		std::vector<PrCore::Math::vec3> CalculateNormals();
		PrCore::Math::vec4 GenerateTangent(int a, int b, int c);

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
		// Add later
		static Meshv2Ptr CreateCube();
		static Meshv2Ptr CreateSphere();
		static Meshv2Ptr CreateCapsule();
		static Meshv2Ptr CreateCylinder();
		static Meshv2Ptr CreatePlane();
		static Meshv2Ptr CreateQuad();
		static Meshv2Ptr CreateLine();
	};

	REGISTRER_RESOURCE_HANDLE(Mesh);
}
