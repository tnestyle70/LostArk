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

## G05. 재설치 원본 재대조와11개 mip 후보

후속 검토에서 원본 `bg_fat_stone_a`를 현재 설치 게임의
`ReleasePC/Packages/542NXYN2RGNOHQ2NYFGMHE94.upk`로 직접 resolve했다.
package SHA256은 `a22f1b4672363f87343d58e287d6bb3e327d362957684b2bc4b426a59b92cef7`다.
기존 캐시 또는 이전 복원 variant만 비교한 G02와 달리 이번에는 원본 native 입력을 읽었다.

| 직접 확인한 입력 | 결과 |
|---|---|
| D/N 원본 Texture2D 크기와 OriginalSize | 모두1024² |
| D/N 원본 native mip | 각각11개, 논리1024²부터1²까지, 작은 tail의 저장 block은4² |
| 현재 source D/N DDS | 각각1024² 단일 mip |
| 원본 MIC와 parent2개를 기존 compiler로 계산 | 현 base material 한 행과 전체 semantic 일치 |
| 원본 position/UV stream→설치 WModel | 전체1696정점 대응, UV0 exact, 최대 위치 오차1.52587890625e-5cm |
| 원본 추가 채널 | UV1=UV0, COLOR0 없음 |
| source D/N 소비 범위 | 발탄17 material 행·54배치, 그중 편집13배치 |

원본 UV tiling·회전·이동·panning static switch는 모두 OFF다. 현재 identity UV와
flags133을 바꾸어야 할 차이를 찾지 못했다. 위 geometry 검사는 native stream의
position/UV 대조이며 이번에 native triangle topology나 원작 actor/camera를 다시
대조한 것으로 확대하지 않는다. 사용자는 Original 상태였는지 확인하지 못했다고 답했다.
따라서 첨부 화면의 정확 actor·활성 material branch는 여전히 미확정이다.

`Engine/Private/Material.cpp`의 DDS 경로는 context 없는
`CreateDDSTextureFromFileEx`를 사용한다. 현재 단일 mip를 그대로 올리며 TGA의
CPU mip 생성 경로를 이 DDS에 적용하지 않는다. source BG shader는16배 anisotropic
WRAP sampler와 identity UV를 소비한다. 그러므로 원본 mip 누락은 확인됐으나,
1024² 자체 또는 scale3/3.62를 화면 흐림의 확정 원인으로 기록하지 않는다.

기존 extractor로 원본 압축 block11개를 각각 후보 DDS에 복구했다. mip0는 현재
설치 DDS와 byte exact이며 색변환·재압축·resampling을 하지 않았다. 후보는 원본
해상도 확대가 아니라 minification/filtering 입력 복구다.

| 후보 | 이전 SHA256 | 후보 SHA256 |
|---|---|---|
| `bg_fat_stone_a_tex_bg_fat_stone_rock02_d_ksr.dds` | `bf368595d42be15ea8f139d8d838d21be92dea08153eea21acfb9656c058b8f9` | `36487d0a2db0cb7e73aa9529df117b2f8a9651ef4807d3b9b30109266a66e75d` |
| `bg_fat_stone_a_tex_bg_fat_stone_rock02_n_ksr.dds` | `27b6eb26c2e93a75c477c0b37978af3358402f35fb96f82986898628e11a8657` | `8f1e7c7624596197f03e2131b846612ccfbb51c9035de1ef73f34eca5be9eccb` |

제품과 같은 DirectXTK loader를 사용하는 out WARP probe는 기존 파일의
Texture2D/SRV mip1과 후보의 mip11·mostDetailedMip0·1024²를 확인했다.
D는 BC1 sRGB, N은 BC5 linear이며 GPU readback의 모든 압축 mip bytes가 후보와
exact 일치했다. 이는 실제 GPU texture/SRV 로드 검사이며 게임 화면 판정은 아니다.
이번 별도 probe만 MSVC14.44로 컴파일했고 Product 재빌드는 실행하지 않았다.

근거는 `out/ValtanWaitingStoneReview20261004`의
`fresh-material-parameters.json`, `source-geometry-material-review.json`,
`install-candidate-receipt.json`, 두 `.dds.receipt.json`과 `loader_*.log`다.
후보 준비 시점에는 제품 Resources를 교체하지 않았으며 설치 완료는 후속 절에서
별도로 기록한다. Data/placement/geometry/shader는 변경하지 않았다.

## G06. 원본 mip 설치와 실제 GPU 로드 재검증

사용자의 후속 복원 요청에 따라 G05에서 검증한 D/N 두 DDS를 기존 Resources 경로에 설치했다.
설치 직전 두 target·candidate 및 material/placement 소비 문서의 SHA를 다시 확인했고,
`out/ValtanWaitingStoneReview20261004/install-backup-ca1ba927e6e54b1484a823e584e694f9`
에 원본을 보존한 뒤 같은 디렉터리의 stage 파일로 각각 원자 교체했다. 실패 시 자기 설치 hash와
일치하는 파일만 복구하도록 처리했다. 설치 SHA는 G05 후보 SHA와 동일하다.

설치한 파일은 Resources 상대 경로
`Map/LV_LUT_HEARTRB_ED/SourceFullRestore/Textures/` 아래
`bg_fat_stone_a_tex_bg_fat_stone_rock02_d_ksr.dds`(699,192B)와
`bg_fat_stone_a_tex_bg_fat_stone_rock02_n_ksr.dds`(1,398,256B)다.
이 두 교체본은 다른 PC에 Drive로 함께 전달해야 하며 아직 업로드하지 않았다.

설치된 target 자체를 제품과 같은 DirectXTK DDS loader로 다시 읽었다.
D와 N 모두1024²·Texture/SRV11mip·MostDetailedMip0이고, GPU에서 읽은 모든 압축 mip
bytes가 설치 파일과 exact 일치했다. D는 BC1 sRGB, N은 BC5 linear다.
근거는 같은 out의 `install-receipt.json`, `loader_installed_d.log`,
`loader_installed_n.log`이며, 기존 authoring/runtime material 및 placement는 보존했다.
Resources만 교체했으므로 C++/HLSL 컴파일과 Map 재게시가 필요한 변경은 아니다.

Client/UI 실행·자동 Reload·게임 화면 판정은 수행하지 않았다. 다음 Client가 해당 material을
새로 준비할 때 교체 DDS를 읽으며 현재 메모리의 texture를 갱신한 것으로 설명하지 않는다.
이번 완료 범위는 원본 mip 누락 복구이며, 화면의 정확한 actor·복원 재질 선택과 확대된 무늬의
최종 확인은 남아 있다. 최고 해상도·UV·배치 scale을 바꾸지 않았다.
