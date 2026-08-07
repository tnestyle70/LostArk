# MapTool navigation bootstrap 관문 수정 계획

대상: `Client/Private/MapTool.cpp`
증상: Debug Lobby `Test` -> `LEVEL::DEVELOPMENT` -> F1 -> Map Tool -> Area `Bern` 선택이 실패하고
Area가 Character Select로 남는다. 상태 줄에 `Required navigation source is missing: LV_BER_BERNCASTLE`.

## 1. 현재 실제 반영 상태와 이번 G 경계

이번 G는 Map Tool Area 전환의 navigation 관문이 "파일 없음"과 "검사 실패"를 구분하지 못하는
오류 판정만 고친다. navigation 데이터 생성, Bern 동기 stage 부하, `Load_RuntimeBlockers`의
동일 잠복 패턴은 이 G에 포함하지 않는다.

### 1.1 근본 원인 실측

`std::filesystem::is_regular_file(path, ec)`는 파일이 없을 때 `ec`에 `2 (ERROR_FILE_NOT_FOUND)`를
설정한다. 형제 함수 `exists(path, ec)`는 같은 상황에서 `ec`를 지운다. 표준이 `exists`에만
`status_known`이면 `ec`를 지우도록 규정하기 때문이다.

같은 toolchain(VS 2022 Community, x64)에서 실제 저장소 경로로 확인한 결과다.

```text
MISSING file:
  is_regular_file -> false  ec=2
  exists          -> false  ec=0
```

현재 관문은 `if (error || (!hasSource && !allowNavigationBootstrap))`이므로 `error ||`가 먼저
단락 평가되고, `allowNavigationBootstrap`은 읽히지 않는다. `LV_BER_BERNCASTLE`은 이 플래그가
`true`인 유일한 Area이므로(`MapTool.cpp:2084-2085`) bootstrap 경로 전체가 도달 불가였다.

### 1.2 Area별 실측 대조

```text
Training Map      navigationPolicy = NONE          검사 자체를 건너뜀       열림
Character Select  SOURCE_PAINT, navsource 존재     ec=0                     열림
Valtan            SOURCE_PAINT, navsource 존재     ec=0                     열림
Bern              SOURCE_PAINT, navsource 없음     ec=2 -> 거부             막힘
```

`Data/Navigation`에는 `LV_LOBBY_CLASSSELECT_SL00.navsource/.navpaint`,
`LV_LUT_HEARTRB_ED.navsource/.navpaint/.navblockers`,
`LV_DEV_TRAINING_GROUND.navgrid.json`만 있고 Bern 항목은 없다. Bern의 `.navsource`는 손으로
쓰는 문서가 아니라 Map Tool `Place Nav Bounds` -> `Bake` 산출물이므로, 관문이 막힌 상태에서는
파일을 만들 수단 자체가 없다.

## 2. 수정 파일

| 구분 | 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/MapTool.cpp` | navigation 관문의 오류 판정 교정 |

새 C++ 파일이 없으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다.

파일 인코딩은 CP949(BOM 없음, CRLF, 탭 들여쓰기)이며 한글 주석이 156 바이트 존재한다.
편집은 Latin-1 왕복으로 바이트를 보존해 수행하고, 새 주석은 파일 다수 주석과 같은 영문
ASCII로 작성한다.

## 3. 추가 선언의 한 줄 책임

### 3.1 include

`#include <system_error>` — `std::errc`를 사용하기 위해 추가한다. 기존에도 `std::error_code`를
쓰고 있어 `<filesystem>` 경유로 간접 포함되지만, 직접 의존을 명시해 전이 포함에 기대지 않는다.
알파벳 순서상 `<map>`과 `<unordered_map>` 사이에 둔다.

### 3.2 헬퍼

```text
파일   : Client/Private/MapTool.cpp
작업   : 추가
기준점 : 익명 namespace의 ReadRequiredString 정의 직후, ResolveDataCatalogPath 정의 직전
위치   : 두 함수 사이
대상   : 파일 지역 함수 정의
이유   : 같은 판정을 세 곳에서 반복하지 않고 한 의미 단위로 소유한다
연결   : Switch_EditorArea, Load_NavigationDocument 두 함수의 세 호출 지점
```

`IsFileInspectionFailure(const std::error_code&) -> bool_t`
한 줄 책임: `error_code`가 진짜 검사 실패인지, 단순히 대상이 없다는 뜻인지 구분한다.
`no_such_file_or_directory`는 실패가 아니며, `ERROR_FILE_NOT_FOUND(2)`와
`ERROR_PATH_NOT_FOUND(3)` 모두 이 generic 조건에 대응한다.

이름은 익명 namespace의 기존 `IsFinite`, `ReadTextFile`, `ReadRequiredString`,
`ResolveDataCatalogPath`와 같은 PascalCase 규칙을 따른다.

## 4. 교체 코드 정본

MapTool.cpp는 약 150KB / 5,000줄이므로 전체 전문 대신 정확한 기준점과 교체 가능한 완전한
블록을 싣는다. 파일 전문이 필요하면 별도로 요청한다.

### 4-1. include 블록

적용 위치: `#include <map>` 다음 줄

```cpp
#include <map>
#include <system_error>
#include <unordered_map>
```

### 4-2. 익명 namespace 헬퍼

적용 위치: `ReadRequiredString` 닫는 `}` 다음, `std::filesystem::path ResolveDataCatalogPath(` 직전

```cpp
	/* is_regular_file reports a missing file through the error_code, so a
	   bare "if (error)" cannot separate an absent optional document from a
	   real inspection failure. Only the latter may abort a load. */
	bool_t IsFileInspectionFailure(const std::error_code& error)
	{
		return error && error != std::errc::no_such_file_or_directory;
	}
```

### 4-3. `CMapTool::Switch_EditorArea` navigation 관문

적용 위치: `EDITOR_NAVIGATION_POLICY::NONE != descriptor.navigationPolicy` 블록 안의 `if`

교체 전

```cpp
		if (error || (!hasSource && !descriptor.allowNavigationBootstrap))
```

교체 후

```cpp
		if (IsFileInspectionFailure(error) ||
			(!hasSource && !descriptor.allowNavigationBootstrap))
```

### 4-4. `CMapTool::Load_NavigationDocument` workspace 관문

적용 위치: `CMapEditorWorkspaceService::Is_Active()` 블록 안의 `if`

교체 전

```cpp
		if (sourceError || (!hasSource && !active->allowNavigationBootstrap))
```

교체 후

```cpp
		if (IsFileInspectionFailure(sourceError) ||
			(!hasSource && !active->allowNavigationBootstrap))
```

### 4-5. `CMapTool::Load_NavigationDocument` contract 관문

적용 위치: `stagedContract.sourcePath` 검사 직후의 `if`. 바로 아래 `if (!hasSource)` bootstrap
분기가 이미 존재하지만 이 `if`가 먼저 return해서 도달 불가였다.

교체 전

```cpp
	if (sourceError)
```

교체 후

```cpp
	if (IsFileInspectionFailure(sourceError))
```

## 5. 불변식

- bootstrap이 허용되지 않은 Area(Character Select, Valtan)에서 navigation source가 없으면
  계속 fail-closed로 거부한다. 이 수정은 그 경계를 완화하지 않는다.
- 권한 거부 같은 실제 I/O 오류는 계속 실패로 잡는다.
- `Switch_EditorArea`의 `parse -> validate -> stage -> commit`과 실패 시 이전 Area 유지 계약은
  바꾸지 않는다. 이번 변경은 validate 단계의 판정만 교정한다.
- Map Tool은 `Data` 저작 문서만 저장하고 runtime 생성물을 만들지 않는다는 경계도 그대로다.

## 6. 검증

1. Client x64 Debug 빌드 성공
2. `git diff --check`
3. CP949 한글 주석 보존과 non-ASCII 바이트 수 불변 확인
4. ProjectAudit 실행과 baseline 대조
5. 실행 검증: Lobby -> `Test` -> F1 -> `Map Tool` -> Area `Bern` 선택이 성공하고 Area 표시가
   `LV_BER_BERNCASTLE`로 바뀌는지, Navigation 패널이 `Navigation bootstrap: place Nav Bounds and Bake`
   상태가 되는지 확인
6. 회귀 확인: Character Select, Valtan, Training Map 세 Area가 이전과 같이 열리는지 확인

## 7. 이번 G에 포함하지 않는 것

- `Load_RuntimeBlockers`의 동일 패턴(현재 호출자 0개인 함수)
- Bern의 1,003 모델 / 텍스처 약 1.77GB 동기 stage로 인한 장시간 무응답
- Bern navigation 실제 bake와 Server 제품 navigation 활성화
