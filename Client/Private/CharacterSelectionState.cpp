// 엔진 및 Client 공통 Windows 헤더보다 WinSock2 선언을 먼저 포함한다.
#include <WinSock2.h>
// 선택·생성 대기·Bern 입장 확정과 capture/restore 진행 상태를 구현한다.
#include "CharacterSelectionState.h"

// 빈 슬롯을 선택하고 local 캐릭터 ID로 카드와 수신 월드 상태를 조회·갱신한다.
#include "CharacterRoster.h"
// 현재 Level이 Bern인지 판정할 때 Client LEVEL enum을 사용한다.
#include "Client_Defines.h"
// 현재 Level ID를 조회해 HUD 상태 저장과 복원 대기의 적용 범위를 결정한다.
#include "GameInstance.h"
// 마지막으로 수신한 아이템·재화·칭호 값을 로스터에 보관할 때 읽는다.
#include "CombatHUDViewModel.h"
// 닉네임 검증과 capture/restore 결과의 공유 메시지·판정 값을 사용한다.
#include "Network/PacketMessages.h"
// 저장 외형 JSON에서 음성 종류를 읽어 서버 입장 identity에 넣는다.
#include "CustomizingView.h"

// GetCurrentProcessId로 이 EXE의 테스트용 audition 닉네임을 만든다.
#include <Windows.h>

// 재선택·capture 응답이 가리키는 캐릭터 ID를 로스터에서 찾는다.
#include <algorithm>
// 선택·생성·capture/restore 전역 상태를 scoped_lock으로 보호한다. 로스터 전체의 외부 접근까지 잠그지는 않는다.
#include <mutex>
// 아직 정해지지 않은 직업·생성 대기·선택 슬롯을 값 부재와 구분한다.
#include <optional>
// 닉네임·외형·local ID와 진단 문자열을 process 상태에 소유한다.
#include <string>
// 검증한 임시 생성 값과 입장 identity를 확정할 때 문자열 소유권을 이동한다.
#include <utility>

namespace
{
	// 생성과 기존 카드 재선택이 같은 입장 대기를 사용한다. 문자열을 값으로 소유하고 신규일 때만 iSlot을 사용한다.
	struct PENDING_CHARACTER_CREATION final
	{
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strNickname;
		std::string strAppearanceJson;
		std::string strCharacterId;
		size_t iSlot = Client::CCharacterRoster::MAX_CHARACTERS;
		bool_t bNewCharacter = false;
	};

	// 이 파일의 선택/입장/동기화 상태를 보호한다. HUD/Level 조회와 roster의 외부 사용 전체를 잠그는 mutex는 아니다.
	std::mutex g_SelectionMutex;
	std::optional<LostArk::Shared::CHARACTER_CLASS_ID> g_SelectedClass;
	std::optional<std::string> g_CreatedNickname;
	std::optional<PENDING_CHARACTER_CREATION> g_PendingCreation;
	std::string g_ActiveAppearanceJson;
	std::string g_ActiveCharacterId;
	std::optional<size_t> g_CreationSlot;
	// 이전 슬롯 상태를 새 Bern Server player에 복원하기 전 HUD 기본값으로 덮지 않도록 보호한다.
	bool_t g_RestoreRequired = false;
	std::uint32_t g_RestoreRequestSequence = 0u;
	std::uint32_t g_NextStateSequence = 0u;
	// 복귀 전 최종 수신을 요청 번호와 world 세대에 묶는다. 다른 연결/캐릭터의 응답이 같은 슬롯을 덮지 않게 한다.
	Client::CHARACTER_CAPTURE_STATUS g_CaptureStatus = Client::CHARACTER_CAPTURE_STATUS::NONE;
	std::uint32_t g_CaptureSequence = 0u;
	LostArk::Shared::WORLD_ID g_CaptureWorld = LostArk::Shared::WORLD_ID::BERN;
	LostArk::Shared::PLAYER_ID g_CapturePlayer = 0u;
	LostArk::Shared::NET_ENTITY_ID g_CaptureEntity = 0u;
	std::uint64_t g_CaptureGeneration = 0u;
	std::string g_CaptureCharacterId;
	bool_t g_HasFinalCapture = false;

	// 지원 class/nickname을 확인한 뒤 문자열을 임시 값으로 복사하고 마지막에 pending을 교체한다.
	// 신규 생성은 빈 슬롯을 정하되 roster에 아직 추가하지 않는다. 외형 JSON의 구문 검증은 여기서 수행하지 않는다.
	bool_t Stage_Pending(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		const std::string_view nickname, const std::string_view appearanceJson,
		const bool_t bNewCharacter, const std::string_view characterId = {})
	{
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
				characterClass) ||
			!LostArk::Shared::Is_Valid_PlayerNickname(nickname))
		{
			return false;
		}

		PENDING_CHARACTER_CREATION staged{};
		staged.eCharacterClass = characterClass;
		staged.strNickname.assign(nickname);
		staged.strAppearanceJson.assign(appearanceJson);
		staged.strCharacterId.assign(characterId);
		staged.bNewCharacter = bNewCharacter;

		std::scoped_lock lock{ g_SelectionMutex };
		if (bNewCharacter)
		{
			if (!g_CreationSlot.has_value())
				for (size_t slot = 0; slot < Client::CCharacterRoster::MAX_CHARACTERS; ++slot)
					if (!Client::CCharacterRoster::Is_Occupied(slot))
					{
						g_CreationSlot = slot;
						break;
					}
			if (!g_CreationSlot.has_value() || Client::CCharacterRoster::Is_Occupied(*g_CreationSlot))
				return false;
			staged.iSlot = *g_CreationSlot;
		}
		g_PendingCreation = std::move(staged);
		return true;
	}

	// 실행 process의 테스트용 이름을 한 번 만들어 정적 수명으로 보관한다. 영구 계정 ID나 닉네임 고유성 계약은 아니다.
	const std::string& Get_AuditionNickname()
	{
		static const std::string nickname =
			"Test-" + std::to_string(GetCurrentProcessId());
		return nickname;
	}
}

// Level/Lobby의 선택 class를 기록한다. 지원하지 않는 입력은 이전 선택을 유지한다.
bool_t Client::CCharacterSelectionState::Select(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
		characterClass))
	{
		return false;
	}

	std::scoped_lock lock{ g_SelectionMutex };
	g_SelectedClass = characterClass;
	return true;
}

// 명시적으로 선택된 class가 있는지만 잠금 안에서 확인한다.
bool_t Client::CCharacterSelectionState::Has_Selection()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_SelectedClass.has_value();
}

// 선택된 값만 복사해 lock 밖으로 내보낸다. 미선택이면 출력은 변경하지 않는다.
bool_t Client::CCharacterSelectionState::Try_Get_SelectedClass(
	LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (!g_SelectedClass.has_value())
		return false;

	outCharacterClass = *g_SelectedClass;
	return true;
}

// MainApp이 선택창의 빈 카드 번호를 전달한다. 범위/점유를 확인하고 이후 생성에 쓸 슬롯을 기록한다.
bool_t Client::CCharacterSelectionState::Select_CreationSlot(const size_t slot)
{
	if (slot >= CCharacterRoster::MAX_CHARACTERS || CCharacterRoster::Is_Occupied(slot))
		return false;
	std::scoped_lock lock{ g_SelectionMutex };
	g_CreationSlot = slot;
	return true;
}

// Level_CharacterSelect의 생성 확인에서 호출한다. Server 입장 실패 시 호출자가 Cancel_PendingCreation으로 대기를 정리한다.
bool_t Client::CCharacterSelectionState::Stage_Creation(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::string_view nickname, const std::string_view appearanceJson)
{
	return Stage_Pending(characterClass, nickname, appearanceJson, true);
}

// MainApp이 카드에서 읽은 ID/class/nickname/외형이 현재 roster와 같은지 다시 대조한다.
// 명시적 false는 pending을 교체하지 않는다. 해당 roster를 순회하는 동안 외부 동시 변경을 막는 책임은 호출 흐름에 있다.
bool_t Client::CCharacterSelectionState::Stage_ExistingEntry(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::string_view nickname, const std::string_view appearanceJson, const std::string_view characterId)
{
	const auto& entries = CCharacterRoster::Get_Entries();
	const auto entry = std::find_if(entries.begin(), entries.end(), [&](const auto& candidate)
		{
			return candidate.strCharacterId == characterId && candidate.eCharacterClass == characterClass &&
				candidate.strNickname == nickname;
		});
	if (characterId.empty() || entry == entries.end() || entry->strAppearanceJson != appearanceJson)
		return false;
	return Stage_Pending(characterClass, entry->strNickname, entry->strAppearanceJson, false, entry->strCharacterId);
}

// spawn/표시 준비가 참조할 외형 문자열을 복사한다. 반환 후 caller가 보관해도 내부 문자열 수명에 의존하지 않는다.
std::string Client::CCharacterSelectionState::Get_ActiveAppearanceJson()
{
	std::scoped_lock lock{ g_SelectionMutex };
	// Bern identity 확정보다 먼저 입장 캐릭터가 spawn하므로 pending 외형을 우선한다.
	// 확정 외형만 읽으면 이전 캐릭터 외형이나 빈 외형으로 첫 표시를 만들 수 있다.
	if (g_PendingCreation.has_value())
		return g_PendingCreation->strAppearanceJson;
	return g_ActiveAppearanceJson;
}

// MainApp 전환 경로와 replication 연결 종료 경로가 마지막 수신 HUD 상태를 보관한다.
// Server capture barrier 응답을 요청하는 함수는 아니며, barrier/restore 보호 중에는 이 보조 저장을 건너뛴다.
void Client::CCharacterSelectionState::Capture_ActiveWorldState()
{
	std::string characterId;
	{
		std::scoped_lock lock{ g_SelectionMutex };
		if (g_ActiveCharacterId.empty() || g_RestoreRequired || g_HasFinalCapture ||
			g_CaptureStatus != CHARACTER_CAPTURE_STATUS::NONE)
			return;
		characterId = g_ActiveCharacterId;
	}
	const auto level = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
	if (level != LEVEL::BERN && level != LEVEL::VALTAN_ARENA &&
		level != LEVEL::KOUKUSAYDON_ARENA && level != LEVEL::MAHARAKA &&
		level != LEVEL::COLOSSEUM)
		return;
	const CCombatHUDViewModel& ViewModel = CCombatHUDViewModel::Get();
	const LostArk::Shared::S2C_INVENTORY_SNAPSHOT& Inventory = ViewModel.Get_Inventory();
	const auto& Player = ViewModel.Get_Player();
	// 유효 player와 inventory를 아직 받지 못했다면 저장할 상태로 보지 않고 기존 슬롯을 유지한다.
	if (!Player.isValid || Player.isPreview || !ViewModel.Has_Inventory())
		return;
	CHARACTER_WORLD_STATE State{};
	State.bValid = true;
	State.Items = Inventory.Items;
	State.iSilver = Inventory.iSilver;
	State.iGold = Inventory.iGold;
	State.iHonorTitleId = Player.iHonorTitleId;
	std::string strStatus;
	if (!CCharacterRoster::Update_WorldState(characterId, State, strStatus))
		OutputDebugStringA(("[CharacterSelection] World state capture failed: " + strStatus + "\n").c_str());
}

// 확정된 local ID의 존재만 확인한다. 현재 Server 연결이나 살아 있는 캐릭터 여부를 판정하지 않는다.
bool_t Client::CCharacterSelectionState::Has_ActiveCharacter()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return !g_ActiveCharacterId.empty();
}

// 카드 선택 복구에 쓸 local ID를 값으로 복사한다.
std::string Client::CCharacterSelectionState::Get_ActiveCharacterId()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_ActiveCharacterId;
}

// 요청 전/응답 대기를 포함한 복원 필요 상태다. 거절 뒤에도 true를 유지한다.
bool_t Client::CCharacterSelectionState::Is_RestorePending()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_RestoreRequired;
}

// PlayerController 등의 gameplay 명령 차단에 쓰는 상태다. capture의 CAPTURED/FAILED도 MainApp이 Finish할 때까지 대기로 본다.
bool_t Client::CCharacterSelectionState::Is_WorldStateSyncPending()
{
	std::scoped_lock lock{ g_SelectionMutex };
	// 저장 상태의 복원 보호는 복구 뒤에도 유지하지만, 생성 아레나의 audition 입력까지 Bern 복원 대기로 막지는 않는다.
	const bool_t restoringBern = g_RestoreRequired &&
		CGameInstance::Get().Get_CurrentLevelID() == static_cast<uint32_t>(LEVEL::BERN);
	return restoringBern || g_CaptureStatus != CHARACTER_CAPTURE_STATUS::NONE;
}

// 같은 process의 capture/restore 요청에 쓰는 번호다. 증분은 잠금 안에서 처리하고 wrap의 0은 생략한다.
std::uint32_t Client::CCharacterSelectionState::Next_StateRequestSequence()
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (++g_NextStateSequence == 0u)
		++g_NextStateSequence;
	return g_NextStateSequence;
}

// MainApp이 요청 전송 전에 비교 기준을 고정한다. 입력 또는 현재 상태가 허용되지 않으면 아무 capture 필드도 바꾸지 않는다.
bool_t Client::CCharacterSelectionState::Begin_WorldStateCapture(const std::uint32_t sequence,
	const LostArk::Shared::WORLD_ID world, const LostArk::Shared::PLAYER_ID player,
	const LostArk::Shared::NET_ENTITY_ID entity, const std::uint64_t generation)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (!sequence || !player || !entity || !generation || g_ActiveCharacterId.empty() ||
		g_RestoreRequired || g_CaptureStatus != CHARACTER_CAPTURE_STATUS::NONE)
		return false;
	g_CaptureSequence = sequence;
	g_CaptureWorld = world;
	g_CapturePlayer = player;
	g_CaptureEntity = entity;
	g_CaptureGeneration = generation;
	g_CaptureCharacterId = g_ActiveCharacterId;
	g_CaptureStatus = CHARACTER_CAPTURE_STATUS::WAITING;
	return true;
}

// ClientReplication이 world 세대와 함께 전달한다. 현재 요청이 아니면 false이고 상태를 보존한다.
// 요청 번호/세대가 맞으면 우선 FAILED로 표시하고 나머지 identity와 Server 결과까지 맞아야 roster를 갱신한다.
// true는 응답을 처리했다는 뜻이다. 성공 여부는 Get_WorldStateCaptureStatus의 CAPTURED로 별도 확인한다.
bool_t Client::CCharacterSelectionState::Apply_CaptureResult(
	const LostArk::Shared::S2C_CAPTURE_CHARACTER_RESULT& result, const std::uint64_t generation)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (g_CaptureStatus != CHARACTER_CAPTURE_STATUS::WAITING ||
		result.iRequestSequence != g_CaptureSequence || generation != g_CaptureGeneration)
		return false;
	g_CaptureStatus = CHARACTER_CAPTURE_STATUS::FAILED;
	const auto& entries = CCharacterRoster::Get_Entries();
	const auto entry = std::find_if(entries.begin(), entries.end(), [&](const auto& candidate)
		{
			return candidate.strCharacterId == g_CaptureCharacterId;
		});
	if (result.eResult != LostArk::Shared::CHARACTER_CAPTURE_RESULT::CAPTURED ||
		result.eWorldId != g_CaptureWorld || result.iPlayerId != g_CapturePlayer ||
		result.iNetEntityId != g_CaptureEntity || g_ActiveCharacterId != g_CaptureCharacterId ||
		entry == entries.end() || result.eCharacterClass != entry->eCharacterClass)
		return true;
	CHARACTER_WORLD_STATE state{};
	state.bValid = true;
	state.Items = result.Items;
	state.iSilver = result.iSilver;
	state.iGold = result.iGold;
	state.iHonorTitleId = result.iHonorTitleId;
	std::string status;
	if (CCharacterRoster::Update_WorldState(g_CaptureCharacterId, state, status))
	{
		g_CaptureStatus = CHARACTER_CAPTURE_STATUS::CAPTURED;
		g_HasFinalCapture = true;
	}
	return true;
}

// MainApp의 복귀 처리에서 응답 대기/성공/실패를 구분한다. 상태를 소비하거나 초기화하지 않는다.
Client::CHARACTER_CAPTURE_STATUS Client::CCharacterSelectionState::Get_WorldStateCaptureStatus()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_CaptureStatus;
}

// MainApp의 성공/실패/시간 초과 후 요청 대기를 닫는다. world/player/entity/generation의 이전 값은 남지만 NONE 상태로 무시된다.
void Client::CCharacterSelectionState::Finish_WorldStateCapture()
{
	std::scoped_lock lock{ g_SelectionMutex };
	// 거절/시간 초과로 끝난 capture도 오래된 HUD snapshot으로 덮지 않도록 보호한다.
	if (g_CaptureStatus != CHARACTER_CAPTURE_STATUS::NONE)
		g_HasFinalCapture = true;
	g_CaptureStatus = CHARACTER_CAPTURE_STATUS::NONE;
	g_CaptureSequence = 0u;
	g_CaptureCharacterId.clear();
}

// Level_Bern이 아직 전송하지 않은 복원 요청을 만들 때만 저장값을 얻는다. 조건이 맞지 않으면 출력은 유지한다.
bool_t Client::CCharacterSelectionState::Try_Get_ActiveWorldState(CHARACTER_WORLD_STATE& outState)
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_RestoreRequired && g_RestoreRequestSequence == 0u && !g_ActiveCharacterId.empty() &&
		CCharacterRoster::Try_Get_WorldState(g_ActiveCharacterId, outState);
}

// Level_Bern의 command sink 전송 성공 뒤 요청 번호를 기록한다. 번호 자체의 0 여부를 이 함수가 검증하지는 않는다.
void Client::CCharacterSelectionState::Mark_RestoreRequested(const std::uint32_t sequence)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (g_RestoreRequired)
		g_RestoreRequestSequence = sequence;
}

// ClientReplication이 요청 번호가 맞는 응답인지 판별한다. true는 응답 소비이며 Server APPLIED와 같은 의미가 아니다.
// APPLIED일 때만 복원 보호와 최종 capture 표시를 해제한다. 저장된 roster 자체는 이 응답으로 다시 쓰지 않는다.
bool_t Client::CCharacterSelectionState::Apply_RestoreResult(
	const LostArk::Shared::S2C_RESTORE_CHARACTER_RESULT& result)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (!g_RestoreRequired || !g_RestoreRequestSequence || result.iRequestSequence != g_RestoreRequestSequence)
		return false;
	// 거절된 요청 번호와 복원 필요 상태는 그대로 남긴다. 다른 월드에서 돌아와 임의 재전송하거나 기본값을 저장하지 않게 한다.
	if (result.eResult == LostArk::Shared::CHARACTER_RESTORE_RESULT::APPLIED)
	{
		g_RestoreRequired = false;
		g_HasFinalCapture = false;
	}
	return true;
}

// 신규 생성과 기존 카드 재선택 모두 pending으로 취급한다. 이름과 달리 신규 여부만 묻는 함수는 아니다.
bool_t Client::CCharacterSelectionState::Has_PendingCreation()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_PendingCreation.has_value();
}

// MainApp의 Bern 전환 성공 이후 호출이다. pending 부재 또는 roster Add의 명시적 실패면 기존 확정 identity를 유지한다.
// 저장 상태가 있는 기존 카드면 복원 필요를 켜고 요청 번호를 초기화한다. allocation 예외 rollback을 제공하는 함수는 아니다.
bool_t Client::CCharacterSelectionState::Commit_PendingCreation()
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (!g_PendingCreation.has_value())
		return false;

	std::string activeCharacterId = g_PendingCreation->strCharacterId;
	// 새 캐릭터를 roster에 추가하는 시점은 MainApp이 실제 Bern 진입을 완료한 뒤다.
	if (g_PendingCreation->bNewCharacter)
	{
		size_t iNewIndex = 0;
		std::string strRosterStatus;
		if (!CCharacterRoster::Add(g_PendingCreation->iSlot, g_PendingCreation->eCharacterClass,
			g_PendingCreation->strNickname, g_PendingCreation->strAppearanceJson,
			iNewIndex, strRosterStatus))
		{
			OutputDebugStringA(("[CharacterSelection] Roster creation failed: " + strRosterStatus + "\n").c_str());
			return false;
		}
		activeCharacterId = CCharacterRoster::Get_Entries()[iNewIndex].strCharacterId;
	}
	g_SelectedClass = g_PendingCreation->eCharacterClass;
	g_ActiveCharacterId = std::move(activeCharacterId);
	g_HasFinalCapture = false;
	g_CaptureStatus = CHARACTER_CAPTURE_STATUS::NONE;
	CHARACTER_WORLD_STATE saved;
	g_RestoreRequired = !g_ActiveCharacterId.empty() && CCharacterRoster::Try_Get_WorldState(g_ActiveCharacterId, saved);
	g_RestoreRequestSequence = 0u;
	g_CreatedNickname = std::move(g_PendingCreation->strNickname);
	g_ActiveAppearanceJson = std::move(g_PendingCreation->strAppearanceJson);
	g_CreationSlot.reset();
	g_PendingCreation.reset();
	return true;
}

// 대기 정보만 지운다. 현재 확정 identity, roster, 생성 슬롯 선택은 그대로 남겨 취소 후 재시도에 사용한다.
void Client::CCharacterSelectionState::Cancel_PendingCreation()
{
	std::scoped_lock lock{ g_SelectionMutex };
	g_PendingCreation.reset();
}

// Lobby의 입장 요청이 사용할 값을 임시로 조립한다. 최종 검증을 통과한 경우만 outIdentity를 교체한다.
bool_t Client::CCharacterSelectionState::Try_Resolve_ForWorld(
	const LostArk::Shared::WORLD_ID worldId,
	CHARACTER_ENTRY_IDENTITY& outIdentity)
{
	using namespace LostArk::Shared;

	std::scoped_lock lock{ g_SelectionMutex };
	CHARACTER_ENTRY_IDENTITY staged{};

	// 생성 아레나/수련장은 audition, Bern은 pending 우선, 그 밖의 지원 월드는 확정 identity 또는 audition을 사용한다.
	switch (worldId)
	{
	case WORLD_ID::CHARACTER_SELECT_ARENA:
	case WORLD_ID::TRAINING_GROUND:
		staged.eCharacterClass = g_SelectedClass.value_or(
			CHARACTER_CLASS_ID::LANCE_MASTER);
		staged.strNickname = Get_AuditionNickname();
		staged.eSource = CHARACTER_ENTRY_IDENTITY_SOURCE::AUDITION;
		break;

	case WORLD_ID::BERN:
		if (g_PendingCreation.has_value())
		{
			staged.eCharacterClass = g_PendingCreation->eCharacterClass;
			staged.strNickname = g_PendingCreation->strNickname;
			staged.iVoiceType = CCustomizingView::Read_SavedVoiceType(
				g_PendingCreation->strAppearanceJson);
			staged.eSource =
				CHARACTER_ENTRY_IDENTITY_SOURCE::PENDING_CREATION;
			break;
		}
		// pending이 없는 직접 Bern 입장은 기존 확정 identity 또는 audition 정책을 공유한다.
		// 명시적으로 stage한 pending만 Bern 활성화 뒤 commit 대상이 된다.
		[[fallthrough]];

	case WORLD_ID::VALTAN_ARENA:
	case WORLD_ID::KOUKUSAYDON_ARENA:
	case WORLD_ID::MAHARAKA:
	case WORLD_ID::COLOSSEUM:
		staged.eCharacterClass = g_SelectedClass.value_or(
			CHARACTER_CLASS_ID::LANCE_MASTER);
		if (g_CreatedNickname.has_value())
		{
			staged.strNickname = *g_CreatedNickname;
			staged.iVoiceType = CCustomizingView::Read_SavedVoiceType(g_ActiveAppearanceJson);
			staged.eSource = CHARACTER_ENTRY_IDENTITY_SOURCE::CREATED;
		}
		else
		{
			staged.strNickname = Get_AuditionNickname();
			staged.eSource = CHARACTER_ENTRY_IDENTITY_SOURCE::AUDITION;
		}
		break;

	default:
		return false;
	}

	if (!Is_Supported_Playable_Character_Class(
			staged.eCharacterClass) ||
		!Is_Valid_PlayerNickname(staged.strNickname))
	{
		return false;
	}

	outIdentity = std::move(staged);
	return true;
}
