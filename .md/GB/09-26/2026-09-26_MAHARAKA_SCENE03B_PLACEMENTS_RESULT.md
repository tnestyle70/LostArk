# 2026-09-26 마하라카 SCENE03B 워터팡 무대 배치 설치·게시 결과

지시 범위는 무대 바닥 18조각의 배치를 마하라카 Area에 넣고 게시하는 것, 그리고 직전 작업이
멈춘 이유인 "손으로 쓴 `mapmaterials.json`을 보존하면서 Area catalog를 재생성하는 방법"을
푸는 것이었다. **둘 다 완료했다.** 배치 +18, 자산 +2로 게시했고 기존 행은 한 줄도 잃지 않았다.

## 1. 막혀 있던 문제의 실제 답 — 애초에 위험이 없었다

직전 작업은 `build_maptool_scene.py`의 `--materials-output`이 `mapmaterials.json`을 덮을 수
있다고 보고 멈췄다. 코드를 읽으면 그 위험은 **이 Area에 존재하지 않는다.**

| 위치 | 내용 |
|---|---|
| `build_maptool_scene.py:1078` | `material_text = None` |
| `:1079` | `if source_material_build:` 일 때만 `material_text`를 채운다 |
| `:1089` | 그 안에서만 material 문서를 만든다 |
| `:1161~1162` | `if material_text is not None:` 일 때만 `materials_output`을 쓰기 목록에 넣는다 |
| `:617~619` | `--source-materials-receipt`와 `--materials-output`은 **함께** 주어야 한다 |
| `:1193` | `--materials-output`은 **required가 아니다** |

즉 두 인자를 주지 않으면 `mapmaterials.json`은 **열리지도 않는다.**

그리고 이 Area는 원래부터 그렇게 빌드됐다. 기존 `build.receipt.json`의 증거다.

```
outputs            = ['catalog', 'placements']      <- materials 키 없음
sourceMaterialBuild = null
```

`mapmaterials.json`은 `Data/Maps/Authoring/`에만 있고 `Imported/`에는 없다. scene builder는
`Imported/`에 쓴다. 두 문서의 수명이 처음부터 분리돼 있었다.

**결론: 두 인자를 생략하고 재생성하면 된다.** 병합 스크립트도, 도구 수정도 필요 없었다.
실제로 재생성 후 `mapmaterials.json`의 SHA-256이 저작본·게시본 모두 `8ad7ecdb3eb926c1`로
백업과 완전히 동일하다.

## 2. 실행한 파이프라인

이 Area의 실제 생성 순서는 2단이다. `landscape-merge.receipt.json`의 입력 해시가 scene build의
출력 해시와 정확히 맞물려 이를 증명한다(`04AD51D4…`, `03C24116…`).

```
build_maptool_scene.py   -> base catalog/placements
merge_maptool_landscape.py -> +16 landscape 자산·배치
Publish-MapAuthoring.ps1 -> DataFiles
```

### 2.1 입력 확보

4개 manifest를 receipt의 SHA-256으로 역추적해 전부 찾았다.

| 역할 | 실제 파일 |
|---|---|
| assetManifest | `LV_OCN_EVENTIS_MHP_20260919/admitted.inventory.json` |
| runtimeManifest | `MHPr2/manifests/map_material_runtime_assets.json` |
| overlayManifest | `…_20260919/Restore/foliage/LV_OCN_EVENTIS_MHP.foliage.overlay.json` |
| renderProfileManifest | `Data/Maps/Imported/…/LV_OCN_EVENTIS_MHP.renderprofiles.json` |

asset·runtime manifest는 기존 receipt 해시와 **일치 확인 후** 사용했고, 무대 2종을 더해
382 + 2 = 384로 병합했다(assetId 충돌 0). 384개 모델이 전부
`Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP`에 실재함을 먼저 확인했다(누락 0).

배치 입력은 기존 3개 + 무대 1개로 3,823 + 18 = **3,841**행이다. 무대 18행은 직전 작업이
필터한 파일을 썼고, 새 추출본 60행의 **바이트 동일 부분집합**임을 대조했다(내용 다른 행 0).

### 2.2 scene build

```
--expect-assets 392 --expect-source-placements 3841
--expect-overlay-assets 8 --expect-overlay-placements 812
--allow-partial-material-preview
(--source-materials-receipt / --materials-output 미지정)
```

exit 0. 출력 `392 자산 / 4,653 배치`. receipt `outputs`에 `materials` 키 없음, `sourceMaterialBuild: null`.

`--expect-assets`는 overlay를 포함한 전체 catalog 수라서 처음 384로 주고 실패했다(`asset count
mismatch: 392`). 392로 고쳐 통과했다.

### 2.3 landscape merge

`merge_maptool_landscape.py`를 무대 배치에 직접 쓸 수는 **없었다.** `:486~491`이 들어오는 배치
전체에 `transformSource == "component"`를 요구하는데, 무대 18조각은 `interpactor`의 actor
transform이라 `transform.source`가 18/18 모두 `actor`다. 이 가드는 landscape 전용 안전장치이므로
건드리지 않고, 기존 16행 재병합에만 사용했다.

```
--expect-base-assets 392 --expect-base-placements 4653
--expect-landscape-assets 16 --expect-landscape-placements 16
--expect-output-assets 408 --expect-output-placements 4669
```

exit 0. `--runtime-root`는 `Client/Bin/Resources`다(landscape 모델 경로가 `Map/…`로 시작해
Resources 상대이므로). 처음 `…/Map`으로 줘서 `Map\Map\…` 이중 경로로 실패했다.

## 3. 반드시 알아야 할 것 — scene builder는 `renderMode Water`를 만들 수 없다

재생성 결과를 baseline과 대조하니 **catalog 10행이 달라졌다.** 전부 같은 한 필드였다.

```
renderMode:  Water -> Opaque   (10행)
```

원인은 `build_maptool_scene.py:573~578`의 `render_profile_text()`다.

```python
if render_mode not in ("Opaque", "Alpha", "Sky", "Additive"):
    raise ValueError(f"invalid renderMode: {render_mode}")
```

`Water`가 허용 목록에 **없다.** 반면 publisher는 `Water`를 받는다
(`Publish-MapAuthoring.ps1:186` 의 `@('Opaque','Alpha','Sky','Additive','Water')`).
그리고 `renderprofiles.json` 307개 profile 중 `renderMode`를 가진 것은 `Sky` 하나뿐이다.

**즉 이 10행의 `Water`는 scene builder가 재현할 수 없는 저작 상태이고, 단순 재생성은 그것을
조용히 `Opaque`로 떨어뜨린다.** 해당 자산은 물·물속 바위다.

| 자산 |
|---|
| `…_WATER02_SM_OVR_53F9EEC1FCA4`, `…_WATER03_SM_OVR_53F9EEC1FCA4` |
| `…_LV_MODULE_WATER02_512_OVR_{3CB26CF55D22, 53F9EEC1FCA4, E67B61F41453}` |
| `BG_FAT_STONE_ROCK{01,02,03}_SM_OVR_1A2629364329` |
| `LV_BER_BERNILF_FLOOR01_SM_OVR_53F9EEC1FCA4`, `…_FLOOR02_SM_OVR_53F9EEC1FCA4` |

### 대응

`preserve_authored_render_modes.py`로 **기존 행에 대해서만** baseline의 renderMode 토큰을
그대로 이어받았다. 안전장치를 걸었다.

- 토큰 수가 달라진 행이 있으면 중단
- renderMode 외의 필드가 달라진 행이 있으면 중단 (전수 검사, 실제로 0건)
- baseline 값이 publisher 허용 5종이 아니면 중단
- 신규 2행은 builder가 낸 값을 그대로 둔다

적용 후 baseline 406행이 **전부 바이트 동일**함을 재확인했고, 영수증
`final/render-mode-preservation.receipt.json`에 복원한 10행을 기록했다.

**이것은 파이프라인 결함이며 팀 확인이 필요하다.** 근본 해결은 `render_profile_text()`가
`Water`를 받게 하고 `renderprofiles.json`에 그 10행을 저작 입력으로 넣는 것이다. 이번에는
공용 도구를 고치지 않고 보존만 했다.

솔직히 적어 둘 점이 하나 있다. 보존 스크립트는 공백 분할 인덱스 18을 renderMode로 썼는데,
label에 공백이 있는 행에서는 그 인덱스가 밀린다. 실제 필드는 **따옴표 인식 토큰 인덱스 11**이다
(publisher의 `$tokens[11]`과 같다). 다만 스크립트가 "토큰 수 동일 + 단일 필드만 차이 +
baseline 값이 허용 모드" 세 조건을 모두 강제했고, 최종 바이트 동일성 검사로 결과를 독립
확인했으므로 적용 결과는 정확하다. 이후 재사용할 때는 인덱스 11을 쓰는 게 맞다.

## 4. 게시와 전후 수치

```
Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Validate -Scope Area   exit 0
                                                    -Mode Publish  -Scope Area   exit 0
                                                    -Mode Check    -Scope Area   exit 0
```

Publish 출력 `PlacementCount 4669`, `Sha256 84769867bf87fe24…`.

| 항목 | 이전 | 이후 | 변화 |
|---|---|---|---|
| catalog 자산 | 406 | **408** | +2 |
| 배치 | 4,651 | **4,669** | **+18** |
| baseline catalog 행 사라짐/변경 | — | **0 / 0** | |
| baseline 배치 행 사라짐/변경 | — | **0 / 0** | |
| `mapmaterials.json` materials | 329 | 329 | SHA 동일 |
| `mapmaterials.json` 사라진/바뀐 행 | — | **0 / 0** | |
| `source.map.water-42.v1` 행 | 10 | 10 | |
| catalog renderMode 분포 | — | Opaque 397 / **Water 10** / Sky 1 | |
| 게시 catalog 모델 파일 누락 | — | **0 / 408** | |

게시 catalog 헤더는 `LOSTARK_MAP_ASSET_CATALOG 5 "LV_OCN_EVENTIS_MHP" 408 "LV_OCN_EVENTIS_MHP.mapmaterials.json"`다.

**지시의 "물 7행"은 실제와 다르다.** `mapmaterials.json`의 water family 행은 **10개**이고
`mapwater.json`의 `waters`도 **10개**다. 파일이 바이트 동일하므로 손실 위험은 애초에 없었고,
검사 기준을 실제 값으로 바로잡았다.

## 5. 무대 좌표 검증

게시된 18행을 다시 읽어 계산했다.

| 항목 | 실측 | 기대 | 판정 |
|---|---|---|---|
| 중심 | (75.064, −984.326) | (75.064, −984.326) | 일치 |
| 반경 | 6.160 ~ 6.209 m | 6.160 ~ 6.209 | 일치 |
| 높이 | 22.4 단일값 | 22.4 | 일치 |
| 인접 각 | 19.65 ~ 20.46° | 19.65 ~ 20.46 | 일치 |
| 자산 구성 | floor01a 9 + floor01b 9 | 9 + 9 | 일치 |
| scale tail | 단일값 1종 | — | 변환 1회만 적용 |

## 6. 재질 override 연결

무대 색은 **variant 자산에 이미 구워져 있다.** `floor01a`의 MIC
`lv_ocn_eventis_mhp.mat.bg_ocn_etc_floor01a_mi_lnh`가 override 서명 `0636fcf08a2e…`로 자산 ID
`MAP_167F91F6940F_…_OVR_0636FCF08A2E`에 반영되고, 설치된 텍스처에 원본에서 뽑은
`bg_ocn_etc_floor01a_n_lnh.png`와 `diffuse.dds`가 들어 있다.

`mapmaterials.json`에는 이 2종의 행을 **추가하지 않았다.** 그 문서는 catalog 408자산 중
251개만 덮고 **157개는 행이 없다.** 행이 없는 것이 이 Area의 정상 상태이며, 행 없는 자산은
cook된 wmodel의 재질로 그려진다. 329행을 건드리지 않는 것이 지시의 보존 요구와도 맞다.

원본 시각 동일성은 별개다. 신규 2종의 `admissionState`는
`geometry-preview-partial-material`이고 이는 이 Area 기존 382자산 전부와 같은 상태다.

## 7. Resources — 이전 사고 재발 없음

직전 작업에서 `install` 단계가 Area 폴더를 동기화해 382개를 삭제한 사고가 있었다. 이번에는
**cook·install을 전혀 실행하지 않았다**(무대 메시 2종이 이미 설치돼 있으므로 불필요).

| 항목 | 값 | 기대 |
|---|---|---|
| 자산 디렉터리 | 384 | 384 |
| 전체 파일 | 6,323 | 6,323 |
| `.wmodel` | 384 | 384 |

## 8. 빌드/자동검사

빌드 **0회**(데이터 작업, C++ 변경 없음). publisher Validate/Publish/Check 각 1회 exit 0.
검증 스크립트 3개 전부 PASS. `git diff --check`의 5건은 `MainApp.cpp`의 기존 의도된 줄 끝
공백이며 정리하지 않았다.

## 9. 수정한 파일

내 쓰기는 `23:03:38`(저작 5파일)과 publisher의 `23:04:27`(게시 5파일)뿐이다.

```
Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapassets              408 자산
Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapplacements          4,669 배치
Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.build.receipt.json
Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.landscape-merge.receipt.json
Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapplacements         4,669 배치
Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.{mapassets,mapplacements}           publisher
Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.{maplights,mapmaterials,mapwater}.json  publisher, 내용 동일
```

해시로 무변경을 재확인한 파일: `Imported/…renderprofiles.json`,
`Authoring/…{mapmaterials,maplights,mapwater}.json`, `DataFiles/…mapmaterials.json`.
`Tools/` 아래 변경과 `Data/Maps/MapCatalog.json`은 mtime이 내 작업 창 이전이며 다른 작업 것이다.

## 10. 사용자 수동확인

**Lobby → Maharaka 진입 후 world 좌표 `(75.06, 22.40, −984.33)` 중심, 반경 약 6.2 m**를 본다.
`floor01a`/`floor01b`가 20° 간격으로 번갈아 놓인 **원형 바닥 18조각**이 있어야 한다.
바로 아래 높이 19.92에 기존 `POOL01_SM` 배치가 있으므로 무대는 그 풀 위에 얹힌 형태다.

**회전·흔들림·붕괴·복구는 이번 범위가 아니며 무대는 움직이지 않는다.** 원본 곡선은 별도
작업에서 전부 판독됐다(`2026-09-26_MAHARAKA_STAGE_CURVES_RESULT.md`: 흔들림 3.0032초 yaw
±2.02°, 붕괴 타일당 3.01 m 하강 + 바깥 0.92 m, 복구 이동·회전 0) 지만 WorldSequence 저작으로
변환하지 않았다.

재질 색이 원작과 다르게 보이면 6절의 `geometry-preview-partial-material` 상태를 먼저 본다.

## 11. 미완료와 다음 조사 위치

- **`render_profile_text()`가 `Water`를 못 만드는 파이프라인 결함**(3절). 다음 Area 재생성에서
  같은 손실이 재발한다. 근본 해결은 허용 목록 확장 + `renderprofiles.json` 저작 입력화다.
- **무대 회전·붕괴 미연결.** 곡선은 판독됐고 막힌 것 네 가지는 curves RESULT 11절에 있다
  (키별 탄젠트가 WorldSequence 스키마에 없음, 음수 키 시각, Euler→quaternion 축 규약 미확정,
  사운드·emitter 45개 소유자).
- **제외한 42개 컷신 후처리 판**(흰 판 35·비 6·반구 1). world 배치가 아니라 컷신 경로이며
  `scene03b`의 `interptracktoggle` 5개를 읽어야 한다.
- `merge_maptool_landscape.py`의 `transformSource=component` 가드 때문에 actor transform 배치는
  이 도구로 병합할 수 없다. 앞으로도 scene 재생성 경로를 써야 한다.
- 이전 작업의 Resources 복구본 바이트 동일성 미보장은 그대로 남아 있다.

## 12. 산출물

`out/MaharakaContinuation_20260926_193455/Scene03bPlacements/`

```
backup/{Imported,Authoring,DataFiles}/  15파일 + backup.manifest.json (SHA-256)
Placements_Admitted/                    입력 4파일 (3,841행)
asset_manifest.merged.json              384 자산
runtime_manifest.merged.json            384 자산
scene/                                  scene build 출력 3파일
merged/                                 landscape merge 출력 3파일
final/                                  renderMode 보존 catalog + 영수증
prepare_inputs.py  preserve_authored_render_modes.py
verify_superset.py  verify_published.py  check_render_modes.py
scene_build.log  merge.log  publish_validate.log  publish.log  publish_check.log
```
