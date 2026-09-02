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
	/* Exactly one semantic Server event identity is authored. Damage-backed
	   pulses use hitId; visual-only terminal pulses use presentationEventId. */
	std::string strHitId;
	std::string strPresentationEventId;
	std::string strSoundBank;
	std::string strSoundEvent;
	/* Runtime-only assets pinned from the exact admitted catalog snapshot. */
	std::vector<std::string> ResolvedAssetIds;
};

struct VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT final
{
	uint32_t iFormatVersion = 0u;
	std::string strOwnerArchetypeId;
	std::vector<VALTAN_COMBAT_OBJECT_SOUND_CUE> Cues;
};

/* Opaque recovery state for a joined Workbench Save. Once Begin returns
   true, the caller must resolve the replacement exactly once with Commit or
   Rollback. The previous source remains recoverable until then. */
class VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT final
{
	friend class CValtanCombatObjectSoundCueDocument;

public:
	VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT() = default;
	VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT(
		const VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT&) = delete;
	VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT& operator=(
		const VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT&) = delete;
	VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT(
		VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT&&) noexcept = default;
	VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT& operator=(
		VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT&&) noexcept = default;

private:
	std::filesystem::path Destination;
	std::filesystem::path Rollback;
	bool_t bHadPrevious = false;
	bool_t bActive = false;
};

/* Joins a presentation-only Sound binding to one Server-owned semantic event.
The combat object Product document proves either the hitId or the
presentationEventId tuple exists; the Server still transmits only that stable
identity and never learns an asset path. */
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
	/* Recoverable whole-file replacement used by the Workbench's joined Save.
	   Begin validates and promotes the Sound source while retaining its exact
	   previous bytes. Commit discards that recovery copy only after the
	   gameplay Product save succeeds; Rollback restores it when a later domain
	   fails. */
	static bool_t Begin_SourceReplacement(
		const VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& document,
		VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT& transaction,
		std::string& strOutStatus);
	static bool_t Commit_SourceReplacement(
		VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT& transaction,
		std::string& strOutStatus);
	static bool_t Rollback_SourceReplacement(
		VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT& transaction,
		std::string& strOutStatus);
	/* Whole-file atomic replace. Validation runs before the destination is
	   touched, so malformed joins or missing catalog assets preserve the
	   previous admitted source. Standalone callers use the same recoverable
	   primitive and immediately commit it. */
	static bool_t Save_Source(
		const VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& document,
		std::string& strOutStatus);
};

NS_END
