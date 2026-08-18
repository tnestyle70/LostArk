#include "ValtanPatternPreviewDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <unordered_set>
#include <utility>

namespace
{
	using namespace Client;

	constexpr std::string_view DOCUMENT_SCHEMA =
		"lostark.valtan-pattern-preview";
	constexpr double DOCUMENT_VERSION = 2.0;
	constexpr std::string_view CLIP_SEQUENCE_MAGIC = "LOSTARK_CLIP_SEQ";
	constexpr uint32_t CLIP_SEQUENCE_VERSION = 2u;
	constexpr uint32_t PATTERN_COUNT = 67u;
	constexpr uint32_t MAX_SOURCE_SEQUENCES = 4096u;
	constexpr size_t MAX_SEQUENCES_PER_PATTERN = 16u;
	constexpr uint32_t MAX_SEQUENCE_REPEAT = 8u;
	constexpr size_t MAX_CLIPS_PER_SOURCE_SEQUENCE = 64u;
	constexpr size_t MAX_PLAYLIST_ITEMS = 2048u;

	struct SOURCE_SEQUENCE final
	{
		uint32_t iSourceActionId = 0u;
		int32_t iSequenceIndex = -1;
		std::string strName;
		std::string strMode;
		std::vector<std::string> Clips;
	};

	using SOURCE_SEQUENCE_KEY = std::pair<uint32_t, int32_t>;
	using SOURCE_SEQUENCE_INDEX =
		std::map<SOURCE_SEQUENCE_KEY, SOURCE_SEQUENCE>;

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* pValue = object.Find(pName);
		return nullptr != pValue && pValue->Get_Type() == eType ?
			pValue : nullptr;
	}

	bool_t Has_ExactProperties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> Names)
	{
		if (!object.Is_Object() || object.Get_Object().size() != Names.size())
			return false;
		for (const std::string_view Name : Names)
		{
			if (nullptr == object.Find(Name))
				return false;
		}
		return true;
	}

	bool_t Is_StableToken(const std::string_view Value)
	{
		if (Value.empty() || Value.size() > 255u)
			return false;
		for (const unsigned char Character : Value)
		{
			if (!(Character >= 'a' && Character <= 'z') &&
				!(Character >= 'A' && Character <= 'Z') &&
				!(Character >= '0' && Character <= '9') &&
				Character != '_' && Character != '-' && Character != '.')
			{
				return false;
			}
		}
		return true;
	}

	bool_t Is_DisplayText(
		const std::string_view Value,
		const size_t iMaximumBytes)
	{
		if (Value.empty() || Value.size() > iMaximumBytes)
			return false;
		return std::none_of(
			Value.begin(), Value.end(),
			[](const unsigned char Character)
			{
				return Character < 0x20u && Character != '\t';
			});
	}

	bool_t Is_SourceSequenceMode(const std::string_view Value)
	{
		return "SEQUENCE" == Value || "HOLD" == Value || "COMBO" == Value;
	}

	bool_t Parse_U32(
		const DATA_JSON_VALUE& Value,
		uint32_t& outValue)
	{
		if (!Value.Is_Number())
			return false;
		const double Number = Value.Get_Number();
		if (!std::isfinite(Number) || Number < 0.0 ||
			Number > static_cast<double>((std::numeric_limits<uint32_t>::max)()) ||
			std::floor(Number) != Number)
		{
			return false;
		}
		outValue = static_cast<uint32_t>(Number);
		return true;
	}

	bool_t Parse_I32(
		const DATA_JSON_VALUE& Value,
		int32_t& outValue)
	{
		if (!Value.Is_Number())
			return false;
		const double Number = Value.Get_Number();
		if (!std::isfinite(Number) || Number < 0.0 ||
			Number > static_cast<double>((std::numeric_limits<int32_t>::max)()) ||
			std::floor(Number) != Number)
		{
			return false;
		}
		outValue = static_cast<int32_t>(Number);
		return true;
	}

	bool_t Parse_U32Token(
		const std::string_view Value,
		uint32_t& outValue)
	{
		if (Value.empty())
			return false;
		uint64_t Result = 0u;
		for (const unsigned char Character : Value)
		{
			if (Character < '0' || Character > '9')
				return false;
			Result = Result * 10u + static_cast<uint64_t>(Character - '0');
			if (Result > (std::numeric_limits<uint32_t>::max)())
				return false;
		}
		outValue = static_cast<uint32_t>(Result);
		return true;
	}

	bool_t Parse_I32Token(
		const std::string_view Value,
		int32_t& outValue)
	{
		uint32_t Parsed = 0u;
		if (!Parse_U32Token(Value, Parsed) ||
			Parsed > static_cast<uint32_t>((std::numeric_limits<int32_t>::max)()))
		{
			return false;
		}
		outValue = static_cast<int32_t>(Parsed);
		return true;
	}

	VALTAN_PATTERN_PREVIEW_EVIDENCE Parse_Evidence(
		const std::string_view Value)
	{
		if ("SOURCE_FAMILY_DIRECT" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::SOURCE_FAMILY_DIRECT;
		if ("USER_CONFIRMED_FAMILY" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::USER_CONFIRMED_FAMILY;
		if ("CANDIDATE" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::CANDIDATE;
		if ("COMPOSITE" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::COMPOSITE;
		if ("UNRESOLVED" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::UNRESOLVED;
		if ("NO_ANIMATION" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::NO_ANIMATION;
		return VALTAN_PATTERN_PREVIEW_EVIDENCE::END;
	}

	void Skip_Space(
		const std::string_view Text,
		size_t& iCursor)
	{
		while (iCursor < Text.size() &&
			(Text[iCursor] == ' ' || Text[iCursor] == '\t' ||
				Text[iCursor] == '\r'))
		{
			++iCursor;
		}
	}

	bool_t Read_Token(
		const std::string_view Text,
		size_t& iCursor,
		std::string& outToken)
	{
		Skip_Space(Text, iCursor);
		const size_t iStart = iCursor;
		while (iCursor < Text.size() && Text[iCursor] != ' ' &&
			Text[iCursor] != '\t' && Text[iCursor] != '\r')
		{
			++iCursor;
		}
		if (iStart == iCursor)
			return false;
		outToken.assign(Text.substr(iStart, iCursor - iStart));
		return true;
	}

	bool_t Read_Quoted(
		const std::string_view Text,
		size_t& iCursor,
		std::string& outValue)
	{
		Skip_Space(Text, iCursor);
		if (iCursor >= Text.size() || '"' != Text[iCursor])
			return false;
		++iCursor;
		const size_t iStart = iCursor;
		while (iCursor < Text.size() && '"' != Text[iCursor])
		{
			if ('\\' == Text[iCursor] || '\r' == Text[iCursor])
				return false;
			++iCursor;
		}
		if (iCursor >= Text.size())
			return false;
		outValue.assign(Text.substr(iStart, iCursor - iStart));
		++iCursor;
		return true;
	}

	bool_t Read_Pair(
		const std::string_view Text,
		size_t& iCursor,
		std::string& outKey,
		std::string& outValue)
	{
		Skip_Space(Text, iCursor);
		const size_t iKeyStart = iCursor;
		while (iCursor < Text.size() && Text[iCursor] != '=' &&
			Text[iCursor] != ' ' && Text[iCursor] != '\t' &&
			Text[iCursor] != '\r')
		{
			++iCursor;
		}
		if (iKeyStart == iCursor || iCursor >= Text.size() ||
			'=' != Text[iCursor])
		{
			return false;
		}
		outKey.assign(Text.substr(iKeyStart, iCursor - iKeyStart));
		++iCursor;
		if (iCursor < Text.size() && '"' == Text[iCursor])
			return Read_Quoted(Text, iCursor, outValue);

		const size_t iValueStart = iCursor;
		while (iCursor < Text.size() && Text[iCursor] != ' ' &&
			Text[iCursor] != '\t' && Text[iCursor] != '\r')
		{
			++iCursor;
		}
		if (iValueStart == iCursor)
			return false;
		outValue.assign(Text.substr(iValueStart, iCursor - iValueStart));
		return true;
	}

	bool_t At_End(
		const std::string_view Text,
		size_t iCursor)
	{
		Skip_Space(Text, iCursor);
		return iCursor == Text.size();
	}

	bool_t Parse_ClipList(
		const std::string_view Value,
		std::vector<std::string>& outClips)
	{
		std::vector<std::string> Staged;
		size_t iStart = 0u;
		while (iStart <= Value.size())
		{
			const size_t iComma = Value.find(',', iStart);
			const std::string_view Clip = Value.substr(
				iStart,
				std::string_view::npos == iComma ?
					std::string_view::npos : iComma - iStart);
			if (!Is_StableToken(Clip))
				return false;
			Staged.emplace_back(Clip);
			if (Staged.size() > MAX_CLIPS_PER_SOURCE_SEQUENCE)
				return false;
			if (std::string_view::npos == iComma)
				break;
			iStart = iComma + 1u;
		}
		if (Staged.empty())
			return false;
		outClips = std::move(Staged);
		return true;
	}

	bool_t Load_SourceSequenceIndex(
		const std::string_view animationAssetId,
		SOURCE_SEQUENCE_INDEX& outIndex,
		std::string& outStatus)
	{
		const std::filesystem::path Path =
			CValtanPatternPreviewDocument::Resolve_ClipSequencePath(
				animationAssetId);
		std::ifstream Input(Path, std::ios::binary);
		if (Path.empty() || !Input)
		{
			outStatus = "Valtan source clip sequence is missing: " + Path.string();
			return false;
		}

		std::string Header;
		if (!std::getline(Input, Header))
		{
			outStatus = "Valtan source clip sequence header is missing.";
			return false;
		}
		if (!Header.empty() && '\r' == Header.back())
			Header.pop_back();

		size_t iCursor = 0u;
		std::string Magic;
		std::string VersionToken;
		std::string Owner;
		std::string CountToken;
		uint32_t iVersion = 0u;
		uint32_t iDeclaredCount = 0u;
		if (!Read_Token(Header, iCursor, Magic) ||
			!Read_Token(Header, iCursor, VersionToken) ||
			!Read_Quoted(Header, iCursor, Owner) ||
			!Read_Token(Header, iCursor, CountToken) ||
			!At_End(Header, iCursor) || Magic != CLIP_SEQUENCE_MAGIC ||
			!Parse_U32Token(VersionToken, iVersion) ||
			CLIP_SEQUENCE_VERSION != iVersion || Owner != animationAssetId ||
			!Parse_U32Token(CountToken, iDeclaredCount) ||
			0u == iDeclaredCount || iDeclaredCount > MAX_SOURCE_SEQUENCES)
		{
			outStatus = "Valtan source clip sequence header is invalid.";
			return false;
		}

		SOURCE_SEQUENCE_INDEX Staged;
		std::string Line;
		uint32_t iPhysicalRows = 0u;
		while (std::getline(Input, Line))
		{
			if (!Line.empty() && '\r' == Line.back())
				Line.pop_back();
			if (Line.empty())
			{
				outStatus = "Valtan source clip sequence contains a blank row.";
				return false;
			}

			iCursor = 0u;
			std::string ActionToken;
			SOURCE_SEQUENCE Sequence;
			if (!Read_Token(Line, iCursor, ActionToken) ||
				!Parse_U32Token(ActionToken, Sequence.iSourceActionId) ||
				0u == Sequence.iSourceActionId ||
				!Read_Quoted(Line, iCursor, Sequence.strName) ||
				!Is_DisplayText(Sequence.strName, 512u))
			{
				outStatus = "Valtan source clip sequence row identity is invalid.";
				return false;
			}

			bool_t bSawSequence = false;
			bool_t bSawMode = false;
			bool_t bSawClips = false;
			while (!At_End(Line, iCursor))
			{
				std::string Key;
				std::string Value;
				if (!Read_Pair(Line, iCursor, Key, Value))
				{
					outStatus = "Valtan source clip sequence row field is malformed.";
					return false;
				}
				if ("seq" == Key && !bSawSequence)
				{
					bSawSequence = Parse_I32Token(
						Value, Sequence.iSequenceIndex);
					if (!bSawSequence)
					{
						outStatus = "Valtan source sequence index is invalid.";
						return false;
					}
				}
				else if ("mode" == Key && !bSawMode)
				{
					bSawMode = Is_SourceSequenceMode(Value);
					if (!bSawMode)
					{
						outStatus = "Valtan source sequence mode is invalid.";
						return false;
					}
					Sequence.strMode = std::move(Value);
				}
				else if ("clips" == Key && !bSawClips)
				{
					bSawClips = Parse_ClipList(Value, Sequence.Clips);
					if (!bSawClips)
					{
						outStatus = "Valtan source sequence clip list is invalid.";
						return false;
					}
				}
				else
				{
					outStatus =
						"Valtan source clip sequence row has a duplicate or unknown field.";
					return false;
				}
			}

			if (!bSawSequence || !bSawMode || !bSawClips ||
				!Staged.emplace(
					SOURCE_SEQUENCE_KEY{
						Sequence.iSourceActionId, Sequence.iSequenceIndex },
					std::move(Sequence)).second)
			{
				outStatus =
					"Valtan source clip sequence has a duplicate action/sequence key.";
				return false;
			}
			++iPhysicalRows;
		}

		if (iPhysicalRows != iDeclaredCount || Staged.size() != iDeclaredCount)
		{
			outStatus = "Valtan source clip sequence declared count does not match.";
			return false;
		}

		outIndex = std::move(Staged);
		outStatus = "Loaded exact Valtan source action/sequence index.";
		return true;
	}

	bool_t Validate_RowPolicy(
		const VALTAN_PATTERN_PREVIEW_DOCUMENT& document,
		std::string& outStatus)
	{
		if (!Is_StableToken(document.strAnimationAssetId) ||
			document.Patterns.size() != PATTERN_COUNT)
		{
			outStatus = "Valtan pattern preview owner or pattern count is invalid.";
			return false;
		}

		for (size_t iPattern = 0u;
			iPattern < document.Patterns.size(); ++iPattern)
		{
			const VALTAN_PATTERN_PREVIEW_ENTRY& Pattern =
				document.Patterns[iPattern];
			const bool_t bMarkerEvidence =
				VALTAN_PATTERN_PREVIEW_EVIDENCE::UNRESOLVED ==
					Pattern.eEvidence ||
				VALTAN_PATTERN_PREVIEW_EVIDENCE::NO_ANIMATION ==
					Pattern.eEvidence;
			if (Pattern.iNumber != static_cast<uint32_t>(iPattern + 1u) ||
				!Is_DisplayText(Pattern.strLabel, 256u) ||
				!Is_DisplayText(Pattern.strNote, 512u) ||
				Pattern.eEvidence >= VALTAN_PATTERN_PREVIEW_EVIDENCE::END ||
				Pattern.Sequences.size() > MAX_SEQUENCES_PER_PATTERN ||
				(bMarkerEvidence != Pattern.Sequences.empty()))
			{
				outStatus =
					"Valtan pattern preview row order, evidence, or sequence policy is invalid.";
				return false;
			}
			for (const VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF& Reference :
				Pattern.Sequences)
			{
				if (0u == Reference.iSourceActionId ||
					Reference.iSequenceIndex < 0 || 0u == Reference.iRepeat ||
					Reference.iRepeat > MAX_SEQUENCE_REPEAT)
				{
					outStatus =
						"Valtan pattern preview source sequence reference is invalid.";
					return false;
				}
			}
		}
		return true;
	}
}

std::filesystem::path Client::CValtanPatternPreviewDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		L"Animation/Authored/Valtan/Valtan.patternpreview.json");
}

std::filesystem::path
Client::CValtanPatternPreviewDocument::Resolve_ClipSequencePath(
	const std::string_view animationAssetId)
{
	if (!Is_StableToken(animationAssetId))
		return {};
	const std::string Asset{ animationAssetId };
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation/Reference") /
		std::filesystem::path(Asset) /
		std::filesystem::path(Asset + ".clipseq"));
}

bool_t Client::CValtanPatternPreviewDocument::Parse_Text(
	const std::string_view text,
	VALTAN_PATTERN_PREVIEW_DOCUMENT& outDocument,
	std::string& outStatus)
{
	DATA_JSON_VALUE Root;
	std::string ParseError;
	if (!CDataJson::Parse(text, Root, ParseError))
	{
		outStatus = "Valtan pattern preview JSON is malformed: " + ParseError;
		return false;
	}
	if (!Has_ExactProperties(
			Root,
			{ "schema", "formatVersion", "animationAssetId", "patterns" }))
	{
		outStatus =
			"Valtan pattern preview root has an unexpected field set.";
		return false;
	}

	const DATA_JSON_VALUE* pSchema = Required(
		Root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* pVersion = Required(
		Root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* pAsset = Required(
		Root, "animationAssetId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* pPatterns = Required(
		Root, "patterns", DATA_JSON_TYPE::ARRAY);
	if (nullptr == pSchema || pSchema->Get_String() != DOCUMENT_SCHEMA ||
		nullptr == pVersion || pVersion->Get_Number() != DOCUMENT_VERSION ||
		nullptr == pAsset || !Is_StableToken(pAsset->Get_String()) ||
		nullptr == pPatterns ||
		pPatterns->Get_Array().size() != PATTERN_COUNT)
	{
		outStatus = "Valtan pattern preview header is invalid.";
		return false;
	}

	VALTAN_PATTERN_PREVIEW_DOCUMENT Staged;
	Staged.strAnimationAssetId = pAsset->Get_String();
	Staged.Patterns.reserve(pPatterns->Get_Array().size());
	for (const DATA_JSON_VALUE& PatternValue : pPatterns->Get_Array())
	{
		if (!Has_ExactProperties(
				PatternValue,
				{ "number", "label", "confidence", "note", "sequences" }))
		{
			outStatus =
				"Valtan pattern preview row has an unexpected field set.";
			return false;
		}

		const DATA_JSON_VALUE* pNumber = Required(
			PatternValue, "number", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* pLabel = Required(
			PatternValue, "label", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pEvidence = Required(
			PatternValue, "confidence", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pNote = Required(
			PatternValue, "note", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSequences = Required(
			PatternValue, "sequences", DATA_JSON_TYPE::ARRAY);

		VALTAN_PATTERN_PREVIEW_ENTRY Pattern;
		if (nullptr == pNumber || !Parse_U32(*pNumber, Pattern.iNumber) ||
			nullptr == pLabel ||
			!Is_DisplayText(pLabel->Get_String(), 256u) ||
			nullptr == pEvidence ||
			nullptr == pNote ||
			!Is_DisplayText(pNote->Get_String(), 512u) ||
			nullptr == pSequences ||
			pSequences->Get_Array().size() > MAX_SEQUENCES_PER_PATTERN)
		{
			outStatus = "Valtan pattern preview row is invalid.";
			return false;
		}
		Pattern.strLabel = pLabel->Get_String();
		Pattern.eEvidence = Parse_Evidence(pEvidence->Get_String());
		Pattern.strNote = pNote->Get_String();
		Pattern.Sequences.reserve(pSequences->Get_Array().size());

		for (const DATA_JSON_VALUE& SequenceValue :
			pSequences->Get_Array())
		{
			if (!Has_ExactProperties(
					SequenceValue,
					{ "sourceActionId", "sequenceIndex", "repeat" }))
			{
				outStatus =
					"Valtan pattern preview source reference has an unexpected field set.";
				return false;
			}

			const DATA_JSON_VALUE* pSourceActionId = Required(
				SequenceValue, "sourceActionId", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* pSequenceIndex = Required(
				SequenceValue, "sequenceIndex", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* pRepeat = Required(
				SequenceValue, "repeat", DATA_JSON_TYPE::NUMBER);
			VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF Reference;
			if (nullptr == pSourceActionId ||
				!Parse_U32(*pSourceActionId, Reference.iSourceActionId) ||
				nullptr == pSequenceIndex ||
				!Parse_I32(*pSequenceIndex, Reference.iSequenceIndex) ||
				nullptr == pRepeat ||
				!Parse_U32(*pRepeat, Reference.iRepeat))
			{
				outStatus =
					"Valtan pattern preview source reference is invalid.";
				return false;
			}
			Pattern.Sequences.push_back(std::move(Reference));
		}
		Staged.Patterns.push_back(std::move(Pattern));
	}

	if (!Validate_RowPolicy(Staged, outStatus))
		return false;
	outDocument = std::move(Staged);
	outStatus = "Parsed 67 Valtan source-sequence preview rows.";
	return true;
}

bool_t Client::CValtanPatternPreviewDocument::Resolve_SourceSequences(
	const VALTAN_PATTERN_PREVIEW_DOCUMENT& document,
	const std::string_view expectedAnimationAssetId,
	const std::vector<std::string>& availableClips,
	VALTAN_PATTERN_PREVIEW_DOCUMENT& outResolvedDocument,
	std::string& outStatus)
{
	if (!Is_StableToken(expectedAnimationAssetId) ||
		document.strAnimationAssetId != expectedAnimationAssetId)
	{
		outStatus =
			"Valtan pattern preview owner does not match the target model.";
		return false;
	}
	if (availableClips.empty())
	{
		outStatus = "Valtan target model exposes no animation clips.";
		return false;
	}
	if (!Validate_RowPolicy(document, outStatus))
		return false;

	std::unordered_set<std::string> AvailableClipSet;
	AvailableClipSet.reserve(availableClips.size());
	for (const std::string& Clip : availableClips)
	{
		if (!Is_StableToken(Clip) || !AvailableClipSet.insert(Clip).second)
		{
			outStatus =
				"Valtan target model clip catalog is invalid or contains duplicates.";
			return false;
		}
	}

	SOURCE_SEQUENCE_INDEX SourceIndex;
	if (!Load_SourceSequenceIndex(
			expectedAnimationAssetId, SourceIndex, outStatus))
	{
		return false;
	}

	VALTAN_PATTERN_PREVIEW_DOCUMENT Staged = document;
	size_t iResolvedSequenceCount = 0u;
	size_t iExpandedItemCount = 0u;
	for (VALTAN_PATTERN_PREVIEW_ENTRY& Pattern : Staged.Patterns)
	{
		if (Pattern.Sequences.empty())
		{
			++iExpandedItemCount;
			continue;
		}
		for (VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF& Reference :
			Pattern.Sequences)
		{
			Reference.strSequenceName.clear();
			Reference.strSequenceMode.clear();
			Reference.ResolvedClips.clear();

			const auto Found = SourceIndex.find(
				SOURCE_SEQUENCE_KEY{
					Reference.iSourceActionId, Reference.iSequenceIndex });
			if (SourceIndex.end() == Found)
			{
				outStatus =
					"Valtan pattern " + std::to_string(Pattern.iNumber) +
					" references missing source action " +
					std::to_string(Reference.iSourceActionId) + " sequence " +
					std::to_string(Reference.iSequenceIndex) + ".";
				return false;
			}

			const SOURCE_SEQUENCE& Source = Found->second;
			for (const std::string& Clip : Source.Clips)
			{
				if (!AvailableClipSet.contains(Clip))
				{
					outStatus =
						"Valtan pattern " + std::to_string(Pattern.iNumber) +
						" source action " +
						std::to_string(Reference.iSourceActionId) +
						" sequence " +
						std::to_string(Reference.iSequenceIndex) +
						" references clip missing from the target model: " +
						Clip + ".";
					return false;
				}
			}

			Reference.strSequenceName = Source.strName;
			Reference.strSequenceMode = Source.strMode;
			Reference.ResolvedClips = Source.Clips;
			++iResolvedSequenceCount;
			iExpandedItemCount +=
				Source.Clips.size() * static_cast<size_t>(Reference.iRepeat);
			if (iExpandedItemCount > MAX_PLAYLIST_ITEMS)
			{
				outStatus =
					"Valtan pattern preview expands beyond the bounded playlist budget.";
				return false;
			}
		}
	}

	outResolvedDocument = std::move(Staged);
	outStatus = "Resolved " + std::to_string(iResolvedSequenceCount) +
		" Valtan source action/sequence reference(s) against the target model.";
	return true;
}

bool_t Client::CValtanPatternPreviewDocument::Validate(
	const VALTAN_PATTERN_PREVIEW_DOCUMENT& document,
	const std::string_view expectedAnimationAssetId,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	VALTAN_PATTERN_PREVIEW_DOCUMENT Resolved;
	return Resolve_SourceSequences(
		document,
		expectedAnimationAssetId,
		availableClips,
		Resolved,
		outStatus);
}

bool_t Client::CValtanPatternPreviewDocument::Load(
	const std::string_view expectedAnimationAssetId,
	const std::vector<std::string>& availableClips,
	VALTAN_PATTERN_PREVIEW_DOCUMENT& outDocument,
	std::string& outStatus)
{
	const std::filesystem::path Path = Resolve_Path();
	std::ifstream Input(Path, std::ios::binary);
	if (Path.empty() || !Input)
	{
		outStatus = "Valtan pattern preview document is missing: " + Path.string();
		return false;
	}
	const std::string Text{
		std::istreambuf_iterator<char>(Input),
		std::istreambuf_iterator<char>() };
	VALTAN_PATTERN_PREVIEW_DOCUMENT Authored;
	VALTAN_PATTERN_PREVIEW_DOCUMENT Resolved;
	if (!Parse_Text(Text, Authored, outStatus) ||
		!Resolve_SourceSequences(
			Authored,
			expectedAnimationAssetId,
			availableClips,
			Resolved,
			outStatus))
	{
		return false;
	}
	outDocument = std::move(Resolved);
	return true;
}

bool_t Client::CValtanPatternPreviewDocument::Build_Playlist(
	const VALTAN_PATTERN_PREVIEW_DOCUMENT& resolvedDocument,
	const uint32_t iFirstPattern,
	const uint32_t iLastPattern,
	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM>& outPlaylist,
	std::string& outStatus)
{
	if (!Validate_RowPolicy(resolvedDocument, outStatus))
		return false;
	if (0u == iFirstPattern || iFirstPattern > iLastPattern ||
		iLastPattern > resolvedDocument.Patterns.size())
	{
		outStatus = "Valtan pattern preview playlist range is invalid.";
		return false;
	}

	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> Staged;
	for (uint32_t iPattern = iFirstPattern;
		iPattern <= iLastPattern; ++iPattern)
	{
		const VALTAN_PATTERN_PREVIEW_ENTRY& Pattern =
			resolvedDocument.Patterns[static_cast<size_t>(iPattern - 1u)];
		const size_t iPatternStart = Staged.size();
		if (Pattern.Sequences.empty())
		{
			VALTAN_PATTERN_PREVIEW_PLAY_ITEM Marker;
			Marker.iPatternNumber = Pattern.iNumber;
			Marker.strPatternLabel = Pattern.strLabel;
			Marker.eEvidence = Pattern.eEvidence;
			Marker.strNote = Pattern.strNote;
			Marker.bPatternMarker = true;
			Staged.push_back(std::move(Marker));
		}
		else
		{
			for (const VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF& Reference :
				Pattern.Sequences)
			{
				if (!Reference.Is_Resolved())
				{
					outStatus =
						"Valtan playlist requires a fully resolved source sequence document.";
					return false;
				}
				for (uint32_t iRepeat = 1u;
					iRepeat <= Reference.iRepeat; ++iRepeat)
				{
					for (size_t iSourceStep = 0u;
						iSourceStep < Reference.ResolvedClips.size(); ++iSourceStep)
					{
						VALTAN_PATTERN_PREVIEW_PLAY_ITEM Item;
						Item.iPatternNumber = Pattern.iNumber;
						Item.strPatternLabel = Pattern.strLabel;
						Item.eEvidence = Pattern.eEvidence;
						Item.strNote = Pattern.strNote;
						Item.iSourceActionId = Reference.iSourceActionId;
						Item.iSequenceIndex = Reference.iSequenceIndex;
						Item.iSequenceRepeatNumber = iRepeat;
						Item.iSequenceRepeatCount = Reference.iRepeat;
						Item.iSourceStepNumber =
							static_cast<uint32_t>(iSourceStep + 1u);
						Item.iSourceStepCount = static_cast<uint32_t>(
							Reference.ResolvedClips.size());
						Item.strSequenceName = Reference.strSequenceName;
						Item.strSequenceMode = Reference.strSequenceMode;
						Item.strClipName = Reference.ResolvedClips[iSourceStep];
						Staged.push_back(std::move(Item));
						if (Staged.size() > MAX_PLAYLIST_ITEMS)
						{
							outStatus =
								"Valtan pattern preview playlist exceeds its bounded item budget.";
							return false;
						}
					}
				}
			}
		}

		const size_t iPatternStepCount = Staged.size() - iPatternStart;
		if (0u == iPatternStepCount)
		{
			outStatus = "Valtan pattern preview produced an empty pattern.";
			return false;
		}
		for (size_t iStep = 0u; iStep < iPatternStepCount; ++iStep)
		{
			VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
				Staged[iPatternStart + iStep];
			Item.iStepNumber = static_cast<uint32_t>(iStep + 1u);
			Item.iStepCount = static_cast<uint32_t>(iPatternStepCount);
		}
	}

	outPlaylist = std::move(Staged);
	outStatus = "Built bounded Valtan source-sequence playlist with " +
		std::to_string(outPlaylist.size()) + " item(s).";
	return true;
}

const char_t* Client::CValtanPatternPreviewDocument::Evidence_Name(
	const VALTAN_PATTERN_PREVIEW_EVIDENCE eEvidence)
{
	switch (eEvidence)
	{
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::SOURCE_FAMILY_DIRECT:
		return "SOURCE_FAMILY_DIRECT";
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::USER_CONFIRMED_FAMILY:
		return "USER_CONFIRMED_FAMILY";
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::CANDIDATE:
		return "CANDIDATE";
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::COMPOSITE:
		return "COMPOSITE";
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::UNRESOLVED:
		return "UNRESOLVED";
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::NO_ANIMATION:
		return "NO_ANIMATION";
	default:
		return "INVALID";
	}
}
