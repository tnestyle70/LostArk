# 차원술사 BA 전체 모션·stage별 Effect 및 T 확정 입력 연결 수정 구현 계획

## 1. 목표와 종료 증거

- 기준선은 `origin/main@625471363811a20e296aab77c3ee4117479f4335`이다.
- 차원술사 기본 공격 `2050010`을 `_01/_02/_03/_04`의 네 독립 BA stage clip으로 복구한다.
- `actionDurationMs`는 단발 입력의 전체 모션 시간, `comboAdvanceMs`는 버퍼된 다음 LMB가 다음 stage로 전환되는 시간으로 분리한다.
- BA1~BA4 clip source `0ms`에 기존 stable Product Effect ID를 1:1 연결하고 `root / follow / action_facing / natural` 계약을 유지한다.
- 차원술사 T `2050500`의 targeting preview가 열린 뒤 valid 위치에서 LMB를 누르면 typed `GROUND_POINT` command가 서버로 전달되고, 승인 snapshot이 기존 `pc_sp_m_00_sk_sk_dimensionprison` 애니메이션을 재생하게 한다.
- V0 또는 과거 Effect payload를 복구하지 않는다. `main`의 현재 authored Effect 내용과 runtime payload는 그대로 두며, 이후 시각 authoring은 사용자가 같은 stable ID에서 수행한다.
- gameplay balance publisher, Effect publisher validation, focused Python contract, Server contract test, Client build가 실제 수정 데이터를 소비해야 자동 구현 완료다. 화면 결과는 사용자가 직접 판정한다.

## 2. 현재 실측과 교정 경계

2026-08-24 변경은 도화가의 빠른 `comboAdvanceMs`인 `276/269/494/1267ms`를 차원술사의 animation wall duration과 Server `actionDurationMs`에도 사용했다. 그 결과 BA1/BA2는 같은 `_01` clip의 `[0,300)`, `[300,600)` 구간으로 잘렸고 `_02` clip은 Product binding에서 빠졌다. BA3/BA4도 각각 `2.15991903`, `1.34175217` 배속으로 짧아졌다.

Server의 public 계약은 `comboAdvanceMs`를 buffered BA continuation boundary로만 소비하며, 입력이 없거나 MOVE/SKILL 명령을 대기할 때는 `actionDurationMs`까지 현재 stage를 유지한다. 따라서 두 시간을 같게 만든 차원술사 데이터가 교정 대상이고 공용 Server 알고리즘은 바꾸지 않는다.

`main`에는 다음 stable ID와 v13 authoring 문서가 이미 모두 존재한다.

| BA | Product Effect stable ID | 이번 작업의 payload 변경 |
|---|---|---|
| BA1 | `effect.dimensionmaster.skill.2050010.ba1.unified` | 없음 |
| BA2 | `effect.dimensionmaster.skill.2050010.ba2.unified` | 없음 |
| BA3 | `effect.dimensionmaster.skill.2050010.ba3.unified` | 없음 |
| BA4 | `effect.dimensionmaster.skill.2050010.ba4.unified` | 없음 |

이번 작업은 Element 수, delay, texture, material, source recipe 또는 content-addressed runtime hash를 고정하거나 바꾸지 않는다. Effect Tool에서 이후 내용을 교체해도 stable ID와 cue 연결만 유지되면 같은 stage에서 재생돼야 한다.

## G00. BA stage animation·Server timing·root motion

### 3. 대상 계약

| BA | stage clip | presentation wall duration | Server hit | buffered combo advance | 입력 window |
|---|---|---:|---:|---:|---:|
| BA1 | `_01`, source `0..1400ms`, `playRate=2.0` | `700ms` | `50ms` | `276ms` | `92..276ms` |
| BA2 | `_02`, natural full clip, `1.0` | `1500ms` | `43ms` | `269ms` | `179..269ms` |
| BA3 | `_03`, natural full clip, `1.0` | `1067ms` | `28ms` | `494ms` | `93..494ms` |
| BA4 | `_04`, natural full clip, `1.0` | `1700ms` | `335ms` | terminal `1700ms` | 없음 |

BA1~BA3 입력 window와 빠른 cadence는 `main`에서 유지해 입력 감각을 임의로 넓히지 않는다. BA4는 terminal stage이므로 validator 계약에 따라 `comboAdvanceMs == actionDurationMs`다.

### 4. 수정 파일과 흐름

- `Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json`
  - stage 0은 `_01` source `1400ms / playRate 2.0`으로 재생한다.
  - stage 1~3은 `_02/_03/_04` natural full clip으로 복구한다.
- `Data/Balance/PlayerSkills.json`
  - top-level mirror와 네 `comboStages[].actionDurationMs/hitTimeMs`를 full motion clock으로 복구한다.
  - BA1~BA3 `comboAdvanceMs`와 입력 window는 빠른 연계 경계로 보존한다.
- `Data/Animation/RootMotion/DimensionMaster.rootmotion.json`
  - `700/1500/1067/1700ms` stage curve와 끝 이동량 `0.8418/0.5131/0.2802/1.0404`를 복구한다.
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`
  - gameplay authoring 변경 뒤 receipt 동기화 도구로 실제 result를 갱신한다.
- `Server/Private/ServerGameplayContractTests.cpp`
  - full duration과 distinct combo boundary를 따로 고정한다.
  - 단발 BA1은 `700ms`까지 유지되고 buffered BA는 `276ms`에 BA2로 넘어감을 각각 검증한다.

공용 `sourceStartMs` parser/runtime 지원과 `CPlayerSkillSystem` 알고리즘은 다른 action의 public 기능이므로 제거하거나 우회하지 않는다.

## G01. BA1~BA4 Product Effect cue 연결

### 5. stable ID 연결

- `Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents`
  - BA1 `_01@0ms`, BA2 `_02@0ms`, BA3 `_03@0ms`, BA4 `_04@0ms`를 각 unified Product ID에 1:1 연결한다.
  - 네 cue의 `root / follow / action_facing / natural`을 유지한다.
- `Data/Effects/EffectCatalog.json`과 네 `Data/Effects/Authored/*.unified.effect.json`
  - 이미 필요한 stable ID와 v13 문서가 있으므로 수정하지 않는다.
- `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json`과 hashed authored payload
  - Effect source가 바뀌지 않으므로 이번 작업에서 publish하거나 수정하지 않는다.
  - `Publish-Effects.ps1 -Mode Validate`로 cue가 가리키는 네 stable ID의 admission만 확인한다.

`stop=natural`이므로 빠른 LMB로 다음 stage에 진입해도 현재 authored Effect의 particle tail은 자체 lifetime까지 유지한다. animation stage 종료가 Effect를 강제 종료하는 별도 경로는 만들지 않는다.

## G02. T ground-target confirm과 실제 애니메이션 연결

### 6. 확인된 단절과 수정 경계

`CClientReplication::Create_Character`는 모든 replicated Character에
`pNavigationPrototypeTag = nullptr`를 전달한다. 따라서 로컬 차원술사도
`CCharacter::Try_SampleTargetGround`를 항상 실패하고, preview 이미지는 표시돼도
`CGROUND_TARGETING_STATE::Can_Confirm()`이 한 번도 true가 되지 않는다. LMB confirm packet,
Server action, `2050500` presentation animation이 전부 이 지점에서 막힌다.

Loader는 이미 각 Level의 stable Area ID로
`Prototype_Component_Navigation_<AreaId>`를 등록한다. 다음 한 경로만 연결한다.

```text
LevelRegistry map area ID
-> CClientReplication::DESC.strMapAreaId
-> CMapNavigationContract::Resolve_Area
-> local replicated Character navigation component
-> valid LMB confirm
-> typed GROUND_POINT C2S_USE_SKILL
-> Server-approved snapshot
-> pc_sp_m_00_sk_sk_dimensionprison
```

- remote Character에는 navigation component를 추가하지 않는다.
- Client navigation은 preview의 valid/invalid 판정에만 사용한다. Server의 finite/range/current
  navigation 재검증과 승인 target XYZ 권위는 그대로 유지한다.
- `PlayerSkillTargeting.json`, `2050500` skillbinding, Server damage/timing, T Effect authored payload는
  이미 존재하므로 수정하지 않는다.
- Character Select, Bern, Development, Valtan 네 product Level이 각자의 실제 map area ID를
  replication에 명시한다. level enum이나 prototype tag를 replication이 추측하지 않는다.

구현은 `CClientReplication::DESC::strMapAreaId`와
`m_strLocalPlayerNavigationPrototypeTag`를 추가하고, `Initialize`에서
`CMapNavigationContract::Resolve_Area`의 runtime grid와 prototype tag를 검증·보관한 뒤
`Create_Character`가 locally controlled Character에만 그 tag를 전달하는 한 경로로 닫는다.
`Level_CharacterSelect.cpp`, `Level_Bern.cpp`, `Level_Development.cpp`,
`Level_ValtanArena.cpp`는 모두 `CLevelRegistry` descriptor의 `pMapAreaId`를 전달한다.

## G03. focused contract와 결과 문서

### 7. 실행형 검증과 현재 종료 상태

- `Tools/EffectPipeline/test_dimensionmaster_2050010_stage_split.py`
  - 기존 discovery 파일명은 유지하되 full-stage 연결 contract로 교체한다.
  - stage clip 1:1, full duration 대 combo advance, root-motion duration/end, animevent Product cue를 검증한다.
  - Element 수, delay bucket, runtime hash는 사용자의 후속 Effect authoring을 막지 않도록 고정하지 않는다. 네 v13 stable ID가 catalog와 authoring 문서에서 resolve되는지만 검사한다.
- gameplay balance publisher `Validate/Publish`와 balance runtime-set publisher를 실행한다.
- Effect publisher는 `Validate`를 실행한다.
- `ActionPresentationTimelineHarness`, `Server.exe --contract-test`, 관련 Debug/Release build를 실행한다.
- JSON parse, `git diff --check`, clean stage 범위를 확인한다.
- `test_ground_target_preview_prototype_scope.py`는 네 product Level의 Area ID 전달,
  navigation contract resolve, local-only Character component 연결을 검증한다.

T wiring 반영 뒤 focused ground-target test는 3 tests PASS이고, post-fix Client x64 Debug와
Release compile/link도 각각 PASS다. Release 전체
`Invoke-BuildAndRegression.ps1 -Configuration Release`는 Engine, Shared,
NetworkProtocolHarness, Server, Client compile/link와 Effect validate까지 완료한 뒤 기존
`Sync-EffectDataProject.ps1 -Check` stale registration에서 중단됐다. 이는 full regression
PASS가 아니라 기존 baseline gate이며, 자동 검증 결과의 상세 구분은 대응 RESULT가 소유한다.

### 8. 사용자 화면 확인

에이전트는 Client를 실행·조작하거나 visual PASS를 기록하지 않는다. 자동 검증 뒤 사용자가 Character Select에서 차원술사를 선택해 다음을 직접 확인한다.

1. LMB 한 번은 BA1 모션을 재생하고 약 `700ms` 뒤 종료한다.
2. 다음 LMB가 buffer되면 BA1은 `276ms`에 BA2로 전환하고 이후 `_02/_03/_04` 순서로 이어진다.
3. 현재 `main`의 BA1/BA2/BA3/BA4 authored Effect가 각 stage 시작에 한 번씩 재생된다.
4. Effect Tool에서 같은 stable ID 내용을 바꾼 뒤 publish해도 cue를 다시 연결할 필요 없이 해당 BA에서 재생된다.
5. T로 range/cursor preview를 연 뒤 valid 위치에서 LMB를 누르면 preview가 닫히고
   `pc_sp_m_00_sk_sk_dimensionprison` 애니메이션이 재생된다.
6. invalid 위치의 red preview와 RMB cancel은 계속 action을 시작하지 않는다.

사용자의 서면 관찰 전에는 `PENDING_USER_VISUAL_GATE`를 유지한다.
