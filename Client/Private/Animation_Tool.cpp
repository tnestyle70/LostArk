#include "imgui.h"

#include "Animation_Tool.h"

#include "ActionPresentationTimeline.h"
#include "AnimationPreviewAssets.h"
#include "AnimationTargetService.h"
#include "BalanceTool.h"
#include "BossTool.h"
#include "CameraTool.h"
#include "Character.h"
#include "DataJson.h"
#include "Effect_Catalog.h"
#include "Effect_Tool.h"
#include "EffectAuthoringTransfer.h"
#include "Effect_RuntimeAuthority.h"
#include "GameInstance.h"
#include "HitAreaWire.h"
#include "Level_ValtanArena.h"
#include "MainApp.h"
#include "Model.h"
#include "Part_Body.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "SoundCueCatalog.h"
#include "Transform.h"
#include "Valtan.h"

#include <charconv>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <span>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace
{
	constexpr const char_t* EVENT_FILE_MAGIC = "LOSTARK_ANIM_EVENTS";
	/* v6 separates position follow/snapshot from anchor/action-facing rotation. */
	constexpr int32_t EVENT_FILE_VERSION = 6;
	constexpr int32_t MAX_EVENT_COUNT = 100000;

	/* Used when a clip carries no usable rate, which would otherwise make the
	frame <-> millisecond conversion divide by zero. */
	constexpr f32_t DEFAULT_TICK_RATE = 30.f;
	constexpr f32_t WORKBENCH_DEFAULT_WIDTH = 900.f;
	constexpr f32_t WORKBENCH_DEFAULT_HEIGHT = 700.f;
	constexpr f32_t WORKBENCH_FALLBACK_MIN_WIDTH = 320.f;
	constexpr f32_t WORKBENCH_FALLBACK_MIN_HEIGHT = 200.f;
	/* The outer Workbench may contract to a small viewport, but the three-pane
	   authoring surface keeps a commercial-editor-sized logical canvas and
	   scrolls horizontally instead of crushing Outliner and Detail to labels. */
	constexpr f32_t WORKBENCH_THREE_PANE_INNER_WIDTH = 1120.f;
	constexpr f32_t KAKUL_ACTION_LIST_DEFAULT_WIDTH = 320.f;
	constexpr f32_t KAKUL_ACTION_LIST_MIN_WIDTH = 280.f;
	constexpr f32_t KAKUL_ACTION_DETAIL_MIN_WIDTH = 620.f;
	constexpr f32_t KAKUL_ACTION_SPLITTER_WIDTH = 8.f;
	constexpr uint64_t VALTAN_PATTERN_CREATE_TIMEOUT_MILLISECONDS = 120000u;
	constexpr std::uintmax_t VALTAN_PATTERN_CREATE_MAX_DIAGNOSTIC_BYTES =
		2u * 1024u * 1024u;
	constexpr std::array<const char_t*, 3u>
		VALTAN_PATTERN_CREATE_TARGET_POLICIES = {
			"NONE", "LOCK_NEAREST_ON_START", "LOCK_RANDOM_ALIVE_ON_START" };
	constexpr std::array<const char_t*, 2u>
		VALTAN_PATTERN_CREATE_AIM_POLICIES = {
			"NONE", "LOCK_FACING_ON_START" };

	bool_t Is_LowerSha256(const std::string_view strValue)
	{
		return 64u == strValue.size() &&
			std::all_of(strValue.begin(), strValue.end(),
				[](const char_t Character)
				{
					return (Character >= '0' && Character <= '9') ||
						(Character >= 'a' && Character <= 'f');
				});
	}

	bool_t Is_StablePatternAuthoringId(const std::string_view strValue)
	{
		return !strValue.empty() && strValue.size() <= 160u &&
			std::all_of(strValue.begin(), strValue.end(),
				[](const char_t Character)
				{
					return (Character >= 'a' && Character <= 'z') ||
						(Character >= 'A' && Character <= 'Z') ||
						(Character >= '0' && Character <= '9') ||
						Character == '_' || Character == '-' || Character == '.';
				});
	}

	bool_t Read_BoundedFile(
		const std::filesystem::path& Path,
		const std::uintmax_t iMaximumBytes,
		std::string& strOutBytes,
		std::string& strOutError)
	{
		std::error_code Error;
		const std::uintmax_t iSize = std::filesystem::file_size(Path, Error);
		if (Error || iSize > iMaximumBytes)
		{
			strOutError = Error ?
				"could not inspect " + Path.string() + ": " + Error.message() :
				"file exceeds the bounded diagnostic/input limit: " + Path.string();
			return false;
		}
		std::ifstream Stream(Path, std::ios::binary);
		if (!Stream)
		{
			strOutError = "could not open " + Path.string();
			return false;
		}
		strOutBytes.resize(static_cast<std::size_t>(iSize));
		if (0u != iSize && !Stream.read(
			strOutBytes.data(), static_cast<std::streamsize>(iSize)))
		{
			strOutError = "could not read " + Path.string();
			return false;
		}
		return true;
	}

	bool_t Resolve_PythonExecutable(
		std::filesystem::path& Out,
		std::string& strOutError)
	{
		std::vector<wchar_t> Buffer(32768u, L'\0');
		const DWORD iLength = SearchPathW(
			nullptr, L"python.exe", nullptr,
			static_cast<DWORD>(Buffer.size()), Buffer.data(), nullptr);
		if (0u == iLength || iLength >= Buffer.size())
		{
			strOutError =
				"python.exe was not found through the developer process PATH.";
			return false;
		}
		std::error_code Error;
		Out = std::filesystem::weakly_canonical(
			std::filesystem::path(Buffer.data()), Error);
		if (Error || !std::filesystem::is_regular_file(Out, Error) || Error)
		{
			strOutError =
				"the resolved Python executable is not a regular file.";
			Out.clear();
			return false;
		}
		return true;
	}

	std::string_view ValtanSoundBankForEvent(
		const std::string_view strSoundEvent)
	{
		if (strSoundEvent.starts_with("G_Voltan1_"))
			return "S_Mob_G_Voltan1";
		if (strSoundEvent.starts_with("G_Voltan2_"))
			return "S_Mob_G_Voltan2";
		return {};
	}

	bool_t IsValtanSoundAuthoringCandidate(const std::string& strSoundEvent)
	{
		if (ValtanSoundBankForEvent(strSoundEvent).empty())
			return false;
		const std::vector<std::string>& Variants =
			Client::CSoundCueCatalog::Find_Variants("Valtan", strSoundEvent);
		if (Variants.empty())
			return false;
		for (const std::string& strAssetId : Variants)
		{
			const std::filesystem::path Path =
				Client::CRuntimeAssetRoot::Resolve(strAssetId);
			std::error_code Error;
			if (Path.empty() ||
				!std::filesystem::is_regular_file(Path, Error) || Error)
			{
				return false;
			}
		}
		return true;
	}

	std::unordered_map<std::string, f32_t>
	CollectModelClipSourceDurationSeconds(
		const shared_ptr<Engine::CModel>& pModel)
	{
		std::unordered_map<std::string, f32_t> durations;
		if (nullptr == pModel)
			return durations;
		const std::uint32_t animationCount = pModel->Get_NumAnimations();
		durations.reserve(animationCount);
		for (std::uint32_t animation = 0u;
			animation < animationCount; ++animation)
		{
			const char_t* const clipName =
				pModel->Get_AnimationName(animation);
			const f32_t ticksPerSecond =
				pModel->Get_AnimationTickPerSecond(animation);
			f32_t positionTicks = 0.f;
			f32_t durationTicks = 0.f;
			if (nullptr == clipName || '\0' == clipName[0] ||
				ticksPerSecond <= 0.f ||
				!pModel->Get_AnimationProgress(
					animation, positionTicks, durationTicks) ||
				durationTicks <= 0.f)
			{
				continue;
			}
			const f32_t sourceDurationSeconds =
				durationTicks / ticksPerSecond;
			if (std::isfinite(sourceDurationSeconds) &&
				sourceDurationSeconds > 0.f)
			{
				durations.emplace(clipName, sourceDurationSeconds);
			}
		}
		return durations;
	}

	struct VALTAN_NATIVE_CLIP_TIMING final
	{
		f32_t fDurationTicks = 0.f;
		f32_t fTicksPerSecond = 0.f;
	};

	using VALTAN_NATIVE_CLIP_INVENTORY =
		std::unordered_map<std::string, VALTAN_NATIVE_CLIP_TIMING>;

	bool_t BuildStrictValtanNativeClipInventory(
		const shared_ptr<Engine::CModel>& pModel,
		VALTAN_NATIVE_CLIP_INVENTORY& Out,
		std::string& Status)
	{
		Out.clear();
		if (nullptr == pModel || 0u == pModel->Get_NumAnimations())
		{
			Status = "The admitted Valtan CModel has no native Animation inventory.";
			return false;
		}
		Out.reserve(pModel->Get_NumAnimations());
		for (uint32_t iAnimation = 0u;
			iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
		{
			const char_t* const pClipName =
				pModel->Get_AnimationName(iAnimation);
			const f32_t fTicksPerSecond =
				pModel->Get_AnimationTickPerSecond(iAnimation);
			f32_t fPositionTicks = 0.f;
			f32_t fDurationTicks = 0.f;
			uint32_t iRoundedDurationMs = 0u;
			if (nullptr == pClipName ||
				!Is_StablePatternAuthoringId(pClipName) ||
				!pModel->Get_AnimationProgress(
					iAnimation, fPositionTicks, fDurationTicks) ||
				!Client::CActionPresentationTimeline::
					Validate_AuthoredSourceWindow(
						fDurationTicks, fTicksPerSecond,
						0u, 0u, 1.f, iRoundedDurationMs) ||
				0u == iRoundedDurationMs)
			{
				Status = "The admitted Valtan CModel contains malformed native Animation metadata at index " +
					std::to_string(iAnimation) + ".";
				Out.clear();
				return false;
			}
			if (!Out.emplace(
					pClipName,
					VALTAN_NATIVE_CLIP_TIMING{
						fDurationTicks, fTicksPerSecond }).second)
			{
				Status = "The admitted Valtan CModel contains a duplicate Animation clip: " +
					std::string(pClipName) + ".";
				Out.clear();
				return false;
			}
		}
		return true;
	}

	bool_t SameValtanAnimationAuthoringSignature(
		const Client::VALTAN_STAGE_VIEW& Left,
		const Client::VALTAN_STAGE_VIEW& Right)
	{
		if (Left.strSequenceRole != Right.strSequenceRole ||
			Left.bSuppressAnimation != Right.bSuppressAnimation ||
			Left.ClipOccurrences.size() != Right.ClipOccurrences.size())
		{
			return false;
		}
		return std::equal(
			Left.ClipOccurrences.begin(), Left.ClipOccurrences.end(),
			Right.ClipOccurrences.begin(),
			[](const Client::VALTAN_CLIP_OCCURRENCE_VIEW& A,
				const Client::VALTAN_CLIP_OCCURRENCE_VIEW& B)
			{
				return A.strClipOccurrenceId == B.strClipOccurrenceId &&
					A.strClipName == B.strClipName &&
					A.strMappingBasis == B.strMappingBasis &&
					A.iSourceStartMs == B.iSourceStartMs &&
					A.iPlayMs == B.iPlayMs &&
					A.fPlayRate == B.fPlayRate &&
					A.bLoop == B.bLoop;
			});
	}

	bool_t ValidateValtanStageNativeAnimationWindows(
		const Client::VALTAN_STAGE_VIEW& Stage,
		const VALTAN_NATIVE_CLIP_INVENTORY& Inventory,
		std::string& Status)
	{
		if ("WAIT" == Stage.strSequenceRole)
		{
			if (!Stage.bSuppressAnimation || !Stage.ClipOccurrences.empty())
			{
				Status = "WAIT Stage " + Stage.strStageId +
					" must remain Animation NONE. Create or select an ACTIVE Stage before assigning a Sequence.";
				return false;
			}
			return true;
		}
		if (Stage.bSuppressAnimation)
		{
			if (!Stage.ClipOccurrences.empty())
			{
				Status = "Animation NONE Stage " + Stage.strStageId +
					" cannot retain clip occurrences.";
				return false;
			}
			return true;
		}
		if (Stage.ClipOccurrences.empty())
		{
			Status = "Animation Stage " + Stage.strStageId +
				" has no native clip occurrence.";
			return false;
		}
		for (const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
			Stage.ClipOccurrences)
		{
			const auto Native = Inventory.find(Occurrence.strClipName);
			if (Native == Inventory.end())
			{
				Status = "Animation Stage " + Stage.strStageId +
					" references a clip absent from the admitted Valtan CModel: " +
					Occurrence.strClipName + ".";
				return false;
			}
			uint32_t iRoundedRemainingMs = 0u;
			if (!Client::CActionPresentationTimeline::
					Validate_AuthoredSourceWindow(
						Native->second.fDurationTicks,
						Native->second.fTicksPerSecond,
						Occurrence.iSourceStartMs,
						Occurrence.iPlayMs,
						Occurrence.fPlayRate,
						iRoundedRemainingMs))
			{
				Status = "Animation Stage " + Stage.strStageId + " occurrence " +
					Occurrence.strClipOccurrenceId +
					" escapes the admitted native source window for " +
					Occurrence.strClipName + ".";
				return false;
			}
		}
		return true;
	}
	/* Both Valtan bodies author into one document. That is only true because
	Tools/ModelAssetConverter/bake_ghost_valtan_animset.py puts the product clip
	vocabulary on the ghost rig, so a chain written on either body names clips
	the other also owns, and promote_valtan_animation_chains.py still joins this
	one document to the one presentation model it pins.

	The ghost body also keeps its own rpbf_02.ao_* clips, which the product body
	does not have. Authoring one of those into this shared document would make
	that promotion reject a clip the product model cannot play, so the clip list
	only offers the vocabulary both bodies share. */
	constexpr const char_t* CUSTOM_CHAIN_CLIP_PREFIX = "mesh_";
	constexpr const wchar_t* VALTAN_PATTERN_TRANSACTION_LOCK_RELATIVE =
		L"out\\ValtanPatternTransactions\\create-pattern.lock";

	class SCOPED_VALTAN_PATTERN_TRANSACTION_LOCK final
	{
	public:
		~SCOPED_VALTAN_PATTERN_TRANSACTION_LOCK()
		{
			if (INVALID_HANDLE_VALUE == m_hFile)
				return;
			UnlockFileEx(m_hFile, 0u, 1u, 0u, &m_Overlap);
			CloseHandle(m_hFile);
		}

		bool_t Try_Acquire(
			const std::filesystem::path& ProjectRoot,
			std::string& strOutError)
		{
			if (ProjectRoot.empty())
			{
				strOutError = "project root is unavailable";
				return false;
			}
			const std::filesystem::path LockPath =
				ProjectRoot / VALTAN_PATTERN_TRANSACTION_LOCK_RELATIVE;
			std::error_code DirectoryError;
			std::filesystem::create_directories(
				LockPath.parent_path(), DirectoryError);
			if (DirectoryError)
			{
				strOutError = "lock directory creation failed: " +
					DirectoryError.message();
				return false;
			}
			m_hFile = CreateFileW(
				LockPath.c_str(), GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (INVALID_HANDLE_VALUE == m_hFile)
			{
				strOutError = "lock open failed with Win32 error " +
					std::to_string(GetLastError());
				return false;
			}

			LARGE_INTEGER Size{};
			if (FALSE == GetFileSizeEx(m_hFile, &Size))
			{
				strOutError = "lock size query failed with Win32 error " +
					std::to_string(GetLastError());
				CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
				return false;
			}
			if (Size.QuadPart < 1)
			{
				const char_t Byte = '\0';
				DWORD iWritten = 0u;
				LARGE_INTEGER Begin{};
				if (FALSE == SetFilePointerEx(
						m_hFile, Begin, nullptr, FILE_BEGIN) ||
					FALSE == WriteFile(
						m_hFile, &Byte, 1u, &iWritten, nullptr) ||
					1u != iWritten || FALSE == FlushFileBuffers(m_hFile))
				{
					strOutError = "lock initialization failed with Win32 error " +
						std::to_string(GetLastError());
					CloseHandle(m_hFile);
					m_hFile = INVALID_HANDLE_VALUE;
					return false;
				}
			}

			if (FALSE == LockFileEx(m_hFile,
				LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
				0u, 1u, 0u, &m_Overlap))
			{
				strOutError = "another Create/Project writer owns the lock (Win32 " +
					std::to_string(GetLastError()) + ")";
				CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
				return false;
			}
			return true;
		}

	private:
		HANDLE m_hFile = INVALID_HANDLE_VALUE;
		OVERLAPPED m_Overlap{};
	};

	struct CUSTOM_CHAIN_PROFILE
	{
		const char_t* pAssetName;
		const wchar_t* pFileName;
		const char_t* pFileLabel;
		const char_t* pOccurrencePrefix;
		const char_t* pWindowTitle;
		/* The clip a finished chain returns to. Both bodies answer to the same
		name once the donor is attached; naming a clip the body does not have
		leaves it frozen on its last pose instead of idling. */
		const char_t* pIdleClip;
	};

	constexpr std::array<CUSTOM_CHAIN_PROFILE, 2u> CUSTOM_CHAIN_PROFILES = {
		CUSTOM_CHAIN_PROFILE{
			"Valtan",
			L"Valtan.presentation.debug.json",
			"Data/Valtan/Valtan.presentation.debug.json",
			"valtan.debug",
			"Valtan Animation Sequence Intake",
			"mesh_idle_battle_1"
		},
		CUSTOM_CHAIN_PROFILE{
			"Valtan_Ghost_MN_RPBF_02",
			L"Valtan.presentation.debug.json",
			"Data/Valtan/Valtan.presentation.debug.json",
			"valtan.debug",
			"Valtan Animation Sequence Intake (Ghost body)",
			"mesh_idle_battle_1"
		}
	};

	const CUSTOM_CHAIN_PROFILE* Find_CustomChainProfile(
		const std::string& strAssetName)
	{
		for (const CUSTOM_CHAIN_PROFILE& Profile : CUSTOM_CHAIN_PROFILES)
		{
			if (strAssetName == Profile.pAssetName)
				return &Profile;
		}
		return nullptr;
	}

	struct KAKUL_ACTION_PROFILE_CONTRACT final
	{
		const char_t* pProfileId;
		const char_t* pPreviewAssetName;
		const char_t* pModelAssetId;
	};

	constexpr std::array<KAKUL_ACTION_PROFILE_CONTRACT, 4u>
		KAKUL_ACTION_PROFILES = {
			KAKUL_ACTION_PROFILE_CONTRACT{
				"MN_RPCT_05", "MN_RPCT_05",
				"Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05" },
			KAKUL_ACTION_PROFILE_CONTRACT{
				"MN_RPCT_06", "MN_RPCT_06",
				"Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06" },
			KAKUL_ACTION_PROFILE_CONTRACT{
				"MN_RPCT_07", "MN_RPCT_05",
				"Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05" },
			KAKUL_ACTION_PROFILE_CONTRACT{
				"MN_RPCZ_00", "MN_RPCZ_00",
				"Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00" },
		};

	const KAKUL_ACTION_PROFILE_CONTRACT* Find_KakulActionProfile(
		const std::string_view strProfileId)
	{
		const auto Found = std::find_if(
			KAKUL_ACTION_PROFILES.begin(), KAKUL_ACTION_PROFILES.end(),
			[strProfileId](const KAKUL_ACTION_PROFILE_CONTRACT& Candidate)
			{
				return strProfileId == Candidate.pProfileId;
			});
		return Found == KAKUL_ACTION_PROFILES.end() ? nullptr : &*Found;
	}

	const KAKUL_ACTION_PROFILE_CONTRACT* Default_KakulActionProfileForAsset(
		const std::string_view strAssetName)
	{
		/* MN_RPCT_05 is the default for the shared 05/07 physical body. A
		   Resource Files request can still select the exact 07 alias. */
		for (const KAKUL_ACTION_PROFILE_CONTRACT& Profile : KAKUL_ACTION_PROFILES)
		{
			if (strAssetName == Profile.pPreviewAssetName)
				return &Profile;
		}
		return nullptr;
	}

	constexpr std::array<std::string_view, 7u>
		VALTAN_PATTERN_MASTER_ORDER = {
			"VALTAN_WHIRLWIND",
			"VALTAN_DASH_CHARGE",
			"VALTAN_FOUR_SLASH",
			"VALTAN_FIST_IN_OUT",
			"VALTAN_HIGH_JUMP",
			"VALTAN_FLOOR_WIPE_130",
			"VALTAN_ARENA_BREAK_109",
		};

	const Client::VALTAN_PATTERN_VIEW* Find_ValtanPatternMaster(
		const Client::VALTAN_PATTERN_TREE_VIEW& View,
		const std::string_view strPatternId)
	{
		const auto FindIn = [strPatternId](
			const std::vector<Client::VALTAN_PATTERN_VIEW>& Patterns)
			-> const Client::VALTAN_PATTERN_VIEW*
		{
			const auto Pattern = std::find_if(
				Patterns.begin(), Patterns.end(),
				[strPatternId](const Client::VALTAN_PATTERN_VIEW& Candidate)
				{
					return Candidate.bAuthoringMasterManaged &&
						Candidate.strPatternId == strPatternId;
				});
			return Pattern == Patterns.end() ? nullptr : &*Pattern;
		};
		if (const Client::VALTAN_PATTERN_VIEW* pPattern = FindIn(View.Rotation))
			return pPattern;
		return FindIn(View.Gimmicks);
	}

	bool_t Try_ResolveValtanArenaPatternAction(
		const std::string_view strStageKind,
		LostArk::Shared::WORLD_ENTITY_ACTION& eOutAction)
	{
		using LostArk::Shared::WORLD_ENTITY_ACTION;
		if ("WINDUP" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_WINDUP;
		else if ("ACTIVE" == strStageKind || "GROGGY" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_ACTIVE;
		else if ("RECOVERY" == strStageKind || "PART_BREAK" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_RECOVERY;
		else
			return false;
		return true;
	}

	void Skip_Space(const char_t*& p)
	{
		while (' ' == *p || '\t' == *p)
			++p;
	}

	bool_t Read_Quoted(const char_t*& p, std::string& out)
	{
		Skip_Space(p);
		if ('\"' != *p)
			return false;
		++p;
		out.clear();
		while ('\0' != *p && '\"' != *p)
			out.push_back(*p++);
		if ('\"' != *p)
			return false;
		++p;
		return true;
	}

	bool_t Read_Token(const char_t*& p, std::string& out)
	{
		Skip_Space(p);
		out.clear();
		while ('\0' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
			out.push_back(*p++);
		return !out.empty();
	}

	/* key=value, where value may be quoted so paths with spaces survive. */
	bool_t Read_Pair(const char_t*& p, std::string& key, std::string& value)
	{
		Skip_Space(p);
		key.clear();
		while ('\0' != *p && '=' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
			key.push_back(*p++);
		if ('=' != *p || key.empty())
			return false;
		++p;

		value.clear();
		if ('\"' == *p)
		{
			++p;
			while ('\0' != *p && '\"' != *p)
				value.push_back(*p++);
			if ('\"' != *p)
				return false;
			++p;
		}
		else
		{
			while ('\0' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
				value.push_back(*p++);
		}
		return true;
	}

	bool_t Parse_Integer(const std::string& text, int32_t& outValue)
	{
		if (text.empty())
			return false;

		const char_t* begin = text.data();
		const char_t* end = begin + text.size();
		const auto result = std::from_chars(begin, end, outValue);
		return std::errc{} == result.ec && result.ptr == end;
	}

	bool_t Parse_Float(const std::string& text, f32_t& outValue)
	{
		if (text.empty())
			return false;
		char_t* pEnd = nullptr;
		outValue = std::strtof(text.c_str(), &pEnd);
		return pEnd == text.c_str() + text.size() &&
			std::isfinite(outValue);
	}

	bool_t Has_RemainingToken(const char_t* p)
	{
		Skip_Space(p);
		return '\0' != *p && '\r' != *p && '\n' != *p;
	}

	bool_t Is_SafeQuotedText(const std::string& text)
	{
		return std::string::npos == text.find_first_of("\"\r\n");
	}

	bool_t Parse_CommaSeparatedStableTokens(
		const std::string& Text,
		std::vector<std::string>& Out)
	{
		Out.clear();
		std::size_t iStart = 0u;
		while (iStart <= Text.size())
		{
			const std::size_t iComma = Text.find(',', iStart);
			const std::string Token = Text.substr(
				iStart, std::string::npos == iComma ? std::string::npos :
					iComma - iStart);
			if (!Is_StablePatternAuthoringId(Token))
				return false;
			Out.push_back(Token);
			if (Out.size() > 64u || std::string::npos == iComma)
				break;
			iStart = iComma + 1u;
		}
		return !Out.empty() && Out.size() <= 64u;
	}

	bool_t Parse_CommaSeparatedNonnegativeSeconds(
		const std::string& Text,
		std::vector<f32_t>& Out)
	{
		Out.clear();
		std::size_t iStart = 0u;
		while (iStart <= Text.size())
		{
			const std::size_t iComma = Text.find(',', iStart);
			const std::string Token = Text.substr(
				iStart, std::string::npos == iComma ? std::string::npos :
					iComma - iStart);
			f32_t fSeconds = 0.f;
			if (!Parse_Float(Token, fSeconds) || fSeconds < 0.f ||
				fSeconds > 600.f)
			{
				return false;
			}
			Out.push_back(fSeconds);
			if (Out.size() > 64u || std::string::npos == iComma)
				break;
			iStart = iComma + 1u;
		}
		return !Out.empty() && Out.size() <= 64u;
	}

	bool_t Load_ValtanCompositionSequenceLibrary(
		std::vector<Client::CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW>& Out,
		std::string& Status)
	{
		using SEQUENCE_VIEW =
			Client::CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW;
		using CLIP_VIEW =
			Client::CAnimation_Tool::COMPOSITION_SEQUENCE_CLIP_VIEW;
		const std::filesystem::path Root =
			Client::CProjectDataRoot::Resolve(L"Animation/Reference/Valtan");
		const std::filesystem::path SequencePath = Root / L"Valtan.clipseq";
		const std::filesystem::path CutPath = Root / L"Valtan.clipcuts";
		std::ifstream SequenceFile(SequencePath, std::ios::binary);
		std::ifstream CutFile(CutPath, std::ios::binary);
		if (Root.empty() || !SequenceFile || !CutFile)
		{
			Status =
				"Valtan Sequence library is missing its .clipseq or .clipcuts source.";
			return false;
		}

		auto ParseHeader = [&Status](
			std::ifstream& Stream,
			const char_t* const pExpectedMagic,
			const int32_t iExpectedVersion,
			int32_t& iOutCount)
		{
			std::string Line;
			if (!std::getline(Stream, Line))
				return false;
			const char_t* p = Line.c_str();
			std::string Magic;
			std::string Version;
			std::string Owner;
			std::string Count;
			int32_t iVersion = 0;
			if (!Read_Token(p, Magic) || !Read_Token(p, Version) ||
				!Read_Quoted(p, Owner) || !Read_Token(p, Count) ||
				Has_RemainingToken(p) ||
				!Parse_Integer(Version, iVersion) ||
				!Parse_Integer(Count, iOutCount) ||
				Magic != pExpectedMagic || iVersion != iExpectedVersion ||
				"Valtan" != Owner || iOutCount < 1 || iOutCount > 4096)
			{
				Status = std::string(pExpectedMagic) +
					" header is malformed or does not name Valtan.";
				return false;
			}
			return true;
		};

		int32_t iSequenceCount = 0;
		if (!ParseHeader(
				SequenceFile, "LOSTARK_CLIP_SEQ", 2, iSequenceCount))
		{
			return false;
		}
		std::vector<SEQUENCE_VIEW> Staged;
		Staged.reserve(static_cast<std::size_t>(iSequenceCount));
		std::map<std::pair<int32_t, int32_t>, std::size_t> SequenceByKey;
		std::string Line;
		for (int32_t iRow = 0; iRow < iSequenceCount; ++iRow)
		{
			if (!std::getline(SequenceFile, Line))
			{
				Status = "Valtan.clipseq ended before its declared row count.";
				return false;
			}
			const char_t* p = Line.c_str();
			std::string Action;
			std::string DisplayName;
			int32_t iAction = 0;
			if (!Read_Token(p, Action) || !Read_Quoted(p, DisplayName) ||
				!Parse_Integer(Action, iAction) || iAction <= 0 ||
				!Is_SafeQuotedText(DisplayName) || DisplayName.empty())
			{
				Status = "Valtan.clipseq contains a malformed action identity/name row.";
				return false;
			}
			SEQUENCE_VIEW Sequence;
			Sequence.iSkillId = iAction;
			bool_t bSawSequence = false;
			bool_t bSawMode = false;
			bool_t bSawClips = false;
			std::vector<std::string> Clips;
			std::string Key;
			std::string Value;
			while (Read_Pair(p, Key, Value))
			{
				if ("seq" == Key && !bSawSequence)
				{
					bSawSequence = Parse_Integer(
						Value, Sequence.iSequenceIndex) &&
						Sequence.iSequenceIndex >= 0;
				}
				else if ("mode" == Key && !bSawMode)
				{
					bSawMode = "COMBO" == Value || "HOLD" == Value ||
						"SEQUENCE" == Value;
					Sequence.strMode = Value;
				}
				else if ("clips" == Key && !bSawClips)
				{
					bSawClips = Parse_CommaSeparatedStableTokens(Value, Clips);
				}
				else
				{
					Status = "Valtan.clipseq contains a duplicate or unknown field.";
					return false;
				}
			}
			if (Has_RemainingToken(p) || !bSawSequence || !bSawMode ||
				!bSawClips || !SequenceByKey.emplace(
					std::pair{ Sequence.iSkillId, Sequence.iSequenceIndex },
					Staged.size()).second)
			{
				Status = "Valtan.clipseq sequence row is incomplete or duplicated.";
				return false;
			}
			Sequence.strDisplayName = std::move(DisplayName);
			Sequence.Clips.reserve(Clips.size());
			for (std::string& Clip : Clips)
			{
				CLIP_VIEW View;
				View.strClipName = std::move(Clip);
				Sequence.Clips.push_back(std::move(View));
			}
			Staged.push_back(std::move(Sequence));
		}
		while (std::getline(SequenceFile, Line))
		{
			if (Line.find_first_not_of(" \t\r\n") != std::string::npos)
			{
				Status = "Valtan.clipseq contains rows beyond its declared count.";
				return false;
			}
		}

		int32_t iCutCount = 0;
		if (!ParseHeader(CutFile, "LOSTARK_CLIP_CUTS", 1, iCutCount) ||
			iCutCount != iSequenceCount)
		{
			Status = "Valtan.clipcuts does not match the Sequence catalog count.";
			return false;
		}
		std::map<std::pair<int32_t, int32_t>, std::vector<f32_t>> CutsByKey;
		for (int32_t iRow = 0; iRow < iCutCount; ++iRow)
		{
			if (!std::getline(CutFile, Line))
			{
				Status = "Valtan.clipcuts ended before its declared row count.";
				return false;
			}
			const char_t* p = Line.c_str();
			std::string Action;
			int32_t iAction = 0;
			int32_t iSequence = -1;
			bool_t bSawSequence = false;
			bool_t bSawCuts = false;
			std::vector<f32_t> Cuts;
			if (!Read_Token(p, Action) || !Parse_Integer(Action, iAction) ||
				iAction <= 0)
			{
				Status = "Valtan.clipcuts contains a malformed action identity.";
				return false;
			}
			std::string Key;
			std::string Value;
			while (Read_Pair(p, Key, Value))
			{
				if ("seq" == Key && !bSawSequence)
					bSawSequence = Parse_Integer(Value, iSequence) && iSequence >= 0;
				else if ("cuts" == Key && !bSawCuts)
					bSawCuts = Parse_CommaSeparatedNonnegativeSeconds(Value, Cuts);
				else
				{
					Status = "Valtan.clipcuts contains a duplicate or unknown field.";
					return false;
				}
			}
			if (Has_RemainingToken(p) || !bSawSequence || !bSawCuts ||
				!CutsByKey.emplace(std::pair{ iAction, iSequence },
					std::move(Cuts)).second)
			{
				Status = "Valtan.clipcuts row is incomplete or duplicated.";
				return false;
			}
		}

		for (SEQUENCE_VIEW& Sequence : Staged)
		{
			const auto Cuts = CutsByKey.find(
				{ Sequence.iSkillId, Sequence.iSequenceIndex });
			if (Cuts == CutsByKey.end() ||
				Cuts->second.size() != Sequence.Clips.size())
			{
				Status = "Valtan Sequence has no exact per-clip source timing: " +
					std::to_string(Sequence.iSkillId) + "/" +
					std::to_string(Sequence.iSequenceIndex) + ".";
				return false;
			}
			std::vector<CLIP_VIEW> TimedClips;
			TimedClips.reserve(Sequence.Clips.size());
			for (std::size_t iClip = 0u; iClip < Sequence.Clips.size(); ++iClip)
			{
				/* Extracted 0.000 cuts are explicit skipped source steps, not a
				   malformed Sequence and not a zero-duration authoring slot. Keep
				   the exact positional join above, then omit only those markers from
				   the model-independent composition view. */
				if (Cuts->second[iClip] <= 0.f)
					continue;
				CLIP_VIEW Clip = std::move(Sequence.Clips[iClip]);
				Clip.iDurationMs = static_cast<uint32_t>(
					std::llround(static_cast<double>(Cuts->second[iClip]) * 1000.0));
				if (0u == Clip.iDurationMs)
				{
					Status = "Valtan Sequence contains a positive cut below one millisecond: " +
						std::to_string(Sequence.iSkillId) + "/" +
						std::to_string(Sequence.iSequenceIndex) + ".";
					return false;
				}
				Clip.bUsesNativeDuration = false;
				TimedClips.push_back(std::move(Clip));
			}
			if (TimedClips.empty())
			{
				Status = "Valtan Sequence contains only skipped source steps: " +
					std::to_string(Sequence.iSkillId) + "/" +
					std::to_string(Sequence.iSequenceIndex) + ".";
				return false;
			}
			Sequence.Clips = std::move(TimedClips);
		}
		Out = std::move(Staged);
		Status = "Loaded " + std::to_string(Out.size()) +
			" exact Valtan source Sequences from .clipseq + .clipcuts (model-independent).";
		return true;
	}

	bool_t Contains_NoCase(const char_t* pText, const char_t* pNeedle)
	{
		if (nullptr == pText)
			return false;
		if ('\0' == pNeedle[0])
			return true;

		for (const char_t* p = pText; *p != '\0'; ++p)
		{
			const char_t* a = p;
			const char_t* b = pNeedle;
			while (*a != '\0' && *b != '\0' &&
				tolower(static_cast<unsigned char>(*a)) == tolower(static_cast<unsigned char>(*b)))
			{
				++a;
				++b;
			}
			if ('\0' == *b)
				return true;
		}
		return false;
	}

	/* Start_Animation cuts straight to the new clip's first frame, which is what
	   the product Valtan still does. The preview blends the way CCharacter
	   already does so the animator judges motion rather than the seam. The
	   blend snapshot has to be taken before the track reset overwrites the
	   bones, hence the two calls in this order. Clips repeated back to back
	   keep their hard restart: the pose is identical, so there is nothing to
	   blend, and a re-strike is meant to read as a new strike. */
	bool_t Start_PreviewClip(
		const shared_ptr<Engine::CModel>& pModel,
		const char_t* pClipName,
		const bool_t bLoop,
		const f32_t fBlendSeconds)
	{
		if (nullptr == pModel || nullptr == pClipName)
			return false;
		if (fBlendSeconds > 0.f)
			pModel->Set_Animation(pClipName, bLoop, fBlendSeconds);
		return pModel->Start_Animation(pClipName, bLoop);
	}
}

Client::CAnimation_Tool::CAnimation_Tool(
	shared_ptr<CCharacterPreviewPanel> pPreviewPanel,
	CBalanceTool* const pBalanceTool,
	CBossTool* const pBossTool)
	: m_pPreviewPanel(std::move(pPreviewPanel))
	, m_pBalanceTool(pBalanceTool)
	, m_pBossTool(pBossTool)
{
}

Client::CAnimation_Tool::~CAnimation_Tool()
{
	if (nullptr != m_hValtanPatternCreateProcess)
	{
		/* The child owns an all-or-nothing source/Product transaction.  Closing
		   our observation handle on tool teardown must not terminate it midway;
		   the preserved request and diagnostic paths remain the recovery trail. */
		CloseHandle(static_cast<HANDLE>(m_hValtanPatternCreateProcess));
		m_hValtanPatternCreateProcess = nullptr;
	}
	if (nullptr != m_pPreviewPanel)
		m_pPreviewPanel->Set_SessionLock(
			CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL, false, {});
}

bool_t Client::CAnimation_Tool::Consume_EffectToolOpenRequest(
	EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& outRequest)
{
	if (!m_hasEffectToolOpenRequest)
		return false;
	outRequest.strPatternId = std::move(m_strEffectToolOpenPatternId);
	outRequest.strStageId = std::move(m_strEffectToolOpenStageId);
	outRequest.strCueOccurrenceId =
		std::move(m_strEffectToolOpenCueOccurrenceId);
	outRequest.strEffectAssetId = std::move(m_strEffectToolOpenEffectAssetId);
	m_strEffectToolOpenPatternId.clear();
	m_strEffectToolOpenStageId.clear();
	m_strEffectToolOpenCueOccurrenceId.clear();
	m_strEffectToolOpenEffectAssetId.clear();
	m_hasEffectToolOpenRequest = false;
	return true;
}

bool_t Client::CAnimation_Tool::Consume_CameraToolOpenRequest(
	CAMERA_TOOL_OPEN_REQUEST& outRequest)
{
	if (!m_hasCameraToolOpenRequest)
		return false;
	outRequest.strCueId = std::move(m_strCameraToolOpenCueId);
	m_strCameraToolOpenCueId.clear();
	m_hasCameraToolOpenRequest = false;
	return true;
}

void Client::CAnimation_Tool::Update(
	const f32_t fTimeDelta,
	const bool_t bIsActiveTool)
{
	Poll_ValtanPatternCreateCommand();

	if (m_bValtanPatternMasterPlaying)
	{
		const shared_ptr<Engine::CModel> PreviewModel =
			m_ValtanPatternMasterModel.lock();
		const shared_ptr<CValtan> PreviewBoss =
			m_ValtanPatternMasterBoss.lock();
		if (nullptr == PreviewModel || nullptr == PreviewBoss ||
			0u == m_iValtanPatternMasterTargetGeneration ||
			m_iValtanPatternMasterTargetGeneration !=
				CAnimationTargetService::Resolve_TargetGeneration() ||
			CAnimationTargetService::Resolve_Model() != PreviewModel ||
			CAnimationTargetService::Resolve_Boss() != PreviewBoss ||
			PreviewBoss->Get_BodyModel() != PreviewModel)
		{
			if (nullptr != PreviewModel)
			{
				Stop_ValtanPatternMasterPreview(
					PreviewModel,
					"Valtan Pattern Master preview cancelled because the animation target changed; the original boss was restored to idle.");
			}
			else
			{
				Reset_ValtanPatternMasterPreviewState(
					"Valtan Pattern Master preview cancelled because the animation target changed.");
			}
			return;
		}
		if (!bIsActiveTool)
		{
			Stop_ValtanPatternMasterPreview(
				PreviewModel,
				"Valtan Pattern Master preview stopped because Animation Tool was deactivated; idle restored.");
			return;
		}
		if (m_iValtanPatternMasterItem >=
			m_ValtanPatternMasterPlaylist.size())
		{
			Stop_ValtanPatternMasterPreview(
				PreviewModel,
				"Valtan Pattern Master preview stopped because its admitted timeline became invalid; idle restored.");
			return;
		}

		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
			m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
		const char_t* pCurrentClip = PreviewModel->Get_AnimationName(
			PreviewModel->Get_CurrentAnimIndex());
		if (nullptr == pCurrentClip || Item.strClipName != pCurrentClip)
		{
			Stop_ValtanPatternMasterPreview(
				PreviewModel,
				"Valtan Pattern Master preview stopped because another control replaced its clip; idle restored.");
			return;
		}
		if (!m_bValtanPatternMasterPaused &&
			std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
		{
			m_fValtanPatternMasterItemElapsedSeconds +=
				fTimeDelta * m_fValtanPatternPreviewSpeed;
			const f32_t fItemDurationSeconds =
				static_cast<f32_t>(Item.iAuthoringWallMs) * 0.001f;
			if (m_fValtanPatternMasterItemElapsedSeconds + 0.000001f >=
				fItemDurationSeconds)
			{
				Advance_ValtanPatternMasterPreview(PreviewModel);
			}
			else if (!Apply_ValtanPatternMasterPose(
				PreviewModel, Item,
				m_fValtanPatternMasterItemElapsedSeconds, false))
			{
				Stop_ValtanPatternMasterPreview(
					PreviewModel,
					"Valtan Pattern Master preview stopped because its source-clock sample failed; idle restored.");
				return;
			}
		}
		Update_ValtanPatternMasterHitAreaPreview();
		return;
	}

	if (!m_bValtanPatternPreviewPlaying)
		return;

	const shared_ptr<Engine::CModel> PreviewModel =
		m_ValtanPatternPreviewModel.lock();
	if (nullptr == PreviewModel ||
		0u == m_iValtanPatternPreviewTargetGeneration ||
		m_iValtanPatternPreviewTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_Model() != PreviewModel)
	{
		if (nullptr != PreviewModel)
			PreviewModel->Set_AnimationSpeed(1.f);
		Reset_ValtanPatternPreviewState(
			"Pattern preview cancelled because the animation target changed.");
		return;
	}

	if (!bIsActiveTool)
	{
		Stop_ValtanPatternPreview(
			PreviewModel,
			"Pattern preview stopped because Animation Tool was deactivated; idle restored.");
		return;
	}
	if (m_bValtanPatternPreviewPaused ||
		m_iValtanPatternPreviewItem >= m_ValtanPatternPreviewPlaylist.size())
	{
		return;
	}

	const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
		m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
	const char_t* pExpectedClip = Item.bPatternMarker ?
		"mesh_idle_battle_1" : Item.strClipName.c_str();
	const char_t* pCurrentClip = PreviewModel->Get_AnimationName(
		PreviewModel->Get_CurrentAnimIndex());
	if (nullptr == pCurrentClip ||
		std::string_view{ pExpectedClip } != std::string_view{ pCurrentClip })
	{
		Stop_ValtanPatternPreview(
			PreviewModel,
			"Pattern preview stopped because another control replaced the current source clip; idle restored.");
		return;
	}

	if (!std::isfinite(fTimeDelta) || fTimeDelta <= 0.f)
		return;
	m_fValtanPatternPreviewElapsedSeconds +=
		fTimeDelta * m_fValtanPatternPreviewSpeed;
	if (m_fValtanPatternPreviewElapsedSeconds >=
		m_fValtanPatternPreviewItemDurationSeconds)
	{
		Advance_ValtanPatternPreview(PreviewModel);
	}
	Update_ValtanPatternHitAreaPreview();
}

shared_ptr<Engine::CModel> Client::CAnimation_Tool::Resolve_Model() const
{
	return CAnimationTargetService::Resolve_Model();
}

shared_ptr<Client::CCharacter> Client::CAnimation_Tool::Resolve_Character() const
{
	return CAnimationTargetService::Resolve_Character();
}

bool_t Client::CAnimation_Tool::Is_AnyDocumentDirty() const
{
	return m_bDirty || m_bSkillBindingDirty ||
		m_bValtanPatternSoundCuesDirty ||
		m_bValtanCombatObjectSoundCuesDirty ||
		m_bValtanPatternAnimationBindingDirty || m_bKakulActionDirty;
}

bool_t Client::CAnimation_Tool::Is_ValtanDocumentDirty() const
{
	return m_bValtanPatternSoundCuesDirty ||
		m_bValtanCombatObjectSoundCuesDirty ||
		m_bValtanPatternAnimationBindingDirty;
}

void Client::CAnimation_Tool::On_LevelChanged()
{
	/* LEVEL is an enum, not a level-instance generation.  MainApp calls this
	   after every committed Change_Level so re-entering VALTAN_ARENA with the
	   same enum cannot inherit an earlier failed/successful staging latch. */
	m_iValtanAutoPreviewAttemptGeneration = 0u;
	m_iValtanAutoPreviewSuccessGeneration = 0u;
	m_bValtanWorkspaceTabInitialized = false;
}

void Client::CAnimation_Tool::Reset_KakulActionDocumentState(
	const bool_t bClearProfile)
{
	m_KakulActionReference = {};
	m_KakulActionAuthored = {};
	m_bKakulActionLoadAttempted = false;
	m_bKakulActionDirty = false;
	m_bKakulActionReloadConfirmationRequested = false;
	m_iSelectedKakulAction = -1;
	m_iSelectedKakulStage = 0;
	m_iSelectedKakulSlot = 0;
	m_KakulActionFilter[0] = '\0';
	m_strKakulActionStatus.clear();
	if (bClearProfile)
		m_strKakulProfileId.clear();
}

bool_t Client::CAnimation_Tool::Open_ValtanWorkspace()
{
	if (Is_AnyDocumentDirty() && !m_AssetName.empty() &&
		"Valtan" != m_AssetName)
	{
		m_Status =
			"Save or discard the current Animation document before opening the Valtan data workspace.";
		return false;
	}

	m_bValtanDataWorkspaceRequested = true;
	m_bValtanWorkspaceTabInitialized = false;
	m_Status =
		"Opened the canonical Valtan Pattern workspace. Product data remains visible even when the local Model View is unavailable.";
	return true;
}

bool_t Client::CAnimation_Tool::Stage_ValtanCompositionPreview(
	std::string& strOutStatus)
{
	if (!Open_ValtanWorkspace())
	{
		strOutStatus = m_Status;
		return false;
	}
	if (nullptr == m_pPreviewPanel ||
		!m_pPreviewPanel->Select_TargetAsset("Valtan"))
	{
		strOutStatus = nullptr == m_pPreviewPanel ?
			"Valtan preview panel is unavailable; canonical data-only authoring remains available." :
			"Valtan Model View could not be staged; canonical data-only authoring remains available: " +
				m_pPreviewPanel->Get_Status();
		return false;
	}
	const std::string strAssetName =
		CAnimationTargetService::Resolve_AssetName();
	const shared_ptr<CValtan> pBoss = CAnimationTargetService::Resolve_Boss();
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if ("Valtan" != strAssetName || nullptr == pBoss || nullptr == pModel ||
		pBoss->Get_BodyModel() != pModel)
	{
		strOutStatus =
			"The selected target is not the dedicated Valtan Model View; no unrelated scene model was substituted.";
		return false;
	}
	if (!Sync_AssetName())
	{
		strOutStatus = m_Status;
		return false;
	}
	if (!m_bValtanPatternMasterLoadAttempted)
	{
		m_bValtanPatternMasterLoadAttempted = true;
		if (!Reload_ValtanPatternMaster())
		{
			strOutStatus = m_strValtanPatternMasterStatus;
			return false;
		}
	}
	m_iValtanAutoPreviewAttemptGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iValtanAutoPreviewSuccessGeneration =
		m_iValtanAutoPreviewAttemptGeneration;
	strOutStatus = "Dedicated Valtan Model View is ready for Action Composition preview.";
	return true;
}

bool_t Client::CAnimation_Tool::Play_ValtanCompositionPattern(
	const std::string& strPatternId,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	std::string& strOutStatus)
{
	if (strPatternId.empty())
	{
		strOutStatus = "Select one stable Valtan Pattern before preview.";
		return false;
	}
	if (!Stage_ValtanCompositionPreview(strOutStatus))
		return false;
	if (VALTAN_PATTERN_MASTER_ADMISSION_STATE::ADMITTED !=
		m_eValtanPatternMasterAdmission && !Reload_ValtanPatternMaster())
	{
		strOutStatus = m_strValtanPatternMasterStatus;
		return false;
	}
	const VALTAN_PATTERN_VIEW* const pPattern =
		Find_ValtanPatternMaster(m_ValtanPatternMasterView, strPatternId);
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pPattern || nullptr == pModel)
	{
		strOutStatus = nullptr == pPattern ?
			"The selected Pattern is absent from the admitted animation preview graph." :
			"The dedicated Valtan preview model disappeared before playback.";
		return false;
	}
	m_bValtanCompositionDraftPreviewReady = false;
	m_ValtanCompositionDraftPreview = {};
	m_eValtanPatternMasterPath = ePath;
	const bool_t bStarted = Start_ValtanPatternMasterPreview(
		pModel, *pPattern, ePath);
	strOutStatus = m_strValtanPatternMasterStatus;
	return bStarted;
}

bool_t Client::CAnimation_Tool::Play_ValtanCompositionDraftPattern(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	std::string& strOutStatus)
{
	if (Pattern.strPatternId.empty() || Pattern.Stages.empty())
	{
		strOutStatus =
			"The effective authoring draft has no stable Pattern/stage timeline.";
		return false;
	}
	if (!Stage_ValtanCompositionPreview(strOutStatus))
		return false;
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pModel)
	{
		strOutStatus =
			"The dedicated Valtan preview model disappeared before draft playback.";
		return false;
	}
	m_ValtanCompositionDraftPreview = Pattern;
	m_bValtanCompositionDraftPreviewReady = true;
	m_eValtanPatternMasterPath = ePath;
	const bool_t bStarted = Start_ValtanPatternMasterPreview(
		pModel, m_ValtanCompositionDraftPreview, ePath);
	if (!bStarted)
	{
		m_bValtanCompositionDraftPreviewReady = false;
		m_ValtanCompositionDraftPreview = {};
	}
	strOutStatus = m_strValtanPatternMasterStatus;
	return bStarted;
}

bool_t Client::CAnimation_Tool::Seek_ValtanCompositionPattern(
	const std::string& strPatternId,
	const uint32_t iPositionMs,
	const bool_t bPause,
	std::string& strOutStatus)
{
	const bool_t bSamePattern = m_bValtanPatternMasterPlaying &&
		!m_ValtanPatternMasterPlaylist.empty() &&
		m_ValtanPatternMasterPlaylist.front().strPatternId == strPatternId;
	if (!bSamePattern)
	{
		const bool_t bDraftMatches = m_bValtanCompositionDraftPreviewReady &&
			m_ValtanCompositionDraftPreview.strPatternId == strPatternId;
		const VALTAN_PATTERN_VIEW DraftCopy = bDraftMatches ?
			m_ValtanCompositionDraftPreview : VALTAN_PATTERN_VIEW{};
		const bool_t bStarted = bDraftMatches ?
			Play_ValtanCompositionDraftPattern(
				DraftCopy,
				VALTAN_PATTERN_PREVIEW_PATH::NORMAL, strOutStatus) :
			Play_ValtanCompositionPattern(
				strPatternId, VALTAN_PATTERN_PREVIEW_PATH::NORMAL, strOutStatus);
		if (!bStarted)
			return false;
	}
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pModel || !Seek_ValtanPatternMasterPreview(
			pModel, static_cast<f32_t>(iPositionMs) * 0.001f, bPause, true))
	{
		strOutStatus = m_strValtanPatternMasterStatus.empty() ?
			"Valtan composition preview seek was rejected." :
			m_strValtanPatternMasterStatus;
		return false;
	}
	strOutStatus = m_strValtanPatternMasterStatus;
	return true;
}

void Client::CAnimation_Tool::Stop_ValtanCompositionPattern(
	std::string& strOutStatus)
{
	if (const shared_ptr<Engine::CModel> pModel = Resolve_Model())
	{
		Stop_ValtanPatternMasterPreview(
			pModel, "Action Composition preview stopped.");
	}
	else
	{
		Reset_ValtanPatternMasterPreviewState(
			"Action Composition preview stopped after its Model View disappeared.");
	}
	strOutStatus = m_strValtanPatternMasterStatus;
}

Client::CAnimation_Tool::COMPOSITION_PREVIEW_STATE
Client::CAnimation_Tool::Get_ValtanCompositionPreviewState() const
{
	COMPOSITION_PREVIEW_STATE State;
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	const shared_ptr<CValtan> pBoss = CAnimationTargetService::Resolve_Boss();
	State.bModelReady = nullptr != pModel && nullptr != pBoss &&
		pBoss->Get_BodyModel() == pModel &&
		"Valtan" == CAnimationTargetService::Resolve_AssetName();
	State.bPlaying = m_bValtanPatternMasterPlaying;
	State.bPaused = m_bValtanPatternMasterPaused;
	State.iDurationMs = m_iValtanPatternMasterDurationMs;
	if (!m_ValtanPatternMasterPlaylist.empty())
		State.strPatternId = m_ValtanPatternMasterPlaylist.front().strPatternId;
	if (m_bValtanPatternMasterPlaying &&
		m_iValtanPatternMasterItem < m_ValtanPatternMasterPlaylist.size())
	{
		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
			m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
		const uint64_t iPositionMs =
			static_cast<uint64_t>(Item.iTimelineStartMs) +
			static_cast<uint64_t>(std::llround(
				static_cast<double>(m_fValtanPatternMasterItemElapsedSeconds) *
				1000.0));
		State.iPositionMs = static_cast<uint32_t>((std::min)(
			iPositionMs, static_cast<uint64_t>(State.iDurationMs)));
	}
	State.strStatus = m_strValtanPatternMasterStatus;
	return State;
}

bool_t Client::CAnimation_Tool::Get_ValtanCompositionSequences(
	std::vector<COMPOSITION_SEQUENCE_VIEW>& OutSequences,
	std::string& strOutStatus)
{
	OutSequences.clear();
	return Load_ValtanCompositionSequenceLibrary(
		OutSequences, strOutStatus);
}

bool_t Client::CAnimation_Tool::Resolve_ValtanCompositionNativeModel(
	shared_ptr<Engine::CModel>& pOutModel,
	std::string& strOutStatus) const
{
	pOutModel.reset();
	const std::string strAssetName =
		CAnimationTargetService::Resolve_AssetName();
	const shared_ptr<CValtan> pBoss = CAnimationTargetService::Resolve_Boss();
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if ("Valtan" != strAssetName || nullptr == pBoss || nullptr == pModel ||
		pBoss->Get_BodyModel() != pModel)
	{
		strOutStatus =
			"Native Animation authoring is blocked until the dedicated admitted Valtan Model View is staged; no unrelated scene model is substituted.";
		return false;
	}
	pOutModel = pModel;
	return true;
}

bool_t Client::CAnimation_Tool::Resolve_ValtanCompositionNativeClipDurationMs(
	const std::string& strClipName,
	uint32_t& iOutRoundedDurationMs,
	std::string& strOutStatus) const
{
	iOutRoundedDurationMs = 0u;
	shared_ptr<Engine::CModel> pModel;
	if (!Resolve_ValtanCompositionNativeModel(pModel, strOutStatus))
		return false;
	VALTAN_NATIVE_CLIP_INVENTORY Inventory;
	if (!BuildStrictValtanNativeClipInventory(
			pModel, Inventory, strOutStatus))
	{
		return false;
	}
	const auto Found = Inventory.find(strClipName);
	if (Found == Inventory.end())
	{
		strOutStatus =
			"The selected Sequence clip is absent from the admitted Valtan CModel: " +
			strClipName + ".";
		return false;
	}
	if (!CActionPresentationTimeline::Validate_AuthoredSourceWindow(
			Found->second.fDurationTicks,
			Found->second.fTicksPerSecond,
			0u, 0u, 1.f, iOutRoundedDurationMs) ||
		0u == iOutRoundedDurationMs)
	{
		strOutStatus =
			"The selected Sequence clip has no usable native duration: " +
			strClipName + ".";
		iOutRoundedDurationMs = 0u;
		return false;
	}
	strOutStatus = "Resolved native Valtan clip duration.";
	return true;
}

bool_t Client::CAnimation_Tool::
Validate_ValtanCompositionAnimationStageMutation(
	const VALTAN_STAGE_VIEW& BaselineStage,
	const VALTAN_STAGE_VIEW& CandidateStage,
	std::string& strOutStatus) const
{
	if (SameValtanAnimationAuthoringSignature(
			BaselineStage, CandidateStage))
	{
		strOutStatus = "Native Animation source windows are unchanged.";
		return true;
	}
	VALTAN_NATIVE_CLIP_INVENTORY Inventory;
	if (!CandidateStage.bSuppressAnimation &&
		"WAIT" != CandidateStage.strSequenceRole)
	{
		shared_ptr<Engine::CModel> pModel;
		if (!Resolve_ValtanCompositionNativeModel(pModel, strOutStatus) ||
			!BuildStrictValtanNativeClipInventory(
				pModel, Inventory, strOutStatus))
		{
			return false;
		}
	}
	if (!ValidateValtanStageNativeAnimationWindows(
			CandidateStage, Inventory, strOutStatus))
	{
		return false;
	}
	strOutStatus = "Native Animation Stage mutation admitted.";
	return true;
}

bool_t Client::CAnimation_Tool::
Validate_ValtanCompositionAnimationGraphMutations(
	const std::vector<VALTAN_PATTERN_VIEW>& BaselinePatterns,
	const std::vector<VALTAN_PATTERN_VIEW>& CandidatePatterns,
	std::string& strOutStatus) const
{
	VALTAN_NATIVE_CLIP_INVENTORY Inventory;
	bool_t bInventoryReady = false;
	for (const VALTAN_PATTERN_VIEW& CandidatePattern : CandidatePatterns)
	{
		const auto BaselinePattern = std::find_if(
			BaselinePatterns.begin(), BaselinePatterns.end(),
			[&CandidatePattern](const VALTAN_PATTERN_VIEW& Pattern)
			{
				return Pattern.strPatternId == CandidatePattern.strPatternId;
			});
		for (const VALTAN_STAGE_VIEW& CandidateStage : CandidatePattern.Stages)
		{
			const VALTAN_STAGE_VIEW* pBaselineStage = nullptr;
			if (BaselinePattern != BaselinePatterns.end())
			{
				const auto FoundStage = std::find_if(
					BaselinePattern->Stages.begin(), BaselinePattern->Stages.end(),
					[&CandidateStage](const VALTAN_STAGE_VIEW& Stage)
					{
						return Stage.strStageId == CandidateStage.strStageId;
					});
				if (FoundStage != BaselinePattern->Stages.end())
					pBaselineStage = &*FoundStage;
			}
			if (nullptr != pBaselineStage &&
				SameValtanAnimationAuthoringSignature(
					*pBaselineStage, CandidateStage))
			{
				continue;
			}
			if (!CandidateStage.bSuppressAnimation &&
				"WAIT" != CandidateStage.strSequenceRole &&
				!bInventoryReady)
			{
				shared_ptr<Engine::CModel> pModel;
				if (!Resolve_ValtanCompositionNativeModel(
						pModel, strOutStatus) ||
					!BuildStrictValtanNativeClipInventory(
						pModel, Inventory, strOutStatus))
				{
					return false;
				}
				bInventoryReady = true;
			}
			if (!ValidateValtanStageNativeAnimationWindows(
					CandidateStage, Inventory, strOutStatus))
			{
				strOutStatus = CandidatePattern.strPatternId + "/" +
					CandidateStage.strStageId + ": " + strOutStatus;
				return false;
			}
		}
	}
	strOutStatus = "All changed native Animation source windows are admitted.";
	return true;
}

bool_t Client::CAnimation_Tool::Preview_ValtanCompositionSequence(
	const int32_t iSkillId,
	const int32_t iSequenceIndex,
	std::string& strOutStatus)
{
	if (!Stage_ValtanCompositionPreview(strOutStatus))
		return false;
	if (!m_bClipSeqLoadAttempted)
	{
		m_bClipSeqLoadAttempted = true;
		(void)Load_ClipSeq();
	}
	const auto Found = std::find_if(
		m_ClipSeqs.begin(), m_ClipSeqs.end(),
		[iSkillId, iSequenceIndex](const CLIP_SEQ& Sequence)
		{
			return Sequence.iSkillId == iSkillId &&
				Sequence.iSeqIndex == iSequenceIndex;
		});
	if (Found == m_ClipSeqs.end())
	{
		strOutStatus = "The selected extracted Animation Sequence no longer exists.";
		return false;
	}
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pModel || !Start_ValtanSequencePreview(
			pModel, static_cast<std::size_t>(Found - m_ClipSeqs.begin())))
	{
		strOutStatus = "Could not preview the selected extracted Animation Sequence.";
		return false;
	}
	strOutStatus = "Previewing source Animation Sequence " +
		std::to_string(iSkillId) + "/" + std::to_string(iSequenceIndex) + ".";
	return true;
}

bool_t Client::CAnimation_Tool::Stage_ValtanCompositionIntakeSequence(
	const int32_t iSkillId,
	const int32_t iSequenceIndex,
	const std::string& strTargetPatternId,
	const std::string& strTargetStageId,
	std::string& strOutStatus)
{
	std::vector<COMPOSITION_SEQUENCE_VIEW> Sequences;
	if (!Get_ValtanCompositionSequences(Sequences, strOutStatus))
		return false;
	const auto Found = std::find_if(
		Sequences.begin(), Sequences.end(),
		[iSkillId, iSequenceIndex](const COMPOSITION_SEQUENCE_VIEW& Sequence)
		{
			return Sequence.iSkillId == iSkillId &&
				Sequence.iSequenceIndex == iSequenceIndex;
		});
	if (Found == Sequences.end() || Found->Clips.empty())
	{
		strOutStatus = "The selected Animation Sequence cannot stage an empty Intake.";
		return false;
	}
	m_CustomChainSteps.clear();
	m_CustomChainSteps.reserve(Found->Clips.size());
	for (const COMPOSITION_SEQUENCE_CLIP_VIEW& Clip : Found->Clips)
	{
		CUSTOM_CHAIN_STEP Step;
		Step.clipName = Clip.strClipName;
		Step.fDurationSeconds = static_cast<f32_t>(Clip.iDurationMs) * 0.001f;
		m_CustomChainSteps.push_back(std::move(Step));
	}
	const std::string strChainId = "sequence." + std::to_string(iSkillId) +
		"." + std::to_string(iSequenceIndex);
	snprintf(m_CustomChainId, sizeof(m_CustomChainId), "%s", strChainId.c_str());
	snprintf(m_CustomChainTargetPatternId,
		sizeof(m_CustomChainTargetPatternId), "%s", strTargetPatternId.c_str());
	snprintf(m_CustomChainTargetStageId,
		sizeof(m_CustomChainTargetStageId), "%s", strTargetStageId.c_str());
	m_iValtanPatternCreateSourceKind = 0;
	m_bValtanPatternCreateExactSourceSelection = true;
	m_iValtanPatternCreateSourceActionId = iSkillId;
	m_iValtanPatternCreateSourceSequenceIndex = iSequenceIndex;
	m_strValtanPatternCreateValidatedRequestSha256.clear();
	strOutStatus = "Staged " + std::to_string(m_CustomChainSteps.size()) +
		" Sequence clips in Create New Pattern Intake. Review the Pattern ID and Apply transaction below.";
	return true;
}

void Client::CAnimation_Tool::
Invalidate_ValtanPatternCreateExactSourceSelection()
{
	m_bValtanPatternCreateExactSourceSelection = false;
	m_iValtanPatternCreateSourceActionId = -1;
	m_iValtanPatternCreateSourceSequenceIndex = -1;
	m_strValtanPatternCreateValidatedRequestSha256.clear();
}

void Client::CAnimation_Tool::Set_ValtanCompositionLoop(const bool_t bLoop)
{
	m_bValtanCompositionLoop = bLoop;
}

bool_t Client::CAnimation_Tool::Consume_ValtanCompositionPatternCreated(
	std::string& strOutPatternId)
{
	if (!m_bValtanCompositionPatternCreatedPending)
		return false;
	strOutPatternId = std::move(m_strValtanCompositionPatternCreatedId);
	m_strValtanCompositionPatternCreatedId.clear();
	m_bValtanCompositionPatternCreatedPending = false;
	return !strOutPatternId.empty();
}

bool_t Client::CAnimation_Tool::Is_ValtanCompositionPatternTransactionActive() const
{
	return nullptr != m_hValtanPatternCreateProcess;
}

bool_t Client::CAnimation_Tool::Ensure_ValtanCompositionPatternSounds(
	std::string& strOutStatus)
{
	if (!m_bValtanPatternSoundCuesReady)
		(void)Reload_ValtanPatternSoundCues();
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return m_bValtanPatternSoundCuesReady;
}

bool_t Client::CAnimation_Tool::Reload_ValtanCompositionPatternSounds(
	std::string& strOutStatus)
{
	const bool_t bReloaded = Reload_ValtanPatternSoundCues();
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return bReloaded;
}

bool_t Client::CAnimation_Tool::
Can_CommitValtanCompositionPatternSoundGeneration(
	std::string& strOutStatus)
{
	if (nullptr == m_pBossTool)
	{
		strOutStatus =
			"Pattern Sound Save/reload/Apply requires the Boss playback owner boundary.";
		return false;
	}
	return m_pBossTool->Can_CommitPatternSoundGeneration(strOutStatus);
}

bool_t Client::CAnimation_Tool::Is_ValtanCompositionPatternSoundRuntimeReady(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus) const
{
	if (!ExpectedRevision.Is_Valid())
	{
		strOutStatus =
			"Pattern Sound runtime apply requires one valid Server-active Pattern revision.";
		return false;
	}
	if (m_bValtanPatternSoundCuesDirty)
	{
		strOutStatus =
			"Pattern Sound playback is blocked by an unsaved Sound owner draft.";
		return false;
	}
	if (!m_bValtanPatternSoundRuntimeApplyReady ||
		m_ValtanPatternSoundRuntimeAppliedRevision != ExpectedRevision)
	{
		strOutStatus = m_strValtanPatternSoundCueStatus.empty() ?
			"Pattern Sound active-consumer apply is pending for the exact Server-active Pattern revision." :
			m_strValtanPatternSoundCueStatus;
		return false;
	}
	strOutStatus =
		"Pattern Sound consumers are pinned to Server revision " +
		LostArk::Shared::Format_GameplayDataRevision(ExpectedRevision) + ".";
	return true;
}

bool_t Client::CAnimation_Tool::Apply_ValtanCompositionPatternSoundsToActiveConsumers(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus)
{
	if (!Can_CommitValtanCompositionPatternSoundGeneration(strOutStatus))
		return false;
	if (!ExpectedRevision.Is_Valid())
	{
		m_bValtanPatternSoundRuntimeApplyReady = false;
		m_ValtanPatternSoundRuntimeAppliedRevision = {};
		strOutStatus =
			"Pattern Sound consumer reload requires one valid revision receipt from Boss Tool.";
		return false;
	}
	std::string ArenaStatus =
		"No active Valtan Arena; Server Pattern playback admission cannot be established until the primary replicated Valtan consumer exists.";
	bool_t bArenaReloaded = false;
	if (CLevel_ValtanArena* const pArena = CLevel_ValtanArena::Get_Active())
	{
		bArenaReloaded =
			pArena->Reload_PrimaryValtanPresentationAuthoring(
				ExpectedRevision, ArenaStatus);
	}
	std::string PreviewStatus =
		"No Development preview Valtan is active.";
	bool_t bPreviewReloaded = true;
	if (const shared_ptr<CValtan> Boss =
			CAnimationTargetService::Resolve_Boss())
	{
		bPreviewReloaded =
			Boss->Reload_PatternPresentationAuthoring(PreviewStatus);
		if (!bPreviewReloaded)
		{
			PreviewStatus =
				"Development preview reload rejected: " + PreviewStatus;
		}
	}
	m_bValtanPatternSoundRuntimeApplyReady =
		bArenaReloaded && bPreviewReloaded;
	m_ValtanPatternSoundRuntimeAppliedRevision =
		m_bValtanPatternSoundRuntimeApplyReady ? ExpectedRevision :
			LostArk::Shared::GameplayDataRevision{};
	strOutStatus =
		(bArenaReloaded ? ArenaStatus :
			"ACTIVE ARENA RELOAD REJECTED: " + ArenaStatus) + " " +
		PreviewStatus +
		" Consumer reload receipt is provisional until Boss Tool revalidates the exact Server revision.";
	return m_bValtanPatternSoundRuntimeApplyReady;
}

bool_t Client::CAnimation_Tool::Retry_ValtanCompositionPatternSoundRuntimeApply(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus)
{
	if (!Can_CommitValtanCompositionPatternSoundGeneration(strOutStatus))
		return false;
	if (!ExpectedRevision.Is_Valid())
	{
		m_bValtanPatternSoundRuntimeApplyReady = false;
		m_ValtanPatternSoundRuntimeAppliedRevision = {};
		strOutStatus =
			"Pattern Sound runtime apply rejected because Boss Tool supplied no valid expected Pattern revision.";
		return false;
	}
	if (m_bValtanPatternSoundCuesDirty)
	{
		strOutStatus =
			"Pattern Sound runtime apply is blocked by an unsaved Sound owner draft.";
		return false;
	}
	if (!Reload_ValtanPatternSoundCues())
	{
		m_bValtanPatternSoundRuntimeApplyReady = false;
		m_ValtanPatternSoundRuntimeAppliedRevision = {};
		strOutStatus = m_strValtanPatternSoundCueStatus;
		return false;
	}
	std::string ApplyStatus;
	const bool_t bApplied =
		Apply_ValtanCompositionPatternSoundsToActiveConsumers(
			ExpectedRevision, ApplyStatus);
	m_strValtanPatternSoundCueStatus = bApplied ?
		"Pattern Sound source rejoined and applied to every active consumer at exact Server revision " +
			LostArk::Shared::Format_GameplayDataRevision(ExpectedRevision) + ". " +
			ApplyStatus :
		"Pattern Sound source rejoined, but one or more active consumers preserved their previous cache. Complete Play/Restart remain blocked until Retry Apply succeeds or the Arena is re-entered. " +
			ApplyStatus;
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return bApplied;
}

void Client::CAnimation_Tool::Invalidate_ValtanCompositionPatternSoundRuntimeApply(
	const std::string& strStatus)
{
	m_bValtanPatternSoundRuntimeApplyReady = false;
	m_ValtanPatternSoundRuntimeAppliedRevision = {};
	m_strValtanPatternSoundCueStatus = strStatus.empty() ?
		"Pattern Sound runtime receipt was invalidated before exact revision commit." :
		strStatus;
}

const Client::VALTAN_PATTERN_SOUND_CUE_DOCUMENT*
Client::CAnimation_Tool::Get_ValtanCompositionPatternSoundDraft(
	bool_t& bOutDirty,
	std::string& strOutStatus) const
{
	bOutDirty = m_bValtanPatternSoundCuesDirty;
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return m_bValtanPatternSoundCuesReady ?
		&m_ValtanPatternSoundCues : nullptr;
}

std::vector<std::string>
Client::CAnimation_Tool::Collect_ValtanCompositionPatternSoundEvents() const
{
	const std::vector<std::string> AllEvents =
		CSoundCueCatalog::Collect_EventNames("Valtan");
	std::vector<std::string> Admitted;
	Admitted.reserve(AllEvents.size());
	for (const std::string& Event : AllEvents)
	{
		if (IsValtanSoundAuthoringCandidate(Event))
			Admitted.push_back(Event);
	}
	return Admitted;
}

bool_t Client::CAnimation_Tool::Resolve_ValtanCompositionPatternSoundWindow(
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	uint32_t& iOutMinimumStartMs,
	uint32_t& iOutMaximumStartMs,
	bool_t& bOutLoop,
	std::string& strOutStatus) const
{
	const auto Clip = std::find_if(
		Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
		[&strClipOccurrenceId](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
		{
			return Candidate.strClipOccurrenceId == strClipOccurrenceId;
		});
	if (Stage.ClipOccurrences.end() == Clip || 0u == Stage.iDurationMs)
	{
		strOutStatus =
			"Pattern Sound occurrence does not resolve one admitted clip/stage wall.";
		return false;
	}

	const std::unordered_map<std::string, f32_t> Durations =
		CollectModelClipSourceDurationSeconds(Resolve_Model());
	if (Durations.empty())
	{
		strOutStatus =
			"Pattern Sound timing edit requires the admitted Valtan model source durations.";
		return false;
	}

	std::vector<ACTION_PRESENTATION_CLIP_TIMING> Timings;
	Timings.reserve(Stage.ClipOccurrences.size());
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
		Stage.ClipOccurrences)
	{
		const auto Duration = Durations.find(Occurrence.strClipName);
		if (Durations.end() == Duration)
		{
			strOutStatus = "Pattern Sound timing edit is missing model clip: " +
				Occurrence.strClipName + ".";
			return false;
		}
		ACTION_PRESENTATION_CLIP_TIMING Timing{
			Duration->second,
			Occurrence.iPlayMs,
			Occurrence.fPlayRate,
			Occurrence.bLoop,
			static_cast<f32_t>(Occurrence.iSourceStartMs) * 0.001f };
		f32_t fSourceDurationSeconds = 0.f;
		f32_t fWallDurationSeconds = 0.f;
		if (!CActionPresentationTimeline::Resolve_ClipDuration(
				Timing, fSourceDurationSeconds, fWallDurationSeconds))
		{
			strOutStatus = "Pattern Sound timing edit rejected invalid clip timing: " +
				Occurrence.strClipOccurrenceId + ".";
			return false;
		}
		Timings.push_back(Timing);
	}

	const std::size_t iClipIndex = static_cast<std::size_t>(
		Clip - Stage.ClipOccurrences.begin());
	f32_t fResolvedSourceDurationSeconds = 0.f;
	f32_t fResolvedWallDurationSeconds = 0.f;
	f32_t fClipStageWallStartSeconds = 0.f;
	if (!CActionPresentationTimeline::Resolve_ClipDuration(
			Timings[iClipIndex], fResolvedSourceDurationSeconds,
			fResolvedWallDurationSeconds) ||
		!CActionPresentationTimeline::Resolve_CueWallOffset(
			Timings, iClipIndex,
			Timings[iClipIndex].fSourceStartSeconds,
			0u, fClipStageWallStartSeconds))
	{
		strOutStatus =
			"Pattern Sound timing edit could not resolve the clip on the Stage wall.";
		return false;
	}

	const f64_t fRemainingStageWallSeconds = (std::max)(0.0,
		static_cast<f64_t>(Stage.iDurationMs) * 0.001 -
		static_cast<f64_t>(fClipStageWallStartSeconds));
	const f64_t fResolvedSourceEndSeconds =
		static_cast<f64_t>(Clip->iSourceStartMs) * 0.001 +
		static_cast<f64_t>(fResolvedSourceDurationSeconds);
	const f64_t fStageSourceEndSeconds =
		static_cast<f64_t>(Clip->iSourceStartMs) * 0.001 +
		fRemainingStageWallSeconds * static_cast<f64_t>(Clip->fPlayRate);
	const f64_t fEffectiveSourceEndMilliseconds = 1000.0 *
		(std::min)(fResolvedSourceEndSeconds, fStageSourceEndSeconds);
	if (!std::isfinite(fEffectiveSourceEndMilliseconds) ||
		fEffectiveSourceEndMilliseconds <=
			static_cast<f64_t>(Clip->iSourceStartMs))
	{
		strOutStatus =
			"Pattern Sound timing edit has no source sample inside the Stage wall.";
		return false;
	}
	const std::uint64_t iSourceEndExclusiveMs =
		static_cast<std::uint64_t>(std::ceil(fEffectiveSourceEndMilliseconds));
	if (iSourceEndExclusiveMs <= Clip->iSourceStartMs)
	{
		strOutStatus =
			"Pattern Sound timing edit resolved an empty source window.";
		return false;
	}

	iOutMinimumStartMs = Clip->iSourceStartMs;
	iOutMaximumStartMs = static_cast<uint32_t>((std::min)(
		iSourceEndExclusiveMs - 1u,
		static_cast<std::uint64_t>(
			(std::numeric_limits<uint32_t>::max)())));
	bOutLoop = Clip->bLoop;
	strOutStatus = "Resolved Pattern Sound source window " +
		std::to_string(iOutMinimumStartMs) + ".." +
		std::to_string(iOutMaximumStartMs) + " ms.";
	return true;
}

bool_t Client::CAnimation_Tool::
Validate_ValtanCompositionPatternSoundStageDependencies(
	const VALTAN_PATTERN_VIEW& BaselinePattern,
	const VALTAN_STAGE_VIEW& BaselineStage,
	const VALTAN_STAGE_VIEW& CandidateStage,
	std::string& strOutStatus) const
{
	if (!m_bValtanPatternSoundCuesReady)
	{
		strOutStatus =
			"Pattern/Animation mutation requires the admitted Pattern Sound source before dependency validation.";
		return false;
	}
	if (m_bValtanPatternSoundCuesDirty)
	{
		strOutStatus =
			"Pattern/Animation mutation is blocked while the separate Pattern Sound source has an unsaved draft.";
		return false;
	}
	if (BaselinePattern.strPatternId.empty() ||
		BaselineStage.strStageId.empty() ||
		BaselineStage.strStageId != CandidateStage.strStageId)
	{
		strOutStatus =
			"Pattern Sound dependency admission requires one exact baseline/candidate Pattern and Stage identity.";
		return false;
	}

	std::vector<const VALTAN_PATTERN_SOUND_CUE*> Rows;
	for (const VALTAN_PATTERN_SOUND_CUE& Cue : m_ValtanPatternSoundCues.Cues)
	{
		if (Cue.strPatternId == BaselinePattern.strPatternId &&
			Cue.strStageId == BaselineStage.strStageId)
		{
			Rows.push_back(&Cue);
		}
	}
	/* Do not manufacture a model requirement for a Stage that owns no Sound
	   dependency.  The loaded source inventory above is still mandatory so an
	   unavailable owner cannot be mistaken for an empty Stage. */
	if (Rows.empty())
	{
		strOutStatus = "No Pattern Sound dependency is attached to this Stage.";
		return true;
	}

	for (const VALTAN_PATTERN_SOUND_CUE* const pCue : Rows)
	{
		const VALTAN_PATTERN_SOUND_CUE& Cue = *pCue;
		if (Cue.strActionId != BaselineStage.strActionId ||
			Cue.strActionId != CandidateStage.strActionId)
		{
			strOutStatus =
				"Pattern Sound row no longer resolves the exact Pattern/Stage/action tuple: " +
				Cue.strOccurrenceId + ".";
			return false;
		}
		const std::size_t iBaselineClipCount = static_cast<std::size_t>(
			std::count_if(
				BaselineStage.ClipOccurrences.begin(),
				BaselineStage.ClipOccurrences.end(),
				[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
				{
					return Clip.strClipOccurrenceId == Cue.strClipOccurrenceId;
				}));
		const std::size_t iCandidateClipCount = static_cast<std::size_t>(
			std::count_if(
				CandidateStage.ClipOccurrences.begin(),
				CandidateStage.ClipOccurrences.end(),
				[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
				{
					return Clip.strClipOccurrenceId == Cue.strClipOccurrenceId;
				}));
		const auto BaselineClip = std::find_if(
			BaselineStage.ClipOccurrences.begin(),
			BaselineStage.ClipOccurrences.end(),
			[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId == Cue.strClipOccurrenceId;
			});
		const auto CandidateClip = std::find_if(
			CandidateStage.ClipOccurrences.begin(),
			CandidateStage.ClipOccurrences.end(),
			[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId == Cue.strClipOccurrenceId;
			});
		if (1u != iBaselineClipCount || 1u != iCandidateClipCount ||
			BaselineStage.ClipOccurrences.end() == BaselineClip ||
			CandidateStage.ClipOccurrences.end() == CandidateClip)
		{
			strOutStatus =
				"Pattern Sound row would dangle or become ambiguous because its clipOccurrenceId does not resolve exactly once: " +
				Cue.strClipOccurrenceId + ".";
			return false;
		}
		if (BaselineClip->strClipName != CandidateClip->strClipName)
		{
			strOutStatus =
				"Pattern Sound-qualified clipOccurrenceId cannot be reused for another clip; remove or retarget the Sound row explicitly first: " +
				Cue.strClipOccurrenceId + ".";
			return false;
		}

		uint32_t iMinimumStartMs = 0u;
		uint32_t iMaximumStartMs = 0u;
		bool_t bLoop = false;
		if (!Resolve_ValtanCompositionPatternSoundWindow(
				CandidateStage, Cue.strClipOccurrenceId,
				iMinimumStartMs, iMaximumStartMs, bLoop, strOutStatus))
		{
			strOutStatus =
				"Pattern Sound dependency timing rejected " +
				Cue.strOccurrenceId + ": " + strOutStatus;
			return false;
		}
		if (Cue.iStartMs < iMinimumStartMs ||
			Cue.iStartMs > iMaximumStartMs ||
			(VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				Cue.eRepeatPolicy && !bLoop))
		{
			strOutStatus =
				"Pattern Sound row is outside the candidate clip/Stage window or keeps each_loop on a non-loop clip: " +
				Cue.strOccurrenceId + ".";
			return false;
		}
	}
	strOutStatus = "Validated " + std::to_string(Rows.size()) +
		" Pattern Sound dependency row(s) against the candidate Stage.";
	return true;
}

bool_t Client::CAnimation_Tool::
Validate_ValtanCompositionPatternSoundGraphDependencies(
	const std::vector<VALTAN_PATTERN_VIEW>& BaselinePatterns,
	const std::vector<VALTAN_PATTERN_VIEW>& CandidatePatterns,
	std::string& strOutStatus) const
{
	if (!m_bValtanPatternSoundCuesReady)
	{
		strOutStatus =
			"Canonical Save requires the admitted Pattern Sound source before dependency validation.";
		return false;
	}
	if (m_bValtanPatternSoundCuesDirty)
	{
		strOutStatus =
			"Canonical Save is blocked while the separate Pattern Sound source has an unsaved draft.";
		return false;
	}
	if (m_ValtanPatternSoundCues.Cues.empty())
	{
		strOutStatus = "The admitted Pattern Sound source has no dependency rows.";
		return true;
	}

	const auto FindUniquePattern = [](
		const std::vector<VALTAN_PATTERN_VIEW>& Patterns,
		const std::string& strPatternId,
		const VALTAN_PATTERN_VIEW*& pOutPattern)
	{
		pOutPattern = nullptr;
		for (const VALTAN_PATTERN_VIEW& Pattern : Patterns)
		{
			if (Pattern.strPatternId != strPatternId)
				continue;
			if (nullptr != pOutPattern)
				return false;
			pOutPattern = &Pattern;
		}
		return nullptr != pOutPattern;
	};
	const auto FindUniqueStage = [](
		const VALTAN_PATTERN_VIEW& Pattern,
		const std::string& strStageId,
		const VALTAN_STAGE_VIEW*& pOutStage)
	{
		pOutStage = nullptr;
		for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
		{
			if (Stage.strStageId != strStageId)
				continue;
			if (nullptr != pOutStage)
				return false;
			pOutStage = &Stage;
		}
		return nullptr != pOutStage;
	};

	std::unordered_set<std::string> ValidatedStages;
	for (const VALTAN_PATTERN_SOUND_CUE& Cue : m_ValtanPatternSoundCues.Cues)
	{
		const std::string StageKey = Cue.strPatternId + "/" + Cue.strStageId;
		if (!ValidatedStages.insert(StageKey).second)
			continue;

		const VALTAN_PATTERN_VIEW* pBaselinePattern = nullptr;
		const VALTAN_PATTERN_VIEW* pCandidatePattern = nullptr;
		const VALTAN_STAGE_VIEW* pBaselineStage = nullptr;
		const VALTAN_STAGE_VIEW* pCandidateStage = nullptr;
		if (!FindUniquePattern(
				BaselinePatterns, Cue.strPatternId, pBaselinePattern) ||
			!FindUniquePattern(
				CandidatePatterns, Cue.strPatternId, pCandidatePattern) ||
			!FindUniqueStage(*pBaselinePattern, Cue.strStageId, pBaselineStage) ||
			!FindUniqueStage(*pCandidatePattern, Cue.strStageId, pCandidateStage))
		{
			strOutStatus =
				"Pattern Sound dependency does not resolve exactly one candidate Pattern/Stage: " +
				StageKey + ".";
			return false;
		}
		if (!Validate_ValtanCompositionPatternSoundStageDependencies(
				*pBaselinePattern, *pBaselineStage, *pCandidateStage,
				strOutStatus))
		{
			return false;
		}
	}
	strOutStatus = "Validated all " +
		std::to_string(m_ValtanPatternSoundCues.Cues.size()) +
		" Pattern Sound row(s) against the complete candidate Pattern graph.";
	return true;
}

bool_t Client::CAnimation_Tool::Patch_ValtanCompositionPatternSound(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strOccurrenceId,
	const std::string& strSoundEvent,
	const uint32_t iStartMs,
	const VALTAN_PATTERN_SOUND_REPEAT_POLICY eRepeatPolicy,
	std::string& strOutStatus)
{
	std::string AuthoringRevision;
	std::string AuthoringStatus;
	bool_t bCanonicalDraftDirty = false;
	if (Is_ValtanCompositionPatternTransactionActive() ||
		nullptr == m_pBalanceTool ||
		!m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bCanonicalDraftDirty, AuthoringStatus) ||
		bCanonicalDraftDirty)
	{
		strOutStatus = Is_ValtanCompositionPatternTransactionActive() ?
			"Pattern Sound patch is blocked while Create New Pattern owns the Pattern dependency transaction." :
			"Pattern Sound patch requires a clean admitted Pattern/Animation source generation: " +
				AuthoringStatus;
		return false;
	}
	if (!m_bValtanPatternSoundCuesReady || strOccurrenceId.empty() ||
		(eRepeatPolicy != VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE &&
		 eRepeatPolicy != VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP))
	{
		strOutStatus =
			"Pattern Sound patch requires one admitted stable occurrence.";
		return false;
	}
	const std::vector<std::string> Events =
		Collect_ValtanCompositionPatternSoundEvents();
	if (Events.end() == std::find(
			Events.begin(), Events.end(), strSoundEvent))
	{
		strOutStatus =
			"Pattern Sound patch rejected an event without admitted Valtan assets.";
		return false;
	}

	uint32_t iMinimumStartMs = 0u;
	uint32_t iMaximumStartMs = 0u;
	bool_t bLoop = false;
	const auto Current = std::find_if(
		m_ValtanPatternSoundCues.Cues.begin(),
		m_ValtanPatternSoundCues.Cues.end(),
		[&](const VALTAN_PATTERN_SOUND_CUE& Cue)
		{
			return Cue.strOccurrenceId == strOccurrenceId &&
				Cue.strPatternId == Pattern.strPatternId &&
				Cue.strStageId == Stage.strStageId &&
				Cue.strActionId == Stage.strActionId;
		});
	if (m_ValtanPatternSoundCues.Cues.end() == Current ||
		!Resolve_ValtanCompositionPatternSoundWindow(
			Stage, Current->strClipOccurrenceId,
			iMinimumStartMs, iMaximumStartMs, bLoop, strOutStatus))
	{
		if (m_ValtanPatternSoundCues.Cues.end() == Current)
			strOutStatus =
				"Pattern Sound patch did not resolve the exact selected tuple.";
		return false;
	}
	if (iStartMs < iMinimumStartMs || iStartMs > iMaximumStartMs ||
		(VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP == eRepeatPolicy &&
		 !bLoop))
	{
		strOutStatus =
			"Pattern Sound patch is outside the model/Stage source window or requests each_loop on a non-loop clip.";
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged = m_ValtanPatternSoundCues;
	VALTAN_PATTERN_SOUND_CUE& Candidate = Staged.Cues[
		static_cast<std::size_t>(Current - m_ValtanPatternSoundCues.Cues.begin())];
	Candidate.strSoundEvent = strSoundEvent;
	Candidate.strSoundBank = std::string(
		ValtanSoundBankForEvent(strSoundEvent));
	Candidate.iStartMs = iStartMs;
	Candidate.eRepeatPolicy = eRepeatPolicy;
	if (Candidate == *Current)
	{
		strOutStatus = "Pattern Sound draft is unchanged.";
		return true;
	}
	m_ValtanPatternSoundCues = std::move(Staged);
	m_bValtanPatternSoundCuesDirty = true;
	++m_iValtanPatternSoundDraftGeneration;
	m_strValtanPatternSoundCueStatus =
		"UNSAVED Pattern Sound occurrence: " + strOccurrenceId + ".";
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Add_ValtanCompositionPatternSound(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	const std::string& strSoundEvent,
	const uint32_t iStartMs,
	const VALTAN_PATTERN_SOUND_REPEAT_POLICY eRepeatPolicy,
	VALTAN_PATTERN_SOUND_CUE_ROW_ID& OutCreatedRowId,
	std::string& strOutStatus)
{
	std::string AuthoringRevision;
	std::string AuthoringStatus;
	bool_t bCanonicalDraftDirty = false;
	if (Is_ValtanCompositionPatternTransactionActive() ||
		nullptr == m_pBalanceTool ||
		!m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bCanonicalDraftDirty, AuthoringStatus) ||
		bCanonicalDraftDirty)
	{
		strOutStatus = Is_ValtanCompositionPatternTransactionActive() ?
			"Pattern Sound Add is blocked while Create New Pattern owns the Pattern dependency transaction." :
			"Pattern Sound Add requires a clean admitted Pattern/Animation source generation: " +
				AuthoringStatus;
		return false;
	}
	if (!m_bValtanPatternSoundCuesReady ||
		Pattern.strPatternId.empty() || Stage.strStageId.empty() ||
		Stage.strActionId.empty() || strClipOccurrenceId.empty() ||
		(eRepeatPolicy != VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE &&
		 eRepeatPolicy != VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP))
	{
		strOutStatus =
			"Pattern Sound Add requires one admitted Pattern/Stage/clip occurrence.";
		return false;
	}
	const std::vector<std::string> Events =
		Collect_ValtanCompositionPatternSoundEvents();
	if (Events.end() == std::find(
			Events.begin(), Events.end(), strSoundEvent))
	{
		strOutStatus =
			"Pattern Sound Add rejected an event without admitted Valtan assets.";
		return false;
	}

	uint32_t iMinimumStartMs = 0u;
	uint32_t iMaximumStartMs = 0u;
	bool_t bLoop = false;
	if (!Resolve_ValtanCompositionPatternSoundWindow(
			Stage, strClipOccurrenceId, iMinimumStartMs, iMaximumStartMs,
			bLoop, strOutStatus))
	{
		return false;
	}
	if (iStartMs < iMinimumStartMs || iStartMs > iMaximumStartMs ||
		(VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP == eRepeatPolicy &&
		 !bLoop))
	{
		strOutStatus =
			"Pattern Sound Add is outside the selected clip source window or requests each_loop on a non-loop clip.";
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE_ADD_ROW Row;
	Row.strPatternId = Pattern.strPatternId;
	Row.strStageId = Stage.strStageId;
	Row.strActionId = Stage.strActionId;
	Row.strClipOccurrenceId = strClipOccurrenceId;
	Row.strSoundEvent = strSoundEvent;
	Row.strSoundBank = std::string(ValtanSoundBankForEvent(strSoundEvent));
	Row.eRepeatPolicy = eRepeatPolicy;
	Row.iStartMs = iStartMs;
	const std::unordered_map<std::string, f32_t> Durations =
		CollectModelClipSourceDurationSeconds(Resolve_Model());
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged = m_ValtanPatternSoundCues;
	VALTAN_PATTERN_SOUND_CUE_ROW_ID CreatedRowId;
	std::string AddStatus;
	if (!CValtanPatternSoundCueDocument::Add_AuthoringRow(
			Staged, Row, Durations, CreatedRowId, AddStatus))
	{
		strOutStatus =
			"Pattern Sound Add rejected; admitted draft preserved: " + AddStatus;
		return false;
	}
	m_ValtanPatternSoundCues = std::move(Staged);
	m_bValtanPatternSoundCuesDirty = true;
	++m_iValtanPatternSoundDraftGeneration;
	OutCreatedRowId = CreatedRowId;
	m_strValtanPatternSoundCueStatus =
		"UNSAVED Pattern Sound row added: " + CreatedRowId.strBindingId +
		" / " + CreatedRowId.strOccurrenceId + ". " + AddStatus;
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Remove_ValtanCompositionPatternSound(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_PATTERN_SOUND_CUE_ROW_ID& RowId,
	std::string& strOutStatus)
{
	std::string AuthoringRevision;
	std::string AuthoringStatus;
	bool_t bCanonicalDraftDirty = false;
	if (Is_ValtanCompositionPatternTransactionActive() ||
		nullptr == m_pBalanceTool ||
		!m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bCanonicalDraftDirty, AuthoringStatus) ||
		bCanonicalDraftDirty)
	{
		strOutStatus = Is_ValtanCompositionPatternTransactionActive() ?
			"Pattern Sound Remove is blocked while Create New Pattern owns the Pattern dependency transaction." :
			"Pattern Sound Remove requires a clean admitted Pattern/Animation source generation: " +
				AuthoringStatus;
		return false;
	}
	if (!m_bValtanPatternSoundCuesReady)
	{
		strOutStatus =
			"Pattern Sound Remove requires the admitted typed source draft.";
		return false;
	}
	const auto Exact = std::find_if(
		m_ValtanPatternSoundCues.Cues.begin(),
		m_ValtanPatternSoundCues.Cues.end(),
		[&](const VALTAN_PATTERN_SOUND_CUE& Cue)
		{
			return Cue.strBindingId == RowId.strBindingId &&
				Cue.strOccurrenceId == RowId.strOccurrenceId &&
				Cue.strPatternId == Pattern.strPatternId &&
				Cue.strStageId == Stage.strStageId &&
				Cue.strActionId == Stage.strActionId;
		});
	if (m_ValtanPatternSoundCues.Cues.end() == Exact)
	{
		strOutStatus =
			"Pattern Sound Remove did not resolve the exact selected Pattern/Stage row.";
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged = m_ValtanPatternSoundCues;
	std::string RemoveStatus;
	if (!CValtanPatternSoundCueDocument::Remove_AuthoringRow(
			Staged, RowId, RemoveStatus))
	{
		strOutStatus =
			"Pattern Sound Remove rejected; admitted draft preserved: " +
			RemoveStatus;
		return false;
	}
	m_ValtanPatternSoundCues = std::move(Staged);
	m_bValtanPatternSoundCuesDirty = true;
	++m_iValtanPatternSoundDraftGeneration;
	m_strValtanPatternSoundCueStatus =
		"UNSAVED Pattern Sound row removed: " + RowId.strBindingId +
		" / " + RowId.strOccurrenceId + ". " + RemoveStatus;
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Save_ValtanCompositionPatternSounds(
	std::string& strOutStatus)
{
	if (!Can_CommitValtanCompositionPatternSoundGeneration(strOutStatus))
		return false;
	std::string AuthoringRevision;
	std::string AuthoringStatus;
	bool_t bCanonicalDraftDirty = false;
	if (Is_ValtanCompositionPatternTransactionActive() ||
		nullptr == m_pBalanceTool ||
		!m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bCanonicalDraftDirty, AuthoringStatus) ||
		bCanonicalDraftDirty)
	{
		strOutStatus = Is_ValtanCompositionPatternTransactionActive() ?
			"Pattern Sound Save is blocked while Create New Pattern owns the Pattern dependency transaction." :
			"Pattern Sound Save requires a clean admitted Pattern/Animation source generation: " +
				AuthoringStatus;
		return false;
	}
	if (!m_bValtanPatternSoundCuesReady)
	{
		strOutStatus =
			"Pattern Sound Save requires the admitted typed source draft.";
		return false;
	}
	if (!m_bValtanPatternSoundCuesDirty)
	{
		strOutStatus = "Pattern Sound source has no staged changes.";
		return true;
	}
	const std::unordered_map<std::string, f32_t> Durations =
		CollectModelClipSourceDurationSeconds(Resolve_Model());
	std::string SaveStatus;
	if (!CValtanPatternSoundCueDocument::Save_Atomic(
			m_ValtanPatternSoundCues, Durations,
			m_strValtanPatternSoundCueBaselineSourceBytes, SaveStatus))
	{
		m_strValtanPatternSoundCueStatus =
			"Pattern Sound Save rejected; source and admitted draft were preserved: " +
			SaveStatus;
		strOutStatus = m_strValtanPatternSoundCueStatus;
		return false;
	}

	m_bValtanPatternSoundCuesDirty = false;
	const bool_t bDraftReloaded = Reload_ValtanPatternSoundCues();
	const std::string DraftStatus = m_strValtanPatternSoundCueStatus;
	if (!bDraftReloaded)
	{
		m_bValtanPatternSoundRuntimeApplyReady = false;
		m_ValtanPatternSoundRuntimeAppliedRevision = {};
	}
	m_strValtanPatternSoundCueStatus =
		"Pattern Sound SOURCE SAVED (separate from Pattern Save/Publish). " +
		SaveStatus + " " +
		(bDraftReloaded ? DraftStatus :
			"WORKBENCH RELOAD REJECTED; saved source remains committed: " +
			DraftStatus) + " " +
		"ACTIVE CONSUMER APPLY DEFERRED; Complete Play/Restart remain blocked until the Workbench observes the exact Server-active Pattern revision and Retry Apply succeeds.";
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return bDraftReloaded;
}

void Client::CAnimation_Tool::Render_ValtanCompositionPatternCreator()
{
	if (!m_bCustomChainLibraryLoadAttempted)
	{
		m_bCustomChainLibraryLoadAttempted = true;
		(void)Load_CustomChainLibrary();
	}
	if (!m_bValtanPatternMasterLoadAttempted)
	{
		/* This tab is rendered every frame.  Latch the command edge before the
		   file-backed canonical reload so a rejected load is not retried at the
		   render rate. */
		m_bValtanPatternMasterLoadAttempted = true;
		(void)Reload_ValtanPatternMaster();
	}
	Render_ValtanPatternCreatePanel();
}

bool_t Client::CAnimation_Tool::Open_KakulProfile(
	const std::string& profileId)
{
	const KAKUL_ACTION_PROFILE_CONTRACT* pProfile =
		Find_KakulActionProfile(profileId);
	const bool_t bClipOnlyDonor = "MN_RPCT_00" == profileId;
	if (nullptr == pProfile && !bClipOnlyDonor)
	{
		m_Status = "KoukuSaton action profile is not admitted: " + profileId;
		return false;
	}
	const char_t* pPreviewAsset = bClipOnlyDonor ?
		"MN_RPCT_00" : pProfile->pPreviewAssetName;

	if (Is_AnyDocumentDirty() && !m_strKakulProfileId.empty() &&
		m_strKakulProfileId != profileId)
	{
		m_Status =
			"Save or discard the current Animation document before opening another KoukuSaton profile.";
		return false;
	}
	if (!m_pPreviewPanel->Select_TargetAsset(pPreviewAsset))
	{
		m_Status = "KoukuSaton profile selected, but its physical preview body could not open: " +
			m_pPreviewPanel->Get_Status();
		return false;
	}

	const bool_t bProfileChanged = m_strKakulProfileId != profileId;
	if (bProfileChanged)
		Reset_KakulActionDocumentState(true);
	m_bValtanDataWorkspaceRequested = false;
	m_strKakulProfileId = profileId;
	m_Status = bClipOnlyDonor ?
		"Opened MN_RPCT_00 as a local clip donor preview; action profiles 05/07 consume this clip vocabulary." :
		("Opened KoukuSaton extracted action profile " + profileId +
		 " as a local REFERENCE_ONLY preview.");
	return true;
}

bool_t Client::CAnimation_Tool::Sync_AssetName()
{
	const string assetName =
		CAnimationTargetService::Resolve_AssetName();
	if (assetName.empty())
	{
		if (Is_AnyDocumentDirty())
		{
			m_PendingAssetName.clear();
			m_Status =
				"Animation target disappeared while an authored document is dirty. "
				"The unsaved document is preserved.";
			return false;
		}
		return true;
	}
	if (m_AssetName == assetName)
	{
		m_PendingAssetName.clear();
		return true;
	}
	if (Is_AnyDocumentDirty() && !m_AssetName.empty())
	{
		m_PendingAssetName = assetName;
		m_Status =
			"Target changed while an Animation document is dirty. "
			"Return to the original target and Save, or discard explicitly.";
		return false;
	}

	Adopt_AssetName(assetName);
	return true;
}

void Client::CAnimation_Tool::Adopt_AssetName(
	const std::string& assetName)
{
	/* A different class means every loaded file belongs to the wrong asset. Drop
	them rather than mixing two classes' clips in one list. */
	m_AssetName = assetName;
	m_bValtanDataWorkspaceRequested = "Valtan" == assetName;
	std::string strNextKakulProfile;
	if (const KAKUL_ACTION_PROFILE_CONTRACT* pCurrent =
			Find_KakulActionProfile(m_strKakulProfileId);
		nullptr != pCurrent && assetName == pCurrent->pPreviewAssetName)
	{
		strNextKakulProfile = pCurrent->pProfileId;
	}
	else if (const KAKUL_ACTION_PROFILE_CONTRACT* pDefault =
			Default_KakulActionProfileForAsset(assetName))
	{
		strNextKakulProfile = pDefault->pProfileId;
	}
	else if ("MN_RPCT_00" == assetName)
	{
		strNextKakulProfile = "MN_RPCT_00";
	}
	Reset_KakulActionDocumentState(true);
	m_strKakulProfileId = std::move(strNextKakulProfile);
	m_PendingAssetName.clear();
	m_Events.clear();
	m_SkillRef.clear();
	m_ClipMap.clear();
	m_ClipNotify.clear();
	m_ClipLength.clear();
	m_ClipSeqs.clear();
	m_ClipChainCounts.clear();
	m_DuplicateBodyClips.clear();
	m_bDuplicateScanDone = false;
	m_SkillBindingDocument = {};
	m_iSelectedEvent = -1;
	m_iRefWireSkillId = 0;
	m_iRefWireHitIndex = -1;
	m_bDirty = false;
	m_bSkillBindingDirty = false;
	m_iSelectedSkillBinding = -1;
	m_iSelectedSkillClip = 0;
	m_SkillBindingStatus.clear();
	m_bLoadAttempted = false;
	m_bRefLoadAttempted = false;
	m_bClipMapLoadAttempted = false;
	m_bClipNotifyLoadAttempted = false;
	m_bClipSeqLoadAttempted = false;
	m_bSkillBindingLoadAttempted = false;
	m_ValtanPatternMasterView = {};
	m_ValtanPatternMasterPlaylist.clear();
	m_bValtanPatternMasterLoadAttempted = false;
	m_eValtanPatternMasterAdmission =
		VALTAN_PATTERN_MASTER_ADMISSION_STATE::UNLOADED;
	m_bValtanPatternMasterPlaying = false;
	m_bValtanPatternMasterPaused = false;
	m_bShowValtanSourceReferenceWindow = false;
	m_eValtanWorkbenchSelection =
		VALTAN_WORKBENCH_SELECTION_KIND::STAGE;
	m_eValtanWorkbenchDetailOwner =
		VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY;
	m_bValtanWorkbenchFocusDetailRequested = false;
	m_bValtanWorkspaceTabInitialized = false;
	m_strValtanWorkbenchPatternId.clear();
	m_strValtanWorkbenchStageId.clear();
	m_ValtanPatternAnimationBindingDraft = {};
	m_bValtanPatternAnimationBindingLoadAttempted = false;
	m_bValtanPatternAnimationBindingReady = false;
	m_bValtanPatternAnimationBindingDirty = false;
	m_strValtanPatternAnimationBindingBaselineSourceBytes.clear();
	m_strValtanPatternAnimationBindingStatus.clear();
	m_strValtanAnimationBindingDetailActionId.clear();
	m_strValtanAnimationBindingNewClip.clear();
	m_ValtanPatternSoundCues = {};
	m_bValtanPatternSoundCuesReady = false;
	m_bValtanPatternSoundCuesDirty = false;
	m_iValtanPatternSoundDraftGeneration = 0u;
	m_bValtanPatternSoundRuntimeApplyReady = false;
	m_ValtanPatternSoundRuntimeAppliedRevision = {};
	m_strValtanPatternSoundCueBaselineSourceBytes.clear();
	m_strValtanPatternSoundCueStatus.clear();
	m_strValtanPatternSoundAddClipOccurrenceId.clear();
	m_strValtanPatternSoundAddEvent.clear();
	m_iValtanPatternSoundAddStartMs = 0u;
	m_eValtanPatternSoundAddRepeatPolicy =
		VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
	m_iValtanPatternMasterSelected = 0;
	m_iValtanPatternMasterItem = 0u;
	m_fValtanPatternMasterItemElapsedSeconds = 0.f;
	m_iValtanPatternMasterDurationMs = 0u;
	m_eValtanPatternMasterPath = VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	m_strValtanPatternMasterStatus.clear();
	m_hasCameraToolOpenRequest = false;
	m_strCameraToolOpenCueId.clear();
	m_ValtanPatternMasterModel.reset();
	m_ValtanPatternMasterBoss.reset();
	m_iValtanPatternMasterTargetGeneration = 0u;
	m_ValtanPatternPreviewDocument = {};
	m_ValtanPatternPreviewPlaylist.clear();
	m_bValtanPatternPreviewLoadAttempted = false;
	m_bValtanPatternPreviewPlaying = false;
	m_bValtanPatternPreviewPaused = false;
	m_iValtanPatternPreviewSelected = 0;
	m_iValtanPatternPreviewItem = 0u;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_fValtanPatternPreviewItemDurationSeconds = 0.f;
	m_fValtanPatternPreviewSpeed = 1.f;
	m_strValtanPatternPreviewStatus.clear();
	m_ValtanPatternPreviewModel.reset();
	m_iValtanPatternPreviewTargetGeneration = 0u;
	m_ValtanEncounterReference.Clear();
	m_bValtanEncounterReferenceLoadAttempted = false;
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	/* Chain steps name clips of the body they were built from and the library
	is one authoring body's document, so both are dropped with the target. Kept
	across a switch they would be saved into the next body's file. */
	m_CustomChainSteps.clear();
	m_CustomChainLibrary.clear();
	Invalidate_ValtanPatternCreateExactSourceSelection();
	m_bCustomChainLibraryLoadAttempted = false;
	m_bShowValtanCustomChainWindow = false;
	m_CustomChainFilter[0] = '\0';
	m_CustomChainId[0] = '\0';
	m_CustomChainTargetPatternId[0] = '\0';
	m_CustomChainTargetStageId[0] = '\0';
	m_strCustomChainStatus.clear();
}

void Client::CAnimation_Tool::Render_TargetConflict()
{
	ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::Text("Unsaved document: %s", m_AssetName.c_str());
	if (!m_PendingAssetName.empty())
	{
		ImGui::Text("Pending target: %s", m_PendingAssetName.c_str());
		if (ImGui::Button("Discard Unsaved Animation Documents and Switch"))
			Adopt_AssetName(m_PendingAssetName);
		return;
	}

	ImGui::TextUnformatted("The original target is no longer available.");
	if (ImGui::Button("Discard Orphaned Animation Documents"))
		Adopt_AssetName(std::string{});
}

/* Preview lifecycle now belongs to CCharacterPreviewPanel so Effect authoring
   can read the same target. The tool keeps only the dirty-document policy that
   decides when the target may change. */

bool_t Client::CAnimation_Tool::Is_Window(EVENT_KIND eKind)
{
	return ETOI(eKind) < ETOI(EVENT_KIND::SOUND);
}

const char_t* Client::CAnimation_Tool::Kind_Name(EVENT_KIND eKind)
{
	switch (eKind)
	{
	case EVENT_KIND::CANCEL:     return "CANCEL";
	case EVENT_KIND::SUPERARMOR: return "SUPERARMOR";
	case EVENT_KIND::INVULN:     return "INVULN";
	case EVENT_KIND::MOVE:       return "MOVE";
	case EVENT_KIND::COUNTER:    return "COUNTER";
	case EVENT_KIND::SOUND:      return "SOUND";
	case EVENT_KIND::EFFECT:     return "EFFECT";
	case EVENT_KIND::SHAKE:      return "SHAKE";
	case EVENT_KIND::STAGE:      return "STAGE";
	case EVENT_KIND::HIT:
	default:                     return "HIT";
	}
}

const char_t* Client::CAnimation_Tool::Area_Name(int32_t iAreaType)
{
	switch (iAreaType)
	{
	case 1:  return "circle";
	case 2:  return "box";
	case 3:  return "fan";
	default: return "none";
	}
}

f32_t Client::CAnimation_Tool::Get_ClipTickRate(const shared_ptr<Engine::CModel>& pModel,
	const std::string& clipName)
{
	if (nullptr != pModel)
	{
		const uint32_t iCount = pModel->Get_NumAnimations();
		for (uint32_t i = 0; i < iCount; ++i)
		{
			const char_t* pName = pModel->Get_AnimationName(i);
			if (nullptr != pName && clipName == pName)
			{
				const f32_t fRate = pModel->Get_AnimationTickPerSecond(i);
				if (fRate > 0.f)
					return fRate;
				break;
			}
		}
	}
	return DEFAULT_TICK_RATE;
}

int32_t Client::CAnimation_Tool::Ms_To_Frame(int32_t iMs, f32_t fTickRate)
{
	if (fTickRate <= 0.f)
		fTickRate = DEFAULT_TICK_RATE;

	const f32_t fFrame = static_cast<f32_t>(iMs) * fTickRate / 1000.f;
	return static_cast<int32_t>(fFrame >= 0.f ? fFrame + 0.5f : fFrame - 0.5f);
}

int32_t Client::CAnimation_Tool::Frame_To_Ms(int32_t iFrame, f32_t fTickRate)
{
	if (fTickRate <= 0.f)
		fTickRate = DEFAULT_TICK_RATE;

	const f32_t fMs = static_cast<f32_t>(iFrame) * 1000.f / fTickRate;
	return static_cast<int32_t>(fMs >= 0.f ? fMs + 0.5f : fMs - 0.5f);
}

/* Hit 0 lands at the window start and each repeat iRepeatMs later. The window
width is the tolerance of every hit, not a span the hits are spread across --
that is how the extracted game data is shaped. */
int32_t Client::CAnimation_Tool::Get_TickMs(const ANIM_EVENT& evt, int32_t iTickIndex)
{
	if (iTickIndex <= 0)
		return evt.iStartMs;

	return evt.iStartMs + evt.hit.iRepeatMs * iTickIndex;
}

int32_t Client::CAnimation_Tool::Get_ActiveTick(const ANIM_EVENT& evt, int32_t iMs)
{
	const int32_t iWidth = evt.iEndMs - evt.iStartMs;
	const int32_t iCount = evt.hit.iRepeatCount > 1 ? evt.hit.iRepeatCount : 1;

	for (int32_t i = 0; i < iCount; ++i)
	{
		const int32_t iTick = Get_TickMs(evt, i);
		if (iMs >= iTick && iMs <= iTick + iWidth)
			return i;
	}
	return -1;
}

std::string Client::CAnimation_Tool::Get_EventFilePath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Authored") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".animevents")).string();
}

std::string Client::CAnimation_Tool::Get_SkillReferencePath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".skilltiming")).string();
}

std::string Client::CAnimation_Tool::Get_ClipMapPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".clipmap")).string();
}

std::string Client::CAnimation_Tool::Get_ClipNotifyPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".animnotify")).string();
}

std::string Client::CAnimation_Tool::Get_ClipSeqPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".clipseq")).string();
}

int32_t Client::CAnimation_Tool::Get_ChainOffsetMs(const CLIP_SEQ& seq, int32_t iIndex) const
{
	f32_t fSeconds = 0.f;
	for (int32_t i = 0; i < iIndex && i < static_cast<int32_t>(seq.clips.size()); ++i)
	{
		const auto it = m_ClipLength.find(seq.clips[i]);
		if (m_ClipLength.end() != it)
			fSeconds += it->second;
	}
	return static_cast<int32_t>(fSeconds * 1000.f + 0.5f);
}

const Client::CAnimation_Tool::CLIP_INFO*
Client::CAnimation_Tool::Find_ClipInfo(const char_t* pClipName) const
{
	if (nullptr == pClipName)
		return nullptr;

	const auto it = m_ClipMap.find(pClipName);
	return m_ClipMap.end() == it ? nullptr : &it->second;
}

void Client::CAnimation_Tool::Select_Clip(const shared_ptr<Engine::CModel>& pModel,
	const std::string& clipName)
{
	const uint32_t iCount = pModel->Get_NumAnimations();
	for (uint32_t i = 0; i < iCount; ++i)
	{
		const char_t* pName = pModel->Get_AnimationName(i);
		if (nullptr != pName && clipName == pName)
		{
			pModel->Set_Animation(i, m_bLoop);
			pModel->Set_AnimTrackPosition(i, 0.f);
			m_iSelectedEvent = -1;
			return;
		}
	}
}

void Client::CAnimation_Tool::Render()
{
	ImVec2 workbenchDefaultSize{
		WORKBENCH_DEFAULT_WIDTH, WORKBENCH_DEFAULT_HEIGHT };
	ImVec2 workbenchMinimumSize{
		WORKBENCH_FALLBACK_MIN_WIDTH, WORKBENCH_FALLBACK_MIN_HEIGHT };
	ImVec2 workbenchMaximumSize{ FLT_MAX, FLT_MAX };
	const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	if (nullptr != pViewport)
	{
		/* The saved imgui.ini size is still respected above this floor. On a
		   smaller monitor the floor contracts to the usable work area instead
		   of forcing the Workbench outside the viewport. */
		workbenchMaximumSize = ImVec2(
			(std::max)(WORKBENCH_FALLBACK_MIN_WIDTH, pViewport->WorkSize.x),
			(std::max)(WORKBENCH_FALLBACK_MIN_HEIGHT, pViewport->WorkSize.y));
		workbenchDefaultSize = ImVec2(
			(std::min)(WORKBENCH_DEFAULT_WIDTH, workbenchMaximumSize.x),
			(std::min)(WORKBENCH_DEFAULT_HEIGHT, workbenchMaximumSize.y));
		workbenchMinimumSize = ImVec2(
			(std::min)(WORKBENCH_FALLBACK_MIN_WIDTH, workbenchMaximumSize.x),
			(std::min)(WORKBENCH_FALLBACK_MIN_HEIGHT, workbenchMaximumSize.y));
		if (m_bResetWorkbenchLayoutRequested)
		{
			ImGui::SetNextWindowPos(
				pViewport->WorkPos, ImGuiCond_Always);
		}
	}
	ImGui::SetNextWindowSize(
		workbenchDefaultSize,
		m_bResetWorkbenchLayoutRequested ?
			ImGuiCond_Always : ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(
		workbenchMinimumSize, workbenchMaximumSize);
	m_bResetWorkbenchLayoutRequested = false;
	const char_t* const pTargetLockReason =
		"Save or discard Animation Events, Skill Bindings, Valtan Pattern Animation Bindings, Valtan Pattern Sound, KoukuSaton Action Bindings, and Workbench Sound bindings before changing target.";
	const bool_t isTargetLocked = Is_AnyDocumentDirty();
	m_pPreviewPanel->Set_SessionLock(
		CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL,
		isTargetLocked,
		pTargetLockReason);
	m_pPreviewPanel->Refresh_Level();
	const uint32_t iCurrentLevel =
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t bValtanArena =
		ETOUI(LEVEL::VALTAN_ARENA) == iCurrentLevel;
	std::string strResolvedAssetName;
	shared_ptr<CValtan> pResolvedValtanBoss;
	shared_ptr<Engine::CModel> pResolvedModel;
	bool_t bExactValtanPreview = false;
	uint64_t iResolvedTargetGeneration = 0u;
	const auto RefreshResolvedTarget = [&]()
	{
		strResolvedAssetName = CAnimationTargetService::Resolve_AssetName();
		pResolvedValtanBoss = CAnimationTargetService::Resolve_Boss();
		pResolvedModel = Resolve_Model();
		iResolvedTargetGeneration =
			CAnimationTargetService::Resolve_TargetGeneration();
		bExactValtanPreview = "Valtan" == strResolvedAssetName &&
			nullptr != pResolvedValtanBoss && nullptr != pResolvedModel &&
			pResolvedValtanBoss->Get_BodyModel() == pResolvedModel;
	};
	RefreshResolvedTarget();
	const auto TryStageValtanPreview = [&]()
	{
		/* A dirty Valtan owner may reconstitute its same typed preview body.  The
		   selector remains locked for every user-driven target change, and locks
		   owned by other tools still reject this programmatic same-owner retry. */
		const bool_t bTemporarilyReleaseOwnLock =
			isTargetLocked && Is_ValtanDocumentDirty();
		if (bTemporarilyReleaseOwnLock)
		{
			m_pPreviewPanel->Set_SessionLock(
				CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL, false, {});
		}
		const bool_t bSelected =
			m_pPreviewPanel->Select_TargetAsset("Valtan");
		if (bTemporarilyReleaseOwnLock)
		{
			m_pPreviewPanel->Set_SessionLock(
				CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL,
				true, pTargetLockReason);
		}
		RefreshResolvedTarget();
		if (!bSelected || !bExactValtanPreview)
		{
			m_Status =
				"Valtan Model View could not be staged; canonical Pattern data-only mode remains available: " +
				m_pPreviewPanel->Get_Status();
			return false;
		}
		m_iValtanAutoPreviewSuccessGeneration = iResolvedTargetGeneration;
		m_iValtanAutoPreviewAttemptGeneration = iResolvedTargetGeneration;
		if (m_AssetName.empty() && Is_ValtanDocumentDirty())
			m_AssetName = "Valtan";
		m_Status =
			"Valtan Model View staged from the exact dedicated boss preview; canonical Product workspace preserved.";
		return true;
	};
	if (!bValtanArena)
	{
		m_iValtanAutoPreviewAttemptGeneration = 0u;
		m_iValtanAutoPreviewSuccessGeneration = 0u;
	}
	else
	{
		m_bValtanDataWorkspaceRequested = true;
		if (bExactValtanPreview)
		{
			m_iValtanAutoPreviewAttemptGeneration = iResolvedTargetGeneration;
			m_iValtanAutoPreviewSuccessGeneration = iResolvedTargetGeneration;
		}
		else if (m_iValtanAutoPreviewAttemptGeneration !=
			iResolvedTargetGeneration)
		{
			m_iValtanAutoPreviewAttemptGeneration = iResolvedTargetGeneration;
			if (!isTargetLocked || Is_ValtanDocumentDirty())
				(void)TryStageValtanPreview();
		}
	}

	if (!ImGui::Begin(
		"Animation Clip Tool###AnimationClipToolResizableV1",
		nullptr,
		ImGuiWindowFlags_AlwaysVerticalScrollbar))
	{
		ImGui::End();
		return;
	}

	if (ImGui::SmallButton("Reset Animation Tool Layout"))
	{
		m_bResetWorkbenchLayoutRequested = true;
		m_fKakulActionListWidth = KAKUL_ACTION_LIST_DEFAULT_WIDTH;
		m_Status =
			"Animation Tool layout reset requested; the viewport-aware size applies next frame.";
	}
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Clip preview and Animation Sequence Intake only; Pattern composition lives in the independent Action Composition Workbench.");

	const bool_t bDirtyNonValtanOwner = isTargetLocked &&
		!Is_ValtanDocumentDirty() && !m_AssetName.empty() &&
		"Valtan" != m_AssetName;
	bool_t bValtanWorkspaceMode = !bDirtyNonValtanOwner &&
		(bValtanArena || m_bValtanDataWorkspaceRequested ||
			"Valtan" == m_AssetName || bExactValtanPreview);
	if (bValtanWorkspaceMode)
	{
		/* Target picking is secondary to Product pattern authoring.  Keeping this
		   collapsed prevents the generic model inventory from pushing the joined
		   Pattern shell below the first screen. */
		if (ImGui::CollapsingHeader("Target / Model View"))
			m_pPreviewPanel->Render_Selector(false, {});
	}
	else
	{
		m_pPreviewPanel->Render_Selector(false, {});
	}
	RefreshResolvedTarget();
	bValtanWorkspaceMode = !bDirtyNonValtanOwner &&
		(bValtanArena || m_bValtanDataWorkspaceRequested ||
			"Valtan" == m_AssetName || bExactValtanPreview);
	if (bValtanWorkspaceMode)
	{
		bool_t bTargetSynchronized = true;
		if (bExactValtanPreview)
		{
			if (m_AssetName.empty() && Is_ValtanDocumentDirty())
				m_AssetName = "Valtan";
			else
				bTargetSynchronized = Sync_AssetName();
		}
		else if (Is_ValtanDocumentDirty())
		{
			m_PendingAssetName = strResolvedAssetName;
			m_Status = strResolvedAssetName.empty() ?
				"The exact Valtan preview disappeared while a Valtan typed draft is dirty. The draft and canonical workspace are preserved." :
				"A scene target replaced the exact Valtan preview while a Valtan typed draft is dirty. The scene model is ignored and the draft is preserved.";
			bTargetSynchronized = false;
		}
		if (!bTargetSynchronized)
			Render_TargetConflict();

		const shared_ptr<Engine::CModel> pValtanPreviewModel =
			bExactValtanPreview ? pResolvedModel : nullptr;
		const bool_t bValtanDataOnlyMode = nullptr == pValtanPreviewModel;
		ImGui::SeparatorText("Valtan Workspace Capability");
		ImGui::TextColored(
			bValtanDataOnlyMode ? ImVec4(1.f, 0.75f, 0.2f, 1.f) :
				ImVec4(0.35f, 0.85f, 0.45f, 1.f),
			"Preview Model: %s", bValtanDataOnlyMode ? "MISSING" : "READY");
		if (bValtanDataOnlyMode)
		{
			ImGui::TextWrapped(
				"The dedicated Valtan Model View is required for clip playback and Sequence Intake. Pattern/Stage data remains available in Action Composition Workbench; no scene player model is substituted here.");
			if (ImGui::SmallButton("Retry Valtan Model View"))
				(void)TryStageValtanPreview();
		}
		if (!m_Status.empty())
			ImGui::TextWrapped("%s", m_Status.c_str());
		Render_ValtanAnimationSourceWorkspace(pValtanPreviewModel);
		ImGui::End();
		return;
	}

	const bool_t bTargetSynchronized = Sync_AssetName();
	if (!bTargetSynchronized)
	{
		Render_TargetConflict();
		ImGui::End();
		return;
	}
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pModel)
	{
		ImGui::TextUnformatted("No animated character resolved.");
		ImGui::Separator();
		ImGui::TextUnformatted(
			"Enter Character Select or Development from the Lobby.");
		ImGui::End();
		return;
	}

	if (m_AssetName.empty())
	{
		ImGui::TextUnformatted("The selected target carries no asset name.");
		ImGui::End();
		return;
	}

	const auto previewAsset = std::find_if(
		ANIMATION_PREVIEW_ASSETS.begin(),
		ANIMATION_PREVIEW_ASSETS.end(),
		[this](const ANIMATION_PREVIEW_ASSET& asset)
		{
			return nullptr != asset.pAssetName &&
				m_AssetName == asset.pAssetName;
		});
	if (previewAsset != ANIMATION_PREVIEW_ASSETS.end() &&
		previewAsset->bPlaybackOnly)
	{
		if (!m_bClipMapLoadAttempted)
		{
			m_bClipMapLoadAttempted = true;
			Load_ClipMap();
		}
		if (!m_bClipSeqLoadAttempted)
		{
			m_bClipSeqLoadAttempted = true;
			Load_ClipSeq();
		}
		if (!m_bClipNotifyLoadAttempted)
		{
			m_bClipNotifyLoadAttempted = true;
			Load_ClipNotify();
		}
		if (!m_bRefLoadAttempted)
		{
			m_bRefLoadAttempted = true;
			Load_SkillReference();
		}
		ImGui::Text(
			"Asset: %s   Animations: %u",
			m_AssetName.c_str(),
			pModel->Get_NumAnimations());
		ImGui::TextDisabled(
			"Playback-only preview: Product events and gameplay skill bindings are disabled.");
		ImGui::BeginDisabled(
			m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying);
		Render_Playback(pModel);
		ImGui::EndDisabled();
		if ("Valtan" == m_AssetName)
		{
			Render_ValtanPatternPreview(pModel);
		}
		else if (nullptr != Find_KakulActionProfile(m_strKakulProfileId))
		{
			Render_KakulActionBindings(pModel);
		}
		else if ("MN_RPCT_00" == m_AssetName)
		{
			ImGui::SeparatorText("KoukuSaton Clip Donor");
			ImGui::TextWrapped(
				"MN_RPCT_00 exposes its physical clips for local preview. Select the "
				"MN_RPCT_05 or MN_RPCT_07 action profile in Resource Files to edit "
				"an extracted sequence; no Server Product pattern is inferred here.");
		}
		else if (nullptr != Find_CustomChainProfile(m_AssetName))
		{
			/* This body owns no admitted pattern master, so only the
			   hand-assembled chain workflow opens for it. Playback reuses the
			   same transport window the source sequences use, which reads the
			   playlist rather than any Valtan document. */
			ImGui::SeparatorText("Animation Sequence Intake");
			ImGui::TextWrapped(
				"No pattern master is admitted for this body. Chains assembled "
				"here are saved as Animation Intake and are not Product patterns "
				"until the reviewed promotion pipeline admits them.");
			if (ImGui::SmallButton("Open Animation Sequence Intake"))
				m_bShowValtanCustomChainWindow = true;
			if (m_bShowValtanCustomChainWindow)
				Render_ValtanCustomChainWindow(pModel);
			if (m_bShowValtanSourceReferenceWindow)
				Render_ValtanPatternReferenceWindow(pModel);
		}
		ImGui::BeginDisabled(
			m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying);
		/* The source chain rows are not drawn for Valtan: the Custom Chain
		   window owns chain playback now, and the per-clip buttons crowded the
		   panel the animator actually works in. Other assets keep them. */
		Render_NotifyReference(pModel);
		Render_HitAreaWires(pModel);
		Render_SkillReference(pModel, true);
		if (!m_Status.empty())
			ImGui::TextWrapped("%s", m_Status.c_str());
		ImGui::SeparatorText("Clips");
		ImGui::SetNextItemWidth(-1.f);
		ImGui::InputTextWithHint(
			"##filter",
			"filter by name",
			m_Filter,
			sizeof(m_Filter));
		Render_AnimationList(pModel);
		ImGui::EndDisabled();
		ImGui::End();
		return;
	}

	/* Load once the target actually exists, so a missing file is not reported
	before the level is even open. */
	if (!m_bLoadAttempted)
	{
		m_bLoadAttempted = true;
		Load_Events(pModel);
	}
	Consume_EffectTransfer(pModel);

	/* The game-extracted skill timing is optional; a missing file just hides the
	reference panel and never blocks event authoring. */
	if (!m_bRefLoadAttempted)
	{
		m_bRefLoadAttempted = true;
		Load_SkillReference();
	}

	/* Also optional: without it the clip list just shows the English clip names. */
	if (!m_bClipMapLoadAttempted)
	{
		m_bClipMapLoadAttempted = true;
		Load_ClipMap();
	}

	if (!m_bClipSeqLoadAttempted)
	{
		m_bClipSeqLoadAttempted = true;
		Load_ClipSeq();
	}

	if (!m_bClipNotifyLoadAttempted)
	{
		m_bClipNotifyLoadAttempted = true;
		Load_ClipNotify();

		/* Timing kinds default on; effects and sounds are the noisy ones and are
		better pulled in deliberately. */
		m_bImportKind[ETOI(EVENT_KIND::HIT)] = true;
		m_bImportKind[ETOI(EVENT_KIND::CANCEL)] = true;
		m_bImportKind[ETOI(EVENT_KIND::SUPERARMOR)] = true;
		m_bImportKind[ETOI(EVENT_KIND::SHAKE)] = true;
	}

	ImGui::Text("Asset: %s   Animations: %u", m_AssetName.c_str(), pModel->Get_NumAnimations());

	Render_Playback(pModel);
	Render_ClipChain(pModel);
	Render_SkillBindings(pModel, Resolve_Character());
	Render_HitEvents(pModel);
	Render_HitAreaWires(pModel);
	Render_SkillReference(pModel, false);

	ImGui::SeparatorText("Clips");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##filter", "filter by name", m_Filter, sizeof(m_Filter));

	Render_AnimationList(pModel);

	ImGui::End();
}

void Client::CAnimation_Tool::Consume_EffectTransfer(
	const shared_ptr<Engine::CModel>& pModel)
{
	EFFECT_AUTHORING_CUE_TRANSFER Transfer;
	if (!CEffectAuthoringTransfer::Consume(Transfer))
		return;
	if (Transfer.iTargetGeneration !=
		CAnimationTargetService::Resolve_TargetGeneration() ||
		Transfer.strAnimationAssetId != m_AssetName ||
		Transfer.strClipName.empty() ||
		!CEffectCatalog::Contains(Transfer.strEffectAssetId) ||
		Transfer.ePivotKind >= EFFECT_CUE_PIVOT_KIND::END ||
		Transfer.eOrientationPolicy >= EFFECT_ORIENTATION_POLICY::END ||
		(EFFECT_ORIENTATION_POLICY::ACTION_FACING ==
			Transfer.eOrientationPolicy &&
			(Transfer.ePivotKind != EFFECT_CUE_PIVOT_KIND::PLAYER_ROOT ||
			 "root" != Transfer.strAnchorSlotId)))
	{
		m_Status = "Effect cue transfer rejected because the animation target or admitted Effect changed.";
		return;
	}

	ANIM_EVENT Event;
	Event.clipName = Transfer.strClipName;
	Event.eKind = EVENT_KIND::EFFECT;
	Event.iStartMs = static_cast<int32_t>(Transfer.iTimeMs);
	Event.iEndMs = EFFECT_STOP_POLICY::CUE_END == Transfer.eStopPolicy ?
		Event.iStartMs + static_cast<int32_t>(Transfer.iDurationMs) :
		Event.iStartMs;
	Event.sPayload = Transfer.strEffectAssetId;
	Event.eEffectReferenceKind = EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID;
	Event.sAnchorSlotId = Transfer.strAnchorSlotId;
	Event.eFollowPolicy = Transfer.eFollowPolicy;
	Event.eOrientationPolicy = Transfer.eOrientationPolicy;
	Event.eStopPolicy = Transfer.eStopPolicy;
	Event.EffectLocalTransform = Transfer.LocalTransform;

	const auto Duplicate = std::find_if(
		m_Events.begin(), m_Events.end(),
		[&Event](const ANIM_EVENT& Existing)
		{
			return Existing.eKind == EVENT_KIND::EFFECT &&
				Existing.eEffectReferenceKind ==
					EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID &&
				Existing.clipName == Event.clipName &&
				Existing.iStartMs == Event.iStartMs &&
				Existing.sPayload == Event.sPayload &&
				Existing.sAnchorSlotId == Event.sAnchorSlotId;
		});
	if (Duplicate != m_Events.end())
	{
		m_Status = "Effect cue transfer rejected because the same cue already exists.";
		return;
	}

	std::vector<ANIM_EVENT> Staged = m_Events;
	Staged.push_back(Event);
	std::string ValidationStatus;
	if (!Validate_Events(pModel, Staged, ValidationStatus))
	{
		m_Status = "Effect cue transfer rejected: " + ValidationStatus;
		return;
	}
	m_Events = std::move(Staged);
	m_iSelectedEvent = static_cast<int32_t>(m_Events.size() - 1u);
	strncpy_s(m_PayloadEdit, Event.sPayload.c_str(), _TRUNCATE);
	m_bDirty = true;
	Select_Clip(pModel, Event.clipName);
	const f32_t fTickRate = Get_ClipTickRate(pModel, Event.clipName);
	pModel->Set_AnimPaused(true);
	pModel->Set_AnimTrackPosition(
		pModel->Get_CurrentAnimIndex(),
		static_cast<f32_t>(Event.iStartMs) * fTickRate / 1000.f);
	m_iSelectedEvent = static_cast<int32_t>(m_Events.size() - 1u);
	m_Status = "Added admitted Effect cue from Effect Tool; review and Save explicitly.";
}

void Client::CAnimation_Tool::Render_Playback(const shared_ptr<Engine::CModel>& pModel)
{
	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const bool_t bPaused = pModel->Is_AnimPaused();

	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	ImGui::Text("%s [%u] %s", bPaused ? "Paused:" : "Playing:",
		iCurrentIndex, nullptr != pCurrentName ? pCurrentName : "(none)");

	if (const CLIP_INFO* pInfo = Find_ClipInfo(pCurrentName))
	{
		ImGui::SameLine();
		ImGui::TextDisabled("= %s (%d)", pInfo->name.c_str(), pInfo->iSkillId);
	}

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const bool_t bHasTrack =
		pModel->Get_AnimationProgress(iCurrentIndex, fPosition, fDuration) && fDuration > 0.f;

	/* Track positions are in ticks. Blender exports one key per frame, so 1 tick == 1 frame. */
	if (bHasTrack)
	{
		f32_t fScrub = fPosition;

		/* SliderFloat feeds only the slider value to its format string, so the
		duration has to be baked in and the value slot escaped as %%. */
		char_t szScrubFormat[64]{};
		snprintf(szScrubFormat, sizeof(szScrubFormat), "frame %%.1f / %.0f", fDuration);

		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::SliderFloat("##scrub", &fScrub, 0.f, fDuration, szScrubFormat))
		{
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrentIndex, fScrub);
		}
	}
	else
	{
		ImGui::TextUnformatted("Track: n/a");
	}

	if (ImGui::Button(bPaused ? "Play" : "Pause"))
		pModel->Set_AnimPaused(!bPaused);

	ImGui::SameLine();
	ImGui::BeginDisabled(!bHasTrack);
	if (ImGui::Button("< Frame"))
	{
		pModel->Set_AnimPaused(true);
		pModel->Set_AnimTrackPosition(iCurrentIndex, fPosition - 1.f);
	}
	ImGui::SameLine();
	if (ImGui::Button("Frame >"))
	{
		pModel->Set_AnimPaused(true);
		pModel->Set_AnimTrackPosition(iCurrentIndex, fPosition + 1.f);
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Restart"))
		pModel->Set_AnimTrackPosition(iCurrentIndex, 0.f);

	ImGui::SameLine();
	if (ImGui::Checkbox("Loop", &m_bLoop))
		pModel->Set_Animation(iCurrentIndex, m_bLoop);
}

const char_t* Client::CAnimation_Tool::ValtanPatternMasterPathName(
	const VALTAN_PATTERN_PREVIEW_PATH ePath)
{
	switch (ePath)
	{
	case VALTAN_PATTERN_PREVIEW_PATH::NORMAL:
		return "Normal";
	case VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY:
		return "Counter Hit -> Groggy";
	case VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY:
		return "Wall -> Groggy -> Recovery";
	case VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK:
		return "Wall -> Groggy -> Part Break";
	default:
		return "Invalid";
	}
}

const char_t* Client::CAnimation_Tool::ValtanPatternMasterAdmissionLabel() const
{
	switch (m_eValtanPatternMasterAdmission)
	{
	case VALTAN_PATTERN_MASTER_ADMISSION_STATE::UNLOADED:
		return "NOT LOADED";
	case VALTAN_PATTERN_MASTER_ADMISSION_STATE::ADMITTED:
		return "ADMITTED";
	case VALTAN_PATTERN_MASTER_ADMISSION_STATE::STALE_PRESERVED:
		return "STALE PRESERVED / RELOAD REJECTED";
	case VALTAN_PATTERN_MASTER_ADMISSION_STATE::REJECTED:
		return "REJECTED";
	default:
		return "INVALID";
	}
}

std::vector<const Client::VALTAN_PATTERN_VIEW*>
Client::CAnimation_Tool::Collect_ValtanPatternMasterPatterns() const
{
	std::vector<const VALTAN_PATTERN_VIEW*> Patterns;
	Patterns.reserve(m_ValtanPatternMasterView.Get_PatternCount());
	std::unordered_set<std::string> Collected;
	for (const std::string_view strPatternId : VALTAN_PATTERN_MASTER_ORDER)
	{
		if (const VALTAN_PATTERN_VIEW* pPattern = Find_ValtanPatternMaster(
			m_ValtanPatternMasterView, strPatternId))
		{
			Patterns.push_back(pPattern);
			Collected.insert(pPattern->strPatternId);
		}
	}
	const auto AppendProductPresentation = [&Patterns, &Collected](
		const std::vector<VALTAN_PATTERN_VIEW>& Source)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Source)
		{
			if (!Pattern.Stages.empty() &&
				Collected.insert(Pattern.strPatternId).second)
			{
				Patterns.push_back(&Pattern);
			}
		}
	};
	AppendProductPresentation(m_ValtanPatternMasterView.Rotation);
	AppendProductPresentation(m_ValtanPatternMasterView.Gimmicks);
	return Patterns;
}

bool_t Client::CAnimation_Tool::Reload_ValtanPatternMaster()
{
	const auto RejectReload = [this](std::string Diagnostic)
	{
		const bool_t bHasPreservedAdmission =
			Collect_ValtanPatternMasterPatterns().size() >=
				VALTAN_PATTERN_MASTER_ORDER.size();
		m_eValtanPatternMasterAdmission = bHasPreservedAdmission ?
			VALTAN_PATTERN_MASTER_ADMISSION_STATE::STALE_PRESERVED :
			VALTAN_PATTERN_MASTER_ADMISSION_STATE::REJECTED;
		m_strValtanPatternMasterStatus = bHasPreservedAdmission ?
			"Valtan Pattern Master reload rejected; previous admitted graph and pose preserved as STALE: " + Diagnostic :
			"Valtan Pattern Master reload rejected; no canonical graph is admitted: " + Diagnostic;
		return false;
	};
	VALTAN_PATTERN_TREE_VIEW Staged;
	std::string Status;
	if (!CValtanPatternTree::Load(Staged, Status))
		return RejectReload(Status);

	size_t iManagedPatternCount = 0u;
	const auto CountManaged = [&iManagedPatternCount](
		const std::vector<VALTAN_PATTERN_VIEW>& Patterns)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Patterns)
		{
			if (Pattern.bAuthoringMasterManaged)
				++iManagedPatternCount;
		}
	};
	CountManaged(Staged.Rotation);
	CountManaged(Staged.Gimmicks);
	if (iManagedPatternCount < VALTAN_PATTERN_MASTER_ORDER.size())
	{
		return RejectReload(
			"expected the 7 baseline managed patterns, found " +
			std::to_string(iManagedPatternCount) +
			".");
	}
	for (const std::string_view strPatternId : VALTAN_PATTERN_MASTER_ORDER)
	{
		if (nullptr == Find_ValtanPatternMaster(Staged, strPatternId))
		{
			return RejectReload(
				"required pattern is missing: " +
				std::string(strPatternId) + ".");
		}
	}

	m_ValtanPatternMasterView = std::move(Staged);
	m_bValtanCompositionDraftPreviewReady = false;
	m_ValtanCompositionDraftPreview = {};
	m_eValtanPatternMasterAdmission =
		VALTAN_PATTERN_MASTER_ADMISSION_STATE::ADMITTED;
	/* Sound is a fail-open presentation lane.  Its loader still stages the
	   complete document and only commits on a valid gameplay/animation join;
	   a rejected refresh preserves the last admitted lane while animation and
	   Server gameplay remain usable. */
	if (!m_bValtanPatternSoundCuesDirty)
		(void)Reload_ValtanPatternSoundCues();
	(void)Reload_ValtanPatternShakeCues();
	(void)Reload_ValtanCombatObjectSoundCues();
	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
		Collect_ValtanPatternMasterPatterns();
	m_iValtanPatternMasterSelected = std::clamp(
		m_iValtanPatternMasterSelected, 0,
		static_cast<int32_t>(Patterns.size() - 1u));
	m_strValtanPatternMasterStatus =
		"Valtan Product presentation admitted: " +
		std::to_string(Patterns.size()) + " patterns (" +
		std::to_string(iManagedPatternCount) + " authoring-master managed) from "
		"Data/Valtan/Valtan.gameplay.json + Valtan.presentation.json. " +
		Status;
	return true;
}

bool_t Client::CAnimation_Tool::Reload_ValtanPatternSoundCues()
{
	std::string LifecycleStatus;
	if (!Can_CommitValtanCompositionPatternSoundGeneration(LifecycleStatus))
	{
		m_strValtanPatternSoundCueStatus = std::move(LifecycleStatus);
		return false;
	}
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged;
	std::string StagedBaselineSourceBytes;
	std::string Status;
	if (!CValtanPatternSoundCueDocument::Load_ForAuthoring(
		Staged, StagedBaselineSourceBytes, Status))
	{
		m_strValtanPatternSoundCueStatus =
			"Strict Pattern Sound authoring reload rejected; the previous admitted draft was preserved: " +
			Status;
		return false;
	}
	m_ValtanPatternSoundCues = std::move(Staged);
	++m_iValtanPatternSoundDraftGeneration;
	m_strValtanPatternSoundCueBaselineSourceBytes =
		std::move(StagedBaselineSourceBytes);
	m_bValtanPatternSoundCuesReady = true;
	m_bValtanPatternSoundCuesDirty = false;
	/* Source admission is data-only.  The Workbench may apply it to active
	   Arena/preview consumers only after the exact immutable Pattern revision is
	   confirmed Server-active. */
	m_bValtanPatternSoundRuntimeApplyReady = false;
	m_ValtanPatternSoundRuntimeAppliedRevision = {};
	m_strValtanPatternSoundCueStatus =
		"Strict authoring admission (active consumer apply pending): " + Status;
	return true;
}

bool_t Client::CAnimation_Tool::Reload_ValtanPatternShakeCues()
{
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT Staged;
	std::string Status;
	if (!CValtanPatternShakeCueDocument::Load_Source(Staged, Status))
	{
		m_strValtanPatternShakeCueStatus =
			"Camera/Shake lane reload rejected; the previous admitted lane was preserved: " +
			Status;
		return false;
	}
	m_ValtanPatternShakeCues = std::move(Staged);
	m_bValtanPatternShakeCuesReady = true;
	m_strValtanPatternShakeCueStatus =
		"Admitted " + std::to_string(m_ValtanPatternShakeCues.Cues.size()) +
		" Valtan camera-shake cue occurrences.";
	return true;
}

bool_t Client::CAnimation_Tool::Reload_ValtanCombatObjectSoundCues()
{
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT Staged;
	std::string Status;
	if (!CValtanCombatObjectSoundCueDocument::Load_Source(Staged, Status))
	{
		m_strValtanCombatObjectSoundCueStatus =
			"Server-hit Sound lane reload rejected; the previous admitted lane was preserved: " +
			Status;
		return false;
	}
	m_ValtanCombatObjectSoundCues = std::move(Staged);
	m_bValtanCombatObjectSoundCuesReady = true;
	m_strValtanCombatObjectSoundCueStatus =
		"Admitted " +
		std::to_string(m_ValtanCombatObjectSoundCues.Cues.size()) +
		" Server-hit-qualified Valtan Sound cue(s).";
	return true;
}

bool_t Client::CAnimation_Tool::Preview_ValtanSoundAsset(
	const std::string& strResourceAssetId)
{
	const std::filesystem::path SoundPath =
		CRuntimeAssetRoot::Resolve(strResourceAssetId);
	std::error_code Error;
	if (SoundPath.empty() ||
		!std::filesystem::is_regular_file(SoundPath, Error) || Error)
	{
		m_strValtanPatternSoundCueStatus =
			"Sound asset preview rejected: Resources-relative asset is missing or invalid: " +
			strResourceAssetId + ".";
		return false;
	}
	if (FAILED(CGameInstance::Get().Play_Sound(SoundPath.wstring(), 1.f)))
	{
		m_strValtanPatternSoundCueStatus =
			"Sound asset preview failed: " + strResourceAssetId + ".";
		return false;
	}
	m_strValtanPatternSoundCueStatus =
		"Previewing sound asset: " + strResourceAssetId + ".";
	return true;
}

bool_t Client::CAnimation_Tool::Build_ValtanPatternMasterTimeline(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	const shared_ptr<Engine::CModel>& pModel,
	std::vector<VALTAN_PATTERN_MASTER_PLAY_ITEM>& OutPlaylist,
	uint32_t& iOutDurationMs,
	std::string& strOutStatus) const
{
	OutPlaylist.clear();
	iOutDurationMs = 0u;
	strOutStatus.clear();
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		strOutStatus = "Animation target changed before the master timeline was staged.";
		return false;
	}
	if (Pattern.Stages.empty())
	{
		strOutStatus = "Selected pattern has no admitted stages in the split "
			"Valtan.gameplay.json + Valtan.presentation.json source.";
		return false;
	}

	std::vector<const VALTAN_STAGE_VIEW*> StagePath;
	if (!CValtanPatternTree::Build_PreviewStagePath(
		Pattern, ePath, StagePath, strOutStatus))
	{
		return false;
	}

	uint64_t iTimelineMs = 0u;
	for (const VALTAN_STAGE_VIEW* pStage : StagePath)
	{
		if (nullptr == pStage || 0u == pStage->iDurationMs ||
			pStage->strActionId.empty())
		{
			strOutStatus =
				"Master branch graph contains an incomplete animation stage.";
			return false;
		}
		const VALTAN_STAGE_VIEW& Stage = *pStage;
		LostArk::Shared::WORLD_ENTITY_ACTION eStageAction =
			LostArk::Shared::WORLD_ENTITY_ACTION::END;
		if (!Try_ResolveValtanArenaPatternAction(
				Stage.strStageKind, eStageAction) ||
			LostArk::Shared::WORLD_ENTITY_ACTION::END == eStageAction)
		{
			strOutStatus = "Master stage kind has no Arena snapshot action: " +
				Stage.strStageKind + ".";
			return false;
		}
		const uint64_t iStageTimelineStartMs = iTimelineMs;
		if (Stage.bSuppressAnimation)
		{
			if (!Stage.ClipOccurrences.empty())
			{
				strOutStatus = "NONE animation stage unexpectedly owns clips: " +
					Stage.strActionId + ".";
				return false;
			}
			VALTAN_PATTERN_MASTER_PLAY_ITEM Item;
			Item.strPatternId = Pattern.strPatternId;
			Item.strPatternDisplayName = Pattern.strDisplayName;
			Item.strStageId = Stage.strStageId;
			Item.strSequenceRole = Stage.strSequenceRole;
			Item.strStageKind = Stage.strStageKind;
			Item.strActionId = Stage.strActionId;
			Item.iAuthoringWallMs = Stage.iDurationMs;
			Item.iTimelineStartMs = static_cast<uint32_t>(iTimelineMs);
			Item.iStageTimelineStartMs =
				static_cast<uint32_t>(iStageTimelineStartMs);
			Item.iOccurrenceNumber = 1u;
			Item.iOccurrenceCount = 1u;
			Item.bSuppressAnimation = true;
			OutPlaylist.push_back(std::move(Item));
			iTimelineMs += Stage.iDurationMs;
			if (iTimelineMs > static_cast<uint64_t>(
					(std::numeric_limits<uint32_t>::max)()))
			{
				strOutStatus = "Master authoring timeline duration overflowed.";
				return false;
			}
			continue;
		}
		if (Stage.ClipOccurrences.empty())
		{
			strOutStatus = "Master branch graph contains an unbound animation stage: " +
				Stage.strActionId + ".";
			return false;
		}
		const size_t iPlayableOccurrenceCount =
			Stage.iAuthoringRepeatCount > 1u ?
				static_cast<size_t>(Stage.iAuthoringRepeatCount) :
				Stage.ClipOccurrences.size();
		if (0u == iPlayableOccurrenceCount ||
			iPlayableOccurrenceCount != Stage.ClipOccurrences.size())
		{
			strOutStatus = "Master repeatCount does not own exactly its explicit occurrences for " +
				Stage.strActionId + ".";
			return false;
		}

		uint64_t iStageAnimationWallMs = 0u;
		for (size_t iClip = 0u; iClip < iPlayableOccurrenceCount; ++iClip)
		{
			const VALTAN_CLIP_OCCURRENCE_VIEW& Clip =
				Stage.ClipOccurrences[iClip];
			if (Clip.strClipOccurrenceId.empty() || Clip.strClipName.empty() ||
				0u == Clip.iAuthoringWallMs ||
				!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f)
			{
				strOutStatus = "Master occurrence is incomplete: " +
					Clip.strClipOccurrenceId + ".";
				return false;
			}

			uint32_t iAnimationIndex = (std::numeric_limits<uint32_t>::max)();
			for (uint32_t iAnimation = 0u;
				iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
			{
				const char_t* pName = pModel->Get_AnimationName(iAnimation);
				if (nullptr != pName && Clip.strClipName == pName)
				{
					iAnimationIndex = iAnimation;
					break;
				}
			}
			if ((std::numeric_limits<uint32_t>::max)() == iAnimationIndex)
			{
				strOutStatus = "Scene Valtan model is missing master clip " +
					Clip.strClipName + ".";
				return false;
			}

			f32_t fTrackPosition = 0.f;
			f32_t fTrackDuration = 0.f;
			const f32_t fTickRate =
				pModel->Get_AnimationTickPerSecond(iAnimationIndex);
			if (!std::isfinite(fTickRate) || fTickRate <= 0.f ||
				!pModel->Get_AnimationProgress(
					iAnimationIndex, fTrackPosition, fTrackDuration) ||
				!std::isfinite(fTrackDuration) || fTrackDuration <= 0.f)
			{
				strOutStatus = "Master clip has no valid model clock: " +
					Clip.strClipName + ".";
				return false;
			}
			const ACTION_PRESENTATION_CLIP_TIMING Timing{
				fTrackDuration / fTickRate,
				Clip.iPlayMs,
				Clip.fPlayRate,
				Clip.bLoop,
				static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f };
			f32_t fSourceDuration = 0.f;
			f32_t fSourceWallDuration = 0.f;
			if (!CActionPresentationTimeline::Resolve_ClipDuration(
				Timing, fSourceDuration, fSourceWallDuration))
			{
				strOutStatus = "Master source window is outside the model clip: " +
					Clip.strClipOccurrenceId + ".";
				return false;
			}

			VALTAN_PATTERN_MASTER_PLAY_ITEM Item;
			Item.strPatternId = Pattern.strPatternId;
			Item.strPatternDisplayName = Pattern.strDisplayName;
			Item.strStageId = Stage.strStageId;
			Item.strSequenceRole = Stage.strSequenceRole;
			Item.strStageKind = Stage.strStageKind;
			Item.strActionId = Stage.strActionId;
			Item.strClipOccurrenceId = Clip.strClipOccurrenceId;
			Item.strClipName = Clip.strClipName;
			Item.iSourceStartMs = Clip.iSourceStartMs;
			Item.iPlayMs = Clip.iPlayMs;
			Item.iAuthoringWallMs = Clip.iAuthoringWallMs;
			Item.iTimelineStartMs = static_cast<uint32_t>(iTimelineMs);
			Item.iStageTimelineStartMs =
				static_cast<uint32_t>(iStageTimelineStartMs);
			Item.iOccurrenceNumber = static_cast<uint32_t>(iClip + 1u);
			Item.iOccurrenceCount = static_cast<uint32_t>(
				iPlayableOccurrenceCount);
			Item.fPlayRate = Clip.fPlayRate;
			Item.bRepeatUntilStageEnd = Clip.bLoop;
			OutPlaylist.push_back(std::move(Item));

			iStageAnimationWallMs += Clip.iAuthoringWallMs;
			iTimelineMs += Clip.iAuthoringWallMs;
			if (iTimelineMs > static_cast<uint64_t>(
				(std::numeric_limits<uint32_t>::max)()))
			{
				strOutStatus = "Master authoring timeline duration overflowed.";
				return false;
			}
		}
		if (iStageAnimationWallMs != Stage.iDurationMs)
		{
			strOutStatus = "Master occurrences do not fill Server stage " +
				Stage.strStageId + ".";
			return false;
		}
	}
	if (OutPlaylist.empty() || 0u == iTimelineMs)
	{
		strOutStatus = "Master authoring timeline is empty.";
		return false;
	}
	iOutDurationMs = static_cast<uint32_t>(iTimelineMs);
	strOutStatus = "Admitted " + Pattern.strPatternId + " / " +
		ValtanPatternMasterPathName(ePath) + " / " +
		std::to_string(OutPlaylist.size()) + " presentation items / " +
		std::to_string(iOutDurationMs) + " ms.";
	return true;
}

bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath)
{
	if (VALTAN_PATTERN_MASTER_ADMISSION_STATE::ADMITTED !=
		m_eValtanPatternMasterAdmission)
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Offline play rejected: the preserved graph is display-only until a fresh canonical reload is ADMITTED.";
		return false;
	}
	std::vector<VALTAN_PATTERN_MASTER_PLAY_ITEM> StagedPlaylist;
	uint32_t iStagedDurationMs = 0u;
	std::string Status;
	if (!Build_ValtanPatternMasterTimeline(
		Pattern, ePath, pModel, StagedPlaylist, iStagedDurationMs, Status))
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Master play rejected; current model pose preserved: " +
			Status;
		return false;
	}

	const shared_ptr<CValtan> PreviewBoss =
		CAnimationTargetService::Resolve_Boss();
	if (nullptr == PreviewBoss || PreviewBoss->Get_BodyModel() != pModel)
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Master play rejected; the staged model is not the current local Valtan boss.";
		return false;
	}
	if (!PreviewBoss->Stage_LocalPatternAuthoringPreview(Pattern, Status))
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Master play rejected; effective draft preview staging failed: " +
			Status;
		return false;
	}

	const uint32_t iPreviousAnimation = pModel->Get_CurrentAnimIndex();
	f32_t fPreviousPosition = 0.f;
	f32_t fPreviousDuration = 0.f;
	const bool_t bPreviousTrack = pModel->Get_AnimationProgress(
		iPreviousAnimation, fPreviousPosition, fPreviousDuration);
	const bool_t bPreviousPaused = pModel->Is_AnimPaused();
	const bool_t bPreviousLoop = pModel->Is_AnimLoop();
	const f32_t fPreviousSpeed = m_bValtanPatternPreviewPlaying ?
		m_fValtanPatternPreviewSpeed : 1.f;
	if (m_bValtanPatternPreviewPlaying)
	{
		Reset_ValtanPatternPreviewState(
			"Source reference preview yielded to Valtan Pattern Master.");
	}

	m_ValtanPatternMasterPlaylist = std::move(StagedPlaylist);
	m_iValtanPatternMasterItem = 0u;
	m_fValtanPatternMasterItemElapsedSeconds = 0.f;
	m_iValtanPatternMasterDurationMs = iStagedDurationMs;
	m_eValtanPatternMasterPath = ePath;
	m_bValtanPatternMasterPlaying = true;
	m_bValtanPatternMasterPaused = false;
	m_ValtanPatternMasterModel = pModel;
	m_ValtanPatternMasterBoss = PreviewBoss;
	m_iValtanPatternMasterTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_strValtanPatternMasterStatus = Status;
	if (Activate_ValtanPatternMasterItem(pModel, 0u, 0.f))
	{
		Update_ValtanPatternMasterHitAreaPreview();
		return true;
	}

	Reset_ValtanPatternMasterPreviewState(
		"Valtan Pattern Master play failed; previous model pose restored.");
	if (bPreviousTrack &&
		pModel->Start_Animation(iPreviousAnimation, bPreviousLoop))
	{
		pModel->Set_AnimationSpeed(fPreviousSpeed);
		pModel->Set_AnimTrackPosition(iPreviousAnimation, fPreviousPosition);
		pModel->Set_AnimPaused(bPreviousPaused);
		pModel->Play_Animation(0.f);
	}
	return false;
}

bool_t Client::CAnimation_Tool::Apply_ValtanPatternMasterPose(
	const shared_ptr<Engine::CModel>& pModel,
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item,
	const f32_t fLocalWallSeconds,
	const bool_t bForceAnimationEdge) const
{
	if (nullptr == pModel || !std::isfinite(fLocalWallSeconds) ||
		fLocalWallSeconds < 0.f)
	{
		return false;
	}
	const shared_ptr<CValtan> Boss = m_ValtanPatternMasterBoss.lock();
	if (nullptr == Boss || Boss->Get_BodyModel() != pModel)
		return false;
	LostArk::Shared::WORLD_ENTITY_ACTION ePatternAction =
		LostArk::Shared::WORLD_ENTITY_ACTION::END;
	if (!Try_ResolveValtanArenaPatternAction(
			Item.strStageKind, ePatternAction))
	{
		return false;
	}
	const f32_t fStageWallSeconds =
		(static_cast<f32_t>(Item.iTimelineStartMs -
			Item.iStageTimelineStartMs) * 0.001f) +
		fLocalWallSeconds;
	if (!Boss->Apply_LocalPatternPresentationSample(
			ePatternAction,
			Item.strActionId,
			fStageWallSeconds,
			bForceAnimationEdge))
	{
		return false;
	}
	/* Animation Tool owns the wall clock and samples the Product presentation
	   pose explicitly. Pausing the model prevents a second local frame clock
	   from drifting between those samples. */
	pModel->Set_AnimPaused(true);
	return true;
}

bool_t Client::CAnimation_Tool::Activate_ValtanPatternMasterItem(
	const shared_ptr<Engine::CModel>& pModel,
	const std::size_t iItem,
	const f32_t fLocalWallSeconds)
{
	if (nullptr == pModel ||
		iItem >= m_ValtanPatternMasterPlaylist.size())
	{
		return false;
	}
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
		m_ValtanPatternMasterPlaylist[iItem];
	const f32_t fDurationSeconds =
		static_cast<f32_t>(Item.iAuthoringWallMs) * 0.001f;
	/* The master timeline drives the pose itself: it holds the model paused and
	writes the authored source time every frame.  A blend cannot finish on that
	path -- CModel::Play_Animation feeds Update_AnimBlend a zero delta while the
	model is paused, so m_fBlendElapsed never advances and every bone is pulled
	back to the captured blend-from pose at ratio zero.  Start the occurrence
	without a blend; the very next Apply_ValtanPatternMasterPose owns the pose. */
	if (!std::isfinite(fLocalWallSeconds) || fLocalWallSeconds < 0.f ||
		fLocalWallSeconds > fDurationSeconds + 0.000001f)
	{
		return false;
	}
	m_iValtanPatternMasterItem = iItem;
	m_fValtanPatternMasterItemElapsedSeconds =
		std::clamp(fLocalWallSeconds, 0.f, fDurationSeconds);
	if (!Apply_ValtanPatternMasterPose(
		pModel, Item, m_fValtanPatternMasterItemElapsedSeconds, true))
	{
		return false;
	}
	m_strValtanPatternMasterStatus = Item.bSuppressAnimation ?
		"Playing admitted NONE stage (boss pose hold) | " +
			Item.strStageId + " / " + Item.strSequenceRole + "." :
		"Playing admitted occurrence " + Item.strClipOccurrenceId + " | " +
			Item.strStageId + " / " + Item.strSequenceRole + ".";
	return true;
}

bool_t Client::CAnimation_Tool::Seek_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const f32_t fTimelineSeconds,
	const bool_t bPause,
	const bool_t bResetPresentationTransport)
{
	if (!m_bValtanPatternMasterPlaying || nullptr == pModel ||
		m_ValtanPatternMasterPlaylist.empty() ||
		!std::isfinite(fTimelineSeconds) || fTimelineSeconds < 0.f)
	{
		return false;
	}
	const f32_t fTimelineMs = std::clamp(
		fTimelineSeconds * 1000.f, 0.f,
		static_cast<f32_t>(m_iValtanPatternMasterDurationMs));
	for (size_t iItem = 0u;
		iItem < m_ValtanPatternMasterPlaylist.size(); ++iItem)
	{
		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
			m_ValtanPatternMasterPlaylist[iItem];
		const f32_t fEndMs = static_cast<f32_t>(
			Item.iTimelineStartMs + Item.iAuthoringWallMs);
		const bool_t bLast =
			iItem + 1u == m_ValtanPatternMasterPlaylist.size();
		if (fTimelineMs < fEndMs || bLast)
		{
			f32_t fLocalSeconds = std::clamp(
				(fTimelineMs - static_cast<f32_t>(Item.iTimelineStartMs)) *
					0.001f,
				0.f,
				static_cast<f32_t>(Item.iAuthoringWallMs) * 0.001f);
			/* Sampling an exact looping endpoint wraps to source time zero.
			   The timeline endpoint means the final visible pose, so only that
			   endpoint is moved to the previous representable wall time. */
			if (bLast && fTimelineMs >= fEndMs && fLocalSeconds > 0.f)
				fLocalSeconds = std::nextafter(fLocalSeconds, 0.f);
			if (bResetPresentationTransport)
			{
				const shared_ptr<CValtan> PreviewBoss =
					m_ValtanPatternMasterBoss.lock();
				if (nullptr == PreviewBoss || PreviewBoss->Get_BodyModel() != pModel)
					return false;
				PreviewBoss->Reset_LocalPatternPreviewTransport();
			}
			if (!Activate_ValtanPatternMasterItem(
				pModel, iItem, fLocalSeconds))
			{
				return false;
			}
			m_bValtanPatternMasterPaused = bPause;
			Update_ValtanPatternMasterHitAreaPreview();
			return true;
		}
	}
	return false;
}

void Client::CAnimation_Tool::Advance_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (!m_bValtanPatternMasterPlaying ||
		m_iValtanPatternMasterItem >= m_ValtanPatternMasterPlaylist.size())
	{
		return;
	}
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
		m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
	const f32_t fAbsoluteSeconds =
		static_cast<f32_t>(Item.iTimelineStartMs) * 0.001f +
		m_fValtanPatternMasterItemElapsedSeconds;
	if (fAbsoluteSeconds * 1000.f + 0.001f >=
		static_cast<f32_t>(m_iValtanPatternMasterDurationMs))
	{
		if (m_bValtanCompositionLoop &&
			Seek_ValtanPatternMasterPreview(pModel, 0.f, false, true))
		{
			m_strValtanPatternMasterStatus =
				"Valtan Pattern preview loop restarted from the admitted first occurrence.";
			return;
		}
		Stop_ValtanPatternMasterPreview(
			pModel,
			"Valtan Pattern Master timeline completed; idle restored.");
		return;
	}
	if (!Seek_ValtanPatternMasterPreview(
			pModel, fAbsoluteSeconds, false, false))
	{
		Stop_ValtanPatternMasterPreview(
			pModel,
			"Valtan Pattern Master timeline failed to advance; idle restored.");
	}
}

void Client::CAnimation_Tool::Stop_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const std::string& status)
{
	if (nullptr != pModel)
	{
		m_bLoop = true;
		pModel->Set_AnimationSpeed(1.f);
		if (!pModel->Start_Animation("mesh_idle_battle_1", true))
			pModel->Set_AnimPaused(true);
	}
	Reset_ValtanPatternMasterPreviewState(status);
}

void Client::CAnimation_Tool::Reset_ValtanPatternMasterPreviewState(
	const std::string& status)
{
	const shared_ptr<Engine::CModel> PreviewModel =
		m_ValtanPatternMasterModel.lock();
	const shared_ptr<CValtan> PreviewBoss =
		m_ValtanPatternMasterBoss.lock();
	m_bValtanPatternMasterPlaying = false;
	m_bValtanPatternMasterPaused = false;
	m_iValtanPatternMasterItem = 0u;
	m_fValtanPatternMasterItemElapsedSeconds = 0.f;
	m_iValtanPatternMasterDurationMs = 0u;
	m_ValtanPatternMasterPlaylist.clear();
	m_strValtanPatternMasterStatus = status;
	m_ValtanPatternMasterModel.reset();
	m_ValtanPatternMasterBoss.reset();
	m_iValtanPatternMasterTargetGeneration = 0u;
	if (nullptr != PreviewBoss)
	{
		PreviewBoss->Reset_LocalPatternPresentationSample();
	}
	else if (nullptr != PreviewModel)
	{
		PreviewModel->Set_AnimationSpeed(1.f);
		PreviewModel->Set_AnimPaused(false);
	}
#ifdef _DEBUG
	if (nullptr != PreviewBoss)
	{
		PreviewBoss->Clear_PatternHitAreaPreview();
	}
#endif
}

void Client::CAnimation_Tool::Update_ValtanPatternMasterHitAreaPreview()
{
#ifdef _DEBUG
	const shared_ptr<CValtan> Boss = CAnimationTargetService::Resolve_Boss();
	if (nullptr == Boss)
		return;
	if (!m_bValtanPatternMasterPlaying ||
		m_iValtanPatternMasterItem >= m_ValtanPatternMasterPlaylist.size())
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
		m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
	const f32_t fStageSeconds =
		(static_cast<f32_t>(Item.iTimelineStartMs -
			Item.iStageTimelineStartMs) * 0.001f) +
		m_fValtanPatternMasterItemElapsedSeconds;
	Boss->Set_PatternHitAreaPreview(Item.strActionId, fStageSeconds);
#endif
}

bool_t Client::CAnimation_Tool::Load_ValtanAnimationBindingDraft(
	const shared_ptr<Engine::CModel>& pModel)
{
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT Staged;
	std::string Status;
	const std::vector<std::string> AvailableClips = Collect_ClipNames(pModel);
	if (!CValtanPatternAnimationBindingDocument::Load(
		"Valtan", "BOSS_VALTAN", AvailableClips, Staged, Status))
	{
		m_strValtanPatternAnimationBindingStatus =
			"Animation Product reload rejected; the current read-only projection was preserved: " +
			Status;
		return false;
	}
	m_ValtanPatternAnimationBindingDraft = std::move(Staged);
	m_strValtanPatternAnimationBindingBaselineSourceBytes.clear();
	m_bValtanPatternAnimationBindingReady = true;
	m_bValtanPatternAnimationBindingDirty = false;
	m_strValtanPatternAnimationBindingStatus = std::move(Status);
	if (AvailableClips.end() == std::find(
		AvailableClips.begin(), AvailableClips.end(),
		m_strValtanAnimationBindingNewClip))
	{
		m_strValtanAnimationBindingNewClip = AvailableClips.empty() ?
			std::string{} : AvailableClips.front();
	}
	return true;
}

bool_t Client::CAnimation_Tool::Render_ValtanAnimationBindingInspector(
	const shared_ptr<Engine::CModel>& pModel,
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage)
{
	if (!m_bValtanPatternAnimationBindingLoadAttempted)
	{
		m_bValtanPatternAnimationBindingLoadAttempted = true;
		(void)Load_ValtanAnimationBindingDraft(pModel);
	}

	ImGui::SeparatorText("Animation / Presentation Projection");
	ImGui::TextWrapped(
		"AUTHORING OWNER: Data/Valtan/Valtan.presentation.json");
	ImGui::TextDisabled(
		"READ-ONLY GENERATED PRODUCT: Data/Animation/Authored/Valtan/Valtan.patternbindings.json");
	ImGui::TextDisabled(
		"Pattern %s | semantic stage %s | gameplay action %s",
		Pattern.strPatternId.c_str(), Stage.strStageId.c_str(),
		Stage.strActionId.c_str());
	if (!m_strValtanPatternAnimationBindingStatus.empty())
	{
		ImGui::TextWrapped(
			"%s", m_strValtanPatternAnimationBindingStatus.c_str());
	}
	if (ImGui::SmallButton("Reload Read-only Animation Product"))
		(void)Load_ValtanAnimationBindingDraft(pModel);

	ImGui::SeparatorText("Authoring Occurrences / Projected Binding");
	if (Stage.ClipOccurrences.empty())
	{
		ImGui::TextDisabled(
			"No animation occurrence is owned by this presentation stage.");
	}
	else
	{
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
			Stage.ClipOccurrences)
		{
			ImGui::PushID(Occurrence.strClipOccurrenceId.c_str());
			ImGui::TextWrapped(
				"%s | %s | source %u ms | play %u ms @ %.3fx | loop %s",
				Occurrence.strClipOccurrenceId.c_str(),
				Occurrence.strClipName.c_str(), Occurrence.iSourceStartMs,
				Occurrence.iPlayMs, Occurrence.fPlayRate,
				Occurrence.bLoop ? "true" : "false");
			ImGui::PopID();
		}
	}
	const auto ProductBinding = std::find_if(
		m_ValtanPatternAnimationBindingDraft.Bindings.begin(),
		m_ValtanPatternAnimationBindingDraft.Bindings.end(),
		[&Stage](const BOSS_PATTERN_ANIMATION_BINDING& Binding)
		{
			return Binding.strActionId == Stage.strActionId;
		});
	if (!m_bValtanPatternAnimationBindingReady ||
		m_ValtanPatternAnimationBindingDraft.Bindings.end() == ProductBinding)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.35f, 0.25f, 1.f),
			"Projected Product binding is missing or rejected for this action.");
	}
	else
	{
		ImGui::TextDisabled(
			"Projected Product parity: %zu clip(s), playback %s",
			ProductBinding->Clips.size(),
			ProductBinding->bSuppressAnimation ? "NONE" : "CLIP_SEQUENCE");
	}
	ImGui::TextWrapped(
		"Sequence rows are read-only until a typed presentation-source adapter stages them through Save Authoring -> Publish Candidate -> admitted Product reload. Direct Product Save is blocked in CValtanPatternAnimationBindingDocument.");
	return false;

}

void Client::CAnimation_Tool::Render_ValtanCounterWindowInspector(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& SavedStage)
{
	if ("WINDUP" != SavedStage.strStageKind)
		return;

	ImGui::SeparatorText("Counter / Groggy Server Edge");
	if (nullptr == m_pBalanceTool)
	{
		ImGui::TextDisabled(
			"Typed Counter window draft is unavailable: Balance Tool owner is missing.");
		return;
	}

	CBalanceTool::VALTAN_COUNTER_WINDOW_EDIT Counter{};
	std::string CounterStatus;
	if (!m_pBalanceTool->Get_ValtanCounterWindowDraft(
		Pattern.strPatternId, SavedStage.strStageId,
		Counter, CounterStatus))
	{
		ImGui::TextDisabled("Counter draft unavailable: %s", CounterStatus.c_str());
		return;
	}

	std::vector<const VALTAN_STAGE_VIEW*> GroggyTargets;
	for (const VALTAN_STAGE_VIEW& Candidate : Pattern.Stages)
	{
		if ("GROGGY" == Candidate.strStageKind)
			GroggyTargets.push_back(&Candidate);
	}
	const auto SubmitCounter = [&](const CBalanceTool::VALTAN_COUNTER_WINDOW_EDIT& Edit)
	{
		if (m_pBalanceTool->Set_ValtanCounterWindowDraft(
			Pattern.strPatternId, SavedStage.strStageId,
			Edit, CounterStatus))
		{
			(void)m_pBalanceTool->Get_ValtanCounterWindowDraft(
				Pattern.strPatternId, SavedStage.strStageId,
				Counter, CounterStatus);
		}
		m_strValtanPatternMasterStatus = CounterStatus;
	};

	bool_t bEnabled = Counter.enabled;
	ImGui::BeginDisabled(!bEnabled && GroggyTargets.empty());
	if (ImGui::Checkbox("Counter Enabled", &bEnabled))
	{
		CBalanceTool::VALTAN_COUNTER_WINDOW_EDIT Changed = Counter;
		Changed.enabled = bEnabled;
		if (bEnabled && Changed.successStageId.empty() &&
			!GroggyTargets.empty())
		{
			Changed.successStageId = GroggyTargets.front()->strStageId;
			Changed.successActionId = GroggyTargets.front()->strActionId;
		}
		SubmitCounter(Changed);
	}
	ImGui::EndDisabled();
	if (GroggyTargets.empty())
	{
		ImGui::TextDisabled(
			"No same-pattern GROGGY stage/action is available; Counter cannot be enabled.");
	}

	const std::string TargetLabel = Counter.enabled ?
		Counter.successStageId + " / " + Counter.successActionId : "NONE";
	ImGui::TextDisabled("Current Counter target: %s", TargetLabel.c_str());
	ImGui::BeginDisabled(!Counter.enabled);
	if (ImGui::BeginCombo(
		"Counter success GROGGY stage/action", TargetLabel.c_str()))
	{
		for (const VALTAN_STAGE_VIEW* const pTarget : GroggyTargets)
		{
			const bool_t bSelected =
				pTarget->strStageId == Counter.successStageId &&
				pTarget->strActionId == Counter.successActionId;
			const std::string Label = pTarget->strStageId + " / " +
				pTarget->strActionId;
			if (ImGui::Selectable(Label.c_str(), bSelected) && !bSelected)
			{
				CBalanceTool::VALTAN_COUNTER_WINDOW_EDIT Changed = Counter;
				Changed.successStageId = pTarget->strStageId;
				Changed.successActionId = pTarget->strActionId;
				SubmitCounter(Changed);
			}
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	const auto FlagPairState = [](
		const std::vector<VALTAN_STAGE_ACTION_VIEW>& Actions,
		const std::string_view strFlagId)
	{
		std::size_t iRows = 0u;
		std::size_t iEnterTrue = 0u;
		std::size_t iExitFalse = 0u;
		for (const VALTAN_STAGE_ACTION_VIEW& Action : Actions)
		{
			if ("SET_BOSS_FLAG" != Action.strKind ||
				strFlagId != Action.strTargetId)
			{
				continue;
			}
			++iRows;
			if ("ENTER" == Action.strTrigger && Action.fValue > 0.5f)
				++iEnterTrue;
			if ("EXIT" == Action.strTrigger && Action.fValue < 0.5f)
				++iExitFalse;
		}
		if (0u == iRows)
			return 0;
		return 2u == iRows && 1u == iEnterTrue && 1u == iExitFalse ? 1 : -1;
	};

	CBalanceTool::PATTERN_STAGE_EDIT SourceDraft{};
	std::string DraftStatus;
	const bool_t bSourceDraftReady = m_pBalanceTool->Get_ValtanStageDraft(
		Pattern.strPatternId, SavedStage.strStageId,
		SourceDraft, DraftStatus);
	const int32_t iCounterablePair = bSourceDraftReady ?
		FlagPairState(SourceDraft.actions, "boss.flag.counterable") : -1;
	if (Counter.enabled && 1 == iCounterablePair)
	{
		ImGui::TextColored(
			ImVec4(0.35f, 0.82f, 0.45f, 1.f),
			"Counterable ENTER=true / EXIT=false");
		ImGui::TextDisabled("Counterable true | COUNTER_HIT -> %s",
			Counter.successActionId.c_str());
	}
	else if (!Counter.enabled && 0 == iCounterablePair)
	{
		ImGui::TextDisabled("Counterable false | paired rows absent");
	}
	else
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.35f, 0.25f, 1.f),
			"Counterable pair is inconsistent in the current Balance Tool draft.");
	}

	if (Counter.enabled)
	{
		CBalanceTool::PATTERN_STAGE_EDIT GroggyDraft{};
		const bool_t bGroggyDraftReady =
			m_pBalanceTool->Get_ValtanStageDraft(
				Pattern.strPatternId, Counter.successStageId,
				GroggyDraft, DraftStatus);
		const int32_t iGroggyPair = bGroggyDraftReady ?
			FlagPairState(GroggyDraft.actions, "boss.flag.groggy") : -1;
		if (1 == iGroggyPair)
		{
			ImGui::TextColored(
				ImVec4(0.35f, 0.82f, 0.45f, 1.f),
				"Groggy ENTER=true / EXIT=false");
		}
		else
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.35f, 0.25f, 1.f),
				"Groggy flag pair is inconsistent in the current Balance Tool draft.");
		}
	}
	for (const VALTAN_STAGE_BRANCH_VIEW& Branch : SavedStage.Branches)
	{
		if ("COUNTER_HIT" == Branch.strOutcome)
			continue;
		ImGui::TextDisabled(
			"TIMEOUT/default branch %s -> %s (saved non-Counter edge)",
			Branch.strOutcome.c_str(),
			Branch.strNextActionId.has_value() ?
				Branch.strNextActionId->c_str() : "TERMINAL");
	}
}

void Client::CAnimation_Tool::Render_ValtanStageDraftInspector(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& SavedStage)
{
	const std::string& strPatternId = Pattern.strPatternId;
	if (nullptr == m_pBalanceTool)
	{
		ImGui::TextDisabled(
			"Typed Server gameplay draft is unavailable: Balance Tool owner is missing.");
		return;
	}

	CBalanceTool::PATTERN_STAGE_EDIT Draft{};
	std::string DraftStatus;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
		strPatternId, SavedStage.strStageId, Draft, DraftStatus))
	{
		ImGui::TextDisabled("Draft unavailable: %s", DraftStatus.c_str());
		return;
	}
	const auto SubmitDraft = [&]()
	{
		VALTAN_STAGE_VIEW CandidateStage = SavedStage;
		CandidateStage.strActionId = Draft.actionId;
		CandidateStage.iDurationMs = Draft.durationMs;
		CandidateStage.iAuthoringRepeatCount = Draft.animationRepeatCount;
		CandidateStage.strAnimationEndPolicy = Draft.animationEndPolicy;
		CandidateStage.bSuppressAnimation = Draft.animationSlots.empty();
		CandidateStage.ClipOccurrences.clear();
		CandidateStage.ClipOccurrences.reserve(Draft.animationSlots.size());
		for (const CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
			Draft.animationSlots)
		{
			VALTAN_CLIP_OCCURRENCE_VIEW Occurrence;
			Occurrence.strClipOccurrenceId = Slot.clipOccurrenceId;
			Occurrence.strClipName = Slot.clip;
			Occurrence.strMappingBasis = Slot.mappingBasis;
			Occurrence.iSourceStartMs = Slot.sourceStartMs;
			Occurrence.iPlayMs = Slot.playMs;
			Occurrence.fPlayRate = static_cast<f32_t>(Slot.playRate);
			Occurrence.bLoop = Slot.repeatUntilStageEnd;
			CandidateStage.ClipOccurrences.push_back(std::move(Occurrence));
		}
		if (!Validate_ValtanCompositionAnimationStageMutation(
				SavedStage, CandidateStage, DraftStatus))
		{
			m_strValtanPatternMasterStatus =
				"Stage draft rejected before mutation by native Animation admission: " +
				DraftStatus;
			return;
		}
		if (!Validate_ValtanCompositionPatternSoundStageDependencies(
				Pattern, SavedStage, CandidateStage, DraftStatus))
		{
			m_strValtanPatternMasterStatus =
				"Stage draft rejected before mutation: " + DraftStatus;
			return;
		}
		(void)m_pBalanceTool->Set_ValtanStageDraft(
			strPatternId, SavedStage.strStageId, Draft, DraftStatus);
		m_strValtanPatternMasterStatus = DraftStatus;
	};

	ImGui::SeparatorText("Server Stage Clock");
	const uint32_t iOne = 1u;
	const uint32_t iStepMs = 100u;
	const uint32_t iFastStepMs = 1000u;
	ImGui::BeginDisabled(!Draft.durationEditable);
	ImGui::SetNextItemWidth(210.f);
	if (ImGui::InputScalar(
		"Server wall / blank timeline ms",
		ImGuiDataType_U32, &Draft.durationMs,
		&iStepMs, &iFastStepMs, "%u"))
	{
		Draft.durationMs = std::clamp(Draft.durationMs, 1u, 600000u);
		SubmitDraft();
	}
	ImGui::EndDisabled();
	if (!Draft.durationEditable)
	{
		ImGui::TextDisabled(
			"Duration is read-only under this Stage's typed gameplay policy.");
	}
	if (Draft.durationMs != SavedStage.iDurationMs)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.70f, 0.20f, 1.f),
			"Draft %u ms | saved source %u ms",
			Draft.durationMs, SavedStage.iDurationMs);
	}
	if ("VALTAN_HIGH_JUMP" == strPatternId &&
		"AIRBORNE" == SavedStage.strStageId)
	{
		ImGui::TextWrapped(
			"AIRBORNE duration is the boss stage/blank wall-clock. It extends the looping axe-flight animation, but it does not change a spawned axe's lifetime or its local hit atMs.");
		std::uint32_t iDraftAxes = 0u;
		std::uint32_t iSavedAxes = 0u;
		std::uint32_t iArenaRandomAxes = 0u;
		std::uint32_t iMaximumAxeObjects = 0u;
		std::string AxeStatus;
		if (m_pBalanceTool->Get_ValtanHighJumpAxeCountDraft(
			iDraftAxes, iSavedAxes, iArenaRandomAxes,
			iMaximumAxeObjects, AxeStatus))
		{
			const std::uint32_t iOneAxe = 1u;
			ImGui::SetNextItemWidth(210.f);
			if (ImGui::InputScalar(
				"Axes per alive player", ImGuiDataType_U32, &iDraftAxes,
				&iOneAxe, nullptr, "%u"))
			{
				iDraftAxes = std::clamp(iDraftAxes, 1u, 8u);
				(void)m_pBalanceTool->Set_ValtanHighJumpAxeCountDraft(
					iDraftAxes, AxeStatus);
				m_strValtanPatternMasterStatus = AxeStatus;
			}
			ImGui::TextDisabled(
				"PER_ALIVE_PLAYER draft %u | saved %u | arena-random %u | maximum total %u",
				iDraftAxes, iSavedAxes, iArenaRandomAxes, iMaximumAxeObjects);
		}
		else
		{
			ImGui::TextDisabled("Axe-volley draft unavailable: %s", AxeStatus.c_str());
		}
	}

	ImGui::SeparatorText("Stage Identity / Motion");
	ImGui::TextDisabled("Pattern %s | stage %s | action %s",
		strPatternId.c_str(), Draft.stageId.c_str(), Draft.actionId.c_str());
	ImGui::TextDisabled("Kind %s | sequence role %s | animation end %s",
		Draft.stageKind.c_str(), SavedStage.strSequenceRole.c_str(),
		SavedStage.strAnimationEndPolicy.c_str());
	if (SavedStage.Motion.has_value())
	{
		const VALTAN_STAGE_MOTION_VIEW& Motion = *SavedStage.Motion;
		ImGui::TextDisabled(
			"Motion %s | delay %u ms | speed %.3f m/s | distance %.3f m | corner %u | half extents [%.3f, %.3f] m",
			Motion.strKind.c_str(), Motion.iRetargetDelayMs, Motion.fSpeedMps,
			Motion.fDistance, Motion.iCornerIndex,
			Motion.HalfExtentsM[0], Motion.HalfExtentsM[1]);
	}
	else
	{
		ImGui::TextDisabled("Motion: NONE");
	}
	if (Draft.portalRushMotionEditable)
	{
		const auto NormalizePortalRush = [&]()
		{
			Draft.portalRetargetDelayMs = (std::min)(
				Draft.portalRetargetDelayMs, Draft.durationMs - 1u);
			Draft.portalSpeedMps = std::clamp(
				Draft.portalSpeedMps, 0.1, 1000.0);
			const double maximumDistanceM = Draft.portalSpeedMps *
				static_cast<double>(
					Draft.durationMs - Draft.portalRetargetDelayMs) / 1000.0;
			Draft.portalDistanceM = std::clamp(
				Draft.portalDistanceM, 0.000001,
				(std::min)(1000.0, maximumDistanceM));
			const double travelEndMs =
				static_cast<double>(Draft.portalRetargetDelayMs) +
				Draft.portalDistanceM / Draft.portalSpeedMps * 1000.0;
			Draft.hitOffsetsMs.clear();
			for (std::uint32_t offsetMs = Draft.portalRetargetDelayMs;
				static_cast<double>(offsetMs) < travelEndMs &&
				Draft.hitOffsetsMs.size() < 64u; offsetMs += 50u)
			{
				Draft.hitOffsetsMs.push_back(offsetMs);
			}
			Draft.hitCount = static_cast<std::uint32_t>(
				Draft.hitOffsetsMs.size());
			Draft.hitIntervalMs = 0u;
			Draft.hitDelayMs = 0u;
		};
		ImGui::TextWrapped(
			"Typed WARP rush: the Server retargets and waits, then moves the selected distance at the selected speed. Swept hit samples are regenerated every 50 ms only while travel is active.");
		const std::uint32_t iDelayStepMs = 50u;
		const std::uint32_t iDelayFastStepMs = 100u;
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::InputScalar("Retarget / wait delay ms", ImGuiDataType_U32,
			&Draft.portalRetargetDelayMs, &iDelayStepMs,
			&iDelayFastStepMs, "%u"))
		{
			NormalizePortalRush();
			SubmitDraft();
		}
		const double fSpeedStep = 0.5;
		const double fSpeedFastStep = 5.0;
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::InputDouble("Rush speed m/s", &Draft.portalSpeedMps,
			fSpeedStep, fSpeedFastStep, "%.3f"))
		{
			NormalizePortalRush();
			SubmitDraft();
		}
		const double fDistanceStep = 0.25;
		const double fDistanceFastStep = 1.0;
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::InputDouble("Rush distance m", &Draft.portalDistanceM,
			fDistanceStep, fDistanceFastStep, "%.3f"))
		{
			NormalizePortalRush();
			SubmitDraft();
		}
		const double travelMs = Draft.portalDistanceM /
			Draft.portalSpeedMps * 1000.0;
		ImGui::TextDisabled(
			"Travel %.3f ms | finish %.3f / %u ms | swept hits %zu",
			travelMs,
			static_cast<double>(Draft.portalRetargetDelayMs) + travelMs,
			Draft.durationMs, Draft.hitOffsetsMs.size());
	}

	ImGui::SeparatorText("Server Stage Actions");
	if (Draft.actions.empty())
		ImGui::TextDisabled("No typed stage actions.");
	for (std::size_t iAction = 0u; iAction < Draft.actions.size(); ++iAction)
	{
		VALTAN_STAGE_ACTION_VIEW& Action = Draft.actions[iAction];
		const VALTAN_STAGE_ACTION_VIEW* const pSavedAction =
			iAction < SavedStage.Actions.size() ?
				&SavedStage.Actions[iAction] : nullptr;
		ImGui::PushID(static_cast<int>(iAction));
		ImGui::TextWrapped(
			"Typed row %zu | trigger=%s | kind=%s",
			iAction + 1u, Action.strTrigger.c_str(), Action.strKind.c_str());
		ImGui::TextDisabled(
			"targetId=%s | value=%.3f | durationMs=%u",
			Action.strTargetId.c_str(), Action.fValue, Action.iDurationMs);
		if ("SET_BOSS_FLAG" == Action.strKind &&
			"boss.flag.counterable" == Action.strTargetId)
		{
			const char_t* const pCounterableState = Action.fValue > 0.5f ?
				"Counterable true" : "Counterable false";
			ImGui::TextColored(
				ImVec4(0.35f, 0.75f, 1.f, 1.f),
				"%s | exact typed %s row",
				pCounterableState, Action.strTrigger.c_str());
		}
		if ("RELEASE_GRABBED_PLAYERS" == Action.strKind)
		{
			ImGui::TextDisabled(
				"releaseMode=%s | speedMps=%.3f | durationMs=%u | yawOffsetDegrees=%.3f",
				Action.strReleaseMode.c_str(), Action.fSpeedMps,
				Action.iDurationMs, Action.fYawOffsetDegrees);
			static constexpr const char_t* RELEASE_MODES[] = {
				"HOLD", "OPPOSITE_KNOCKBACK", "ARENA_EJECTION" };
			if (ImGui::BeginCombo("Release mode", Action.strReleaseMode.c_str()))
			{
				for (const char_t* const pMode : RELEASE_MODES)
				{
					const bool_t bSelected = Action.strReleaseMode == pMode;
					if (ImGui::Selectable(pMode, bSelected) && !bSelected)
					{
						Action.strReleaseMode = pMode;
						if ("HOLD" == Action.strReleaseMode)
						{
							Action.fSpeedMps = 0.f;
							Action.iDurationMs = 0u;
							Action.fYawOffsetDegrees = 0.f;
						}
						else
						{
							Action.fSpeedMps = (std::max)(Action.fSpeedMps, 0.1f);
							Action.iDurationMs = (std::max)(Action.iDurationMs, 1u);
							if ("OPPOSITE_KNOCKBACK" == Action.strReleaseMode)
								Action.fYawOffsetDegrees = 0.f;
						}
						SubmitDraft();
					}
					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			const bool_t bLaunch = "HOLD" != Action.strReleaseMode;
			const bool_t bYawEditable =
				"ARENA_EJECTION" == Action.strReleaseMode;
			ImGui::BeginDisabled(!bLaunch);
			const float fSpeedStep = 0.1f;
			const float fSpeedFastStep = 1.f;
			ImGui::SetNextItemWidth(190.f);
			if (ImGui::InputFloat("Release speed m/s", &Action.fSpeedMps,
				fSpeedStep, fSpeedFastStep, "%.3f"))
			{
				Action.fSpeedMps = std::clamp(Action.fSpeedMps, 0.1f, 50.f);
				SubmitDraft();
			}
			ImGui::SetNextItemWidth(190.f);
			if (ImGui::InputScalar("Release duration ms", ImGuiDataType_U32,
				&Action.iDurationMs, &iOne, &iStepMs, "%u"))
			{
				Action.iDurationMs = std::clamp(Action.iDurationMs, 1u, 5000u);
				SubmitDraft();
			}
			ImGui::EndDisabled();
			const float fYawStep = 1.f;
			const float fYawFastStep = 15.f;
			ImGui::BeginDisabled(!bYawEditable);
			ImGui::SetNextItemWidth(190.f);
			if (ImGui::InputFloat("Release yaw offset deg",
				&Action.fYawOffsetDegrees, fYawStep, fYawFastStep, "%.3f"))
			{
				Action.fYawOffsetDegrees = std::clamp(
					Action.fYawOffsetDegrees, -180.f, 180.f);
				SubmitDraft();
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Set 180 deg Draft"))
			{
				Action.fYawOffsetDegrees = 180.f;
				SubmitDraft();
			}
			ImGui::EndDisabled();
			const f32_t fSavedYaw = nullptr == pSavedAction ?
				Action.fYawOffsetDegrees : pSavedAction->fYawOffsetDegrees;
			ImGui::TextDisabled(
				"Saved yawOffsetDegrees %.3f | draft %.3f | delta %.3f",
				fSavedYaw, Action.fYawOffsetDegrees,
				Action.fYawOffsetDegrees - fSavedYaw);
			ImGui::TextDisabled(
				"Server release baseline: yawOffsetDegrees 0 ejects backward from boss facing; +180 flips that result forward. Save above, then compare Pattern Offline or Complete Play.");
		}
		else
		{
			ImGui::TextDisabled("Read-only typed action family.");
		}
		ImGui::PopID();
	}

	ImGui::SeparatorText("Branches / Counter Proxy");
	if (SavedStage.Branches.empty())
		ImGui::TextDisabled("Branches: sequential/default edge.");
	for (const VALTAN_STAGE_BRANCH_VIEW& Branch : SavedStage.Branches)
	{
		ImGui::BulletText("%s -> %s", Branch.strOutcome.c_str(),
			Branch.strNextActionId.has_value() ?
				Branch.strNextActionId->c_str() : "TERMINAL");
	}
	if (SavedStage.CounterProxy.has_value())
	{
		const VALTAN_COUNTER_PROXY_VIEW& Counter = *SavedStage.CounterProxy;
		ImGui::TextDisabled(
			"Counter proxy %s | forward %.3f | right %.3f | radius %.3f m",
			Counter.strSpace.c_str(), Counter.fForwardOffsetM,
			Counter.fRightOffsetM, Counter.fRadiusM);
	}
	else
	{
		ImGui::TextDisabled("Counter proxy: NONE");
	}

	ImGui::SeparatorText("Effect Cue Details");
	if (Draft.productCues.empty())
	{
		if ("VALTAN_WARP" == strPatternId &&
			("STEP_01" == SavedStage.strStageId ||
			 "STEP_10" == SavedStage.strStageId))
		{
			ImGui::TextDisabled(
				"No authored portal cue owner exists on this selected start/return stage; no synthetic Effect row is created.");
		}
		else
		{
			ImGui::TextDisabled("No Product Effect cues on this stage.");
		}
	}
	for (std::size_t iCue = 0u; iCue < Draft.productCues.size(); ++iCue)
	{
		VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue = Draft.productCues[iCue];
		ImGui::PushID(Cue.strOccurrenceId.c_str());
		ImGui::TextWrapped("%s", Cue.strEffectAssetId.c_str());
		ImGui::TextDisabled("Binding %s | occurrence %s",
			Cue.strBindingId.c_str(), Cue.strOccurrenceId.c_str());
		ImGui::TextDisabled("V1 alias %s | anchor %s",
			Cue.strV1EffectAssetId.empty() ? "NONE" :
				Cue.strV1EffectAssetId.c_str(), Cue.strAnchorSlotId.c_str());
		ImGui::TextDisabled("Follow %s | stop %s | repeat %s | scale %s",
			Cue.strFollowPolicy.c_str(), Cue.strStopPolicy.c_str(),
			Cue.strRepeatPolicy.c_str(), Cue.strScalePolicy.c_str());
		ImGui::TextDisabled(
			"Position [%.3f %.3f %.3f] | rotation X/Z [%.3f %.3f] | scale [%.3f %.3f %.3f]",
			Cue.LocalTransform.vPosition.x, Cue.LocalTransform.vPosition.y,
			Cue.LocalTransform.vPosition.z,
			Cue.LocalTransform.vRotationDegrees.x,
			Cue.LocalTransform.vRotationDegrees.z,
			Cue.LocalTransform.vScale.x, Cue.LocalTransform.vScale.y,
			Cue.LocalTransform.vScale.z);
		const float fYawStep = 1.f;
		const float fYawFastStep = 15.f;
		ImGui::SetNextItemWidth(190.f);
		if (ImGui::InputFloat("Local Y rotation deg",
			&Cue.LocalTransform.vRotationDegrees.y,
			fYawStep, fYawFastStep, "%.3f"))
		{
			Cue.LocalTransform.vRotationDegrees.y = std::clamp(
				Cue.LocalTransform.vRotationDegrees.y, -180.f, 180.f);
			SubmitDraft();
		}
		if ("arena.center.facing" == Cue.strAnchorSlotId)
		{
			ImGui::TextDisabled(
				"Sector final yaw = Server-selected target facing + this local Y rotation.");
		}
		ImGui::TextDisabled("Source %u..%s | stage clock %s +%u ms",
			Cue.iSourceStartMs,
			Cue.bHasSourceEnd ? std::to_string(Cue.iSourceEndMs).c_str() : "END",
			Cue.bUsesStageClock ? "YES" : "NO", Cue.iStageOffsetMs);
		ImGui::PopID();
	}

	ImGui::SeparatorText("Server Hit / Collider");
	ImGui::TextDisabled("Action %s | stage kind %s",
		Draft.actionId.c_str(), Draft.stageKind.c_str());
	ImGui::TextDisabled("DamageProfile (read-only): %s",
		Draft.damageProfileId.empty() ? "NONE" : Draft.damageProfileId.c_str());
	ImGui::TextDisabled("Player response: %s | attachment: %s",
		Draft.playerResponse.c_str(), Draft.attachmentSlot.c_str());
	if (!Draft.hitEditable)
	{
		ImGui::TextDisabled(
			"Collider NONE. Adding a new Server hit also requires a typed DamageProfile choice, so it remains in Balance Tool.");
	}
	else
	{

	static constexpr const char_t* HIT_SHAPES[] = {
		"CIRCLE", "RING", "CONE", "BOX", "CROSS", "SIX_DIRECTIONS" };
	if (ImGui::BeginCombo("Collider shape", Draft.hitShape.c_str()))
	{
		for (const char_t* const pShape : HIT_SHAPES)
		{
			const bool_t bSelected = Draft.hitShape == pShape;
			if (ImGui::Selectable(pShape, bSelected) && !bSelected)
			{
				const double fExtent = (std::max)({
					1.0, Draft.hitOuterRadius, Draft.hitInnerRadius,
					Draft.hitLength, Draft.hitHalfWidth });
				Draft.hitShape = pShape;
				Draft.hitOuterRadius = 0.0;
				Draft.hitInnerRadius = 0.0;
				Draft.hitAngleDegrees = 0.0;
				Draft.hitLength = 0.0;
				Draft.hitHalfWidth = 0.0;
				if ("CIRCLE" == Draft.hitShape)
					Draft.hitOuterRadius = fExtent;
				else if ("RING" == Draft.hitShape)
				{
					Draft.hitOuterRadius = fExtent;
					Draft.hitInnerRadius = (std::max)(0.1, fExtent * 0.5);
				}
				else if ("CONE" == Draft.hitShape)
				{
					Draft.hitAngleDegrees = 90.0;
					Draft.hitLength = fExtent;
				}
				else
				{
					Draft.hitLength = fExtent;
					Draft.hitHalfWidth = (std::max)(0.1, fExtent * 0.5);
				}
				SubmitDraft();
			}
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	const auto EditGeometry = [&](const char_t* const pLabel,
		double& fValue, const double fMinimum, const double fMaximum)
	{
		const double fStep = 0.1;
		const double fFastStep = 1.0;
		ImGui::SetNextItemWidth(210.f);
		if (!ImGui::InputDouble(
			pLabel, &fValue, fStep, fFastStep, "%.3f"))
		{
			return;
		}
		fValue = std::clamp(fValue, fMinimum, fMaximum);
		SubmitDraft();
	};
	if ("CIRCLE" == Draft.hitShape || "RING" == Draft.hitShape)
	{
		const double fMinimumOuter = "RING" == Draft.hitShape ?
			Draft.hitInnerRadius + 0.001 : 0.001;
		EditGeometry("Outer radius m", Draft.hitOuterRadius,
			fMinimumOuter, 1000.0);
	}
	if ("RING" == Draft.hitShape)
	{
		EditGeometry("Inner radius m", Draft.hitInnerRadius,
			0.001, (std::max)(0.001, Draft.hitOuterRadius - 0.001));
	}
	if ("CONE" == Draft.hitShape)
	{
		EditGeometry("Angle degrees", Draft.hitAngleDegrees, 0.001, 180.0);
		EditGeometry("Length m", Draft.hitLength, 0.001, 1000.0);
	}
	if ("BOX" == Draft.hitShape || "CROSS" == Draft.hitShape ||
		"SIX_DIRECTIONS" == Draft.hitShape)
	{
		EditGeometry("Length m", Draft.hitLength, 0.001, 1000.0);
		EditGeometry("Half width m", Draft.hitHalfWidth, 0.001, 1000.0);
	}

	ImGui::SeparatorText("Server Hit Schedule");
	if (!Draft.hitOffsetsMs.empty())
	{
		ImGui::TextDisabled(
			"EXPLICIT_OFFSETS is read-only here; preserve the source contact ordering in Balance Tool.");
		for (std::size_t iOffset = 0u;
			iOffset < Draft.hitOffsetsMs.size(); ++iOffset)
		{
			ImGui::BulletText("hit %zu at stage +%u ms",
				iOffset + 1u, Draft.hitOffsetsMs[iOffset]);
		}
	}
	else
	{
		ImGui::SetNextItemWidth(180.f);
		if (ImGui::InputScalar(
			"Hit count", ImGuiDataType_U32, &Draft.hitCount,
			&iOne, nullptr, "%u"))
		{
			Draft.hitCount = std::clamp(Draft.hitCount, 1u, 64u);
			if (1u == Draft.hitCount)
				Draft.hitIntervalMs = 0u;
			else if (0u == Draft.hitIntervalMs)
				Draft.hitIntervalMs = 1u;
			SubmitDraft();
		}
		const uint32_t iMaximumDelay = Draft.durationMs - 1u;
		ImGui::SetNextItemWidth(180.f);
		if (ImGui::InputScalar(
			"First hit offset ms", ImGuiDataType_U32, &Draft.hitDelayMs,
			&iOne, &iStepMs, "%u"))
		{
			Draft.hitDelayMs = (std::min)(Draft.hitDelayMs, iMaximumDelay);
			SubmitDraft();
		}
		ImGui::BeginDisabled(1u == Draft.hitCount);
		ImGui::SetNextItemWidth(180.f);
		if (ImGui::InputScalar(
			"Hit interval ms", ImGuiDataType_U32, &Draft.hitIntervalMs,
			&iOne, &iStepMs, "%u"))
		{
			const uint32_t iRemaining = Draft.durationMs - 1u -
				(std::min)(Draft.hitDelayMs, Draft.durationMs - 1u);
			const uint32_t iMaximumInterval = Draft.hitCount > 1u ?
				iRemaining / (Draft.hitCount - 1u) : 0u;
			Draft.hitIntervalMs = std::clamp(
				Draft.hitIntervalMs, 1u, (std::max)(1u, iMaximumInterval));
			SubmitDraft();
		}
		ImGui::EndDisabled();
	}

	ImGui::SeparatorText("Server Player Reaction");
	const bool_t bCapture = "CAPTURE" == Draft.playerResponse;
	ImGui::BeginDisabled(bCapture);
	double fPushStep = 0.1;
	double fPushFastStep = 1.0;
	ImGui::SetNextItemWidth(190.f);
	if (ImGui::InputDouble(
		"Push range m", &Draft.pushRangeM,
		fPushStep, fPushFastStep, "%.3f"))
	{
		Draft.pushRangeM = std::clamp(Draft.pushRangeM, -20.0, 20.0);
		if (std::abs(Draft.pushRangeM) < 0.000001)
		{
			Draft.pushRangeM = 0.0;
			Draft.pushMs = 0u;
		}
		else if (0u == Draft.pushMs)
		{
			Draft.pushMs = 1u;
		}
		SubmitDraft();
	}
	ImGui::BeginDisabled(0.0 == Draft.pushRangeM);
	ImGui::SetNextItemWidth(190.f);
	if (ImGui::InputScalar(
		"Push duration ms", ImGuiDataType_U32, &Draft.pushMs,
		&iOne, &iStepMs, "%u"))
	{
		Draft.pushMs = std::clamp(Draft.pushMs, 1u, 600000u);
		SubmitDraft();
	}
	ImGui::EndDisabled();
	if (ImGui::Checkbox("Knockdown", &Draft.knockdown))
	{
		Draft.downMs = Draft.knockdown ?
			(std::max)(Draft.downMs, 1u) : 0u;
		SubmitDraft();
	}
	ImGui::BeginDisabled(!Draft.knockdown);
	ImGui::SetNextItemWidth(190.f);
	if (ImGui::InputScalar(
		"Down duration ms", ImGuiDataType_U32, &Draft.downMs,
		&iOne, &iStepMs, "%u"))
	{
		Draft.downMs = std::clamp(Draft.downMs, 1u, 600000u);
		SubmitDraft();
	}
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	const char_t* const pDirectionPolicy = 0.0 == Draft.pushRangeM ? "NONE" :
		(Draft.pushRangeM > 0.0 ? "AWAY_FROM_HIT_SOURCE" :
			"TOWARD_HIT_SOURCE");
	const double fPushSpeedMps = 0u == Draft.pushMs ? 0.0 :
		std::abs(Draft.pushRangeM) * 1000.0 /
		static_cast<double>(Draft.pushMs);
	ImGui::TextDisabled(
		"Direction policy (Server-derived): %s | derived speed %.3f m/s",
		pDirectionPolicy, fPushSpeedMps);
	if (bCapture)
	{
		ImGui::TextDisabled(
			"CAPTURE owns attachment instead of push/knockdown; reaction values are read-only zero.");
	}
	}
}

bool_t Client::CAnimation_Tool::Render_ValtanPatternSoundInspector(
	const shared_ptr<Engine::CModel>& pModel,
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage)
{
	bool_t bReloadJoinedWorkbenchAfterSave = false;
	ImGui::SeparatorText("Pattern Sound Typed Owner");
	ImGui::TextWrapped(
		"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json");
	ImGui::TextDisabled(
		"Selection %s / %s | action %s",
		Pattern.strPatternId.c_str(), Stage.strStageId.c_str(),
		Stage.strActionId.c_str());
	const std::unordered_map<std::string, f32_t>
		ClipSourceDurationSecondsByName =
			CollectModelClipSourceDurationSeconds(pModel);

	std::string SoundLifecycleStatus;
	const bool_t bSoundSourceCommitAdmitted =
		Can_CommitValtanCompositionPatternSoundGeneration(
			SoundLifecycleStatus);
	ImGui::BeginDisabled(!bSoundSourceCommitAdmitted);
	if (ImGui::SmallButton(
		m_bValtanPatternSoundCuesDirty ?
			"Discard / Reload Pattern Sound" : "Reload Pattern Sound"))
	{
		(void)Reload_ValtanPatternSoundCues();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(
		!bSoundSourceCommitAdmitted ||
		!m_bValtanPatternSoundCuesReady ||
		!m_bValtanPatternSoundCuesDirty);
	if (ImGui::SmallButton("Save Pattern Sound"))
	{
		std::string SaveStatus;
		bReloadJoinedWorkbenchAfterSave =
			Save_ValtanCompositionPatternSounds(SaveStatus);
		m_strValtanPatternSoundCueStatus = std::move(SaveStatus);
	}
	ImGui::EndDisabled();
	if (!bSoundSourceCommitAdmitted && !SoundLifecycleStatus.empty())
		ImGui::TextWrapped("%s", SoundLifecycleStatus.c_str());
	if (!m_strValtanPatternSoundCueStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanPatternSoundCueStatus.c_str());

	if (!m_bValtanPatternSoundCuesReady)
	{
		ImGui::TextDisabled(
			"Strict Pattern Sound authoring source is not admitted.");
		return bReloadJoinedWorkbenchAfterSave;
	}

	const std::vector<std::string> AllSoundEvents =
		CSoundCueCatalog::Collect_EventNames("Valtan");
	std::vector<const std::string*> AuthoringEvents;
	AuthoringEvents.reserve(AllSoundEvents.size());
	for (const std::string& EventName : AllSoundEvents)
	{
		if (IsValtanSoundAuthoringCandidate(EventName))
			AuthoringEvents.push_back(&EventName);
	}
	std::vector<ACTION_PRESENTATION_CLIP_TIMING> RuntimeClipTimings;
	RuntimeClipTimings.reserve(Stage.ClipOccurrences.size());
	bool_t bRuntimeClipTimingsReady = !Stage.ClipOccurrences.empty();
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Stage.ClipOccurrences)
	{
		const auto Duration =
			ClipSourceDurationSecondsByName.find(Clip.strClipName);
		if (ClipSourceDurationSecondsByName.end() == Duration)
		{
			bRuntimeClipTimingsReady = false;
			break;
		}
		ACTION_PRESENTATION_CLIP_TIMING Timing{
			Duration->second,
			Clip.iPlayMs,
			Clip.fPlayRate,
			Clip.bLoop,
			static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f };
		f32_t fSourceDurationSeconds = 0.f;
		f32_t fWallDurationSeconds = 0.f;
		if (!CActionPresentationTimeline::Resolve_ClipDuration(
				Timing, fSourceDurationSeconds, fWallDurationSeconds))
		{
			bRuntimeClipTimingsReady = false;
			break;
		}
		RuntimeClipTimings.push_back(Timing);
	}

	ImGui::SeparatorText("Add Pattern Sound Row");
	const auto FindAddClip = [this, &Stage]()
	{
		return std::find_if(
			Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
			[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
			{
				return Candidate.strClipOccurrenceId ==
					m_strValtanPatternSoundAddClipOccurrenceId;
			});
	};
	auto AddClip = FindAddClip();
	if (Stage.ClipOccurrences.end() == AddClip &&
		!Stage.ClipOccurrences.empty())
	{
		m_strValtanPatternSoundAddClipOccurrenceId =
			Stage.ClipOccurrences.front().strClipOccurrenceId;
		m_iValtanPatternSoundAddStartMs =
			Stage.ClipOccurrences.front().iSourceStartMs;
		m_eValtanPatternSoundAddRepeatPolicy =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		AddClip = FindAddClip();
	}
	const auto FindAddEvent = [this, &AuthoringEvents]()
	{
		return std::find_if(
			AuthoringEvents.begin(), AuthoringEvents.end(),
			[this](const std::string* const pCandidate)
			{
				return nullptr != pCandidate &&
					*pCandidate == m_strValtanPatternSoundAddEvent;
			});
	};
	auto AddEvent = FindAddEvent();
	if (AuthoringEvents.end() == AddEvent && !AuthoringEvents.empty())
	{
		m_strValtanPatternSoundAddEvent = *AuthoringEvents.front();
		AddEvent = FindAddEvent();
	}

	const char_t* const pAddClipPreview =
		Stage.ClipOccurrences.end() == AddClip ?
			"Select an admitted clip occurrence" :
			AddClip->strClipOccurrenceId.c_str();
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::BeginCombo(
		"Clip Occurrence##AddPatternSound", pAddClipPreview))
	{
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate :
			Stage.ClipOccurrences)
		{
			const bool_t bSelected =
				Candidate.strClipOccurrenceId ==
					m_strValtanPatternSoundAddClipOccurrenceId;
			const std::string Label = Candidate.strClipOccurrenceId +
				" | " + Candidate.strClipName;
			if (ImGui::Selectable(Label.c_str(), bSelected) && !bSelected)
			{
				m_strValtanPatternSoundAddClipOccurrenceId =
					Candidate.strClipOccurrenceId;
				m_iValtanPatternSoundAddStartMs = Candidate.iSourceStartMs;
				m_eValtanPatternSoundAddRepeatPolicy =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
			}
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	AddClip = FindAddClip();
	const char_t* const pAddEventPreview =
		AuthoringEvents.end() == AddEvent ?
			"Select a validated Valtan event" :
			m_strValtanPatternSoundAddEvent.c_str();
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::BeginCombo(
		"Sound Event##AddPatternSound", pAddEventPreview))
	{
		for (const std::string* const pEventName : AuthoringEvents)
		{
			const bool_t bSelected =
				*pEventName == m_strValtanPatternSoundAddEvent;
			if (ImGui::Selectable(pEventName->c_str(), bSelected) && !bSelected)
				m_strValtanPatternSoundAddEvent = *pEventName;
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	AddEvent = FindAddEvent();
	ImGui::TextDisabled(
		"Sound Bank (derived): %s",
		std::string(ValtanSoundBankForEvent(
			m_strValtanPatternSoundAddEvent)).c_str());

	std::uint32_t iAddMinimumStartMs = 0u;
	std::uint32_t iAddMaximumStartMs = 0u;
	f64_t fAddModelDurationMilliseconds = 0.0;
	f64_t fAddRemainingStageWallMilliseconds = 0.0;
	bool_t bAddRuntimeWindowReady = false;
	if (Stage.ClipOccurrences.end() != AddClip)
	{
		iAddMinimumStartMs = AddClip->iSourceStartMs;
		const std::size_t iClipIndex = static_cast<std::size_t>(
			AddClip - Stage.ClipOccurrences.begin());
		f32_t fResolvedSourceDurationSeconds = 0.f;
		f32_t fResolvedWallDurationSeconds = 0.f;
		f32_t fClipStageWallStartSeconds = 0.f;
		bAddRuntimeWindowReady = bRuntimeClipTimingsReady &&
			iClipIndex < RuntimeClipTimings.size() &&
			CActionPresentationTimeline::Resolve_ClipDuration(
				RuntimeClipTimings[iClipIndex],
				fResolvedSourceDurationSeconds,
				fResolvedWallDurationSeconds) &&
			CActionPresentationTimeline::Resolve_CueWallOffset(
				RuntimeClipTimings, iClipIndex,
				RuntimeClipTimings[iClipIndex].fSourceStartSeconds,
				0u, fClipStageWallStartSeconds);
		const f64_t fRemainingStageWallSeconds =
			bAddRuntimeWindowReady ? (std::max)(0.0,
				static_cast<f64_t>(Stage.iDurationMs) * 0.001 -
				static_cast<f64_t>(fClipStageWallStartSeconds)) : 0.0;
		const f64_t fResolvedSourceEndSeconds =
			static_cast<f64_t>(AddClip->iSourceStartMs) * 0.001 +
			static_cast<f64_t>(fResolvedSourceDurationSeconds);
		const f64_t fStageSourceEndSeconds =
			static_cast<f64_t>(AddClip->iSourceStartMs) * 0.001 +
			fRemainingStageWallSeconds *
				static_cast<f64_t>(AddClip->fPlayRate);
		const f64_t fEffectiveSourceEndMilliseconds = 1000.0 *
			(std::min)(fResolvedSourceEndSeconds, fStageSourceEndSeconds);
		const std::uint64_t iEffectiveSourceEndExclusiveMs =
			bAddRuntimeWindowReady &&
			std::isfinite(fEffectiveSourceEndMilliseconds) &&
			fEffectiveSourceEndMilliseconds > 0.0 ?
				static_cast<std::uint64_t>(
					std::ceil(fEffectiveSourceEndMilliseconds)) : 0u;
		bAddRuntimeWindowReady =
			iEffectiveSourceEndExclusiveMs > iAddMinimumStartMs;
		iAddMaximumStartMs = bAddRuntimeWindowReady ?
			static_cast<std::uint32_t>((std::min)(
				iEffectiveSourceEndExclusiveMs - 1u,
				static_cast<std::uint64_t>(
					(std::numeric_limits<std::uint32_t>::max)()))) :
			iAddMinimumStartMs;
		const auto ModelDuration =
			ClipSourceDurationSecondsByName.find(AddClip->strClipName);
		if (ClipSourceDurationSecondsByName.end() != ModelDuration)
			fAddModelDurationMilliseconds =
				static_cast<f64_t>(ModelDuration->second) * 1000.0;
		fAddRemainingStageWallMilliseconds =
			fRemainingStageWallSeconds * 1000.0;
		m_iValtanPatternSoundAddStartMs = std::clamp(
			m_iValtanPatternSoundAddStartMs,
			iAddMinimumStartMs, iAddMaximumStartMs);
		if (!AddClip->bLoop)
		{
			m_eValtanPatternSoundAddRepeatPolicy =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		}
	}
	const std::uint32_t iStepMs = 1u;
	const std::uint32_t iFastStepMs = 100u;
	ImGui::BeginDisabled(!bAddRuntimeWindowReady);
	ImGui::SetNextItemWidth(210.f);
	if (ImGui::InputScalar(
		"startMs##AddPatternSound", ImGuiDataType_U32,
		&m_iValtanPatternSoundAddStartMs,
		&iStepMs, &iFastStepMs, "%u"))
	{
		m_iValtanPatternSoundAddStartMs = std::clamp(
			m_iValtanPatternSoundAddStartMs,
			iAddMinimumStartMs, iAddMaximumStartMs);
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Runtime-equivalent source window: %u..%u ms | model end %.3f ms | stage wall remaining %.3f ms",
		iAddMinimumStartMs, iAddMaximumStartMs,
		fAddModelDurationMilliseconds,
		fAddRemainingStageWallMilliseconds);
	const char_t* const pAddRepeatLabel =
		VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
			m_eValtanPatternSoundAddRepeatPolicy ? "each_loop" : "once";
	ImGui::SetNextItemWidth(210.f);
	if (ImGui::BeginCombo(
		"Repeat Policy##AddPatternSound", pAddRepeatLabel))
	{
		const bool_t bOnce =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE ==
				m_eValtanPatternSoundAddRepeatPolicy;
		if (ImGui::Selectable("once", bOnce) && !bOnce)
		{
			m_eValtanPatternSoundAddRepeatPolicy =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		}
		ImGui::BeginDisabled(
			Stage.ClipOccurrences.end() == AddClip || !AddClip->bLoop);
		const bool_t bEachLoop =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				m_eValtanPatternSoundAddRepeatPolicy;
		if (ImGui::Selectable("each_loop", bEachLoop) && !bEachLoop)
		{
			m_eValtanPatternSoundAddRepeatPolicy =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
		}
		ImGui::EndDisabled();
		ImGui::EndCombo();
	}
	const bool_t bCanAddPatternSoundRow =
		Stage.ClipOccurrences.end() != AddClip &&
		AuthoringEvents.end() != AddEvent && bAddRuntimeWindowReady;
	ImGui::BeginDisabled(!bCanAddPatternSoundRow);
	if (ImGui::SmallButton("Add Exact Pattern Sound Row"))
	{
		VALTAN_PATTERN_SOUND_CUE_ADD_ROW Row;
		Row.strPatternId = Pattern.strPatternId;
		Row.strStageId = Stage.strStageId;
		Row.strActionId = Stage.strActionId;
		Row.strClipOccurrenceId = AddClip->strClipOccurrenceId;
		Row.strSoundEvent = m_strValtanPatternSoundAddEvent;
		Row.strSoundBank = std::string(
			ValtanSoundBankForEvent(Row.strSoundEvent));
		Row.eRepeatPolicy = m_eValtanPatternSoundAddRepeatPolicy;
		Row.iStartMs = m_iValtanPatternSoundAddStartMs;
		VALTAN_PATTERN_SOUND_CUE_ROW_ID CreatedRowId;
		std::string AddStatus;
		if (CValtanPatternSoundCueDocument::Add_AuthoringRow(
			m_ValtanPatternSoundCues, Row,
			ClipSourceDurationSecondsByName, CreatedRowId, AddStatus))
		{
			m_bValtanPatternSoundCuesDirty = true;
			++m_iValtanPatternSoundDraftGeneration;
			m_strValtanPatternSoundCueStatus =
				"UNSAVED Pattern Sound row added: " +
				CreatedRowId.strBindingId + " / " +
				CreatedRowId.strOccurrenceId + ". " + AddStatus;
		}
		else
		{
			m_strValtanPatternSoundCueStatus =
				"Pattern Sound Add Row rejected; admitted draft preserved: " +
				AddStatus;
		}
	}
	ImGui::EndDisabled();
	if (!bAddRuntimeWindowReady)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.35f, 0.25f, 1.f),
			"Add is blocked until the selected clip has a model-backed source remainder inside the Server stage wall.");
	}

	std::size_t iSelectedRows = 0u;
	VALTAN_PATTERN_SOUND_CUE_ROW_ID PendingRemoveRowId;
	bool_t bHasPendingRemoveRow = false;
	for (VALTAN_PATTERN_SOUND_CUE& Cue : m_ValtanPatternSoundCues.Cues)
	{
		if (Cue.strPatternId != Pattern.strPatternId ||
			Cue.strStageId != Stage.strStageId)
		{
			continue;
		}
		++iSelectedRows;
		ImGui::PushID(Cue.strBindingId.c_str());
		ImGui::Separator();
		ImGui::TextDisabled("bindingId: %s", Cue.strBindingId.c_str());
		ImGui::TextDisabled("occurrenceId: %s", Cue.strOccurrenceId.c_str());
		ImGui::TextDisabled("patternId: %s", Cue.strPatternId.c_str());
		ImGui::TextDisabled("stageId: %s", Cue.strStageId.c_str());
		ImGui::TextDisabled("actionId: %s", Cue.strActionId.c_str());
		ImGui::TextDisabled(
			"clipOccurrenceId: %s", Cue.strClipOccurrenceId.c_str());

		const auto Clip = std::find_if(
			Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
			[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
			{
				return Candidate.strClipOccurrenceId ==
					Cue.strClipOccurrenceId;
			});
		const bool_t bClipReady = Stage.ClipOccurrences.end() != Clip;
		const bool_t bLoopClip = bClipReady && Clip->bLoop;

		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::BeginCombo("Sound Event", Cue.strSoundEvent.c_str()))
		{
			for (const std::string* const pEventName : AuthoringEvents)
			{
				const bool_t bSelected = *pEventName == Cue.strSoundEvent;
				if (ImGui::Selectable(pEventName->c_str(), bSelected) &&
					!bSelected)
				{
					Cue.strSoundEvent = *pEventName;
					Cue.strSoundBank = std::string(
						ValtanSoundBankForEvent(*pEventName));
					m_bValtanPatternSoundCuesDirty = true;
					++m_iValtanPatternSoundDraftGeneration;
					m_strValtanPatternSoundCueStatus =
						"UNSAVED Pattern Sound event: " + Cue.strBindingId +
						" -> " + Cue.strSoundEvent;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::TextDisabled(
			"Sound Bank (derived from event prefix): %s",
			Cue.strSoundBank.c_str());

		if (bClipReady)
		{
			const std::uint32_t iMinimumStartMs = Clip->iSourceStartMs;
			const std::size_t iClipIndex = static_cast<std::size_t>(
				Clip - Stage.ClipOccurrences.begin());
			f32_t fResolvedSourceDurationSeconds = 0.f;
			f32_t fResolvedWallDurationSeconds = 0.f;
			f32_t fClipStageWallStartSeconds = 0.f;
			const bool_t bRuntimeWindowReady =
				bRuntimeClipTimingsReady &&
				iClipIndex < RuntimeClipTimings.size() &&
				CActionPresentationTimeline::Resolve_ClipDuration(
					RuntimeClipTimings[iClipIndex],
					fResolvedSourceDurationSeconds,
					fResolvedWallDurationSeconds) &&
				CActionPresentationTimeline::Resolve_CueWallOffset(
					RuntimeClipTimings, iClipIndex,
					RuntimeClipTimings[iClipIndex].fSourceStartSeconds,
					0u, fClipStageWallStartSeconds);
			const f64_t fSavedStageWallSeconds =
				static_cast<f64_t>(Cue.iStageDurationMs) * 0.001;
			const f64_t fRemainingStageWallSeconds = bRuntimeWindowReady ?
				(std::max)(0.0, fSavedStageWallSeconds -
					static_cast<f64_t>(fClipStageWallStartSeconds)) : 0.0;
			const f64_t fResolvedSourceEndSeconds =
				static_cast<f64_t>(Clip->iSourceStartMs) * 0.001 +
				static_cast<f64_t>(fResolvedSourceDurationSeconds);
			const f64_t fStageSourceEndSeconds =
				static_cast<f64_t>(Clip->iSourceStartMs) * 0.001 +
				fRemainingStageWallSeconds *
					static_cast<f64_t>(Clip->fPlayRate);
			const f64_t fEffectiveSourceEndMilliseconds = 1000.0 *
				(std::min)(fResolvedSourceEndSeconds, fStageSourceEndSeconds);
			const std::uint64_t iEffectiveSourceEndExclusiveMs =
				bRuntimeWindowReady &&
				std::isfinite(fEffectiveSourceEndMilliseconds) &&
				fEffectiveSourceEndMilliseconds > 0.0 ?
				static_cast<std::uint64_t>(
					std::ceil(fEffectiveSourceEndMilliseconds)) : 0u;
			const bool_t bEditableRuntimeWindow =
				iEffectiveSourceEndExclusiveMs > iMinimumStartMs;
			const std::uint32_t iMaximumStartMs =
				bEditableRuntimeWindow ? static_cast<std::uint32_t>((std::min)(
					iEffectiveSourceEndExclusiveMs - 1u,
					static_cast<std::uint64_t>(
						(std::numeric_limits<std::uint32_t>::max)()))) :
				iMinimumStartMs;
			const std::uint32_t iStepMs = 1u;
			const std::uint32_t iFastStepMs = 100u;
			ImGui::BeginDisabled(!bEditableRuntimeWindow);
			ImGui::SetNextItemWidth(210.f);
			if (ImGui::InputScalar(
				"startMs", ImGuiDataType_U32, &Cue.iStartMs,
				&iStepMs, &iFastStepMs, "%u"))
			{
				Cue.iStartMs = std::clamp(
					Cue.iStartMs, iMinimumStartMs, iMaximumStartMs);
				m_bValtanPatternSoundCuesDirty = true;
				++m_iValtanPatternSoundDraftGeneration;
				m_strValtanPatternSoundCueStatus =
					"UNSAVED Pattern Sound startMs: " + Cue.strBindingId;
			}
			ImGui::EndDisabled();
			const auto ModelDuration =
				ClipSourceDurationSecondsByName.find(Clip->strClipName);
			const f64_t fModelDurationMilliseconds =
				ClipSourceDurationSecondsByName.end() != ModelDuration ?
					static_cast<f64_t>(ModelDuration->second) * 1000.0 : 0.0;
			ImGui::TextDisabled(
				"Runtime-equivalent source window: %u..%u ms | clip loop=%s",
				iMinimumStartMs, iMaximumStartMs,
				bLoopClip ? "true" : "false");
			ImGui::TextDisabled(
				"Model source end %.3f ms | stage wall remaining %.3f / %u ms",
				fModelDurationMilliseconds,
				fRemainingStageWallSeconds * 1000.0,
				Cue.iStageDurationMs);
			if (!bEditableRuntimeWindow)
			{
				ImGui::TextColored(
					ImVec4(1.f, 0.35f, 0.25f, 1.f),
					"No editable startMs survives the current model source window and saved Server stage wall.");
			}
		}
		else
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.35f, 0.25f, 1.f),
				"The stable clip occurrence is absent from this admitted stage.");
		}

		const char_t* const pRepeatLabel =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				Cue.eRepeatPolicy ? "each_loop" : "once";
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::BeginCombo("Repeat Policy", pRepeatLabel))
		{
			const bool_t bOnce = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE ==
				Cue.eRepeatPolicy;
			if (ImGui::Selectable("once", bOnce) && !bOnce)
			{
				Cue.eRepeatPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
				m_bValtanPatternSoundCuesDirty = true;
				++m_iValtanPatternSoundDraftGeneration;
				m_strValtanPatternSoundCueStatus =
					"UNSAVED Pattern Sound repeatPolicy: " + Cue.strBindingId;
			}
			ImGui::BeginDisabled(!bLoopClip);
			const bool_t bEachLoop =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					Cue.eRepeatPolicy;
			if (ImGui::Selectable("each_loop", bEachLoop) && !bEachLoop)
			{
				Cue.eRepeatPolicy =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
				m_bValtanPatternSoundCuesDirty = true;
				++m_iValtanPatternSoundDraftGeneration;
				m_strValtanPatternSoundCueStatus =
					"UNSAVED Pattern Sound repeatPolicy: " + Cue.strBindingId;
			}
			ImGui::EndDisabled();
			ImGui::EndCombo();
		}

		const std::vector<std::string>& Variants =
			CSoundCueCatalog::Find_Variants("Valtan", Cue.strSoundEvent);
		for (std::size_t iVariant = 0u; iVariant < Variants.size(); ++iVariant)
		{
			ImGui::PushID(static_cast<int32_t>(iVariant));
			if (ImGui::SmallButton("Preview Selected WAV"))
				(void)Preview_ValtanSoundAsset(Variants[iVariant]);
			ImGui::SameLine();
			ImGui::TextDisabled("%s", Variants[iVariant].c_str());
			ImGui::PopID();
		}
		if (ImGui::SmallButton("Remove Exact Pattern Sound Row"))
		{
			PendingRemoveRowId.strBindingId = Cue.strBindingId;
			PendingRemoveRowId.strOccurrenceId = Cue.strOccurrenceId;
			bHasPendingRemoveRow = true;
		}
		ImGui::PopID();
	}
	if (bHasPendingRemoveRow)
	{
		std::string RemoveStatus;
		if (CValtanPatternSoundCueDocument::Remove_AuthoringRow(
			m_ValtanPatternSoundCues, PendingRemoveRowId, RemoveStatus))
		{
			m_bValtanPatternSoundCuesDirty = true;
			++m_iValtanPatternSoundDraftGeneration;
			m_strValtanPatternSoundCueStatus =
				"UNSAVED Pattern Sound row removed: " +
				PendingRemoveRowId.strBindingId + " / " +
				PendingRemoveRowId.strOccurrenceId + ". " + RemoveStatus;
		}
		else
		{
			m_strValtanPatternSoundCueStatus =
				"Pattern Sound Remove Row rejected; admitted draft preserved: " +
				RemoveStatus;
		}
	}
	if (0u == iSelectedRows)
	{
		ImGui::TextDisabled(
			"No exact pattern/stage-qualified Pattern Sound row is admitted for this selection.");
	}
	ImGui::TextDisabled(
		"Inventory is typed: Add allocates deterministic stable IDs; Remove requires the exact bindingId + occurrenceId pair. Save remains a separate CAS boundary.");
	return bReloadJoinedWorkbenchAfterSave;
}

void Client::CAnimation_Tool::Render_ValtanPresentationLanes(
	const VALTAN_PATTERN_VIEW& Pattern,
	const std::string_view strStageFilter)
{
	struct JOINED_COMBAT_OBJECT_SOUND final
	{
		VALTAN_COMBAT_OBJECT_SOUND_CUE* pCue = nullptr;
		uint32_t iHitOffsetMs = 0u;
	};
	const std::vector<std::string> ValtanSoundEventNames =
		CSoundCueCatalog::Collect_EventNames("Valtan");

	ImGui::SeparatorText("Joined Presentation Lanes");
	ImGui::TextDisabled(
		"Server Stage -> Animation -> Effect -> Sound Asset -> Camera/Shake -> World Event -> Combat Object");
	if (!m_strValtanPatternSoundCueStatus.empty())
		ImGui::TextWrapped("Sound: %s", m_strValtanPatternSoundCueStatus.c_str());
	if (!m_strValtanPatternShakeCueStatus.empty())
		ImGui::TextWrapped("Camera/Shake: %s",
			m_strValtanPatternShakeCueStatus.c_str());
	if (!m_strValtanCombatObjectSoundCueStatus.empty())
		ImGui::TextWrapped("Server-hit Sound: %s",
			m_strValtanCombatObjectSoundCueStatus.c_str());

	for (std::size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[iStage];
		if (!strStageFilter.empty() &&
			Stage.strStageId != strStageFilter)
		{
			continue;
		}
		std::vector<const VALTAN_PATTERN_SOUND_CUE*> SoundCues;
		if (m_bValtanPatternSoundCuesReady)
		{
			for (const VALTAN_PATTERN_SOUND_CUE& Cue :
				m_ValtanPatternSoundCues.Cues)
			{
				if (Cue.strPatternId == Pattern.strPatternId &&
					Cue.strStageId == Stage.strStageId)
				{
					SoundCues.push_back(&Cue);
				}
			}
		}
		std::vector<const VALTAN_PATTERN_SHAKE_CUE*> ShakeCues;
		if (m_bValtanPatternShakeCuesReady)
		{
			for (const VALTAN_PATTERN_SHAKE_CUE& Cue :
				m_ValtanPatternShakeCues.Cues)
			{
				if (Cue.strPatternId == Pattern.strPatternId &&
					Cue.strStageId == Stage.strStageId)
				{
					ShakeCues.push_back(&Cue);
				}
			}
		}
		std::vector<const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW*> WorldEvents;
		for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event :
			Pattern.WorldEventTriggerRefs)
		{
			if (Event.strPatternId == Pattern.strPatternId &&
				Event.strStageId == Stage.strStageId)
			{
				WorldEvents.push_back(&Event);
			}
		}
		std::vector<JOINED_COMBAT_OBJECT_SOUND> CombatObjectSoundCues;
		std::size_t iMissingCombatObjectSounds = 0u;
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& CombatObject :
			Stage.CombatObjectEffects)
		{
			if (CombatObject.HitIds.size() != CombatObject.HitOffsetsMs.size())
			{
				iMissingCombatObjectSounds += (std::max)(
					CombatObject.HitIds.size(), CombatObject.HitOffsetsMs.size());
				continue;
			}
			for (std::size_t iHit = 0u; iHit < CombatObject.HitIds.size(); ++iHit)
			{
				VALTAN_COMBAT_OBJECT_SOUND_CUE* pJoinedCue = nullptr;
				if (m_bValtanCombatObjectSoundCuesReady)
				{
					const auto Found = std::find_if(
						m_ValtanCombatObjectSoundCues.Cues.begin(),
						m_ValtanCombatObjectSoundCues.Cues.end(),
						[&CombatObject, &iHit](
							const VALTAN_COMBAT_OBJECT_SOUND_CUE& Cue)
						{
							return Cue.strCombatObjectArchetypeId ==
									CombatObject.strCombatObjectArchetypeId &&
								Cue.strHitId == CombatObject.HitIds[iHit];
						});
					if (Found != m_ValtanCombatObjectSoundCues.Cues.end())
						pJoinedCue = &*Found;
				}
				if (nullptr == pJoinedCue)
				{
					++iMissingCombatObjectSounds;
					continue;
				}
				CombatObjectSoundCues.push_back(
					{ pJoinedCue, CombatObject.HitOffsetsMs[iHit] });
			}
		}

		ImGui::PushID(static_cast<int32_t>(iStage));
		const std::string StageLabel = Stage.strStageId + " | " +
			std::to_string(Stage.iDurationMs) + " ms | animation " +
			std::to_string(Stage.ClipOccurrences.size()) + " | effect " +
			std::to_string(Stage.ProductCues.size() +
				Stage.CombatObjectEffects.size()) + " | sound " +
			std::to_string(SoundCues.size() + CombatObjectSoundCues.size()) +
			" | camera/shake " +
			std::to_string(Stage.CameraInvocations.size() + ShakeCues.size()) +
			" | world " + std::to_string(WorldEvents.size());

		const auto ResolveClipSourceMsToStageMs = [&Stage](
			const std::string_view strClipOccurrenceId,
			const std::uint32_t iSourceMs)
		{
			std::uint64_t iWallCursorMs = 0u;
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
				Stage.ClipOccurrences)
			{
				if (Clip.strClipOccurrenceId == strClipOccurrenceId)
				{
					const std::uint32_t iLocalSourceMs =
						iSourceMs > Clip.iSourceStartMs ?
							iSourceMs - Clip.iSourceStartMs : 0u;
					const f32_t fRate = Clip.fPlayRate > 0.f ?
						Clip.fPlayRate : 1.f;
					const std::uint64_t iLocalWallMs =
						static_cast<std::uint64_t>(std::llround(
							static_cast<double>(iLocalSourceMs) /
							static_cast<double>(fRate)));
					return static_cast<std::uint32_t>((std::min)(
						iWallCursorMs + iLocalWallMs,
						static_cast<std::uint64_t>(Stage.iDurationMs)));
				}
				iWallCursorMs += Clip.iAuthoringWallMs;
			}
			return 0u;
		};

		if (ImGui::BeginTable(
			"##JoinedTrackSegments", 3,
			ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_Resizable |
			ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn(
				"Track", ImGuiTableColumnFlags_WidthFixed, 90.f);
			ImGui::TableSetupColumn(
				"Time Axis (stage ms)", ImGuiTableColumnFlags_WidthStretch, 0.46f);
			ImGui::TableSetupColumn(
				"Stable owner row", ImGuiTableColumnFlags_WidthStretch, 0.54f);
			ImGui::TableHeadersRow();
			std::size_t iTrackRow = 0u;
			const auto TrackSegment = [&](
				const char_t* const pTrack,
				const std::uint32_t iStartMs,
				const std::uint32_t iEndMs,
				const std::string& strOwner,
				const ImU32 iColor)
			{
				ImGui::PushID(static_cast<int32_t>(iTrackRow++));
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(pTrack);
				ImGui::TableSetColumnIndex(1);
				const f32_t fWidth = (std::max)(
					80.f, ImGui::GetContentRegionAvail().x);
				const f32_t fHeight = ImGui::GetTextLineHeightWithSpacing();
				const ImVec2 Position = ImGui::GetCursorScreenPos();
				ImGui::InvisibleButton(
					"##track-segment", ImVec2(fWidth, fHeight));
				ImDrawList* const pDrawList = ImGui::GetWindowDrawList();
				pDrawList->AddRectFilled(
					Position,
					ImVec2(Position.x + fWidth, Position.y + fHeight),
					IM_COL32(38, 42, 48, 255), 3.f);
				const f32_t fDuration = static_cast<f32_t>((std::max)(
					Stage.iDurationMs, 1u));
				const std::uint32_t iClampedStart = (std::min)(
					iStartMs, Stage.iDurationMs);
				const std::uint32_t iClampedEnd = (std::min)(
					(std::max)(iEndMs, iClampedStart), Stage.iDurationMs);
				const f32_t fStartX = Position.x + fWidth *
					static_cast<f32_t>(iClampedStart) / fDuration;
				const f32_t fEndX = (std::max)(
					fStartX + 3.f,
					Position.x + fWidth *
						static_cast<f32_t>(iClampedEnd) / fDuration);
				pDrawList->AddRectFilled(
					ImVec2(fStartX, Position.y + 2.f),
					ImVec2((std::min)(fEndX, Position.x + fWidth),
						Position.y + fHeight - 2.f),
					iColor, 3.f);
				const std::string Range = std::to_string(iStartMs) +
					".." + std::to_string(iEndMs) + " ms";
				pDrawList->AddText(
					ImVec2(Position.x + 4.f, Position.y + 1.f),
					IM_COL32(245, 245, 245, 255), Range.c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::TextWrapped("%s", strOwner.c_str());
				ImGui::PopID();
			};

			std::uint64_t iAnimationCursorMs = 0u;
			if (Stage.bSuppressAnimation)
			{
				TrackSegment(
					"Animation", 0u, Stage.iDurationMs,
					"explicit NONE / pose hold",
					IM_COL32(84, 132, 220, 255));
			}
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
				Stage.ClipOccurrences)
			{
				const std::uint32_t iStartMs =
					static_cast<std::uint32_t>((std::min)(
						iAnimationCursorMs,
						static_cast<std::uint64_t>(UINT32_MAX)));
				iAnimationCursorMs += Clip.iAuthoringWallMs;
				const std::uint32_t iEndMs =
					static_cast<std::uint32_t>((std::min)(
						iAnimationCursorMs,
						static_cast<std::uint64_t>(UINT32_MAX)));
				TrackSegment(
					"Animation", iStartMs, iEndMs,
					Clip.strClipOccurrenceId + " | " + Clip.strClipName,
					IM_COL32(84, 132, 220, 255));
			}
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
				Stage.ProductCues)
			{
				const std::uint32_t iStartMs = Cue.bUsesStageClock ?
					Cue.iStageOffsetMs : ResolveClipSourceMsToStageMs(
						Cue.strClipOccurrenceId, Cue.iSourceStartMs);
				std::uint32_t iEndMs = iStartMs;
				if (!Cue.bUsesStageClock && Cue.bHasSourceEnd)
				{
					iEndMs = ResolveClipSourceMsToStageMs(
						Cue.strClipOccurrenceId, Cue.iSourceEndMs);
				}
				if ("each_loop" == Cue.strRepeatPolicy)
					iEndMs = Stage.iDurationMs;
				TrackSegment(
					"Effect", iStartMs, iEndMs,
					Cue.strOccurrenceId + " | " + Cue.strEffectAssetId,
					IM_COL32(164, 101, 220, 255));
			}
			for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& CombatObject :
				Stage.CombatObjectEffects)
			{
				TrackSegment(
					"Effect", 0u, CombatObject.iLifetimeMs,
					"Server combat object " +
						CombatObject.strCombatObjectArchetypeId + " | " +
						CombatObject.strEffectAssetId + " (local lifetime)",
					IM_COL32(150, 88, 205, 255));
			}
			for (const VALTAN_PATTERN_SOUND_CUE* const pCue : SoundCues)
			{
				const std::uint32_t iStartMs = ResolveClipSourceMsToStageMs(
					pCue->strClipOccurrenceId, pCue->iStartMs);
				const std::uint32_t iEndMs =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
						pCue->eRepeatPolicy ? Stage.iDurationMs : iStartMs;
				TrackSegment(
					"Sound", iStartMs, iEndMs,
					pCue->strOccurrenceId + " | " + pCue->strSoundEvent,
					IM_COL32(70, 175, 118, 255));
			}
			for (const JOINED_COMBAT_OBJECT_SOUND& Joined :
				CombatObjectSoundCues)
			{
				TrackSegment(
					"Sound", Joined.iHitOffsetMs, Joined.iHitOffsetMs,
					Joined.pCue->strBindingId + " | " +
						Joined.pCue->strSoundEvent + " (combat-object local)",
					IM_COL32(55, 150, 103, 255));
			}
			for (const VALTAN_CAMERA_INVOCATION_VIEW& Invocation :
				Stage.CameraInvocations)
			{
				TrackSegment(
					"Camera", Invocation.iStartOffsetMs,
					Invocation.iStartOffsetMs + Invocation.iDurationMs,
					Invocation.strCameraInvocationId + " | " +
						Invocation.strCameraCueId,
					IM_COL32(218, 154, 63, 255));
			}
			for (const VALTAN_PATTERN_SHAKE_CUE* const pCue : ShakeCues)
			{
				const std::uint32_t iStartMs = ResolveClipSourceMsToStageMs(
					pCue->strClipOccurrenceId, pCue->iStartMs);
				const std::uint64_t iShakeEndMs =
					static_cast<std::uint64_t>(iStartMs) +
					static_cast<std::uint64_t>(std::llround(
						static_cast<double>(pCue->Spec.fDurationSeconds) *
						1000.0));
				TrackSegment(
					"Camera", iStartMs,
					VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP ==
						pCue->eRepeatPolicy ? Stage.iDurationMs :
							static_cast<std::uint32_t>((std::min)(
								iShakeEndMs,
								static_cast<std::uint64_t>(UINT32_MAX))),
					pCue->strOccurrenceId + " | camera shake",
					IM_COL32(202, 130, 52, 255));
			}
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW* const pEvent :
				WorldEvents)
			{
				const bool_t bExitEdge =
					std::string_view::npos != pEvent->strTriggerKind.find("EXIT") ||
					std::string_view::npos != pEvent->strTriggerKind.find("END");
				const std::uint32_t iEventMs = bExitEdge ?
					Stage.iDurationMs : 0u;
				TrackSegment(
					"World", iEventMs, iEventMs,
					pEvent->strPatternId + "/" + pEvent->strStageId +
						" | " + pEvent->strTriggerKind,
					IM_COL32(198, 84, 91, 255));
			}
			ImGui::EndTable();
		}
		if (ImGui::TreeNodeEx(
			StageLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextDisabled("Server action: %s", Stage.strActionId.c_str());

			ImGui::SeparatorText("Animation");
			if (Stage.bSuppressAnimation)
				ImGui::TextDisabled("NONE (hold current boss pose)");
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
				Stage.ClipOccurrences)
			{
				ImGui::BulletText(
					"%s | %s | source +%u ms | play %u ms | wall %u ms",
					Clip.strClipOccurrenceId.c_str(), Clip.strClipName.c_str(),
					Clip.iSourceStartMs, Clip.iPlayMs, Clip.iAuthoringWallMs);
			}

			ImGui::SeparatorText("Effect");
			if (Stage.ProductCues.empty() && Stage.CombatObjectEffects.empty())
				ImGui::TextDisabled("No Product Effect cue on this stage.");
			for (std::size_t iCue = 0u; iCue < Stage.ProductCues.size(); ++iCue)
			{
				const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue =
					Stage.ProductCues[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | %s | anchor %s | %s",
					Cue.strOccurrenceId.c_str(), Cue.strEffectAssetId.c_str(),
					Cue.strAnchorSlotId.c_str(),
					Cue.bUsesStageClock ? "stage clock" :
						Cue.strClipOccurrenceId.c_str());
				ImGui::SameLine();
				if (ImGui::SmallButton("Open Effect Tool"))
				{
					m_strEffectToolOpenPatternId = Pattern.strPatternId;
					m_strEffectToolOpenStageId = Stage.strStageId;
					m_strEffectToolOpenCueOccurrenceId = Cue.strOccurrenceId;
					m_strEffectToolOpenEffectAssetId = Cue.strEffectAssetId;
					m_hasEffectToolOpenRequest = true;
				}
				ImGui::PopID();
			}
			for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& CombatObject :
				Stage.CombatObjectEffects)
			{
				ImGui::BulletText(
					"Server combat object %s x%u -> %s",
					CombatObject.strCombatObjectArchetypeId.c_str(),
					CombatObject.iSpawnValue,
					CombatObject.strEffectAssetId.c_str());
				ImGui::TextDisabled(
					"  Product clock (read-only): life %u ms | %s | origin %s | direction %s | speed %.3f m/s | max %.3f m",
					CombatObject.iLifetimeMs,
					CombatObject.strKind.c_str(),
					CombatObject.strOriginPolicy.c_str(),
					CombatObject.strDirectionPolicy.c_str(),
					CombatObject.fSpeedMps,
					CombatObject.fMaximumDistanceM);
				for (std::size_t iHit = 0u;
					iHit < CombatObject.HitOffsetsMs.size(); ++iHit)
				{
					const char_t* pHitId = iHit < CombatObject.HitIds.size() ?
						CombatObject.HitIds[iHit].c_str() : "MISSING_HIT_ID";
					ImGui::TextDisabled(
						"  Server hit %s at combat-object +%u ms (local clock; stage duration %u ms)",
						pHitId, CombatObject.HitOffsetsMs[iHit],
						Stage.iDurationMs);
				}
				if ("VALTAN_HIGH_JUMP" == Pattern.strPatternId &&
					"AIRBORNE" == Stage.strStageId)
				{
					ImGui::TextColored(
						ImVec4(0.35f, 0.75f, 1.f, 1.f),
						"  Separate clocks: AIRBORNE stage %u ms | axe lifetime %u ms | first axe-local hit atMs %u",
						Stage.iDurationMs, CombatObject.iLifetimeMs,
						CombatObject.HitOffsetsMs.empty() ? 0u :
							CombatObject.HitOffsetsMs.front());
				}
			}

			ImGui::SeparatorText("Sound");
			if (SoundCues.empty() && CombatObjectSoundCues.empty())
				ImGui::TextDisabled("No Sound cue on this stage.");
			for (std::size_t iCue = 0u; iCue < SoundCues.size(); ++iCue)
			{
				const VALTAN_PATTERN_SOUND_CUE& Cue = *SoundCues[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | event %s | clip +%u ms | %s",
					Cue.strOccurrenceId.c_str(), Cue.strSoundEvent.c_str(),
					Cue.iStartMs,
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
						Cue.eRepeatPolicy ? "each loop" : "once");
				const std::vector<std::string>& Variants =
					CSoundCueCatalog::Find_Variants("Valtan", Cue.strSoundEvent);
				if (Variants.empty())
				{
					ImGui::TextColored(
						ImVec4(1.f, 0.35f, 0.25f, 1.f),
						"  MISSING: event has no WAV asset variant.");
				}
				for (std::size_t iVariant = 0u;
					iVariant < Variants.size(); ++iVariant)
				{
					ImGui::PushID(static_cast<int32_t>(iVariant));
					if (ImGui::SmallButton("Preview WAV"))
						(void)Preview_ValtanSoundAsset(Variants[iVariant]);
					ImGui::SameLine();
					ImGui::TextDisabled("%s", Variants[iVariant].c_str());
					ImGui::PopID();
				}
				ImGui::PopID();
			}
			if (!CombatObjectSoundCues.empty())
				ImGui::SeparatorText("Server Hit Sound");
			ImGui::PushID("server-hit-sounds");
			for (std::size_t iCue = 0u;
				iCue < CombatObjectSoundCues.size(); ++iCue)
			{
				const JOINED_COMBAT_OBJECT_SOUND& Joined =
					CombatObjectSoundCues[iCue];
				VALTAN_COMBAT_OBJECT_SOUND_CUE& Cue = *Joined.pCue;
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | %s + %s | event %s | object +%u ms",
					Cue.strBindingId.c_str(),
					Cue.strCombatObjectArchetypeId.c_str(), Cue.strHitId.c_str(),
					Cue.strSoundEvent.c_str(), Joined.iHitOffsetMs);
				ImGui::SetNextItemWidth(360.f);
				if (ImGui::BeginCombo("Impact Sound Event", Cue.strSoundEvent.c_str()))
				{
					for (const std::string& EventName : ValtanSoundEventNames)
					{
						const bool_t bSelected = EventName == Cue.strSoundEvent;
						if (ImGui::Selectable(EventName.c_str(), bSelected) && !bSelected)
						{
							Cue.strSoundEvent = EventName;
							m_bValtanCombatObjectSoundCuesDirty = true;
							m_strValtanCombatObjectSoundCueStatus =
								"UNSAVED: " + Cue.strBindingId + " -> " + EventName;
						}
						if (bSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				const std::vector<std::string>& Variants =
					CSoundCueCatalog::Find_Variants("Valtan", Cue.strSoundEvent);
				for (std::size_t iVariant = 0u;
					iVariant < Variants.size(); ++iVariant)
				{
					ImGui::PushID(static_cast<int32_t>(iVariant));
					if (ImGui::SmallButton("Preview Impact WAV"))
						(void)Preview_ValtanSoundAsset(Variants[iVariant]);
					ImGui::SameLine();
					ImGui::TextDisabled("%s", Variants[iVariant].c_str());
					ImGui::PopID();
				}
				ImGui::PopID();
			}
			ImGui::PopID();
			if (0u != iMissingCombatObjectSounds)
			{
				ImGui::TextColored(
					ImVec4(1.f, 0.25f, 0.20f, 1.f),
					"COVERAGE GAP: %zu Server combat-object hit(s) have no exact Sound binding.",
					iMissingCombatObjectSounds);
			}

			ImGui::SeparatorText("Camera / Shake");
			if (Stage.CameraInvocations.empty() && ShakeCues.empty())
				ImGui::TextDisabled("No Camera or Shake cue on this stage.");
			for (std::size_t iCue = 0u;
				iCue < Stage.CameraInvocations.size(); ++iCue)
			{
				const VALTAN_CAMERA_INVOCATION_VIEW& Invocation =
					Stage.CameraInvocations[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s -> %s | %s +%u ms | %s %u ms",
					Invocation.strCameraInvocationId.c_str(),
					Invocation.strCameraCueId.c_str(),
					Invocation.strTrigger.c_str(), Invocation.iStartOffsetMs,
					Invocation.strDurationPolicy.c_str(), Invocation.iDurationMs);
				ImGui::SameLine();
				if (ImGui::SmallButton("Open Camera Tool"))
				{
					m_strCameraToolOpenCueId = Invocation.strCameraCueId;
					m_hasCameraToolOpenRequest = true;
				}
				ImGui::PopID();
			}
			ImGui::PushID("camera-shakes");
			for (std::size_t iCue = 0u; iCue < ShakeCues.size(); ++iCue)
			{
				const VALTAN_PATTERN_SHAKE_CUE& Cue = *ShakeCues[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | %s +%u ms | %.3f s | F/R/U/FOV amp %.3g/%.3g/%.3g/%.3g | %s",
					Cue.strOccurrenceId.c_str(), Cue.strClipOccurrenceId.c_str(),
					Cue.iStartMs, Cue.Spec.fDurationSeconds,
					Cue.Spec.Forward.fAmplitude, Cue.Spec.Right.fAmplitude,
					Cue.Spec.Up.fAmplitude, Cue.Spec.Fov.fAmplitude,
					VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP ==
						Cue.eRepeatPolicy ? "each loop" : "once");
				ImGui::PopID();
			}
			ImGui::PopID();

			ImGui::SeparatorText("World Event / Runtime UI");
			if (WorldEvents.empty())
				ImGui::TextDisabled("No world-event trigger on this stage.");
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW* const pEvent :
				WorldEvents)
			{
				ImGui::BulletText(
					"%s/%s -> %s",
					pEvent->strPatternId.c_str(), pEvent->strStageId.c_str(),
					pEvent->strTriggerKind.c_str());
			}
			ImGui::TextDisabled(
				"HUD/UI observes replicated Server state; this stage does not author a second UI command path.");

			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void Client::CAnimation_Tool::Render_ValtanSelectedResourceUsage(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW* pStage)
{
	ImGui::SeparatorText("Used by Selected Pattern");
	ImGui::TextDisabled(
		"Semantic runtime relationships only. Choose a slot to focus its typed Persistent Detail owner.");
	const auto SemanticSlot = [this](
		const char_t* pSlot,
		const VALTAN_WORKBENCH_DETAIL_OWNER eOwner,
		const std::string& Meaning,
		const std::string_view ExactOwner)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushID(pSlot);
		if (ImGui::SmallButton(pSlot))
		{
			m_eValtanWorkbenchDetailOwner = eOwner;
			m_bValtanWorkbenchFocusDetailRequested = true;
		}
		ImGui::PopID();
		ImGui::TableSetColumnIndex(1);
		ImGui::TextWrapped("%s", Meaning.c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::TextWrapped(
			"%.*s", static_cast<int32_t>(ExactOwner.size()), ExactOwner.data());
	};
	if (ImGui::BeginTable(
		"##ValtanSelectedSemanticResources", 3,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn(
			"Meaning slot", ImGuiTableColumnFlags_WidthFixed, 190.f);
		ImGui::TableSetupColumn(
			"Selected relationship", ImGuiTableColumnFlags_WidthStretch, 0.55f);
		ImGui::TableSetupColumn(
			"Exact typed owner", ImGuiTableColumnFlags_WidthStretch, 0.45f);
		ImGui::TableHeadersRow();

		const std::string StageIdentity = nullptr == pStage ?
			"Pattern " + Pattern.strPatternId +
				" selected; choose a semantic stage for exact action ownership." :
			"Pattern " + Pattern.strPatternId + " / stage " +
				pStage->strStageId + " / action " + pStage->strActionId +
				" / wall " + std::to_string(pStage->iDurationMs) + " ms";
		SemanticSlot(
			"Gameplay Source / Product",
			VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY,
			StageIdentity,
			"Data/Valtan/Valtan.gameplay.json -> Data/Encounters/Valtan/ValtanEncounter.json");

		std::string HitMeaning =
			"No semantic stage selected; collider/hit ownership is not guessed.";
		if (nullptr != pStage)
		{
			HitMeaning = pStage->Has_HitShape() ?
				("shape " + pStage->strHitShape + " | damage " +
				 pStage->strServerDamageProfileId + " | hits " +
				 std::to_string(pStage->iHitCount)) :
				"No Server hit shape on the selected stage; this is an explicit empty gameplay slot.";
		}
		SemanticSlot(
			"Collider / Hit", VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY,
			HitMeaning,
			"Data/Valtan/Valtan.gameplay.json + Data/Balance/DamageProfiles.json");

		std::string MotionMeaning =
			"No semantic stage selected; motion/action/branch flow is not guessed.";
		if (nullptr != pStage)
		{
			MotionMeaning = pStage->Motion.has_value() ?
				("stage motion " + pStage->Motion->strKind + " | actions " +
				 std::to_string(pStage->Actions.size()) + " | branches " +
				 std::to_string(pStage->Branches.size())) :
				("No stage motion; actions " +
				 std::to_string(pStage->Actions.size()) + " | branches " +
				 std::to_string(pStage->Branches.size()));
		}
		else if (Pattern.ServerMotion.has_value())
		{
			MotionMeaning = "pattern motion " + Pattern.ServerMotion->strKind;
		}
		SemanticSlot(
			"Motion / Flow", VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY,
			MotionMeaning,
			"Data/Valtan/Valtan.gameplay.json");

		const std::string ChainMeaning =
			Pattern.strSourceAnimationChainId.empty() ?
				"No promoted Animation Intake chain is recorded for this Product pattern." :
				("source chain " + Pattern.strSourceAnimationChainId);
		SemanticSlot(
			"Animation Chain", VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION,
			ChainMeaning,
			"Data/Valtan/Valtan.presentation.json (joined read-only chain identity)");

		std::string ClipMeaning =
			"No semantic stage selected; clip occurrences are not guessed.";
		if (nullptr != pStage)
		{
			if (pStage->bSuppressAnimation)
			{
				ClipMeaning = "Explicit NONE / hold current boss pose.";
			}
			else if (pStage->ClipOccurrences.empty())
			{
				ClipMeaning = "No admitted clip occurrence on the selected stage.";
			}
			else
			{
				ClipMeaning.clear();
				for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
					pStage->ClipOccurrences)
				{
					if (!ClipMeaning.empty())
						ClipMeaning += " | ";
					ClipMeaning += Clip.strClipOccurrenceId + " -> " +
						Clip.strClipName;
				}
			}
		}
		SemanticSlot(
			"Animation Clips", VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION,
			ClipMeaning,
			"OWNER Data/Valtan/Valtan.presentation.json -> READ-ONLY PRODUCT Data/Animation/Authored/Valtan/Valtan.patternbindings.json");

		std::string EffectMeaning =
			"No semantic stage selected; Effect ownership is not guessed.";
		if (nullptr != pStage)
		{
			if (pStage->ProductCues.empty())
			{
				EffectMeaning =
					"No Product Effect cue on the selected stage; no synthetic Effect JSON is offered.";
			}
			else
			{
				EffectMeaning.clear();
				for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
					pStage->ProductCues)
				{
					if (!EffectMeaning.empty())
						EffectMeaning += " | ";
					EffectMeaning += Cue.strOccurrenceId + " -> " +
						Cue.strEffectAssetId + " (Data/Effects/Authored/" +
						Cue.strEffectAssetId + ".effect.json)";
				}
			}
		}
		SemanticSlot(
			"Effect Cues / Assets", VALTAN_WORKBENCH_DETAIL_OWNER::EFFECT,
			EffectMeaning,
			"OWNER Data/Valtan/Valtan.presentation.json -> READ-ONLY PRODUCT Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json + exact authored Effect assets");

		std::string SoundMeaning =
			"No Pattern Sound cue on the selected stage; no WAV asset is inferred.";
		if (nullptr != pStage)
		{
			for (const VALTAN_PATTERN_SOUND_CUE& Cue :
				m_ValtanPatternSoundCues.Cues)
			{
				if (Cue.strPatternId != Pattern.strPatternId ||
					Cue.strStageId != pStage->strStageId)
				{
					continue;
				}
				if (0u == SoundMeaning.find("No Pattern Sound cue"))
					SoundMeaning.clear();
				if (!SoundMeaning.empty())
					SoundMeaning += " | ";
				SoundMeaning += Cue.strOccurrenceId + " -> " + Cue.strSoundEvent;
				const std::vector<std::string>& Variants =
					CSoundCueCatalog::Find_Variants("Valtan", Cue.strSoundEvent);
				if (Variants.empty())
					SoundMeaning += " (MISSING asset variant)";
				for (const std::string& Variant : Variants)
					SoundMeaning += " -> " + Variant;
			}
		}
		SemanticSlot(
			"Sound Cues / Assets", VALTAN_WORKBENCH_DETAIL_OWNER::SOUND,
			SoundMeaning,
			"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json + Data/Sound catalog assets");

		std::string CameraWorldMeaning =
			"No semantic stage selected; Camera/World relationships are not guessed.";
		if (nullptr != pStage)
		{
			CameraWorldMeaning = "camera invocations " +
				std::to_string(pStage->CameraInvocations.size());
			std::size_t iWorldCount = 0u;
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event :
				Pattern.WorldEventTriggerRefs)
			{
				if (Event.strStageId == pStage->strStageId)
					++iWorldCount;
			}
			CameraWorldMeaning += " | world triggers " +
				std::to_string(iWorldCount);
		}
		SemanticSlot(
			"Camera / World", VALTAN_WORKBENCH_DETAIL_OWNER::CAMERA,
			CameraWorldMeaning,
			"Data/Animation/Authored/Valtan/Valtan.patternshakecues.json + Data/Encounters/Valtan typed owners");
		ImGui::EndTable();
	}

	if (ImGui::CollapsingHeader("Raw owner index / diagnostics"))
	{
		ImGui::TextDisabled(
			"Complete owner inventory for diagnosis only. It is not a second JSON editor and does not replace the semantic slots above.");
		ImGui::TextDisabled(
			"Animation/Effect invocation Products are projector-owned and never dirty in this Workbench.");
		if (m_bValtanPatternSoundCuesDirty)
			ImGui::TextColored(
				ImVec4(1.f, 0.75f, 0.2f, 1.f),
				"Pattern Sound owner: UNSAVED typed draft");
		if (ImGui::BeginTable(
			"##ValtanWorkbenchOwnerFiles", 3,
			ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
				ImGuiTableFlags_Resizable |
				ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn(
				"Domain", ImGuiTableColumnFlags_WidthFixed, 110.f);
			ImGui::TableSetupColumn(
				"Owner / state", ImGuiTableColumnFlags_WidthFixed, 235.f);
			ImGui::TableSetupColumn(
				"Exact source", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();
			const auto DataFileRow = [](
				const char_t* pDomain,
				const char_t* pOwnerState,
				const std::string_view strPath)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(pDomain);
				ImGui::TableSetColumnIndex(1);
				ImGui::TextWrapped("%s", pOwnerState);
				ImGui::TableSetColumnIndex(2);
				ImGui::TextWrapped(
					"%.*s", static_cast<int32_t>(strPath.size()),
					strPath.data());
			};
			DataFileRow(
				"Gameplay", "EDITABLE HERE / typed atomic Save",
				"Data/Valtan/Valtan.gameplay.json");
			DataFileRow(
				"Presentation", "AUTHORING OWNER / immutable revision Save",
				"Data/Valtan/Valtan.presentation.json");
			DataFileRow(
				"Animation Product", "READ-ONLY / generated by projector",
				"Data/Animation/Authored/Valtan/Valtan.patternbindings.json");
			DataFileRow(
				"Effect Invocation Product", "READ-ONLY / generated by projector",
				"Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json");
			DataFileRow(
				"Sound", "EDITABLE IN PERSISTENT DETAIL / typed owner Save",
				"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json");
			DataFileRow(
				"Camera / World", "READ-ONLY HERE / typed owner tools",
				"Data/Animation/Authored/Valtan/Valtan.patternshakecues.json + Data/Encounters/Valtan");
			ImGui::EndTable();
		}
	}
}

void Client::CAnimation_Tool::Render_ValtanPatternMasterUnavailableShell(
	const std::size_t iAdmittedPatternCount,
	const bool_t bHasPreviewModel)
{
	if (ImGui::BeginTable(
		"##ValtanWorkbenchMain", 3,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollX,
		ImVec2(0.f, 420.f), WORKBENCH_THREE_PANE_INNER_WIDTH))
	{
		ImGui::TableSetupColumn(
			"Master / Outliner", ImGuiTableColumnFlags_WidthStretch, 0.22f);
		ImGui::TableSetupColumn(
			"Preview / Transport", ImGuiTableColumnFlags_WidthStretch, 0.46f);
		ImGui::TableSetupColumn(
			"Persistent Detail", ImGuiTableColumnFlags_WidthStretch, 0.32f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		if (ImGui::BeginChild(
			"##ValtanWorkbenchOutliner", ImVec2(0.f, 0.f),
			ImGuiChildFlags_Borders))
		{
			ImGui::SeparatorText("Master / Outliner");
			ImGui::TextUnformatted("Valtan | boss.valtan");
			ImGui::TextColored(
				ImVec4(1.f, 0.45f, 0.35f, 1.f),
				"Canonical inventory rejected");
			ImGui::TextWrapped(
				"%zu pattern rows admitted; no synthetic or stale pattern is offered.",
				iAdmittedPatternCount);
		}
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(1);
		if (ImGui::BeginChild(
			"##ValtanWorkbenchPreview", ImVec2(0.f, 0.f),
			ImGuiChildFlags_Borders))
		{
			ImGui::SeparatorText("Preview / Transport");
			ImGui::TextDisabled(
				"Preview Model: %s", bHasPreviewModel ? "READY" : "MISSING");
			ImGui::TextColored(
				ImVec4(1.f, 0.45f, 0.35f, 1.f),
				"Complete Play blocked: canonical pattern selection is unavailable.");
			ImGui::TextWrapped(
				"Fix the exact joined-source error below, then Reload Valtan Pattern Master. The previous Product data is never guessed or partially admitted.");
		}
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(2);
		if (ImGui::BeginChild(
			"##ValtanWorkbenchDetail", ImVec2(0.f, 0.f),
			ImGuiChildFlags_Borders))
		{
			ImGui::SeparatorText("Persistent Detail");
			ImGui::TextColored(
				ImVec4(1.f, 0.45f, 0.35f, 1.f),
				"Canonical Join: %s",
				ValtanPatternMasterAdmissionLabel());
			if (!m_strValtanPatternMasterStatus.empty())
				ImGui::TextWrapped("%s", m_strValtanPatternMasterStatus.c_str());
		}
		ImGui::EndChild();
		ImGui::EndTable();
	}

	if (ImGui::BeginChild(
		"##ValtanWorkbenchSequencer", ImVec2(0.f, 180.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::SeparatorText("Sequencer / Joined Tracks");
		ImGui::TextDisabled(
			"Animation, Effect, Sound, Camera, Light/World and Combat Object tracks require one admitted stable pattern/stage selection.");
	}
	ImGui::EndChild();

	if (ImGui::BeginChild(
		"##ValtanWorkbenchDataFiles", ImVec2(0.f, 390.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::SeparatorText("Data Files / Canonical Join Diagnostic");
		ImGui::TextWrapped(
			"%s", m_strValtanPatternMasterStatus.empty() ?
				"Canonical loader returned no diagnostic." :
				m_strValtanPatternMasterStatus.c_str());
		ImGui::Separator();
		ImGui::BulletText("Gameplay | Data/Valtan/Valtan.gameplay.json");
		ImGui::BulletText("Presentation | Data/Valtan/Valtan.presentation.json");
		ImGui::BulletText("Encounter | Data/Encounters/Valtan/ValtanEncounter.json");
		ImGui::BulletText(
			"Animation Product (read-only) | Data/Animation/Authored/Valtan/Valtan.patternbindings.json");
		ImGui::BulletText(
			"Effect Invocation Product (read-only) | Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json");
		ImGui::BulletText(
			"Sound | Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json");
		ImGui::BulletText(
			"Camera/World | Data/Encounters/Valtan typed owner documents");
		ImGui::BeginDisabled(nullptr == m_pBalanceTool);
		if (ImGui::SmallButton("Open Valtan Balance / Gameplay##RejectedDataFiles"))
			m_pBalanceTool->Open_Valtan();
		ImGui::EndDisabled();
	}
	ImGui::EndChild();
}

void Client::CAnimation_Tool::Render_ValtanPatternMaster(
	const shared_ptr<Engine::CModel>& pModel)
{
	const bool_t bHasPreviewModel = nullptr != pModel;
	bool_t bReloadPatternMasterAfterSave = false;
	if (!m_bValtanPatternMasterLoadAttempted)
	{
		m_bValtanPatternMasterLoadAttempted = true;
		(void)Reload_ValtanPatternMaster();
	}
	const bool_t bMutationAdmitted =
		VALTAN_PATTERN_MASTER_ADMISSION_STATE::ADMITTED ==
			m_eValtanPatternMasterAdmission;
	ImGui::SeparatorText("Valtan Action Presentation Workbench");
	ImGui::TextWrapped(
		"One joined view over Server Stage, Animation, Effect, Sound Asset, and Combat Object. Pattern Offline samples the Product animation locally; Server Replay/Live submits the same stable pattern ID to the real Arena authority, where movement, hit, grab, damage, Effect, and Sound run.");
	ImGui::TextDisabled(
		"Encounter target: Valtan (Product Server authority). KoukuSaton Arena is admitted; its boss/pattern Server vertical slice remains deferred.");
	ImGui::SeparatorText("Applied Product Sources / Editability");
	ImGui::BulletText(
		"EDIT + SAVE: Data/Valtan/Valtan.gameplay.json | Server stage clock, collider, hit schedule and player reaction");
	ImGui::BulletText(
		"EDIT + SAVE: Data/Balance/BossProfiles.json and DamageProfiles.json | Valtan gameplay values through the typed Balance transaction");
	ImGui::BulletText(
		"AUTHORING OWNER: Data/Valtan/Valtan.presentation.json | animation occurrences and managed Effect invocations save only through the immutable joined revision pipeline");
	ImGui::BulletText(
		"EDIT + OWNER SAVE: Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json | stable-row Sound event, derived bank, source start and repeat policy");
	ImGui::BulletText(
		"READ-ONLY PRODUCT: Valtan.patternbindings.json + Valtan.patterneffectcues.json | projector outputs consumed only after canonical admission");
	ImGui::BulletText(
		"JOINED LANES: V1 Effect, Sound, Camera/Shake, World Event and Server Combat Object bindings are listed below with their owner-tool boundary");
	ImGui::TextDisabled(
		"Animation Sequence Intake is an offline reviewed source. It is not presented as Product Save until Client presentation generation staging and rollback are admitted.");
	if (!bMutationAdmitted)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.55f, 0.25f, 1.f),
			"READ-ONLY DIAGNOSTIC VIEW: admission is %s. Save, Create Apply, Complete Play and every Server mutation are blocked.",
			ValtanPatternMasterAdmissionLabel());
	}
	ImGui::BeginDisabled(nullptr == m_pBalanceTool);
	if (ImGui::SmallButton("Open Valtan Balance / Gameplay"))
		m_pBalanceTool->Open_Valtan();
	ImGui::EndDisabled();

	if (ImGui::CollapsingHeader("Server Arena Environment"))
	{
	ImGui::TextDisabled(
		"These presets use the active Valtan room's destruction transaction; no wall, collision or navigation state is changed locally.");
	ImGui::PushID("WorkbenchArenaPreset");
	ImGui::BeginDisabled(nullptr == m_pBossTool || !bMutationAdmitted);
	const auto arenaPresetButton = [this](
		const char_t* label,
		const LostArk::Shared::VALTAN_ARENA_PRESET preset)
	{
		if (!ImGui::Button(label))
			return;
		std::string status;
		(void)m_pBossTool->Set_ServerArenaPreset(preset, status);
		m_strValtanPatternMasterStatus = std::move(status);
	};
	arenaPresetButton(
		"Fresh / All Walls", LostArk::Shared::VALTAN_ARENA_PRESET::FRESH);
	ImGui::SameLine();
	arenaPresetButton(
		"Circle / Walls Gone",
		LostArk::Shared::VALTAN_ARENA_PRESET::CIRCLE_WALLS_GONE);
	arenaPresetButton(
		"Break 3 O'Clock",
		LostArk::Shared::VALTAN_ARENA_PRESET::THREE_OCLOCK_BROKEN);
	ImGui::SameLine();
	arenaPresetButton(
		"Break 9 O'Clock",
		LostArk::Shared::VALTAN_ARENA_PRESET::NINE_OCLOCK_BROKEN);
	arenaPresetButton(
		"Break 3 + 9 O'Clock",
		LostArk::Shared::VALTAN_ARENA_PRESET::BOTH_SIDES_BROKEN);
	ImGui::EndDisabled();

	CBossTool::VALTAN_ARENA_ACTIVE_STATE activeState{};
	std::string activeStateStatus;
	const bool_t bActiveStateReady = nullptr != m_pBossTool &&
		m_pBossTool->Get_ServerArenaActiveState(
			activeState, activeStateStatus);
	ImGui::SeparatorText("Arena Active (Server actual)");
	ImGui::TextDisabled(
		"Active boxes are replicated facts. Mutations use the five exact Server presets above.");
	const auto actualCheckbox = [](const char_t* label, const bool_t active)
	{
		bool_t value = active;
		ImGui::BeginDisabled(true);
		ImGui::Checkbox(label, &value);
		ImGui::EndDisabled();
	};
	ImGui::BeginDisabled(!bActiveStateReady);
	actualCheckbox(
		"Ordinary walls / debris sources Active",
		activeState.bOrdinaryWallsActive);
	actualCheckbox("109 outer ring Active", activeState.bOuterRingActive);
	actualCheckbox(
		"3 o'clock floor / collision / Nav Active",
		activeState.bThreeOClockFloorActive);
	actualCheckbox(
		"9 o'clock floor / collision / Nav Active",
		activeState.bNineOClockFloorActive);
	ImGui::EndDisabled();
	ImGui::Text(
		"Debris actors %u | collision %u | nav regions %u | nav revision %llu",
		activeState.iDebrisActorCount,
		activeState.iActiveCollisionCount,
		activeState.iActiveNavigationRegionCount,
		static_cast<unsigned long long>(activeState.iNavigationRevision));
	if (!activeStateStatus.empty())
		ImGui::TextDisabled("%s", activeStateStatus.c_str());
	if (nullptr != m_pBossTool)
	{
		const std::string arenaStatus =
			m_pBossTool->Get_ServerArenaPresetStatus();
		if (!arenaStatus.empty())
			ImGui::TextWrapped("Arena: %s", arenaStatus.c_str());
	}
	ImGui::PopID();
	}

	if (!m_bValtanPatternMasterLoadAttempted)
	{
		m_bValtanPatternMasterLoadAttempted = true;
		Reload_ValtanPatternMaster();
	}
	ImGui::BeginDisabled(
		m_bValtanPatternMasterPlaying || m_bValtanPatternPreviewPlaying ||
		m_bValtanPatternSoundCuesDirty ||
		m_bValtanCombatObjectSoundCuesDirty ||
		m_bValtanPatternAnimationBindingDirty);
	if (ImGui::SmallButton("Reload Valtan Pattern Master"))
		Reload_ValtanPatternMaster();
	ImGui::EndDisabled();
	if (m_bValtanPatternSoundCuesDirty && ImGui::IsItemHovered(
			ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(
			"Save or discard the typed Pattern Sound draft before reload.");
	}
	else if (m_bValtanCombatObjectSoundCuesDirty && ImGui::IsItemHovered(
			ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("Save the edited Server-hit Sound binding before reload.");
	}
	else if (m_bValtanPatternAnimationBindingDirty && ImGui::IsItemHovered(
		ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(
			"Save or discard the typed Animation binding draft before reload.");
	}

	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
		Collect_ValtanPatternMasterPatterns();
	const bool_t bHasAdmittedGraph =
		Patterns.size() >= VALTAN_PATTERN_MASTER_ORDER.size();
	const bool_t bReady = bHasAdmittedGraph &&
		(VALTAN_PATTERN_MASTER_ADMISSION_STATE::ADMITTED ==
			m_eValtanPatternMasterAdmission ||
		 VALTAN_PATTERN_MASTER_ADMISSION_STATE::STALE_PRESERVED ==
			m_eValtanPatternMasterAdmission);
	ImGui::SeparatorText("Workbench Admission");
	ImGui::Text(
		"Preview Model: %s | Canonical Join: %s | Inventory: %zu patterns / %zu required managed IDs",
		bHasPreviewModel ? "READY" : "MISSING",
		ValtanPatternMasterAdmissionLabel(),
		Patterns.size(), VALTAN_PATTERN_MASTER_ORDER.size());
	if (!bReady)
	{
		if (!m_strValtanPatternMasterStatus.empty())
			ImGui::TextWrapped("%s", m_strValtanPatternMasterStatus.c_str());
		Render_ValtanPatternMasterUnavailableShell(
			Patterns.size(), bHasPreviewModel);
	}
	else
	{
	m_iValtanPatternMasterSelected = std::clamp(
		m_iValtanPatternMasterSelected, 0,
		static_cast<int32_t>(Patterns.size() - 1u));
	const VALTAN_PATTERN_VIEW* pSelected =
		Patterns[static_cast<size_t>(m_iValtanPatternMasterSelected)];
	const auto StablePattern = std::find_if(
		Patterns.begin(), Patterns.end(),
		[this](const VALTAN_PATTERN_VIEW* pPattern)
		{
			return nullptr != pPattern &&
				pPattern->strPatternId == m_strValtanWorkbenchPatternId;
		});
	if (Patterns.end() != StablePattern)
		pSelected = *StablePattern;
	else
	{
		m_strValtanWorkbenchPatternId = pSelected->strPatternId;
		m_strValtanWorkbenchStageId.clear();
	}
#ifdef _DEBUG
	if (!m_bValtanPatternSoundCuesDirty)
	{
		if (CMainApp* const pApp = CMainApp::Get_Active())
		{
			const std::string& strSharedPatternId =
				pApp->Debug_GetSelectedCompletePlayPatternId();
			if (strSharedPatternId.empty())
			{
				(void)pApp->Debug_SelectCompletePlayPattern(
					pSelected->strPatternId);
			}
			else
			{
				const auto Found = std::find_if(
					Patterns.begin(), Patterns.end(),
					[&strSharedPatternId](const VALTAN_PATTERN_VIEW* pPattern)
					{
						return nullptr != pPattern &&
							pPattern->strPatternId == strSharedPatternId;
					});
				if (Patterns.end() != Found)
				{
					if (m_strValtanWorkbenchPatternId !=
						(*Found)->strPatternId)
					{
						m_strValtanWorkbenchStageId.clear();
						m_eValtanWorkbenchSelection =
							VALTAN_WORKBENCH_SELECTION_KIND::PATTERN;
					}
					pSelected = *Found;
					m_strValtanWorkbenchPatternId = pSelected->strPatternId;
				}
			}
		}
	}
#endif
	const auto SelectedPatternIterator = std::find_if(
		Patterns.begin(), Patterns.end(),
		[pSelected](const VALTAN_PATTERN_VIEW* pPattern)
		{
			return pPattern == pSelected;
		});
	m_iValtanPatternMasterSelected = static_cast<int32_t>(
		std::distance(Patterns.begin(), SelectedPatternIterator));
	const VALTAN_STAGE_VIEW* pSelectedStage = nullptr;
	const auto SelectedStageIterator = std::find_if(
		pSelected->Stages.begin(), pSelected->Stages.end(),
		[this](const VALTAN_STAGE_VIEW& Stage)
		{
			return Stage.strStageId == m_strValtanWorkbenchStageId;
		});
	if (pSelected->Stages.end() != SelectedStageIterator)
		pSelectedStage = &*SelectedStageIterator;
	else if (!pSelected->Stages.empty())
	{
		pSelectedStage = &pSelected->Stages.front();
		m_strValtanWorkbenchStageId = pSelectedStage->strStageId;
	}

	const auto SelectWorkbenchItem =
		[this, &Patterns, &pSelected, &pSelectedStage](
			const VALTAN_PATTERN_VIEW& Pattern,
			const VALTAN_STAGE_VIEW* pStage,
			const VALTAN_WORKBENCH_SELECTION_KIND eKind)
	{
		const bool_t bPatternChanged =
			m_strValtanWorkbenchPatternId != Pattern.strPatternId;
		pSelected = &Pattern;
		pSelectedStage = pStage;
		if (nullptr == pSelectedStage && !Pattern.Stages.empty())
			pSelectedStage = &Pattern.Stages.front();
		m_strValtanWorkbenchPatternId = Pattern.strPatternId;
		m_strValtanWorkbenchStageId = nullptr == pSelectedStage ?
			std::string{} : pSelectedStage->strStageId;
		m_eValtanWorkbenchSelection = eKind;
		const auto Selected = std::find_if(
			Patterns.begin(), Patterns.end(),
			[&Pattern](const VALTAN_PATTERN_VIEW* pCandidate)
			{
				return nullptr != pCandidate &&
					pCandidate->strPatternId == Pattern.strPatternId;
			});
		if (Patterns.end() != Selected)
		{
			m_iValtanPatternMasterSelected = static_cast<int32_t>(
				std::distance(Patterns.begin(), Selected));
		}
		if (bPatternChanged)
			m_eValtanPatternMasterPath = VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
#ifdef _DEBUG
		if (CMainApp* const pApp = CMainApp::Get_Active())
			(void)pApp->Debug_SelectCompletePlayPattern(Pattern.strPatternId);
#endif
	};

	const bool_t bWorkbenchMainVisible = ImGui::BeginTable(
		"##ValtanWorkbenchMain", 3,
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_BordersInnerV |
		ImGuiTableFlags_SizingStretchProp |
		ImGuiTableFlags_ScrollX,
		ImVec2(0.f, 680.f), WORKBENCH_THREE_PANE_INNER_WIDTH);
	if (!bWorkbenchMainVisible)
	{
		ImGui::TextDisabled(
			"Main panes are outside the current scroll clip; Sequencer and Data Files remain available below.");
	}
	else
	{
	ImGui::TableSetupColumn(
		"Master / Outliner", ImGuiTableColumnFlags_WidthStretch, 0.22f);
	ImGui::TableSetupColumn(
		"Preview / Transport", ImGuiTableColumnFlags_WidthStretch, 0.46f);
	ImGui::TableSetupColumn(
		"Persistent Detail", ImGuiTableColumnFlags_WidthStretch, 0.32f);
	ImGui::TableNextRow();

	ImGui::TableSetColumnIndex(0);
	if (ImGui::BeginChild(
		"##ValtanWorkbenchOutliner", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::SeparatorText("Master / Outliner");
		ImGui::BeginDisabled(
			m_bValtanPatternMasterPlaying ||
			m_bValtanPatternSoundCuesDirty);
		const bool_t bTargetSelected =
			VALTAN_WORKBENCH_SELECTION_KIND::TARGET ==
				m_eValtanWorkbenchSelection;
		if (ImGui::Selectable(
			"Valtan##boss.valtan", bTargetSelected,
			ImGuiSelectableFlags_SpanAllColumns))
		{
			m_eValtanWorkbenchSelection =
				VALTAN_WORKBENCH_SELECTION_KIND::TARGET;
		}
		ImGui::TextDisabled("boss.valtan | Product Server authority");
		for (const VALTAN_PATTERN_VIEW* const pPattern : Patterns)
		{
			if (nullptr == pPattern)
				continue;
			const VALTAN_PATTERN_VIEW& Pattern = *pPattern;
			ImGui::PushID(Pattern.strPatternId.c_str());
			ImGuiTreeNodeFlags PatternFlags =
				ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_SpanAvailWidth;
			if (VALTAN_WORKBENCH_SELECTION_KIND::PATTERN ==
					m_eValtanWorkbenchSelection &&
				m_strValtanWorkbenchPatternId == Pattern.strPatternId)
			{
				PatternFlags |= ImGuiTreeNodeFlags_Selected;
			}
			const std::string PatternLabel = Pattern.strDisplayName + " | " +
				Pattern.strPatternId + "##pattern-tree";
			const bool_t bPatternOpen = ImGui::TreeNodeEx(
				PatternLabel.c_str(), PatternFlags);
			if (ImGui::IsItemClicked())
			{
				SelectWorkbenchItem(
					Pattern, nullptr,
					VALTAN_WORKBENCH_SELECTION_KIND::PATTERN);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"%s | %zu stable stages",
					Pattern.strPatternId.c_str(), Pattern.Stages.size());
			}
			if (bPatternOpen)
			{
				for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				{
					ImGui::PushID(Stage.strStageId.c_str());
					const bool_t bStageSelected =
						VALTAN_WORKBENCH_SELECTION_KIND::STAGE ==
							m_eValtanWorkbenchSelection &&
						m_strValtanWorkbenchPatternId ==
							Pattern.strPatternId &&
						m_strValtanWorkbenchStageId == Stage.strStageId;
					const std::string StageLabel = Stage.strStageId +
						" | " + Stage.strSequenceRole + " | " +
						Stage.strActionId;
					if (ImGui::Selectable(
						StageLabel.c_str(), bStageSelected))
					{
						SelectWorkbenchItem(
							Pattern, &Stage,
							VALTAN_WORKBENCH_SELECTION_KIND::STAGE);
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::EndDisabled();
	}
	ImGui::EndChild();

	ImGui::TableSetColumnIndex(1);
	ImGui::BeginChild(
		"##ValtanWorkbenchPreview", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders);
	ImGui::SeparatorText("Preview / Transport");
	if (pSelected->bManualServerAudition)
	{
		ImGui::TextDisabled(
			"Manual Server audition | phase %u | source chain %s | automatic rotation disabled",
			pSelected->iAuthoringPhase,
			pSelected->strSourceAnimationChainId.c_str());
	}
	ImGui::BeginDisabled(
		m_bValtanPatternMasterPlaying ||
		m_bValtanPatternSoundCuesDirty);
	if (ImGui::BeginCombo(
		"Pattern##ValtanPatternMaster",
		(pSelected->strPatternId + " | " +
			pSelected->strDisplayName).c_str()))
	{
		for (size_t iPattern = 0u; iPattern < Patterns.size(); ++iPattern)
		{
			const VALTAN_PATTERN_VIEW& Pattern = *Patterns[iPattern];
			const std::string Label = Pattern.strPatternId + " | " +
				Pattern.strDisplayName;
			if (ImGui::Selectable(
				Label.c_str(),
				static_cast<int32_t>(iPattern) ==
					m_iValtanPatternMasterSelected))
			{
				SelectWorkbenchItem(
					Pattern, nullptr,
					VALTAN_WORKBENCH_SELECTION_KIND::PATTERN);
			}
		}
		ImGui::EndCombo();
	}
	if ("VALTAN_DASH_CHARGE" == pSelected->strPatternId &&
		ImGui::BeginCombo(
			"Dash authoring path##ValtanPatternMaster",
			ValtanPatternMasterPathName(m_eValtanPatternMasterPath)))
	{
		for (const VALTAN_PATTERN_PREVIEW_PATH eCandidate : {
			VALTAN_PATTERN_PREVIEW_PATH::NORMAL,
			VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY,
			VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY,
			VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK })
		{
			if (ImGui::Selectable(
				ValtanPatternMasterPathName(eCandidate),
				eCandidate == m_eValtanPatternMasterPath))
			{
				m_eValtanPatternMasterPath = eCandidate;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	ImGui::SeparatorText("Playback Modes");
	ImGui::BeginDisabled(!bHasPreviewModel || !bMutationAdmitted);
	if (ImGui::Button("Pattern Offline"))
	{
		Start_ValtanPatternMasterPreview(
			pModel, *pSelected, m_eValtanPatternMasterPath);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bMutationAdmitted);
	if (ImGui::Button("Complete Play (Server/Arena)"))
	{
		std::string Status;
#ifdef _DEBUG
		if (CMainApp* const pApp = CMainApp::Get_Active())
		{
			if (pApp->Debug_SelectCompletePlayPattern(
					pSelected->strPatternId))
			{
				(void)pApp->Debug_CompletePlaySelected(Status);
			}
			else
			{
				Status =
					"Selected Workbench pattern is not in the shared Server inventory.";
			}
		}
		else
			Status = "Complete Play workspace is unavailable.";
#else
		Status = "Complete Play is available only in a Debug authoring build.";
#endif
		m_strValtanPatternMasterStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(
		!bHasPreviewModel || !m_bValtanPatternMasterPlaying);
	if (ImGui::Button(
		m_bValtanPatternMasterPaused ? "Resume Master" : "Pause Master"))
	{
		m_bValtanPatternMasterPaused = !m_bValtanPatternMasterPaused;
		pModel->Set_AnimPaused(true);
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop Master"))
	{
		Stop_ValtanPatternMasterPreview(
			pModel, "Valtan Pattern Master preview stopped; idle restored.");
	}
	ImGui::EndDisabled();
	if (!bHasPreviewModel)
	{
		ImGui::TextDisabled(
			"Local Pattern Offline transport requires the Valtan Model View. Complete Play remains Server-authoritative and requires ADMITTED canonical data.");
	}

	const auto RenderPersistentSave =
		[this, &bReloadPatternMasterAfterSave, bMutationAdmitted]()
	{
	ImGui::SeparatorText("Authoring Revision");
	const bool_t bDraftDirty = nullptr != m_pBalanceTool &&
		m_pBalanceTool->Is_ValtanDraftDirty();
	ImGui::TextDisabled(
		"%s | gameplay wall clock and presentation remain separate joined domains",
		bDraftDirty ? "UNSAVED GAMEPLAY DRAFT" : "saved source");
	if (m_bValtanCombatObjectSoundCuesDirty)
		ImGui::TextColored(ImVec4(1.f, 0.75f, 0.2f, 1.f),
			"UNSAVED SERVER-HIT SOUND DRAFT (read-only until its canonical typed transaction is available)");
	ImGui::BeginDisabled(nullptr == m_pBalanceTool || !bMutationAdmitted);
	if (ImGui::Button("Save"))
	{
		std::string Status;
		if (m_bValtanCombatObjectSoundCuesDirty)
		{
			Status =
				"Save blocked: combat-object Sound is part of immutable presentation M; the legacy direct source replacement was retired and this draft was not written.";
		}
		else if (m_pBalanceTool->Save_ValtanProduct(Status))
		{
			bReloadPatternMasterAfterSave = true;
		}
		m_strValtanPatternMasterStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	if (nullptr != m_pBalanceTool)
	{
		ImGui::TextDisabled("Runtime activation: %s",
			m_pBalanceTool->Get_ValtanCandidateApplyClass().empty() ? "NONE" :
				m_pBalanceTool->Get_ValtanCandidateApplyClass().c_str());
	}
	};

	ImGui::SetNextItemWidth(120.f);
	if (ImGui::SliderFloat(
		"Preview speed##ValtanPatternMaster",
		&m_fValtanPatternPreviewSpeed, 0.25f, 3.f, "%.2fx"))
	{
		m_fValtanPatternPreviewSpeed = std::clamp(
			m_fValtanPatternPreviewSpeed, 0.25f, 3.f);
	}
	if (bHasPreviewModel && m_bValtanPatternMasterPlaying &&
		m_iValtanPatternMasterItem < m_ValtanPatternMasterPlaylist.size())
	{
		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
			m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
		f32_t fTimelineSeconds =
			static_cast<f32_t>(Item.iTimelineStartMs) * 0.001f +
			m_fValtanPatternMasterItemElapsedSeconds;
		const f32_t fDurationSeconds =
			static_cast<f32_t>(m_iValtanPatternMasterDurationMs) * 0.001f;
		char_t TimelineFormat[96]{};
		snprintf(
			TimelineFormat, sizeof(TimelineFormat),
			"%%.3f s / %.3f s", fDurationSeconds);
		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::SliderFloat(
			"##ValtanPatternMasterTimeline",
			&fTimelineSeconds, 0.f, fDurationSeconds, TimelineFormat))
		{
			if (!Seek_ValtanPatternMasterPreview(
				pModel, fTimelineSeconds, true, true))
			{
				m_strValtanPatternMasterStatus =
					"Master seek rejected; current admitted pose preserved.";
			}
		}
		if (Item.bSuppressAnimation)
		{
			ImGui::Text(
				"Now %s / %s | animation NONE | wall %u ms",
				Item.strStageId.c_str(), Item.strSequenceRole.c_str(),
				Item.iAuthoringWallMs);
			ImGui::TextDisabled(
				"Boss pose hold; no additional Valtan animation clip is started.");
		}
		else
		{
			ImGui::Text(
				"Now %s / %s | occurrence %u/%u | wall %u ms",
				Item.strStageId.c_str(), Item.strSequenceRole.c_str(),
				Item.iOccurrenceNumber, Item.iOccurrenceCount,
				Item.iAuthoringWallMs);
			ImGui::TextDisabled(
				"%s | source +%u ms, play %u ms, rate %.6g, repeatUntilStageEnd=%s",
				Item.strClipName.c_str(), Item.iSourceStartMs, Item.iPlayMs,
				Item.fPlayRate,
				Item.bRepeatUntilStageEnd ? "true" : "false");
		}
	}
	if (!m_strValtanPatternMasterStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanPatternMasterStatus.c_str());

	if (ImGui::CollapsingHeader("Selection / Presentation Reference"))
	{
	ImGui::SeparatorText("Phase-1 weighted normal selection");
	ImGui::TextDisabled(
		"%s | five admitted normal patterns; health-bar mechanics keep queue precedence",
		m_ValtanPatternMasterView.NormalSelection.strSelectionMode.c_str());
	for (const VALTAN_NORMAL_SELECTION_RANGE_VIEW& Range :
		m_ValtanPatternMasterView.NormalSelection.Ranges)
	{
		ImGui::BulletText(
			"%s | bars %u > HP bar > %u",
			Range.strRotationId.c_str(), Range.iFromHealthBar,
			Range.iToHealthBar);
	}
	for (const std::string& PatternId :
		m_ValtanPatternMasterView.NormalSelection.PatternIds)
	{
		const auto Found = std::find_if(
			Patterns.begin(), Patterns.end(),
			[&PatternId](const VALTAN_PATTERN_VIEW* pPattern)
			{
				return nullptr != pPattern &&
					pPattern->strPatternId == PatternId;
			});
		if (Found == Patterns.end())
			continue;
		const VALTAN_PATTERN_VIEW& Pattern = **Found;
		ImGui::TextDisabled(
			"%s | weight %u | range %.1f..%.1f | max consecutive %u | %s / %s",
			Pattern.strPatternId.c_str(), Pattern.iSelectionWeight,
			Pattern.fMinimumRange, Pattern.fMaximumRange,
			Pattern.iMaximumConsecutiveUses,
			Pattern.strArmorRequirement.c_str(),
			Pattern.strPhaseRequirement.c_str());
	}

	ImGui::SeparatorText("Counter reaction animation layers (reference only)");
	ImGui::TextDisabled(
		"These exact legacy Encounter actions are not admitted into the seven-pattern Phase-1 pool.");
	for (const VALTAN_COUNTER_REACTION_LAYER_VIEW& Layer :
		m_ValtanPatternMasterView.CounterReactionLayers)
	{
		ImGui::PushID(Layer.strReactionLayerId.c_str());
		const std::string Label = Layer.strReactionLayerId + " | " +
			Layer.strOwnerPatternId + "/" + Layer.strOwnerStageId;
		if (ImGui::TreeNodeEx(Label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
		{
			for (const auto* pAction :
				{ &Layer.Window, &Layer.Success, &Layer.Failure })
			{
				ImGui::TextDisabled("action %s", pAction->strActionId.c_str());
				for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
					pAction->ClipOccurrences)
				{
					ImGui::BulletText(
						"%s | %s", Clip.strClipOccurrenceId.c_str(),
						Clip.strClipName.c_str());
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	ImGui::SeparatorText("Presentation sources");
	for (const VALTAN_PRESENTATION_SOURCE_VIEW& Source :
		pSelected->PresentationSources)
	{
		ImGui::BulletText(
			"action %u | sequence %u | %s",
			Source.iSourceActionId, Source.iSequenceIndex,
			Source.strRole.c_str());
	}
	}

	ImGui::SeparatorText("Selected stage / sequence role");
	std::vector<const VALTAN_STAGE_VIEW*> PreviewPath;
	std::string PreviewPathStatus;
	if (CValtanPatternTree::Build_PreviewStagePath(
		*pSelected, m_eValtanPatternMasterPath,
		PreviewPath, PreviewPathStatus))
	{
		std::string PathLabel = "Current path: ";
		for (size_t iStage = 0u; iStage < PreviewPath.size(); ++iStage)
		{
			if (0u != iStage)
				PathLabel += " -> ";
			PathLabel += PreviewPath[iStage]->strStageId;
		}
		ImGui::TextDisabled("%s", PathLabel.c_str());
	}
	else
	{
		ImGui::TextDisabled("Current path rejected: %s",
			PreviewPathStatus.c_str());
	}
	for (size_t iStage = 0u; iStage < pSelected->Stages.size(); ++iStage)
	{
		const VALTAN_STAGE_VIEW& Stage = pSelected->Stages[iStage];
		if (nullptr == pSelectedStage ||
			Stage.strStageId != pSelectedStage->strStageId)
		{
			continue;
		}
		ImGui::PushID(static_cast<int32_t>(iStage));
		char_t Label[512]{};
		snprintf(
			Label, sizeof(Label),
			"%02zu  %s | %s | %u ms | %s | repeatCount %u | %zu occurrences",
			iStage + 1u, Stage.strStageId.c_str(),
			Stage.strSequenceRole.c_str(), Stage.iDurationMs,
			Stage.strAnimationEndPolicy.c_str(),
			Stage.iAuthoringRepeatCount, Stage.ClipOccurrences.size());
		if (ImGui::TreeNodeEx(Label, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextDisabled("action %s | %s | damage %s",
				Stage.strActionId.c_str(), Stage.strStageKind.c_str(),
				Stage.strServerDamageProfileId.empty() ? "NONE" :
					Stage.strServerDamageProfileId.c_str());
			for (size_t iClip = 0u;
				iClip < Stage.ClipOccurrences.size(); ++iClip)
			{
				const VALTAN_CLIP_OCCURRENCE_VIEW& Clip =
					Stage.ClipOccurrences[iClip];
				ImGui::BulletText(
					"%zu. %s | %s | wall %u ms",
					iClip + 1u, Clip.strClipName.c_str(),
					Clip.strClipOccurrenceId.c_str(),
					Clip.iAuthoringWallMs);
				ImGui::Indent();
				ImGui::TextDisabled(
					"sourceStartMs=%u playMs=%u playRate=%.6g repeatUntilStageEnd=%s",
					Clip.iSourceStartMs, Clip.iPlayMs, Clip.fPlayRate,
					Clip.bLoop ? "true" : "false");
				ImGui::Unindent();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	ImGui::EndChild();

	ImGui::TableSetColumnIndex(2);
	ImGui::BeginChild(
		"##ValtanWorkbenchDetail", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders);
	ImGui::SeparatorText("Persistent Detail");
	if (m_bValtanWorkbenchFocusDetailRequested)
	{
		ImGui::SetScrollHereY(0.f);
		m_bValtanWorkbenchFocusDetailRequested = false;
	}
	const char_t* pSelectionKind = "Stage";
	switch (m_eValtanWorkbenchSelection)
	{
	case VALTAN_WORKBENCH_SELECTION_KIND::TARGET:
		pSelectionKind = "Target";
		break;
	case VALTAN_WORKBENCH_SELECTION_KIND::PATTERN:
		pSelectionKind = "Pattern";
		break;
	case VALTAN_WORKBENCH_SELECTION_KIND::STAGE:
		pSelectionKind = "Stage";
		break;
	}
	ImGui::TextDisabled("Selection: %s", pSelectionKind);
	ImGui::TextWrapped(
		"target boss.valtan | pattern %s | stage %s",
		m_strValtanWorkbenchPatternId.c_str(),
		m_strValtanWorkbenchStageId.empty() ? "NONE" :
			m_strValtanWorkbenchStageId.c_str());
	const auto DetailOwnerButton = [this](
		const char_t* pLabel,
		const VALTAN_WORKBENCH_DETAIL_OWNER eOwner)
	{
		if (ImGui::RadioButton(
			pLabel, eOwner == m_eValtanWorkbenchDetailOwner))
		{
			m_eValtanWorkbenchDetailOwner = eOwner;
		}
	};
	DetailOwnerButton(
		"Gameplay", VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY);
	ImGui::SameLine();
	DetailOwnerButton(
		"Animation", VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION);
	ImGui::SameLine();
	DetailOwnerButton("Effect", VALTAN_WORKBENCH_DETAIL_OWNER::EFFECT);
	DetailOwnerButton("Sound", VALTAN_WORKBENCH_DETAIL_OWNER::SOUND);
	ImGui::SameLine();
	DetailOwnerButton("Camera", VALTAN_WORKBENCH_DETAIL_OWNER::CAMERA);
	ImGui::SameLine();
	DetailOwnerButton("World", VALTAN_WORKBENCH_DETAIL_OWNER::WORLD);

	if (VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY ==
		m_eValtanWorkbenchDetailOwner)
	{
		RenderPersistentSave();
		if (VALTAN_WORKBENCH_SELECTION_KIND::TARGET ==
			m_eValtanWorkbenchSelection)
		{
			ImGui::SeparatorText("Target Detail");
			ImGui::TextWrapped(
				"Valtan is the Product Server-authority target. Select a pattern or stage in the Outliner to edit its admitted typed gameplay draft.");
		}
		else if (nullptr != pSelectedStage)
		{
			ImGui::SeparatorText("Selected Stage Draft");
			ImGui::TextWrapped(
				"%s / %s | action %s | %u ms",
				pSelected->strPatternId.c_str(),
				pSelectedStage->strStageId.c_str(),
				pSelectedStage->strActionId.c_str(),
				pSelectedStage->iDurationMs);
			ImGui::BeginDisabled(!bMutationAdmitted);
			Render_ValtanCounterWindowInspector(
				*pSelected, *pSelectedStage);
			Render_ValtanStageDraftInspector(
				*pSelected, *pSelectedStage);
			ImGui::EndDisabled();
		}
	}
	else if (VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION ==
		m_eValtanWorkbenchDetailOwner)
	{
		if (!bHasPreviewModel)
		{
			ImGui::SeparatorText("Animation Sequence Owner (read-only data mode)");
			ImGui::TextWrapped(
				"Saved Animation occurrence/sequence rows remain visible in the joined Sequencer below. Add, reorder and Save require the exact Valtan Model View clip vocabulary and native durations.");
		}
		else if (nullptr == pSelectedStage ||
			VALTAN_WORKBENCH_SELECTION_KIND::TARGET ==
				m_eValtanWorkbenchSelection)
		{
			ImGui::TextDisabled(
				"Select a pattern or semantic stage to open its typed animation sequence owner.");
		}
		else
		{
			if (Render_ValtanAnimationBindingInspector(
				pModel, *pSelected, *pSelectedStage))
			{
				bReloadPatternMasterAfterSave = true;
			}
		}
	}
	else if (VALTAN_WORKBENCH_DETAIL_OWNER::EFFECT ==
		m_eValtanWorkbenchDetailOwner)
	{
		ImGui::SeparatorText("Effect Owner (read-only here)");
		if (nullptr != pSelectedStage)
		{
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
				pSelectedStage->ProductCues)
			{
				ImGui::PushID(Cue.strOccurrenceId.c_str());
				ImGui::TextWrapped(
					"Data/Effects/Authored/%s.effect.json",
					Cue.strEffectAssetId.c_str());
				if (ImGui::SmallButton("Open Effect Tool##PersistentDetail"))
				{
					m_strEffectToolOpenPatternId = pSelected->strPatternId;
					m_strEffectToolOpenStageId = pSelectedStage->strStageId;
					m_strEffectToolOpenCueOccurrenceId = Cue.strOccurrenceId;
					m_strEffectToolOpenEffectAssetId = Cue.strEffectAssetId;
					m_hasEffectToolOpenRequest = true;
				}
				ImGui::PopID();
			}
		}
	}
	else if (VALTAN_WORKBENCH_DETAIL_OWNER::SOUND ==
		m_eValtanWorkbenchDetailOwner)
	{
		RenderPersistentSave();
		ImGui::BulletText(
			"Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json");
		ImGui::TextWrapped(
			"Server combat-object impact event rows remain typed-editable in the selected Sequencer rows and commit with the Product Save above.");
		if (!bHasPreviewModel)
		{
			ImGui::TextWrapped(
				"Saved Sound rows remain visible in the joined Sequencer. Add/Edit/Save of clip-bound Pattern Sound requires the Model View so clip duration admission cannot be guessed.");
		}
		else if (nullptr != pSelectedStage)
		{
			ImGui::BeginDisabled(!bMutationAdmitted);
			if (Render_ValtanPatternSoundInspector(
				pModel, *pSelected, *pSelectedStage))
			{
				bReloadPatternMasterAfterSave = true;
			}
			ImGui::EndDisabled();
		}
		else
		{
			ImGui::TextDisabled(
				"Select a semantic stage to edit its exact Pattern Sound rows.");
		}
	}
	else if (VALTAN_WORKBENCH_DETAIL_OWNER::CAMERA ==
		m_eValtanWorkbenchDetailOwner)
	{
		ImGui::SeparatorText("Camera Owner (read-only here)");
		if (nullptr != pSelectedStage)
		{
			for (const VALTAN_CAMERA_INVOCATION_VIEW& Invocation :
				pSelectedStage->CameraInvocations)
			{
				ImGui::PushID(Invocation.strCameraInvocationId.c_str());
				ImGui::TextWrapped(
					"%s -> %s", Invocation.strCameraInvocationId.c_str(),
					Invocation.strCameraCueId.c_str());
				if (ImGui::SmallButton("Open Camera Tool##PersistentDetail"))
				{
					m_strCameraToolOpenCueId = Invocation.strCameraCueId;
					m_hasCameraToolOpenRequest = true;
				}
				ImGui::PopID();
			}
		}
	}
	else
	{
		ImGui::SeparatorText("World Owner (read-only here)");
		if (nullptr != pSelectedStage)
		{
			for (const VALTAN_STAGE_BRANCH_VIEW& Branch :
				pSelectedStage->Branches)
			{
				ImGui::BulletText(
					"%s -> %s", Branch.strOutcome.c_str(),
					Branch.strNextActionId.has_value() ?
						Branch.strNextActionId->c_str() : "TERMINAL");
			}
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event :
				pSelected->WorldEventTriggerRefs)
			{
				if (Event.strStageId == pSelectedStage->strStageId)
					ImGui::BulletText("world event %s", Event.strTriggerKind.c_str());
			}
		}
	}
	ImGui::EndChild();
	ImGui::EndTable();
	}

	if (ImGui::BeginChild(
		"##ValtanWorkbenchSequencer", ImVec2(0.f, 520.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::SeparatorText("Sequencer / Joined Tracks");
		if (nullptr != pSelectedStage)
		{
			ImGui::TextDisabled(
				"Selected stage filter: %s / %s | action %s | %u ms",
				pSelected->strPatternId.c_str(),
				pSelectedStage->strStageId.c_str(),
				pSelectedStage->strActionId.c_str(),
				pSelectedStage->iDurationMs);
			ImGui::BeginDisabled(!bMutationAdmitted);
			Render_ValtanPresentationLanes(
				*pSelected, pSelectedStage->strStageId);
			ImGui::EndDisabled();
		}
		else
		{
			ImGui::TextDisabled(
				"Select a semantic stage to join Animation, Effect, Sound, Camera, and World owner tracks.");
		}
	}
	ImGui::EndChild();

	if (ImGui::BeginChild(
		"##ValtanWorkbenchDataFiles", ImVec2(0.f, 360.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::SeparatorText("Data Files");
		ImGui::TextDisabled(
			"Exact owners for the current stable selection. JSON is never edited generically from this pane.");
		if (!m_strValtanPatternMasterStatus.empty())
		{
			ImGui::TextWrapped(
				"Canonical join: %s",
				m_strValtanPatternMasterStatus.c_str());
		}
		Render_ValtanSelectedResourceUsage(*pSelected, pSelectedStage);

		ImGui::BeginDisabled(nullptr == m_pBalanceTool);
		if (ImGui::SmallButton("Open Valtan Balance / Gameplay##DataFiles"))
			m_pBalanceTool->Open_Valtan();
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!bHasPreviewModel);
		if (ImGui::SmallButton("Open Animation Sequence Intake##DataFiles"))
			m_bShowValtanCustomChainWindow = true;
		ImGui::EndDisabled();

		if (nullptr != pSelectedStage)
		{
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
				pSelectedStage->ProductCues)
			{
				ImGui::PushID(Cue.strOccurrenceId.c_str());
				if (ImGui::SmallButton("Open Effect Tool"))
				{
					m_strEffectToolOpenPatternId =
						pSelected->strPatternId;
					m_strEffectToolOpenStageId =
						pSelectedStage->strStageId;
					m_strEffectToolOpenCueOccurrenceId =
						Cue.strOccurrenceId;
					m_strEffectToolOpenEffectAssetId =
						Cue.strEffectAssetId;
					m_hasEffectToolOpenRequest = true;
				}
				ImGui::SameLine();
				ImGui::TextDisabled(
					"%s", Cue.strEffectAssetId.c_str());
				ImGui::PopID();
			}
			for (const VALTAN_CAMERA_INVOCATION_VIEW& Invocation :
				pSelectedStage->CameraInvocations)
			{
				ImGui::PushID(Invocation.strCameraInvocationId.c_str());
				if (ImGui::SmallButton("Open Camera Tool"))
				{
					m_strCameraToolOpenCueId = Invocation.strCameraCueId;
					m_hasCameraToolOpenRequest = true;
				}
				ImGui::SameLine();
				ImGui::TextDisabled(
					"%s", Invocation.strCameraCueId.c_str());
				ImGui::PopID();
			}
		}
	}
	ImGui::EndChild();
	if (bReloadPatternMasterAfterSave)
	{
		/* The previous inline transaction reloaded before publishing its final
		   user-facing Save status. Keep that observable order while deferring the
		   pointer-invalidating reload until every pane has finished this frame. */
		std::string SaveStatus = m_strValtanPatternMasterStatus;
		if (Reload_ValtanPatternMaster())
		{
			m_strValtanPatternMasterStatus = std::move(SaveStatus);
		}
		else
		{
			const std::string ReloadDiagnostic =
				m_strValtanPatternMasterStatus;
			m_strValtanPatternMasterStatus = std::move(SaveStatus) +
				" Joined Workbench reload rejected; previous admitted view preserved. " +
				ReloadDiagnostic;
		}
	}
	}
}

void Client::CAnimation_Tool::Render_ValtanWorkspaceTabs(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (!ImGui::BeginTabBar("##ValtanWorkspaceTabs"))
		return;

	const ImGuiTabItemFlags PatternFlags =
		m_bValtanWorkspaceTabInitialized ? ImGuiTabItemFlags_None :
			ImGuiTabItemFlags_SetSelected;
	m_bValtanWorkspaceTabInitialized = true;
	if (ImGui::BeginTabItem("Pattern Workbench", nullptr, PatternFlags))
	{
		Render_ValtanPatternMaster(pModel);
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Animation Clips / Sequence Intake"))
	{
		Render_ValtanAnimationSourceWorkspace(pModel);
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();
}

void Client::CAnimation_Tool::Render_ValtanAnimationSourceWorkspace(
	const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SeparatorText("Animation Clips / Sequence Intake");
	if (nullptr == pModel)
	{
		ImGui::TextWrapped(
			"The exact Valtan Model View is required for local playback, source reference 1-67, Skill Timing, clip inventory and Animation Sequence Intake. The Pattern Workbench remains fully available in the first tab.");
		return;
	}

	/* Reference and clip documents are deliberately admitted only while this
	   source tab is visible.  The default Product tab never opens Skill Timing
	   or parses the large source/reference inventories as incidental UI. */
	if (!m_bClipMapLoadAttempted)
	{
		m_bClipMapLoadAttempted = true;
		Load_ClipMap();
	}
	if (!m_bClipSeqLoadAttempted)
	{
		m_bClipSeqLoadAttempted = true;
		Load_ClipSeq();
	}
	if (!m_bClipNotifyLoadAttempted)
	{
		m_bClipNotifyLoadAttempted = true;
		Load_ClipNotify();
	}
	if (!m_bRefLoadAttempted)
	{
		m_bRefLoadAttempted = true;
		Load_SkillReference();
	}

	ImGui::Text(
		"Asset: Valtan   Animations: %u", pModel->Get_NumAnimations());
	ImGui::TextDisabled(
		"REFERENCE / INTAKE: local playback never changes Server gameplay or Product owners until an explicit typed promotion succeeds.");
	ImGui::BeginDisabled(
		m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying);
	Render_Playback(pModel);
	ImGui::EndDisabled();
	Render_ValtanPatternPreview(pModel);

	ImGui::SeparatorText("Animation Sequence Intake");
	if (ImGui::SmallButton("Open Animation Sequence Intake"))
		m_bShowValtanCustomChainWindow = true;
	if (m_bShowValtanCustomChainWindow)
		Render_ValtanCustomChainWindow(pModel);

	ImGui::BeginDisabled(
		m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying);
	Render_NotifyReference(pModel);
	Render_HitAreaWires(pModel);
	Render_SkillReference(pModel, true);
	if (!m_Status.empty())
		ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::SeparatorText("Clips");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##filter", "filter by name", m_Filter, sizeof(m_Filter));
	Render_AnimationList(pModel);
	ImGui::EndDisabled();
}

void Client::CAnimation_Tool::Render_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SeparatorText("Secondary / Read-only Source Reference (1-67)");
	ImGui::TextWrapped(
		"Historical reference only: Valtan.patternpreview.json and Valtan.clipseq are source evidence, not the Product pattern master. Every source clip plays once for review; environment, movement, effects, and damage are not simulated here.");

	if (!m_bValtanPatternPreviewLoadAttempted)
	{
		m_bValtanPatternPreviewLoadAttempted = true;
		std::string status;
		VALTAN_PATTERN_PREVIEW_DOCUMENT staged;
		if (CValtanPatternPreviewDocument::Load(
				"Valtan", Collect_ClipNames(pModel), staged, status))
		{
			m_ValtanPatternPreviewDocument = std::move(staged);
			m_strValtanPatternPreviewStatus = status;
		}
		else
		{
			m_strValtanPatternPreviewStatus =
				"Pattern preview unavailable; current animation preserved: " + status;
		}
	}

	const bool_t bReady =
		67u == m_ValtanPatternPreviewDocument.Patterns.size();
	ImGui::BeginDisabled(!bReady);
	if (ImGui::Button("Play All 1-67"))
		Start_ValtanPatternPreview(pModel, 1u, 67u);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	if (ImGui::SliderFloat(
			"Speed##ValtanPatternPreview",
			&m_fValtanPatternPreviewSpeed,
			0.5f,
			3.f,
			"%.1fx"))
	{
		if (m_bValtanPatternPreviewPlaying)
			pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
	}

	const int32_t iPatternCount = static_cast<int32_t>(
		m_ValtanPatternPreviewDocument.Patterns.size());
	if (iPatternCount > 0)
	{
		m_iValtanPatternPreviewSelected = std::clamp(
			m_iValtanPatternPreviewSelected, 0, iPatternCount - 1);
	}
	if (ImGui::Button("< Pattern") && m_iValtanPatternPreviewSelected > 0)
	{
		--m_iValtanPatternPreviewSelected;
		Start_ValtanPatternPreview(
			pModel,
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1),
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1));
	}
	ImGui::SameLine();
	if (ImGui::Button("Local Pattern Preview") && iPatternCount > 0)
	{
		const uint32_t Number =
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1);
		Start_ValtanPatternPreview(pModel, Number, Number);
	}
	ImGui::SameLine();
	if (ImGui::Button("Pattern >") &&
		m_iValtanPatternPreviewSelected + 1 < iPatternCount)
	{
		++m_iValtanPatternPreviewSelected;
		Start_ValtanPatternPreview(
			pModel,
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1),
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1));
	}

	ImGui::BeginDisabled(!m_bValtanPatternPreviewPlaying);
	if (ImGui::Button(m_bValtanPatternPreviewPaused ? "Resume" : "Pause Sequence"))
	{
		m_bValtanPatternPreviewPaused = !m_bValtanPatternPreviewPaused;
		pModel->Set_AnimPaused(m_bValtanPatternPreviewPaused);
	}
	ImGui::SameLine();
	if (ImGui::Button("Replay Step"))
		Activate_ValtanPatternPreviewItem(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Skip Step"))
		Advance_ValtanPatternPreview(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
		Stop_ValtanPatternPreview(pModel, "Pattern preview stopped; idle restored.");
	ImGui::EndDisabled();
	ImGui::EndDisabled();

	if (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size() &&
		m_ValtanPatternPreviewPlaylist[
			m_iValtanPatternPreviewItem].iPatternNumber >= 1u)
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		m_iValtanPatternPreviewSelected = static_cast<int32_t>(
			Item.iPatternNumber - 1u);
	}

	if (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size())
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		ImGui::Text(
			"Now %02u  [%s]  step %u/%u  %s",
			Item.iPatternNumber,
			CValtanPatternPreviewDocument::Evidence_Name(Item.eEvidence),
			Item.iStepNumber,
			Item.iStepCount,
			Item.bPatternMarker ? "(no body animation)" : Item.strClipName.c_str());
		if (Item.bPatternMarker)
		{
			ImGui::TextDisabled("Source sequence: pattern marker only");
		}
		else
		{
			ImGui::Text(
				"Source action %u  sequence %d  repeat %u/%u  source step %u/%u",
				Item.iSourceActionId,
				Item.iSequenceIndex,
				Item.iSequenceRepeatNumber,
				Item.iSequenceRepeatCount,
				Item.iSourceStepNumber,
				Item.iSourceStepCount);
			ImGui::TextWrapped(
				"%s  [%s]",
				Item.strSequenceName.c_str(),
				Item.strSequenceMode.c_str());
		}
		const f32_t Duration = (std::max)(
			0.001f, m_fValtanPatternPreviewItemDurationSeconds);
		ImGui::ProgressBar(
			std::clamp(m_fValtanPatternPreviewElapsedSeconds / Duration, 0.f, 1.f),
			ImVec2(-1.f, 0.f));
		ImGui::TextWrapped("%s", Item.strPatternLabel.c_str());
		ImGui::TextDisabled("%s", Item.strNote.c_str());
	}
	if (!m_strValtanPatternPreviewStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanPatternPreviewStatus.c_str());

	if (bReady && iPatternCount > 0)
	{
		const VALTAN_PATTERN_PREVIEW_ENTRY& SelectedPattern =
			m_ValtanPatternPreviewDocument.Patterns[
				static_cast<std::size_t>(m_iValtanPatternPreviewSelected)];
		ImGui::SeparatorText("Selected source sequences");
		if (SelectedPattern.Sequences.empty())
		{
			ImGui::TextDisabled("No body-animation source sequence for this pattern.");
		}
		else
		{
			for (const VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF& Sequence :
				SelectedPattern.Sequences)
			{
				ImGui::BulletText(
					"action %u  sequence %d  repeat x%u  [%s]",
					Sequence.iSourceActionId,
					Sequence.iSequenceIndex,
					Sequence.iRepeat,
					Sequence.strSequenceMode.c_str());
				ImGui::Indent();
				ImGui::TextWrapped("%s", Sequence.strSequenceName.c_str());
				ImGui::Unindent();
			}
		}
	}

	if (ImGui::SmallButton("Open Read-only Source Sequence Window"))
		m_bShowValtanSourceReferenceWindow = true;
	if (m_bShowValtanSourceReferenceWindow)
		Render_ValtanPatternReferenceWindow(pModel);

	if (bReady)
	{
		const bool_t bListVisible = ImGui::BeginChild(
			"##ValtanPatternPreviewList",
			ImVec2(0.f, 260.f),
			ImGuiChildFlags_Borders,
			ImGuiWindowFlags_NoScrollWithMouse);
		if (bListVisible)
		{
			for (int32_t Index = 0; Index < iPatternCount; ++Index)
			{
				const VALTAN_PATTERN_PREVIEW_ENTRY& Pattern =
					m_ValtanPatternPreviewDocument.Patterns[Index];
				char_t Label[512]{};
				snprintf(
					Label,
					sizeof(Label),
					"%02u  [%s]  %s",
					Pattern.iNumber,
					CValtanPatternPreviewDocument::Evidence_Name(Pattern.eEvidence),
					Pattern.strLabel.c_str());
				ImGui::PushID(Index);
				if (ImGui::Selectable(
						Label, Index == m_iValtanPatternPreviewSelected))
				{
					m_iValtanPatternPreviewSelected = Index;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", Pattern.strNote.c_str());
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
	}
}

void Client::CAnimation_Tool::Render_ValtanPatternReferenceWindow(
	const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SetNextWindowSize(ImVec2(460.f, 560.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(540.f, 60.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(
		"Valtan Source Reference (Read-only)",
		&m_bShowValtanSourceReferenceWindow))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"Secondary source evidence only. One button per read-only Valtan.clipseq sequence; Product authoring must use Valtan Pattern Master in the main Animation Tool window.");

	if (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size())
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		ImGui::Text("Now step %u/%u  %s",
			Item.iStepNumber,
			Item.iStepCount,
			Item.bPatternMarker ? "(no body animation)" :
				Item.strClipName.c_str());
		ImGui::TextWrapped("%s", Item.strPatternLabel.c_str());
		const f32_t Duration = (std::max)(
			0.001f, m_fValtanPatternPreviewItemDurationSeconds);
		ImGui::ProgressBar(
			std::clamp(
				m_fValtanPatternPreviewElapsedSeconds / Duration, 0.f, 1.f),
			ImVec2(-1.f, 0.f));
		if (ImGui::Button(
			m_bValtanPatternPreviewPaused ? "Resume" : "Pause"))
		{
			m_bValtanPatternPreviewPaused = !m_bValtanPatternPreviewPaused;
			pModel->Set_AnimPaused(m_bValtanPatternPreviewPaused);
		}
		ImGui::SameLine();
		if (ImGui::Button("Replay"))
			Activate_ValtanPatternPreviewItem(pModel);
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
		{
			Stop_ValtanPatternPreview(
				pModel, "Pattern preview stopped; idle restored.");
		}
	}
	else
	{
		ImGui::TextDisabled("Nothing playing.");
	}
	ImGui::SetNextItemWidth(120.f);
	if (ImGui::SliderFloat(
			"Speed##ValtanPatternReference",
			&m_fValtanPatternPreviewSpeed, 0.5f, 3.f, "%.1fx") &&
		m_bValtanPatternPreviewPlaying)
	{
		pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
	}

	ImGui::Checkbox(
		"Raid patterns only (420xxx)", &m_bValtanRaidSequencesOnly);
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##valtanpatternfilter",
		"filter by skill id or name",
		m_ValtanPatternFilter,
		sizeof(m_ValtanPatternFilter));

	if (ImGui::BeginChild(
		"##ValtanPatternReferenceButtons",
		ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		if (m_ClipSeqs.empty())
		{
			ImGui::TextDisabled(
				"No clip sequences loaded from Valtan.clipseq.");
		}
		const int32_t iSequenceCount =
			static_cast<int32_t>(m_ClipSeqs.size());
		for (int32_t Index = 0; Index < iSequenceCount; ++Index)
		{
			const CLIP_SEQ& Seq = m_ClipSeqs[Index];
			if (m_bValtanRaidSequencesOnly &&
				(Seq.iSkillId < 420000 || Seq.iSkillId > 429999))
			{
				continue;
			}
			char_t SkillId[16]{};
			snprintf(SkillId, sizeof(SkillId), "%d", Seq.iSkillId);
			if (!Contains_NoCase(SkillId, m_ValtanPatternFilter) &&
				!Contains_NoCase(Seq.name.c_str(), m_ValtanPatternFilter))
			{
				continue;
			}
			char_t Label[512]{};
			snprintf(Label, sizeof(Label), "%s  seq%d [%s]  (%zu clips)##%d",
				Seq.name.empty() ? SkillId : Seq.name.c_str(),
				Seq.iSeqIndex,
				Seq.sMode.c_str(),
				Seq.clips.size(),
				Index);
			ImGui::PushID(Index);
			const bool_t bPlayingThis = m_bValtanPatternPreviewPlaying &&
				m_iValtanSequenceSelected == Index;
			if (bPlayingThis)
			{
				ImGui::PushStyleColor(
					ImGuiCol_Button, ImVec4(0.20f, 0.45f, 0.24f, 1.f));
			}
			if (ImGui::Button(Label, ImVec2(-1.f, 0.f)))
				Start_ValtanSequencePreview(
					pModel, static_cast<std::size_t>(Index));
			if (bPlayingThis)
				ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				std::string clipList;
				for (const std::string& clip : Seq.clips)
				{
					if (!clipList.empty())
						clipList += "\n";
					clipList += clip;
				}
				ImGui::SetTooltip("skill %d  seq %d\n%s",
					Seq.iSkillId, Seq.iSeqIndex, clipList.c_str());
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::End();
}

bool_t Client::CAnimation_Tool::Start_ValtanSequencePreview(
	const shared_ptr<Engine::CModel>& pModel,
	const std::size_t iSequenceIndex)
{
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		m_strValtanPatternPreviewStatus =
			"Sequence preview start rejected because the animation target changed.";
		return false;
	}
	if (iSequenceIndex >= m_ClipSeqs.size())
		return false;
	const CLIP_SEQ& Seq = m_ClipSeqs[iSequenceIndex];
	if (Seq.clips.empty())
	{
		m_strValtanPatternPreviewStatus =
			"Sequence preview start rejected because the sequence has no clips.";
		return false;
	}

	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> staged;
	staged.reserve(Seq.clips.size());
	const uint32_t iStepCount = static_cast<uint32_t>(Seq.clips.size());
	std::string label = Seq.name.empty() ?
		std::to_string(Seq.iSkillId) : Seq.name;
	label += " seq" + std::to_string(Seq.iSeqIndex) +
		" [" + Seq.sMode + "]";
	const bool_t bHasCuts = Seq.cuts.size() == Seq.clips.size();
	for (uint32_t iStep = 0u; iStep < iStepCount; ++iStep)
	{
		VALTAN_PATTERN_PREVIEW_PLAY_ITEM item;
		item.strPatternLabel = label;
		item.iSourceActionId = static_cast<uint32_t>(Seq.iSkillId);
		item.iSequenceIndex = Seq.iSeqIndex;
		item.iSequenceRepeatNumber = 1u;
		item.iSequenceRepeatCount = 1u;
		item.iSourceStepNumber = iStep + 1u;
		item.iSourceStepCount = iStepCount;
		item.strSequenceName = Seq.name;
		item.strSequenceMode = Seq.sMode;
		item.strClipName = Seq.clips[iStep];
		item.iStepNumber = iStep + 1u;
		item.iStepCount = iStepCount;
		if (bHasCuts && std::isfinite(Seq.cuts[iStep]))
		{
			/* The source stage hands over here; a near-zero cut is a stage
			the original skips through instantly. */
			if (Seq.cuts[iStep] < 0.02f)
				continue;
			item.fAuthoredDurationSeconds = Seq.cuts[iStep];
		}
		else
		{
			const auto length = m_ClipLength.find(item.strClipName);
			if (m_ClipLength.end() != length && length->second > 0.f &&
				std::isfinite(length->second))
			{
				item.fAuthoredDurationSeconds = length->second;
			}
		}
		staged.push_back(std::move(item));
	}
	if (staged.empty())
	{
		m_strValtanPatternPreviewStatus =
			"Sequence preview start rejected because every step is skipped.";
		return false;
	}
	if (m_bValtanPatternMasterPlaying)
	{
		Reset_ValtanPatternMasterPreviewState(
			"Valtan Pattern Master yielded to the read-only source reference.");
	}

	m_ValtanPatternPreviewPlaylist = std::move(staged);
	m_iValtanPatternPreviewItem = 0u;
	m_bValtanPatternPreviewPlaying = true;
	m_bValtanPatternPreviewPaused = false;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_ValtanPatternPreviewModel = pModel;
	m_iValtanPatternPreviewTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iValtanSequenceSelected = static_cast<int32_t>(iSequenceIndex);
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	m_strValtanPatternPreviewStatus =
		"Playing source sequence " + label + ".";
	return Activate_ValtanPatternPreviewItem(pModel);
}

std::filesystem::path Client::CAnimation_Tool::Get_CustomChainFilePath() const
{
	const CUSTOM_CHAIN_PROFILE* pProfile =
		Find_CustomChainProfile(m_AssetName);
	if (nullptr == pProfile)
		return std::filesystem::path{};
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Valtan") / pProfile->pFileName);
}

bool_t Client::CAnimation_Tool::Read_CustomChainDocument(
	const std::filesystem::path& source,
	std::vector<CUSTOM_CHAIN_ENTRY>& Out,
	std::string& strOutError) const
{
	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, source.c_str(), L"rb") || nullptr == file)
	{
		strOutError = "could not open " + source.filename().string();
		return false;
	}
	std::string text;
	char_t buffer[4096]{};
	size_t read = 0u;
	while (0u < (read = fread(buffer, 1u, sizeof(buffer), file)))
		text.append(buffer, read);
	fclose(file);
	return Parse_CustomChainDocument(text, Out, strOutError);
}

bool_t Client::CAnimation_Tool::Parse_CustomChainDocument(
	const std::string& text,
	std::vector<CUSTOM_CHAIN_ENTRY>& Out,
	std::string& strOutError) const
{
	DATA_JSON_VALUE root;
	std::string error;
	if (!CDataJson::Parse(text, root, error) || !root.Is_Object())
	{
		strOutError = "document is unreadable: " + error;
		return false;
	}
	const DATA_JSON_VALUE* pSchema = root.Find("schema");
	const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
	const DATA_JSON_VALUE* pBoss = root.Find("bossArchetypeId");
	const DATA_JSON_VALUE* pEncounter = root.Find("encounterId");
	const DATA_JSON_VALUE* pChains = root.Find("chains");
	if (5u != root.Get_Object().size() ||
		nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != "lostark.valtan-pattern-presentation-debug" ||
		nullptr == pVersion || !pVersion->Is_Number() ||
		1.0 != pVersion->Get_Number() ||
		nullptr == pBoss || !pBoss->Is_String() ||
		pBoss->Get_String() != "BOSS_VALTAN" ||
		nullptr == pEncounter || !pEncounter->Is_String() ||
		pEncounter->Get_String() != "ENCOUNTER_VALTAN" ||
		nullptr == pChains || !pChains->Is_Array())
	{
		strOutError = "document header/version/owner or chain array is invalid";
		return false;
	}

	/* Every row is strict and the result is staged first.  Skipping one broken
	   chain/occurrence would turn a malformed owner into a valid-looking partial
	   library and a later Save would permanently erase the skipped data. */
	std::vector<CUSTOM_CHAIN_ENTRY> staged;
	std::unordered_set<std::string> chainIds;
	std::unordered_set<std::string> occurrenceIds;
	for (size_t iChain = 0u; iChain < pChains->Get_Array().size(); ++iChain)
	{
		const DATA_JSON_VALUE& Chain = pChains->Get_Array()[iChain];
		if (!Chain.Is_Object() || 4u != Chain.Get_Object().size())
		{
			strOutError = "chain[" + std::to_string(iChain) +
				"] is not an exact object";
			return false;
		}
		const DATA_JSON_VALUE* pId = Chain.Find("chainId");
		const DATA_JSON_VALUE* pPattern = Chain.Find("targetPatternId");
		const DATA_JSON_VALUE* pStage = Chain.Find("targetStageId");
		const DATA_JSON_VALUE* pAnimation = Chain.Find("animation");
		if (nullptr == pId || !pId->Is_String() || pId->Get_String().empty() ||
			!chainIds.insert(pId->Get_String()).second ||
			nullptr == pPattern || !pPattern->Is_String() ||
			nullptr == pStage || !pStage->Is_String() ||
			nullptr == pAnimation || !pAnimation->Is_Object() ||
			3u != pAnimation->Get_Object().size())
		{
			strOutError = "chain[" + std::to_string(iChain) +
				"] has an invalid/duplicate identity, target, or animation";
			return false;
		}
		const DATA_JSON_VALUE* pEndPolicy = pAnimation->Find("endPolicy");
		const DATA_JSON_VALUE* pRepeatCount = pAnimation->Find("repeatCount");
		const DATA_JSON_VALUE* pOccurrences = pAnimation->Find("occurrences");
		if (nullptr == pEndPolicy || !pEndPolicy->Is_String() ||
			(pEndPolicy->Get_String() != "EXACT" &&
			 pEndPolicy->Get_String() != "NATIVE_CLIP_LENGTHS") ||
			nullptr == pRepeatCount || !pRepeatCount->Is_Number() ||
			!std::isfinite(pRepeatCount->Get_Number()) ||
			std::floor(pRepeatCount->Get_Number()) != pRepeatCount->Get_Number() ||
			nullptr == pOccurrences || !pOccurrences->Is_Array() ||
			pOccurrences->Get_Array().empty() ||
			pOccurrences->Get_Array().size() > 64u ||
			pRepeatCount->Get_Number() !=
				static_cast<double>(pOccurrences->Get_Array().size()))
		{
			strOutError = "chain[" + std::to_string(iChain) +
				"] has an invalid end policy, repeat count, or occurrence array";
			return false;
		}

		CUSTOM_CHAIN_ENTRY Entry;
		Entry.chainId = pId->Get_String();
		Entry.targetPatternId = pPattern->Get_String();
		Entry.targetStageId = pStage->Get_String();
		for (size_t iOccurrence = 0u;
			iOccurrence < pOccurrences->Get_Array().size(); ++iOccurrence)
		{
			const DATA_JSON_VALUE& Occurrence =
				pOccurrences->Get_Array()[iOccurrence];
			if (!Occurrence.Is_Object() || 7u != Occurrence.Get_Object().size())
			{
				strOutError = "chain[" + std::to_string(iChain) +
					"].occurrence[" + std::to_string(iOccurrence) +
					"] is not an exact object";
				return false;
			}
			const DATA_JSON_VALUE* pOccurrenceId =
				Occurrence.Find("clipOccurrenceId");
			const DATA_JSON_VALUE* pClip = Occurrence.Find("clip");
			const DATA_JSON_VALUE* pMappingBasis =
				Occurrence.Find("mappingBasis");
			const DATA_JSON_VALUE* pSourceStart =
				Occurrence.Find("sourceStartMs");
			const DATA_JSON_VALUE* pPlayMs = Occurrence.Find("playMs");
			const DATA_JSON_VALUE* pPlayRate = Occurrence.Find("playRate");
			const DATA_JSON_VALUE* pRepeatUntilStageEnd =
				Occurrence.Find("repeatUntilStageEnd");
			if (nullptr == pOccurrenceId || !pOccurrenceId->Is_String() ||
				pOccurrenceId->Get_String().empty() ||
				!occurrenceIds.insert(pOccurrenceId->Get_String()).second ||
				nullptr == pClip || !pClip->Is_String() ||
				pClip->Get_String().empty() ||
				nullptr == pMappingBasis || !pMappingBasis->Is_String() ||
				pMappingBasis->Get_String() != "PROJECT_AUTHORED" ||
				nullptr == pSourceStart || !pSourceStart->Is_Number() ||
				!std::isfinite(pSourceStart->Get_Number()) ||
				std::floor(pSourceStart->Get_Number()) != pSourceStart->Get_Number() ||
				pSourceStart->Get_Number() < 0.0 ||
				nullptr == pPlayMs || !pPlayMs->Is_Number() ||
				!std::isfinite(pPlayMs->Get_Number()) ||
				std::floor(pPlayMs->Get_Number()) != pPlayMs->Get_Number() ||
				pPlayMs->Get_Number() < 0.0 ||
				pPlayMs->Get_Number() > static_cast<double>(
					(std::numeric_limits<int32_t>::max)()) ||
				nullptr == pPlayRate || !pPlayRate->Is_Number() ||
				!std::isfinite(pPlayRate->Get_Number()) ||
				pPlayRate->Get_Number() <= 0.0 ||
				nullptr == pRepeatUntilStageEnd ||
				!pRepeatUntilStageEnd->Is_Boolean())
			{
				strOutError = "chain[" + std::to_string(iChain) +
					"].occurrence[" + std::to_string(iOccurrence) +
					"] has invalid identity, clip, timing, rate, or loop fields";
				return false;
			}
			CUSTOM_CHAIN_STEP Step;
			Step.clipName = pClip->Get_String();
			Step.fDurationSeconds =
				static_cast<f32_t>(pPlayMs->Get_Number()) * 0.001f;
			Entry.steps.push_back(std::move(Step));
		}
		staged.push_back(std::move(Entry));
	}

	Out = std::move(staged);
	strOutError.clear();
	return true;
}

bool_t Client::CAnimation_Tool::Load_CustomChainLibrary()
{
	const std::filesystem::path source = Get_CustomChainFilePath();
	if (source.empty())
	{
		m_CustomChainLibrary.clear();
		m_strCustomChainStatus =
			"Custom chains are not admitted for this target.";
		return false;
	}
	std::error_code existsError;
	if (!std::filesystem::exists(source, existsError) || existsError)
	{
		/* Nothing saved yet is the normal first state, not a failure. */
		m_CustomChainLibrary.clear();
		m_strCustomChainStatus.clear();
		return true;
	}

	std::vector<CUSTOM_CHAIN_ENTRY> staged;
	std::string error;
	if (!Read_CustomChainDocument(source, staged, error))
	{
		m_strCustomChainStatus =
			"Debug chain file rejected; saved chains preserved: " + error;
		return false;
	}

	m_CustomChainLibrary = std::move(staged);
	m_strCustomChainStatus = "Loaded " +
		std::to_string(m_CustomChainLibrary.size()) + " saved chains.";
	return true;
}

bool_t Client::CAnimation_Tool::Save_CustomChainLibrary()
{
	const std::filesystem::path destination = Get_CustomChainFilePath();
	const CUSTOM_CHAIN_PROFILE* pProfile =
		Find_CustomChainProfile(m_AssetName);
	if (destination.empty() || nullptr == pProfile)
	{
		m_strCustomChainStatus =
			"Save rejected: custom chains are not admitted for this target.";
		return false;
	}
	SCOPED_VALTAN_PATTERN_TRANSACTION_LOCK TransactionLock;
	std::string strLockError;
	if (!TransactionLock.Try_Acquire(
			CProjectDataRoot::Get().parent_path(), strLockError))
	{
		m_strCustomChainStatus =
			"Save rejected before mutation: " + strLockError +
			". The in-memory chain and previous source bytes were preserved.";
		return false;
	}
	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		m_strCustomChainStatus =
			"Save failed to create the Data directory: " +
			directoryError.message();
		return false;
	}

	std::filesystem::path temporary = destination;
	temporary += L".tmp";
	std::error_code removeError;
	std::filesystem::remove(temporary, removeError);

	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, temporary.c_str(), L"wb") || nullptr == file)
	{
		m_strCustomChainStatus =
			"Save failed to open a temporary file; previous chains preserved.";
		return false;
	}

	bool_t bWritten = 0 <= fprintf(file,
		"{\n"
		"  \"schema\": \"lostark.valtan-pattern-presentation-debug\",\n"
		"  \"formatVersion\": 1,\n"
		"  \"bossArchetypeId\": \"BOSS_VALTAN\",\n"
		"  \"encounterId\": \"ENCOUNTER_VALTAN\",\n"
		"  \"chains\": [\n");
	for (size_t iChain = 0u; iChain < m_CustomChainLibrary.size(); ++iChain)
	{
		const CUSTOM_CHAIN_ENTRY& Entry = m_CustomChainLibrary[iChain];
		/* A step left at zero has no authored wall, so the chain cannot claim
		   it fills a Server stage exactly. The distinction is recorded rather
		   than guessed away, because merging it is a decision, not a rename. */
		bool_t bEveryStepAuthored = true;
		for (const CUSTOM_CHAIN_STEP& Step : Entry.steps)
		{
			if (Step.fDurationSeconds <= 0.f)
				bEveryStepAuthored = false;
		}
		bWritten = bWritten && 0 <= fprintf(file,
			"    {\n"
			"      \"chainId\": \"%s\",\n"
			"      \"targetPatternId\": \"%s\",\n"
			"      \"targetStageId\": \"%s\",\n"
			"      \"animation\": {\n"
			"        \"endPolicy\": \"%s\",\n"
			"        \"repeatCount\": %zu,\n"
			"        \"occurrences\": [\n",
			CDataJson::Escape(Entry.chainId).c_str(),
			CDataJson::Escape(Entry.targetPatternId).c_str(),
			CDataJson::Escape(Entry.targetStageId).c_str(),
			bEveryStepAuthored ? "EXACT" : "NATIVE_CLIP_LENGTHS",
			Entry.steps.size());
		for (size_t iStep = 0u; iStep < Entry.steps.size(); ++iStep)
		{
			const CUSTOM_CHAIN_STEP& Step = Entry.steps[iStep];
			const int32_t iPlayMs = static_cast<int32_t>(
				std::lround(Step.fDurationSeconds * 1000.f));
			bWritten = bWritten && 0 <= fprintf(file,
				"          {\n"
				"            \"clipOccurrenceId\": \"%s.%s.clip.%02zu\",\n"
				"            \"clip\": \"%s\",\n"
				"            \"mappingBasis\": \"PROJECT_AUTHORED\",\n"
				"            \"sourceStartMs\": 0,\n"
				"            \"playMs\": %d,\n"
				"            \"playRate\": 1.0,\n"
				"            \"repeatUntilStageEnd\": false\n"
				"          }%s\n",
				pProfile->pOccurrencePrefix,
				CDataJson::Escape(Entry.chainId).c_str(),
				iStep + 1u,
				CDataJson::Escape(Step.clipName).c_str(),
				iPlayMs < 0 ? 0 : iPlayMs,
				iStep + 1u == Entry.steps.size() ? "" : ",");
		}
		bWritten = bWritten && 0 <= fprintf(file,
			"        ]\n"
			"      }\n"
			"    }%s\n",
			iChain + 1u == m_CustomChainLibrary.size() ? "" : ",");
	}
	bWritten = bWritten && 0 <= fprintf(file, "  ]\n}\n");
	const bool_t bClosed = 0 == fclose(file);

	if (!bWritten || !bClosed)
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_strCustomChainStatus =
			"Save failed while writing; previous chains preserved.";
		return false;
	}

	if (FALSE == MoveFileExW(temporary.c_str(), destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		const DWORD iReplaceError = GetLastError();
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_strCustomChainStatus =
			"Save failed to atomically replace the file; previous chains "
			"preserved. Win32 error " + std::to_string(iReplaceError) + ".";
		return false;
	}

	m_strCustomChainStatus = "Saved " +
		std::to_string(m_CustomChainLibrary.size()) + " chains to " +
		destination.filename().string() + ".";
	return true;
}

void Client::CAnimation_Tool::Render_ValtanPatternCreatePanel()
{
	ImGui::SeparatorText("Create New Pattern");
	ImGui::TextWrapped(
		"Promote one reviewed Animation Intake chain into a new audition-only "
		"Valtan pattern. Validate performs the full staged transaction without "
		"writing. Apply is enabled only for those exact validated request bytes.");
	ImGui::TextDisabled(
		"Defaults: MANUAL_SERVER_AUDITION / AUDITION_ONLY; hit, motion and "
		"special gameplay logic remain NONE until authored by their typed owners.");

	const bool_t bBusy = nullptr != m_hValtanPatternCreateProcess;
	const auto InvalidateValidation = [this]()
	{
		m_strValtanPatternCreateValidatedRequestSha256.clear();
	};
	ImGui::BeginDisabled(bBusy);
	const char_t* const SourceKinds[] = {
		"Current assembled chain", "Saved intake chain" };
	ImGui::SetNextItemWidth(240.f);
	if (ImGui::Combo(
		"Intake source", &m_iValtanPatternCreateSourceKind,
		SourceKinds, static_cast<int32_t>(std::size(SourceKinds))))
	{
		InvalidateValidation();
	}
	m_iValtanPatternCreateSourceKind = std::clamp(
		m_iValtanPatternCreateSourceKind, 0, 1);

	if (1 == m_iValtanPatternCreateSourceKind)
	{
		if (m_CustomChainLibrary.empty())
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.6f, 0.25f, 1.f),
				"No saved intake chain is available. Save or Reload Animation Intake first.");
		}
		else
		{
			m_iValtanPatternCreateSavedIndex = std::clamp(
				m_iValtanPatternCreateSavedIndex, 0,
				static_cast<int32_t>(m_CustomChainLibrary.size() - 1u));
			const CUSTOM_CHAIN_ENTRY& Selected = m_CustomChainLibrary[
				static_cast<size_t>(m_iValtanPatternCreateSavedIndex)];
			ImGui::SetNextItemWidth(360.f);
			if (ImGui::BeginCombo(
				"Saved intake chain", Selected.chainId.c_str()))
			{
				for (size_t iChain = 0u;
					iChain < m_CustomChainLibrary.size(); ++iChain)
				{
					const bool_t bSelected = iChain == static_cast<size_t>(
						m_iValtanPatternCreateSavedIndex);
					if (ImGui::Selectable(
						m_CustomChainLibrary[iChain].chainId.c_str(), bSelected))
					{
						m_iValtanPatternCreateSavedIndex =
							static_cast<int32_t>(iChain);
						InvalidateValidation();
					}
					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::TextDisabled(
				"Selected: %s | %zu clips",
				Selected.chainId.c_str(), Selected.steps.size());
		}
	}
	else
	{
		ImGui::TextDisabled(
			"Current assembled chain: %s | %zu clips",
			'\0' == m_CustomChainId[0] ? "<name required>" :
				m_CustomChainId,
			m_CustomChainSteps.size());
	}

	ImGui::SetNextItemWidth(360.f);
	if (ImGui::InputTextWithHint(
		"Stable patternId", "VALTAN_MY_NEW_PATTERN",
		m_ValtanPatternCreatePatternId,
		sizeof(m_ValtanPatternCreatePatternId)))
	{
		InvalidateValidation();
	}
	ImGui::SetNextItemWidth(360.f);
	if (ImGui::InputTextWithHint(
		"Display name", "Korean or English authoring label",
		m_ValtanPatternCreateDisplayName,
		sizeof(m_ValtanPatternCreateDisplayName)))
	{
		InvalidateValidation();
	}
	ImGui::SetNextItemWidth(200.f);
	if (ImGui::SliderInt(
		"Authoring phase", &m_iValtanPatternCreateAuthoringPhase, 1, 3))
	{
		InvalidateValidation();
	}
	ImGui::SetNextItemWidth(300.f);
	if (ImGui::Combo(
		"Target policy", &m_iValtanPatternCreateTargetPolicy,
		VALTAN_PATTERN_CREATE_TARGET_POLICIES.data(),
		static_cast<int32_t>(VALTAN_PATTERN_CREATE_TARGET_POLICIES.size())))
	{
		InvalidateValidation();
	}
	ImGui::SetNextItemWidth(300.f);
	if (ImGui::Combo(
		"Aim policy", &m_iValtanPatternCreateAimPolicy,
		VALTAN_PATTERN_CREATE_AIM_POLICIES.data(),
		static_cast<int32_t>(VALTAN_PATTERN_CREATE_AIM_POLICIES.size())))
	{
		InvalidateValidation();
	}
	ImGui::EndDisabled();

	if ((0 == m_iValtanPatternCreateTargetPolicy) !=
		(0 == m_iValtanPatternCreateAimPolicy))
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.6f, 0.25f, 1.f),
			"Target and Aim must both be NONE or both select a lock policy.");
	}
	const bool_t bBalanceDirty = nullptr == m_pBalanceTool ||
		m_pBalanceTool->Is_ValtanDraftDirty();
	const bool_t bOtherValtanOwnerDirty = Is_ValtanDocumentDirty();
	const bool_t bMutationAdmitted =
		VALTAN_PATTERN_MASTER_ADMISSION_STATE::ADMITTED ==
			m_eValtanPatternMasterAdmission;
	if (bBalanceDirty || bOtherValtanOwnerDirty)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.6f, 0.25f, 1.f),
			"Apply blocked: save or discard every Balance / Valtan Animation-Sound owner draft first. Validate remains read-only.");
	}
	if (!bMutationAdmitted)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.6f, 0.25f, 1.f),
			"Apply blocked: canonical admission is %s. A preserved stale view is diagnostic-only.",
			ValtanPatternMasterAdmissionLabel());
	}

	ImGui::BeginDisabled(bBusy);
	if (ImGui::Button("Validate Create Request"))
		(void)Start_ValtanPatternCreateCommand(false);
	ImGui::SameLine();
	ImGui::BeginDisabled(
		m_strValtanPatternCreateValidatedRequestSha256.empty() ||
		bBalanceDirty || bOtherValtanOwnerDirty || !bMutationAdmitted);
	if (ImGui::Button("Apply Create Pattern"))
		(void)Start_ValtanPatternCreateCommand(true);
	ImGui::EndDisabled();
	ImGui::EndDisabled();

	if (!m_strValtanPatternCreateValidatedRequestSha256.empty())
	{
		ImGui::TextColored(
			ImVec4(0.35f, 0.9f, 0.45f, 1.f),
			"VALIDATED request SHA-256: %s",
			m_strValtanPatternCreateValidatedRequestSha256.c_str());
	}
	if (!m_strValtanPatternCreateStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanPatternCreateStatus.c_str());
	if (!m_ValtanPatternCreateRequestPath.empty())
	{
		ImGui::TextDisabled(
			"Request file (preserved): %s",
			m_ValtanPatternCreateRequestPath.string().c_str());
	}
	if (!m_ValtanPatternCreateDiagnosticPath.empty())
	{
		ImGui::TextDisabled(
			"Diagnostic file (preserved): %s",
			m_ValtanPatternCreateDiagnosticPath.string().c_str());
	}
	if (m_bValtanPatternCreateHasExitCode)
		ImGui::TextDisabled(
			"Last exit code: %u", m_iValtanPatternCreateExitCode);
	if (!m_strValtanPatternCreateDiagnostic.empty() &&
		ImGui::CollapsingHeader("Create process diagnostic"))
	{
		ImGui::TextWrapped("%s", m_strValtanPatternCreateDiagnostic.c_str());
	}
}

bool_t Client::CAnimation_Tool::Build_ValtanPatternCreateRequest(
	std::string& strOutRequest,
	std::string& strOutError) const
{
	strOutRequest.clear();
	strOutError.clear();
	const std::string strPatternId = m_ValtanPatternCreatePatternId;
	const std::string strDisplayName = m_ValtanPatternCreateDisplayName;
	if (!Is_StablePatternAuthoringId(strPatternId))
	{
		strOutError =
			"patternId must be a 1..160 character stable ID [A-Za-z0-9_.-].";
		return false;
	}
	const size_t iDisplayFirst = strDisplayName.find_first_not_of(" \t\r\n");
	const size_t iDisplayLast = strDisplayName.find_last_not_of(" \t\r\n");
	if (strDisplayName.empty() || 0u != iDisplayFirst ||
		iDisplayLast != strDisplayName.size() - 1u)
	{
		strOutError = "Display name must be non-empty and trimmed.";
		return false;
	}
	if (m_iValtanPatternCreateAuthoringPhase < 1 ||
		m_iValtanPatternCreateAuthoringPhase > 3 ||
		m_iValtanPatternCreateTargetPolicy < 0 ||
		static_cast<size_t>(m_iValtanPatternCreateTargetPolicy) >=
			VALTAN_PATTERN_CREATE_TARGET_POLICIES.size() ||
		m_iValtanPatternCreateAimPolicy < 0 ||
		static_cast<size_t>(m_iValtanPatternCreateAimPolicy) >=
			VALTAN_PATTERN_CREATE_AIM_POLICIES.size())
	{
		strOutError = "Create New Pattern policy or phase selection is invalid.";
		return false;
	}
	if ((0 == m_iValtanPatternCreateTargetPolicy) !=
		(0 == m_iValtanPatternCreateAimPolicy))
	{
		strOutError =
			"Target and Aim must both be NONE or both select a lock policy.";
		return false;
	}

	const std::filesystem::path SourcePath = Get_CustomChainFilePath();
	std::string strSourceBytes;
	if (SourcePath.empty() || !Read_BoundedFile(
			SourcePath, VALTAN_PATTERN_CREATE_MAX_DIAGNOSTIC_BYTES,
			strSourceBytes, strOutError))
	{
		strOutError = "Could not read the exact Animation Intake source: " +
			strOutError;
		return false;
	}
	const std::string strSourceSha256 =
		CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(strSourceBytes);
	if (!Is_LowerSha256(strSourceSha256))
	{
		strOutError = "Could not compute the Animation Intake source SHA-256.";
		return false;
	}

	std::ostringstream Request;
	Request << R"json({
  "schema": "lostark.valtan-animation-pattern-create-request",
  "formatVersion": 1,
  "expectedSourceSha256": ")json" << strSourceSha256 << R"json(",
  "patternId": ")json" << CDataJson::Escape(strPatternId) << R"json(",
  "displayName": ")json" << CDataJson::Escape(strDisplayName) << R"json(",
  "authoringPhase": )json" << m_iValtanPatternCreateAuthoringPhase << R"json(,
  "targetPolicy": ")json" << VALTAN_PATTERN_CREATE_TARGET_POLICIES[
		static_cast<size_t>(m_iValtanPatternCreateTargetPolicy)] << R"json(",
  "aimPolicy": ")json" << VALTAN_PATTERN_CREATE_AIM_POLICIES[
		static_cast<size_t>(m_iValtanPatternCreateAimPolicy)] << R"json(",
  "intakeChain": )json";

	if (1 == m_iValtanPatternCreateSourceKind)
	{
		if (m_CustomChainLibrary.empty() ||
			m_iValtanPatternCreateSavedIndex < 0 ||
			static_cast<size_t>(m_iValtanPatternCreateSavedIndex) >=
				m_CustomChainLibrary.size())
		{
			strOutError = "Select one saved intake chain.";
			return false;
		}
		const CUSTOM_CHAIN_ENTRY& Entry = m_CustomChainLibrary[
			static_cast<size_t>(m_iValtanPatternCreateSavedIndex)];
		if (!Is_StablePatternAuthoringId(Entry.chainId))
		{
			strOutError = "The selected saved chain has an invalid stable ID.";
			return false;
		}
		Request << R"json({
    "selectionKind": "SAVED_INTAKE_CHAIN",
    "sourceChainId": ")json" << CDataJson::Escape(Entry.chainId) << R"json("
  }
})json";
	}
	else
	{
		const std::string strChainId = m_CustomChainId;
		if (!Is_StablePatternAuthoringId(strChainId))
		{
			strOutError =
				"Current assembled chain needs a stable chain name before validation.";
			return false;
		}
		if (m_CustomChainSteps.empty() || m_CustomChainSteps.size() > 64u)
		{
			strOutError = "Current assembled chain must contain 1..64 clips.";
			return false;
		}
		Request << R"json({
    "selectionKind": "CURRENT_CHAIN",
)json";
		if (m_bValtanPatternCreateExactSourceSelection)
		{
			if (m_iValtanPatternCreateSourceActionId <= 0 ||
				m_iValtanPatternCreateSourceSequenceIndex < 0 ||
				m_iValtanPatternCreateSourceSequenceIndex > 4096)
			{
				strOutError =
					"The staged exact Animation source identity is outside the canonical action/sequence range.";
				return false;
			}
			Request << R"json(    "sourceActionId": )json" <<
				m_iValtanPatternCreateSourceActionId << R"json(,
    "sourceSequenceIndex": )json" <<
				m_iValtanPatternCreateSourceSequenceIndex << R"json(,
)json";
		}
		Request << R"json(    "chain": {
      "chainId": ")json" << CDataJson::Escape(strChainId) << R"json(",
      "targetPatternId": "",
      "targetStageId": "",
      "animation": {
        "endPolicy": "NATIVE_CLIP_LENGTHS",
        "repeatCount": )json" << m_CustomChainSteps.size() << R"json(,
        "occurrences": [
)json";
		for (size_t iStep = 0u; iStep < m_CustomChainSteps.size(); ++iStep)
		{
			const CUSTOM_CHAIN_STEP& Step = m_CustomChainSteps[iStep];
			if (!Is_StablePatternAuthoringId(Step.clipName) ||
				!std::isfinite(Step.fDurationSeconds) ||
				Step.fDurationSeconds < 0.f ||
				Step.fDurationSeconds * 1000.0 >
					static_cast<double>((std::numeric_limits<int32_t>::max)()))
			{
				strOutError =
					"Current assembled chain contains an invalid clip or duration.";
				return false;
			}
			const int32_t iPlayMs = static_cast<int32_t>(
				std::lround(Step.fDurationSeconds * 1000.f));
			std::ostringstream OccurrenceId;
			OccurrenceId << "valtan.debug." << strChainId << ".clip." <<
				std::setw(2) << std::setfill('0') << iStep + 1u;
			if (!Is_StablePatternAuthoringId(OccurrenceId.str()))
			{
				strOutError =
					"The current chain name is too long for stable clip occurrence IDs.";
				return false;
			}
			Request << R"json(          {
            "clipOccurrenceId": ")json" <<
				CDataJson::Escape(OccurrenceId.str()) << R"json(",
            "clip": ")json" << CDataJson::Escape(Step.clipName) << R"json(",
            "mappingBasis": "PROJECT_AUTHORED",
            "sourceStartMs": 0,
            "playMs": )json" << iPlayMs << R"json(,
            "playRate": 1.0,
            "repeatUntilStageEnd": false
          })json" << (iStep + 1u == m_CustomChainSteps.size() ? "\n" : ",\n");
		}
		Request << R"json(        ]
      }
    }
  }
})json";
	}

	strOutRequest = Request.str();
	DATA_JSON_VALUE Verification;
	std::string strParseError;
	if (!CDataJson::Parse(strOutRequest, Verification, strParseError) ||
		!Verification.Is_Object())
	{
		strOutRequest.clear();
		strOutError =
			"The strict Create request could not self-parse: " + strParseError;
		return false;
	}
	return true;
}

bool_t Client::CAnimation_Tool::Start_ValtanPatternCreateCommand(
	const bool_t bApply)
{
	if (nullptr != m_hValtanPatternCreateProcess)
	{
		m_strValtanPatternCreateStatus =
			"A Create New Pattern Validate/Apply process is already running.";
		return false;
	}
	if (bApply && (nullptr == m_pBalanceTool ||
		m_pBalanceTool->Is_ValtanDraftDirty() || Is_ValtanDocumentDirty()))
	{
		m_strValtanPatternCreateStatus =
			"Apply rejected before mutation: save or discard every Balance / Valtan Animation-Sound owner draft first.";
		return false;
	}
	if (bApply && VALTAN_PATTERN_MASTER_ADMISSION_STATE::ADMITTED !=
		m_eValtanPatternMasterAdmission)
	{
		m_strValtanPatternCreateStatus =
			"Apply rejected before mutation: canonical admission is " +
			std::string(ValtanPatternMasterAdmissionLabel()) +
			"; preserved stale data is diagnostic-only.";
		return false;
	}

	std::string strRequest;
	std::string strError;
	if (!Build_ValtanPatternCreateRequest(strRequest, strError))
	{
		m_strValtanPatternCreateStatus =
			"Create New Pattern request rejected: " + strError;
		return false;
	}
	const std::string strRequestSha256 =
		CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(strRequest);
	if (!Is_LowerSha256(strRequestSha256))
	{
		m_strValtanPatternCreateStatus =
			"Create New Pattern request SHA-256 could not be computed.";
		return false;
	}
	if (bApply && strRequestSha256 !=
		m_strValtanPatternCreateValidatedRequestSha256)
	{
		m_strValtanPatternCreateValidatedRequestSha256.clear();
		m_strValtanPatternCreateStatus =
			"Apply rejected: the source or authoring fields changed after Validate. Validate the exact request again.";
		return false;
	}

	std::error_code Error;
	const std::filesystem::path ProjectRoot =
		std::filesystem::weakly_canonical(
			CProjectDataRoot::Get().parent_path(), Error);
	const std::filesystem::path Script = ProjectRoot / L"Tools" /
		L"ValtanPipeline" / L"promote_valtan_animation_chains.py";
	if (Error || ProjectRoot.empty() ||
		!std::filesystem::is_directory(ProjectRoot, Error) || Error ||
		!std::filesystem::is_regular_file(Script, Error) || Error)
	{
		m_strValtanPatternCreateStatus =
			"The fixed Create New Pattern backend could not be resolved; no process was started.";
		return false;
	}
	std::filesystem::path Python;
	if (!Resolve_PythonExecutable(Python, strError))
	{
		m_strValtanPatternCreateStatus = strError;
		return false;
	}
	const std::filesystem::path TemporaryRoot =
		std::filesystem::temp_directory_path(Error);
	if (Error || TemporaryRoot.empty())
	{
		m_strValtanPatternCreateStatus =
			"The Create New Pattern diagnostic directory could not be resolved.";
		return false;
	}
	if (0u == ++m_iValtanPatternCreateCommandSequence)
		++m_iValtanPatternCreateCommandSequence;
	const std::wstring strStem = L"LostArk.ValtanCreatePattern." +
		std::to_wstring(GetCurrentProcessId()) + L"." +
		std::to_wstring(GetTickCount64()) + L"." +
		std::to_wstring(m_iValtanPatternCreateCommandSequence) +
		(bApply ? L".Apply" : L".Validate");
	m_ValtanPatternCreateRequestPath =
		TemporaryRoot / (strStem + L".request.json");
	m_ValtanPatternCreateDiagnosticPath =
		TemporaryRoot / (strStem + L".diagnostic.log");

	{
		std::ofstream RequestFile(
			m_ValtanPatternCreateRequestPath,
			std::ios::binary | std::ios::out | std::ios::trunc);
		RequestFile.write(
			strRequest.data(), static_cast<std::streamsize>(strRequest.size()));
		RequestFile.flush();
		if (!RequestFile)
		{
			m_strValtanPatternCreateStatus =
				"Could not write the preserved Create New Pattern request file.";
			return false;
		}
	}

	SECURITY_ATTRIBUTES Security{};
	Security.nLength = sizeof(Security);
	Security.bInheritHandle = TRUE;
	const HANDLE Output = CreateFileW(
		m_ValtanPatternCreateDiagnosticPath.c_str(), GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_DELETE, &Security, CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == Output)
	{
		m_strValtanPatternCreateStatus =
			"Could not create the preserved Create diagnostic file (Win32 " +
			std::to_string(GetLastError()) + ").";
		return false;
	}
	const HANDLE Input = CreateFileW(
		L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		&Security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == Input)
	{
		const DWORD iError = GetLastError();
		CloseHandle(Output);
		m_strValtanPatternCreateStatus =
			"Could not create the Create process input handle (Win32 " +
			std::to_string(iError) + ").";
		return false;
	}

	/* User-authored fields live only in the JSON request.  The executable,
	   script, mode, repository and request paths below are fixed/canonical;
	   lpApplicationName pins python.exe and no shell interprets the command. */
	std::wstring Command = L"\"" + Python.wstring() + L"\" \"" +
		Script.wstring() + L"\" --repo-root \"" + ProjectRoot.wstring() +
		L"\" --mode " + (bApply ? L"Apply" : L"Validate") +
		L" --request-file \"" +
		m_ValtanPatternCreateRequestPath.wstring() + L"\"";
	std::vector<wchar_t> MutableCommand(Command.begin(), Command.end());
	MutableCommand.push_back(L'\0');
	STARTUPINFOW Startup{};
	Startup.cb = sizeof(Startup);
	Startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	Startup.wShowWindow = SW_HIDE;
	Startup.hStdInput = Input;
	Startup.hStdOutput = Output;
	Startup.hStdError = Output;
	PROCESS_INFORMATION Process{};
	const BOOL bCreated = CreateProcessW(
		Python.c_str(), MutableCommand.data(), nullptr, nullptr, TRUE,
		CREATE_NO_WINDOW, nullptr, ProjectRoot.c_str(), &Startup, &Process);
	const DWORD iCreateError = bCreated ? ERROR_SUCCESS : GetLastError();
	CloseHandle(Input);
	CloseHandle(Output);
	if (!bCreated)
	{
		m_strValtanPatternCreateStatus =
			"Could not start the Create New Pattern backend (Win32 " +
			std::to_string(iCreateError) + "). Request and diagnostic files were preserved.";
		return false;
	}
	CloseHandle(Process.hThread);
	m_hValtanPatternCreateProcess = Process.hProcess;
	m_strValtanPatternCreateActiveRequestSha256 = strRequestSha256;
	m_strValtanPatternCreateActivePatternId = m_ValtanPatternCreatePatternId;
	m_bValtanPatternCreateActiveApply = bApply;
	m_bValtanPatternCreateHasExitCode = false;
	m_iValtanPatternCreateExitCode = 0u;
	m_iValtanPatternCreateStartedAtMilliseconds = GetTickCount64();
	m_strValtanPatternCreateDiagnostic.clear();
	if (!bApply)
		m_strValtanPatternCreateValidatedRequestSha256.clear();
	m_strValtanPatternCreateStatus = std::string(
		bApply ? "Apply Create Pattern" : "Validate Create Request") +
		" is running. Its request and diagnostic files are preserved.";
	return true;
}

bool_t Client::CAnimation_Tool::Parse_ValtanPatternCreateResult(
	const std::string& strDiagnostic,
	std::string& strOutError) const
{
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strDiagnostic, Root, strOutError) ||
		!Root.Is_Object())
	{
		strOutError = "Create backend result is not strict JSON: " + strOutError;
		return false;
	}
	constexpr std::array<std::string_view, 11u> RequiredKeys = {
		"schema", "formatVersion", "mode", "patternId", "sourceChainId",
		"admissionState", "selectionMode", "sourceSha256", "patternCount",
		"stageCount", "projectedArtifactCount" };
	if (Root.Get_Object().size() != RequiredKeys.size() ||
		!std::all_of(RequiredKeys.begin(), RequiredKeys.end(),
			[&Root](const std::string_view strKey)
			{
				return Root.Get_Object().contains(std::string(strKey));
			}))
	{
		strOutError = "Create backend result properties do not match v1.";
		return false;
	}
	const auto String = [&Root](const std::string_view strKey) ->
		const DATA_JSON_VALUE*
	{
		const DATA_JSON_VALUE* const pValue = Root.Find(strKey);
		return nullptr != pValue && pValue->Is_String() ? pValue : nullptr;
	};
	const auto UnsignedInteger = [&Root](
		const std::string_view strKey, uint64_t& iOut) -> bool_t
	{
		const DATA_JSON_VALUE* const pValue = Root.Find(strKey);
		if (nullptr == pValue || !pValue->Is_Number() ||
			pValue->Was_FloatingPointToken() ||
			!std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() >
				static_cast<double>((std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
		iOut = static_cast<uint64_t>(pValue->Get_Number());
		return static_cast<double>(iOut) == pValue->Get_Number();
	};
	const DATA_JSON_VALUE* const pSchema = String("schema");
	const DATA_JSON_VALUE* const pMode = String("mode");
	const DATA_JSON_VALUE* const pPatternId = String("patternId");
	const DATA_JSON_VALUE* const pSourceChainId = String("sourceChainId");
	const DATA_JSON_VALUE* const pAdmission = String("admissionState");
	const DATA_JSON_VALUE* const pSelection = String("selectionMode");
	const DATA_JSON_VALUE* const pSourceSha = String("sourceSha256");
	uint64_t iVersion = 0u;
	uint64_t iPatternCount = 0u;
	uint64_t iStageCount = 0u;
	uint64_t iProjectedCount = 0u;
	if (nullptr == pSchema || nullptr == pMode || nullptr == pPatternId ||
		nullptr == pSourceChainId || nullptr == pAdmission ||
		nullptr == pSelection || nullptr == pSourceSha ||
		!UnsignedInteger("formatVersion", iVersion) || 1u != iVersion ||
		!UnsignedInteger("patternCount", iPatternCount) ||
		!UnsignedInteger("stageCount", iStageCount) ||
		!UnsignedInteger("projectedArtifactCount", iProjectedCount) ||
		0u == iPatternCount || 0u == iStageCount || 0u == iProjectedCount ||
		pSchema->Get_String() !=
			"lostark.valtan-animation-pattern-create-result" ||
		pMode->Get_String() !=
			(m_bValtanPatternCreateActiveApply ? "Apply" : "Validate") ||
		pPatternId->Get_String() != m_strValtanPatternCreateActivePatternId ||
		!Is_StablePatternAuthoringId(pSourceChainId->Get_String()) ||
		pAdmission->Get_String() != "MANUAL_SERVER_AUDITION" ||
		pSelection->Get_String() != "AUDITION_ONLY" ||
		!Is_LowerSha256(pSourceSha->Get_String()))
	{
		strOutError =
			"Create backend result identity, mode or safe-default contract is invalid.";
		return false;
	}
	return true;
}

void Client::CAnimation_Tool::Poll_ValtanPatternCreateCommand()
{
	if (nullptr == m_hValtanPatternCreateProcess)
		return;
	const HANDLE Process = static_cast<HANDLE>(m_hValtanPatternCreateProcess);
	const DWORD iWait = WaitForSingleObject(Process, 0u);
	if (WAIT_TIMEOUT == iWait)
	{
		if (GetTickCount64() - m_iValtanPatternCreateStartedAtMilliseconds >=
			VALTAN_PATTERN_CREATE_TIMEOUT_MILLISECONDS &&
			std::string::npos == m_strValtanPatternCreateStatus.find(
				"still running after"))
		{
			m_strValtanPatternCreateStatus =
				"Create New Pattern is still running after 120 seconds. It was not terminated because the all-or-nothing transaction must finish or roll back; request and diagnostic files remain available.";
		}
		return;
	}
	if (WAIT_OBJECT_0 != iWait)
	{
		m_strValtanPatternCreateStatus =
			"Create New Pattern process observation failed (Win32 " +
			std::to_string(GetLastError()) +
			"). The child was not terminated and its files remain preserved.";
		return;
	}

	DWORD iExitCode = 1u;
	const bool_t bExitKnown = FALSE != GetExitCodeProcess(Process, &iExitCode);
	CloseHandle(Process);
	m_hValtanPatternCreateProcess = nullptr;
	m_bValtanPatternCreateHasExitCode = bExitKnown;
	m_iValtanPatternCreateExitCode = iExitCode;
	std::string strReadError;
	if (!Read_BoundedFile(
			m_ValtanPatternCreateDiagnosticPath,
			VALTAN_PATTERN_CREATE_MAX_DIAGNOSTIC_BYTES,
			m_strValtanPatternCreateDiagnostic, strReadError))
	{
		m_strValtanPatternCreateStatus =
			"Create New Pattern exited, but its bounded diagnostic could not be read: " +
			strReadError + ". Files remain preserved.";
		return;
	}
	if (!bExitKnown)
	{
		m_strValtanPatternCreateStatus =
			"Create New Pattern exited without a readable exit code. No success is assumed; inspect the preserved diagnostic.";
		return;
	}
	if (0u != iExitCode)
	{
		if (!m_bValtanPatternCreateActiveApply)
			m_strValtanPatternCreateValidatedRequestSha256.clear();
		m_strValtanPatternCreateStatus = std::string(
			m_bValtanPatternCreateActiveApply ?
				"Apply Create Pattern" : "Validate Create Request") +
			" failed with exit code " + std::to_string(iExitCode) +
			". No success is assumed; inspect the preserved diagnostic.";
		return;
	}

	std::string strParseError;
	if (!Parse_ValtanPatternCreateResult(
			m_strValtanPatternCreateDiagnostic, strParseError))
	{
		if (!m_bValtanPatternCreateActiveApply)
			m_strValtanPatternCreateValidatedRequestSha256.clear();
		m_strValtanPatternCreateStatus =
			"Create New Pattern returned exit code 0 but its typed result was rejected: " +
			strParseError + ". No success is assumed.";
		return;
	}
	if (!m_bValtanPatternCreateActiveApply)
	{
		m_strValtanPatternCreateValidatedRequestSha256 =
			m_strValtanPatternCreateActiveRequestSha256;
		m_strValtanPatternCreateStatus =
			"Create New Pattern Validate passed. Apply is unlocked only for this exact request SHA-256.";
		return;
	}

	m_strValtanPatternCreateValidatedRequestSha256.clear();
	std::string strBalanceReloadStatus =
		"Balance source reload is unavailable.";
	const bool_t bBalanceReloaded = nullptr != m_pBalanceTool &&
		m_pBalanceTool->Reload_ValtanSource(strBalanceReloadStatus);
	m_bCustomChainLibraryLoadAttempted = true;
	const bool_t bIntakeReloaded = bBalanceReloaded && Load_CustomChainLibrary();
	const std::string strIntakeStatus = bBalanceReloaded ?
		m_strCustomChainStatus :
		"Intake reload was not attempted because joined source/Product admission failed.";
	const bool_t bAnimationReloaded = bBalanceReloaded &&
		Reload_ValtanPatternMaster();
	const std::string strAnimationStatus = bBalanceReloaded ?
		m_strValtanPatternMasterStatus :
		"Anim joined-master reload was not attempted because joined source/Product admission failed.";
	std::string strBossStatus =
		bAnimationReloaded ? "Boss canonical graph reload is unavailable." :
			"Boss canonical graph reload was not attempted because Anim reload failed.";
	const bool_t bBossReloaded = bAnimationReloaded &&
		nullptr != m_pBossTool &&
		m_pBossTool->Reload_CanonicalGraph(strBossStatus);
	const bool_t bAnimationAdmitted = bAnimationReloaded &&
		VALTAN_PATTERN_MASTER_ADMISSION_STATE::ADMITTED ==
			m_eValtanPatternMasterAdmission;
	bool_t bSelected = false;
	if (bAnimationAdmitted)
	{
		const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
			Collect_ValtanPatternMasterPatterns();
		for (size_t iPattern = 0u; iPattern < Patterns.size(); ++iPattern)
		{
			const VALTAN_PATTERN_VIEW* const pPattern = Patterns[iPattern];
			if (nullptr == pPattern || pPattern->strPatternId !=
				m_strValtanPatternCreateActivePatternId)
			{
				continue;
			}
			m_iValtanPatternMasterSelected = static_cast<int32_t>(iPattern);
			m_strValtanWorkbenchPatternId = pPattern->strPatternId;
			m_strValtanWorkbenchStageId = pPattern->Stages.empty() ?
				std::string{} : pPattern->Stages.front().strStageId;
			m_eValtanWorkbenchSelection =
				VALTAN_WORKBENCH_SELECTION_KIND::PATTERN;
			m_eValtanWorkbenchDetailOwner =
				VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION;
			m_bValtanWorkbenchFocusDetailRequested = true;
			bSelected = true;
			break;
		}
	}
#ifdef _DEBUG
	if (bSelected)
	{
		if (CMainApp* const pApp = CMainApp::Get_Active())
			(void)pApp->Debug_SelectCompletePlayPattern(
				m_strValtanPatternCreateActivePatternId);
	}
#endif
	const bool_t bReloadClosureAdmitted =
		bBalanceReloaded && bIntakeReloaded &&
		bAnimationAdmitted && bBossReloaded && bSelected;
	if (bReloadClosureAdmitted)
	{
		m_strValtanCompositionPatternCreatedId =
			m_strValtanPatternCreateActivePatternId;
		m_bValtanCompositionPatternCreatedPending = true;
	}
	m_strValtanPatternCreateStatus =
		std::string(bReloadClosureAdmitted ?
			"Apply completed source -> Product -> canonical reload closure for " :
			"Apply source transaction committed, but Product/canonical reload closure is INCOMPLETE for ") +
		m_strValtanPatternCreateActivePatternId + ". Balance source reload: " +
		(bBalanceReloaded ? "PASS" : "REJECTED") + " (" +
		strBalanceReloadStatus +
		"). Apply transaction source/Product closure: PASS (validated before exit 0). Intake reload: " +
		(bIntakeReloaded ? "PASS" : "REJECTED") + " (" + strIntakeStatus +
		"). Anim joined-master admission/selection: " +
		(bAnimationAdmitted && bSelected ? "PASS" : "REJECTED") + " (" +
		strAnimationStatus + "). Boss canonical graph/inventory reload: " +
		(bBossReloaded ? "PASS" : "REJECTED") + " (" + strBossStatus + ").";
}

void Client::CAnimation_Tool::Render_ValtanCustomChainWindow(
	const shared_ptr<Engine::CModel>& pModel)
{
	const CUSTOM_CHAIN_PROFILE* pProfile =
		Find_CustomChainProfile(m_AssetName);
	if (nullptr == pProfile)
	{
		m_bShowValtanCustomChainWindow = false;
		return;
	}

	if (!ImGui::Begin(
		pProfile->pWindowTitle, &m_bShowValtanCustomChainWindow))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"Assemble a chain by hand out of the model's own clips. Seconds carry "
		"the same meaning the source cuts do: 0 plays the clip's native length, "
		"a shorter value cuts it, a longer one loops it until the step is "
		"filled. Save Animation Intake writes the review source only; it does "
		"not silently replace Server gameplay or Product presentation.");

	const bool_t bBusy =
		m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying ||
		nullptr != m_hValtanPatternCreateProcess;

	ImGui::BeginDisabled(bBusy || m_CustomChainSteps.empty());
	if (ImGui::Button("Play Chain"))
	{
		/* The transport that owns step, progress, pause and speed already
		   lives in the source reference window, so playback opens it. */
		if (Start_ValtanCustomChainPreview(pModel))
			m_bShowValtanSourceReferenceWindow = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Steps"))
	{
		m_CustomChainSteps.clear();
		Invalidate_ValtanPatternCreateExactSourceSelection();
	}
	ImGui::EndDisabled();

	ImGui::SetNextItemWidth(160.f);
	ImGui::SliderFloat(
		"Blend##customchainblend", &m_fPreviewBlendSeconds, 0.f, 0.5f, "%.3f s");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(
			"Preview-only cross-fade when a step changes clip. 0 matches the "
			"product Valtan, which does not blend yet; 0.12 matches the value "
			"CCharacter already uses.");
	}

	if (!m_bCustomChainLibraryLoadAttempted)
	{
		m_bCustomChainLibraryLoadAttempted = true;
		Load_CustomChainLibrary();
	}

	ImGui::SeparatorText("Saved Animation Intake");
	ImGui::TextDisabled("%s", pProfile->pFileLabel);
	ImGui::BeginDisabled(bBusy);
	ImGui::SetNextItemWidth(160.f);
	if (ImGui::InputTextWithHint("##chainid", "chain name",
		m_CustomChainId, sizeof(m_CustomChainId)))
	{
		Invalidate_ValtanPatternCreateExactSourceSelection();
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(160.f);
	ImGui::InputTextWithHint("##chaintargetpattern", "target patternId",
		m_CustomChainTargetPatternId, sizeof(m_CustomChainTargetPatternId));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::InputTextWithHint("##chaintargetstage", "target stageId",
		m_CustomChainTargetStageId, sizeof(m_CustomChainTargetStageId));

	if (ImGui::Button("Save Animation Intake"))
	{
		if ('\0' == m_CustomChainId[0])
		{
			m_strCustomChainStatus = "Save rejected: name the chain first.";
		}
		else if (m_CustomChainSteps.empty())
		{
			m_strCustomChainStatus = "Save rejected: the chain owns no step.";
		}
		else
		{
			CUSTOM_CHAIN_ENTRY Entry;
			Entry.chainId = m_CustomChainId;
			Entry.targetPatternId = m_CustomChainTargetPatternId;
			Entry.targetStageId = m_CustomChainTargetStageId;
			Entry.steps = m_CustomChainSteps;
			/* Same name replaces in place so re-saving a chain being tuned
			   does not grow a pile of near-identical entries. */
			const std::vector<CUSTOM_CHAIN_ENTRY> Previous =
				m_CustomChainLibrary;
			bool_t bReplaced = false;
			for (CUSTOM_CHAIN_ENTRY& Existing : m_CustomChainLibrary)
			{
				if (Existing.chainId != Entry.chainId)
					continue;
				Existing = Entry;
				bReplaced = true;
				break;
			}
			if (!bReplaced)
				m_CustomChainLibrary.push_back(std::move(Entry));
			if (!Save_CustomChainLibrary())
				m_CustomChainLibrary = Previous;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload File"))
	{
		Load_CustomChainLibrary();
	}
	ImGui::EndDisabled();

	if (!m_strCustomChainStatus.empty())
		ImGui::TextWrapped("%s", m_strCustomChainStatus.c_str());

	if (!m_CustomChainLibrary.empty())
	{
		size_t iDeleteEntry = m_CustomChainLibrary.size();
		ImGui::BeginDisabled(bBusy);
		for (size_t iEntry = 0u; iEntry < m_CustomChainLibrary.size(); ++iEntry)
		{
			const CUSTOM_CHAIN_ENTRY& Entry = m_CustomChainLibrary[iEntry];
			ImGui::PushID(static_cast<int32_t>(iEntry) + 20000);
			if (ImGui::SmallButton("Load"))
			{
				m_CustomChainSteps = Entry.steps;
				Invalidate_ValtanPatternCreateExactSourceSelection();
				snprintf(m_CustomChainId, sizeof(m_CustomChainId), "%s",
					Entry.chainId.c_str());
				snprintf(m_CustomChainTargetPatternId,
					sizeof(m_CustomChainTargetPatternId), "%s",
					Entry.targetPatternId.c_str());
				snprintf(m_CustomChainTargetStageId,
					sizeof(m_CustomChainTargetStageId), "%s",
					Entry.targetStageId.c_str());
				m_strCustomChainStatus =
					"Loaded chain " + Entry.chainId + " into the steps.";
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Delete"))
				iDeleteEntry = iEntry;
			ImGui::SameLine();
			ImGui::Text("%s  |  %s / %s  |  %zu steps",
				Entry.chainId.c_str(),
				Entry.targetPatternId.empty() ? "-" :
					Entry.targetPatternId.c_str(),
				Entry.targetStageId.empty() ? "-" :
					Entry.targetStageId.c_str(),
				Entry.steps.size());
			ImGui::PopID();
		}
		ImGui::EndDisabled();
		if (iDeleteEntry < m_CustomChainLibrary.size())
		{
			const std::vector<CUSTOM_CHAIN_ENTRY> Previous =
				m_CustomChainLibrary;
			m_CustomChainLibrary.erase(
				m_CustomChainLibrary.begin() +
				static_cast<std::ptrdiff_t>(iDeleteEntry));
			if (!Save_CustomChainLibrary())
				m_CustomChainLibrary = Previous;
		}
	}

	Render_ValtanPatternCreatePanel();

	ImGui::SeparatorText("Steps");
	if (m_CustomChainSteps.empty())
	{
		ImGui::TextDisabled("No step yet. Add clips from the list below.");
	}
	else
	{
		f32_t fTotalSeconds = 0.f;
		size_t iRemove = m_CustomChainSteps.size();
		size_t iMoveUp = m_CustomChainSteps.size();
		size_t iMoveDown = m_CustomChainSteps.size();
		ImGui::BeginDisabled(bBusy);
		for (size_t iStep = 0u; iStep < m_CustomChainSteps.size(); ++iStep)
		{
			CUSTOM_CHAIN_STEP& Step = m_CustomChainSteps[iStep];
			ImGui::PushID(static_cast<int32_t>(iStep));
			ImGui::Text("%2zu", iStep + 1u);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80.f);
			if (ImGui::InputFloat(
					"##seconds", &Step.fDurationSeconds, 0.f, 0.f, "%.3f"))
			{
				Invalidate_ValtanPatternCreateExactSourceSelection();
			}
			if (!std::isfinite(Step.fDurationSeconds) ||
				Step.fDurationSeconds < 0.f)
			{
				Step.fDurationSeconds = 0.f;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("^"))
				iMoveUp = iStep;
			ImGui::SameLine();
			if (ImGui::SmallButton("v"))
				iMoveDown = iStep;
			ImGui::SameLine();
			if (ImGui::SmallButton("x"))
				iRemove = iStep;
			ImGui::SameLine();
			ImGui::TextUnformatted(Step.clipName.c_str());
			fTotalSeconds += Step.fDurationSeconds;
			ImGui::PopID();
		}
		ImGui::EndDisabled();
		ImGui::Text(
			"%zu steps, %.3f s of authored length (steps left at 0 add their "
			"native length on top).",
			m_CustomChainSteps.size(), fTotalSeconds);

		/* The list is edited after the row loop so the vector never moves while
		   ImGui is still drawing from it. */
		if (iRemove < m_CustomChainSteps.size())
		{
			m_CustomChainSteps.erase(
				m_CustomChainSteps.begin() +
					static_cast<std::ptrdiff_t>(iRemove));
			Invalidate_ValtanPatternCreateExactSourceSelection();
		}
		else if (iMoveUp > 0u && iMoveUp < m_CustomChainSteps.size())
		{
			std::swap(
				m_CustomChainSteps[iMoveUp - 1u], m_CustomChainSteps[iMoveUp]);
			Invalidate_ValtanPatternCreateExactSourceSelection();
		}
		else if (iMoveDown + 1u < m_CustomChainSteps.size())
		{
			std::swap(
				m_CustomChainSteps[iMoveDown], m_CustomChainSteps[iMoveDown + 1u]);
			Invalidate_ValtanPatternCreateExactSourceSelection();
		}
	}

	ImGui::SeparatorText("Clips");
	ImGui::TextDisabled(
		"Only %s* clips are listed: both Valtan bodies share that vocabulary, "
		"so a chain authored here plays on either.",
		CUSTOM_CHAIN_CLIP_PREFIX);
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##customchainfilter",
		"filter by clip name",
		m_CustomChainFilter,
		sizeof(m_CustomChainFilter));

	ImGui::BeginDisabled(bBusy);
	if (ImGui::BeginChild(
		"##customchainclips",
		ImVec2(0.f, 260.f),
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		for (uint32_t iAnimation = 0u;
			iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
		{
			const char_t* pName = pModel->Get_AnimationName(iAnimation);
			if (nullptr == pName ||
				!Contains_NoCase(pName, m_CustomChainFilter))
			{
				continue;
			}
			/* The ghost body carries its own rpbf_02.ao_* clips on top of the
			   shared vocabulary. They play, but the product body has no clip of
			   that name, so authoring one into this shared document would break
			   the promotion that pins the product model. */
			if (0 != std::strncmp(
				pName,
				CUSTOM_CHAIN_CLIP_PREFIX,
				std::strlen(CUSTOM_CHAIN_CLIP_PREFIX)))
			{
				continue;
			}
			ImGui::PushID(static_cast<int32_t>(iAnimation));
			if (ImGui::SmallButton("+"))
			{
				CUSTOM_CHAIN_STEP Step;
				Step.clipName = pName;
				m_CustomChainSteps.push_back(std::move(Step));
				Invalidate_ValtanPatternCreateExactSourceSelection();
			}
			ImGui::SameLine();
			ImGui::TextUnformatted(pName);
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::EndDisabled();

	ImGui::End();
}

bool_t Client::CAnimation_Tool::Start_ValtanCustomChainPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		m_strValtanPatternPreviewStatus =
			"Custom chain start rejected because the animation target changed.";
		return false;
	}
	if (m_CustomChainSteps.empty())
	{
		m_strValtanPatternPreviewStatus =
			"Custom chain start rejected because it owns no step.";
		return false;
	}

	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> staged;
	staged.reserve(m_CustomChainSteps.size());
	const uint32_t iStepCount =
		static_cast<uint32_t>(m_CustomChainSteps.size());
	for (uint32_t iStep = 0u; iStep < iStepCount; ++iStep)
	{
		const CUSTOM_CHAIN_STEP& Step = m_CustomChainSteps[iStep];
		VALTAN_PATTERN_PREVIEW_PLAY_ITEM item;
		item.strPatternLabel = "Custom chain";
		item.iSequenceIndex = -1;
		item.iSequenceRepeatNumber = 1u;
		item.iSequenceRepeatCount = 1u;
		item.iSourceStepNumber = iStep + 1u;
		item.iSourceStepCount = iStepCount;
		item.strSequenceName = "Custom chain";
		item.strSequenceMode = "CUSTOM";
		item.strClipName = Step.clipName;
		item.iStepNumber = iStep + 1u;
		item.iStepCount = iStepCount;
		/* A non-positive or non-finite entry means the animator has not chosen
		   a length yet, which the playlist already reads as the native one. */
		item.fAuthoredDurationSeconds =
			std::isfinite(Step.fDurationSeconds) && Step.fDurationSeconds > 0.f ?
				Step.fDurationSeconds : 0.f;
		staged.push_back(std::move(item));
	}

	if (m_bValtanPatternMasterPlaying)
	{
		Reset_ValtanPatternMasterPreviewState(
			"Valtan Pattern Master yielded to the custom chain.");
	}

	m_ValtanPatternPreviewPlaylist = std::move(staged);
	m_iValtanPatternPreviewItem = 0u;
	m_bValtanPatternPreviewPlaying = true;
	m_bValtanPatternPreviewPaused = false;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_ValtanPatternPreviewModel = pModel;
	m_iValtanPatternPreviewTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iValtanSequenceSelected = -1;
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	m_strValtanPatternPreviewStatus = "Playing the custom chain: " +
		std::to_string(iStepCount) + " steps.";
	return Activate_ValtanPatternPreviewItem(pModel);
}

bool_t Client::CAnimation_Tool::Start_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const uint32_t iFirstPattern,
	const uint32_t iLastPattern)
{
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		m_strValtanPatternPreviewStatus =
			"Pattern preview start rejected because the animation target changed.";
		return false;
	}
	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> staged;
	std::string status;
	if (!CValtanPatternPreviewDocument::Build_Playlist(
			m_ValtanPatternPreviewDocument,
			iFirstPattern,
			iLastPattern,
			staged,
			status))
	{
		m_strValtanPatternPreviewStatus =
			"Pattern preview start rejected; current pose preserved: " + status;
		return false;
	}
	if (m_bValtanPatternMasterPlaying)
	{
		Reset_ValtanPatternMasterPreviewState(
			"Valtan Pattern Master yielded to the read-only source reference.");
	}

	m_ValtanPatternPreviewPlaylist = std::move(staged);
	m_iValtanPatternPreviewItem = 0u;
	m_bValtanPatternPreviewPlaying = true;
	m_bValtanPatternPreviewPaused = false;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_ValtanPatternPreviewModel = pModel;
	m_iValtanPatternPreviewTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iValtanSequenceSelected = -1;
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	m_strValtanPatternPreviewStatus = status;
	return Activate_ValtanPatternPreviewItem(pModel);
}

bool_t Client::CAnimation_Tool::Activate_ValtanPatternPreviewItem(
	const shared_ptr<Engine::CModel>& pModel)
{
	constexpr f32_t PATTERN_MARKER_DURATION_SECONDS = 0.45f;
	std::string skippedStatus;
	const auto RecordSkip = [&skippedStatus](const std::string& reason)
	{
		if (!skippedStatus.empty())
			skippedStatus += " ";
		skippedStatus += reason;
	};

	while (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size())
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		m_fValtanPatternPreviewElapsedSeconds = 0.f;
		m_fValtanPatternPreviewItemDurationSeconds = 0.f;
		m_bValtanPatternPreviewPaused = false;

		if (Item.bPatternMarker)
		{
			if (!pModel->Start_Animation("mesh_idle_battle_1", true))
			{
				RecordSkip(
					"Skipped pattern marker because the idle clip is unavailable.");
				++m_iValtanPatternPreviewItem;
				continue;
			}

			m_bLoop = true;
			m_fValtanPatternPreviewItemDurationSeconds =
				PATTERN_MARKER_DURATION_SECONDS;
			pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
			m_strValtanPatternPreviewStatus = skippedStatus;
			if (!m_strValtanPatternPreviewStatus.empty())
				m_strValtanPatternPreviewStatus += " ";
			m_strValtanPatternPreviewStatus +=
				"Pattern " + std::to_string(Item.iPatternNumber) +
				" step " + std::to_string(Item.iStepNumber) + "/" +
				std::to_string(Item.iStepCount) +
				" marker started for 450 ms.";
			return true;
		}

		if (!Start_PreviewClip(
			pModel, Item.strClipName.c_str(), false, m_fPreviewBlendSeconds))
		{
			RecordSkip(
				"Skipped unavailable clip without stopping Play All: " +
				Item.strClipName + ".");
			++m_iValtanPatternPreviewItem;
			continue;
		}

		const uint32_t AnimationIndex = pModel->Get_CurrentAnimIndex();
		f32_t Position = 0.f;
		f32_t DurationTicks = 0.f;
		const f32_t TickRate =
			pModel->Get_AnimationTickPerSecond(AnimationIndex);
		if (!std::isfinite(TickRate) || TickRate <= 0.f ||
			!pModel->Get_AnimationProgress(
				AnimationIndex, Position, DurationTicks) ||
			!std::isfinite(DurationTicks) || DurationTicks <= 0.f)
		{
			RecordSkip(
				"Skipped clip with invalid native duration without stopping Play All: " +
				Item.strClipName + ".");
			++m_iValtanPatternPreviewItem;
			continue;
		}

		const f32_t NativeDurationSeconds = DurationTicks / TickRate;
		if (!std::isfinite(NativeDurationSeconds) ||
			NativeDurationSeconds <= 0.f)
		{
			RecordSkip(
				"Skipped clip with invalid native duration without stopping Play All: " +
				Item.strClipName + ".");
			++m_iValtanPatternPreviewItem;
			continue;
		}

		f32_t fItemDurationSeconds = NativeDurationSeconds;
		bool_t bLoopClip = false;
		std::string durationNote = " started for its full native duration (";
		if (Item.fAuthoredDurationSeconds > 0.f)
		{
			fItemDurationSeconds = Item.fAuthoredDurationSeconds;
			bLoopClip =
				Item.fAuthoredDurationSeconds > NativeDurationSeconds + 0.001f;
			durationNote = bLoopClip ?
				" started looping for its source stage length (" :
				" started for its source stage length (";
			if (bLoopClip &&
				!Start_PreviewClip(
					pModel, Item.strClipName.c_str(), true,
					m_fPreviewBlendSeconds))
			{
				RecordSkip(
					"Skipped unavailable clip without stopping Play All: " +
					Item.strClipName + ".");
				++m_iValtanPatternPreviewItem;
				continue;
			}
		}

		m_bLoop = bLoopClip;
		m_fValtanPatternPreviewItemDurationSeconds = fItemDurationSeconds;
		pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
		m_strValtanPatternPreviewStatus = skippedStatus;
		if (!m_strValtanPatternPreviewStatus.empty())
			m_strValtanPatternPreviewStatus += " ";
		m_strValtanPatternPreviewStatus +=
			"Pattern " + std::to_string(Item.iPatternNumber) +
			" step " + std::to_string(Item.iStepNumber) + "/" +
			std::to_string(Item.iStepCount) +
			durationNote +
			std::to_string(fItemDurationSeconds) + " s).";
		return true;
	}

	if (!skippedStatus.empty())
		skippedStatus += " ";
	Stop_ValtanPatternPreview(
		pModel, skippedStatus + "Pattern preview completed; idle restored.");
	return false;
}

void Client::CAnimation_Tool::Advance_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (!m_bValtanPatternPreviewPlaying)
		return;
	const bool_t bHadItem =
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size();
	const VALTAN_PATTERN_PREVIEW_PLAY_ITEM Finished = bHadItem ?
		m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem] :
		VALTAN_PATTERN_PREVIEW_PLAY_ITEM{};
	const f32_t fFinishedDurationSeconds =
		m_fValtanPatternPreviewItemDurationSeconds;
	++m_iValtanPatternPreviewItem;
	if (m_iValtanPatternPreviewItem >= m_ValtanPatternPreviewPlaylist.size())
	{
		Stop_ValtanPatternPreview(
			pModel, "Pattern preview completed; idle restored.");
		return;
	}
	Activate_ValtanPatternPreviewItem(pModel);
	/* The pattern hit clock keeps counting only while the next started item
	   continues the same source sequence pass; anything else restarts it. */
	if (!m_bValtanPatternPreviewPlaying ||
		m_iValtanPatternPreviewItem >= m_ValtanPatternPreviewPlaylist.size())
	{
		m_fValtanPatternHitTimelineBaseSeconds = 0.f;
		return;
	}
	const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Current =
		m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
	const bool_t bSameTimeline = bHadItem &&
		!Finished.bPatternMarker && !Current.bPatternMarker &&
		Finished.iPatternNumber == Current.iPatternNumber &&
		Finished.iSourceActionId == Current.iSourceActionId &&
		Finished.iSequenceIndex == Current.iSequenceIndex &&
		Finished.iSequenceRepeatNumber == Current.iSequenceRepeatNumber;
	if (bSameTimeline)
		m_fValtanPatternHitTimelineBaseSeconds += fFinishedDurationSeconds;
	else
		m_fValtanPatternHitTimelineBaseSeconds = 0.f;
}

void Client::CAnimation_Tool::Stop_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const std::string& status)
{
	if (nullptr != pModel)
	{
		m_bLoop = true;
		pModel->Set_AnimationSpeed(1.f);
		/* Each authoring body names its own idle; an unknown target keeps the
		   product clip so the existing Valtan paths are unchanged. */
		const CUSTOM_CHAIN_PROFILE* pProfile =
			Find_CustomChainProfile(m_AssetName);
		const char_t* pIdleClip = nullptr != pProfile ?
			pProfile->pIdleClip : "mesh_idle_battle_1";
		if (!pModel->Start_Animation(pIdleClip, true))
			pModel->Set_AnimPaused(true);
	}
	Reset_ValtanPatternPreviewState(status);
}

void Client::CAnimation_Tool::Reset_ValtanPatternPreviewState(
	const std::string& status)
{
	m_bValtanPatternPreviewPlaying = false;
	m_bValtanPatternPreviewPaused = false;
	m_iValtanPatternPreviewItem = 0u;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_fValtanPatternPreviewItemDurationSeconds = 0.f;
	m_ValtanPatternPreviewPlaylist.clear();
	m_strValtanPatternPreviewStatus = status;
	m_bLoop = true;
	m_ValtanPatternPreviewModel.reset();
	m_iValtanPatternPreviewTargetGeneration = 0u;
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
#ifdef _DEBUG
	if (const shared_ptr<CValtan> Boss =
		CAnimationTargetService::Resolve_Boss())
	{
		Boss->Clear_PatternHitAreaPreview();
	}
#endif
}

void Client::CAnimation_Tool::Update_ValtanPatternHitAreaPreview()
{
#ifdef _DEBUG
	const shared_ptr<CValtan> Boss = CAnimationTargetService::Resolve_Boss();
	if (nullptr == Boss)
		return;
	if (!m_bValtanPatternPreviewPlaying ||
		m_iValtanPatternPreviewItem >= m_ValtanPatternPreviewPlaylist.size())
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}
	const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
		m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
	if (Item.bPatternMarker || 0u == Item.iSourceActionId)
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}

	if (!m_bValtanEncounterReferenceLoadAttempted)
	{
		m_bValtanEncounterReferenceLoadAttempted = true;
		std::string status;
		if (!m_ValtanEncounterReference.Load(CProjectDataRoot::Resolve(
				std::filesystem::path(L"Encounters") / L"Valtan" /
				L"ValtanEncounter.json"), status))
		{
			m_ValtanEncounterReference.Clear();
		}
	}
	if (!m_ValtanEncounterReference.Is_Ready())
		return;

	const ENCOUNTER_PATTERN_REFERENCE* pPattern = nullptr;
	for (const ENCOUNTER_PATTERN_REFERENCE& pattern :
		m_ValtanEncounterReference.Get_Patterns())
	{
		if (std::find(pattern.sourceActionIds.begin(),
				pattern.sourceActionIds.end(),
				Item.iSourceActionId) != pattern.sourceActionIds.end())
		{
			pPattern = &pattern;
			break;
		}
	}
	if (nullptr == pPattern)
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}

	/* The stage lookup runs on the whole-pattern clock: seconds of finished
	   same-sequence items plus the current item's local elapsed. */
	const f32_t fTimelineMs = (m_fValtanPatternHitTimelineBaseSeconds +
		m_fValtanPatternPreviewElapsedSeconds) * 1000.f;
	const ENCOUNTER_STAGE_REFERENCE* pStage = nullptr;
	for (const ENCOUNTER_STAGE_REFERENCE& stage : pPattern->stages)
	{
		const f32_t fStartMs = static_cast<f32_t>(stage.iStartOffsetMs);
		if (fTimelineMs >= fStartMs &&
			fTimelineMs < fStartMs + static_cast<f32_t>(stage.iDurationMs))
		{
			pStage = &stage;
			break;
		}
	}
	if (nullptr == pStage)
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}
	Boss->Set_PatternHitAreaPreview(
		pStage->actionId,
		(fTimelineMs - static_cast<f32_t>(pStage->iStartOffsetMs)) / 1000.f);
#endif
}

/* The chains this clip belongs to. A skill plays its clips in a fixed order and
each tripod build takes a different route, so the same clip shows up in several.
Clicking a step both navigates and tells the reference panel where that step
starts on the whole-cast clock. */
void Client::CAnimation_Tool::Render_ClipChain(const shared_ptr<Engine::CModel>& pModel)
{
	if (m_ClipSeqs.empty())
		return;

	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	if (nullptr == pCurrentName)
		return;

	int32_t iMatches = 0;
	for (size_t iSeq = 0u; iSeq < m_ClipSeqs.size(); ++iSeq)
	{
		const CLIP_SEQ& seq = m_ClipSeqs[iSeq];
		int32_t iPos = -1;
		for (int32_t i = 0; i < static_cast<int32_t>(seq.clips.size()); ++i)
		{
			if (seq.clips[i] == pCurrentName)
			{
				iPos = i;
				break;
			}
		}
		if (iPos < 0)
			continue;

		if (0 == iMatches)
			ImGui::SeparatorText("Chain");
		++iMatches;

		ImGui::PushID(seq.iSkillId * 100 + seq.iSeqIndex);

		const int32_t iOffsetMs = Get_ChainOffsetMs(seq, iPos);
		/* The numbered buttons below select one clip. Pressing the chain itself
		   runs every step in authored order, which is the only way to judge how
		   the sequence reads as one motion. The transport lives in the source
		   reference window, so opening it is part of starting playback. */
		char_t szChain[192]{};
		snprintf(szChain, sizeof(szChain), "%s  seq%d  [%s]  %d/%d",
			seq.name.c_str(), seq.iSeqIndex,
			seq.sMode.empty() ? "?" : seq.sMode.c_str(),
			iPos + 1, static_cast<int32_t>(seq.clips.size()));
		if (ImGui::Button(szChain))
		{
			if (Start_ValtanSequencePreview(pModel, iSeq))
				m_bShowValtanSourceReferenceWindow = true;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Play all %d clips in order.",
				static_cast<int32_t>(seq.clips.size()));

		ImGui::SameLine();
		char_t szUse[64]{};
		snprintf(szUse, sizeof(szUse), "use offset %d ms", iOffsetMs);
		if (ImGui::SmallButton(szUse))
		{
			m_iCastOffsetMs = iOffsetMs;
			m_Status = "Cast offset set to " + std::to_string(iOffsetMs) +
				" ms from " + seq.name + " seq" + std::to_string(seq.iSeqIndex);
		}

		for (int32_t i = 0; i < static_cast<int32_t>(seq.clips.size()); ++i)
		{
			if (i > 0)
				ImGui::SameLine(0.f, 2.f);

			ImGui::PushID(i);
			const bool_t bHere = (i == iPos);
			if (bHere)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.f, 0.4f, 1.f));

			char_t szStep[16]{};
			snprintf(szStep, sizeof(szStep), "%d", i + 1);
			if (ImGui::SmallButton(szStep))
				Select_Clip(pModel, seq.clips[i]);

			if (bHere)
				ImGui::PopStyleColor();

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s\nstarts %d ms into the cast",
					seq.clips[i].c_str(), Get_ChainOffsetMs(seq, i));

			ImGui::PopID();
		}

		ImGui::PopID();
	}
}

void Client::CAnimation_Tool::Render_NotifyReference(
	const shared_ptr<Engine::CModel>& pModel)
{
	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	if (nullptr == pCurrentName)
		return;

	const auto it = m_ClipNotify.find(pCurrentName);
	if (m_ClipNotify.end() == it || it->second.empty())
		return;

	char_t szHeader[64]{};
	snprintf(szHeader, sizeof(szHeader), "Original Notifies (%d)",
		static_cast<int32_t>(it->second.size()));
	ImGui::SeparatorText(szHeader);

	const f32_t fRate = Get_ClipTickRate(pModel, pCurrentName);
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const bool_t bHasTrack = pModel->Get_AnimationProgress(
		iCurrentIndex, fPosition, fDuration) && fDuration > 0.f;
	const f32_t fNowSec =
		bHasTrack && fRate > 0.f ? fPosition / fRate : -1.f;

	if (!ImGui::BeginChild("##notifyref", ImVec2(0.f, 180.f),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		return;
	}

	int32_t iRow = 0;
	for (const NOTIFY_ROW& row : it->second)
	{
		const bool_t bActive = fNowSec >= 0.f && fNowSec >= row.fTime &&
			fNowSec <= row.fTime + (std::max)(row.fDuration, 0.05f);
		const char_t* pText = !row.sAsset.empty() ?
			row.sAsset.c_str() : row.sLabel.c_str();

		char_t szRow[256]{};
		const int32_t iFrame =
			static_cast<int32_t>(row.fTime * fRate + 0.5f);
		if (row.fDuration > 0.f)
			snprintf(szRow, sizeof(szRow), "f%-4d %-6s %.2f-%.2fs  %s",
				iFrame, Kind_Name(row.eKind), row.fTime,
				row.fTime + row.fDuration, pText);
		else
			snprintf(szRow, sizeof(szRow), "f%-4d %-6s %.2fs  %s",
				iFrame, Kind_Name(row.eKind), row.fTime, pText);

		ImGui::PushID(iRow++);
		if (ImGui::Selectable(szRow, bActive))
		{
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrentIndex, row.fTime * fRate);
			if (EVENT_KIND::HIT == row.eKind && !row.sAsset.empty())
				Bind_ReferenceWire(row.sAsset);
		}
		ImGui::PopID();
	}

	ImGui::EndChild();
}

void Client::CAnimation_Tool::Bind_ReferenceWire(const std::string& sourceKey)
{
	for (const SKILL_TIMING& ref : m_SkillRef)
	{
		for (size_t h = 0; h < ref.hits.size(); ++h)
		{
			const std::string& keys = ref.hits[h].sSourceKeys;
			size_t pos = 0;
			bool_t bMatch = false;
			while (std::string::npos != (pos = keys.find(sourceKey, pos)))
			{
				const size_t end = pos + sourceKey.size();
				if ((0 == pos || ',' == keys[pos - 1]) &&
					(keys.size() == end || ',' == keys[end]))
				{
					bMatch = true;
					break;
				}
				pos = end;
			}
			if (!bMatch)
				continue;

			m_iRefWireSkillId = ref.iSkillId;
			m_iRefWireHitIndex = static_cast<int32_t>(h);
			m_Status = "Reference wire bound to SkillEffect " + sourceKey +
				" of " + ref.name;
			return;
		}
	}
	m_Status = "SkillEffect " + sourceKey +
		" has no judgement shape in the skill timing reference.";
}

std::vector<std::string> Client::CAnimation_Tool::Collect_ClipNames(
	const shared_ptr<Engine::CModel>& pModel) const
{
	std::vector<std::string> clips;
	if (nullptr == pModel)
		return clips;
	clips.reserve(pModel->Get_NumAnimations());
	for (uint32_t index = 0; index < pModel->Get_NumAnimations(); ++index)
	{
		const char_t* clipName = pModel->Get_AnimationName(index);
		if (nullptr != clipName)
			clips.emplace_back(clipName);
	}
	return clips;
}

Client::KAKUL_ANIMATION_ACTION_BINDING*
Client::CAnimation_Tool::Find_KakulActionBinding(
	const std::uint32_t iSourceActionId,
	const std::string& strStageId,
	const std::string& strSlotId)
{
	const auto Found = std::find_if(
		m_KakulActionAuthored.Bindings.begin(),
		m_KakulActionAuthored.Bindings.end(),
		[&](const KAKUL_ANIMATION_ACTION_BINDING& Binding)
		{
			return Binding.iSourceActionId == iSourceActionId &&
				Binding.strStageId == strStageId &&
				Binding.strSlotId == strSlotId;
		});
	return Found == m_KakulActionAuthored.Bindings.end() ?
		nullptr : &*Found;
}

void Client::CAnimation_Tool::Upsert_KakulActionBinding(
	const KAKUL_ANIMATION_ACTION_SLOT_REFERENCE& ReferenceSlot,
	const std::uint32_t iSourceActionId,
	const std::string& strStageId,
	const std::string& strRuntimeClip,
	const std::uint32_t iSourceStartMs,
	const std::uint32_t iPlayMs,
	const f32_t fPlayRate,
	const bool_t bLoop)
{
	KAKUL_ANIMATION_ACTION_BINDING* pBinding = Find_KakulActionBinding(
		iSourceActionId, strStageId, ReferenceSlot.strSlotId);
	if (nullptr == pBinding)
	{
		KAKUL_ANIMATION_ACTION_BINDING Binding;
		Binding.iSourceActionId = iSourceActionId;
		Binding.strStageId = strStageId;
		Binding.strSlotId = ReferenceSlot.strSlotId;
		m_KakulActionAuthored.Bindings.push_back(std::move(Binding));
		pBinding = &m_KakulActionAuthored.Bindings.back();
	}
	pBinding->strRuntimeClip = strRuntimeClip;
	pBinding->iSourceStartMs = iSourceStartMs;
	pBinding->iPlayMs = iPlayMs;
	pBinding->fPlayRate = fPlayRate;
	pBinding->bLoop = bLoop;
	pBinding->strMappingBasis = "PROJECT_AUTHORED";
	pBinding->strAuthority = "REFERENCE_ONLY";
	std::sort(
		m_KakulActionAuthored.Bindings.begin(),
		m_KakulActionAuthored.Bindings.end(),
		[](const KAKUL_ANIMATION_ACTION_BINDING& Left,
			const KAKUL_ANIMATION_ACTION_BINDING& Right)
		{
			return std::tie(
				Left.iSourceActionId, Left.strStageId, Left.strSlotId) <
				std::tie(
					Right.iSourceActionId, Right.strStageId, Right.strSlotId);
		});
	m_bKakulActionDirty = true;
	m_strKakulActionStatus =
		"Staged one PROJECT_AUTHORED local slot override. Save commits only the sparse binding document.";
}

void Client::CAnimation_Tool::Remove_KakulActionBinding(
	const std::uint32_t iSourceActionId,
	const std::string& strStageId,
	const std::string& strSlotId)
{
	const auto NewEnd = std::remove_if(
		m_KakulActionAuthored.Bindings.begin(),
		m_KakulActionAuthored.Bindings.end(),
		[&](const KAKUL_ANIMATION_ACTION_BINDING& Binding)
		{
			return Binding.iSourceActionId == iSourceActionId &&
				Binding.strStageId == strStageId &&
				Binding.strSlotId == strSlotId;
		});
	if (NewEnd == m_KakulActionAuthored.Bindings.end())
		return;
	m_KakulActionAuthored.Bindings.erase(
		NewEnd, m_KakulActionAuthored.Bindings.end());
	m_bKakulActionDirty = true;
	m_strKakulActionStatus =
		"Removed the local override; this slot now resolves to its extracted default.";
}

bool_t Client::CAnimation_Tool::Load_KakulActionBindings(
	const shared_ptr<Engine::CModel>& pModel)
{
	const KAKUL_ACTION_PROFILE_CONTRACT* pProfile =
		Find_KakulActionProfile(m_strKakulProfileId);
	if (nullptr == pProfile || nullptr == pModel)
	{
		m_strKakulActionStatus =
			"No exact KoukuSaton action profile and physical WModel are selected.";
		return false;
	}

	KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT StagedReference;
	KAKUL_ANIMATION_ACTION_AUTHORED_DOCUMENT StagedAuthored;
	std::string Status;
	if (!CKakulAnimationActionDocument::Load(
		pProfile->pProfileId,
		pProfile->pModelAssetId,
		Collect_ClipNames(pModel),
		StagedReference,
		StagedAuthored,
		Status))
	{
		m_strKakulActionStatus =
			"Load rejected; current KoukuSaton action draft preserved: " + Status;
		return false;
	}

	m_KakulActionReference = std::move(StagedReference);
	m_KakulActionAuthored = std::move(StagedAuthored);
	m_iSelectedKakulAction = -1;
	for (int32_t iAction = 0;
		iAction < static_cast<int32_t>(m_KakulActionReference.Actions.size());
		++iAction)
	{
		const KAKUL_ANIMATION_ACTION_REFERENCE& Action =
			m_KakulActionReference.Actions[iAction];
		const bool_t bHasSlot = std::any_of(
			Action.Stages.begin(), Action.Stages.end(),
			[](const KAKUL_ANIMATION_ACTION_STAGE_REFERENCE& Stage)
			{
				return !Stage.Slots.empty();
			});
		if (bHasSlot && "REVIEW_CANDIDATE" == Action.strReviewStatus)
		{
			m_iSelectedKakulAction = iAction;
			break;
		}
	}
	if (m_iSelectedKakulAction < 0 &&
		!m_KakulActionReference.Actions.empty())
	{
		m_iSelectedKakulAction = 0;
	}
	m_iSelectedKakulStage = 0;
	m_iSelectedKakulSlot = 0;
	if (m_iSelectedKakulAction >= 0)
	{
		const auto& Stages = m_KakulActionReference.Actions[
			m_iSelectedKakulAction].Stages;
		for (int32_t iStage = 0;
			iStage < static_cast<int32_t>(Stages.size()); ++iStage)
		{
			if (!Stages[iStage].Slots.empty())
			{
				m_iSelectedKakulStage = iStage;
				break;
			}
		}
	}
	m_bKakulActionDirty = false;
	m_strKakulActionStatus = Status;
	return true;
}

bool_t Client::CAnimation_Tool::Save_KakulActionBindings(
	const shared_ptr<Engine::CModel>& pModel)
{
	const KAKUL_ACTION_PROFILE_CONTRACT* pProfile =
		Find_KakulActionProfile(m_strKakulProfileId);
	if (nullptr == pProfile || nullptr == pModel ||
		m_KakulActionReference.Actions.empty())
	{
		m_strKakulActionStatus =
			"KoukuSaton action Save requires a validated profile reference and physical WModel.";
		return false;
	}
	std::string Status;
	if (!CKakulAnimationActionDocument::Save_Atomic(
		m_KakulActionAuthored,
		m_KakulActionReference,
		pProfile->pProfileId,
		pProfile->pModelAssetId,
		Collect_ClipNames(pModel),
		Status))
	{
		m_strKakulActionStatus =
			"Save rejected; destination and current KoukuSaton bindings preserved: " +
			Status;
		return false;
	}
	m_bKakulActionDirty = false;
	m_strKakulActionStatus = Status;
	return true;
}

void Client::CAnimation_Tool::Render_KakulActionBindings(
	const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SeparatorText("KoukuSaton Extracted Action Sequences");
	ImGui::TextColored(
		ImVec4(0.95f, 0.75f, 0.2f, 1.f),
		"Local Extracted Action Preview / REFERENCE_ONLY");
	ImGui::TextWrapped(
		"The extracted action sequence is the immutable default. Save writes only "
		"PROJECT_AUTHORED slot overrides; it does not create a Server Product boss pattern.");

	if ("MN_RPCT_05" == m_AssetName)
	{
		ImGui::TextDisabled("Physical body MN_RPCT_05 serves two extracted profiles:");
		ImGui::BeginDisabled("MN_RPCT_05" == m_strKakulProfileId);
		if (ImGui::SmallButton("Open MN_RPCT_05 Profile"))
			(void)Open_KakulProfile("MN_RPCT_05");
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled("MN_RPCT_07" == m_strKakulProfileId);
		if (ImGui::SmallButton("Open MN_RPCT_07 Alias Profile"))
			(void)Open_KakulProfile("MN_RPCT_07");
		ImGui::EndDisabled();
	}

	if (!m_bKakulActionLoadAttempted)
	{
		m_bKakulActionLoadAttempted = true;
		(void)Load_KakulActionBindings(pModel);
	}
	if (m_KakulActionReference.Actions.empty())
	{
		if (!m_strKakulActionStatus.empty())
			ImGui::TextWrapped("%s", m_strKakulActionStatus.c_str());
		return;
	}

	ImGui::Text(
		"Profile: %s | Actions: %zu | Local overrides: %zu",
		m_strKakulProfileId.c_str(),
		m_KakulActionReference.Actions.size(),
		m_KakulActionAuthored.Bindings.size());
	if (ImGui::Button("Save KoukuSaton Action Bindings"))
		(void)Save_KakulActionBindings(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Reload KoukuSaton Action Bindings"))
	{
		if (m_bKakulActionDirty)
			m_bKakulActionReloadConfirmationRequested = true;
		else
			(void)Load_KakulActionBindings(pModel);
	}
	if (m_bKakulActionDirty)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted("*");
	}
	if (m_bKakulActionReloadConfirmationRequested)
	{
		ImGui::OpenPopup("Discard unsaved KoukuSaton Action Bindings?");
		m_bKakulActionReloadConfirmationRequested = false;
	}
	if (ImGui::BeginPopupModal(
		"Discard unsaved KoukuSaton Action Bindings?", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(
			"Reload replaces only the unsaved sparse KoukuSaton slot overrides.");
		if (ImGui::Button("Discard Overrides and Reload"))
		{
			if (Load_KakulActionBindings(pModel))
				ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	if (!m_strKakulActionStatus.empty())
		ImGui::TextWrapped("%s", m_strKakulActionStatus.c_str());

	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##KakulActionFilter", "Filter Korean name or sourceActionId...",
		m_KakulActionFilter, sizeof(m_KakulActionFilter));
	const std::string strFilter = m_KakulActionFilter;
	std::vector<int32_t> VisibleActions;
	VisibleActions.reserve(m_KakulActionReference.Actions.size());
	for (int32_t iAction = 0;
		iAction < static_cast<int32_t>(m_KakulActionReference.Actions.size());
		++iAction)
	{
		const KAKUL_ANIMATION_ACTION_REFERENCE& Action =
			m_KakulActionReference.Actions[iAction];
		if (!strFilter.empty() &&
			std::string::npos == Action.strDisplayName.find(strFilter) &&
			std::string::npos == std::to_string(
				Action.iSourceActionId).find(strFilter))
		{
			continue;
		}
		VisibleActions.push_back(iAction);
	}

	const f32_t fKakulWorkspaceWidth =
		(std::max)(0.f, ImGui::GetContentRegionAvail().x);
	const bool_t bKakulPanelsSideBySide =
		fKakulWorkspaceWidth >= KAKUL_ACTION_LIST_MIN_WIDTH +
			KAKUL_ACTION_SPLITTER_WIDTH + KAKUL_ACTION_DETAIL_MIN_WIDTH;
	const f32_t fKakulActionListMaximumWidth = (std::max)(
		KAKUL_ACTION_LIST_MIN_WIDTH,
		fKakulWorkspaceWidth - KAKUL_ACTION_SPLITTER_WIDTH -
			KAKUL_ACTION_DETAIL_MIN_WIDTH);
	m_fKakulActionListWidth = std::clamp(
		m_fKakulActionListWidth,
		KAKUL_ACTION_LIST_MIN_WIDTH,
		fKakulActionListMaximumWidth);
	const f32_t fKakulActionListWidth =
		bKakulPanelsSideBySide ? m_fKakulActionListWidth : 0.f;
	const f32_t fKakulActionListHeight =
		bKakulPanelsSideBySide ? 460.f : 260.f;

	if (ImGui::BeginChild(
		"##KakulActions",
		ImVec2(fKakulActionListWidth, fKakulActionListHeight),
		ImGuiChildFlags_Borders))
	{
		ImGui::TextDisabled("Korean Action / Sequence (%zu)", VisibleActions.size());
		ImGuiListClipper Clipper;
		Clipper.Begin(static_cast<int32_t>(VisibleActions.size()));
		while (Clipper.Step())
		{
			for (int32_t iVisible = Clipper.DisplayStart;
				iVisible < Clipper.DisplayEnd; ++iVisible)
			{
				const int32_t iAction = VisibleActions[iVisible];
				const KAKUL_ANIMATION_ACTION_REFERENCE& Action =
					m_KakulActionReference.Actions[iAction];
				char_t Label[512]{};
				snprintf(
					Label, sizeof(Label), "%u  %s  [%s]",
					Action.iSourceActionId,
					Action.strDisplayName.c_str(),
					Action.strReviewStatus.c_str());
				ImGui::PushID(iAction);
				if (ImGui::Selectable(
					Label, m_iSelectedKakulAction == iAction))
				{
					m_iSelectedKakulAction = iAction;
					m_iSelectedKakulStage = 0;
					m_iSelectedKakulSlot = 0;
					for (int32_t iStage = 0;
						iStage < static_cast<int32_t>(Action.Stages.size());
						++iStage)
					{
						if (!Action.Stages[iStage].Slots.empty())
						{
							m_iSelectedKakulStage = iStage;
							break;
						}
					}
				}
				ImGui::PopID();
			}
		}
	}
	ImGui::EndChild();
	if (bKakulPanelsSideBySide)
	{
		ImGui::SameLine(0.f, 0.f);
		(void)ImGui::InvisibleButton("##KakulActionSplitter",
			ImVec2(KAKUL_ACTION_SPLITTER_WIDTH, 460.f));
		if (ImGui::IsItemHovered() || ImGui::IsItemActive())
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		if (ImGui::IsItemActive())
		{
			m_fKakulActionListWidth = std::clamp(
				m_fKakulActionListWidth + ImGui::GetIO().MouseDelta.x,
				KAKUL_ACTION_LIST_MIN_WIDTH,
				fKakulActionListMaximumWidth);
		}
		ImGui::SameLine(0.f, 0.f);
	}
	else
	{
		ImGui::TextDisabled(
			"Compact viewport: Action list is stacked above Selected Slot Detail.");
	}

	if (ImGui::BeginChild(
		"##KakulActionDetail", ImVec2(0.f, 460.f),
		ImGuiChildFlags_Borders))
	{
		if (m_iSelectedKakulAction < 0 ||
			m_iSelectedKakulAction >= static_cast<int32_t>(
				m_KakulActionReference.Actions.size()))
		{
			ImGui::TextUnformatted("Select one extracted action sequence.");
		}
		else
		{
			const KAKUL_ANIMATION_ACTION_REFERENCE& Action =
				m_KakulActionReference.Actions[m_iSelectedKakulAction];
			ImGui::TextWrapped(
				"%u | %s", Action.iSourceActionId,
				Action.strDisplayName.c_str());
			if ("HOLDOUT" == Action.strReviewStatus)
			{
				ImGui::TextColored(
					ImVec4(1.f, 0.45f, 0.25f, 1.f),
					"HOLDOUT: local inspection is allowed; Product promotion is not implied.");
			}

			if (Action.Stages.empty())
			{
				ImGui::TextUnformatted("This action has no extracted stages.");
			}
			else
			{
				m_iSelectedKakulStage = std::clamp(
					m_iSelectedKakulStage, 0,
					static_cast<int32_t>(Action.Stages.size()) - 1);
				const KAKUL_ANIMATION_ACTION_STAGE_REFERENCE& SelectedStage =
					Action.Stages[m_iSelectedKakulStage];
				char_t StageLabel[192]{};
				snprintf(
					StageLabel, sizeof(StageLabel), "%u / %s | %zu slot(s)",
					SelectedStage.iStageOrdinal,
					SelectedStage.strStageId.c_str(),
					SelectedStage.Slots.size());
				ImGui::SetNextItemWidth(-1.f);
				if (ImGui::BeginCombo("Stage", StageLabel))
				{
					for (int32_t iStage = 0;
						iStage < static_cast<int32_t>(Action.Stages.size());
						++iStage)
					{
						const auto& Stage = Action.Stages[iStage];
						char_t CandidateLabel[192]{};
						snprintf(
							CandidateLabel, sizeof(CandidateLabel),
							"%u / %s | %zu slot(s)%s",
							Stage.iStageOrdinal, Stage.strStageId.c_str(),
							Stage.Slots.size(),
							Stage.HoldoutClipNames.empty() ? "" : " | HOLDOUT refs");
						ImGui::PushID(iStage);
						if (ImGui::Selectable(
							CandidateLabel, iStage == m_iSelectedKakulStage))
						{
							m_iSelectedKakulStage = iStage;
							m_iSelectedKakulSlot = 0;
						}
						ImGui::PopID();
					}
					ImGui::EndCombo();
				}

				const KAKUL_ANIMATION_ACTION_STAGE_REFERENCE& Stage =
					Action.Stages[m_iSelectedKakulStage];
				if (!Stage.HoldoutClipNames.empty())
				{
					ImGui::TextDisabled("Unresolved extracted clip evidence:");
					for (const std::string& Holdout : Stage.HoldoutClipNames)
						ImGui::BulletText("%s", Holdout.c_str());
				}

				if (ImGui::BeginChild(
					"##KakulSlots", ImVec2(0.f, 105.f),
					ImGuiChildFlags_Borders))
				{
					for (int32_t iSlot = 0;
						iSlot < static_cast<int32_t>(Stage.Slots.size()); ++iSlot)
					{
						const auto& Slot = Stage.Slots[iSlot];
						const KAKUL_ANIMATION_ACTION_BINDING* pOverride =
							Find_KakulActionBinding(
								Action.iSourceActionId, Stage.strStageId,
								Slot.strSlotId);
						const std::string& EffectiveClip = nullptr != pOverride ?
							pOverride->strRuntimeClip : Slot.strRuntimeClip;
						const std::string Label = Slot.strSlotId + "  " +
							EffectiveClip + (nullptr != pOverride ? "  *" : "");
						ImGui::PushID(iSlot);
						if (ImGui::Selectable(
							Label.c_str(), iSlot == m_iSelectedKakulSlot))
						{
							m_iSelectedKakulSlot = iSlot;
							Select_Clip(pModel, EffectiveClip);
						}
						ImGui::PopID();
					}
				}
				ImGui::EndChild();

				if (Stage.Slots.empty())
				{
					ImGui::TextDisabled(
						"This stage has no exact physical WModel slot to author.");
				}
				else
				{
					m_iSelectedKakulSlot = std::clamp(
						m_iSelectedKakulSlot, 0,
						static_cast<int32_t>(Stage.Slots.size()) - 1);
					const KAKUL_ANIMATION_ACTION_SLOT_REFERENCE& Slot =
						Stage.Slots[m_iSelectedKakulSlot];
					const KAKUL_ANIMATION_ACTION_BINDING* pOverride =
						Find_KakulActionBinding(
							Action.iSourceActionId, Stage.strStageId,
							Slot.strSlotId);
					std::string strEditedRuntimeClip = nullptr != pOverride ?
						pOverride->strRuntimeClip : Slot.strRuntimeClip;
					int32_t iEditedSourceStartMs = static_cast<int32_t>(
						nullptr != pOverride ? pOverride->iSourceStartMs :
						Slot.iSourceStartMs);
					int32_t iEditedPlayMs = static_cast<int32_t>(
						nullptr != pOverride ? pOverride->iPlayMs : Slot.iPlayMs);
					f32_t fEditedPlayRate = nullptr != pOverride ?
						pOverride->fPlayRate : Slot.fPlayRate;
					bool_t bEditedLoop = nullptr != pOverride ?
						pOverride->bLoop : Slot.bLoop;
					bool_t bEdited = false;

					ImGui::SeparatorText("Selected Slot Detail");
					ImGui::Text("Identity: %s / %s", Stage.strStageId.c_str(),
						Slot.strSlotId.c_str());
					ImGui::TextWrapped(
						"Extracted: %s -> default runtime: %s",
						Slot.strExtractedClip.c_str(), Slot.strRuntimeClip.c_str());
					const char_t* pCurrentClip = pModel->Get_AnimationName(
						pModel->Get_CurrentAnimIndex());
					ImGui::TextDisabled(
						"Current WModel clip: %s",
						nullptr != pCurrentClip ? pCurrentClip : "(none)");

					const std::vector<std::string> AvailableClips =
						Collect_ClipNames(pModel);
					ImGui::SetNextItemWidth(-1.f);
					if (ImGui::BeginCombo(
						"Runtime Clip", strEditedRuntimeClip.c_str()))
					{
						for (const std::string& Clip : AvailableClips)
						{
							if (ImGui::Selectable(
								Clip.c_str(), Clip == strEditedRuntimeClip))
							{
								strEditedRuntimeClip = Clip;
								bEdited = true;
								Select_Clip(pModel, Clip);
							}
						}
						ImGui::EndCombo();
					}
					if (nullptr != pCurrentClip &&
						ImGui::Button("Assign Current WModel Clip"))
					{
						strEditedRuntimeClip = pCurrentClip;
						bEdited = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("Preview Effective Clip"))
						Select_Clip(pModel, strEditedRuntimeClip);

					if (ImGui::InputInt(
						"Source Start (ms)", &iEditedSourceStartMs))
					{
						iEditedSourceStartMs = std::clamp(
							iEditedSourceStartMs, 0, 600000);
						bEdited = true;
					}
					if (ImGui::InputInt("Play Window (ms)", &iEditedPlayMs))
					{
						iEditedPlayMs = std::clamp(iEditedPlayMs, 1, 600000);
						bEdited = true;
					}
					if (ImGui::InputFloat(
						"Play Rate", &fEditedPlayRate, 0.01f, 0.1f, "%.3f"))
					{
						fEditedPlayRate = std::clamp(fEditedPlayRate, 0.01f, 16.f);
						bEdited = true;
					}
					if (ImGui::Checkbox("Loop", &bEditedLoop))
						bEdited = true;

					bool_t bReset = false;
					if (nullptr != pOverride)
					{
						if (ImGui::Button("Reset Slot to Extracted Default"))
							bReset = true;
					}
					if (bReset)
					{
						Remove_KakulActionBinding(
							Action.iSourceActionId, Stage.strStageId,
							Slot.strSlotId);
						Select_Clip(pModel, Slot.strRuntimeClip);
					}
					else if (bEdited)
					{
						Upsert_KakulActionBinding(
							Slot, Action.iSourceActionId, Stage.strStageId,
							strEditedRuntimeClip,
							static_cast<std::uint32_t>(iEditedSourceStartMs),
							static_cast<std::uint32_t>(iEditedPlayMs),
							fEditedPlayRate, bEditedLoop);
					}
				}
			}
		}
	}
	ImGui::EndChild();
}

Client::ANIMATION_SKILL_BINDING*
Client::CAnimation_Tool::Find_SkillBinding(
	const LostArk::Shared::SKILL_ID skillId)
{
	for (ANIMATION_SKILL_BINDING& binding :
		m_SkillBindingDocument.Bindings)
	{
		if (binding.iSkillId == skillId)
			return &binding;
	}
	return nullptr;
}

bool_t Client::CAnimation_Tool::Load_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	ANIMATION_SKILL_BINDING_DOCUMENT staged;
	std::string status;
	if (!CAnimationSkillBindingDocument::Load(
		m_AssetName,
		characterClass,
		CPlayerSkillCatalog::Get_Skills(),
		Collect_ClipNames(pModel),
		staged,
		status))
	{
		m_SkillBindingStatus =
			"Load rejected; current Skill Bindings preserved: " + status;
		return false;
	}

	m_SkillBindingDocument = std::move(staged);
	m_iSelectedSkillBinding = -1;
	m_iSelectedSkillClip = 0;
	m_bSkillBindingDirty = false;
	m_SkillBindingStatus = status;
	return true;
}

bool_t Client::CAnimation_Tool::Save_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const shared_ptr<CCharacter>& pCharacter)
{
	if (nullptr == pCharacter || nullptr == pCharacter->Get_Spec() ||
		nullptr == pCharacter->Get_Spec()->pAssetName ||
		m_AssetName != pCharacter->Get_Spec()->pAssetName)
	{
		m_SkillBindingStatus =
			"Skill Bindings can only be saved for the selected Scene Character.";
		return false;
	}

	std::string status;
	if (!CAnimationSkillBindingDocument::Save_Atomic(
		m_SkillBindingDocument,
		m_AssetName,
		pCharacter->Get_Spec()->eCharacterClass,
		CPlayerSkillCatalog::Get_Skills(),
		Collect_ClipNames(pModel),
		status))
	{
		m_SkillBindingStatus =
			"Save rejected; destination and current bindings preserved: " + status;
		return false;
	}

	m_bSkillBindingDirty = false;
	if (!pCharacter->Reload_SkillAnimationBindings())
	{
		m_SkillBindingStatus = status +
			" [saved, but the live Character kept its previous binding set]";
		return true;
	}
	m_SkillBindingStatus = status + " [live Character refreshed]";
	return true;
}

bool_t Client::CAnimation_Tool::Create_SkillBindingDraft(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (nullptr == pModel || m_AssetName.empty())
		return false;
	const char_t* currentClip = pModel->Get_AnimationName(
		pModel->Get_CurrentAnimIndex());
	if (nullptr == currentClip)
	{
		m_SkillBindingStatus =
			"Select one model clip before creating a repair draft.";
		return false;
	}

	ANIMATION_SKILL_BINDING_DOCUMENT staged;
	staged.strAnimationAssetId = m_AssetName;
	staged.eCharacterClass = characterClass;
	for (const PLAYER_SKILL_DEFINITION& definition :
		CPlayerSkillCatalog::Get_Skills())
	{
		if (definition.eCharacterClass != characterClass)
			continue;
		ANIMATION_SKILL_BINDING binding;
		binding.iSkillId = definition.iSkillId;
		const bool_t isStaged =
			LostArk::Shared::PLAYER_SKILL_KIND::COMBO ==
				definition.eSkillKind ||
			LostArk::Shared::PLAYER_SKILL_KIND::HOLD == definition.eSkillKind ||
			LostArk::Shared::PLAYER_SKILL_KIND::COUNTER ==
				definition.eSkillKind;
		const std::size_t stageCount =
			isStaged ? definition.iComboStageCount : 1u;
		ANIMATION_SKILL_STAGE seedStage;
		seedStage.Clips.assign(
			1u, ANIMATION_SKILL_CLIP{ currentClip, 0u, 1.f });
		binding.Stages.assign(stageCount, seedStage);
		staged.Bindings.push_back(std::move(binding));
	}
	if (staged.Bindings.empty())
	{
		m_SkillBindingStatus =
			"PlayerSkills has no definitions for this Character class.";
		return false;
	}

	m_SkillBindingDocument = std::move(staged);
	m_iSelectedSkillBinding = 0;
	m_iSelectedSkillClip = 0;
	m_bSkillBindingDirty = true;
	m_SkillBindingStatus =
		"Created a complete repair draft. Every row currently uses " +
		std::string(currentClip) +
		"; assign intended clips before Save.";
	return true;
}

void Client::CAnimation_Tool::Render_SkillBindingReloadConfirmation(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (m_bSkillBindingReloadConfirmationRequested)
	{
		ImGui::OpenPopup("Discard unsaved Skill Animation Bindings?");
		m_bSkillBindingReloadConfirmationRequested = false;
	}
	if (!ImGui::BeginPopupModal(
		"Discard unsaved Skill Animation Bindings?",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}
	ImGui::TextUnformatted(
		"Reload replaces only the unsaved key/skill animation binding document.");
	if (ImGui::Button("Discard Bindings and Reload"))
	{
		if (Load_SkillBindings(pModel, characterClass))
			ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		ImGui::CloseCurrentPopup();
	ImGui::EndPopup();
}

void Client::CAnimation_Tool::Render_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const shared_ptr<CCharacter>& pCharacter)
{
	ImGui::SeparatorText("Key -> Skill Animation");
	if (nullptr == pCharacter || nullptr == pCharacter->Get_Spec() ||
		nullptr == pCharacter->Get_Spec()->pAssetName ||
		m_AssetName != pCharacter->Get_Spec()->pAssetName)
	{
		ImGui::TextDisabled(
			"Select Scene Character to author gameplay key bindings."
			" Reference-only preview assets are not playable classes.");
		return;
	}

	const CHARACTER_SPEC* spec = pCharacter->Get_Spec();
	if (!m_bSkillBindingLoadAttempted)
	{
		m_bSkillBindingLoadAttempted = true;
		Load_SkillBindings(pModel, spec->eCharacterClass);
	}
	if (m_SkillBindingDocument.Bindings.empty())
	{
		ImGui::TextWrapped("%s", m_SkillBindingStatus.c_str());
		ImGui::TextWrapped(
			"The Character remains available even when this document is missing or "
			"invalid. Create a complete in-memory repair draft from the currently "
			"selected model clip, then assign each skill and Save.");
		if (ImGui::Button("Create Repair Draft from Current Clip"))
			Create_SkillBindingDraft(pModel, spec->eCharacterClass);
		return;
	}

	ImGui::TextWrapped(
		"PlayerSkills owns key -> skillId and Server timing. This panel saves only "
		"the approved skillId -> ordered presentation clips. BA1/BA2/... are "
		"indexed directly by the replicated comboStage.");
	if (ImGui::Button("Save Skill Bindings"))
		Save_SkillBindings(pModel, pCharacter);
	ImGui::SameLine();
	if (ImGui::Button("Reload Skill Bindings"))
	{
		if (m_bSkillBindingDirty)
			m_bSkillBindingReloadConfirmationRequested = true;
		else
			Load_SkillBindings(pModel, spec->eCharacterClass);
	}
	if (m_bSkillBindingDirty)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted("*");
	}
	Render_SkillBindingReloadConfirmation(pModel, spec->eCharacterClass);
	if (!m_SkillBindingStatus.empty())
		ImGui::TextWrapped("%s", m_SkillBindingStatus.c_str());

	const uint32_t currentAnimationIndex = pModel->Get_CurrentAnimIndex();
	const char_t* currentClip =
		pModel->Get_AnimationName(currentAnimationIndex);
	ImGui::TextDisabled(
		"Current clip: %s",
		nullptr != currentClip ? currentClip : "(none)");

	std::vector<const PLAYER_SKILL_DEFINITION*> classSkills;
	for (const PLAYER_SKILL_DEFINITION& definition :
		CPlayerSkillCatalog::Get_Skills())
	{
		if (definition.eCharacterClass == spec->eCharacterClass)
			classSkills.push_back(&definition);
	}
	const auto slotRank = [](const std::string& slot)
	{
		constexpr const char_t* preferred[] =
		{
			"Q", "W", "E", "R", "A", "S", "D", "F",
			"T", "V", "ALT_V", "LMB"
		};
		for (int32_t index = 0; index < static_cast<int32_t>(std::size(preferred)); ++index)
		{
			if (slot == preferred[index])
				return index;
		}
		return static_cast<int32_t>(std::size(preferred));
	};
	std::sort(classSkills.begin(), classSkills.end(),
		[&](const PLAYER_SKILL_DEFINITION* left,
			const PLAYER_SKILL_DEFINITION* right)
		{
			const int32_t leftRank = slotRank(left->strInputSlot);
			const int32_t rightRank = slotRank(right->strInputSlot);
			return leftRank != rightRank ? leftRank < rightRank :
				left->strInputSlot < right->strInputSlot;
		});

	if (!ImGui::BeginChild(
		"##skillbindings",
		ImVec2(0.f, 300.f),
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		return;
	}
	for (const PLAYER_SKILL_DEFINITION* definition : classSkills)
	{
		const std::string slotLabel =
			"ALT_V" == definition->strInputSlot ? "ALT+V" :
			("LMB" == definition->strInputSlot ? "BA / LMB" :
				definition->strInputSlot);
		ANIMATION_SKILL_BINDING* binding =
			Find_SkillBinding(definition->iSkillId);
		if (nullptr == binding)
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.35f, 0.35f, 1.f),
				"%-8s  %u missing authored binding",
				slotLabel.c_str(),
				definition->iSkillId);
			continue;
		}
		const int32_t bindingIndex = static_cast<int32_t>(
			binding - m_SkillBindingDocument.Bindings.data());
		ImGui::PushID(bindingIndex);
		const bool_t isCombo =
			LostArk::Shared::PLAYER_SKILL_KIND::COMBO ==
			definition->eSkillKind;
		const bool_t isStaged = isCombo ||
			LostArk::Shared::PLAYER_SKILL_KIND::HOLD ==
				definition->eSkillKind ||
			LostArk::Shared::PLAYER_SKILL_KIND::COUNTER ==
				definition->eSkillKind;
		char_t header[192]{};
		snprintf(
			header,
			sizeof(header),
			"%s  %u  %s  [%s]",
			slotLabel.c_str(),
			definition->iSkillId,
			definition->strDisplayName.c_str(),
			isCombo ? "COMBO" : "ACTIVE");
		if (ImGui::TreeNodeEx("##binding", ImGuiTreeNodeFlags_DefaultOpen, "%s", header))
		{
			for (int32_t stageIndex = 0;
				stageIndex < static_cast<int32_t>(binding->Stages.size());
				++stageIndex)
			{
				ImGui::PushID(stageIndex);
				const ANIMATION_SKILL_STAGE& stage = binding->Stages[stageIndex];
				if (isStaged)
					ImGui::TextDisabled("BA%d", stageIndex + 1);
				for (int32_t clipIndex = 0;
					clipIndex < static_cast<int32_t>(stage.Clips.size());
					++clipIndex)
				{
					ImGui::PushID(clipIndex);
					char_t clipLabel[MAX_PATH + 32]{};
					if (isStaged)
					{
						snprintf(
							clipLabel,
							sizeof(clipLabel),
							"  BA%d.%d  %s",
							stageIndex + 1,
							clipIndex + 1,
							stage.Clips[clipIndex].strClipName.c_str());
					}
					else
					{
						snprintf(
							clipLabel,
							sizeof(clipLabel),
							"clip%d  %s",
							clipIndex + 1,
							stage.Clips[clipIndex].strClipName.c_str());
					}
					const bool_t selected =
						m_iSelectedSkillBinding == bindingIndex &&
						m_iSelectedSkillStage == stageIndex &&
						m_iSelectedSkillClip == clipIndex;
					if (ImGui::Selectable(clipLabel, selected))
					{
						m_iSelectedSkillBinding = bindingIndex;
						m_iSelectedSkillStage = stageIndex;
						m_iSelectedSkillClip = clipIndex;
						Select_Clip(pModel, stage.Clips[clipIndex].strClipName);
					}
					ImGui::PopID();
				}
				ImGui::PopID();
			}

			int32_t selectedStage =
				m_iSelectedSkillBinding == bindingIndex ?
				m_iSelectedSkillStage : 0;
			if (selectedStage < 0 ||
				selectedStage >= static_cast<int32_t>(binding->Stages.size()))
			{
				selectedStage = 0;
			}
			std::vector<ANIMATION_SKILL_CLIP>& selectedClips =
				binding->Stages[selectedStage].Clips;
			int32_t selectedClip =
				m_iSelectedSkillBinding == bindingIndex ?
				m_iSelectedSkillClip : 0;
			if (selectedClip < 0 ||
				selectedClip >= static_cast<int32_t>(selectedClips.size()))
			{
				selectedClip = 0;
			}
			std::size_t boundClips = 0u;
			for (const ANIMATION_SKILL_STAGE& stage : binding->Stages)
				boundClips += stage.Clips.size();

			ImGui::BeginDisabled(nullptr == currentClip);
			if (ImGui::Button("Assign Current Clip to Selected Step"))
			{
				selectedClips[selectedClip].strClipName = currentClip;
				m_iSelectedSkillBinding = bindingIndex;
				m_iSelectedSkillStage = selectedStage;
				m_iSelectedSkillClip = selectedClip;
				m_bSkillBindingDirty = true;
				m_SkillBindingStatus =
					"Assigned " + std::string(currentClip) + " to " +
					slotLabel + (isStaged ?
						(" BA" + std::to_string(selectedStage + 1) + "." +
							std::to_string(selectedClip + 1)) :
						(" clip" + std::to_string(selectedClip + 1)));
			}
			ImGui::SameLine();
			/* A stage may hold several clips, so this adds inside the selected
			stage. The stage count itself stays Server-owned. */
			if (ImGui::Button("Add Current Step") && boundClips < 16u)
			{
				selectedClips.insert(
					selectedClips.begin() + selectedClip + 1,
					ANIMATION_SKILL_CLIP{ currentClip, 0u, 1.f });
				m_iSelectedSkillBinding = bindingIndex;
				m_iSelectedSkillStage = selectedStage;
				m_iSelectedSkillClip = selectedClip + 1;
				m_bSkillBindingDirty = true;
			}
			ImGui::EndDisabled();

			if (selectedClips.size() > 1u)
			{
				if (ImGui::SmallButton("Up") && selectedClip > 0)
				{
					std::swap(
						selectedClips[selectedClip],
						selectedClips[selectedClip - 1]);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillStage = selectedStage;
					m_iSelectedSkillClip = selectedClip - 1;
					m_bSkillBindingDirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Down") &&
					selectedClip + 1 < static_cast<int32_t>(selectedClips.size()))
				{
					std::swap(
						selectedClips[selectedClip],
						selectedClips[selectedClip + 1]);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillStage = selectedStage;
					m_iSelectedSkillClip = selectedClip + 1;
					m_bSkillBindingDirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Remove Step"))
				{
					selectedClips.erase(selectedClips.begin() + selectedClip);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillStage = selectedStage;
					m_iSelectedSkillClip = std::min(
						selectedClip,
						static_cast<int32_t>(selectedClips.size()) - 1);
					m_bSkillBindingDirty = true;
				}
			}
			if (isStaged)
			{
				ImGui::TextDisabled(
					"BA stage count is fixed by PlayerSkills comboStages (%zu).",
					definition->iComboStageCount);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::EndChild();
}

void Client::CAnimation_Tool::Render_HitEvents(const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SeparatorText("Events");

	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	if (nullptr == pCurrentName)
	{
		ImGui::TextUnformatted("No clip selected.");
		return;
	}

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const bool_t bHasTrack =
		pModel->Get_AnimationProgress(iCurrentIndex, fPosition, fDuration) && fDuration > 0.f;

	const f32_t fRate = Get_ClipTickRate(pModel, pCurrentName);
	const int32_t iFrame = static_cast<int32_t>(fPosition);
	const int32_t iNowMs = Frame_To_Ms(iFrame, fRate);

	auto Add_Event = [&](EVENT_KIND eKind)
	{
		ANIM_EVENT evt{};
		evt.clipName = pCurrentName;
		evt.eKind = eKind;
		if (EVENT_KIND::EFFECT == eKind)
		{
			evt.eEffectReferenceKind =
				EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
		}
		evt.iStartMs = iNowMs;
		evt.iEndMs = iNowMs;
		m_Events.push_back(evt);
		m_iSelectedEvent = static_cast<int32_t>(m_Events.size()) - 1;
		m_PayloadEdit[0] = '\0';
		m_bDirty = true;
	};

	/* Everything is added at the playhead; window kinds then grow an end. */
	if (ImGui::Button("Hit"))
		Add_Event(EVENT_KIND::HIT);
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		Add_Event(EVENT_KIND::CANCEL);
	ImGui::SameLine();
	if (ImGui::Button("SuperArmor"))
		Add_Event(EVENT_KIND::SUPERARMOR);
	ImGui::SameLine();
	if (ImGui::Button("Invuln"))
		Add_Event(EVENT_KIND::INVULN);
	ImGui::SameLine();
	if (ImGui::Button("Move"))
		Add_Event(EVENT_KIND::MOVE);

	if (ImGui::Button("Sound"))
		Add_Event(EVENT_KIND::SOUND);
	ImGui::SameLine();
	if (ImGui::Button("Effect"))
		Add_Event(EVENT_KIND::EFFECT);

	ImGui::SameLine();
	if (ImGui::Button("Save"))
		Save_Events(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		if (m_bDirty)
			m_bReloadConfirmationRequested = true;
		else
			Load_Events(pModel);
	}
	Render_ReloadConfirmation(pModel);

	ImGui::SameLine();
	ImGui::TextDisabled("%.0f fps", fRate);
	ImGui::SameLine();
	ImGui::Checkbox("show areas", &m_bShowHitAreas);

	/* Original notifies for this clip, lifted from the game's Action table. */
	const auto itNotify = m_ClipNotify.find(pCurrentName);
	if (m_ClipNotify.end() != itNotify)
	{
		int32_t iAvailable = 0;
		for (const NOTIFY_ROW& row : itNotify->second)
		{
			if (m_bImportKind[ETOI(row.eKind)])
				++iAvailable;
		}

		char_t szImport[96]{};
		snprintf(szImport, sizeof(szImport), "Import original (%d of %d)",
			iAvailable, static_cast<int32_t>(itNotify->second.size()));

		if (ImGui::Button(szImport))
		{
			int32_t iShapedHits = 0;
			const int32_t iAdded = Import_Notifies(pCurrentName, fRate, iShapedHits);
			m_Status = "Imported " + std::to_string(iAdded) +
				" original notify event(s) onto " + pCurrentName;
			if (iShapedHits > 0)
				m_Status += ", " + std::to_string(iShapedHits) +
					" HIT shape(s) from skill timing reference";
		}

		ImGui::SameLine();
		ImGui::TextDisabled("|");

		for (int32_t k = 0; k < ETOI(EVENT_KIND::END); ++k)
		{
			const EVENT_KIND eKind = static_cast<EVENT_KIND>(k);
			if (EVENT_KIND::INVULN == eKind || EVENT_KIND::MOVE == eKind)
				continue;

			ImGui::SameLine();
			ImGui::Checkbox(Kind_Name(eKind), &m_bImportKind[k]);
		}
	}

	if (m_bDirty)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted("*");
	}

	if (!m_Status.empty())
		ImGui::TextWrapped("%s", m_Status.c_str());

	if (!ImGui::BeginChild("##eventlist", ImVec2(0.f, 170.f),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		return;
	}

	int32_t iShown = 0;
	int32_t iRemoveIndex = -1;

	for (int32_t i = 0; i < static_cast<int32_t>(m_Events.size()); ++i)
	{
		ANIM_EVENT& evt = m_Events[i];
		if (evt.clipName != pCurrentName)
			continue;

		++iShown;
		ImGui::PushID(i);

		const bool_t bWindow = Is_Window(evt.eKind);
		const bool_t bHit = EVENT_KIND::HIT == evt.eKind;
		const bool_t bEffectCueWindow =
			EVENT_KIND::EFFECT == evt.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				evt.eEffectReferenceKind &&
			EFFECT_STOP_POLICY::CUE_END == evt.eStopPolicy;

		int32_t iStartFrame = Ms_To_Frame(evt.iStartMs, fRate);
		int32_t iEndFrame = Ms_To_Frame(evt.iEndMs, fRate);

		/* A window highlights over its span (and over every repeat); a point event
		only on its own frame. */
		const int32_t iActiveTick = (bWindow && bHasTrack) ? Get_ActiveTick(evt, iNowMs) : -1;
		const bool_t bActive = bWindow ? (iActiveTick >= 0) : (iStartFrame == iFrame);
		if (bActive)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.f, 0.4f, 1.f));

		char_t szLabel[192]{};
		if (bHit && evt.hit.iRepeatCount > 1)
			snprintf(szLabel, sizeof(szLabel), "HIT %d - %d  x%d every %dms",
				iStartFrame, iEndFrame, evt.hit.iRepeatCount, evt.hit.iRepeatMs);
		else if (bWindow)
			snprintf(szLabel, sizeof(szLabel), "%s %d - %d  %s",
				Kind_Name(evt.eKind), iStartFrame, iEndFrame, evt.sPayload.c_str());
		else
			snprintf(szLabel, sizeof(szLabel), "%s %d  %s",
				Kind_Name(evt.eKind), iStartFrame,
				evt.sPayload.empty() ? "(unset)" : evt.sPayload.c_str());

		if (bWindow && bActive && evt.hit.iRepeatCount > 1)
		{
			char_t szActive[32]{};
			snprintf(szActive, sizeof(szActive), "  <== tick %d/%d",
				iActiveTick + 1, evt.hit.iRepeatCount);
			strcat_s(szLabel, szActive);
		}

		/* Imported rows are replaced wholesale on the next import, so they are
		worth telling apart from hand-authored ones. */
		if (evt.bImported)
			strcat_s(szLabel, "  *orig");

		if (ImGui::Selectable(szLabel, i == m_iSelectedEvent))
		{
			m_iSelectedEvent = i;
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrentIndex, static_cast<f32_t>(iStartFrame));
			/* Prime the payload editor with the row just opened. */
			strncpy_s(m_PayloadEdit, evt.sPayload.c_str(), _TRUNCATE);
		}

		if (bActive)
			ImGui::PopStyleColor();

		if (i == m_iSelectedEvent)
		{
			const int32_t iMaxFrame = bHasTrack ? static_cast<int32_t>(fDuration) : 0;

			/* Frames are the editing unit but milliseconds are what is stored, so
			only a frame the user actually moved is written back. Converting both
			ways every frame would round the stored value away. */
			if (bWindow || bEffectCueWindow)
			{
				ImGui::SetNextItemWidth(200.f);
				if (ImGui::DragIntRange2("start / end", &iStartFrame, &iEndFrame,
					0.2f, 0, iMaxFrame, "%d", "%d"))
				{
					evt.iStartMs = Frame_To_Ms(iStartFrame, fRate);
					evt.iEndMs = Frame_To_Ms(iEndFrame, fRate);
					m_bDirty = true;
				}
			}
			else
			{
				ImGui::SetNextItemWidth(120.f);
				if (ImGui::DragInt("frame", &iStartFrame, 0.2f, 0, iMaxFrame))
				{
					evt.iStartMs = Frame_To_Ms(iStartFrame, fRate);
					evt.iEndMs = evt.iStartMs;
					m_bDirty = true;
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Remove"))
				iRemoveIndex = i;

			ImGui::SameLine();
			ImGui::TextDisabled("%d - %d ms", evt.iStartMs, evt.iEndMs);

			if (bHit)
				Render_HitDetail(evt);
			else
			{
				const char_t* pHint = "effect / particle key";
				if (EVENT_KIND::SOUND == evt.eKind)
					pHint = "sound cue key";
				else if (bWindow)
					pHint = "what this window is for";

				ImGui::SetNextItemWidth(-1.f);
				if (ImGui::InputTextWithHint("##payload", pHint,
					m_PayloadEdit, sizeof(m_PayloadEdit)))
				{
					evt.sPayload = m_PayloadEdit;
					m_bDirty = true;
				}
				if (EVENT_KIND::EFFECT == evt.eKind)
				{
					const bool_t bAsset =
						EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
							evt.eEffectReferenceKind;
					if (ImGui::RadioButton("Source Reference", !bAsset))
					{
						evt.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
						m_bDirty = true;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Admitted EffectAssetId", bAsset))
					{
						evt.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID;
						const vector<string> IDs =
							CEffectCatalog::Get_EffectAssetIds();
						if (!IDs.empty() &&
							!CEffectCatalog::Contains(evt.sPayload))
						{
							evt.sPayload = IDs.front();
							strncpy_s(m_PayloadEdit,
								evt.sPayload.c_str(), _TRUNCATE);
						}
						m_bDirty = true;
					}
					if (EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
						evt.eEffectReferenceKind)
					{
						const vector<string> IDs =
							CEffectCatalog::Get_EffectAssetIds();
						if (ImGui::BeginCombo("Effect Asset",
							evt.sPayload.empty() ? "Select admitted Effect" :
								evt.sPayload.c_str()))
						{
							for (const string& ID : IDs)
							{
								if (ImGui::Selectable(ID.c_str(),
									ID == evt.sPayload))
								{
									evt.sPayload = ID;
									strncpy_s(m_PayloadEdit,
										ID.c_str(), _TRUNCATE);
									m_bDirty = true;
								}
							}
							ImGui::EndCombo();
						}
						char_t anchor[129]{};
						strncpy_s(anchor, evt.sAnchorSlotId.c_str(), _TRUNCATE);
						if (ImGui::InputText("Anchor (root or bone)",
							anchor, sizeof(anchor)))
						{
							evt.sAnchorSlotId = anchor;
							m_bDirty = true;
						}
						int32_t follow = ETOI(evt.eFollowPolicy);
						if (ImGui::RadioButton("Follow", 0 == follow))
						{
							evt.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
							m_bDirty = true;
						}
						ImGui::SameLine();
						if (ImGui::RadioButton("Snapshot", 1 == follow))
						{
							evt.eFollowPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
							m_bDirty = true;
						}
						int32_t orientation = ETOI(evt.eOrientationPolicy);
						if (ImGui::RadioButton(
							"Anchor Orientation", 0 == orientation))
						{
							evt.eOrientationPolicy =
								EFFECT_ORIENTATION_POLICY::ANCHOR;
							m_bDirty = true;
						}
						ImGui::SameLine();
						if (ImGui::RadioButton(
							"Action Facing", 1 == orientation))
						{
							evt.eOrientationPolicy =
								EFFECT_ORIENTATION_POLICY::ACTION_FACING;
							m_bDirty = true;
						}
						int32_t stop = ETOI(evt.eStopPolicy);
						if (ImGui::RadioButton("Natural Stop", 0 == stop))
						{
							evt.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
							evt.iEndMs = evt.iStartMs;
							m_bDirty = true;
						}
						ImGui::SameLine();
						if (ImGui::RadioButton("Cue End Stop", 1 == stop))
						{
							evt.eStopPolicy = EFFECT_STOP_POLICY::CUE_END;
							if (evt.iEndMs <= evt.iStartMs)
								evt.iEndMs = evt.iStartMs + 100;
							m_bDirty = true;
						}
						m_bDirty |= ImGui::InputFloat3("Effect Local Position",
							&evt.EffectLocalTransform.vPosition.x, "%.3f");
						m_bDirty |= ImGui::InputFloat3("Effect Local Rotation",
							&evt.EffectLocalTransform.vRotationDegrees.x, "%.3f");
						m_bDirty |= ImGui::InputFloat3("Effect Local Scale",
							&evt.EffectLocalTransform.vScale.x, "%.3f");
					}
				}
			}
		}

		ImGui::PopID();
	}

	if (0 == iShown)
		ImGui::TextUnformatted("No event on this clip.");

	ImGui::EndChild();

	if (iRemoveIndex >= 0)
	{
		m_Events.erase(m_Events.begin() + iRemoveIndex);
		m_iSelectedEvent = -1;
		m_bDirty = true;
	}
}

/* The combat half of a HIT. Values here come straight from the extracted game
tables, so they stay in the source units (milliseconds and raw game distance)
rather than being normalised into frames or metres. */
void Client::CAnimation_Tool::Render_HitDetail(ANIM_EVENT& evt)
{
	HIT_PARAMS& p = evt.hit;

	if (ImGui::TreeNodeEx("##hitdetail", ImGuiTreeNodeFlags_DefaultOpen, "hit detail"))
	{
		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("repeat", &p.iRepeatCount, 0.1f, 1, 64))
		{
			if (p.iRepeatCount < 1)
				p.iRepeatCount = 1;
			m_bDirty = true;
		}

		if (p.iRepeatCount > 1)
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("every (ms)", &p.iRepeatMs, 1.f, 0, 5000))
			{
				if (p.iRepeatMs < 0)
					p.iRepeatMs = 0;
				m_bDirty = true;
			}

			/* Spell the hit times out so they can be checked against the pose. */
			std::string ticks;
			for (int32_t t = 0; t < p.iRepeatCount && t < 16; ++t)
			{
				if (!ticks.empty())
					ticks += ", ";
				ticks += std::to_string(Get_TickMs(evt, t));
			}
			if (p.iRepeatCount > 16)
				ticks += ", ...";
			ImGui::TextWrapped("hits at: %s ms", ticks.c_str());
		}

		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("freeze (ms)", &p.iFreezeMs, 1.f, 0, 5000))
			m_bDirty = true;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.f);
		if (ImGui::DragInt("in", &p.iFreezeInMs, 1.f, 0, 5000))
			m_bDirty = true;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.f);
		if (ImGui::DragInt("out", &p.iFreezeOutMs, 1.f, 0, 5000))
			m_bDirty = true;

		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("push (ms)", &p.iPushMs, 1.f, -5000, 5000))
			m_bDirty = true;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("distance", &p.iPushRange, 1.f, -5000, 5000))
			m_bDirty = true;
		if (p.iPushMs < 0)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(pull)");
		}

		ImGui::SetNextItemWidth(110.f);
		if (ImGui::SliderInt("area", &p.iAreaType, 0, 3, Area_Name(p.iAreaType)))
			m_bDirty = true;

		if (p.iAreaType > 0)
		{
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("range", &p.iAreaRange, 1.f, 0, 10000))
				m_bDirty = true;

			/* The official AreaAngle column is the box width in cm and the fan
			sweep in degrees; a circle leaves it at 0. */
			if (1 != p.iAreaType)
			{
				ImGui::SameLine();
				ImGui::SetNextItemWidth(110.f);
				if (ImGui::DragInt(2 == p.iAreaType ? "width" : "angle",
					&p.iAreaAngle, 1.f, 0, 2 == p.iAreaType ? 10000 : 720))
				{
					m_bDirty = true;
				}
			}

			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("height", &p.iAreaHeight, 1.f, 0, 10000))
				m_bDirty = true;
			ImGui::SameLine();
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("offset X", &p.iAreaOffsetX, 1.f, -10000, 10000))
				m_bDirty = true;

			if (2 != p.iAreaType)
			{
				ImGui::SetNextItemWidth(110.f);
				if (ImGui::DragInt("inner", &p.iAreaInner, 1.f, 0, 10000))
					m_bDirty = true;
			}

			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("max targets", &p.iMaxTargets, 0.1f, 0, 64))
				m_bDirty = true;
		}

		ImGui::TreePop();
	}
}

void Client::CAnimation_Tool::Render_HitAreaWires(
	const shared_ptr<Engine::CModel>& pModel) const
{
	if (!m_bShowHitAreas || nullptr == pModel)
		return;
	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	if (nullptr == pCurrentName)
		return;
	float4x4_t Root{};
	if (!CAnimationTargetService::Resolve_RootTransform(&Root))
		return;

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const bool_t bHasTrack = pModel->Get_AnimationProgress(
		iCurrentIndex, fPosition, fDuration) && fDuration > 0.f;
	const f32_t fRate = Get_ClipTickRate(pModel, pCurrentName);
	const int32_t iNowMs = Frame_To_Ms(static_cast<int32_t>(fPosition), fRate);

	auto Draw_Area = [&](const HIT_PARAMS& p, ImU32 iColor)
	{
		CHitAreaWire::Draw(Root, { p.iAreaType, p.iAreaRange, p.iAreaAngle,
			p.iAreaHeight, p.iAreaOffsetX, p.iAreaInner }, iColor);
	};

	for (int32_t i = 0; i < static_cast<int32_t>(m_Events.size()); ++i)
	{
		const ANIM_EVENT& evt = m_Events[i];
		if (evt.clipName != pCurrentName || EVENT_KIND::HIT != evt.eKind ||
			evt.hit.iAreaType <= 0)
			continue;
		const bool_t bActive = bHasTrack && Get_ActiveTick(evt, iNowMs) >= 0;
		const bool_t bSelected = i == m_iSelectedEvent;
		if (!bActive && !bSelected)
			continue;
		Draw_Area(evt.hit, bActive ?
			IM_COL32(255, 70, 60, 255) : IM_COL32(255, 210, 90, 170));
	}

	if (m_iRefWireHitIndex >= 0)
	{
		for (const SKILL_TIMING& row : m_SkillRef)
		{
			if (row.iSkillId != m_iRefWireSkillId)
				continue;
			if (m_iRefWireHitIndex < static_cast<int32_t>(row.hits.size()))
				Draw_Area(row.hits[m_iRefWireHitIndex].hit,
					IM_COL32(80, 200, 255, 255));
			break;
		}
	}
}

void Client::CAnimation_Tool::Render_AnimationList(const shared_ptr<Engine::CModel>& pModel)
{
	/* A zero height means "every pixel left in the window", which let the list
	   swallow the panel and the mouse wheel with it: the sections above it
	   could not be reached again without dragging the scrollbar. A fixed height
	   keeps the wheel over the list scrolling the list and the wheel anywhere
	   else scrolling the tool window. */
	constexpr f32_t CLIP_LIST_HEIGHT = 280.f;
	if (!ImGui::BeginChild(
		"##cliplist", ImVec2(0.f, CLIP_LIST_HEIGHT),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		return;
	}

	const uint32_t iNumAnimations = pModel->Get_NumAnimations();
	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	uint32_t iShown = 0;

	if (!m_bDuplicateScanDone)
	{
		m_bDuplicateScanDone = true;
		std::unordered_set<std::string> names;
		names.reserve(iNumAnimations);
		for (uint32_t i = 0; i < iNumAnimations; ++i)
		{
			const char_t* pName = pModel->Get_AnimationName(i);
			if (nullptr != pName)
				names.insert(pName);
		}
		for (const std::string& name : names)
		{
			if (0 == name.rfind("mesh_", 0))
				continue;
			if (0 < names.count("mesh_" + name))
				m_DuplicateBodyClips.insert(name);
		}
	}

	if (!m_DuplicateBodyClips.empty())
		ImGui::TextDisabled("%zu body clips hidden (same motion listed as mesh_*)",
			m_DuplicateBodyClips.size());

	for (uint32_t i = 0; i < iNumAnimations; ++i)
	{
		const char_t* pName = pModel->Get_AnimationName(i);
		if (nullptr == pName)
			continue;

		if (0 < m_DuplicateBodyClips.count(pName))
			continue;

		/* The clip names are English and the skill names Korean, so the filter has
		to look at both or a Korean search finds nothing. */
		const CLIP_INFO* pInfo = Find_ClipInfo(pName);
		if (!Contains_NoCase(pName, m_Filter) &&
			(nullptr == pInfo || !Contains_NoCase(pInfo->name.c_str(), m_Filter)))
			continue;

		++iShown;

		/* Mark clips that already carry events so authored work is easy to find. */
		int32_t iEventCount = 0;
		for (const ANIM_EVENT& evt : m_Events)
		{
			if (evt.clipName == pName)
				++iEventCount;
		}

		char_t szLabel[MAX_PATH + 160]{};
		snprintf(szLabel, sizeof(szLabel), "%3u  %s", i, pName);

		if (nullptr != pInfo)
		{
			char_t szSkill[128]{};
			snprintf(szSkill, sizeof(szSkill), "   %s (%d)",
				pInfo->name.c_str(), pInfo->iSkillId);
			strcat_s(szLabel, szSkill);
		}

		const auto chain = m_ClipChainCounts.find(pName);
		if (m_ClipChainCounts.end() != chain)
		{
			char_t szChain[24]{};
			snprintf(szChain, sizeof(szChain), "  <chain x%d>", chain->second);
			strcat_s(szLabel, szChain);
		}

		if (iEventCount > 0)
		{
			char_t szCount[16]{};
			snprintf(szCount, sizeof(szCount), "  [%d]", iEventCount);
			strcat_s(szLabel, szCount);
		}

		if (ImGui::Selectable(szLabel, i == iCurrentIndex))
		{
			/* Always show a newly picked clip from its start, but keep the pause state. */
			pModel->Set_Animation(i, m_bLoop);
			pModel->Set_AnimTrackPosition(i, 0.f);
			m_iSelectedEvent = -1;
		}
	}

	if (0 == iShown)
		ImGui::TextUnformatted("No clip matches the filter.");

	ImGui::EndChild();
}

bool_t Client::CAnimation_Tool::Write_EventsToPath(
	const std::filesystem::path& path,
	const std::vector<ANIM_EVENT>& events,
	std::string& outStatus) const
{
	outStatus.clear();

	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, path.c_str(), L"wb") || nullptr == file)
	{
		outStatus = "Could not open temporary Animation Events file.";
		return false;
	}

	bool_t writeSucceeded = 0 <= fprintf(
		file,
		"%s %d \"%s\" %d\n",
		EVENT_FILE_MAGIC,
		EVENT_FILE_VERSION,
		m_AssetName.c_str(),
		static_cast<int32_t>(events.size()));

	for (const ANIM_EVENT& event : events)
	{
		const bool_t bAdmittedEffect =
			EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind;
		const char_t* kindName = Kind_Name(event.eKind);
		if (EVENT_KIND::HIT == event.eKind)
		{
			const HIT_PARAMS& hit = event.hit;
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d endms=%d rep=%d repms=%d "
				"fz=%d fzin=%d fzout=%d push=%d pushr=%d "
				"area=%d ar=%d aa=%d ah=%d ax=%d arem=%d maxt=%d%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.iEndMs,
				hit.iRepeatCount, hit.iRepeatMs,
				hit.iFreezeMs, hit.iFreezeInMs, hit.iFreezeOutMs,
				hit.iPushMs, hit.iPushRange,
				hit.iAreaType, hit.iAreaRange, hit.iAreaAngle,
				hit.iAreaHeight, hit.iAreaOffsetX, hit.iAreaInner,
				hit.iMaxTargets,
				event.bImported ? " src=orig" : "");
		}
		else if (Is_Window(event.eKind))
		{
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d endms=%d payload=\"%s\"%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.iEndMs, event.sPayload.c_str(),
				event.bImported ? " src=orig" : "");
		}
		else if (EVENT_KIND::EFFECT == event.eKind)
		{
			if (EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind)
			{
				writeSucceeded = writeSucceeded && 0 <= fprintf(
					file,
					"\"%s\" %s startms=%d endms=%d payload=\"%s\" "
					"effectref=asset anchor=\"%s\" follow=%s orientation=%s stop=%s "
					"px=%.9g py=%.9g pz=%.9g rx=%.9g ry=%.9g rz=%.9g "
					"sx=%.9g sy=%.9g sz=%.9g%s\n",
					event.clipName.c_str(), kindName,
					event.iStartMs, event.iEndMs,
					event.sPayload.c_str(), event.sAnchorSlotId.c_str(),
					EFFECT_FOLLOW_POLICY::FOLLOW == event.eFollowPolicy ?
						"follow" : "snapshot",
					EFFECT_ORIENTATION_POLICY::ANCHOR ==
						event.eOrientationPolicy ? "anchor" : "action_facing",
					EFFECT_STOP_POLICY::NATURAL == event.eStopPolicy ?
						"natural" : "cue_end",
					event.EffectLocalTransform.vPosition.x,
					event.EffectLocalTransform.vPosition.y,
					event.EffectLocalTransform.vPosition.z,
					event.EffectLocalTransform.vRotationDegrees.x,
					event.EffectLocalTransform.vRotationDegrees.y,
					event.EffectLocalTransform.vRotationDegrees.z,
					event.EffectLocalTransform.vScale.x,
					event.EffectLocalTransform.vScale.y,
					event.EffectLocalTransform.vScale.z,
					event.bImported ? " src=orig" : "");
			}
			else
			{
				writeSucceeded = writeSucceeded && 0 <= fprintf(
					file,
					"\"%s\" %s startms=%d payload=\"%s\" "
					"effectref=source%s\n",
					event.clipName.c_str(), kindName,
					event.iStartMs, event.sPayload.c_str(),
					event.bImported ? " src=orig" : "");
			}
		}
		else
		{
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d payload=\"%s\"%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.sPayload.c_str(),
				event.bImported ? " src=orig" : "");
		}
	}

	const bool_t flushSucceeded = 0 == fflush(file);
	const bool_t commitSucceeded =
		flushSucceeded && 0 == _commit(_fileno(file));
	const bool_t closeSucceeded = 0 == fclose(file);
	if (!writeSucceeded || !flushSucceeded ||
		!commitSucceeded || !closeSucceeded)
	{
		outStatus = "Could not flush Animation Events temporary file.";
		return false;
	}

	return true;
}

bool_t Client::CAnimation_Tool::Validate_Events(
	const shared_ptr<Engine::CModel>& pModel,
	const std::vector<ANIM_EVENT>& events,
	std::string& outStatus) const
{
	outStatus.clear();
	if (nullptr == pModel || m_AssetName.empty() ||
		events.size() > static_cast<size_t>(MAX_EVENT_COUNT))
	{
		outStatus = "Animation event document header is invalid.";
		return false;
	}

	for (const ANIM_EVENT& event : events)
	{
		const bool_t bAdmittedEffect =
			EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind;
		bool_t hasClip = false;
		for (uint32_t index = 0;
			index < pModel->Get_NumAnimations();
			++index)
		{
			const char_t* clipName =
				pModel->Get_AnimationName(index);
			if (nullptr != clipName && event.clipName == clipName)
			{
				hasClip = true;
				break;
			}
		}

		if (!hasClip ||
			!Is_SafeQuotedText(event.clipName) ||
			!Is_SafeQuotedText(event.sPayload) ||
			event.iStartMs < 0 ||
			event.iEndMs < event.iStartMs ||
			(!Is_Window(event.eKind) &&
				event.iEndMs != event.iStartMs &&
				!(bAdmittedEffect &&
					EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy)) ||
			ETOI(event.eKind) < 0 ||
			ETOI(event.eKind) >= ETOI(EVENT_KIND::END))
		{
			outStatus =
				"Animation event clip, time, kind, or payload is invalid.";
			return false;
		}

		if (EVENT_KIND::HIT == event.eKind &&
			(event.hit.iRepeatCount < 1 ||
				event.hit.iRepeatCount > 1024 ||
				event.hit.iRepeatMs < 0))
		{
			outStatus = "Animation HIT repeat contract is invalid.";
			return false;
		}

		if (EVENT_KIND::EFFECT == event.eKind)
		{
			if (EFFECT_REFERENCE_KIND::NONE ==
				event.eEffectReferenceKind)
			{
				outStatus =
					"Animation EFFECT row has no reference kind.";
				return false;
			}
			if (EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind)
			{
				const EFFECT_TRANSFORM_DESC& local =
					event.EffectLocalTransform;
				if (!CEffectCatalog::Contains(event.sPayload) ||
					event.sAnchorSlotId.empty() ||
					!Is_SafeQuotedText(event.sAnchorSlotId) ||
					EFFECT_FOLLOW_POLICY::END == event.eFollowPolicy ||
					EFFECT_ORIENTATION_POLICY::END ==
						event.eOrientationPolicy ||
					(EFFECT_ORIENTATION_POLICY::ACTION_FACING ==
						event.eOrientationPolicy &&
						"root" != event.sAnchorSlotId) ||
					EFFECT_STOP_POLICY::END == event.eStopPolicy ||
					!std::isfinite(local.vPosition.x) ||
					!std::isfinite(local.vPosition.y) ||
					!std::isfinite(local.vPosition.z) ||
					!std::isfinite(local.vRotationDegrees.x) ||
					!std::isfinite(local.vRotationDegrees.y) ||
					!std::isfinite(local.vRotationDegrees.z) ||
					!std::isfinite(local.vScale.x) ||
					!std::isfinite(local.vScale.y) ||
					!std::isfinite(local.vScale.z) ||
					local.vScale.x <= 0.f || local.vScale.y <= 0.f ||
					local.vScale.z <= 0.f ||
					(EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy &&
						event.iEndMs <= event.iStartMs))
				{
					outStatus =
						"Admitted Effect cue ID, anchor, policy, or local transform is invalid.";
					return false;
				}
			}
		}
		else if (EFFECT_REFERENCE_KIND::NONE !=
			event.eEffectReferenceKind)
		{
			outStatus =
				"Only Animation EFFECT rows may carry an effect reference.";
			return false;
		}
	}

	return true;
}

bool_t Client::CAnimation_Tool::Events_AreEqual(
	const std::vector<ANIM_EVENT>& left,
	const std::vector<ANIM_EVENT>& right) const
{
	if (left.size() != right.size())
		return false;

	const auto hitsAreEqual = [](const HIT_PARAMS& a, const HIT_PARAMS& b)
	{
		return a.iRepeatCount == b.iRepeatCount &&
			a.iRepeatMs == b.iRepeatMs &&
			a.iFreezeMs == b.iFreezeMs &&
			a.iFreezeInMs == b.iFreezeInMs &&
			a.iFreezeOutMs == b.iFreezeOutMs &&
			a.iPushMs == b.iPushMs &&
			a.iPushRange == b.iPushRange &&
			a.iAreaType == b.iAreaType &&
			a.iAreaRange == b.iAreaRange &&
			a.iAreaAngle == b.iAreaAngle &&
			a.iAreaHeight == b.iAreaHeight &&
			a.iAreaOffsetX == b.iAreaOffsetX &&
			a.iAreaInner == b.iAreaInner &&
			a.iMaxTargets == b.iMaxTargets;
	};
	const auto effectTransformsAreEqual = [](
		const EFFECT_TRANSFORM_DESC& a,
		const EFFECT_TRANSFORM_DESC& b)
	{
		return a.vPosition.x == b.vPosition.x &&
			a.vPosition.y == b.vPosition.y &&
			a.vPosition.z == b.vPosition.z &&
			a.vRotationDegrees.x == b.vRotationDegrees.x &&
			a.vRotationDegrees.y == b.vRotationDegrees.y &&
			a.vRotationDegrees.z == b.vRotationDegrees.z &&
			a.vScale.x == b.vScale.x &&
			a.vScale.y == b.vScale.y &&
			a.vScale.z == b.vScale.z;
	};

	for (size_t index = 0; index < left.size(); ++index)
	{
		const ANIM_EVENT& a = left[index];
		const ANIM_EVENT& b = right[index];
		if (a.clipName != b.clipName ||
			a.eKind != b.eKind ||
			a.iStartMs != b.iStartMs ||
			a.iEndMs != b.iEndMs ||
			a.sPayload != b.sPayload ||
			a.eEffectReferenceKind != b.eEffectReferenceKind ||
			a.sAnchorSlotId != b.sAnchorSlotId ||
			a.eFollowPolicy != b.eFollowPolicy ||
			a.eOrientationPolicy != b.eOrientationPolicy ||
			a.eStopPolicy != b.eStopPolicy ||
			!effectTransformsAreEqual(a.EffectLocalTransform,
				b.EffectLocalTransform) ||
			a.bImported != b.bImported ||
			!hitsAreEqual(a.hit, b.hit))
		{
			return false;
		}
	}
	return true;
}

bool_t Client::CAnimation_Tool::Load_EventsFromPath(
	const std::filesystem::path& path,
	const shared_ptr<Engine::CModel>& pModel,
	std::vector<ANIM_EVENT>& outEvents,
	int32_t& outSourceVersion,
	std::string& outStatus) const
{
	outEvents.clear();
	outSourceVersion = {};
	outStatus.clear();

	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, path.c_str(), L"rb") || nullptr == file)
	{
		outStatus = "No event file: " + path.string();
		return false;
	}

	int32_t lineNumber = 1;
	const auto fail = [&](
		const std::string& message) -> bool_t
	{
		fclose(file);
		outStatus = message + " [line " +
			std::to_string(lineNumber) + "]";
		return false;
	};

	char_t line[4096]{};
	if (nullptr == fgets(line, sizeof(line), file))
		return fail("Animation event file is empty.");

	const char_t* cursor = line;
	std::string magic;
	std::string versionToken;
	std::string owner;
	std::string countToken;
	int32_t version = {};
	int32_t declaredCount = {};
	if (!Read_Token(cursor, magic) ||
		!Read_Token(cursor, versionToken) ||
		!Read_Quoted(cursor, owner) ||
		!Read_Token(cursor, countToken) ||
		Has_RemainingToken(cursor) ||
		EVENT_FILE_MAGIC != magic ||
		!Parse_Integer(versionToken, version) ||
		!Parse_Integer(countToken, declaredCount) ||
		version < 1 || version > EVENT_FILE_VERSION ||
		declaredCount < 0 || declaredCount > MAX_EVENT_COUNT ||
		owner != m_AssetName)
	{
		return fail("Animation event header is invalid.");
	}

	std::vector<ANIM_EVENT> staged;
	staged.reserve(static_cast<size_t>(declaredCount));

	while (nullptr != fgets(line, sizeof(line), file))
	{
		++lineNumber;
		if (staged.size() >= static_cast<size_t>(declaredCount))
			return fail("Animation event row count exceeds the header.");

		const size_t lineLength = strlen(line);
		if (lineLength == sizeof(line) - 1u &&
			'\n' != line[lineLength - 1u] && !feof(file))
		{
			return fail("Animation event row is too long.");
		}

		cursor = line;
		std::string clipName;
		std::string kindName;
		if (!Read_Quoted(cursor, clipName) ||
			!Read_Token(cursor, kindName))
		{
			return fail("Malformed Animation event row.");
		}

		ANIM_EVENT event{};
		event.clipName = clipName;
		if ("HIT" == kindName)
			event.eKind = EVENT_KIND::HIT;
		else if ("CANCEL" == kindName)
			event.eKind = EVENT_KIND::CANCEL;
		else if ("SUPERARMOR" == kindName)
			event.eKind = EVENT_KIND::SUPERARMOR;
		else if ("INVULN" == kindName)
			event.eKind = EVENT_KIND::INVULN;
		else if ("MOVE" == kindName)
			event.eKind = EVENT_KIND::MOVE;
		else if ("SOUND" == kindName)
			event.eKind = EVENT_KIND::SOUND;
		else if ("EFFECT" == kindName)
		{
			event.eKind = EVENT_KIND::EFFECT;
			if (version < 4)
			{
				event.eEffectReferenceKind =
					EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
			}
		}
		else if ("SHAKE" == kindName)
			event.eKind = EVENT_KIND::SHAKE;
		else if ("COUNTER" == kindName)
			event.eKind = EVENT_KIND::COUNTER;
		else if ("STAGE" == kindName)
			event.eKind = EVENT_KIND::STAGE;
		else
			return fail("Unknown Animation event kind.");

		int32_t startFrame = {};
		int32_t endFrame = {};
		int32_t intervalFrame = {};
		bool_t hasStartFrame = false;
		bool_t hasEndFrame = false;
		bool_t hasStartMs = false;
		bool_t hasEndMs = false;
		bool_t hasEffectReference = false;
		bool_t hasOrientation = false;

		if (1 == version)
		{
			std::string startToken;
			std::string endToken;
			if (EVENT_KIND::HIT != event.eKind ||
				!Read_Token(cursor, startToken) ||
				!Read_Token(cursor, endToken) ||
				Has_RemainingToken(cursor) ||
				!Parse_Integer(startToken, startFrame) ||
				!Parse_Integer(endToken, endFrame))
			{
				return fail("Invalid v1 Animation HIT row.");
			}
			hasStartFrame = true;
			hasEndFrame = true;
		}
		else
		{
			std::unordered_set<std::string> keys;
			while (Has_RemainingToken(cursor))
			{
				std::string key;
				std::string value;
				if (!Read_Pair(cursor, key, value) ||
					!keys.insert(key).second)
				{
					return fail(
						"Malformed or duplicate Animation event field.");
				}

				if ("payload" == key)
				{
					event.sPayload = value;
					continue;
				}
				if ("src" == key)
				{
					if ("orig" != value)
						return fail("Unknown Animation event source.");
					event.bImported = true;
					continue;
				}
				if ("effectref" == key)
				{
					if (version < 4 ||
						EVENT_KIND::EFFECT != event.eKind)
					{
						return fail(
							"effectref is only valid for v4-v6 EFFECT rows.");
					}
					if ("source" == value)
					{
						event.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
					}
					else if ("asset-id" == value || "asset" == value)
					{
						event.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID;
					}
					else
						return fail("Unknown Animation effect reference kind.");
					hasEffectReference = true;
					continue;
				}
				if ("anchor" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("anchor is only valid for v5/v6 EFFECT rows.");
					event.sAnchorSlotId = value;
					continue;
				}
				if ("follow" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("follow is only valid for v5/v6 EFFECT rows.");
					if ("follow" == value)
						event.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
					else if ("snapshot" == value)
						event.eFollowPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
					else
						return fail("Unknown Effect follow policy.");
					continue;
				}
				if ("orientation" == key)
				{
					if (version < 6 || EVENT_KIND::EFFECT != event.eKind)
					{
						return fail(
							"orientation is only valid for v6 EFFECT rows.");
					}
					if ("anchor" == value)
					{
						event.eOrientationPolicy =
							EFFECT_ORIENTATION_POLICY::ANCHOR;
					}
					else if ("action_facing" == value)
					{
						event.eOrientationPolicy =
							EFFECT_ORIENTATION_POLICY::ACTION_FACING;
					}
					else
						return fail("Unknown Effect orientation policy.");
					hasOrientation = true;
					continue;
				}
				if ("stop" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("stop is only valid for v5/v6 EFFECT rows.");
					if ("natural" == value)
						event.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
					else if ("cue_end" == value)
						event.eStopPolicy = EFFECT_STOP_POLICY::CUE_END;
					else
						return fail("Unknown Effect stop policy.");
					continue;
				}
				if (key == "px" || key == "py" || key == "pz" ||
					key == "rx" || key == "ry" || key == "rz" ||
					key == "sx" || key == "sy" || key == "sz")
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("Effect local transform is only valid in v5/v6.");
					f32_t number = 0.f;
					if (!Parse_Float(value, number))
						return fail("Animation Effect transform number is invalid.");
					if ("px" == key) event.EffectLocalTransform.vPosition.x = number;
					else if ("py" == key) event.EffectLocalTransform.vPosition.y = number;
					else if ("pz" == key) event.EffectLocalTransform.vPosition.z = number;
					else if ("rx" == key) event.EffectLocalTransform.vRotationDegrees.x = number;
					else if ("ry" == key) event.EffectLocalTransform.vRotationDegrees.y = number;
					else if ("rz" == key) event.EffectLocalTransform.vRotationDegrees.z = number;
					else if ("sx" == key) event.EffectLocalTransform.vScale.x = number;
					else if ("sy" == key) event.EffectLocalTransform.vScale.y = number;
					else if ("sz" == key) event.EffectLocalTransform.vScale.z = number;
					continue;
				}

				int32_t number = {};
				if (!Parse_Integer(value, number))
					return fail("Animation event number is invalid.");

				if ("start" == key)
				{
					startFrame = number;
					hasStartFrame = true;
				}
				else if ("end" == key)
				{
					endFrame = number;
					hasEndFrame = true;
				}
				else if ("count" == key)
					event.hit.iRepeatCount = number;
				else if ("interval" == key)
					intervalFrame = number;
				else if ("startms" == key)
				{
					event.iStartMs = number;
					hasStartMs = true;
				}
				else if ("endms" == key)
				{
					event.iEndMs = number;
					hasEndMs = true;
				}
				else if ("rep" == key)
					event.hit.iRepeatCount = number;
				else if ("repms" == key)
					event.hit.iRepeatMs = number;
				else if ("fz" == key)
					event.hit.iFreezeMs = number;
				else if ("fzin" == key)
					event.hit.iFreezeInMs = number;
				else if ("fzout" == key)
					event.hit.iFreezeOutMs = number;
				else if ("push" == key)
					event.hit.iPushMs = number;
				else if ("pushr" == key)
					event.hit.iPushRange = number;
				else if ("area" == key)
					event.hit.iAreaType = number;
				else if ("ar" == key)
					event.hit.iAreaRange = number;
				else if ("aa" == key)
					event.hit.iAreaAngle = number;
				else if ("ah" == key)
					event.hit.iAreaHeight = number;
				else if ("ax" == key)
					event.hit.iAreaOffsetX = number;
				else if ("arem" == key)
					event.hit.iAreaInner = number;
				else if ("maxt" == key)
					event.hit.iMaxTargets = number;
				else
					return fail("Unknown Animation event field.");
			}
		}

		if (2 == version)
		{
			if (!hasStartFrame ||
				(Is_Window(event.eKind) && !hasEndFrame))
			{
				return fail("v2 Animation event time is missing.");
			}
			if (!hasEndFrame)
				endFrame = startFrame;
		}
		else if (version >= 3)
		{
			if (!hasStartMs ||
				(Is_Window(event.eKind) && !hasEndMs))
			{
				return fail("Animation event millisecond time is missing.");
			}
			if (!hasEndMs)
				event.iEndMs = event.iStartMs;
		}

		if (version < 3)
		{
			const f32_t tickRate =
				Get_ClipTickRate(pModel, event.clipName);
			if (2 == version &&
				intervalFrame > 0 &&
				event.hit.iRepeatCount <= 1)
			{
				event.hit.iRepeatCount =
					(endFrame - startFrame) / intervalFrame + 1;
			}
			if (event.hit.iRepeatCount > 1)
			{
				event.hit.iRepeatMs = intervalFrame > 0 ?
					Frame_To_Ms(intervalFrame, tickRate) :
					Frame_To_Ms(
						endFrame - startFrame,
						tickRate) /
						(event.hit.iRepeatCount - 1);
				endFrame = startFrame;
			}
			event.iStartMs = Frame_To_Ms(startFrame, tickRate);
			event.iEndMs = Frame_To_Ms(endFrame, tickRate);
		}

		if (event.hit.iRepeatCount < 1)
			event.hit.iRepeatCount = 1;
		if (!Is_Window(event.eKind) &&
			!(EVENT_KIND::EFFECT == event.eKind &&
				EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
					event.eEffectReferenceKind &&
				EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy))
			event.iEndMs = event.iStartMs;
		if (3 == version && EVENT_KIND::EFFECT == event.eKind &&
			!hasEffectReference)
		{
			event.eEffectReferenceKind =
				EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
		}
		if (version >= 4 &&
			EVENT_KIND::EFFECT == event.eKind &&
			!hasEffectReference)
		{
			if (event.bImported)
				event.eEffectReferenceKind =
					EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
			else
				return fail("Authored v4-v6 EFFECT row requires effectref.");
		}
		if (hasOrientation &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID !=
				event.eEffectReferenceKind)
		{
			return fail(
				"orientation is only valid for admitted Effect asset rows.");
		}
		if (EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind &&
			EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy &&
			(!hasEndMs || event.iEndMs <= event.iStartMs))
		{
			return fail("cue_end requires endms greater than startms.");
		}

		staged.push_back(std::move(event));
	}

	if (0 != ferror(file))
		return fail("Animation event file read failed.");
	if (0 != fclose(file))
	{
		outStatus = "Animation event file close failed.";
		return false;
	}
	file = nullptr;

	if (staged.size() != static_cast<size_t>(declaredCount))
	{
		outStatus = "Animation event row count does not match the header.";
		return false;
	}

	outEvents = std::move(staged);
	outSourceVersion = version;
	return true;
}

bool_t Client::CAnimation_Tool::Save_Events(
	const shared_ptr<Engine::CModel>& pModel)
{
	std::string validationStatus;
	if (!Validate_Events(pModel, m_Events, validationStatus))
	{
		m_Status = "Save rejected: " + validationStatus;
		return false;
	}

	const std::filesystem::path destination{ Get_EventFilePath() };
	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		m_Status = "Save failed to create authoring directory: " +
			directoryError.message();
		return false;
	}

	std::filesystem::path temporary = destination;
	temporary += L".tmp";
	std::error_code removeError;
	std::filesystem::remove(temporary, removeError);
	if (removeError)
	{
		m_Status = "Save failed to clear stale temporary file: " +
			removeError.message();
		return false;
	}

	std::string transactionStatus;
	if (!Write_EventsToPath(
		temporary, m_Events, transactionStatus))
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_Status = "Save failed; previous file preserved: " +
			transactionStatus;
		return false;
	}

	std::vector<ANIM_EVENT> staged;
	int32_t sourceVersion = {};
	if (!Load_EventsFromPath(
		temporary,
		pModel,
		staged,
		sourceVersion,
		transactionStatus) ||
		EVENT_FILE_VERSION != sourceVersion ||
		!Validate_Events(pModel, staged, transactionStatus) ||
		!Events_AreEqual(staged, m_Events))
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		if (transactionStatus.empty())
		{
			transactionStatus =
				"Animation event round trip changed the document.";
		}
		m_Status = "Save validation failed; previous file preserved: " +
			transactionStatus;
		return false;
	}

	if (!MoveFileExW(
		temporary.c_str(),
		destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		const DWORD errorCode = GetLastError();
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_Status = "Atomic replace failed; previous file preserved. Win32 error " +
			std::to_string(errorCode) + ".";
		return false;
	}

	m_bDirty = false;
	m_Status = "Saved " + std::to_string(m_Events.size()) +
		" event(s) atomically to " + destination.string();
	return true;
}

bool_t Client::CAnimation_Tool::Load_Events(
	const shared_ptr<Engine::CModel>& pModel)
{
	const std::filesystem::path path{ Get_EventFilePath() };
	std::vector<ANIM_EVENT> staged;
	int32_t sourceVersion = {};
	std::string loadStatus;
	if (!Load_EventsFromPath(
		path, pModel, staged, sourceVersion, loadStatus) ||
		!Validate_Events(pModel, staged, loadStatus))
	{
		m_Status =
			"Load rejected; current document preserved: " + loadStatus;
		return false;
	}

	m_Events = std::move(staged);
	m_iSelectedEvent = -1;
	/* v3 already stores milliseconds and can be represented by v4 without moving
	any marker. Loading it must not create a false unsaved document that blocks a
	Character Select target change. Only v1/v2 frame conversion is dirty. */
	m_bDirty = sourceVersion < 3;
	m_Status = "Loaded " + std::to_string(m_Events.size()) +
		" event(s) from " + path.string();
	if (m_bDirty)
	{
		m_Status += " [converted from v" +
			std::to_string(sourceVersion) +
			" in memory; review and Save explicitly]";
	}
	else if (sourceVersion < EVENT_FILE_VERSION)
	{
		m_Status +=
			" [legacy source references preserved; the next authored Save writes v6]";
	}
	return true;
}

void Client::CAnimation_Tool::Render_ReloadConfirmation(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (m_bReloadConfirmationRequested)
	{
		ImGui::OpenPopup("Discard unsaved Animation Events?");
		m_bReloadConfirmationRequested = false;
	}

	if (!ImGui::BeginPopupModal(
		"Discard unsaved Animation Events?",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	ImGui::TextUnformatted(
		"Reload will replace the current unsaved Animation document.");
	if (ImGui::Button("Discard and Reload"))
	{
		if (Load_Events(pModel))
			ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		ImGui::CloseCurrentPopup();
	ImGui::EndPopup();
}

bool_t Client::CAnimation_Tool::Load_ClipMap()
{
	m_ClipMap.clear();

	const std::string path = Get_ClipMapPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[1024]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_CLIP_MAP")
	{
		fclose(pFile);
		return false;
	}

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string clip;
		if (!Read_Quoted(p, clip))
			continue;

		CLIP_INFO info{};
		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("skill" == key)
				info.iSkillId = atoi(value.c_str());
			else if ("name" == key)
				info.name = value;
		}

		m_ClipMap[clip] = info;
	}

	fclose(pFile);
	return true;
}

bool_t Client::CAnimation_Tool::Load_ClipNotify()
{
	m_ClipNotify.clear();

	const std::string path = Get_ClipNotifyPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_ANIM_NOTIFY")
	{
		fclose(pFile);
		return false;
	}

	std::vector<NOTIFY_ROW>* pCurrent = nullptr;

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		/* A clip header opens with a quoted name; its rows start with "n". */
		std::string token;
		const char_t* pProbe = p;
		if (Read_Quoted(pProbe, token))
		{
			pCurrent = &m_ClipNotify[token];

			/* The header also carries the clip's own length, which the chain
			offsets are summed from. */
			std::string hkey, hvalue;
			while (Read_Pair(pProbe, hkey, hvalue))
			{
				if ("len" == hkey)
					m_ClipLength[token] = static_cast<f32_t>(atof(hvalue.c_str()));
			}
			continue;
		}

		if (!Read_Token(p, token) || "n" != token || nullptr == pCurrent)
			continue;

		NOTIFY_ROW row{};
		bool_t bKnown = false;

		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("t" == key)
				row.fTime = static_cast<f32_t>(atof(value.c_str()));
			else if ("d" == key)
				row.fDuration = static_cast<f32_t>(atof(value.c_str()));
			else if ("asset" == key)
				row.sAsset = value;
			else if ("label" == key)
				row.sLabel = value;
			else if ("kind" == key)
			{
				bKnown = true;
				if ("HIT" == value)
					row.eKind = EVENT_KIND::HIT;
				else if ("CANCEL" == value)
					row.eKind = EVENT_KIND::CANCEL;
				else if ("SUPERARMOR" == value)
					row.eKind = EVENT_KIND::SUPERARMOR;
				else if ("COUNTER" == value)
					row.eKind = EVENT_KIND::COUNTER;
				else if ("SOUND" == value)
					row.eKind = EVENT_KIND::SOUND;
				else if ("EFFECT" == value)
					row.eKind = EVENT_KIND::EFFECT;
				else if ("SHAKE" == value)
					row.eKind = EVENT_KIND::SHAKE;
				else if ("STAGE" == value)
					row.eKind = EVENT_KIND::STAGE;
				else
					bKnown = false;
			}
		}

		if (bKnown)
			pCurrent->push_back(row);
	}

	fclose(pFile);
	return true;
}

bool_t Client::CAnimation_Tool::Load_ClipSeq()
{
	m_ClipSeqs.clear();
	m_ClipChainCounts.clear();

	const std::string path = Get_ClipSeqPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_CLIP_SEQ")
	{
		fclose(pFile);
		return false;
	}

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string idToken, name;
		if (!Read_Token(p, idToken) || !Read_Quoted(p, name))
			continue;

		CLIP_SEQ seq{};
		seq.iSkillId = atoi(idToken.c_str());
		seq.name = name;

		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("seq" == key)
				seq.iSeqIndex = atoi(value.c_str());
			else if ("mode" == key)
				seq.sMode = value;
			else if ("clips" == key)
			{
				/* comma separated, in playback order */
				size_t start = 0;
				while (start <= value.size())
				{
					const size_t comma = value.find(',', start);
					const std::string one = value.substr(start,
						std::string::npos == comma ? std::string::npos : comma - start);
					if (!one.empty())
						seq.clips.push_back(one);
					if (std::string::npos == comma)
						break;
					start = comma + 1;
				}
			}
		}

		/* One-clip chains are kept: they are still what the skill casts, and the
		runtime reads the same file. */
		if (!seq.clips.empty())
			m_ClipSeqs.push_back(seq);
	}

	for (const CLIP_SEQ& seq : m_ClipSeqs)
	{
		for (const std::string& clip : seq.clips)
			++m_ClipChainCounts[clip];
	}

	fclose(pFile);
	Load_ClipCuts();
	return true;
}

void Client::CAnimation_Tool::Load_ClipCuts()
{
	const std::string path = CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".clipcuts")).string();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_CLIP_CUTS")
	{
		fclose(pFile);
		return;
	}

	std::map<std::pair<int32_t, int32_t>, std::vector<f32_t>> cutsBySequence;
	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;
		std::string idToken;
		if (!Read_Token(p, idToken))
			continue;

		int32_t iSeqIndex = 0;
		std::vector<f32_t> cuts;
		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("seq" == key)
				iSeqIndex = atoi(value.c_str());
			else if ("cuts" == key)
			{
				size_t start = 0;
				while (start <= value.size())
				{
					const size_t comma = value.find(',', start);
					const std::string one = value.substr(start,
						std::string::npos == comma ?
							std::string::npos : comma - start);
					if (!one.empty())
						cuts.push_back(
							static_cast<f32_t>(atof(one.c_str())));
					if (std::string::npos == comma)
						break;
					start = comma + 1;
				}
			}
		}
		if (!cuts.empty())
			cutsBySequence[{ atoi(idToken.c_str()), iSeqIndex }] =
				std::move(cuts);
	}
	fclose(pFile);

	for (CLIP_SEQ& seq : m_ClipSeqs)
	{
		const auto found =
			cutsBySequence.find({ seq.iSkillId, seq.iSeqIndex });
		if (cutsBySequence.end() != found &&
			found->second.size() == seq.clips.size())
		{
			seq.cuts = found->second;
		}
	}
}

int32_t Client::CAnimation_Tool::Import_Notifies(const char_t* pClipName, f32_t fTickRate,
	int32_t& iShapedHits)
{
	iShapedHits = 0;

	const auto it = m_ClipNotify.find(pClipName);
	if (m_ClipNotify.end() == it)
		return 0;

	/* Drop only what a previous import put here so hand-authored rows survive. */
	for (auto e = m_Events.begin(); e != m_Events.end(); )
		e = (e->bImported && e->clipName == pClipName) ? m_Events.erase(e) : e + 1;

	const SKILL_TIMING* pRef = m_bImportKind[ETOI(EVENT_KIND::HIT)]
		? Find_ReferenceRow(pClipName) : nullptr;
	int32_t iHitOrdinal = nullptr != pRef ? Count_PrecedingChainHits(pClipName) : 0;

	int32_t iAdded = 0;
	for (const NOTIFY_ROW& row : it->second)
	{
		if (!m_bImportKind[ETOI(row.eKind)])
			continue;

		ANIM_EVENT evt{};
		evt.clipName = pClipName;
		evt.eKind = row.eKind;
		if (EVENT_KIND::EFFECT == evt.eKind)
			evt.eEffectReferenceKind =
				EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
		evt.bImported = true;
		evt.iStartMs = static_cast<int32_t>(row.fTime * 1000.f + 0.5f);
		evt.iEndMs = Is_Window(row.eKind)
			? evt.iStartMs + static_cast<int32_t>(row.fDuration * 1000.f + 0.5f)
			: evt.iStartMs;
		/* Point kinds name what they fire; a window instead carries the game's
		note on what it is for, which is the only thing distinguishing a move
		cancel from a dodge cancel. */
		evt.sPayload = Is_Window(row.eKind) ? row.sLabel : row.sAsset;

		/* The game spawns the same particle from several attach points; for
		authoring those collapse to one row. */
		bool_t bDuplicate = false;
		for (const ANIM_EVENT& other : m_Events)
		{
			if (other.bImported && other.clipName == evt.clipName &&
				other.eKind == evt.eKind && other.iStartMs == evt.iStartMs &&
				other.iEndMs == evt.iEndMs && other.sPayload == evt.sPayload)
			{
				bDuplicate = true;
				break;
			}
		}
		if (bDuplicate)
			continue;

		if (EVENT_KIND::HIT == evt.eKind && nullptr != pRef)
		{
			const int32_t iLast = static_cast<int32_t>(pRef->hits.size()) - 1;
			const SKILL_HIT& src = pRef->hits[iHitOrdinal < iLast ? iHitOrdinal : iLast];
			++iHitOrdinal;
			if (src.hit.iAreaType > 0)
			{
				evt.hit = src.hit;
				++iShapedHits;
			}
		}

		m_Events.push_back(evt);
		++iAdded;
	}

	m_iSelectedEvent = -1;
	m_bDirty = true;
	return iAdded;
}

const Client::CAnimation_Tool::SKILL_TIMING* Client::CAnimation_Tool::Find_ReferenceRow(
	const char_t* pClipName) const
{
	const CLIP_INFO* pInfo = Find_ClipInfo(pClipName);
	if (nullptr == pInfo || pInfo->iSkillId <= 0)
		return nullptr;

	const SKILL_TIMING* pVariant = nullptr;
	for (const SKILL_TIMING& row : m_SkillRef)
	{
		if (row.hits.empty())
			continue;
		if (row.iSkillId == pInfo->iSkillId)
			return &row;
		if (nullptr == pVariant && row.iBaseSkillId == pInfo->iSkillId)
			pVariant = &row;
	}
	return pVariant;
}

int32_t Client::CAnimation_Tool::Count_PrecedingChainHits(const char_t* pClipName) const
{
	for (const CLIP_SEQ& seq : m_ClipSeqs)
	{
		int32_t iCount = 0;
		for (const std::string& clip : seq.clips)
		{
			if (clip == pClipName)
				return iCount;
			const auto it = m_ClipNotify.find(clip);
			if (m_ClipNotify.end() != it)
				iCount += Count_DistinctHitNotifies(it->second);
		}
	}
	return 0;
}

int32_t Client::CAnimation_Tool::Count_DistinctHitNotifies(const std::vector<NOTIFY_ROW>& rows)
{
	int32_t iCount = 0;
	for (size_t i = 0; i < rows.size(); ++i)
	{
		if (EVENT_KIND::HIT != rows[i].eKind)
			continue;
		bool_t bDuplicate = false;
		for (size_t j = 0; j < i; ++j)
		{
			if (EVENT_KIND::HIT == rows[j].eKind &&
				rows[j].fTime == rows[i].fTime && rows[j].fDuration == rows[i].fDuration &&
				rows[j].sLabel == rows[i].sLabel)
			{
				bDuplicate = true;
				break;
			}
		}
		if (!bDuplicate)
			++iCount;
	}
	return iCount;
}

bool_t Client::CAnimation_Tool::Load_SkillReference()
{
	m_SkillRef.clear();

	const std::string path = Get_SkillReferencePath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_SKILL_TIMING")
	{
		fclose(pFile);
		return false;
	}

	/* Set once the row being read gets a hit line of its own, so the windows read
	off the skill line are only a stand-in for v1 files. */
	bool_t bCurrentHasDetail = false;

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string idToken, name;
		if (!Read_Token(p, idToken))
			continue;

		/* A v2 "hit" continuation line details the skill line above it. v1 files
		have none, and an older build drops them because they carry no quoted
		name where it expects one. */
		if (("hit" == idToken || "shape" == idToken) && !m_SkillRef.empty())
		{
			const bool_t bShape = "shape" == idToken;

			if (!bCurrentHasDetail)
			{
				bCurrentHasDetail = true;
				m_SkillRef.back().hits.clear();
			}

			SKILL_HIT hit{};
			hit.bTimed = !bShape;
			HIT_PARAMS& hp = hit.hit;

			std::string key, value;
			while (Read_Pair(p, key, value))
			{
				const int32_t v = atoi(value.c_str());

				if ("t" == key)
					hit.iTimeMs = v;
				else if ("w" == key)
					hit.iWidthMs = v;
				else if ("timed" == key)
					hit.bTimed = (0 != v);
				else if ("rep" == key)
					hp.iRepeatCount = v;
				else if ("repms" == key)
					hp.iRepeatMs = v;
				else if ("fz" == key)
					hp.iFreezeMs = v;
				else if ("fzin" == key)
					hp.iFreezeInMs = v;
				else if ("fzout" == key)
					hp.iFreezeOutMs = v;
				else if ("push" == key)
					hp.iPushMs = v;
				else if ("pushr" == key)
					hp.iPushRange = v;
				else if ("area" == key)
					hp.iAreaType = v;
				else if ("ar" == key)
					hp.iAreaRange = v;
				else if ("aa" == key)
					hp.iAreaAngle = v;
				else if ("ah" == key)
					hp.iAreaHeight = v;
				else if ("ax" == key)
					hp.iAreaOffsetX = v;
				else if ("arem" == key)
					hp.iAreaInner = v;
				else if ("maxt" == key)
					hp.iMaxTargets = v;
				else if ("pks" == key)
					hit.sSourceKeys = value;
			}

			if (hp.iRepeatCount < 1)
				hp.iRepeatCount = 1;

			if (bShape)
			{
				bool_t bDuplicate = false;
				for (const SKILL_HIT& other : m_SkillRef.back().hits)
				{
					const HIT_PARAMS& op = other.hit;
					if (!other.bTimed &&
						op.iAreaType == hp.iAreaType &&
						op.iAreaRange == hp.iAreaRange &&
						op.iAreaAngle == hp.iAreaAngle &&
						op.iAreaHeight == hp.iAreaHeight &&
						op.iAreaOffsetX == hp.iAreaOffsetX &&
						op.iAreaInner == hp.iAreaInner &&
						op.iMaxTargets == hp.iMaxTargets &&
						op.iRepeatCount == hp.iRepeatCount &&
						op.iRepeatMs == hp.iRepeatMs &&
						op.iFreezeMs == hp.iFreezeMs &&
						op.iFreezeInMs == hp.iFreezeInMs &&
						op.iFreezeOutMs == hp.iFreezeOutMs &&
						op.iPushMs == hp.iPushMs &&
						op.iPushRange == hp.iPushRange)
					{
						bDuplicate = true;
						break;
					}
				}
				if (bDuplicate)
					continue;
			}

			m_SkillRef.back().hits.push_back(hit);
			continue;
		}

		if (!Read_Quoted(p, name))
			continue;

		SKILL_TIMING row{};
		row.iSkillId = atoi(idToken.c_str());
		row.name = name;

		/* v1 has no hit lines, so its windows stand in as bare hits with no combat
		detail. A following hit line replaces them. */
		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("sa" == key)
				row.iSuperArmor = atoi(value.c_str());
			else if ("move" == key)
				row.iMoveSpeed = atoi(value.c_str());
			else if ("base" == key)
				row.iBaseSkillId = atoi(value.c_str());
			else if ("hits" == key)
			{
				/* value is "a-b,c-d": comma-separated second windows. */
				size_t start = 0;
				while (start <= value.size())
				{
					const size_t comma = value.find(',', start);
					const std::string span = value.substr(start,
						std::string::npos == comma ? std::string::npos : comma - start);

					const size_t dash = span.find('-');
					if (std::string::npos != dash)
					{
						const f32_t a = static_cast<f32_t>(atof(span.substr(0, dash).c_str()));
						const f32_t b = static_cast<f32_t>(atof(span.substr(dash + 1).c_str()));

						SKILL_HIT hit{};
						hit.iTimeMs = static_cast<int32_t>(a * 1000.f + 0.5f);
						hit.iWidthMs = static_cast<int32_t>((b - a) * 1000.f + 0.5f);
						row.hits.push_back(hit);
					}

					if (std::string::npos == comma)
						break;
					start = comma + 1;
				}
			}
		}

		m_SkillRef.push_back(row);
		bCurrentHasDetail = false;
	}

	fclose(pFile);
	return true;
}

void Client::CAnimation_Tool::Render_SkillReference(
	const shared_ptr<Engine::CModel>& pModel,
	bool_t bReadOnly)
{
	if (m_SkillRef.empty())
		return;

	/* Its own floating window instead of a header embedded in the main tool, so it
	can be dragged out and placed wherever (e.g. next to the event list). */
	ImGui::SetNextWindowSize(ImVec2(520.f, 420.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(500.f, 60.f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Skill Timing Reference (game-extracted)"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped("Times run from the start of the whole skill cast (all its clips in "
		"sequence). For a mid-cast clip set the offset to where that clip begins in the "
		"cast so a stamp lands on the right local time.");

	ImGui::SetNextItemWidth(160.f);
	ImGui::DragInt("cast offset (ms)", &m_iCastOffsetMs, 5.f, 0, 100000);
	if (m_iCastOffsetMs < 0)
		m_iCastOffsetMs = 0;

	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##reffilter", "filter by id or name", m_RefFilter, sizeof(m_RefFilter));

	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	const f32_t fRate = Get_ClipTickRate(pModel,
		nullptr != pCurrentName ? pCurrentName : "");

	if (!ImGui::BeginChild("##reflist", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		ImGui::End();
		return;
	}

	for (const SKILL_TIMING& row : m_SkillRef)
	{
		char_t szId[32]{};
		snprintf(szId, sizeof(szId), "%d", row.iSkillId);
		if (!Contains_NoCase(szId, m_RefFilter) && !Contains_NoCase(row.name.c_str(), m_RefFilter))
			continue;

		ImGui::PushID(row.iSkillId);

		ImGui::Text("%d  %s", row.iSkillId, row.name.empty() ? "(no name)" : row.name.c_str());
		if (row.iSuperArmor > 0 || row.iMoveSpeed > 0)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("superarmor %d  move %d", row.iSuperArmor, row.iMoveSpeed);
		}

		/* The clips this skill actually plays, so the right one can be opened
		without guessing which English clip name matches the Korean skill. */
		const int32_t iOwner = row.iBaseSkillId > 0 ? row.iBaseSkillId : row.iSkillId;
		int32_t iClipButton = 0;
		for (const auto& entry : m_ClipMap)
		{
			if (entry.second.iSkillId != iOwner)
				continue;

			if (iClipButton > 0 && (iClipButton % 3) != 0)
				ImGui::SameLine();
			++iClipButton;

			ImGui::PushID(iClipButton);
			if (ImGui::SmallButton(entry.first.c_str()))
				Select_Clip(pModel, entry.first);
			ImGui::PopID();
		}

		for (size_t h = 0; h < row.hits.size(); ++h)
		{
			const SKILL_HIT& src = row.hits[h];

			/* Cast time minus where the current clip starts in the cast. */
			int32_t iStartMs = src.iTimeMs - m_iCastOffsetMs;
			if (iStartMs < 0)
				iStartMs = 0;
			const int32_t iEndMs = iStartMs + src.iWidthMs;

			ImGui::PushID(static_cast<int32_t>(h));

			if (bReadOnly)
			{
				if (src.hit.iAreaType > 0)
				{
					const bool_t bWired = row.iSkillId == m_iRefWireSkillId &&
						static_cast<int32_t>(h) == m_iRefWireHitIndex;
					const std::string firstKey = src.sSourceKeys.substr(
						0, src.sSourceKeys.find(','));
					char_t szWire[160]{};
					snprintf(szWire, sizeof(szWire),
						"wire  %s%d  angle %d  offset %d  hole %d  [%s]",
						Area_Name(src.hit.iAreaType), src.hit.iAreaRange,
						src.hit.iAreaAngle, src.hit.iAreaOffsetX,
						src.hit.iAreaInner,
						firstKey.empty() ? "-" : firstKey.c_str());
					if (ImGui::Selectable(szWire, bWired))
					{
						m_iRefWireSkillId = bWired ? 0 : row.iSkillId;
						m_iRefWireHitIndex =
							bWired ? -1 : static_cast<int32_t>(h);
					}
				}
				ImGui::PopID();
				continue;
			}

			char_t szBtn[192]{};
			if (src.bTimed)
				snprintf(szBtn, sizeof(szBtn), "stamp  %d-%d ms  ->  frame %d  %s%d",
					src.iTimeMs, src.iTimeMs + src.iWidthMs,
					Ms_To_Frame(iStartMs, fRate),
					Area_Name(src.hit.iAreaType), src.hit.iAreaRange);
			else
				/* No time in the source data; it lands at the playhead instead and
				only the combat values are worth copying. */
				snprintf(szBtn, sizeof(szBtn), "stamp (untimed)  %s%d  freeze %d",
					Area_Name(src.hit.iAreaType), src.hit.iAreaRange, src.hit.iFreezeMs);

			ImGui::BeginDisabled(nullptr == pCurrentName);
			if (ImGui::Button(szBtn))
			{
				ANIM_EVENT evt{};
				evt.clipName = pCurrentName;
				evt.eKind = EVENT_KIND::HIT;
				evt.hit = src.hit;

				if (src.bTimed)
				{
					evt.iStartMs = iStartMs;
					evt.iEndMs = iEndMs;
				}
				else
				{
					f32_t fPosition = 0.f;
					f32_t fDuration = 0.f;
					pModel->Get_AnimationProgress(iCurrentIndex, fPosition, fDuration);
					evt.iStartMs = Frame_To_Ms(static_cast<int32_t>(fPosition), fRate);
					evt.iEndMs = evt.iStartMs;
				}

				m_Events.push_back(evt);
				m_iSelectedEvent = static_cast<int32_t>(m_Events.size()) - 1;
				m_PayloadEdit[0] = '\0';
				m_bDirty = true;
				m_Status = "Stamped skill " + std::to_string(row.iSkillId) +
					" onto " + pCurrentName;
			}
			ImGui::EndDisabled();

			ImGui::PopID();
		}

		ImGui::Separator();
		ImGui::PopID();
	}

	ImGui::EndChild();
	ImGui::End();
}
