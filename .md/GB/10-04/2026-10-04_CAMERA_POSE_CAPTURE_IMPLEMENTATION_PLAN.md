# Effect Sequence Camera 자유 시점 포즈 캡처 구현 계획

## G00. 현재 코드와 이번 범위

기준은 `codex/effect-sequencer-dpi`, `e4940a6cc423c392eedf2d6bcc083892346386f9`다.
사용자는 ALT+V의 Sequence Camera Tool에서도 World Movie의 `Use free cam pos`로
현재 자유 시점의 구도를 선택한 키에 반영하기를 요청했다.

`CSequenceCameraEditor::Render`에는 optional `SEQUENCE_CAMERA_CAPTURE` callback과
동일 버튼이 이미 있다. `Set_KeyPose`는 Eye·LookAt·Up만 후보 행에 넣고 공통 validator를
통과한 뒤 해당 필드만 commit한다. Effect Recovery 호출자는 callback을 전달하지 않으며,
현재 공통 UI는 Model root 행의 버튼을 일괄 비활성화한다.

Guardian 49420 원본 sidecar는 Model root와 Horizontal FOV를 사용한다. 09-22 Guardian
ALT V PLAN/RESULT의 원본 좌표 변환·구간·FOV를 유지한다. 10-04 Movie 자유 카메라 FOV
PLAN/RESULT에서 반영한 `CCameraTool::Capture_ViewPose`의 runtime FOV 범위를 재사용한다.
키 시간·FOV·축·보간·Cut before·ID와 원본 source metadata는 이번 버튼으로 바꾸지 않는다.

## G01. 파일과 callback 계약

| 파일 | 변경 책임 |
|---|---|
| `Client/Private/EffectAuthoringSequencer_Camera.cpp` | 현재 카메라의 자유 시점 여부를 확인하고, 캡처한 월드 포즈를 선택한 원본 행의 좌표계로 변환하여 공통 편집기에 전달한다. |
| `Client/Private/SequenceCameraEditor.cpp` | 좌표계 변환을 맡은 callback이 있을 때 Model root 행도 같은 버튼을 사용한다. |
| `Client/Public/SequenceCameraEditor.h` | callback이 행의 좌표계로 포즈를 반환하고 실패 시 기존 출력과 키를 보존한다는 계약을 명시한다. |

새 타입·저장 필드·런타임 owner는 만들지 않는다. 세 파일은 기존 Client project/filter에
등록돼 있으므로 프로젝트 파일 변경이 없다. 기존 World 호출자는 WORLD 행만 사용한다.

## G02. 캡처와 변환의 함수 흐름

`Render_RecoveryCameraTool`의 기존 Render 호출에 callback을 전달한다. callback은
현재 `m_Camera`를 `CCamera_Free`로 확인하고 Follow requested 또는 presentation override가
있으면 F6 자유 시점 안내를 반환한다. 소유권을 뺏거나 F6를 자동 변경하지 않는다.

`CCameraTool::Capture_ViewPose`는 local candidate에 Eye·LookAt·Up을 읽는다. 좁은
수직 FOV도 기존 runtime 범위에서 캡처하지만 `Set_KeyPose`는 그 FOV를 사용하지 않는다.
Model root 행은 현재 preview model/root가 있는지 확인한 뒤 기존 `Resolve_Root`와
같은 기준 행렬을 읽는다. World 행은 월드 포즈를 그대로 사용한다.

파일 내부 변환 helper는 root의 모든 성분과 determinant·inverse의 유한성 및 역행렬
존재를 검사한다. Eye와 LookAt은 inverse root의 좌표 변환, Up은 방향 변환을 적용한다.
변환한 시선·Up basis가 유효한 후보만 반환한다. 실패는 callback status로 전달하고
입력 포즈를 보존한다. 새 callback에는 FOV 축 환산을 넣지 않는다.

공통 `Set_KeyPose -> CEffectRecoveryCamera::Validate`를 통과한 변경만 기존 Apply/preview
refresh 경로에 들어간다. Save camera source와 Publish saved cameras는 기존 별도 명령으로
남기며 버튼 클릭 자체로 파일 저장·publish·자동 reload를 수행하지 않는다.
기존 `Capture_CameraKey`의 새 키/FOV 캡처와 camera owner/priority 로직은 변경하지 않는다.

## G03. 검증과 반영 경계

현재 실제 함수 본문과 DirectXMath를 사용하는 작은 CPU probe에서 identity·회전·이동·비균일
scale root의 world→model→world Eye/LookAt/Up 왕복을 확인한다. singular·NaN·Inf root와
잘못된 시선·Up을 거부하고 포즈/키를 보존하는지 확인한다. 실제 `Capture_ViewPose`의
9.610678도 수직 FOV 입력이 통과하고 선택 키의 Horizontal FOV·시간·ID·보간·cut이
유지되는지 공통 Set_KeyPose와 validator로 확인한다.

두 변경 CPP를 실제 Debug MSVC 옵션으로 독립 컴파일하고 UTF-8/CRLF 및 scoped
`git diff --check`를 확인한다. 사용자 Effect draft와 Camera JSON 등 Data는 수정하지 않는다.
EXE Product build는 상위 작업에서 실행 중 Client 교체 시점에 수행한다. Client/UI 실행과
화면 판정은 하지 않으며 RESULT에는 CPU 검증·컴파일과 미실행 화면 확인을 구분한다.
