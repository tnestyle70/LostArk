# 베른 제작 지구·항구 nav 복구 RESULT

2026-10-03 PR 통합 확인: 아래 기록 이후 Debug Product가 2026-10-02 17:40 KST에 PASS했다.
근거는 `out/BuildPipeline/runs/20261002T084006819Z-debug-product.json`과 외부
`Build-Debug-GuideNavMovie-Exit.json`이다. 현재 입력47개는 검토본 SHA와 일치하고 nav8파일은
기존 반영 영수증의 afterSha256과 모두 일치한다. 공식 Area 한정 navigation Validate도 다시 통과했다.
아래의 Debug 링크 대기는 해소됐으며 Release Product는 아직 실행 중이다.
사용자 실제 이동·화면 확인은 완료로 기록하지 않는다.

## G01. 정본 반영

2026-10-02 현재 visible Landscape 실물 기하를 기준으로 Bern navpaint에20,943셀,
Bern3 항구에225셀을 추가했다. 실제 바닥이 있고 경사/높이/장애물 근접/연결성 검사를 통과한
누락 셀만 복구했다. 기존 walkable 높이의 float bit와 기존 paint4,455행은 모두 보존했다.

두 authoring paint와 공식 publisher가 외부에서 생성한 Client navgrid2개, Server navgrid2개와
navsurface2개를 원자적으로 교체했다. Client에는 publisher 계약대로 navsurface를 추가하지 않았다.
원본1255개 입력 hash를 교체 직전에 재확인했다. Bern2/root/BernSea·policy·navregions·blockers,
NPC/이동 목적지·trigger·rendering option·Resources는 변경하지 않았다.

소스/게시의 실제8파일과 SHA, 원본 백업은 저장소 밖
`Desktop/LostArkTransfer/Sync-20261002/Recovery-20261002-1315/BernNav-20261002/Applied-Receipt.json` 및
같은 폴더의 `BeforeApply`에 있다. 이전 후보와 ZIP도 보존한다.

## G02. 원인과 검증

기존 navsource/paint의 공식 재게시 bytes는 원래 설치된 grid와 같았다. Git/LFS/전송 누락이 아니라
실제 바닥을 포함하지 않은 nav coverage 문제였다. squarehole3의 도착은33셀에 고립돼 있었고,
세 squarehole 도착 자체가 walkable이어도 서로의 경로는 모두 실패했다.

| 검사 | 기존 설치본 | 복구 후보 및 반영 bytes |
|---|---|---|
| 이동/spawn 도착12개 exact walkable | 12개 | 12개 보존 |
| squarehole1/2/3 양방향6경로 | 전부 실패 | 전부 성공 |
| 제작 지구 남쪽 인접 목적지 | 1m 앞의 다른 셀로 보정 | 요청 XZ 종점까지 성공 |
| NPC 접근 가능 영역 | 44명 중36명 | 43명 |
| 기존 Server Navigation 계약+추가 회귀 | 51 PASS/7 FAIL | 58 PASS/0 FAIL |
| 변경 Guide 계약 | 원래 nav로111 PASS | 새 nav로111 PASS |

NPC는0.75~3m 주변, 높이±2m의 실제 ground 접근점을 찾은 뒤 CServerNavigation의 경로와
Smooth_Path 요청 종점을 검증했다. 후보의43 NPC 접근점과 squarehole3쌍 등46개 경로가
exact start/goal·도착 XZ를 만족했다. 이 범위의 기존 성공 경로 퇴행은 없었다.
Nav CPP 회귀는 CWorldBootstrap에서 현재 게시 목적지를 읽고 양방향 이동과1m 높이 guard를
검사한다. 기존17m deck 점프 거부·계단·layer·blocker 검사도 유지됐다.

실제 v143 Server source compile와 product object를 사용하는 외부 진단 링크는 exit0이다.
새 nav에서 Navigation58개는13.77초, Guide111개는47.02초, 둘 다 exit0이다.
증거는 `Bern-NavContract-20261002/*-Exit.json`, `BernNav-20261002/Native-Contracts-Ready.json`,
`BernNav-20261002/Native-Accessibility-All43-Exit.json`에 있다.
공식 Area 한정 Publish는 `Bern-Ground-Candidate-02-Staged`에서 실행했다.

## G03. 보존한 경계와 미확인 사항

필터가 생략한 foliage17,645배치는 실물15개 grass/crop/flower 모델과 SHA를 대조했다.
나무 몸통·바위가 없음을 실제 vertex/index와 투영 그림으로 확인했으며 static TREE/CLIFF/WALL은
장애물 검사에 포함했다. generation은 낯선 foliage 모델을 자동 제외하지 않는다.
반경0.25m·바닥 위0.20~1.80m의 triangle footprint 검사는 보수적 proxy다.
전체 PhysX capsule sweep이나 게임 화면의 완전한 충돌 검증을 대신하지 않는다.

시내와 항구는 별도 detail region으로 기존 Set Sail 이동을 사용한다. 둘 사이 도보 연결은
추가하지 않았다. Bern2의 npc.bern.src.42는 BernCastleIndoor_11의 별도 실내이며 현재
Gameplay에 대응 입구가 없다. 성/도서관 nav까지 억지로 연결하면 허공 길이 되므로 보존했다.
이 NPC의 실내 입장 기능 추가는 별도 미구현 사항이다.

디스크 데이터 반영은 완료했지만 실행 중 Server의 nav가 자동 갱신된 것은 아니다.
새 Movie/Guide/Nav 코드를 포함한 정식 Product Debug·Release 링크, 서버 재시작 및 사용자의
실제 제작 지구/항구 이동·건물 동행·Movie FPS 확인은 아직 남아 있다. 게임을 자동 종료하거나
편집 중 Reload하지 않았다. [전체 코드와 절차 PLAN](2026-10-02_BERN_NAV_GROUND_RECOVERY_PLAN.md).

반영 후 공식 `Publish-ServerNavigation.ps1 -Mode Validate -AreaId LV_BER_BERNCASTLE`도 exit0이다.
새 통합 queue PID2704의 사용자 앱 종료 대기를 실제 확인했다. 입력47개와 HEAD를 고정하며
게임을 종료하지 않고 Debug→Release Product를 순차 실행한다. 시작/대기 영수증은 외부 Recovery의
`GuideNavMovie-Build-Queue-Started.json`과 `GuideNavMovie-Build-Queue-Start-Verified.json`이다.
현재 이 두 Product 빌드는 아직 시작하지 않았다.
