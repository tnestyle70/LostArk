#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

struct EFFECT_COMPONENT_EMITTER_DESC final
{
	std::string strEmitterId;
	std::string strElementId;
	uint32_t iSourceElementIndex = 0u;
	std::string strRendererType;
	bool_t bVisible = true;
	uint32_t iResourceBindingCount = 0u;
	uint32_t iModuleCount = 0u;
};

struct EFFECT_COMPONENT_DESC final
{
	std::string strComponentAssetId;
	std::string strDisplayName;
	std::string strComponentType;
	std::string strSourceEffectAssetId;
	std::string strSourceGroupId;
	std::vector<std::string> SourceNodes;
	std::string strSourceElementSha256;
	std::vector<EFFECT_COMPONENT_EMITTER_DESC> Emitters;
	EFFECT_DOCUMENT_DESC Document;
};

struct EFFECT_COMPONENT_CUE_DESC final
{
	std::string strCueId;
	std::string strComponentAssetId;
	f32_t fStartDelaySeconds = 0.f;
	bool_t bVisible = true;
	std::string strAnchorSlotId = "root";
	EFFECT_TRANSFORM_DESC LocalTransform;
};

struct EFFECT_ASSEMBLY_DESC final
{
	std::string strEffectAssetId;
	std::string strDisplayName;
	uint32_t iSourceAuthoringVersion = 0u;
	std::string strSourceDocumentSha256;
	std::string strSourceDocumentFileSha256;
	EFFECT_PARTICLE_SYSTEM_DESC ParticleSystem;
	std::vector<EFFECT_MODEL_CUE_DESC> ModelCues;
	std::vector<EFFECT_COMPONENT_CUE_DESC> ComponentCues;
};

NS_END
