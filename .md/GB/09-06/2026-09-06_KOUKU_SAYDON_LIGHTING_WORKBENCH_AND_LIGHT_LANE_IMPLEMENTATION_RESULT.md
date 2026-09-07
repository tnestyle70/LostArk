# 2026-09-06 쿠크세이튼 조명 저작 구현 결과

## G00. 범위와 현재 상태

사용자가 요청한 Map / Character / Boss 조명 생성·저장, 기본 방향광 편집, Composition LIGHT lane,
Level별 Rendering Quality 저장을 구현했다. 최종 자동 검증 시각은 2026-09-07 KST다.
같은 작업의 [구현 계획서](2026-09-06_KOUKU_SAYDON_LIGHTING_WORKBENCH_AND_LIGHT_LANE_IMPLEMENTATION_PLAN.md)를 갱신했다.

작업 브랜치는 `codex/kouku-lighting-anchors`다. 시작 시 존재하던 다른 기능의 대규모 미커밋 변경을 보존했고,
자동 stage/commit/push는 하지 않았다. Client/UI를 실행·조작하거나 화면을 캡처하지 않았다.
구현과 아래 자동 검증은 완료했으며, 실제 UI 입력·저장·재진입과 아레나 조명 품질은 사용자 확인 전이다.

## G01. Rendering Workbench와 저장 정본

상단 `Level category`에서 Level별 설정을 선택한다. 쿠크의 기본 profile은 `scene.kakulsaydon.g1.base.v1`이다.
`Rendering Quality / Scene Profile` 탭의 Quality는 선택 Level의 base profile에 저장된다.
패턴의 임시 Scene Profile 전환과 복구는 Level Quality owner를 유지한다.

`Light Resources` 목록은 All / Map / Character / Boss로 분류하고, Create는 anchor와 광원 종류를 따로 받는다.
Map Point/Spot은 현재 플레이어 위치보다 8m 위에서 시작한다. Spot은 아래 방향, 바닥 원 반경, cone,
거리 감쇠, RGB와 밝기를 조절할 수 있다. 실제 무대 배치는 사용자가 결정한다.

| 항목 | 실제 저장 정본 | 동작 |
|---|---|---|
| Default Directional Light | `Data/Rendering/Authored/RenderingProfiles.json` | Map 목록에서 기존 Scene Profile 방향광을 편집·저장·재로드한다. 중복 방향광을 만들지 않는다. |
| Map 배치 조명 | `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.maplights.json` | formatVersion 2로 저장하고 Map publisher로 런타임에 배포한다. |
| 재사용 조명 | `Data/Rendering/Authored/LightResources.json` | Character/Boss 및 선택적인 MAP resource를 저장하고 전용 publisher로 배포한다. |
| 패턴 LIGHT 사용 | `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | 조명 ID, 시작·길이, anchor, offset·회전, 밝기 배율과 fade를 저장한다. |

기본 방향광은 기존 Rendering Profile 정본을 그대로 편집한다. 다른 Map 조명처럼 maplights에 복사하지 않는다.
기존 Valtan v1 Point 조명 22개는 그대로 읽고, v1 저작 저장은 거부한다.

초기 재사용 조명은 `light.runtime.1`의 `스포트라이트조명_캐릭터`와 `light.runtime.2`의
`스포트라이트조명_세이튼` 두 Spot이다. 각각 높이·cone을 다르게 둔 튜닝 시작값이다.
쿠크 Map 조명 원본은 빈 배열로 시작한다. 무대 Spot이 이미 배치되었다는 의미는 아니다.

조명 UI 구현은 `MainApp_RenderingLighting.cpp`로 분리했고 기존 MainApp 소유권을 유지했다.
새 H/CPP와 JSON 원본은 Client 프로젝트·filters의 필요한 항목에 등록했다.

## G02. LIGHT lane과 Engine Spot

Action Workbench의 쿠크 Light resource 목록에서 Map / Character / Boss 항목을 선택해
`Append selected Light`로 timeline에 추가한다. LIGHT box의 시작·길이와 anchor를 개별 편집한다.

- MAP은 고정 world 위치를 사용한다. Map 원본 조명도 같은 ID의 조회 항목으로 Append할 수 있다.
- Character는 살아 있는 복제 플레이어마다 조명을 적용한다.
- Boss는 해당 패턴을 실행하는 보스 entity와 선택 bone을 기준으로 한다.
- 종료·Reset·Level 퇴장 시 제출을 중단한다. 잘못된 row, 일시적인 anchor 부재와 예산 초과는 해당 조명만 생략한다.

Workbench는 저작 정의를, 제품 재생은 publish된 정의를 읽는다. Map 원본을 LightResources에 복제 저장하지 않는다.
Map 상시 조명과 같은 항목의 LIGHT box를 함께 켜면 밝기가 합산된다. 패턴에서만 켜려면 Map 항목의
`enabled`를 끄고 box에서 사용한다.

Engine에 SPOT과 deferred pass 16을 추가하고 기존 Point 거리 감쇠에 cone 감쇠를 결합했다.
기존 LIGHT_DESC 필드 offset을 유지하며 cone cosine을 뒤에 추가했다. Scene/transient 검증과 실패 보존을 연결했다.
광원 표현은 Client presentation이며 Server 전투 판정이나 protocol을 바꾸지 않는다.

Map draft preview는 창·탭 종료, Level 선택 변경, published reload 시 해제한다.
잘못된 저장 입력, 외부 원본 변경과 publish 실패는 이전 정상 상태를 보존한다.

## G03. 실행한 자동 검증

로그 위치는 저장소의 `out/LightingWork/`다. 빌드 산출물과 로그는 소스 커밋 대상이 아니다.

| 검증 | 실제 결과와 근거 |
|---|---|
| Debug Product | Engine → Shared → Server → Client 컴파일·링크 및 SDK/shader/runtime DLL 배포 PASS. `product-debug-final.log` |
| 최종 빌드 receipt | `out/BuildPipeline/runs/20260906T152624283Z-debug-product.json`; 모든 step PASS, missingRuntimeInputs 없음 |
| 기존 PointLight native harness | Debug 빌드·실행 PASS. Spot cone/pass, 혼합 Scene/transient, 잘못된 방향·cone rollback, Map v2 왕복·빈 배열·실패 보존, Valtan v1 22개·읽기 전용 검사. `point-light-harness-build.log`, `point-light-harness-run.log` |
| Compiled shader closure | Debug x64 PASS, producer 24 / Client consumer 23 / harness consumer 1. Engine/Client shader 동일성 확인. `shader-closure.log` |
| Rendering publisher tests | LightResources 9개 + RenderingProfiles 4개 = 13/13 PASS. `rendering-tests.log` |
| Composition projector tests | 48/48 PASS. `composition-light-tests.log` |
| Build domain receipt tests | 18/18 PASS. `domain-tests.log` |
| 실제 Composition Validate | sourceRevision 76, Product pattern 6, stage 66, output 2. `composition-light-projector-validate.log` |
| 실제 domain Publish | LightResources revision 1 / 조명 2개, RenderingProfiles, 쿠크 Map 3,231 placement / file 7개, Composition source revision 76 배포 성공 |
| 구조·공백 | 변경 JSON과 Client 프로젝트·filters XML parse, ProjectReference GUID 구조, `git diff --check` PASS |

Python 검사는 합계 79개다. Product 빌드 자체는 데이터 publish와 Client 실행을 하지 않으며,
위 domain 배포는 별도의 명시적 publisher 실행 결과다. Native/WARP 수치 검사는 사용자 화면 판정을 대체하지 않는다.
기존 C4819 인코딩 경고는 남아 있다. 최종 빌드 오류는 없다.

## G04. 사용자가 확인할 실행 경로

이 PC는 LAN 설정 검사에서 `server-host`였고 endpoint는 `192.168.0.4:7777`이다.
TCP 7777 LocalSubnet 방화벽 설정을 확인했다. 작업 종료 시 Server와 Client 프로세스는 실행되어 있지 않다.
Visual Studio의 `Server + Client` profile을 선택하고 사용자가 `Ctrl+F5`로 시작한다.

1. Lobby → `KoukuSaydon` → F1 → `Rendering Workbench` → `Level category: KoukuSaydon` → `Light Resources`로 들어간다.
2. Map의 `Default Directional Light`를 튜닝하고 `Save Light` → `Publish Light` → 재로드/재진입으로 값을 확인한다.
3. `Anchor type: Map`, `Light type: Spot`으로 `Create Light`를 누른다. 위치·아래 방향·바닥 원 크기·RGB를 조절하고
   `Save Light` → `Publish Light` → `Reload Published Map Lights`와 재진입으로 저장 결과를 확인한다.
4. Character/Boss의 초기 두 Spot을 각각 튜닝·Save·Publish한다. F1 → `Action Workbench`의 쿠크 Composition에서
   Light resource 항목을 선택하고 `Append selected Light`로 추가해 시작·길이·anchor를 설정한다.
5. 저작 preview를 확인하고 기존 Save → `Publish All PRODUCT` 절차 후 Debug Server를 재시작한다.
   `Play Published Product (Server)`로 실제 패턴의 조명 시간과 대상 추적을 확인한다.
6. `Rendering Quality / Scene Profile`에서 Level Quality를 Save·Publish하고 Scene Profile box 전후와 Level 재진입 후의 보존을 확인한다.

화면의 밝기·색·Spot 폭, UI 저장 왕복과 실제 패턴 lifetime은 아직 사용자 수동 검증 전이다.
modifier catalog, light group blend, Scene Profile 보간과 고급 조명 기법은 이번 범위에 추가하지 않았다.


## G05. 09-07 독립 창·Light Sequencer·세 패턴 조명

이 절이 사용자 검증 이후의 현재 상태다. Light Resources(왼쪽), Light Sequencer(아래),
Light Detail과 Rendering Workbench(오른쪽)는 독립 창이다. Windows 메뉴로 창 재열기/배치 초기화를 지원한다.
Create Light와 All Lights는 Resources에 모았으며 용도는 Map Profile / Scene Profile / Anchor Light다.
Anchor Light 하위는 Map/Character/Boss이고 Directional/Point/Spot은 별도 광원 종류다.
Map 기본 Directional은 Level base profile의 기존 광원만 편집한다. Scene 분위기와 조명은 Light Detail,
Level별 FXAA/SSAO/Bloom 품질은 Rendering Workbench에서 저장한다.

Light Sequencer는 선택 조명·Scene의 lifetime과 Play/Pause/Seek/Stop을 지원한다.
Scene preview는 이전 profile을 기억하고 Stop/종료 시 복구한다. Map preview는 복사 문서에서만 enabled를 켜며
저장 원본을 수정하지 않는다. 선택·Level 변경/도구 종료·published reload 시 임시 preview를 정리한다.
단일 Character 리소스 audition은 로컬 캐릭터를 사용한다. Composition preview와 Server Product는
같은 방 Server snapshot의 캐릭터 전원을 수집하며 사망 HP 조건으로 제외하지 않는다.

기존 ID는 보존했고 이름만 씬프로필_암전 / 스포트라이트_캐릭터 / 스포트라이트_세이튼으로 저장했다.
RenderingProfiles의 optional displayName은 strict type/UTF-8 길이를 검사하고, 기존 이름 없는 문서는 ID를 표시한다.
기존 두 Spot의 offset/rotation/range/cone/RGB/brightness와 사용자가 저장한 Rendering 품질·방향광 값은 보존했다.

| 패턴 | Scene Profile 및 PLAYER/BOSS Spot 적용 창 |
|---|---|
| 진짜 세이튼 찾기 KAKULSAYDON_G1_PATTERN_2 | 2007~26134ms, 기존 암전 시작·종료 유지 |
| 댄스타임 KAKULSAYDON_G1_PATTERN_6 | 0~31467ms |
| 룰렛 KAKULSAYDON_G1_PATTERN_7 | 0~33669ms |

PLAYER/BOSS row를 각각 하나씩 배치했다. 실제 PLAYER row는 방 캐릭터 수만큼 기존 transient light를 제출한다.
Server patternId/startTick/sequence와 같은 방 snapshot broadcast를 그대로 사용하므로 새 light packet은 없다.
기존 애니메이션·로직·소환·World 배치는 보존했다. Scene blendMs의 기존 즉시 전환 정책은 바꾸지 않았다.

| 실행한 자동 검증 | 결과 |
|---|---|
| 최종 Debug Product | PASS, out/BuildPipeline/runs/20260907T030125810Z-debug-product.json |
| Rendering publisher tests | 15/15 PASS; optional 한글 이름 왕복/legacy 및 invalid 이름의 이전 runtime 보존 포함 |
| Composition projector tests | 49/49 PASS; 3패턴 Scene/Spot 창 일치와 tracked Product 최신성 포함 |
| domain Publish | RenderingProfiles revision17, LightResources revision2(2개), Composition revision77(6 patterns/66 stages/2 outputs), Gameplay bootstrap 성공 |
| 변경 JSON parse | 9개 PASS; 사용자 baseline 대비 RenderingProfiles는 revision과 dark displayName만 변경 |
| 최종 구조·공백 | RenderingProfiles source/runtime semantic equality 및 git diff --check PASS; 변경 XML 없음 |
| 빌드·검증 로그 | out/ObjectLightWorkspace/product-debug-final.log, rendering-tests-final.log, composition-tests-final.log, *-publish.log |

사용자가 앞서 확인한 Character/Boss Spot preview 결과는 이전 버전 확인이며, 이번 독립 창과 3패턴 화면 PASS로
승격하지 않는다. Client/UI는 실행·조작·캡처하지 않았다. Server + Client profile의 Ctrl+F5로 다시 시작한 뒤
F1 Tools → Rendering Workbench → Light Resources의 세 카테고리와 Light Sequencer를 확인한다.
F1 Tools → Action Workbench에서 세 패턴을 선택하고 Play Published Product (Server)로 암전·세이튼·전원 Spot을 확인한다.
밝기·크기·저장 버튼 왕복·다인 화면 결과는 사용자 확인 대기다. 자동 stage/commit/push는 하지 않았다.


## G06. 09-07 Map 조명 생성 이름과 수명 구분

Map Profile의 기존 이름 입력은 다른 필드와 같은 한 줄 형식이었다. 사용자의 입력란 발견 문제를
반영해 Create 공통 Light name 라벨과 전체 폭 입력칸으로 바꾸고 공백 이름의 신규 생성을 막았다.
이름은 기존 Create → displayName → Save_Authored → maplights JSON 경로를 그대로 사용한다.
Map Directional은 기존 기본광을 선택하는 버튼/읽기 전용 이름으로 구분해 이름을 입력하고도
새 광원이 만들어졌다고 오해하는 흐름을 없앴다.

Create/목록/Detail은 Map Profile(persistent), Scene Profile(mood), Anchor Light(pattern)을 안내한다.
Map Profile은 Area의 enabled 배치로 유지하고 Save Light 뒤 Publish Light로 런타임에 배포한다.
Anchor Light의 Map(fixed world)은 Action Workbench box의 수명 동안만 고정 월드 위치에 생성한다.
두 경로는 저장 정본과 수명이 다르며 자동 변환하거나 중복 배치하지 않는다.

확인된 Preview 버그는 MAP을 BOSS가 아니라는 이유로 PLAYER 분기에 넣어 Play 시점의 플레이어
좌표를 복사한 것이었다. MAP Preview pivot을 identity로 고쳐 Composition의 MAP 처리와 일치시켰다.
Detail은 World position으로 표시하며 Place above player (+8m)를 명시적으로 누를 때만 위치를 복사한다.
PLAYER/BOSS 추적은 유지한다. 사용자가 만들고 있는 리소스와 dirty runtime JSON은 수정하지 않는다.

자동 검증은 Debug Product 컴파일·링크·배포 PASS(20260907T043238497Z-debug-product.json),
git diff --check PASS다. 위치 계산의 CPU 확인에서는 동일 localOffset에 서로 다른 두 플레이어
위치를 주어도 identity MAP 기준의 결과가 동일했다(map-light-position-check.json). 이는 실제
Client/GPU 재생 테스트가 아니다. 이름 저장의 기존 codec/호출 경로를 검토했고 데이터·XML 변경과
새 publisher는 없다. 빌드 로그는 out/ObjectLightWorkspace/map-light-preview-build.log다.
사용자 종료 후 빌드했으며 Client/UI 실행·입력·Save 버튼 왕복·화면 검증은 사용자 확인 대기다.

## G07. Complete Play 애니메이션 로드 수정과 무력화 V2 동기화

조명 기능에서 patternbindings root에 lightResourceRevision을 추가했지만, 애니메이션 소비자인
KoukuSaydonPresentationAssetService의 strict property 목록이 이전 형식만 허용했다. 생성된 문서 전체가
malformed로 거부되어 무력화·댄스타임·룰렛의 보스 애니메이션이 기본 idle에 머물렀다. World 룰렛은
독립 경로로 정상 실행돼 오브젝트만 움직였다. 로더가 현재/legacy root를 허용하고 optional revision을
positive uint32로 검사하게 수정했다. 누락된 Product action을 idle 성공으로 숨기지 않고 실패 진단을 남긴다.
실제 projector 출력과 strict reader의 root/row 계약을 연결한 회귀 검사는 수정 전 실패·수정 후 성공했다.

이펙트 작업자는 boss.kouku.disarm 그룹에 21개 child를 저장했으나 Composition은 예전 단일 방패 leaf와
yaw90/b_effectroot 변환을 유지했다. 전체 그룹을 21개 LEAF occurrence로 전개해 방패2·모으기3·블러1·
별line5·연기5·decal5를 Sequencer의 개별 Effect box로 연결했다. 그룹 자체를 중복 실행하지 않는다.
작업자의 Source V2 파일은 그대로이며 child asset/offset/rotation/scale/start를 사용한다.
21개×9 위치 계산에서 원본 그룹과 오차 0m였다. 사용자 Save revision78과 새 Pattern8을 보존하고
최종 Composition revision81 및 대응 Product로 publish했다.

방패 창은 5263~15947ms이며 별은 원본 그룹 내부 시작 offset을 이 창에 더한다. 방패와 늦은 decal은
이 창에 맞춰 수명을 조절했다. Composition Fade 0은 기존 Runtime의 -1 보존값, dissolve는 (-1,-1) 쌍으로
전달해 별의 원본 dissolveInEnd=.701을 유지한다. 양수 fade만 명시 override한다. 공용 EffectV2Runtime의
기존 zero/negative 계약은 수정하지 않았다. Particle LEAF는 native emitter 수명을 유지하며 표시 box만
최종 정리 시각을 소유한다. 연기5개는 emission .7초+최대 particle 잔향 .9초=1.6초로 배치했다.

방패 앞/뒤 중심은 [0,.5,0]/[0,.5,.5], yaw는 0/180도다. 같은 BOSS SECTOR 2개를 STAGGER Logic에 연결해
Server가 두 방향을 판정한다. 기존 Collider region 전송 경계를 재사용했다. 원본 메시 4,778 vertex의
외곽을 기준으로 반각35.868636도/전체71.737272도/Debug 반경2.148847m를 적용했다. 이는 texture alpha의
최종 가시 실루엣을 판정한 값은 아니다. followBoss=false 또는 잘못된 anchor/shape는 publish에서 거부한다.

| 실제 확인 | 결과 |
|---|---|
| Composition projector | 52/52 PASS; 21개 전개, 방패 연결, followBoss 거부 포함 |
| 바인딩 reader 회귀 | 실제 생성 문서 검사 1/1 PASS; 이전 reader로 실패 재현 |
| 기존 Effect occurrence/product | 9/9 PASS |
| Composition/Gameplay publish | PASS, sourceRevision81, 6 Product patterns/66 stages |
| 첫 Debug Product | PASS, 20260907T051604640Z-debug-product.json |
| 신규 Server 방패 검사 | 두 방향·회전·중심·35도 내부/40도 외부·종료 해제 PASS |
| Server 전체 contract | 8 FAIL: 기존 NPC fixture v7와 WorldBootstrap reader v8 불일치. HEAD에도 동일 코드이며 이번 수정 밖 |
| Client product 전체 source 검사 | 16 PASS/2 기존 FAIL: Level scene 기본값과 Add_GameObject 호출수 기대. HEAD에서도 동일 실패 |
| V2 source/물리 입력 | 171 leaf/29 group 검사 PASS. 전체 validator는 기존 Composition-only curtain_1 집계 누락으로 실패 |

구조화된 근거는 out/ShieldCounterAlignment와 out/WorldObjectMotionAudit에 있다. 기존 전체 검사 실패를
관련 기능 PASS로 숨기거나 무관한 fixture를 이번 기능에서 바꾸지 않았다. 물리 Resources·효과 source는
추가로 전달할 변경이 없고 Drive 업로드도 하지 않았다. 실제 보스 모션·방패/별의 방향·최종 타이밍은
새 Server + Client에서 Action Workbench의 무력화 시작·댄스타임·룰렛 Complete Play로 사용자 확인한다.
