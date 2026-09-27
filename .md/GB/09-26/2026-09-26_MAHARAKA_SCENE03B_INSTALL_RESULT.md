# 2026-09-26 마하라카 SCENE03B 워터팡 무대 설치 결과

지시받은 범위는 `SCENE03B` 60배치를 마하라카 Area에 설치하고 publisher Validate → Publish → Check까지
수행하는 것이었다. **설치는 Resources 메시 2종까지 완료했고 배치·재질 연결과 게시는 하지 않았다.**
지시의 전제 두 개가 실제 데이터와 달랐고, 작업 중 Resources 데이터 손실 사고가 한 건 발생해
복구했다. 아래에 근거와 함께 전부 기록한다.

## 1. 지시 전제의 정정 두 건

### 1.1 "바닥 메시는 이미 Resources에 설치돼 있다" — 사실이 아니다

지시는 `Client/Bin/Resources/Map` 아래 `floor01a` 105개 / `floor01b` 53개 파일을 근거로
"다시 cook하지 마라"고 했다. 그 105개의 실제 출처를 세어 보면 다른 맵들이다.

| 출처 폴더 | 파일 수 |
|---|---|
| `LV_BER_BERNCASTLE` | 48 |
| `LV_LUT_HEARTRB_ED` | 27 |
| `TRAININGMAP` | 8 |
| `LV_OCN_EVENTIS_MHP` | 4 |
| `ENVIRONMENT_CURATED` | 4 |
| `BG_RAD_VALTAN_A` | 4 |
| 기타 lobby/charselect | 10 |

`BG_RAD_VALTAN_FLOOR01A_SM`, `bg_elg_aryanorb_cubefloor01a_*`, `ENV_..._LUTOMB_FLOOR01A_SM_KHK`처럼
이름에 `floor01a`가 들어갈 뿐인 다른 메시다. 실제 무대 메시 이름으로 좁히면 결과가 0이다.

```
find Client/Bin/Resources/Map -iname "*floor01a*lnh*"  -> 0개
find Client/Bin/Resources/Map -iname "*floor01b*lnh*"  -> 0개
grep -i "floor01a_sm_lnh" ...LV_OCN_EVENTIS_MHP.mapassets -> 0행
```

즉 `bg_ocn_etc_floor01a_sm_lnh` / `floor01b_sm_lnh`는 **Resources에도 Area catalog에도 없었다.**
substring 오탐이었다. 따라서 cook이 필요했고 실제로 cook했다.

### 1.2 "60배치가 전부 무대 구성물" — 42개는 컷신 후처리 판이다

staging 60배치의 자산 구성은 다음과 같다.

| 자산 | 수 | class | 재질 override |
|---|---|---|---|
| `bfm_planbottom_01` | 35 | interpactor | `scene_a.mat.white_t2` |
| `bg_ocn_etc_floor01a_sm_lnh` | 9 | interpactor | `lv_ocn_eventis_mhp.mat.bg_ocn_etc_floor01a_mi_lnh` |
| `bg_ocn_etc_floor01b_sm_lnh` | 9 | interpactor | 없음 |
| `bfm_mossfog_001` | 6 | interpactor | `fx_d_po_rain_01/02`, `fx_c_pa_filmnoise_01_tr`, `fx_c_po_lensvignett_01_tr` |
| `fm_e_halfsphere_001` | 1 | interpactor | `scene_a.mat.white_t2` |

좌표와 스케일을 재면 두 집단이 분명히 갈린다.

| 집단 | x 범위 | 높이 | z 범위 | scale3D |
|---|---|---|---|---|
| 바닥 18조각 | 68.85~81.24 | **22.40 전부 동일** | -990.40~-978.24 | (0.95, 0.95, 1.188) 단일값 |
| 나머지 42개 | 44.14~50.69 | 26.89~30.12 | -967.83~-966.79 | 0.003~0.2, 9종 |

42개는 한 지점(x≈50.6, z≈-967)에 겹쳐 쌓여 있고 스케일이 0.3~20%이며 재질이 전부
화면 후처리(흰 판, 비, 필름 노이즈, 렌즈 비네트)다. `bfx_sm_00` / `fx_sm_00` / `fx_post`는
컷신 FX 패키지다. **이것을 정적 world 배치로 넣으면 맵 한가운데 높이 27 m에 흰 판과 비 판이
박힌다.** 복원이 아니라 회귀이므로 제외했고, 제외 근거를 `Placements_Stage` 문서의
`stageFilter` 블록에 남겼다.

따라서 설치 대상은 **18조각**이며, 그중 재질 override를 가진 것은 `floor01a` 9개다.

## 2. 실제 구현

### 2.1 cook 파이프라인 (committed 도구 그대로)

`build_map_material_variants.py`의 기존 6단계를 SCENE03B 18배치로 범위를 좁혀 실행했다.

```
inventory  --placements-dir Placements_Stage --level-prefix LV_OCN_EVENTIS_MHP_
           --expect-packages 1 --expect-source-meshes 2 --expect-variants 2
           --expect-placements 18 --expect-override-placements 9
extract    --umodel umodel_lostark_v7.exe --region kr
hydrate-catalog --source-root SourceRaw/source
cook       --converter Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe
install    --resources-root Client/Bin/Resources --allow-partial-material-preview
```

결과 수치다.

| 단계 | 결과 |
|---|---|
| inventory | sourceMeshes 2, variants 2, placements 18, overridePlacements 9, sourceHidden 0 |
| extract | 2/2 exported |
| hydrate-catalog | materials 5, textures 7, meshDefaults 2, **gapCount 0** |
| cook | 2/2, `textureSlotsComplete: true` ×2 |
| install | 신규 자산 2종 14파일 |

생성된 자산 ID는 `MAP_167F91F6940F_BG_OCN_ETC_FLOOR01A_SM_LNH_OVR_0636FCF08A2E`(MIC override 반영)와
`MAP_AF1951C8B827_BG_OCN_ETC_FLOOR01B_SM_LNH`(base)다. 마하라카 전용 MIC
`lv_ocn_eventis_mhp.mat.bg_ocn_etc_floor01a_mi_lnh`는 `MAT_A3A3C4CF66C14929`로 추출돼
override variant에 반영됐다.

### 2.2 공용 도구 수정 1건 — 검토 필요

`hydrate-catalog`가 텍스처 4종에서 실패했다.

```
ERROR: invalid physical texture package: Packages/U1E2KRUD6GE2RUDM276RZDS7S8YU.upk
```

원인은 `build_map_material_variants.py:1403`의 가드다. `physicalPackage`가 **평이름 1개**여야
한다고 요구하는데, 엔진 공용 텍스처(`spec`, `flat_gray`, `ambientreflection_02`,
`t_tds_specular04`)는 `Packages/<name>.upk`로 기록된다. 두 파일 모두 실제로
`ReleasePC/Packages/` 아래에 존재한다.

바로 위 `safe_relative_path()`가 이미 절대경로·드라이브·`..`를 거부하므로 경로 세그먼트를
하나 더 허용해도 `package_root` 밖으로 나가지 않는다. 그래서 최소 수정했다.

```python
-    if len(package_relative.parts) != 1 or package_relative.suffix.casefold() != ".upk":
+    # Engine-common texture packages are recorded as "Packages/<name>.upk"; safe_relative_path
+    # already rejects absolute paths, drives and ".." so one nested folder stays in package_root.
+    if len(package_relative.parts) not in (1, 2) or package_relative.suffix.casefold() != ".upk":
```

+203 bytes, ASCII·LF 유지, 주석 영문. 기존 회귀
`test_build_map_material_variants.py` **15 tests OK**(수정 전후 동일)를 확인했다. 수정 후
텍스처 4종이 전부 exported로 바뀌었다. **이 수정은 쿠크 등 다른 Area의 같은 단계에도 영향을
주므로 팀 검토가 필요하다.** 동작은 넓히기만 하며 기존에 통과하던 입력을 막지 않는다.

### 2.3 native converter의 한글 경로 한계

`ModelAssetConverter.exe`(assimp)가 저장소 경로의 `졸업팀폴`을 `\ufffd\ufffd...`로 깨뜨려
cook이 실패했다.

```
assimp failed: Unable to open file "C:\Users\USER\source\\ufffd\ufffd...\LostArk\out\...\pack\bg_ocn_etc_floor01a_sm_lnh.gltf"
```

도구를 고치는 대신 cook만 ASCII 경로 `C:/LostArkExtract/Scene03bCook_20260926`에서 수행했다.
2026-09-19 원본 cook도 `C:/LostArkExtract/` 아래에서 돌았으므로 같은 관행이다. 산출물 manifest는
저장소 `out/.../Scene03bInstall/`로 복사해 기록을 남겼다.

## 3. live 설치

`Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP/` 아래 신규 자산 2종, 파일 14개. Drive 전달 목록이다.

```
Map/LV_OCN_EVENTIS_MHP/MAP_167F91F6940F_BG_OCN_ETC_FLOOR01A_SM_LNH_OVR_0636FCF08A2E/
    MAP_167F91F6940F_BG_OCN_ETC_FLOOR01A_SM_LNH_OVR_0636FCF08A2E.wmodel
    textures/19e8ec0bb6e9_flat_gray.png
    textures/4388c5eb477a_ambientreflection_13.dds
    textures/9934688136d4_diffuse.dds
    textures/9934688136d4_spec.dds
    textures/a33a13c31444_bg_ocn_etc_floor01a_n_lnh.png
    textures/b2378a8f80d6_t_tds_specular04.dds
Map/LV_OCN_EVENTIS_MHP/MAP_AF1951C8B827_BG_OCN_ETC_FLOOR01B_SM_LNH/
    (같은 구성 7파일)
```

`admissionState`는 `geometry-preview-partial-material`이다. 이는 새로운 타협이 아니라
**이 Area 기존 382자산 전부와 같은 상태**다. build receipt의 `runtimeMaterialAdmission` 382개가
모두 `mode: geometry-preview-partial-material`, `originalVisualFidelityVerified: false`다.
`--allow-partial-material-preview`는 geometry 확인용이며 원본 시각 동일성 인증이 아니다.

## 4. Resources 데이터 손실 사고와 복구

### 4.1 무슨 일이 있었나

`install` 단계는 `Resources/Map/<AreaId>` 폴더를 **자기가 소유한 것으로 간주하고 넘겨받은
manifest 내용과 정확히 일치하도록 동기화**한다. 자산 2종짜리 manifest로 실행한 결과
**기존 382개 자산 디렉터리가 전부 삭제되고 2개만 남았다.**

```
설치 전: 382 디렉터리
설치 후: 2 디렉터리 + .lostark-area-install.receipt.json (assetCount: 2)
```

이는 "기존 팀장 파일을 삭제·수정·덮어쓰지 마라"는 경계를 위반한 것이다. 내가 실행한 명령이
원인이며, 도구의 동작을 사전에 확인하지 않은 내 잘못이다.

### 4.2 복구

cook 원본이 2026-09-19 recook staging에 남아 있었다.

```
C:/LostArkExtract/MHPr2/runtime   382개 (09-19 04:45, 최종 recook)
C:/LostArkExtract/MHPrt/runtime   382개 (09-19 01:20)
```

게시본 `.mapassets`가 참조하는 고유 자산이 정확히 382개이고 `MHPr2/runtime`이 그 382개를
전부 보유함을 먼저 확인한 뒤, 삭제된 것만 복사했다(신규 2종은 건드리지 않음).

복구 검증이다.

| 항목 | 결과 |
|---|---|
| 복원 디렉터리 수 | 382 |
| 최종 디렉터리 수 | **384** (기존 382 + 신규 2) |
| 게시 catalog 참조 382종 전부 존재 | 예 |
| `.wmodel` 누락 디렉터리 | 0 |
| 표본 12자산 212파일 SHA-256 대조 | **불일치 0** |
| Area 폴더 총 파일 | 6,323 |

`install`이 만든 `.lostark-area-install.receipt.json`은 `assetCount: 2`로 현재 상태를 잘못
기술하므로 제거했다(백업 보관). 베른·발탄·마하라카 LAND 폴더에는 이 영수증이 없고 쿠크에만
있으므로, 마하라카의 원래 상태는 영수증 없음이 맞다.

### 4.3 남는 위험

복구본은 09-19 staging의 cook 산출물이다. 삭제 직전 디스크에 있던 파일과 **바이트 동일하다고
단정하지 않는다.** 표본 12자산은 일치했고 참조 382종이 모두 있으며 wmodel 누락이 없다는 것까지
확인했다. 그 사이에 누가 수동으로 교체한 파일이 있었다면 그 변경은 되살리지 못한다.
사용자가 마하라카 화면에서 이전과 다른 점이 보이면 이 사고를 먼저 의심해야 한다.

## 5. 게시 — 하지 않았다

배치 행·catalog 행·`mapmaterials` override 추가와 publisher Validate/Publish/Check를
**수행하지 않았다.** 이유는 다음과 같다.

`Data/Maps/Imported/*.mapassets`와 `*.mapplacements`는 `build_maptool_scene.py`가 생성하는
문서다. 손으로 행을 덧붙이는 것은 생성물 수동 편집이고, 정규 경로는 SCENE03B를
`Placements_Admitted`에 넣고 scene builder를 다시 돌리는 것이다. 그런데 그 경로는 Area catalog와
배치를 통째로 재생성하며, `--materials-output`이 손으로 작성한 329행 `mapmaterials.json`(물 7행
포함)을 덮을 수 있다. 지시의 "329 mapmaterials 행과 사용자 override를 하나도 잃지 마라"와
정면으로 충돌한다.

4절의 손실 사고를 낸 직후에 Area 전체 재생성을 강행하는 것은 위험 대비 이득이 맞지 않다고
판단해 여기서 멈췄다. 다음 작업자가 결정할 사항이다.

검증한 상태: 저작·게시 파일 7개가 내 작업 시작 시점 백업과 **SHA-256 전부 동일**이다.

| 파일 | 대조 |
|---|---|
| `Imported/*.mapassets` | 동일 |
| `Imported/*.mapplacements` | 동일 |
| `Imported/*.build.receipt.json` | 동일 |
| `Authoring/*.mapplacements` | 동일 |
| `Authoring/*.mapmaterials.json` | 동일 |
| `DataFiles/Map/*.mapassets` | 동일 |
| `DataFiles/Map/*.mapplacements` | 동일 |

`git status`에 보이는 이 파일들의 `M` 표시는 내 작업 이전부터 있던 다른 작업의 변경이다.

## 6. 빌드/자동검사

빌드 0회(데이터 작업이며 C++ 변경 없음). publisher 0회.
`test_build_map_material_variants.py` 15 tests OK(도구 수정 전후 동일).

## 7. 무대 좌표 검증

필터한 18배치를 `project = (src.x/100, src.z/100, -src.y/100)`로 변환한 실측이다.

| 항목 | 실측 | 기대 | 판정 |
|---|---|---|---|
| 중심 | (75.064, -984.326) | (75.07, -984.31) | 오차 **0.017 m** |
| 반경 | 6.160 ~ 6.209 m | 6.160 ~ 6.215 m | 일치 |
| 높이 | 22.4 단일값 | 22.40 | 일치 |
| 인접 각 간격 | 19.65 ~ 20.46° | 20° 등간격 | 일치 |
| scale3D | (0.95, 0.95, 1.1875) 단일값 | — | 변환 1회만 적용 |

## 8. 사용자 수동확인

**현재 게임을 켜면 무대는 보이지 않는다.** Resources에 메시만 들어갔고 배치·catalog·게시가
없기 때문이다. 화면에서 확인할 것이 아직 없으므로 확인 요청 항목도 없다.

배치까지 연결된 뒤에 볼 위치는 Lobby → Maharaka 진입 후 world 좌표 **(75.06, 22.40, -984.33)**
중심의 반경 약 6.2 m 원형 바닥 18조각이다. **회전·붕괴·복구는 이번 범위가 아니며 움직이지
않는다.** 회전 키는 아직 확보되지 않았다(`interptrackmove` 87개의 PosTrack/EulerTrack이 tagged
property로 직렬화되지 않아 별도 native 파싱이 필요하고, 그 작업은 다른 fork가 담당한다).

## 9. 미완료와 다음 조사 위치

- **배치·catalog·게시 미수행.** 정규 경로는 SCENE03B 18배치를 `Placements_Admitted`에 넣고
  `build_maptool_scene.py`를 다시 돌리는 것이다. 이때 329행 `mapmaterials.json` 보존 방법을
  먼저 정해야 한다(`--materials-output`을 주지 않거나 stage-merge).
- **`mapmaterials` override 행 미작성.** `floor01a`의 MIC는 variant 자산 ID에 이미 구워졌지만,
  `mapmaterials` 계약으로 별도 선언할지는 catalog 재생성 방식이 정해진 뒤에 결정해야 한다.
- **공용 도구 수정 1건 팀 검토 필요**(2.2절). 쿠크 Area의 같은 단계에 영향을 준다.
- **복구본의 바이트 동일성 미보장**(4.3절).
- 제외한 42개 컷신 후처리 판은 world 배치가 아니라 컷신 재생 경로에서 다뤄야 한다.
  어느 matinee가 이것들을 켜고 끄는지는 `scene03b`의 `interptracktoggle` 5개를 읽어야 한다.

## 10. 산출물

- `out/MaharakaContinuation_20260926_193455/Scene03bInstall/`
  (`Placements_Stage`, `SourceRaw`, `SourceMat`, `SourceTex`, `manifests`, `backup`, 스크립트 3개)
- cook 산출물: `C:/LostArkExtract/Scene03bCook_20260926/Runtime/` (566 KB)
- 백업: `Scene03bInstall/backup/` — 저작·게시 7파일, 수정 전 도구 원본,
  stale install 영수증
