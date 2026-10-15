// Loader의 스레드·진행·준비 함수 선언을 구현한다.
#include "Loader.h"
// 정적·애니메이션·instance 셰이더 입력 레이아웃을 지정한다.
#include "Engine_VertexTypes.h"

// 원형 등록·자원 rollback과 profiler 조회에 공용 Engine 진입점을 사용한다.
#include "GameInstance.h"
// Level·모델·navigation 준비 구간의 소요 시간을 기록한다.
#include "Profiler.h"

// 보스·몬스터·탈것의 게시된 표현 정의를 조회한다.
#include "ActorCatalog.h"
// Debug 미리보기 모델·donor·무기 선언 목록을 사용한다.
#include "AnimationPreviewAssets.h"
// 지도와 저작 도구가 복제할 자유 카메라 원형을 만든다.
#include "Camera_Free.h"
// 기존 발탄 몸체 관련 컴파일 의존성을 유지한다. 이 파일의 직접 준비는 표현 서비스가 담당한다.
#include "Body_Valtan.h"
// 복제 플레이어가 사용할 캐릭터 객체 원형을 만든다.
#include "Character.h"
// 선택 가능 직업 목록과 몸체 모델 prototype tag를 조회한다.
#include "CharacterCatalog.h"
// 마하라카 참가 외형의 장비 visual-set을 미리 준비한다.
#include "EquipmentPresentationService.h"
// 워터팡 의상 아이템 ID에서 visual-set ID를 owner 스레드에서 조회한다.
#include "ItemCatalog.h"
// 워터팡 물총 원형과 참가 표현 준비를 연결한다.
#include "MaharakaWaterpangPresentation.h"
// 지도·미리보기 모델과 geometry 공유 재질 변형을 생성한다.
#include "Model.h"
// 기존 플레이어 스킬 catalog 컴파일 의존성을 유지한다. 저작 snapshot 고정은 캐릭터 asset 서비스가 담당한다.
#include "PlayerSkillCatalog.h"
// 기존 선택 상태 헤더 의존성을 유지한다. 실제 준비 직업은 NetworkManager의 Server 승인 값을 사용한다.
#include "CharacterSelectionState.h"
// 선택 화면의 추가 연출 배경 Area 목록을 읽는다.
#include "ClassSelectionPresentation.h"
// 캐릭터·월드 entity가 복제할 collider 원형을 등록한다.
#include "Collider.h"
// Deploy 준비 경로가 사용하는 catalog 타입의 기존 의존성을 유지한다.
#include "DeployPropCatalog.h"
// Deploy 표현 객체의 기존 컴파일 의존성을 유지한다. 원형 생성은 공용 runtime이 담당한다.
#include "DeployPropObject.h"
// Level과 Loader가 공유하는 Deploy 공용 원형·Area 준비를 호출한다.
#include "DeployPropRuntime.h"
// Effect catalog와 함께 쓰는 기존 선언 의존성을 유지한다.
#include "Effect_Catalog.h"
// epoch·revision이 고정된 Effect mailbox를 열고 취소 명령을 보낸다.
#include "Effect_LoadPreparationJob.h"
// Effect producer의 준비 루프를 호출하며 commit은 Loading owner에 남긴다.
#include "Effect_PresentationService.h"
// Esther action 효과음을 미리 준비한다.
#include "EstherActionSoundCueDocument.h"
// Esther cutin 이미지 프레임을 미리 읽는다.
#include "EstherCutinPresentationService.h"
// 원형 등록·자원 rollback과 profiler 조회에 공용 Engine 진입점을 사용한다.
#include "GameInstance.h"
// 대상 Level의 맵 범위와 준비 함수를 동일한 descriptor에서 얻는다.
#include "LevelRegistry.h"
// worker 시작 실패의 구체적인 원인을 로비 복구 진단에 남긴다.
#include "LevelTransitionService.h"
// Area가 선언한 모델·재질 정의를 읽는다.
#include "MapAssetCatalog.h"
// 개별 지도 배치 객체의 Clone용 원형을 등록한다.
#include "MapAssetObject.h"
// 저작 미리보기 셰이더 prototype tag를 사용한다.
#include "MapAssetPreview.h"
// 게시된 기본·상세 영역 navigation 경로와 step height를 확인한다.
#include "MapNavigationContract.h"
// registry scope로 필요한 배치를 고르고 활성화가 소비할 stage를 보관한다.
#include "MapPlacementRuntime.h"
// 동일 geometry instance 배치를 위한 객체 원형을 등록한다.
#include "MapStaticBatchObject.h"
// SOURCEINDEX 입력을 쓰는 chunk 셰이더와 객체 원형을 등록한다.
#include "MapStaticChunkObject.h"
// 직업 목록과 셰이더 입력 선언의 고정 크기 배열을 사용한다.
#include <array>
// 원본 연출 문서를 Loading에서 준비해 활성화 프레임의 재파싱을 피한다.
#include "WorldSequencePlayer.h"
// 연출 배우를 복제할 객체 원형을 등록한다.
#include "WorldSequenceObject.h"
// 기본 NavGrid와 상세 region grid를 생성하고 결합한다.
#include "Navigation.h"
// 현재 입장 승인으로 확정된 로컬 직업을 조회한다.
#include "NetworkManager.h"
// 발탄 아레나의 지원 몬스터 모델을 선택적으로 준비한다.
#include "MonsterPresentationAssetService.h"
// 쿠크 보스 표현의 Level별 원형을 준비한다.
#include "KoukuSaydonPresentationAssetService.h"
// 월드별 NPC 표현 binding을 준비한다. 누락은 각 Level의 선택적 실패 경계를 따른다.
#include "NpcPlacementPresentationService.h"
// Esther·워터팡 NPC의 모델 원형을 준비한다.
#include "NpcPresentationAssetService.h"
// 캐릭터 몸체 파트 객체 원형을 등록한다.
#include "Part_Body.h"
// 장착 장비 파트 객체 원형을 등록한다.
#include "Part_Equipment.h"
// 탈것 파트 객체 원형을 등록한다.
#include "Part_Vehicle.h"
// Level별 탈것 준비 상태를 시작하고 Release의 모델을 미리 준비한다.
#include "VehiclePresentationAssetService.h"
// 직업 저작 snapshot을 고정하고 준비된 모델 원형을 등록한다.
#include "PlayableCharacterAssetService.h"
// Resources 상대 asset ID를 검증한 실제 경로로 해석한다.
#include "RuntimeAssetRoot.h"
// Debug 저작 도구의 trigger·collision·spawn 박스 원형을 만든다.
#include "Trigger_Box.h"
// 발탄 객체의 기존 컴파일 의존성을 유지한다. 실제 준비는 표현 서비스에 위임한다.
#include "Valtan.h"
// 발탄 본체·유령 모델의 공용 준비 경로를 호출한다.
#include "ValtanPresentationAssetService.h"

#ifdef _DEBUG
// Debug Development가 Map Editor 요청인지 확인하고 준비 실패 시 요청을 취소한다.
#include "MapEditorWorkspaceService.h"
#endif

// 직업 중복 검색과 셰이더 입력 복사, 진행량 상한에 사용한다.
#include <algorithm>
// 직업 목록과 셰이더 입력 선언의 고정 크기 배열을 사용한다.
#include <array>
// 미리보기 변환의 유한성 및 navigation step height 오차를 검사한다.
#include <cmath>
// worker 예외를 HRESULT로 변환하고 종료 불변식 실패 시 terminate한다.
#include <exception>
// geometry 준비 중 특정 모델을 가리키지 않는 실패 index 표식을 사용한다.
#include <limits>
// load scope에 포함된 배치의 asset ID를 중복 없이 모은다.
#include <unordered_set>
// 같은 실제 모델 경로의 재질 변형을 geometry 준비 그룹으로 묶는다.
#include <unordered_map>

namespace
{
// Loader 인스턴스 외부의 실패 진단용 마지막 상태다. 인스턴스 진행 정보와 별도 mutex로 보호한다.
std::mutex g_ActiveStatusMutex;
std::string g_ActiveStatus = "Loader has not started.";

// Windows wide 진행 문구를 로그·UI용 UTF-8 복사본으로 바꾼다. 빈 입력 또는 길이 계산 실패는 빈 문자열이다.
std::string ToUtf8(const tchar_t *pText)
{
	if (nullptr == pText || L'\0' == *pText)
		return {};
	const int characterCount = static_cast<int>(wcslen(pText));
	const int byteCount = WideCharToMultiByte(CP_UTF8, 0, pText, characterCount, nullptr, 0, nullptr, nullptr);
	if (byteCount <= 0)
		return {};
	std::string result(static_cast<size_t>(byteCount), '\0');
	WideCharToMultiByte(CP_UTF8, 0, pText, characterCount, result.data(), byteCount, nullptr, nullptr);
	return result;
}

// Level별 Ready_For의 임시 준비 범위다. Commit 전 이탈하면 해당 Level의 등록 자원을 정리한다.
// 여기서 외부 catalog나 표시 서비스의 모든 상태까지 되돌리는 것은 아니다.
class CLevelResourceRollbackScope final
{
  public:
	explicit CLevelResourceRollbackScope(const uint32_t levelIndex) : m_iLevelIndex{levelIndex}
	{
	}

	~CLevelResourceRollbackScope()
	{
		if (m_isCommitted)
			return;

		if (FAILED(CGameInstance::Get().Clear_Resources(m_iLevelIndex)))
		{
			OutputDebugStringA("[Loader] Failed to roll back level resources.\n");
		}
	}

	CLevelResourceRollbackScope(const CLevelResourceRollbackScope &) = delete;
	CLevelResourceRollbackScope &operator=(const CLevelResourceRollbackScope &) = delete;

	void Commit()
	{
		m_isCommitted = true;
	}

  private:
	uint32_t m_iLevelIndex = {};
	bool_t m_isCommitted = false;
};

// Resources 상대 ID에 공용 경로 검사를 적용한다. 파일 열기와 모델 준비는 이 함수에서 하지 않는다.
std::string ResolveAssetPath(const std::filesystem::path &relativePath)
{
	return CRuntimeAssetRoot::Resolve(relativePath).string();
}

// Loading 진행 문구에 사용할 직업명을 고른다. 알 수 없는 ID는 표시용 Unknown이며 입장 승인을 대신하지 않는다.
const tchar_t *Get_CharacterClassName(const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
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
} // namespace

// Create가 장치 COM 참조를 보관한다. worker는 Initialize가 입력 snapshot을 준비한 뒤 시작한다.
CLoader::CLoader(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice{pDevice}, m_pContext{pContext}
{
}

// 소유한 worker가 this를 더 이상 참조하지 않도록 Free를 거쳐 소멸한다.
CLoader::~CLoader()
{
	Free();
}

// Level worker의 진입점이다. COM 초기화 성공 때만 해제하며 Start_Loading이 실제 준비·상태 공개를 담당한다.
// main thread의 진입점 함수는 client.cpp의 APIENTRY 함수이고, worker thread 함수의 진입점은 해당 APIENTRY threadmain(void *pArgument)
uint32_t APIENTRY ThreadMain(void *pArgument)
{
	const HRESULT comResult = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	CLoader *pLoader = static_cast<CLoader *>(pArgument);
	const HRESULT loadResult = nullptr == pLoader ? E_POINTER : pLoader->Start_Loading();

	if (SUCCEEDED(comResult))
		CoUninitialize();

	return FAILED(loadResult) ? 1u : 0u;
}

// Effect job을 만들지 않는 기존 진입점이다. 상태와 worker 생성은 공통 overload에 위임한다.
HRESULT CLoader::Initialize(const LEVEL eNextLevelID)
{
	return Initialize(eNextLevelID, 0u, 0u);
}

// Loading owner가 호출한다. 대상·Effect 입력을 검증하고 catalog snapshot을 고정한 뒤 worker를 만든다.
// 실패는 구체적인 진단을 먼저 남기며 Create의 임시 Loader 소멸이 시작된 worker까지 정리한다.
HRESULT CLoader::Initialize(const LEVEL eNextLevelID, const uint64_t iEffectLoadJobEpoch,
							const uint64_t iEffectCatalogRevision)
{
	//오류 원인 진단 후 결과 반환
	const auto reject = [](const HRESULT result, const std::string_view source, const std::string_view detail)
	{
		// Create가 실패한 Loader를 정리하고 MainApp이 일반적인 null 오류를 보고하기 전에 구체적인 원인을 남긴다.
		// 전환 서비스는 이번 로드 시도의 최초 복구 원인을 보존한다.
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_LOADING_START_FAILED, source, detail, result);
		return result;
	};

	if (nullptr == CLevelRegistry::Find(eNextLevelID))
    {
        return reject(E_INVALIDARG, "loader.initialize.target", "Target Level is not registered.");
	}

	if ((0u == iEffectLoadJobEpoch) != (0u == iEffectCatalogRevision))
	{
        return reject(E_INVALIDARG,
                      "loader.initialize.effect-input",
                      "Effect preparation epoch and catalog revision must both be present or absent.");
	}

	if (0u != iEffectLoadJobEpoch)
	{
		m_pEffectLoadJob = std::make_shared<CEffectLoadPreparationJob>();
		std::string JobStatus;
		if (nullptr == m_pEffectLoadJob ||
			!m_pEffectLoadJob->Open(iEffectLoadJobEpoch, iEffectCatalogRevision, JobStatus))
		{
			OutputDebugStringA(("[Loader][Effect] " + JobStatus + "\n").c_str());
			m_pEffectLoadJob.reset();
			return reject(E_FAIL, "loader.initialize.effect-job", JobStatus);
		}
	}

	// 변경 가능한 catalog의 직업·스킬 입력을 owner 스레드에서 고정한다.
	// 이후 F1 재로드가 있어도 worker는 이 읽기 전용 snapshot을 사용한다.
	if (eNextLevelID != LEVEL::LOBBY)
	{
		// ActorCatalog의 지연 초기화는 공유 vector를 바꾸므로 Level·Effect worker가 읽기 전에 owner에서 마친다.
		if (!CActorCatalog::Initialize())
		{
			OutputDebugStringA(("[Loader] " + CActorCatalog::Get_Status() + "\n").c_str());
			return reject(E_FAIL, "loader.initialize.actor-catalog", CActorCatalog::Get_Status());
		}

		//준비 완료된 상태의 CHARACTER_CLASS_ID 받아오기
		m_ePreparedCharacterClass = CNetworkManager::Get().Get_LocalCharacterClass();

		// 로비에서 승인받은 입장 세대의 직업을 사용한다. 선택 화면의 선택 캐시는 Server 입장 권한을 대신하지 않는다.
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(m_ePreparedCharacterClass))
		{
			Set_Status(TEXT("Character preparation rejected: Server admission has no supported class."));

			OutputDebugStringA("[Loader] Missing/invalid Server-approved character class.\n");

			return reject(E_INVALIDARG, "loader.initialize.character-class",
						  "Server admission has no supported character class.");
		}

		std::vector<LostArk::Shared::CHARACTER_CLASS_ID> selectedClass = { m_ePreparedCharacterClass };
		//마하라카 레벨일 경우 AI와 아바타 준비
		if (eNextLevelID == LEVEL::MAHARAKA)
		{
			//가디언나이트, 창술사 AI selectedClass에 push_back해서 aiClass 준비 시키기 
            for (const auto aiClass : {LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT,
                                       LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER})
            {
                if (std::find(selectedClass.begin(), selectedClass.end(), aiClass) == selectedClass.end())
                    selectedClass.push_back(aiClass);
            }
				
			// worker를 시작하기 전에 catalog owner에서 아이템에 대응하는 visual-set ID를 복사한다.
			for (unsigned slot = 0u; slot < 12u; ++slot)
			{
				std::string item = slot % 2u ? "AVATAR_LANCEMASTER_MOKOKO_036" : "AVATAR_GUARDIANKNIGHT_MOKOKO_036";

                if (slot / 2u)
                {
                    item += "-" + std::to_string(slot / 2u);
                }
				//접미사를 찾고, CItemCatalog에서 준비
				for (const char *suffix : {"_HEAD", "_OUTFIT"})
				{
					const auto *definition = CItemCatalog::Find_ById(item + suffix);

                    if (!definition || definition->strVisualSetId.empty())
                    {
                        return reject(E_FAIL,
                                      "loader.initialize.waterpang-avatar",
                                      "Waterpang avatar item is unavailable.");
                    }
						
					m_MaharakaAvatarVisualSetIds.push_back(definition->strVisualSetId);
				}
			}
		}

		std::span<const LostArk::Shared::CHARACTER_CLASS_ID> preparationClasses = selectedClass;

#ifndef _DEBUG
        if (eNextLevelID == LEVEL::CHARACTER_SELECT)
        {
            preparationClasses = CCharacterCatalog::CHARACTER_SELECT_CLASSES;
		}
#endif

		for (const auto characterClass : preparationClasses)
		{
			auto input = CPlayableCharacterAssetService::Capture_AuthoringInput(characterClass);

			if (!input)
            {
                return reject(E_FAIL,
                              "loader.initialize.character-authoring",
                              "Character authoring snapshot could not be captured; player skills are unavailable.");
            }
				
			m_CharacterAuthoringInputs.emplace(characterClass, std::move(input));
		}
	}

	m_eNextLevelID = eNextLevelID;
	m_iResult.store(S_FALSE, std::memory_order_release);
	m_eState.store(STATE::RUNNING, std::memory_order_release);

	// Effect 요청은 Loading owner가 고정해 mailbox에 넣는다.
	// producer는 지도·캐릭터 준비와 겹쳐 명령을 기다릴 수 있으며 대상 Level의 원형 registry를 직접 변경하지 않는다.
	if (m_pEffectLoadJob && (m_pDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_SINGLETHREADED) == 0u)
	{
		m_hEffectThread =
			reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0u, &CLoader::EffectThreadMain, this, 0u, nullptr));

		if (!m_hEffectThread)
			OutputDebugStringA("[Loader] Effect producer creation failed; using serial preparation.\n");
	}

	//thread의 생성? 여기에서 worker thread가 만들어지고, APIENTRY 진입점이 만들어져서, 진입할 수 있는 구조가 되나?
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadMain, this, 0, nullptr));

	if (nullptr == m_hThread)
	{
		Request_Cancellation();
		m_iResult.store(E_FAIL, std::memory_order_release);
		m_eState.store(STATE::FAILED, std::memory_order_release);
		return reject(E_FAIL, "loader.initialize.worker", "Could not create the Level loading worker.");
	}

	return S_OK;
}

// Level worker가 registry의 준비 경로를 실행하고 Effect worker 결과까지 합친다.
// 예외·취소·Effect 최초 실패를 HRESULT로 정리한 뒤 결과와 최종 상태를 release로 공개한다.
HRESULT CLoader::Start_Loading()
{
	// worker의 profiler 구간은 끝난 프레임에 집계된다. 오래 걸린 구간은 Long operations 목록에서 확인한다.
	Engine::CProfiler *const pProfiler = CGameInstance::Get().Get_Profiler();

	HRESULT result = S_OK;

	try
	{
		Engine::CProfilerScope levelScope(pProfiler, "Loader.LevelLoad");
		result = CLevelRegistry::Execute_Load(m_eNextLevelID, *this);
	}
	catch (const std::bad_alloc &)
	{
		result = E_OUTOFMEMORY;
	}
	catch (...)
	{
		result = E_FAIL;
	}
	if (FAILED(result))
		Request_Cancellation();
	if (nullptr != m_hEffectThread)
	{
		// 이 worker가 기다리는 동안 주 스레드는 Effect queue를 계속 소비한다.
		// 종료 제한 시간과 두 producer HANDLE의 정리는 Free가 관리한다.
		const DWORD waited = WaitForSingleObject(m_hEffectThread, INFINITE);
		if (WAIT_OBJECT_0 != waited)
		{
			OutputDebugStringA("[Loader] Effect producer join failed; terminating the process.\n");
			if (!TerminateProcess(GetCurrentProcess(), ERROR_INVALID_HANDLE))
				std::terminate();
			__assume(0);
		}
		const HRESULT effectResult = static_cast<HRESULT>(m_iEffectResult.load(std::memory_order_acquire));
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
		catch (const std::bad_alloc &)
		{
			result = E_OUTOFMEMORY;
		}
		catch (...)
		{
			result = E_FAIL;
		}
	}
	if (SUCCEEDED(result) && m_isCancellationRequested.load(std::memory_order_acquire))
		result = HRESULT_FROM_WIN32(ERROR_CANCELLED);
	if (FAILED(result) && m_pEffectLoadJob)
	{
		const auto Failure = m_pEffectLoadJob->Get_FirstFailure();
		if (Failure && FAILED(static_cast<HRESULT>(Failure->iRootCode)))
			result = static_cast<HRESULT>(Failure->iRootCode);
	}
	if (FAILED(result))
		Request_Cancellation();
	m_iResult.store(result, std::memory_order_release);
	m_eState.store(SUCCEEDED(result) ? STATE::SUCCEEDED : STATE::FAILED, std::memory_order_release);
	if (FAILED(result))
		OutputDebugStringW(L"[Loader] Level load failed.\n");
	return result;
}

// Effect producer의 공용 준비 루프에 장치·mailbox·취소·batch를 전달한다. 등록 commit은 주 스레드에 남긴다.
HRESULT CLoader::Run_EffectLoadPreparation()
{
	return CEffectPresentationService::Run_ProductPreparationWorker(
		m_pDevice, m_pContext, m_pEffectLoadJob, &m_isCancellationRequested, &m_EffectPreparationBatch);
}

// Effect 전용 worker의 COM 수명과 예외 경계다. 결과를 공개하고 실패하면 Level 쪽에도 협력 취소를 요청한다.
unsigned __stdcall CLoader::EffectThreadMain(void *pArgument)
{
	auto *loader = static_cast<CLoader *>(pArgument);
	const HRESULT apartment = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	HRESULT result = apartment;
	if (SUCCEEDED(apartment))
	{
		try
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.EffectPreparation");
			result = loader->Run_EffectLoadPreparation();
		}
		catch (const std::bad_alloc &)
		{
			result = E_OUTOFMEMORY;
		}
		catch (...)
		{
			result = E_FAIL;
		}
		CoUninitialize();
	}
	loader->m_iEffectResult.store(result, std::memory_order_release);
	if (FAILED(result))
		loader->Request_Cancellation();
	return FAILED(result) ? 1u : 0u;
}

// 모든 준비 callback이 확인할 atomic을 먼저 설정하고 열려 있는 Effect job에 취소 명령을 보낸다.
// 명령 게시 예외는 로그로 남기되 Free의 제한 시간 대기는 계속 유지한다.
void CLoader::Request_Cancellation() noexcept
{
	m_isCancellationRequested.store(true, std::memory_order_release);
	try
	{
		if (m_pEffectLoadJob && !m_pEffectLoadJob->Is_Cancelled() && !m_pEffectLoadJob->Is_Closed())
		{
			std::optional<EFFECT_LOAD_JOB_COMMAND> displaced;
			std::string status;
			(void)m_pEffectLoadJob->Post_Command(
				EFFECT_LOAD_JOB_COMMAND::Cancel(m_pEffectLoadJob->Get_CurrentEpoch(),
												m_pEffectLoadJob->Get_CurrentCatalogRevision()),
				displaced, status);
		}
	}
	catch (...)
	{
		OutputDebugStringA("[Loader] Could not post Effect cancellation; bounded shutdown remains active.\n");
	}
}

// 준비 함수가 새 단계에 들어갈 때 호출한다. 텍스트·작업량·단계를 함께 갱신하고 공용 진단 문구도 복사한다.
void CLoader::Set_Status(const tchar_t *pStatus)
{
	{
		lock_guard<mutex> lock(m_StatusMutex);
		const bool_t bChanged =
			nullptr == pStatus ? L'\0' != m_szLoadingText[0] : 0 != wcscmp(m_szLoadingText, pStatus);
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

// 모델 준비 callback이 같은 단계의 수량을 갱신한다. 완료 수는 total로 제한하고 total 0은 미확정 진행으로 표현한다.
void CLoader::Set_DeterminateStatus(const tchar_t *pStatus, const size_t iCompleted, const size_t iTotal)
{
	{
		lock_guard<mutex> lock(m_StatusMutex);
		const bool_t bChanged =
			nullptr == pStatus ? L'\0' != m_szLoadingText[0] : 0 != wcscmp(m_szLoadingText, pStatus);
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

// Level별 Ready_For가 전체 단계 수를 선언하고 시작 index를 0으로 되돌린다.
void CLoader::Declare_Phases(const size_t iCount)
{
	lock_guard<mutex> lock(m_StatusMutex);
	m_iPhaseIndex = 0u;
	m_iPhaseCount = iCount;
}

// 선택적 지도 범위 등 실제 추가 준비가 있을 때 전체 단계 수를 늘린다.
void CLoader::Add_Phases(const size_t iCount)
{
	lock_guard<mutex> lock(m_StatusMutex);
	m_iPhaseCount += iCount;
}

// 실패 진단 소비자가 마지막 진행 문구를 복사해 받는다. 내부 문자열 참조를 노출하지 않는다.
std::string CLoader::Get_ActiveStatus()
{
	lock_guard<mutex> lock(g_ActiveStatusMutex);
	return g_ActiveStatus;
}

// Loading 화면이 한 번의 lock에서 단계·수량·경과 시간을 복사한다. UTF-8 변환은 lock 밖에서 수행한다.
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
		Snapshot.iElapsedMs = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
														std::chrono::steady_clock::now() - m_ProgressPhaseStarted)
														.count());
	}
	Snapshot.strStatus = ToUtf8(Status);
	return Snapshot;
}

// Debug 표시용 버퍼로 현재 문구를 안전하게 복사한다. 잘못된 출력 버퍼는 변경하지 않는다.
void CLoader::Copy_Status(tchar_t *pOutput, const size_t outputCount) const
{
	if (nullptr == pOutput || 0u == outputCount)
		return;
	lock_guard<mutex> lock(m_StatusMutex);
	wcsncpy_s(pOutput, outputCount, m_szLoadingText, _TRUNCATE);
}

#ifdef _DEBUG
// Debug 창 제목을 현재 준비 문구로 갱신한다. worker 상태 조회용이며 로딩을 진행시키지 않는다.
void CLoader::Print_Text()
{
	tchar_t status[MAX_PATH]{};
	Copy_Status(status, size(status));
	SetWindowText(g_hWnd, status);
}
#endif

// 로비의 저장 캐릭터 카드가 나중에 사용할 공용 렌더링 원형만 준비한다. 성공 전 실패는 로비 자원을 정리한다.
HRESULT CLoader::Ready_For_Lobby()
{
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::LOBBY));
	Declare_Phases(3u);
	Set_Status(TEXT("LOBBY: stage selection UI"));
	// 처음 열린 캐릭터 카드가 저장한 직업을 준비한다. Lobby가 직업 모델을 미리 읽지 않아도 공용 렌더링 원형은 필요하다.
	const uint32_t levelIndex = ETOUI(LEVEL::LOBBY);
	CPlayableCharacterAssetService::Begin_LevelLoad(levelIndex);
	if (FAILED(Ready_StaticMeshShader(levelIndex)) || FAILED(Ready_AnimatedMeshShader(levelIndex)) ||
		FAILED(Ready_Character_Shared_Prototypes(levelIndex)))
	{
		OutputDebugStringA("[Loader][Lobby] Shared character rendering preparation failed.\n");
		return E_FAIL;
	}
	Set_Status(TEXT("Lobby loading complete"));
	rollback.Commit();
	return S_OK;
}

// 캐릭터 선택 맵·직업 원형과 선택적 연출을 준비한다. 필수 실패는 rollback하며 선택적 배경·탈것 오류는 로그로 격리한다.
HRESULT CLoader::Ready_For_CharacterSelect()
{
	CValtanPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::CHARACTER_SELECT));
	CNpcPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::CHARACTER_SELECT));
	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::CHARACTER_SELECT));
	if (FAILED(CNpcPlacementPresentationService::Load(ETOUI(LEVEL::CHARACTER_SELECT), "CHARACTER_SELECT_ARENA")))
	{
		OutputDebugStringA(
			("[Loader][NpcPresentation] " + CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
#ifdef _DEBUG
	const std::array<LostArk::Shared::CHARACTER_CLASS_ID, 1> characterClasses = {m_ePreparedCharacterClass};
#else
	// 첫 사용 직업 모델의 준비를 진입 Loading 단계에서 끝낸다.
	constexpr auto characterClasses = CCharacterCatalog::CHARACTER_SELECT_CLASSES;
#endif

	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::CHARACTER_SELECT));

	const CLIENT_LEVEL_DESCRIPTOR *pEntry = CLevelRegistry::Find(LEVEL::CHARACTER_SELECT);

	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId)
		return E_INVALIDARG;

	Declare_Phases(9u);
	Set_Status(TEXT("CHARACTER SELECT: visual map"));
	if (FAILED(Ready_MapArea(ETOUI(LEVEL::CHARACTER_SELECT), pEntry->pMapAreaId, pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("CHARACTER SELECT: playable classes"));
	if (FAILED(Ready_Character_Rendering(ETOUI(LEVEL::CHARACTER_SELECT), characterClasses)))
	{
		return E_FAIL;
	}
	// Server가 이 아레나에서도 Esther gauge를 허용하므로 발탄과 같은 소환 모델을 미리 준비한다.
	// 키를 누른 프레임에 최초 모델 준비가 몰리지 않게 하며 누락 자료는 해당 소환 표시만 격리한다.
	Set_Status(TEXT("CHARACTER SELECT: esther summon presentation"));
	Ready_EstherSummonPresentation(ETOUI(LEVEL::CHARACTER_SELECT));
	CVehiclePresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::CHARACTER_SELECT));
#ifndef _DEBUG
	Set_Status(TEXT("CHARACTER SELECT: vehicle presentation"));
	for (const VEHICLE_ACTOR_ENTRY &vehicle : CActorCatalog::Get_Vehicles())
	{
		if (FAILED(CVehiclePresentationAssetService::Ensure_Prototypes(
				m_pDevice, m_pContext, ETOUI(LEVEL::CHARACTER_SELECT), vehicle.vehicleId)))
		{
			OutputDebugStringA(("[Loader][VehiclePresentation] CHARACTER SELECT vehicle " +
								std::to_string(vehicle.vehicleId) + " is unavailable; the level loads on foot.\n")
								   .c_str());
		}
	}
#endif
	Set_Status(TEXT("CHARACTER SELECT: class selection cinematics"));
	if (CClassSelectionPresentation::Is_Configured())
	{
		std::vector<std::string> backgroundAreas;
		std::string backgroundStatus;
		if (!CClassSelectionPresentation::Load_BackgroundAreas(
				pEntry->pMapAreaId, pEntry->pPresentationMapAreaId ? pEntry->pPresentationMapAreaId : "",
				backgroundAreas, backgroundStatus))
			OutputDebugStringA(("[Loader][ClassSelectionCinema] " + backgroundStatus + "\n").c_str());
		else
			for (const auto &backgroundArea : backgroundAreas)
			{
				// 주 Area는 이미 준비했으므로 같은 원형을 중복 등록하지 않는다.
				if (backgroundArea == pEntry->pMapAreaId)
					continue;
				if (FAILED(Ready_MapArea(ETOUI(LEVEL::CHARACTER_SELECT), backgroundArea,
										 pEntry->PresentationMapLoadScope, false)))
				{
					if (m_isCancellationRequested.load(std::memory_order_acquire))
						return HRESULT_FROM_WIN32(ERROR_CANCELLED);
					OutputDebugStringA(
						("[Loader][ClassSelectionCinema] Optional background unavailable: " + backgroundArea + "\n")
							.c_str());
				}
			}
	}
	std::string selectionSequenceStatus;
	if (!CWorldSequencePlayer::Prepare_AreaLoad(ETOUI(LEVEL::CHARACTER_SELECT), pEntry->pMapAreaId,
												pEntry->MapLoadScope, selectionSequenceStatus, [this]()
												{ return m_isCancellationRequested.load(std::memory_order_acquire); }))
	{
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		OutputDebugStringA(("[Loader][ClassSelectionCinema] " + selectionSequenceStatus + "\n").c_str());
	}
	Set_Status(TEXT("Character Select loading complete"));
	rollback.Commit();
	return S_OK;
}

// registry의 베른 scope와 Server 승인 직업을 준비한다. NPC binding과 탈것은 선택적이며 필수 준비 성공 후 rollback을 해제한다.
HRESULT CLoader::Ready_For_Bern()
{
	CNpcPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::BERN));
	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::BERN));
	if (FAILED(CNpcPlacementPresentationService::Load(ETOUI(LEVEL::BERN), "BERN")))
	{
		OutputDebugStringA(
			("[Loader][NpcPresentation] " + CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::BERN));
	Declare_Phases(7u);
	Set_Status(TEXT("BERN: world catalog and placements"));

	const CLIENT_LEVEL_DESCRIPTOR *pEntry = CLevelRegistry::Find(LEVEL::BERN);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(ETOUI(LEVEL::BERN), pEntry->pMapAreaId, pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("BERN: session character bundle"));
	const std::array<LostArk::Shared::CHARACTER_CLASS_ID, 1> selectedClass = {m_ePreparedCharacterClass};
	if (FAILED(Ready_Character_Rendering(ETOUI(LEVEL::BERN), selectedClass)))
		return E_FAIL;

	CVehiclePresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::BERN));
#ifndef _DEBUG
	Set_Status(TEXT("BERN: vehicle presentation"));
	for (const VEHICLE_ACTOR_ENTRY &vehicle : CActorCatalog::Get_Vehicles())
	{
		if (FAILED(CVehiclePresentationAssetService::Ensure_Prototypes(m_pDevice, m_pContext, ETOUI(LEVEL::BERN),
																	   vehicle.vehicleId)))
		{
			OutputDebugStringA(("[Loader][VehiclePresentation] BERN vehicle " + std::to_string(vehicle.vehicleId) +
								" is unavailable; the level loads on foot.\n")
								   .c_str());
		}
	}
#endif
	Set_Status(TEXT("Bern loading complete"));
	rollback.Commit();
	return S_OK;
}

// 발탄 맵·승인 직업·보스·Deploy를 필수 준비하고 몬스터·Esther·원본 연출을 각 선택적 경계에서 준비한다.
HRESULT CLoader::Ready_For_ValtanArena()
{
	CValtanPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::VALTAN_ARENA));
	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::VALTAN_ARENA));
	if (FAILED(CNpcPlacementPresentationService::Load(ETOUI(LEVEL::VALTAN_ARENA), "VALTAN_ARENA")))
	{
		OutputDebugStringA(
			("[Loader][NpcPresentation] " + CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CMonsterPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::VALTAN_ARENA));
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::VALTAN_ARENA));
	Declare_Phases(10u);
	Set_Status(TEXT("VALTAN: arena map"));

	const CLIENT_LEVEL_DESCRIPTOR *pEntry = CLevelRegistry::Find(LEVEL::VALTAN_ARENA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(ETOUI(LEVEL::VALTAN_ARENA), pEntry->pMapAreaId, pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: network player rendering"));
	const std::array<LostArk::Shared::CHARACTER_CLASS_ID, 1> selectedClass = {m_ePreparedCharacterClass};
	if (FAILED(Ready_Character_Rendering(ETOUI(LEVEL::VALTAN_ARENA), selectedClass)))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: server-authoritative boss presentation"));
	if (FAILED(Ready_ValtanPresentation(ETOUI(LEVEL::VALTAN_ARENA))))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: monster presentation warmup"));
	for (const MONSTER_ACTOR_ENTRY &monster : CActorCatalog::Get_Monsters())
	{
		if (monster.runtimeStatus != "supported")
			continue;
		if (FAILED(CMonsterPresentationAssetService::Ensure_Prototypes(
				m_pDevice, m_pContext, ETOUI(LEVEL::VALTAN_ARENA), monster.archetypeId)))
		{
			OutputDebugStringA(
				("[Loader][MonsterPresentation] unavailable archetype: " + monster.archetypeId + "\n").c_str());
		}
	}
	// 전투 중 나타나는 Esther의 모델은 아레나가 열리기 전에 원형으로 준비한다.
	// 현재 소환 목록은 아래 Ready_EstherSummonPresentation에 선언되어 있으며 Server의 소환 목록과 대응한다.
	Set_Status(TEXT("VALTAN: esther summon presentation"));
	Ready_EstherSummonPresentation(ETOUI(LEVEL::VALTAN_ARENA));
	Set_Status(TEXT("VALTAN: deploy environment prototypes"));
	if (FAILED(Ready_DeployPropArea(ETOUI(LEVEL::VALTAN_ARENA), pEntry->pMapAreaId)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("VALTAN: source cinematic world sequences"));
	// 원본 연출 배우는 Map Tool을 열기 전에도 Server 표시와 Workbench가 사용한다.
	// 다른 발탄 원형과 같은 rollback 범위의 Level 로드에서 Clone용 원형을 등록한다.
	if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::VALTAN_ARENA), CWorldSequenceObject::PROTOTYPE_TAG,
												  CWorldSequenceObject::Create(m_pDevice, m_pContext))))
	{
		Set_Status(TEXT("VALTAN: source cinematic object prototype failed"));
		return E_FAIL;
	}
	std::string worldSequenceStatus;
	if (!CWorldSequencePlayer::Prepare_AreaLoad(ETOUI(LEVEL::VALTAN_ARENA), pEntry->pMapAreaId, pEntry->MapLoadScope,
												worldSequenceStatus, [this]()
												{ return m_isCancellationRequested.load(std::memory_order_acquire); }))
	{
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		OutputDebugStringA(("[Loader][ValtanSourceCinema] " + worldSequenceStatus + "\n").c_str());
	}
	Set_Status(TEXT("Valtan arena loading complete"));
	rollback.Commit();
	return S_OK;
}

// 쿠크 맵·승인 직업·보스·Deploy를 준비한다. 선택적 표시 오류는 기록하며 취소는 전체 준비 실패로 전달한다.
HRESULT CLoader::Ready_For_KoukuSaydonArena()
{
	//Map Asset, NPC Asset Loading
	CKoukuSaydonPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::KOUKUSAYDON_ARENA));

	CNpcPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::KOUKUSAYDON_ARENA));

	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::KOUKUSAYDON_ARENA));

	if (FAILED(CNpcPlacementPresentationService::Load(ETOUI(LEVEL::KOUKUSAYDON_ARENA), "KOUKUSAYDON_ARENA")))
	{
		// NPC 배치 표시가 아직 저작되지 않아도 월드에 입장할 수 있다.
		// Server entity는 catalog 기본 idle을 사용하며 누락된 선택적 binding은 로그로 남긴다.
		OutputDebugStringA(
			("[Loader][NpcPresentation] " + CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}

	CMonsterPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::KOUKUSAYDON_ARENA));

	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::KOUKUSAYDON_ARENA));
	Declare_Phases(9u);
	Set_Status(TEXT("KoukuSaydon: arena map"));

	const CLIENT_LEVEL_DESCRIPTOR *pEntry = CLevelRegistry::Find(LEVEL::KOUKUSAYDON_ARENA);

	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(ETOUI(LEVEL::KOUKUSAYDON_ARENA), pEntry->pMapAreaId, pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("KoukuSaydon: server-approved character rendering"));
	const std::array<LostArk::Shared::CHARACTER_CLASS_ID, 1> selectedClass = {m_ePreparedCharacterClass};
	if (FAILED(Ready_Character_Rendering(ETOUI(LEVEL::KOUKUSAYDON_ARENA), selectedClass)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("KoukuSaydon: Server boss presentation"));
	if (FAILED(CKoukuSaydonPresentationAssetService::Ensure_Prototypes(
			m_pDevice, m_pContext, ETOUI(LEVEL::KOUKUSAYDON_ARENA), "BOSS_KOUKUSAYDON_G1_KOUKU")))
	{
		OutputDebugStringA(
			("[Loader][KoukuSaydonPresentation] " + CKoukuSaydonPresentationAssetService::Get_Status() + "\n").c_str());
		return E_FAIL;
	}

	// Server가 이 레이드에서도 gauge를 허용하므로 발탄과 같은 Esther 소환 원형을 아레나 진입 전에 준비한다.
	Set_Status(TEXT("KoukuSaydon: esther summon presentation"));
	Ready_EstherSummonPresentation(ETOUI(LEVEL::KOUKUSAYDON_ARENA));
	Set_Status(TEXT("KoukuSaydon: deploy environment prototypes"));
	if (FAILED(Ready_DeployPropArea(ETOUI(LEVEL::KOUKUSAYDON_ARENA), pEntry->pMapAreaId)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("KoukuSaydon: world sequence document"));
	std::string worldSequenceStatus;
	if (!CWorldSequencePlayer::Prepare_AreaLoad(ETOUI(LEVEL::KOUKUSAYDON_ARENA), pEntry->pMapAreaId,
												pEntry->MapLoadScope, worldSequenceStatus, [this]()
												{ return m_isCancellationRequested.load(std::memory_order_acquire); }))
	{
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		// Level의 선택적 연출 실패 경계를 유지한다. 활성화는 이 준비 결과를 소비하며 게임 프레임에서 다시 파싱하지 않는다.
		OutputDebugStringA(("[Loader][WorldSequence] " + worldSequenceStatus + "\n").c_str());
	}

	Set_Status(TEXT("KoukuSaydon arena loading complete"));
	rollback.Commit();
	return S_OK;
}

// Debug Map Editor 요청이면 저작 핵심 원형만 준비하고, 그 외에는 Server 수련장 맵·승인 직업·미리보기를 준비한다.
HRESULT CLoader::Ready_For_Development()
{
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::DEVELOPMENT));

#ifdef _DEBUG
	if (CMapEditorWorkspaceService::Is_Requested())
	{
		CNpcPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::DEVELOPMENT));
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

	CValtanPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::DEVELOPMENT));
	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::DEVELOPMENT));
	if (FAILED(CNpcPlacementPresentationService::Load(ETOUI(LEVEL::DEVELOPMENT), "TRAINING_GROUND")))
	{
		OutputDebugStringA(
			("[Loader][NpcPresentation] " + CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}

	const CLIENT_LEVEL_DESCRIPTOR *pEntry = CLevelRegistry::Find(LEVEL::DEVELOPMENT);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId)
		return E_INVALIDARG;

	Declare_Phases(6u);
	Set_Status(TEXT("TEST: training map"));
	if (FAILED(Ready_MapArea(ETOUI(LEVEL::DEVELOPMENT), pEntry->pMapAreaId, pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	const std::array<LostArk::Shared::CHARACTER_CLASS_ID, 1> selectedClass = {m_ePreparedCharacterClass};
	Set_Status(TEXT("TEST: server-approved character rendering"));
	if (FAILED(Ready_Character_Rendering(ETOUI(LEVEL::DEVELOPMENT), selectedClass)))
	{
		return E_FAIL;
	}
	if (FAILED(Ready_AnimationPreviewModels(ETOUI(LEVEL::DEVELOPMENT))))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("Development scenario loading complete"));
	rollback.Commit();
	return S_OK;
}

// 섬 맵과 승인 직업·AI 직업을 준비한 뒤 물총·의상·NPC 본·필수 clip을 검증한다.
// 필수 준비 실패는 섬 자원을 rollback하며 선택적 world sequence 오류는 로그를 남긴다.
HRESULT CLoader::Ready_For_Maharaka()
{
	CNpcPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::MAHARAKA));
	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::MAHARAKA));
	if (FAILED(CNpcPlacementPresentationService::Load(ETOUI(LEVEL::MAHARAKA), "MAHARAKA")))
	{
		OutputDebugStringA(
			("[Loader][NpcPresentation] " + CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::MAHARAKA));
	Declare_Phases(9u);
	Set_Status(TEXT("MAHARAKA: island catalog and placements"));

	const CLIENT_LEVEL_DESCRIPTOR *pEntry = CLevelRegistry::Find(LEVEL::MAHARAKA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(ETOUI(LEVEL::MAHARAKA), pEntry->pMapAreaId, pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("MAHARAKA: session character bundle"));
	std::vector<LostArk::Shared::CHARACTER_CLASS_ID> selectedClass = {m_ePreparedCharacterClass};
	for (const auto aiClass :
		 {LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT, LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER})
		if (std::find(selectedClass.begin(), selectedClass.end(), aiClass) == selectedClass.end())
			selectedClass.push_back(aiClass);
	if (FAILED(Ready_Character_Rendering(ETOUI(LEVEL::MAHARAKA), selectedClass)))
		return E_FAIL;

	const auto cancelled = [this]() { return m_isCancellationRequested.load(std::memory_order_acquire); };

	const auto level = ETOUI(LEVEL::MAHARAKA);

	std::string preloadStatus;

	Set_Status(TEXT("MAHARAKA: Waterpang weapons and contestant costumes"));

	if (cancelled())
    {
        return HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }
		
	if (FAILED(CMaharakaWaterpangPresentation::Ensure_WaterGunPrototype(m_pDevice, m_pContext, level, preloadStatus)))
    {
        return E_FAIL;
    }
		
	CEquipmentPresentationCatalog equipmentCatalog;
	CEquipmentPresentationService equipment(m_pDevice, m_pContext);
	if (!equipmentCatalog.Load(preloadStatus))
		return E_FAIL;
	for (const auto &set : m_MaharakaAvatarVisualSetIds)
	{
		if (cancelled())
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		if (!equipment.Preload_VisualSets(level, equipmentCatalog, std::span<const std::string>(&set, 1u),
										  preloadStatus))
		{
			OutputDebugStringA(("[Loader][Waterpang] " + preloadStatus + "\n").c_str());
			return E_FAIL;
		}
	}
	Set_Status(TEXT("MAHARAKA: Waterpang NPC models and materials"));
	for (const char *archetype : LostArk::Shared::MAHARAKA_WATERPANG_AI_NPCS)
	{
		if (cancelled())
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		if (FAILED(CNpcPresentationAssetService::Ensure_Prototypes(m_pDevice, m_pContext, level, archetype)))
			return E_FAIL;
		const auto model = std::dynamic_pointer_cast<CModel>(CGameInstance::Get().Clone_Prototype(
			level, CNpcPresentationAssetService::Get_ModelPrototypeTag(archetype)));
		if (!model || !model->Has_Bone("bip001-r-hand"))
			return E_FAIL;
	}
	// 준비한 모든 참가 직업이 실제 물총 idle·run clip을 갖췄는지 입장 전에 확인한다.
	for (const auto characterClass : selectedClass)
	{
		const auto *spec = CCharacterCatalog::Find_Spec(characterClass);

		const auto model =
			spec ? std::dynamic_pointer_cast<CModel>(CGameInstance::Get().Clone_Prototype(level, spec->pBodyModelTag))
				 : nullptr;

		bool idle = false, run = false;

		if (model)
        {
			for (uint32_t clip = 0u; clip < model->Get_NumAnimations(); ++clip)
			{
                if (const auto* name = model->Get_AnimationName(clip))
                {
                    idle |= std::string_view(name) == "watergun_idle";
                    run |= std::string_view(name) == "watergun_run";
                }
			}
		}

		if (!model || !model->Has_Bone("bip001-prop3") || !idle || !run)
        {
            return E_FAIL;
        }
	}

	if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::MAHARAKA), 
		CWorldSequenceObject::PROTOTYPE_TAG, CWorldSequenceObject::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

	std::string waterpangStatus;

	if (!CWorldSequencePlayer::Prepare_AreaLoad(ETOUI(LEVEL::MAHARAKA), pEntry->pMapAreaId, pEntry->MapLoadScope,
												waterpangStatus, [this]()
												{ return m_isCancellationRequested.load(std::memory_order_acquire); }))
	{
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		OutputDebugStringA(("[Loader][Waterpang] " + waterpangStatus + "\n").c_str());
	}
	Set_Status(TEXT("Maharaka loading complete"));
	rollback.Commit();
	return S_OK;
}

// 콜로세움 맵과 로컬 승인 직업을 준비한다. NPC binding 누락은 기록하고 필수 자원 성공 뒤 commit한다.
HRESULT CLoader::Ready_For_Colosseum()
{
	CNpcPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::COLOSSEUM));
	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::COLOSSEUM));
	if (FAILED(CNpcPlacementPresentationService::Load(ETOUI(LEVEL::COLOSSEUM), "COLOSSEUM")))
	{
		OutputDebugStringA(
			("[Loader][NpcPresentation] " + CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::COLOSSEUM));
	Declare_Phases(5u);
	Set_Status(TEXT("COLOSSEUM: arena catalog and placements"));

	const CLIENT_LEVEL_DESCRIPTOR *pEntry = CLevelRegistry::Find(LEVEL::COLOSSEUM);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(ETOUI(LEVEL::COLOSSEUM), pEntry->pMapAreaId, pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("COLOSSEUM: session character bundle"));
	const std::array<LostArk::Shared::CHARACTER_CLASS_ID, 1> selectedClass = {m_ePreparedCharacterClass};
	if (FAILED(Ready_Character_Rendering(ETOUI(LEVEL::COLOSSEUM), selectedClass)))
		return E_FAIL;

	Set_Status(TEXT("Colosseum loading complete"));
	rollback.Commit();
	return S_OK;
}

// Level별 준비가 같은 registry scope로 필요한 배치·모델만 선택한다. 모델을 geometry 그룹별 임시 슬롯에 준비하고 navigation 검증 뒤 등록한다.
// 중간 실패는 임시 모델을 소멸시키며, 이미 추가한 공용 원형의 정리는 상위 Level rollback 범위가 담당한다.
HRESULT CLoader::Ready_MapArea(const uint32_t iLevelIndex, const std::string &areaId, const MAP_LOAD_SCOPE &loadScope,
							   const bool_t prepareMapCore)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || areaId.empty())
		return E_INVALIDARG;

	// 선택적 준비가 실패해도 이전 방문의 배치 stage가 다시 소비되지 않도록 먼저 폐기한다.
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
				if (!CMapPlacementRuntime::Read_Placements(mapCatalog, scopedPlacements, placementStatus))
				{
					{
						lock_guard<mutex> activeLock(g_ActiveStatusMutex);
						g_ActiveStatus = "Map " + areaId + ": product load scope: " + placementStatus;
					}
					OutputDebugStringA(("[Loader][Map] " + placementStatus + "\n").c_str());
					return E_FAIL;
				}
				CMapPlacementRuntime::Apply_LoadScope(mapCatalog, loadScope, scopedPlacements);
				for (const MAP_PLACEMENT_RECORD &record : scopedPlacements)
					requiredAssetIds.insert(record.assetId);
				if (requiredAssetIds.empty())
				{
					lock_guard<mutex> activeLock(g_ActiveStatusMutex);
					g_ActiveStatus = "Map " + areaId + ": product load scope selected no placements";
					return E_FAIL;
				}
			}
		}

		const size_t requiredModelCount =
			loadScope.isEnabled ? requiredAssetIds.size() : mapCatalog.Get_Entries().size();
		std::vector<const MAP_ASSET_ENTRY *> entries;
		std::vector<std::vector<size_t>> groups;
		std::unordered_map<std::wstring, size_t> geometryGroups;
		entries.reserve(requiredModelCount);
		// catalog 순서를 유지해 순차 로드와 같은 물리 모델·재질·LOD 입력을 선택한다.
		// 병렬 준비는 서로 독립적인 geometry 그룹 사이에서만 수행한다.
		for (const MAP_ASSET_ENTRY &entry : mapCatalog.Get_Entries())
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

		// worker 시작 전에 모든 결과 슬롯을 확보한다. 서로 다른 geometry 그룹은 서로 다른 슬롯만 수정한다.
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
			const auto preparation = m_AssetPreparationBatch.Run(
				m_pDevice.Get(), groups.size(), &m_isCancellationRequested,
				[&](const size_t groupIndex) -> HRESULT
				{
					size_t activeIndex = (std::numeric_limits<size_t>::max)();
					try
					{
						Engine::CProfilerScope workerScope(CGameInstance::Get().Get_Profiler(),
														   "Loader.Map.ModelWorker");
						const matrix_t transform = XMMatrixScaling(0.01f, 0.01f, 0.01f);
						const CModel *geometry = nullptr;
						for (const size_t index : groups[groupIndex])
						{
							if (failed.load(std::memory_order_acquire))
								return S_OK; // 실제 오류는 실패한 그룹의 결과가 보존한다.
							if (m_isCancellationRequested.load(std::memory_order_acquire))
								return HRESULT_FROM_WIN32(ERROR_CANCELLED);
							activeIndex = index;
							const MAP_ASSET_ENTRY &entry = *entries[index];
							Engine::MODEL_ASSET_LOAD_DESC loadDesc;
							loadDesc.assetRoot = assetRoot;
							loadDesc.meshPath = entry.resolvedModelPath;
							loadDesc.materialOverrides = entry.materialOverrides;
							auto model =
								geometry ? CModel::Create_MaterialVariant(*geometry, loadDesc)
										 : CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, loadDesc, transform);
							if (!model)
							{
								failedModels[groupIndex] = index;
								failed.store(true, std::memory_order_release);
								return E_FAIL;
							}
							if (!geometry)
								geometry = model.get();
							// 그룹의 첫 임시 모델이 geometry 수명을 유지한다.
							// worker는 컨테이너 크기나 다른 그룹의 catalog 슬롯을 변경하지 않는다.
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
						throw; // 공용 worker 예외 경계가 E_OUTOFMEMORY와 E_FAIL로 구분한다.
					}
				});
			HRESULT result = preparation.result;
			if (SUCCEEDED(result) && m_isCancellationRequested.load(std::memory_order_acquire))
				result = HRESULT_FROM_WIN32(ERROR_CANCELLED);
			if (SUCCEEDED(result) && completed != requiredModelCount)
				result = E_FAIL;
			const double elapsedMs =
				std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
			char diagnostic[512]{};
			sprintf_s(
				diagnostic,
				"[Loader][Map] area=%s models=%zu geometryGroups=%zu variants=%zu workers=%u prepared=%zu elapsedMs=%.3f result=0x%08lX\n",
				areaId.c_str(), requiredModelCount, groups.size(), requiredModelCount - groups.size(),
				preparation.workerCount, completed, elapsedMs, static_cast<unsigned long>(result));
			OutputDebugStringA(diagnostic);
			if (FAILED(result))
			{
				const size_t failedIndex = preparation.failedTask < failedModels.size()
											   ? failedModels[preparation.failedTask]
											   : (std::numeric_limits<size_t>::max)();
				if (failedIndex < entries.size())
				{
					const MAP_ASSET_ENTRY &entry = *entries[failedIndex];
					{
						lock_guard<mutex> activeLock(g_ActiveStatusMutex);
						g_ActiveStatus = "Map " + areaId + ": model prototype failed " + entry.id + " / " +
										 entry.modelRelativePath.generic_string() + " (" + std::to_string(completed) +
										 "/" + std::to_string(requiredModelCount) + ")";
					}
					OutputDebugStringW((L"[Loader][Map] Model failed: " + entry.prototypeTag + L" / " +
										entry.resolvedModelPath.wstring() + L"\n")
										   .c_str());
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
				if (!CMapNavigationContract::Read_RuntimeStepHeight(navigationContract, maximumStepHeight,
																	navigationStatus))
				{
					OutputDebugStringA(("[Loader][Map] " + navigationStatus + "\n").c_str());
					return E_FAIL;
				}
				auto navigation = CNavigation::Create_NavGrid(
					m_pDevice, m_pContext, navigationContract.runtimePath.c_str(), maximumStepHeight);
				if (!navigation)
					return E_FAIL;
				// Server는 상세 영역의 region grid로 판정하므로 로컬 플레이어의 예측 높이·경로도 같은 grid를 사용해야 한다.
				std::vector<MAP_NAVIGATION_REGION> regions;
				if (!CMapNavigationContract::Read_RuntimeRegionManifest(areaId, regions, navigationStatus))
				{
					OutputDebugStringA(("[Loader][Map] " + navigationStatus + "\n").c_str());
					return E_FAIL;
				}
				for (const MAP_NAVIGATION_REGION &region : regions)
				{
					MAP_NAVIGATION_CONTRACT regionContract;
					f32_t regionStepHeight = 0.f;
					if (!CMapNavigationContract::Resolve_Region(areaId, region.regionId, regionContract,
																navigationStatus) ||
						!regionContract.runtimeGridAvailable ||
						!CMapNavigationContract::Read_RuntimeStepHeight(regionContract, regionStepHeight,
																		navigationStatus) ||
						std::abs(regionStepHeight - region.stepHeight) > 0.000001f)
					{
						OutputDebugStringA(("[Loader][Map] Navigation region rejected: " + region.regionId + " (" +
											navigationStatus + ")\n")
											   .c_str());
						return E_FAIL;
					}
					auto regionNavigation = CNavigation::Create_NavGrid(
						m_pDevice, m_pContext, regionContract.runtimePath.c_str(), regionStepHeight);
					if (!regionNavigation || FAILED(navigation->Attach_Region(std::move(regionNavigation))))
					{
						return E_FAIL;
					}
				}
				staged.emplace_back(navigationContract.prototypeTag, std::move(navigation));
			}
		}
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);

		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.Map.Commit");
			if (!staged.empty() && FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
				return E_FAIL;
			// 모델·navigation 준비에 실패하면 배치 stage를 게시하지 않는다.
			if (loadScope.isEnabled)
				CMapPlacementRuntime::Cache_LoadStage(areaId, loadScope, mapCatalog, scopedPlacements);
		}
		return S_OK;
	}
	catch (const std::bad_alloc &)
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

// 지도 표시·미리보기·chunk·instance·카메라와 배치 객체 원형을 대상 Level에 순서대로 등록한다.
// 이 함수 내부의 등록 일부가 성공한 뒤 실패할 수 있으므로 호출자의 Level rollback 범위 안에서 사용한다.
HRESULT CLoader::Ready_MapAuthoringCore(const uint32_t iLevelIndex)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || FAILED(Ready_StaticMeshShader(iLevelIndex)) ||
		FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, CMapAssetPreview::SHADER_PROTOTYPE_TAG,
												  CShader::Create(m_pDevice, m_pContext,
																  TEXT("../Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl"),
																  VTXMESH::Elements, VTXMESH::iNumElements))))
	{
		return E_FAIL;
	}

	std::array<D3D11_INPUT_ELEMENT_DESC, VTXMESH::iNumElements + 1u> chunkElements{};
	std::copy(std::begin(VTXMESH::Elements), std::end(VTXMESH::Elements), chunkElements.begin());
	chunkElements.back() = {"SOURCEINDEX", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0};
	if (FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex, CMapStaticChunkObject::ShaderTag,
			CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMeshMapChunk.hlsl"),
							chunkElements.data(), static_cast<uint32_t>(chunkElements.size())))) ||
		FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, CMapStaticChunkObject::PrototypeTag,
												  CMapStaticChunkObject::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	Set_Status(TEXT("Map: editor core prototypes"));
	if (FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex, TEXT("Prototype_Component_Shader_VtxMeshMapInstance"),
			CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl"),
							VTXMESHINSTANCE::Elements, VTXMESHINSTANCE::iNumElements))) ||
		FAILED(Ready_Camera_Prototype(iLevelIndex)) ||
		FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, TEXT("Prototype_GameObject_MapAsset"),
												  CMapAssetObject::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, TEXT("Prototype_GameObject_MapStaticBatch"),
												  CMapStaticBatchObject::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

#ifdef _DEBUG
	// Map Tool이 저작할 수 있는 Level은 trigger·collision·spawn anchor 박스 원형을 자기 Level index에 등록한다.
	if ((iLevelIndex == ETOUI(LEVEL::DEVELOPMENT) || iLevelIndex == ETOUI(LEVEL::MAHARAKA) ||
		 iLevelIndex == ETOUI(LEVEL::COLOSSEUM) || iLevelIndex == ETOUI(LEVEL::CHARACTER_SELECT) ||
		 iLevelIndex == ETOUI(LEVEL::BERN) || iLevelIndex == ETOUI(LEVEL::VALTAN_ARENA) ||
		 iLevelIndex == ETOUI(LEVEL::KOUKUSAYDON_ARENA)) &&
		FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, TEXT("Prototype_GameObject_TriggerBox"),
												  CTrigger_Box::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}
#endif

	return S_OK;
}

// 지도 준비가 사용할 자유 카메라 원형을 대상 Level에 등록한다. 인스턴스 생성과 카메라 선택은 Level이 담당한다.
HRESULT CLoader::Ready_Camera_Prototype(const uint32_t iLevelIndex)
{
	return CGameInstance::Get().Add_Prototype(iLevelIndex, TEXT("Prototype_GameObject_Camera_Free"),
											  CCamera_Free::Create(m_pDevice, m_pContext));
}

// 정적 메시 셰이더 생성과 원형 등록 실패를 구분해 진단한다. 생성한 소유권은 등록 호출로 이동한다.
HRESULT CLoader::Ready_StaticMeshShader(const uint32_t iLevelIndex)
{
	auto shader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
								  VTXMESH::Elements, VTXMESH::iNumElements);
	if (!shader)
	{
		Set_Status(TEXT("Map: Shader_VtxMeshBinary creation failed (compiled shader or program variants)"));
		OutputDebugStringA("[Loader][Shader] Shader_VtxMeshBinary creation failed before prototype registration.\n");
		return E_FAIL;
	}
	const HRESULT result = CGameInstance::Get().Add_Prototype(
		iLevelIndex, TEXT("Prototype_Component_Shader_VtxMeshBinary"), std::move(shader));
	if (FAILED(result))
	{
		Set_Status(TEXT("Map: Shader_VtxMeshBinary prototype registration failed"));
		OutputDebugStringA("[Loader][Shader] Shader_VtxMeshBinary prototype registration failed.\n");
	}
	return result;
}

// 캐릭터와 Esther NPC가 사용할 애니메이션 셰이더 원형을 등록한다. 후속 실패 시 정리는 상위 Level 범위가 담당한다.
HRESULT CLoader::Ready_AnimatedMeshShader(const uint32_t iLevelIndex)
{
	if (FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex, TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
			CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
							VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
	{
		return E_FAIL;
	}
	// Esther 소환 NPC와 화면 cutin은 별도 deferred 재질 셰이더 원형을 사용하여 공용 셰이더 pass 변경과 분리한다.
	return CGameInstance::Get().Add_Prototype(iLevelIndex, TEXT("Prototype_Component_Shader_VtxEstherNpc"),
											  CShader::Create(m_pDevice, m_pContext,
															  TEXT("../Bin/ShaderFiles/Shader_VtxEstherNpc.hlsl"),
															  VTXANIMMESH::Elements, VTXANIMMESH::iNumElements));
}

// Map Editor 준비가 공용 Deploy 객체 원형을 요구한다. 잘못된 Level과 준비 실패를 HRESULT로 전달한다.
HRESULT CLoader::Ready_DeployPropCore(const uint32_t iLevelIndex)
{
	if (iLevelIndex >= ETOUI(LEVEL::END))
		return E_INVALIDARG;
	std::string status;
	if (!CDeployPropRuntime::Ensure_ObjectPrototype(m_pDevice, m_pContext, iLevelIndex, status))
	{
		OutputDebugStringA(("[Loader][DeployProp] " + status + "\n").c_str());
		return E_FAIL;
	}
	return S_OK;
}

// Deploy의 실제 준비는 CDeployPropRuntime이 소유한다.
// Loader에서 Deploy를 제외하는 Area도 자기 Level에서 같은 준비 경로를 사용하도록 구현을 공유한다.
// Area별 Deploy 준비를 공용 runtime에 위임하고 취소 여부를 callback으로 전달한다. 실패 시 취소와 일반 오류를 구분한다.
HRESULT CLoader::Ready_DeployPropArea(const uint32_t iLevelIndex, const std::string &areaId)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || areaId.empty())
		return E_INVALIDARG;

	std::string status;
	if (!CDeployPropRuntime::Ensure_AreaPrototypes(
			m_pDevice, m_pContext, iLevelIndex, areaId, status,
			[this]() -> bool_t { return m_isCancellationRequested.load(std::memory_order_acquire); }))
	{
		OutputDebugStringA(("[Loader][DeployProp] " + status + "\n").c_str());
		return m_isCancellationRequested.load(std::memory_order_acquire) ? HRESULT_FROM_WIN32(ERROR_CANCELLED) : E_FAIL;
	}
	return S_OK;
}
// 직업 입력을 먼저 검증한 뒤 공용 원형과 직업별 모델을 준비한다.
// Initialize가 고정한 snapshot만 사용하며 callback의 완료량은 Loading 진행 표시로 연결한다.
HRESULT CLoader::Ready_Character_Rendering(const uint32_t iLevelIndex,
										   const std::span<const LostArk::Shared::CHARACTER_CLASS_ID> characterClasses)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || characterClasses.empty())
	{
		return E_INVALIDARG;
	}
	for (const auto characterClass : characterClasses)
	{
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(characterClass))
		{
			return E_INVALIDARG;
		}
	}

	CPlayableCharacterAssetService::Begin_LevelLoad(iLevelIndex);

	if (FAILED(Ready_AnimatedMeshShader(iLevelIndex)) || FAILED(Ready_Character_Shared_Prototypes(iLevelIndex)))
	{
		return E_FAIL;
	}

	for (size_t classIndex = 0; classIndex < characterClasses.size(); ++classIndex)
	{
		const auto characterClass = characterClasses[classIndex];
		const auto authoring = m_CharacterAuthoringInputs.find(characterClass);
		if (authoring == m_CharacterAuthoringInputs.end() || !authoring->second)
			return E_INVALIDARG;
		const auto progress = [this, classIndex, classCount = characterClasses.size(), characterClass](
								  const size_t completedModelCount, const size_t totalModelCount, const std::string &)
		{
			tchar_t status[MAX_PATH]{};
			_snwprintf_s(status, std::size(status), _TRUNCATE, TEXT("Character %zu/%zu | %s models"), classIndex + 1u,
						 classCount, Get_CharacterClassName(characterClass));
			Set_DeterminateStatus(status, completedModelCount, totalModelCount);
		};
		if (FAILED(CPlayableCharacterAssetService::Ensure_Prototypes(m_pDevice, m_pContext, iLevelIndex, characterClass,
																	 &m_isCancellationRequested, progress,
																	 authoring->second, &m_AssetPreparationBatch)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

// 몸체·장비·탈것 파트와 캐릭터 객체, 플레이어·월드 collider 원형을 등록한다. 직업별 모델 자료는 포함하지 않는다.
HRESULT CLoader::Ready_Character_Shared_Prototypes(const uint32_t iLevelIndex)
{
	if (FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, TEXT("Prototype_GameObject_Part_Equipment"),
												  CPart_Equipment::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, TEXT("Prototype_GameObject_Part_Body"),
												  CPart_Body::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, TEXT("Prototype_GameObject_Part_Vehicle"),
												  CPart_Vehicle::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, TEXT("Prototype_GameObject_Character"),
												  CCharacter::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, TEXT("Prototype_Component_Collider_Player"),
												  CCollider::Create(m_pDevice, m_pContext, COLLIDER::OBB))) ||
		FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, TEXT("Prototype_Component_Collider_WorldEntity"),
												  CCollider::Create(m_pDevice, m_pContext, COLLIDER::SPHERE))))
	{
		return E_FAIL;
	}

	return S_OK;
}

// Debug 저작 미리보기의 선택적 비플레이 모델과 donor·무기를 준비한다. Release에서는 준비 없이 성공한다.
// 누락 모델 일부는 건너뛰지만 잘못된 선언·donor 불일치·등록 실패는 상위 rollback으로 전달한다.
HRESULT CLoader::Ready_AnimationPreviewModels(const uint32_t iLevelIndex)
{
#ifdef _DEBUG
	if (iLevelIndex != ETOUI(LEVEL::CHARACTER_SELECT) && iLevelIndex != ETOUI(LEVEL::DEVELOPMENT))
	{
		return E_INVALIDARG;
	}

	constexpr std::array<const wchar_t*, 7> playablePrototypeTags = {
		TEXT("Prototype_Component_Model_LanceMaster"),	   TEXT("Prototype_Component_Model_GunSlinger"),
		TEXT("Prototype_Component_Model_Slayer"),		   TEXT("Prototype_Component_Model_Artist"),
		TEXT("Prototype_Component_Model_DimensionMaster"), TEXT("Prototype_Component_Model_Warlord"),
		TEXT("Prototype_Component_Model_GuardianKnight")};

	for (const ANIMATION_PREVIEW_ASSET &asset : ANIMATION_PREVIEW_ASSETS)
	{
		bool_t bPlayablePrototype = false;
		for (const wchar_t *pPlayableTag : playablePrototypeTags)
		{
			if (0 == wcscmp(asset.pPrototypeTag, pPlayableTag))
			{
				bPlayablePrototype = true;
				break;
			}
		}
		// 플레이 가능한 직업은 Character Select의 전용 준비 경로를 사용한다.
		// 이 직접 로드는 플레이 불가 배우의 선택적 저작 미리보기만 담당한다.
		if (bPlayablePrototype)
		{
			continue;
		}
		if (nullptr == asset.pModelAssetId || nullptr == asset.pPrototypeTag || !std::isfinite(asset.fPreviewScale) ||
			asset.fPreviewScale <= 0.f || !std::isfinite(asset.fPreviewYawDegrees))
		{
			return E_FAIL;
		}
		const BOSS_ACTOR_ENTRY *pBoss = nullptr;
		if (nullptr != asset.pBossArchetypeId)
		{
			pBoss = CActorCatalog::Find_Boss(asset.pBossArchetypeId);
			if (nullptr == pBoss || pBoss->bodyModel != asset.pModelAssetId)
				return E_FAIL;

			// 제품 발탄은 Model View의 선택적 콘텐츠다.
			// Character Select 진입마다 준비하지 않고 선택 또는 실제 Server spawn 전에 전용 경로로 준비한다.
			continue;
		}
		const filesystem::path path = CRuntimeAssetRoot::Resolve(asset.pModelAssetId);
		if (path.empty() || !filesystem::is_regular_file(path))
			continue;
		const matrix_t previewTransform =
			XMMatrixScaling(asset.fPreviewScale, asset.fPreviewScale, asset.fPreviewScale) *
			XMMatrixRotationY(XMConvertToRadians(asset.fPreviewYawDegrees));

		unique_ptr<CModel> model =
			CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, path.string().c_str(), previewTransform);
		if (nullptr == model)
			continue;
		// 미리보기 몸체도 자기 animation donor를 선언할 수 있다.
		// 제품 보스와 같은 Attach_AnimationSet 계약으로 다른 rig donor를 거부해 잘못된 골격 표시를 막는다.
		if (nullptr != asset.pAnimationSetAssetId)
		{
			const filesystem::path donorPath = CRuntimeAssetRoot::Resolve(asset.pAnimationSetAssetId);
			if (donorPath.empty() || !filesystem::is_regular_file(donorPath))
				return E_FAIL;
			const unique_ptr<CModel> donor =
				CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, donorPath.string().c_str(), previewTransform);
			if (nullptr == donor || FAILED(model->Attach_AnimationSet(*donor)))
			{
				return E_FAIL;
			}
		}
		if (nullptr != pBoss)
		{
			const filesystem::path animSetPath = CRuntimeAssetRoot::Resolve(pBoss->animationSetId);
			if (!animSetPath.empty() && filesystem::is_regular_file(animSetPath))
			{
				const unique_ptr<CModel> animSet =
					CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, animSetPath.string().c_str(), previewTransform);
				if (nullptr == animSet || FAILED(model->Attach_AnimationSet(*animSet)))
				{
					return E_FAIL;
				}
			}
		}
		if (FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, asset.pPrototypeTag, move(model))))
		{
			return E_FAIL;
		}

		// 선택적 본 부착 무기는 몸체와 함께 준비해 Development와 Character Select에서 같은 구성을 사용한다.
		// 본 행렬이 미리보기 scale·yaw를 이미 가지므로 정적 무기에는 몸체에 대한 단위 배율만 적용한다.
		const bool_t bDeclaresWeapon = nullptr != asset.pWeaponModelAssetId && nullptr != asset.pWeaponPrototypeTag &&
									   nullptr != asset.pWeaponSocketBone;
		if (!bDeclaresWeapon)
			continue;
		if (!std::isfinite(asset.fWeaponScale) || asset.fWeaponScale <= 0.f)
			return E_FAIL;
		const filesystem::path weaponPath = CRuntimeAssetRoot::Resolve(asset.pWeaponModelAssetId);
		if (weaponPath.empty() || !filesystem::is_regular_file(weaponPath))
			continue;
		unique_ptr<CModel> weaponModel =
			CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, weaponPath.string().c_str(),
						   XMMatrixScaling(asset.fWeaponScale, asset.fWeaponScale, asset.fWeaponScale));
		if (nullptr == weaponModel ||
			FAILED(CGameInstance::Get().Add_Prototype(iLevelIndex, asset.pWeaponPrototypeTag, move(weaponModel))))
		{
			return E_FAIL;
		}
	}
#endif
	return S_OK;
}

// 발탄의 본체와 유령 표현을 같은 Loading 단계에서 준비한다. 어느 쪽이든 필수 준비 실패를 호출자에게 전달한다.
HRESULT CLoader::Ready_ValtanPresentation(const uint32_t iLevelIndex)
{
	if (FAILED(CValtanPresentationAssetService::Ensure_Prototypes(m_pDevice, m_pContext, iLevelIndex, "BOSS_VALTAN")))
	{
		return E_FAIL;
	}
	// 후반 연출이 반복해서 사용하는 유령 몸체·animation-set은 아레나 Loading에서 미리 준비한다.
	// 의존 보스가 처음 화면에 나타나는 시점에 원형을 준비하지 않는다.
	return CValtanPresentationAssetService::Ensure_Prototypes(m_pDevice, m_pContext, iLevelIndex, "BOSS_VALTAN_GHOST");
}

// 캐릭터 선택과 레이드가 공유하는 Esther 모델·cutin·효과음을 미리 준비한다. 누락은 해당 표현만 격리하고 입장을 막지 않는다.
void CLoader::Ready_EstherSummonPresentation(const uint32_t iLevelIndex)
{
	for (const char *pEstherArchetypeId : {"NPC_59030", "NPC_58700", "NPC_59060", "NPC_59504", "NPC_59620"})
	{
		if (FAILED(CNpcPresentationAssetService::Ensure_Prototypes(m_pDevice, m_pContext, iLevelIndex,
																   pEstherArchetypeId)))
		{
			OutputDebugStringA((std::string("[Loader][NpcPresentation] esther summon "
											"presentation is unavailable (") +
								pEstherArchetypeId + "); the arena loads without it.\n")
								   .c_str());
		}
		(void)CEstherCutinPresentationService::Preload_Frames(m_pDevice.Get(), pEstherArchetypeId);
	}
	(void)CEstherActionSoundCueDocument::Preload_Sounds();
}

// CLevel_Loading에 초기화된 Loader의 단독 소유권을 넘긴다. 실패하면 지역 unique_ptr이 Free를 호출해 worker를 정리한다.
unique_ptr<CLoader> CLoader::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
									const LEVEL eNextLevelID, const uint64_t iEffectLoadJobEpoch,
									const uint64_t iEffectCatalogRevision)
{
	auto pInstance = unique_ptr<CLoader>(new CLoader(pDevice, pContext));
	if (FAILED(pInstance->Initialize(eNextLevelID, iEffectLoadJobEpoch, iEffectCatalogRevision)))
		return nullptr;
	return pInstance;
}

// 취소 요청 후 두 producer를 최대 5초 기다리고, 동기 I/O 취소 후 다시 5초 기다린다.
// 종료를 확인하기 전에 this·장치·callback 입력을 해제하지 않으며 대기 실패나 시간 초과는 프로세스를 종료한다.
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
			OutputDebugStringA("[Loader] Worker wait failed; terminating the process to preserve loader invariants.\n");
			if (!TerminateProcess(GetCurrentProcess(), ERROR_SUCCESS == errorCode ? ERROR_INVALID_HANDLE : errorCode))
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
