#include "Effect_DocumentCodec_Internal.h"
#include "Effect_MaterialTemplate.h"

#include <algorithm>
#include <atomic>
#include <array>
#include <cctype>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <set>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>


using namespace Client::EffectDocumentCodecDetail;

namespace Client::EffectDocumentCodecDetail
{


	bool_t Validate_MaterialExecution(
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution,
		std::string& strOutError)
	{
		const auto AllZero = [](const auto& Values)
		{
			return std::all_of(Values.begin(), Values.end(),
				[](const uint32_t Value) { return 0u == Value; });
		};
		if (!Execution.bEnabled)
		{
			if (Execution.bAuthoringApproximate && !Execution.bFailClosed)
			{
				strOutError =
					"Authoring-approximate authored Material must stay fail-closed.";
				return false;
			}
			if (Execution.eFidelity !=
					Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::SOURCE_EXACT ||
				1u != Execution.iVersion ||
				Client::EFFECT_MATERIAL_EXECUTION_BACKEND::GENERIC !=
					Execution.eBackend ||
				0u != Execution.iOpcode || 0u != Execution.iPassIndex ||
				!Execution.strRasterizerState.empty() ||
				!Execution.strDepthStencilState.empty() ||
				!Execution.strBlendState.empty() ||
				0u != Execution.iStencilReference ||
				0u != Execution.iTextureLaneCount ||
				0u != Execution.iTextureMask || !Execution.TextureLanes.empty() ||
				!Is_DefaultStandardColorV1(Execution.StandardColorV1) ||
				0u != Execution.iDynamicConsumedMask ||
				0u != Execution.iDynamicSuppressedMask ||
				0u != Execution.iParticleColorPolicy ||
				0u != Execution.iParticleColorConsumedMask ||
				0u != Execution.iParticleColorSuppressedMask ||
				0u != Execution.iScalarCount || 0u != Execution.iVectorCount ||
				0u != Execution.iInputCount ||
				!AllZero(Execution.InputConsumedMask) ||
				!AllZero(Execution.InputSuppressedMask) ||
				!AllZero(Execution.VectorComponentConsumedMask) ||
				!AllZero(Execution.VectorComponentSuppressedMask) ||
				0u != Execution.iStaticInputCount ||
				0u != Execution.iStaticSelectedMask ||
				0u != Execution.iStaticConsumedMask ||
				0u != Execution.iStaticSuppressedMask ||
				0u != Execution.iRenderInputCount ||
				0u != Execution.iRenderConsumedMask ||
				0u != Execution.iRenderSuppressedMask ||
				!Execution.Scalars.empty() || !Execution.Vectors.empty() ||
				!Execution.ArtistParameters.empty() || !Execution.Colors.empty())
			{
				strOutError =
					"Disabled authored Material execution carries hidden state.";
				return false;
			}
			return true;
		}
		if (Execution.bFailClosed)
		{
			strOutError =
				"Enabled authored Material execution cannot also be fail-closed.";
			return false;
		}
		if (Execution.bAuthoringApproximate)
		{
			strOutError =
				"Enabled authored Material execution cannot be authoring-approximate.";
			return false;
		}
		const bool_t bProjectTunedApprox = Execution.eFidelity ==
			Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::PROJECT_TUNED_APPROX;
		const bool_t bProjectTunedOpcode = Execution.eBackend ==
			Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			(Execution.iOpcode == 1001u || Execution.iOpcode == 1002u ||
			 Execution.iOpcode == 1003u || Execution.iOpcode == 1004u);
		if (bProjectTunedApprox != bProjectTunedOpcode)
		{
			strOutError =
				"Enabled material execution fidelity/opcode contract changed.";
			return false;
		}

		if (1u != Execution.iVersion ||
			Execution.eBackend <=
				Client::EFFECT_MATERIAL_EXECUTION_BACKEND::GENERIC ||
			Execution.eBackend >=
				Client::EFFECT_MATERIAL_EXECUTION_BACKEND::END ||
			Execution.iOpcode > 65535u || Execution.iPassIndex > 63u ||
			!Is_StableId(Execution.strRasterizerState) ||
			!Is_StableId(Execution.strDepthStencilState) ||
			!Is_StableId(Execution.strBlendState) ||
			Execution.iStencilReference > 255u ||
			Execution.iTextureLaneCount > MAX_AUTHORED_MATERIAL_TEXTURE_LANES ||
			Execution.iTextureLaneCount != Execution.TextureLanes.size())
		{
			strOutError =
				"Authored Material execution identity, pass, or lane count is invalid.";
			return false;
		}
		const uint32_t iExpectedTextureMask =
			0u == Execution.iTextureLaneCount ? 0u :
				((1u << Execution.iTextureLaneCount) - 1u);
		if (Execution.iTextureMask != iExpectedTextureMask ||
			(Client::EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL ==
				Execution.eBackend && 6u != Execution.iTextureLaneCount))
		{
			strOutError =
				"Authored Material texture mask is not the bounded contiguous lane contract.";
			return false;
		}

		std::unordered_set<std::string> LaneIds;
		std::unordered_set<uint32_t> TextureRegisters;
		for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
		{
			const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane =
				Execution.TextureLanes[iLane];
			Client::EFFECT_RESOURCE_FILE_KIND eActualKind =
				Client::EFFECT_RESOURCE_FILE_KIND::END;
			const bool_t bChannelValid = Lane.strSourceChannel.empty() ||
				(Lane.strSourceChannel.size() <= 4u &&
					std::all_of(Lane.strSourceChannel.begin(),
						Lane.strSourceChannel.end(), [](const char_t Character)
						{
							return std::string_view("RGBA").find(Character) !=
								std::string_view::npos;
						}));
			const bool_t bChannelRequired =
				Execution.eBackend ==
					Client::EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL ||
				Execution.eBackend ==
					Client::EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1;
			const Client::EFFECT_MATERIAL_SAMPLER_DESC& Sampler = Lane.Sampler;
			const bool_t bLaneIdStable = Is_StableId(Lane.strLaneId);
			const bool_t bRoleStable = Is_StableId(Lane.strRole);
			const bool_t bLaneIdUnique = LaneIds.insert(Lane.strLaneId).second;
			const bool_t bSafeAsset =
				Client::CEffectDocumentCodec::Is_SafeResourceAssetId(
					Lane.strAssetId, &eActualKind);
			const bool_t bTextureRegisterUnique =
				TextureRegisters.insert(Lane.iTextureRegister).second;
			if (!bLaneIdStable || !bRoleStable || !bLaneIdUnique || !bSafeAsset ||
				eActualKind != Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE ||
				Lane.iTextureRegister != iLane ||
				!bTextureRegisterUnique ||
				Lane.iSamplerRegister != 5u + iLane || !bChannelValid ||
				(bChannelRequired && Lane.strSourceChannel.empty()) ||
				Lane.eColorSpace >= Client::EFFECT_TEXTURE_COLOR_SPACE::END ||
				Sampler.eFilter >= Client::EFFECT_MATERIAL_TEXTURE_FILTER::END ||
				Sampler.eAddressU >=
					Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::END ||
				Sampler.eAddressV >=
					Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::END ||
				Sampler.eAddressW >=
					Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::END ||
				Sampler.eComparison >=
					Client::EFFECT_MATERIAL_COMPARISON_FUNCTION::END ||
				!std::isfinite(Sampler.fMipLodBias) ||
				std::abs(Sampler.fMipLodBias) > 16.f ||
				Sampler.iMaxAnisotropy < 1u || Sampler.iMaxAnisotropy > 16u ||
				!Is_Finite(Sampler.vBorderColor) ||
				!std::isfinite(Sampler.fMinLod) ||
				!std::isfinite(Sampler.fMaxLod) ||
				Sampler.fMinLod > Sampler.fMaxLod)
			{
				std::ostringstream Detail;
				Detail << "Authored Material texture lane or sampler is invalid: "
					<< Lane.strLaneId << " (role=" << Lane.strRole
					<< ", asset=" << Lane.strAssetId
					<< ", stable=" << bLaneIdStable << "/" << bRoleStable
					<< ", unique=" << bLaneIdUnique << "/"
					<< bTextureRegisterUnique << ", safe=" << bSafeAsset
					<< ", register=" << Lane.iTextureRegister << "/"
					<< Lane.iSamplerRegister << ", channel="
					<< Lane.strSourceChannel << ", colorSpace="
					<< static_cast<uint32_t>(Lane.eColorSpace)
					<< ", filter=" << static_cast<uint32_t>(Sampler.eFilter)
					<< ", address=" << static_cast<uint32_t>(Sampler.eAddressU)
					<< "/" << static_cast<uint32_t>(Sampler.eAddressV)
					<< "/" << static_cast<uint32_t>(Sampler.eAddressW)
					<< ", comparison="
					<< static_cast<uint32_t>(Sampler.eComparison)
					<< ", mipBias=" << Sampler.fMipLodBias
					<< ", anisotropy=" << Sampler.iMaxAnisotropy
					<< ", lod=" << Sampler.fMinLod << "/"
					<< Sampler.fMaxLod << ").";
				strOutError = Detail.str();
				return false;
			}
		}

		const auto MaskWithinCount = [](const uint32_t Mask,
			const uint32_t Count)
		{
			if (Count >= 32u)
				return true;
			const uint32_t Allowed = 0u == Count ? 0u : (1u << Count) - 1u;
			return 0u == (Mask & ~Allowed);
		};
		const auto MasksDisjointWithinCount = [&MaskWithinCount](
			const uint32_t Consumed, const uint32_t Suppressed,
			const uint32_t Count)
		{
			return 0u == (Consumed & Suppressed) &&
				MaskWithinCount(Consumed, Count) &&
				MaskWithinCount(Suppressed, Count);
		};
		if (Execution.iDynamicConsumedMask > 0x0fu ||
			Execution.iDynamicSuppressedMask > 0x0fu ||
			0u != (Execution.iDynamicConsumedMask &
				Execution.iDynamicSuppressedMask) ||
			Execution.iParticleColorPolicy > 3u ||
			Execution.iParticleColorConsumedMask > 0x0fu ||
			Execution.iParticleColorSuppressedMask > 0x0fu ||
			0u != (Execution.iParticleColorConsumedMask &
				Execution.iParticleColorSuppressedMask) ||
			Execution.iScalarCount > MAX_AUTHORED_MATERIAL_SCALARS ||
			Execution.iVectorCount > 3u || Execution.iInputCount > 64u ||
			Execution.iStaticInputCount > 32u ||
			Execution.iRenderInputCount > 32u ||
			Execution.Scalars.size() != Execution.iScalarCount ||
			Execution.Vectors.size() != Execution.iVectorCount ||
			!MasksDisjointWithinCount(Execution.InputConsumedMask[0u],
				Execution.InputSuppressedMask[0u],
				std::min(Execution.iInputCount, 32u)) ||
			!MasksDisjointWithinCount(Execution.InputConsumedMask[1u],
				Execution.InputSuppressedMask[1u],
				Execution.iInputCount > 32u ? Execution.iInputCount - 32u : 0u) ||
			!MasksDisjointWithinCount(Execution.iStaticConsumedMask,
				Execution.iStaticSuppressedMask, Execution.iStaticInputCount) ||
			!MaskWithinCount(Execution.iStaticSelectedMask,
				Execution.iStaticInputCount) ||
			0u != (Execution.iStaticSelectedMask &
				~(Execution.iStaticConsumedMask |
				  Execution.iStaticSuppressedMask)) ||
			!MasksDisjointWithinCount(Execution.iRenderConsumedMask,
				Execution.iRenderSuppressedMask, Execution.iRenderInputCount))
		{
			std::ostringstream Detail;
			Detail << "Authored Material packed counts or masks are invalid"
				<< " (dynamic=" << Execution.iDynamicConsumedMask << "/"
				<< Execution.iDynamicSuppressedMask << ", particleColor="
				<< Execution.iParticleColorPolicy << "/"
				<< Execution.iParticleColorConsumedMask << "/"
				<< Execution.iParticleColorSuppressedMask << ", scalar="
				<< Execution.iScalarCount << "/" << Execution.Scalars.size()
				<< ", vector=" << Execution.iVectorCount << "/"
				<< Execution.Vectors.size() << ", input="
				<< Execution.iInputCount << "/" << std::hex
				<< Execution.InputConsumedMask[0u] << "/"
				<< Execution.InputSuppressedMask[0u] << "/"
				<< Execution.InputConsumedMask[1u] << "/"
				<< Execution.InputSuppressedMask[1u] << std::dec
				<< ", static=" << Execution.iStaticInputCount << "/"
				<< Execution.iStaticSelectedMask << "/"
				<< Execution.iStaticConsumedMask << "/"
				<< Execution.iStaticSuppressedMask << ", render="
				<< Execution.iRenderInputCount << "/"
				<< Execution.iRenderConsumedMask << "/"
				<< Execution.iRenderSuppressedMask << ").";
			strOutError = Detail.str();
			return false;
		}
		for (size_t iVector = 0u;
			iVector < Execution.VectorComponentConsumedMask.size(); ++iVector)
		{
			const uint32_t iComponentCount =
				iVector < Execution.iVectorCount ? 4u : 0u;
			if (!MasksDisjointWithinCount(
					Execution.VectorComponentConsumedMask[iVector],
					Execution.VectorComponentSuppressedMask[iVector],
					iComponentCount))
			{
				strOutError =
					"Authored Material vector component masks are invalid.";
				return false;
			}
		}

		const auto ValidateParameterNames = [](const auto& Parameters,
			const uint32_t iPackedCount)
		{
			std::unordered_set<std::string> Names;
			std::unordered_set<uint32_t> Indices;
			for (const auto& Parameter : Parameters)
			{
				if (!Is_StableId(Parameter.strName) ||
					Parameter.iPackedIndex >= iPackedCount ||
					!Names.insert(Parameter.strName).second ||
					!Indices.insert(Parameter.iPackedIndex).second)
				{
					return false;
				}
			}
			return true;
		};
		if (!ValidateParameterNames(Execution.Scalars, Execution.iScalarCount) ||
			!ValidateParameterNames(Execution.Vectors, Execution.iVectorCount) ||
			!std::all_of(Execution.Scalars.begin(), Execution.Scalars.end(),
				[](const Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Parameter)
				{
					return std::isfinite(Parameter.fValue);
				}) ||
			!std::all_of(Execution.Vectors.begin(), Execution.Vectors.end(),
				[](const Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Parameter)
				{
					return Is_Finite(Parameter.vValue);
				}))
		{
			strOutError = "Authored Material packed scalar or vector is invalid.";
			return false;
		}

		const bool_t bArtist =
			Client::EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4 ==
				Execution.eBackend;
		if ((!bArtist && (!Execution.ArtistParameters.empty() ||
				!Execution.Colors.empty())) ||
			Execution.ArtistParameters.size() > MAX_AUTHORED_MATERIAL_VECTORS ||
			Execution.Colors.size() > MAX_AUTHORED_MATERIAL_COLORS ||
			!ValidateParameterNames(Execution.ArtistParameters,
				static_cast<uint32_t>(MAX_AUTHORED_MATERIAL_VECTORS)) ||
			!ValidateParameterNames(Execution.Colors,
				static_cast<uint32_t>(MAX_AUTHORED_MATERIAL_COLORS)) ||
			!std::all_of(Execution.ArtistParameters.begin(),
				Execution.ArtistParameters.end(),
				[](const Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Parameter)
				{
					return Is_Finite(Parameter.vValue);
				}) ||
			!std::all_of(Execution.Colors.begin(), Execution.Colors.end(),
				[](const Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Parameter)
				{
					return Is_Finite(Parameter.vValue);
				}))
		{
			strOutError = "Authored Artist Visual parameter or color is invalid.";
			return false;
		}
		return Validate_StandardColorV1Execution(Execution, strOutError);
	}


	bool_t Read_SourceAdmission(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_ADMISSION_DESC& Out,
		std::string& strOutError);


	bool_t Read_Distribution(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DISTRIBUTION_DESC& Out,
		const bool_t bSourceContract,
		std::string& strOutError)
	{
		if (!bSourceContract)
		{
			constexpr const char_t* SourceOnlyFields[] = {
				"referenceId", "occurrenceId", "payloadStatus", "fidelity",
				"executionAdmission", "parameterBinding", "parameterName"
			};
			for (const char_t* pField : SourceOnlyFields)
			{
				if (nullptr != Value.Find(pField))
				{
					strOutError =
						"Legacy Effect distribution contains native-v14 evidence.";
					return false;
				}
			}
		}
		if (bSourceContract && !Validate_ExactFields(Value,
			{ "propertyPath", "referenceId", "occurrenceId", "payloadStatus",
				"fidelity", "executionAdmission", "sourceClass", "sourceObjectPath",
				"parameterBinding", "parameterName", "componentCount",
				"operation", "randomLockAxes", "lookupTableChunkSize",
				"lookupTableNumElements", "lookupTableTimeScale",
				"lookupTableStartTime", "defaultMinimum", "defaultMaximum",
				"lookupTable", "keys" },
			"Effect source distribution", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pExecutionAdmission = bSourceContract ?
			Find_Field(Value, "executionAdmission",
				Client::DATA_JSON_TYPE::OBJECT, strOutError) : nullptr;
		if (bSourceContract &&
			(nullptr == pExecutionAdmission ||
			 !Read_String(Value, "referenceId", Out.strReferenceId,
				strOutError) ||
			 !Read_String(Value, "occurrenceId", Out.strOccurrenceId,
				strOutError) ||
			 !Read_String(Value, "payloadStatus", Out.strPayloadStatus,
				strOutError) ||
			 !Read_String(Value, "fidelity", Out.strFidelity, strOutError) ||
			 !Read_SourceAdmission(*pExecutionAdmission,
				Out.ExecutionAdmission, strOutError)))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pLookupTable = Find_Field(
			Value, "lookupTable", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pKeys = Find_Field(
			Value, "keys", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		if (nullptr == pLookupTable || nullptr == pKeys ||
			!Read_String(Value, "propertyPath", Out.strPropertyPath,
				strOutError) ||
			!Read_String(Value, "sourceClass", Out.strSourceClass,
				strOutError) ||
			!Read_String(Value, "sourceObjectPath", Out.strSourceObjectPath,
				strOutError) ||
			!Read_UInt(Value, "componentCount", Out.iComponentCount,
				strOutError) ||
			!Read_UInt(Value, "operation", Out.iOperation, strOutError) ||
			!Read_UInt(Value, "lookupTableChunkSize",
				Out.iLookupTableChunkSize, strOutError) ||
			!Read_UInt(Value, "lookupTableNumElements",
				Out.iLookupTableNumElements, strOutError) ||
			!Read_Float(Value, "lookupTableTimeScale",
				Out.fLookupTableTimeScale, strOutError) ||
			!Read_Float(Value, "lookupTableStartTime",
				Out.fLookupTableStartTime, strOutError) ||
			!Read_Array(Value, "defaultMinimum", &Out.vDefaultMinimum.x,
				4u, strOutError) ||
			!Read_Array(Value, "defaultMaximum", &Out.vDefaultMaximum.x,
				4u, strOutError))
		{
			return false;
		}
		if (bSourceContract)
		{
			const Client::DATA_JSON_VALUE* pBinding = Value.Find(
				"parameterBinding");
			const Client::DATA_JSON_VALUE* pName = Value.Find("parameterName");
			const bool_t bParticleParameter =
				Is_ParticleParameterDistribution(Out.strSourceClass);
			if (bParticleParameter)
			{
				if (nullptr == pBinding || !pBinding->Is_String() ||
					nullptr == pName || !pName->Is_String() ||
					!Parse_Token(pBinding->Get_String(),
						DISTRIBUTION_PARAMETER_BINDING_TOKENS,
						std::size(DISTRIBUTION_PARAMETER_BINDING_TOKENS),
						Out.eParameterBinding))
				{
					strOutError =
						"Effect ParticleParameter binding is missing or invalid.";
					return false;
				}
				Out.strParameterName = pName->Get_String();
				if ((Client::EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ==
						Out.eParameterBinding && !Out.strParameterName.empty()) ||
					(Client::EFFECT_DISTRIBUTION_PARAMETER_BINDING::ACTION_CUE ==
						Out.eParameterBinding && Out.strParameterName.empty()))
				{
					strOutError =
						"Effect ParticleParameter name and binding disagree.";
					return false;
				}
			}
			else if (nullptr != pBinding || nullptr != pName)
			{
				strOutError =
					"Non-ParticleParameter distribution carries source binding fields.";
				return false;
			}
		}
		const Client::DATA_JSON_VALUE* pRandomLockAxes =
			Value.Find("randomLockAxes");
		if (bSourceContract && nullptr == pRandomLockAxes)
		{
			strOutError = "Missing or invalid field: randomLockAxes";
			return false;
		}
		if (nullptr != pRandomLockAxes)
		{
			if (!pRandomLockAxes->Is_Number() ||
				!std::isfinite(pRandomLockAxes->Get_Number()) ||
				pRandomLockAxes->Get_Number() !=
					std::floor(pRandomLockAxes->Get_Number()) ||
				pRandomLockAxes->Get_Number() < 0.0 ||
				pRandomLockAxes->Get_Number() > 4.0)
			{
				strOutError = "Effect distribution randomLockAxes is invalid.";
				return false;
			}
			Out.iRandomLockAxes = static_cast<uint32_t>(
				pRandomLockAxes->Get_Number());
		}
		Out.LookupTable.reserve(pLookupTable->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& Item : pLookupTable->Get_Array())
		{
			if (!Item.Is_Number() || !std::isfinite(Item.Get_Number()))
			{
				strOutError = "Effect distribution lookup table is invalid.";
				return false;
			}
			Out.LookupTable.push_back(static_cast<f32_t>(Item.Get_Number()));
		}
		Out.Keys.reserve(pKeys->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& KeyValue : pKeys->Get_Array())
		{
			if (bSourceContract && !Validate_ExactFields(KeyValue,
				{ "time", "minimum", "maximum", "arriveTangentMinimum",
					"leaveTangentMinimum", "arriveTangentMaximum",
					"leaveTangentMaximum", "interpolation" },
				"Effect source distribution key", strOutError))
			{
				return false;
			}
			const Client::DATA_JSON_VALUE* pInterpolation =
				KeyValue.Is_Object() ? KeyValue.Find("interpolation") : nullptr;
			Client::EFFECT_DISTRIBUTION_KEY_DESC Key;
			if (nullptr == pInterpolation || !pInterpolation->Is_String() ||
				!Parse_Token(pInterpolation->Get_String(),
					DISTRIBUTION_INTERPOLATION_TOKENS,
					std::size(DISTRIBUTION_INTERPOLATION_TOKENS),
					Key.eInterpolation) ||
				!Read_Float(KeyValue, "time", Key.fTime, strOutError) ||
				!Read_Array(KeyValue, "minimum", &Key.vMinimum.x, 4u,
					strOutError) ||
				!Read_Array(KeyValue, "maximum", &Key.vMaximum.x, 4u,
					strOutError) ||
				!Read_Array(KeyValue, "arriveTangentMinimum",
					&Key.vArriveTangentMinimum.x, 4u, strOutError) ||
				!Read_Array(KeyValue, "leaveTangentMinimum",
					&Key.vLeaveTangentMinimum.x, 4u, strOutError) ||
				!Read_Array(KeyValue, "arriveTangentMaximum",
					&Key.vArriveTangentMaximum.x, 4u, strOutError) ||
				!Read_Array(KeyValue, "leaveTangentMaximum",
					&Key.vLeaveTangentMaximum.x, 4u, strOutError))
			{
				return false;
			}
			Out.Keys.push_back(std::move(Key));
		}
		return true;
	}

	bool_t Validate_ElementMaterial(const Client::EFFECT_ELEMENT_DESC& Element,
		const bool_t bSourceContract, std::string& strOutError)
	{
		using namespace Client;
		const EFFECT_MATERIAL_TEMPLATE_DESC* pMaterialTemplate =
			Find_EffectMaterialTemplate(Element.Material.strTemplateId);
		if (nullptr == pMaterialTemplate)
		{
			strOutError = "Effect Material Template is not registered: " +
				Element.Material.strTemplateId;
			return false;
		}
		if (Element.Material.bColorTexturesSRGB &&
			(Element.Material.strTemplateId != EFFECT_STANDARD_MATERIAL_TEMPLATE_ID ||
			 Element.Material.SourceMaterial.bEnabled ||
			 Element.Material.Execution.bEnabled ||
			 Element.Material.Execution.bFailClosed || Element.SourceRecipe.bEnabled))
		{
			strOutError =
				"colorTexturesSRGB requires an ordinary authored standard material: " +
				Element.strElementId;
			return false;
		}
		if (Element.Material.strSourceMaterialPath.size() > 512u ||
			(Element.Material.strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID &&
				(Element.Material.strSourceMaterialPath.empty() ||
					!Has_VisibleCharacter(
						Element.Material.strSourceMaterialPath))))
		{
			strOutError = "Effect source Material identity is invalid.";
			return false;
		}
		if (!Validate_MaterialExecution(Element.Material.Execution,
			strOutError))
		{
			strOutError += " Element: " + Element.strElementId + ".";
			return false;
		}
		const bool_t bStandardColorBackend =
			Element.Material.Execution.bEnabled &&
			Element.Material.Execution.eBackend ==
				EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1;
		const bool_t bStandardColorTemplate =
			Element.Material.strTemplateId == EFFECT_STANDARD_COLOR_V1_TEMPLATE_ID;
		const bool_t bStandardColorMeshCarrier =
			Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.strRendererShape == "mesh" &&
			Element.ResourceBindings.size() == 1u &&
			Element.ResourceBindings[0u].strSlotId == "meshModel" &&
			!Element.ResourceBindings[0u].strAssetId.empty();
		const bool_t bStandardColorResourceContract =
			bStandardColorMeshCarrier ||
			(Element.ResourceBindings.empty() &&
			 ((Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			   Element.SourceRecipe.bEnabled &&
			   Element.SourceRecipe.strRendererShape == "sprite") ||
			  Element.eKind == EFFECT_ELEMENT_KIND::DECAL ||
			  Element.eKind == EFFECT_ELEMENT_KIND::TRAIL));
		if (bStandardColorBackend != bStandardColorTemplate ||
			(bStandardColorBackend &&
			 (Element.eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
			  Element.eKind != EFFECT_ELEMENT_KIND::DECAL &&
			  Element.eKind != EFFECT_ELEMENT_KIND::TRAIL)) ||
			(bStandardColorBackend &&
			 (Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
			  Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END)) ||
			(bStandardColorBackend && !bStandardColorResourceContract) ||
			(bStandardColorBackend && Element.Material.SourceMaterial.bEnabled) ||
			(bStandardColorBackend &&
			 Element.Material.eRenderProfile ==
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE) ||
			(bStandardColorBackend &&
			 Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			 (!Element.SourceRecipe.bEnabled ||
			  (Element.SourceRecipe.strRendererShape != "sprite" &&
			   Element.SourceRecipe.strRendererShape != "mesh"))) ||
			(bStandardColorBackend &&
			 (0.f != Element.Detail.Color.fDistortionIntensity ||
			  Element.Detail.Color.bDistortionOnBaseMaterial ||
			  0.f != Element.Detail.Color.fRadialTime ||
			  0.f != Element.Detail.Color.fRadialIntensity)))
		{
			strOutError =
				"StandardColorV1 template, carrier, or generic-only state is invalid: " +
				Element.strElementId + ".";
			return false;
		}
		const bool_t bAuthoringExecutionTarget =
			Is_EffectAuthoringExecutionTarget(Element.Material.Execution);
		const bool_t bGeometryOnlySourceCarrier =
			Is_EffectFailClosedSourceGeometryCarrier(Element);
		if (Element.bVisible && !bAuthoringExecutionTarget &&
			!bGeometryOnlySourceCarrier)
		{
			strOutError =
				"Hard fail-closed authored Element is not a typed source geometry carrier: " +
				Element.strElementId + ".";
			return false;
		}
		const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
			Element.Material.SourceMaterial;
		if (SourceMaterial.eSourceBlendClass >=
			EFFECT_SOURCE_BLEND_CLASS::END ||
			(!SourceMaterial.bEnabled &&
			 SourceMaterial.eSourceBlendClass !=
				EFFECT_SOURCE_BLEND_CLASS::UNKNOWN))
		{
			strOutError =
				"Effect source Material blend evidence is invalid.";
			return false;
		}
		if (SourceMaterial.bEnabled)
		{
			if (!Is_StableId(SourceMaterial.strProfileId) ||
				!Is_StableId(SourceMaterial.strRuntimeShaderProfileId) ||
				!Is_SupportedEffectSourceRuntimeShaderProfile(
					SourceMaterial.strRuntimeShaderProfileId) ||
				SourceMaterial.strParentMaterialPath.empty() ||
				SourceMaterial.strParentMaterialPath.size() > 512u ||
				!Has_VisibleCharacter(
					SourceMaterial.strParentMaterialPath) ||
				SourceMaterial.eStatus >=
					EFFECT_SOURCE_MATERIAL_STATUS::UNSUPPORTED ||
				!Is_StableId(SourceMaterial.strSubUVMode) ||
				!Is_SupportedEffectSourceSubUVMode(
					SourceMaterial.strSubUVMode) ||
				SourceMaterial.Textures.size() > 32u ||
				SourceMaterial.Scalars.size() > 128u ||
				SourceMaterial.Vectors.size() > 128u ||
				SourceMaterial.StaticSwitches.size() > 128u)
			{
				strOutError = "Effect source Material profile metadata is invalid.";
				return false;
			}
			std::unordered_set<std::string> TextureNames;
			for (const EFFECT_NAMED_TEXTURE_DESC& Texture :
				SourceMaterial.Textures)
			{
				EFFECT_RESOURCE_FILE_KIND eActualKind =
					EFFECT_RESOURCE_FILE_KIND::END;
				if (Texture.strName.empty() || Texture.strName.size() > 128u ||
					!Has_VisibleCharacter(Texture.strName) ||
					Texture.strGroup.size() > 128u ||
					(!Texture.strGroup.empty() &&
						!Has_VisibleCharacter(Texture.strGroup)) ||
					Texture.strSourceObjectPath.size() > 512u ||
					(!Texture.strSourceObjectPath.empty() &&
						!Has_VisibleCharacter(Texture.strSourceObjectPath)) ||
					(!Texture.strAssetId.empty() &&
						Texture.strSourceObjectPath.empty()) ||
					(!Texture.strAssetId.empty() &&
						(!CEffectDocumentCodec::Is_SafeResourceAssetId(Texture.strAssetId, &eActualKind) ||
							eActualKind != EFFECT_RESOURCE_FILE_KIND::TEXTURE)) ||
					Texture.eAddressU >= EFFECT_TEXTURE_ADDRESS_MODE::END ||
					Texture.eAddressV >= EFFECT_TEXTURE_ADDRESS_MODE::END ||
					Texture.eColorSpace >= EFFECT_TEXTURE_COLOR_SPACE::END ||
					Texture.strSamplingEvidence.empty() ||
					Texture.strSamplingEvidence.size() > 128u ||
					!Is_StableId(Texture.strSamplingEvidence) ||
					!TextureNames.insert(Texture.strName).second)
				{
					strOutError = "Effect source Material texture is invalid: " +
						Texture.strName + " (" + Texture.strAssetId + ").";
					return false;
				}
			}
			std::unordered_set<std::string> ScalarNames;
			for (const EFFECT_NAMED_FLOAT_DESC& Scalar :
				SourceMaterial.Scalars)
			{
				if (Scalar.strName.empty() || Scalar.strName.size() > 128u ||
					!Has_VisibleCharacter(Scalar.strName) ||
					Scalar.strGroup.size() > 128u ||
					(!Scalar.strGroup.empty() &&
						!Has_VisibleCharacter(Scalar.strGroup)) ||
					!std::isfinite(Scalar.fValue) ||
					!ScalarNames.insert(Scalar.strName).second)
				{
					strOutError = "Effect source Material scalar is invalid.";
					return false;
				}
			}
			std::unordered_set<std::string> VectorNames;
			for (const EFFECT_NAMED_FLOAT4_DESC& Vector :
				SourceMaterial.Vectors)
			{
				if (Vector.strName.empty() || Vector.strName.size() > 128u ||
					!Has_VisibleCharacter(Vector.strName) ||
					Vector.strGroup.size() > 128u ||
					(!Vector.strGroup.empty() &&
						!Has_VisibleCharacter(Vector.strGroup)) ||
					!Is_Finite(Vector.vValue) ||
					!VectorNames.insert(Vector.strName).second)
				{
					strOutError = "Effect source Material vector is invalid.";
					return false;
				}
			}
			std::unordered_set<std::string> SwitchNames;
			for (const EFFECT_NAMED_BOOL_DESC& Switch :
				SourceMaterial.StaticSwitches)
			{
				if (Switch.strName.empty() || Switch.strName.size() > 128u ||
					!Has_VisibleCharacter(Switch.strName) ||
					Switch.strGroup.size() > 128u ||
					(!Switch.strGroup.empty() &&
						!Has_VisibleCharacter(Switch.strGroup)) ||
					!SwitchNames.insert(Switch.strName).second)
				{
					strOutError = "Effect source Material switch is invalid.";
					return false;
				}
			}
			for (const std::string& Semantic :
				SourceMaterial.DynamicParameterSemantics)
			{
				if (!Is_StableId(Semantic) ||
					!Is_SupportedEffectSourceDynamicParameterSemantic(Semantic))
				{
					strOutError =
						"Effect source Material Dynamic Parameter semantic is invalid.";
					return false;
				}
			}
		}

		std::unordered_set<std::string> Slots;
		for (const EFFECT_RESOURCE_BINDING_DESC& Binding : Element.ResourceBindings)
		{
			const bool_t bMeshShape =
				Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			const EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput = bMeshShape ?
				nullptr : Find_EffectMaterialInput(
					*pMaterialTemplate, Binding.strSlotId);
			const EFFECT_RESOURCE_SLOT eRuntimeSlot = bMeshShape ?
				EFFECT_RESOURCE_SLOT::MESH_MODEL :
				(nullptr == pInput ? EFFECT_RESOURCE_SLOT::END :
					pInput->eRuntimeSlot);
			EFFECT_RESOURCE_FILE_KIND eActualKind = EFFECT_RESOURCE_FILE_KIND::END;
			const EFFECT_RESOURCE_FILE_KIND eExpectedKind = bMeshShape ?
				EFFECT_RESOURCE_FILE_KIND::MODEL :
				(nullptr == pInput ? EFFECT_RESOURCE_FILE_KIND::END :
					pInput->eAllowedResourceKind);
			if (!CEffectDocumentCodec::Is_ResourceSlotAllowed(Element.eKind, eRuntimeSlot) ||
				eExpectedKind == EFFECT_RESOURCE_FILE_KIND::END ||
				!Slots.insert(Binding.strSlotId).second ||
				!CEffectDocumentCodec::Is_SafeElementResourceAssetId(Element.eKind,
					Binding.strSlotId, Binding.strAssetId, &eActualKind) ||
				eActualKind != eExpectedKind)
			{
				strOutError = "Effect resource slot, path, file, or duplicate is invalid.";
				return false;
			}
		}
		/* Hard fail-closed rows remain loadable source evidence.  Every execution
		   target, including an authoring-approximate preview, must satisfy the
		   same source-profile/resource contract before it can be activated. */
		if (!bSourceContract && bAuthoringExecutionTarget)
		{
			bool_t bSourceMaterialOwnsDrawableContract = false;
			if (!Validate_ExecutableSourceMaterialCarrier(
					Element, bSourceMaterialOwnsDrawableContract, strOutError))
			{
				strOutError =
					"Ordinary authored source Material is not admitted: " +
					Element.strElementId + ": " + strOutError;
				return false;
			}
		}
		return true;
	}


	bool_t Validate_ElementSourceMaterialSlots(const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		using namespace Client;
		const auto& Slots = Element.Detail.Mesh.SourceMaterialSlots;
		if (Slots.empty())
			return Validate_ElementMaterial(Element, false, strOutError);
		if (Slots.size() > 32u || Element.eKind != EFFECT_ELEMENT_KIND::PARTICLE ||
			!Element.SourceRecipe.bEnabled || Element.SourceRecipe.strRendererShape != "mesh" ||
			Element.Detail.Mesh.bUseModelMaterial || !Element.RuntimeCarrier.Is_Empty())
		{
			strOutError = "Source material slots require an ordinary source mesh particle carrier.";
			return false;
		}
		const EFFECT_SOURCE_MODULE_DESC* pModule = nullptr;
		for (const auto& Module : Element.SourceRecipe.Modules)
		{
			if (Normalize_SourceModuleClass(Module.strClassName) == "particlemodulemeshmaterial")
			{
				if (nullptr != pModule || !Module.Distributions.empty())
				{
					strOutError = "Source material slots require one static MeshMaterial module.";
					return false;
				}
				pModule = &Module;
			}
			if (Normalize_SourceModuleClass(Module.strClassName) == "particlemoduletypedatamesh")
			{
				for (const auto& Literal : Module.Literals)
				{
					if (Literal.strPropertyPath == "boverridematerial" &&
						(Literal.eKind != EFFECT_SOURCE_LITERAL_KIND::BOOLEAN || Literal.bBoolean))
					{
						strOutError = "Mesh TypeData override conflicts with source material slots.";
						return false;
					}
				}
			}
		}
		if (nullptr == pModule)
		{
			strOutError = "Source material slots lack their MeshMaterial source module.";
			return false;
		}
		size_t iSourcePathCount = 0u;
		for (const auto& Literal : pModule->Literals)
		{
			if (Literal.strPropertyPath == "benabled" &&
				(Literal.eKind != EFFECT_SOURCE_LITERAL_KIND::BOOLEAN || !Literal.bBoolean))
			{
				strOutError = "Source MeshMaterial module is disabled or invalid.";
				return false;
			}
			if (Literal.strPropertyPath.starts_with("meshmaterials[") &&
				Literal.strPropertyPath.ends_with("].objectpath"))
				++iSourcePathCount;
		}
		if (iSourcePathCount != Slots.size())
		{
			strOutError = "Source material slots do not cover the MeshMaterial source array.";
			return false;
		}
		// The inactive Required.Material is preserved and checked as evidence;
		// it is never promoted to a fallback for a missing source slot.
		EFFECT_ELEMENT_DESC Leaf = Element;
		Leaf.Detail.Mesh.SourceMaterialSlots.clear();
		Leaf.bVisible = false;
		if (!Validate_ElementMaterial(Leaf, false, strOutError))
			return false;
		Leaf.bVisible = Element.bVisible;
		std::unordered_set<uint32_t> Indices;
		for (const auto& Slot : Slots)
		{
			const std::string Path = "meshmaterials[" +
				std::to_string(Slot.iSourceMaterialIndex) + "].objectpath";
			const auto iMatchingPaths = std::count_if(pModule->Literals.begin(),
				pModule->Literals.end(), [&](const auto& Literal)
				{
					return Literal.strPropertyPath == Path &&
						Literal.eKind == EFFECT_SOURCE_LITERAL_KIND::STRING &&
						Literal.strString == Slot.Material.strSourceMaterialPath;
				});
			Leaf.Material = Slot.Material;
			if (!Indices.insert(Slot.iSourceMaterialIndex).second || iMatchingPaths != 1 ||
				Slot.Material.strTemplateId != EFFECT_SOURCE_MATERIAL_TEMPLATE_ID ||
				!Slot.Material.SourceMaterial.bEnabled || Slot.Material.Execution.bEnabled ||
				Slot.Material.Execution.bFailClosed || Slot.Material.Execution.bAuthoringApproximate ||
				!(Has_DimensionMasterQMaterialContract(Leaf) ||
				  Has_DimensionMasterVMaterialContract(Leaf) ||
				  Has_DimensionMasterALTVMaterialContract(Leaf) ||
				  Has_DimensionMasterWRMaterialContract(Leaf) ||
				  Has_DimensionMasterSDMaterialContract(Leaf) ||
				  Has_ArtistMaterialContract(Leaf) || Has_WarlordNativeMaterialContract(Leaf) ||
				  Has_LanceMasterVAMaterialContract(Leaf)))
			{
				strOutError = "Source material slot identity or exact native material contract is invalid: " +
					Element.strElementId + " slot " + std::to_string(Slot.iSourceMaterialIndex);
				return false;
			}
			if (!Validate_ElementMaterial(Leaf, false, strOutError))
				return false;
		}
		return true;
	}

}
