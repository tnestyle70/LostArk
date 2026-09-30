# 2026-09-30 "증명의 전장"(PvP) 맵 원본 데이터 조사 RESULT

조사 전용이다. 게임 코드, Data, Resources는 수정하거나 추출하지 않았다. 작업 파일은 `C:\Users\USER\.claude\jobs\46aea322\tmp\proving_ground\` 아래에만 있다.

## 0. 결론

**있음.** 설치 클라이언트(`C:\ProgramData\Smilegate\Games\LOSTARK\EFGame`, 패키지 최신 수정 2026-09-28)에 PvP 전용 레벨 패키지가 실제로 들어 있다.

- "증명의 전장"은 내부 이름이 **Colosseum**인 매치메이킹 PvP 콘텐츠다. 문자열 표에서 `증명의 전장` 344건이 `sys.colosseum.*`, `sys.arkpass.mission_colosseum_*`, `EFTable_Colosseum*` 표와 연결된다.
- 맵은 **루테란 성 안에 있는 지형이 아니다.** 신청만 대도시(루테란 성 `10801` 포함 18개 도시)에서 하고, 실제 전투는 별도 인스턴스 레벨(`lv_pvp_*`)에서 열린다. 사용자가 말한 "루테란성의 증명의 전장"은 **콜로세움(zone 30201)**이 루테란 성 미술을 쓰기 때문으로 보인다(아래 근거).
- 레벨은 8개 이름으로 확인된다: `lv_pvp_colosseum`, `lv_pvp_coloh`, `lv_pvp_bfdawn`, `lv_pvp_karlajavil`, `lv_pvp_retown`, `lv_pvp_stern`, `lv_pvp_changc`, `lv_pvp_aos_test_01`(이름상 테스트).
- 표 `EFTable_ZoneBase`에는 이 PvP 존(30201~30207, 30251~30254)의 행이 **없다**. 존 이름 문자열만 남아 있다. 그래서 존 ID와 레벨 이름의 정확한 연결(LevelFileName)은 표로 확정하지 못했다.

## 1. 존과 콘텐츠 (사실)

`EFTable_GameMsg`의 `tip.name.zonebase_*` 문자열:

| 존 ID | 표시 이름 |
|---|---|
| 30201 | 콜로세움 |
| 30202 | 지하 투기장 |
| 30203 | 무투 대회장 |
| 30204 | 새벽의 전장 |
| 30205 | 붉은장막 투기장 |
| 30206 | 갈가마귀 전장 |
| 30207 | LostArk 스타디움 |
| 30251 | 검독수리 전장 |
| 30252 | 청룡의 영역 |
| 30253 | 녹슨 금속의 땅 |
| 30254 | 협력 요새전 |

- `EFTable_Colosseum`(18행)의 PrimaryKey가 30201~30206이고 팀 대기 위치, 라운드 시작 위치, 연출 자세, 관전 카메라 ID를 가진다. 이 위치 ID가 가리키는 표(Spot 계열, `EFTable_SpotPath` 등)는 이번에 풀지 않았다.
- 모드: 일반전, 섬멸전, 대장전(경쟁전), 1:1 투혼전, 협력전, 사용자 대결(`sys.colosseum.*`). 시즌 10개(`EFTable_ColosseumSeasonSchedule`).
- 신청 제한: "증명의 전장은 대도시에서만 신청할 수 있다"(`sys.colosseum.enlist_result_failure_member_invalid_enlist_zone`). `ZoneBase.ColosseumEnlistEnabled`가 켜진 마을은 로아룬, 레온하트, 리겐스 마을, 슈테른, 아리안오브, 창천, **루테란 성(10801)**, 칼라자 마을, 모코코마을, 라니아 마을, 베른 성, 칼리나리, 위대한 성, 니아 마을, 샤, 플레체, 엘네아드, 카다룸이다.
- `ZoneBase.PvPType`은 수련장류(`30663` 넓은 공터)에서만 1이다. PvP 존은 클라이언트 표에서 빠져 있다(서버 전용 정의일 가능성, 추론).

## 2. 레벨 패키지 (사실 + 추론)

방법: 설치 `Packages/*.upk` 중 level export가 있는 9,815개의 이름표를 읽어 `lv_pvp_*` 이름을 찾았다(9,815개 중 오류 0, 428초, 4 worker). 폴더는 난독화 이름이고 논리명과 파일명은 다음처럼 대응한다. 이름표에 그 레벨 이름이 있는 패키지를 나열한 것이라, 여러 맵이 공유하는 패키지가 섞여 있다.

| 레벨 | persistent(_ps) 또는 메인 패키지 | 크기 | export | 이름이 나오는 패키지 수(단독/전체) | 특징 |
|---|---|---|---|---|---|
| `lv_pvp_colosseum` | `312NV1V2RCO3OGGQUA2NVGFT.upk` | 2.5MB | 3,361 | 17 / 21 (단독 합계 117MB) | 정적메시 액터 1,168, 데칼 30, 이미터 20, BSP 7, 스트리밍 always-loaded 4 |
| `lv_pvp_coloh` | `312NV1V2RCO3OB2NVGFT71E9.upk` | 9.1MB | 8,790 | 12 / 12 (173MB) | 액터 2,147, 이미터 488, 미니맵 볼륨 있음 |
| `lv_pvp_bfdawn` | `312NV1V2R5XJY8H2RVGF071E.upk` | 129KB | 106 | 10 / 11 (52MB) | 스트리밍 always-loaded 5, `_sl01`, `_land01`, 경로 차단 볼륨 |
| `lv_pvp_karlajavil` | `201MU0U1QVX82XOX0H21QUF.upk` | 46KB | 75 | 22 / 27 (165MB) | 지형(Landscape) 9 component, 데칼 185 |
| `lv_pvp_retown` | `312NV1V2R9QNO8H2RVGF071E.upk` | 152KB | 229 | 8 / 8 (4.7MB) | `_sl00`, `_sl01` |
| `lv_pvp_stern` | `312NV1V2RGNQ9H2NVGFT71E9.upk` | 1.8MB | 2,101 | 7 / 7 (9.9MB) | 회전 모션 액터 24 |
| `lv_pvp_changc` | `312NV1V2RCBYH4C2RVGF071E.upk` | 7.8MB | 5,957 | 7 / 7 (7.7MB) | 지형 3 component, 인스턴스 메시 84 |
| `lv_pvp_aos_test_01` | 여러 개 | 51MB | 36,042 | 6 / 7 | 지형 133 component, 이름상 테스트 맵 |

- **콜로세움이 루테란 성 미술을 쓴다는 근거(사실):** `lv_pvp_colosseum` persistent 패키지의 이름표에 `lv_lut_lucastle`, `lv_lut_lucastle_lightfunction`, `bg_lut_lucastle_castledeco01_*`, `bg_lut_lucastle_library03_*`, `bg_lut_lucastle_librarydoor01_*`, `lv_ber_berncastle`이 있다. 또 로딩 이미지 이름이 **`PVP_LUTERAN_30201`**이다(아래 4절).
- 서브레벨은 `_scene01a`, `_scene02a`(연출), `_music`, `_sound`, `_sl00/_sl01` 형태다. 콜로세움은 `lv_pvp_colosseum_ps`(persistent)와 `_scene01a/02a`, `_music`, `_sound1`이 확인된다.
- **존 ID와 레벨 이름의 연결은 추론이다:**
  - 콜로세움(30201) ↔ `lv_pvp_colosseum`: 이름과 로딩 이미지(`PVP_LUTERAN_30201`)로 확실히 가깝다.
  - 새벽의 전장(30204) ↔ `lv_pvp_bfdawn`(dawn=새벽): 이름 일치.
  - 청룡의 영역(30252) ↔ `lv_pvp_changc`(창천=청룡): 추론.
  - 녹슨 금속의 땅(30253) ↔ `lv_pvp_stern`(슈테른, 금속): 추론.
  - 붉은장막 투기장(30205) ↔ `LV_SHS_RCArena_D`: 이 프로젝트가 이미 쓰는 훈련장 원본(`LV_SHS_RCARENA_D`)과 같은 이름이고, `ZoneBase` 10311/10318도 `붉은장막 투기장`이다.
  - 지하 투기장(30202), 무투 대회장(30203), 갈가마귀 전장(30206), LostArk 스타디움(30207), 검독수리 전장(30251), 협력 요새전(30254)은 `lv_pvp_coloh`, `lv_pvp_karlajavil`, `lv_pvp_retown` 중 어느 것인지 **확정하지 못했다.**
- 스트리밍 서브레벨의 정확한 구성(always-loaded가 가리키는 패키지 이름)은 export의 property 파싱이 필요해 이번에 확정하지 않았다.

## 3. 우리 프로젝트로 가져오려면 (구현하지 않음)

프로젝트 현황: `Data/Maps/MapCatalog.json`의 Area 12개(베른, 발탄 `LV_LUT_HEARTRB_ED`, 쿠크 `LV_LUT_MIDNIGHTC_ED`, 훈련장, 캐릭터 선택 7개, 마하라카)에 PvP 레벨은 없다. 코드와 문서에도 `lv_pvp_*` 언급이 없다.

필요한 작업:

1. **대상 확정:** 어느 PvP 맵을 가져올지 결정. 콜로세움부터 하는 것이 자연스럽다(루테란 성 미술을 재사용).
2. **패키지 구성 확정:** persistent 패키지의 스트리밍 sublevel 패키지 이름을 property로 읽어 필요한 패키지 목록을 만든다(콜로세움은 21개 패키지에 이름이 나온다).
3. **배치 추출:** `Tools/LevelPlacementExtractor/extract_ue3_placements.py` schema3로 정적메시 배치 추출. 콜로세움 단독 패키지 합계 기준으로 정적메시 액터 약 13,500개, component 약 31,600개(대략치, 공유 패키지 포함 가능성).
4. **모델 cook, 재질, 텍스처:** 기존 절차(`cook_wmodel_geometry_contract.py`, `extract_source_map_material_parameters.py`, `extract_ue3_texture_mips.py`, `build_source_map_materials.py`). 루테란 성 소품을 쿠크 맵과 공유할 수 있는지는 자산 ID 대조가 필요하다.
5. **처리 못 하는 클래스 확인:** 콜로세움 persistent에 **BSP `model`/`polys` 7개**와 데칼 30개, 번역 볼륨(`eftranslucentvolume`) 4개가 있다. 이번 세션 조사에서 베른 바닥에 BSP가 의심된 전례가 있고, 현재 추출기가 BSP와 데칼을 처리하지 않을 수 있다(미확인). 지형이 있는 `karlajavil`, `changc`, `aos`는 `LandscapeExtractor`가 필요하다.
6. **Area/Level 등록:** 새 Area(`MapCatalog`, `Data/Maps/Imported`, `Authoring`), 새 `LEVEL` 또는 기존 Level 셸 재사용(마하라카는 `LEVEL::DEVELOPMENT` 셸을 재사용했다). `AGENTS.md`의 레벨 추가 계약(enum, registry, loader, 프로젝트 등록, Server+Client 진입 검증)을 따른다.
7. **서버:** 새 `WORLD_ID`, navigation(`.navsource` bake), 팀 시작 위치와 라운드 위치. PvP 규칙(매칭, 팀, 라운드, 점수)은 별도 대형 작업이다. `EFTable_Colosseum`의 위치 ID가 가리키는 표를 풀어야 원본 위치를 쓸 수 있다.
8. **연출/사운드:** 원본 `_scene01a/02a` 연출, 음악 서브레벨.

난이도/위험:
- 배치만 가져와 보이게 하는 데는 마하라카 작업(14개 서브레벨 중 3개만 추출된 전례)과 비슷한 규모로 보인다. 콜로세움은 export가 7만 개 안팎이라 베른 성(약 5만 배치) 수준이다.
- 큰 위험: (a) BSP/데칼 미처리, (b) 스트리밍 서브레벨 누락으로 일부 지형이 비는 문제, (c) 존 ID↔레벨 이름이 표로 확정되지 않은 점, (d) 팀 시작 위치와 카메라가 서버 전용 표에 있을 가능성, (e) PvP 전투 규칙 자체는 이 프로젝트에 없다.

## 4. 관련 자산 (있으면 목록만)

- **로딩 이미지:** `EFTable_LoadingImageGroup` 그룹 30201에 `PVP_20220727_1`~`_4` 4행(SelectionFactor 100). 그러나 `ExtRes/Loading/ZONE`(634개) 이름을 복호한 결과 PvP 이미지는 `PVP_LUTERAN_30201`(`ReleasePC/Packages/ExtRes/Loading/ZONE/U0U1Q2TMP8XG1BKZDZ6EZ1D.ipk`) 하나뿐이고 `PVP_20220727_*`는 이 폴더에 없다(다른 위치일 수 있음, 미확인). 이 이미지는 복호하지 않았다.
- **BGM/사운드:** 레벨 이름표에 `lv_pvp_*_music`, `_sound`, `_soundstream` 서브레벨이 있다. Wwise 이벤트 이름은 저장되어 있지 않아(해시) 실제 이벤트/파일은 확인하지 못했다.
- **미니맵:** `coloh`, `bfdawn`, `retown`, `stern`의 persistent에 `EFMinimapVolume`이 있다. `colosseum`, `karlajavil`은 확인되지 않았다.
- **UI:** `sys.colosseum.*` 문구 다수, 시즌/계급 표(`EFTable_ColosseumRank`, `PvPLevelInfo`, `PvPLevelReward`). UI 리소스 이름은 조사하지 않았다.

## 5. 확인하지 못한 것

- PvP 존 ID와 `LevelFileName`의 공식 연결(표에 없음). 2026 클라이언트에서 콜로세움 대기실, 라운드 위치의 실제 좌표.
- 각 맵의 정확한 스트리밍 서브레벨 목록과 패키지별 액터 수(위 수치는 이름표 기준 근사).
- 콜로세움 데칼/BSP가 시각적으로 필수인지.
- Spot 계열 표(`EFTable_SpotPath` 등)의 내용과 `EFTable_Colosseum` 위치 ID의 대응.
- `PVP_20220727_*` 로딩 이미지의 위치.
- "루테란성에 입구가 있다"는 사용자 설명의 정확한 게임 내 위치(NPC/포탈). 클라이언트 표에서는 UI 신청 방식이고 NPC/포탈 정의는 찾지 못했다.

## 6. 작업 파일

`C:\Users\USER\.claude\jobs\46aea322\tmp\proving_ground\`: `data2_list.txt`(data2.lpk 목록), `tables/`(복호한 EFTable 35개), `scan_colosseum.py/json`(9,815개 레벨 패키지 이름 스캔), `inspect_ps.py/json`(persistent 패키지 구성), `zone_loading_names.txt`(ZONE 로딩 이미지 이름 복호 634건).
