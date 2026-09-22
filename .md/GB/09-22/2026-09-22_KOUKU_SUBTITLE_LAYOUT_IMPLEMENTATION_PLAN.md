# Kouku Subtitle Box 화면 배치 연결 구현 계획

기준: `GB/collider-pattern-bug-fix`, `583e4b5bbd2d`, 2026-09-22.

## G01. 기존 occurrence와 화면 표시 계약

현재 Subtitle Box의 PositionOffset/Scale은 편집·직렬화·Preview 요청에 포함되지만
PresentationPlayer의 active row와 Collect_Subtitles에는 text/upper만 전달된다.
MainApp은 고정 글자 높이를 사용하므로 Box 숫자가 화면에 반영되지 않는다.

SUBTITLE의 기존 PositionOffset X/Y를 1080 높이 기준 화면 pixel offset으로 소비하고
양의 Y를 아래 방향으로 정한다. Scale X는 균일 글자 배율이며 Box Detail의 단일 Text scale
입력이 XYZ를 함께 설정한다. 기존 zero offset/unit scale은 기존 배치를 보존한다.
자막에는 사용하지 않는 3D rotation/Z scale 입력을 노출하지 않는다.

## G02. 기존 Preview owner와 제품 renderer 연결

KoukuSaydonActionWorkbench의 기존 geometry overlay/Save 흐름에 Subtitle을 허용한다.
offset/scale 변경은 같은 preview clock의 선택 occurrence만 업데이트한다. 활성 preview가 없으면
해당 자막 시작에서 일시정지 preview를 요청한다. Preview 버튼은 기존 edited occurrence snapshot을
소비하고 Apply/Save는 기존 stable ID 및 validation/atomic save를 유지한다.

KoukuSaydonPresentationPlayer는 occurrence offset/scale을 active row와 typed subtitle view에
전달하고 MainApp::RenderCinematicSubtitles는 그 값을 글자 높이·행 간격·화면 위치에 사용한다.
World/Valtan subtitle 및 말풍선의 기본 배치는 유지한다. 새 C++ 파일/새 JSON 필드가 없으므로
프로젝트 등록이나 schema version 변경은 필요 없다.

## G03. 검증과 데이터 후보

최신 소스의 해당 블록만 인코딩을 보존해 수정하고 out/KoukuSubtitleLayout20260922에
수정 전 bytes/hash를 저장한다. 실제 consumer를 사용한 headless 숫자 검사, codec/projector의
offset/scale 보존 및 invalid rollback 검사, 변경 TU 집중 컴파일과 diff check를 실행한다.
Client/UI는 실행하지 않는다.

Sequence 정본의 KAKULSAYDON_G1_PATTERN_5(2관문_클리어) 자막 6행만 Text scale 2,
화면 Y offset +90 reference pixel 후보로 준비한다. 두 줄 자막도 확대 전보다 아래로 이동하고
1080 화면 하단 안에 남는 값을 실제 font consumer로 확인한다. Boss 문서의 같은 pattern ID는 다른 패턴이므로
대상으로 삼지 않는다. 최종 파일 반영/publish/통합 Build는 통합 담당이 수행한다.
