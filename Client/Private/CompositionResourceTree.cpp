#include "imgui.h"

#include "CompositionResourceTree.h"

#include <algorithm>
#include <iterator>

void Client::InsertResourceTree(
	COMPOSITION_RESOURCE_TREE_NODE& Root,
	const std::vector<std::string>& CategorySegments,
	const std::size_t iLeafIndex)
{
	COMPOSITION_RESOURCE_TREE_NODE* pNode = &Root;
	std::string strPath;
	for (const std::string& Segment : CategorySegments)
	{
		if (Segment.empty())
			continue;
		if (!strPath.empty())
			strPath += '/';
		strPath += Segment;
		auto Child = std::find_if(
			pNode->Children.begin(), pNode->Children.end(),
			[&Segment](const COMPOSITION_RESOURCE_TREE_NODE& Candidate)
			{ return Candidate.strSegment == Segment; });
		if (pNode->Children.end() == Child)
		{
			COMPOSITION_RESOURCE_TREE_NODE NewChild;
			NewChild.strSegment = Segment;
			NewChild.strStablePath = strPath;
			pNode->Children.push_back(std::move(NewChild));
			Child = std::prev(pNode->Children.end());
		}
		pNode = &*Child;
	}
	pNode->LeafIndices.push_back(iLeafIndex);
}

std::size_t Client::FinalizeResourceTree(COMPOSITION_RESOURCE_TREE_NODE& Node)
{
	std::sort(
		Node.Children.begin(), Node.Children.end(),
		[](const COMPOSITION_RESOURCE_TREE_NODE& Left,
			const COMPOSITION_RESOURCE_TREE_NODE& Right)
		{ return Left.strSegment < Right.strSegment; });
	Node.iRecursiveLeafCount = Node.LeafIndices.size();
	for (COMPOSITION_RESOURCE_TREE_NODE& Child : Node.Children)
		Node.iRecursiveLeafCount += FinalizeResourceTree(Child);
	return Node.iRecursiveLeafCount;
}

void Client::RenderResourceTree(
	const COMPOSITION_RESOURCE_TREE_NODE& Node,
	const std::function<void(std::size_t)>& RenderLeaf)
{
	for (const COMPOSITION_RESOURCE_TREE_NODE& Child : Node.Children)
	{
		ImGui::PushID(Child.strStablePath.c_str());
		// Search changes the visible count, not the category's expansion identity.
		if (ImGui::TreeNodeEx("##ResourceCategory", ImGuiTreeNodeFlags_SpanAvailWidth,
				"%s (%zu)", Child.strSegment.c_str(), Child.iRecursiveLeafCount))
		{
			RenderResourceTree(Child, RenderLeaf);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	for (const std::size_t iLeafIndex : Node.LeafIndices)
		RenderLeaf(iLeafIndex);
}
