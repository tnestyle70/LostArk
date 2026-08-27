#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

struct VALTAN_PATTERN_AUTHORING_EFFECT_BINDING final
{
	std::string strPatternId;
	std::string strEffectAssetId;
	std::string strAuthoringPath;
	std::string strState = "DRAFT_ATTACHED";

	bool operator==(
		const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING&) const = default;
};

struct VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT final
{
	std::string strBossArchetypeId = "BOSS_VALTAN";
	std::vector<VALTAN_PATTERN_AUTHORING_EFFECT_BINDING> Bindings;

	bool operator==(
		const VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT&) const = default;
};

/* Serializes the complete two-document New Effect transaction across Tool
   processes. The opaque handle keeps Win32 out of the public header. */
class CValtanPatternAuthoringEffectTransaction final
{
public:
	CValtanPatternAuthoringEffectTransaction() = default;
	~CValtanPatternAuthoringEffectTransaction();

	CValtanPatternAuthoringEffectTransaction(
		const CValtanPatternAuthoringEffectTransaction&) = delete;
	CValtanPatternAuthoringEffectTransaction& operator=(
		const CValtanPatternAuthoringEffectTransaction&) = delete;

	bool_t Try_Acquire(std::string& strOutStatus);

private:
	void* m_pMutexHandle = nullptr;
	bool_t m_bOwned = false;
};

/* Tool-only authoring ownership. This document deliberately has no stage,
   clip, timing, anchor, animation, Product, or Server trigger fields. */
class CValtanPatternAuthoringEffectDocument final
{
public:
	static std::filesystem::path Resolve_Path();
	static std::string Build_AuthoringPath(
		std::string_view strEffectAssetId);
	static std::filesystem::path Resolve_AuthoringPath(
		const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING& Binding);

	static bool_t Parse_Text(
		std::string_view strText,
		VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& OutDocument,
		std::string& strOutStatus);
	static bool_t Validate(
		const VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& Document,
		std::string& strOutStatus);
	static std::string Serialize(
		const VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& Document);
	static bool_t Load(
		VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& OutDocument,
		std::string& strOutCanonicalBaseline,
		std::string& strOutStatus);
	static bool_t Save_AtomicIfUnchanged(
		const VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& Document,
		std::string_view strExpectedCanonicalBaseline,
		std::string& strOutStatus);
};

NS_END
