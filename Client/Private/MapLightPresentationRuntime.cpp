#include "MapLightPresentationRuntime.h"

#include "Effect_LightPresentation.h"
#include "MapAssetCatalog.h"
#include "Presentation_Manager.h"

bool_t Client::CMapLightPresentationRuntime::Load(
	const std::filesystem::path& path,
	const std::string& expectedAreaId)
{
	CMapLightDocument staged;
	std::string status;
	if (!staged.Load(path, expectedAreaId, status))
	{
		m_Status = status;
		return false;
	}
	m_Document = std::move(staged);
	m_Status = status;
	return true;
}

bool_t Client::CMapLightPresentationRuntime::Load_Runtime(
	const std::string& areaId)
{
	if (areaId.empty())
	{
		m_Status = "Map light runtime Area is empty";
		return false;
	}
	return Load(
		CMapAssetCatalog::Get_MapDataRoot() /
			std::filesystem::path(areaId + ".maplights.json"),
		areaId);
}

bool_t Client::CMapLightPresentationRuntime::Submit_Frame()
{
	if (!m_Document.Is_Ready())
	{
		m_Status = "Map light presentation is not ready";
		return false;
	}
	const shared_ptr<CMapLightPresentationRuntime> self =
		weak_from_this().lock();
	if (nullptr == self)
	{
		m_Status = "Map light presentation requires shared ownership";
		return false;
	}
	const HRESULT result = CPresentation_Manager::Get().Add_FrameProvider(
		self);
	if (FAILED(result))
	{
		m_Status = "Map light frame provider registration failed";
		return false;
	}
	return true;
}

HRESULT Client::CMapLightPresentationRuntime::Submit_Presentation()
{
	if (!m_Document.Is_Ready())
	{
		m_Status = "Map light presentation is not ready during submit";
		return E_FAIL;
	}

	CPresentation_Manager& presentation = CPresentation_Manager::Get();
	const uint64_t lightCount = static_cast<uint64_t>(
		m_Document.Get_Lights().size());
	presentation.Register_ProviderSubmissionExpectation(
		lightCount, lightCount, 0u, 0u);

	for (const MAP_POINT_LIGHT_RECORD& record : m_Document.Get_Lights())
	{
		EFFECT_EVALUATED_LIGHT evaluated{};
		evaluated.vWorldPosition = record.position;
		evaluated.fRange = record.radiusMeters;
		evaluated.fIntensity = record.brightness;
		evaluated.vColor = record.color;
		evaluated.vAmbient = { 0.f, 0.f, 0.f, 0.f };
		evaluated.fFalloffExponent = record.falloffExponent;

		LIGHT_DESC light{};
		if (!Try_BuildEffectPointLightDesc(evaluated, light))
		{
			m_Status = "Map point light mapping failed: " + record.lightId;
			return E_FAIL;
		}
		const HRESULT result = presentation.Add_TransientLight(light);
		if (FAILED(result))
		{
			m_Status = "Map point light submission failed: " + record.lightId;
			return E_FAIL;
		}
	}
	m_Status = "Map light presentation submitted: " +
		std::to_string(m_Document.Get_Lights().size()) + " point lights";
	return S_OK;
}

void Client::CMapLightPresentationRuntime::Clear()
{
	m_Document.Clear();
	m_Status = "Map light presentation is not loaded";
}
