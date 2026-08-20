#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

enum class EFFECT_VISUAL_PROGRAM_FAMILY : uint8_t
{
	MESH_PARTICLE,
	SPRITE_PARTICLE,
	DECAL_PARTICLE,
	CASCADE_RIBBON,
	ANIMATION_TRAIL,
	LIGHT_PARTICLE,
	SCREEN_POST,
	END,
};

enum class EFFECT_VISUAL_PROGRAM_DISPOSITION : uint8_t
{
	ADMITTED_BOUNDED,
	FAIL_CLOSED,
	END,
};

enum class EFFECT_VISUAL_PROGRAM_PROJECTION_KIND : uint8_t
{
	SOURCE_RECIPE_OVERLAY_V1,
	ADAPTER_PACKET_V1,
	END,
};

enum class EFFECT_VISUAL_PROGRAM_VALUE_DISPOSITION : uint8_t
{
	CONSUMED,
	SUPPRESSED,
	END,
};

enum class EFFECT_VISUAL_PROGRAM_VALUE_VARIANT : uint8_t
{
	F64,
	F64X4,
	TEXTURE_ID,
	BOOLEAN,
	ENUM_STRING,
	END,
};

struct EFFECT_VISUAL_PROGRAM_SELECTOR final
{
	std::string strEffectAssetId;
	std::string strOccurrenceId;
	std::string strSelectorSha256;
};

struct EFFECT_VISUAL_PROGRAM_SOURCE_IDENTITY final
{
	std::string strSourceRowId;
	std::string strSourceRowSha256;
	std::string strSourceRecordId;
	std::string strSourceRecordSha256;
	std::string strSourceRecipeSha256;
	std::string strModuleClosureSha256;
	uint32_t iModuleCount = 0u;
};

struct EFFECT_VISUAL_PROGRAM_TARGET_IDENTITY final
{
	std::string strTargetElementId;
	std::string strTargetRecordSha256;
	std::string strTargetPayloadRawSha256;
};

struct EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW final
{
	std::string strRole;
	std::string strSlotId;
	std::string strAssetId;
	std::string strResolutionStatus;
	std::string strRawSha256;
	uint64_t iByteCount = 0u;
	std::string strShaderRegister;
	std::string strSourceChannel;
};

struct EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_INPUT final
{
	std::string strInputId;
	std::string strNormalizedParameterName;
	std::string strRowSha256;
	EFFECT_VISUAL_PROGRAM_VALUE_VARIANT eVariant =
		EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::END;
	EFFECT_VISUAL_PROGRAM_VALUE_DISPOSITION eDisposition =
		EFFECT_VISUAL_PROGRAM_VALUE_DISPOSITION::END;
	std::optional<double> fScalarValue;
	std::optional<std::array<double, 4u>> vVectorValue;
	std::string strTextureId;
	std::optional<uint32_t> iPackedScalarIndex;
	std::optional<uint32_t> iPackedVectorIndex;
};

struct EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_STATIC final
{
	std::string strBindingId;
	std::string strNormalizedParameterName;
	std::string strRowSha256;
	std::string strPolicyRowId;
	bool_t bSourceValue = false;
	bool_t bSelectedValue = false;
};

struct EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_RENDER final
{
	std::string strBindingId;
	std::string strFieldName;
	std::string strRowSha256;
	std::string strSourceStatus;
	std::string strSourceFidelity;
	EFFECT_VISUAL_PROGRAM_VALUE_VARIANT eVariant =
		EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::END;
	EFFECT_VISUAL_PROGRAM_VALUE_DISPOSITION eDisposition =
		EFFECT_VISUAL_PROGRAM_VALUE_DISPOSITION::END;
	std::optional<bool_t> bValue;
	std::string strEnumValue;
	std::optional<double> fValue;
};

struct EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_SRV final
{
	std::string strRole;
	std::string strAssetId;
	std::string strRawSha256;
	uint64_t iByteCount = 0u;
	std::string strShaderRegister;
	std::string strSourceChannel;
	std::string strRuntimeSamplerRegister;
	std::string strSourceSamplerEvidence;
	std::string strSamplerPolicy;
	std::string strLinearFormat;
	bool_t bSrgb = false;
	uint32_t iWidth = 0u;
	uint32_t iHeight = 0u;
	uint32_t iMipCount = 0u;
	uint32_t iArraySize = 0u;
};

struct EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_PACKET final
{
	uint32_t iPacketVersion = 0u;
	std::string strAdapterId;
	bool_t bBoundedSemanticReplay = false;
	bool_t bNativeExecution = false;
	bool_t bNativeVertexFactoryAdmitted = false;
	bool_t bNativeMrtAdmitted = false;
	std::string strRuntimeCarrier;
	std::string strNativeVertexFactoryCandidate;
	std::string strNativeVertexShaderSha256;
	std::string strNativePixelShaderSha256;
	std::string strRenderProfile;
	uint32_t iPassIndex = 0u;
	std::string strRasterizerState;
	std::string strDepthStencilState;
	std::string strBlendState;
	uint32_t iStencilReference = 0u;
	uint32_t iOpcode = 0u;
	uint32_t iTextureLaneCount = 0u;
	uint32_t iTextureMask = 0u;
	uint32_t iDynamicConsumedMask = 0u;
	uint32_t iDynamicSuppressedMask = 0u;
	uint32_t iParticleColorPolicy = 0u;
	uint32_t iParticleColorConsumedMask = 0u;
	uint32_t iParticleColorSuppressedMask = 0u;
	std::array<uint32_t, 2u> InputConsumedMask{};
	std::array<uint32_t, 2u> InputSuppressedMask{};
	std::array<uint32_t, 3u> VectorComponentConsumedMask{};
	std::array<uint32_t, 3u> VectorComponentSuppressedMask{};
	uint32_t iStaticSelectedMask = 0u;
	uint32_t iStaticConsumedMask = 0u;
	uint32_t iStaticSuppressedMask = 0u;
	uint32_t iRenderConsumedMask = 0u;
	uint32_t iRenderSuppressedMask = 0u;
	std::vector<EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_INPUT> Inputs;
	std::vector<EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_STATIC> StaticBindings;
	std::vector<EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_RENDER> RenderBindings;
	std::array<double, 22u> PackedScalars{};
	std::array<std::array<double, 4u>, 3u> PackedVectors{};
	std::array<EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_SRV, 6u> Srvs{};
	std::vector<std::string> PreservedLimitations;
	std::string strPacketSha256;
};

struct EFFECT_VISUAL_PROGRAM_TRAIL_TIMING final
{
	double fStartDelaySeconds = 0.0;
	double fLifeTimeSeconds = 0.0;
	double fAfterImageSeconds = 0.0;
	double fDissolveStartNormalized = 0.0;
};

struct EFFECT_VISUAL_PROGRAM_TRAIL_ATTACHMENT final
{
	bool_t bEnabled = false;
	bool_t bFollow = false;
	std::string strSourceAnchorSlotId;
	std::string strRuntimeAnchorSlotId;
	std::string strRuntimeBoneName;
	double fSnapshotRootSourceBasisYawDegrees = 0.0;
	std::array<double, 3u> vPosition{};
	std::array<double, 3u> vRotationDegrees{};
	std::array<double, 3u> vScale{ 1.0, 1.0, 1.0 };
};

struct EFFECT_VISUAL_PROGRAM_TRAIL_GEOMETRY final
{
	uint32_t iMaxPoints = 0u;
	double fPointLifeTimeSeconds = 0.0;
	double fSampleIntervalSeconds = 0.0;
	double fMinimumDistance = 0.0;
	double fStartWidth = 0.0;
	double fEndWidth = 0.0;
	bool_t bFaceCamera = false;
};

struct EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_SAMPLE final
{
	double fRelativeTimeSeconds = 0.0;
	std::array<double, 3u> vFirstEdgeUE3Cm{};
	std::array<double, 3u> vControlPointUE3Cm{};
	std::array<double, 3u> vSecondEdgeUE3Cm{};
};

struct EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY final
{
	std::string strHistoryId;
	std::string strSourceKind;
	std::string strSourceArtifactPath;
	std::string strSourceArtifactRawSha256;
	std::string strCoordinateBasis;
	double fSourceEndTimeSeconds = 0.0;
	double fPlaybackClampSeconds = 0.0;
	uint32_t iSampleCount = 0u;
	std::vector<EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_SAMPLE> Samples;
	std::string strSamplesSha256;
	std::string strHistorySha256;
};

struct EFFECT_VISUAL_PROGRAM_CASCADE_RIBBON_PACKET final
{
	uint32_t iPacketVersion = 0u;
	std::string strAdapterId;
	bool_t bBoundedSemanticReplay = false;
	bool_t bNativeExecution = false;
	std::string strRuntimeCarrier;
	std::string strTypeDataStableId;
	std::string strTypeDataClassName;
	std::string strTypeDataObjectPath;
	std::string strTypeDataModuleSha256;
	std::string strResolvedRendererShape;
	double fTilingDistance = 0.0;
	double fDistanceTessellationStepSize = 0.0;
	double fTangentTessellationScalar = 0.0;
	double fLodValidity = 0.0;
	uint32_t iOperationalMaxPoints = 0u;
	EFFECT_VISUAL_PROGRAM_TRAIL_TIMING Timing;
	EFFECT_VISUAL_PROGRAM_TRAIL_ATTACHMENT Attachment;
	EFFECT_VISUAL_PROGRAM_TRAIL_GEOMETRY Trail;
	std::string strSourceRecipeSha256;
	std::string strModuleClosureSha256;
	uint32_t iModuleCount = 0u;
	std::vector<std::string> PreservedLimitations;
	std::string strPacketSha256;
};

struct EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_PACKET final
{
	uint32_t iPacketVersion = 0u;
	std::string strAdapterId;
	bool_t bBoundedSemanticReplay = false;
	bool_t bNativeExecution = false;
	std::string strRuntimeCarrier;
	std::string strSourceNotifyType;
	std::string strSourceEventId;
	std::string strSourceEventRecordSha256;
	std::string strSourceAsset;
	std::string strClip;
	double fLocalTimeSeconds = 0.0;
	double fGlobalTimeSeconds = 0.0;
	double fDurationSeconds = 0.0;
	std::string strTargetElementId;
	EFFECT_VISUAL_PROGRAM_TRAIL_TIMING TargetTiming;
	EFFECT_VISUAL_PROGRAM_TRAIL_ATTACHMENT Attachment;
	EFFECT_VISUAL_PROGRAM_TRAIL_GEOMETRY Trail;
	std::string strHistoryId;
	std::string strHistorySha256;
	double fPlaybackClampSeconds = 0.0;
	std::string strCoordinateBasis;
	std::vector<std::string> PreservedLimitations;
	std::string strPacketSha256;
};

enum class EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LANE : uint8_t
{
	FIRST_EDGE,
	END,
};

struct EFFECT_VISUAL_PROGRAM_LIGHT_PROFILE final
{
	bool_t bEnabled = false;
	std::string strProfileId;
	std::string strStatus;
	double fRange = 0.0;
	double fIntensity = 0.0;
	std::array<double, 4u> vColor{};
	std::array<double, 4u> vAmbient{};
	double fFalloffExponent = 0.0;
};

struct EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LIGHT_PACKET final
{
	uint32_t iPacketVersion = 0u;
	std::string strAdapterId;
	bool_t bBoundedSemanticReplay = false;
	bool_t bNativeExecution = false;
	std::string strRuntimeCarrier;
	std::string strSourceEventId;
	std::string strSourceEventRecordSha256;
	std::string strTargetElementId;
	std::string strHistoryId;
	std::string strHistorySha256;
	EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LANE eLane =
		EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LANE::END;
	double fActiveStartSeconds = 0.0;
	double fActiveDurationSeconds = 0.0;
	double fActiveEndSeconds = 0.0;
	double fHistoryPlaybackClampSeconds = 0.0;
	std::string strCoordinateBasis;
	std::string strAttachmentEvidenceStatus;
	EFFECT_VISUAL_PROGRAM_LIGHT_PROFILE TargetLight;
	std::vector<std::string> PreservedLimitations;
	std::string strPacketSha256;
};

struct EFFECT_VISUAL_PROGRAM_ROW final
{
	EFFECT_VISUAL_PROGRAM_SELECTOR Selector;
	EFFECT_VISUAL_PROGRAM_FAMILY eFamily = EFFECT_VISUAL_PROGRAM_FAMILY::END;
	EFFECT_VISUAL_PROGRAM_DISPOSITION eDisposition =
		EFFECT_VISUAL_PROGRAM_DISPOSITION::END;
	std::string strAdapterId;
	std::string strPacketLayout;
	std::string strFidelity;
	bool_t bTuningEligibleTransform = false;
	EFFECT_VISUAL_PROGRAM_SOURCE_IDENTITY SourceIdentity;
	std::optional<EFFECT_VISUAL_PROGRAM_TARGET_IDENTITY> TargetIdentity;
	std::vector<EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW> Resources;
	std::optional<EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_PACKET> LocalDecalPacket;
	std::vector<std::string> AdmissionBlockers;
	std::string strRowSha256;
};

struct EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT final
{
	EFFECT_VISUAL_PROGRAM_SELECTOR Selector;
	EFFECT_VISUAL_PROGRAM_FAMILY eFamily = EFFECT_VISUAL_PROGRAM_FAMILY::END;
	EFFECT_VISUAL_PROGRAM_DISPOSITION eDisposition =
		EFFECT_VISUAL_PROGRAM_DISPOSITION::END;
	std::string strAdapterId;
	std::string strPacketLayout;
	std::string strFidelity;
	bool_t bTuningEligibleTransform = false;
	std::string strSourceRecordId;
	std::string strSourceRecordSha256;
	std::string strSourcePayloadRawSha256;
	EFFECT_VISUAL_PROGRAM_TARGET_IDENTITY TargetIdentity;
	std::string strStageId;
	std::string strSourceEventId;
	double fSourceTimelineSeconds = 0.0;
	double fLocalTimeSeconds = 0.0;
	double fDurationSeconds = 0.0;
	std::vector<EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW> Resources;
	std::optional<EFFECT_VISUAL_PROGRAM_CASCADE_RIBBON_PACKET>
		CascadeRibbonPacket;
	std::optional<EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_PACKET>
		AnimationTrailPacket;
	std::optional<EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LIGHT_PACKET>
		BakedEdgeLightPacket;
	std::vector<std::string> AdmissionBlockers;
	std::string strRowSha256;
};

struct EFFECT_VISUAL_PROGRAM final
{
	std::string strEffectAssetId;
	EFFECT_VISUAL_PROGRAM_PROJECTION_KIND eProjectionKind =
		EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::END;
	std::string strBaseDocumentRawSha256;
	std::string strBaseDocumentCanonicalSha256;
	std::string strBaseDocumentTypedCodecSha256;
	uint64_t iProjectedDocumentCanonicalByteCount = 0u;
	std::string strProjectedDocumentSha256;
	std::string strProjectedDocumentTypedCodecSha256;
	std::shared_ptr<const EFFECT_DOCUMENT_DESC> pProjectedDocument;
	std::vector<EFFECT_VISUAL_PROGRAM_ROW> VisualRows;
	std::vector<EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT> SupplementalElements;
	std::vector<EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY>
		BakedEdgeHistories;
	std::string strProgramSha256;
};

struct EFFECT_VISUAL_PROGRAM_CORPUS final
{
	uint32_t iFormatVersion = 0u;
	std::string strRuntimeId;
	std::string strSourceCorpusArtifactSha256;
	uint32_t iDeclaredProgramCount = 0u;
	uint32_t iDeclaredSourceRecipeOverlayProgramCount = 0u;
	uint32_t iDeclaredAdapterPacketProgramCount = 0u;
	uint32_t iDeclaredVisualRowCount = 0u;
	uint32_t iDeclaredSourceRecipeOverlayCount = 0u;
	uint32_t iDeclaredLocalDecalAdapterPacketCount = 0u;
	uint32_t iDeclaredCascadeRibbonVisualRowCount = 0u;
	uint32_t iDeclaredSupplementalElementCount = 0u;
	uint32_t iDeclaredArtistCascadeRibbonElementCount = 0u;
	uint32_t iDeclaredAnimationTrailElementCount = 0u;
	uint32_t iDeclaredBakedEdgeLightElementCount = 0u;
	uint32_t iDeclaredFailClosedCount = 0u;
	uint32_t iDeclaredExtensionCanaryCount = 0u;
	std::vector<EFFECT_VISUAL_PROGRAM> Programs;
	std::string strArtifactSha256;
};

/* Only CEffectVisualProgramCorpusCodec can construct this token.  Playback
   accepts the token rather than a caller-provided boolean so an ordinary
   authoring document cannot opt itself into source-module execution. */
struct EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION final
{
public:
	const EFFECT_DOCUMENT_DESC& Get_Document() const { return *m_pDocument; }
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC>& Get_DocumentShared() const
	{
		return m_pDocument;
	}
	const std::string& Get_EffectAssetId() const { return m_strEffectAssetId; }
	const std::string& Get_ProgramSha256() const { return m_strProgramSha256; }
	EFFECT_VISUAL_PROGRAM_PROJECTION_KIND Get_ProjectionKind() const
	{
		return m_eProjectionKind;
	}
	const std::string& Get_BaseDocumentCanonicalSha256() const
	{
		return m_strBaseDocumentCanonicalSha256;
	}
	const std::string& Get_ProjectedDocumentSha256() const
	{
		return m_strProjectedDocumentSha256;
	}
	const std::string& Get_AdmissionTokenSha256() const
	{
		return m_strAdmissionTokenSha256;
	}
	const std::vector<EFFECT_VISUAL_PROGRAM_SELECTOR>& Get_AdmittedSelectors() const
	{
		return m_AdmittedSelectors;
	}
	const std::vector<EFFECT_VISUAL_PROGRAM_ROW>& Get_AdmittedRows() const
	{
		return m_AdmittedRows;
	}
	const std::vector<EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT>&
	Get_AdmittedSupplementalElements() const
	{
		return m_AdmittedSupplementalElements;
	}
	const std::vector<EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY>&
	Get_BakedEdgeHistories() const
	{
		return m_BakedEdgeHistories;
	}
	const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY*
	Find_BakedEdgeHistory(std::string_view strHistoryId) const;
	const EFFECT_VISUAL_PROGRAM_ROW* Find_RowByOccurrenceId(
		std::string_view strOccurrenceId) const;
	const EFFECT_VISUAL_PROGRAM_ROW* Find_RowByTargetElementId(
		std::string_view strTargetElementId) const;
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
	Find_SupplementalElementByOccurrenceId(
		std::string_view strOccurrenceId) const;
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
	Find_SupplementalElementByTargetElementId(
		std::string_view strTargetElementId) const;
	bool_t Is_Valid() const
	{
		return nullptr != m_pDocument &&
			m_eProjectionKind != EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::END &&
			!m_strAdmissionTokenSha256.empty();
	}

private:
	friend class CEffectVisualProgramCorpusCodec;
	std::shared_ptr<const EFFECT_DOCUMENT_DESC> m_pDocument;
	EFFECT_VISUAL_PROGRAM_PROJECTION_KIND m_eProjectionKind =
		EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::END;
	std::string m_strEffectAssetId;
	std::string m_strProgramSha256;
	std::string m_strBaseDocumentCanonicalSha256;
	std::string m_strProjectedDocumentSha256;
	std::string m_strAdmissionTokenSha256;
	std::vector<EFFECT_VISUAL_PROGRAM_SELECTOR> m_AdmittedSelectors;
	std::vector<EFFECT_VISUAL_PROGRAM_ROW> m_AdmittedRows;
	std::vector<EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT>
		m_AdmittedSupplementalElements;
	std::vector<EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY>
		m_BakedEdgeHistories;
};

struct EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_REQUEST final
{
	std::string strEffectAssetId;
	std::string strOccurrenceId;
	std::string strRowSha256;
	std::string strTargetElementId;
	std::string strSourceRecordId;
};

struct EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_STAGE final
{
	EFFECT_ELEMENT_DESC Element;
	EFFECT_VISUAL_PROGRAM_FAMILY eSourceFamily =
		EFFECT_VISUAL_PROGRAM_FAMILY::END;
	bool_t bSupplemental = false;
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pProjection;
	EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_REQUEST Identity;

	bool_t Is_Valid() const
	{
		return nullptr != pProjection && pProjection->Is_Valid() &&
			eSourceFamily != EFFECT_VISUAL_PROGRAM_FAMILY::END &&
			!Identity.strEffectAssetId.empty() &&
			!Identity.strOccurrenceId.empty() &&
			!Identity.strRowSha256.empty() &&
			!Identity.strTargetElementId.empty() &&
			!Identity.strSourceRecordId.empty() &&
			Element.strElementId == Identity.strTargetElementId;
	}
};

class CEffectVisualProgramCorpusCodec final
{
public:
	static bool_t Parse(
		std::string_view Utf8Json,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_CORPUS>& InOutCorpus,
		std::string& strOutError);
	static bool_t Load(
		const std::filesystem::path& Path,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_CORPUS>& InOutCorpus,
		std::string& strOutError);
	static bool_t Validate(
		const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
		std::string& strOutError);
	static const EFFECT_VISUAL_PROGRAM* Find_Program(
		const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
		std::string_view strEffectAssetId);
	static const EFFECT_VISUAL_PROGRAM_ROW* Find_Row(
		const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
		const EFFECT_VISUAL_PROGRAM_SELECTOR& Selector);
	static const EFFECT_VISUAL_PROGRAM_ROW* Find_RowByTargetElementId(
		const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
		std::string_view strEffectAssetId,
		std::string_view strTargetElementId);
	static const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
	Find_SupplementalElement(
		const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
		const EFFECT_VISUAL_PROGRAM_SELECTOR& Selector);
	static const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
	Find_SupplementalElementByTargetElementId(
		const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
		std::string_view strEffectAssetId,
		std::string_view strTargetElementId);
	static bool_t Create_DocumentProjection(
		const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
		const EFFECT_DOCUMENT_DESC& BaseDocument,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
			InOutProjection,
		std::string& strOutError);
	static bool_t Derive_TransformTunedProjection(
		const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& SourceProjection,
		const EFFECT_DOCUMENT_DESC& TunedDocument,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
			InOutProjection,
		std::string& strOutError);
	static bool_t Build_ElementAuthoringPresetStage(
		const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
			pProjection,
		const EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_REQUEST& Request,
		EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_STAGE& InOutStage,
		std::string& strOutError);
	static std::string Compute_DocumentCanonicalSha256(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
};

NS_END
