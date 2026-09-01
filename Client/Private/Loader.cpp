#include "Loader.h"

#include "ActorCatalog.h"
#include "AnimationPreviewAssets.h"
#include "Camera_Free.h"
#include "Body_Valtan.h"
#include "Character.h"
#include "CharacterSelectionState.h"
#include "Collider.h"
#include "DeployPropCatalog.h"
#include "DeployPropObject.h"
#include "DeployPropRuntime.h"
#include "Effect_Catalog.h"
#include "Effect_LoadPreparationJob.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "MapAssetCatalog.h"
#include "MapAssetObject.h"
#include "MapAssetPreview.h"
#include "MapNavigationContract.h"
#include "MapPlacementRuntime.h"
#include "MapStaticBatchObject.h"
#include "Navigation.h"
#include "NetworkManager.h"
#include "MonsterPresentationAssetService.h"
#include "NpcPlacementPresentationService.h"
#include "NpcPresentationAssetService.h"
#include "Part_Body.h"
#include "Part_Equipment.h"
#include "PlayableCharacterAssetService.h"
#include "RuntimeAssetRoot.h"
#include "Trigger_Box.h"
#include "Valtan.h"
#include "ValtanPresentationAssetService.h"

#ifdef _DEBUG
#include "MapEditorWorkspaceService.h"
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <unordered_set>

namespace
{
	std::mutex g_ActiveStatusMutex;
	std::string g_ActiveStatus = "Loader has not started.";

	std::string ToUtf8(const tchar_t* pText)
	{
		if (nullptr == pText || L'\0' == *pText)
			return {};
		const int characterCount = static_cast<int>(wcslen(pText));
		const int byteCount = WideCharToMultiByte(
			CP_UTF8, 0, pText, characterCount, nullptr, 0, nullptr, nullptr);
		if (byteCount <= 0)
			return {};
		std::string result(static_cast<size_t>(byteCount), '\0');
		WideCharToMultiByte(
			CP_UTF8, 0, pText, characterCount, result.data(), byteCount,
			nullptr, nullptr);
		return result;
	}

	class CLevelResourceRollbackScope final
	{
	public:
		explicit CLevelResourceRollbackScope(const uint32_t levelIndex)
			: m_iLevelIndex { levelIndex }
		{
		}

		~CLevelResourceRollbackScope()
		{
			if (m_isCommitted)
				return;

			if (FAILED(CGameInstance::Get().Clear_Resources(m_iLevelIndex)))
			{
				OutputDebugStringA(
					"[Loader] Failed to roll back level resources.\n");
			}
		}

		CLevelResourceRollbackScope(
			const CLevelResourceRollbackScope&) = delete;
		CLevelResourceRollbackScope& operator=(
			const CLevelResourceRollbackScope&) = delete;

		void Commit()
		{
			m_isCommitted = true;
		}

	private:
		uint32_t m_iLevelIndex = {};
		bool_t m_isCommitted = false;
	};

	std::string ResolveAssetPath(const std::filesystem::path& relativePath)
	{
		return CRuntimeAssetRoot::Resolve(relativePath).string();
	}

	const tchar_t* Get_CharacterClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return TEXT("Lance Master");
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return TEXT("Gunslinger");
		case CHARACTER_CLASS_ID::SLAYER:
			return TEXT("Slayer");
		case CHARACTER_CLASS_ID::ARTIST:
			return TEXT("Artist");
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return TEXT("DimensionMaster");
		case CHARACTER_CLASS_ID::WARLORD:
			return TEXT("Warlord");
		default:
			return TEXT("Unknown");
		}
	}
}

CLoader::CLoader(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
}

CLoader::~CLoader()
{
	Free();
}

uint32_t APIENTRY ThreadMain(void* pArgument)
{
	const HRESULT comResult =
		CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	CLoader* pLoader = static_cast<CLoader*>(pArgument);
	const HRESULT loadResult =
		nullptr == pLoader ? E_POINTER : pLoader->Start_Loading();

	if (SUCCEEDED(comResult))
		CoUninitialize();

	return FAILED(loadResult) ? 1u : 0u;
}

HRESULT CLoader::Initialize(const LEVEL eNextLevelID)

{
	return Initialize(eNextLevelID, 0u, 0u);
}

HRESULT CLoader::Initialize(
	const LEVEL eNextLevelID,
	const uint64_t iEffectLoadJobEpoch,
	const uint64_t iEffectCatalogRevision)
{
	if (nullptr == CLevelRegistry::Find(eNextLevelID))
		return E_INVALIDARG;
	if ((0u == iEffectLoadJobEpoch) != (0u == iEffectCatalogRevision))
		return E_INVALIDARG;
	if (0u != iEffectLoadJobEpoch)
	{
		m_pEffectLoadJob = std::make_shared<CEffectLoadPreparationJob>();
		std::string JobStatus;
		if (nullptr == m_pEffectLoadJob || !m_pEffectLoadJob->Open(
			iEffectLoadJobEpoch, iEffectCatalogRevision, JobStatus))
		{
			OutputDebugStringA(("[Loader][Effect] " + JobStatus + "\n").c_str());
			m_pEffectLoadJob.reset();
			return E_FAIL;
		}
	}

	m_eNextLevelID = eNextLevelID;
	m_iResult.store(S_FALSE, std::memory_order_release);
	m_eState.store(STATE::RUNNING, std::memory_order_release);

	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(
		nullptr,
		0,
		ThreadMain,
		this,
		0,
		nullptr));
	if (nullptr == m_hThread)
	{
		m_iResult.store(E_FAIL, std::memory_order_release);
		m_eState.store(STATE::FAILED, std::memory_order_release);
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CLoader::Start_Loading()
{
	HRESULT result =
		CLevelRegistry::Execute_Load(m_eNextLevelID, *this);
	if (SUCCEEDED(result) && nullptr != m_pEffectLoadJob)
		result = Run_EffectLoadPreparation();
	m_iResult.store(result, std::memory_order_release);
	m_eState.store(
		SUCCEEDED(result) ? STATE::SUCCEEDED : STATE::FAILED,
		std::memory_order_release);
	if (FAILED(result))
		OutputDebugStringW(L"[Loader] Level load failed.\n");
	return result;
}

HRESULT CLoader::Run_EffectLoadPreparation()
{
	if (nullptr == m_pEffectLoadJob)
		return S_OK;

	std::optional<EFFECT_LOAD_JOB_COMMAND> PendingBatchCommand;
	for (;;)
	{
		EFFECT_LOAD_JOB_COMMAND Command;
		EFFECT_LOAD_MAILBOX_WAIT_RESULT WaitResult =
			EFFECT_LOAD_MAILBOX_WAIT_RESULT::COMMAND;
		if (PendingBatchCommand.has_value())
		{
			Command = std::move(*PendingBatchCommand);
			PendingBatchCommand.reset();
		}
		else
		{
			WaitResult = m_pEffectLoadJob->Wait_Pop_Command(Command);
		}
		if (EFFECT_LOAD_MAILBOX_WAIT_RESULT::CANCELLED == WaitResult)
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		if (EFFECT_LOAD_MAILBOX_WAIT_RESULT::CLOSED == WaitResult)
			return S_OK;
		if (EFFECT_LOAD_MAILBOX_WAIT_RESULT::COMMAND != WaitResult ||
			!Command.Is_Valid())
		{
			return E_FAIL;
		}
		if (EFFECT_LOAD_JOB_COMMAND_KIND::CANCEL == Command.eKind)
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		if (EFFECT_LOAD_JOB_COMMAND_KIND::CLOSE == Command.eKind)
			return S_OK;
		if (EFFECT_LOAD_JOB_COMMAND_KIND::ACCEPT_DISCOVERY != Command.eKind &&
			EFFECT_LOAD_JOB_COMMAND_KIND::REBASE != Command.eKind)
		{
			return E_FAIL;
		}

		const auto Batch =
			std::static_pointer_cast<const EFFECT_PRODUCT_LOAD_STAGE_BATCH>(
				Command.pImmutablePayload);
		if (nullptr == Batch ||
			Batch->Targets.size() != Command.EffectAssetIds.size())
		{
			return E_FAIL;
		}
		for (size_t i = 0u; i < Batch->Targets.size(); ++i)
		{
			if (Batch->Targets[i].strEffectAssetId != Command.EffectAssetIds[i])
				return E_FAIL;
		}

		EFFECT_LOAD_PROGRESS_SNAPSHOT Progress;
		Progress.iJobEpoch = Command.iJobEpoch;
		Progress.iCatalogRevision = Command.iCatalogRevision;
		Progress.ePhase = EFFECT_LOAD_PROGRESS_PHASE::TARGET_STAGE;
		Progress.bDeterminate = true;
		Progress.iTotal = static_cast<uint32_t>(Batch->Targets.size());
		Progress.strStatus = "Staging Product Effect resources.";
		m_pEffectLoadJob->Publish_Progress(Progress);

		bool_t bEpochRebased = false;
		for (size_t i = 0u; i < Batch->Targets.size(); ++i)
		{
			if (m_isCancellationRequested.load(std::memory_order_acquire) ||
				m_pEffectLoadJob->Is_Cancelled())
			{
				return HRESULT_FROM_WIN32(ERROR_CANCELLED);
			}

			const EFFECT_PRODUCT_LOAD_STAGE_REQUEST& Request =
				Batch->Targets[i];
			Progress.iCompleted = static_cast<uint32_t>(i);
			Progress.strCurrentId = Request.strEffectAssetId;
			Progress.strStatus =
				"Preparing Product Effect document and device resources.";
			m_pEffectLoadJob->Publish_Progress(Progress);

			std::shared_ptr<const EFFECT_PRODUCT_LOADING_TARGET_STAGE> Staged;
			std::string StageStatus;
			const bool_t bStaged =
				CEffectPresentationService::Stage_LoadingProductTarget(
					m_pDevice, m_pContext, Request, Staged, StageStatus);
			EFFECT_LOAD_JOB_RESULT Result;
			Result.eKind = EFFECT_LOAD_JOB_RESULT_KIND::TARGET_STAGED;
			Result.iJobEpoch = Command.iJobEpoch;
			Result.iCatalogRevision = Command.iCatalogRevision;
			Result.strEffectAssetId = Request.strEffectAssetId;
			if (bStaged)
			{
				/* Keep one worker-owned reference through the matching ACK.  The
				   owner-thread renderer commit swaps the previous prepared maps and
				   shared-asset session into this candidate; retaining this reference
				   makes their potentially large COM/map destruction happen back on
				   the Loader worker after the ACK instead of on the UI frame. */
				Result.pImmutablePayload = Staged;
			}
			else
			{
				EFFECT_LOAD_FAILURE_RECEIPT Failure;
				Failure.iJobEpoch = Command.iJobEpoch;
				Failure.iCatalogRevision = Command.iCatalogRevision;
				Failure.strEffectAssetId = Request.strEffectAssetId;
				Failure.iRootCode = E_FAIL;
				Failure.strRootMessage = StageStatus.empty() ?
					"Product Effect worker stage failed." : StageStatus;
				Result.Failure = std::move(Failure);
				++Progress.iIsolatedFailureCount;
			}

			const EFFECT_LOAD_RESULT_PUSH_RESULT PushResult =
				m_pEffectLoadJob->Push_Result_Wait(std::move(Result));
			if (EFFECT_LOAD_RESULT_PUSH_RESULT::REBASED == PushResult)
			{
				bEpochRebased = true;
				break;
			}
			if (EFFECT_LOAD_RESULT_PUSH_RESULT::CANCELLED == PushResult)
				return HRESULT_FROM_WIN32(ERROR_CANCELLED);
			if (EFFECT_LOAD_RESULT_PUSH_RESULT::CLOSED == PushResult)
				return S_OK;
			if (EFFECT_LOAD_RESULT_PUSH_RESULT::PUSHED != PushResult)
				return E_FAIL;

			/* The staged payload remains the worker's current target until the
			   main-thread owner has committed either its prepared record or its
			   isolated failure receipt.  This bounds in-flight GPU/resource work
			   to one target and prevents worker staging from outrunning commit. */
			Progress.strStatus =
				"Waiting for Product Effect target commit acknowledgment.";
			m_pEffectLoadJob->Publish_Progress(Progress);
			EFFECT_LOAD_JOB_COMMAND AckCommand;
			const EFFECT_LOAD_MAILBOX_WAIT_RESULT AckWaitResult =
				m_pEffectLoadJob->Wait_Pop_Command(AckCommand);
			if (EFFECT_LOAD_MAILBOX_WAIT_RESULT::CANCELLED == AckWaitResult)
				return HRESULT_FROM_WIN32(ERROR_CANCELLED);
			if (EFFECT_LOAD_MAILBOX_WAIT_RESULT::CLOSED == AckWaitResult)
				return S_OK;
			if (EFFECT_LOAD_MAILBOX_WAIT_RESULT::COMMAND != AckWaitResult ||
				!AckCommand.Is_Valid())
			{
				OutputDebugStringA(
					"[Loader][Effect] Target commit ACK wait returned an invalid command.\n");
				return E_FAIL;
			}
			if (EFFECT_LOAD_JOB_COMMAND_KIND::CANCEL == AckCommand.eKind)
				return HRESULT_FROM_WIN32(ERROR_CANCELLED);
			if (EFFECT_LOAD_JOB_COMMAND_KIND::CLOSE == AckCommand.eKind)
				return S_OK;
			if (EFFECT_LOAD_JOB_COMMAND_KIND::REBASE == AckCommand.eKind)
			{
				PendingBatchCommand = std::move(AckCommand);
				bEpochRebased = true;
				break;
			}
			if (EFFECT_LOAD_JOB_COMMAND_KIND::TARGET_COMMIT_ACK !=
					AckCommand.eKind ||
				AckCommand.iJobEpoch != Command.iJobEpoch ||
				AckCommand.iCatalogRevision != Command.iCatalogRevision ||
				1u != AckCommand.EffectAssetIds.size() ||
				AckCommand.EffectAssetIds.front() != Request.strEffectAssetId)
			{
				OutputDebugStringA(
					"[Loader][Effect] Target commit ACK identity does not match the staged target.\n");
				return E_FAIL;
			}
			Progress.iCompleted = static_cast<uint32_t>(i + 1u);
			Progress.strCurrentId.clear();
			Progress.strStatus = Progress.iCompleted == Progress.iTotal ?
				"Finalizing Product Effect staging epoch." :
				"Product Effect target committed.";
			m_pEffectLoadJob->Publish_Progress(Progress);
		}
		if (bEpochRebased)
			continue;

		EFFECT_LOAD_JOB_RESULT Complete;
		Complete.eKind = EFFECT_LOAD_JOB_RESULT_KIND::EPOCH_STAGE_COMPLETE;
		Complete.iJobEpoch = Command.iJobEpoch;
		Complete.iCatalogRevision = Command.iCatalogRevision;
		const EFFECT_LOAD_RESULT_PUSH_RESULT CompleteResult =
			m_pEffectLoadJob->Push_Result_Wait(std::move(Complete));
		if (EFFECT_LOAD_RESULT_PUSH_RESULT::REBASED == CompleteResult)
			continue;
		if (EFFECT_LOAD_RESULT_PUSH_RESULT::CANCELLED == CompleteResult)
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		if (EFFECT_LOAD_RESULT_PUSH_RESULT::CLOSED == CompleteResult)
			return S_OK;
		if (EFFECT_LOAD_RESULT_PUSH_RESULT::PUSHED != CompleteResult)
			return E_FAIL;
		Progress.ePhase = EFFECT_LOAD_PROGRESS_PHASE::EPOCH_STAGE_COMPLETE;
		Progress.iCompleted = Progress.iTotal;
		Progress.strCurrentId.clear();
		Progress.strStatus = "Product Effect resource staging complete.";
		m_pEffectLoadJob->Publish_Progress(Progress);
	}
}

void CLoader::Set_Status(const tchar_t* pStatus)
{
	{
		lock_guard<mutex> lock(m_StatusMutex);
		const bool_t bChanged = nullptr == pStatus ?
			L'\0' != m_szLoadingText[0] :
			0 != wcscmp(m_szLoadingText, pStatus);
		if (nullptr == pStatus)
			m_szLoadingText[0] = L'\0';
		else
			wcsncpy_s(m_szLoadingText, pStatus, _TRUNCATE);
		m_bProgressDeterminate = false;
		m_iProgressCompleted = 0u;
		m_iProgressTotal = 0u;
		if (bChanged)
			m_ProgressPhaseStarted = std::chrono::steady_clock::now();
	}

	lock_guard<mutex> activeLock(g_ActiveStatusMutex);
	g_ActiveStatus = ToUtf8(pStatus);
}

void CLoader::Set_DeterminateStatus(
	const tchar_t* pStatus,
	const size_t iCompleted,
	const size_t iTotal)
{
	{
		lock_guard<mutex> lock(m_StatusMutex);
		const bool_t bChanged = nullptr == pStatus ?
			L'\0' != m_szLoadingText[0] :
			0 != wcscmp(m_szLoadingText, pStatus);
		if (nullptr == pStatus)
			m_szLoadingText[0] = L'\0';
		else
			wcsncpy_s(m_szLoadingText, pStatus, _TRUNCATE);
		m_bProgressDeterminate = 0u != iTotal;
		m_iProgressCompleted = (min)(iCompleted, iTotal);
		m_iProgressTotal = iTotal;
		if (bChanged)
			m_ProgressPhaseStarted = std::chrono::steady_clock::now();
	}

	lock_guard<mutex> activeLock(g_ActiveStatusMutex);
	g_ActiveStatus = ToUtf8(pStatus);
}

std::string CLoader::Get_ActiveStatus()
{
	lock_guard<mutex> lock(g_ActiveStatusMutex);
	return g_ActiveStatus;
}

CLoader::PROGRESS_SNAPSHOT CLoader::Get_ProgressSnapshot() const
{
	PROGRESS_SNAPSHOT Snapshot;
	tchar_t Status[MAX_PATH]{};
	{
		lock_guard<mutex> lock(m_StatusMutex);
		wcsncpy_s(Status, m_szLoadingText, _TRUNCATE);
		Snapshot.bDeterminate = m_bProgressDeterminate;
		Snapshot.iCompleted = m_iProgressCompleted;
		Snapshot.iTotal = m_iProgressTotal;
		Snapshot.iElapsedMs = static_cast<uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() -
				m_ProgressPhaseStarted).count());
	}
	Snapshot.strStatus = ToUtf8(Status);
	return Snapshot;
}

void CLoader::Copy_Status(
	tchar_t* pOutput,
	const size_t outputCount) const
{
	if (nullptr == pOutput || 0u == outputCount)
		return;
	lock_guard<mutex> lock(m_StatusMutex);
	wcsncpy_s(pOutput, outputCount, m_szLoadingText, _TRUNCATE);
}

#ifdef _DEBUG
void CLoader::Print_Text()
{
	tchar_t status[MAX_PATH]{};
	Copy_Status(status, size(status));
	SetWindowText(g_hWnd, status);
}
#endif

HRESULT CLoader::Ready_For_Lobby()
{
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::LOBBY));
	Set_Status(TEXT("LOBBY: stage selection UI"));
	Set_Status(TEXT("Lobby loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_CharacterSelect()
{
	CValtanPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::CHARACTER_SELECT));
	CNpcPlacementPresentationService::Begin_LevelLoad(
		ETOUI(LEVEL::CHARACTER_SELECT));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::CHARACTER_SELECT), "CHARACTER_SELECT_ARENA")))
	{
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	using LostArk::Shared::CHARACTER_CLASS_ID;
	CHARACTER_CLASS_ID initialClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	if (!CCharacterSelectionState::Try_Get_SelectedClass(initialClass) ||
		!LostArk::Shared::Is_Supported_Playable_Character_Class(initialClass))
	{
		initialClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	}

	const std::array characterClasses = { initialClass };

	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::CHARACTER_SELECT));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::CHARACTER_SELECT);

	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId)
		return E_INVALIDARG;

	Set_Status(TEXT("CHARACTER SELECT: visual map"));
	if (FAILED(Ready_MapArea(
		ETOUI(LEVEL::CHARACTER_SELECT),
		pEntry->pMapAreaId,
		pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("CHARACTER SELECT: playable classes"));
	if (FAILED(Ready_Character_Rendering(
			ETOUI(LEVEL::CHARACTER_SELECT),
			characterClasses)))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("Character Select loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_Bern()
{
	CNpcPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::BERN));
	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::BERN));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::BERN), "BERN")))
	{
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::BERN));
	Set_Status(TEXT("BERN: world catalog and placements"));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::BERN);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::BERN),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("BERN: session character bundle"));
	const std::array selectedClass =
	{
		CNetworkManager::Get().Get_LocalCharacterClass()
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::BERN),
		selectedClass)))
		return E_FAIL;

	Set_Status(TEXT("Bern loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_ValtanArena()
{
	CValtanPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::VALTAN_ARENA));
	CNpcPlacementPresentationService::Begin_LevelLoad(
		ETOUI(LEVEL::VALTAN_ARENA));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::VALTAN_ARENA), "VALTAN_ARENA")))
	{
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CMonsterPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::VALTAN_ARENA));
	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::VALTAN_ARENA));
	Set_Status(TEXT("VALTAN: arena map"));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::VALTAN_ARENA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::VALTAN_ARENA),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: network player rendering"));
	const std::array selectedClass =
	{
		CNetworkManager::Get().Get_LocalCharacterClass()
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::VALTAN_ARENA),
		selectedClass)))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: server-authoritative boss presentation"));
	if (FAILED(Ready_ValtanPresentation(
		ETOUI(LEVEL::VALTAN_ARENA))))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: monster presentation warmup"));
	for (const MONSTER_ACTOR_ENTRY& monster : CActorCatalog::Get_Monsters())
	{
		if (monster.runtimeStatus != "supported")
			continue;
		if (FAILED(CMonsterPresentationAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			ETOUI(LEVEL::VALTAN_ARENA),
			monster.archetypeId)))
		{
			OutputDebugStringA((
				"[Loader][MonsterPresentation] unavailable archetype: " +
				monster.archetypeId + "\n").c_str());
		}
	}
	/* The raid Esther summons spawn mid-fight, so their models must already be
	prototypes when the arena opens: the lazy admission on first spawn stalls
	the frame the caster presses the key. This list mirrors the server's
	CEstherSkillSystem roster and moves into a data contract with it. */
	Set_Status(TEXT("VALTAN: esther summon presentation"));
	for (const char* pEstherArchetypeId :
		{ "NPC_59030", "NPC_58700", "NPC_59060" })
	{
		if (FAILED(CNpcPresentationAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			ETOUI(LEVEL::VALTAN_ARENA),
			pEstherArchetypeId)))
		{
			/* The summon payload lives in the team-managed Resources folder, so
			a machine that has not received it yet must still be able to enter
			the raid. Only that summon's presentation is isolated: the gauge,
			the Server skill and every other arena contract are untouched. */
			OutputDebugStringA(
				(std::string("[Loader][NpcPresentation] VALTAN esther summon "
					"presentation is unavailable (") + pEstherArchetypeId +
					"); the arena loads without it.\n").c_str());
		}
	}
	Set_Status(TEXT("VALTAN: deploy environment prototypes"));
	if (FAILED(Ready_DeployPropArea(
		ETOUI(LEVEL::VALTAN_ARENA),
		pEntry->pMapAreaId)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("Valtan arena loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_KakulSaydonArena()
{
	CNpcPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::KAKULSAYDON_ARENA));
	CNpcPlacementPresentationService::Begin_LevelLoad(
		ETOUI(LEVEL::KAKULSAYDON_ARENA));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::KAKULSAYDON_ARENA), "KAKULSAYDON_ARENA")))
	{
		/* The world remains enterable when it has no authored NPC placement
		   presentation yet. Reliable Server entities retain their catalog idle
		   fallback, while the missing optional binding is reported explicitly. */
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CMonsterPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::KAKULSAYDON_ARENA));

	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::KAKULSAYDON_ARENA));
	Set_Status(TEXT("KoukuSaton: arena map"));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::KAKULSAYDON_ARENA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::KAKULSAYDON_ARENA),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("KoukuSaton: server-approved character rendering"));
	const std::array selectedClass =
	{
		CNetworkManager::Get().Get_LocalCharacterClass()
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		selectedClass)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("KoukuSaton arena loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_Development()
{
	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::DEVELOPMENT));

#ifdef _DEBUG
	if (CMapEditorWorkspaceService::Is_Requested())
	{
		CNpcPresentationAssetService::Begin_LevelLoad(
			ETOUI(LEVEL::DEVELOPMENT));
		Set_Status(TEXT("MAP EDITOR: core rendering resources"));
		if (FAILED(Ready_MapAuthoringCore(ETOUI(LEVEL::DEVELOPMENT))))
		{
			CMapEditorWorkspaceService::Cancel();
			return E_FAIL;
		}
		if (FAILED(Ready_AnimatedMeshShader(ETOUI(LEVEL::DEVELOPMENT))) ||
			FAILED(Ready_DeployPropCore(ETOUI(LEVEL::DEVELOPMENT))))
		{
			CMapEditorWorkspaceService::Cancel();
			return E_FAIL;
		}

		Set_Status(TEXT("Map Editor workspace loading complete"));
		rollback.Commit();
		return S_OK;
	}
#endif

	CValtanPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::DEVELOPMENT));
	CNpcPlacementPresentationService::Begin_LevelLoad(
		ETOUI(LEVEL::DEVELOPMENT));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::DEVELOPMENT), "TRAINING_GROUND")))
	{
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::DEVELOPMENT);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId)
		return E_INVALIDARG;

	Set_Status(TEXT("TEST: training map"));
	if (FAILED(Ready_MapArea(
		ETOUI(LEVEL::DEVELOPMENT),
		pEntry->pMapAreaId,
		pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	const std::array selectedClass =
	{
		CNetworkManager::Get().Get_LocalCharacterClass()
	};
	Set_Status(TEXT("TEST: server-approved character rendering"));
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::DEVELOPMENT),
		selectedClass)))
	{
		return E_FAIL;
	}
	if (FAILED(Ready_AnimationPreviewModels(
		ETOUI(LEVEL::DEVELOPMENT))))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("Development scenario loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_MapArea(
	const uint32_t iLevelIndex,
	const std::string& areaId,
	const MAP_LOAD_SCOPE& loadScope)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || areaId.empty())
		return E_INVALIDARG;

	if (FAILED(Ready_MapAuthoringCore(iLevelIndex)))
		return E_FAIL;

	CMapAssetCatalog mapCatalog;
	Set_Status(TEXT("Map: explicit area catalog"));
	if (!mapCatalog.Load_Area(areaId))
	{
		OutputDebugStringA((
			"[Loader][Map] " +
			mapCatalog.Get_Status() +
			"\n").c_str());
		return E_FAIL;
	}

	std::unordered_set<std::string> requiredAssetIds;
	std::vector<MAP_PLACEMENT_RECORD> scopedPlacements;
	if (loadScope.isEnabled)
	{
		std::string placementStatus;
		Set_Status(TEXT("Map: product load scope"));
		if (!CMapPlacementRuntime::Read_Placements(
			mapCatalog, scopedPlacements, placementStatus))
		{
			OutputDebugStringA(("[Loader][Map] " + placementStatus + "\n").c_str());
			return E_FAIL;
		}
		CMapPlacementRuntime::Apply_LoadScope(
			mapCatalog, loadScope, scopedPlacements);
		for (const MAP_PLACEMENT_RECORD& record : scopedPlacements)
			requiredAssetIds.insert(record.assetId);
		if (requiredAssetIds.empty())
			return E_FAIL;
		CMapPlacementRuntime::Cache_LoadStage(
			areaId,
			loadScope,
			mapCatalog,
			scopedPlacements);
	}

	const matrix_t mapAssetTransform =
		XMMatrixScaling(0.01f, 0.01f, 0.01f);
	const size_t requiredModelCount = loadScope.isEnabled ?
		requiredAssetIds.size() : mapCatalog.Get_Entries().size();
	size_t loadedModelCount = {};
	for (const MAP_ASSET_ENTRY& entry : mapCatalog.Get_Entries())
	{
		if (loadScope.isEnabled &&
			requiredAssetIds.end() == requiredAssetIds.find(entry.id))
		{
			continue;
		}
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);

		tchar_t progress[128]{};
		_snwprintf_s(
			progress,
			std::size(progress),
			_TRUNCATE,
			TEXT("Map: model prototypes %zu/%zu"),
			loadedModelCount,
			requiredModelCount);
		Set_DeterminateStatus(
			progress, loadedModelCount, requiredModelCount);

		const std::string modelPath =
			entry.resolvedModelPath.string();
		auto pModel = CModel::Create(
			m_pDevice,
			m_pContext,
			MODEL::NONANIM,
			modelPath.c_str(),
			mapAssetTransform);
		if (nullptr == pModel ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				entry.prototypeTag,
				std::move(pModel))))
		{
			const std::wstring detail =
				L"[Loader][Map] Model failed: " +
				entry.prototypeTag +
				L" / " +
				entry.resolvedModelPath.wstring() +
				L"\n";
			OutputDebugStringW(detail.c_str());
			return E_FAIL;
		}
		++loadedModelCount;
	}
	if (loadedModelCount != requiredModelCount)
		return E_FAIL;

	MAP_NAVIGATION_CONTRACT navigationContract;
	std::string navigationStatus;
	Set_Status(TEXT("Map: navigation contract"));
	if (!CMapNavigationContract::Resolve_Area(
		areaId,
		navigationContract,
		navigationStatus))
	{
		OutputDebugStringA((
			"[Loader][Map] " +
			navigationStatus +
			"\n").c_str());
		return E_FAIL;
	}

	if (navigationContract.runtimeGridAvailable)
	{
		auto pNavigation = CNavigation::Create_NavGrid(
			m_pDevice,
			m_pContext,
			navigationContract.runtimePath.c_str());
		if (nullptr == pNavigation ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				navigationContract.prototypeTag,
				std::move(pNavigation))))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLoader::Ready_MapAuthoringCore(const uint32_t iLevelIndex)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) ||
		FAILED(Ready_StaticMeshShader(iLevelIndex)) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			CMapAssetPreview::SHADER_PROTOTYPE_TAG,
			CShader::Create(
				m_pDevice,
				m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl"),
				VTXMESH::Elements,
				VTXMESH::iNumElements))))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("Map: editor core prototypes"));
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxMeshMapInstance"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl"),
			VTXMESHINSTANCE::Elements,
			VTXMESHINSTANCE::iNumElements))) ||
		FAILED(Ready_Camera_Prototype(iLevelIndex)) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_MapAsset"),
			CMapAssetObject::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_MapStaticBatch"),
			CMapStaticBatchObject::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

#ifdef _DEBUG
	if ((iLevelIndex == ETOUI(LEVEL::DEVELOPMENT) ||
		iLevelIndex == ETOUI(LEVEL::BERN)) &&
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_TriggerBox"),
			CTrigger_Box::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}
#endif

	return S_OK;
}

HRESULT CLoader::Ready_Camera_Prototype(
	const uint32_t iLevelIndex)
{
	return CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_Camera_Free"),
		CCamera_Free::Create(m_pDevice, m_pContext));
}

HRESULT CLoader::Ready_StaticMeshShader(
	const uint32_t iLevelIndex)
{
	return CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
			VTXMESH::Elements,
			VTXMESH::iNumElements));
}

HRESULT CLoader::Ready_AnimatedMeshShader(
	const uint32_t iLevelIndex)
{
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
			VTXANIMMESH::Elements,
			VTXANIMMESH::iNumElements))))
	{
		return E_FAIL;
	}
	/* Esther summon NPCs and their screen cutin pin their own copy of the
	   deferred material path so shared-shader pass churn cannot shift them. */
	return CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxEstherNpc"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEstherNpc.hlsl"),
			VTXANIMMESH::Elements,
			VTXANIMMESH::iNumElements));
}

HRESULT CLoader::Ready_DeployPropCore(const uint32_t iLevelIndex)
{
	if (iLevelIndex >= ETOUI(LEVEL::END))
		return E_INVALIDARG;
	std::string status;
	if (!CDeployPropRuntime::Ensure_ObjectPrototype(
		m_pDevice, m_pContext, iLevelIndex, status))
	{
		OutputDebugStringA(("[Loader][DeployProp] " + status +
			"\n").c_str());
		return E_FAIL;
	}
	return S_OK;
}

/* The deploy admission itself lives with CDeployPropRuntime so an Area whose
   loader contract excludes deploy can stage the same prototypes from its own
   Level without a second implementation. */
HRESULT CLoader::Ready_DeployPropArea(
	const uint32_t iLevelIndex,
	const std::string& areaId)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || areaId.empty())
		return E_INVALIDARG;

	std::string status;
	if (!CDeployPropRuntime::Ensure_AreaPrototypes(
		m_pDevice,
		m_pContext,
		iLevelIndex,
		areaId,
		status,
		[this]() -> bool_t
		{
			return m_isCancellationRequested.load(
				std::memory_order_acquire);
		}))
	{
		OutputDebugStringA(("[Loader][DeployProp] " + status +
			"\n").c_str());
		return m_isCancellationRequested.load(std::memory_order_acquire) ?
			HRESULT_FROM_WIN32(ERROR_CANCELLED) : E_FAIL;
	}
	return S_OK;
}
HRESULT CLoader::Ready_Character_Rendering(
	const uint32_t iLevelIndex,
	const std::span<const LostArk::Shared::CHARACTER_CLASS_ID>
		characterClasses)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || characterClasses.empty())
	{
		return E_INVALIDARG;
	}
	for (const auto characterClass : characterClasses)
	{
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
			characterClass))
		{
			return E_INVALIDARG;
		}
	}

	CPlayableCharacterAssetService::Begin_LevelLoad(iLevelIndex);

	if (FAILED(Ready_AnimatedMeshShader(iLevelIndex)) ||
		FAILED(Ready_Character_Shared_Prototypes(iLevelIndex)))
	{
		return E_FAIL;
	}

	for (size_t classIndex = 0;
		classIndex < characterClasses.size();
		++classIndex)
	{
		const auto characterClass = characterClasses[classIndex];
		const auto progress = [
			this,
			classIndex,
			classCount = characterClasses.size(),
			characterClass](
			const size_t completedModelCount,
			const size_t totalModelCount,
			const std::string& assetId)
		{
			const std::wstring fileName =
				std::filesystem::path(assetId).filename().wstring();
			tchar_t status[MAX_PATH]{};
			_snwprintf_s(
				status,
				std::size(status),
				_TRUNCATE,
				TEXT("Character %zu/%zu | %s models %zu/%zu | %s"),
				classIndex + 1u,
				classCount,
				Get_CharacterClassName(characterClass),
				completedModelCount,
				totalModelCount,
				fileName.c_str());
			Set_DeterminateStatus(
				status, completedModelCount, totalModelCount);
		};
		if (FAILED(CPlayableCharacterAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			iLevelIndex,
			characterClass,
			&m_isCancellationRequested,
			progress)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLoader::Ready_Character_Shared_Prototypes(
	const uint32_t iLevelIndex)
{
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_Part_Equipment"),
		CPart_Equipment::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_Part_Body"),
			CPart_Body::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_Character"),
		CCharacter::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Collider_Player"),
		CCollider::Create(m_pDevice, m_pContext, COLLIDER::OBB))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Collider_WorldEntity"),
		CCollider::Create(m_pDevice, m_pContext, COLLIDER::SPHERE))))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CLoader::Ready_AnimationPreviewModels(
	const uint32_t iLevelIndex)
{
#ifdef _DEBUG
	if (iLevelIndex != ETOUI(LEVEL::CHARACTER_SELECT) &&
		iLevelIndex != ETOUI(LEVEL::DEVELOPMENT))
	{
		return E_INVALIDARG;
	}

	constexpr std::array playablePrototypeTags =
	{
		TEXT("Prototype_Component_Model_LanceMaster"),
		TEXT("Prototype_Component_Model_GunSlinger"),
		TEXT("Prototype_Component_Model_Slayer"),
		TEXT("Prototype_Component_Model_Artist"),
		TEXT("Prototype_Component_Model_DimensionMaster"),
		TEXT("Prototype_Component_Model_Warlord")
	};

	for (const ANIMATION_PREVIEW_ASSET& asset :
		ANIMATION_PREVIEW_ASSETS)
	{
		bool_t bPlayablePrototype = false;
		for (const wchar_t* pPlayableTag : playablePrototypeTags)
		{
			if (0 == wcscmp(asset.pPrototypeTag, pPlayableTag))
			{
				bPlayablePrototype = true;
				break;
			}
		}
		// Playable classes are loaded on demand by Character Select. This direct
		// path is only for optional non-playable authoring previews.
		if (bPlayablePrototype)
		{
			continue;
		}
		if (nullptr == asset.pModelAssetId ||
			nullptr == asset.pPrototypeTag ||
			!std::isfinite(asset.fPreviewScale) ||
			asset.fPreviewScale <= 0.f ||
			!std::isfinite(asset.fPreviewYawDegrees))
		{
			return E_FAIL;
		}
		const BOSS_ACTOR_ENTRY* pBoss = nullptr;
		if (nullptr != asset.pBossArchetypeId)
		{
			pBoss = CActorCatalog::Find_Boss(asset.pBossArchetypeId);
			if (nullptr == pBoss || pBoss->bodyModel != asset.pModelAssetId)
				return E_FAIL;

			/* Product Valtan is optional Model View content. It is prepared on
			   selection (or before an actual Server spawn), not on every Character
			   Select entry. */
			continue;
		}
		const filesystem::path path =
			CRuntimeAssetRoot::Resolve(asset.pModelAssetId);
		if (path.empty() || !filesystem::is_regular_file(path))
			continue;
		const matrix_t previewTransform =
			XMMatrixScaling(
				asset.fPreviewScale,
				asset.fPreviewScale,
				asset.fPreviewScale) *
			XMMatrixRotationY(
				XMConvertToRadians(asset.fPreviewYawDegrees));

		unique_ptr<CModel> model = CModel::Create(
			m_pDevice,
			m_pContext,
			MODEL::ANIM,
			path.string().c_str(),
			previewTransform);
		if (nullptr == model)
			continue;
		/* A preview-only body may declare its own donor. The attach is the same
		   contract the product boss uses, so a donor built against another rig
		   fails here instead of previewing a torn mesh. */
		if (nullptr != asset.pAnimationSetAssetId)
		{
			const filesystem::path donorPath =
				CRuntimeAssetRoot::Resolve(asset.pAnimationSetAssetId);
			if (donorPath.empty() || !filesystem::is_regular_file(donorPath))
				return E_FAIL;
			const unique_ptr<CModel> donor = CModel::Create(
				m_pDevice,
				m_pContext,
				MODEL::ANIM,
				donorPath.string().c_str(),
				previewTransform);
			if (nullptr == donor ||
				FAILED(model->Attach_AnimationSet(*donor)))
			{
				return E_FAIL;
			}
		}
		if (nullptr != pBoss)
		{
			const filesystem::path animSetPath =
				CRuntimeAssetRoot::Resolve(pBoss->animationSetId);
			if (!animSetPath.empty() &&
				filesystem::is_regular_file(animSetPath))
			{
				const unique_ptr<CModel> animSet = CModel::Create(
					m_pDevice,
					m_pContext,
					MODEL::ANIM,
					animSetPath.string().c_str(),
					previewTransform);
				if (nullptr == animSet ||
					FAILED(model->Attach_AnimationSet(*animSet)))
				{
					return E_FAIL;
				}
			}
		}
		if (FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				asset.pPrototypeTag,
				move(model))))
		{
			return E_FAIL;
		}

		/* An optional socketed weapon is prepared with its body so Development
		   and Character Select stage the same composition. It is a static mesh
		   riding one bone, so only the unit ratio to the body is applied: the
		   socket bone matrix already carries the preview scale and yaw. */
		const bool_t bDeclaresWeapon =
			nullptr != asset.pWeaponModelAssetId &&
			nullptr != asset.pWeaponPrototypeTag &&
			nullptr != asset.pWeaponSocketBone;
		if (!bDeclaresWeapon)
			continue;
		if (!std::isfinite(asset.fWeaponScale) || asset.fWeaponScale <= 0.f)
			return E_FAIL;
		const filesystem::path weaponPath =
			CRuntimeAssetRoot::Resolve(asset.pWeaponModelAssetId);
		if (weaponPath.empty() || !filesystem::is_regular_file(weaponPath))
			continue;
		unique_ptr<CModel> weaponModel = CModel::Create(
			m_pDevice,
			m_pContext,
			MODEL::NONANIM,
			weaponPath.string().c_str(),
			XMMatrixScaling(
				asset.fWeaponScale,
				asset.fWeaponScale,
				asset.fWeaponScale));
		if (nullptr == weaponModel ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				asset.pWeaponPrototypeTag,
				move(weaponModel))))
		{
			return E_FAIL;
		}
	}
#endif
	return S_OK;
}

HRESULT CLoader::Ready_ValtanPresentation(const uint32_t iLevelIndex)
{
	return CValtanPresentationAssetService::Ensure_Prototypes(
		m_pDevice,
		m_pContext,
		iLevelIndex);
}

unique_ptr<CLoader> CLoader::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eNextLevelID,
	const uint64_t iEffectLoadJobEpoch,
	const uint64_t iEffectCatalogRevision)
{
	auto pInstance = unique_ptr<CLoader>(
		new CLoader(pDevice, pContext));
	if (FAILED(pInstance->Initialize(
		eNextLevelID, iEffectLoadJobEpoch, iEffectCatalogRevision)))
		return nullptr;
	return pInstance;
}

void CLoader::Free()
{
	if (nullptr != m_hThread)
	{
		m_isCancellationRequested.store(true, std::memory_order_release);
		if (nullptr != m_pEffectLoadJob &&
			!m_pEffectLoadJob->Is_Cancelled() &&
			!m_pEffectLoadJob->Is_Closed())
		{
			std::optional<EFFECT_LOAD_JOB_COMMAND> Displaced;
			std::string CancelStatus;
			(void)m_pEffectLoadJob->Post_Command(
				EFFECT_LOAD_JOB_COMMAND::Cancel(
					m_pEffectLoadJob->Get_CurrentEpoch(),
					m_pEffectLoadJob->Get_CurrentCatalogRevision()),
				Displaced,
				CancelStatus);
		}
		const auto FailFastOnWaitFailure = [](const DWORD waitResult) noexcept
		{
			if (WAIT_FAILED != waitResult)
				return;
			const DWORD errorCode = GetLastError();
			OutputDebugStringA(
				"[Loader] Worker wait failed; terminating the process to preserve loader invariants.\n");
			if (!TerminateProcess(
					GetCurrentProcess(),
					ERROR_SUCCESS == errorCode ? ERROR_INVALID_HANDLE : errorCode))
			{
				std::terminate();
			}
			__assume(0);
		};
		DWORD waitResult = WaitForSingleObject(m_hThread, 5000);
		FailFastOnWaitFailure(waitResult);
		if (WAIT_TIMEOUT == waitResult)
		{
			CancelSynchronousIo(m_hThread);
			waitResult = WaitForSingleObject(m_hThread, 5000);
			FailFastOnWaitFailure(waitResult);
		}
		if (WAIT_TIMEOUT == waitResult)
		{
			OutputDebugStringA(
				"[Loader] Worker exceeded shutdown deadline; terminating the process to preserve loader invariants.\n");
			if (!TerminateProcess(GetCurrentProcess(), ERROR_TIMEOUT))
				std::terminate();
			__assume(0);
		}
		CloseHandle(m_hThread);
		m_hThread = nullptr;
	}
}
