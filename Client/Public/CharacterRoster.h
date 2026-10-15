#pragma once

// bool_t와 namespace 매크로 등 기존 엔진 공통 타입을 사용한다.
#include "Engine_Defines.h"
// 캐릭터 카드가 보관하는 서버 인벤토리 수신값 INVENTORY_ITEM_SNAPSHOT을 값으로 담는다.
#include "Network/PacketMessages.h"
// 카드의 직업과 생성 입력을 CHARACTER_CLASS_ID의 공유 enum으로 구분한다.
#include "Network/PacketType.h"

// 캐릭터 ID·닉네임·외형 JSON과 결과 상태 메시지를 소유한다.
#include <string>
// 고정 개수로 만든 캐릭터 슬롯 목록과 인벤토리 항목의 값 목록을 제공한다.
#include <vector>

NS_BEGIN(Client)

// EXE마다 비어 있는 6개 슬롯으로 시작한다. 이름/외형/마지막 수신 inventory/재화/칭호를 캐릭터 전환 동안 메모리에 보관한다.
// process 종료로 사라지며 기존 개인 파일을 읽거나 쓰거나 삭제하지 않는다.
// Client가 받은 상태의 값 복사다. bValid는 수신 여부 표시이며 서버 경제/아이템 규칙을 이 struct가 검증하지 않는다.
struct CHARACTER_WORLD_STATE
{
	bool_t bValid = false;
	std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT> Items;
	uint32_t iSilver = 0;
	uint32_t iGold = 0;
	uint32_t iHonorTitleId = 0;
};

// 카드 하나의 identity/외형/저장 전투 상태를 값으로 소유한다. Server 연결이나 실제 캐릭터 객체를 보관하지 않는다.
struct CHARACTER_ROSTER_ENTRY
{
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string strNickname;
	// CCustomizingView::Serialize_Appearance가 만든 외형 문서 문자열을 그대로 보관한다. 외형을 만들지 않았으면 비어 있다.
	std::string strAppearanceJson;
	CHARACTER_WORLD_STATE World;
	// process-session 내부 ID다. nickname이나 표시 순번을 캐릭터 상태 조회 key로 쓰지 않는다.
	std::string strCharacterId;
};

// 선택창과 CCharacterSelectionState가 공유한다. 내부 mutex가 없으므로 동시 읽기/쓰기는 호출자가 직렬화해야 한다.
class CCharacterRoster final
{
public:
	// 캐릭터 선택창의 고정 카드 수다.
	static constexpr size_t MAX_CHARACTERS = 6;

	// 6개 고정 슬롯의 차용 const 참조다. ID가 비어 있으면 빈 슬롯이며 정렬하지 않는다. 이후 변경 호출은 원소 내용을 바꿀 수 있다.
	static const std::vector<CHARACTER_ROSTER_ENTRY>& Get_Entries();
	// vector 크기 6과 달리 ID가 채워진 슬롯 수만 센다.
	static size_t Get_CharacterCount();
	static bool_t Is_Occupied(size_t slot);
	// 카드 점유와 nickname UTF-8 계약을 확인한 뒤 이름만 바꾼다. ID와 슬롯 순서는 그대로다.
	static bool_t Rename(size_t iIndex, const std::string& strNickname, std::string& outStatus);
	// ID가 같은 session 캐릭터의 World만 교체한다. 명시적 false는 이전 World를 유지하고 outStatus에 사유를 적는다.
	static bool_t Update_WorldState(const std::string& strCharacterId, const CHARACTER_WORLD_STATE& State, std::string& outStatus);
	// 유효한 저장 상태가 있는 ID만 출력으로 복사한다. false이면 기존 출력 값을 유지한다.
	static bool_t Try_Get_WorldState(const std::string& strCharacterId, CHARACTER_WORLD_STATE& outState);
	// MainApp의 Bern 입장 확정 경로가 호출한다. 지원 class/닉네임/빈 슬롯을 검사하고 새 local ID를 발급한다.
	// 외형 JSON은 저장 문자열로만 복사한다. 명시적 false는 슬롯과 outIndex를 유지하며 status는 갱신한다.
	static bool_t Add(size_t slot, LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass,
		const std::string& strNickname, const std::string& strAppearanceJson,
		size_t& outIndex, std::string& outStatus);
};

NS_END
