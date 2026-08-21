# 2026-08-21 Esther All Effects와 Effect Family 구현 계획서

## 0. 목표와 현재 실측

이번 변경은 발탄 에스더 세 슬롯의 원본 effect family를 source evidence로 추출하고, 기존 Effect runtime과 All Effects 저작 경로에 연결한다.

```text
Esther
  Silian / Thirain | NPC_59030
    완성된 패자의 검 | npc_evt1_sk_swordofchampion_bk
  Wei | NPC_58700
    각성 도철 | npc_sk_dochul
  Bahuntur / Balthorr | NPC_59060
    아크투르스의 숨결 | npc_sk_breathofarcturus
```

`도철`은 별도 에스더 owner가 아니라 웨이의 스킬과 소환 연출이다. stable owner는 표시 이름이 아니라 `NPC_59030`, `NPC_58700`, `NPC_59060`이다. Sillian/Thirain, Waye/Wei, Bahuntur/Balthorr 표기는 검색 alias와 UI label로만 사용한다.

현재 저장소에는 세 NPC의 model, animation set, `esther.strike` action과 Server 소환 수명주기는 존재하지만 Effect cue는 없다. `Data/Animation/Authored/Esther`, `effect.esther.*`, `Client/Bin/Resources/Effect/Esther`도 없다. 따라서 All Effects tree만 추가하면 실제 `Ctrl+Z/X/C` 소환에는 아무 effect도 재생되지 않는다. 종료 상태는 다음 수직 슬라이스를 모두 포함해야 한다.

```text
UE3 Action/Projectile/ParticleSystem evidence
  -> Esther-owned runtime resources
  -> v13 direct-authored Effect documents
  -> Esther.animevents exact clip cues
  -> EffectCatalog/direct source owner index
  -> All Effects Esther tree/Open Editor/Play/Save
  -> Server-spawned CNpc action-age Effect playback
```

원본 실측은 다음과 같다.

- 실리안 action `542600`의 `FX_ESTHER_LRSA_00` closure는 Sword01, Teleport, Sword02, Sword03 ribbon, SwordSwing, Exp01/02/03과 공용 light/distortion/post를 소유한다. 현재 clip 5.2초와 같은 연합군 액션이다.
- 웨이 action `541000`의 Effect notify는 Projectile `541000.loa`를 소유한다. 이 문서는 도철 skeletal mesh와 `SK_Dochul_01/02/03`, 웨이의 양손 spear/impact/despawn particle family를 함께 소유한다. 손 particle 하나만 복원하면 요청한 호랑이 공격이 아니다.
- 바훈투르 action `542800`은 현재 clip이 아니라 구형 `Att_Battle_9_01/02/03`이므로 그 timing을 재사용하지 않는다. 전수 조사로 찾은 Projectile `531200.loa`가 `MN_YOBR_00_SK`, weapon, `MN_YOBR_00_Ani.SK_BreathOfArcturus`, `FX_ESTHER_YOBR_00`의 attack/trail family와 `PC_COMMON_ESTHER` 바훈투르 공격 sound를 한 sequence로 소유한다. 따라서 바훈투르는 Projectile 531200 timing을 exact evidence로 사용한다.

현재 브랜치는 `codex/esther-all-effects-family-0821`이며 기반 commit `2803d332`는 차원술사 T summon, 워로드 HOLD family, 그 이전 도화가 E/차원술사 BA/Effect Tool Save Hot Reload 변경을 포함한다.

사용자가 별도로 손튜닝 중인 다음 문서는 읽기만 하며 stage, rewrite, commit하지 않는다.

```text
Data/Effects/Authored/effect.dimensionmaster.skill.2050100.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050220.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050230.unified.effect.json
```

## G00. Source evidence와 실패 경계 고정

### 입력 정본

- Action: `NP_LRSA_00.loa`, `NP_DPWI_00.loa`, `MN_YOBR_00*.loa`, `COMMONACTION.loa`
- Wei projectile: `Common_Extra/XMLData/Projectile/541000.loa`
- packages: `FX_ESTHER_LRSA_00`, `FX_ESTHER_DPWI_00`, `FX_ESTHER_YOBR_00`
- NPC model/animation contract: `Data/Actors/NpcCatalog.json`
- Server roster/action lifetime: `Server/Public/EstherSkillSystem.h`

기존 `extract_action_effect_notifies.py`, particle graph/material closure, imported-effect builder와 resource cooker를 확장한다. 파일명 유사도나 다른 캐릭터 문서에 우연히 들어온 `FX_ESTHER_*` module을 정본으로 사용하지 않는다.

### provenance 계약

- 실리안과 웨이는 exact action/projectile occurrence timing을 보존한다.
- 바훈투르는 SkillEffect/Projectile 531200 identity, clip `SK_BreathOfArcturus`와 particle/model occurrence timing을 exact evidence로 보존한다.
- 누락 mesh/DDS/material closure는 흰 카드나 generic texture로 대체하지 않고 해당 occurrence를 fail-close하고 receipt에 남긴다.
- 추출 staging과 UModel 중간 산출물은 repository에 넣지 않는다. 최종 Resources-relative asset과 재현 가능한 receipt/materializer만 관리한다.

## G01. Esther effect 문서와 runtime resource closure

### 신규 정본

```text
Data/Effects/Authored/effect.esther.npc_59030.sword_of_champion.unified.effect.json
Data/Effects/Authored/effect.esther.npc_58700.awakened_dochul.unified.effect.json
Data/Effects/Authored/effect.esther.npc_59060.breath_of_arcturus.unified.effect.json
Data/Effects/Imported/Esther/...
Data/Effects/SourceEvidence/Esther/...
Client/Bin/Resources/Effect/Esther/Silian/...
Client/Bin/Resources/Effect/Esther/Wei/...
Client/Bin/Resources/Effect/Esther/Bahuntur/...
```

각 인물은 하나의 action family 문서를 가지되 occurrence의 phase identity와 source time은 유지한다. 거대한 단일 catalog stub이나 세 인물을 합친 문서는 만들지 않는다.

웨이 문서는 도철 model cue의 mesh, animation set과 particle family를 한 transaction으로 stage한다. model/animation/resource 하나라도 빠지면 도철 occurrence만 unavailable 처리하고 웨이 NPC와 다른 effect는 보존한다.

renderer/playback은 기존 sprite, mesh, sprite particle, mesh particle, decal, ribbon과 source material family를 재사용한다. Esther 전용 shader/runtime을 새로 만들지 않는다. exact material closure가 기존 family에 속하지 않을 때만 source identity와 focused WARP witness가 있는 최소 profile을 추가한다.

## G02. Generic animevents cue 계약 재사용

신규 Esther 전용 cue parser를 만들지 않는다. 검증된 `CAnimationEffectCueDocument`의 formatVersion 5 계약을 재사용한다.

```text
Data/Animation/Authored/Esther/Esther.animevents
```

`Data/Actors/NpcCatalog.json`의 세 supported NPC는 공용 cue asset ID `Esther`를 명시한다. 각 NPC load는 자기 animation set의 available clip으로 같은 문서를 filter한다. 정확한 clip, start/end, effectAssetId, root/bone anchor, follow/snapshot과 stop policy를 기존 parser가 검증한다.

파서는 다음 실패에서 기존 staged document를 보존한다.

- schema/version/asset identity 오류
- NpcCatalog에 없는 archetype/action/clip
- EffectCatalog에 없는 effect ID
- 중복 clip/start/effect occurrence
- clip 범위를 벗어난 start/end
- unsafe resource path 또는 존재하지 않는 runtime bone

## G03. 실제 CNpc action Effect playback

### 수정 범위

```text
Client/Public|Private/Npc.*
Client/Private/ClientReplication.cpp
Client/Public|Private/Effect_PresentationService.*
Client/Public|Private/Effect_ProductPrewarmQueue.*
Client/Private/Loader.cpp 또는 Valtan priority-prewarm caller
```

`CNpc`는 Server snapshot의 `strActionId`, `iActionStartTick`, current server tick을 받아 `CActionPresentationTimeline`으로 action age를 계산한다. 첫 spawn packet에는 action start tick이 없으므로 첫 clip과 cutin은 즉시 표시하고, 첫 snapshot에서 최대 한 tick 이내에 exact clip/sample과 이미 도래한 cue를 catch-up한다.

Effect spawn owner는 raw pointer가 아니라 typed weak `CNpc` owner를 가진다. root transform과 optional model bone anchor는 실제 NPC에서 resolve한다. action 변경 시 이전 `actionStartTick` occurrence만 stop하고, despawn/world reset에는 해당 NPC owner의 pending/active effect를 모두 정리한다.

runtime 실패 정책은 다음과 같다.

- cue 문서 load 실패는 NPC spawn과 animation을 막지 않는다.
- effect document/resource 준비 실패는 그 effect target만 unavailable 처리한다.
- stale/older action tick, duplicate snapshot과 duplicate cue edge는 재생하지 않는다.
- late snapshot catch-up에서 이미 `CUE_END`를 지난 effect는 spawn하지 않는다.
- action 교체/despawn cleanup 실패가 다른 NPC/character/Valtan occurrence에 전파되지 않는다.

세 Esther effect ID는 Valtan entry priority prewarm에 포함한다. 전체 global queue가 빌 때까지 NPC spawn을 막지 않으며 준비되지 않은 cue만 fail-close한다.

## G04. All Effects Esther tree와 Model View

### 수정 범위

```text
Client/Public|Private/Effect_DirectAuthoredSourceIndex.*
Client/Public|Private/Effect_Tool.*
Client/Public/CharacterPreviewPanel.h
필요 시 기존 NPC presentation service의 read-only preview entry
```

All Effects owner 선택은 player/Valtan boolean 조합이 아니라 explicit owner kind로 정리하고 `ESTHER`를 추가한다.

```text
All Effects
  Esther
    Silian | NPC_59030
      완성된 패자의 검
        phase/cue -> Play Full Effect / Open Editor
    Wei | NPC_58700
      각성 도철
        phase/cue -> Play Full Effect / Open Editor
    Bahuntur | NPC_59060
      아크투르스의 숨결
        phase/cue -> Play Full Effect / Open Editor
```

Direct source index에 `ESTHER_ACTION` owner를 추가하고 effect ID를 exact archetype/action/clip/phase에 연결한다. Esther entry는 player skill candidate나 Valtan pattern candidate로 유입되지 않는다.

Model View는 generic player body를 쓰지 않고 기존 `CNpcPresentationAssetService`와 NpcCatalog model+animation set을 재사용한다. Open Editor/Play는 선택한 exact NPC clip 한 개와 phase-local document만 stage한다. refresh 실패는 이전 Esther tree와 preview를 보존한다.

Save는 atomic authored file 저장 뒤 selected Esther effect 하나를 같은-revision catalog/renderer replacement로 교체한다. player 전용 owner count를 억지로 재사용하지 않고 exact Esther cue owner mapping을 사용한다. active occurrence는 기존 shared document를 끝까지 사용하고 다음 spawn부터 새 document를 본다. publisher, approval, admission, global refresh는 다시 도입하지 않는다.

## G05. Catalog, publisher와 project 등록

세 v13 document를 `Data/Effects/EffectCatalog.json`의 direct-authored row로 등록하고 runtime catalog publisher가 기존 4-field direct row를 생성하도록 한다. 새 authored/source evidence/materializer와 cue JSON은 `Client.vcxproj/.filters`의 `96.DataFiles`에만 등록한다. 새 C++ 파일이 필요할 때만 source/header를 기존 물리 폴더와 일치하는 filter에 추가한다.

`Client/Bin/Resources`는 최종 runtime asset만 두고 staging, UModel dump, build output을 넣지 않는다. Publish 실행은 사용자의 authored edit가 동결된 시점에만 수행한다. Validate는 concurrent edit pin이 걸리면 재시도하지 않고 정확한 파일을 보고한다.

## G06. 자동 검증

### source/materializer

- exact package SHA, action/projectile identity와 occurrence count
- 모든 Resources-relative dependency 존재와 type
- Wei 도철 mesh/animation clip 01/02/03 identity
- Balthorr SkillEffect/Projectile 531200, `SK_BreathOfArcturus`와 exact occurrence timing
- materializer idempotence와 source mutation fail-close

### focused ClientFrontendHarness

신규 mode `--effect-esther-authoring-tree-fast`에서 다음을 고정한다.

1. exact 세 NPC/archetype/action/clip/model/animation set
2. `Esther > owner > skill > phase` hierarchy와 Wei/Dochul 관계
3. generic animevents parse/filter 및 corrupt load rollback
4. unknown/duplicate owner/action/clip/effect/path/bone reject
5. DirectIndex Esther owner와 player/Valtan isolation
6. 30Hz late snapshot action-age seek/cue catch-up, duplicate/old tick reject
7. action replacement/despawn owner cleanup
8. NPC root/bone anchor와 missing-anchor occurrence isolation
9. selected Save Hot Reload 뒤 exact target만 교체되고 unrelated prepared target/active handle 보존
10. source material family별 WARP nonzero draw와 malformed tuple fail-close

### build/regression

```text
1. Python AST/unit/materializer check
2. Effect publisher Validate
3. Test-EffectPipeline.ps1
4. ClientFrontendHarness x64 Debug/Release build + focused mode
5. Engine/Shared가 바뀌지 않으면 Client x64 Debug/Release isolated full build/link
6. 관련 Network/Server contract는 protocol 또는 Server data를 바꿀 때만 재실행
7. git diff --check, JSON/XML parse, build artifact audit
```

실행 중인 canonical Client/Server는 종료, 교체, 재시작하지 않는다.

## G07. 사용자 수동 확인

자동 검증 후 사용자가 최신 Debug Client에서 직접 확인한다.

1. All Effects의 별도 Esther root와 세 인물/스킬/phase tree가 보이는지 확인한다.
2. 실리안은 등장/충전/대검 휘두르기/impact가 source timing으로 이어지는지 확인한다.
3. 웨이는 도철 호랑이 model animation과 양손/impact particle이 하나의 action으로 이어지는지 확인한다.
4. 바훈투르는 중앙의 거대한 원형 장판과 attack/trail family가 Projectile 531200 sequence와 4.033초 clip 안에서 의도한 시점과 크기로 보이는지 확인한다.
5. Open Editor에서 phase를 저장한 뒤 재시작 없이 다음 Play와 실제 `Ctrl+Z/X/C` 소환에 적용되는지 확인한다.
6. action 종료/despawn 뒤 trail, particle, model cue가 남지 않는지 확인한다.

시각 fidelity와 manual first pixel은 사용자의 서면 관찰 전에는 PASS로 기록하지 않는다.

## G08. 완료 단위

다음이 모두 성립할 때 한 commit으로 완료한다.

- 세 Esther owner와 실제 스킬 관계가 stable NPC ID로 고정된다.
- 세 effect family가 Esther-owned resource/document/cue로 존재한다.
- All Effects Open/Play/Save와 실제 CNpc action playback이 같은 cue/document 정본을 소비한다.
- renderer는 기존 Effect runtime을 재사용하고 source 근거 없는 broad family를 추가하지 않는다.
- focused harness, publisher validation, Effect pipeline, Debug/Release build가 통과한다.
- 세 사용자 dirty DimensionMaster 문서는 stage/commit에서 제외된다.
- RESULT는 exact/source-derived, PROJECT_TUNED, 자동 검증, 사용자 육안 대기를 분리한다.
