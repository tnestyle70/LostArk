#include "Layer.h"

#include "GameInstance.h"
#include "ContainerObject.h"
#include "Profiler.h"

CLayer::CLayer(uint32_t levelIndex, const wstring_t& layerTag)
{
    std::string tag = "Unnamed";
    const int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
        layerTag.data(), static_cast<int>(layerTag.size()), nullptr, 0, nullptr, nullptr);
    if (length > 0)
    {
        tag.resize(static_cast<size_t>(length));
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, layerTag.data(),
            static_cast<int>(layerTag.size()), tag.data(), length, nullptr, nullptr);
    }
    const std::string prefix = "Layer.L" + std::to_string(levelIndex) + "." + tag;
    m_ProfileScopeNames = { prefix + ".PriorityUpdate", prefix + ".Update",
        prefix + ".PostPhysicsUpdate", prefix + ".LateUpdate" };
    m_FinalCameraScopeName = prefix + ".FinalCamera";
}

CLayer::~CLayer()
{
}

shared_ptr<CGameObject> CLayer::Get_GameObject(uint32_t iIndex)
{
	if (iIndex >= m_GameObjects.size())
		return nullptr;

	auto iter = m_GameObjects.begin();
	for (uint32_t index = 0; index < iIndex; ++index)
		++iter;

	return iter != m_GameObjects.end() ? *iter : nullptr;
}

shared_ptr<CComponent> CLayer::Get_Component(const wstring_t& strComponentTag, uint32_t iIndex)
{
	if (iIndex >= m_GameObjects.size())
		return nullptr;

	auto	iter = m_GameObjects.begin();

	for (uint32_t i = 0; i < iIndex; i++)
		++iter;

	if(iter == m_GameObjects.end())
		return nullptr;

	return (*iter)->Get_Component(strComponentTag);
}

shared_ptr<CComponent> CLayer::Get_Component(const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex)
{
	if (iIndex >= m_GameObjects.size())
		return nullptr;

	auto	iter = m_GameObjects.begin();

	for (uint32_t i = 0; i < iIndex; i++)
		++iter;

	if (iter == m_GameObjects.end())
		return nullptr;

	return static_pointer_cast<CContainerObject>(*iter)->Get_Component(strPartTag, strComponentTag);
}

HRESULT CLayer::Add_GameObject(shared_ptr<CGameObject> pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	m_GameObjects.push_back(pGameObject);
	const uint8_t phases = pGameObject->Get_UpdatePhaseMask();
	for (size_t phase = 0; phase < m_PhaseObjects.size(); ++phase)
		if (phases & (1u << phase)) m_PhaseObjects[phase].push_back(pGameObject.get());
	if (pGameObject->Uses_FinalCameraSubmission())
		m_FinalCameraObjects.push_back(pGameObject.get());

	return S_OK;
}

HRESULT CLayer::Remove_GameObject(const shared_ptr<CGameObject>& pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	auto iter = find(m_GameObjects.begin(), m_GameObjects.end(), pGameObject);
	if (iter == m_GameObjects.end())
		return E_FAIL;

	for (auto& phase : m_PhaseObjects)
	{
		const auto member = std::find(phase.begin(), phase.end(), pGameObject.get());
		if (member != phase.end()) phase.erase(member);
	}
	const auto finalMember = std::find(m_FinalCameraObjects.begin(), m_FinalCameraObjects.end(), pGameObject.get());
	if (finalMember != m_FinalCameraObjects.end()) m_FinalCameraObjects.erase(finalMember);
	m_GameObjects.erase(iter);
	return S_OK;
}

void CLayer::Priority_Update(f32_t fTimeDelta)
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_ProfileScopeNames[0]);
	for (CGameObject* pGameObject : m_PhaseObjects[0])
	{
		if (nullptr != pGameObject)
			pGameObject->Priority_Update(fTimeDelta);
	}
}

void CLayer::Update(f32_t fTimeDelta)
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_ProfileScopeNames[1]);
	for (CGameObject* pGameObject : m_PhaseObjects[1])
	{
		if (nullptr != pGameObject)
			pGameObject->Update(fTimeDelta);
	}
}

void CLayer::Post_Physics_Update(f32_t fTimeDelta)
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_ProfileScopeNames[2]);
	for (CGameObject* pGameObject : m_PhaseObjects[2])
	{
		if (nullptr != pGameObject)
			pGameObject->Post_Physics_Update(fTimeDelta);
	}
}

void CLayer::Late_Update(f32_t fTimeDelta)
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_ProfileScopeNames[3]);
	for (CGameObject* pGameObject : m_PhaseObjects[3])
	{
		if (nullptr != pGameObject)
			pGameObject->Late_Update(fTimeDelta);
	}
}

void CLayer::Submit_FinalCamera()
{
	if (m_FinalCameraObjects.empty()) return;
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_FinalCameraScopeName);
	for (CGameObject* object : m_FinalCameraObjects)
		object->Submit_FinalCamera();
}

shared_ptr<CLayer> CLayer::Create(uint32_t levelIndex, const wstring_t& layerTag)
{
	return shared_ptr<CLayer>(new CLayer(levelIndex, layerTag));
}
