# 저장한 워로드 Movie 구도 기준 이동 경로 보정 결과

## G00. 최신 저장본과 확정 범위

사용자가 저장한 워로드 Intro Camera2·4·5의 첫 0ms Eye·LookAt·Up을 기준으로 보정했다.
후속 확인에서 창술사는 이번에 편집하지 않았다고 명시했으며, 실제로 LANCE_MASTER의
Intro/Loop 전체가 HEAD의 기존 09-30 G23 보정본과 일치했다. 창술사에는 재적용하지 않았다.

저장본 SHA256은 `6b73a904d0ff3a04bee6287e0642ab3a8602c3ef82ed7fd3fbbb464ea621bd6a`다.
Intro2의 첫 24ms Eye 이동은 4.6503m, Intro4의 첫 560ms는 3.4637m, Intro5의 첫 8ms는
3.0258m였다. 새 첫 키 이후에 이전 곡선이 남아 원래 구도로 급히 돌아가는 상태였다.
카메라는 WORLD 행이며 runtime은 identity root와 기존 source clock으로 샘플한다.
좌표계·시간 소비 코드를 변경할 문제가 아니므로 데이터 pose만 보정했다.

## G01. 후보의 실제 변경

09-30 G21/G23과 동일하게 직전 설치본 HEAD의 컷별 첫 basis와 현재 저장한 첫 basis 사이의
회전으로 상대 Eye 경로·시선 벡터·Up을 변환했다. 새 첫 Eye를 평행이동 기준으로 사용했다.
시선 길이만 새/기존 첫 길이의 일정 비율로 맞췄으며 Eye 경로 거리와 FOV는 확대하지 않았다.
대응 Loop는 자기 기존 첫 키·곡선·FOV·시각을 사용해 같은 새 구도에 맞췄다.

| 컷 | Intro 키 수 | Loop 키 수 | 보존한 사용자 기준 |
|---|---:|---:|---|
| Camera2 | 94 | 95 | Intro2 첫 키 전체 |
| Camera4 | 17 | 17 | Intro4 첫 키 전체 |
| Camera5 | 42 | 42 | Intro5 첫 키 전체 |

대상은 6컷 307키이며 첫 키 3개를 exact 보존하고 나머지 304키의 Eye·LookAt·Up,
총 912개 vector 필드만 바꿨다. 모든 ID·timeMs·FOV·FOV axis·cut·curve/easing·source·clock,
repeatMovie·holdAfterCameraId와 다른 클래스/컷은 그대로다. 저장본에 있던 차원술사 제외 목록
8개 추가 등 비대상 사용자 변경도 후보에 정확히 유지했다. 기존 컷 경계의 즉시 전환은 유지하며
새 컷 사이 블렌딩이나 무봉합 이동을 추가하지 않았다.

후보 SHA256은 `40d5aa75c280431f5223358b131dbb8160f17d184284caa0e7965c61112675a1`다.
설치 대상 필드는 stable classId/phase/cameraId/keyId로 기록했고 각 대상 행의 입력 의존 hash와
필드별 before/after를 함께 남겼다. 후보 생성 전후 원본 디스크 bytes가 같음을 확인했다.

## G02. 실제 parser와 sampler 검증

현재 `CClassSelectionPresentation::Parse`와 phase/effect/material/light helper,
`CEffectRecoveryCamera::Parse/Validate/Sample`, `CValtanCinematicCameraController::Sample_Cue`의
실제 본문과 실제 헤더를 사용했다. `DataJson.cpp`, `SourceCharacterMaterialParameters.cpp`도
현재 제품 TU를 컴파일했다. parser/sampler stub은 없으며 GPU·UI·렌더 객체 수명은 실행하지 않았다.
MSVC14.44 Debug CPU 콘솔의 컴파일·링크·실행이 모두 exit0이다.

- 직전 HEAD, 사용자 저장본, 후보 전체 JSON 모두 5 scene의 제품 Parse 통과.
- 대상 6컷 전체를 source 1ms 간격으로 55,132번 sample하여 failures 0.
- double 기준 회전/이동과 float runtime의 최대 Eye 차이는 0.000522568m,
  시선 방향 0.00369278도, Up 0.000510998도다.
- 첫 0→1ms 최대 Eye 이동은 0.0000194311m다. 사용자 첫 키 직후 이전 구도로 돌아가는
  큰 이동이 제거됐음을 수치로 확인했다.
- double 키 간 Eye 거리 보존 최대 오차는 5.15e-13m다. 키 시간과 runtime FOV는 같았다.
- stable-field patch 912개를 저장본에 재적용한 문서와 후보 전체가 deep-equal이다.
  기준 3키와 LANCE_MASTER 전체를 포함해 모든 다른 필드가 정확히 보존됐다.

증거는 `out/MovieCameraSavedPose20261004/`의 `before.json`, `baseline.head.json`,
`candidate.json`, `stable-field-patches.json`, `transforms.json`, `candidate-validation.json`,
`native-provenance.json`, `native-compile.log`, `native-run.log`다.
이 표본 수는 별도 기능 검사나 화면 합격 수가 아니다.

## G03. 설치와 화면 확인 경계

2026-10-04 03:45 KST에 최신 저장본과 대상 6행을 다시 대조하고 912개 vector 필드의
문자열 구간만 변경하여 비대상 bytes를 보존했다. Windows ReplaceFileW로 원자 교체했으며
실제 교체된 백업 bytes가 직전 저장본과 일치함을 확인했다. 교체 경쟁 시 자기 변경만
rollback하는 보호를 유지했다. 설치 SHA256은
`e1c5099083f005d1507a4050e14fb668da584815aff2fc460b6501d445874f3b`다.
`native.exe Data/Camera/ClassSelection.cinematics.json`으로 실제 설치본을 다시 읽어
5 scene Parse와 55,132 source 시점 검사를 동일하게 통과했다. 근거는
`out/MovieCameraApply20261004/install-receipt-20261004T034558-1791053158564074800.json`,
그 receipt의 actualBackup과 `installed-native.log`다.
무관한 동시 저장 필드 보존, 같은 카메라 행의 동시 수정 거절, 후보의 비대상 변경 거절도
`install-guard-checks.json`으로 확인했다. Git에는 워로드 6컷과 사용자가 수정한 기준 키만
포함하며 차원술사 제외 목록 등 다른 사용자 변경은 작업 디스크에만 보존한다.

카메라 source-only 변경은 World runtime publish를 요구하지 않는다. 실행 중 메모리 draft를
자동 변경하거나 Reload하지 않았으며 Client/UI 실행·조작·화면 캡처도 하지 않았다.
사용자는 WORLD 편집기의 Reload saved movie 후 워로드 Play All로 구도와 기존 hard cut을
확인한다. 최종 구도·시각적 자연스러움은 사용자 확인으로 남는다.

## G04. 통합 제품 빌드

사용자가 Client/Server 종료 후 Debug·Release 반영을 명시적으로 승인했다.
Debug Product 빌드·배포는 `out/BuildPipeline/runs/20261003T185221321Z-debug-product.json`
에서 PASS이며 Engine, Shared, Server, Client와 shader 배포를 포함한다.
렌더링 A/B, Effect Sequencer DPI, Effect Camera 자유 포즈 캡처 및 이번 Movie 모델 선택·위치,
워로드 native702 불투명도 수정이 함께 포함된다. 데이터 publisher를 자동 실행하지 않았다.
Release Product 결과는 다음 검증이 끝난 뒤 같은 절에 기록한다.
