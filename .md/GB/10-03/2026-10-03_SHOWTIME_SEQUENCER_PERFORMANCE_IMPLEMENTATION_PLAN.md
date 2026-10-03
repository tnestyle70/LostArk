# 쇼타임 시퀀서 병목 개선 구현 계획

## G00. 목표와 현재 실측

사용자가 저장한 `디버그_쇼타임_시퀀서_20261003_140615_881_frame179_53880_0.json`의
120프레임(60~179)을 분석한다. 실제 패턴 재생은 정상이고 시퀀서 미리보기에서 약1FPS가
발생한다는 사용자 관찰을 기준으로, 미리보기 계산과 UI 나열 비용을 각각 줄인다.

frame60~63의 CPU는 약1.4~1.54초이며 `Kouku.Presentation.BundleSample`은 각각 약1.1초다.
frame60~62는 CPU scope가 누락되어 Self를 확정하지 않는다. 누락 없는 frame63에서는
BundleSample1132.8912ms 중 직접 자식 구간을 제외한1129.5366ms가 해당 함수 내부에 남는다.
평상시 Composition Timeline은 약20ms(Layout약12ms, Draw약7ms)다.
두 수치를 같은 원인으로 합치거나 GPU timestamp를 GPU 사용률로 해석하지 않는다.

## G01. Action Workbench UI

`Client/Private/KoukuSaydonActionWorkbench.cpp`와 대응 header가 저작 문서의 timeline UI를 소유한다.
현재 Presentation resource lookup과 child row는 이미 generation cache를 사용한다.
개선 대상은 변경 없는 프레임에도 반복하는 lane interval/문자열 구성과 display row 배치,
그리고 화면 밖 box의 UI 제출이다.

레이아웃에 실제 영향을 주는 draft generation, pattern identity, 시간축 폭과 외부 World inventory
입력을 확인해 cache를 무효화한다. 스크롤·선택·drag·marquee·context menu의 stable ID 계약을
보존하며 표시되지 않는 box의 작업만 줄인다. 새 C++ 파일을 추가하지 않으므로 project/filter
등록 변경은 없다.

## G02. Bundle 미리보기

`CKoukuSaydonPresentationPlayer::Sample_BundlePreview`의 호출자와 WORLD/effect/pose 소비자를
추적한다. 계측에서 비어 있는 약1.1초 구간은 실제 소스와 저장된 PATTERN_35의 입력으로
재현해 반복 작업을 특정한 후 수정한다. 연속 재생과 뒤로 seek의 기존 의미, 현재 actor/bone
anchor, 사용자 저작 데이터와 rendering options를 보존한다. 하위 단계 계측은 적은 수의 coarse
scope로 추가해 다음 사용자 캡처에서도 원인을 확인할 수 있게 한다.

## G03. 검증과 적용

실제 저장 입력을 사용하는 CPU 비교 검증으로 기존 결과·cold/warm cache·편집 후 무효화를
대조한다. 수정 TU를 원래 Debug 설정으로 컴파일하고 `git diff --check`를 실행한다.
실행 중 Client의 화면은 사용자가 확인한다. EXE가 점유되어 있으면 소스 검증과 후보 준비를
끝낸 뒤 Client 저장·종료만 안내하며 자동 종료/Reload를 하지 않는다.

수정 후 제품 링크와 새 프로파일러 캡처가 확인되기 전에는 촬영 FPS가 해결됐다고 기록하지 않는다.
실행한 검사와 남은 확인은 대응 RESULT에 구분한다. 사용자가 방금 저장한 Composition JSON은
수정하거나 publish하지 않는다.
