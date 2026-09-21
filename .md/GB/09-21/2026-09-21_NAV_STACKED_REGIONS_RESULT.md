# 2026-09-21 네비게이션 영역 층 겹침(Stacked layers) 설계 변경 RESULT

확정 수준 표기: **[실측]** 이번 작업에서 명령을 실행해 출력으로 확인, **[추정]** 실측 값에서 추론했지만 직접 확인하지 않음, **[미확인]** 하지 않았거나 확인하지 못함.

## 1. 요약

베른 Bern(위)과 Bern3(아래)처럼 XZ 범위가 겹치는 세부 영역을 "서로 다른 높이 층"으로 허용했다. 종전에는 게시기와 Server가 영역 XZ 사각형이 겹치기만 해도 거부해서 Bern3를 게시할 수 없었다(`Navigation regions overlap: ... Bern and ... Bern3`). 이제 겹친 XZ에서 양쪽 모두 걸을 수 있는 셀의 지면 높이 차가 `max(2 m, 두 영역 step 정책 중 큰 값의 2배)`보다 작을 때만 거부하고, 통과한 층은 질의가 넘긴 높이 힌트로 고른다.

- 게시기·Server·문서·테스트를 바꿨다. **작업 폴더에 미커밋으로 남아 있다.** 커밋·게시·제품 빌드는 하지 않았다.
- 사용자의 현재 Bern3 원본(`Bern3.navsource`/`.navpaint`, `navregions`)은 수정하지 않았다 [실측: 수정 시각 13:08~13:12 그대로]. 이 원본은 새 규칙으로 **이미 게시가 통과한다** [실측: 작업 폴더 밖 사본 게시].
- 게시된 산출물(`Server/Bin/DataFiles/Navigation`, `Client/Bin/DataFiles/Navigation` 84개)은 작업 전후 해시가 같다 [실측].

## 2. 새 계약

1. **로드/게시 시 거부 조건**: 두 영역의 XZ 사각형이 겹치고, 겹친 XZ에서 양쪽 모두 걸을 수 있는 셀이 있으며 그 지면 높이 차가 `max(2 m, 2 × max(step A, step B))`보다 작으면 거부한다(경계는 포함해서 통과: 정확히 2.0 m는 통과). 각 격자의 모든 walkable 셀 중심을 상대 격자의 그 위치 셀과 짝지어 검사한다. 게시기(`Publish-ServerNavigation.ps1`)와 Server(`CServerNavigation::Find_AmbiguousLayerOverlap`)가 같은 규칙을 적용한다.
   - 근거: 실제 갑판 위 이동체의 높이는 자기 지면에서 step 한 번 정도 어긋날 수 있다. 두 층 지면이 그보다 충분히 멀어야 "지면이 가장 가까운 층"이 한 값으로 정해진다. 하한 2 m는 step이 작은 Area에서도 최소한의 여유를 준다.
2. **질의가 층을 고르는 순서** (`CServerNavigation::Select_Region`, 이 선택은 이 함수 한 곳이 소유):
   - 질의의 XZ를 담는 영역이 0개 또는 1개면 종전과 똑같다(영역이 없으면 기본 격자).
   - 여러 개면: 그 XZ에 걸을 수 있는 셀이 있는 영역이 없는 영역보다 앞선다 → 그중 지면 높이가 힌트에 가장 가까운 영역 → 동률이거나 힌트가 없으면 매니페스트 선언 순서.
   - 지면이 없는 후보끼리는 그 영역 walkable 셀의 높이 범위와 힌트 사이 거리로 정한다.
3. **호출자가 높이를 넘긴다.** `Find_Path/Smooth_Path/Find_PathToReachablePointWithinRadius/Project_Point/Project_PointOnSameLevel/Sample_Position/Resolve_TraversalStep/Has_LineOfSight/Is_PointWalkableExact`에 마지막 선택 인자(`startY`/`hintY`/`fromY`, 기본 `NAVIGATION_HEIGHT_UNKNOWN`)를 더했다. 인자를 안 넘기는 기존 호출은 종전과 같이 컴파일되고, 겹친 XZ에서만 "매니페스트 순서"로 답한다.
4. 층 사이는 걸어서 이어지지 않는다(영역 밖으로 걷지 않는 기존 규칙 그대로). 오가는 것은 movePlayer 같은 저작된 이동이다.
5. 매니페스트 형식(`LOSTARK_NAVGRID_REGIONS 1`), 런타임 파일 형식, Shared protocol은 **바꾸지 않았다.**

## 3. 바꾼 파일

Server 코어
- `Server/Public/ServerNavigation.h` (+58 −11), `Server/Private/ServerNavigation.cpp` (+186 −34): 높이 힌트 선택, 층 모호성 검사, 힌트 인자.
- `Server/Private/ServerGameplayContractTests_Navigation.cpp` (+162): 층 계약 테스트 9건.

호출부(높이 힌트만 붙임)
- `GameRoom_PlayerCommands.cpp` 8곳(클릭 이동 목표·시야·경로·평활화, 부활 위치, 에스더 소환 착지, 클래스 변경 스폰 투영), `GameRoom_PlayerSimulation.cpp` 6곳(이동 중 시야·걸음·재경로·넉백 클램프), `PlayerSkillSystem.h/.cpp`(`Clamp_StepToWalkable`에 `startY` 추가, 스킬 조준 지점·스킬 이동), `GameRoom_VehicleRiding.cpp` 1곳, `MonsterBrain.cpp` 1곳(넉백 클램프), `NpcBehaviorRuntime.cpp` 11곳(배회·순찰·스폰 투영, 단일 호출부의 지역 헬퍼에 `currentY` 추가).

게시기
- `Tools/NavigationPipeline/Publish-ServerNavigation.ps1` (+232 −15): 겹침 검사를 위 규칙으로 교체, 배치(스폰/보스) 검사가 배치 높이로 소유 격자를 고르도록 `Select-NavigationOwnerGrid` 추가, C# 측정 함수(`MeasureLayerOverlap`, `GroundAt`, `WalkableBand`), `ContractTest`에 층 테스트 추가.

Client
- `Client/Private/LevelNavigationDebug.cpp` (+2 −13): F1 `Show Navigation`이 "게시된 세부 격자가 겹치면 Reload 실패"로 거부하던 코드를 제거했다(겹친 층도 각자 높이로 그려 그리기 변경은 없다).

문서
- `CLAUDE.md`(서버 길찾기 문장 정정), `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`(세부 영역 절에 층 규칙과 "호출자가 높이를 넘긴다" 추가).

인코딩: 전부 ASCII(문서는 UTF-8)이며 줄끝 CRLF를 그대로 유지했다 [실측: bare LF 0, `git diff --check` 지적 없음]. wave-buttons 브랜치의 로컬 커밋(`a6020164`, `9c473666`)이 건드린 파일과 겹치는 파일은 없다 [실측].

## 4. 검증 (실행한 것과 결과)

모든 실행은 작업 폴더 밖 사본(`out/NavStackedRegions20260921/`, `C:\Users\USER\.claude\jobs\46aea322\tmp\navlayer\`)에서 했다. MSBuild 제품 빌드가 아니라 `cl`/`link` 직접 호출(Debug, `/std:c++20 /Od /MDd`)이다.

1. **수정 전 기준선** [실측]: HEAD `1cd9f938`의 Server+Shared 96개 TU를 컴파일·링크(경고 0)해 `Server.exe --navigation-contract-test`를 돌렸다. 25건 통과, `navigation failures : 0`(실제 Bern 데이터 포함).
2. **수정 후 Server** [실측]: HEAD `9c473666`(wave 브랜치 로컬 커밋 반영) + 제 파일 전부를 새로 컴파일·링크(경고 0). 같은 테스트 34건 통과(기존 25 + 새 9), `navigation failures : 0`.
3. **새 테스트가 옛 구현에서 실패함** [실측]: 옛 `ServerNavigation.cpp`에 새 테스트 파일(힌트 인자 6곳만 제거한 변형)을 컴파일·링크해 돌리면 기존 25건은 통과하고 새 9건이 실패한다(`navigation failures : 9`, 종료 코드 1). 한계: 첫 항목(층 로드)이 실패하면 뒤 항목이 연쇄로 실패하므로, 힌트 선택·지면 우선·동률 규칙 자체를 옛 코드로 가려내지는 못한다. 그 규칙들은 새 코드에서만 통과를 확인했다.
4. **게시기 `ContractTest`** [실측]: 수정 게시기 통과(종료 코드 0). HEAD 게시기에 새 층 테스트 함수만 끼워 돌리면 `Stacked layers 8 m apart were rejected`로 실패한다.
5. **기존 Area 회귀 없음** [실측]: HEAD 입력(`git archive`)으로 원본 게시기와 수정 게시기를 각각 사본에서 돌려 만든 산출물이 Server 42개·Client 42개 모두 **바이트 동일**하고, 커밋된 `Server/Bin/DataFiles/Navigation`·`Client/Bin/DataFiles/Navigation`과도 전부 동일하다(6개 Area).
6. **사용자의 현재 Bern3 게시** [실측]: 수정 게시기로 게시가 성공한다(`Bern3 191x110 cellSize=0.5 walkable=4764`). 기존 41개 파일은 그대로이고 `LV_BER_BERNCASTLE.navregions`만 3영역으로 바뀌며 `Bern3.navgrid/.navblockers/.navpolicy`가 Server·Client 쪽에 각각 추가된다.
7. **겹침의 실제 수치** [실측]: Bern 걷는 셀 80862개(지면 Y 9.28~56.00), Bern3 4764개(Y 7.38~22.36). 두 격자가 같은 XZ에서 모두 걸을 수 있는 곳은 14곳(양방향 세면 28쌍)이고 높이 차는 최소 14.664 m(x=219.75, z=−201.25, 34.057/19.393), 최대 20.675 m다. 2 m 미만은 0쌍이라 기준의 7배 이상 여유가 있다. Bern 전체의 높이 범위가 Bern3와 겹치므로 "Bern이 항상 위"가 아니라 겹친 XZ마다 지면 차이로 구분한다.
8. **실제 데이터로 Server 로드·질의** [실측]: 위 게시 산출물을 새 Server 코드가 로드한다(`regions=3`). 같은 XZ에서 힌트 34.1은 34.057(Bern), 19.4는 19.393(Bern3), 힌트 없음은 34.057(선언 순서상 Bern)을 답한다. Bern3 범위 격자점 21010곳에서 아래층 힌트가 Bern3 지면(<23 m)으로 답한 곳 4764, 두 힌트가 서로 다른 층을 고르는 곳 14. 걸음 검사: 아래층 인접쌍 4330 중 3813 허용(모두 아래층에 남음)·517은 기존 1 m 단차 규칙으로 막힘·이상 0, 위층 28쌍 중 23 허용·5 단차 막힘·이상 0.
9. **호출부 변경 영향 없음** [실측]: 수정 전/후 빌드로 같은 데이터에서 다른 계약 테스트를 150초 상한으로 돌려 출력을 줄 단위로 비교했다. `bingo`(70), `card-maze`(69), `kouku-object-overlap`(88), `vehicle-riding`(25), `world-playback`(50 통과·2 실패), `kouku-support-surface`(180 통과·4 실패, 타임아웃 시점까지), `debug-teleport`(무작위 `[MarioControl] layout=` 줄을 뺀 451줄)가 모두 같다. 실패 항목은 수정 전에도 똑같이 실패한다(기존 실패). 이 비교는 **일부 테스트의 앞부분**이다: `kouku-support-surface`와 `debug-teleport`는 타임아웃으로 끝까지 가지 못했다.
10. **Client 오버레이 문법** [실측]: `cl /Zs`로 수정 전 파일(대조군)과 수정 후 파일 모두 종료 코드 0(링크·제품 빌드 아님).
11. **작업 폴더 무변경** [실측]: 게시 산출물 84개 해시 동일, Bern3 원본 세 파일 수정 시각 동일.

## 5. 하지 않은 것 / 확인하지 못한 것

- [미확인] MSBuild 제품 빌드(`Invoke-BuildAndRegression.ps1`) 성공 여부. 사본 컴파일은 PCH·`/MP` 없이 했으므로 제품 빌드와 완전히 같은 조건이 아니다.
- [미확인] Client 전체 빌드와 실행, 게임 안 이동·충돌 체감, Server를 다시 시작한 뒤의 동작.
- [미확인] `Server.exe --contract-test` 전체(과거에 60분 넘게 걸리며 기존 실패가 있어 돌리지 않았다). 위 9번의 일부 테스트만 비교했다.
- [미확인] `-AreaId LV_BER_BERNCASTLE`로 좁힌 게시 명령(6개 Area를 모두 게시하는 형태만 사본에서 실행했다).
- [미확인] 사용자가 Bern3를 재저작한 뒤의 게시 결과. 현재 원본 기준으로만 통과를 확인했다.
- 게시는 하지 않았다. `Server/Bin/DataFiles`, `Client/Bin/DataFiles`는 그대로다.

## 6. 한계와 위험

- **호출부 일부는 아직 높이를 넘기지 않는다** [실측]. 층이 겹치는 곳은 지금 Bern뿐이라 실효 영향은 없지만, 다음 파일의 네비 질의는 겹친 구간에서 "선언 순서 첫 번째 걸을 수 있는 영역"이 답한다: 보스/패턴(`GameRoom_BossSimulation/BossStageActions/ValtanAudition`, `ValtanBrain`), Kouku(`GameRoom_Kouku*`, `KoukuSaydon*`), `GameRoom_WorldEntities/WorldDestruction/Helpers/Admission`, `GameRoom.cpp`, `MonsterBrain.cpp`의 넉백 외 질의, `Restore_PatternBoundPlayer`와 마리오 레일·아레나 추방 경로. 새 호출을 추가하면 자기 높이를 넘겨야 한다(가이드에 적었다).
- **지면이 있는 영역이 지면 없는 영역보다 앞선다** [실측 수치]. 그래서 Bern3 범위에서 아래층 힌트의 질의가 "Bern3에는 지면이 없고 Bern에만 있는" XZ에서는 위층 지면으로 답한다(격자점 21010곳 중 45곳). 이동 질의는 이동체 자신의 위치로 층이 정해져 영향이 없지만, 목표 지점 투영·스폰 같은 직접 투영 질의는 그런 XZ에서 위층 높이를 돌려줄 수 있다 [추정: 실제 게임에서 그런 위치를 누르는지는 확인하지 않았다].
- **Client 제품 Loader는 영역 격자를 읽지 않고 기본 격자만 쓴다** [실측: `Loader.cpp`가 `<Area>.navgrid`만 `Create_NavGrid`로 로드, Client/Engine 검색에서 영역 소비처는 F1 디버그 오버레이와 MapTool뿐]. 클릭 이동의 로컬 예측이 Bern3 층에서 어떻게 보일지는 확인하지 않았다 [미확인]. Server 스냅샷이 위치를 확정하는 구조는 그대로다.
- 힌트 없는 질의의 겹친 구간 답은 매니페스트 순서(현재 `Bern`, `Bern2`, `Bern3`)에 의존한다. 순서를 바꾸면 그 구간의 힌트 없는 답이 바뀐다(힌트 있는 답은 바뀌지 않는다).
- `Tools/KoukuSaydonPipeline/validate_kouku_saydon_world_admission.py`의 `_select_grid`는 "첫 영역 우선" 옛 규칙을 복제하고 있다. Kouku 영역 4개는 서로 겹치지 않아 결과가 같지만 [실측] 층을 쌓게 되면 같은 규칙으로 고쳐야 한다. 이번에는 건드리지 않았다.
- 새 검사는 walkable 셀 전수를 짝짓는다(Bern×Bern3 게시 시 수 초 이내였다).

## 7. 사용자가 다음에 할 일

1. Bern3를 재저작하지 않아도 현재 원본으로 게시가 통과한다. 재저작하면 겹친 XZ의 지면 높이 차가 `max(2 m, 2 × step)` 이상인지 게시 로그(`Navigation regions overlap: ... only N m apart`)로 확인된다.
2. 게시: `powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Server` 또는 네비만 `powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Publish`. 결과로 `Server/Bin/DataFiles/Navigation`과 `Client/Bin/DataFiles/Navigation`에 `LV_BER_BERNCASTLE.Bern3.*`와 갱신된 `LV_BER_BERNCASTLE.navregions`가 생긴다. 이 파일들은 Git 전달 대상이다.
3. Server(코드가 바뀌었다)는 Product Build 후 재시작이 필요하다. Client는 오버레이 수정 때문에 다시 빌드하면 F1 `Show Navigation`이 겹친 층을 로드한다(안 해도 게임 진행에는 영향 없다).
4. 실제 확인: Bern3 층으로 가는 movePlayer 목표 높이를 Bern3 지면으로 두고, 그 층에서 클릭 이동·스킬 이동이 아래층에 남는지 직접 확인한다.

## 8. 산출물 위치

- 사본·측정 출력: `out/NavStackedRegions20260921/` (git 제외). 게시기 사본 출력은 `pub/{orig,mod,bern3}/out/`.
- 패치·빌드·검증 스크립트: `C:\Users\USER\.claude\jobs\46aea322\tmp\navlayer\` (`patch_server_nav.py`, `patch_nav_tests.py`, `patch_publisher.py`, `patch_callers.py`, `patch_overlay.py`, `patch_docs.py`, `build_srv.py`, `compare_outputs.py`, `measure_bern3.py`, `bern3_probe.cpp` 등). 작업 잡을 지우면 함께 사라진다.
