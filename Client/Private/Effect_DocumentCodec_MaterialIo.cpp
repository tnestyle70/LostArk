#include "Effect_DocumentCodec_Internal.h"
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


	void Write_Renderer(
		std::ostringstream& Output,
		const Client::EFFECT_RENDERER_DESC& Renderer)
	{
		Output << "      \"renderer\": { \"type\": \""
			<< RENDERER_TYPE_TOKENS[static_cast<size_t>(Renderer.eType)]
			<< "\", \"sourceSpace\": \""
			<< SOURCE_SPACE_TOKENS[static_cast<size_t>(Renderer.eSourceSpace)]
			<< "\" },\n";
	}


	bool_t Read_SourceMaterialProfile(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_MATERIAL_DESC& Out,
		std::string& strOutError)
	{
		if (!Value.Is_Object() ||
			!Read_Bool(Value, "enabled", Out.bEnabled, strOutError))
		{
			return false;
		}
		/* A typed semantic replay may deliberately disable native source
		   execution while retaining the recovered parent/profile as immutable
		   evidence for its exact occurrence allowlist.  Minimal disabled
		   profiles remain valid; a profileId opts into full evidence parsing. */
		if (!Out.bEnabled && nullptr == Value.Find("profileId"))
			return true;
		if (const Client::DATA_JSON_VALUE* pSourceBlendClass =
			Value.Find("sourceBlendClass"))
		{
			if (!pSourceBlendClass->Is_String() ||
				!Parse_Token(pSourceBlendClass->Get_String(),
					SOURCE_BLEND_CLASS_TOKENS,
					std::size(SOURCE_BLEND_CLASS_TOKENS),
					Out.eSourceBlendClass))
			{
				strOutError =
					"Effect source Material blend class is invalid.";
				return false;
			}
		}
		const Client::DATA_JSON_VALUE* pTextures = Value.Find("textures");
		if (nullptr != pTextures && !pTextures->Is_Array())
		{
			strOutError = "Effect source Material textures must be an array.";
			return false;
		}
		const Client::DATA_JSON_VALUE* pScalars = Find_Field(
			Value, "scalars", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pVectors = Find_Field(
			Value, "vectors", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pSwitches = Find_Field(
			Value, "staticSwitches", Client::DATA_JSON_TYPE::ARRAY,
			strOutError);
		const Client::DATA_JSON_VALUE* pDynamicSemantics = Find_Field(
			Value, "dynamicParameterSemantics",
			Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pStatus = Find_Field(
			Value, "semanticStatus", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		if (nullptr == pScalars || nullptr == pVectors ||
			nullptr == pSwitches || nullptr == pDynamicSemantics ||
			nullptr == pStatus ||
			!Read_String(Value, "profileId", Out.strProfileId,
				strOutError) ||
			!Read_String(Value, "runtimeShaderProfileId",
				Out.strRuntimeShaderProfileId, strOutError) ||
			!Read_String(Value, "parentMaterialPath",
				Out.strParentMaterialPath, strOutError) ||
			!Read_String(Value, "subUVMode", Out.strSubUVMode,
				strOutError) ||
			!Parse_Token(pStatus->Get_String(),
				SOURCE_MATERIAL_STATUS_TOKENS,
				std::size(SOURCE_MATERIAL_STATUS_TOKENS), Out.eStatus) ||
			pDynamicSemantics->Get_Array().size() !=
				Out.DynamicParameterSemantics.size())
		{
			return false;
		}
		for (size_t iSemantic = 0u;
			iSemantic < Out.DynamicParameterSemantics.size(); ++iSemantic)
		{
			const Client::DATA_JSON_VALUE& Semantic =
				pDynamicSemantics->Get_Array()[iSemantic];
			if (!Semantic.Is_String())
				return false;
			Out.DynamicParameterSemantics[iSemantic] =
				Semantic.Get_String();
		}
		if (nullptr != pTextures)
		{
			for (const Client::DATA_JSON_VALUE& Item : pTextures->Get_Array())
			{
				Client::EFFECT_NAMED_TEXTURE_DESC Texture;
				if (!Item.Is_Object() ||
					!Read_String(Item, "name", Texture.strName, strOutError) ||
					!Read_String(Item, "sourceObjectPath",
						Texture.strSourceObjectPath, strOutError) ||
					!Read_String(Item, "assetId", Texture.strAssetId,
						strOutError))
				{
					return false;
				}
				if (const Client::DATA_JSON_VALUE* pGroup = Item.Find("group"))
				{
					if (!pGroup->Is_String())
						return false;
					Texture.strGroup = pGroup->Get_String();
				}
				if (const Client::DATA_JSON_VALUE* pAddressU = Item.Find("addressU"))
				{
					if (!pAddressU->Is_String() ||
						!Parse_Token(pAddressU->Get_String(),
							TEXTURE_ADDRESS_MODE_TOKENS,
							std::size(TEXTURE_ADDRESS_MODE_TOKENS),
							Texture.eAddressU))
						return false;
				}
				if (const Client::DATA_JSON_VALUE* pAddressV = Item.Find("addressV"))
				{
					if (!pAddressV->Is_String() ||
						!Parse_Token(pAddressV->Get_String(),
							TEXTURE_ADDRESS_MODE_TOKENS,
							std::size(TEXTURE_ADDRESS_MODE_TOKENS),
							Texture.eAddressV))
						return false;
				}
				if (const Client::DATA_JSON_VALUE* pColorSpace =
					Item.Find("colorSpace"))
				{
					if (!pColorSpace->Is_String() ||
						!Parse_Token(pColorSpace->Get_String(),
							TEXTURE_COLOR_SPACE_TOKENS,
							std::size(TEXTURE_COLOR_SPACE_TOKENS),
							Texture.eColorSpace))
						return false;
				}
				if (const Client::DATA_JSON_VALUE* pEvidence =
					Item.Find("samplingEvidence"))
				{
					if (!pEvidence->Is_String())
						return false;
					Texture.strSamplingEvidence = pEvidence->Get_String();
				}
				Out.Textures.push_back(std::move(Texture));
			}
		}
		for (const Client::DATA_JSON_VALUE& Item : pScalars->Get_Array())
		{
			Client::EFFECT_NAMED_FLOAT_DESC Scalar;
			if (!Item.Is_Object() ||
				!Read_String(Item, "name", Scalar.strName, strOutError) ||
				!Read_Float(Item, "value", Scalar.fValue, strOutError))
			{
				return false;
			}
			if (const Client::DATA_JSON_VALUE* pGroup = Item.Find("group"))
			{
				if (!pGroup->Is_String())
					return false;
				Scalar.strGroup = pGroup->Get_String();
			}
			Out.Scalars.push_back(std::move(Scalar));
		}
		for (const Client::DATA_JSON_VALUE& Item : pVectors->Get_Array())
		{
			Client::EFFECT_NAMED_FLOAT4_DESC Vector;
			if (!Item.Is_Object() ||
				!Read_String(Item, "name", Vector.strName, strOutError) ||
				!Read_Array(Item, "value", &Vector.vValue.x, 4u,
					strOutError))
			{
				return false;
			}
			if (const Client::DATA_JSON_VALUE* pGroup = Item.Find("group"))
			{
				if (!pGroup->Is_String())
					return false;
				Vector.strGroup = pGroup->Get_String();
			}
			Out.Vectors.push_back(std::move(Vector));
		}
		for (const Client::DATA_JSON_VALUE& Item : pSwitches->Get_Array())
		{
			Client::EFFECT_NAMED_BOOL_DESC Switch;
			if (!Item.Is_Object() ||
				!Read_String(Item, "name", Switch.strName, strOutError) ||
				!Read_Bool(Item, "value", Switch.bValue, strOutError))
			{
				return false;
			}
			if (const Client::DATA_JSON_VALUE* pGroup = Item.Find("group"))
			{
				if (!pGroup->Is_String())
					return false;
				Switch.strGroup = pGroup->Get_String();
			}
			Out.StaticSwitches.push_back(std::move(Switch));
		}
		return true;
	}


	const char_t* SourceMaterialStatusToken(
		const Client::EFFECT_SOURCE_MATERIAL_STATUS eStatus)
	{
		const size_t iIndex = static_cast<size_t>(eStatus);
		return iIndex < std::size(SOURCE_MATERIAL_STATUS_TOKENS) ?
			SOURCE_MATERIAL_STATUS_TOKENS[iIndex] : "unsupported";
	}


	void Write_SourceMaterialProfile(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source)
	{
		Output << "{ \"enabled\": "
			<< (Source.bEnabled ? "true" : "false");
		/* Preserve optional disabled source evidence across Tool round trips.
		   Empty disabled profiles keep the compact legacy representation. */
		if (!Source.bEnabled && Source.strProfileId.empty())
		{
			Output << " }";
			return;
		}
		Output << ", \"profileId\": \""
			<< Client::CDataJson::Escape(Source.strProfileId)
			<< "\", \"runtimeShaderProfileId\": \""
			<< Client::CDataJson::Escape(Source.strRuntimeShaderProfileId)
			<< "\", \"parentMaterialPath\": \""
			<< Client::CDataJson::Escape(Source.strParentMaterialPath)
			<< "\", \"semanticStatus\": \""
			<< SourceMaterialStatusToken(Source.eStatus) << '"';
		if (Source.eSourceBlendClass !=
			Client::EFFECT_SOURCE_BLEND_CLASS::UNKNOWN)
		{
			Output << ", \"sourceBlendClass\": \""
				<< SOURCE_BLEND_CLASS_TOKENS[static_cast<size_t>(
					Source.eSourceBlendClass)] << '"';
		}
		Output << ", \"textures\": [";
		for (size_t i = 0u; i < Source.Textures.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << "{ \"name\": \""
				<< Client::CDataJson::Escape(Source.Textures[i].strName)
				<< '"';
			if (!Source.Textures[i].strGroup.empty())
			{
				Output << ", \"group\": \""
					<< Client::CDataJson::Escape(Source.Textures[i].strGroup)
					<< '"';
			}
			Output << ", \"sourceObjectPath\": \""
				<< Client::CDataJson::Escape(
					Source.Textures[i].strSourceObjectPath)
				<< "\", \"assetId\": \""
				<< Client::CDataJson::Escape(Source.Textures[i].strAssetId)
				<< "\", \"addressU\": \""
				<< TEXTURE_ADDRESS_MODE_TOKENS[
					static_cast<size_t>(Source.Textures[i].eAddressU)]
				<< "\", \"addressV\": \""
				<< TEXTURE_ADDRESS_MODE_TOKENS[
					static_cast<size_t>(Source.Textures[i].eAddressV)]
				<< "\", \"colorSpace\": \""
				<< TEXTURE_COLOR_SPACE_TOKENS[
					static_cast<size_t>(Source.Textures[i].eColorSpace)]
				<< "\", \"samplingEvidence\": \""
				<< Client::CDataJson::Escape(
					Source.Textures[i].strSamplingEvidence)
				<< "\" }";
		}
		Output << "], \"scalars\": [";
		for (size_t i = 0u; i < Source.Scalars.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << "{ \"name\": \""
				<< Client::CDataJson::Escape(Source.Scalars[i].strName)
				<< '"';
			if (!Source.Scalars[i].strGroup.empty())
			{
				Output << ", \"group\": \""
					<< Client::CDataJson::Escape(Source.Scalars[i].strGroup)
					<< '"';
			}
			Output << ", \"value\": " << Source.Scalars[i].fValue << " }";
		}
		Output << "], \"vectors\": [";
		for (size_t i = 0u; i < Source.Vectors.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << "{ \"name\": \""
				<< Client::CDataJson::Escape(Source.Vectors[i].strName)
				<< '"';
			if (!Source.Vectors[i].strGroup.empty())
			{
				Output << ", \"group\": \""
					<< Client::CDataJson::Escape(Source.Vectors[i].strGroup)
					<< '"';
			}
			Output << ", \"value\": ";
			Write_Float4(Output, Source.Vectors[i].vValue);
			Output << " }";
		}
		Output << "], \"staticSwitches\": [";
		for (size_t i = 0u; i < Source.StaticSwitches.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << "{ \"name\": \""
				<< Client::CDataJson::Escape(
					Source.StaticSwitches[i].strName)
				<< '"';
			if (!Source.StaticSwitches[i].strGroup.empty())
			{
				Output << ", \"group\": \""
					<< Client::CDataJson::Escape(
						Source.StaticSwitches[i].strGroup)
					<< '"';
			}
			Output << ", \"value\": "
				<< (Source.StaticSwitches[i].bValue ? "true" : "false")
				<< " }";
		}
		Output << "], \"dynamicParameterSemantics\": [";
		for (size_t i = 0u; i < Source.DynamicParameterSemantics.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << '"' << Client::CDataJson::Escape(
				Source.DynamicParameterSemantics[i]) << '"';
		}
		Output << "], \"subUVMode\": \""
			<< Client::CDataJson::Escape(Source.strSubUVMode) << "\" }";
	}


	bool_t Read_MaterialSampler(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_MATERIAL_SAMPLER_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value,
			{ "filter", "addressU", "addressV", "addressW", "mipLodBias",
				"maxAnisotropy", "comparison", "borderColor", "minLod",
				"maxLod" }, "Effect authored Material sampler", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pFilter = Find_Field(
			Value, "filter", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pAddressU = Find_Field(
			Value, "addressU", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pAddressV = Find_Field(
			Value, "addressV", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pAddressW = Find_Field(
			Value, "addressW", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pComparison = Find_Field(
			Value, "comparison", Client::DATA_JSON_TYPE::STRING, strOutError);
		if (nullptr == pFilter || nullptr == pAddressU || nullptr == pAddressV ||
			nullptr == pAddressW || nullptr == pComparison ||
			!Parse_Token(pFilter->Get_String(), MATERIAL_TEXTURE_FILTER_TOKENS,
				std::size(MATERIAL_TEXTURE_FILTER_TOKENS), Out.eFilter) ||
			!Parse_Token(pAddressU->Get_String(),
				MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS,
				std::size(MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS), Out.eAddressU) ||
			!Parse_Token(pAddressV->Get_String(),
				MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS,
				std::size(MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS), Out.eAddressV) ||
			!Parse_Token(pAddressW->Get_String(),
				MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS,
				std::size(MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS), Out.eAddressW) ||
			!Parse_Token(pComparison->Get_String(),
				MATERIAL_COMPARISON_FUNCTION_TOKENS,
				std::size(MATERIAL_COMPARISON_FUNCTION_TOKENS), Out.eComparison) ||
			!Read_Float(Value, "mipLodBias", Out.fMipLodBias, strOutError) ||
			!Read_UInt(Value, "maxAnisotropy", Out.iMaxAnisotropy,
				strOutError) ||
			!Read_Array(Value, "borderColor", &Out.vBorderColor.x, 4u,
				strOutError) ||
			!Read_Float(Value, "minLod", Out.fMinLod, strOutError) ||
			!Read_Float(Value, "maxLod", Out.fMaxLod, strOutError))
		{
			return false;
		}
		return true;
	}


	void Write_MaterialSampler(
		std::ostringstream& Output,
		const Client::EFFECT_MATERIAL_SAMPLER_DESC& Sampler)
	{
		Output << "{ \"filter\": \""
			<< MATERIAL_TEXTURE_FILTER_TOKENS[
				static_cast<size_t>(Sampler.eFilter)]
			<< "\", \"addressU\": \""
			<< MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS[
				static_cast<size_t>(Sampler.eAddressU)]
			<< "\", \"addressV\": \""
			<< MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS[
				static_cast<size_t>(Sampler.eAddressV)]
			<< "\", \"addressW\": \""
			<< MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS[
				static_cast<size_t>(Sampler.eAddressW)]
			<< "\", \"mipLodBias\": " << Sampler.fMipLodBias
			<< ", \"maxAnisotropy\": " << Sampler.iMaxAnisotropy
			<< ", \"comparison\": \""
			<< MATERIAL_COMPARISON_FUNCTION_TOKENS[
				static_cast<size_t>(Sampler.eComparison)]
			<< "\", \"borderColor\": ";
		Write_Float4(Output, Sampler.vBorderColor);
		Output << ", \"minLod\": " << Sampler.fMinLod
			<< ", \"maxLod\": " << Sampler.fMaxLod << " }";
	}


	bool_t Read_StandardColorV1(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_STANDARD_COLOR_V1_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value,
			{ "packetVersion", "baseRadianceLaneId", "baseRadianceChannel",
				"coverageLaneId", "coverageChannel", "emissiveMode",
				"lifetimeEnvelope", "dissolveMode", "dissolveLaneId",
				"dissolveChannel", "dissolveSoftness", "missingLanePolicy" },
			"Effect StandardColorV1 packet", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pBaseChannel = Find_Field(
			Value, "baseRadianceChannel", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		const Client::DATA_JSON_VALUE* pCoverageChannel = Find_Field(
			Value, "coverageChannel", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		const Client::DATA_JSON_VALUE* pEmissiveMode = Find_Field(
			Value, "emissiveMode", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pLifetimeEnvelope = Find_Field(
			Value, "lifetimeEnvelope", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		const Client::DATA_JSON_VALUE* pDissolveMode = Find_Field(
			Value, "dissolveMode", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pDissolveChannel = Find_Field(
			Value, "dissolveChannel", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		const Client::DATA_JSON_VALUE* pMissingLanePolicy = Find_Field(
			Value, "missingLanePolicy", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		return nullptr != pBaseChannel && nullptr != pCoverageChannel &&
			nullptr != pEmissiveMode && nullptr != pLifetimeEnvelope &&
			nullptr != pDissolveMode && nullptr != pDissolveChannel &&
			nullptr != pMissingLanePolicy &&
			Read_UInt(Value, "packetVersion", Out.iPacketVersion, strOutError) &&
			Read_String(Value, "baseRadianceLaneId",
				Out.strBaseRadianceLaneId, strOutError) &&
			Parse_Token(pBaseChannel->Get_String(),
				STANDARD_COLOR_CHANNEL_TOKENS,
				std::size(STANDARD_COLOR_CHANNEL_TOKENS),
				Out.eBaseRadianceChannel) &&
			Read_String(Value, "coverageLaneId", Out.strCoverageLaneId,
				strOutError) &&
			Parse_Token(pCoverageChannel->Get_String(),
				STANDARD_COLOR_CHANNEL_TOKENS,
				std::size(STANDARD_COLOR_CHANNEL_TOKENS),
				Out.eCoverageChannel) &&
			Parse_Token(pEmissiveMode->Get_String(),
				STANDARD_COLOR_EMISSIVE_MODE_TOKENS,
				std::size(STANDARD_COLOR_EMISSIVE_MODE_TOKENS),
				Out.eEmissiveMode) &&
			Parse_Token(pLifetimeEnvelope->Get_String(),
				STANDARD_COLOR_LIFETIME_ENVELOPE_TOKENS,
				std::size(STANDARD_COLOR_LIFETIME_ENVELOPE_TOKENS),
				Out.eLifetimeEnvelope) &&
			Parse_Token(pDissolveMode->Get_String(),
				STANDARD_COLOR_DISSOLVE_MODE_TOKENS,
				std::size(STANDARD_COLOR_DISSOLVE_MODE_TOKENS),
				Out.eDissolveMode) &&
			Read_String(Value, "dissolveLaneId", Out.strDissolveLaneId,
				strOutError) &&
			Parse_Token(pDissolveChannel->Get_String(),
				STANDARD_COLOR_CHANNEL_TOKENS,
				std::size(STANDARD_COLOR_CHANNEL_TOKENS),
				Out.eDissolveChannel) &&
			Read_Float(Value, "dissolveSoftness", Out.fDissolveSoftness,
				strOutError) &&
			Parse_Token(pMissingLanePolicy->Get_String(),
				STANDARD_COLOR_MISSING_LANE_POLICY_TOKENS,
				std::size(STANDARD_COLOR_MISSING_LANE_POLICY_TOKENS),
				Out.eMissingLanePolicy);
	}


	void Write_StandardColorV1(
		std::ostringstream& Output,
		const Client::EFFECT_STANDARD_COLOR_V1_DESC& Packet)
	{
		Output << "{ \"packetVersion\": " << Packet.iPacketVersion
			<< ", \"baseRadianceLaneId\": \""
			<< Client::CDataJson::Escape(Packet.strBaseRadianceLaneId)
			<< "\", \"baseRadianceChannel\": \""
			<< STANDARD_COLOR_CHANNEL_TOKENS[
				static_cast<size_t>(Packet.eBaseRadianceChannel)]
			<< "\", \"coverageLaneId\": \""
			<< Client::CDataJson::Escape(Packet.strCoverageLaneId)
			<< "\", \"coverageChannel\": \""
			<< STANDARD_COLOR_CHANNEL_TOKENS[
				static_cast<size_t>(Packet.eCoverageChannel)]
			<< "\", \"emissiveMode\": \""
			<< STANDARD_COLOR_EMISSIVE_MODE_TOKENS[
				static_cast<size_t>(Packet.eEmissiveMode)]
			<< "\", \"lifetimeEnvelope\": \""
			<< STANDARD_COLOR_LIFETIME_ENVELOPE_TOKENS[
				static_cast<size_t>(Packet.eLifetimeEnvelope)]
			<< "\", \"dissolveMode\": \""
			<< STANDARD_COLOR_DISSOLVE_MODE_TOKENS[
				static_cast<size_t>(Packet.eDissolveMode)]
			<< "\", \"dissolveLaneId\": \""
			<< Client::CDataJson::Escape(Packet.strDissolveLaneId)
			<< "\", \"dissolveChannel\": \""
			<< STANDARD_COLOR_CHANNEL_TOKENS[
				static_cast<size_t>(Packet.eDissolveChannel)]
			<< "\", \"dissolveSoftness\": " << Packet.fDissolveSoftness
			<< ", \"missingLanePolicy\": \""
			<< STANDARD_COLOR_MISSING_LANE_POLICY_TOKENS[
				static_cast<size_t>(Packet.eMissingLanePolicy)] << "\" }";
	}


	bool_t Read_MaterialExecution(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_MATERIAL_EXECUTION_DESC& Out,
		std::string& strOutError)
	{
		if (!Value.Is_Object() ||
			!Read_Bool(Value, "enabled", Out.bEnabled, strOutError))
		{
			return false;
		}
		if (!Out.bEnabled)
		{
			const Client::DATA_JSON_VALUE* pFailClosed =
				Value.Find("failClosed");
			if (nullptr != pFailClosed)
			{
				if (!pFailClosed->Is_Boolean() ||
					!pFailClosed->Get_Boolean())
				{
					strOutError =
						"Disabled authored Material failClosed must be true when present.";
					return false;
				}
				Out.bFailClosed = true;
			}
			const Client::DATA_JSON_VALUE* pAuthoringApproximate =
				Value.Find("authoringApproximate");
			if (nullptr != pAuthoringApproximate)
			{
				if (!pAuthoringApproximate->Is_Boolean() ||
					!pAuthoringApproximate->Get_Boolean())
				{
					strOutError =
						"Disabled authored Material authoringApproximate must be true when present.";
					return false;
				}
				if (!Out.bFailClosed)
				{
					strOutError =
						"Authoring-approximate authored Material must stay fail-closed.";
					return false;
				}
				Out.bAuthoringApproximate = true;
			}
			const size_t iExpectedFieldCount = 1u +
				(Out.bFailClosed ? 1u : 0u) +
				(Out.bAuthoringApproximate ? 1u : 0u);
			if (iExpectedFieldCount != Value.Get_Object().size())
			{
				strOutError =
					"Disabled authored Material execution carries hidden state.";
				return false;
			}
			return true;
		}
		if (!Validate_ExactFields(Value,
			{ "enabled", "fidelity", "version", "backend", "opcode", "passIndex",
				"renderState", "textureLaneCount", "textureMask",
				"textureLanes", "dynamicConsumedMask",
				"dynamicSuppressedMask", "particleColorPolicy",
				"particleColorConsumedMask", "particleColorSuppressedMask",
				"scalarCount", "vectorCount", "inputCount",
				"inputConsumedMask", "inputSuppressedMask",
				"vectorComponentConsumedMask",
				"vectorComponentSuppressedMask", "staticInputCount",
				"staticSelectedMask", "staticConsumedMask",
				"staticSuppressedMask", "renderInputCount",
				"renderConsumedMask", "renderSuppressedMask", "scalars",
				"vectors", "artistParameters", "colors", "standardColor" },
			"Effect authored Material execution",
			strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pBackend = Find_Field(
			Value, "backend", Client::DATA_JSON_TYPE::STRING, strOutError);
		Out.eFidelity =
			Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::SOURCE_EXACT;
		const Client::DATA_JSON_VALUE* pFidelity = Value.Find("fidelity");
		const Client::DATA_JSON_VALUE* pRenderState = Find_Field(
			Value, "renderState", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pTextureLanes = Find_Field(
			Value, "textureLanes", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pScalars = Find_Field(
			Value, "scalars", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pVectors = Find_Field(
			Value, "vectors", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pArtistParameters = Find_Field(
			Value, "artistParameters", Client::DATA_JSON_TYPE::ARRAY,
			strOutError);
		const Client::DATA_JSON_VALUE* pColors = Find_Field(
			Value, "colors", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pStandardColor =
			Value.Find("standardColor");
		if (nullptr != pFidelity &&
			(!pFidelity->Is_String() ||
			 pFidelity->Get_String() != "PROJECT_TUNED_APPROX"))
		{
			strOutError = "Effect material execution fidelity is unsupported.";
			return false;
		}
		if (nullptr != pFidelity)
		{
			Out.eFidelity = Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::
				PROJECT_TUNED_APPROX;
		}
		if (nullptr == pBackend || nullptr == pRenderState ||
			nullptr == pTextureLanes || nullptr == pScalars ||
			nullptr == pVectors || nullptr == pArtistParameters ||
			nullptr == pColors ||
			!Parse_Token(pBackend->Get_String(),
				MATERIAL_EXECUTION_BACKEND_TOKENS,
				std::size(MATERIAL_EXECUTION_BACKEND_TOKENS), Out.eBackend) ||
			!Validate_ExactFields(*pRenderState,
				{ "rasterizer", "depthStencil", "blend", "stencilReference" },
				"Effect authored Material render state", strOutError) ||
			!Read_UInt(Value, "version", Out.iVersion, strOutError) ||
			!Read_UInt(Value, "opcode", Out.iOpcode, strOutError) ||
			!Read_UInt(Value, "passIndex", Out.iPassIndex, strOutError) ||
			!Read_String(*pRenderState, "rasterizer", Out.strRasterizerState,
				strOutError) ||
			!Read_String(*pRenderState, "depthStencil",
				Out.strDepthStencilState, strOutError) ||
			!Read_String(*pRenderState, "blend", Out.strBlendState,
				strOutError) ||
			!Read_UInt(*pRenderState, "stencilReference",
				Out.iStencilReference, strOutError) ||
			!Read_UInt(Value, "textureLaneCount", Out.iTextureLaneCount,
				strOutError) ||
			!Read_UInt(Value, "textureMask", Out.iTextureMask, strOutError) ||
			!Read_UInt(Value, "dynamicConsumedMask",
				Out.iDynamicConsumedMask, strOutError) ||
			!Read_UInt(Value, "dynamicSuppressedMask",
				Out.iDynamicSuppressedMask, strOutError) ||
			!Read_UInt(Value, "particleColorPolicy", Out.iParticleColorPolicy,
				strOutError) ||
			!Read_UInt(Value, "particleColorConsumedMask",
				Out.iParticleColorConsumedMask, strOutError) ||
			!Read_UInt(Value, "particleColorSuppressedMask",
				Out.iParticleColorSuppressedMask, strOutError) ||
			!Read_UInt(Value, "scalarCount", Out.iScalarCount, strOutError) ||
			!Read_UInt(Value, "vectorCount", Out.iVectorCount, strOutError) ||
			!Read_UInt(Value, "inputCount", Out.iInputCount, strOutError) ||
			!Read_UIntArray(Value, "inputConsumedMask", Out.InputConsumedMask,
				strOutError) ||
			!Read_UIntArray(Value, "inputSuppressedMask", Out.InputSuppressedMask,
				strOutError) ||
			!Read_UIntArray(Value, "vectorComponentConsumedMask",
				Out.VectorComponentConsumedMask, strOutError) ||
			!Read_UIntArray(Value, "vectorComponentSuppressedMask",
				Out.VectorComponentSuppressedMask, strOutError) ||
			!Read_UInt(Value, "staticInputCount", Out.iStaticInputCount,
				strOutError) ||
			!Read_UInt(Value, "staticSelectedMask", Out.iStaticSelectedMask,
				strOutError) ||
			!Read_UInt(Value, "staticConsumedMask", Out.iStaticConsumedMask,
				strOutError) ||
			!Read_UInt(Value, "staticSuppressedMask", Out.iStaticSuppressedMask,
				strOutError) ||
			!Read_UInt(Value, "renderInputCount", Out.iRenderInputCount,
				strOutError) ||
			!Read_UInt(Value, "renderConsumedMask", Out.iRenderConsumedMask,
				strOutError) ||
			!Read_UInt(Value, "renderSuppressedMask", Out.iRenderSuppressedMask,
				strOutError))
		{
			return false;
		}
		const bool_t bProjectTunedApprox = Out.eFidelity ==
			Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::PROJECT_TUNED_APPROX;
		const bool_t bProjectTunedOpcode = Out.eBackend ==
			Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			(Out.iOpcode == 1001u || Out.iOpcode == 1002u ||
			 Out.iOpcode == 1003u || Out.iOpcode == 1004u);
		if (bProjectTunedApprox != bProjectTunedOpcode)
		{
			strOutError =
				"Effect material execution fidelity/opcode contract changed.";
			return false;
		}
		if (nullptr != pStandardColor &&
			(!pStandardColor->Is_Object() ||
			 !Read_StandardColorV1(
				 *pStandardColor, Out.StandardColorV1, strOutError)))
		{
			return false;
		}
		const bool_t bStandardColorBackend = Out.eBackend ==
			Client::EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1;
		if (bStandardColorBackend != (nullptr != pStandardColor))
		{
			strOutError =
				"Effect StandardColorV1 packet presence does not match its backend.";
			return false;
		}

		Out.TextureLanes.reserve(pTextureLanes->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& LaneValue :
			pTextureLanes->Get_Array())
		{
			if (!Validate_ExactFields(LaneValue,
				{ "laneId", "role", "assetId", "textureRegister",
					"samplerRegister", "sourceChannel", "colorSpace", "sampler" },
				"Effect authored Material texture lane", strOutError))
			{
				return false;
			}
			const Client::DATA_JSON_VALUE* pColorSpace = Find_Field(
				LaneValue, "colorSpace", Client::DATA_JSON_TYPE::STRING,
				strOutError);
			const Client::DATA_JSON_VALUE* pSampler = Find_Field(
				LaneValue, "sampler", Client::DATA_JSON_TYPE::OBJECT,
				strOutError);
			Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC Lane;
			if (nullptr == pColorSpace || nullptr == pSampler ||
				!Read_String(LaneValue, "laneId", Lane.strLaneId, strOutError) ||
				!Read_String(LaneValue, "role", Lane.strRole, strOutError) ||
				!Read_String(LaneValue, "assetId", Lane.strAssetId,
					strOutError) ||
				!Read_UInt(LaneValue, "textureRegister", Lane.iTextureRegister,
					strOutError) ||
				!Read_UInt(LaneValue, "samplerRegister", Lane.iSamplerRegister,
					strOutError) ||
				!Read_String(LaneValue, "sourceChannel", Lane.strSourceChannel,
					strOutError) ||
				!Parse_Token(pColorSpace->Get_String(), TEXTURE_COLOR_SPACE_TOKENS,
					std::size(TEXTURE_COLOR_SPACE_TOKENS), Lane.eColorSpace) ||
				!Read_MaterialSampler(*pSampler, Lane.Sampler, strOutError))
			{
				return false;
			}
			Out.TextureLanes.push_back(std::move(Lane));
		}

		Out.Scalars.reserve(pScalars->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& ParameterValue :
			pScalars->Get_Array())
		{
			if (!Validate_ExactFields(ParameterValue,
				{ "name", "packedIndex", "value" },
				"Effect authored Material scalar", strOutError))
			{
				return false;
			}
			Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC Parameter;
			if (!Read_String(ParameterValue, "name", Parameter.strName,
					strOutError) ||
				!Read_UInt(ParameterValue, "packedIndex", Parameter.iPackedIndex,
					strOutError) ||
				!Read_Float(ParameterValue, "value", Parameter.fValue,
					strOutError))
			{
				return false;
			}
			Out.Scalars.push_back(std::move(Parameter));
		}

		const auto ReadVectorParameters = [&strOutError](
			const Client::DATA_JSON_VALUE& Parameters,
			std::vector<Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC>& OutValues,
			const std::string_view strContext) -> bool_t
		{
			OutValues.reserve(Parameters.Get_Array().size());
			for (const Client::DATA_JSON_VALUE& ParameterValue :
				Parameters.Get_Array())
			{
				if (!Validate_ExactFields(ParameterValue,
					{ "name", "packedIndex", "value" }, strContext,
					strOutError))
				{
					return false;
				}
				Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC Parameter;
				if (!Read_String(ParameterValue, "name", Parameter.strName,
						strOutError) ||
					!Read_UInt(ParameterValue, "packedIndex",
						Parameter.iPackedIndex, strOutError) ||
					!Read_Array(ParameterValue, "value", &Parameter.vValue.x, 4u,
						strOutError))
				{
					return false;
				}
				OutValues.push_back(std::move(Parameter));
			}
			return true;
		};
		return ReadVectorParameters(*pVectors, Out.Vectors,
			"Effect authored Material vector") &&
			ReadVectorParameters(*pArtistParameters, Out.ArtistParameters,
				"Effect authored Artist Visual parameter") &&
			ReadVectorParameters(*pColors, Out.Colors,
				"Effect authored Material color");
	}


	void Write_MaterialExecution(
		std::ostringstream& Output,
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution)
	{
		Output << "{ \"enabled\": "
			<< (Execution.bEnabled ? "true" : "false");
		if (!Execution.bEnabled)
		{
			if (Execution.bFailClosed)
				Output << ", \"failClosed\": true";
			if (Execution.bAuthoringApproximate)
				Output << ", \"authoringApproximate\": true";
			Output << " }";
			return;
		}
		if (Execution.eFidelity ==
			Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::PROJECT_TUNED_APPROX)
		{
			Output << ", \"fidelity\": \"PROJECT_TUNED_APPROX\"";
		}
		Output << ", \"version\": " << Execution.iVersion
			<< ", \"backend\": \""
			<< MATERIAL_EXECUTION_BACKEND_TOKENS[
				static_cast<size_t>(Execution.eBackend)]
			<< "\", \"opcode\": " << Execution.iOpcode
			<< ", \"passIndex\": " << Execution.iPassIndex
			<< ", \"renderState\": { \"rasterizer\": \""
			<< Client::CDataJson::Escape(Execution.strRasterizerState)
			<< "\", \"depthStencil\": \""
			<< Client::CDataJson::Escape(Execution.strDepthStencilState)
			<< "\", \"blend\": \""
			<< Client::CDataJson::Escape(Execution.strBlendState)
			<< "\", \"stencilReference\": " << Execution.iStencilReference
			<< " }, \"textureLaneCount\": " << Execution.iTextureLaneCount
			<< ", \"textureMask\": " << Execution.iTextureMask
			<< ", \"textureLanes\": [";
		for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
		{
			if (0u != iLane)
				Output << ", ";
			const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane =
				Execution.TextureLanes[iLane];
			Output << "{ \"laneId\": \""
				<< Client::CDataJson::Escape(Lane.strLaneId)
				<< "\", \"role\": \""
				<< Client::CDataJson::Escape(Lane.strRole)
				<< "\", \"assetId\": \""
				<< Client::CDataJson::Escape(Lane.strAssetId)
				<< "\", \"textureRegister\": " << Lane.iTextureRegister
				<< ", \"samplerRegister\": " << Lane.iSamplerRegister
				<< ", \"sourceChannel\": \""
				<< Client::CDataJson::Escape(Lane.strSourceChannel)
				<< "\", \"colorSpace\": \""
				<< TEXTURE_COLOR_SPACE_TOKENS[
					static_cast<size_t>(Lane.eColorSpace)]
				<< "\", \"sampler\": ";
			Write_MaterialSampler(Output, Lane.Sampler);
			Output << " }";
		}
		Output << "]";
		if (Execution.eBackend ==
			Client::EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1)
		{
			Output << ", \"standardColor\": ";
			Write_StandardColorV1(Output, Execution.StandardColorV1);
		}
		Output << ", \"dynamicConsumedMask\": " << Execution.iDynamicConsumedMask
			<< ", \"dynamicSuppressedMask\": "
			<< Execution.iDynamicSuppressedMask
			<< ", \"particleColorPolicy\": "
			<< Execution.iParticleColorPolicy
			<< ", \"particleColorConsumedMask\": "
			<< Execution.iParticleColorConsumedMask
			<< ", \"particleColorSuppressedMask\": "
			<< Execution.iParticleColorSuppressedMask
			<< ", \"scalarCount\": " << Execution.iScalarCount
			<< ", \"vectorCount\": " << Execution.iVectorCount
			<< ", \"inputCount\": " << Execution.iInputCount
			<< ", \"inputConsumedMask\": ";
		Write_UIntArray(Output, Execution.InputConsumedMask);
		Output << ", \"inputSuppressedMask\": ";
		Write_UIntArray(Output, Execution.InputSuppressedMask);
		Output << ", \"vectorComponentConsumedMask\": ";
		Write_UIntArray(Output, Execution.VectorComponentConsumedMask);
		Output << ", \"vectorComponentSuppressedMask\": ";
		Write_UIntArray(Output, Execution.VectorComponentSuppressedMask);
		Output << ", \"staticInputCount\": " << Execution.iStaticInputCount
			<< ", \"staticSelectedMask\": "
			<< Execution.iStaticSelectedMask
			<< ", \"staticConsumedMask\": "
			<< Execution.iStaticConsumedMask
			<< ", \"staticSuppressedMask\": "
			<< Execution.iStaticSuppressedMask
			<< ", \"renderInputCount\": " << Execution.iRenderInputCount
			<< ", \"renderConsumedMask\": "
			<< Execution.iRenderConsumedMask
			<< ", \"renderSuppressedMask\": "
			<< Execution.iRenderSuppressedMask
			<< ", \"scalars\": [";
		for (size_t i = 0u; i < Execution.Scalars.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << "{ \"name\": \""
				<< Client::CDataJson::Escape(Execution.Scalars[i].strName)
				<< "\", \"packedIndex\": "
				<< Execution.Scalars[i].iPackedIndex
				<< ", \"value\": " << Execution.Scalars[i].fValue << " }";
		}
		const auto WriteVectorParameters = [&Output](const auto& Parameters)
		{
			for (size_t i = 0u; i < Parameters.size(); ++i)
			{
				if (0u != i)
					Output << ", ";
				Output << "{ \"name\": \""
					<< Client::CDataJson::Escape(Parameters[i].strName)
					<< "\", \"packedIndex\": "
					<< Parameters[i].iPackedIndex << ", \"value\": ";
				Write_Float4(Output, Parameters[i].vValue);
				Output << " }";
			}
		};
		Output << "], \"vectors\": [";
		WriteVectorParameters(Execution.Vectors);
		Output << "], \"artistParameters\": [";
		WriteVectorParameters(Execution.ArtistParameters);
		Output << "], \"colors\": [";
		WriteVectorParameters(Execution.Colors);
		Output << "] }";
	}


	bool_t Is_DefaultStandardColorV1(
		const Client::EFFECT_STANDARD_COLOR_V1_DESC& Packet)
	{
		return 0u == Packet.iPacketVersion &&
			Packet.strBaseRadianceLaneId.empty() &&
			Packet.eBaseRadianceChannel ==
				Client::EFFECT_STANDARD_COLOR_CHANNEL::INVALID &&
			Packet.strCoverageLaneId.empty() &&
			Packet.eCoverageChannel ==
				Client::EFFECT_STANDARD_COLOR_CHANNEL::INVALID &&
			Packet.eEmissiveMode ==
				Client::EFFECT_STANDARD_COLOR_EMISSIVE_MODE::NONE &&
			Packet.eLifetimeEnvelope ==
				Client::EFFECT_STANDARD_COLOR_LIFETIME_ENVELOPE::INVALID &&
			Packet.eDissolveMode ==
				Client::EFFECT_STANDARD_COLOR_DISSOLVE_MODE::NONE &&
			Packet.strDissolveLaneId.empty() &&
			Packet.eDissolveChannel ==
				Client::EFFECT_STANDARD_COLOR_CHANNEL::INVALID &&
			0.f == Packet.fDissolveSoftness &&
			Packet.eMissingLanePolicy ==
				Client::EFFECT_STANDARD_COLOR_MISSING_LANE_POLICY::INVALID;
	}


	std::string_view StandardColorChannelCharacters(
		const Client::EFFECT_STANDARD_COLOR_CHANNEL eChannel)
	{
		switch (eChannel)
		{
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::R:
			return "R";
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::G:
			return "G";
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::B:
			return "B";
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::A:
			return "A";
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::RGB:
			return "RGB";
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::INVALID:
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::END:
		default:
			return {};
		}
	}


	bool_t Is_CanonicalStandardColorSourceChannel(
		const std::string_view strChannel)
	{
		if (strChannel.empty() || strChannel.size() > 4u)
			return false;
		size_t iPrevious = 0u;
		bool_t bFirst = true;
		for (const char_t Character : strChannel)
		{
			const size_t iPosition = std::string_view("RGBA").find(Character);
			if (iPosition == std::string_view::npos ||
				(!bFirst && iPosition <= iPrevious))
			{
				return false;
			}
			iPrevious = iPosition;
			bFirst = false;
		}
		return true;
	}


	bool_t StandardColorLaneContainsChannel(
		const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane,
		const Client::EFFECT_STANDARD_COLOR_CHANNEL eChannel)
	{
		const std::string_view Required =
			StandardColorChannelCharacters(eChannel);
		return !Required.empty() && std::all_of(
			Required.begin(), Required.end(), [&Lane](const char_t Character)
			{
				return Lane.strSourceChannel.find(Character) != std::string::npos;
			});
	}


	bool_t Validate_StandardColorV1Execution(
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution,
		std::string& strOutError)
	{
		using namespace Client;
		const bool_t bStandard = Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1;
		if (!bStandard)
		{
			if (!Is_DefaultStandardColorV1(Execution.StandardColorV1))
			{
				strOutError =
					"Non-StandardColor backend carries a StandardColorV1 packet.";
				return false;
			}
			return true;
		}

		const EFFECT_STANDARD_COLOR_V1_DESC& Packet =
			Execution.StandardColorV1;
		const auto AllZero = [](const auto& Values)
		{
			return std::all_of(Values.begin(), Values.end(),
				[](const uint32_t Value) { return 0u == Value; });
		};
		if (1u != Execution.iVersion || 1u != Execution.iOpcode ||
			1u != Packet.iPacketVersion || 0u == Execution.iTextureLaneCount ||
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
			!Execution.ArtistParameters.empty() || !Execution.Colors.empty() ||
			Packet.eBaseRadianceChannel < EFFECT_STANDARD_COLOR_CHANNEL::R ||
			(Packet.eBaseRadianceChannel != EFFECT_STANDARD_COLOR_CHANNEL::R &&
			 Packet.eBaseRadianceChannel != EFFECT_STANDARD_COLOR_CHANNEL::G &&
			 Packet.eBaseRadianceChannel != EFFECT_STANDARD_COLOR_CHANNEL::B &&
			 Packet.eBaseRadianceChannel != EFFECT_STANDARD_COLOR_CHANNEL::RGB) ||
			Packet.eCoverageChannel < EFFECT_STANDARD_COLOR_CHANNEL::R ||
			Packet.eCoverageChannel > EFFECT_STANDARD_COLOR_CHANNEL::A ||
			Packet.eEmissiveMode >= EFFECT_STANDARD_COLOR_EMISSIVE_MODE::END ||
			Packet.eLifetimeEnvelope !=
				EFFECT_STANDARD_COLOR_LIFETIME_ENVELOPE::CARRIER_ALPHA ||
			Packet.eDissolveMode >= EFFECT_STANDARD_COLOR_DISSOLVE_MODE::END ||
			Packet.eMissingLanePolicy !=
				EFFECT_STANDARD_COLOR_MISSING_LANE_POLICY::FAIL_CLOSED ||
			!std::isfinite(Packet.fDissolveSoftness) ||
			Packet.fDissolveSoftness < 0.f || Packet.fDissolveSoftness > 1.f)
		{
			strOutError = "StandardColorV1 packet identity or hidden state is invalid.";
			return false;
		}

		const auto FindLane = [&Execution](const std::string_view strLaneId,
			size_t& iOutIndex) -> const EFFECT_MATERIAL_TEXTURE_LANE_DESC*
		{
			for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
			{
				if (Execution.TextureLanes[iLane].strLaneId == strLaneId)
				{
					iOutIndex =
						Execution.TextureLanes[iLane].iTextureRegister;
					return &Execution.TextureLanes[iLane];
				}
			}
			return nullptr;
		};
		size_t iBaseLane = 0u;
		size_t iCoverageLane = 0u;
		const EFFECT_MATERIAL_TEXTURE_LANE_DESC* pBaseLane = FindLane(
			Packet.strBaseRadianceLaneId, iBaseLane);
		const EFFECT_MATERIAL_TEXTURE_LANE_DESC* pCoverageLane = FindLane(
			Packet.strCoverageLaneId, iCoverageLane);
		if (nullptr == pBaseLane || nullptr == pCoverageLane ||
			!Is_CanonicalStandardColorSourceChannel(pBaseLane->strSourceChannel) ||
			!Is_CanonicalStandardColorSourceChannel(
				pCoverageLane->strSourceChannel) ||
			!StandardColorLaneContainsChannel(
				*pBaseLane, Packet.eBaseRadianceChannel) ||
			!StandardColorLaneContainsChannel(
				*pCoverageLane, Packet.eCoverageChannel) ||
			(Packet.eCoverageChannel != EFFECT_STANDARD_COLOR_CHANNEL::A &&
			 pCoverageLane->eColorSpace != EFFECT_TEXTURE_COLOR_SPACE::LINEAR))
		{
			strOutError = "StandardColorV1 base-radiance or coverage lane is invalid.";
			return false;
		}

		uint32_t iRequiredMask = (1u << iBaseLane) | (1u << iCoverageLane);
		if (Packet.eDissolveMode == EFFECT_STANDARD_COLOR_DISSOLVE_MODE::NONE)
		{
			if (!Packet.strDissolveLaneId.empty() ||
				Packet.eDissolveChannel != EFFECT_STANDARD_COLOR_CHANNEL::INVALID ||
				0.f != Packet.fDissolveSoftness)
			{
				strOutError = "Disabled StandardColorV1 dissolve carries hidden state.";
				return false;
			}
		}
		else
		{
			size_t iDissolveLane = 0u;
			const EFFECT_MATERIAL_TEXTURE_LANE_DESC* pDissolveLane = FindLane(
				Packet.strDissolveLaneId, iDissolveLane);
			if (nullptr == pDissolveLane ||
				Packet.eDissolveChannel < EFFECT_STANDARD_COLOR_CHANNEL::R ||
				Packet.eDissolveChannel > EFFECT_STANDARD_COLOR_CHANNEL::A ||
				!Is_CanonicalStandardColorSourceChannel(
					pDissolveLane->strSourceChannel) ||
				!StandardColorLaneContainsChannel(
					*pDissolveLane, Packet.eDissolveChannel) ||
				(Packet.eDissolveChannel != EFFECT_STANDARD_COLOR_CHANNEL::A &&
				 pDissolveLane->eColorSpace != EFFECT_TEXTURE_COLOR_SPACE::LINEAR))
			{
				strOutError = "StandardColorV1 dissolve lane is invalid.";
				return false;
			}
			iRequiredMask |= 1u << iDissolveLane;
		}
		if (iRequiredMask != Execution.iTextureMask)
		{
			strOutError =
				"StandardColorV1 texture mask contains an unreferenced or missing lane.";
			return false;
		}
		return true;
	}

}
