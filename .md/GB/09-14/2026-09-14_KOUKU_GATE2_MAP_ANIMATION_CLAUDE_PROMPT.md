# Claude 명령 — 2관문 전체 재생·맵 애니메이션만

```text
C:/Users/USER/source/졸업팀폴/LostArk에서 저장소 규칙과 아래 계획서 전체를 읽고 2관문 맵 애니메이션 원본 복구/수정을 진행해줘.
C:/Users/USER/source/졸업팀폴/LostArk/.md/GB/09-14/2026-09-14_KOUKU_GATE2_MAP_ANIMATION_PLAN.md

이번 범위는 이 새 계획서가 우선이야. 예전 G15 전체나 3관문·빙고·1관문 작업을 함께 실행하지 마.

목표는 F1 → Action Workbench → Composition Actions → Sequence → Gate2 → 2관문_진입컷씬의 전체 Play가 진행되고, 저장된 원래 세트 위치에서 책·테이블·받침·카드 등 맵 소품이 원본처럼 움직이며 Save/Reload 후 유지되는 거야. Object Preview at Character에서 캐릭터 앞에 보이는 것만으로 완료하지 마.

현재 Claude가 수정한 Level_KakulSaydonArena_WorldObjects.cpp/MainApp.cpp/Level_KakulSaydonArena.h의 미커밋 preview 기준점 변경을 보존하고 먼저 검증해줘. position0+emissions없음을 모든 절대좌표의 증명으로 취급하지 말고, 첫 키 숨김/원거리 이동/여러 target/일반 모션을 확인해. 이 편의 offset을 저장 World나 전체 Sequence에 적용하지 마.

첫 우선순위는 전체 Play가0ms에 멈추는 원인과 수정이야. Workbench 요청→MainApp 소비→Presentation/World 준비→admission→clock/owner의 최초 실패를 찾아줘. 이 문제는 별개라고 남겨두고 Object preview만 고쳐서 끝내지 마. 실패한 필수 트랙을 삭제하거나 오류를 무시해서 성공시키지 마.

현재 P3는 WORLD31개고 그중14개 묶음에 부모Matinee를 복구한130개 소품이 이미 들어 있어. 예전17개 상태로 되돌리거나165개 정지부모 후보를 중복 설치하지 마. 원본group/parent/키와 현재 설치본을 비교하고 틀린부분만 고쳐줘. 손뼈부착35개는 미설치 제약으로 따로 확인해줘.

Table의 gate2_intro_27s 존재와 펼침이 보이는 것만으로 원본 일치를 선언하지 마. 원본 reverse11초/forward15.66초, Book/HandBook, parent/root/scale/visibility/hold를 검사하고 필요한 소품만 재생성해. 원본 clip을 중복 Append하지 마.

현재 카메라는 실제7개 분할 샷에 Composition5박스가 연결돼 있으니 M06대로 기존 원본 샷의 참조/길이만 정상화해줘. 새 카메라 연출이나 임의FOV 보정은 하지 마. 이 불일치를0ms 정지의 원인이라고 증거 없이 단정하지 마.

전등·암전·안개·재질·셰이더·FX와 세이튼/쿠크 배우 골격 재제작은 제외야. 소품 문제를 고치려면 배우 수정이 꼭 필요한 경우 정확한 시각/근거/범위를 보고하고 승인받아줘. 1관문벽 카메라 작업과 같은Camera/Composition 파일에 동시에 저장하지 마. 먼저 끝난 작업의 최신본을 기준으로 이어서 설치해줘.

사용자 draft와 다른세션 변경 보존→후보→검증→baseline확인→선택적용 순서를 지켜줘. Object Save/Save Camera/Sequence Save/모델재쿠킹을 구분하고 Table에 Save Animated Props를 안내하지 마. 기존writer가 새필드를 누락하지 않는지와 Save/Reload를 확인해줘. runtime은 해당scope publisher로만 만들고 전체생성기install/전체파일복원은 하지 마.

C++를 수정했으니 cl /Zs만으로 끝내지 말고 필요한 Product 증분 빌드·링크까지 준비해줘. Client가 실행중이면 Save/종료를 요청해. 데이터만 바뀌면 불필요한 전체빌드하지 마. Clean/Rebuild/추적파일삭제/강제셰이더제외는 금지야. Client/UI는 내가 실행하니 자동 실행·조작·화면캡처는 하지 마.

추가 필수 목표: M09대로 내가 2관문 소품을 직접 수정할 수 있게 해줘. 전체 Sequence를 보면서 소품/묶음/개별 target을 골라 위치·회전·크기·동작 키와 시각·표시 여부·지원되는 clip 재생 설정을 수정해야 해. Action Workbench의 Edit This Motion/Object에서 불가능하면 Map Tool → World Sequence에 해당 stable instance/template/target을 연결하고 필요한 편집 기능을 구현해줘. 버튼이나 설명만 만들어 놓거나 단독 Preview만 되게 하고 끝내지 마. Table은 OBJECT_RESOURCE라서 Deploy로 복제하거나 Save Animated Props로 저장하라고 하면 안 돼.

전체 연출 Pause/Seek → 소품 선택·수정 → Apply/같은 시각 재평가 → Save → Reload → 전체 Play를 지원해줘. 여러 target 중 하나 수정 시 나머지 보존, 절대 좌표 이중 보정 방지, Object/Sequence 정본 공유, 외부 Save/draft 충돌 보호를 검증해줘. 데이터 수정마다 재빌드를 요구하지 마. 구워진 뼈 동작 수정은 별도 재쿠킹임을 구분하고 사용자 보정을 생성기가 덮어쓰지 않게 해줘. 소품 하나의 위치와 키 시각을 수정·저장·재실행하여 유지되는지 확인할 절차와 정확한 편집/저장/게시 버튼을 인계해줘.

계획 설명만으로 끝내지 말고 진단과 승인범위 수정을 진행해줘. 마지막에 실제원인, 바뀐대상, 빌드/데이터검사, 내가 전체Play로 확인할 경로·시각과 직접 수정/Save 방법, 남은35개/배우/재질제약을 정리해줘. 0ms가 남거나 편집·저장 연결이 미구현이거나 사용자재생이 미확인이면 전체복원완료/완벽이라고 하지 마.
```
