# G01 — 마하라카 런타임 MapTool 연결

## 원인과 변경 경계

사용자 화면 Level9에서 Runtime_AuthoringTargets가 빈 값을 반환한다. 마하라카가 runtime
분기와 Load_EditorAreaRegistry의 5개 Area 목록 모두에 없기 때문이다. 결과적으로
Handle_LevelTransition은 authoring Level을 END로 두며 카탈로그와 Layer_Camera를 찾지 못한다.

기존 베른/발탄의 runtime attach 방식을 마하라카에 적용한다. 원본 데이터, 컷신 시간,
카메라 키, NPC, 네트워크 권위, 렌더링 설정은 수정하지 않는다.

## 파일과 함수

- Client/Public/Level_Development.h: Debug 전용으로 기존 map runtime/device/context를 빌려준다.
  Deploy 문서가 없는 마하라카는 attach 계약에 필요한 빈 level-owned Deploy runtime을 둔다.
  authoring active flag는 맵 self-motion만 멈추며 replication은 계속 처리한다.
- Client/Private/Level_Development.cpp: 편집 중 self-motion의 transform 덮어쓰기를 막는다.
- Client/Private/Loader.cpp: 새로 열린 gameplay 편집의 TriggerBox clone을 위해
  Ready_MapAuthoringCore의 Debug prototype 등록 대상에 MAHARAKA를 추가한다.
- Client/Private/MapTool_Area.cpp: 마하라카 registry/게임플레이 문서, runtime targets, batches,
  structural permission, self-motion pause/rebase를 기존 함수에 추가한다.
  uniform navgrid를 SOURCE_PAINT로 잘못 읽지 않는다. 마하라카 navigation bake는 추가하지 않는다.

## 실패와 소유권

기존 source/runtime placement ID·asset ID 검사, catalog stage/commit/rollback을 유지한다.
오래된 게시본은 오류를 표시하며 부분 로드로 Save하지 않는다. MapTool 닫기/Stop은 기존
동작으로 카메라·미리보기 자세를 반환한다. 마하라카 Level의 수명이 containers를 소유한다.
새 C++ 파일과 project/filter 변경은 없다. UTF-8 BOM 없는 기존 인코딩을 유지한다.

## 검증

정상 증분 Debug Product Build와 관련 연결 회귀 검사를 실행한다. Client는 사용자가 직접
실행해 Maharaka → F1 → Open Map Tool → Camera에서 통합 컷신 편집을 끈 채
일반 컷신 목록의 워터팡을 확인한다. 통합 Composition 편집은 발탄·쿠크만 지원한다.
빌드 성공과 사용자 화면 확인은 구분한다. 적용 전문은 아래에 같은 파일 기준으로 보존한다.

## 적용 파일 전문

### Client/Private/Loader.cpp

```cpp
#include "Loader.h"
#include "Engine_VertexTypes.h"

#include "GameInstance.h"
#include "Profiler.h"

#include "ActorCatalog.h"
#include "AnimationPreviewAssets.h"
#include "Camera_Free.h"
#include "Body_Valtan.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "CharacterSelectionState.h"
#include "ClassSelectionPresentation.h"
#include "Collider.h"
#include "DeployPropCatalog.h"
#include "DeployPropObject.h"
#include "DeployPropRuntime.h"
#include "Effect_Catalog.h"
#include "Effect_LoadPreparationJob.h"
#include "Effect_PresentationService.h"
#include "EstherActionSoundCueDocument.h"
#include "EstherCutinPresentationService.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MapAssetCatalog.h"
#include "MapAssetObject.h"
#include "MapAssetPreview.h"
#include "MapNavigationContract.h"
#include "MapPlacementRuntime.h"
#include "MapStaticBatchObject.h"
#include "WorldSequencePlayer.h"
#include "WorldSequenceObject.h"
#include "Navigation.h"
#include "NetworkManager.h"
#include "MonsterPresentationAssetService.h"
#include "KoukuSaydonPresentationAssetService.h"
#include "NpcPlacementPresentationService.h"
#include "NpcPresentationAssetService.h"
#include "Part_Body.h"
#include "Part_Equipment.h"
#include "Part_Vehicle.h"
#include "VehiclePresentationAssetService.h"
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
#include <limits>
#include <unordered_set>
#include <unordered_map>

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
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT:
			return TEXT("GuardianKnight");
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
	const auto reject = [](const HRESULT result, const std::string_view source,
		const std::string_view detail)
	{
		// Record before Create destroys this Loader and MainApp reports the generic null.
		// The transition service preserves the first recovery for this load attempt.
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_LOADING_START_FAILED,
			source, detail, result);
		return result;
	};
	if (nullptr == CLevelRegistry::Find(eNextLevelID))
		return reject(E_INVALIDARG, "loader.initialize.target", "Target Level is not registered.");
	if ((0u == iEffectLoadJobEpoch) != (0u == iEffectCatalogRevision))
		return reject(E_INVALIDARG, "loader.initialize.effect-input",
			"Effect preparation epoch and catalog revision must both be present or absent.");
	if (0u != iEffectLoadJobEpoch)
	{
		m_pEffectLoadJob = std::make_shared<CEffectLoadPreparationJob>();
		std::string JobStatus;
		if (nullptr == m_pEffectLoadJob || !m_pEffectLoadJob->Open(
			iEffectLoadJobEpoch, iEffectCatalogRevision, JobStatus))
		{
			OutputDebugStringA(("[Loader][Effect] " + JobStatus + "\n").c_str());
			m_pEffectLoadJob.reset();
			return reject(E_FAIL, "loader.initialize.effect-job", JobStatus);
		}
	}

	// Stage mutable catalog membership/skills on the owner thread. The loader
	// worker reads this immutable selection even if an explicit F1 reload occurs.
	if (eNextLevelID != LEVEL::LOBBY)
	{
		// ActorCatalog's lazy initialization mutates shared vectors. Finish it
		// on this owner before level and Effect preparation can read them.
		if (!CActorCatalog::Initialize())
		{
			OutputDebugStringA(("[Loader] " + CActorCatalog::Get_Status() + "\n").c_str());
			return reject(E_FAIL, "loader.initialize.actor-catalog", CActorCatalog::Get_Status());
		}
		m_ePreparedCharacterClass = CNetworkManager::Get().Get_LocalCharacterClass();
		// Lobby's accepted enter-world generation preserves this exact class.
		// The optional character-selection UI cache is not admission authority.
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(m_ePreparedCharacterClass))
		{
			Set_Status(TEXT("Character preparation rejected: Server admission has no supported class."));
			OutputDebugStringA("[Loader] Missing/invalid Server-approved character class.\n");
			return reject(E_INVALIDARG, "loader.initialize.character-class",
				"Server admission has no supported character class.");
		}
		const std::array selectedClass = { m_ePreparedCharacterClass };
		std::span<const LostArk::Shared::CHARACTER_CLASS_ID> preparationClasses = selectedClass;
#ifndef _DEBUG
		if (eNextLevelID == LEVEL::CHARACTER_SELECT)
			preparationClasses = CCharacterCatalog::CHARACTER_SELECT_CLASSES;
#endif
		for (const auto characterClass : preparationClasses)
		{
			auto input = CPlayableCharacterAssetService::Capture_AuthoringInput(characterClass);
			if (!input)
				return reject(E_FAIL, "loader.initialize.character-authoring",
					"Character authoring snapshot could not be captured; player skills are unavailable.");
			m_CharacterAuthoringInputs.emplace(characterClass, std::move(input));
		}
	}
	m_eNextLevelID = eNextLevelID;
	m_iResult.store(S_FALSE, std::memory_order_release);
	m_eState.store(STATE::RUNNING, std::memory_order_release);

	// Effect requests are captured and posted by the Loading Level owner.
	// This producer can wait for that mailbox while map/character work starts;
	// it never mutates the target level's prototype registry.
	if (m_pEffectLoadJob &&
		(m_pDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_SINGLETHREADED) == 0u)
	{
		m_hEffectThread = reinterpret_cast<HANDLE>(_beginthreadex(
			nullptr, 0u, &CLoader::EffectThreadMain, this, 0u, nullptr));
		if (!m_hEffectThread)
			OutputDebugStringA("[Loader] Effect producer creation failed; using serial preparation.\n");
	}
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(
		nullptr,
		0,
		ThreadMain,
		this,
		0,
		nullptr));
	if (nullptr == m_hThread)
	{
		Request_Cancellation();
		m_iResult.store(E_FAIL, std::memory_order_release);
		m_eState.store(STATE::FAILED, std::memory_order_release);
		return reject(E_FAIL, "loader.initialize.worker", "Could not create the Level loading worker.");
	}
	return S_OK;
}

HRESULT CLoader::Start_Loading()
{
	/* Worker-thread scopes: the profiler attributes them to the frame in
	   which they end, and long ones surface in its Long operations list. */
	Engine::CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
	HRESULT result = S_OK;
	try
	{
		Engine::CProfilerScope levelScope(pProfiler, "Loader.LevelLoad");
		result = CLevelRegistry::Execute_Load(m_eNextLevelID, *this);
	}
	catch (const std::bad_alloc&) { result = E_OUTOFMEMORY; }
	catch (...) { result = E_FAIL; }
	if (FAILED(result))
		Request_Cancellation();
	if (nullptr != m_hEffectThread)
	{
		// Main keeps consuming the Effect queue while this worker joins. Free
		// owns the bounded shutdown deadline and both producer handles.
		const DWORD waited = WaitForSingleObject(m_hEffectThread, INFINITE);
		if (WAIT_OBJECT_0 != waited)
		{
			OutputDebugStringA("[Loader] Effect producer join failed; terminating the process.\n");
			if (!TerminateProcess(GetCurrentProcess(), ERROR_INVALID_HANDLE))
				std::terminate();
			__assume(0);
		}
		const HRESULT effectResult = static_cast<HRESULT>(
			m_iEffectResult.load(std::memory_order_acquire));
		if (SUCCEEDED(result) || result == HRESULT_FROM_WIN32(ERROR_CANCELLED))
			result = effectResult;
	}
	else if (SUCCEEDED(result) && nullptr != m_pEffectLoadJob)
	{
		try
		{
			Engine::CProfilerScope effectScope(pProfiler, "Loader.EffectPreparation");
			result = Run_EffectLoadPreparation();
		}
		catch (const std::bad_alloc&) { result = E_OUTOFMEMORY; }
		catch (...) { result = E_FAIL; }
	}
	if (SUCCEEDED(result) && m_isCancellationRequested.load(std::memory_order_acquire))
		result = HRESULT_FROM_WIN32(ERROR_CANCELLED);
	if (FAILED(result))
		Request_Cancellation();
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
	return CEffectPresentationService::Run_ProductPreparationWorker(
		m_pDevice, m_pContext, m_pEffectLoadJob, &m_isCancellationRequested,
		&m_EffectPreparationBatch);
}

unsigned __stdcall CLoader::EffectThreadMain(void* pArgument)
{
	auto* loader = static_cast<CLoader*>(pArgument);
	const HRESULT apartment = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	HRESULT result = apartment;
	if (SUCCEEDED(apartment))
	{
		try
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.EffectPreparation");
			result = loader->Run_EffectLoadPreparation();
		}
		catch (const std::bad_alloc&) { result = E_OUTOFMEMORY; }
		catch (...) { result = E_FAIL; }
		CoUninitialize();
	}
	loader->m_iEffectResult.store(result, std::memory_order_release);
	if (FAILED(result))
		loader->Request_Cancellation();
	return FAILED(result) ? 1u : 0u;
}

void CLoader::Request_Cancellation() noexcept
{
	m_isCancellationRequested.store(true, std::memory_order_release);
	try
	{
		if (m_pEffectLoadJob && !m_pEffectLoadJob->Is_Cancelled() &&
			!m_pEffectLoadJob->Is_Closed())
		{
			std::optional<EFFECT_LOAD_JOB_COMMAND> displaced;
			std::string status;
			(void)m_pEffectLoadJob->Post_Command(EFFECT_LOAD_JOB_COMMAND::Cancel(
				m_pEffectLoadJob->Get_CurrentEpoch(),
				m_pEffectLoadJob->Get_CurrentCatalogRevision()), displaced, status);
		}
	}
	catch (...)
	{
		OutputDebugStringA("[Loader] Could not post Effect cancellation; bounded shutdown remains active.\n");
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
		if (nullptr != pStatus)
			++m_iPhaseIndex;
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

void CLoader::Declare_Phases(const size_t iCount)
{
	lock_guard<mutex> lock(m_StatusMutex);
	m_iPhaseIndex = 0u;
	m_iPhaseCount = iCount;
}

void CLoader::Add_Phases(const size_t iCount)
{
	lock_guard<mutex> lock(m_StatusMutex);
	m_iPhaseCount += iCount;
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
		Snapshot.iPhaseIndex = m_iPhaseIndex;
		Snapshot.iPhaseCount = m_iPhaseCount;
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
	Declare_Phases(2u);
	Set_Status(TEXT("LOBBY: stage selection UI"));
	Set_Status(TEXT("Lobby loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_CharacterSelect()
{
	CValtanPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::CHARACTER_SELECT));
	CNpcPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::CHARACTER_SELECT));
	CNpcPlacementPresentationService::Begin_LevelLoad(
		ETOUI(LEVEL::CHARACTER_SELECT));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::CHARACTER_SELECT), "CHARACTER_SELECT_ARENA")))
	{
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
#ifdef _DEBUG
	const std::array characterClasses = { m_ePreparedCharacterClass };
#else
	// Move first-use class model preparation into the entry Loading phase.
	constexpr auto characterClasses = CCharacterCatalog::CHARACTER_SELECT_CLASSES;
#endif

	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::CHARACTER_SELECT));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::CHARACTER_SELECT);

	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId)
		return E_INVALIDARG;

	Declare_Phases(9u);
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
	/* The Server enables the Esther gauge in this test arena too, so the same
	summon roster as VALTAN is admitted here: a lazy first-spawn admission stalls
	the frame the caster presses the key. Missing payload isolates only the summon. */
	Set_Status(TEXT("CHARACTER SELECT: esther summon presentation"));
	Ready_EstherSummonPresentation(ETOUI(LEVEL::CHARACTER_SELECT));
	CVehiclePresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::CHARACTER_SELECT));
#ifndef _DEBUG
	Set_Status(TEXT("CHARACTER SELECT: vehicle presentation"));
	for (const VEHICLE_ACTOR_ENTRY& vehicle : CActorCatalog::Get_Vehicles())
	{
		if (FAILED(CVehiclePresentationAssetService::Ensure_Prototypes(
			m_pDevice, m_pContext, ETOUI(LEVEL::CHARACTER_SELECT), vehicle.vehicleId)))
		{
			OutputDebugStringA(("[Loader][VehiclePresentation] CHARACTER SELECT vehicle " +
				std::to_string(vehicle.vehicleId) + " is unavailable; the level loads on foot.\n").c_str());
		}
	}
#endif
	Set_Status(TEXT("CHARACTER SELECT: class selection cinematics"));
	if (CClassSelectionPresentation::Is_Configured())
	{
		std::vector<std::string> backgroundAreas;
		std::string backgroundStatus;
		if (!CClassSelectionPresentation::Load_BackgroundAreas(pEntry->pMapAreaId,
			pEntry->pPresentationMapAreaId ? pEntry->pPresentationMapAreaId : "",
			backgroundAreas, backgroundStatus))
			OutputDebugStringA(("[Loader][ClassSelectionCinema] " + backgroundStatus + "\n").c_str());
		else for (const auto& backgroundArea : backgroundAreas)
		{
			// The primary Area is already prepared; never register its prototypes twice.
			if (backgroundArea == pEntry->pMapAreaId) continue;
			if (FAILED(Ready_MapArea(ETOUI(LEVEL::CHARACTER_SELECT), backgroundArea,
				pEntry->PresentationMapLoadScope, false)))
			{
				if (m_isCancellationRequested.load(std::memory_order_acquire))
					return HRESULT_FROM_WIN32(ERROR_CANCELLED);
				OutputDebugStringA(("[Loader][ClassSelectionCinema] Optional background unavailable: " +
					backgroundArea + "\n").c_str());
			}
		}
	}
	std::string selectionSequenceStatus;
	if (!CWorldSequencePlayer::Prepare_AreaLoad(ETOUI(LEVEL::CHARACTER_SELECT),
		pEntry->pMapAreaId, pEntry->MapLoadScope, selectionSequenceStatus,
		[this]() { return m_isCancellationRequested.load(std::memory_order_acquire); }))
	{
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		OutputDebugStringA(("[Loader][ClassSelectionCinema] " + selectionSequenceStatus + "\n").c_str());
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
	Declare_Phases(7u);
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
		m_ePreparedCharacterClass
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::BERN),
		selectedClass)))
		return E_FAIL;

	CVehiclePresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::BERN));
#ifndef _DEBUG
	Set_Status(TEXT("BERN: vehicle presentation"));
	for (const VEHICLE_ACTOR_ENTRY& vehicle : CActorCatalog::Get_Vehicles())
	{
		if (FAILED(CVehiclePresentationAssetService::Ensure_Prototypes(
			m_pDevice, m_pContext, ETOUI(LEVEL::BERN), vehicle.vehicleId)))
		{
			OutputDebugStringA(("[Loader][VehiclePresentation] BERN vehicle " +
				std::to_string(vehicle.vehicleId) + " is unavailable; the level loads on foot.\n").c_str());
		}
	}
#endif
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
	Declare_Phases(10u);
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
		m_ePreparedCharacterClass
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
	Ready_EstherSummonPresentation(ETOUI(LEVEL::VALTAN_ARENA));
	Set_Status(TEXT("VALTAN: deploy environment prototypes"));
	if (FAILED(Ready_DeployPropArea(
		ETOUI(LEVEL::VALTAN_ARENA),
		pEntry->pMapAreaId)))
	{
		return E_FAIL;
	}

    Set_Status(TEXT("VALTAN: source cinematic world sequences"));
    // Source actors are used by Server presentation and Workbench before Map
    // Tool is ever opened. Register their clone factory in the Level load,
    // under the same rollback scope as the other Valtan prototypes.
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::VALTAN_ARENA), CWorldSequenceObject::PROTOTYPE_TAG,
        CWorldSequenceObject::Create(m_pDevice, m_pContext))))
    {
        Set_Status(TEXT("VALTAN: source cinematic object prototype failed"));
        return E_FAIL;
    }
    std::string worldSequenceStatus;
    if (!CWorldSequencePlayer::Prepare_AreaLoad(ETOUI(LEVEL::VALTAN_ARENA),
        pEntry->pMapAreaId, pEntry->MapLoadScope, worldSequenceStatus,
        [this]() { return m_isCancellationRequested.load(std::memory_order_acquire); }))
    {
        if (m_isCancellationRequested.load(std::memory_order_acquire))
            return HRESULT_FROM_WIN32(ERROR_CANCELLED);
        OutputDebugStringA(("[Loader][ValtanSourceCinema] " + worldSequenceStatus + "\n").c_str());
    }
	Set_Status(TEXT("Valtan arena loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_KakulSaydonArena()
{
	CKoukuSaydonPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::KAKULSAYDON_ARENA));
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
	Declare_Phases(9u);
	Set_Status(TEXT("KoukuSaydon: arena map"));

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

	Set_Status(TEXT("KoukuSaydon: server-approved character rendering"));
	const std::array selectedClass =
	{
		m_ePreparedCharacterClass
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		selectedClass)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("KoukuSaydon: Server boss presentation"));
	if (FAILED(CKoukuSaydonPresentationAssetService::Ensure_Prototypes(
		m_pDevice,
		m_pContext,
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		"BOSS_KAKULSAYDON_G1_KOUKU")))
	{
		OutputDebugStringA(("[Loader][KoukuSaydonPresentation] " +
			CKoukuSaydonPresentationAssetService::Get_Status() + "\n").c_str());
		return E_FAIL;
	}

	/* Same Esther roster as Valtan (the Server enables the gauge in this raid too); the
	   summons spawn mid-fight, so their models are prototypes before the arena opens. */
	Set_Status(TEXT("KoukuSaydon: esther summon presentation"));
	Ready_EstherSummonPresentation(ETOUI(LEVEL::KAKULSAYDON_ARENA));
	Set_Status(TEXT("KoukuSaydon: deploy environment prototypes"));
	if (FAILED(Ready_DeployPropArea(
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		pEntry->pMapAreaId)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("KoukuSaydon: world sequence document"));
	std::string worldSequenceStatus;
	if (!CWorldSequencePlayer::Prepare_AreaLoad(ETOUI(LEVEL::KAKULSAYDON_ARENA),
		pEntry->pMapAreaId, pEntry->MapLoadScope, worldSequenceStatus,
		[this]() { return m_isCancellationRequested.load(std::memory_order_acquire); }))
	{
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		// Preserve the Level's optional-presentation failure boundary. Activation
		// consumes this exact failure and never reparses on the game frame.
		OutputDebugStringA(("[Loader][WorldSequence] " + worldSequenceStatus + "\n").c_str());
	}

	Set_Status(TEXT("KoukuSaydon arena loading complete"));
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
		Declare_Phases(3u);
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

	Declare_Phases(6u);
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
		m_ePreparedCharacterClass
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

HRESULT CLoader::Ready_For_Maharaka()
{
	CNpcPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::MAHARAKA));
	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::MAHARAKA));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::MAHARAKA), "MAHARAKA")))
	{
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::MAHARAKA));
	Declare_Phases(6u);
	Set_Status(TEXT("MAHARAKA: island catalog and placements"));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::MAHARAKA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::MAHARAKA),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("MAHARAKA: session character bundle"));
	const std::array selectedClass =
	{
		m_ePreparedCharacterClass
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::MAHARAKA),
		selectedClass)))
		return E_FAIL;

	Set_Status(TEXT("Maharaka loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_MapArea(
	const uint32_t iLevelIndex,
	const std::string& areaId,
	const MAP_LOAD_SCOPE& loadScope, const bool_t prepareMapCore)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || areaId.empty())
		return E_INVALIDARG;

	// A failed optional preparation must not leave a previous visit's stage available.
	CMapPlacementRuntime::Discard_LoadStage(areaId);
	try
	{
		if (prepareMapCore && FAILED(Ready_MapAuthoringCore(iLevelIndex)))
			return E_FAIL;

		CMapAssetCatalog mapCatalog;
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.Map.Catalog");
			Set_Status(TEXT("Map: explicit area catalog"));
			if (!mapCatalog.Load_Area(areaId))
			{
				{
					lock_guard<mutex> activeLock(g_ActiveStatusMutex);
					g_ActiveStatus = "Map " + areaId + ": " + mapCatalog.Get_Status();
				}
				OutputDebugStringA(("[Loader][Map] " + mapCatalog.Get_Status() + "\n").c_str());
				return E_FAIL;
			}
		}

		std::unordered_set<std::string> requiredAssetIds;
		std::vector<MAP_PLACEMENT_RECORD> scopedPlacements;
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.Map.Scope");
			if (loadScope.isEnabled)
			{
				std::string placementStatus;
				Add_Phases(1u);
				Set_Status(TEXT("Map: product load scope"));
				if (!CMapPlacementRuntime::Read_Placements(
					mapCatalog, scopedPlacements, placementStatus))
				{
					{
						lock_guard<mutex> activeLock(g_ActiveStatusMutex);
						g_ActiveStatus = "Map " + areaId + ": product load scope: " + placementStatus;
					}
					OutputDebugStringA(("[Loader][Map] " + placementStatus + "\n").c_str());
					return E_FAIL;
				}
				CMapPlacementRuntime::Apply_LoadScope(mapCatalog, loadScope, scopedPlacements);
				for (const MAP_PLACEMENT_RECORD& record : scopedPlacements)
					requiredAssetIds.insert(record.assetId);
				if (requiredAssetIds.empty())
				{
					lock_guard<mutex> activeLock(g_ActiveStatusMutex);
					g_ActiveStatus = "Map " + areaId + ": product load scope selected no placements";
					return E_FAIL;
				}
			}
		}

		const size_t requiredModelCount = loadScope.isEnabled ?
			requiredAssetIds.size() : mapCatalog.Get_Entries().size();
		std::vector<const MAP_ASSET_ENTRY*> entries;
		std::vector<std::vector<size_t>> groups;
		std::unordered_map<std::wstring, size_t> geometryGroups;
		entries.reserve(requiredModelCount);
		// Catalog order selects the same physical base and material/LOD inputs
		// as the serial loader; scheduling only changes independent groups.
		for (const MAP_ASSET_ENTRY& entry : mapCatalog.Get_Entries())
		{
			if (loadScope.isEnabled && !requiredAssetIds.contains(entry.id))
				continue;
			const size_t index = entries.size();
			entries.push_back(&entry);
			const std::wstring key = entry.resolvedModelPath.lexically_normal().wstring();
			const auto [found, inserted] = geometryGroups.emplace(key, groups.size());
			if (inserted)
				groups.emplace_back();
			groups[found->second].push_back(index);
		}
		if (entries.size() != requiredModelCount)
			return E_FAIL;

		std::vector<std::pair<wstring_t, unique_ptr<CPrototype>>> staged(requiredModelCount);
		for (size_t index = 0u; index < entries.size(); ++index)
			staged[index].first = entries[index]->prototypeTag;
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.Map.Models");
			const auto started = std::chrono::steady_clock::now();
			const auto assetRoot = CRuntimeAssetRoot::Get();
			std::vector<size_t> failedModels(groups.size(), (std::numeric_limits<size_t>::max)());
			std::atomic_bool failed = false;
			std::mutex progressMutex;
			size_t completed = 0u;
			Set_DeterminateStatus(TEXT("Map: model prototypes"), 0u, requiredModelCount);
			const auto preparation = m_AssetPreparationBatch.Run(m_pDevice.Get(), groups.size(),
				&m_isCancellationRequested, [&](const size_t groupIndex) -> HRESULT
				{
					size_t activeIndex = (std::numeric_limits<size_t>::max)();
					try
					{
						Engine::CProfilerScope workerScope(CGameInstance::Get().Get_Profiler(),
							"Loader.Map.ModelWorker");
						const matrix_t transform = XMMatrixScaling(0.01f, 0.01f, 0.01f);
						const CModel* geometry = nullptr;
						for (const size_t index : groups[groupIndex])
						{
							if (failed.load(std::memory_order_acquire))
								return S_OK; // The failing group's result owns the actual error.
							if (m_isCancellationRequested.load(std::memory_order_acquire))
								return HRESULT_FROM_WIN32(ERROR_CANCELLED);
							activeIndex = index;
							const MAP_ASSET_ENTRY& entry = *entries[index];
							Engine::MODEL_ASSET_LOAD_DESC loadDesc;
							loadDesc.assetRoot = assetRoot;
							loadDesc.meshPath = entry.resolvedModelPath;
							loadDesc.materialOverrides = entry.materialOverrides;
							auto model = geometry ?
								CModel::Create_MaterialVariant(*geometry, loadDesc) :
								CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, loadDesc, transform);
							if (!model)
							{
								failedModels[groupIndex] = index;
								failed.store(true, std::memory_order_release);
								return E_FAIL;
							}
							if (!geometry)
								geometry = model.get();
							// The group's first staged model keeps geometry alive. No worker
							// changes the container or another group's catalog slots.
							staged[index].second = std::move(model);
							{
								std::scoped_lock progressLock(progressMutex);
								++completed;
								Set_DeterminateStatus(TEXT("Map: model prototypes"), completed, requiredModelCount);
							}
						}
						return S_OK;
					}
					catch (...)
					{
						failedModels[groupIndex] = activeIndex;
						failed.store(true, std::memory_order_release);
						throw; // The common worker boundary preserves E_OUTOFMEMORY/E_FAIL.
					}
				});
			HRESULT result = preparation.result;
			if (SUCCEEDED(result) && m_isCancellationRequested.load(std::memory_order_acquire))
				result = HRESULT_FROM_WIN32(ERROR_CANCELLED);
			if (SUCCEEDED(result) && completed != requiredModelCount)
				result = E_FAIL;
			const double elapsedMs = std::chrono::duration<double, std::milli>(
				std::chrono::steady_clock::now() - started).count();
			char diagnostic[512]{};
			sprintf_s(diagnostic,
				"[Loader][Map] area=%s models=%zu geometryGroups=%zu variants=%zu workers=%u prepared=%zu elapsedMs=%.3f result=0x%08lX\n",
				areaId.c_str(), requiredModelCount, groups.size(),
				requiredModelCount - groups.size(), preparation.workerCount, completed, elapsedMs,
				static_cast<unsigned long>(result));
			OutputDebugStringA(diagnostic);
			if (FAILED(result))
			{
				const size_t failedIndex = preparation.failedTask < failedModels.size() ?
					failedModels[preparation.failedTask] : (std::numeric_limits<size_t>::max)();
				if (failedIndex < entries.size())
				{
					const MAP_ASSET_ENTRY& entry = *entries[failedIndex];
					{
						lock_guard<mutex> activeLock(g_ActiveStatusMutex);
						g_ActiveStatus = "Map " + areaId + ": model prototype failed " +
							entry.id + " / " + entry.modelRelativePath.generic_string() +
							" (" + std::to_string(completed) + "/" +
							std::to_string(requiredModelCount) + ")";
					}
					OutputDebugStringW((L"[Loader][Map] Model failed: " + entry.prototypeTag +
						L" / " + entry.resolvedModelPath.wstring() + L"\n").c_str());
				}
				return result;
			}
		}

		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.Map.Navigation");
			MAP_NAVIGATION_CONTRACT navigationContract;
			std::string navigationStatus;
			Set_Status(TEXT("Map: navigation contract"));
			if (!CMapNavigationContract::Resolve_Area(areaId, navigationContract, navigationStatus))
			{
				OutputDebugStringA(("[Loader][Map] " + navigationStatus + "\n").c_str());
				return E_FAIL;
			}
			if (navigationContract.runtimeGridAvailable)
			{
				f32_t maximumStepHeight = 0.f;
				if (!CMapNavigationContract::Read_RuntimeStepHeight(
					navigationContract, maximumStepHeight, navigationStatus))
				{
					OutputDebugStringA(("[Loader][Map] " + navigationStatus + "\n").c_str());
					return E_FAIL;
				}
				auto navigation = CNavigation::Create_NavGrid(m_pDevice, m_pContext,
					navigationContract.runtimePath.c_str(), maximumStepHeight);
				if (!navigation)
					return E_FAIL;
				staged.emplace_back(navigationContract.prototypeTag, std::move(navigation));
			}
		}
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);

		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.Map.Commit");
			if (!staged.empty() &&
				FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
				return E_FAIL;
			// Failed model/navigation preparation never publishes a placement stage.
			if (loadScope.isEnabled)
				CMapPlacementRuntime::Cache_LoadStage(areaId, loadScope, mapCatalog, scopedPlacements);
		}
		return S_OK;
	}
	catch (const std::bad_alloc&)
	{
		OutputDebugStringA("[Loader][Map] Map preparation allocation failed.\n");
		return E_OUTOFMEMORY;
	}
	catch (...)
	{
		OutputDebugStringA("[Loader][Map] Map preparation failed with an exception.\n");
		return E_FAIL;
	}
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
	/* Every level the Map Tool can author stages its trigger, collision and
	   spawn anchor boxes as TriggerBox clones under that level's index. */
	if ((iLevelIndex == ETOUI(LEVEL::DEVELOPMENT) ||
		iLevelIndex == ETOUI(LEVEL::MAHARAKA) ||
		iLevelIndex == ETOUI(LEVEL::CHARACTER_SELECT) ||
		iLevelIndex == ETOUI(LEVEL::BERN) ||
		iLevelIndex == ETOUI(LEVEL::VALTAN_ARENA) ||
		iLevelIndex == ETOUI(LEVEL::KAKULSAYDON_ARENA)) &&
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
	auto shader = CShader::Create(
		m_pDevice,
		m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
		VTXMESH::Elements,
		VTXMESH::iNumElements);
	if (!shader)
	{
		Set_Status(TEXT("Map: Shader_VtxMeshBinary creation failed (compiled shader or program variants)"));
		OutputDebugStringA("[Loader][Shader] Shader_VtxMeshBinary creation failed before prototype registration.\n");
		return E_FAIL;
	}
	const HRESULT result = CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		std::move(shader));
	if (FAILED(result))
	{
		Set_Status(TEXT("Map: Shader_VtxMeshBinary prototype registration failed"));
		OutputDebugStringA("[Loader][Shader] Shader_VtxMeshBinary prototype registration failed.\n");
	}
	return result;
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
		const auto authoring = m_CharacterAuthoringInputs.find(characterClass);
		if (authoring == m_CharacterAuthoringInputs.end() || !authoring->second) return E_INVALIDARG;
		const auto progress = [
			this,
			classIndex,
			classCount = characterClasses.size(),
			characterClass](
			const size_t completedModelCount,
			const size_t totalModelCount,
			const std::string&)
		{
			tchar_t status[MAX_PATH]{};
			_snwprintf_s(
				status,
				std::size(status),
				_TRUNCATE,
				TEXT("Character %zu/%zu | %s models"),
				classIndex + 1u,
				classCount,
				Get_CharacterClassName(characterClass));
			Set_DeterminateStatus(
				status, completedModelCount, totalModelCount);
		};
		if (FAILED(CPlayableCharacterAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			iLevelIndex,
			characterClass,
			&m_isCancellationRequested,
			progress,
			authoring->second,
			&m_AssetPreparationBatch)))
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
			TEXT("Prototype_GameObject_Part_Vehicle"),
			CPart_Vehicle::Create(m_pDevice, m_pContext))) ||
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
		TEXT("Prototype_Component_Model_Warlord"),
		TEXT("Prototype_Component_Model_GuardianKnight")
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
	if (FAILED(CValtanPresentationAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			iLevelIndex,
			"BOSS_VALTAN")))
	{
		return E_FAIL;
	}
	/* The finale repeatedly checks out the ghost rig.  Register its body and
	animation-set prototypes while the arena loader still owns the loading
	window, never on the first visible dependent-boss spawn. */
	return CValtanPresentationAssetService::Ensure_Prototypes(
		m_pDevice,
		m_pContext,
		iLevelIndex,
		"BOSS_VALTAN_GHOST");
}

void CLoader::Ready_EstherSummonPresentation(const uint32_t iLevelIndex)
{
	for (const char* pEstherArchetypeId :
		{ "NPC_59030", "NPC_58700", "NPC_59060", "NPC_59504", "NPC_59620" })
	{
		if (FAILED(CNpcPresentationAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			iLevelIndex,
			pEstherArchetypeId)))
		{
			OutputDebugStringA(
				(std::string("[Loader][NpcPresentation] esther summon "
					"presentation is unavailable (") + pEstherArchetypeId +
					"); the arena loads without it.\n").c_str());
		}
		(void)CEstherCutinPresentationService::Preload_Frames(
			m_pDevice.Get(), pEstherArchetypeId);
	}
	(void)CEstherActionSoundCueDocument::Preload_Sounds();
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
	Request_Cancellation();
	HANDLE workers[2]{};
	DWORD workerCount = 0u;
	if (m_hThread)
		workers[workerCount++] = m_hThread;
	if (m_hEffectThread)
		workers[workerCount++] = m_hEffectThread;
	if (workerCount != 0u)
	{
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
		DWORD waitResult = WaitForMultipleObjects(workerCount, workers, TRUE, 5000);
		FailFastOnWaitFailure(waitResult);
		if (WAIT_TIMEOUT == waitResult)
		{
			for (DWORD index = 0u; index < workerCount; ++index)
				CancelSynchronousIo(workers[index]);
			m_AssetPreparationBatch.Cancel_SynchronousIo();
			m_EffectPreparationBatch.Cancel_SynchronousIo();
			waitResult = WaitForMultipleObjects(workerCount, workers, TRUE, 5000);
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
		for (DWORD index = 0u; index < workerCount; ++index)
			CloseHandle(workers[index]);
		m_hThread = nullptr;
		m_hEffectThread = nullptr;
	}
}
```


### Client/Public/Level_Development.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#ifdef _DEBUG
#include "DeployPropRuntime.h"
#endif
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "PlayerController.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CMapLightPresentationRuntime;
class IPlayerCommandSink;

class CLevel_Development final : public CLevel
{
private:
	CLevel_Development(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eLevel);

public:
	virtual ~CLevel_Development();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	// The current Development/Training or Maharaka owner exposes its existing camera.
	static CLevel_Development* Get_Active(LEVEL level)
	{
		return s_pActiveInstance && s_pActiveInstance->m_eLevel == level ?
			s_pActiveInstance : nullptr;
	}
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera.lock(); }
#ifdef _DEBUG
	// Borrow the existing Maharaka map; the Level remains its owner.
	CMapPlacementRuntime& Get_MapAuthoringRuntime() { return m_MapRuntime; }
	CDeployPropRuntime& Get_MapAuthoringDeploy() { return m_MapAuthoringDeploy; }
	const ComPtr<ID3D11Device>& Get_MapAuthoringDevice() const { return m_pDevice; }
	const ComPtr<ID3D11DeviceContext>& Get_MapAuthoringContext() const { return m_pContext; }
	void Set_MapAuthoringActive(bool_t active) { m_bMapAuthoringActive = active; }
	void Rebase_MapAuthoringSelfMotions(const std::vector<MAP_PLACEMENT_RECORD>& records)
	{ m_MapRuntime.Rebase_AuthoringSelfMotions(records); }
#endif

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera(const wstring_t& strLayerTag);
	bool_t Bind_CameraToLocalCharacter();

private:
	// Registry entry this instance plays; only DEVELOPMENT may open the Map Editor.
	LEVEL m_eLevel = LEVEL::DEVELOPMENT;
	CMapPlacementRuntime m_MapRuntime;
#ifdef _DEBUG
	// Maharaka has no Deploy source pair. Runtime attach still needs a live owner.
	CDeployPropRuntime m_MapAuthoringDeploy;
	bool_t m_bMapAuthoringActive = false;
#endif
	// Maharaka only: the published source lights of the island, submitted every frame.
	shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	bool_t m_bMapLightSubmissionFailureReported = false;
	bool_t m_isMapEditorWorkspace = false;
	weak_ptr<CCamera_Free> m_pCamera;
	weak_ptr<CCharacter> m_pCameraTarget;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	CPlayerController m_PlayerController;
	static CLevel_Development* s_pActiveInstance;

public:
	static unique_ptr<CLevel_Development> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eLevel = LEVEL::DEVELOPMENT);
};

NS_END
```

### Client/Private/Level_Development.cpp

```cpp
#include "Level_Development.h"

#include "Camera_Free.h"
#include "ArenaCameraProfile.h"
#include "Character.h"
#include "CharacterSelectionState.h"
#include "CombatHUDViewModel.h"
#include "EffectFailureDiagnostic.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MainApp.h"
#include "MapLightPresentationRuntime.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "Transform.h"

#ifdef _DEBUG
#include "MapEditorWorkspaceService.h"
#endif

CLevel_Development* CLevel_Development::s_pActiveInstance = nullptr;

CLevel_Development::CLevel_Development(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eLevel)
	: CLevel{ pDevice, pContext }
	, m_eLevel{ eLevel }
{
	s_pActiveInstance = this;
}

CLevel_Development::~CLevel_Development()
{
	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
#ifdef _DEBUG
	if (m_isMapEditorWorkspace)
	{
		CCombatHUDViewModel::Get().Reset_RuntimeState();
		CMapEditorWorkspaceService::Cancel();
	}
#endif
}

HRESULT CLevel_Development::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

#ifdef _DEBUG
	m_isMapEditorWorkspace =
		LEVEL::DEVELOPMENT == m_eLevel &&
		CMapEditorWorkspaceService::Is_Requested();
	if (m_isMapEditorWorkspace)
	{
		auto previewClass =
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER;
		CCharacterSelectionState::Try_Get_SelectedClass(previewClass);

		if (FAILED(Ready_Lights()) ||
			FAILED(Ready_Camera(TEXT("Layer_Camera"))) ||
			!CCombatHUDViewModel::Get().Initialize_Definitions() ||
			!CCombatHUDViewModel::Get().Apply_CharacterPreview(previewClass))
		{
			CCombatHUDViewModel::Get().Reset_RuntimeState();
			CMapEditorWorkspaceService::Cancel();
			return E_FAIL;
		}
		CMapEditorWorkspaceService::Set_Active(true);
		return S_OK;
	}
#endif

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(m_eLevel);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		!m_MapRuntime.Load_Area(
			ETOUI(m_eLevel),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope))
	{
		OutputDebugStringA((
			"[Level_Development] " + m_MapRuntime.Get_Status() + "\n").c_str());
		Write_EffectFailureDiagnostic("map.area.load-failed",
			m_MapRuntime.Get_Status());
		return E_FAIL;
	}

	// Report what the Maharaka documents delivered: WATER assets and their water rows,
	// and assets that carry source material rows. Written once per level entry.
	if (LEVEL::MAHARAKA == m_eLevel)
	{
		const CMapAssetCatalog& catalog = m_MapRuntime.Get_Catalog();
		size_t waterAssets = 0u;
		size_t waterRows = 0u;
		size_t materialAssets = 0u;
		for (const MAP_ASSET_ENTRY& entry : catalog.Get_Entries())
		{
			if (MAP_ASSET_RENDER_MODE::WATER == entry.renderProfile.renderMode)
			{
				++waterAssets;
				if (nullptr != catalog.Find_Water(entry.id))
					++waterRows;
			}
			if (!entry.materialOverrides.empty())
				++materialAssets;
		}
		Write_EffectFailureDiagnostic("map.water.loaded",
			"area=" + catalog.Get_AreaId() +
			" assets=" + std::to_string(catalog.Get_Entries().size()) +
			" waterAssets=" + std::to_string(waterAssets) +
			" waterRows=" + std::to_string(waterRows) +
			" materialAssets=" + std::to_string(materialAssets));
	}

	// The island's EFActorMotion rows (turning and rocking props). An absent
	// document is not an error; a rejected one is reported and the island stays static.
	if (LEVEL::MAHARAKA == m_eLevel &&
		!m_MapRuntime.Load_SelfMotions(pEntry->pMapAreaId))
	{
		OutputDebugStringA(
			"[Level_Development] Self-motion document was rejected.\n");
	}

	if (FAILED(Ready_Lights()) ||
		FAILED(Ready_Camera(TEXT("Layer_Camera"))))
	{
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	CClientReplication::DESC replicationDesc{};
	replicationDesc.pDevice = m_pDevice;
	replicationDesc.pContext = m_pContext;
	replicationDesc.iPrototypeLevelIndex = ETOUI(m_eLevel);
	replicationDesc.iLayerLevelIndex = ETOUI(m_eLevel);
	replicationDesc.strMapAreaId = pEntry->pMapAreaId;
	replicationDesc.strPlayerLayerTag = TEXT("Layer_Player");
	replicationDesc.strWorldEntityLayerTag = TEXT("Layer_WorldEntity");
	if (!m_Replication.Initialize(replicationDesc))
	{
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
	if (!m_PlayerController.Initialize_TargetingPreview(
			ETOUI(m_eLevel)))
	{
		return E_FAIL;
	}
	if (!m_PlayerController.Initialize_ClickMoveEffect(
			ETOUI(m_eLevel)))
	{
		return E_FAIL;
	}
	return S_OK;
}

void CLevel_Development::Update(const f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_isMapEditorWorkspace)
		return;

	if (LEVEL::MAHARAKA == m_eLevel)
	{
#ifdef _DEBUG
		// MapTool owns sampled poses while editing; keep replication and lights live.
		if (!m_bMapAuthoringActive)
#endif
		m_MapRuntime.Update_SelfMotions(fTimeDelta);
		if (m_pMapLightPresentation &&
			!m_pMapLightPresentation->Submit_Frame() &&
			!m_bMapLightSubmissionFailureReported)
		{
			m_bMapLightSubmissionFailureReported = true;
			OutputDebugStringA(("[Level_Development][MapLight] " +
				m_pMapLightPresentation->Get_Status() + "\n").c_str());
		}
	}

	if (!m_Replication.Update())
	{
		OutputDebugStringA(
			"[Level_Development] Failed to apply training replication.\n");
	}
	if (m_Replication.Has_PendingConnectionLoss())
	{
		CLevelTransitionService::Report_NetworkRecovery(
			"level-development.network-connection-lost",
			"Training replication observed a disconnected Server session.");
		CNetworkManager::Get().Close_ServerConnection();
		if (CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"network.connection-lost"))
		{
			m_Replication.Acknowledge_ConnectionLoss();
			return;
		}
		OutputDebugStringA(
			"[Level_Development] Lobby recovery request was rejected; retrying.\n");
	}

	Bind_CameraToLocalCharacter();
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	m_PlayerController.Set_LocalCharacter(localCharacter);
	const shared_ptr<CCamera_Free> camera = m_pCamera.lock();
	m_PlayerController.Update(
		nullptr != camera && camera->Is_FollowEnabled());
}

HRESULT CLevel_Development::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(m_isMapEditorWorkspace ?
		TEXT("LostArk Map Editor Workspace") :
		LEVEL::MAHARAKA == m_eLevel ?
		TEXT("LostArk Maharaka Paradise") :
		TEXT("LostArk Test Training Ground"));
#endif
	return S_OK;
}

HRESULT CLevel_Development::Ready_Lights()
{
	if (LEVEL::MAHARAKA != m_eLevel)
		return S_OK;

	// The island's Area declares a light pair, so a missing or rejected document
	// fails the load like Character Select does instead of showing an unlit island.
	const CLIENT_LEVEL_DESCRIPTOR* pEntry = CLevelRegistry::Find(m_eLevel);
	auto staged = make_shared<CMapLightPresentationRuntime>();
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		!staged->Load_Runtime(pEntry->pMapAreaId))
	{
		OutputDebugStringA(("[Level_Development][MapLight] " +
			staged->Get_Status() + "\n").c_str());
		return E_FAIL;
	}
	m_pMapLightPresentation = std::move(staged);
	m_bMapLightSubmissionFailureReported = false;
	return S_OK;
}

HRESULT CLevel_Development::Ready_Camera(
	const wstring_t& strLayerTag)
{
	// Training/Maharaka share the saved Character Select sizes, independent
	// of whichever arena last wrote the process presentation profile.
	ARENA_CAMERA_PROFILE sizeProfile = CArenaCameraProfile::Default(ARENA_CAMERA_MAP::CHARACTER_SELECT);
	std::string sizeStatus;
	if (!CArenaCameraProfile::Load(ARENA_CAMERA_MAP::CHARACTER_SELECT, sizeProfile, sizeStatus))
		OutputDebugStringA(("[Level_Development][CharacterSize] " + sizeStatus + "\n").c_str());
	CCharacter::Set_MapPresentationSizeProfile(sizeProfile);
	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(-18.f, 10.f, -18.f);
	cameraDesc.vAt = float3_t(0.f, 3.f, 0.f);
	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = 4000.f;
	cameraDesc.fSpeedPerSec = 20.f;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.pFollowTarget = nullptr;
	cameraDesc.vPositionOffset = float3_t(0.4f, 7.5f, 4.5f);
	cameraDesc.vLookOffset = float3_t(0.f, 1.2f, 0.f);
	cameraDesc.fFollowResponse = 18.f;
	cameraDesc.isFollowEnabled = false;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(m_eLevel),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(m_eLevel),
		strLayerTag,
		&cameraDesc,
		&gameObject)))
	{
		return E_FAIL;
	}

	const shared_ptr<CCamera_Free> camera =
		dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == camera)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(m_eLevel),
			strLayerTag,
			gameObject);
		return E_FAIL;
	}

	m_pCamera = camera;
	return S_OK;
}

bool_t CLevel_Development::Bind_CameraToLocalCharacter()
{
	const shared_ptr<CCamera_Free> camera = m_pCamera.lock();
	if (nullptr == camera)
		return false;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter)
	{
		m_pCameraTarget.reset();
		camera->Set_FollowTarget(nullptr);
		camera->Set_FollowEnabled(false);
		return true;
	}
	if (m_pCameraTarget.lock() == localCharacter)
		return true;

	const shared_ptr<CTransform> transform =
		localCharacter->Get_Transform();
	if (nullptr == transform)
		return false;

	m_pCameraTarget = localCharacter;
	camera->Set_PositionOffset(float3_t(0.4f, 7.5f, 4.5f));
	camera->Set_FollowTarget(transform);
	camera->Set_FollowEnabled(true);
	return true;
}

unique_ptr<CLevel_Development> CLevel_Development::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eLevel)
{
	auto instance = unique_ptr<CLevel_Development>(
		new CLevel_Development(pDevice, pContext, eLevel));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}
```

### Client/Private/MapTool_Area.cpp

```cpp
#include "imgui.h"
#include "MapTool_Internal.h"
#include "WorldSequenceToolPanel.h"
#include "Camera_Free.h"
#include "DataJson.h"
#include "ActorCatalog.h"
#include "GameInstance.h"
#include "MapEditorWorkspaceService.h"
#include "Level_Bern.h"
#include "Level_CharacterSelect.h"
#include "Level_Development.h"
#include "Level_KakulSaydonArena.h"
#include "Level_ValtanArena.h"
#include "MapAssetPreview.h"
#include "DestructionSimulationController.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>




bool_t Client::CMapTool::Ensure_AuthoringPrototypes()
{
	return Ensure_AuthoringPrototypes(m_Catalog);
}

bool_t Client::CMapTool::Admit_AuthoringPrototype(
	const MAP_ASSET_ENTRY& asset)
{
	/* This tool owns every map prototype it admits into the authoring level,
	   so its own fingerprint already answers the identity question. Probing
	   the engine instead deep clones an admitted model only to drop it, which
	   an Area the size of Bern would pay for a thousand times per switch. */
	const auto fingerprint = m_PrototypeModelPaths.find(asset.prototypeTag);
	if (fingerprint != m_PrototypeModelPaths.end())
	{
		if (fingerprint->second.lexically_normal() !=
			asset.resolvedModelPath.lexically_normal())
		{
			m_Status = "Prototype tag already belongs to another model: " +
				asset.id;
			return false;
		}
		return true;
	}

	const shared_ptr<CModel> existing =
		dynamic_pointer_cast<CModel>(
			CGameInstance::Get().Clone_Prototype(
				m_iAuthoringLevelIndex,
				asset.prototypeTag));
	if (nullptr != existing)
	{
		m_Status = "Prototype tag already belongs to another model: " +
			asset.id;
		return false;
	}

	Engine::MODEL_ASSET_LOAD_DESC loadDesc;
	loadDesc.assetRoot = CRuntimeAssetRoot::Get();
	loadDesc.meshPath = asset.resolvedModelPath;
	loadDesc.materialOverrides = asset.materialOverrides;
	auto model = CModel::Create(
		m_pDevice,
		m_pContext,
		MODEL::NONANIM,
		loadDesc,
		XMMatrixScaling(0.01f, 0.01f, 0.01f));
	if (nullptr == model ||
		FAILED(CGameInstance::Get().Add_Prototype(
			m_iAuthoringLevelIndex,
			asset.prototypeTag,
			std::move(model))))
	{
		m_Status = "Map authoring model admission failed: " + asset.id;
		return false;
	}
	m_PrototypeModelPaths.emplace(
		asset.prototypeTag,
		asset.resolvedModelPath.lexically_normal());
	return true;
}

bool_t Client::CMapTool::Ensure_AuthoringPrototypes(
	const CMapAssetCatalog& catalog)
{
	if (!catalog.Is_Ready() ||
		m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status = "Map authoring prototype admission is unavailable";
		return false;
	}

	for (const MAP_ASSET_ENTRY& asset : catalog.Get_Entries())
	{
		if (!Admit_AuthoringPrototype(asset))
			return false;
	}

	m_Status = "Map authoring admitted " +
		std::to_string(catalog.Get_Entries().size()) +
		" model prototypes";
	return true;
}

bool_t Client::CMapTool::Ensure_DeployAuthoringPrototypes(
	const CDeployPropCatalog& catalog)
{
	if (!catalog.Is_Ready() ||
		m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status = "DeployProp authoring prototype admission is unavailable";
		return false;
	}

	const matrix_t modelTransform =
		XMMatrixScaling(0.01f, 0.01f, 0.01f);
	const auto admitModel = [this, &modelTransform](
		const std::wstring& prototypeTag,
		const std::filesystem::path& modelPath,
		const std::filesystem::path& relativeModelPath,
		const MODEL modelKind,
		const std::string& assetId)
	{
		const shared_ptr<CModel> existing = dynamic_pointer_cast<CModel>(
			CGameInstance::Get().Clone_Prototype(
				m_iAuthoringLevelIndex, prototypeTag));
		if (nullptr != existing)
		{
			const auto fingerprint = m_PrototypeModelPaths.find(prototypeTag);
			return fingerprint != m_PrototypeModelPaths.end() &&
				fingerprint->second.lexically_normal() ==
					modelPath.lexically_normal();
		}

		MODEL_ASSET_LOAD_DESC loadDesc;
		if (!CActorCatalog::Build_ModelLoadDescription(
			relativeModelPath.generic_string(), loadDesc, m_Status))
			return false;

		auto model = CModel::Create(
			m_pDevice,
			m_pContext,
			modelKind,
			loadDesc,
			modelTransform);
		if (nullptr == model || FAILED(CGameInstance::Get().Add_Prototype(
			m_iAuthoringLevelIndex,
			prototypeTag,
			std::move(model))))
		{
			m_Status = "DeployProp model admission failed: " + assetId;
			return false;
		}
		m_PrototypeModelPaths.emplace(
			prototypeTag, modelPath.lexically_normal());
		return true;
	};

	for (const DEPLOY_PROP_ASSET_ENTRY& asset : catalog.Get_Assets())
	{
		const MODEL modelKind =
			DEPLOY_PROP_MODEL_KIND::ANIM == asset.kind ?
			MODEL::ANIM : MODEL::NONANIM;
		if (!admitModel(
			asset.intactPrototypeTag,
			asset.intactResolvedPath,
			asset.intactRelativePath,
			modelKind,
			asset.id))
		{
			return false;
		}
		if (DEPLOY_PROP_MODEL_KIND::STATIC == asset.kind &&
			!asset.fracturedPrototypeTag.empty() &&
			!admitModel(
				asset.fracturedPrototypeTag,
				asset.fracturedResolvedPath,
				asset.fracturedRelativePath,
				MODEL::NONANIM,
				asset.id))
		{
			return false;
		}
	}
	return true;
}

bool_t Client::CMapTool::Load_EditorAreaRegistry()
{
	const std::filesystem::path path =
		CProjectDataRoot::Resolve(L"Maps/MapCatalog.json");
	std::string text;
	std::string parseError;
	DATA_JSON_VALUE root;
	if (path.empty() || !ReadTextFile(path, text) ||
		!CDataJson::Parse(text, root, parseError) || !root.Is_Object())
	{
		m_Status = "Map editor catalog parse failed: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = root.Find("schema");
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* areas = root.Find("areas");
	if (nullptr == schema || !schema->Is_String() ||
		schema->Get_String() != "lostark.map-catalog" ||
		nullptr == version || !version->Is_Number() ||
		version->Get_Number() != 1.0 ||
		nullptr == areas || !areas->Is_Array())
	{
		m_Status = "Map editor catalog header is invalid";
		return false;
	}

	const std::array<std::pair<const char_t*, const char_t*>, 6> targets =
	{{
		{ "LV_LOBBY_CLASSSELECT_SL00", "Character Select" },
		{ "LV_BER_BERNCASTLE", "Bern" },
		{ "LV_LUT_HEARTRB_ED", "Valtan" },
		{ "LV_LUT_MIDNIGHTC_ED", "KoukuSaydon / MidnightC ED" },
		{ "LV_SHS_RCARENA_D", "Training Map" },
		{ "LV_OCN_EVENTIS_MHP", "Maharaka Paradise" },
	}};
	std::vector<EDITOR_AREA_DESCRIPTOR> staged;
	staged.reserve(targets.size());
	for (const auto& target : targets)
	{
		const DATA_JSON_VALUE* selected = nullptr;
		for (const DATA_JSON_VALUE& candidate : areas->Get_Array())
		{
			const DATA_JSON_VALUE* id = candidate.Find("id");
			if (candidate.Is_Object() && nullptr != id && id->Is_String() &&
				id->Get_String() == target.first)
			{
				if (nullptr != selected)
				{
					m_Status = "Duplicate MapCatalog area: " +
						std::string(target.first);
					return false;
				}
				selected = &candidate;
			}
		}
		if (nullptr == selected)
		{
			m_Status = "MapCatalog area is missing: " +
				std::string(target.first);
			return false;
		}

		EDITOR_AREA_DESCRIPTOR descriptor;
		descriptor.areaId = target.first;
		descriptor.label = target.second;
		std::string sourceCatalog;
		std::string sourcePlacements;
		if (!ReadRequiredString(*selected, "sourceCatalog", sourceCatalog) ||
			!ReadRequiredString(*selected, "sourcePlacements", sourcePlacements))
		{
			m_Status = "MapCatalog authoring source is missing: " +
				descriptor.areaId;
			return false;
		}
		descriptor.sourceCatalog = ResolveDataCatalogPath(sourceCatalog);
		descriptor.sourcePlacements =
			ResolveDataCatalogPath(sourcePlacements);
		const DATA_JSON_VALUE* sourceMaterials = selected->Find("sourceMaterials");
		const DATA_JSON_VALUE* runtimeMaterials = selected->Find("materials");
		if ((nullptr == sourceMaterials) != (nullptr == runtimeMaterials))
		{
			m_Status = "MapCatalog material source/runtime pair is incomplete: " +
				descriptor.areaId;
			return false;
		}
		if (nullptr != sourceMaterials)
		{
			const std::string expectedSource = "Data/Maps/Authoring/" +
				descriptor.areaId + "/" + descriptor.areaId + ".mapmaterials.json";
			const std::string expectedRuntime = "Client/Bin/DataFiles/Map/" +
				descriptor.areaId + ".mapmaterials.json";
			if (!sourceMaterials->Is_String() || !runtimeMaterials->Is_String() ||
				sourceMaterials->Get_String() != expectedSource ||
				runtimeMaterials->Get_String() != expectedRuntime)
			{
				m_Status = "MapCatalog material paths are not canonical: " +
					descriptor.areaId;
				return false;
			}
			descriptor.sourceMaterials = ResolveDataCatalogPath(expectedSource);
			if (descriptor.sourceMaterials.empty())
			{
				m_Status = "MapCatalog material path escapes Data root: " +
					descriptor.areaId;
				return false;
			}
		}
		const DATA_JSON_VALUE* sourceLights = selected->Find("sourceLights");
		if (nullptr != sourceLights)
		{
			if (!sourceLights->Is_String() ||
				sourceLights->Get_String().empty())
			{
				m_Status = "MapCatalog light authoring source is invalid: " +
					descriptor.areaId;
				return false;
			}
			descriptor.sourceLights = ResolveDataCatalogPath(
				sourceLights->Get_String());
			if (descriptor.sourceLights.empty())
			{
				m_Status = "MapCatalog light path escapes Data root: " +
					descriptor.areaId;
				return false;
			}
		}
		const DATA_JSON_VALUE* sourceDeployCatalog =
			selected->Find("sourceDeployCatalog");
		const DATA_JSON_VALUE* sourceDeployPlacements =
			selected->Find("sourceDeployPlacements");
		if ((nullptr == sourceDeployCatalog) !=
			(nullptr == sourceDeployPlacements) ||
			(nullptr != sourceDeployCatalog &&
				(!sourceDeployCatalog->Is_String() ||
					sourceDeployCatalog->Get_String().empty() ||
					!sourceDeployPlacements->Is_String() ||
					sourceDeployPlacements->Get_String().empty())))
		{
			m_Status = "MapCatalog deploy authoring pair is invalid: " +
				descriptor.areaId;
			return false;
		}
		if (nullptr != sourceDeployCatalog)
		{
			descriptor.sourceDeployCatalog = ResolveDataCatalogPath(
				sourceDeployCatalog->Get_String());
			descriptor.sourceDeployPlacements = ResolveDataCatalogPath(
				sourceDeployPlacements->Get_String());
			if (descriptor.sourceDeployCatalog.empty() ||
				descriptor.sourceDeployPlacements.empty())
			{
				m_Status = "MapCatalog deploy path escapes Data root: " +
					descriptor.areaId;
				return false;
			}
		}

		/* Camera shots are an optional Client presentation layer. The document
		   is named per Area and is simply absent where none is authored. */
		descriptor.cameraShotDocument = CProjectDataRoot::Resolve(
			std::filesystem::path("Maps") / "Authoring" / descriptor.areaId /
			(descriptor.areaId + ".camerashots.json"));

		if (descriptor.areaId == "LV_LOBBY_CLASSSELECT_SL00" ||
			descriptor.areaId == "LV_BER_BERNCASTLE" ||
			descriptor.areaId == "LV_LUT_HEARTRB_ED" ||
			descriptor.areaId == "LV_LUT_MIDNIGHTC_ED")
		{
			std::string source;
			std::string paint;
			if (!ReadRequiredString(*selected, "navigationSource", source) ||
				!ReadRequiredString(*selected, "navigationPaint", paint))
			{
				m_Status = "MapCatalog navigation source is missing: " +
					descriptor.areaId;
				return false;
			}
			descriptor.navigationSource = ResolveDataCatalogPath(source);
			descriptor.navigationPaint = ResolveDataCatalogPath(paint);
			descriptor.navigationPolicy =
				EDITOR_NAVIGATION_POLICY::SOURCE_PAINT;
			descriptor.allowNavigationBootstrap =
				descriptor.areaId == "LV_BER_BERNCASTLE";
		}
		if (descriptor.areaId == "LV_LUT_HEARTRB_ED")
		{
			if (descriptor.sourceLights.empty())
			{
				m_Status = "Valtan source light presentation is missing";
				return false;
			}
			std::string blockers;
			if (!ReadRequiredString(*selected, "navigationBlockers", blockers))
			{
				m_Status = "Valtan navigation blocker source is missing";
				return false;
			}
			descriptor.navigationBlockers = ResolveDataCatalogPath(blockers);
			descriptor.navigationPolicy =
				EDITOR_NAVIGATION_POLICY::SOURCE_PAINT_BLOCKERS;
			/* The editor area registry already declares the explicit areas
			   the Map Tool opens, so the destruction reference path is
			   declared the same way instead of adding a field to the shared
			   MapCatalog schema. Render_DestructionEncounterSource cross
			   checks the loaded encounterId against the boss placement of
			   this Area. */
			descriptor.encounterReference = CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanEncounter.json");
			descriptor.worldEventsDocument = CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanWorldEvents.json");
			descriptor.destructionSimulationDocument = CProjectDataRoot::Resolve(
				L"Maps/Authoring/LV_LUT_HEARTRB_ED/"
				L"LV_LUT_HEARTRB_ED.destructionsimulation.json");
			if (descriptor.encounterReference.empty() ||
				descriptor.worldEventsDocument.empty() ||
				descriptor.destructionSimulationDocument.empty())
			{
				m_Status = "Valtan encounter reference path is invalid";
				return false;
			}
		}

		if (descriptor.areaId == "LV_LOBBY_CLASSSELECT_SL00" ||
			descriptor.areaId == "LV_BER_BERNCASTLE" ||
			descriptor.areaId == "LV_LUT_HEARTRB_ED" ||
			descriptor.areaId == "LV_LUT_MIDNIGHTC_ED" ||
			descriptor.areaId == "LV_OCN_EVENTIS_MHP")
		{
			std::string gameplay;
			if (!ReadRequiredString(*selected, "gameplayDocument", gameplay))
			{
				m_Status = "MapCatalog gameplay document is missing: " +
					descriptor.areaId;
				return false;
			}
			descriptor.gameplayDocument = ResolveDataCatalogPath(gameplay);
			descriptor.gameplayPolicy =
				EDITOR_GAMEPLAY_POLICY::REQUIRED;
		}

		if (descriptor.sourceCatalog.empty() ||
			descriptor.sourcePlacements.empty() ||
			((!descriptor.sourceDeployCatalog.empty() ||
				!descriptor.sourceDeployPlacements.empty()) &&
				(descriptor.sourceDeployCatalog.empty() ||
					descriptor.sourceDeployPlacements.empty())) ||
			(EDITOR_NAVIGATION_POLICY::NONE != descriptor.navigationPolicy &&
				(descriptor.navigationSource.empty() ||
					descriptor.navigationPaint.empty())) ||
			(EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy &&
				descriptor.gameplayDocument.empty()))
		{
			m_Status = "MapCatalog path escapes Data root: " +
				descriptor.areaId;
			return false;
		}
		staged.push_back(std::move(descriptor));
	}

	m_EditorAreas = std::move(staged);
	return true;
}

const Client::CMapTool::EDITOR_AREA_DESCRIPTOR*
Client::CMapTool::Get_ActiveEditorArea() const
{
	return m_iActiveEditorArea < m_EditorAreas.size() ?
		&m_EditorAreas[m_iActiveEditorArea] : nullptr;
}

CWorldSequencePlayer::TARGET_SET Client::CMapTool::Runtime_AuthoringTargets() const
{
#ifdef _DEBUG
	const uint32_t levelIndex = CGameInstance::Get().Get_CurrentLevelID();
	if (levelIndex == ETOUI(LEVEL::MAHARAKA))
	{
		if (auto* level = CLevel_Development::Get_Active(LEVEL::MAHARAKA))
		{
			CWorldSequencePlayer::TARGET_SET targets;
			targets.levelIndex = levelIndex;
			targets.pCatalog = &level->Get_MapAuthoringRuntime().Get_Catalog();
			targets.pPlacements = &level->Get_MapAuthoringRuntime().Get_MutablePlacements();
			targets.pDeployRuntime = &level->Get_MapAuthoringDeploy();
			targets.device = level->Get_MapAuthoringDevice();
			targets.context = level->Get_MapAuthoringContext();
			return targets;
		}
	}
	if (levelIndex == ETOUI(LEVEL::KAKULSAYDON_ARENA))
		if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
			return arena->Get_CompositionWorldTargets();
	/* Valtan lends the same map runtime. It has no World Sequence owner, so the
	   Object preparation and anchor hooks stay empty here. */
	if (levelIndex == ETOUI(LEVEL::VALTAN_ARENA))
	{
		if (auto* arena = CLevel_ValtanArena::Get_Active())
		{
			CWorldSequencePlayer::TARGET_SET targets;
			targets.levelIndex = levelIndex;
			targets.pCatalog = &arena->Get_MapAuthoringRuntime().Get_Catalog();
			targets.pPlacements = &arena->Get_MapAuthoringRuntime().Get_MutablePlacements();
			targets.pDeployRuntime = &arena->Get_MapAuthoringDeploy();
			targets.device = arena->Get_MapAuthoringDevice();
			targets.context = arena->Get_MapAuthoringContext();
			return targets;
		}
	}
	/* Bern and Character Select lend the same live map runtime. Neither Area
	   declares a DeployProp source pair, so the level-owned deploy runtime they
	   hand over stays empty; it exists because the attach contract needs a real
	   owner. Neither level drives map self motions or a World Sequence, so the
	   Object preparation and anchor hooks stay empty here too. */
	if (levelIndex == ETOUI(LEVEL::BERN))
	{
		if (auto* level = CLevel_Bern::Get_Active())
		{
			CWorldSequencePlayer::TARGET_SET targets;
			targets.levelIndex = levelIndex;
			targets.pCatalog = &level->Get_MapAuthoringRuntime().Get_Catalog();
			targets.pPlacements = &level->Get_MapAuthoringRuntime().Get_MutablePlacements();
			targets.pDeployRuntime = &level->Get_MapAuthoringDeploy();
			targets.device = level->Get_MapAuthoringDevice();
			targets.context = level->Get_MapAuthoringContext();
			return targets;
		}
	}
	if (levelIndex == ETOUI(LEVEL::CHARACTER_SELECT))
	{
		if (auto* level = CLevel_CharacterSelect::Get_Active())
		{
			CWorldSequencePlayer::TARGET_SET targets;
			targets.levelIndex = levelIndex;
			targets.pCatalog = &level->Get_MapAuthoringRuntime().Get_Catalog();
			targets.pPlacements = &level->Get_MapAuthoringRuntime().Get_MutablePlacements();
			targets.pDeployRuntime = &level->Get_MapAuthoringDeploy();
			targets.device = level->Get_MapAuthoringDevice();
			targets.context = level->Get_MapAuthoringContext();
			return targets;
		}
	}
#endif
	return {};
}

bool_t Client::CMapTool::Is_MapAuthoringLevel() const
{
	return (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::DEVELOPMENT) &&
		CMapEditorWorkspaceService::Is_Active()) ||
		((m_bOpen || m_bRuntimeAuthoring) && Runtime_AuthoringTargets().pPlacements != nullptr);
}

vector<Client::CMapTool::PLACED_ENTRY>& Client::CMapTool::Authoring_Placements()
{
	const auto targets = m_bRuntimeAuthoring ? Runtime_AuthoringTargets() : CWorldSequencePlayer::TARGET_SET{};
	return targets.pPlacements ? *targets.pPlacements : m_Placements;
}
const vector<Client::CMapTool::PLACED_ENTRY>& Client::CMapTool::Authoring_Placements() const
{ return const_cast<CMapTool*>(this)->Authoring_Placements(); }

vector<Client::CMapTool::STATIC_BATCH_ENTRY>& Client::CMapTool::Authoring_Batches()
{
#ifdef _DEBUG
	if (m_bRuntimeAuthoring && Runtime_AuthoringTargets().pPlacements)
	{
		if (auto* maharaka = CLevel_Development::Get_Active(LEVEL::MAHARAKA))
			if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::MAHARAKA))
				return maharaka->Get_MapAuthoringRuntime().Get_AuthoringBatches();
		if (auto* kouku = CLevel_KakulSaydonArena::Get_Active())
			if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA))
				return kouku->Get_MapAuthoringBatches();
		if (auto* valtan = CLevel_ValtanArena::Get_Active())
			if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
				return valtan->Get_MapAuthoringRuntime().Get_AuthoringBatches();
		if (auto* bern = CLevel_Bern::Get_Active())
			if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::BERN))
				return bern->Get_MapAuthoringRuntime().Get_AuthoringBatches();
		if (auto* select = CLevel_CharacterSelect::Get_Active())
			if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
				return select->Get_MapAuthoringRuntime().Get_AuthoringBatches();
	}
#endif
	return m_StaticBatches;
}
const vector<Client::CMapTool::STATIC_BATCH_ENTRY>& Client::CMapTool::Authoring_Batches() const
{ return const_cast<CMapTool*>(this)->Authoring_Batches(); }

CDeployPropRuntime& Client::CMapTool::Authoring_Deploy()
{
	const auto targets = m_bRuntimeAuthoring ? Runtime_AuthoringTargets() : CWorldSequencePlayer::TARGET_SET{};
	return targets.pDeployRuntime ? *targets.pDeployRuntime : m_DeployRuntime;
}
const CDeployPropRuntime& Client::CMapTool::Authoring_Deploy() const
{ return const_cast<CMapTool*>(this)->Authoring_Deploy(); }

bool_t Client::CMapTool::Can_ChangeRuntimeStructure()
{
	if (m_bRuntimeAuthoring && m_pWorldSequenceToolPanel && m_pWorldSequenceToolPanel->Is_PreviewActive())
	{
		m_Status = "Stop / Restore the Map Tool sequence before changing object membership.";
		return false;
	}
#ifdef _DEBUG
	if (m_bRuntimeAuthoring && !Can_ReplaceRuntimeAuthoringTargets())
	{
		m_Status = "Stop active arena/Object/Composition playback before adding, deleting or reloading map objects.";
		return false;
	}
#endif
	return true;
}

bool_t Client::CMapTool::Can_ReplaceRuntimeAuthoringTargets() const
{
#ifdef _DEBUG
	if (!Runtime_AuthoringTargets().pPlacements)
		return false;
	if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA))
	{
		auto* arena = CLevel_KakulSaydonArena::Get_Active();
		return nullptr != arena && arena->Can_ReplaceMapAuthoringTargets();
	}
	/* Valtan, Bern, Character Select and Maharaka have no level-owned World Sequence or
	   Composition preview to stop before the tool replaces placements. */
	const uint32_t levelIndex = CGameInstance::Get().Get_CurrentLevelID();
	return levelIndex == ETOUI(LEVEL::VALTAN_ARENA) ||
		levelIndex == ETOUI(LEVEL::MAHARAKA) ||
		levelIndex == ETOUI(LEVEL::BERN) ||
		levelIndex == ETOUI(LEVEL::CHARACTER_SELECT);
#else
	return false;
#endif
}

void Client::CMapTool::Apply_RuntimeAuthoringActive(const bool_t active)
{
#ifdef _DEBUG
	if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::MAHARAKA))
	{
		if (auto* level = CLevel_Development::Get_Active(LEVEL::MAHARAKA))
			level->Set_MapAuthoringActive(active);
		return;
	}
	/* Both Maharaka and Kouku stop self motions while the tool owns poses. */
	if (CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::KAKULSAYDON_ARENA))
		return;
	if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
		arena->Set_MapAuthoringActive(active);
#else
	(void)active;
#endif
}

void Client::CMapTool::Remember_RuntimePlacement(const MAP_PLACEMENT_RECORD& record)
{
	if (!m_bRuntimeAuthoring) return;
	const auto found = m_RuntimePlacementIndex.find(record.placementId);
	if (found == m_RuntimePlacementIndex.end())
	{
		m_RuntimePlacementIndex.emplace(record.placementId, m_RuntimePlacementDraft.size());
		m_RuntimePlacementDraft.push_back(record);
	}
	else m_RuntimePlacementDraft[found->second] = record;
	Rebase_RuntimeMotions();
}

void Client::CMapTool::Reset_RuntimePlacementDraft(const vector<MAP_PLACEMENT_RECORD>& records)
{
	m_RuntimePlacementDraft = records;
	m_RuntimePlacementIndex.clear();
	for (size_t index = 0; index < m_RuntimePlacementDraft.size(); ++index)
		m_RuntimePlacementIndex.emplace(m_RuntimePlacementDraft[index].placementId, index);
	Rebase_RuntimeMotions();
}

void Client::CMapTool::Forget_RuntimePlacement(uint64_t placementId)
{
	if (!m_bRuntimeAuthoring) return;
	std::erase_if(m_RuntimePlacementDraft, [placementId](const auto& record) { return record.placementId == placementId; });
	// Reset accepts a reference to the same vector; self-assignment preserves it.
	Reset_RuntimePlacementDraft(m_RuntimePlacementDraft);
}

void Client::CMapTool::Rebase_RuntimeMotions()
{
#ifdef _DEBUG
	if (m_bRuntimeAuthoring && Runtime_AuthoringTargets().pPlacements &&
		CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::MAHARAKA))
		if (auto* level = CLevel_Development::Get_Active(LEVEL::MAHARAKA))
			level->Rebase_MapAuthoringSelfMotions(m_RuntimePlacementDraft);
	if (m_bRuntimeAuthoring && Runtime_AuthoringTargets().pPlacements &&
		CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA))
		if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
			arena->Rebase_MapAuthoringSelfMotions(m_RuntimePlacementDraft);
#endif
}

const MAP_PLACEMENT_RECORD& Client::CMapTool::Authored_Placement(const PLACED_ENTRY& entry) const
{
	if (m_bRuntimeAuthoring)
	{
		const auto found = m_RuntimePlacementIndex.find(entry.record.placementId);
		if (found != m_RuntimePlacementIndex.end()) return m_RuntimePlacementDraft[found->second];
	}
	return entry.record;
}

bool_t Client::CMapTool::Begin_EditorAreaSwitch(const size_t descriptorIndex)
{
	if (descriptorIndex >= m_EditorAreas.size() ||
		m_iAuthoringLevelIndex != ETOUI(LEVEL::DEVELOPMENT) ||
		!CMapEditorWorkspaceService::Is_Active() ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status = "Map editor Area switch is unavailable";
		return false;
	}
	/* The preview drives this Area's actors; stop it while its targets are
	   still the ones it was staged against. */
	Stop_EditorCutscene();
	if (nullptr != m_pWorldSequenceToolPanel && m_Catalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, m_Catalog, Authoring_Placements(),
			Authoring_Deploy());
		if (m_pWorldSequenceToolPanel->Is_PreviewActive())
		{
			m_Status = m_pWorldSequenceToolPanel->Get_Status();
			return false;
		}
	}

	const EDITOR_AREA_DESCRIPTOR& descriptor =
		m_EditorAreas[descriptorIndex];
	const std::filesystem::path worldSequencePath =
		descriptor.sourcePlacements.parent_path() /
		std::filesystem::path(descriptor.areaId + ".worldsequences.json");
	std::string recoveryStatus;
	SCOPED_AUTHORING_SAVE_LOCK authoringLock;
	if (!authoringLock.Acquire(worldSequencePath, recoveryStatus) ||
		!RecoverAuthoringTransactionUnderLock(
		descriptor.sourcePlacements, worldSequencePath, recoveryStatus))
	{
		m_Status = recoveryStatus;
		return false;
	}
	std::error_code error;
	if (!std::filesystem::is_regular_file(descriptor.sourceCatalog, error) ||
		error ||
		!std::filesystem::is_regular_file(descriptor.sourcePlacements, error) ||
		error)
	{
		m_Status = "Required authoring map source is missing: " +
			descriptor.areaId;
		return false;
	}

	/* Only the model catalog is read here so the admission can start. Every
	   other authoring document still loads inside the one atomic
	   Switch_EditorArea transaction that runs once the prototypes exist. */
	CMapAssetCatalog stagedCatalog;
	if (!stagedCatalog.Load_Source(
		descriptor.sourceCatalog,
		descriptor.sourcePlacements,
		descriptor.areaId,
		descriptor.sourceMaterials))
	{
		m_Status = stagedCatalog.Get_Status();
		return false;
	}

	m_EditorAreaPreload.iDescriptorIndex = descriptorIndex;
	m_EditorAreaPreload.Catalog = std::move(stagedCatalog);
	m_EditorAreaPreload.iNextEntry = 0;
	Report_EditorAreaPreloadProgress();
	return true;
}

void Client::CMapTool::Report_EditorAreaPreloadProgress()
{
	if (!m_EditorAreaPreload.Is_Active())
		return;

	m_Status = "Preparing " +
		m_EditorAreas[m_EditorAreaPreload.iDescriptorIndex].label + ": " +
		std::to_string(m_EditorAreaPreload.iNextEntry) + " / " +
		std::to_string(m_EditorAreaPreload.Catalog.Get_Entries().size()) +
		" model prototypes";
}

void Client::CMapTool::Update_EditorAreaPreload()
{
	if (!m_EditorAreaPreload.Is_Active())
		return;

	if (m_iAuthoringLevelIndex != ETOUI(LEVEL::DEVELOPMENT) ||
		!CMapEditorWorkspaceService::Is_Active() ||
		m_EditorAreaPreload.iDescriptorIndex >= m_EditorAreas.size())
	{
		m_EditorAreaPreload = {};
		m_Status = "Map editor Area switch was cancelled";
		return;
	}

	const std::vector<MAP_ASSET_ENTRY>& entries =
		m_EditorAreaPreload.Catalog.Get_Entries();
	const auto admissionStart = std::chrono::steady_clock::now();
	while (m_EditorAreaPreload.iNextEntry < entries.size())
	{
		if (!Admit_AuthoringPrototype(
			entries[m_EditorAreaPreload.iNextEntry]))
		{
			/* The failing asset already named itself in the status and no
			   Area state was replaced, so the editor keeps the loaded Area. */
			m_EditorAreaPreload = {};
			return;
		}
		++m_EditorAreaPreload.iNextEntry;
		if (std::chrono::steady_clock::now() - admissionStart >=
			EDITOR_AREA_ADMISSION_FRAME_BUDGET)
		{
			break;
		}
	}

	if (m_EditorAreaPreload.iNextEntry < entries.size())
	{
		Report_EditorAreaPreloadProgress();
		return;
	}

	const size_t iCommitIndex = m_EditorAreaPreload.iDescriptorIndex;
	m_EditorAreaPreload = {};
	Switch_EditorArea(iCommitIndex);
}

bool_t Client::CMapTool::Switch_EditorArea(const size_t descriptorIndex)
{
	const auto runtimeTargets = Runtime_AuthoringTargets();
	const bool_t runtimeAttach = runtimeTargets.pPlacements && runtimeTargets.pDeployRuntime && runtimeTargets.pCatalog;
	if (descriptorIndex >= m_EditorAreas.size() || !Is_MapAuthoringLevel() ||
		(runtimeAttach && m_EditorAreas[descriptorIndex].areaId != runtimeTargets.pCatalog->Get_AreaId()))
	{
		m_Status = "Map editor Area switch is unavailable";
		return false;
	}

	const EDITOR_AREA_DESCRIPTOR& descriptor =
		m_EditorAreas[descriptorIndex];
	/* Whatever this switch replaces, a running cutscene preview must not keep
	   driving the previous catalog's actors. */
	Stop_EditorCutscene();
	const std::filesystem::path worldSequencePath =
		descriptor.sourcePlacements.parent_path() /
		std::filesystem::path(descriptor.areaId + ".worldsequences.json");
	std::string recoveryStatus;
	SCOPED_AUTHORING_SAVE_LOCK authoringLock;
	if (!authoringLock.Acquire(worldSequencePath, recoveryStatus) ||
		!RecoverAuthoringTransactionUnderLock(
		descriptor.sourcePlacements, worldSequencePath, recoveryStatus))
	{
		m_Status = recoveryStatus;
		return false;
	}
	std::error_code error;
	if (!std::filesystem::is_regular_file(descriptor.sourceCatalog, error) ||
		error ||
		!std::filesystem::is_regular_file(descriptor.sourcePlacements, error) ||
		error)
	{
		m_Status = "Required authoring map source is missing: " +
			descriptor.areaId;
		return false;
	}

	CMapAssetCatalog stagedCatalog;
	if (!stagedCatalog.Load_Source(
		descriptor.sourceCatalog,
		descriptor.sourcePlacements,
		descriptor.areaId,
		descriptor.sourceMaterials))
	{
		m_Status = stagedCatalog.Get_Status();
		return false;
	}
	if (runtimeAttach && !stagedCatalog.Bind_RuntimePrototypes(*runtimeTargets.pCatalog))
	{
		m_Status = stagedCatalog.Get_Status();
		return false;
	}

	std::vector<MAP_PLACEMENT_RECORD> records;
	std::string stagedStatus;
	if (!CMapPlacementDocument::Read(
		descriptor.sourcePlacements,
		stagedCatalog,
		records,
		stagedStatus))
	{
		m_Status = stagedStatus;
		return false;
	}
	if (runtimeAttach)
	{
		// Refuse a partial/stale runtime: Save must never delete an unloaded source row.
		std::unordered_map<uint64_t, std::string> liveIds;
		for (const auto& entry : *runtimeTargets.pPlacements)
			liveIds.emplace(entry.record.placementId, entry.record.assetId);
		if (liveIds.size() != records.size() || std::any_of(records.begin(), records.end(),
			[&liveIds](const auto& record) {
				const auto found = liveIds.find(record.placementId);
				return found == liveIds.end() || found->second != record.assetId;
			}))
		{
			m_Status = "Runtime/source placement IDs differ. Save in Test, publish and re-enter this level before runtime editing.";
			return false;
		}
	}
	/* The sequence document reference-validates Deploy animation tracks, so
	   it is loaded once this Area's Deploy runtime has been staged below. */
	auto stagedWorldSequencePanel =
		std::make_unique<CWorldSequenceToolPanel>();

	CWorldGameplayDocument stagedWorld;
	CSpawnGroupDocument stagedSpawnGroups;
	if (EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy)
	{
		error.clear();
		if (!std::filesystem::is_regular_file(
			descriptor.gameplayDocument, error) || error ||
			!stagedWorld.Load(
				descriptor.gameplayDocument,
				descriptor.areaId,
				stagedStatus))
		{
			m_Status = error ?
				"Could not inspect required gameplay document" : stagedStatus;
			return false;
		}
		const std::filesystem::path spawnGroupsPath =
			descriptor.gameplayDocument.parent_path() / L"SpawnGroups.world.json";
		if (!stagedSpawnGroups.Load(
			spawnGroupsPath, descriptor.areaId, stagedStatus))
		{
			m_Status = stagedStatus;
			return false;
		}
	}

	CNavGridPaintDocument stagedNavigation;
	CNavRuntimeBlockerDocument stagedBlockers;
	bool_t navigationLoaded = false;
	if (EDITOR_NAVIGATION_POLICY::NONE != descriptor.navigationPolicy)
	{
		error.clear();
		const bool_t hasSource = std::filesystem::is_regular_file(
			descriptor.navigationSource, error);
		if (IsFileInspectionFailure(error) ||
			(!hasSource && !descriptor.allowNavigationBootstrap))
		{
			m_Status = "Required navigation source is missing: " +
				descriptor.areaId;
			return false;
		}
		if (hasSource)
		{
			if (!stagedNavigation.Load(
				descriptor.navigationSource,
				descriptor.navigationPaint,
				stagedStatus) ||
				stagedNavigation.Get_Desc().areaId != descriptor.areaId ||
				!stagedBlockers.Load(
					descriptor.navigationBlockers,
					stagedNavigation.Get_Desc(),
					stagedStatus))
			{
				m_Status = stagedStatus;
				return false;
			}
			navigationLoaded = true;
		}
	}

	/* Pure data document: an Area without a world events path simply has no
	   destruction authoring, a missing file for Valtan starts empty so the
	   first Save can create it, and a corrupt file fails the switch like the
	   other authoring documents do. */
	CWorldDestructionDocument stagedDestruction;
	CEncounterPatternReference stagedEncounterReference;
	std::string stagedEncounterStatus;
	if (!descriptor.worldEventsDocument.empty())
	{
		if (descriptor.encounterReference.empty() ||
			!stagedEncounterReference.Load(
				descriptor.encounterReference, stagedEncounterStatus))
		{
			m_Status = descriptor.encounterReference.empty() ?
				"World destruction requires an encounter reference" :
				stagedEncounterStatus;
			return false;
		}
		std::error_code destructionError;
		const bool_t hasDocument = std::filesystem::is_regular_file(
			descriptor.worldEventsDocument, destructionError);
		if (IsFileInspectionFailure(destructionError))
		{
			m_Status = "World destruction document is unreadable: " +
				descriptor.areaId;
			return false;
		}
		std::string destructionStatus;
		if (!hasDocument)
		{
			stagedDestruction.Reset_Empty();
		}
		else if (!stagedDestruction.Load(
			descriptor.worldEventsDocument,
			descriptor.areaId,
			"ENCOUNTER_VALTAN",
			destructionStatus))
		{
			m_Status = destructionStatus;
			return false;
		}
	}

	CDestructionSimulationDocument stagedSimulation;
	if (!descriptor.destructionSimulationDocument.empty())
	{
		std::error_code simulationError;
		const bool_t hasSimulation = std::filesystem::is_regular_file(
			descriptor.destructionSimulationDocument, simulationError);
		if (IsFileInspectionFailure(simulationError))
		{
			m_Status = "Destruction simulation document is unreadable: " +
				descriptor.areaId;
			return false;
		}
		std::string simulationStatus;
		if (!hasSimulation)
		{
			stagedSimulation.Reset_Empty();
		}
		else if (!stagedSimulation.Load(
			descriptor.destructionSimulationDocument,
			descriptor.areaId,
			simulationStatus))
		{
			m_Status = simulationStatus;
			return false;
		}
	}
	if (stagedSimulation.Is_Ready() && stagedDestruction.Is_Ready() &&
		!stagedSimulation.Validate_GroupReferences(
			stagedDestruction, stagedStatus))
	{
		m_Status = stagedStatus;
		return false;
	}
	shared_ptr<CMapLightPresentationRuntime> stagedMapLightPresentation;
	if (!descriptor.sourceLights.empty())
	{
		stagedMapLightPresentation =
			make_shared<CMapLightPresentationRuntime>();
		if (!stagedMapLightPresentation->Load(
			descriptor.sourceLights, descriptor.areaId))
		{
			m_Status = stagedMapLightPresentation->Get_Status();
			return false;
		}
	}

	/* Read-only here: a rejected shot document must never block the Area, it
	   only leaves the list empty with a reported reason. */
	(void)Load_CameraShots(descriptor);

	if (!runtimeAttach && !Ensure_AuthoringPrototypes(stagedCatalog))
		return false;
	const bool_t stagedDebrisPrototypesReady =
		!descriptor.destructionSimulationDocument.empty() &&
		Ensure_DestructionDebrisAuthoringPrototypes(runtimeAttach);
	const std::string stagedDebrisPrototypeStatus =
		descriptor.destructionSimulationDocument.empty() ?
		"PROJECT_AUTHORED debris is not declared for this Area" : m_Status;

	CMapAssetCatalog previousCatalog = m_Catalog;
	m_Catalog = stagedCatalog;
	std::vector<PLACED_ENTRY> stagedPlacements;
	std::vector<STATIC_BATCH_ENTRY> stagedBatches;
	if (!runtimeAttach && !Stage_PlacementRuntime(records, stagedPlacements, stagedBatches))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		m_Catalog = std::move(previousCatalog);
		m_Status = "Map Area runtime stage rolled back: " + descriptor.areaId;
		return false;
	}
	CDeployPropRuntime stagedDeployRuntime;
	if (!runtimeAttach && !Stage_DeployProps(descriptor, stagedDeployRuntime))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		m_Catalog = std::move(previousCatalog);
		if (m_Status.empty())
			m_Status = "Deploy prop staging failed: " + descriptor.areaId;
		return false;
	}
	if (!stagedWorldSequencePanel->Load_Area(
		worldSequencePath, descriptor.sourcePlacements, descriptor.areaId,
		stagedCatalog, records, runtimeAttach ? *runtimeTargets.pDeployRuntime : stagedDeployRuntime,
		stagedStatus))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = stagedStatus;
		return false;
	}
	std::vector<MAP_PLACEMENT_RECORD> stableLinkedRecords;
	if (!CMapPlacementDocument::Read(descriptor.sourcePlacements,
		stagedCatalog, stableLinkedRecords, stagedStatus) ||
		!AreExactlySamePlacementRecords(records, stableLinkedRecords) ||
		!stagedWorldSequencePanel->Matches_LinkedSourceBaseline(stagedStatus))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = "Linked map/sequence source changed while the Area was loading: " +
			stagedStatus;
		return false;
	}
	if (stagedDestruction.Is_Ready() &&
		!Validate_DestructionExternalReferences(
			stagedDestruction,
			runtimeAttach ? *runtimeTargets.pDeployRuntime : stagedDeployRuntime,
			stagedBlockers,
			stagedWorld,
			stagedEncounterReference,
			stagedStatus))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = stagedStatus;
		return false;
	}
	vector<TRIGGER_BOX_ENTRY> stagedTriggerBoxes;
	if (!Stage_WorldTriggerBoxes(stagedWorld, stagedTriggerBoxes))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = "World trigger box preview staging failed: " + descriptor.areaId +
			" (" + m_WorldGameplayStatus + ")";
		return false;
	}
	vector<NPC_PREVIEW_ENTRY> stagedNpcPreviews;
	if (!runtimeAttach && !Stage_WorldNpcPreviews(
		stagedWorld, stagedNpcPreviews, &stagedNavigation, &stagedBlockers))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		Remove_WorldTriggerBoxes(stagedTriggerBoxes);
		m_Catalog = std::move(previousCatalog);
		m_Status = "World NPC preview staging failed: " + descriptor.areaId;
		return false;
	}
	vector<TRIGGER_BOX_ENTRY> stagedSpawnAnchorBoxes;
	if (!Stage_SpawnAnchorBoxes(stagedSpawnGroups, stagedSpawnAnchorBoxes))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		Remove_WorldTriggerBoxes(stagedTriggerBoxes);
		Remove_WorldNpcPreviews(stagedNpcPreviews);
		m_Catalog = std::move(previousCatalog);
		m_Status = "Spawn anchor box staging failed: " + descriptor.areaId +
			" (" + m_WorldGameplayStatus + ")";
		return false;
	}

	/* The staged runtime owns preview seams into the current Deploy runtime.
	   Release them before the Area transaction move-assigns that owner. The
	   previous catalog must be used because m_Catalog currently stages the
	   destination Area. */
	if (nullptr != m_pWorldSequenceToolPanel && previousCatalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, previousCatalog, Authoring_Placements(),
			Authoring_Deploy());
		if (m_pWorldSequenceToolPanel->Is_PreviewActive())
		{
			const std::string restoreFailure =
				m_pWorldSequenceToolPanel->Get_Status();
			Remove_PlacementRuntime(stagedPlacements, stagedBatches);
			stagedDeployRuntime.Clear();
			Remove_WorldTriggerBoxes(stagedTriggerBoxes);
			Remove_WorldNpcPreviews(stagedNpcPreviews);
			Remove_WorldTriggerBoxes(stagedSpawnAnchorBoxes);
			m_Catalog = std::move(previousCatalog);
			m_Status = restoreFailure;
			return false;
		}
	}
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	m_bDestructionSimulationClearRequested = false;
	if (!runtimeAttach) Remove_PlacementRuntime(Authoring_Placements(), Authoring_Batches());
	Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
	Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
	Remove_WorldNpcPreviews(m_WorldNpcPreviews);
	if (!runtimeAttach)
	{
		Authoring_Placements() = std::move(stagedPlacements);
		Authoring_Batches() = std::move(stagedBatches);
		Authoring_Deploy() = std::move(stagedDeployRuntime);
	}
	m_bRuntimeAuthoring = runtimeAttach;
	Reset_RuntimePlacementDraft(runtimeAttach ? records : vector<MAP_PLACEMENT_RECORD>{});
	if (runtimeAttach)
	{
		// These identities belong to the active arena loader, not a second prototype owner.
		for (const auto& asset : runtimeTargets.pCatalog->Get_Entries())
			m_PrototypeModelPaths.emplace(asset.prototypeTag, asset.resolvedModelPath.lexically_normal());
		for (const auto& asset : runtimeTargets.pDeployRuntime->Get_Catalog().Get_Assets())
		{
			m_PrototypeModelPaths.emplace(asset.intactPrototypeTag, asset.intactResolvedPath.lexically_normal());
			if (!asset.fracturedPrototypeTag.empty())
				m_PrototypeModelPaths.emplace(asset.fracturedPrototypeTag, asset.fracturedResolvedPath.lexically_normal());
		}
	}
	m_pWorldSequenceToolPanel = std::move(stagedWorldSequencePanel);
	m_WorldGameplayDocument = std::move(stagedWorld);
	m_WorldTriggerBoxes = std::move(stagedTriggerBoxes);
	m_WorldNpcPreviews = std::move(stagedNpcPreviews);
	m_SpawnGroupDocument = std::move(stagedSpawnGroups);
	m_SpawnAnchorBoxes = std::move(stagedSpawnAnchorBoxes);
	m_NavigationDocument = std::move(stagedNavigation);
	m_RuntimeBlockerDocument = std::move(stagedBlockers);
	m_DestructionDocument = std::move(stagedDestruction);
	m_EncounterReference = std::move(stagedEncounterReference);
	m_DestructionSimulationDocument = std::move(stagedSimulation);
	m_pMapLightPresentation = std::move(stagedMapLightPresentation);
	m_bMapLightSubmissionFailureReported = false;
	m_WorldEventsPath = descriptor.worldEventsDocument;
	m_NavigationSourcePath = descriptor.navigationSource;
	m_NavigationPaintPath = descriptor.navigationPaint;
	m_RuntimeBlockerPath = descriptor.navigationBlockers;
	m_NavigationRuntimePath.clear();
	m_NavigationBakeDesc = navigationLoaded ?
		m_NavigationDocument.Get_BakeDesc() : NAVGRID_BAKE_DESC{};
	m_iActiveEditorArea = descriptorIndex;
	m_bDestructionDebrisPrototypesReady = stagedDebrisPrototypesReady;
	m_DestructionDebrisPrototypeStatus = stagedDebrisPrototypeStatus;
	m_iPendingEditorArea = SIZE_MAX;
	m_isEditorAreaSwitchPending = false;
	m_iSelectedPlacementId = 0;
	m_SelectedWorldPlacementId.clear();
	m_bWorldNpcContinuousPlacement = false;
	m_WorldNpcBrushPreset.reset();
	m_bWorldNpcWaypointPickArmed = false;
	m_bWorldNpcBatchCenterPickArmed = false;
	m_bWorldNpcBatchCenterValid = false;
	m_WorldNpcBehaviorDraftPlacementId.clear();
	m_WorldNpcBehaviorDraft.reset();
	m_bWorldNpcBehaviorDraftDirty = false;
	m_WorldNpcBatchArchetypePool.clear();
	m_WorldNpcBatchDraft.clear();
	m_iWorldNpcBatchDraftBaseRevision = 0;
	m_bWorldNpcBatchCopySelectedBehavior = false;
	m_SelectedSpawnAnchorId.clear();
	m_SelectedSpawnGroupId.clear();
	m_SelectedSpawnWaveId.clear();
	m_SelectedAssetId.clear();
	Remove_WorldTriggerBoxes(m_DestructionHighlightBoxes);
	m_SelectedDestructionGroupId.clear();
	m_SelectedDestructionBindingId.clear();
	m_SelectedDestructionStageId.clear();
	m_SelectedDestructionPatternId.clear();
	m_iSelectedDeployPlacementId = 0;
	m_bDeployDirty = false;
	m_bAnimatedPropPlacementArmed = false;
	m_SelectedDeployAssetId.clear();
	m_AnimatedPropFilter[0] = '\0';
	m_iSelectedAnimatedPropPlacementId = 0;
	Sync_AnimatedPropTransformDraft();
	m_DestructionPreviewPreviousStates.clear();
	m_bDestructionPickArmed = false;
	m_bDestructionAddMemberArmed = false;
	m_bDestructionNewSettingArmed = false;
	m_bDestructionTimelinePlaying = false;
	m_fDestructionTimelineMs = 0.f;
	m_EncounterReferenceStatus = descriptor.encounterReference.empty() ?
		"Active Area declares no encounter reference" :
		stagedEncounterStatus;
	Reset_DestructionSimulationUI();
	if (!m_DestructionSimulationDocument.Get_Profiles().empty())
	{
		const DESTRUCTION_SIMULATION_PROFILE& firstProfile =
			m_DestructionSimulationDocument.Get_Profiles().front();
		m_SelectedDestructionGroupId = firstProfile.groupId;
		Select_DestructionSimulationProfile(firstProfile.profileId);
		Refresh_DestructionHighlight();
	}
	if (!m_bDestructionDebrisPrototypesReady &&
		!descriptor.destructionSimulationDocument.empty())
	{
		m_DestructionSimulationStatus = m_DestructionDebrisPrototypeStatus;
	}
	m_DestructionStatus = descriptor.worldEventsDocument.empty() ?
		"World destruction authoring disabled for this Area" :
		"World destruction authoring ready";
	m_iNextPlacementId = 1;
	for (const PLACED_ENTRY& entry : Authoring_Placements())
	{
		if ((entry.record.transformSource == "editor" ||
			entry.record.transformSource == "legacy") &&
			entry.record.placementId <=
				CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
		{
			m_iNextPlacementId = (std::max)(
				m_iNextPlacementId, entry.record.placementId + 1);
		}
	}
	m_bDirty = false;
	m_bWorldGameplayDirty = false;
	m_bSpawnGroupsDirty = false;
	m_WorldGameplayStatus =
		EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy ?
		"Gameplay authoring ready" : "Gameplay authoring disabled for this Area";
	m_NavigationStatus =
		EDITOR_NAVIGATION_POLICY::NONE == descriptor.navigationPolicy ?
		"Navigation authoring disabled for this Area" :
		(navigationLoaded ? "Navigation authoring ready" :
			"Navigation bootstrap: place Nav Bounds and Bake");
	m_Status = "Active editor Area: " + descriptor.label + " (" +
		descriptor.areaId + ") / " + std::to_string(Authoring_Placements().size()) +
		" placements. Runtime publish is separate.";
	if (!runtimeAttach) Set_EnvironmentPhase(ENVIRONMENT_PHASE::BASELINE);
	Rebuild_EditorSublevelJumps();

	if (!runtimeAttach && !Focus_ActiveEditorAreaCamera())
		m_Status += " Camera focus unavailable: " + m_CameraStatus;
	return true;
}

void Client::CMapTool::Handle_LevelTransition(
	uint32_t currentLevelIndex,
	bool_t isMapAuthoringLevel)
{
	const uint32_t targetLevelIndex = isMapAuthoringLevel ?
		currentLevelIndex : ETOUI(LEVEL::END);
	if (targetLevelIndex == m_iAuthoringLevelIndex)
		return;
	/* The cutscene session's actors and camera claim belonged to the Level
	   being left; drop the tool's references before its containers go. */
	Abandon_EditorCutscene("Cutscene preview stopped: the authoring Level changed.");
	/* The old Level owns the Deploy overlay targets and may already be torn
	   down. Do not carry that preview request into the next Area. */
	m_bCameraPreviewSurroundingsCleared = false;
	m_CameraPreviewSuppressedDeployPlacementIds.clear();
	// Old level ownership is already gone during a Level transition. Never clear borrowed containers.
	m_bRuntimeAuthoring = false;
	m_RuntimePlacementDraft.clear();
	m_RuntimePlacementIndex.clear();
	if (nullptr != m_pWorldSequenceToolPanel && m_Catalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, m_Catalog, Authoring_Placements(),
			Authoring_Deploy());
	}
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	Reset_DestructionSimulationUI();

	m_ePlacementState = PLACEMENT_STATE::IDLE;
	m_bWorldGameplayPlacementArmed = false;
	m_bWorldNpcContinuousPlacement = false;
	m_WorldNpcBrushPreset.reset();
	m_bWorldTriggerTargetPickArmed = false;
	m_bWorldNpcWaypointPickArmed = false;
	m_bWorldNpcBatchCenterPickArmed = false;
	m_bWorldNpcBatchCenterValid = false;
	m_WorldNpcBehaviorDraftPlacementId.clear();
	m_WorldNpcBehaviorDraft.reset();
	m_bWorldNpcBehaviorDraftDirty = false;
	m_WorldNpcBatchArchetypePool.clear();
	m_WorldNpcBatchDraft.clear();
	m_iWorldNpcBatchDraftBaseRevision = 0;
	m_bWorldNpcBatchCopySelectedBehavior = false;
	m_bSpawnAnchorPlacementArmed = false;
	m_bAnimatedPropPlacementArmed = false;
	m_SelectedWorldPlacementId.clear();
	m_SelectedSpawnAnchorId.clear();
	m_SelectedSpawnGroupId.clear();
	m_SelectedSpawnWaveId.clear();
	m_iSelectedPlacementId = 0;
	m_SelectedAssetId.clear();
	m_SelectedDeployAssetId.clear();
	m_AnimatedPropFilter[0] = '\0';
	m_iSelectedAnimatedPropPlacementId = 0;
	Sync_AnimatedPropTransformDraft();
	m_pAssetTestCamera.reset();
	if (nullptr != m_pAssetPreview)
		m_pAssetPreview->Reset_LevelResources();

	Authoring_Placements().clear();
	Authoring_Batches().clear();
	Authoring_Deploy().Reset_ClearedLevelTracking();
	m_WorldTriggerBoxes.clear();
	m_SpawnAnchorBoxes.clear();
	m_iNextPlacementId = 1;
	m_bDirty = false;
	m_bDeployDirty = false;
	m_bWorldGameplayDirty = false;
	m_bSpawnGroupsDirty = false;
	m_SpawnGroupDocument.Reset();
	m_DestructionSimulationDocument.Clear();
	m_pMapLightPresentation.reset();
	m_bMapLightSubmissionFailureReported = false;
	if (nullptr != m_pWorldSequenceToolPanel)
		m_pWorldSequenceToolPanel->Reset();
	m_Catalog = CMapAssetCatalog{};
	m_EditorAreas.clear();
	m_EditorSublevelJumps.clear();
	m_iActiveEditorArea = SIZE_MAX;
	m_iPendingEditorArea = SIZE_MAX;
	m_isEditorAreaSwitchPending = false;
	m_isEditorExitPending = false;
	m_PrototypeModelPaths.clear();
	m_EditorAreaPreload = {};
	m_bDestructionDebrisPrototypesReady = false;
	/* Prototypes live under the Level index that is being left. */
	m_bWorldObjectPrototypeReady = false;
	m_DestructionDebrisPrototypeStatus =
		"PROJECT_AUTHORED debris models are not admitted";
	m_iAuthoringLevelIndex = targetLevelIndex;

	if (!isMapAuthoringLevel)
	{
		m_Status = "Current level has no map Area to author";
		m_WorldGameplayStatus = "Current level has no gameplay Area";
		m_NavigationStatus = "Current level has no navigation Area";
		m_CameraStatus = "Current level has no authoring camera";
		return;
	}

	Find_AssetTestCamera();
	if (!Load_EditorAreaRegistry() || m_EditorAreas.empty())
		return;
	const auto runtimeTargets = Runtime_AuthoringTargets();
	if (runtimeTargets.pCatalog)
	{
		const auto found = std::find_if(m_EditorAreas.begin(), m_EditorAreas.end(),
			[&runtimeTargets](const auto& area) { return area.areaId == runtimeTargets.pCatalog->Get_AreaId(); });
		if (found == m_EditorAreas.end())
			m_Status = "The current runtime Area is not registered for authoring.";
		else Switch_EditorArea(static_cast<size_t>(found - m_EditorAreas.begin()));
	}
	else Switch_EditorArea(0);
}
bool_t Client::CMapTool::Find_AssetTestCamera()
{
	const shared_ptr<CGameObject> gameObject =
		CGameInstance::Get().Get_GameObject(
			m_iAuthoringLevelIndex,
			TEXT("Layer_Camera"),
			0);
	const shared_ptr<CCamera_Free> camera =
		dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == camera)
	{
		m_pAssetTestCamera.reset();
		m_CameraStatus = "ASSET_TEST camera is unavailable";
		return false;
	}

	m_pAssetTestCamera = camera;
	m_CameraStatus = "Camera ready";
	return true;
}

bool_t Client::CMapTool::Focus_ActiveEditorAreaCamera()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor)
	{
		m_CameraStatus = "Select an Area before focusing the camera";
		return false;
	}

	float3_t center{};
	f32_t radius = 0.f;
	bool_t hasFrame = false;
	/* Product spawn data is the stable authoring focus. Distant backdrop
	   meshes must not pull the editor camera away from the playable entry.
	   Any Area that declares player spawns uses them; one without a gameplay
	   document falls through to the placement bounds below. */
	hasFrame = TryBuildGameplaySpawnFrame(
		m_WorldGameplayDocument,
		center,
		radius);

	if (!hasFrame && !Authoring_Placements().empty())
	{
		float3_t minimum = Authoring_Placements().front().record.position;
		float3_t maximum = minimum;
		for (const PLACED_ENTRY& entry : Authoring_Placements())
		{
			minimum.x = (std::min)(minimum.x, entry.record.position.x);
			minimum.y = (std::min)(minimum.y, entry.record.position.y);
			minimum.z = (std::min)(minimum.z, entry.record.position.z);
			maximum.x = (std::max)(maximum.x, entry.record.position.x);
			maximum.y = (std::max)(maximum.y, entry.record.position.y);
			maximum.z = (std::max)(maximum.z, entry.record.position.z);
		}
		center = float3_t(
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f);
		radius = (std::max)(50.f,
			(std::max)(maximum.x - minimum.x, maximum.z - minimum.z) * 0.35f);
		hasFrame = true;
	}
	if (!hasFrame)
	{
		m_CameraStatus = "Active Area has no valid camera focus target";
		return false;
	}

	shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr == camera)
	{
		if (!Find_AssetTestCamera())
			return false;
		camera = m_pAssetTestCamera.lock();
	}
	if (nullptr == camera)
	{
		m_CameraStatus = "ASSET_TEST camera reacquisition failed";
		return false;
	}

	camera->Frame_Area(center, radius);
	m_CameraStatus = "Camera focused on " + descriptor->label;
	return true;
}

/* An extracted Area can span several authored sublevels that sit thousands of
   units apart, so walking between them is impractical. Grouping the committed
   placements by their authored sourceLevel gives one framing target per space
   without hard-coding any single Area's coordinates. */
void Client::CMapTool::Rebuild_EditorSublevelJumps()
{
	m_EditorSublevelJumps.clear();

	struct SUBLEVEL_BOUNDS final
	{
		float3_t minimum{};
		float3_t maximum{};
		size_t count = 0u;
	};
	std::map<std::string, SUBLEVEL_BOUNDS> bounds;
	for (const PLACED_ENTRY& entry : Authoring_Placements())
	{
		if (entry.record.sourceLevel.empty())
			continue;
		const float3_t& position = entry.record.position;
		const auto [iter, inserted] = bounds.try_emplace(
			entry.record.sourceLevel, SUBLEVEL_BOUNDS{ position, position, 0u });
		SUBLEVEL_BOUNDS& value = iter->second;
		if (!inserted)
		{
			value.minimum.x = (std::min)(value.minimum.x, position.x);
			value.minimum.y = (std::min)(value.minimum.y, position.y);
			value.minimum.z = (std::min)(value.minimum.z, position.z);
			value.maximum.x = (std::max)(value.maximum.x, position.x);
			value.maximum.y = (std::max)(value.maximum.y, position.y);
			value.maximum.z = (std::max)(value.maximum.z, position.z);
		}
		++value.count;
	}

	/* A single group is the whole Area, which the Focus Area button already
	   frames, so publishing one shortcut for it would only add a redundant key. */
	if (bounds.size() < 2u)
		return;

	for (const auto& [sourceLevel, value] : bounds)
	{
		EDITOR_SUBLEVEL_JUMP jump;
		jump.label = sourceLevel;
		jump.center = float3_t(
			(value.minimum.x + value.maximum.x) * 0.5f,
			(value.minimum.y + value.maximum.y) * 0.5f,
			(value.minimum.z + value.maximum.z) * 0.5f);
		/* Same framing allowance the Area focus uses, so a jump lands at a
		   comparable distance instead of inside the geometry. */
		jump.radius = (std::max)(50.f,
			(std::max)(value.maximum.x - value.minimum.x,
				value.maximum.z - value.minimum.z) * 0.35f);
		jump.placementCount = value.count;
		m_EditorSublevelJumps.push_back(std::move(jump));
	}
}

bool_t Client::CMapTool::Jump_ToEditorSublevel(const size_t jumpIndex)
{
	if (jumpIndex >= m_EditorSublevelJumps.size())
		return false;

	shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr == camera)
	{
		if (!Find_AssetTestCamera())
			return false;
		camera = m_pAssetTestCamera.lock();
	}
	if (nullptr == camera)
	{
		m_CameraStatus = "ASSET_TEST camera reacquisition failed";
		return false;
	}

	const EDITOR_SUBLEVEL_JUMP& jump = m_EditorSublevelJumps[jumpIndex];
	camera->Frame_Area(jump.center, jump.radius);
	m_CameraStatus = "Camera jumped to " + jump.label + " (" +
		std::to_string(jump.placementCount) + " placements)";
	return true;
}

/* Driven from the workspace bar, which only the isolated Development editor
   shell renders. The keys therefore stop existing outside that shell, outside
   an Area that produced jump targets, and while a text field has focus. */
void Client::CMapTool::Update_EditorSublevelJumpShortcuts()
{
	if (m_EditorSublevelJumps.empty() || ImGui::GetIO().WantTextInput)
		return;

	static constexpr ImGuiKey ROW_KEYS[] = {
		ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4, ImGuiKey_5,
		ImGuiKey_6, ImGuiKey_7, ImGuiKey_8, ImGuiKey_9 };
	static constexpr ImGuiKey PAD_KEYS[] = {
		ImGuiKey_Keypad1, ImGuiKey_Keypad2, ImGuiKey_Keypad3,
		ImGuiKey_Keypad4, ImGuiKey_Keypad5, ImGuiKey_Keypad6,
		ImGuiKey_Keypad7, ImGuiKey_Keypad8, ImGuiKey_Keypad9 };
	const size_t bound = (std::min)(
		m_EditorSublevelJumps.size(), std::size(ROW_KEYS));
	for (size_t index = 0u; index < bound; ++index)
	{
		if (ImGui::IsKeyPressed(ROW_KEYS[index], false) ||
			ImGui::IsKeyPressed(PAD_KEYS[index], false))
		{
			(void)Jump_ToEditorSublevel(index);
			return;
		}
	}
}
```
