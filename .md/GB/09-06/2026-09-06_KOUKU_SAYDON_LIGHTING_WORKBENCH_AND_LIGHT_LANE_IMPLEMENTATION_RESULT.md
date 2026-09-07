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
