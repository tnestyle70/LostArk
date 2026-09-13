#include "imgui.h"
#include "Effect_Tool_Internal.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_Artist31470ShaderRegistry.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_DocumentRenderer.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_RuntimeAuthority.h"
#include "Effect_VisualProgramCorpus.h"
#include "Logic_DimensionMaster.h"
#include "MapEffectPresentationRuntime.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <set>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "Model.h"
#include "Transform.h"

bool_t Client::CEffect_Tool::Render_ProjectTunedSurfaceParameters(
	EFFECT_ELEMENT_DESC& Element,
	bool_t& bChanged)
{
	constexpr uint32_t WATER_DROPLET_BURST_OPCODE = 1003u;
	constexpr uint32_t GLASS_MESH_OPCODE = 1004u;
	const EFFECT_MATERIAL_EXECUTION_DESC& Execution =
		Element.Material.Execution;
	const bool_t bReservedProjectTunedOpcode =
		Execution.iOpcode == WATER_DROPLET_BURST_OPCODE ||
		Execution.iOpcode == GLASS_MESH_OPCODE;
	if (!bReservedProjectTunedOpcode)
		return false;
	const bool_t bProjectTunedRuntimeV2 = Execution.bEnabled &&
		Execution.eFidelity ==
			EFFECT_MATERIAL_EXECUTION_FIDELITY::PROJECT_TUNED_APPROX &&
		Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2;
	/* These two packets are deliberately edited by semantic name. Packed index
	   remains transport ABI and never becomes an artist-facing control. Returning
	   true even for a malformed packet keeps the generic +/-100000 editor hidden. */
	struct SCALAR_CONTROL_DESC final
	{
		std::string_view strName;
		std::string_view strLabel;
		std::string_view strTooltip;
		f32_t fMinimum = 0.f;
		f32_t fMaximum = 1.f;
		f32_t fStep = 0.01f;
		const char* pFormat = "%.3f";
	};
	struct VECTOR_CONTROL_DESC final
	{
		std::string_view strName;
		std::string_view strLabel;
		std::string_view strTooltip;
		f32_t fRgbMaximum = 1.f;
		bool_t bEditAlpha = false;
	};

	static constexpr std::array<std::string_view, 16u> WATER_SCALAR_NAMES = {{
		"water.noise-tiling", "water.pan-x", "water.pan-y",
		"water.second-octave-scale", "water.flow-warp",
		"water.mask-threshold", "water.edge-softness", "water.rim-width",
		"water.coverage-power", "water.body-strength",
		"water.rim-strength", "water.distortion-strength",
		"water.alpha-gain", "water.fade-start-seconds",
		"water.fade-end-seconds", "water.card-feather"
	}};
	static constexpr std::array<std::string_view, 2u> WATER_VECTOR_NAMES = {{
		"water.body-color", "water.rim-color"
	}};
	static constexpr std::array<std::array<f32_t, 2u>, 16u>
		WATER_SCALAR_BOUNDS = {{
		{{ 0.01f, 16.f }}, {{ -8.f, 8.f }}, {{ -8.f, 8.f }},
		{{ 0.5f, 8.f }}, {{ 0.f, 0.25f }}, {{ 0.f, 1.f }},
		{{ 0.1f, 8.f }}, {{ 0.001f, 0.5f }}, {{ 0.1f, 4.f }},
		{{ 0.f, 8.f }}, {{ 0.f, 8.f }}, {{ -0.025f, 0.025f }},
		{{ 0.f, 4.f }}, {{ 0.f, 5.f }}, {{ 0.001f, 5.f }},
		{{ 0.001f, 0.49f }}
	}};
	static constexpr std::array<SCALAR_CONTROL_DESC, 10u> WATER_CONTROLS = {{
		{ "water.mask-threshold", "Coverage Cutoff",
			"Fluid-mask cutoff. Lower values retain more of the droplet card.",
			0.f, 1.f, 0.001f, "%.3f" },
		{ "water.edge-softness", "Edge AA Width",
			"Multiplies derivative-based mask antialiasing; this is not blur radius.",
			0.1f, 8.f, 0.01f, "%.3f" },
		{ "water.rim-width", "Fluid Rim Width",
			"Distance between the outer fluid threshold and the bright inner rim.",
			0.001f, 0.5f, 0.001f, "%.3f" },
		{ "water.coverage-power", "Body Coverage Power",
			"Shapes body coverage after the texture mask without changing the card silhouette.",
			0.1f, 4.f, 0.01f, "%.3f" },
		{ "water.body-strength", "Body Radiance",
			"Linear-HDR strength of the droplet body.",
			0.f, 8.f, 0.01f, "%.3f" },
		{ "water.rim-strength", "Rim Radiance",
			"Linear-HDR strength of the texture and spherical Fresnel rim.",
			0.f, 8.f, 0.01f, "%.3f" },
		{ "water.flow-warp", "Flow Warp (UV)",
			"Maximum signed-flow displacement applied before sampling the fluid mask.",
			0.f, 0.25f, 0.001f, "%.4f" },
		{ "water.distortion-strength", "Refraction / Distortion (UV)",
			"Signed RT1 distortion strength. Zero disables scene refraction.",
			-0.025f, 0.025f, 0.0001f, "%.5f" },
		{ "water.fade-start-seconds", "Surface Fade Start (s)",
			"Effect-local time at which the project-tuned surface begins fading.",
			0.f, 5.f, 0.001f, "%.4f" },
		{ "water.fade-end-seconds", "Surface Fade End (s)",
			"Effect-local time at which the project-tuned surface reaches zero.",
			0.f, 5.f, 0.001f, "%.4f" }
	}};
	static constexpr std::array<VECTOR_CONTROL_DESC, 2u> WATER_COLORS = {{
		{ "water.body-color", "Body Tint (Linear HDR)",
			"Linear RGB 0..4; alpha multiplies body radiance (0..1).",
			4.f, true },
		{ "water.rim-color", "Rim Tint (Linear HDR)",
			"Linear RGB 0..8; alpha multiplies rim radiance (0..1).",
			8.f, true }
	}};

	static constexpr std::array<std::string_view, 8u> GLASS_SCALAR_NAMES = {{
		"CoverageGain", "BodyOpacity", "FresnelPower", "EdgeGain",
		"CrackGain", "RefractionStrength", "DistortionClamp", "EmissionGain"
	}};
	static constexpr std::array<std::string_view, 2u> GLASS_VECTOR_NAMES = {{
		"BodyTintLinear", "EdgeTintLinear"
	}};
	static constexpr std::array<std::array<f32_t, 2u>, 8u>
		GLASS_SCALAR_BOUNDS = {{
		{{ 0.f, 4.f }}, {{ 0.f, 1.f }}, {{ 0.25f, 16.f }},
		{{ 0.f, 8.f }}, {{ 0.f, 4.f }}, {{ -0.025f, 0.025f }},
		{{ 0.f, 0.025f }}, {{ 0.f, 8.f }}
	}};
	static constexpr std::array<SCALAR_CONTROL_DESC, 8u> GLASS_CONTROLS = {{
		{ "CoverageGain", "Coverage Gain",
			"Amplifies mesh coverage before the final bounded opacity calculation.",
			0.f, 4.f, 0.01f, "%.3f" },
		{ "BodyOpacity", "Body Opacity",
			"Linear opacity of the glass body before Fresnel and crack accents.",
			0.f, 1.f, 0.005f, "%.3f" },
		{ "FresnelPower", "Fresnel Power",
			"Higher values make the view-angle edge band narrower.",
			0.25f, 16.f, 0.05f, "%.3f" },
		{ "EdgeGain", "Edge Gain",
			"Strength of the view-angle glass edge.",
			0.f, 8.f, 0.01f, "%.3f" },
		{ "CrackGain", "Crack Gain",
			"Strength of crack/coverage detail sampled from the glass texture lane.",
			0.f, 4.f, 0.01f, "%.3f" },
		{ "RefractionStrength", "Refraction Strength (UV)",
			"Signed multiplier for the raw RT1 screen-distortion direction.",
			-0.025f, 0.025f, 0.0001f, "%.5f" },
		{ "DistortionClamp", "Maximum Distortion (UV)",
			"Absolute RT1 clamp applied after Refraction Strength.",
			0.f, 0.025f, 0.0001f, "%.5f" },
		{ "EmissionGain", "Emission Gain",
			"Linear-HDR emission multiplier for the glass body and edge.",
			0.f, 8.f, 0.01f, "%.3f" }
	}};
	static constexpr std::array<VECTOR_CONTROL_DESC, 2u> GLASS_COLORS = {{
		{ "BodyTintLinear", "Body Tint (Linear HDR)",
			"Linear RGB 0..4; alpha is bounded to 0..1.", 4.f, true },
		{ "EdgeTintLinear", "Edge Tint (Linear HDR)",
			"Linear RGB 0..8; alpha is bounded to 0..1.", 8.f, true }
	}};

	const auto MatchesOrderedScalars = [&Execution](const auto& Names,
		const size_t iCount)
	{
		if (Execution.iScalarCount != iCount ||
			Execution.Scalars.size() != iCount || iCount > Names.size())
		{
			return false;
		}
		for (size_t i = 0u; i < iCount; ++i)
		{
			if (Execution.Scalars[i].strName != Names[i] ||
				Execution.Scalars[i].iPackedIndex != i)
			{
				return false;
			}
		}
		return true;
	};
	const auto MatchesOrderedVectors = [&Execution](const auto& Names)
	{
		if (Execution.iVectorCount != Names.size() ||
			Execution.Vectors.size() != Names.size())
		{
			return false;
		}
		for (size_t i = 0u; i < Names.size(); ++i)
		{
			if (Execution.Vectors[i].strName != Names[i] ||
				Execution.Vectors[i].iPackedIndex != i)
			{
				return false;
			}
		}
		return true;
	};
	const auto MatchesScalarBounds = [&Execution](const auto& Bounds)
	{
		if (Execution.Scalars.size() != Bounds.size())
			return false;
		for (size_t i = 0u; i < Bounds.size(); ++i)
		{
			const f32_t fValue = Execution.Scalars[i].fValue;
			if (!std::isfinite(fValue) || fValue < Bounds[i][0u] ||
				fValue > Bounds[i][1u])
			{
				return false;
			}
		}
		return true;
	};
	const auto MatchesVectorBounds = [&Execution](
		const std::array<f32_t, 2u>& RgbMaximums)
	{
		if (Execution.Vectors.size() != RgbMaximums.size())
			return false;
		for (size_t i = 0u; i < RgbMaximums.size(); ++i)
		{
			const float4_t Value = Execution.Vectors[i].vValue;
			if (!std::isfinite(Value.x) || !std::isfinite(Value.y) ||
				!std::isfinite(Value.z) || !std::isfinite(Value.w) ||
				Value.x < 0.f || Value.y < 0.f || Value.z < 0.f ||
				Value.x > RgbMaximums[i] || Value.y > RgbMaximums[i] ||
				Value.z > RgbMaximums[i] || Value.w < 0.f || Value.w > 1.f)
			{
				return false;
			}
		}
		return true;
	};
	const bool_t bWater = Execution.iOpcode == WATER_DROPLET_BURST_OPCODE;
	const bool_t bGlass = Execution.iOpcode == GLASS_MESH_OPCODE;
	const size_t iGlassScalarCount = Execution.Scalars.size();
	const bool_t bPacketEnvelopeValid = bProjectTunedRuntimeV2 &&
		!Execution.bFailClosed && !Execution.bAuthoringApproximate &&
		Execution.iVersion == 1u && Execution.iPassIndex == 1u &&
		Execution.strRasterizerState == "RS_Cull_None" &&
		Execution.strDepthStencilState == "DSS_ReadOnly" &&
		Execution.strBlendState == "BS_EffectAlpha" &&
		Execution.iStencilReference == 0u &&
		Execution.iDynamicConsumedMask == 0u &&
		Execution.iDynamicSuppressedMask == 0x0fu &&
		Execution.iParticleColorPolicy == 2u &&
		Execution.iParticleColorConsumedMask == 0x08u &&
		Execution.iParticleColorSuppressedMask == 0x07u &&
		Execution.iStaticInputCount == 0u &&
		Execution.iStaticSelectedMask == 0u &&
		Execution.iStaticConsumedMask == 0u &&
		Execution.iStaticSuppressedMask == 0u &&
		Execution.iRenderInputCount == 6u &&
		Execution.iRenderConsumedMask == 0x2fu &&
		Execution.iRenderSuppressedMask == 0x10u &&
		Execution.ArtistParameters.empty() && Execution.Colors.empty();
	const bool_t bWaterPacketShape = bWater &&
		Execution.iTextureLaneCount == 2u && Execution.iTextureMask == 0x03u &&
		Execution.TextureLanes.size() == 2u && Execution.iInputCount == 16u &&
		Execution.InputConsumedMask == std::array<uint32_t, 2u>{ 0xffffu, 0u };
	const bool_t bGlassPacketShape = bGlass &&
		Execution.iTextureLaneCount == 1u && Execution.iTextureMask == 0x01u &&
		Execution.TextureLanes.size() == 1u && Execution.iInputCount == 8u &&
		Execution.InputConsumedMask == std::array<uint32_t, 2u>{ 0xffu, 0u };
	const bool_t bCarrierValid =
		Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
		!Element.Material.SourceMaterial.bEnabled &&
		Element.Material.eRenderProfile ==
			EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ &&
		(!bGlass || Resolve_AuthoringFamily(Element) ==
			EFFECT_AUTHORING_FAMILY::MESH_PARTICLE);
	const bool_t bOccurrenceValid =
		(bWater && Element.strElementId ==
			"project-tuned.water-burst.2050230.01") ||
		(bGlass && Element.strElementId ==
			"project-tuned.glass-mirror-shards.2050230.01");
	const bool_t bSemanticAbiValid = bPacketEnvelopeValid &&
		bCarrierValid && bOccurrenceValid &&
		((bWater && MatchesOrderedScalars(
			WATER_SCALAR_NAMES, WATER_SCALAR_NAMES.size()) &&
			MatchesOrderedVectors(WATER_VECTOR_NAMES) &&
			MatchesScalarBounds(WATER_SCALAR_BOUNDS) &&
			MatchesVectorBounds({ 4.f, 8.f }) && bWaterPacketShape &&
			Execution.Scalars[14u].fValue > Execution.Scalars[13u].fValue) ||
		 (bGlass && iGlassScalarCount == GLASS_SCALAR_NAMES.size() &&
			MatchesOrderedScalars(GLASS_SCALAR_NAMES, iGlassScalarCount) &&
			MatchesOrderedVectors(GLASS_VECTOR_NAMES) &&
			MatchesScalarBounds(GLASS_SCALAR_BOUNDS) &&
			MatchesVectorBounds({ 4.f, 8.f }) && bGlassPacketShape));

	if (!ImGui::CollapsingHeader(
			"Project Tuned Surface###MaterialParameters",
			ImGuiTreeNodeFlags_DefaultOpen))
	{
		return true;
	}
	ImGui::TextDisabled(
		"Semantic controls only. Packed indices/registers stay hidden and Apply stages the complete typed packet.");
	if (!bSemanticAbiValid)
	{
		ImGui::TextColored(ImVec4(1.f, 0.45f, 0.25f, 1.f),
			"Surface controls locked: opcode %u carrier/name/index ABI is incomplete or changed.",
			Execution.iOpcode);
		ImGui::TextDisabled(
			"The raw Material Parameters fallback remains hidden so a malformed typed packet cannot be edited ambiguously.");
		return true;
	}

	const auto FindScalar = [&Element](const std::string_view strName)
		-> const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC*
	{
		const auto Found = std::find_if(
			Element.Material.Execution.Scalars.begin(),
			Element.Material.Execution.Scalars.end(),
			[strName](const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Value)
			{ return Value.strName == strName; });
		return Found == Element.Material.Execution.Scalars.end() ?
			nullptr : &*Found;
	};
	const auto FindVector = [&Element](const std::string_view strName)
		-> const EFFECT_MATERIAL_VECTOR_PARAMETER_DESC*
	{
		const auto Found = std::find_if(
			Element.Material.Execution.Vectors.begin(),
			Element.Material.Execution.Vectors.end(),
			[strName](const EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Value)
			{ return Value.strName == strName; });
		return Found == Element.Material.Execution.Vectors.end() ?
			nullptr : &*Found;
	};
	const auto FindScalarOverride = [&Element](const std::string_view strName)
		-> const EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC*
	{
		const auto Found = std::find_if(
			Element.AuthoringOverrides.Scalars.begin(),
			Element.AuthoringOverrides.Scalars.end(),
			[strName](const EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Value)
			{ return Value.strName == strName; });
		return Found == Element.AuthoringOverrides.Scalars.end() ?
			nullptr : &*Found;
	};
	const auto HasColorOverride = [&Element](const std::string_view strName)
	{
		return std::any_of(Element.AuthoringOverrides.Colors.begin(),
			Element.AuthoringOverrides.Colors.end(),
			[strName](const EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Value)
			{ return Value.strName == strName; });
	};

	const auto RenderScalar = [this, &Element, &bChanged, &FindScalar,
		&FindScalarOverride](const SCALAR_CONTROL_DESC& Control,
		const f32_t fMinimum, const f32_t fMaximum)
	{
		const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC* pParameter =
			FindScalar(Control.strName);
		if (nullptr == pParameter || fMaximum < fMinimum)
			return;
		ImGui::PushID(Control.strName.data());
		f32_t fValue = pParameter->fValue;
		if (ImGui::DragFloat(Control.strLabel.data(), &fValue, Control.fStep,
			fMinimum, fMaximum, Control.pFormat,
			ImGuiSliderFlags_AlwaysClamp))
		{
			fValue = std::clamp(fValue, fMinimum, fMaximum);
			std::string strError;
			if (CEffectDocumentCodec::Set_AuthoringScalarOverride(
					Element, Control.strName, fValue, strError))
			{
				bChanged = true;
			}
			else
			{
				m_strDetailStatus =
					"Project Tuned scalar rejected: " + strError;
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\nAllowed: %g .. %g",
				Control.strTooltip.data(), fMinimum, fMaximum);
		const EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC* pOverride =
			FindScalarOverride(Control.strName);
		if (nullptr != pOverride)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f), "Modified");
			bool_t bResetKeepsFadeInterval = true;
			if (Control.strName == "water.fade-start-seconds")
			{
				const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC* pFadeEnd =
					FindScalar("water.fade-end-seconds");
				bResetKeepsFadeInterval = nullptr != pFadeEnd &&
					pFadeEnd->fValue > pOverride->fCompilerValue;
			}
			else if (Control.strName == "water.fade-end-seconds")
			{
				const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC* pFadeStart =
					FindScalar("water.fade-start-seconds");
				bResetKeepsFadeInterval = nullptr != pFadeStart &&
					pOverride->fCompilerValue > pFadeStart->fValue;
			}
			if (!bResetKeepsFadeInterval)
			{
				ImGui::SameLine();
				ImGui::TextDisabled(
					"Reset the other fade boundary first");
			}
			else
			{
				ImGui::SameLine();
				if (ImGui::SmallButton("Reset to Baseline"))
				{
					std::string strError;
					if (CEffectDocumentCodec::Reset_AuthoringScalarOverride(
							Element, Control.strName, strError))
					{
						bChanged = true;
					}
					else
					{
						m_strDetailStatus =
							"Project Tuned scalar reset rejected: " +
							strError;
					}
				}
			}
		}
		ImGui::PopID();
	};
	const auto RenderVector = [this, &Element, &bChanged, &FindVector,
		&HasColorOverride](const VECTOR_CONTROL_DESC& Control)
	{
		const EFFECT_MATERIAL_VECTOR_PARAMETER_DESC* pParameter =
			FindVector(Control.strName);
		if (nullptr == pParameter)
			return;
		ImGui::PushID(Control.strName.data());
		float4_t vValue = pParameter->vValue;
		const bool_t bEdited = Control.bEditAlpha ?
			ImGui::ColorEdit4(Control.strLabel.data(), &vValue.x,
				ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR) :
			ImGui::ColorEdit3(Control.strLabel.data(), &vValue.x,
				ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		if (bEdited)
		{
			vValue.x = std::clamp(vValue.x, 0.f, Control.fRgbMaximum);
			vValue.y = std::clamp(vValue.y, 0.f, Control.fRgbMaximum);
			vValue.z = std::clamp(vValue.z, 0.f, Control.fRgbMaximum);
			if (Control.bEditAlpha)
				vValue.w = std::clamp(vValue.w, 0.f, 1.f);
			std::string strError;
			if (CEffectDocumentCodec::Set_AuthoringColorOverride(
					Element, Control.strName, vValue, strError))
			{
				bChanged = true;
			}
			else
			{
				m_strDetailStatus =
					"Project Tuned color rejected: " + strError;
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\nRGB allowed: 0 .. %g%s",
				Control.strTooltip.data(), Control.fRgbMaximum,
				Control.bEditAlpha ? "; alpha: 0 .. 1" : "");
		if (HasColorOverride(Control.strName))
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f), "Modified");
			ImGui::SameLine();
			if (ImGui::SmallButton("Reset to Baseline"))
			{
				std::string strError;
				if (CEffectDocumentCodec::Reset_AuthoringColorOverride(
						Element, Control.strName, strError))
				{
					bChanged = true;
				}
				else
				{
					m_strDetailStatus =
						"Project Tuned color reset rejected: " + strError;
				}
			}
		}
		ImGui::PopID();
	};

	if (bWater)
	{
		ImGui::SeparatorText("Water Tint");
		for (const VECTOR_CONTROL_DESC& Control : WATER_COLORS)
			RenderVector(Control);
		ImGui::SeparatorText("Water Coverage");
		for (size_t i = 0u; i < 6u; ++i)
			RenderScalar(WATER_CONTROLS[i], WATER_CONTROLS[i].fMinimum,
				WATER_CONTROLS[i].fMaximum);
		ImGui::SeparatorText("Water Flow / Refraction");
		for (size_t i = 6u; i < 8u; ++i)
			RenderScalar(WATER_CONTROLS[i], WATER_CONTROLS[i].fMinimum,
				WATER_CONTROLS[i].fMaximum);
		ImGui::SeparatorText("Water Surface Timing");
		const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC* pFadeEnd =
			FindScalar("water.fade-end-seconds");
		const f32_t fFadeStartMaximum = nullptr == pFadeEnd ?
			std::nextafter(5.f,
				-(std::numeric_limits<f32_t>::infinity)()) :
			std::nextafter((std::min)(5.f, pFadeEnd->fValue),
				-(std::numeric_limits<f32_t>::infinity)());
		RenderScalar(WATER_CONTROLS[8u], WATER_CONTROLS[8u].fMinimum,
			fFadeStartMaximum);
		const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC* pFadeStart =
			FindScalar("water.fade-start-seconds");
		const f32_t fFadeEndMinimum = nullptr == pFadeStart ?
			std::nextafter(0.f,
				(std::numeric_limits<f32_t>::infinity)()) :
			std::nextafter((std::max)(0.f, pFadeStart->fValue),
				(std::numeric_limits<f32_t>::infinity)());
		RenderScalar(WATER_CONTROLS[9u], fFadeEndMinimum,
			WATER_CONTROLS[9u].fMaximum);
	}
	else
	{
		ImGui::SeparatorText("Glass Tint");
		for (const VECTOR_CONTROL_DESC& Control : GLASS_COLORS)
			RenderVector(Control);
		ImGui::SeparatorText("Glass Coverage / Edge");
		for (size_t i = 0u; i < 5u; ++i)
			RenderScalar(GLASS_CONTROLS[i], GLASS_CONTROLS[i].fMinimum,
				GLASS_CONTROLS[i].fMaximum);
		ImGui::SeparatorText("Glass Refraction");
		for (size_t i = 5u; i < 7u; ++i)
			RenderScalar(GLASS_CONTROLS[i], GLASS_CONTROLS[i].fMinimum,
				GLASS_CONTROLS[i].fMaximum);
		ImGui::SeparatorText("Glass Emission");
		RenderScalar(GLASS_CONTROLS[7u], GLASS_CONTROLS[7u].fMinimum,
			GLASS_CONTROLS[7u].fMaximum);
	}
	return true;
}

void Client::CEffect_Tool::Render_AuthoringMaterialParameters(
	EFFECT_ELEMENT_DESC& Element,
	bool_t& bChanged)
{
	if (Render_ProjectTunedSurfaceParameters(Element, bChanged))
		return;
	if (Element.Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
		!Element.Material.SourceMaterial.bEnabled &&
		!Element.Material.Execution.bEnabled &&
		!Element.Material.Execution.bFailClosed &&
		!Element.SourceRecipe.bEnabled)
	{
		bChanged |= ImGui::Checkbox("sRGB Base / Emissive", &Element.Material.bColorTexturesSRGB);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Decode colour textures as sRGB; mask, noise, and dissolve stay linear. "
				"Apply stages new texture views and preserves the previous Effect if loading fails.");
		}
	}

	struct SCALAR_CONTROL final
	{
		std::string strName;
		f32_t fValue = 0.f;
		bool_t bConsistent = true;
	};
	struct VECTOR_CONTROL final
	{
		std::string strName;
		float4_t vValue{};
		bool_t bConsistent = true;
	};
	std::vector<SCALAR_CONTROL> ScalarControls;
	std::vector<VECTOR_CONTROL> VectorControls;
	std::unordered_map<std::string, size_t> ScalarCounts;
	std::unordered_map<std::string, size_t> VectorCounts;
	const auto AddScalar = [&ScalarControls, &ScalarCounts](
		const std::string& strName, const f32_t fValue)
	{
		if (strName.empty())
			return;
		++ScalarCounts[strName];
		const auto Existing = std::find_if(ScalarControls.begin(),
			ScalarControls.end(), [&strName](const SCALAR_CONTROL& Control)
			{
				return Control.strName == strName;
			});
		if (Existing == ScalarControls.end())
			ScalarControls.push_back({ strName, fValue, true });
		else if (Existing->fValue != fValue)
			Existing->bConsistent = false;
	};
	const auto AddVector = [&VectorControls, &VectorCounts](
		const std::string& strName, const float4_t& vValue)
	{
		if (strName.empty())
			return;
		++VectorCounts[strName];
		const auto Existing = std::find_if(VectorControls.begin(),
			VectorControls.end(), [&strName](const VECTOR_CONTROL& Control)
			{
				return Control.strName == strName;
			});
		if (Existing == VectorControls.end())
		{
			VectorControls.push_back({ strName, vValue, true });
		}
		else if (Existing->vValue.x != vValue.x ||
			Existing->vValue.y != vValue.y ||
			Existing->vValue.z != vValue.z ||
			Existing->vValue.w != vValue.w)
		{
			Existing->bConsistent = false;
		}
	};
	for (const EFFECT_NAMED_FLOAT_DESC& Scalar :
		Element.Material.SourceMaterial.Scalars)
	{
		AddScalar(Scalar.strName, Scalar.fValue);
	}
	for (const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Scalar :
		Element.Material.Execution.Scalars)
	{
		AddScalar(Scalar.strName, Scalar.fValue);
	}
	for (const EFFECT_NAMED_FLOAT4_DESC& Vector :
		Element.Material.SourceMaterial.Vectors)
	{
		AddVector(Vector.strName, Vector.vValue);
	}
	const auto AddExecutionVectors = [&AddVector](
		const std::vector<EFFECT_MATERIAL_VECTOR_PARAMETER_DESC>& Values)
	{
		for (const EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Value : Values)
			AddVector(Value.strName, Value.vValue);
	};
	AddExecutionVectors(Element.Material.Execution.Vectors);
	AddExecutionVectors(Element.Material.Execution.ArtistParameters);
	AddExecutionVectors(Element.Material.Execution.Colors);
	std::erase_if(ScalarControls,
		[&VectorCounts](const SCALAR_CONTROL& Control)
		{
			return !Control.bConsistent ||
				0u != VectorCounts.count(Control.strName);
		});
	std::erase_if(VectorControls,
		[&ScalarCounts](const VECTOR_CONTROL& Control)
		{
			return !Control.bConsistent ||
				0u != ScalarCounts.count(Control.strName);
		});
	if (ScalarControls.empty() && VectorControls.empty())
		return;
	if (!ImGui::CollapsingHeader(
			"Material Parameters", ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}
	ImGui::TextDisabled(
		"Only compiler-declared parameters are editable. Overrides never change admission.");
	for (const SCALAR_CONTROL& Control : ScalarControls)
	{
		ImGui::PushID("scalar");
		ImGui::PushID(Control.strName.c_str());
		f32_t fValue = Control.fValue;
		if (ImGui::DragFloat(Control.strName.c_str(), &fValue, 0.001f,
			-100000.f, 100000.f, "%.6g"))
		{
			std::string strError;
			if (CEffectDocumentCodec::Set_AuthoringScalarOverride(
					Element, Control.strName, fValue, strError))
			{
				bChanged = true;
			}
			else
			{
				m_strDetailStatus = "Scalar override rejected: " + strError;
			}
		}
		const auto Override = std::find_if(
			Element.AuthoringOverrides.Scalars.begin(),
			Element.AuthoringOverrides.Scalars.end(),
			[&Control](const EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Candidate)
			{ return Candidate.strName == Control.strName; });
		if (Override != Element.AuthoringOverrides.Scalars.end())
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f), "Modified");
			ImGui::SameLine();
			if (ImGui::SmallButton("Reset to Source"))
			{
				std::string strError;
				if (CEffectDocumentCodec::Reset_AuthoringScalarOverride(
						Element, Control.strName, strError))
				{
					bChanged = true;
				}
				else
				{
					m_strDetailStatus = "Scalar reset rejected: " + strError;
				}
			}
		}
		ImGui::PopID();
		ImGui::PopID();
	}
	for (const VECTOR_CONTROL& Control : VectorControls)
	{
		ImGui::PushID("vector");
		ImGui::PushID(Control.strName.c_str());
		float4_t vValue = Control.vValue;
		if (DragFloat4(Control.strName.c_str(), vValue, 0.001f,
			-100000.f, 100000.f, "%.6g"))
		{
			std::string strError;
			if (CEffectDocumentCodec::Set_AuthoringColorOverride(
					Element, Control.strName, vValue, strError))
			{
				bChanged = true;
			}
			else
			{
				m_strDetailStatus = "Vector override rejected: " + strError;
			}
		}
		const auto Override = std::find_if(
			Element.AuthoringOverrides.Colors.begin(),
			Element.AuthoringOverrides.Colors.end(),
			[&Control](const EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Candidate)
			{ return Candidate.strName == Control.strName; });
		if (Override != Element.AuthoringOverrides.Colors.end())
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f), "Modified");
			ImGui::SameLine();
			if (ImGui::SmallButton("Reset to Source"))
			{
				std::string strError;
				if (CEffectDocumentCodec::Reset_AuthoringColorOverride(
						Element, Control.strName, strError))
				{
					bChanged = true;
				}
				else
				{
					m_strDetailStatus = "Vector reset rejected: " + strError;
				}
			}
		}
		ImGui::PopID();
		ImGui::PopID();
	}
}

void Client::CEffect_Tool::Render_KindDetail(
    EFFECT_ELEMENT_DESC& Element,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Type Detail",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;
    EFFECT_DETAIL_DESC& Detail = Element.Detail;
    switch (Element.eKind)
    {
	case EFFECT_ELEMENT_KIND::MESH:
		bChanged |= ImGui::Checkbox("Use Model Material",
			&Detail.Mesh.bUseModelMaterial);
		ImGui::TextDisabled("Model Import Scale is edited once in Size above.");
        break;
    case EFFECT_ELEMENT_KIND::SPRITE:
        bChanged |= ImGui::Checkbox("Billboard",
            &Detail.Sprite.bBillboard);
		bChanged |= ImGui::DragFloat("Billboard Roll Degrees",
			&Detail.Sprite.fBillboardRollDegrees, 1.f, -3600.f, 3600.f,
			"%.1f", ImGuiSliderFlags_AlwaysClamp);
		bChanged |= ImGui::DragFloat("Billboard Roll Degrees Per Second",
			&Detail.Sprite.fBillboardRollDegreesPerSecond, 1.f, -3600.f,
			3600.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        break;
	case EFFECT_ELEMENT_KIND::DECAL:
		ImGui::TextDisabled(Is_SourceParticleCarrier(Element) ?
			"Source Size and Projection Depth are edited once in Source Playback Tuning above." :
			"Decal Size and Projection Depth are edited once in Size above.");
		break;
	case EFFECT_ELEMENT_KIND::PARTICLE:
	{
		const bool_t bSourcePlayback = Element.SourceRecipe.bEnabled;
		const bool_t bCompilerOwnedSourceParticle =
			m_bDetailDraftPortableRecipeReadOnly &&
			bSourcePlayback;
		const bool_t bMeshParticle = Resolve_AuthoringFamily(Element) ==
			EFFECT_AUTHORING_FAMILY::MESH_PARTICLE;
		bool_t bFixedCenterSpacing =
			Detail.Particle.fFixedCenterSpacingWorldUnits > 0.f;
		const bool_t bNativeSpriteDynamicsEditable = !bMeshParticle &&
			Is_DirectHandAuthoredElement(Element) && Detail.Particle.bBillboard &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::END &&
			Element.Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			!Element.Material.Execution.bFailClosed &&
			!Element.Material.Execution.bAuthoringApproximate;
		const bool_t bGenericMeshRingFillCarrier = bMeshParticle &&
			Is_DirectHandAuthoredElement(Element) &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::END &&
			Element.Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			!Element.Material.Execution.bFailClosed &&
			!Element.Material.Execution.bAuthoringApproximate &&
			Element.Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
		if (bCompilerOwnedSourceParticle)
		{
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
				"Runtime SourceRecipe owns spawn rate/bursts, particle lifetime, initial position/velocity/acceleration, size, and DynamicParameter values. Those Detail controls are read-only because changing them would not affect playback.");
			ImGui::TextDisabled(
				"Working overlays: Max Particles, Random Seed, Local Space, Target Attractor, sprite-family Billboard/Roll, Element Transform, Start Delay, Color/material, and supported resources.");
		}
		if (bMeshParticle)
		{
			ImGui::SeparatorText("Mesh Carrier");
			bChanged |= ImGui::Checkbox("Use Model Material",
				&Detail.Mesh.bUseModelMaterial);
			ImGui::TextDisabled(
				"Model Import Scale is edited once in Size or Source Playback Tuning above.");
			if (bGenericMeshRingFillCarrier)
			{
				ImGui::SeparatorText("Mesh Ring Fill (Raw Radial V)");
				EFFECT_MESH_RING_FILL_DESC& RingFill = Detail.Mesh.RingFill;
				bool_t bRingFillEnabled = RingFill.bEnabled;
				if (ImGui::Checkbox("Enable Mesh Ring Fill", &bRingFillEnabled))
				{
					if (bRingFillEnabled)
					{
						RingFill = EFFECT_MESH_RING_FILL_DESC{};
						RingFill.bEnabled = true;
						RingFill.fProgress = 0.f;
					}
					else
					{
						RingFill = EFFECT_MESH_RING_FILL_DESC{};
						Detail.LinearLerp.bRingFillProgress = false;
						Detail.LinearLerp.fEndRingFillProgress = 1.f;
					}
					bChanged = true;
				}
				if (RingFill.bEnabled)
				{
					bChanged |= ImGui::SliderFloat("Ring Fill Progress",
						&RingFill.fProgress, 0.f, 1.f, "%.3f",
						ImGuiSliderFlags_AlwaysClamp);
					static const char* const s_RingFillDirectionLabels[] =
					{
						"Inner To Outer (V low to high)",
						"Outer To Inner (V high to low)"
					};
					int32_t iDirection = static_cast<int32_t>(
						RingFill.eDirection);
					if (ImGui::Combo("Ring Fill Direction", &iDirection,
						s_RingFillDirectionLabels,
						IM_ARRAYSIZE(s_RingFillDirectionLabels)))
					{
						RingFill.eDirection =
							static_cast<EFFECT_RING_FILL_DIRECTION>(iDirection);
						bChanged = true;
					}
					bChanged |= ImGui::SliderFloat("Ring Fill Feather",
						&RingFill.fFeather, 0.f, 0.5f, "%.3f",
						ImGuiSliderFlags_AlwaysClamp);
					bChanged |= ImGui::Checkbox("Invert Carrier Radial V",
						&RingFill.bInvert);
					ImGui::TextDisabled(
						"Uses the mesh carrier's raw TEXCOORD0.y. Invert calibrates reversed carrier UVs before Direction is applied.");
					ImGui::TextDisabled(
						"Lerp uses Element Timing Life; particle lifetime may extend past it for a short completed-ring hold.");
				}
			}
			else
			{
				ImGui::TextDisabled(
					"Ring Fill requires a direct-authored effect.standard Mesh Particle with a non-Opaque profile.");
			}
		}
		ImGui::SeparatorText("Particle Runtime Overlays");
		if (ImGui::InputScalar("Max Particles", ImGuiDataType_U32,
			&Detail.Particle.iMaxParticles))
		{
			const uint32_t iMinimum = (std::min)(2048u,
				(std::max)(1u, Detail.Particle.iBurstCount));
			Detail.Particle.iMaxParticles = std::clamp(
				Detail.Particle.iMaxParticles, iMinimum, 2048u);
			bChanged = true;
		}
		if (ImGui::InputScalar("Random Seed", ImGuiDataType_U32,
			&Detail.Particle.iRandomSeed))
		{
			Detail.Particle.iRandomSeed = (std::max)(
				1u, Detail.Particle.iRandomSeed);
			bChanged = true;
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Seeds the emitter stream. Source modules with their own explicit seed keep that module-local seed.");
		}
		if (bCompilerOwnedSourceParticle)
		{
			ImGui::TextDisabled(
				"The compiler-owned SourceRecipe supplies spawn, lifetime, motion, size, and Dynamic Parameters. Tune its evaluated result in Source Playback Tuning above; Max Particles and Random Seed remain working overlays.");
		}
		else
		{
			ImGui::SeparatorText("Particle Spawn");
			const f32_t fPreviousFixedCenterSpacing =
				Detail.Particle.fFixedCenterSpacingWorldUnits;
			if (ImGui::DragFloat("Fixed Center Spacing (world m)",
				&Detail.Particle.fFixedCenterSpacingWorldUnits, 0.05f,
				0.f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				Detail.Particle.fFixedCenterSpacingWorldUnits = std::clamp(
					Detail.Particle.fFixedCenterSpacingWorldUnits, 0.f, 1000.f);
				if (Detail.Particle.fFixedCenterSpacingWorldUnits > 0.f &&
					Detail.Particle.fFixedCenterSpacingWorldUnits < 0.001f)
				{
					Detail.Particle.fFixedCenterSpacingWorldUnits = 0.001f;
				}
				if (Detail.Particle.fFixedCenterSpacingWorldUnits > 0.f)
				{
					Detail.Particle.fSpawnRatePerSecond = 0.f;
					Detail.Particle.iBurstCount = 0u;
					Detail.Particle.vInitialPositionMin = {};
					Detail.Particle.vInitialPositionMax = {};
					Detail.Particle.vInitialVelocityMin = {};
					Detail.Particle.vInitialVelocityMax = {};
					Detail.Particle.vAcceleration = {};
					Detail.Particle.bLocalSpace = false;
					Detail.Particle.SpawnShape = {};
					Detail.Particle.InitialOrientation = {};
					Detail.Particle.InitialVelocity = {};
					Detail.Particle.TargetAttractor = {};
				}
				else if (fPreviousFixedCenterSpacing > 0.f &&
					Detail.Particle.fSpawnRatePerSecond <= 0.f &&
					0u == Detail.Particle.iBurstCount)
				{
					Detail.Particle.fSpawnRatePerSecond = 20.f;
				}
				bFixedCenterSpacing =
					Detail.Particle.fFixedCenterSpacingWorldUnits > 0.f;
				bChanged = true;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"0 keeps time-rate spawning. A positive value samples the moving Element root by travelled world distance, so centres stay exact even between fixed ticks. Visible surface gap equals this centre spacing minus the model's projected length.");
			}
			const bool_t bEvenRingDistribution =
				Detail.Particle.SpawnShape.eKind ==
					EFFECT_PARTICLE_SPAWN_SHAPE::RING &&
				Detail.Particle.SpawnShape.eDistribution ==
					EFFECT_PARTICLE_SPAWN_DISTRIBUTION::EVEN;
			ImGui::BeginDisabled(
				bFixedCenterSpacing || bEvenRingDistribution);
			bChanged |= ImGui::DragFloat("Spawn Rate / Second",
				&Detail.Particle.fSpawnRatePerSecond, 1.f, 0.f, 2048.f,
				"%.3f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::EndDisabled();
			ImGui::BeginDisabled(bFixedCenterSpacing);
			if (ImGui::InputScalar("Fixed Burst at Element Start",
				ImGuiDataType_U32, &Detail.Particle.iBurstCount))
			{
				const uint32_t iMinimum = bEvenRingDistribution ? 2u : 0u;
				Detail.Particle.iBurstCount = std::clamp(
					Detail.Particle.iBurstCount, iMinimum,
					Detail.Particle.iMaxParticles);
				bChanged = true;
			}
			ImGui::EndDisabled();
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Emits this many particles once at the Element-local start.");
			}
			ImGui::TextDisabled(bFixedCenterSpacing ?
				"Fixed spacing owns birth cadence. Initial Position/Velocity and Acceleration are locked to zero; move the Element Transform to draw the row." :
				"Start Delay controls the skill time; Spawn Rate and this fixed burst can be used together.");

			ImGui::SeparatorText("Particle Lifetime and Shape");
			if (DragFloat2("Particle Life Min/Max",
			Detail.Particle.vLifeTimeSeconds, 0.01f, 0.001f, 30.f))
		{
            Detail.Particle.vLifeTimeSeconds.y = (std::max)(
                Detail.Particle.vLifeTimeSeconds.x,
                Detail.Particle.vLifeTimeSeconds.y);
			bChanged = true;
		}
		ImGui::BeginDisabled(bFixedCenterSpacing);
		const bool_t bPositionMinChanged = DragFloat3(
			"Initial Position Min", Detail.Particle.vInitialPositionMin,
			0.01f, -1000.f, 1000.f);
		const bool_t bPositionMaxChanged = DragFloat3(
			"Initial Position Max", Detail.Particle.vInitialPositionMax,
			0.01f, -1000.f, 1000.f);
		if (bPositionMinChanged || bPositionMaxChanged)
		{
			Detail.Particle.vInitialPositionMax.x = (std::max)(
				Detail.Particle.vInitialPositionMin.x,
				Detail.Particle.vInitialPositionMax.x);
			Detail.Particle.vInitialPositionMax.y = (std::max)(
				Detail.Particle.vInitialPositionMin.y,
				Detail.Particle.vInitialPositionMax.y);
			Detail.Particle.vInitialPositionMax.z = (std::max)(
				Detail.Particle.vInitialPositionMin.z,
				Detail.Particle.vInitialPositionMax.z);
			bChanged = true;
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled(bFixedCenterSpacing);
		const bool_t bVelocityMinChanged = DragFloat3("Initial Velocity Min",
            Detail.Particle.vInitialVelocityMin, 0.01f, -1000.f, 1000.f);
        const bool_t bVelocityMaxChanged = DragFloat3("Initial Velocity Max",
            Detail.Particle.vInitialVelocityMax, 0.01f, -1000.f, 1000.f);
        if (bVelocityMinChanged || bVelocityMaxChanged)
        {
            Detail.Particle.vInitialVelocityMax.x = (std::max)(
                Detail.Particle.vInitialVelocityMin.x,
                Detail.Particle.vInitialVelocityMax.x);
            Detail.Particle.vInitialVelocityMax.y = (std::max)(
                Detail.Particle.vInitialVelocityMin.y,
                Detail.Particle.vInitialVelocityMax.y);
            Detail.Particle.vInitialVelocityMax.z = (std::max)(
                Detail.Particle.vInitialVelocityMin.z,
                Detail.Particle.vInitialVelocityMax.z);
			bChanged = true;
		}
		bChanged |= DragFloat3("Acceleration",
			Detail.Particle.vAcceleration, 0.01f, -1000.f, 1000.f);
		ImGui::EndDisabled();
		if (bNativeSpriteDynamicsEditable)
		{
			bChanged |= ImGui::DragFloat("Particle Drag",
				&Detail.Particle.fDrag, 0.01f, 0.f, 1000.f, "%.3f",
				ImGuiSliderFlags_AlwaysClamp);
			if (DragFloat2("Particle Rotation Min/Max Degrees",
				Detail.Particle.vRotationRangeDegrees, 1.f, -3600.f, 3600.f))
			{
				Detail.Particle.vRotationRangeDegrees.y = (std::max)(
					Detail.Particle.vRotationRangeDegrees.x,
					Detail.Particle.vRotationRangeDegrees.y);
				bChanged = true;
			}
			if (DragFloat2("Particle Spin Min/Max Degrees Per Second",
				Detail.Particle.vSpinRangeDegreesPerSecond, 1.f, -3600.f, 3600.f))
			{
				Detail.Particle.vSpinRangeDegreesPerSecond.y = (std::max)(
					Detail.Particle.vSpinRangeDegreesPerSecond.x,
					Detail.Particle.vSpinRangeDegreesPerSecond.y);
				bChanged = true;
			}
			const bool_t bHasAtlas =
				static_cast<int64_t>(Detail.UV.iTileColumns) * Detail.UV.iTileRows > 1;
			const bool_t bDisableLifeSubUV = !bHasAtlas && !Detail.Particle.bSubUVOverLife;
			if (bDisableLifeSubUV)
				ImGui::BeginDisabled();
			if (ImGui::Checkbox("SubUV over particle life",
				&Detail.Particle.bSubUVOverLife))
			{
				if (Detail.Particle.bSubUVOverLife)
				{
					Detail.UV.bSequence = false;
					Detail.UV.bLoop = false;
					Detail.UV.iTileIndex = 0;
				}
				bChanged = true;
			}
			if (bDisableLifeSubUV)
				ImGui::EndDisabled();
			ImGui::TextDisabled(
				"Set UV Tile Columns/Rows above 1 for an atlas. Life SubUV plays all frames once over each particle's own lifetime.");
		}

		/* Spawn volume and emission direction: the two axes the authored Detail
		   could not express at all, so a ring that collapses inward or a mesh
		   that flies along an arc had to stay owned by the source modules. */
		ImGui::SeparatorText("Particle Spawn Volume");
		EFFECT_PARTICLE_SPAWN_SHAPE_DESC& Shape = Detail.Particle.SpawnShape;
		ImGui::BeginDisabled(bFixedCenterSpacing);
		static const char* const s_SpawnShapeLabels[] =
		{
			"Point (Initial Position box)", "Sphere", "Ring (XZ)", "Box"
		};
		int32_t iSpawnShape = static_cast<int32_t>(Shape.eKind);
		if (ImGui::Combo("Spawn Shape", &iSpawnShape, s_SpawnShapeLabels,
			IM_ARRAYSIZE(s_SpawnShapeLabels)))
		{
			Shape.eKind = static_cast<EFFECT_PARTICLE_SPAWN_SHAPE>(iSpawnShape);
			if (EFFECT_PARTICLE_SPAWN_SHAPE::SPHERE == Shape.eKind ||
				EFFECT_PARTICLE_SPAWN_SHAPE::RING == Shape.eKind)
			{
				Shape.fRadius = (std::max)(0.001f, Shape.fRadius);
			}
			if (EFFECT_PARTICLE_SPAWN_SHAPE::BOX == Shape.eKind &&
				Shape.vExtents.x <= 0.f && Shape.vExtents.y <= 0.f &&
				Shape.vExtents.z <= 0.f)
			{
				Shape.vExtents = { 0.5f, 0.5f, 0.5f };
			}
			if (EFFECT_PARTICLE_SPAWN_SHAPE::RING != Shape.eKind)
			{
				Shape.eDistribution =
					EFFECT_PARTICLE_SPAWN_DISTRIBUTION::RANDOM;
				Detail.Particle.InitialOrientation =
					EFFECT_PARTICLE_INITIAL_ORIENTATION_DESC{};
			}
			bChanged = true;
		}
		if (EFFECT_PARTICLE_SPAWN_SHAPE::POINT != Shape.eKind)
		{
			if (EFFECT_PARTICLE_SPAWN_SHAPE::BOX == Shape.eKind)
			{
				bChanged |= DragFloat3("Spawn Box Half Extents",
					Shape.vExtents, 0.01f, 0.f, 1000.f);
			}
			else
			{
				const bool_t bRadiusChanged = ImGui::DragFloat("Spawn Radius",
					&Shape.fRadius, 0.01f, 0.001f, 1000.f, "%.3f",
					ImGuiSliderFlags_AlwaysClamp);
				const bool_t bInnerChanged = ImGui::DragFloat(
					"Spawn Inner Radius", &Shape.fInnerRadius, 0.01f, 0.f,
					1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
				if (bRadiusChanged || bInnerChanged)
				{
					Shape.fInnerRadius = (std::min)(
						Shape.fInnerRadius, Shape.fRadius);
					bChanged = true;
				}
				bChanged |= ImGui::DragFloat("Spawn Arc Degrees",
					&Shape.fArcDegrees, 1.f, 0.001f, 360.f, "%.1f",
					ImGuiSliderFlags_AlwaysClamp);
			}
			ImGui::TextDisabled(
				"The shape offset is added on top of the Initial Position box.");
		}
		if (EFFECT_PARTICLE_SPAWN_SHAPE::RING == Shape.eKind &&
			Is_DirectHandAuthoredElement(Element))
		{
			static const char* const s_DistributionLabels[] =
			{
				"Random", "Even (Fixed Burst)"
			};
			int32_t iDistribution = static_cast<int32_t>(Shape.eDistribution);
			if (ImGui::Combo("Ring Distribution", &iDistribution,
				s_DistributionLabels, IM_ARRAYSIZE(s_DistributionLabels)))
			{
				Shape.eDistribution =
					static_cast<EFFECT_PARTICLE_SPAWN_DISTRIBUTION>(
						iDistribution);
				if (EFFECT_PARTICLE_SPAWN_DISTRIBUTION::EVEN ==
					Shape.eDistribution)
				{
					Detail.Particle.fSpawnRatePerSecond = 0.f;
					Detail.Particle.iBurstCount = (std::max)(
						2u, Detail.Particle.iBurstCount);
					Detail.Particle.iMaxParticles = (std::max)(
						Detail.Particle.iMaxParticles,
						Detail.Particle.iBurstCount);
				}
				bChanged = true;
			}
			if (EFFECT_PARTICLE_SPAWN_DISTRIBUTION::EVEN ==
				Shape.eDistribution)
			{
				ImGui::TextDisabled(
					"Even uses one fixed burst: full 360 degrees uses i/N without a duplicate endpoint; partial arcs include both endpoints.");
			}

			if (!bMeshParticle)
			{
				EFFECT_PARTICLE_INITIAL_ORIENTATION_DESC& Orientation =
					Detail.Particle.InitialOrientation;
				static const char* const s_OrientationModeLabels[] =
				{
					"Fixed", "Ground Radial Outward",
					"Ground Radial Inward", "Ground Tangent Clockwise",
					"Ground Tangent Counter Clockwise"
				};
				int32_t iOrientationMode = static_cast<int32_t>(Orientation.eMode);
				if (ImGui::Combo("Initial Sprite Orientation", &iOrientationMode,
					s_OrientationModeLabels,
					IM_ARRAYSIZE(s_OrientationModeLabels)))
				{
					Orientation.eMode =
						static_cast<EFFECT_PARTICLE_ORIENTATION_MODE>(
							iOrientationMode);
					if (EFFECT_PARTICLE_ORIENTATION_MODE::FIXED ==
						Orientation.eMode)
					{
						Orientation.fOffsetDegrees = 0.f;
					}
					else
					{
						Detail.Particle.bBillboard = false;
					}
					bChanged = true;
				}
				if (EFFECT_PARTICLE_ORIENTATION_MODE::FIXED !=
					Orientation.eMode)
				{
					bChanged |= ImGui::DragFloat("Orientation Offset Degrees",
						&Orientation.fOffsetDegrees, 1.f, -3600.f, 3600.f,
						"%.1f", ImGuiSliderFlags_AlwaysClamp);
					ImGui::TextDisabled(
						"Ground orientation is set at particle birth and requires Billboard OFF.");
				}
			}
		}

		ImGui::SeparatorText("Particle Emission Direction");
		EFFECT_PARTICLE_INITIAL_VELOCITY_DESC& Emission =
			Detail.Particle.InitialVelocity;
		static const char* const s_VelocityModeLabels[] =
		{
			"Fixed (Initial Velocity box)", "Outward", "Inward", "Cone (+Y)"
		};
		int32_t iVelocityMode = static_cast<int32_t>(Emission.eMode);
		if (ImGui::Combo("Emission Mode", &iVelocityMode, s_VelocityModeLabels,
			IM_ARRAYSIZE(s_VelocityModeLabels)))
		{
			Emission.eMode =
				static_cast<EFFECT_PARTICLE_VELOCITY_MODE>(iVelocityMode);
			if (Emission.eMode != EFFECT_PARTICLE_VELOCITY_MODE::CONE)
				Emission.bUniformSolidAngle = false;
			bChanged = true;
		}
		if (EFFECT_PARTICLE_VELOCITY_MODE::FIXED != Emission.eMode)
		{
			if (DragFloat2("Emission Speed Min/Max", Emission.vSpeedRange,
				0.01f, -1000.f, 1000.f))
			{
				Emission.vSpeedRange.y = (std::max)(
					Emission.vSpeedRange.x, Emission.vSpeedRange.y);
				bChanged = true;
			}
			if (EFFECT_PARTICLE_VELOCITY_MODE::CONE == Emission.eMode)
			{
				bChanged |= ImGui::DragFloat("Cone Half Angle Degrees",
					&Emission.fConeAngleDegrees, 1.f, 0.f, 180.f, "%.1f",
					ImGuiSliderFlags_AlwaysClamp);
				if (bNativeSpriteDynamicsEditable)
				{
					bChanged |= ImGui::Checkbox("Uniform Cone Solid Angle",
						&Emission.bUniformSolidAngle);
				}
			}
			ImGui::TextDisabled(
				"Outward and Inward are radial about the Element origin and replace the Initial Velocity box.");
		}
		ImGui::EndDisabled();
		}
		if (bCompilerOwnedSourceParticle)
		{
			ImGui::TextDisabled(
				"Source-owned raw fields are hidden because editing them would not change this portable runtime recipe.");
		}

		ImGui::SeparatorText("Particle Target Attractor");
		EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC& Attractor =
			Detail.Particle.TargetAttractor;
		ImGui::BeginDisabled(bFixedCenterSpacing);
		bool_t bAttractorEnabled = Attractor.bEnabled;
		if (ImGui::Checkbox("Enable Target Attractor", &bAttractorEnabled))
		{
			if (bAttractorEnabled)
			{
				Attractor.bEnabled = true;
				if (Attractor.fRadialAcceleration == 0.f)
					Attractor.fRadialAcceleration = 8.f;
			}
			else
			{
				Attractor = EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC{};
			}
			bChanged = true;
		}
		if (Attractor.bEnabled)
		{
			static const char* const s_AttractorTargetSpaceLabels[] =
			{
				"Effect Root Local", "Element Local"
			};
			int32_t iTargetSpace = static_cast<int32_t>(
				Attractor.eTargetSpace);
			if (ImGui::Combo("Target Space", &iTargetSpace,
				s_AttractorTargetSpaceLabels,
				IM_ARRAYSIZE(s_AttractorTargetSpaceLabels)))
			{
				Attractor.eTargetSpace =
					static_cast<EFFECT_PARTICLE_ATTRACTOR_TARGET_SPACE>(
						iTargetSpace);
				bChanged = true;
			}
			bChanged |= DragFloat3("Target Offset",
				Attractor.vTargetOffset, 0.01f, -1000.f, 1000.f);
			if (DragFloat2("Active Normalized Min/Max",
				Attractor.vActiveNormalized, 0.01f, 0.f, 1.f))
			{
				Attractor.vActiveNormalized.x = std::clamp(
					Attractor.vActiveNormalized.x, 0.f, 0.999f);
				Attractor.vActiveNormalized.y = std::clamp(
					Attractor.vActiveNormalized.y,
					Attractor.vActiveNormalized.x + 0.001f, 1.f);
				bChanged = true;
			}
			bChanged |= ImGui::DragFloat("Radial Acceleration",
				&Attractor.fRadialAcceleration, 0.1f, 0.f, 10000.f,
				"%.3f", ImGuiSliderFlags_AlwaysClamp);
			bChanged |= ImGui::DragFloat("Tangential Acceleration",
				&Attractor.fTangentialAcceleration, 0.1f, -10000.f,
				10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			bChanged |= ImGui::DragFloat("Maximum Speed",
				&Attractor.fMaximumSpeed, 0.1f, 0.001f, 1000.f,
				"%.3f", ImGuiSliderFlags_AlwaysClamp);
			bChanged |= ImGui::DragFloat("Convergence Radius",
				&Attractor.fConvergenceRadius, 0.01f, 0.001f, 1000.f,
				"%.3f", ImGuiSliderFlags_AlwaysClamp);
			bChanged |= ImGui::DragFloat("Arrival Damping",
				&Attractor.fArrivalDamping, 0.1f, 0.f, 1000.f,
				"%.3f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextDisabled(
				"Authored PROJECT_TUNED motion layer. SourceRecipe modules run first; this layer then steers the effective velocity toward the selected centre.");
		}
		ImGui::EndDisabled();
		if (bFixedCenterSpacing)
		{
			ImGui::TextDisabled(
				"Fixed spacing freezes each birth centre, so Target Attractor is unavailable.");
		}
		const bool_t bOrientationLocksParticleBasis =
			EFFECT_PARTICLE_ORIENTATION_MODE::FIXED !=
				Detail.Particle.InitialOrientation.eMode;
		ImGui::BeginDisabled(bFixedCenterSpacing);
		bChanged |= ImGui::Checkbox("Particle Local Space",
			&Detail.Particle.bLocalSpace);
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Local Space keeps spawned particles relative to this Element; "
				"disabled particles freeze the emitter position/orientation at birth; manual scale curves keep animating.");
		}
		if (!bMeshParticle)
		{
			if (bOrientationLocksParticleBasis)
				ImGui::BeginDisabled();
			bChanged |= ImGui::Checkbox("Particle Billboard",
				&Detail.Particle.bBillboard);
			if (bOrientationLocksParticleBasis)
				ImGui::EndDisabled();
			if (Detail.Particle.bBillboard)
			{
				/* The renderer rebuilds a billboarded quad from the camera every
				   frame, so the Transform rotation above never reaches it. */
				bChanged |= ImGui::DragFloat("Billboard Roll Degrees##particle",
					&Detail.Sprite.fBillboardRollDegrees, 1.f, -3600.f, 3600.f,
					"%.1f", ImGuiSliderFlags_AlwaysClamp);
				bChanged |= ImGui::DragFloat(
					"Billboard Roll Degrees Per Second##particle",
					&Detail.Sprite.fBillboardRollDegreesPerSecond, 1.f, -3600.f,
					3600.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::TextDisabled(
					"Billboard faces the camera, so Transform rotation does not apply. Source Playback Tuning > Rotation x scales source rotation modules on top of this roll.");
			}
		}
		else
		{
			ImGui::TextDisabled(
				"Mesh Particles use their model orientation; Billboard and sprite Roll are not consumed.");
		}
		if (bCompilerOwnedSourceParticle)
		{
			ImGui::TextDisabled(
				"SourceRecipe ParameterDynamic modules own runtime material parameters; ignored Detail fallbacks are hidden.");
		}
		else
		{
			ImGui::SeparatorText("Particle Dynamic Material Parameters");
			ImGui::TextDisabled(
				"Enabled components interpolate Start -> End over particle life.");
			bChanged |= ImGui::CheckboxFlags(
				"X##particle.dynamic", &Detail.Particle.iDynamicParameterComponentMask,
				1u << 0u);
			ImGui::SameLine();
			bChanged |= ImGui::CheckboxFlags(
				"Y##particle.dynamic", &Detail.Particle.iDynamicParameterComponentMask,
				1u << 1u);
			ImGui::SameLine();
			bChanged |= ImGui::CheckboxFlags(
				"Z##particle.dynamic", &Detail.Particle.iDynamicParameterComponentMask,
				1u << 2u);
			ImGui::SameLine();
			bChanged |= ImGui::CheckboxFlags(
				"W##particle.dynamic", &Detail.Particle.iDynamicParameterComponentMask,
				1u << 3u);
			bChanged |= DragFloat4("Dynamic Start",
				Detail.Particle.vDynamicParameterStart,
				0.01f, -1000.f, 1000.f);
			bChanged |= DragFloat4("Dynamic End",
				Detail.Particle.vDynamicParameterEnd,
				0.01f, -1000.f, 1000.f);
			const uint32_t iTrackAConsumedMask =
				Element.Material.Execution.iDynamicConsumedMask & 0x0fu;
			if (Element.Material.Execution.bEnabled &&
				0u != iTrackAConsumedMask)
			{
				ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
					"Track A consumed mask 0x%X: Dynamic Start/End is the editable bounded source carrier.",
					iTrackAConsumedMask);
			}
		}
        break;
    }
	case EFFECT_ELEMENT_KIND::TRAIL:
	{
        bChanged |= ImGui::InputScalar("Trail Max Points",
            ImGuiDataType_U32, &Detail.Trail.iMaxPoints);
        bChanged |= ImGui::DragFloat("Trail Point Life",
            &Detail.Trail.fPointLifeTimeSeconds,
            0.01f, 0.001f, 30.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::DragFloat("Trail Sample Interval",
            &Detail.Trail.fSampleIntervalSeconds,
            0.001f, 0.001f, 1.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::DragFloat("Trail Minimum Distance",
            &Detail.Trail.fMinimumDistance,
            0.001f, 0.f, 100.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
		const bool_t bSourceOwnedTrailGeometry =
			Element.SourceRecipe.bEnabled || Element.Material.Execution.bEnabled;
		ImGui::BeginDisabled(bSourceOwnedTrailGeometry);
		bChanged |= ImGui::DragFloat("Trail UV Repeat Distance",
			&Detail.Trail.fTilingDistanceWorldUnits,
			0.01f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		bChanged |= ImGui::DragFloat("Trail Curve Step",
			&Detail.Trail.fDistanceTessellationStepWorldUnits,
			0.001f, 0.f, 10.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::EndDisabled();
		if (bSourceOwnedTrailGeometry)
		{
			ImGui::TextDisabled(
				"Typed source ribbons own fixed UV/tessellation values; manual Trail controls are read-only.");
		}
		ImGui::TextDisabled(
			"UV Repeat Distance > 0 maps U by traveled distance; 0 keeps legacy one-texture-per-segment UVs.");
		ImGui::TextDisabled(
			"Curve Step > 0 subdivides long segments when UV Repeat Distance is also > 0.");
		ImGui::TextDisabled("Trail Start/End Width are edited once in Size above.");
        bChanged |= ImGui::Checkbox("Trail Faces Camera",
            &Detail.Trail.bFaceCamera);
        break;
	}
    case EFFECT_ELEMENT_KIND::LIGHT:
	{
		EFFECT_LIGHT_DETAIL_DESC& Light = Detail.Light;
		ImGui::SeparatorText("Presentation Light");
		ImGui::TextDisabled("Profile: Point Light (Reconstructed v1)");
		if (!Light.bEnabled)
		{
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
				"This source row has no admitted typed Light payload. Delete or hide it here; enabling it requires source-backed materialization.");
		}
		bool_t bPresentationChanged = false;
		ImGui::BeginDisabled(!Light.bEnabled);
		bPresentationChanged |= ImGui::DragFloat("Light Range",
			&Light.fRange, 0.01f, 0.001f, 100000.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		bPresentationChanged |= ImGui::DragFloat("Light Intensity",
			&Light.fIntensity, 0.01f, 0.f, 100000.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		bPresentationChanged |= ImGui::ColorEdit4("Light Color",
			&Light.vColor.x, ImGuiColorEditFlags_Float |
			ImGuiColorEditFlags_HDR);
		bPresentationChanged |= ImGui::ColorEdit4("Ambient Color",
			&Light.vAmbient.x, ImGuiColorEditFlags_Float |
			ImGuiColorEditFlags_HDR);
		bPresentationChanged |= ImGui::DragFloat("Falloff Exponent",
			&Light.fFalloffExponent, 0.01f, 0.001f, 128.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		ImGui::EndDisabled();
		if (bPresentationChanged)
		{
			Light.eProfile = EFFECT_LIGHT_PROFILE::POINT_RECONSTRUCTED_V1;
			Light.eStatus =
				EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE;
			bChanged = true;
		}
		ImGui::TextDisabled(
			"Visible, Transform, Timing, Color Multiply, Solo and Delete use the same active Element transaction.");
        break;
	}
    case EFFECT_ELEMENT_KIND::SCREEN_POST:
	{
		EFFECT_SCREEN_POST_DETAIL_DESC& Post = Detail.ScreenPost;
		ImGui::SeparatorText("Presentation Screen Post");
		if (!Post.bEnabled)
		{
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
				"This source row has no admitted typed Screen Post payload. Delete or hide it here; enabling it requires source-backed materialization.");
		}
		bool_t bPresentationChanged = false;
		ImGui::BeginDisabled(!Post.bEnabled);
		if (ImGui::BeginCombo("Screen Post Profile",
			ScreenPostProfile_Label(Post.eProfile)))
		{
			for (uint8_t iProfile = 0u;
				iProfile < static_cast<uint8_t>(EFFECT_SCREEN_POST_PROFILE::END);
				++iProfile)
			{
				const EFFECT_SCREEN_POST_PROFILE eCandidate =
					static_cast<EFFECT_SCREEN_POST_PROFILE>(iProfile);
				const bool_t bSelected = eCandidate == Post.eProfile;
				if (ImGui::Selectable(ScreenPostProfile_Label(eCandidate),
					bSelected))
				{
					Post.eProfile = eCandidate;
					bPresentationChanged = true;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		bPresentationChanged |= ImGui::DragFloat("Post Intensity",
			&Post.fIntensity, 0.001f, 0.f, 100.f, "%.4f",
			ImGuiSliderFlags_AlwaysClamp);
		bPresentationChanged |= ImGui::DragFloat("Post Secondary Intensity",
			&Post.fSecondaryIntensity, 0.001f, 0.f, 100.f, "%.4f",
			ImGuiSliderFlags_AlwaysClamp);
		bPresentationChanged |= ImGui::DragFloat("Post Frequency",
			&Post.fFrequency, 0.01f, 0.f, 1000.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		bPresentationChanged |= ImGui::ColorEdit4("Post Tint",
			&Post.vTint.x, ImGuiColorEditFlags_Float |
			ImGuiColorEditFlags_HDR);
		if (ImGui::InputScalar("Post Random Seed", ImGuiDataType_U32,
			&Post.iRandomSeed))
		{
			Post.iRandomSeed = (std::max)(1u, Post.iRandomSeed);
			bPresentationChanged = true;
		}
		ImGui::EndDisabled();
		if (bPresentationChanged)
		{
			Post.eStatus =
				EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE;
			bChanged = true;
		}
		ImGui::TextDisabled(
			"The global Preview ScreenPost switch remains a non-persistent A/B gate; these fields persist only after Apply and Save.");
        break;
	}
    case EFFECT_ELEMENT_KIND::END:
    default:
        break;
    }
    if (Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
        Element.eKind == EFFECT_ELEMENT_KIND::SPRITE)
    {
        ImGui::SeparatorText("After Image");
        bChanged |= ImGui::DragFloat("AfterImage Sample Interval",
            &Detail.AfterImage.fSampleIntervalSeconds,
            0.001f, 0.001f, 30.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::InputScalar("AfterImage Max Copies",
            ImGuiDataType_U32, &Detail.AfterImage.iMaxCopies);
        bChanged |= ImGui::DragFloat("AfterImage Alpha Exponent",
            &Detail.AfterImage.fAlphaExponent,
            0.01f, 0.001f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    }
}

void Client::CEffect_Tool::Render_SourceRecipeDetail(
    EFFECT_CASCADE_RECIPE_DESC& Recipe,
    bool_t& bChanged,
	const bool_t bPortableReadOnly)
{
    if (!ImGui::CollapsingHeader("Original Emitter / Module Stack",
        ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

	ImGui::TextDisabled(bPortableReadOnly ?
		"Compiler-owned portable recipe. Runtime-owned Particle Detail controls and this module stack are read-only; authored overlays and supported resource overrides remain editable outside this section." :
        "Imported UE3 values. Editing changes only the Authored document; the Imported baseline remains unchanged.");
    if (Recipe.bAuthoredModuleOverrides)
        ImGui::TextDisabled("Authored module tuning: changes are saved in this Effect copy.");
    ImGui::BeginDisabled(bPortableReadOnly);
    bChanged |= ImGui::Checkbox("Execute Source Recipe", &Recipe.bEnabled);
    ImGui::Text("Renderer: %s",
        Recipe.strRendererShape.empty() ? "(unspecified)" :
            Recipe.strRendererShape.c_str());
    bChanged |= ImGui::DragFloat("Emitter Delay",
        &Recipe.fEmitterDelaySeconds, 0.001f, 0.f, 3600.f, "%.6f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::DragFloat("Emitter Duration",
        &Recipe.fEmitterDurationSeconds, 0.001f, 0.f, 3600.f, "%.6f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::InputScalar("Emitter Loop Count (0 = infinite)",
        ImGuiDataType_U32, &Recipe.iEmitterLoopCount);

    if (ImGui::TreeNodeEx("Bursts", ImGuiTreeNodeFlags_DefaultOpen,
        "Bursts (%zu)", Recipe.Bursts.size()))
    {
        for (size_t iBurst = 0u; iBurst < Recipe.Bursts.size(); ++iBurst)
        {
            EFFECT_PARTICLE_BURST_DESC& Burst = Recipe.Bursts[iBurst];
            ImGui::PushID(static_cast<int>(iBurst));
            ImGui::SeparatorText(("Burst " + std::to_string(iBurst)).c_str());
            bChanged |= ImGui::DragFloat("Time",
                &Burst.fTimeSeconds, 0.001f, 0.f, 3600.f, "%.6f",
                ImGuiSliderFlags_AlwaysClamp);
            bChanged |= ImGui::InputScalar(
                "Count Minimum", ImGuiDataType_U32, &Burst.iCountMinimum);
            bChanged |= ImGui::InputScalar(
                "Count Maximum", ImGuiDataType_U32, &Burst.iCountMaximum);
            Burst.iCountMaximum = (std::max)(
                Burst.iCountMinimum, Burst.iCountMaximum);
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    ImGui::SeparatorText("Dynamic Module Editors");
    ImGui::TextDisabled(
        "%zu modules; duplicate classes execute in source order.",
        Recipe.Modules.size());
    for (EFFECT_SOURCE_MODULE_DESC& Module : Recipe.Modules)
        Render_SourceModuleDetail(Module, bChanged);
    ImGui::EndDisabled();
}

void Client::CEffect_Tool::Render_SourceModuleDetail(
    EFFECT_SOURCE_MODULE_DESC& Module,
	bool_t& bChanged,
	const bool_t bDefaultOpen)
{
    ImGui::PushID(Module.strStableId.c_str());
	const SOURCE_MODULE_UI_DESC ModuleUI =
		Describe_SourceModule(Module.strClassName);
    const std::string strLabel = std::string(ModuleUI.pRole) + "##module";
	const ImGuiTreeNodeFlags Flags = bDefaultOpen ?
		ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None;
	if (ImGui::TreeNodeEx(strLabel.c_str(), Flags,
		"%s | %s | %zu values | %zu distributions",
		ModuleUI.pRole, Module.strClassName.c_str(), Module.Literals.size(),
        Module.Distributions.size()))
    {
		ImGui::TextWrapped("%s", ModuleUI.pDescription);
        ImGui::TextDisabled("Stable ID: %s", Module.strStableId.c_str());
		ImGui::TextDisabled("Source class: %s", Module.strClassName.c_str());
        ImGui::TextWrapped("Source: %s", Module.strObjectPath.c_str());

        if (!Module.Literals.empty() &&
            ImGui::TreeNodeEx("Literal Values", ImGuiTreeNodeFlags_DefaultOpen,
                "Literal Values (%zu)", Module.Literals.size()))
        {
            for (EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
            {
                ImGui::PushID(Literal.strPropertyPath.c_str());
				const std::string FriendlyLabel =
					Friendly_SourcePropertyLabel(Literal.strPropertyPath);
                switch (Literal.eKind)
                {
                case EFFECT_SOURCE_LITERAL_KIND::BOOLEAN:
                    bChanged |= ImGui::Checkbox(
						FriendlyLabel.c_str(), &Literal.bBoolean);
                    break;
                case EFFECT_SOURCE_LITERAL_KIND::NUMBER:
                {
                    const double fStep = 0.001;
                    bChanged |= ImGui::DragScalar(
						FriendlyLabel.c_str(), ImGuiDataType_Double,
                        &Literal.fNumber, 0.01f, nullptr, nullptr, "%.9g");
                    (void)fStep;
                    break;
                }
                case EFFECT_SOURCE_LITERAL_KIND::STRING:
                    ImGui::TextWrapped("%s = %s",
						FriendlyLabel.c_str(),
                        Literal.strString.c_str());
                    break;
                case EFFECT_SOURCE_LITERAL_KIND::END:
                default:
                    ImGui::TextDisabled("%s = (invalid)",
                        Literal.strPropertyPath.c_str());
                    break;
                }
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Source property: %s",
						Literal.strPropertyPath.c_str());
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

		for (EFFECT_DISTRIBUTION_DESC& Distribution : Module.Distributions)
			Render_SourceDistributionDetail(
				Distribution, Module.strClassName, bChanged);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void Client::CEffect_Tool::Render_SourceDistributionDetail(
    EFFECT_DISTRIBUTION_DESC& Distribution,
	const std::string_view strModuleClassName,
    bool_t& bChanged)
{
    ImGui::PushID(Distribution.strPropertyPath.c_str());
	const SOURCE_MODULE_UI_DESC ModuleUI =
		Describe_SourceModule(strModuleClassName);
	const std::string FriendlyLabel =
		Friendly_SourcePropertyLabel(Distribution.strPropertyPath);
	if (ImGui::TreeNodeEx("##distribution", ImGuiTreeNodeFlags_None,
		"%s / %s | %uD | keys %zu | table %zu",
		ModuleUI.pRole, FriendlyLabel.c_str(),
        Distribution.iComponentCount, Distribution.Keys.size(),
        Distribution.LookupTable.size()))
    {
		ImGui::TextDisabled("Source property: %s",
			Distribution.strPropertyPath.c_str());
        ImGui::TextDisabled("Distribution: %s",
            Distribution.strSourceClass.empty() ? "inline cooked table" :
                Distribution.strSourceClass.c_str());
        if (!Distribution.strSourceObjectPath.empty())
            ImGui::TextWrapped("Source: %s",
                Distribution.strSourceObjectPath.c_str());
        bChanged |= ImGui::InputScalar(
            "Operation", ImGuiDataType_U32, &Distribution.iOperation);
        Distribution.iOperation = (std::min)(3u, Distribution.iOperation);
        bChanged |= ImGui::DragFloat("Lookup Time Scale",
            &Distribution.fLookupTableTimeScale, 0.001f,
            -100000.f, 100000.f, "%.9g");
        bChanged |= ImGui::DragFloat("Lookup Start Time",
            &Distribution.fLookupTableStartTime, 0.001f,
            -100000.f, 100000.f, "%.9g");
        bChanged |= DragFloat4("Default Minimum",
            Distribution.vDefaultMinimum, 0.001f, -100000.f, 100000.f,
            "%.6f");
        bChanged |= DragFloat4("Default Maximum",
            Distribution.vDefaultMaximum, 0.001f, -100000.f, 100000.f,
            "%.6f");

        if (!Distribution.Keys.empty() &&
            ImGui::TreeNodeEx("Curve Keys", ImGuiTreeNodeFlags_None,
                "Curve Keys (%zu)", Distribution.Keys.size()))
        {
            for (size_t iKey = 0u; iKey < Distribution.Keys.size(); ++iKey)
            {
                EFFECT_DISTRIBUTION_KEY_DESC& Key = Distribution.Keys[iKey];
                ImGui::PushID(static_cast<int>(iKey));
                ImGui::SeparatorText(("Key " + std::to_string(iKey)).c_str());
                bChanged |= ImGui::DragFloat("Time", &Key.fTime,
                    0.001f, -100000.f, 100000.f, "%.9g");
                bChanged |= DragFloat4("Minimum", Key.vMinimum,
                    0.001f, -100000.f, 100000.f, "%.6f");
                bChanged |= DragFloat4("Maximum", Key.vMaximum,
                    0.001f, -100000.f, 100000.f, "%.6f");
                bChanged |= DragFloat4("Arrive Tangent Minimum",
                    Key.vArriveTangentMinimum, 0.001f,
                    -100000.f, 100000.f, "%.6f");
                bChanged |= DragFloat4("Leave Tangent Minimum",
                    Key.vLeaveTangentMinimum, 0.001f,
                    -100000.f, 100000.f, "%.6f");
                bChanged |= DragFloat4("Arrive Tangent Maximum",
                    Key.vArriveTangentMaximum, 0.001f,
                    -100000.f, 100000.f, "%.6f");
                bChanged |= DragFloat4("Leave Tangent Maximum",
                    Key.vLeaveTangentMaximum, 0.001f,
                    -100000.f, 100000.f, "%.6f");
                int32_t iInterpolation = static_cast<int32_t>(
                    Key.eInterpolation);
                constexpr const char* INTERPOLATION_LABELS[] =
                {
                    "Constant", "Linear", "Cubic"
                };
                if (ImGui::Combo("Interpolation", &iInterpolation,
                    INTERPOLATION_LABELS,
                    static_cast<int>(std::size(INTERPOLATION_LABELS))))
                {
                    Key.eInterpolation =
                        static_cast<EFFECT_DISTRIBUTION_INTERPOLATION>(
                            iInterpolation);
                    bChanged = true;
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        if (!Distribution.LookupTable.empty() &&
            ImGui::TreeNodeEx("Cooked Lookup Table", ImGuiTreeNodeFlags_None,
                "Cooked Lookup Table (%zu)", Distribution.LookupTable.size()))
        {
            ImGui::TextDisabled(
                "The runtime evaluates this table before source curve keys.");
            ImGuiListClipper Clipper;
            Clipper.Begin(static_cast<int>(Distribution.LookupTable.size()));
            while (Clipper.Step())
            {
                for (int32_t iValue = Clipper.DisplayStart;
                    iValue < Clipper.DisplayEnd; ++iValue)
                {
                    ImGui::PushID(iValue);
                    bChanged |= ImGui::DragFloat(
                        std::to_string(iValue).c_str(),
                        &Distribution.LookupTable[
                            static_cast<size_t>(iValue)],
                        0.001f, -100000.f, 100000.f, "%.9g");
                    ImGui::PopID();
                }
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void Client::CEffect_Tool::Render_LerpDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged,
	const bool_t bLockProjectTunedCarrierColor)
{
    if (!ImGui::CollapsingHeader("Linear Lerp",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;
    EFFECT_LINEAR_LERP_DESC& Lerp = Detail.LinearLerp;
    ImGui::TextDisabled(
        "Enable a Lerp checkbox to interpolate its Start value to End over Lifetime. Enabling restarts live preview.");
    const auto RenderLerpToggle = [this, &bChanged](
        const char* pLabel,
        bool_t& bEnabled)
    {
        const bool_t bWasEnabled = bEnabled;
        const bool_t bToggleChanged = ImGui::Checkbox(pLabel, &bEnabled);
        bChanged |= bToggleChanged;
        if (bToggleChanged && !bWasEnabled && bEnabled)
        {
			/* The draft must stage before both clocks restart. A rejected
			   resource/material edit must not consume the current animation. */
			m_bDetailDraftPreviewRestartRequested = true;
        }
    };
    RenderLerpToggle("Lerp Position", Lerp.bPosition);
    if (Lerp.bPosition)
        bChanged |= DragFloat3(
            "Position End", Lerp.vEndPosition, 0.01f, -1000.f, 1000.f);
    RenderLerpToggle("Lerp Rotation", Lerp.bRotation);
    if (Lerp.bRotation)
        bChanged |= DragFloat3(
            "Rotation End", Lerp.vEndRotationDegrees, 0.25f, -360.f, 360.f);
    RenderLerpToggle("Lerp Revolution", Lerp.bRevolution);
    if (Lerp.bRevolution)
        bChanged |= DragFloat3("Revolution End",
            Lerp.vEndRevolutionDegreesPerSecond, 0.5f, -3600.f, 3600.f);
    RenderLerpToggle("Lerp Scaling", Lerp.bScale);
    if (Lerp.bScale)
        bChanged |= DragFloat3(
            "Scaling End", Lerp.vEndScale, 0.01f, 0.001f, 100.f);
    RenderLerpToggle("Lerp Velocity", Lerp.bVelocity);
    if (Lerp.bVelocity)
        bChanged |= DragFloat3("Velocity End",
            Lerp.vEndVelocityPerSecond, 0.01f, -1000.f, 1000.f);
	ImGui::BeginDisabled(bLockProjectTunedCarrierColor);
    RenderLerpToggle("Lerp ColorOffset", Lerp.bColorOffset);
    if (Lerp.bColorOffset)
        bChanged |= DragFloat4(
            "ColorOffset End", Lerp.vEndColorOffset, 0.01f, -10.f, 10.f);
    RenderLerpToggle("Lerp Color Multiply", Lerp.bColorMultiply);
    if (Lerp.bColorMultiply)
        bChanged |= DragFloat4("Color Multiply End",
            Lerp.vEndColorMultiply, 0.01f, 0.f, 10.f);
	RenderLerpToggle("Lerp Emissive Intensity (HDR)",
		Lerp.bEmissiveIntensity);
	if (Lerp.bEmissiveIntensity)
		bChanged |= ImGui::DragFloat("Emissive Intensity End##lerp",
			&Lerp.fEndEmissiveIntensity, 0.05f, 0.f, 100.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	if (bLockProjectTunedCarrierColor)
	{
		ImGui::TextDisabled(
			"Carrier color/emission lerps are locked for opcode 1003/1004; Project Tuned Surface owns those channels.");
	}
	if (Detail.Mesh.RingFill.bEnabled)
	{
		ImGui::SeparatorText("Mesh Ring Fill");
		const bool_t bWasRingFillLerpEnabled = Lerp.bRingFillProgress;
		RenderLerpToggle("Lerp Ring Fill Progress", Lerp.bRingFillProgress);
		if (bWasRingFillLerpEnabled && !Lerp.bRingFillProgress)
			Lerp.fEndRingFillProgress = 1.f;
		if (Lerp.bRingFillProgress)
		{
			bChanged |= ImGui::SliderFloat("Ring Fill Progress End##lerp",
				&Lerp.fEndRingFillProgress, 0.f, 1.f, "%.3f",
				ImGuiSliderFlags_AlwaysClamp);
		}
	}
}

void Client::CEffect_Tool::Render_AssemblyHierarchy(
	const std::string& strEffectAssetId)
{
	const std::shared_ptr<const EFFECT_ASSEMBLY_DESC> Assembly =
		CEffectCatalog::Find_Assembly(strEffectAssetId);
	if (nullptr == Assembly)
	{
		ImGui::TextDisabled("Runtime Assembly is not admitted.");
		return;
	}
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC> RuntimeDocument =
		CEffectCatalog::Find(strEffectAssetId);
	const PARTICLE_LAYER_SUMMARY ParticleSummary = nullptr != RuntimeDocument ?
		Summarize_ParticleLayers(*RuntimeDocument) : PARTICLE_LAYER_SUMMARY{};
	const bool_t bParticleSystemSelected =
		m_ActiveDocument.has_value() &&
		m_ActiveDocument->strEffectAssetId == strEffectAssetId &&
		EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM == m_eDetailSelection;
	const std::string ParticleSystemLabel = "Cascade System | Emitters " +
		std::to_string(ParticleSummary.iSourceEmitterCount) +
		" | Mesh Particles " +
		std::to_string(ParticleSummary.iMeshRendererCount) +
		" | Sprite Particles " +
		std::to_string(ParticleSummary.iSpriteRendererCount) + " | Unresolved " +
		std::to_string(ParticleSummary.iUnresolvedRendererCount);
	const bool_t bParticleSystemOpen = ImGui::TreeNodeEx(
		(ParticleSystemLabel + "##particle-system." + strEffectAssetId).c_str(),
		ImGuiTreeNodeFlags_OpenOnArrow |
		(bParticleSystemSelected ? ImGuiTreeNodeFlags_Selected : 0));
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		Try_SelectParticleSystem(strEffectAssetId);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(
			"Aggregate controls preserve all source Emitters. Open a Component or "
			"Emitter below for source-level editing.");
	if (bParticleSystemOpen)
	{
		ImGui::TextDisabled("Source Systems %zu | Budget %llu",
			ParticleSummary.iSourceSystemCount,
			static_cast<unsigned long long>(ParticleSummary.iParticleBudget));
		if (ParticleSummary.iSourceEmitterCount > 0u &&
			ImGui::SmallButton("Open First Emitter"))
		{
			Try_SelectFirstEmitter(strEffectAssetId, {});
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode(("Timeline (" +
		std::to_string(Assembly->ComponentCues.size()) + " Component Cues)##" +
		strEffectAssetId).c_str()))
	{
		for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly->ComponentCues)
			ImGui::BulletText("%.3f s | %s", Cue.fStartDelaySeconds,
				Cue.strComponentAssetId.c_str());
		if (!Assembly->ModelCues.empty() && ImGui::TreeNode(
			("Animated Model Cues (" +
				std::to_string(Assembly->ModelCues.size()) + ")").c_str()))
		{
			for (const EFFECT_MODEL_CUE_DESC& Cue : Assembly->ModelCues)
				ImGui::BulletText("%.3f s | %s | %s",
					Cue.fStartDelaySeconds, Cue.strClipName.c_str(),
					Cue.strModelAssetId.c_str());
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (!ImGui::TreeNode(("Components (" +
		std::to_string(Assembly->ComponentCues.size()) + ")##components." +
		strEffectAssetId).c_str()))
	{
		return;
	}
	for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly->ComponentCues)
	{
		const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
			CEffectCatalog::Find_Component(Cue.strComponentAssetId);
		if (nullptr == Component)
		{
			ImGui::BulletText("MISSING %s", Cue.strComponentAssetId.c_str());
			continue;
		}
		ImGui::PushID(Component->strComponentAssetId.c_str());
		const bool_t bSelected =
			m_strSelectedComponentId == Component->strComponentAssetId &&
			m_eDetailSelection >= EFFECT_DETAIL_SELECTION::COMPONENT;
		const std::string Label = Component->strDisplayName + " | " +
			Component->strComponentType + " | " +
			std::to_string(Component->Emitters.size()) + " Emitters";
		const bool_t bOpen = ImGui::TreeNodeEx(Label.c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow |
			(bSelected ? ImGuiTreeNodeFlags_Selected : 0));
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			Try_SelectComponent(strEffectAssetId,
				Component->strComponentAssetId);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Stable ID: %s\nSource Group: %s\nCue: %.3f s",
				Component->strComponentAssetId.c_str(),
				Component->strSourceGroupId.c_str(), Cue.fStartDelaySeconds);
		if (bOpen)
		{
			for (const EFFECT_COMPONENT_EMITTER_DESC& Emitter :
				Component->Emitters)
			{
				const auto ElementIterator = std::find_if(
					Component->Document.Elements.begin(),
					Component->Document.Elements.end(),
					[&Emitter](const EFFECT_ELEMENT_DESC& Element)
					{
						return Element.strElementId == Emitter.strElementId;
					});
				if (Component->Document.Elements.end() == ElementIterator)
					continue;
				const EFFECT_ELEMENT_DESC& Element = *ElementIterator;
				ImGui::PushID(Emitter.strEmitterId.c_str());
				const bool_t bEmitterSelected = bSelected &&
					m_strSelectedEmitterId == Emitter.strEmitterId &&
					m_eDetailSelection >= EFFECT_DETAIL_SELECTION::EMITTER;
				const std::string EmitterLabel = Element.strDisplayName + " | " +
					Element_RendererLabel(Element) + " | Runtime " +
					Emitter.strRendererType + " | " +
					std::to_string(Emitter.iModuleCount) + " Modules";
				const bool_t bEmitterOpen = ImGui::TreeNodeEx(
					EmitterLabel.c_str(), ImGuiTreeNodeFlags_OpenOnArrow |
					(bEmitterSelected ? ImGuiTreeNodeFlags_Selected : 0));
				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
					Try_SelectEmitter(strEffectAssetId,
						Component->strComponentAssetId, Emitter.strEmitterId);
				if (bEmitterOpen)
				{
					ImGui::BulletText("Renderer: %s",
						Emitter.strRendererType.c_str());
					if (ImGui::TreeNode(("Resources (" +
						std::to_string(Element.ResourceBindings.size()) + ")").c_str()))
					{
						for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
							Element.ResourceBindings)
						{
							ImGui::BulletText("%s: %s", Binding.strSlotId.c_str(),
								Binding.strAssetId.c_str());
						}
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(("Modules (" +
						std::to_string(Element.SourceRecipe.Modules.size()) + ")").c_str()))
					{
						for (const EFFECT_SOURCE_MODULE_DESC& Module :
							Element.SourceRecipe.Modules)
						{
							const bool_t bModuleSelected = bEmitterSelected &&
								m_eDetailSelection ==
									EFFECT_DETAIL_SELECTION::SOURCE_MODULE &&
								m_strSelectedSourceModuleId == Module.strStableId;
							const std::string ModuleLabel = Module.strClassName + "##" +
								Module.strStableId;
							if (ImGui::Selectable(ModuleLabel.c_str(), bModuleSelected))
								Try_SelectSourceModule(strEffectAssetId,
									Component->strComponentAssetId,
									Emitter.strEmitterId, Module.strStableId);
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("%s\n%s", Module.strStableId.c_str(),
									Module.strObjectPath.c_str());
						}
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::TreePop();
}

void Client::CEffect_Tool::Render_VisualProgramAuthoring(
	const EFFECT_SKILL_TREE_ENTRY& Entry,
	const size_t iCueIndex)
{
	if (iCueIndex >= Entry.ProductCues.size())
		return;
	const std::string& strEffectAssetId =
		Entry.ProductCues[iCueIndex].Cue.strEffectAssetId;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> Program =
		CEffectCatalog::Find_VisualProgram(strEffectAssetId);
	if (nullptr == Program)
		return;
	const bool_t bArtistFAdapter =
		strEffectAssetId == ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
		Program->eProjectionKind ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		AuthoringProjection = bArtistFAdapter ?
			(ARTIST_F_PREPARATION_STATE::READY ==
					m_eArtistFSourcePreparationState &&
			 m_iArtistFSourceSnapshotRevision ==
				CEffectCatalog::Get_RuntimeRevision() ?
				m_pArtistFSourceProjection : nullptr) :
			CEffectCatalog::Find_VisualProjection_Loaded(strEffectAssetId);
	const bool_t bAdapterPacketProgram = Program->eProjectionKind ==
		EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	if (bAdapterPacketProgram && nullptr == AuthoringProjection)
		ImGui::TextDisabled(
			"This source recipe is unavailable for migration.");
	const auto IsAuthorableVisualRow =
		[&AuthoringProjection](const EFFECT_VISUAL_PROGRAM_ROW& Row)
		{
			if (nullptr == AuthoringProjection ||
				Row.eDisposition !=
					EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
				!Row.TargetIdentity.has_value())
			{
				return false;
			}
			const EFFECT_VISUAL_PROGRAM_ROW* pProjectedRow =
				AuthoringProjection->Find_RowByOccurrenceId(
					Row.Selector.strOccurrenceId);
			return nullptr != pProjectedRow &&
				pProjectedRow->eDisposition ==
					EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED &&
				pProjectedRow->strRowSha256 == Row.strRowSha256 &&
				pProjectedRow->TargetIdentity.has_value() &&
				pProjectedRow->TargetIdentity->strTargetElementId ==
					Row.TargetIdentity->strTargetElementId &&
				pProjectedRow->SourceIdentity.strSourceRecordId ==
					Row.SourceIdentity.strSourceRecordId;
		};
	const auto IsAuthorableSupplementalRow =
		[&AuthoringProjection](
			const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Row)
		{
			if (nullptr == AuthoringProjection ||
				Row.eDisposition !=
					EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
				Row.TargetIdentity.strTargetElementId.empty())
			{
				return false;
			}
			const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pProjectedRow =
				AuthoringProjection->Find_SupplementalElementByOccurrenceId(
					Row.Selector.strOccurrenceId);
			return nullptr != pProjectedRow &&
				pProjectedRow->eDisposition ==
					EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED &&
				pProjectedRow->strRowSha256 == Row.strRowSha256 &&
				pProjectedRow->TargetIdentity.strTargetElementId ==
					Row.TargetIdentity.strTargetElementId &&
				pProjectedRow->strSourceRecordId == Row.strSourceRecordId;
		};

	static constexpr std::array<EFFECT_VISUAL_PROGRAM_FAMILY, 7u> FAMILIES{
		EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE,
		EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE,
		EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE,
		EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON,
		EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL,
		EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE,
		EFFECT_VISUAL_PROGRAM_FAMILY::SCREEN_POST };
	if (!ImGui::TreeNodeEx("Track A Element Seeds",
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
	{
		return;
	}
	ImGui::TextDisabled(
		"Read-only source data. Load Seed copies WModel/DDS and safe starter values into Element Authoring; it never changes Current Effect by itself.");
	for (const EFFECT_VISUAL_PROGRAM_FAMILY eFamily : FAMILIES)
	{
		const size_t iRowCount = static_cast<size_t>(std::count_if(
			Program->VisualRows.begin(), Program->VisualRows.end(),
			[eFamily, &IsAuthorableVisualRow](
				const EFFECT_VISUAL_PROGRAM_ROW& Row)
			{ return Row.eFamily == eFamily && IsAuthorableVisualRow(Row); })) +
			static_cast<size_t>(std::count_if(
				Program->SupplementalElements.begin(),
				Program->SupplementalElements.end(),
				[eFamily, &IsAuthorableSupplementalRow](
					const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Row)
				{ return Row.eFamily == eFamily &&
					IsAuthorableSupplementalRow(Row); }));
		if (0u == iRowCount)
			continue;
		ImGui::PushID(static_cast<int>(eFamily));
		const std::string FamilyLabel =
			std::string(VisualProgramFamilyLabel(eFamily)) + " (" +
			std::to_string(iRowCount) + ")";
		if (ImGui::TreeNode(FamilyLabel.c_str()))
		{
			size_t iElementOrdinal = 0u;
			for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program->VisualRows)
			{
				if (Row.eFamily != eFamily)
					continue;
				ImGui::PushID(Row.Selector.strOccurrenceId.c_str());
				if (!IsAuthorableVisualRow(Row))
				{
					ImGui::PopID();
					continue;
				}
				++iElementOrdinal;
				const bool_t bSelected =
					m_SourceElementPresetSelection.has_value() &&
					m_SourceElementPresetSelection->strSourceEffectAssetId ==
						strEffectAssetId &&
					m_SourceElementPresetSelection->strOccurrenceId ==
						Row.Selector.strOccurrenceId &&
					m_SourceElementPresetSelection->strRowSha256 == Row.strRowSha256;
				const std::string Label = VisualProgramElementRowLabel(
					eFamily, iElementOrdinal,
					Row.SourceIdentity.strSourceRecordId,
					Row.Resources);
				if (bSelected)
					ImGui::TextColored(ImVec4(0.36f, 0.72f, 1.f, 1.f),
						"%s", Label.c_str());
				else
					ImGui::TextWrapped("%s", Label.c_str());
				const bool_t bOccurrenceHovered = ImGui::IsItemHovered();
				ImGui::SameLine();
				if (ImGui::SmallButton("Load Seed"))
					Try_OpenVisualProgramElementForAuthoring(strEffectAssetId,
						Row.Selector.strOccurrenceId, Row.strRowSha256,
						Row.TargetIdentity->strTargetElementId,
						Row.SourceIdentity.strSourceRecordId);
				if (bOccurrenceHovered)
				{
					ImGui::SetTooltip(
						"Source: %s\nSlots: %s\nLoad Seed copies this immutable source into Element Authoring. Use Create Element, then Save Changes.",
						Row.SourceIdentity.strSourceRecordId.c_str(),
						VisualProgramResourceSlotSummary(Row.Resources).c_str());
				}
				ImGui::PopID();
			}
			for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Row :
				Program->SupplementalElements)
			{
				if (Row.eFamily != eFamily)
					continue;
				ImGui::PushID(Row.Selector.strOccurrenceId.c_str());
				if (!IsAuthorableSupplementalRow(Row))
				{
					ImGui::PopID();
					continue;
				}
				++iElementOrdinal;
				const bool_t bSelected =
					m_SourceElementPresetSelection.has_value() &&
					m_SourceElementPresetSelection->strSourceEffectAssetId ==
						strEffectAssetId &&
					m_SourceElementPresetSelection->strOccurrenceId ==
						Row.Selector.strOccurrenceId &&
					m_SourceElementPresetSelection->strRowSha256 == Row.strRowSha256;
				std::string Label = VisualProgramElementRowLabel(
					eFamily, iElementOrdinal, Row.strSourceRecordId,
					Row.Resources);
				if (bSelected)
					ImGui::TextColored(ImVec4(0.36f, 0.72f, 1.f, 1.f),
						"%s", Label.c_str());
				else
					ImGui::TextWrapped("%s", Label.c_str());
				const bool_t bOccurrenceHovered = ImGui::IsItemHovered();
				ImGui::SameLine();
				if (ImGui::SmallButton("Load Seed"))
					Try_OpenVisualProgramElementForAuthoring(strEffectAssetId,
						Row.Selector.strOccurrenceId, Row.strRowSha256,
						Row.TargetIdentity.strTargetElementId,
						Row.strSourceRecordId);
				if (bOccurrenceHovered)
				{
					ImGui::SetTooltip(
						"Source: %s\nSlots: %s\nLoad Seed copies this immutable source into Element Authoring. Use Create Element, then Save Changes.",
						Row.strSourceRecordId.c_str(),
						VisualProgramResourceSlotSummary(Row.Resources).c_str());
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::TreePop();
}

void Client::CEffect_Tool::Render_ArtistFCoreAuthoring()
{
	/* Rendering the tree must stay metadata-only.  Source projection and typed
	   material preparation synchronously prewarm WModels/DDS resources, so they
	   are initiated only by Load Seed/Upgrade below. */
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		CEffectCatalog::Find_RuntimeProgramEntry(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		nullptr == pEntry ? nullptr : pEntry->Get_Program();
	const uint64_t iRuntimeRevision = CEffectCatalog::Get_RuntimeRevision();
	const bool_t bSourcePreparedForRevision =
		ARTIST_F_PREPARATION_STATE::READY ==
			m_eArtistFSourcePreparationState &&
		m_iArtistFSourceSnapshotRevision == iRuntimeRevision;
	const bool_t bMaterialPreparedForRevision =
		ARTIST_F_PREPARATION_STATE::READY ==
			m_eArtistFMaterialPreparationState &&
		m_iArtistFMaterialExecutionSnapshotRevision == iRuntimeRevision;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProjection = bSourcePreparedForRevision ?
			m_pArtistFSourceProjection : nullptr;
	const bool_t bEditableSkillEffectActive =
		m_ActiveDocument.has_value() &&
		(EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource ||
		 EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource) &&
		m_ActiveDocument->strEffectAssetId ==
			ARTIST_F_UNIFIED_EFFECT_ASSET_ID;
	if (nullptr == pProgram ||
		pProgram->strRuntimeCatalogAssetId != ARTIST_F_VISUAL_PROGRAM_ASSET_ID)
	{
		ImGui::TextDisabled("Artist F source recipe is unavailable.");
		if (!m_strArtistFSourceSnapshotStatus.empty())
			ImGui::TextWrapped("%s", m_strArtistFSourceSnapshotStatus.c_str());
		return;
	}
	if ((ARTIST_F_PREPARATION_STATE::FAILED ==
			m_eArtistFSourcePreparationState &&
		 m_iArtistFSourcePreparationAttemptRevision == iRuntimeRevision) ||
		(ARTIST_F_PREPARATION_STATE::FAILED ==
			m_eArtistFMaterialPreparationState &&
		 m_iArtistFMaterialPreparationAttemptRevision == iRuntimeRevision))
	{
		ImGui::TextColored(ImVec4(1.f, 0.55f, 0.25f, 1.f),
			"Track A preparation failed. Refresh after source/catalog changes before retrying.");
		if (!m_strArtistFSourceSnapshotStatus.empty())
			ImGui::TextWrapped("%s", m_strArtistFSourceSnapshotStatus.c_str());
	}
	else if (!bMaterialPreparedForRevision)
	{
		ImGui::TextDisabled(
			"Track A seed resources are prepared only when Load Seed is pressed.");
	}

	const bool_t bRootOpen = ImGui::TreeNodeEx(
		"Track A Element Seeds (33)##artist-f-seeds",
		ImGuiTreeNodeFlags_OpenOnArrow);
	if (!bRootOpen)
		return;
	ImGui::TextDisabled(
		"Read-only source library. Open Editable Skill Effect above, then Load Seed -> Create Element -> tune -> Save Changes.");
	if (!bEditableSkillEffectActive)
	{
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
			"Open the Artist F Editable Skill Effect first. Seeds are ingredients, not a playable Effect.");
	}

	static constexpr std::array<EFFECT_GPU_RENDER_FAMILY, 4u> FAMILIES{
		EFFECT_GPU_RENDER_FAMILY::MESH,
		EFFECT_GPU_RENDER_FAMILY::SPRITE,
		EFFECT_GPU_RENDER_FAMILY::DECAL,
		EFFECT_GPU_RENDER_FAMILY::RIBBON };
	static constexpr std::array<size_t, 4u> EXPECTED_COUNTS{
		13u, 16u, 3u, 1u };
	const auto LoadSeed = [this](
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter) -> bool_t
	{
		if (!Ensure_ArtistFMaterialExecutionSnapshots())
		{
			m_strElementStatus = m_strArtistFSourceSnapshotStatus.empty() ?
				"Artist F Track A material snapshots are unavailable." :
				m_strArtistFSourceSnapshotStatus;
			return false;
		}
		const std::shared_ptr<const
			EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pProjection =
			m_pArtistFSourceProjection;
		if (nullptr == pProjection || !pProjection->Is_Valid())
		{
			m_strElementStatus =
				"Artist F Track A source projection is unavailable after preparation.";
			return false;
		}
		const std::string strVisualOccurrenceId =
			Emitter.strMaterialOccurrenceId.value_or(Emitter.Row.strId);
		const EFFECT_VISUAL_PROGRAM_ROW* pVisualRow =
			pProjection->Find_RowByOccurrenceId(strVisualOccurrenceId);
		const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pSupplemental =
			pProjection->Find_SupplementalElementByOccurrenceId(
				strVisualOccurrenceId);
		if (nullptr != pVisualRow && nullptr != pSupplemental)
		{
			m_strElementStatus =
				"The Track A seed matched both a visual row and a supplemental Element.";
			return false;
		}
		const bool_t bRequiresStrictPreset =
			strVisualOccurrenceId == "source-active-003" ||
			strVisualOccurrenceId == "source-active-020" ||
			strVisualOccurrenceId == "source-active-021";
		if (bRequiresStrictPreset && nullptr == pVisualRow &&
			nullptr == pSupplemental)
		{
			m_strElementStatus =
				"The typed Artist F Decal/Ribbon seed lost its admitted Track A identity.";
			return false;
		}
		bool_t bLoaded = false;
		if (nullptr != pVisualRow)
		{
			if (!pVisualRow->TargetIdentity.has_value() ||
				pVisualRow->TargetIdentity->strTargetElementId !=
					Emitter.strSourceElementId)
			{
				m_strElementStatus =
					"The admitted Track A seed target no longer matches the selected source Element.";
				return false;
			}
			bLoaded = Try_OpenVisualProgramElementForAuthoring(
				ARTIST_F_VISUAL_PROGRAM_ASSET_ID,
				pVisualRow->Selector.strOccurrenceId,
				pVisualRow->strRowSha256,
				pVisualRow->TargetIdentity->strTargetElementId,
				pVisualRow->SourceIdentity.strSourceRecordId);
		}
		else if (nullptr != pSupplemental)
		{
			if (pSupplemental->TargetIdentity.strTargetElementId !=
				Emitter.strSourceElementId)
			{
				m_strElementStatus =
					"The admitted Track A supplemental target no longer matches the selected source Element.";
				return false;
			}
			bLoaded = Try_OpenVisualProgramElementForAuthoring(
				ARTIST_F_VISUAL_PROGRAM_ASSET_ID,
				pSupplemental->Selector.strOccurrenceId,
				pSupplemental->strRowSha256,
				pSupplemental->TargetIdentity.strTargetElementId,
				pSupplemental->strSourceRecordId);
		}
		else
		{
			bLoaded = Try_OpenArtistFReconstructedElementForAuthoring(Emitter);
		}
		if (!bLoaded)
			return false;

		const auto Source = std::find_if(
			pProjection->Get_Document().Elements.begin(),
			pProjection->Get_Document().Elements.end(),
			[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
			{ return Candidate.strElementId == Emitter.strSourceElementId; });
		std::string Error;
		if (Source == pProjection->Get_Document().Elements.end() ||
			!Try_ApplyArtistFTrackASeedData(
				Emitter, *Source, m_MeshAuthoringDraft, Error))
		{
			m_strElementStatus = Error.empty() ?
				"Artist F Track A seed data could not be normalized for authoring." :
				Error;
			return false;
		}
		const auto Registry = Emitter.strMaterialOccurrenceId.has_value() ?
			Find_Artist31470ShaderRegistry(
				Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId) :
			std::nullopt;
		if (!Registry.has_value())
		{
			m_strElementStatus =
				"Artist F Track A seed lost its shader-registry identity.";
			return false;
		}
		if (m_MeshAuthoringDraft.Material.Execution.bEnabled)
		{
			m_strElementStatus =
				"Loaded Artist F Track A seed with its typed Material slots, fixed burst, local-space, WModel import scale, and constant-one DynamicParameter fallback where consumed. Attachment basis remains emit-start baked. Ribbon history restoration is outside this pass. Use Create Element, then Save Changes.";
		}
		else if (Registry->eBackend ==
			EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON)
		{
			m_strElementStatus =
				"Loaded Artist F #17 FiniteCommon seed with its bounded standard authored Material. Use Create Element, tune, then Save Changes.";
		}
		else
		{
			m_strElementStatus =
				"Loaded an Artist F fail-closed seed (#1/#16/#26/#33). It is invisible by default because its missing SceneColor/depth/fog/MRT contract must not be replaced by a generic white/distortion fallback.";
		}
		return true;
	};
	for (size_t iFamilyIndex = 0u; iFamilyIndex < FAMILIES.size();
		++iFamilyIndex)
	{
		const EFFECT_GPU_RENDER_FAMILY eFamily = FAMILIES[iFamilyIndex];
		std::vector<const EFFECT_RUNTIME_PROGRAM_EMITTER*> Emitters;
		for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : pProgram->Emitters)
		{
			EFFECT_GPU_RENDER_FAMILY eEmitterFamily =
				EFFECT_GPU_RENDER_FAMILY::END;
			if (!Emitter.bVisible ||
				!Try_ResolveArtistCoreFamily(Emitter.eRenderer, eEmitterFamily) ||
				eEmitterFamily != eFamily)
			{
				continue;
			}
			if (nullptr == pProjection)
			{
				Emitters.push_back(&Emitter);
				continue;
			}
			const auto Element = std::find_if(
				pProjection->Get_Document().Elements.begin(),
				pProjection->Get_Document().Elements.end(),
				[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId == Emitter.strSourceElementId;
				});
			if (Element != pProjection->Get_Document().Elements.end() &&
				Element->bVisible)
			{
				Emitters.push_back(&Emitter);
			}
		}

		ImGui::PushID(static_cast<int>(eFamily));
		const std::string FamilyLabel =
			std::string(ArtistCoreFamilyLabel(eFamily)) + " (" +
			std::to_string(Emitters.size()) + ")";
		if (ImGui::TreeNodeEx(FamilyLabel.c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow))
		{
			if (Emitters.size() != EXPECTED_COUNTS[iFamilyIndex])
			{
				ImGui::TextColored(ImVec4(1.f, 0.55f, 0.25f, 1.f),
					"Fail closed: expected %zu visible Elements, found %zu.",
					EXPECTED_COUNTS[iFamilyIndex], Emitters.size());
			}
			for (size_t iElement = 0u; iElement < Emitters.size(); ++iElement)
			{
				const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter = *Emitters[iElement];
				ImGui::PushID(Emitter.Row.strId.c_str());
				const EFFECT_ELEMENT_DESC* pElement = nullptr;
				if (nullptr != pProjection)
				{
					const auto ProjectedElement = std::find_if(
						pProjection->Get_Document().Elements.begin(),
						pProjection->Get_Document().Elements.end(),
						[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
						{
							return Candidate.strElementId ==
								Emitter.strSourceElementId;
						});
					if (ProjectedElement !=
						pProjection->Get_Document().Elements.end())
					{
						pElement = &*ProjectedElement;
					}
				}
				const std::string strVisualOccurrenceId =
					Emitter.strMaterialOccurrenceId.value_or(Emitter.Row.strId);
				const bool_t bUsesAdmittedVisualSeed =
					nullptr != pProjection &&
					(nullptr != pProjection->Find_RowByOccurrenceId(
						strVisualOccurrenceId) ||
					 nullptr != pProjection->Find_SupplementalElementByOccurrenceId(
						strVisualOccurrenceId));
				const std::string& strSelectionOccurrenceId =
					bUsesAdmittedVisualSeed ? strVisualOccurrenceId :
						Emitter.Row.strId;
				const bool_t bSelected =
					m_SourceElementPresetSelection.has_value() &&
					m_SourceElementPresetSelection->strSourceEffectAssetId ==
						ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
					m_SourceElementPresetSelection->strOccurrenceId ==
						strSelectionOccurrenceId;
				const std::string Label =
					std::string(ArtistCoreFamilyLabel(eFamily)) + " " +
					(iElement + 1u < 10u ? "0" : "") +
					std::to_string(iElement + 1u) + " | " +
					(nullptr == pElement ?
						StableIdentityLeaf(Emitter.strSourceEmitterPath) :
						PrimaryAuthoringResourceLeaf(*pElement));
				if (bSelected)
				{
					ImGui::TextColored(ImVec4(0.36f, 0.72f, 1.f, 1.f),
						"%s", Label.c_str());
				}
				else
				{
					ImGui::TextWrapped("%s", Label.c_str());
				}
				const bool_t bLabelHovered = ImGui::IsItemHovered();
				ImGui::SameLine();
				ImGui::BeginDisabled(!bEditableSkillEffectActive);
				if (ImGui::SmallButton("Load Seed"))
					LoadSeed(Emitter);
				ImGui::EndDisabled();
				if (bLabelHovered)
				{
					ImGui::SetTooltip(
						"Track A source Element\nSource: %s\nSlots: %s\nLoad Seed -> Create Element -> Save Changes",
						Emitter.strSourceEmitterPath.c_str(),
						nullptr == pElement ? "deferred until Load Seed" :
							AuthoringElementResourceSlotSummary(*pElement).c_str());
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::TreePop();
}
