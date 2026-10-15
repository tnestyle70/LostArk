// 같은 편집 세션을 여러 도구가 포함하므로 한 번역 단위 안에서 선언이 중복되지 않게 한다.
#pragma once

// MainApp이 소유하는 쿠크 패턴·시퀀스 편집 세션의 공개 경계다.
// Resources 목록과 선택 상태, 미저장 문서, 실행 요청을 보관하고 실제 모델·월드·Server 실행은
// MainApp이 연결한 기존 소비자에게 맡긴다. 두 workspace는 같은 타입의 별도 인스턴스다.
#include "CompositionResourceTree.h"        // Animation/Action 검색 결과를 화면용 트리로 묶는다.
// m_ResourceTree와 m_PhysicalResourceTree가 COMPOSITION_RESOURCE_TREE_NODE를 값으로 소유하므로 노드의 완전한 정의가 필요하다.
#include "EffectAuthoringResourceTree.h"    // V1/V2 Effect의 원본 자원과 폴더 분류를 표시한다.
// EFFECT_TOOL_KOUKU_EFFECT_TREE를 값으로 보관하고 CEffectAuthoringResourceTree::RESOURCE를 조회표 값으로 쓰므로 중첩 타입 선언도 필요하다.
#include "CompositionAnimationResource.h"   // 공용 Resources에서 들어오는 clip·sequence 값 계약이다.
// m_ModelResources·m_SequenceResources와 전달 함수가 같은 clip/sequence 값 계약을 사용한다. 목록 교체는 inline setter에서도 수행한다.
#include "KoukuCinematicAnimationCatalog.h"  // 여러 actor의 연출 Animation을 한 번에 추가할 자료형이다.
// m_CinematicGroups와 Append_CinematicGroup이 공유하는 KOUKU_CINEMATIC_ANIMATION_GROUP의 행 구조를 가져온다.
#include "CompositionWorkbenchSession.h"    // 공용 shell·MapTool이 호출하는 pane와 clipboard 인터페이스다.
// 상속 대상 ICompositionWorkbenchSession과 COMPOSITION_NATIVE_TRANSFER의 기반 계약을 사용한다. 기반 클래스의 완전한 정의가 필요하다.
#include "EditorUndoHistory.h"              // 문서·선택·외부 World snapshot을 한 편집 이력으로 보관한다.
// m_EditHistory는 CEditorUndoHistory<EDIT_HISTORY_STATE> 객체 자체이므로 템플릿 정의와 내부 보관 구조를 이 선언에서 알아야 한다.
#include "KoukuSaydonCompositionDocument.h"  // 저작 정의·occurrence와 검증/원자 저장의 문서 소유자다.
// m_Document·m_Draft·각 편집값을 값 멤버로 소유한다. 문서의 완전한 타입과 정의/배치 구조가 객체 크기와 소멸에 필요하다.

#include <cstdint>      // 저장 revision, 편집 generation, ms 단위 시계의 폭을 고정한다.
// iSourceRevision은 저장본 번호, m_iDraftGeneration은 메모리 문서 변경 번호다. 같은 정수형이어도 이 두 의미를 비교로 섞지 않는다.
#include <functional>   // World/모델/완주 승인 작업을 실제 소유자의 callback으로 연결한다.
// m_EditWorldAnimation·m_PresentationModelResolver는 호출 가능한 작업을 보관한다. 외부 객체의 수명 보장은 callback의 캡처 방식과 해제 순서에 달려 있다.
#include <map>          // stable ID별 미적용 편집값과 정렬된 자원 분류를 보관한다.
// EFFECT_RESOURCE_CATEGORY_NODE::children은 키 순서로 표시할 분류이고, m_SummonPolicyDrafts는 ID별 원본/입력 쌍이다. 같은 container도 역할은 다르다.
#include <memory>       // 이력 snapshot을 공유하고 CPP에 숨긴 timeline cache를 단독 소유한다.
// m_HistoryDocument는 여러 이력이 같은 불변 사본을 공유하고, m_pTimelineLayout은 세션 하나가 단독으로 해제한다.
#include <string>       // 세션이 소유하는 ID·이름·진단 문자열이다.
// 선택 ID와 진단, 요청에 실리는 JSON은 std::string이 자체 저장 공간을 소유해 원본 목록이나 파일의 수명과 분리한다.
#include <string_view>  // 동기 함수가 호출 동안만 읽는 이름·ID 입력이다.
// 함수 입력은 문자열을 빌려 읽는다. 다음 프레임에도 필요한 ID는 대응 std::string 멤버나 요청 값으로 복사한다.
#include <vector>       // 문서의 순서 있는 행, 선택 목록, 외부 요청의 값 snapshot이다.
// 재생 순서·문서 행·선택 목록을 순회하기 위한 연속 목록이다. 삽입·삭제·문서 교체 뒤에는 이전 행 주소나 index를 그대로 쓰지 않는다.
#include <unordered_map> // stable ID에서 행 index·source metadata로 찾는 파생 조회표다.
// m_DraftPresentationResourceIndices는 ID를 현재 vector 위치로 바꾸는 조회표다. generation이 달라지면 재구성해야 원본 행과 일치한다.

// 저작 문서와 ImGui 세션은 Client 책임이다. 같은 이름공간 안에서도 실제 전투 판정과 월드 객체의 소유자는 외부에 남긴다.
namespace Client
{
	// 모델은 resolver의 참조로만 받고 World 문서는 shared_ptr<const ...>로만 보관한다.
	// 이 헤더는 구체 모델/World 구현을 소유하지 않으므로 완전한 정의는 사용하는 CPP에서 읽는다.
	// enum은 기반 정수형을 고정한 선언만 필요하고, ANIMATION_MODEL_TARGET_VIEW는 callback의 참조 인수로만 노출한다.
	// CWorldSequenceDocument도 여기서는 불변 shared_ptr로 보관하며 필드를 읽지 않아 전방 선언으로 경계를 유지한다.
	enum class ANIMATION_BONE_TARGET : uint8_t;
	struct ANIMATION_MODEL_TARGET_VIEW;
	class CWorldSequenceDocument;
	// Play Pattern 클릭 시 고정한 저작 문서의 실행 준비 결과다. MainApp이 BossTool에 값으로 넘긴다.
	// 임시 gameplay/presentation/encounter projection이며 저작 Save나 정식 Publish 결과가 아니다.
	// revision은 저장본, draft generation은 이 세션의 편집본, world generation은 접속 월드를 구분한다.
    struct KOUKU_DRAFT_PLAY_REQUEST final
    {
        // bBundle에 따라 target은 Bundle 또는 Pattern ID다. 관문과 대상 배치는 준비/입장 검증에 사용한다.
        std::string strTargetId, strGateId;
        bool bBundle = false;
        std::uint32_t iSourceRevision = 0u;
        std::uint64_t iDraftGeneration = 0u, iWorldGeneration = 0u;
        // 게시·서버 준비 소비자가 선택한 실행 범위를 확인한다. 포인터나 화면 row index를 보내지 않는다.
        std::vector<std::string> TargetPlacementIds, PatternIds, BundleIds;
        // 임시 projection 파일에서 읽은 bytes를 요청 자체가 소유하여 준비 디렉터리 수명과 분리한다.
        std::string GameplayRows, PresentationJson, EncounterJson;
    };

	// Collider의 ENTER_AREA → 피해 RESULT 연결을 한 번에 저작할 입력이다.
	// 수치·재진입·밀치기 정책을 Set_ColliderTriggerDamage가 검증하고 문서 후보에 적용한다.
	// 피해 계산과 실제 player 이동은 Server가 담당하며 이 구조체는 실행 상태를 갖지 않는다.
	struct KOUKU_COLLIDER_DAMAGE_SETTINGS final
	{
		// 비율 피해와 고정 피해 중 bFixedDamage가 선택한다. UI 입력의 음수도 검증 전까지 표현한다.
		int32_t iPercent = 10;
		bool_t bFixedDamage = false;
		int32_t iDamageAmount = 500;
		// 영역 재진입·밀치기 이후 재판정과 반복 간격(ms)을 서로 다른 정책으로 남긴다.
		bool_t bRearmOnExit = false, bRepeatAfterKnockback = false;
		std::uint32_t iRepeatIntervalMs = 0u;
		// 높이·범위는 m, 지속 시간은 ms, 방향 offset은 degree다. Server 결과 Logic으로 전달된다.
		double fPushHeightM = 0.0;
		double fPushRangeM = 0.0;
		std::uint32_t iPushMs = 0u;
		std::string strPushDirection = "AWAY_FROM_BOSS";
		bool_t bForcePush = false, bPushCanLeaveArena = false, bPushBallistic = false;
		double fPushYawOffsetDegrees = 0.0;
	};

	// 왼쪽 계층 선택은 타임라인의 box 선택과 별개다. 같은 ID 문자열도 이 종류로 해석한다.
	enum class KOUKU_PATTERN_SELECTION : std::uint8_t { GATE, FOLDER, BUNDLE, PATTERN, BINGO_PARENT };

	// MainApp이 한 번 소비해 실제 preview owner에 전달하는 로컬 시계 명령이다.
	// 화면 Stop은 PAUSE로 시간을 유지하고, Reset이 STOP을 요청해 actor와 preview 상태를 해제한다.
	// MainApp이 소비한다? 실제로 Sequencer 기준으로 소비되는 구간과 내용이 어디인지를 파악한다.
	enum class KOUKU_PREVIEW_TRANSPORT : std::uint8_t
	{
		NONE,
		PAUSE,
		RESUME,
		STOP,
		SEEK
	};

	// MainApp이 실행 소비자의 시계를 매 프레임 값으로 돌려준다. Workbench는 이를 표시할 뿐 모델을 소유하지 않는다.
	// bPlaying은 preview 세션의 존재, bPaused는 그 세션의 시계 정지다. 두 값이 동시에 true일 수 있다.
	struct KOUKU_PREVIEW_STATE final
	{
		bool_t bPlaying = false;
		bool_t bPaused = false;
		std::uint32_t iClockMs = 0u;
		std::uint32_t iDurationMs = 0u;
		std::string strPatternId;
		std::string strStatus;
	};

	// MainApp이 World 문서의 Animation track을 타임라인 표시용 값으로 투영한 행이다.
	// 원본 문서 포인터를 보관하지 않아 World 문서 재로드 이후에도 조회 목록 자체는 유효하다.
	struct KOUKU_WORLD_ANIMATION_INFO final
	{
		std::string strClipName, strSlotId, strDisplayName;
		// local start는 Motion 내부 ms다. instance speed와 track rate는 각각 시간 변환과 clip 재생에 쓰인다.
		std::uint32_t iLocalStartMs = 0u;
		double fInstanceSpeed = 1., fTrackPlaybackRate = 1.;
		// 표시 start/end는 instance delay + local time / instance speed, 합성 rate는 두 배속의 곱이다.
		// source in/out(ms)은 원본 clip 구간이므로 표시 시계와 별도로 보존한다.
		double fStartMs = 0., fEndMs = 0., fPlaybackRate = 1.;
		std::uint32_t iSourceStartMs = 0u, iSourceEndMs = 0u;
		bool bLoop = false, bHoldLastFrame = false;
	};

	// World lane 드래그가 외부 문서 소유자에게 보내는 편집값이다. expectedStartMs로 같은 slot의
	// 이전 시작 시각을 확인하고, 새 시작과 source 구간만 요청해 무관한 World 설정을 보존한다.
	struct KOUKU_WORLD_ANIMATION_EDIT final
	{
		std::string instanceId, slotId, clipName;
		std::uint32_t expectedStartMs = 0u, startMs = 0u, sourceInMs = 0u, sourceOutMs = 0u;
	};

	// World Resources 한 행의 값 snapshot이다. 일반 행의 strInstanceId는 Motion instance,
	// bMotionGroup 행은 여러 Motion을 묶은 Object ID다. 이 구분을 Append가 소비한다.
	// bound XZ는 연결된 배치의 월드 좌표(m)이며 Roulette 중심 복사에 사용한다.
	struct KOUKU_WORLD_SEQUENCE_RESOURCE final
	{
		std::string strInstanceId;
		std::string strDisplayName;
		std::uint32_t iDurationMs = 0u;
		bool_t bHasBoundPlacement = false;
		f32_t fBoundX = 0.f;
		f32_t fBoundZ = 0.f;
		std::string strObjectResourceId;
		bool_t bEnabled = true;
		bool_t bSupportsPlacement = false;
		// 저장된 Motion 기준 위치만 복사한다. emission 간격과 key offset은 원본 local 좌표에 남는다.
		std::array<double, 3u> SavedPosition{};
		std::string strObjectDisplayName;
		std::string strAnchorKind = "WORLD";
		bool_t bDefaultMotion = false;
		// 모델 없는 Object의 저장된 Motion 묶음이면 strInstanceId를 group ID로 해석한다.
		bool_t bMotionGroup = false;
		// 이 기본 Motion을 유일하게 소유하는 저장 group이 있으면 Object Append가 그 group을 사용한다.
		std::string strAppendGroupObjectId;
		std::vector<std::string> AnimationClips;
		std::string strModelAssetId;
		std::vector<KOUKU_WORLD_ANIMATION_INFO> AnimationTracks;
		// Motion의 저작 emission 수다. seeded emitter는 1이며 Box Detail은 이 범위 안에서 행을 고른다.
		std::uint32_t iEmissionCount = 1u;
		// 파괴 수명과 최대 HP는 원본 Object의 표시/배치 정보다. 현재 전투 HP를 추적하는 값은 아니다.
		bool_t bUntilDestroyed = false;
		std::uint32_t iMaximumHp = 0u;
	};

	// Composition의 World box에서 Object Tool을 열 때 MainApp이 소비하는 일회성 deep link다.
	// 원래 pattern/occurrence와 workspace를 함께 보내 편집 결과가 돌아갈 소유자를 구분한다.
	struct KOUKU_WORLD_OBJECT_EDIT_REQUEST final
	{
		std::string strObjectId;
		std::string strMotionInstanceId;
		std::string strPatternId, strOccurrenceId;
		bool bSequenceWorkspace = false, bExplicitPlacement = false;
		bool bFocusAnimationClips = false;
		// MAP box의 임시 배치는 이 occurrence 소유다. 공유 Motion의 원점으로 덮어쓰지 않는다.
		std::optional<KOUKU_SAYDON_WORLD_PLACEMENT> PreviewPlacement;
	};

	// resource와 occurrence를 복사해 다음 MainApp update까지 보존하는 재생 요청이다.
	// 편집 중 원본 vector가 바뀌어도 소비자가 dangling pointer를 읽지 않도록 값으로 전달한다.
	struct KOUKU_PRESENTATION_PREVIEW_REQUEST final
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE Resource;
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE Occurrence;
		// 배치만 미리 볼 때 함께 표시할 World box들이다. 보스나 Logic 실행을 시작하지 않는다.
		std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE> WorldBoxes;
		std::string strSourcePatternId;
		// 이 occurrence가 실제 활성 preview와 일치할 때만 기존 시계를 유지하며 편집값을 반영한다.
		std::string strEditedOccurrenceId;
	};

	// Summon box의 미적용 배치값을 실제 preview 소유자에게 전달한다. 저작 원본 저장과 분리된 값이다.
	struct KOUKU_SUMMON_PLACEMENT_PREVIEW_REQUEST final
	{
		std::string strPatternId;
		KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE Occurrence;
	};

	// 동일 pattern/occurrence의 transform preview를 덮어쓸 키와 값이다.
	// pending 전달 큐와 Save 전 staged overlay가 같은 자료형을 사용하되 수명은 각각 관리한다.
	struct KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST final
	{
		std::string strPatternId;
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE Occurrence;
		bool operator==(const KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST&) const = default;
	};


	// 실제 월드 picking·camera focus는 MainApp이 수행하고 결과만 편집 세션으로 돌려준다.
	enum class KOUKU_MAP_EFFECT_PLACEMENT_ACTION : std::uint8_t { PICK, FOCUS };

	// picking을 시작한 선택의 ID·transform·시간·draft generation을 함께 고정한다.
	// Position은 월드 m, Scale은 무차원 배율이다. 대기 중 다른 box로 바뀌면 완료 시 적용을 거부한다.
	struct KOUKU_MAP_EFFECT_PLACEMENT final
	{
		std::string strPatternId, strOccurrenceId, strResourceId, strEffectAssetId;
		std::array<double, 3u> Position{}, Scale{1.0, 1.0, 1.0};
		std::uint32_t iStartMs = 0u, iDurationMs = 0u;
		std::uint64_t iDraftGeneration = 0u;
		bool operator==(const KOUKU_MAP_EFFECT_PLACEMENT&) const = default;
	};

	// token은 이전 비동기 picking 결과가 새 요청을 완료시키지 못하게 구분하는 세션 내 번호다.
	struct KOUKU_MAP_EFFECT_PLACEMENT_REQUEST final
	{
		KOUKU_MAP_EFFECT_PLACEMENT_ACTION eAction = KOUKU_MAP_EFFECT_PLACEMENT_ACTION::PICK;
		std::uint64_t iRequestToken = 0u;
		KOUKU_MAP_EFFECT_PLACEMENT Selection;
	};

	// MainApp 소유의 편집 세션이다. 생성 시 패턴/시퀀스 문서 경로를 고정하고 미저장 draft를 독립 보관한다.
	// 공용 shell·MapTool은 pane를 차용하고 MainApp은 pending 요청을 실제 도구/서비스에 전달한다.
	// update와 ImGui frame에서 순서대로 접근한다. 외부 준비 process는 세션 메모리를 직접 쓰지 않으며
	// Tick_Background가 완료를 관찰한다. 창을 숨기는 것과 문서 폐기·Server 전투 중지는 별개다.
	// 공용 host의 pane 렌더링은 Begin → 종류별 pane → End로 묶으며, 이 타입은 쿠크 문서의 선택·검증·저장을 맡는다.
	// pane를 그리다가 문서가 바뀌는 문제를 막기 위해 선택 ID와 미적용 입력, 실행 요청을 별도 상태로 보관한다.
	// 아래 클래스 안의 짧은 정의는 언어상 inline이다. 여러 CPP에 같은 정의를 제공하며 호출 치환 여부는 컴파일러가 정한다.
	class CKoukuSaydonActionWorkbench final : public ICompositionWorkbenchSession
	{
#ifdef LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS
		friend struct CKoukuSaydonWorkbenchTestAccess;
#endif
	public:
		// 세션 수명·pane 호스팅: Begin/End 쌍은 한 프레임에 한 host만 호출한다.
		explicit CKoukuSaydonActionWorkbench(bool sequenceWorkspace = false);
		~CKoukuSaydonActionWorkbench();

		void Open();
		bool_t Select_ActorProfile(std::string_view actorProfileId, std::string& outStatus);
		// host가 읽는 창 표시 상태다. 값만 조회하며 문서 로드나 preview 정지는 이 getter에서 일어나지 않는다.
		[[nodiscard]] bool_t Is_Open() const noexcept
		{
			return m_bOpen;
		}
		// 독립 창 진입점이다. 현재 공용 Action Workbench는 아래 Begin/Pane/End 경로를 사용한다.
		void Render();
		void Render_WorldPlacementTuning(std::string_view patternId, std::string_view occurrenceId);
		void Begin_WorkbenchFrame() override;
		void Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE pane) override;
		void End_WorkbenchFrame() override;
		// 문서와 선택, staged geometry, 연결된 World 편집을 함께 되돌린다. pane의 행 참조가 끝난 뒤 처리한다.
		bool Undo_Edit(std::string& status);
		bool Redo_Edit(std::string& status);
		COMPOSITION_WORKBENCH_VIEW_REQUEST Consume_WorkbenchViewRequest() override;
		// 창이 닫혀도 MainApp이 호출한다. 프로세스 완료·진단 수신을 Render 호출 여부에 묶지 않는다.
		void Tick_Background()
		{
			Poll_PublishProcess();
			Poll_DraftPlayProcess();
		}
		// 게시 완료 edge를 한 번만 돌려줘 BossTool의 제품 목록을 다시 읽게 한다. 시퀀스 세션은 제외한다.
		bool_t Consume_ProductInventoryRefreshRequest()
		{
			const bool_t requested = m_bProductInventoryRefreshRequested;
			m_bProductInventoryRefreshRequested = false;
			return !m_bSequenceWorkspace && requested;
		}
		bool_t Consume_PresentationPreviewRequest(KOUKU_PRESENTATION_PREVIEW_REQUEST& outRequest);
		// 외부 World 문서의 실제 소유자가 편집·저장을 수행한다. callback 실패 시 status를 받아
		// 이 세션의 미저장 상태를 유지하며, callback 교체는 MainApp이 문서 수명과 함께 관리한다.
		void Set_WorldAnimationCallbacks(
			std::function<bool_t(const KOUKU_WORLD_ANIMATION_EDIT&, std::string&)> edit,
			std::function<bool_t(std::string&)> save)
		{
			m_EditWorldAnimation = std::move(edit);
			m_SaveWorldAnimation = std::move(save);
		}
		// Undo snapshot은 불변 World 문서를 공유한다. raw pointer의 수명에 의존하지 않는다.
		using WORLD_HISTORY_DOCUMENT = std::shared_ptr<const CWorldSequenceDocument>;
		void Set_WorldAnimationHistoryCallbacks(
			std::function<WORLD_HISTORY_DOCUMENT(bool)> capture,
			std::function<bool(const WORLD_HISTORY_DOCUMENT&, const WORLD_HISTORY_DOCUMENT&, std::string&)> restore)
		{
			m_CaptureWorldAnimationHistory = std::move(capture);
			m_RestoreWorldAnimationHistory = std::move(restore);
		}
		bool_t Consume_WorldObjectEditRequest(KOUKU_WORLD_OBJECT_EDIT_REQUEST& outRequest);
		void Notify_WorldObjectEditResult(std::string status)
		{
			m_strStatus = std::move(status);
		}
		// MainApp이 기존 reader의 camera/audio/light 목록을 전달한다. 이 함수는 V1 metadata를
		// 보강하고 전달받은 inventory를 교체한다. V1 metadata 읽기 실패는 해당 이전 cache를 유지한다.
		void Set_PresentationResources(
			std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> resources,
			std::string status);
		bool_t Consume_PresentationResourceRefreshRequest();
		// 공용 Resources의 drag/drop 계약이다. 가능 여부 질의는 읽기 전용, Append만 후보 문서를 commit한다.
		bool Can_AppendCompositionAnimationResource(
			const COMPOSITION_ANIMATION_RESOURCE& resource, bool asNewStage,
			std::string& outStatus) const override;
		bool Append_CompositionAnimationResource(
			const COMPOSITION_ANIMATION_RESOURCE& resource, bool asNewStage,
			std::string& outStatus) override;
		// MainApp이 갱신 요청을 한 번 가져가면 flag를 비운다. 목록 읽기의 성공 여부는 Set_ModelResources 등의 결과로 따로 반영한다.
		bool Consume_ResourceRefreshRequest()
		{
			const bool requested = m_bResourceRefreshRequested;
			m_bResourceRefreshRequested = false;
			return requested;
		}
		// 실행 도구가 읽은 자원 목록을 복사해 UI가 사용한다. 저장된 composition 정의와 별도 inventory다.
		void Set_ModelResources(std::vector<COMPOSITION_ANIMATION_RESOURCE> resources,
			std::string status);
		// sequence 조회 실패는 기존 목록을 보존하고 진단만 바꾼다. tree는 다음 표시 때 재구성한다.
		void Set_SequenceResources(std::vector<COMPOSITION_ANIMATION_SEQUENCE_RESOURCE> resources,
			std::string status, bool_t succeeded = true)
		{
			if (succeeded)
				m_SequenceResources = std::move(resources);
			m_strSequenceResourceStatus = std::move(status);
			m_bResourceTreeDirty = true;
		}
		// World 목록 교체마다 generation을 올려 timeline의 World track cache를 다시 계산하게 한다.
		void Set_WorldSequenceResources(
			std::vector<KOUKU_WORLD_SEQUENCE_RESOURCE> resources, std::string status)
		{
			m_WorldSequenceResources = std::move(resources);
			++m_iWorldSequenceResourceGeneration;
			m_strWorldSequenceResourceStatus = std::move(status);
		}
		// 선택 가능한 profile ID 목록만 전달한다. 팀장 RenderingProfiles 값 자체를 수정하지 않는다.
		void Set_RenderingProfileResources(
			std::vector<std::string> profileIds, std::string status)
		{
			m_RenderingProfileIds = std::move(profileIds);
			m_strRenderingProfileResourceStatus = std::move(status);
		}
		// shell의 관문 선택은 세션 선택 상태로만 반영한다. local preview 명령은 아래 Consume 경계로 나간다.
		void Select_WorkbenchBoss(COMPOSITION_WORKBENCH_BOSS boss) override;
		bool_t Consume_PreviewTransportRequest(
			KOUKU_PREVIEW_TRANSPORT& outTransport,
			std::uint32_t& outSeekMs);
		[[nodiscard]] bool_t Has_PendingLocalPreviewRequest() const noexcept
		{
			return m_bPatternPreviewRequestPending || m_bBundlePreviewRequestPending ||
				m_bPreviewRequestPending || m_bPresentationPreviewRequestPending;
		}
		[[nodiscard]] bool_t Has_PendingPreviewReset() const noexcept
		{
			return m_bExplicitPreviewReset && m_ePendingTransport == KOUKU_PREVIEW_TRANSPORT::STOP;
		}
		// 실제 소비자가 돌려준 상태를 표시한다. 시작 결과를 기다리던 프레임에는 반복된 진단도 다시 노출한다.
		void Set_PreviewState(const KOUKU_PREVIEW_STATE& state)
		{
			if (!state.strStatus.empty() && (m_bPreviewResultStatusPending ||
				state.strStatus != m_PreviewState.strStatus))
				m_strStatus = state.strStatus;
			m_bPreviewResultStatusPending = false;
			m_PreviewState = state;
		}
		// 이 참조 getter들은 세션의 현재 표시값을 차용한다. 다음 상태 갱신 뒤에도 이전 내용을 보존하려면 호출자가 값을 복사한다.
		[[nodiscard]] const std::string& Get_Status() const noexcept
		{
			return m_strStatus;
		}
		[[nodiscard]] const KOUKU_PREVIEW_STATE& Get_PreviewState() const noexcept
		{
			return m_PreviewState;
		}


		// 저작 문서 수명: Reload는 읽기 성공 후 draft를 교체하고, Save는 source 저장을 요청한다.
		// UI는 dirty Reload 전에 확인 창을 띄운다. 공개 Reload 자체가 확인 대화를 소유하지는 않는다.
		bool_t Reload(std::string& outStatus);
		bool_t Save(std::string& outStatus);
		// Publish는 전체 저장 트리를 대상으로 별도 process를 준비한다. Save 성공을 runtime 게시 성공으로 취급하지 않는다.
		bool_t Publish_AllPatterns(std::string& outStatus);
		[[nodiscard]] bool_t Is_PublishRunning() const noexcept
		{
			return nullptr != m_hPublishProcess;
		}
		bool_t Validate_Draft(std::string& outStatus) const;
		[[nodiscard]] bool_t Has_Composition() const noexcept
		{
			return m_bHasDraft;
		}
		// 문서에 아직 합치지 않은 geometry overlay도 저장 대상이며, 외부 World 편집은 전체 dirty에만 더한다.
		[[nodiscard]] bool_t Is_CompositionDirty() const noexcept
		{
			return m_bDirty || !m_StagedPresentationGeometry.empty();
		}
		[[nodiscard]] bool_t Is_Dirty() const noexcept
		{
			return Is_CompositionDirty() || m_WorldAnimationEditsPending;
		}
		void Notify_WorldAnimationSaved() noexcept
		{
			m_WorldAnimationEditsPending = false;
		}
		[[nodiscard]] std::uint64_t Get_DraftGeneration() const noexcept
		{
			return m_iDraftGeneration;
		}
		// 반환 참조는 이 세션 소유다. 다음 commit/reload까지의 동기 조회에 쓰고 외부에 오래 저장하지 않는다.
		// preview getter는 재생을 위해 확장한 자식 snapshot이 있으면 그것을 우선한다.
		[[nodiscard]] const KOUKU_SAYDON_COMPOSITION_DOCUMENT&
			Get_Composition() const noexcept
		{
			return m_Draft;
		}
		[[nodiscard]] const KOUKU_SAYDON_COMPOSITION_DOCUMENT& Get_PatternPreviewDocument() const noexcept
		{
			return m_ExpandedPatternPreview ? *m_ExpandedPatternPreview : m_Draft;
		}
		[[nodiscard]] const std::vector<KOUKU_SAYDON_COMPOSITION_PATTERN>&
			Get_AuthoringPatterns() const noexcept
		{
			return m_Draft.Patterns;
		}

		// 계층·box 선택은 stable ID로 주고받는다. 문서 vector 위치는 외부 저장/선택 계약이 아니다.
		[[nodiscard]] const std::string& Get_SelectedPatternId() const noexcept
		{
			return m_strSelectedPatternId;
		}
        [[nodiscard]] const std::string& Get_SelectedPresentationOccurrenceId() const noexcept
		{
			return m_strSelectedPresentationOccurrenceId;
		}
		bool_t Select_BingoParent(std::string& outStatus);
		bool_t Set_BingoFlow(const KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW& flow, std::string& outStatus);
		bool_t Select_PatternById(
			std::string_view patternId,
			std::string& outStatus);
		// MainApp이 picking/focus를 실행한다. 완료 시 token·generation·선택 snapshot을 검사해 다른 편집을 덮어쓰지 않는다.
		bool_t Get_SelectedMapEffectPlacement(KOUKU_MAP_EFFECT_PLACEMENT& outPlacement) const;
		bool_t Request_MapEffectPlacement(KOUKU_MAP_EFFECT_PLACEMENT_ACTION action);
		bool_t Consume_MapEffectPlacementRequest(KOUKU_MAP_EFFECT_PLACEMENT_REQUEST& outRequest);
		bool_t Is_MapEffectPlacementRequestCurrent(const KOUKU_MAP_EFFECT_PLACEMENT_REQUEST& request) const;
		bool_t Complete_MapEffectPlacementRequest(const KOUKU_MAP_EFFECT_PLACEMENT_REQUEST& request,
			const std::array<double, 3u>& position, std::string& outStatus);
		void Cancel_MapEffectPlacementRequest(std::uint64_t token = 0u);
		bool_t Select_WorldBoxById(std::string_view patternId,
			std::string_view occurrenceId, std::string& outStatus);
		bool_t Select_PresentationBoxById(std::string_view patternId,
			std::string_view occurrenceId, std::string& outStatus);
		// 단일 Collider를 조절 대상으로 선택하되 저장된 선택 group 연결은 유지한다.
		bool_t Select_ColliderGroupMemberById(std::string_view patternId,
			std::string_view occurrenceId, std::string& outStatus);
		bool_t Consume_AnimationPreviewRequest(
			KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& outRequest);
		// 끝 시각에서 Play하면 처음으로 돌아가고, 정지 상태 scrub은 끝점 자세를 그대로 샘플링한다.
		bool_t Request_PatternPreview(std::string_view patternId,
			std::uint32_t startClockMs, std::string& outStatus, bool_t startPaused = false,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE* presentationOverride = nullptr);
		// 완주 요청은 주입된 승인 callback으로 넘긴다. callback이 없으면 선택 Gate의 유효한 전투 진입 Sequence 하나를 0 ms에서 요청한다.
		bool_t Request_CompleteSequencePlay(std::string& outStatus, std::string_view gateId = {});
		void Set_CompleteSequenceAdmission(std::function<bool_t(std::string_view, std::string&)> admission)
		{
			m_CompleteSequenceAdmission = std::move(admission);
		}
		void Notify_SequencePreviewAdmission(bool_t succeeded, const std::string& status);
		bool_t Advance_CompleteSequencePlay(std::string_view completedPatternId, std::string* completedGate = nullptr);
		void Cancel_CompleteSequencePlay();
		[[nodiscard]] bool_t Is_CompleteSequencePlaying() const noexcept
		{
			return !m_CompleteSequencePatternIds.empty();
		}
		void Request_PreviewReset();
		bool_t Request_PreviewPause();
		bool_t Request_PreviewResume();
		bool_t Request_PatternScrub(std::string_view patternId, std::uint32_t clockMs, std::string& outStatus);
		bool_t Request_BundleScrub(std::uint32_t clockMs);
		// Bone Collider 상세 preview는 actor Animation을 유지하고 해당 box 시작 또는 현재 pause 시각에서 재생한다.
		bool_t Request_ColliderBoxPreview(std::string_view patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& value, std::string& outStatus);
		// transform만 staged overlay로 보관한다. 활성 preview의 시계·actor·session을 재생성하지 않고 갱신한다.
		bool_t Request_PresentationGeometryPreview(std::string_view patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& value, std::string& outStatus);
		bool_t Consume_PresentationGeometryPreviewRequest(KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST& outRequest);
		bool_t Consume_SummonPlacementPreviewRequest(KOUKU_SUMMON_PLACEMENT_PREVIEW_REQUEST& outRequest);
		void Cancel_PresentationGeometryPreview(bool_t discardStagedGeometry = true);
		bool_t Consume_PatternPreviewRequest(
			KOUKU_SAYDON_COMPOSITION_PATTERN& outPattern,
			std::uint32_t& outStartClockMs,
			bool_t& outStartPaused,
			std::string& outTargetAssetName);
		bool_t Consume_BundlePreviewRequest(std::string& bundleId, std::uint32_t& clockMs, bool_t& paused);
		bool_t Consume_BundleServerPlayRequest(std::string& bundleId, std::uint32_t& revision);
		// 선택한 Pattern/Bundle의 draft snapshot을 검증해 임시 실행 projection 준비를 시작한다.
		// 월드가 바뀌면 결과를 거부한다. 준비 중 draft 편집은 클릭 snapshot을 유지하되 자동 선택 추적을 보류한다.
		bool_t Request_SelectedServerPlay(std::string& outStatus);
        bool_t Consume_DraftServerPlayRequest(KOUKU_DRAFT_PLAY_REQUEST& request);
        void Cancel_DraftPlayPreparation();
		bool_t Select_BundleById(std::string_view bundleId, std::string& status);
		bool_t Consume_ServerPlayRequest(
			std::string& outPatternId,
			std::uint32_t& outSourceRevision);
		// BossTool 준비가 끝나도 이 세션의 projection process나 미소비 결과가 있으면 pending을 유지한다.
		void Set_ServerPlayPreparationPending(bool_t pending, std::string_view status = {})
		{
			m_bServerPlayPreparationPending = pending || m_hDraftPlayProcess != nullptr || m_bDraftPlayReady;
			if (!status.empty())
				m_strStatus = status;
		}
		// MainApp에 취소 의사를 한 번 전달하는 통로다. false로 돌리는 것은 요청 소비이며 Server의 취소 완료 판정이 아니다.
		bool_t Consume_ServerPlayCancelRequest()
		{
			const bool_t requested = m_bServerPlayCancelRequested;
			m_bServerPlayCancelRequested = false;
			return requested;
		}

		// Create Pattern의 목적지 선택은 Parent/Bundle 관계를 검증한 뒤 세션 상태를 바꾼다.
		bool_t Set_PatternCreationDestination(std::string_view folderId,
			std::string_view bundleId, std::string& outStatus);
		bool_t Create_Pattern(
			std::string_view displayName,
			std::string_view category,
			std::string& outPatternId,
			std::string& outStatus);
		bool_t Create_ParentTimeline(std::string_view folderId, std::string& outStatus);
		// Parent folder 또는 timeline Pattern ID를 받는다. timeline 신규 생성과 자식 box 추가를 한 후보로 commit한다.
		bool_t Append_PatternBox(std::string_view ownerId, std::string_view childId,
			std::uint32_t startMs, std::uint32_t durationMs, std::string& outStatus);
		bool_t Set_PatternBoxWindow(std::string_view ownerId, std::string_view occurrenceId,
			std::uint32_t startMs, std::uint32_t durationMs, bool_t repeat, std::string& outStatus);
		bool_t Repeat_ParentCycle(std::string_view ownerId, std::uint32_t loopWindowMs,
			std::string& outCyclePatternId, std::string& outStatus);
		bool_t Delete_Pattern(
			std::string_view patternId,
			std::string& outStatus);
		bool_t Rename_Pattern(
			std::string_view patternId,
			std::string_view displayName,
			std::string& outStatus);
		bool_t Set_PatternFolder(std::string_view patternId,
			std::string_view folderId, std::string& outStatus);
		bool_t Set_PatternCategory(
			std::string_view patternId,
			std::string_view category,
			std::string& outStatus);
		bool_t Set_PatternAuthoringStatus(
			std::string_view patternId,
			std::string_view authoringStatus,
			std::string& outStatus);

		// Stage/Animation 편집: Stage 시간은 보스 진행, occurrence의 source 구간과 재생 창은 clip 소비를 결정한다.
		bool_t Add_Stage(
			std::string_view patternId,
			std::string_view stageKind,
			std::uint32_t durationMs,
			std::string& outStageId,
			std::string& outStatus);
		bool_t Append_AnimationAsStage(
			std::string_view patternId,
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& source,
			std::string& outStageId,
			std::string& outOccurrenceId,
			std::string& outStatus);
		// 추출 Action의 원본 stage마다 Stage를 만들고 그 slot들을 연속 배치한다. 참조 원본은 변경하지 않는다.
		bool_t Append_ActionAsStages(
			std::string_view patternId,
			std::string_view profileId,
			std::uint32_t sourceActionId,
			std::string& outStatus);
		// 기존 Stage의 마지막 box 뒤에 slot들을 추가하고 이를 담을 만큼 Stage 시간을 늘린다.
		bool_t Append_ActionToStage(
			std::string_view patternId,
			std::string_view stageId,
			std::string_view profileId,
			std::uint32_t sourceActionId,
			std::string& outStatus);
		bool_t Delete_Stage(
			std::string_view patternId,
			std::string_view stageId,
			std::string& outStatus);
		// 모든 선택 ID를 먼저 확인한 뒤 Stage와 여러 lane의 선택을 한 문서 commit으로 삭제한다.
		bool_t Delete_TimelineSelection(
			std::string_view patternId,
			const std::vector<std::string>& stageIds,
			const std::vector<std::string>& occurrenceIds,
			std::string& outStatus);
		// Copy는 선택과 소유 참조를 값 snapshot으로 고정한다. Paste는 목적지의 기존 시계를 밀지 않고 끝에 붙인다.
		bool_t Copy_TimelineSelection(std::string_view patternId,
			const std::vector<std::string>& stageIds,
			const std::vector<std::string>& occurrenceIds, std::string& outStatus);
		bool_t Paste_TimelineClipboard(std::string_view patternId, std::string& outStatus);
		bool_t Has_TimelineClipboard() const noexcept
		{
			return m_TimelineClipboard.has_value();
		}
		// Stage 전체와 개별 lane 선택을 함께 복제해도 같은 자식 box는 한 번만 복사한다.
		// 상대 시각과 소유 참조를 새 ID로 재연결하며 atOriginalTime에 따라 원래 위치 또는 뒤에 배치한다.
		bool_t Duplicate_TimelineSelection(
			std::string_view patternId,
			const std::vector<std::string>& stageIds,
			const std::vector<std::string>& occurrenceIds,
			std::string& outStatus, bool_t atOriginalTime = false);
		// group은 함께 선택할 저작 metadata다. Collider geometry의 소유자는 각 occurrence로 유지한다.
		bool_t Set_ColliderSelectionGroup(std::string_view patternId,
			const std::vector<std::string>& occurrenceIds, bool_t grouped, std::string& outStatus);
		// 각 Effect anchor를 유지하고 공통 기준으로 중심 이동·회전을 계산한다.
		bool_t Set_EffectSelectionGroup(std::string_view patternId,
			const std::vector<std::string>& occurrenceIds, bool_t grouped, std::string& outStatus);
		// Sound의 각 원본 구간을 보존하면서 timeline 이동만 묶는다.
		bool_t Set_SoundSelectionGroup(std::string_view patternId,
			const std::vector<std::string>& occurrenceIds, bool_t grouped, std::string& outStatus);
		bool_t Transform_SelectedEffects(const std::array<double, 3u>& translation,
			const std::array<double, 4u>& rotationDelta, std::string& outStatus);
		bool_t Transform_SelectedColliders(const std::array<double, 3u>& translation,
			double yawDeltaDegrees, std::string& outStatus);
		// 전체 lane 수명을 바꾸며 기존 Stage 시계는 유지한다. 모든 기존 행의 끝보다 짧은 입력은 거부한다.
		// Stage도 명시 수명도 없는 최초 빈 Pattern은 ACTIVE Stage 생성으로 진입한다.
		bool_t Set_PatternDuration(std::string_view patternId,
			std::uint32_t durationMs, std::string& outStatus);
		bool_t Move_Stage(
			std::string_view patternId,
			std::string_view stageId,
			int32_t direction,
			std::string& outStatus);
		// Earlier/Later가 선택 Stage와 Animation 소유 관계를 함께 재정렬한다.
		// 선택의 내부 순서·ID는 유지하고 Pattern 경계를 넘는 이동은 전체 거부한다.
		bool_t Move_SelectedStage(
			std::string_view patternId,
			int32_t direction,
			std::string& outStatus);
		// 첫 Animation의 offset과 뒤따르는 Stage 시각을 조정한다. 다른 lane의 절대 시각은 유지한다.
		bool_t Set_PatternStartOffset(std::string_view patternId,
			std::uint32_t startOffsetMs, std::string& outStatus);
		bool_t Set_StageDuration(
			std::string_view patternId,
			std::string_view stageId,
			std::uint32_t durationMs,
			std::string& outStatus);
		bool_t Set_StageKind(
			std::string_view patternId,
			std::string_view stageId,
			std::string_view stageKind,
			std::string& outStatus);

		bool_t Set_StageRetargetOnEnter(
			std::string_view patternId,
			std::string_view stageId,
			bool_t retargetOnEnter,
			std::string& outStatus);

		bool_t Bind_Animation(
			std::string_view patternId,
			std::string_view targetStageId,
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& source,
			std::uint32_t startOffsetMs,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Move_Animation(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t startOffsetMs,
			std::string& outStatus);
		bool_t Trim_Animation(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t sourceStartMs,
			std::uint32_t playMs,
			std::string& outStatus, std::uint32_t sourceEndMs = 0u);
        bool_t Loop_AnimationToStageEnd(std::string_view patternId, std::string_view occurrenceId, std::string& outStatus);
		// 배속·종료 정책은 occurrence 소유다. EXACT로 원본 clip 구간을 넘는 재생은 거부한다.
		// drag/trim으로 늘어난 창은 Normalize_EndPolicyForWindow가 HOLD_LAST_POSE로 바꿀 수 있다.
		bool_t Set_AnimationPlayback(
			std::string_view patternId,
			std::string_view occurrenceId,
			f32_t playRate,
			std::string_view endPolicy,
			std::string& outStatus);
		bool_t Set_AnimationBlend(std::string_view patternId, std::string_view occurrenceId,
			std::uint32_t blendInMs, std::string& outStatus);
		bool_t Duplicate_Animation(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Delete_Animation(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outStatus);
		bool_t Move_AnimationToStage(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string_view targetStageId,
			std::uint32_t startOffsetMs,
			std::string& outStatus);

		// Logic 정의는 Resources가 공유하고 box는 Pattern 기준 실행 창과 결과 연결을 소유한다.
		// 현재 typed 판단/결과 값과 Server 소비 경로가 있으며, 편집 검증과 게시 가능 여부 검증은 구분한다.
		// 정의 변경은 그 정의를 쓰는 box의 호환되지 않는 hold/outcome/collider 연결도 후보 안에서 정리한다.
		bool_t Create_Logic(
			std::string_view displayName,
			std::string_view logicType,
			std::string& outLogicId,
			std::string& outStatus);
		bool_t Delete_Logic(
			std::string_view logicId,
			std::string& outStatus);
		bool_t Append_LogicBox(
			std::string_view patternId,
			std::string_view logicId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Set_LogicBoxWindow(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outStatus);
		bool_t Delete_LogicBox(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outStatus);
		bool_t Set_LogicBoxHold(std::string_view patternId, std::string_view occurrenceId,
			std::string_view holdOccurrenceId, std::string& outStatus);
		// DURATION box의 결과 slot 하나를 순서 있는 최대 네 RESULT ID로 교체한다. 빈 목록은 해제다.
		// 연결은 box 소유라서 같은 판단 정의도 Pattern마다 서로 다른 후속 결과로 이어질 수 있다.
		bool_t Set_LogicBoxOutcomes(
			std::string_view patternId,
			std::string_view occurrenceId,
			KOUKU_SAYDON_OUTCOME_SLOT slot,
			const std::vector<std::string>& resultLogicIds,
			std::string& outStatus);
		// 정의 ID·표시 이름·Logic 종류를 유지하면서 typed 판단/결과 값만 교체한다.
		bool_t Set_LogicDefinitionValues(
			std::string_view logicId,
			const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& values,
			std::string& outStatus);

		// 선택한 정의를 찾아 아래 transaction에 넘긴다. 정의가 없으면 기존 Collider 연결을 보존한다.
		bool_t Connect_ColliderLogic(const std::string& patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence,
			const std::string& logicId, std::string& outStatus);
		// 정의 값, 정확한 Logic 실행 창, Collider 연결을 같은 후보 문서에서 검증하고 함께 commit한다.
		bool_t Set_ColliderLogicValues(std::string_view patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence,
			const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& values, std::string& outStatus);
		// 앞뒤 trim 시 timeline 창과 source 구간을 함께 조정한다. 인수 row만 바꾸며 문서 commit은 호출자가 한다.
		static void Trim_PresentationWindow(KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& row,
			std::int64_t deltaMs, bool_t trimStart, std::uint32_t timelineDurationMs,
			std::uint32_t sourceDurationMs, bool_t effect);
		bool_t Set_PresentationBox(std::string_view patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& value, std::string& outStatus);
		// 표시 flag 하나만 반영하므로 아직 Apply하지 않은 다른 Box Detail 입력을 덮어쓰지 않는다.
		bool_t Set_PresentationBoxDebugRender(std::string_view patternId,
			std::string_view occurrenceId, bool_t visible, std::string& outStatus);

		// ENTER_AREA와 피해 RESULT를 함께 연결하고 다른 결과 slot은 유지한다. 과거 문서 전체를 직접 덮어쓰지 않는다.
		bool_t Set_ColliderTriggerDamage(const std::string& patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence,
			const KOUKU_COLLIDER_DAMAGE_SETTINGS& settings, std::string& outStatus);
		bool_t Set_ColliderTriggerDamage(const std::string& patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence,
			std::uint32_t percent, std::string& outStatus)
		{
			KOUKU_COLLIDER_DAMAGE_SETTINGS settings;
			// 범위 밖 unsigned 입력은 101로 전달해 공용 검증에서 거부한다. signed 변환 overflow를 피한다.
			settings.iPercent = percent <= 100u ? static_cast<int32_t>(percent) : 101;
			return Set_ColliderTriggerDamage(patternId, occurrence, settings, outStatus);
		}

		// Summon 정의는 소환 방식·공유 정책, box는 Pattern 안의 시작/수명과 개별 배치를 소유한다.
		// 현재 명시 Pattern 소환과 네 방향 clone 정책을 저작하므로 단순 이름 catalog로 한정하지 않는다.
		bool_t Create_Summon(
			std::string_view displayName,
			std::string& outSummonId,
			std::string& outStatus);
		bool_t Delete_Summon(
			std::string_view summonId,
			std::string& outStatus);
		bool_t Append_SummonBox(
			std::string_view patternId,
			std::string_view summonId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Set_SummonBoxWindow(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outStatus);
		bool_t Delete_SummonBox(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outStatus);

		// World lane은 기존 WorldSequence/Motion을 참조한다. box가 시작 시각·배속·배치를 갖는다.
		// 모델의 본 조회와 캐릭터 앞 배치는 실제 preview/Level 소유자가 resolver로 제공한다.
		// 본 이름·모델 상태는 실제 preview 소유자가 알아야 한다. 문서 편집기는 resolver 성공 시 얻은 view만 해당 호출에서 사용한다.
		using PRESENTATION_MODEL_RESOLVER = std::function<bool_t(
			const KOUKU_SAYDON_COMPOSITION_PATTERN&, ANIMATION_BONE_TARGET, ANIMATION_MODEL_TARGET_VIEW&)>;
		void Set_PresentationModelResolver(PRESENTATION_MODEL_RESOLVER resolver)
		{
			m_PresentationModelResolver = std::move(resolver);
		}
		void Set_WorldPlacementResolver(std::function<bool_t(KOUKU_SAYDON_WORLD_PLACEMENT&, std::string&)> resolver)
		{
			m_WorldPlacementResolver = std::move(resolver);
		}
		bool_t Append_WorldResource(std::string_view instanceId, std::string& outStatus, bool_t asObject = false);
		bool_t Append_WorldObject(std::string_view objectResourceId, std::string& outStatus);
		bool_t Set_WorldBoxPlacement(std::string_view patternId, std::string_view occurrenceId,
			const KOUKU_SAYDON_WORLD_PLACEMENT& placement, std::string& outStatus);
		bool_t Place_WorldBoxNearCharacter(std::string_view patternId, std::string_view occurrenceId,
			std::string& outStatus);
		bool_t Create_World(
			std::string_view displayName,
			std::string_view sequenceInstanceId,
			std::string& outWorldId,
			std::string& outStatus);
		bool_t Set_WorldCompanionEffect(std::string_view worldId,
			std::string_view resourceId, std::string& outStatus);
		bool_t Delete_World(
			std::string_view worldId,
			std::string& outStatus);
		bool_t Append_WorldBox(
			std::string_view patternId,
			std::string_view worldId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Set_WorldBoxWindow(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			f32_t playbackSpeed,
			std::string& outStatus);
		bool_t Delete_WorldBox(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outStatus);

		// Scene Profile 정의는 기존 rendering profile ID를 참조하고 box는 적용 창·blend(ms)를 소유한다.
		// 여기서 팀장 조율값 자체를 바꾸지 않으며 실제 profile 전환은 preview/runtime 소비자가 수행한다.
		bool_t Create_SceneProfile(
			std::string_view displayName,
			std::string_view renderingProfileId,
			std::string& outSceneProfileId,
			std::string& outStatus);
		bool_t Delete_SceneProfile(
			std::string_view sceneProfileId,
			std::string& outStatus);
		bool_t Append_SceneProfileBox(
			std::string_view patternId,
			std::string_view sceneProfileId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Set_SceneProfileBoxWindow(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::uint32_t blendMs,
			std::string& outStatus);
		bool_t Delete_SceneProfileBox(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outStatus);

    // 공용 shell의 clipboard 명령과 resource drop을 이 세션의 검증/commit 경계로 받는다.
    bool Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND command, std::string& status) override;
    bool Insert_CompositionTransfer(const COMPOSITION_TRANSFER& transfer, std::string& status) override;

	private:
		// 표시 이름과 삭제 확인: stable ID는 유지하고 참조 영향·확인 시점 generation을 검사한다.
		enum class RENAME_TARGET : std::uint8_t
		{
			PATTERN, FOLDER, BUNDLE, LOGIC, SUMMON, WORLD, SCENE_PROFILE, PRESENTATION
		};
		bool_t Rename_Item(RENAME_TARGET target, std::string_view id,
			std::string_view displayName, std::string& outStatus);
		bool_t Render_RenameControl(RENAME_TARGET target, std::string_view id,
			std::string_view displayName);
		std::vector<std::string> Collect_PatternDeleteReferences(std::string_view patternId, bool onlyBlocking = false) const;
		void Request_PatternDelete(std::string_view patternId);
		void Render_PatternDeleteContext(std::string_view patternId);
		void Render_PatternDeleteConfirmation();
		void Render_PresentationWorldAnchor(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence);
		void Render_PresentationAnchor(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence, bool_t effect);

		// resource tree의 leaf는 이 세션이 보관한 참조 snapshot 안의 조회 index다.
		// 일반 Action은 document/action index, sequence leaf는 bSequence에 따라 sequence 목록 index로 해석한다.
		struct RESOURCE_ACTION_LEAF final
		{
			std::size_t iDocument = 0u;
			std::size_t iAction = 0u;
			bool_t bSequence = false;
		};

		// 편집 명령의 공통 반영점이다. 부모 시간 동기화·문서 검증을 통과한 후보만 draft와 교체한다.
		// 실패는 기존 draft를 유지한다. 성공은 history·dirty·generation·선택/입력 동기화를 함께 갱신한다.
		bool_t Commit_Candidate(
			KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate,
			std::string_view successStatus,
			std::string& outStatus);
		bool_t Is_AppendAdmitted(
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& source,
			std::string& outStatus) const;
		bool_t Resolve_NativeClipMs(
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
			std::uint32_t& outNativeMs) const;
		bool_t Validate_SourceStart(
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
			std::string& outStatus) const;
		// Ctrl+D/Ctrl+V가 공유하는 복제 처리다. 선택의 소유 참조까지 모으고 새 ID를 할당한 뒤
		// region/group/hold/outcome을 재연결하며 필요한 Logic 정의를 복사한다. mode가 배치·이동 범위를 정한다.
		enum class TIMELINE_CLONE_MODE : std::uint8_t { DUPLICATE_AFTER, DUPLICATE_SAME_TIME, PASTE_APPEND };
		struct TIMELINE_CLONE_RESULT final
		{
			std::vector<std::string> NewStageIds, NewBoxIds;
			std::uint32_t iDetachedBoundaryBlends = 0u;
			// 빈 Pattern에 붙일 때 임시 수명 뒤가 아니라 0 ms부터 놓았다는 결과를 진단에 사용한다.
			bool_t bPlacedFromZero = false;
		};
		bool_t Clone_TimelineSelectionInto(KOUKU_SAYDON_COMPOSITION_DOCUMENT& candidate,
			const KOUKU_SAYDON_COMPOSITION_PATTERN& source, KOUKU_SAYDON_COMPOSITION_PATTERN& target,
			const std::vector<std::string>& stageIds, const std::vector<std::string>& occurrenceIds,
			TIMELINE_CLONE_MODE mode, TIMELINE_CLONE_RESULT& outResult, std::string& outStatus);
		// 복제한 새 행들을 선택하고 이전 단일 종류 선택을 비운다. Ctrl+D와 Ctrl+V가 같은 후처리를 쓴다.
		void Select_ClonedTimelineRows(const std::string& targetId, TIMELINE_CLONE_RESULT result);
		// 요청 준비는 값 snapshot만 만든다. MainApp의 Consume가 실제 재생을 실행한 뒤 preview state로 결과를 돌려준다.
		void Queue_AnimationPreview(
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence);
		bool_t Normalize_EndPolicyForWindow(
			KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
			std::string& outNote) const;
		void Queue_ModelResourcePreview(const COMPOSITION_ANIMATION_RESOURCE& resource);
		void Queue_SequencePreview(const COMPOSITION_ANIMATION_SEQUENCE_RESOURCE& sequence);
		void Queue_ResourcePatternPreview(KOUKU_SAYDON_COMPOSITION_PATTERN pattern,
			const std::string& targetAssetName);
		bool_t Has_StaleLocalPreviewSnapshot() const;
		bool_t Refresh_LocalPreviewSnapshot(std::uint32_t clockMs, bool_t paused, std::string& outStatus);
		void Stop_StaleLocalPreview(std::string_view status);
		bool_t Queue_BundleDocumentPreview(std::string_view bundleId, std::uint32_t clockMs, bool_t paused);
		bool_t Queue_PatternDocumentPreview(KOUKU_SAYDON_COMPOSITION_DOCUMENT previewDraft,
			std::string_view patternId, std::uint32_t startClockMs, std::string& outStatus, bool_t startPaused);
		void Queue_SequenceEffectPreview(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& source);
		bool_t Get_MapEffectPlacementSelection(KOUKU_MAP_EFFECT_PLACEMENT& outPlacement,
			bool_t allowWorldPositionPick) const;
		void Configure_SequenceEffectOccurrence(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource,
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence) const;
		bool_t Stage_PresentationSource(KOUKU_SAYDON_COMPOSITION_DOCUMENT& candidate,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& source,
			std::string& resourceId, std::string& outStatus);
		void Queue_SlotPreview(
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action,
			const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& stage,
			const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot);
		void Queue_ActionPreview(
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action,
			const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE* selectedStage);
		// 화면용 파생 view와 선택/입력 buffer를 재구성한다. 이 cache 자체를 저작 정의로 저장하지 않는다.
		void Rebuild_ResourceTree();
		void Normalize_Selection();
		void Synchronize_EditorFields();
		// 공용 shell이 위임하는 pane 내용이다. 표시 중 얻은 문서 포인터를 commit 뒤 다시 사용하지 않는다.
		void Render_Toolbar();
		void Render_PatternsAndResources();
		bool_t Render_AppendPatternButton();
		void Render_PatternTree(bool_t resourcePicker);
		void Rebuild_PatternTreeView();
		const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE* Find_DraftPresentationResource(
			const std::string& resourceId);
		void Render_PatternResources();
		bool_t Resolve_PatternResourceAppend(std::string& ownerId, std::uint32_t& startMs,
			std::uint32_t& durationMs, std::string& status) const;
		void Select_Hierarchy(KOUKU_PATTERN_SELECTION kind, std::string_view id);
		void Render_HierarchyDetails();
		bool_t Render_PatternReferenceControls(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		void Render_BundleTimeline();
		void Render_BundleTransport();
		bool_t Request_BundlePreview(std::uint32_t clockMs, bool_t paused = false);
		bool_t Create_Hierarchy(bool_t bundle);
		bool_t Link_BundlePattern(std::string_view patternId);
		void Render_BundleCommonDetails();
		void Render_ResourceTree();
		void Render_AnimationResources();
		void Rebuild_PatternChildRows(const KOUKU_SAYDON_COMPOSITION_PATTERN& parent);
		bool_t Render_CinematicResources();
		bool_t Append_CinematicGroup(const KOUKU_CINEMATIC_ANIMATION_GROUP& group);
		// 자원 종류별 UI는 미적용 입력을 유지하고 해당 typed 편집 명령으로만 문서에 반영한다.
		void Render_LogicResources();
		void Render_LogicDefinitionValues(
			const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic,
			std::string_view colliderPatternId = {},
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE* collider = nullptr, bool_t inlineDetail = false);
		// Logic 값 편집의 종류별 panel이다. 전달받은 미적용 draft만 바꾸며,
		// 검증·Apply와 문서 commit은 Render_LogicDefinitionValues의 공통 완료 경로가 소유한다.
		void Render_LogicPursuitValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft);
		void Render_LogicRouletteRegionValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft);
		void Render_LogicShowtimeVolleyValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft, const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern);
		void Render_LogicShowtimeValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft, const std::string_view colliderPatternId);
		void Render_LogicCrossDirectionValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft);
		void Render_LogicTrackTargetValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft, const std::string_view colliderPatternId);
		void Render_LogicKnockbackValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft);
		void Render_LogicFearValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft);
		void Render_LogicWorldMotionValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft);
		void Render_LogicContactMotionValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft);
		void Render_LogicObjectContactValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft);
		void Render_LogicAlbionCircleValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft);
		void Render_LogicMazeEntryValues(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic, KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft);
		void Render_LogicAirborneValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft, const std::string_view colliderPatternId);
		void Render_LogicJudgementValues(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic, KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft, const std::string_view colliderPatternId);
		void Render_LogicOutcomeValues(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic, KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft);
		void Render_LogicTriggerValues(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic, KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& draft, const std::string_view colliderPatternId, const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE* collider);
		void Render_LogicBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		bool_t Render_LogicOutcomeSlots(const std::string& patternId, const std::string& occurrenceId);
        bool_t Render_SummonPolicy(const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION& summon,
            const KOUKU_SAYDON_COMPOSITION_PATTERN* parent);
		void Render_SummonResources();
		void Render_SummonSpawnRows(
			std::vector<KOUKU_SAYDON_COMPOSITION_SUMMON_PATTERN_SPAWN>& spawns,
			const std::vector<const KOUKU_SAYDON_COMPOSITION_PATTERN*>& choices, bool& changed);
		void Render_SummonFormation(
			const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const std::string& occurrenceId,
			std::vector<KOUKU_SAYDON_COMPOSITION_SUMMON_PATTERN_SPAWN>& spawns,
			const std::vector<const KOUKU_SAYDON_COMPOSITION_PATTERN*>& choices, bool& changed);
		void Render_SummonBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		// Presentation 자원/배치·카메라·그룹 UI다. 모델·오디오·카메라의 실행 소유권은 외부 소비자에 남는다.
		std::uint32_t Resolve_SoundDurationMs(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource);
		void Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND kind);
		void Render_CameraAuthoring(std::string_view shotId, std::string_view patternId = {}, std::string_view occurrenceId = {});
		void Render_CameraWindow();
		bool_t Make_DedicatedCameraShot(std::string_view patternId, std::string_view occurrenceId, std::string& outStatus);
		bool_t Move_PresentationTimelineSelection(std::string_view patternId, const std::vector<std::string>& occurrenceIds,
			std::int64_t deltaMs, std::uint64_t generation, std::string& outStatus);
		bool_t Collect_EffectPlacements(std::string_view patternId,
			const std::vector<std::string>& occurrenceIds,
			std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE>& outBoxes, std::string& outStatus) const;
		bool_t Collect_ColliderPlacements(std::string_view patternId,
			const std::vector<std::string>& occurrenceIds,
			std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE>& outBoxes, std::string& outStatus) const;
		bool_t Set_PresentationSelectionGroup(std::string_view patternId,
			const std::vector<std::string>& occurrenceIds, KOUKU_SAYDON_PRESENTATION_KIND kind,
			bool_t grouped, std::string& outStatus);
		bool_t Render_SoundGroupDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		bool_t Render_EffectGroupDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		bool_t Render_ColliderGroupDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		void Render_ColliderBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		void Render_PresentationBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		bool_t Create_PresentationResource(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& source,
			std::string_view displayName, std::string& outStatus);
		bool_t Append_PresentationSource(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& source,
			std::string& outStatus);
		bool_t Append_PresentationBox(std::string_view resourceId, std::string& outStatus);
		bool_t Append_PresentationCandidate(KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate,
			std::string_view resourceId, std::string& outStatus);
		bool_t Delete_PresentationBox(std::string_view patternId, std::string_view occurrenceId, std::string& outStatus);
		void Queue_WorldBoxPreview(std::string_view patternId, std::string_view occurrenceId);
		void Queue_WorldObjectEdit(std::string_view patternId, std::string_view occurrenceId,
			std::string_view motionId = {}, bool focusAnimationClips = false);
		void Queue_SummonBoxPreview(std::string_view patternId, std::string_view occurrenceId, bool rebuild);
		void Queue_PresentationPreview(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE* occurrence = nullptr);
		// World/Scene Profile pane는 외부 inventory와 내부 occurrence를 연결한다.
		void Render_WorldResources();
		bool_t Render_WorldCompanionSelector(const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world);
		void Render_WorldBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		void Render_SceneProfileResources();
		void Render_SceneProfileBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		void Render_ResourcesWindow();
		// timeline은 선택·drag 후보를 모은 뒤 frame-local 행 참조가 끝난 시점에 편집 명령을 적용한다.
		bool_t Can_ReorderTimelineSelection(const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern) const;
		bool_t Render_TimelineControls(const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern,
		    const std::string& patternId, bool_t patternReady, std::uint32_t durationMs,
		    bool_t patternPreview, bool_t serverPlayback);
		void Collect_TimelineWorldClipRows(const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern);
		void Allocate_TimelineDisplayRows(std::uint64_t minimumBoxMs);
		void Ensure_TimelineLayout(const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern,
		    const std::string& patternId, std::uint64_t minimumBoxMs,
		    std::vector<std::uint32_t>& cameraBlendOutMs);
		void Render_Timeline();
		void Render_BingoParentTimeline();
		void Render_BingoParentDetails();
		void Select_BingoParentBox(int kind, const std::string& entryId = {});
		void Clear_TimelineSelection();
		void Process_TimelineClipboardShortcuts();
		void Select_TimelineBox(const std::string& stageId,
			const std::string& occurrenceId, bool_t toggle);
		// 로컬 preview와 Server 재생의 표시/요청을 구분한다. Server snapshot을 따라가는 선택은 입력 중 중단할 수 있다.
		void Render_Transport();
		void Render_ServerPlayButton(const char_t* label);
		void Synchronize_ServerPatternPlayback();
		bool_t Queue_CompleteSequenceItem(std::vector<std::string> patternIds,
			std::size_t index, std::string& outStatus);
		void Render_CompleteSequenceTransport();
		void Stop_Preview();
		// 상세 frame은 Pattern → 선택 lane → Stage → Animation 순서다. 편집 commit이 발생하면 즉시 끝낸다.
		bool Render_PatternProperties(const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern, const std::string& patternId);
		const KOUKU_SAYDON_COMPOSITION_STAGE* Render_StageProperties(
			const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern, const std::string& patternId);
		void Render_AnimationProperties(const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern,
			const KOUKU_SAYDON_COMPOSITION_STAGE* stage, const std::string& patternId);
		void Render_Details();
		void Render_ReloadConfirmation();
		// 외부 process의 관찰 handle·진단 파일을 소비한다. Render가 아닌 MainApp background tick에서도 호출한다.
		void Poll_PublishProcess();
        bool_t Start_DraftPlayPreparation(KOUKU_DRAFT_PLAY_REQUEST request,
            const KOUKU_SAYDON_COMPOSITION_DOCUMENT& snapshot, std::string& status);
        void Poll_DraftPlayProcess();

	private:
		// 세션의 source 경로·문서 baseline과 외부 자원 inventory. 모든 container는 이 객체가 소유한다.
		// m_Draft는 아래 편집 이력 구역에 있으며 m_Document의 last-good/저장 freshness와 수명이 다르다.
		// 같은 관문 선택과 다른 저장 경로를 혼동하지 않도록 문서 종류는 생성 시 고정한다. 관문 전환은 별도 선택 ID만 바꾼다.
		const bool m_bSequenceWorkspace;
		CKoukuSaydonCompositionDocument m_Document;
		KOUKU_SAYDON_ACTION_REFERENCE_SET m_ResourceReferences;
		std::vector<COMPOSITION_ANIMATION_RESOURCE> m_ModelResources;
		std::vector<COMPOSITION_ANIMATION_SEQUENCE_RESOURCE> m_SequenceResources;
		std::string m_strSequenceResourceStatus;
		std::string m_strSelectedSequenceResourceId;
		std::string m_strSelectedResourceTargetAsset;
		std::string m_strResourceStatus;
		char m_ResourceSearch[128]{};
		bool m_bResourcesOpen = true;
		bool m_bResourceRefreshRequested = true;
		// 표시용 트리와 leaf index는 inventory/검색 변경 시 다시 만든다. 원본 resource ID를 대신하지 않는다.
		COMPOSITION_RESOURCE_TREE_NODE m_ResourceTree;
		COMPOSITION_RESOURCE_TREE_NODE m_PhysicalResourceTree;
		std::vector<RESOURCE_ACTION_LEAF> m_ResourceLeaves;
		std::string m_strResourceTreeQuery;
		bool_t m_bResourceTreeDirty = true;
		std::string m_strSelectedResourceProfileId;
		std::uint32_t m_iSelectedResourceActionId = 0u;
		// 펼친 Action/Stage는 stable source ID로 기억해 검색·catalog 재구성 뒤에도 유지한다.
		std::string m_strExpandedResourceActionId;
		std::string m_strExpandedResourceStageId;
		// Resources 탭과 새 Logic 입력 buffer는 UI 상태다. Apply 전까지 저장 문서의 정의와 구분한다.
		int32_t m_iSelectedResourceCategory = 0;
		int32_t m_iNewLogicType = 0;
		int32_t m_iNewLogicBoxDurationMs = 1000;
		char_t m_NewLogicName[256]{};
		// Bingo 부모 UI는 Flow 저작값을 편집한다. 실행 중 자식 Pattern의 시계는 Server 소유다.
		int m_iBingoParentBox = 0; // 0 공통 board, 1 special, 2 normal entry 선택이다.
		std::string m_strBingoParentEntryId;
		int m_iBingoEntryWaitMs = 0;
		bool m_bBingoParentReturn = false;
		std::string m_strSelectedLogicId;
		std::string m_strSelectedLogicOccurrenceId;
		int32_t m_iLogicBoxStartMs = 0;
		int32_t m_iLogicBoxDurationMs = 1000;
		// 새 Summon box는 기본적으로 Pattern의 남은 수명을 사용한다. 개별 미적용 정책은 정의 ID별로 격리한다.
		char_t m_NewSummonName[256]{};
		std::string m_strSelectedSummonId;
        // pair의 첫 값은 입력을 시작한 정의, 둘째는 편집값이다. 외부 commit으로 정의가 바뀌면 둘 다 다시 읽는다.
        // source와 summon 정의가 달라질 때만 입력을 다시 맞춘다. 매 frame 문서값을 덮어쓰면 Apply 전 편집을 계속 잃기 때문이다.
        std::map<std::string, std::pair<KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION,
            KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION>> m_SummonPolicyDrafts;
		std::string m_strSelectedSummonOccurrenceId;
		int32_t m_iNewSummonBoxDurationMs = 1000;
		bool_t m_bNewSummonBoxToPatternEnd = true;
		int32_t m_iSummonBoxStartMs = 0;
		int32_t m_iSummonBoxDurationMs = 1000;
        std::string m_strSummonFormationOwnerId, m_strSummonFormationPatternId;
        float m_SummonFormationCenter[3]{};
        float m_fSummonFormationHalfWidth = 5.f, m_fSummonFormationSpacing = 2.f;
        float m_fSummonFormationYaw = 0.f;
		// 분류 node의 rows는 아래 view가 소유한 vector의 index다. tree와 rows를 같은 무효화 단위로 다룬다.
		struct EFFECT_RESOURCE_CATEGORY_NODE final
		{
			std::map<std::string, EFFECT_RESOURCE_CATEGORY_NODE> children;
			std::vector<std::size_t> rows;
			bool selected = false, kouku = false;
		};
		// 세션별 자원 표시 projection이다. 외부 refresh는 inventory를 바꾸고 draft 편집은 Created 목록을 무효화한다.
		// rows가 값을 소유하고 sourceIndices는 조회용이다. generation·검색 조건이 달라지면 함께 다시 만든다.
		// 검색할 때마다 원본 문서를 바꾸지 않고 표시용 행을 만든다. 원본·검색·generation 중 하나가 달라지면 이 파생 결과를 다시 만든다.
		struct EFFECT_RESOURCE_VIEW_CACHE final
		{
			bool valid = false, treeValid = false;
			int version = -1;
			std::uint64_t draftGeneration = 0u;
			std::string owner, search;
			std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> rows;
			std::unordered_map<std::string, std::size_t> sourceIndices;
			EFFECT_RESOURCE_CATEGORY_NODE categories;
			EFFECT_TOOL_KOUKU_EFFECT_TREE koukuTree;
			std::vector<std::size_t> koukuRows;
		};
		// Sound도 source inventory와 draft에서 만든 목록을 분리해 저장하고 같은 ID→index 조회 규칙을 쓴다.
		struct SOUND_RESOURCE_VIEW_CACHE final
		{
			bool valid = false;
			std::uint64_t draftGeneration = 0u;
			std::string search;
			std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> rows;
			std::unordered_map<std::string, std::size_t> sourceIndices;
		};
		SOUND_RESOURCE_VIEW_CACHE m_SoundSourceView, m_CreatedSoundView;
		// 원본 WAV 길이(ms) cache다. occurrence의 재생 창·trim 구간을 저장하는 곳이 아니다.
		std::unordered_map<std::string, std::uint32_t> m_SoundNativeDurations;
		char_t m_SoundResourceSearch[256]{};
		EFFECT_RESOURCE_VIEW_CACHE m_EffectSourceView, m_CreatedEffectView;
		std::unordered_map<std::string, std::string> m_EffectResourceOwners;
		std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> m_PresentationResourceInventory;
		std::string m_strPresentationResourceStatus;
		std::string m_strSelectedPresentationResourceId;
		std::string m_strSelectedPresentationSourceId;
		// camera 문서·검증은 Area의 기존 소유자에게 있다. 여기에는 열린 shot과 미적용 UI 선택만 둔다.
		bool_t m_bCameraWindowOpen = false;
		std::string m_strCameraWindowShotId;
		// 창을 연 CAMERA occurrence를 기억해 shot 시각과 Sequence 시각 사이의 변환에 사용한다.
		std::string m_strCameraWindowPatternId;
		std::string m_strCameraWindowOccurrenceId;
		// 여러 owner가 공유하는 shot은 이 ID로 명시 편집을 선택하기 전까지 읽기 전용이다.
		std::string m_strCameraSharedEditShotId;
		std::string m_strCameraKeyId;
		int32_t m_iCameraCaptureMs = 1000;
		char_t m_NewCameraActionName[129]{};
		int32_t m_iLightResourceCategory = 0;
		int32_t m_iEffectResourceVersion = 0;
		// Sequence Effect의 Play/Append가 같은 선택 모델과 시작 시계를 소비한다.
		int32_t m_iSequenceEffectStart = 0;
		std::string m_strEffectResourceOwner = "KoukuSaydon";
		bool_t m_bLocatePresentationSource = false;
		// Effect 원본 metadata는 asset ID로 조회한다. 사용자 폴더 분류와 실물 inventory를 별도 cache로 보관한다.
		std::unordered_map<std::string, CEffectAuthoringResourceTree::RESOURCE> m_EffectSourceInventory;
		std::unordered_map<std::string, CEffectAuthoringResourceTree::RESOURCE> m_EffectSourceOrganization;
        EFFECT_TOOL_KOUKU_EFFECT_TREE m_KoukuEffectResourceTree;
		std::string m_strEffectSourceMetadataStatus;
		std::string m_strExpandedV1EffectId;
		std::string m_strV1ElementResourceStatus;
		std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> m_V1ElementResources;
		char_t m_PresentationResourceSearch[128]{};
		// Rename 입력은 Apply 전까지 UI buffer다. 표시 이름을 바꿔도 stable ID와 참조는 바꾸지 않는다.
		std::string m_strRenameItemId;
		char_t m_RenameDisplayName[256]{};
		// 삭제 확인을 시작한 ID와 generation이다. 그동안 문서가 바뀌면 오래된 확인을 그대로 적용하지 않는다.
		std::string m_strDeletePatternId;
		std::uint64_t m_iDeletePatternGeneration = 0u;
		bool_t m_bPatternDeleteConfirmationRequested = false;
		std::string m_strSelectedPresentationOccurrenceId;
		char_t m_NewPresentationName[256]{};
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE m_PresentationBoxEdit;
		std::string m_strColliderSelectionTransformKey;
		float m_fColliderSelectionYaw = 0.f;
		std::string m_strEffectSelectionTransformKey;
		std::array<float, 3u> m_EffectSelectionRotationDegrees{};
		// picking 대기 동안 선택과 미적용 Box Detail을 함께 고정한다. 완료 결과는 token과 snapshot 일치를 확인한다.
		KOUKU_MAP_EFFECT_PLACEMENT_REQUEST m_MapEffectPlacementRequest;
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE m_MapEffectPlacementEditSnapshot;
		std::uint64_t m_iNextMapEffectPlacementToken = 0u;
		bool_t m_bMapEffectPlacementRequestPending = false;
		std::string m_strColliderExecutionType = "DURATION";
		std::string m_strColliderExecutionEditId;
		std::string m_strColliderLogicDefinitionId;
		int m_iColliderDetailSection = 0;
		KOUKU_COLLIDER_DAMAGE_SETTINGS m_ColliderDamageSettings;
		bool_t m_bColliderDamageMode = false;
		bool_t m_bColliderDetachDamage = false;
		bool_t m_bColliderDamageDirty = false;

		// pending은 MainApp에 아직 전달하지 않은 요청, staged는 draft에 아직 합치지 않은 geometry다.
		// 요청을 소비했다고 staged 편집을 버리지 않는다. Save·취소·history가 각각 반영/폐기 시점을 결정한다.
		std::optional<KOUKU_SUMMON_PLACEMENT_PREVIEW_REQUEST> m_PendingSummonPlacementPreview;
		std::vector<KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST> m_PendingPresentationGeometryPreviews;
		std::vector<KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST> m_StagedPresentationGeometry;
		std::string m_strPresentationGeometryPreviewPatternId;
		std::string m_strPresentationGeometryPreviewOccurrenceId;
		KOUKU_WORLD_OBJECT_EDIT_REQUEST m_PendingWorldObjectEditRequest;
		// 외부 World 편집 callback과 immutable 이력 snapshot이다. m_WorldAnimationEditsPending은 별도 저장 책임을 표시한다.
		std::function<bool_t(const KOUKU_WORLD_ANIMATION_EDIT&, std::string&)> m_EditWorldAnimation;
		std::function<bool_t(std::string&)> m_SaveWorldAnimation;
		std::function<WORLD_HISTORY_DOCUMENT(bool)> m_CaptureWorldAnimationHistory;
		std::function<bool(const WORLD_HISTORY_DOCUMENT&, const WORLD_HISTORY_DOCUMENT&, std::string&)> m_RestoreWorldAnimationHistory;
		WORLD_HISTORY_DOCUMENT m_HistoryWorldDocument;
		bool m_WorldAnimationEditsPending = false;
		std::string m_SelectedWorldAnimationId;
		int m_WorldAnimationDragMode = -1;
		KOUKU_PRESENTATION_PREVIEW_REQUEST m_PendingPresentationPreviewRequest;
		bool_t m_bPresentationPreviewRequestPending = false;
		bool_t m_bPresentationResourceRefreshRequested = true;
		bool_t m_bProductInventoryRefreshRequested = false;
		// World 문서에서 받은 값 목록과 선택 box 입력이다. 실제 arena 객체 포인터를 목록에 보관하지 않는다.
		std::vector<KOUKU_WORLD_SEQUENCE_RESOURCE> m_WorldSequenceResources;
		PRESENTATION_MODEL_RESOLVER m_PresentationModelResolver;
		std::function<bool_t(KOUKU_SAYDON_WORLD_PLACEMENT&, std::string&)> m_WorldPlacementResolver;
		bool_t Stage_NewWorldPlacement(std::string_view instanceId,
			std::optional<KOUKU_SAYDON_WORLD_PLACEMENT>& outPlacement, std::string& outStatus) const;
		std::string m_strWorldSequenceResourceStatus;
		char_t m_NewWorldName[256]{};
		std::string m_strNewWorldObjectId;
		char m_WorldObjectFilter[128]{};
		std::string m_strSelectedWorldId;
		std::string m_strSelectedWorldOccurrenceId;
		int32_t m_iWorldBoxStartMs = 0;
		int32_t m_iWorldBoxDurationMs = 1000;
		f32_t m_fWorldBoxPlaybackSpeed = 1.f;
		// rendering profile ID 목록과 선택 box의 시간/보간 입력이다. profile 품질 값의 소유권은 없다.
		std::vector<std::string> m_RenderingProfileIds;
		std::string m_strRenderingProfileResourceStatus;
		char_t m_NewSceneProfileName[256]{};
		std::string m_strNewSceneProfileRenderingId;
		std::string m_strSelectedSceneProfileId;
		std::string m_strSelectedSceneProfileOccurrenceId;
		bool_t m_bNewSceneProfileBoxToPatternEnd = true;
		int32_t m_iNewSceneProfileBoxDurationMs = 1000;
		int32_t m_iSceneProfileBoxStartMs = 0;
		int32_t m_iSceneProfileBoxDurationMs = 1000;
		int32_t m_iSceneProfileBoxBlendMs = 500;
		// Apply Values 전의 typed Logic 입력이다. 문서 draft와 분리해 미완성 숫자 입력을 유지한다.
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION m_LogicValueDraft;
		std::string m_strLogicValueDraftId;
		// 정의 stable ID별로 inline Outcome의 미적용 입력을 격리한다.
		// 여러 Outcome을 동시에 펼쳐도 입력이 섞이지 않도록 정의 ID별 사본을 둔다. Apply/복원 경로가 해당 사본을 비우거나 갱신한다.
		std::map<std::string, KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION> m_InlineLogicValueDrafts;
		// Collider 상세 UI는 별도 입력 복사본과 그 정의 ID를 보관해 Resources 입력과 구분한다.
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION m_ColliderLogicValueDraft;
		std::string m_strColliderLogicValueDraftId;
		// shell에서 선택한 보스 관문의 표시값이다. 실행 중 Server state와 독립된 UI 선택이다.
		std::string m_strBossVariantLabel;

		// 자식 Pattern을 확장한 preview 전용 문서다. 저작 source나 runtime 제품 문서의 두 번째 정본이 아니다.
		std::optional<KOUKU_SAYDON_COMPOSITION_DOCUMENT> m_ExpandedPatternPreview;
		std::string m_strSelectedPatternOccurrenceId;
		std::string m_strParentReturnPatternId;
		// Resources에서 고른 추가 대상은 현재 편집 Pattern·cursor·preview 선택을 바꾸지 않는다.
		std::string m_strAppendPatternId;
		bool_t m_bPatternResourceTabRequested = false;
		COMPOSITION_WORKBENCH_VIEW_REQUEST m_WorkbenchViewRequest;
		int32_t m_iPatternBoxStartMs = 0, m_iPatternBoxDurationMs = 1000;
		int32_t m_iParentLoopWindowMs = 30000;
		bool_t m_bPatternBoxRepeat = false;
		// Copy 당시 source와 참조 정의를 값으로 보관한다. 원본 삭제/수정 뒤 Paste의 복원·충돌 판단에 필요하다.
		struct TIMELINE_CLIPBOARD final
		{
			std::string strCompositionId, strAreaId, strActorProfileId, strGateId, strTargetBossPlacementId;
			// Copy 시점의 ID 발급 counter다. Paste가 목적지 counter를 올려 복원된 정의와 신규 ID가 겹치지 않게 한다.
			std::uint32_t iNextPresentationResourceOrdinal = 1u, iNextWorldOrdinal = 1u, iNextLogicOrdinal = 1u, iNextSummonOrdinal = 1u, iNextSceneProfileOrdinal = 1u;
			std::uint32_t iDetachedBoundaryBlends = 0u;
			// 미적용 배치까지 합친 source Pattern 복사본이다. 각 행의 Pattern 기준 시각을 유지한다.
			KOUKU_SAYDON_COMPOSITION_PATTERN Pattern;
			// Stage 자식 등 소유 관계까지 포함한 선택이다. Stage ID와 모든 lane의 occurrence ID를 구분한다.
			std::vector<std::string> SelectedStageIds, SelectedOccurrenceIds;
			// 선택이 참조하는 정의다. 목적지에서 사라진 정의는 복원하고 같은 ID의 달라진 정의는 충돌로 거부한다.
			std::vector<KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION> Logics;
			std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION> Worlds;
			std::vector<KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION> Summons;
			std::vector<KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION> SceneProfiles;
			std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> Resources;
			// 상태 표시용 수량이다. 순서는 Stage/Animation/Pattern/Logic/Summon/World/SceneProfile/Effect/Collider/기타 Presentation이다.
			std::array<std::size_t, 10u> Counts{};
		};
		// 공용 clipboard가 타입을 식별하는 payload다. 이 세션의 native snapshot을 다른 Composition 세션에 전달한다.
		struct COMPOSITION_NATIVE_TRANSFER final : COMPOSITION_EFFECT_TRANSFER
		{
			TIMELINE_CLIPBOARD clipboard;
			std::string_view Type() const noexcept override
			{
				return "kouku.timeline.v1";
			}
		};
		std::optional<TIMELINE_CLIPBOARD> m_TimelineClipboard;
        bool m_CompositionResourcesFocused = false;
		// 현재 편집 중인 문서의 유일한 mutable 원본이다. 후보 검증 뒤 교체하므로 row 포인터는 commit을 넘겨 보관하지 않는다.
		KOUKU_SAYDON_COMPOSITION_DOCUMENT m_Draft;
		// Undo 단위는 문서뿐 아니라 staged 배치·외부 World 문서·안정적인 선택 ID·cursor를 포함한다.
		// 문서는 shared_ptr<const ...>로 고정하고 Restore는 expected World snapshot과의 충돌도 검사한다.
		// 문서만 복원하면 외부 World와 선택이 서로 다른 시점을 가리킬 수 있어 한 편집 단위에 함께 묶는다.
		// Restore_EditHistory가 후보 검증과 외부 expected 사본 대조를 먼저 수행하고 성공 후 현재 세션을 교체한다.
		struct EDIT_HISTORY_STATE
		{
			std::shared_ptr<const KOUKU_SAYDON_COMPOSITION_DOCUMENT> document;
			std::vector<KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST> geometry;
			WORLD_HISTORY_DOCUMENT worldDocument;
			KOUKU_PATTERN_SELECTION patternSelection = KOUKU_PATTERN_SELECTION::GATE;
			std::string gate, folder, bundle, pattern, stage, animation, logic, logicBox, summon, summonBox;
			std::string world, worldBox, scene, sceneBox, presentation, presentationBox, patternBox, timelinePattern, cursorPattern;
			std::vector<std::string> stages, occurrences;
			std::uint32_t cursor = 0u;
            int bingoParentBox = 0, bingoEntryWaitMs = 0;
            std::string bingoParentEntryId;
            bool bingoParentReturn = false;
		};
		EDIT_HISTORY_STATE Capture_EditHistory(bool prepareWorld = false);
		void Begin_EditHistory(bool prepareWorld = false);
		bool Restore_EditHistory(const EDIT_HISTORY_STATE& state, const EDIT_HISTORY_STATE& expected, std::string& status);
		void Finish_EditHistory(bool force = false);
		void Render_EditHistoryButtons(const char* id);
		void Process_EditHistoryRequest();
		// history는 최대 16개 단위다. 같은 generation의 document snapshot을 공유해 중복 복사를 줄인다.
		// 보관 한도 16을 넘으면 가장 오래된 항목부터 제거한다. 이력은 되돌릴 메모리 상태를 보관하고 디스크 저장본은 m_Document가 관리한다.
		CEditorUndoHistory<EDIT_HISTORY_STATE> m_EditHistory{16u};
		std::shared_ptr<const KOUKU_SAYDON_COMPOSITION_DOCUMENT> m_HistoryDocument;
		std::uint64_t m_HistoryDocumentGeneration = UINT64_MAX;
		int m_PendingHistoryDirection = 0;
		bool m_HistoryRestoring = false;
		bool m_HistoryWindowFocused = false;
		// tree view는 표시 이름과 stable ID를 소유한다. 교체된 draft의 행 포인터를 cache하지 않는다.
		struct PATTERN_TREE_LEAF
		{
			std::string patternId, label;
			bool exists = false, matchesGate = false;
		};
		struct PATTERN_TREE_LEAVES
		{
			std::vector<PATTERN_TREE_LEAF> rows;
			bool hasMultilineLabel = false;
		};
		struct PATTERN_TREE_BUNDLE
		{
			std::string id, label;
			PATTERN_TREE_LEAVES leaves;
		};
		struct PATTERN_TREE_FOLDER
		{
			std::string id, label, timelinePatternId;
			std::vector<PATTERN_TREE_BUNDLE> bundles;
			PATTERN_TREE_LEAVES leaves;
		};
		struct PATTERN_TREE_GATE
		{
			std::string id, label;
			std::vector<PATTERN_TREE_FOLDER> folders;
			PATTERN_TREE_LEAVES leaves;
		};
		std::vector<PATTERN_TREE_GATE> m_PatternTreeView;
		std::uint64_t m_iPatternTreeViewGeneration = UINT64_MAX;
		// 조회 index는 m_Draft.PresentationResources의 현재 generation에서만 유효하다.
		std::unordered_map<std::string, std::size_t> m_DraftPresentationResourceIndices;
		std::uint64_t m_iDraftPresentationIndexGeneration = UINT64_MAX;
		// 계층 선택·생성 목적지와 Bundle 복귀/drag 상태다. stable ID로 다시 찾아 vector 교체를 견딘다.
		KOUKU_PATTERN_SELECTION m_ePatternSelection = KOUKU_PATTERN_SELECTION::GATE;
		std::string m_strSelectedGateId = "GATE1";
		std::string m_strModelViewProfile;
		std::string m_strSelectedFolderId;
		std::string m_strSelectedBundleId;
		std::string m_strCreateFolderId;
		std::string m_strCreateBundleId;
		std::string m_strCreateActorProfileId = "MN_RPCT_05";
		std::string m_strBundleReturnId;
		std::string m_strBundleDragMemberId;
		char m_NewFolderName[256]{};
		char m_NewBundleName[256]{};
		char m_HierarchyName[256]{};
		// Bundle 로컬 preview와 Server 재생 요청은 별개다. 각 Consume가 해당 pending 상태만 비운다.
		bool_t m_bBundlePreviewRequestPending = false;
		std::string m_strPendingBundlePreviewId;
		std::string m_strPendingBundleServerId;
		std::uint32_t m_iPendingBundleServerRevision = 0;
		std::string m_strSelectedPatternId;
		std::string m_strSelectedActorProfileId = "MN_RPCT_05";
		std::string m_strSelectedStageId;
		std::string m_strSelectedOccurrenceId;
		// 자원 선택과 실제 요청 snapshot을 분리해 클릭 이후 선택 변경이 대기 중 실행값을 바꾸지 않게 한다.
		KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE m_SelectedResource;
		KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE m_PendingPreviewRequest;
		KOUKU_SAYDON_COMPOSITION_PATTERN m_PendingPatternPreview;
		std::string m_strPendingPreviewTargetAsset;
		KOUKU_PREVIEW_STATE m_PreviewState;
		// preview를 만든 draft generation을 기억해 편집 뒤 Resume/Seek가 오래된 문서를 재생하지 않게 한다.
		std::string m_strLocalPreviewSnapshotId;
		std::uint64_t m_iLocalPreviewSnapshotGeneration = UINT64_MAX;
		bool_t m_bLocalPreviewSnapshotIsBundle = false;
		// 내부 순차 preview의 원본 순서·현재 index·소비자 승인 상태다. 외부 완주 callback 경로와 구분한다.
		std::vector<std::string> m_CompleteSequencePatternIds;
		std::function<bool_t(std::string_view, std::string&)> m_CompleteSequenceAdmission;
		std::size_t m_iCompleteSequenceIndex = 0u;
		bool_t m_bCompleteSequenceAdmitted = false;
		// transport는 마지막 예약 명령과 seek 시각이다. cursor는 화면 편집 위치로 실행 소비자의 시계와 별도다.
		KOUKU_PREVIEW_TRANSPORT m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
		bool_t m_bExplicitPreviewReset = false;
		std::uint32_t m_iPendingSeekMs = 0u;
		std::uint32_t m_iCursorMs = 0u;
		std::string m_strCursorPatternId;
		std::uint32_t m_iPendingPreviewStartMs = 0u;
		bool_t m_bPendingPreviewStartPaused = false;
		std::string m_strStatus;
		// 외부 Publish/임시 draft projection process의 관찰 상태다. handle은 이 객체가 닫지만
		// 소멸 시 process를 강제 종료하지 않는다. world generation은 결과 채택, draft generation은 선택 추적에 쓴다.
		std::filesystem::path m_PublishDiagnosticPath;
		void* m_hPublishProcess = nullptr;
        void* m_hDraftPlayProcess = nullptr;
        std::filesystem::path m_DraftPlayDirectory;
        KOUKU_DRAFT_PLAY_REQUEST m_DraftPlayRequest;
        bool_t m_bDraftPlayReady = false, m_bDraftPlayCancelled = false;
        std::uint64_t m_iServerFollowDraftGeneration = 0u;
		std::uint64_t m_iPublishStartedAtMilliseconds = 0u;
		std::string m_strPendingServerPlayPatternId;
		std::uint32_t m_iPendingServerPlaySourceRevision = 0u;
		// 서버 실행 root/live ID와 요청 번호로 선택을 따라간다. 사용자 편집 중에는 자동 추적을 보류한다.
		std::string m_strServerFollowRootPatternId;
		std::string m_strServerFollowLivePatternId;
		std::uint32_t m_iServerFollowPreviousRequest = 0u;
		bool_t m_bServerSelectionFollowSuspended = false;

        // 확장 자식의 표시용 행이다. runtime ID와 source ID를 함께 두어 원본 상세로 돌아갈 수 있게 한다.
        struct PATTERN_CHILD_ROW
        {
            std::string runtimeId, patternId, stageId, occurrenceId, label;
            std::uint32_t startMs = 0u, durationMs = 0u, color = 0u;
        };
        std::vector<PATTERN_CHILD_ROW> m_PatternChildRows;
        std::string m_strChildRowsPattern, m_strChildRowsStatus;
        std::uint64_t m_iChildRowsGeneration = UINT64_MAX;
        bool_t m_bShowPatternChildRows = true;
        std::vector<KOUKU_CINEMATIC_ANIMATION_GROUP> m_CinematicGroups;
		bool_t m_bCinematicCatalogLoaded = false;
		std::string m_strCinematicStatus;
		// layout 구현은 CPP에 숨겨 헤더에 ImGui 도형 의존성을 전파하지 않는다.
		// unique_ptr의 삭제는 완전한 cache 정의를 읽는 CPP 소멸자에서 수행한다.
		// 실제 정의는 Private/KoukuSaydonActionWorkbench_TimelineTypes.h에 있다. 공개 H에는 이름만 두고 생성·행 배치·소멸 CPP에서 완전한 타입을 읽는다.
		struct TIMELINE_LAYOUT_CACHE;
		std::unique_ptr<TIMELINE_LAYOUT_CACHE> m_pTimelineLayout;
		std::uint64_t m_iWorldSequenceResourceGeneration = 0u;
		std::uint64_t m_iDraftGeneration = 0u;
		// 드래그 시작값과 generation을 고정한다. 매 프레임 누적 오차나 다른 commit에 대한 적용을 피한다.
		std::uint32_t m_iDragOriginOffsetMs = 0u;
		std::uint32_t m_iDragOriginSourceMs = 0u;
		std::uint32_t m_iDragOriginPlayMs = 0u;
		int m_iTimelineDragMode = 0; // 0 이동, 1/2 앞뒤 trim, 3 저장된 Presentation group 이동이다.
		std::string m_strTimelineGroupDragOccurrenceId;
		std::vector<std::string> m_TimelineGroupDragOccurrenceIds;
		std::uint64_t m_iTimelineGroupDragGeneration = 0u;
		// 여러 lane의 선택은 소유 Pattern ID와 함께 보관한다. marquee 좌표는 ImGui 화면 좌표다.
		std::string m_strTimelineSelectionPatternId;
		std::vector<std::string> m_TimelineSelectedStageIds;
		std::vector<std::string> m_TimelineSelectedOccurrenceIds;
		bool_t m_bTimelineMarqueeActive = false;
		f32_t m_fTimelineMarqueeStartX = 0.f;
		f32_t m_fTimelineMarqueeStartY = 0.f;
		f32_t m_fPixelsPerSecond = 90.f;
		// ImGui의 입력 buffer다. typed 편집 함수에 전달해 검증받기 전에는 문서값과 다를 수 있다.
		char_t m_PatternName[256]{};
		char_t m_NewPatternName[256]{};
		int m_iNewPatternCategory = 1;
		int32_t m_iPatternDurationMs = 0;
		int32_t m_iNewStageDurationMs = 1000;
		int32_t m_iOccurrenceStartOffsetMs = 0;
		int32_t m_iOccurrenceSourceStartMs = 0;
        int32_t m_iOccurrenceSourceEndMs = 0;
		int32_t m_iOccurrencePlayMs = 1;
		f32_t m_fOccurrencePlayRate = 1.f;
		int32_t m_iOccurrenceEndPolicy = 0;
		int32_t m_iOccurrenceBlendInMs = 0;
		int32_t m_iSelectedNewStageKind = 0;
		// 독립 창 상태, 공유 pane 활성 frame, 초기 로드, dirty 상태를 구분한다. 숨김은 draft 폐기가 아니다.
		bool_t m_bOpen = true;
		bool_t m_bSharedWorkspaceActive = false;
		bool_t m_bTimelineClipboardFocusThisFrame = false;
		int32_t m_iTimelineClipboardShortcutFrame = -1;
		bool_t m_bLoadAttempted = false;
		bool_t m_bHasDraft = false;
		bool_t m_bDirty = false;
		bool_t m_bTimelineMaximized = false;
		bool_t m_bFitRequested = true;
		bool_t m_bReloadConfirmationRequested = false;
		bool_t m_bHasSelectedResource = false;
		bool_t m_bPreviewRequestPending = false;
		bool_t m_bPatternPreviewRequestPending = false;
		// 다음 시작 요청의 소비 결과는 같은 진단 문자열이 반복돼도 다시 표시한다.
		bool_t m_bPreviewResultStatusPending = false;
		bool_t m_bServerPlayRequestPending = false;
		bool_t m_bServerPlayPreparationPending = false;
		bool_t m_bServerPlayCancelRequested = false;
	};
}
