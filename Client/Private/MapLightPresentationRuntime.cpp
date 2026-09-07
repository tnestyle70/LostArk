#include "MapLightPresentationRuntime.h"

#include "Effect_LightPresentation.h"
#include "MapAssetCatalog.h"
#include "Presentation_Manager.h"
#include <algorithm>
#include <cmath>
#include <vector>

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

bool_t Client::CMapLightPresentationRuntime::Replace_Document(const CMapLightDocument& document)
{
	if (!document.Is_Ready()) {m_Status="Cannot preview an unready light document.";return false;}
	m_Document=document;m_Status="Map light authoring preview ready.";return true;
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
	std::vector<LIGHT_DESC> lights;
	for (const MAP_POINT_LIGHT_RECORD& record : m_Document.Get_Lights())
	{
		if (!record.enabled || s_fSceneIntensityMultiplier == 0.f) continue;
		const f32_t brightness = record.brightness * s_fSceneIntensityMultiplier;
		EFFECT_EVALUATED_LIGHT evaluated{};
		evaluated.vWorldPosition = record.position;
		evaluated.fRange = record.radiusMeters;
		evaluated.fIntensity = brightness;
		evaluated.vColor = record.color;
		evaluated.vAmbient = { 0.f, 0.f, 0.f, 0.f };
		evaluated.fFalloffExponent = record.falloffExponent;

		LIGHT_DESC light{};
		if (!Try_BuildEffectPointLightDesc(evaluated, light))
		{
			if (record.kind == LIGHT::DIRECTIONAL) light = {};
			else
			{
			m_Status = "Map point light mapping failed: " + record.lightId;
			return E_FAIL;
			}
		}
		light.eType=record.kind;
		if (record.kind==LIGHT::SPOT || record.kind==LIGHT::DIRECTIONAL)
		{
			const auto& r=record.rotationDegrees;
			const matrix_t rotation=XMMatrixRotationRollPitchYaw(XMConvertToRadians(r.x),XMConvertToRadians(r.y),XMConvertToRadians(r.z));
			XMStoreFloat4(&light.vDirection,XMVectorSetW(XMVector3Normalize(rotation.r[2]),0.f));
			light.fSpotInnerCos=std::cos(XMConvertToRadians(record.innerConeDegrees));
			light.fSpotOuterCos=std::cos(XMConvertToRadians(record.outerConeDegrees));
			light.vDiffuse={record.color.x*brightness,record.color.y*brightness,record.color.z*brightness,1.f};
			light.vAmbient={0,0,0,0};light.vSpecular={0,0,0,0};
		}
		lights.push_back(light);
	}
	const size_t used=presentation.Get_TransientLights().size();
	const size_t budget=used<56u?56u-used:0u;
	const size_t count=(std::min)(lights.size(),budget);
	presentation.Register_ProviderSubmissionExpectation(count,count,0u,0u);
	for(size_t i=0;i<count;++i)
	{
		const LIGHT_DESC& light=lights[i];
		const HRESULT result = presentation.Add_TransientLight(light);
		if (FAILED(result))
		{
			m_Status = "Map light submission failed.";
			return E_FAIL;
		}
	}
	m_Status = "Map light presentation submitted: " +
		std::to_string(count) + " lights; skipped by budget: "+std::to_string(lights.size()-count);
	return S_OK;
}

void Client::CMapLightPresentationRuntime::Clear()
{
	m_Document.Clear();
	m_Status = "Map light presentation is not loaded";
}
