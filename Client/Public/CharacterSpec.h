#pragma once

// Client 이름 공간과 공통 선언을 사용한다.
#include "Client_Defines.h"
// clip index·색·logic 소유에 쓸 엔진 기본 타입을 사용한다.
#include "Engine_Defines.h"
// 직업별 선택적 천·머리카락 solver의 BONE_CHAIN_SPEC을 사용한다.
#include "BoneChainSimulation.h"
// Server와 공유하는 직업·stance·skill ID를 표현 설정에 연결한다.
#include "Network/PacketMessages.h"

NS_BEGIN(Client)

class CCharacter;

// 직업별 공통 표현 상태의 clip 이름을 배열에서 찾는 index다. 개별 skill clip은 이 배열 밖에서 처리한다.
enum class CHARACTER_ANIM {
	IDLE, RUN, HIT, DEAD,
	KNOCKDOWN, KNOCKDOWN_LAND, DOWN_LOOP, STANDUP,
	// 에스더 호출 표현 clip이다. 지정하지 않는 직업은 해당 배열 원소를 nullptr로 둔다.
	ESTHER_CAST,
	// 생성 화면의 idle 표현이다. clip이 없으면 공통 경로가 해당 직업의 일반 idle을 사용한다.
	CUSTOMIZING_IDLE,
	FEAR,
	// 스퀘어홀 노래 표현 clip이다. 지정하지 않은 직업은 nullptr다.
	SQUAREHOLE_SONG,
	END
};

// 직업별 로컬 표현 갱신 인터페이스다. 입력·전송·gameplay 판정 API는 이 계약에 없다.
class ICharacterLogic
{
public:
	// CCharacter의 unique_ptr<ICharacterLogic>가 파생 logic을 올바르게 소멸시킨다.
	virtual ~ICharacterLogic() = default;

public:
	virtual void Update_Presentation(
		CCharacter& Character,
		f32_t fTimeDelta) = 0;
};

// CCharacter가 base/avatar 표시 관계를 판단하는 종류다. DEFAULT_HELMET은 AVATAR_HEAD, 나머지 DEFAULT는 AVATAR_ARMOR에 가려진다.
enum class EQUIPMENT_SLOT_KIND
{
	DEFAULT,
	DEFAULT_HELMET,
	AVATAR_HEAD,
	AVATAR_ARMOR,
	// stance로 표시하는 전용 Part다. 의상 교체로 가리지 않으며 요구 stance가 지정된다.
	IDENTITY,
};

// 외형 편집에서 교체할 부위다. base/avatar 표시 관계인 EQUIPMENT_SLOT_KIND와는 역할이 다르다.
enum class EQUIPMENT_PRESENTATION_SLOT
{
	HEAD,
	SHOULDER,
	UPPER,
	LOWER,
	HANDS,
	WEAPON,
	END,
};

// END 미만 enum 값을 bit mask로 바꾸고 END 이상의 값은 0을 반환한다.
constexpr uint32_t EquipmentPresentationSlotMask(
	const EQUIPMENT_PRESENTATION_SLOT eSlot)
{
	return eSlot < EQUIPMENT_PRESENTATION_SLOT::END ?
		1u << ETOUI(eSlot) : 0u;
}

constexpr uint32_t EQUIPMENT_PRESENTATION_SLOT_MASK_ALL =
	(1u << ETOUI(EQUIPMENT_PRESENTATION_SLOT::END)) - 1u;

struct EQUIPMENT_PART_SPEC
{
	const tchar_t* pPartTag;      // 본체 뒤에 갱신되도록 정한 Part tag다.
	const tchar_t* pModelTag;

	// 장비 자체의 i번 submesh를 숨기는 mask다.
	uint32_t iHiddenMeshMask;
	// Part는 생성하되 처음에는 표시하지 않는다. 이후 공통 외형/stance 처리에서 표시를 조절한다.
	bool_t isHidden;

	EQUIPMENT_SLOT_KIND eSlotKind = EQUIPMENT_SLOT_KIND::DEFAULT;
	EQUIPMENT_PRESENTATION_SLOT ePresentationSlot =
		EQUIPMENT_PRESENTATION_SLOT::END;
	// NONE은 stance 제한이 없고 그 외 값은 해당 stance에서만 표시한다.
	LostArk::Shared::PLAYER_STANCE_ID eRequiredStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
};

// 공통 외형 승인 검사다. IDENTITY의 END 슬롯 예외는 GuardianKnight dragon wing 계약에만 허용한다.
constexpr bool_t Is_ValidEquipmentPresentationPart(
	const EQUIPMENT_PART_SPEC& part,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	using LostArk::Shared::CHARACTER_CLASS_ID;
	using LostArk::Shared::PLAYER_STANCE_ID;
	switch (part.eSlotKind)
	{
	case EQUIPMENT_SLOT_KIND::IDENTITY:
		return CHARACTER_CLASS_ID::GUARDIANKNIGHT == characterClass &&
			EQUIPMENT_PRESENTATION_SLOT::END == part.ePresentationSlot &&
			PLAYER_STANCE_ID::GUARDIANKNIGHT_DRAGON == part.eRequiredStance;
	case EQUIPMENT_SLOT_KIND::DEFAULT:
	case EQUIPMENT_SLOT_KIND::DEFAULT_HELMET:
	case EQUIPMENT_SLOT_KIND::AVATAR_HEAD:
	case EQUIPMENT_SLOT_KIND::AVATAR_ARMOR:
		return part.ePresentationSlot >= EQUIPMENT_PRESENTATION_SLOT::HEAD &&
			part.ePresentationSlot < EQUIPMENT_PRESENTATION_SLOT::WEAPON &&
			PLAYER_STANCE_ID::NONE == part.eRequiredStance;
	default:
		return false;
	}
}

// 단일 본에 붙이는 무기 Part의 정의다. CPart_Equipment 초기화는 실제 소켓이 없으면 실패한다.
struct WEAPON_PART_SPEC
{
	const tchar_t* pPartTag;      // 장비 뒤에 갱신되도록 정한 Part tag다.
	const tchar_t* pModelTag;
	const char_t* pSocketBone;
	f32_t fSocketYawDegrees = 0.f;
	LostArk::Shared::PLAYER_STANCE_ID eRequiredStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
};

// Server action snapshot의 skill ID에 대응하는 선택적 발광 표현 설정이다.
struct SKILL_SURFACE_EMISSIVE_SPEC
{
	LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
	float4_t vColor = float4_t(1.f, 1.f, 1.f, 1.f);
	f32_t fIntensity = 0.f;
};

// 지정한 stance에서 공통 IDLE/RUN을 대신할 clip이다. 목록에 없는 stance는 공통 clip을 사용한다.
struct STANCE_LOCOMOTION_SPEC
{
	LostArk::Shared::PLAYER_STANCE_ID eStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	const char_t* pIdleClip = nullptr;
	const char_t* pRunClip = nullptr;
};

// CCharacter가 공통 조립 경로에서 읽는 직업별 불변 설정이다. 각 직업 CPP의 정적 배열·문자열을 가리킨다.
struct CHARACTER_SPEC
{
	// Animation 등 직업별 데이터 경로를 찾는 stable asset 이름이다.
	const char_t* pAssetName;

	// snapshot 유무와 관계없이 표현 설정의 직업을 식별한다. 네트워크의 CHARACTER_CLASS_ID와 연결한다.
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass;

	const tchar_t* pBodyModelTag;
	const tchar_t* pShaderTag;

	// 장비가 대신 그리는 본체 submesh를 숨긴다.
	uint32_t iBodyHiddenMeshMask;

	// 본체에 포함된 머리카락 mask다. fallback 정책이 켜지면 HEAD 교체 전까지 유지하며 0은 숨길 비트가 없다는 뜻이다.
	uint32_t iBodyHairMeshMask;

	const tchar_t* pWeaponShaderTag;
	const WEAPON_PART_SPEC* pWeapons;
	uint32_t iNumWeapons;

	const EQUIPMENT_PART_SPEC* pEquipment;
	uint32_t iNumEquipment;

	// CHARACTER_ANIM 순서의 clip 이름이다. 실제 Model에서 이름으로 조회하며 nullptr인 선택 기능도 있다.
	const char_t* AnimationClips[ETOUI(CHARACTER_ANIM::END)];

	// 로컬 Character가 필요할 때 호출하는 factory다. 반환 logic은 그 Character가 단독 소유한다.
	unique_ptr<ICharacterLogic> (*pCreateLogic)();

	const SKILL_SURFACE_EMISSIVE_SPEC* pSkillSurfaceEmissives = nullptr;
	uint32_t iNumSkillSurfaceEmissives = 0u;

	const STANCE_LOCOMOTION_SPEC* pStanceLocomotion = nullptr;
	uint32_t iNumStanceLocomotion = 0u;

	// animation 뒤에 추가로 계산할 머리카락·천 체인이다. 비어 있으면 이 spec의 체인 solver가 활성화되지 않는다.
	const BONE_CHAIN_SPEC* pBoneChains = nullptr;
	uint32_t iNumBoneChains = 0u;

	// FaceSliders/<race>.facesliders.json을 고를 종족 코드다. nullptr면 해당 slider 설정을 지정하지 않는다.
	const char_t* pFaceSliderRace = nullptr;

	// 기본 hair Part가 없는 경우 HEAD 교체 commit 전까지 본체 머리카락을 유지하도록 한다.
	bool_t isBodyHairFallback = false;
};

NS_END
