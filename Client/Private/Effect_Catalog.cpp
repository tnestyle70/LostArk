#include "Effect_Catalog.h"

#include "DataJson.h"
#include "Effect_DocumentCodec.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <unordered_set>

namespace
{
    std::map<std::string,
        std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>,
        std::less<>> g_Effects;
	std::map<std::string,
		std::shared_ptr<const Client::EFFECT_ASSEMBLY_DESC>,
		std::less<>> g_Assemblies;
	std::map<std::string,
		std::shared_ptr<const Client::EFFECT_COMPONENT_DESC>,
		std::less<>> g_Components;
    uint64_t g_iRuntimeRevision = 0u;
    std::string g_strStatus = "Effect catalog has not been loaded.";

    std::filesystem::path Get_ModuleDirectory()
    {
        wchar_t Buffer[32768]{};
        const DWORD Length = GetModuleFileNameW(
            nullptr, Buffer, static_cast<DWORD>(std::size(Buffer)));
        if (0u == Length || Length >= std::size(Buffer))
            return {};
        return std::filesystem::path(Buffer).parent_path();
    }

    std::filesystem::path Find_RuntimeCatalog()
    {
        const std::filesystem::path Module = Get_ModuleDirectory();
        const std::filesystem::path Adjacent =
            Module / L"DataFiles" / L"Effect" / L"EffectCatalog.runtime.json";
        if (std::filesystem::is_regular_file(Adjacent))
            return Adjacent;
        const std::filesystem::path Parent = Module.parent_path() /
            L"DataFiles" / L"Effect" / L"EffectCatalog.runtime.json";
        return std::filesystem::is_regular_file(Parent) ? Parent : Adjacent;
    }

    const Client::DATA_JSON_VALUE* Required(
        const Client::DATA_JSON_VALUE& Object,
        const char* pName,
        const Client::DATA_JSON_TYPE eType)
    {
        const Client::DATA_JSON_VALUE* pValue = Object.Find(pName);
        return nullptr != pValue && pValue->Get_Type() == eType ?
            pValue : nullptr;
    }

    bool Is_LowerHexSha256(const std::string& Value)
    {
        return 64u == Value.size() && std::all_of(
            Value.begin(), Value.end(), [](const char Character)
            {
                return (Character >= '0' && Character <= '9') ||
                    (Character >= 'a' && Character <= 'f');
            });
    }

	bool Is_StableId(const std::string& Value)
	{
		return !Value.empty() && Value.size() <= 256u &&
			std::all_of(Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= 'a' && Character <= 'z') ||
					(Character >= 'A' && Character <= 'Z') ||
					(Character >= '0' && Character <= '9') ||
					Character == '_' || Character == '-' || Character == '.';
			});
	}

	bool Read_U32(const Client::DATA_JSON_VALUE& Object,
		const char* pName, uint32_t& iOutValue)
	{
		const Client::DATA_JSON_VALUE* pValue = Required(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() != std::floor(pValue->Get_Number()) ||
			pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() > static_cast<double>(UINT32_MAX))
		{
			return false;
		}
		iOutValue = static_cast<uint32_t>(pValue->Get_Number());
		return true;
	}

	bool Read_Float(const Client::DATA_JSON_VALUE& Object,
		const char* pName, f32_t& fOutValue)
	{
		const Client::DATA_JSON_VALUE* pValue = Required(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() < -FLT_MAX || pValue->Get_Number() > FLT_MAX)
		{
			return false;
		}
		fOutValue = static_cast<f32_t>(pValue->Get_Number());
		return true;
	}

	bool Read_Float3(const Client::DATA_JSON_VALUE& Object,
		const char* pName, float3_t& vOutValue)
	{
		const Client::DATA_JSON_VALUE* pValue = Required(
			Object, pName, Client::DATA_JSON_TYPE::ARRAY);
		if (nullptr == pValue || 3u != pValue->Get_Array().size())
			return false;
		f32_t Values[3]{};
		for (size_t i = 0u; i < 3u; ++i)
		{
			const Client::DATA_JSON_VALUE& Item = pValue->Get_Array()[i];
			if (!Item.Is_Number() || !std::isfinite(Item.Get_Number()) ||
				Item.Get_Number() < -FLT_MAX || Item.Get_Number() > FLT_MAX)
			{
				return false;
			}
			Values[i] = static_cast<f32_t>(Item.Get_Number());
		}
		vOutValue = { Values[0], Values[1], Values[2] };
		return true;
	}

	bool Parse_Transform(const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_TRANSFORM_DESC& OutTransform)
	{
		return Value.Is_Object() &&
			Read_Float3(Value, "position", OutTransform.vPosition) &&
			Read_Float3(Value, "rotationDegrees",
				OutTransform.vRotationDegrees) &&
			Read_Float3(Value, "scale", OutTransform.vScale) &&
			OutTransform.vScale.x > 0.f && OutTransform.vScale.y > 0.f &&
			OutTransform.vScale.z > 0.f;
	}

	bool Is_Identity(const Client::EFFECT_TRANSFORM_DESC& Transform)
	{
		return Transform.vPosition.x == 0.f && Transform.vPosition.y == 0.f &&
			Transform.vPosition.z == 0.f &&
			Transform.vRotationDegrees.x == 0.f &&
			Transform.vRotationDegrees.y == 0.f &&
			Transform.vRotationDegrees.z == 0.f &&
			Transform.vScale.x == 1.f && Transform.vScale.y == 1.f &&
			Transform.vScale.z == 1.f;
	}

	bool Parse_Component(const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_COMPONENT_DESC& OutComponent, std::string& strOutError)
	{
		using namespace Client;
		const DATA_JSON_VALUE* pSchema = Required(
			Value, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pVersion = Required(
			Value, "version", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* pAssetId = Required(
			Value, "componentAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDisplayName = Required(
			Value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pType = Required(
			Value, "componentType", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSource = Required(
			Value, "source", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pEmitters = Required(
			Value, "emitters", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pDocument = Required(
			Value, "document", DATA_JSON_TYPE::OBJECT);
		if (nullptr == pSchema || pSchema->Get_String() !=
				"lostark.effect-component" || nullptr == pVersion ||
			pVersion->Get_Number() != 1.0 || nullptr == pAssetId ||
			!Is_StableId(pAssetId->Get_String()) || nullptr == pDisplayName ||
			pDisplayName->Get_String().empty() || nullptr == pType ||
			pType->Get_String().empty() || nullptr == pSource ||
			nullptr == pEmitters || nullptr == pDocument)
		{
			strOutError = "Effect Component header is invalid.";
			return false;
		}
		EFFECT_COMPONENT_DESC Staged;
		Staged.strComponentAssetId = pAssetId->Get_String();
		Staged.strDisplayName = pDisplayName->Get_String();
		Staged.strComponentType = pType->Get_String();
		const DATA_JSON_VALUE* pSourceEffect = Required(
			*pSource, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pGroup = Required(
			*pSource, "groupId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pNodes = Required(
			*pSource, "sourceNodes", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pSourceSha = Required(
			*pSource, "sourceElementSha256", DATA_JSON_TYPE::STRING);
		if (nullptr == pSourceEffect || !Is_StableId(pSourceEffect->Get_String()) ||
			nullptr == pGroup || !Is_StableId(pGroup->Get_String()) ||
			nullptr == pNodes || nullptr == pSourceSha ||
			!Is_LowerHexSha256(pSourceSha->Get_String()))
		{
			strOutError = "Effect Component source provenance is invalid.";
			return false;
		}
		Staged.strSourceEffectAssetId = pSourceEffect->Get_String();
		Staged.strSourceGroupId = pGroup->Get_String();
		Staged.strSourceElementSha256 = pSourceSha->Get_String();
		for (const DATA_JSON_VALUE& Node : pNodes->Get_Array())
		{
			if (!Node.Is_String())
			{
				strOutError = "Effect Component source node is invalid.";
				return false;
			}
			Staged.SourceNodes.push_back(Node.Get_String());
		}
		if (!CEffectDocumentCodec::Parse_Value(
			*pDocument, Staged.Document, strOutError) ||
			Staged.Document.strEffectAssetId != Staged.strComponentAssetId)
		{
			return false;
		}
		std::map<std::string, const EFFECT_ELEMENT_DESC*, std::less<>> Elements;
		for (const EFFECT_ELEMENT_DESC& Element : Staged.Document.Elements)
			Elements.emplace(Element.strElementId, &Element);
		if (Elements.size() != Staged.Document.Elements.size() ||
			pEmitters->Get_Array().size() != Elements.size())
		{
			strOutError = "Effect Component Emitter/Element identity is invalid.";
			return false;
		}
		for (const DATA_JSON_VALUE& EmitterValue : pEmitters->Get_Array())
		{
			const DATA_JSON_VALUE* pEmitterId = Required(
				EmitterValue, "emitterId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pElementId = Required(
				EmitterValue, "elementId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pRenderer = Required(
				EmitterValue, "renderer", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pVisible = Required(
				EmitterValue, "visible", DATA_JSON_TYPE::BOOLEAN);
			EFFECT_COMPONENT_EMITTER_DESC Emitter;
			if (nullptr == pEmitterId || !Is_StableId(pEmitterId->Get_String()) ||
				nullptr == pElementId || !Is_StableId(pElementId->Get_String()) ||
				nullptr == pRenderer || pRenderer->Get_String().empty() ||
				nullptr == pVisible ||
				!Read_U32(EmitterValue, "sourceElementIndex",
					Emitter.iSourceElementIndex) ||
				!Read_U32(EmitterValue, "resourceBindingCount",
					Emitter.iResourceBindingCount) ||
				!Read_U32(EmitterValue, "moduleCount", Emitter.iModuleCount))
			{
				strOutError = "Effect Component Emitter metadata is invalid.";
				return false;
			}
			const auto ElementIterator = Elements.find(pElementId->Get_String());
			if (Elements.end() == ElementIterator ||
				Emitter.iResourceBindingCount !=
					ElementIterator->second->ResourceBindings.size() ||
				Emitter.iModuleCount !=
					ElementIterator->second->SourceRecipe.Modules.size() ||
				pVisible->Get_Boolean() != ElementIterator->second->bVisible)
			{
				strOutError = "Effect Component Emitter payload does not match its Element.";
				return false;
			}
			Emitter.strEmitterId = pEmitterId->Get_String();
			Emitter.strElementId = pElementId->Get_String();
			Emitter.strRendererType = pRenderer->Get_String();
			Emitter.bVisible = pVisible->Get_Boolean();
			Staged.Emitters.push_back(std::move(Emitter));
		}
		OutComponent = std::move(Staged);
		return true;
	}

	bool Parse_Assembly(const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_ASSEMBLY_DESC& OutAssembly, std::string& strOutError)
	{
		using namespace Client;
		const DATA_JSON_VALUE* pSchema = Required(
			Value, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pVersion = Required(
			Value, "version", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* pAssetId = Required(
			Value, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDisplayName = Required(
			Value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceSha = Required(
			Value, "sourceDocumentSha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceFileSha = Required(
			Value, "sourceDocumentFileSha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pParticleSystem = Required(
			Value, "particleSystem", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pModelCues = Required(
			Value, "modelCues", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pComponentCues = Required(
			Value, "componentCues", DATA_JSON_TYPE::ARRAY);
		uint32_t iSourceVersion = 0u;
		if (nullptr == pSchema || pSchema->Get_String() !=
				"lostark.effect-assembly" || nullptr == pVersion ||
			pVersion->Get_Number() != 1.0 || nullptr == pAssetId ||
			!Is_StableId(pAssetId->Get_String()) || nullptr == pDisplayName ||
			pDisplayName->Get_String().empty() || nullptr == pSourceSha ||
			!Is_LowerHexSha256(pSourceSha->Get_String()) ||
			nullptr == pSourceFileSha ||
			!Is_LowerHexSha256(pSourceFileSha->Get_String()) ||
			nullptr == pParticleSystem || nullptr == pModelCues ||
			nullptr == pComponentCues ||
			!Read_U32(Value, "sourceAuthoringVersion", iSourceVersion) ||
			iSourceVersion < EFFECT_AUTHORING_MIN_SUPPORTED_VERSION ||
			iSourceVersion > EFFECT_AUTHORING_FORMAT_VERSION)
		{
			strOutError = "Effect Assembly header is invalid.";
			return false;
		}

		DATA_JSON_VALUE::OBJECT HeaderFields;
		HeaderFields.emplace("schema",
			DATA_JSON_VALUE::String("lostark.effect-authoring"));
		HeaderFields.emplace("version",
			DATA_JSON_VALUE::Number(static_cast<double>(iSourceVersion)));
		HeaderFields.emplace("effectAssetId", *pAssetId);
		HeaderFields.emplace("displayName", *pDisplayName);
		HeaderFields.emplace("particleSystem", *pParticleSystem);
		HeaderFields.emplace("modelCues", *pModelCues);
		HeaderFields.emplace("elements", DATA_JSON_VALUE::Array({}));
		EFFECT_DOCUMENT_DESC HeaderDocument;
		if (!CEffectDocumentCodec::Parse_Value(
			DATA_JSON_VALUE::Object(std::move(HeaderFields)),
			HeaderDocument, strOutError))
		{
			return false;
		}

		EFFECT_ASSEMBLY_DESC Staged;
		Staged.strEffectAssetId = pAssetId->Get_String();
		Staged.strDisplayName = pDisplayName->Get_String();
		Staged.iSourceAuthoringVersion = iSourceVersion;
		Staged.strSourceDocumentSha256 = pSourceSha->Get_String();
		Staged.strSourceDocumentFileSha256 = pSourceFileSha->Get_String();
		Staged.ParticleSystem = HeaderDocument.ParticleSystem;
		Staged.ModelCues = std::move(HeaderDocument.ModelCues);
		std::unordered_set<std::string> CueIds;
		std::unordered_set<std::string> ComponentIds;
		for (const DATA_JSON_VALUE& CueValue : pComponentCues->Get_Array())
		{
			const DATA_JSON_VALUE* pCueId = Required(
				CueValue, "cueId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pComponentId = Required(
				CueValue, "componentAssetId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pVisible = Required(
				CueValue, "visible", DATA_JSON_TYPE::BOOLEAN);
			const DATA_JSON_VALUE* pAnchor = Required(
				CueValue, "anchor", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pTransform = Required(
				CueValue, "localTransform", DATA_JSON_TYPE::OBJECT);
			EFFECT_COMPONENT_CUE_DESC Cue;
			if (nullptr == pCueId || !Is_StableId(pCueId->Get_String()) ||
				!CueIds.insert(pCueId->Get_String()).second ||
				nullptr == pComponentId ||
				!Is_StableId(pComponentId->Get_String()) ||
				!ComponentIds.insert(pComponentId->Get_String()).second ||
				nullptr == pVisible || !pVisible->Get_Boolean() ||
				nullptr == pAnchor || pAnchor->Get_String() != "root" ||
				nullptr == pTransform ||
				!Read_Float(CueValue, "startDelaySeconds",
					Cue.fStartDelaySeconds) || Cue.fStartDelaySeconds < 0.f ||
				!Parse_Transform(*pTransform, Cue.LocalTransform) ||
				!Is_Identity(Cue.LocalTransform))
			{
				strOutError = "Effect Assembly Component cue is invalid or requires an unsupported transform.";
				return false;
			}
			Cue.strCueId = pCueId->Get_String();
			Cue.strComponentAssetId = pComponentId->Get_String();
			Cue.bVisible = pVisible->Get_Boolean();
			Cue.strAnchorSlotId = pAnchor->Get_String();
			Staged.ComponentCues.push_back(std::move(Cue));
		}
		if (Staged.ComponentCues.empty())
		{
			strOutError = "Effect Assembly has no Component cues.";
			return false;
		}
		OutAssembly = std::move(Staged);
		return true;
	}

	bool Compile_Assembly(const Client::EFFECT_ASSEMBLY_DESC& Assembly,
		const std::map<std::string,
			std::shared_ptr<const Client::EFFECT_COMPONENT_DESC>,
			std::less<>>& Components,
		Client::EFFECT_DOCUMENT_DESC& OutDocument, std::string& strOutError)
	{
		using namespace Client;
		struct INDEXED_ELEMENT final
		{
			uint32_t iSourceIndex = 0u;
			EFFECT_ELEMENT_DESC Element;
		};
		std::vector<INDEXED_ELEMENT> IndexedElements;
		std::set<uint32_t> SourceIndices;
		std::unordered_set<std::string> ElementIds;
		for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly.ComponentCues)
		{
			const auto ComponentIterator = Components.find(Cue.strComponentAssetId);
			if (Components.end() == ComponentIterator ||
				ComponentIterator->second->strSourceEffectAssetId !=
					Assembly.strEffectAssetId)
			{
				strOutError = "Effect Assembly references a missing or foreign Component: " +
					Cue.strComponentAssetId;
				return false;
			}
			const EFFECT_COMPONENT_DESC& Component = *ComponentIterator->second;
			std::map<std::string, const EFFECT_ELEMENT_DESC*, std::less<>> Elements;
			for (const EFFECT_ELEMENT_DESC& Element : Component.Document.Elements)
				Elements.emplace(Element.strElementId, &Element);
			for (const EFFECT_COMPONENT_EMITTER_DESC& Emitter : Component.Emitters)
			{
				const auto ElementIterator = Elements.find(Emitter.strElementId);
				if (Elements.end() == ElementIterator ||
					!SourceIndices.insert(Emitter.iSourceElementIndex).second ||
					!ElementIds.insert(Emitter.strElementId).second)
				{
					strOutError = "Effect Assembly has a duplicate or missing Emitter Element.";
					return false;
				}
				INDEXED_ELEMENT Indexed;
				Indexed.iSourceIndex = Emitter.iSourceElementIndex;
				Indexed.Element = *ElementIterator->second;
				Indexed.Element.Detail.Timing.fStartDelaySeconds +=
					Cue.fStartDelaySeconds;
				if (!std::isfinite(
					Indexed.Element.Detail.Timing.fStartDelaySeconds))
				{
					strOutError = "Effect Assembly produced a non-finite timeline.";
					return false;
				}
				IndexedElements.push_back(std::move(Indexed));
			}
		}
		std::sort(IndexedElements.begin(), IndexedElements.end(),
			[](const INDEXED_ELEMENT& Left, const INDEXED_ELEMENT& Right)
			{
				return Left.iSourceIndex < Right.iSourceIndex;
			});
		EFFECT_DOCUMENT_DESC Staged;
		Staged.strEffectAssetId = Assembly.strEffectAssetId;
		Staged.strDisplayName = Assembly.strDisplayName;
		Staged.ParticleSystem = Assembly.ParticleSystem;
		Staged.ModelCues = Assembly.ModelCues;
		Staged.Elements.reserve(IndexedElements.size());
		for (INDEXED_ELEMENT& Indexed : IndexedElements)
			Staged.Elements.push_back(std::move(Indexed.Element));
		if (!CEffectDocumentCodec::Validate_Drawable(Staged, strOutError))
			return false;
		OutDocument = std::move(Staged);
		return true;
	}
}

bool_t Client::CEffectCatalog::Load(std::string& strOutStatus)
{
    const std::filesystem::path Path = Find_RuntimeCatalog();
    std::ifstream Input(Path, std::ios::binary);
    if (!Input)
    {
        strOutStatus = "Missing EffectCatalog.runtime.json: " + Path.string();
        g_strStatus = strOutStatus;
        return false;
    }
    const std::string Text{
        std::istreambuf_iterator<char>(Input),
        std::istreambuf_iterator<char>() };
    DATA_JSON_VALUE Root;
    std::string Error;
	DATA_JSON_PARSE_LIMITS RuntimeCatalogLimits;
	RuntimeCatalogLimits.iMaximumBytes = 64u * 1024u * 1024u;
	RuntimeCatalogLimits.iMaximumDepth = 64u;
	RuntimeCatalogLimits.iMaximumValues = 3'000'000u;
    if (!CDataJson::Parse(Text, Root, Error, RuntimeCatalogLimits) ||
		!Root.Is_Object())
    {
        strOutStatus = "Effect runtime catalog JSON parse failed: " + Error;
        g_strStatus = strOutStatus;
        return false;
    }
    const DATA_JSON_VALUE* pVersion = Required(
        Root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* pComponents = Required(
		Root, "components", DATA_JSON_TYPE::ARRAY);
    const DATA_JSON_VALUE* pEffects = Required(
        Root, "effects", DATA_JSON_TYPE::ARRAY);
    if (nullptr == pVersion || 2.0 != pVersion->Get_Number() ||
		nullptr == pComponents || nullptr == pEffects)
    {
        strOutStatus = "Effect runtime catalog must be formatVersion 2 with Component and Effect arrays.";
        g_strStatus = strOutStatus;
        return false;
    }

    std::map<std::string,
        std::shared_ptr<const EFFECT_DOCUMENT_DESC>, std::less<>> Staged;
	std::map<std::string,
		std::shared_ptr<const EFFECT_COMPONENT_DESC>, std::less<>>
		StagedComponents;
	std::map<std::string,
		std::shared_ptr<const EFFECT_ASSEMBLY_DESC>, std::less<>>
		StagedAssemblies;
	for (const DATA_JSON_VALUE& ComponentValue : pComponents->Get_Array())
	{
		EFFECT_COMPONENT_DESC Component;
		if (!Parse_Component(ComponentValue, Component, Error))
		{
			strOutStatus = "Effect runtime Component rejected: " + Error;
			g_strStatus = strOutStatus;
			return false;
		}
		const std::string ComponentId = Component.strComponentAssetId;
		if (!StagedComponents.emplace(ComponentId,
			std::make_shared<const EFFECT_COMPONENT_DESC>(
				std::move(Component))).second)
		{
			strOutStatus = "Duplicate Effect Component in runtime catalog: " +
				ComponentId;
			g_strStatus = strOutStatus;
			return false;
		}
	}
	for (const DATA_JSON_VALUE& Entry : pEffects->Get_Array())
    {
        if (!Entry.Is_Object())
        {
            strOutStatus = "Effect runtime catalog contains a non-object entry.";
            g_strStatus = strOutStatus;
            return false;
        }
        const DATA_JSON_VALUE* pAssetId = Required(
            Entry, "effectAssetId", DATA_JSON_TYPE::STRING);
        const DATA_JSON_VALUE* pAuthoringVersion = Required(
            Entry, "authoringFormatVersion", DATA_JSON_TYPE::NUMBER);
        const DATA_JSON_VALUE* pContentSha = Required(
            Entry, "contentSha256", DATA_JSON_TYPE::STRING);
        const DATA_JSON_VALUE* pDependencies = Required(
            Entry, "dependencies", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pAssembly = Required(
			Entry, "assembly", DATA_JSON_TYPE::OBJECT);
        const double AuthoringVersion = nullptr == pAuthoringVersion ?
            0.0 : pAuthoringVersion->Get_Number();
        if (nullptr == pAssetId || pAssetId->Get_String().empty() ||
            nullptr == pAuthoringVersion ||
            AuthoringVersion != std::floor(AuthoringVersion) ||
            AuthoringVersion < EFFECT_AUTHORING_MIN_SUPPORTED_VERSION ||
            AuthoringVersion > EFFECT_AUTHORING_FORMAT_VERSION ||
            nullptr == pContentSha || !Is_LowerHexSha256(pContentSha->Get_String()) ||
			nullptr == pDependencies || nullptr == pAssembly)
        {
            strOutStatus = "Effect runtime catalog entry has an invalid header.";
            g_strStatus = strOutStatus;
            return false;
        }
        std::map<std::string, std::string, std::less<>> Dependencies;
        for (const DATA_JSON_VALUE& Dependency : pDependencies->Get_Array())
        {
            const DATA_JSON_VALUE* pDependencyId = Required(
                Dependency, "assetId", DATA_JSON_TYPE::STRING);
            const DATA_JSON_VALUE* pDependencySha = Required(
                Dependency, "sha256", DATA_JSON_TYPE::STRING);
            if (nullptr == pDependencyId ||
                (!CEffectDocumentCodec::Is_SafeResourceAssetId(
                    pDependencyId->Get_String()) &&
                 !CEffectDocumentCodec::Is_SafeModelCueAssetId(
                    pDependencyId->Get_String())) ||
                nullptr == pDependencySha ||
                !Is_LowerHexSha256(pDependencySha->Get_String()))
            {
                strOutStatus = "Effect runtime catalog has an invalid dependency.";
                g_strStatus = strOutStatus;
                return false;
            }
            if (!Dependencies.emplace(
                pDependencyId->Get_String(),
                pDependencySha->Get_String()).second)
            {
                strOutStatus = "Effect runtime catalog has a duplicate dependency.";
                g_strStatus = strOutStatus;
                return false;
            }
        }

		EFFECT_ASSEMBLY_DESC Assembly;
		if (!Parse_Assembly(*pAssembly, Assembly, Error) ||
			Assembly.strEffectAssetId != pAssetId->Get_String() ||
			Assembly.iSourceAuthoringVersion !=
				static_cast<uint32_t>(AuthoringVersion) ||
			Assembly.strSourceDocumentFileSha256 != pContentSha->Get_String())
        {
            strOutStatus = "Effect runtime Assembly rejected for " +
                pAssetId->Get_String() + ": " + Error;
            g_strStatus = strOutStatus;
            return false;
        }
		EFFECT_DOCUMENT_DESC Document;
		if (!Compile_Assembly(Assembly, StagedComponents, Document, Error))
		{
			strOutStatus = "Effect runtime Assembly compile failed for " +
				pAssetId->Get_String() + ": " + Error;
			g_strStatus = strOutStatus;
			return false;
		}
        std::vector<std::string> DocumentDependencies;
        CEffectDocumentCodec::Collect_ResourceAssetIds(
            Document, DocumentDependencies);
        if (DocumentDependencies.size() != Dependencies.size())
        {
            strOutStatus = "Effect runtime dependency set does not match its compiled Assembly.";
            g_strStatus = strOutStatus;
            return false;
        }
        for (const std::string& DependencyId : DocumentDependencies)
        {
			if (!Dependencies.contains(DependencyId))
			{
				strOutStatus = "Effect runtime dependency is missing from the manifest: " +
					DependencyId;
				g_strStatus = strOutStatus;
				return false;
			}
		}
		auto CommittedAssembly =
			std::make_shared<const EFFECT_ASSEMBLY_DESC>(std::move(Assembly));
		if (!StagedAssemblies.emplace(pAssetId->Get_String(),
			std::move(CommittedAssembly)).second)
		{
			strOutStatus = "Duplicate Effect Assembly in runtime catalog: " +
				pAssetId->Get_String();
			g_strStatus = strOutStatus;
			return false;
		}
		auto Committed = std::make_shared<const EFFECT_DOCUMENT_DESC>(
            std::move(Document));
        if (!Staged.emplace(pAssetId->Get_String(),
            std::move(Committed)).second)
        {
            strOutStatus = "Duplicate EffectAssetId in runtime catalog: " +
                pAssetId->Get_String();
            g_strStatus = strOutStatus;
            return false;
        }
    }

	std::unordered_set<std::string> ReferencedComponentIds;
	for (const auto& [EffectId, Assembly] : StagedAssemblies)
	{
		for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly->ComponentCues)
			ReferencedComponentIds.insert(Cue.strComponentAssetId);
	}
	if (ReferencedComponentIds.size() != StagedComponents.size())
	{
		strOutStatus = "Effect runtime catalog contains an unreferenced Component.";
		g_strStatus = strOutStatus;
		return false;
	}

    g_Effects = std::move(Staged);
	g_Assemblies = std::move(StagedAssemblies);
	g_Components = std::move(StagedComponents);
    ++g_iRuntimeRevision;
    g_strStatus = "Loaded " + std::to_string(g_Effects.size()) +
        " Effect Assemblies and " + std::to_string(g_Components.size()) +
		" Components.";
    strOutStatus = g_strStatus;
    return true;
}

std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>
Client::CEffectCatalog::Find(const std::string& strEffectAssetId)
{
    const auto Iterator = g_Effects.find(strEffectAssetId);
    return g_Effects.end() == Iterator ? nullptr : Iterator->second;
}

std::shared_ptr<const Client::EFFECT_ASSEMBLY_DESC>
Client::CEffectCatalog::Find_Assembly(const std::string& strEffectAssetId)
{
	const auto Iterator = g_Assemblies.find(strEffectAssetId);
	return g_Assemblies.end() == Iterator ? nullptr : Iterator->second;
}

std::shared_ptr<const Client::EFFECT_COMPONENT_DESC>
Client::CEffectCatalog::Find_Component(const std::string& strComponentAssetId)
{
	const auto Iterator = g_Components.find(strComponentAssetId);
	return g_Components.end() == Iterator ? nullptr : Iterator->second;
}

bool_t Client::CEffectCatalog::Contains(const std::string& strEffectAssetId)
{
    return g_Effects.contains(strEffectAssetId);
}

std::vector<std::string> Client::CEffectCatalog::Get_EffectAssetIds()
{
    std::vector<std::string> Result;
    Result.reserve(g_Effects.size());
    for (const auto& [AssetId, Document] : g_Effects)
        Result.push_back(AssetId);
    return Result;
}

std::vector<std::string> Client::CEffectCatalog::Get_ComponentAssetIds()
{
	std::vector<std::string> Result;
	Result.reserve(g_Components.size());
	for (const auto& [AssetId, Component] : g_Components)
		Result.push_back(AssetId);
	return Result;
}

uint64_t Client::CEffectCatalog::Get_RuntimeRevision()
{
    return g_iRuntimeRevision;
}

const std::string& Client::CEffectCatalog::Get_Status()
{
    return g_strStatus;
}

void Client::CEffectCatalog::Clear()
{
    g_Effects.clear();
	g_Assemblies.clear();
	g_Components.clear();
    g_strStatus = "Effect catalog cleared.";
}
