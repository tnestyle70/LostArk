# Claude 전달 명령 — 2관문 진입 카메라·맵 애니메이션

아래 명령을 그대로 전달한다. 계획서에는 과거 다른 작업도 들어 있으므로 **G15만 실행 범위**라는 문장을 삭제하지 않는다.

```text
현재 실제 작업 저장소 C:/Users/USER/source/졸업팀폴/LostArk에서 2관문 진입컷씬의 카메라·맵 애니메이션 수정 작업을 진행해줘.

먼저 저장소 AGENTS.md/CLAUDE.md와 현재 작업 규칙, 아래 계획서의 G15 전체를 읽어줘.
C:/Users/USER/source/졸업팀폴/LostArk/.md/GB/09-12/2026-09-12_KOUKU_SOURCE_SEQUENCE_RESTORE_IMPLEMENTATION_PLAN.md

이번 실행 범위는 G15만이야. 과거 G12/G13의 3관문·빙고·팝업북·조명 작업을 함께 실행하지 마. 기존 G13-R3~R6의 실패/되돌림 기록도 확인해줘.
C:/Users/USER/source/졸업팀폴/LostArk/.md/GB/09-13/2026-09-13_KOUKU_G12_CUTSCENES_CAMERA_MAP_RESULT.md

목표는 F1 → Action Workbench → Composition Actions → Sequence → Gate 2 → 2관문_진입컷씬 하나를 선택하면 카메라, 책, 도박판, 의자·촛대·주변 무대가 함께 올바르게 재생되고, 기존 Object Detail/Camera/Sequence에서 각각 편집·저장한 뒤 재로드해도 유지되는 거야. 다른 Demo나 안내문만 있는 창으로 대체하지 마.

원본 영상: C:/Users/USER/OneDrive/바탕 화면/2관문 컷신 .mp4
현재 작업 영상: C:/Users/USER/OneDrive/바탕 화면/2관문 진입(프레임워크1).mp4
원본 타임라인: C:/Users/USER/OneDrive/바탕 화면/쿠크1관문_연출_원본_20260913/01_컷신별_타임라인/1관문클리어_221_SCENE04A_matinee2_전체.json

P3 stable ID KAKULSAYDON_G1_PATTERN_3와 27초는 유지해. 현재 table에 gate2_intro_27s가 있고 펼침이 보인다는 것만으로 원작 동작까지 정확하다거나 차이가 카메라/FX뿐이라고 결론 내리지 마. 영상의 사건과 viewport를 먼저 맞추고 원본 → 생성 후보 → 저장 파일 → 실제 재생의 최초 불일치를 찾아줘.

작업 순서는 기존 책/테이블의 reverse·forward·hold·골격/전체 TRS 검증, 주변 무대의 parent/local 변환 복구, 그 기준에 맞춘 카메라 경로/구도/전환 확인, 저장·재로드·반복 재생 검증이야. 이미 정확한 모델은 재쿠킹하지 말고 원본 clip을 중복 Append하지 마. 현재 5개 카메라와 과거 7개 분할의 차이도 실파일로 확인해줘.

165개 backdrop은 과거 수직 바닥 띠와 공중 카드 문제로 되돌렸어. 옛 후보를 그대로 재설치하거나 전부 90도 회전하지 마. 원본 부모/상대 변환·ignore-base·초기 pose와 geometry를 대표 소품부터 확인해서 실제 같은 규칙의 대상에만 적용해줘. 기존 의자·촛대와 중복 생성하지 마.

전등·암전·안개·환경광·재질·셰이더·연기/카드/빛 링 FX는 팀장 담당이므로 변경하지 마. 세이튼/쿠크 배우 골격 재제작이 필요하다는 근거가 나오면 해당 구간과 필요한 범위를 보고해서 승인받아줘. 카메라를 억지로 돌려 가리지 말고 독립적인 소품 작업은 계속해줘.

현재 다른 세션의 카드미로/맵/C++ 변경과 툴 draft를 보존해. 코드 정답이 확정되지 않은 부분은 임의 패치하지 말고 먼저 측정해. 확인된 수정은 계획서에 정확한 교체 블록을 보강한 뒤 후보 생성·검증·baseline 확인을 거쳐 적용해줘. 전체 생성기 --install, 파일 전체 복원, freshness 검사 제거는 금지야.

Object Save, Save Camera, Sequence Save, 모델 재쿠킹은 별개야. Table에 Save Animated Props를 안내하지 마. World만 바뀌면 해당 scope, Camera/배치도 바뀌면 Area publisher로 검증·게시·Check하고 runtime DataFiles를 직접 편집하지 마. 같은 Area의 다른 작업이 같이 게시되지 않는지도 조정해줘.

데이터만 바꾸면 C++/셰이더 전체 빌드를 하지 마. C++ 변경이 입증될 때만 정상 Product 증분 Build를 사용하고 Clean/Rebuild/추적파일 삭제/강제 셰이더 제외를 하지 마. Client/UI는 내가 실행할 테니 자동 실행·조작·화면 캡처하지 마.

계획 요약으로 끝내지 말고, 필요한 진단과 승인된 범위의 수정을 진행해줘. 마지막에는 실제 원인과 바꾼 것, 검사 결과, 내가 누를 정확한 경로/시각/Save 방법, 팀장에게 넘길 Data와 Resources 위치를 정리해줘. 사용자 재생 확인 전에는 완벽/복원 완료/visual PASS라고 하지 말고 적용 및 자동 검사 완료와 사용자 확인 대기를 분리해줘.
```
