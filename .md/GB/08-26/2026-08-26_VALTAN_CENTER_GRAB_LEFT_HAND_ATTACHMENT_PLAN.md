# 발탄 catch-breath 잡기·유지·발사 시퀀스 계획

## 현재 실제 반영 상태와 이번 작업 경계

`VALTAN_SEQUENCE_CATCH_BREATH`에는 이미 다음 네 애니메이션 스테이지가 있다.

```text
STEP_01  mesh_att_battle_21_01  2000 ms
STEP_02  mesh_att_battle_21_02   500 ms
STEP_03  mesh_att_battle_21_03  4000 ms LOOP_TO_STAGE_END
STEP_04  mesh_att_battle_21_04  2000 ms
```

그러나 현재 gameplay promotion은 이 패턴을 `targetPolicy: NONE`, `aimPolicy: NONE`으로
생성하고, 네 스테이지에는 플레이어 잡기나 발사 Server 로직이 없다. 기존 dirty worktree에는
다른 제품 패턴인 `VALTAN_CENTER_GRAB_COUNTER_64`의 대상 ID를 snapshot으로 복제하고 Client에서
왼손 본을 따라가게 하는 표현 코드만 반영돼 있다.

이번 작업은 기존 target snapshot 계약을 재사용하되 부착 대상을
`VALTAN_SEQUENCE_CATCH_BREATH`의 원본 notify 시간축으로 옮기고, 다음 Server 권위 시퀀스를 완성한다.

```text
STEP_01 ENTER: 살아 있는 플레이어 한 명을 Server가 잠금
STEP_01 1400ms HIT notify:
             -> 기존 행동/이동/투사체 취소
             -> GRABBED 행동 상태와 발탄 owner/occurrence 기록
             -> Client는 같은 NetEntityId를 왼손 본에 부착
STEP_02:      GRABBED 유지, 애니메이션의 뒤돌기 동안 왼손 추적 유지
STEP_03:      정확히 4000 ms 동안 GRABBED 유지, loop clip 반복
STEP_04 650ms HIT notify: 왼손 부착 해제
             -> 발탄의 뒤돌기 이후 정면 방향으로 남은 1350 ms 강제 발사
             -> 일반 navigation/collision clamp를 통과하지 않고 arena 밖까지 이동
             -> 발사 종료 시 Server FALLING 전이, 기존 45 tick 낙사 사망 사용
패턴 중단:   STEP_04 전 중단이면 잡힌 플레이어를 안전하게 해제
```

## 수정 파일과 역할

| 구분 | 파일 | 역할 |
|---|---|---|
| 수정 | `Data/Valtan/Valtan.animation-chain-promotions.json` | catch-breath의 Server target/aim 정책 저작 |
| 수정 | `Tools/ValtanPipeline/promote_valtan_animation_chains.py` | promotion별 optional target/aim을 검증하고 gameplay 정본에 투영 |
| 생성 갱신 | `Data/Valtan/Valtan.gameplay.json` 및 Valtan Product 문서 | publisher가 target/aim과 기존 4단계 시간을 일치시킴 |
| 수정 | `Shared/Public/Network/PacketMessages.h` | 기존 wire ordinal 뒤에 `GRABBED` 행동 상태 추가 |
| 수정 | `Shared/Private/Network/PacketMessages.cpp` | GRABBED snapshot은 skill 없이 non-zero start tick을 요구 |
| 수정 | `Server/Public/ServerPlayer.h` | 잡은 boss occurrence와 강제 발사 벡터·속도·남은 시간을 소유 |
| 수정 | `Server/Public/GameRoom.h` | catch-breath stage transition, pose 유지, 발사/fall helper 계약 |
| 수정 | `Server/Private/GameRoom.cpp` | stage edge에서 잡기/발사 commit, 취소 rollback, 발사와 낙사 진행 |
| 수정 | `Client/Private/Character.cpp` | GRABBED snapshot을 idle body pose로 소비 |
| 수정 | `Client/Public/Character.h` | 부착 표현이 중복 저장 없이 authoritative network action을 읽는 read-only 경계 |
| 수정 | `Client/Private/ClientReplication.cpp` | GRABBED인 STEP_01~04 Server target을 왼손 본에 부착하고 shot notify에서 해제 |
| 수정 | `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` | GRABBED wire round trip과 malformed shape 거부 |
| 수정 | `Server/Private/ServerGameplayContractTests.cpp` | target lock, 1400ms grab, 4초 유지, 650ms shot, FALLING 전이 검증 |

새 C++ 파일은 만들지 않으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다.

## Server 자료구조와 불변식

`SERVER_PLAYER`에 저장하는 owner는 Client pointer나 prototype tag가 아니라 Server가 발급한
`NET_ENTITY_ID`와 해당 boss의 `iPatternSequence`다. 둘이 모두 현재 catch-breath occurrence와
일치할 때만 GRABBED를 유지한다.

강제 발사는 단위 XZ 방향, m/s 속도, 남은 초를 소유한다. 일반 knockback은 navigation과
collision에 막히는 것이 정상 계약이므로 재사용하지 않는다. catch-breath 발사만 별도 진행하고,
STEP_04 shot notify 뒤 남은 1.35초가 끝나면 기존 `FALLING -> DEAD -> revive` 수명 주기로 합류한다.

유지해야 할 불변식은 다음과 같다.

1. GRABBED는 살아 있는 플레이어, 유효한 boss entity, non-zero pattern sequence에만 존재한다.
2. GRABBED 동안 `iCurrentSkillId`는 invalid이고 이동 경로, pending input, trigger move,
   일반 knockback, projectile는 모두 비어 있다.
3. STEP_04의 650ms shot notify는 같은 boss occurrence가 잡은 플레이어 한 명만 발사한다.
4. STEP_04 전 패턴 중단·audition reset·revive는 owner와 발사 상태를 전부 지운다.
5. Client는 대상을 다시 고르지 않고 boss snapshot의 `iPatternTargetNetEntityId`만 소비한다.
6. Client bone/행렬 실패는 Server gameplay를 되돌리지 않고 해당 표현만 격리한다.

## 함수 호출 흐름

```text
CValtanBrain::BeginPattern
-> LOCK_NEAREST_ON_START로 가장 가까운 살아 있는 iPatternTargetEntityId 고정
-> GameRoom::Apply_BossPatternStageTransition
-> Prepare_ValtanCatchBreathPlayerTransition
-> 기존 boss stage action/world destruction preflight
-> Commit_ValtanCatchBreathPlayerTransition
-> STEP_01 1400ms notify~STEP_04 650ms notify: Maintain_ValtanCatchBreathGrabPose
-> STEP_04 650ms notify: Arm_ValtanCatchBreathThrow
-> Update_Players
-> Advance_ValtanCatchBreathThrow (1.35초, clamp 없음)
-> Begin_PlayerFall
-> 기존 Update_PlayerFall이 45 tick 뒤 DEAD

Server WORLD_ENTITY_SNAPSHOT target ID
-> ClientReplication::Apply_WorldSnapshot
-> Character snapshot pose 적용
-> Update_ValtanGrabAttachment
-> PlayerWorld = GripOffset * LeftHandWorld
```

## 검증

1. promotion tool `--check`와 Valtan pipeline publisher validation
2. strict JSON parse와 catch-breath target/aim/4단계 exact contract
3. Shared + NetworkProtocolHarness x64 Debug build, failures 0
4. Server x64 Debug build와 `Server.exe --contract-test`
5. Client x64 Debug build
6. `git diff --check`
7. 사용자가 `Server + Client`를 실행하고 로비 → 발탄 → F1 → 패턴 목록에서
   `[P2 Animation] catch-breath` 한 줄을 눌러 1.4초 잡기 notify, 0.5초 뒤돌기,
   4초 유지, 발사 clip 0.65초 shot notify와 낙사를 직접 확인
