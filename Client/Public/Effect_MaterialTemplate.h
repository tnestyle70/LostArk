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

inline const EFFECT_MATERIAL_TEMPLATE_DESC* Find_EffectMaterialTemplate(
	const std::string_view strTemplateId)
{
	return strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID ?
		&EFFECT_STANDARD_MATERIAL_TEMPLATE : nullptr;
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
