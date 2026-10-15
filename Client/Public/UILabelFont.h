#pragma once

// Client 이름 공간과 기존 공통 선언을 사용한다.
#include "Client_Defines.h"
// 폰트 tag·좌표·색에 쓰는 wstring_t, float2_t, fvector_t를 사용한다.
#include "Engine_Defines.h"
// 등록 여부·글자 크기를 측정하고 선택한 폰트로 Draw_Text를 호출한다.
#include "GameInstance.h"

// 크기 차이는 fabs, 실제 출력 위치는 round로 계산한다.
#include <cmath>
// family 이름과 숫자 크기를 조합해 폰트 tag를 만든다.
#include <string>
// 폰트 tag별 등록 여부를 처음 조회한 결과로 캐시한다.
#include <unordered_map>

NS_BEGIN(Client)

// UI 글자의 목표 픽셀 크기에 가까운 등록 폰트를 골라 과도한 확대·축소를 줄인다.
// MainApp::Ready_Fonts가 등록한 크기별 tag를 쓰며, 없으면 기본 family와 배율을 반환한다.
namespace UILabelFont
{
	constexpr int32_t BAKED_SIZES[] = { 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 25, 28 };
	constexpr f32_t LARGEST_BAKED = 28.f;
	constexpr f32_t SNAP_TOLERANCE_PX = 1.f;

	// 큰 제목·숫자용 Latin 전용 크기다. 한글을 포함한 문자열에는 Resolve를 사용한다.
	constexpr int32_t LATIN_DISPLAY_SIZES[] =
		{ 40, 56, 72, 96, 128, 160, 200, 256 };
	// 이 크기보다 작으면 큰 Latin 폰트 대신 일반 라벨의 선택 경로를 사용한다.
	constexpr f32_t SMALLEST_LATIN_DISPLAY = 36.f;

	// 문자 0의 측정 높이가 양수인지 한 번 확인한다. 이후 등록 변경은 이 캐시에 반영되지 않는다.
	inline bool_t Is_Registered(const wstring_t& strTag)
	{
		static std::unordered_map<wstring_t, bool_t> s_Known;
		const auto it = s_Known.find(strTag);
		if (it != s_Known.end())
			return it->second;
		const bool_t bKnown = CGameInstance::Get().Measure_Text(strTag, L"0").y > 0.f;
		s_Known.emplace(strTag, bKnown);
		return bKnown;
	}

	// 화면 픽셀 기준 줄 높이에 맞는 tag와 Draw_Text에 넘길 배율을 반환한다.
	inline wstring_t Resolve(const wstring_t& strFamilyTag, const f32_t fTargetPx, f32_t& outScale)
	{
		outScale = 1.f;
		// 작은 크기에서는 가장 가까운 사전 제작 폰트를 찾고, 1픽셀 이내면 배율 1로 사용한다.
		if (fTargetPx <= LARGEST_BAKED + SNAP_TOLERANCE_PX)
		{
			int32_t iBest = 0;
			f32_t fBestDiff = 1e9f;
			for (const int32_t iSize : BAKED_SIZES)
			{
				const f32_t fDiff = std::fabs(fTargetPx - static_cast<f32_t>(iSize));
				if (fDiff < fBestDiff) { fBestDiff = fDiff; iBest = iSize; }
			}
			const wstring_t strTag = strFamilyTag + L"_" + std::to_wstring(iBest);
			if (Is_Registered(strTag))
			{
				outScale = (fBestDiff <= SNAP_TOLERANCE_PX) ? 1.f : fTargetPx / static_cast<f32_t>(iBest);
				return strTag;
			}
		}
		// 크기별 폰트가 없으면 기본 폰트의 측정 높이로 배율을 구한다.
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFamilyTag, L"0");
		outScale = (vMeasured.y > 0.f) ? fTargetPx / vMeasured.y : 1.f;
		return strFamilyTag;
	}

	// 화면 픽셀의 중심에 글자를 맞추고 좌상단을 정수 픽셀로 반올림한다.
	// 양수 최대 폭을 넘으면 더 작은 크기를 선택하며 실제 그린 폭을 반환한다.
	inline f32_t Draw_Centered(const wstring_t& strFamilyTag, const wchar_t* pText,
		const f32_t fCenterX, const f32_t fCenterY, const f32_t fTargetPx, const fvector_t vColor,
		const f32_t fMaxScreenWidth = 0.f)
	{
		if (nullptr == pText || L'\0' == pText[0] || fTargetPx <= 0.f)
			return 0.f;
		f32_t fScale = 1.f;
		wstring_t strTag = Resolve(strFamilyTag, fTargetPx, fScale);
		float2_t vMeasured = CGameInstance::Get().Measure_Text(strTag, pText);
		if (fMaxScreenWidth > 0.f && vMeasured.x * fScale > fMaxScreenWidth)
		{
			const f32_t fFitPx = fTargetPx * fMaxScreenWidth / (vMeasured.x * fScale);
			strTag = Resolve(strFamilyTag, fFitPx, fScale);
			vMeasured = CGameInstance::Get().Measure_Text(strTag, pText);
		}
		const float2_t vTopLeft(std::round(fCenterX - vMeasured.x * fScale * 0.5f),
			std::round(fCenterY - vMeasured.y * fScale * 0.5f));
		CGameInstance::Get().Draw_Text(strTag, pText, vTopLeft, vColor, 0.f, float2_t(0.f, 0.f), fScale);
		return vMeasured.x * fScale;
	}

	// Latin 전용 큰 글자의 tag와 배율을 고른다. 미등록 또는 작은 크기는 Resolve로 처리한다.
	inline wstring_t Resolve_LatinDisplay(
		const wstring_t& strFamilyTag, const f32_t fTargetPx, f32_t& outScale)
	{
		if (fTargetPx < SMALLEST_LATIN_DISPLAY)
			return Resolve(strFamilyTag, fTargetPx, outScale);

		int32_t iBest = 0;
		f32_t fBestDiff = 1e9f;
		for (const int32_t iSize : LATIN_DISPLAY_SIZES)
		{
			const f32_t fDiff = std::fabs(fTargetPx - static_cast<f32_t>(iSize));
			if (fDiff < fBestDiff) { fBestDiff = fDiff; iBest = iSize; }
		}
		const wstring_t strTag = strFamilyTag + L"_Latin" + std::to_wstring(iBest);
		if (Is_Registered(strTag))
		{
			outScale = (fBestDiff <= SNAP_TOLERANCE_PX)
				? 1.f : fTargetPx / static_cast<f32_t>(iBest);
			return strTag;
		}
		return Resolve(strFamilyTag, fTargetPx, outScale);
	}
}

NS_END
