#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "CameraShakeService.h"
#include "Network/PacketType.h"
#include "BinaryAsset/ModelAssetData.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

// Server 부위 생존 비트와 연결할 수 있는 보스 갑옷 정의의 최대 수다.
inline constexpr std::size_t MAX_BOSS_ARMOR_PARTS = 4u;
// 보스 한 종류가 정의할 수 있는 연출 수다. 실행 중 생성된 전투 오브젝트 개수와는 별개다.
inline constexpr std::size_t MAX_BOSS_COMBAT_OBJECT_VISUALS = 32u;

/* 로드 뒤에도 보관하는 원본 캐릭터 재질의 저작 값이다. 생성 화면에서 피부색·광택·화장·데칼
일부를 바꾸더라도 셰이더 상수 행은 전체 값으로 다시 만들어야 하므로 원래 매개변수들을 함께 둔다.
값 형태는 SourceCharacterMaterial::PARAMETER_VALUES와 같으며, 생성된 패킹 헤더에 의존하지 않는다. */
struct CHARACTER_MATERIAL_PARAMETERS final
{
	std::string materialName;
	std::string family;
	std::map<std::string, std::array<float, 4>> values;
};

// Server 직업 ID를 본체·장비·애니메이션·재질 정의에 연결한다. 생성된 캐릭터 객체는 별도 소유다.
struct CHARACTER_ACTOR_ENTRY final
{
	std::string archetypeId;
	LostArk::Shared::CHARACTER_CLASS_ID networkClassId =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string assetId;
	// 표시 크기만 조절하는 선택적 v4 값이다. 모델 단위 변환과 전투 판정 크기는 바꾸지 않는다.
	f32_t presentationScale = 1.f;
	std::string bodyModel;
	std::vector<std::string> equipmentModels;
	std::vector<std::string> weaponModels;
	std::map<std::string, std::vector<Engine::MODEL_MATERIAL_OVERRIDE>, std::less<>> modelMaterialOverrides;
	/* 원본 캐릭터 셰이더용 값만 재질 이름으로 modelMaterialOverrides와 연결한다.
	그 밖의 재질에는 이 목록을 만들지 않는다. */
	std::map<std::string, std::vector<CHARACTER_MATERIAL_PARAMETERS>, std::less<>>
		modelMaterialParameters;
	/* 본체를 준비할 때 선언 순서대로 붙일 공용 애니메이션의 .wmodel 목록이다.
	에스더 호출·커스터마이징 대기처럼 다른 묶음의 클립을 빌려 쓸 때만 항목을 둔다. */
	std::vector<std::string> animationSetModels;
	/* Drive로 배포하는 선택적 물총 클립이다. Maharaka에서만 준비하며,
	누락되면 캐릭터 전체 대신 물총 장착만 사용할 수 없게 한다. */
	std::vector<std::string> waterGunAnimationSetModels;
	std::string animationSetId;
	std::string runtimeStatus;
};

// 보스 갑옷의 표시 정의다. 실제 파괴 여부는 Server snapshot에서 읽는다.
struct BOSS_ARMOR_PART_ENTRY final
{
	// BossParts 밸런스 데이터와 같은 부위를 가리키는 저장 ID다.
	std::string partId;
	// Server의 BOSS_COMBAT_SNAPSHOT::iAlivePartMask에서 해당 부위의 생존 여부를 나타내는 한 비트다.
	std::uint32_t stateMask = 0u;
	std::string modelAssetId;
};

enum class BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND : uint8_t
{
	EFFECT_V1,
	EFFECT_V2_GROUP
};

// 독립 전투 오브젝트의 활성 연출을 Effect V2 그룹으로 재생할 때 필요한 입력이다.
struct BOSS_COMBAT_OBJECT_EFFECT_V2_GROUP final
{
	std::string groupId;
	f32_t playbackRate = 1.f;
	/* Server 적중과 시각 재생을 연결하는 계약이 검증된 경우에만 true다.
	false이면 아래 적중 시각·ID는 기본값으로 두고 그룹 자체의 재생 시간만 사용한다. */
	bool_t bHasHitSync = false;
	uint32_t visualHitMs = 0u;
	std::string serverHitId;
};

// 낙하 도끼처럼 보스의 손과 별도로 생성되는 전투 오브젝트의 표현 정의다.
// Server의 오브젝트 종류·표현 ID를 이펙트에 연결하며, 손 무기의 소켓 보정값은 사용하지 않는다.
struct BOSS_COMBAT_OBJECT_VISUAL_ENTRY final
{
	std::string combatObjectArchetypeId;
	std::string clientVisualId;
	/* All Effects·Composition에서 편집할 V1 참조는 effectAssetId에 유지한다.
	V2 그룹을 선택하면 제품에서 생성하는 연출만 교체하며 두 방식을 동시에 재생하지 않는다. */
	BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND activeEffectKind =
		BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1;
	std::string effectAssetId;
	BOSS_COMBAT_OBJECT_EFFECT_V2_GROUP effectV2Group;
	/* 신뢰성 전송된 HIT_PULSE마다 생성해 자체 수명이 끝날 때까지 재생할 선택적 이펙트다.
	Server는 전투 오브젝트·적중의 ID와 자세를 보내고, 실제 표현 에셋은 Client가 선택한다. */
	std::string hitEffectAssetId;
	// 지정한 Server 수명 이벤트를 받았을 때 경고 연출을 시작하기 위한 대응 ID다.
	std::string armedPresentationEventId;
	std::string armedEffectAssetId;
	bool_t stopActiveOnHit = false;
	// 지정한 Server 이벤트에서 기존 대기 연출을 종료하고 armed 연출로 바꾼다.
	bool_t stopActiveOnArmed = false;
	// armed V1 연출 하나가 종료 부분까지 재생한다. 미리 시작하지 못했으면 같은 에셋을 적중 때 시작한다.
	// 두 armed 플래그가 모두 false이면 armed/hit 공용 에셋은 적중 때만 시작한다.
	bool_t armedEffectOwnsTerminal = false;
	// Server가 정한 전투 오브젝트 위치·방향에 곱할 시각 배율이다.
	float3_t worldScale = { 1.f, 1.f, 1.f };

	// 복제된 위치(m)·Y축 회전(도)에 시각 배율을 합성해 이펙트 생성 입력을 만든다. 카탈로그 값은 바꾸지 않는다.
	float4x4_t Make_WorldRoot(const float3_t& position, const f32_t yawDegrees) const
	{
		float4x4_t root{};
		DirectX::XMStoreFloat4x4(&root,
			DirectX::XMMatrixScaling(worldScale.x, worldScale.y, worldScale.z) *
			DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(yawDegrees)) *
			DirectX::XMMatrixTranslation(position.x, position.y, position.z));
		return root;
	}
};

/* 원본 LookInfo의 본 부착 파티클 한 건이다. 위치는 원본 단위를 변환한 미터 값이다.
같은 ParticleSystem을 여러 번 붙여도 occurrenceId로 각 부착을 구분한다. */
struct BOSS_DEFAULT_PARTICLE_ENTRY final
{
	std::string occurrenceId;
	std::string effectAssetId;
	std::string boneName;
	float3_t position{};
	float3_t rotationDegrees{};
	float3_t scale{ 1.f, 1.f, 1.f };
};

// 발탄·유령 발탄의 손 본(b_wp_r_01)에 붙는 도끼 Part의 추가 보정값이다.
// CValtan::Update가 이 값을 CPart_Equipment에 복사한다. 독립 낙하 도끼의 위치·판정과는 무관하다.
struct BOSS_WEAPON_SOCKET_TRANSFORM final
{
	// 길이를 1로 맞춘 소켓 본의 X/Y/Z축을 따른 추가 이동량(m)이다.
	// 본에 포함된 model preScale은 다시 곱하지 않으며, 이후 보스의 presentationScale은 적용된다.
	float3_t positionMeters{};
	// 소켓에 붙이기 전에 추가할 X/Y/Z 회전(pitch/yaw/roll, 도)이다. 두 벡터의 0은 추가 보정 없음이다.
	float3_t rotationDegrees{};
};

// 보스 하나의 표현 정의다. 실제 CValtan·Part 객체와 Server 전투 상태를 이 구조체가 소유하지 않는다.
struct BOSS_ACTOR_ENTRY final
{
	struct PRESENTATION_CLIPS final
	{
		std::string idle;
		std::string chase;
		std::string patternWindup;
		std::string patternActive;
		std::string patternRecovery;
		std::string dead;
	};

	std::string archetypeId;
	std::string visualAssetId;
	f32_t presentationScale = {};
	// 모델 원본 단위를 변환하는 배율이다. 보스 전체의 표시 배율보다 먼저 적용한다.
	f32_t bodyModelPreScale = {};
	/* 별도 무기가 없는 본체 일체형 표현에서만 0을 허용한다.
	발탄의 표현 계약은 본체와 별도 무기 모델을 모두 요구한다. */
	f32_t weaponModelPreScale = {};
	/* 원본 무기 축을 소켓에 맞추는 X/Y/Z 회전(pitch/yaw/roll, 도)이다.
	모델 원형을 만들 때 배율보다 먼저 적용하며, 별도 무기가 없으면 0이다.
	실행 중 손 위치를 조절하는 아래 weaponSocketTransform과 적용 단계가 다르다. */
	float3_t weaponModelPreRotationDegrees = {};
	// 손 도끼의 세션 보정값이며, 편집기의 Save가 같은 필드를 BossCatalog.json에 기록한다.
	BOSS_WEAPON_SOCKET_TRANSFORM weaponSocketTransform;
	std::string bodyModel;
	std::string weaponModel;
	/* 본체 골격의 자세를 공유하는 갑옷 모델이다. 자체 애니메이션을 재생하지 않으며
	선언한 순서대로 Part를 붙인다. */
	std::vector<std::string> armorModels;
	/* 본체 골격을 공유하면서 Server의 부위 상태에 따라 표시할 갑옷 정의다.
	배열 순서·Prototype tag가 아닌 partId/stateMask로 전투 상태와 연결한다. */
	std::vector<BOSS_ARMOR_PART_ENTRY> armorParts;
	std::vector<BOSS_COMBAT_OBJECT_VISUAL_ENTRY> combatObjectVisuals;
	std::vector<BOSS_DEFAULT_PARTICLE_ENTRY> defaultParticles;
	std::string animationSetId;
	std::string serverProfileId;
	std::string clientPresentationId;
	std::string presentationStatus;
	PRESENTATION_CLIPS presentationClips;
};

// Server의 NPC 종류·행동을 Client 모델과 재생 클립에 연결하는 정의다.
struct NPC_ACTOR_ENTRY final
{
	std::string archetypeId;
	std::string clientPresentationId;
	std::string modelAssetId;
	std::string animationSetId;
	std::string idleClip;
	std::string runtimeStatus;
	/* 에스더 소환처럼 Server snapshot의 행동 ID로 재생할 클립 목록이다.
	각 클립을 한 번씩 순서대로 재생하며, 등록되지 않은 행동은 idleClip으로 표시한다. */
	std::map<std::string, std::vector<std::string>, std::less<>> actionClips;
	/* 에스더 타격 행동의 시작을 기준으로 delayMs 뒤 한 번 재생하는 전체 화면 컷인이다.
	원본 Esther_Skill_<Name> 영상을 UI 기준 해상도의 <framePrefix>_NNN.dds로 변환하며,
	000부터 frameCount-1까지 fps 속도로 재생한다. frameCount가 0이면 컷인이 없다.
	delayMs는 시전자의 호출 기준 SkillDecoDelayTime에서 Server의 소환 착지 대기 1000ms를
	뺀 값이며 음수가 되면 0으로 맞춘다. */
	struct CUTIN_MOVIE final
	{
		std::string framePrefix;
		std::uint32_t frameCount = 0;
		f32_t fps = 0.f;
		std::uint32_t delayMs = 0;
	};
	CUTIN_MOVIE cutinMovie;
	/* "esther"이면 전용 NPC 셰이더를 사용하고, 빈 값이면 공용 애니메이션 메시 셰이더를 쓴다.
	등록되지 않은 프로필은 JSON을 읽을 때 거부한다. */
	std::string shaderProfile;
};

/* 탑승 중 직업별 본체가 재생할 자세다. 해당 직업 모델에 변환해 둔
"<armature>_ride_<mode>_*" 클립 이름을 참조한다. */
struct VEHICLE_RIDER_ENTRY final
{
	LostArk::Shared::CHARACTER_CLASS_ID characterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string idleClip;
	std::string runClip;
};

// 같은 탈것 스킬이라도 직업별 탑승자 본체가 재생할 클립 체인은 다를 수 있다.
struct VEHICLE_SKILL_RIDER_ENTRY final
{
	LostArk::Shared::CHARACTER_CLASS_ID characterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::vector<std::string> clips;
};

/* 탈것 스킬의 Particle notify 묶음을 V1 Effect 문서 하나로 복원한 재생 항목이다.
startMs는 전체 스킬이 아니라 clipIndex가 가리키는 클립 시작부터의 시간(ms)이다. */
struct VEHICLE_SKILL_EFFECT_CUE final
{
	std::uint32_t clipIndex = 0u;
	std::string effectAssetId;
	std::uint32_t startMs = 0u;
	bool_t bStopAtCueEnd = false;
	/* "followPolicy": "SNAPSHOT"이면 스킬 시작 위치에 이펙트 root를 남겨
	탈것 이동에 끌려가지 않게 한다. 문서 내부의 본 부착 항목은 계속 해당 본을 따른다. */
	bool_t bSnapshotRoot = false;
};

/* 탈것 스킬 클립의 원본 AKEvent를 Vehicle 사운드 카탈로그에서 찾아 재생한다.
startMs는 clipIndex가 가리키는 클립 시작 기준 시간(ms)이다. */
struct VEHICLE_SKILL_SOUND_CUE final
{
	std::uint32_t clipIndex = 0u;
	std::string event;
	std::uint32_t startMs = 0u;
};

/* 반복 이동 클립에서 발이 지면에 닿는 원본 floor-material notify에 대응한다.
startMs는 스킬 시작이 아닌 clip의 시작을 기준으로 한 발소리 시점(ms)이다. */
struct VEHICLE_LOCOMOTION_SOUND_CUE final
{
	std::string clip;
	std::uint32_t startMs = 0u;
	std::string event;
};

// 지정 클립의 startMs에서 CAMERA_SHAKE_SPEC으로 카메라 흔들림을 요청한다.
struct VEHICLE_SKILL_SHAKE_CUE final
{
    std::uint32_t clipIndex = 0u;
    std::uint32_t startMs = 0u;
    std::string payload;
    CAMERA_SHAKE_SPEC Spec;
};

// 스킬 클립의 특정 시점부터 조명 밝기 배율을 적용하고 초 단위로 진입·유지·종료한다.
struct VEHICLE_DIRECTIONAL_LIGHT_CUE final
{
    std::uint32_t clipIndex = 0u, startMs = 0u;
    f32_t fadeInSeconds = 0.f, holdSeconds = 0.f, fadeOutSeconds = 0.f;
    f32_t brightnessMultiplier = 1.f;
};

// 재질 벡터 변화에서 사용할 시간(초)과 네 성분의 값이다.
struct VEHICLE_MATERIAL_VECTOR_KEY final
{
    f32_t seconds = 0.f;
    std::array<f32_t, 4> value{};
};
// 클립 내 startMs에서 지정 재질 매개변수의 키 목록을 재생한다.
struct VEHICLE_MATERIAL_VECTOR_CUE final
{
    std::uint32_t clipIndex = 0u, startMs = 0u;
    std::string parameter;
    std::vector<VEHICLE_MATERIAL_VECTOR_KEY> keys;
};

// Server가 수행하는 탈것 스킬에 대응할 단축 슬롯과 탈것·직업별 탑승자의 연속 클립·연출이다.
struct VEHICLE_SKILL_ENTRY final
{
	std::uint32_t skillId = 0u;
	std::string inputSlot;
	std::vector<std::string> vehicleClips;
	f32_t flightLoopStartSeconds = 0.f;
	f32_t flightLoopEndSeconds = 0.f;
	f32_t flightLandingStartSeconds = 0.f;
	std::vector<VEHICLE_SKILL_RIDER_ENTRY> riders;
	std::vector<VEHICLE_SKILL_EFFECT_CUE> effectCues;
	std::vector<VEHICLE_SKILL_SOUND_CUE> soundCues;
    std::vector<VEHICLE_SKILL_SHAKE_CUE> shakeCues;
    std::vector<VEHICLE_DIRECTIONAL_LIGHT_CUE> directionalLightCues;
    std::vector<VEHICLE_MATERIAL_VECTOR_CUE> materialVectorCues;

	// 이 항목의 riders 원소를 차용한다. 없으면 nullptr이며, 소유 항목 파괴·목록 재할당 뒤에는 다시 찾아야 한다.
	const VEHICLE_SKILL_RIDER_ENTRY* Find_Rider(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
	{
		for (const VEHICLE_SKILL_RIDER_ENTRY& rider : riders)
			if (rider.characterClass == characterClass)
				return &rider;
		return nullptr;
	}
};

/* 탑승 성공부터의 경과 시간(ms)으로 시작한다. ambient 반복 연출은 탈것 교체까지 유지하고,
mount 연출은 한 번 재생한 뒤 자체 수명에 따라 끝난다. */
struct VEHICLE_LIFETIME_EFFECT_CUE final
{
    std::string effectAssetId;
    std::uint32_t startMs = 0u;
};

/* Server가 복제하는 EFTable_Vehicle의 vehicleId를 Client의 탈것 표현과 연결한다.
모델·탑승 자세·연출의 정의이며, 현재 탑승 상태나 이동 판정을 소유하지 않는다. */
struct VEHICLE_ACTOR_ENTRY final
{
	std::uint32_t vehicleId = 0u;
	std::string archetypeId;
	std::string modelAssetId;
	f32_t modelPreScale = 0.f;
	std::string seatBone;
	std::string vehicleIdleClip;
	std::string vehicleRunClip;
	std::vector<VEHICLE_RIDER_ENTRY> riders;
	std::vector<VEHICLE_SKILL_ENTRY> skills;
	std::vector<VEHICLE_LOCOMOTION_SOUND_CUE> locomotionSoundCues;
    std::vector<VEHICLE_LIFETIME_EFFECT_CUE> ambientEffectCues;
    std::vector<VEHICLE_LIFETIME_EFFECT_CUE> mountEffectCues;
    std::string mountSoundEvent;
    std::string dismountSoundEvent;
    std::vector<CHARACTER_MATERIAL_PARAMETERS> modelMaterialParameters;
	std::string runtimeStatus;
	bool_t seatBoneRotatesRider = false;
	// EFTable_VoyageShip에서 온 배는 선박 NPC 창의 목록에만 표시한다.
	bool_t isShip = false;
	// 원본 선박 1레벨의 최대 보급량이다. 항해 HUD가 게이지 용량으로 읽으며 0이면 해당 값이 없다.
	std::uint32_t iMaxSupply = 0u;
	/* 좌석 본에 더할 탈것 root 공간의 이동량(m)이다. 배처럼 수면 위 좌석 본이 없는 경우
	갑판에 서도록 탑승자 위치를 보정한다. */
	float3_t seatOffset = { 0.f, 0.f, 0.f };
	// 모델 원형에 적용할 Y축 회전(도)이다. -90은 원본의 +X 전방을 +Z로 맞춘다.
	f32_t modelYawDegrees = -90.f;
	/* 모델 원형의 배율 적용 뒤 +Y로 올릴 높이(m)다. 메시와 본을 함께 이동한다.
	배의 용골 기준 원점을 보정해 가장 낮은 정점이 Server의 해상 이동 높이에 놓이게 한다.
	다른 탈것은 0으로 둔다. */
	f32_t modelLiftMeters = 0.f;

	// 아래 조회는 이 항목의 vector 원소를 차용하며, 해당 조건이 없으면 nullptr이다.
	// 소유 항목을 파괴하거나 목록을 재할당하면 차용 주소도 무효가 되므로 저장한 주소를 재사용하지 않는다.
	const VEHICLE_RIDER_ENTRY* Find_Rider(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
	{
		for (const VEHICLE_RIDER_ENTRY& rider : riders)
			if (rider.characterClass == characterClass)
				return &rider;
		return nullptr;
	}

	const VEHICLE_SKILL_ENTRY* Find_Skill(const std::uint32_t skillId) const
	{
		for (const VEHICLE_SKILL_ENTRY& skill : skills)
			if (skill.skillId == skillId)
				return &skill;
		return nullptr;
	}

	const VEHICLE_SKILL_ENTRY* Find_SkillBySlot(const std::string_view inputSlot) const
	{
		for (const VEHICLE_SKILL_ENTRY& skill : skills)
			if (skill.inputSlot == inputSlot)
				return &skill;
		return nullptr;
	}
};

// Server 몬스터 종류를 시각 모델과 행동별 클립에 연결한다. 공격 판정·피해량은 포함하지 않는다.
struct MONSTER_ACTOR_ENTRY final
{
	struct ATTACK_PRESENTATION final
	{
		std::string clip;
		f32_t playbackRate = 1.f;
		// 공격 클립 종료 때 생성하고 자체 수명에 따라 끝낼 선택적 이펙트다.
		std::string endEffectAssetId;
	};

	struct PRESENTATION_CLIPS final
	{
		std::string idle;
		std::string chase;
		std::string hit;
		std::string dead;
	};

	std::string archetypeId;
	std::string clientPresentationId;
	std::string modelAssetId;
	f32_t modelScale = 1.f;
	f32_t modelYawDegrees = 0.f;
	std::vector<ATTACK_PRESENTATION> attackPresentations;
	f32_t hitDurationSeconds = 0.f;
	PRESENTATION_CLIPS presentationClips;
	std::string runtimeStatus;
};

// JSON에서 읽은 표현 정의를 프로세스 메모리에 보관한다. GameObject 생성·수명 관리는 각 소비자가 맡는다.
// 여기서 canonical(정본)은 편집하여 저장하는 Data/Actors/BossCatalog.json 같은 저작 원본을 뜻한다.
// 현재 Initialize는 CProjectDataRoot로 그 원본을 직접 읽는다. publisher의 DataFiles 산출물과 이 캐시는 별개다.
// 초기화 성공 후 파일을 다시 읽는 API는 없으며, 런타임 값을 바꾸거나 파일을 저장해도 서로 자동 동기화되지 않는다.
class CActorCatalog final
{
public:
	// 최초 성공 때만 전체 카탈로그를 준비한다. 하나라도 실패하면 준비한 목록을 모두 비우고 false를 반환한다.
	// 내부 동기화가 없으므로 여러 스레드가 읽기 시작하기 전에 초기화를 끝내야 한다.
	static bool_t Initialize();
	// 모델을 소유한 카탈로그 항목의 재질 설정으로 제품과 미리보기의 로드 입력을 함께 만든다.
	// characterAssetId가 있으면 그 캐릭터가 소유한 모델만 허용하고, 비어 있으면 등록된 비플레이어 모델도 찾는다.
	// 실패하면 outDesc를 보존하고 원인은 outStatus로 돌려준다.
	static bool_t Build_ModelLoadDescription(std::string_view modelAssetId,
		Engine::MODEL_ASSET_LOAD_DESC& outDesc, std::string& outStatus,
		std::string_view characterAssetId = {});
	// 파생 static/skinned 모델에 재질을 제공할 원본 모델을 명시한다. 빈 원본은 정확한 모델 ID 조회를 유지한다.
	// 원본을 지정하면 실제 재질 슬롯이 대응해야 하며, 재질 이름 검색이나 셰이더 계열 대체로 추정하지 않는다.
	// 실패하면 outDesc를 보존하고 원인은 outStatus로 돌려준다.
	static bool_t Build_DerivedModelLoadDescription(std::string_view modelAssetId,
		std::string_view materialSourceModelAssetId,
		Engine::MODEL_ASSET_LOAD_DESC& outDesc, std::string& outStatus);
	// Find_*는 초기화 실패·대상 없음에 nullptr을 반환하고, 성공하면 캐시 원소의 주소를 빌려준다.
	// Get_* 목록 참조도 캐시를 차용한다. 초기화 성공 후 목록을 교체하지 않아 종료까지 주소가 유지되지만,
	// 도끼 보정값은 setter로 바뀔 수 있으므로 반환값은 독립된 스냅샷이 아니며 호출자가 해제해서는 안 된다.
	static const CHARACTER_ACTOR_ENTRY* Find_Character(
		LostArk::Shared::CHARACTER_CLASS_ID networkClassId);
	static const BOSS_ACTOR_ENTRY* Find_Boss(std::string_view archetypeId);
	static const std::vector<BOSS_ACTOR_ENTRY>& Get_Bosses();
	// 각 성분이 유한하고 위치 ±10m·회전 ±360도 이내인지 검사한다. 입력이나 캐시는 변경하지 않는다.
	static bool_t Validate_BossWeaponSocketTransform(const BOSS_WEAPON_SOCKET_TRANSFORM& value);
	// 메인 스레드의 도끼 편집기가 발탄/유령 발탄의 메모리 캐시 값만 바꾼다.
	// CValtan::Update가 다음 갱신에 읽으며, JSON 저장·게시·Server 전투 상태 변경은 수행하지 않는다.
	// 대상·범위 검증에 실패하면 기존 값을 유지하고 status에 이유를 돌려준다.
	static bool_t Set_ValtanWeaponSocketTransform(std::string_view archetypeId,
		const BOSS_WEAPON_SOCKET_TRANSFORM& value, std::string& status);
	static const BOSS_COMBAT_OBJECT_VISUAL_ENTRY*
		Find_BossCombatObjectVisual(
			std::string_view bossArchetypeId,
			std::string_view combatObjectArchetypeId,
			std::string_view clientVisualId);
	static const NPC_ACTOR_ENTRY* Find_Npc(std::string_view archetypeId);
	static const std::vector<NPC_ACTOR_ENTRY>& Get_Npcs();
	static const VEHICLE_ACTOR_ENTRY* Find_Vehicle(std::uint32_t vehicleId);
	static const VEHICLE_ACTOR_ENTRY* Find_VehicleByArchetype(std::string_view archetypeId);
	static const std::vector<VEHICLE_ACTOR_ENTRY>& Get_Vehicles();
	static const MONSTER_ACTOR_ENTRY* Find_Monster(
		std::string_view archetypeId);
	static const std::vector<MONSTER_ACTOR_ENTRY>& Get_Monsters();
	// 마지막 초기화 결과 문자열을 차용한다. 이 조회 자체는 초기화를 시작하지 않는다.
	static const std::string& Get_Status();
};

NS_END
