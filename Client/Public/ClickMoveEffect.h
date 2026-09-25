#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "Effect_PresentationService.h"

NS_BEGIN(Client)
class CCharacter;

/* Cosmetic feedback for a submitted typed move command, never Server navigation
   approval. The existing Product Effect owner renders these prepared World assets. */
class CClickMoveEffect final : public CGameObject
{
public:
	static constexpr const wchar_t* PROTOTYPE_TAG =
		L"Prototype_GameObject_ClickMoveEffect";
	static bool_t Uses_LevelMarkers(LEVEL level);
	static std::vector<std::string> Queue_LevelResources(LEVEL level);

private:
	CClickMoveEffect(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	CClickMoveEffect(const CClickMoveEffect& prototype);

public:
	virtual ~CClickMoveEffect();
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	bool_t Initialize_Effects(uint32_t levelIndex);
	void Play(const float3_t& worldPosition,
		const shared_ptr<CCharacter>& character);
	void Play_Ping(const float3_t& worldPosition, std::uint32_t senderId, std::uint32_t sequence);
	void Clear_Move();
	void Clear();

private:
	void Report_Failure(const std::string& status);
	bool_t Spawn_Marker(const char* effectId, const char* placementId,
		const float3_t& position, EFFECT_WORLD_ROOT_HANDLE& handle, bool_t sustained = false);
	uint32_t m_iLevelIndex = ETOUI(LEVEL::END);
	EFFECT_WORLD_ROOT_HANDLE m_ClickHandle;
	struct ROOM_PING_MARKER
	{
		EFFECT_WORLD_ROOT_HANDLE Handle;
		f32_t fSeconds = 0.f;
	};
	std::vector<ROOM_PING_MARKER> m_RoomPings;
	weak_ptr<CCharacter> m_pCharacter;
	f32_t m_fClickSeconds = 0.f;
	std::string m_strLastFailure;

public:
	static unique_ptr<CClickMoveEffect> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};
NS_END
