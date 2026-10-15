// 장면 빛의 교체와 프레임 지연 조명 배치 제출을 구현한다.
#include "Light_Manager.h"
// 빛 설명과 렌더 수신 대상을 소비한다.
#include "Engine_RenderTypes.h"

// 장면/일시적 빛 모두에 공통 기하·감쇠 검사를 적용한다.
#include "Light.h"
// 현재 카메라 행렬과 profiler를 조회한다.
#include "GameInstance.h"
// 수신 대상별 CPU/GPU 구간과 제외/업로드/draw 개수를 기록한다.
#include "Profiler.h"
// 이번 프레임 transient 빛 목록과 shader 배치 크기를 읽는다.
#include "Presentation_Manager.h"
// 배치 record와 offset을 shader에 바인딩한다.
#include "Shader.h"
// 연속된 같은 종류의 빛을 quad instance draw로 제출한다.
#include "VIBuffer_Rect.h"

// 고정 크기 업로드 record와 종류 배열을 호출 스택에 보관한다.
#include <array>

// 카메라 평면 정규화와 빛 입력의 유한성을 검사한다.
#include <cmath>
// 역행렬 가능성 검사와 float 연산 오차의 보수적 여유를 계산한다.
#include <limits>

namespace
{
	// 현재 VIEW/PROJ의 여섯 clip 평면을 월드 공간에 표현한다. 만들 수 없는 카메라는 빛을 제외하지 않는 쪽으로 처리한다.
	struct LIGHT_CAMERA_VOLUME final
	{
		double planes[6][4]{};
		double roundoffWeights[6][4]{};
		bool valid = false;

		// 부분 pivot 소거로 유한 행렬의 비특이성을 확인한다. 역행렬 자체를 출력하거나 완벽한 수치 조건을 보장하는 함수는 아니다.
		static bool HasFiniteInverse(const float4x4_t* matrix)
		{
			if (!matrix)
				return false;
			// 입력 행렬은 보존하고 double 지역 배열에서만 소거한다. 모든 원소가 유한해야 이후 pivot 비교를 진행한다.
			double rows[4][4]{};
			for (size_t row = 0u; row < 4u; ++row)
				for (size_t column = 0u; column < 4u; ++column)
				{
					rows[row][column] = matrix->m[row][column];
					if (!std::isfinite(rows[row][column]))
						return false;
				}
			// 열마다 절댓값이 가장 큰 남은 pivot을 선택한다. 0에 가까운 pivot이나 소거 중 비유한 결과는 사용 불가다.
			for (size_t column = 0u; column < 4u; ++column)
			{
				size_t pivot = column;
				for (size_t row = column + 1u; row < 4u; ++row)
					if (std::abs(rows[row][column]) > std::abs(rows[pivot][column]))
						pivot = row;
				if (std::abs(rows[pivot][column]) <= (std::numeric_limits<double>::min)())
					return false;
				for (size_t entry = column; entry < 4u; ++entry)
					std::swap(rows[column][entry], rows[pivot][entry]);
				for (size_t row = column + 1u; row < 4u; ++row)
				{
					const double factor = rows[row][column] / rows[column][column];
					for (size_t entry = column + 1u; entry < 4u; ++entry)
					{
						rows[row][entry] -= factor * rows[column][entry];
						if (!std::isfinite(rows[row][entry]))
							return false;
					}
				}
			}
			return true;
		}

		// D3D 행벡터 clip 부등식의 계수를 view/projection에서 계산하고 법선 길이로 나눠 거리 판정에 쓴다.
		LIGHT_CAMERA_VOLUME(const float4x4_t* view, const float4x4_t* projection)
		{
			if (!HasFiniteInverse(view) || !HasFiniteInverse(projection))
				return;
			// 행벡터 D3D clip 부등식은 +/-x-w, +/-y-w, z-w, -z다. 내부가 0 이하가 되도록 평면을 만든다.
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
				if (!std::isfinite(length) || length <= (std::numeric_limits<double>::min)())
					return;
				for (size_t entry = 0u; entry < 4u; ++entry)
				{
					planes[plane][entry] /= length;
					roundoffWeights[plane][entry] /= length;
					if (!std::isfinite(planes[plane][entry]) || !std::isfinite(roundoffWeights[plane][entry]))
						return;
				}
			}
			valid = true;
		}

		// 지역광 range에 0.05 월드 단위 여유를 더한 구를 검사한다. spot도 cone 대신 포함 구로 보수적으로 검사한다.
		bool Intersects(const LIGHT_DESC& light) const
		{
			if (!valid)
				return true;
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
				// 큰 월드 좌표에서의 상쇄를 포함해 float view/projection 계산의 오차 여유를 거리 경계에 더한다.
				const double tolerance = 32. * std::numeric_limits<float>::epsilon() * (1. + magnitude);
				if (!std::isfinite(distance) || !std::isfinite(tolerance))
					return true;
				outside = outside || distance > radius + tolerance;
			}
			return !outside;
		}
	};

	// 색상/방향/위치 네 성분의 유한성만 검사한다.
	bool_t IsFinite4(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}

	// 공통 광원 검사에 더해 diffuse/ambient/specular의 유한성과 RGB 음수 금지를 검사한다.
	bool_t IsValidSceneLight(const LIGHT_DESC& Light)
	{
		if (!CLight::Is_ValidDesc(Light))
			return false;
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

// 호환 진입점이다. 현재 장면 목록을 한 빛으로 교체하는 기존 의미를 유지한다.
HRESULT CLight_Manager::Add_Light(const LIGHT_DESC& LightDesc)
{
	vector<LIGHT_DESC> SceneLights{ LightDesc };
	return Replace_SceneLights(std::move(SceneLights));
}

// 전체 후보를 검증하고 마지막 swap에서만 현재 목록을 바꾼다. 빈 vector는 장면 빛을 비우는 유효한 요청이다.
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

// Renderer와 portrait 조명 경로가 호출한다. shader/buffer와 지원 수신 pass를 확인한 뒤 원본 순서대로 빛을 제출한다.
HRESULT CLight_Manager::Render_Lights(
    shared_ptr<class CShader> pShader,
    shared_ptr<class CVIBuffer_Rect> pVIBuffer,
    bool_t bEnableSceneDirectionalShadow, LIGHT_RECEIVER ePassReceiver, bool_t bSourceLightMask)
{
    if (!pShader || !pVIBuffer || (ePassReceiver != LIGHT_RECEIVER::ALL &&
        ePassReceiver != LIGHT_RECEIVER::SOURCE_CHARACTER))
        return E_INVALIDARG;

    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope stageScope(profiler, "Render.Lights.StageAndSubmit");
    // portrait 조명도 이 관리자를 사용하므로 Render.Lights 바깥 호출도 같은 소유 구간에서 측정한다.
    const char* receiverScope = ePassReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER ?
        "Render.Lights.CharacterReceivers" : "Render.Lights.WorldReceivers";
    CProfilerScope receiverCpuScope(profiler, receiverScope);
    CProfilerGpuScope receiverGpuScope(profiler, receiverScope);
    const LIGHT_CAMERA_VOLUME cameraVolume(CGameInstance::Get().Get_Transform(D3DTS::VIEW),
        CGameInstance::Get().Get_Transform(D3DTS::PROJ));
    // shader 배열 크기만큼 반복 업로드한다. 이 배치 크기를 frame 전체 빛 개수 상한으로 사용하지 않는다.
    constexpr uint32_t maximumLightInstances = CPresentation_Manager::LIGHT_RENDER_BATCH_SIZE;
    // Shader_Deferred의 DEFERRED_LIGHT_INPUT과 같은 128-byte 배열 원소다. float4 여섯 개, uint4 flags, 간접광 float4 순서를 유지한다.
    struct LightInstance
    {
        float4_t direction, position, diffuse, ambient, specular, attenuation;
        uint32_t flags[4]; // 수신 대상, 정적 그림자 channel, 방향 그림자, 원본 캐릭터 지역광 범위
        float4_t sourceCharacterAmbient;
    };
    static_assert(sizeof(LightInstance) == 128u);
    std::array<LightInstance, maximumLightInstances> instances;
    std::array<LIGHT, maximumLightInstances> types;
    uint32_t count = 0u;
    // 첫 장면 방향광에만 각 그림자 역할을 배정하기 위한 호출별 상태다. 이 값은 batch flush 후에도 유지한다.
    bool_t directionalShadowConsumed = false;
    bool_t staticShadowConsumed = false;
    const bool_t sourceMask =
        bSourceLightMask && ePassReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER;

    // 준비된 record를 업로드하고 연속된 같은 종류끼리 draw한다. 성공한 뒤 count를 비우며 실패 이전 draw는 이미 제출된 상태다.
    const auto flush = [&]() -> HRESULT
    {
        if (count == 0u)
            return S_OK;
        // record/byte 계측은 업로드 시도 전에 증가한다. 뒤의 API 실패에도 증가분을 되돌리지 않는다.
        CProfilerScope submitScope(profiler, "Render.Lights.UploadAndDraw");
        if (profiler)
        {
            profiler->Add_Counter(EProfilerCounter::LightRecords, count);
            profiler->Add_Counter(EProfilerCounter::LightUploadBytes, sizeof(LightInstance) * count);
        }
        if (FAILED(pShader->Bind_RawValue("g_LightInstances", instances.data(),
            sizeof(LightInstance) * count)))
            return E_FAIL;
        // 원본 순서와 연속된 빛 종류를 유지한다. batch 경계가 생겨도 순서를 바꾸지 않는다.
        for (uint32_t first = 0u; first < count;)
        {
            uint32_t end = first + 1u;
            while (end < count && types[end] == types[first])
                ++end;
            // Shader_Deferred의 22~24는 일반 instanced pass, 25~27은 같은 순서의 source mask pass다.
            const uint32_t typePass = types[first] == LIGHT::DIRECTIONAL ? 0u :
                types[first] == LIGHT::POINT ? 1u : 2u;
            const uint32_t pass = 22u + typePass + (sourceMask ? 3u : 0u);
            if (FAILED(pShader->Bind_RawValue("g_LightInstanceOffset", &first, sizeof(first))) ||
                FAILED(pShader->Begin(pass)) ||
                FAILED(pVIBuffer->Render_Instanced(end - first)))
                return E_FAIL;
            if (profiler)
                profiler->Add_Counter(EProfilerCounter::LightDrawCalls);
            first = end;
        }
        count = 0u;
        return S_OK;
    };

    // 수신 대상/입력/카메라 조건을 확인하고 record 하나를 준비한다. 꽉 찬 batch는 다음 record를 쓰기 전에 flush한다.
    const auto stage = [&](const LIGHT_DESC& light, bool_t scene) -> HRESULT
    {
        // 일반 pass에서는 SOURCE_CHARACTER 전용 빛을 먼저 생략한다. 나머지 수신 정책은 record의 flags도 shader에 전달한다.
        if (light.eReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER &&
            ePassReceiver == LIGHT_RECEIVER::ALL)
            return S_OK;
        if (!CLight::Is_ValidDesc(light))
            return E_INVALIDARG;
        if (light.eType != LIGHT::DIRECTIONAL)
        {
            if (profiler)
                profiler->Add_Counter(EProfilerCounter::LightCullingCandidates);
            // 이번 deferred 제출에서만 제외한다. forward 소비자가 읽는 원본 빛 목록은 그대로 유지한다.
            if (!cameraVolume.Intersects(light))
            {
                if (profiler)
                    profiler->Add_Counter(EProfilerCounter::LightCullingRejected);
                return S_OK;
            }
        }
        if (count == maximumLightInstances && FAILED(flush()))
            return E_FAIL;
        const bool_t directional = light.eType == LIGHT::DIRECTIONAL;
        // 동적 방향 그림자는 활성 요청을, 정적 기본 channel은 장면의 첫 방향광 여부를 각각 적용한다.
        const bool_t shadow = scene && directional && bEnableSceneDirectionalShadow && !directionalShadowConsumed;
        const bool_t staticShadow = scene && directional && !staticShadowConsumed;
        auto& record = instances[count];
        record.direction = light.vDirection;
        record.position = light.vPosition;
        record.diffuse = light.vDiffuse;
        record.ambient = light.vAmbient;
        // 원본 캐릭터 간접광은 scene 설명에서만 소비한다. transient 설명의 해당 성분은 여기서 0으로 보낸다.
        record.sourceCharacterAmbient = scene ? light.vSourceCharacterAmbient : float4_t{};
        record.specular = light.vSpecular;
        record.attenuation = float4_t(light.fRange, light.fFalloffExponent,
            light.fSpotInnerCos, light.fSpotOuterCos);
        record.flags[0] = static_cast<uint32_t>(light.eReceiver);
        record.flags[1] = light.staticShadowChannel != 0u ? light.staticShadowChannel : (staticShadow ? 1u : 0u);
        record.flags[2] = shadow ? 1u : 0u;
        record.flags[3] = !directional && ePassReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER ? 1u : 0u;
        types[count++] = light.eType;
        if (shadow)
            directionalShadowConsumed = true;
        if (staticShadow)
            staticShadowConsumed = true;
        return S_OK;
    };

    // 장면→transient 순서를 보존하며 마지막 미완료 batch도 제출한다. 원본 목록을 정렬하거나 제거하지 않는다.
    for (const auto& light : m_SceneLights)
        if (FAILED(stage(light, true)))
            return E_FAIL;
    for (const auto& light : CPresentation_Manager::Get().Get_TransientLights())
        if (FAILED(stage(light, false)))
            return E_FAIL;
    return flush();
}

// 빈 목록의 소유자를 만들고 초기 조명 값은 장면이 전달하게 한다.
unique_ptr<CLight_Manager> CLight_Manager::Create()
{
    return unique_ptr<CLight_Manager>(new CLight_Manager());
}
