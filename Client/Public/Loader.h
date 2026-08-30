#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "MapLoadScope.h"
#include "Network/PacketType.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <span>
#include <string>

NS_BEGIN(Client)

class CLevelRegistry;
class CEffectLoadPreparationJob;

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
	enum class STATE : uint32_t
	{
		IDLE,
		RUNNING,
		SUCCEEDED,
		FAILED
	};
	struct PROGRESS_SNAPSHOT final
	{
		std::string strStatus;
		bool_t bDeterminate = false;
		size_t iCompleted = 0u;
		size_t iTotal = 0u;
		uint64_t iElapsedMs = 0u;
	};

	STATE Get_State() const
	{
		return m_eState.load(std::memory_order_acquire);
	}
	HRESULT Initialize(LEVEL eNextLevelID);
	HRESULT Initialize(
		LEVEL eNextLevelID,
		uint64_t iEffectLoadJobEpoch,
		uint64_t iEffectCatalogRevision);
	HRESULT Start_Loading();
	bool_t Finished() const
	{
		return STATE::SUCCEEDED == Get_State();
	}
	bool_t Failed() const { return STATE::FAILED == Get_State(); }
	HRESULT Get_Result() const
	{
		return static_cast<HRESULT>(
			m_iResult.load(std::memory_order_acquire));
	}
	static std::string Get_ActiveStatus();
	PROGRESS_SNAPSHOT Get_ProgressSnapshot() const;
	std::shared_ptr<CEffectLoadPreparationJob> Get_EffectLoadJob() const
	{
		return m_pEffectLoadJob;
	}

#ifdef _DEBUG
	void Print_Text();
#endif

private:
	HRESULT Ready_For_Lobby();
	HRESULT Ready_For_CharacterSelect();
	HRESULT Ready_For_Bern();
	HRESULT Ready_For_ValtanArena();
	HRESULT Ready_For_KakulSaydonArena();
	HRESULT Ready_For_Development();

	HRESULT Ready_MapArea(
		uint32_t iLevelIndex,
		const std::string& areaId,
		const MAP_LOAD_SCOPE& loadScope = {});
	HRESULT Ready_MapAuthoringCore(uint32_t iLevelIndex);
	HRESULT Ready_Camera_Prototype(uint32_t iLevelIndex);
	HRESULT Ready_StaticMeshShader(uint32_t iLevelIndex);
	HRESULT Ready_AnimatedMeshShader(uint32_t iLevelIndex);
	HRESULT Ready_DeployPropCore(uint32_t iLevelIndex);
	HRESULT Ready_DeployPropArea(
		uint32_t iLevelIndex,
		const std::string& areaId);
	HRESULT Ready_Character_Rendering(
		uint32_t iLevelIndex,
		std::span<const LostArk::Shared::CHARACTER_CLASS_ID> characterClasses);
	HRESULT Ready_Character_Shared_Prototypes(uint32_t iLevelIndex);
	HRESULT Ready_AnimationPreviewModels(uint32_t iLevelIndex);
	HRESULT Ready_ValtanPresentation(uint32_t iLevelIndex);
	HRESULT Run_EffectLoadPreparation();
	void Set_Status(const tchar_t* pStatus);
	void Set_DeterminateStatus(
		const tchar_t* pStatus,
		size_t iCompleted,
		size_t iTotal);
	void Copy_Status(tchar_t* pOutput, size_t outputCount) const;

private:
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	LEVEL m_eNextLevelID = LEVEL::END;
	HANDLE m_hThread = {};
	tchar_t m_szLoadingText[MAX_PATH] = {};
	mutable std::mutex m_StatusMutex;
	bool_t m_bProgressDeterminate = false;
	size_t m_iProgressCompleted = 0u;
	size_t m_iProgressTotal = 0u;
	std::chrono::steady_clock::time_point m_ProgressPhaseStarted =
		std::chrono::steady_clock::now();
	std::atomic<STATE> m_eState = STATE::IDLE;
	std::atomic<long> m_iResult = S_FALSE;
	std::atomic_bool m_isCancellationRequested = false;
	std::shared_ptr<CEffectLoadPreparationJob> m_pEffectLoadJob;

public:
	static unique_ptr<CLoader> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eNextLevelID,
		uint64_t iEffectLoadJobEpoch = 0u,
		uint64_t iEffectCatalogRevision = 0u);
	void Free();
};

NS_END
