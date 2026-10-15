#pragma once

// bool_t와 namespace 매크로 등 상태 API의 엔진 공통 타입을 사용한다.
#include "Engine_Defines.h"
// 직업·월드·entity ID와 capture/restore 응답, 기본 음성 번호의 공유 계약을 사용한다.
#include "Network/PacketMessages.h"

// 입장 identity의 닉네임과 조회할 캐릭터 ID·외형 JSON을 소유 문자열로 전달한다.
#include <string>
// 생성·재선택 입력의 닉네임·외형·ID를 호출 중 차용하고 내부 상태에는 복사한다.
#include <string_view>

NS_BEGIN(Client)

// 입장 요청이 테스트용, 확정된 캐릭터, 아직 Bern에 확정하지 않은 선택 중 어디서 왔는지 구분한다.
enum class CHARACTER_ENTRY_IDENTITY_SOURCE
{
	AUDITION,
	CREATED,
	PENDING_CREATION,
	END
};

// Lobby가 Server 입장 명령에 전달할 class/nickname/voice의 값 묶음이다. 네트워크 연결이나 roster를 소유하지 않는다.
struct CHARACTER_ENTRY_IDENTITY final
{
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string strNickname;
	// 입장에 쓰는 외형 문서에서 읽은 음성 종류다. 외형이 없으면 기본 종류를 사용한다.
	std::uint8_t iVoiceType = LostArk::Shared::MIN_VOICE_TYPE;
	CHARACTER_ENTRY_IDENTITY_SOURCE eSource =
		CHARACTER_ENTRY_IDENTITY_SOURCE::END;
};

struct CHARACTER_WORLD_STATE;

// 명시적 캐릭터 선택 복귀에 사용하는 Server capture 요청 상태다. WAITING 이후 응답과 MainApp 종료 처리가 분리된다.
enum class CHARACTER_CAPTURE_STATUS
{
	NONE,
	WAITING,
	CAPTURED,
	FAILED
};

// process 전역의 선택/입장 대기/활성 캐릭터와 capture/restore 상태를 관리한다. 파일 저장은 하지 않는다.
// 내부 선택 변수는 mutex로 보호하지만 CCharacterRoster의 모든 외부 접근을 동기화하는 클래스는 아니다.
class CCharacterSelectionState final
{
public:
	// 지원 class만 선택한다. 이 호출만으로 캐릭터를 만들거나 현재 Server class를 변경하지 않는다.
	static bool_t Select(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	static bool_t Has_Selection();
	// 선택이 없으면 false이며 출력 class는 그대로 둔다.
	static bool_t Try_Get_SelectedClass(
		LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass);
	// 새 캐릭터의 카드 슬롯을 먼저 선택하고 생성 정보를 임시 보관한다. 실제 Bern 입장 뒤 roster에 추가한다.
	// 외형은 CCustomizingView::Serialize_Appearance의 문서 문자열이며 없으면 빈 값이다.
	static bool_t Select_CreationSlot(size_t slot);
	static bool_t Stage_Creation(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		std::string_view nickname, std::string_view appearanceJson);
	// 기존 roster의 ID/class/nickname/외형이 모두 같은지 확인해 입장 대기로 보관한다. 새 슬롯을 만들지는 않는다.
	static bool_t Stage_ExistingEntry(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		std::string_view nickname, std::string_view appearanceJson, std::string_view characterId);
	// 입장 대기 중이면 pending 외형을, 그 밖에는 마지막 확정 외형을 값으로 돌려준다. Bern 확정 전 spawn에도 사용한다.
	static std::string Get_ActiveAppearanceJson();
	// MainApp/ClientReplication이 마지막 HUD 수신값을 process roster에 보관할 때 호출한다.
	// 복원 대기, 최종 capture 처리 중/완료, 유효 inventory 미수신이면 이전 슬롯을 유지한다.
	static void Capture_ActiveWorldState();
	static bool_t Has_ActiveCharacter();
	// 확정된 캐릭터의 process-session ID를 복사한다. roster 카드 재선택에 쓰며 Server entity ID와 다르다.
	static std::string Get_ActiveCharacterId();
	static bool_t Is_RestorePending();
	// Bern 복원 대기 또는 capture 상태가 NONE이 아닌 동안 gameplay 입력을 보류하는 데 사용한다.
	static bool_t Is_WorldStateSyncPending();
	// process 안의 capture/restore 요청 번호를 만들며 uint32 wrap에서 예약값 0을 건너뛴다.
	static std::uint32_t Next_StateRequestSequence();
	// MainApp이 전송 직전에 요청 식별자와 현재 world/player/entity/generation을 기록한다. false면 기존 capture 상태를 유지한다.
	static bool_t Begin_WorldStateCapture(std::uint32_t sequence,
		LostArk::Shared::WORLD_ID world, LostArk::Shared::PLAYER_ID player,
		LostArk::Shared::NET_ENTITY_ID entity, std::uint64_t generation);
	// ClientReplication이 수신 응답을 전달한다. true는 해당 요청의 응답을 처리했다는 뜻이며 CAPTURED/FAILED는 별도 조회한다.
	static bool_t Apply_CaptureResult(const LostArk::Shared::S2C_CAPTURE_CHARACTER_RESULT& result,
		std::uint64_t generation);
	static CHARACTER_CAPTURE_STATUS Get_WorldStateCaptureStatus();
	// MainApp의 성공/거절/시간 초과 마무리다. 오래된 HUD 값으로 슬롯이 덮이지 않게 보호 표시는 남긴다.
	static void Finish_WorldStateCapture();
	// Bern 복원이 필요하고 아직 요청하지 않았을 때만 저장 상태를 복사한다. false이면 outState를 보존한다.
	static bool_t Try_Get_ActiveWorldState(CHARACTER_WORLD_STATE& outState);
	// Level_Bern이 전송에 성공한 요청 번호를 기록한다. 이 함수가 Server 승인을 대신하지 않는다.
	static void Mark_RestoreRequested(std::uint32_t sequence);
	// false는 현재 대기 요청과 무관한 응답이다. true라도 거절일 수 있으며 그때는 복원 잠금과 저장 상태를 유지한다.
	static bool_t Apply_RestoreResult(const LostArk::Shared::S2C_RESTORE_CHARACTER_RESULT& result);
	static bool_t Has_PendingCreation();
	// MainApp이 Bern profile 활성화와 Change_Level 성공 뒤 호출한다. 새 캐릭터만 roster에 추가하고 재선택은 저장 상태 복원을 준비한다.
	static bool_t Commit_PendingCreation();
	// 입장/생성 취소 시 pending만 버린다. 확정 identity와 roster, 선택해 둔 생성 슬롯은 보존한다.
	static void Cancel_PendingCreation();
	// World별 입장 identity를 임시 값으로 만든다. 지원되지 않는 World/identity는 false이며 이전 출력을 유지한다.
	static bool_t Try_Resolve_ForWorld(
		LostArk::Shared::WORLD_ID worldId,
		CHARACTER_ENTRY_IDENTITY& outIdentity);
};

NS_END
