#pragma once

#include "Animation_Tool.h"
#include "ProjectDataRoot.h"

namespace AnimationToolDetail
{

	constexpr const char_t* EVENT_FILE_MAGIC = "LOSTARK_ANIM_EVENTS";
	/* v6 separates position follow/snapshot from anchor/action-facing rotation. */
	constexpr int32_t EVENT_FILE_VERSION = 6;
	constexpr int32_t MAX_EVENT_COUNT = 100000;

	/* Used when a clip carries no usable rate, which would otherwise make the
	frame <-> millisecond conversion divide by zero. */
	constexpr f32_t DEFAULT_TICK_RATE = 30.f;
	/* Large-named Saydon actions preview at the size the running exe admitted
	for the big Saydon boss: the catalog bodyModelPreScale over the MN_RPCT_06
	preview admission scale, so the preview matches the spawned boss. The
	catalog is required; invalid input preserves the current preview scale. */
	f32_t Large_SaydonPreviewMultiplier();


	f32_t Resolve_LargeNamePreviewScale(const std::string_view name);


	f32_t Resolve_ActionPreviewScale(
		const Client::KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
		const std::uint32_t sourceActionId);

	constexpr f32_t WORKBENCH_DEFAULT_WIDTH = 900.f;
	constexpr f32_t WORKBENCH_DEFAULT_HEIGHT = 700.f;
	constexpr f32_t WORKBENCH_FALLBACK_MIN_WIDTH = 320.f;
	constexpr f32_t WORKBENCH_FALLBACK_MIN_HEIGHT = 200.f;
	/* The outer Workbench may contract to a small viewport, but the three-pane
	   authoring surface keeps a commercial-editor-sized logical canvas and
	   scrolls horizontally instead of crushing Outliner and Detail to labels. */
	constexpr f32_t WORKBENCH_THREE_PANE_INNER_WIDTH = 1120.f;
	constexpr f32_t KOUKU_SAYDON_ACTION_LIST_DEFAULT_WIDTH = 320.f;
	constexpr f32_t KOUKU_SAYDON_ACTION_LIST_MIN_WIDTH = 280.f;
	constexpr f32_t KOUKU_SAYDON_ACTION_DETAIL_MIN_WIDTH = 620.f;
	constexpr f32_t KOUKU_SAYDON_ACTION_SPLITTER_WIDTH = 8.f;
	constexpr uint64_t VALTAN_PATTERN_CREATE_TIMEOUT_MILLISECONDS = 120000u;
	constexpr std::uintmax_t VALTAN_PATTERN_CREATE_MAX_DIAGNOSTIC_BYTES =
		2u * 1024u * 1024u;
	constexpr std::array<const char_t*, 3u>
		VALTAN_PATTERN_CREATE_TARGET_POLICIES = {
			"NONE", "LOCK_NEAREST_ON_START", "LOCK_RANDOM_ALIVE_ON_START" };
	constexpr std::array<const char_t*, 2u>
		VALTAN_PATTERN_CREATE_AIM_POLICIES = {
			"NONE", "LOCK_FACING_ON_START" };

	bool_t Is_LowerSha256(const std::string_view strValue);


	bool_t Is_StablePatternAuthoringId(const std::string_view strValue);


	bool_t Read_BoundedFile(
		const std::filesystem::path& Path,
		const std::uintmax_t iMaximumBytes,
		std::string& strOutBytes,
		std::string& strOutError);


	bool_t Resolve_PythonExecutable(
		std::filesystem::path& Out,
		std::string& strOutError);


	std::string_view ValtanSoundBankForEvent(
		const std::string_view strSoundEvent);


	bool_t IsValtanSoundAuthoringCandidate(const std::string& strSoundEvent);


	std::unordered_map<std::string, f32_t>
	CollectModelClipSourceDurationSeconds(
		const shared_ptr<Engine::CModel>& pModel);


	struct VALTAN_NATIVE_CLIP_TIMING final
	{
		f32_t fDurationTicks = 0.f;
		f32_t fTicksPerSecond = 0.f;
	};

	using VALTAN_NATIVE_CLIP_INVENTORY =
		std::unordered_map<std::string, VALTAN_NATIVE_CLIP_TIMING>;

	bool_t BuildStrictValtanNativeClipInventory(
		const std::vector<Client::COMPOSITION_ANIMATION_RESOURCE>& resources,
		VALTAN_NATIVE_CLIP_INVENTORY& Out,
		std::string& Status);


	bool_t SameValtanAnimationAuthoringSignature(
		const Client::VALTAN_STAGE_VIEW& Left,
		const Client::VALTAN_STAGE_VIEW& Right);


	bool_t ValidateValtanStageNativeAnimationWindows(
		const Client::VALTAN_STAGE_VIEW& Stage,
		const VALTAN_NATIVE_CLIP_INVENTORY& Inventory,
		std::string& Status);

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
		const std::string& strAssetName);


	struct KOUKU_SAYDON_ACTION_PROFILE_CONTRACT final
	{
		const char_t* pProfileId;
		const char_t* pCategoryLabel;
		const char_t* pModelPolicy;
		const char_t* pPreviewAssetName;
		const char_t* pModelAssetId;
	};

	constexpr std::array<KOUKU_SAYDON_ACTION_PROFILE_CONTRACT, 4u>
		KOUKU_SAYDON_ACTION_PROFILES = {
			KOUKU_SAYDON_ACTION_PROFILE_CONTRACT{
				"MN_RPCT_05", "Saydon",
				"Dedicated MN_RPCT_05 body, authored scale 1.0x",
				"MN_RPCT_05",
				"Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05" },
			KOUKU_SAYDON_ACTION_PROFILE_CONTRACT{
				"MN_RPCT_06", "Large Saydon",
				"Dedicated MN_RPCT_06 body/skeleton; large-named action preview multiplier 100x",
				"MN_RPCT_06",
				"Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06" },
			KOUKU_SAYDON_ACTION_PROFILE_CONTRACT{
				"MN_RPCT_07", "Kouku + Saydon",
				"Authoring profile alias on the shared MN_RPCT_05 body, scale 1.0x",
				"MN_RPCT_05",
				"Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05" },
			KOUKU_SAYDON_ACTION_PROFILE_CONTRACT{
				"MN_RPCZ_00", "Kouku",
				"Dedicated MN_RPCZ_00 body, authored scale 1.0x",
				"MN_RPCZ_00",
				"Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00" },
		};

	const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* Find_KoukuSaydonActionProfile(
		const std::string_view strProfileId);


	const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* Default_KoukuSaydonActionProfileForAsset(
		const std::string_view strAssetName);


	const Client::VALTAN_PATTERN_VIEW* Find_ValtanPatternMaster(
		const Client::VALTAN_PATTERN_TREE_VIEW& View,
		const std::string_view strPatternId);


	bool_t Try_ResolveValtanArenaPatternAction(
		const std::string_view strStageKind,
		LostArk::Shared::WORLD_ENTITY_ACTION& eOutAction);


	void Skip_Space(const char_t*& p);


	bool_t Read_Quoted(const char_t*& p, std::string& out);


	bool_t Read_Token(const char_t*& p, std::string& out);


	/* key=value, where value may be quoted so paths with spaces survive. */
	bool_t Read_Pair(const char_t*& p, std::string& key, std::string& value);


	bool_t Parse_Integer(const std::string& text, int32_t& outValue);


	bool_t Parse_Float(const std::string& text, f32_t& outValue);


	bool_t Has_RemainingToken(const char_t* p);


	bool_t Is_SafeQuotedText(const std::string& text);


	bool_t Parse_CommaSeparatedStableTokens(
		const std::string& Text,
		std::vector<std::string>& Out);


	bool_t Parse_CommaSeparatedNonnegativeSeconds(
		const std::string& Text,
		std::vector<f32_t>& Out);


	bool_t Load_ValtanCompositionSequenceLibrary(
		std::vector<Client::CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW>& Out,
		std::string& Status);


	const char_t* Resolve_ActionCompositionCategory(
		const std::string_view strProfileId,
		const std::string_view strDisplayName);


	bool_t Load_KoukuSaydonCompositionSequenceLibrary(
		std::vector<Client::CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW>& Out,
		std::string& Status);


	bool_t Contains_NoCase(const char_t* pText, const char_t* pNeedle);


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
		const f32_t fBlendSeconds);

}
using namespace AnimationToolDetail;
