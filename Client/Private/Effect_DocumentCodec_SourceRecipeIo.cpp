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


	bool_t Read_SourceCompilerEvidence(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value,
			{ "artifactFileSha256", "artifactSelfSha256", "evidenceId",
				"sourceEvidenceStatus", "sourceCueId", "sourceOccurrenceId",
				"sourceSystemId", "sourceEmitterPath", "sourceEmitterNodeId",
				"lodSelectionPolicy", "selectedLodPath", "selectedLodNodeId",
				"selectedLodArrayIndex", "selectedLodLevelProvenance",
				"selectedLodEnabledProvenance", "nonSelectedLodCount",
				"moduleReferenceOrder", "cueLocalTransform",
				"parameterOverrides", "compositionOrder",
				"localReferenceClosureFileSha256",
				"localReferenceClosureSelfSha256", "geometryParityFileSha256",
				"geometryParitySelfSha256" },
			"Effect source compiler evidence", strOutError))
			return false;

		const Client::DATA_JSON_VALUE* pModules = Find_Field(
			Value, "moduleReferenceOrder", Client::DATA_JSON_TYPE::ARRAY,
			strOutError);
		const Client::DATA_JSON_VALUE* pCueTransform = Find_Field(
			Value, "cueLocalTransform", Client::DATA_JSON_TYPE::OBJECT,
			strOutError);
		const Client::DATA_JSON_VALUE* pParameters = Find_Field(
			Value, "parameterOverrides", Client::DATA_JSON_TYPE::ARRAY,
			strOutError);
		if (nullptr == pModules || nullptr == pCueTransform ||
			nullptr == pParameters ||
			!Read_String(Value, "artifactFileSha256",
				Out.strArtifactFileSha256, strOutError) ||
			!Read_String(Value, "artifactSelfSha256",
				Out.strArtifactSelfSha256, strOutError) ||
			!Read_String(Value, "evidenceId", Out.strEvidenceId, strOutError) ||
			!Read_String(Value, "sourceEvidenceStatus",
				Out.strSourceEvidenceStatus, strOutError) ||
			!Read_String(Value, "sourceCueId", Out.strSourceCueId, strOutError) ||
			!Read_String(Value, "sourceOccurrenceId",
				Out.strSourceOccurrenceId, strOutError) ||
			!Read_String(Value, "sourceSystemId", Out.strSourceSystemId,
				strOutError) ||
			!Read_String(Value, "sourceEmitterPath", Out.strSourceEmitterPath,
				strOutError) ||
			!Read_String(Value, "sourceEmitterNodeId",
				Out.strSourceEmitterNodeId, strOutError) ||
			!Read_String(Value, "lodSelectionPolicy", Out.strLodSelectionPolicy,
				strOutError) ||
			!Read_String(Value, "selectedLodPath", Out.strSelectedLodPath,
				strOutError) ||
			!Read_String(Value, "selectedLodNodeId", Out.strSelectedLodNodeId,
				strOutError) ||
			!Read_UInt(Value, "selectedLodArrayIndex",
				Out.iSelectedLodArrayIndex, strOutError) ||
			!Read_String(Value, "selectedLodLevelProvenance",
				Out.strSelectedLodLevelProvenance, strOutError) ||
			!Read_String(Value, "selectedLodEnabledProvenance",
				Out.strSelectedLodEnabledProvenance, strOutError) ||
			!Read_UInt(Value, "nonSelectedLodCount", Out.iNonSelectedLodCount,
				strOutError) ||
			!Read_StringArray(Value, "compositionOrder", Out.CompositionOrder,
				strOutError) ||
			!Read_String(Value, "localReferenceClosureFileSha256",
				Out.strLocalReferenceClosureFileSha256, strOutError) ||
			!Read_String(Value, "localReferenceClosureSelfSha256",
				Out.strLocalReferenceClosureSelfSha256, strOutError) ||
			!Read_String(Value, "geometryParityFileSha256",
				Out.strGeometryParityFileSha256, strOutError) ||
			!Read_String(Value, "geometryParitySelfSha256",
				Out.strGeometryParitySelfSha256, strOutError) ||
			!Validate_ExactFields(*pCueTransform,
				{ "sourcePositionUeUnits", "position", "rotationDegrees", "scale" },
				"Effect source cue-local transform", strOutError) ||
			!Read_Array(*pCueTransform, "sourcePositionUeUnits",
				&Out.vCueSourcePositionUeUnits.x, 3u, strOutError) ||
			!Read_Array(*pCueTransform, "position",
				&Out.CueLocalTransform.vPosition.x, 3u, strOutError) ||
			!Read_Array(*pCueTransform, "rotationDegrees",
				&Out.CueLocalTransform.vRotationDegrees.x, 3u, strOutError) ||
			!Read_Array(*pCueTransform, "scale",
				&Out.CueLocalTransform.vScale.x, 3u, strOutError))
			return false;

		Out.ModuleReferenceOrder.reserve(pModules->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& ModuleValue : pModules->Get_Array())
		{
			if (!Validate_ExactFields(ModuleValue,
				{ "order", "sourceReferenceIndex", "role", "sourceObjectId",
					"sourceRecordSha256" }, "Effect source module reference",
				strOutError))
				return false;
			Client::EFFECT_SOURCE_MODULE_REFERENCE_DESC Module;
			if (!Read_UInt(ModuleValue, "order", Module.iOrder, strOutError) ||
				!Read_UInt(ModuleValue, "sourceReferenceIndex",
					Module.iSourceReferenceIndex, strOutError) ||
				!Read_String(ModuleValue, "role", Module.strRole, strOutError) ||
				!Read_String(ModuleValue, "sourceObjectId",
					Module.strSourceObjectId, strOutError) ||
				!Read_String(ModuleValue, "sourceRecordSha256",
					Module.strSourceRecordSha256, strOutError))
				return false;
			Out.ModuleReferenceOrder.push_back(std::move(Module));
		}

		Out.ParameterOverrides.reserve(pParameters->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& ParameterValue :
			pParameters->Get_Array())
		{
			if (!Validate_ExactFields(ParameterValue,
				{ "sourceIndex", "name", "sourceTypeCode",
					"sourceRecordByteOffset", "type", "scalarValue", "vectorValue",
					"sourceValueByteOffset" }, "Effect source parameter override",
				strOutError))
				return false;
			Client::EFFECT_SOURCE_PARAMETER_OVERRIDE_DESC Parameter;
			if (!Read_UInt(ParameterValue, "sourceIndex", Parameter.iSourceIndex,
					strOutError) ||
				!Read_String(ParameterValue, "name", Parameter.strName,
					strOutError) ||
				!Read_UInt(ParameterValue, "sourceTypeCode",
					Parameter.iSourceTypeCode, strOutError) ||
				!Read_UInt(ParameterValue, "sourceRecordByteOffset",
					Parameter.iSourceRecordByteOffset, strOutError) ||
				!Read_String(ParameterValue, "type", Parameter.strType,
					strOutError) ||
				!Read_UInt(ParameterValue, "sourceValueByteOffset",
					Parameter.iSourceValueByteOffset, strOutError))
				return false;
			if (Parameter.strType == "scalar")
			{
				if (nullptr != ParameterValue.Find("vectorValue") ||
					!Read_Double(ParameterValue, "scalarValue",
						Parameter.fScalarValue, strOutError))
					return false;
			}
			else if (Parameter.strType == "vector")
			{
				if (nullptr != ParameterValue.Find("scalarValue") ||
					!Read_Array(ParameterValue, "vectorValue",
						&Parameter.vVectorValue.x, 3u, strOutError))
					return false;
			}
			else
			{
				strOutError = "Effect source parameter override type is invalid.";
				return false;
			}
			Out.ParameterOverrides.push_back(std::move(Parameter));
		}
		return true;
	}


	bool_t Read_SourceAdmission(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_ADMISSION_DESC& Out,
		std::string& strOutError)
	{
		return Validate_ExactFields(Value, { "allowed", "blockers" },
			"Effect source execution admission", strOutError) &&
			Read_Bool(Value, "allowed", Out.bAllowed, strOutError) &&
			Read_StringArray(Value, "blockers", Out.Blockers, strOutError);
	}


	bool_t Read_SourceTypedField(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_TYPED_FIELD_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value, { "propertyPath", "kind", "value" },
			"Effect source typed evidence field", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pKind = Find_Field(
			Value, "kind", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pValue = Value.Is_Object() ?
			Value.Find("value") : nullptr;
		if (nullptr == pKind || nullptr == pValue ||
			!Read_String(Value, "propertyPath", Out.strPropertyPath,
				strOutError) ||
			!Parse_Token(pKind->Get_String(), SOURCE_TYPED_FIELD_KIND_TOKENS,
				std::size(SOURCE_TYPED_FIELD_KIND_TOKENS), Out.eKind))
		{
			if (strOutError.empty())
				strOutError = "Effect source typed evidence kind is invalid.";
			return false;
		}
		switch (Out.eKind)
		{
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::BOOLEAN:
			if (!pValue->Is_Boolean())
			{
				strOutError = "Effect source typed Boolean evidence is invalid.";
				return false;
			}
			Out.bBoolean = pValue->Get_Boolean();
			return true;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::NUMBER:
			if (!pValue->Is_Number() || !std::isfinite(pValue->Get_Number()))
			{
				strOutError = "Effect source typed number evidence is invalid.";
				return false;
			}
			Out.fNumber = pValue->Get_Number();
			return true;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::STRING:
			if (!pValue->Is_String())
			{
				strOutError = "Effect source typed string evidence is invalid.";
				return false;
			}
			Out.strString = pValue->Get_String();
			return true;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::VECTOR:
			return Read_Array(Value, "value", &Out.vVector.x, 4u, strOutError);
		default:
			strOutError = "Effect source typed evidence kind is invalid.";
			return false;
		}
	}


	bool_t Read_SourceLocalReferenceBinding(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value,
			{ "referenceKind", "referenceId", "definitionId", "occurrenceId",
				"moduleStableId", "propertyPath", "provenance", "exactPayload",
				"currentDefaultEvidence", "executionAdmission" },
			"Effect source local-reference binding", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pExactPayload = Find_Field(
			Value, "exactPayload", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pCurrentDefaultEvidence = Find_Field(
			Value, "currentDefaultEvidence", Client::DATA_JSON_TYPE::ARRAY,
			strOutError);
		const Client::DATA_JSON_VALUE* pExecutionAdmission = Find_Field(
			Value, "executionAdmission", Client::DATA_JSON_TYPE::OBJECT,
			strOutError);
		if (nullptr == pExactPayload || nullptr == pCurrentDefaultEvidence ||
			nullptr == pExecutionAdmission ||
			!Read_String(Value, "referenceKind", Out.strReferenceKind,
				strOutError) ||
			!Read_String(Value, "referenceId", Out.strReferenceId, strOutError) ||
			!Read_String(Value, "definitionId", Out.strDefinitionId, strOutError) ||
			!Read_String(Value, "occurrenceId", Out.strOccurrenceId, strOutError) ||
			!Read_String(Value, "moduleStableId", Out.strModuleStableId,
				strOutError) ||
			!Read_String(Value, "propertyPath", Out.strPropertyPath,
				strOutError) ||
			!Read_String(Value, "provenance", Out.strProvenance, strOutError) ||
			!Read_SourceAdmission(*pExecutionAdmission,
				Out.ExecutionAdmission, strOutError))
		{
			return false;
		}
		Out.ExactPayload.reserve(pExactPayload->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& FieldValue :
			pExactPayload->Get_Array())
		{
			Client::EFFECT_SOURCE_TYPED_FIELD_DESC Field;
			if (!Read_SourceTypedField(FieldValue, Field, strOutError))
				return false;
			Out.ExactPayload.push_back(std::move(Field));
		}
		Out.CurrentDefaultEvidence.reserve(
			pCurrentDefaultEvidence->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& FieldValue :
			pCurrentDefaultEvidence->Get_Array())
		{
			Client::EFFECT_SOURCE_TYPED_FIELD_DESC Field;
			if (!Read_SourceTypedField(FieldValue, Field, strOutError))
				return false;
			Out.CurrentDefaultEvidence.push_back(std::move(Field));
		}
		return true;
	}


	bool_t Read_SourceMaterialAdmission(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_MATERIAL_ADMISSION_DESC& Out,
		std::string& strOutError)
	{
		return Validate_ExactFields(Value,
			{ "status", "sourceMaterialPaths", "materialRecipeId",
				"renderStateRecipeId", "blockers" },
			"Effect source material admission", strOutError) &&
			Read_String(Value, "status", Out.strStatus, strOutError) &&
			Read_StringArray(Value, "sourceMaterialPaths",
				Out.SourceMaterialPaths, strOutError) &&
			Read_String(Value, "materialRecipeId", Out.strMaterialRecipeId,
				strOutError) &&
			Read_String(Value, "renderStateRecipeId", Out.strRenderStateRecipeId,
				strOutError) &&
			Read_StringArray(Value, "blockers", Out.Blockers, strOutError);
	}


	bool_t Read_SourceGeometryBinding(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Out,
		std::string& strOutError)
	{
		return Validate_ExactFields(Value,
			{ "enabled", "assetId", "receiptFileSha256", "receiptSelfSha256",
				"carrierGeometryPreScale", "particleScaleSemantics", "status",
				"blockers" }, "Effect source geometry binding", strOutError) &&
			Read_Bool(Value, "enabled", Out.bEnabled, strOutError) &&
			Read_String(Value, "assetId", Out.strAssetId, strOutError) &&
			Read_String(Value, "receiptFileSha256", Out.strReceiptFileSha256,
				strOutError) &&
			Read_String(Value, "receiptSelfSha256", Out.strReceiptSelfSha256,
				strOutError) &&
			Read_Float(Value, "carrierGeometryPreScale",
				Out.fCarrierGeometryPreScale, strOutError) &&
			Read_String(Value, "particleScaleSemantics",
				Out.strParticleScaleSemantics, strOutError) &&
			Read_String(Value, "status", Out.strStatus, strOutError) &&
			Read_StringArray(Value, "blockers", Out.Blockers, strOutError);
	}


    constexpr const char_t* SOURCE_TRANSFORM_FRAME_TOKENS[] =
        { "WORLD", "RELATIVE_TO_INITIAL", "PARENT" };


    bool_t Validate_SourceTransformTrack(
        const Client::EFFECT_SOURCE_TRANSFORM_TRACK& Track, std::string& Error)
    {
        using namespace Client;
        if (Track.strSourceOccurrenceId.empty() || Track.strSourceOccurrenceId.size() > 256u ||
            !std::isfinite(Track.fSourceTimeOriginSeconds) || !Is_Finite(Track.vPreviewOriginUE3Cm) ||
            Track.Nodes.empty() || Track.Nodes.size() > 16u)
        { Error = "Effect source transform track identity, clock or nodes are invalid."; return false; }
        if (Track.AlphaScale && (Track.AlphaScale->iComponentCount != 3u ||
            Track.AlphaScale->iOperation != 1u || !CEffectDistribution::Validate(*Track.AlphaScale, Error)))
            return false;
        for (size_t i = 0u; i < Track.Nodes.size(); ++i)
        {
            const auto& Node = Track.Nodes[i];
            if (Node.strSourceObjectPath.empty() || Node.strSourceObjectPath.size() > 512u ||
                Node.eFrame >= EFFECT_SOURCE_TRANSFORM_FRAME::END ||
                (i == 0u && Node.eFrame == EFFECT_SOURCE_TRANSFORM_FRAME::PARENT) ||
                !Is_Finite(Node.vInitialPositionUE3Cm) || !Is_Finite(Node.vInitialEulerDegrees) ||
                !Is_Finite(Node.vScaleUE3) || Node.vScaleUE3.x == 0.f || Node.vScaleUE3.y == 0.f || Node.vScaleUE3.z == 0.f ||
                Node.Position.iOperation != 1u || Node.Euler.iOperation != 1u ||
                Node.Position.iComponentCount != 3u || Node.Euler.iComponentCount != 3u ||
                !CEffectDistribution::Validate(Node.Position, Error) || !CEffectDistribution::Validate(Node.Euler, Error))
            { if (Error.empty()) Error = "Effect source transform node or curve is invalid."; return false; }
        }
        return true;
    }


    bool_t Read_SourceTransformCurve(const Client::DATA_JSON_VALUE& Value,
        Client::EFFECT_DISTRIBUTION_DESC& Out, const char_t* Property, std::string& Error)
    {
        using namespace Client;
        if (!Value.Is_Array() || Value.Get_Array().size() > 4096u)
        { Error = "Effect source transform curve must be a bounded array."; return false; }
        Out.strPropertyPath = Property;
        Out.strSourceClass = "interptrackmove";
        Out.iComponentCount = 3u;
        Out.iOperation = 1u;
        for (const DATA_JSON_VALUE& ValueKey : Value.Get_Array())
        {
            EFFECT_DISTRIBUTION_KEY_DESC Key;
            std::string Interpolation;
            if (!ValueKey.Is_Object() || !Validate_ExactFields(ValueKey,
                { "timeSeconds", "value", "arriveTangent", "leaveTangent", "interpolation" },
                "Effect source transform curve key", Error) ||
                !Read_Float(ValueKey, "timeSeconds", Key.fTime, Error) ||
                !Read_Array(ValueKey, "value", &Key.vMinimum.x, 3u, Error) ||
                !Read_Array(ValueKey, "arriveTangent", &Key.vArriveTangentMinimum.x, 3u, Error) ||
                !Read_Array(ValueKey, "leaveTangent", &Key.vLeaveTangentMinimum.x, 3u, Error) ||
                !Read_String(ValueKey, "interpolation", Interpolation, Error) ||
                !Parse_Token(Interpolation, DISTRIBUTION_INTERPOLATION_TOKENS,
                    std::size(DISTRIBUTION_INTERPOLATION_TOKENS), Key.eInterpolation))
            { if (Error.empty()) Error = "Effect source transform key is invalid."; return false; }
            Key.vMaximum = Key.vMinimum;
            Key.vArriveTangentMaximum = Key.vArriveTangentMinimum;
            Key.vLeaveTangentMaximum = Key.vLeaveTangentMinimum;
            Out.Keys.push_back(Key);
        }
        return true;
    }


    bool_t Read_SourceTransformTrack(const Client::DATA_JSON_VALUE& Value,
        Client::EFFECT_SOURCE_TRANSFORM_TRACK& Out, std::string& Error)
    {
        using namespace Client;
        if (!Value.Is_Object() || !Validate_ExactFields(Value,
            { "sourceOccurrenceId", "sourceTimeOriginSeconds", "previewOriginUE3Cm", "nodes", "alphaScaleKeys" },
            "Effect source transform track", Error)) return false;
        const auto* Nodes = Find_Field(Value, "nodes", DATA_JSON_TYPE::ARRAY, Error);
        if (!Nodes || !Read_String(Value, "sourceOccurrenceId", Out.strSourceOccurrenceId, Error) ||
            !Read_Float(Value, "sourceTimeOriginSeconds", Out.fSourceTimeOriginSeconds, Error) ||
            !Read_Array(Value, "previewOriginUE3Cm", &Out.vPreviewOriginUE3Cm.x, 3u, Error)) return false;
        for (const auto& V : Nodes->Get_Array())
        {
            EFFECT_SOURCE_TRANSFORM_NODE N;
            std::string Frame;
            if (!V.Is_Object() || !Validate_ExactFields(V,
                { "sourceObjectPath", "frame", "initialPositionUE3Cm", "initialEulerDegrees", "scaleUE3", "positionKeys", "eulerKeys" },
                "Effect source transform node", Error)) return false;
            const auto* Position = Find_Field(V, "positionKeys", DATA_JSON_TYPE::ARRAY, Error);
            const auto* Euler = Find_Field(V, "eulerKeys", DATA_JSON_TYPE::ARRAY, Error);
            if (!Position || !Euler || !Read_String(V, "sourceObjectPath", N.strSourceObjectPath, Error) ||
                !Read_String(V, "frame", Frame, Error) ||
                !Parse_Token(Frame, SOURCE_TRANSFORM_FRAME_TOKENS, std::size(SOURCE_TRANSFORM_FRAME_TOKENS), N.eFrame) ||
                !Read_Array(V, "initialPositionUE3Cm", &N.vInitialPositionUE3Cm.x, 3u, Error) ||
                !Read_Array(V, "initialEulerDegrees", &N.vInitialEulerDegrees.x, 3u, Error) ||
                !Read_Array(V, "scaleUE3", &N.vScaleUE3.x, 3u, Error) ||
                !Read_SourceTransformCurve(*Position, N.Position, "position", Error) ||
                !Read_SourceTransformCurve(*Euler, N.Euler, "euler", Error)) return false;
            Out.Nodes.push_back(std::move(N));
        }
        if (const auto* Alpha = Value.Find("alphaScaleKeys"))
        {
            EFFECT_DISTRIBUTION_DESC Curve;
            if (!Read_SourceTransformCurve(*Alpha, Curve, "alphaScale", Error)) return false;
            Out.AlphaScale = std::move(Curve);
        }
        return Validate_SourceTransformTrack(Out, Error);
    }


    void Write_SourceTransformCurve(std::ostringstream& Output,
        const Client::EFFECT_DISTRIBUTION_DESC& Curve)
    {
        Output << '[';
        for (size_t i = 0; i < Curve.Keys.size(); ++i)
        {
            const auto& K = Curve.Keys[i];
            Output << (i ? ", " : "") << "{ \"timeSeconds\": " << K.fTime << ", \"value\": ";
            Write_Float3(Output, {K.vMinimum.x,K.vMinimum.y,K.vMinimum.z});
            Output << ", \"arriveTangent\": ";
            Write_Float3(Output, {K.vArriveTangentMinimum.x,K.vArriveTangentMinimum.y,K.vArriveTangentMinimum.z});
            Output << ", \"leaveTangent\": ";
            Write_Float3(Output, {K.vLeaveTangentMinimum.x,K.vLeaveTangentMinimum.y,K.vLeaveTangentMinimum.z});
            Output << ", \"interpolation\": \"" << DISTRIBUTION_INTERPOLATION_TOKENS[static_cast<size_t>(K.eInterpolation)] << "\" }";
        }
        Output << ']';
    }


    void Write_SourceTransformTrack(std::ostringstream& Output,
        const Client::EFFECT_SOURCE_TRANSFORM_TRACK& Track)
    {
        Output << "      \"sourceTransformTrack\": { \"sourceOccurrenceId\": \""
            << Client::CDataJson::Escape(Track.strSourceOccurrenceId)
            << "\", \"sourceTimeOriginSeconds\": " << Track.fSourceTimeOriginSeconds << ", \"previewOriginUE3Cm\": ";
        Write_Float3(Output, Track.vPreviewOriginUE3Cm);
        Output << ", \"nodes\": [";
        for (size_t i=0; i < Track.Nodes.size(); ++i)
        {
            const auto& N = Track.Nodes[i];
            Output << (i ? ", " : "") << "{ \"sourceObjectPath\": \"" << Client::CDataJson::Escape(N.strSourceObjectPath)
                << "\", \"frame\": \"" << SOURCE_TRANSFORM_FRAME_TOKENS[static_cast<size_t>(N.eFrame)]
                << "\", \"initialPositionUE3Cm\": ";
            Write_Float3(Output, N.vInitialPositionUE3Cm);
            Output << ", \"initialEulerDegrees\": "; Write_Float3(Output,N.vInitialEulerDegrees);
            Output << ", \"scaleUE3\": "; Write_Float3(Output,N.vScaleUE3);
            Output << ", \"positionKeys\": "; Write_SourceTransformCurve(Output,N.Position);
            Output << ", \"eulerKeys\": "; Write_SourceTransformCurve(Output,N.Euler);
            Output << " }";
        }
        Output << ']';
        if (Track.AlphaScale)
        {
            Output << ", \"alphaScaleKeys\": ";
            Write_SourceTransformCurve(Output,*Track.AlphaScale);
        }
        Output << " },\n";
    }


	bool_t Read_SourceRecipe(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_CASCADE_RECIPE_DESC& Out,
		const bool_t bSourceContract,
		std::string& strOutError)
	{
		if (!bSourceContract)
		{
			constexpr const char_t* SourceOnlyFields[] = {
				"sourceContractProfileId", "sourceContractSha256",
				"sourceGraphSha256", "sourceClosureSha256",
				"sourceMaterialClosureSha256", "sourcePeakActiveParticles",
				"localReferenceBindings", "moduleCoverage", "compilerEvidence",
				"compiledExecutionAdmission", "materialAdmission",
				"geometryBinding"
			};
			for (const char_t* pField : SourceOnlyFields)
			{
				if (nullptr != Value.Find(pField))
				{
					strOutError =
						"Legacy Effect sourceRecipe contains native-v14 evidence.";
					return false;
				}
			}
		}
		if (bSourceContract && !Validate_ExactFields(Value,
			{ "enabled", "rendererShape", "sourceContractProfileId",
				"sourceContractSha256", "sourceGraphSha256",
				"sourceClosureSha256", "sourceMaterialClosureSha256",
				"sourcePeakActiveParticles", "emitterDelaySeconds",
				"emitterDurationSeconds", "emitterLoopCount", "bursts",
				"modules", "localReferenceBindings", "moduleCoverage",
				"compilerEvidence",
				"compiledExecutionAdmission", "materialAdmission",
				"geometryBinding" },
			"Effect source recipe", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pBursts = Find_Field(
			Value, "bursts", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pModules = Find_Field(
			Value, "modules", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pCoverage = bSourceContract ?
			Find_Field(Value, "moduleCoverage", Client::DATA_JSON_TYPE::ARRAY,
				strOutError) : nullptr;
		const Client::DATA_JSON_VALUE* pLocalReferenceBindings = bSourceContract ?
			Find_Field(Value, "localReferenceBindings",
				Client::DATA_JSON_TYPE::ARRAY, strOutError) : nullptr;
		const Client::DATA_JSON_VALUE* pCompilerEvidence = bSourceContract ?
			Find_Field(Value, "compilerEvidence", Client::DATA_JSON_TYPE::OBJECT,
				strOutError) : nullptr;
		const Client::DATA_JSON_VALUE* pExecutionAdmission = bSourceContract ?
			Find_Field(Value, "compiledExecutionAdmission",
				Client::DATA_JSON_TYPE::OBJECT, strOutError) : nullptr;
		const Client::DATA_JSON_VALUE* pMaterialAdmission = bSourceContract ?
			Find_Field(Value, "materialAdmission", Client::DATA_JSON_TYPE::OBJECT,
				strOutError) : nullptr;
		const Client::DATA_JSON_VALUE* pGeometryBinding = bSourceContract ?
			Find_Field(Value, "geometryBinding", Client::DATA_JSON_TYPE::OBJECT,
				strOutError) : nullptr;
		if (nullptr == pBursts || nullptr == pModules ||
			(bSourceContract && (nullptr == pLocalReferenceBindings ||
				nullptr == pCoverage ||
				nullptr == pCompilerEvidence || nullptr == pExecutionAdmission ||
				nullptr == pMaterialAdmission || nullptr == pGeometryBinding)) ||
			!Read_Bool(Value, "enabled", Out.bEnabled, strOutError) ||
			!Read_OptionalBool(Value, "authoredModuleOverrides",
				Out.bAuthoredModuleOverrides, strOutError) ||
			!Read_OptionalBool(Value, "simulationOnly", Out.bSimulationOnly, strOutError) ||
			(nullptr != Value.Find("particleSystemOccurrenceId") &&
			 !Read_String(Value, "particleSystemOccurrenceId", Out.strParticleSystemOccurrenceId, strOutError)) ||
			(nullptr != Value.Find("emitterName") &&
			 !Read_String(Value, "emitterName", Out.strEmitterName, strOutError)) ||
			!Read_String(Value, "rendererShape", Out.strRendererShape,
				strOutError) ||
			!Read_Float(Value, "emitterDelaySeconds",
				Out.fEmitterDelaySeconds, strOutError) ||
			!Read_Float(Value, "emitterDurationSeconds",
				Out.fEmitterDurationSeconds, strOutError) ||
			!Read_UInt(Value, "emitterLoopCount", Out.iEmitterLoopCount,
				strOutError) ||
			(bSourceContract &&
				(!Read_String(Value, "sourceContractProfileId",
					Out.strSourceContractProfileId, strOutError) ||
				 !Read_String(Value, "sourceContractSha256",
					Out.strSourceContractSha256, strOutError) ||
				 !Read_String(Value, "sourceGraphSha256",
					Out.strSourceGraphSha256, strOutError) ||
				 !Read_String(Value, "sourceClosureSha256",
					Out.strSourceClosureSha256, strOutError) ||
				 !Read_String(Value, "sourceMaterialClosureSha256",
					Out.strSourceMaterialClosureSha256, strOutError) ||
				 !Read_UInt(Value, "sourcePeakActiveParticles",
					Out.iSourcePeakActiveParticles, strOutError) ||
				 !Read_SourceCompilerEvidence(*pCompilerEvidence,
					Out.CompilerEvidence, strOutError) ||
				 !Read_SourceAdmission(*pExecutionAdmission,
					Out.CompiledExecutionAdmission, strOutError) ||
				 !Read_SourceMaterialAdmission(*pMaterialAdmission,
					Out.MaterialAdmission, strOutError) ||
				 !Read_SourceGeometryBinding(*pGeometryBinding,
					Out.GeometryBinding, strOutError))))
		{
			return false;
		}
		Out.Bursts.reserve(pBursts->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& BurstValue : pBursts->Get_Array())
		{
			if (bSourceContract && !Validate_ExactFields(BurstValue,
				{ "timeSeconds", "countMinimum", "countMaximum" },
				"Effect source burst", strOutError))
				return false;
			Client::EFFECT_PARTICLE_BURST_DESC Burst;
			if (!BurstValue.Is_Object() ||
				!Read_Float(BurstValue, "timeSeconds", Burst.fTimeSeconds,
					strOutError) ||
				!Read_UInt(BurstValue, "countMinimum", Burst.iCountMinimum,
					strOutError) ||
				!Read_UInt(BurstValue, "countMaximum", Burst.iCountMaximum,
					strOutError))
			{
				return false;
			}
			Out.Bursts.push_back(Burst);
		}
		Out.Modules.reserve(pModules->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& ModuleValue : pModules->Get_Array())
		{
			if (bSourceContract && !Validate_ExactFields(ModuleValue,
				{ "stableId", "className", "objectPath", "literals",
					"distributions" }, "Effect source module", strOutError))
				return false;
			const Client::DATA_JSON_VALUE* pLiterals = ModuleValue.Is_Object() ?
				ModuleValue.Find("literals") : nullptr;
			const Client::DATA_JSON_VALUE* pDistributions =
				ModuleValue.Is_Object() ? ModuleValue.Find("distributions") :
				nullptr;
			Client::EFFECT_SOURCE_MODULE_DESC Module;
			if (nullptr == pLiterals || !pLiterals->Is_Array() ||
				nullptr == pDistributions || !pDistributions->Is_Array() ||
				!Read_String(ModuleValue, "stableId", Module.strStableId,
					strOutError) ||
				!Read_String(ModuleValue, "className", Module.strClassName,
					strOutError) ||
				!Read_String(ModuleValue, "objectPath", Module.strObjectPath,
					strOutError))
			{
				return false;
			}
			for (const Client::DATA_JSON_VALUE& LiteralValue :
				pLiterals->Get_Array())
			{
				if (bSourceContract && !Validate_ExactFields(LiteralValue,
					{ "propertyPath", "kind", "value" },
					"Effect source literal", strOutError))
					return false;
				const Client::DATA_JSON_VALUE* pKind = LiteralValue.Is_Object() ?
					LiteralValue.Find("kind") : nullptr;
				Client::EFFECT_SOURCE_LITERAL_DESC Literal;
				if (nullptr == pKind || !pKind->Is_String() ||
					!Read_String(LiteralValue, "propertyPath",
						Literal.strPropertyPath, strOutError) ||
					!Parse_Token(pKind->Get_String(),
						SOURCE_LITERAL_KIND_TOKENS,
						std::size(SOURCE_LITERAL_KIND_TOKENS), Literal.eKind))
				{
					return false;
				}
				const Client::DATA_JSON_VALUE* pLiteralValue =
					LiteralValue.Find("value");
				if (nullptr == pLiteralValue ||
					(Client::EFFECT_SOURCE_LITERAL_KIND::BOOLEAN ==
						Literal.eKind && !pLiteralValue->Is_Boolean()) ||
					(Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER ==
						Literal.eKind && !pLiteralValue->Is_Number()) ||
					(Client::EFFECT_SOURCE_LITERAL_KIND::STRING ==
						Literal.eKind && !pLiteralValue->Is_String()))
				{
					strOutError = "Effect source literal value is invalid.";
					return false;
				}
				if (Client::EFFECT_SOURCE_LITERAL_KIND::BOOLEAN == Literal.eKind)
					Literal.bBoolean = pLiteralValue->Get_Boolean();
				else if (Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER == Literal.eKind)
					Literal.fNumber = pLiteralValue->Get_Number();
				else
					Literal.strString = pLiteralValue->Get_String();
				Module.Literals.push_back(std::move(Literal));
			}
			for (const Client::DATA_JSON_VALUE& DistributionValue :
				pDistributions->Get_Array())
			{
				Client::EFFECT_DISTRIBUTION_DESC Distribution;
				if (!DistributionValue.Is_Object() ||
					!Read_Distribution(DistributionValue, Distribution,
						bSourceContract,
						strOutError))
				{
					return false;
				}
				Module.Distributions.push_back(std::move(Distribution));
			}
			Out.Modules.push_back(std::move(Module));
		}
		if (bSourceContract)
		{
			Out.LocalReferenceBindings.reserve(
				pLocalReferenceBindings->Get_Array().size());
			for (const Client::DATA_JSON_VALUE& BindingValue :
				pLocalReferenceBindings->Get_Array())
			{
				Client::EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC Binding;
				if (!Read_SourceLocalReferenceBinding(
					BindingValue, Binding, strOutError))
				{
					return false;
				}
				Out.LocalReferenceBindings.push_back(std::move(Binding));
			}
			Out.ModuleCoverage.reserve(pCoverage->Get_Array().size());
			for (const Client::DATA_JSON_VALUE& CoverageValue :
				pCoverage->Get_Array())
			{
				if (!Validate_ExactFields(CoverageValue,
					{ "moduleStableId", "exactSourceClass", "aliasId",
						"normalizedClass", "status", "blockers", "properties" },
					"Effect source module coverage",
					strOutError))
					return false;
				const Client::DATA_JSON_VALUE* pExactSourceClass =
					CoverageValue.Find("exactSourceClass");
				const Client::DATA_JSON_VALUE* pAliasId =
					CoverageValue.Find("aliasId");
				if ((nullptr == pExactSourceClass) != (nullptr == pAliasId))
				{
					strOutError = "Effect source module class lineage is incomplete.";
					return false;
				}
				const Client::DATA_JSON_VALUE* pStatus = Find_Field(
					CoverageValue, "status", Client::DATA_JSON_TYPE::STRING,
					strOutError);
				const Client::DATA_JSON_VALUE* pProperties = Find_Field(
					CoverageValue, "properties", Client::DATA_JSON_TYPE::ARRAY,
					strOutError);
				Client::EFFECT_SOURCE_MODULE_COVERAGE_DESC Coverage;
				if (nullptr == pStatus || nullptr == pProperties ||
					!Read_String(CoverageValue, "moduleStableId",
						Coverage.strModuleStableId, strOutError) ||
					(nullptr != pExactSourceClass &&
						(!Read_String(CoverageValue, "exactSourceClass",
							Coverage.strExactSourceClass, strOutError) ||
						 !Read_String(CoverageValue, "aliasId",
							Coverage.strAliasId, strOutError))) ||
					!Read_String(CoverageValue, "normalizedClass",
						Coverage.strNormalizedClass, strOutError) ||
					!Read_StringArray(CoverageValue, "blockers", Coverage.Blockers,
						strOutError) ||
					!Parse_Token(pStatus->Get_String(),
						SOURCE_COVERAGE_STATUS_TOKENS,
						std::size(SOURCE_COVERAGE_STATUS_TOKENS),
						Coverage.eStatus))
					return false;
				if (nullptr != pExactSourceClass &&
					Coverage.strExactSourceClass.empty())
				{
					strOutError = "Effect source exact module class is empty.";
					return false;
				}
				for (const Client::DATA_JSON_VALUE& PropertyValue :
					pProperties->Get_Array())
				{
					if (!Validate_ExactFields(PropertyValue,
						{ "propertyPath", "storage", "status", "provenance",
							"blockers" },
						"Effect source property coverage", strOutError))
						return false;
					const Client::DATA_JSON_VALUE* pPropertyStatus = Find_Field(
						PropertyValue, "status", Client::DATA_JSON_TYPE::STRING,
						strOutError);
					Client::EFFECT_SOURCE_PROPERTY_COVERAGE_DESC Property;
					if (nullptr == pPropertyStatus ||
						!Read_String(PropertyValue, "propertyPath",
							Property.strPropertyPath, strOutError) ||
						!Read_String(PropertyValue, "storage",
							Property.strStorage, strOutError) ||
						!Read_String(PropertyValue, "provenance",
							Property.strProvenance, strOutError) ||
						!Read_StringArray(PropertyValue, "blockers",
							Property.Blockers, strOutError) ||
						!Parse_Token(pPropertyStatus->Get_String(),
							SOURCE_COVERAGE_STATUS_TOKENS,
							std::size(SOURCE_COVERAGE_STATUS_TOKENS),
							Property.eStatus))
						return false;
					Coverage.Properties.push_back(std::move(Property));
				}
				Out.ModuleCoverage.push_back(std::move(Coverage));
			}
		}
		return true;
	}


	void Write_SourceCompilerEvidence(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& Evidence)
	{
		Output << "{ \"artifactFileSha256\": \"" << Evidence.strArtifactFileSha256
			<< "\", \"artifactSelfSha256\": \"" << Evidence.strArtifactSelfSha256
			<< "\", \"evidenceId\": \""
			<< Client::CDataJson::Escape(Evidence.strEvidenceId)
			<< "\", \"sourceEvidenceStatus\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceEvidenceStatus)
			<< "\", \"sourceCueId\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceCueId)
			<< "\", \"sourceOccurrenceId\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceOccurrenceId)
			<< "\", \"sourceSystemId\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceSystemId)
			<< "\", \"sourceEmitterPath\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceEmitterPath)
			<< "\", \"sourceEmitterNodeId\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceEmitterNodeId)
			<< "\", \"lodSelectionPolicy\": \""
			<< Client::CDataJson::Escape(Evidence.strLodSelectionPolicy)
			<< "\", \"selectedLodPath\": \""
			<< Client::CDataJson::Escape(Evidence.strSelectedLodPath)
			<< "\", \"selectedLodNodeId\": \""
			<< Client::CDataJson::Escape(Evidence.strSelectedLodNodeId)
			<< "\", \"selectedLodArrayIndex\": "
			<< Evidence.iSelectedLodArrayIndex
			<< ", \"selectedLodLevelProvenance\": \""
			<< Client::CDataJson::Escape(Evidence.strSelectedLodLevelProvenance)
			<< "\", \"selectedLodEnabledProvenance\": \""
			<< Client::CDataJson::Escape(Evidence.strSelectedLodEnabledProvenance)
			<< "\", \"nonSelectedLodCount\": "
			<< Evidence.iNonSelectedLodCount << ", \"moduleReferenceOrder\": [";
		for (size_t iModule = 0u;
			iModule < Evidence.ModuleReferenceOrder.size(); ++iModule)
		{
			const Client::EFFECT_SOURCE_MODULE_REFERENCE_DESC& Module =
				Evidence.ModuleReferenceOrder[iModule];
			if (iModule > 0u)
				Output << ", ";
			Output << "{ \"order\": " << Module.iOrder
				<< ", \"sourceReferenceIndex\": "
				<< Module.iSourceReferenceIndex << ", \"role\": \""
				<< Client::CDataJson::Escape(Module.strRole)
				<< "\", \"sourceObjectId\": \""
				<< Client::CDataJson::Escape(Module.strSourceObjectId)
				<< "\", \"sourceRecordSha256\": \""
				<< Module.strSourceRecordSha256 << "\" }";
		}
		Output << "], \"cueLocalTransform\": { \"sourcePositionUeUnits\": ";
		Write_Float3(Output, Evidence.vCueSourcePositionUeUnits);
		Output << ", \"position\": ";
		Write_Float3(Output, Evidence.CueLocalTransform.vPosition);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output, Evidence.CueLocalTransform.vRotationDegrees);
		Output << ", \"scale\": ";
		Write_Float3(Output, Evidence.CueLocalTransform.vScale);
		Output << " }, \"parameterOverrides\": [";
		for (size_t iParameter = 0u;
			iParameter < Evidence.ParameterOverrides.size(); ++iParameter)
		{
			const Client::EFFECT_SOURCE_PARAMETER_OVERRIDE_DESC& Parameter =
				Evidence.ParameterOverrides[iParameter];
			if (iParameter > 0u)
				Output << ", ";
			Output << "{ \"sourceIndex\": " << Parameter.iSourceIndex
				<< ", \"name\": \""
				<< Client::CDataJson::Escape(Parameter.strName)
				<< "\", \"sourceTypeCode\": " << Parameter.iSourceTypeCode
				<< ", \"sourceRecordByteOffset\": "
				<< Parameter.iSourceRecordByteOffset << ", \"type\": \""
				<< Parameter.strType << "\", ";
			if (Parameter.strType == "scalar")
			{
				Output << "\"scalarValue\": "
					<< std::setprecision(std::numeric_limits<f64_t>::max_digits10)
					<< Parameter.fScalarValue << std::setprecision(9);
			}
			else
			{
				Output << "\"vectorValue\": ";
				Write_Float3(Output, Parameter.vVectorValue);
			}
			Output << ", \"sourceValueByteOffset\": "
				<< Parameter.iSourceValueByteOffset << " }";
		}
		Output << "], \"compositionOrder\": ";
		Write_StringArray(Output, Evidence.CompositionOrder);
		Output << ", \"localReferenceClosureFileSha256\": \""
			<< Evidence.strLocalReferenceClosureFileSha256
			<< "\", \"localReferenceClosureSelfSha256\": \""
			<< Evidence.strLocalReferenceClosureSelfSha256
			<< "\", \"geometryParityFileSha256\": \""
			<< Evidence.strGeometryParityFileSha256
			<< "\", \"geometryParitySelfSha256\": \""
			<< Evidence.strGeometryParitySelfSha256 << "\" }";
	}


	void Write_SourceAdmission(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_ADMISSION_DESC& Admission)
	{
		Output << "{ \"allowed\": " << (Admission.bAllowed ? "true" : "false")
			<< ", \"blockers\": ";
		Write_StringArray(Output, Admission.Blockers);
		Output << " }";
	}


	void Write_SourceTypedField(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_TYPED_FIELD_DESC& Field)
	{
		Output << "{ \"propertyPath\": \""
			<< Client::CDataJson::Escape(Field.strPropertyPath)
			<< "\", \"kind\": \""
			<< SOURCE_TYPED_FIELD_KIND_TOKENS[static_cast<size_t>(Field.eKind)]
			<< "\", \"value\": ";
		switch (Field.eKind)
		{
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::BOOLEAN:
			Output << (Field.bBoolean ? "true" : "false");
			break;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::NUMBER:
			Output << std::setprecision(
				std::numeric_limits<f64_t>::max_digits10) << Field.fNumber
				<< std::setprecision(9);
			break;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::STRING:
			Output << '"' << Client::CDataJson::Escape(Field.strString) << '"';
			break;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::VECTOR:
			Write_Float4(Output, Field.vVector);
			break;
		default:
			Output << "null";
			break;
		}
		Output << " }";
	}


	void Write_SourceLocalReferenceBinding(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC& Binding)
	{
		Output << "{ \"referenceKind\": \""
			<< Client::CDataJson::Escape(Binding.strReferenceKind)
			<< "\", \"referenceId\": \""
			<< Client::CDataJson::Escape(Binding.strReferenceId)
			<< "\", \"definitionId\": \""
			<< Client::CDataJson::Escape(Binding.strDefinitionId)
			<< "\", \"occurrenceId\": \""
			<< Client::CDataJson::Escape(Binding.strOccurrenceId)
			<< "\", \"moduleStableId\": \""
			<< Client::CDataJson::Escape(Binding.strModuleStableId)
			<< "\", \"propertyPath\": \""
			<< Client::CDataJson::Escape(Binding.strPropertyPath)
			<< "\", \"provenance\": \""
			<< Client::CDataJson::Escape(Binding.strProvenance)
			<< "\", \"exactPayload\": [";
		for (size_t iField = 0u; iField < Binding.ExactPayload.size(); ++iField)
		{
			if (iField > 0u)
				Output << ", ";
			Write_SourceTypedField(Output, Binding.ExactPayload[iField]);
		}
		Output << "], \"currentDefaultEvidence\": [";
		for (size_t iField = 0u;
			iField < Binding.CurrentDefaultEvidence.size(); ++iField)
		{
			if (iField > 0u)
				Output << ", ";
			Write_SourceTypedField(
				Output, Binding.CurrentDefaultEvidence[iField]);
		}
		Output << "], \"executionAdmission\": ";
		Write_SourceAdmission(Output, Binding.ExecutionAdmission);
		Output << " }";
	}


	void Write_SourceMaterialAdmission(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_MATERIAL_ADMISSION_DESC& Admission)
	{
		Output << "{ \"status\": \""
			<< Client::CDataJson::Escape(Admission.strStatus)
			<< "\", \"sourceMaterialPaths\": ";
		Write_StringArray(Output, Admission.SourceMaterialPaths);
		Output << ", \"materialRecipeId\": \""
			<< Client::CDataJson::Escape(Admission.strMaterialRecipeId)
			<< "\", \"renderStateRecipeId\": \""
			<< Client::CDataJson::Escape(Admission.strRenderStateRecipeId)
			<< "\", \"blockers\": ";
		Write_StringArray(Output, Admission.Blockers);
		Output << " }";
	}


	void Write_SourceGeometryBinding(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Binding)
	{
		Output << "{ \"enabled\": " << (Binding.bEnabled ? "true" : "false")
			<< ", \"assetId\": \""
			<< Client::CDataJson::Escape(Binding.strAssetId)
			<< "\", \"receiptFileSha256\": \""
			<< Binding.strReceiptFileSha256
			<< "\", \"receiptSelfSha256\": \""
			<< Binding.strReceiptSelfSha256
			<< "\", \"carrierGeometryPreScale\": "
			<< Binding.fCarrierGeometryPreScale
			<< ", \"particleScaleSemantics\": \""
			<< Client::CDataJson::Escape(Binding.strParticleScaleSemantics)
			<< "\", \"status\": \""
			<< Client::CDataJson::Escape(Binding.strStatus)
			<< "\", \"blockers\": ";
		Write_StringArray(Output, Binding.Blockers);
		Output << " }";
	}


	void Write_SourceRecipe(
		std::ostringstream& Output,
		const Client::EFFECT_CASCADE_RECIPE_DESC& Recipe,
		const bool_t bSourceContract)
	{
		Output << "      \"sourceRecipe\": { \"enabled\": "
			<< (Recipe.bEnabled ? "true" : "false")
			<< ", \"rendererShape\": \""
			<< Client::CDataJson::Escape(Recipe.strRendererShape) << '"';
		if (Recipe.bSimulationOnly)
			Output << ", \"simulationOnly\": true";
		if (!Recipe.strParticleSystemOccurrenceId.empty())
			Output << ", \"particleSystemOccurrenceId\": \"" << Client::CDataJson::Escape(Recipe.strParticleSystemOccurrenceId) << '"';
		if (!Recipe.strEmitterName.empty())
			Output << ", \"emitterName\": \"" << Client::CDataJson::Escape(Recipe.strEmitterName) << '"';
		if (Recipe.bAuthoredModuleOverrides && !bSourceContract)
			Output << ", \"authoredModuleOverrides\": true";
		if (bSourceContract)
		{
			Output << ", \"sourceContractProfileId\": \""
				<< Client::CDataJson::Escape(Recipe.strSourceContractProfileId)
				<< "\", \"sourceContractSha256\": \""
				<< Recipe.strSourceContractSha256
				<< "\", \"sourceGraphSha256\": \""
				<< Recipe.strSourceGraphSha256
				<< "\", \"sourceClosureSha256\": \""
				<< Recipe.strSourceClosureSha256
				<< "\", \"sourceMaterialClosureSha256\": \""
				<< Recipe.strSourceMaterialClosureSha256
				<< "\", \"sourcePeakActiveParticles\": "
				<< Recipe.iSourcePeakActiveParticles;
		}
		Output << ", \"emitterDelaySeconds\": "
			<< Recipe.fEmitterDelaySeconds
			<< ", \"emitterDurationSeconds\": "
			<< Recipe.fEmitterDurationSeconds
			<< ", \"emitterLoopCount\": " << Recipe.iEmitterLoopCount
			<< ",\n        \"bursts\": [";
		for (size_t iBurst = 0u; iBurst < Recipe.Bursts.size(); ++iBurst)
		{
			const Client::EFFECT_PARTICLE_BURST_DESC& Burst =
				Recipe.Bursts[iBurst];
			Output << (0u == iBurst ? "\n" : ",\n")
				<< "          { \"timeSeconds\": " << Burst.fTimeSeconds
				<< ", \"countMinimum\": " << Burst.iCountMinimum
				<< ", \"countMaximum\": " << Burst.iCountMaximum
				<< " }";
		}
		if (!Recipe.Bursts.empty())
			Output << '\n';
		Output << "        ],\n        \"modules\": [";
		for (size_t iModule = 0u; iModule < Recipe.Modules.size(); ++iModule)
		{
			const Client::EFFECT_SOURCE_MODULE_DESC& Module =
				Recipe.Modules[iModule];
			Output << (0u == iModule ? "\n" : ",\n")
				<< "          { \"stableId\": \""
				<< Client::CDataJson::Escape(Module.strStableId)
				<< "\", \"className\": \""
				<< Client::CDataJson::Escape(Module.strClassName)
				<< "\", \"objectPath\": \""
				<< Client::CDataJson::Escape(Module.strObjectPath)
				<< "\",\n            \"literals\": [";
			for (size_t iLiteral = 0u; iLiteral < Module.Literals.size();
				++iLiteral)
			{
				const Client::EFFECT_SOURCE_LITERAL_DESC& Literal =
					Module.Literals[iLiteral];
				Output << (0u == iLiteral ? "\n" : ",\n")
					<< "              { \"propertyPath\": \""
					<< Client::CDataJson::Escape(Literal.strPropertyPath)
					<< "\", \"kind\": \""
					<< SOURCE_LITERAL_KIND_TOKENS[
						static_cast<size_t>(Literal.eKind)]
					<< "\", \"value\": ";
				if (Client::EFFECT_SOURCE_LITERAL_KIND::BOOLEAN == Literal.eKind)
					Output << (Literal.bBoolean ? "true" : "false");
				else if (Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER == Literal.eKind)
				{
					Output << std::setprecision(
						std::numeric_limits<f64_t>::max_digits10)
						<< Literal.fNumber << std::setprecision(9);
				}
				else
					Output << '"' << Client::CDataJson::Escape(
						Literal.strString) << '"';
				Output << " }";
			}
			if (!Module.Literals.empty())
				Output << '\n';
			Output << "            ],\n            \"distributions\": [";
			for (size_t iDistribution = 0u;
				iDistribution < Module.Distributions.size(); ++iDistribution)
			{
				const Client::EFFECT_DISTRIBUTION_DESC& Distribution =
					Module.Distributions[iDistribution];
				Output << (0u == iDistribution ? "\n" : ",\n")
					<< "              { \"propertyPath\": \""
					<< Client::CDataJson::Escape(Distribution.strPropertyPath) << '"';
				if (bSourceContract)
				{
					Output << ", \"referenceId\": \""
						<< Client::CDataJson::Escape(Distribution.strReferenceId)
						<< "\", \"occurrenceId\": \""
						<< Client::CDataJson::Escape(Distribution.strOccurrenceId)
						<< "\", \"payloadStatus\": \""
						<< Client::CDataJson::Escape(Distribution.strPayloadStatus)
						<< "\", \"fidelity\": \""
						<< Client::CDataJson::Escape(Distribution.strFidelity)
						<< "\", \"executionAdmission\": ";
					Write_SourceAdmission(
						Output, Distribution.ExecutionAdmission);
				}
				Output << ", \"sourceClass\": \""
					<< Client::CDataJson::Escape(Distribution.strSourceClass)
					<< "\", \"sourceObjectPath\": \""
					<< Client::CDataJson::Escape(
						Distribution.strSourceObjectPath) << '"';
				if (bSourceContract &&
					Is_ParticleParameterDistribution(Distribution.strSourceClass))
				{
					Output << ", \"parameterBinding\": \""
						<< DISTRIBUTION_PARAMETER_BINDING_TOKENS[
							static_cast<size_t>(Distribution.eParameterBinding)]
						<< "\", \"parameterName\": \""
						<< Client::CDataJson::Escape(Distribution.strParameterName)
						<< '"';
				}
				Output << ", \"componentCount\": "
					<< Distribution.iComponentCount
					<< ", \"operation\": " << Distribution.iOperation
					<< ", \"randomLockAxes\": "
					<< Distribution.iRandomLockAxes
					<< ", \"lookupTableChunkSize\": "
					<< Distribution.iLookupTableChunkSize
					<< ", \"lookupTableNumElements\": "
					<< Distribution.iLookupTableNumElements
					<< ", \"lookupTableTimeScale\": "
					<< Distribution.fLookupTableTimeScale
					<< ", \"lookupTableStartTime\": "
					<< Distribution.fLookupTableStartTime
					<< ", \"defaultMinimum\": ";
				Write_Float4(Output, Distribution.vDefaultMinimum);
				Output << ", \"defaultMaximum\": ";
				Write_Float4(Output, Distribution.vDefaultMaximum);
				Output << ", \"lookupTable\": [";
				for (size_t iValue = 0u;
					iValue < Distribution.LookupTable.size(); ++iValue)
				{
					if (iValue > 0u)
						Output << ", ";
					Output << Distribution.LookupTable[iValue];
				}
				Output << "], \"keys\": [";
				for (size_t iKey = 0u; iKey < Distribution.Keys.size(); ++iKey)
				{
					const Client::EFFECT_DISTRIBUTION_KEY_DESC& Key =
						Distribution.Keys[iKey];
					Output << (0u == iKey ? "\n" : ",\n")
						<< "                { \"time\": " << Key.fTime
						<< ", \"minimum\": ";
					Write_Float4(Output, Key.vMinimum);
					Output << ", \"maximum\": ";
					Write_Float4(Output, Key.vMaximum);
					Output << ", \"arriveTangentMinimum\": ";
					Write_Float4(Output, Key.vArriveTangentMinimum);
					Output << ", \"leaveTangentMinimum\": ";
					Write_Float4(Output, Key.vLeaveTangentMinimum);
					Output << ", \"arriveTangentMaximum\": ";
					Write_Float4(Output, Key.vArriveTangentMaximum);
					Output << ", \"leaveTangentMaximum\": ";
					Write_Float4(Output, Key.vLeaveTangentMaximum);
					Output << ", \"interpolation\": \""
						<< DISTRIBUTION_INTERPOLATION_TOKENS[
							static_cast<size_t>(Key.eInterpolation)]
						<< "\" }";
				}
				if (!Distribution.Keys.empty())
					Output << '\n';
				Output << "              ] }";
			}
			if (!Module.Distributions.empty())
				Output << '\n';
			Output << "            ] }";
		}
		if (!Recipe.Modules.empty())
			Output << '\n';
		Output << "        ]";
		if (bSourceContract)
		{
			Output << ",\n        \"localReferenceBindings\": [";
			for (size_t iBinding = 0u;
				iBinding < Recipe.LocalReferenceBindings.size(); ++iBinding)
			{
				Output << (0u == iBinding ? "\n          " : ",\n          ");
				Write_SourceLocalReferenceBinding(
					Output, Recipe.LocalReferenceBindings[iBinding]);
			}
			if (!Recipe.LocalReferenceBindings.empty())
				Output << '\n';
			Output << "        ],\n        \"moduleCoverage\": [";
			for (size_t iCoverage = 0u;
				iCoverage < Recipe.ModuleCoverage.size(); ++iCoverage)
			{
				const Client::EFFECT_SOURCE_MODULE_COVERAGE_DESC& Coverage =
					Recipe.ModuleCoverage[iCoverage];
				Output << (0u == iCoverage ? "\n" : ",\n")
					<< "          { \"moduleStableId\": \""
					<< Client::CDataJson::Escape(Coverage.strModuleStableId);
				if (!Coverage.strExactSourceClass.empty())
				{
					Output << "\", \"exactSourceClass\": \""
						<< Client::CDataJson::Escape(Coverage.strExactSourceClass)
						<< "\", \"aliasId\": \""
						<< Client::CDataJson::Escape(Coverage.strAliasId);
				}
				Output << "\", \"normalizedClass\": \""
					<< Client::CDataJson::Escape(Coverage.strNormalizedClass)
					<< "\", \"status\": \""
					<< SOURCE_COVERAGE_STATUS_TOKENS[
						static_cast<size_t>(Coverage.eStatus)]
					<< "\", \"blockers\": ";
				Write_StringArray(Output, Coverage.Blockers);
				Output << ", \"properties\": [";
				for (size_t iProperty = 0u;
					iProperty < Coverage.Properties.size(); ++iProperty)
				{
					const Client::EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& Property =
						Coverage.Properties[iProperty];
					Output << (0u == iProperty ? "\n" : ",\n")
						<< "            { \"propertyPath\": \""
						<< Client::CDataJson::Escape(Property.strPropertyPath)
						<< "\", \"storage\": \""
						<< Client::CDataJson::Escape(Property.strStorage)
						<< "\", \"status\": \""
						<< SOURCE_COVERAGE_STATUS_TOKENS[
							static_cast<size_t>(Property.eStatus)]
						<< "\", \"provenance\": \""
						<< Client::CDataJson::Escape(Property.strProvenance)
						<< "\", \"blockers\": ";
					Write_StringArray(Output, Property.Blockers);
					Output << " }";
				}
				if (!Coverage.Properties.empty())
					Output << '\n';
				Output << "          ] }";
			}
			if (!Recipe.ModuleCoverage.empty())
				Output << '\n';
			Output << "        ],\n        \"compilerEvidence\": ";
			Write_SourceCompilerEvidence(Output, Recipe.CompilerEvidence);
			Output << ",\n        \"compiledExecutionAdmission\": ";
			Write_SourceAdmission(Output, Recipe.CompiledExecutionAdmission);
			Output << ",\n        \"materialAdmission\": ";
			Write_SourceMaterialAdmission(Output, Recipe.MaterialAdmission);
			Output << ",\n        \"geometryBinding\": ";
			Write_SourceGeometryBinding(Output, Recipe.GeometryBinding);
		}
		Output << " }";
	}


	bool_t Read_PresentationDetail(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DETAIL_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pLight = Find_Field(
			Value, "light", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pScreenPost = Find_Field(
			Value, "screenPost", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		if (nullptr == pLight || nullptr == pScreenPost ||
			!Read_Bool(*pLight, "enabled", Out.Light.bEnabled, strOutError) ||
			!Read_Bool(*pScreenPost, "enabled", Out.ScreenPost.bEnabled,
				strOutError))
		{
			return false;
		}

		if (Out.Light.bEnabled)
		{
			const Client::DATA_JSON_VALUE* pProfile = Find_Field(
				*pLight, "profileId", Client::DATA_JSON_TYPE::STRING,
				strOutError);
			const Client::DATA_JSON_VALUE* pStatus = Find_Field(
				*pLight, "status", Client::DATA_JSON_TYPE::STRING,
				strOutError);
			if (nullptr == pProfile || nullptr == pStatus ||
				!Parse_Token(pProfile->Get_String(), LIGHT_PROFILE_TOKENS,
					std::size(LIGHT_PROFILE_TOKENS), Out.Light.eProfile) ||
				!Parse_Token(pStatus->Get_String(),
					PRESENTATION_RUNTIME_STATUS_TOKENS,
					std::size(PRESENTATION_RUNTIME_STATUS_TOKENS),
					Out.Light.eStatus) ||
				!Read_Float(*pLight, "range", Out.Light.fRange, strOutError) ||
				!Read_Float(*pLight, "intensity", Out.Light.fIntensity,
					strOutError) ||
				!Read_Array(*pLight, "color", &Out.Light.vColor.x, 4u,
					strOutError) ||
				!Read_Array(*pLight, "ambient", &Out.Light.vAmbient.x, 4u,
					strOutError) ||
				!Read_Float(*pLight, "falloffExponent",
					Out.Light.fFalloffExponent, strOutError))
			{
				return false;
			}
		}

		if (Out.ScreenPost.bEnabled)
		{
			const Client::DATA_JSON_VALUE* pProfile = Find_Field(
				*pScreenPost, "profileId", Client::DATA_JSON_TYPE::STRING,
				strOutError);
			const Client::DATA_JSON_VALUE* pStatus = Find_Field(
				*pScreenPost, "status", Client::DATA_JSON_TYPE::STRING,
				strOutError);
			if (nullptr == pProfile || nullptr == pStatus ||
				!Parse_Token(pProfile->Get_String(), SCREEN_POST_PROFILE_TOKENS,
					std::size(SCREEN_POST_PROFILE_TOKENS),
					Out.ScreenPost.eProfile) ||
				!Parse_Token(pStatus->Get_String(),
					PRESENTATION_RUNTIME_STATUS_TOKENS,
					std::size(PRESENTATION_RUNTIME_STATUS_TOKENS),
					Out.ScreenPost.eStatus) ||
				!Read_Float(*pScreenPost, "intensity",
					Out.ScreenPost.fIntensity, strOutError) ||
				!Read_Float(*pScreenPost, "secondaryIntensity",
					Out.ScreenPost.fSecondaryIntensity, strOutError) ||
				!Read_Float(*pScreenPost, "frequency",
					Out.ScreenPost.fFrequency, strOutError) ||
				!Read_Array(*pScreenPost, "tint",
					&Out.ScreenPost.vTint.x, 4u, strOutError) ||
				!Read_UInt(*pScreenPost, "randomSeed",
					Out.ScreenPost.iRandomSeed, strOutError))
			{
				return false;
			}
		}
		return true;
	}


	const char_t* RuntimeStatusToken(
		const Client::EFFECT_PRESENTATION_RUNTIME_STATUS eStatus)
	{
		const size_t iIndex = static_cast<size_t>(eStatus);
		return iIndex < std::size(PRESENTATION_RUNTIME_STATUS_TOKENS) ?
			PRESENTATION_RUNTIME_STATUS_TOKENS[iIndex] :
			"reconstructed_profile";
	}


	void Write_PresentationDetail(
		std::ostringstream& Output,
		const Client::EFFECT_DETAIL_DESC& Detail)
	{
		Output << "        \"light\": { \"enabled\": "
			<< (Detail.Light.bEnabled ? "true" : "false");
		if (Detail.Light.bEnabled)
		{
			Output << ", \"profileId\": \""
				<< LIGHT_PROFILE_TOKENS[static_cast<size_t>(
					Detail.Light.eProfile)]
				<< "\", \"status\": \""
				<< RuntimeStatusToken(Detail.Light.eStatus)
				<< "\", \"range\": " << Detail.Light.fRange
				<< ", \"intensity\": " << Detail.Light.fIntensity
				<< ", \"color\": ";
			Write_Float4(Output, Detail.Light.vColor);
			Output << ", \"ambient\": ";
			Write_Float4(Output, Detail.Light.vAmbient);
			Output << ", \"falloffExponent\": "
				<< Detail.Light.fFalloffExponent;
		}
		Output << " },\n        \"screenPost\": { \"enabled\": "
			<< (Detail.ScreenPost.bEnabled ? "true" : "false");
		if (Detail.ScreenPost.bEnabled)
		{
			Output << ", \"profileId\": \""
				<< SCREEN_POST_PROFILE_TOKENS[static_cast<size_t>(
					Detail.ScreenPost.eProfile)]
				<< "\", \"status\": \""
				<< RuntimeStatusToken(Detail.ScreenPost.eStatus)
				<< "\", \"intensity\": "
				<< Detail.ScreenPost.fIntensity
				<< ", \"secondaryIntensity\": "
				<< Detail.ScreenPost.fSecondaryIntensity
				<< ", \"frequency\": " << Detail.ScreenPost.fFrequency
				<< ", \"tint\": ";
			Write_Float4(Output, Detail.ScreenPost.vTint);
			Output << ", \"randomSeed\": "
				<< Detail.ScreenPost.iRandomSeed;
		}
		Output << " }\n";
	}

}
