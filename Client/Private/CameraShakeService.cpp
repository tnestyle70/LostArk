// SHAKE payload 검증, 진동 등록·만료와 카메라 적용 전 합산 샘플을 구현한다.
#include "CameraShakeService.h"

// 입력의 유한성을 검사하고 rad/s 주파수와 경과 초에서 sin 진동을 계산한다.
#include <cmath>
// 종결자가 있는 임시 문자열을 strtof로 읽고 끝까지 소비했는지 확인한다.
#include <cstdlib>

namespace
{
	// 동시 진동 수를 8개로 제한한다. 추가 등록 시 가장 먼저 등록한 항목부터 버린다.
	constexpr size_t MAX_ACTIVE_SHAKES = 8u;

	// 종결자가 있는 복사 문자열로 strtof를 호출하고 끝까지 읽었는지/유한한지 확인한다.
	// false라도 Out에 변환값이 쓰일 수 있지만 외부 Parse_PayloadSpec은 임시 Spec만 이 함수에 넘긴다.
	bool Parse_Number(const std::string_view Text, f32_t& Out)
	{
		if (Text.empty())
			return false;
		const std::string Owned(Text);
		char* pEnd = nullptr;
		Out = std::strtof(Owned.c_str(), &pEnd);
		return pEnd == Owned.c_str() + Owned.size() && std::isfinite(Out);
	}

	// amplitude,frequency 두 값을 읽는다. 음수 frequency는 양수로 바꾸고 amplitude 부호도 반전해 같은 sin 결과를 유지한다.
	bool Parse_Oscillator(
		const std::string_view Text,
		Client::CAMERA_SHAKE_OSCILLATOR& Out)
	{
		const size_t Comma = Text.find(',');
		if (std::string_view::npos == Comma)
			return false;
		if (!Parse_Number(Text.substr(0u, Comma), Out.fAmplitude) ||
			!Parse_Number(Text.substr(Comma + 1u), Out.fFrequency))
			return false;
		if (Out.fFrequency < 0.f)
		{
			Out.fFrequency = -Out.fFrequency;
			Out.fAmplitude = -Out.fAmplitude;
		}
		return true;
	}

	// 위상은 등록 시각 기준 elapsed 초에서 계산한다. 주파수는 Hz가 아니라 rad/s라 2*pi를 추가하지 않는다.
	f32_t Oscillate(
		const Client::CAMERA_SHAKE_OSCILLATOR& Oscillator,
		const f32_t fElapsedSeconds)
	{
		if (0.f == Oscillator.fAmplitude)
			return 0.f;
		return Oscillator.fAmplitude *
			std::sin(Oscillator.fFrequency * fElapsedSeconds);
	}
}

// 카메라별 객체가 아닌 process 공유 저장소다. 각 항목은 Spec과 현재 경과 시간을 값으로 소유한다.
std::vector<Client::CCameraShakeService::INSTANCE>
	Client::CCameraShakeService::s_Instances;

// cue 문서와 ActorCatalog가 사용하는 payload parser다. 필수 7개 키와 선택 stop만 허용하고 중복을 거부한다.
// 임시 Spec을 마지막에 대입하므로 명시적 false는 이전 OutSpec을 보존한다. 성공이 기존 status를 지우지는 않는다.
bool_t Client::CCameraShakeService::Parse_PayloadSpec(
	const std::string_view Payload,
	CAMERA_SHAKE_SPEC& OutSpec,
	std::string& strOutStatus)
{
	CAMERA_SHAKE_SPEC Staged;
	bool bHasDuration = false;
	bool bHasBlendIn = false;
	bool bHasBlendOut = false;
	bool bHasForward = false;
	bool bHasRight = false;
	bool bHasUp = false;
	bool bHasFov = false;
	bool bHasStop = false;
	size_t Cursor = 0u;
	// 세미콜론으로 필드를 나눈다. 마지막 세미콜론 뒤의 빈 필드도 검사하므로 빈 payload/끝 세미콜론은 거부된다.
	while (Cursor <= Payload.size())
	{
		size_t End = Payload.find(';', Cursor);
		if (std::string_view::npos == End)
			End = Payload.size();
		const std::string_view Field = Payload.substr(Cursor, End - Cursor);
		Cursor = End + 1u;
		const size_t Equal = Field.find('=');
		if (std::string_view::npos == Equal || 0u == Equal)
		{
			strOutStatus = "SHAKE payload field is not key=value: " +
				std::string(Field);
			return false;
		}
		const std::string_view Key = Field.substr(0u, Equal);
		const std::string_view Value = Field.substr(Equal + 1u);
		bool bDuplicate = false;
		bool bParsed = false;
		if ("dur" == Key)
		{
			bDuplicate = bHasDuration;
			bHasDuration = true;
			bParsed = Parse_Number(Value, Staged.fDurationSeconds) &&
				Staged.fDurationSeconds > 0.f;
		}
		else if ("stop" == Key)
		{
			bDuplicate = bHasStop;
			bHasStop = true;
			bParsed = Parse_Number(Value, Staged.fStopAfterSeconds) &&
				Staged.fStopAfterSeconds > 0.f;
		}
		else if ("in" == Key)
		{
			bDuplicate = bHasBlendIn;
			bHasBlendIn = true;
			bParsed = Parse_Number(Value, Staged.fBlendInSeconds) &&
				Staged.fBlendInSeconds >= 0.f;
		}
		else if ("out" == Key)
		{
			bDuplicate = bHasBlendOut;
			bHasBlendOut = true;
			bParsed = Parse_Number(Value, Staged.fBlendOutSeconds) &&
				Staged.fBlendOutSeconds >= 0.f;
		}
		else if ("x" == Key)
		{
			bDuplicate = bHasForward;
			bHasForward = true;
			bParsed = Parse_Oscillator(Value, Staged.Forward);
		}
		else if ("y" == Key)
		{
			bDuplicate = bHasRight;
			bHasRight = true;
			bParsed = Parse_Oscillator(Value, Staged.Right);
		}
		else if ("z" == Key)
		{
			bDuplicate = bHasUp;
			bHasUp = true;
			bParsed = Parse_Oscillator(Value, Staged.Up);
		}
		else if ("fov" == Key)
		{
			bDuplicate = bHasFov;
			bHasFov = true;
			bParsed = Parse_Oscillator(Value, Staged.Fov);
		}
		else
		{
			strOutStatus = "SHAKE payload has an unknown key: " +
				std::string(Key);
			return false;
		}
		if (bDuplicate)
		{
			strOutStatus = "SHAKE payload repeats key: " + std::string(Key);
			return false;
		}
		if (!bParsed)
		{
			strOutStatus = "SHAKE payload value is invalid for key: " +
				std::string(Key);
			return false;
		}
	}
	if (!bHasDuration || !bHasBlendIn || !bHasBlendOut || !bHasForward ||
		!bHasRight || !bHasUp || !bHasFov)
	{
		strOutStatus = "SHAKE payload is missing a required key.";
		return false;
	}
	OutSpec = Staged;
	return true;
}

// elapsed가 자연 duration 또는 선택 stop에 도달하면 종료한다. 출력은 먼저 비운다.
// Spec의 진폭/주파수 등 전체 유효성은 Parse 등 입력 계층 책임이며 이 함수는 완전한 재검증을 하지 않는다.
bool_t Client::CCameraShakeService::Evaluate(
	const CAMERA_SHAKE_SPEC& Spec,
	const f32_t fElapsedSeconds,
	CAMERA_SHAKE_SAMPLE& OutSample)
{
	OutSample = {};
	if (!std::isfinite(fElapsedSeconds) || fElapsedSeconds < 0.f ||
		Spec.fDurationSeconds <= 0.f ||
		fElapsedSeconds >= Spec.fDurationSeconds ||
		(Spec.fStopAfterSeconds > 0.f && fElapsedSeconds >= Spec.fStopAfterSeconds))
	{
		return false;
	}
	// blend-in 비율과 자연 duration 기준 blend-out 비율을 곱한다. stop이 짧아져도 blend-out의 기준 끝은 duration이다.
	f32_t fEnvelope = 1.f;
	if (Spec.fBlendInSeconds > 0.f && fElapsedSeconds < Spec.fBlendInSeconds)
		fEnvelope *= fElapsedSeconds / Spec.fBlendInSeconds;
	const f32_t fRemainingSeconds = Spec.fDurationSeconds - fElapsedSeconds;
	if (Spec.fBlendOutSeconds > 0.f && fRemainingSeconds < Spec.fBlendOutSeconds)
		fEnvelope *= fRemainingSeconds / Spec.fBlendOutSeconds;
	OutSample.fForward = Oscillate(Spec.Forward, fElapsedSeconds) * fEnvelope;
	OutSample.fRight = Oscillate(Spec.Right, fElapsedSeconds) * fEnvelope;
	OutSample.fUp = Oscillate(Spec.Up, fElapsedSeconds) * fEnvelope;
	OutSample.fFovDeltaDegrees = Oscillate(Spec.Fov, fElapsedSeconds) * fEnvelope;
	return true;
}

// Character/Valtan의 cue 발생이 호출한다. 늦게 도착한 cue는 전달 elapsed에서 시작하고 이미 끝났으면 목록을 바꾸지 않는다.
void Client::CCameraShakeService::Trigger(
	const CAMERA_SHAKE_SPEC& Spec,
	const f32_t fInitialElapsedSeconds)
{
	if (!std::isfinite(fInitialElapsedSeconds) || fInitialElapsedSeconds < 0.f ||
		Spec.fDurationSeconds <= 0.f ||
		fInitialElapsedSeconds >= Spec.fDurationSeconds ||
		(Spec.fStopAfterSeconds > 0.f && fInitialElapsedSeconds >= Spec.fStopAfterSeconds))
	{
		return;
	}
	if (s_Instances.empty())
		s_Instances.reserve(MAX_ACTIVE_SHAKES);
	if (s_Instances.size() >= MAX_ACTIVE_SHAKES)
		s_Instances.erase(s_Instances.begin());
	s_Instances.push_back({ Spec, fInitialElapsedSeconds });
}

// Camera_Free가 보이는 흔들림을 적용하거나 자유/연출 카메라에서 소비만 할 때 호출한다.
// 각 항목은 이번 delta를 먼저 더한 시각으로 표본화하고 남은 항목만 합산한다.
bool_t Client::CCameraShakeService::Sample(
	const f32_t fTimeDelta,
	CAMERA_SHAKE_SAMPLE& OutSample)
{
	OutSample = {};
	const f32_t fStep =
		std::isfinite(fTimeDelta) && fTimeDelta > 0.f ? fTimeDelta : 0.f;
	bool_t bActive = false;
	for (size_t i = 0u; i < s_Instances.size();)
	{
		INSTANCE& Instance = s_Instances[i];
		Instance.fElapsedSeconds += fStep;
		CAMERA_SHAKE_SAMPLE Part;
		if (!Evaluate(Instance.Spec, Instance.fElapsedSeconds, Part))
		{
			// 지운 다음 항목이 같은 index로 당겨지므로 이 경로에서는 i를 증가시키지 않는다.
			s_Instances.erase(s_Instances.begin() + i);
			continue;
		}
		OutSample.fForward += Part.fForward;
		OutSample.fRight += Part.fRight;
		OutSample.fUp += Part.fUp;
		OutSample.fFovDeltaDegrees += Part.fFovDeltaDegrees;
		bActive = true;
		++i;
	}
	return bActive;
}

// 카메라 초기화 등이 활성 목록만 비운다. 카메라 offset/FOV 원상복구는 소비자에게 남는다.
void Client::CCameraShakeService::Clear()
{
	s_Instances.clear();
}
