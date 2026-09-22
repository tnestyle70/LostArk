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
