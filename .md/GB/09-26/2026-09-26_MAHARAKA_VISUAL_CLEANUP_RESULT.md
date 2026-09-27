# 마하라카 바닥 얼룩·물 색·원형 무대 RESULT (2026-09-26)

작성: 조정자가 시킨 임무 F. 사용자 원문: "노랑색이랑 초록색으로 바닥에 이상하게 표시되어있는것들이 있어 그거 지워버리든가 해서 없애 버려 이상해. 그리고 물 색깔이 어디는 이쁘게 나오는데 어디는 막 지금 흐리게 나오잖아 원작 데이터 보면서 원작이랑 최대한 가깝게 적용시켜줄래?"

세션 중간에 두 번 한도(429)로 끊겨 세 번에 걸쳐 이어서 작업했다. 빌드는 하지 않았다(조정자가 별도로 돌린다). Client를 실행할 수 없어 화면 결과는 확정하지 못한다 — 아래 수치는 전부 파일·코드 검사 결과다.

## 1. 관찰 (사용자 첨부 스크린샷 2장)

- `스크린샷 2026-09-26 000432.png`: 해변 모래 위에 샛노란 타원 얼룩 1개, 형광 초록 얼룩 여러 개, 곡선 스트로크 모양. 왼쪽 바다는 짙은 파랑 평면.
- `스크린샷 2026-09-26 000519.png`: 아레나 풀장 물이 밝은 하늘색 거품 노이즈로 흐릿하게 보임, 가운데 원형 무대가 회색(원작은 분홍·보라 줄무늬), 왼쪽 아래 초록 얼룩.

## 2. 바닥 노랑·초록 얼룩 — 원인 확정, 수정 완료

**원인 (확정, 코드로 재현):** 섬 지형(랜드스케이프) 46개 타일은 `Tools/LandscapeExtractor/extract_ue3_landscape.py`가 UE3 weightmap을 텍셀별로 베이크한 `baked_diffuse.png`다. 이 도구의 `adjusted_layer_diffuse()`가 레이어별 `brightness × color`를 **클램프 없이** 곱한 뒤 가중 평균했다. 원본 마스터 재질(`layer_sources.json`)의 레이어03(땅, brightness 4.0)·레이어04(초록, brightness 4.0)·레이어02(잔디, brightness 2.0)는 밝기 배율이 1.0을 훌쩍 넘고, weight가 1.0에 가까운 텍셀에서 결과값이 1.0을 넘어 최종 byte 변환에서 **255로 딱 잘려(clip) 평평한 채도 100% 얼룩**이 된다.

**증거:** 타일 `MAP_19DDE51893D1_LAND01_LC_01170`의 기존 `baked_diffuse.png`에서 영향받은 텍셀의 R 채널 최댓값이 정확히 255였다(클리핑 확정). diff mask(`out/MaharakaVisual20260926/`에 남긴 조사 중 시각 확인)는 곡선 띠 모양이었고, 사용자 스크린샷의 곡선형 노랑·초록 얼룩과 형태가 일치한다.

**수정:** `adjusted_layer_diffuse()`가 반환하기 직전 각 채널을 `min(1.0, max(0.0, …))`로 클램프하도록 한 줄 단위로 고쳤다(주석 포함, 앵커 매치 1회 확인, LF/무BOM 인코딩 보존). 이미 1.0 미만인 텍셀은 계산이 그대로다(레이어 자체의 albedo는 어차피 0..1로 쓰이므로 이 클램프는 코드의 기존 가정과 일치한다).

**검증:**
- 기존 단위 테스트 23개 전부 통과(`python -m unittest test_extract_ue3_landscape`, 변경 후).
- 스테이징된 원본 데이터(umodel 재호출 없이 09-19 스테이징의 `SourceRaw` 컴포넌트 JSON + weightmap 원본 픽셀 + 마스터 재질 JSON을 그대로 읽어 이 도구의 실제 함수(`build_layer_sources`, `bake_component_textures`)를 재사용)로 46개 컴포넌트 전체를 다시 구웠다.
- 섬 16개 타일 중 **8개**의 `baked_diffuse.png`가 바뀌었고 나머지 8개는 바이트까지 동일했다(클램프가 이미 정상인 픽셀을 건드리지 않는다는 증거). `baked_normal.png`는 8개 전부 바이트 동일(클램프는 diffuse에만 영향).
  - 바뀐 타일: `MAP_0AFFAE225908_LAND01_LC_01172`, `MAP_19DDE51893D1_LAND01_LC_01170`, `MAP_4CBD4C4B4315_LAND01_LC_01175`, `MAP_86DAC42217AC_LAND01_LC_01173`, `MAP_A6038D5C62B9_LAND01_LC_01179`, `MAP_B239E97CFE1A_LAND01_LC_01171`, `MAP_E082280F30AF_LAND01_LC_01180`, `MAP_F4D937137796_LAND01_LC_01169`.
- 8개 타일의 `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP_LAND/Landscape/<타일>/textures/baked_diffuse.png`를 새 바이트로 교체했다(원본은 `out/MaharakaVisual20260926/backup/landscape_tiles/<타일>/baked_diffuse.png.before`에 보존). 이 파일은 Git이 추적하지 않는 팀장 Drive 물리 폴더에 있고, `Data/Maps` 저작 문서를 건드리지 않았으므로 **재게시가 필요 없다**(아래 5절).

**확인 못 한 것:** 이 클리핑 수정이 사용자 스크린샷의 특정 얼룩을 화면에서 정확히 없애는지는 Client 실행 없이 확정할 수 없다. 곡선 형태 일치와 클리핑 증거로 원인이 맞다고 판단했지만, 최종 판정은 사용자 몫이다.

**저장소에 반영한 것:** `Tools/LandscapeExtractor/extract_ue3_landscape.py` 한 파일(진짜 버그 수정, 팀 기존 도구를 그대로 재사용). 재굽기는 이름 기반 숨김이나 `out` 전용 스크립트가 아니라 이 저장소 도구의 실제 함수를 그대로 불러 썼다.

## 3. 물 색 — 재조사 결과 대부분 이미 완료, 3개 자산만 미해결

09-25일 밤 fork(마하라카 물 복원)가 남긴 것을 다시 코드·데이터로 확인했다. **끊기기 전에 내가 "물 평면 20개 중 7개만 water-41, 13개는 옛 거품 경로"라고 적었던 것은 틀렸다** — 다시 확인하니 그 13개는 대부분 같은 물 메시의 **다른 y 높이에 쌓인 별도 불투명 재질**(바닥판·색 있는 풀 안쪽 등, `bg-source-opaque-masked`/`bg_base_opa_overlay` family로 정상적으로 색 재질이 붙어 있음)이거나, 팀장 렌더링 규칙과 무관한 정상 배치였다. 정정한다.

재확인한 사실:
- `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapwater.json`의 물 10행 중 `kind=Water`로 게시된 자산 7종은 **전부** `source.map.water-41.v1` 근사 행이 이미 붙어 있다(어제 fork 작업, `PROJECT_AUTHORED`로 표시됨). 추가로 손댈 것이 없다.
- 남은 3개 자산(`MAP_4C8A5424422D/557CAE338ABF/A98778E158B2_BG_FAT_STONE_ROCK0{1,2,3}_SM_OVR_1A2629364329`, 총 13개 배치)은 `mapwater.json`에서 `provenance=SOURCE_MATERIAL_EXACT`로 물로 분류돼 있지만 `mapmaterials.json`에 재질 행이 **0개**다. 원본 재질 체인이 `specialresource.mi.river_water_mi`(강물 프리셋, `lv_ber_kandad.mat.lv_ber_kandad_f_water_01`)를 거치는데, 이건 어제 fork의 `author_ocean_water_rows.py`가 `sky_color`→`diffuse_color` 매핑을 쓰는 바다(ocean_trn 직계) 전용이라 의도적으로 건너뛴 것이다(스크립트 주석에 그렇게 적혀 있고, 이번에 코드로 재확인함). 강물 프리셋은 `sky_color` 필드 자체가 없어 같은 매핑을 쓸 수 없고, 다른 값 매핑을 새로 만드는 것은 이번 예산 안에서 확신 있게 끝내지 못해 **미해결로 남긴다**(추측으로 매핑하지 않음). 바위 주변 잔물결/포말 장식으로 보이며 메인 풀·바다보다 영향이 작다.

**이번에 물 데이터는 바꾸지 않았다.** 이미 완료된 부분을 다시 만들지 않았고, 미해결 3종은 정직하게 남겼다.

## 4. 원형 무대(워터팡 아레나 줄무늬 원판) — 특정 실패, 미해결

원작 조감도의 분홍·보라 줄무늬 원형 무대에 해당하는 자산을 섬 중심(바다 평면 위치 기준 반경 20m, 30m로 두 번 검색) 배치에서 찾았으나, 그 자리 근처의 대형 메시(`TST_ADD_06_1`, `LV_BER_BERNILF_FLOOR01_SM_OVR_82C061BF8240` 등)는 이미 색 있는 재질 행이 붙어 있어 회색 폴백이 아니었다. 화면에 보이는 회색 원판과 정확히 대응하는 자산을 이번 예산 안에서 특정하지 못했다. 추측으로 아무 자산이나 고치지 않고 **미해결로 기록**한다. 다음에 다시 볼 때는 정확한 원판 좌표(예: F1 피킹이나 사용자 관찰)부터 먼저 확보하는 것을 권한다.

## 5. 게시

Data 저작 문서(`mapmaterials.json`, `mapwater.json`, `mapplacements` 등)를 이번에 하나도 바꾸지 않았다(2·3·4절 모두 재확인만 하거나 Resources 텍스처만 교체). 그래서 **재게시가 필요 없다.** 그래도 상태 확인을 위해 `Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Validate`를 빌드 잠금 아래 실행했다(13:41:12~13:41:17, 5초): `PlacementCount 4651`, `FileCount 5`, 오류 없음. 로그 `out/MaharakaVisual20260926/logs/validate.log`.

## 6. 검증

- 지형 도구 단위 테스트 23개 통과(변경 후).
- 재굽기 46개 컴포넌트 전부 성공, 8개만 변경, normal 전부 동일.
- 렌더링 보호 파일 무변경: `git diff --stat -- Engine/Private/Renderer.cpp Engine/Public/Renderer.h Client/Bin/ShaderFiles Data/Rendering Client/Bin/DataFiles/Rendering` 비어 있음.
- 팀장 리소스(마하라카 재질/물 텍스처 등)는 건드리지 않았다. 이번에 바꾼 Resources 파일은 지형 타일 8개의 `baked_diffuse.png`뿐이고, 전부 백업이 있다.
- 저장소 추적 파일 변경은 정확히 1개: `Tools/LandscapeExtractor/extract_ue3_landscape.py`. 시작(재개) 시점 `git status --short`는 `out/MaharakaVisual20260926/git_status_resume.txt`에 저장했고, 그 이후 diff로 이 사실을 확인했다.
- 빌드가 필요한 변경(C++)은 없다. Python 도구 수정과 Resources 텍스처 교체뿐이라 **Client 재시작만으로 반영된다**(재빌드 불필요).

## 7. 사용자가 Client 재시작 뒤 확인할 것

1. `Lobby → Maharaka`로 들어간다(재빌드 없이 이미 켜진 Client를 재시작만 해도 됨 — Resources는 프로세스 시작 시 로드되므로 완전 재시작 필요).
2. 2절에서 나열한 8개 타일 위치의 해변·잔디 지역에서 노랑 타원·초록 얼룩이 사라졌는지 확인한다. 8개 타일 좌표는 섬 좌표 x≈0~119, z≈-912~-1032 범위(정확한 타일별 위치는 `out/MaharakaVisual20260926/`의 조사 기록에 있음).
3. 물 색은 이번에 바꾸지 않았으므로 09-25일 밤 상태와 같다.
4. 원형 무대는 이번에도 회색 그대로일 것이다(미해결).

## 8. 되돌리는 방법

`out/MaharakaVisual20260926/backup/landscape_tiles/<타일>/baked_diffuse.png.before`를 해당 `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP_LAND/Landscape/<타일>/textures/baked_diffuse.png`에 덮어쓴다. 도구 코드는 `out/MaharakaVisual20260926/backup/extract_ue3_landscape.py.before-clamp`가 원본이다.

## 9. 사용자 결정이 필요한 것

- 물의 강물 프리셋 3자산(13 배치, 미해결)을 위해 새 파라미터 매핑을 만들지, 이번처럼 미해결로 둘지.
- 원형 무대 자산을 정확히 특정하려면 F1으로 직접 좌표를 찍어 알려주는 쪽이 빠를 수 있다.

## 10. 최종 재확인

**확인한 것:** 얼룩 원인(클리핑)을 코드·바이트 증거로 확정, 도구 단위 테스트 통과, 재굽기 결과가 8개 타일에서만 바뀌고 나머지는 완전 동일함을 바이트로 대조, 정상 텍셀은 값이 안 바뀜을 diff로 확인, 렌더링 보호 파일 무변경, 저작 문서 무변경이라 재게시 불필요함을 Validate로 확인, 배·카메라 등 다른 fork 파일을 건드리지 않았음을 git status로 확인.
**확인하지 못한 것:** 실제 화면에서 얼룩이 사라졌는지(Client 미실행), 물의 강물 프리셋 3자산 매핑, 원형 무대 자산 특정.

MAHARAKA_VISUAL_DONE (partial): 얼룩(항목 A)은 원인 확정·수정·적용까지 끝냈고 화면 확인만 남았다. 물(항목 B)은 강물 프리셋 3자산 매핑이 미해결이다. 원형 무대(항목 C)는 자산을 특정하지 못해 미해결이다. 게시(항목 D)는 데이터 변경이 없어 재게시가 필요 없음을 Validate로 확인했다.
