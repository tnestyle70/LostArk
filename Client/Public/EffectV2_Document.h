#pragma once

#include "Client_Defines.h"
#include "EffectV2_Object.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class EFFECT_V2_TYPE : int32_t
{
	MESH,
	TEXTURE,
	PARTICLE,
	DECAL,
	TRAIL,
	SCREEN_POST,
	END
};

struct EFFECT_V2_PART_OVERRIDE final
{
	bool_t bVisible = true;
	std::string strBaseAssetId;
};

struct EFFECT_V2_DOCUMENT final
{
	std::string strEffectId;
	EFFECT_V2_TYPE eType = EFFECT_V2_TYPE::MESH;
	CEffectV2Object::DESC Desc;
	std::vector<EFFECT_V2_PART_OVERRIDE> Parts;
	std::string strAnimationClip;
};

enum class EFFECT_V2_RESOURCE_KIND : int32_t
{
	LEAF,
	GROUP,
	END
};

enum class EFFECT_V2_CLOCK_BASIS : int32_t
{
	STAGE,
	CLIP_OCCURRENCE,
	END
};

enum class EFFECT_V2_REPEAT_POLICY : int32_t
{
	ONCE,
	EACH_LOOP,
	END
};

enum class EFFECT_V2_FOLLOW_POLICY : int32_t
{
	FOLLOW_SLOT,
	SNAPSHOT_AT_START,
	END
};

enum class EFFECT_V2_ROTATION_BASIS : int32_t
{
	SLOT,
	TARGET_YAW,
	WORLD,
	END
};

enum class EFFECT_V2_STOP_POLICY : int32_t
{
	NATURAL,
	STAGE_END,
	CLIP_OCCURRENCE_END,
	EXPLICIT,
	END
};

struct EFFECT_V2_LOCAL_TRANSFORM final
{
	float3_t vTranslation = { 0.f, 0.f, 0.f };
	/* Pitch, yaw and roll in degrees. */
	float3_t vRotation = { 0.f, 0.f, 0.f };
	float3_t vScale = { 1.f, 1.f, 1.f };
};

struct EFFECT_V2_BINDING final
{
	/* Stable formatVersion 2 identity and typed document fields. */
	std::string strBindingId;
	EFFECT_V2_RESOURCE_KIND eResourceKind = EFFECT_V2_RESOURCE_KIND::LEAF;
	std::string strResourceId;
	std::string strPatternId;
	std::string strStageId;
	std::string strActionId;
	EFFECT_V2_CLOCK_BASIS eClockBasis = EFFECT_V2_CLOCK_BASIS::STAGE;
	std::string strClipOccurrenceId;
	EFFECT_V2_REPEAT_POLICY eRepeatPolicy = EFFECT_V2_REPEAT_POLICY::ONCE;
	std::string strAnchorSlotId;
	EFFECT_V2_FOLLOW_POLICY eFollowPolicy = EFFECT_V2_FOLLOW_POLICY::FOLLOW_SLOT;
	EFFECT_V2_ROTATION_BASIS eRotationBasis = EFFECT_V2_ROTATION_BASIS::TARGET_YAW;
	EFFECT_V2_LOCAL_TRANSFORM LocalTransform;
	EFFECT_V2_STOP_POLICY eStopPolicy = EFFECT_V2_STOP_POLICY::NATURAL;

	/* Runtime convenience mirrors. Parse_Bindings fills these from the typed
	   v2 fields so existing consumers can migrate without an ABI-breaking
	   replacement of their member access. */
	std::string strEffectId;
	std::string strGroupId;
	std::string strClip;
	std::string strStage;
	uint32_t iStartMs = 0u;
	std::string strBone;
	bool_t bFollowBone = true;
	CEffectV2Object::PIVOT_ROTATION eRotation = CEffectV2Object::PIVOT_ROTATION::TARGET_YAW;
	bool_t bStopWithClip = false;
	/* Pivot-local placement of this binding; group child offsets stack on
	   top, so one group re-aims per binding row. */
	float3_t vOffset = { 0.f, 0.f, 0.f };
	f32_t fYawDegrees = 0.f;
};

enum class EFFECT_V2_CHILD_STOP : int32_t
{
	KILL,
	DEACTIVATE,
	END
};

/* One emitter slot of a group. Times are group-local milliseconds; the
   binding that plays the group adds its own startMs. iDurationMs 0 leaves
   the child to its document lifetime. vOffset/fYawDegrees place the child
   relative to the group pivot and survive bone following. */
struct EFFECT_V2_GROUP_CHILD final
{
	std::string strChildId;
	EFFECT_V2_RESOURCE_KIND eResourceKind = EFFECT_V2_RESOURCE_KIND::LEAF;
	std::string strResourceId;
	EFFECT_V2_LOCAL_TRANSFORM LocalTransform;

	/* Runtime convenience mirrors for the leaf-only consumer during its
	   migration to typed nested resources. */
	std::string strEffectId;
	std::string strGroupId;
	uint32_t iStartMs = 0u;
	uint32_t iDurationMs = 0u;
	EFFECT_V2_CHILD_STOP eStop = EFFECT_V2_CHILD_STOP::DEACTIVATE;
	float3_t vOffset = { 0.f, 0.f, 0.f };
	f32_t fPitchDegrees = 0.f;
	f32_t fYawDegrees = 0.f;
	f32_t fRollDegrees = 0.f;
	/* Multiplies the child document's scale track; particle sprite sizes
	   take the X component uniformly. */
	float3_t vScale = { 1.f, 1.f, 1.f };
};

/* A group owns clock and placement only; every look comes from a typed leaf
   or nested group resource. iDurationMs 0 ends with the last child, otherwise
   it caps every child stop. Authored array order is deterministic spawn order;
   strChildId is the stable mutation identity. */
struct EFFECT_V2_GROUP final
{
	std::string strGroupId;
	uint32_t iDurationMs = 0u;
	std::vector<EFFECT_V2_GROUP_CHILD> Children;
};

class CEffectV2Document final
{
public:
	static std::filesystem::path Document_Directory();
	static std::filesystem::path Binding_Directory();
	static std::filesystem::path Group_Directory();
	static std::filesystem::path Document_Path(const std::string& strEffectId);
	static std::filesystem::path Binding_Path(const std::string& strArchetypeId);
	static std::filesystem::path Group_Path(const std::string& strGroupId);
	static bool_t Is_ValidEffectId(const std::string& strEffectId);

	static bool_t Parse_Document(
		const std::string& strText,
		EFFECT_V2_DOCUMENT& OutDocument,
		std::string& strOutError);
	static bool_t Parse_Bindings(
		const std::string& strText,
		const std::string& strExpectedArchetypeId,
		std::vector<EFFECT_V2_BINDING>& OutBindings,
		std::string& strOutError);
	static bool_t Parse_Group(
		const std::string& strText,
		EFFECT_V2_GROUP& OutGroup,
		std::string& strOutError);
	static std::string Serialize_Document(const EFFECT_V2_DOCUMENT& Document);
	static std::string Serialize_Bindings(
		const std::string& strArchetypeId,
		const std::vector<EFFECT_V2_BINDING>& Bindings);
	static std::string Serialize_Group(const EFFECT_V2_GROUP& Group);

	static bool_t Load_DocumentFile(
		const std::string& strEffectId,
		EFFECT_V2_DOCUMENT& OutDocument,
		std::string& strOutError);
	static bool_t Load_BindingsFile(
		const std::string& strArchetypeId,
		std::vector<EFFECT_V2_BINDING>& OutBindings,
		std::string& strOutError);
	static bool_t Load_GroupFile(
		const std::string& strGroupId,
		EFFECT_V2_GROUP& OutGroup,
		std::string& strOutError);
	static bool_t Write_AtomicFile(
		const std::filesystem::path& Target,
		const std::string& strText,
		std::string& strOutError);

	static const char* Type_Key(EFFECT_V2_TYPE eType);
	static const char* Rotation_Key(CEffectV2Object::PIVOT_ROTATION eRotation);
	static const char* Child_Stop_Key(EFFECT_V2_CHILD_STOP eStop);
	static CEffectV2Object::SHAPE Shape_ForType(EFFECT_V2_TYPE eType);
};

NS_END
