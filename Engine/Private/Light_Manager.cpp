#include "Light_Manager.h"
#include "Engine_RenderTypes.h"

#include "Light.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "Presentation_Manager.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

#include <array>

#include <cmath>
#include <limits>

namespace
{
	struct LIGHT_CAMERA_VOLUME final
	{
		double planes[6][4]{};
		double roundoffWeights[6][4]{};
		bool valid = false;

		static bool HasFiniteInverse(const float4x4_t* matrix)
		{
			if (!matrix) return false;
			double rows[4][4]{};
			for (size_t row = 0u; row < 4u; ++row)
				for (size_t column = 0u; column < 4u; ++column)
				{
					rows[row][column] = matrix->m[row][column];
					if (!std::isfinite(rows[row][column])) return false;
				}
			for (size_t column = 0u; column < 4u; ++column)
			{
				size_t pivot = column;
				for (size_t row = column + 1u; row < 4u; ++row)
					if (std::abs(rows[row][column]) > std::abs(rows[pivot][column])) pivot = row;
				if (std::abs(rows[pivot][column]) <= (std::numeric_limits<double>::min)()) return false;
				for (size_t entry = column; entry < 4u; ++entry)
					std::swap(rows[column][entry], rows[pivot][entry]);
				for (size_t row = column + 1u; row < 4u; ++row)
				{
					const double factor = rows[row][column] / rows[column][column];
					for (size_t entry = column + 1u; entry < 4u; ++entry)
					{
						rows[row][entry] -= factor * rows[column][entry];
						if (!std::isfinite(rows[row][entry])) return false;
					}
				}
			}
			return true;
		}

		LIGHT_CAMERA_VOLUME(const float4x4_t* view, const float4x4_t* projection)
		{
			if (!HasFiniteInverse(view) || !HasFiniteInverse(projection)) return;
			// Row-vector D3D clip inequalities: +/-x-w, +/-y-w, z-w, -z.
			for (size_t plane = 0u; plane < 6u; ++plane)
			{
				const size_t axis = plane / 2u;
				const double axisSign = (plane & 1u) ? -1. : 1.;
				const double wSign = plane == 5u ? 0. : -1.;
				for (size_t row = 0u; row < 4u; ++row)
					for (size_t column = 0u; column < 4u; ++column)
					{
						const double a = projection->m[column][axis];
						const double w = wSign * projection->m[column][3];
						planes[plane][row] += double(view->m[row][column]) * (axisSign * a + w);
						roundoffWeights[plane][row] += std::abs(double(view->m[row][column])) *
							(std::abs(a) + std::abs(w));
					}
				const double length = std::sqrt(planes[plane][0] * planes[plane][0] +
					planes[plane][1] * planes[plane][1] + planes[plane][2] * planes[plane][2]);
				if (!std::isfinite(length) || length <= (std::numeric_limits<double>::min)()) return;
				for (size_t entry = 0u; entry < 4u; ++entry)
				{
					planes[plane][entry] /= length;
					roundoffWeights[plane][entry] /= length;
					if (!std::isfinite(planes[plane][entry]) || !std::isfinite(roundoffWeights[plane][entry])) return;
				}
			}
			valid = true;
		}

		bool Intersects(const LIGHT_DESC& light) const
		{
			if (!valid) return true;
			const double center[3] = {light.vPosition.x, light.vPosition.y, light.vPosition.z};
			const double radius = double(light.fRange) + 0.05;
			bool outside = false;
			for (size_t plane = 0u; plane < 6u; ++plane)
			{
				double distance = planes[plane][3];
				double magnitude = roundoffWeights[plane][3];
				for (size_t axis = 0u; axis < 3u; ++axis)
				{
					distance += planes[plane][axis] * center[axis];
					magnitude += roundoffWeights[plane][axis] * (std::abs(center[axis]) + radius);
				}
				// Enclose float view/projection evaluation, including cancellation at large world coordinates.
				const double tolerance = 32. * std::numeric_limits<float>::epsilon() * (1. + magnitude);
				if (!std::isfinite(distance) || !std::isfinite(tolerance)) return true;
				outside = outside || distance > radius + tolerance;
			}
			return !outside;
		}
	};

	bool_t IsFinite4(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}

	bool_t IsValidSceneLight(const LIGHT_DESC& Light)
	{
		if (!CLight::Is_ValidDesc(Light)) return false;
		if (Light.staticShadowChannel > 15u || (Light.eReceiver != LIGHT_RECEIVER::ALL &&
			Light.eReceiver != LIGHT_RECEIVER::SOURCE_CHARACTER &&
			Light.eReceiver != LIGHT_RECEIVER::UNBAKED) ||
			(LIGHT::POINT != Light.eType && LIGHT::SPOT != Light.eType &&
			LIGHT::DIRECTIONAL != Light.eType) || !IsFinite4(Light.vDiffuse) ||
			!IsFinite4(Light.vAmbient) || !IsFinite4(Light.vSpecular) ||
			!std::isfinite(Light.fFalloffExponent) ||
			Light.fFalloffExponent <= 0.f ||
			Light.vDiffuse.x < 0.f || Light.vDiffuse.y < 0.f ||
			Light.vDiffuse.z < 0.f || Light.vAmbient.x < 0.f ||
			Light.vAmbient.y < 0.f || Light.vAmbient.z < 0.f ||
			Light.vSpecular.x < 0.f || Light.vSpecular.y < 0.f ||
			Light.vSpecular.z < 0.f)
		{
			return false;
		}
		if (LIGHT::POINT != Light.eType)
		{
			const float fDirectionLengthSquared =
				Light.vDirection.x * Light.vDirection.x +
				Light.vDirection.y * Light.vDirection.y +
				Light.vDirection.z * Light.vDirection.z;
			if (!IsFinite4(Light.vDirection) ||
				!std::isfinite(fDirectionLengthSquared) ||
				fDirectionLengthSquared <= 0.000001f)
				return false;
		}
		if (LIGHT::DIRECTIONAL != Light.eType &&
			(!IsFinite4(Light.vPosition) || !std::isfinite(Light.fRange) ||
			 Light.fRange <= 0.f))
			return false;
		return LIGHT::SPOT != Light.eType ||
			(std::isfinite(Light.fSpotInnerCos) &&
			 std::isfinite(Light.fSpotOuterCos) &&
			 Light.fSpotOuterCos > 0.f &&
			 Light.fSpotOuterCos <= Light.fSpotInnerCos &&
			 Light.fSpotOuterCos < 1.f && Light.fSpotInnerCos <= 1.f);
	}
}

CLight_Manager::CLight_Manager()
{
}

CLight_Manager::~CLight_Manager()
{
}

HRESULT CLight_Manager::Add_Light(const LIGHT_DESC& LightDesc)
{
	vector<LIGHT_DESC> SceneLights{ LightDesc };
	return Replace_SceneLights(std::move(SceneLights));
}

HRESULT CLight_Manager::Replace_SceneLights(vector<LIGHT_DESC> SceneLights)
{
	if (SceneLights.size() > 16u)
		return E_INVALIDARG;
	for (const LIGHT_DESC& LightDesc : SceneLights)
	{
		if (!IsValidSceneLight(LightDesc))
			return E_INVALIDARG;
	}
	m_SceneLights.swap(SceneLights);
	return S_OK;
}

HRESULT CLight_Manager::Render_Lights(
    shared_ptr<class CShader> pShader,
    shared_ptr<class CVIBuffer_Rect> pVIBuffer,
    bool_t bEnableSceneDirectionalShadow, LIGHT_RECEIVER ePassReceiver, bool_t bSourceLightMask)
{
    if (!pShader || !pVIBuffer || (ePassReceiver != LIGHT_RECEIVER::ALL &&
        ePassReceiver != LIGHT_RECEIVER::SOURCE_CHARACTER)) return E_INVALIDARG;

    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope stageScope(profiler, "Render.Lights.StageAndSubmit");
    const LIGHT_CAMERA_VOLUME cameraVolume(CGameInstance::Get().Get_Transform(D3DTS::VIEW),
        CGameInstance::Get().Get_Transform(D3DTS::PROJ));
    // Upload bounded shader records repeatedly; the frame has no light-count cap.
    constexpr uint32_t maximumLightInstances = CPresentation_Manager::LIGHT_RENDER_BATCH_SIZE;
    struct LightInstance
    {
        float4_t direction, position, diffuse, ambient, specular, attenuation;
        uint32_t flags[4]; // receiver, static channel, directional shadow, source local bounds
        float4_t sourceCharacterAmbient;
    };
    static_assert(sizeof(LightInstance) == 128u);
    std::array<LightInstance, maximumLightInstances> instances;
    std::array<LIGHT, maximumLightInstances> types;
    uint32_t count = 0u;
    bool_t directionalShadowConsumed = false;
    bool_t staticShadowConsumed = false;
    const bool_t sourceMask = bSourceLightMask && ePassReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER;
    const auto flush = [&]() -> HRESULT
    {
        if (count == 0u) return S_OK;
        CProfilerScope submitScope(profiler, "Render.Lights.UploadAndDraw");
        if (profiler)
        {
            profiler->Add_Counter(EProfilerCounter::LightRecords, count);
            profiler->Add_Counter(EProfilerCounter::LightUploadBytes, sizeof(LightInstance) * count);
        }
        if (FAILED(pShader->Bind_RawValue("g_LightInstances", instances.data(),
            sizeof(LightInstance) * count))) return E_FAIL;
        // Preserve source order and type runs, including across batch boundaries.
        for (uint32_t first = 0u; first < count;)
        {
            uint32_t end = first + 1u;
            while (end < count && types[end] == types[first]) ++end;
            const uint32_t typePass = types[first] == LIGHT::DIRECTIONAL ? 0u :
                types[first] == LIGHT::POINT ? 1u : 2u;
            const uint32_t pass = 22u + typePass + (sourceMask ? 3u : 0u);
            if (FAILED(pShader->Bind_RawValue("g_LightInstanceOffset", &first, sizeof(first))) ||
                FAILED(pShader->Begin(pass)) ||
                FAILED(pVIBuffer->Render_Instanced(end - first))) return E_FAIL;
            if (profiler) profiler->Add_Counter(EProfilerCounter::LightDrawCalls);
            first = end;
        }
        count = 0u;
        return S_OK;
    };
    const auto stage = [&](const LIGHT_DESC& light, bool_t scene) -> HRESULT
    {
        if (light.eReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER &&
            ePassReceiver == LIGHT_RECEIVER::ALL) return S_OK;
        if (!CLight::Is_ValidDesc(light)) return E_INVALIDARG;
        if (light.eType != LIGHT::DIRECTIONAL)
        {
            if (profiler) profiler->Add_Counter(EProfilerCounter::LightCullingCandidates);
            // Cull only this deferred submission. Forward consumers retain the original light list.
            if (!cameraVolume.Intersects(light))
            {
                if (profiler) profiler->Add_Counter(EProfilerCounter::LightCullingRejected);
                return S_OK;
            }
        }
        if (count == maximumLightInstances && FAILED(flush())) return E_FAIL;
        const bool_t directional = light.eType == LIGHT::DIRECTIONAL;
        const bool_t shadow = scene && directional && bEnableSceneDirectionalShadow && !directionalShadowConsumed;
        const bool_t staticShadow = scene && directional && !staticShadowConsumed;
        auto& record = instances[count];
        record.direction = light.vDirection;
        record.position = light.vPosition;
        record.diffuse = light.vDiffuse;
        record.ambient = light.vAmbient;
        record.sourceCharacterAmbient = scene ? light.vSourceCharacterAmbient : float4_t{};
        record.specular = light.vSpecular;
        record.attenuation = float4_t(light.fRange, light.fFalloffExponent,
            light.fSpotInnerCos, light.fSpotOuterCos);
        record.flags[0] = static_cast<uint32_t>(light.eReceiver);
        record.flags[1] = light.staticShadowChannel != 0u ? light.staticShadowChannel : (staticShadow ? 1u : 0u);
        record.flags[2] = shadow ? 1u : 0u;
        record.flags[3] = !directional && ePassReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER ? 1u : 0u;
        types[count++] = light.eType;
        if (shadow) directionalShadowConsumed = true;
        if (staticShadow) staticShadowConsumed = true;
        return S_OK;
    };
    for (const auto& light : m_SceneLights)
        if (FAILED(stage(light, true))) return E_FAIL;
    for (const auto& light : CPresentation_Manager::Get().Get_TransientLights())
        if (FAILED(stage(light, false))) return E_FAIL;
    return flush();
}

unique_ptr<CLight_Manager> CLight_Manager::Create()
{
    return unique_ptr<CLight_Manager>(new CLight_Manager());
}
