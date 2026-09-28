# 2026-09-29 가디언 나이트 벽 타기(WallClimb) AnimSet 추가 RESULT

## 배경

- 발탄의 G 상호작용 벽 타기는 `<Asset>.interactionbindings.json`의 `wallClimb` 블록과
  `Character/<Class>/AnimSets/<Class>_WallClimbAnimSet.wmodel`(`wall_climb_loop`, `wall_climb_end`)을 쓴다.
- 가디언 나이트는 09-21 통합 때 Esther/Customizing/Ride 6종/MazeHammer/TerrainJump만 쿠킹했고 WallClimb이 없었다.
  바인딩 파일에도 `wallClimb` 블록이 없어 다른 클래스처럼 기존 locomotion loop로 대체되고 있었다.

## 구현

- `Tools/ActorXAssetCooker/build_wall_climb_player_animations.py`: `SOURCES`에 `"GuardianKnight": "PC_DL_00"` 추가,
  카드미로 빌더와 같은 `--classes` 필터 추가(기본은 전체).
- 쿠킹: `--source/--warlord-source C:\Users\95jus\Downloads\umodel_win32\_export_ddk_psk --classes GuardianKnight`.
  원본 `PC_DL_00/AnimSet/pc_dl_00_ani.psa`(sha256 e419ba42…)의 `act_creep_up_1`(41f) → `wall_climb_loop`,
  `act_creep_up_end_1`(61f) → `wall_climb_end`, 233 PSA bones + carrier 1. 바디 skeleton 섹션 보존 확인.
  출력 `Client/Bin/Resources/Character/GuardianKnight/AnimSets/GuardianKnight_WallClimbAnimSet.wmodel`
  (sha256 8e6cdf8b…, receipt `out/GuardianKnightWallClimb20260929/receipt.json`).
- `Data/Actors/CharacterCatalog.json` `PLAYER_GUARDIANKNIGHT.animationSetModels`에 위 wmodel 등록.
- `Data/Animation/Authored/GuardianKnight/GuardianKnight.interactionbindings.json`에 다른 클래스와 같은
  `wallClimb`(loop/end, `endStartSeconds` 4) 블록 추가.

## 검증

- 두 JSON parse 정상. wmodel 클립 `wall_climb_loop` 40틱 / `wall_climb_end` 60틱(다른 클래스와 동일).
- Server 변경 없음. 로컬 Server+Client 재시작. 발탄 절벽 G 상호작용 화면 확인은 사용자 몫.

## 전달

- `GuardianKnight_WallClimbAnimSet.wmodel`은 Git 비추적 Resources라 Drive로 별도 전달해야 한다.
