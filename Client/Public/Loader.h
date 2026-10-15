#pragma once

// 로드할 LEVEL과 Client 공통 타입을 사용한다.
#include "Client_Defines.h"
// 그래픽 장치·스레드 HANDLE·HRESULT 등 Engine과 Windows 공통 타입을 사용한다.
#include "Engine_Defines.h"
// 레벨 registry와 같은 지도 준비 범위를 전달한다.
#include "MapLoadScope.h"
// Server가 승인한 CHARACTER_CLASS_ID를 캐릭터 준비 입력으로 보관한다.
#include "Network/PacketType.h"
// worker가 읽을 AUTHORING_INPUT snapshot 타입을 보관한다.
#include "PlayableCharacterAssetService.h"
// 모델 및 Effect 준비 batch를 직접 소유하고 종료 때 자식 I/O 취소를 전달한다.
#include "AssetPreparationBatch.h"

// worker의 완료 결과·상태·협력 취소를 다른 스레드에 공개한다.
#include <atomic>
// 단계별 경과 시간을 단조 시계로 측정한다.
#include <chrono>
// 준비 개수와 진행 index에 size_t를 사용한다.
#include <cstddef>
// 문자열과 여러 필드로 구성된 진행 snapshot을 한 번에 보호한다.
#include <mutex>
// 호출 동안 사용할 직업 ID 목록을 복사 없이 전달한다.
#include <span>
// UTF-8 진행 문구와 Area·외형 ID를 소유한다.
#include <string>
// 직업 ID로 읽기 전용 저작 snapshot을 찾는다.
#include <unordered_map>
// 마하라카 참가 외형의 stable visual-set ID를 순서대로 보관한다.
#include <vector>

NS_BEGIN(Client)
// registry만 private Ready_For 경로를 선택하도록 friend로 연결한다.

class CLevelRegistry;
// 공유 job 포인터만 보관하므로 mailbox 구현은 CPP에서 포함한다.
class CEffectLoadPreparationJob;

// CLevel_Loading이 단독 소유하며 대상 Level의 자원 준비와 worker 종료를 관리한다.
// 주 스레드는 입력 snapshot을 고정하고 진행을 조회하며, worker는 준비 결과를 공개한다.
// 실제 Level 활성화와 Effect 등록 commit은 Loading/MainApp 경계에서 수행한다.
class CLoader final
{
	friend class CLevelRegistry;

private:
	CLoader(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	~CLoader();

public:
	// Initialize가 RUNNING을 공개하고 Start_Loading이 최종 결과를 저장한 뒤 SUCCEEDED 또는 FAILED를 공개한다.
	enum class STATE : uint32_t
	{
		IDLE,
		RUNNING,
		SUCCEEDED,
		FAILED
	};
	// Loading 화면이 한 프레임에 읽는 진행 복사본이다. 수량·단계는 같은 mutex 아래 묶어서 복사한다.
	struct PROGRESS_SNAPSHOT final
	{
		std::string strStatus;
		bool_t bDeterminate = false;
		size_t iCompleted = 0u;
		size_t iTotal = 0u;
		uint64_t iElapsedMs = 0u;
		// Ready_For 경로가 선언한 전체 단계 수와 현재 단계다. 첫 상태 전에는 0, 이후에는 1부터 센다.
		// Loading 화면은 이 값으로 개별 단계 진행률을 전체 진행률에 합친다.
		size_t iPhaseIndex = 0u;
		size_t iPhaseCount = 0u;
	};

	// worker의 release 저장과 짝을 이루는 acquire 조회다. 성공 상태는 준비 함수 종료를 뜻하며 화면 활성화는 별도다.
	STATE Get_State() const
	{
		return m_eState.load(std::memory_order_acquire);
	}
	// Effect job 없이 일반 로드를 시작하는 진입점이다. 아래 overload에 0/0을 전달한다.
	HRESULT Initialize(LEVEL eNextLevelID);
	// Loading이 owner 스레드에서 입력을 고정하고 worker를 만든다. epoch와 revision은 둘 다 있거나 둘 다 없어야 한다.
	HRESULT Initialize(
		LEVEL eNextLevelID,
		uint64_t iEffectLoadJobEpoch,
		uint64_t iEffectCatalogRevision);
	// Level worker 진입점이 호출한다. registry 준비와 Effect 준비 결과를 합친 후 최종 상태를 공개한다.
	HRESULT Start_Loading();
	// Loading이 다음 준비·활성화 단계로 진행할 수 있는 worker 성공 여부를 확인한다.
	bool_t Finished() const
	{
		return STATE::SUCCEEDED == Get_State();
	}
	// Loading이 실패 진단과 복구 경로를 선택할 때 사용한다.
	bool_t Failed() const
	{
		return STATE::FAILED == Get_State();
	}
	// 최종 HRESULT를 조회한다. 작업 중에는 초기값 S_FALSE가 남을 수 있어 상태와 함께 사용한다.
	HRESULT Get_Result() const
	{
		return static_cast<HRESULT>(
			m_iResult.load(std::memory_order_acquire));
	}
	// 공용 오류 표시가 마지막 준비 단계의 문구를 복사해 받는다.
	static std::string Get_ActiveStatus();
	// 작업량·전체 단계·경과 ms를 일관된 복사본으로 반환한다. UI는 내부 버퍼를 차용하지 않는다.
	PROGRESS_SNAPSHOT Get_ProgressSnapshot() const;
	// Loading owner가 mailbox 명령·commit 결과를 교환할 job의 공유 소유권을 받는다.
	std::shared_ptr<CEffectLoadPreparationJob> Get_EffectLoadJob() const
	{
		return m_pEffectLoadJob;
	}

#ifdef _DEBUG
	// Debug의 창 제목에 현재 진행 문구를 표시한다. 자원 준비 상태를 변경하지 않는다.
	void Print_Text();
#endif

private:
	// 아래 Level별 준비는 registry만 선택한다. 필수 자원이 실패하면 해당 Level의 rollback 범위가 정리한다.
	// NPC·탈것·소환 표시처럼 선택적인 자료는 각 경로에서 로그를 남기고 계속할 수 있다.
	HRESULT Ready_For_Lobby();
	HRESULT Ready_For_CharacterSelect();
	HRESULT Ready_For_Bern();
	HRESULT Ready_For_ValtanArena();
	HRESULT Ready_For_KoukuSaydonArena();
	HRESULT Ready_For_Development();
	HRESULT Ready_For_Maharaka();
	HRESULT Ready_For_Colosseum();

	// Area 정의·같은 load scope의 배치·모델·navigation을 준비한 뒤 원형과 배치 stage를 공개한다.
	HRESULT Ready_MapArea(
		uint32_t iLevelIndex,
		const std::string& areaId,
		const MAP_LOAD_SCOPE& loadScope = {},
		bool_t prepareMapCore = true);
	// 지도 렌더링·카메라·배치 객체의 공용 원형을 등록한다. 추가 후 실패 정리는 상위 Level 범위가 담당한다.
	HRESULT Ready_MapAuthoringCore(uint32_t iLevelIndex);
	HRESULT Ready_Camera_Prototype(uint32_t iLevelIndex);
	HRESULT Ready_StaticMeshShader(uint32_t iLevelIndex);
	HRESULT Ready_AnimatedMeshShader(uint32_t iLevelIndex);
	HRESULT Ready_DeployPropCore(uint32_t iLevelIndex);
	HRESULT Ready_DeployPropArea(
		uint32_t iLevelIndex,
		const std::string& areaId);
	// 이미 고정한 직업별 AUTHORING_INPUT으로 캐릭터 원형을 준비한다. span과 snapshot은 호출 동안만 읽는다.
	HRESULT Ready_Character_Rendering(
		uint32_t iLevelIndex,
		std::span<const LostArk::Shared::CHARACTER_CLASS_ID> characterClasses);
	HRESULT Ready_Character_Shared_Prototypes(uint32_t iLevelIndex);
	HRESULT Ready_AnimationPreviewModels(uint32_t iLevelIndex);
	HRESULT Ready_ValtanPresentation(uint32_t iLevelIndex);
	void Ready_EstherSummonPresentation(uint32_t iLevelIndex);
	// Effect producer가 job 명령을 받아 준비하며, 실제 target registry commit은 Loading owner가 처리한다.
	HRESULT Run_EffectLoadPreparation();
	// 별도 Effect worker의 COM 수명과 예외·결과 공개 경계를 제공한다.
	static unsigned __stdcall EffectThreadMain(void* pArgument);
	// atomic 취소와 Effect mailbox 취소를 요청한다. 이 함수는 종료 대기나 HANDLE 정리를 하지 않는다.
	void Request_Cancellation() noexcept;
	// 새 단계의 상태 문구를 기록하고 단계 index를 증가시킨다. 같은 문구라도 호출마다 단계는 진행한다.
	void Set_Status(const tchar_t* pStatus);
	// 같은 단계의 완료량을 갱신한다. 전체 단계 index는 증가시키지 않는다.
	void Set_DeterminateStatus(
		const tchar_t* pStatus,
		size_t iCompleted,
		size_t iTotal);
	// Ready_For 경로가 기본 단계 수를 선언하고 추가 분기는 Add_Phases로 늘린다.
	// 각 Set_Status 호출은 현재 단계 index를 한 번 증가시킨다.
	void Declare_Phases(size_t iCount);
	void Add_Phases(size_t iCount);
	// Debug 창 제목용 호출자 버퍼로 복사한다. null 또는 크기 0이면 아무것도 쓰지 않는다.
	void Copy_Status(tchar_t* pOutput, size_t outputCount) const;

private:
	// Loader와 worker가 쓰는 장치 참조다. Free가 모든 worker를 마친 뒤 Loader와 함께 해제된다.
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	// Initialize에서 확정한 목적지와 직업·외형 snapshot이다. worker 시작 뒤에는 입력을 교체하지 않는다.
	LEVEL m_eNextLevelID = LEVEL::END;
    std::vector<std::string> m_MaharakaAvatarVisualSetIds;
	LostArk::Shared::CHARACTER_CLASS_ID m_ePreparedCharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID,
		std::shared_ptr<const CPlayableCharacterAssetService::AUTHORING_INPUT>> m_CharacterAuthoringInputs;
	// Loader가 두 producer HANDLE을 소유한다. 자식 batch는 각 batch가 소유하며 Free가 모두 종료될 때까지 기다린다.
	HANDLE m_hThread = {};
	HANDLE m_hEffectThread = {};
	CAssetPreparationBatch m_AssetPreparationBatch;
	CAssetPreparationBatch m_EffectPreparationBatch;
	// 상태 문자열·작업량·단계·시작 시각은 m_StatusMutex로 함께 보호한다.
	tchar_t m_szLoadingText[MAX_PATH] = {};
	mutable std::mutex m_StatusMutex;
	bool_t m_bProgressDeterminate = false;
	size_t m_iProgressCompleted = 0u;
	size_t m_iProgressTotal = 0u;
	size_t m_iPhaseIndex = 0u;
	size_t m_iPhaseCount = 0u;
	std::chrono::steady_clock::time_point m_ProgressPhaseStarted =
		std::chrono::steady_clock::now();
	// 단일 값의 스레드 간 공개는 atomic을 사용한다. 취소는 요청이며 종료 완료를 뜻하지 않는다.
	std::atomic<STATE> m_eState = STATE::IDLE;
	std::atomic<long> m_iResult = S_FALSE;
	std::atomic<long> m_iEffectResult = E_PENDING;
	std::atomic_bool m_isCancellationRequested = false;
	// Loading owner와 Effect worker가 공유하는 준비 mailbox다. 생성은 worker 시작 전에 끝낸다.
	std::shared_ptr<CEffectLoadPreparationJob> m_pEffectLoadJob;

public:
	// CLevel_Loading이 성공한 Loader만 소유한다. Initialize 실패 시 임시 객체 소멸이 worker 정리까지 수행한다.
	static unique_ptr<CLoader> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eNextLevelID,
		uint64_t iEffectLoadJobEpoch = 0u,
		uint64_t iEffectCatalogRevision = 0u);
	// 협력 취소 후 5초 대기, 동기 I/O 취소 후 추가 5초 대기를 수행한다. 시간 초과는 프로세스 fail-fast다.
	void Free();
};

NS_END
