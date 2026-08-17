# 2026-08-18 발탄 페이즈·패턴·제품 이펙트 연결 구현 계획

branch: `feature/effect-cooked-shader-recovery-and-transform-fix`

기존 네 캐릭터의 `EffectCatalog -> Product prewarm -> authoritative action edge ->
CEffectPresentationService` 경로를 발탄에도 연결한다. Effect Tool에서는 발탄 9페이즈,
32패턴, 121스테이지를 4단 트리로 모두 탐색하고, 게임에서는 서버가 승인한 정확한
`patternId/actionId/stageIndex/actionStartTick`에 맞춰 발탄 이펙트를 재생한다.

이번 사용자 요청으로 직전 계획의 두 비목표였던 `EffectCatalog 등록/게임 내 재생`과
`6방향 서버 판정`을 범위에 포함한다. 화면 결과는 사용자가 직접 확인하고 튜닝한다.

---

## G00. 현재 실측과 정본

```text
발탄 encounter                     9 표시 페이즈 / 32 패턴 / 121 스테이지
소스 clip이 있는 스테이지          121
소스 emitter가 있는 스테이지        99
의도적으로 조용한 스테이지          22
생성 대상 Effect 문서               99
생성 element                        3,106
  원본 emitter 기반                 3,098
  115줄 판정 가이드                    8
기존 420633 canary                   별도 보존, 제품 cue 생성 대상에서는 제외
```

데이터 정본은 다음처럼 나눈다.

| 정본 | 책임 |
|---|---|
| `Data/Encounters/Valtan/ValtanEncounter.json` | 서버 패턴·스테이지·시간·피격 형상 |
| `Data/Animation/Authored/Valtan/Valtan.patternbindings.json` | `actionId -> model clip` |
| `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json` | `actionId -> Product Effect cue` |
| `Data/Effects/Authored/effect.valtan.*.effect.json` | 사용자가 튜닝할 시각 문서 |
| `Data/Effects/EffectCatalog.json` | 생성 가능한 Effect 정의 |
| `Client/Bin/DataFiles/Effect` | publisher가 교체하는 제품 runtime 결과 |

`Valtan.patterneffects.json`은 420633의 풍부한 source evidence를 계속 소유한다. 99개 생성
문서에 가짜 evidence를 만들지 않고, 가벼운 제품 cue 문서를 별도로 둔다.

---

## G01. 종료 목표

```text
1. All Effects > Valtan
   PHASE -> PATTERN -> STAGE/ANIMATION -> EFFECT 4단 트리

2. Open / Play
   stage 행에서 문서를 열면 발탄 Model View와 해당 clip이 같이 선택됨

3. All Effects
   명명 규칙으로 생성한 99문서와 420633 canary를 숨김 없이 표시

4. Product runtime
   99문서를 catalog에 publish하고 Valtan Arena 로딩 중 준비

5. authoritative playback
   CValtan이 서버 snapshot의 stage edge와 action age로 exact cue를 1회 spawn/late-seek

6. 115줄 패턴
   FIRST_SMASH는 실제 6방향 서버 판정, SECOND_SMASH는 기존 arena-wide wipe 판정 유지
   예고/충격 이펙트에는 사용자가 바로 튜닝할 수 있는 직관적 가이드가 존재
```

화면의 첫 픽셀, 색, 크기, 위치, occurrence fidelity는 자동 PASS로 처리하지 않는다.

---

## G02. Effect Tool 4단 트리와 재시딩

### G02.1 트리 계약

`CValtanPatternTree`는 encounter의 health-bar gate로 9개 표시 페이즈를 파생한다. 페이즈는
저장 ID가 아니라 화면 그룹이며, 패턴과 문서를 복사하지 않고 기존 벡터의 index를 참조한다.

한 stage에는 둘 이상의 문서가 붙을 수 있다.

```text
PATTERN_EFFECT_BINDING   420633 source-evidence canary
NAMING_RULE              effect.valtan.<pattern-slug>.<stage-slug>
```

`VALTAN_STAGE_VIEW::Effects`가 둘을 함께 들고, `Get_EffectCount()`는 effect가 있는 stage 수,
`Get_EffectDocumentCount()`는 실제 문서 행 수를 센다. 트리 렌더 중 JSON을 디코드하지 않고,
Open/Play 명령 때만 문서를 읽는다.

### G02.2 저작 화면

```text
All Effects > Valtan
  OPENING
  PHASE 1..9
    [Gate] 패턴
    [Rotation] 패턴
      stageId / kind / duration / hit shape / clip
        effectAssetId [Open] [Play] [Solo]
```

검색은 pattern/stage/action/clip/effect ID를 대상으로 하고, 해당 조상 노드를 함께 연다.
`Repeat rotation in every phase`는 같은 패턴을 복제하지 않고 alias로 다시 보여준다.

### G02.3 emitter 재시딩과 8슬롯

시더는 ParticleSystem 전체가 아니라 material binding emitter를 element 하나로 만든다.
표준 텍스처 슬롯은 다음 8개다.

```text
base / noise / mask / emissive / dissolve / base2 / mask2 / noise2
```

8개를 넘는 source reference는 버리지 않고 `unboundResources`에 보존한다. mesh만 있고 base가
없는 element는 WModel의 material을 쓰도록 `useModelMaterial=true`로 시딩한다.

중립 초기값은 복원 수치가 아니라 눈으로 튜닝하기 위한 시작점이다.

```text
scale                 1.0
maxParticles          8
burstCount            8
particle lifetime     0.6..1.0s
element lifetime      encounter stage duration
sourceRecipe.enabled  false
```

---

## G03. 발탄 Product cue와 catalog

신규 source 문서 `Valtan.patterneffectcues.json`은 다음 exact root를 갖는다.

```json
{
  "schema": "lostark.valtan-pattern-effect-cues",
  "formatVersion": 1,
  "ownerArchetypeId": "BOSS_VALTAN",
  "cues": []
}
```

각 cue는 다음 11필드를 정확히 소유한다.

```text
bindingId, patternId, stageId, actionId, effectAssetId,
anchorSlotId, followPolicy, stopPolicy, startMs, endMs, localTransform
```

`CValtanPatternEffectCueDocument`는 `parse -> validate -> stage -> commit`을 지킨다.
schema/version/owner, stable ID, 중복, encounter tuple, stage duration, finite transform,
follow/stop policy를 전부 통과한 뒤에만 이전 문서를 교체한다. Product load는 추가로 runtime
catalog와 spawn admission을 검사한다. 실패는 발탄 spawn/서버 상태를 막지 않고 이펙트만
격리한다.

시더의 `--write`는 99 Effect 문서, cue 문서, source EffectCatalog의 99개
`DIRECT_AUTHORED_DOCUMENT_V13` 행을 같은 deterministic projection으로 갱신한다.
publisher는 cue 집합과 authored Valtan 문서 집합의 exact equality를 검사한다.

---

## G04. 권위 스테이지 재생과 수명

`CValtan::Apply_NetworkState`는 기존 서버 snapshot 검증과 animation seek가 성공한 뒤에만
Effect cue를 소비한다.

```text
accepted snapshot
  -> patternId/actionId/stageIndex/actionStartTick commit
  -> edge/abort면 이전 actionStartTick의 pending/active boss Effect만 취소
  -> edge면 occurrence-local spawned binding set 초기화
  -> action age가 startMs를 지난 cue 선택
  -> EFFECT_SPAWN_DESC.pBossOwner = CValtan
  -> actionStartTick + patternSequence + stageIndex + bindingId occurrence ID
  -> initialSampleTimeSeconds로 late seek
  -> CEffectPresentationService::Spawn
```

반복 snapshot은 같은 binding을 다시 spawn하지 않는다. cue spawn 실패는 로그와 해당 표현만
격리하고 snapshot 적용 결과를 실패로 바꾸지 않는다. `cue_end`는 stage 구간에서 끝나며,
death/despawn/level clear에는 `Stop_BossOwner`가 잔여 인스턴스를 정리한다.

애니메이션 binding이 없거나 손상된 경우에는 generic boss clip으로 폴백하되, encounter와
root/bone anchor가 유효한 Effect cue는 독립적으로 유지한다. 같은 프레임에 A -> B snapshot이
연속 소비돼도 `Stop_BossAction(owner, oldActionStartTick)`가 A의 pending cue를 먼저 제거한다.

Valtan Arena activation 전에는 99개 boss target과 현재 player class의 Product cue를 priority
queue로 등록하고 incremental prewarm probe가 정리될 때까지 loading progress를 유지한다.
런타임 spawn은 준비되지 않은 문서를 동기 I/O로 우회하지 않는다.

Character Select의 Server Arena audition은 모든 입장에 99개를 강제 준비하지 않는다. 사용자가
Valtan spawn을 선택했을 때만 99개를 priority queue에 추가하고, 해당 boss target set이 전부
prepared된 뒤 prototype과 Server spawn request를 정확히 한 번 진행한다. 기존 player queue는
보존하며 준비/전송/응답 timeout은 재시도 가능한 presentation 상태로 격리한다.

---

## G05. 115줄 6방향과 전멸 이펙트

stable pattern/stage/action/effect ID는 바꾸지 않는다. 직관성은 표시 이름과 binding ID로 준다.

| stage | 표시/binding 의미 | 서버 판정 | 저작 가이드 |
|---|---|---|---|
| WINDUP | `six-direction-telegraph` | NONE | 0°/60°/120° 중심축 3개 |
| FIRST_SMASH | `six-direction-impact` | `SIX_DIRECTIONS` | 0°/60°/120° 중심축 3개 |
| INTERVAL | `arena-wipe-telegraph` | NONE | 반경 100 ring |
| SECOND_SMASH | `arena-wipe-impact` | CIRCLE r=100 | 반경 100 ring |

`SIX_DIRECTIONS`는 boss yaw 기준 0°·60°·120°의 centered rectangle 세 개 합집합이다.
각 rectangle의 half-length 14, half-width 2.2가 양쪽으로 뻗어 정확히 여섯 팔을 만든다.
Shared primitive, Server catalog/parser/brain/collision, Balance publisher/tool, Effect Tool shape
요약과 contract test를 한 변경 단위로 갱신한다. 네트워크는 이미 action/stage/yaw를 복제하므로
packet 변경이 없다.

`SECOND_SMASH`는 기존 100000% 일반 damage + defense/counter 정책을 그대로 유지한다.
이번 변경은 별도 `LETHAL` primitive를 만들거나 counter 정책을 바꾸지 않는다.

---

## G06. 변경 파일군

```text
Client/Public|Private/ValtanPatternTree.*
Client/Public|Private/ValtanPatternEffectCueDocument.*                 신규
Client/Public|Private/Valtan.*
Client/Public|Private/Effect_DirectAuthoredSourceIndex.*
Client/Public/Effect_PresentationService.h
Client/Public|Private/Level_Loading.*
Client/Public|Private/Level_CharacterSelect.*
Client/Private/CharacterSelectArenaSpawnGate.h                         신규
Client/Private/ClientReplication.cpp
Client/Private/Effect_Tool.cpp
Client/Private/BalanceTool.cpp
Client/Default/Client.vcxproj(.filters)

Shared/Public|Private/Gameplay/CombatCollisionContract.*
Server/Public/GameplayCatalog.h
Server/Private/GameplayCatalog.cpp
Server/Private/ValtanBrain.cpp
Server/Private/ServerCollisionSystem.cpp
Server/Private/ServerGameplayContractTests.cpp

Tools/EffectPipeline/build_valtan_stage_effects.py
Tools/EffectPipeline/test_build_valtan_stage_effects.py                 신규
Tools/EffectPipeline/Publish-Effects.ps1
Tools/GameplayPipeline/Publish-GameplayBalance.ps1

Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json           신규
Data/Effects/EffectCatalog.json
Data/Effects/Authored/effect.valtan.*.effect.json
Data/Encounters/Valtan/ValtanEncounter.json
Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json
```

신규 C++ 두 파일은 `Client.vcxproj`와 `.filters`에 등록한다. generated runtime catalog와
Gameplay bootstrap은 publisher만 교체한다.

---

## G07. 검증과 종료 기준

```text
python -m unittest Tools.EffectPipeline.test_build_valtan_stage_effects -v
powershell -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
powershell -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Publish
powershell -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate

Shared Debug/Release build
Server Debug/Release build + Server.exe --contract-test
Client Debug/Release build
focused parser/source-index tests 또는 /Zs compile
ClientFrontendHarness --character-select-valtan-prewarm-fast
git diff --check
```

자동 종료 증거는 `99 cues = 99 authored documents`, `22 silent stages`, runtime catalog의
99 Valtan entries, server six arms hit/six gaps miss, Server contract failures 0, Client build
성공이다.

수동 종료 증거는 사용자가 직접 다음을 확인한 뒤에만 기록한다.

```text
Effect Tool > All Effects > Valtan에서 4단 트리와 99문서가 보임
FLOOR_WIPE_130의 Open/Play가 해당 clip과 문서를 같이 재생함
Server + Client Valtan에서 6방향 예고/충격과 전멸 예고/충격이 stage에 맞춰 보임
Character Select Server Arena의 Valtan spawn이 준비 뒤 한 번만 요청되고 Effect가 보임
네 캐릭터 Effect와 발탄 Effect의 크기·위치·색을 눈으로 튜닝 가능
```

---

## G08. 하지 않는 것

```text
원본 Cascade 수치가 없는 element의 크기·속도·색을 복원값이라고 주장하지 않는다.
Valtan.patterneffects.json에 가짜 source evidence 99행을 만들지 않는다.
420633 canary의 stable ID나 evidence binding을 변경하지 않는다.
Client 로컬 AI를 제품 Valtan 판정에 사용하지 않는다.
전멸기를 새로운 절대 사망/LETHAL 계약으로 바꾸지 않는다.
에이전트가 Client/UI를 실행·조작하거나 visual PASS를 대신 선언하지 않는다.
```
