# Claude 전달 명령 — 기존 시퀀스 원본 대조 후 필요한 부분만 수정

아래가 최신 지시다. 이전 원본검토/디버그 패턴4개 추가 명령 대신 보낸다.

```text
작업 저장소는 C:/Users/USER/source/졸업팀폴/LostArk야.
저장소 규칙과 아래 수정된 설명서를 끝까지 읽고 진행해줘.
C:/Users/USER/source/졸업팀폴/LostArk/.md/GB/09-15/2026-09-15_KOUKU_FOUR_CUTSCENES_CAMERA_MAP_PLAN.md

디버그/원본검토 패턴4개 추가 계획은 취소야. 기존 시퀀스를 직접 원본과 대조해줘.
이미 있는 패턴이나 다른 사람의 미커밋 변경을 삭제/되돌리라는 뜻은 아니야.
기존 Pattern ID·이름·연결 구조를 유지하고, 수정 전 데이터는 파일/값 백업으로 보관해줘.

대상은 바탕 화면 영상 1관문 시작 .mp4, 2관문 컷신 .mp4,
3관문 컷신 .mp4, 3관문 끝.mp4에 해당하는 현재 시퀀스야.
카메라와 맵 애니메이션만 조사·비교·필요 수정해줘.
책·테이블·벽·의자·촛대·무대 조각은 맵 소품에 포함해.
조명·암전·재질·안개·셰이더·FX·사운드·자막·배우 동작은 건드리지 마.
현재 커진 보스/플레이어 모델 크기도 그대로 두고 카메라 보정의 입력으로 봐줘.

먼저 현재 Pattern occurrence가 실제 참조하는 shot/instance/template/target/clip을 찾아줘.
원본 package→Matinee/활성 Director→actor/parent→track/clip과 매칭해 비교표부터 작성해줘.
JSON 숫자만 비교하지 말고 좌표계·단위·parent·relative-to-initial·FOV/aspect·
source/패턴/shot 시간·재생속도·baked clip을 고려한 실제 평가 결과를 비교해줘.
GateIntroPlan/EncoreFinalPlan의 raw.json은 이미 m/Y-up으로 변환됐고 fovY도 aspect1.5로 계산된 요약이야.
이걸 미변환 원본으로 취급해 다시 축/단위를 바꾸지 마. 현재 생성기는16/9를 쓰므로 fovY 직접비교도 금지야.
UPK 속성/원본 수평FOV와 검증된 viewport 조건으로 대조해줘. 추출 실패를 빈 트랙/맵 동작 없음으로 처리하지 마.

판정별로 이렇게 처리해줘.
1. 원본과 같음: 유지. 리소스/패턴 추가, 재쿠킹, 불필요한 Save/revision 증가 금지.
2. 누락/잘못된 연결/변환/동작 오류: 근거를 기록하고 기존 CAMERA/WORLD 대상만 최소 복구.
3. 기존 수동 보정: 원본과 다르다고 삭제하지 말고 이유를 확인하고 유효하면 유지.
4. 데이터는 같은데 모델 크기로 구도가 어긋남: 원본 재설치 없이 카메라 구도 보정으로 분리.
5. 원인/의미 미확정: 덮어쓰지 말고 근거와 필요한 확인을 보고.

구도 보정은 원본 오류 복구와 별도 diff/표로 남겨줘.
현재 유효 scale/pose/배치/pivot을 확인하고 카메라 거리·높이·바라보는 지점부터 조정해줘.
필요한 FOV/roll/곡선·전환만 보정하고 월드 원점 기준 단순 배율 확대는 하지 마.
보스 크기를 줄이거나 조명/암전을 변경해서 맞추지 마.
원본과 같은 책/무대 동작은 모델이 커졌다는 이유만으로 바꾸지 마.
내가 원본과 현재 화면을 비교할 시각·랜드마크·편집 경로를 알려줘.

현재 P8은 1관문_연출이고 팝업북에서 호출 중이야.
구형 build_gate_cutscenes_g12.py의 P8=빙고 엔딩을 그대로 설치하지 마.
종료 패턴이 최신 저장본에도 없다면 현재 연결 없음으로 보고하고 새 패턴 추가는 확인받아줘.
현재 P7의 도착 구간만 원본과 일치할 수 있으니 전체 구간 누락과 키 오류를 구분해줘.
앞부분 확장 때문에 배우/암전/전투 진입 시각을 바꿔야 하면 자동 retime하지 말고 보고해줘.
2관문 Table의 기존 baked clip과130개/14묶음이 맞으면 그대로 둬.
카메라5컷과 기술적7샷을 구분하고 정상 연결이면 다시 추가하지 마.
이번 재감사에서 P3 revision59의 연결 오류는 확인됐어: 박스4는4460ms인데 샷4는2224ms,
박스5는cam6 표시명인데 실제 샷5는1104ms의cam4 중간 조각이야. 샷6/7은P3에 연결이 없어.
최신 파일이 같은 상태면 기존7샷을 재사용하고 필요한 Composition camera 참조/박스만 복구해줘.
경계는0/2100/13950/19490/21714/22818/23950/27000ms야. 이미 고쳐졌다면 no-op이야.
이 사실은 연결 오류의 증거지 샷 곡선/전체 화면의 원본 일치 완료 증거는 아니야.
원본에 맵 애니메이션이 없으면 없다고 적고 배우/FX를 맵 동작으로 만들지 마.

카메라/맵 리소스가 다른 패턴이나 게임 AUTO와 공유되면 소비자부터 알려줘.
이번 범위 밖 소비자에 자동 전파하지 말고, 최소 참조 분리 또는 공유 적용을 확인받아줘.
기존 activation/sequenceInstanceId/enterCombatOnFinish/트리거를 일괄 변경하지 마.
공유 mapplacements 영구 수정이 필요해도 영향 검증과 별도 확인이 먼저야.

기존 Action Workbench Sequence에서 편집하고 저장하는 방식을 유지해줘.
CAMERA→Open Composition Camera→Save Camera,
OBJECT_RESOURCE는 WORLD→Edit This Motion/Object,
MAP/DEPLOY placement 동작은 지원되는 Map Tool World Sequence를 사용하고,
박스는 기존 Save Sequence를 사용해줘. 별도 저장 사본/재생기를 만들지 마.
전체 Pause/Seek→수정→Apply/같은 시각 평가→Save→Reload→전체 Play를 확인해줘.
Table은 OBJECT_RESOURCE이므로 Deploy로 복제하거나 Save Animated Props로 안내하지 마.
지금 Map Tool preview는 숫자targetId만 해석하므로 문자열OBJECT_RESOURCE Table은 ID만 넘겨도 안 돼.
또 전체 Sequence는 Object draft가 아니라 Get_SavedDocument를 사용해. 전체 미저장 편집 즉시반영을 완료로 쓰지 마.
필요하면 기존Object/World평가경로를 재사용한 draft/동일시각평가·복구 연결을 최소구현하고 검증해줘.
Map Tool fallback도 target해석부터 애니메이션평가/Stop·Restore까지 연결해야 하며 버튼추가만으로 끝내지 마.

Save 전 부작용 검사부터 해줘. Map Tool Save는 다른탭의dirty Deploy/배치/Gameplay/Navigation/Destruction도 저장할 수 있어.
Object Save는 연결Composition emission동기화와 WorldSequences 자동Publish를 수행하고,
전투참조가 있으면 Publish_AllPatterns까지 이어질 수 있어. Sequence자체Save와 구분해줘.
범위밖 draft/변경/게시가 있으면 임의저장·Discard·연결삭제 없이 보고하고 분리/승인을 받아줘.
미저장 미리보기를 보려고 우선Save/Publish하지 말고 자동게시와 수동publisher를 동시에 실행하지 마.

수정 전 백업→비교표→오류 복구→모델 크기에 맞춘 구도 보정→검증→선택 저장 순서야.
한 컷신씩 진행하고 같은 정본을 여러 세션이 동시에 설치하지 마.
변경0도 정상 결과야. 자동 재실행으로 동일 리소스가 늘거나 내 보정이 덮이지 않게 해줘.
실제 reader/Save 왕복과 필요한 scope publisher로 검사하고 범위 밖 pending 조명/Effect/Material/배치가 함께 게시되지 않게 해줘.
데이터-only는 빌드하지 마. C++ 수정이 꼭 필요할 때만 근거·정확한 코드·focused 검증 후 Product 증분 Build해줘.
Clean/Rebuild/추적파일 삭제/강제 shader skip은 금지야.
Client/UI 실행·조작·캡처와 최종 화면 확인은 내가 할게.

마지막에는 동일 유지 / 오류 복구 / 현재 모델 대응 구도 보정 / 미확정·현재 연결 없음으로 나눠 보고해줘.
기존 Pattern/shot/target ID, 원본 출처, 보정 전후 값, 공유 영향,
내가 수정·저장·재생할 정확한 경로와 사용자 확인 대기를 적어줘.
원본 데이터 동일과 최종 화면 동일을 혼동하거나 완벽하다고 선언하지 마.
```
