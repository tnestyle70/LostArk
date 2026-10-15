#pragma once

// Effect 변환·추종·종료 정책과 발탄 cue의 크기 정책 타입을 뷰에 보관한다.
#include "ValtanPatternEffectCueDocument.h"
// stage가 소유하는 장면 프로필·광원 occurrence 타입을 기존 애니메이션 문서 계약과 공유한다.
#include "AnimationSkillBindingDocument.h"
// Client 공통 선언과 WinSock2→Windows의 선행 포함 순서를 유지한다.
#include "Client_Defines.h"
// bool_t·f32_t·float3_t와 namespace 매크로 등 공통 엔진 타입을 사용한다.
#include "Engine_Defines.h"
// 잡기 stage의 왼손 부착 오프셋을 PLAYER_HAND_GRIP_LOCAL_OFFSET 값으로 보관한다.
#include "PlayerHandGripTransform.h"
// 추가 접촉 판정의 Shared ATTACK_HIT_TEMPLATE 값을 읽기·직렬화 경계에 그대로 전달한다.
#include "Gameplay/AttackHitTemplate.h"

// 이동 범위와 착지 위치처럼 축 개수가 고정된 값을 보관한다.
#include <array>
// 현재 트리의 행 개수와 임시 조회 index에 size_t를 사용한다.
#include <cstddef>
// 저작 원본 및 Effect 문서 경로를 OS 경로 값으로 전달한다.
#include <filesystem>
// 유효한 vector index와 구분할 INVALID_INDEX 값을 정한다.
#include <limits>
// 미지정 상태와 실제 값을 구분해야 하는 이동·부착·분기·응답 필드를 보관한다.
#include <optional>
// 결합 결과가 stable ID와 표시 이름·진단 문자열을 소유한다.
#include <string>
// 게시 명령 같은 고정 문자열을 소유 복사 없이 참조한다.
#include <string_view>
// Effect asset ID별 원본 clip 목록과 표시 이름을 찾는 색인을 보관한다.
#include <unordered_map>
// 저작 순서가 의미 있는 stage·clip·분기와 결합 결과 목록을 소유한다.
#include <vector>

NS_BEGIN(Client)

class DATA_JSON_VALUE;
// PatternTree와 EncounterPatternReference의 선택적 attackContacts를 Shared 값으로 읽는다.
// 누락은 목록을 비우고 성공하며 잘못된 명시 값은 기존 contacts를 보존한다.
bool Parse_ValtanStageAttackContacts(const DATA_JSON_VALUE* value,
	std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE>& contacts);
// BalanceTool이 contact 배열을 저장할 때 사용하는 JSON 문자열 생성이다.
// 입력 값의 의미 검증이나 디스크 저장은 수행하지 않으므로 호출자가 검증된 목록을 전달해야 한다.
std::string Serialize_ValtanStageAttackContacts(
	const std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE>& contacts);

// stage에서 편집할 Effect 문서를 찾은 근거다. Product cue의 ID를 우선 사용한다.
// 일치하는 근거 binding은 같은 행에 기록하고, cue가 없는 경우에만 이름 규칙으로 찾는다.
enum class VALTAN_STAGE_EFFECT_ORIGIN : uint8_t
{
	PRODUCT_CUE,
	PATTERN_EFFECT_BINDING,
	NAMING_RULE,
	END
};

struct VALTAN_STAGE_EFFECT_VIEW final
{
	std::string strEffectAssetId;
	std::filesystem::path DocumentPath;
	VALTAN_STAGE_EFFECT_ORIGIN eOrigin = VALTAN_STAGE_EFFECT_ORIGIN::END;
	// 문서를 실제로 열거나 재생한 뒤 채우는 결과다. 트리 행 표시만으로 Effect JSON을 해석하지 않는다.
	uint32_t iElementCount = 0u;
	bool_t bParsed = false;
	bool_t bPatternEffectBinding = false;
};

struct VALTAN_PRODUCT_EFFECT_CUE_VIEW final
{
	std::string strBindingId;
	std::string strOccurrenceId;
	std::string strPatternId;
	std::string strStageId;
	std::string strActionId;
	std::string strClipOccurrenceId;
	std::string strEffectAssetId;
	std::string strV1EffectAssetId;
	std::string strAnchorSlotId;
	EFFECT_TRANSFORM_DESC LocalTransform{};
	EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
	EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
	std::string strFollowPolicy;
	std::string strStopPolicy;
	std::string strRepeatPolicy;
	VALTAN_PATTERN_EFFECT_SCALE_POLICY eScalePolicy =
		VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
	std::string strScalePolicy;
	float3_t vWorldScale{ 1.f, 1.f, 1.f };
	bool_t bHasExplicitScalePolicy = false;
	// CLIP_OCCURRENCE cue의 시각은 원본 clip 내부 시간이다.
	// stage 시계 cue는 본체 애니메이션과 독립적으로 iStageOffsetMs를 사용한다.
	bool_t bUsesStageClock = false;
	uint32_t iStageOffsetMs = 0u;
	uint32_t iSourceStartMs = 0u;
	uint32_t iSourceEndMs = 0u;
	// Effect 리소스 안에서 재생을 시작할 시간이다. 생략 여부를 별도로 보관해 기존 시작점 해석을 유지한다.
	uint32_t iPlaybackOffsetMs = 0u;
	bool_t bHasPlaybackOffset = false;
	uint32_t iStageDurationMs = 0u;
	bool_t bHasSourceEnd = false;
};

// Server 전투 오브젝트가 소유하는 연출 이벤트와 오브젝트 내부 ms 시각이다.
// 보스 본체 cue와 구분해, 움직이는 전투 오브젝트의 연출을 생성 stage 아래에서 찾게 한다.
struct VALTAN_COMBAT_OBJECT_PRESENTATION_EVENT_VIEW final
{
	std::string strPresentationEventId;
	uint32_t iAtMs = 0u;
};

// Server 전투 오브젝트의 실제 hit ID와 도형 및 피격 정책을 함께 보관한다.
// 도구가 표시용 도형만으로 원본 계약을 대체하지 않도록 소유 패턴과 연결하는 값이다.
struct VALTAN_COMBAT_OBJECT_HIT_VIEW final
{
	std::string strHitId;
	std::string strTriggerKind;
	uint32_t iAtMs = 0u;
	uint32_t iRepeatCount = 1u;
	uint32_t iRepeatIntervalMs = 0u;
	std::string strHitShape;
	f32_t fInnerRadiusM = 0.f;
	f32_t fOuterRadiusM = 0.f;
	std::string strServerDamageProfileId;
	f32_t fPushRangeM = 0.f;
	uint32_t iPushMs = 0u;
	bool_t bKnockdown = false;
	uint32_t iDownMs = 0u;
	bool operator==(const VALTAN_COMBAT_OBJECT_HIT_VIEW&) const = default;
};

// 소유 보스의 action hit 이후 오브젝트를 준비시키는 연결이다.
// 지연은 ms이며 연출 이벤트 ID와 gameplay trigger action ID를 구분해 보관한다.
struct VALTAN_COMBAT_OBJECT_OWNER_HIT_CHAIN_VIEW final
{
	std::string strTriggerActionId;
	uint32_t iDelayMs = 0u;
	std::string strArmedPresentationEventId;
};

struct VALTAN_COMBAT_OBJECT_EFFECT_VIEW final
{
	std::string strPreparationEventId;
	std::string strCombatObjectArchetypeId;
	std::string strClientVisualId;
	std::string strEffectAssetId;
	// 제품 연출은 V1 편집 fallback 대신 하나의 Effect V2 그룹을 사용할 수 있다.
	// Composition에는 그룹의 stable ID를 보관하고 기존 로컬 미리보기용 strEffectAssetId도 유지한다.
	std::string strEffectV2GroupId;
	std::string strTrigger;
	uint32_t iSpawnValue = 0u;
	// 일제 생성의 배치 정책과 도형을 보존한다. 표시용 생성 개수만으로 배치를 복원하지 않는다.
	// 각 미리보기 경로는 자신이 지원하는 배치 정책을 별도로 판정해야 한다.
	std::string strVolleyPolicy;
	std::string strVolleyLayout;
	f32_t fVolleyRadiusM = 0.f;
	f32_t fVolleyStartAngleDegrees = 0.f;
	f32_t fVolleyAngleStepDegrees = 0.f;
	bool_t bVolleyAllowOverlap = false;
	uint32_t iVolleyMaximumTotalObjects = 0u;
	uint32_t iSpawnScheduleCount = 0u;
	uint32_t iFirstSpawnOffsetMs = 0u;
	uint32_t iSpawnIntervalMs = 0u;
	std::string strArenaRandomKind;
	std::string strArenaAnchor;
	uint32_t iArenaRandomCount = 0u;
	f32_t fArenaRandomRadiusM = 0.f;
	f32_t fArenaHeightToleranceM = 0.f;
	// 전투 오브젝트의 종류, 이동과 수명이다. 생성 stage와 독립된 오브젝트 시계로 해석한다.
	// 이 값의 원본은 전투 오브젝트 문서이며 stage 길이만 바꿔 함께 수정할 수 없다.
	std::string strKind;
	std::string strOriginPolicy;
	std::string strDirectionPolicy;
	f32_t fSpeedMps = 0.f;
	f32_t fMaximumDistanceM = 0.f;
	uint32_t iLifetimeMs = 0u;
	f32_t fCoverRadiusM = 0.f;
	std::optional<VALTAN_COMBAT_OBJECT_OWNER_HIT_CHAIN_VIEW> OwnerHitChain;
	// 전투 오브젝트 내부 Server hit의 identity와 ms 시각이다.
	// 생성 유무뿐 아니라 실제 hit와 사운드 cue의 연결도 도구에서 대조할 수 있게 한다.
	std::vector<std::string> HitIds;
	std::vector<uint32_t> HitOffsetsMs;
	std::vector<VALTAN_COMBAT_OBJECT_HIT_VIEW> Hits;
	// 종료 연출 등의 이벤트 시계는 Server hit와 별개다.
	// 여기에는 오브젝트가 소유하는 이벤트 ID와 시각을 남기며 실제 시각 리소스는 catalog가 연결한다.
	std::vector<VALTAN_COMBAT_OBJECT_PRESENTATION_EVENT_VIEW>
		PresentationEvents;
	bool_t bHasHitAnchor = false;
	std::string strHitAnchorKind = "BOSS_CURRENT";
	f32_t fHitAnchorForwardOffsetM = 0.f;
	f32_t fHitAnchorRightOffsetM = 0.f;
	f32_t fHitAnchorYawOffsetDegrees = 0.f;
	bool_t bHasHitActivation = false;
	uint32_t iHitActivationStartMs = 0u;
	uint32_t iHitActivationLifetimeMs = 0u;
};

// 애니메이션 저작자가 정한 순서와 stable clipOccurrenceId를 보존한다.
// 표시 순번은 저장 identity가 아니며 다른 문서와의 연결에는 clipOccurrenceId를 사용한다.
struct VALTAN_CLIP_OCCURRENCE_VIEW final
{
	std::string strClipOccurrenceId;
	std::string strClipName;
	std::string strMappingBasis;
	uint32_t iSourceStartMs = 0u;
	uint32_t iPlayMs = 0u;
	// 이 occurrence에 배분한 실제 재생 시간 예산이다. 단위는 ms다.
	// 원본 clip 내부 절단 시간과 구분해, 반복 clip의 종료와 타임라인 탐색이 stage 경계를 따르게 한다.
	uint32_t iAuthoringWallMs = 0u;
	f32_t fPlayRate = 1.f;
	bool_t bLoop = false;
	std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW> ProductCues;
};

// Effect와 Pattern 탐색기가 공유하는 원본 애니메이션 색인이다. Effect asset ID로 occurrence 목록을 찾는다.
using VALTAN_FULL_RESTORE_CLIP_INDEX = std::unordered_map<std::string,
	std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>>;

// stage의 이동 정책과 원본 파라미터를 보관한다. 도구 뷰 자체가 위치를 적분하지 않는다.
struct VALTAN_STAGE_MOTION_VIEW final
{
	std::string strKind;
	uint32_t iRetargetDelayMs = 0u;
	f32_t fSpeedMps = 0.f;
	f32_t fDistance = 0.f;
	uint32_t iCornerIndex = 0u;
	std::array<f32_t, 2u> HalfExtentsM{};
};

struct VALTAN_STAGE_ACTION_VIEW final
{
	std::string strTrigger;
	std::string strKind;
	std::string strTargetId;
	f32_t fValue = 0.f;
	// RELEASE_GRABBED_PLAYERS는 일반 fValue 대신 전용 release 정책과 이동 값을 사용한다.
	std::string strReleaseMode;
	f32_t fSpeedMps = 0.f;
	uint32_t iDurationMs = 0u;
	// ARENA_EJECTION에서만 Server가 선택한 보스 방향에 더하는 각도다. 단위는 degree다.
	// 0은 기존 뒤쪽 발사를 유지하며 HOLD와 OPPOSITE_KNOCKBACK에는 0만 허용한다.
	f32_t fYawOffsetDegrees = 0.f;
};

// Server stage가 카메라 cue를 호출하는 연결이다. 호출 시점과 길이는 이 행이 소유한다.
// 실제 shot의 공간 구성은 별도 cinematic cue 문서가 소유하므로 여기에 복제하지 않는다.
struct VALTAN_CAMERA_INVOCATION_VIEW final
{
	std::string strCameraInvocationId;
	std::string strCameraCueId;
	std::string strTrigger;
	uint32_t iStartOffsetMs = 0u;
	std::string strDurationPolicy;
	uint32_t iDurationMs = 0u;
};

// Server로 투영할 분기 순서를 보존한다. nextActionId가 없으면 현재 패턴 경로가 끝난다.
// nextPatternId가 있으면 다른 패턴으로 이어지는 분기이며 로컬 action ID와 구분한다.
struct VALTAN_STAGE_BRANCH_VIEW final
{
	std::string strOutcome;
	std::optional<std::string> strNextActionId;
	std::optional<std::string> strNextPatternId;
};

// 일반 hit에 추가되는 counter 판정 정책이다. 도구의 결합 결과에도 typed 값을 유지한다.
// Product 대조 과정에서 Server가 소비하는 특수 판정이 사라지지 않게 한다.
struct VALTAN_COUNTER_PROXY_VIEW final
{
	// Product v4는 도형 종류를 명시한다. strSpace는 기존 BOSS_LOCAL 원형 편집기의 호환 표현이다.
	std::string strKind = "BOSS_LOCAL_CIRCLE";
	std::string strSpace;
	f32_t fForwardOffsetM = 0.f;
	f32_t fRightOffsetM = 0.f;
	f32_t fRadiusM = 0.f;
	f32_t fArcDegrees = 0.f;
};

struct VALTAN_BOSS_RESPONSE_VIEW final
{
	std::string strKind;
	uint32_t iThreshold = 0u;
	bool operator==(const VALTAN_BOSS_RESPONSE_VIEW&) const = default;
};

// 한 stage를 편집하고 미리보기 위해 gameplay와 presentation 문서를 결합한 값이다.
// 재생 clip, ms 길이, Server 판정과 Effect를 actionId 및 각 stable 참조 ID로 연결한다.
// 이 구조체가 Server의 전투 상태를 실행하거나 소유하는 것은 아니다.
struct VALTAN_STAGE_VIEW final
{
	std::string strStageId;
	std::string strSequenceRole;
	std::string strActionId;
	std::string strStageKind;
	uint32_t iDurationMs = 0u;
	// 이 stage에만 적용되는 선택적 Server 저작 높이 오프셋이다. 단위는 m다.
	f32_t fVerticalOffsetM = 0.f;
	uint32_t iAuthoringRepeatCount = 0u;
	std::string strAnimationEndPolicy;
	// 새 clip이나 fallback 애니메이션을 고르지 않고 현재 본체 pose를 유지하는 연출 계약이다.
	bool_t bSuppressAnimation = false;
	// 본체 렌더 제출을 숨기는 선택적 stage 시계 구간 [fromMs, toMs)다.
	// Effect와 카메라 재생을 중단하는 값이나 Server gameplay의 가시성 상태가 아니다.
	bool_t bHasBodyHiddenWindow = false;
	uint32_t iBodyHiddenFromMs = 0u;
	uint32_t iBodyHiddenToMs = 0u;

	// 피해 판정은 Server가 수행한다. 도구는 이 값을 복사해 Effect의 시각적 범위와 판정 시간을 대조한다.
	std::string strHitShape;
	f32_t fHitOuterRadius = 0.f;
	f32_t fHitInnerRadius = 0.f;
	f32_t fHitAngleDegrees = 0.f;
	f32_t fHitLength = 0.f;
	f32_t fHitHalfWidth = 0.f;
	uint32_t iHitCount = 0u;
	uint32_t iHitIntervalMs = 0u;
	uint32_t iHitDelayMs = 0u;
	// stage 시작 기준의 순서 있는 접촉 시각이다. 비어 있으면 iHitDelayMs + k * iHitIntervalMs를 사용한다.
	std::vector<uint32_t> HitOffsetsMs;
	std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> AttackContacts;
	// 선택적 Server 판정 원점과 활성 구간을 도구에 보존한다.
	// BOSS_CURRENT는 현재 보스 기준이고 STAGE_ORIGIN은 활성 구간에서 저장한 기준 transform을 사용한다.
	bool_t bHasHitAnchor = false;
	std::string strHitAnchorKind = "BOSS_CURRENT";
	f32_t fHitAnchorForwardOffsetM = 0.f;
	f32_t fHitAnchorRightOffsetM = 0.f;
	f32_t fHitAnchorYawOffsetDegrees = 0.f;
	bool_t bHasHitActivation = false;
	uint32_t iHitActivationStartMs = 0u;
	uint32_t iHitActivationLifetimeMs = 0u;
	std::string strServerDamageProfileId;
	// 필드가 없으면 일반 DAMAGE 반응이다. 잡기 stage는 피격 반응과 왼손 부착 슬롯을 한 계약으로 지정한다.
	std::string strPlayerResponse = "DAMAGE";
	std::string strAttachmentSlot = "NONE";
	std::optional<PLAYER_HAND_GRIP_LOCAL_OFFSET> GripLocalOffset;
	std::string strPartDamagePolicy = "NORMAL";
	std::optional<VALTAN_COUNTER_PROXY_VIEW> CounterProxy;
	std::optional<VALTAN_BOSS_RESPONSE_VIEW> BossResponse;
	f32_t fPushRangeM = 0.f;
	uint32_t iPushMs = 0u;
	bool_t bKnockdown = false;
	uint32_t iDownMs = 0u;
	// 비어 있으면 Pattern 조준 정책을 상속하며 값이 있으면 stage 전용 정책이다.
	std::string strAimTargetPolicy;
	bool_t bHasAimEnd = false;
	uint32_t iAimEndMs = 0u;
	bool_t bHasAimResponseScale = false;
	f32_t fAimResponseScale = 1.f;
	std::optional<VALTAN_STAGE_MOTION_VIEW> Motion;
	std::vector<VALTAN_STAGE_ACTION_VIEW> Actions;
	std::vector<VALTAN_STAGE_BRANCH_VIEW> Branches;
	std::vector<VALTAN_CAMERA_INVOCATION_VIEW> CameraInvocations;
	std::vector<BOSS_STAGE_SCENE_PROFILE_OCCURRENCE> SceneProfileOccurrences;
	std::vector<BOSS_STAGE_LIGHT_OCCURRENCE> LightOccurrences;

	// 저작 순서와 stable occurrence ID를 함께 보관한다.
	// 기존 이름 기반 호출자와 읽기 전용 v1 입력을 위해 아래 호환 뷰도 함께 채운다.
	std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> ClipOccurrences;
	std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW> ProductCues;
	// 정확한 범위로 연결한 V2 원본 binding의 존재 여부다. 합성한 V1 cue의 존재 여부가 아니다.
	bool_t bHasStageEffectV2Bindings = false;
	std::vector<VALTAN_COMBAT_OBJECT_EFFECT_VIEW> CombatObjectEffects;
	std::vector<std::string> RuntimeClipNames;
	std::string strRuntimeClipName;
	std::optional<VALTAN_PRODUCT_EFFECT_CUE_VIEW> ProductCue;
	std::vector<VALTAN_STAGE_EFFECT_VIEW> Effects;
	// 트리의 IndependentEffects 항목을 가리키는 stable ID다.
	// 패턴 안에서 같은 Effect를 중복 소유하거나 직접 편집하는 행으로 사용하지 않는다.
	std::vector<std::string> IndependentEffectIds;

	bool_t Has_Effect() const
	{
		return !Effects.empty() || !CombatObjectEffects.empty();
	}
	bool_t Has_ClipBinding() const
	{
		return !ClipOccurrences.empty();
	}
	bool_t Has_ProductCue() const
	{
		return !ProductCues.empty();
	}
	bool_t Has_HitShape() const
	{
		return !strHitShape.empty() && "NONE" != strHitShape;
	}
};

// 패턴 전체의 이동 연출과 Server 시간 경계를 연결한다.
// LandingPosition과 높이는 m, takeoff/travel 구간은 ms이며 stage ID로 이동 구간을 찾는다.
struct VALTAN_PATTERN_SERVER_MOTION_VIEW final
{
	std::string strKind;
	std::string strAnchorId;
	std::array<f32_t, 3u> LandingPosition{};
	f32_t fApexHeight = 0.f;
	std::string strTravelStageId;
	uint32_t iTakeoffStartMs = 0u;
	uint32_t iTakeoffEndMs = 0u;
	uint32_t iTravelStartMs = 0u;
	uint32_t iTravelEndMs = 0u;
	bool_t bMoveToAnchorBeforeTakeoff = false;
};

// 독립된 유령과 포탈 생명주기를 Product와 대조하기 위한 gameplay 값이다.
struct VALTAN_PATTERN_FINALE_VIEW final
{
	std::string strKind;
	std::string strGhostArchetypeId;
	std::vector<std::string> GhostPatternIds;
	std::array<f32_t, 2u> SpawnHalfExtentsM{};
	uint32_t iMaximumActiveGhosts = 0u;
	uint32_t iAuxiliarySpawnIntervalMs = 0u;
	uint32_t iPortalSpawnIntervalMs = 7900u;
	bool operator==(const VALTAN_PATTERN_FINALE_VIEW&) const = default;
};

struct VALTAN_PRESENTATION_SOURCE_VIEW final
{
	uint32_t iSourceActionId = 0u;
	uint32_t iSequenceIndex = 0u;
	std::string strRole;
};

struct VALTAN_PATTERN_REACTION_VIEW final
{
	std::string strTriggerKind;
	std::string strStageId;
};

struct VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW final
{
	std::string strPatternId;
	std::string strStageId;
	std::string strTriggerKind;
};

// 선택 집합 내부의 후보 순서와 가중치다.
// 가중치를 다른 체력 구간이나 Pattern 전체의 호환 가중치로 복사하지 않는다.
struct VALTAN_SELECTION_CANDIDATE_VIEW final
{
	std::string strPatternId;
	uint32_t iWeight = 1u;
	bool_t bEnabled = true;
};

struct VALTAN_SELECTION_SET_VIEW final
{
	std::string strSelectionSetId;
	std::string strMode;
	std::vector<VALTAN_SELECTION_CANDIDATE_VIEW> Candidates;
	// 순서 반복 모드는 같은 패턴 ID가 여러 번 나와도 각 순번을 그대로 보존한다.
	std::vector<std::string> PatternIds;
};

// 체력 구간과 gameplay phase를 선택 집합의 stable ID에 연결한다.
// maximum은 포함하고 minimum은 제외하는 경계다.
struct VALTAN_SELECTION_WINDOW_VIEW final
{
	std::string strWindowId;
	uint32_t iGameplayPhase = 0u;
	uint32_t iMaximumHealthBarInclusive = 0u;
	uint32_t iMinimumHealthBarExclusive = 0u;
	std::string strSelectionSetId;
	std::string strCompatibilityRotationId;
};

struct VALTAN_MECHANIC_VIEW final
{
	std::string strMechanicId;
	std::string strPatternId;
	std::string strTriggerKind;
	uint32_t iHealthBar = 0u;
	// 같은 iHealthBar에서 발생하는 기믹의 동률 순서다. 체력 내림차순이 우선이다.
	uint32_t iTriggerOrder = 0u;
	bool_t bOncePerEncounter = false;
	std::string strFailurePolicy;
};

// 애니메이션 중심으로 만든 패턴의 수동 Server 시험 메타데이터다.
// 시험 목록 등록만으로 일반 rotation이나 체력 기믹 후보가 되지는 않는다.
struct VALTAN_MANUAL_AUDITION_VIEW final
{
	std::string strPatternId;
	std::string strSourceChainId;
	uint32_t iAuthoringPhase = 0u;
	std::string strAdmissionState;
};

// 저작 selection set으로 연결되지 않은 기존 Product rotation을 보관하는 호환 뷰다.
// 현재 읽은 행을 표시하기 위한 값이며 이 구조체만으로 저작 후보로 승격하지 않는다.
struct VALTAN_LEGACY_ROTATION_VIEW final
{
	std::string strRotationId;
	std::string strSelectionMode;
	uint32_t iFromHealthBar = 0u;
	uint32_t iToHealthBar = 0u;
	std::vector<std::string> PatternIds;
};

struct VALTAN_NORMAL_SELECTION_RANGE_VIEW final
{
	std::string strRotationId;
	uint32_t iFromHealthBar = 0u;
	uint32_t iToHealthBar = 0u;
};

struct VALTAN_NORMAL_SELECTION_VIEW final
{
	std::string strSelectionMode;
	std::vector<VALTAN_NORMAL_SELECTION_RANGE_VIEW> Ranges;
	std::vector<std::string> PatternIds;
};

// 저작 identity를 기존 Encounter stage 및 Product 애니메이션과 연결하는 참조용 반응이다.
// 이 연결만으로 기존 소유 패턴을 gameplay 선택 후보에 추가하지 않는다.
struct VALTAN_REACTION_ANIMATION_ACTION_VIEW final
{
	std::string strActionId;
	std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> ClipOccurrences;
};

struct VALTAN_COUNTER_REACTION_LAYER_VIEW final
{
	std::string strReactionLayerId;
	std::string strAdmissionScope;
	std::string strOwnerPatternId;
	std::string strOwnerStageId;
	VALTAN_REACTION_ANIMATION_ACTION_VIEW Window;
	VALTAN_REACTION_ANIMATION_ACTION_VIEW Success;
	VALTAN_REACTION_ANIMATION_ACTION_VIEW Failure;
};

struct VALTAN_PATTERN_VIEW final
{
	std::string strPatternId;
	std::string strCategory;
	uint32_t iMinimumPhase = 0u;
	uint32_t iMaximumPhase = 0u;
	std::string strTargetPolicy;
	std::string strAimPolicy;
	std::string strDisplayName;
	std::string strActionId;
	// 엄격한 결합에서는 원본 진입 action을 첫 Product stage와 대조한 뒤 채운다.
	// 원본 전용 로드는 첫 저작 stage를 사용하며 legacy Product 전용 행은 비워 둔다.
	std::string strEntryActionId;
	std::vector<uint32_t> SourceActionIds;
	std::string strSelectionMode;
	int32_t iMinimumHealthBar = 0;
	int32_t iMaximumHealthBar = 0;
	int32_t iTriggerHealthBar = 0;
	uint32_t iTriggerOrder = 0u;
	std::string strArmorRequirement;
	std::string strPhaseRequirement;
	bool_t bInvulnerableWhileRunning = false;
	uint32_t iSelectionWeight = 0u;
	uint32_t iMaximumConsecutiveUses = 0u;
	f32_t fMinimumRange = 0.f;
	f32_t fMaximumRange = 0.f;
	// 0은 선택적 높이 필드의 생략값이다. 명시한 저작 값은 유한한 범위의 0이 아닌 값으로 검증한다.
	f32_t fVerticalOffsetM = 0.f;
	std::optional<VALTAN_PATTERN_SERVER_MOTION_VIEW> ServerMotion;
	std::optional<VALTAN_PATTERN_FINALE_VIEW> Finale;
	uint32_t iSourceSequenceIndex = 0u;
	std::vector<VALTAN_PRESENTATION_SOURCE_VIEW> PresentationSources;
	// 저작 원본이 소유하는 presentation 및 반응 참조를 typed 값과 원본 순서로 유지한다.
	// ValtanEncounter.json에 투영되지 않는 참조도 결합 결과에서 잃지 않도록 별도로 보관한다.
	std::vector<VALTAN_PATTERN_REACTION_VIEW> Reactions;
	std::vector<std::string> CameraCueIds;
	std::vector<VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW> WorldEventTriggerRefs;
	bool_t bAuthoringMasterManaged = false;
	bool_t bManualServerAudition = false;
	std::string strSourceAnimationChainId;
	uint32_t iAuthoringPhase = 0u;
	std::string strAdmissionState;
	std::vector<VALTAN_STAGE_VIEW> Stages;

	// 이 트리의 분류 기준은 triggerHealthBar의 0 여부다.
	// 0이 아닌 행을 Gimmicks에 넣으며 실제 Server 실행 가능 여부는 별도 계약이 결정한다.
	bool_t Is_Gimmick() const
	{
		return 0 != iTriggerHealthBar;
	}
};

enum class VALTAN_PATTERN_PREVIEW_PATH : uint8_t
{
	NORMAL,
	COUNTER_GROGGY,
	WALL_GROGGY,
	PART_BREAK,
	CAPTURE_SUCCESS,
	END
};

enum class VALTAN_PATTERN_TREE_LOAD_POLICY : uint8_t
{
	// 현재 원본 gameplay와 presentation의 값이 생성된 Product와 일치하는지 대조한다.
	// 실행 순서의 정본은 gameplay 저작 문서이며 이전 생성 rotation의 순서를 두 번째 저작 정본으로 삼지 않는다.
	REQUIRE_ACTIVE_PRODUCT_PARITY,
	// 저장한 revision 복원용이다. stable topology와 presentation 참조를 확인한다.
	// 오늘의 Product 수치와 비교하는 대신 저장된 gameplay 값을 임시 뷰에 덮어쓴다.
	RESTORE_AUTHORING_SNAPSHOT,
	END
};

// Server 패턴 또는 전투 오브젝트가 trigger를 소유하는 재사용 Effect의 저작 identity다.
// All Effects의 독립 항목으로 두고 소유 패턴 안에서는 읽기 전용 ID 참조를 사용한다.
struct VALTAN_INDEPENDENT_EFFECT_VIEW final
{
	std::string strIndependentEffectId;
	std::string strDisplayName;
	std::string strEffectAssetId;
	std::string strOwnership;
	std::string strOwnerPatternId;
	std::string strOwnerStageId;
	std::string strTriggerPolicy;
	std::string strCombatObjectArchetypeId;
	std::string strClientVisualId;
	std::string strEffectCueBindingId;
	// SERVER_PATTERN_STAGE는 Product clip occurrence 또는 명시적 stage 시각과 연결한다.
	// SERVER_COMBAT_OBJECT에는 보스 본체 cue가 없으므로 이 cue 연결값을 비워 둔다.
	std::string strCueClipOccurrenceId;
	std::string strCueMappingBasis;
	uint32_t iCueSourceStartMs = 0u;
	uint32_t iCueSourceEndMs = 0u;
	bool_t bHasCueProjection = false;
	bool_t bHasCueSourceEnd = false;
	bool_t bUsesCueStageClock = false;
	uint32_t iCueStageOffsetMs = 0u;
};

// 기믹의 체력 경계에서 파생한 화면 표시용 구간이다.
// Server gameplay phase나 저장된 stable ID와 구분하며 현재 트리의 체력 범위로 계산한다.
struct VALTAN_PHASE_VIEW final
{
	static constexpr size_t INVALID_INDEX =
		(std::numeric_limits<size_t>::max)();

	uint32_t iPhaseNumber = 0u;
	int32_t iBandTopHealthBar = 0;
	int32_t iBandBottomHealthBar = 0;
	// 마지막 기믹 아래에 남은 표시 구간은 gate 패턴이 없으므로 비어 있다.
	std::string strGatePatternId;
	int32_t iGateTriggerHealthBar = 0;

	// 현재 트리의 Gimmicks와 Rotation에 대한 인덱스다. 저장 identity가 아니다.
	// 여러 구간에 나타나는 같은 rotation을 복사하지 않고 원본 행을 함께 참조한다.
	std::vector<size_t> GimmickIndices;
	std::vector<size_t> RotationIndices;
};

// 결합 결과의 문자열과 vector 값을 소유한다. stage 포인터와 표시 인덱스는 이 결과의 수명 안에서만 유효하다.
// 성공한 전체 로드가 이 값을 교체하므로 소비자는 이전 vector의 포인터를 다시 찾아야 한다.
struct VALTAN_PATTERN_TREE_VIEW final
{
	// 저장 리소스를 화면에 표시하기 위한 이름이다. 저장 identity로 사용하거나 직렬화하지 않는다.
	std::unordered_map<std::string, std::string> EffectResourceDisplayLabels;
	// gameplay 저작 문서가 패턴 정의와 실행 순서를 함께 소유한다.
	// 도구는 별도 저장 Flow를 다시 결합하지 않고 같은 원본에서 읽은 순서를 사용한다.
	std::string strScriptedSequenceId;
	std::string strScriptedSequenceMode;
	// 자동 체력 반복의 입장 연출 패턴 ID다. 실행 순서를 편집할 때도 이 별도 identity를 유지한다.
	std::string strEntranceCinematicPatternId;
	uint32_t iScriptedSequenceInterStepPursuitMs = 0u;
	std::vector<std::string> ScriptedSequencePatternIds;
	// PatternIds[index] 뒤 전환에 대응하는 추적 시간이다.
	// 선택 배열이 없는 기존 원본은 공통 기본 추적 시간으로 확장한다.
	std::vector<uint32_t> ScriptedSequenceTransitionPursuitMs;
	std::vector<VALTAN_PATTERN_VIEW> Gimmicks;
	std::vector<VALTAN_PATTERN_VIEW> Rotation;
	std::vector<VALTAN_INDEPENDENT_EFFECT_VIEW> IndependentEffects;
	std::vector<VALTAN_SELECTION_SET_VIEW> SelectionSets;
	std::vector<VALTAN_SELECTION_WINDOW_VIEW> SelectionWindows;
	std::vector<VALTAN_MECHANIC_VIEW> Mechanics;
	std::vector<VALTAN_MANUAL_AUDITION_VIEW> ManualAuditions;
	std::vector<VALTAN_LEGACY_ROTATION_VIEW> LegacyRotations;
	VALTAN_NORMAL_SELECTION_VIEW NormalSelection;
	std::vector<VALTAN_COUNTER_REACTION_LAYER_VIEW> CounterReactionLayers;
	std::vector<VALTAN_PHASE_VIEW> Phases;
	// introPatternId에 대응하는 현재 Rotation 인덱스다. 없으면 INVALID_INDEX다.
	size_t iIntroRotationIndex = VALTAN_PHASE_VIEW::INVALID_INDEX;

	size_t Get_PatternCount() const
	{
		return Gimmicks.size() + Rotation.size();
	}
	// 아래 집계는 Gimmicks와 Rotation의 현재 내용을 합산한다.
	// Get_EffectCount는 Effect가 연결된 stage 수이며 Get_EffectDocumentCount는 연결 행 수다.
	// 동일 asset ID를 하나로 합친 고유 리소스 개수가 아니다.
	size_t Get_StageCount() const;
	size_t Get_EffectCount() const;
	size_t Get_EffectDocumentCount() const;
	size_t Get_ClipBoundStageCount() const;
	size_t Get_ProductCueStageCount() const;
	size_t Get_ClipOccurrenceCount() const;
	size_t Get_ProductCueCount() const;
	size_t Get_CombatObjectEffectCount() const;
};

// 검증된 저작 패턴을 공통 도구의 재생 목록으로 분류한다. legacy Product 전용 행은 제외한다.
// 내용이 없는 수동 저작 행은 목록에서 제외될 수 있으며 각 vector의 크기는 고정 제한이 아니다.
struct VALTAN_TOOL_AUDITION_INVENTORY final
{
	std::vector<std::string> CorePatternIds;
	std::vector<std::string> AnimatorPatternIds;
	std::vector<std::string> DerivedPatternIds;

	size_t Get_PatternCount() const
	{
		return CorePatternIds.size() + AnimatorPatternIds.size() +
			DerivedPatternIds.size();
	}
	bool_t Contains(const std::string& strPatternId) const;
};

// 공용 읽기 진입과 엄격한 결합의 실패 이유다. 진단 문장과 별개로 재시도 정책을 선택한다.
// STALE_PROJECTION은 writer의 재투영이 필요하며 reader가 자동으로 Product를 수정하지 않는다.
enum class VALTAN_CANONICAL_READ_FAILURE_KIND : uint8_t
{
	NONE,
	WRITER_BUSY,
	WRITER_RECOVERY_REQUIRED,
	ADMISSION_IO,
	ADMISSION_STATE_INVALID,
	GENERATION_CHANGED,
	SOURCE_INVALID,
	PRODUCT_INVALID,
	STALE_PROJECTION,
	END
};

struct VALTAN_CANONICAL_READ_DIAGNOSTIC final
{
	static constexpr std::string_view PRODUCT_PROJECTION_COMMAND =
		"powershell -ExecutionPolicy Bypass -File "
		"Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2";

	VALTAN_CANONICAL_READ_FAILURE_KIND eFailure =
		VALTAN_CANONICAL_READ_FAILURE_KIND::NONE;
	std::string strStatus;
	// 실패를 특정 stable 소유자에 귀속할 수 있을 때 채우는 진단 ID다.
	// 이 필드 자체가 해당 패턴의 편집 또는 재생을 허용하지는 않는다.
	std::string strRejectedPatternId;
	std::string strRejectedStageId;

	void Clear()
	{
		eFailure = VALTAN_CANONICAL_READ_FAILURE_KIND::NONE;
		strStatus.clear();
		strRejectedPatternId.clear();
		strRejectedStageId.clear();
	}
	bool_t Is_AutomaticRetryable() const
	{
		return VALTAN_CANONICAL_READ_FAILURE_KIND::WRITER_BUSY == eFailure ||
			VALTAN_CANONICAL_READ_FAILURE_KIND::GENERATION_CHANGED == eFailure;
	}
	bool_t Requires_ProductProjection() const
	{
		return VALTAN_CANONICAL_READ_FAILURE_KIND::STALE_PROJECTION == eFailure;
	}
};

// Create/Project writer와 같은 create-pattern.lock의 byte 0을 공유 모드로 보유한다.
// 첫 파일 읽기부터 최종 검증까지 유지하면 이 잠금 계약을 지키는 writer와 읽기가 섞이지 않는다.
// active-generation journal이 남으면 writer의 복구를 기다리며 reader가 복구하지 않는다.
// Win32 잠금 구현은 CPP에 숨기고 이 객체의 소멸자가 해제한다.
class CValtanCanonicalProductReadAdmission final
{
public:
	CValtanCanonicalProductReadAdmission() = default;
	~CValtanCanonicalProductReadAdmission();
	CValtanCanonicalProductReadAdmission(
		const CValtanCanonicalProductReadAdmission&) = delete;
	CValtanCanonicalProductReadAdmission& operator=(
		const CValtanCanonicalProductReadAdmission&) = delete;

	// 읽기 잠금과 journal 검사에 성공하면 불투명 상태의 소유권을 이 객체로 넘긴다.
	// 이미 획득한 객체의 재획득은 실패하며 기존 잠금을 보존한다. 파일 잠금 대기는 즉시 실패 방식이다.
	bool_t Acquire(std::string& strOutStatus);
	bool_t Acquire(VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic);
	// WRITER_BUSY와 GENERATION_CHANGED를 자동 재시도 가능한 이유로 분류한다.
	// 복구 journal이나 원본 재투영 실패를 이 함수가 직접 복구하지는 않는다.
	static bool_t Is_TransientFailure(
		VALTAN_CANONICAL_READ_FAILURE_KIND eFailure);
	// 호출자가 임시 결합 결과를 반영하기 전에 journal 부재를 다시 확인한다.
	// 이 함수는 호출자의 캐시를 변경하지 않으며 이전 결과 보존은 호출자의 stage/commit 경계가 책임진다.
	bool_t Validate_StillCurrent(std::string& strOutStatus) const;
	bool_t Validate_StillCurrent(
		VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic) const;
	bool_t Is_Acquired() const
	{
		return nullptr != m_pState;
	}

private:
	// 이 객체가 단독 소유하는 CPP 내부 상태다. 소멸 시 delete와 함께 파일 잠금 및 handle을 해제한다.
	// 별도 thread 동기화는 제공하지 않으므로 동일 객체의 Acquire와 소멸을 동시에 호출하지 않는다.
	void* m_pState = nullptr;
};

// Valtan의 물리 저작 원본과 Encounter, animation binding 및 Effect cue를 결합하는 도구용 경계다.
// Load 계열은 임시 뷰를 검증한 뒤 출력에 반영한다. 저작 JSON이나 Product를 게시하지 않는다.
// BalanceTool, ValtanActionWorkbench, ValtanBossTool과 Effect/Animation 도구가 소비한다.
class CValtanPatternTree final
{
public:
	// 탐색기 갱신 때 원본 clip 메타데이터를 읽는다. 실패하면 기존 OutIndex를 유지한다.
	// 순수 matcher는 action 또는 cue 연결과 정확한 clip 이름 일치를 함께 요구한다.
	static bool_t Load_FullRestoreSourceClips(
		const std::filesystem::path& Path,
		VALTAN_FULL_RESTORE_CLIP_INDEX& OutIndex,
		std::string& strOutStatus,
		bool_t bUseAuthoredPreview = false);
	static bool_t Matches_FullRestoreSource(
		const VALTAN_PATTERN_VIEW& Pattern,
		const std::string& strEffectAssetId,
		const VALTAN_FULL_RESTORE_CLIP_INDEX& Index);

	// Effect 선택기와 제목에서 사용하는 표시 이름만 구성한다.
	// stable ID, 저장 문서 이름과 재생 소유권은 변경하지 않는다.
	static void Build_EffectResourceDisplayLabels(
		VALTAN_PATTERN_TREE_VIEW& View,
		const std::unordered_map<std::string, std::string>& SavedNames,
		const VALTAN_FULL_RESTORE_CLIP_INDEX& SourceClips);
	static std::string Describe_EffectResource(
		const VALTAN_PATTERN_TREE_VIEW& View, const std::string& strEffectAssetId,
		const std::string& strSavedName, const VALTAN_FULL_RESTORE_CLIP_INDEX& Index);

	// Product와의 값 일치를 요구하지 않는 원본 목록 로드다. split schema와 identity를 검증한다.
	// 실제 리소스와 재생 준비 검증은 Preview/Publish의 책임이며 실패하면 기존 OutView를 유지한다.
	static bool_t Load_Authoring_WhileAdmitted(
		const CValtanCanonicalProductReadAdmission& Admission,
		VALTAN_PATTERN_TREE_VIEW& OutView,
		std::string& strOutStatus);

	// 단독 읽기용 진입점이다. 자체 admission을 잡고 엄격하게 결합한 뒤 성공한 출력만 교체한다.
	static bool_t Load(
		VALTAN_PATTERN_TREE_VIEW& OutView,
		std::string& strOutStatus);
	static bool_t Load(
		VALTAN_PATTERN_TREE_VIEW& OutView,
		VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic);
	// 여러 문서를 묶어 읽는 호출자가 이미 가진 admission을 사용한다.
	// 동일 byte 잠금을 중복 획득하지 않고 결합 전후의 journal 상태를 확인한다.
	static bool_t Load_WhileAdmitted(
		const CValtanCanonicalProductReadAdmission& Admission,
		VALTAN_PATTERN_TREE_VIEW& OutView,
		std::string& strOutStatus);
	static bool_t Load_WhileAdmitted(
		const CValtanCanonicalProductReadAdmission& Admission,
		VALTAN_PATTERN_TREE_VIEW& OutView,
		VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic);
	// 저작 경로에는 절대 경로를 전달할 수 있다. actor와 전투 오브젝트 연결은 현재 Product에서 읽는다.
	// 정책에 따라 원본 값까지 일치시킬지 저장한 원본 snapshot을 덮어쓸지 결정한다.
	// 보충: 복원 정책의 덮어쓰기 대상은 현재 Product로 만든 임시 메모리 뷰다.
	// 읽은 원본 gameplay 값을 그 뷰에 반영하며 snapshot 파일이나 Product 파일을 저장하지 않는다.
	// 이 overload는 admission을 자체 획득하지 않으므로 필요한 잠금은 호출자가 보유해야 한다.
	static bool_t Load_FromAuthoringPaths(
		const std::filesystem::path& GameplayPath,
		const std::filesystem::path& PresentationPath,
		VALTAN_PATTERN_TREE_VIEW& OutView,
		std::string& strOutStatus,
		VALTAN_PATTERN_TREE_LOAD_POLICY ePolicy,
		VALTAN_CANONICAL_READ_DIAGNOSTIC* pOutDiagnostic = nullptr);
	// Boss Tool, All Effects와 Next가 공유하는 재생 identity 목록을 만든다.
	// stage와 수동 시험 메타데이터의 양쪽 소유 관계를 확인하며 실패하면 기존 목록을 유지한다.
	static bool_t Build_PlayablePatternInventory(
		const VALTAN_PATTERN_TREE_VIEW& View,
		VALTAN_TOOL_AUDITION_INVENTORY& OutInventory,
		std::string& strOutError);
	// 같은 재생 목록을 원본 sequence index와 stable ID 순으로 정렬한다. 실패하면 기존 출력을 유지한다.
	static bool_t Build_NextPatternInventory(
		const VALTAN_PATTERN_TREE_VIEW& View,
		std::vector<std::string>& OutPatternIds,
		std::string& strOutError);
	// All Effects와 Boss Tool에서 공용으로 표시할 identity 및 clip 시간 설명이다.
	// 표시 이름의 원본 소유권을 바꾸거나 이 문자열로 런타임 명령을 만들지 않는다.
	static std::string Build_PatternIdentitySummary(
		const VALTAN_PATTERN_VIEW& Pattern);
	// effect.valtan.<pattern-slug>.<stage-slug> 형태의 fallback ID를 만든다.
	// Tools/EffectPipeline/build_valtan_stage_effects.py와 같은 이름 규칙을 따른다.
	static std::string Build_StageEffectAssetId(
		const std::string& strPatternActionId,
		const VALTAN_STAGE_VIEW& Stage);
	// action ID를 따라 선택한 미리보기 분기 경로를 만든다. 분기가 없는 stage만 다음 순번으로 이어진다.
	// 반환 포인터는 Pattern.Stages를 차용하므로 원본 vector 변경이나 파괴 뒤에는 사용할 수 없다.
	// 검증 실패 시 기존 OutStages를 유지하며 다른 패턴으로의 분기에서는 현재 경로를 끝낸다.
	static bool_t Build_PreviewStagePath(
		const VALTAN_PATTERN_VIEW& Pattern,
		VALTAN_PATTERN_PREVIEW_PATH ePath,
		std::vector<const VALTAN_STAGE_VIEW*>& OutStages,
		std::string& strOutStatus);
};

NS_END
