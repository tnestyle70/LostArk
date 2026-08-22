# 2026-08-22 Valtan Carrier V1 Product 재기준선 결과

## 1. 판정

이번 변경은 발탄의 구형 clip aggregate Product를 폐기하고, 검토된 source branch에서
`pattern + action + ordered clip occurrence + source occurrence + carrier`가 정확히 join되는 행만
다시 만든 carrier-first 기준선이다.

현재 자동 구현 상태는 `CARRIER_V1_COMMON_TRANSLUCENT / USER_VISUAL_PENDING`이다. 정확한
Sprite/Mesh/Decal과 애니메이션 cue는 연결됐고 Effect Tool에서 손튜닝할 수 있지만, 원본 family별
RT0 식과 사용자 육안 승인이 아직 없으므로 `V1_COMPLETE`나 원본 visual parity로 부르지 않는다.

## 2. 기존 V0 제거

기존 발탄 Product의 system-wide mesh fallback과 clip-name aggregate 행을 다음처럼 제거했다.

| 항목 | 결과 |
|---|---:|
| strict legacy V0 행 | 3,032 |
| strict legacy 문서 | 97 |
| 제거한 기존 Product 행 전체 | 3,612 |
| retire한 boss-root Effect / cue | 105 / 105 |
| 남은 old non-exact boss-root 행 | 0 |
| 중복 clip root owner | 0 |

retire한 기존 authored 문서는 catalog에서만 숨긴 것이 아니라 `elements=[]` evidence shell로
물리적으로 비웠다. 삭제 전 byte/canonical hash, 행 수, exact 이동 수는
`Valtan.legacy-v0-carrier-migration-inventory.v1.json`과
`Valtan.carrier-v1-materialization-receipt.v1.json`이 봉인한다. Git history가 원문 복구 경계다.

예외는 세 가지다.

- `effect.valtan.pattern.420633.active`: 검증된 Whirlwind 9행을 보존하고 exact WModel 2행은
  `modelPreScale=0.01`로 다시 봉인했다.
- `effect.valtan.red-blade-wave.active`: combat-object owner를 exact 5행으로 교체했다.
- `effect.valtan.sky-axe.active`: BossCatalog 외부 owner가 소비하는 3행을
  `BLOCKED_EXTERNAL_OWNER` 해시 예외로 남겼다. exact 완료 수치에는 포함하지 않는다.

## 3. exact carrier 재구성

검토된 24개 패턴, 45개 ordered clip occurrence에서 660개 exact projection을 얻었다.

| carrier | exact | common-translucent Product |
|---|---:|---:|
| Sprite | 455 | 454 |
| Mesh | 173 | 171 |
| Decal | 32 | 32 |
| 합계 | 660 | 657 |

Product에는 44개 exact clip group을 만들었다. 43개는 pattern cue owner이고 1개는 Red Blade
combat-object owner다. 보호된 Whirlwind exact alias 3행은 새 문서로 복제하지 않았다. 최종 발탄
catalog는 46개, boss-root cue는 44개, 실행 Product 행은 669개다.

각 새 행은 다음을 보존한다.

- occurrence full key, carrier key, emitter source order, renderer shape
- exact source material path와 source profile identity
- texture/resource lane과 portable source recipe
- clip-local start delay와 root attachment/follow 정책
- Mesh 한정 exact WModel 1개와 `modelPreScale=0.01`

Sprite와 Decal에는 `meshModel`과 `modelPreScale`이 없다. system 안의 mesh를 다른 emitter에
복사하는 fallback도 없다. EF TypeDataDecal closure와 Base DDS가 모두 닫힌 32행만 실제
`kind=decal`로 승격했다.

## 4. 미해석 family 처리

family가 아직 닫히지 않은 657개 drawable 행은 공통
`effect.standard + alpha_two_sided_depth_read` RT0로 실행한다. 이때
`sourceMaterialPath`, source profile, resources, source recipe는 지우지 않고 보존하며
`sourceProfile.enabled=false`로 두어 이후 family executor 교체 지점을 유지했다.

공통 translucent가 carrier를 위조할 수는 없다. Light, ScreenPost, generic Dust,
missing-resource 및 unresolved runtime-adapter 행은 Sprite quad나 임의 Mesh로 바꾸지 않고 blocker
ledger에 남겼다.

| 미승격 증거 | 수치 |
|---|---:|
| reviewed projection ledger 전체 | 1,577 |
| drawable로 승격하지 않은 projection | 917 |
| source carrier가 없는 reviewed occurrence | 197 |

source branch가 검토되지 않은 7개 패턴과 action binding이 없는 2개 encounter 패턴도 기존 V0는
제거하되 새 행을 추측 삽입하지 않았다. 따라서 이번 결과의 정확한 완료 범위는 "검토된 24개
패턴의 exact carrier 기준선"이다.

## 5. 3연격 실제 결과

`VALTAN_FRONT_BACK_FRONT / SMASHES / valtan.attack.front-back-front.active.clip.01`은 이제 하나의
clip owner와 하나의 cue로 실행된다.

```text
effect  effect.valtan.carrier-v1.attack.front-back-front.active.clip-01
rows    108
shape   Sprite 72 / Mesh 28 / Decal 8
render  effect.standard / alpha_two_sided_depth_read
scale   모든 Mesh 0.01
```

과거의 “60행 전부 MeshParticle” 문서를 재사용한 것이 아니다. 3연격 clip에 실제로 join되는
여러 source occurrence의 emitter를 전량 펼친 결과가 108행이며, 각 행은 원본 carrier 종류를
유지한다. 따라서 행 수가 작아 보이도록 삭제하지 않았고, 반대로 다른 clip의 aggregate를 끌어오지도
않았다.

이전 WATERTRAIL 두 canary는 Product owner가 아니며 authored 문서도 `elements=[]`로 비웠다.
historical element/document hash와 source evidence는 imported ledger만 보존한다. Four Slash
WATERTRAIL source는 새 Carrier V1 exact 행과 교차검증되며, FBF WATERTRAIL source는 아직
`UNRESOLVED_RUNTIME_ADAPTER`라 Product에 승격하지 않았다.

## 6. cue 이력과 회귀 방지

pattern-occurrence v2의 기존 baseline cue identity 99개는 폐기하지 않고 successor chain으로
보존했다.

```text
baseline identity                  99
Carrier V1 retirement             105
exact successor mapping            48
successor 없이 명시 retire          57
현재 cue                            44
  Carrier V1 clip cue               43
  보호 Whirlwind                     1
```

Carrier receipt와 현재 cue 문서의 canonical hash를 migration receipt가 교차 봉인한다. 임의 cue,
cue metadata, retirement mapping 또는 receipt hash가 바뀌면 검증이 실패한다.

SafeReviewedGaps, ReviewedSourceFamilies, WATERTRAIL은 새 Product를 다시 덮어쓰는 writer가 아니라
historical witness + Carrier V1 successor 검증기로 전환했다.

## 7. 자동 검증

다음 검증을 통과했다.

- source occurrence inventory `--check`: exact denominator 660
- legacy V0 migration inventory `--check`: 97문서 / 3,032행 historical preimage
- Carrier V1 materializer `--mode check`: `APPLIED`, `changed=0`, 660/657
- pattern occurrence migration `--check`: 130 bindings / 137 clips / 44 cues
- SafeReviewedGaps / ReviewedSourceFamilies / WATERTRAIL successor `--check`
- 전체 Effect pipeline Python contract: 189 tests PASS / optional `jsonschema` 2 tests SKIP
- Effect project registration `-Check`: files 1,919 / filters 212
- `Publish-Effects.ps1 -Mode Publish`: catalog 145개 / components 0개 PASS
- Server x64 Debug build 및 `Server.exe --contract-test`: PASS / failures 0
- Client x64 Debug build: PASS
- `git diff --check`

긴 Carrier V1 effect ID 때문에 authored publish 임시 파일 경로가 Windows `MAX_PATH`를 넘던
문제도, authored 폴더 안의 짧은 transaction-local 임시 이름을 쓰도록 교정했다. 실제 145개
catalog publish로 동일 실패가 재발하지 않음을 확인했다. Carrier receipt는 전체 EffectCatalog가
아니라 `effect.valtan.` 46행 slice만 봉인하므로, 병렬 캐릭터 복원이 catalog 행을 추가해도 발탄
계약은 불필요하게 stale되지 않는다. Client 실행과 visual PASS는 사용자 전용 경계다.

## 8. 사용자 수동 검증 경로

빌드 후 사용자는 All Effects에서 다음 경로를 연다.

```text
Valtan
  -> FRONT_BACK_FRONT
  -> SMASHES
  -> valtan.attack.front-back-front.active.clip.01
  -> effect.valtan.carrier-v1.attack.front-back-front.active.clip-01
```

확인할 항목은 하나의 cue만 재생되는지, 100배 mesh가 없는지, Sprite/Mesh/Decal carrier가 서로
뒤섞이지 않는지, animation 전체 timeline에서 세 타격과 바닥 decal의 timing을 손튜닝하고 저장한
뒤 재로드할 수 있는지다. 사용자의 서면 관찰 전에는 visual PASS로 기록하지 않는다.
