#include "ProjectDataRoot.h"

#include <cwctype>

namespace
{
	filesystem::path ReadEnvironmentPath(const wchar_t* pName)
	{
		wchar_t value[32768]{};
		const DWORD length = GetEnvironmentVariableW(
			pName, value, static_cast<DWORD>(std::size(value)));
		if (0 == length || length >= std::size(value))
			return {};
		return filesystem::path(value).lexically_normal();
	}

	filesystem::path GetModuleDirectory()
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr, modulePath,
			static_cast<DWORD>(std::size(modulePath)));
		if (0 == length || length >= std::size(modulePath))
			return {};
		return filesystem::path(modulePath).parent_path();
	}

	filesystem::path Canonicalize(const filesystem::path& path)
	{
		error_code error;
		filesystem::path resolved =
			filesystem::weakly_canonical(path, error);
		if (!error)
			return resolved;

		error.clear();
		resolved = filesystem::absolute(path, error);
		return error ? filesystem::path{} :
			resolved.lexically_normal();
	}

	bool_t EqualComponent(
		const filesystem::path& left,
		const filesystem::path& right)
	{
		const wstring leftValue = left.native();
		const wstring rightValue = right.native();
		if (leftValue.size() != rightValue.size())
			return false;

		for (size_t index = 0; index < leftValue.size(); ++index)
		{
			if (towlower(leftValue[index]) !=
				towlower(rightValue[index]))
			{
				return false;
			}
		}
		return true;
	}

	bool_t IsDescendant(
		const filesystem::path& root,
		const filesystem::path& candidate)
	{
		auto rootIterator = root.begin();
		auto candidateIterator = candidate.begin();
		for (; rootIterator != root.end();
			++rootIterator, ++candidateIterator)
		{
			if (candidateIterator == candidate.end() ||
				!EqualComponent(*rootIterator, *candidateIterator))
			{
				return false;
			}
		}
		return candidateIterator != candidate.end();
	}
}

filesystem::path CProjectDataRoot::Get()
{
	const filesystem::path configured =
		ReadEnvironmentPath(L"LOSTARK_PROJECT_DATA_ROOT");
	if (!configured.empty())
		return configured;

	for (filesystem::path current = GetModuleDirectory();
		!current.empty(); current = current.parent_path())
	{
		const filesystem::path candidate = current / L"Data";
		if (filesystem::is_directory(candidate))
			return candidate.lexically_normal();
		if (current == current.root_path())
			break;
	}

	return GetModuleDirectory().parent_path().parent_path().
		parent_path() / L"Data";
}

filesystem::path CProjectDataRoot::Resolve(
	const filesystem::path& relativePath)
{
	if (relativePath.empty() || relativePath.is_absolute() ||
		relativePath.has_root_path())
	{
		return {};
	}

	const filesystem::path root = Canonicalize(Get());
	const filesystem::path candidate =
		Canonicalize(root / relativePath);
	if (root.empty() || candidate.empty() ||
		!IsDescendant(root, candidate))
	{
		return {};
	}
	return candidate;
}
