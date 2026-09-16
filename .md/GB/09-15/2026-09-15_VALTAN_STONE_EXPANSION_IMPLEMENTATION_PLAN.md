# 발탄 정적 석재 원본 입력 확장 계획

## G00. 현재 실측과 목표

기존 09-08 중앙 floor4·rock3, 09-11 circle1의 입력을 보존한다. 현재 맵의 같은 원본
floor01 24배치와 rock02 419배치를 실제 component export로 읽었다. 원본 MIC는 rock04
27배치, rock05 164배치, rock02 252배치다. 04와 05는 native engine-equivalent static set이
같고 기존 `bg_base_opa_overlay` 구현과 연결된다. 02는 vertexcolor paint switch가 다르므로
이 단계에서 같은 shader로 복사하지 않는다.

이번 변경은 같은 계산을 사용하는 191배치의 원본 UV1·tangent handedness·component 색과
개별 RNM atlas를 연결한다. 기존 7배치는 유지하고 새 184배치를 후보로 처리한다. 배치별
native 데이터가 다른 경우 각각 원본을 사용하며 미해석 입력을 흰색이나 다른 배치 값으로 대체하지 않는다.

## G01. geometry와 material 데이터

기존 `cook_wmodel_geometry_contract.py`와 `CModel -> CMaterial` 경로를 사용한다. 원본
StaticMesh native index stream과 source glTF corner를 대응시키고 UV1·tangent W·BGRA 색을
검증한다. 같은 geometry·색이면 payload를 재사용하고 배치 RNM은 stable sourcePlacementId로
분리한다. source BGRA를 RGBA로 한 번만 변환한다.

`Data/Maps/Imported/LV_LUT_HEARTRB_ED/*.mapassets`에 필요한 variant를 추가하고
`Data/Maps/Authoring/LV_LUT_HEARTRB_ED/*.mapplacements`의 해당 asset 참조만 교체한다.
같은 Area의 mapmaterials에 원본 MIC04/05 상속 상수·texture와 atlas 입력을 추가한다.
MapCatalog는 Valtan assetCount만 갱신한다. transform·stable ID·Deploy·gameplay는 보존한다.

## G02. Resources와 실패 경계

원본 package는 읽기 전용이며 추출·조리 중간물은 `out/FullMapRestoration20260915/Valtan`에
둔다. 설치는 기존 `Resources/Map/LV_LUT_HEARTRB_ED/SourceStoneRestore`와
`Resources/Map/Lighting/Valtan` 아래 신규 경로로 한다. 원본 RNM pair·scale/bias는 배치마다
검사하고 현재 direct light의 baked GUID 교집합을 확인한다. 구운 조명을 direct로 다시 추가하지 않는다.

## G03. 검증과 남은 구분

geometry topology/channel 검사와 실제 DDS 존재·입력 JSON 검증 후 기존 Area publisher의
Validate → Publish → Check를 사용한다. 새 광역 하네스·Client/UI 실행·캡처는 하지 않는다.
공용 C++/shader 변경과 Product 빌드는 통합 담당이 소유한다. 사진의 밝은 Deploy BRICK_A/B와
중앙 하층, 다른 static permutation, 안개·skybox·원본 후처리는 별도 소비자이며 이 석재 확장의
완료 수치에 섞지 않는다. 결과에는 실제 적용 수, 미지원 입력, 사용자 화면 확인을 분리한다.

## G04. 중앙 Deploy A/B 원본 입력 연결

통합 담당과 소유 범위를 조정했다. A/B의 source StaticMesh LOD0 material array는 slot0
`lv_lut_heartrb.mat.bg_pap_stone_rock04_mi_ksr`, slot1
`bg_rad_valtan_a.mat.bg_rad_valtan_crack_floor01_mi_lsj`다. slot0은 검증된 overlay,
slot1은 normal·diffuse saturation·specular만 활성인 source BG permutation을 사용한다.
기존 dummy material의 재구성 emission은 원본 MIC 입력이 아니며 native 재질이 연결된
모델에서는 공용 렌더 담당이 그 별도 emission 경로를 차단한다.

A의 원본 25,819 정점과 UV2개, B의 34,306 정점과 UV3개, 두 모델의 native BGRA를
새 Resources `Map/LV_LUT_HEARTRB_ED/SourceDeployRestore`에 설치한다. 기존 FBX WModel의
재계산 normal/tangent와 strict geometry 비교가 달라 원본 glTF로 기존 converter container를
새로 만든 뒤 같은 geometry cooker에서 검증한다. 원본과 기존의 index 수 및 bounds를
따로 대조하며 배치 TRS와 preScale, 파괴 동작은 바꾸지 않는다.

Deploy catalog A/B의 model ID와 BossCatalog의 해당 네 material override만 연결한다.
BossCatalog parser는 통합 담당이 기존 MapAssetCatalog 검증기를 재사용하도록 확장한다.
소품은 합성 배치이므로 다른 static component의 RNM이나 shadow atlas를 복사하지 않는다.

## G05. 나머지 rock02 원본 분기 확장

추가 조사에서 기존 general overlay shader가 vertex paint를 끄고 world-up normal과
`overlay_amount`로 coverage를 계산하는 분기를 이미 지원함을 확인했다. 원본 MIC02와
MIC04 static set 값의 차이는 `1.use_vertexcolor_paint` 하나다. MIC02는 원본 값에 따라
`sourceOverlayFlags=259`, `sourceDirection=[0,1,0,0]`을 명시하며 MIC04/05와 재질 수치를
공유하지 않는다. 원본252배치의 BGRA827정점은 세 그룹이고 native RNM 전체와 방향광
shadow100개를 각각 대응시켰다. 같은 공통 cooker와 Area publisher로 추가한다.

최종 catalog716은 현재 single catalog2048, 전체 catalog32768 제한 안에 있다. 새252행을
연결해 이번 대상 floor24·rock419의 전체443배치를 닫으며 다른 원본 geometry family와
중앙 소품의 시각 승인까지 완료한 것으로 표현하지 않는다.

## G06. 원본 접선 basis와 난간

UE packed normal.W를 그대로 glTF tangent.W로 복사하지 않는다. UE→glTF(X,Z,Y)와
glTF→runtime(X,Y,-Z)는 각각 determinant -1이므로 준비 glTF W는 native sign의 음수,
최종 runtime W는 native sign이다. native UV 미분의 종법선과 cross(N,T)를 대조하고
최종 WModel의 정점·index·색·UV도 확인한다. 이번 신규 모델의 부호 오류를 이 기준으로
교정했으며 기존 정상7개는 전 채널 일치 확인 후 보존한다.

난간 `bg_rad_valtan_floor01_sm`은 native20,440정점·61,884index·UV2개를 보존한다.
실제 source4slot은 LV wall04a의 detail-normal BG, LUT deco02의 simple BG, LV wall06의
방향 overlay, LV roofarch01의 detail-normal/subspecular BG다. 서로 다른 MIC와 선택된
texture를 별도로 등록하며 원본에 없는 RNM·COLOR를 다른 오브젝트에서 복사하지 않는다.
새 모델은 SourceDeployRestore에 설치하고 rail model ID와 해당 네 ActorCatalog 행만
교체한다. 하늘의 sky-cinema/molding/opaque 세 source 재질은 shader 동치와 소비자
연결을 먼저 확인하고 그 결과를 별도 기록한다.

## G07. 원본 투명 배경 배치527

PS export527은 export525와 cloudplane geometry를 공유하지만 실제 actor MIC가 다르다.
527의 molding source static set은 기존 `source.map.translucent-59.v1`와 base GUID·switch가
동일하다. 현재 `CMapAssetObject`의 material별 translucent → BLEND → pass3와 native
forward binder/PS 소비를 확인했다. 별도 asset variant에 실제 MIC 상수·3 texture·원본
UV0/UV1/TBN의 4정점·6index를 등록하고 source527의 asset 참조만 교체한다.

UModel이 native half 0을 3.0517578125e-5로 내보낸 UV0 세 정점은 native 값으로 교정한다.
허용 오차를 넓히지 않으며 다른 불일치는 실패로 처리한다. 원본에 없는 COLOR·RNM을
추가하지 않는다. 525의 sky-cinema와 528의 opaque sky는 shader와 background 소비자가
다르므로 이번527의 연결 수에 포함하지 않는다.

## G08. 원본 바닥 재질 전환 뒤 Deploy 입장 설정 교정

`CDeployPropObject::Initialize`는 `deferredEmissiveOverlay=true`인 정적 모델의
두 번째 mesh에 EMISSIVE texture가 있는지 검사한다. 원본 A/B의 crack surface는
발광하지 않는데, geometry 교체 뒤 Imported deploy catalog의 기존 overlay 값1이 남았다.
렌더 단계에서 native surface의 중복 발광을 막는 것만으로 초기화 검사를 통과하지 못한다.

해당 flag는 Map Effect의 파괴 바닥 surface owner 계약에서도 사용하므로 유지한다.
`DeployPropObject.cpp`의 초기화가 기존 `Should_RenderDeferredEmissiveOverlay`와 같은
surface family를 판정하게 한다. native surface는 기존 map material 준비·렌더 경로가
입력을 검증하고, LEGACY surface에만 기존 EMISSIVE texture 필수 검사를 적용한다.
가짜 emissive나 별도 fallback을 추가하지 않는다. public header·project 등록은 변하지 않는다.

재질·리소스·placement·파괴 상태와 다른 Deploy10종을 보존한다. 실제 설치 A/B의 재질
입력과 초기화 분기, Area Validate/Check, 최소 컴파일과 Product build를 확인한다.
실제 입장과 화면 확인은 사용자가 Lobby → Valtan에서 수행한다.

## G09. 전체 PS·SL 원본 재질과 정점·조명 연결

PS와 SL00~SL05 전체로 범위를 확장한다. 원본 StaticMeshComponent의 ordered material override와
native section 기본 MIC, native mesh index와 UModel corner를 대응한다. 원본 COLOR, UV0~UV2,
tangent.W, RNM 평균·방향 계수와 component atlas 좌표를 함께 운반한다. 추출 identity는 package를
포함한 source 전체 경로다. 같은 leaf 이름으로 다른 package의 원본을 대신하지 않는다.

지원 BG·overlay·foliage·grass family는 실제 static 분기와 대조한다. NULL texture는 부모 이름으로
채우지 않고 native material map의 uniform expression/referencedTextures index로 확인한다.
RNM·shadow texture 조합별 variant가 single 2,048개 한도를 넘으면 기존 Bern mapset 경로를 재사용한다.
BASE/PS/SL00~SL05 여덟 shard와 MapCatalog canonical entry, 96.DataFiles의 None항목을 갱신한다.
source asset 정의와 placement stable ID/TRS/visibility, 기존 CModel/CMaterial 경로를 보존한다.

설치 직전 source SHA를 확인하고 새 resource만 추가한다. 공식 Area publisher의 Publish/Check,
전체 placement 보존과 resource closure를 확인한다. Client/UI와 최종 화면 확인은 사용자가 수행한다.

## G10. overlay 후속 분기

subspecular, specular saturation, emissive, bump는 실제 MIC별 native shader를 먼저 대조한다.
기존 BG shared parameter를 program7에서도 소비하도록 parser, MapAssetRenderUtils, shader,
publisher를 함께 연결한다. 다른 static surface로 flag를 덮어써 통과시키지 않는다.
실행 중 Client의 EXE/DLL/CSO 교체와 Product 빌드는 root 통합 작업에서 수행한다.


G10의 추가 분기는 source material native map 9종의 Base/Direct 명령을 수치로 대조한다.
방향 기반 overlay의 specular half-dot은 비정규화 mixed normal, vertex-paint overlay는
base normal을 사용하므로 bit1024를 별도로 운반한다. `sourceBump`는 bit512에 대응하며
기존 MODEL_SURFACE의 BG 파라미터 필드를 재사용한다. Engine Model의 허용값 검증과
Material의 emissive texture 준비까지 같은 경로로 연결한다. 새 C++/header 파일은 없다.

사용자 검증 전 새 overlay 배치는 `out/ValtanArenaMaterial20260915/BroadAuthoring`과
native/model/texture receipt에서 검증용 세트로 준비한다. 현재 설치된 3,967 asset의 Area를
자동으로 교체하지 않는다. 별도 PS525/528 및 LightFunction은 원본 CPU 조명·blend/depth와
shader 입력 소유자를 확인해 검증 세트를 준비한다.

## G11. 검증된 overlay 9종의 추가 설치

후속 사용자 요청에 따라 root가 실행 중 Client/Server의 종료를 확인하고 검증 완료 후보의
추가 설치를 요청했다. 앞선 out-only 인계와 구분해 9 MIC/922배치만 설치 대상으로 삼는다.
원본11파일의 SHA가 후보 작성 시점과 일치함을 다시 확인했다. 사용자 VS 빌드가 진행 중이므로
빌드 종료 확인 전에는 out-only staging과 문서 갱신만 수행한다.

`BroadAuthoring`의 검증된 305 variant/387 material/922 lighting을 현재 파일과 대조한다.
13,184 placement의 stable ID/source ID/TRS/visibility와 기존 material/lighting 행은 보존하고,
922개 asset 참조만 변경한다. 기존8개 shard에 필요한 정의만 더하고 MapCatalog의 해당 Area
수치만 변경한다. 새 리소스만 설치하며 기존 동일 경로의 byte가 다르면 덮어쓰지 않는다.

별도 OverlayInstall 디렉터리에 staging·변경 전 원본·SHA·receipt를 둔다. 쓰기 직전 원본과
staging SHA를 재검사하고 변경이 있으면 설치를 중단한다. 과거 Pass2 installer/backup은
사용하지 않는다. 다른 Area publisher와 순서를 맞춘 뒤 공식 Area Publish/Check를 수행한다.
실제 새 리소스의 존재, 전체 배치 보존과 JSON parse, diff 검사를 RESULT에 기록한다.
최종 Product 통합 빌드는 root가 수행한다. 하늘 program90, LightFunction, 환경 기본값과
FX product cue는 이 설치에 포함하지 않으며 화면 판정은 사용자가 한다.
