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
