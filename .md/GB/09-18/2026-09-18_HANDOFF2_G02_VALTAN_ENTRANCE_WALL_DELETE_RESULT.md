# 2026-09-18 HANDOFF2 G02 — 발탄 입구 011·025·026 실제 삭제 RESULT

입력: `2026-09-18_HANDOFF2_INTEGRATED_REPAIR_PLAN.md` G02와 사용자 확정 지시("입구 벽은 미리보기 숨김이 아니라 실제 삭제").
범위: 109 외곽 링 슬롯 011·025·026의 본체+틈막이 6배치와, 그 배치에만 걸린 충돌·네비·파괴 참조, 이를 게시할 좁은 게시 범위.

## 기존 반영 (이번 작업 전 상태)

- 외곽 링은 30슬롯(12° 간격) 60배치(`DEPLOY_ITR_02306` 본체 30 + `DEPLOY_ITR_02307` 틈막이 30)로 입구를 360° 막고 있었다.
- `Publish-MapAuthoring.ps1`은 Scope `Area | WorldSequences | Lights`뿐이었고 deploy는 `Area`(조명 동반)로만 게시됐다.
- `Publish-WorldGameplay.ps1`은 `ALL | KAKULSAYDON_ARENA`만 받았고 Client World 출력 경로가 고정이었다.
- `Publish-ServerNavigation.ps1`은 `-OutputRoot`를 바꿔도 Client 출력이 live `Client/Bin/DataFiles/Navigation`에 고정이었다.
- 변경 전 검증 8개 전부 EXIT 0 (`out/Handoff2G02Baseline/validate_baseline.log`).

## 삭제 전 대조 (전부 일치 — 삭제 진행 조건 충족)

| 항목 | 계획서 | 실제 |
|---|---|---|
| deploy 6행 | 011/025/026 본체·틈막이 좌표·asset·Y 23.04 | 좌표 6자리까지 일치, asset 02306/02307, Y 23.040000 |
| WorldEvents group | 3, memberPlacementIds = 본체+틈막이 쌍 | groups[20..22], 쌍 일치 |
| mutation / binding | 3 / 6 | mutations[20..22] / bindings 14·28·29(109 IMPACT)·84·98·99(돌진 COLLISION_IMPACT) |
| collisionBox | 슬롯당 본체+`.receiver` | 6개 존재 |

## 이번 수정

### 데이터 (바이트 범위 삭제만, 나머지 바이트·서식·줄끝 무변경)

| 파일 | 변경 |
|---|---|
| `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployplacements` | 6행 삭제, 헤더 151→145 (CRLF) |
| `Data/Encounters/Valtan/ValtanWorldEvents.json` | group 3, mutation 3, binding 6 삭제 |
| `Data/Valtan/Valtan.worldeventsets.json` | set `arena-break-109.outer-wall` 멤버 3 삭제 (97→94). **계획서 파일표에 없던 파일**: 발탄 파이프라인이 이 멤버로 `ValtanWorldEvents.json` 109 바인딩을 같은 순번으로 투영하므로, 빼지 않으면 없는 바인딩을 가리켜 검증이 깨진다. 승인된 "삭제한 벽의 파괴 참조" 범위로 판단해 같은 3개만 제거 |
| `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json` | collisionBox 6 삭제, revision 573→574. 기존 미커밋 변경 보존(편집 직전 백업 대비 삭제 97줄 + revision 1줄뿐) |
| `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.destructionsimulation.json` | profile 3 삭제 |
| `Data/Navigation/LV_LUT_HEARTRB_ED.navblockers` | REGION 3블록(25+47+43=115칸)과 헤더 104→101. 조건 `condition.valtan.outerwall109.<id>.destroyed`는 각 region 전용이라 함께 사라짐, 공유 region/condition 없음 |

삭제된 115칸은 기본 네비에서 대부분 바닥+walkable(높이 21.7~25.5m)이다. 허공이 아니다.
보존: frontwallA/B(02315/02316), 나머지 27슬롯 54배치, 바닥·기둥·159통로, 전투 stage·대미지·보상, 모델 asset 파일.

### 도구

| 파일 | 변경 |
|---|---|
| `Tools/WorldPipeline/sync_valtan_109_outer_wall_gap_fillers.py` | `SOURCE_COUNT` 제거. `SOURCE_SLOT_COUNT=30`(원래 기하) + `APPROVED_REMOVED_SLOT_ANGLES={11:120,25:132,26:144}` + `ACTIVE_SUFFIXES`(27). 삭제 슬롯이 deploy/events/simulation에 다시 나타나면 오류, 미승인 누락도 오류, 각도 격자 검사는 "전체 − 승인 3슬롯" |
| `Tools/WorldPipeline/test_sync_valtan_109_outer_wall_gap_fillers.py` | fixture를 27슬롯으로. 추가 5개: 승인 삭제 유지·재실행 멱등, 미승인 누락(12번) 오류, 삭제 슬롯(25번) 재등장 오류, 삭제 그룹(26번) 이벤트 재등장 오류, 행 순서 뒤집기 후 동일 행·안정 |
| `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1` | 30/60/30 상수를 `$outerRingSlotCount=30` − `$approvedRemovedOuterSlotIds`(3)에서 파생(27/54/27), 돌진 40→`10+27`, 제품 그룹 99→`69+27`. 1~30 슬롯 각각 "승인 삭제면 없어야, 아니면 있어야" 정확 집합 검사 추가. 025를 쓰던 debris recipe 음성 테스트를 잔존 024로 교체(검사 유지) |
| `Tools/WorldPipeline/Split-ValtanIndependentWallGroups.ps1` | 회귀 검사(`-Mode CheckNavigation`)의 독립 벽 99→96, Migrate "이미 분할" 판정 외곽 30→27. 레거시 마이그레이션 분기(508·547·769행)는 과거 데이터 전용이라 미변경 |
| `Tools/MapPipeline/Publish-MapAuthoring.ps1` | **신규 `-Scope Deploy`**: 출력은 `<Area>.deployassets`, `<Area>.deployplacements` 2개뿐. 조명·재질·이펙트·물·카메라샷·월드시퀀스·맵 배치 미출력. deploy 카탈로그는 runtime과 **내용**(줄끝 제외)이 같을 때만 실리고, 다르거나 미게시면 거부 |
| `Tools/WorldPipeline/Publish-WorldGameplay.ps1` | **신규 `-WorldId VALTAN_ARENA`**: 발탄 world/spawn/encounterprops bootstrap + 발탄 npcpresentation·viewer만. 이 범위에서는 쿠크 stagemarkers와 공용 SequenceViewer.labels를 쓰지 않음. **신규 `-ClientOutputRoot`**(기본 `Client/Bin/DataFiles/World`) |
| `Tools/NavigationPipeline/Publish-ServerNavigation.ps1` | **신규 `-ClientOutputRoot`**(기본 `Client/Bin/DataFiles/Navigation`), 절대/상대 경로 허용. region 없는 Area 게시 시 이전 runtime `.navregions`(Server·Client)를 같은 트랜잭션에서 삭제하고 실패 시 복원(G05-6). 편집은 임시파일+원자 교체 |

### 서버 계약 테스트 (C++, 실제 게시 데이터 개수 단언 갱신)

| 파일 | 변경 |
|---|---|
| `WorldDestructionBootstrapContractTests.cpp` | 그룹·변이 105→102, 바인딩 224→218, 멤버 143→137, 외곽 30→27, 외곽 멤버 60→54, 109 전이 97→94, 외곽 전이 30→27, 배치 135→129, 메시지 |
| `ServerGameplayContractTests_ValtanDash.cpp` | 외곽 receiver 30→27, 온전 외곽 30→27, 부분 배치 BREAKING 29→26, 최종 DESPAWNED 30→27, 메시지 |
| `ServerGameplayContractTests_SpawnGroups.cpp` | 109 BREAKING 97→94, 다음 이벤트 seq 98→95, 이벤트 수·마지막 seq 97→94, 주석 |
| `ServerGameplayContractTests_Runner.cpp` | 발탄 충돌 박스 141→135(외곽 본체 6개·receiver 포함 슬롯당 2개 제거), 주석 |
| `ServerGameplayContractTests_WorldDestruction.cpp` | "131°(입구)도 막혀야 한다" 단언을 **사용자 결정에 맞게 반대로**: 131°는 통과, 150°·294°는 여전히 차단. 0°·60°·216° 차단 단언 유지. `sweepAcrossRing(bearing, expectBlocked=true)` |

131°/150° 판정 근거[계산]: 삭제 후 collisionBox로 서버 스윕(반경 15→21m, 플레이어 반경 0.45m, 높이 1.8m)을 파이썬으로 재현 → 131°·140° 열림, 120°·125°(다른 벽 `17695249952621776147`)·145°·150°(027 등) 차단. **실제 C++ 실행 판정은 Server 빌드 후 `--contract-test`로 확인해야 한다.**

## 게시 (승격 완료)

1. staging(`out/Handoff2G02Staging`)에 네 게시를 먼저 실행하고 live 16개 파일 해시 무변경을 확인했다.
2. 14:06, Server.exe·Client.exe 미실행(devenv만) 확인 후 한 묶음으로 승격. 실패 시 전체 백업(`out/Handoff2G02Promote/backup`)에서 복원하는 스크립트로 실행. 네 단계 모두 EXIT 0, 승격본 = staging 14/14 일치.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Publish -AreaId LV_LUT_HEARTRB_ED
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1 -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish -WorldId VALTAN_ARENA
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Scope Deploy -Mode Publish
```

내용이 바뀐 live 출력: Server/Client `LV_LUT_HEARTRB_ED.navblockers`(104→101), `VALTAN_ARENA.worldbootstrap`(rev 574, 159→153), `VALTAN_ARENA.worlddestructionbootstrap`, Client `LV_LUT_HEARTRB_ED.worlddestruction.json`, `.worlddestructionpresentation.json`, `.viewer.world.json`, `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.deployplacements`. 같은 바이트로 재기록: navgrid·navpolicy·spawngroups·encounterprops·npcpresentation, deployassets(내용 동일, CRLF→LF).
무변경 확인: 발탄 maplights/mapmaterials/mapeffects(09-16), 쿠크 stagemarkers·SequenceViewer.labels·BERN.worldbootstrap(10:53), 메인이 13:44에 게시한 베른 네비 7파일.

참고[확인]: 앞서 "발탄 deployassets 게시본이 저작본과 다르다"던 판정은 **내용이 아니라 줄끝**(git 체크아웃 CRLF vs publisher LF) 차이였다. 13줄 내용 동일.

## 자동 검증 (명령·exit code)

| 명령 | 결과 |
|---|---|
| `Project-ValtanPatternMaster.ps1 -Mode Validate` | 0 (worldMembers 94, Product drift 없음) |
| `Publish-ValtanWorldDestruction.ps1 -Mode Validate` | 0 (groups 102, bindings 218, outer 27/54/27, dash 37, contacts 69) |
| `Publish-ValtanWorldDestruction.ps1 -Mode ContractTest` | 0 |
| `python -B test_sync_valtan_109_outer_wall_gap_fillers.py` | 11 tests OK |
| `sync_valtan_109_outer_wall_gap_fillers.py --check-only` | 0 (재실행 diff 0) |
| `Split-ValtanIndependentWallGroups.ps1 -Mode CheckNavigation` | 0 |
| `Publish-ServerNavigation.ps1 -Mode Validate -AreaId LV_LUT_HEARTRB_ED` | 0 |
| `Publish-ServerNavigation.ps1 -Mode ContractTest` | 0 |
| 잔존 `.navregions` 제거 격리 시험(`out/Handoff2G02NavManifestTest`) | 0, Server·Client 가짜 manifest 삭제 확인 |
| `Publish-WorldGameplay.ps1 -Mode Validate` (ALL) / `-WorldId VALTAN_ARENA` | 0 / 0 (VALTAN 153 placements) |
| `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Mode Validate` (Area) | 0 (FileCount 24) |
| `... -Scope Deploy -Mode Validate` / `-Mode Check`(승격 후) | 0 / 0 |
| 격리 `/Zs` 서버 테스트 5 TU (`out/IsolatedCompile_G02`) | 5/5 EXIT 0, 경고는 기존 헤더 C4819뿐 |
| `git diff --check` (변경 22파일) | 0 |
| 저작 원본·live 게시본 삭제 ID 잔존 | 0 (남은 1곳은 파괴 게시 스크립트의 승인 삭제 목록) |
| 전투·카메라·밸런스·조명 원본 | git 무변경 (`ValtanEncounter`, `ValtanCinematicCamera`, `Data/Balance`, `Valtan.gameplay/presentation`, `maplights`, `Tools/ValtanPipeline`) |

## 사용자 확인 (미실시 — 화면 판정은 사용자 몫)

- **Server 재시작 필요**(worldbootstrap·navblockers·파괴 bootstrap 변경). Client도 재실행(네비·deploy·파괴 runtime 변경). 게임 동작용 C++ 빌드는 불필요.
- 서버 계약 테스트를 돌리려면 Server를 빌드한 뒤 `Server.exe --contract-test`. 여기서 131° 통과·150°/294° 차단 단언과 개수 단언이 실제로 확정된다.
- 확인 위치:
  - 입장: Stage_Boss(반경 36.5m) → Stage_Boss_ArenaEntry(반경 18.8m, 방위 150°, triggerOnce)로 들어오는 흐름. 입구 방향(아레나 중심 기준 대략 방위 120~148°)에 벽이 없어야 한다.
  - 입장 컷신 카메라가 025 자리를 가림 없이 지나가는지.
  - 109 IMPACT: 남은 27장이 날아가고 입구 3슬롯 자리엔 아무것도 없어야 한다.
  - 돌진(VALTAN_DASH_CHARGE)이 남은 외곽 벽에 부딪혀 부서지는지.
  - 재입장·리셋 후 벽 상태.

## 미지원·추가 승인 필요 / 위험

- **전투 중 입구 이탈**[확인]: 서버에 발탄 leash·아레나 경계 리셋이 없다(`ENCOUNTER_RESET`은 핫리로드용). 입구가 열려 전투 중 걸어 나갈 수 있고, 보스 추적도 네비로 통로까지 따라올 수 있다. 투명벽을 다시 만들지 않았다. 필요하면 "전투 활성 동안만 입구를 막는 encounter 범위 게이트" 같은 최소 보완안을 별도 승인으로 검토.
- `Server/Private/ServerGameplayContractTests_Runner.cpp:735~750`의 베른 단언(배치 36·NPC 30)은 NPC 작업으로 베른이 55배치가 되면서 이미 어긋나 있다. G02 범위 밖이라 미변경.
- 주석만 "30 outer ring"으로 남은 곳: `Server/Public/GameRoom.h:1252`, `Client/Public/WorldDestructionDebrisPresentationRuntime.h:72`. 헤더라 전체 재컴파일을 유발해 손대지 않았다(동작 무관).
- `valtan_tuning_pipeline.py`의 `migrate-preview`·`emit-bootstrap-patch`(일회성 마이그레이션)는 97/135 고정이라 지금 데이터로 돌리면 실패한다. 일반 `validate` 경로는 통과. 보호 파일이라 미변경.
- `-Scope Deploy` 결과 출력의 `PlacementCount 0`은 맵 배치 행 수를 세는 필드라서다(Lights/WorldSequences Scope와 같은 표기).
- 원본 frontwallA/B의 원작 처리 시점은 미확인(계획서대로 대상 아님).

## 내 실수

1. deploy 카탈로그 "stale"을 처음에 내용 차이로 여겼다가 바이트 비교로 줄끝 차이임을 확인했다.
2. 서버 테스트 패치 앵커를 `})) &&`로 잘못 적어 첫 적용이 멈췄다(쓰기 전 전부 검증하는 방식이라 파일 손상 없음).
3. 세션 gotcha인 heredoc 작은따옴표 문제로 테스트 패치 1회 실패 → Write 도구로 스크립트 작성.
4. `printf`로 배치를 만들다 `\2022`·`\v` 이스케이프로 경로가 깨졌다 → 파이썬으로 바이트 작성.
5. 격리 컴파일 로그를 UTF-16으로 받는 걸 몰라 결과 추출을 두 번 다시 했다.
