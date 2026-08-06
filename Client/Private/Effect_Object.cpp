#include "Effect_Object.h"

#include "Effect_DocumentRenderer.h"
#include "GameInstance.h"

Client::CEffectObject::CEffectObject(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(pDevice, pContext),
	  m_pRenderer(make_unique<CEffectDocumentRenderer>(
		  std::move(pDevice), std::move(pContext)))
{
	XMStoreFloat4x4(&m_RootWorld, XMMatrixIdentity());
}

Client::CEffectObject::~CEffectObject() = default;

HRESULT Client::CEffectObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT Client::CEffectObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)) ||
		FAILED(m_pRenderer->Initialize()))
	{
		return E_FAIL;
	}
	if (nullptr == pArg)
		return S_OK;
	const EFFECT_OBJECT_DESC& Desc =
		*static_cast<EFFECT_OBJECT_DESC*>(pArg);
	m_RootWorld = Desc.RootWorld;
	m_bPlaying = Desc.bAutoPlay;
	if (nullptr != Desc.pDocument &&
		!Stage_Document(*Desc.pDocument, m_strStatus))
	{
		return E_FAIL;
	}
	return S_OK;
}

bool_t Client::CEffectObject::Stage_Document(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	CEffectPlayback StagedPlayback;
	if (!StagedPlayback.Stage_Document(Document, strOutError))
		return false;
	if (!m_pRenderer->Stage_Document(Document, strOutError))
		return false;
	m_Playback = std::move(StagedPlayback);
	m_Playback.Seek(0.f, m_RootWorld);
	m_strStatus = "Effect Document staged.";
	strOutError.clear();
	return true;
}

void Client::CEffectObject::Set_RootWorld(const float4x4_t& RootWorld)
{
	m_RootWorld = RootWorld;
	m_Playback.Update(0.f, m_RootWorld);
}

void Client::CEffectObject::Set_SampleTime(const f32_t fSampleTimeSeconds)
{
	m_bPlaying = false;
	m_Playback.Seek(fSampleTimeSeconds, m_RootWorld);
}

void Client::CEffectObject::Advance_Preview(const f32_t fTimeDelta)
{
	m_Playback.Update((std::max)(0.f, fTimeDelta), m_RootWorld);
}

void Client::CEffectObject::Reset()
{
	m_Playback.Reset();
	m_Playback.Seek(0.f, m_RootWorld);
}

void Client::CEffectObject::Update(const f32_t fTimeDelta)
{
	if (m_bPlaying)
		m_Playback.Update(fTimeDelta, m_RootWorld);
}

void Client::CEffectObject::Late_Update(const f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (!m_bVisible)
		return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::BLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT Client::CEffectObject::Render()
{
	if (!m_bVisible)
		return S_FALSE;
	const HRESULT Result = m_pRenderer->Render(m_Playback.Get_Frame());
	if (FAILED(Result))
		m_strStatus = m_pRenderer->Get_Status();
	return Result;
}

unique_ptr<Client::CEffectObject> Client::CEffectObject::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	unique_ptr<CEffectObject> Instance(new CEffectObject(
		std::move(pDevice), std::move(pContext)));
	if (FAILED(Instance->Initialize_Prototype()))
		return nullptr;
	return Instance;
}

shared_ptr<CPrototype> Client::CEffectObject::Clone(void* pArg)
{
	shared_ptr<CEffectObject> Instance(new CEffectObject(
		m_pDevice, m_pContext));
	if (FAILED(Instance->Initialize(pArg)))
		return nullptr;
	return Instance;
}
