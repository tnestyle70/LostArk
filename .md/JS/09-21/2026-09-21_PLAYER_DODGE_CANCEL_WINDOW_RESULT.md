# 2026-09-21 플레이어 회피기(Space) 캔슬 창 RESULT

브랜치 `feature/player-dodge-cancel-window`, base `main db01564c`.

## 원작 확인

- `EFTable_Skill.ActionType`: 0 일반 스킬, 1 평타, 2 회피기, 3 기상기. 4직업 회피기는
  창술사 34020 탄영·34520 돌파(스탠스별), 도화가 31020, 워로드 17020, 차원술사 2050020.
- InputTiming notify 라벨은 스킬 캔슬과 별도로 회피 캔슬을 갖는다. 라벨은
  `회피캔슬`, `[회피캔슬]`, `회피기캔슬`, `회피기 캔슬`, `회피기캔슬(테스트)`,
  `회피기 및 절룡세 캔슬`(창술사), `이동기캔슬`, `이동기 캔슬`(도화가). 4직업 행 수는
  창술사 94, 워로드 60, 차원술사 52, 도화가 42.
- 회피 창은 스킬 캔슬 창보다 먼저 열리고 그 창을 덮는다. 도화가 `sdm_att_battle_1_01`은
  이동기캔슬 500ms, 스킬캔슬 900ms, 이동캔슬 1000ms. 창술사 `flm_sk_dragonupfly_02_re`는
  회피 500ms 창만 있고 스킬 캔슬 창이 없다.
- `win=DODGE_CANCEL` 분류는 기획자 한글 라벨을 접은 것이며 원본 notify의 enum 필드가 아니다.
  6직업에서 일관되므로 근거로 채택했다.
- 빈 payload는 회피에 포함하지 않는다(사용자 결정). 같은 클립에서 빈 행과 회피 행이 다른
  시각에 열린다(`flm_sk_chestdestruction_01`: 빈 551ms, 회피 574ms).

## 변경

| 파일 | 내용 |
|---|---|
| `Tools/CharacterAnimationIntake/build_cancelwindows.py` | `DODGE_CANCEL_PAYLOADS` 추가, 출력에 `dodgeCancel`, formatVersion 2 |
| `Data/Animation/CancelWindows/{LanceMaster,Artist,DimensionMaster,Warlord}.cancelwindows.json` | 재생성. 회피 창 보유 스킬 창술사 17/24, 도화가 14/16, 차원술사 12/14, 워로드 16/16. 창술사 2개는 회피 창만 있음 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | formatVersion 2, `dodgeCancel` 필수 속성, `SKILLCANCEL`/`SKILLSTAGECANCEL` kind `DODGE` |
| `Server/Public/GameplayCatalog.h` | `DodgeCancelWindows`(skill·stage), `PLAYER_CANCEL_INPUT { SKILL, MOVE, DODGE }`, `Is_InsideCancelWindow`의 bool을 enum으로 교체 |
| `Server/Private/GameplayCatalog.cpp` | kind `DODGE` 행 파싱 |
| `Server/Private/GameRoom_PlayerCommands.cpp` | 이동 캔슬 호출을 `PLAYER_CANCEL_INPUT::MOVE`로 |
| `Server/Private/PlayerSkillSystem.cpp` | 요청 스킬이 `SPACE` ACTIVE면 SKILL 창 ∪ DODGE 창, 그 외는 SKILL 창만 |
| `Server/Private/ServerGameplayContractTests_PlayerActions.cpp` | 34040 800ms 시점: 34090 거부·34020 승인 계약 |

회피기 판별은 서버 `PLAYER_SKILL_DEFINITION::strInputSlot == "SPACE"`와 `ACTIVE`다. 기상기도 SPACE지만
KNOCKDOWN 가드가 먼저 걸러낸다. bootstrap 행 형식·protocol·Client는 바꾸지 않았다.

## 검증

- `git diff --check` 통과.
- 4직업 cancelwindows 재생성 완료.
- `Publish-GameplayBalance.ps1 -Mode Publish` PASS. `Gameplay.bootstrap`에 `DODGE` 캔슬 행 69개
  (예: `SKILLCANCEL 34040 DODGE 1 698:1500`).
- publisher 구문 파싱 오류 0, `build_cancelwindows.py --check` 재실행 unchanged.
- Debug Product 빌드 PASS(Server OBJ 70개 재컴파일, Engine/Shared/Client 변경 없음).
- `Server.exe --contract-test`: 새 계약 2건 PASS
  (`Load the authored dodge cancel windows`, `Admit the Space dodge inside the dodge window`).
  기존 스킬 캔슬 3건도 PASS. 전체 실패 23건은 쿠크 관문 보스·발탄 dash/arena·Bern navigation·
  DimensionMaster T ground-target·Result collider 항목이며 캔슬 창 코드가 닿지 않는 경로다.
  Result collider 계약은 34040을 NONE에서 한 번만 시작해 damage/stagger 수치를 비교하므로
  캔슬 창을 읽지 않는다. 다만 main 기준 baseline 실행은 하지 않았다.
- 실제 아레나 Space 캔슬 체감: 사용자 확인.

## 남은 것

- 회피기 쿨 3000ms는 전체 스킬 테스트용 임시값이며 나중에 일괄 복원한다(사용자).
- 건슬링어·슬레이어는 4직업 범위 밖이라 회피 창을 넣지 않았다.
- 창술사 `회피기 및 절룡세 캔슬`의 절룡세 예외는 회피기만 허용했다.
