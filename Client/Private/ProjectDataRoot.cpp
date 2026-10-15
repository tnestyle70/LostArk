// 호출자가 사용하는 경로 조회 계약과 Windows·Engine 공통 타입을 가져온다.
#include "ProjectDataRoot.h"

// Windows 경로 구성 요소를 대소문자 구분 없이 비교할 때 사용한다.
#include <cwctype>

namespace
{
	// 루트 선택 함수가 환경 변수의 경로를 읽는다. 비어 있거나 버퍼보다 긴 값은 빈 경로로 거부한다.
	filesystem::path ReadEnvironmentPath(const wchar_t* pName)
	{
		wchar_t value[32768]{};
		const DWORD length = GetEnvironmentVariableW(
			pName, value, static_cast<DWORD>(std::size(value)));
		if (0 == length || length >= std::size(value))
			return {};
		return filesystem::path(value).lexically_normal();
	}

	// 현재 작업 디렉터리와 무관하게 실행 파일 위치를 기준으로 배포 폴더를 찾는다. API 실패는 빈 경로다.
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

	// 존재하는 경로를 우선 정규화하고, 실패하면 절대 경로의 어휘 정규화로 새 문서 경로도 처리한다.
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

	// 경로 요소를 대소문자 구분 없이 비교한다. 같은 접두사의 다른 디렉터리를 허용하지 않도록 요소별로 검사한다.
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

	// 후보가 root의 모든 요소를 공유하는 자손인지 검사한다. root 자체를 파일 대상으로 반환하지 않는다.
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

// 저작 문서 소비자가 Data 정본을 찾는다. 명시 환경 변수를 우선하고 실행 파일부터 상위 폴더를 탐색한다.
filesystem::path CProjectDataRoot::Get()
{
	const filesystem::path configured =
		ReadEnvironmentPath(L"LOSTARK_PROJECT_DATA_ROOT");
	if (!configured.empty())
		return configured;

	// 현재 작업 디렉터리 변화에 영향받지 않으며, 파일시스템 루트에서 탐색을 끝낸다.
	for (filesystem::path current = GetModuleDirectory();
		!current.empty(); current = current.parent_path())
	{
		const filesystem::path candidate = current / L"Data";
		if (filesystem::is_directory(candidate))
			return candidate.lexically_normal();
		if (current == current.root_path())
			break;
	}

	// 탐색에 실패하면 기존 Client/Bin 배치 기준의 후보를 반환한다. 이 경로의 존재는 보장하지 않는다.
	return GetModuleDirectory().parent_path().parent_path().
		parent_path() / L"Data";
}

// 호출자가 넘긴 Data 상대 경로를 정규화하고 자손 여부를 확인한다.
// 실패하면 빈 경로를 반환하며 실제 문서 읽기·쓰기와 오류 표시는 호출자에게 남긴다.
filesystem::path CProjectDataRoot::Resolve(
	const filesystem::path& relativePath)
{
	if (relativePath.empty() || relativePath.is_absolute() ||
		relativePath.has_root_path())
	{
		return {};
	}

	// 결합 전후를 모두 정규화하여 .. 또는 실제 경로 해석으로 Data 밖에 도달하는 입력을 거부한다.
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
