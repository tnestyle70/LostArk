# 가디언 나이트 기존 다섯 의상 슬롯 구현 결과

## G00. 반영 범위

사용자의 최종 지시에 따라 다른 class와 같은 기존 의상 슬롯 다섯 개만 사용한다.
GuardianKnight의 두 무기 행을 실제 의상으로 교체했으며 별도 무기 선택, UI 슬롯,
schema, preset 저장 field를 추가하지 않았다. 기존 UI C++/H, layout와 생성기는 변경하지 않는다.

| 기존 슬롯 index | visualSet ID suffix | 출처 | 다섯 부위 정점 합계 |
|---|---|---|---:|
| 0 | `class_select_hr00.outfit` | 현재 HR00 기본 의상 | 97,736 |
| 1 | `original_00.outfit` | 보존된 PC_DDK_00 의상 | 56,178 |
| 2 | `source_ddk_01.outfit` | PC_DDK_01 기본 mesh | 41,413 |
| 3 | `source_ddk_02.outfit` | PC_DDK_02 기본 mesh | 32,330 |
| 4 | `source_ddk_03.outfit` | PC_DDK_03 기본 mesh | 37,619 |

모든 ID의 prefix는 `character.guardian_knight.`다. 다섯 upper WModel의 SHA-256은 서로
다르며, 현재 기본 1개와 실제 대안 4개를 연결했다. PC_DDK_04/05는 조사한 package에 자체
mesh가 없으므로 임의 geometry를 만들거나 기존 모델을 새 의상으로 중복 등록하지 않았다.

기존 `Wear_CustomizingSet -> CEquipmentPresentationService::Apply_Preview ->
CCharacter::Apply_EquipmentPreview` 경로를 그대로 사용한다. 각 의상은 UPPER primary에
UPPER/LOWER/HANDS/SHOULDER/HEAD를 점유한다. 성공할 때 다섯 기본 갑옷 part를 교체하고
기존 body의 의상/머리 hidden mask 및 무기·Wing 경로를 유지한다. 사용자 수정 지시 후
별도 무기 선택 제안으로 만들었던 UI/schema 변경은 모두 제거했다.

## G01. 원본 추출·모델

설치 원본 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages`에서
UModel LostArk v7의 `-gltf -dds -noanim -groups -game=lostark -kr -nameresolve`로
세 package의 upper/lower/arm/shoulder/helmet 15개를 추출했다. 기존
`normalize_character_equipment_gltf.py`와 `ModelAssetConverter --scale 100 --no-auto-textures`를
사용했다. 신규 별도 모델 런타임은 없다.

15개 모두 설치 GuardianKnight master 285개 본과 같은 palette 순서로 변환됐다.
추가 본은 0, 모든 실제 weighted inverse bind의 master 차이는 0이며 원본 frame 변환 전
최대 inverse-bind 차이는 0.000894로 검사 범위 안이다. normalizer 결과는 15/15 COOKED다.
Resources ID는 `Character/GuardianKnight/Equipment/source_ddk_XX/<part>.wmodel`이다.

PC_DDK_03의 lower source가 실제로 참조하는 `pc_ddk_03-3_lower_mi`와
`pc_ddk_03-3_lower1_mi`를 그대로 유지했다. 이름만 보고 임의의 기본 MIC로 교체하지 않았다.

## G02. 원본 재질과 native1526

29개 실제 MIC의 RefShaderCache GPU-skin Base/Light, uniform expression, 부모 상속 후
parameter와 texture color space를 추출해 15개 모델의 34개 named-material override로 연결했다.
기존 native1/94/95/183/198을 재사용하며 기존183/198의 scene environment 입력과 finite guard는
보존했다. shader ID와 uniform packing을 함께 비교했으며 같은 픽셀 shader ID만으로
서로 다른 packing을 재사용하지 않았다.

`PC_DDK_01.mat.pc_ddk_01_upper1_mi`의 emissive 갑옷은 기존에 없는 Base
`a68bdca4b6e4b34bb222fafbd8cc25bd` / Light `26eada8cd67300489567bd8748f26356` 조합이다.
기존 generator/registration writer로 `source.character.equipment-native-1526.v1`을 등록했다.
기존1472 cohort에 들어가므로 새 shader project item이나 새 C++ TU는 필요하지 않다.
Engine/Client Base·Light leaf와 dispatch, Engine registry, Client parameter packing을 연결했다.

이 Base의 instruction201은 미복구 engine BRDF lookup을 0으로 공급하고 instruction206은
`1 / r2.z`를 계산한다. native198에 이미 있는 같은 lookup/reciprocal 경계와 대조하여
1526의 정확한 source shader ID와 `div r2.y, ..., r2.z` 한 instruction에만
`r2.z != 0.f ? 1.f / r2.z : 0.f`를 적용했다. lookup이 0일 때 전체 표면에 INF/NaN을
전파하지 않고, nonzero 입력의 native reciprocal은 유지한다. 임의 LUT·재질값은 만들지 않았다.
이 처리는 emitter에도 들어가 재생성 후 유지된다. 다른 작업의 native600 머리 identity 수정은
그대로 보존됐다.

같은 `pc_dk_av_base_upper_d` leaf가 PC_DDK_01과 PC_DK_AV_BASEBODY package에서 서로
다른 원본 texture를 뜻하는 것도 확인했다. source01은 자기 피부 texture를 사용하고 source02/03은
basebody texture를 사용한다. texture map을 source package.object 단위로 닫고 각 MIC의 map을
별도로 `command_rows`에 전달했다. 전역 leaf 이름으로 다른 package texture를 섞지 않았다.

## G03. 데이터·아이콘·배포

`EquipmentPresentationCatalog.json`에 full outfit 3개를 추가하고 `CharacterCatalog.json`의
root `modelMaterialOverrides`에 34개 행만 추가했다. 교체 직전 최신 저장본을 읽고 원본 backup,
hash freshness, atomic replace와 자기 변경만 rollback하는 기존 transaction을 사용했다.
기존 모든 character field와 material row, 모든 기존 equipment set이 그대로 보존된 것을
다시 비교했다. 다른 작업의 Guardian eye `shadowfactor=1`, `tdspecular_intensity=.25`도 보존했다.

UI는 `CustomizingCostumes.json`과 `CustomizingIcons.json`의 Guardian costume 배열만 바뀐다.
ObjectUnit 0..4, 다른 class, 기존 layout bytes는 유지한다. 신규 아이콘은 원본
`IconInfo.loa`와 DDK_Item_0 atlas의 DDK_Item_19/1/50을 64×64로 잘라 각각01/02/03에 연결했다.
01 아이콘은 원본 Item table의 PC_DDK_01-1 upper 계열 아이콘이며 모델은 PC_DDK_01 기본 mesh다.

전달 폴더는 `C:/Users/user/Desktop/GBResources`다. Resources-relative 경로를 그대로 유지하고
추출 파일, glTF, 로그, cache, CSO, EXE/DLL은 넣지 않았다. 기존 GBResources의 다른 파일은
보존했다. 설치/전달 manifest는 `out/GuardianOutfits20260927/install-receipt.json`이다.

| 전달 구분 | 파일 수 | bytes |
|---|---:|---:|
| 신규 모델·texture·아이콘 | 156 | 108,597,944 |
| 신규 의상이 참조하는 기존 공유 texture 의존성 | 24 | 12,094,412 |
| 합계 | 180 | 120,692,356 |

두 Resources 위치에서 180개 전부 SHA-256/size가 일치한다. 공유24개도 원본 package에서
추출한 texture와 bytes 또는 decoded pixels가 정확히 같은지 확인했다. 실제 새 catalog가
참조하는 공통 의존성을 함께 전달하여 다른 PC에 병합해도 신규 의상의 texture closure가 닫힌다.

## G04. 검증 증거

- `normalize-report.json`: 15/15 cook, master285, appended0, weighted bind delta0.
- `unique-models.json`: 다섯 의상 upper 및 부위 hash/정점 수.
- `material-receipt.json`: 29 MIC의 정확한 shader pair/packing과 보존한 runtime 보정.
- native1526 `verify`: Base825줄, Light503줄, Configure 재생성 일치.
- `test_source_character_program_groups.py`: 4/4 PASS, cohort/registry 정합.
- `shared-dependency-native-proof.json`, `install-receipt.json`: 원본 texture 및 양쪽 배포 hash.
- `out/GuardianCustomizing20260927/five-choice-structure.json`: 기존5슬롯/다른class/layout 보존.
- 네 변경 JSON parse 및 관련 `git diff --check`: PASS.
- `consumer-test/baseline-report.json`: 실제 기존 두 의상10part의 production reader/service/
  Character/Part_Equipment/CMaterial 경로351 assertions PASS. idle/run weighted palette가
  body 기준과 delta0이며 unknown set/missing model/두 번째 clone 실패도 이전 의상을 보존한다.

최종 다섯 의상의 headless consumer와 native1526 raster 검사 및 최종 Product Debug/Release는
통합 빌드 후 별도로 기록한다. Client/UI 실행·조작은 하지 않았다. 실제 생성 화면의 다섯
의상 외형과 최종 색·광택 판정은 사용자 화면 확인 범위다.

## G05. 현재 소스의 실제 소비자·GPU 추가 검증

전체 다섯 의상에 production `CActorCatalog -> CEquipmentPresentationService ->
CCharacter -> CPart_Equipment -> CModel/CMaterial`을 연결한 별도 no-window EXE를 실행했다.
25개 실제 장착 part,50개 idle/run 자세에서 weighted render palette와 body posed reference의
최대 차이는0이다. 모든 native Base/Light binding 및 의상별 unknown ID·missing model·
두 번째 clone 실패 후 기존 part pointer/occupied mask 보존을 포함해 **879 assertions PASS**다.
현재 ActorCatalog/header를 out에서 좁게 컴파일하고 새 정식 Engine DLL/Light CSO와 현재
Base source의 별도 FXC를 사용한 후보 검사이며, `candidate-*` 증거와
`consumer-test/candidate-verification.json`의 source SHA를 보관했다.

신규1526은 실제 source01 upper6024정점,제품 preTransform(scale0.0001,Y-90도),actual
camera binder와 골격으로 GPU draw/readback을 실행했다.3방향의 유효 pixel은282/247/324,
Base RT0/RT4와 실제 Deferred 직접광의 모든 RGBA non-finite는0이고 직접광은 nonzero다.
`out/Guardian1526Gpu20260927/gpu-finite-report.json`이 입력 SHA와 수치를 소유한다.
이 검사 역시 새 정식 Engine DLL/Deferred1472와 out 전용 Base1472 FXC를 사용했다.
최종 제품 Debug/Release 빌드 및 정식 Client CSO 재실행 결과는 후속 항목에 추가한다.

이후 정식 Debug Product의 Client Base1472 CSO,Engine Deferred1472 CSO,Engine DLL을
독립 복사하고 양쪽 SHA256 동일성을 기록하여 GPU probe를 재실행했다. 종료0,동일3방향
282/247/324pixel 및9개 color-target 표본 모두 non-finite0으로 통과했다.
이 WARP 실행은 `product-run.log`에 보존했다. 이후 같은 정식 CSO/DLL로
`D3D_DRIVER_TYPE_HARDWARE` 검사도 실행하여 종료0,동일 coverage 및 non-finite0을
확인했다. 최신 `gpu-finite-report.json`과 `product-hardware-run.log`는 실제 하드웨어
GPU 실행을 기록한다. Client 화면이나 전체 post-process의 시각 판정을 대신하지 않는다.

## G06. 정식 Debug 산출물 소비자 재검증

최종 Debug Product는 `out/BuildPipeline/runs/20260926T215804868Z-debug-product.json`에서
PASS다. 완료된 `Client/Bin/Debug`의 DLL/CSO 등236개 runtime 입력을 독립 복사하여
5의상 소비자를 `build.py --run-only`로 다시 실행했다. 후보 CSO override는0이며
879 assertions,25개 part,50개 자세,weighted palette 최대 차이0,실패 시 이전 part 보존을
다시 통과했다. `consumer-test/final-verification.json`, `runtime-inputs.json`,
`consumer-report.json`, `run.log`에 증거를 남겼다.

검사 EXE는 현재 ActorCatalog를 별도 컴파일하고 기존 Product Client archive를 연결한
headless fixture다. 이번 재실행으로 정식 runtime DLL/CSO 소비를 확인했으며, 제품 Client
자체를 실행한 것으로 표현하지 않는다. 제품 산출물은 수정하지 않았고 probe는 종료됐다.

최종 Release Product도 `out/BuildPipeline/runs/20260926T223356797Z-release-product.json`에서
PASS다. 의상·눈·머리카락이 소비하는 SourceCharacter 계열72개 CSO의 Debug/Release
SHA256이 모두 같음을 확인했다. 통합 결과와 화면 확인 경계는
[베른·발탄 통합 결과 G05](2026-09-27_BERN_VALTAN_CAPTURE_OPTIMIZATION_RESULT.md)에 있다.
