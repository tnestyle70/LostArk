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
	void Clear();

private:
	void Clear_Destination();
	void Report_Failure(const std::string& status);
	bool_t Sample_Destination(bool_t rebuildHistory);
	uint32_t m_iLevelIndex = ETOUI(LEVEL::END);
	EFFECT_WORLD_ROOT_HANDLE m_ClickHandle;
	EFFECT_WORLD_ROOT_HANDLE m_DestinationHandle;
	weak_ptr<CCharacter> m_pCharacter;
	float3_t m_WorldPosition{};
	float4x4_t m_RootWorld{};
	f32_t m_fClickSeconds = 0.f;
	f32_t m_fDestinationSeconds = 0.f;
	bool_t m_hasObservedMovement = false;
	std::string m_strLastFailure;

public:
	static unique_ptr<CClickMoveEffect> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};
NS_END
