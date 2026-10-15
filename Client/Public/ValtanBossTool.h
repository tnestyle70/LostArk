#pragma once

#include "Client_Defines.h"
//손 무기 소켓 변환을 draft 값으로 보관하므로 완전한 타입 정의가 필요하다.
#include "ActorCatalog.h"

#include "BossLogicFlowView.h"
#include "CameraTool.h"
#include "EncounterPatternReference.h"
#include "Network/PacketMessages.h"
#include "ValtanCinematicCameraDocument.h"
#include "ValtanPatternFlowDocument.h"
#include "ValtanPatternSoundCueDocument.h"
#include "ValtanPatternTree.h"
#include "ValtanViewAdmission.h"

#include <array>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class IPlayerCommandSink;
class CBalanceTool;
class CValtanPatternSoundSourceReadAdmission;
struct VALTAN_PATTERN_SOUND_SOURCE_RECEIPT;
struct EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST;
struct HUD_BOSS_STATE;

//MainApp이 소유하는 발탄 저작·검증 도구. graph와 UI draft를 보관하고 재생·아레나 변경은 기존 Server 서비스에 요청한다.
//손 무기 소켓 preview는 ActorCatalog에 전달하며, 패턴 실행·애니메이션·이펙트 생성 경로는 기존 런타임이 맡는다.
class CValtanBossTool final
{
public:
	struct VALTAN_ARENA_ACTIVE_STATE final
	{
		bool_t bSynchronized = false;
		bool_t bOrdinaryWallsActive = false;
		bool_t bOuterRingActive = false;
		bool_t bThreeOClockFloorActive = false;
		bool_t bNineOClockFloorActive = false;
		uint32_t iDebrisActorCount = 0u;
		uint32_t iActiveCollisionCount = 0u;
		uint32_t iActiveNavigationRegionCount = 0u;
		uint64_t iNavigationRevision = 0u;
	};
	struct SERVER_PATTERN_OPTION final
	{
		std::string strPatternId;
		std::string strDisplayName;
	};

	//MainApp의 생성·창 열기와 프레임 호출 경계.
	CValtanBossTool(
		std::shared_ptr<IPlayerCommandSink> CommandSink,
		CBalanceTool* pBalanceTool);
	void Open();
	[[nodiscard]] bool_t Is_Open() const noexcept
	{
		return m_bOpen;
	}
	//Action Workbench의 연결 요청을 받아 기존 Flow 문서가 있는 탭을 연다. 두 번째 문서나 재생 서비스를 만들지 않는다.
	void Open_PatternFlow();
	//같은 graph·HUD 관찰값·canvas 상태로 별도 Logic Pattern 창을 연다. 새 parser나 게임플레이 런타임은 만들지 않는다.
	void Open_LogicPattern();
	bool_t Consume_LogicPatternOpenRequest();
	[[nodiscard]] bool_t Is_LogicPatternOpen() const noexcept
	{
		return m_bLogicPatternOpen;
	}
	void Update(bool_t bValtanBossToolVisible, bool_t bLogicPatternVisible);
	void Render();

	//MainApp의 아레나 패널이 직접 호출한다. Open·graph 로드 없이 손 무기 소켓 draft를 편집한다.
	//본체·유령의 b_wp_r_01 부착 위치·각도이며, 낙하 도끼 패턴의 생성·낙하·충돌을 편집하는 창이 아니다.
	//preview, 정본 저장, 기존 BalanceTool 게시를 각각 사용자의 명시 동작으로 연결한다.
    void Render_AxeEditor();

	void Render_LogicPatternWindow();
	//Workbench가 전달한 stable Pattern ID를 현재 audition 목록에서 다시 확인하고 기존 typed 서비스로 재생을 요청한다.
	//선택한 단일 패턴의 Server 재생·재시작·다음 패턴 예약.
	bool_t Play_ServerPattern(
		const std::string& strPatternId,
		std::string& strOutStatus);
    bool_t Is_PlayPreparationPending() const { return m_PlayPreparation.has_value(); }
    void Cancel_PlayPreparation(const std::string& reason);
	//Workbench가 선택한 단일 패턴 occurrence의 재시작을 요청한다. 이전 occurrence가 맞는지는 Server가 판정한다.
	bool_t Restart_ServerPattern(
		const std::string& strPatternId,
		std::string& strOutStatus);
	//현재 Server occurrence 다음에 검증된 Pattern 하나를 예약한다. 아레나 전체를 초기화하지 않는다.
	bool_t Queue_NextServerPattern(
		const std::string& strPatternId,
		std::string& strOutStatus);
	//Complete Play 가능 여부를 Server 활성 revision과 검사한다. 파일 저장 자체를 별도 활성화 절차로 취급하지 않는다.
	//재생 가능 revision과 외부 저작 변경 허용 여부 조회.
	bool_t Can_Play_ServerPattern(std::string& strOutStatus) const;
	//현재 재생에 필요한 Server 활성 revision을 얻는다. 연출 준비자는 이 값과 소비자 reload 결과를 함께 고정한다.
	bool_t Get_ServerActivePatternRevision(
		LostArk::Shared::GameplayDataRevision& OutRevision,
		std::string& strOutStatus) const;
	//ImGui 활성화 표시용 메모리 관찰이다. 파일 hash·parse 없이 읽으며 실제 명령은 제출 직전에 더 강한 admission을 거친다.
	bool_t Observe_ServerActivePatternRevision(
		LostArk::Shared::GameplayDataRevision& OutRevision,
		std::string& strOutStatus) const;
	//Animation의 Pattern Sound 변경 허용 여부를 판정한다. 준비·audition·Restart·Next·Flow가 점유 중이면 Sound 세대 변경을 막는다.
	bool_t Can_CommitPatternSoundGeneration(
		std::string& strOutStatus) const;
	//요청한 Pattern과 이 도구의 consumer에 해당하는 준비·Server 판정 상태만 반환한다. 다른 소비자의 상태를 섞지 않는다.
	bool_t Get_ServerPatternStatus(
		const std::string& strPatternId,
		std::string& strOutStatus,
		bool_t& bOutInFlight) const;
	//정본 목록 재구성과 아레나 상태 명령·조회.
	bool_t Get_ServerPatternOptions(
		std::vector<SERVER_PATTERN_OPTION>& outOptions,
		std::string& strOutStatus);
	//외부 저작 저장 뒤 graph와 audition·Next 목록을 함께 다시 구성한다. 실패하면 이전 표시 자료를 보존하고 명령은 제한한다.
	bool_t Reload_CanonicalGraph(std::string& strOutStatus);
	bool_t Set_ServerArenaPreset(
		LostArk::Shared::VALTAN_ARENA_PRESET preset,
		std::string& strOutStatus);
	bool_t Get_ServerArenaActiveState(
		VALTAN_ARENA_ACTIVE_STATE& outState,
		std::string& strOutStatus) const;
	std::string Get_ServerArenaPresetStatus() const;
	bool_t Is_ServerArenaPresetPending() const;
	//하위 도구를 여는 요청을 MainApp이 한 번 소비.
	bool_t Consume_CameraToolOpenRequest(
		CAMERA_TOOL_OPEN_REQUEST& outRequest);
	bool_t Consume_EffectToolOpenRequest(
		EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& outRequest);

private:
    //손 무기 소켓의 로드 기준·미저장 preview를 본체와 유령별로 보관.
    struct AXE_DRAFT final
    {
        //최초 자동 로드를 시도했는지. 축 편집 여부가 아니며 실패 뒤 매 프레임 재로드하는 것을 막는다.
        bool_t attempted = false;
        //유효한 saved/value를 확보했는지. 실패한 최초 로드에서는 편집을 열지 않는다.
        bool_t loaded = false;
        //마지막 성공한 로드·저장의 기준값. Save가 최신 정본과 변경 축을 병합할 때 비교한다.
        BOSS_WEAPON_SOCKET_TRANSFORM saved;
        //현재 편집값. 최초 Load는 정본 값만 채우고 슬라이더·Reset 성공 때 preview에 반영한다.
        //Save·Reload의 preview 적용 성공까지 이 값만으로 보장하지는 않는다.
        BOSS_WEAPON_SOCKET_TRANSFORM value;
        //이 draft의 로드·저장·게시 결과. 오류가 생겨도 기존 유효 값을 지우지 않고 이유를 표시한다.
        std::string status;
    };

    //최초 열기·Reload가 검증한 정본을 draft에 반영한다. 실패하면 이전 유효 편집값을 보존한다.
    bool_t Load_AxeDraft(const char_t* archetype, AXE_DRAFT& draft);
    //편집한 축만 최신 정본에 병합하고 검증·백업·원자적 교체 뒤 saved/value를 함께 갱신한다.
    bool_t Save_AxeDraft(const char_t* archetype, AXE_DRAFT& draft);

	//정본 graph·파생 목록의 검증·교체와 변경 권한 판정.
	bool_t Reload_Graph();
	//publisher 잠금·세대 교체 같은 일시 충돌만 지연 재시도한다. 의미 검증 실패는 자동 반복하지 않는다.
	void Schedule_CanonicalReloadRetry();
	//최초 정본 결합 실패 때 검증된 Product를 읽기 전용 대체 표시로 사용한다. 저장·재생 권한은 부여하지 않는다.
	bool_t Fail_GraphReload(
		const std::string& strStrictFailure,
		CValtanCanonicalProductReadAdmission* pCanonicalAdmission);
	bool_t Can_MutateCanonicalGraph(std::string& strOutStatus) const;
	//클릭 직전 Server revision·연출 소비자·Sound receipt를 함께 고정한다. 준비 완료 경로만 자신의 준비 점유를 통과시킨다.
	//재생 제출 전 검증, 자원 준비와 단일 패턴 명령.
	bool_t Acquire_ServerPlaybackAdmission(
		LostArk::Shared::GameplayDataRevision& OutRevision,
		VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& OutSoundReceipt,
		CValtanPatternSoundSourceReadAdmission& SoundAdmission,
		std::string& strOutStatus, bool_t preparedOwner = false) const;
	bool_t Submit_SelectedPattern();
    void Update_PlayPreparation();
	//이 도구가 소유한 단일 패턴 occurrence만 재시작한다. Server는 아레나를 유지하고 보스만 초기화한다.
	bool_t Restart_SelectedPattern();
	//Flow의 단일 슬롯 시험과 전체 순서 재생.
	bool_t Preview_SelectedFlowSlotIsolated();
	bool_t Start_Flow(
		const LostArk::Shared::GameplayDataRevision*
			pRequiredDefinitionRevision = nullptr);
	bool_t Start_FlowAtSlot(
		const std::string& strStartSlotId,
		const LostArk::Shared::GameplayDataRevision*
			pRequiredDefinitionRevision = nullptr);
	//저장된 전체 scriptedSequence를 다시 읽고 아레나 초기화 후 첫 패턴부터 요청한다. 단일 패턴 Restart와 구분한다.
	bool_t Restart_SavedFlow();
	//플레이어 부활 요청과 Flow 저작 문서의 로드·저장·게시.
	bool_t Request_RevivePlayer(std::string& strOutStatus);
	bool_t Reload_FlowDocument();
	bool_t Save_FlowDocument();
	bool_t Retry_FlowProductPublishApply();
	//정본·연출 일치 여부와 HUD live 관찰값 갱신.
	void Refresh_PresentationFreshness(bool_t bForce = false);
	void Synchronize_LiveSelection();
	//Boss·Flow·읽기 전용 Logic Pattern 화면 구성.
	void Render_BossVerificationTab();
	void Render_PatternFlowTab();
	void Render_LogicFlowTab();
	void Render_LogicPatternContent();
	void Render_LogicPatternInspector(
		const VALTAN_PATTERN_VIEW& Pattern,
		const HUD_BOSS_STATE& Boss,
		bool_t bLivePattern);
	//정본 결합 실패 시 이전 Product 자료의 읽기 전용 표시.
	void Render_ProductFallbackPatternList();
	void Render_ProductFallbackSelectedPattern();
	void Render_ProductFallbackLogicPattern();
	//HUD action 전이를 관찰하고 Logic canvas용 graph로 투영.
	void Update_LogicFlowObservation();
	bool_t Project_LogicFlowView(const VALTAN_PATTERN_VIEW& Pattern);
	//Flow 노드·슬롯·연결 편집과 다음 패턴 선택 UI.
	void Render_FlowGraphEditor();
	void Render_FlowSlotList();
	void Render_FlowSelectedSlot();
	void Render_AddPatternPopup();
	bool_t Render_AddPatternNodePopup();
	void Render_NextPatternCard();
	void Render_NextPatternPicker();
	//현재 재생 요약·조작 버튼·패턴 목록·선택 상세 UI.
	void Render_LiveSummary();
	void Render_ActionBar();
	void Render_PatternList();
	void Render_CurrentPatternList();
	void Render_SelectedPattern();
	//선택한 패턴의 링 데이터 저작·저장.
	bool_t Render_SelectedPatternRingAuthoring(
		const VALTAN_PATTERN_VIEW& Pattern);
	bool_t Save_SelectedPatternRingAuthoring();
	//수동 선택 동기화와 Stage에 연결된 연출 정보 표시.
	void Select_Pattern(const VALTAN_PATTERN_VIEW& Pattern);
	void Normalize_CurrentFlowSelection();
	void Render_ConnectionSummary(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage);
	//도구가 보관한 자료를 stable ID로 조회. 반환 포인터는 해당 자료 reload 전까지만 차용.
	const VALTAN_PATTERN_VIEW* Find_Pattern(
		const std::string& strPatternId) const;
	const VALTAN_PATTERN_VIEW* Find_AuditionPattern(
		const std::string& strPatternId) const;
	const ENCOUNTER_PATTERN_REFERENCE* Find_ProductFallbackPattern(
		const std::string& strPatternId) const;
	const ENCOUNTER_STAGE_REFERENCE* Find_ProductFallbackStage(
		const ENCOUNTER_PATTERN_REFERENCE& Pattern,
		const std::string& strActionId) const;
	const VALTAN_STAGE_VIEW* Find_LiveStage(
		const VALTAN_PATTERN_VIEW& Pattern) const;
	const VALTAN_STAGE_VIEW* Find_SelectedStage(
		const VALTAN_PATTERN_VIEW& Pattern) const;
	const VALTAN_INDEPENDENT_EFFECT_VIEW* Find_IndependentEffect(
		const std::string& strIndependentEffectId) const;
	const VALTAN_CINEMATIC_CAMERA_CUE* Find_CameraCue(
		const std::string& strCueId) const;
	const VALTAN_PATTERN_FLOW_SLOT* Find_SelectedFlowSlot() const;
	const VALTAN_PATTERN_FLOW_NODE* Find_SelectedFlowNode() const;
	const VALTAN_PATTERN_FLOW_EDGE* Find_SelectedFlowEdge() const;
	std::vector<std::string> Build_AdmittedPatternIds() const;
private:
    //단일 패턴 제출 전 자원 준비의 고정 입력. Update가 완료·취소할 때까지 보관.
    struct PLAY_PREPARATION {
        //준비 시작 때 선택한 stable Pattern ID. 선택이 바뀌면 준비를 취소한다.
        std::string patternId;
        //준비 시작의 Server 활성 revision. 마지막 제출 전에도 같은 값이어야 한다.
        LostArk::Shared::GameplayDataRevision revision;
        //같은 재생에 묶은 Pattern Sound 세대 receipt.
        VALTAN_PATTERN_SOUND_SOURCE_RECEIPT sound;
        //접속·월드가 바뀐 뒤 이전 준비를 제출하지 않기 위한 inbound 세대.
        uint32_t worldGeneration=0;
        //GetTickCount64 기준 시작 시각(ms). 준비 제한 시간 판정에 사용한다.
        uint64_t startTick=0;
    };
    //값이 있으면 아직 단일 패턴 재생 명령 제출 전이다. 매 Update에서 진행하고 최종 Submit 전에 비운다.
    std::optional<PLAY_PREPARATION> m_PlayPreparation;
    //optional이 비워진 뒤에도 해당 Pattern의 취소·실패 결과를 Workbench에 반환하기 위한 마지막 표시값.
    std::string m_strPreparationPatternId, m_strPreparationStatus;
    bool_t m_bPreparationTerminal = false;
	//정본 검증을 마친 표시 snapshot과 파생 재생 목록. Reload_Graph 성공 시 함께 교체.
	VALTAN_PATTERN_TREE_VIEW m_Graph;
	VALTAN_TOOL_AUDITION_INVENTORY m_AuditionInventory;
	std::vector<std::string> m_NextPatternIds;
	//연출 연결을 조회하는 보조 문서와 정본 실패 시 읽기 전용 Product 대체 자료.
	CEncounterPatternReference m_EncounterReference;
	CEncounterPatternReference m_ProductFallbackEncounterReference;
	CValtanCinematicCameraDocument m_CameraDocument;
	//이 도구가 소유하는 Flow 저장 기준·편집 draft. 런타임 진행 상태는 FlowService가 별도로 소유.
	CValtanPatternFlowDocument m_FlowDocument;
	//Logic canvas의 투영 결과·확대/이동·선택·관찰 이력. 게임플레이 분기를 실행하는 상태가 아니다.
	BOSS_LOGIC_FLOW_VIEW m_LogicFlowView;
	BOSS_LOGIC_FLOW_CANVAS_STATE m_LogicFlowCanvasState;
	BOSS_LOGIC_FLOW_SELECTION m_LogicFlowSelection;
	CBossLogicFlowObservedEdgeResolver m_LogicFlowObservedEdges;
	//생성자로 받은 명령 sink는 공유 수명을 유지하며 부활 요청 등에 사용.
	std::shared_ptr<IPlayerCommandSink> m_pCommandSink;
	//MainApp 소유 BalanceTool을 차용해 게시 상태·저장 작업을 조회. 이 도구는 생성·delete하지 않는다.
	CBalanceTool* m_pBalanceTool = nullptr;

    //손 무기 소켓 편집 상태. MainApp이 소유한 도구 수명 동안 본체·유령 값을 유지하고 주 스레드 패널만 접근.
    //창 닫기·Level 이동으로 reset하지 않으며 소멸 시 저장·게시·preview 복원을 자동 수행하지 않는다.
    std::array<AXE_DRAFT, 2u> m_AxeDrafts{};
	//0은 BOSS_VALTAN, 1은 BOSS_VALTAN_GHOST를 고르는 UI 인덱스이며 저장 ID로 사용하지 않는다.
    int32_t m_iSelectedAxe = 0;

	//각 목록의 검색 입력 버퍼. UI 필터이며 저장·네트워크 identity가 아니다.
	std::array<char_t, 128u> m_PatternSearch{};
	std::array<char_t, 128u> m_FlowPatternSearch{};
	std::array<char_t, 128u> m_NextPatternSearch{};

	//사용자가 Inspector에서 선택한 stable Pattern/Stage ID. 선택은 재생 명령을 보내지 않는다.
	std::string m_strSelectedPatternId;
	std::string m_strSelectedStageId;

	//Server 관찰값은 위 수동 Inspector 선택값과 별개다. Follow Live는 live 행 표시·스크롤만 따르게 한다.
	//Synchronize_LiveSelection이 HUD의 Server Pattern ID를 audition 목록에서 확인해 복사. 미해석 시 빈 값.
	std::string m_strLivePatternId;
	//HUD actionId와 일치하는 현재 Pattern의 Stage를 찾아 얻은 stable ID. stage 배열 인덱스나 clip 이름이 아니다.
	//해석 실패는 빈 값이며 Inspector 선택을 바꾸지 않고 LIVE 강조·상태 표시에 사용한다.
	std::string m_strLiveStageId;
	//같은 live Pattern 행으로 매 프레임 강제 스크롤하지 않기 위한 마지막 자동 표시 ID.
	std::string m_strLastAutoRevealedLivePatternId;
	//Flow draft의 선택 노드/슬롯 ID. graph UI는 node ID, 목록 UI는 slot ID로 현재 문서에서 다시 조회.
	std::string m_strSelectedFlowSlotId;
	//저장된 Current Patterns 목록의 슬롯 ID. Flow revision이 같을 때 Follow Live로 현재 슬롯을 따른다.
	std::string m_strSelectedCurrentFlowSlotId;
	//Flow 연결 선택·연결 시작점과 반복 재생을 요청한 Pattern ID.
	std::string m_strSelectedFlowEdgeId;
	std::string m_strFlowLinkSourceNodeId;
	std::string m_strRepeatPatternId;
	//명령·각 패널의 진단 문자열. Server 상태를 결정하지 않고 사용자에게 결과를 표시.
	std::string m_strStatus =
		"Select a pattern, then play it through the Server.";
	std::string m_strFlowStatus =
		"Load the gameplay scriptedSequence, then start it through the Server.";
	std::string m_strNextPatternStatus =
		"Choose the next pattern during live Server playback or while Valtan is idle.";
	std::string m_strCameraStatus;
	std::string m_strActionFeedback;
	std::string m_strPresentationFreshnessStatus;
	//최근 검증한 정본 revision 문자열. 같은 revision의 반복 freshness 검사를 생략하는 기준.
	std::string m_strPresentationFreshnessRevision;
	std::string m_strLogicFlowStatus;
	//graph 교체 때 증가하는 Client 투영 세대. Server tick이나 저장 revision이 아니다.
	uint64_t m_iLogicFlowSourceGeneration = 0u;
	//명시 부활 요청의 순번과 Flow 연결 편집의 최대 통과 횟수 입력.
	uint32_t m_iNextReviveSequence = 1u;
	uint32_t m_iFlowLinkMaximumTraversals = 1u;

	//툴을 열었는지 열지 않았는지? m_bOpen은 Boss 창 자체의 열림이고 실제 표시에는 MainApp 선택도 필요하다.
	//Boss 창의 열림·다음 Render의 focus 요청과 별도 Logic 창 상태. MainApp 표시 선택과 함께 판정.
	bool_t m_bOpen = false;
	bool_t m_bFocusPending = false;
	bool_t m_bLogicPatternOpen = false;
	bool_t m_bLogicPatternFocusPending = false;
	bool_t m_bLogicPatternOpenRequest = false;
	bool_t m_bSelectPatternFlowTab = false;
	//최초 graph 로드 시도 여부와 일시 충돌의 다음 재시도 예약.
	bool_t m_bGraphLoadAttempted = false;
	bool_t m_bCanonicalReloadRetryPending = false;

	//ImGui::GetTime 초 단위의 다음 재시도 시각. 주 스레드 Update가 검사.
	double m_dNextCanonicalReloadRetrySeconds = 0.0;
	//표시 자료와 변경 권한을 분리한다. Reload 실패 시 이전 자료는 보이지만 Can_MutateValtanView가 명령을 막는다.
	VALTAN_VIEW_ADMISSION m_eGraphAdmission =
		VALTAN_VIEW_ADMISSION::UNLOADED;
	//각 표시 자료의 보유 여부. 명령 허용 여부는 위 admission과 별도로 검사.
	bool_t m_bGraphReady = false;
	bool_t m_bProductFallbackReady = false;
	bool_t m_bNextPatternInventoryReady = false;
	//live 목록 따라가기·단일 패턴 반복·부활 응답 표시의 UI 정책.
	bool_t m_bFollowLive = true;
	bool_t m_bRepeat = false;
	bool_t m_bReviveFeedbackPending = false;
	//최근 확인한 연출 기준 일치 결과와 Flow 미저장 변경 폐기 확인·편집 보기 선택.
	bool_t m_bPresentationBaselineIntact = false;
	bool_t m_bConfirmDiscardDirtyFlow = false;
	bool_t m_bFlowGraphEditor = true;
	//버튼이 만든 하위 도구 열기 요청. MainApp의 Consume 호출에서 출력 후 비우는 한 건의 보관함.
	bool_t m_hasCameraToolOpenRequest = false;
	CAMERA_TOOL_OPEN_REQUEST m_CameraToolOpenRequest;
	bool_t m_hasEffectToolOpenRequest = false;
	std::string m_strEffectToolOpenPatternId;
	std::string m_strEffectToolOpenStageId;
	std::string m_strEffectToolOpenCueOccurrenceId;
	std::string m_strEffectToolOpenEffectAssetId;
};

NS_END
