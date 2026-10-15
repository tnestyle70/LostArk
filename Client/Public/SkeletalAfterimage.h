#pragma once

// Client 공통 선언과 수치 타입을 사용한다.
#include "Client_Defines.h"
// MODEL_VIEW와 sample의 행렬·색 타입을 사용한다.
#include "Engine_Defines.h"
// 시간순 sample에서 가장 오래된 항목을 앞에서 제거한다.
#include <deque>
// view의 공유 자원 참조와 현재 모델의 weak 식별을 보관한다.
#include <memory>
// mesh별 본 행렬 palette를 sample의 값으로 복사한다.
#include <vector>

namespace Engine { class CModel; class CShader; }

namespace Client
{
// 이미 계산된 모델 자세·행렬을 제한된 수만큼 복사해 잠시 그리는 잔상 상태다.
// 직접 애니메이션 시간을 진행하거나 모델을 Clone하지 않으며 이동·충돌도 처리하지 않는다.
class CSkeletalAfterimage final
{
public:
    // 호출자가 고른 Model·paletteModel·Shader는 shared 참조이고 행렬·mask는 값 사본이다.
    struct MODEL_VIEW final
    {
        std::shared_ptr<Engine::CModel> model, paletteModel;
        std::shared_ptr<Engine::CShader> shader;
        float4x4_t world{};
        uint32_t hiddenMeshMask = 0u;
        bool socketed = false;
    };
    struct SETTINGS final
    {
        float sampleIntervalSeconds = .05f;
        float sampleLifetimeSeconds = .25f;
        uint32_t maxSamples = 6u;
        float4_t color{1.8f, 1.8f, 1.8f, .38f};
        bool capturePoseChanges = false;
        bool interpolateColor = false;
        float sourceColorIntensity = 0.f;
        float4_t endColor{1.f, 1.f, 1.f, 1.f};
        // sourceChannels를 켜면 color/endColor를 rim 채널로 쓰고 source alpha·ambient 값을 적용한다.
        // 기존 호출은 sourceChannels를 끈 상태의 색·투명도·초기화 규칙을 사용한다.
        bool sourceChannels = false;
        float sourceInitialAlpha = 1.f;
        float userAlphaScale = 1.f;
        float sourceAlphaHoldSeconds = 0.f;
        float4_t ambientStart{0.f, 0.f, 0.f, 0.f};
        float4_t ambientEnd{0.f, 0.f, 0.f, 0.f};
    };
    // 유효한 설정만 반영한다. 일반 설정 전환은 Reset하고 source→source 전환은 기존 sample을 보존한다.
    bool Configure(const SETTINGS& settings);
    // view를 복사해 Model·Shader 참조를 보유한다. Reset만으로 이 참조가 지워지지는 않는다.
    void Set_PresentationView(const MODEL_VIEW& view) { m_View = view; }
    // 이미 샘플된 첫 자세를 지정 age초로 추가한다. 입력/캡처 실패는 false다.
    bool Capture_Initial(const MODEL_VIEW& view, float ageSeconds);
    // sample을 노화시키고 emitting 중 간격이 지나면 현재 실제 자세 하나를 추가한다.
    void Update(float deltaSeconds, bool emitting,
        const std::shared_ptr<Engine::CModel>& model, const float4x4_t& world);
    // 표현 clock의 각 주기에서 한 자세를 표시한다. lifetime보다 주기가 길면 남은 구간은 완전히 숨긴다.
    void Sample_Pulse(float elapsedSeconds, const std::shared_ptr<Engine::CModel>& model,
        const float4x4_t& world);
    // sample·clock·weak 모델 식별을 초기화한다. 설정과 m_View 공유 참조는 유지한다.
    void Reset();
    bool Has_Samples() const { return !m_Samples.empty(); }
    // 저장 자세로 그린 뒤 현재 World와 palette/법선 입력을 복원한다. sample 없음은 S_FALSE다.
    HRESULT Render(const std::shared_ptr<Engine::CModel>& model,
        const std::shared_ptr<Engine::CShader>& shader, const float4x4_t& liveWorld);

private:
    struct SAMPLE final
    {
        float4x4_t world{};
        std::vector<std::vector<float4x4_t>> palettes;
        float ageSeconds = 0.f;
        uint32_t hiddenMeshMask = 0u;
        SETTINGS settings; // 각 발생은 당시 수명·색 설정을 값으로 보관한다.
    };
    // View는 자원을 공유 소유한다. 이 객체 소멸 또는 다음 view 대입 때 기존 공유 참조가 정리된다.
    MODEL_VIEW m_View;
    SETTINGS m_Settings;
    // sample은 행렬·palette·설정 값을 소유하고 m_Model은 모델 교체를 판별하는 weak 참조다.
    std::deque<SAMPLE> m_Samples;
    std::weak_ptr<Engine::CModel> m_Model;
    float m_Accumulator = 0.f;
    bool m_SuppressedUntilDisabled = false;
    bool m_WasEmitting = false;
    float m_PulseClockSeconds = -1.f;
    float m_PulseBirthSeconds = -1.f;
    // 현재 palette를 값으로 복사한다. socketed view는 본 palette 없이 mesh 수만 유지한다.
    bool Capture_Sample(const std::shared_ptr<Engine::CModel>& model,
        const float4x4_t& world, SAMPLE& sample);
    // sample을 비우고 반복 실패를 억제한다. 일반 Update의 emitting 해제 또는 Reset이 억제를 푼다.
    void Suppress_FailedPresentation();
};
}
