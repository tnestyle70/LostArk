# 쿠크 Sequence 재생과 Object 편집 구현 계획

## G00. 현재 연결과 변경 범위

`MainApp.cpp`는 Resource Preview를 시작한 뒤 이전 Animation Preview를 정리한다.
현재 정리 결과를 공용 상태 문자열에 기록해 `No KoukuSaydon composition preview is playing`이
정상 Resource 시작 결과를 가린다. Presentation Player는 occurrence별 실패 여부만 저장하고
이유는 공용 문자열에 두므로, 뒤의 정상 Effect admission 메시지가 앞의 실패를 덮어쓴다.

기존 사용자 변경과 미저장 draft freshness 경계는 유지한다. Client/UI를 실행하거나 캡처하지
않는다. MainApp Preview 정리, Presentation Player의 capture 경계와 실패 전달,
WorldObjectTool의 timeline 선택·복제 및 Kouku Workbench animation 행을 수정한다.
Server Mario logic와 Composition schema 변경은 병렬 Server 작업에서 소유한다.

## G01. 재생 실패 보존과 실제 Effect 준비

작은 오망성의 실제 정본 `effect.kouku.gate3.mario.boss.pentagram.full.restore`를 현재
codec·Product admission·설치 Resources로 검증한다. 실패의 구체 원인을 교정하고
준비 큐의 원인 문자열을 occurrence에서 보존한다. capture는 아직 준비 중인 대상과 실제
실패를 구분하고, 해당 capture와 현재 활성 occurrence의 실패만 처리한다.

## G02. Object timeline

`CWorldObjectTool`은 기존 `WORLD_SEQUENCE_TEMPLATE`의 transform, animationTracks,
effectTracks를 소비한다. Stage/Transform/Animation/Effect 행 이름을 표시하고 animation
clip은 slot별 한 행에 배치한다. 개별 box 선택을 Detail에 연결하고 duplicate 명령은
현재 문서 복사 → 타이밍·ID 검증 → template 교체로 처리한다. Animation duplicate는
원래 window 뒤에 같은 clip을 반복하며 필요한 motion 종료 key만 연장한다.

## G03. Pattern과 팝업북

Workbench의 Pattern animation clip별 행을 stage/profile의 한 행으로 통합한다. 팝업북의
기존 펼친 기본 자세, WORLD Sequence, 조명, 실제 Saydon target/clip 소비를 대조하여
소비가 끊긴 부분을 수정한다. 저장된 사용자 pattern 배치와 연출 타이밍은 유지한다.

## G04. 검증

변경 TU의 Debug 컴파일, 실제 Effect codec/Product staging, Object 문서 duplicate의
저장·재로드와 실패 보존, 변경 JSON/XML parse, `git diff --check`를 확인한다.
새 C++ 파일은 추가하지 않으므로 project/filter 등록 변경은 없다. Product 전체 빌드는
root 작업에서 조율하며 사용자에게 F1 재생·선택·복제·저장 확인 경로를 전달한다.
