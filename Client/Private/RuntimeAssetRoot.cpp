// 호출자가 사용하는 경로 조회 계약과 Windows·Engine 공통 타입을 가져온다.
#include "RuntimeAssetRoot.h"

// Windows 경로 구성 요소를 대소문자 구분 없이 비교할 때 사용한다.
#include <cwctype>

namespace
{
	// 루트 선택 함수가 환경 변수의 경로를 읽는다. 비어 있거나 버퍼보다 긴 값은 빈 경로로 거부한다.
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

	// 현재 작업 디렉터리와 무관하게 실행 파일 위치를 기준으로 배포 폴더를 찾는다. API 실패는 빈 경로다.
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

	// 경로 문자열 전체가 아니라 구성 요소 하나를 비교하여 이름 접두사가 같은 다른 폴더를 구분한다.
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

	// 정규화된 root의 모든 구성 요소가 일치하고 뒤에 자손 요소가 남아 있어야 한다. 루트 자체는 거부한다.
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

	// 기존 경로는 weakly_canonical로 정규화하고, 실패하면 절대 경로의 어휘 정규화를 시도한다.
	// 아직 없는 파일도 처리할 수 있으므로 반환 성공이 파일 존재를 의미하지는 않는다.
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

	// 호출자의 상대 asset ID를 루트 아래에 결합한 뒤 다시 검사한다.
	// 절대 경로나 드라이브 지정 입력, 정규화 후 루트 밖으로 나가는 입력을 빈 경로로 거부한다.
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

// 모든 asset 소비자가 같은 우선순위를 사용하도록 루트 선택을 한곳에 둔다.
// 환경 변수는 명시값을 우선하며, 기본 배치 탐색이 실패해도 EXE 옆 Resources 후보를 반환한다.
filesystem::path CRuntimeAssetRoot::Get_ResourceRoot()
{
	const filesystem::path configuredRoot =
		ReadEnvironmentPath(L"LOSTARK_RESOURCE_ROOT");

	if (!configuredRoot.empty())
		return configuredRoot;

	// 기존 공유 asset 환경 변수는 새 Resources 환경 변수가 없을 때만 사용한다.
	const filesystem::path configuredAssetRoot =
		ReadEnvironmentPath(L"LOSTARK_SHARED_ASSET_ROOT");

	if (!configuredAssetRoot.empty())
		return configuredAssetRoot;

	const filesystem::path moduleDirectory =
		GetModuleDirectory();

	if (moduleDirectory.empty())
		return {};

	// 배포 EXE 옆을 먼저 확인하고, Debug/Release 하위 배치에서는 한 단계 위도 확인한다.
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

// 기존 Get 호출자도 동일한 Resources 루트 선택을 사용한다.
filesystem::path CRuntimeAssetRoot::Get()
{
	return Get_ResourceRoot();
}

// 폰트 전용 조회에 사용할 하위 루트를 구성한다. 폴더 존재 확인이나 생성은 수행하지 않는다.
filesystem::path CRuntimeAssetRoot::Get_FontRoot()
{
	return Get_ResourceRoot() / L"Fonts";
}

// 모델·텍스처 등의 상대 ID에 공용 루트 내부 검사를 적용한다.
filesystem::path CRuntimeAssetRoot::Resolve(
	const filesystem::path& relativePath)
{
	return ResolveInsideRoot(Get(), relativePath);
}

// 폰트는 Resources 전체가 아니라 Fonts 하위로 접근 범위를 좁힌다.
filesystem::path CRuntimeAssetRoot::Resolve_Font(
	const filesystem::path& relativePath)
{
	return ResolveInsideRoot(Get_FontRoot(), relativePath);
}
