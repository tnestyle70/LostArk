#pragma once

// Client 이름 공간과 공통 선언을 사용한다.
#include "Client_Defines.h"
// 편집 UI에서 공유하는 엔진 기본 선언을 유지한다.
#include "Engine_Defines.h"
// 불변 transfer snapshot의 shared_ptr 타입을 사용한다.
#include "CompositionEditing.h"

// catalog의 leaf index와 재귀 개수에 size_t를 사용한다.
#include <cstddef>
// leaf 그리기와 drag snapshot 캡처를 호출자의 callback으로 받는다.
#include <functional>
// 분류 표시명과 ImGui 식별 경로를 값으로 보관한다.
#include <string>
// 자식 노드와 leaf index 목록을 노드가 소유한다.
#include <vector>

NS_BEGIN(Client)

// 호출자가 읽은 catalog snapshot의 분류 트리다. leaf는 해당 snapshot의 index이며 파일 I/O는 하지 않는다.
// 각 Workbench가 자기 catalog에 대해 트리를 만들고 같은 그리기 함수를 사용한다.
struct COMPOSITION_RESOURCE_TREE_NODE final
{
	std::string strSegment;
	std::string strStablePath;
	std::vector<std::size_t> LeafIndices;
	std::vector<COMPOSITION_RESOURCE_TREE_NODE> Children;
	std::size_t iRecursiveLeafCount = 0u;
};

// 분류 경로의 빈 segment를 건너뛰고 필요한 노드를 만든 뒤 leaf index를 추가한다.
void InsertResourceTree(
	COMPOSITION_RESOURCE_TREE_NODE& Root,
	const std::vector<std::string>& CategorySegments,
	std::size_t iLeafIndex);

// 자식 이름을 정렬하고 하위 leaf 수를 계산해 저장한다. leaf 자체 순서는 유지한다.
std::size_t FinalizeResourceTree(COMPOSITION_RESOURCE_TREE_NODE& Node);

// 열린 분류를 재귀 표시하고 leaf마다 callback을 호출한다. callback은 호출 동안만 차용한다.
void RenderResourceTree(
	const COMPOSITION_RESOURCE_TREE_NODE& Node,
	const std::function<void(std::size_t)>& RenderLeaf,
	const char* emptyMessage = nullptr);

// resource 항목 직후 호출한다. drag 시작 때 불변 snapshot을 한 번 받아 보관한다.
// ImGui에는 원본 객체 주소 대신 token만 전달한다.
void Offer_CompositionResourceDrag(const char* label,
    const std::function<COMPOSITION_TRANSFER()>& capture);

// 현재 창에서 전달 완료된 token이 일치할 때 불변 snapshot의 공유 참조를 반환한다. 아니면 빈 참조다.
COMPOSITION_TRANSFER Accept_CompositionResourceDropInWindow();

NS_END
