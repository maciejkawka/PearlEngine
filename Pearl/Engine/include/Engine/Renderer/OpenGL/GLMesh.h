#pragma once
#include"Renderer/Resources/Mesh.h"

namespace PrRenderer::OpenGL {

	class GLMesh : public Resources::Mesh {
	public:
		GLMesh(const std::string& p_name, PrCore::Resources::ResourceID p_ID) :
			Mesh(p_name, p_ID)
		{}

		GLMesh() :
			Mesh()
		{}

		void Bind() override;
		void Unbind() override;

	private:
		void PreLoadImpl() override;
		bool LoadImpl() override;
		void PostLoadImpl() override;

		void PreUnloadImpl() override;
		bool UnloadImpl() override;
		void PostUnloadImpl() override;

		virtual void CalculateSize() override;

		bool ValidateBuffers();
		void UpdateBuffers();
		bool PopulateOBJ();

	};
}