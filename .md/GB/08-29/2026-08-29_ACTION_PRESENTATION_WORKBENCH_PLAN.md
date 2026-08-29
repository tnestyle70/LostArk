# Action Presentation Workbench 통합 구현 계획

## 1. 목표

Debug Developer Tools의 Animation, Effect V1/V2, Boss 기능을 별도 테스트 레벨로 복제하지 않고 실제 `VALTAN_ARENA`의 Server authority 위에서 조율한다. 사용자는 한 화면에서 패턴 단계, 애니메이션 clip sequence, Effect, Sound, Camera/Shake, combat object, arena 파괴 상태를 함께 보고 수정한다.

재생 모드는 다음 셋으로 분리한다.

- `Clip Preview`: 기존 model clip 단독 preview
- `Pattern Offline`: Product 데이터를 읽는 비권위 preview
- `Server Replay / Live`: 선택한 stable pattern ID를 현재 Valtan Arena의 Server audition service로 제출

사용자가 보는 변경 명령은 `Save` 하나다. domain validation, 원본 atomic replace, Product projection, Server revision apply는 이 명령 내부 순서로 실행한다.

`Publish Candidate` 같은 중간 구현 용어는 UI에 노출하지 않는다.

## 2. 정본과 권위

| 영역 | 저작 정본 | Product/Runtime 소비 | 권위 |
|---|---|---|---|
| Valtan stage/action/duration | `Data/Valtan/Valtan.presentation.json` | `Data/Encounters/Valtan/ValtanEncounter.json`, Server bootstrap | Server |
| animation sequence | `Data/Animation/Authored/Valtan/Valtan.patternbindings.json` | `CValtan` presentation | Client presentation |
| clip sound cue | `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json` | `CValtan` action timeline | Client presentation |
| combat-object impact sound | `Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json` | reliable Server semantic event + `CValtan` | Server occurrence, Client asset resolution |
| combat object hit | `Data/Valtan/Valtan.combatobjects.json` | `Data/Encounters/Valtan/ValtanCombatObjects.json`, Server bootstrap | Server |
| Sound assets | `Data/Sound/CharacterSoundCatalog.json`의 Resources-relative ID | `Client/Bin/Resources/Sound/...` | Client presentation |
| Effect | 현재 V1/V2 domain 정본 | Effect presentation service | Client presentation |
| arena preset | typed `VALTAN_ARENA_PRESET` request | world destruction/collision/navigation transaction | Server |

## 3. 구현 수직 슬라이스

### G1. Workbench orchestration

- `CMainApp`의 Debug tool 이름을 `Action Presentation Workbench`로 통일한다.
- `CAnimation_Tool`은 stage, animation, Effect, Sound, combat-object lane을 join해 표시한다.
- `CBossTool`과 `CEffect_Tool_V2`는 Valtan Arena의 동일 `CValtanPatternAuditionService`를 소비한다.
- arena preset은 `Fresh`, `Circle Walls Gone`, `3 o'clock Broken`, `9 o'clock Broken`, `Both Sides Broken`을 제공한다.
- 9시 단독 상태를 표현하기 위해 예전 단조 증가 phase 값 대신 Server가 reset 후 선택 상태를 원자적으로 적용한다.

### G2. 도끼 바닥 충돌음의 의미 이벤트

애니메이션 clip 시간에 바닥음을 복제하지 않는다. `combatobject.valtan.high-jump.target-axe`의 `hit.valtan.high-jump.target-axe.01` timed pulse가 실제로 commit된 tick에 Server가 reliable `S2C_COMBAT_OBJECT_PRESENTATION_EVENT`를 보낸다.

패킷은 기존 enum 끝에 append하고 다음 값을 가진다.

- room-monotonic `eventSequence`
- `serverTick`, `combatObjectId`, `sourceNetEntityId`
- `combatObjectArchetypeId`, `ownerPatternId`, `ownerStageActionId`, `hitId`
- `repeatIndex`
- 실제 world position/yaw
- pinned gameplay data revision

Client는 source boss를 stable network ID로 찾고 `(combatObjectArchetypeId, hitId)` sound binding을 resolve한다. asset path나 sound event 이름은 Server에 보내지 않는다.

### G3. Sound 데이터 폐쇄성

- Valtan Authoring hit의 `hitId`를 Product JSON과 Server bootstrap까지 보존한다.
- 새 combat-object sound cue 문서는 exact property, stable ID, duplicate source tuple, Product hit 존재, Sound Catalog event 존재를 검증한다.
- `G_Voltan2_Attack09_ProjExp1`은 실제 추출된 네 `Sound/Valtan/g_voltan2_attack09_projexp1__*.wav`를 variant로 등록한다.
- Workbench는 이 cue를 combat-object lane과 Sound lane 양쪽에서 source → binding → asset 경로로 표시하고 직접 미리 듣기를 제공한다.
- 누락 cue는 coverage gap으로 표시하고 validator 실패로 만든다.

### G4. 빌드 병목 제거

- `Framework.sln` 기본 Build는 Engine, Shared, Server, Client만 남긴다.
- 정본 runner는 `Product`, `Core`, `FullDiagnostic` profile을 제공하고 기본값은 `Core`다.
- 제품 프로젝트에 병렬 컴파일을 활성화하고 Engine intermediate 경로를 구성별로 분리한다.
- 기존 `EffectRenderContractHarness`의 28개 Client CPP 재컴파일과 Imported Artist 코퍼스 결합을 제거한다. 현재 파일명은 제품 CPP/ProjectReference가 0개인 단일 CPP optional WARP 프로브로 축소하고 solution과 중앙 runner에서는 제외한다. V1/V2 Product CSO draw/readback과 runtime asset-root 경계만 focused 실행하며 나머지 data/stage/commit 계약은 domain validator와 Product fixture가 소유한다.
- Imported Artist corpus에만 결합된 assertion과 산출물은 Product gate로 복귀시키지 않는다.
- `ValtanFourPlayerHarness`와 wrapper/project/source를 삭제한다. 네 사람 network party 계약과 네 Server AI actor 계약을 분리하고, protocol에는 8개 전투원 snapshot round-trip만 고정한다. 봇 판단·스킬·생존은 향후 Server raid composition 수직 슬라이스가 소유한다.

### G5. Kakul 경계

Kakul resource intake만으로 가짜 arena/tool mode를 만들지 않는다. World ID, Server room gameplay, encounter data, Client level descriptor가 모두 존재하는 Product 수직 슬라이스가 들어온 뒤 동일 Workbench adapter를 연결한다.

## 4. 실패 및 rollback

- JSON은 `parse -> validate -> stage -> commit`을 지킨다.
- Save는 임시 파일에 쓴 뒤 atomic replace하며 validation/publisher 실패 시 기존 정본과 live revision을 유지한다.
- arena preset은 destruction, collision, navigation, boss hold를 한 transaction으로 preflight하고 하나라도 실패하면 Fresh 상태로 rollback한다.
- presentation event decode 실패는 출력 구조체를 변경하지 않는다.
- Sound binding 또는 asset resolve 실패는 gameplay를 멈추지 않되 명시적인 presentation diagnostic을 남긴다.

## 5. 검증

1. JSON/XML parse와 `git diff --check`
2. Valtan pipeline validate/publish와 Workbench static contract
3. NetworkProtocolHarness: round-trip, invalid enum/ID/string/revision, destination preservation, append-only ordinal
4. Server contract: 다섯 arena preset exact group count와 timed hit presentation event 1회 발생/ordering/reset
5. Client compile: new packet queue ordering, source boss resolution, sound binding exact join
6. `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product/Core/FullDiagnostic`
7. 같은 Release profiles
8. 사용자가 직접 `Client/Default`에서 Lobby 네 버튼, F1 Workbench, Valtan Arena preset, High Jump 도끼 충돌음을 육안·청각 판정

## 6. 완료 경계

자동 검증은 구조와 실행 계약까지만 PASS로 기록한다. Client 화면, Effect fidelity, 실제 들리는 음색과 음량은 사용자의 수동 판정 전에는 PASS로 기록하지 않는다. 원본 `C:\Users\user\Desktop\LostArk`가 다른 세션의 미커밋 변경으로 dirty한 동안에는 자동 merge하지 않고, 전용 worktree에서 검증 가능한 commit을 만든 뒤 충돌·dependency closure를 보고한다.
