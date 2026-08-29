#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

struct VALTAN_COMBAT_OBJECT_SOUND_CUE final
{
	std::string strBindingId;
	std::string strCombatObjectArchetypeId;
	std::string strHitId;
	std::string strSoundBank;
	std::string strSoundEvent;
};

struct VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT final
{
	uint32_t iFormatVersion = 0u;
	std::string strOwnerArchetypeId;
	std::vector<VALTAN_COMBAT_OBJECT_SOUND_CUE> Cues;
};

/* Joins a presentation-only Sound binding to the Server-owned semantic hit.
The combat object Product document proves the source tuple exists; the Server
still transmits only that tuple and never learns an asset path. */
class CValtanCombatObjectSoundCueDocument final
{
public:
	static std::filesystem::path Resolve_Path();
	static std::filesystem::path Resolve_CombatObjectProductPath();

	static bool_t Parse_Text(
		std::string_view strCueText,
		std::string_view strCombatObjectProductText,
		VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& inOutDocument,
		std::string& strOutStatus);

	static bool_t Load_Source(
		VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& inOutDocument,
		std::string& strOutStatus);
	static bool_t Validate_SourceDraft(
		const VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& document,
		std::string& strOutStatus);
	/* Whole-file atomic replace. Validation runs before the destination is
	   touched, so malformed joins or missing catalog assets preserve the
	   previous admitted source. */
	static bool_t Save_Source(
		const VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& document,
		std::string& strOutStatus);
};

NS_END
