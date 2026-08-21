# 2026-08-21 4캐릭터 Effect Cue와 BA 회귀 수정 계획

## 1. 목표

창술사·도화가·차원술사·워로드의 기존 손튜닝과 renderer family를 보존하면서, 실행 파일과 Effect runtime 데이터가 서로 다른 시점이던 문제를 제거한다. 건슬링어와 슬레이어는 이번 범위가 아니다.

- 차원술사 LMB는 물리 클릭 한 번당 Server command 한 번만 제출한다.
- 차원술사 BA cue는 `clip01/02/04 -> ba1.unified`, `clip03 -> ba3.unified`를 유지한다.
- 도화가의 일부 누락 cue가 클래스 animevents 전체를 rollback하지 않게 한다.
- 워로드 17240 HOLD의 Start/Charge/Release가 `ba1/ba2/ba3`과 정확히 연결되는지 고정한다.
- Effect Tool은 같은 문서를 공유하는 여러 stage를 자동 연속 재생하지 않고 선택한 Product cue 한 개만 재생한다.
- authored source, sealed runtime catalog, Client EXE를 한 스냅샷으로 publish/build한다.

## 2. 확인된 원인

### 2.1 실행 스냅샷 불일치

사용자가 확인한 Debug Client는 09:40 빌드였지만 인접 EffectCatalog는 07:26의 192행 구본이었다. 차원술사 A source는 34 elements인데 실행 runtime은 10 elements였고, 도화가 Product cue 4개가 runtime에서 누락됐다.

### 2.2 차원술사 한 클릭 두 타

`CBASIC_ATTACK_REPEAT_SCHEDULER`가 LMB hold 180ms 뒤, 이후 100ms마다 COMBO command를 재전송했다. `ba1.unified`가 애니메이션을 두 번 소유한 것이 아니다.

### 2.3 도화가 BA 전체 무음

다음 네 source target은 존재하지만 192행 runtime catalog에는 없었다.

- `effect.artist.skill.31050.clip1.unified`
- `effect.artist.skill.31050.clip2.unified`
- `effect.artist.skill.31210.ba4.unified`
- `effect.artist.skill.31950.unified`

하나의 target 실패가 Artist animevents 전체를 rollback해 정상 BA1~4도 함께 사라졌다.

### 2.4 워로드 17240

정본은 COMBO가 아니라 HOLD 3 phase다. `clip01 -> ba1`, `clip02 -> ba2`, `clip07 -> ba3`이며 clip03/04는 Charge 연속/loop다. 기존 family UI는 이 데이터를 삭제하지 않고 Hold Start/Charge/Release로 표시한다.

## 3. 구현 단위

### G01. LMB physical edge

- `Client/Public/PlayerController.h`
- `Client/Private/PlayerController.cpp`
- `Data/Balance/PlayerSkills.json`
- balance provenance receipt와 Server contract

raw physical up→down edge당 한 번만 제출한다. 차원술사 nonterminal input window는 실제 stage duration `1400/1500/1067ms`까지 열어 명시적 다음 클릭을 보존한다.

### G02. Cue target 부분 격리

- `Client/Public/AnimationEffectCueDocument.h`
- `Client/Private/AnimationEffectCueDocument.cpp`
- `Client/Private/Character.cpp`

구조·clip·transform·duplicate 검증은 전체 문서에 적용한다. 그 뒤 catalog에 없는 exact target만 `UnavailableEffectAssetIds`로 격리하고 유효 sibling cue는 commit한다.

### G03. Runtime publish

`Publish-Effects.ps1 -Mode Publish`로 obsolete legacy Artist ba4를 제거하고 누락 4개를 포함한 195행 runtime catalog와 content-addressed sealed 문서를 생성한다. 사용자 손튜닝 Q/S/F authored 문서도 이번 통합의 정본 입력으로 함께 보존한다.

### G04. 실행형 회귀

- LMB edge: tap/hold/UI block/release/new press
- Artist BA: skillbinding→animevents→catalog→preview exact 4 stage
- DimensionMaster: `ba1/ba1/ba3/ba1`
- Warlord 17240: `ba1/ba2/ba3`와 clip01/02/07
- four-class runtime parity 195행
- malformed/duplicate cue rollback과 unavailable sibling isolation

## 4. 검증 및 병합 순서

1. Gameplay Balance Validate/Publish와 receipt parity
2. Effect Publish/Validate 및 EffectPipeline 회귀
3. ClientFrontendHarness focused modes
4. Engine→UpdateLib→Shared/NPH→Server→Client Debug/Release build
5. `git diff --check`, JSON/XML, catalog hash/path 검사
6. feature branch commit/push, PR로 main 병합
7. 동일 commit의 EXE와 DataFiles를 canonical 실행 경로에 배치

자동 검증은 command/stage/clip/effect ID와 draw/runtime 구조까지만 판정한다. 실제 화면의 모양과 타이밍 체감은 사용자가 Character Select와 All Effects에서 확인한다.

## 5. 완료 조건

- 한 번의 LMB hold가 다음 BA stage를 자동 버퍼링하지 않는다.
- 도화가의 일부 unavailable cue가 BA 전체를 끊지 않는다.
- 4캐릭터 active Product Effect union과 runtime catalog가 195행으로 일치한다.
- 기존 도화가 E, 차원술사 R/A/T, 워로드 HOLD family와 사용자 Q/S/F 손튜닝이 유지된다.
- Client/Server Debug·Release 빌드와 관련 publisher/harness 결과를 RESULT에 실제 실행값으로 기록한다.
