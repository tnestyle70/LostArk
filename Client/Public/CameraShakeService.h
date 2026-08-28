#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

/* amplitude * sin(frequency * t). Frequency is radians per second; amplitude
   keeps the authored source unit (cm for translation, degrees for FOV). */
struct CAMERA_SHAKE_OSCILLATOR final
{
	f32_t fAmplitude = 0.f;
	f32_t fFrequency = 0.f;
};

/* One SHAKE payload: "dur=..;in=..;out=..;x=a,f;y=a,f;z=a,f;fov=a,f".
   x/y/z are camera-local forward/right/up. */
struct CAMERA_SHAKE_SPEC final
{
	f32_t fDurationSeconds = 0.f;
	f32_t fBlendInSeconds = 0.f;
	f32_t fBlendOutSeconds = 0.f;
	CAMERA_SHAKE_OSCILLATOR Forward;
	CAMERA_SHAKE_OSCILLATOR Right;
	CAMERA_SHAKE_OSCILLATOR Up;
	CAMERA_SHAKE_OSCILLATOR Fov;
};

struct CAMERA_SHAKE_SAMPLE final
{
	f32_t fForward = 0.f;
	f32_t fRight = 0.f;
	f32_t fUp = 0.f;
	f32_t fFovDeltaDegrees = 0.f;
};

/* Character presentation triggers; the gameplay follow camera samples once per
   frame and applies. No CGameInstance access, so the harness compiles it alone. */
class CCameraShakeService final
{
public:
	static bool_t Parse_PayloadSpec(
		std::string_view Payload,
		CAMERA_SHAKE_SPEC& OutSpec,
		std::string& strOutStatus);
	static bool_t Evaluate(
		const CAMERA_SHAKE_SPEC& Spec,
		f32_t fElapsedSeconds,
		CAMERA_SHAKE_SAMPLE& OutSample);

	static void Trigger(
		const CAMERA_SHAKE_SPEC& Spec,
		f32_t fInitialElapsedSeconds);
	static bool_t Sample(
		f32_t fTimeDelta,
		CAMERA_SHAKE_SAMPLE& OutSample);
	static void Clear();

private:
	struct INSTANCE final
	{
		CAMERA_SHAKE_SPEC Spec;
		f32_t fElapsedSeconds = 0.f;
	};
	static std::vector<INSTANCE> s_Instances;
};

NS_END
