#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "MapLoadScope.h"

#include <atomic>
#include <mutex>

NS_BEGIN(Client)

class CLevelRegistry;

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

	STATE Get_State() const
	{
		return m_eState.load(std::memory_order_acquire);
	}
	HRESULT Initialize(LEVEL eNextLevelID);
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

#ifdef _DEBUG
	void Print_Text();
#endif

private:
	HRESULT Ready_For_Lobby();
	HRESULT Ready_For_Bern();
	HRESULT Ready_For_ValtanArena();
	HRESULT Ready_For_Development();

	HRESULT Ready_MapArea(
		uint32_t iLevelIndex,
		const std::string& areaId,
		const MAP_LOAD_SCOPE& loadScope = {});
	HRESULT Ready_Camera_Prototype(uint32_t iLevelIndex);
	HRESULT Ready_StaticMeshShader(uint32_t iLevelIndex);
	HRESULT Ready_Character_Rendering(uint32_t iLevelIndex);
	HRESULT Ready_Character_Shared_Prototypes(uint32_t iLevelIndex);
	HRESULT Ready_LanceMaster_Prototypes(uint32_t iLevelIndex);
	HRESULT Ready_ValtanPresentation(uint32_t iLevelIndex);
	void Set_Status(const tchar_t* pStatus);
	void Copy_Status(tchar_t* pOutput, size_t outputCount) const;

private:
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	LEVEL m_eNextLevelID = LEVEL::END;
	HANDLE m_hThread = {};
	tchar_t m_szLoadingText[MAX_PATH] = {};
	mutable std::mutex m_StatusMutex;
	std::atomic<STATE> m_eState = STATE::IDLE;
	std::atomic<long> m_iResult = S_FALSE;
	std::atomic_bool m_isCancellationRequested = false;

public:
	static unique_ptr<CLoader> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eNextLevelID);
	void Free();
};

NS_END
