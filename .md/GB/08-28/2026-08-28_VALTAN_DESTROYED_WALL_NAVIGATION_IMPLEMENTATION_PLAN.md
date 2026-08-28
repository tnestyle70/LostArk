# 발탄 파괴 벽 이동·돌진·몸체 충돌 반경 구현 계획

작성일: 2026-08-28

초기 구현 기준: main `cd120501`에서 분기 후 PR #251 `a6871a2f` 통합, `codex/valtan-arena-navigation-next`
초기 구현 폴더: `C:/w/valtan-arena-next`
최종 적용 기준: PR #251 `a6871a2f` 기반 `codex/valtan-arena-next-desktop`, `C:/Users/user/Desktop/LostArk/Framework.sln`

반영 내용과 실제 검증·미검증 경계는
[벽·Nav RESULT](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_DESTROYED_WALL_NAVIGATION_IMPLEMENTATION_RESULT.md)에 기록한다.

## G00. 현재 호출·데이터 계약

`Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`의 collisionBox와
`ValtanWorldEvents.json`의 mutation을 `Publish-ValtanWorldDestruction.ps1`이 연결한다.
Server `CommitWorldDestructionTransaction`은 collision/nav를 stage한 뒤 파괴 상태와 함께 commit한다.
`Prepare_StateChanges`의 정확 ID 또는 점으로 구분한 자식 ID 규칙을 유지한다.

입구 frontwallA/B의 source collision은 숫자 leaf지만 receiver는 group의 형제 leaf다.
변경 전 publisher는 source leaf만 mutation에 넣어 파괴 후 두 receiver를 남겼다.
돌진의 swept body와 플레이어 이동이 이 잔존 collisionBox에 막힐 수 있어,
현재 publisher는 source와 sibling receiver를 함께 소유하는 입구 group prefix를 검증한다.

변경 전 nav 분할 도구의 nearest-wall/nearest-cell fallback은 일부 벽과 수 m 떨어진 셀을
그 벽의 소유로 배정했다. XZ만 비교하면 위층 벽이 아래층 길까지 막는다. 현재는 실제 collision footprint와
셀 높이에서 시작하는 Server body의 수직 범위를 함께 비교하여 재계산한 정본을 검증한다.
기존 base nav, 바닥 높이, 84/30 바닥 붕괴의 반대 polarity는 유지한다.
main에 이미 반영된 109의 외벽30+내벽67 파괴 스케줄도 유지한다.

## G01. 입구 receiver 소유권

- 수정: `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1`의 collisionStateId 선택.
- 입구만 source와 receiver를 모두 포함하는 정확 group prefix를 사용한다.
- 같은 prefix에 다른 파괴 group의 collision이 섞이거나 receiver가 mutation 범위를 벗어나면 거부한다.
- 다른 벽은 기존 source leaf와 `.receiver` 자식 계약을 유지한다.
- publisher ContractTest는 모든 충돌체의 유일한 mutation 소유와 최종 해제 범위를 검증한다.
- 현재 99개 벽의 collisionBox 141개를 각각 한 mutation이 소유한다. BREAKING에서는 추가 impact 수신을 막고
  벽의 이동 차단은 유지한다. DESPAWNED의 due tick에서 source·receiver collision과 해당 Nav를 함께 해제한다.
- 생성 bootstrap은 publisher로만 재생성한다.

## G02. 벽 nav와 이동 경로

- `Data/Navigation/LV_LUT_HEARTRB_ED.navblockers`의 wall 영역만 source collision의 XZ OBB와
  base-walkable 셀의 실제 교차로 교정한다. 셀 ground Y에서 `BossProfiles.json`의 radius 1.4m로 얻은
  body 높이 2.8m와 `WorldCollisionContract.h`의 player 높이 1.8m를 함께 검사한다. 실제 겹치는 벽의 refcount는 유지한다.
- 공통 계산은 `Tools/WorldPipeline/ValtanWallNavigation.ps1`을 repair와 publisher가 함께 사용한다.
  다른 층밖에 없는 벽에 nearest cell을 억지로 만들지 않는다. 99개 벽 중 98개에 wall nav가 있다.
  기존 floor region 6개를 합쳐 104개 영역을 유지하며, wall cell 참조 3905개·고유 셀 2109개다.
- `Tools/WorldPipeline/Split-ValtanIndependentWallGroups.ps1`의 오래된 nearest 배정이
  잘못된 셀을 다시 생성하지 않게 수정한다.
- publisher에서 셀의 실제 소유 geometry, 층 높이와 base walkability를 검사한다.
- `Server/Private/ServerNavigation.cpp`의 `Has_LineOfSight`는 현재 dynamic blocker도 검사한다.
  고정 간격 표본 대신 DDA/supercover로 교차하는 모든 셀과 모서리·경계의 양쪽 셀을 확인한다.
  `Sample_Position`의 바닥 높이 조회와 낙하 소비 계약은 변경하지 않는다.
- 기존 `ServerGameplayContractTests.cpp`에 파괴 전/후 이동·돌진·경로, 이웃 벽 및 붕괴 바닥 보존을 추가한다.

## G03. 현재 발탄 크기의 몸체 반경

변경 전 `BossProfiles.json`의 radius는 3m이고 Server spawn이 Client Debug sphere까지 전달한다.
Client collider는 translation만 적용하므로 presentationScale의 중복 곱은 없다.
body WModel의 실제 skin palette와 현재 `presentationScale=1.0`을 수치로 측정하여
몸통/다리 기준의 반경 1.4m로 줄인다. idle/run 99개 frame에서 다리의 최대 XZ 반경은 약 1.3781m다.
무기·뿔·공격 때 뻗는 팔은 몸체 반경에 포함하지 않는다.

`Data/Balance/BossProfiles.json`과 provenance receipt를 갱신하고 기존 balance publisher로 생성한다.
presentationScale, 공격 shape, 스킬 사거리, 장판 반경은 이 변경으로 같이 축소하지 않는다.
Server sweep과 Client spawn radius가 같은 profile 값을 소비하는 실행 계약을 검사한다.

## G04. 검증·실행 경계

최종 빌드·실행 대상은 `C:/Users/user/Desktop/LostArk/Framework.sln`이다.
단계별 자동 검증은 재개했으며, 개별 성공과 최종 Desktop 전체 회귀 완료를 구분한다.
최종 Desktop Debug/Release 전체 자동 검증과 사용자 시각 판정은 미완료이며, 선행 worktree의 검증 이력과 구분한다.

Navigation/WorldDestruction/GameplayBalance publisher Validate와 관련 ContractTest,
Shared protocol 및 Server gameplay harness, Debug/Release build를 실행한다.
빌드 전 Engine → UpdateLib → Shared → Server → Client 순서를 유지한다.
변경 JSON/XML parse와 `git diff --check`를 확인한다. 사용자 승인 없이 기존 실행 중인 Server/Client를 종료하지 않는다.

새 C++ 파일은 필요하지 않으므로 프로젝트·filter에 신규 소스 등록은 없다.
Next Pattern/Trash의 신규 wire·harness는 별도 대응 계획에서 다룬다.
에이전트는 Client/UI를 실행하거나 캡처하지 않는다. 최종 화면상 크기와 이동은 사용자가 판정하며,
자동 수치 검증과 수동 미확인 항목을 RESULT에서 분리한다.
