# 마하라카 맵 복구 — ocean 카메라, 라이트맵 밉, 지형 표면, 사진 기준 받침대

## 현재 상태

**맵 전체 복원 완료가 아니다.** 모코모코 받침대와 중앙 지지 기둥은 사용자 승인 후
사진 기준 재구성하여 게시했다(G05). G05 적용 당시 원본 모델 연결을 찾았다는 뜻은 아니다.
후속 G07에서 큰 모코모코와 함께 참조되는 원본 `ITR_02453`을 발견·추출했다.
G07에서는 조사/변환 후보까지만 진행했으나, **G10에서 임시 받침 35개를 원본 1개로 교체하고
재질·NPC 접촉 높이까지 실제 게시했다.** 중앙 지지 기둥은 여전히 사진 기반 재구성이다.
두 구조물은 후속 사용자 스크린샷에 나타났지만 외형 일치 승인을 대신 기록하지 않는다.
침수의 확인된 지형 좌표 오류는 G06에서 수정·설치했다. 새 화면의 지형/수면 경계와
색·조명 최종 일치는 사용자 확인이 필요하다.
G01~G07에서는 NPC 동작 기능을 제외하고 두 NPC의 받침 접촉 높이만 변경했다.
후속 G08에서 사용자 승인에 따라 기존 두 NPC 포함 총 30명(안내원 5 + 방문객 23 + 기존 2)을 게시했다.
안내원·방문객의 외형/역할은 기존 NPC를 활용한 재구성이며 원본 마하라카 NPC 신원 복원이 아니다.
사용자가 제공한 09-26 및 09-27 01시 스크린샷을 관찰 입력으로 사용했다.
직접 Client 실행·화면 캡처·visual PASS 판정은 하지 않았다.

실제 수정 저장소는 `C:/Users/USER/source/졸업팀폴/LostArk`, 브랜치는
`codex/maharaka-map-restoration`이다. 별도 Codex worktree의 이전 데이터는 게시하지 않았다.
시작 정본은 기존 A/B/C 결과의 catalog 867 / placement 4,669 / material 947행이다.
예전 408/329행 후보로 롤백하거나 전체 Area 디렉터리를 교체하지 않았다.

## G01. ocean-42의 잘못된 관찰 방향 수정

`Shader_SourceMapForwardPrograms.hlsli`의 `EvaluateSourceMapWater`는 source 좌표로
변환한 절대 월드 위치를 `values[8]`에 넣고, `SourceMapWater42`와 baked 분기는
`source[0]`을 0으로 고정했다. 원본 연산의 위치 복구와 시선 계산에 이 두 값이 같이 쓰인다.
이 조합은 실제 카메라가 아니라 월드 원점을 관찰자로 삼아 Fresnel을 계산한다.
월드 원점에서 멀리 떨어진 마하라카 수면에서는 시선 각도 자체가 크게 달라진다.
이는 확인된 수학적 결함이며 모든 침수/색 문제의 단일 원인이라는 뜻은 아니다.

program 42에 한해 `values[8]=S(world)-S(camera)`, `values[9]=S(camera)`로 전달한다.
`S(x,y,z)=100*(x,-z,y)`이며 원본 PS의 `source[0]`은 `values[9]`를 소비한다.
UV 계산은 다시 `S(world)`를 얻고 시선은 `S(camera-world)`를 얻는다.
투영 마지막 행을 `100*mul(float4(camera,1),vp)`로 맞춰 최종 clip/NDC/depth를 보존했다.
Engine/Client wrapper를 함께 변경했고 source PS의 baked/non-baked 양쪽에 연결했다.
다른 water program, 원본 물 색/강도, 물 높이, 배치, 팀장 rendering profile은 변경하지 않았다.

변경 파일:

- `Client/Bin/ShaderFiles/Shader_SourceMapForwardPrograms.hlsli`
- `Engine/Bin/ShaderFiles/Shader_SourceMapForwardPrograms.hlsli`
- `Client/Bin/ShaderFiles/Shader_SourceMapWaterPrograms.hlsli`
- `Tools/MapPipeline/test_ocean_camera_frame.py`

새 C++ 파일과 프로젝트 등록 변경은 없다. 기존 include를 소비하는 셰이더 빌드로 연결된다.
회귀 4건은 좌표 분리, 전역 평행이동 불변성, 투영 보존, 양쪽 source 경로/미러를 검사한다.
독립 검토에서도 v9 충돌이나 좌표·투영 모순은 발견되지 않았으며, GPU 화면 판정은 별도다.

## G02. 실제 참조하는 라이트맵 92개에 원본 밉 체인 설치

기존 B4에서 108개 원본 밉 체인을 추출했지만 실제 Resources의 같은 DDS는
단일 mip였다. 현재 material 참조 closure에 속하는 92개만 설치했고 미사용 16개는 제외했다.
각 파일의 source identity, SOURCE_MIP_CHAIN_VALIDATED receipt, native 압축 payload 크기,
설치 전 RGBA 최상위 mip와 후보 최상위 mip의 픽셀 일치를 확인했다.
새 색 보정이나 필터로 생성한 mip가 아니라 원본 압축 mip를 설치했다.

- 설치 폴더: `Client/Bin/Resources/Map/Lighting/Maharaka/`
- 파일별 기록: `out/MaharakaMapRestoration20260927/lightmap-mips-installed.json`
- 교체 전 보존본: `out/MaharakaMapRestoration20260927/lightmaps-before/`
- 원본 후보: `out/MaharakaRebuild_B/b4_lightmap_mips/`

교체 직전 freshness와 백업을 검사하고 임시 파일 교체를 사용했다. 실패 시 자신이 바꾼 파일을
복구하는 설치 경로를 사용했다. Map authoring/runtime 문서의 참조 경로는 변경하지 않았다.
팀 전달 시 이 92개 Resources DDS가 별도 배포 대상이며 컴파일 CSO를 Git에 넣지 않는다.

## G03. 받침대 재탐색 결과와 아직 풀리지 않은 연결

사용자는 2021년 버전으로 기억한다고 답했으며 다른 추출본 경로는 모른다고 했다.
그 답을 특정 설치본이 2021 원본이라는 증거로 승격하지 않았다.

큰 모코모코 위치 근처 Deploy Prop 570954와 570987의 배치는 남아 있다.
797개 추출 DB를 primary-key 기준으로 다시 조사했지만 두 ID에 대한 Prop/Npc 모델 연결은
찾지 못했다. NpcStat의 같은 정수는 레벨/수치 행이며 모델 근거가 아니다.
이 결과는 원본 메시가 삭제됐다는 증거가 아니다.

기존 전체 LookInfo 59,888건 검색에 더해 Prop LookInfo 5,258건을 다시 검색했다.
현재 Maharaka Returns의 ITR_02655/02843/03048/03049/03050/02465 연결도 읽었다.
BG_OCN 패키지 40개 및 ITR_02*/03* 패키지 1,273개의 객체 목록을 확인했고,
후자의 static/skeletal bounds에서 높이/비율 조건 후보 144개를 분류했다(읽기 오류 0).
이 조건은 탐색 필터이지 원본 admission 근거가 아니다.
ITR_02238은 작은 바위 계열, ITR_02450은 tree_signature03 계열로 확인했으며
둘 다 받침대로 설치하지 않았다. 누락 Prop ID와 후보 메시의 정확한 연결은 미해결이다.

기존 모델 MN_ISMP_00/00-1의 머리·모자와 별도 받침대를 혼동하지 않는다.
이미 들어간 무대 색/배치, catalog 및 RNM 변종을 임의 수정하지 않았다.

## 실행한 검사

| 검사 | 결과 |
|---|---|
| ocean camera-frame Python 회귀 | 4건 PASS |
| Maharaka Area publisher Validate / Check | 둘 다 PASS, placement 4,669 |
| Debug Product 증분 build | PASS, Engine/Shared/Server/Client |
| build receipt | `out/BuildPipeline/runs/20260927T035617652Z-debug-product.json` |
| 이번 빌드의 shader output | CSO 24개 갱신 |
| git diff --check | PASS, 줄끝 LF/CRLF 안내만 발생 |
| Client 실행·시각 검증 | 수행하지 않음 |

빌드에는 기존 FXC 경고가 있었으며 무경고 빌드로 표현하지 않는다. Release는 실행하지 않았다.
이번 변경은 Resources 교체와 shader 컴파일이고 데이터 Publish는 하지 않았다.

## 다음 확인 경계

실제 저장소의 Debug Client를 사용자가 실행하고 Lobby의 Maharaka로 들어가 기존과 같은
위치/각도에서 수면을 확인해야 한다. 수정된 셰이더와 DDS는 실행 중 Client에 자동 갱신되지 않는다.
새 화면 없이 기존 스크린샷만으로 이번 물 수정의 효과나 현재 무대 색을 판정할 수 없다.

지형 bake는 아직 8-bit 파생 텍스처 경로이며 원본 HDR/RNM/GPU 재질 전체와 동일하지 않다.
ShadowMap2D, 환경 profile의 원본 활성 상태, 받침대 모델 연결도 별도 미완료다.
화면을 맞추려고 전체 수면을 내리거나 색을 임의 도색하는 보정은 적용하지 않았다.

## G04. 지형의 경사 기반 강제 절벽 재질 분리 제거 — 적용

후속 요청에서 `write_component_gltf`를 조사했다. 기존 변환기는 면 경사만으로
삼각형을 `CLIFF_MATERIAL_NAME`에 보내고 별도 world-height UV를 만들었다.
따라서 앞서 source layer contract로 굽던 diffuse/normal 결과는 그 면에서 소비되지
않았다. 레이어 bake에서 임의 slope blend를 끈 것만으로 geometry의 별도 분기는
제거되지 않은 상태였다. 흰색 경사면과 레이어 단절의 확인된 경로이며, 노랑/초록
색 전체나 모든 침수 인상의 단일 원인으로 주장하지 않는다.

`write_component_gltf(..., source_painted_surface=True)`는 원본 component UV와
동일한 baked material을 모든 면에서 사용한다. 기존 호출의 기본 동작은 유지한다.
새 실제 소비자 `Tools/LandscapeExtractor/restore_maharaka_painted_surface.py`는
마하라카의 현재 catalog에 등록된 16개 component만 선택한다. 이전 native-layer bake
보고서의 shader-map key와 설치 texture SHA를 검사하고 원본 높이/weight를 다시 읽는다.
모델은 기존 ModelAssetConverter와 CModel/CMaterial 경로로 재생성했다.

- 실제 Resources에 WModel 16개 교체 완료. PNG, 물 높이, placement, catalog, rendering profile은 불변.
- 변환 전후 123,008개 삼각형을 정점 위치(0.001 source cm 반올림)와 winding을 포함한 multiset으로 비교: 동일.
- component별 7,688개 삼각형 / 구멍 0개, source-painted material 연결 확인.
- 경사면도 component UV를 사용하며 임의 별도 cliff primitive는 0개.
- 이전 모델: `out/MaharakaMapRestoration20260927/painted-surface/before/`
- 파일별 기록: `out/MaharakaMapRestoration20260927/painted-surface/receipt.json`
- 배포 대상은 기록의 `modelAssetId` 16개. 기존 92개 DDS 배포 항목에 추가된다.

이번 G04에는 C++/shader 변경이 없으므로 새 Client 링크는 필요하지 않다. 이미 실행한
Client는 종료 후 다시 입장해야 교체된 WModel을 읽는다. 게시 문서가 바뀌지 않았으므로
Publish는 하지 않았고 변경 후 Area Validate와 Check는 모두 PASS(4,669 placements).
Landscape 테스트 35건 PASS에는 새 경사면 UV 유지, legacy 동작 유지, 구멍 보존 3건이
포함된다. 사용자 화면 확인은 아직 없으며 visual PASS로 기록하지 않는다.

### 받침대 추가 탐색 및 남은 경계

초기 번호대 ITR_00*/ITR_01* 원본 패키지 1,580개도 추가 검색했다. 큰 세로 bounds 후보
317개를 분류했다. 5개 패키지는 현재 파서가 압축 chunk 0 형식을 거부했으므로 전부
정상 해석했다고 기록하지 않는다. 기존 Prop 테이블 설명과 조인해도 570954/570987의
모델 연결은 확정되지 않았다. 비슷한 나무/기둥 모델을 원본이라고 설치하지 않았다.

G04 당시 **모코모코 받침대와 중앙 기둥은 미적용**이었다. 이후 사용자가 사진 기준
재구성을 승인하여 G05에서 적용했다. 원본 연결 복원과 사진 기반 재구성은 구분한다.
8-bit terrain bake의 HDR 손실, Landscape native lighting, 수면/바닥의 최종 육안 일치도
별도 미완료다. 이 G04를 맵 완전 복원이나 침수/노랑/초록 전체 해결로 보고하지 않는다.

## G05. 사용자 승인 사진 기준 받침 구조물 재구성 — 게시 완료

사용자는 원본 목재·기둥 자산을 조합하는 제안에 `이것도 바로 진행해줘`로 승인했다.
기존 추출 WModel과 원본 표면 재질을 재사용하되 배치·크기는 사진 기준 재구성이다.
원본 Prop 570954/570987 모델 연결을 찾았다고 기록하지 않는다.

### 실제 적용

- 큰 모코모코: 세로 기둥 4개, 가로보 12개, 대각 버팀대 8개, 상판 목재 11개 = 35개.
- 중앙 모코모코: 지지 기둥 1개. 합계 36개 overlay placement 추가.
- 큰 구조물 중심 X/Z = `(75.179, -1007.262)`, 기둥 하단 Y 19.7, 상판 윗면 Y 24.4.
  기둥 간격 3.6 m, 상판 약 4.5 × 4.4 m. 중앙 기둥 Y 22.5~23.65, 폭 0.42 m.
- 설치된 NPC idle 정점의 최하단 오프셋을 반영하여 큰 NPC Y는
  `19.904 -> 24.46260592`, 중앙 NPC Y는 `22.36 -> 23.66841349`로 변경.
  X/Z, yaw, 모델, 애니메이션, behavior와 플레이어 spawn 4개는 그대로다.
- Server의 stationary NPC 배치 경로가 정본 Y를 소비하며 Client만 이동시키는 우회는 없다.
- catalog 867 -> 870, placement 4,669 -> 4,705, material 947 -> 950.
  추가 전 모든 행을 그대로 보존했고 placementLighting 전체도 불변임을 비교했다.

### 사용 자산과 재구성 표식

| 새 stable asset ID | 재사용 원본 asset ID |
|---|---|
| `MAP_MHP_RECON_SUPPORT_COLUMN` | `MAP_45CFCC4B82EE_BG_PAP_NIATOWN_COLUMN01_SM` |
| `MAP_MHP_RECON_SUPPORT_TIMBER` | `MAP_7E328EAB6CCD_BG_PAP_NIATOWN_FENCE01B_SM` |
| `MAP_MHP_RECON_CENTRAL_PILLAR` | `MAP_12CD7B3924ED_BG_BER_RANIAT_PILLAR04_SM_PSY_OVR_6109C9D9F804` |

물리 경로는 각각 `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP/<원본 asset ID>/<원본 asset ID>.wmodel`이다.
Resources-relative ID는 위 경로에서 `Client/Bin/Resources/`를 뺀 값이다.
새 variant는 이 경로를 그대로 참조한다. 별도 리소스 파일을 생성하거나 기존 메시를 교체하지 않았다.
material은 원본의 표면 정의를 복제하되 이전 위치에 구운 `bakedLighting`은 제거했다.
기존 위치 RNM을 새 구조물에 잘못 씌우지 않는다. 새 RNM을 굽거나 원본 조명을 복구한 것은 아니다.
placement source ID는 모두 `reconstruction.maharaka.*`, catalog 설명에도
`User-approved photo reconstruction ... not original placement`를 기록했다.

변경 정본은 Area Imported catalog, Authoring placements/materials, World Gameplay 4개 문서다.
recipe 및 변경 전 보존본은 `out/MaharakaMapRestoration20260927/supports/`에 있다.
`recipe.json`에 모든 stable ID, 원본 자산, 부재 양끝 좌표, 폭·두께, transform이 들어 있다.
기존 CModel/CMaterial 경로를 사용하며 새 C++/셰이더/프로젝트 등록 변경은 없다.

### 게시·자동 검증

- `Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Validate/Publish/Check`: 모두 PASS, 4,705 placements.
- `Publish-WorldGameplay.ps1 -WorldId MAHARAKA -Mode Validate/Publish`: 모두 PASS, 6 placements.
- `Tools/MapPipeline/test_maharaka_reconstructed_supports.py`: 5 tests PASS.
  stable ID/유효 transform, 원본 재질 재사용·잘못된 RNM 제거, 실제 WModel 정점의 기둥/상판 높이,
  Server 게시 위치 일치, Client 게시 배치 일치를 검사한다.
- 기존 catalog 867행, placements 4,669행, materials 947행 보존 비교 PASS.
- `git diff --check`: PASS(줄끝 안내만 있음).
- 첫 비상승 Map Check는 sandbox의 Python 접근 제한으로 실패했고, 동일 명령을
  허용된 권한으로 다시 실행하여 PASS했다. 데이터 오류를 숨긴 것이 아니다.

이 G05는 데이터 변경이므로 추가 Client 빌드는 필요 없다. 기존 G01의 Debug 빌드는 유지한다.
검사 시 Client/Server 프로세스는 발견되지 않았으며 에이전트가 실행하지 않았다.
사용자는 실제 저장소의 Server + Client를 새로 시작하고 Lobby -> Maharaka로 입장한다.
Server 재시작은 두 NPC의 새 높이를 읽기 위해 필요하다. 구조물 비율/외형의 최종 육안 확인은
사용자가 해야 하며 아직 visual PASS가 아니다. 노랑/초록 지형 색과 침수 인상의 해결을
G05의 완료에 포함하지 않는다.

## G06. 지형 타일의 이중 Z축 변환으로 육지가 수면 아래에 나타남 — 수정·설치

### 원인과 실제 소비자

사용자가 제공한 `스크린샷 2026-09-27 144804.png`에서 수영장 밖 테이블 주변까지
바다가 드러나는 것을 확인했다. `component_positions`는 원본 UE `(X,Y,Z)`를 이미
Client `(X,Z,-Y)`로 바꾼 local 정점을 만든다. 그러나 `write_component_gltf`는 이
왼손 좌표를 오른손 좌표인 glTF에 그대로 기록했고, 설치된 ModelAssetConverter는
glTF를 읽으며 Z와 winding을 변환했다. glTF/WModel의 실제 float 정점 비교로
`(0,20.48,-0.64)m -> (0,2048,+64)cm`의 잘못된 경계를 확인했다.
Loader와 MapTool의 CModel pretransform은 `(0.01,0.01,0.01)`이므로 이를 되돌리지 않는다.

그 결과 component anchor는 맞지만 지형은 반대쪽으로 펼쳐졌다. 39.68m 폭의 타일
최외곽 정점은 원래 위치에서 79.36m 떨어질 수 있다. 평지 대신 다른 조각의 낮은 높이가
테이블 아래로 들어오며 바다가 드러났다. 모래/노랑/초록 레이어도 그 잘못된 조각 위치에
따라갔다. 물의 높이나 조명을 낮춰 해결할 문제가 아니다.

기존 G04의 123,008개 삼각형 보존 검사는 **잘못된 이전 모델과 같은지**만 확인했다.
따라서 source glTF의 좌표 설명과 최종 cooked consumer가 일치하는지는 놓쳤다.
그 검사를 원본 좌표 검증으로 확대 해석하지 않는다.

### 수정

- glTF 직렬화 경계에서 position/normal/tangent Z를 반사하고 tangent W와 삼각형
  winding도 같이 변환한다. 원본 높이·component UV·painted layer·hole 값은 유지한다.
- glTF는 `UE X,Z,Y` 오른손 meter 좌표를 소유하며, 기존 converter를 지난 WModel이
  `UE X,Z,-Y` 왼손 cm 좌표를 갖는다. shader/runtime에 보정 분기를 추가하지 않는다.
- `restore_maharaka_painted_surface.py --restore-coordinate-basis`로 현재 마하라카
  catalog의 지형 16개만 다시 변환해 실제 Resources에 설치했다.
- 범용 exporter 수정은 이후 재추출에도 적용되지만 베른 등 다른 맵의 설치 모델은
  재생성하거나 교체하지 않았다. 다른 맵의 수동 보정 배치는 별도 원본 대조 대상이다.
- 새 소비자 검증은 최종 WModel 정점의 UV로 원본 grid identity를 되찾아 위치·법선·
  tangent·triangle winding을 비교한다. placement anchor/identity scale, 원본 collision
  heightfield, component seam도 검사한다. 이전 모델과의 동일성만으로 통과시키지 않는다.

### 같은 월드 위치에서 높이 비교

| 원본 테이블 placement | 테이블 원점 Y | 수정 전 지형 Y | 수정 후 지형 Y |
|---|---:|---:|---:|
| `SL01:export:5954` | 20.48 | 15.36 | 20.48 |
| `SL01:export:5955` | 20.48 | 15.36 | 20.48 |
| `SL01:export:6616` | 20.40 | 15.0189 | 20.48 |
| `SL01:export:6646` | 20.48 | 16.5968 | 20.48 |
| `SL01:export:6651` | 20.48 | 15.36 | 20.48 |

위 ID의 prefix는 `LV_OCN_EVENTIS_MHP_`다. 값은 실제 WModel 높이를 월드 X/Z에서
삼각형 보간하여 구했다. 바다는 원본 Y 19.6299988을 유지하므로 복구된 평지 Y20.48은
수면 위다. 중앙 `(75.05,-984.32)`의 지형은 전후 모두 Y19.84이며 수영장 바닥은 유지된다.
침수를 고치려고 수면 메시를 숨기거나 전역 물 높이를 내리지 않았다.

### 설치 및 검사

- 교체 WModel: 16개, 실제 Resources SHA와 후보 일치 16/16.
- 최종 정점 63,504개 / triangle 123,008개를 원본 기준으로 검사.
- 최종 위치 최대 오차 0.000002442m 미만, packed normal 최대 오차 0.00000003 미만.
- collision height 샘플 63,504개 불일치 0.
- 인접 경계 24쌍, 1,512 height/packed-normal 샘플 불일치 0.
- Landscape unit 37건 PASS. 신규 2건은 top/cliff 양쪽의 glTF basis·winding·bounds를 검사.
- Area publisher Validate / Check PASS(4,705 placements). JSON/정본 게시물 변경이 없어 재게시 불필요.
- C++와 shader를 바꾸지 않아 추가 Client 빌드 불필요. 실행 중 Client는 기존 모델을 들고
  있으므로 저장할 편집값이 있다면 저장한 뒤 Client를 재시작하여 다시 입장한다.
- 설치 기록: `out/MaharakaMapRestoration20260927/coordinate-basis/receipt.json`.
- 교체 전 16개 백업: 같은 폴더의 `before/`. 덮어쓴 원본이 아니라 직전 파생 WModel 보존본이다.
- 이번 교체에서는 PNG/지형 높이 원본/수면/정본 placement/NPC/팀 rendering option을 변경하지 않았다.

직접 Client 실행이나 화면 캡처는 하지 않았다. 새 screenshot 이전에는 시각적 일치 PASS를
기록하지 않는다. 8-bit terrain 색의 HDR 손실과 Landscape native lighting 등은 이 좌표
수정과 별개다. 이번 적용을 맵 전체의 색·조명 복원 완료로 표현하지 않는다.

## G07. 큰 모코모코 원본 구조물 참조 재조사 — 발견/후보, 미게시

### 이전 판단 정정

G05의 목재 35개는 원본 받침 메시가 아니다. 사용자 승인하에 만든 사진 기반 재구성이다.
후속 조사에서 원본 `ITR_02453.mesh.itr_02453_sk`와 큰 모코모코의 씬 참조를 확인했다.
이 자산은 기존 설치 catalog에 없었지만, 이전 SCENE04A 추출 placement에는 이미 참조가 있었다.
배치 개수가 1개라는 이유로 어트랙션 가능성을 낮게 보거나 `_sk` 이름으로 skeletal을 추측한
09-26 보고는 잘못된 검색 판단이므로 해당 문서도 교정했다. 실제 UPK class는 `StaticMesh`다.

이번 발견은 아래의 원본 object reference와 좌표 일치에 근거한다. 비슷한 이름/사진만으로
선택한 것이 아니다. `570954/570987`의 모델 연결을 찾아냈다고 바꿔 쓰지도 않는다.

### 원본 연결

아래 package index는 UE3의 1-based object reference다. placement ID의 export 번호는 0-based다.

| 원본 위치 | 실제 참조 |
|---|---|
| `LV_OCN_EVENTIS_MHP_SCENE04A`, actor 55 | `efmotionstaticmeshactor_0 -> staticmeshcomponent 613` |
| component 613 | import -123 -> `itr_02453.mesh.itr_02453_sk` |
| actor 71 | `efskeletalmeshactorlookinfomat_2`, LookInfo `EFDLChar_MN_ISMP_00.MN_ISMP_00` |
| actor 71 component 583 | `mn_ismp_00.mesh.mn_ismp_00_sk`, original AnimSet and 3 MICs |
| SeqVar_Object 507 / 508 | actor 71 / actor 55 |
| SeqAct_ToggleHidden 403, Target | `[507,508]`: 모코모코와 구조물을 같은 표시/숨김 명령으로 제어 |

두 actor는 원본 XY `(7517.85205078125, 100726.1640625)` cm 및 yaw -90°가 같다.
구조물 Z는 `1990.4390869140625` cm, 씬 모코모코 Z는 `1991.6051025390625` cm다.
source placement ID는 `LV_OCN_EVENTIS_MHP_SCENE04A:export:612`다.
기존 공용 좌표 변환 결과는 position `(75.1785205078,19.9043908691,-1007.261640625)` m,
quaternion `(0,-0.7071067812,0,0.7071067812)`, scale `(1,1,1)`이다.

메시 물리 패키지는 설치본의
`C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages/GL70PYCQXJDRYTKA0DNH7K.upk`다.
`ITR_02453`의 export 8(0-based)이 `mesh.itr_02453_sk`다. 단일 메시 안에 4개 material slot이 있다.
원본 actor 높이가 거의 같다는 사실을 최종 gameplay NPC의 상판 접촉 높이 증명으로 쓰지 않는다.
기존 G05의 head Y=24.46260592 역시 사진 재구성 높이이므로 원본값으로 재분류하지 않는다.

### 실행한 추출·검사

- 현재 LPK 인덱스의 XML payload 107,483개/865,418,073 bytes 검색, 추출 오류 0.
  `MN_ISMP_00` 참조 22파일은 LookInfo/Action/Projectile 후속 조사 입력이다.
  해당 검색에서 특정 숫자 ID가 없었다는 결과는 전체 원본 자산 삭제 증거가 아니다.
- UModel 비대화형 export 및 Bern 공용 `export_one -> cook_one -> preserve_cooked_geometry` 실행.
  candidate asset ID `MAP_65096D72C5C9_ITR_02453_SK`.
- WModel 후보 30,621 vertices / 31,350 triangles / 4 submeshes.
  source glTF와 변환 후 위치·법선·접선·UV·topology 대응 검사 통과.
  glTF 자체의 전체 UPK 채널 무손실 여부나 화면 일치까지 증명한 것은 아니다.
- `itr_02453.mat.itr_02453_01_mi`~`04_mi`의 부모/상수/텍스처 및 원본 shader cache 추출.
  01~03은 monster 계열, 04는 `sk_foliage_msk`다. 일반 BG 목재 셰이더로 치환하지 않았다.
- LocalVertexFactory의 Base/Light PS를 별도로 추출해 GPU-skin PS와 4/4 문서 일치를 확인.
  원본 VS도 별도로 추출했다. VS layout을 PS 이름으로 추측하지 않았다.
- 텍스처 10개, 저장된 원본 mip 총 108개를 추출했다. mip0 compressed block 일치 및
  모든 원본 package 무변경 검사 통과. 필터링/재압축/색 보정하지 않았다.
- native 함수와 Configure 4세트를 후보로 생성했다. 1528~1531은 후보 번호일 뿐,
  Engine registry/Client 소스에 설치하거나 번호를 예약하지 않았다.
- 원본 actor/component/LookInfo/ToggleHidden/좌표 일치 assertion과 mip hash 검사 통과.

### 아직 적용하지 않은 경계

**현재 화면의 35개 재구성물은 그대로다. 이번 조사를 교체 완료/원본 복원 완료로 보고하지 않는다.**

1. 원본 4개의 PS/Configure 및 LocalVertexFactory varying을 기존 CModel/CMaterial 경로에 연결해야 한다.
   현재 등록된 프로그램과 정확히 같은 것으로 확인되지 않았다. 임의 family 재사용은 금지한다.
2. 04번 잎 장식은 원본 VS `86a842b8c698dc4c911f97cc554ba7b8`의 wind 변형을 사용한다.
   현재 `source_foliage_wind.PROGRAMS`의 3종에 없는 프로그램이다. source vertex color와
   wind uniform/owner/bounds 입력을 확인·연결해야 한다. export glTF에 COLOR_0이 없다는 이유로
   원본 color가 없거나 wind가 꺼져 있다고 단정하지 않는다.
3. 원본 씬 배우 좌표와 gameplay actor의 최종 높이 관계를 확인해야 한다. 기존 사진 기준
   offset을 새 원본 상판에 재사용하거나 메시 전체 높이를 접촉 높이로 삼지 않는다.
4. 위 검증 후 큰 모코모코 재구성 placement 35개만 원본 placement로 교체한다.
   별개인 중앙 기둥, 지형, NPC 수량, 사용자 rendering 설정은 함께 지우지 않는다.
5. 필요한 Debug build, Area/World scoped publisher 및 사용자 화면 확인은 아직 수행하지 않았다.

증거/재개 후보는 실제 저장소의
`out/MaharakaMapRestoration20260927/original-stand/source-audit/`에 보존했다.
`reference-closure/audit.json`은 `SOURCE_CLOSURE_AND_STAGED_GEOMETRY_VERIFIED_NOT_INSTALLED` 상태다.
`reference-closure/scene04.json`, `native-static/`, `stand-vertex-evidence.json`, `mips/`, `generated/`와
`original-stand/source/`, `original-stand/runtime/`을 함께 보존했다.
동봉 스크립트는 조사 당시 scratch 경로를 사용하므로 재실행할 때 경로를 확인한다.

이번 단계는 저작/게시/Resources runtime을 변경하지 않았다. UI 실행·캡처·visual PASS도 없다.

## G08. 사용자 승인 NPC 30명 랜덤 배치 — 저작/게시 완료

사용자 요청은 `30마리만 랜덤으로 배치해봐 그리고 어트렉션 안내원들은 포함시키고`다.
앞서 제안한 기존 모델 활용 재구성을 승인한 것으로 처리했다. 신규 30명을 더한 것이 아니라,
기존 큰 모코모코/워터캐논 2개를 포함한 총 30개 NPC다. 플레이어 spawn 4개는 불변이다.
이번 변경은 NPC 배치만이며 G07 원본 받침 후보의 설치, 이벤트 기능, 지형/렌더링 변경은 포함하지 않는다.

### 배치 기준과 역할

- 안내원 5명은 아래 행사 접근 지점에 고정하고 방문객 23명은 seed `20260927`로 한 번만 무작위 선정했다.
  게임 실행 때마다 재추첨하지 않는다. MapTool에서 각 placement를 그대로 편집할 수 있다.
- 후보 위치는 `out/MaharakaFunctions20260926/deploy_57009.json`의 NPC actor 좌표다.
  `UE (X,Y,Z) cm -> Client (X,Z,-Y) m`로 변환하고 Y는 설치된 지형 삼각형에 맞췄다.
  원본 배치 위치를 쓴 사실이 그 위치의 NPC 신원/외형/역할까지 원본이라는 뜻은 아니다.
- 현재 navigation authoring은 uniform grid라 물/지상을 구별하지 못한다. 이를 안전 판정으로 쓰지 않고
  최종 Resources 지형 16개와 Water 배치의 삼각형 높이를 실제 placement transform으로 비교했다.
  중심과 X/Z ±0.6m 지점 모두 지면 차이 0.12m 이하, 수면 위 0.25m 이상인 후보만 사용했다.
- 기존 NPC/플레이어 시작점과 최소 1.5m, 방문객 선정 시 이미 고른 지점과 최소 2.5m 간격을 요구했다.
  이 검사는 지형/수면/인물 간격 검사이며 모든 장식물과의 정밀 충돌 또는 육안 검사 통과는 아니다.
- `npc.maharaka.reconstructed.*` ID로 원본 두 NPC와 구분한다. 방문객 ID는 좌표 출처 actor 번호도 남긴다.
  기존 `NpcCatalog.json`의 지원되는 소형 인간 모델 28개와 공유 애니셋 4개를 재사용한다.
  30개 archetype ID가 서로 다르지만 외형이 모두 완전히 다르다는 판정은 하지 않았다.

| 안내 역할 | stable ID 끝부분 | 재사용 archetype | 위치 X,Y,Z(m) | 좌표 원본 actor |
|---|---|---|---|---:|
| 바이킹 레스토랑 접근부 | `guide.viking-restaurant` | `NPC_FORMAN` | 67.393,20.48,-964.99 | 133 |
| 모코모코 접근부 | `guide.mokomoko` | `NPC_AYLARA` | 74.638,20.48,-1016.558 | 76 |
| 레인보우 익스프레스 접근부 | `guide.rainbow-express` | `NPC_11592` | 99.748,20.48,-986.858 | 168 |
| 워터팡 접근부 | `guide.waterpang` | `NPC_SCHMIDT` | 52.195,20.48,-981.059 | 159 |
| 레이싱 입구 | `guide.racing-entry` | `NPC_11749` | 76.694,20.48,-1028.98 | 69 |

위 역할은 이번 저작 분류다. 원본 안내원 NPC ID를 복구했다는 의미가 아니고, 클릭 대화/행사 시작이나
이름표를 추가한 것도 아니다. `behavior: null`, `idleClip: null`로 기존 stationary/default idle을 쓴다.
추가 순찰/배회 AI와 프레임별 랜덤 배치는 없다. 새 body 파일 합계 11.762MiB는 이미 설치된 파일의 크기이며
텍스처·GPU/CPU 비용이나 실행 FPS 측정값이 아니다. 렉이 없다고 보장하지 않는다.

### 적용 파일과 검증

- `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json`: revision 3 -> 4, 전체 6 -> 34 placements.
  기존 6개는 보존했고 28개만 추가했다. NpcCatalog와 Resources는 수정하지 않았다.
- 공식 World publisher의 `-WorldId MAHARAKA -Mode Validate` 및 `-Mode Publish`: PASS, 34 placements.
  실제 변경 출력은 `Server/Bin/DataFiles/World/MAHARAKA.worldbootstrap`이다.
  Client `MAHARAKA.npcpresentation.json`은 placement별 clip override가 없어 기존 빈 entries를 유지한다.
  빈 entries는 NPC 누락이 아니다. NPC 생성은 Server world entity, 모델/기본 clip은 catalog가 소유한다.
- `Tools/WorldPipeline/test_maharaka_npc_population.py`: 4 tests PASS.
  30명 제한·5개 역할·원래 spawn/두 모코모코 위치·dry ground·간격·게시 위치 일치를 검사한다.
  추가 모델 28개 및 donor 4개의 WModel을 읽고 Engine WSkeletonReader 방식의 skeleton hash/본 수,
  기본 idle 존재와 body/donor clip 이름 충돌 부재를 검사했다.
- 기존 `Tools/MapPipeline/test_maharaka_reconstructed_supports.py`: 5 tests PASS.
- C++/셰이더/프로젝트 등록 변경 없음. 이번 데이터 배치에 새 빌드는 필요하지 않다.
- 새 Resources를 만들지 않았으므로 이번 추가로 생긴 Drive 배포 파일은 없다.
  다른 PC는 NpcCatalog가 참조하는 기존 `Client/Bin/Resources/Character/NPC`가 있어야 한다.

### 사용자 확인

검사 당시 Client/Server는 실행 중이지 않았다. 에이전트가 실행하지 않았다.
실제 저장소의 Server + Client를 새로 시작하고 Lobby -> Maharaka로 들어간다.
World bootstrap은 서버 시작 시 읽으므로 이미 떠 있는 별도 팀 서버에는 파일 전달 및 재시작이 필요하다.
MapTool -> World Gameplay에서 `npc.maharaka.reconstructed.guide.*`를 찾아 안내원 위치를 편집할 수 있다.
실제 화면의 발 접촉, 주변 장식물 겹침, 안내 지점 가독성 및 프레임 영향은 사용자 육안 확인 대기다.
이번 NPC 배치 완료를 맵 전체 복원/원본 받침 교체/이벤트 기능 완료로 보고하지 않는다.

## G09. 원본 전체 참조 재대조와 누락 재질 1차 복구

사용자는 G07의 실제 원본 받침 발견을 근거로 마하라카 전체를 다시 대조하고 누락을 복구하도록
요청했다. 이번 G09는 **전체 목록 대조 및 첫 재질 복구분**이다. 전체 맵 복원이 끝난 상태는 아니다.
G07 받침의 런타임 교체, 조건부 배우/이펙트/사운드 연결까지 완료했다는 의미도 아니다.

### 조사 경계와 실제 확인

- `Tools/LevelPlacementExtractor/audit_maharaka_source_coverage.py`를 추가했다.
  PS의 단 하나의 WorldInfo가 소유하는 `streaminglevels` 참조 14개를 따라 PS 포함 15레벨을 검사한다.
  독립된 streaming export를 이름만 보고 포함하지 않으며, `eflevelstreamingalwaysloaded`와
  `levelstreamingkismet`을 구별한다. 추가/누락 입력 레벨은 실패한다.
- 15개 source placement 문서가 지시하는 물리 UPK의 SHA256을 현재 원본과 비교했다.
  export 43,094개를 분류했고 static placement 8,194개를 실제 저작 배치의 sourcePlacementId와 조인했다.
- static 3,841개는 현재 설치 배치와 위치/회전/스케일이 일치한다. q와 -q는 같은 회전으로 검사한다.
  이것은 재질·그림자·애니메이션·화면 동일성 PASS가 아니다.
- static ID가 현재 Map placement에 없는 항목은 4,353개다. 이를 4,353개의 새 배경 물체로 설치하면 안 된다.
  source actor의 base/baseSkelComponent/baseBoneName/relative transform도 함께 보존했다.
- 현재 추가 864개는 foliage ID 812개, Landscape 16개, 승인된 사진 재구성 36개다.
  static 원장에 없다는 이유로 삭제하지 않았다. 이번 원장은 foliage 각 인스턴스의 원본 채널/재질을
  독립 검증한 결과가 아니며, 해당 항목은 후속 비교 대상이다.
- source object import는 전체 package object path를 보존한다. 짧은 leaf name을 근거로 자산을 치환하지 않는다.

| static Map ID 미설치 구분 | 개수 | 확인 상태 |
|---|---:|---|
| PS | 243 | 원본 visibility가 false. culling helper 242개와 별도 pool mesh 1개. 상시 배경 추가 금지 |
| LAND01 | 80 | 이전 섬 범위 밖의 배치. 레이싱 범위와 함께 검토 |
| SL02 | 3,934 | 이전 섬 범위 필터가 제외한 레이싱 쪽 배치. 삭제된 원본이 아님 |
| STANDARD_TRACK | 19 | 19개 모두 원본 base 연결 있음. 무조건 정적 배치하면 안 됨 |
| SCENE03B | 42 | 42개 모두 base 연결을 가진 효과 메시. 현재 무대 조각 18개 설치와 별개 |
| SCENE04A | 1 | G07의 실제 `itr_02453.mesh.itr_02453_sk` 받침 |
| SCENE06A | 2 | 1개는 원본 `bip001` 부착. 월드 좌표만 복사하면 연결 손실 |
| SCENE07A | 29 | 이 중 2개는 base 연결. 나머지도 Matinee/표시 조건 확인 필요 |
| SL01 | 3 | `bg_tot_movillage_a.mesh.bg_tot_movillage_decoprop07f_sm_artree`의 원본 배치 |

SL01 세 배치는 `5630/5982/7602`다. 기존 `admission.exclusions.json`은 glTF normal/tangent가
parallel 또는 degenerate여서 제외했다고 기록하고 있다. 원본 모델이 없다는 뜻이 아니다.
검증을 끄거나 법선을 임의로 재생성해 완료로 처리하지 않았다.

또한 source export에 skeletal actor 68개, LookInfo skeletal actor 112개, emitter 748개,
AnimControl track 531개, ambient sound actor 95개, music volume 6개가 있다.
이는 상시 동시 활성 수나 실제로 추가해야 할 NPC 수가 아니다. 승인된 30명 NPC를 늘리지 않았다.
generic property parser 오류 174개 및 native tail은 명시적으로 남겼다. 오류를 원본 부재로 승격하지 않는다.

### 실제 저작/게시 수정: 5개 모델의 누락 재질 9슬롯

`Tools/LevelPlacementExtractor/restore_maharaka_missing_material_slots.py`를 추가했다.
원본 explicit material override의 **전체 MIC 경로와 슬롯 번호**를 대조하고, 실제 설치 WModel의
해당 슬롯 이름을 읽어 연결한다. 같은 원본 MIC를 사용하는 기존 admitted surface가 모두 동일할 때만
재사용한다. donor 사이에 값 차이가 있으면 사용자 튜닝인지 구분할 수 없으므로 거부한다.

| 실제 모델 | 복구 슬롯 수 |
|---|---:|
| `BG_PAP_NIATOWN_HOUSE03_SM_MSJ_OVR_4847A14960DB` | 2 |
| `BG_PAP_ETC_FRUITBASKET02_SM_OVR_2E27CDFEFB29` | 1 |
| `BG_LUT_LUCASTLE_HOUSEINSTAIR02A_SM_MSJ_OVR_975B41755FA7` | 1 |
| `LV_OCN_PEYTO_SHIP15_SM_OVR_159469611338` | 4 |
| `LV_ATT_STERN_FENCE06_SM_OVR_E01ED32C11B7` | 1 |

- 실제 영향 범위는 46개 placement, 55개 placement-slot 참조다. 새로운 모델 46개를 생성한 것이 아니다.
- 재질 행은 950 -> 959. 기존 950행과 placementLighting 및 나머지 모든 JSON field는 의미적으로 동일함을 검사했다.
  원래 JSON 줄바꿈/공백도 유지해 전체 파일 재포맷을 방지한다.
- donor의 bakedLighting은 절대 복사하지 않는다. water/native/wind/다른 render contract는 이번 도구가 거부한다.
  원본 packageIndex 0은 부모 재질 해석 대상이지, 임의 회색/임의 비슷한 MIC로 바꾸지 않는다.
- 설치 Resources의 실제 파일 존재, cooked material slot, 현재 source/placement identity를 확인하고,
  원본/저작/리소스가 후보 생성 중 변경되면 쓰기를 거부한다. 기존 override는 덮지 않는다.
- 이번 연결은 기존 admitted MIC surface를 정확한 누락 슬롯에 재사용한 것이다.
  해당 surface 자체의 모든 native PS/VS 수학을 이번 단계에서 다시 독립 검증했다는 뜻은 아니다.

변경 후 explicit override 참조 검사는 일치 2,563 / 슬롯 미해결 788 / null override 부모 해석 대상 214다.
이 수치는 placement-slot 횟수이며 고유 모델/고유 MIC 수가 아니다. 원본 default mesh material 전수 해석률도 아니다.
같은 슬롯으로 정확하게 조인된 명시적 참조 중 sourceMaterial 차이는 0건이다.

### 검증과 산출물

- 신규 원본 감사 unit test 5개 PASS: WorldInfo closure, 중복/없는 참조, null override, full-path/slot, bone attachment.
- 신규 재질 복구 unit test 5개 PASS: 동일 leaf의 다른 package 거부, 조명 분리, 상충 튜닝 거부,
  water/wind 별도 계약, 기존 JSON 내용/서식 보존.
- 기존 NPC population 4개 및 reconstruction supports 5개 PASS. NPC 30명과 기존 재구성물은 보존됐다.
- 공식 Area publisher Validate / Publish / Check PASS, 4,705 placements.
- 재실행 시 추가 0행으로 중복 생성이 없음을 확인했다. `git diff --check`도 PASS다.
  최초 비승격 diff 검사는 Git LFS 임시 디렉터리 접근 제한으로 실패했고, 권한을 확보한 재검사에서 통과했다.
- authoring/runtime mapmaterials만 이번 데이터 복구로 변경했다. catalog, placement, WorldGameplay, 조명,
  scene profile, shader, C++는 G09에서 변경하지 않았다. 새 C++ 빌드나 추가 Resources 배포는 필요 없다.
- 원본 감사: `out/MaharakaMapRestoration20260927/full-source-audit/coverage.json` 및 레벨별 objects/static-comparison.
- 첫 복구 전 저장본·후보·정확한 9개 연결 목록:
  `out/MaharakaMapRestoration20260927/missing-material-bindings/{before.mapmaterials.json,candidate.mapmaterials.json,report.json}`.
  out은 조사/복구 증거이며 제품 런타임 정본이 아니다.

### 아직 남은 복원

1. G07 원본 받침: 4개 native 재질, 잎 VS 입력/vertex color, 실제 머리 연출 높이를 연결한 뒤
   재구성 35개를 교체해야 한다. **이번에도 받침 교체 완료는 아니다.**
2. SL01 세 메시의 native tangent/normal 근거와 변환을 복구해야 한다.
3. SCENE03B 효과 42개, SCENE06A/07A 소품, STANDARD_TRACK 배우 부착·움직임·표시 조건을
   기존 World presentation/sequence 소비자에 연결해야 한다. 정적 배치로 위장하지 않는다.
4. skeletal/particle/audio의 원본 객체 참조 및 실제 활성 조건, 미해결 재질/default 슬롯,
   foliage/RNM/ShadowMap/지형·수면을 각각 계속 대조해야 한다.
5. 레이싱 범위는 별도 로드/진입 범위를 가진 원본으로 검토해야 한다. 수천 개를 현재 섬에 상시 로드하지 않았다.

검사 시 Client/Server는 실행 중이지 않았다. UI 실행·캡처·visual PASS는 하지 않았다.
사용자 확인 경로는 실제 저장소의 Client -> Lobby -> Maharaka 재진입이다.
현재 첫 재질 복구의 화면 확인만 가능하며 전체 원본 복원 검증으로 해석하지 않는다.

## G10. 원본 모코모코 받침대 실제 교체

### 반영한 데이터와 소비자

- G07/G09의 미설치 상태를 변경했다. 사진 기반 `reconstruction.maharaka.mokomoko.*` 35개를
  제거하고 `itr_02453.mesh.itr_02453_sk` 원본 StaticMesh 1개를 설치·게시했다.
  원본 sourcePlacementId는 `LV_OCN_EVENTIS_MHP_SCENE04A:export:612`이며,
  기존 제품 섬 로드 범위인 `LV_OCN_EVENTIS_MHP_SL01`에 명시적으로 승격했다.
  원본 시네마틱의 전체 ToggleHidden/애니메이션 타임라인을 복원한 것은 아니다.
- 원본 좌표 `[75.17852050781251,19.904390869140624,-1007.261640625]`,
  quaternion `[0,-0.7071067811865475,0,0.7071067811865476]`, scale 1을 보존했다.
- CModel -> CMaterial 경로에 원본 native Base/Light 및 CPU 상수 프로그램 1528~1531을 등록했다.
  실제 WModel 슬롯은 **04, 01, 02, 03** 순서다. leaf 이름 유사성으로 다른 재질을 대신 넣지 않았다.
  네 슬롯 모두 정확한 원본 MIC, Texture expression index, sRGB/linear, draw state를 연결한다.
- 원본 DDS 10개의 mip 108개를 그대로 설치했다. foliage 부모의 `flat_white`는 실제 추출 PNG의
  2x2 RGBA 전 픽셀과 기존 EngineDefaults DDS가 일치함을 확인해 DDS 계약으로 연결했다.
  PNG를 새 DDS인 것처럼 확장자만 바꾸지 않았다.
- LocalVertexFactory Base/Light 출력 레지스터를 기존 SourceMovieStatic 어댑터에 정확히 추가했다.
  잎 VS `86a842b8c698dc4c911f97cc554ba7b8`의 변위 명령 28~111을 literal 변환해
  정적 메시 VS의 program 1531 분기에 연결했다. MIC의 실제 풍속·빈도·강도와 source bounds를 사용한다.
  글로벌 바람은 기존 UE3 no-wind fallback `(0,0,1,0)`이다. LostArk 실행 중 전역 바람값을
  복구하거나 원본과 같은 바람 애니메이션을 확인했다는 뜻은 아니다.
- 현재 NPC idle pose의 머리가 상판에 걸리도록 실제 설치 WModel 삼각형과 NPC XZ의 수직 교차를 계산했다.
  상판 Y=25.13647559882726, idle 최저점 보정 후 NPC Y=25.19908152다.
  이는 현재 gameplay idle의 접촉 어댑터이며 원본 시네마틱 actor root Y와 혼동하지 않는다.
  나머지 NPC와 중앙 재구성 기둥은 그대로다.
- 맵 placement 4705 -> 4671, catalog 870 -> 871, 재질 959 -> 963,
  World revision 4 -> 5. NPC는 30명, World 총 34행을 유지한다.
  이전 959개 재질, placementLighting, 나머지 모든 material field와 다른 World 33행이
  교체 전 저장본과 동일함을 비교했다.

### 도구·자동 검증

- `Tools/MapPipeline/restore_maharaka_original_stand.py`: 입력 source identity, 슬롯 순서,
  35개 대상 개수, 기존 resource/authoring freshness 확인 후 Resources와 저작본을 stage/commit한다.
  기존 자료는 `out/MaharakaMapRestoration20260927/original-stand/replacement/before`에 보존한다.
  runtime 문서는 직접 쓰지 않고 공식 publisher를 사용한다. 재실행 시 resource bytes와 네 바인딩
  일치를 검사하고 중복 배치를 생성하지 않는다.
- `generate_maharaka_stand_vertex_shader.py --check`: 원본 VS 명령에서 재생성한 HLSL과 설치본 일치.
- `build_vehicle_source_material.py verify`: 네 프로그램 각각 Base, Light, configure EXACT.
- 받침·중앙 기둥·게시 일치 검사 6개, NPC 유지/애니메이션/지면 검사 4개 PASS.
- 신규 교체 계약 unit test 5개 PASS: 대상 범위, 실제 삼각형 접촉, 동시 편집 보호,
  두 번째 파일 승격 실패 시 첫 파일 rollback, C++/양쪽 HLSL 정적 재질 분류 일치.
- Area publisher Validate / Publish / Check PASS (4671 placements).
  WorldGameplay MAHARAKA Validate / Publish PASS (34 placements).
- project/filter XML parse 및 `git diff --check` PASS.
- `SourceCharacterShaderVariantProbe.cpp`에 정적 Base/Light program 1528~1531의
  실제 CShader 선택·상수·텍스처·Clone 검사 8개를 추가했다.
- Debug 제품 빌드와 이 probe 실행 결과는 아래 최종 검증에 기록한다.
- 추가 실행한 범용 DXBC 변환기 회귀 34개는 31개 PASS, 1개 FAIL, 2개 ERROR였다.
  FAIL은 기존 `.gitattributes`의 hash-bearing 경로 eol 기대 불일치, ERROR 2개는
  고정 SDK `10.0.22621.0` D3DCompiler 미설치다. System32 DLL도 해당 테스트의 고정
  binary identity와 달라 통과시키지 않았다. 범용 도구/정책은 이번 요청에서 변경하지 않았고
  이 전체 회귀를 PASS로 보고하지 않는다. 새 실제 셰이더는 제품 SDK 10.0.26100.0으로 빌드한다.

### 리소스 배포·수동 경계

- 새 Resources 상대 경로:
  `Map/LV_OCN_EVENTIS_MHP/MAP_65096D72C5C9_ITR_02453_SK/MAP_65096D72C5C9_ITR_02453_SK.wmodel`,
  같은 디렉터리의 `textures/itr_02453*.dds` 10개.
- 재사용 필수 파일:
  `Map/LV_OCN_EVENTIS_MHP_SOURCE_MATERIALS/EngineDefaults/726b99cf1d98_flat_white.dds`.
- 실제 폴더는 저장소 `Client/Bin/Resources` 아래다. 리소스 배포에는 위 WModel/DDS가 필요하며
  out의 중간 소스·변환 로그는 런타임 입력이 아니다.
- UModel glTF -> WModel의 4 submesh / 30621 vertices / 31350 triangles 보존과 material 순서는 확인했다.
  geometry receipt의 `UNKNOWN_UPK_TO_GLTF_BLOCKED`를 임의 PASS로 바꾸지 않았다.
  추가 PSK export에는 vertex color/extra UV chunk가 없고, CModel의 기존 absent-color white carrier를 쓴다.
- 사용자 확인은 새 Debug Client -> Lobby -> Maharaka 재진입 후 큰 모코모코 받침 위치다.
  NPC 높이가 Server 데이터이므로 새 Server에서 확인한다. 에이전트는 Client/UI를 실행하거나
  캡처하지 않았고, 최종 visual PASS와 전체 맵 원본 복원 완료를 선언하지 않는다.

### G10 최종 자동 검증

- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product -MaxCompilerProcesses 4` PASS.
  Engine / Shared / Server / Client compile/deploy와 기본 runtime 참조 검사를 완료했다.
  제품 compile 결과: `out/BuildPipeline/runs/20260927T085038912Z-debug-product.json`.
  Client.exe는 2026-09-27 17:50 KST 갱신됐다. 이번 빌드는 CSO 72개를 갱신했고 약 28분 49초 소요됐다.
  Release 및 전체 Core/FullDiagnostic suite를 실행했다는 뜻은 아니다.
- 정적 받침 그룹 1472 별도 FXC 검사 PASS. 별도 후보와 실제 Debug CSO SHA256이
  `35c020f201fd885ce1a25f6e4f9372cb80bd693075fa3df471f36cde9912156f`로 일치한다.
- `Test-SourceCharacterShaderVariants.ps1 -Configuration Debug` PASS:
  program 134 / clone 14 / failure case 9 / light pass 504 / native pass 144 / numeric draw 46,
  windowsCreated=0. 새 원본 받침 4개 프로그램의 static Base/Light 8개 검사도 포함한다.
  로그: `out/MaharakaMapRestoration20260927/original-stand/shader-probe/probe.log`.
- 별도로 모델 내부 UTF-16 상대 texture 경로 10개 모두 실제 모델 디렉터리 아래 존재함을 검사했다.
- 이번 교체 focused tests 15개와 이전 누락 재질 바인딩 회귀 5개 PASS.
  위에 기록한 범용 translator 환경/설정 실패 3개는 이 성공에 포함시키지 않았다.
- 교체용 이전 35개 배치 저작본은 replacement/before에 보존됐으며 필요하면 원본 행을 확인할 수 있다.
  Git commit/push/PR, Client 자율 실행, 화면 캡처는 하지 않았다.

## G11 — 마하라카 원본 NPC 재조사 (교체 미완료)

> 이 절은 추출 당시 기록이다. 이후 사용자의 교체 요청으로 원본 주민 28명의 실제 설치·게시를 진행했다.
> 현재 상태와 검증 경계는 [NPC 교체 RESULT](2026-09-27_MAHARAKA_NPC_REPLACEMENT_RESULT.md)를 따른다.

### 요청과 기존 결과 정정

- 사용자의 기준은 마하라카 원본 맵에 실제 사용된 NPC 외형과 배치다. 다른 지역의
  기존 NPC를 안내원으로 지정하는 작업이 아니다. 총 인원은 최대 30명이다.
- G08의 재구성 NPC 28명은 원본 좌표를 일부 사용했어도 원본 NPC 외형 연결을
  증명하지 못한다. 이들을 원본 주민/안내원 복원 완료로 취급하지 않는다.
- 이번 조사에서는 기존 World 34행, NPC 30명, G10 모코모코 받침/높이를 변경하지 않았다.
  새 모델은 추출 후보일 뿐 Resources 설치, WModel 교체, 게시, 빌드를 하지 않았다.

### 확인된 원본 근거

- `57009/DeployData.loa`를 다시 읽은 결과 NPC 배치 170개, 숫자 NPC ID 80개다.
  현재 추출 `EFTable_Npc.db`에 정확히 조인되는 ID는 570910/570911 두 개다.
  나머지 78개는 이 테이블에 행이 없다는 뜻이며, 원본 모델이 삭제됐다는 뜻이 아니다.
- 마하라카 SCENE03A/04A/06A/07A 원본 UPK에서 LookInfo actor 111개와
  고유 LookInfo 54개를 확인했다. 이 중 76 actor는 Matinee의 실제 연결을 따라
  애니메이션 트랙까지 연결된다. SCENE03A의 다른 구역 actor 2개를 섬 주민으로
  자동 배치해서는 안 된다. 위 숫자는 게시 NPC 수가 아니다.
- 씬 LookInfo 54개와 Deploy의 추가 원본 1개, 총 55개 LookInfo 파일을 기존 LPK
  색인의 정확한 파일명 참조로 추출했다. 색인 전체를 새로 생성한 검사는 아니다.
- 기존 일반 객체 덤프는 `eflookinfosmactorpartmaterialinfo`, `attachment`,
  `linearcolor` 일부를 hex 앞부분만 남겼다. 이번에는 원본 UPK를 다시 읽어
  전체 속성을 해석했다. 몸 재질 변형이 있는 actor 76개, 소켓 부착이 있는 actor
  14개다. 피부/옷 색, 마스크 설정, 머리와 소품을 무시하고 같은 몸 모델로 합치면
  원본 외형을 복원한 것이 아니다.
- 원본이 직접 참조하는 `MN_ISNF_00` 몸과 `MN_HEAD_FE02_029` 머리,
  `MN_ISNM_00` 몸과 `MN_HEAD_MA02_058` 머리, 각 원본 normal AnimSet을
  UModel로 추출했다. 4개 모델 export와 2개 AnimSet export가 exit 0이다.
  이 성공은 native material 재현이나 게임 안 외형 검증 성공이 아니다.

### 조사 도구와 보관 위치

- `Tools/WorldPipeline/audit_maharaka_source_npcs.py`는 읽기 전용 원본 조사와
  지정한 out 폴더의 증거 출력만 한다. numeric NPC ID, 씬 actor ID, LookInfo를
  별도 필드로 보존한다. 위치/이름 유사성으로 서로 연결하거나 안내원을 지정하지 않는다.
- 조사 결과: `out/MaharakaMapRestoration20260927/source-npcs/source-npc-audit.json`.
  원본 패키지 hash, Deploy/DB hash, 배우/부품/재질 속성, 트랙, 추출 참조를 기록했다.
- 추출 모델/애니메이션 후보: `C:/LostArkExtract/MaharakaNpcSource20260927/raw`.
  제품 Resources 외부이며 현재 게임이 소비하지 않는다.

### 자동 검증과 남은 작업

- Python 문법, 정확한/누락 NPC ID 조인, UE→Client 좌표 변환,
  중복 actor/잘린 입력/잘못된 클래스 길이/빈 입력 거부를 검사했다.
- 씬 actor ID 111개의 유일성, LookInfo 55개 정확 참조 추출 상태,
  원본 UPK hash 일치, 미확인 numeric NPC/guide ID를 임의 생성하지 않음을 검사했다.
- `git diff --check` 통과. 첫 sandbox 실행은 Crypto 의존성/LFS 접근이 막혀
  권한을 받아 다시 검사했다. 초기 합성 테스트 fixture의 추가 null 바이트를
  바로잡은 뒤 위 검사가 통과했다.
- 안내원 신원은 확인된 것이 0개다. 씬 원본 주민이 발견된 것과 누락된 78개
  Deploy NPC ID/안내원 역할이 연결된 것은 별개다. 현 단계에서 가짜 연결을 게시하지 않는다.
- 남은 작업은 안내원/Deploy 연결 추적, 선택한 원본 주민의 부품·색상·소품·애니메이션·
  native 재질 보존 변환, 최대 30명 범위의 배치 교체와 publisher 검증이다.
  기존 재구성 28명을 교체했다거나 사용자가 게임에서 새 원본 NPC를 볼 수 있다는
  상태는 아직 아니다. Client 실행/화면 검증은 하지 않았다.
