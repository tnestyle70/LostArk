# 차원술사 BA1 반속 재생·이펙트 동기화 구현 계획

## 1. 목표

차원술사 기본 공격 `2050010`의 1단계(BA1)를 기존 1.4초에서 0.7초의 Server 권위 action clock으로 줄인다. BA1 unified 이펙트가 시작되는 같은 시점에 캐릭터가 실제 원본 찌르기 애니메이션을 정확히 한 번 재생하고, 캐릭터·이펙트·root motion·다음 입력 창이 하나의 시계를 소비하게 한다.

Client가 콤보 단계를 임의 생성하거나 자동 진행하지 않는다. 기존처럼 마우스 down edge를 Server에 보내고 Server snapshot의 `iComboStage`만 표현한다.

## 2. 조사 결과와 결정

| 항목 | 실측 | 구현 결정 |
|---|---|---|
| 제품 clip | `pc_sp_m_00_sk_att_battle_1_01`, 30 TPS, 원본 4.0초 | clip identity를 바꾸지 않는다. |
| 유효 공격 구간 | 기존 binding `playMs=1400`, source HIT 약 0.1002초 | 원본 1.4초 구간 전체를 보존한다. |
| 기존 BA1 권위 시간 | duration/advance/input close 1400ms, hit/input open 100ms | 전부 같은 비율로 700/50ms로 축소한다. |
| 제품 effect cue | BA1 clip local 0ms의 `effect.dimensionmaster.skill.2050010.ba1.unified` 한 건 | cue를 복제하지 않고 clip과 같은 2배속 action clock을 전달한다. |
| root motion | stage 0, 0~1400ms, 최종 변위 약 0.8418m | 변위는 유지하고 sample 시간만 0~700ms로 절반 압축한다. |

단순히 `playMs`를 700으로 줄이면 원본 자세를 0.7초에서 잘라 찌르기 후반을 잃는다. 따라서 source trim은 `playMs=1400`으로 유지하고 `playRate=2.0`을 선언한다. 제품 `CCharacter`는 같은 rate를 `CModel::Set_AnimationSpeed`와 `EFFECT_SPAWN_DESC::fPlaybackRate`에 전달하므로 캐릭터와 이펙트가 0.7초 wall clock에서 source 1.4초를 함께 소비한다.

## 3. 수직 슬라이스

1. `PlayerSkills.json`의 BA1 action/hit/combo/input 시간을 절반으로 변경한다.
2. `DimensionMaster.skillbindings.json`의 BA1 exact source clip에 `playRate=2.0`을 선언한다.
3. `DimensionMaster.rootmotion.json` stage 0 sample 시간을 절반으로 압축한다.
4. 공식 receipt의 변경 필드만 `PROJECT_TUNED`로 동기화한다.
5. Server contract에서 700ms 경계 이전에는 BA1을 유지하고 경계에서만 다음 stage/종료가 일어나는지 검사한다.
6. Client focused harness에서 실제 WModel, exact clip, weapon-bone pose 변화, 단일 Product cue, animation/effect shared clock, malformed rate atomic rollback을 검사한다.

## 4. 변경 파일

- `Data/Balance/PlayerSkills.json`
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`
- `Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json`
- `Data/Animation/RootMotion/DimensionMaster.rootmotion.json`
- `Server/Private/ServerGameplayContractTests.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`

Valtan authored/candidate/cue와 공용 renderer/shader는 수정하지 않는다.

## 5. 검증 계획

- gameplay balance/runtime-set publisher Validate
- Debug/Release Engine, Shared, Server, ClientFrontendHarness build
- Debug/Release `ClientFrontendHarness.exe --dimension-ba1-sync-fast`
- Debug/Release `Server.exe --contract-test`
- JSON parse, `git diff --check`
- 사용자가 직접 인게임에서 BA1 한 번 입력 시 0.7초 안에 실제 찌르기 1회와 BA1 unified 이펙트가 동시에 보이는지 최종 육안 판정
