#pragma once

#include "FbxEntityGraph.h"
#include "Core/Resources/IResource.h"
#include "Core/ECS/SceneManager.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"
#include "Renderer/Resources/Texture2D.h"

namespace PrEditor::Assets {

	class FbxResource : public PrCore::Resources::IResourceData {
	public:
		FbxResource(std::unique_ptr<FbxEntityGraph>&& p_entityGraph, std::vector<PrRenderer::Resources::MaterialHandle>&& p_materials,
			std::vector<PrRenderer::Resources::MeshHandle>&& p_meshes);
		~FbxResource() override = default;

		const std::vector<PrRenderer::Resources::MaterialHandle> GetMaterialList() const { return m_materials; }
		const std::vector<PrRenderer::Resources::MeshHandle>     GetMeshList() const { return m_meshes; }

		const std::unique_ptr<FbxEntityGraph>&                   GetEntityGraph() const { return m_entityGraph; }
		void                                                     AddEntitesToScene(PrCore::ECS::Scene* p_scene);

		size_t                                                   GetByteSize() const override;

	private:
		std::unique_ptr<FbxEntityGraph> m_entityGraph;

		std::vector<PrRenderer::Resources::MaterialHandle> m_materials;
		std::vector<PrRenderer::Resources::MeshHandle>     m_meshes;
	};

	REGISTRER_RESOURCE_HANDLE(FbxResource);
	typedef std::shared_ptr<FbxResource> FbxResourcePtr;
}