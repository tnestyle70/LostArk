# 2026-09-18 설명서 2 통합 수정 — G05(베른 네비·바인딩)·G06(NPC 19명 인수) RESULT

입력 계획: `C:/Users/USER/.codex/worktrees/7395/LostArk/.md/GB/09-18/2026-09-18_HANDOFF2_INTEGRATED_REPAIR_PLAN.md` G05·G06.
작업 저장소: `C:/Users/USER/source/졸업팀폴/LostArk`. commit/push 없음, Client 실행 없음, 제품 빌드 없음.

## G05 — Bern·Character Select 바인딩과 Bern 네비

### 기존 반영 (읽기로 존재 확인, 재구현 안 함)

- `MapTool_Area.cpp` `Runtime_AuthoringTargets` 의 `LEVEL::VALTAN_ARENA`(477)·`BERN`(496)·`CHARACTER_SELECT`(510) 분기, `Can_ReplaceRuntimeAuthoringTargets`(605-607), 레벨 전환 시 `m_EditorSublevelJumps.clear()`(1378, 1508).
- `Loader.cpp` TriggerBox 등록 레벨에 `CHARACTER_SELECT`(479-483 부근).
- `MapTool_Navigation.cpp` runtime-authoring Load(242)·Save(644)·계약 해석(725), `MapTool_NavigationPanels.cpp`(33) — 모두 `Is_Active() || m_bRuntimeAuthoring`.
- `Level_Bern.h`(79-80, 197), `Level_CharacterSelect.h`(239-240, 299) `Get_MapAuthoringRuntime/Deploy` 와 `m_MapAuthoringDeploy`.

### Bern 새 네비 상태 (13:30 실측)

- region `LV_BER_BERNCASTLE.Bern`: 570x560, 0.5m, X[15,300) Z[-245,35) Y 33~56. `navsource`·`navregions` 12:42. base 는 옛 통로 50x347.
- `Bern.navpaint` 는 13:31 까지 디스크에 없었다. 사용자는 칠하고 저장했다고 했다.

### 사용자 Save 가 파일로 남지 않은 이유 (코드·파일 시각 판정)

- `Client.exe` 는 11:48:10 빌드였고, runtime-attach Save 허용 수정(`MapTool_Navigation.cpp`, mtime 12:11:28)은 13:13:34 빌드에 처음 들어갔다(`MapTool_Navigation.obj` 13:13:31).
- 옛 코드는 runtime attach 에서 `Save_Navigation` 을 `Navigation save is only available in the Map Editor workspace` 로 거부한다(git diff 의 삭제 줄). Apply Bake 는 `CNavGridBaker::Save_Source` 로 navsource 를 직접 쓰므로 12:42 navsource·navregions 만 남은 것과 일치한다.
- 현재 코드: region 선택 시 Load(281-283)와 Save(655-667) 모두 `CMapNavigationContract::Resolve_Region` → `Data/Navigation/LV_BER_BERNCASTLE.Bern.navpaint` 로 같은 경로를 쓴다. 정적 판독으로는 결함 없음. [미확인] 13:13 이후 새 빌드로 Save 했는데도 없었다면 다른 원인이며, 그때는 화면 상태 문구가 필요하다.

### 스폰 바닥 누락 원인 (측정)

- `player_1`(245,445)·`player_2`(242,441): 셀 중심을 덮는 삼각형이 없다(`TryProjectHeight` 는 가장자리 포함, epsilon 1e-5). 구멍 12칸이 원형 바닥 장식 `BG_BER_BERNCASTLE_FLOOR06_SM`(x 스케일 −0.47 반전)·`FLOOR07_SM`(yaw 180) 중심 (136.92, −23.02) 반경 0.29~1.4m 안에 몰려 있다. 반전 스케일은 원인이 아니다(`NavGridBaker.cpp:405-406` 법선 y 에 절댓값). 이 원형 메시 영역의 틈/형상 때문으로 판정하며, 메시 삼각형 자체는 디코드하지 않았다([추론]).
- `player_3`(248,439): 바닥은 42.616 로 잡혔으나 경사 불가. 0.07m 옆에 `BG_ATT_FOLIAGE_TOTGRASS03_SM`(y 42.26) 이 있다. bake 수집(`MapTool_NavigationBake.cpp:74-83`)은 `LV_NAVIMESH` 그룹과 `CUL_BOX` 만 빼고 풀·덩굴까지 넣고, 칸마다 가장 높은 면을 고른다(`NavGridBaker.cpp:491-496`). 풀잎 윗면이 바닥 대신 잡힌 것이다.

### 이번 수정 — `Data/Navigation/LV_BER_BERNCASTLE.Bern.navpaint` (신규, 13:34)

메인 지시로 스폰 3칸만 칠했다. 쓰기 직전 파일 부재와 navsource(1789702966)/navregions(1789702968) mtime 불변을 재확인하고 `.tmp` → `os.replace` 로 교체했다. 서식은 Map Tool writer(`NavGridPaintDocument.cpp:821-860`, LF, v3)와 같다.

```
LOSTARK_NAVGRID_PAINT 3 "LV_BER_BERNCASTLE.Bern" 570 560 0.5 15 -245 3
248 439 WALKABLE 42.3034935
242 441 WALKABLE 42.2554836
245 445 WALKABLE 42.2554817
```

| 스폰 | 근거 | 스폰 y 와 차 |
|---|---|---|
| player_3 | walkable 이웃 4칸 중앙값(42.289~42.33). 풀 끝 42.616 대신 바닥 | 0.205 (게시 허용 0.25) |
| player_2 | walkable 이웃 4칸 중앙값. 이웃 4칸뿐이라 seam 자동 규칙(5칸 이상) 불가 → 높이 명시 | 0.045 |
| player_1 | walkable 이웃 7칸 중앙값 | 0.006 |

연결용 추가 칸은 필요 없었다. base paint 5,175칸은 건드리지 않았다.

### 자동 검증

- `Publish-ServerNavigation.ps1 -Mode Validate -AreaId LV_BER_BERNCASTLE` EXIT 0 (13:39). base 50x347 walkable 7674 maxStep 17.03, region 570x560 walkable 80114 maxStep 22.12. maxStep 은 측정 최대 인접 높이차이고 런타임 정책은 1.0m 다. 정책은 바꾸지 않았다.
- 메인이 13:44:25 에 Publish(EXIT 0). 아래 연결성은 게시본 `Server/Bin/DataFiles/Navigation/LV_BER_BERNCASTLE.Bern.navgrid` 바이트로 다시 계산했다(walkable 80114 일치).
- Character Select Validate 는 메인 지시(동시 실행 금지)에 따라 이번에 다시 돌리지 않았다. 앞선 결과: 62x62, walkable 2368.

### 연결성 (서버 규칙 재현: 8방향, 두 칸 walkable, 높이차 ≤ 1.0m, 대각선은 양옆 칸도 통과 가능 — `ServerNavigation.cpp` `Find_Path`·`Is_CellTraversalAllowed`)

- 성분 4,766개. 스폰 성분 23,582칸.
- 도달(2m 이내 스폰 성분 칸): 스폰 4, 발탄 triggerBox 1, 팀 NPC 30, 새 NPC 3(그레텔·로넬리·집배원 레노엘).
- 미도달 새 NPC 15명:

| NPC | 스폰 성분까지 직선 최단 | 소속 |
|---|---|---|
| 아카테냥·스트라벨·에단·마레인 | 2.7 / 3.8 / 4.5 / 5.1m | 자기 칸 비walkable 또는 남쪽 성분(2,235칸) |
| 윈리 | 9.5m | 자기 칸 비walkable |
| 바스커빌·바빌루·옥타니아 | 17.0 / 21.4 / 21.9m | 서쪽(1,429칸 등) |
| 타르코 | 30.3m | 작은 성분(16칸) |
| 마놀린·스이에·케실리·젠킨스·벨몬드·카흐로스 | 34.7~61.4m | 동쪽 성분(7,453칸) |

- 스텔리아(z 275.647)는 region 밖이다. 이 region 의 검증 범위 밖.
- 끊김 지점(측정): 동쪽은 (167.8, −25.2)·(170.8, −24.8)에서 이웃 칸 높이차 2.59·2.55m, 서쪽은 (56.8, −139.8) 4.3m, 남쪽은 대각선 모서리만 맞닿고 양옆이 높이 48.07~48.1 경사 불가 칸(바닥 47.63).
- 끊김 지점 4m 안에는 기둥(`RANIAT_PILLAR03`, `BERNCASTLE_PILLAR01/11`), 아치형 꽃문(`CHRISTMAS_FLOWERGATE02B`), 벽·창틀, 나무·풀이 몰려 있다. 게시 스크립트 주석(`Publish-ServerNavigation.ps1:1014-1017`)도 "Bern 단일 높이 bake 는 다리·테라스·아치를 바닥 통로 위로 본다. v3 paint 가 통로 능선 세 곳을 바닥 높이로 고쳤다"고 적는다. 새 region 에는 그 보정이 없다. [추론] 통로 위 구조물·식생 윗면이 바닥 대신 잡혀 1m 넘는 턱이 생겼다.
- 지시대로 이 끊김은 칠하지 않았다. 마을 전체 연결은 완료가 아니다.

### region 정책

region 안에서 시작한 질의는 region 밖을 보지 않는다(`Select_Region`). base 통로(x 124~149, z −176~−2)는 region 안에 완전히 들어가 영향이 없다. 스텔리아처럼 region 밖을 걸어서 잇는 도시 전체 연속 이동은 현재 불가. base 확장은 적용하지 않았다(보존안: 사용자 region paint·navsource 는 그대로 두고 base 를 넓히려면 옛 base paint 5,175칸을 새 격자로 좌표 변환해 옮기는 작업이 먼저 필요).

### 문서 교정

`.md/TEAM/AREA_DATA_LAYER_GUIDE.md` 두 문장(CRLF 유지, diff-check 통과):
- "publisher 는 player spawn/trigger 연결성을 검증한다" → 실제 검사(enabled playerSpawn/boss 칸 walkable·높이 0.25m, `-RequireSingleComponent` 선언 Area 만 단일 성분)로 교정, trigger·NPC 연결성은 검사하지 않음을 명시.
- "Client 제품 아레나는 열람용이며 편집은 Lobby → Test" → 네 Debug 제품 Level 의 런타임 연결 Map Tool 도 `Data/Navigation` 저작 파일을 로드·저장하고 live Navigation 은 바꾸지 않는다로 교정.

### 사용자 확인 대기

- 베른 재입장 후 스폰 4곳에서 이동, 발탄 트리거까지 이동, 그레텔·로넬리·레노엘 접근.
- Map Tool 로 Bern 네비를 다시 저장할 때는 Map Tool 을 새로 열어 디스크의 새 paint 를 읽은 뒤에만 Save.
- Character Select 외형 변화(기둥 350·석상·재질 6)는 판정 대기. 원복·Area Publish 안 함.

### 미지원·추가 승인

- 동·서·남 15명 접근을 위한 끊김 보정(통로 위 구조물 높이 교정 paint 또는 bake 대상 필터 변경)은 범위·방법 결정이 필요하다.
- 스텔리아(북쪽 별도 덩어리) 접근은 region 확장 또는 별도 region 필요.

## G06 — NPC 19명 인수 검증 (재쿠킹 없음)

### 연결 대조 (배치 → NpcCatalog → modelAssetId → LookInfo → 실제 쿠킹 입력)

- 19명 모두: archetype 이 카탈로그에 있고 `runtimeStatus=supported`, modelAssetId 패키지 = LookInfo body 패키지, 머리 파츠(B_multipart 17)는 `bern_cooked_head` 병합본, 일체형 2(MN_CNGG_00·MN_PETAT_00)는 `bern_cooked_cm`.
- **AnimSet 불일치 3명**: LookInfo 지정 세트와 다른 PSA 로 구웠다. 필요한 PSA 는 `out/BernNpcSource20260917/rigs/` 에 있다.
  - 스이에 MN_CNBF_00: LookInfo `HM_FE02_G011_00A_Ani` / 쿠킹 `hm_fe02_0000_00a_nor_ani`
  - 마법학자 에단 MN_CNDM_00: LookInfo `HM_MA02_0000_00C_Nor_Ani`(+`_Com_Ani`) / 쿠킹 `hm_ma02_0000_00a_nor_ani`
  - 스텔리아 NP_SJWD_00: LookInfo `HM_FE03_G010_00A_Ani` / 쿠킹 `hm_fe03_0000_00a_nor_ani`
- LookInfo 무기 파츠 미부착 5명: 스이에 1, 에단 1, 로넬리 1, 젠킨스 1, 스텔리아 2. bodyMaterials override 미적용 4명: 바빌루 2, 아카테냥 1, 벨몬드 1, 젠킨스 1.

### 설치 wmodel 측정 (백업 `tmp/npc_head_fix_backup` 과 비교)

- 17개: 서브메시 1→2, 정점 증가(예 MN_CNCM_00 31,212→50,873), 메시 최대축 7~23% 증가. 골격 span 은 병합 전후 동일(12,747~15,978 → ×0.0001 = 1.27~1.60m), 팀 NPC 14,305~15,978 과 같은 범위 → preScale·cm 중복 적용 없음.
- **정점 수**: 팀 NPC 4,177~5,853 대비 이번 19명 15,480~73,483(병합본 15,480~58,468, MN_CNGG_00 73,483). 팀 NPC 의 3~12배다. 베른 성능 부담과 관련된 사실로 남긴다.
- 목 이음새: NP_LRJP_00 평균 최근접 1.850→1.453cm, MN_CNCN_00 기준 자세 보정 0.176cm 는 머리 누락 RESULT 인용이며 이번에 재측정하지 않았다.
- `_cm`/`_m` 마스크 미연결은 색조 차이로 분리한다(재질 시스템 개편 안 함).

### 이번 수정

- 재현 스크립트 보존: `out/BernNpcSource20260917/scripts/` 에 46개 복사 + `README.txt`(Git 무시 대상 확인). 추출(resolve_lookinfo·parse_npc_v2·dump_npc_records·verify_npc_ids·bern_npc_full_list·bern_npc_report·bern_export_run), 쿠킹(bern_cook_run·cook_targets·cook_targets2·cook_textured·make_cook_cm·make_cook_cm_worker·run_cm_workers.sh·cook_cm·head_pairs·psk_head_merge·cook_head), 설치·검증(bern_install·install_cm·fix_install_lookup·install_verify_head·wmodel_bounds·bounds_any·bern_geom_check), 배포·배치(update_distribution·restore_bern19_source), 데이터(func_npcs·func_with_sym·sel19·installed_clips·bern19_newpos·npccatalog_before19·bern_world_before19), 로그(cm_a/b/c.out, bern_cooked_head_a/b/c.out, cook_batch1/2.txt, boss_yaw.txt, diag_lrjp.txt), 보조(boss_yaw_check·bone_diff). 대용량 staging·원본은 복사하지 않았다.
- `Copy_ResourceDistribution_2026-09-18.ps1`: 목록은 그대로, 복사부를 전체 사전검사 → 형제 폴더 staging → 개수·크기 확인 → 기존 파일 백업 후 교체 → 설치 크기 재확인 → 실패 시 복구·신규 파일 제거 → staging/backup 정리로 바꿨다. CRLF, BOM 없음, 실행 메시지는 ASCII. 이전본 `tmp/Copy_ResourceDistribution_2026-09-18.ps1.before-harden`.

### 자동 검증

- ps1 구문 오류 0. 임시 폴더 시험: 새 설치 `copied 147 of 147 files (0 replaced, 147 new)`, 재실행 `147 replaced`, 원본 1개 삭제 시 `1 of 147 source files are missing; nothing was copied` 로 중단·대상 0개·임시 폴더 잔존 0.

### 태준 미니맵 NPC 아이콘

- 로컬 모든 ref(426 브랜치, 마지막 fetch 09-17 10:44)에서 미니맵 NPC 마커 코드 흔적이 없다(`-S` 로 `Npcs;`·`MINIMAP_NPC`·`MapSymbol`·`NpcMarker`·`Collect_MinimapNpc`, `-G npc` on `Data/UI/Minimap`·`MinimapView.*`).
- 태준 미니맵은 `c65139d3`(09-05, `origin/feature/minimap`, PR #320 병합) 로 플레이어·파티·보스만 표시한다. 09-05 이후 태준 커밋 20여 개도 UI(이름표·탈것·MVP·로딩) 이며 미니맵 NPC 가 아니다.
- 판정: 담당 브랜치 반영 문제(아직 push 전이거나 09-17 10:44 이후 push). fetch·pull·merge 는 하지 않았다. 19명 아이콘 매핑 계약 비교는 코드가 없어 수행 불가.

### 사용자 확인 대기

- 얼굴이 목에 맞게 붙었는지, 머리카락·피부 텍스처, idle 중 머리 동작. 스이에·에단·스텔리아의 대기 동작이 원작과 다른지.

### 미지원·추가 승인

- AnimSet 불일치 3명과 무기·재질 override 는 재쿠킹이 필요해 이번 범위(재쿠킹 금지) 밖이다.

## 내 실수

1. 연결성 첫 출력에서 도달한 NPC 행을 인쇄하지 않아, 새 NPC 18명 전원이 미도달인 것처럼 한 번 요약했다. NPC 별 거리 표로 다시 계산해 그레텔·로넬리(0.2·0.3m)가 도달임을 바로잡았다.
2. 0-1 BFS 끊김 탐색에서 대각선 모서리 규칙을 넣지 않아 남쪽을 비용 0으로 냈다. 맞닿은 칸 분석으로 모서리 규칙 차단임을 확인했다.
3. `wmodel_bounds.skel_chain_height` 반환이 dict 인 것을 확인하지 않고 round 를 적용해 한 번 실패했다.
