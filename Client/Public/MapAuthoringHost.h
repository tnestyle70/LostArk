#pragma once

// Client 이름 공간과 Level ID·기본 타입을 사용한다.
#include "Client_Defines.h"
// Level이 소유한 asset catalog를 const 참조로 노출한다.
#include "MapAssetCatalog.h"
// 자체 이동 재기준 입력인 MAP_PLACEMENT_RECORD를 선언한다.
#include "MapPlacementDocument.h"
// 배치 runtime과 배치·static batch 항목 타입을 사용한다.
#include "MapPlacementRuntime.h"
// 저작용 sequence의 TARGET_SET 반환 타입을 사용한다.
#include "WorldSequencePlayer.h"

// 구조 변경 거부 이유를 출력한다.
#include <string>
// Level의 배치·batch 목록과 입력 records를 참조한다.
#include <vector>

NS_BEGIN(Client)

class CDeployPropRuntime;

// 현재 제품 Level의 맵을 저작 도구에 노출하는 인터페이스다.
// Level이 소유한 placement·batch·Deploy를 도구가 차용하며 별도 Area를 복제하지 않는다.
class IMapAuthoringHost
{
public:
	// 파생 Level의 다형 소멸을 지원한다. 도구에 Level 소유권을 넘기는 계약은 아니다.
	virtual ~IMapAuthoringHost() = default;

	virtual uint32_t Get_MapAuthoringLevelIndex() const = 0;
	// 도구 상태 표시용 짧은 이름이다. 반환 문자열은 host가 제공하는 차용 값이다.
	virtual const char_t* Get_MapAuthoringLabel() const = 0;
	// 아래 runtime/catalog/목록 참조는 host가 살아 있고 해당 저장소가 유지되는 동안만 사용한다.
	virtual CMapPlacementRuntime& Get_MapAuthoringRuntime() = 0;
	virtual const CMapAssetCatalog& Get_MapAuthoringCatalog() const = 0;
	virtual std::vector<MAP_RUNTIME_PLACED_ENTRY>& Get_MapAuthoringPlacements() = 0;
	virtual std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY>& Get_MapAuthoringBatches() = 0;
	// Deploy가 없는 Level은 nullptr다. 도구는 해당 패널을 생략한다.
	virtual CDeployPropRuntime* Get_MapAuthoringDeployRuntime() = 0;
	// 배치 구조를 변경할 수 있는지 Level의 현재 사용 상태를 검사하고 거부 이유를 반환한다.
	virtual bool_t Can_ChangeMapAuthoringStructure(std::string& outReason) const = 0;
	// 갱신한 배치 기록에 맞춰 Level의 자체 이동 기준을 다시 설정한다.
	virtual void Rebase_MapAuthoringSelfMotions(
		const std::vector<MAP_PLACEMENT_RECORD>& records) = 0;
	// 저작 부착 상태를 Level에 알린다. 실제 처리 방식은 파생 Level이 구현한다.
	virtual void Set_MapAuthoringActive(bool_t active) = 0;
	// 공용 WorldSequence 도구가 사용할 현재 대상 참조를 만든다. Deploy 포인터는 없을 수 있다.
	virtual CWorldSequencePlayer::TARGET_SET Make_MapAuthoringTargets() = 0;
};

// Debug의 현재 Level이 구현한 host를 차용한다. 미지원 Level과 Release는 nullptr다.
IMapAuthoringHost* Find_ActiveMapAuthoringHost();

NS_END
