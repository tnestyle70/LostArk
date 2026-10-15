#pragma once

// 기존 Client 공통 선언과 WinSock2→Windows 선행 포함 순서를 유지한다.
#include "Client_Defines.h"
// 진동 값과 샘플의 f32_t·float3_t·bool_t 및 엔진 namespace 매크로를 사용한다.
#include "Engine_Defines.h"

// payload 거절 이유를 호출자에게 소유 문자열로 전달한다.
#include <string>
// 파싱 중인 SHAKE payload를 복사 없이 차용한다. 영구 보관하는 입력은 아니다.
#include <string_view>
// process 동안 활성 진동 인스턴스와 각 경과 시간을 보관한다.
#include <vector>

NS_BEGIN(Client)

// amplitude * sin(frequency * t)에서 frequency 단위는 rad/s다.
// 이동 amplitude는 저작 단위 cm, FOV amplitude는 degree를 유지한다.
struct CAMERA_SHAKE_OSCILLATOR final
{
	f32_t fAmplitude = 0.f;
	f32_t fFrequency = 0.f;
};

// SHAKE payload의 duration/blend와 x/y/z/fov 진동을 담는다.
// x/y/z는 월드 축이 아닌 카메라의 forward/right/up이며 실제 월드 변환은 Camera_Free가 수행한다.
struct CAMERA_SHAKE_SPEC final
{
	f32_t fDurationSeconds = 0.f;
	// 선택적인 notify 종료 시각(초)이다. 0이면 자연 duration/blend envelope를 유지한다.
	f32_t fStopAfterSeconds = 0.f;
	f32_t fBlendInSeconds = 0.f;
	f32_t fBlendOutSeconds = 0.f;
	CAMERA_SHAKE_OSCILLATOR Forward;
	CAMERA_SHAKE_OSCILLATOR Right;
	CAMERA_SHAKE_OSCILLATOR Up;
	CAMERA_SHAKE_OSCILLATOR Fov;
};

// 카메라 로컬 이동(cm)과 FOV 변화(degree)의 합산 출력이다. Camera_Free가 이동에 0.01을 곱해 m로 변환한다.
struct CAMERA_SHAKE_SAMPLE final
{
	f32_t fForward = 0.f;
	f32_t fRight = 0.f;
	f32_t fUp = 0.f;
	f32_t fFovDeltaDegrees = 0.f;
};

// Character/Valtan 표시가 trigger하고 Camera_Free가 프레임마다 소비하는 process 공유 흔들림이다.
// CGameInstance나 카메라 객체를 소유하지 않고 값과 경과 시간만 보관한다.
class CCameraShakeService final
{
public:
	// cue 문서/카탈로그가 key=value payload를 읽을 때 사용한다. false면 OutSpec을 보존하고 상태에 사유를 기록한다.
	// 성공 시 status를 별도로 비우지는 않는다.
	static bool_t Parse_PayloadSpec(
		std::string_view Payload,
		CAMERA_SHAKE_SPEC& OutSpec,
		std::string& strOutStatus);
	// 값 하나를 지정된 경과 시간에 표본화한다. 출력은 먼저 0으로 비우며 구간 밖이면 false다.
	// 직접 전달한 Spec의 모든 float를 다시 검증하는 함수는 아니므로 저작 검증된 값을 전달해야 한다.
	static bool_t Evaluate(
		const CAMERA_SHAKE_SPEC& Spec,
		f32_t fElapsedSeconds,
		CAMERA_SHAKE_SAMPLE& OutSample);

	// 이미 지난 시각부터 시작하도록 Spec을 복사해 등록한다. 최대 8개이며 넘으면 가장 오래 등록한 항목을 제거한다.
	static void Trigger(
		const CAMERA_SHAKE_SPEC& Spec,
		f32_t fInitialElapsedSeconds);
	// 경과 시간을 먼저 더하고 살아 있는 항목을 합산한다. 잘못된 delta는 0으로 취급하고 종료 항목은 제거한다.
	// false일 때도 출력은 0이며 목록이 정리될 수 있다. 같은 프레임 중복 호출은 시간을 중복 진행시킨다.
	static bool_t Sample(
		f32_t fTimeDelta,
		CAMERA_SHAKE_SAMPLE& OutSample);
	// 모든 활성 흔들림을 비운다. 현재 카메라 Transform에서 이미 적용한 offset을 되돌리는 일은 Camera_Free가 담당한다.
	static void Clear();

private:
	struct INSTANCE final
	{
		CAMERA_SHAKE_SPEC Spec;
		f32_t fElapsedSeconds = 0.f;
	};
	// process 전체가 공유하는 값 목록이다. 내부 lock이 없으므로 Trigger/Sample/Clear의 동시 접근은 호출자가 막아야 한다.
	static std::vector<INSTANCE> s_Instances;
};

NS_END
