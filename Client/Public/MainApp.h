#pragma once

// Client 이름 공간·LEVEL·공통 타입을 사용한다.
#include "Client_Defines.h"
// 장면 렌더링·품질 draft와 행렬·광원 값을 멤버로 보관한다.
#include "Engine_RenderTypes.h"
// D3D COM 참조·HRESULT·컨테이너 등의 공통 선언을 사용한다.
#include "Engine_Defines.h"
// Loading 전환과 로비 승인 흐름을 연결하는 명령 토큰을 사용한다.
#include "LobbyCommandService.h"
// Server 요청과 복제 식별자·장비·전투 enum을 원래 typed 값으로 보관한다.
#include "Network/PacketMessages.h"
// 장면/품질 프로필의 조회·활성화 서비스 수명을 소유한다.
#include "RenderingProfileService.h"
// 재사용 광원 정의와 미리보기용 catalog를 소유한다.
#include "LightResourceCatalog.h"
// Area 조명 draft·저장·게시의 기존 세션을 값으로 소유한다.
#include "AreaLightAuthoringSession.h"
// Server 복제 상태를 읽는 HUD 타입과 Debug preview 값을 사용한다.
#include "CombatHUDViewModel.h"

// 고정 개수의 창 표시·slot·아레나 조회 상태를 보관한다.
#include <array>
// 요청 timeout·프레임 제한·미리보기 경과 시간을 steady_clock으로 측정한다.
#include <chrono>
// 이 헤더에서는 직접 쓰지 않지만 MainApp.cpp의 파일 경로 처리가 간접 포함에 의존한다.
#include <filesystem>

NS_BEGIN(Engine)
class CImGuiLayer;
NS_END

NS_BEGIN(Client)

// MainApp이 보관하거나 연결할 Client UI·도구의 타입들이다. 실제 생성과 연결은 MainApp.cpp에서 한다.
class CMapTool;
class CLevelNavigationDebug;
class CEffect_Tool;
class CEffect_Tool_V2;
class CAnimation_Tool;
class CValtanActionWorkbench;
class CKoukuSaydonActionWorkbench;
class CKoukuSaydonPresentationPlayer;
class CHUDLayoutTool;
class CUILayoutRuntime;
class CDungeonTimerView;
class CWorldHealthBarView;
class CBalanceTool;
class CValtanBossTool;
class CKoukuSaydonBossTool;
class CCameraTool;
class CCharacterPreviewPanel;
class CEquipmentAuthoringTool;
class CProfilerTool;
class CSequencerTool;
class CCharacterActionWorkbench;
class CWorldObjectTool;
class CWorldLevelTool;
class CWorldSceneTool;
class CGuideAITool;
class CMaharakaAITool;
struct WORLD_LEVEL_TOOL_REQUEST;
struct KOUKU_MAP_EFFECT_PLACEMENT_REQUEST;
struct KOUKU_SAYDON_COMPOSITION_DOCUMENT;
struct KOUKU_SAYDON_COMPOSITION_PATTERN;
class CWorldSequenceDocument;
class CRenderingBenchmark;
class CSkillWindowView;
class CInventoryView;
class CRepairWindowView;
class CShopWindowView;
class CDurabilityHudView;
class CCombatAnalysisFrameView;
class CCharacterInfoWindowView;
class CCharacterPortraitRenderer;
class CAvatarBookWindowView;
class CVehicleWindowView;
class CHonorTitleWindowView;
class CWorldMapWindowView;
class CSystemOptionWindowView;
class CSongCastGaugeView;
class CQuickSlotDragView;
class CPlayerController;
class CChatWindowView;
class CPartyWindowView;
class CCharacterSelectWindowView;
class CMinimapView;
class CEstherCutinPresentationService;

// Client.cpp가 생성·해제하며 제품 UI·도구와 프레임 순서, Level 전환 요청을 조정한다.
// Engine의 manager는 CGameInstance가 소유한다. CObject_Manager → CLayer → CGameObject → CComponent로 실제 객체를 관리한다.
// MainApp은 UI/도구 객체와 표시·요청 대기 상태를 보관하고 게임 객체 등록·갱신·렌더는 Engine 경로로 요청한다.
class CMainApp final
{
private:
	// 도구별 표시 상태와 단일 viewport 입력 소유자를 같은 index 집합으로 연결한다.
	enum class DEBUG_TOOL
	{
		NONE,
		MAP,
		VALTAN_ACTION_WORKBENCH,
		KOUKU_SAYDON_ACTION_WORKBENCH,
		ANIMATION,
		EFFECT,
		// V2 문서와 미리보기 창은 기존 도구와 독립적으로 관리한다.
		EFFECT_V2,
		RENDERING,
		UI,
		BALANCE,
		VALTAN_BOSS,
		KOUKU_SAYDON_BOSS,
		VALTAN_LOGIC_PATTERN,
		CAMERA,
		EQUIPMENT,
		SEQUENCER,
		SEQUENCER_BENCHMARK,
		PROFILER,
		WORLD_OBJECT,
		WORLD_LEVEL,
		WORLD_SCENE,
		GUIDE_AI,
		MAHARAKA_AI,
		EFFECT_COMPOSITION,
		COUNT
	};

	// 자원 검색 행의 경로·검색어와 열 도구를 보관한다.
	struct DEBUG_RESOURCE_FILE
	{
		string strDomain;
		string strSource;
		string strRelativePath;
		string strSearchText;
		DEBUG_TOOL eTool = DEBUG_TOOL::NONE;
	};

private:
	// Client.cpp의 Create 경로에서 현재 앱을 등록한다.
	CMainApp();

public:
	// 현재 앱 주소를 비운 뒤 Free로 도구·네트워크·Engine 종료 순서를 진행한다.
	~CMainApp();

public:
	// Engine과 공용 원형·UI를 준비하고 Lobby로 가는 Loading을 시작한다.
	HRESULT Initialize();
	// Client.cpp가 전달한 delta(초)로 입력·Level·UI를 갱신하고 Level Update 뒤 전환 요청을 처리한다.
	void Update(f32_t fTimeDelta);
	// Engine의 월드/UI 렌더와 도구 표시를 연결한다. 제품 글자는 UI 합성 뒤에 그린다.
	HRESULT Render();
	// Client.cpp의 프레임 끝에서 전경/배경 FPS 제한을 맞춘다. 제한이 없으면 기준 시각만 갱신한다.
	void Limit_FrameRate();

	// Level/NPC가 MainApp 소유 창을 열 때 현재 인스턴스를 차용한다. 생성 시 등록하고 소멸 전에 비운다.
	static CMainApp* Get_Active()
	{
		return s_pActiveInstance;
	}
	// 베른 슈미트 NPC가 재련 창을 연다. 새로 열 때만 대기 상태를 초기화한다.
	void Open_ItemUpgradeWindow();
	// 베른 선박 NPC가 탈것 창을 선박 전용 모드로 연다.
	void Open_ShipWindow();
	// 베른 수리 NPC가 MainApp 소유 수리 창을 연다.
	void Open_RepairWindow();
	// 베른 물약 상인의 placement ID로 상점의 상품과 요청 대상을 정한다.
	void Open_ShopWindow(const string& strNpcPlacementId);
	// 월드 하단 메뉴 버튼이 Escape와 같은 환경설정 창을 연다. 이미 열려 있으면 유지한다.
	void Open_SystemOptionsWindow();
	// 활성 캐릭터가 있으면 Server의 상태 capture를 요청하고 이후 Pump가 로비 복귀를 마무리한다.
	// false는 capture 대기 시작과 예약·전송 실패를 모두 포함한다. true는 로비 LOAD 요청을 예약하고 연결을 닫은 경우다.
	bool_t Return_ToCharacterSelect();
	// Update가 capture 완료·연결 세대·timeout을 확인한다. 실패하면 슬롯을 다시 덮어쓰지 않고 대기를 정리하며 진단을 기록한다.
	// Apply_CaptureResult가 이미 반영한 뒤 생긴 전환 실패까지 로스터에서 되돌리는 함수는 아니다.
	void Pump_ReturnToCharacterSelect();

	// Debug 제목의 FPS를 최소 500ms 간격으로 갱신한다. 기본 제목이 바뀌면 간격을 기다리지 않고 반영한다.
	static void Update_DebugWindowTitleWithFps(const wchar_t* pBaseTitle);
	// 각 도구의 Complete Play 선택을 공용 stable Pattern ID에 모은다. 도구별 편집 draft는 유지한다.
	bool_t Debug_SelectCompletePlayPattern(const std::string& strPatternId);
	const std::string& Debug_GetSelectedCompletePlayPatternId() const;
	// 선택한 패턴을 CValtanBossTool → CValtanPatternAuditionService의 Server 재생 경로로 요청한다.
	bool_t Debug_CompletePlaySelected(std::string& strOutStatus);
    // Map Tool 편집 전에 World Scene의 placement 편집 해제를 요청한다. 실패하면 그 도구의 draft를 보존한다.
    bool Debug_PrepareMapToolAuthoring(std::string& status);
    // 같은 Area의 Object Details 편집 전에 Map Tool의 미저장 변경과 preview 복원을 확인한다. 거부 이유는 status로 전달한다.
    bool Debug_PrepareWorldPlacementEditing(const std::string& areaId, std::string& status);
    // 현재 live Area의 stable placement를 공용 상세 창에 넘긴다. 준비·선택 실패 시 이전 창 표시 상태 복원을 시도한다.
    bool Debug_OpenWorldObjectDetails(const std::string& areaId, uint64_t placementId,
        bool deploy, std::string& status);
	// 통합 composition 도구의 요청을 Valtan Boss Tool의 Flow 편집 창에 연결한다.
	bool_t Debug_OpenValtanPatternFlow(std::string& strOutStatus);

private:
	// Update가 자원 준비→START 응답→참가자 READY→Server 시각 연출을 진행한다.
	// 연결·world generation 변경은 이전 요청 추적을 비우며, 준비 실패와 Server 응답 지연을 구분해 기록한다.
	void UpdateKoukuGateCompletePlay();
    // 쿠크 Complete Play의 로컬 준비·READY 전송·Server 수신 상태를 표시한다.
    void RenderKoukuRaidPreparationDiagnostics();
    // 현재 run epoch의 준비·전송 진단이다. observedAt은 로컬 표시용 시각이다.
    struct KOUKU_RAID_PREPARATION_DIAGNOSTICS final
    {
        uint32_t epoch = 0u;
        std::string stage, detail, submission;
        bool failed = false;
        std::chrono::steady_clock::time_point observedAt{};
    };
    KOUKU_RAID_PREPARATION_DIAGNOSTICS m_KoukuRaidPreparationDiagnostics;
	// Update와 도구 화면이 공유하는 관문 실행 상태다. Sequence 복사본은 document epoch가 바뀌면 다시 읽는다.
	string m_strKoukuCompletePlayFlowGate;
	std::unique_ptr<KOUKU_SAYDON_COMPOSITION_DOCUMENT> m_pKoukuRaidSequenceDocument;
	// 전송한 START/STOP 원문과 대기 순번을 보존해 정확한 응답에 대응한다. 다음 순번이 0이 되면 새 요청을 거부한다.
	LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST m_KoukuRaidRequest;
	std::uint32_t m_iNextKoukuRaidRequest = 1u, m_iKoukuRaidPendingRequest = 0u, m_iKoukuRaidDocumentEpoch = 0u;
	// run·문서·패턴·시작 tick의 키로 이미 시작한 연출과 실패한 연출을 구분한다. 같은 실패 키의 재시작을 막는다.
	std::string m_strKoukuRaidPresentationKey, m_strKoukuRaidFailedKey;
	// START/STOP 전송 후 5초가 지나면 지연 안내만 기록한다. timeout으로 대기 순번을 지우거나 두 번째 START를 허용하지 않는다.
	std::chrono::steady_clock::time_point m_KoukuRaidReplyDeadline{};
	std::string m_strKoukuRaidReplyStatus; // broadcast 레이드 표시와 별도로 보관하는 최신 미해결·거절 요청 상태다.
    // START를 보내기 전의 임시 준비다. 요청의 revision과 대상 패턴을 값으로 고정하고 연결 세대·20분 제한을 Update에서 재검사한다.
    // 실패·취소 시 이 준비를 폐기한다. 서버에 이미 전송한 요청은 위 pending 순번으로 따로 추적한다.
    struct KOUKU_RAID_RESOURCE_PREPARATION final
    {
        LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
        std::vector<std::string> patternIds;
        uint64_t worldGeneration = 0u;
        std::chrono::steady_clock::time_point deadline;
    };
    std::optional<KOUKU_RAID_RESOURCE_PREPARATION> m_KoukuRaidResourcePreparation;
    // Server PREPARING의 run epoch별 게시 패턴 목록이다. 같은 실행의 자원 준비를 이어갈 때 목록을 재사용한다.
    uint32_t m_iKoukuRaidResourceEpoch = 0u;
    std::vector<std::string> m_KoukuRaidResourcePatternIds;

	// START/STOP 응답을 기다리는 중 요청된 중지는 응답을 받은 뒤 처리한다. 로컬 preview 중지 예약과 Server 실행 epoch를 구분한다.
	bool_t m_bKoukuRaidStopAfterAdmission = false;
	bool_t m_bKoukuLocalPreviewStopRequested = false;
	uint64_t m_iKoukuCompletePlayWorldGeneration = 0u;
	// 이 Client가 READY 또는 FAILED 제출에 성공한 run epoch다. Server의 READY 확인 여부는 수신한 ready mask로 별도 판정한다.
	std::uint32_t m_iKoukuRaidAcknowledgedEpoch = 0u;
	string m_strKoukuCompletePlayStatus =
		"Load the KoukuSaydon inventory, pick a gate and a saved pattern, then Complete Play.";

	// Initialize가 제품 폰트를 등록한다. 필수 폰트 실패만 시작을 막는다.
	HRESULT Ready_Fonts();
	// Level을 넘어 쓸 셰이더·quad·Effect·UI 원형을 Engine의 STATIC 저장소에 등록한다.
	HRESULT Ready_Prototype_For_Static();
	// Loading 화면 자원을 STATIC 원형으로 등록한다. 문서 누락은 건너뛰고 등록 실패는 반환한다.
	HRESULT Ready_Prototype_For_LoadingChrome();
	// 목적지 descriptor와 profile을 검사한 뒤 Loading을 생성한다. 교체 실패 시 이전 profile 복원을 시도하며 실패 진단을 보존한다.
	HRESULT Start_Level(
		LEVEL eTargetLevel,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	// 프레임 경계에서 비동기 준비의 취소 완료를 기다린 뒤 LOAD/ACTIVATE 요청을 한 번 소비한다.
	// 활성화 실패는 생성 자원을 정리하고 연결을 닫은 뒤 기존 서비스로 로비 복구를 예약한다.
	void Apply_LevelRequest();
	// MainApp 소유 ImGui layer를 장치·창에 연결하고 실행 모드별 UI 설정을 적용한다.
	HRESULT ReadyImGuiRuntime();
	// 제품 단축키·프레임 제한·F7에서 전경 창이 이 process에 속하는지 확인한다.
	static bool_t IsWindowOwnedByCurrentProcess(HWND hWnd);
	// Update가 비동기 저장 진행을 확인하고 F7 상승 edge로 Profiler 창을 토글한다. 텍스트 입력 중에는 단축키를 소비하지 않는다.
	void UpdateProfilerRuntime();

	//I/P/N/M 키로 열고 닫기
	void Update_RuntimeWindowShortcuts();

	//UI가 숨겨진 동안 네 단축키의 눌림 상태만 갱신한다.
	void Sync_RuntimeWindowShortcutKeys(bool_t bFocused);

	// Update가 cinematic 표시 조건을 UI router에 맞추고, 억제가 시작되면 창 gesture와 운반 아이콘을 취소한다.
	void Sync_CinematicUI();
	// 현재 연출의 시각·배우 위치로 자막을 표시한다. 수집한 월드 자막은 instanceId·subtitleTrackId 쌍으로 같은 프레임의 중복을 제거한다.
	void RenderCinematicSubtitles();

	// 복제 상태를 HUD_Layout의 STATIC sprite에 반영한다. 표시 대상이 아니면 남아 있는 슬롯을 명시적으로 숨긴다.
	void Update_CombatHUD(f32_t fTimeDelta);
	// m_pHUDRuntimeView의 슬롯만 숨긴다. 보스·에스더 등 별도 view는 각 Hide 함수가 맡는다.
	void Hide_CombatHUD();
	// HealthBar/ManaBar는 빈 배경을 유지하고 별도 Fill 슬롯의 g_FillRatio로 현재/최대 HP·resource 비율을 표시한다.
	void Update_PlayerHealthManaBar();
	// 등록된 아이템을 Item_1..4_Icon에 표시한다. 인벤토리 drop의 화면 위치를 실제 viewport 기준 슬롯 rect와 비교한다.
	void Update_ItemQuickSlots();
	// UI 합성 뒤 스킬·특수·아이템 슬롯 그림 위에 단축키 글자를 그린다.
	void RenderQuickSlotKeyLabels();
	// 해양 HUD의 보급량·노트 속도와 슬롯·버튼 단축키 글자를 표시한다.
	void RenderShipHudTexts();
	// 쿠크 HUD 모드가 있으면 class identity와 T/V를 숨기고 Q..F에 모드 아이콘·쿨다운을 덮어쓴다.
	// 기본 스킬 아이콘·쿨다운 갱신 뒤 실행하며 모드가 없으면 추가 문양 슬롯을 숨긴다.
	void Update_KoukuHudMode();
	// 탑승 HUD가 class identity 대신 탈것 문양과 전용 스킬을 표시한다.
	// 쿠크 HUD 갱신 뒤 실행하며 미탑승 상태의 문양은 기본 ownerClass 갱신에서 숨긴다.
	void Update_VehicleHud();
	// Server가 마하라카 물총 장착을 알리면 줍기 문양과 Q/W/E/R 물총 스킬·복제된 쿨다운을 표시한다.
	// 탈것 HUD 다음에 실행하고 class identity·T/V를 숨기며 A/S/D/F는 빈 어두운 슬롯으로 둔다.
	void Update_WaterGunHud();
	// 인벤토리·탈것 창이 시작한 아이콘 운반을 소비한다. 아이템은 Item_1..4, 탈것은 SpecialSkill_1..6에 drop하며 다른 곳은 놓는다.
	// 창 Update 다음에 실행해 같은 프레임의 선택을 읽는다.
	void Update_QuickSlotDrag();
	// SpecialSkill_1..6_Icon에 5/6/7/8/9/0으로 등록한 탈것을 표시한다.
	void Update_SpecialQuickSlots();
	// Space 이동 스킬 또는 탑승 탈것의 Space 돌진 쿨다운을 문양 위에 표시한다.
	// 쿨다운이 없거나 쿠크 상호작용 모드이면 숨기며 탈것 HUD 다음에 갱신한다.
	void Update_SpecialSlot();
	// 복제된 탈것·방어 태세·침묵·속박·공포 등의 상태로 buff/debuff 줄을 구성한다.
	// 아이콘 정의는 HudBuffSources.json, 남은 시간 글자는 RenderSkillCooldownText가 담당한다.
	void Update_BuffBar();
	// SkillSlotMarks.json의 combo/holding/perfectCombo 표시와 Server combo 입력 시간의 원형 표시를 갱신한다.
	// 탑승 또는 쿠크 모드에서는 숨긴다.
	void Update_SkillSlotMarks();
	// SkillSlotMarks.json과 HudBuffSources.json을 읽는다. 파일 누락·오류는 해당 표시를 비운 상태로 처리한다.
	void Load_HudQuickSlotData();
	// class quick slot 밖 스킬의 남은 쿨다운 비율을 catalog cooldownMs로 계산한다. 0은 준비 완료이며 슬롯 위 초 단위 글자도 예약한다.
	f32_t Resolve_HudCooldownRatio(const HUD_PLAYER_STATE& player, uint32_t iSkillId,
		uint32_t iCooldownMs, const string& strTextSlotId);
	// 현재 Level의 플레이어 controller를 차용한다. 탈것 요청은 이 controller를 거치며 없는 Level이면 nullptr이다.
	CPlayerController* Find_ActivePlayerController() const;
	// KoukuHudModes.json의 모드 정의를 읽는다. 누락·오류면 빈 목록을 유지해 임의 모드를 표시하지 않는다.
	void Load_KoukuHudModes();
	// LOBBY에서 배경·로고·서버 목록과 접속(서버 선택), 종료/뒤로/환경설정 버튼을 갱신한다. 접속은 캐릭터 선택 창을 연다.
	void Update_LobbyButtons(f32_t fTimeDelta);
	// 로비의 캐릭터 선택 창을 갱신하고 생성 의도를 CHARACTER_SELECT 명령으로 연결한다. 열린 동안 로비 버튼 입력을 막는다.
	void Update_CharacterSelectWindow(f32_t fTimeDelta);
	// 프레임 시작에 다른 runtime 창이 없을 때만 Escape로 환경설정을 연다.
	// 같은 Escape로 다른 창을 닫으며 환경설정까지 열지 않고, 영상 옵션 변경 후 장면 프로필을 다시 활성화한다.
	void Update_SystemOptionWindow(f32_t fTimeDelta);
	bool_t Is_AnyRuntimeWindowOpen() const;
	// Loading 진입 때 runtime 창을 닫는다. STATIC sprite만 숨기고 열린 상태를 남겨 글자가 Loading 위에 그려지는 일을 막는다.
	void Close_RuntimeWindowsForLoading();
	// 미니게임·시네마틱·Loading의 HUD 숨김 조건을 조회한다. 제품 sprite와 텍스트가 같은 화면 점유 판정을 사용한다.
	bool_t Is_KoukuMinigameHUDHidden() const;
	bool_t Is_KoukuBossHealthBarHidden() const;
	bool_t Is_RuntimeUIScreenSuppressed() const;
	// Escape 한 번에 가장 최근에 연 창 하나를 닫기 위한 창 종류다.
	enum class ESCAPE_WINDOW : uint8_t
	{
		INVENTORY, CHARACTER_INFO, AVATAR_BOOK, HONOR_TITLE, VEHICLE, WORLD_MAP, REPAIR, SHOP, END
	};
	bool_t Is_EscapeWindowOpen(ESCAPE_WINDOW eWindow) const;
	// 닫힌 창은 순서에서 빼고 새로 열린 창을 맨 뒤에 추가한다.
	void Sync_EscapeWindowOrder();
	// 가장 최근에 연 창 하나를 닫는다. 열린 창이 없으면 false다.
	bool_t Close_TopEscapeWindow();
	// 이 프레임의 Escape를 나중에 직접 소비할 팝업·아이콘 운반 상태가 있는지 확인한다.
	bool_t Is_EscapeOwnedElsewhere() const;
	// Update 끝에서 창 rect를 텍스트 가림 영역으로 등록해 뒤의 이름표·HUD·말풍선이 창을 뚫고 보이지 않게 한다.
	// 표시 중인 UI의 텍스트 layer를 CUITextOcclusion에 함께 등록한다.
	void Register_UITextOccluders();
	// 서버 선택 패널의 제목·행·버튼·저작권과 Release 상태 글자를 EndFrame 이후 표시한다.
	void RenderLobbyButtonText();
	// LobbyServers.json을 서버 목록에 읽는다. 누락·오류면 행을 만들지 않고 접속 버튼을 비활성화한다.
	void Load_LobbyServers();
	// 게임 시작·서버 선택·카드 라벨은 RenderLobbyButtonText와 같이 EndFrame 뒤에 그려 UI 그림 위에 표시한다.
	void RenderCharacterSelectWindowText();
	// 현재 Level이 제공하는 복제 marker snapshot으로 우측 상단 미니맵을 갱신한다. 지원하지 않는 Level에서는 숨긴다.
	void Update_Minimap(f32_t fTimeDelta);
	void RenderMinimapText();
	// UI 합성 뒤 빈 버튼 그림 위에 흰색 "장비 재련"을 그린다.
	void RenderItemUpgradeButtonText();
	void RenderItemUpgradeLevelText();
	void RenderItemUpgradeMaterialCounts();
	void RenderItemUpgradeGaugePercentText();
	// WAITING에서 대기 원 위에 "화면을 클릭하여 결과 즉시 확인"을 표시한다.
	void RenderItemUpgradeResultWaitText();
	// SUCCESS frame 위에 아이템 이름·등급·"재련 성공"을 표시한다.
	void RenderItemUpgradeSuccessDetailText();
	// FAIL frame 위에 아이템 이름·"재련 실패"를 표시한다. 강화 단계는 그대로다.
	void RenderItemUpgradeFailDetailText();
	void RenderItemUpgradeListText();
	// UIInputRouter 입력으로 장비 선택·성장·재련 결과 화면을 갱신한다. 닫힘/미지원 Level에서는 STATIC 슬롯을 숨긴다.
	void Update_ItemUpgrade(f32_t fTimeDelta);
	void Update_CustomizingSceneProfile();
	void Update_ClassShowcaseShadowFocus();
	void Update_KoukuGateSceneProfile();
	void Hide_ItemUpgrade();
	void Update_ItemUpgradeSelection();
	// 성장 버튼을 누르면 gauge를 0부터 다시 채우고 CoreFlash를 한 번 시작한다.
	// 100%가 될 때까지 WingedRingGold·LevelUpMotion2Big·CompleteEffect는 숨긴다.
	void Update_ItemUpgradeGrowButton();
	// 선택한 인벤토리 identity와 기대 강화 단계를 Server 재련 요청에 담는다.
	void Update_ItemUpgradeReforgeButton();
	void Update_ItemUpgradeServerResult();
	// 요청과 일치하는 Server 결과가 도착해야 클릭으로 결과를 공개한다. 응답 대기 중에는 성공을 만들어 내지 않는다.
	void Update_ItemUpgradeResultWaitClick();
	// 표시 중인 성공/실패 확인 버튼을 누르면 결과 modal을 닫고 gauge를 대기 상태 0%로 되돌린다.
	void Update_ItemUpgradeResultOkButton();
	// 재련 요청/결과 닫기 때 gauge를 대기 0%로 돌려 이전 완료 표시를 지운다.
	void Reset_ItemUpgradeIdleGauge();
	// 결과 modal 뒤의 기본 중앙 패널을 숨기거나 복원한다. 완료 이펙트는 성장/초기화 흐름이 따로 관리한다.
	void Set_ItemUpgradeCenterPanelVisible(bool_t bVisible);
	// Update가 보스 HP의 fill·다음 줄·frame·무력화·구분선·줄 전환·피격 광택을 갱신한다.
	// 표시 조건이나 보스 유효성이 깨지면 STATIC 슬롯이 이전 상태를 남기지 않도록 모두 숨긴다.
	void Update_BossHealthBar();
	void Hide_BossHealthBar();
	// 복제된 적·다른 플레이어의 현재 pose를 최종 카메라 갱신 뒤 화면에 투영한다.
	void Update_WorldHealthBars(f32_t fTimeDelta);
	std::array<f32_t, 12> Get_HealthBarPositions() const;
	bool_t Set_HealthBarPositions(const std::array<f32_t, 12>& offsets);
	std::array<f32_t, 2> Get_MechanicBarScale() const;
	bool_t Set_MechanicBarScale(const std::array<f32_t, 2>& scale);
	bool_t Set_ValtanMechanicTuning(const std::array<f32_t, 8>& tuning);
	void Apply_MechanicBarRect();
	void Update_MechanicBarAnchor();
	bool_t Save_HealthBarPositions(std::string& status);
	bool_t Reload_HealthBarPositions(std::string& status);
	void RenderHUDBarPositionControls();
	// Server의 combo stage·action 시작 tick과 PlayerSkills.json의 stage 길이로 HOLD 진행률을 재구성한다.
	// 네트워크에 연속 충전 퍼센트가 없으므로 관측한 단계 시간을 사용한다.
	void Update_ChargeGauge();
	// 충전 gauge와 같은 표시 조건으로 스킬 이름을 track 가운데 그린다. EndFrame 이후의 제품 텍스트 pass를 사용한다.
	void RenderChargeGaugeText();
	// 보스 제목·HP·남은 줄 수는 EndFrame 뒤에 그린다. Draw_Text는 즉시 제출하므로 아래 UI 그림의 합성 이후에 호출한다.
	void RenderBossHealthBarText();
	// Level이 제공한 rect에 "사망하였습니다"·"부활"·"관전하기"를 UI 합성 뒤 그린다. 패널과 클릭 처리는 Level이 맡는다.
	void RenderDeadSceneText();
	// 레이드 클리어 제목은 Level이 채운 Get_RaidClearTextRects를 사용해 EndFrame 뒤에 표시한다.
	void RenderRaidClearText();
	// Level이 준비한 rect와 "OO을(를) 획득하였습니다" 문구를 ViewModel에서 받아 표시한다.
	void RenderItemAnnounceText();
	// 방이 공유하는 에스더 gauge를 STATIC UI 슬롯으로 표시한다. 발탄·쿠크 아레나와 유효한 gauge/player 등 표시 조건이 필요하다.
	// 조건을 벗어나면 Hide_EstherUI로 이전 프레임의 sprite도 숨긴다.
	void Update_EstherGauge(f32_t fTimeDelta);
	void Hide_EstherUI();
	// ESTHER 및 준비 완료·Ctrl+Z/X/C 안내는 gauge 위에 EndFrame 이후 그린다.
	void RenderEstherGaugeText();
	// 창술사의 3구간 identity를 퍼센트별 Gauge0/1/2 키프레임과 최초 점화→유지 반복 이펙트로 표시한다.
	void Update_LanceMasterIdentityGauge();
	// Server snapshot의 identity·태세·socket 수로 가디언나이트 구슬과 Embereth 10개 socket을 표시한다.
	void Update_GuardianKnightIdentity();
	// 각 Ember socket의 마지막 show/hide/lock 상태다. 바뀔 때만 전환을 재생하고 같으면 끝 프레임을 유지한다. 빈 값은 최초 상태다.
	string m_EmberSocketStates[10];
	// Server DAMAGE_EVENT의 피격 당시 월드 위치에 숫자를 띄운다. 대상의 이후 이동을 따라가지 않으며 EndFrame 뒤 Draw_Text로 표시한다.
	void RenderDamageNumbers();
	// Debug는 FPS 표시 옵션을 따른다. Release는 저장 옵션을 바꾸지 않고 FPS를 표시하며 상위 cinematic 숨김 조건을 따른다.
	void RenderFpsText();
	// class·stance에 대응한 스킬 아이콘을 Skill_<X>_Icon에 넣는다. 기존 Frame 슬롯은 sprite 순서상 위에 남긴다.
	void Update_SkillIcons();
	// 스킬 쿨다운을 g_ArcRatio로 원형 clip한다. 12시 방향에서 시계 방향으로 줄이며 남은 초 글자는 별도 텍스트 pass다.
	void Update_SkillCooldowns();
	// 스킬 슬롯·도화가 Z/X 위의 남은 초 글자를 EndFrame 이후 표시한다.
	void RenderSkillCooldownText();
	// 준비 완료→사용 불가 전환을 감지해 Skill_<X>_Flash 키프레임을 한 번 재생한다.
	void Update_QuickSlotFlash();
	// 제품 UI 합성 뒤 같은 표시 조건으로 HUD 글자와 보스 이름·HP·남은 줄 수를 그린다.
	void RenderCombatHUDText();

	// Debug 초기화에서 Profiler 기록·활성 상태를 초기화하고 HUD 배치 도구를 만든다. 나머지 도구는 필요할 때 준비한다.
	HRESULT ReadyDebugTools();
	// 열 도구와 연결할 세션을 준비한다. bShowWindow가 true이면 공용 표시·입력 focus도 갱신한다.
	HRESULT EnsureDebugTool(DEBUG_TOOL eTool, bool_t bShowWindow = true);
	// 리소스 선택에 필요한 공용 모델 preview·Balance·발탄·Animation 도구를 준비한다. 공용 창 표시·focus는 별도 열기 경로가 담당한다.
	HRESULT EnsureAnimationPreviewBackend();
	// 이전 도구 이름을 공용 창에 대응시켜 표시 상태를 읽는다. World Object/Sequence는 공용 창의 선택 대상도 함께 확인한다.
	bool_t IsDebugToolVisible(DEBUG_TOOL eTool) const;
	// 공용 표시 flag를 갱신하고 닫힐 때 입력·focus와 해당 도구 preview를 정리한다. 도구 객체와 편집 draft는 소유 멤버에 남긴다.
	void SetDebugToolVisible(DEBUG_TOOL eTool, bool_t bVisible);
	// 도구를 숨기고 preview를 정리한다. 도구 객체와 편집 draft는 유지한다.
	void CloseAllDebugTools();

	// 목적지 버튼과 전환 추적 상태를 표시한다. 15초 경과는 화면의 추적만 해제하며 실제 입장·전환 소유자의 처리는 취소하지 않는다.
	void RenderDebugLevelNavigation();
	// 현재 Level의 카메라·로컬 위치를 표시하고 명시적 Move Player를 controller에 예약한다. 물총 로컬 preview와 Server 패턴 요청은 별도 버튼이다.
	void RenderArenaCameraAndPlayerControls();
	// 현재 자유 카메라 이동 속도를 m/s로 편집한다. 아레나는 Level setter로, 다른 맵은 현재 카메라 인스턴스로 전달한다.
	void RenderCameraSpeedControls();
	// Developer Tools의 승하차·비행 카메라·속도 저작 패널이다. controller 요청, 로컬 preview, 파일 저장의 경계를 나눈다.
	void RenderDragonControls();
	// 캐릭터 선택 Level의 바닥 preset과 m/degree 단위 보정을 편집한다. Apply/Restore가 실제 Level에 요청하고 draft 선택만으로 적용하지 않는다.
	void RenderCharacterSelectFloorSwapControls();
	// F1의 명시적 쿠크 UI 미리보기는 HUD ViewModel의 preview override를 편집한다.
	// 광기·모드·슬롯 재배치·샘플 쿨다운을 조절하며 Server의 실제 기믹 상태와 구분한다.
	void RenderKoukuUiPreviewControls();
	// 지원 목적지를 Lobby 또는 Character Select의 typed 입장 경로에 예약한다. true는 요청 수락이며 실제 목적지 도착을 뜻하지 않는다.
	bool_t RequestDebugLevelNavigation(LEVEL eTargetLevel);
	// Resources·Data의 허용 루트에서 도구별 검색 목록을 다시 만든다.
	void RefreshDebugResourceFiles();
	// 쿠크 아레나 관문 버튼은 보스 활성화·자기 플레이어 이동·HUD 대상 변경을 Server 경계로 요청한다.
	void RenderKoukuSaydonArenaControls();
	// 발탄 아레나 안에서 Stage_1/Stage_2 일반 몬스터 재소환을 Server에 요청한다.
	void RenderValtanArenaControls();
	// 발탄 아레나의 복제 상태를 표시하고 벽·지형 변경 버튼을 기존 Server 명령 경계로 전달한다.
	void RenderValtanArenaStateControls();
	// 목록 index를 검사한 뒤 해당 domain 도구를 연다. 지원하는 보스·직업 경로만 구체적 문서에 연결하고 임의 raw 파일을 편집본으로 취급하지 않는다.
	void OpenDebugResourceFile(size_t iFile);

	// 발탄 도구에서 저장된 Server 패턴 목록을 받아 ID·표시 목록을 교체한다. 조회 실패면 이전 목록을 유지하며 성공하면 가능한 이전 선택을 보존한다.
	void RefreshCompletePlayPatternOptions();
	// 발탄 패턴 선택·준비 취소·Complete Play 버튼과 Server 진행 상태를 표시한다. 목록 로드는 명시 요청에서 시작한다.
	void RenderCompletePlayControls();
	// 저장된 쿠크 패턴을 관문별로 표시한다. Complete Play는 기존 CKoukuSaydonBossTool의 Server 요청 경로를 사용한다.
	void RenderKoukuSaydonCompletePlayControls();
	// 활성 쿠크 아레나·command sink·대기 여부·지원 관문·저장 문서와 게시 revision 일치를 확인한다. 도구 생성·Reload는 할 수 있으나 START는 보내지 않는다.
	bool_t PrepareKoukuGateCompletePlay(std::string_view gateId, std::string& status);
	// 검증한 관문 요청을 자원 준비 상태로 보관한다. true여도 아직 START 전송 전이며 Update가 준비·revision을 다시 검사한 뒤 제출한다.
	bool_t StartKoukuGateCompletePlay(std::string_view gateId, std::string& status);
	// 호출자에게 연출 완료와 다음 흐름은 Server 소유라는 상태만 돌려준다. Client가 다음 관문을 직접 진행시키지 않는다.
	void FinishKoukuGateCompletePlay(std::string_view gateId, std::string& status);
	// 전송 전 준비는 폐기하고, 응답 대기 중이면 중지를 예약한다. 활성 실행 소유자는 STOP을 제출하며 응답과 presentation 정리를 구분한다.
	void CancelKoukuGateCompletePlay(const std::string& status);

	// ROOM_PLAYER_ARRIVAL의 slot·목적지를 검증해 시각순 cue를 만든다. 시작할 때 이전 cue를 비우므로 실패 시 이전 목록을 복원하지 않는다.
	bool BeginKoukuSequenceArrivals(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, uint32_t startClockMs, std::string& status);
	// preview 시각(ms)에 도달한 cue를 Server 이동 명령으로 한 번 제출한다. advance가 false여도 이미 보낸 요청의 timeout은 확인한다.
	bool SampleKoukuSequenceArrivals(uint32_t clockMs, bool advance, std::string& status);
	// PLACE_ROOM_PLAYER 응답을 요청 순번·월드·패턴·현재 연결 world generation에 대응시킨다. 거절은 다음 sample이 읽을 실패로 남긴다.
	bool ConsumeKoukuSequenceArrivalResult(const LostArk::Shared::S2C_DEBUG_WORLD_PLAYBACK_RESULT& result);
	// 제출했으나 Server 결과가 없는 cue를 조회한다. 아직 시작 시각 전인 cue는 대기로 세지 않는다.
	bool KoukuSequenceArrivalsPending() const;
	// cue·패턴·실패·연결 세대를 비운다. 이전 응답과 구분하기 위한 실행 epoch는 여기서 초기화하지 않는다.
	void ClearKoukuSequenceArrivals();
	// 저작 ROOM_PLAYER_ARRIVAL의 시각(ms)과 player slot, Server 요청 순번·응답 deadline을 함께 보관한다.
	struct KOUKU_SEQUENCE_ARRIVAL_CUE final
	{
		std::string occurrenceId;
		uint32_t startMs = 0u, playerSlot = 0u, requestSequence = 0u;
		float3_t position{};
		bool submitted = false, completed = false, moved = false;
		std::chrono::steady_clock::time_point deadline{};
	};
	// cue는 이 세션 소유다. 실행 epoch는 요청에 싣고, 응답은 요청 순번·월드·패턴·현재 연결 world generation으로 구분한다.
	std::vector<KOUKU_SEQUENCE_ARRIVAL_CUE> m_KoukuSequenceArrivalCues;
	std::string m_strKoukuSequenceArrivalPatternId, m_strKoukuSequenceArrivalFailure;
	std::string m_strKoukuSequenceArrivalCompletedPreview;
	uint32_t m_iKoukuSequenceArrivalRunEpoch = 0u;
	uint64_t m_iKoukuSequenceArrivalWorldGeneration = 0u;
	bool m_bKoukuCompletePreservesArrivalPosition = false;
	// 발탄·쿠크의 Server 확정 아레나 상태를 공용 패널에 표시하고 typed 명령 버튼을 제공한다.
	void RenderServerArenaActiveControls();
	// 전경 창의 F1 상승 edge로 Developer Tools를 토글한다. ImGui나 제품 UI가 텍스트를 입력 중이면 건너뛴다.
	void UpdateDebugToolShortcut();

	// Action·Sequence workbench의 월드 애니메이션 편집·저장·이력 callback을 World Object 소유자에 연결한다.
	// 문서 포인터·세대·revision이 바뀌면 draft 또는 현재 Level 문서에서 리소스 목록을 재구성한다.
	void RefreshWorldObjectResources();
	// 통합 연출 창만 미저장 Motion 편집을 미리 본다. 다른 소비 경로는 이미 읽은 저장 문서를 유지한다.
	const CWorldSequenceDocument* CompositionPreviewWorldSource() const;
	// Development 편집기의 열린 Area 또는 제품 Level descriptor의 Area를 반환한다. 대응 Area가 없으면 빈 문자열이다.
	std::string GetWorldLevelAreaId() const;
	// 선택 위치(m)와 반경을 현재 저작 카메라에 전달한다. 카메라 확보에 실패하면 false이며, preview 중지를 시도한 뒤 override 해제에 실패할 수도 있다.
	bool FocusWorldLevelPosition(const float3_t& position, float radius, std::string& status);
	// workbench의 token·draft·Area를 확인해 한 클릭의 GPU 월드 위치를 전달한다. 저장은 해당 도구가 담당한다.
	bool UpdateMapEffectPlacementInput();
	// Guide trigger 박스가 예약한 한 번의 표면 위치 선택을 처리한다. 취소·소유권 변경은 기존 위치를 유지하고 표면 miss는 다음 클릭을 기다린다.
	bool UpdateWorldLevelPlacementPickInput();
	// 예약한 live mesh 선택의 소유 도구·Level·focus를 검사한다. 성공은 stable 배치 선택만 바꾸고 TRS는 수정하지 않는다.
	bool UpdateWorldMeshInspectionInput();
	// 현재 Level의 저작 host가 있으면 World Scene 진입 버튼을 표시한다.
	void RenderWorldMeshInspection();
	// World Scene 창의 선택·focus·일회 picking을 중재한다. 새 picking을 예약할 때 다른 저작 소비자의 예약을 해제한다.
	void RenderWorldSceneTool();
	// 선택 위치와 1m 축을 화면에 투영한다. 저작 시작·종료 ms는 안내할 때 초로 환산한다.
	void RenderMapEffectPlacementMarker();
	// 활성 Area와 composition draft 세대를 통합 목록에 전달한다. Map Tool 준비 대기는 60초 안에서 재조회하고 완료·닫힘 때 해제한다.
	void UpdateWorldLevelTool();
	// runtime generation과 stable ID를 확인해 통합 목록의 요청을 기존 저작 소유자에게 연결한다.
	void RenderWorldLevelTool();

	// preview 소유자가 바뀌면 이전 workbench 재생을 멈추고 collider 저작 세대를 비운 뒤 새 소유자를 기록한다.
	void ClaimCompositionPreviewOwner(DEBUG_TOOL owner);
	// 현재 로컬 preview 소유자가 요청한 경우에만 정리한다. Server 시각으로 재생 중인 presentation은 이 경로로 멈추지 않는다.
	void StopCompositionPreview(DEBUG_TOOL owner);
	// Balance 도구 열기와 수리 창 표시 버튼을 제공한다. 수치 편집과 요청 처리는 각 기존 도구에 남긴다.
	void RenderBalanceTestLauncher();
	// F1 허브의 Level·카메라·도구 버튼을 기존 소유자의 명령에 연결한다.
	void RenderDeveloperTools();

	// 두 Area의 trigger·연출·보스 패턴 목록과 필터를 표시한다. 선택한 버튼만 실행 요청 경계로 전달한다.
	void RenderSequenceViewer();
	// 아레나 playback 응답과 편집기 준비를 확인한다. 5초 응답·60초 편집기 timeout 뒤 자동 Server 재전송은 하지 않는다.
	void UpdateSequenceViewer();
	// 최초 열기·Refresh에서 종류별 목록을 다시 읽는다. 특정 종류 읽기 실패는 그 종류의 이전 행을 유지한다.
	void RefreshSequenceViewer();
	// 선택한 stable 행의 버튼을 Test 편집기 preview 또는 아레나의 Server 명령으로 연결한다. 전송 수락과 실제 실행 결과를 구분한다.
	void ExecuteSequenceViewerAction(int action);
	// 목록용 stable ID·표시 이름·관련 실행 대상을 소유한다. vector index를 Server identity로 보내지 않는다.
	struct SEQUENCE_VIEWER_ROW
	{
		int kind = 0; // trigger, 월드 시퀀스, 보스 패턴의 종류 값이다.
		std::string id, name, location, action, sequenceId, related, error;
		float3_t position{};
		bool enabled = true, hasPosition = false;
	};
	std::array<std::vector<SEQUENCE_VIEWER_ROW>, 2> m_SequenceViewerRows;
	std::array<std::string, 2> m_SequenceViewerLoadStatus;
	std::string m_SequenceViewerSelection;
	std::string m_SequenceViewerStatus;
	std::array<char, 256> m_SequenceViewerSearch{};
	bool m_bSequenceViewerLoaded = false;
	int m_iSequenceViewerArea = 0, m_iSequenceViewerKind = 0;
	std::uint32_t m_iSequenceViewerRequest = 0;
	std::uint32_t m_iSequenceViewerAwaitingRequest = 0;
	std::chrono::steady_clock::time_point m_SequenceViewerReplyDeadline{};
	int m_iSequenceViewerPendingEditorAction = -1;
	std::chrono::steady_clock::time_point m_SequenceViewerPendingDeadline{};

	// Debug의 rendering profile·품질·광원 편집 패널을 구성한다. 저장과 세션 비교 측정은 각 서비스의 명시 경계로 전달한다.
	void RenderRenderingWorkbench();
	// Release에서도 사용하는 최적화 비교 창이다. 현재 profile의 세션 측정 UI를 연결하고 닫히면 공용 도구 표시 상태를 맞춘다.
	void RenderOptimizationBenchmarkPanel();
	// 지도 광원·장면 프로필·재사용 광원 목록을 표시한다. 선택과 생성·저장·게시·재로드 버튼을 구분한다.
	void RenderLightingWorkbench();
	// stable 선택 ID가 바뀌면 이전 preview를 멈추고 상세 패널 draft를 새 대상에 맞춘다.
	void SelectRenderingLight(const string& id);
	// 선택 prefix에 따라 지도 행·프로필·재사용 정의의 상세 입력을 나눈다. Save와 Publish는 별도 버튼으로 실행한다.
	void RenderLightDetail();
	// 기존 profile draft와 Update_Profile 경계에 상세 입력을 전달한다. 저장·게시 자체는 상위 패널이 담당한다.
	void RenderSceneProfileDetail();
	// 선택 광원의 재생·일시정지·seek를 조절한다. ms 길이는 세션 preview 상태이며 저작 문서의 수명을 저장하지 않는다.
	void RenderLightSequencer();
	// preview를 먼저 멈추고 게시된 Area 광원을 재조회한다. 후속 실패가 앞선 정지나 목록 갱신까지 되돌리지는 않는다.
	bool ReloadPublishedMapLights();
	// 이전 preview를 정지한 뒤 선택 종류의 anchor·복사 문서·profile을 준비한다. 실패해도 이전 재생을 복원하지 않는다.
	bool StartLightingPreview();
	// 지도 문서 복사본에서 선택 광원을 켜고 나머지 행의 enabled는 보존한다. 검증 후 별도 preview runtime에 전달한다.
	bool StageMapLightPreview();
	// 같은 Level에서 자신이 활성화한 profile을 계속 소유할 때 이전 profile 복원을 시도한다. 복원 결과와 별개로 preview 상태는 정리한다.
	void StopLightingPreview();
	// Debug Update가 preview 시각과 프레임 광원을 갱신한다. 창 닫힘·Level 변경은 정지하고 anchor 실패는 해당 프레임 제출을 건너뛴다.
	void UpdateLightingPreview();
	// 표시가 켜져 있고 Profiler가 있을 때 live FPS·CPU·유효한 GPU 측정값을 입력 없는 overlay로 그린다.
	void RenderProfilerOverlay();
	// 기록 초기화와 JSON 저장 버튼을 제공한다. 저장은 공용 ProfilerTool의 비동기 경로로 요청하고 진행 상태를 표시한다.
	void RenderProfilerSettings();

private:
	// Engine이 만든 장치/context를 UI·도구 생성에 공유한다. Engine manager를 이 멤버가 소유하는 것은 아니다.
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	// 프로필 목록과 활성화 상태는 Client service가 보관하고 실제 렌더 설정은 Engine에 적용한다.
	CRenderingProfileService m_RenderingProfiles;
	// 캐릭터 생성의 어두운 무대 프로필을 활성화하기 전 ID다. 열기/닫기 전환에서만 프로필을 바꾼다.
	string m_strSceneProfileBeforeCustomizing;
	bool_t m_bClassShowcaseShadowFocusActive = false;
	float4_t m_vShadowFocusBeforeClassShowcase{};
	float4_t m_vClassShowcaseShadowFocus{};
	string m_strKoukuGateProfileRequest;
	string m_strKoukuGateProfileApplied;
	string m_strSceneProfileBeforeKoukuGate;
	CLightResourceCatalog m_LightResources;
	unique_ptr<CKoukuSaydonPresentationPlayer> m_pKoukuPresentationPlayer;
	unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
	unique_ptr<CProfilerTool> m_pProfilerTool;
	bool_t m_bRuntimeProfilerVisible = false;
	bool_t m_bF7Down = false;
	// MainApp이 layout을 보관하고 실제 CUI_Sprite는 Engine의 STATIC Layer에도 등록한다. 먼저 생성해 다른 창 아래에 그린다.
	unique_ptr<CUILayoutRuntime> m_pHUDRuntimeView = { nullptr };
	// HUD 다음에 생성하는 전투 분석 창이다. ViewModel 통계를 읽어 다른 runtime 창 아래에 표시한다.
	unique_ptr<CCombatAnalysisFrameView> m_pCombatAnalysisView = { nullptr };
	// KoukuHudModes.json 한 모드의 문양과 Q..F 아이콘 목록이다. 기믹 순서가 있으면 재배치하며 스킬 없는 슬롯은 비운다.
	struct KOUKU_HUD_MODE_SKILL
	{
		string strDisplayName;
		string strIconPath;
	};
	struct KOUKU_HUD_MODE_DEF
	{
		string strId;
		string strEmblemSlot;
		bool_t bRandomOrder = false;
		vector<KOUKU_HUD_MODE_SKILL> Skills;
	};
	vector<KOUKU_HUD_MODE_DEF> m_KoukuHudModes;
	struct HUD_SKILL_MARK
	{
		uint32_t iSkillId = 0u;
		string strMark;
	};
	vector<HUD_SKILL_MARK> m_HudSkillMarks;
	vector<pair<string, string>> m_HudSkillMarkAssets;
	struct HUD_BUFF_SOURCE
	{
		string strSource;
		string strStance;
		bool_t bDebuff = false;
		string strIconAsset;
	};
	vector<HUD_BUFF_SOURCE> m_HudBuffSources;
	// SkillBuff ID를 HudBuffIcons.json의 이미지에 대응시킨다. snapshot에만 있고 정의가 없는 buff는 슬롯을 그리지 않는다.
	unordered_map<uint32_t, string> m_HudSkillBuffIcons;
	// HUD 갱신이 그 프레임의 쿨다운·buff 남은 시간 글자를 예약한다. RenderSkillCooldownText가 EndFrame 뒤 소비한다.
	struct HUD_TIMED_TEXT
	{
		string strSlotId;
		uint32_t iEndTick = 0u;
		bool_t bDebuff = false;
		bool_t bUnderSlot = false;
	};
	vector<HUD_TIMED_TEXT> m_HudTimedTexts;
	bool_t m_bHudSpecialSlotShown = false;
	// 로컬 플레이어가 배에 탔을 때 해양 HUD가 숨긴 HP/마나·아이템·class 특수 슬롯의 글자도 숨기는 표시 상태다.
	bool_t m_bShipHudActive = false;
	// Update_VehicleHud가 채우는 보급량/최대 보급량과 노트 단위 선박 속도다.
	uint32_t m_iShipSupply = 0u;
	uint32_t m_iShipSupplyMax = 0u;
	f32_t m_fShipKnots = 0.f;
#ifdef _DEBUG
	unique_ptr<CLevelNavigationDebug> m_pLevelNavigationDebug;
	// 아직 적용하지 않은 UI draft다. 실제 설치된 바닥은 현재 Level이 소유한다.
	std::string m_strCharacterSelectFloorDraftId;
	float3_t m_vCharacterSelectFloorOffsetMeters{};
	f32_t m_fCharacterSelectFloorYawDegrees = 0.f;
	int32_t m_iCharacterSelectFloorEnvironment = 1;
	bool_t m_bCharacterSelectFloorShaderDefaultBrightness = false;
	bool_t m_bKoukuUiPreview = false;
	HUD_KOUKU_GIMMICK_STATE m_KoukuUiPreview;
	// 명시적 저작 미리보기 시간이다. 제품의 제한 시간은 Server deadline을 읽는다.
	bool_t m_bDungeonTimerPreview = false;
	f32_t m_fDungeonTimerStartSeconds = 90.f;
	f32_t m_fDungeonTimerWarningSeconds = 10.f;
#endif
	// BossUI.json의 layout을 보관한다. 보스 HP는 class별 전투 HUD와 따로 갱신한다.
	unique_ptr<CUILayoutRuntime> m_pBossUIView = { nullptr };
	// MainApp이 보관하는 쿠크 미니게임 제한 시간 UI다. Server deadline을 읽어 화면에 고정 표시한다.
	unique_ptr<CDungeonTimerView> m_pDungeonTimerView;
	unique_ptr<CWorldHealthBarView> m_pWorldHealthBarView = { nullptr };
	// X/Y 쌍 순서는 기믹·아군·일반 몬스터·쿠크세이튼·쿠크·발탄이다.
	std::array<f32_t, 12> m_HealthBarOffsets{}, m_SavedHealthBarOffsets{};
	// 기믹 줄에 적용할 너비/높이 배율이다. 기준 rect를 직접 덮어쓰지 않는다.
	std::array<f32_t, 2> m_MechanicBarScale{ 1.f / 3.f, 1.f };
	std::array<f32_t, 2> m_SavedMechanicBarScale{ 1.f / 3.f, 1.f };
	// 그룹마다 머리 기준 X/Y offset과 너비/높이 배율을 둔다. 발탄 무력화와 갑옷 PNG는 각각 조절한다.
	std::array<f32_t, 8> m_ValtanMechanicTuning{ 0.f, 0.f, 1.f / 3.f, 1.f, -24.f, -40.f, 1.f, 1.f };
	std::array<f32_t, 8> m_SavedValtanMechanicTuning{ 0.f, 0.f, 1.f / 3.f, 1.f, -24.f, -40.f, 1.f, 1.f };
	bool_t m_bValtanStaggerDebug = false;
	bool_t m_bValtanArmorBreakDebug = false;
	std::array<float4_t, 3> m_MechanicBarBaseRects{};
	std::array<bool_t, 3> m_MechanicBarHasBase{};
	// 최종 카메라 갱신 뒤 월드 HP bar와 같은 실제 머리 anchor에서 읽은 화면 위치다.
	float2_t m_MechanicBarHeadAnchor{};
	bool_t m_bMechanicBarHasHeadAnchor = false;
	bool_t m_bMechanicBarUIAllowed = false;
	std::string m_strHealthBarPositionStatus;
	// 레이드 공통 에스더 gauge의 STATIC layout을 보관한다.
	unique_ptr<CUILayoutRuntime> m_pEstherUIView = { nullptr };
	// 에스더 전면 컷인 flipbook을 소유한다. gauge UI 뒤에 생성해 gauge와 전투 HUD 위에 그린다.
	unique_ptr<CEstherCutinPresentationService> m_pEstherCutinService = { nullptr };
	// 재련 layout과 gauge/결과 연출을 보관한다. 실제 강화 단계는 Server 응답을 읽으며 P 키는 캐릭터 정보 창이 사용한다.
	unique_ptr<CUILayoutRuntime> m_pItemUpgradeView = { nullptr };
	bool_t m_bItemUpgradePreviewVisible = false;
	// BuildItemUpgradeSlots의 현재 표시 목록에 대한 index다. 행 선택으로 바뀌며 열기·프레임 동기화 때 유효 범위로 제한한다.
	int32_t m_iItemUpgradeSelectedSlot = 0;
	// 성장 버튼이 시작한 0~100 표시값이다. gauge와 퍼센트 글자가 같은 값을 읽는다.
	int32_t m_iItemUpgradePreviousPercent = 0;
	// 0→100 채우기 진행 여부다. 완료 이펙트는 채우기 중 숨기고 100에서 표시하며 SmeltGlow는 대기 0%에서만 반복한다.
	bool_t m_bItemUpgradeGrowing = false;
	f64_t m_dItemUpgradeGrowStartSeconds = -1.0;
	// 채우기 완료 뒤 WingedRingGold·LevelUpMotion2Big의 alpha 0→1 시작 시각이다.
	// 음수는 fade 대기가 없음을 뜻하며 새 클릭마다 초기화한다.
	f64_t m_dItemUpgradeCompleteRevealStartSeconds = -1.0;
	// 성장 클릭 때 CoreFlash를 한 번 시작할 예약이다. 프레임 갱신이 소비해 비우며 ShockwaveRing은 28프레임/20fps 뒤에 예약한다.
	bool_t m_bItemUpgradeCoreFlashPending = false;
	f64_t m_dItemUpgradeShockwaveScheduledAt = -1.0;
	// 재련 요청·응답의 화면 단계다. WAITING은 응답/공개 대기, SUCCESS/FAIL은 결과 modal이며 확인 전 새 시도를 막는다.
	enum class ITEM_UPGRADE_ATTEMPT_RESULT
	{
		NONE, WAITING, SUCCESS, FAIL
	};
	ITEM_UPGRADE_ATTEMPT_RESULT m_eItemUpgradeAttemptResult = ITEM_UPGRADE_ATTEMPT_RESULT::NONE;
	// 결과 연출만 위한 상태다. 실제 강화 단계는 Server 인벤토리 snapshot에서 읽는다.
	bool_t m_bItemUpgradePendingAttemptSuccess = false;
	bool_t m_bItemUpgradeResultUnavailable = false;
	uint32_t m_iNextItemUpgradeRequest = 1;
	uint32_t m_iPendingItemUpgradeRequest = 0;
	f64_t m_dItemUpgradeRequestDeadline = -1.0;
	string m_strItemUpgradeAttemptItemId;
	string m_strItemUpgradeAttemptIcon;
	wstring m_strItemUpgradeAttemptName;
	LostArk::Shared::EQUIPMENT_SLOT m_eItemUpgradeAttemptSlot = LostArk::Shared::EQUIPMENT_SLOT::NONE;
	uint16_t m_iItemUpgradeConfirmedLevel = 0;
	// SmeltSuccess/Fail 표시 후 120프레임/30fps에 맞춘 4초 대기의 만료 시각이다. 애니메이션 완료 통지를 받는 값은 아니다.
	// 그 뒤 대기 원과 burst를 숨겨 결과 문구만 남기며 음수는 종료 예약이 없음을 뜻한다.
	f64_t m_dItemUpgradeResultSettleAt = -1.0;
	// 로컬 플레이어 stance가 바뀔 때만 identity 아이콘 전환을 재생하기 위한 이전 값이다. NONE은 최초 관측도 전환으로 만들게 한다.
	LostArk::Shared::PLAYER_STANCE_ID m_ePreviousHudStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	// HOLD는 조기 해제로 stage가 짧아질 수 있어 관측한 combo stage·시작 tick으로 실제 누적 시간을 계산한다.
	// 현재 stage의 시작 tick과 앞서 완료한 stage의 ms를 별도 보관하며 INVALID_SKILL_ID는 추적하지 않는 상태다.
	// 단축·건너뛴 stage를 관측하면 같은 스킬 사용 동안 취소를 유지해 충전 gauge를 숨긴다.
	LostArk::Shared::SKILL_ID m_iChargeGaugeTrackedSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
	std::uint8_t m_iChargeGaugeTrackedComboStage = 0;
	std::uint32_t m_iChargeGaugeStageStartTick = 0;
	f32_t m_fChargeGaugeElapsedBeforeCurrentStageMs = 0.f;
	bool_t m_bChargeGaugeCancelled = false;
	// Q/W/E/R/A/S/D/F/T/V 순서의 준비 완료 여부다. ready→not-ready 전환만 사용 이펙트를 시작한다.
	// 준비된 스킬의 cooldown end tick은 매번 현재 tick일 수 있으므로 raw tick 증가로 판정하지 않는다.
	bool_t m_bPreviousQuickSlotReady[10] =
		{ true, true, true, true, true, true, true, true, true, true };
	// 창술사 identity 3구간 각각이 100에 도달하는 전환을 감지해 점화 이펙트를 한 번만 시작한다.
	bool_t m_bLanceGaugeSegmentWasFull[3] = { false, false, false };
	// ImGui::GetTime 기준 구간별 점화 시작 초다. 단발 점화가 끝난 후 유지 loop로 바꾸며 음수는 비활성이다.
	f64_t m_dLanceGaugeIgniteStartSeconds[3] = { -1.0, -1.0, -1.0 };
	bool_t m_bLanceGaugeLoopStarted[3] = { false, false, false };
	// ViewModel의 Server 피격 정보에 Client 표시 시각·애니메이션 상태를 더한 숫자 표시 기록이다.
	struct FLOATING_DAMAGE_NUMBER
	{
		LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::PLAYER_ID iSourcePlayerId = LostArk::Shared::INVALID_PLAYER_ID;
		f64_t dSpawnSeconds = 0.0;
		float3_t vWorldPosition = {};
		uint32_t iAmount = 0;
		bool_t isOutgoing = false;
		// 0은 피해량, 1은 카운터, 2는 무력화 성공이다.
		uint8_t iMechanicSuccess = 0u;
		// NONE이면 iAmount를 피해량으로, 문양이 있으면 카드 미로 사냥꾼의 조각 수로 표시한다.
		LostArk::Shared::MECHANIC_CARD_SYMBOL eCardMazeSuit =
			LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
		// 피격 종류에 대응하는 숫자 색과 재생 구간을 고른다.
		LostArk::Shared::DAMAGE_HIT_FLAG eHitFlag =
			LostArk::Shared::DAMAGE_HIT_FLAG::NORMAL;
		// 같은 대상의 여러 숫자가 겹치지 않도록 anchor에서 벌리는 기준 pixel 거리다.
		// 원본 영상에서 추적한 수치가 아닌 프로젝트 표시값이다.
		f32_t fScatterX = 0.f;
		f32_t fScatterY = 0.f;
	};
	vector<FLOATING_DAMAGE_NUMBER> m_FloatingDamageNumbers;
	// 보스 줄 전환과 피격을 감지하는 이전 상태다. 줄 감소 시 흰 fill과 교차 fade하고 HP 감소 시 fill 끝에 광택을 준다.
	// 원본 mark의 확장·감쇠 움직임은 절차적 표시로 근사하며 별도 원본 이미지 추출 성공을 의미하지 않는다.
	string m_strPreviousBossArchetypeId;
	uint32_t m_iPreviousBossBarsRemaining = 0u;
	f64_t m_dBossBarTickFlashStartSeconds = -1.0;
	uint32_t m_iPreviousBossCurrentHp = 0u;
	uint32_t m_iPreviousBossMaximumHp = 0u;
	f64_t m_dBossHitGlowStartSeconds = -1.0;
	f32_t m_fBossHitGlowFillRatio = 0.f;
	// rolling damage buffer에서 이미 띄운 snapshot batch를 구분하는 Server tick 커서다.
	// 동일 snapshot의 이벤트들은 같은 tick으로 묶이므로 처리한 이전 batch가 buffer에서 잘려도 중복 표시하지 않는다.
	uint32_t m_iLastRenderedDamageServerTick = 0u;
	// Lobby_Layout.json의 반복 배경과 버튼을 소유한다. STATIC CUI_Sprite라 엔진 경로로 그려지고 Debug ImGui보다 아래에 있다.
	unique_ptr<CUILayoutRuntime> m_pLobbyBackgroundView = { nullptr };
	struct LOBBY_SERVER_ENTRY
	{
		wstring		strName;
		wstring		strState;
		wstring		strTag;
		uint32_t	iCharacterCount = 0;
		// 이 행의 인원 수를 CCharacterRoster의 현재 process-session 캐릭터 수로 표시할지 결정한다. 계정·디스크 저장 캐릭터 수가 아니다.
		bool_t		bCountsSavedCharacters = false;
		bool_t		bCreatable = true;
	};
	vector<LOBBY_SERVER_ENTRY>	m_LobbyServers;
	int32_t						m_iLobbySelectedServer = -1;
	// 로비 밖에서는 false다. 다음 로비 최초 프레임에 로고 도입부를 다시 시작한다.
	bool_t						m_bLobbyWasActive = false;
	f32_t						m_fLobbyLogoIntroElapsed = 0.f;
	// 로비의 "게임 시작" 버튼으로 여는 창을 보관한다. STATIC sprite를 갱신하고 선택 의도는 MainApp이 소비한다.
	unique_ptr<CCharacterSelectWindowView> m_pCharacterSelectWindowView = { nullptr };
	// 현재 Level의 복제 marker로 갱신하는 STATIC 미니맵 view를 보관한다.
	unique_ptr<CMinimapView> m_pMinimapView = { nullptr };
	// 현재 초기화에서 생성하지 않는 기존 스킬 창 멤버다. K 단축키도 연결하지 않는다.
	unique_ptr<CSkillWindowView> m_pSkillWindowView = { nullptr };
	// Release에서도 I 키로 여는 인벤토리다.
	unique_ptr<CInventoryView> m_pInventoryView = { nullptr };
	// 베른 수리 NPC가 여는 수리 창이다. 독립 토글 키 없이 Open/Close와 Escape 경로를 사용한다.
	unique_ptr<CRepairWindowView> m_pRepairWindowView = { nullptr };
	// NPC별 상점 view를 보관한다. MainApp이 구매 의도를 받아 CNetworkManager::Send_BuyItems로 보낸다.
	unique_ptr<CShopWindowView> m_pShopWindowView = { nullptr };
	// 미니맵 아래 내구도 HUD다. 창이 아니므로 별도의 열림 상태나 Escape 순서를 갖지 않는다.
	unique_ptr<CDurabilityHudView> m_pDurabilityHudView = { nullptr };
	bool_t m_bIDown = false;
	// Release에서도 P 키로 여는 캐릭터 정보 창이다. 초상화는 Render의 월드 pass 전에 준비한다.
	unique_ptr<CCharacterInfoWindowView> m_pCharacterInfoView = { nullptr };
	// 캐릭터 정보 창이 여는 아바타 view다. 장착 상태는 Server 복제값을 읽는다.
	unique_ptr<CAvatarBookWindowView> m_pAvatarBookView = { nullptr };
	// 베른에 콜로세움 LOAD 요청이 남아 있는 Render에서 로컬 캐릭터를 그리는 초상화다. 요청 소비가 늦어지면 반복해서 그릴 수 있다.
	// 전환 서비스가 결과 SRV의 COM 참조를 보관하므로 이후 Loading에서도 이미지 수명을 유지한다.
	unique_ptr<CCharacterPortraitRenderer> m_pTransferPortrait = { nullptr };
	void Render_ColosseumTransferPortrait();
	bool_t m_bCharacterInfoKeyDown = false;
	// Release에서도 N 키로 여는 탈것 창이다. 아바타 창 뒤에 생성해 위에 표시하며 Alt+V는 기존 스킬 슬롯으로 유지한다.
	unique_ptr<CVehicleWindowView> m_pVehicleWindowView = { nullptr };
	bool_t m_bVehicleWindowKeyDown = false;
	// 캐릭터 정보의 칭호 변경 버튼이 열며 먼저 생성된 창들 위에 표시한다.
	unique_ptr<CHonorTitleWindowView> m_pHonorTitleWindowView = { nullptr };
	// M 키로 여는 구역 단계의 월드 지도 창이다.
	unique_ptr<CWorldMapWindowView> m_pWorldMapWindowView = { nullptr };
	// 네모난 구멍의 노래를 연주하는 동안 표시하는 주황색 시전 gauge다.
	unique_ptr<CSongCastGaugeView> m_pSongCastGaugeView = { nullptr };
	bool_t m_bWorldMapKeyDown = false;
	// 다른 runtime 창이 없을 때 Escape로 여는 환경설정이다. 기존 오디오 버스와 지원되는 영상 옵션을 편집한다.
	unique_ptr<CSystemOptionWindowView> m_pSystemOptionView = { nullptr };
	bool_t m_bSystemOptionKeyDown = false;
	// 캐릭터 선택 복귀가 예약되면 도착한 로비에서 선택 창을 열도록 남기는 상태다.
	bool_t m_bOpenCharacterSelectOnLobby = false;
	bool_t m_bCharacterReturnPending = false;
	f64_t m_dCharacterCaptureStartedAt = 0.0;
	uint64_t m_iCharacterCaptureWorldGeneration = 0u;
	// Loading 최초 진입에서 창 정리를 한 번 수행하기 위한 이전 Level 상태다.
	bool_t m_bWasLoadingLevel = false;
	// 열린 창을 오래된 순서로 보관한다. 유효 구간은 [0, m_iEscapeWindowCount)다.
	ESCAPE_WINDOW m_EscapeWindowOrder[ETOUI(ESCAPE_WINDOW::END)] = {};
	uint32_t m_iEscapeWindowCount = 0u;
	// 프레임 대기와 FPS 표시를 위한 평활값·직전 피격 시각·프레임 종료 시각이다.
	f32_t m_fSmoothedFps = 0.f;
	f64_t m_dLastDamageSeconds = -1000.0;   // FPS의 전투 중 판정에 사용하는 최근 피격 시각이다.
	std::chrono::steady_clock::time_point m_LastFrameEnd{};
	// 모든 runtime 창 뒤에 생성해 위에 표시하는 운반 아이콘이다.
	// 아이템은 1..4, 탈것은 5..0에 연결하며 등록은 Client 표시 상태다.
	unique_ptr<CQuickSlotDragView> m_pQuickSlotDragView = { nullptr };
	uint32_t m_iSpecialQuickSlotVehicle[6] = {};
	bool_t m_bSpecialKeyDown[6] = {};
	// 각 Item_1..4의 item ID 연결은 Client 로컬 상태다. Server에는 인벤토리 item ID로 사용을 요청한다.
	// 이 연결 자체를 별도 quick slot 프로토콜이나 영구 저장 데이터로 보내지 않는다.
	string m_strItemQuickSlot[4];
	bool_t m_bItemKeyDown[4] = {};
	uint32_t m_iNextUseItemSequence = 1u;
	// Release에서도 Enter로 여는 채팅 입력 창이다.
	unique_ptr<CChatWindowView> m_pChatWindowView = { nullptr };
	bool_t m_bEnterDown = false;
	bool_t m_bEscapeDown = false;
	// Release에서도 표시하는 파티 창이다. 현재 Level의 복제된 party roster·체력 view를 받아 표시한다.
	unique_ptr<CPartyWindowView> m_pPartyWindowView = { nullptr };

	// MainApp이 도구를 보관한다. 닫을 때 draft는 유지하고 Level 전환 때 차용 Level과 preview 연결을 정리한다.
	unique_ptr<CMapTool> m_pMapTool = { nullptr };
	unique_ptr<CEffect_Tool> m_pEffectTool = { nullptr };
	unique_ptr<CEffect_Tool_V2> m_pEffectToolV2 = { nullptr };
	unique_ptr<CAnimation_Tool> m_pAnimationTool = { nullptr };
	unique_ptr<CValtanActionWorkbench> m_pValtanActionWorkbench =
		{ nullptr };
	unique_ptr<CKoukuSaydonActionWorkbench> m_pKoukuSaydonActionWorkbench =
		{ nullptr };
	// Animation·Effect·Character 도구가 같은 preview panel을 공유한다. Level 전환/종료 때 MainApp이 공용 preview를 정리한다.
	shared_ptr<CCharacterPreviewPanel> m_pCharacterPreviewPanel = { nullptr };
	unique_ptr<CEquipmentAuthoringTool> m_pEquipmentAuthoringTool = { nullptr };
	unique_ptr<CHUDLayoutTool> m_pHUDLayoutTool = { nullptr };
	unique_ptr<CBalanceTool> m_pBalanceTool = { nullptr };
	unique_ptr<CValtanBossTool> m_pValtanBossTool = { nullptr };
	unique_ptr<CKoukuSaydonBossTool> m_pKoukuSaydonBossTool = { nullptr };
	unique_ptr<CCameraTool> m_pCameraTool = { nullptr };
#ifdef _DEBUG
	unique_ptr<CWorldObjectTool> m_pWorldObjectTool;
	unique_ptr<CWorldLevelTool> m_pWorldLevelTool;
	unique_ptr<CWorldSceneTool> m_pWorldSceneTool;
	bool m_bWorldMeshPickArmed = false, m_bWorldMeshPickLeftDown = true;
	bool m_bWorldMeshPickSuppressMouse = false;
	uint32_t m_iWorldMeshPickLevel = UINT32_MAX;
	unique_ptr<CGuideAITool> m_pGuideAITool;
	unique_ptr<CMaharakaAITool> m_pMaharakaAITool;
	unique_ptr<WORLD_LEVEL_TOOL_REQUEST> m_pWorldLevelPendingMapRequest;
#endif
	std::chrono::steady_clock::time_point m_WorldLevelMapDeadline{};
	// World Level 배치와 Guide trigger가 공유하는 단일 viewport pick 예약이다.
	bool m_bWorldLevelPickArmed = false;
	DEBUG_TOOL m_eWorldLevelPickOwner = DEBUG_TOOL::WORLD_LEVEL;
	uint32_t m_iWorldLevelPickLevel = UINT32_MAX;
	bool m_bWorldLevelPickLeftDown = true;
	bool m_bWorldLevelPickSuppressMouse = false;
	unique_ptr<KOUKU_MAP_EFFECT_PLACEMENT_REQUEST> m_pMapEffectPlacementRequest;
	DEBUG_TOOL m_eMapEffectPlacementOwner = DEBUG_TOOL::NONE;
	uint32_t m_iMapEffectPlacementLevel = UINT32_MAX;
	bool m_bMapEffectPlacementLeftDown = true;
	bool m_bMapEffectPlacementSuppressMouse = false;
	std::string m_strMapEffectPlacementStatus;
	// 현재 선택한 문서를 차용하는 cache다. generation/revision과 함께 검사하고 Level 전환에서 차용 포인터를 비운다.
	const CWorldSequenceDocument* m_pWorldObjectCatalogSource = nullptr;
	uint64_t m_iWorldObjectCatalogGeneration = UINT64_MAX;
	uint32_t m_iWorldObjectCatalogRevision = 0;
	unique_ptr<CSequencerTool> m_pSequencerTool = { nullptr };
	unique_ptr<CKoukuSaydonActionWorkbench> m_pSequenceActionWorkbench;
	unique_ptr<CCharacterActionWorkbench> m_pCharacterActionWorkbench;
	// 공용 미리보기 시계는 UI focus와 무관하게 시작한 세션이 소유한다.
	DEBUG_TOOL m_eCompositionPreviewOwner = DEBUG_TOOL::NONE;
	DEBUG_TOOL m_eColliderAuthoringOwner = DEBUG_TOOL::NONE;
	uint64_t m_iColliderAuthoringDraftGeneration = UINT64_MAX;
	uint64_t m_iColliderAuthoringSerial = 0;
	unique_ptr<CRenderingBenchmark> m_pRenderingBenchmark = { nullptr };
	bool_t m_bF1Down = false;
	bool_t m_bDeveloperToolsVisible = false;
	bool_t m_bProfilerVisible = false;
	bool_t m_bRenderQualityDraftInitialized = false;
	array<bool_t, static_cast<size_t>(DEBUG_TOOL::COUNT)>
		m_DebugToolVisible = {};
	// 여러 창을 표시해도 월드 viewport 입력은 한 도구에만 준다. 창 표시와 입력 focus를 따로 추적한다.
	DEBUG_TOOL m_eDebugInputOwner = DEBUG_TOOL::NONE;
	DEBUG_TOOL m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
	LEVEL m_eDebugLevelNavigationTarget = LEVEL::END;
	std::chrono::steady_clock::time_point m_DebugLevelNavigationDeadline{};
	bool_t m_bDebugLevelNavigationDeadlineActive = false;
	string m_strDebugLevelNavigationStatus =
		"Choose a destination. Gameplay Areas always use the existing Server-approved entry route.";
	vector<DEBUG_RESOURCE_FILE> m_DebugResourceFiles;
	array<char_t, 192> m_DebugResourceSearch = {};
	bool_t m_bDebugResourceScanAttempted = false;
	size_t m_iSelectedDebugResourceFile =
		static_cast<size_t>(-1);
	string m_strDebugResourceStatus =
		"Resource Files has not been indexed yet.";
	string m_strServerArenaActiveStatus =
		"Enter the Server-approved Valtan Arena to inspect Active state.";
	bool_t m_bServerArenaPresetStatusTracking = false;
	uint32_t m_iNextKoukuStageTeleportRequestSequence = 1u;
	string m_strKoukuStageTeleportStatus =
		"Enter the Server-approved KoukuSaydon Arena to inspect SL01-SL05 stages.";
	// 쿠크 Complete Play 목록은 Boss Tool에서 읽고 관문 combo는 목록만 거른다. 실제 선택은 stable Pattern ID다.
	bool_t m_bKoukuCompletePlayLoadAttempted = false;
	bool_t m_bKoukuBingoHammerColliders = false;
	int32_t m_iKoukuCompletePlayGate = 0;
	int32_t m_iKoukuCompletePlayCategory = 0;
	uint32_t m_iKoukuCompletePlayFlowRevision = 0u;
	bool_t m_bKoukuCompletePlayAwaitingGate = false;
	bool_t m_bKoukuFlowPublishPending = false;
	int32_t m_iKoukuCompletePlaySelection = 0;
	string m_strKoukuCompletePlayPatternId;

	vector<string> m_CompletePlayPatternIds;
	vector<string> m_CompletePlayPatternLabels;
	// 선택 정본은 stable ID다. UI index는 매 프레임 여기서 찾으며 저장 identity로 사용하지 않는다.
	string m_strCompletePlayPatternId;
	bool_t m_bCompletePlayPatternLoadAttempted = false;
	bool_t m_bCompletePlayStatusTracking = false;
	string m_strCompletePlayTrackedPatternId;
	string m_strCompletePlayStatus =
		"Select a saved semantic pattern, then submit Complete Play to the Server.";
	RENDER_QUALITY_SETTINGS m_RenderQualityDraft = {};
	SCENE_RENDERING_PROFILE m_SceneRenderingDraft = {};
	string m_strRenderingDraftProfileId;
	string m_strRenderingSelectedProfileId;
	string m_strRenderingQualityProfileId;
	LEVEL m_eRenderingSelectedLevel = LEVEL::END;
	uint32_t m_iRenderingLastLevel = UINT32_MAX;
	char m_szRenderingNewProfileId[129]{};
	char m_szRenderingNewProfileName[257]{};
	// 조명 저작 세션과 목록 선택은 저장 문서의 상태를 보관한다. 아래 preview 시계·문서 복사본과 구분한다.
	CAreaLightAuthoringSession m_AreaLightSession;
	string m_strRenderingLightAreaAttempt;
	string m_strSelectedRenderingLightId = "@default-directional";
	int m_iRenderingLightCategory = 0;
	int m_iRenderingLightUsageCategory = 0;
	int m_iRenderingLightCreateUsage = 0;
	int m_iRenderingLightCreateAnchor = 0;
	int m_iRenderingLightCreateType = 2;
	char m_szRenderingLightName[129] = "Spotlight";
	string m_strLightingStatus;
	// 미리보기 ID·이전 profile·Level을 함께 보관한다. Stop은 자신이 활성화한 profile을 여전히 소유할 때만 복원을 시도한다.
	string m_strLightPreviewResourceId;
	string m_strLightPreviewSelectionId;
	string m_strLightPreviewPreviousProfileId;
	string m_strLightPreviewSceneProfileId;
	shared_ptr<CMapLightPresentationRuntime> m_pLightSequencerMapPreview;
	std::chrono::steady_clock::time_point m_LightPreviewLastUpdate{};
	double m_fLightPreviewCursorMs = 0.0;
	int m_iLightPreviewLifetimeMs = 3000;
	bool_t m_bLightPreviewPaused = false;
	float4x4_t m_LightPreviewMapPivot{};
	uint32_t m_iLightPreviewLevel = UINT32_MAX;
	bool_t m_bLightingDebugWire = true;
	bool_t m_bLightResourcesWindowVisible = true;
	bool_t m_bLightDetailWindowVisible = true;
	bool_t m_bLightSequencerWindowVisible = true;
	bool_t m_bRenderingQualityWindowVisible = true;
	bool_t m_bResetLightingLayout = false;
	bool_t m_bPreviewMapLightDraft = true;
	shared_ptr<uint32_t> m_pLightPreviewSkipped = make_shared<uint32_t>(0u);
	string m_strToolStatus =
		"Select a tool. Map authoring targets the current level Area.";
	string m_strRenderingStatus =
		"Rendering profiles are loaded from the published runtime catalog.";
	string m_strProfilerCaptureStatus;
	// Debug 아이템 지급 목록의 catalog index와 로컬 요청 순번이다.
	// Server가 돌려준 전체 인벤토리 snapshot으로 표시를 갱신하며 이 패널이 결과를 로컬 생성하지 않는다.
	int32_t m_iSelectedDebugItemIndex = 0;
	uint32_t m_iNextDebugGiveItemSequence = 1;
	string m_strDebugItemStatus;

private:
	// Level/UI가 현재 MainApp에 접근하는 비소유 주소다. 실제 수명은 Client.cpp의 앱 소유자가 관리한다.
	static CMainApp* s_pActiveInstance;

public:
	// Client.cpp가 호출해 준비된 MainApp을 받는다. 초기화 실패는 nullptr로 알린다.
	static unique_ptr<CMainApp> Create();
	// 도구/네트워크를 정리하고 Engine을 종료한다. Loading worker 종료 뒤 공유 준비 cache를 해제한다.
	void Free();
};

NS_END
