# 발탄 World 에테르 구슬 구현 결과

## G00. 적용한 실행 흐름

기존 `effect.valtan.ether.orb`를 Area World Effect로 배치했다. 벽 위 대기 → 해당 벽의 실제
Server 파괴 commit → 900ms 낙하 → 착지 → 살아 있는 플레이어 collider 접촉 → 구슬 소멸과
로나운의 기운 획득을 연결했다. 공중에서는 획득할 수 없고 동시 접촉에도 한 명에게만 지급한다.

버프는 시간제 일반 무적이 아니라 6방향 후 전멸 타격을 1회 막는 보호다. 방어 성공 시 버프를
소모하고 기존 파란 `무적` 문구를 표시한다. 일반 피해와 강제 encounter 실패는 막지 않는다.
사망·reset에는 버프를 지우며 외곽 109 지형 파괴는 미획득 구슬 전체를 제거한다. 일반 돌진은
연결된 벽의 구슬만 낙하시킨다. reset은 원래 벽 위 상태를 복구한다.

## G01. 정본·World 편집과 초기 위치

정본은 `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapeffects.json`이다.
기존 바닥 효과를 보존하고 `world.valtan.ether.01`~`06` 여섯 행을 추가했다. World 목록의
`에테르 구슬` 아래 1~6에서 연결 벽, 대기 위치, 착지 위치, 낙하 시간, 획득 반경을 수정한다.
대기·낙하·착지 Preview는 기존 World renderer를 사용하고 저장된 대기 위치를 변경하지 않는다.

정확한 진입은 `F1 → Effect Tool V1 → All Effects → World → Valtan → 에테르 구슬 → 1~6`다.
`Wall position`, `Landing position`, `Fall duration (ms)`, `Pickup radius (m)`, `Wall ID`로
편집하고 `Preview position`, `Play fall`, `Reset preview`, `Stop preview`로 확인한다.
마지막 검토에서 기존 Valtan 목록 외에 World 목록에도 같은 편집기 진입을 연결했다.

초기 벽은 외곽에서 약 60도씩 분산된 기존 placement 1, 6, 10, 12, 17, 30을 선택했다.
대기 높이는 실제 벽 collider 상단 + 0.4m다. 착지점은 해당 벽을 제거한 뒤 아레나 중앙에서
걸어갈 수 있는 nav cell 중 다른 collider와 겹치지 않는 가까운 지점이며 바닥 + 0.4m다.
낭떠러지와 남은 벽을 피하므로 수직 낙하만 하지 않고 안쪽으로 4.07~8.37m 이동한다.
최종 배치는 사용자가 화면에서 조절한다. 아래 좌표는 최초 게시 위치다.

| 구슬 | 벽 placement | 대기 X/Y/Z | 착지 X/Y/Z |
|---|---|---|---|
| 1 | 1090000000000001 | 172.133 / 28.108 / -122.502 | 164.75 / 23.391 / -122.75 |
| 2 | 1090000000000006 | 164.464 / 28.108 / -108.335 | 160.25 / 23.429 / -114.75 |
| 3 | 1090000000000010 | 151.474 / 28.108 / -106.609 | 147.75 / 23.330 / -108.25 |
| 4 | 1090000000000012 | 139.927 / 28.108 / -121.618 | 147.75 / 23.417 / -120.75 |
| 5 | 1090000000000017 | 147.596 / 28.108 / -135.785 | 151.25 / 23.410 / -128.25 |
| 6 | 1090000000000030 | 163.699 / 28.108 / -136.227 | 159.25 / 23.445 / -130.25 |

상세 위치·접근성 근거는 `out/ValtanWorldEther20260928/placement-receipt.json`에 기록했다.
획득 반경은 0.75m, 최초 낙하 시간은 27 Server tick이다.

## G02. 게시·복제와 실패 경계

Map publisher의 Effects scope가 Client mapeffects와 Server
`DataFiles/World/VALTAN_ARENA.worldpickupsbootstrap`을 같은 transaction으로 게시한다.
잘못된 벽 ID, 중복 ID, finite·범위 위반, 접근 불가능한 바닥은 거부하고 이전 결과를 보존한다.
저장 직전 원본 hash 재확인과 백업·CAS를 유지한다. 새 C++ 파일과 별도 renderer는 없다.

Shared protocol 119로 WorldPickups와 Ronaun guard/grantTick을 복제한다. Client는 snapshot이
없는 동안 추측 생성하지 않으며 COLLECTED/REMOVED는 pending과 active 효과를 모두 정리한다.
획득 문구는 grantTick으로 중복을 막는다. Server 시작 시 새 bootstrap을 읽으므로 Server와
Client를 같은 최신 빌드로 다시 실행해야 한다. 도구의 미저장 draft나 메모리를 자동 Reload하지 않았다.

## G03. 실행한 검증

- 실제 Server `--valtan-arena-support-contract-test`: 49 PASS, failures 0. 기존 지형·바훈투르와
  실제 돌진 파괴, 낙하 중 획득 금지, collider 높이·생존 조건, 동시 접촉 단일 지급, 전멸 1회
  방어, 실제 snapshot 재전송, 외곽 파괴 정리 및 reset을 확인했다.
- Shared protocol 하네스: 1338 PASS, failures 0.
- Client 실제 문서 parser/save native: 여섯 구슬 roundtrip, 잘못된 입력의 기존 문서 보존,
  CAS 저장을 확인했다. 로그는 `out/ValtanEtherPickup20260928/native/run.log`다.
- Map publisher: 4개 테스트 PASS. 실제 Client/Server 쌍 게시, 잘못된 입력 보존, 첫째·둘째
  파일 교체 뒤 실패 주입 rollback, Validate 비수정 및 source v15를 확인했다.
- Effects Publish 후 최종 Check PASS: `out/ValtanWorldEther20260928/map-check-final.log`.
- Engine·Shared·Server·Client Debug Product 빌드 PASS. 기록은
  `out/BuildPipeline/runs/20260928T150526625Z-debug-product.json`이다. World 메뉴 연결과
  도끼 착지 snapshot의 pattern 시작 tick 기록까지 포함한 최종 재빌드다.

Client와 아레나 UI를 자동 실행하지 않았다. 구슬의 실제 크기·시인성·낙하 궤적과 사용자 최종
배치, 폰트 화면은 사용자가 최신 Server/Client에서 확인할 남은 항목이다. 수치·실행 계약 검증을
GPU 화면 성공으로 대신 기록하지 않는다.

## G04. 09-29 SOURCE_LOOP 진입 실패 수정

사용자가 제보한 `level-valtan.initialize / Map Effect source-loop clone/attach rejected:
effect.valtan.ether.orb`를 `Client/Default/EffectFailure.user.log`의 00:14:45.828,
pid 43664 발생과 대조했다. 실제 거절 이유는 owner-sustained sprite/mesh-only admission이다.
구슬은 이미 정상 준비된 nonempty source이며 sprite 5개와 portable Cascade Ribbon 1개를
포함한다. TRAIL kind인 Ribbon을 해당 검사만 제외하고 있었다.

`Effect_Playback.cpp`에서 기존 particle sprite/mesh 또는 검증된 portable Cascade Ribbon을
허용하도록 바로잡았다. SourceRecipe enabled, finite positive duration, 최소 EmitterLoops=0,
ModelCue/OwnerControl 없음과 기존 playback admission 검사는 유지했다. 일반 trail/light
허용 확대, source loop 값 변경, Element 삭제나 LOCAL_LOOP 전환은 하지 않았다.
`MapEffectPresentationRuntime.cpp`는 Stop 전에 실제 service 오류를 복사하여 원래 context,
effectAssetId, stable placementId와 함께 보여 준다. 구슬 source와 게시 데이터는 변경하지 않았다.

실제 설치 자원을 사용하는 창 없는 native 검사에서 기존 Product의 같은 실패를 재현했다.
수정 후에는 실제 `world.valtan.ether.01`~`06` 여섯 배치 모두 current Level LOADING에서
SOURCE_LOOP `Spawn_LevelPlacement → Commit_PendingWorldRootSpawns → Update_WorldRoot`
검사를 통과했다. 각 Stop 뒤 handle이 무효화되고 최종 loading Effect layer가 비는 것까지 확인했다.
로그는 `out/ValtanCinematicEditor20260928/orb-before-run.log`와 `orb-run.log`다.

최종 정상 Debug Product 빌드 PASS: `out/BuildPipeline/runs/20260928T151923322Z-debug-product.json`.
수정한 두 C++를 컴파일하고 `Client/Bin/Debug/Client.exe`를 링크·배포했다. EXE 시간은
2026-09-29 00:19 KST다. 이 최종 빌드에는 진행 중이던 발탄 연출 전체 편집·Solo 보완도 포함된다.
데이터 publish는 실행하지 않았고 Client/UI를 자동으로 실행하지 않았다. 같은 생성 실패 경로의
native 성공과 사용자의 실제 발탄 진입·구슬 화면 확인은 구분한다.

장시간 CPU 검사도 `out/ValtanEtherLoopFix20260929/receipt.json`에서 PASS다. 실제 문서의
준비 duration은 12초이며 10초/60초 모두 sprite 8개가 유지됐다. 원본 rate 0 Ribbon은 초기
0.4초 구간의 짧은 이력 뒤 사라지는 입력을 그대로 보존했다. 데이터 복사본의 rate 20 시험은
별도로 지속 Ribbon과 bounded point 수·finite 좌표를 확인했다. 기존 sprite/mesh 입장과
finite-only·미지원 source·서로 충돌하는 owner/bounded clock 거부도 유지됐다.
입장 회귀 receipt는 `out/ValtanCinematicEditor20260928/orb-admission-receipt.json`이다.
