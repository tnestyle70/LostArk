# 콜로세움 관중 NPC 40마리 배치 결과 (2026-09-30)

## 한 일
- `Data/Worlds/LV_PVP_COLOSSEUM/Gameplay.world.json`(revision 1 → 2)에 `npc.colosseum.spectator.01~40` NPC 40개를 추가했다.
- 위치는 ARENA04 경사 관중석과 동/서 플랫폼 표면 위이며, 8개 방향(45° 구역)에 5마리씩 나눴다.
  게시된 navgrid의 걸을 수 있는 셀에서 2.5m 이상 떨어진 지점만 사용했다.
- 방향은 경기장 중앙을 바라보게 하고 ±8° 흔들림을 줬다.
- 환호 애니메이션은 placement `idleClip`으로 지정했다 (behavior=null, 제자리 반복).
  - HM_MA01(NPC_11748, 12249): `npc_sc_cheer_loop_1~4`
  - HM_MA02(NPC_11752, 11816, 12250, 12911, 12917, 13203): `npc_sc_cheer_loop_1` 또는 `npc_evt2_clap_1`
  - HM_FE03(NPC_11592, 12905): `npc_sc_cheer_loop_1`
  - HM_FE04(NPC_11749, 18394): `npc_sc_cheer_loop_1~2`
- 12종 archetype은 모두 이미 Bern 배포 리소스에 있는 NPC라 추가 리소스가 필요 없다.

## 게시
- `Publish-WorldGameplay.ps1 -Mode Validate/Publish -WorldId COLOSSEUM` 성공 (46 placements = 플레이어 스폰 6 + NPC 40).
- `Server/Bin/DataFiles/World/COLOSSEUM.worldbootstrap`, `Client/Bin/DataFiles/World/COLOSSEUM.npcpresentation.json` 갱신.
  presentation에 spectator 40건, cheer/clap idleClip 반영을 확인했다.

## 확인하지 않은 것
- 빌드와 Client 실행, 화면 확인은 하지 않았다. 관중 위치가 좌석 위에 자연스럽게 앉는지, 애니메이션 재생은 사용자가 확인한다.
- 게시된 파일은 Server 재시작 후 소비된다.
- NPC는 서버 엔티티라 플레이어 크기의 blocking body를 가진다. 관중석이 이동 영역 밖이라 영향은 없다고 판단했다.
