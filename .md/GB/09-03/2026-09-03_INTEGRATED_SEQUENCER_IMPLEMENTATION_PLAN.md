# 2026-09-03 통합 Sequencer 구현 계획서 (쿠크세이튼 연출 통합)

브랜치 `GB/valtan-bugfix-koukusaydon-pattern`, HEAD `063e1a1a` = `origin/main`. 이 문서는 구현 계획서이며
디테일 계획서(전체 코드)는 각 G 착수 시 별도로 작성한다.

읽은 정본: `AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, `.md/GB/계획서하네스규칙.local.md`, `.md/TEAM/README.md`,
`.md/GB/09-03/2026-09-03_TOOLCHAIN_UNIFICATION_AND_BOTTLENECK_REMOVAL_IMPLEMENTATION_PLAN.md`(§13, §16.1),
`.md/GB/09-03/2026-09-03_KAKUL_SAYDON_PATTERN_INTAKE_AND_COMPOSITION_SAVE_AUDIT.md`.

아래 1장은 전부 현재 저장소 파일 실측이다. 기억이나 이전 계획의 결론을 옮기지 않았다.

전제: `Client/Bin/Resources`가 복구되기 전에는 이 계획의 실행 검증(Effect/Map/Sound 의존)을 시작할 수 없다.
문서·검증기·Server 계약처럼 리소스에 의존하지 않는 G00은 복구 전에도 진행할 수 있다.

---

## 0. 목표와 종료 증거

```text
목표
  쿠크세이튼 한 관문의 연출(보스 패턴 참조, 맵 애니메이션, 카메라, 조명·fog·post 분위기,
  화면 암전, 유리·관문 파괴, 스폰)을 하나의 sequence 문서와 하나의 playhead로 저작하고,
  Sequencer 창에서 Composition과 같은 box·timeline·Resources `+` 방식으로 편집·Save하며,
  Server가 시작 시각을 결정하면 Client CSequencePlayer가 owner별 adapter로 재생한다.

종료 증거
  1. Data/Sequences/<areaId>/<sequenceId>.sequence.json 이 validator/publisher(BuildDomains 도메인)를 통과한다.
  2. Server triggerBox playSequence 가 통합 sequence id 를 방송하고, Client Level 이 통합 sequence 로 해석해 재생한다.
  3. 네이티브 하네스 contract: adapter 호출 순서, seek, stop, adapter 실패 시 이전 상태 복원.
  4. Sequencer 창에서 track box 추가/이동/trim/삭제 후 Save → 같은 revision 재오픈 → publish 상태 표시.
  5. Debug FullDiagnostic PASS. 화면·음향의 최종 판정은 사용자 smoke.
```

---

## 1. 현재 실측 — 이미 존재하는 owner

| 연출 축 | 저작 정본 | 런타임 소비자 | 시작 권위 | 현재 한계 |
|---|---|---|---|---|
| 보스 패턴 | `Data/Valtan/Valtan.gameplay.json` + `.presentation.json` → `Data/Encounters/Valtan/*` | Server `CValtanBrain`, Client `CValtan` | Server | 쿠크세이튼은 `Data/Animation/Authored/KakulSaydon/*.patternbindings.json`이 `authority=REFERENCE_ONLY`. Server 전투 계약 0 (audit §1.4의 6종 문서가 아직 없음) |
| 맵 애니메이션 | `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` (`lostark.world-sequences` v2: templates[tracks/animationTracks] + instances) | `CWorldSequencePlayer::Play(instanceId, TARGET_SET)` | Server trigger `playSequence` → `WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE` → `CGameRoom::Broadcast_WorldSequencePlay` → `S2C_WORLD_SEQUENCE_PLAY{strSequenceInstanceId}` → `CClientReplication::Consume_WorldSequencePlays` → `CLevel_KakulSaydonArena::Start_ServerRequestedSequence` | 사다리·다리 링크(`KAKULSAYDON_PAPER_BRIDGE_LINKS`)와 서커스 피날레 placement 22개가 Level CPP 상수. 여러 instance를 시간차로 묶는 문서 없음 |
| 카메라 | 쿠크: `LV_LUT_MIDNIGHTC_ED.camerashots.json`(box 진입형 shot, eye/lookAt/fov/blend). 발탄: `ValtanCinematicCameraDocument`(keyframe cue) + `CValtanCinematicCameraController` | `Update_CameraShots`(플레이어 위치 OBB로 shot 선택), 발탄 Level 전용 controller | Client 위치 판정 / Server 패턴 edge | shot을 시각 기준으로 강제 시작하는 API 없음 |
| 조명·fog·post 분위기 | `Data/Rendering/Authored/RenderingProfiles.json` (`lostark.rendering-profiles` v1: globalQuality + profiles[light/shadow/fog/exposure·bloom multiplier]) | `CRenderingProfileService::Activate_Profile(id)` | `CMainApp` Level activation이 `CLevelRegistry` descriptor의 profile 하나를 켠다 (쿠크 = `scene.development.neutral.v1`) | 플레이 중 전환 없음, blend 없음 |
| 화면 암전·post | Engine `PRESENTATION_SCREEN_OVERLAY_DESC`(pass 14, tint/alpha/texture), `PRESENTATION_SCREEN_POST_DESC`(RGB_NOISE/ZOOM_BLUR/FILM_NOISE) | Client `CEffectScreenOverlayPresentation`(screen-overlay family Effect asset) → `CEffect_PresentationService::Spawn` | Effect cue owner | 시각 기준 시작·정지를 묶는 문서 없음 |
| 이펙트 | `Data/Effects/EffectCatalog.json` + `Authored/*.effect.json`, V2 group/bindings | `CEffect_PresentationService::Spawn_WorldRoot / Spawn_LevelPlacement / Seek_WorldRoot / Stop_WorldRoot` | pattern cue / level placement | 독립 시각 기준 재생 문서 없음 |
| 파괴(유리·관문) | 발탄 전용: `Data/Encounters/Valtan` world event set(`lostark.valtan-world-event-set-authoring`: members[bindingId, groupId, mutationId, offsetMs]), `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/*.debrisrecipe.json`, `.destructionsimulation.json` | Server `CWorldDestructionRuntime`(group/mutation/binding, persistent transaction), Client `CWorldDestructionProjectionRuntime`(full/delta sync) | Server | 도메인 `world.destruction` inputs가 `LV_LUT_HEARTRB_ED`/Valtan 고정. 쿠크세이튼 파괴 데이터 0 |
| 스폰 | `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` v6: playerSpawn 9, triggerBox 6(`1Stage_Final`, `jump.1~3`, `paper.1~2` → `playSequence` event), boss placement 0 | Server `CServerTriggerSystem`, `CSpawnGroupRuntime` | Server | 쿠크 boss/spawn group 없음 |
| 스테이지 마커 | `Data/Worlds/LV_LUT_MIDNIGHTC_ED/StageMarkers.json` `semanticStatus=SOURCE_LEVEL_ID_ONLY` | 없음 | — | 관문 의미 미확정 |
| 저작 툴 | Composition(7 lane `TIMELINE_ITEM`, Save 3-owner + receipt), Sequencer v0(`CSequencerTool`: Composition timeline mirror + Pattern 선택), Camera Tool, Map Tool(`CWorldSequenceToolPanel`), Rendering Workbench(`Save_Authored/Publish_Runtime`) | — | — | Sequencer는 문서·Save·`+`가 없음 |

핵심 실측 두 가지.

- 시작 권위 경로가 이미 하나 있다: `playSequence` trigger → `S2C_WORLD_SEQUENCE_PLAY(문자열 ID)`. 통합 sequence는 새 packet 없이 이 문자열 ID 공간에 `sequence.` 접두 ID를 추가해 같은 경로로 시작할 수 있다.
- 각 연출 owner는 이미 typed runtime API가 있다(`Play`, `Activate_Profile`, `Spawn_*`, camera shot). 통합 sequence가 새로 만들 것은 "문서 + 시계 + adapter"뿐이며 두 번째 renderer/player를 만들지 않는다.

---

## 2. 설계 결정

### 2.1 Stage와 Pattern은 Composition이 계속 소유한다

통합 sequence는 보스 패턴을 `patternId`로 참조만 한다. 패턴 실행·판정은 Server, 패턴 내부 lane 편집은 Composition이다. Sequencer의 `ACTOR_PATTERN` track을 선택하면 기존 `Consume_CompositionOpenRequest` 경로로 Composition이 열리고, Composition의 `Get_TimelineItems()`를 sequence 시각 offset만큼 밀어 read-only로 겹쳐 보인다. 이 계획은 Stage를 branch 노드로 바꾸지 않는다(09-03 계획서 §6 유지).

### 2.2 시작 시각은 Server가 결정하고, sequence는 Client presentation이다

```text
v1  Server triggerBox playSequence(targetId = "sequence.kakul.gate3.clear")  ->  S2C_WORLD_SEQUENCE_PLAY
    Client Level: Start_ServerRequestedSequence(id)
      id 가 CSequenceCatalog 에 있으면  CSequencePlayer::Play(id)
      없으면                            기존 CWorldSequencePlayer 경로 (호환)
v2  Composition presentation.json stage 에 sequenceInvocations[] 추가 -> Server pattern stage edge 가 같은 id 방송
```

gameplay 상태(navigation, collision, HP, spawn)를 바꾸는 연출은 sequence가 "요청"하지 않는다. 유리·관문 파괴는 Server `CWorldDestructionRuntime` 계약을 쿠크세이튼으로 확장한 뒤 Server가 방송한 파괴 delta에 Client presentation을 정렬한다(G05). 스폰은 계속 Server trigger `activateSpawnGroup/activateEncounter`이며 sequence의 `SPAWN` track은 v1에서 제외한다.

### 2.3 Client가 phase/HP 조건을 판정하지 않는다

"특정 기믹과 타이밍 기준으로 까맣게 변하는 연출"의 조건은 Server encounter 메타데이터(trigger, pattern edge, health bar)이며, sequence 문서는 조건을 갖지 않는다. v1은 trigger, v2는 pattern stage invocation이 조건의 전부다.

### 2.4 문서 정본과 두 번째 writer 금지

`Data/Sequences/<areaId>/<sequenceId>.sequence.json` 하나가 정본이다. world sequence instance, camera shot, rendering profile, effect asset, pattern은 각자의 정본을 유지하고 sequence는 stable ID만 참조한다. Sequencer 창은 이 문서 하나만 쓴다.

### 2.5 재생기 하나, adapter 하나씩

`CSequencePlayer`(Client, Level 소유)가 시계를 소유하고 track kind별 adapter 인터페이스를 호출한다. adapter는 기존 owner API 호출과 시작 전 상태 복원만 담당한다.

| track.kind | adapter가 호출하는 기존 API | 종료·복원 |
|---|---|---|
| `WORLD_SEQUENCE` | `CWorldSequencePlayer::Play(instanceId, targets)` | 기존 player의 hold/loop 규칙 |
| `SCENE_PROFILE` | `CRenderingProfileService::Activate_Profile(profileId)` | sequence 종료 시 시작 전 profile로 복귀(옵션 `restoreOnEnd`) |
| `CAMERA_SHOT` | `CLevel_KakulSaydonArena` camera shot을 shotId로 강제 활성(G02에서 `Force_CameraShot(shotId, blendInMs)` 추가) | `stopMs`에 해제, blendOut은 shot 값 |
| `EFFECT` | `CEffect_PresentationService::Spawn_WorldRoot` 또는 `Spawn_LevelPlacement`, `Seek_WorldRoot`(preview), `Stop_WorldRoot` | `stopMs` 또는 natural |
| `SCREEN_OVERLAY` | 같은 `Spawn` 경로의 screen-overlay family Effect asset (암전 = 검은 overlay + alpha ramp) | `stopMs` |
| `ACTOR_PATTERN` | 없음(참조 표시). Server가 실행 | — |
| `WORLD_EVENT`(G05) | 없음. Server 파괴 delta 관측을 sequence 시각과 대조해 진단만 | — |

### 2.6 Save와 publish는 기존 receipt 규칙을 따른다

parse → validate → stage → commit, 문서 `revision` CAS, 실패 시 이전 문서 보존. Save 뒤 publish는 09-03 G13/G14로 만든 백그라운드 job과 receipt 상태(`SOURCE_COMMITTED → PUBLISH_RUNNING → PUBLISHED`)를 그대로 쓴다. 이를 위해 `CBalanceTool`의 job 멤버를 `CPipelineJob`(Client/Public/PipelineJob.h)으로 추출해 Balance와 Sequencer가 공유한다(G03).

---

## 3. 문서 계약

```text
Data/Sequences/<areaId>/<sequenceId>.sequence.json      schema "lostark.sequence", formatVersion 1
{
  "schema": "lostark.sequence",
  "formatVersion": 1,
  "sequenceId": "sequence.kakul.gate3.clear",          stable id, 접두 "sequence." 필수 (world.sequence.instance.* 와 충돌 금지)
  "areaId": "LV_LUT_MIDNIGHTC_ED",
  "revision": 1,                                          CAS 정수
  "durationMs": 14000,                                    모든 track 의 startMs/stopMs 상한
  "displayName": "3관문 클리어 - 유리 파괴와 관문 붕괴",
  "tracks": [
    { "trackId": "track.01", "kind": "SCENE_PROFILE",  "startMs": 0,     "profileId": "scene.kakul.gate3.blackout.v1", "transitionMs": 0, "restoreOnEnd": true },
    { "trackId": "track.02", "kind": "SCREEN_OVERLAY", "startMs": 0,     "stopMs": 2500, "effectAssetId": "effect.screen.blackout.v1" },
    { "trackId": "track.03", "kind": "CAMERA_SHOT",    "startMs": 500,   "stopMs": 9000, "shotId": "shot.gate3.glass" },
    { "trackId": "track.04", "kind": "WORLD_SEQUENCE", "startMs": 2500,  "instanceId": "world.sequence.instance.gate3.glass" },
    { "trackId": "track.05", "kind": "EFFECT",         "startMs": 2500,  "stopMs": 6000, "ownerKind": "V1_DOCUMENT", "effectAssetId": "effect.kakul.glass-shatter.v1",
                                                        "anchor": { "kind": "LEVEL_PLACEMENT", "placementId": 41, "localTransform": { "translation": [0,0,0], "rotation": [0,0,0], "scale": [1,1,1] } } },
    { "trackId": "track.06", "kind": "WORLD_SEQUENCE", "startMs": 6000,  "instanceId": "world.sequence.instance.gate3.collapse" },
    { "trackId": "track.07", "kind": "ACTOR_PATTERN",  "startMs": 9000,  "entityRole": "BOSS", "patternId": "KAKUL_GATE3_ENTRANCE" }
  ]
}
```

검증 규칙(G00 validator가 강제).

- `sequenceId`, `trackId` stable id, 중복 금지. `startMs <= stopMs <= durationMs`.
- `WORLD_SEQUENCE.instanceId`는 같은 area의 `<Area>.worldsequences.json` instances에 존재.
- `CAMERA_SHOT.shotId`는 `<Area>.camerashots.json` shots에 존재.
- `SCENE_PROFILE.profileId`는 `RenderingProfiles.json` profiles에 존재.
- `EFFECT.effectAssetId`는 `EffectCatalog.json`(V1) 또는 V2 group catalog에 존재. `anchor.kind`는 `WORLD_ROOT | LEVEL_PLACEMENT`.
- `SCREEN_OVERLAY.effectAssetId`는 screen-overlay family 문서.
- `ACTOR_PATTERN.patternId`는 해당 area encounter의 Product(`Data/Encounters/<Boss>/*Encounter.json`)에 존재. 쿠크 Product가 생기기 전에는 validator가 `UNAVAILABLE_PATTERN_PRODUCT`로 거절한다(placeholder 금지).
- 한 문서 안에 `SCENE_PROFILE` 겹침 금지(같은 시각에 두 profile 불가). `CAMERA_SHOT` 겹침 금지.

런타임 문서: publisher가 `Client/Bin/DataFiles/Sequences/<areaId>/<sequenceId>.sequence.json`으로 복사한다(검증 통과본만). Client는 runtime만 읽는다.

Server 계약: `Publish-WorldGameplay.ps1`의 `playSequence` 검증이 `sequenceInstanceId`를 두 집합(world sequence instances ∪ `Data/Sequences/<Area>` sequenceIds)으로 확인한다. bootstrap row와 `S2C_WORLD_SEQUENCE_PLAY`는 변경 없음(문자열 ID).

---

## 4. 변경·신규 파일

| 파일 | 상태 | 역할 |
|---|---|---|
| `Data/Sequences/LV_LUT_MIDNIGHTC_ED/sequence.kakul.gate3.clear.sequence.json` | 신규 | 첫 저작 문서(G02 smoke 대상) |
| `Data/Rendering/Authored/RenderingProfiles.json` | 수정 | `scene.kakul.*` profile 추가(관문별 분위기, 암전용 저노출) |
| `Data/Effects/Authored/effect.screen.blackout.v1.effect.json` + `EffectCatalog.json` | 신규/수정 | 암전 screen overlay asset |
| `Tools/SequencePipeline/sequence_document.py` | 신규 | schema/ID/시간 검증, owner 문서 cross-check |
| `Tools/SequencePipeline/Publish-Sequences.ps1` | 신규 | `-Mode Validate|Publish`, 검증 통과본을 `Client/Bin/DataFiles/Sequences`로 transaction 복사 |
| `Tools/SequencePipeline/test_sequence_document_contract.py` | 신규 | 정상/잘못된 version·ID·시간/중복/owner 부재/중간 실패 rollback |
| `Tools/Build/BuildDomains.json` | 수정 | 도메인 `sequence.presentation`(publisher; inputs `Data/Sequences/**`, `Data/Maps/Authoring/**/*.worldsequences.json`, `*.camerashots.json`, `Data/Rendering/Authored/RenderingProfiles.json`, `Data/Effects/EffectCatalog.json`; outputs `Client/Bin/DataFiles/Sequences/**`) |
| `Tools/WorldPipeline/Publish-WorldGameplay.ps1` | 수정 | `playSequence` 대상에 통합 sequence id 허용 |
| `Client/Public/SequenceDocument.h`, `Client/Private/SequenceDocument.cpp` | 신규 | `SEQUENCE_TRACK`, `SEQUENCE_DOCUMENT`, `CSequenceDocument`(Parse_Text/Validate/Serialize, CAS revision) |
| `Client/Public/SequenceCatalog.h`, `Client/Private/SequenceCatalog.cpp` | 신규 | area별 runtime 문서 parse → validate → stage → commit, `Find(sequenceId)` |
| `Client/Public/SequencePlayer.h`, `Client/Private/SequencePlayer.cpp` | 신규 | 시계 + `ISequenceTrackAdapter` 호출, preview seek, Stop_All, 실패 시 복원 |
| `Client/Public/SequenceTrackAdapters.h`, `Client/Private/SequenceTrackAdapters.cpp` | 신규 | WorldSequence / SceneProfile / CameraShot / Effect / ScreenOverlay adapter |
| `Client/Private/Level_KakulSaydonArena.cpp/.h` | 수정 | `CSequenceCatalog`·`CSequencePlayer` 소유, `Start_ServerRequestedSequence`에서 통합 sequence 우선 해석, `Force_CameraShot(shotId, blendInMs)`/`Release_CameraShot` 공개 |
| `Client/Public/PipelineJob.h`, `Client/Private/PipelineJob.cpp` | 신규 | `CBalanceTool`의 백그라운드 publish job 추출(Launch/Update/State) |
| `Client/Public/BalanceTool.h`, `Client/Private/BalanceTool.cpp` | 수정 | job 멤버를 `CPipelineJob`으로 교체(공개 계약 유지) |
| `Client/Public/SequencerTool.h`, `Client/Private/SequencerTool.cpp` | 수정 | v1: 문서 소유, track lane/box, Detail, Resources `+`, Save/Retry, preview |
| `Client/Private/MainApp.cpp` | 수정 | Sequencer 생성 시 `CSequenceCatalog`/현재 Level player 연결, job 폴링 |
| `Client/Default/Client.vcxproj`, `Client.vcxproj.filters` | 수정 | 신규 8개 파일 등록, 필터 `03. Tools\05. Sequencer` |
| `Tools/ValtanPatternAuditionServiceHarness/Private/SequencePlayerContractTests.cpp` + `.vcxproj/.filters` | 신규 | mock adapter로 순서·seek·stop·rollback contract |
| `Tools/Build/Invoke-BuildAndRegression.ps1` | 수정 | FullDiagnostic 변경 domain 목록에 `sequence.presentation` 및 위 Python contract 추가 |
| `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`, `AREA_DATA_LAYER_GUIDE.md`, `CLAUDE.md` | 수정 | Sequences optional layer, Sequencer 사용법(G03 종료 시) |

Server 코드 변경은 v1(G00~G04)에 없다. G05(파괴)에서 `Publish-ValtanWorldDestruction.ps1`의 area 일반화와 Server bootstrap 입력이 바뀐다.

---

## 5. G별 구현 범위와 검증

### G00 — 문서 계약, validator, publisher, BuildDomains (리소스 없이 진행 가능)

- `Tools/SequencePipeline/sequence_document.py`: 3장 규칙 전부. owner 문서 cross-check는 `Data/Maps/Authoring/<Area>/*.worldsequences.json`, `*.camerashots.json`, `Data/Rendering/Authored/RenderingProfiles.json`, `Data/Effects/EffectCatalog.json`, `Data/Encounters/*/*Encounter.json`을 읽는다.
- `Publish-Sequences.ps1 -Mode Validate|Publish`: Validate는 전 문서 검증, Publish는 임시 폴더 stage 후 `Client/Bin/DataFiles/Sequences` 한 transaction 교체, 중간 실패 시 전부 rollback.
- BuildDomains `sequence.presentation` 도메인 등록. `Run-FullPipeline.ps1`은 이미 도메인 전체를 실행하므로 변경 없음.
- `Publish-WorldGameplay.ps1`: `playSequence.sequenceInstanceId` 허용 집합 확장.
- 검증: `python -m unittest Tools.SequencePipeline.test_sequence_document_contract` 전부 PASS, `Publish-Sequences.ps1 -Mode Validate` PASS, `Publish-WorldGameplay.ps1 -Mode Validate` PASS, `python Tools/Build/test_build_domain_pipeline_receipts.py` PASS.

### G01 — Client 문서·재생기·adapter 계약

- `CSequenceDocument::Parse_Text`는 3장 규칙과 같은 검사를 C++에서도 수행한다(런타임 문서를 신뢰하지 않음).
- `CSequenceCatalog::Load_Area(areaId)`: runtime 폴더의 모든 문서를 parse → validate → stage → commit. 하나라도 실패하면 이전 catalog 보존.
- `CSequencePlayer`: `Play(sequenceId)`, `Update(dt)`, `Seek(ms)`(preview 전용), `Stop_All()`. track은 `startMs` 도달 시 adapter `Begin`, `stopMs` 도달 시 `End`, 문서 끝에서 `restoreOnEnd` 처리. adapter `Begin` 실패는 해당 track만 격리하고 상태 문자열에 남기며 나머지 track은 계속한다.
- `ISequenceTrackAdapter { Begin(track, ctx, status); End(track, ctx); Seek(track, ms, ctx); Restore(ctx); }` 5종 구현. adapter는 owner API만 호출하고 자체 렌더링·문서 저장을 하지 않는다.
- vcxproj/filters 등록. 네이티브 하네스 `SequencePlayerContractTests.cpp`: mock adapter로 (a) 시작 순서 = startMs 정렬, (b) 같은 시각 track은 문서 순서, (c) Seek 후 활성 집합 정확, (d) Begin 실패 track 격리·나머지 진행, (e) Stop_All이 Restore를 역순 호출.
- 검증: `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic`에서 하네스 PASS.

### G02 — Level 연결과 첫 sequence smoke

- `CLevel_KakulSaydonArena`: `m_SequenceCatalog.Load_Area(LV_LUT_MIDNIGHTC_ED)` 실패는 Level 진입 실패가 아니라 "sequence 없음"으로 기록(맵은 진입). `Start_ServerRequestedSequence`가 통합 sequence를 우선 해석.
- camera shot 강제 시작: `Force_CameraShot(shotId, blendInMs)`, `Release_CameraShot()`을 adapter가 호출. 자유 카메라(F6)가 켜져 있으면 shot을 무시하는 기존 규칙 유지.
- SceneProfile adapter: 시작 전 `Get_ActiveProfileId()`를 저장하고 `restoreOnEnd`에서 복귀. `transitionMs`는 v1에서 즉시 전환만 지원하고 값은 문서에 보존한다(blend는 `CRenderingProfileService`의 별도 슬라이스).
- 첫 문서 `sequence.kakul.gate3.clear`와 triggerBox 1개(`Gameplay.world.json` `playSequence`)로 연결. 유리·관문 파괴는 G05 전까지 `WORLD_SEQUENCE`(transform key로 조각 낙하 흉내) + `EFFECT`로 표현한다.
- 검증: `Publish-WorldGameplay.ps1` PASS, Server `--contract-test` PASS(trigger 방송), Debug Core PASS. 사용자 smoke: Lobby → Character Select → Debug typed transfer로 쿠크 진입 → trigger box 진입 → 암전 → 카메라 → 맵 애니메이션 → 복귀.

### G03 — Sequencer 창 v1 (저작)

- 문서 선택: area 콤보 + sequence 콤보 + `New`(sequenceId 입력, stable id 검사).
- lane: track kind별 lane(SCENE_PROFILE, CAMERA, WORLD_SEQUENCE, EFFECT, SCREEN_OVERLAY, PATTERN). box 선택/본체 이동/좌우 trim(`startMs/stopMs`)/삭제는 Composition과 같은 조작 규칙. `SCENE_PROFILE`·`CAMERA_SHOT` 겹침은 즉시 거부.
- Detail: kind별 필드(profileId, shotId, instanceId, effectAssetId, anchor, restoreOnEnd).
- Resources 탭 `+`: 탭 5개(World Sequences, Camera Shots, Scene Profiles, Effects(V1/V2), Patterns). 각 탭은 owner 문서를 읽기 전용으로 나열하고 `+`는 playhead 위치에 track을 추가한다. 새 world sequence·camera shot·profile 자체를 만드는 것은 각 owner 툴(Map Tool, Camera Tool, Rendering Workbench)이며 여기서 만들지 않는다.
- Save: `CSequenceDocument::Serialize` → 임시 파일 → 재파싱 검증 → revision CAS → rename commit. 실패 시 원본 보존. Save 뒤 `CPipelineJob`으로 `Publish-Sequences.ps1 -Mode Publish` 실행, receipt 상태 표시와 Retry(09-03 G13 규칙).
- Preview: 현재 Level이 같은 area이면 `CSequencePlayer::Seek`로 adapter를 구동하고 Stop 시 복원. 다른 Level이면 preview 불가를 표시(두 번째 렌더러 없음).
- 검증: Python contract에 Sequencer CPP text oracle(second writer 부재, Save가 `Serialize`+CAS만 사용) 추가. 사용자 smoke: `+` 5종 → 이동/trim → Save → 재오픈 → publish 상태.

### G04 — Composition과의 연결

- `ACTOR_PATTERN` 선택 시 `Consume_CompositionOpenRequest`로 Composition을 열고 `Select_PatternById`로 같은 Pattern을 선택, Composition timeline을 sequence offset만큼 밀어 Sequencer lane 아래에 read-only로 표시.
- Composition Save receipt와 Sequencer Save receipt는 같은 `CPipelineJob`을 쓰되 한 번에 하나만 실행(PUBLISH_BUSY).
- v2 시작 권위: `Valtan.presentation.json`(쿠크 `Kakul.presentation.json`) stage `sequenceInvocations[]` → projector → Product → Server stage ENTER edge에서 `Broadcast_WorldSequencePlay(sequenceId)`. 이 항목은 쿠크 Product(G06)와 함께 닫는다.

### G05 — Server 상태를 바꾸는 연출: 유리·관문 파괴

- 데이터: `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/*.debrisrecipe.json`, `Data/Worlds/LV_LUT_MIDNIGHTC_ED/EncounterProps.world.json`, world event set 문서(schema를 `lostark.world-event-set-authoring`로 일반화하고 Valtan 문서는 그대로 읽히게 alias).
- publisher: `Publish-ValtanWorldDestruction.ps1`의 area/encounter 하드코딩을 파라미터화하고 BuildDomains `world.destruction`을 area별 도메인 2개로 분리.
- Server: `CWorldDestructionRuntime` 초기화 입력에 쿠크 bootstrap 추가. trigger `activateEncounter` 또는 pattern edge가 mutation을 commit하고 delta를 방송(기존 경로).
- Client: `CWorldDestructionProjectionRuntime`가 쿠크 area도 소비. sequence `WORLD_EVENT` track은 Server delta 도착 시각과 문서 시각의 차이를 진단 문자열로 보이기만 한다.
- 검증: Server `--contract-test`에 쿠크 destruction bootstrap 시나리오, NetworkProtocol 하네스, Debug FullDiagnostic.

### G06 — 쿠크세이튼 보스 패턴 Product (별도 계획)

audit §1.4의 6종 문서(`BossCatalog`, `BossProfiles`, `Data/Kakul/Kakul.gameplay.json`, `Kakul.presentation.json`, `Kakul.combatobjects.json`, `Data/Encounters/KakulSaydon/*`)와 Server row tag·brain·`GAMEPLAY_BOOTSTRAP_FORMAT_VERSION` bump는 이 계획의 범위 밖이며 별도 구현 계획서로 다룬다. `ACTOR_PATTERN` track과 G04 v2 시작 권위는 이 Product가 있어야 실제로 재생된다.

---

## 6. 하지 않는 것

- Client에서 phase/HP/기믹 조건을 판정하는 sequence 조건식.
- world sequence, camera shot, rendering profile, effect asset을 Sequencer에서 생성·저장하는 두 번째 writer.
- Stage를 branch 전용 노드로 바꾸는 모델 변경.
- Rendering profile blend(`transitionMs` 실제 보간)와 PBR/CSM 같은 렌더링 기능.
- Sequencer가 Client 내부에서 빌드를 동기 실행하는 것(빌드는 루트 BAT, Client 종료 후).

---

## 7. 검증 명령

```powershell
python -m unittest Tools.SequencePipeline.test_sequence_document_contract
powershell -ExecutionPolicy Bypass -File Tools/SequencePipeline/Publish-Sequences.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
python Tools/Build/test_build_domain_pipeline_receipts.py
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic
RunFullPipeline.bat
git diff --check
```

각 G의 RESULT는 실제 diff와 실행 증거만 기록하고, 사용자 smoke(암전·카메라·맵 애니메이션·파괴의 화면 판정)는 사용자 서면 판정 전 PASS로 적지 않는다.
