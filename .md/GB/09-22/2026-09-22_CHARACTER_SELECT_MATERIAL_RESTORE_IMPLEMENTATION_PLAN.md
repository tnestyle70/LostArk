# Character Select 누락 재질 연결 구현 계획

## G00. 현재 데이터와 변경 경계

현재 `LV_LOBBY_CLASSSELECT_SL00`은 catalog209, placement804(visible772), source material247행,
placementLighting731행이다. 오래된 문서의9행/29배치는 현재 상태가 아니다. 실제 visible45배치의
8개 모델,15개 effective MIC에 source material 연결이 없으며 모델 실물 누락은0이다.

원본 MIC의 명시 NULL texture는 추측한 흰 텍스처로 대체하지 않는다. 기존 원본 MaterialMap의
texture expression이 증명한 fallback과 현재 공용 source material compiler를 연결한다.
재질이 여러 slot인 모델은 한 slot만 성공한 상태로 배치를 교체하지 않는다.

## G01. 누락 surface와 배치별 입력

`Tools/LevelPlacementExtractor`의 기존 resolver/compiler, 원본 StaticMeshComponent RNM,
현재 WModel의 named slot·UV1을 결합한다. 신규 재질 variant만 Imported catalog에 추가하고,
기존 stable source placement ID의 asset ID만 교체한다. 기존 transform·visibility·사용자 편집,
09-17에 제거된 editor 별 배치와 Floor Swap 동작은 보존한다.

변경 정본은 해당 Area Imported `.mapassets`, Authoring `.mapplacements`와 `.mapmaterials.json`,
`Data/Maps/MapCatalog.json`의 해당 Area count다. 기존 material compiler의 surface family로41배치를 연결하고, 남은4배치의 원본 shader
3종은 native program208/209/210을 사용한다.209는 기존 source-map forward,208/210은
기존 source-character static-map cohort 경로를 확장한다. 신규 C++ class는 없으며
새 HLSL leaf와 cohort208 wrapper는 Client/Engine 프로젝트와 filters에 등록한다. 새 리소스는 runtime Resources 설치와 함께
`C:/Users/user/Desktop/GBResources/Map/`의 같은 상대 경로로 전달한다.

## G02. 중앙 문양의 구조 확인

기존 원본 geometry 대조에서는 FLOOR12·BRIDGE01E의 native LOD·삼각형 누락이 없었다.
원격 직업별11쌍과 중앙 조립체는 별개의 배치다. 원작의 물방울·삼중 곡선은 원본 carrier와
실제 표시 조건을 찾은 경우에만 연결한다. 유사 모델 추가·다리 삭제·일괄 높이 보정으로
미식별을 숨기지 않는다. 조명·그림자는 병행 작업의 소유 범위이므로 공통 renderer를 수정하지 않는다.

## G03. 검증과 전달

실제 source slot 전수 join, 원본 package/texture hash, WModel named material·UV 채널,
runtime resources 참조와 기존 Area `Validate → Publish → Check`, JSON parse,
`git diff --check`로 변경을 확인한다. 가능하면 기존 비UI CMapAssetCatalog→CModel 검사를 재사용한다.
후속 통합 빌드는 상위 작업과 조율한다. Client/UI 실행·캡처 및 최종 화면 판정은 사용자가 수행한다.
RESULT에는 설치·게시·소비자 검증과 원작 미확정 입력을 분리한다.

## G04. 장면 전체의 색 합성과 실제 픽셀 입력

사용자 추가 요청으로 문양보다 장면 전체의 노란 분위기와 Rendering Benchmark를 우선한다.
기존 G02의 조명 경계는 이 추가 요청에 따라 필요한 공통 합성 경로까지 확장한다.
노란색 전체의 단일 원인은 아직 확정하지 않았다. 원본 diffuse는 따뜻하지만 RNM 평균은
거의 중립이고 환경 큐브는 차가운 경향이다. 원본 DXBC 수식 검증 후보와 제품을 대조하면
제품 RNM에는 `(1-metallic)`과 `(1-reflectionBRDF)`가 누락되어 있다. 이 알려진 에너지
분배 항은 복구하되 미식별 SH/hemisphere owner 입력을 임의의 흰색·파란색으로 만들지 않는다.

`Engine_RenderTypes.h`의 session material 설정에 현재 Level 전용 PBR 비교 값과
RNM/IBL/직접 확산광 및 전체 SceneHDR/톤 이후/그레이딩 이후 보기를 추가한다.
`Renderer.cpp`는 유효 범위를 검증한 뒤 commit하고 실제 light/final pass에 전달한다.
`MapAssetRenderUtils`는 실제 성공한 바인딩의 surface/RNM 입력을 짧은 수명의 읽기 전용
snapshot으로 제공한다. `RenderingBenchmark`는 직접광 두 항·RNM·IBL 기여도와 노멀·거칠기,
이전 RNM 식 비교를 제공하며 캡처 조건에도 이 값을 기록한다. 도구 종료·Level 전환 때
비교값을 해제하고 Save/Publish 및 다른 Level의 원본 값을 변경하지 않는다.

`MainApp.cpp`의 긴 material binding 표는 제거한다. Benchmark에는 전체 장면의 합성 단계,
현재 유효한 전역 수치, 선택한 한 PBR surface의 실제 입력과 비활성·미연결 상태를 표시한다.
셰이더 상수와 텍스처에서 픽셀마다 샘플되는 값은 구분하고 CPU 바인딩을 GPU 픽셀 실측으로
설명하지 않는다. 새 C++ 파일은 필요하지 않다. HLSL은 기존 Engine/Client 미러를 유지한다.

검증은 수정 TU 컴파일, FXC, 원본 수식 대조와 설정 경계 검사, 정식 Product 빌드 및
`git diff --check`다. Client와 최종 색감의 화면 검증은 사용자에게 남는다. GI/Nanite/Lumen
추가를 이 결함 수정의 완료 조건이나 원본 동등성 근거로 삼지 않는다.

## G04. 재개 이후 확인된 PBR 간접광 보상과 Rendering Benchmark

사용자 우선순위에 따라 중앙 문양의 추가 조사를 중단하고, 재질과 장면 전체가 노랗게 보이는
원인을 분리한다. 원본 DXBC와 동등성을 확인한 RNM 수식의 `(1-metallic)` 및
`(1-reflectionBRDF)`가 현재 부분 복구식에서 빠져 있음을 확인했다. 해당 보상을 복구하고
기존식을 선택하는 명시적인 비교 옵션을 함께 제공한다. 실제 lookup이 없는 material에는
가짜 값을 만들지 않고 metal mask만 적용한다. SH·hemisphere 및 native BRDF payload의
미확정 경계는 유지한다.

PBR 전용 direct diffuse/specular/RNM/IBL gain, normal scale·roughness offset을 기존
Rendering settings와 Benchmark에 연결한다. shader에서는 새 RenderTarget 없이 marker3의
기존 CharacterGeometry에 RNM+IBL, CharacterSurface에 IBL을 저장한다. 진단8/9/10은
RNM/IBL/direct diffuse,11/12/13은 marker와 관계없는 SceneHDR/tone/grading 단계를 표시한다.
직접광·간접광 계측과 전체 장면 색 변환 진단을 구분하며, directional 색이나 LUT를 추측해
덮어쓰지 않는다. C++ settings/binding/Benchmark는 상위 작업이 통합하고 shader 변경은
현재 작업에서 담당한다.


## G05. before-restoration.v1 비교 선택 재노출 (2026-09-23)

`RenderingBenchmark.cpp::Render_RestorationSection`의 기존 Before 버튼을 저장된
`before-restoration.v1` ID로 명확히 표시한다. 해당 baseline을 이 Workbench가 소유한
동안 같은 버튼은 진입 profile로 돌아가는 동작을 제공한다. 복원 source profile의
선택 상태와 두 full profile ID도 표시한다. 기존 session profile transaction,
실패 시 현재 상태 보존, 종료·Level 변경·외부 owner 변경 처리 경로를 재사용한다.

이 선택의 범위는 scene profile의 light, environment, shadow, fog, post-process와
map-light multiplier다. 설치된 geometry·source material equation/pass·map light 배치와
Effect 자료를 과거 버전으로 되돌리는 기능으로 설명하지 않는다. 별도의 live comparison은
유효한 채 유지되므로 활성 시에는 화면에 그 상태와 Reset 경로를 알린다.

신규 H/CPP나 프로젝트 등록은 없다. RenderingBenchmark TU 컴파일과 변경 diff를 확인하고,
Client/UI 실행·실제 버튼 클릭·최종 화면 비교는 사용자 확인으로 남긴다.


## G06. cubeDiffuse의 선택적 scene profile 계약 (2026-09-23)

`RenderingProfileService.h/.cpp`의 environment에 `cubeDiffuse`를 선택적으로 연결한다.
model은 `RGBM6_LAMBERT_SH3`, intensity는 유한한0~4, packedSH는7행의 float4이며
각 성분은 유한한−64~64, 마지막 행w는0이다. 이 입력은 RGBM6 cooked cube를 적분한
프로젝트 근사이며 원작 native SH9 packed 입력으로 설명하지 않는다.

SCENE_RENDERING_PROFILE은 presence flag,7개 SH행과 intensity를 소유한다. parser가
완전한 block만 stage하고 Validate_Profile은 직접 메모리 편집도 같은 범위로 검증한다.
명시적으로0인 block과 미선언을 구분해 Serialize_Catalog의 Save roundtrip을 보존한다.
기존 Stage_RenderEnvironment가 성공한 뒤 같은 stagedEnvironment로 계수를 전달하고
기존 Commit_Resolved 성공 시점에만 교체한다. 미선언 profile은 간접광 intensity0을 유지한다.

`Publish-RenderingProfiles.ps1`은 같은 shape/model/finite/reserved 계약을 확인한다.
알 수 없는 필드·누락·잘못된 행 수·범위 오류를 이전 게시본 교체 전에 거절한다.
새 H/CPP나 프로젝트 등록은 없으며 Data/Resources와 실제 runtime 게시본은 수정하지 않는다.
검증은 수정 TU 컴파일, 기존 publisher 테스트의 선택적 roundtrip/실패 보존 확장과
`git diff --check`다. GPU diffuse 소비와 전체 Product 빌드는 상위 통합 범위다.

## G07. 큐브 확산 간접광의 합성·비교

원본 FLOOR12 DXBC의 mad_sat가 확인되므로 albedo의0~1 제한은 유지한다. 원본 PS
WorldInfo/CharacterCloseupScene의 활성 LUT 목록도 비어 있으므로 중립 LUT를 유지한다.
하늘 mesh와 이미 연결된 반사용 cube를 구분한다. 원본 cube의 RGBM6 선형 값을 적분한
Lambert E/pi의7행을 선택 scene profile로 전달하며 원본 serialized SH의 대체 복원으로
표기하지 않는다. 원본 static RNM에 포함된 하늘 비중은 미확정이므로 추가량 비교를 제공한다.

`RENDER_ENVIRONMENT_STATE`는 계수와 intensity를 소유한다. `Renderer::Render_Combined`가
한 번 바인딩하고 marker3 PBR 표면에만 albedo×(1-metallic)×(1-F0)×materialAO로 더한다.
직접광 그림자와 dynamicBakedShadow는 이 하늘 항에 다시 곱하지 않는다. screenAO와 fog는
기존 합성 순서대로 적용한다. native 캐릭터·일반 발탄·emissive와 기존 RNM/IBL 수식은 유지한다.

`RenderingBenchmark`는 cube diffuse 단독 보기와0~4 session gain을 제공한다. 기존
capture 조건 기록·닫기·Level 전환 복구에 이 값도 포함한다. 기본 미선언 profile은0이며
before-restoration 프로필은 이전 조명을 유지한다. 후보는 현재 Character Select 프로필의
environment에만 추가하고 최신 디스크 저장본의 최종 적용 승인을 받은 뒤 merge/publish한다.
새 파일/프로젝트 등록은 없다. 실제 SH 수치/합성 shader와 carrier roundtrip을 검사하고,
Product 빌드는 점유된 EXE/DLL을 교체할 수 있을 때 수행한다. Client 화면 판정은 사용자에게 남는다.


## G08. 원본 PBR 간접광 owner와 계산 복원 (2026-09-23)

G07의 cooked cube 추가광은 사용자 화면에서 원작 색 복원에 실패했다. 베른·발탄·쿠크의
공용 map PBR 행은0이고 Character Select SL00은182행이므로 다른 맵의 RNM 복원 성공을
이 경로의 완성으로 간주하지 않는다. actual FLOOR12_01 MIC의 static key를 현재 원본
RefShaderCache와 재join했고, 원본 PS82f66791의 계산이 실제 대상임을 확인했다.

EFEngine mapped CPU binder0x5f0c30의 serialized SH9→packed7을 사용한다. 286개 입력에서
원본 함수가 보낸112byte와 독립 float32 구현이 완전히 같았다. 이 계수는 G07 Lambert
근사와 별도의 native 계약이며 마지막 행w는1이다. Component environment alpha는
shader floor가 아니며 실제 SetMesh는 environmentColor.w=0을 전송한다. 원본 회전은
sin(angle),cos(angle) 순서이며 component120도와 global0도를 구분한다.

기존 CModel→CMaterial surface에 선택적인 environment.sourceIndirect 입력을 연결한다.
이 block은 UE3_NATIVE_PBR 모델 식별자, 별도 원본 cube/color/rotation/BRDF, native packedSH7행, 원본 primitive의 upper/lower
sky color와 ambientAndSkyFactor를 소유한다. 미선언은 기존 경로를 유지하고 유한 값·행 수·
예약w를 parser, publisher, 직접 CModel override에서 모두 검증한다. 원본 hemisphere의
setter와 실제 primitive light policy를 join한 값만 후보에 싣는다.

Rendering profile environment.useSourcePBRIndirect 선택값으로 기존 profile transaction을
통해 식을 선택한다. 미선언/false는 기존 수식이며 before-restoration은 이 값을 추가하지
않는다. 켜진 profile에서 sourceIndirect가 있는 PBR만 SH×(RNM+hemisphere), 원본 AO와
reflection energy, cube specular를 소비한다. direct light와 다른 material family는 유지한다.
사용자 화면에서 실패한 cubeDiffuse 추가량은 두 복원 profile의 후보에서0으로 설정한다.

CreatePBRPreintegratedGFTexture의 실제128×32,128sample,RG16_UNORM 생성 알고리즘을
복구한 DDS 후보를 만든다. 기존 CMaterial에 원본 BRDF와 cube SRV를 별도로 보존해 이전 모드의
리소스를 유지한다. 원본 color LUT와 BRDF lookup을 혼동하지 않는다. LUT owner가 비어 있다는 단일 근거로 전체 원작
postprocess가 중립이라고 주장하지 않는다. 새 파란색 LUT나 albedo tint 보정은 만들지 않는다.

현재 data를 직접 덮어쓰지 않고 stable asset/material ID별 후보를 만들고 실제 FLOOR12
DDS·RNM·SH·lookup으로 GPU 비교한다. schema 실패 rollback, 관련 TU/FXC, mirror 일치와
변경 diff를 확인한 후 최종 저장본에 필드별 병합·backup·CAS·원자 교체·domain publish한다.
점유된 제품 EXE/DLL을 링크할 때만 종료가 필요하며 Client 화면은 사용자가 확인한다.
새 C++ 파일은 필요하지 않는다. 적용 전 완성된 후보 전문은 대응 DETAIL_PLAN에 보존한다.

G08 추가 owner 확인: 실제 PBR placement 586개는 Sky1087이 이미 구워졌거나 lighting channel이
달라 추가 반구광이 모두0이다. 전역 .15를 더하지 않는다. 제품의 기존 cube 행17개와 환경
미연결165개를 원본 owner로 재join한다. 실제 사용181행은 명시적 cube18행과 global163행이며 미배치1행은 제외한다. 새 연결의 legacyEnabled=false는 native 모드
외에서 이전의 환경 없음 상태를 유지한다. 원본 global owner가 확정된 cube/SH만 데이터에 싣는다.
