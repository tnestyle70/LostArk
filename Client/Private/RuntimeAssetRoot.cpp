#include "RuntimeAssetRoot.h"

filesystem::path CRuntimeAssetRoot::Get()
{
	wchar_t environmentRoot[32768]{};
	const DWORD environmentLength = GetEnvironmentVariableW(
		L"LOSTARK_SHARED_ASSET_ROOT",
		environmentRoot,
		static_cast<DWORD>(size(environmentRoot)));
	if (0 < environmentLength && environmentLength < size(environmentRoot))
		return filesystem::path(environmentRoot).lexically_normal();

	wchar_t modulePath[32768]{};
	const DWORD moduleLength = GetModuleFileNameW(
		nullptr,
		modulePath,
		static_cast<DWORD>(size(modulePath)));
	if (0 == moduleLength || moduleLength >= size(modulePath))
		return {};

	const filesystem::path moduleDirectory =
		filesystem::path(modulePath).parent_path();
	const filesystem::path adjacentRoot =
		moduleDirectory / L"Resources" / L"LostArk";
	if (filesystem::exists(adjacentRoot))
		return adjacentRoot.lexically_normal();

	const filesystem::path parentRoot =
		moduleDirectory.parent_path() / L"Resources" / L"LostArk";
	if (filesystem::exists(parentRoot))
		return parentRoot.lexically_normal();

	return adjacentRoot.lexically_normal();
}

filesystem::path CRuntimeAssetRoot::Resolve(const filesystem::path& relativePath)
{
	return (Get() / relativePath).lexically_normal();
}
