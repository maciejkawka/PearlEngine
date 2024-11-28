#include "Editor/Assets/Model/ModelEntityGraph.h"

#include "Core/Utils/PathUtils.h"

using namespace PrEditor::Assets;

const ModelEntityNode* PrEditor::Assets::ModelEntityGraph::GetNodeRecursive(std::string_view p_nodePath, const ModelEntityNode* p_node, int depth)
{
	if (depth > s_maxDepth)
	{
		PR_ASSERT(false, "Reached maximum imported model Hierarhy depth. Consider increasing depth or shrinking model file hierarchy");
		return nullptr;
	}

	auto paths = PrCore::PathUtils::SplitPathInPlace(p_nodePath);
	if (paths.empty())
		return nullptr;

	if (PrCore::PathUtils::GetFileInPlace(p_node->nodePath) != paths[0])
		return nullptr;

	if (p_node->children.empty() && paths.size() == 1)
		return p_node;

	p_nodePath.remove_prefix(paths[0].size() + 1);
	for (auto node : p_node->children)
	{
		auto foundNode = GetNodeRecursive(p_nodePath, node, depth + 1);
		if (foundNode)
			return foundNode;
	}

	return nullptr;
}

ModelEntityGraph::~ModelEntityGraph()
{
	ClearGraphRecursive(m_root, 0);
}

void ModelEntityGraph::ClearGraphRecursive(ModelEntityNode* p_node, int depth)
{
	if (depth > s_maxDepth)
	{
		PR_ASSERT(false, "Reached maximum model hierarhy depth. Consider increasing depth or shrinking model file hierarchy");
		return;
	}

	for (auto node : p_node->children)
		ClearGraphRecursive(node, depth + 1);

	delete p_node;
}

void ModelEntityGraph::ForEachNodes(ModelNodeVisitor p_visitor)
{
	ForEachNodesRecursive(p_visitor, m_root, 0);
}

void ModelEntityGraph::ForEachNodesRecursive(ModelNodeVisitor p_visitor, const ModelEntityNode* p_node, int depth)
{
	if (depth > s_maxDepth)
	{
		PR_ASSERT(false, "Reached maximum model hierarhy depth. Consider increasing depth or shrinking model file hierarchy");
		return;
	}

	// Traverse goes from the root the the leafs
	// Root is visited first
	p_visitor(p_node);

	for (const ModelEntityNode* node : p_node->children)
		ForEachNodesRecursive(p_visitor, node, depth + 1);
}

const PrEditor::Assets::ModelEntityNode* ModelEntityGraph::GetNode(const std::string& p_entityPath)
{
	return GetNodeRecursive(p_entityPath, m_root, 0);
}

const PrEditor::Assets::ModelEntityNode* ModelEntityGraph::GetRoot()
{
	return m_root;
}
