#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>

NS_BEGIN(Client)

enum class COMPOSITION_SOURCE_STATUS : uint8_t
{
	SHADOW,
	REFERENCE_ONLY,
	AUTHORITATIVE,
};

struct BOSS_COMPOSITION_SOURCE_DOCUMENT final
{
	std::string role;
	std::string path;
};

struct BOSS_COMPOSITION_REFERENCE_PROFILE final
{
	std::string profileId;
	std::string actionReferencePath;
	std::string actionBindingPath;
	std::string patternBindingPath;
	std::string expectedReferenceRevision;
	uint32_t expectedActionCount = 0u;
};

struct BOSS_COMPOSITION_COVERAGE final
{
	std::string kind;
	std::string expectedIdentitySha256;
	uint32_t expectedPatternCount = 0u;
	uint32_t expectedStageCount = 0u;
	uint32_t expectedProfileCount = 0u;
	uint32_t expectedActionCount = 0u;
	std::vector<BOSS_COMPOSITION_REFERENCE_PROFILE> profiles;
};

struct BOSS_COMPOSITION_PATTERN final
{
	std::string patternId;
};

/* Authoring-facade view of one boss composition. It deliberately does not
   execute gameplay or presentation: SHADOW documents join the existing typed
   Valtan owners for inspection, while the Server/Product paths remain the
   only runtime authority until an explicit migration receipt promotes the
   document to AUTHORITATIVE. */
class CBossCompositionDocument final
{
public:
	bool_t Load(
		const std::filesystem::path& path,
		std::string& outStatus);
	void Clear();

	[[nodiscard]] const std::string& Get_CompositionId() const noexcept
	{
		return m_CompositionId;
	}
	[[nodiscard]] const std::string& Get_BossArchetypeId() const noexcept
	{
		return m_BossArchetypeId;
	}
	[[nodiscard]] const std::string& Get_EncounterId() const noexcept
	{
		return m_EncounterId;
	}
	[[nodiscard]] const std::string& Get_AreaId() const noexcept
	{
		return m_AreaId;
	}
	[[nodiscard]] uint32_t Get_Revision() const noexcept
	{
		return m_Revision;
	}
	[[nodiscard]] COMPOSITION_SOURCE_STATUS Get_Status() const noexcept
	{
		return m_Status;
	}
	[[nodiscard]] const std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT>&
	Get_SourceDocuments() const noexcept
	{
		return m_SourceDocuments;
	}
	[[nodiscard]] const std::string& Get_DisplayName() const noexcept
	{
		return m_DisplayName;
	}
	[[nodiscard]] const BOSS_COMPOSITION_COVERAGE& Get_Coverage() const noexcept
	{
		return m_Coverage;
	}
	[[nodiscard]] const std::vector<BOSS_COMPOSITION_PATTERN>&
	Get_Patterns() const noexcept
	{
		return m_Patterns;
	}
	[[nodiscard]] const BOSS_COMPOSITION_PATTERN* Find_Pattern(
		const std::string& patternId) const;

	static const char_t* Status_ToString(COMPOSITION_SOURCE_STATUS status);

private:
	std::string m_CompositionId;
	std::string m_DisplayName;
	std::string m_BossArchetypeId;
	std::string m_EncounterId;
	std::string m_AreaId;
	uint32_t m_Revision = 0u;
	COMPOSITION_SOURCE_STATUS m_Status = COMPOSITION_SOURCE_STATUS::SHADOW;
	std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT> m_SourceDocuments;
	BOSS_COMPOSITION_COVERAGE m_Coverage;
	std::vector<BOSS_COMPOSITION_PATTERN> m_Patterns;
};

enum class ARENA_SEQUENCER_TRACK_KIND : uint8_t
{
	WORLD_SEQUENCE,
	CAMERA_SHOT,
	ACTOR_PATTERN,
};

struct ARENA_WORLD_SEQUENCE_REFERENCE final
{
	std::string instanceId;
};

struct ARENA_CAMERA_SHOT_REFERENCE final
{
	std::string shotId;
};

struct ARENA_ACTOR_PATTERN_REFERENCE final
{
	std::string bossCompositionId;
	std::string patternId;
};

using ARENA_SEQUENCER_TRACK_REFERENCE = std::variant<
	ARENA_WORLD_SEQUENCE_REFERENCE,
	ARENA_CAMERA_SHOT_REFERENCE,
	ARENA_ACTOR_PATTERN_REFERENCE>;

struct ARENA_SEQUENCER_TRACK final
{
	std::string trackId;
	ARENA_SEQUENCER_TRACK_KIND kind =
		ARENA_SEQUENCER_TRACK_KIND::WORLD_SEQUENCE;
	ARENA_SEQUENCER_TRACK_REFERENCE reference;
	uint32_t startMs = 0u;
	uint32_t endMs = 0u;
	bool_t hasEndMs = false;
};

/* The arena document is a scheduling facade over existing typed owners. The
   payload stays with its Camera/World/Effect/UI owner; this reader retains the
   common identity and clock required by the unified Sequencer surface. */
class CArenaSequencerDocument final
{
public:
	bool_t Load(
		const std::filesystem::path& path,
		std::string& outStatus);
	void Clear();

	[[nodiscard]] const std::string& Get_SequencerId() const noexcept
	{
		return m_SequencerId;
	}
	[[nodiscard]] const std::string& Get_AreaId() const noexcept
	{
		return m_AreaId;
	}
	[[nodiscard]] uint32_t Get_Revision() const noexcept
	{
		return m_Revision;
	}
	[[nodiscard]] uint32_t Get_DurationMs() const noexcept
	{
		return m_DurationMs;
	}
	[[nodiscard]] COMPOSITION_SOURCE_STATUS Get_Status() const noexcept
	{
		return m_Status;
	}
	[[nodiscard]] const std::string& Get_DisplayName() const noexcept
	{
		return m_DisplayName;
	}
	[[nodiscard]] const std::string& Get_BossCompositionId() const noexcept
	{
		return m_BossCompositionId;
	}
	[[nodiscard]] const std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT>&
	Get_SourceDocuments() const noexcept
	{
		return m_SourceDocuments;
	}
	[[nodiscard]] const std::vector<ARENA_SEQUENCER_TRACK>&
	Get_Tracks() const noexcept
	{
		return m_Tracks;
	}
	static const char_t* TrackKind_ToString(ARENA_SEQUENCER_TRACK_KIND kind);

private:
	std::string m_SequencerId;
	std::string m_DisplayName;
	std::string m_AreaId;
	std::string m_BossCompositionId;
	uint32_t m_Revision = 0u;
	uint32_t m_DurationMs = 0u;
	COMPOSITION_SOURCE_STATUS m_Status = COMPOSITION_SOURCE_STATUS::SHADOW;
	std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT> m_SourceDocuments;
	std::vector<ARENA_SEQUENCER_TRACK> m_Tracks;
};

/* Loads one selected v1 Boss/Arena descriptor pair into local staging and
   commits only after their identity, Area and actor-Pattern references agree.
   A malformed unrelated boss must not block the selected pair. */
class CCompositionDocumentCatalog final
{
public:
	bool_t Load_Pair(
		const std::string& compositionId,
		const std::string& sequencerId,
		std::string& outStatus);
	void Clear();
	[[nodiscard]] const CBossCompositionDocument* Find_Boss(
		const std::string& compositionId) const;
	[[nodiscard]] const CArenaSequencerDocument* Find_Arena(
		const std::string& sequencerId) const;

private:
	std::vector<CBossCompositionDocument> m_BossDocuments;
	std::vector<CArenaSequencerDocument> m_ArenaDocuments;
};

NS_END
