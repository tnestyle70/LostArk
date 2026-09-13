#include "imgui.h"
#include "Effect_Tool_Internal.h"
#include "AnimationSkillBindingDocument.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "DataJson.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_DocumentRenderer.h"
#include "ValtanPatternEffectCueDocument.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Playback.h"
#include "Effect_PresentationService.h"
#include "Effect_RuntimeAuthority.h"
#include "Effect_ThumbnailCache.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Logic_Artist.h"
#include "Logic_DimensionMaster.h"
#include "Logic_GunSlinger.h"
#include "Logic_LanceMaster.h"
#include "Logic_Slayer.h"
#include "Logic_Warlord.h"
#include "MainApp.h"
#include "Model.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <set>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "Transform.h"
#include "Valtan.h"

namespace EffectToolDetail
{

	bool Is_ValtanExactHistoryPreviewEffectAssetId(
		const std::string_view strEffectAssetId)
	{
		return strEffectAssetId == VALTAN_EXACT_HISTORY_EFFECT_ASSET_ID ||
			strEffectAssetId == VALTAN_EXACT_HISTORY_V1_EFFECT_ASSET_ID;
	}

	bool Matches_ValtanExactHistoryBinding(
		const std::string_view strBindingId,
		const std::string_view strBindingEffectAssetId,
		const std::string_view strPreviewEffectAssetId)
	{
		return strBindingId == VALTAN_EXACT_HISTORY_BINDING_ID &&
			strBindingEffectAssetId == VALTAN_EXACT_HISTORY_EFFECT_ASSET_ID &&
			Is_ValtanExactHistoryPreviewEffectAssetId(strPreviewEffectAssetId);
	}

	const char_t* Resolve_ValtanServerPatternBossPlacement(
		const uint32_t iLevel)
	{
		return ETOUI(Client::LEVEL::VALTAN_ARENA) == iLevel ?
			VALTAN_ARENA_BOSS_PLACEMENT_ID : nullptr;
	}

	void Select_SharedCompletePlayPattern(
		const std::string& strPatternId)
	{
#ifdef _DEBUG
		if (CMainApp* const pApp = CMainApp::Get_Active())
			(void)pApp->Debug_SelectCompletePlayPattern(strPatternId);
#else
		UNREFERENCED_PARAMETER(strPatternId);
#endif
	}

	bool Remove_EffectDocumentIfCanonical(
		const std::filesystem::path& Path,
		const std::string_view strExpectedCanonical,
		std::string& strOutStatus)
	{
		/* Deny write/delete sharing while the exact file is parsed and marked for
		   deletion. This closes the compare-then-remove race in New Effect
		   rollback; an editor that already owns an incompatible handle wins and
		   the file is preserved. */
		const HANDLE hFile = CreateFileW(
			Path.c_str(), GENERIC_READ | DELETE, FILE_SHARE_READ, nullptr,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			strOutStatus =
				"The Effect document could not be locked for exact deletion.";
			return false;
		}

		constexpr LONGLONG MAX_ROLLBACK_DOCUMENT_BYTES =
			64ll * 1024ll * 1024ll;
		LARGE_INTEGER FileSize{};
		std::string Text;
		bool_t bRead = FALSE != GetFileSizeEx(hFile, &FileSize) &&
			FileSize.QuadPart >= 0ll &&
			FileSize.QuadPart <= MAX_ROLLBACK_DOCUMENT_BYTES;
		if (bRead)
		{
			Text.resize(static_cast<size_t>(FileSize.QuadPart));
			size_t iOffset = 0u;
			while (iOffset < Text.size())
			{
				const DWORD iChunk = static_cast<DWORD>((std::min)(
					Text.size() - iOffset,
					static_cast<size_t>((std::numeric_limits<DWORD>::max)())));
				DWORD iRead = 0u;
				if (FALSE == ReadFile(
						hFile, Text.data() + iOffset, iChunk, &iRead, nullptr) ||
					0u == iRead)
				{
					bRead = false;
					break;
				}
				iOffset += iRead;
			}
		}
		Client::EFFECT_DOCUMENT_DESC Current;
		std::string Error;
		const bool_t bExact = bRead &&
			Client::CEffectDocumentCodec::Parse(Text, Current, Error) &&
			Client::CEffectDocumentCodec::Serialize(Current) ==
				strExpectedCanonical;
		if (!bExact)
		{
			CloseHandle(hFile);
			strOutStatus = Error.empty() ?
				"The Effect document changed before exact deletion and was preserved." :
				("The Effect document could not be verified for exact deletion: " +
				 Error);
			return false;
		}

		FILE_DISPOSITION_INFO Disposition{};
		Disposition.DeleteFile = TRUE;
		const bool_t bMarked = FALSE != SetFileInformationByHandle(
			hFile, FileDispositionInfo, &Disposition, sizeof(Disposition));
		CloseHandle(hFile);
		strOutStatus = bMarked ?
			"The Effect document was deleted after exact canonical verification." :
			"The Effect document could not be marked for exact deletion and was preserved.";
		return bMarked;
	}

	size_t Count_ValtanProductReferences(
		const Client::VALTAN_PATTERN_TREE_VIEW& Tree,
		const std::string& strEffectAssetId)
	{
		size_t iCount = 0u;
		const auto CountPattern = [&strEffectAssetId, &iCount](
			const Client::VALTAN_PATTERN_VIEW& Pattern)
		{
			for (const Client::VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
			{
				for (const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
					Stage.ProductCues)
				{
					if (Cue.strEffectAssetId == strEffectAssetId ||
						Cue.strV1EffectAssetId == strEffectAssetId)
					{
						++iCount;
					}
				}
				for (const Client::VALTAN_STAGE_EFFECT_VIEW& Effect : Stage.Effects)
				{
					if (Effect.strEffectAssetId == strEffectAssetId)
						++iCount;
				}
				for (const Client::VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Effect :
					Stage.CombatObjectEffects)
				{
					if (Effect.strEffectAssetId == strEffectAssetId)
						++iCount;
				}
			}
		};
		for (const Client::VALTAN_PATTERN_VIEW& Pattern : Tree.Gimmicks)
			CountPattern(Pattern);
		for (const Client::VALTAN_PATTERN_VIEW& Pattern : Tree.Rotation)
			CountPattern(Pattern);
		for (const Client::VALTAN_INDEPENDENT_EFFECT_VIEW& Effect :
			Tree.IndependentEffects)
		{
			if (Effect.strEffectAssetId == strEffectAssetId)
				++iCount;
		}
		return iCount;
	}

	bool_t Try_LockAndInspectAuthoritativeProductOwnership(
		const std::string& strEffectAssetId,
		CAuthoritativeProductSourceReadLocks& SourceLocks,
		bool_t& bOutProductOwned,
		std::string& strOutStatus)
	{
		bOutProductOwned = false;
		if (!SourceLocks.Try_Acquire(strOutStatus))
			return false;
		bool_t bCatalogContains = false;
		if (!Client::CEffectCatalog::Try_ContainsSourceRegistrationFresh(
				strEffectAssetId, bCatalogContains, strOutStatus))
		{
			return false;
		}
		Client::VALTAN_PATTERN_TREE_VIEW FreshTree;
		std::string TreeStatus;
		if (!Client::CValtanPatternTree::Load(FreshTree, TreeStatus))
		{
			strOutStatus =
				"Fresh Valtan Product graph preflight failed; the Draft was preserved. " +
				TreeStatus;
			return false;
		}
		const size_t iReferenceCount = Count_ValtanProductReferences(
			FreshTree, strEffectAssetId);
		bOutProductOwned = bCatalogContains || 0u != iReferenceCount;
		strOutStatus = bOutProductOwned ?
			("Authoritative Product owns the Effect (Valtan references=" +
			 std::to_string(iReferenceCount) + ").") :
			"Authoritative Product does not own the Draft Effect.";
		return true;
	}

	bool_t Read_TransactionRawBytes(
		const std::filesystem::path& Path,
		std::string& OutBytes,
		std::string& strOutStatus)
	{
		OutBytes.clear();
		const HANDLE hFile = CreateFileW(Path.c_str(), GENERIC_READ,
			FILE_SHARE_READ, nullptr, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, nullptr);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			strOutStatus = "Transaction source could not be opened.";
			return false;
		}
		LARGE_INTEGER FileSize{};
		if (FALSE == GetFileSizeEx(hFile, &FileSize) ||
			FileSize.QuadPart <= 0ll ||
			static_cast<uint64_t>(FileSize.QuadPart) >
				MAX_EFFECT_TOOL_TRANSACTION_BYTES)
		{
			CloseHandle(hFile);
			strOutStatus =
				"Transaction source is empty or exceeds 16 MiB.";
			return false;
		}
		OutBytes.resize(static_cast<size_t>(FileSize.QuadPart));
		size_t iOffset = 0u;
		while (iOffset < OutBytes.size())
		{
			const DWORD iRequest = static_cast<DWORD>((std::min)(
				OutBytes.size() - iOffset,
				static_cast<size_t>((std::numeric_limits<DWORD>::max)())));
			DWORD iRead = 0u;
			if (FALSE == ReadFile(hFile, OutBytes.data() + iOffset,
					iRequest, &iRead, nullptr) || 0u == iRead)
			{
				CloseHandle(hFile);
				OutBytes.clear();
				strOutStatus = "Transaction source read was incomplete.";
				return false;
			}
			iOffset += iRead;
		}
		CloseHandle(hFile);
		strOutStatus.clear();
		return true;
	}

	std::filesystem::path Make_TransactionTemporaryPath(
		const std::filesystem::path& Destination,
		const wchar_t* pRole)
	{
		static std::atomic_uint64_t Counter = 0u;
		return Destination.wstring() + L"." + pRole + L"." +
			std::to_wstring(std::chrono::steady_clock::now().
				time_since_epoch().count()) + L"." +
			std::to_wstring(Counter.fetch_add(
				1u, std::memory_order_relaxed));
	}

	bool_t Write_TransactionRawBytesAtomicIfUnchanged(
		const std::filesystem::path& Path,
		const std::string_view Replacement,
		const std::string_view ExpectedCurrent,
		std::string& strOutStatus)
	{
		std::string Observed;
		if (!Read_TransactionRawBytes(Path, Observed, strOutStatus) ||
			Observed != ExpectedCurrent)
		{
			strOutStatus =
				"Transaction source changed before save; Reload Source first.";
			return false;
		}
		const std::filesystem::path Temporary =
			Make_TransactionTemporaryPath(Path, L"tmp");
		const HANDLE hFile = CreateFileW(Temporary.c_str(), GENERIC_WRITE,
			0u, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			strOutStatus = "Transaction temporary file could not be created.";
			return false;
		}
		size_t iOffset = 0u;
		bool_t bWrote = true;
		while (iOffset < Replacement.size())
		{
			const DWORD iRequest = static_cast<DWORD>((std::min)(
				Replacement.size() - iOffset,
				static_cast<size_t>((std::numeric_limits<DWORD>::max)())));
			DWORD iWritten = 0u;
			if (FALSE == WriteFile(hFile, Replacement.data() + iOffset,
					iRequest, &iWritten, nullptr) || 0u == iWritten)
			{
				bWrote = false;
				break;
			}
			iOffset += iWritten;
		}
		const bool_t bFlushed = bWrote && FALSE != FlushFileBuffers(hFile);
		CloseHandle(hFile);
		if (!bFlushed)
		{
			DeleteFileW(Temporary.c_str());
			strOutStatus = "Transaction temporary write/flush failed.";
			return false;
		}
		Observed.clear();
		if (!Read_TransactionRawBytes(Path, Observed, strOutStatus) ||
			Observed != ExpectedCurrent)
		{
			DeleteFileW(Temporary.c_str());
			strOutStatus =
				"Transaction source changed during save; temporary data was discarded.";
			return false;
		}
		if (FALSE == MoveFileExW(Temporary.c_str(), Path.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			DeleteFileW(Temporary.c_str());
			strOutStatus = "Transaction write-through replace failed.";
			return false;
		}
		strOutStatus =
			"Transaction source replaced by exact-byte two-pass CAS.";
		return true;
	}

	bool_t Create_TransactionRawBytesDurableIfAbsent(
		const std::filesystem::path& Path,
		const std::string_view Contents,
		std::string& strOutStatus)
	{
		if (Path.empty() || Contents.empty())
		{
			strOutStatus =
				"Transaction create requires a path and non-empty contents.";
			return false;
		}
		const std::filesystem::path Temporary =
			Make_TransactionTemporaryPath(Path, L"create");
		const HANDLE hFile = CreateFileW(Temporary.c_str(), GENERIC_WRITE,
			0u, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			strOutStatus =
				"Transaction create temporary file could not be created.";
			return false;
		}
		size_t iOffset = 0u;
		bool_t bWrote = true;
		while (iOffset < Contents.size())
		{
			const DWORD iRequest = static_cast<DWORD>((std::min)(
				Contents.size() - iOffset,
				static_cast<size_t>((std::numeric_limits<DWORD>::max)())));
			DWORD iWritten = 0u;
			if (FALSE == WriteFile(hFile, Contents.data() + iOffset,
					iRequest, &iWritten, nullptr) || 0u == iWritten)
			{
				bWrote = false;
				break;
			}
			iOffset += iWritten;
		}
		const bool_t bFlushed = bWrote && FALSE != FlushFileBuffers(hFile);
		CloseHandle(hFile);
		if (!bFlushed)
		{
			DeleteFileW(Temporary.c_str());
			strOutStatus =
				"Transaction create temporary write/flush failed.";
			return false;
		}

		/* No REPLACE flag: a concurrent creator wins and its bytes are preserved.
		   WRITE_THROUGH makes the new source as durable as the two replacement
		   documents before the transaction advances to their promotion. */
		if (FALSE == MoveFileExW(Temporary.c_str(), Path.c_str(),
				MOVEFILE_WRITE_THROUGH))
		{
			const DWORD iError = GetLastError();
			DeleteFileW(Temporary.c_str());
			strOutStatus = ERROR_ALREADY_EXISTS == iError ||
				ERROR_FILE_EXISTS == iError ?
				"Transaction create collided with an existing destination." :
				"Transaction create write-through promotion failed.";
			return false;
		}
		strOutStatus =
			"Transaction source created by flushed write-through promotion.";
		return true;
	}

	bool_t Remove_TransactionFileIfExactRaw(
		const std::filesystem::path& Path,
		const std::string_view ExpectedRaw,
		std::string& strOutStatus)
	{
		const HANDLE hFile = CreateFileW(Path.c_str(), GENERIC_READ | DELETE,
			FILE_SHARE_READ, nullptr, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, nullptr);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			strOutStatus =
				"Transaction-created file could not be locked for rollback.";
			return false;
		}
		LARGE_INTEGER FileSize{};
		std::string Observed;
		bool_t bExact = FALSE != GetFileSizeEx(hFile, &FileSize) &&
			FileSize.QuadPart >= 0ll &&
			static_cast<uint64_t>(FileSize.QuadPart) <=
				MAX_EFFECT_TOOL_TRANSACTION_BYTES &&
			static_cast<uint64_t>(FileSize.QuadPart) == ExpectedRaw.size();
		if (bExact)
		{
			Observed.resize(static_cast<size_t>(FileSize.QuadPart));
			size_t iOffset = 0u;
			while (iOffset < Observed.size())
			{
				const DWORD iRequest = static_cast<DWORD>((std::min)(
					Observed.size() - iOffset,
					static_cast<size_t>((std::numeric_limits<DWORD>::max)())));
				DWORD iRead = 0u;
				if (FALSE == ReadFile(hFile, Observed.data() + iOffset,
						iRequest, &iRead, nullptr) || 0u == iRead)
				{
					bExact = false;
					break;
				}
				iOffset += iRead;
			}
			bExact = bExact && Observed == ExpectedRaw;
		}
		if (!bExact)
		{
			CloseHandle(hFile);
			strOutStatus =
				"Transaction-created file changed before rollback and was preserved.";
			return false;
		}
		FILE_DISPOSITION_INFO Disposition{};
		Disposition.DeleteFile = TRUE;
		const bool_t bMarked = FALSE != SetFileInformationByHandle(
			hFile, FileDispositionInfo, &Disposition, sizeof(Disposition));
		CloseHandle(hFile);
		strOutStatus = bMarked ?
			"Transaction-created file was removed by exact-byte CAS." :
			"Transaction-created file could not be marked for rollback.";
		return bMarked;
	}

	bool_t Build_EffectCatalogWithDirectAuthoredRow(
		const std::string& strEffectAssetId,
		std::string& OutPreviousRaw,
		std::string& OutCandidateRaw,
		std::filesystem::path& OutCatalogPath,
		std::string& strOutStatus)
	{
		OutCatalogPath = Client::CProjectDataRoot::Resolve(
			std::filesystem::path(L"Effects") / L"EffectCatalog.json");
		if (OutCatalogPath.empty() || !Read_TransactionRawBytes(
				OutCatalogPath, OutPreviousRaw, strOutStatus))
		{
			return false;
		}
		Client::DATA_JSON_VALUE Root;
		Client::DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = MAX_EFFECT_TOOL_TRANSACTION_BYTES;
		Limits.iMaximumDepth = 16u;
		Limits.iMaximumValues = 100'000u;
		std::string Error;
		if (!Client::CDataJson::Parse(
				OutPreviousRaw, Root, Error, Limits) || !Root.Is_Object() ||
			Root.Get_Object().size() != 2u ||
			Root.Get_ObjectInsertionOrder() !=
				std::vector<std::string>{ "formatVersion", "effects" })
		{
			strOutStatus = "EffectCatalog root is not canonical: " + Error;
			return false;
		}
		const Client::DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
		const Client::DATA_JSON_VALUE* pEffects = Root.Find("effects");
		if (nullptr == pVersion || !pVersion->Is_Number() ||
			pVersion->Was_FloatingPointToken() ||
			pVersion->Get_Number() != 1.0 || nullptr == pEffects ||
			!pEffects->Is_Array() || pEffects->Get_Array().empty())
		{
			strOutStatus = "EffectCatalog version/effects array is invalid.";
			return false;
		}
		std::vector<std::string> EffectIds;
		EffectIds.reserve(pEffects->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& Row : pEffects->Get_Array())
		{
			const Client::DATA_JSON_VALUE* pId = Row.Find("effectAssetId");
			if (!Row.Is_Object() || nullptr == pId || !pId->Is_String() ||
				pId->Get_String().empty())
			{
				strOutStatus =
					"EffectCatalog contains a row without a stable Effect identity.";
				return false;
			}
			EffectIds.push_back(pId->Get_String());
		}
		std::set<std::string, std::less<>> UniqueEffectIds(
			EffectIds.begin(), EffectIds.end());
		if (UniqueEffectIds.size() != EffectIds.size())
		{
			strOutStatus =
				"EffectCatalog contains duplicate Effect identities.";
			return false;
		}
		if (UniqueEffectIds.contains(strEffectAssetId))
		{
			strOutStatus =
				"EffectCatalog already contains the requested Effect ID.";
			return false;
		}
		const std::string NewLine =
			std::string::npos != OutPreviousRaw.find("\r\n") ? "\r\n" : "\n";
		const std::string EscapedId = Client::CDataJson::Escape(
			strEffectAssetId);
		const std::string RowBody =
			"    {" + NewLine +
			"      \"effectAssetId\": \"" + EscapedId + "\"," + NewLine +
			"      \"payloadKind\": \"DIRECT_AUTHORED_DOCUMENT\"," + NewLine +
			"      \"authoringPath\": \"Effects/Authored/" +
				EscapedId + ".effect.json\"" + NewLine +
			"    }";
		OutCandidateRaw = OutPreviousRaw;
		const size_t iArrayClose = OutCandidateRaw.rfind(NewLine + "  ]");
		if (std::string::npos == iArrayClose)
		{
			strOutStatus = "EffectCatalog effects array close was not found.";
			return false;
		}
		const size_t iLastToken = OutCandidateRaw.find_last_not_of(
			" \t\r\n", iArrayClose);
		if (std::string::npos == iLastToken ||
			OutCandidateRaw[iLastToken] != '}')
		{
			strOutStatus =
				"EffectCatalog last row is not safe for exact insertion.";
			return false;
		}
		OutCandidateRaw.insert(
			iLastToken + 1u, "," + NewLine + RowBody);
		Client::DATA_JSON_VALUE CandidateRoot;
		if (!Client::CDataJson::Parse(
				OutCandidateRaw, CandidateRoot, Error, Limits))
		{
			strOutStatus =
				"EffectCatalog candidate serialization is invalid: " + Error;
			return false;
		}
		strOutStatus.clear();
		return true;
	}

	const char_t* Tool_PlayerStanceLabel(
		const LostArk::Shared::PLAYER_STANCE_ID eStance)
	{
		using LostArk::Shared::PLAYER_STANCE_ID;
		switch (eStance)
		{
		case PLAYER_STANCE_ID::NONE: return "NONE";
		case PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR:
			return "LANCE_MASTER_LONG_SPEAR";
		case PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR:
			return "LANCE_MASTER_SHORT_SPEAR";
		case PLAYER_STANCE_ID::WARLORD_NORMAL: return "WARLORD_NORMAL";
		case PLAYER_STANCE_ID::WARLORD_DEFENSE: return "WARLORD_DEFENSE";
		case PLAYER_STANCE_ID::END:
		default: return "INVALID";
		}
	}

	std::string Tool_SkillIdentitySuffix(
		const Client::PLAYER_SKILL_DEFINITION& Skill)
	{
		return " | #" + std::to_string(Skill.iSkillId) + " | Stance " +
			Tool_PlayerStanceLabel(Skill.eRequiredStance);
	}

	std::vector<TOOL_SOURCE_ANCHOR_REQUEST> Collect_ToolSourceAnchorRequests(
		const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		std::vector<TOOL_SOURCE_ANCHOR_REQUEST> Requests;
		const auto AddRequest = [&Requests](TOOL_SOURCE_ANCHOR_REQUEST Request)
		{
			if (Request.strRuntimeAnchorSlotId.empty() ||
				(Request.eOrientation != Client::EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
				 Request.strRuntimeBoneName.empty()))
			{
				return;
			}
			const auto Existing = std::find_if(Requests.begin(), Requests.end(),
				[&Request](const TOOL_SOURCE_ANCHOR_REQUEST& Value)
				{
					return Value.strRuntimeAnchorSlotId ==
						Request.strRuntimeAnchorSlotId;
				});
			if (Existing == Requests.end())
				Requests.push_back(std::move(Request));
		};

		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Element.bVisible &&
				Element.ActionCueAttachment.bEnabled &&
				Element.ActionCueAttachment.bFollow &&
				Element.ActionCueAttachment.strModelCueId.empty())
			{
				AddRequest({
					Element.ActionCueAttachment.strRuntimeAnchorSlotId,
					Element.ActionCueAttachment.strRuntimeBoneName,
					Element.ActionCueAttachment.SocketLocalTransform,
					Element.ActionCueAttachment.eOrientation,
					Client::CEffectPresentationService::Requires_SourceBoneImportScaleNormalization(
						Document.strEffectAssetId) });
			}
		}
		std::sort(Requests.begin(), Requests.end(),
			[](const TOOL_SOURCE_ANCHOR_REQUEST& Left,
				const TOOL_SOURCE_ANCHOR_REQUEST& Right)
			{
				return Left.strRuntimeAnchorSlotId < Right.strRuntimeAnchorSlotId;
			});
		return Requests;
	}

	bool_t Has_RequiredSourceFollowAttachments(const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		return std::any_of(Document.Elements.begin(), Document.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{ return Element.bVisible && Element.ActionCueAttachment.bEnabled &&
				Element.ActionCueAttachment.bFollow &&
				(Element.ActionCueAttachment.eOrientation == Client::EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW ||
				 Element.ActionCueAttachment.eOrientation == Client::EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW); });
	}

	bool Try_ResolveToolAttachmentOwnerWorld(float4x4_t& OutWorld)
	{
		const auto pBoss = Client::CAnimationTargetService::Resolve_Boss();
		const auto pCharacter = Client::CAnimationTargetService::Resolve_Character();
		const auto pTargetModel = Client::CAnimationTargetService::Resolve_Model();
		if ((nullptr != pBoss && pBoss->Get_BodyModel() != pTargetModel) ||
			(nullptr == pBoss && (nullptr == pCharacter || pCharacter->Get_BodyModel() != pTargetModel)))
			return false;
		const auto pTransform = nullptr != pBoss ? pBoss->Get_Transform() :
			(nullptr != pCharacter ? pCharacter->Get_Transform() : nullptr);
		if (nullptr == pTransform)
			return false;
		OutWorld = *pTransform->Get_WorldMatrixPtr();
		return true;
	}

	bool Build_ToolValtanSourceAnchorWorld(
		const float4x4_t& RawBone,
		const float4x4_t& SampledOwnerRoot,
		const Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY eScalePolicy,
		const float3_t& WorldScale,
		const float4x4_t& ActualOwnerWorld,
		const Client::EFFECT_ATTACHMENT_ORIENTATION eOrientation,
		float4x4_t& OutWorld)
	{
		if (eOrientation == Client::EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW)
		{
			return Client::CEffectPlayback::Build_OwnerYawBoneAnchorWorld(
				RawBone, SampledOwnerRoot, ActualOwnerWorld, OutWorld);
		}
		if (eOrientation != Client::EFFECT_ATTACHMENT_ORIENTATION::BONE)
			return false;
		float4x4_t EffectiveOwnerRoot{};
		if (!Client::CEffectPresentationService::Build_CueScalePolicyAnchor(
				eScalePolicy, WorldScale, SampledOwnerRoot,
				EffectiveOwnerRoot))
		{
			return false;
		}
		Client::EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
		AnchorBuild.RawBone = RawBone;
		AnchorBuild.OwnerWorld = EffectiveOwnerRoot;
		return Client::CEffectPresentationService::Build_SourceBoneAnchorWorld(
			AnchorBuild, OutWorld);
	}

	bool Try_ResolveToolCameraAnchorWorld(
		const Client::EFFECT_TRANSFORM_DESC& Local, float4x4_t& OutWorld)
	{
		const float4x4_t* pCameraWorld = Engine::CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
		if (nullptr == pCameraWorld)
			return false;
		const matrix_t CameraWorld = XMLoadFloat4x4(pCameraWorld);
		const float fDeterminant = XMVectorGetX(XMMatrixDeterminant(CameraWorld));
		if (XMMatrixIsNaN(CameraWorld) || XMMatrixIsInfinite(CameraWorld) ||
			!std::isfinite(fDeterminant) || std::abs(fDeterminant) < 1.0e-12f)
			return false;
		const matrix_t SocketLocal = XMMatrixScaling(Local.vScale.x, Local.vScale.y, Local.vScale.z) *
			XMMatrixRotationRollPitchYaw(XMConvertToRadians(Local.vRotationDegrees.x),
				XMConvertToRadians(Local.vRotationDegrees.y), XMConvertToRadians(Local.vRotationDegrees.z)) *
			XMMatrixTranslation(Local.vPosition.x, Local.vPosition.y, Local.vPosition.z);
		XMStoreFloat4x4(&OutWorld, SocketLocal * CameraWorld);
		return true;
	}

	bool Resolve_ToolSourceAnchorWorlds(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW* pValtanCue,
		std::unordered_map<std::string, float4x4_t>& OutWorlds,
		std::string& strOutError)
	{
		OutWorlds.clear();
		const std::vector<TOOL_SOURCE_ANCHOR_REQUEST> Requests =
			Collect_ToolSourceAnchorRequests(Document);
		OutWorlds.reserve(Requests.size());
		std::shared_ptr<Engine::CModel> pValtanModel;
		float4x4_t SampledValtanOwnerRoot{};
		float4x4_t ActualOwnerWorld{};
		const bool_t bNeedsOwnerYaw = std::any_of(Requests.begin(), Requests.end(),
			[](const TOOL_SOURCE_ANCHOR_REQUEST& Request)
			{ return Request.eOrientation == Client::EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW; });
		if (bNeedsOwnerYaw && !Try_ResolveToolAttachmentOwnerWorld(ActualOwnerWorld))
		{
			strOutError = "owner_yaw attachment requires the actual preview owner transform.";
			return false;
		}
		const bool_t bNeedsImportScaleNormalization = std::any_of(Requests.begin(), Requests.end(),
			[](const TOOL_SOURCE_ANCHOR_REQUEST& Request)
			{ return Request.bNormalizeSourceImportScale; });
		if ((nullptr != pValtanCue || bNeedsOwnerYaw || bNeedsImportScaleNormalization) && !Requests.empty())
		{
			pValtanModel = Client::CAnimationTargetService::Resolve_Model();
			if (nullptr == pValtanModel ||
				!Client::CAnimationTargetService::Resolve_RootTransform(
					&SampledValtanOwnerRoot))
			{
				strOutError =
					"preview model cannot resolve the Valtan cue owner root.";
				return false;
			}
		}
		bool_t bAllResolved = true;
		for (const TOOL_SOURCE_ANCHOR_REQUEST& Request : Requests)
		{
			if (Request.eOrientation == Client::EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW)
			{
				float4x4_t CameraAnchorWorld{};
				if (!Try_ResolveToolCameraAnchorWorld(Request.SocketLocalTransform, CameraAnchorWorld))
				{
					strOutError = "camera_view requires the actual invertible scene view.";
					bAllResolved = false;
					continue;
				}
				OutWorlds.emplace(Request.strRuntimeAnchorSlotId, CameraAnchorWorld);
				continue;
			}
			float4x4_t BoneAnchorWorld{};
			const bool_t bUsesRawBone = nullptr != pValtanCue ||
				Request.bNormalizeSourceImportScale ||
				Request.eOrientation == Client::EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW;
			const bool_t bResolved = bUsesRawBone ?
				(nullptr != pValtanModel &&
				 pValtanModel->Has_Bone(Request.strRuntimeBoneName.c_str())) :
				Client::CAnimationTargetService::Resolve_AnchorTransform(
					Request.strRuntimeBoneName.c_str(), &BoneAnchorWorld);
			if (!bResolved)
			{
				if (strOutError.empty())
				{
					strOutError = "preview model cannot resolve source slot '" +
						Request.strRuntimeAnchorSlotId + "' from bone '" +
						Request.strRuntimeBoneName + "'.";
				}
				bAllResolved = false;
				continue;
			}
			if (bUsesRawBone)
			{
				float4x4_t RawBone{};
				XMStoreFloat4x4(&RawBone, pValtanModel->Get_BoneMatrix(
					Request.strRuntimeBoneName.c_str()));
				if (Request.bNormalizeSourceImportScale && nullptr == pValtanCue &&
					Request.eOrientation == Client::EFFECT_ATTACHMENT_ORIENTATION::BONE)
				{
					Client::EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
					AnchorBuild.RawBone = RawBone;
					AnchorBuild.OwnerWorld = SampledValtanOwnerRoot;
					if (!Client::CEffectPresentationService::Build_SourceBoneAnchorWorld(AnchorBuild, BoneAnchorWorld))
					{
						strOutError = "Source follow bone does not match its admitted import scale: " + Request.strRuntimeBoneName;
						bAllResolved = false;
						continue;
					}
				}
				else if (!Build_ToolValtanSourceAnchorWorld(
						RawBone, SampledValtanOwnerRoot,
						nullptr != pValtanCue ? pValtanCue->eScalePolicy :
							Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE,
						nullptr != pValtanCue ? pValtanCue->vWorldScale : float3_t{ 1.f, 1.f, 1.f },
						ActualOwnerWorld, Request.eOrientation, BoneAnchorWorld))
				{
					if (strOutError.empty())
					{
						strOutError =
							"preview model cannot normalize the Valtan source bone '" +
							Request.strRuntimeBoneName + "' with its cue scale policy.";
					}
					bAllResolved = false;
					continue;
				}
			}

			const Client::EFFECT_TRANSFORM_DESC& Local =
				Request.SocketLocalTransform;
			const matrix_t SocketLocal = XMMatrixScaling(
				Local.vScale.x, Local.vScale.y, Local.vScale.z) *
				XMMatrixRotationRollPitchYaw(
					XMConvertToRadians(Local.vRotationDegrees.x),
					XMConvertToRadians(Local.vRotationDegrees.y),
					XMConvertToRadians(Local.vRotationDegrees.z)) *
				XMMatrixTranslation(
					Local.vPosition.x, Local.vPosition.y, Local.vPosition.z);
			float4x4_t World{};
			XMStoreFloat4x4(&World,
				SocketLocal * XMLoadFloat4x4(&BoneAnchorWorld));
			OutWorlds.emplace(Request.strRuntimeAnchorSlotId, World);
		}
		if (bAllResolved)
			strOutError.clear();
		return bAllResolved;
	}

	bool Is_CompilerOwnedPortableRecipe(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Element.SourceRecipe.bEnabled && !Element.SourceRecipe.bAuthoredModuleOverrides &&
			!Document.bSourceContract &&
			Element.SourceRecipe.strSourceContractProfileId.empty() &&
			Element.SourceRecipe.strSourceContractSha256.empty() &&
			Element.SourceRecipe.strSourceGraphSha256.empty();
	}

	bool Try_ParseMaterialExecutionLaneSlotId(
		const std::string_view strSlotId,
		std::string_view& strOutLaneId)
	{
		return Client::Try_ParseEffectMaterialExecutionLaneStableSlotId(
			strSlotId, strOutLaneId);
	}

	std::string MaterialExecutionLaneSlotId(const std::string& strLaneId)
	{
		return Client::Build_EffectMaterialExecutionLaneStableSlotId(strLaneId);
	}

	Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC* Find_MaterialExecutionLane(
		Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId)
	{
		std::string_view strLaneId;
		if (!Try_ParseMaterialExecutionLaneSlotId(strSlotId, strLaneId))
			return nullptr;
		const auto Iterator = std::find_if(
			Element.Material.Execution.TextureLanes.begin(),
			Element.Material.Execution.TextureLanes.end(),
			[strLaneId](
				const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane)
			{ return Lane.strLaneId == strLaneId; });
		return Iterator == Element.Material.Execution.TextureLanes.end() ?
			nullptr : &*Iterator;
	}

	Client::EFFECT_NAMED_TEXTURE_DESC* Find_SourceMaterialTexture(
		Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId)
	{
		std::string_view strName;
		if (!Client::Try_ParseEffectSourceMaterialTextureStableSlotId(
				strSlotId, strName))
		{
			return nullptr;
		}
		const auto Iterator = std::find_if(
			Element.Material.SourceMaterial.Textures.begin(),
			Element.Material.SourceMaterial.Textures.end(),
			[strName](const Client::EFFECT_NAMED_TEXTURE_DESC& Texture)
			{ return Texture.strName == strName; });
		return Iterator == Element.Material.SourceMaterial.Textures.end() ?
			nullptr : &*Iterator;
	}

	const Client::EFFECT_NAMED_TEXTURE_DESC* Find_SourceMaterialTexture(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId)
	{
		std::string_view strName;
		if (!Client::Try_ParseEffectSourceMaterialTextureStableSlotId(
				strSlotId, strName))
		{
			return nullptr;
		}
		const auto Iterator = std::find_if(
			Element.Material.SourceMaterial.Textures.begin(),
			Element.Material.SourceMaterial.Textures.end(),
			[strName](const Client::EFFECT_NAMED_TEXTURE_DESC& Texture)
			{ return Texture.strName == strName; });
		return Iterator == Element.Material.SourceMaterial.Textures.end() ?
			nullptr : &*Iterator;
	}

	bool Reset_AllAuthoringOverrides(
		Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		Client::EFFECT_ELEMENT_DESC Staged = Element;
		std::vector<std::string> ResourceSlots;
		std::vector<std::string> Scalars;
		std::vector<std::string> Colors;
		ResourceSlots.reserve(Staged.AuthoringOverrides.ResourceBindings.size());
		Scalars.reserve(Staged.AuthoringOverrides.Scalars.size());
		Colors.reserve(Staged.AuthoringOverrides.Colors.size());
		for (const Client::EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override :
			Staged.AuthoringOverrides.ResourceBindings)
		{
			ResourceSlots.push_back(Override.strSlotId);
		}
		for (const Client::EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Override :
			Staged.AuthoringOverrides.Scalars)
		{
			Scalars.push_back(Override.strName);
		}
		for (const Client::EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Override :
			Staged.AuthoringOverrides.Colors)
		{
			Colors.push_back(Override.strName);
		}
		for (const std::string& strSlotId : ResourceSlots)
		{
			if (!Client::CEffectDocumentCodec::Reset_AuthoringResourceOverride(
					Staged, strSlotId, strOutError))
			{
				return false;
			}
		}
		for (const std::string& strName : Scalars)
		{
			if (!Client::CEffectDocumentCodec::Reset_AuthoringScalarOverride(
					Staged, strName, strOutError))
			{
				return false;
			}
		}
		for (const std::string& strName : Colors)
		{
			if (!Client::CEffectDocumentCodec::Reset_AuthoringColorOverride(
					Staged, strName, strOutError))
			{
				return false;
			}
		}
		Element = std::move(Staged);
		strOutError.clear();
		return true;
	}

	const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC*
	Find_MaterialExecutionLane(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId)
	{
		std::string_view strLaneId;
		if (!Try_ParseMaterialExecutionLaneSlotId(strSlotId, strLaneId))
			return nullptr;
		const auto Iterator = std::find_if(
			Element.Material.Execution.TextureLanes.begin(),
			Element.Material.Execution.TextureLanes.end(),
			[strLaneId](
				const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane)
			{ return Lane.strLaneId == strLaneId; });
		return Iterator == Element.Material.Execution.TextureLanes.end() ?
			nullptr : &*Iterator;
	}

	const char* MaterialExecutionBackendLabel(
		const Client::EFFECT_MATERIAL_EXECUTION_BACKEND eBackend)
	{
		switch (eBackend)
		{
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::GENERIC:
			return "Generic";
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2:
			return "RuntimeMaterialV2";
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4:
			return "ArtistVisualV4";
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL:
			return "LocalDecal";
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1:
			return "StandardColorV1";
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::END:
		default:
			return "Invalid";
		}
	}

	const char* MaterialTextureColorSpaceLabel(
		const Client::EFFECT_TEXTURE_COLOR_SPACE eColorSpace)
	{
		switch (eColorSpace)
		{
		case Client::EFFECT_TEXTURE_COLOR_SPACE::LINEAR:
			return "Linear";
		case Client::EFFECT_TEXTURE_COLOR_SPACE::SRGB:
			return "sRGB";
		case Client::EFFECT_TEXTURE_COLOR_SPACE::END:
		default:
			return "Invalid";
		}
	}

	const char* MaterialTextureFilterLabel(
		const Client::EFFECT_MATERIAL_TEXTURE_FILTER eFilter)
	{
		switch (eFilter)
		{
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::POINT:
			return "Point";
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR:
			return "Linear";
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::ANISOTROPIC:
			return "Anisotropic";
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::END:
		default:
			return "Invalid";
		}
	}

	bool Try_ResolveArtistCoreFamily(
		const Client::EFFECT_RUNTIME_RENDERER_KIND eRenderer,
		Client::EFFECT_GPU_RENDER_FAMILY& eOutFamily)
	{
		switch (eRenderer)
		{
		case Client::EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE:
			eOutFamily = Client::EFFECT_GPU_RENDER_FAMILY::MESH;
			return true;
		case Client::EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE:
			eOutFamily = Client::EFFECT_GPU_RENDER_FAMILY::SPRITE;
			return true;
		case Client::EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE:
			eOutFamily = Client::EFFECT_GPU_RENDER_FAMILY::DECAL;
			return true;
		case Client::EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON:
			eOutFamily = Client::EFFECT_GPU_RENDER_FAMILY::RIBBON;
			return true;
		case Client::EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST:
		case Client::EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE:
		default:
			eOutFamily = Client::EFFECT_GPU_RENDER_FAMILY::END;
			return false;
		}
	}

	bool Try_NarrowRuntimeFloat3(
		const std::array<double, 3u>& Source,
		float3_t& OutValue)
	{
		constexpr double MAX_FLOAT =
			static_cast<double>((std::numeric_limits<f32_t>::max)());
		for (const double Value : Source)
		{
			if (!std::isfinite(Value) || std::abs(Value) > MAX_FLOAT)
				return false;
		}
		OutValue = {
			static_cast<f32_t>(Source[0]),
			static_cast<f32_t>(Source[1]),
			static_cast<f32_t>(Source[2])
		};
		return true;
	}

	const char* ArtistCoreFamilyLabel(
		const Client::EFFECT_GPU_RENDER_FAMILY eFamily)
	{
		switch (eFamily)
		{
		case Client::EFFECT_GPU_RENDER_FAMILY::MESH:
			return "MeshParticle";
		case Client::EFFECT_GPU_RENDER_FAMILY::SPRITE:
			return "SpriteParticle";
		case Client::EFFECT_GPU_RENDER_FAMILY::DECAL:
			return "LocalDecal";
		case Client::EFFECT_GPU_RENDER_FAMILY::RIBBON:
			return "CascadeRibbon";
		case Client::EFFECT_GPU_RENDER_FAMILY::END:
		default:
			return "Invalid";
		}
	}

	const char* VisualProgramFamilyLabel(
		const Client::EFFECT_VISUAL_PROGRAM_FAMILY eFamily)
	{
		switch (eFamily)
		{
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE:
			return "MeshParticle";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE:
			return "SpriteParticle";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE:
			return "LocalDecal";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON:
			return "CascadeRibbon";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL:
			return "AnimationTrail";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE:
			return "LightParticle";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::SCREEN_POST:
			return "ScreenPost";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::END:
		default:
			return "Invalid";
		}
	}

	std::string StableIdentityLeaf(const std::string_view strIdentity)
	{
		const size_t iEvent = strIdentity.find(".event_source-");
		const std::string_view Primary = std::string_view::npos == iEvent ?
			strIdentity : strIdentity.substr(0u, iEvent);
		const size_t iSeparator = Primary.find_last_of("./:\\");
		std::string Label = std::string_view::npos == iSeparator ?
			std::string(Primary) : std::string(Primary.substr(iSeparator + 1u));
		if (std::string_view::npos != iEvent)
		{
			Label += " @ ";
			Label += strIdentity.substr(iEvent + std::string_view(
				".event_source-").size());
		}
		return Label.empty() ? std::string("unnamed") : Label;
	}

	std::string ResourceAssetLeaf(const std::string_view strAssetId)
	{
		if (strAssetId.empty())
			return {};
		return std::filesystem::path(strAssetId).filename().string();
	}

	std::string PrimaryVisualResourceLeaf(
		const std::vector<Client::EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW>&
			Resources)
	{
		static constexpr std::array<std::string_view, 9u> SLOT_PRIORITY{
			"meshModel", "base", "emissive", "mask", "noise", "dissolve",
			"base2", "mask2", "noise2" };
		for (const std::string_view strSlot : SLOT_PRIORITY)
		{
			const auto Found = std::find_if(Resources.begin(), Resources.end(),
				[strSlot](const auto& Resource)
				{ return Resource.strSlotId == strSlot && !Resource.strAssetId.empty(); });
			if (Found != Resources.end())
				return ResourceAssetLeaf(Found->strAssetId);
		}
		const auto Found = std::find_if(Resources.begin(), Resources.end(),
			[](const auto& Resource) { return !Resource.strAssetId.empty(); });
		return Found == Resources.end() ? std::string("unbound") :
			ResourceAssetLeaf(Found->strAssetId);
	}

	std::string PrimaryAuthoringResourceLeaf(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		static constexpr std::array<std::string_view, 9u> SLOT_PRIORITY{
			"meshModel", "base", "emissive", "mask", "noise", "dissolve",
			"base2", "mask2", "noise2" };
		for (const std::string_view strSlot : SLOT_PRIORITY)
		{
			const auto Found = std::find_if(Element.ResourceBindings.begin(),
				Element.ResourceBindings.end(),
				[strSlot](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
				{ return Binding.strSlotId == strSlot && !Binding.strAssetId.empty(); });
			if (Found != Element.ResourceBindings.end())
				return ResourceAssetLeaf(Found->strAssetId);
		}
		const auto Found = std::find_if(Element.Material.SourceMaterial.Textures.begin(),
			Element.Material.SourceMaterial.Textures.end(),
			[](const Client::EFFECT_NAMED_TEXTURE_DESC& Texture)
			{ return !Texture.strAssetId.empty(); });
		if (Found != Element.Material.SourceMaterial.Textures.end())
			return ResourceAssetLeaf(Found->strAssetId);
		const auto* pQProgram = Client::Find_DimensionMasterQProgram(
			Element.Material.SourceMaterial.strRuntimeShaderProfileId);
		if (nullptr != pQProgram && 47u == pQProgram->iProfileIndex &&
			Element.Material.SourceMaterial.bEnabled &&
			Element.ResourceBindings.empty() &&
			Client::Has_DimensionMasterQMaterialContract(Element))
			return "Procedural Glow (no texture)";
		return "unbound";
	}

	std::string FriendlyModelCueLabel(
		const size_t iOrdinal,
		const Client::EFFECT_MODEL_CUE_DESC& Cue)
	{
		std::ostringstream Label;
		Label << "Summon ";
		if (iOrdinal < 10u)
			Label << '0';
		Label << iOrdinal << " | ";
		const std::string ResourceLeaf = ResourceAssetLeaf(Cue.strModelAssetId);
		Label << (ResourceLeaf.empty() ? std::string("unbound") : ResourceLeaf);
		return Label.str();
	}

	std::string FriendlyDocumentLabel(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const std::string_view strFallback)
	{
		if (!strFallback.empty() &&
			(Document.strEffectAssetId == ARTIST_F_UNIFIED_EFFECT_ASSET_ID ||
			 Document.strEffectAssetId ==
				DIMENSION_MASTER_T_UNIFIED_EFFECT_ASSET_ID))
		{
			return std::string(strFallback);
		}
		if (!Document.strDisplayName.empty())
			return Document.strDisplayName;
		return strFallback.empty() ? std::string("Effect") :
			std::string(strFallback);
	}

	std::string StableUnifiedElementId(
		const Client::EFFECT_GPU_RENDER_FAMILY eFamily,
		const std::string_view strSourceIdentity)
	{
		const std::string Digest =
			Client::CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				std::string(ArtistCoreFamilyLabel(eFamily)) + "\n" +
				std::string(strSourceIdentity));
		std::string Prefix;
		switch (eFamily)
		{
		case Client::EFFECT_GPU_RENDER_FAMILY::MESH: Prefix = "mesh"; break;
		case Client::EFFECT_GPU_RENDER_FAMILY::SPRITE: Prefix = "sprite"; break;
		case Client::EFFECT_GPU_RENDER_FAMILY::DECAL: Prefix = "decal"; break;
		case Client::EFFECT_GPU_RENDER_FAMILY::RIBBON: Prefix = "ribbon"; break;
		case Client::EFFECT_GPU_RENDER_FAMILY::END:
		default: return {};
		}
		return Prefix + "." + Digest.substr(0u, 16u);
	}

	std::string VisualProgramResourceSlotSummary(
		const std::vector<Client::EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW>&
			Resources)
	{
		std::string Summary;
		for (const Client::EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW& Resource :
			Resources)
		{
			if (Resource.strSlotId.empty())
				continue;
			if (!Summary.empty())
				Summary += ", ";
			Summary += Resource.strSlotId;
			if (!Resource.strAssetId.empty())
			{
				Summary += "=";
				Summary += std::filesystem::path(
					Resource.strAssetId).filename().string();
			}
		}
		return Summary.empty() ? std::string("no bound slots") : Summary;
	}

	std::string AuthoringElementResourceSlotSummary(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		std::string Summary;
		std::set<std::string, std::less<>> SeenSlots;
		const auto Append = [&Summary, &SeenSlots](
			const std::string& strSlotId,
			const std::string& strAssetId)
		{
			if (strSlotId.empty() || strAssetId.empty() ||
				!SeenSlots.insert(strSlotId).second)
			{
				return;
			}
			if (!Summary.empty())
				Summary += ", ";
			Summary += strSlotId + "=" +
				std::filesystem::path(strAssetId).filename().string();
		};
		for (const Client::EFFECT_RESOURCE_BINDING_DESC& Binding :
			Element.ResourceBindings)
		{
			Append(Binding.strSlotId, Binding.strAssetId);
		}
		for (const Client::EFFECT_NAMED_TEXTURE_DESC& Texture :
			Element.Material.SourceMaterial.Textures)
		{
			Append(Texture.strName, Texture.strAssetId);
		}
		for (const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane :
			Element.Material.Execution.TextureLanes)
		{
			Append(Lane.strRole.empty() ? Lane.strLaneId : Lane.strRole,
				Lane.strAssetId);
		}
		return Summary.empty() ? std::string("no bound slots") : Summary;
	}

	std::string VisualProgramElementRowLabel(
		const Client::EFFECT_VISUAL_PROGRAM_FAMILY eFamily,
		const size_t iElementOrdinal,
		const std::string& strSourceRecordId,
		const std::vector<Client::EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW>&
			Resources)
	{
		(void)strSourceRecordId;
		return std::string(VisualProgramFamilyLabel(eFamily)) + " " +
			(iElementOrdinal < 10u ? "0" : "") +
			std::to_string(iElementOrdinal) + " | " +
			PrimaryVisualResourceLeaf(Resources);
	}

	void Upsert_OccurrenceTuningEntry(
		Client::EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		const std::string& strOccurrenceId,
		const std::string& strSourceRowSha256,
		const Client::EFFECT_OCCURRENCE_LOCAL_TRANSFORM& Transform)
	{
		auto Found = std::lower_bound(
			Document.Entries.begin(), Document.Entries.end(), strOccurrenceId,
			[](const Client::EFFECT_OCCURRENCE_TUNING_ENTRY& Entry,
				const std::string& Id)
			{
				return Entry.strOccurrenceId < Id;
			});
		Client::EFFECT_OCCURRENCE_TUNING_ENTRY Staged;
		Staged.strOccurrenceId = strOccurrenceId;
		Staged.strSourceOccurrenceRowSha256 = strSourceRowSha256;
		Staged.strProvenance = "PROJECT_TUNED";
		Staged.EffectiveLocalTransform = Transform;
		if (Found != Document.Entries.end() &&
			Found->strOccurrenceId == strOccurrenceId)
		{
			*Found = std::move(Staged);
		}
		else
		{
			Document.Entries.insert(Found, std::move(Staged));
		}
	}

	void Remove_OccurrenceTuningEntry(
		Client::EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		const std::string& strOccurrenceId)
	{
		const auto Found = std::lower_bound(
			Document.Entries.begin(), Document.Entries.end(), strOccurrenceId,
			[](const Client::EFFECT_OCCURRENCE_TUNING_ENTRY& Entry,
				const std::string& Id)
			{
				return Entry.strOccurrenceId < Id;
			});
		if (Found != Document.Entries.end() &&
			Found->strOccurrenceId == strOccurrenceId)
		{
			Document.Entries.erase(Found);
		}
	}

	void Upsert_SourceAuthoringOverlayEntry(
		Client::EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
		const std::string& strOccurrenceId,
		const std::string& strSourceRowSha256,
		const std::string& strSourceElementId,
		const Client::EFFECT_OCCURRENCE_LOCAL_TRANSFORM& Transform)
	{
		auto Found = std::lower_bound(
			Document.Entries.begin(), Document.Entries.end(), strOccurrenceId,
			[](const Client::EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY& Entry,
				const std::string& Id)
			{
				return Entry.strOccurrenceId < Id;
			});
		Client::EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY Staged;
		Staged.strOccurrenceId = strOccurrenceId;
		Staged.strSourceOccurrenceRowSha256 = strSourceRowSha256;
		Staged.strSourceElementId = strSourceElementId;
		Staged.strProvenance = "PROJECT_TUNED";
		Staged.EffectiveLocalTransform = Transform;
		if (Found != Document.Entries.end() &&
			Found->strOccurrenceId == strOccurrenceId)
		{
			*Found = std::move(Staged);
		}
		else
		{
			Document.Entries.insert(Found, std::move(Staged));
		}
	}

	void Remove_SourceAuthoringOverlayEntry(
		Client::EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
		const std::string& strOccurrenceId)
	{
		const auto Found = std::lower_bound(
			Document.Entries.begin(), Document.Entries.end(), strOccurrenceId,
			[](const Client::EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY& Entry,
				const std::string& Id)
			{
				return Entry.strOccurrenceId < Id;
			});
		if (Found != Document.Entries.end() &&
			Found->strOccurrenceId == strOccurrenceId)
		{
			Document.Entries.erase(Found);
		}
	}

	bool Start_OwnedToolProcess(
		std::wstring Command,
		const std::filesystem::path& WorkingDirectory,
		const std::string_view strLabel,
		HANDLE& hOutProcess,
		std::string& strOutStatus)
	{
		hOutProcess = nullptr;
		if (Command.empty() || WorkingDirectory.empty())
		{
			strOutStatus = std::string(strLabel) + " command is invalid.";
			return false;
		}
		std::vector<wchar_t> MutableCommand(Command.begin(), Command.end());
		MutableCommand.push_back(L'\0');
		STARTUPINFOW Startup{};
		Startup.cb = sizeof(Startup);
		PROCESS_INFORMATION Process{};
		if (!CreateProcessW(
			nullptr, MutableCommand.data(), nullptr, nullptr, FALSE,
			CREATE_NO_WINDOW, nullptr, WorkingDirectory.c_str(),
			&Startup, &Process))
		{
			strOutStatus = "Could not start " + std::string(strLabel) + ".";
			return false;
		}
		CloseHandle(Process.hThread);
		hOutProcess = Process.hProcess;
		strOutStatus = std::string(strLabel) + " started.";
		return true;
	}

    bool Run_OwnedToolProcess(
        std::wstring Command,
        const std::filesystem::path& WorkingDirectory,
        const DWORD iTimeoutMilliseconds,
        const std::string_view strLabel,
        std::string& strOutStatus)
    {
        if (Command.empty() || WorkingDirectory.empty())
        {
            strOutStatus = std::string(strLabel) + " command is invalid.";
            return false;
        }
        std::vector<wchar_t> MutableCommand(Command.begin(), Command.end());
        MutableCommand.push_back(L'\0');
        STARTUPINFOW Startup{};
        Startup.cb = sizeof(Startup);
        PROCESS_INFORMATION Process{};
        if (!CreateProcessW(
            nullptr, MutableCommand.data(), nullptr, nullptr, FALSE,
            CREATE_NO_WINDOW, nullptr, WorkingDirectory.c_str(),
            &Startup, &Process))
        {
            strOutStatus = "Could not start " + std::string(strLabel) + ".";
            return false;
        }
        CloseHandle(Process.hThread);
        const DWORD iWait = WaitForSingleObject(
            Process.hProcess, iTimeoutMilliseconds);
        if (WAIT_TIMEOUT == iWait)
        {
            TerminateProcess(Process.hProcess, 124u);
            WaitForSingleObject(Process.hProcess, 5000u);
            CloseHandle(Process.hProcess);
            strOutStatus = std::string(strLabel) +
                " timed out; its owned process was terminated.";
            return false;
        }
        DWORD iExitCode = 1u;
        const bool bSucceeded = WAIT_OBJECT_0 == iWait &&
            GetExitCodeProcess(Process.hProcess, &iExitCode) &&
            0u == iExitCode;
        CloseHandle(Process.hProcess);
        strOutStatus = bSucceeded ?
            std::string(strLabel) + " succeeded." :
            std::string(strLabel) + " failed with exit code " +
                std::to_string(iExitCode) + ".";
        return bSucceeded;
    }

    const char* Kind_Label(const Client::EFFECT_ELEMENT_KIND eKind)
    {
        switch (eKind)
        {
        case Client::EFFECT_ELEMENT_KIND::MESH: return "Standalone Mesh";
        case Client::EFFECT_ELEMENT_KIND::SPRITE: return "Standalone Sprite";
        case Client::EFFECT_ELEMENT_KIND::PARTICLE: return "Cascade Particle";
        case Client::EFFECT_ELEMENT_KIND::DECAL: return "Decal";
        case Client::EFFECT_ELEMENT_KIND::TRAIL: return "Trail";
        case Client::EFFECT_ELEMENT_KIND::LIGHT: return "Light";
        case Client::EFFECT_ELEMENT_KIND::SCREEN_POST: return "Screen Post";
        case Client::EFFECT_ELEMENT_KIND::END:
        default: return "Invalid";
        }
    }

	const char* AuthoringFamily_Label(
		const Client::EFFECT_AUTHORING_FAMILY eFamily)
	{
		return Client::Get_EffectToolAuthoringFamilyLabel(eFamily);
	}

	Client::EFFECT_ELEMENT_KIND AuthoringFamily_Kind(
		const Client::EFFECT_AUTHORING_FAMILY eFamily)
	{
		switch (eFamily)
		{
		case Client::EFFECT_AUTHORING_FAMILY::MESH:
			return Client::EFFECT_ELEMENT_KIND::MESH;
		case Client::EFFECT_AUTHORING_FAMILY::SPRITE:
			return Client::EFFECT_ELEMENT_KIND::SPRITE;
		case Client::EFFECT_AUTHORING_FAMILY::MESH_PARTICLE:
		case Client::EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE:
			return Client::EFFECT_ELEMENT_KIND::PARTICLE;
		case Client::EFFECT_AUTHORING_FAMILY::LOCAL_DECAL:
			return Client::EFFECT_ELEMENT_KIND::DECAL;
		case Client::EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON:
			return Client::EFFECT_ELEMENT_KIND::TRAIL;
		case Client::EFFECT_AUTHORING_FAMILY::PRESENTATION_LIGHT:
			return Client::EFFECT_ELEMENT_KIND::LIGHT;
		case Client::EFFECT_AUTHORING_FAMILY::PRESENTATION_SCREEN_POST:
			return Client::EFFECT_ELEMENT_KIND::SCREEN_POST;
		case Client::EFFECT_AUTHORING_FAMILY::END:
		default: return Client::EFFECT_ELEMENT_KIND::END;
		}
	}

	bool_t AuthoringFamily_CanCreate(
		const Client::EFFECT_AUTHORING_FAMILY eFamily)
	{
		return eFamily >= Client::EFFECT_AUTHORING_FAMILY::MESH &&
			eFamily <= Client::EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON;
	}

	bool_t AuthoringFamily_RequiresMesh(
		const Client::EFFECT_AUTHORING_FAMILY eFamily)
	{
		return eFamily == Client::EFFECT_AUTHORING_FAMILY::MESH ||
			eFamily == Client::EFFECT_AUTHORING_FAMILY::MESH_PARTICLE;
	}

	const char* AuthoringFamily_ElementPrefix(
		const Client::EFFECT_AUTHORING_FAMILY eFamily)
	{
		switch (eFamily)
		{
		case Client::EFFECT_AUTHORING_FAMILY::MESH: return "mesh";
		case Client::EFFECT_AUTHORING_FAMILY::SPRITE: return "sprite";
		case Client::EFFECT_AUTHORING_FAMILY::MESH_PARTICLE:
			return "mesh_particle";
		case Client::EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE:
			return "sprite_particle";
		case Client::EFFECT_AUTHORING_FAMILY::LOCAL_DECAL:
			return "local_decal";
		case Client::EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON:
			return "trail_ribbon";
		case Client::EFFECT_AUTHORING_FAMILY::PRESENTATION_LIGHT:
			return "presentation_light";
		case Client::EFFECT_AUTHORING_FAMILY::PRESENTATION_SCREEN_POST:
			return "presentation_screen_post";
		case Client::EFFECT_AUTHORING_FAMILY::END:
		default: return "element";
		}
	}

	Client::EFFECT_AUTHORING_FAMILY Resolve_AuthoringFamily(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Client::Resolve_EffectToolAuthoringFamily(Element);
	}

	bool_t Can_EditElementFollowAttachment(const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Element.eKind == Client::EFFECT_ELEMENT_KIND::TRAIL ||
			(Resolve_AuthoringFamily(Element) == Client::EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE &&
			 !Element.SourceRecipe.bEnabled && Element.RuntimeCarrier.Is_Empty());
	}

	const char_t* ScreenPostProfile_Label(
		const Client::EFFECT_SCREEN_POST_PROFILE eProfile)
	{
		switch (eProfile)
		{
		case Client::EFFECT_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED_V1:
			return "RGB Noise (Reconstructed v1)";
		case Client::EFFECT_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED_V1:
			return "Zoom Blur (Reconstructed v1)";
		case Client::EFFECT_SCREEN_POST_PROFILE::FILM_NOISE_RECONSTRUCTED_V1:
			return "Film Noise (Reconstructed v1)";
		case Client::EFFECT_SCREEN_POST_PROFILE::MOTION_BLUR_RECONSTRUCTED_V1:
			return "Motion Blur (Native)";
		case Client::EFFECT_SCREEN_POST_PROFILE::END:
		default:
			return "Unresolved";
		}
	}

	bool_t HasAuthoringApproximate(
		const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		return std::any_of(Document.Elements.begin(), Document.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Client::Get_EffectAuthoringFidelity(
					Element.Material.Execution) ==
					Client::EFFECT_AUTHORING_FIDELITY::APPROXIMATE;
			});
	}

	std::string FriendlyAuthoringElementLabel(
		const Client::EFFECT_AUTHORING_FAMILY eFamily,
		const size_t iOrdinal,
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		std::ostringstream Label;
		Label << AuthoringFamily_Label(eFamily) << ' ';
		if (iOrdinal < 10u)
			Label << '0';
		Label << iOrdinal << " | " << PrimaryAuthoringResourceLeaf(Element) <<
			" | t+" << std::fixed << std::setprecision(3) <<
			Element.Detail.Timing.fStartDelaySeconds << 's';
		if (Element.strSourceNode.starts_with("valtan.source."))
			Label << " [SOURCE]";
		else if (Element.strSourceNode.starts_with("project-authored:"))
			Label << " [PROJECT]";
		const Client::EFFECT_AUTHORING_FIDELITY eFidelity =
			Client::Get_EffectAuthoringFidelity(Element.Material.Execution);
		if (eFidelity == Client::EFFECT_AUTHORING_FIDELITY::APPROXIMATE)
			Label << " [APPROXIMATE]";
		else if (eFidelity ==
			Client::EFFECT_AUTHORING_FIDELITY::PROJECT_TUNED_APPROX)
			Label << " [PROJECT_TUNED_APPROX]";
		return Label.str();
	}

	bool_t AuthoringFamily_AllowsSlot(
		const Client::EFFECT_AUTHORING_FAMILY eFamily,
		const std::string_view strSlotId)
	{
		return strSlotId != Client::EFFECT_MESH_SHAPE_SLOT_ID ||
			AuthoringFamily_RequiresMesh(eFamily);
	}

    const char* Slot_Label(const Client::EFFECT_RESOURCE_SLOT eSlot)
    {
        switch (eSlot)
        {
        case Client::EFFECT_RESOURCE_SLOT::MESH_MODEL: return "Mesh";
        case Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE: return "Base";
        case Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE: return "Noise";
        case Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE: return "Mask";
        case Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE: return "Emissive";
        case Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE: return "Dissolve";
        case Client::EFFECT_RESOURCE_SLOT::BASE2_TEXTURE: return "Base 2";
        case Client::EFFECT_RESOURCE_SLOT::MASK2_TEXTURE: return "Mask 2";
        case Client::EFFECT_RESOURCE_SLOT::NOISE2_TEXTURE: return "Noise 2";
        case Client::EFFECT_RESOURCE_SLOT::END:
        default: return "Invalid";
        }
    }

    const char* SourceMaterialStatus_Label(
        const Client::EFFECT_SOURCE_MATERIAL_STATUS eStatus)
    {
        using Client::EFFECT_SOURCE_MATERIAL_STATUS;
        switch (eStatus)
        {
        case EFFECT_SOURCE_MATERIAL_STATUS::SOURCE_EXACT:
            return "SOURCE_EXACT";
        case EFFECT_SOURCE_MATERIAL_STATUS::RUNTIME_EXACT:
            return "RUNTIME_EXACT";
        case EFFECT_SOURCE_MATERIAL_STATUS::RECONSTRUCTED_PROFILE:
            return "RECONSTRUCTED_PROFILE";
        case EFFECT_SOURCE_MATERIAL_STATUS::UNSUPPORTED:
            return "UNSUPPORTED";
        case EFFECT_SOURCE_MATERIAL_STATUS::MISSING_RESOURCE:
            return "MISSING_RESOURCE";
        case EFFECT_SOURCE_MATERIAL_STATUS::END:
        default:
            return "INVALID";
        }
    }

    const char* Class_Label(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass)
    {
        using LostArk::Shared::CHARACTER_CLASS_ID;
        switch (eClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER: return "Lance Master";
        case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
        case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
        case CHARACTER_CLASS_ID::ARTIST: return "Artist";
        case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "Dimension Master";
        case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
        case CHARACTER_CLASS_ID::END:
        default: return "Invalid";
        }
    }

    const char* Resource_DomainId(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass)
    {
        using LostArk::Shared::CHARACTER_CLASS_ID;
        switch (eClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER: return "LanceMaster";
        case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
        case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
        case CHARACTER_CLASS_ID::ARTIST: return "Artist";
        case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DimensionMaster";
        case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
        case CHARACTER_CLASS_ID::END:
        default: return nullptr;
        }
    }

    bool Is_KoukuEffectAssetId(const std::string_view strEffectAssetId)
    {
        return strEffectAssetId.starts_with("effect.kouku.");
    }

    bool Is_WorldEffectAssetId(const std::string_view strEffectAssetId)
    {
        return strEffectAssetId.starts_with("effect.world.");
    }

    bool Is_SceneAnchoredEffectAssetId(const std::string_view strEffectAssetId)
    {
        return Is_KoukuEffectAssetId(strEffectAssetId) || Is_WorldEffectAssetId(strEffectAssetId);
    }

    std::string EffectAsset_DomainId(const std::string& strEffectAssetId)
    {
        constexpr std::pair<std::string_view, std::string_view> Domains[] =
        {
            { "effect.lancemaster.", "LanceMaster" },
            { "effect.gunslinger.", "Gunslinger" },
            { "effect.slayer.", "Slayer" },
            { "effect.artist.", "Artist" },
            { "effect.dimensionmaster.", "DimensionMaster" },
            { "effect.warlord.", "Warlord" },
            { "effect.valtan.", "Valtan" },
            { "effect.kouku.", "KoukuSaydon" },
            { "effect.world.", "World" }
        };
        for (const auto& [Prefix, DomainId] : Domains)
        {
            if (strEffectAssetId.starts_with(Prefix))
                return std::string(DomainId);
        }
        return "Uncategorized";
    }

    std::string First_PathComponent(const std::filesystem::path& Relative)
    {
        const auto Iterator = Relative.begin();
        if (Iterator == Relative.end())
            return {};
        return Iterator->generic_string();
    }

    bool Try_DeriveEffectAssetIdFromFilename(
        const std::filesystem::path& Path,
        const Client::EFFECT_DOCUMENT_SOURCE eSource,
        std::string& OutAssetId)
    {
        constexpr std::string_view EffectSuffix = ".effect.json";
        constexpr std::string_view ImportedSuffix = ".imported";
        const std::string Name = Path.filename().string();
        if (!Name.ends_with(EffectSuffix))
            return false;

        std::string AssetId = Name.substr(
            0u, Name.size() - EffectSuffix.size());
        if (Client::EFFECT_DOCUMENT_SOURCE::IMPORTED == eSource &&
            AssetId.ends_with(ImportedSuffix))
        {
            AssetId.resize(AssetId.size() - ImportedSuffix.size());
        }
        if (AssetId.empty() || AssetId.size() > 128u ||
            !std::all_of(AssetId.begin(), AssetId.end(),
                [](const char Character)
                {
                    const unsigned char Value =
                        static_cast<unsigned char>(Character);
                    return 0 != std::isalnum(Value) || Character == '_' ||
                        Character == '-' || Character == '.';
                }))
        {
            return false;
        }
        OutAssetId = std::move(AssetId);
        return true;
    }

    bool_t Ensure_PlayerSkillCatalog(std::string& OutStatus)
    {
        if (!Client::CPlayerSkillCatalog::Get_Skills().empty())
        {
            OutStatus = "Using the loaded player skill catalog.";
            return true;
        }
        return Client::CPlayerSkillCatalog::Load(OutStatus);
    }

    const char* Animation_AssetName(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass)
    {
        using LostArk::Shared::CHARACTER_CLASS_ID;
        switch (eClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER: return "LanceMaster";
        case CHARACTER_CLASS_ID::GUNSLINGER: return "GunSlinger";
        case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
        case CHARACTER_CLASS_ID::ARTIST: return "Artist";
        case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DimensionMaster";
        case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
        case CHARACTER_CLASS_ID::END:
        default: return nullptr;
        }
    }

    std::vector<std::string> Collect_AnimationClipNames(
        const std::shared_ptr<Engine::CModel>& pModel)
    {
        std::vector<std::string> Clips;
        if (nullptr == pModel)
            return Clips;
        Clips.reserve(pModel->Get_NumAnimations());
        for (uint32_t iAnimation = 0u;
            iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
        {
            const char* pName = pModel->Get_AnimationName(iAnimation);
            if (nullptr != pName)
                Clips.emplace_back(pName);
        }
        return Clips;
    }

    bool Read_TextFile(
        const std::filesystem::path& Path,
        std::string& OutText,
        std::string& OutStatus)
    {
        OutText.clear();
        std::ifstream Input(Path, std::ios::binary);
        if (Path.empty() || !Input)
        {
            OutStatus = "Could not open presentation document: " +
                Path.string();
            return false;
        }
        std::ostringstream Buffer;
        Buffer << Input.rdbuf();
        if (!Input.good() && !Input.eof())
        {
            OutStatus = "Could not read presentation document: " +
                Path.string();
            return false;
        }
        OutText = Buffer.str();
        return true;
    }

	bool Validate_RegistryBoundAuditionSourceFreshness(
		const std::filesystem::path& SourceDocumentPath,
		const std::string_view strExpectedRawSha256,
		std::string& strOutStatus)
	{
		std::error_code FileError;
		const std::uintmax_t iFileSize = std::filesystem::file_size(
			SourceDocumentPath, FileError);
		if (SourceDocumentPath.empty() || strExpectedRawSha256.size() != 64u ||
			FileError || 0u == iFileSize ||
			iFileSize > 64u * 1024u * 1024u)
		{
			strOutStatus =
				"Registry-bound audition source is missing, empty, or exceeds 64 MiB.";
			return false;
		}
		std::string SourceBytes;
		if (!Read_TextFile(SourceDocumentPath, SourceBytes, strOutStatus))
			return false;
		const std::string ActualSha256 =
			Client::CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(SourceBytes);
		if (ActualSha256 != strExpectedRawSha256)
		{
			strOutStatus =
				"Registry-bound audition source changed after its catalog pin. Refresh the index only after sourceDocumentRawSha256 is deliberately updated.";
			return false;
		}
		strOutStatus.clear();
		return true;
	}

    std::vector<Client::ANIMATION_SKILL_CLIP> Flatten_BindingClips(
        const Client::ANIMATION_SKILL_BINDING& Binding)
    {
        std::vector<Client::ANIMATION_SKILL_CLIP> Clips;
        for (const Client::ANIMATION_SKILL_STAGE& Stage : Binding.Stages)
        {
            Clips.insert(Clips.end(), Stage.Clips.begin(), Stage.Clips.end());
        }
        return Clips;
    }

    bool Try_ParseEffectDiagnosticRow(
        const std::string_view Line,
        std::string& OutClip,
        bool_t& OutImported,
        bool_t& OutEmptyPayload)
    {
        OutClip.clear();
        OutImported = false;
        OutEmptyPayload = false;
        if (Line.empty() || Line.front() != '"')
            return false;
        const size_t iClipEnd = Line.find('"', 1u);
        if (std::string_view::npos == iClipEnd ||
            std::string_view::npos == Line.find(" EFFECT ", iClipEnd) ||
            std::string_view::npos != Line.find(" effectref=asset "))
        {
            return false;
        }
        const size_t iPayload = Line.find(" payload=\"", iClipEnd);
        if (std::string_view::npos == iPayload)
            return false;
        const size_t iValueBegin = iPayload + 10u;
        const size_t iValueEnd = Line.find('"', iValueBegin);
        if (std::string_view::npos == iValueEnd)
            return false;
        OutClip.assign(Line.substr(1u, iClipEnd - 1u));
        OutImported = std::string_view::npos != Line.find(" src=orig");
        OutEmptyPayload = iValueBegin == iValueEnd;
        return true;
    }

	bool_t Try_ExtractPlanarYawDegrees(
		const float4x4_t& Root,
		f32_t& fOutYawDegrees)
	{
		const f32_t fForwardLength = std::sqrt(
			Root._31 * Root._31 + Root._33 * Root._33);
		if (!std::isfinite(Root._31) || !std::isfinite(Root._33) ||
			!std::isfinite(fForwardLength) || fForwardLength <= 1.0e-6f)
		{
			return false;
		}
		const f32_t fYawDegrees = XMConvertToDegrees(
			std::atan2(Root._31, Root._33));
		if (!std::isfinite(fYawDegrees))
			return false;
		fOutYawDegrees = fYawDegrees;
		return true;
	}

	const char* Source_Label(const Client::EFFECT_DOCUMENT_SOURCE eSource)
	{
        switch (eSource)
        {
        case Client::EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT: return "New";
        case Client::EFFECT_DOCUMENT_SOURCE::AUTHORED: return "Authored";
        case Client::EFFECT_DOCUMENT_SOURCE::IMPORTED: return "Imported";
        case Client::EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE:
            return "Imported Draft";
		case Client::EFFECT_DOCUMENT_SOURCE::MIGRATION_REFERENCE:
			return "Migration Reference";
		case Client::EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY:
			return "Assembly";
		case Client::EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT:
			return "WFX Component";
		case Client::EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM:
			return "Visual Program Copy";
        case Client::EFFECT_DOCUMENT_SOURCE::END:
        default: return "Invalid";
		}
	}

	std::string Unified_CandidateAssetId(const std::string_view AssetId)
	{
		constexpr std::string_view LegacyToken = ".authored-baseline";
		constexpr std::string_view UnifiedSuffix = ".unified";
		std::string Candidate(AssetId);
		if (const size_t iLegacyToken = Candidate.find(LegacyToken);
			iLegacyToken != std::string::npos)
		{
			Candidate.erase(iLegacyToken, LegacyToken.size());
		}
		if (!Candidate.ends_with(UnifiedSuffix))
			Candidate += UnifiedSuffix;
		return Candidate;
	}

	const char* Profile_Label(const Client::EFFECT_RENDER_PROFILE eProfile)
    {
        switch (eProfile)
        {
        case Client::EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE:
            return "Opaque / Back / Depth Write";
        case Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
            return "Alpha / Two Sided / Depth Read";
        case Client::EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
            return "Additive / Two Sided / Depth Read";
        case Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ:
            return "Alpha / One Sided / Depth Read";
        case Client::EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ:
            return "Additive / One Sided / Depth Read";
        case Client::EFFECT_RENDER_PROFILE::END:
        default: return "Invalid";
        }
    }

    bool Contains_NoCase(
        const std::string& Value,
        const std::string_view Filter)
    {
        if (Filter.empty())
            return true;
        return Value.end() != std::search(
            Value.begin(), Value.end(), Filter.begin(), Filter.end(),
            [](const char Left, const char Right)
            {
                return std::tolower(static_cast<unsigned char>(Left)) ==
                    std::tolower(static_cast<unsigned char>(Right));
            });
    }

    std::string Build_ValtanV0EffectAssetId(const std::string_view ClipName)
    {
        std::string Suffix;
        Suffix.reserve((std::min)(ClipName.size(), size_t{ 80u }));
        bool_t bPreviousSeparator = false;
        for (const char_t Character : ClipName)
        {
            const unsigned char Value =
                static_cast<unsigned char>(Character);
            if (0 != std::isalnum(Value))
            {
                if (Suffix.size() >= 80u)
                    break;
                Suffix.push_back(static_cast<char_t>(std::tolower(Value)));
                bPreviousSeparator = false;
            }
            else if (!Suffix.empty() && !bPreviousSeparator)
            {
                Suffix.push_back('-');
                bPreviousSeparator = true;
            }
        }
        while (!Suffix.empty() && '-' == Suffix.back())
            Suffix.pop_back();
        if (Suffix.empty())
            Suffix = "cue";
        return "effect.valtan.user." + Suffix + ".v001";
    }

    bool Matches_MeshShapeCategory(
        const std::string& strAssetId,
        const std::string_view strCategory)
    {
        if (strCategory.empty() || "All" == strCategory)
            return true;
        const auto HasAny = [&strAssetId](
            const std::initializer_list<std::string_view> Tokens)
        {
            return std::any_of(Tokens.begin(), Tokens.end(),
                [&strAssetId](const std::string_view Token)
                {
                    return Contains_NoCase(strAssetId, Token);
                });
        };
        if ("Ring / Torus / Circle" == strCategory)
            return HasAny({ "ring", "torus", "circle" });
        if ("Slash / Trail / Plane" == strCategory)
            return HasAny({ "slash", "swing", "sword", "trail", "plane" });
        if ("Crack / Broken" == strCategory)
            return HasAny({ "crack", "broken" });
        if ("Sphere / Hemisphere" == strCategory)
            return HasAny({ "sphere", "hemisphere" });
        if ("Cylinder / Cone" == strCategory)
            return HasAny({ "cylinder", "cone" });
        if ("Helix" == strCategory)
            return HasAny({ "helix" });
        if ("Box / Cube / Square" == strCategory)
            return HasAny({ "box", "cube", "square" });
        if ("Wave / Aurora / Electric" == strCategory)
            return HasAny({ "wave", "aurora", "electric" });
        if ("Other" == strCategory)
        {
            return !HasAny({ "ring", "torus", "circle", "slash", "swing",
                "sword", "trail", "plane", "crack", "broken", "sphere",
                "hemisphere", "cylinder", "cone", "helix", "box", "cube",
                "square", "wave", "aurora", "electric" });
        }
        return true;
    }

    // Effect DDS filenames follow fx_<bucket>_<kind>_<index>[_variant]. The
    // bucket letter only mirrors the source package folder, so the kind token
    // is the only part that says what the texture is for. Tokens that also
    // appear inside unrelated words are anchored with the separator.
    bool Matches_TextureKindCategory(
        const std::string& strAssetId,
        const std::string_view strCategory)
    {
        if (strCategory.empty() || "All" == strCategory)
            return true;
        const auto HasAny = [&strAssetId](
            const std::initializer_list<std::string_view> Tokens)
        {
            return std::any_of(Tokens.begin(), Tokens.end(),
                [&strAssetId](const std::string_view Token)
                {
                    return Contains_NoCase(strAssetId, Token);
                });
        };
        if ("Base / Sprite" == strCategory)
            return HasAny({ "atypical", "glow", "shine", "star", "_hit",
                "spatter", "fragment", "stoneparts", "aura" });
        if ("Noise / Distortion" == strCategory)
            return HasAny({ "noise", "flow", "turbulence" });
        if ("Normal / Bump" == strCategory)
            return HasAny({ "normal", "_n.", "_n_" });
        if ("Decal / Ground" == strCategory)
            return HasAny({ "decal", "grid", "symbol", "sector" });
        if ("Ring / Shockwave" == strCategory)
            return HasAny({ "ring", "wave" });
        if ("Trail / Beam" == strCategory)
            return HasAny({ "trail", "_line", "auraline", "thunder",
                "electric", "electile" });
        if ("Cloud / Smoke / Fire" == strCategory)
            return HasAny({ "cloud", "smoke", "fire", "fogsheet" });
        if ("Fluid / Water" == strCategory)
            return HasAny({ "fluid", "liquid", "water", "softriver", "_ice" });
        if ("Other" == strCategory)
        {
            return !HasAny({ "atypical", "glow", "shine", "star", "_hit",
                "spatter", "fragment", "stoneparts", "aura", "noise", "flow",
                "turbulence", "normal", "_n.", "_n_", "decal", "grid",
                "symbol", "sector", "ring", "wave", "trail", "_line",
                "auraline", "thunder", "electric", "electile", "cloud",
                "smoke", "fire", "fogsheet", "fluid", "liquid", "water",
                "softriver", "_ice" });
        }
        return true;
    }

    CASCADE_RENDERER_KIND Resolve_CascadeRendererKind(
        const Client::EFFECT_ELEMENT_DESC& Element)
    {
        const bool_t bHasMeshModel = std::any_of(
            Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
            [](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
            {
                return Binding.strSlotId ==
                    Client::EFFECT_MESH_SHAPE_SLOT_ID;
            });
        if (!Element.SourceRecipe.bEnabled)
            return bHasMeshModel ? CASCADE_RENDERER_KIND::MESH :
                CASCADE_RENDERER_KIND::SPRITE;
        if (Element.SourceRecipe.strRendererShape == "mesh")
            return bHasMeshModel ? CASCADE_RENDERER_KIND::MESH :
                CASCADE_RENDERER_KIND::UNRESOLVED;
        if (Element.SourceRecipe.strRendererShape == "sprite")
            return bHasMeshModel ? CASCADE_RENDERER_KIND::UNRESOLVED :
                CASCADE_RENDERER_KIND::SPRITE;
        return CASCADE_RENDERER_KIND::UNRESOLVED;
    }

    const char* CascadeRenderer_Label(const CASCADE_RENDERER_KIND eKind)
    {
        switch (eKind)
        {
        case CASCADE_RENDERER_KIND::MESH: return "Mesh Renderer";
        case CASCADE_RENDERER_KIND::SPRITE: return "Sprite Renderer";
        case CASCADE_RENDERER_KIND::UNRESOLVED:
        default: return "Unresolved Renderer";
        }
    }

    const char* Element_RendererLabel(
        const Client::EFFECT_ELEMENT_DESC& Element)
    {
        return Client::EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind ?
            CascadeRenderer_Label(Resolve_CascadeRendererKind(Element)) :
            Kind_Label(Element.eKind);
    }

	const char* PreviewPivot_Label(
		const Client::EFFECT_PREVIEW_PIVOT_KIND eKind)
	{
		switch (eKind)
		{
		case Client::EFFECT_PREVIEW_PIVOT_KIND::WORLD: return "World";
		case Client::EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT: return "Player Root";
		case Client::EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET: return "Weapon Socket";
		case Client::EFFECT_PREVIEW_PIVOT_KIND::MODEL_BONE: return "Model Bone";
		case Client::EFFECT_PREVIEW_PIVOT_KIND::END:
		default: return "Invalid";
		}
	}

	std::string Lower_Ascii(const std::string_view Value)
	{
		std::string Result(Value);
		std::transform(Result.begin(), Result.end(), Result.begin(),
			[](const char Value)
			{
				return static_cast<char>(std::tolower(
					static_cast<unsigned char>(Value)));
			});
		return Result;
	}

	SOURCE_MODULE_UI_DESC Describe_SourceModule(
		const std::string_view strClassName)
	{
		const std::string ClassName = Lower_Ascii(strClassName);
		if (ClassName.find("velocityoverlife") != std::string::npos)
			return { "Velocity Over Life",
				"Scales or replaces particle velocity over normalized lifetime." };
		if (ClassName.find("meshrotationrate") != std::string::npos ||
			ClassName.find("rotationrate") != std::string::npos)
			return { "Rotation Speed",
				"Controls sprite or mesh angular velocity over particle life." };
		if (ClassName.find("meshrotation") != std::string::npos ||
			ClassName.find("rotation") != std::string::npos)
			return { "Initial Rotation",
				"Defines the initial sprite or mesh orientation distribution." };
		if (ClassName.find("initiallocation") != std::string::npos ||
			ClassName.find("locationdirect") != std::string::npos)
			return { "Initial Location",
				"Defines the emitter-relative starting position or direct source location." };
		if (ClassName.find("bone") != std::string::npos ||
			ClassName.find("socket") != std::string::npos)
			return { "Bone / Socket Location",
				"Spawns from the named source bones or sockets and their selection policy." };
		if (ClassName.find("locationsphere") != std::string::npos ||
			ClassName.find("locationprimitivesphere") != std::string::npos)
			return { "Sphere Surface",
				"Spawns particles from the source sphere volume or surface contract." };
		if (ClassName.find("locationcylinder") != std::string::npos ||
			ClassName.find("locationprimitivecylinder") != std::string::npos)
			return { "Cylinder Surface",
				"Spawns particles from the source cylinder volume or surface contract." };
		if (ClassName.find("locationcircle") != std::string::npos ||
			ClassName.find("locationprimitivecircle") != std::string::npos)
			return { "Circle Surface",
				"Spawns particles from the source circle radius or surface contract." };
		if (ClassName.find("vectorfield") != std::string::npos)
			return { "Vector Field",
				"Applies the referenced local vector-field force and its source parameters." };
		if (ClassName.find("cameraoffset") != std::string::npos)
			return { "Camera Offset",
				"Offsets particles along the active camera direction." };
		if (ClassName.find("subuv") != std::string::npos)
			return { "SubUV Animation",
				"Selects and blends source texture-atlas frames over particle life." };
		if (ClassName.find("dynamicparameter") != std::string::npos ||
			ClassName.find("parameterdynamic") != std::string::npos)
			return { "Dynamic Material Parameters",
				"Evaluates the four source particle-to-material parameter channels." };
		if (ClassName.find("meshmaterial") != std::string::npos)
			return { "Mesh Material Override",
				"Binds the source material overrides for mesh renderer sections." };
		if (ClassName.find("typedatamesh") != std::string::npos)
			return { "Mesh Renderer",
				"Defines the source mesh renderer asset, alignment, and material policy." };
		if (ClassName.find("axislock") != std::string::npos)
			return { "Renderer Axis Lock",
				"Constrains the renderer-facing or rotation axis using the source policy." };
		if (ClassName.find("orbit") != std::string::npos)
			return { "Orbit",
				"Applies source orbit offset, rotation, and rotation-rate distributions." };
		if (ClassName.find("vortex") != std::string::npos)
			return { "Vortex",
				"Applies the source vortex axis, strength, and radial motion." };
		if (ClassName.find("event") != std::string::npos)
			return { "Particle Event",
				"Defines source event generation or receiver behavior." };
		if (ClassName.find("acceleration") != std::string::npos)
			return { "Acceleration",
				"Adds the source acceleration distribution during particle life." };
		if (ClassName.find("velocity") != std::string::npos)
			return { "Initial Velocity",
				"Defines particle launch direction and speed distributions." };
		if (ClassName.find("lifetime") != std::string::npos)
			return { "Lifetime",
				"Defines the minimum, maximum, or curved particle lifetime." };
		if (ClassName.find("spawn") != std::string::npos)
			return { "Spawn",
				"Defines continuous spawn rate, rate scaling, or per-unit spawning." };
		if (ClassName.find("size") != std::string::npos)
			return { "Size",
				"Defines initial size or size scaling over particle life." };
		if (ClassName.find("color") != std::string::npos ||
			ClassName.find("alpha") != std::string::npos)
			return { "Color / Alpha",
				"Defines source color, alpha, and their lifetime curves." };
		if (ClassName.find("location") != std::string::npos)
			return { "Location",
				"Defines emitter-relative position offsets or source-emitter locations." };
		if (ClassName.find("required") != std::string::npos)
			return { "Emitter Contract",
				"Defines renderer alignment, local space, duration, delay, and loop policy." };
		return {};
	}

	std::string Friendly_SourcePropertyLabel(
		const std::string_view strPropertyPath)
	{
		std::string Label(strPropertyPath);
		constexpr std::string_view DistributionSuffix = ".distribution";
		if (Label.ends_with(DistributionSuffix))
			Label.erase(Label.size() - DistributionSuffix.size());
		const size_t iLastDot = Label.rfind('.');
		if (iLastDot != std::string::npos)
			Label.erase(0u, iLastDot + 1u);
		std::string Friendly;
		Friendly.reserve(Label.size() + 8u);
		for (size_t iCharacter = 0u; iCharacter < Label.size(); ++iCharacter)
		{
			const char Character = Label[iCharacter];
			if (Character == '_' || Character == '-')
			{
				if (!Friendly.empty() && Friendly.back() != ' ')
					Friendly.push_back(' ');
				continue;
			}
			if (!Friendly.empty() && iCharacter > 0u && std::isupper(
				static_cast<unsigned char>(Character)) &&
				!std::isupper(static_cast<unsigned char>(Label[iCharacter - 1u])) &&
				Friendly.back() != ' ')
			{
				Friendly.push_back(' ');
			}
			Friendly.push_back(Character);
		}
		return Friendly.empty() ? std::string(strPropertyPath) : Friendly;
	}

	Client::EFFECT_RESOURCE_FILE_KIND Resource_FileKind(
		const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
	{
		const std::string Extension = Lower_Ascii(
			std::filesystem::path(Binding.strAssetId).extension().string());
		return Extension == ".wmodel" ?
			Client::EFFECT_RESOURCE_FILE_KIND::MODEL :
			Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
	}

	void Render_SourceMaterialParameterGroups(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial,
		Client::CEffectThumbnailCache* pThumbnailCache)
	{
		const size_t iParameterCount = SourceMaterial.Textures.size() +
			SourceMaterial.Scalars.size() +
			SourceMaterial.Vectors.size() + SourceMaterial.StaticSwitches.size();
		if (!ImGui::CollapsingHeader(("Material Instance Parameters (" +
			std::to_string(iParameterCount) + ")").c_str()))
		{
			return;
		}
		ImGui::TextDisabled(
			"Read-only source names, groups, and resolved MI values.");
		ImGui::TextWrapped("Parent: %s",
			SourceMaterial.strParentMaterialPath.empty() ? "(none)" :
				SourceMaterial.strParentMaterialPath.c_str());
		ImGui::TextWrapped("Profile: %s | Runtime: %s",
			SourceMaterial.strProfileId.empty() ? "(none)" :
				SourceMaterial.strProfileId.c_str(),
			SourceMaterial.strRuntimeShaderProfileId.empty() ? "(none)" :
				SourceMaterial.strRuntimeShaderProfileId.c_str());

		std::set<std::string> Groups;
		const auto AddGroup = [&Groups](const std::string& strGroup)
		{
			Groups.insert(strGroup.empty() ? "(Ungrouped)" : strGroup);
		};
		for (const Client::EFFECT_NAMED_TEXTURE_DESC& Texture :
			SourceMaterial.Textures)
			AddGroup(Texture.strGroup);
		for (const Client::EFFECT_NAMED_FLOAT_DESC& Scalar :
			SourceMaterial.Scalars)
			AddGroup(Scalar.strGroup);
		for (const Client::EFFECT_NAMED_FLOAT4_DESC& Vector :
			SourceMaterial.Vectors)
			AddGroup(Vector.strGroup);
		for (const Client::EFFECT_NAMED_BOOL_DESC& StaticSwitch :
			SourceMaterial.StaticSwitches)
			AddGroup(StaticSwitch.strGroup);
		for (const std::string& Group : Groups)
		{
			const auto MatchesGroup = [&Group](const std::string& strGroup)
			{
				return (strGroup.empty() ? "(Ungrouped)" : strGroup) == Group;
			};
			size_t iGroupCount = 0u;
			iGroupCount += static_cast<size_t>(std::count_if(
				SourceMaterial.Textures.begin(), SourceMaterial.Textures.end(),
				[&MatchesGroup](const Client::EFFECT_NAMED_TEXTURE_DESC& Value)
				{ return MatchesGroup(Value.strGroup); }));
			iGroupCount += static_cast<size_t>(std::count_if(
				SourceMaterial.Scalars.begin(), SourceMaterial.Scalars.end(),
				[&MatchesGroup](const Client::EFFECT_NAMED_FLOAT_DESC& Value)
				{ return MatchesGroup(Value.strGroup); }));
			iGroupCount += static_cast<size_t>(std::count_if(
				SourceMaterial.Vectors.begin(), SourceMaterial.Vectors.end(),
				[&MatchesGroup](const Client::EFFECT_NAMED_FLOAT4_DESC& Value)
				{ return MatchesGroup(Value.strGroup); }));
			iGroupCount += static_cast<size_t>(std::count_if(
				SourceMaterial.StaticSwitches.begin(),
				SourceMaterial.StaticSwitches.end(),
				[&MatchesGroup](const Client::EFFECT_NAMED_BOOL_DESC& Value)
				{ return MatchesGroup(Value.strGroup); }));
			const std::string GroupLabel = Group + " (" +
				std::to_string(iGroupCount) + ")";
			if (!ImGui::TreeNode(GroupLabel.c_str()))
				continue;
			for (size_t iTexture = 0u;
				iTexture < SourceMaterial.Textures.size(); ++iTexture)
			{
				const Client::EFFECT_NAMED_TEXTURE_DESC& Texture =
					SourceMaterial.Textures[iTexture];
				if (!MatchesGroup(Texture.strGroup))
					continue;
				const auto AddressLabel = [](
					const Client::EFFECT_TEXTURE_ADDRESS_MODE eMode)
				{
					return eMode == Client::EFFECT_TEXTURE_ADDRESS_MODE::CLAMP ?
						"Clamp" : "Wrap";
				};
				const char* pColorSpace =
					Texture.eColorSpace ==
						Client::EFFECT_TEXTURE_COLOR_SPACE::SRGB ?
					"sRGB" : "Linear";
				ImGui::PushID(static_cast<int>(iTexture));
				ImGui::BeginGroup();
				if (nullptr != pThumbnailCache && !Texture.strAssetId.empty())
				{
					const Client::CEffectThumbnailCache::RESULT Thumbnail =
						pThumbnailCache->Request(Texture.strAssetId,
							Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE);
					if (nullptr != Thumbnail.pTextureView)
						ImGui::Image(Thumbnail.pTextureView, ImVec2(48.f, 48.f));
					else
					{
						ImGui::Button("DDS", ImVec2(48.f, 48.f));
						if (ImGui::IsItemHovered() && nullptr != Thumbnail.pError)
							ImGui::SetTooltip("%s", Thumbnail.pError->c_str());
					}
				}
				else
					ImGui::Button("DDS", ImVec2(48.f, 48.f));
				ImGui::SameLine();
				ImGui::BeginGroup();
				ImGui::Text("Texture | %s", Texture.strName.c_str());
				ImGui::TextWrapped("DDS: %s",
					Texture.strAssetId.empty() ? "(unresolved)" :
						Texture.strAssetId.c_str());
				if (ImGui::IsItemHovered() &&
					!Texture.strSourceObjectPath.empty())
				{
					ImGui::SetTooltip("Source: %s",
						Texture.strSourceObjectPath.c_str());
				}
				ImGui::TextDisabled("Address U/V: %s / %s | %s",
					AddressLabel(Texture.eAddressU),
					AddressLabel(Texture.eAddressV), pColorSpace);
				ImGui::TextDisabled("Sampling: %s",
					Texture.strSamplingEvidence.c_str());
				ImGui::EndGroup();
				ImGui::EndGroup();
				ImGui::PopID();
			}
			for (const Client::EFFECT_NAMED_FLOAT_DESC& Scalar :
				SourceMaterial.Scalars)
			{
				if (MatchesGroup(Scalar.strGroup))
					ImGui::BulletText("Scalar | %s = %.9g",
						Scalar.strName.c_str(), Scalar.fValue);
			}
			for (const Client::EFFECT_NAMED_FLOAT4_DESC& Vector :
				SourceMaterial.Vectors)
			{
				if (MatchesGroup(Vector.strGroup))
					ImGui::BulletText("Vector | %s = [%.6g, %.6g, %.6g, %.6g]",
						Vector.strName.c_str(), Vector.vValue.x, Vector.vValue.y,
						Vector.vValue.z, Vector.vValue.w);
			}
			for (const Client::EFFECT_NAMED_BOOL_DESC& StaticSwitch :
				SourceMaterial.StaticSwitches)
			{
				if (MatchesGroup(StaticSwitch.strGroup))
					ImGui::BulletText("Static Switch | %s = %s",
						StaticSwitch.strName.c_str(),
						StaticSwitch.bValue ? "true" : "false");
			}
			ImGui::TreePop();
		}
		if (0u == iParameterCount)
			ImGui::TextDisabled("(no named MI parameters or texture lanes captured)");
		ImGui::TextDisabled("Dynamic semantics: X=%s | Y=%s | Z=%s | W=%s",
			SourceMaterial.DynamicParameterSemantics[0].c_str(),
			SourceMaterial.DynamicParameterSemantics[1].c_str(),
			SourceMaterial.DynamicParameterSemantics[2].c_str(),
			SourceMaterial.DynamicParameterSemantics[3].c_str());
		ImGui::TextDisabled("SubUV mode: %s",
			SourceMaterial.strSubUVMode.c_str());
	}

    bool Slot_Allowed(
        const Client::EFFECT_ELEMENT_KIND eKind,
        const Client::EFFECT_RESOURCE_SLOT eSlot)
    {
		if (Client::EFFECT_ELEMENT_KIND::LIGHT == eKind ||
			Client::EFFECT_ELEMENT_KIND::SCREEN_POST == eKind)
		{
			return false;
		}
        if (Client::EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
            return Client::EFFECT_ELEMENT_KIND::MESH == eKind ||
                Client::EFFECT_ELEMENT_KIND::PARTICLE == eKind;
        return eKind < Client::EFFECT_ELEMENT_KIND::END &&
            eSlot >= Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE &&
            eSlot <= Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE;
    }

    Client::EFFECT_RESOURCE_FILE_KIND Slot_FileKind(
        const Client::EFFECT_RESOURCE_SLOT eSlot)
    {
        return Client::EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot ?
            Client::EFFECT_RESOURCE_FILE_KIND::MODEL :
            Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
    }

    std::string Default_SlotId(const Client::EFFECT_ELEMENT_KIND eKind)
    {
        return Client::EFFECT_ELEMENT_KIND::MESH == eKind ?
            std::string(Client::EFFECT_MESH_SHAPE_SLOT_ID) :
            std::string(Client::EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
    }

    const Client::EFFECT_RESOURCE_BINDING_DESC* Find_Binding(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
        const auto Iterator = std::find_if(
            Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
            [strSlotId](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
            {
                return Binding.strSlotId == strSlotId;
            });
        return Iterator == Element.ResourceBindings.end() ?
            nullptr : &*Iterator;
    }

	bool Is_ParticleMasterEmissionProgram(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Client::Resolve_EffectStrictTypedSourceProfile(
			Element.Material.strSourceMaterialPath,
			Element.Material.SourceMaterial) ==
			Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLE_MASTER_01;
	}

	bool Has_EffectiveEmissiveRadianceInput(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		if (Is_ParticleMasterEmissionProgram(Element))
			return Client::Has_EffectParticleMasterNamedTextureContract(
				Element.Material.SourceMaterial);
		const Client::EFFECT_RESOURCE_BINDING_DESC* pEmissive =
			Find_Binding(Element, "emissive");
		return nullptr != pEmissive && !pEmissive->strAssetId.empty();
	}

	bool Is_SourceDecalBaseAdmissionCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Client::EFFECT_ELEMENT_KIND::DECAL == Element.eKind &&
			Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.strRendererShape == "decal" &&
			Element.Material.strTemplateId ==
				Client::EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			!Element.Material.SourceMaterial.bEnabled &&
			!Element.Material.Execution.bEnabled;
	}

	bool Has_BaseTextureBinding(const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const Client::EFFECT_RESOURCE_BINDING_DESC* pBase = Find_Binding(
			Element, Client::EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
		return nullptr != pBase && !pBase->strAssetId.empty();
	}

	bool Is_MissingBaseSourceDecal(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Is_SourceDecalBaseAdmissionCarrier(Element) &&
			Element.Material.Execution.bFailClosed &&
			!Has_BaseTextureBinding(Element);
	}

	bool Is_BaseTextureSlot(const std::string_view strSlotId)
	{
		return strSlotId ==
			Client::EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId;
	}

	bool Is_ElementPreviewAdmitted(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		if (Element.eKind == Client::EFFECT_ELEMENT_KIND::LIGHT ||
			Element.eKind == Client::EFFECT_ELEMENT_KIND::SCREEN_POST)
		{
			return Client::Is_EffectToolPresentationPreviewAdmitted(Element);
		}
		// An authoring-approximate carrier owns its exact source resources and
		// only lacks proven material semantics.  It previews so the artist can
		// tune it; product admission is refused elsewhere and is unaffected.
		return Element.bVisible &&
			Client::Is_EffectElementAuthoringExecutionTarget(Element);
	}

	const char* ElementPreviewAdmissionReason(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		if (!Element.bVisible)
			return "Hidden Element: enable Visible before Solo.";
		if (Element.eKind == Client::EFFECT_ELEMENT_KIND::LIGHT ||
			Element.eKind == Client::EFFECT_ELEMENT_KIND::SCREEN_POST)
		{
			return Is_ElementPreviewAdmitted(Element) ?
				"Presentation payload is eligible; Solo checks its preview preparation." :
				"Presentation payload is not admitted for preview; document validation does not make it playable.";
		}
		if (!Client::Is_EffectElementAuthoringExecutionTarget(Element))
		{
			return Element.Detail.Mesh.SourceMaterialSlots.empty() ?
				"Material execution is fail-closed; editing and Save remain available." :
				"A source material slot is not admitted; inspect the Material slot list.";
		}
		return "Eligible for Solo; resources and source dependencies are checked when Solo is pressed.";
	}

	void Prune_MissingElementMarks(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::set<std::string, std::less<>>& Marks)
	{
		if (Marks.empty())
			return;
		// This view lives only during pruning: no pointers survive a document edit.
		std::unordered_set<std::string_view> ElementIds;
		ElementIds.reserve(Document.Elements.size());
		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
			ElementIds.insert(Element.strElementId);
		std::erase_if(Marks, [&ElementIds](const std::string& Id)
			{ return !ElementIds.contains(Id); });
	}

    PARTICLE_LAYER_SUMMARY Summarize_ParticleLayers(
        const Client::EFFECT_DOCUMENT_DESC& Document)
    {
        PARTICLE_LAYER_SUMMARY Summary;
        struct SOURCE_EMITTER_BUDGET final
        {
            uint32_t iMaxParticles = 0u;
            bool_t bHasBaseLayer = false;
        };
        std::map<std::string, SOURCE_EMITTER_BUDGET> SourceEmitterBudgets;
        std::set<std::string> SourceSystems;
        for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
        {
            if (Client::EFFECT_ELEMENT_KIND::MESH == Element.eKind)
            {
                ++Summary.iStandaloneMeshCount;
                continue;
            }
            if (Client::EFFECT_ELEMENT_KIND::SPRITE == Element.eKind)
            {
                ++Summary.iStandaloneSpriteCount;
                continue;
            }
            if (Client::EFFECT_ELEMENT_KIND::PARTICLE != Element.eKind)
                continue;
            ++Summary.iLayerCount;
            if (!Element.strGroupId.empty())
                SourceSystems.insert(Element.strGroupId);
            Summary.iParticleBudget += Element.Detail.Particle.iMaxParticles;
            switch (Resolve_CascadeRendererKind(Element))
            {
            case CASCADE_RENDERER_KIND::MESH:
                ++Summary.iMeshRendererCount;
                break;
            case CASCADE_RENDERER_KIND::SPRITE:
                ++Summary.iSpriteRendererCount;
                break;
            case CASCADE_RENDERER_KIND::UNRESOLVED:
            default:
                ++Summary.iUnresolvedRendererCount;
                break;
            }

            if (!Element.strSourceNode.empty())
            {
                std::string strSourceEmitter = Element.strSourceNode;
                const size_t iBurstMarker = strSourceEmitter.rfind("|burst:");
                const bool_t bBurstLayer = std::string::npos != iBurstMarker ||
                    std::string::npos != Element.strDisplayName.rfind(" Burst ");
                if (std::string::npos != iBurstMarker)
                    strSourceEmitter.erase(iBurstMarker);
                SOURCE_EMITTER_BUDGET& EmitterBudget =
                    SourceEmitterBudgets[strSourceEmitter];
                EmitterBudget.iMaxParticles = (std::max)(
                    EmitterBudget.iMaxParticles,
                    Element.Detail.Particle.iMaxParticles);
                EmitterBudget.bHasBaseLayer =
                    EmitterBudget.bHasBaseLayer || !bBurstLayer;
            }
        }
        Summary.iSourceSystemCount = SourceSystems.size();
        Summary.iSourceEmitterCount = SourceEmitterBudgets.size();
        for (const auto& Entry : SourceEmitterBudgets)
        {
            const SOURCE_EMITTER_BUDGET& EmitterBudget = Entry.second;
            if (!EmitterBudget.bHasBaseLayer)
                Summary.iParticleBudget += EmitterBudget.iMaxParticles;
        }
        return Summary;
    }

	bool_t Is_SourceParticleCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		if (!Element.SourceRecipe.bEnabled)
			return false;
		const std::string_view Shape = Element.SourceRecipe.strRendererShape;
		return Shape == "mesh" || Shape == "sprite" || Shape == "decal";
	}

	bool_t Is_PreviewParticleSimulationElement(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Client::EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind ||
			Is_SourceParticleCarrier(Element);
	}

	f32_t Element_PreviewEndSeconds(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Client::CEffectPlayback::Calculate_ElementEndSeconds(
			Element, Is_SourceParticleCarrier(Element));
	}

    bool Slot_Allowed(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
		if (Client::EFFECT_ELEMENT_KIND::LIGHT == Element.eKind ||
			Client::EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind)
		{
			return false;
		}
        if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID)
            return Client::EFFECT_ELEMENT_KIND::MESH == Element.eKind ||
                Client::EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind;
		if (nullptr != Find_Binding(Element, strSlotId))
			return true;
        return nullptr != Client::Find_EffectMaterialInput(
            Element.Material.strTemplateId, strSlotId);
    }

	bool Is_DirectHandAuthoredElement(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const bool_t bDirectHandAuthored = Element.strSourceNode.empty() ||
			Element.strSourceNode.starts_with("authored-copy:");
		return bDirectHandAuthored && !Element.SourceRecipe.bEnabled &&
			!Element.SourcePresentation.bEnabled &&
			!Element.Material.SourceMaterial.bEnabled &&
			Element.Material.strSourceMaterialPath.empty() &&
			!Element.Material.Execution.bEnabled;
	}

	bool Is_GenericLinearRevealCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const bool_t bSpriteCarrier =
			Element.eKind == Client::EFFECT_ELEMENT_KIND::SPRITE ||
			(Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE &&
			 nullptr == Find_Binding(Element, Client::EFFECT_MESH_SHAPE_SLOT_ID));
		return bSpriteCarrier && Is_DirectHandAuthoredElement(Element) &&
			Element.Renderer.eType == Client::EFFECT_RENDERER_TYPE::END &&
			Element.Material.strTemplateId ==
				Client::EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			!Element.Material.Execution.bFailClosed &&
			!Element.Material.Execution.bAuthoringApproximate &&
			Element.Material.eRenderProfile !=
				Client::EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
	}

	bool Is_OptionalHandAuthoredResourceSlot(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId)
	{
		if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID ||
			strSlotId == "base" || !Is_DirectHandAuthoredElement(Element))
		{
			return false;
		}
		return nullptr != Client::Find_EffectMaterialInput(
			Element.Material.strTemplateId, strSlotId);
	}

	Client::EFFECT_RESOURCE_FILE_KIND Slot_FileKind(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
        if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID)
            return Client::EFFECT_RESOURCE_FILE_KIND::MODEL;
		if (const Client::EFFECT_RESOURCE_BINDING_DESC* pBinding =
			Find_Binding(Element, strSlotId))
		{
			return Resource_FileKind(*pBinding);
		}
        const Client::EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
            Client::Find_EffectMaterialInput(
                Element.Material.strTemplateId, strSlotId);
        return nullptr == pInput ? Client::EFFECT_RESOURCE_FILE_KIND::END :
            pInput->eAllowedResourceKind;
    }

    std::string Slot_Label(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
        if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID)
            return "Mesh Shape";
        const Client::EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
            Client::Find_EffectMaterialInput(
                Element.Material.strTemplateId, strSlotId);
        return nullptr == pInput ? std::string(strSlotId) :
            std::string(pInput->strDisplayName);
    }

    bool InputFloat2(const char* Label, float2_t& Value)
    {
        return ImGui::InputFloat2(Label, &Value.x, "%.3f");
    }

    bool InputFloat3(const char* Label, float3_t& Value)
    {
        return ImGui::InputFloat3(Label, &Value.x, "%.3f");
    }

    bool InputFloat4(const char* Label, float4_t& Value)
    {
        return ImGui::InputFloat4(Label, &Value.x, "%.3f");
    }

    bool DragFloat2(
        const char* Label,
        float2_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format)
    {
        return ImGui::DragFloat2(
            Label, &Value.x, Speed, Minimum, Maximum, Format,
            ImGuiSliderFlags_AlwaysClamp);
    }

    bool DragFloat3(
        const char* Label,
        float3_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format)
    {
        return ImGui::DragFloat3(
            Label, &Value.x, Speed, Minimum, Maximum, Format,
            ImGuiSliderFlags_AlwaysClamp);
    }

    bool DragFloat4(
        const char* Label,
        float4_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format)
    {
        return ImGui::DragFloat4(
            Label, &Value.x, Speed, Minimum, Maximum, Format,
            ImGuiSliderFlags_AlwaysClamp);
    }

    void Copy_Buffer(char* pDestination, const size_t iCapacity,
        const std::string& Source)
    {
        if (nullptr == pDestination || 0u == iCapacity)
            return;
        const size_t Count = (std::min)(iCapacity - 1u, Source.size());
        std::memcpy(pDestination, Source.data(), Count);
        pDestination[Count] = '\0';
    }

    bool_t Is_ManualElementGroupMember(
        const Client::EFFECT_ELEMENT_DESC& Element)
    {
		return Element.strGroupId.starts_with("manual.");
    }

    std::string ManualGroup_Label(const std::string& strGroupId)
    {
        const size_t iSeparator = strGroupId.find_last_of('.');
        const std::string strLeaf = std::string::npos == iSeparator ?
            strGroupId : strGroupId.substr(iSeparator + 1u);
        if (strLeaf.starts_with("hit") && strLeaf.size() > 3u)
            return "Hit " + strLeaf.substr(3u) + " | " + strGroupId;
        return strGroupId;
    }

    std::string ManualElement_Label(
        const Client::EFFECT_ELEMENT_DESC& Element)
    {
        const size_t iSeparator = Element.strElementId.find_last_of('.');
        const std::string strLeaf = std::string::npos == iSeparator ?
            Element.strElementId : Element.strElementId.substr(iSeparator + 1u);
        return std::string(Element.bVisible ? "[ON] " : "[OFF] ") +
            strLeaf + "##" + Element.strElementId;
    }

    float4x4_t Identity_Matrix()
    {
        float4x4_t Result{};
        XMStoreFloat4x4(&Result, XMMatrixIdentity());
        return Result;
    }

    const Client::CHARACTER_SPEC* Resolve_CurrentTargetSpec()
    {
        const std::string assetName =
            Client::CAnimationTargetService::Resolve_AssetName();
        const Client::CHARACTER_SPEC* specs[] =
        {
            &Client::Spec_LanceMaster,
            &Client::Spec_GunSlinger,
            &Client::Spec_Slayer,
            &Client::Spec_Artist,
            &Client::Spec_DimensionMaster,
            &Client::Spec_Warlord
        };
        for (const Client::CHARACTER_SPEC* pSpec : specs)
        {
            if (nullptr != pSpec && nullptr != pSpec->pAssetName &&
                assetName == pSpec->pAssetName)
                return pSpec;
        }
        return nullptr;
    }
}
