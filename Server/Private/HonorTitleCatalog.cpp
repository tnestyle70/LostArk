#include "HonorTitleCatalog.h"

#include <Windows.h>

#include <charconv>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace
{
	/* Same root resolution as the other Server catalogs (VehicleCatalog.cpp):
	LOSTARK_SERVER_DATA_ROOT, else <exe>/../DataFiles. */
	std::filesystem::path Resolve_DataRoot()
	{
		wchar_t configured[32768]{};
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", configured,
			static_cast<DWORD>(std::size(configured)));
		if (0u != configuredLength && configuredLength < std::size(configured))
			return std::filesystem::path(configured).lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == moduleLength || moduleLength >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path().parent_path() /
			L"DataFiles";
	}

	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	std::vector<std::string_view> SplitTabs(const std::string_view line)
	{
		std::vector<std::string_view> fields;
		std::size_t start = 0u;
		while (true)
		{
			const std::size_t tab = line.find('\t', start);
			if (std::string_view::npos == tab)
			{
				fields.push_back(line.substr(start));
				return fields;
			}
			fields.push_back(line.substr(start, tab - start));
			start = tab + 1u;
		}
	}

	template<typename T>
	bool ParseNumber(const std::string_view value, T& output)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), output);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}
}

bool LostArk::Server::CHonorTitleCatalog::Load()
{
	std::unordered_set<LostArk::Shared::HONOR_TITLE_ID> staged;

	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"HonorTitles" / L"HonorTitles.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing honor title bootstrap: " + path.string();
		return false;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Honor title bootstrap is empty";
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0u;
	std::uint32_t rowCount = 0u;
	if (3u != header.size() || "LOSTARK_HONOR_TITLE_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) || 1u != version ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 4096u)
	{
		m_strStatus = "Honor title bootstrap header is invalid";
		return false;
	}

	for (std::uint32_t row = 0u; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Honor title bootstrap row is truncated";
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		LostArk::Shared::HONOR_TITLE_ID titleId = LostArk::Shared::INVALID_HONOR_TITLE_ID;
		if (2u != fields.size() || "TITLE" != fields[0] ||
			!ParseNumber(fields[1], titleId) ||
			LostArk::Shared::INVALID_HONOR_TITLE_ID == titleId)
		{
			m_strStatus = "Honor title bootstrap row is invalid";
			return false;
		}
		if (!staged.insert(titleId).second)
		{
			m_strStatus = "Duplicate honor title ID";
			return false;
		}
	}

	if (std::getline(input, line))
	{
		m_strStatus = "Honor title bootstrap has trailing rows";
		return false;
	}

	m_Titles = std::move(staged);
	m_strStatus = "Loaded honor title bootstrap";
	return true;
}

bool LostArk::Server::CHonorTitleCatalog::Has_Title(
	const LostArk::Shared::HONOR_TITLE_ID titleId) const
{
	return m_Titles.contains(titleId);
}
