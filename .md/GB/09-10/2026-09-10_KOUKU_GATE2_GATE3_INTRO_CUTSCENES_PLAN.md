# 쿠크세이튼 2관문 시작 / 3관문 시작 컷신 — 원본 조사와 구현 인계 설명서

작성일: 2026-09-10. 대상 저장소: C:/Users/USER/source/졸업팀폴/LostArk.
요청 결과물: C:/Users/USER/OneDrive/바탕 화면/2관문 시작 3관문 시작 컷신.txt.

이 문서는 사용자가 제공한 두 영상과 현재 PC의 원본 UPK, 추출 자료, 실제 런타임 모델, 현재 소스를 대조한 설계·설명서다. 제품 코드 적용 보고서가 아니다. 이 문서를 복사한다고 컷신이 자동으로 생기지 않는다. 아래의 원본 사실과 구현 요구사항을 구분해서 순서대로 적용한다. 기존 코드 전체를 덮어쓰는 패치가 아니라, 다른 구현자가 현재 변경을 보존하면서 작업하기 위한 상세 인계다.

## 읽기 전 안내 — 이 문서의 완전성과 검증 범위

이 문서는 “빠짐없이 검증된 완벽한 구현서”가 아니다. 자세하게 작성했지만, 분량과 완전성은 다르다.

확실히 확인한 것은 원본 컷신 ID, 카메라 전환 시각, 주요 모션, 모델 파일, 현재 프레임워크 연결 구조다.

아래 항목은 아직 확인 또는 검증이 남아 있다.

- 원본 애니메이션 A/B 슬롯의 혼합 가중치.
- 파티클 내부 수치와 일부 이벤트의 정확한 역할.
- 원본 카메라 화면비 설정.
- 실제 구현·빌드·재생을 통한 설계 검증.

또한 그대로 복사하면 구현이 끝나는 전체 C++ 코드까지 들어간 문서는 아니다. 다른 구현자가 따라갈 상세 지침이다.

따라서 이 문서의 정확한 평가는 “원본 근거를 갖춘 상세 계획서지만, 완전 검증된 최종 구현서는 아니다”이다. 앞선 안내에서는 상세하다는 점을 강조하면서 이 차이를 충분히 분명하게 전달하지 못했다. 구현자는 위의 미확인 항목을 확정된 원본값으로 취급하지 말고, 확인 결과와 실제 실행 검증을 후속 결과 문서에 기록해야 한다.

## G00. 먼저 무엇을 만드는지 정확히 고정한다

### 목표

2관문: 세이튼과 플레이어가 있는 장면 → 책을 이용한 연기 → 플레이어 소멸/암전 → 도박판 테이블이 펼쳐짐 → 쿠크와 카드 연출 → 2관문 실제 플레이로 인계.

3관문: 2관문 도박판의 쿠크와 거대 세이튼 → 거대 망치 연기 → 보라색 이동 공간 → 3관문 무대 등장과 보스 연기 → 플레이어 도착 → 실제 플레이로 인계.

카메라만 넣는 작업이 아니다. 다음 여덟 가지가 같은 시간축을 읽어야 한다.

1. 카메라 위치·방향·화각·컷 전환.
2. 보스와 플레이어의 외형, 애니메이션, 보이는 시점.
3. 책·테이블·의자·촛대 등 소품의 Transform과 뼈 애니메이션.
4. 파티클과 조명.
5. 암전과 필요한 화면 연출.
6. 음성·효과음·자막.
7. 서버의 전투 정지, 플레이어 배치, 관문 전환 확정.
8. 중단·재생 종료·접속 종료 때의 정리와 원래 상태 복구.

### 작업에서 하지 않을 것

- 영상 파일을 게임 화면에 그대로 재생하는 방식으로 대체하지 않는다.
- 보스를 Client에 전투 몬스터로 임의 소환하지 않는다.
- 영상의 브라우저 메뉴, 재생 바, 마우스 커서, 녹화 검은 여백, 편집자가 넣은 관문 이름을 만들지 않는다.
- 원본 게임의 컷신 스킵 투표 UI는 이번 시작 컷신 복원과 별도 기능이다. F1 중단과 제품 파티 스킵을 혼동하지 않는다.
- 기존 1관문 팝업북, 마리오, 카드미로, 갈고리, 빙고 데이터를 통째로 덮어쓰지 않는다.
- 컷신이 재생된다는 이유만으로 전투 이동·damage·관문 활성화를 Client에 넘기지 않는다.

### 조사 결과의 신뢰 수준

원본 확인: 실제 UPK의 Matinee ID, Director 순서, 주요 애니메이션 키, 역재생·속도·슬로모션 키, LookInfo 모델 식별자.
현재 파일 확인: WModel 존재와 내부 clip 이름, 현재 카메라/WorldSequence/Composition 인터페이스와 제약.
영상 관찰: 첨부 영상의 장면 구성과 대략적 시점. 1.5초 간격 프레임으로 관찰했으므로 타격 프레임의 정확한 시각은 원본 키와 최종 재생 비교로 판정한다.
프로젝트 설계: 아래의 ID, 상태 전이, 실패 처리, 오프라인 베이크 전략은 우리 프레임워크 적용안이다. 원본이 동일한 C++ 구조를 쓴다고 주장하지 않는다.
추출 범위 제한: 파티클 내부 emitter 전체 수치, 모든 Kismet 신호 목적, Actor 애니메이션 A/B 슬롯 가중치는 이번 조사에서 완전 해석됐다고 할 수 없다. 이름만 보고 수치를 지어내지 않는다.

## G01. 원본이 실제로 어디에 있는가

### 원본 패키지

공통 폴더:
C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages/

| 대상 | 물리 패키지 파일 | 논리 Scene | Matinee | InterpData | 원본 길이 |
|---|---|---|---|---|---|
| 2관문 시작 | B9AVB2VAZIQRPQCJVKAVYRAVOKYPY806.upk | SCENE04A | efseqact_matinee_2 | interpdata_2 | 27,000ms |
| 3관문 시작 | B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8M6.upk | SCENE02A | efseqact_matinee_10 | interpdata_10 | 약 35,368ms |

Scene 번호를 관문 번호로 해석하면 안 된다. SCENE02A가 이 영상의 3관문 시작이다. SCENE03A를 골라서 3관문 컷신이라고 연결하면 다른 연출이 나온다.

기존 추출 폴더:
C:/Users/USER/OneDrive/바탕 화면/쿠크_컷신_전체추출_20260904/

참조 파일:
- SCENE04A_806.cutscene.v2.json
- SCENE04A_806.director.json
- SCENE02A_8M6.cutscene.v2.json
- SCENE02A_8M6.director.json
- 추출도구/la_upk.py
- 추출도구/la_props.py
- 추출도구/la_scene.py

이 JSON은 모든 값이 완전히 풀린 최종 런타임 데이터가 아니다. decodedPartial인 배열과 첫 항목만 읽은 기록이 있다. actor/group 연결 조사에는 유용하지만, 첫 animation key만 읽어서 컷신을 만들면 도중 모션이 빠진다.

이번 재조사 파일:
C:/Users/USER/source/졸업팀폴/LostArk/out/GateIntroPlan/SCENE04A.raw.json
C:/Users/USER/source/졸업팀폴/LostArk/out/GateIntroPlan/SCENE02A.raw.json

재조사에 사용한 독립 추출 스크립트:
C:/Users/USER/source/졸업팀폴/LostArk/out/GateIntroPlan/extract_scene_matinee.py

이 스크립트는 09-05 계획서의 코드를 조사용으로 재현한 것이다. 이름과 첫 docstring의 “completely”를 제품 품질 보증으로 받아들이지 않는다. collect 함수도 실제로는 “이 reader가 아는 track”을 수집한다. vector tangent, 알려지지 않은 track class, 슬롯 가중치까지 다 보존하는 최종 exporter로 승격하려면 아래 검사를 추가해야 한다. SOURCE_ASPECT=1.5도 원본 확인값으로 채택하지 않는다.

### 구현자가 원본을 읽을 순서

1. 지정된 UPK 하나를 연다. 전체 1,000개 맵 재스캔부터 시작하지 않는다.
2. 지정된 Matinee의 InterpData object reference를 찾는다.
3. InterpData의 InterpGroups 배열을 선언된 개수만큼 전부 읽는다. 2관문 대상 98개, 3관문 대상 67개다.
4. group마다 GroupName, export index, outer 경로, track reference 배열을 기록한다.
5. track마다 class, export index, byte offset, disabled 값과 원본 key 개수를 기록한다.
6. Matinee VariableLinks의 LinkDesc와 group 이름을 조인하여 actor reference 또는 named variable을 찾는다.
7. actor는 mesh property만 보지 말고 LookInfoKey, component, base, hardAttach, material override, socket attachment도 본다.
8. 모든 카메라 actor를 컷 목록에 넣지 않는다. 활성 Director가 가리키는 GroupName만 실제 shot이다.
9. 같은 이름의 interptrackanimcontrol_1이 다른 outer 아래 여러 개 나올 수 있다. 문자열 이름만 dictionary key로 쓰지 않는다. package+export index가 조사 identity다.
10. runtime stable ID는 조사 export index와 별개로 발급한다. runtime에 포인터나 vector index를 저장하지 않는다.

### 무엇을 누락 검사해야 하는가

- 배열 헤더의 개수와 실제 파싱 개수가 같은가.
- bDisableTrack은 property 존재만이 아니라 올바른 BoolProperty 해석으로 판정했는가.
- 카메라/actor initial Transform이 빠지지 않았는가.
- MoveTrack의 absolute/relative frame과 base actor 연결을 읽었는가.
- 곡선 arrive/leave tangent와 interpolation mode를 잃지 않았는가.
- animation start/end offset, reverse, loop, rate, slot과 weight를 보존했는가.
- toggle ON/OFF/TRIGGER와 최초 상태를 구분했는가.
- light, material, visibility, skel-control track을 조용히 버리지 않았는가.
- 소리와 Event 출력 링크를 해석하지 않았으면 원본 의미가 확정됐다고 기록하지 않았는가.

값을 못 읽으면 해당 actor/track와 이유를 보고한다. 정상 actor 전체를 지우거나 원점으로 대체하지 않는다. 조사용 raw JSON과 제품 authoring JSON은 서로 다른 문서다.

## G02. 2관문 영상과 원본 시간을 함께 이해한다

첨부 영상: C:/Users/USER/OneDrive/바탕 화면/2관문 컷신 .mp4.
파일 길이는 약 27.333초다. 마지막 약 27초 프레임에는 다음 3관문 장면과 제목이 보인다. 따라서 이 파일 끝까지를 2관문 연출 길이로 강제하지 않는다. 제품 기준은 원본 27초다.

### 활성 카메라 컷 순서

| 원본 시간(ms) | 카메라 group | 의미 |
|---:|---|---|
| -1010 | cam1 | 프리롤에서 이미 선택된 첫 카메라. 우리 runtime t=0에서는 cam1의 t=0 상태를 평가한다. |
| 2100 | cam2 | 세이튼의 책 연기 쪽으로 컷 전환 |
| 13950 | cam3 | 암전 중 테이블 촬영 공간 카메라로 바뀜 |
| 19490 | cam4 | 테이블 위 쿠크 쪽 연출 |
| 23950 | cam6 | 후반 쿠크/카드 연출 |

모든 위 Director transitionMs는 0이다. 즉 원본의 카메라 간 전환은 컷이다. 500ms 부드러운 이동을 일괄 추가하면 다른 장면을 가로지르며 벽·빈 공간을 보게 된다. 같은 카메라 안의 MoveTrack 보간과 카메라끼리의 컷은 서로 다른 개념이다.

실제 shot JSON의 내부 구간들은 blendInMs=0, blendOutMs=0으로 작성한다. Director transition만 0으로 옮기고 기존 shot의 blendOutMs=900을 남기면 다음 CAMERA occurrence와 점유 구간이 겹친다. 현재 Composition publisher도 CAMERA의 종료 점유에 blendOutMs를 포함한다. 마지막 shot에서만 명시적인 제품 handoff 복귀 정책을 적용한다. 이 규칙은 아래 3관문 13개 shot에도 동일하다.

### 시간대별 해야 하는 일

0~2.1초: 플레이어와 세이튼의 시작 구도. 컷신용 참여자 슬롯을 고정하고 기존 전투 모션과 이동이 덮어쓰지 않게 한다. 첫 프레임은 이미 진행 중인 음수 시간 animation을 평가한 자세여야 한다.

2.1~11.7초: 얼굴·책 중심의 연기. 책을 단순 평면으로 대체하지 않는다. 세이튼 몸의 책 연기 clip과 별도 책 prop의 clip이 각자 존재한다. 손 소켓 또는 원본 attach parent를 읽어서 책이 공중에서 따로 노는 현상을 막는다. 실제 attach 여부는 actor binding별로 적용하며 모든 책에 임의 b_wp_1을 넣지 않는다.

11.66~16.29초: 소멸 이펙트와 검정 화면. 화면이 검어졌을 때 촬영 세트를 전환한다. 한 공간에서 테이블까지 카메라를 길게 보간하지 않는다. fade 아래에서 staging한 객체를 활성화해야 검정이 걷힐 때 pop-in이 없다.

15.66초부터: table의 evt2_table_open01 재생이 시작된다. fade가 완전히 걷히는 16.29초에는 테이블 애니메이션이 이미 약 0.63초 진행돼 있어야 한다. “밝아진 뒤 애니메이션 시작”으로 구현하면 시간차가 난다.

19.49~23.95초: 쿠크 중심 shot. 의자·촛대·카드 등의 소품과 효과가 같은 컷신 시간을 읽는다.

23.95~27초: 후반 연기와 최종 암전. 27초 원본 끝은 검정 1이다. 서버 목적지 commit과 제품 follow camera 준비가 끝난 뒤 별도의 프로젝트용 reveal을 수행해야 한다. 이 reveal은 원본 27초 안에 있던 데이터라고 표시하지 않는다.

### 원본 fade 키: 0은 투명, 1은 검정 불투명

0:1 / 970:0 / 11660:0 / 13480:1 / 14720:1 / 16290:0 / 25590:0 / 27000:1.

단순 ON/OFF가 아니라 곡선 값이다. 원본 mode는 cim_curveautoclamped이며 이 키의 tangent는 0이다. UI 이미지 alpha를 해당 곡선으로 샘플링한다. 화면 전체를 덮는 UI는 조명·노출을 낮추는 것과 다르다.

### 중요한 애니메이션 원본 키

세이튼 / slot a:
- -1290ms: evt2_book01, rate 1, 비루프.
- 5830ms: evt2_book02_loop, rate 1, 루프.
- 10450ms: att_battle_16_01, rate 1, 비루프.
- 15350ms: idle_normal_1, rate 1, 루프.

세이튼 / slot b:
- 4330ms: idle_normal_1.
- 4900ms: evt2_book03.
- 7450ms: evt2_book01, 역재생.

table:
- 11000ms: evt2_table_open01 역재생.
- 15660ms: evt2_table_open01 정재생.

책10:
- 4110ms: evt2_book02 정재생.
- 9040ms: evt2_book01 역재생.

주의: slot a와 slot b는 이름만 다른 순차 재생 목록이 아니다. 같은 시점에 둘 다 키를 가질 수 있다. 원본 AnimTree/slot weight를 확인해서 최종 포즈를 만들어야 한다. 마지막에 읽은 track으로 덮어쓰는 처리는 원본 복원이 아니다.

## G03. 3관문 영상과 원본 시간을 함께 이해한다

첨부 영상: C:/Users/USER/OneDrive/바탕 화면/3관문 컷신 .mp4.
파일 길이는 약 36.466초다. 원본 timeline 약 35.368초와 녹화 길이가 같지 않다. 영상 시작 trim과 원본 Matinee 슬로모션 적용 방식의 영향을 구분한다. 다른 컷신에 쓰던 1.03초 offset을 복사하지 않는다.

### 활성 카메라 컷 순서: 재추출 정수 ms 값

0 c1
2160 c1_a0
3376 c1_a1
4730 c1_a2
5686 c1_1
6178 c1_2
6599 c2
8257 c3
14375 c3_a1
16710 c4
20561 c4_a1
22070 c4_1
26420 c6

모두 transitionMs=0이다. 영상 설명에서 3.38초처럼 둥글게 말해도 authoring에는 위 값을 보존한다. 35.368초 동안 한 카메라를 쭉 움직이는 작업이 아니라, 13개 shot window를 정확히 잇는 작업이다.

### 영상에서 보이는 큰 흐름

0~2초: 작은 쿠크가 도박판 쪽에서 말한다.
2~6.6초: 거대 세이튼이 큰 뿅망치를 드는 연기, 휘두름, 충격 장면. 모델의 무기를 몸과 별도로 월드 원점에 놓지 않는다.
6.6~8.3초: 충격 이후 전환. 첫 암전이 이어진다.
8.3~16.7초: 보라색 워프 공간에서 날아가는 연기. 플레이어 simulation을 이 경로로 밀어 보내는 것이 아니라 컷신 표현을 이동시킨다.
16.7~20.6초: 3관문 무대에 보스가 등장한다. 암전이 걷히기 전에 무대·보스·필요 조명이 준비돼야 한다.
20.6~26.4초: 보스 근접 shot과 두 인물의 연기.
26.4초 이후: 후반 대사·강조·카메라 후퇴. 29초 부근 원본 slomo가 들어 있다.
후반 약 33초: 플레이어 도착/낙하 마무리 모션. 최종 플레이 위치와 제품 카메라로 인계한다.

### 원본 fade 키

7852:0 / 8257:1 / 8877:0.6 / 9849:0 / 14895:0 / 16139:1 / 17192:1 / 18441:0 / 32929:0.

8877ms 값이 0.6이다. 이 지점을 검정/비검정 bool로 줄이면 원본의 밝아지는 느낌이 달라진다. 해당 곡선의 arrive/leave 값도 raw 데이터에서 함께 사용한다.

### 원본 slomo 키

0ms=1.0
28938ms=1.0
29012ms=0.2
29299ms=0.2
29514ms=1.0

중요: 이 숫자를 Server fixed tick의 time scale에 넣지 않는다. 게임 전체를 느리게 하면 네트워크 tick, cooldown, 타격 판정이 함께 깨진다. 또한 Matinee 자체가 time dilation을 무시하는지/적용받는지는 별도 설정을 읽어야 한다. 적용 여부 확인 전에 모든 track에 slomo를 두 번 곱하지 않는다. 컷신 전용 clock mapping을 만들고 카메라·배우·효과가 동일한 결과 시간을 사용하게 한다. 원본 길이는 timeline 길이이고 실제 wall-clock 길이는 설정에 따라 달라질 수 있다.

### 주요 원본 animation: 거대 세이튼 MN_RPCT_06

slot a: 0 idle_battle_1 loop → 4144 att_battle_1_02 rate 0.7.
slot b: 1899 att_battle_5_01 loop → 5338 evt2_rpct_respawn_01 reverse → 8102 evt2_rpct_move_01 loop.

### 작은 쿠크 MN_RPCZ_00

slot a: 0 idle_battle_1 loop → 4989 run_battle_1 rate 1.5 loop.
slot b: 1997 att_battle_5_01 → 3776 att_battle_10_01 → 6589 evt2_rpcz_jump_01 → 8041 evt2_rpcz_move_01 loop.

### 도착 보스 원본 MN_RPCT_07

slot a: 0 idle_battle_1 loop → 28864 att_battle_13_01.
slot b: 0 idle_battle_1 loop → 17542 att_battle_15_03 → 19967 att_battle_13_03 → 25909 evt2_rpct_talk_04.

### 플레이어 예: pc_1

slot a: 0 idle_battle_1 loop → 12072 idle_normal_1 loop → 33104 act_fall_end_1.
slot b: 0 idle_battle_1 loop → 726 run_battle_1 loop → 2258 idle_battle_1 loop → 12072 idle_normal_1 loop.

pc_1 결과를 다른 클래스의 같은 clip 번호로 적용하지 않는다. 이름이 같아도 모델 내부 index는 다르다. 참여자 slot과 캐릭터 class를 연결하고 해당 class의 실제 clip name/asset 계약으로 resolve한다. 없는 clip을 다른 인덱스로 재생하지 말고 해당 표현 실패로 진단한다.

## G04. 모델과 애니메이션: 실제로 있는 것과 다시 준비해야 하는 것

### 보스 몸체

원본 LookInfoKey 재확인 결과:
- 2관문 세이튼: EFDLChar_MN_RPCT_05.MN_RPCT_05.
- 2관문 쿠크: EFDLChar_MN_RPCZ_00.MN_RPCZ_00.
- 3관문 망치 세이튼: EFDLChar_MN_RPCT_06.MN_RPCT_06.
- 3관문 작은 쿠크: EFDLChar_MN_RPCZ_00.MN_RPCZ_00.
- 3관문 도착 배우: EFDLChar_MN_RPCT_07.MN_RPCT_07.

mesh=null로 나온 actor가 “모델이 없다”는 뜻은 아니다. 위 actor는 LookInfoKey로 외형을 가져온다. 플레이어 actor는 동적 외형 바인딩도 고려해야 한다.

현재 물리 파일과 내부 clip 수:
- Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel: 249 clips.
- Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel: 34 clips.
- Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel: 91 clips.
- Character/KoukuSaton/MN_RPCT_00/MN_RPCT_00.wmodel: 249 clips도 존재하지만, 원본 LookInfo를 무시하고 이것으로 일괄 교체하지 않는다.

위 경로는 Client/Bin/Resources 상대 경로다. 파일 존재뿐 아니라 WModel 내부 animation 목록을 읽어 확인했다.

현재 C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp의 ACTION_PROFILES는 MN_RPCT_07을 MN_RPCT_05 물리 모델로 연결한다. 이는 현재 프로젝트의 명시적 alias다. 원본 07 파일이 직접 쓰이는 상태라고 설명하면 안 된다. 이 alias로 원본과 시각적으로 같은지는 사용자 화면 비교가 필요하다.

### clip name은 원본의 짧은 이름 그대로가 아니다

| 원본 | 현재 실제 이름 | 모델 | 길이(초) |
|---|---|---|---:|
| evt2_book01 | rpct00_evt2_book01 | MN_RPCT_05 | 8.0 |
| evt2_book02_loop | rpct00_evt2_book02_loop | MN_RPCT_05 | 2.0 |
| evt2_book03 | rpct00_evt2_book03 | MN_RPCT_05 | 6.3333 |
| att_battle_16_01 | rpct00_att_battle_16_01 | MN_RPCT_05 | 3.2667 |
| att_battle_15_03 | rpct00_att_battle_15_03 | MN_RPCT_05 | 3.7333 |
| att_battle_13_03 | rpct00_att_battle_13_03 | MN_RPCT_05 | 2.6667 |
| evt2_rpct_talk_04 | rpct00_evt2_rpct_talk_04 | MN_RPCT_05 | 4.0 |
| att_battle_13_01 | rpct00_att_battle_13_01 | MN_RPCT_05 | 4.6667 |
| att_battle_1_02 | mn_rpct_06_sk.ao_att_battle_1_02 | MN_RPCT_06 | 1.0 |
| att_battle_5_01 | mn_rpct_06_sk.ao_att_battle_5_01 | MN_RPCT_06 | 5.5 |
| evt2_rpct_respawn_01 | mn_rpct_06_sk.ao_evt2_rpct_respawn_01 | MN_RPCT_06 | 2.5333 |
| evt2_rpct_move_01 | mn_rpct_06_sk.ao_evt2_rpct_move_01 | MN_RPCT_06 | 1.2 |
| evt2_rpcz_jump_01 | rpcz00_evt2_rpcz_jump_01 | MN_RPCZ_00 | 0.7667 |
| evt2_rpcz_move_01 | rpcz00_evt2_rpcz_move_01 | MN_RPCZ_00 | 2.0 |

PlayAnimation("evt2_book01")가 실패한다고 clip이 없는 것으로 결론내리지 않는다. 반대로 비슷한 문자열이 있다는 이유로 rig까지 호환된다고 단정하지 않는다. profile별 명시적 매핑으로 해결한다.

### 무기 부착

원본의 거대 세이튼 attachment와 현재 KoukuSaydonPresentationAssetService.cpp 모두 b_wp_1 소켓을 사용한다. 몸체는 skeleton을 평가하고, 무기는 그 프레임의 socket world transform을 소비해야 한다. 무기를 한번 손 근처에 배치하고 끝내면 팔을 움직일 때 따라가지 않는다.

원칙: weapon local → socket combined → actor world 순서를 엔진의 실제 row/column-vector 관례에 맞춰 사용한다. 이미 구현된 CNpc/쿠크 presentation 조립 경로를 재사용한다. 다른 런타임용 독립 모델 로더를 만들지 않는다. cutscene proxy가 일반 world object 한 개라면 몸체·무기를 자동으로 조립한다고 기대하면 안 된다.

기존 무기를 그대로 쓰되 필요한 경우 컷신 proxy도 같은 조립 factory를 소비하게 한다. 또는 원본 소켓으로 움직이는 무기를 별도 시퀀스 대상으로 베이크할 수 있으나, 이 경우 모든 프레임에서 손과의 상대 오차를 수치 검사해야 한다. 월드 원점 보정값으로 손잡이를 맞추는 방식은 사용하지 않는다.

### 책 / 테이블 / 종이무대

원본 PSK/PSA 폴더:
C:/Users/USER/OneDrive/바탕 화면/쿠크_도박판컷신_에셋_20260904/1_컷신소품_테이블과책/

실제 파일:
- SkeletalMesh3/bg_rad_koukusaton_book.psk
- AnimSet/bg_rad_koukusaton_book_evt2_ani.psa
- SkeletalMesh3/bg_rad_koukusaton_table.psk
- AnimSet/bg_rad_koukusaton_table_evt2_ani.psa
- SkeletalMesh3/bg_rad_koukusaton_paperstage.psk
- AnimSet/bg_rad_koukusaton_paperstage_evt2_ani.psa

현재 책 cooked asset:
Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/DEPLOY_CINE_KOUKU_BOOK/DEPLOY_CINE_KOUKU_BOOK.wmodel.
현재 deploy catalog에 DEPLOY_CINE_KOUKU_BOOK과 DEPLOY_BG_RAD_KOUKUSATON_PAPERSTAGE가 있다.
현재 등록 책 clip은 bg_rad_koukusaton_book.ao_evt2_book02다. 이번 장면에서 요구하는 book01 역재생까지 포함됐는지 별도 clip 목록 검사 후 부족한 clip을 같은 PSK rig에 cook한다.

현재 조사한 deploy catalog에는 예전 도박판 컷신용 TABLE 항목이 없다. 원본 table PSK/PSA는 있으므로 다시 cook·catalog 등록해야 한다. 09-05 RESULT에 예전 테이블 작업 제거 이력이 있다. 예전 코드와 리소스가 지금도 남아 있다고 가정하지 않는다.

제안하는 새 cooked asset ID:
Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/DEPLOY_CINE_KOUKU_TABLE/DEPLOY_CINE_KOUKU_TABLE.wmodel.
이는 작성 제안이며 현재 존재 확인 경로가 아니다. 실제 생성 후 catalog와 물리 파일, material dependency를 같이 검증한다.

PSK/PSA를 Resources 밑에 원본 자료로 쌓지 않는다. 원본과 변환 중간물은 out 또는 외부 추출 폴더에 둔다. 최종 CModel/CMaterial이 읽는 cooked 결과와 필요한 texture만 Resources의 기존 7개 최상위 폴더에 넣는다.

## G05. 좌표를 잘못 옮기면 왜 컷신이 망가지는가

### 단위와 축

원본 UE3 좌표 cm, Z-up → 현재 m, Y-up:
X = sourceX × 0.01
Y = sourceZ × 0.01
Z = -sourceY × 0.01

위치와 접선의 축 변환을 일관되게 적용한다. 회전은 Euler 성분 세 개를 위치처럼 바꾸지 않는다. 원본 회전으로 basis를 만든 다음 좌표계 변환행렬로 변환하고 최종 quaternion을 얻는다. quaternion 정규화와 인접 q/-q 부호 연속성도 유지한다.

### 2관문은 촬영 공간이 둘 이상이다

기존 추출 actor 위치 예:
- cam2 약 (-2.9927, 2.5232, 739.8415).
- cam3 약 (-349.0059, -60.3536, 481.7285).
- cam6 약 (-302.6581, -98.6521, 453.5023).
- 테이블 기둥 dummy 약 (-297.7281, -102.1628, 447.0391).

앞 장면은 1관문 쪽 Z≈737, 뒤 장면은 Y≈-100의 별도 촬영 세트다. table 쪽만 현재 플레이 가능한 2관문 지점으로 옮기려면 그 세트의 모든 actor, camera, light, effect에 같은 anchor 변환을 적용해야 한다. 카메라만 보정하면 테이블은 화면 밖에 남는다.

### 3관문도 세 공간이다

- 도박판: 거대 세이튼 약 (8.7294, 10.6540, 318.3723), 원본 actor scale=(6,6,6).
- 이동 연출 공간: 일부 카메라/조명이 Z≈312, Y<0 쪽 별도 공간을 사용.
- 3관문 도착 무대: c4 약 (-0.6621, 1.1225, 942.7518), c6 약 (-2.8122, 1.4710, 944.7998).

여기서 원본 scale 6은 actor scale이다. 모델 import의 cm→m preScale과 같은 뜻이 아니다. preScale을 두 번 곱하거나 현재 보스 body scale을 다시 곱하지 않는다. 최종 bounding box 높이로 확인한다. 이전 마리오 변신용 1.5m 크기를 이 거대 세이튼에 적용하지 않는다.

### 권장 공간 전략

처음에는 원본 촬영 세트 좌표를 보존한 컷신 전용 presentation을 사용한다. 실제 플레이어의 서버 위치는 검증된 안전 위치에 고정하고, 화면에 보이는 연기용 proxy만 촬영 세트에서 움직인다. 촬영 세트는 맵 로드 scope에 포함시키되 nav bake나 gameplay collider를 만들 필요는 없다.

부득이하게 재배치할 때만 명시적인 set anchor를 사용한다. 제안 set ID: gate2_source_arena, gate2_table_set, gate3_table_set, gate3_warp_set, gate3_stage_set. 이 이름은 프로젝트 식별자이지 원본 node 이름이 아니다.

원본 parent chain을 완전히 world transform으로 풀고 그 결과에 set anchor를 한번 적용한다. child local offset에 anchor를 넣은 뒤 parent에도 또 anchor를 넣으면 이중 이동한다. cameraactor로 만든 dummy도 parent일 수 있으므로 실제 카메라가 아니라고 삭제하지 않는다.

### mesh vertex와 actor animation의 책임

모델 skeleton의 root가 이미 이동하고 있는 clip인지 읽는다. actor MoveTrack도 같은 이동을 갖고 있으면 둘 다 더해서 두 배로 움직이지 않게 한다. 원본 rootMotion 플래그와 clip root motion 실제 값, component initial offset을 같이 확인한다. cutscene에서는 기본적으로 원본 최종 world pose를 재현하는 것이 목표이며 navigation으로 매 프레임 바닥에 투영하지 않는다. 공중 워프 동작이 바닥으로 꺼지는 원인이 된다.

## G06. 시간축과 카메라를 변환하는 구체적인 방법

### 단일 시간축

제품 컷신은 서버가 발급한 runEpoch와 시작 tick을 식별자로 한다. Client에서 버튼 누른 순간의 로컬 시각을 정답으로 사용하지 않는다. 컷신 elapsed는 서버와 동기화된 시각에서 계산하고, actor/camera/effect 모두 그 elapsed를 샘플링한다. FPS가 낮으면 누락 프레임을 억지로 모두 그리는 대신 해당 시각 상태로 바로 간다.

애니메이션은 누적 deltaTime만으로 시작하면 다른 장치와 어긋난다. t=10초에 늦게 시작한 경우 0초부터 재생하지 말고 clip local time까지 복원한다. 단발 효과는 seek 때 과거 폭발을 전부 재발사하지 않는 정책이 필요하다. 지속 효과는 현재 구간에서 살아 있어야 하는 것만 생성한다.

### 음수 프리롤 처리

2관문 evt2_book01은 -1290ms에 시작한다. t=0에 시작 포즈를 보여주면 1.29초 늦어진다. 정재생이면 t=0에서 clip 1.29초 지점을 샘플링한다. 음수 key를 unsigned int에 직접 넣지 않는다. 원본 데이터는 signed/float 시간을 보존하고 제품 time=0 초기 상태로 정규화한다.

### animation local time 계산

유효 clip 구간 시작 = sourceStartOffset.
유효 끝 = clipDuration - sourceEndOffset.
진행량 = (sceneTime - keyStartTime) × playRate.
정재생 non-loop는 시작+진행량을 유효 구간에 clamp한다.
역재생 non-loop는 끝-진행량을 clamp한다.
loop는 유효 구간 길이에 대해 modulo하되 정확한 경계의 hold 정책을 정한다.
다음 key가 시작하면 해당 window ownership을 넘긴다. keyStart가 같은 두 슬롯은 순차 key 두 개로 합치지 않는다.

WorldSequence animationTrack은 현재 reverse와 sourceStartOffset 필드가 없다. Kouku Composition animation occurrence에는 iSourceStartMs와 iPlayMs가 있으므로 “엔진 전체가 offset을 지원 안 함”이라고 말하면 틀린다. 대상과 소비 경로에 따라 다르다. 이 작업에서는 고정 책/테이블/보스 표현의 역재생 및 A/B 합성을 오프라인에서 하나의 최종 clip으로 베이크하는 방식을 우선한다. 그러면 제품 WorldSequence는 정재생 한 clip을 정확한 시간으로 샘플링하면 된다.

다만 베이크 전에 A/B 가중치와 skeleton control을 복원해야 한다. 가중치 미해독 상태에서 둘 중 하나를 임의로 선택한 결과를 원본 clip이라 이름 붙이지 않는다. 동적 플레이어는 class별 연기 clip/바인딩이 필요하므로 고정 보스 모델 하나의 베이크로 대체하지 않는다.

### 보간

원본 cubic 곡선을 우리 LINEAR 또는 SMOOTH_STEP 하나로 단순 치환하지 않는다. 오프라인에서 원본 curve mode/tangent로 평가한 후, 오차가 허용 범위에 들어오도록 적응형 샘플링한다. 기본 제안 허용오차는 위치 0.02m, 회전 0.5도, FOV 0.1도다. 이 수치는 프로젝트 품질 기준이며 원본 엔진 상수가 아니다.

처음/끝/key discontinuity/카메라 컷은 반드시 샘플에 남긴다. linear runtime sampler를 선택했으면 검증할 때도 그 sampler로 재구성하여 원본과 비교한다. 원본 키를 일정 간격으로 줄이고 검증하지 않는 최적화는 금지한다.

### 카메라 FOV

원본 FOVAngle은 수평각 기준으로 조사한다. 제품 camera field는 fovYDegrees다.
fovY = 2 × atan(tan(fovX/2) / aspect).
삼각함수 내부는 radians, 데이터 표기는 degrees로 구분한다.
원본 CameraActor의 aspect constraint/class default와 화면 구도를 함께 확인한다. 녹화 파일 2160×1440의 비율 1.5는 게임 viewport 비율을 증명하지 않는다. 이전 추출기의 SOURCE_ASPECT=1.5 주석만을 근거로 전부 변환하지 않는다. 첨부 파일은 브라우저/검은 여백이 포함된 화면 기록이다.

### 카메라 roll과 key 제한

현재 CValtanCinematicCameraController pose는 Eye/LookAt/FovY이며 Up/roll을 담지 않는다. Camera_Free의 follow roll은 존재하지만 이것이 시네마틱 키별 roll 지원과 같은 것은 아니다. EffectAuthoringSequencer_Camera에는 upVectors와 검증/샘플링 코드가 있으므로 그 수학을 먼저 검토하고 공통 pose 계약으로 연결할 수 있는지 확인한다. authoring 전용 카메라를 제품 컷신의 두 번째 runtime으로 쓰지 않는다.

roll이 있는 원본 카메라를 Eye/LookAt만으로 베이크하면 진행 방향은 맞아도 기울어진 구도가 사라진다. roll이 없음을 검증한 shot만 현재 pose로 처리하고, 실제 roll이 필요한 shot에는 기존 camera ownership 경로의 up 지원을 연결한다.

CameraTool/ValtanCinematicCameraDocument의 key 제한은 64개다. 35초×60fps=2100개를 한 shot에 넣으면 실패한다. Director 기준으로 shot을 나누고 각 shot을 오차 기반 축약한다. 여전히 64개 초과면 동일 카메라의 연속 segment로 나누되 경계 blend=0, 동일 시각 pose를 공유한다. 제한을 무작정 100000으로 올리지 않는다.

### shot local time

예: 3관문 c4는 global 16710ms부터 시작한다. 제품 shot 파일을 local time으로 저장하면 local=global-16710이다. key 시간만 local로 바꾸고 occurrence start를 0으로 두면 틀린다. occurrence는 16710, 내부 첫 key는 0이다. shot 끝은 다음 cut 시각 직전의 원본 평가값으로 만든다. 마지막 c6은 원본 끝까지 유지한다.

## G07. 이펙트·조명·음성: 모델이 있는 부분과 없는 부분을 구분한다

테이블/책/보스는 실제 skeletal mesh와 animation 원본이 있다. 따라서 “이 컷신은 전부 이펙트라서 처음부터 눈대중으로 만들어야 한다”는 결론은 틀리다. 반면 파티클 이름과 spawn timing을 읽었다고 emitter 내부를 완전히 복원한 것은 아니다.

### 2관문에서 확인한 particle reference 18종

par_d_rpct03_sk14_02
par_d_rpct03_sk14_31
par_g_boxdust_001
par_g_ghostship_01
par_g_rpcz_00_trumpet_b_loc_int
par_g_rpcz_00_weapon_spawn_loc_int
par_l_rpcz_00-1_sk_02_1_loc_int
par_q_cardfly_01
par_q_chip_01
par_q_chip_02
par_q_chip_03
par_q_chip_04
par_q_field_01
par_q_rpct_exp_02
par_q_rpctgate_01
par_q_trail_01
par_u_rpcz_doll_appear_cast_01_loc_int
par_u_rpcz_spotlight_01_loc_int

### 3관문에서 확인한 particle reference 15종

par_d_rpct_sk12_13_loc_int
par_g_rpcz_00_bazooka_c_loc_int
par_j_lightshafting_01
par_m_glow_001
par_q_coloredpaper_01
par_q_darkfield_01
par_q_field_01
par_q_movingtrail_01
par_q_rpct_exp_01
par_q_rpct_exp_02
par_q_rpctgate_01
par_q_trail_01
par_q_warpspace_01
par_u_rpcz_beam_03_loc_int
par_u_rpcz_bigarea_down_01_loc_int

몇 종의 대표 원본 이름(par_q_warpspace_01, par_q_cardfly_01, par_q_rpctgate_01, par_u_rpcz_spotlight_01_loc_int)을 Data/Effects/V2에서 정확 문자열 검색했을 때 일치가 나오지 않았다. 이것은 같은 이름의 authored 연결을 못 찾았다는 뜻이다. 다른 ID로 복원돼 있는 효과나 대응 texture가 전혀 없다는 증거는 아니다.

### 연결 절차

1. 원본 particle actor에서 template reference를 읽는다.
2. actor binding과 toggle key로 켜질 시점/꺼질 시점/위치를 확정한다.
3. 해당 particle가 참조하는 mesh/material/texture를 찾을 수 있으면 별도로 목록화한다.
4. 현재 EffectV2 Independent/Groups/Authored에서 같은 리소스를 재사용하는 항목을 찾는다.
5. 일치하는 authored 효과가 있으면 그 stable resource ID를 사용한다.
6. 없다면 기존 EffectV2 authoring으로 효과를 구성한다. 원본 timing/anchor와 프로젝트 튜닝 emitter 수치를 분리해 기록한다.
7. Composition EFFECT occurrence 또는 WorldSequence effectTrack의 실제 소비 경로에 연결한다.
8. 종료 때 이 컷신 run이 만든 handle만 stop/release한다. 전역 effect clear로 전투 이펙트까지 지우지 않는다.

워프 공간은 중요도가 높다. 카메라와 보스가 준비돼도 배경을 채우는 보라색 공간 효과가 없으면 검은 바닥 아래 배우가 떠 있는 장면이 된다. 먼저 timing과 공간을 맞춘 뒤 swirl/particle 밀도/빛 번짐을 사용자와 튜닝한다. 효과 미완성 상태는 “워프 연출 효과 대체 상태”로 표시하고 원본 복원 완료라고 하지 않는다.

### 원본 event 숫자를 함부로 해석하지 않는다

2관문 이벤트 이름 1, 3, 4와 3관문 s0/s1, w_in/w_out은 신호 이름이다. 숫자 1을 사운드 1번, s1을 스폰 1번이라고 추측하지 않는다. Matinee 출력 링크와 연결 Kismet node, 대상 actor 및 sound reference를 따라간 뒤 typed cue로 변환한다. 확인 전에는 damage나 teleport를 이 event에 연결하지 않는다.

기존 추출 목록에는 bgm_midnightc_ed_m06_scene_movetobook, scene_midnightc_ed_moveintosatonbook 및 stop 계열 등의 AkEvent 이름이 있다. 이 이름만으로 재생 가능한 WAV가 생기는 것은 아니다. Sound 폴더에서 물리 파일과 현재 sound catalog ID까지 resolve해야 한다. 음성/음악이 준비 안 됐으면 자동 재생 없는 무음 시각 검증과 최종 음성 연결 검증을 분리한다.

### 조명

원본 spotlight/pointlight도 촬영 세트에 종속된다. 제품 maplights에 이미 켜진 동일 위치 조명이 있는지 확인해서 중복 조도를 막는다. 컷신용 조명은 occurrence 수명으로 켜고 끄거나, 기존 light property를 snapshot/restore한다. 영구 maplights intensity를 직접 바꾸고 종료 때 잊지 않는다.

## G08. 현재 프레임워크에서 어디에 연결하는가

아래 모든 경로의 기준은 C:/Users/USER/source/졸업팀폴/LostArk/다. 구현 전 해당 파일의 현재 diff를 확인한다. 이 저장소에는 이미 다른 기능의 미커밋 변경이 많다.

### 데이터 저작 소유자

Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json
  소품 Transform, object resource, 순차 animation, effect track. 최종 런타임 파일을 직접 수정하지 않는다.

Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json
  Director에서 분해한 shot의 Eye/LookAt/FOV와 timing. MapTool과 기존 Camera Tool이 읽는 정본.

Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.deployassets
  생성 가능한 책/테이블/애니메이션 prop 정의. placement와 별도다.

Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.deployplacements
  영구 배치가 필요한 prop의 인스턴스 ID/Transform. 일시 컷신 objectResource면 불필요한 영구 배치를 늘리지 않는다.

Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json
  경로에 Gate1이 있어도 현재 여러 관문 actor profile과 occurrence를 다루는 공통 저작 경로다. 이름만 보고 Gate2 폴더에 두 번째 런타임 document를 만들지 않는다.

Data/Effects/V2/Independent.json, Authored, Groups
  이펙트 정의와 연결. par_* 문자열을 그대로 resource ID로 쓰지 않는다.

Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json
  제품 진입 trigger/action의 서버 저작 정본. 새 의미의 action을 현재 parser가 알아서 지원한다고 가정하지 않는다.

Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json
  제품 actor action과 실제 clip presentation 연결. 컷신의 임의 world prop clip과 보스 combat action을 구분한다.

### C++ 소유자와 실제 현재 기준점

Client/Public/WorldSequenceDocument.h
  WORLD_SEQUENCE_OBJECT_RESOURCE, WORLD_SEQUENCE_ANIMATION_TRACK, WORLD_SEQUENCE_INSTANCE. 기존 target은 MAP_PLACEMENT/DEPLOY_PLACEMENT/OBJECT_RESOURCE다. PLAYER_ENTITY라는 필드를 JSON에 추가해도 현재 런타임은 처리하지 않는다.

Client/Private/WorldSequenceDocument.cpp
  parser/validation/save. schema 확장 필요시 읽기·검증·쓰기와 Python publisher를 함께 바꾼다. 알 수 없는 key를 버리고 성공했다고 하지 않는다.

Client/Private/WorldSequencePlayer.cpp, WorldSequencePlayer_Objects.cpp, WorldSequenceObject.cpp
  실제 객체 생성, 시각 샘플 적용, animation window, 종료 복구. CModel/CMaterial 경로를 유지한다. MAX_TRACK_COUNT=32, MAX_KEY_COUNT=256이므로 source 98개 group을 한 template에 무조건 밀어 넣지 않는다. 동일 run의 여러 sequence member로 나눈다.

Client/Public/KoukuSaydonCompositionDocument.h, Client/Private/KoukuSaydonCompositionDocument.cpp
  occurrence/source offset/actor profile/document 검증. actor profile 07→05 alias가 있는 위치다.

Client/Private/KoukuSaydonPresentationPlayer.cpp
  runEpoch와 product bundle session, CAMERA/EFFECT/SOUND/LIGHT occurrence의 실행·정리. 같은 컷신에 전용 로컬 타이머를 새로 만들어 이 경로와 경쟁시키지 않는다.

Client/Private/KoukuSaydonPresentationAssetService.cpp
  모델 prototype과 clip resolve, b_wp_1 무기 조립 계약. actor proxy도 이 공통 조립 원리를 재사용한다.

Client/Public/Level_KakulSaydonArena.h, Client/Private/Level_KakulSaydonArena.cpp
  Sample_CompositionCamera, Stop_CompositionCamera, Resolve_CompositionFollowPose, Update_TriggerMoveFade가 실제 기준점이다. camera override는 이미 owner를 가진다. 암전은 기존 TriggerMoveFade의 이동 추정 조건에 억지로 컷신을 태우지 말고, 명시적 cinematic 시간/owner 입력으로 분리 연결한다.

Client/Public/ValtanCinematicCameraController.h, Client/Private/ValtanCinematicCameraController.cpp
  기존 카메라 pose와 sampler. roll 확장시 Valtan 영향도 반드시 확인한다.

Client/Private/EffectAuthoringSequencer_Camera.cpp
  upVectors 수학과 검증 참고. 제품 런타임을 이 툴 객체에 의존시키지 않는다.

Client/Private/ClientReplication.cpp, Character.cpp, PlayerController.cpp 및 대응 Public 헤더
  실제 actor와 연기용 presentation ownership, 입력 잠금의 소비 지점. 입력 잠금만으로 Server movement를 막았다고 간주하지 않는다. Character가 network command를 판정하지 않도록 기존 경계를 지킨다.

Server/Public/GameRoom.h, Server/Private/GameRoom.cpp
  참여자, run lifecycle, 위치/관문 commit, 실패 복구의 권위. Place_PartyForCutscene는 현재 world.sequence.instance.original_kouku 하나에만 반응하는 팝업북 전용 함수다. 이름이 일반적으로 보여도 두 새 컷신을 자동으로 지원하지 않는다.

Server/Private/KoukuSaydonLogicRuntime.cpp 및 대응 헤더
  기존 run/패턴/전투 소유권과 컷신 종료 후 보스 activation 연결을 확인할 위치.

Shared/Public/Network/PacketMessages.h, PacketType.h, Shared/Private/Network/PacketMessages.cpp
  현재 메시지로 표현 불가능한 lifecycle 필드를 추가할 경우 writer/reader/검증을 한꺼번에 연결할 위치. runtime asset path는 보내지 않는다. 현재 protocol version을 읽고 기능 변경 단위로 갱신한다. 과거 문서의 69를 강제로 복원하지 않는다.

### publisher

Tools/MapPipeline/Publish-MapAuthoring.ps1
Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py
Tools/Build/Invoke-BuildDomainOwner.ps1

Data 저작 → 해당 publisher 검증 → Client/Bin/DataFiles 및 Server 생성 입력 순서다. camera authoring 저장만 하고 publish를 빼먹으면 이전에 겪은 “빌드해도 카메라가 안 바뀜” 문제가 다시 생긴다.

## G09. 구현 구조를 고정한다 — 컷신 연기와 실제 게임 상태를 분리

### 권장 구현 형태

카메라·소품·효과는 기존 Composition/WorldSequence 재생기를 사용한다. 그 위에 필요한 것은 별도의 모델 엔진이 아니라 “이번 run의 참여자, 잠금, 시작/종료/실패”를 관리하는 컷신 lifecycle 계약이다. 실제 클래스를 추가하기 전에 GameRoom과 기존 product run 상태에 결합 가능한지 조사하고 기능을 확장한다. 빈 Manager 클래스를 먼저 만들지 않는다.

### 제안 stable ID

### 반드시 분리할 실제 계약: 컷신 배우의 관문과 목적지 관문

현재 Composition의 BOSS_PLACEMENTS는 GATE1+MN_RPCT_05를 boss.kakulsaydon.g1.saydon에, GATE2+MN_RPCT_06을 boss.kakulsaydon.g2.big-saydon에 연결한다. GATE2+MN_RPCT_05 조합은 기본 전투 보스 계약에 없다. 따라서 원본 2관문 시작에서 05가 연기한다고 GATE2 combat pattern의 actorProfileId를 05로 넣으면 안 된다.

이 계획의 확정 적용안은 컷신 lifecycle을 GameRoom의 world presentation run 경계에 붙이는 것이다. 컷신을 가짜 combat pattern 하나로 만들거나 전투 gate/body 검증을 느슨하게 하지 않는다. 기존 WorldSequence와 CAMERA/EFFECT/LIGHT/SOUND 실행기를 재사용하되, 그 실행기에 시간을 공급하는 컷신 run 입력을 명시적으로 연결한다. 현재 Composition product run에서만 호출되는 부분은 공통 실행 함수로 분리하여 기존 combat 호출자와 새 cinematic 호출자가 같은 함수를 소비하게 한다. 별도의 모델/효과 재생 엔진을 만들지 않는다.

2관문 시작: source presentation 배우 05와 RPCZ → 컷신 proxy 연기 → destination GATE2의 boss.kakulsaydon.g2.big-saydon(MN_RPCT_06) 및 boss.kakulsaydon.g2.kouku(MN_RPCZ_00)를 서버가 검증·활성화한다.
3관문 시작: source presentation 배우 06/RPCZ, arrival 배우 원본07→현재 alias05 → 컷신 proxy 연기 → destination GATE3의 boss.kakulsaydon.g3.saydon(MN_RPCT_05)를 서버가 검증·활성화한다.

위 placement ID는 현재 코드의 기본 계약이며, enabled 상태와 실제 spawn group/encounter 연결은 진입 transaction에서 검증한다. 배우 proxy에는 이 실제 combat entity의 HP/AI를 붙이지 않는다. 컷신 제목의 관문 번호가 source 배우를 결정하는 규칙도 아니다. 목적지와 연기 모델을 별개 field/definition 참조로 소유해야 한다.

### 제안 식별자 목록

컷신 정의: kouku.cutscene.gate2.intro / kouku.cutscene.gate3.intro.
배우 slot: saydon, kouku, arrival_saydon, player.1, player.2, player.3, player.4.
책/테이블 slot: book.main, table.main.
shot ID prefix: kouku.gate2.intro.cam1, kouku.gate3.intro.c1.
F1 표시 이름: 2관문 시작 — 책에서 도박판으로 / 3관문 시작 — 거대 망치·워프·무대 등장.

이 ID는 제안이다. 현재 catalog 전체에서 중복 검사 후 등록한다. 원본 이름은 별도 source metadata로 보존해서 후속 튜닝 중에도 추적 가능하게 한다.

### lifecycle 상태와 의미

IDLE: 진행 중인 컷신 없음. 기존 전투/카메라 owner 유지.
PREPARING: 참여자 roster를 고정하고 Client들이 필요한 리소스를 stage한다. 아직 월드 위치/보스 상태를 파괴하지 않는다.
READY: 필요한 자산과 목적지 검증이 성공했다. 시작 tick을 서버가 선택한다.
PLAYING: 같은 runEpoch/startTick으로 컷신 시간을 계산한다. Server가 이동·스킬·AI 진행 정책을 적용한다.
HANDOFF: 원본 끝 또는 명시한 black window에서 서버가 목적지와 다음 encounter를 한 번만 commit한다. Client는 승인된 snapshot을 기다린다.
FINISHED: 승인된 실제 Character 표시, proxy 제거, follow camera/reveal 완료.
ABORTED: 실패 원인과 함께 이 run의 자원/잠금을 해제한다. 현재 유효한 게임 상태를 보존한다.

이 상태 이름도 설계안이다. 새 packet enum을 같은 이름으로 무조건 추가하라는 뜻은 아니다. 현재 product run 계약이 같은 의미를 이미 가진다면 그것을 재사용한다.

### 상태의 owner와 수명

runEpoch: 서버가 발급하며 한 방에서 진행한 한 번의 컷신을 식별한다. 중복 start/stop과 오래된 packet을 구분한다.
cutsceneId: Data definition의 stable ID. 물리 경로가 아니다.
participant list: PlayerId/NetEntityId와 연기 slot의 고정 매핑. 플레이어 접속 순서가 바뀌어도 컷신 중 slot을 재정렬하지 않는다.
startTick: Server 시간축. Client가 각자 갱신하는 frame counter가 아니다.
destination: 서버에서 이미 검증한 플레이 가능 위치/관문 식별자. 원본 underground 촬영좌표가 아니다.
presentation handles: Client run owner가 소유하는 camera/effect/light/proxy handle. 다른 run이나 전투 actor를 지우지 않는다.
saved presentation state: 보이는 상태, 입력/UI owner, 원래 카메라 전환 정보. 전투 HP 전체를 Client snapshot으로 되돌리는 용도가 아니다.
status/error: cutsceneId, runEpoch, actor/clip/asset/phase와 실패 사유. 성공값처럼 숨기지 않는다.

### actor proxy 방식의 정확한 의미

실제 플레이어 entity는 Server snapshot으로 계속 존재한다. 컷신 중에는 그 entity의 combat presentation을 owner 단위로 감추고, 동일한 class/외형을 참조하는 연기용 표시 객체가 컷신 경로와 clip을 소비한다. proxy에는 damage collider, AI, 네트워크 entity ID 발급이 없다. simulation 복제본이 아니라 표시 전용이다.

먼저 1인으로 확인할 때 빈 세 슬롯에는 기본 남성 캐릭터를 임의 생성하지 않는다. 등장하지 않게 하되 원본 shot 구도는 유지한다. 4인일 때만 네 실제 참여자의 외형을 배치한다. 성별·class에 따라 없는 전용 낙하 animation은 별도의 명시적 class mapping으로 해결한다.

proxy가 현재 구조에서 필요 이상으로 커지면 기존 Character presentation에 cutscene override owner를 붙이는 방식도 가능하나, replication이 매 tick Transform을 덮어쓰지 않도록 우선순위를 정의해야 한다. 어느 방식이든 같은 actor를 두 소유자가 동시에 움직이면 안 된다. 이 문서의 기본안은 실제 simulation 위치와 연기 위치를 분리하는 proxy 방식이다.

### 자동 진입 연결

F1 수동 재생을 먼저 닫는다. 다음에 실제 2관문/3관문 입장 흐름의 서버 승인 지점에 cutsceneId를 연결한다. 어떤 HP threshold에서 원본이 시작했는지 확인하지 않고 임의 체력 분기에 넣지 않는다. 사용자의 “관문 시작” 목표에 맞춰 entry 승인 흐름으로 연결한다.

현재 Debug_ActivateGate는 위에서 요구하는 transaction이 아니다. Level_KakulSaydonArena.cpp의 이 함수는 Request_DespawnAllWorldEntities, 개별 Request_SpawnWorldEntity, 로컬 플레이어 Request_DebugTeleportToPosition을 별도 요청으로 제출한다. 마지막 승인으로 관문/HUD 표시를 확정하지만, 이전 보스 despawn까지 rollback하는 단일 서버 입장 transaction은 아니다.

따라서 Debug_ActivateGate를 먼저 호출한 뒤 승인 callback에서 컷신 준비를 시작하지 않는다. 그 시점에는 이미 원래 보스/위치가 바뀌어 준비 실패 보존 조건을 지킬 수 없다. 새 컷신 시작 typed 요청이 Server GameRoom에서 prepare/ready/start/handoff를 소유하고, 목적지 활성화는 그 transaction의 commit 단계에서만 수행한다. 기존 Debug_ActivateGate는 컷신 없이 관문을 직접 테스트하는 별도 기존 기능으로 유지한다. 새 제품 입장 경로와 F1의 “전체 시작 컷신” 항목은 새 typed lifecycle 요청을 사용한다.

같은 LV_LUT_MIDNIGHTC_ED 안의 관문 이동이면 새 LEVEL enum을 만들지 않는다. approved placement/encounter 전환으로 처리한다. 다른 Level transition이 실제 필요할 때만 CLevelTransitionService 경로를 사용한다. Client에서 Change_Level을 직접 호출하지 않는다.

## G10. 서버 입장부터 종료까지 한 단계씩 구현하는 순서

### 시작 요청을 받았을 때

1. typed command sink에서 서버로 요청이 도착했는지 확인한다. F1 UI에서 socket을 직접 호출하지 않는다.
2. 현재 session/world/room, 요청 권한, cutscene ID를 검사한다.
3. 방에 다른 컷신이 진행 중이면 중복 실행하지 않고 기존 run 상태를 회신한다.
4. 살아 있는 참여자와 관문 진입 대상을 고정한다. 종료/이탈 정책도 같은 상태에 기록한다.
5. 목적지 spawn/nav/profile activation을 stage한다. 검증 실패면 현재 위치와 전투 상태를 그대로 둔다.
6. Client가 컷신 resource 준비 상태를 제출하는 계약을 사용한다. 제품 리소스 경로 자체를 서버에 보내지 않는다.
7. 준비 timeout은 유한 값으로 둔다. 프로젝트 초기 제안은 10초지만 원본 수치가 아니다. 하나가 실패했다고 모두 영구 정지시키지 않는다.
8. 준비 성공 후 startTick/runEpoch를 확정하여 방에 broadcast한다.

### 재생 중

1. Server는 참여자의 기존 이동 경로·pending skill·버퍼 입력을 정의된 정책으로 중지한다.
2. Client 입력 잠금과 별도로 Server command handler도 해당 run 참여자의 이동/공격을 거부한다.
3. 보스 AI와 해당 encounter damage 진행을 컷신 상태에 맞춰 정지한다. 임의 무적 HP를 덮어쓰는 대신 권위 상태를 사용한다.
4. Client는 실제 actor 표현을 숨기고 staged proxy를 활성화한다.
5. 컷신 time을 camera, props, actor clip, effect에 공급한다.
6. gameplay navgrid를 매 프레임 컷신 배우에게 적용하지 않는다.
7. Server fixed tick 자체를 느리게 하지 않는다.
8. 플레이어가 disconnect하면 사전 정책대로 해당 slot을 비우거나 run을 abort한다. 다른 slot을 순간 이동시켜 채우지 않는다.

### 정상 종료

1. Server가 종료 조건을 만족했는지 평가한다. Client의 “끝났다” 메세지만으로 전원 teleport하지 않는다.
2. 목적지와 보스 활성화가 아직 유효한지 검사한다.
3. 같은 run에서 destination commit을 한 번만 한다. 중복 종료 packet으로 두 번 이동하지 않는다.
4. Client가 snapshot을 반영한 뒤 실제 character를 표시한다.
5. 2관문은 원본 종료 black 유지 상태에서 목적지 follow pose를 준비하고 밝아진다.
6. 3관문은 원본 마지막 장면과 목적지 pose를 맞추고, 필요한 짧은 handoff blend만 적용한다. 마지막 원본 키를 임의로 조기 종료하지 않는다.
7. 이 run의 proxy/effect/light를 정리한다.
8. 해당 run owner의 camera override와 입력 잠금만 해제한다.
9. 다음 encounter를 시작한다. 종료 화면이 안정되기 전에 보스 첫 타격을 넣지 않는다.

### 실패/중단

리소스 준비 실패: start 이전이면 기존 화면/위치/보스를 유지, 실패 항목 표시.
재생 중 model/clip 적용 실패: 부분 객체를 정리하고 서버 abort 정책에 따라 기존 유효 위치로 인계. 기존 combat character를 다시 보여준다.
Level 이탈: camera/effect/light/proxy owner 해제. 다음 Level에 검정 overlay가 남지 않아야 한다.
재생 2회 연속: 이전 run의 saved state를 새 run에 재사용하지 않는다.
F6 free camera: 현재 Sample_CompositionCamera는 제품 follow가 아니면 적용하지 않는다. F1 관찰용 F6와 제품 컷신 lock 정책을 명시하고, 카메라만 멈춘 채 서버가 끝없이 잠기는 상태를 만들지 않는다.
통신 끊김: 현재 프로젝트의 disconnect→Lobby 계약을 따른다. 임의 local 전투로 fallback하지 않는다.

## G11. 화면 암전·HUD·자막을 만드는 세부 지침

현재 KakulFade UI는 trigger 이동/미로 전환을 위한 화면 fade다. cutscene는 원본 curve를 직접 공급해야 한다. Update_TriggerMoveFade의 위치 변화 감지에 의존해서 원본 black key를 흉내 내지 않는다.

CUILayoutRuntime/CUIObject 계열로 전체 화면 이미지와 alpha를 표현한다. UI image asset은 UI/<Domain>/ 상대 ID로 저장한다. 검정 화면을 ImGui full-screen window로 제품 런타임에 만들지 않는다.

overlay는 owner key를 가져야 한다. trigger fade와 cinematic fade가 동시에 요청되면 우선순위를 정하고 하나가 종료됐다고 다른 owner의 alpha를 0으로 만들지 않는다. resize/aspect 변경에도 viewport 전체를 덮어야 한다. authoring window 크기에 고정된 1280×720 사각형을 사용하지 않는다.

제품 HUD 숨김은 현재 UI visibility를 저장하고 컷신 owner가 요청한 동안만 적용한다. 원래 꺼져 있던 창을 종료 후 강제로 켜지 않는다. 마우스 gameplay 입력 소비도 HUD visibility와 별개의 command policy다.

자막은 원본 대사/음성 cue를 확인한 뒤 문자열과 타이밍을 데이터로 관리한다. 영상에서 읽히지 않은 문장을 기억으로 채우지 않는다. 자막 UI와 컷신 스킵 투표는 별도다. 영상의 [3/4] 표시를 게임 규칙으로 복사하지 않는다.

## G12. 실제 작업 순서 — 한꺼번에 구현하지 말고 이 순서대로 완료한다

### G12-1. 원본 timeline 확정

UPK 두 개에서 group/track/actor/Director/animation/toggle/fade/slomo를 재추출한다. 알려지지 않은 track 수와 해석 제한을 출력한다. 성공 증거는 “파일 생성”이 아니라 지정 Matinee와 key count/ID가 일치하는 것이다.

### G12-2. 배우와 소품 최소 준비

현재 세 모델의 clip mapping을 확정한다. table를 PSK+PSA에서 CModel/CMaterial cooked asset으로 만든다. book01이 필요하면 현재 book 모델에 포함시킨다. 무기 b_wp_1 attachment를 점검한다. 모델 import preScale과 actor scale을 분리한다. 이 단계에서 리소스 설치 폴더와 Drive 전달 목록이 나온다.

### G12-3. 카메라만 검증하는 F1 항목

정적 기준 배우/소품을 함께 보여주며 각 Director shot을 재생한다. 임의 전투 damage나 관문 이동은 하지 않는다. 2관문 5shot, 3관문 13shot의 앞/뒤 구도가 맞는지 사용자가 확인한다. camera만 움직이는데 모델이 없는 상태로 “구도 맞음”을 판단하지 않는다.

### G12-4. Transform과 애니메이션 연결

원본 actor world pose를 베이크하여 WorldSequence로 연결한다. source group 수가 track 제한을 넘으면 의미별 member로 나눈다. book/table 역재생과 보스 A/B blend를 완성한 baked clip을 붙인다. 동적 플레이어 class mapping을 연결한다.

### G12-5. 암전·visibility·조명

원본 fade curve와 toggle을 연결한다. actor가 켜지기 전에 카메라가 보는 검정 화면/세트 전환을 맞춘다. light 복구를 테스트한다. 모든 state를 처음부터 재생했을 때와 중간 seek했을 때 같은지 수치로 확인한다.

### G12-6. 이펙트·음성·자막

기존 EffectV2 리소스 재사용을 우선하고 없다면 timing은 원본, 표현 수치는 명시적 프로젝트 튜닝으로 저작한다. 사용자 비교 영상과 나란히 볼 때 누락한 layer를 목록으로 남긴다.

### G12-7. Server lifecycle 연결

resource ready, 참여자 잠금, run clock, destination commit, abort를 연결한다. Shared writer/reader와 Server contract test도 같은 변경 단위로 반영한다. 하나의 Client에서 보였다는 이유로 4인 동기화 완료로 기록하지 않는다.

### G12-8. 제품 관문 시작에 연결

F1 수동 실행 성공 후 실제 관문 승인 흐름에서 같은 cutscene definition을 실행한다. F1용 복제 시퀀스와 제품용 복제 시퀀스를 따로 유지하지 않는다. 같은 데이터가 preview/제품 run의 서로 다른 authority mode에서 소비되게 한다.

### G12-9. 종료/재시작 회귀 확인

2관문 → 중단 → 3관문 → 중단 → 다시 2관문, 정상 종료 후 마리오/미로/F6, 두 번 연속 재생, 리소스 일부 누락, 중간 disconnect를 확인한다. 다른 연출의 배치/카메라/조명이 원래대로 남는지 확인한다.

## G13. F1에 어떻게 보여줄 것인가

기존 Developer Tools 시퀀스 뷰어의 쿠크 탭에 아래 두 항목을 추가한다.

실제 목록/버튼 구현 위치는 Client/Private/MainApp_SequenceViewer.cpp의 RefreshSequenceViewer와 그 파일의 재생 처리다. 한글 이름 데이터의 현재 정본은 Data/Maps/SequenceViewer.labels.json이고, 제품 fallback은 Client/Bin/DataFiles/World/SequenceViewer.labels.json이다. C++ FriendlyName 함수에 두 이름을 하드코딩하는 대신 기존 label 문서 형식으로 stable ID를 연결한다. viewer 생성물을 직접 편집하지 않는다.

2관문 시작 — 책에서 도박판으로
3관문 시작 — 거대 망치·워프·무대 등장

표시할 보조 정보: 관문, source Scene/InterpData, 전체 timeline 길이, resource 준비 상태, 현재 shot 이름, elapsed, preview/product 상태, 실패 이유.

버튼 의미:
- Play: 선택 컷신 전체를 기존 typed 실행 경로로 요청.
- Stop: 현재 선택 run만 중단. 월드 전체 시퀀스 clear가 아니다.
- 구간 선택: 책 연기 / 테이블 열기 / 망치 / 워프 / 무대 등장 같은 한글 이름과 원본 시간 범위를 표시.
- Seek: 저작 preview에서 상태 샘플 확인. 제품 관문 진행/보상/teleport event를 다시 발생시키지 않는다.

Test맵/다른 Area에서 목록은 볼 수 있어도 필요한 쿠크 맵이 없는데 원점에 소환하지 않는다. 준비되지 않은 항목은 이유와 승인된 Area 이동 경로를 안내한다. 새 direct Level 변경 버튼이나 F2~F5 기능키를 추가하지 않는다. 실제 Client 조작과 최종 화면 판정은 사용자가 한다.

## G14. 데이터 저장·프로젝트 등록·리소스 전달

### 저장과 로드

원본 추출 결과를 곧바로 Client/Bin/DataFiles에 쓰지 않는다. authoring stage 문서를 만들고 해당 schema 검증 후 현재 문서에서 이 컷신 소유 ID만 upsert한다. 다른 template/placement를 지우지 않는다. 실패하면 기존 authoring 파일과 runtime 파일을 유지한다.

parse: 문법과 타입을 읽는다.
validate: ID 중복, 참조 대상, clip 존재, finite Transform, duration/key limit, asset path 안전성을 검사한다.
stage: 모든 필요한 prototype/clone/data를 임시 owner에 준비한다.
commit: 전부 준비됐을 때 한 번에 새 항목을 노출한다.
rollback: stage중 생긴 객체/handle만 해제하고 기존 Level과 Scene을 유지한다.

### 신규 C++ 파일을 만들 경우

현재 파일 확장이 기본이다. 새 클래스를 만들어야 한다면 실제 호출자와 소유 수명이 생겼을 때만 만든다. 새 파일을 추가한 변경에는 해당 프로젝트의 Default/*.vcxproj와 *.vcxproj.filters 등록을 함께 넣는다. 예를 들어 Client/Public의 새 헤더는 ClInclude, Client/Private의 새 구현은 ClCompile로 등록한다. 기존 필터를 재배치하지 않고 인접한 같은 책임 파일의 Filter를 사용한다. 경로가 등록되지 않으면 IDE에서 보이더라도 빌드되지 않을 수 있다.

Shared 변경은 Shared project, Server 변경은 Server project에 각각 등록한다. 새 Git Data JSON은 Client 프로젝트 96.DataFiles의 None 항목만 사용한다. Data를 별도 Content 복제본으로 만들지 않는다. 기존 C++의 인코딩을 유지하고 새 C++은 UTF-8 BOM 없음, 이 문서는 UTF-8로 유지한다.

### Drive로 전달할 항목

기존 MN_RPCT_05/06, MN_RPCZ_00 모델을 변경하지 않고 재사용한다면 그 파일을 새 리소스로 재배포할 필요는 없다. 단, 팀원 PC에 해당 clip을 가진 실제 버전이 있는지 확인한다.

새로 cook하는 TABLE 모델, 추가 clip으로 갱신한 BOOK 모델, 컷신 전용 baked clip/model 결과, 새 EffectV2가 필요로 하는 mesh/texture, 새 Sound/UI 물리 파일만 별도로 목록화한다. 각각 Resources 상대 ID와 실제 설치 폴더를 적는다. Git에는 JSON/소스/문서만 포함하고 Resources binary를 force-add하지 않는다.

actor palette 모델과 EffectV2 metadata만 보냈는데 실제 texture를 빼먹으면 흰색으로 나온다. material dependency를 함께 확인한다. 반대로 모든 추출 PSK/PSA/UPK를 팀 배포 ZIP에 넣지 않는다. 원본 추출 묶음은 저작 참고이지 제품 설치 자료가 아니다.

## G15. 검증 명령과 사람이 직접 볼 순서

아래는 구현 후 실행할 명령이다. 이 계획서 작성 세션에서 제품 publish/build를 실행했다는 뜻이 아니다.

작업 폴더:
C:/Users/USER/source/졸업팀폴/LostArk

맵 저작 검증:
```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Validate -Scope Area
```

검증 성공 후 맵 카메라/시퀀스 포함 publish:
```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish -Scope Area
```

Kouku 도메인 생성 계약 갱신:
```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon
```

최소 제품 Debug 빌드:
```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

마지막 whitespace 검사:
```powershell
git diff --check
```

새 Shared packet을 추가한 경우에는 NetworkProtocolHarness의 실제 build/run 명령을 현재 프로젝트에서 확인해 별도로 실행한다. JSON parse와 compile만으로 network payload 검증을 대신하지 않는다. 일반 컷신 카메라 튜닝마다 전체 FullDiagnostic을 강제하지 않는다.

### 사용자 실행 순서

1. 맵툴 변경을 저장하고 실행 중인 Client/Server가 새 파일 교체를 막지 않게 종료한다.
2. 현재 LAN endpoint 설정을 유지한다. 2026-09-30까지 팀 정본은 192.168.0.14:7777이며, 서버가 꺼져 있다고 127.0.0.1로 바꾸지 않는다.
3. 이 PC의 LAN sync 결과는 client다. 공유 서버 담당자가 Server를 켠 후 사용자가 Debug x64 Client를 Ctrl+F5로 시작한다.
4. Lobby에서 KoukuSaydon을 선택하고 Server 승인을 기다린다.
5. F1 → 시퀀스 뷰어 → 쿠크 → 2관문 시작 항목을 재생한다. 이 항목은 구현 후 추가될 목표 경로다.
6. 전체 27초의 5shot, 책, 테이블, 암전, 최종 인계를 확인한다.
7. Stop 후 3관문 시작을 재생한다. 13shot, 손 망치, 워프, 도착 보스, 29초 부근 강조, 플레이어 낙하를 확인한다.
8. 정상 종료 후 이동/공격을 눌러 입력 잠금이 해제됐는지 확인한다.
9. F6 전환과 다시 Play/Stop을 확인한다. 캐릭터가 복제되거나 배경 조명이 남지 않아야 한다.
10. 최종적으로 4인에서 모든 참가자의 같은 시작·종료·목적지와 본인 외형을 확인한다.

### 필수 수치/로그 검사

- 동일 run의 카메라와 배우 시간이 같은가.
- 2관문 cam2 2100ms, cam3 13950ms / 3관문 c4 16710ms에서 정확한 cut이 발생하는가.
- 시작 전에 대기하다가 새 run에서 elapsed가 0 기준으로 초기화되는가.
- negative pre-roll이 0으로 잘리지 않았는가.
- table 15660ms 시작과 16290ms fade 종료가 따로 기록되는가.
- wrong clip name이면 actor/profile/clip이 상태에 표시되는가.
- 동일 run start 두 번/stop 두 번을 받아도 객체와 teleport가 중복되지 않는가.
- 30fps/60fps/144fps에서 같은 시각의 pose를 얻는가.
- 종료 후 해당 run effect/light/proxy 개수가 0이며 기존 전투 객체는 살아 있는가.
- finite duration WorldSequence 종료 시 현재 Update가 Stop_Instance(id, targets, true)를 호출하는 경로를 재사용했는가. 이미 존재하는 baseline 복구를 또 다른 코드로 중복 구현하지 않았는가.

### 화면 확인은 사용자 판정

에이전트는 첨부 영상을 분석할 수 있지만 Client를 자율 실행하거나 화면을 캡처해서 시각 PASS를 선언하지 않는다. 이번 조사에서 이미 “원본과 동일하게 보인다”는 판정은 하지 않았다. 수치 검증, 빌드, 사용자의 실제 화면 확인을 RESULT에 따로 기록한다.

## G16. 자주 생길 오류와 원인을 바로 찾는 표

| 증상 | 우선 확인할 원인 | 조치 |
|---|---|---|
| 카메라만 이동하고 보스가 없음 | LookInfo actor를 mesh=null이라 삭제함 | LookInfoKey와 현재 profile alias resolve |
| 카메라가 수백 미터를 날아감 | Director 컷을 부드러운 단일 path로 변환함 | shot window 분리, transition=0 |
| 테이블 펼쳐짐이 늦음 | fade 끝난 뒤 clip을 시작함 | global 15660ms 기준으로 진행시간 복원 |
| 책이 손에서 떨어짐 | attach parent/socket 미반영 | 원본 binding과 evaluated bone 기준 조립 |
| 망치가 발밑에 있음 | body-only world object에 무기 월드 배치 | 기존 b_wp_1 조립 경로 재사용 |
| 배우가 땅으로 내려감 | 컷신 pose를 navigation-project함 | presentation 경로와 simulation 위치 분리 |
| 앞 장면은 맞고 뒤 장면은 없음 | 다른 촬영 set에 anchor를 누락 | actor/camera/light/effect에 동일 set transform |
| 모션이 갑자기 idle로 변함 | replication/AI와 cutscene가 동시에 소유 | run owner의 presentation override 정의 |
| 모션이 뒤집히거나 끊김 | reverse/start offset/A-B weight 누락 | 원본 최종 pose 베이크 후 재생 |
| 워프가 그냥 검은 화면 | 파티클 참조만 있고 authored 효과 없음 | EffectV2 리소스 연결/복원 상태 구분 |
| 원본보다 구도가 너무 넓음 | fovX를 fovY로 사용하거나 aspect 1.5 고정 | 실제 viewport 계약으로 FOV 변환 |
| 29초 후 동기화가 깨짐 | slomo를 전체 Server clock 또는 두 번 적용 | cinematic 시간 매핑 하나로 통일 |
| 빌드해도 옛 카메라 | authoring만 저장하고 publish 안 함 | 해당 Map publisher 실행 |
| 끝나도 검정 | 2관문 원본 끝 alpha=1 후 reveal 없음 | 승인된 handoff 뒤 프로젝트 reveal |
| 끝나도 안 움직임 | 입력/Server 잠금 owner 잔류 | FINISHED/ABORTED 공통 해제 검사 |
| 다른 효과가 사라짐 | 전역 effect clear 사용 | run이 소유한 handle만 해제 |
| F1에서 재생은 되나 실제 입장 안 됨 | preview만 연결, 서버 lifecycle 미연결 | entry 승인 → run → commit 계약 연결 |

## G17. 구현자에게 그대로 전달할 지시문

현재 저장소와 AGENTS.md를 먼저 읽으세요. 이 문서의 원본 사실을 기준으로 두 컷신을 구현하되 다른 기능의 미커밋 변경을 덮어쓰지 마세요.

2관문은 SCENE04A/interpdata_2, 3관문은 SCENE02A/interpdata_10입니다. SCENE 번호를 관문 번호로 바꾸지 마세요. 카메라 컷, 배우 clip, Transform, fade, toggle, effect를 하나의 시간축으로 연결하세요.

첫 구현은 원본 촬영 세트를 보존하는 시각 재생입니다. 실제 플레이어 simulation을 지하 촬영 좌표나 워프 경로로 순간이동시키지 마세요. 고정 배우는 기존 모델 조립과 CModel을 재사용하고, 필요 역재생/A-B 합성은 원본 해석 후 오프라인 clip으로 베이크하세요. clip 이름은 실제 WModel 목록으로 연결하세요.

원본 파티클 이름이 있다는 것과 EffectV2에 재생 가능한 효과가 있다는 것은 다릅니다. source particle → 현재 resource ID → 실제 물리 dependency → occurrence의 연결을 확인하세요. 확인되지 않은 emitter 수치를 원본값이라고 말하지 마세요.

F1에서 두 컷신을 개별 재생·중단하고 원본 시간 구간을 확인할 수 있게 하세요. 그 뒤 서버의 시작/잠금/종료/목적지 commit까지 연결하여 제품 진입을 완성하세요. Place_PartyForCutscene는 현재 팝업북 한 개 전용이므로 두 컷신이 이미 지원된다고 착각하지 마세요.

새 JSON field, Shared field, C++ 선언을 만들면 parser/writer/publisher/actual consumer/test까지 한 기능 단위로 닫으세요. 문서와 interface만 추가하고 완료했다고 하지 마세요. 새 C++ 파일은 project/filter에 등록하고, 적용 전에 해당 변경의 전체 최종 코드를 현재 파일과 대조 가능한 형태로 별도 G별 구현안에 보존하세요. 이 인계 설명서를 기존 수천 줄 파일을 대체하는 소스로 사용하지 마세요.

마지막에는 소스 반영, 데이터 publish, 최소 compile, 수치 검사, 사용자 화면 확인을 분리해 보고하세요. Client/UI는 사용자만 직접 실행·조작·최종 판정합니다. Drive로 전달할 새 Resources 파일과 상대 ID도 따로 알려주세요.

## G18. 이번 문서 작성으로 확인된 것 / 아직 실행하지 않은 것

확인: 두 영상의 장면 흐름, 원본 두 패키지의 대상 Matinee와 Director, 주요 clip/reverse/slomo/fade 키, 원본 actor LookInfo, 현재 모델 내부 clip, 원본 table/book PSK/PSA 존재, 현재 C++ 소비 경로와 지원 한계.

확인하지 않은 것을 완료라고 하지 않음: 모든 파티클 emitter 내부 완전 해석, Kismet 이벤트 전부의 의미, A/B 애니메이션 최종 weight, 원본 camera aspect class default, 현재 프레임워크에서 두 컷신 전체 재생, 4인 Server 동기화, 사용자 최종 visual fidelity.

이 문서 작성 작업은 제품 C++/JSON/Resources를 수정하거나 컷신을 빌드·실행하지 않는다. 조사용 out 파일과 계획/설명서만 작성했다.


## 부록 A. 원본 group/actor 인벤토리 — 연결 누락 방지용

아래는 기존 v2 추출본의 actor 메타데이터다. 배열 부분 해독 한계가 있으므로 camera/animation timing 정본으로 쓰지 않는다. null은 없음 확정이 아니라 미연결/동적 바인딩 가능성이다. 특히 세이튼·쿠크 LookInfo는 G04의 UPK 재확인 결과를 우선한다. 위치는 해당 추출본이 변환한 m/Y-up 기준이며 parent/relative 이동까지 평가한 최종 매 프레임 위치가 아니다.

### 2관문: 98개 group

| GroupName | Actor | Mesh / Particle | 초기 위치(m) | Base / HardAttach |
|---|---|---|---|---|
| table | skeletalmeshactormat_6 | null | 0.0000, 0.0000, 0.0000 | null / false |
| 책10 | skeletalmeshactormat_7 | bg_rad_koukusaton_book | -42.7534, 9.3168, 667.5698 | null / false |
| bdum | cameraactor_25 | null | -0.8134, 2.2304, 737.0328 | efskeletalmeshactorlookinfomat_2 / true |
| tab1 | null | null | null | null / false |
| tab1dum | cameraactor_34 | null | -0.8134, 2.2304, 737.0328 | efskeletalmeshactorlookinfomat_2 / true |
| tabldum | cameraactor_35 | null | -0.8435, 2.7256, 737.8380 | null / true |
| dd | cameraactor_39 | null | -0.9561, 2.7256, 737.7257 | null / true |
| ll | null | null | null | null / false |
| tabrdum | null | null | null | null / false |
| tabetcdum | cameraactor_37 | null | -0.8134, 2.2304, 737.0328 | efskeletalmeshactorlookinfomat_2 / true |
| book | skeletalmeshactormat_1 | null | 0.0000, 0.0000, 0.0000 | null / false |
| 데스크1 | cameraactor_0 | null | -297.7281, -101.4545, 447.0391 | null / false |
| 데스크2 | cameraactor_2 | null | -297.7281, -101.4545, 447.0391 | null / false |
| 데스크기둥 | cameraactor_1 | null | -297.7281, -102.1628, 447.0391 | null / true |
| None | null | null | null | null / false |
| 기둥1 | cameraactor_11 | null | -292.6448, -102.3880, 459.2780 | null / false |
| 기둥2 | cameraactor_12 | null | -300.3274, -102.3880, 460.0284 | null / false |
| 기둥3 | cameraactor_13 | null | -307.0918, -102.3880, 456.4082 | null / false |
| 기둥4 | cameraactor_14 | null | -310.7262, -102.3880, 449.6180 | null / false |
| 기둥5 | cameraactor_15 | null | -309.9719, -102.3880, 441.9585 | null / false |
| 책1 | null | null | null | null / false |
| 책11 | null | null | null | null / false |
| 책2 | null | null | null | null / false |
| 의자1 | efmotionstaticmeshactor_74 | bg_rad_koukusaton_chair01_sm_hht | -295.9331, -108.4963, 465.1856 | null / false |
| 의자2 | efmotionstaticmeshactor_75 | bg_rad_koukusaton_chair01_sm_hht | -306.3192, -108.4963, 463.1197 | null / false |
| 의자3 | efmotionstaticmeshactor_76 | bg_rad_koukusaton_chair01_sm_hht | -313.8072, -108.4963, 455.6317 | null / false |
| 의자4 | efmotionstaticmeshactor_77 | bg_rad_koukusaton_chair01_sm_hht | -315.8731, -108.4963, 445.2457 | null / false |
| 촛대1 | null | null | null | null / false |
| can1 | efmotionstaticmeshactor_0 | bg_rad_koukusaton_deco19_sm_hht | -311.0712, -108.4963, 434.5886 | null / false |
| candum1 | cameraactor_26 | null | -298.1073, -90.7867, 447.5375 | skeletalmeshactormat_1 / true |
| 촛대2 | null | null | null | null / false |
| can2 | efmotionstaticmeshactor_1 | bg_rad_koukusaton_deco19_sm_hht | -285.2759, -108.4963, 460.3837 | null / false |
| candum2 | cameraactor_27 | null | -298.0366, -91.1058, 447.5893 | skeletalmeshactormat_1 / true |
| 쿠크 | efskeletalmeshactorlookinfomat_1 | null | 0.0000, 0.0000, 0.0000 | null / false |
| d1 | cameraactor_6 | null | -297.6660, -90.2285, 457.3908 | null / false |
| d2 | cameraactor_5 | null | -292.4693, -90.2285, 456.1309 | null / false |
| d3 | cameraactor_4 | null | -287.9529, -90.2285, 455.3150 | null / false |
| d4 | cameraactor_3 | null | -282.2403, -90.2285, 454.3906 | null / false |
| d5 | cameraactor_10 | null | -315.0237, -90.2285, 444.4513 | null / false |
| d6 | cameraactor_9 | null | -309.8270, -90.2285, 443.1913 | null / false |
| d7 | cameraactor_8 | null | -305.0558, -90.2285, 442.6302 | null / false |
| d8 | cameraactor_7 | null | -299.5980, -90.2285, 441.4510 | null / false |
| 세이튼 | efskeletalmeshactorlookinfomat_2 | null | 0.0000, 0.0000, 0.0000 | null / false |
| pp1 | emitter_33 | par_q_rpct_exp_02 | -0.8663, 2.2072, 737.7977 | null / false |
| cam1 | cameraactor_17 | null | 0.0000, 0.0000, 0.0000 | null / false |
| cam6 | cameraactor_36 | null | -302.6581, -98.6521, 453.5023 | null / false |
| h1 | emitter_0 | par_q_rpctgate_01 | -3.5742, 1.4020, 740.4846 | null / false |
| pc1 | skeletalmeshactormat_3 | null | 0.0000, 0.0000, 0.0000 | null / false |
| pow1 | emitter_1 | par_q_rpct_exp_02 | -7.0697, 2.1743, 742.7337 | null / false |
| pow11 | emitter_5 | par_q_trail_01 | -7.0697, 2.1743, 742.7337 | null / false |
| pow12 | emitter_10 | par_q_rpct_exp_02 | -305.5211, -101.2050, 453.1934 | null / false |
| pc2 | skeletalmeshactormat_2 | null | 0.0000, 0.0000, 0.0000 | null / false |
| pow2 | emitter_2 | par_q_rpct_exp_02 | -6.3075, 2.1743, 744.5712 | null / false |
| pow21 | emitter_6 | par_q_trail_01 | -6.3075, 2.1743, 744.5712 | null / false |
| pow22 | emitter_11 | par_q_rpct_exp_02 | -304.7012, -101.2050, 455.0979 | null / false |
| pc3 | skeletalmeshactormat_4 | null | 0.0000, 0.0000, 0.0000 | null / false |
| pow3 | emitter_3 | par_q_rpct_exp_02 | -8.3411, 2.1743, 742.1411 | null / false |
| pow31 | emitter_7 | par_q_trail_01 | -8.3411, 2.1743, 742.1411 | null / false |
| pow32 | emitter_12 | par_q_rpct_exp_02 | -306.7925, -101.2050, 452.6009 | null / false |
| pc4 | skeletalmeshactormat_0 | null | 0.0000, 0.0000, 0.0000 | null / false |
| pow4 | emitter_4 | par_q_rpct_exp_02 | -4.6778, 2.1743, 744.8520 | null / false |
| pow41 | emitter_8 | par_q_trail_01 | -4.6778, 2.1743, 744.8541 | null / false |
| pow42 | emitter_13 | par_q_rpct_exp_02 | -303.1292, -101.2050, 455.3118 | null / false |
| cam2 | cameraactor_18 | null | -2.9927, 2.5232, 739.8415 | null / false |
| 책 | null | null | null | null / false |
| 책110 | null | null | null | null / false |
| cam3 | cameraactor_19 | null | -349.0059, -60.3536, 481.7285 | null / false |
| cam4 | cameraactor_20 | null | 0.0000, 0.0000, 0.0000 | null / false |
| card | emitter_9 | par_q_cardfly_01 | -289.9363, -106.1394, 449.0567 | null / false |
| card1 | emitter_15 | par_q_cardfly_01 | -289.5197, -116.5389, 425.0339 | null / false |
| card3 | emitter_34 | par_q_cardfly_01 | -283.6314, -115.8209, 441.7821 | null / false |
| dark | emitter_14 | par_q_field_01 | -300.9911, -115.9741, 453.5718 | null / false |
| cam5 | cameraactor_21 | null | 0.4455, 19.4045, 322.0824 | null / false |
| chip | emitter_16 | par_q_chip_01 | -305.3856, -101.3050, 441.6670 | null / false |
| chip1 | emitter_17 | par_q_chip_02 | -293.6745, -101.3050, 453.1920 | null / false |
| chip2 | emitter_18 | par_q_chip_03 | -293.8282, -101.3050, 454.3873 | null / false |
| chip3 | emitter_19 | par_q_chip_04 | -304.0526, -101.3050, 443.3347 | null / false |
| hpow | emitter_41 | par_d_rpct03_sk14_31 | -0.7622, 2.2645, 737.0757 | efskeletalmeshactorlookinfomat_2 / false |
| pow20 | emitter_42 | par_u_rpcz_doll_appear_cast_01_loc_int | -300.5492, -96.8233, 449.9554 | null / false |
| l1 | pointlightmovable_0 | null | -1.4445, 2.2797, 738.6213 | null / false |
| cann1 | null | null | null | null / false |
| cann2 | null | null | null | null / false |
| bpow | emitter_45 | par_d_rpct03_sk14_02 | -324.6088, -84.0177, 475.3826 | null / false |
| d10 | emitter_47 | par_g_boxdust_001 | -301.1906, -107.6342, 471.2911 | null / false |
| d20 | emitter_48 | par_g_boxdust_001 | -296.2716, -107.8632, 471.8976 | null / false |
| d30 | emitter_49 | par_g_boxdust_001 | -320.6065, -107.8632, 458.8454 | null / false |
| d40 | emitter_50 | par_g_boxdust_001 | -319.5003, -107.8632, 444.6870 | null / false |
| d50 | emitter_51 | par_g_boxdust_001 | -301.1654, -101.1157, 450.3049 | null / false |
| card0 | emitter_52 | par_l_rpcz_00-1_sk_02_1_loc_int | -301.4577, -101.3876, 450.7918 | null / false |
| imp | emitter_54 | par_g_rpcz_00_trumpet_b_loc_int | -301.4577, -100.6061, 450.7918 | null / false |
| spo | emitter_55 | par_u_rpcz_spotlight_01_loc_int | -302.6119, -101.3874, 451.1593 | null / false |
| sk | interpactor_3 | fm_b_plane_002 | -97.1233, 0.8070, -102.0407 | null / false |
| l2 | pointlightmovable_1 | null | -317.0468, -98.0381, 438.0245 | null / false |
| l3 | pointlightmovable_2 | null | -288.8999, -98.0381, 466.2210 | null / false |
| sound | null | null | null | null / false |
| fog | seqvar_named_0 | null | 0.0000, 0.0000, 0.0000 | null / false |
| po11 | emitter_31 | par_g_rpcz_00_weapon_spawn_loc_int | -1.4520, 2.5629, 738.1384 | null / false |
| loop | emitter_32 | par_g_ghostship_01 | -0.7334, 2.2977, 737.0775 | efskeletalmeshactorlookinfomat_2 / false |

### 3관문: 67개 group

| GroupName | Actor | Mesh / Particle | 초기 위치(m) | Base / HardAttach |
|---|---|---|---|---|
| 세이튼 | efskeletalmeshactorlookinfomat_0 | mn_rpct_06_sk | 8.7294, 10.6540, 318.3723 | null / false |
| 쿠크_rpcz | efskeletalmeshactorlookinfomat_2 | mn_rpcz_00_sk | 3.7180, 10.5258, 325.6007 | null / false |
| None | null | null | null | null / false |
| c1 | cameraactor_5 | null | -8.8697, 23.0227, 326.4416 | null / false |
| 쿠크세이튼_도착 | efskeletalmeshactorlookinfomat_3 | null | 0.0000, 0.0000, 0.0000 | null / false |
| pc_이펙트1 | null | null | null | null / false |
| pc_1 | skeletalmeshactormat_10 | null | 0.0000, 0.0000, 0.0000 | null / false |
| pc_2 | skeletalmeshactormat_11 | null | 0.0000, 0.0000, 0.0000 | null / false |
| pc_3 | skeletalmeshactormat_12 | null | 0.0000, 0.0000, 0.0000 | null / false |
| pc_4 | skeletalmeshactormat_14 | null | 0.0000, 0.0000, 0.0000 | null / false |
| c2 | cameraactor_19 | null | 2.0959, 11.0055, 324.5677 | null / false |
| 신규_마법진_이펙트 | emitter_4 | par_q_rpctgate_01 | 9.0825, 10.5907, 318.8225 | null / false |
| 마_s2 | emitter_10 | par_u_rpcz_beam_03_loc_int | 9.3314, 8.8124, 318.1649 | null / false |
| 마법진_루프 | emitter_5 | par_j_lightshafting_01 | 9.0826, 5.5939, 318.5420 | null / false |
| c3 | cameraactor_20 | null | 5.7171, 10.9355, 322.7396 | null / false |
| c4_p | cameraactor_32 | null | 5.2839, 1.0469, 948.1738 | null / false |
| c4 | cameraactor_22 | null | -0.6621, 1.1225, 942.7518 | cameraactor_32 / true |
| 이동구간_빛 | emitter_6 | par_m_glow_001 | 7.7374, -13.5022, 300.6265 | null / false |
| 이동구간_다크 | emitter_9 | par_q_darkfield_01 | 16.5575, -12.4640, 296.4181 | null / false |
| 신규_터널_이동구간 | emitter_7 | par_q_warpspace_01 | 4.0371, -11.7686, 310.1532 | null / false |
| 임시_터널구간_이펙트2 | emitter_3 | par_u_rpcz_beam_03_loc_int | 3.8844, -11.7484, 309.9015 | null / false |
| pc1_펑 | emitter_1 | par_q_rpct_exp_02 | 6.8179, 10.6790, 326.8945 | null / false |
| pc2_펑 | emitter_11 | par_q_rpct_exp_02 | 9.5711, 10.6790, 326.4112 | null / false |
| pc1_이동 | emitter_2 | par_q_trail_01 | 10.8696, 10.5757, 325.7777 | null / false |
| pc2_이동 | emitter_12 | par_q_trail_01 | 5.9238, 10.5757, 325.2101 | null / false |
| pc3_이동 | emitter_13 | par_q_trail_01 | 2.4684, 10.5757, 321.4011 | null / false |
| pc4_이동 | emitter_14 | par_q_trail_01 | 1.8429, 10.5758, 316.4504 | null / false |
| pc1_펑2 | emitter_15 | par_q_rpct_exp_02 | -4.7587, 1.0444, 950.9471 | null / false |
| 스폿라이트 | spotlightmovable_9 | null | 0.0457, 24.0382, 941.9393 | null / false |
| 라이트 | pointlightmovable_0 | null | -1.9076, 2.1638, 943.4412 | null / false |
| c4_1 | cameraactor_1 | null | -15.9164, 3.0481, 945.2603 | cameraactor_32 / true |
| c1_1 | cameraactor_10 | null | 2.2471, 10.9984, 323.9869 | null / false |
| c1_2 | cameraactor_11 | null | 2.2471, 10.9984, 323.9867 | null / false |
| pc1_터널구간 | emitter_18 | par_q_movingtrail_01 | 3.4096, 10.5934, 316.0610 | null / false |
| 쿠크_마법진_op1 | emitter_19 | par_g_rpcz_00_bazooka_c_loc_int | 3.7713, 10.5499, 325.6078 | null / false |
| 쿠크_마법진_op2 | emitter_20 | par_u_rpcz_bigarea_down_01_loc_int | 3.7713, 10.2285, 325.6078 | null / false |
| c1_a1 | cameraactor_12 | null | 2.4511, 10.5957, 328.9689 | null / false |
| c1_a2 | cameraactor_13 | null | 2.6462, 10.7753, 329.3068 | null / false |
| pc3_펑 | emitter_21 | par_q_rpct_exp_02 | 2.0406, 10.6790, 320.7083 | null / false |
| pc4_펑 | emitter_22 | par_q_rpct_exp_02 | 1.1477, 10.6790, 323.3973 | null / false |
| 마법_펑1 | emitter_23 | par_q_rpct_exp_02 | 9.4392, 10.2661, 318.5972 | null / false |
| c1_a0 | cameraactor_0 | null | 2.2946, 10.5957, 329.2569 | null / false |
| c5 | cameraactor_2 | null | -2.2844, 2.1825, 942.0952 | null / false |
| c6 | cameraactor_3 | null | -2.8122, 1.4710, 944.7998 | cameraactor_6 / false |
| c6_p | cameraactor_6 | null | -2.8121, 1.4607, 944.7995 | null / false |
| 터널_p1 | emitter_24 | par_q_movingtrail_01 | -1.1968, -9.9614, 315.7136 | null / false |
| 터널_p2 | emitter_25 | par_q_movingtrail_01 | -1.3647, -9.5303, 315.7974 | null / false |
| 터널_p3 | emitter_26 | par_q_movingtrail_01 | -1.7281, -10.3434, 315.9456 | null / false |
| 터널_p4 | emitter_27 | par_q_movingtrail_01 | -1.8410, -9.7238, 315.9916 | null / false |
| ak1 | emitter_28 | par_d_rpct_sk12_13_loc_int | 0.0000, 1.3218, 942.0800 | null / false |
| aaa | emitter_29 | par_q_rpct_exp_01 | -0.1356, 1.3271, 942.1135 | null / false |
| pc_11 | emitter_30 | par_q_trail_01 | -0.8087, 0.6410, 942.0800 | null / false |
| pc_12 | emitter_33 | par_q_trail_01 | 1.6192, 2.9808, 945.4022 | null / false |
| pc_13 | emitter_32 | par_q_trail_01 | 2.6200, 2.7254, 947.7004 | null / false |
| pc_14 | emitter_31 | par_q_trail_01 | 3.8755, 2.5837, 946.4891 | null / false |
| ss1 | emitter_34 | par_q_coloredpaper_01 | 0.0670, 1.4116, 943.3314 | null / false |
| la_1 | emitter_35 | par_q_rpct_exp_02 | -17.5096, 28.8218, 960.5302 | null / false |
| la_2 | emitter_36 | par_q_rpct_exp_02 | -18.5484, 27.8079, 959.6418 | null / false |
| la_3 | emitter_37 | par_q_rpct_exp_02 | -18.0333, 27.8079, 961.2834 | null / false |
| la_4 | emitter_38 | par_q_rpct_exp_02 | -19.2246, 27.8079, 960.1470 | null / false |
| 터널_펑11 | emitter_39 | par_q_rpct_exp_01 | -1.1980, -9.5937, 316.6800 | null / false |
| c4_a1 | cameraactor_9 | null | -13.4716, 2.2199, 955.4659 | null / false |
| 라이트1 | pointlightmovable_1 | null | 0.2633, -11.0767, 312.6821 | null / false |
| c1_a3 | cameraactor_18 | null | 12.8798, 12.2354, 324.2099 | null / false |
| c3_a1 | cameraactor_24 | null | -2.7876, -10.5722, 309.2046 | null / false |
| sound | null | null | null | null / false |
| 터널_다크1 | emitter_44 | par_q_field_01 | -69.1444, -3.4003, 330.5632 | null / false |
