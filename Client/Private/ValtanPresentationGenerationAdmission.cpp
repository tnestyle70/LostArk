#include "ValtanPresentationGenerationAdmission.h"

#include "DataJson.h"
#include "EffectV2_Document.h"
#include "ProjectDataRoot.h"
#include "ValtanPatternTree.h"

#include <algorithm>
#include <array>
#include <bcrypt.h>
#include <cwctype>
#include <fstream>
#include <functional>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	using Client::DATA_JSON_VALUE;
	using Client::VALTAN_PRESENTATION_GENERATION_ARTIFACT_RECEIPT;
	using Client::VALTAN_PRESENTATION_GENERATION_RECEIPT;
	using LostArk::Shared::GameplayDataRevision;

	constexpr std::uint64_t MAX_ARTIFACT_BYTES = 64ull * 1024ull * 1024ull;
	constexpr std::string_view EFFECT_V2_BINDINGS_RELATIVE =
		"Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json";
	constexpr std::string_view EFFECT_V2_AUTHORED_ROOT =
		"Data/Effects/V2/Authored";
	constexpr std::string_view EFFECT_V2_GROUP_ROOT =
		"Data/Effects/V2/Groups";
	constexpr std::string_view EFFECT_V2_DOCUMENT_SUFFIX = ".effectv2.json";
	constexpr std::string_view EFFECT_V2_GROUP_SUFFIX = ".effectv2group.json";

	const std::map<std::string, std::string> FIXED_ARTIFACTS{
		{ "Data/Animation/Authored/Valtan/Valtan.patternbindings.json", "ANIMATION" },
		{ "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json", "EFFECT" },
		{ "Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json", "EFFECT" },
		{ "Data/Animation/Authored/Valtan/Valtan.patternshakecues.json", "CAMERA" },
		{ "Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json", "COMBAT_VISUAL" },
		{ "Data/Sound/CharacterSoundCatalog.json", "COMBAT_VISUAL" },
		{ "Data/Effects/EffectCatalog.json", "EFFECT" },
		{ std::string(EFFECT_V2_BINDINGS_RELATIVE), "EFFECT" },
		{ "Data/Encounters/Valtan/ValtanEncounter.json", "COMBAT_VISUAL" },
		{ "Data/Encounters/Valtan/ValtanCombatObjects.json", "COMBAT_VISUAL" },
		{ "Data/Actors/BossCatalog.json", "COMBAT_VISUAL" },
		{ "Data/Encounters/Valtan/ValtanCinematicCamera.json", "CAMERA" },
		{ "Data/Encounters/Valtan/ValtanWorldEvents.json", "WORLD_EVENT_SET" },
	};

	bool Is_SafeRelativePath(const std::string& text)
	{
		if (text.empty() || '/' == text.front() || '\\' == text.front() ||
			std::string::npos != text.find('\\') ||
			(text.size() >= 2u && ':' == text[1]))
		{
			return false;
		}
		std::stringstream input(text);
		std::string part;
		while (std::getline(input, part, '/'))
		{
			if (part.empty() || "." == part || ".." == part)
				return false;
		}
		return true;
	}

	bool Is_Descendant(
		const std::filesystem::path& root,
		const std::filesystem::path& candidate)
	{
		std::error_code error;
		const auto canonicalRoot = std::filesystem::weakly_canonical(root, error);
		if (error || canonicalRoot.empty())
			return false;
		error.clear();
		const auto canonicalCandidate =
			std::filesystem::weakly_canonical(candidate, error);
		if (error || canonicalCandidate.empty())
			return false;
		auto rootPart = canonicalRoot.begin();
		auto candidatePart = canonicalCandidate.begin();
		for (; rootPart != canonicalRoot.end(); ++rootPart, ++candidatePart)
		{
			if (candidatePart == canonicalCandidate.end())
				return false;
			std::wstring left = rootPart->native();
			std::wstring right = candidatePart->native();
			std::transform(left.begin(), left.end(), left.begin(), ::towlower);
			std::transform(right.begin(), right.end(), right.begin(), ::towlower);
			if (left != right)
				return false;
		}
		return candidatePart != canonicalCandidate.end();
	}

	bool Read_File(
		const std::filesystem::path& path,
		std::string& bytes,
		std::string& status)
	{
		std::error_code error;
		const std::uint64_t size = std::filesystem::file_size(path, error);
		if (error || 0u == size || size > MAX_ARTIFACT_BYTES ||
			size > static_cast<std::uint64_t>(
				(std::numeric_limits<std::streamsize>::max)()))
		{
			status = "Valtan presentation artifact size is invalid: " + path.string();
			return false;
		}
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			status = "Valtan presentation artifact is missing: " + path.string();
			return false;
		}
		std::string staged(static_cast<std::size_t>(size), '\0');
		input.read(staged.data(), static_cast<std::streamsize>(staged.size()));
		if (!input || input.gcount() != static_cast<std::streamsize>(staged.size()))
		{
			status = "Valtan presentation artifact read was incomplete: " +
				path.string();
			return false;
		}
		bytes = std::move(staged);
		return true;
	}

	bool Hash_Bytes(
		const std::string_view bytes,
		GameplayDataRevision& revision)
	{
		if (bytes.empty() || bytes.size() >
			static_cast<std::size_t>((std::numeric_limits<ULONG>::max)()))
		{
			return false;
		}
		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD objectBytes = 0u;
		DWORD hashBytes = 0u;
		DWORD written = 0u;
		std::vector<unsigned char> hashObject;
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(
				&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
			0 <= BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&objectBytes), sizeof(objectBytes),
				&written, 0u) &&
			0 <= BCryptGetProperty(algorithm, BCRYPT_HASH_LENGTH,
				reinterpret_cast<PUCHAR>(&hashBytes), sizeof(hashBytes),
				&written, 0u) && hashBytes == revision.Bytes.size())
		{
			hashObject.resize(objectBytes);
			if (0 <= BCryptCreateHash(algorithm, &hash, hashObject.data(),
					objectBytes, nullptr, 0u, 0u) &&
				0 <= BCryptHashData(hash, reinterpret_cast<PUCHAR>(
						const_cast<char*>(bytes.data())),
					static_cast<ULONG>(bytes.size()), 0u) &&
				0 <= BCryptFinishHash(hash, revision.Bytes.data(), hashBytes, 0u))
			{
				succeeded = revision.Is_Valid();
			}
		}
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0u);
		return succeeded;
	}

	std::vector<std::string_view> Split_Tabs(const std::string& line)
	{
		std::vector<std::string_view> fields;
		std::string_view remaining(line);
		for (;;)
		{
			const std::size_t split = remaining.find('\t');
			fields.push_back(remaining.substr(0u, split));
			if (std::string_view::npos == split)
				break;
			remaining.remove_prefix(split + 1u);
		}
		return fields;
	}

	bool Parse_Bootstrap(
		const std::string& bytes,
		GameplayDataRevision& gameplayRevision,
		GameplayDataRevision& generationId,
		std::string& status)
	{
		if (!Hash_Bytes(bytes, gameplayRevision))
		{
			status = "Gameplay.bootstrap SHA-256 could not be calculated.";
			return false;
		}
		std::istringstream input(bytes);
		std::string line;
		if (!std::getline(input, line))
		{
			status = "Gameplay.bootstrap is empty.";
			return false;
		}
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
		const auto header = Split_Tabs(line);
		std::uint32_t version = 0u;
		std::uint32_t rowCount = 0u;
		try
		{
			if (3u != header.size() ||
				"LOSTARK_GAMEPLAY_BOOTSTRAP" != header[0])
				throw std::invalid_argument("header");
			version = static_cast<std::uint32_t>(std::stoul(std::string(header[1])));
			rowCount = static_cast<std::uint32_t>(std::stoul(std::string(header[2])));
		}
		catch (const std::exception&)
		{
			status = "Gameplay.bootstrap header is invalid.";
			return false;
		}
		if (LostArk::Shared::GAMEPLAY_BOOTSTRAP_FORMAT_VERSION != version ||
			0u == rowCount ||
			rowCount > 4096u)
		{
			status = "Gameplay.bootstrap version or row count is invalid.";
			return false;
		}
		for (std::uint32_t index = 0u; index < rowCount; ++index)
		{
			if (!std::getline(input, line))
			{
				status = "Gameplay.bootstrap row set is truncated.";
				return false;
			}
			if (!line.empty() && '\r' == line.back())
				line.pop_back();
			const auto fields = Split_Tabs(line);
			if (!fields.empty() && "PATTERNPRESENTATIONGENERATION" == fields[0])
			{
				if (3u != fields.size() || "ENCOUNTER_VALTAN" != fields[1] ||
					generationId.Is_Valid() ||
					!LostArk::Shared::Try_Parse_GameplayDataRevision(
						fields[2], generationId))
				{
					status = "Gameplay.bootstrap presentation generation row is invalid.";
					return false;
				}
			}
		}
		if (!generationId.Is_Valid())
		{
			status = "Gameplay.bootstrap has no Valtan presentation generation.";
			return false;
		}
		return true;
	}

	bool Read_String(
		const DATA_JSON_VALUE& object,
		const char* field,
		std::string& value)
	{
		const DATA_JSON_VALUE* found = object.Find(field);
		if (nullptr == found || !found->Is_String())
			return false;
		value = found->Get_String();
		return true;
	}

	bool Read_Unsigned(
		const DATA_JSON_VALUE& object,
		const char* field,
		std::uint64_t& value)
	{
		const DATA_JSON_VALUE* found = object.Find(field);
		if (nullptr == found || !found->Is_Number() ||
			found->Was_FloatingPointToken() || found->Get_Number() < 0.0 ||
			found->Get_Number() > static_cast<double>(
				(std::numeric_limits<std::uint64_t>::max)()))
		{
			return false;
		}
		value = static_cast<std::uint64_t>(found->Get_Number());
		return static_cast<double>(value) == found->Get_Number();
	}

	bool Has_ExactProperties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> properties)
	{
		if (!object.Is_Object() || object.Get_Object().size() != properties.size())
			return false;
		return std::all_of(properties.begin(), properties.end(),
			[&object](const std::string_view property)
			{
				return nullptr != object.Find(property);
			});
	}

	bool Is_StableEffectV2Id(const std::string& id)
	{
		return !id.empty() && id.size() <= 80u &&
			std::all_of(id.begin(), id.end(), [](const char value)
			{
				return ('A' <= value && value <= 'Z') ||
					('a' <= value && value <= 'z') ||
					('0' <= value && value <= '9') ||
					'.' == value || '_' == value || '-' == value;
			});
	}

	bool Build_EffectV2Relative(
		const std::string_view relativeRoot,
		const std::string& stableId,
		const std::string_view suffix,
		const std::string_view context,
		std::string& relative,
		std::string& status)
	{
		if (!Is_StableEffectV2Id(stableId))
		{
			status = std::string(context) +
				" is not a stable Effect V2 ID: " + stableId;
			return false;
		}
		relative = std::string(relativeRoot) + "/" + stableId +
			std::string(suffix);
		if (!Is_SafeRelativePath(relative))
		{
			status = std::string(context) +
				" produced an unsafe Effect V2 path: " + relative;
			return false;
		}
		return true;
	}

	bool Read_EffectV2Json(
		const std::filesystem::path& root,
		const std::string& relative,
		const std::string_view context,
		DATA_JSON_VALUE& document,
		std::string& status,
		std::string* const pSourceBytes = nullptr)
	{
		if (!Is_SafeRelativePath(relative))
		{
			status = std::string(context) +
				" path is unsafe: " + relative;
			return false;
		}
		const std::filesystem::path physical =
			root / std::filesystem::path(relative);
		if (!Is_Descendant(root, physical))
		{
			status = std::string(context) +
				" escaped the repository root: " + relative;
			return false;
		}
		std::error_code error;
		const std::filesystem::file_status fileStatus =
			std::filesystem::symlink_status(physical, error);
		if (error || !std::filesystem::is_regular_file(fileStatus) ||
			std::filesystem::is_symlink(fileStatus))
		{
			status = std::string(context) +
				" is missing or is not a regular file: " + relative;
			return false;
		}

		std::string bytes;
		if (!Read_File(physical, bytes, status))
			return false;
		if (nullptr != pSourceBytes)
			*pSourceBytes = bytes;
		std::string parseError;
		if (!Client::CDataJson::Parse(bytes, document, parseError) ||
			!document.Is_Object())
		{
			status = std::string(context) +
				" JSON parse failed: " + parseError;
			return false;
		}
		return true;
	}

	bool Read_EffectV2BindingIdentity(
		const DATA_JSON_VALUE& row,
		const std::size_t ordinal,
		std::set<std::tuple<std::string, std::string, std::string,
			std::string, std::uint64_t, std::string>>& identities,
		std::set<std::string>& leafIds,
		std::set<std::string>& groupIds,
		std::string& status)
	{
		const std::string context =
			"BOSS_VALTAN Effect V2 bindings[" + std::to_string(ordinal) + "]";
		if (!row.Is_Object())
		{
			status = context + " is not an object.";
			return false;
		}

		const DATA_JSON_VALUE* effect = row.Find("effectId");
		const DATA_JSON_VALUE* group = row.Find("group");
		const bool hasEffect = nullptr != effect;
		const bool hasGroup = nullptr != group;
		if (hasEffect == hasGroup)
		{
			status = context +
				" must contain exactly one of effectId/group.";
			return false;
		}

		std::string effectId;
		std::string groupId;
		std::string ignoredRelative;
		if (hasEffect)
		{
			if (!Read_String(row, "effectId", effectId))
			{
				status = context + ".effectId is invalid.";
				return false;
			}
			if (!Build_EffectV2Relative(EFFECT_V2_AUTHORED_ROOT,
					effectId, EFFECT_V2_DOCUMENT_SUFFIX,
					context + ".effectId", ignoredRelative, status))
			{
				return false;
			}
			leafIds.insert(effectId);
		}
		else
		{
			if (!Read_String(row, "group", groupId))
			{
				status = context + ".group is invalid.";
				return false;
			}
			if (!Build_EffectV2Relative(EFFECT_V2_GROUP_ROOT,
					groupId, EFFECT_V2_GROUP_SUFFIX,
					context + ".group", ignoredRelative, status))
			{
				return false;
			}
			groupIds.insert(groupId);
		}

		const DATA_JSON_VALUE* clip = row.Find("clip");
		const DATA_JSON_VALUE* stage = row.Find("stage");
		const bool hasClip = nullptr != clip;
		const bool hasStage = nullptr != stage;
		std::string clipId;
		std::string stageId;
		std::string bone;
		std::uint64_t startMs = 0u;
		if (hasClip == hasStage ||
			(hasClip && (!Read_String(row, "clip", clipId) || clipId.empty())) ||
			(hasStage && (!Read_String(row, "stage", stageId) || stageId.empty())) ||
			!Read_Unsigned(row, "startMs", startMs) || startMs > 600000u ||
			!Read_String(row, "bone", bone))
		{
			status = context +
				" has an invalid exact subject/clock/startMs/bone identity.";
			return false;
		}

		const auto identity = std::make_tuple(
			effectId, groupId, clipId, stageId, startMs, bone);
		if (!identities.insert(identity).second)
		{
			status = context + " duplicates an earlier binding identity: " +
				(hasEffect ? effectId : groupId) + ".";
			return false;
		}
		return true;
	}

	bool Build_EffectV2Closure(
		const std::filesystem::path& root,
		std::set<std::string>& expected,
		std::string& status)
	{
		DATA_JSON_VALUE bindings;
		std::string bindingBytes;
		if (!Read_EffectV2Json(root, std::string(EFFECT_V2_BINDINGS_RELATIVE),
				"BOSS_VALTAN Effect V2 bindings", bindings, status, &bindingBytes))
		{
			return false;
		}
		std::string schema;
		std::string archetypeId;
		std::uint64_t version = 0u;
		const DATA_JSON_VALUE* rows = bindings.Find("bindings");
		if (!Has_ExactProperties(bindings,
				{ "schema", "formatVersion", "archetypeId", "bindings" }) ||
			!Read_String(bindings, "schema", schema) ||
			"lostark.effect-v2-bindings" != schema ||
			!Read_Unsigned(bindings, "formatVersion", version) || 2u != version ||
			!Read_String(bindings, "archetypeId", archetypeId) ||
			"BOSS_VALTAN" != archetypeId || nullptr == rows ||
			!rows->Is_Array() || rows->Get_Array().empty())
		{
			status = "BOSS_VALTAN Effect V2 binding header/rows are invalid.";
			return false;
		}

		std::vector<Client::EFFECT_V2_BINDING> parsedBindings;
		std::string parseError;
		if (!Client::CEffectV2Document::Parse_Bindings(
				bindingBytes, "BOSS_VALTAN", parsedBindings, parseError) ||
			parsedBindings.empty())
		{
			status = "BOSS_VALTAN Effect V2 bindings failed strict v2 admission: " +
				parseError;
			return false;
		}
		std::set<std::string> leafIds;
		std::set<std::string> groupIds;
		for (const Client::EFFECT_V2_BINDING& binding : parsedBindings)
		{
			if (Client::EFFECT_V2_RESOURCE_KIND::LEAF == binding.eResourceKind)
				leafIds.insert(binding.strResourceId);
			else
				groupIds.insert(binding.strResourceId);
		}

		DATA_JSON_VALUE bossCatalog;
		if (!Read_EffectV2Json(root, "Data/Actors/BossCatalog.json",
				"BossCatalog Effect V2 owners", bossCatalog, status))
		{
			return false;
		}
		std::string bossSchema;
		std::uint64_t bossVersion = 0u;
		const DATA_JSON_VALUE* bosses = bossCatalog.Find("bosses");
		if (!Has_ExactProperties(bossCatalog,
				{ "schema", "formatVersion", "bosses" }) ||
			!Read_String(bossCatalog, "schema", bossSchema) ||
			"lostark.boss-catalog" != bossSchema ||
			!Read_Unsigned(bossCatalog, "formatVersion", bossVersion) ||
			6u != bossVersion || nullptr == bosses || !bosses->Is_Array())
		{
			status = "BossCatalog Effect V2 owner header is invalid.";
			return false;
		}
		std::size_t valtanOwnerCount = 0u;
		for (const DATA_JSON_VALUE& boss : bosses->Get_Array())
		{
			std::string bossArchetypeId;
			if (!boss.Is_Object() ||
				!Read_String(boss, "archetypeId", bossArchetypeId) ||
				"BOSS_VALTAN" != bossArchetypeId)
			{
				continue;
			}
			++valtanOwnerCount;
			const DATA_JSON_VALUE* visuals = boss.Find("combatObjectVisuals");
			if (nullptr == visuals || !visuals->Is_Array())
			{
				status = "BOSS_VALTAN combatObjectVisuals are invalid.";
				return false;
			}
			for (const DATA_JSON_VALUE& visual : visuals->Get_Array())
			{
				const DATA_JSON_VALUE* group = visual.Is_Object() ?
					visual.Find("effectV2Group") : nullptr;
				if (nullptr == group)
					continue;
				std::string groupId;
				std::string ignoredRelative;
				if (!Has_ExactProperties(*group,
						{ "groupId", "playbackRate", "visualHitMs", "serverHitId" }) ||
					!Read_String(*group, "groupId", groupId) ||
					!Build_EffectV2Relative(EFFECT_V2_GROUP_ROOT, groupId,
						EFFECT_V2_GROUP_SUFFIX,
						"BossCatalog combat-object Effect V2 groupId",
						ignoredRelative, status))
				{
					return false;
				}
				groupIds.insert(groupId);
			}
		}
		if (1u != valtanOwnerCount)
		{
			status = "BossCatalog has no unique BOSS_VALTAN owner.";
			return false;
		}
		for (const std::string& leafId : leafIds)
		{
			if (groupIds.contains(leafId))
			{
				status =
					"BOSS_VALTAN Effect V2 IDs collide between a leaf and group: " +
					leafId;
				return false;
			}
		}

		for (const std::string& groupId : groupIds)
		{
			std::string relative;
			if (!Build_EffectV2Relative(EFFECT_V2_GROUP_ROOT, groupId,
					EFFECT_V2_GROUP_SUFFIX, "BOSS_VALTAN Effect V2 groupId",
					relative, status))
			{
				return false;
			}
			DATA_JSON_VALUE group;
			std::string groupBytes;
			if (!Read_EffectV2Json(root, relative,
					"BOSS_VALTAN Effect V2 group " + groupId, group, status,
					&groupBytes))
			{
				return false;
			}
			Client::EFFECT_V2_GROUP parsedGroup;
			if (!Client::CEffectV2Document::Parse_Group(
					groupBytes, parsedGroup, parseError) ||
				parsedGroup.strGroupId != groupId)
			{
				status = "BOSS_VALTAN Effect V2 group failed strict v2 admission: " +
					groupId + ": " + parseError;
				return false;
			}
			std::string loadedGroupId;
			std::string groupSchema;
			std::uint64_t groupVersion = 0u;
			const DATA_JSON_VALUE* children = group.Find("children");
			if (!Has_ExactProperties(group,
					{ "schema", "formatVersion", "groupId", "durationMs", "children" }) ||
				!Read_String(group, "schema", groupSchema) ||
				"lostark.effect-v2-group" != groupSchema ||
				!Read_Unsigned(group, "formatVersion", groupVersion) ||
				2u != groupVersion ||
				!Read_String(group, "groupId", loadedGroupId) ||
				loadedGroupId != groupId || nullptr == children ||
				!children->Is_Array() || children->Get_Array().empty())
			{
				status =
					"BOSS_VALTAN Effect V2 group identity/children are invalid: " +
					groupId;
				return false;
			}
			if (!expected.insert(relative).second)
			{
				status = "BOSS_VALTAN Effect V2 closure has a duplicate group path: " +
					relative;
				return false;
			}

			for (std::size_t ordinal = 0u;
				ordinal < parsedGroup.Children.size(); ++ordinal)
			{
				const std::string context = "BOSS_VALTAN Effect V2 group " +
					groupId + ".children[" + std::to_string(ordinal) + "]";
				const Client::EFFECT_V2_GROUP_CHILD& child =
					parsedGroup.Children[ordinal];
				std::string ignoredRelative;
				if (Client::EFFECT_V2_RESOURCE_KIND::GROUP == child.eResourceKind)
				{
					status = context +
						" references a nested group, which the current Valtan Effect V2 runtime does not admit.";
					return false;
				}
				if (!Build_EffectV2Relative(EFFECT_V2_AUTHORED_ROOT,
						child.strResourceId, EFFECT_V2_DOCUMENT_SUFFIX,
						context + ".resource.id",
						ignoredRelative, status))
				{
					return false;
				}
				if (child.strResourceId == groupId ||
					groupIds.contains(child.strResourceId))
				{
					status = context +
						" refers to a group instead of an authored leaf: " +
						child.strResourceId;
					return false;
				}
				leafIds.insert(child.strResourceId);
			}
		}

		for (const std::string& effectId : leafIds)
		{
			std::string relative;
			if (!Build_EffectV2Relative(EFFECT_V2_AUTHORED_ROOT, effectId,
					EFFECT_V2_DOCUMENT_SUFFIX,
					"BOSS_VALTAN Effect V2 leaf effectId", relative, status))
			{
				return false;
			}
			DATA_JSON_VALUE leaf;
			if (!Read_EffectV2Json(root, relative,
					"BOSS_VALTAN Effect V2 leaf " + effectId, leaf, status))
			{
				return false;
			}
			std::string leafSchema;
			std::string loadedEffectId;
			std::uint64_t leafVersion = 0u;
			if (!Has_ExactProperties(leaf,
					{ "schema", "formatVersion", "effectId", "effectType",
						"slots", "params", "parts" }) ||
				!Read_String(leaf, "schema", leafSchema) ||
				"lostark.effect-v2" != leafSchema ||
				!Read_Unsigned(leaf, "formatVersion", leafVersion) ||
				1u != leafVersion ||
				!Read_String(leaf, "effectId", loadedEffectId) ||
				loadedEffectId != effectId)
			{
				status = "BOSS_VALTAN Effect V2 leaf identity is invalid: " +
					effectId;
				return false;
			}
			if (!expected.insert(relative).second)
			{
				status = "BOSS_VALTAN Effect V2 closure has a duplicate leaf path: " +
					relative;
				return false;
			}
		}
		return !leafIds.empty();
	}

	bool Build_EffectClosure(
		const std::filesystem::path& root,
		std::set<std::string>& expected,
		std::string& status)
	{
		std::string cueText;
		std::string catalogText;
		if (!Read_File(root / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json",
				cueText, status) ||
			!Read_File(root / "Data/Effects/EffectCatalog.json", catalogText, status))
		{
			return false;
		}
		DATA_JSON_VALUE cues;
		DATA_JSON_VALUE catalog;
		std::string parseError;
		if (!Client::CDataJson::Parse(cueText, cues, parseError) ||
			!cues.Is_Object() ||
			!Client::CDataJson::Parse(catalogText, catalog, parseError) ||
			!catalog.Is_Object())
		{
			status = "Valtan Effect closure JSON parse failed: " + parseError;
			return false;
		}
		const DATA_JSON_VALUE* cueRows = cues.Find("cues");
		const DATA_JSON_VALUE* catalogRows = catalog.Find("effects");
		if (nullptr == cueRows || !cueRows->Is_Array() ||
			nullptr == catalogRows || !catalogRows->Is_Array())
		{
			status = "Valtan Effect closure arrays are unavailable.";
			return false;
		}
		std::map<std::string, const DATA_JSON_VALUE*> byId;
		for (const DATA_JSON_VALUE& row : catalogRows->Get_Array())
		{
			std::string id;
			if (!row.Is_Object() || !Read_String(row, "effectAssetId", id) ||
				id.empty() || !byId.emplace(id, &row).second)
			{
				status = "Effect catalog has an invalid or duplicate stable ID.";
				return false;
			}
		}
		for (const DATA_JSON_VALUE& cue : cueRows->Get_Array())
		{
			std::string id;
			if (!cue.Is_Object() || !Read_String(cue, "effectAssetId", id))
			{
				status = "Pattern Effect cue has no stable Effect ID.";
				return false;
			}
			const auto found = byId.find(id);
			std::string payloadKind;
			std::string authoringPath;
			if (byId.end() == found ||
				!Read_String(*found->second, "payloadKind", payloadKind) ||
				"DIRECT_AUTHORED_DOCUMENT" != payloadKind ||
				!Read_String(*found->second, "authoringPath", authoringPath) ||
				authoringPath.rfind("Effects/Authored/", 0u) != 0u ||
				!authoringPath.ends_with(".effect.json"))
			{
				status = "Pattern Effect cue does not resolve to one authored document: " + id;
				return false;
			}
			const std::string relative = "Data/" + authoringPath;
			if (!Is_SafeRelativePath(relative))
			{
				status = "Pattern Effect authored document path is unsafe.";
				return false;
			}
			expected.insert(relative);
		}
		return expected.size() > FIXED_ARTIFACTS.size();
	}

	bool Load_Receipt(
		const std::filesystem::path& root,
		VALTAN_PRESENTATION_GENERATION_RECEIPT& receipt,
		std::string& status)
	{
		std::string bootstrapBytes;
		if (!Read_File(root / "Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap",
				bootstrapBytes, status))
		{
			return false;
		}
		GameplayDataRevision gameplayRevision{};
		GameplayDataRevision generationId{};
		if (!Parse_Bootstrap(
				bootstrapBytes, gameplayRevision, generationId, status))
		{
			return false;
		}
		std::map<std::string, std::string> expected = FIXED_ARTIFACTS;
		std::set<std::string> expectedPaths;
		for (const auto& [path, lane] : expected)
			expectedPaths.insert(path);
		if (!Build_EffectClosure(root, expectedPaths, status) ||
			!Build_EffectV2Closure(root, expectedPaths, status))
			return false;
		for (const std::string& path : expectedPaths)
		{
			if (!expected.contains(path))
				expected.emplace(path, "EFFECT");
		}

		/* Local authoring consumes the current typed physical closure directly.
		   The packaged generation manifest is a build receipt and may legitimately
		   lag behind source edits; using its bytes, hashes, or inventory as a Lobby
		   admission gate made normal authoring disconnect the user. */
		std::vector<VALTAN_PRESENTATION_GENERATION_ARTIFACT_RECEIPT> rows;
		rows.reserve(expected.size());
		for (const auto& [path, lane] : expected)
		{
			if (!Is_SafeRelativePath(path) || lane.empty())
			{
				status = "Valtan presentation source inventory is invalid.";
				return false;
			}
			const auto physical = root / std::filesystem::path(path);
			if (!Is_Descendant(root, physical))
			{
				status = "Valtan presentation generation artifact escaped the repository root.";
				return false;
			}
			std::string physicalBytes;
			GameplayDataRevision physicalRevision{};
			if (!Read_File(physical, physicalBytes, status))
				return false;
			if (physicalBytes.empty() || physicalBytes.size() > MAX_ARTIFACT_BYTES)
			{
				status = "Valtan presentation source is empty or too large: " + path;
				return false;
			}
			if (!Hash_Bytes(physicalBytes, physicalRevision))
			{
				status = "Valtan presentation artifact SHA-256 could not be calculated: " +
					path;
				return false;
			}
			rows.push_back({
				path, lane, physicalRevision,
				static_cast<std::uint64_t>(physicalBytes.size()) });
		}
		std::sort(rows.begin(), rows.end(), [](const auto& left, const auto& right)
			{
				return std::tie(left.strRelativePath, left.strLane) <
					std::tie(right.strRelativePath, right.strLane);
			});
		receipt.ServerGameplayRevision = gameplayRevision;
		receipt.PresentationGenerationId = generationId;
		receipt.Artifacts = std::move(rows);
		return true;
	}
}

struct Client::CValtanPresentationGenerationReadAdmission::STATE final
{
	std::filesystem::path RepositoryRoot;
	std::unique_ptr<CValtanCanonicalProductReadAdmission> CanonicalAdmission;
	VALTAN_PRESENTATION_GENERATION_RECEIPT Receipt;
};

bool Client::VALTAN_PRESENTATION_GENERATION_RECEIPT::Is_Valid() const
{
	if (!ServerGameplayRevision.Is_Valid() ||
		!PresentationGenerationId.Is_Valid() || Artifacts.empty())
	{
		return false;
	}
	std::set<std::string> paths;
	for (const auto& artifact : Artifacts)
	{
		if (!artifact.Revision.Is_Valid() || artifact.strLane.empty() ||
			0u == artifact.iBytes ||
			!paths.insert(artifact.strRelativePath).second)
		{
			return false;
		}
	}
	return true;
}

Client::CValtanPresentationGenerationReadAdmission::
	CValtanPresentationGenerationReadAdmission() = default;
Client::CValtanPresentationGenerationReadAdmission::
	~CValtanPresentationGenerationReadAdmission() = default;

bool Client::CValtanPresentationGenerationReadAdmission::
	Acquire_PackagedBaseline(
		VALTAN_PRESENTATION_GENERATION_RECEIPT& OutReceipt,
		std::string& strOutStatus)
{
	VALTAN_CANONICAL_READ_DIAGNOSTIC Diagnostic;
	const bool_t bAcquired = Acquire_PackagedBaseline(
		OutReceipt, Diagnostic);
	strOutStatus = std::move(Diagnostic.strStatus);
	return bAcquired;
}

bool Client::CValtanPresentationGenerationReadAdmission::
	Acquire_PackagedBaseline(
		VALTAN_PRESENTATION_GENERATION_RECEIPT& OutReceipt,
		VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic)
{
	return Acquire_PackagedBaselineFromRoot(
		CProjectDataRoot::Get().parent_path(), OutReceipt, OutDiagnostic);
}

bool Client::CValtanPresentationGenerationReadAdmission::
	Acquire_PackagedBaselineFromRoot(
		const std::filesystem::path& RepositoryRoot,
		VALTAN_PRESENTATION_GENERATION_RECEIPT& OutReceipt,
		std::string& strOutStatus)
{
	VALTAN_CANONICAL_READ_DIAGNOSTIC Diagnostic;
	const bool_t bAcquired = Acquire_PackagedBaselineFromRoot(
		RepositoryRoot, OutReceipt, Diagnostic);
	strOutStatus = std::move(Diagnostic.strStatus);
	return bAcquired;
}

bool Client::CValtanPresentationGenerationReadAdmission::
	Acquire_PackagedBaselineFromRoot(
		const std::filesystem::path& RepositoryRoot,
		VALTAN_PRESENTATION_GENERATION_RECEIPT& OutReceipt,
		VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic)
{
	OutDiagnostic.Clear();
	if (nullptr != m_pState)
	{
		OutDiagnostic.eFailure = VALTAN_CANONICAL_READ_FAILURE_KIND::
			ADMISSION_STATE_INVALID;
		OutDiagnostic.strStatus =
			"Valtan presentation generation admission is already held.";
		return false;
	}
	auto staged = std::make_unique<STATE>();
	staged->RepositoryRoot = RepositoryRoot;
	staged->CanonicalAdmission =
		std::make_unique<CValtanCanonicalProductReadAdmission>();
	if (nullptr == staged->CanonicalAdmission ||
		!staged->CanonicalAdmission->Acquire(OutDiagnostic))
	{
		if (nullptr == staged->CanonicalAdmission)
		{
			OutDiagnostic.eFailure = VALTAN_CANONICAL_READ_FAILURE_KIND::
				ADMISSION_IO;
			OutDiagnostic.strStatus =
				"Valtan canonical presentation admission allocation failed.";
		}
		return false;
	}
	if (!Load_Receipt(
			RepositoryRoot, staged->Receipt, OutDiagnostic.strStatus))
	{
		OutDiagnostic.eFailure =
			VALTAN_CANONICAL_READ_FAILURE_KIND::PRODUCT_INVALID;
		return false;
	}
	if (!staged->Receipt.Is_Valid())
	{
		OutDiagnostic.eFailure =
			VALTAN_CANONICAL_READ_FAILURE_KIND::PRODUCT_INVALID;
		OutDiagnostic.strStatus =
			"Valtan presentation source receipt is incomplete.";
		return false;
	}
	const auto receipt = staged->Receipt;
	m_pState = std::move(staged);
	OutReceipt = receipt;
	OutDiagnostic.Clear();
	OutDiagnostic.strStatus =
		"Admitted the current validated Valtan presentation sources.";
	return true;
}

bool Client::CValtanPresentationGenerationReadAdmission::Acquire_Receipt(
	const GameplayDataRevision& ExpectedServerRevision,
	const VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
	std::string& strOutStatus)
{
	return Acquire_ReceiptFromRoot(CProjectDataRoot::Get().parent_path(),
		ExpectedServerRevision, ExpectedReceipt, strOutStatus);
}

bool Client::CValtanPresentationGenerationReadAdmission::Acquire_ExactReceipt(
	const GameplayDataRevision& ExpectedServerRevision,
	const VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
	std::string& strOutStatus)
{
	return Acquire_ExactReceiptFromRoot(
		CProjectDataRoot::Get().parent_path(), ExpectedServerRevision,
		ExpectedReceipt, strOutStatus);
}

bool Client::CValtanPresentationGenerationReadAdmission::Acquire_ReceiptFromRoot(
	const std::filesystem::path& RepositoryRoot,
	const GameplayDataRevision& ExpectedServerRevision,
	const VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
	std::string& strOutStatus)
{
	if (!ExpectedServerRevision.Is_Valid())
	{
		strOutStatus = "Expected Server gameplay revision is invalid.";
		return false;
	}
	(void)ExpectedReceipt;
	VALTAN_PRESENTATION_GENERATION_RECEIPT physical;
	if (!Acquire_PackagedBaselineFromRoot(
			RepositoryRoot, physical, strOutStatus))
	{
		return false;
	}
	/* The caller pins only the authoritative Server gameplay revision. The
	   presentation receipt supplied at world entry is historical context, not an
	   identity gate for ordinary typed local authoring files. Keep the current
	   validated physical closure held by this admission so Validate_StillCurrent
	   can still reject a concurrent write before the caller commits its caches. */
	physical.ServerGameplayRevision = ExpectedServerRevision;
	m_pState->Receipt = physical;
	strOutStatus = "Admitted the current validated Valtan presentation files.";
	return true;
}

bool Client::CValtanPresentationGenerationReadAdmission::
	Acquire_ExactReceiptFromRoot(
		const std::filesystem::path& RepositoryRoot,
		const GameplayDataRevision& ExpectedServerRevision,
		const VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
		std::string& strOutStatus)
{
	if (!ExpectedServerRevision.Is_Valid() || !ExpectedReceipt.Is_Valid() ||
		ExpectedReceipt.ServerGameplayRevision != ExpectedServerRevision)
	{
		strOutStatus =
			"Prepared Valtan presentation receipt or Server revision is invalid.";
		return false;
	}
	VALTAN_PRESENTATION_GENERATION_RECEIPT physical;
	if (!Acquire_PackagedBaselineFromRoot(
			RepositoryRoot, physical, strOutStatus))
	{
		return false;
	}
	physical.ServerGameplayRevision = ExpectedServerRevision;
	if (physical != ExpectedReceipt)
	{
		m_pState.reset();
		strOutStatus =
			"Saved Valtan presentation files changed after this Server generation was prepared.";
		return false;
	}
	m_pState->Receipt = physical;
	strOutStatus =
		"Admitted the exact saved Valtan presentation generation prepared for the Server revision.";
	return true;
}

bool Client::CValtanPresentationGenerationReadAdmission::
	Validate_StillCurrent(std::string& strOutStatus) const
{
	VALTAN_CANONICAL_READ_DIAGNOSTIC Diagnostic;
	const bool_t bCurrent = Validate_StillCurrent(Diagnostic);
	strOutStatus = std::move(Diagnostic.strStatus);
	return bCurrent;
}

bool Client::CValtanPresentationGenerationReadAdmission::
	Validate_StillCurrent(
		VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic) const
{
	OutDiagnostic.Clear();
	if (nullptr == m_pState || nullptr == m_pState->CanonicalAdmission)
	{
		OutDiagnostic.eFailure = VALTAN_CANONICAL_READ_FAILURE_KIND::
			ADMISSION_STATE_INVALID;
		OutDiagnostic.strStatus =
			"Valtan presentation generation admission is not held.";
		return false;
	}
	if (!m_pState->CanonicalAdmission->Validate_StillCurrent(OutDiagnostic))
		return false;
	VALTAN_PRESENTATION_GENERATION_RECEIPT physical;
	if (!Load_Receipt(
			m_pState->RepositoryRoot, physical, OutDiagnostic.strStatus))
	{
		OutDiagnostic.eFailure =
			VALTAN_CANONICAL_READ_FAILURE_KIND::PRODUCT_INVALID;
		return false;
	}
	physical.ServerGameplayRevision =
		m_pState->Receipt.ServerGameplayRevision;
	if (physical != m_pState->Receipt)
	{
		OutDiagnostic.eFailure =
			VALTAN_CANONICAL_READ_FAILURE_KIND::GENERATION_CHANGED;
		OutDiagnostic.strStatus =
			"Valtan presentation generation changed during typed cache staging.";
		return false;
	}
	OutDiagnostic.Clear();
	OutDiagnostic.strStatus =
		"Valtan presentation sources remained unchanged through commit.";
	return true;
}

bool Client::CValtanPresentationGenerationReadAdmission::Try_Get_CurrentReceipt(
	VALTAN_PRESENTATION_GENERATION_RECEIPT& OutReceipt) const
{
	if (nullptr == m_pState || !m_pState->Receipt.Is_Valid())
		return false;
	OutReceipt = m_pState->Receipt;
	return true;
}

bool Client::CValtanPresentationGenerationReadAdmission::Is_Acquired() const
{
	return nullptr != m_pState;
}
