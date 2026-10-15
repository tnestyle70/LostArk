// 트리 표시와 drag source/target·payload 검사를 사용한다.
#include "imgui.h"

#include "CompositionResourceTree.h"

// 분류 검색·정렬과 drop 영역 최소 크기를 계산한다.
#include <algorithm>
// 추가한 vector의 마지막 자식 iterator를 구한다.
#include <iterator>

namespace
{
    constexpr const char* TransferPayload = "COMPOSITION_RESOURCE_V1";
    // UI 스레드의 현재 drag snapshot을 공유 보유한다. drop 뒤에도 다음 drag 교체 또는 프로세스 종료까지 남는다.
    Client::COMPOSITION_TRANSFER DragTransfer;
    std::uint64_t DragToken = 0;
}

// 새 resource drag에서만 캡처하고 token을 발행한다. callback이 빈 참조를 주면 payload를 만들지 않는다.
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

// payload 형식·전달 완료·크기·현재 token을 모두 확인한 뒤 보관된 snapshot 참조를 복사한다.
Client::COMPOSITION_TRANSFER Client::Accept_CompositionResourceDropInWindow()
{
    const ImGuiPayload* active = ImGui::GetDragDropPayload();
    if (!active || !active->IsDataType(TransferPayload)) return {};
    // resource drag가 있을 때만 창 내용 영역 크기의 비대화형 ImGui 항목을 만든다.
    // drop 검사 뒤 원래 cursor 위치로 돌려놓는다.
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
		// 검색으로 leaf 개수가 바뀌어도 분류 경로를 ImGui ID로 유지한다.
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
