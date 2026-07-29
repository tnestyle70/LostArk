#include "Effect_ResourceCatalog.h"

#include <charconv>
#include <fstream>

bool_t Client::CEffect_ResourceCatalog::Load(
	const filesystem::path& ParticleSystemsCsv,
	string* pError)
{
	m_Entries.clear();
	m_isLoaded = false;

	ifstream Stream(ParticleSystemsCsv, ios::binary);
	if (!Stream)
	{
		if (nullptr != pError)
			*pError = "Cannot open SourceCatalog/particle_systems.csv.";
		return false;
	}

	string strLine;
	vector<string> Columns;
	if (!getline(Stream, strLine) ||
		!Parse_CsvLine(strLine, Columns) ||
		Columns.size() != 9)
	{
		if (nullptr != pError)
			*pError = "Particle source catalog header is invalid.";
		return false;
	}

	uint32_t iLineNumber = 1;
	while (getline(Stream, strLine))
	{
		++iLineNumber;
		if (strLine.empty())
			continue;

		if (!Parse_CsvLine(strLine, Columns) || Columns.size() != 9)
		{
			if (nullptr != pError)
				*pError = "Invalid CSV row at line " +
					to_string(iLineNumber) + ".";
			m_Entries.clear();
			return false;
		}

		EFFECT_SOURCE_PARTICLE_ENTRY Entry;
		Entry.strGroup = move(Columns[0]);
		Entry.strRole = move(Columns[1]);
		Entry.strLogicalPackage = move(Columns[2]);
		Entry.strPhysicalPackage = move(Columns[3]);

		const char* pExportBegin = Columns[4].data();
		const char* pExportEnd = pExportBegin + Columns[4].size();
		const auto ExportResult = from_chars(
			pExportBegin, pExportEnd, Entry.iExportIndex);
		if (ExportResult.ec != errc{} ||
			ExportResult.ptr != pExportEnd ||
			!Parse_Hex(Columns[5], Entry.iExportOffset) ||
			!Parse_Hex(Columns[6], Entry.iExportSize))
		{
			if (nullptr != pError)
				*pError = "Invalid numeric field at line " +
					to_string(iLineNumber) + ".";
			m_Entries.clear();
			return false;
		}

		Entry.strObjectName = move(Columns[7]);
		Entry.strCandidateTags = move(Columns[8]);
		m_Entries.push_back(move(Entry));
	}

	m_isLoaded = true;
	return true;
}

vector<size_t> Client::CEffect_ResourceCatalog::Find(
	const string& strGroup,
	const string& strNameFilter) const
{
	vector<size_t> Results;
	const string strGroupLower = ToLower(strGroup);
	const string strFilterLower = ToLower(strNameFilter);

	for (size_t i = 0; i < m_Entries.size(); ++i)
	{
		const EFFECT_SOURCE_PARTICLE_ENTRY& Entry = m_Entries[i];
		if (!strGroupLower.empty() &&
			ToLower(Entry.strGroup) != strGroupLower)
		{
			continue;
		}

		if (!strFilterLower.empty())
		{
			const string strSearchText =
				ToLower(Entry.strLogicalPackage + "." +
					Entry.strObjectName + ";" +
					Entry.strCandidateTags);
			if (string::npos == strSearchText.find(strFilterLower))
				continue;
		}

		Results.push_back(i);
	}
	return Results;
}

bool_t Client::CEffect_ResourceCatalog::Parse_CsvLine(
	const string& strLine,
	vector<string>& OutColumns)
{
	OutColumns.clear();
	string strValue;
	bool_t isQuoted = false;

	for (size_t i = 0; i < strLine.size(); ++i)
	{
		const char_t ch = strLine[i];
		if ('"' == ch)
		{
			if (isQuoted && i + 1 < strLine.size() &&
				'"' == strLine[i + 1])
			{
				strValue.push_back('"');
				++i;
			}
			else
			{
				isQuoted = !isQuoted;
			}
		}
		else if (',' == ch && !isQuoted)
		{
			OutColumns.push_back(move(strValue));
			strValue.clear();
		}
		else if ('\r' != ch)
		{
			strValue.push_back(ch);
		}
	}

	if (isQuoted)
		return false;

	OutColumns.push_back(move(strValue));
	return true;
}

bool_t Client::CEffect_ResourceCatalog::Parse_Hex(
	const string& strText,
	uint64_t& OutValue)
{
	const char* pBegin = strText.data();
	const char* pEnd = pBegin + strText.size();
	const auto Result = from_chars(pBegin, pEnd, OutValue, 16);
	return Result.ec == errc{} && Result.ptr == pEnd;
}

string Client::CEffect_ResourceCatalog::ToLower(string strValue)
{
	transform(strValue.begin(), strValue.end(), strValue.begin(),
		[](const unsigned char ch)
		{
			return static_cast<char_t>(tolower(ch));
		});
	return strValue;
}
