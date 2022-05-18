#pragma once
#include"Renderer/Resources/Mesh.h"

namespace PrRenderer::OpenGL {

	class GLMesh : public Resources::Mesh {
	public:
		GLMesh() = default;

		//Constructor for managed resource
		GLMesh(const std::string& p_name, PrCore::Resources::ResourceHandle p_handle) :
			Mesh(p_name, p_handle)
		{}

		void Bind() override;
		void Unbind() override;

		void RecalculateNormals() override;
		void RecalculateTangents() override;

	private:
		void PreLoadImpl() override;
		bool LoadImpl() override;
		void PostLoadImpl() override;

		void PreUnloadImpl() override;
		bool UnloadImpl() override;
		void PostUnloadImpl() override;

		void LoadDefault() override;

		void UpdateBuffers() override;
		void CalculateSize() override;
		
		bool PopulateOBJ();
	};
}