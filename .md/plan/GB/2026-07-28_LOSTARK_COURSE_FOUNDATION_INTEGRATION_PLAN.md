# Session - 21일차 수업 프레임워크 기반 저장소·필터·런타임 자산 통합 로드맵
좌표: LA-F00~F08 · 축: C2 이동>계산, C4 수명은 선언된다
관련: 없음

> 문서 성격: 전체 방향을 고정하는 roadmap이다. 실제 source/project 변경은 F00, F01처럼 한 단계씩 별도 dated PLAN/RESULT와 완전한 파일별 변경안으로 연다.

## 1. 결정 기록

- 문제·제약: 현재 `Framework.sln`은 Engine DLL과 Client EXE를 직접 소유하는 21일차 레거시 MSBuild 프로젝트다. CMake를 같이 넣으면 같은 source 목록을 `.vcxproj`와 CMake에 중복 등록한다.
- 실패한 가장 단순한 대안: Winters의 `GenerateProjects.bat`만 복사할 수 없다. BAT는 Winters의 8개 프로젝트 메타데이터가 박힌 Python 생성기를 호출하는 껍데기다. Visual Studio filter를 사람이 편집하는 방식도 물리 폴더와 어긋난다.
- 메커니즘: MSBuild project의 설정·링크·비source item은 수동 정본으로 보존한다. 생성기가 관리한다고 표시한 `ClCompile/ClInclude` 영역만 물리 source tree를 정본으로 삼고 `.vcxproj/.filters`에 투영한다.
- 대조: Dungeons는 `.sln/.vcxproj/.vcxproj.filters`를 모두 Git 추적하고 filter 자동화가 없다. Winters는 이 세 파일을 추적하되 Python 생성기로 source/filter drift를 검사한다.
- 대가: 당장은 CMake 자동 탐색을 얻지 못한다. 대신 기존 수업 코드의 빌드·디버깅 흐름을 유지하고, clean clone과 파일 등록만 결정적으로 만든다.

## 2. 한 문장 방향과 중단선

> 21일차 OOP/Prototype/Clone 프레임워크를 실행 골격으로 유지하고, `Resource_LostArk`에 이미 존재하는 Winters W-format 모델 하나를 먼저 렌더한 뒤 Converter·ImGui·FMOD를 독립 slice로 붙인다. ECS와 CMake는 지금 추가하지 않는다.

현재 보이는 “필터·ImGui·FMOD·Converter·Profiler를 동시에 완성”하려는 흐름은 동시 보완 충동이다. 이번 1시간의 천장은 `F00 clean-clone build 계약 + F01 filter 계약`이다. 모델 렌더와 도구 통합은 각 단계의 실패 원인을 분리하기 위해 앞 단계의 증거가 있어야 연다.

## 3. 실측된 현재 상태

| 영역 | 현재 LostArk | 확인된 위험 | 결정 |
|---|---|---|---|
| 빌드 | Engine DLL + Client EXE, Client가 `EngineSDK/Inc`와 `EngineSDK/lib`에 의존 | `EngineSDK`는 ignore인데 solution에 Engine→UpdateLib→Client 순서가 없어 clean clone `/m` race | `BuildDebug.bat = Engine build → UpdateLib → Client build` |
| project/filter | source를 `.vcxproj`가 열거, `.filters`는 별도 수동 구조 | `Client/Default/Client.cpp`, RC/icon, HLSL, vendor header를 단순 scan이 지울 수 있음 | 설정/고정 item 보존, 관리 source block만 생성 |
| 렌더 | `CMainApp`이 DX11 device/context와 `GameInstance`를 구동 | 없음; 현재 기준선으로 사용 가능 | 기존 렌더 loop 유지 |
| 모델 | `CModel`이 런타임 Assimp로 FBX를 직접 파싱 | load 지연·Assimp runtime 결합 | 기존 prototype 흐름 아래 W-format reader로 교체 |
| runtime authoring | `CMainApp::Ready_Gara`가 Navigation.dat와 mask DDS를 실행 중 생성 | Client가 read-only runtime이 아님 | Converter/Tool 단계에서 밖으로 이동 |
| 준비된 자산 | `Resource_LostArk/03_RuntimePack/EldenSmoke/...`에 `.wmesh/.wmat/.wskel/.wanim/.dds` 존재 | 계획이 이를 무시하면 새 converter를 중복 제작 | 기존 pack으로 reader/render부터 검증 |
| ImGui | 없음, 입력은 WndProc가 아니라 DirectInput polling도 사용 | WndProc 전달만으로 game input capture 불가 | Engine service + `CMainApp` owner + DirectInput gate |
| FMOD | SDK와 코드 없음 | 실제 lib/dll 이름과 재배포 권한 미확정 | 별도 F07, `CONFIRM_NEEDED` 유지 |
| Profiler | 없음 | Debug 수치로 최적화 판단 위험 | 모델 render 이후 Release 계측에서만 도입 |

2026-07-28 `Framework.sln / Debug / x64` 실제 빌드는 exit code 0이다. FXC deprecation, 수치 축소 변환, third-party PDB 누락 경고가 있다. `pwsh.exe` 미발견 문구는 Visual Studio가 전역 import한 vcpkg applocal target이 PowerShell 7을 먼저 탐색한 뒤 Windows PowerShell로 fallback하는 과정이다. Client 실행 smoke는 아직 하지 않았다.

## 4. 정본과 파일 추가 규칙

### 4.1 정본 분리

```text
빌드 설정 정본
  Framework.sln
  */Default/*.vcxproj의 PropertyGroup, ItemDefinitionGroup, Import, 고정 item

물리 source 정본
  Engine/Public/*.h, Engine/Private/*.cpp
  Client/Public/*.h, Client/Private/*.cpp
  이후 AssetFormat/Public|Private|Tests, Tools/AssetConverter/Public|Private

생성 투영
  각 project의 관리 대상 ClInclude/ClCompile block
  각 .vcxproj.filters의 source filter/item

Git 제외 생성물
  .vs, x64, EngineSDK, exe/dll/lib/pdb/cso, out, _work
```

`.vcxproj` 전체가 생성물인 것이 아니다. 빌드 설정과 고정 item은 사람이 소유하고, 표시된 source ItemGroup만 생성기가 소유한다. `.filters`는 Git에 올리지만 전부 생성기가 소유하여 수동 편집하지 않는다.

### 4.2 생성기 보존·제외 계약

생성기는 다음만 바꾼다.

- scan root 아래의 `.h/.hpp/.cpp/.c`에 해당하는 `ClInclude/ClCompile` managed block.
- managed source에 대응하는 `.filters` item과 물리 경로 filter.
- 경로 대소문자와 정렬을 정규화해 두 번째 실행이 byte-identical하도록 한다.

반드시 보존한다.

- `Client/Default/Client.cpp`, `Client.h`, `framework.h`, `Resource.h`, `targetver.h`.
- `Client.rc`, `Client.ico`, `small.ico`, `ResourceCompile`, `Image`, `Text`, `None`.
- `Engine/Bin/ShaderFiles`와 `Client/Bin/ShaderFiles`의 `FxCompile/None` 및 shader metadata.
- 모든 PropertyGroup, ItemDefinitionGroup, Import, ProjectReference, custom build metadata.

scan에서 제외한다.

- 현재 `Engine/Public/Assimp`, `Engine/Public/DirectXTK`, `Engine/Public/Fx11` vendor tree.
- `Bin`, `Default`, `EngineSDK`, `ThirdPartyLib`, `.vs`, `x64`, `out`, `_work`.

새 target은 project shell과 solution 등록을 한 번 명시적으로 만든 뒤 생성기의 target descriptor 목록에 추가한다. F02에서 `AssetFormat` static library가 세 번째, `AssetFormatTests` console app이 네 번째, F05에서 `AssetConverter` console app이 다섯 번째 target이 된다. 생성기는 project 종류·link 설정을 발명하지 않고 각 target의 managed source block과 filters만 갱신한다.

### 4.3 팀원의 파일 추가 절차

```text
1. 물리 Public/Private의 올바른 폴더에 파일을 만든다.
2. GenerateProjects.bat을 실행한다.
3. GenerateProjects.bat --check를 실행한다.
4. BuildDebug.bat을 실행한다.
5. source와 함께 변경된 .vcxproj/.filters를 commit한다.
```

F01 완료 전 임시 절차만 `Add Existing Item`이다. `Add New Item`으로 임의 filter를 만들거나 `.filters`를 직접 편집하지 않는다.

## 5. 목표 물리 구조

현재 코드를 한 번에 이동하지 않는다. 새 소유권이 생길 때만 다음 목적지를 만든다.

```text
LostArk/
  Framework.sln
  GenerateProjects.bat
  BuildDebug.bat
  Tools/
    BuildSystem/                    project/filter source-block 생성·검사
    AssetConverter/                 W-format writer console target
  AssetFormat/
    Public/                         W-format POD/schema, reader/writer 계약
    Private/                        검증·직렬화 구현
    Tests/                          bad header/payload/range negative tests
    Default/AssetFormat.vcxproj     static library
  ThirdParty/
    DearImGui/                      pinned source + Win32/DX11 backend
    FMOD/                           include/lib/bin; 권리 확인 후만 배포
    Assimp/                         최종적으로 converter 입력 전용
    DirectXTK/                      DDS/WIC/runtime 보조
  Engine/
    Public/
    Private/
    Default/Engine.vcxproj
  Client/
    Public/
    Private/
    Default/Client.vcxproj
    Bin/Resources/                  runtime read root
  Content/
    Manifests/                      provenance/license/redistribution allowlist
  _work/                            추출·실험 중간물, Git 제외
  out/                              cook/build 중간물, Git 제외
```

`Engine/Public` 아래 vendor header 이동은 F00/F01에서 하지 않는다. baseline과 generator가 닫힌 뒤 별도 dependency slice로 이동한다.

## 6. 단계별 gate

### F00 — 저장소와 clean-clone build 계약

적용:

- `.gitignore`는 `.vs/x64/EngineSDK`와 컴파일 산출물을 제외하고 `.sln/.vcxproj/.filters`, source, 필요한 runtime data를 보존한다.
- `.gitattributes`와 provenance allowlist가 승인된 큰 파일만 LFS로 보낸다. Lost Ark 추출물·FMOD·vendor binary는 LFS 여부와 별개로 재배포 승인이 없으면 push하지 않는다.
- Assimp, DirectXTK, Effects11 등 기존 build dependency 각각을 `committed allowlist` 또는 `documented bootstrap` 중 하나로 판정한다. 어느 쪽도 아니면 clean-clone PASS가 아니다.
- `BuildDebug.bat`은 `vswhere.exe`로 MSBuild를 찾고 반드시 `Engine Debug x64 → UpdateLib.bat → Client Debug x64` 순서로 실행한다. `Framework.sln /m`은 clean clone 공식 진입점으로 사용하지 않는다.
- vcpkg를 사용하지 않으면 두 project에 `VcpkgApplocalDeps=false`를 명시한다.

예측과 검증:

| 변경 전 예측 | 명령/관찰 | PASS | FAIL |
|---|---|---|---|
| 빈 `EngineSDK`에서 solution `/m`은 순서 보장이 없다 | 새 clone에서 `BuildDebug.bat` | EngineSDK 생성 후 Client.exe 생성, exit 0 | include/lib 누락, 순서 race, nonzero |
| LFS pattern 누락 시 큰 파일이 일반 Git blob이 된다 | `git check-attr filter -- <승인된 대표 자산>` | `filter: lfs` | unspecified 또는 일반 Git |
| remote LFS가 정상이라면 새 clone에서 객체를 복원한다 | `git lfs pull`, `git lfs fsck` | 누락/손상 0 | missing/corrupt object |

수동 실행은 `Client/Default`를 working directory로 `../Bin/Client.exe`를 실행한다. 기존 Logo/GamePlay 화면이 뜨고 종료 시 crash가 없어야 한다. 별도 smoke PowerShell script는 만들지 않는다.

### F01 — project/filter managed source 자동화

적용:

- LostArk 전용 Python 생성기는 target descriptor, scan root, exclude root, managed block만 안다.
- `GenerateProjects.bat`은 Python 진입점 호출만 한다.
- `--check`는 미등록 물리 source, 사라진 source 등록, filter drift, managed block 밖의 source 중복을 실패시킨다.

예측과 검증:

```text
예측: 물리 Public/Private에 probe .h/.cpp를 만들면 생성 전 --check는 실패한다.
실행: GenerateProjects.bat
관찰: Engine/Client managed source와 물리 filter에 probe가 등록된다.
실행: GenerateProjects.bat && GenerateProjects.bat --check
PASS: 두 번째 실행 후 project/filter diff 0, BuildDebug.bat exit 0.
FAIL: RC/icon/HLSL/Client.cpp/vendor metadata 변경, duplicate include, 두 번째 diff 존재.
```

probe는 검증 뒤 별도 삭제 계획에 따라 제거하며, 기존 source를 희생하지 않는다.

### F02 — Winters W-format을 canonical AssetFormat으로 포팅

결정:

- 새 LostArk 전용 mesh binary를 만들지 않는다.
- Winters의 `.wmesh/.wmat/.wskel/.wanim`과 공통 WINT header를 v1 canonical contract로 포팅한다.
- `AssetFormat` static library 하나가 reader/writer/validation을 소유하고 Engine, `AssetFormatTests`, AssetConverter가 함께 링크한다. 같은 구현을 여러 project에 복사 컴파일하지 않는다.
- `AssetFormatTests` console target이 CPU-only 정상/오류 fixture를 실행한다. static library만 빌드한 상태는 F02 PASS가 아니다.
- texture는 `.wtex`를 만들지 않고 DDS/WIC 경로를 유지한다.
- 기존 v1 공통 16-byte header의 정본은 `magic/version/flags/content_size`다. v1은 little-endian 고정이고 현재 runtime pack의 `flags=0`을 그대로 지원한다. payload hash는 v1 파일 내부 계약이 아니며 provenance manifest의 SHA-256으로 관리한다. 파일 내 digest가 필요해지면 호환 flag를 추측하지 않고 v2 layout/version을 별도 계획한다.

예측과 검증:

```text
정상: Resource_LostArk runtime pack의 header/section/count/path를 CPU-only inspect test가 읽는다.
거부: bad magic, unknown version, byte-swapped header(InvalidMagic/InvalidHeader),
      truncated payload, content_size mismatch, vertex/index/bone/clip count 범위 초과,
      section offset overflow.
PASS: 정상 fixture 통과 + 각 corrupt fixture가 명시적 error code로 실패.
```

### F03 — 준비된 runtime pack으로 bind-pose 모델 한 개 렌더

자동/팀 검증과 로컬 Lost Ark 시각 검증을 분리한다.

- 자동/팀 입력: `AssetFormatTests --write-smoke-fixture <out-path>`가 procedural triangle W fixture를 생성한다. CPU test는 reader를 검증하고 F03 Client가 그 파일을 읽어 GPU upload/render를 검증하므로 외부 자산이 필요 없다.
- 로컬 시각 입력: task-specific 환경 변수 `LOSTARK_LOCAL_ASSET_ROOT`가 `Resource_LostArk`를 가리킨다. 기본 개인 PC 절대 경로를 코드나 project에 넣지 않는다.
- 현재 로컬 값의 대상은 `03_RuntimePack/EldenSmoke/Character/SK_GSC_BST_00` 한 개다. provenance 승인 전 저장소에 복사하거나 push하지 않는다.

```text
sk_gsc_bst_00.wmesh
sk_gsc_bst_00.wmat
textures/sk_gsc_cha_00_d_2.dds
  → AssetFormat reader
  → Engine GPU vertex/index/material/texture
  → 기존 Client Loader의 prototype tag
  → 기존 Scene/Clone
  → bind-pose 모델 1개
```

기존 PrototypeManager/Loader/Clone은 유지한다. FBX/Assimp fallback은 이 모델의 W-format parity를 확인할 때까지만 남긴다.

필수 팀 PASS는 procedural W triangle이 기존 Scene/Prototype 경로로 렌더되는 것이다. 사용자 로컬 목표 PASS는 `LOSTARK_LOCAL_ASSET_ROOT`의 실제 Lost Ark mesh silhouette, material texture, winding/좌표축이 확인된 캡처다. 파란 빈 창, placeholder cube, FBX fallback은 로컬 목표 실패다. 로컬 proprietary pack 부재는 팀/CI 실패가 아니라 명시적 `LOCAL_ASSET_UNAVAILABLE`이다.

### F04 — skeleton과 animation 한 clip

`sk_gsc_bst_00.wskel`과 `sk_transformationattack.wanim`을 같은 모델에 연결한다. bone parent/range, bind/inverse bind, track key range, duration/TPS를 검증하고 한 clip을 재생한다.

PASS는 bind pose가 무너지지 않고 animation이 반복되며, missing clip 시 crash 대신 명시적 load failure가 관측되는 것이다.

### F05 — AssetConverter parity

Converter는 독립 Console target이며 Assimp 입력을 소유하고 Client를 include하지 않는다. F02의 `AssetFormat` writer를 링크해 동일한 W-format을 쓴다.

```text
동일 normalized FBX
  → AssetConverter
  → .wmesh/.wmat/.wskel/.wanim
  → F02 inspect/negative tests
  → F03/F04 runtime loader
```

PASS는 같은 입력과 옵션에서 출력이 byte-identical하고 실제 렌더가 유지되는 것이다. 원본 normalized FBX와 converter option provenance가 확보된 pack만 section/count/material/animation semantic parity까지 요구한다. provenance가 없는 기존 pack은 inspect 가능한 section/count와 시각 parity까지만 비교하며, 단순 hash 출력만으로 통과시키지 않는다.

### F06 — Dear ImGui DX11/Win32 shell

소유권을 다음으로 고정한다.

- Engine은 `CImGuiService` 타입과 Win32/DX11 backend wrapper를 소유한다.
- Client `CMainApp`이 service 인스턴스를 단독 소유한다.
- `CMainApp::Initialize`: `Initialize_Engine` 성공 뒤 ImGui 생성.
- `CMainApp::Update`: ImGui BeginFrame 뒤 현재 capture state를 Engine input gate에 전달하고 game update.
- `CMainApp::Render`: `Render_Begin` 뒤 game render와 `Framework Status` panel을 구성하고, ImGui draw를 `Render_End/Present` 직전에 수행.
- `CMainApp::Free`: `Release_Engine` 전에 ImGui backend/context를 역순 종료.
- Client WndProc는 backend header/API를 직접 include하지 않고 `CImGuiService::HandleWindowMessage(...)`만 호출한다. service가 초기화 여부를 확인한 뒤 `ImGui_ImplWin32_WndProcHandler`에 전달한다.
- `CGameInstance`/`CInput_Device`의 DirectInput 조회는 `WantCaptureKeyboard/Mouse`가 true인 장치 입력을 neutral state로 반환한다.

DX11이므로 core와 `imgui_impl_win32`, `imgui_impl_dx11`만 포함한다. docking, multi-viewport, DX12, 한글 폰트, editor panel은 제외한다. 별도 F06 구현 계획은 Winters ImGui guide의 작업 계약, wireframe, empty/error state, 실제 화면 캡처를 포함한다.

PASS는 `Framework Status` 창이 모델 위에 표시되고 UI 조작 중 카메라 WASD/마우스가 움직이지 않으며, UI 밖에서는 기존 입력이 동작하고 종료 crash가 없는 것이다.

### F07 — FMOD Core shell

`CONFIRM_NEEDED`: 실제 사용할 FMOD SDK 버전, Debug/Release import library와 runtime DLL 이름, 팀/포트폴리오 저장소 재배포 조건.

확인 후 Engine Sound service가 `System_Create → init`, 매 frame `update`, 종료 시 sound/channel release → `close → release`를 소유한다. 부분 초기화 실패도 생성된 자원만 역순 정리한다. Client exe 옆 DLL 배치는 PostBuild가 담당한다. focus loss 정책은 master mute, focus gain 시 이전 master volume 복원으로 고정한다.

PASS는 BGM loop 한 개, 중첩 SFX 한 개, focus loss 정책, 종료/재시작을 확인하고 missing DLL/audio에서 명시적 초기화 실패가 나는 것이다.

### F08 — Tool과 profiler

- `Ready_Gara`의 Navigation/DDS 생성을 별도 Tool command로 이동하고 Client runtime을 read-only로 만든다.
- Map Tool은 cooked map manifest를 저장하고 Client가 같은 manifest를 읽는다.
- Tracy는 Release+profiling 구성에서 frame/update/render/converter stage를 측정할 때만 연다.
- Shader Tool, AI, Boss, Player, UI 골격은 실제 소비 계약이 생긴 뒤 각각 별도 vertical slice로 연다.

## 7. Git/LFS와 재배포 경계

현재 ignore 적용 뒤 candidate는 544개/약 325.88 MiB다. Git LFS는 설치되어 있지만 `.gitattributes`가 없다.

- allowlist manifest는 각 binary asset/vendor 항목에 origin, license/terms pointer, redistributable 여부, SHA-256, repo 상대 경로, 승인자를 기록한다.
- `.fbx/.dds/.png/.tga/.jpg/.wav/.mp3` wildcard만으로 push를 허용하지 않는다. allowlist 승인과 LFS attribute를 둘 다 통과해야 한다.
- FMOD SDK, Lost Ark 추출물, vendor lib/DLL은 “LFS에 들어감”과 “배포 가능”을 분리한다.
- `.exe/.pdb/.cso`, bundled viewer, `_work` 원본 전체는 Git/LFS 모두 제외한다.
- 초기 push gate는 `git check-attr`, `git lfs ls-files`, `git lfs fsck`, 별도 새 clone의 `git lfs pull`, `BuildDebug.bat`, 수동 Client 실행이다.

## 8. 전체 의존성 우선순위

| 의존성 | 역할 | 도입 단계 |
|---|---|---|
| DX11/DXGI/D3DCompiler | 현재 renderer | 기존 |
| DirectXTK | DDS/WIC와 기존 보조 | 기존, 위치 정리는 후속 |
| Assimp | 현재 runtime FBX, 최종 converter 입력 | F05 후 runtime 제거 |
| AssetFormat | W binary reader/writer 정본 | F02 |
| Dear ImGui Win32/DX11 | debug UI/tool shell | F06 |
| FMOD Core | BGM/SFX | F07, SDK/권리 확인 후 |
| Tracy | Release profiling | F08 이후 |
| Lua/FlatBuffers/DX12/ECS | 현재 모델 렌더 목표에 불필요 | 도입하지 않음 |

## 9. 오늘 실행 순서

1. `.gitattributes`의 LFS 확장자 기준은 초기 baseline commit 전에 반영했다. F00 별도 PLAN에서는 allowlist와 `BuildDebug.bat`의 전체 파일 내용을 먼저 보여준다.
2. 사용자가 직접 반영하거나 명시적으로 구현을 요청한 뒤 clean-clone build/run을 닫는다.
3. F01 별도 PLAN에서 LostArk 전용 source/filter 생성기의 전체 파일 내용을 보여준다.
4. F01 검증·commit 뒤 F02 W-format reader로 이동한다.

현재 baseline commit 준비에서는 `.gitignore`와 `.gitattributes`만 반영했다. BAT/Python, AssetFormat, renderer, ImGui, FMOD, Converter는 구현하지 않았다.

## 10. 서브 에이전트 비평

### 1차 비평 — `/root/w05_model_roadmap_critique`

판정: FAIL, P0=0, P1=7, P2=2.

| 지적 | 처분 | 반영 |
|---|---|---|
| EngineSDK를 ignore하면서 solution `/m` clean clone 순서가 없음 | 수용 | F00을 Engine→UpdateLib→Client `BuildDebug.bat` 계약으로 교체 |
| `.vcxproj` 정본과 생성 범위 모순, 고정 item/vendor 위험 | 수용 | §4에 managed block, 보존 item, 제외 root, target 추가 절차 명시 |
| 준비된 W runtime pack보다 ImGui/FMOD/Converter가 앞섬 | 수용 | F02~F04를 W reader→모델→animation으로 재정렬 |
| ImGui owner와 DirectInput capture 미정 | 수용 | F06에 owner, 정확한 frame/shutdown hook, DirectInput neutral gate 고정 |
| AssetFormat reader/writer 정본이 미뤄짐 | 수용 | F02부터 canonical Winters W-format static library로 고정 |
| LFS가 재배포 권리를 대신함 | 수용 | allowlist manifest와 clean-clone LFS gate 추가 |
| roadmap에 구현 계획 수준 파일/검증이 부족 | 수용 | 문서 성격을 roadmap으로 제한하고 단계별 별도 PLAN/RESULT, prediction/PASS/FAIL 명시 |
| FMOD 실물 미확인 | 수용 | F07을 `CONFIRM_NEEDED`로 유지 |
| 포맷 negative test 부족 | 수용 | F02에 corrupt/range/overflow 거부 항목 추가 |

### 2차 비평 — `/root/w05_model_roadmap_critique`

판정: FAIL, P0=0, P1=4, P2=3.

| 지적 | 처분 | 반영 |
|---|---|---|
| AssetFormat test 실행 target 없음 | 수용 | `AssetFormatTests` console target과 CPU-only gate 추가 |
| v1 header에 없는 hash/endian 검증 요구 | 수용 | v1 실제 필드·little-endian·flags=0 계약, SHA-256은 manifest로 교정 |
| F03 개인 절대 경로로 팀 재현 불가 | 수용 | procedural fixture 필수 gate와 `LOSTARK_LOCAL_ASSET_ROOT` 로컬 gate 분리 |
| Client가 ImGui backend를 직접 호출해 wrapper 경계 누수 | 수용 | Client는 service의 `HandleWindowMessage`만 호출하도록 수정 |
| 기존 vendor dependency 획득 정책 부족 | 수용 | F00에서 dependency별 committed/bootstrap 판정 강제 |
| converter option provenance 없는 pack parity 과다 요구 | 수용 | provenance 있는 pack만 semantic parity, 없으면 inspect/visual parity로 제한 |
| FMOD focus loss 정책 미정 | 수용 | focus loss master mute, gain 시 이전 volume 복원으로 결정 |

### 3차 비평 — `/root/w05_model_roadmap_critique`

판정: PASS, P0=0, P1=0, P2=2.

| 지적 | 처분 | 반영 |
|---|---|---|
| 물리 source 정본에 `AssetFormat/Tests` 누락 | 수용 | §4.1 scan root에 Tests 추가 |
| F02/F03 exact test command와 fixture/material 정책은 후속 PLAN에서 필요 | 수용 | roadmap 경계를 유지하고 F02/F03 별도 PLAN의 필수 결정으로 고정 |

accepted/held P0/P1 잔여 0. 로드맵 비평 게이트 통과.
