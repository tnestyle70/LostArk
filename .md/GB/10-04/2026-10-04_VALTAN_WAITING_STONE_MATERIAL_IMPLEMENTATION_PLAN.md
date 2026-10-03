# 발탄 입장 대기 돌의 원본 재질 연결 구현 계획

## G00. 대상과 원인 경계

사용자가 첨부한 발탄 입장 대기 화면의 큰 돌에서 눌리거나 늘어진 무늬를 지적했다.
현재 입장 집결은 `LV_LUT_HEARTRB_ED`의 `Stage_Boss_Assembly`,
위치 `(123.38, 23.05, -89.2)`다. 발판 중심은 SL00 export1442다.
근처 큰 돌 후보 `editor:LV_LUT_HEARTRB_ED:129`는 원본 component가 아니라
편집기로 추가한 `MAP_557CAE338ABF_BG_FAT_STONE_ROCK02_SM`이며,
저장 scale `(3, 1, 3.61999989)`를 사용한다. 첨부 화면만으로 이 actor를 확정하지 않는다.

이 base asset의 편집 배치 13개에 원본 재질 행이 없어 legacy 계산을 사용한다.
같은 원본 mesh의 복원 variant 15개에는 기본 MIC
`bg_fat_stone_a.mat.bg_fat_stone_rock02_mi_ksr`가 연결되어 있다.
다른 한 variant의 `lv_lut_heartrb.mat.bg_fat_stone_rock02_mi_ksr`는
component override이므로 기본 재질 근거로 복사하지 않는다.

현재 설치 legacy/native 모델의 1,696정점 Position/Normal/UV0/Tangent XYZW가
전부 같다. 2,860개 삼각형도 cyclic index 순서만 다르며 winding과 면은 같다.
native의 추가 UV1은 UV0와 같고 COLOR0는 없다. 기본 MIC 15행은
asset ID와 배치별 bakedLighting을 제외하면 모두 같고, legacy/native D/N DDS
payload도 같다. 원본 UPK는 현재 PC에 없으므로 새 원본 추출이나 raw oracle 확인을
했다고 주장하지 않는다. 기존 복원 정본과 현재 설치 파일이 이번 연결의 근거다.

실제 active mapset의 8개 shard 13,184행은 Authoring과 모두 일치한다.
비활성 통합 `.mapplacements`와 mapset에 없는 LANDSCAPE 파일을 비교해
게시가 오래됐다고 판단하지 않는다. 정식 publisher의 현행 Area Check도 통과했다.

## G01. 재질 한 행과 실제 소비자

수정 파일은 다음 두 문서다.

- 저작 정본 `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapmaterials.json`
- 정식 publisher 출력 `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapmaterials.json`

기존 기본 MIC의 non-baked 입력을 base asset ID에 한 행 연결한다.
key는 `MAP_557CAE338ABF_BG_FAT_STONE_ROCK02_SM`과
`bg_fat_stone_rock02_mi_ksr`다. family `bg-source-opaque-masked`, flags133,
기존 diffuse/normal texture와 brightness/color/normal/specular/UV 값을 그대로 쓴다.
다른 component의 lightmap atlas, 좌표, static shadow는 추가하지 않는다.

흐름은 기존 `CMapAssetCatalog::Parse_ModelSurface` → CModel material override →
CMaterial texture 준비 → MapAssetRenderUtils family8 → 공통 source BG shader다.
새 schema, shader, C++ 파일과 프로젝트 등록은 필요 없다.
실제 영향을 받는 배치는 editor112~119,124~126,128~129의 13개다.
원본 VFR 배치, 8개 shard, 사용자 위치·회전·scale, geometry와 Resources는 보존한다.

후보는 `out/ValtanWaitingStone20261004/candidate-material-row.json`에 준비한다.
기존 문서의 bytes를 재직렬화하지 않고 `materials` 배열에 한 행만 삽입한다.
반영 직전 최신 SHA를 다시 확인하고 원본 백업 뒤 atomic replace한다.
이후 정식 `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Scope Area`로
Validate/Publish/Check하며, 실패하면 이번 추가만 원래 bytes로 되돌린다.
공유 Product 빌드 중에는 live Data 교체를 시작하지 않는다.

## G02. 검증과 남은 판단

기존 4,511 material row 및 11,076 placementLighting row 보존, 정확한 한 행 증가,
13개 editor 배치의 asset/TRS 보존과 active shard/catalog/다른 optional layer의 hash를
확인한다. 실제 shader `MapSourceBGUV` 본문과 설치 BC1/sRGB DDS를 WARP에서
1,696정점 및 2,860면 중심의 4,556 UV로 비교한다. mip0 샘플은 공간 좌표의
동치만 분리하며 실제 Product pixel quad derivative나 화면 판정을 대신하지 않는다.

이 재질 행의 UV transform은 identity이고 텍스처 payload가 같으므로,
원본 재질 연결이 사용자의 비균일 scale로 확대된 무늬를 줄이는 수정은 아니다.
scale 변경이나 임의 UV 반복 배수, 전역 triplanar 투영을 이번 복원에 섞지 않는다.
재질 연결 완료와 첨부 화면의 늘어짐 해결, 최종 사용자 화면 확인을 구분한다.
후자의 판단에는 실제 선택된 actor와 필요시 재확보한 원본 scene/MIC/UV 비교가 남는다.

C++/shader 변경이 없는 데이터 수정이므로 별도 제품 재컴파일은 필요 없다.
실행하지 않은 Client/UI 검사와 GPU 화면 결과를 PASS로 기록하지 않는다.
