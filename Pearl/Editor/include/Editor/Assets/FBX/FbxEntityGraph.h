#pragma once

#include "Core/ECS/Components/RendererComponents.h"
#include "Core/Math/Math.h"

namespace PrEditor::Assets {

	// Represents the ECS Entity in the FBX graph
	// It holds ECS basic information and then it is copied to the Scene
	struct FbxEntity {

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

	struct FbxEntityNode {

		FbxEntityNode() = default;
		~FbxEntityNode()
		{
			if(entity)
				delete entity;
		}

		std::string               nodePath;
		FbxEntityNode*            parent = nullptr;
		std::list<FbxEntityNode*> children;
		FbxEntity*                entity = nullptr;
	};
	using FbxNodeVisitor = std::function<void(const FbxEntityNode*)>;

	// Simple Graph to store FBX Entity hierarchy, provides simple DSF traversal and Node lookups
	// Used to add entities to scene, do not modify this graph is should be read only
	class FbxEntityGraph {
	public:
		FbxEntityGraph() = delete;
		FbxEntityGraph(FbxEntityNode* p_root) :
			m_root(p_root)
		{}

		~FbxEntityGraph();

		const FbxEntityNode* GetNode(const std::string& p_entityPath);
		const FbxEntityNode* GetRoot();
		void                 ForEachNodes(FbxNodeVisitor p_visitor);

		const static size_t  s_maxDepth = 20;

	private:
		void                 ClearGraphRecursive(FbxEntityNode* p_node, int depth);
		const FbxEntityNode* GetNodeRecursive(std::string_view p_nodePath, const FbxEntityNode* p_node, int depth);
		void                 ForEachNodesRecursive(FbxNodeVisitor p_visitor, const FbxEntityNode* p_node, int depth);

		FbxEntityNode*   m_root = nullptr;
	};
}