# 2026-08-18 발탄 페이즈·패턴·제품 이펙트 연결 결과

branch: `feature/effect-cooked-shader-recovery-and-transform-fix`

발탄을 기존 네 캐릭터와 같은 Product Effect 경로에 연결했다. Effect Tool은 발탄 전체를
`페이즈 -> 패턴 -> stage/animation -> Effect` 4단 트리로 표시하고, 제품 런타임은 Server가
승인한 exact pattern stage에서 99개 발탄 문서를 재생한다. 115줄 패턴의 첫 타격은 실제
6방향 판정으로 바뀌었고, 6방향/전멸 예고와 충격에는 바로 손으로 튜닝할 수 있는 가이드를
추가했다.

화면의 크기, 위치, 색, 원본 충실도는 사용자가 직접 판정할 항목이며 자동 PASS로 기록하지
않았다.

---

## G00. 최종 수치

```text
발탄 표시 트리             9 phases / 32 patterns / 121 stages
Effect가 있는 stage        99
트리의 Effect 문서 행      100
  생성 문서                99
  420633 canary              1 (별도 보존)
의도적으로 조용한 stage    22 (Create Effect 행)

생성 element             3,106
  원본 emitter 기반       3,098
  115줄 저작 가이드           8
mesh element             1,940
slot 밖 source ref          217 (unboundResources에 이름 보존)

source EffectCatalog       302 rows / Valtan 99
Product runtime catalog    196 rows / Valtan 99
typed direct source index  PlayerSkill 101 + BossPattern 99
```

기존 `effect.valtan.pattern.420633.active`는 덮어쓰지 않았다. `Valtan.patterneffects.json`의
source-evidence canary로 계속 남고, 99개 Product cue 집합에서는 제외했다. `WHIRLWIND/SPIN`
stage에서는 canary와 naming-rule 생성 문서가 둘 다 보이므로 Effect 문서 행이 100개다.

---

## G01. 구현 완료

### G01.1 All Effects와 저작 초기값

- `CValtanPatternTree`가 encounter 체력 gate를 9개 표시 phase로 파생한다.
- 한 stage에 binding 문서와 naming-rule 문서를 동시에 연결할 수 있다.
- All Effects의 Valtan은 phase, pattern, stage/clip, Effect 순서로 표시한다.
- stage 행은 duration과 Server hit shape 수치를 표시하고 `Open/Play`가 발탄 모델과 해당 clip을
  함께 선택한다.
- 22개 source emitter 없는 stage는 거짓 문서를 만들지 않고 `Create Effect` 행으로 남겼다.
- 시더를 ParticleSystem 단위에서 emitter 단위로 바꿨고 문서 예산으로 element를 버리지 않는다.
- 표준 material은 `base/noise/mask/emissive/dissolve/base2/mask2/noise2` 8슬롯을 실제 shader
  합성까지 사용한다. 초과 217건은 `unboundResources`에 보존되어 이후 수동 재매핑할 수 있다.
- source mesh만 있고 base texture가 없는 element는 model material을 사용한다.

다른 세션의 미커밋 family 변경에서
`EFFECT_SOURCE_RUNTIME_SHADER_PROFILE_IDS`의 선언 크기가 실제 22개 항목과 달리 `21u`였던
선행 빌드 오류는 그 한 글자만 `22u`로 교정했다.

### G01.2 Product catalog와 authoritative playback

- 신규 `Valtan.patterneffectcues.json` 99행을 strict parser로 읽는다.
- schema/version/owner, stable ID, 중복, encounter pattern/stage/action, duration, transform,
  follow/stop policy를 `parse -> validate -> stage -> commit` 순서로 검사한다.
- source catalog의 99개 Valtan 행을 `DIRECT_AUTHORED_DOCUMENT_V13`으로 publish했다.
- direct source index를 `PLAYER_SKILL`과 `BOSS_PATTERN` typed owner로 확장했다. 가짜 class/skill
  owner는 만들지 않았다.
- Valtan Arena 로딩은 현재 선택 class와 발탄 99개를 priority prewarm하고 준비 상태를 probe한다.
- `CValtan`은 Server snapshot의 pattern/action/stage/actionStartTick을 승인한 뒤 cue를 한 번만
  spawn하고, 늦게 받은 snapshot은 action age로 seek한다.
- 새 authoritative edge나 abort는 이전 `actionStartTick`의 pending/active Effect만 먼저
  취소한다. 같은 프레임 A -> B backlog에서도 A가 뒤늦게 commit되지 않는다.
- animation binding 실패는 generic clip으로 폴백하고 Effect cue를 함께 버리지 않는다.
- death, replicated despawn, level clear는 boss owner의 잔여 Effect를 정리한다.

Character Select의 Server Arena는 사용자가 Valtan spawn을 선택할 때만 99개를 on-demand
priority prewarm한다. 해당 99개가 준비된 뒤 prototype 준비와 Server request를 정확히 한 번
자동 실행한다. 일반 몬스터/루가루의 즉시 요청과 기존 player Effect queue는 그대로 보존한다.
prewarm 30초 timeout, 전송 실패, Server 응답 timeout은 각각 재시도 가능한 상태다.

### G01.3 115줄 6방향과 전멸 표시

| stage | 표시 이름 | Effect ID | Server/저작 형상 |
|---|---|---|---|
| `WINDUP` | 발탄 / 115줄 / 6방향 공격 예고 | `effect.valtan.floor-wipe-130.windup` | 0/60/120도 strip 3개 |
| `FIRST_SMASH` | 발탄 / 115줄 / 6방향 공격 충격 | `effect.valtan.floor-wipe-130.first-smash` | `SIX_DIRECTIONS`, half-length 14, half-width 2.2 |
| `INTERVAL` | 발탄 / 115줄 / 전멸 공격 예고 | `effect.valtan.floor-wipe-130.interval` | 반경 100 ring |
| `SECOND_SMASH` | 발탄 / 115줄 / 전멸 공격 충격 | `effect.valtan.floor-wipe-130.second-smash` | `CIRCLE`, 반경 100 |

`SIX_DIRECTIONS`는 boss yaw 기준 0/60/120도의 centered rectangle 세 개 합집합이다. 각 축이
양쪽으로 뻗어 여섯 팔을 만들며 30도 사이 gap은 맞지 않는다. Shared primitive, Server
catalog/brain/collision, publisher, Balance Tool, Effect Tool 요약을 같은 계약으로 갱신했다.

`SECOND_SMASH`는 기존 100000% 일반 damage를 유지한다. 이번 변경에서 절대 사망 `LETHAL`
primitive나 counter/defense 정책을 새로 만들지는 않았다.

---

## G02. 자동 검증

```text
Sync-TeamLanEndpoint.ps1
  server-host / 127.0.0.1:7777 / Server + Client profile

python -m unittest Tools.EffectPipeline.test_build_valtan_stage_effects -v
  4 tests / OK
  121 = 99 cues + 22 silent, generator projection, canary 제외,
  6방향/전멸 guide와 Server geometry 일치

Valtan authored/cue/catalog JSON parse
  103 files / PASS

Publish-Effects.ps1 -Mode Validate
  196 runtime Effect entries / Valtan 99 / PASS
Publish-Effects.ps1 -Mode Publish
  196 Effect entries published / PASS

Publish-GameplayBalance.ps1 -Mode Validate, Publish
  6 profiles / 132 skills / 108 damage / 1 boss
  32 patterns / 121 stages / PASS

Shared x64 Debug, Release
  PASS
Server x64 Debug, Release
  PASS
Server.exe --contract-test (Debug, Release)
  failures: 0 / PASS
  six arms hit, six gaps miss, queued floor-wipe damage 포함

Client x64 Debug, Release
  PASS / Client.exe link 완료

ClientFrontendHarness --character-select-valtan-prewarm-fast
  14 PASS / failures: 0
ClientFrontendHarness --valtan-pattern-effects-fast
  11 PASS / failures: 0
ClientFrontendHarness --effect-incremental-prewarm-fast
  8 PASS / failures: 0

Client/Harness vcxproj + filters XML parse
  4 files / PASS
git diff --check
  exit 0
```

Client 빌드에는 기존 C4819와 LNK4099 warning만 있었고 error는 없었다.

---

## G03. 사용자 수동 확인 경로

자동으로 Client나 UI를 실행하지 않았다. 현재 endpoint 정본 판정은 `server-host`이므로
Visual Studio의 `Server + Client` profile을 사용자가 `Ctrl+F5`로 실행한다.

### Effect Tool에서 손 튜닝

```text
F1 > Effect Tool > All Effects > Valtan
  > PHASE 2  bar 158-115
  > VALTAN_FLOOR_WIPE_130
  > WINDUP / FIRST_SMASH / INTERVAL / SECOND_SMASH
  > Open 또는 Play
```

각 문서에서 element의 Transform scale/position, color, timing을 조절한다. 기존 네 캐릭터는
All Effects의 Artist, Lance Master, Dimension Master, Warlord 선택을 그대로 사용한다.

### 제품 연결 확인

```text
Lobby > Valtan
  115줄 패턴에서 6방향 예고/충격 -> 전멸 예고/충격 순서 확인

Lobby > Character Select > Server Arena > Valtan > Spawn Selected
  99개 준비 상태가 끝난 뒤 Server spawn 요청이 한 번 나가고 pattern Effect가 보이는지 확인
```

확인할 시각 항목은 발탄 기준 root 위치, 0/60/120도 축 방향, 반경 100 크기, stage 전환 시
이전 예고 잔상 유무, 원본 emitter들의 크기/색/밀도다.

---

## G04. 남은 경계

- visual fidelity와 first-pixel PASS는 사용자 판정 대기다.
- 22개 stage는 대응 source emitter가 없어 Product cue를 만들지 않았다. 필요하면
  `Create Effect`로 저작한 뒤 cue/catalog projection에 명시적으로 편입해야 한다.
- 217개 초과 resource reference는 버리지 않고 이름을 보존했지만, 화면 기여를 원하면 8개
  표준 슬롯 중 의도한 의미로 수동 재매핑해야 한다.
- neutral emitter 초기값은 손 튜닝 시작점이지 원본 Cascade 수치의 완전 복원이라고 주장하지
  않는다.
- 전멸기는 현재 높은 일반 damage이며 counter/defense를 무시하는 절대 사망 계약은 아니다.
- 기존 대규모 dirty worktree의 다른 담당 변경을 보존했으며 이 작업에서 stage/commit/push하지
  않았다.
