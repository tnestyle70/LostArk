#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "GameInstance.h"

#include <cmath>
#include <string>
#include <unordered_map>

NS_BEGIN(Client)

/* Crisp small labels for the runtime UI windows. The shipped LOA sprite fonts are baked at
32-42 px; drawing them at 10-18 px through SpriteBatch's bilinear sampler is what made the
window text blurry. CMainApp::Ready_Fonts registers the pre-downsampled Resources/UI/Fonts
variants as "<family>_<lineSpacingPx>" (see scratch bake_small_fonts.py); Resolve picks the one
nearest the on-screen size a label wants and, when it is within a pixel, draws it at exactly
1:1 so no resampling happens at all. Sizes past the largest baked step, or a family with no
baked variants delivered, fall back to the family font scaled as before. */
namespace UILabelFont
{
	constexpr int32_t BAKED_SIZES[] = { 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 25, 28 };
	constexpr f32_t LARGEST_BAKED = 28.f;
	constexpr f32_t SNAP_TOLERANCE_PX = 1.f;

	/* Display sizes, re-rasterised from the retail TTF by
	Tools/FontPipeline/bake_spritefont.py rather than downsampled, because there is
	nothing above 42 px to downsample from -- a 110 pt title off the family atlas is
	a 3.4x magnify of a 27 px glyph. These carry Latin only: a full Korean set at
	these sizes runs to hundreds of megabytes and the labels that need them (a
	title word, a percentage) never contain Korean. Use Resolve_LatinDisplay only
	for text you know is Latin; anything else stays on Resolve. */
	constexpr int32_t LATIN_DISPLAY_SIZES[] =
		{ 40, 56, 72, 96, 128, 160, 200, 256 };
	/* Below this the family atlas (baked at 32-42 px) is the closer match than
	the smallest display step, so Resolve_LatinDisplay hands back to Resolve. */
	constexpr f32_t SMALLEST_LATIN_DISPLAY = 36.f;

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

	/* strFamilyTag: "Font_YG760" / "Font_YoonGasiIIM" (any other tag passes through).
	fTargetPx: wanted line spacing in screen pixels. outScale: the SpriteFont scale to draw
	the returned tag with. */
	inline wstring_t Resolve(const wstring_t& strFamilyTag, const f32_t fTargetPx, f32_t& outScale)
	{
		outScale = 1.f;
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
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFamilyTag, L"0");
		outScale = (vMeasured.y > 0.f) ? fTargetPx / vMeasured.y : 1.f;
		return strFamilyTag;
	}

	/* Display-size counterpart of Resolve, for Latin-only text (a title word, a
	percentage). Below the smallest display step it defers to Resolve, so a caller
	can use it for one label across every viewport size. */
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
