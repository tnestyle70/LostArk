# F1 Complete Play 아레나 보존과 Workbench 저작 구현 계획서

## 목표

F1 Developer Tools에서 등록된 발탄 패턴을 스크롤 목록으로 선택하고 `Complete Play`를 누르면,
현재 Server가 확정한 벽·Debris·collision·Nav 상태를 그대로 보존한 채 보스와 해당 패턴만 다시 시작한다.
동시에 Action Presentation Workbench의 기존 joined view를 실제 정본 데이터에 연결해, 이미 지원하는
패턴 gameplay 수치 저장과 presentation sequence 저작의 완료·미완료 경계를 명확히 한다.

## 시작 상태 실측

- F1은 `CBossTool::Get_ServerPatternOptions`로 Product pattern inventory를 이미 읽고,
  `CBossTool::Play_ServerPattern`으로 Server typed audition을 제출한다.
- 현재 F1 선택 UI는 dropdown이며 상시 보이는 스크롤 목록은 아니다.
- `PLAY_PATTERN_ID`의 Valtan Arena 분기가 `Reset_ValtanAuditionState`를 호출해 World destruction,
  Encounter props, collision, Nav blocker까지 Fresh로 되돌린다.
- Character Select에는 이미 보스·attachment·combat object만 교체하는
  `Reset_ValtanBossOnlyAuditionState`가 존재한다.
- Workbench는 pattern/stage, collider/hit/push/knockdown, combat object, animation occurrence,
  Effect/Sound/Camera/World lane을 이미 결합해 표시한다. gameplay stage 수치는 Balance Tool의
  Product save 경계로 저장하지만, Product animation occurrence 자체는 아직 읽기 전용이다.
- Effect Tool의 `Play All`과 `Solo`는 미저장 element까지 확인하는 local authoring preview다.
  stable pattern ID가 있는 Server 재생은 각 Tool의 별도 `Complete Play`가 담당한다.

## G00. 실패를 먼저 고정하는 Server 회귀

Server gameplay contract에서 `BOTH_SIDES_BROKEN` 아레나를 만든 뒤 `PLAY_PATTERN_ID`를 제출한다.
요청 전후의 destruction group exact state, navigation revision, collision revision을 비교하고,
보스 pending pattern만 선택 ID로 바뀌는지 검사한다. 현재 구현에서 실패하고 수정 뒤 통과하는
동적 회귀를 완료 조건으로 사용한다.

## G01. Complete Play reset 경계 교정

`PLAY_PATTERN_ID`는 Valtan Arena와 Character Select 모두 기존 boss-only reset을 사용한다.
보스 상태, boss attachment, boss-owned combat object, audition lifecycle은 초기화하되 다음 상태는
절대 변경하지 않는다.

- ordinary/outer wall과 floor destruction group
- Debris/Encounter prop active state
- Server collision active state와 revision
- Server Nav blocker active state와 revision

이전 패턴의 Debug pillar 예약 상태는 boss-only reset에서 함께 해제해 새 패턴으로 새지 않게 한다.
Fresh/Phase 2/3시/9시/양쪽 preset은 명시적으로 눌렀을 때만 기존 full arena transaction을 사용한다.

## G02. F1 패턴 스크롤 선택

기존 shared selected pattern과 Server inventory를 유지하고 dropdown을 고정 높이 스크롤 목록으로 바꾼다.
`ImGuiListClipper`로 전체 등록 패턴을 비용 없이 표시하고 선택 행을 유지한다. `Complete Play`는 선택된
semantic pattern ID만 서버에 제출하며 arena preset을 암시적으로 호출하지 않는다.

## G03. Workbench 실제 저작 경계

Workbench는 임의 JSON 편집기가 아니라 Product 정본의 typed inspector다.

- gameplay/balance: 기존 stage duration, collider, hit schedule, push/knockdown 등 지원 필드를 표시·저장한다.
- animation sequence: 현재 Product occurrence와 clip binding을 pattern/stage/action/occurrence stable ID로
  선택·편집하고, parse -> validate -> stage -> commit/publish 경계가 존재하는 필드만 저장한다.
- presentation: Effect V1/V2, Sound, Camera/Shake, World/Combat Object는 joined lane과 정확한 owner Tool
  deep-link를 유지한다. 별도 정본 저장 API가 없는 필드는 읽기 전용이라고 UI에 표시하고 가짜 저장을 만들지 않는다.
- 한 번의 사용자 `Save`는 지원하는 domain validator와 publisher/apply를 내부에서 실행하며 부분 저장 실패 시
  기존 Product 상태를 보존한다.

Product animation occurrence 저장 계약이 현재 코드에 없다면 이번 변경에서는 먼저 해당 계약의 최소 수직
슬라이스와 회귀를 추가하고, 원본 extraction/reference 문서를 직접 덮어쓰지 않는다.

세부 typed slice는 다음을 포함한다.

- `RELEASE_GRABBED_PLAYERS`: speed/duration과 별개의 `yawOffsetDegrees`를 Source, Product,
  Server bootstrap/runtime, Workbench draft 전체에 연결한다.
- Sector Effect: Server가 선택한 `arena.center.facing`은 유지하고 cue의 local Y rotation만 stable
  occurrence ID로 수정한다.
- High Jump: AIRBORNE stage duration과 alive player당 도끼 수를 typed gameplay draft로 저장한다.
- 버러지 잡기: `CATCH_PRE_IMPACT`의 실제 추출 Shot3 cue가 세 Product pattern에 모두 결합되는지 검증한다.
- Esther: Server snapshot의 action edge를 identity로 사용해 player `ESTHER_CAST`와 Esther NPC strike의
  최소 근거 cue만 재생한다. exact notify 근거가 없는 나머지 WAV를 임의 타이밍에 연결하지 않는다.

## G04. KakulSaydon 추출 액션 시퀀스 저작

쿠크세이튼은 물리 WModel과 추출 action 근거는 있지만 Server Product world/pattern은 아직 없다. 따라서
Valtan Complete Play를 복제하지 않고 다음 두 문서를 분리한다.

- `Data/Animation/Reference/KakulSaydon/*.actionreference.json`: generator가 만든 immutable
  `REFERENCE_ONLY` action/stage/slot 기본 sequence, 한글 표시명, physical-model join과 holdout 증거.
- `Data/Animation/Authored/KakulSaydon/*.actionbindings.json`: 사용자가 바꾼 exact slot만 기록하는
  `PROJECT_AUTHORED` sparse override. 초기 문서는 비어 있다.

Resource Files에서 프로필 문서나 물리 Character를 선택하면 같은 Workbench가 정확한
`MN_RPCT_05/06/07/RPCZ_00` profile을 받고, 07은 실제 `MN_RPCT_05` body를 사용한다. Workbench는
한글 action -> stage -> slot을 나열하고, reference default를 즉시 채워 보여 주며, 현재 WModel clip을
선택해 timing/loop와 함께 원자 저장한다. Load/Save는 stale revision, 잘못된 identity, 중복, 없는 clip,
잘못된 수치에서 기존 메모리와 파일을 보존한다. 이 화면과 버튼은 `Local Extracted Action Preview`이며
Kakul Server Product pattern으로 표시하지 않는다.

## G05. 검증과 종료 경계

- Python Workbench/F1 정적 계약
- Kakul generator 재현성, reference/authored strict schema와 sparse override 계약
- Esther/Valtan sound asset closure와 replicated occurrence dedup 계약
- Server gameplay contract 동적 회귀
- 변경 domain publisher validation
- Debug Product/Core 빌드와 회귀
- Release Product/Core 빌드와 회귀
- `git diff --check`, JSON/XML parse, 변경 파일만 stage

Client/UI는 에이전트가 실행하거나 조작하지 않는다. 자동 검증 뒤 사용자가 `Server + Client`로 진입해
F1에서 벽 preset을 선택하고 패턴 Complete Play 전후의 벽·Debris·Nav 시각 상태와 패턴 재생을 직접 판정한다.

## G06. KakulSaydon Arena 수직 슬라이스

복구된 물리 payload는 `LV_LUT_MIDNIGHTC_ED` Area ID를 정본으로 사용한다. `KakulSaydon`은
Resource Files와 F1에서 쓰는 사람이 읽는 collection 이름일 뿐 Map asset ID나 Server world ID를
대체하지 않는다. 현재 admission 실측은 Resource Collection과 Development Geometry Preview까지
통과하고 Server Product Level은 42개 계약 누락으로 차단된다. 따라서 Client 전용 레벨이나 임의
텔레포트로 우회하지 않고 다음을 한 변경 단위에서 닫는다.

- `Data/Maps/MapCatalog.json`의 product map, authoring placement, navigation, gameplay 경로
- 추출 placement의 실제 좌표 군집과 package/actor 이름에 근거한 stage marker 문서
- stable player spawn과 stage marker가 포함된 `Gameplay.world.json`
- MapTool bake 형식의 navsource/navpaint와 Client/Server 동일 runtime navgrid
- `WORLD_ID::KAKULSAYDON_ARENA`, Server shared simulation, world bootstrap
- `LEVEL::KAKULSAYDON_ARENA`, registry, loader, transition, 실제 level class
- Character Select에서 typed Server admission을 요청하는 KakulSaydon 진입 버튼
- F1 `Valtan / KakulSaydon` 탭과 현재 world에서만 활성화되는 stage 이동 명령

Mario 1~4나 관문 이름은 추출 원본에 그 identity와 위치 근거가 있을 때만 stable marker로 만든다.
단순 좌표 군집을 Mario라고 추측하지 않는다. exact 근거가 없는 구간은 package/cluster 이름으로
표시하고 UI에서 `추출 좌표 기준`임을 명시한다. stage 이동은 Client transform 변경이 아니라 Server가
현재 player를 해당 Area의 nav-valid stable marker로 옮긴 snapshot 결과만 소비한다.

## G07. 독립 Effect Asset/Instance와 공통 Authoring Scene

Effect V1/V2의 descriptor는 애니메이션 occurrence와 별개로 생성·저장·preview 가능해야 한다.
통합 화면은 다음 identity를 분리한다.

- Effect Asset: V1/V2 owner tool이 소유하는 reusable definition ID
- Effect Instance: Scene/Sequence가 소유하는 stable instance ID, effect asset ID, transform, pivot
- Binding: pattern/action/stage가 독립 effect asset 또는 instance occurrence를 참조하는 관계

World pivot은 Area transform을, Model pivot은 stable preview actor와 bone/anchor ID를 사용한다.
pointer, vector index, Prototype tag는 저장하지 않는다. V1/V2의 기존 local Play All/Solo는 미저장
Asset/Instance preview로 유지하고, `Complete Play`만 saved semantic pattern을 Server에 제출한다.
공통 Authoring Scene은 선택된 실제 Level의 map, actor, camera를 공유하되 tool별 draft ownership은
합치지 않는다.

## G08. Sequencer와 통합 Detail

Sequencer는 또 하나의 Effect/Animation 런타임이 아니라 existing owner domain을 시간축에서 조율하는
저작 문서다. stable sequence/track/section ID를 사용하고 다음 track을 단계적으로 지원한다.

- Animation clip/action section
- independent Effect instance section
- Sound point/window section
- Camera section
- Map/World presentation section

선택된 section은 오른쪽 Detail에서 exact owner data를 편집하고, 하단 Resource Files에서 고른
Animation/Effect V1/V2/Sound/Camera/Map asset ID를 전달받는다. `Save`는 sequence 문서만 원자 저장하며
각 원본 asset을 암시적으로 덮어쓰지 않는다. owner asset 자체를 편집할 때는 해당 Tool의 typed Save를
호출한다. 초기 구현은 local authored sequence 재생을 제공하고, Server Product pattern과 결합된
section만 `Complete Play` coverage에 포함한다.

## G09. 확장 검증

- Kakul admission gate `server-product-level`
- stage marker identity/중복/Area/nav coverage validator와 Server teleport protocol 회귀
- Effect Asset과 Instance가 animation 없이 생성·저장·재로드·재생되는 계약
- sequence parse -> validate -> stage -> commit, stale asset ID, duplicate section, invalid time rollback
- Valtan/Kakul F1 탭이 다른 world 명령을 제출하지 않는 실패 경로
- Debug/Release Core, `git diff --check`, JSON/XML parse

Kakul 화면 결과, stage 위치, map/effect/animation fidelity는 최종적으로 사용자가 직접 판정한다.

## G10. 물리 정본과 커밋 증거 경계

현재 작업은 `C:\Users\user\Desktop\LostArk` 한 물리 폴더에서만 통합한다. 다른 worktree의 파일을
폴더 단위로 덮어쓰지 않고, 현재 branch의 누적 커밋과 다른 작업자의 미커밋 파일을 먼저 보존한다.
개발 중 dirty build는 컴파일 진단으로만 사용하고 최종 PASS 증거로 승격하지 않는다.

최종 검증은 다음 identity를 같은 실행 증거에 묶는다.

- exact Git HEAD와 dirty 여부
- Data·source·shader input fingerprint
- domain publisher/validator receipt와 재사용 여부
- Debug/Release Client.exe, Server.exe와 대응 PDB의 SHA-256, 크기, 수정 시각
- 각 publish, compile/link, harness 단계의 경과 시간

`-SkipBuild`는 위 source/input과 binary receipt가 정확히 일치할 때만 허용한다. source, Data, project,
shader가 바뀌었거나 EXE/PDB pair가 다른 경우에는 stale binary 검증을 거부한다.

## G11. Effect V1/V2와 Drive Resource 보존

Effect Tool V2는 삭제 대상이 아니다. 현재 정본은 V2 파일 80개, Authored 75개, Binding 4개,
TextureSlotUsage 1개이며 Esther NPC와 Valtan hand 제품 소비자가 존재한다. V2 validator와 runtime consumer,
Tool을 통합 Workbench의 authoring owner로 보존한다.

V1은 Catalog 171 / Authored 171의 exact closure를 보존한다. 삭제 대상은 현재 Product consumer가 없는
과거 후보 생성·복구·증거용 apply/build/materialize/promote/replay corpus다. caller와 Data consumer를
확인하지 않은 Effect asset이나 물리 resource는 삭제하지 않는다.

`Client/Bin/Resources`는 팀장의 Drive 물리 정본이다. Git index에서는 추적하지 않지만 실제 파일은
그 자리에 보존한다. EXE/Data 전달 ZIP은 실행 바이너리와 generated DataFiles만 포함하고 Resource payload는
포함하지 않는다. 설치기는 허용 경로, SHA-256, staging, rollback을 검증하며 Resource 경로를 덮어쓰려는
ZIP을 거부한다.

## G12. Domain publisher와 build receipt

하나의 build domain graph가 Valtan Product, Effect V2, World Gameplay, Navigation, Destruction,
Gameplay Balance, Item, Reward의 입력·도구·출력을 소유한다. 같은 fingerprint의 domain은 한 번만
publish 또는 validate하고 이후 Product/Core 단계는 receipt를 재사용한다.

Server/Client project의 기존 pre-build hook은 Visual Studio 단독 빌드 안전망으로 보존한다. 중앙 runner가
receipt를 실제로 검증한 경우에만 명시적 MSBuild property로 중복 hook을 건너뛴다. Effect V2와
`ValidateV2`를 제거하거나 빈 성공으로 대체하지 않는다.

목표 실행 흐름은 다음과 같다.

```text
Git HEAD + dirty/source diff
-> changed domain fingerprint
-> domain별 publish/materialize/validate 1회
-> receipt의 output hash 검증
-> Engine/Shared/Server/Client compile/link
-> 영향 harness
-> CSO closure
-> EXE/PDB/source identity evidence
-> 사용자 visual/audio smoke
```

## G13. Broad harness 제거와 assertion 이관

기본 Solution Build는 Engine, Shared, Server, Client 네 제품만 유지한다. 강한 계약인
NetworkProtocol, CharacterSelectIsolation, WModelGeometry는 보존하고 변경 domain에서만 실행한다.
Physics, PointLight, ValtanPatternAudition은 domain-only 진단으로 제한한다.

- MapFrustumContractHarness: water assertion을 MapPipeline으로 이관한 뒤 solution, runner, 문서와 물리
  프로젝트를 제거한다.
- ActionPresentationTimelineHarness: sound/effect/camera/monster/party assertions를 작은 기존 domain
  harness로 이관하고 broad project를 제거한다.
- EffectRenderContractHarness: compiled shader/WARP/resource-root/stage-commit의 고유 assertion을
  Product CSO closure와 V2 focused contract로 이관한 뒤 물리 project와 runner를 제거한다.

assertion 이관은 문자열 존재 검사만으로 대체하지 않는다. 가능한 항목은 실제 parser/evaluator 또는 native
실행 계약으로 유지하고, 남은 harness가 제품 CPP를 직접 재컴파일하면 정확한 수와 후속 library 경계를
RESULT에 남긴다.

## G14. Tools와 제품 compile critical path 정리

이번 변경에서 caller 0이 확인된 HUD `.cfg` 일회성 DataMigration과 빈 ProjectAudit/Profiler 잔재를
정리한다. EffectPipeline과 LevelPlacementExtractor의 대량 복원 corpus는 active validator와 Kakul/map
추출 consumer의 dependency closure를 먼저 분리한 뒤 의미 단위로 제거한다. `MaterialEvaluatorHarness`,
`ModelAssetConverter/Bin`, Valtan tuning transaction 경로를 이름만 보고 삭제하지 않는다.

다음 compile 병목은 별도 검증 가능한 수직 슬라이스로 분리한다.

- Server 제품에서 Full-only `ServerGameplayContractTests.cpp`와
  `WorldDestructionBootstrapContractTests.cpp`를 contract executable/library로 분리
- Release Client critical path에서 Tool/Preview/Diagnostic TU를 editor target 또는 library로 분리
- `Effect_Tool.cpp`, `Effect_DocumentRenderer.cpp`, `Effect_DocumentCodec.cpp`, `MapTool.cpp`,
  `Animation_Tool.cpp`를 실제 책임 단위로 분해
- ActorX FBX 변환 구현을 CharacterAnimationIntake 흐름으로 통일하고 Sound intake owner를 분리
- UpdateLib와 Client SDK/runtime 배포의 중복 copy owner를 하나로 통일

디스크 캐시 정리는 compile 최적화와 분리한다. `Intermediate/ValtanTuningAuthoring`,
`ValtanTuningCandidates`, `ValtanTuningRuntime`은 일반 cache가 아니므로 삭제하지 않는다. 정확한 active
IntDir가 아닌 stale output만 경로를 재확인한 뒤 정리한다.

## G15. 최종 자동 검증과 사용자 인계

모든 구현 변경을 의미 단위 커밋으로 닫은 뒤 같은 clean HEAD에서 Debug/Release Product와 Core를 실행한다.
모든 domain이 바뀐 경우에만 FullDiagnostic을 확장한다. 실행하지 않은 publisher나 harness를 PASS로 쓰지
않고, Client/UI 화면과 소리는 사용자가 다음 경로에서 직접 판정한다.

- 여섯 캐릭터 Character Select와 4캐릭터 우선 스킬 smoke
- Valtan Arena의 F1 wall preset, 선택 pattern Complete Play, slot/Effect/Sound
- 4인 party 입장과 보스 사망 뒤 presentation
- Kakul Arena 진입, SL01~SL05 Server teleport, Workbench action binding

최종 RESULT는 `구현 완료`, `자동 검증`, `사용자 수동 검증 대기`, `남은 구조 병목`을 서로 분리한다.
