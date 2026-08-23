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
	std::string strEffectId;
	std::string strClip;
	uint32_t iStartMs = 0u;
	std::string strBone;
	bool_t bFollowBone = true;
	CEffectV2Object::PIVOT_ROTATION eRotation = CEffectV2Object::PIVOT_ROTATION::TARGET_YAW;
	bool_t bStopWithClip = false;
};

class CEffectV2Document final
{
public:
	static std::filesystem::path Document_Directory();
	static std::filesystem::path Binding_Directory();
	static std::filesystem::path Document_Path(const std::string& strEffectId);
	static std::filesystem::path Binding_Path(const std::string& strArchetypeId);
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
	static std::string Serialize_Document(const EFFECT_V2_DOCUMENT& Document);
	static std::string Serialize_Bindings(
		const std::string& strArchetypeId,
		const std::vector<EFFECT_V2_BINDING>& Bindings);

	static bool_t Load_DocumentFile(
		const std::string& strEffectId,
		EFFECT_V2_DOCUMENT& OutDocument,
		std::string& strOutError);
	static bool_t Load_BindingsFile(
		const std::string& strArchetypeId,
		std::vector<EFFECT_V2_BINDING>& OutBindings,
		std::string& strOutError);
	static bool_t Write_AtomicFile(
		const std::filesystem::path& Target,
		const std::string& strText,
		std::string& strOutError);

	static const char* Type_Key(EFFECT_V2_TYPE eType);
	static const char* Rotation_Key(CEffectV2Object::PIVOT_ROTATION eRotation);
	static CEffectV2Object::SHAPE Shape_ForType(EFFECT_V2_TYPE eType);
};

NS_END
