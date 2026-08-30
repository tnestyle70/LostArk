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

struct EFFECT_V2_BINDING final
{
	/* Exactly one of strEffectId (leaf document) or strGroupId (group
	   document) names what spawns. */
	std::string strEffectId;
	std::string strGroupId;
	/* Exactly one of strClip (model clip start clock) or strStage (Server
	   pattern stage actionId, stage-local clock) keys the binding. */
	std::string strClip;
	std::string strStage;
	uint32_t iStartMs = 0u;
	std::string strBone;
	bool_t bFollowBone = true;
	CEffectV2Object::PIVOT_ROTATION eRotation = CEffectV2Object::PIVOT_ROTATION::TARGET_YAW;
	bool_t bStopWithClip = false;
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
	std::string strEffectId;
	uint32_t iStartMs = 0u;
	uint32_t iDurationMs = 0u;
	EFFECT_V2_CHILD_STOP eStop = EFFECT_V2_CHILD_STOP::DEACTIVATE;
	float3_t vOffset = { 0.f, 0.f, 0.f };
	f32_t fYawDegrees = 0.f;
};

/* A group owns clock and placement only; every look comes from the child
   documents. iDurationMs 0 ends with the last child, otherwise it caps
   every child stop. Children must be leaf documents (no nesting). */
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
