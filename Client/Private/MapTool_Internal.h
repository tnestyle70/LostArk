#pragma once

#include "MapTool.h"
#include "ProjectDataRoot.h"
#include "DestructionSimulationController.h"
#include <chrono>
namespace Client { class DATA_JSON_VALUE; struct FMapStaticInstance; }

namespace MapToolDetail
{

	constexpr const char* KAKUL_AREA_ID = "LV_LUT_MIDNIGHTC_ED";
	/* build_arena_rise.py splits the 430 visible arena placements into
	   32-track templates. Every instance has to start on the same frame; their
	   own startDelayMs staggers the rise from the floor upward. */
	constexpr uint32_t KAKUL_ARENA_RISE_INSTANCE_COUNT = 14u;
	/* Instances converted straight from the original UE3 Matinee source are
	   named after the sequence they came from, so the whole import plays by
	   walking the document instead of a hand written list. */
	constexpr const char* KAKUL_ORIGINAL_INSTANCE_PREFIX =
		"world.sequence.instance.original_";
	/* Every imported Mario stage sequence shares this prefix, so the loop
	   picks them up from the document rather than a list that would fall
	   behind the next import. */
	constexpr const char* KAKUL_MARIO_INSTANCE_PREFIX =
		"world.sequence.instance.mario_";
	/* The card maze march states, in the order the Setos leave:
	   3->9, 6->12, 9->3, 12->6 o'clock. Every corridor lane carries its own
	   instance under this prefix, so one wave is the whole rank instead of a
	   single Seto, and a new lane needs no list here. */
	constexpr const char* CARDMIRO_MARCH_INSTANCE_PREFIX =
		"cardmiro.march.instance.";
	/* Every march instance whose id starts with that prefix, in document
	   order. One wave is a rank across all nine corridor lanes. */
	std::vector<std::string> Collect_CardMiroMarchInstanceIds(
		const CWorldSequenceDocument& document);

	/* Rewind this far before the authored end. One slow frame is well
	   inside it, which is what keeps an instance from ever finishing. */
	constexpr f32_t KAKUL_MARIO_LOOP_REWIND_MARGIN_MS = 120.f;
	/* The pop-up book. Its unfold clip runs 2370ms; the arena instances start
	   with it and hold their folded pose until each group is due. */
	constexpr const char* KAKUL_BOOK_INSTANCE_ID =
		"world.sequence.instance.book_open";
	/* Deploy placement 7: the one book, at the original's own pose and scale.
	   The arena no longer carries a second copy. */
	constexpr uint64_t KAKUL_BOOK_PLACEMENT_ID = 7ull;
	/* The cutscene set is authored hidden and shown only while the unfold
	   runs; hand-authored arena placements start at the end of the range. */
	constexpr uint64_t KAKUL_CUTSCENE_SET_FIRST_ID = 41ull;
	constexpr uint64_t KAKUL_CUTSCENE_SET_END_ID = 300ull;
	/* The reference holds the finished miniature inside the book for a camera
	   push-in before it cuts to the arena. The book leaves this long after the
	   last arena_rise instance has settled, whatever the sheet timing is. */
	constexpr f32_t KAKUL_BOOK_HOLD_AFTER_ARENA_MS = 1500.f;
	/* One slow frame of cutscene time; a stall longer than this is a load
	   hitch, not elapsed animation. */
	constexpr f32_t KAKUL_CUTSCENE_MAX_STEP_SECONDS = 0.1f;
	struct AUTHORING_FILE_BACKUP
	{
		std::filesystem::path destination;
		std::filesystem::path backup;
		bool_t hadOriginal = false;
	};

	class SCOPED_AUTHORING_SAVE_LOCK final
	{
	public:
		SCOPED_AUTHORING_SAVE_LOCK() = default;
		~SCOPED_AUTHORING_SAVE_LOCK()
		{
			if (INVALID_HANDLE_VALUE != m_Handle)
				CloseHandle(m_Handle);
		}

		SCOPED_AUTHORING_SAVE_LOCK(const SCOPED_AUTHORING_SAVE_LOCK&) = delete;
		SCOPED_AUTHORING_SAVE_LOCK& operator=(
			const SCOPED_AUTHORING_SAVE_LOCK&) = delete;

		bool_t Acquire(
			const std::filesystem::path& sequencePath,
			std::string& outStatus)
		{
			if (INVALID_HANDLE_VALUE != m_Handle || sequencePath.empty())
			{
				outStatus = "Linked authoring save lock path is invalid";
				return false;
			}
			std::filesystem::path lockPath = sequencePath;
			lockPath += L".linked-save.lock";
			m_Handle = CreateFileW(lockPath.c_str(),
				GENERIC_READ | GENERIC_WRITE | DELETE, 0, nullptr, OPEN_ALWAYS,
				FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
			if (INVALID_HANDLE_VALUE == m_Handle)
			{
				const DWORD error = GetLastError();
				if (ERROR_SHARING_VIOLATION == error ||
					ERROR_LOCK_VIOLATION == error)
				{
					outStatus =
						"This Area is being loaded or saved by another MapTool process";
				}
				else
				{
					outStatus = "Could not open Area authoring lock " +
						lockPath.filename().string() + " (Windows error " +
						std::to_string(error) + ")";
				}
				return false;
			}
			return true;
		}

	private:
		HANDLE m_Handle = INVALID_HANDLE_VALUE;
	};

	bool_t PrepareAuthoringBackup(
		const std::filesystem::path& destination,
		AUTHORING_FILE_BACKUP& outBackup,
		std::string& outStatus);


	bool_t RestoreAuthoringBackup(const AUTHORING_FILE_BACKUP& backup);


	void DiscardAuthoringBackup(const AUTHORING_FILE_BACKUP& backup);


	std::filesystem::path GetAuthoringTransactionMarker(
		const std::filesystem::path& sequencePath);


	bool_t WriteAuthoringTransactionMarker(
		const AUTHORING_FILE_BACKUP& placementBackup,
		const AUTHORING_FILE_BACKUP& sequenceBackup,
		std::string& outStatus);


	bool_t ClearAuthoringTransactionMarker(
		const std::filesystem::path& sequencePath);


	bool_t RecoverAuthoringTransactionUnderLock(
		const std::filesystem::path& placementPath,
		const std::filesystem::path& sequencePath,
		std::string& outStatus);


	bool_t AreExactlySamePlacementRecords(
		const std::vector<MAP_PLACEMENT_RECORD>& expected,
		const std::vector<MAP_PLACEMENT_RECORD>& actual);


	/* How long one frame may spend admitting map model prototypes while an
	   Area switch is in flight. Bern is 1003 prototypes over 1.8 GB, so the
	   editor spreads that work instead of stalling inside a single frame. */
	constexpr std::chrono::milliseconds
		EDITOR_AREA_ADMISSION_FRAME_BUDGET{ 8 };

	bool_t NpcBatchBodiesOverlapVertically(
		const f32_t leftGroundY,
		const f32_t rightGroundY);


	bool_t NpcBatchBodyOverlapsCollisionHeight(
		const f32_t groundY,
		const WORLD_GAMEPLAY_PLACEMENT& collisionBox);


	bool_t IsFinite(const float3_t& value);


	bool_t IsFiniteQuaternion(const float4_t& value);


	/* Animated Prop authoring edits a whole placement pose in degrees and
	   stores the normalized quaternion the Deploy placement row owns. The
	   World Sequence panel keeps its own equivalent for keyframe offsets. */
	float3_t DeployQuaternionToEulerDegrees(const float4_t& value);


	float4_t DeployEulerDegreesToQuaternion(const float3_t& value);


	void ShowAuthoringHelp(const char_t* text);


	bool_t MatchesAnimatedPropFilter(
		const std::string& label,
		const std::string& assetId,
		const char_t* filter);


	std::string Editor_AreaShortName(const std::string& areaId);


	std::string Editor_NpcArchetypeToken(const std::string& archetypeId);


	uint32_t Editor_StableSeed(const std::string& value);


	bool_t TryBuildCentralEditorFrame(
		const vector<float3_t>& positions,
		float3_t& outCenter,
		f32_t& outRadius);


	/* Frames every enabled player spawn the Area declares. It used to take a
	preferred placement id, but no authored document has ever carried the two
	ids the caller asked for, so that lookup always fell through to whichever
	spawn happened to come first and the named branches were dead weight. The
	spawn set is the stable authoring focus: it is what the Area declares as
	the playable entry, and it cannot be dragged away by backdrop meshes. */
	bool_t TryBuildGameplaySpawnFrame(
		const CWorldGameplayDocument& document,
		float3_t& outCenter,
		f32_t& outRadius);


	bool_t IsBernLandscapePlacement(
		const CMapAssetCatalog& catalog,
		const MAP_PLACEMENT_RECORD& record);


	bool_t IsBatchEligible(const MAP_ASSET_ENTRY& asset);


	HRESULT BuildStaticInstance(
		const MAP_ASSET_ENTRY& asset,
		const shared_ptr<CModel>& model,
		const MAP_PLACEMENT_RECORD& record,
		FMapStaticInstance& outInstance);


	bool_t MatchesFilter(const std::string& text, const char* pFilter);


	uint64_t HashStableAuthoringId(const std::string& value);


	std::string ToStableHex(const uint64_t value);


	bool_t IsSameNavigationFloat(f32_t left, f32_t right);


	bool_t HasSameNavigationGridIdentity(
		const NAVGRID_AUTHORING_DESC& left,
		const NAVGRID_AUTHORING_DESC& right);


	bool_t HasSameNavigationPath(
		const std::filesystem::path& left,
		const std::filesystem::path& right);


	/* Distinct from the product level and the cinematic owners so an editor
	   preview never collides with a shipped override. */
	constexpr uint64_t CAMERA_SHOT_PREVIEW_OWNER_ID = 0x4D54434D53485450ull;
	constexpr std::string_view MARIO_SEQUENCE_ROOT =
		"world.sequence.instance.mario_";

	bool_t ReadTextFile(
		const std::filesystem::path& path,
		std::string& outText);


	bool_t ReadRequiredString(
		const Client::DATA_JSON_VALUE& object,
		const char_t* pName,
		std::string& outValue);


	/* is_regular_file reports a missing file through the error_code, so a
	   bare "if (error)" cannot separate an absent optional document from a
	   real inspection failure. Only the latter may abort a load. */
	bool_t IsFileInspectionFailure(const std::error_code& error);


	std::filesystem::path ResolveDataCatalogPath(
		const std::string& value);


	const char_t* SimulationPlaybackStateLabel(
		const DESTRUCTION_SIMULATION_PLAYBACK_STATE state);


	const char_t* SimulationElementStateLabel(
		const DESTRUCTION_SIMULATION_ELEMENT_STATE state);


	/* Two co-located Deploy walls that share every authored simulation value are
	   one wall to the author. They stay two document elements because an element
	   owns exactly one source placement, but the outliner, the authoring panel and
	   the Solo scope treat them as a single emitter. Profiles whose emitters were
	   tuned apart (the 3705102 wall ring) stay split, so a linked Apply can never
	   overwrite a direction the author set per wall. */
	bool_t HasSharedEmitterAuthoring(
		const Client::DESTRUCTION_SIMULATION_ELEMENT& left,
		const Client::DESTRUCTION_SIMULATION_ELEMENT& right);


	bool_t AreEmittersAuthoredAsOneWall(
		const Client::DESTRUCTION_SIMULATION_PROFILE& profile);


	const char_t* SimulationScopeLabel(
		const DESTRUCTION_SIMULATION_SCOPE scope);


	bool_t NormalizeSimulationDirection(float3_t& direction);

}
using namespace MapToolDetail;
