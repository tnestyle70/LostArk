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
Release Product 빌드·배포도 `out/BuildPipeline/runs/20261003T185650391Z-release-product.json`
에서 PASS다. 두 구성 모두 실제 Build를 수행했으며 Debug 144.287초, Release 145.777초다.
기존 C4819/C4828 인코딩 및 Release 외부 DirectXTK PDB 경고는 남지만 컴파일·링크 오류는 없다.
빌드 후 카메라 설치 hash가 유지됨을 확인했다. Product의 runtime file/navigation/reward 검사는
통과했으며 Client/UI 실행과 최종 구도·모델 표시 확인은 수행하지 않았다.
독립 검토는 제품 변경 commit `0df4567904c11be66ab41c290bd03d953f10307d`에서 PASS이며
필수 수정 결함이 없었다. 이후 변경은 이 빌드 결과와 기존 세 기능의 반영 상태 문서뿐이다.

## G05. cam05_a 추가 구도 반영

사용자가 추가 저장한 워로드 `cam05_a`는 Intro Camera2 첫 0ms 키의 Eye·LookAt·Up만
변경된 상태였다. 직전 설치본 `e1c509...`와 main `b084c9dbc`의 대상 Intro/Loop 행이 같음을
확인하고 그 직전 곡선에서 새 구도로 보정했다. 새 기준 Eye는
`(-2105.0986328125, 1.7545170783996582, 2105.798828125)`다.

Intro94키·Loop95키 중 저장한 첫 키 하나를 그대로 유지하고 나머지188키의 pose564필드만
바꿨다. 첫0→24ms의 Eye 이동은2.8998513m에서0.000912546m로 줄었다. 첫 구도 뒤에
옛 경로로 돌아가는 불연속을 제거했으며 경로 길이·시간·FOV·hard cut은 유지했다.
다른 워로드 컷·창술사·제외 목록을 포함한 모든 비대상 필드가 그대로임을 확인했다.

실제 현재 제품 parser로 전체5scene을 읽고 두 컷의12,600 source-ms를 sample하여
failures0을 확인했다. 예상 변환 대비 최대 Eye 오차는0.000452723m, 시선0.00350746도,
Up0.000394554도다. 첫1ms 최대 Eye 이동은0.00000107288m다. 증거는
`out/MovieCam05Refine20261004`의 before/baseline/candidate, stable-field patches,
candidate-validation 및 native-run/provenance다. 이전 증거는 덮어쓰지 않았다.

2026-10-04 04:35 KST에 최신2행 의존 검증 후564필드 구간만 원자 교체하고 실제 교체본을
백업했다. 저장본 SHA256은 `91f04aae027f38bdb5de569a95b93fa66353baa94e62145a24000095cd7b713f`,
설치 SHA256은 `72a43675afc9f61077e0f52c617d65494a9839cca5e911aea10b9a71b88c47d5`다.
실제 설치본의 동일 Parse/Sample 재검사도 통과했다. 근거는
`out/MovieCam05Apply20261004/install-receipt-20261004T043519-1791056119507399700.json`과
`installed-native.log`다. Git에는 이2컷과 사용자 기준 키만 포함하고 다른 사용자 저장분은
작업 디스크에 보존한다. C++/shader 변경·추가 Product 빌드·publish·Client 종료·Reload는
수행하지 않았다. 사용자는 `Reload saved movie → Play All`로 새 경로를 확인한다.
