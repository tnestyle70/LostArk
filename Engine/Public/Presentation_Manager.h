#pragma once

// frame provider와 화면 합성 descriptor/실패 범위를 소비한다.
#include "PresentationProvider.h"
// 일시적인 광원 설명 LIGHT_DESC를 값으로 보관한다.
#include "Engine_RenderTypes.h"

NS_BEGIN(Engine)

// 채널별 설정 수·예상 제출 수·실제 시도와 결과다. suppressed도 정상적으로 처리한 시도에 포함한다.
struct PRESENTATION_CHANNEL_SUBMISSION_STATS final
{
	uint64_t iConfigured = 0u;
	uint64_t iExpected = 0u;
	uint64_t iAttempted = 0u;
	uint64_t iAccepted = 0u;
	uint64_t iSuppressed = 0u;
	uint64_t iFailed = 0u;
};

// 마지막 provider 제출의 집계다. completed와 committed는 GPU 렌더 성공을 뜻하지 않는다.
struct PRESENTATION_SUBMISSION_STATS final
{
	PRESENTATION_CHANNEL_SUBMISSION_STATS Lights;
	PRESENTATION_CHANNEL_SUBMISSION_STATS ScreenPosts;
	PRESENTATION_CHANNEL_SUBMISSION_STATS ScreenOverlays;
	uint64_t iProviderFailures = 0u;
	bool_t bCompleted = false;
	bool_t bCommitted = false;
};

// 렌더 프레임 동안 provider와 빛/화면 합성 후보를 소유하는 process singleton이다. 호출 직렬화는 렌더 흐름이 맡으며 내부 잠금은 없다.
class ENGINE_DLL CPresentation_Manager final
{
private:
	CPresentation_Manager();

public:
	static CPresentation_Manager& Get();
	// Shader_Deferred의 g_LightInstances 배열 크기다. draw 한 묶음의 크기이며 frame 전체 상한이 아니다.
	static constexpr uint32_t LIGHT_RENDER_BATCH_SIZE = 400u;

	// 동일 객체 주소의 중복 등록은 S_FALSE다. null 등록 실패는 기억해 다음 Submit에서 전체 실패로 처리한다.
	HRESULT Add_FrameProvider(shared_ptr<IPresentationProvider> pProvider);
	// 등록 목록을 인계하고 후보를 모은다. 격리된 provider만 취소할 수 있으며 전역 실패는 이번 전체 후보를 비운다.
	// S_FALSE도 정상 commit일 수 있다. 기능 억제 또는 격리 실패가 포함됐다는 결과다.
	HRESULT Submit_FrameProviders();
	// 각 Add는 비활성 채널이면 S_FALSE, 입력 거부면 실패, 보관 성공이면 S_OK다. 활성 제출 중에는 시도 결과를 집계한다.
	HRESULT Add_TransientLight(const LIGHT_DESC& LightDesc);
	HRESULT Add_ScreenPost(
		const PRESENTATION_SCREEN_POST_DESC& ScreenPostDesc);
	HRESULT Add_ScreenOverlay(
		const PRESENTATION_SCREEN_OVERLAY_DESC& ScreenOverlayDesc);
	// 최근 처리에서 기록한 실패 범위를 차용한다. 다음 제출/Add/검증 기록이 값을 바꿀 수 있다.
	PRESENTATION_FAILURE_SCOPE Get_LastFailureScope() const
	{
		return m_eLastFailureScope;
	}
	// provider가 설정된 수와 이번 실제 제출 예정 수를 누적한다. Submit transaction 밖에서는 기록하지 않는다.
	void Register_ProviderSubmissionExpectation(
		uint64_t iConfiguredLightCount,
		uint64_t iExpectedLightCount,
		uint64_t iConfiguredScreenPostCount,
		uint64_t iExpectedScreenPostCount);
	void Register_ProviderScreenOverlayExpectation(
		uint64_t iConfiguredScreenOverlayCount,
		uint64_t iExpectedScreenOverlayCount);
	// Add 전에 provider가 거부한 입력을 실패 시도로 기록한다. transaction 밖에서도 실패 범위 값은 바꾼다.
	void Record_TransientLightValidationFailure();
	void Record_ScreenPostValidationFailure();
	void Record_ScreenOverlayValidationFailure();
	// 내부 집계의 차용 참조다. 다음 Submit/기록 호출이 내용을 바꾸므로 영구 snapshot이면 복사해야 한다.
	const PRESENTATION_SUBMISSION_STATS& Get_LastSubmissionStats() const
	{
		return m_LastSubmissionStats;
	}
	// 이번 frame의 소유 배열을 차용한다. Add/Clear/다음 Submit 이후 원소 주소와 값의 유효성을 보장하지 않는다.
	const vector<LIGHT_DESC>& Get_TransientLights() const
	{
		return m_TransientLights;
	}
	const vector<PRESENTATION_SCREEN_POST_DESC>& Get_ScreenPosts() const
	{
		return m_ScreenPosts;
	}
	const vector<PRESENTATION_SCREEN_OVERLAY_DESC>& Get_ScreenOverlays() const
	{
		return m_ScreenOverlays;
	}
	// 각 Clear는 보관 후보만 비운다. 마지막 집계/개수/실패 범위/활성 옵션은 초기화하지 않는다.
	void Clear_TransientLights();
	void Clear_ScreenPosts();
	void Clear_ScreenOverlays();
	void Clear_Frame();
	// 다음 Add의 수락 여부를 정한다. 이미 보관한 후보를 즉시 제거하거나 저작 설정을 저장하지 않는다.
	void Set_TransientLightsEnabled(bool_t bEnabled)
	{
		m_bTransientLightsEnabled = bEnabled;
	}
	void Set_ScreenPostsEnabled(bool_t bEnabled)
	{
		m_bScreenPostsEnabled = bEnabled;
	}
	void Set_ScreenOverlaysEnabled(bool_t bEnabled)
	{
		m_bScreenOverlaysEnabled = bEnabled;
	}
	bool_t Are_TransientLightsEnabled() const
	{
		return m_bTransientLightsEnabled;
	}
	bool_t Are_ScreenPostsEnabled() const
	{
		return m_bScreenPostsEnabled;
	}
	bool_t Are_ScreenOverlaysEnabled() const
	{
		return m_bScreenOverlaysEnabled;
	}
	// 마지막 성공한 제출의 개수다. Submit 시작 때 0으로 초기화하며 Clear_Frame 뒤에도 최근 결과로 남는다.
	uint32_t Get_LastTransientLightCount() const
	{
		return m_iLastTransientLightCount;
	}
	uint32_t Get_LastScreenPostCount() const
	{
		return m_iLastScreenPostCount;
	}
	uint32_t Get_LastScreenOverlayCount() const
	{
		return m_iLastScreenOverlayCount;
	}

private:
	// 다음 Submit까지 provider 수명을 유지한다. 각 descriptor의 COM/shared_ptr도 소비가 끝날 때까지 자원을 소유한다.
	vector<shared_ptr<IPresentationProvider>> m_FrameProviders;
	vector<LIGHT_DESC> m_TransientLights;
	vector<PRESENTATION_SCREEN_POST_DESC> m_ScreenPosts;
	vector<PRESENTATION_SCREEN_OVERLAY_DESC> m_ScreenOverlays;
	bool_t m_bTransientLightsEnabled = true;
	bool_t m_bScreenPostsEnabled = true;
	bool_t m_bScreenOverlaysEnabled = true;
	bool_t m_bSubmissionTransactionActive = false;
	uint32_t m_iLastTransientLightCount = {};
	uint32_t m_iLastScreenPostCount = {};
	uint32_t m_iLastScreenOverlayCount = {};
	PRESENTATION_SUBMISSION_STATS m_LastSubmissionStats;
	PRESENTATION_FAILURE_SCOPE m_eLastFailureScope =
		PRESENTATION_FAILURE_SCOPE::NONE;
	// null provider 등록 오류를 다음 frame 제출 경계로 전달한다. Clear_Frame만으로는 이 보류 실패를 지우지 않는다.
	HRESULT m_hPendingProviderFailure = S_OK;
};

NS_END
