# 도화가 S Typed Rect V1 확장 실험 결과

branch: `codex/v1-first-pixel-experiment`

base: `origin/main@45d2e7befbed434cb9679aeccbf6d4783bbe5c0a`

plan: `2026-08-22_FOUR_CHARACTER_VALTAN_EFFECT_V1_FULL_MIGRATION_MASTER_PLAN.md`

## 1. 결론

첫 V1 확장 기준점은 도화가로 고정했다. 도화가 A의 기존 SpriteWave 검격 한 행은 새 코드가
필요 없는 control이며, 실제 구현 canary는 도화가 S의 손튜닝 Sprite Rect 두 행이다.

```text
control                         도화가 A SpriteWave 검격 1행
implementation canary           sprite.artist.31420.grass-coverage.v1
data-only expansion proof       sprite.artist.31420.grass-tip-emissive.v1
```

S의 두 행은 하나의 class-neutral typed RT0 program을 공유한다. body 뒤 tip을 연결할 때 C++와
HLSL 추가가 0이고 descriptor의 coverage/dissolve texture 역할만 교환된다. 따라서 이번 변경은
스킬별 셰이더 복제가 아니라 `Sprite Rect carrier + typed packet + reusable RT0 family`의 첫 확장
증명이다.

자동 구현과 build는 완료했지만 화면 결과는 사용자가 아직 판정하지 않았다. 현재 상태는
`IMPLEMENTED_AUTOMATED / USER_REVIEW_PENDING`이며 `V1_COMPLETE`가 아니다.

## 2. V1 완료선과 도화가 F 근거

이번 실험은 다음 완료선을 사용한다.

```text
V1_COMPLETE
  = 올바른 의미의 carrier
  + typed RT0 Base 계산식
  + texture 역할/channel/color-space/sampler
  + color/alpha/UV/time
  + render state/composition
  + 사용자 PNG A/B 및 손튜닝 승인

NATIVE_PARITY
  = exact cooked child/permutation/native VF/pass/MRT의 선택적 후속 연구
```

도화가 F가 잘 나온 근거는 모든 원본 픽셀이 bit-exact native UE3로 실행됐기 때문이 아니다.
의미 있는 element를 올바른 Mesh/Sprite/Decal/Ribbon carrier에 놓고, 각 element에 맞는 typed RT0
계산식과 texture 역할, 색·알파·UV·시간값을 공급한 뒤 사용자가 PNG A/B와 손튜닝으로 composition을
닫았기 때문이다. 이것이 현재 합의한 V1 golden이다.

따라서 도화가 F의 성공은 재현 가능한 구조적 근거가 있지만, 자동 증거만으로 원본 화면과 동일하다고
선언할 수는 없다. 마지막 visual fidelity는 이번 S canary도 동일하게 사용자가 판정한다.

## 3. 보존한 Product 경계

도화가 S `31420`의 현재 Product 3행을 유지했다.

- 기존 source particle 1행은 변경하지 않았다.
- 손튜닝 body/tip의 stable ID, transform, size, color, UV, spawn/lifetime, attachment를 유지했다.
- source 36행이나 과거 invisible element를 bulk restore하지 않았다.
- body/tip의 `material.execution` packet만 추가했다.

두 typed 행의 texture ABI는 다음과 같다.

| register | role | body | tip | channel |
|---:|---|---|---|---|
| t0 | base radiance | `fx_a_line_003.dds` | 동일 | RGBA |
| t1 | coverage | `fx_o_grass_04.dds` | `fx_o_grass_03.dds` | R |
| t2 | emissive radiance | `fx_d_fluid_007.dds` | 동일 | RGB |
| t3 | dissolve | `fx_o_grass_03.dds` | `fx_o_grass_04.dds` | R |

네 lane은 linear/wrap sampler를 사용한다. render state는 기존 alpha two-sided depth-read와 pass 1을
유지한다.

## 4. 구현

### 4.1 typed RT0 family

RuntimeMaterialV2 opcode 21을 추가했다.

```text
base.rgba * colorMultiply + colorOffset
coverage  = t1.r
dissolve  = step(lifetime dissolve amount, t3.r)
alpha     = base.a * coverage * dissolve
radiance  = base.rgb + t2.rgb * emissiveIntensity
RT0       = float4(radiance, alpha)
RT1       = 0
```

invalid carrier, template, lane role/channel/register, sampler, hidden CB input 또는 distortion 상태는 stage와
shader 양쪽에서 fail-close한다. opcode는 character/effect/stable ID를 참조하지 않으므로 같은 ABI를
만족하는 다른 Sprite Rect occurrence가 데이터만으로 재사용할 수 있다.

### 4.2 Rect runtime 배선

- Rect shader가 RuntimeMaterialV2 opcode 21을 dispatch한다.
- Rect도 runtime DDS와 sampler를 bind하고 draw 뒤 sampler state를 복구한다.
- Rect packet validator가 요구하는 `g_SourceTextureMask`를 typed Rect 경로에서 별도로 bind한다.
- 알 수 없는 typed Rect opcode는 generic translucent로 보이지 않고 fail-close한다.

리뷰 중 최초 구현에서 Rect source texture mask가 공급되지 않는 P0를 발견했다. 이 상태라면 정상
packet도 shader에서 clip될 수 있었다. mask bind를 추가하고 Debug build를 다시 통과시켰다.

### 4.3 deterministic materializer와 publish

도화가 S materializer가 같은 stable 두 행을 idempotent하게 갱신하며 기존 legacy 행에서 typed packet만
추가한다. Effect publisher는 새 authored SHA-256을 catalog에 기록하고 sealed runtime 문서를 생성했다.
catalog와 sealed 문서는 같은 commit에 포함한다.

## 5. 자동 검증

### PASS

```text
python Tools/EffectPipeline/test_apply_artist_31420_grass_tip_fade.py
  Ran 8 tests, OK

python Tools/EffectPipeline/apply_artist_31420_grass_tip_fade.py --check
  PASS

Publish-Effects.ps1 -Mode Validate
  PASS before rebase, 205 entries

Publish-Effects.ps1 -Mode Publish
  PASS after latest-main rebase conflict regeneration, 207 effects

sealed runtime identity
  opcode 21 rows = 2
  body/tip lane mapping = expected

Shader_VtxEffectRectPreview standalone FXC
  PASS

Client x64 Debug
  PASS after Rect source-mask fix

Client x64 Release
  PASS

git diff --check
  PASS
```

기존 C4819/LNK4099와 FXC X3577/X4000 경고는 남지만 compile/link 실패는 없다.

## 6. 사용자 수동 검증

사용자는 최신 main의 Client를 직접 실행해 Effect Tool에서 다음 순서로 확인한다.

1. `effect.artist.skill.31420.unified`를 연다.
2. `sprite.artist.31420.grass-coverage.v1`만 Solo한다.
3. 사각 카드가 아니라 grass coverage로 경계가 잘리는지 확인한다.
4. lifetime 후반에 grass dissolve와 alpha가 함께 사라지는지 확인한다.
5. `sprite.artist.31420.grass-tip-emissive.v1`만 Solo한다.
6. 풀끝 emissive가 별도 point light 없이 bloom 성분으로 보이고 body와 함께 소멸하는지 확인한다.
7. 도화가 S 전체 composition을 확인한다.
8. 대조군으로 도화가 A의 `authored.source-particle.cb346af47371feedccf9b652`를 Solo해 기존
   SpriteWave 검격이 계속 보이는지 확인한다.

기록할 상태는 다음 네 가지다.

```text
body first pixel / rectangular card     PENDING
body coverage + dissolve                PENDING
tip emissive + shared fade              PENDING
Artist A SpriteWave control             PENDING
```

## 7. 다음 확장 판정

사용자 승인 시 같은 방식으로 cohort를 넓힌다.

1. 창술사 D: 기존 missiletrail stable 한 행을 no-code Solo한 뒤 Product join을 복구한다.
2. 창술사 F: MakeFlow 한 행의 parent 5-lane과 DynamicParameter를 복원한다.
3. 워로드 F: 현재 56행을 보존하고 WPO SinWave 전기 Mesh 두 행을 typed family로 올린다.
4. 차원술사 F: 현재 Fluid01 packet의 실제 first pixel을 닫고 의미 있는 missing element만 선택 복구한다.
5. 도화가 A/S: 사용자 승인값만 손튜닝하고 source 전량을 되살리지 않는다.

S가 보이지 않거나 의미 없는 결과면 전체 확장을 강행하지 않는다. first-pixel telemetry와 화면 관찰로
carrier, lane, alpha, lifetime 중 실패 층을 좁혀 같은 canary에서 교정하거나 이 V1 확장안을 폐기한다.
