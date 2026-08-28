#include "ValtanClearRewards.h"

#include <Windows.h>

#include <algorithm>
#include <charconv>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace
{
	// Same resolution rule as CItemCatalog's own Resolve_DataRoot.
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

	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> fields;
		const std::string_view view(line);
		std::size_t start = 0;
		while (true)
		{
			const std::size_t tab = view.find('\t', start);
			fields.push_back(view.substr(
				start, std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1;
		}
		return fields;
	}

	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	template<typename T>
	bool ParseNumber(const std::string_view value, T& output)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), output);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}

	bool IsStableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 64u &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	}
}

bool LostArk::Server::CValtanClearRewards::Load()
{
	std::vector<std::string> previousItemIds = std::move(m_ItemIds);
	m_ItemIds.clear();

	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"Valtan" / L"ClearRewards.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing Valtan clear rewards bootstrap: " + path.string();
		m_ItemIds = std::move(previousItemIds);
		return false;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Valtan clear rewards bootstrap is empty";
		m_ItemIds = std::move(previousItemIds);
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t rowCount = 0;
	if (3u != header.size() ||
		"LOSTARK_VALTAN_CLEAR_REWARDS_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) || 1u != version ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 64u)
	{
		m_strStatus = "Valtan clear rewards bootstrap header is invalid";
		m_ItemIds = std::move(previousItemIds);
		return false;
	}

	m_ItemIds.reserve(rowCount);
	for (std::uint32_t row = 0; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Valtan clear rewards bootstrap row is truncated";
			m_ItemIds = std::move(previousItemIds);
			return false;
		}
		StripCarriageReturn(line);
		if (!IsStableId(line))
		{
			m_strStatus = "Valtan clear rewards bootstrap row is invalid";
			m_ItemIds = std::move(previousItemIds);
			return false;
		}
		m_ItemIds.emplace_back(line);
	}

	if (std::getline(input, line))
	{
		m_strStatus = "Valtan clear rewards bootstrap has trailing rows";
		m_ItemIds = std::move(previousItemIds);
		return false;
	}

	m_strStatus = "Loaded Valtan clear rewards bootstrap";
	return true;
}
