#include "ItemCatalog.h"

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
	// Same resolution rule as CGameplayCatalog::Resolve_DataRoot: an explicit
	// override first, otherwise the DataFiles folder next to the running exe.
	std::filesystem::path Resolve_DataRoot()
	{
		std::vector<wchar_t> pathBuffer(32768u);
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
			return std::filesystem::path(pathBuffer.data()).lexically_normal();

		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
		if (0u == moduleLength || moduleLength >= pathBuffer.size())
			return {};
		return std::filesystem::path(pathBuffer.data()).parent_path().parent_path() /
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

bool LostArk::Server::CItemCatalog::Load()
{
	using ITEM_MAP = decltype(m_Items);
	ITEM_MAP previousItems = std::move(m_Items);
	m_Items.clear();

	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"Items" / L"Items.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing item bootstrap: " + path.string();
		m_Items = std::move(previousItems);
		return false;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Item bootstrap is empty";
		m_Items = std::move(previousItems);
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t rowCount = 0;
	if (3u != header.size() || "LOSTARK_ITEM_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) || 2u != version ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 4096u)
	{
		m_strStatus = "Item bootstrap header is invalid";
		m_Items = std::move(previousItems);
		return false;
	}

	for (std::uint32_t row = 0; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Item bootstrap row is truncated";
			m_Items = std::move(previousItems);
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		SERVER_ITEM_DEFINITION item{};
		if (4u != fields.size() || "ITEM" != fields[0] || !IsStableId(fields[1]) ||
			!ParseNumber(fields[2], item.iMaxStack) || 0u == item.iMaxStack ||
			!ParseNumber(fields[3], item.iHealPercent) || item.iHealPercent > 100u)
		{
			m_strStatus = "Item bootstrap row is invalid";
			m_Items = std::move(previousItems);
			return false;
		}
		item.strItemId = fields[1];
		if (!m_Items.emplace(item.strItemId, std::move(item)).second)
		{
			m_strStatus = "Duplicate item ID";
			m_Items = std::move(previousItems);
			return false;
		}
	}

	if (std::getline(input, line))
	{
		m_strStatus = "Item bootstrap has trailing rows";
		m_Items = std::move(previousItems);
		return false;
	}

	m_strStatus = "Loaded item bootstrap";
	return true;
}

const LostArk::Server::SERVER_ITEM_DEFINITION*
LostArk::Server::CItemCatalog::Find_Item(const std::string& itemId) const
{
	const auto iter = m_Items.find(itemId);
	return m_Items.end() == iter ? nullptr : &iter->second;
}
