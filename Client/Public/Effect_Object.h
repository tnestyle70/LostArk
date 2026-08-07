#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_Playback.h"
#include "GameObject.h"
#include "PresentationProvider.h"

#include <memory>
#include <string>
#include <unordered_map>

NS_BEGIN(Client)

class CEffectDocumentRenderer;

class CEffectObject final : public CGameObject, public IPresentationProvider
{
public:
	struct EFFECT_OBJECT_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		const EFFECT_DOCUMENT_DESC* pDocument = nullptr;
		float4x4_t RootWorld{};
		bool_t bAutoPlay = true;
		f32_t fPlaybackRate = 1.f;
	};

private:
	CEffectObject(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CEffectObject();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Submit_Presentation() override;

	bool_t Stage_Document(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	void Set_RootWorld(const float4x4_t& RootWorld);
	void Set_SourceAnchorWorlds(
		const std::unordered_map<std::string, float4x4_t>& SourceAnchorWorlds);
	void Set_SampleTime(f32_t fSampleTimeSeconds);
	void Advance_Preview(f32_t fTimeDelta);
	void Set_Playing(bool_t bPlaying) { m_bPlaying = bPlaying; }
	void Set_Visible(bool_t bVisible) { m_bVisible = bVisible; }
	void Reset();
	bool_t Is_Finished() const { return m_Playback.Is_Finished(); }
	bool_t Query_ParticleRuntimeProbe(
		std::string_view strElementId,
		EFFECT_PARTICLE_RUNTIME_PROBE& OutProbe) const
	{
		return m_Playback.Query_ParticleRuntimeProbe(strElementId, OutProbe);
	}
	const std::string& Get_Status() const { return m_strStatus; }

private:
	unique_ptr<CEffectDocumentRenderer> m_pRenderer;
	CEffectPlayback m_Playback;
	float4x4_t m_RootWorld{};
	bool_t m_bPlaying = true;
	bool_t m_bVisible = true;
	f32_t m_fPlaybackRate = 1.f;
	std::string m_strStatus;

public:
	static unique_ptr<CEffectObject> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
