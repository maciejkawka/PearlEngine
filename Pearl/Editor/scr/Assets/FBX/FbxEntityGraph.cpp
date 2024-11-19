#include "Editor/Assets/FBX/FbxEntityGraph.h"

#include "Core/Utils/PathUtils.h"
using namespace PrEditor::Assets;

const FbxEntityNode* PrEditor::Assets::FbxEntityGraph::GetNodeRecursive(std::string_view p_nodePath, const FbxEntityNode* p_node, int depth)
{
	if (depth > s_maxDepth)
	{
		PR_ASSERT(false, "Reached maximum FBX Hierarhy depth. Consider increasing depth or shrinking FBX file hierarchy");
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

FbxEntityGraph::~FbxEntityGraph()
{
	ClearGraphRecursive(m_root, 0);
}

void FbxEntityGraph::ClearGraphRecursive(FbxEntityNode* p_node, int depth)
{
	if (depth > s_maxDepth)
	{
		PR_ASSERT(false, "Reached maximum FBX Hierarhy depth. Consider increasing depth or shrinking FBX file hierarchy");
		return;
	}

	for (auto node : p_node->children)
		ClearGraphRecursive(node, depth + 1);

	delete p_node;
}

void FbxEntityGraph::ForEachNodes(FbxNodeVisitor p_visitor)
{
	ForEachNodesRecursive(p_visitor, m_root, 0);
}

void FbxEntityGraph::ForEachNodesRecursive(FbxNodeVisitor p_visitor, const FbxEntityNode* p_node, int depth)
{
	if (depth > s_maxDepth)
	{
		PR_ASSERT(false, "Reached maximum FBX Hierarhy depth. Consider increasing depth or shrinking FBX file hierarchy");
		return;
	}

	// Traverse goes from the root the the leafs
	// Root is visited first
	p_visitor(p_node);

	for (const FbxEntityNode* node : p_node->children)
		ForEachNodesRecursive(p_visitor, node, depth + 1);
}

const PrEditor::Assets::FbxEntityNode* FbxEntityGraph::GetNode(const std::string& p_entityPath)
{
	return GetNodeRecursive(p_entityPath, m_root, 0);
}

const PrEditor::Assets::FbxEntityNode* FbxEntityGraph::GetRoot()
{
	return m_root;
}
