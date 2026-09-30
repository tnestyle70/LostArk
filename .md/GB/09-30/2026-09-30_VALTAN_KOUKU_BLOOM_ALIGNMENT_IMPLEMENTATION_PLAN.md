# 발탄 기본 Bloom을 쿠크 기본값에 맞추기

## G00. 요청과 현재 기준

사용자는 발탄과 쿠크의 스킬 번짐 차이를 조사한 뒤 발탄의 Bloom을 쿠크와 같게
맞추도록 요청했으며, 현재 디스크 저장본 기준 반영을 명시적으로 승인했다.

LevelRegistry의 발탄 기본 profile은 `scene.valtan.cool-low-key.v1`, 쿠크 기본은
`scene.kakulsaydon.g1.base.v1`이다. 현재 revision 86에서 발탄은 Bloom ON이고
쿠크 기본은 OFF다. V1 문서 intensity는 scene intensity와 추가로 곱하지 않지만
공통 Bloom enable과 threshold, knee 및 최종 tone/LUT를 소비한다.

## G01. 정본 데이터의 변경 범위

`Data/Rendering/Authored/RenderingProfiles.json`에서 stable profile ID로 발탄 기본
profile을 찾는다. 다음 필드만 쿠크 기본 profile의 현재 값으로 맞춘다.

| 위치 | 필드 | 이전 | 적용값 |
|---|---|---:|---:|
| qualityOverride | bloomEnabled | true | false |
| qualityOverride | bloomThreshold | 2.74000001 | 1 |
| qualityOverride | bloomSoftKnee | 1 | 0.5 |
| qualityOverride | bloomIntensity | 0.5 | 0.800000012 |
| profile | bloomIntensityMultiplier | 1 | 0 |
| 두 environmentRegions의 postProcess | bloomThreshold | 2.74000001 | 1 |
| 두 environmentRegions의 postProcess | bloomIntensity | 0.5 | 0.800000012 |

두 region ID는 `valtan.ps.environment.31.convex.0`과
`valtan.ps.environment.31.convex.1`이다. Bloom scatter 1과 white tint는 이미
같으므로 유지한다. revision은 최신 저장본에서 1 증가시킨다.

노출, gamma, tone, LUT, 조명, 안개, SSAO/FXAA, 개별 Effect 문서와 타 profile은
보존한다. 발탄 before-restoration/source-rendering 비교 profile도 유지한다.
쿠크의 지역별 모양이나 조명을 발탄에 복제하는 작업은 아니다.

## G02. 저장과 게시

최신 source/runtime을 백업하고 JSON 후보를 만든다. 기존 publisher의 Validate와
Publish를 사용하여 후보 runtime을 생성한다. 교체 직전 원본 hash를 재확인하며,
동시 변경이 있으면 덮어쓰지 않고 최신 저장본으로 다시 병합한다. 설치는 파일별
원자 교체를 사용하고 실패하면 자기 변경만 hash 확인 후 rollback한다.

정식 publisher는 `Tools/RenderingPipeline/Publish-RenderingProfiles.ps1`이다.
게시 목적지는 `Client/Bin/DataFiles/Rendering/RenderingProfiles.runtime.json`이다.
새 C++/shader/project 등록은 없으며 컴파일을 요구하지 않는 데이터 튜닝이다.

## G03. 검증과 사용자 확인

기존 publisher의 source Validate, staged Publish 및 게시본 Validate를 수행한다.
source/runtime의 해당 profile 값, revision과 요청 외 필드 보존을 검사하고
`git diff --check`를 실행한다. 기존 dirty 변경은 stage/commit하지 않는다.

Client/UI는 에이전트가 실행하거나 조작하지 않는다. 사용자는 Rendering Workbench에서
`Reload Runtime` 후 Benchmark의 `Rendering restoration`에서 발탄 기본 profile과
`Bloom off`를 확인한다. 게시 파일 반영과 실행 중 메모리 Reload, 최종 화면 판정은
별개이며 실제 실행한 검증만 RESULT에 기록한다.
