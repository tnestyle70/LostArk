# 프로젝트 수정·병합 빌드 범위 축소 결과

## G00. 결론과 검증 범위

2026-09-12 시작, 09-13 검증. 큰 CPP의 컴파일 단위, 공용 헤더, 생성 재질 표, 셰이더 include와 생성기의 변경 범위를 줄였다. Engine/Shared/Server/Client x64 Debug의 정규 Product 빌드·링크·배포가 완료됐으며 기본 `Client/Bin/Debug/Client.exe`와 `Server/Bin/Debug/Server.exe`에 반영됐다.

**완료한 것은 변경 전파 범위 축소다. 모든 빌드 병목을 없앤 것은 아니다.** 변경 없는 빌드가 빠르다는 사실과, 무거운 입력을 바꾼 뒤 컴파일이 빠르다는 사실은 다르다. 공통 모델 셰이더의 큰 최적화 작업과 일부 큰 단일 함수는 남는다. 아래 시간은 해당 장비·입력·실행 조건의 측정이며 팀 전체 PC의 보장 시간이 아니다.

| 검증 | 결과 | 해석 |
|---|---|---|
| 기본 경로 Product 빌드·링크·배포 | PASS, 9.500초 | 앞서 C++/FX 컴파일을 끝낸 뒤 최종 링크·배포한 시간 |
| 변경 없는 Product 반복 | PASS, 4.486초 | OBJ/PCH/CSO/바이너리 쓰기 모두 0 |
| Codec DetailIo CPP 하나 저장 재현 | PASS, 10.191초 | 해당 OBJ 1개와 Client 링크, PCH/CSO 쓰기 0 |
| Artist 재질 표 하나 저장 재현 | PASS, 10.806초 | `Effect_ArtistMaterial.obj` 1개와 Client 링크, PCH/CSO 쓰기 0 |
| native shader leaf 변경 fixture | PASS | 관련 CSO 3개만 변경, 나머지 10개 보존 |
| 정규 Engine/Client FxCompile | PASS, 421.125초 / 488.005초 | 공용 입력 분리 후 필요한 FX를 재생성한 단계; 남은 무거운 비용 |

CPP/표 저장 재현은 **원본 bytes를 바꾸지 않고 마지막 저장 시간을 갱신**해 MSBuild의 실제 재컴파일 범위를 측정했다. 논리 변경의 실행 검증이나 이전 거대 CPP 대비 속도 배율로 해석하지 않는다. FX fixture도 대표 13개에 대한 범위 검증이며 전체 셰이더의 3개만 항상 바뀐다는 뜻은 아니다. 일부 격리 컴파일은 다른 검증과 병행했으므로 개별 `/c` 시간은 성능 A/B 결과로 사용하지 않는다.

## G01. 프로젝트와 대형 CPP

등록된 CPP는 Engine 78→79, Client 226→293, Shared 7→8, Server 31→82, 합계 342→462개다. 7개 큰 CPP를 115개 컴파일 단위로 나누고, 재질 표 owner CPP 8개와 PCH 생성 CPP 4개를 추가한 결과다. 파일 수 자체를 줄이는 최적화가 아니라 한 번의 수정이 컴파일하는 구현 범위를 줄이는 최적화다.

| 기존 owner | 이전 행 수 | 현재 CPP 수 | 분리 책임 |
|---|---:|---:|---|
| Effect_Tool | 34,576 | 13 | resource/detail/material/catalog/IO/playback/occurrence/history |
| Effect_DocumentRenderer | 23,178 | 12 | staging/preparation/catalog/material/geometry/particles/rendering/cache |
| MapTool | 16,724 | 13 | area/placement/world gameplay/navigation/destruction/cutscene |
| Animation_Tool | 16,289 | 12 | pattern/composition/skill binding/Valtan/Kouku/IO |
| GameRoom | 18,035 | 17 | admission/command/inventory/party/world/replication/boss/simulation |
| ServerGameplayContractTests | 31,302 | 35 | CLI dispatcher와 실제 테스트 suite, 공통 fixture |
| Effect_DocumentCodec | 15,023 | 13 | JSON primitive/material/detail/source/validation/upgrade/atomic IO |

작은 함수들을 임의로 흩뜨리지 않고 의미별 구현을 옮겼다. Private helper는 선언과 단일 정의로 공유하며, 큰 helper 구현을 각 CPP에 복사하지 않았다. 공개 상태·저장 형식·tick 순서·실패 처리·renderer GPU 계약은 유지했다. 신규 항목은 vcxproj와 filters에 등록했다. 기존 CPP 342개의 컴파일 metadata를 보존했고 분리본은 원래 `/bigobj`, `/utf-8` 등 파일 옵션을 이어받았다.

Server 테스트는 파일만 나누지 않았다. 약 27,870줄 worker의 실제 suite를 분리해 dispatcher를 약 1,651줄, 최대 suite를 약 3,367줄로 줄였다. 기존 free-function 테스트의 private 접근은 실제 Private runner class로 연결했고 일반 gameplay public API를 늘리지 않았다. 공용 runner header는 GameRoom/App/Session을 include하지 않는다. 이 타입의 실제 소비는 각각 26/7/9 CPP이며 skill fixture는 필요한 2개 CPP만 읽는다.

Client 도구의 함수·type 본문 1,166개, Codec 함수 199개, Server 테스트 원문 63개 범위를 보존했다. Server suite 호출을 원래 위치에 풀어 넣으면 worker의 토큰·문자열·순서가 재구성되고 호출·정의의 참조 인수도 일치한다.

## G02. PCH와 헤더 의존성

`Tools/Build/CppCompilation.props`와 `CppStandardPch.h`를 네 제품이 공유한다. PCH는 STL 15개 헤더만 포함한다. Engine/gameplay/authoring/재질 표와 Windows/vendor 헤더를 넣지 않아 기능 변경이 PCH를 무효화하지 않게 했다. 각 프로젝트·구성의 IntDir이 PCH를 소유한다. x64에서만 활성화하고 `LostArkUsePch=false`를 지원한다.

`/utf-8` 입력과 최적화·RTC 옵션이 다른 입력은 `NotUsing`과 빈 `ForcedIncludeFiles`로 PCH에서 제외한다. `/MP`는 기존 경로를 사용하며 기본 worker 수는 논리 CPU 수와 8 중 작은 값, 명시적 `CL_MPCount`/runner override가 우선한다. 기존 Release/Win32 조건과 metadata를 유지했으나 두 구성의 전체 제품 빌드를 실행한 것은 아니다.

`Engine_Defines.h`의 Assimp/DirectXTK/FX11/DirectInput/Engine_Struct 일괄 include를 제거했다. 초기화·렌더·애니메이션·vertex 타입을 각각 `Engine_InitTypes`, `Engine_RenderTypes`, `Engine_AnimationTypes`, `Engine_VertexTypes`로 나눴다. `Engine_RenderFwd`는 불완전 타입으로 충분한 API를 지원한다. `Engine_Struct`는 필요한 곳이 명시적으로 선택하는 호환 aggregate다.

| 실제 CL.read 의존 | Engine 이전 → 현재 | Client 이전 → 현재 |
|---|---:|---:|
| Engine_Struct 전체 | 71 → 0 | 217 → 0 |
| Assimp Importer | 71 → 1 | 217 → 0 |
| FX11 | 71 → 1 | 217 → 0 |
| DirectXTK | 71 → 9 | 217 → 16 |
| 신규 RenderTypes | 없음 → 8 | 없음 → 111 |
| 신규 VertexTypes | 없음 → 18 | 없음 → 136 |

CPP 분리로 표의 분모도 증가했다. Engine_Defines 자체는 여전히 71/283개, GameInstance는 33/162개에 연결된다. 이를 전체 의존성이 사라졌다고 해석하지 않는다. 현재 타입을 실제 소비하는 재컴파일은 필요하다.

Assimp 및 DirectXTK 포인터는 올바른 namespace에서 전방 선언하고 실제 멤버를 쓰는 CPP에 완전 타입을 include했다. CShader의 기본 복사 생성자·대입은 CPP의 `= default`로 옮겨 FX11의 완전 타입이 공용 헤더에서 필요하지 않게 했다. DirectInput의 POINT 및 Winsock 포함 순서, PCH의 `std::byte`와 Windows RPC 선언 순서도 실제 컴파일에서 확인해 소비자에서 바로잡았다.

## G03. 생성 재질 표

Artist, LanceMasterVA, Warlord, DimensionMaster Q/V/ALTV/WR/SD의 변경되는 큰 배열을 Private `*_Tables.inl`과 이를 한 번 컴파일하는 CPP로 옮겼다. public descriptor와 작은 inline 조회/검증 함수는 유지하고 표는 `extern const std::span`으로 제공한다. backing array와 span은 static storage/`constinit`으로 초기화 순서 의존을 만들지 않는다.

8개 public 헤더 합계는 **3,337,370→58,955 bytes**, Artist는 **1,760,576→9,626 bytes**다. 표는 여전히 존재하며 이제 한 owner가 컴파일한다. 큰 데이터가 사라졌거나 runtime lookup 방식이 달라진 것은 아니다.

실제 generator/installer는 `native_material_tables.py`를 통해 기존 형식 확장·분리 저장을 사용한다. 원문 재확장 bytes 일치, 재적용 write 0, 신규 프로그램 추가 시 INL만 변경, candidate 생성, malformed/stale 입력 거부와 교체 실패 시 기존 파일 복구를 검증했다. Q/WR public inline 함수가 쓰는 작은 상수 4개는 실제 컴파일에서 발견해 public에 유지하도록 writer와 출력을 함께 보정했다.

## G04. 셰이더 변경 범위와 남은 핵심 비용

4개 native dispatcher의 case를 group별 물리 include 42개로 분리했다. Warlord Group000 본문도 별도 include로 옮겨 공통 파일은 492,941→15,683 bytes다. 원래 guard·순서·함수·program ID를 보존했고 생성기는 바뀌지 않은 파일의 mtime을 유지한다.

SourceCharacter는 Engine 정본과 Client 복사본 모두 common/Base/Light로 분리했다. Base 수정과 Light 수정의 재컴파일 대상을 구분할 수 있다. FXC 전처리 94개의 token/order, 모델 pass별 include 경계, 대표 FX 13개 실제 컴파일과 변경 leaf의 실제 CSO 범위를 검증했다.

**이펙트 하나 추가가 셰이더 파일 하나 추가를 의미하지 않는다.** 기존 carrier/material profile을 사용하는 데이터 추가는 새 FX를 요구하지 않는다. native 프로그램을 확장하면 기존 ID group의 표·본문·case를 갱신하고, 새 group이 필요할 때 대응 carrier/project 등록이 필요하다.

다음은 아직 해결하지 않은 비용이다.

| 현재 무거운 입력 | 실제 구조 | 단순 분할만으로 해결되지 않는 이유 |
|---|---|---|
| Deferred SourceCharacter Light | 37-program 평가에 도달하는 PS entry 12개 | 광원 3종 × Legacy/instance × 일반/early-depth 실행 속성이 다름 |
| Animated Model Base | Base 37-program 평가에 도달하는 PS 3개 | alpha 정책이 달라 동일 엔트리가 아님 |
| Effect renderer preparation | 단일 함수 약 7,122줄 | CPP 분리 후에도 해당 함수의 컴파일 작업은 한 단위 |
| 큰 Workbench/Balance/기타 구현 | 전체 inventory에 크기·fanout 기록 | 크기만으로 시간 개선을 확정할 수 없고 실제 편집 경로별 측정 필요 |

Deferred 28 pass에는 compile 식 56개, 고유 엔트리 30개가 있다. 중복은 VS 두 종류이며 PS 28개는 서로 다르다. VS 공유는 미측정 후보일 뿐 12개의 큰 Light 최적화 작업을 제거하지 못한다. StaticModel은 이미 20 pass/고유 compile 9개, AnimModel은 9 pass/고유 compile 8개로 동일 compile 중복을 공유한다. 실제 명령은 `/T fx_5_0 /O1`이다.

일반 PS에서 native 평가를 제거해 Light 경로를 12→6개로 줄이는 변경은 현재 계약에 맞지 않는다. `Renderer.cpp`는 source mask 생성·그리기 실패 또는 MSAA 조건 불충족 시에도 source 재질을 그린다. 이때 `Light_Manager.cpp`는 일반 instanced pass 22~24를 선택하고 `Shader_Deferred.hlsl`의 row가 0이 아닌 분기가 native 조명을 처리한다. 일반 PS의 native 평가도 실제 fallback 소비자가 있으므로 단순 제거하면 해당 상황의 재질 조명이 깨진다. 이 후보는 읽기 전용으로 검토하고 코드 변경에서 제외했다. 421/488초는 프로젝트의 FX 단계 시간이며 개별 PS별 최적화 시간을 따로 측정한 값은 아니다.

다음 셰이더 설계는 재질 프로그램의 구현 family와 실행 pass별 특수화 경계를 줄이는 작업이어야 한다. 단순 CMake 교체, 분산 worker 추가, ZIP 배포만으로 이 FX 내부의 큰 최적화 작업이 분리되지는 않는다. 일반/early-depth 속성이나 alpha 정책을 임의로 합치거나 최적화를 끄면 기존 GPU 동작·성능 계약이 달라질 수 있으므로 이번 변경에서 그렇게 처리하지 않았다.

## G05. 팀 빌드와 검사 소비자

정규 `Product`는 컴파일·링크·배포 경로다. publisher, 광역 source/resource hash, 전체 계약 검사와 Client 실행을 필수 전제에 붙이지 않는다. `Core`/`FullDiagnostic`은 별도 선택 경로로 유지한다. 실행 ZIP에는 개발용 OBJ/PCH/tlog가 없으므로 C++ 증분 캐시로 설명하지 않는다.

runner는 실제 compiler 선택·추적 상태 변화·OBJ/PCH/CSO/바이너리 변경 수와 제한된 재빌드 이유를 기존 결과 JSON에 기록한다. 출력 크기/mtime 기반 쓰기 집계이며 실패한 compiler 시도 수를 뜻하지 않는다. 실패 빌드도 exit와 단계 결과를 보존한다.

단일 CPP 파일의 문자열만 읽던 검사와 generator 소비자는 `cpp_source_domains.py`를 통해 **프로젝트에 실제 등록된 동일 owner CPP와 Private _Internal.h**를 읽는다. 독립 Effect_Tool_V2 owner는 구분한다. 물리 파일 identity/옵션 검사는 원래 파일을 유지하고 함수 검사는 다음 함수의 파일 순서 대신 정확한 signature/brace 경계를 사용한다. 금지 API 소비자 검사도 분할된 실제 구현을 계속 검사한다.

| source 검사 | 현재와 원본 CPP 비교 |
|---|---|
| Client reader 36모듈 620 tests | 양쪽 44 fail + 18 error, 추가 실패 0 |
| pipeline 5모듈 74 tests | 양쪽 1 fail + 10 error, 추가 실패 0 |
| GameRoom 관련 234 tests | 비교 시 양쪽 16 fail + 2 error, 추가 실패 0 |
| source helper / active consumer | 5/5, 45/45 PASS |
| shader canary reader | 2/2 PASS |

기존 assertion을 삭제해 통과시키지 않았다. 위 기존 실패에는 현재 문서·opcode corpus·UI/API·protocol version 기대와 맞지 않는 항목이 있으며, 이 변경이 전체 저장소의 모든 기존 검사를 통과시킨다는 의미는 아니다.

## G06. 컴파일·실행 검증

- 정규 x64 Debug Engine/Shared/Server/Client Product Build, link, deploy PASS.
- 최종 project metadata 정리 뒤 Product 재실행 PASS. 네 제품 모두 OBJ/PCH/CSO/바이너리 쓰기 0, 프로젝트·filters XML 8개 parse 및 CPP 중복 등록 검사 PASS, `git diff --check` PASS.
- 정규 Engine/Client FxCompile PASS. 기존 X4000 경고와 vendor PDB LNK4099 경고는 오류와 구분한다.
- 분리 Client CPP 51개, Codec 13개, Server suite Debug 35개 실제 `/c` PASS. Server CLI/조건부 영역 Release 6개 `/c` PASS.
- 별도 절대 IntDir에서 PCH 없는 Engine GameInstance/Shader/Model/Mesh 4개와 material 8개, Engine 타입 헤더 6개 standalone PASS. 18개 fresh OBJ와 `/Yu`, `/Yc`, `/FI`가 없는 명령을 확인했다.
- headless 실행은 최신 Server.exe를 out에 복사하고 TEMP/TMP 및 Diagnostics를 격리했다. 일반 Server listener와 Client/UI는 실행하지 않았다. 운영 Data/runtime 파일 1,725개의 SHA 변경은 0개다.

Headless 결과: WorldPlayback/KoukuBundles/GroundTarget PASS. DebugTeleport의 21개 실패는 기존 실행 로그와 순서·중복까지 같다. 전체 contract는 1,295 PASS/13개 실패이며, 보존 원문 GameRoom/ServerGameplayContractTests 두 CPP로 분할 OBJ 52개만 대체하고 나머지 link 입력 31개의 SHA를 고정한 별도 baseline도 같은 결과다. 실패 문구·순서·중복이 모두 같고 분할 후 추가 실패는 0개다. 13개에는 원문 v7 fixture와 현재 v8~11 parser 불일치 8개 및 Kouku Product 5개가 포함된다. baseline 비교에서도 운영 입력 1,725개는 변경되지 않았다. 기존 실행 실패를 compile PASS나 visual PASS로 덮어쓰지 않는다.

Client 화면·effect fidelity는 사용자가 확인한다. 에이전트의 build/headless 결과는 시각 품질 판정이 아니다. 기존 쿠크 Composition Resources/Append 작업과 다른 미커밋 변경은 보존했으며 이 작업의 이름으로 섞어 자동 commit/push하지 않았다.

## G07. 증거 위치와 다음 판단 기준

- 계획: `2026-09-12_PROJECT_BUILD_ISOLATION_IMPLEMENTATION_PLAN.md`.
- 전체 파일 크기·현재/과거 include fanout: `out/ProjectBuildIsolation20260912/build-surface-inventory-final.json`, 같은 이름의 `.md`.
- 원문 보존: `cpp-body-preservation.json`, `server-tests-preservation.json`, `codec-structural-verification.json`과 각 분리 결과 문서.
- 정규 빌드: `out/BuildPipeline/runs/20260912T151859798Z-debug-product.json`, 무변경 반복 `20260912T151936909Z-debug-product.json`.
- 최종 재확인: `out/BuildPipeline/runs/20260912T154344901Z-debug-product.json`, `out/ProjectBuildIsolation20260912/final-checks.json`.
- 실제 저장 범위: `out/ProjectBuildIsolation20260912/cpp-incremental-probes.json`.
- 셰이더: `shader-result.md`, `shader-incremental-verification.json`, `product-fxcompile-result.json`.
- PCH 독립 확인: `nopch-final20260913/verification.json`.
- 생성기/검사: `material-fixture-result.json`, `source-reader-verification-summary.json`.
- headless: `headless-contract-run/results.json`, 원문 baseline 비교 `server-monolith-baseline/comparison.json`.

후속 최적화의 성공 기준은 파일 수 감소나 무변경 빌드 시간만이 아니다. 팀이 실제로 하는 CPP 편집, public header 변경, native material 추가, 공통 셰이더 수정, 새 PC/캐시 없는 빌드를 구분해 compiler/link/검사 시간과 재생성 대상을 측정해야 한다. 이번 결과는 첫 세 경로의 변경 범위를 줄인 근거이며, 공통 셰이더의 큰 컴파일 비용까지 해결했다는 근거는 아니다.
