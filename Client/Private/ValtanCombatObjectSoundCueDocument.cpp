#include "ValtanCombatObjectSoundCueDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "SoundCueCatalog.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <tuple>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr std::string_view SCHEMA =
		"lostark.valtan-combat-object-sound-cues";
	constexpr std::string_view PRODUCT_SCHEMA =
		"lostark.valtan-combat-objects";
	constexpr std::string_view OWNER_ARCHETYPE_ID = "BOSS_VALTAN";
	constexpr std::string_view ENCOUNTER_ID = "ENCOUNTER_VALTAN";
	constexpr std::string_view SOUND_BANK = "S_Mob_G_Voltan2";
	constexpr uint32_t FORMAT_VERSION = 1u;
	constexpr std::size_t MAX_CUE_COUNT = 256u;

	bool_t Is_ExactObject(const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key)
			{
				return nullptr != value.Find(key);
			});
	}

	bool_t Is_StableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 160u &&
			std::all_of(value.begin(), value.end(),
				[](const char_t character)
				{
					const unsigned char raw =
						static_cast<unsigned char>(character);
					return 0 != std::isalnum(raw) || character == '_' ||
						character == '-' || character == '.';
				});
	}

	bool_t Read_StableString(const DATA_JSON_VALUE& parent,
		const char_t* key, std::string& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_String() ||
			!Is_StableId(value->Get_String()))
		{
			return false;
		}
		outValue = value->Get_String();
		return true;
	}

	bool_t Read_ExactUnsigned(const DATA_JSON_VALUE& parent,
		const char_t* key, const uint32_t expected)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		return nullptr != value && value->Is_Number() &&
			value->Get_Number() == static_cast<double>(expected);
	}

	bool_t Read_File(const std::filesystem::path& path,
		std::string& outText)
	{
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
			return false;
		outText.assign(std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>());
		return true;
	}

	std::string Make_SourceKey(const std::string_view archetypeId,
		const std::string_view hitId)
	{
		return std::string(archetypeId) + "\n" + std::string(hitId);
	}

	bool_t Is_ValidSoundAssetId(const std::string_view assetId)
	{
		if (!assetId.starts_with("Sound/") ||
			!assetId.ends_with(".wav") ||
			std::string_view::npos != assetId.find('\\') ||
			std::string_view::npos != assetId.find(':'))
		{
			return false;
		}
		std::size_t begin = 0u;
		while (begin <= assetId.size())
		{
			const std::size_t end = assetId.find('/', begin);
			const std::string_view component = assetId.substr(begin,
				std::string_view::npos == end ? assetId.size() - begin : end - begin);
			if (component.empty() || "." == component || ".." == component)
				return false;
			if (std::string_view::npos == end)
				break;
			begin = end + 1u;
		}
		return true;
	}

	std::string Serialize_Document(
		const VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& document)
	{
		std::ostringstream output;
		output <<
			"{\n"
			"  \"schema\": \"lostark.valtan-combat-object-sound-cues\",\n"
			"  \"formatVersion\": 1,\n"
			"  \"ownerArchetypeId\": \"" << document.strOwnerArchetypeId <<
			"\",\n  \"cues\": [\n";
		for (std::size_t i = 0u; i < document.Cues.size(); ++i)
		{
			const VALTAN_COMBAT_OBJECT_SOUND_CUE& cue = document.Cues[i];
			output <<
				"    {\n"
				"      \"bindingId\": \"" << cue.strBindingId << "\",\n"
				"      \"combatObjectArchetypeId\": \"" <<
				cue.strCombatObjectArchetypeId << "\",\n"
				"      \"hitId\": \"" << cue.strHitId << "\",\n"
				"      \"soundBank\": \"" << cue.strSoundBank << "\",\n"
				"      \"soundEvent\": \"" << cue.strSoundEvent << "\"\n"
				"    }";
			if (i + 1u != document.Cues.size())
				output << ',';
			output << '\n';
		}
		output << "  ]\n}\n";
		return output.str();
	}

	std::wstring Make_TransactionSuffix()
	{
		static std::atomic_uint64_t sequence{ 0u };
		return L"." + std::to_wstring(GetCurrentProcessId()) + L"." +
			std::to_wstring(GetTickCount64()) + L"." +
			std::to_wstring(sequence.fetch_add(1u, std::memory_order_relaxed));
	}

	bool_t Write_StagedSource(const std::filesystem::path& path,
		const std::string& text, std::string& outStatus)
	{
		const HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0u,
			nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (INVALID_HANDLE_VALUE == file)
		{
			outStatus = "Valtan Sound cue staging file could not be created (Win32 " +
				std::to_string(GetLastError()) + ").";
			return false;
		}

		bool_t wrote = true;
		std::size_t offset = 0u;
		while (offset < text.size())
		{
			const DWORD requested = static_cast<DWORD>(std::min<std::size_t>(
				text.size() - offset, static_cast<std::size_t>(MAXDWORD)));
			DWORD written = 0u;
			if (!WriteFile(file, text.data() + offset, requested, &written,
					nullptr) || 0u == written)
			{
				wrote = false;
				break;
			}
			offset += written;
		}
		const bool_t durable = wrote && FlushFileBuffers(file);
		const DWORD writeError = durable ? ERROR_SUCCESS : GetLastError();
		const bool_t closed = CloseHandle(file);
		const DWORD closeError = closed ? ERROR_SUCCESS : GetLastError();
		if (!durable || !closed)
		{
			DeleteFileW(path.c_str());
			outStatus = "Valtan Sound cue staging write failed (Win32 " +
				std::to_string(durable ? closeError : writeError) +
				"); previous source preserved.";
			return false;
		}
		return true;
	}

	bool_t Validate_CatalogAssets(
		const VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& document,
		std::string& outStatus)
	{
		for (const VALTAN_COMBAT_OBJECT_SOUND_CUE& cue : document.Cues)
		{
			const std::vector<std::string>& variants =
				CSoundCueCatalog::Find_Variants("Valtan", cue.strSoundEvent);
			if (variants.empty() || !std::all_of(variants.begin(), variants.end(),
					[](const std::string& assetId)
					{
						return Is_ValidSoundAssetId(assetId);
					}))
			{
				outStatus =
					"Valtan combat-object Sound cue event has no valid asset: " +
					cue.strBindingId;
				return false;
			}
		}
		return true;
	}
}

std::filesystem::path
Client::CValtanCombatObjectSoundCueDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
		L"Valtan.combatobjectsoundcues.json");
}

std::filesystem::path
Client::CValtanCombatObjectSoundCueDocument::Resolve_CombatObjectProductPath()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Encounters") / L"Valtan" /
		L"ValtanCombatObjects.json");
}

bool_t Client::CValtanCombatObjectSoundCueDocument::Parse_Text(
	const std::string_view cueText,
	const std::string_view combatObjectProductText,
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& inOutDocument,
	std::string& outStatus)
{
	DATA_JSON_PARSE_LIMITS limits{};
	limits.iMaximumBytes = 1024u * 1024u;
	limits.iMaximumDepth = 12u;
	limits.iMaximumValues = 32768u;
	DATA_JSON_VALUE productRoot;
	std::string parseError;
	if (!CDataJson::Parse(combatObjectProductText, productRoot,
			parseError, limits) ||
		!Is_ExactObject(productRoot,
			{ "schema", "formatVersion", "encounterId", "objects" }))
	{
		outStatus = "Valtan combat-object Product JSON is malformed: " +
			parseError;
		return false;
	}
	const DATA_JSON_VALUE* productSchema = productRoot.Find("schema");
	const DATA_JSON_VALUE* encounter = productRoot.Find("encounterId");
	const DATA_JSON_VALUE* objects = productRoot.Find("objects");
	if (nullptr == productSchema || !productSchema->Is_String() ||
		PRODUCT_SCHEMA != productSchema->Get_String() ||
		!Read_ExactUnsigned(productRoot, "formatVersion", FORMAT_VERSION) ||
		nullptr == encounter || !encounter->Is_String() ||
		ENCOUNTER_ID != encounter->Get_String() ||
		nullptr == objects || !objects->Is_Array() ||
		objects->Get_Array().empty())
	{
		outStatus = "Valtan combat-object Product header is invalid.";
		return false;
	}

	std::unordered_set<std::string> productSources;
	for (const DATA_JSON_VALUE& object : objects->Get_Array())
	{
		std::string archetypeId;
		const DATA_JSON_VALUE* hits = object.Find("hits");
		if (!Read_StableString(object, "combatObjectArchetypeId", archetypeId) ||
			nullptr == hits || !hits->Is_Array() || hits->Get_Array().empty())
		{
			outStatus = "Valtan combat-object Product source is invalid.";
			return false;
		}
		for (const DATA_JSON_VALUE& hit : hits->Get_Array())
		{
			std::string hitId;
			if (!Read_StableString(hit, "hitId", hitId) ||
				!productSources.insert(Make_SourceKey(archetypeId, hitId)).second)
			{
				outStatus =
					"Valtan combat-object Product hit identity is missing or duplicate.";
				return false;
			}
		}
	}

	DATA_JSON_VALUE cueRoot;
	parseError.clear();
	if (!CDataJson::Parse(cueText, cueRoot, parseError, limits) ||
		!Is_ExactObject(cueRoot,
			{ "schema", "formatVersion", "ownerArchetypeId", "cues" }))
	{
		outStatus = "Valtan combat-object Sound cue JSON is malformed: " +
			parseError;
		return false;
	}
	const DATA_JSON_VALUE* cueSchema = cueRoot.Find("schema");
	const DATA_JSON_VALUE* owner = cueRoot.Find("ownerArchetypeId");
	const DATA_JSON_VALUE* cues = cueRoot.Find("cues");
	if (nullptr == cueSchema || !cueSchema->Is_String() ||
		SCHEMA != cueSchema->Get_String() ||
		!Read_ExactUnsigned(cueRoot, "formatVersion", FORMAT_VERSION) ||
		nullptr == owner || !owner->Is_String() ||
		OWNER_ARCHETYPE_ID != owner->Get_String() ||
		nullptr == cues || !cues->Is_Array() || cues->Get_Array().empty() ||
		cues->Get_Array().size() > MAX_CUE_COUNT)
	{
		outStatus = "Valtan combat-object Sound cue header is invalid.";
		return false;
	}

	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT staged;
	staged.iFormatVersion = FORMAT_VERSION;
	staged.strOwnerArchetypeId = std::string(OWNER_ARCHETYPE_ID);
	staged.Cues.reserve(cues->Get_Array().size());
	std::unordered_set<std::string> bindingIds;
	std::unordered_set<std::string> boundSources;
	for (const DATA_JSON_VALUE& value : cues->Get_Array())
	{
		if (!Is_ExactObject(value,
			{ "bindingId", "combatObjectArchetypeId", "hitId",
			  "soundBank", "soundEvent" }))
		{
			outStatus =
				"Valtan combat-object Sound cue has unexpected properties.";
			return false;
		}
		VALTAN_COMBAT_OBJECT_SOUND_CUE cue;
		if (!Read_StableString(value, "bindingId", cue.strBindingId) ||
			!Read_StableString(value, "combatObjectArchetypeId",
				cue.strCombatObjectArchetypeId) ||
			!Read_StableString(value, "hitId", cue.strHitId) ||
			!Read_StableString(value, "soundBank", cue.strSoundBank) ||
			!Read_StableString(value, "soundEvent", cue.strSoundEvent) ||
			SOUND_BANK != cue.strSoundBank ||
			!bindingIds.insert(cue.strBindingId).second)
		{
			outStatus =
				"Valtan combat-object Sound cue identity is invalid.";
			return false;
		}
		const std::string sourceKey = Make_SourceKey(
			cue.strCombatObjectArchetypeId, cue.strHitId);
		if (!productSources.contains(sourceKey) ||
			!boundSources.insert(sourceKey).second)
		{
			outStatus =
				"Valtan combat-object Sound cue source is missing or duplicate: " +
				cue.strBindingId;
			return false;
		}
		staged.Cues.push_back(std::move(cue));
	}
	std::sort(staged.Cues.begin(), staged.Cues.end(),
		[](const VALTAN_COMBAT_OBJECT_SOUND_CUE& left,
			const VALTAN_COMBAT_OBJECT_SOUND_CUE& right)
		{
			return std::tie(left.strCombatObjectArchetypeId, left.strHitId,
				left.strBindingId) <
				std::tie(right.strCombatObjectArchetypeId, right.strHitId,
					right.strBindingId);
		});
	inOutDocument = std::move(staged);
	outStatus = "Parsed " + std::to_string(inOutDocument.Cues.size()) +
		" Server-hit-qualified Valtan combat-object Sound cue(s).";
	return true;
}

bool_t Client::CValtanCombatObjectSoundCueDocument::Load_Source(
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& inOutDocument,
	std::string& outStatus)
{
	const std::filesystem::path cuePath = Resolve_Path();
	const std::filesystem::path productPath = Resolve_CombatObjectProductPath();
	std::string cueText;
	std::string productText;
	if (!Read_File(cuePath, cueText))
	{
		outStatus = "Missing Valtan combat-object Sound cue document: " +
			cuePath.string();
		return false;
	}
	if (!Read_File(productPath, productText))
	{
		outStatus = "Missing Valtan combat-object Product document: " +
			productPath.string();
		return false;
	}
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT staged;
	if (!Parse_Text(cueText, productText, staged, outStatus))
		return false;
	if (!Validate_CatalogAssets(staged, outStatus))
		return false;
	inOutDocument = std::move(staged);
	return true;
}

bool_t Client::CValtanCombatObjectSoundCueDocument::Validate_SourceDraft(
	const VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& document,
	std::string& outStatus)
{
	std::string productText;
	if (!Read_File(Resolve_CombatObjectProductPath(), productText))
	{
		outStatus = "Missing Valtan combat-object Product document.";
		return false;
	}
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT staged;
	if (!Parse_Text(Serialize_Document(document), productText, staged, outStatus) ||
		!Validate_CatalogAssets(staged, outStatus))
	{
		return false;
	}
	outStatus = "Validated " + std::to_string(staged.Cues.size()) +
		" Server-hit-qualified Valtan Sound cue(s).";
	return true;
}

bool_t Client::CValtanCombatObjectSoundCueDocument::Begin_SourceReplacement(
	const VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& document,
	VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT& transaction,
	std::string& outStatus)
{
	if (transaction.bActive)
	{
		outStatus = "Valtan Sound cue source replacement is already active.";
		return false;
	}
	if (!Validate_SourceDraft(document, outStatus))
		return false;
	const std::filesystem::path destination = Resolve_Path();
	if (destination.empty())
	{
		outStatus = "Valtan combat-object Sound cue destination is unresolved.";
		return false;
	}
	std::error_code directoryError;
	std::filesystem::create_directories(destination.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create Valtan Sound cue directory: " +
			directoryError.message();
		return false;
	}
	const std::wstring suffix = Make_TransactionSuffix();
	std::filesystem::path temporary = destination;
	temporary += L".stage" + suffix;
	std::filesystem::path rollback = destination;
	rollback += L".rollback" + suffix;
	std::error_code cleanupError;
	std::filesystem::remove(temporary, cleanupError);
	std::filesystem::remove(rollback, cleanupError);
	if (!Write_StagedSource(temporary, Serialize_Document(document), outStatus))
		return false;

	cleanupError.clear();
	const bool_t hadPrevious =
		std::filesystem::is_regular_file(destination, cleanupError);
	const bool_t destinationExists =
		std::filesystem::exists(destination, cleanupError);
	if (cleanupError || (destinationExists && !hadPrevious))
	{
		std::filesystem::remove(temporary, cleanupError);
		outStatus =
			"Valtan Sound cue destination is not a replaceable regular file.";
		return false;
	}
	if (hadPrevious && !CopyFileW(destination.c_str(), rollback.c_str(), TRUE))
	{
		const DWORD error = GetLastError();
		std::filesystem::remove(temporary, cleanupError);
		outStatus = "Valtan Sound cue rollback copy failed (Win32 " +
			std::to_string(error) + "); previous source preserved.";
		return false;
	}
	if (!MoveFileExW(temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		const DWORD error = GetLastError();
		std::filesystem::remove(temporary, cleanupError);
		std::filesystem::remove(rollback, cleanupError);
		outStatus = "Valtan Sound cue recoverable replace failed (Win32 " +
			std::to_string(error) + "); previous source preserved.";
		return false;
	}
	transaction.Destination = destination;
	transaction.Rollback = rollback;
	transaction.bHadPrevious = hadPrevious;
	transaction.bActive = true;
	outStatus = "Prepared " + std::to_string(document.Cues.size()) +
		" recoverable Server-hit-qualified Valtan Sound cue(s).";
	return true;
}

bool_t Client::CValtanCombatObjectSoundCueDocument::Commit_SourceReplacement(
	VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT& transaction,
	std::string& outStatus)
{
	if (!transaction.bActive || transaction.Destination.empty())
	{
		outStatus = "No active Valtan Sound cue source replacement to commit.";
		return false;
	}
	const std::string rollbackPath = transaction.Rollback.string();
	std::error_code cleanupError;
	if (transaction.bHadPrevious)
		std::filesystem::remove(transaction.Rollback, cleanupError);
	transaction = {};
	if (cleanupError)
	{
		outStatus =
			"Valtan Sound cue source was committed; rollback-copy cleanup was deferred: " +
			cleanupError.message() + " (" + rollbackPath + ").";
		return true;
	}
	outStatus = "Committed Valtan Sound cue source replacement.";
	return true;
}

bool_t Client::CValtanCombatObjectSoundCueDocument::Rollback_SourceReplacement(
	VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT& transaction,
	std::string& outStatus)
{
	if (!transaction.bActive || transaction.Destination.empty())
	{
		outStatus = "No active Valtan Sound cue source replacement to roll back.";
		return false;
	}
	if (transaction.bHadPrevious)
	{
		if (transaction.Rollback.empty() ||
			!MoveFileExW(transaction.Rollback.c_str(),
				transaction.Destination.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			outStatus = "Valtan Sound cue rollback failed (Win32 " +
				std::to_string(GetLastError()) + "); recovery copy remains at " +
				transaction.Rollback.string() + ".";
			return false;
		}
	}
	else
	{
		const bool_t removed = DeleteFileW(transaction.Destination.c_str());
		const DWORD error = removed ? ERROR_SUCCESS : GetLastError();
		if (!removed && ERROR_FILE_NOT_FOUND != error)
		{
			outStatus =
				"Valtan Sound cue rollback could not remove the newly created source (Win32 " +
				std::to_string(error) + ").";
			return false;
		}
	}
	transaction = {};
	outStatus = "Restored the previous Valtan Sound cue source.";
	return true;
}

bool_t Client::CValtanCombatObjectSoundCueDocument::Save_Source(
	const VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT& document,
	std::string& outStatus)
{
	VALTAN_COMBAT_OBJECT_SOUND_CUE_SOURCE_REPLACEMENT transaction;
	if (!Begin_SourceReplacement(document, transaction, outStatus))
		return false;
	std::string commitStatus;
	if (!Commit_SourceReplacement(transaction, commitStatus))
	{
		std::string rollbackStatus;
		(void)Rollback_SourceReplacement(transaction, rollbackStatus);
		outStatus = "Valtan Sound cue commit failed: " + commitStatus + " " +
			rollbackStatus;
		return false;
	}
	outStatus = "Saved " + std::to_string(document.Cues.size()) +
		" Server-hit-qualified Valtan Sound cue(s). " + commitStatus;
	return true;
}
