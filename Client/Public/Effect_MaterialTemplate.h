#pragma once

#include "Effect_AuthoringDocument.h"

#include <array>
#include <cstddef>
#include <string_view>

NS_BEGIN(Client)

enum class EFFECT_MATERIAL_INPUT_SEMANTIC : uint8_t
{
	BASE,
	NOISE,
	MASK,
	EMISSIVE,
	DISSOLVE,
	END
};

struct EFFECT_MATERIAL_INPUT_SLOT_DESC final
{
	std::string_view strSlotId;
	std::string_view strDisplayName;
	std::string_view strHlslBindingName;
	EFFECT_MATERIAL_INPUT_SEMANTIC eSemantic =
		EFFECT_MATERIAL_INPUT_SEMANTIC::END;
	EFFECT_RESOURCE_FILE_KIND eAllowedResourceKind =
		EFFECT_RESOURCE_FILE_KIND::END;
	EFFECT_RESOURCE_SLOT eRuntimeSlot = EFFECT_RESOURCE_SLOT::END;
};

struct EFFECT_MATERIAL_TEMPLATE_DESC final
{
	std::string_view strTemplateId;
	std::string_view strShaderProfileId;
	const EFFECT_MATERIAL_INPUT_SLOT_DESC* pInputs = nullptr;
	std::size_t iInputCount = 0u;
};

inline constexpr std::string_view EFFECT_MESH_SHAPE_SLOT_ID = "meshModel";
inline constexpr std::string_view EFFECT_STANDARD_MATERIAL_TEMPLATE_ID =
	"effect.standard";
inline constexpr std::string_view EFFECT_SOURCE_MATERIAL_TEMPLATE_ID =
	"effect.source_material";
inline constexpr f32_t EFFECT_MANUAL_MESH_DEFAULT_SCALE = 0.01f;

inline constexpr std::array<std::string_view, 15u>
	EFFECT_SOURCE_RUNTIME_SHADER_PROFILE_IDS = {{
		"effect.ue3.reconstructed-standard.v1",
		"effect.ue3.fallback-blocked.v1",
		"effect.ue3.circle.v1",
		"effect.ue3.dot.v1",
		"effect.ue3.ring.v1",
		"effect.ue3.aura.v1",
		"effect.ue3.one-layer-distortion.v1",
		"effect.ue3.grouped-translucent.v1",
		"effect.ue3.shine.v1",
		"effect.ue3.blackline-aura.v1",
		"effect.ue3.linearflow-02.v1",
		"effect.ue3.slice.v1",
		"effect.ue3.missiletrail-01.v1",
		"effect.ue3.local-crack.v1",
		"effect.ue3.procedural-center-glow.v1"
	}};

inline constexpr std::array<std::string_view, 19u>
	EFFECT_SOURCE_DYNAMIC_PARAMETER_SEMANTICS = {{
		"unbound",
		"opacity",
		"emissive",
		"dissolve",
		"uv_pan",
		"distortion",
		"radial_size",
		"mask_a_offset",
		"mask_b_offset",
		"mask_a_distort",
		"mask_b_distort",
		"mask_a_pan",
		"flow_strength",
		"mask_b_pan",
		"diffuse_pan",
		"missile_alpha_pan",
		"missile_noise_strength",
		"missile_noise_pan",
		"missile_dissolve"
	}};

inline constexpr std::array<std::string_view, 3u>
	EFFECT_SOURCE_SUBUV_MODES = {{
		"none",
		"psuvim_linear_blend",
		"psuvim_linear_blend_random_flip_square"
	}};

inline constexpr std::array<EFFECT_MATERIAL_INPUT_SLOT_DESC, 5u>
	EFFECT_STANDARD_MATERIAL_INPUTS = {{
		{ "base", "Base", "g_BaseTexture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::BASE,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::BASE_TEXTURE },
		{ "noise", "Noise", "g_NoiseTexture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::NOISE,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::NOISE_TEXTURE },
		{ "mask", "Mask", "g_MaskTexture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::MASK,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::MASK_TEXTURE },
		{ "emissive", "Emissive", "g_EmissiveTexture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::EMISSIVE,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE },
		{ "dissolve", "Dissolve", "g_DissolveTexture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::DISSOLVE,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE }
	}};

inline constexpr EFFECT_MATERIAL_TEMPLATE_DESC
	EFFECT_STANDARD_MATERIAL_TEMPLATE = {
		EFFECT_STANDARD_MATERIAL_TEMPLATE_ID,
		"effect.standard.hlsl.v1",
		EFFECT_STANDARD_MATERIAL_INPUTS.data(),
		EFFECT_STANDARD_MATERIAL_INPUTS.size()
	};

inline constexpr EFFECT_MATERIAL_TEMPLATE_DESC
	EFFECT_SOURCE_MATERIAL_TEMPLATE = {
		EFFECT_SOURCE_MATERIAL_TEMPLATE_ID,
		"effect.source-material.ue3-profile-runtime.v1",
		EFFECT_STANDARD_MATERIAL_INPUTS.data(),
		EFFECT_STANDARD_MATERIAL_INPUTS.size()
	};

inline const EFFECT_MATERIAL_TEMPLATE_DESC* Find_EffectMaterialTemplate(
	const std::string_view strTemplateId)
{
	if (strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID)
		return &EFFECT_STANDARD_MATERIAL_TEMPLATE;
	if (strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID)
		return &EFFECT_SOURCE_MATERIAL_TEMPLATE;
	return nullptr;
}

template <std::size_t Size>
inline bool_t Contains_EffectMaterialToken(
	const std::array<std::string_view, Size>& Tokens,
	const std::string_view strValue)
{
	for (const std::string_view strToken : Tokens)
	{
		if (strToken == strValue)
			return true;
	}
	return false;
}

inline bool_t Is_SupportedEffectSourceRuntimeShaderProfile(
	const std::string_view strProfileId)
{
	return Contains_EffectMaterialToken(
		EFFECT_SOURCE_RUNTIME_SHADER_PROFILE_IDS, strProfileId);
}

inline bool_t Is_SupportedEffectSourceDynamicParameterSemantic(
	const std::string_view strSemantic)
{
	return Contains_EffectMaterialToken(
		EFFECT_SOURCE_DYNAMIC_PARAMETER_SEMANTICS, strSemantic);
}

inline bool_t Is_SupportedEffectSourceSubUVMode(
	const std::string_view strMode)
{
	return Contains_EffectMaterialToken(EFFECT_SOURCE_SUBUV_MODES, strMode);
}

inline bool_t Is_EffectSourceMaterialStagingSignatureEqual(
	const EFFECT_SOURCE_MATERIAL_DESC& Left,
	const EFFECT_SOURCE_MATERIAL_DESC& Right)
{
	if (Left.bEnabled != Right.bEnabled ||
		Left.strProfileId != Right.strProfileId ||
		Left.strRuntimeShaderProfileId != Right.strRuntimeShaderProfileId ||
		Left.strParentMaterialPath != Right.strParentMaterialPath ||
		Left.eStatus != Right.eStatus ||
		Left.DynamicParameterSemantics != Right.DynamicParameterSemantics ||
		Left.strSubUVMode != Right.strSubUVMode ||
		Left.Textures.size() != Right.Textures.size() ||
		Left.Scalars.size() != Right.Scalars.size() ||
		Left.Vectors.size() != Right.Vectors.size() ||
		Left.StaticSwitches.size() != Right.StaticSwitches.size())
	{
		return false;
	}
	for (std::size_t i = 0u; i < Left.Textures.size(); ++i)
	{
		const EFFECT_NAMED_TEXTURE_DESC& A = Left.Textures[i];
		const EFFECT_NAMED_TEXTURE_DESC& B = Right.Textures[i];
		if (A.strName != B.strName || A.strGroup != B.strGroup ||
			A.strSourceObjectPath != B.strSourceObjectPath ||
			A.strAssetId != B.strAssetId ||
			A.eAddressU != B.eAddressU || A.eAddressV != B.eAddressV ||
			A.eColorSpace != B.eColorSpace ||
			A.strSamplingEvidence != B.strSamplingEvidence)
		{
			return false;
		}
	}
	for (std::size_t i = 0u; i < Left.Scalars.size(); ++i)
	{
		const EFFECT_NAMED_FLOAT_DESC& A = Left.Scalars[i];
		const EFFECT_NAMED_FLOAT_DESC& B = Right.Scalars[i];
		if (A.strName != B.strName || A.strGroup != B.strGroup ||
			A.fValue != B.fValue)
		{
			return false;
		}
	}
	for (std::size_t i = 0u; i < Left.Vectors.size(); ++i)
	{
		const EFFECT_NAMED_FLOAT4_DESC& A = Left.Vectors[i];
		const EFFECT_NAMED_FLOAT4_DESC& B = Right.Vectors[i];
		if (A.strName != B.strName || A.strGroup != B.strGroup ||
			A.vValue.x != B.vValue.x || A.vValue.y != B.vValue.y ||
			A.vValue.z != B.vValue.z || A.vValue.w != B.vValue.w)
		{
			return false;
		}
	}
	for (std::size_t i = 0u; i < Left.StaticSwitches.size(); ++i)
	{
		const EFFECT_NAMED_BOOL_DESC& A = Left.StaticSwitches[i];
		const EFFECT_NAMED_BOOL_DESC& B = Right.StaticSwitches[i];
		if (A.strName != B.strName || A.strGroup != B.strGroup ||
			A.bValue != B.bValue)
		{
			return false;
		}
	}
	return true;
}

inline bool_t Contains_EffectMaterialTokenNoCase(
	const std::string_view strValue,
	const std::string_view strToken)
{
	if (strToken.empty())
		return true;
	if (strToken.size() > strValue.size())
		return false;
	const auto LowerAscii = [](const char Character)
	{
		return Character >= 'A' && Character <= 'Z' ?
			static_cast<char>(Character + ('a' - 'A')) : Character;
	};
	for (std::size_t iOffset = 0u;
		iOffset + strToken.size() <= strValue.size(); ++iOffset)
	{
		bool_t bMatches = true;
		for (std::size_t iCharacter = 0u;
			iCharacter < strToken.size(); ++iCharacter)
		{
			if (LowerAscii(strValue[iOffset + iCharacter]) !=
				LowerAscii(strToken[iCharacter]))
			{
				bMatches = false;
				break;
			}
		}
		if (bMatches)
			return true;
	}
	return false;
}

inline bool_t Is_UnsafeEffectBaseTextureAssetId(
	const std::string_view strAssetId)
{
	return strAssetId.empty() ||
		Contains_EffectMaterialTokenNoCase(strAssetId, "blankwhite") ||
		Contains_EffectMaterialTokenNoCase(strAssetId, "normal") ||
		Contains_EffectMaterialTokenNoCase(strAssetId, "bump") ||
		Contains_EffectMaterialTokenNoCase(strAssetId, "_n.dds") ||
		Contains_EffectMaterialTokenNoCase(strAssetId, "_n_");
}

inline bool_t Is_EffectManualMeshAuthoringContractSatisfied(
	const bool_t bHasMeshModel,
	const bool_t bHasBaseTexture,
	const bool_t bUnsafeBaseTexture)
{
	return bHasMeshModel && bHasBaseTexture && !bUnsafeBaseTexture;
}

inline bool_t Is_EffectManualMeshCreateReady(
	const std::string_view strEffectName,
	const bool_t bHasMeshModel,
	const bool_t bHasBaseTexture,
	const bool_t bUnsafeBaseTexture)
{
	return !strEffectName.empty() &&
		Is_EffectManualMeshAuthoringContractSatisfied(
			bHasMeshModel, bHasBaseTexture, bUnsafeBaseTexture);
}

enum EFFECT_GROUPED_MATERIAL_FLAGS : uint32_t
{
	EFFECT_GROUPED_MATERIAL_HAS_ALPHA = 1u << 0u,
	EFFECT_GROUPED_MATERIAL_HAS_EMISSIVE = 1u << 1u,
	EFFECT_GROUPED_MATERIAL_HAS_NOISE = 1u << 2u,
	EFFECT_GROUPED_MATERIAL_HAS_DISTORTION = 1u << 3u,
	EFFECT_GROUPED_MATERIAL_HAS_DISSOLVE = 1u << 4u
};

struct EFFECT_GROUPED_TRANSLUCENT_CONSTANTS final
{
	// xy: UV scale, zw: UV panning per second.
	float4_t vUVScalePan = { 1.f, 1.f, 0.f, 0.f };
	// x: alpha strength, y: alpha power,
	// z: emissive strength, w: emissive power.
	float4_t vAlphaEmissive = { 1.f, 1.f, 1.f, 1.f };
	// x: noise UV strength, y: distortion strength,
	// z: dissolve threshold, w: dissolve hardness.
	float4_t vNoiseDissolve = { 0.f, 0.f, 0.f, 1.f };
	float4_t vTint = { 1.f, 1.f, 1.f, 1.f };
	uint32_t iFlags = 0u;
};

inline bool_t EffectMaterialParameterContains(
	const std::string_view strName,
	const std::string_view strGroup,
	const std::string_view strToken)
{
	return Contains_EffectMaterialTokenNoCase(strName, strToken) ||
		Contains_EffectMaterialTokenNoCase(strGroup, strToken);
}

inline bool_t EffectMaterialParameterIsStrength(
	const std::string_view strName)
{
	return Contains_EffectMaterialTokenNoCase(strName, ".str") ||
		Contains_EffectMaterialTokenNoCase(strName, "_str") ||
		Contains_EffectMaterialTokenNoCase(strName, "strength") ||
		Contains_EffectMaterialTokenNoCase(strName, "intensity") ||
		Contains_EffectMaterialTokenNoCase(strName, "density");
}

inline bool_t EffectMaterialParameterIsPower(
	const std::string_view strName)
{
	return Contains_EffectMaterialTokenNoCase(strName, "pow") ||
		Contains_EffectMaterialTokenNoCase(strName, "hardness");
}

inline bool_t EffectMaterialParameterIsAxisX(
	const std::string_view strName)
{
	return Contains_EffectMaterialTokenNoCase(strName, ".x") ||
		Contains_EffectMaterialTokenNoCase(strName, "_x") ||
		Contains_EffectMaterialTokenNoCase(strName, "scale_r") ||
		Contains_EffectMaterialTokenNoCase(strName, "uvscale_r");
}

inline bool_t EffectMaterialParameterIsAxisY(
	const std::string_view strName)
{
	return Contains_EffectMaterialTokenNoCase(strName, ".y") ||
		Contains_EffectMaterialTokenNoCase(strName, "_y") ||
		Contains_EffectMaterialTokenNoCase(strName, "scale_g") ||
		Contains_EffectMaterialTokenNoCase(strName, "uvscale_g");
}

inline int32_t EffectMaterialUVPriority(
	const std::string_view strName,
	const std::string_view strGroup)
{
	if (EffectMaterialParameterContains(strName, strGroup, "alpha") ||
		EffectMaterialParameterContains(strName, strGroup, "map_a") ||
		EffectMaterialParameterContains(strName, strGroup, "main") ||
		EffectMaterialParameterContains(strName, strGroup, "diff"))
	{
		return 3;
	}
	if (EffectMaterialParameterContains(strName, strGroup, "emiss"))
		return 2;
	return 1;
}

inline EFFECT_GROUPED_TRANSLUCENT_CONSTANTS
Build_EffectGroupedTranslucentConstants(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	EFFECT_GROUPED_TRANSLUCENT_CONSTANTS Result;
	int32_t iScaleXPriority = -1;
	int32_t iScaleYPriority = -1;
	int32_t iPanXPriority = -1;
	int32_t iPanYPriority = -1;
	int32_t iAlphaStrengthPriority = -1;
	int32_t iAlphaPowerPriority = -1;
	int32_t iEmissiveStrengthPriority = -1;
	int32_t iEmissivePowerPriority = -1;
	int32_t iTintPriority = -1;

	for (const EFFECT_NAMED_FLOAT_DESC& Scalar : Source.Scalars)
	{
		const std::string_view strName = Scalar.strName;
		const std::string_view strGroup = Scalar.strGroup;
		const bool_t bAlpha =
			EffectMaterialParameterContains(strName, strGroup, "alpha") ||
			EffectMaterialParameterContains(strName, strGroup, "mask") ||
			EffectMaterialParameterContains(strName, strGroup, "opacity") ||
			EffectMaterialParameterContains(strName, strGroup, "density");
		const bool_t bEmissive =
			EffectMaterialParameterContains(strName, strGroup, "emiss");
		const bool_t bNoise =
			EffectMaterialParameterContains(strName, strGroup, "noise") ||
			EffectMaterialParameterContains(strName, strGroup, "flow");
		const bool_t bDistortion =
			EffectMaterialParameterContains(strName, strGroup, "distort");
		const bool_t bDissolve =
			EffectMaterialParameterContains(strName, strGroup, "dissol");

		if (bAlpha)
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_ALPHA;
		if (bEmissive)
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_EMISSIVE;
		if (bNoise)
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_NOISE;
		if (bDistortion)
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_DISTORTION;
		if (bDissolve)
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_DISSOLVE;

		const bool_t bUVScale =
			EffectMaterialParameterContains(strName, strGroup, "uvscale") ||
			EffectMaterialParameterContains(strName, strGroup, "uv_scale") ||
			EffectMaterialParameterContains(strName, strGroup, "tile");
		const bool_t bUVPan =
			EffectMaterialParameterContains(strName, strGroup, "pan");
		if (bUVScale || bUVPan)
		{
			const int32_t iPriority =
				EffectMaterialUVPriority(strName, strGroup);
			const bool_t bAxisX = EffectMaterialParameterIsAxisX(strName);
			const bool_t bAxisY = EffectMaterialParameterIsAxisY(strName);
			if (bUVScale && (!bAxisY || bAxisX) &&
				iPriority > iScaleXPriority)
			{
				Result.vUVScalePan.x = Scalar.fValue;
				iScaleXPriority = iPriority;
			}
			if (bUVScale && (!bAxisX || bAxisY) &&
				iPriority > iScaleYPriority)
			{
				Result.vUVScalePan.y = Scalar.fValue;
				iScaleYPriority = iPriority;
			}
			if (bUVPan && (!bAxisY || bAxisX) &&
				iPriority > iPanXPriority)
			{
				Result.vUVScalePan.z = Scalar.fValue;
				iPanXPriority = iPriority;
			}
			if (bUVPan && (!bAxisX || bAxisY) &&
				iPriority > iPanYPriority)
			{
				Result.vUVScalePan.w = Scalar.fValue;
				iPanYPriority = iPriority;
			}
		}

		if (bAlpha)
		{
			const int32_t iPriority =
				EffectMaterialParameterContains(strName, strGroup, "opacity") ? 4 :
				(EffectMaterialParameterContains(strName, strGroup, "alpha") ? 3 :
				(EffectMaterialParameterContains(strName, strGroup, "mask") ? 2 : 1));
			if (EffectMaterialParameterIsStrength(strName) &&
				iPriority > iAlphaStrengthPriority)
			{
				Result.vAlphaEmissive.x = Scalar.fValue;
				iAlphaStrengthPriority = iPriority;
			}
			if (EffectMaterialParameterIsPower(strName) &&
				iPriority > iAlphaPowerPriority)
			{
				Result.vAlphaEmissive.y = Scalar.fValue;
				iAlphaPowerPriority = iPriority;
			}
		}
		if (bEmissive)
		{
			const int32_t iPriority =
				Contains_EffectMaterialTokenNoCase(strGroup, "emiss") ? 2 : 1;
			if (EffectMaterialParameterIsStrength(strName) &&
				iPriority > iEmissiveStrengthPriority)
			{
				Result.vAlphaEmissive.z = Scalar.fValue;
				iEmissiveStrengthPriority = iPriority;
			}
			if (EffectMaterialParameterIsPower(strName) &&
				iPriority > iEmissivePowerPriority)
			{
				Result.vAlphaEmissive.w = Scalar.fValue;
				iEmissivePowerPriority = iPriority;
			}
		}
		if ((bNoise || bDistortion) &&
			EffectMaterialParameterIsStrength(strName))
		{
			if (bNoise)
				Result.vNoiseDissolve.x = Scalar.fValue;
			if (bDistortion)
				Result.vNoiseDissolve.y = Scalar.fValue;
		}
		if (bDissolve)
		{
			if (EffectMaterialParameterIsPower(strName))
				Result.vNoiseDissolve.w = Scalar.fValue;
			else if (EffectMaterialParameterIsStrength(strName) ||
				EffectMaterialParameterContains(strName, strGroup, "threshold") ||
				EffectMaterialParameterContains(strName, strGroup, "amount"))
			{
				Result.vNoiseDissolve.z = Scalar.fValue;
			}
		}
	}

	for (const EFFECT_NAMED_FLOAT4_DESC& Vector : Source.Vectors)
	{
		const bool_t bEmissive =
			EffectMaterialParameterContains(
				Vector.strName, Vector.strGroup, "emiss");
		const bool_t bColor = bEmissive ||
			EffectMaterialParameterContains(
				Vector.strName, Vector.strGroup, "color");
		if (!bColor)
			continue;
		const int32_t iPriority = bEmissive ? 2 : 1;
		if (iPriority <= iTintPriority)
			continue;
		Result.vTint = Vector.vValue;
		iTintPriority = iPriority;
		if (bEmissive)
		{
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_EMISSIVE;
			if (Vector.vValue.w > 1.f && iEmissiveStrengthPriority < 0)
				Result.vAlphaEmissive.z = Vector.vValue.w;
		}
	}
	return Result;
}

inline bool_t Is_EffectGroupedTranslucentResourceContractSatisfied(
	const EFFECT_GROUPED_TRANSLUCENT_CONSTANTS& Constants,
	const bool_t bSafeBase,
	const bool_t bHasMask,
	const bool_t bHasEmissive,
	const bool_t bHasDissolve)
{
	if (0u != (Constants.iFlags & EFFECT_GROUPED_MATERIAL_HAS_ALPHA) &&
		!bSafeBase && !bHasMask && !bHasDissolve)
	{
		return false;
	}
	if (0u != (Constants.iFlags & EFFECT_GROUPED_MATERIAL_HAS_EMISSIVE) &&
		!bSafeBase && !bHasEmissive)
	{
		return false;
	}
	return bSafeBase || bHasMask || bHasEmissive;
}

inline bool_t Is_EffectGroupedTranslucentResourceContractSatisfied(
	const EFFECT_SOURCE_MATERIAL_DESC& Source,
	const bool_t bSafeBase,
	const bool_t bHasMask,
	const bool_t bHasEmissive,
	const bool_t bHasDissolve)
{
	return Is_EffectGroupedTranslucentResourceContractSatisfied(
		Build_EffectGroupedTranslucentConstants(Source), bSafeBase,
		bHasMask, bHasEmissive, bHasDissolve);
}

inline bool_t Is_EffectFiniteProfileResourceContractSatisfied(
	const std::string_view strRuntimeShaderProfileId,
	const bool_t bSafeBase,
	const bool_t bHasMask,
	const bool_t bHasDissolve,
	const bool_t bHasMesh)
{
	if (strRuntimeShaderProfileId == "effect.ue3.shine.v1")
		return bSafeBase && bHasMask;
	if (strRuntimeShaderProfileId == "effect.ue3.blackline-aura.v1")
		return bHasMask && bHasDissolve;
	if (strRuntimeShaderProfileId == "effect.ue3.local-crack.v1")
		return false;
	if (strRuntimeShaderProfileId == "effect.ue3.slice.v1")
		return bSafeBase;
	if (strRuntimeShaderProfileId == "effect.ue3.missiletrail-01.v1")
		return bSafeBase && bHasMask && bHasDissolve && bHasMesh;
	if (strRuntimeShaderProfileId ==
		"effect.ue3.procedural-center-glow.v1")
	{
		return true;
	}
	return true;
}

inline bool_t Is_EffectLocalCrackResourceContractSatisfied(
	const bool_t bHasNormal,
	const bool_t bHasReflection,
	const bool_t bHasDissolve,
	const bool_t bHasMesh)
{
	return bHasNormal && bHasReflection && bHasDissolve && bHasMesh;
}

inline bool_t Has_EffectLocalCrackNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	static constexpr std::array<std::string_view, 3u> RequiredNames = {{
		"normal_tex", "refle_tex", "dissolve_tex"
	}};
	for (const std::string_view strRequiredName : RequiredNames)
	{
		bool_t bFound = false;
		for (const EFFECT_NAMED_TEXTURE_DESC& Texture : Source.Textures)
		{
			if (Texture.strName == strRequiredName &&
				!Texture.strAssetId.empty() &&
				!Texture.strSamplingEvidence.empty() &&
				Texture.strSamplingEvidence != "legacy_default")
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
			return false;
	}
	return true;
}

inline bool_t Is_EffectLegacyLocalCrackResourceContractSatisfied(
	const EFFECT_SOURCE_MATERIAL_DESC& Source,
	const bool_t bHasDissolve,
	const bool_t bHasMesh)
{
	return Source.Textures.empty() && bHasDissolve && bHasMesh;
}

inline const EFFECT_MATERIAL_INPUT_SLOT_DESC* Find_EffectMaterialInput(
	const EFFECT_MATERIAL_TEMPLATE_DESC& Template,
	const std::string_view strSlotId)
{
	for (std::size_t iInput = 0u; iInput < Template.iInputCount; ++iInput)
	{
		if (Template.pInputs[iInput].strSlotId == strSlotId)
			return &Template.pInputs[iInput];
	}
	return nullptr;
}

inline const EFFECT_MATERIAL_INPUT_SLOT_DESC* Find_EffectMaterialInput(
	const std::string_view strTemplateId,
	const std::string_view strSlotId)
{
	const EFFECT_MATERIAL_TEMPLATE_DESC* pTemplate =
		Find_EffectMaterialTemplate(strTemplateId);
	return nullptr == pTemplate ? nullptr :
		Find_EffectMaterialInput(*pTemplate, strSlotId);
}

inline const EFFECT_MATERIAL_INPUT_SLOT_DESC* Find_EffectMaterialInput(
	const EFFECT_MATERIAL_TEMPLATE_DESC& Template,
	const EFFECT_RESOURCE_SLOT eRuntimeSlot)
{
	for (std::size_t iInput = 0u; iInput < Template.iInputCount; ++iInput)
	{
		if (Template.pInputs[iInput].eRuntimeSlot == eRuntimeSlot)
			return &Template.pInputs[iInput];
	}
	return nullptr;
}

inline const EFFECT_MATERIAL_INPUT_SLOT_DESC* Find_EffectMaterialInput(
	const EFFECT_MATERIAL_TEMPLATE_DESC& Template,
	const EFFECT_MATERIAL_INPUT_SEMANTIC eSemantic)
{
	for (std::size_t iInput = 0u; iInput < Template.iInputCount; ++iInput)
	{
		if (Template.pInputs[iInput].eSemantic == eSemantic)
			return &Template.pInputs[iInput];
	}
	return nullptr;
}

NS_END
