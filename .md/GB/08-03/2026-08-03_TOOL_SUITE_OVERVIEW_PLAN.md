# 2026-08-03 툴 스위트 개요 계획 (참조 영상 패리티 + 발전형)

이 문서는 참조 영상 스크린샷 9장(`C:\Users\user\Desktop\툴\01~09_*.png`)에 보이는
Effect 툴 / 렌더 타겟 디버그 뷰 / Map 툴 / 카메라 툴을 우리 프로젝트에서
"동일 기능 + 발전형"으로 구현하기 위한 상위 계획서다.
개별 수직 슬라이스의 상세 설계는 아래 문서가 담당한다.

- `2026-08-03_CHARACTER_SELECT_LEVEL_VERTICAL_SLICE_PLAN.md` — 학습 방식의 첫 수직 슬라이스
- `2026-08-03_CHARACTER_SELECT_IMPLEMENTATION_GUIDE.md` — 직접 구현·검토 순서
- `2026-08-03_RENDERTARGET_DEBUGGER_PLAN.md` — 렌더 타겟 전체 인스펙터
- `2026-08-03_CAMERA_TOOL_PLAN.md` — 카메라 컷 캡처/재생/저장 툴
- `2026-08-03_MAPTOOL_INTERACTION_PLAN.md` — MapTool 선택/기즈모/복제/콜라이더 확장
- `2026-08-03_EFFECT_TOOL_ADVANCEMENT_PLAN.md` — Effect 툴 패리티 마감과 게임플레이 적용

기본 작성 모드는 `.md/계획서작성규칙.local.md`의 STRUCTURE_FIRST다. 파일 책임 / H 계약 /
CPP 흐름 / 의존성을 먼저 설명하고 사용자가 구조를 작성한다. 사용자가 전체 코드를 명시적으로
요청한 슬라이스만 CODE_EXPLICIT로 전환한다.

## 1. 현재 체크포인트

2026-08-03 코드베이스 실측 결과(8개 영역 병렬 조사, 이 저장소 기준):

이미 있는 것 — 참조 영상과 달리 우리는 맨땅이 아니다.

| 영역 | 현재 상태 |
|---|---|
| 툴 허브 | 현재 코드는 scenario/catalog를 읽지만 CharacterSelect 수직 슬라이스에서 폐기한다. 최종 구조는 `CMainApp`의 `_DEBUG` F1 허브가 `DEBUG_TOOL` enum {NONE, MAP, ANIMATION, EFFECT, UI}를 직접 받아 Test(`LEVEL::DEVELOPMENT`)에서만 lazy 생성한다. 허브는 Level을 전환하지 않는다. |
| MapTool | `CMapTool`(약 3,100줄) 4개 모드: MAP_ASSETS(배치), WORLD_GAMEPLAY(spawn/npc/boss), NAVIGATION(bake/paint/blocker), CAMERA(follow/free 패널). 저장은 `Data/Maps/Authoring`·`Data/Worlds`, publish는 스크립트만 |
| Effect Tool | `CEffect_Tool` + `CEffect_Runtime` + `CEffect_AssetIO`(LOSTARK_EFFECT 5 텍스트 + .weffect 바이너리) + CPU `CEffect_ParticleSimulator`. 이미터 SPRITE/MESH/BEAM/RIBBON/ANIM_TRAIL, SubUV 시퀀스, 소프트 파티클, 왜곡/블룸은 디퍼드 체인이 처리. launch option 기반 open/HDR 자동 실행은 폐기하고 툴의 명시적 버튼으로 바꾼다. |
| 렌더 타겟 | 13개 타겟 / 7개 MRT가 태그 문자열로 등록(`Target_Diffuse`…`Target_BloomResult`). `_DEBUG` 셰이더 쿼드 썸네일 경로(`Ready_DebugDesc`+`Render_Debug`)는 있으나 현재 그림자 1장만 활성 |
| 카메라 | `CCamera_Free` 단일 클래스(follow/free, F6). 카메라 컷/경로/데이터 파일은 저장소 전체에 전무 |
| 입력 게이팅 | ImGui 캡처 → `CGameInstance::SetInputBlocked`, MapTool 전용 `ConsumesWorldLeftMouse` → 좌클릭만 차단 |
| 검증 | Client 내부 smoke harness는 폐기한다. Debug/Release build, 기존 NetworkProtocolHarness, Server `--contract-test`, ProjectAudit, Visual Studio Server+Client 수동 실행을 사용한다. |

없는 것 — 참조 영상 대비 갭이자 이번 설계의 대상.

| 갭 | 대응 슬라이스 |
|---|---|
| 렌더 타겟 전체 그리드 뷰(영상 03) — SRV 열거/접근 API 자체가 엔진에 없음 | RENDERTARGET_DEBUGGER |
| 카메라 컷리스트/포즈 캡처/재생/저장(영상 09) — 코드·데이터 모두 전무 | CAMERA_TOOL |
| 배치 오브젝트 클릭 선택, 트랜스폼 기즈모, Clone/사각 인스턴싱, 콜라이더 어소링(영상 04~08) | MAPTOOL_INTERACTION |
| DECAL 이미터, GPU 파티클 인스턴싱(현재 파티클 1개당 1드로우), 스킬→이펙트 게임플레이 적용(영상 01~02의 "적용" 부분) | EFFECT_TOOL_ADVANCEMENT |
| 배치 모델 애니메이션 미리보기(영상 05) — 맵 모델은 전부 NONANIM admission | MAPTOOL_INTERACTION 후속 단계 |
| QuadTree 컬링 툴(영상 08) — 현재 frustum + static batch 컬링으로 충분한지 측정 후 결정 | MAPTOOL_INTERACTION 후속 단계(측정 게이트) |

## 2. 전체 수직 흐름 한 줄

참조 영상 기능 인벤토리 → 제작자 시나리오에서 실제 소비자 확인 → 파일 존재 이유 →
H include/자료구조/함수 계약 → CPP 호출·상태 흐름 → 사용자 구현 → 실패 재현·대안 비교 →
필요한 등록만 수행 → Debug/Release 빌드 + 실제 Level 수동 검증 + ProjectAudit 검증.

## 3. 모든 슬라이스의 학습·설계 완료 계약

기능을 실행했다는 사실만으로 완료하지 않는다. 파일·클래스·함수마다 다음 질문에 답할 수
있어야 한다.

1. 왜 이 기능과 파일이 필요한가. 실제 제작자는 어느 왕복 시간을 줄이는가.
2. 헤더가 노출하는 타입은 무엇이며 각 include를 public에 둘 이유가 있는가. forward 선언이나
   CPP include로 결합을 낮출 수 있는가.
3. 각 자료구조가 표현하는 상태는 무엇인가. `array/vector/map/optional/variant/queue` 중 왜 현재
   형태가 맞으며 규모·검색·정렬·주소 안정성·저장 ID 측면의 대안은 무엇인가.
4. 함수의 직접 호출자, 호출 시점과 thread, 입력, 반환, 읽는 상태, 변경하는 상태, 외부 부작용,
   다음 소비자는 누구인가.
5. 정상 입력 하나가 UI에서 runtime 결과까지 어떤 순서로 이동하는가.
6. 어떤 불변식을 어느 함수가 세우고 어느 함수가 소비하는가.
7. 대표 실패를 어떻게 재현하며 parse/validate/stage/commit 중 어디서 실패하고 무엇을 rollback하는가.
8. 작은 요구사항 변경 한 건을 어느 파일과 함수까지만 고치면 되는가.
9. 다른 대안은 무엇이었고 현재 구조를 선택한 비용과 이점은 무엇인가.

각 신규 H/CPP 계획에는 최소한 다음 표를 둔다.

| 검토 대상 | 반드시 기록할 내용 |
|---|---|
| 파일 | 존재 이유, owner, 생성·파괴 시점, 호출자·소비자, 소유하지 않는 책임 |
| include | 필요한 symbol, public 노출 이유, forward 선언 가능 여부, 의존 방향 |
| 멤버 | 표현 상태, 초기값, 변경 함수, owner, 불변식 |
| 함수 | 호출자/thread, 입력·출력, 읽기·쓰기 상태, 부작용, 성공·실패 조건 |
| 자료구조 | 연산 요구, 선택 이유, 대안과 전환 조건 |
| 검증 | breakpoint, 관찰값, 실패 주입, rollback 결과, 작은 변경 연습 |

## 4. 참조 영상 ↔ 우리 계약 매핑 원칙

참조 영상의 구현을 그대로 이식하지 않는다. 다음 치환이 항상 적용된다.

| 참조 영상 방식 | 우리 계약 |
|---|---|
| 바이너리 `.data`/`.mesh` 저장 | JSON(`schema: lostark.<kebab>` + `formatVersion`) 또는 기존 텍스트 계약(LOSTARK_* 헤더). 신규 문서는 JSON |
| 툴이 런타임 폴더에 직접 저장 | 툴은 `Data/` authoring만 쓰고 publisher 스크립트만 런타임 문서 교체 |
| 임의 전역 단축키(F키 다수) | F1(허브)·F6(카메라)만. 나머지는 ImGui 버튼 |
| 툴 안의 자체 레벨 전환 | 툴은 Level을 전환하지 않는다. Lobby의 네 버튼만 `CLevelTransitionService::Request_Load(LEVEL)`을 사용 |
| 별도 모델/이펙트 런타임 | 기존 `CModel`/`CEffect_Runtime` 단일 경로 확장 |
| 저장 ID = 이름/인덱스 | stable ID(`^[A-Za-z0-9_.-]{1,128}$` 또는 uint64 도메인 분할) |

## 5. 기능 유형별 등록 절차

새 H/CPP를 만들었다고 catalog와 scenario까지 자동으로 추가하지 않는다. 먼저 실제 소비 방식으로
기능 유형을 판정한다.

| 유형 | 예시 | 필요한 연결 | 추가하면 안 되는 것 |
|---|---|---|---|
| 내부 제품 Level | CharacterSelect | `LEVEL`, Registry create/load, LEVEL-only transition, Loader, project/filter | CLI scenario, LevelCatalog 엔트리, MainApp parser |
| 기존 Level 내부 패널 | RenderTarget debugger, MapTool 하위 패널 | 기존 F1 허브/소유 툴에 command·view만 연결 | 신규 LEVEL·scenario·두 번째 runtime |
| 기존 Test Level의 제작 툴 | Map/Animation/Effect/UI/Camera | F1 허브의 `DEBUG_TOOL`, 기존 tool owner, 필요한 authoring data | 새 LEVEL, scenario, launch option, Client smoke harness |
| authoring 데이터만 추가 | Balance schema, physics branch 문서 | `Data/` 정본, validator/publisher, `96.DataFiles`, runtime consumer | 툴만 읽는 두 번째 정본 |

모든 유형에서 물리 H/CPP와 `Client.vcxproj`/`.filters` 등록은 일치시킨다. 신규 `Data/` 원본은
`96.DataFiles`의 `None` 항목으로만 노출한다. Client scenario/catalog/launch parser는 다시 만들지 않는다.

허브 소유 진단 토글(Profiler 패턴)로 충분한 기능은 이 절차 대신 `RenderDeveloperTools`
체크박스 하나로 끝난다. 렌더 타겟 인스펙터가 이 경량 경로를 쓴다(해당 문서 참조).

## 6. 공통 금지·주의 경계

- 제작 툴은 `_DEBUG` 전용 유지. Release Client 내부에 툴 smoke 분기를 넣지 않는다.
- Client executable에 scenario parser, 자동 UI 조작, 자동 종료, smoke report writer를 추가하지 않는다.
- ImGui 렌더 중 파일 읽기/모델 재디코드 금지. 로드는 parse → validate → stage → commit + rollback.
- `Engine/Public` 헤더를 바꾸면 UpdateLib.bat → Client 재빌드까지가 한 검증 단위.
- MapTool 계열 세계 클릭은 `ImGui::GetIO().WantCaptureMouse` 확인 + `ConsumesWorldLeftMouse` 스타일 중재를 거친다.
- 멀티 뷰포트가 켜져 있으므로(도킹+뷰포트) 렌더 상태를 바꾸는 draw callback은 반드시 원복한다.
- 로더 워커에서 모달 금지, 실패는 HRESULT/상태 문자열로 패널에 표시한다.

## 7. 구현 순서 권장

각 슬라이스는 독립 브랜치/독립 커밋 단위다. 권장 순서와 이유:

0. **CharacterSelect 수직 슬라이스** — Level/Registry/Loader/UI command/제품 진입 경계를
   직접 설명하고 구현한다. 새 scenario와 catalog를 만들지 않는 최소 연결을 첫 기준으로 삼는다.
1. **렌더 타겟 인스펙터** — 가장 작고, Engine 공개 API 추가 + UpdateLib 사이클을 한 번
   연습한다. 이후 Effect/Map 작업의 시각 디버깅 기반이 된다.
2. **카메라 툴** — 신규 파일 위주라 기존 코드와의 충돌면이 가장 좁다. 5절에서 실제 소비
   유형을 먼저 판정한 뒤 필요한 허브·데이터 등록만 완주한다.
3. **MapTool 상호작용(선택→기즈모→복제→콜라이더)** — 단계가 많아 내부에서 다시 슬라이스를
   나눈다. 선행 두 슬라이스에서 만든 픽킹/디버그 드로우 경험을 재사용한다.
4. **Effect 발전(DECAL→GPU 인스턴싱→게임플레이 적용)** — 게임플레이 적용은
   Data/Shared/Server/Client를 관통하는 가장 큰 슬라이스이므로 마지막에 둔다.

## 8. 검증 정책 (모든 슬라이스 공통)

- 빌드: Engine 변경 시 Engine → UpdateLib.bat → Client, 아니면 Client만. Debug x64 기준.
- 실행: `Framework.slnLaunch`로 Server+Client를 시작하고 Lobby의 Test로 진입한 뒤 F1 허브에서 해당 툴을 열어 수동 검증한다.
- 자동 계약: 기존 NetworkProtocolHarness와 Server `--contract-test`, ProjectAudit를 실행한다. Client 내부 harness는 추가하지 않는다.
- 오디트: `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` 통과. 신규 publisher를 만든 슬라이스는
  `-FailureAfterPromote` 주입 픽스처까지 추가한다.
- 완료 보고: 해당 슬라이스의 RESULT 문서에 실행한 검증만 기록한다(문서 작성 ≠ 완료).
- 학습 확인: 사용자가 호출 흐름과 불변식을 설명하고 작은 변경 한 건을 직접 반영한 뒤 다음
  슬라이스로 이동한다.

## 9. 다음 단계

1. CharacterSelect에서 잘못 추가한 scenario 제거와 최소 Level 연결을 사용자가 구현
2. 함수 단위 REVIEW_HINT → 빌드/실패 재현 → 작은 변경 연습으로 첫 학습 루프 완료
3. 렌더 타깃부터 동일한 템플릿으로 다음 수직 슬라이스 진행
4. 슬라이스 완료 시마다 RESULT 문서 작성 + 커밋(별도 `codex/` 또는 팀 브랜치)
