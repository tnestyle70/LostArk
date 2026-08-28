# 에스더·발탄 도끼·도넛 사운드 추출 인계와 런타임 결선 구현 계획

## G00. 결론과 현재 작업 경계

다른 세션이 추출 중인 에스더 사운드와 발탄 도끼·도넛 사운드는 한 종류의 애니메이션
cue로 묶지 않는다. 실제 Server 승인 occurrence와 현재 Client clock은 다음 세 경로로 나눈다.

| 대상 | 재생을 승인하는 occurrence | 시간 정본 |
|---|---|---|
| 플레이어 에스더 호출 | `(playerNetEntityId, ESTHER_CAST, actionStartTick)` | player snapshot의 `serverTick - actionStartTick` |
| 소환 에스더 공격 | `(npcNetEntityId, archetypeId, esther.strike, actionStartTick)` | world entity snapshot의 `serverTick - actionStartTick` |
| 발탄 타깃 도끼·도넛 | `(sourceBossNetEntityId, combatObjectArchetypeId, spawnTick)` | combat-object reliable spawn의 `serverTick - spawnTick` |

입력키, Debug preview, 컷인 시작, 로컬 애니메이션 진행률, Effect handle 성공 여부는 사운드
발생 권위가 아니다. Server가 승인하지 않은 입력 단계에서 먼저 재생하지 않는다.

이 문서는 구현 계획서다. 이번 세션에서는 C++·JSON·WAV·publisher를 수정하지 않고,
추출 세션 결과가 인계된 뒤 바로 적용할 데이터와 호출 경로만 고정한다. Client/UI를 실행하거나
청취 PASS를 대신 판정하지 않는다. bug-fix PR, 추출 PR, sound runtime PR의 commit·push·PR·merge도
지금 수행하지 않는다.

구현 PR의 merge 순서는 다음으로 고정한다.

1. `codex/lostark-sound-extraction-runtime`의 최종 추출 PR
2. 현재 발탄 bug-fix PR과 그 merge 뒤의 combat-object 정의
3. 이 문서의 sound mapping·runtime PR

2번 merge 뒤 도끼·도넛 `hitId`와 `atMs`를 다시 읽고, 값이 바뀌었으면 3번 브랜치에서 cue product를 다시 생성한다.
sound PR이 숫자를 수동 복제해 먼저 고정하지 않는다.

## G01. 현재 구조 실측

### G01-1. 기존 발탄 body animation sound

현재 발탄 본체 애니메이션 사운드는 다음 경로가 이미 동작한다.

```text
Valtan.patternsoundcues.json
→ CValtanPatternSoundCueDocument
→ CValtan::Spawn_DuePatternSoundCues()
→ CSoundCueCatalog::Find_Variants("Valtan", soundEvent)
→ CRuntimeAssetRoot::Resolve()
→ CGameInstance::Play_Sound()
```

이 경로는 boss pattern/action/clip occurrence가 존재할 때만 맞다. high-jump body animation의
takeoff/land cue는 이 경로에 남기지만, 플레이어별로 생성되는 타깃 도끼와 발탄에서 독립된
도넛 occurrence를 `Valtan.patternsoundcues.json`에 추가하지 않는다. 도넛 stage는 현재 독립
combat object이며 타깃 도끼의 피해 시점도 각 object의 `spawnTick` 기준이다.

### G01-2. 에스더 occurrence

Server는 에스더 사용을 승인한 뒤 `CGameRoom`에서 caster를 `PLAYER_ACTION_STATE::ESTHER_CAST`로
바꾸고 `iActionStartTick`을 배정한다. Client는 이 snapshot을
`CClientReplication → CCharacter::Apply_NetworkAction()`으로 전달한다.

현재 `ESTHER_CAST` 분기는 `m_pChain = nullptr`, `m_iEffectActionStartTick = 0`으로 두므로
skill chain 전용 `CCharacter::Update_SoundCues()`는 호출음에 사용할 수 없다. 입력 처리나 기존
skill cue를 억지로 재사용하지 않고 replicated action edge를 별도로 scan한다.

Server가 만드는 소환 에스더는 다음 세 archetype과 같은 semantic action을 사용한다.

| slot | archetype | action | 현재 Server 수명 | Client clip |
|---:|---|---|---:|---|
| 1 | `NPC_59030` | `esther.strike` | 5300ms | `npc_evt1_sk_swordofchampion_bk` |
| 2 | `NPC_58700` | `esther.strike` | 7100ms | `npc_sk_dochul` |
| 3 | `NPC_59060` | `esther.strike` | 4100ms | `npc_sk_breathofarcturus` |

`S2C_WORLD_ENTITY_SPAWNED`에는 `strActionId`만 있고 `serverTick/actionStartTick`이 없다.
따라서 reliable spawn이나 `CEstherCutinPresentationService`의 로컬 elapsed time을 sound clock으로
쓰지 않는다. 첫 `WORLD_ENTITY_SNAPSHOT`의 `iServerTick`, `strActionId`, `iActionStartTick`으로
cue를 catch-up하고 이후 snapshot으로 진행한다.

### G01-3. 타깃 도끼와 도넛 occurrence

현재 정본은 `Data/Valtan/Valtan.combatobjects.json`이다.

| object | stable hit | Server hit time | lifetime |
|---|---|---:|---:|
| `combatobject.valtan.high-jump.target-axe` | `hit.valtan.high-jump.target-axe.01` | 1200ms | Server definition |
| `combatobject.valtan.fist-in-out.donut` | `hit.valtan.fist-in-out.donut.01` | 1600ms | 2600ms |

`S2C_COMBAT_OBJECT_SPAWNED`는 `combatObjectId`, `sourceNetEntityId`, `spawnTick`, `serverTick`,
`combatObjectArchetypeId`, `clientVisualId`를 이미 전달한다. `COMBAT_OBJECT_PROJECTION_RECORD`도
`spawnTick`을 보존한다. 새 network field나 wire version 없이 이 clock을 그대로 사용한다.

high-jump는 `PER_ALIVE_PLAYER`, 세 wave, wave 간격 1333ms다. 4인 생존 시 같은 wave에서
타깃 도끼가 네 개 생기므로 object ID마다 동일 사운드를 재생하면 한 순간에 네 번 겹친다.
타깃 도끼의 공용 spawn/hit음은 같은 `source boss + archetype + cueId + spawnTick`을 한 occurrence로
합친다. 다음 wave는 `spawnTick`이 다르므로 다시 한 번 재생한다.

### G01-4. 추출 branch와 현재 배포 위험

현재 `codex/lostark-sound-extraction-runtime`의 `cfa1ce5a`에는 다음 기반이 있다.

- `Tools/Audio/Extract-LostArkSound.ps1`
- `Tools/Audio/Resolve-LostArkSoundEvents.ps1`
- `Tools/Audio/Publish-LostArkSound.ps1`
- `Tools/Audio/Test-LostArkSound.ps1`
- `Tools/Audio/LostArkSoundSelection.json`

생성 결과는 `Sound/SoundCatalog.json`, `Sound/ExtractionManifest.json`에 event/media/origin과
PCM16·48kHz·channel·duration·SHA-256을 남긴다. 같은 역할의 두 번째 추출기를 만들지 않고 이
파이프라인을 확장한다.

다만 현재 selection에는 에스더 player/NPC bank가 없고, `Publish-LostArkSound.ps1 -Replace`는
category 증분 교체가 아니라 `Client/Bin/Resources/Sound` 전체를 backup 후 통째로 바꾼다.
또 추출 branch의 발탄 asset ID는 `Sound/Boss/Valtan/...`, 현재
`CharacterSoundCatalog.json`의 Valtan ID는 `Sound/Valtan/...`이다. 별도 staging과 전체 catalog
closure 검증 없이 live root에 `-Replace`를 실행하지 않는다.

현재 Engine sound API는 2D fire-and-forget one-shot이다. 이 PR은 정확한 occurrence·timing·dedup과
one-shot WAV 결선까지다. 여러 combat object가 각각 소유하는 3D emitter 또는 instance loop
channel은 이번 범위가 아니다. 추출 결과가 진짜 loop segment라면 유한 one-shot mix로 publish할지,
별도 Engine channel slice가 필요한지 receipt 단계에서 `미확정`으로 막는다.

## G02. 추출 세션 인계 receipt

WAV 파일명만 받아 의미를 추측하지 않는다. 추출 세션은 cue 후보마다 다음 필드를 넘긴다.

| 필드 | 의미와 검증 |
|---|---|
| `scope` | `PLAYER_ESTHER_CAST`, `NPC_ESTHER_STRIKE`, `VALTAN_TARGET_AXE`, `VALTAN_DONUT` 중 하나 |
| gameplay owner | player action 또는 NPC archetype/action, combat-object archetype/hitId |
| presentation owner | player clip 또는 NPC clip, combat-object clientVisualId |
| Wwise origin | logical package, bank, event name, event ShortID, media ID 목록 |
| source receipt | Action LOA/AKEvent 근거 파일과 hash 또는 동등한 추출 증거 |
| runtime variants | `Sound/.../*.wav` Resources-relative asset ID 목록 |
| audio identity | 각 WAV SHA-256, bytes, duration, PCM codec, sample rate, channels |
| cue basis | action start, object spawn 또는 stable hit 중 무엇을 0점으로 삼는지 |
| playback | one-shot/loop, volume 후보, variant 선택 정책 |

다음 값은 현재 조사상 후보일 뿐 확정값이 아니다.

- player 호출음 후보: `PC_COMMON_ESTHER.PC_Common_FX_Active1/2`
- 실리안 후보: `S_Mob_Silian1.Silian1_Attack9_*`
- Wei와 Bahuntur의 정확한 sound event와 timing
- 타깃 도끼·도넛의 정확한 sound event, variant, volume

시각 Effect timing이나 비슷한 발탄 body event를 audio timing으로 대신 쓰지 않는다. receipt에
owner, event, timing basis 중 하나라도 없으면 해당 row를 `mapping unresolved`로 남기고 runtime
document에 넣지 않는다. 다른 확정 cue는 독립적으로 구현할 수 있다.

## G03. 데이터 정본과 publisher

### G03-1. 에스더 semantic cue 문서

새 `Data/Sound/EstherActionSoundCues.json`은 플레이어 호출과 세 소환 NPC 공격을 한 문서에서
소유하되 row의 owner kind를 명시한다.

각 row는 다음 계약을 가진다.

- `cueId`: document 안에서 유일한 stable ID
- `ownerKind`: `PLAYER_ACTION` 또는 `NPC_ACTION`
- `ownerId`: `ESTHER_CAST` 또는 정확한 NPC archetype ID
- `actionId`: player는 typed `ESTHER_CAST`, NPC는 `esther.strike`
- `catalogOwnerId`: `CSoundCueCatalog` owner bucket
- `soundBank`, `soundEvent`: 추출 receipt의 exact string
- `startMs`: 해당 action start 기준 음원 발생 시점
- `lateToleranceMs`: 늦게 받은 첫 snapshot에서 재생을 허용하는 최대 지연
- `volume`: finite `[0, 1]`
- `repeatPolicy`: 이번 slice에서는 `ONCE_PER_ACTION`만 허용

`formatVersion`, unknown field/value, duplicate cue ID, invalid stable ID, 음수 또는 action 수명 밖의
`startMs`, non-finite volume, catalog unresolved event를 fail-closed한다. 전체 parse/validate가
성공한 staged document만 commit하며 실패 시 직전 document 또는 sound-disabled 상태를 유지한다.

### G03-2. combat-object binding과 generated cue

authoring은 새
`Data/Animation/Authored/Valtan/Valtan.combatobjectsoundbindings.json`이 소유한다.

각 binding은 다음을 저장한다.

- `cueId`
- `combatObjectArchetypeId`
- `clientVisualId`
- `triggerKind`: `SPAWN` 또는 `HIT`
- `triggerHitId`: `HIT`일 때 필수
- `catalogOwnerId`, `soundBank`, `soundEvent`
- `lateToleranceMs`, `volume`, `coalescePolicy`

authoring에 1200/1600 같은 `atMs`를 다시 적지 않는다. 새
`Tools/ValtanPipeline/build_valtan_combat_object_sound_cues.py`가
`Valtan.combatobjects.json`의 `triggerHitId`를 정확히 하나 resolve하고 다음 product를 생성한다.

`Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json`

product에는 runtime이 바로 읽는 `startMs`를 쓰되 `triggerHitId`와 source revision도 보존한다.
object/hit 누락, 중복 hitId, non-timed hit에 대한 timed cue, `startMs >= lifetimeMs`, visual join 불일치,
catalog event 미해결은 publish 실패다. 기존 `Valtan.patternsoundcues.json`에는 이 row를 넣지 않는다.

### G03-3. 기존 sound catalog 재사용

runtime WAV lookup은 새 catalog를 병행하지 않고 기존
`Data/Sound/CharacterSoundCatalog.json → CSoundCueCatalog::Find_Variants(owner, event)`를
재사용한다. C++의 `strClassName/s_ClassEvents` 명칭과 주석은 실제 사용에 맞게
`strOwnerId/s_OwnerEvents`로 교정하되 JSON의 기존 `classes` compatibility key와 player class
bucket은 이번 PR에서 전면 migration하지 않는다. 이미 `Valtan`이 non-player owner로 쓰이는
현재 형식을 보존한다.

새 owner bucket은 cue 문서의 `catalogOwnerId`가 결정한다. 기본안은 다음이다.

- `Esther`
- `ValtanCombatObject`

새 `Tools/Audio/Publish-GameplaySoundCueCatalog.ps1`은 extraction `SoundCatalog.json`과
`ExtractionManifest.json`, 두 cue reference document를 읽어 exact event alias를 WAV variant에
resolve한다. 기존 catalog의 unrelated owner/event를 stage copy에 그대로 보존하고, 해당 owner의
요청 event만 교체한 뒤 검증 성공 시 한 번에 commit한다.

기존 `build_sound_catalog.py`가 Common+여섯 class만으로 전체 catalog를 재작성하거나,
`build_valtan_sound_catalog.py`가 Valtan body cue event만으로 owner bucket을 덮어써도 Esther와
combat-object event가 사라지지 않도록 두 도구에 unrelated owner/event union 보존 회귀를 추가한다.

## G04. Sound root staging과 원자 교체

### G04-1. 추출·resolve

추출 PR을 병합한 뒤 `LostArkSoundSelection.json`에 receipt의 exact logical package만 추가한다.
bank 이름을 파일명 검색으로 추정하거나 전체 설치 폴더를 runtime dependency로 두지 않는다.

먼저 `Publish-LostArkSound.ps1 -PlanOnly`로 candidate 수와 category를 검토한다. 실제 publish도
live `Client/Bin/Resources/Sound`가 아닌 작업용 `.../Resources/Sound` staging을 `-OutputRoot`로
지정한다. 이 단계에서 `-Replace`를 live root에 직접 주지 않는다.

### G04-2. 전체 dependency closure

staging에는 새 에스더·도끼·도넛 WAV만 두지 않는다. commit 후보는 다음 closure 전체를 포함한다.

1. 새 extraction `SoundCatalog.json`, `ExtractionManifest.json`
2. 현재 `CharacterSoundCatalog.json`이 참조하는 모든 WAV
3. 새 Esther/ValtanCombatObject event의 모든 admitted variant
4. BGM/UI 등 현재 runtime 코드와 제품 JSON이 직접 참조하는 `Sound/...` asset

`Sound/Boss/Valtan`으로 기존 Valtan을 canonicalize하려면 현재 Valtan catalog event가 100% 새
staging에서 exact resolve되어야 한다. 100%면 모든 path와 WAV를 같은 commit에서 전환한다.
한 건이라도 미해결이면 기존 `Sound/Valtan` asset과 path 전체를 유지하고 부분 migration하지 않는다.

### G04-3. commit과 rollback

`Test-LostArkSound.ps1`에 gameplay catalog 경로 검사를 추가해 staging 기준으로 다음을 먼저
통과시킨다.

- 모든 catalog asset ID가 `Sound/...wav`이고 absolute path/drive/`..`가 없음
- 모든 referenced WAV 존재, case-insensitive duplicate 0
- catalog/manifest SHA-256·bytes·duration과 실제 파일 일치
- PCM16, 48kHz, mono/stereo, positive duration
- raw WEM/BNK/PCK/key 잔존 0
- 기존 owner/event loss 0, 새 required event unresolved 0

검증 후에만 기존 Sound root를 recoverable backup으로 옮기고 staged root를 같은 부모 안에서
한 번에 commit한다. 새 root 이동이 실패하면 backup을 복구한다. Data catalog와 Sound root 중
한쪽만 교체된 부분 commit을 허용하지 않는다.

## G05. 에스더 runtime

### G05-1. 승인 edge와 clock

새 `CEstherActionSoundCueDocument`는 G03-1 문서를 parse/validate/stage/commit한다. 새
`CReplicatedActionSoundRuntime`은 순수한 due scan과 occurrence dedup만 소유하며 실제 packet,
Character animation, cutin, FMOD channel을 소유하지 않는다.

`CClientReplication::Apply_WorldSnapshot()`은 다음 두 입력을 runtime에 전달한다.

```text
player: serverTick + playerNetEntityId + eAction + actionStartTick
NPC:    serverTick + npcNetEntityId + archetypeId + strActionId + actionStartTick
```

player 호출음 key는
`playerNetEntityId/actionStartTick/cueId`, NPC 공격음 key는
`npcNetEntityId/archetypeId/actionStartTick/cueId`다. 같은 snapshot 재적용, 같은 action edge,
컷인 request 재생성은 재생하지 않는다. actionStartTick이 바뀐 다음 승인 occurrence는 다시
재생한다.

due scan은 30Hz signed tick distance로 action age를 구하고 `(previousAge, currentAge]`를 지난
cue만 한 번 제출한다. 첫 snapshot이 cue 뒤에 도착했을 때 lateness가 `lateToleranceMs` 이내면
catch-up하고, 범위 밖이면 소리를 몰아서 내지 않고 attempted로 소비한다. stale/out-of-order
snapshot은 cursor를 되돌리지 않는다.

### G05-2. 수명과 실패 격리

NPC despawn, player despawn, world reset, disconnect에서 해당 occurrence state를 지운다.
document나 catalog가 없거나 `Play_Sound()`가 실패해도 Server player/NPC, animation, cutin,
damage, level은 유지한다. 동일 cue는 실패 후 자동 retry하지 않는다. 재시도는 같은 occurrence가
나중에 중복 재생되는 위험이 있으므로 다음 정상 action occurrence부터 다시 시도한다.

Debug preview button은 제품 sound occurrence를 만들지 않는다. 별도 Debug-only audio preview가
필요하면 exact event를 직접 듣는 authoring 보조일 뿐 제품 dedup state와 분리한다.

## G06. 타깃 도끼·도넛 runtime

### G06-1. visual과 독립된 sound cursor

`CCombatObjectProjectionRuntime::Apply_Spawn()`이 logical record를 commit한 뒤 sound runtime에
object identity와 `spawnTick/serverTick`을 전달한다. `Apply_Snapshot()` 뒤에는 해당 snapshot의
`serverTick`으로 cursor를 전진시킨다.

sound attempted state는 `COMBAT_OBJECT_PROJECTION_RECORD`의 visual handle·visual retry count와
분리한다. Effect spawn 실패, handle 0, `sink.Update()` 실패와 재생성 중에도 sound timeline은
한 번만 진행한다. duplicate reliable spawn은 새 occurrence를 만들지 않는다.

age는 `(serverTick - spawnTick) * 1000 / 30`으로 계산한다. product cue의 `startMs`를 지날 때
한 번 재생하고, 첫 reliable spawn이 늦게 도착한 경우 G05와 같은 tolerance 정책을 적용한다.
object despawn, source boss 제거, world reset에서 cursor와 coalesce key를 함께 제거한다.

### G06-2. coalesce와 ID 재사용

타깃 도끼 기본 `coalescePolicy`는 `SAME_SOURCE_ARCHETYPE_SPAWN_TICK`이다.

```text
sourceBossNetEntityId / combatObjectArchetypeId / cueId / spawnTick
```

같은 wave의 player별 도끼는 한 번만 들리고, 세 wave는 서로 다른 spawnTick이므로 세 번 들린다.
도넛은 한 occurrence에 object 하나이므로 같은 정책을 써도 결과는 한 번이다. 같은
combatObjectId가 despawn 뒤 재사용되더라도 새 spawnTick이면 새 occurrence다.

도끼 hit cue는 generated 1200ms, 도넛 hit cue는 generated 1600ms에서 발화한다. object lifetime
안의 spawn cue가 receipt로 확정되면 0ms cue를 함께 지원한다. 현재 Engine의 단일
`Play_LoopingSound()` channel을 도넛에 쓰지 않고, finite one-shot만 허용한다.

### G06-3. 재생 sink

runtime sink는 `catalogOwnerId + soundEvent`를 `CSoundCueCatalog`로 resolve하고 admitted variant
중 하나를 선택한다. asset ID는 `CRuntimeAssetRoot::Resolve()`로만 실제 path가 된다.
absolute path와 extraction stage path를 C++/JSON에 저장하지 않는다. sound 실패는 해당 cue만
격리하며 logical combat object, Effect, hit와 despawn은 그대로 진행한다.

## G07. 구현 파일과 소비자 폐쇄

| 파일 | 변경 책임 | 검증 소비자 |
|---|---|---|
| `Tools/Audio/LostArkSoundSelection.json` | exact Esther/Valtan logical package admission | extraction plan/resolve test |
| `Tools/Audio/Publish-LostArkSound.ps1` | staging-first publish와 live root 직접 교체 guard | `Test-LostArkSound.ps1` |
| `Tools/Audio/Test-LostArkSound.ps1` | gameplay catalog의 모든 WAV closure 검사 | sound publish gate |
| `Tools/Audio/Publish-GameplaySoundCueCatalog.ps1` 신규 | extraction event → existing owner/event catalog의 보존 merge | publisher fixture |
| `Tools/ValtanPipeline/build_valtan_combat_object_sound_cues.py` 신규 | hitId → authoritative atMs product projection | Valtan pipeline test |
| `Data/Sound/EstherActionSoundCues.json` 신규 | player/NPC semantic action sound cue | Client document harness |
| `Data/Animation/Authored/Valtan/Valtan.combatobjectsoundbindings.json` 신규 | object/hit 기반 sound authoring | Valtan publisher |
| `Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json` 신규 | runtime flat cue product | Client document harness |
| `Data/Sound/CharacterSoundCatalog.json` | `Esther`, `ValtanCombatObject` variant bucket | `CSoundCueCatalog` |
| `Client/Public/SoundCueCatalog.h`, `Client/Private/SoundCueCatalog.cpp` | owner naming, strict asset ID/duplicate validation | Client harness |
| `Client/Public/EstherActionSoundCueDocument.h`, `Client/Private/EstherActionSoundCueDocument.cpp` 신규 | G03-1 strict document | action sound runtime |
| `Client/Public/ValtanCombatObjectSoundCueDocument.h`, `Client/Private/ValtanCombatObjectSoundCueDocument.cpp` 신규 | generated product strict document | combat-object runtime |
| `Client/Public/ReplicatedActionSoundRuntime.h`, `Client/Private/ReplicatedActionSoundRuntime.cpp` 신규 | player/NPC due scan·dedup | `CClientReplication` |
| `Client/Public/CombatObjectProjectionRuntime.h`, `Client/Private/CombatObjectProjectionRuntime.cpp` | object sound cursor와 lifecycle cleanup | `CClientReplication` |
| `Client/Public/ClientReplication.h`, `Client/Private/ClientReplication.cpp` | Server occurrence를 두 runtime에 전달, play sink | product world snapshot |
| `Tools/ActionPresentationTimelineHarness/Private/ClientPartyRegression.cpp` | parser, timing, duplicate, cleanup 회귀 | Debug/Release harness |
| `Client/Default/Client.vcxproj`, `Client/Default/Client.vcxproj.filters` | 새 H/CPP와 세 JSON을 `96.DataFiles`에 등록 | Client build/XML parse |

새 C++ 파일은 UTF-8 BOM 없음으로 만들고 `Client/Default/Client.vcxproj/.filters` 양쪽에 실제
물리 폴더와 맞게 등록한다. `EstherActionSoundCues.json`은 `96.DataFiles\Sound`, 두 Valtan
sound document는 `96.DataFiles\Animation` 아래 `None` item으로 등록한다. 기존 C++ 파일은 현재
인코딩을 유지한다.

## G08. 구현·commit·merge 순서

1. 추출 PR 최종 commit과 current branch의 merge-base를 기록한다.
2. receipt에서 네 scope의 exact owner/event/timing/variant를 확정한다.
3. selection을 확장하고 `-PlanOnly`, isolated stage publish, full-decode를 통과시킨다.
4. Esther source document와 Valtan object binding을 작성한다.
5. object cue generator와 gameplay catalog publisher를 구현해 product/catalog를 생성한다.
6. Sound root 전체 closure와 Data catalog를 transactionally 검증한다.
7. action sound document/runtime을 `CClientReplication`에 연결한다.
8. combat-object sound document/cursor/coalesce를 기존 projection lifecycle에 연결한다.
9. publisher·native harness·Debug/Release build를 통과시킨다.
10. 사용자가 실제 1~4인 Valtan에서 청취한 뒤 sound PR을 올리고 merge한다.

권장 commit은 다음 두 검증 단위지만 같은 sound PR 안에 둔다.

1. `feat(audio): publish esther and valtan occurrence sound assets`
2. `feat(client): project esther and combat-object sound cues`

첫 commit은 extraction receipt, publisher, catalog, asset closure 검증을 닫는다. 두 번째 commit은
두 cue document, runtime, project/filter, harness를 닫는다. asset commit이 Git 제외 physical
Resources를 필요로 하면 PR에는 현재 Product 문서가 참조하는 최소 dependency closure와 상대
asset ID 인계를 명시하고, 전체 추출 corpus나 raw source를 올리지 않는다.

다른 세션의 미커밋 파일을 stage하거나 정리하지 않는다. sound branch가 bug-fix branch의
`Valtan.combatobjects.json`을 병합한 다음 generator를 다시 실행하고, 생성물만 수동 편집하지 않는다.

## G09. 자동 검증 matrix

### G09-1. extraction·catalog

- receipt JSON과 extraction manifest의 event ShortID/media ID/SHA-256 일치
- required event unresolved 0, unrelated owner/event loss 0
- `Sound/...wav` relative path만 허용, absolute/drive/`..` 거부
- case-insensitive duplicate owner/event/asset ID 거부
- PCM16/48kHz/1~2 channel/positive duration와 full decode
- 현재 catalog가 참조하는 WAV 존재 100%
- `Sound/Boss/Valtan` migration coverage가 100%가 아니면 legacy path 전체 보존
- live root 대상 direct `-Replace`를 integration gate 없이 거부

### G09-2. document·publisher

- Esther player/NPC valid rows round-trip
- unknown owner kind/action/archetype, duplicate cueId, invalid start/lateness/volume 거부
- 도끼와 도넛 `triggerHitId`가 각각 정확히 하나 resolve
- `1199ms → 1200ms`, `1599ms → 1600ms` product boundary 일치
- missing/duplicate/non-timed hit와 cue가 lifetime 밖인 경우 publish 실패
- malformed 새 document가 기존 committed document를 바꾸지 않음

### G09-3. action runtime

- 같은 ESTHER_CAST snapshot 반복은 호출음 한 번
- 같은 player가 새 actionStartTick으로 다시 호출하면 한 번 추가
- 세 NPC가 각자 `esther.strike` cue를 자기 action age에 한 번 재생
- reliable NPC spawn만 받은 상태에서는 재생하지 않고 첫 clocked snapshot에서 catch-up
- tolerance 안의 late arrival은 한 번, 밖은 drop 후 몰아 재생하지 않음
- out-of-order snapshot, despawn, disconnect, reset 회귀
- missing asset/Play_Sound failure가 player/NPC/cutin을 막지 않음

### G09-4. combat-object runtime

- 도끼 `1199 → 1200`, 도넛 `1599 → 1600` crossing에서 정확히 한 번
- duplicate reliable spawn/full snapshot에서 재생 없음
- visual spawn failure와 세 번의 visual retry에도 sound 한 번
- 4인 × 세 wave 도끼에서 총 세 번의 coalesced playback
- 서로 다른 boss/source는 서로 coalesce하지 않음
- despawn/source removal/reset 뒤 state 0
- object ID 재사용 + 새 spawnTick은 새 occurrence
- stale tick과 wrap-around signed tick 비교

### G09-5. 실행 명령

구현 후 최소 실행 묶음은 다음이다.

```powershell
pwsh -File Tools/Audio/Test-LostArkSound.ps1 `
  -Root <isolated-staged-Resources-Sound> `
  -FullDecode

python Tools/ValtanPipeline/build_valtan_combat_object_sound_cues.py --check
pwsh -File Tools/Balance/Publish-GameplayBalance.ps1 -Validate
```

실제 스크립트 인자가 추출 PR 최종본에서 달라지면 최종 help를 기준으로 PLAN/RESULT의 명령을
교정한다. 이어서 `ActionPresentationTimelineHarness`의 Debug/Release 회귀와
`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`, `Release`를 실행한다.
`git diff --check`, JSON parse, `.vcxproj/.filters` XML parse도 마지막 gate에 포함한다.

## G10. 사용자 청취와 완료 판정

자동 검증은 event mapping, clock, 중복 방지, WAV 형식까지만 PASS로 판정한다. 실제 음원의 의미,
볼륨, 겹침, 타격 체감은 사용자가 직접 확인한다.

수동 smoke는 다음 순서다.

1. 에스더 세 slot을 각각 사용해 player 호출음과 해당 NPC 공격음을 구분해 듣는다.
2. 호출 입력이 Server에서 거절된 경우 아무 소리도 나지 않는지 확인한다.
3. high-jump를 1인과 4인에서 실행해 같은 wave의 도끼음이 겹치지 않고 세 wave가 각각 들리는지 확인한다.
4. 도끼 피해 순간과 1200ms cue, 도넛 피해 순간과 1600ms cue가 체감상 일치하는지 확인한다.
5. 도넛이 남아 있는 동안 다음 패턴이 시작되어도 sound가 재시작하거나 loop channel을 빼앗지 않는지 확인한다.

다음 값은 extraction receipt와 사용자 청취 전까지 명확하지 않다.

- Wei·Bahuntur와 도끼·도넛의 exact Wwise event
- 각 event 내부 variant 중 제품에 admit할 범위
- 최종 volume과 late tolerance
- loop source를 finite one-shot으로 publish할 수 있는지

이 값은 불명확한 상태로 구현 완료 처리하지 않는다. 확정된 row만 먼저 결선하고 미확정 row는
명시적으로 제외한다. 모든 대상의 receipt, 자동 검증, 사용자 청취가 끝난 뒤 RESULT를 작성하고
sound PR을 merge한다.
