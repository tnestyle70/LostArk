# 발탄 전투 아레나 석재 표면·구운 조명 연결 구현 계획

## G00. 목표와 현재 기준

사용자 요청은 아레나 중앙 바닥과 주변 돌의 첫 복원이다. 앞선 전수조사 정본은
`2026-09-08_VALTAN_MATERIAL_LIGHTING_AUDIT_RESULT.md`다. 원본 정적 패키지의
420개 lightmap은 이미 구운 텍스처이며 동적 광원 420개가 아니다. 이번에는 선택한
배치가 참조하는 평균색/방향 정보 세 쌍만 추출하고, 새 조명 bake는 수행하지 않는다.

현재 브랜치는 `codex/kouku-ball-motion-effects`이며 다른 작업의 미커밋 변경이 많다.
기존 변경과 실행 중인 Client/Server를 보존한다. 화면 조작·최종 육안 판정은 사용자 담당이다.

## G01. 선택한 배치와 리소스

중앙 약 (156.28,23.24,-121.98)m에서 원본 MIC
`lv_lut_heartrb.mat.bg_pap_stone_rock04_mi_ksr`를 사용하는 다음 일곱 배치를 선택한다.

- 중앙 floor01: SL00 export 1271, 1299, 1304, 1337.
- 주변 rock02: SL00 export 1643, SL04 export 2950, SL03 export 4893.

원본 geometry 두 종류의 UV1과 tangent handedness를 보존한다. 중앙 component의
후속 native record에서 발견된 ColorVertexBuffer는 정점 순서와 BGRA 변환을 검증한 뒤
기존 CModel의 COLOR0 배열을 실제 CMesh GPU 입력까지 운반한다. 배치별 색이 다르면
별도 geometry variant를 사용한다.
다른 층의 같은 모델, Deploy의 파괴 바닥·난간과 파편은 이번 교체 대상으로 확대하지 않는다.

리소스는 `Resources/Map/LV_LUT_HEARTRB_ED/SourceStoneRestore/`의 geometry/texture와
`Resources/Map/Lighting/Valtan/`의 선택 lightmap DDS에 둔다. 기존 모델을 덮어쓰지 않고
새 variant를 일곱 배치에만 연결한다. 실제 추가 경로와 Drive 공유 범위는 RESULT에 기록한다.

## G02. 원본 표면 계산과 엔진 계약

이 MIC는 `bg_base_opa`의 기본 D/N + overlay D/N, 정점 R/A, 색·채도·밝기 및
Blinn specular를 쓰는 분기다. PBR ORM이나 BRDF LUT를 이 식에 임의로 추가하지 않는다.
원본 레지스터 재검산으로 세 pass의 overlay weight는 같음을 확인했다. Base/Baked는
mixed normal을 정규화하고 Direct는 그 길이를 보존하며 specular에는 base normal을 쓰는
차이를 보존한다.

`MODEL_SURFACE_FAMILY`에 기존 0~6을 보존하고 `SOURCE_OVERLAY_OPAQUE=7`을 추가한다.
`MODEL_SURFACE_PARAMETERS`와 material override는 이 식의 명시적인 overlay 입력을
소유한다. `CModel::Initialize_Binary`의 기존 override 검증, `CMaterial`의 SRV 로드,
`Bind_StaticMaterialInputs`의 기존 source material 분기를 확장한다. 다른 모델 런타임은 만들지 않는다.

WModel의 CPU COLOR0 배열만으로는 GPU 입력이 연결되지 않는다. `VTXMESH`에 RGBA8
정점색을 offset64로 추가해 stride68로 확장하고, 일반/인스턴스 입력 레이아웃과 `CMesh`
업로드를 함께 수정한다. 기존 WModel 파일 형식과 instance payload176바이트는 보존한다.
Engine public header와 DLL 및 Client를 같은 빌드에서 배포한다.

Static/instanced mesh shader는 원본 COLOR0와 UV1을 읽는다. Deferred marker7은 기존
8개 G-buffer의 RT6/7을 이 표면에 한해 사용해 Direct 전용 diffuse와 mixed normal을
전달한다. 원본 specular에 필요한 base normal과 RGB도 보존한다. 별도 render target은
추가하지 않는다. Base/Baked 출력과 Direct 합성이 albedo를 두 번 곱하지 않도록 연결한다.
SSAO와 FINAL 진단도 marker7의 mixed normal을 읽고 Renderer가 그 RT의 SRV를 명시적으로
바인딩한다. 이 opaque 계열의 diffuse alpha를 그림자 컷아웃으로 사용하지 않는다.

원본 baked에 들어 있는 조명 GUID와 현재 Point22개를 선택 배치에서 대조한다. 실제
구운 조명이 있는 픽셀에는 공통 ambient를 중복 추가하지 않는다. 직접광·그림자·Point의
원본 활성 조건이 확인되지 않은 부분은 전체 조명을 끄거나 새로 배치해 숨기지 않는다.

## G03. 저작·로드·실패 처리

catalog 정본은 `Data/Maps/Imported/LV_LUT_HEARTRB_ED/`이며 placement와 mapmaterials는
`Data/Maps/Authoring/LV_LUT_HEARTRB_ED/`다.
catalog가 선택 variant와 optional material 문서를 참조하고, placementLighting이 stable
placement ID별 atlas scale/bias와 RGB coefficient를 가진다. 기존 ID와 transform은 유지한다.

`CMapAssetCatalog::Load_MaterialOverrides`와 `Publish-MapAuthoring.ps1`은 같은 family,
필수 field·유한 수치·Resources-relative DDS·재질 이름과 UV1 조건을 검증한다. 원본 문서는
검증 후 기존 publisher로 runtime에 배포한다. parse/validate/stage/commit 및 기존 실패
상태 보고를 유지하며 잘못된 항목을 정상 fallback으로 위장하지 않는다.

새 C++ 파일은 계획하지 않는다. 새 HLSL include가 생기면 실제 소유 프로젝트와 filters에
필요한 항목만 등록한다. 빌드가 배포하는 Engine shader는 Engine 쪽을 정본으로 수정한다.

## G04. 검증과 사용자 확인

geometry triangle/좌표·UV1·색 stream 대응, 선택 배치/재질/atlas join, 실제 DDS와 mip,
수정 JSON/XML parse 및 `git diff --check`를 확인한다. 관련 shader 컴파일과 필요한 C++
컴파일을 수행하고 Engine public 계약 변경에 맞는 Product Debug 빌드로 연결을 확인한다.
다른 빌드나 실행 파일 잠금이 있으면 충돌시키지 않고 실행 파일 교체 필요 상태를 알린다.

자동 검증 결과와 실제 Client 실행·사용자 화면 비교는 RESULT에서 구분한다. 사용자는
Server + Client를 실행해 Lobby → Valtan의 중앙 바닥과 주변 바위를 비교한다. 원본 색조,
광원 전체·그림자·안개 및 Deploy 바닥까지 동일해졌다는 판정은 이번 구조 연결로 대신하지 않는다.
