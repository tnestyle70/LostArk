#include "imgui.h"

#include "CompositionResourceTree.h"

#include <algorithm>
#include <iterator>

namespace
{
    constexpr const char* TransferPayload = "COMPOSITION_RESOURCE_V1";
    Client::COMPOSITION_TRANSFER DragTransfer;
    std::uint64_t DragToken = 0;
}

void Client::Offer_CompositionResourceDrag(const char* label,
    const std::function<COMPOSITION_TRANSFER()>& capture)
{
    if (!ImGui::BeginDragDropSource()) return;
    const ImGuiPayload* active = ImGui::GetDragDropPayload();
    if (!active || !active->IsDataType(TransferPayload))
    {
        DragTransfer = capture ? capture() : COMPOSITION_TRANSFER{};
        ++DragToken;
        if (!DragToken) ++DragToken;
    }
    if (DragTransfer)
    {
        ImGui::SetDragDropPayload(TransferPayload, &DragToken, sizeof(DragToken), ImGuiCond_Once);
        ImGui::TextUnformatted(label ? label : DragTransfer->label.c_str());
        ImGui::TextDisabled("Drop in the destination Sequencer or Box Detail.");
    }
    else ImGui::TextDisabled("This resource cannot be transferred.");
    ImGui::EndDragDropSource();
}

Client::COMPOSITION_TRANSFER Client::Accept_CompositionResourceDropInWindow()
{
    const ImGuiPayload* active = ImGui::GetDragDropPayload();
    if (!active || !active->IsDataType(TransferPayload)) return {};
    // A non-interactive item supplies the pane-sized target using public ImGui
    // APIs. It exists only during a resource drag and captures no mouse clicks.
    const ImVec2 cursor = ImGui::GetCursorScreenPos();
    const ImVec2 position = ImGui::GetWindowPos();
    const ImVec2 size = ImGui::GetWindowSize();
    const ImVec2 padding = ImGui::GetStyle().WindowPadding;
    ImGui::SetCursorScreenPos({position.x + padding.x, position.y + padding.y});
    ImGui::Dummy({(std::max)(1.f, size.x - 2.f * padding.x),
        (std::max)(1.f, size.y - 2.f * padding.y)});
    COMPOSITION_TRANSFER result;
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(TransferPayload))
            if (payload->IsDelivery() && payload->DataSize == sizeof(DragToken) &&
                *static_cast<const std::uint64_t*>(payload->Data) == DragToken)
                result = DragTransfer;
        ImGui::EndDragDropTarget();
    }
    ImGui::SetCursorScreenPos(cursor);
    return result;
}

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
	const std::function<void(std::size_t)>& RenderLeaf,
	const char* emptyMessage)
{
	for (const COMPOSITION_RESOURCE_TREE_NODE& Child : Node.Children)
	{
		ImGui::PushID(Child.strStablePath.c_str());
		// Search changes the visible count, not the category's expansion identity.
		if (ImGui::TreeNodeEx("##ResourceCategory", ImGuiTreeNodeFlags_SpanAvailWidth,
				"%s (%zu)", Child.strSegment.c_str(), Child.iRecursiveLeafCount))
		{
			RenderResourceTree(Child, RenderLeaf, emptyMessage);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	for (const std::size_t iLeafIndex : Node.LeafIndices)
		RenderLeaf(iLeafIndex);
	if (emptyMessage && Node.iRecursiveLeafCount == 0u)
		ImGui::TextDisabled("%s", emptyMessage);
}
