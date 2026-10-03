# 발탄 입장 대기 돌의 재질 연결 조사·검증 결과

## G00. 현재 상태와 첨부 화면 경계

2026-10-04 사용자 첨부 화면의 입장 집결 발판은 `LV_LUT_HEARTRB_ED`의
`Stage_Boss_Assembly` 근처다. 화면의 큰 돌 후보는
`editor:LV_LUT_HEARTRB_ED:129`이며, 화면만으로 actor를 확정하지 않았다.
이 문서는 원본 재질 연결 누락과 저장 scale에 따른 무늬 확대를 구분한다.

**원본 재질 한 행의 저작·런타임 반영과 정식 게시 검증을 완료했다.**
공유 빌드는 shader 컴파일 단계에 있고 Map 읽기/게시가 없다는 owner 확인 후,
승인된 Data 두 파일만 반영했다. 이 변경의 완료를 전체 Product 빌드 성공으로 설명하지 않는다.
Client/UI를 실행·조작하거나 사용자 최종 화면을 판정하지 않았다.

## G01. 실제 대상과 기존 데이터

| 항목 | 실측 |
|---|---|
| 집결 위치 | `(123.38, 23.05, -89.2)` |
| 발판 중심 source ID | `LV_LUT_HEARTRB_ED_SL00:export:1442` |
| 후보 base asset | `MAP_557CAE338ABF_BG_FAT_STONE_ROCK02_SM` |
| 원본 mesh 정본 | `bg_fat_stone_a.mesh.bg_fat_stone_rock02_sm` |
| named material | `bg_fat_stone_rock02_mi_ksr` |
| 기본 source MIC | `bg_fat_stone_a.mat.bg_fat_stone_rock02_mi_ksr` |
| base asset 소비 | editor112~119,124~126,128~129의13배치 |
| editor129 저장 위치 | `(116.056999, 20.9190006, -87.670517)` |
| editor129 저장 scale | `(3, 1, 3.61999989)` |
| 실제 world bounds X/Y/Z | `108.730794..123.383204 / 18.741036..23.096966 / -93.018163..-82.322871` |

원본 VFR 15개 기본 MIC 행은 asset ID와 배치별 bakedLighting을 제외하면 같다.
다른 한 VFR의 `lv_lut_heartrb.mat.bg_fat_stone_rock02_mi_ksr`는 brightness0.5,
색 `[0.8,0.9666666,1]`, saturation0.5인 component override다.
같은 leaf 이름과 D/N을 사용하므로 leaf나 DDS만으로 full MIC를 유일하게 역증명할
수 없다. 이번 후보는 full mesh 정본과 이미 복원된 기본15행을 잇는 기본 재질
연결 보완이며, 원본 UPK를 새로 재확인한 결과가 아니다.

## G02. UV·geometry·DDS 검증

기존 legacy WModel과 현재 복원 native WModel의 1,696정점은
Position/Normal/UV0/Tangent XYZW 12float가 모두 exact 일치한다.
2,860삼각형은 cyclic index 순서만 달라 동일한 oriented faces다.
native UV1은 UV0와 같고 COLOR0/UV2는 없다.
legacy DXT1 diffuse와 ATI2 normal은 각각1024²이며 source 복원 DDS의 payload와
전부 같다. 이 비교만으로 원본 native mip 전체 재확보를 주장하지 않는다.

생산 `Shader_MapMaterialSurface.hlsli::MapSourceBGUV` 실제 본문을 추출해
MSVC14.44로 컴파일한 D3D11 WARP probe에서 설치 BC1/sRGB texture를 샘플했다.
1,696정점과2,860면 중심 총4,556UV에서 기존 UV 대비 최대 좌표 오차0,
mip0 RGBA 오차0, failure0이다. 이 검사는 UV와 mip0 샘플의 동치이며
실제 Product 삼각형 interpolation/화면 derivative/mip filtering 검증이 아니다.

근거는 `out/ValtanWaitingStone20261004/prepare.py`, `prepare-receipt.json`,
`actual-uv.hlsl`, `uv_probe.cpp`, `uv_compile.log`, `uv_run.log`다.
기존 설치 파일과 Data는 이 검사에서 변경하지 않았다.

## G03. 게시 경로와 재질 후보 검토

actual mapset이 명시한8개 runtime shard의13,184행은 Authoring과 모두 일치한다.
기존4,511 material row와11,076 placementLighting row도 semantic 일치다.
처음 발견한 통합 runtime `.mapplacements` 차이는 현재 shard 경로에서 읽지 않는
파일이었다. mapset에 없는 LANDSCAPE6행도 활성 배치 집계에서 제외했다.
정식 `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Scope Area -Mode Check`는
13,184 placements /8shards /25files, exit0이다. 게시 경로 수정은 필요 없다.

후보는 `candidate-material-row.json`이며, 기존15개 기본 MIC에서 `bakedLighting`을
제외하고 base asset ID로 연결한 한 행이다. 정상 SRC BG flags133의 diffuse/normal,
specular, 색 공간과 UV 입력을 그대로 쓴다. 다른 component의 atlas/static shadow는
복사하지 않는다. CMapAssetCatalog parser와 CModel/CMaterial 소비자 독립 리뷰에서
필수 채널 결함을 찾지 못했다. UV1은 bakedLighting이 있을 때 필요하며,
COLOR가 없는 현재 두 모델은 동일한 기본 alpha1을 소비한다.

`stage_material.py --apply`로 기존 문서를 전체 재직렬화하지 않고 한 행만 삽입했다.
반영된4512행의 첫 행을 제외한4511행과 placementLighting 및 기존 bytes가 모두
보존되는 것을 검사했다. 원본 SHA 재확인·out 백업 뒤 atomic replace했다.
기존 문서에 있던 bare LF145개는 그대로 보존했고 새67줄은 CRLF다.

| 실제 실행 | 결과 |
|---|---|
| 현행 baseline publisher `-Scope Area -Mode Check` | exit0,13,184 placements /8shards /25files |
| 적용 뒤 같은 publisher `-Mode Validate` | exit0 |
| source freshness 확인 뒤 `-Mode Publish` | exit0 |
| 최종 `-Mode Check` | exit0 |
| Area 관련65파일의 수정 전후 hash | authoring/runtime material 두 파일만 변경 |
| 원본4,511 material /11,076 placementLighting 비교 | 전부 보존 |
| 저작·런타임 최종 JSON | semantic exact 일치 |
| active8shard/catalog/MapCatalog/geometry/DDS/TRS | 보존 |
| 두 material 파일 diff | 각67줄 추가, 삭제0 |
| 관련 `git diff --check` | PASS |

최종 authoring SHA256은
`956b480507833896927ec0fbe219cbce81439c0bb57df5e75effb73ee05de1b2`,
runtime SHA256은 `58263fd1bc60f6f22597af141ff81f339a529dd4d3927dd061f817e0069d4898`다.
`out/ValtanWaitingStone20261004/{apply-receipt,final-receipt}.json` 및
`{validate,publish,check}.log`가 반영 증거다. Resources와 source C++/shader를
바꾸지 않았으므로 이번 데이터 수정에 별도 제품 재컴파일은 필요하지 않다.

## G04. 남은 화면 확인

재질 연결은 legacy normal/specular 계산을 기존 원본 BG family로 바꾸는 보완이다.
UV transform과 DDS가 같아 편집기의 scale3/3.62배로 확대된 텍스처를 줄이지 않는다.
임의 tiling이나 전역 projection, 사용자 scale 변경은 하지 않는다.
첨부 화면의 정확 actor와 원작 기준 공간 UV·층 표현은 사용자 선택 및 필요시
원본 package 재확보 후 대조해야 한다. 현재 결과를 텍스처 늘어짐 해결이나
원작 전체 동등, 사용자 visual PASS로 설명하지 않는다.
