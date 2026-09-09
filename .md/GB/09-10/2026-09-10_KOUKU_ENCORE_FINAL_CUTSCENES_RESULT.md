# 앵콜 / 진짜 마무리 컷신 조사·계획서 작성 결과

작성일: 2026-09-10.
대응 PLAN: .md/GB/09-10/2026-09-10_KOUKU_ENCORE_FINAL_CUTSCENES_PLAN.md.
요청: 두 첨부 영상과 원본 데이터를 조사하여 상세한 구현 계획·설명서를 바탕 화면 TXT로 전달. 제품 기능 구현 요청이 아니다.

## G00. 실제 작성 범위

G00~G26의 계획·구현 지침과 원본 부록을 작성했다. 서두에 확인한 범위와 확인하지 않은 범위, 완전 검증된 제품 구현서나 전체 C++ 교체 패치가 아니라는 점을 명시했다.

부록은 선택한 두 시퀀스의 Group 46개, AnimationControl key 48개, 별도 재열람한 float weight key 52개, visibility/toggle/event, B/C 공간 차이를 담는다. Group 수는 실제 spawn actor 수와 다르다.

원본 후보:

- 앵콜: B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8L6.upk / SCENE07A / interpdata_23 / 23333ms / 23 groups.
- 마지막 기본안: B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8FD.upk / SCENE01B / interpdata_0 / 49083ms / 23 groups.
- 비교 변형: B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8FK.upk / SCENE01C / interpdata_0 / 49083ms / 23 groups.

SCENE01B 선택은 사용자가 준 빙고판 영상과 Z≈1145 공간의 대응에 근거한 구현 기본안이다. 원본 난이도/모든 Kismet 분기 선택까지 확정한 것은 아니다.

## G01. 조사한 것

- 사용자가 제공한 영상만 오프라인으로 읽었다. 앵콜 1.2초 간격 15프레임, 마지막 2초 간격 24프레임의 contact sheet로 장면을 분석했다. Client/UI 실행·자동 조작·게임 화면 캡처는 하지 않았다.
- 기존 추출 자료만 믿지 않고 위 세 UPK의 known track 배열을 현재 reader로 재열람했다. active Director는 앵콜 1개 컷, B/C 각 10개 컷이다.
- 앵콜 fakeui 0ms, FX_MI.fx_d_brokenglass_01_tr과 opacity/type 키, 15433ms 파편 trigger, 관련 light/particle/sound 이름을 확인했다.
- 마지막 source의 30933ms actor 교체, 41067ms HIDE, 40917ms 폭발 시작, fade/slomo 및 작은 쿠크 scale 0.3을 확인했다.
- 원본 LookInfo07이 실제로 mn_rpct_05_sk를 참조하는 점과 현재 모델 clip 목록을 대조했다.
- 현재 WP_MN_RPCT_05의 유일한 clip은 wp_mn_rpct_05_sk.ao_att_battle_17_01이며 원본 wp2의 idle_normal_1은 없다. 현재 missing 상태와 정적 등가 검증/재cook 절차를 분리했다.
- 기존 CModel/WorldSequence/Composition/UI/Camera/Server lifecycle 및 publisher 경계를 확인했다.

진단 산출물은 out/EncoreFinalPlan/의 SCENE07A.raw.json, SCENE01B.raw.json, SCENE01C.raw.json, animation_weights.json, source_details.py, contact_sheets.py와 영상 분석용 프레임이다. raw는 알려진 class의 재추출 결과이지 shader/AnimTree 전체 역해석 결과가 아니다.

## G02. 독립 검토와 실제 근거 확인 후 반영

계획서 규칙에 따라 기존 hook_plan_review 에이전트에게 읽기 전용 검토를 요청했다. 지적을 그대로 결론으로 복사하지 않고 현재 소스와 대조하여 다음을 반영했다.

1. KoukuSaydonEncounter.json은 정본이 아니라 project_encounter/projected_outputs 생성물이다. Composition authoring 및 projector/Server consumer 수정과 generated 파일 검증을 구분했다.
2. 0ms fakeui/BGM/visibility가 일반 crossing에서 빠지지 않게 최초 t=0 처리, 음수 프리롤 상태, Seek의 상태 재구성 정책을 명시했다.
3. 관찰용 F1 Replay와 제품 pending phase 복구 Retry를 별도 typed operation으로 구분했다.
4. 실제 Server의 REJECTED_BOSS_DEAD/ABORTED_BOSS_DEAD와 Client HP0 skip/session 정리를 확인했다. 현재 combat Complete Play로는 사망 보스 마지막 연출을 재생할 수 없으며 별도 cinematic lifetime 연결이 필요하다고 명시했다. HP1/guard 완화 우회는 금지했다.

추가로 복수 MoveTrack 적용 순서의 미확정 범위와 spotlight/named fog Group 구분을 설명했다.

## G03. 실행한 검사

- 진단 JSON 4개: strict UTF-8 읽기 및 JSON parse 성공.
- 조사 스크립트 3개: Python ast.parse 구문검사 성공. 제품 C++ 컴파일 검사가 아니다.
- PLAN: strict UTF-8, replacement character 없음, trailing whitespace 없음, G00~G26 27개 확인.
- 해당 PLAN/RESULT 경로로 한정한 git diff --check 실행 성공. 새 untracked 문서는 diff에 포함되지 않으므로 별도 문자열/UTF-8/공백 검사도 수행한다.
- workspace status는 기존 대규모 dirty 상태를 유지한다. 자동 stage/commit/push하지 않았다. Git ignore 설정 파일 접근 경고는 있었으나 이 문서 검사는 완료됐다.

제품 C++/JSON/XML/Resources를 이번 요청으로 변경하지 않았으므로 제품 컴파일, publisher 실행, runtime test는 수행하지 않았다. 사용자 화면 승인도 없다.

## G04. 전달 상태

대상: C:/Users/USER/OneDrive/바탕 화면/앵콜컷신 진짜 마무리 컷신.txt.
복사 전 대상 파일은 0바이트임을 확인했다. 최종 복사와 바이트 일치 확인 결과는 아래에 기록한다.

전달 완료: 지정 TXT에 저장했고 저장소 PLAN과 SHA256이 일치한다.

- 크기: 93,789바이트.
- 문서 검사 line split 기준: 1,195줄(마지막 개행 뒤 빈 행 포함).
- G 섹션: 27개.
- PLAN/TXT SHA256: A8A8EFD816560E8184FACDAD5DFA0C1840DEF604904547F01C376A5B53D0069B.

복사 과정에서 기존 다른 문서를 덮어쓰지 않았으며 제품 코드/리소스는 변경하지 않았다.

## G05. 남은 구현·검증 경계

- 원본 shader 수식/모든 texture dependency와 particle emitter 속성.
- AnimTree/mask/skel-control, 복수 MoveTrack arbitration을 포함한 최종 pose.
- wp2 원본 idle clip과 static 대체 가능 여부.
- 원본 camera aspect/default, slomo의 실제 timeline mapping, 녹화 시간과 source 시간의 정확한 대응.
- 미연결 actor/named variable 및 원본 event의 모든 Kismet 연결.
- 실제 C++/데이터 구현, 필요한 publisher·최소 컴파일·실행 입력/저장 확인.
- 사용자 직접 컷신 화면 확인과 4인 실행 검증.

문서 전달 완료와 제품 컷신 구현 완료를 혼동하지 않는다. 이번 작업으로 Drive에 전달할 새로운 제품 binary/resource는 만들지 않았다.
