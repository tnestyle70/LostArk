# 2026-08-22 창술사 D·F Product Effect 복원 구현 계획

기준 branch: `codex/lance-df-v1-restore`

기준 commit: `d6684e66` (`origin/main`)

연결 정본:

- [`4캐릭터·Valtan Effect V1 전체 마이그레이션 마스터 계획`](2026-08-22_FOUR_CHARACTER_VALTAN_EFFECT_V1_FULL_MIGRATION_MASTER_PLAN.md)
- [`Effect Family Runtime ABI 복원 가이드`](../../TEAM/EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md)

## 0. 목표와 완료 기준

이 작업은 사용자가 확정한 `V1_COMPLETE` 정의 중 **첫 픽셀 admission 구간**만 소유한다.

```text
V1_COMPLETE
= 올바른 carrier
+ family별 RT0 Base HLSL
+ texture/channel/scalar/DynamicParameter 배선
+ blend/depth
+ attachment/timing
+ Effect Tool 편집·저장
+ 사용자 육안 승인
```

이번 변경의 종료 증거는 `V1_COMPLETE`가 아니라 다음이다.

```text
창술사 D(34110 반월섬)와 F(34150 맹룡열파)가
Product Effect cue -> runtime catalog -> published Authored document로
실제 join되어 나머지 창술사 스킬과 같은 실행 tier에 들어간다.
```

`family별 typed RT0` 승격, 손튜닝, 육안 승인은 이 문서의 완료 조건이 아니다.

## 1. 현재 실측

`Data/Balance/PlayerSkills.json`의 창술사 D·F는 다음이다.

| slot | skillId | displayName | clip |
|---|---|---|---|
| D | 34110 | 반월섬 | `flm_sk_crescentsweep` |
| F | 34150 | 맹룡열파 | `flm_sk_crushingblow` |

두 스킬이 화면에 아무것도 내지 않는 원인은 셰이더나 carrier가 아니라 **admission 누락 두 곳**이다.

```text
Data/Effects/Authored/effect.lancemaster.skill.34110.unified.effect.json   존재 (88 element)
Data/Effects/Authored/effect.lancemaster.skill.34150.unified.effect.json   존재 (186 element)
Data/Effects/Imported/LanceMaster/CurrentCombat/...                        source receipt·converted 존재

Data/Effects/EffectCatalog.json                                            34110/34150 없음
Data/Animation/Authored/LanceMaster/LanceMaster.animevents                 두 clip에 effectref=asset cue 없음
```

`Tools/EffectPipeline/Publish-Effects.ps1`의 `Get-ActiveProductEffectIds`가 네 class animevents의
`effectref=asset` payload를 Product membership 정본으로 사용하므로, cue가 없으면 catalog에 넣어도
runtime catalog에 실리지 않는다. 반대로 cue만 추가하면 catalog join 실패로 publish가 fail-closed한다.
따라서 두 곳을 같은 변경 단위로 연다.

## 2. 변경 파일

| 파일 | 변경 |
|---|---|
| `Data/Effects/EffectCatalog.json` | `effect.lancemaster.skill.34110.unified`, `...34150.unified` 두 `DIRECT_AUTHORED_DOCUMENT_V13` 항목 추가 |
| `Data/Animation/Authored/LanceMaster/LanceMaster.animevents` | `flm_sk_crescentsweep`, `flm_sk_crushingblow`에 `effectref=asset` cue 2행 추가, 헤더 row count `3134 -> 3136` |
| `Data/Effects/Authored/effect.lancemaster.skill.34110.unified.effect.json` | 비어 있던 `particleSystem`에 identity modifier 4-field 추가 |
| `Data/Effects/Authored/effect.lancemaster.skill.34150.unified.effect.json` | 동일 |
| `Client/Bin/DataFiles/Effect/` | publisher 생성물 (직접 편집하지 않음) |

C++, HLSL, `.vcxproj`, `.filters` 변경은 없다.

## 3. G 구현 순서

### G00. admission 결손 실측

`PlayerSkills.json -> skillbindings -> animevents cue -> EffectCatalog -> Authored document` 순서로
D·F가 어느 단계에서 끊기는지 확인한다. 검증: 두 skillId가 catalog와 cue 양쪽에서 부재.

### G01. catalog 등록

`Data/Effects/EffectCatalog.json`에 skillId 오름차순 위치로 두 항목을 삽입한다.
형식은 같은 파일의 다른 `.unified` 항목과 동일한 `effectAssetId / payloadKind / authoringPath` 3필드다.
검증: `python -c json.load` parse와 항목 수 `316 -> 318`.

### G02. Product cue 등록

`LanceMaster.animevents`의 cue block에 clip 이름 알파벳 순서로 2행을 추가하고 헤더 row count를 갱신한다.
행 형식은 기존 41행과 동일한 v5 계약을 그대로 사용한다.

```text
"<clip>" EFFECT startms=0 payload="<effectAssetId>" effectref=asset anchor="root" follow=follow stop=natural px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1
```

검증: `AnimationEffectCueDocument.cpp`의 `ActualRows != DeclaredRows` 계약과 일치하도록 헤더가 `3136`.

### G03. document 결손 보정

두 Authored document의 `particleSystem`이 `{}`여서 publisher의 v8 이상 필수 modifier 검사에 걸린다.
live document와 같은 identity 값을 넣는다.

```json
"particleSystem": {
  "uniformScaleMultiplier": 1.0,
  "yawOffsetDegrees": 0.0,
  "directionYawDegrees": 0.0,
  "initialSpeedMultiplier": 1.0
}
```

검증: `Publish-Effects.ps1 -Mode Validate` PASS.

### G04. publish와 runtime join 확인

`Publish-Effects.ps1 -Mode Publish` 후 runtime catalog가 `205 -> 207`이 되고 두 asset의
`authoredDocumentPath`가 실제 파일로 존재하는지 확인한다.

### G05. 자동 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Test-EffectPipeline.ps1
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Test-EffectDataProjectRegistration.ps1
```

추가로 `CEffectPresentationService::Estimate_DocumentBudget`과 같은 식으로 두 document의 admission
cost를 계산해 `OWNER_BUDGET` 안에 있는지 확인한다. 초과하면 spawn 시점에 통째로 거부되므로 admission만
해서는 화면에 나오지 않는다.

## 4. 이번 변경이 닫지 않는 것

- typed `material.execution` (`runtimeMaterialV2`) 승격. D·F는 전부 `sourceProfile` tier다.
- family별 RT0 Base HLSL 신규 작성. 두 문서가 쓰는 4개 profile은 이미 runtime에 존재한다.
- 손튜닝, `USER_APPROVED`, `manual first pixel`.
- 나머지 class·slot의 동일 결손. 별도 감사 결과는 RESULT에 기록한다.
