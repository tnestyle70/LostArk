# 홀딩 스킬 계약 — 구현 결과

작성자: JS · 2026-08-06 · 브랜치 `feature/player-hold-skill`

계획서는 `2026-08-06_LOSTARK_HOLD_SKILL_CONTRACT_PLAN.md`다. 적룡포(34590)가 첫 대상이며
키를 누르면 start, 유지하면 loop, 떼거나 loop가 끝나면 end가 나온다.

## 1. 계획서 대비 줄어든 것 둘

실제 코드를 보니 새로 만들려던 게 둘 다 불필요했다.

| 계획서 | 실제 |
|---|---|
| snapshot에 `iActionPhase` 신설 | **기존 `iComboStage` 재사용.** 정의가 이미 "서버 소유 1-based 단계 인덱스, 클라이언트가 스스로 세지 않는다"라 홀딩 phase와 같다 |
| `iHoldStartMs`/`iHoldLoopMs`/`iHoldEndMs` 신설 | **기존 `ComboStages` 재사용.** `SKILLSTAGE` 행·파서·damage 경로를 그대로 탄다 |

덕분에 bootstrap 형식과 클라이언트 재생 경로가 바뀌지 않았다. 클라이언트에서 홀딩은
새 메커니즘이 아니라 COMBO와 같은 "서버가 단계를 확정하고 따라간다" 구조다.

`isCombo`는 `isServerStaged`로 개명했다. COMBO와 HOLD 둘 다 해당하므로 이름이 조건을 설명한다.

## 2. 변경 목록

| 계층 | 내용 |
|---|---|
| `Shared/PacketType.h` | `PLAYER_SKILL_KIND::HOLD`, `PACKET_TYPE::C2S_RELEASE_SKILL` |
| `Shared/PacketMessages.h/.cpp` | `C2S_RELEASE_SKILL { iClientSequence, iSkillId }` + write/read |
| `Server/ServerApp.cpp` | 패킷 → `ROOM_COMMAND_TYPE::RELEASE_SKILL` |
| `Server/GameRoom` | `Handle_ReleaseSkill` |
| `Server/PlayerSkillSystem` | `Release()`, phase 상태기, HOLD damage 게이트 |
| `Server/ServerPlayer.h` | `hasReleasedHold` |
| `Server/GameplayCatalog` | `ParseSkillKind`의 HOLD, `SKILLSTAGE` 소유자 허용 |
| `Client/PlayerCommandSink` 계열 | `Request_ReleaseSkill` → `Send_ReleaseSkill` |
| `Client/PlayerController` | 홀딩 키 유지 추적과 해제 엣지 |
| `Client/PlayerSkillCatalog` | HOLD 파싱과 제약(쿨다운 필수, 스테이지 3) |
| `Client/Character` | `isServerStaged`, `CLIP_STEP::loop`, LOOP 클립 looping |
| `Data/Balance/PlayerSkills.json` | 34590 HOLD, 2900ms, 스테이지 3 |
| receipt | 스테이지 field 12개 추가, `fieldEntryCount` 1420 → 1432 |
| `Publish-GameplayBalance.ps1` | HOLD 검증 |

`C2S_RELEASE_SKILL`에 aim을 넣지 않았다. 방향은 시전 시점에 확정되고 release는 입력 엣지만
알린다. 서버가 언제 액션을 끝낼지는 여전히 서버가 정한다.

## 3. 상태기

```text
USE_SKILL 승인      phase 1 (START), 쿨다운 시작
elapsed >= startMs  phase 2 (LOOP)
release 또는 loop 만료  phase 3 (END)
START 중 release    START을 마친 뒤 LOOP를 건너뛰고 phase 3      (holdSkipsLoop)
END의 hitTimeMs     데미지 1회
END 만료            action NONE
```

## 4. 걸린 것 — 새 skillKind 하나에 검증 지점이 다섯

각 계층이 같은 JSON을 독립적으로 검증하므로 한 곳만 고치면 다음 관문에서 막힌다. 전부
하네스가 잡았고 조용히 넘어간 것은 없다.

1. `Publish-GameplayBalance.ps1` — `Unknown skillKind`
2. `CPlayerSkillCatalog` — 클라이언트 읽기 거부
3. `GameplayCatalog::ParseSkillKind` — 서버 bootstrap 거부
4. `GameplayCatalog`의 `SKILLSTAGE` 행 — 소유자를 COMBO로 강제
5. provenance receipt — coverage count 불일치

**가장 중요한 버그는 damage 게이트였다.** stage 1·2의 `hitTimeMs`가 0이라
`elapsed >= 0`이 첫 tick에 참이 되어 데미지가 즉시 적용됐고, 그게 COMBO용
`cancelsIntoNextStage`("히트가 나갔으면 남은 클립을 끊고 다음 단계로")를 깨워서
START→LOOP→END가 몇 tick 만에 지나갔다. 한 번 누르면 바로 찌르기가 나오고 길게 눌러도
유지되지 않았다. HOLD는 damage를 stage 3에서만 적용하고 `cancelsIntoNextStage`를 쓰지 않는다.

## 5. 검증

- `Invoke-BuildAndRegression.ps1 -Configuration Debug`: 전 프로젝트 빌드 성공,
  protocol harness / ClientFrontendHarness / `Server.exe --contract-test` 모두 `failures : 0`,
  balance Validate/Publish 성공.
- `ProjectAudit`: `projects.data-source-visibility` 실패(기존, SkillWindow 2개 미등록).
- 인게임(사용자): 길게 누르면 LOOP가 유지된다. 짧게 누르면 찌르기로 넘어간다.

## 6. 남은 것

- `NetworkProtocolHarness`에 `C2S_RELEASE_SKILL` 왕복과 잘못된 필드 거부 추가.
- `ProjectAudit`에 HOLD 바인딩 clip 수 3 검증 추가.
- **END의 damage 시각 `hitTimeMs=600`은 추정값이다.** `flm_sk_lastwhisper_end`에 HIT notify가
  없고 기존 1595는 `EFTable_SkillEffect` 공용 기본값이었다.
- 34590의 루트 모션 곡선은 고정 2900ms 기준으로 합성돼 있다. duration이 가변이 됐으므로
  phase 상대 시간축으로 다시 봐야 한다.
- `.clipseq`에서 `mode=HOLD`로 잡힌 다른 스킬은 청룡출수이며 현재 바인딩에 없다.
- 홀딩 길이에 따른 위력 변화(원작의 success 구간)는 범위 밖으로 두었다.
