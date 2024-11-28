#pragma once

#include "Core/ECS/Components/RendererComponents.h"
#include "Core/Math/Math.h"

namespace PrEditor::Assets {

	// Represents the ECS Entity in the imported model graph
	// It holds ECS basic information and then it is copied to the Scene
	struct ModelEntity {

		using MaterialVector = std::vector<PrRenderer::Resources::MaterialHandle>;
		// General components
		std::string                           name;
		PrCore::Math::vec3                    position;
		PrCore::Math::quat                    rotation;
		PrCore::Math::vec3                    scale;

		// Renderer components
		PrRenderer::Resources::MeshHandle     mesh;
		PrRenderer::Resources::MeshHandle     shadowMesh;
		MaterialVector                        materials;
		PrRenderer::Resources::LightPtr       light;

		// Move info in the future
	};

	struct ModelEntityNode {

		ModelEntityNode() = default;
		~ModelEntityNode()
		{
			if(entity)
				delete entity;
		}

		std::string                 nodePath;
		ModelEntityNode*            parent = nullptr;
		std::list<ModelEntityNode*> children;
		ModelEntity*                entity = nullptr;
	};
	using ModelNodeVisitor = std::function<void(const ModelEntityNode*)>;

	// Simple Graph to store imported model Entity hierarchy, provides simple DSF traversal and Node lookups
	// Used to add entities to scene, do not modify this graph is should be read only
	class ModelEntityGraph {
	public:
		ModelEntityGraph() = delete;
		ModelEntityGraph(ModelEntityNode* p_root) :
			m_root(p_root)
		{}

		~ModelEntityGraph();

		const ModelEntityNode* GetNode(const std::string& p_entityPath);
		const ModelEntityNode* GetRoot();
		void                   ForEachNodes(ModelNodeVisitor p_visitor);

		const static size_t  s_maxDepth = 20;

	private:
		void                   ClearGraphRecursive(ModelEntityNode* p_node, int depth);
		const ModelEntityNode* GetNodeRecursive(std::string_view p_nodePath, const ModelEntityNode* p_node, int depth);
		void                   ForEachNodesRecursive(ModelNodeVisitor p_visitor, const ModelEntityNode* p_node, int depth);

		ModelEntityNode*   m_root = nullptr;
	};
}