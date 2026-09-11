# 쿠크 특수 표면 연결과 베른 원본 크기 조사 구현 계획

## G00. 범위와 실측

사용자는 기존 쿠크 재질 두 개의 화면 결과를 승인했고, 전체 맵·보스·연출용 모델·공·망치의 나머지 재질과 환경광 복구를 요청했다. 다른 작업이 끝난 뒤 반영하라는 이전 대기 조건은 이번 구현 요청으로 해제됐다. 기존 미커밋 변경은 `out/KoukuFullRestore20260911/baseline.patch`와 status에 보존한다. Client/UI는 실행하지 않으며 최종 화면은 사용자가 확인한다.

일반 맵 표면은 기존 [전체맵 계획](../09-09/2026-09-09_KOUKU_FULL_MAP_MATERIAL_RESTORE_IMPLEMENTATION_PLAN.md)의 BG8 경로를 확장한다. 이 문서는 그 경로에 덮어씌울 수 없는 반투명 표면 4 MIC, spotlight 8 MIC, sky 1 MIC와 베른 크기의 실제 비교 결과를 소유한다.

## G01. 원본 특수 표면의 실제 소비자 연결

원본 `surface_program_groups.json`의 5·19·34·40·45 그룹에 해당하는 선택된 PS를 기존 native material constant/texture 계약으로 옮긴다. 신규 프로그램 33~37은 `source.map.*` 이름을 사용한다. 정적 모델도 기존 `CModel -> CMaterial` 로드와 texture lifetime을 사용하며 별도의 모델 런타임은 만들지 않는다.

- `Shader_SourceMapForwardPrograms.hlsli`: 원본 명령별 HLSL과 forward용 입력 배치를 소유한다. 기존 generic native input/output 및 texture bindings를 재사용한다. 원본 선택된 계산과 현재 scene가 제공하는 카메라·깊이·환경 입력을 구분한다.
- `SourceMapForwardMaterialParameters.h`: 원본 named parameters에서 기존 native constant 배열로 pack한다. 알 수 없는 parameter나 누락 texture를 정상 입력으로 처리하지 않는다.
- `MapAssetCatalog.cpp`, map publisher: 새 family의 정확한 identity·parameter·texture를 검사하고 기존 override stage에 넣는다. 실패하면 기존 catalog를 보존한다.
- `MapAssetRenderUtils.cpp`: 이미 존재하는 `Bind_SourceCharacter` 분기를 통해 native material을 제출하고 forward가 요구하는 scene 입력을 추가 bind한다.
- `Shader_VtxMeshBinary.hlsl`: 실제 fallback map object가 사용하는 Alpha/Sky pass에서 native forward 결과를 소비한다. 기존 BG8/deferred, water, presentation vortex는 각자의 소비 경로를 유지한다.
- Client project/filter는 새 header와 shader include만 등록한다. Engine에서 재사용되는 include는 Engine shader 복사 경계에도 등록한다.

원본의 additive PS가 이미 RGB에 opacity를 곱한 경우 현재 `SrcAlpha+One` 합성에 맞춰 alpha를 1로 제출한다. 원본 alpha 0을 일반 투명도로 해석하여 조명 기둥을 지우지 않는다. scene-depth fade는 실제 G-buffer depth에 연결한다.

## G02. 베른 원본 축척과 설치본 비교

원본 16 package의 정적 배치 32,324개를 다시 읽었다. authoring의 모든 배치가 원본 위치·quaternion·signed scale과 일치하며 최대 오차는 위치 0.000005m, quaternion 4.998e-10, scale 4.955e-8이다. 원본 정적 메시 950개의 실제 LOD0 정점 범위도 설치 WModel과 전부 일치한다. 원본 cm 좌표와 설치 모델 cm 좌표를 비교하고 Loader의 0.01m 변환을 확인한다.

현재 authoring/runtime 배치 50,017개가 전부 동일하다. 원본 축척이 이미 일치하므로 근거 없는 전체 배율 변경은 하지 않는다. 별도 foliage/landscape와 현재 화면의 camera framing·표시 상태는 정적 배치/메시 일치와 구분해 결과에 기록한다. 캐릭터의 사용자 저작 `presentationScale`도 원본 맵 축척으로 오인하지 않는다.

## G03. 검증

선택 PS 원본과 변환된 HLSL의 수치·texture/parameter 연결, 해당 shader FXC compile, 최소 Product 빌드, map publisher Validate/Publish/Check, 변경 JSON/XML parse 및 `git diff --check`를 수행한다. 실제 실행한 항목만 RESULT에 기록한다. 원본 구운빛·캐릭터 수광·특수 표면·베른 조사 결과는 각각 근거와 미확인 경계를 분리한다.

실제 compiled Effect를 CShader로 열고 material bind까지 확인한다. 이번 native 프로그램 증가로 Debug `/Od`의 AnimMeshBinary/ MeshBinary 산출물이 기존 256MiB 제한을 넘는 것을 확인했다. 이미 일부 Effect에서 사용하는 Debug x64 `/O1`을 두 binary mesh shader와 공통 Deferred shader에 적용하고, 같은 세 shader에 중복된 소스 디버그 정보(/Zi) 내장을 끈다. C++ Debug/PDB는 유지한다. C++ Debug 설정과 CShader 입력 크기 검사는 유지하고 최종 Product 산출물로 다시 로딩한다.
