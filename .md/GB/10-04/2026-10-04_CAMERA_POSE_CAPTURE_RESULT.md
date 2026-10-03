# Effect Sequence Camera 자유 시점 포즈 캡처 결과

## G00. 반영한 호출 경로

`EffectAuthoringSequencer_Camera.cpp`의 Recovery Camera 편집기가 기존
`CSequenceCameraEditor::Render`에 자유 카메라 캡처 callback을 전달한다. 공통 편집기는
Model root 행의 일괄 disabled를 제거하여 기존 `Use free cam pos` 버튼을 표시·사용한다.
`SequenceCameraEditor.h`에 callback의 행 좌표계 변환 책임과 실패 시 출력 보존을 명시했다.

callback은 실제 `CCamera_Free`의 Follow requested와 presentation override를 확인하고
F6 자유 시점에서만 공통 `CCameraTool::Capture_ViewPose`를 호출한다. Model root 행은
활성 preview model과 Model root 모드를 확인한 뒤 기존 `Resolve_Root`를 사용한다.
Eye·LookAt은 inverse root의 좌표 변환, Up은 방향 변환으로 원본 행의 좌표계에 넣는다.
World 행은 캡처 포즈를 그대로 전달한다.

추가한 `Camera_ToModelPose`는 root와 inverse의 NaN/Inf, determinant 0, 변환된 basis를
검사한 local candidate만 출력에 commit한다. 고정 determinant epsilon을 쓰지 않아
0.001 preScale의 1e-9 determinant도 허용한다. `Set_KeyPose`의 기존 후보 행 검증으로
좌표 범위와 원본 키 조건을 확인한 뒤 Eye·LookAt·Up만 commit한다.

키 ID·시간·FOV 값/축·Cut before·보간·easing과 다른 키는 유지한다. 캡처 projection이
수직 10도 미만이어도 기존 공통 runtime 범위를 사용하며 선택 키의 FOV에는 덮어쓰지 않는다.
camera owner ID·priority, F6 동작, 기존 Capture key at cursor의 키/FOV 생성은 변경하지 않았다.
Save camera source와 Publish saved cameras는 계속 별도 명령이다.

## G01. 실제 검증

- MSVC14.44, 기존 Debug 옵션 `/Od /MDd /RTC1 /JMC /std:c++20` 및 `/utf-8`로
  `EffectAuthoringSequencer_Camera.cpp`, `SequenceCameraEditor.cpp` 독립 컴파일 exit0.
  기존 EngineSDK 헤더의 C4828 인코딩 경고가 있으며 컴파일 오류는 없다.
- 현재 실제 `Capture_ViewPose`, `Camera_ToModelPose`, `Set_KeyPose`, Recovery Validate/Sample,
  cinematic Sample_Cue 본문과 실제 POSE/KEY/CUE/ROW 헤더를 사용하는 native probe는
  **148 checks, failures 0**이다. CGameInstance의 view/projection getter만 fixture로 바꿨고
  벡터·역행렬·좌표 변환은 실제 DirectXMath를 사용했다.
- identity, 회전/이동, 비균일 scale, 0.001 scale, 반사 scale의 world→model→world를
  실제 Recovery Sample로 왕복했다. Eye·LookAt·Up의 성분 오차 한계는 0.003이며 모두 통과했다.
- Horizontal 17도에서 환산한 수직 9.61068도의 실제 캡처가 성공했다. 선택 키의 기존
  Horizontal 30도·시간·ID·curve/easing/cut과 다른 키 값이 유지됨을 확인했다.
- singular·NaN·Inf·표현 불가능한 inverse, 무효 Eye/LookAt/Up, 누락 ID·Up count 불일치,
  view 없음은 거부했고 해당 포즈/키를 보존했다.
- 독립 읽기 검토에서 변환이 기존 runtime 정방향 Sample과 짝임을 확인했다. World Movie
  소비자는 기존 WORLD 행만 허용하며 callback 추가 없이 기존 호출을 유지한다.
- 세 C++ 파일은 기존 UTF-8 BOM 없음·CRLF를 유지한다. 기존 project/filter 등록을
  확인했으며 새 C++ 파일과 프로젝트 등록 변경은 없다. scoped `git diff --check` 통과.

증거는 `out/EffectCameraPose20261004/`의 두 TU `.rsp/.log/.obj`, `probe-compile.log`,
`probe-run.log`, `build_probe.py`, `cases.cpp.txt`, `provenance.json`, `source-check.json`이다.
probe는 실제 버튼 조작·런타임 owner 전환·파일 저장을 실행하지 않는다.

## G02. 데이터와 사용자 확인 경계

사용자가 편집 중인 Effect JSON, 카메라 JSON과 그 밖의 Data는 수정하지 않았다.
Client/UI 실행·종료·저장·publish·reload·화면 캡처를 하지 않았다. 최종 EXE Product 빌드는
상위 작업에서 실행 중 Client의 교체 시점에 수행하며 현재 결과는 소스/개별 컴파일/CPU 검증이다.

반영 후 사용자는 ALT+V 카메라 행에서 Sequence Camera Tool을 열고 키를 선택한 다음,
Seek/Pause로 해당 시각에 멈추고 F6 자유 카메라로 구도를 맞춰 `Use free cam pos`를 누른다.
키의 Eye·LookAt·Up만 변경되며 기존 시간과 FOV가 유지되는지 확인한다. F6 follow로
돌아와 Play하면 편집한 카메라를 재생한다. 필요할 때 명시적으로 Save camera source와
Publish saved cameras를 사용한다. 최종 구도·화면 판정은 사용자 확인으로 남는다.
