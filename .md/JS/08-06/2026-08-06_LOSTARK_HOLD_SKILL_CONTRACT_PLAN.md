# 홀딩 스킬 계약 — 적룡포(34590)

작성자: JS · 2026-08-06 · 미착수

키를 누르면 `start`, 유지하면 `loop`, 떼거나 loop가 끝나면 `end`가 나오는 홀딩 스킬을
서버 권위로 구현한다. 34590 적룡포가 첫 대상이다.

## 0. 사용자가 확정한 것

| 항목 | 결정 |
|---|---|
| 최대 홀딩 시간 | **LOOP 클립 길이**. 별도 상수를 만들지 않는다 |
| 데미지 시점 | **END 클립 중 1회** |
| 홀딩 길이에 따른 위력 | **고정**. 원작의 success 구간은 이번 범위 밖 |
| 쿨다운 시작 | **시전 시작** (현재와 동일) |

## 1. 왜 계약 변경이 필요한가

`actionDurationMs`가 고정 상수이고 서버가 그 시각에 액션을 끝낸다. 홀딩은 길이가 입력에
따라 가변이라 이 전제가 깨진다.

```text
actionDurationMs = startMs + heldMs + endMs      heldMs ∈ [0, loopMs]
```

적룡포 기준 700 + [0,1000] + 1200 = 1900~2900ms.

## 2. 클라이언트는 새 메커니즘이 아니다

`CCharacter::Update_Chain`은 이미 `isCombo`일 때 자동 전환을 멈추고 서버가 확정한
`comboStage`를 `Advance_ComboStage()`가 반영한다. **홀딩도 같은 모양이다** — 서버가 phase를
확정하고 클라이언트는 해당 클립을 재생한다. `Advance_ComboStage`를 phase 기반으로
일반화하면 되고 새 재생 경로를 만들지 않는다.

LOOP 클립만 `isLoop = true`로 재생한다.

## 3. 변경 목록

| 계층 | 내용 |
|---|---|
| `Shared/PacketType.h` | `PLAYER_SKILL_KIND::HOLD = 2` |
| `Shared/PacketMessages.h` | `C2S_RELEASE_SKILL { iClientSequence, iSkillId }`, snapshot player에 `iActionPhase` |
| `Shared` 직렬화 | 위 두 가지 write/read |
| `Server/GameplayCatalog` | HOLD 파싱, `iHoldStartMs` / `iHoldLoopMs` / `iHoldEndMs` |
| `Server/PlayerSkillSystem` | phase 상태기, release 소비, 가변 duration, END 중 1회 damage |
| `Server/GameRoom` | `C2S_RELEASE_SKILL` → `RoomCommand` |
| `Client/CPlayerController` | 슬롯 키 해제 감지 → `IPlayerCommandSink::Release_Skill` |
| `Client/CCharacter` | phase → 클립, LOOP은 looping |
| `Data/Balance/PlayerSkills.json` | 34590 `skillKind: HOLD` + hold 타이밍, receipt 동기화 |
| publisher | HOLD 행 검증 (3구간 합 = 최대 duration, clip 수 3 강제) |
| `NetworkProtocolHarness` | 새 메시지 왕복, 잘못된 필드 거부 |
| `ProjectAudit` | HOLD 스킬의 바인딩 clip 수 3 검증 |

## 4. 서버 상태기

```text
USE_SKILL 승인
  phase = START, elapsed = 0, cooldown 시작
elapsed >= startMs
  phase = LOOP
phase == LOOP 이고 (release 수신 또는 loop 경과 >= loopMs)
  phase = END, endElapsed = 0
phase == END 이고 endElapsed >= endHitMs 이고 미적용
  damage 1회
endElapsed >= endMs
  action = NONE
```

release가 START 중에 오면 START를 마친 뒤 LOOP를 건너뛰고 END로 간다. 입력을 버리지 않으면서
클립이 튀지 않는다.

## 5. 미해결

- **END의 damage 시각.** `flm_sk_lastwhisper_end`에 HIT notify가 없다. 현재 `hitTimeMs=1595`는
  `EFTable_SkillEffect` 공용 기본값이라 근거가 없다. END 구간 상대 오프셋으로 새로 정해야 하며
  추정값이 된다.
- **다른 홀딩 스킬.** `.clipseq`에서 `mode=HOLD`로 분류된 건 적룡포와 청룡출수 둘이다.
  청룡출수는 현재 바인딩에 없다.
- **34590의 루트 모션.** 현재 곡선은 고정 2900ms 기준으로 합성돼 있다. duration이 가변이 되면
  곡선 시간축도 phase 상대로 바뀌어야 한다.

## 6. 진행 원칙

Shared 프로토콜이 바뀌므로 팀원 전체가 함께 받아야 하고 harness도 같이 간다.
2026-08-06의 두 커밋(`190ca03`, `8765939`)과 섞지 않고 별도 변경 단위로 만든다.
