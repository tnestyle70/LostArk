#include "RuntimeAssetRoot.h"

#include <cwctype>

namespace
{
	filesystem::path ReadEnvironmentPath(const wchar_t* pName)
	{
		wchar_t value[32768]{};
		const DWORD length = GetEnvironmentVariableW(
			pName,
			value,
			static_cast<DWORD>(std::size(value)));
		if (0 == length || length >= std::size(value))
			return {};
		return filesystem::path(value).lexically_normal();
	}

	filesystem::path GetModuleDirectory()
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr,
			modulePath,
			static_cast<DWORD>(std::size(modulePath)));
		if (0 == length || length >= std::size(modulePath))
			return {};
		return filesystem::path(modulePath).parent_path();
	}

	bool_t EqualPathComponent(
		const filesystem::path& left,
		const filesystem::path& right)
	{
		const std::wstring leftValue = left.native();
		const std::wstring rightValue = right.native();
		if (leftValue.size() != rightValue.size())
			return false;

		for (size_t index = 0; index < leftValue.size(); ++index)
		{
			if (std::towlower(leftValue[index]) !=
				std::towlower(rightValue[index]))
			{
				return false;
			}
		}
		return true;
	}

	bool_t IsInsideRoot(
		const filesystem::path& root,
		const filesystem::path& candidate)
	{
		auto rootIter = root.begin();
		auto candidateIter = candidate.begin();
		for (; rootIter != root.end();
			++rootIter, ++candidateIter)
		{
			if (candidateIter == candidate.end() ||
				!EqualPathComponent(*rootIter, *candidateIter))
			{
				return false;
			}
		}
		return candidateIter != candidate.end();
	}

	filesystem::path Canonicalize(
		const filesystem::path& path)
	{
		std::error_code error;
		filesystem::path result =
			filesystem::weakly_canonical(path, error);
		if (!error)
			return result;

		error.clear();
		result = filesystem::absolute(path, error);
		return error ? filesystem::path{} : result.lexically_normal();
	}

	filesystem::path ResolveInsideRoot(
		const filesystem::path& root,
		const filesystem::path& relativePath)
	{
		if (root.empty() ||
			relativePath.empty() ||
			relativePath.is_absolute() ||
			relativePath.has_root_path())
		{
			return {};
		}

		const filesystem::path canonicalRoot =
			Canonicalize(root);
		const filesystem::path canonicalCandidate =
			Canonicalize(root / relativePath);
		if (canonicalRoot.empty() ||
			canonicalCandidate.empty() ||
			!IsInsideRoot(canonicalRoot, canonicalCandidate))
		{
			return {};
		}

		return canonicalCandidate;
	}
}

filesystem::path CRuntimeAssetRoot::Get_ResourceRoot()
{
	const filesystem::path configuredRoot =
		ReadEnvironmentPath(L"LOSTARK_RESOURCE_ROOT");
	if (!configuredRoot.empty())
		return configuredRoot;

	const filesystem::path configuredAssetRoot =
		ReadEnvironmentPath(L"LOSTARK_SHARED_ASSET_ROOT");
	if (!configuredAssetRoot.empty())
		return configuredAssetRoot;

	const filesystem::path moduleDirectory =
		GetModuleDirectory();
	if (moduleDirectory.empty())
		return {};

	const filesystem::path adjacentRoot =
		moduleDirectory / L"Resources";
	if (filesystem::exists(adjacentRoot))
		return adjacentRoot.lexically_normal();

	const filesystem::path parentRoot =
		moduleDirectory.parent_path() / L"Resources";
	if (filesystem::exists(parentRoot))
		return parentRoot.lexically_normal();

	return adjacentRoot.lexically_normal();
}

filesystem::path CRuntimeAssetRoot::Get()
{
	return Get_ResourceRoot();
}

filesystem::path CRuntimeAssetRoot::Get_FontRoot()
{
	return Get_ResourceRoot() / L"Fonts";
}

filesystem::path CRuntimeAssetRoot::Resolve(
	const filesystem::path& relativePath)
{
	return ResolveInsideRoot(Get(), relativePath);
}

filesystem::path CRuntimeAssetRoot::Resolve_Font(
	const filesystem::path& relativePath)
{
	return ResolveInsideRoot(Get_FontRoot(), relativePath);
}
