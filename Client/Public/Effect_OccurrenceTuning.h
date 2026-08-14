#pragma once

#include "Client_Defines.h"
#include "DataJson.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_RuntimeAuthority.h"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_OCCURRENCE_TUNING_FORMAT_VERSION = 1u;
inline constexpr uint32_t EFFECT_SOURCE_AUTHORING_OVERLAY_FORMAT_VERSION = 2u;

struct EFFECT_OCCURRENCE_LOCAL_TRANSFORM final
{
	float3_t vPosition = { 0.f, 0.f, 0.f };
	float3_t vRotationDegrees = { 0.f, 0.f, 0.f };
	float3_t vScale = { 1.f, 1.f, 1.f };
};

struct EFFECT_OCCURRENCE_TUNING_ENTRY final
{
	std::string strOccurrenceId;
	std::string strSourceOccurrenceRowSha256;
	std::string strProvenance = "PROJECT_TUNED";
	EFFECT_OCCURRENCE_LOCAL_TRANSFORM EffectiveLocalTransform;
};

struct EFFECT_OCCURRENCE_TUNING_DOCUMENT final
{
	uint32_t iFormatVersion = EFFECT_OCCURRENCE_TUNING_FORMAT_VERSION;
	std::string strEffectAssetId;
	std::vector<EFFECT_OCCURRENCE_TUNING_ENTRY> Entries;
};

/* A source-backed authoring overlay identifies one immutable reconstructed
   source occurrence by all three stable joins.  Existing source rows may only
   replace effective local P/R/S and visibility.  Separately authored local
   Decals live in the nested ordinary supplemental document; Renderer,
   SourceRecipe, attachment, module/distribution and prepared-resource
   authority of the immutable source rows remain owned by the admitted Program. */
struct EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY final
{
	std::string strOccurrenceId;
	std::string strSourceOccurrenceRowSha256;
	std::string strSourceElementId;
	std::string strProvenance = "PROJECT_TUNED";
	bool_t bVisible = true;
	EFFECT_OCCURRENCE_LOCAL_TRANSFORM EffectiveLocalTransform;
};

struct EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT final
{
	uint32_t iFormatVersion = EFFECT_SOURCE_AUTHORING_OVERLAY_FORMAT_VERSION;
	std::string strEffectAssetId;
	std::string strSourceProgramSha256;
	std::vector<EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY> Entries;
	EFFECT_DOCUMENT_DESC SupplementalDocument;
};

struct EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION;

class CEffectOccurrenceTuningCodec final
{
public:
	static bool_t Parse(
		std::string_view Utf8Json,
		EFFECT_OCCURRENCE_TUNING_DOCUMENT& OutDocument,
		std::string& strOutError);
	static bool_t Parse_RuntimePayload(
		const DATA_JSON_VALUE& Value,
		std::string_view strExpectedCanonicalSha256,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT>& OutDocument,
		std::string& strOutError);
	static bool_t Load(
		const std::filesystem::path& Path,
		EFFECT_OCCURRENCE_TUNING_DOCUMENT& OutDocument,
		std::string& strOutError);
	static bool_t Validate(
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		std::string& strOutError);
	static bool_t Validate_AgainstProgram(
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		std::string& strOutError);
	static bool_t Validate_AgainstProjection(
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& Projection,
		std::string& strOutError);
	static std::string Serialize(
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document);
	static bool_t Save_AtomicIfUnchanged(
		const std::filesystem::path& Path,
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		std::string_view strExpectedCanonicalDocument,
		std::string& strOutError);
	static bool_t Save_AtomicIfUnchanged(
		const std::filesystem::path& Path,
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& Projection,
		std::string_view strExpectedCanonicalDocument,
		std::string& strOutError);
	/* Rebuilds position/rotation/scale from the immutable Program first, then
	   applies the PROJECT_TUNED absolute values. This makes an empty Document a
	   real reset-to-source operation even when the input projection was already
	   tuned by a published catalog snapshot. */
	static bool_t Apply_ToProjectedDocument(
		EFFECT_DOCUMENT_DESC& InOutDocument,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Tuning,
		std::string& strOutError);
	static bool_t Apply_ToProjectedDocument(
		EFFECT_DOCUMENT_DESC& InOutDocument,
		const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& SourceProjection,
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Tuning,
		std::string& strOutError);
	static const EFFECT_OCCURRENCE_TUNING_ENTRY* Find_Entry(
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		std::string_view strOccurrenceId);
};

class CEffectSourceAuthoringOverlayCodec final
{
public:
	static EFFECT_DOCUMENT_DESC Create_EmptySupplementalDocument(
		std::string_view strEffectAssetId);
	static bool_t Parse(
		std::string_view Utf8Json,
		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& OutDocument,
		std::string& strOutError);
	static bool_t Load(
		const std::filesystem::path& Path,
		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& OutDocument,
		std::string& strOutError);
	static bool_t Validate(
		const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
		std::string& strOutError);
	static bool_t Validate_AgainstProgram(
		const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		std::string& strOutError);
	static bool_t Validate_SupplementalDocument(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string_view strExpectedEffectAssetId,
		std::string& strOutError);
	static std::string Serialize(
		const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document);
	static bool_t Save_AtomicIfUnchanged(
		const std::filesystem::path& Path,
		const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		std::string_view strExpectedCanonicalDocument,
		std::string& strOutError);
	static bool_t Apply_ToProjectedDocument(
		EFFECT_DOCUMENT_DESC& InOutDocument,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Overlay,
		std::string& strOutError);
	/* Legacy occurrence tuning is accepted only as a one-time migration input.
	   The resulting source overlay becomes the sole editable authority. */
	static bool_t Migrate_FromOccurrenceTuning(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& LegacyTuning,
		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& OutOverlay,
		std::string& strOutError);
	static const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY* Find_Entry(
		const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
		std::string_view strOccurrenceId);
	static const EFFECT_ELEMENT_DESC* Find_SupplementalDecal(
		const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
		std::string_view strElementId);
};

NS_END
