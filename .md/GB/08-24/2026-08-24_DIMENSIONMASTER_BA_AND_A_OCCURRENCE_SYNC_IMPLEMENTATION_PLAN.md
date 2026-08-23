# 차원술사 BA·A occurrence 동기화 구현 계획

## 1. 목표

- 차원술사 기본 공격 `2050010`의 첫 입력에서 BA1과 BA2 검격이 함께 보이는 회귀를 제거한다.
- 도화가 `31000`의 빠른 4단 cadence를 기준으로 차원술사 BA1~BA4의 Server action, animation, Effect clock을 맞춘다.
- 차원술사 A `2050210`의 한 aggregate Effect를 A1~A4 Product occurrence로 분리해 전체 A animation 안에서 각각 선택·재생·손튜닝할 수 있게 한다.
- 모든 제품 Effect는 차원술사 `root`를 `FOLLOW + ACTION_FACING`으로 사용하며 Server 권위 action을 바꾸지 않는다.

## 2. 현재 원인과 결정

### 2.1 BA1/BA2

- 정답 비교군인 도화가 `31000`은 네 Server stage, 네 clip, 네 Product Effect가 1:1이다.
- 차원술사 BA1 clip `pc_sp_m_00_sk_att_battle_1_01`에는 source 200ms와 400ms에 두 검격 occurrence가 있다.
- 현재 stage 0은 source `0..1400ms`를 `playRate=2`로 한 번에 소비하고, `ba1.unified`도 200ms/400ms 요소를 함께 가진다. 따라서 LMB 한 번에도 두 검격이 보인다.
- 단순 700ms 절반은 두 occurrence를 모두 앞 구간에 포함하므로 해결이 아니다. 두 occurrence 사이인 300ms를 경계로 사용한다.

```text
BA1 = clip01 source [0, 300)
BA2 = clip01 source [300, 600)
BA3 = clip03 전체
BA4 = clip04 전체
```

- playable skillbinding에 `sourceStartMs`를 추가하고 Valtan이 이미 사용하는 absolute source-window clock을 Character와 Effect Tool에 연결한다.
- explicit trim 끝은 half-open `[start,end)`으로 처리해 source 300ms cue가 BA1 끝과 BA2 시작에서 중복되지 않게 한다.
- BA1 문서는 200ms occurrence만 남기고, BA2/BA3/BA4 cue는 각각 기존 `ba2/ba3/ba4.unified`를 사용한다. BA2의 stage 밖 signal tail은 제거한다.
- BA1~BA4 wall cadence는 도화가 transition 기준 `276/269/494/1267ms`에 맞추며, source 구간 전체 자세는 `playRate`로 보존한다.
- root motion은 새 source window와 wall duration으로 재슬라이스하고 각 stage 시작을 0으로 상대화한다.

### 2.2 A1~A4

- 현재 `effect.dimensionmaster.skill.2050210.unified`의 12개 요소는 delay `0.25/0.60/0.90/1.30s` 네 occurrence로 정확히 분리된다.
- 네 bucket을 `a1~a4.unified` 문서로 나누고 각 문서 내부 delay를 cue 기준 0으로 rebase한다.
- `pc_sp_m_00_sk_sk_willowrend`에 source `250/600/900/1300ms` Product cue를 두며 모두 `root / follow / action_facing / natural`을 유지한다.
- 기존 aggregate 문서는 기록으로 보존하되 Product cue에서는 제거해 이중 draw를 막는다.
- A는 하나의 ACTIVE animation 안에서 Effect occurrence만 분리하므로 `PlayerSkills.json`, skillbinding, Server gameplay timing은 변경하지 않는다.

## 3. 구현 단위

1. `ANIMATION_SKILL_CLIP -> CCharacter::CLIP_STEP -> ACTION_PRESENTATION_CLIP_TIMING`에 playable `sourceStartMs`를 연결한다.
2. Character start/seek/finish와 Effect Tool Product preview가 동일한 source window와 play rate를 소비하게 한다.
3. preview candidate resolver가 같은 clip을 재사용한 stage를 source cue window로 구분하게 한다.
4. 차원술사 BA skillbinding, Server stage timing, root motion, Product cue와 BA1/BA2 Effect 내용을 동기화한다.
5. A aggregate 12개 요소를 네 문서로 무손실 partition하고 catalog/animevents/materializer 검증을 갱신한다.
6. parser rollback, half-open cue, exact stage preview, BA/A 데이터 정합을 focused harness로 고정한다.

## 4. 주요 변경 파일

- `Client/Public/AnimationSkillBindingDocument.h`
- `Client/Private/AnimationSkillBindingDocument.cpp`
- `Client/Public/Character.h`
- `Client/Private/Character.cpp`
- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`
- `Client/Private/AnimationEffectCueDocument.cpp`
- `Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json`
- `Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents`
- `Data/Animation/RootMotion/DimensionMaster.rootmotion.json`
- `Data/Balance/PlayerSkills.json`
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`
- `Data/Effects/EffectCatalog.json`
- `Data/Effects/Authored/effect.dimensionmaster.skill.2050010.ba1.unified.effect.json`
- `Data/Effects/Authored/effect.dimensionmaster.skill.2050010.ba2.unified.effect.json`
- `Data/Effects/Authored/effect.dimensionmaster.skill.2050210.a1~a4.unified.effect.json`
- 관련 publisher/materializer와 focused harness

## 5. 검증 계획

1. 모든 변경 JSON parse와 animation binding fail-close/round-trip.
2. BA source window `[0,300)`, `[300,600)` 및 source 300ms cue 단일 귀속 검사.
3. BA 네 stage Server timing, animation wall duration, root-motion duration 일치 검사.
4. A 네 문서 element ID union이 기존 12개와 정확히 같고 중복·누락이 없는지 검사.
5. A Product cue가 정확히 4개이며 aggregate Product cue가 0개인지 검사.
6. Effect publisher와 gameplay balance/runtime-set publisher Validate/Publish.
7. 관련 focused harness와 Debug/Release Server contract test.
8. Engine/Shared/Server/Client Debug/Release 정본 build 및 `git diff --check`.
9. Client 화면·조작·visual fidelity는 사용자가 직접 Character Select와 Effect Tool에서 판정한다.
