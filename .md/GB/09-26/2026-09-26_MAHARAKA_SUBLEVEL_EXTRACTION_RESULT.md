# 2026-09-26 마하라카 미추출 서브레벨 12개 배치·자산 추출 결과

설계서 인계서의 후속 작업이다. G02 2차가 "마하라카 서브레벨 14개 중 3개만 추출했다"를 밝힌 뒤,
나머지 12개의 **배치(placement)와 자산 목록만** 추출했다. 모델 cook·설치·게시는 이번 범위가 아니다.

## 1. 실제 바꾼 것

정본 `Data/**`, 게시본 `Client/Bin/DataFiles/**`, `Client/Bin/Resources`, 코드에 **변경 0건**이다.
산출물 전부가 새 staging 폴더 하나에만 있다.

```
out/MaharakaSublevels_20260926_221236/
  run_extraction.py           추출 실행기
  dump_export_classes.py      export class 인벤토리
  analyze_placements.py       자산·가시성·y 분포 분석
  estimate_cook.py            설치본 대비 cook 필요량 측정
  extraction-run.json         12개 실행 기록(exit code·소요·물리 package 일치)
  export-class-inventory.json export 수·class 분포
  placement-analysis.json     자산·가시성·y·섬 규칙 영향
  cook-estimate.json          설치 wmodel 대비 cook 필요량
  Placements/<논리명>/         서브레벨별 placements.json + manifest
  probe_scene03b/             최초 단일 검증 실행
```

기존 `C:/LostArkExtract/LV_OCN_EVENTIS_MHP_20260919`는 읽기만 했고 덮어쓰지 않았다.

## 2. 원본 근거

- 도구: `Tools/LevelPlacementExtractor/extract_ue3_placements.py`, schema3
- `--package-root C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages`
- `--script-root C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC` (source visibility archetype/CDO)
- `--region kr`, UModel `C:/LostArkExtract/umodel/umodel_lostark_v7.exe`

**12개 전부 UModel `-nameresolve`가 논리명을 물리 UPK로 해석했고, 12개 모두 G02 2차가 확정한
매핑과 일치했다**(`extraction-run.json`의 `physicalMatchesExpected` 12/12 true).

## 3. 서브레벨별 실측

| 논리 서브레벨 | export | 배치 | 고유 메시 | source visible | y 범위(cm) | 섬 규칙 제외 |
|---|---|---|---|---|---|---|
| `SCENE02A` | 432 | 0 | - | - | - | 0 |
| `SCENE02B` | 90 | 0 | - | - | - | 0 |
| `SCENE03A` | 112 | 0 | - | - | - | 0 |
| **`SCENE03B`** | 740 | **60** | 5 | 60/60 true | 96,679~99,040 | 0 |
| `SCENE04A` | 616 | 1 | 1 | 1/1 true | 100,726 | 0 |
| `SCENE06A` | 493 | 2 | 2 | 2/2 true | 97,918~99,656 | 0 |
| `SCENE07A` | 1,156 | 29 | 11 | 29/29 true | 96,568~99,995 | 0 |
| `ENVNPC03` | 42 | 0 | - | - | - | 0 |
| `SOUNDSTREAM` | 119 | 0 | - | - | - | 0 |
| `MUSIC` | 32 | 0 | - | - | - | 0 |
| **`STANDARD_TRACK`** | 4,262 | 19 | 4 | 19/19 true | −101~4,219 | **19 (전부)** |
| **`SL02`** | 9,881 | **3,934** | 99 | 3,934/3,934 true | 0~34,656 | **3,934 (전부)** |

합계 export 17,975, 배치 **4,045**. 기존 3레벨의 배치가 4,149이므로 **거의 같은 규모가 더 있다.**

배치 0인 6개는 원본 부재가 아니라 **static mesh가 없는 레벨**이다. export class 분포로 확인했다.

- `SCENE02A`: `efskeletalmeshactor` 48, `animnodesequence` 48 — 스켈레탈 배우 컷신
- `SCENE02B`: `cameraactor` 7, `eflocaltrigger` 6 — 카메라·트리거
- `SCENE03A`: `interpgroup` 12, `cameraactor` 4 — 연출
- `ENVNPC03`: `efskeletalmeshactor` 3 — 환경 NPC
- `SOUNDSTREAM`: `akambientsound` **95** — 오디오 전용, 기하 없음
- `MUSIC`: `efsoundmusicvolume` 6, `model`/`polys` 7 — 오디오 전용, 기하 없음

## 4. 섬 범위 규칙이 무엇을 버리는지

09-19 admission 규칙은 `island scope = source world y > 50000 cm`이고, 규칙 자신의 설명이
`racing track lies near y 0..35000`이다. 근거: `admission.exclusions.json`.

이번 실측이 그 설명과 정확히 맞는다. **`SL02`의 y가 0~34,656, `STANDARD_TRACK`이 −101~4,219**이다.
즉 이 규칙을 그대로 적용하면 **저지대 두 서브레벨의 배치 3,953개가 한 개도 남지 않는다.**
반대로 섬 위 `SCENE03B/04A/06A/07A`는 y 96,568~100,726이라 **한 개도 걸리지 않는다.**

이 문서는 규칙을 적용하지 않았다. 측정만 했다. 규칙 변경은 사용자 결정이다.

## 5. 우선순위 3개 상세

### 5.1 `SCENE03B` — 워터팡 무대

배치 60개, 고유 메시 5종, 전부 `sourceVisibility.visible = true`.

| 개수 | 메시 | 원본 경로 |
|---|---|---|
| 35 | `bfm_planbottom_01` | `bfx_sm_00.bfm_planbottom_01` |
| **9** | **`bg_ocn_etc_floor01a_sm_lnh`** | `bg_ocn_etc_g.mesh.bg_ocn_etc_floor01a_sm_lnh` |
| **9** | **`bg_ocn_etc_floor01b_sm_lnh`** | `bg_ocn_etc_g.mesh.bg_ocn_etc_floor01b_sm_lnh` |
| 6 | `bfm_mossfog_001` | `bfx_sm_00.bfm_mossfog_001` |
| 1 | `fm_e_halfsphere_001` | `fx_sm_00.fm_e_halfsphere_001` |

**floor01a 9 + floor01b 9 = 18조각**으로 G02 2차의 실측(20° 등간격 18분할, 반경 6.160~6.215 m,
중심 `(75.07, −984.31)`, 높이 Y 22.40 동일)과 개수가 정확히 일치한다.

주의: 이 5종에 `lv_ocn_eventis_mhp.mat.*` 마하라카 전용 경로를 쓰는 메시는 **없다**.
전부 공용 패키지(`bg_ocn_etc_g`, `bfx_sm_00`, `fx_sm_00`)다. G02 2차가 보고한 마하라카 전용 MIC
`bg_ocn_etc_floor01a_mi_lnh`는 메시가 아니라 **재질 override**이며, 이번 추출의
`materialOverrides`에 51개 배치가 override를 갖는 것으로 기록됐다. 재질 자체 추출은 이번 범위 밖이다.

### 5.2 `STANDARD_TRACK` — 레이싱 트랙 "후보"였으나 실제는 연출 패키지

export 4,262개인데 **배치는 19개뿐**이다. class 분포가 `interptrackmove` **850**,
`interptrackanimcontrol` 276, `interpgroup` 600, `interptrackanimcontrolblendkey` 230이다.
**즉 Matinee 애니메이션 트랙 묶음이지 트랙 지오메트리가 아니다.**

배치 19개의 메시는 지프라인 2종(`bg_shs_common_zipline01a/b_sm_old` 각 5개), 꽃
(`bg_rhd_foliage_flower07_sm_ksy` 8개), 장식 1개뿐이다.

**레인보우 익스프레스의 실제 지오메트리는 `STANDARD_TRACK`이 아니라 `SL02`에 있다고 보는 것이
현재 데이터에 맞다.** 다만 이것은 y 범위와 프롭 구성에 근거한 판단이며, 원본이 이 구역을
"레인보우 익스프레스"라고 부르는 직접 증거는 아직 못 찾았다. 미해결로 남긴다.

### 5.3 `SL02` — 두 번째 메인 서브레벨

배치 **3,934개**, 고유 메시 **99종**, 전부 visible. export에 `shadowmap2d` **3,307**,
`emitter` 415, `particlesystemcomponent` 415가 있다. 즉 베이크 그림자와 상시 파티클을 가진
실제 맵 구역이다.

상위 메시는 여러 맵에서 끌어온 조립품이다 — `bg_atm_tree_broadleaftree01e` 405,
`lv_rad_redsanddst_pipe02/01` 322+293, `bg_ocn_stone_rock01` 253,
**`lv_ocn_eventis_mhp_pool04_sm` 253**, `bg_tot_movillage_fence02b` 146,
`bg_anh_delphic_bridge06b` 104, `ehm_pipe_a13` 94 등이다.

**마하라카 전용 메시(`lv_ocn_eventis_mhp.mesh.*`)는 12개 서브레벨 중 `SL02`에만 있다.**

| 개수 | 메시 |
|---|---|
| **253** | `lv_ocn_eventis_mhp_pool04_sm` |
| 14 | `lv_ocn_eventis_mhp_tent01_sm` |
| 7 | `lv_ocn_eventis_mhp_tent02_sm` |
| 2 | `lv_ocn_eventis_mhp_pool02_sm` |
| 2 | `lv_ocn_eventis_mhp_pool01_sm` |
| 1 | `lv_ocn_eventis_mhp_water01_sm` |

합계 279개. 전부 y 0~34,656 구간이라 섬 규칙에 전부 걸린다.
기존 추출에서 `pool04_sm`은 `_PS`에 배치 1개만 있었고 그것도 제외됐다. 실제 주 사용처가 여기였다.

## 6. 자산 중복과 cook 필요량

현재 설치본 `Client/Bin/Resources/Map` 전체에 wmodel **3,905개**(고유 stem), 그중 마하라카
`LV_OCN_EVENTIS_MHP` 382개 910 MB, `_LAND` 16개 31 MB다.

설치된 wmodel과 대조한 결과다.

| 서브레벨 | 배치 | 설치 메시 사용 | cook 필요 배치 | cook 필요 메시 종수 |
|---|---|---|---|---|
| `SCENE03B` | 60 | 41 (2종) | 19 | 3 |
| `SCENE04A` | 1 | 0 | 1 | 1 |
| `SCENE06A` | 2 | 2 (2종) | 0 | 0 |
| `SCENE07A` | 29 | 2 (1종) | 27 | 10 |
| `STANDARD_TRACK` | 19 | 0 | 19 | 4 |
| `SL02` | 3,934 | 1,469 (41종) | 2,465 | 57 |
| **합계** | **4,045** | **1,514** | **2,531** | **74 (중복 제거)** |

즉 배치의 **37%가 이미 설치된 메시를 재사용**한다. `pool04_sm`, `pool01_sm`, `tent01_sm`,
`bfm_planbottom_01`은 **이미 cook되어 설치돼 있다** — 빠진 것은 메시가 아니라 배치였다.

새로 cook해야 하는 것은 74종이다. 워터팡 무대는 `bg_ocn_etc_floor01a_sm_lnh`,
`bg_ocn_etc_floor01b_sm_lnh`, `fm_e_halfsphere_001` **3종뿐**이다.

Resources 증가 추정은 **약 176 MB**다. 근거는 현재 마하라카 382 wmodel 910 MB의 평균 2.38 MB에
74종을 곱한 값이며, 이번 74종 다수가 foliage·소품이라 **상한 추정**이다. 실제 cook 전에는 확정할 수 없다.

## 7. 실행한 명령과 결과

| 단계 | 결과 |
|---|---|
| `extract_ue3_placements.py` × 12 (schema3, kr) | **exit 0 × 12**, 물리 package 일치 12/12 |
| 소요 | 최대 `SL02` 23.3초, 나머지 0.3~1.3초 |
| `propertyErrors` / `unresolvedPlacements` | manifest 기준 각 레벨 0 |
| export class 인벤토리 | 12/12 성공 |
| 빌드 | **0회** (C++ 변경 없음, 지시대로 금지 준수) |
| publisher | **0회** (`-Mode Publish` 미실행) |

## 8. 사용자 확인 대상

**없다.** 화면에 나타나는 변경을 하지 않았다. 추출한 배치는 staging에만 있고 MapTool 저작본·게시본·
Resources 어디에도 반영하지 않았다. 지금 게임을 켜도 마하라카는 이전과 동일하다.

## 9. 미해결과 다음 조사 위치

- **레인보우 익스프레스의 원본 이름 근거**가 없다. `SL02`가 그 구역이라는 판단은 y 범위와 프롭
  구성에 기반한 추론이다. 다음 위치는 `_PS` Kismet의 RemoteEvent 이름과 `standard_track`의
  Matinee 그룹 이름이다.
- **모코모코 어트랙션에 해당하는 서브레벨을 특정하지 못했다.** `SCENE04A/06A/07A`는 배치가
  1·2·29개뿐이라 어트랙션 본체로 보기 어렵다. `SL02` 안에 있을 가능성이 높으나 확인하지 않았다.
- `SCENE03B`의 **재질 override 51건**의 실제 MIC를 추출하지 않았다. 무대 색·줄무늬가 여기 있을
  것이다. 다음은 `extract_source_map_material_parameters.py`다.
- `SCENE04A`의 `itr_02453_sk`는 `_sk` 접미이며 스켈레탈 자산일 수 있다. cook 경로가 다를 수 있어
  확인이 필요하다.
- `SL02`의 `shadowmap2d` 3,307개와 `emitter` 415개는 이번에 다루지 않았다. 조명·상시 이펙트
  복원과 직결된다.
- 섬 범위 규칙을 어떻게 바꿀지는 결정하지 않았다. 저지대를 포함하면 Area 경계·navigation·
  카메라 범위가 함께 영향을 받는다.

## 10. 다음 단계 제안

사용자가 보고한 세 가지 기준으로 순서를 제안한다.

**1순위 `SCENE03B` — 워터팡 무대.** cook이 3종·배치 19개뿐이고 나머지 41개는 설치본을 재사용한다.
섬 규칙에 걸리지 않아 admission 규칙을 건드릴 필요가 없다. 가장 작은 변경으로 눈에 보이는 결과가
나온다. 다만 18조각의 **회전 키가 아직 미확보**다(G02 2차: `interptrackmove` 87개의 `PosTrack`/
`EulerTrack`이 tagged property로 직렬화되지 않아 못 읽음). 정지 상태 무대는 세울 수 있고
회전·붕괴·복구는 별도다.

**2순위 `SL02` — 저지대 구역 전체.** 배치 3,934개로 현재 맵 규모를 두 배로 만든다. 워터팡 아레나
물(`pool04_sm` 253개)과 레인보우 익스프레스 후보가 여기 있다. 다만 **섬 범위 규칙을 바꾸지 않으면
한 개도 들어가지 않는다.** cook 57종·2,465배치로 이번에서 가장 크다. 규칙 변경이 Area 경계와
navigation에 주는 영향을 먼저 정해야 한다.

**3순위 `SCENE07A`(29배치·10종)와 `STANDARD_TRACK`(19배치·4종).** 둘 다 작고 독립적이다.
`SCENE07A`는 섬 위라 규칙 문제가 없다.

`SOUNDSTREAM`(akambientsound 95)과 `MUSIC`은 기하가 없으므로 맵 cook 대상이 아니다. 다만
마하라카 환경음·BGM 복원 시 별도 입력으로 쓸 수 있다.

**추출만으로는 아무것도 보이지 않는다.** 각 서브레벨마다 cook → MapTool 저작본 반영 →
Area publisher Validate/Publish/Check → 사용자 화면 확인이 남아 있고, 이 문서는 그 중 첫 단계의
입력만 만들었다.

## 11. 최종 재확인

1. `extraction-run.json`을 다시 읽어 12개 전부 `exitCode 0`, `physicalMatchesExpected true`,
   `summary` 존재를 확인했다.
2. 22:12(내 시작) 이후 수정된 **추적 파일이 0건**임을 mtime 대조로 확인했다. `git status`의
   추적 변경 82개는 전부 내 시작 전부터 있던 다른 작업의 것이다.
3. `Data/**`, `Client/Bin/DataFiles/**`, `Client/Bin/Resources`, `Tools/**`에 쓰지 않았다.
   `Tools/LevelPlacementExtractor/README.md`는 읽기만 했다.
4. 빌드·publisher·게임 실행 **0회**. 잔류 python·umodel 프로세스 **0개**.
5. 배치 0인 6개 서브레벨을 "원본 없음"으로 적지 않고 **export class 분포로 이유를 확인해** 적었다.
6. `STANDARD_TRACK`을 처음에 "레이싱 트랙"으로 가정했으나 export 분포(`interptrackmove` 850,
   배치 19)를 보고 **Matinee 연출 패키지로 정정**했다. 잘못된 초기 가정을 결론으로 쓰지 않았다.
7. 분석 스크립트가 처음 `assetId` 키를 읽어 고유 메시가 전부 1종으로 나왔다. 실제 키가 `asset`임을
   원본 JSON에서 확인해 고친 뒤 재실행했고, 잘못된 중간 수치를 결론에 쓰지 않았다.
8. Resources 증가량을 **추정**이라고 명시했고 실측이라고 쓰지 않았다.
9. 9절 미해결 항목을 완료로 표시하지 않았다. **추출 완료이며 cook·설치·게시·화면 확인은 전부 남아 있다.**
