#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

enum class VALTAN_PATTERN_PREVIEW_EVIDENCE
{
	SOURCE_FAMILY_DIRECT,
	USER_CONFIRMED_FAMILY,
	CANDIDATE,
	COMPOSITE,
	UNRESOLVED,
	NO_ANIMATION,
	END,
};

/* A preview row names a source action route, never individual clips. The
   ordered clips, display name, and mode are resolved from Valtan.clipseq when
   the document is loaded. */
struct VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF final
{
	uint32_t iSourceActionId = 0u;
	int32_t iSequenceIndex = -1;
	uint32_t iRepeat = 1u;

	/* Resolution projection. These fields are not authored in patternpreview
	   JSON and remain empty until the exact (action, sequence) join succeeds. */
	std::string strSequenceName;
	std::string strSequenceMode;
	std::vector<std::string> ResolvedClips;

	bool_t Is_Resolved() const
	{
		return !strSequenceName.empty() && !strSequenceMode.empty() &&
			!ResolvedClips.empty();
	}

	bool operator==(
		const VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF&) const = default;
};

struct VALTAN_PATTERN_PREVIEW_ENTRY final
{
	uint32_t iNumber = 0u;
	std::string strLabel;
	VALTAN_PATTERN_PREVIEW_EVIDENCE eEvidence =
		VALTAN_PATTERN_PREVIEW_EVIDENCE::END;
	std::string strNote;
	std::vector<VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF> Sequences;

	bool operator==(const VALTAN_PATTERN_PREVIEW_ENTRY&) const = default;
};

struct VALTAN_PATTERN_PREVIEW_DOCUMENT final
{
	std::string strAnimationAssetId;
	std::vector<VALTAN_PATTERN_PREVIEW_ENTRY> Patterns;

	bool operator==(const VALTAN_PATTERN_PREVIEW_DOCUMENT&) const = default;
};

struct VALTAN_PATTERN_PREVIEW_PLAY_ITEM final
{
	uint32_t iPatternNumber = 0u;
	std::string strPatternLabel;
	VALTAN_PATTERN_PREVIEW_EVIDENCE eEvidence =
		VALTAN_PATTERN_PREVIEW_EVIDENCE::END;
	std::string strNote;

	uint32_t iSourceActionId = 0u;
	int32_t iSequenceIndex = -1;
	uint32_t iSequenceRepeatNumber = 0u;
	uint32_t iSequenceRepeatCount = 0u;
	uint32_t iSourceStepNumber = 0u;
	uint32_t iSourceStepCount = 0u;
	std::string strSequenceName;
	std::string strSequenceMode;
	std::string strClipName;

	uint32_t iStepNumber = 0u;
	uint32_t iStepCount = 0u;
	bool_t bPatternMarker = false;
	/* Source stage playback length from the .animnotify header, seconds. Zero
	means play the clip's full native model duration; shorter cuts the clip
	early and longer loops it, the way the source stage drives it. */
	f32_t fAuthoredDurationSeconds = 0.f;

	bool operator==(const VALTAN_PATTERN_PREVIEW_PLAY_ITEM&) const = default;
};

class CValtanPatternPreviewDocument final
{
public:
	static std::filesystem::path Resolve_Path();
	static std::filesystem::path Resolve_ClipSequencePath(
		std::string_view animationAssetId);

	/* Parse_Text parses only authored format-v2 references.
	   Resolve_SourceSequences performs the strict source join and model-clip
	   validation. */
	static bool_t Parse_Text(
		std::string_view text,
		VALTAN_PATTERN_PREVIEW_DOCUMENT& outDocument,
		std::string& outStatus);
	static bool_t Resolve_SourceSequences(
		const VALTAN_PATTERN_PREVIEW_DOCUMENT& document,
		std::string_view expectedAnimationAssetId,
		const std::vector<std::string>& availableClips,
		VALTAN_PATTERN_PREVIEW_DOCUMENT& outResolvedDocument,
		std::string& outStatus);
	static bool_t Validate(
		const VALTAN_PATTERN_PREVIEW_DOCUMENT& document,
		std::string_view expectedAnimationAssetId,
		const std::vector<std::string>& availableClips,
		std::string& outStatus);
	static bool_t Load(
		std::string_view expectedAnimationAssetId,
		const std::vector<std::string>& availableClips,
		VALTAN_PATTERN_PREVIEW_DOCUMENT& outDocument,
		std::string& outStatus);
	static bool_t Build_Playlist(
		const VALTAN_PATTERN_PREVIEW_DOCUMENT& resolvedDocument,
		uint32_t iFirstPattern,
		uint32_t iLastPattern,
		std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM>& outPlaylist,
		std::string& outStatus);
	static const char_t* Evidence_Name(
		VALTAN_PATTERN_PREVIEW_EVIDENCE eEvidence);
};

NS_END
