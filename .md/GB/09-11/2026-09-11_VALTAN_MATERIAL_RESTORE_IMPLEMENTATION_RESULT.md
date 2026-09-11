# 발탄 본체·무기·유령 재질과 중앙 바닥 구현 결과

## G00. 적용한 범위

발탄 본체·분리 갑옷2·도끼·유령의 실제 모델5개, material slot11개를 기존 `CModel -> CMaterial` 경로에 연결했다. BossCatalog의 기존50 override를 보존하고 발탄11개를 추가했다. `CValtanPresentationAssetService`가 body/weapon/armor의 descriptor를 `CActorCatalog::Build_ModelLoadDescription`으로 받아 prototype batch에 사용한다. 원본 native source-character family는 정상 본체·갑옷·무기 program21, 유령 program84다. donor animation은 재질 입력과 별도인 기존 경로를 유지한다.

원본8MIC의 texture closure에서 선택한 DDS26개를 `Client/Bin/Resources/Character/SourceMaterials/Valtan/<package>/`에 설치했다. 총13,723,300바이트다. 최상위 압축 블록은 원본 추출과 동일하며 하위 mip는 기존 색공간별 BOX/separate-alpha adapter로 생성한 프로젝트 데이터다. 원본 lower mip 전체를 이식했다고 해석하지 않는다. Resources는 Git 제외 Drive 입력이다.

수정한 제품 코드는 `Client/Private/ValtanPresentationAssetService.cpp`, `Client/Public/SourceCharacterMaterialParameters.h`, `Engine/Private/Model.cpp`의 최대 program admission83→84, Engine/Client 양쪽 `Shader_SourceCharacterPrograms.hlsli`와 `Shader_SourceCharacterMaterial.hlsli`다. 새 C++ 파일과 project/filter 등록은 없다. BossCatalog의 V2 combat-object 관련 내용은 작업 시작 전부터 있던 미커밋 변경으로 소유를 확정하지 않았다. 이번 재질 작업은 root override11행만 추가했고 통합 담당은 BossCatalog를 직접 수정하지 않았다.

## G01. 유령 source program과 남은 표현 경계

program84는 선택된 원본 Base/Directional DXBC의 산술과 rim/cloud/opacity 시간식을 옮긴 것이다. named parameter의 missing/extra/nonfinite 입력을 기존 strict packing 방식으로 거부한다. 원본 VS varying의 UV·tangent view/light·fog·up 입력을 기존 shared shader에 맞춰 연결했다. 이전72 Base/Light 함수는 각각 그대로이고 새 Base84/Light84만 추가됐다.

원본 유령 Base PS가 참조하는 scene hemisphere/global 상수는 기존 adapter에서0이다. source native DXBC와 adapter를 같은 상수로 비교한 fixture에서 Base RGB0이었으며 Direct RGB는 양수였다. 이것은 원본 scene ambient 복원 완료 증거가 아니다. CBody_Valtan은 기존 deferred pass0/NONBLEND를 사용하며 유령 alpha는 shared ordered coverage 분기로 들어간다. 원본의 정렬된 translucent 합성, scene hemisphere·global 및 실제 화면의 최종 투명감은 아직 동일하다고 판정하지 않는다.

## G02. 중앙 원형 돌바닥에서 실제 수정한 내용

첨부 화면의 중앙 원형 mesh와 밝은 균열 slabs, 바깥 돌은 실제 서로 다른 재질이다. 원본이 다른 표면을 한 재질로 통일하지 않았다. 대신 중앙 원형의 잘못된 MIC 참조를 원본 level override로 고쳤다.

| 슬롯 | 현재 모델의 materialName 키 | 복원한 원본 sourceMaterial |
|---|---|---|
|0|bg_lut_wagloy_circlefloor01_mi_rain_jjy|lv_lut_heartrb.mat.bg_lut_wagloy_circlefloor01_mi_jjy|
|1|bg_lut_wagloy_circlefloor01a_mi_rain_jjy|lv_lut_heartrb.mat.bg_lut_wagloy_circlefloor01a_mi_jjy|

원본 component는 `LV_LUT_HEARTRB_ED_SL00:export:1274`, stable placement는 `15561800956777256508`이다. 두 slot은 원본 brightness0.35/0.8, saturation1/0.6, normal intensity2/1.5, overlay tiling2.5/1.3 등을 각각 유지한다. 누락된 overlayNormalIntensity1과 specularColor 흰색은 `efbasematerial_prologue.bg.base.bg_base_opa`의 실제 expression default에서 읽었다. 두 MIC와 기존 `lv_lut_heartrb.mat.bg_pap_stone_rock04_mi_ksr`의 engine-equivalent FStaticParameterSet SHA256은 모두 `a279897432bdcd1d7642834ddb1f5e8dadd10a5b30090163e722d2d600ee3cfa`다. 따라서 기존 family7 shader adapter를 재사용할 근거가 있다.

원본 static geometry1185정점·2346indices를 원본 section index순서와 GLTF102/1083정점으로 대응시켰다. native position 최대오차3.052e-7, normal1.249e-7, tangent1.213e-7, UV0오차0이었다. 이 원형 mesh의 UModel UV1/tangentW 차이는0이다. 원본 component의80종 BGRA 색을 native index로 연결해 RGBA8으로 한 번 변환했다. 기존 `cook_wmodel_geometry_contract.py`의 topology/channel 검사를 거쳐90,836바이트 variant를 만들었다.

새 variant는 `MAP_FBC80A02F72E_BG_LUT_WAGLOY_CIRCLEFLOOR01_SM_JJY_VST_SL00_E1274`이며 Resources의 `Map/LV_LUT_HEARTRB_ED/SourceStoneRestore/Geometry/<variant>/<variant>.wmodel`에 둔다. 기존 mesh는 그대로 유지한다. 새 D/N DDS4개는 `Map/LV_LUT_HEARTRB_ED/SourceStoneRestore/Textures/`에 원본 최상위 DDS 그대로 설치했다. 신규 네 장에는 lower mip를 생성하지 않았다. 기존 overlay D/N과 SL00 normalizedAverage/directionalMax atlas를 재사용한다.

RNM 입력은 원본 component의 average reference105, directional reference72를 실제 object 이름으로 join했다. coordinateScale은0.1796875, bias는0.25390625이며 averageScale은[1,1,1], directionalScale은[1.301937222480774,1.26918363571167,1.2202305793762207]이다. `mapmaterials`는7→9행, placementLighting은7→8행, imported catalog는279→280행이다. MapCatalog의 해당 assetCount만280으로 바꾸고 stable placement 한 행만 variant로 교체했다.

신규 geometry의 generic legacy evidence header에는 과거 source manifest/cook receipt가 없다는 현재 상태를 명시했다. 현재 원본 UPK와 index/channel 대조는 별도 새 진단으로 확인했으며 generic receipt의 고정된 과거 provenance 역할 이름을 역사적 cook 이력 증거로 사용하지 않는다. 이 작업은 별도 Resource lock/manifest 체계를 제품에 추가하지 않는다.

## G03. 바닥에서 검토만 끝난 범위

Deploy `VALTAN_FLOOR_BRICK_A/B`는 slot0 diffuse만 있고 normal/specular가 비어 있다. slot1 균열은 diffuse/normal과 기존 프로젝트 emissive reconstruction을 쓴다. RAIL4slot도 diffuse 중심이다. 현재 WMat 이름이 dummy_material이고 실제 native mesh default slot MIC의 완전한 join 및 해당 native shader 소비자가 확보되지 않아 이번에는 이 입력을 임의 교체하지 않았다. `CDeployPropObject::Render_Static`는 기존 generic 직접 bind를 쓰므로 ActorCatalog override만 추가해서는 source-character/source-map shader 복원이 닫히지 않는다.

중앙 하층 `LV_LUT_HEARTRB_ED_SL00:export:1196` slot0도 level MIC override와 현재 모델 후보가 다르다. 그러나 원본 Direct/Baked shader가 family7과 다르므로 같은 adapter로 임의 연결하지 않았다. 기존09-08의 static floor4+rock3는 다른 계층이며 이번에 그7행을 변경하지 않았다. 따라서 밝은 A/B 바닥 전체와 난간·하층의 native 복원까지 완료했다고 하지 않는다.

## G04. 실행한 검증과 증거 상태

작업 중 외부 out 정리로 기존09-08 native helper/input과 이번09-11 초기 진단의 대부분이 삭제됐다. 소스 코드·Resources와 남은 floor_review/preservation JSON은 보존됐다. 이 문서는 소실된 파일을 재생성된 성공 로그로 소급하지 않는다. 이후 진단은 `%TEMP%/LostArkValtanRestore20260911/materials`에 새로 만들었다.

삭제 전에 직접 관찰한 값은 다음과 같다. strict packing11행에서 valid/missing/extra/nonfinite/rollback 실패0, ActorCatalog5모델/11override descriptor 실패0, ValtanPresentationAssetService syntax compile exit0, program21/38 Base/Direct WARP 대조144case/147456pixel nonfinite0·relative-error>1e-3 mismatch0이었다. 최초 CModel 검사에서 실제 CModel create는 실행하지 않았고 boundMeshes0이었다. 보존 기록은 이전72함수 각각 불변·기존50 Boss override 불변·신규26texture 존재를 기록한다. 원래 각 실행 로그와 native bytecode fixture는 소실돼 지금 링크 가능한 새 증거로 취급하지 않는다.

삭제 후 새로 실행해 보존한 검증:

- `circle-native-join.json`, `circle-cook.json`: 원본1185정점/2346indices,80종 배치색,2slot topology/channel cook 성공.
- `circle-static-sets.json`, `circle-stone-static-parity.json`: 두 MIC와 기존stone의 engine-equivalent static set 동일. 원본 normal static parameter의 byte compression 형식을29바이트로 읽었다.
- `circle-parent-defaults.json`, `circle-textures.json`, `rnm_decode.py`: 원본 상속값·네 DDS·RNM reference/scale 재확인.
- `current-resources.json`: 현재 Boss 모델5/slot11/DDS26(13,723,300바이트) 물리 존재를 재확인하고, circle 신규DDS4의 설치 payload가 새 원본 추출과 byte-identical인지 확인.
- `json-parse.json`: 변경한 JSON4개 parse, runtime/authoring materials 일치, Boss override61행·map material9행·placementLighting8행 확인.
- `floor-check.log`: 같은 Area publisher `-Mode Check` exit0,13184placements와7출력 일치 확인.
- 담당 변경 경로의 `git diff --check` exit0. Git의 LF→CRLF 안내만 있었다. XML과 project/filter는 이번 담당 변경에 없다.
- `floor-publish.log`: `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Mode Publish` exit0,13184placements,7runtime outputs. 생성물을 직접 편집하지 않았다.

첫 Product Debug 빌드는 통합 담당 실행으로 exit0이었다. 이후 실제 CModel 검사에서 아래 예약 program 충돌을 발견하고 수정했으므로 그 첫 빌드를 최종 완료 상태로 쓰지 않는다. 실제 CModel 검사는 G05의 첫 실행까지만 수행했다. 사용자의 재빌드 후 종료 요청에 따라84 이후 추가 probe는 실행하지 않는다. Client/UI 실행·캡처·visual PASS는 수행하지 않았다. 사용자 화면 판단이 남아 있다.


## G05. 실제 CModel에서 확인한 program 충돌과 수정

새 `model_bind_probe.cpp`를 제품 Engine.lib와 링크한 첫 실행에서 정상 본체3·갑옷1+2·무기2슬롯은 create/clone/prototype-release/bind를 통과했지만 유령 첫 slot의 program38 bind가 `E_FAIL(80004005)`였다. 이 실패는 소스 산술 대조만으로는 찾지 못한 실제 소비자 오류다. `model-probe-first-failed.log`에 보존했다.

원인은38이 실제 `SourceMapWaterMaterialParameters.h`의 water program이며 `CMaterial::Bind_SourceCharacterInputs`가38..63을 forward로 분류한다는 점이었다. `CMaterial::Bind_SourceCharacter`도33..65에는 deferred light용 frame row를 등록하지 않는다. SourceCharacter 함수 목록에서만 빈 번호를 확인한 최초 선택이 잘못됐다.

따라서 유령을 공통 소비자에서 비어 있는84로 옮겼다. named family·parameters·textures는 그대로이고 packing program값, native Base/Light 함수 이름과 switch dispatch, source varying/coverage 분기만84로 변경했다. `CModel`의 source-character 최대 admission 한 줄을83에서84로 확장했다. 기존 map33..65와80..83 조건, Material.cpp는 바꾸지 않았다.84는 기존 일반 deferred frame-row 등록과 source Direct 누적을 소비한다. namespace가 다른 Effect ALTV84는 충돌 대상이 아니다.

`program84-range-check.json`으로 map33..65 비충돌과 native 함수 rename-only를 확인했고 독립 담당의 소비자 범위 검토도 추가 지적 없이 끝났다. 이 변경 뒤 최종 Product 재빌드를 진행했다. 사용자가 “그거 재빌드 결과 나오면 종료해줘”라고 명시해 추가 CModel probe compile/run은 취소했다.84 적용 뒤 유령3slot의 실제 bind와 circle2slot의 실제 CModel create/clone/bind는 미실행 상태다. 최종 Product 실행 결과는 통합 담당 RESULT에서 보고하며 첫 Product 성공이나 source/cook 검사를 이 미실행 검사 대신 사용하지 않는다. CMaterial frame-row/Direct static 함수는 EngineDLL 외부에 export되지 않으므로 isolated probe에서 직접 호출하지 않는다. 새 public 검사 전용 API나 중복 Material 런타임을 추가하지 않는다. source native 본문의38→84 변경은 함수명과 dispatch만이며 숫자 연산은 그대로다. G04의144case 수치는 최초38 상태에서 관찰한 산술 검사로, 현재84 Product 실행 결과를 대신하지 않는다.


최종 검사 경계는 정상 body/armor/weapon8slot의 실제 CModel bind 성공, 초기 ghost38의 실제 bind 실패 및84 수정 완료,84 이후 ghost와 circle CModel 재검사 미실행이다. circle은 원본 geometry/channel cook와 publish/Check까지만 완료했다. 사용자의 종료 요청 뒤 제품 코드·리소스 추가 수정이나 새 진단 실행은 하지 않았다.
