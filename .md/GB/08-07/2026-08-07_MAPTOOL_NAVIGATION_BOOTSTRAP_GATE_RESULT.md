# MapTool navigation bootstrap 관문 수정 결과

대응 계획: `2026-08-07_MAPTOOL_NAVIGATION_BOOTSTRAP_GATE_PLAN.md`
브랜치: `main` 기준 작업 트리 (커밋 전)

## 1. 구현 완료

`Client/Private/MapTool.cpp` 한 파일, 14 insertions / 3 deletions.

| 위치 | 변경 |
|---|---|
| include 블록 | `#include <system_error>` 추가 |
| 익명 namespace | `IsFileInspectionFailure(const std::error_code&)` 추가 |
| `Switch_EditorArea` | navigation 관문의 `error \|\|` -> `IsFileInspectionFailure(error) \|\|` |
| `Load_NavigationDocument` workspace 관문 | `sourceError \|\|` -> `IsFileInspectionFailure(sourceError) \|\|` |
| `Load_NavigationDocument` contract 관문 | `if (sourceError)` -> `if (IsFileInspectionFailure(sourceError))` |

새 C++ 파일이 없으므로 `.vcxproj` / `.vcxproj.filters` 변경 없음.

## 2. 근본 원인 재현 증거

원인을 코드 독해가 아니라 실행으로 확정했다. 문제 경로만 떼어낸 독립 프로그램을 같은
toolchain(VS 2022 Community, x64, C++17)으로 빌드해 실제 저장소 경로로 실행했다.

`ProjectDataRoot.cpp`의 `Canonicalize`/`Get`/`Resolve`와 `MapTool.cpp`의
`ResolveDataCatalogPath`, 그리고 관문 조건식을 그대로 옮긴 결과다.

```text
=== LV_LOBBY_CLASSSELECT_SL00  (allowNavigationBootstrap=false) ===
  hasSource     : true      error.value() : 0    GATE : passes
=== LV_BER_BERNCASTLE          (allowNavigationBootstrap=true) ===
  hasSource     : false     error.value() : 2    GATE : >>> REJECTED
  error.message : The system cannot find the file specified.
=== LV_LUT_HEARTRB_ED          (allowNavigationBootstrap=false) ===
  hasSource     : true      error.value() : 0    GATE : passes
```

`is_regular_file`과 `exists`의 비대칭도 실측했다.

```text
PRESENT file:  is_regular_file -> true   ec=0    exists -> true   ec=0
MISSING file:  is_regular_file -> false  ec=2    exists -> false  ec=0
```

`MapTool.cpp:4589`가 `exists`를 쓰고 있어 같은 결함이 아님도 이 결과로 확인했다.

## 3. 수정안 사전 검증

적용 전에 후보 두 안을 5개 입력으로 검증했다.

| 입력 | 현재 | A안(errc 비교) | B안(file_status) |
|---|---|---|---|
| 파일 있음, bootstrap 불가 | pass | pass | pass |
| 파일 없음, bootstrap 가능 (Bern) | REJECT | pass | pass |
| 상위 폴더 없음, bootstrap 가능 | REJECT | pass | pass |
| 빈 경로, bootstrap 가능 | REJECT | pass | pass |
| 파일 없음, bootstrap 불가 (Valtan형) | REJECT | REJECT | REJECT |

마지막 행이 중요하다. 두 안 모두 bootstrap이 허용되지 않은 Area의 fail-closed 경계를 유지한다.
diff가 작은 A안을 채택했다.

## 4. 자동 검증 (실행함)

| 항목 | 결과 |
|---|---|
| Client x64 Debug 빌드 | 성공. `Client\Bin\Debug\Client.exe` 2026-08-07 13:10:48 생성 |
| `MapTool.cpp` 신규 오류/경고 | 없음 |
| `git diff --check` | clean |
| CP949 보존 | non-ASCII 바이트 156 -> 156, 한글 주석 정상 디코드 |
| 편집 패턴 유일성 | 5개 패턴 각각 정확히 1회 매치 확인 후 적용 |
| ProjectAudit | **실패. 단, 변경 전 baseline에서도 동일 실패** |

빌드 경고 중 `C4819`(Shared 헤더 코드 페이지)와 `LNK4099`(DirectXTK PDB)는 이번 변경 이전부터
있던 것이다.

### 4.1 ProjectAudit 실패는 사전 존재

```text
Project audit failed (1): effect.g09-authoring-world-runtime-boundary:
  paths=0 authoredUnexpected=0 intake=2 shaders=1 symbols=0 project=0
  entry=False detailPreview=True
```

`git checkout -- Client/Private/MapTool.cpp`로 원본을 복원해 다시 실행했고 같은 항목에서
같게 실패했다. Effect Tool 저작/런타임 경계 검사이며 이번 MapTool 변경과 무관하다.
이 항목은 별도 담당 작업으로 남긴다. 확인 후 수정본을 복원했고 파일 크기 150,527 바이트,
non-ASCII 156으로 동일함을 재확인했다.

## 5. 수동 검증 (미실행 — 사용자 확인 대기)

Client는 GUI이므로 다음은 실행해서 눈으로 확인해야 한다. **아직 PASS로 기록하지 않는다.**

1. Lobby -> `Test` -> Server 승인 -> `LEVEL::DEVELOPMENT` 진입
2. F1 -> `Map Tool` -> Area 드롭다운에서 `Bern` 선택
3. 상태 줄이 `Active editor Area: Bern (LV_BER_BERNCASTLE) / 50017 placements`로 바뀌는지
4. Navigation 패널이 `Navigation bootstrap: place Nav Bounds and Bake` 상태인지
5. 회귀: Character Select / Valtan / Training Map 세 Area가 이전과 같이 열리는지

Area 선택 직후 창이 오래 무응답일 수 있다. 아래 6절 참고.

## 6. 알려진 남은 경계

### 6.1 Bern 동기 stage 부하

이번 수정으로 Bern이 관문을 통과하면 그때부터 실제 로드가 시작된다. 실측 페이로드는 다음과 같다.

```text
CharacterSelect   모델   55    7.8 MB     텍스처   108 MB /  160 파일
Valtan            모델  289   23.7 MB     텍스처   418 MB /  638 파일
TrainingMap       모델  302   15.0 MB     텍스처   429 MB /  741 파일
Bern              모델 1003   78.0 MB     텍스처 1,768 MB / 3537 파일
```

`Ensure_AuthoringPrototypes`가 이를 로딩 화면 없이 ImGui 스레드에서 동기로 올린다.
`.md/TEAM/AREA_DATA_LAYER_GUIDE.md` 4절의 "Area 선택 직후 창이 오래 응답하지 않을 수 있다.
중복 선택은 하지 말고 status가 commit될 때까지 기다린다"는 안내는 이번 수정 이후에야 실제
상황과 일치한다. 수정 전에는 stage에 도달하기 전에 거부됐으므로 기다려도 commit되지 않았다.
따라서 해당 문서 문장은 교정이 아니라 그대로 유효해진다.

비동기 로딩 또는 진행 표시는 별도 수직 슬라이스다.

### 6.2 `Load_RuntimeBlockers`의 동일 잠복 패턴

```text
MapTool.cpp  std::error_code runtimeError;
             const bool_t hasRuntime = std::filesystem::is_regular_file(
                 m_NavigationRuntimePath, runtimeError);
             if (runtimeError) { ... return false; }
             ...
             if (!hasRuntime) { ... return true; }   <- 도달 불가
```

`Switch_EditorArea` / `Load_NavigationDocument`와 같은 결함이다. 다만 `Load_RuntimeBlockers`는
현재 `Client` 전체에서 호출자가 0개이며 선언과 정의만 존재한다. Bern 진입 경로를 막지 않으므로
이번 승인 범위(3곳)에 포함하지 않았다. 호출자가 생기기 전에 함께 정리할 항목으로 남긴다.
AGENTS 규칙에 따라 미사용 함수 자체는 임의 삭제하지 않았다.

### 6.3 Bern navigation 데이터 자체

`Data/Navigation`에 Bern의 `.navsource` / `.navpaint`는 여전히 없다. 이번 수정은 "없어도 Map Tool을
열 수 있다"를 복구한 것이지 navigation을 만든 것이 아니다. 실제 bake와 Server 제품 navigation
활성화는 `AREA_DATA_LAYER_GUIDE.md` 4절대로 별도 변경 단위다.

## 7. 폐기한 초기 가설

기록 목적으로 남긴다. 둘 다 실행 증거 없이 코드 독해만으로 세운 가설이었고 실측에서 반증됐다.

1. **prototype tag 충돌**: `.mapassets` 원시 값에서 Character Select와 Bern이 태그 8개를 공유하고
   모델 경로가 달라 `Ensure_AuthoringPrototypes`가 거부한다고 추정했다. 반증: `Load_Source`가
   `MapAssetCatalog.cpp:217-221`에서 모든 태그에 `MapEditorArea:<areaId>:` 접두를 붙이므로 Area 간
   충돌이 발생하지 않는다.
2. **텍스처 1.77GB 과부하**: 동기 로드 중 실패 또는 장시간 정지로 추정했다. 반증: 상태 줄이
   navigation 관문 메시지였고 Visual Studio 출력 창에도 모델 로드 실패가 없었다. 실행이 그 단계에
   도달하지 못했다.

두 경우 모두 화면의 `m_Status`(`MapTool.cpp:592`)를 확인하기 전에 결론을 낸 것이 원인이었다.
