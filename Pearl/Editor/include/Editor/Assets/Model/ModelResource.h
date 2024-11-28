#pragma once

#include "ModelEntityGraph.h"
#include "Core/Resources/IResource.h"
#include "Core/ECS/SceneManager.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"
#include "Renderer/Resources/Texture2D.h"

namespace PrEditor::Assets {

	class ModelResource : public PrCore::Resources::IResourceData {
	public:
		ModelResource(std::unique_ptr<ModelEntityGraph>&& p_entityGraph, std::vector<PrRenderer::Resources::MaterialHandle>&& p_materials,
			std::vector<PrRenderer::Resources::MeshHandle>&& p_meshes);
		~ModelResource() override = default;

		const std::vector<PrRenderer::Resources::MaterialHandle>  GetMaterialList() const { return m_materials; }
		const std::vector<PrRenderer::Resources::MeshHandle>      GetMeshList() const { return m_meshes; }

		const std::unique_ptr<ModelEntityGraph>&                  GetEntityGraph() const { return m_entityGraph; }
		void                                                      AddEntitesToScene(PrCore::ECS::Scene* p_scene);

		size_t                                                    GetByteSize() const override;

	private:
		std::unique_ptr<ModelEntityGraph> m_entityGraph;

		std::vector<PrRenderer::Resources::MaterialHandle> m_materials;
		std::vector<PrRenderer::Resources::MeshHandle>     m_meshes;
	};

	REGISTRER_RESOURCE_HANDLE(ModelResource);
	typedef std::shared_ptr<ModelResource> ModelResourcePtr;
}