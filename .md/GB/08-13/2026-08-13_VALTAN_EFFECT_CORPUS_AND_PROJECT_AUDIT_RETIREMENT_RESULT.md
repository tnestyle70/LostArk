# Valtan Effect corpus 전수조사와 ProjectAudit 퇴역 결과

- 작성일: 2026-08-13
- 기준 encounter: `Data/Encounters/Valtan/ValtanEncounter.json`
- 기준 source: `Data/Effects/Imported/Valtan`과 `Resource_LostArk/05_Reports/EffectExtraction/VALTAN/all_actions`
- 사용자 결정: 전역 `ProjectAudit` 실행기와 report 계약은 퇴역한다.

## 1. 최종 결론

`Client/Bin/Resources/Effect/Valtan`에는 **31개 패턴의 완성 EffectAsset**이 전부 있는 것이 아니다.
현재 닫힌 것은 원본 Action에서 명시적으로 추적 가능한 ParticleSystem graph와 그 직접 binary
dependency다.

- ParticleSystem root: 193/193
- material binding: 335, material parent: 123
- runtime mesh: 52/52 WModel
- runtime texture: 346/346 DDS
- 현재 63 source Action이 쓰는 subset: root 130, mesh 40, texture 273
- Product `EffectCatalog`의 Valtan entry: 0
- `Data/Effects/Authored/Valtan`: 없음
- `Data/Animation/Authored/Valtan`: 없음
- encounter semantic stage에서 원본 source stage/occurrence로 가는 exact binding: 0/115

즉 **재료 창고는 상당히 완비됐지만 패턴별 조립도, 분기 선택, 애니메이션 cue와 제품 소비자는
아직 없다.** Action 전체를 그대로 연결하면 난이도·성공/실패·HP 분기의 Effect가 동시에 재생된다.

## 2. source와 renderer family 분모

현재 Server encounter는 31 pattern, 115 semantic stage, 63 unique source Action이다. 63/63 Action
report가 존재하고 62/63 Action에서 explicit ParticleSystem을 찾았다. `420652`만 explicit root가 0이다.
이 63 Action 내부의 원본 source stage는 1,576개다.

193 graph의 top LOD emitter와 현재 Action subset은 다음과 같다.

| family | 전체 corpus | 현재 63 Action subset |
|---|---:|---:|
| SpriteParticle | 1,155 | 823 |
| MeshParticle | 252 | 175 |
| embedded Decal | 36 | 29 |
| Ribbon | 13 | 6 |
| Trail | 8 | 5 |
| Light | 8 | 6 |

graph 내부 typed Decal과 별개로 Action notify가 가리키는 asset identity 미해결 항목이 있다.

| 미해결 notify | 전체 Action source | 현재 63 Action |
|---|---:|---:|
| generic `Effect` | 3,787 | 3,192 |
| `PlayDecalEffect` | 536 | 490 |
| `DefaultParticle` | 133 | 21 |
| 합계 | 4,456 | 3,703 |

따라서 빨간 장판 계열 `PlayDecalEffect`를 embedded Decal 36개로 대신했다고 판정하면 안 된다.

## 3. 31 pattern 전수 표

`roots`는 해당 pattern의 source Action이 참조하는 unique ParticleSystem 후보 수다. `M/S/D/R/T/L`은
top LOD의 Mesh/Sprite/Decal/Ribbon/Trail/Light emitter 수다. 같은 Action과 root가 여러 pattern에서
재사용되므로 행을 합산하지 않는다. 모든 행의 판정은 `SOURCE_ROOT_EXACT`,
`SEMANTIC_STAGE_BINDING_OPEN`, `PRODUCT_UNBOUND`다.

| pattern | source Action | roots | M/S/D/R/T/L | 미해결 Decal/Effect/Default |
|---|---|---:|---:|---:|
| SWING | 420601, 420660 | 7 | 8/41/1/0/1/2 | 70/137/0 |
| DOWN_SMASH | 420602, 420661 | 10 | 4/64/4/0/1/1 | 28/169/0 |
| IMPRISON_ROAR | 420603 | 11 | 2/43/1/1/0/1 | 0/28/0 |
| DASH_CHARGE | 420604 | 4 | 5/7/1/0/0/0 | 16/240/0 |
| EARTHQUAKE_SMASH | 420605, 420662 | 10 | 10/49/1/0/1/2 | 45/83/0 |
| PARRY | 420606, 420607 | 12 | 3/28/0/1/2/2 | 0/36/0 |
| MAGIC_CHOICE | 420608 | 13 | 10/62/3/0/1/2 | 2/27/0 |
| FOUR_SLASH | 420609 | 14 | 11/59/1/1/1/2 | 15/44/0 |
| HIGH_JUMP | 420610 | 8 | 16/51/2/0/0/1 | 18/130/0 |
| STOMP | 420611 | 3 | 0/12/0/0/0/1 | 0/96/0 |
| BIND_CHARGE_SMASH | 420612~420614 | 14 | 13/90/4/1/0/2 | 12/182/0 |
| GROUND_WAVE_SMASH | 420615 | 11 | 5/41/2/0/4/2 | 3/46/0 |
| SUPER_SMASH | 420619, 420620, 420656, 420657 | 19 | 22/62/5/2/1/3 | 83/274/0 |
| JUMP_SPIN | 420621, 420663 | 10 | 12/32/2/0/3/1 | 30/104/0 |
| PORTAL_RUSH | 420622 | 6 | 10/43/3/0/0/1 | 3/79/0 |
| CHARGE_GRAB_ROAR | 420623, 420631, 420632 | 14 | 16/73/2/1/0/0 | 26/161/0 |
| WHIRLWIND | 420633 | 5 | 4/6/0/0/3/1 | 3/36/0 |
| BACKSTEP_ATTACK | 420635, 420664 | 13 | 14/55/1/0/4/1 | 16/80/0 |
| RED_BLADE_WAVE | 420636 | 8 | 6/17/0/0/1/2 | 0/9/0 |
| FRONT_BACK_FRONT | 420637, 420666 | 12 | 11/77/3/0/1/1 | 59/121/8 |
| FIST_IN_OUT | 420638 | 6 | 3/47/0/0/1/1 | 5/15/0 |
| LEDGE_ROAR | 420639 | 6 | 6/21/1/0/0/0 | 3/15/0 |
| TRIPLE_COUNTER | 420640~420647 | 14 | 13/73/2/0/0/2 | 0/155/0 |
| ARMOR_BREAK_OPENING | 420627, 420628, 420654, 420655 | 8 | 12/62/2/0/0/2 | 4/42/0 |
| FLOOR_WIPE_130 | 420630 | 13 | 19/76/4/0/1/2 | 3/26/0 |
| FOUR_PILLARS_105 | 420610 | 8 | 16/51/2/0/0/1 | 18/130/0 |
| ARENA_BREAK_80 | 420629 | 5 | 0/16/2/0/0/1 | 0/15/0 |
| MAGIC_ORB_STAGGER_76 | 420617, 420618 | 9 | 11/55/2/0/0/1 | 0/161/0 |
| CENTER_GRAB_COUNTER_64 | 420623, 420631 | 14 | 16/73/2/1/0/0 | 21/138/0 |
| ARENA_BREAK_33 | 420629 | 5 | 0/16/2/0/0/1 | 0/15/0 |
| GHOST_TRANSITION_15 | 11 Action | 57 | 47/257/7/2/4/4 | 46/681/13 |

## 4. 사용자가 지목한 패턴

### 4.1 점프 후 도끼 투척

구 format-v2의 `JUMP_AXE_THROW`는 Action `420610`이었다. 최신 31-pattern 정본은 같은 Action을
`HIGH_JUMP`와 `FOUR_PILLARS_105`에 재사용한다. `420610`에는 다음 재료가 있다.

- `Att_Battle_8_01_Start/Loop/End`
- ParticleSystem root 8
- MeshParticle emitter 16
- mesh 6, texture 47
- FilmNoise/ZoomBlur 계열 post root
- 미해결 Decal 18, generic Effect 130

그러나 explicit axe projectile object path는 발견되지 않았다. 현재 증거로 “도끼 투척 완비”라고
판정할 수 없으며 source stage/branch와 unresolved Effect identity를 먼저 해소해야 한다.

### 4.2 5방향 공격

현재 정본과 구 문서 어디에도 `5방향` stable pattern ID 또는 확정 Action/stage가 없다. 이는 화면에서
부르는 명칭일 가능성이 높다. 사용자 관찰 장면을 current pattern, Action, source stage로 interview해
확정하기 전에는 `420630` 같은 비슷한 다방향 Action에 추측으로 연결하지 않는다.

### 4.3 4방향 돌 생성

구 format-v2의 `FOUR_DIRECTION_WALLS`는 `420630`이었고, 최신 정본에서는 `FLOOR_WIPE_130`이다.
다음 원본은 실제로 존재한다.

- `Par_S_RPBF_Stone_01_1`, `Par_S_RPBF_Stone_01_2`
- `fm_a_stone_001`, `fm_d_stoneparts_003`
- cylinder/crack/ring 계열 mesh
- source dependency mesh 8, texture 87

하지만 이것은 돌·파편 **MeshParticle**이다. 서 있는 기둥/벽의 collision, 파괴 HP, replication,
despawn을 가진 world actor가 아니다. 현재 `Gameplay.world.json`의 동적 기둥/돌 actor는 0이고
`ValtanWorldEvents.json`의 binding도 0/5 enabled다. 제품 4기둥은 Effect 복원과 함께 Server actor
spawn, collision, replication, destroy/despawn을 별도 수직 슬라이스로 닫아야 한다.

### 4.4 회전 공격

현재 정본은 `WHIRLWIND`, Action `420633`이다.

- clip: `Att_Battle_20_02/03/04`
- root 5
- MeshParticle 4, Sprite 6, Trail 3, Light 1
- `fm_o_swing_02`, `fm_m_trail_002`, texture 19
- PlayParticle 13, Trail event 10
- 미해결 Decal 3, generic Effect 36

31개 중 작은 animation+trail+mesh vertical canary로 적합하지만 아직 Product binding은 없다.

### 4.5 arena 80/33 파괴

Action `420629`의 explicit Effect root는 5지만 mesh dependency는 0이다. 실제 arena 형상 제거는
`Effect/Valtan`이 아니라 Deploy/Map world mutation 영역이다. 화면 충격 Effect, arena mesh mutation,
collision/navigation 갱신, Server state replication을 같은 mechanic slice로 닫아야 한다.

## 5. 바로 전체 Action을 붙이면 실패하는 이유

한 Action 안에 난이도, 성공/실패, HP 조건 branch가 함께 들어 있다.

- 420610: 46 source stage
- 420630: 27 source stage
- 420604: 62 source stage
- 420623: 63 source stage
- 420616/420658: 각각 82 source stage

또한 `420610`은 HIGH_JUMP/FOUR_PILLARS, `420629`는 ARENA_BREAK_80/33, `420623`은
CHARGE_GRAB_ROAR/CENTER_GRAB_COUNTER가 함께 쓴다. Action 단위로 통째로 cue를 만들면 다른 branch의
Particle, Decal, Post가 중복 폭발한다.

필수 sidecar 계약은 다음과 같다.

`patternId + semanticStage -> actionId + sourceStageIndex + branch condition + clip + occurrenceId`

## 6. 권장 구현 순서

1. 사용자 명칭 `5방향`, `4방향`, `도끼 투척`을 current pattern/Action/source stage로 확정한다.
2. 115 semantic stage의 source-stage sidecar를 만든다.
3. `WHIRLWIND`로 animation + Trail + MeshParticle canary를 닫는다.
4. `JUMP_SPIN` StoneWave/Distortion을 닫는다.
5. `ARMOR_BREAK_OPENING` part destruction을 닫는다.
6. `FOUR_PILLARS_105`는 Effect와 Server world actor를 함께 구현한다.
7. `ARENA_BREAK_80/33`은 Effect와 world mutation/navigation을 함께 구현한다.
8. 마지막으로 branch와 actor가 많은 `GHOST_TRANSITION_15`를 진행한다.

Artist family canary의 증거 형식은 재사용할 수 있지만 Artist의 VF/pass 승인이 Valtan material 335개를
자동 승인하지는 않는다. Valtan occurrence별 ShaderMap/DXBC/VF/pass 판결은 별도로 필요하다.

## 7. ProjectAudit 퇴역

ProjectAudit은 runtime이나 Git resource version manager가 아니었다. 39개 파일, 12,820줄, 121개
검사를 한 번에 실행하고 JSON report 하나로 전체 저장소를 막는 정적 검사 aggregate였다.

이번 변경에서 다음을 제거했다.

- `Tools/ProjectAudit` 39개 파일
- `Invoke-BuildAndRegression.ps1`의 aggregate 호출과 report 생성 계약
- tracked stale `Artifacts/MapToolBernChangeLevel.ProjectAudit.json`
- `.codex_tmp` 아래 생성 ProjectAudit report
- 현재 정본 문서의 ProjectAudit 필수 실행 규칙

다음은 유지했다.

- gameplay/world/navigation/effect/rendering publisher의 `Validate`/`Check`
- NetworkProtocolHarness, ClientFrontendHarness, Server contract test
- 실제 runtime parser가 소비하는 JSON `schema`/`formatVersion`
- stable ID와 Resources-relative path
- Artist MIC/ShaderMap/DXBC/texture 원본 증거 receipt와 runtime admission hash

`Data/AssetPacks.lock.json`과 global immutable resource manifest는 제거 전에 이미 존재하지 않았다.
따라서 임의의 runtime JSON을 삭제하지 않았고, 존재하지 않는 pack lock을 완료 조건으로 요구하던
낡은 문구만 제거했다.

## 8. 검증 상태

- Valtan 조사: 31/31 pattern read-only matrix 완료
- PowerShell parser: `Invoke-BuildAndRegression.ps1` PASS
- `Publish-BalanceRuntimeSet.ps1 -Mode Validate`: PASS
  - player profile 6, skill 132, damage profile 108, boss 1, Valtan pattern 31/stage 115
- `Publish-ServerNavigation.ps1 -Mode Validate`: PASS
- `Publish-RenderingProfiles.ps1 -Mode Validate`: PASS
- `Publish-Effects.ps1 -Mode Validate`: PASS, Effect catalog 102 entry
- `Sync-EffectDataProject.ps1 -Check`: 최초 stale 판정 뒤 정본 동기화 실행,
  재검증 PASS (`files=1407`, `filters=183`)
- reconstructed approval policy unit: 41/41 PASS
- reconstructed approval policy `--check`: PASS
- `git diff --check`: PASS
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -SkipBuild`: domain validator까지 진행한 뒤
  no-argument ClientFrontendHarness 단계가 244초 제한 안에 종료되지 않아 중단했다. 전체 regression
  PASS로 기록하지 않으며, 남은 harness process는 종료했다.
- 전체 Debug/Release compile: 이번 조사·퇴역 변경에서는 실행하지 않았다.
- Client 화면 및 visual fidelity: 사용자 검증 전이므로 PASS로 기록하지 않는다.
