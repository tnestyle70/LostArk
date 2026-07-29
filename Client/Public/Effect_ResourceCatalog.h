#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>

NS_BEGIN(Client)

struct EFFECT_SOURCE_PARTICLE_ENTRY
{
	string strGroup;
	string strRole;
	string strLogicalPackage;
	string strPhysicalPackage;
	uint32_t iExportIndex = {};
	uint64_t iExportOffset = {};
	uint64_t iExportSize = {};
	string strObjectName;
	string strCandidateTags;
};

/*
 * Reads the compact SourceCatalog generated next to the raw Lost Ark assets.
 *
 * The catalog identifies Bard/Glaivier ParticleSystem objects without walking
 * the 80+ GiB resource tree at editor start-up.  It deliberately does not
 * claim to resolve the original Cascade dependency graph; the current export
 * manifest states that those texture/material/module links are unavailable.
 */
class CEffect_ResourceCatalog final
{
public:
	bool_t Load(const filesystem::path& ParticleSystemsCsv,
		string* pError = nullptr);

	const vector<EFFECT_SOURCE_PARTICLE_ENTRY>& Get_Entries() const {
		return m_Entries;
	}

	vector<size_t> Find(const string& strGroup,
		const string& strNameFilter) const;

	bool_t Is_Loaded() const { return m_isLoaded; }

private:
	static bool_t Parse_CsvLine(const string& strLine,
		vector<string>& OutColumns);
	static bool_t Parse_Hex(const string& strText, uint64_t& OutValue);
	static string ToLower(string strValue);

private:
	vector<EFFECT_SOURCE_PARTICLE_ENTRY> m_Entries;
	bool_t m_isLoaded = { false };
};

NS_END
