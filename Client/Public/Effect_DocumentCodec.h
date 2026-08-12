#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

class DATA_JSON_VALUE;

class CEffectDocumentCodec final
{
public:
	static bool_t Validate(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	static bool_t Validate_SourceContract(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	static bool_t Validate_Drawable(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	static bool_t Validate_Artist31470ReconstructedRuntimeDrawable(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);

	static bool_t Parse(
		std::string_view Json,
		EFFECT_DOCUMENT_DESC& OutDocument,
		std::string& strOutError);

	static bool_t Parse_Value(
		const DATA_JSON_VALUE& Value,
		EFFECT_DOCUMENT_DESC& OutDocument,
		std::string& strOutError);

	static std::string Serialize(const EFFECT_DOCUMENT_DESC& Document);

	static bool_t Load(
		const std::filesystem::path& Path,
		EFFECT_DOCUMENT_DESC& OutDocument,
		std::string& strOutError);

	static bool_t Save_Atomic(
		const std::filesystem::path& Path,
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	static bool_t Save_AtomicIfUnchanged(
		const std::filesystem::path& Path,
		const EFFECT_DOCUMENT_DESC& Document,
		std::string_view strExpectedCanonicalDocument,
		std::string& strOutError);

	static void Collect_ResourceAssetIds(
		const EFFECT_DOCUMENT_DESC& Document,
		std::vector<std::string>& OutAssetIds);

	static const char_t* To_Token(EFFECT_ELEMENT_KIND eKind);
	static const char_t* To_Token(EFFECT_RESOURCE_SLOT eSlot);
	static const char_t* To_Token(EFFECT_RENDER_PROFILE eProfile);
	static bool_t Is_ResourceSlotAllowed(
		EFFECT_ELEMENT_KIND eKind,
		EFFECT_RESOURCE_SLOT eSlot);
	static bool_t Is_SafeResourceAssetId(
		const std::string& strAssetId,
		EFFECT_RESOURCE_FILE_KIND* pOutKind = nullptr);
	static bool_t Is_SafeModelCueAssetId(const std::string& strAssetId);
};

NS_END
