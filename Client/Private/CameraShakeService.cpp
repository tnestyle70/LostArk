#include "CameraShakeService.h"

#include <cmath>
#include <cstdlib>

namespace
{
	constexpr size_t MAX_ACTIVE_SHAKES = 8u;

	bool Parse_Number(const std::string_view Text, f32_t& Out)
	{
		if (Text.empty())
			return false;
		const std::string Owned(Text);
		char* pEnd = nullptr;
		Out = std::strtof(Owned.c_str(), &pEnd);
		return pEnd == Owned.c_str() + Owned.size() && std::isfinite(Out);
	}

	bool Parse_Oscillator(
		const std::string_view Text,
		Client::CAMERA_SHAKE_OSCILLATOR& Out)
	{
		const size_t Comma = Text.find(',');
		if (std::string_view::npos == Comma)
			return false;
		return Parse_Number(Text.substr(0u, Comma), Out.fAmplitude) &&
			Parse_Number(Text.substr(Comma + 1u), Out.fFrequency) &&
			Out.fFrequency >= 0.f;
	}

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

std::vector<Client::CCameraShakeService::INSTANCE>
	Client::CCameraShakeService::s_Instances;

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
	size_t Cursor = 0u;
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

bool_t Client::CCameraShakeService::Evaluate(
	const CAMERA_SHAKE_SPEC& Spec,
	const f32_t fElapsedSeconds,
	CAMERA_SHAKE_SAMPLE& OutSample)
{
	OutSample = {};
	if (!std::isfinite(fElapsedSeconds) || fElapsedSeconds < 0.f ||
		Spec.fDurationSeconds <= 0.f ||
		fElapsedSeconds >= Spec.fDurationSeconds)
	{
		return false;
	}
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

void Client::CCameraShakeService::Trigger(
	const CAMERA_SHAKE_SPEC& Spec,
	const f32_t fInitialElapsedSeconds)
{
	if (!std::isfinite(fInitialElapsedSeconds) || fInitialElapsedSeconds < 0.f ||
		Spec.fDurationSeconds <= 0.f ||
		fInitialElapsedSeconds >= Spec.fDurationSeconds)
	{
		return;
	}
	if (s_Instances.empty())
		s_Instances.reserve(MAX_ACTIVE_SHAKES);
	if (s_Instances.size() >= MAX_ACTIVE_SHAKES)
		s_Instances.erase(s_Instances.begin());
	s_Instances.push_back({ Spec, fInitialElapsedSeconds });
}

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

void Client::CCameraShakeService::Clear()
{
	s_Instances.clear();
}
