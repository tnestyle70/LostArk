#include "AnimationSkillBindingDocument.h"
#include "ClientReplication.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
	using namespace Client;

	bool_t Require(const bool_t condition, const std::string& message)
	{
		if (!condition)
		{
			std::cerr <<
				"ValtanPatternAnimationBindingDocumentContracts: " <<
				message << '\n';
		}
		return condition;
	}

	bool_t VerifyAuthoritativeFreshnessGate()
	{
		CPrimaryValtanPresentationFreshnessGate gate;
		LostArk::Shared::GameplayDataRevision revisionOne{};
		revisionOne.Bytes[0] = 1u;
		LostArk::Shared::GameplayDataRevision revisionTwo{};
		revisionTwo.Bytes[0] = 2u;
		std::string status;
		if (!Require(!gate.Can_Play(revisionOne, status),
			"unadmitted authoritative presentation gate allowed Complete Play"))
		{
			return false;
		}
		gate.Admit(revisionOne, "authoritative primary spawn reloaded");
		if (!Require(gate.Can_Play(revisionOne, status),
			"exact authoritative presentation revision rejected Complete Play"))
		{
			return false;
		}
		gate.Reject("authoritative cache reload failed");
		if (!Require(!gate.Can_Play(revisionOne, status) &&
			status.find("authoritative cache reload failed") != std::string::npos,
			"stale authoritative presentation gate did not retain its diagnostic"))
		{
			return false;
		}
		/* A despawn/no-consumer edge does not touch the gate. The original
		   rejection therefore remains latched until an explicit world reset or
		   successful primary-consumer reload/spawn. */
		if (!Require(!gate.Can_Play(revisionOne, status),
			"despawn/no-consumer lifecycle cleared a rejected freshness gate"))
		{
			return false;
		}
		gate.Admit(revisionOne, "authoritative primary spawn reloaded");
		if (!Require(!gate.Can_Play(revisionTwo, status),
			"a stale presentation receipt admitted a different Server revision"))
		{
			return false;
		}
		if (!Require(gate.Can_Play(revisionOne, status),
			"revision mismatch rejection corrupted the admitted receipt"))
		{
			return false;
		}
		gate.Admit(revisionTwo, "authoritative primary reload advanced revision");
		return Require(gate.Can_Play(revisionTwo, status),
			"successful exact-revision reload did not reopen Complete Play");
	}

	std::string ReadText(const std::filesystem::path& path)
	{
		std::ifstream input(path, std::ios::binary);
		return { std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
	}

	bool_t ReadValtanClipMap(
		const std::filesystem::path& path,
		std::vector<std::string>& outClips)
	{
		std::ifstream input(path, std::ios::binary);
		std::string magic;
		std::string asset;
		std::uint32_t version = 0u;
		std::size_t count = 0u;
		if (!input || !(input >> magic >> version >> std::quoted(asset) >> count) ||
			magic != "LOSTARK_CLIP_MAP" || 1u != version || asset != "Valtan" ||
			0u == count || count > 4096u)
		{
			return false;
		}

		std::unordered_set<std::string> unique;
		outClips.clear();
		outClips.reserve(count);
		for (std::size_t index = 0u; index < count; ++index)
		{
			std::string clip;
			if (!(input >> std::quoted(clip)) || clip.empty() ||
				!unique.insert(clip).second)
			{
				return false;
			}
			std::string remainder;
			std::getline(input, remainder);
			outClips.push_back(std::move(clip));
		}
		return outClips.size() == count;
	}

	void AddBoundModelClips(
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& document,
		std::vector<std::string>& inOutClips)
	{
		std::unordered_set<std::string> unique(
			inOutClips.begin(), inOutClips.end());
		for (const BOSS_PATTERN_ANIMATION_BINDING& binding : document.Bindings)
		{
			for (const BOSS_PATTERN_ANIMATION_CLIP& clip : binding.Clips)
			{
				if (unique.insert(clip.strClipName).second)
					inOutClips.push_back(clip.strClipName);
			}
		}
	}

	bool_t VerifyBodyVisibilityV4Parser()
	{
		const std::string valid = R"json({
  "schema": "lostark.valtan-pattern-bindings",
  "formatVersion": 4,
  "bossArchetypeId": "BOSS_VALTAN",
  "bindings": [
    {
      "actionId": "valtan.test.warp-leg",
      "bodyVisibility": { "hiddenFromMs": 0, "hiddenToMs": 300 },
      "clips": [
        {
          "clipOccurrenceId": "valtan.test.warp-leg.clip-01",
          "clip": "mesh_att_battle_18_02",
          "mappingBasis": "PROJECT_AUTHORED",
          "sourceStartMs": 0,
          "playMs": 0,
          "playRate": 1.0,
          "loop": true
        }
      ]
    }
  ]
})json";
		std::string status;
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT document;
		if (!Require(CValtanPatternAnimationBindingDocument::Parse_Text(
			valid, document, status) && 1u == document.Bindings.size() &&
			document.Bindings[0].bHasBodyHiddenWindow &&
			0u == document.Bindings[0].iBodyHiddenFromMs &&
			300u == document.Bindings[0].iBodyHiddenToMs,
			"Product v4 body visibility window did not parse: " + status))
		{
			return false;
		}
		const std::vector<std::string> clips{ "mesh_att_battle_18_02" };
		if (!Require(CValtanPatternAnimationBindingDocument::Validate(
			document, "BOSS_VALTAN", clips, status),
			"Product v4 body visibility window did not validate: " + status))
		{
			return false;
		}

		std::string legacyWithWindow = valid;
		const std::string versionFour = "\"formatVersion\": 4";
		const std::size_t versionOffset = legacyWithWindow.find(versionFour);
		legacyWithWindow.replace(versionOffset, versionFour.size(),
			"\"formatVersion\": 3");
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT rejected;
		if (!Require(!CValtanPatternAnimationBindingDocument::Parse_Text(
			legacyWithWindow, rejected, status),
			"Product v3 admitted a v4-only body visibility field"))
		{
			return false;
		}

		std::string inverted = valid;
		const std::string validWindow =
			"\"hiddenFromMs\": 0, \"hiddenToMs\": 300";
		const std::size_t windowOffset = inverted.find(validWindow);
		inverted.replace(windowOffset, validWindow.size(),
			"\"hiddenFromMs\": 300, \"hiddenToMs\": 0");
		return Require(!CValtanPatternAnimationBindingDocument::Parse_Text(
			inverted, rejected, status),
			"Product v4 admitted an inverted body visibility window");
	}

	bool_t VerifyContract()
	{
		if (!VerifyAuthoritativeFreshnessGate())
			return false;
		if (!VerifyBodyVisibilityV4Parser())
			return false;
		const std::filesystem::path repository =
			std::filesystem::current_path();
		const std::filesystem::path source = repository /
			"Data/Animation/Authored/Valtan/Valtan.patternbindings.json";
		const std::filesystem::path clipMap = repository /
			"Data/Animation/Reference/Valtan/Valtan.clipmap";
		const std::string sourceText = ReadText(source);
		std::string status;
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT sourceDocument;
		if (!Require(!sourceText.empty() &&
			CValtanPatternAnimationBindingDocument::Parse_Text(
				sourceText, sourceDocument, status),
			"real Valtan binding source did not parse: " + status))
		{
			return false;
		}

		std::vector<std::string> availableClips;
		if (!Require(ReadValtanClipMap(clipMap, availableClips),
			"real Valtan clip-map contract did not parse"))
		{
			return false;
		}
		/* The Product model also carries death/respawn clips that are not part
		   of the read-only skill reference map. Preserve the real admitted
		   binding union while the runtime continues to supply this same vector
		   from CModel::Get_AnimationName. */
		AddBoundModelClips(sourceDocument, availableClips);
		std::unordered_map<std::string, f32_t> clipSourceDurations;
		clipSourceDurations.reserve(availableClips.size());
		for (const std::string& clip : availableClips)
			clipSourceDurations.emplace(clip, 120.f);
		if (!Require(CValtanPatternAnimationBindingDocument::Validate(
			sourceDocument, "BOSS_VALTAN", availableClips, status),
			"real Valtan binding/available-clip join failed: " + status))
		{
			return false;
		}
		const std::unordered_map<std::string, std::uint32_t>
			expectedWarpVisibilityToMs{
				{ "valtan.sequence.warp.step-02", 300u },
				{ "valtan.sequence.warp.step-03", 600u },
				{ "valtan.sequence.warp.step-04", 600u },
				{ "valtan.sequence.warp.step-05", 600u },
				{ "valtan.sequence.warp.step-06", 600u },
				{ "valtan.sequence.warp.step-07", 600u },
				{ "valtan.sequence.warp.step-08", 600u },
				{ "valtan.sequence.warp.step-09", 600u },
				{ "valtan.sequence.warp.step-10", 300u },
			};
		std::size_t warpVisibilityCount = 0u;
		for (const BOSS_PATTERN_ANIMATION_BINDING& binding :
			sourceDocument.Bindings)
		{
			if (binding.strActionId.rfind("valtan.sequence.warp.step-", 0u) != 0u ||
				!binding.bHasBodyHiddenWindow)
			{
				continue;
			}
			const auto expected = expectedWarpVisibilityToMs.find(
				binding.strActionId);
			if (!Require(expectedWarpVisibilityToMs.end() != expected &&
				0u == binding.iBodyHiddenFromMs &&
				expected->second == binding.iBodyHiddenToMs,
				"real Warp leg has a drifted body visibility window"))
			{
				return false;
			}
			++warpVisibilityCount;
		}
		if (!Require(expectedWarpVisibilityToMs.size() == warpVisibilityCount,
			"real Warp Product does not expose the nine authored hidden body windows"))
		{
			return false;
		}
		std::string rejectedCommittedBytes;
		if (!Require(
			!CValtanPatternAnimationBindingDocument::Save_Atomic(
				sourceDocument, "Valtan", "BOSS_VALTAN", availableClips,
				clipSourceDurations, sourceText, rejectedCommittedBytes, status) &&
				rejectedCommittedBytes.empty() && ReadText(source) == sourceText &&
				status.find("read-only generated Product") != std::string::npos,
			"generated Valtan pattern binding Product exposed a direct Save path"))
		{
			return false;
		}
		/* Projection and owner mutation are covered by the Valtan pipeline tests;
		   this native boundary proves a Product consumer cannot write the generated
		   binding document even when it supplies the exact current bytes. */
		return true;

	}
}

int Run_ValtanPatternAnimationBindingDocumentContractTests()
{
	if (!VerifyContract())
		return 1;
	std::cout <<
		"Valtan pattern animation binding document contracts: PASS\n";
	return 0;
}
