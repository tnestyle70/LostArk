# 발탄 catch-breath 잡기·유지·발사 시퀀스 결과

## 완료 상태

`VALTAN_SEQUENCE_CATCH_BREATH`를 다음 하나의 Server 권위 패턴으로 연결했다.

1. `mesh_att_battle_21_01` / 2000ms: 가장 가까운 살아 있는 플레이어 한 명을 잠그고 1400ms HIT notify에서 `GRABBED`로 전환
2. `mesh_att_battle_21_02` / 500ms: 같은 플레이어를 왼손에 유지
3. `mesh_att_battle_21_03` / 4000ms: 같은 플레이어를 왼손에 유지하며 clip 반복
4. `mesh_att_battle_21_04` / 2000ms: 650ms HIT notify에서 왼손을 해제하고 뒤돈 방향으로 남은 1350ms 동안 45m/s 강제 발사한 뒤 `FALLING`/사망 처리

## 구현 내용

- promotion manifest가 catch-breath에 `LOCK_NEAREST_ON_START`와
  `LOCK_FACING_ON_START`를 저작하고 gameplay/encounter 산출물에 투영한다.
- `WORLD_ENTITY_SNAPSHOT::iPatternTargetNetEntityId`가 Server가 잠근 대상을 Client에 복제한다.
- `PLAYER_ACTION_STATE::GRABBED`는 이동·스킬을 차단하고 late join에도 잡힌 상태를 보존한다.
- Client는 별도 action 복사본을 만들지 않고 `CCharacter`가 소비한 authoritative network
  action을 읽어 `GRABBED`일 때만 왼손 부착을 적용한다.
- STEP_01 진입은 대상만 잠그며, 원본 1400ms grab notify가 진행 중인 이동, 스킬,
  projectile, trigger motion, knockback을 전부 취소한다.
- STEP_01 grab notify부터 STEP_04 shot notify까지 Server proxy는 boss root에 고정되고 Client는 정확한
  `bip001-l-hand` bone world matrix를 매 frame 적용한다.
- Client local offset은 `PlayerWorld * inverse(LeftHandWorld)`로 구하며 translation을
  hand-local origin으로 정규화한다.
- STEP_04의 원본 650ms shot notify에 attachment를 해제하고, 뒤돌기 clip의 skeletal turn을 반영해
  boss root facing의 반대 방향으로 남은 1.35초간 약 60.75m 이동한다.
- 강제 발사는 navigation과 arena collision clamp를 의도적으로 우회하며 1.35초 뒤 기존
  Server `FALLING -> DEAD -> REVIVE` 수명 주기로 합류한다.
- 잡기 중에는 boss/monster/combat object damage를 거부하지만 solo encounter의
  engageable player 수는 유지한다.
- reset 또는 중간 실패가 STEP_04 전에 일어나면 occurrence owner만 안전하게 해제한다.

## 자동 검증

- animation-chain promotion unit/tree test 24개: PASS
- promotion `Validate`: PASS (`20 patterns`, `94 stages`, `94 occurrences`)
- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
- `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`: PASS
- Shared + NetworkProtocolHarness x64 Debug 빌드: PASS
- NetworkProtocolHarness: PASS, failures 0
  - GRABBED snapshot round trip
  - skill을 실은 GRABBED와 start tick 없는 GRABBED 거부
- Server x64 Debug build/link: PASS
- catch-breath Server executable contract 7개: PASS
  - STEP_01 대상 잠금과 1400ms 전 미부착
  - 정확한 1400ms GRABBED interrupt
  - STEP_01~03 동일 occurrence/target 유지
  - STEP_04 650ms 전 부착 유지
  - 정확한 650ms 해제와 뒤돈 방향 launch
  - 남은 1.35초 launch 후 FALLING 전환
- Client x64 Debug build/link: PASS
- `git diff --check`: PASS

전체 Server contract test에는 이번 작업과 무관한 기존 Bern world placement/path 검사 2건이
실패했다. catch-breath 전용 계약과 Valtan 패턴 계약은 모두 통과했다.

## 사용자 화면 확인

에이전트는 Client를 실행하거나 시각 결과를 대신 판정하지 않았다. Visual Studio에서
`Server + Client`를 `Ctrl+F5`로 실행하고 로비에서 발탄으로 입장한다. F1 패턴 툴에서
`[P2 Animation] catch-breath` 행을 선택해 실행한다.

확인 기준은 다음과 같다.

1. `21_01` 시작에는 대상만 잠기고, 1.4초 HIT notify에서 가장 가까운 살아 있는 플레이어가 왼손 안으로 들어간다.
2. `21_02` 뒤돌기와 `21_03` 4초 반복 동안 같은 플레이어가 손을 따라간다.
3. 이 구간에는 이동과 스킬 입력이 적용되지 않는다.
4. `21_04` 시작 후 0.65초 shot notify까지 손에 유지되다가 뒤돈 방향으로 날아간다.
5. 아레나 밖에서 낙사해 DEAD가 된 뒤 기존 부활 입력으로 정상 복귀한다.
6. 손가락 안의 세부 위치가 어긋나면 화면 기준으로 hand-local translation만 미세 조정한다.
