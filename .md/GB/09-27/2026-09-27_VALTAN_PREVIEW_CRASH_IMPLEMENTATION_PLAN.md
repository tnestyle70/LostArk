# 발탄 Play Preview 비정상 종료 복구 구현 계획

기준: 2026-09-27, `GB/Valtan-Patttern-Complete`, HEAD `369987261`.

## G00. 목표와 확인된 원인 경계

발탄이 Server에 스폰되지 않은 발탄 아레나에서도 Action Workbench의
`Play Preview`가 기존 `CharacterPreviewPanel`의 전용 local preview를 stage하고,
실제 발탄 모델의 휠윈드를 재생하도록 현재 구현과 Debug 실행물을 일치시킨다.
Server boss 생성은 local authoring preview의 전제조건이 아니다.

Windows 오류 기록은 2026-09-27 22:09:19 `Client.exe`의 `0xc0000005` 접근 위반이다.
해당 실행물과 WER의 PE timestamp가 일치하고, RVA `0x1bf0354`는
`std::basic_string<char>::_Equal`의 왼쪽 문자열 객체 size 읽기다.
`return 0`으로 종료된 기록이 아니다. 당시 dump는 남아 있지 않아 과거 호출 스택을
현재 native probe의 스택으로 대체해 단정하지 않는다.

현재 Debug 산출물에서 `Animation_Tool_CompositionSounds.obj`와
`Animation_Tool_ValtanInspector.obj`만 9월 24일 헤더 배치로 남아 있다.
`Animation_Tool.h`는 9월 27일 변경됐으며 다른 관련 translation unit은 새 배치다.
실제 disassembly에서 Sound unit의 `m_pValtanBossTool` 읽기 위치는
`this + 0x1348`, 현재 소스 재컴파일 결과는 `this + 0x13A8`이다.
동일 객체의 필드 위치가 96바이트 어긋나므로 기존 null 검사도 유효한 필드를 검사하지 못한다.

## G01. 변경 단위와 복구 흐름

제품 preview CPP에 추측성 예외 회피나 별도 boss 생성 경로를 추가하지 않는다.
문제의 두 Debug OBJ 및 해당 tracking 정보를 `out/ValtanPreviewCrash20260927`에
보존한 뒤, 두 OBJ만 정상 제품 Debug 빌드로 다시 생성하고 Client를 링크한다.
빌드 후 재생성된 OBJ 시간과 헤더 dependency를 확인한다.
불완전한 dependency가 정상 빌드 뒤에도 남으면 해당 재발 조건을 별도로 조사한다.

`Play Preview -> Seek_EffectivePreview -> Play_EffectivePreview ->
Play_ValtanCompositionDraftPattern -> Stage_ValtanCompositionPreview ->
CharacterPreviewPanel::Select_TargetAsset -> CValtan local clone ->
Start_ValtanPatternMasterPreview`라는 기존 호출/소유권 계약을 유지한다.
게임플레이 Server boss와 편집용 clone은 기존대로 별도다.

바뀌는 공유 문서는 본 PLAN, 대응 RESULT, 반복 결함을 기록하는 gotchas다.
제품 C++/project/filter/JSON의 신규 파일이나 저장 계약 변경은 없다.
EXE/PDB/OBJ/LIB와 진단용 probe는 `out` 및 빌드 경로에만 두고 commit에 포함하지 않는다.

## G02. 검증과 종료 기준

화면 없이 WARP D3D device와 현재 제품 OBJ를 사용하는 작은 native probe로
실제 BossTool/BalanceTool/AnimationTool owner를 만들고, Server boss가 없는 상태에서
실제 휠윈드 문서를 `Play_ValtanCompositionDraftPattern`에 전달한다.
`Seek 0`, `AnimationTool::Update` 및 중지를 실행한다. Client나 ImGui 화면은 실행하지 않는다.

같은 probe에서 기존 OBJ와 문제의 두 unit만 현재 소스로 재컴파일한 OBJ를
A/B 비교한다. 기존 버전 접근 위반과 현재 소스 버전 정상 재생을 이미 확인했으며,
제품 Debug 복구 후에는 임시 대체 OBJ 없이 제품 OBJ만으로 같은 경로를 다시 검증한다.
Source 로직을 고치지 않은 A/B 결과, 원래 WER와 probe fault 위치의 차이,
제품 빌드 결과, 사용자 최종 화면 확인을 대응 RESULT에 구분한다.

종료 검증은 최소 제품 Debug 컴파일/링크, 관련 native 재생 경로,
변경 문서 `git diff --check`다. 사용자 화면과 arena 실제 UI 조작은 사용자가 확인한다.
