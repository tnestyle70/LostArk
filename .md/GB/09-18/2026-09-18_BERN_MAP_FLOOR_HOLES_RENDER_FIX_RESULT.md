# 2026-09-18 베른 바닥 구멍 — 렌더 경로 재진단 RESULT

## 사용자 관찰

17:35 에 `LV_MODULE_*` 272 배치를 visible 0→1 로 `-Scope Placements` 게시했으나, 베른에서 F6 로 본 바닥 구멍이 그대로였다.
게시 후 Client 세션 두 번(17:44 PID 34932, 18:27 PID 27556)이 모두 worldId 1(BERN)에 들어갔다
(`Client/Bin/Debug/Diagnostics/client-session-*.jsonl`). 새 게시본을 읽은 상태에서의 관찰이다.

## 원인

### 1. 이전 조사의 "마을 구멍 50% → 7%" 는 화면 예측으로 틀렸다

`out/BernMapHoles20260918/holes_coverage_after.npz.json` 의 커버리지는 XZ 칸 위에 보이는 메시가 **어떤 높이든** 있으면 바닥으로 셌다.
그래서 마을 바닥(y 34~54)보다 25~40m 아래에 있는 거대한 바다 평면(y 10.7)을 "채움"으로 계산했다.
화면에서 구멍이 메워졌는지는 이 수치로 판단할 수 없다.

### 2. 272개 중 261개는 원본부터 검정이고, 마을 밖(북쪽)에 있다 [확인]

| 에셋 | 개수 | 위치 | 재질 | 화면 |
|---|---|---|---|---|
| `MAP_94EA487101FE_LV_MODULE_MESH02_512_512` | 227 | x 9~253, z 95~384, y 17.8~47.6 | `lv_module.mat.black` → `source.map.black.v1` → program 64 | RGB 0 검정. 12삼각형 상자 |
| `…MESH02_512_512_OVR_30643EFC5E9F` | 34 | x 22~239, z 120~373, y 29.6~42.0 | `specialresource.mi.shadow_mi` → `source.map.shadow-modulate.v1` → program 65 | 검정 그림자 반투명 |

- 재질: `Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE.mapmaterials.json` materials[1010], [1011]. program 매핑 `Client/Public/SourceMapForwardMaterialParameters.h:91-93`.
- program 64 가 원본 native PS 와 같은 RGB0 을 낸다는 대조는 `.md/GB/09-11/2026-09-11_BERN_NATIVE_FORWARD_IMPLEMENTATION_RESULT.md` G04 에 있다.
- 마을 영역(x 15~300, z −245~35)과 겹침 0. 이 261개는 마을 구멍과 무관하고, 켜도 검정이라 북쪽에서도 화면 변화가 거의 없다.

### 3. 마을 아래를 덮는 것은 바다 평면 3개뿐이다 [확인] — 화면 표시는 [미확인]

- `MAP_7BA4AC84CD3A_LV_MODULE_WATER02_512` 3개, y 10.7~10.9, scale (160,60,120)/(500,150,300)/(320,60,120), 범위 x −722~1838, z −1714~435. 마을과 겹침 합 약 59,700m².
- 면 방향: 위(+Y)를 향한다(`out/BernMapHoles20260918/facing_check.py`, 잘 보이는 광장 `FLOOR06` 과 같음). 뒷면 컬링 원인 아님.
- 배치 경로: 재질이 translucent 라 정적 배치 대상이 아니고(`MapPlacementRuntime.cpp:915-927`) `CMapAssetObject` 로 BLEND 그룹에 들어간다(`MapAssetObject.cpp:23-31, 155-172`).
- 컬링: 구-평면 판정만 있고 거리 컷이 없다(`MapAssetRenderUtils.cpp:527-590`, 베른 정책 `LevelRegistry.cpp` baseMargin 0.25 등). 반경이 커서 잘리지 않는다.
- 재질: `source.map.water-43.v1` → program 43, 텍스처 7장 전부 실재. forward water 분기 `Shader_SourceMapForwardPrograms.hlsli:939`, 장면 색 스냅샷 요청(`MapAssetObject.cpp:160-162`).
- **water-43 은 베른에서 이 3개만 쓰고, 이 3개는 오늘 전까지 한 번도 보인 적이 없다**(게시 전 백업 대조). 즉 이 재질이 화면에 어떻게 보이는지는 검증된 적이 없다.
  바다 아래에 아무 형상이 없어 굴절 대상(장면 색)이 검정 클리어 색이므로, 그려지더라도 어둡게 보일 수 있다 [추론].

### 4. 마을 안 연못 8개는 켜졌다 [확인] — 화면은 [사용자 확인 필요]

`LV_MODULE_WATER01_1024` 변형 8개(water-40/41, 분수 등에서 이미 보이는 검증된 재질), y 38~48.
예: x 190~238, z −56~−8, y 38.2(2,277m²) / x 168~196, z −140~−111, y 47.0(839m²) / x 83~95, z −148~−130, y 48.3(218m²).

### 5. 마을 구멍의 실제 구성 (`holes_coverage2.npz.json`, 네비 영역 79,800칸)

- 구멍 39,974칸 중 **20,743칸(52%) — 원본은 지형(LANDSCAPE 42개)이 그 위에 있다.** 우리가 08-25 `d3b72bdc` 에서 절벽 텍스처 늘어짐 때문에 숨겼다.
- 19,193칸(48%) — 원본에도 마을 바닥은 없고, 25~40m 아래 바다만 있다.
- 32칸 — 의도적 숨김(`visibilityOverrides`) 아래. 6칸 — 원본에도 없음.

결론: 화면의 검은 구멍 절반은 **숨긴 지형 자리**이고, 나머지 절반은 **원본도 아래가 바다인 자리**다. 272개 해제는 둘 다 채우지 못한다.

## 이번 수정

- 코드·데이터·게시 변경 없음. C++ 을 바꾸지 않았으므로 빌드도 돌리지 않았다.
- 17:35 의 272개 해제는 원본(`visible=1`)과 일치하므로 되돌리지 않았다.
- 지형 해제용 스크립트를 준비만 했다: `out/BernMapHoles20260918/unhide_bern_landscape.py`
  (기본 드라이런, `--apply` 로 저작본 `:landscape:` 42행만 0→1, 줄끝 보존, 파일 변경 감지).
  드라이런 결과 `landscape rows to unhide 42`, 저작본 sha `ed4a2c92e704` 불변.

## 사용자 확인

1. 연못(켜진 것 확인): F6 → (214, 40, −32) 부근. 물이 보이면 해제가 게임에 반영된 것이다.
2. 바다(water-43 표시 확인): F6 로 (150, 15, −100) 부근까지 내려가 수평으로 보라. 바다 면이 보이면 그려지는 것이고, 위에서 볼 때 어두운 것은 아래가 비어서다. 아무것도 안 보이면 water-43 렌더 경로 결함이라 GPU 캡처로 따로 봐야 한다.
3. 지형: 스폰 북쪽(x 103~198, z −28~35) 등 지형 자리는 계속 검정이다.

## 사용자 결정이 필요한 것

- **지형 42개 해제**: 마을 구멍의 52%가 원본 땅으로 채워진다. 삼각형 +321,970(+1.6%). 대신 08-05 에 FAIL 판정한 절벽 텍스처 늘어짐(절벽 UV 밀도 평지의 7.9%, 지형 면적 64%가 경사)이 다시 보인다.
  적용 명령(저장소 루트, Client·Server 종료):
  ```
  python out/BernMapHoles20260918/unhide_bern_landscape.py --apply
  powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Mode Validate -Scope Placements
  powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Mode Publish -Scope Placements
  ```
- **절벽 재조리 후 해제**: 절벽 삼각형을 원본 `layercliff` 재질·측면 투영 UV 로 분리 재조리. `mapmaterials` 를 조명과 분리해 게시하는 범위가 추가로 필요하다.
- **원본도 바다인 48%**: 원본대로 두면 바다가 보이는 자리다. 광장 바닥 같은 비원본 바닥으로 덮을지는 별도 결정.

## 내 실수

- 없음(파일 쓰기는 이 문서와 `out/` 측정·준비 스크립트뿐). 이전 조사의 커버리지 기준 오류는 위 1절에 기록했다.
