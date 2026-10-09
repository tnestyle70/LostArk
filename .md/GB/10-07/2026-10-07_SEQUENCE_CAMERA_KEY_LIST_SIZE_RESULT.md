# Sequence Camera Tool 키 목록 높이 확대 결과

## G00. 반영한 표시 변경

`Client/Private/SequenceCameraEditor.cpp`의 `CSequenceCameraEditor::Render`에서
`BeginListBox("Keys", {-1.f, 125.f})`를 `BeginListBox("Keys", {-1.f, 500.f})`로 바꿨다.
세로 목록 영역은 정확히 4배이며 가용 폭·글꼴·개별 행 높이는 그대로다.

`ImGuiListClipper`와 stable key ID 선택, seek, 시간·포즈·FOV 편집, 추가·삭제,
validator와 commit 흐름은 바뀌지 않았다. 카메라 JSON, 영상, 자막 및 다른 세션의
미커밋 코드·문서는 이 변경으로 수정하지 않았다.

## G01. 실제 소비자와 스크롤

- `SequencerTool.cpp::Render_CameraWindow`의 `CameraKeys` child는
  `ImGui::BeginChild("CameraKeys", {0.f, 0.f}, ImGuiChildFlags_Borders)`로 만들어지며
  `NoScrollbar`나 `NoScrollWithMouse` 플래그가 없다. 커진 목록 아래 키 편집 항목과 Apply는
  기존 child의 세로 스크롤로 접근한다.
- `EffectAuthoringSequencer_Camera.cpp::Render_RecoveryCameraTool`은 일반
  `ImGui::Begin("Sequence Camera Tool##Recovery", ...)` 창 안에서 같은 편집기를 호출한다.
  스크롤 억제 플래그가 없어 내용이 넘치면 기존 창 세로 스크롤로 접근한다.
- 두 도구의 기본 창 크기와 사용자 배치 저장 파일을 건드리지 않았다. Sequence 저작 UI는
  기존 Debug 전용 노출을 유지하며 Release에 새 저작 도구를 추가하지 않았다.

## G02. 수행한 확인

- 원본과 적용본을 바이트 비교하여 변경이 `125.f` → `500.f` 한 곳뿐임을 확인했다.
- 기존 UTF-8 BOM 없음·CRLF를 유지했다.
- 대상 CPP와 PLAN/RESULT의 `git diff --check`가 exit0으로 통과했다.
- 기존 Client project/filter의 CPP 등록을 확인했다. 새 C++·JSON·XML 파일과 등록 변경은 없다.
- 원본 CPP SHA-256: `ee1c9cbff1c66e8dc01b64f7e9caccbf035915adf985812ef54ddab11112e74b`.
- 적용 CPP SHA-256: `a65d2e2fee934c2e507d0e54db865d969244b2c71aee161ec9036b03843c79c3`.

Debug/Release Product 빌드 완료 증거는 G04에 기록했다. 사용자 화면 확인은 별도다.

## G03. 사용자 화면 확인

Client/UI를 실행·조작하거나 화면 캡처하지 않았다. 사용자가 Debug의 기존
Action Workbench → World → Character Select에서 카메라 박스를 선택하고
Open Sequence Camera Tool을 열어 키 목록과 아래 편집 항목을 확인한다.
창이 짧으면 오른쪽 패널을 스크롤하거나 창 높이를 늘리면 된다.

## G04. Debug/Release Product 빌드 및 배포

사용자가 OpenShot 종료와 디자인·카메라 UI 반영, 두 구성 빌드를 요청한 뒤 정본
`Tools/Build/Invoke-BuildAndRegression.ps1`의 Product 빌드를 Debug → Release 순서로 실행했다.
두 구성 모두 실제 빌드이며 `skippedBuild=false`, 최종 결과 `PASS`다.

| 구성 | 전체 경과 | Engine | Shared | Server | Client |
|---|---:|---:|---:|---:|---:|
| Debug | 255.591s | 23.852s | 0.379s | 49.616s | 178.457s |
| Release | 56.626s | 0.583s | 0.320s | 0.515s | 53.421s |

Release 로그에서 `SequenceCameraEditor.cpp`의 실제 재컴파일과 Client 링크를 확인했다.
해당 Client 단계는 OBJ 1개·binary 1개를 갱신했고 PCH·CSO 재생성은 없었다.
두 구성의 Engine SDK 및 runtime DLL·compiled shader 배포와 Product의 필수 runtime 파일,
Navigation 참조, Item/Valtan 보상 catalog 확인이 완료됐다. 저작 데이터 publish는 수행하지 않았다.

현재 EngineSDK 헤더의 C4819 인코딩 경고가 남아 있고 Debug/Release 링크에는 기존 DirectXTK의
LNK4099 PDB 경고가 있다. 컴파일·링크 오류로 종료한 단계는 없다. 이 표시 높이 변경을 이유로
SDK 인코딩이나 외부 라이브러리를 변경하지 않았다.

빌드 증거:

- `out/BuildPipeline/runs/20261007T101432529Z-debug-product.json`
- `out/BuildPipeline/runs/20261007T101537557Z-release-product.json`
- `out/CameraKeyList20261007/build-debug.log`
- `out/CameraKeyList20261007/build-release.log`

제품 파일은 `Client/Bin/Debug/Client.exe`와 `Client/Bin/Release/Client.exe`에 반영됐다.
빌드 후 Client/UI를 자동 실행·조작하거나 화면을 캡처하지 않았다. 확대된 키 목록의 실제 화면과
아래 편집 항목 스크롤 확인은 사용자가 Debug의 기존 Sequence Camera Tool에서 수행한다.
