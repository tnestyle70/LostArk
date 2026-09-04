# 2026-09-04 발탄 잡기 Server anchor 단일 경로와 마력구 오라 STAGE_END RESULT

브랜치 `GB/KoukuSaydon-Main-Pattern`. 대응 계획은
`2026-09-04_VALTAN_GRAB_SERVER_ANCHOR_AND_STAGGER_AURA_STAGE_END_IMPLEMENTATION_PLAN.md`가 소유한다.
이 문서는 현재 working tree의 실제 G01/G02 구현과 전달받은 실행 증거만 기록한다.

현재 상태는 **G01/G02 구현 반영, targeted 자동 검증 PASS, post-change C++ build와 사용자 화면 판정 대기**다.
피자 rotation 복구 전 실행된 Product exit 0은 이 변경의 최종 build 증거로 사용하지 않는다.

## 1. 범위와 완료 상태

| 범위 | 구현 상태 | 검증 상태 |
|---|---|---|
| G01 Client hand-bone transform overwrite 제거 | 구현 반영 | grip 5/5, 인접 combat-object 19/19 PASS |
| G01 validation-only `gripLocalOffset` 계약 유지 | 구현 반영 | CAPTURE 7행 authoring/projection 검증 PASS |
| G01 삭제 harness와 project 등록 정리 | 구현 반영 | post-change C++ harness build/run PENDING |
| G02 magicball + aura 동일 CHANNEL binding | 구현 반영 | 전용 4/4, Effect V2 5-suite 64/64 PASS |
| G02 core/aura `STAGE_END`, aura 12초 | 구현 반영 | V2 validator, binding validate, G09 alignment PASS |
| Product/Core/FullDiagnostic | 미실행 | PENDING |
| Server+Client 화면 판정 | 미실행 | 사용자 판정 PENDING |

## 2. G01 실제 구현

### 2.1 transform writer 단일화

- `CClientReplication`의 `Stage_PlayerAttachmentPresentation`,
  `Update_PlayerAttachmentPresentations`, `m_PlayerAttachments`와 `bip001-l-hand` lookup을 제거했다.
- `CValtan`의 action/pattern별 grip cache, lookup과 joined reload 단계를 제거했다.
- `CPlayerHandGripTransform`은 `Build_LocalOffset`/`Compose_World` 행렬 합성을 잃고
  `PLAYER_HAND_GRIP_LOCAL_OFFSET` typed shape와 ±10m component validation만 유지한다.
- 잡힌 Character의 position/yaw는 Server가 capture 순간 저장한 boss-local attachment snapshot을 매 fixed tick
  world pose로 갱신해 `PLAYER_SNAPSHOT`에 싣고, Client `CCharacter::Update_NetworkTransform`이 일반 network
  sample 보간 경로로 적용한다. Client가 그 뒤 model hand bone으로 transform을 다시 덮어쓰지 않는다.
- 이 계약은 같은 Server-authoritative snapshot stream을 쓴다는 뜻이다. Client interpolation이 있으므로
  Server 최신 tick과 화면 frame의 bit-exact 좌표 일치를 주장하지 않는다.

Server `GameRoom`의 capture/update/release 동작과 Shared packet field/schema는 바꾸지 않았다. 다만
`PacketMessages.h`, `ServerPlayer.h`, `Character.cpp`의 계약 주석과 Composition Detail 안내는 더 이상
Client hand-bone 합성이 존재한다고 설명하지 않도록 교정했다.

### 2.2 `gripLocalOffset` 잔존 경계

`Data/Valtan/Valtan.gameplay.json`의 CAPTURE hit은 현재 7행이고 모두
`{ forwardM: 0.0, upM: -0.9, rightM: 0.0 }`을 유지한다. Product 투영, encounter parser,
publisher, Balance Tool과 Composition Detail의 typed shape/range validation도 유지한다.

이 값은 현재 Server 또는 Client의 runtime transform에 적용되지 않는다. Composition Detail도 이를
runtime 손 위치 조절값이 아니라 schema validation용 authoring metadata라고 표시한다.

### 2.3 harness와 regression 정리

- 행렬 합성만 검사하던 `PlayerHandGripTransformContractTests.cpp`를 삭제하고
  `ValtanPatternAuditionServiceHarness.vcxproj/.filters`, main 선언·호출·return gate에서 제거했다.
- `test_valtan_grip_local_offset_contract.py`는 삭제된 Client lookup 대신 다음을 고정한다.
  - CAPTURE 7행의 저작·Product 투영 일치.
  - ClientReplication/Valtan에 hand-bone overwrite와 grip cache 심볼이 남지 않음.
  - Server `Update_PlayerAttachment`가 boss-local snapshot을 계속 갱신함.
- `test_valtan_combat_object_hit_effect_presentation_contract.py`는 삭제된 attachment 함수를 앞 함수의
  source delimiter로 쓰지 않고 `Apply_WorldSnapshot`을 사용한다.

## 3. G02 실제 구현

### 3.1 CHANNEL binding

`VALTAN_STAGGER_SLOT / CHANNEL / valtan.authoring.stagger-slot.channel`에는 다음 두 GROUP binding이 있다.

| resource | clock | anchor | stop |
|---|---|---|---|
| `boss.valtan.magicball` | STAGE 0ms, ONCE | `b_effectroot`, FOLLOW_SLOT, TARGET_YAW | `STAGE_END` |
| `boss.valtan.magicball.aura` | STAGE 0ms, ONCE | `b_effectroot`, FOLLOW_SLOT, TARGET_YAW | `STAGE_END` |

기존 magicball binding은 `NATURAL`에서 `STAGE_END`로 바뀌었고 aura binding을 하나 추가했다. CHANNEL에서
누적 damage 1000을 채워 `VALTAN_GROGGY_FOLLOWUP`으로 조기 분기하거나, 12000ms TIMEOUT으로
`FINAL_ATTACK`에 들어가 action/stage가 바뀌면 V2 stage lane이 두 group의 Mesh object를 즉시 끝낸다.
Effect V2 runtime 코드는 바꾸지 않았다.

### 3.2 aura lifetime과 역할

- `boss.valtan.egg.aura_1`, `boss.valtan.egg.aura_2`는 모두 non-loop Mesh이며 lifetime을 10초에서
  CHANNEL과 같은 12초로 늘렸다. 따라서 damage를 넣지 않는 TIMEOUT 경로의 마지막 2초가 비지 않는다.
- 기존 authored alpha/dissolve envelope는 유지한다. 12초 자연 종료와 TIMEOUT stage 전환은 같은 경계이고,
  조기 GROGGY 전환의 `STAGE_END`는 별도 dissolve tail 없이 즉시 제거한다.
- `boss.valtan.magicball.aura`는 `EffectRoles.json`에서 `STATE / NONE`이다. 오라는 상태 표현이며
  damage collider나 Server hit을 만들지 않는다.
- 전용 regression은 두 binding의 exact scope/clock/anchor/stop policy와 aura 두 leaf의 12초 non-loop를 고정한다.

## 4. 자동 검증

### 4.1 확인된 targeted 증거

| 검증 | 결과 |
|---|---|
| `Tools.ValtanPipeline.test_valtan_grip_local_offset_contract` | 5/5 PASS |
| `Tools.ValtanPipeline.test_valtan_combat_object_hit_effect_presentation_contract` | 19/19 PASS |
| `Tools.EffectToolV2.test_valtan_magicball_black_core_contract` | 4/4 PASS |
| Effect V2 관련 5-suite | 64/64 PASS |
| `validate_effect_v2.py` | PASS: 131 authored / 170 bindings / 16 groups / 13 independent / 84 textures |
| `effect_v2_binding_pipeline.py ... validate` | PASS: `validatedBindings=101` |
| G09 hit/effect alignment | PASS: 18 roles / 44 ATTACK / 101 bindings |
| Composition Validate | PASS |

### 4.2 아직 필요한 build·광역 회귀

| 검증 | 상태 |
|---|---|
| post-change Debug Product | PENDING |
| post-change Debug Core | PENDING |
| `ValtanPatternAuditionServiceHarness` compile/run | PENDING; 삭제된 source/project registration의 link closure 확인 필요 |
| NetworkProtocol + Server attachment contract | PENDING; wire/schema와 기존 Server authority 회귀 확인 필요 |
| Debug FullDiagnostic | PENDING |
| 최종 대상 파일 `git diff --check` | PENDING |

targeted Python/V2 PASS는 위 C++ build와 광역 회귀를 대신하지 않는다. 피자 rotation 복구 전의 Product exit 0도
현재 최종 working tree의 증거가 아니므로 RESULT에 PASS로 올리지 않았다.

## 5. 사용자 수동 검증

에이전트는 Client/UI를 실행하거나 visual PASS를 대신 판정하지 않았다. post-change Debug build가 닫힌 뒤
사용자가 `Server + Client` profile로 다음을 확인한다.

1. `VALTAN_TRASH`와 `VALTAN_CATCH_BREATH`에서 GRABBED 동안 Character body, nameplate와 Debug 표현이
   서로 갈라지지 않고 같은 Server snapshot 움직임을 따르는지 확인한다.
2. boss 회전 중 player가 Client hand animation을 따라 별도 궤도로 흔들리지 않는지, release 전후에
   stale attachment가 남지 않는지 확인한다.
3. `VALTAN_STAGGER_SLOT` CHANNEL 시작에 magicball core와 body aura가 같은 root에 함께 보이는지 확인한다.
4. damage 없이 진행했을 때 aura가 10초에 끊기지 않고 12초 TIMEOUT 경계까지 이어진 뒤
   `FINAL_ATTACK` 진입에 사라지는지 확인한다.
5. 다른 회차에서 response 1000을 조기에 채워 GROGGY로 전환한 순간 core와 aura가 함께 사라지고
   aura 자체로 damage가 발생하지 않는지 확인한다.

## 6. 남은 경계

- 현재 Server attachment는 capture 순간의 boss-root-relative pose를 유지한다. 실제 animated 손바닥을
  Server authority로 따라가게 하는 기능은 `serverAttachmentLocalOffset`/replicated socket pose를 별도
  수직 슬라이스로 설계해야 하며 Client bone overwrite를 되살리지 않는다.
- `gripLocalOffset` authoring metadata 자체를 퇴역하는 작업은 이번 범위가 아니다.
- 조기 `STAGE_END`에 dissolve-out을 추가하는 작업은 leaf/runtime stop 계약을 함께 바꾸는 별도 범위다.
- 구현은 공유 dirty working tree에 반영됐으며 이 RESULT 작성 과정에서 stage/commit/push하지 않았다.
