# 맵과 발탄 원본 입력·표현 복원 구현 계획

## G00. 목표와 확인한 입력

사용자가 카메라 복구 성공을 확인했다. 다음 우선순위는 발탄 전투 공간과 발탄 본체,
Character Select, 베른과 발탄 이펙트, 쿠크세이튼이다. 실제 참고 폴더
`C:/Users/user/Desktop/로스트아크_렌더링`의 PNG 13장을 전부 열람했다.
Character Select는 중앙 장식과 외곽 보행 면의 표현이 다르고, 발탄은 서로 다른 바닥 재질의
연결 범위가 불균일하며, 유령의 마스킹·안개·오라가 추가 조사 대상이다. 다른 화면 비율과
플레이어 위치의 스크린샷만으로 전역 맵 배율이나 원본 캐릭터 배율을 결정하지 않는다.

기존 09-14 카메라·렌더링 PLAN/RESULT와 09-11 발탄 재질 복원을 이어서 확장한다.
`pattern-3`의 기존 미커밋 변경을 보존한다. Client 실행과 화면 판정은 사용자가 수행한다.

## G01. 맵별 FOV와 캐릭터 크기

`ArenaCameraProfile.h/.cpp`의 기존 v1 JSON에 optional `characterSizeMultiplier`를 추가한다.
기존 8필드 문서는 1을 사용하고 새 저장은 9필드를 쓴다. 유한한 0.25~4 범위만 허용하며
잘못된 입력과 외부 수정 시 기존 profile·파일을 보존한다.

`Character.h/.cpp`는 기존 catalog presentation scale에 곱하는 화면 표현 배율을 소유한다.
body/equipment/socket이 사용하는 동일 presentation root에 한 번 적용한다. Server 위치,
충돌, 공격 반경은 기존 권위를 유지한다. 네 Level의 profile 적용과 캐릭터 바인딩이 이를
소비하므로 class 교체·재입장에도 저장값을 적용한다. `MainApp.cpp`는 FOV 바로 아래에
Character size 입력과 catalog 기준 복귀를 제공한다. 카메라 preset은 크기 입력을 보존한다.
사용자가 확인한 Bern 수평 FOV 55도를 저장하며 원본 CDO 50도 preset은 출처와 함께 남긴다.
캐릭터 원본 크기는 설치 모델·cook 단위·catalog·원본 actor scale을 대조한 경우에만 수정한다.

## G02. 발탄 맵의 실제 재질 소비자

기존 StaticMesh의 원본 component effective MIC, static parameter set, vertex COLOR,
UV1과 RNM을 join한다. 기존 source family와 같은 static set인 배치는 같은 material program을
재사용하고, 다른 옵션은 해당 원본 shader 근거를 먼저 확보한다. Authoring mapmaterials,
mapplacements와 Imported mapassets를 갱신하고 기존 publisher로 해당 Area를 설치한다.
중앙 Deploy 바닥은 별도 기존 CDeployPropObject 소비자를 조사하여 CModel/CMaterial 경로에
원본 재질을 연결한다. 밝기 보정만으로 diffuse-only 입력을 복원 완료 처리하지 않는다.

정적 Deploy의 `BossCatalog.modelMaterialOverrides`를 기존 map material parser와 bind helper에
연결한다. `CMapAssetCatalog::Parse_ModelSurface`는 단일 row를 같은 parser로 stage하고 성공한
결과만 반환한다. actor 경로는 per-placement bakedLighting과 비지원 blend/cull을 거부한다.
일반 map parser를 복제하거나 새 모델 런타임을 만들지 않는다.

## G03. 발탄 본체와 이펙트

`CBody_Valtan`과 기존 source skeletal shader에서 유령 program 84의 varying, blend, depth
계약을 확인한다. 기존 source translucent forward pass를 재사용해 ordered coverage에 의한
픽셀 소실을 수정한다. normal/ghost/armor/axe의 서로 다른 cook 단위는 실제 정점·preScale·
socket basis를 대조한다. source material의 내용과 actor 크기를 별개로 검증한다.

현재 패턴·진입·2페이즈·whirlwind·무기·오라·푸른 에테르의 원본 occurrence와 실제 cue/asset
소비자를 연결한다. 기존 Effect_PresentationService와 material ABI를 확장하며, 별도 런타임과
원본 근거 없는 전역 회전·배율은 추가하지 않는다.

LookInfo의 기본 particle은 action notify와 별개로 `defaultParticles`에서 실제 본을 참조한다.
기존 Effect_Playback의 owner-sustained 옵션은 Valtan 기본 오라의 source infinite emitter만
살려 둔다. 유한 emitter는 기존 종료 정책을 유지하며 owner 종료·모델 교체 때 handle을 정리한다.

## G04. 맵별 원본 후처리와 환경

Valtan, Character Select, Bern, Kouku의 WorldInfo/CDO/chain/volume와 LUT 본체를 추출한다.
native ToneScale/Range/Toe와 shader 상수 packing은 원본 CPU 함수·상수 근거로 복원한다.
활성 override와 비활성 속성을 구분하고 기존 RenderingProfileService/Benchmark의
Before/Restored/Return 흐름을 확장한다. 입력만 연결한 adapter와 native 식 복원을 구분한다.
안개·스카이·region·맵 재질과 환경 occurrence는 같은 맵의 원본 참조를 따른다.

기존 Engine quality와 Shader_Deferred에 optional sourcePostProcess 및 LUT bake pass를 추가한다.
원본 UE3 customizable curve와 256×16, 16³ 색 보정 LUT를 사용한다. 기존 pass index는 유지하고
별도 선택이 없는 profile의 Hable 출력은 보존한다. 새 GPU 리소스 생성은 stage 후 교체하며
실패 시 기존 quality와 LUT를 보존한다.

Character Select 중앙 문양은 원본 mesh/UV/MIC/배치 누락 여부부터 확인한다. 베른의 물·나무·
폭포·창문 빛, 쿠크의 렌더링과 맵 크기는 실제 원본 배치·카메라 영역·shader로 대조한다.

## G05. 검증과 인계

새 C++ 런타임 파일은 현재 계획에 없다. 필요 시 실제 소비자와 프로젝트/필터 등록을 함께 추가한다.
변경 JSON/XML parse와 입력 참조, 실제 parser의 저장·실패 보존, 원본 단위·투영·shader 수치,
변경 기능의 최소 컴파일과 정상 Debug Product build, `git diff --check`를 확인한다.
이미 통과한 검사를 근거 없이 반복하거나 광역 하네스를 추가하지 않는다.
RESULT에 실제 반영·원본 근거·검증·남은 기능과 사용자 화면 확인을 분리해 기록한다.
