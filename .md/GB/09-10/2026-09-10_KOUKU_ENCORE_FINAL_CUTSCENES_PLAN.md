# 앵콜 컷신 / 진짜 마무리 컷신 — 원본 조사·구현 계획·상세 인계 설명서

작성일: 2026-09-10.
저장소: C:/Users/USER/source/졸업팀폴/LostArk.
사용자 전달 파일: C:/Users/USER/OneDrive/바탕 화면/앵콜컷신 진짜 마무리 컷신.txt.

## 먼저 읽기 — 확인 범위와 문서의 한계

이 문서는 첨부 영상, 현재 PC의 원본 UPK, 기존 추출 JSON, 현재 Resources의 실제 모델, 현재 프레임워크 소스를 대조하여 만든 상세 계획·설명서다. 제품 기능을 이미 구현했다는 결과 보고가 아니다. 전체 C++ 파일을 그대로 교체하면 완성되는 코드 패치도 아니다.

확인한 것: 두 장면의 원본 후보를 좁히고 해당 패키지를 직접 다시 읽었다. 앵콜의 fakeui 신호, 깨진 유리 post-render 재질과 매개변수 시각, 주요 애니메이션·슬롯 가중치 일부, 마지막 컷신의 카메라 10개·배우 visibility 교체·소멸 효과 시각, 현재 모델 clip 존재와 기존 UI/재생 경로를 확인했다.

확인하지 않은 것: 원본 파티클 emitter 전체 속성, 재질 shader 수식과 모든 texture dependency, AnimTree A/B/C 슬롯 및 skeletal control 최종 합성 전부, 원본 camera aspect/class default, 모든 Kismet 연결, 현재 프레임워크의 실제 전체 재생·4인 동기화·사용자 화면 판정. 원본이 읽힌 부분과 프로젝트에서 선택한 구현 정책을 아래에서 구분한다.

따라서 “원본 근거를 갖춘 상세 구현 지침”이며 “완전 검증된 최종 구현서”는 아니다. 분량을 완전성 증거로 삼지 않는다. 이 문서의 작업 순서와 참조를 따라 구현하고, 적용 후 컴파일·수치 검사·사용자 화면 확인을 별도 RESULT에 남긴다.

## G00. 두 컷신은 게임 규칙상 완전히 다른 일을 한다

### 앵콜

겉으로는 던전 클리어처럼 보인다. 화면 가운데 클리어 문양과 문구가 나타나고, 보스가 가까이 와서 화면을 두드리고 깨뜨린다. 이후 계속 싸우겠다는 연기를 한다. 이는 가짜 클리어 연출이다. 이 순간에 보상 지급, 최종 clear flag, 돌아가기 버튼, 다음 던전 이동을 실제로 실행하면 안 된다.

이번 프레임워크 적용 목표: 3관문 본체 전투가 끝나는 서버 조건 → 앵콜 연출 → 빙고 전투 준비/입장 → 빙고 진행. 실제 앵콜 발동 조건은 서버 encounter 규칙으로 정의한다. 단순히 Client에서 HP bar가 0처럼 보였다는 이유로 발동하지 않는다.

### 진짜 마무리

빙고판에서 보스가 쓰러지고, 아파하며 연기하고, 작은 쿠크와 상호작용한 뒤 대사와 함께 사라진다. 여기서는 이미 서버가 빙고 전투 종료를 확정한 상태를 표현한다. 보스가 대사 중 다시 일어난다고 전투 HP/AI를 되살리는 것이 아니다.

이번 적용 목표: 서버 빙고 승리 확정 → 마지막 연기용 proxy 재생 → 소멸/암전 → 진짜 결과 UI와 현재 맵의 안전한 조작 상태. 최종 외부 복귀 목적지는 요청되지 않았으므로 새 자동 teleport를 추가하지 않는다. 현재 맵에서 결과를 보게 하는 것을 기본으로 한다.

### 두 흐름의 불변식

앵콜의 fake clear UI는 presentation만 바꾼다. 진짜 마무리의 raid result는 서버의 결과 상태를 소비한다. 두 UI가 같은 엠블럼 이미지를 사용해도 gameplay 의미까지 공유하지 않는다.

카메라가 끝났다는 사실과 레이드 승리가 확정됐다는 사실도 다르다. F1 Play/Seek/Stop은 보상·완료 횟수·관문 해금을 생성하지 않는다. 제품 실행과 관찰용 재생을 동일 자료의 서로 다른 authority mode로 분리한다.

## G01. 원본 패키지와 실제 선택 근거

원본 폴더:
C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages/

| 역할 | 패키지 파일 | 논리 Scene | Matinee / InterpData | 길이 / Group |
|---|---|---|---|---|
| 앵콜·가짜 클리어 | B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8L6.upk | SCENE07A | efseqact_matinee_23 / interpdata_23 | 23,333ms / 23 |
| 빙고판 마지막 연출 기본안 | B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8FD.upk | SCENE01B | efseqact_matinee_0 / interpdata_0 | 49,083ms / 23 |
| 마지막 연출의 다른 공간 변형 | B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8FK.upk | SCENE01C | efseqact_matinee_0 / interpdata_0 | 49,083ms / 23 |

SCENE07A에는 3.067초짜리 별도 interpdata_0도 있다. paperstage 펼침과 dust를 다루는 짧은 시퀀스이며, 23.333초 가짜 클리어 전체가 아니다. 짧은 항목을 잘못 선택하지 않는다.

### SCENE01B와 SCENE01C를 둘 다 조사한 이유

두 파일의 Director 컷 순서와 많은 배우 키가 같아서 이름만 보면 완전히 같은 파일로 오해하기 쉽다. 실제 비교 결과 actor 초기 좌표·회전, 일부 track 내용이 다르다.

SCENE01B cam01 초기 위치 약 (1.2911, 1.9755, 1145.2497).
SCENE01C cam01 초기 위치 약 (5.8277, 3.2855, 935.4484).
SCENE01B의 다수 배우/카메라는 Z≈1144~1148 빙고 공간에 있다. 현재 빙고 Debug 위치도 (-3.4, 0, 1147.44)다. 사용자의 마지막 영상은 검정·주황 격자 빙고판 위 연출이므로 기본 구현 소스는 SCENE01B로 선택한다.

SCENE01C는 Z≈935 부근 3관문 공간 쪽 변형이다. B의 카메라를 C의 배우와 섞거나 C의 좌표 일부만 B에 넣지 않는다. 두 패키지의 모든 내용이 동일하다거나 원본에서 어떤 난이도에 쓰였는지까지 이번 조사로 확정한 것은 아니다. 필요하면 각 변형을 별도 원본으로 보존하고 한 번에 하나만 사용한다.

### 조사 자료 위치

기존 추출본:
C:/Users/USER/OneDrive/바탕 화면/쿠크_컷신_전체추출_20260904/

SCENE07A_8L6.cutscene.v2.json / SCENE07A_8L6.director.json
SCENE01B_8FD.cutscene.v2.json / SCENE01B_8FD.director.json
SCENE01C_8FK.cutscene.v2.json / SCENE01C_8FK.director.json

이번 재열람 결과:
C:/Users/USER/source/졸업팀폴/LostArk/out/EncoreFinalPlan/SCENE07A.raw.json
C:/Users/USER/source/졸업팀폴/LostArk/out/EncoreFinalPlan/SCENE01B.raw.json
C:/Users/USER/source/졸업팀폴/LostArk/out/EncoreFinalPlan/SCENE01C.raw.json

조사 스크립트는 out/GateIntroPlan/extract_scene_matinee.py와 out/EncoreFinalPlan/source_details.py다. 전자는 알려진 track class만 수집하므로 별도 source_details로 post-render/weight/skel-control/AkEvent도 추가 조사했다. raw JSON만 보고 원본의 모든 기능을 다 읽었다고 판단하지 않는다.

## G02. 첨부 영상에서 관찰한 것과 원본 시간의 차이

### 앵콜 영상

파일: C:/Users/USER/OneDrive/바탕 화면/앵콜 컷신 .mp4.
녹화 길이 약 17.900초. 1.2초 간격으로 15개 프레임을 읽어 장면을 확인했다.

영상 0~2.4초: 던전 클리어 문양과 문구가 화면 중앙에 크게 보임.
3.6~6초: 보스가 화면 가까이 나타나고, 클리어 문구 뒤에서 연기함.
7.2~9.6초: 화면 균열과 파편/빛이 커지고 클리어 표식을 깨뜨리는 장면.
10.8~14.4초: 색종이와 함께 전신/상체 연기, 계속 싸우겠다는 대사.
15.6~16.8초: 얼굴 근접 장면.

영상은 원본 23.333초보다 짧다. 녹화 구간의 앞/뒤 잘림, 편집, 원본 재생 정책 차이를 확인하기 전에는 원본 시간과 녹화 시간을 동일시하지 않는다. 단지 길이 차이 5.433초를 모든 키에서 빼는 것은 검증된 동기화가 아니다. 균열 1단계·2단계·최종 파편이라는 복수 기준 장면을 대조해 offset/rate를 확인한다.

### 마지막 영상

파일: C:/Users/USER/OneDrive/바탕 화면/진짜 마무리 .mp4.
녹화 길이 약 47.333초. 2초 간격으로 24개 프레임을 읽었다.

0~4초: 보스 쓰러짐과 바닥에 누운 연기, 붉은 에너지.
6~14초: 위에서 본 구도와 얼굴 근접, 손을 들고 고통스러워하는 연기.
16~20초: 누워 있다가 상체를 일으키는 보스와 옆의 작은 쿠크.
22~26초: 보스 얼굴과 대사.
28~32초: 작은 쿠크/무기 근접과 배우 교체에 해당하는 연기.
34~38초: 합쳐진 형태의 근접 대사.
40~42초: 흰 연기/마술 효과.
44~46초: 붉은 소멸 효과와 빈 빙고판.

원본 49.083초와 녹화 길이가 다르다. 1.75초를 임의 offset으로 확정하지 않는다. 원본 slomo와 녹화 시작점을 함께 대조해야 한다.

브라우저 재생 바, 마우스 커서, 좌측 편집 제목 “3관문 끝”, 화면 밖 검은 녹화 여백, 스킵 투표 2/4는 구현 대상으로 자동 포함하지 않는다. 게임 화면에서 실제 필요한 연출과 녹화 UI를 분리한다.

## G03. 앵콜의 핵심은 world effect 하나가 아니라 화면 합성이다

### 원본의 직접 증거

SCENE07A 활성 Director는 -133ms에 cam을 선택한다. 그 뒤 다른 Director cut은 없다. 여러 구도로 보이는 것은 같은 cam의 이동과 camera parent 및 배우의 상대 운동을 먼저 봐야 한다. 2·3관문 시작 컷신처럼 여러 camera actor를 순서대로 바꾸는 구조로 임의 재작성하지 않는다.

Director EventTrack:
- 0ms: fakeui.
- 12500ms: s1.
- 13533ms: s1.
- 15433ms: s1.

별도 efinterptrackpostrendermaterial의 targetMaterial:
FX_MI.fx_d_brokenglass_01_tr.

post-render float parameter opacity:
12467ms=0 → 12500ms=1 → 15400ms=1 → 15433ms=0.
보간은 cim_constant다. 선형 fade로 바꾸지 않는다.

post-render float parameter type:
12500ms=0 → 13533ms=1.
이것도 cim_constant다. 원본 shader가 type 값을 어떤 texture/패턴 선택에 쓰는지 material 내부를 더 읽어야 한다. 화면 균열 단계가 바뀌는 시간으로는 활용할 수 있으나, type=1의 구체적인 샘플 수식을 이번 조사로 확정한 것은 아니다.

15433ms에는 break~break12의 12개 파편 emitter가 동시에 TRIGGER된다. 같은 시각 screen opacity가 0이 되는 것은 “2D 균열 표시가 내려가고 파편 연출로 넘어감”을 뒷받침한다. 정확한 shader/emitter 복원은 별도 검증하되 이 시간 분리는 유지한다.

### 화면을 쌓는 순서

권장 합성 순서:
월드와 보스 → 가짜 클리어 엠블럼/문구 → 화면 균열 → 파편/강조 → 자막 및 필요한 최상위 UI.

균열을 월드 EffectV2로만 만들면 UI는 그 위에 멀쩡하게 남을 수 있다. 화면 가운데 엠블럼까지 깨지는 인상을 내려면 가짜 UI를 포함하는 합성 순서를 정의해야 한다. 카메라 앞에 깨진 유리 mesh를 놓았다는 사실만으로 UI를 깨뜨린 것이 아니다.

현재 renderer의 후처리가 UI 이전인지, MainApp의 텍스트가 UI 이후인지 확인하고 실제 draw order를 정한다. MainApp::RenderRaidClearText는 따로 텍스트를 그리므로 이미지 레이어만 가린 뒤 글자가 위에 남는 실수를 주의한다.

### 기존 UI 재사용

현재 정본 레이아웃: Data/UI/RaidClear/RaidClear_Layout.json.
실물 존재 확인: Client/Bin/Resources/UI/RaidClear/RaidClearEmblem.png, RaidClearBgFlash.png.
애니메이션 관련 slot: RaidClear_avtive02, CoreShine, particleLooping, particleLighting, lineLeft, lineRight 등.

이 UI의 아트/배치 원리를 재사용할 수 있다. 다만 현재 Valtan의 Trigger_RaidClear/Update_RaidClear는 raw boss death를 읽고 자체 시간으로 진짜 결과 표시와 ReturnButton을 진행한다. 앵콜에서 이 함수를 호출해서 가짜 clear를 흉내 내지 않는다.

앵콜 owner가 다음만 소비하는 presentation용 view를 구성한다.
- 클리어 엠블럼과 빛.
- 중앙 문구와 등장 timing.
- 균열 단계/파편 단계.
- 해당 run의 명시적 elapsed.

ReturnButton은 생성하지 않거나 visible=false와 enabled=false를 모두 보장한다. 실제 clear callback·아이템 알림·파티 복귀 command는 연결하지 않는다. 숨긴 버튼도 hit-test에 남지 않게 한다.

### 균열 리소스의 현재 상태

KoukuSaydon Effect 물리 폴더에서 brokenglass 이름의 결과는 발견하지 못했다. 전체 Effect 폴더에는 다음 후보가 있다.
Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/fx_h_brokenglass_02_1.dds
Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/fx_h_brokenglass_11_1.dds
Effect/Esther/Balthorr/Textures/FX_TEX_HIGH_03/fx_h_brokenglass_03_1.dds
Effect/Esther/Balthorr/Textures/FX_TEX_HIGH_03/fx_h_brokenglass_02_1.dds

이는 이름이 비슷한 물리 후보이지 FX_MI.fx_d_brokenglass_01_tr의 정확한 texture임이 확인된 것은 아니다. 파일명에 glass가 있다고 최종 정본으로 자동 지정하지 않는다. 먼저 원본 material dependency를 따라가고, 연결이 어려우면 위 리소스를 사용자 확인을 거친 대체 표현으로만 사용한다. 기존 다른 class의 효과를 덮어쓰지 않는다.

원본 균열 재질이 scene color distortion을 사용한다면 단순 PNG overlay로 원본과 동일하다고 할 수 없다. GPU 내부의 정상적인 UI/render-target 합성과 shader가 필요하다. 이는 외부 화면 캡처가 아니지만, 실제 구현 전에 기존 CUIObject/renderer의 offscreen 합성 지원을 확인한다. ImGui 화면 캡처나 제품 UI를 ImGui로 대체하는 방법은 금지한다.

## G04. 앵콜 배우, 카메라 부모, 모델과 애니메이션

### 원본 actor 연결

원본 LookInfoKey: EFDLChar_MN_RPCT_07.MN_RPCT_07.
동일 actor의 SkeletalMeshComponent는 mn_rpct_05_sk를 참조한다. 이는 원본에서도 LookInfo 이름과 실제 mesh 이름이 다를 수 있다는 직접 증거다.

현재 Composition도 source profile MN_RPCT_07을 MN_RPCT_05로 alias한다. 실제 모델:
Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel.

cam group은 cameraactor_31, base는 cameraactor_32, hardAttach가 기록돼 있다.
쿠크세이튼_03은 efskeletalmeshactorlookinfomat_0, base는 cameraactor_31, hardAttach가 기록돼 있다.

따라서 root dummy → cam → 배우의 parent chain을 풀어야 한다. 이 장면의 보스가 카메라 가까이 오는 느낌은 월드 AI가 플레이어에게 접근하는 것이 아니다. camera-space 관계를 재현해야 한다.

초기 metadata 위치(m/Y-up):
cameraactor_32 약 (-1.6056, 1.0469, 942.0125).
cam 약 (-9.6056, 12.2606, 950.0125).
배우 _03 약 (-12.2938, 8.4526, 952.7009).

이 값들은 초기 actor metadata이며 MoveTrack/parent까지 평가한 최종 월드 pose로 간주하지 않는다. root만 옮기고 children에 같은 translation을 또 더하면 이중 이동한다. 원본 relative frame을 읽어 world pose로 한 번 풀거나, runtime에서 parent 순서대로 평가한다.

### _02와 _03은 무조건 두 전투 보스를 뜻하지 않는다

쿠크세이튼_02 group은 기존 metadata에서 actor가 미연결이다. _03에는 실제 actor reference가 있다. _02에 animation track이 존재한다고 같은 모델을 한 개 더 임의 spawn하지 않는다. Matinee VariableLinks의 named/runtime 대상 바인딩을 확인한다.

기본 구현은 _03의 명시적 actor를 컷신 proxy로 표현하고, _02가 실제 보스 presentation을 참조하는 경우에는 원본과 같은 handoff 시각을 명시한다. 어떤 binding인지 확정하지 못하면 해당 group을 원본 완전 복원으로 표시하지 않는다. 화면에 보스 두 개가 겹치지 않는 것이 필수다.

### _03의 slot a 원본 key

0 idle_normal_1 loop.
9333 att_battle_12_06, sourceStartOffset=1000ms.
12533 att_battle_1_01 loop.
14533 att_battle_13_01.
19667 / 20667 / 21667 / 22667 att_battle_13_02, sourceStartOffset=900ms, sourceEndOffset=1000ms, rate=0.4.

### _03의 slot b 원본 key

7833 att_phase1_1_06.
11500 att_battle_1_01.
14000 att_battle_1_01 rate=0.7.
18533 att_battle_13_02 rate=0.8.
20167 / 21167 / 22167 / 23667 att_battle_13_02, sourceStartOffset=900ms, sourceEndOffset=1000ms, rate=0.4.

23667ms key는 원본 InterpLength=23333ms 밖에 있다. 조사 raw에는 보존하되 제품 timeline 끝을 자동으로 23667로 늘리지 않는다. 목표 window에 실제 영향이 없는 key인지 source evaluator 기준으로 분류한다.

_02의 앞쪽 key는 비슷하지만 slot a 16167ms에 att_battle_6_03 rate=0.7로 이어지는 등 후반이 다르다. 두 track을 이름/번호 기준으로 덮어쓰면 잘못된 배우 연기가 된다.

### 이번에 추가로 읽힌 가중치

_03 slot a의 floattrack은 25개 key, _02 slot a는 13개 key다. 예를 들어 _03 slot a는 7833ms에서 1, 8000ms에서 0, 9333ms에서 0, 9667ms에서 1로 변한다. 그래서 7833ms에 시작한 slot b 모션과 a 모션을 단순 “시작 시간이 늦은 것 하나 선택”으로 재생하면 원본 blending과 다르다.

_03 slot b에는 0ms weight=1 키도 있다. 이를 보고 무조건 a+b를 정규화하면 된다고 단정하지 않는다. 실제 AnimTree가 slot을 어떤 순서·mask·override 방식으로 소비하는지 확인해야 한다. 빈 weight curve도 weight=0이라는 뜻이 아니다. class/track 기본값일 수 있다.

원본 animtree import: animblending_kuk9_mix. 또 j_dn, h_dn, h_up, ee_up, l_t2 등 skel-control track이 있다. 눈·입·고개와 관련돼 보이는 이름이더라도 이름만 보고 bone 축과 수식을 임의로 정하지 않는다. clip 재생만 구현하면 표정/대사가 원본과 달라질 수 있다.

## G05. 앵콜 효과·소리·마지막 암전

원본 group과 particle reference:
spark0 → par_c_ring_001.
spark → par_q_colorpaper_01.
disappear2 → par_q_rpct_exp_01.
move → par_y_cmdgr_03-1_spawn_01_loc_int.
break/break2/break5~break12 → par_e_shot_01.
break3/break4 → par_g_icebomb_01_pr.

spark와 spark0는 12500, 13533, 15433ms에 TRIGGER된다. 각 앞뒤 OFF key를 무시하고 하나의 3초 루프 효과로 바꾸지 않는다. TRIGGER는 burst/restart 의미와 소유 handle을 확인해야 한다.

move는 20233ms TRIGGER→20500 OFF, 20700 TRIGGER→20867 OFF다.
break 12개는 15433ms 동시 TRIGGER다. 목록에 열두 개가 있다고 전부 같은 위치에 생성하지 않는다. 각각의 actor local transform과 parent를 유지한다.

disappear2처럼 particle actor가 있다고 항상 재생되는 것은 아니다. 해당 active toggle과 기본 bAutoActivate, 연결 이벤트를 확인한다. 원본 import 목록 자체를 runtime autoplay 목록으로 사용하지 않는다.

원본 sound reference:
bgm_midnightc_ed_m18_scene_fakeclear.
bgm_midnightc_ed_m18_scene_fakeclear_skip.
scene_midnightc_ed_koukustopclearingdungeon.
scene_midnightc_ed_koukustopclearingdungeon_stop.

sound group에서 BGM 시작은 0ms, skip/end 관련 cue는 약 23322ms, 연기 sound event는 약 2100ms가 읽혔다. AkEvent 이름이 바로 WAV 경로는 아니다. 현재 Sound asset/catalog에 대응시키고 물리 음원을 확인한다. 원본 cue 이름만 넘기면 CGameInstance가 알아서 찾아 재생한다고 가정하지 않는다.

원본 fade: 22500ms=0 → 23333ms=1. 마지막은 완전 검정이다. 화면은 검정인데 입력과 빙고가 이미 시작돼 피해를 받지 않게 handoff 단계를 둔다. 목적지/보스 준비 확인 후 프로젝트 reveal을 수행한다. reveal 길이는 원본에 없던 프로젝트 정책임을 기록한다.

postprocess에는 shadow tint와 DOF focus inner radius/blur kernel key도 있다. 기존 scene profile의 blendMs를 opacity처럼 사용하지 않는다. 현재 지원 범위 밖인 DOF/tint 항목은 명시적으로 구현하거나 대체 상태로 기록한다.

## G06. 진짜 마무리의 카메라 10개와 시간 구간

SCENE01B/interpdata_0 활성 Director, 모든 transitionMs=0:

| 시작(ms) | group | 다음 컷까지 길이(ms) |
|---:|---|---:|
| -133 / 제품 시작 0 | cam01 | 4933 |
| 4933 | cam01_1 | 4634 |
| 9567 | cam02 | 3400 |
| 12967 | cam03 | 5833 |
| 18800 | cam04 | 3000 |
| 21800 | cam04_1 | 3200 |
| 25000 | cam05 | 1875 |
| 26875 | cam06 | 4058 |
| 30933 | cam07 | 6067 |
| 37000 | cam08 | 12083 |

원본 negative preroll -133은 unsigned runtime key로 저장하지 않는다. 제품 time=0 상태를 원본 evaluator에서 계산해 첫 pose로 저장한다.

shot JSON의 내부 blendInMs와 blendOutMs는 0으로 한다. 현재 Composition publisher는 CAMERA box 종료 후 blendOutMs까지 점유 구간으로 계산한다. 기존 카메라 preset의 900ms 복귀 값을 남기면 다음 shot과 overlap하거나 컷 사이마다 gameplay camera가 보인다. 마지막 shot에서만 별도의 finish handoff를 관리한다.

### FOV

원본 일부 FOVAngle: cam01 70도, cam01_1 30도, cam04_1 15도, cam05 80도, cam06 43도, cam07 15도, cam08 90도. 이것은 수평 FOV 원본 값이다. 전체 shot에 50도를 하드코딩하지 않는다.

몇 FOV key의 첫 시각이 Director cut보다 늦다. 예: cam01_1 FOV key 5367ms, cut 4933ms. runtime local first key를 잘못 434ms까지 비워 두지 말고 원본의 pre-first-key/default evaluation으로 local time=0 FOV를 계산한다. “아직 key 없음”을 0도 FOV로 대체하면 안 된다.

fovY=2×atan(tan(fovX/2)/aspect). radians/degrees를 분리한다. 이전 reader에 SOURCE_ASPECT=1.5가 있지만 이번 녹화는 2160×1440에 브라우저 여백이 포함되어 있다. 녹화 비율을 게임 viewport 비율로 확정하지 않는다. 원본 camera aspect constraint/class default와 실제 viewport를 확인 후 변환한다.

### roll

cam02에 대응하는 cameraactor_21 초기 원본 rotation에는 roll 5867 Unreal rotation units가 있다. 65536 units=360도 기준 약 32.23도에 해당한다. 최종 방향은 parent와 MoveTrack 평가를 거쳐야 하지만, 이 장면에서 roll을 무조건 0으로 버려도 된다고 말할 수는 없다.

현재 ValtanCinematicCamera pose는 Eye/LookAt/FovY라서 key별 up vector를 담지 못한다. Camera_Free follow roll이 있다는 것만으로 컷신 roll 지원이 완성된 것은 아니다. EffectAuthoringSequencer_Camera.cpp의 upVectors 샘플링을 참고하되 제품 camera ownership 경계에 공통 지원을 연결한다. authoring 툴을 제품 카메라 엔진으로 새로 쓰지 않는다.

## G07. 마지막 장면의 배우는 한 개가 아니다

### 원본 actor 역할

세이튼_1: efskeletalmeshactorlookinfomat_4, LookInfo MN_RPCT_07. 쓰러짐·아픔·일어나기 쪽 연기.
쿠크: efskeletalmeshactorlookinfomat_1, LookInfo MN_RPCZ_00. 옆에서 독립적으로 연기하는 작은 쿠크.
wp2: efskeletalmeshactor_0, component outer 관계로 wp_mn_rpct_05_sk를 사용하는 별도 무기 actor임을 확인.
쿠크세이튼: efskeletalmeshactorlookinfomat_0, LookInfo MN_RPCT_07. 후반에 나타나는 합쳐진 형태의 연기.
쿠크2: efskeletalmeshactorlookinfomat_3, LookInfo MN_RPCZ_00. drawScale 약 0.3, base는 세이튼_1 actor. 독립 쿠크와 다른 slot.

둘 이상의 actor가 같은 model을 써도 같은 instance가 아니다. actor slot별 initial pose, animation, visibility, parent, local scale을 각각 소유한다. 반대로 같은 모델을 쓴다고 항상 모두 표시하지도 않는다.

### 원본 visibility 교체가 핵심

세이튼_1: 0ms SHOW → 30933ms HIDE.
쿠크2: 0ms HIDE → 28542ms SHOW → 30933ms HIDE.
쿠크세이튼: 0ms HIDE → 30933ms SHOW → 41067ms HIDE.

30933ms에 cam07 컷도 발생한다. 즉 카메라 전환과 배우 교체가 같은 시간이다. 이들을 별개 Update 순서로 처리해서 한 프레임 둘 다 보이거나 둘 다 사라지지 않도록, 해당 scene time의 actor state를 전부 평가하고 한 번에 적용한다.

쿠크2는 처음부터 크게 소환된 몬스터가 아니다. base와 scale 0.3을 무시하면 어깨/상체 위 인형이 성인 크기로 겹친다. 단순히 현재 마리오 변신 캐릭터 크기를 재사용하지 않는다.

원본에서 보스가 41067ms에 HIDE된다고 Server world entity를 그 시각에 Client가 직접 despawn하지 않는다. proxy visibility와 이미 확정된 server encounter result는 별개다.

### 세이튼_1 slot a

3000: evt2_atpain01, endOffset=11362ms, rate=0.54696.
4167: evt2_atpain01, startOffset=638ms, rate=0.910718.
18167: evt2_atpain02_loop, loop.
31042: idle_normal_1, loop.

evt2_atpain01 원본 clip 길이는 현재 모델 기준 12초다. 첫 key는 12초 전체가 아니라 마지막 11.362초를 잘라 유효 약 0.638초만 쓰고 느리게 재생한다. “endOffset=11362”를 clip 종료 시각으로 오해하면 전혀 다른 동작이 나온다.

### 세이튼_1 slot b

0: dead_1.
5333: evt2_atpain01, endOffset=3068ms.
12967: evt2_atpain01, startOffset=7009ms, endOffset=2922ms, rate=1.088191.
16333: evt2_atpain01, startOffset=8928ms.
23433: att_battle_25_06_end, rate=0.426705.
25433: evt2_grp01.

slot c는 24125ms walk_normal_1 loop다. a/b/c 세 슬롯이 존재하므로 두 모션만 번갈아 선택하는 구현도 부족할 수 있다.

### 독립 쿠크

slot a: 0 idle_battle_1, rate=0.666991, loop.
slot b: 15700 att_battle_1_01 → 31467 att_battle_3_04.
slot c: 2200 idle_battle_1, 비루프.

### 후반 쿠크세이튼

slot a: 933 idle_normal_1 loop.
slot b: 25417 evt2_grp01 → 36867 att_battle_25_01 rate=0.709708.

보이기 시작하는 30933ms에 evt2_grp01을 0부터 시작하면 약 5.516초 늦다. 숨겨진 actor도 해당 scene time의 pose를 평가해 두고 SHOW될 때 바로 맞는 자세가 나오게 한다. visible=false라서 animation clock을 멈추면 안 된다.

### 쿠크2

slot a: -625 evt2_rpcz_idle_01 loop → 28917 evt2_rpcz_jump_start_01 endOffset=84ms → 29917 evt2_rpcz_idle_01 loop.
slot b: 27542 att_battle_3_06 → 29625 evt2_rpcz_jump_loop_01.
slot c: 27250 att_battle_2_07.

bag_hide, m_dn 등 skel-control도 있다. 얼굴/가방이 원본과 다르면 mesh 오류라고 단정하기 전에 이 control을 확인한다.

### 가중치 조사 결과의 의미

세이튼_1 slot a floattrack 10개 key를 읽었다. 일부 원본 곡선 값은 1보다 크다(예: 7667ms 부근 1.170188). raw export 단계에서 0~1로 잘라 원본을 훼손하지 않는다. 실제 AnimTree 적용 시 clamp하는지, interpolation overshoot인지 별도 평가한다. 쿠크2 slot c는 29125ms=1 → 29292ms=0 key를 가진다.

가중치 숫자 일부를 읽었다는 것과 원본 최종 bone pose를 복원했다는 것은 다르다. AnimTree의 slot 적용 순서, mask, skel-control, 첨부 무기 bone을 포함해 최종 pose를 검증해야 한다.

## G08. 현재 실제 모델의 clip 매핑과 누락된 무기 모션

Resources 기준 실제 파일:
Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel — 249 clips.
Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel — 91 clips.
Character/KoukuSaton/WP_MN_RPCT_05/WP_MN_RPCT_05.wmodel — 1 clip.
Character/KoukuSaton/WP_MN_RPCT_08/wp_mn_rpct_08_1_sk.wmodel — 0 clips.

원본 component 참조에도 wp_mn_rpct_05_sk와 wp_mn_rpct_08_1_sk가 있다. 원본에서 어느 body actor의 어느 part인지 outer/attachment 연결로 추적한다. 모든 무기를 같은 b_wp_1 하나에 임의로 겹치지 않는다.

| 역할 | 실제 runtime clip | 길이(초) |
|---|---|---:|
| 앵콜 접근/공격 | rpct00_att_phase1_1_06 | 1.8333 |
| 앵콜 연기 | rpct00_att_battle_12_06 | 4.3333 |
| 화면 타격 계열 | rpct00_att_battle_1_01 | 2.0 |
| 후반 대사/표정 몸체 | rpct00_att_battle_13_01 | 4.6667 |
| 후반 반복 편집 대상 | rpct00_att_battle_13_02 | 2.6667 |
| 다른 앵콜 group 후반 | rpct00_att_battle_6_03 | 2.2 |
| 쓰러짐 | rpct00_dead_1 | 3.0333 |
| 고통 연기 | rpct00_evt2_atpain01 | 12.0 |
| 고통 loop | rpct00_evt2_atpain02_loop | 1.3333 |
| 일어나기 연결 | rpct00_att_battle_25_06_end | 1.0 |
| 잡기/후반 연기 | rpct00_evt2_grp01 | 6.3333 |
| 소멸 직전 연기 | rpct00_att_battle_25_01 | 3.0 |
| 작은 쿠크 연기 | rpcz00_att_battle_2_07 | 3.9667 |
| 작은 쿠크 연기 | rpcz00_att_battle_3_06 | 1.9 |
| 작은 쿠크 연결 | rpcz00_att_battle_3_04 | 0.5667 |
| 작은 쿠크 idle | rpcz00_evt2_rpcz_idle_01 | 3.0 |
| 작은 쿠크 jump start | rpcz00_evt2_rpcz_jump_start_01 | 1.0 |
| 작은 쿠크 jump loop | rpcz00_evt2_rpcz_jump_loop_01 | 1.0 |

같은 att_battle_1_01이 두 모델에 존재해도 rig와 동작 길이가 다르다. 모델별 명시적 clip mapping을 사용한다. animation index를 저장 ID로 쓰지 않는다.

### 별도 wp2의 문제

원본 wp2 animation track은 idle_normal_1을 요구한다. 현재 WP_MN_RPCT_05.wmodel에는 wp_mn_rpct_05_sk.ao_att_battle_17_01 하나만 있다. 즉 “무기 모델도 있으니 원본 animation까지 준비 완료”는 아니다.

해결 절차:
1. 원본 wp2의 skeleton과 idle_normal_1이 실제로 정적 pose인지/뼈 변화가 있는지 PSK/PSA 또는 해당 AnimSet에서 확인.
2. 필요한 원본 clip을 동일 rig로 cook하여 기존 CModel 경로에 추가하거나 컷신 전용 cooked variant로 분리.
3. 실제 clip이 정적임을 확인한 경우에만 transform-only 표현을 동등 대체로 인정.
4. 없는 idle을 현재 att_battle_17_01로 이름만 바꾸거나 실패시 clip index 0을 재생하지 않는다.
5. source idle이 아직 확인되지 않았다면 해당 prop의 모션 표현을 미완료로 보고한다.

몸체/무기 hand socket은 기존 KoukuSaydonPresentationAssetService의 조립 경로를 재사용한다. 별도 wp2의 경우 바닥에 놓인 actor transform과 손 무기를 동시에 표시하지 않도록 visibility/ownership window를 정의한다.

## G09. 마지막 소멸 효과, 조명, 암전과 슬로모션

### particle timeline

fx: par_mp_deathmon_01_cine, 0ms ON → 12933 OFF.
magic: par_q_magicshow_01, 36900 ON → 40800 OFF.
magic0: 같은 particle 이름이지만 확인한 toggle key가 모두 OFF다. 같은 이름이라고 magic과 함께 자동 재생하지 않는다.
펑: par_q_rpct_exp_01, 40917 ON → 49042 OFF.

shaft group의 연결 actor는 spotlightmovable_0이다. 읽힌 활성 window는 37000 ON → 42375 OFF다. 이 항목을 shaft라는 이름만 보고 particle로 만들지 않는다. spotlight component의 cone/색상/밝기/감쇠 속성을 확인하고 기존 light occurrence로 연결한다. 이 stage light를 일반 point light로 대체했을 때 같은 화면이라고 단정하지 않는다.

40900ms에는 shk event가 있고, 40917ms 폭발 시작, 41067ms 후반 배우 HIDE다. 폭발이 시작된 뒤 약 150ms 동안 배우가 겹치며 가려지는 구조다. 폭발이 끝날 때 배우를 숨기는 것으로 바꾸면 소멸 시점이 크게 늦어진다.

원본 event 이름 shk는 카메라 흔들림으로 해석할 가능성이 있지만 문자열만으로 진폭·회전축을 확정하지 않는다. 연결 노드의 대상과 수치를 읽은 뒤 camera additive로 만들고, 타이밍만 원본에서 가져온 프로젝트 튜닝값은 별도 기록한다.

### sound

bgm_midnightc_ed_m20_scene_finish.
bgm_midnightc_ed_m20_scene_finish_skipend.
scene_midnightc_ed_raidcleared.
scene_midnightc_ed_raidcleared_stop.

AkEvent group에서 BGM 0ms, 끝 관련 cue 약 48974ms, 연기 sound 약 100ms가 확인됐다. source cue와 runtime WAV/OGG 등의 물리 파일, catalog ID까지 조인해야 한다. 이번 조사에서 음성 파일 재생까지 검증한 것은 아니다.

### fade

43900ms=0 → 47375ms=1, 원본 mode cim_curveauto.
timeline 끝 49083ms에는 이미 검정이 유지되는 상태다. 따라서 최종 결과 화면으로 넘어갈 때 검정 alpha를 어떻게 내려줄지 프로젝트 handoff가 필요하다. 원본 키를 끝냈다고 UI를 무조건 alpha=0으로 즉시 초기화해 화면이 튀지 않게 한다.

g_fog에는 fogdensity 변화도 있다. 그 값을 화면 암전으로 대체하지 않는다. fog와 full-screen fade는 서로 다른 입력이다. fog 변경은 해당 run 종료에 원복하고 다른 관문 조명을 지우지 않는다.

### slomo

0=1.0
1033=0.5
1467=0.3
1633=0.3
1767=1.0
9567=1.0
11433=0.5
12967=1.0

이 값은 Server fixed tick 배율로 사용하지 않는다. 진행 중인 레이드 전체 simulation을 느리게 하는 것이 아니라 컷신 전용 time mapping으로 처리한다. Matinee가 자체 time dilation을 무시하는 옵션인지 먼저 확인해야 timeline 49.083초와 실제 재생 wall time의 관계를 확정할 수 있다.

animation key의 playRate와 slomo를 중복 적용할 위험이 있다. clip sampling에는 명시된 playRate, scene→presentation 시간에는 정해진 cinematic mapping을 각각 한 번만 적용한다. camera/effect/sound가 서로 다른 clock을 사용하지 않도록 구현한다.

## G10. 기존 프레임워크에서 재사용할 파일과 현재 한계

모든 아래 경로의 기준은 C:/Users/USER/source/졸업팀폴/LostArk/다. 파일을 수정하기 전에 현재 git diff를 읽는다. 이번 조사 시점에 Client/Server/Shared/Data 여러 파일에 다른 작업의 미커밋 변경이 있다.

### 데이터와 publisher

Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json
  배우 proxy/별도 무기/효과 anchor의 transform·visibility·정재생 baked clip.

Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json
  앵콜 cam 및 마지막 10shot. 원본 global time을 occurrence start와 shot local time으로 분해.

Data/Effects/V2/Independent.json, Authored, Groups
  particle의 구현된 stable resource. par_* 원본 이름은 source reference이지 자동 재생 ID가 아니다.

Data/UI/RaidClear/RaidClear_Layout.json
  기존 아트/레이아웃 재사용 근거. 가짜 클리어 전용 view가 진짜 ReturnButton과 clear state를 쓰지 않게 한다. 원본 레이아웃 전체를 앵콜 시간으로 바꾸어 Valtan을 깨지 않는다.

Data/Maps/SequenceViewer.labels.json
  F1 한글 표시 이름. 제품 fallback은 Client/Bin/DataFiles/World/SequenceViewer.labels.json.

Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json
  관문 배치/트리거의 world authoring 정본. 전투 phase/run 상태를 이 배치 문서에 중복 저장하지 않는다.

Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json
  Kouku authoring 정본. 새로운 phase/cutscene 정의 계약은 이 정본과 기존 projector/Server 소비자에 연결한다. Gate1이라는 폴더명 때문에 3관문용 별도 정본/loader를 임의로 만들지 않는다. 현재 combat pattern 제약과 새 cinematic run 지원은 구분한다.

Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json
  project_kouku_saydon_composition.py의 project_encounter → projected_outputs가 생성하는 Product. 직접 편집할 정본이 아니라 읽기/검증 대상이다. 여기만 수정하면 다음 KoukuSaydon domain publish에서 변경이 사라진다.

Tools/MapPipeline/Publish-MapAuthoring.ps1
Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py
Tools/Build/Invoke-BuildDomainOwner.ps1
  authoring을 runtime으로 검증·발행하는 경로. Client/Bin/DataFiles를 직접 고쳐 정본처럼 쓰지 않는다.

### 기존 재생 경로

Client/Public/WorldSequenceDocument.h
  targetKind는 MAP_PLACEMENT/DEPLOY_PLACEMENT/OBJECT_RESOURCE. WORLD_SEQUENCE_ANIMATION_TRACK은 clipName/startMs/playbackRate/loop/holdLastFrame이며 source trim/reverse/A-B-C 합성을 직접 담지 않는다.

Client/Private/WorldSequencePlayer.cpp, WorldSequencePlayer_Objects.cpp, WorldSequenceObject.cpp
  실제 CModel 객체 생성과 time sample, 종료 정리. 새 렌더 모델 runtime을 만들지 않는다. finite duration 실행은 완료 때 Stop_Instance(id, targets, true)로 baseline 복구를 이미 수행한다. 중복 cleanup 구현으로 다른 owner를 지우지 않는다.

Client/Public/KoukuSaydonCompositionDocument.h, Client/Private/KoukuSaydonCompositionDocument.cpp
  animation occurrence에 sourceStartMs/playMs가 존재한다. 따라서 엔진 전체가 trim 불가능하다고 설명하면 틀리다. WorldSequence 대상과 보스 Composition 대상의 지원 차이다. 모델 07→05 alias와 gate/body join이 있다.

Client/Private/KoukuSaydonPresentationPlayer.cpp
  runEpoch/product bundle, CAMERA/EFFECT/SOUND/LIGHT 등 공통 실행과 소유 handle 정리. cutscene는 동일 실행기를 재사용하되 actor death 때문에 combat run이 종료되는 것과 별개 lifetime을 가져야 한다.

Client/Private/KoukuSaydonPresentationAssetService.cpp
  모델/무기 factory와 clip resolve. body-only world object가 모든 장비와 소켓을 알아서 조립한다고 가정하지 않는다.

Client/Public/Level_KakulSaydonArena.h, Client/Private/Level_KakulSaydonArena.cpp
  Sample_CompositionCamera/Stop_CompositionCamera/Resolve_CompositionFollowPose/Update_TriggerMoveFade. 컷신 원본 fade curve를 기존 위치변화 추정 fade에 억지로 태우지 않는다.

Client/Private/Level_ValtanArena.cpp
  Trigger_RaidClear/Update_RaidClear는 참고할 UI 표현 흐름이다. 앵콜에서 직접 호출할 기능이 아니다. 현재 진짜 clear timing은 약 2.25초 reveal+7.4초 hold라서 23.333초 앵콜 clock과도 다르다.

Client/Public/CombatHUDViewModel.h, Client/Private/CombatHUDViewModel.cpp
  raw boss death와 RaidClear text rect를 가진다. 가짜 UI를 위해 Set_BossDeadRaw(true)를 호출하지 않는다. 현재 raw death latch 자체도 레이드의 모든 phase가 완료됐다는 일반 증거가 아니다.

Client/Private/MainApp.cpp
  RenderRaidClearText가 별도 draw call이다. 가짜 UI의 문구와 파손 합성 순서를 고려해야 한다. 실제 결과 UI와 동시에 double draw하지 않게 owner별 표시 입력을 분리한다.

Client/Private/MainApp_SequenceViewer.cpp
  RefreshSequenceViewer와 typed playback 요청. 목록에서 F1 앵콜/진짜 마무리 항목과 source 정보, 오류를 표시할 위치.

Server/Public/GameRoom.h, Server/Private/GameRoom.cpp
  cutscene run/state/participant/phase commit의 권위. 플레이어 입력 거부, 전투 잠금, 중복 방지, 빙고 destination commit을 여기의 기존 room owner 흐름에 연결한다.

Server/Public/KoukuSaydonLogicRuntime.h, Server/Private/KoukuSaydonLogicRuntime.cpp
  빙고 진행/종료 및 현재 encounter state와의 연결을 확인한다. Client preview 완료를 이 runtime의 전투 승리로 처리하지 않는다.

Shared/Public/Network/PacketMessages.h, PacketType.h, Shared/Private/Network/PacketMessages.cpp
  필요한 run 상태/완료 상태의 typed 복제 계약. Client asset path/clip name을 Server authority 입력으로 넘기지 않는다. 새 field는 writer/reader/bounds validation/현재 protocol version/test를 같은 변경으로 갱신한다.

## G11. 배우 animation을 우리 런타임용으로 준비하는 방법

### 기본 선택

이 두 컷신은 고정 보스 배우 중심이다. 기존 CModel/CNpc 조립을 재사용하는 컷신 proxy를 만들고, 복잡한 clip trim/slot weight/skel-control 합성은 원본 해석 후 오프라인 최종 clip으로 베이크하는 방식을 우선한다. 각 actor slot별 하나의 정재생 baked clip과 world transform track으로 표현하면 runtime의 두 번째 animation engine이 필요 없다.

베이크한 clip은 새로운 original asset인 척하지 않는다. 원본 package/actor/track/key에서 변환된 프로젝트 산출물임을 기록한다. 다른 컷신이나 전투의 기본 model clip을 overwrite하지 않고 이름을 명시적으로 구분한다.

### 정확한 clip 시간 계산

clip 유효 시작 a=sourceStartOffset.
clip 유효 끝 b=clipDuration-sourceEndOffset.
b>a이어야 한다. offset이 유효 구간을 없애면 validation 실패다.
progress=(sceneTime-keyStartTime)×playRate.
정재생 non-loop sample=a+progress를 [a,b]로 clamp.
reverse non-loop sample=b-progress를 [a,b]로 clamp.
loop는 b-a 길이로 modulo하되 끝 경계 정책을 명시한다.

예: 앵콜 att_battle_13_02 길이 2.6667초, startOffset 0.9, endOffset 1.0이면 유효 구간 길이 약 0.7667초다. rate 0.4면 유효 구간 전체가 약 1.9167초에 걸쳐 재생된다. 그러나 slot a/b 다음 key가 그 전에 덮어쓸 수 있으므로 단순 loop 한 개로 대체하지 않는다.

### 뼈와 root 책임

원본 clip root movement와 Actor MoveTrack이 같은 이동을 둘 다 갖는지 확인한다. 두 번 더해서 보스가 지나치게 이동하면 안 된다. cutscene proxy에는 gameplay navigation projection이나 Client 로컬 AI를 적용하지 않는다.

body pose가 평가된 뒤 weapon socket world transform을 계산한다. 부착된 쿠크2도 parent pose/transform 이후 평가한다. world→local 변환과 model preScale=0.01을 혼동하지 않는다. 모델 단위 변환은 import 한 번, actor scale 0.3은 instance에서 한 번이다.

### 애니메이션 coverage 기록

actor별 source clip, 실제 rig, trim, weight, skel-control, socket을 표로 맞춘다. clip이 일부 없을 때 다른 clip을 fallback해서 “전부 성공”이라고 하지 않는다. 고정 pose임을 수치로 확인한 prop만 static 대체를 허용한다.

0초부터 정상 재생한 pose와 중간 seek한 pose가 같아야 한다. hidden actor도 원본 시각으로 샘플링한다. 30933ms에 등장한 배우가 첫 프레임부터 올바른 자세인지 확인한다.

## G12. 카메라·Transform·효과를 변환하는 공통 수학

UE3 cm/Z-up → 현재 m/Y-up: (x,y,z) → (0.01x,0.01z,-0.01y).
위치와 tangent의 축 변환을 함께 처리한다. 회전은 Euler 성분을 위치처럼 재배열하지 말고 원본 basis/quaternion에 좌표계 변환을 적용한다.

parent chain을 유지하는 경우 root부터 children 순서로 평가한다. absolute MoveTrack과 relative MoveTrack을 구분한다. 오프라인 world pose로 풀었다면 runtime에서 원본 base를 다시 곱하지 않는다.

같은 Group에 disabled=false로 읽힌 MoveTrack이 복수 존재한다. 예를 들어 앵콜 쿠크세이튼_03과 마지막 cam01이 그렇다. 이 목록을 모두 translation으로 더하거나, 보이는 첫 track만 임의 선택하면 원본과 다를 수 있다. 해당 class의 track 적용 순서·subtrack·조건/원본 default를 먼저 확인하여 하나의 최종 actor pose로 평가한다. 현재 raw의 enabled 목록만으로 이 arbitration까지 복원됐다고 말하지 않는다. MoveTrack source tangent가 생략된 raw를 그대로 LINEAR 보간한 화면은 원본 등가 검증이 끝난 카메라가 아니다.

원본 cubic tangent/interpMode를 먼저 평가한 다음 우리 runtime LINEAR에 맞게 adaptive key reduction을 한다. 기본 제안 오차: 위치 0.02m, 회전 0.5도, FOV 0.1도. 이는 프로젝트 검증 기준이다. 원본 engine 숫자라고 말하지 않는다.

CameraTool key 제한 64, WorldSequence track 제한 32, key 제한 256을 지킨다. 마지막 컷신 49초×60fps를 한 camera shot에 모두 넣지 않는다. Director window로 나누고 오차 검증으로 축약한다. 앵콜은 원본 Director 한 shot이지만 key 수가 제한을 넘으면 같은 카메라의 연속 segment로 분할하며 경계 pose/clock은 이어지게 한다. segment마다 follow 복귀 blend를 넣지 않는다.

FOV나 visibility 같은 discrete 전환은 key reduction에서 지우지 않는다. 화면 균열 type/opacity의 constant key를 smoothstep으로 바꾸지 않는다.

time mapping은 하나만 만든다. Server fixed tick으로 발급한 시작 시각/runEpoch → 동기화된 elapsed → cinematic source time → 각 track sample 순서다. clip별 playRate는 그 아래에서 한 번 적용한다. particle simulation에 소유 elapsed를 주면서 일반 Update에서도 또 증가시키지 않는다.

연속 정재생 이벤트는 이전 시간 < eventTime <= 현재 시간의 crossing으로 처리한다. 단, 최초 시작은 별도다. previous=0으로 초기화해서 이 비교만 하면 fakeui/BGM/visibility의 0ms 키가 빠진다. 시작 시 t=0의 지속 상태를 먼저 평가하고 0ms 단발 이벤트를 run당 정확히 한 번 처리한 다음 previous=0을 저장한다. 중복 start 메시지가 와도 초기 이벤트를 재발행하지 않는다.

음수 시각의 Director(-133ms)와 쿠크2 idle(-625ms)은 프리롤 상태다. t=0 pose와 현재 카메라를 계산할 때 음수 키에서 이어진 경과 시간까지 반영한다. 음수 시각의 sound/단발 효과를 전부 다시 발사하는 것과는 다르다. 지속 상태와 이벤트를 별도 분류한다.

Seek/늦은 합류는 정재생 crossing을 재실행하지 않고 목적 시각의 상태를 재구성한다. 오래된 파편 12개를 seek할 때 모두 새로 발사하지 않는다. 활성 효과/음악에 age 또는 재시작 정책을 적용하고 이미 지나간 단발음은 생략한다. 보상·관문 전이 이벤트는 presentation seek로 발생시키지 않는다.

## G13. 앵콜의 서버 전이 — 가짜 승리와 실제 빙고 입장을 분리

프로젝트 제안 phase 흐름:
GATE3_COMBAT → ENCORE_PREPARING → ENCORE_PLAYING → BINGO_HANDOFF → BINGO_COMBAT.

이 enum 이름은 설계 용어이며 현재 구현돼 있다는 뜻이 아니다. 기존 room/encounter 상태에 같은 의미가 있으면 재사용한다. 동일 역할의 두 번째 Room Manager를 만들지 않는다.

### 시작 조건

서버가 3관문 본체 종료 조건을 인정한다. 실제 조건은 현 encounter 규칙으로 결정하며 화면의 HP bar는 권위가 아니다. 승리 보상/최종 clear flag를 여기서 발행하지 않는다. 현재 combat DEAD/despawn 정리와 새 encore pending transition이 같은 tick에서 경쟁하지 않도록 처리 순서를 정한다.

권장 처리: 전투 종료 의미를 phase 결과로 먼저 확정하고, 전투 damage/AI를 종료한 뒤 presentation에 필요한 actor definition/pose를 고정한다. proxy는 이 기록을 소비하므로 원본 combat entity가 despawn되어도 연기가 사라지지 않는다. 실제 HP를 1로 고정해 죽음을 가짜로 미루는 방식은 사용하지 않는다.

### 준비

cutsceneId, runEpoch, source phase, destination BINGO, 참가자 roster를 stage한다. required model/shot/fake UI가 준비됐는지 Client가 typed ready 결과를 낸다. Server는 resource 경로를 알 필요 없이 ready 성공/실패와 정의 revision만 소비한다.

초기 권장 준비 timeout은 10초이며 프로젝트 정책이다. timeout/실패면 전투 종료를 취소해 죽은 보스를 부활시키는 것이 아니라, 안전한 phase-transition pending/연출 재시도 상태로 둔다. 이전 관문의 레이드 승리를 확정하지 않고 실패 이유를 표시한다.

관찰용 F1 Play/Replay와 제품 복구 Retry를 구분한다. F1 관찰 재생 성공은 pending phase를 해소하지 않는다. 제품 복구는 실패 UI의 명시적 '연출 준비 다시 시도' 명령을 typed sink로 제출하고, Server가 같은 room/pending phase/runEpoch/참가 권한과 전투 미재개를 확인한 뒤 새 준비 run을 만드는 별도 operation으로 구현한다. stale retry는 거부한다. UI는 이 operation 없이 관찰용 Play를 복구처럼 표시하지 않는다. 무한 자동 재시도 대신 실패 상태와 명시적 재시도를 유지하며, 퇴장/연결 종료 시 자기 pending 참여 상태와 입력 잠금을 기존 leave 경로에서 정리한다.

### 재생 중

참가자 입력은 Client와 Server에서 모두 잠근다. 이동 경로·pending action·combo buffer가 재생 뒤 갑자기 실행되지 않게 기존 입력 상태를 정리한다. 미리 스폰된 빙고 보스가 cutscene 중 공격하지 않도록 authority 상태를 둔다. F6 관찰은 제품 잠금과 별도 debug 정책으로 표시한다.

fakeui event는 앵콜 owner의 화면 표시만 켠다. Set_BossDeadRaw(true), Trigger_RaidClear, 보상/Return command와 연결하지 않는다.

### 빙고 handoff

원본 fade가 끝나는 23333ms에 검정을 유지한다. Server가 현재 빙고 player positions/navigation/collision, boss.kakulsaydon.bingo.saydon / BOSS_KAKULSAYDON_BINGO_SAYDON 정의를 검증한다. 실제 기존 destination spawn을 사용하며 예시 Debug 좌표를 플레이어 네 명에 그대로 겹쳐 넣지 않는다.

전원의 목적지를 stage한 뒤 한 tick에서 위치/관문/보스 활성 상태를 commit한다. 일부만 teleport한 뒤 나머지가 실패하는 흐름은 피한다. committed snapshot을 Client가 반영하면 실제 Character를 표시하고 proxy/fake UI/유리/파편을 정리한 다음 reveal과 follow camera로 인계한다.

현재 Debug_ActivateGate를 먼저 호출하고 그 callback에 앵콜을 붙이지 않는다. 이 함수는 전체 despawn→개별 spawn→로컬 플레이어 teleport를 각각 요청하는 기존 Debug 기능이며 단일 transaction이 아니다. 새 typed cutscene lifecycle이 stage/commit을 소유해야 한다. 기존 Debug 버튼은 기존 직접 테스트 기능으로 유지한다.

## G14. 진짜 마무리의 서버 전이와 결과 처리

제안 흐름:
BINGO_COMBAT → FINAL_RESULT_LATCHED → FINAL_PREPARING → FINAL_PLAYING → RESULT_PRESENTATION.

FINAL_RESULT_LATCHED의 뜻: 서버가 빙고 승리라는 게임 사실을 한 번 확정했다. 보스가 쓰러져 대사하는 동안 그 사실을 취소하거나 HP를 다시 채우지 않는다.

### 보상과 컷신의 관계

기존 권위 보상/clear 시스템이 있으면 그 시스템의 단일 완료 ID를 사용한다. 아직 없다면 이번 “컷신” 요청만으로 가짜 로컬 보상 지급 기능을 만들지 않는다. 결과 표시와 보상 지급 구현 상태를 분리한다.

Client가 videoFinished, Skip, Stop를 보냈다는 이유로 보상을 지급하면 재생마다 중복 보상을 얻거나 끊김으로 보상을 잃는다. 게임 결과는 server encounter/run identity로 idempotent하게 확정하고, 시각 결과창 노출은 컷신 handoff 시점으로 조절한다.

### resource 실패 시

진짜 승리가 확정된 후 cutscene resource가 실패해도 승리를 취소하지 않는다. 해당 연출 실패를 표시하고 정상 결과 상태로 안전하게 인계한다. proxy 준비 전에 combat corpse를 지워 놓고 화면과 조작이 영구 정지하는 상태를 만들지 않는다.

### 재생 중 death presentation 유지

현재 combat Complete Play는 죽은 보스를 대상으로 시작할 수 없다. Server GameRoom.cpp의 REJECTED_BOSS_DEAD 검사(조사 시 6580줄)는 HP0/DEAD를 거부하고, ABORTED_BOSS_DEAD 분기(7010줄)는 audition을 정리한다. Client KoukuSaydonPresentationPlayer.cpp는 HP0 snapshot을 제외하고(1307줄), liveBosses에 없는 session을 Stop_Session/erase한다(1332줄 부근). 줄 번호가 바뀌면 해당 심볼로 찾는다.

따라서 cinematic lifecycle을 실제 소비자에 연결하기 전에는 기존 combat pattern만 등록해서 최종 컷신을 재생할 수 없다. 고정 actor proxy와 cinematic run owner가 전투 entity 생존 여부와 독립된 수명을 가져야 한다. 기존 DEAD guard를 완화하거나 HP1 유지로 우회하지 않는다. 같은 CModel/Composition sampling 기능을 재사용하되 그 실행 수명의 소유자는 combat action이 아니라 컷신 run이다.

Server에는 actor model/animation path를 보내지 않는다. 컷신 ID/run/start/end/권위 결과만 복제한다. Client는 해당 정의로 source slot→model/clip을 resolve한다. 빙고 보스의 DEAD entity가 다시 살아 있다는 fake snapshot을 만들어 animation을 강제로 유지하지 않는다.

### 종료

41067ms 후반 배우 HIDE, 43900부터 fade, 47375 완전 검정, 49083 timeline 끝을 평가한다. 효과 잔여 수명은 해당 occurrence end policy로 처리한다. 최종 결과 UI를 준비하고 명시적 reveal을 한다.

기본은 현 맵의 서버 승인된 안전 상태에 남는다. 사용자 요청 없이 Bern/Lobby/2관문으로 자동 이동하지 않는다. 기존 정식 복귀 버튼이 지원되는 경우만 그 typed command를 연결하고, 지원되지 않으면 돌아가기 기능을 만든 척하지 않는다.

## G15. 각 run의 상태·소유권·실패 경로를 정확히 정의한다

runEpoch: 서버 발급, 해당 재생 한 번의 식별자. 이전 run의 늦은 stop이 새 run을 끊지 못하게 한다.
cutsceneId: 앵콜/마무리 정의 ID. 물리 경로 아님.
mode: F1 preview / 제품 encounter 전이. 같은 자료라도 권위 효과가 다르다.
source phase/destination phase: 게임 진행 전이. actor의 LookInfo profile과 별개.
startTick: 서버 기준 시작, Client frame counter 아님.
participants: 살아 있는/참여 자격 있는 PlayerId와 표시 정책. 컷신 중 재정렬하지 않는다.
prepared handles: Client가 stage 중 만든 proxy/model/UI/effect/light 소유 목록. commit 실패시 이 목록만 제거.
current actor states: slot별 Transform, visible, clip sample time. 한번 계산 후 적용.
screen overlay state: fake UI, glass type/opacity, fade alpha 및 owner.
completion latch: server-side phase/result 적용 여부. 중복 메시지에 다시 적용하지 않는다.
error: phase/actor/asset/clip/track/run과 구체적 실패 원인. 정상 기본값으로 숨기지 않는다.

Client와 Server의 lock 해제는 정상 완료, 준비 실패, runtime 실패, 중단, Level 이탈, disconnect 모두에서 다룬다. 다른 owner의 입력 잠금이나 camera override까지 지우지 않는다.

Level 이탈 후 fake UI/검정 화면/DOF/fog가 다음 Level에 남으면 정리 실패다. 전역 ClearAllEffects/ClearAllWorldEntities로 해결하지 않는다. run이 만든 handle만 종료한다.

앵콜 중 재시도는 동일 phase의 새 run으로만 만든다. 이미 BINGO_COMBAT인 방에 오래된 앵콜 start가 와도 다시 3관문 결과로 되돌리지 않는다. 마지막 컷신 F1 Replay는 이미 확정된 결과/보상 횟수를 변경하지 않는다.

## G16. F1에서 두 항목을 어떻게 보여줄 것인가

기존 MainApp_SequenceViewer.cpp를 확장하여 쿠크세이튼 탭에서 다음을 표시한다.

- 앵콜 — 가짜 클리어·화면 파손·빙고 전환.
- 진짜 마무리 — 빙고 종료·대사·소멸.

보조 정보: source package/InterpData, 원본 길이, actor/clip/이펙트 준비 상태, 현재 shot/elapsed, preview인지 제품인지, 실패 이유.

단계별 확인 지점:
앵콜: 가짜 UI / 보스 접근 / 1차 균열 12500 / 2차 균열 13533 / 파편 15433 / 얼굴 연기 / 23333 handoff.
마무리: 쓰러짐 / 고통 연기 / 독립 쿠크 / 28542 작은 proxy 표시 / 30933 배우 교체 / 36900 마술 / 40917 폭발 / 41067 배우 숨김 / 49083 끝.

Play는 전체 시각 자료를 재생한다. Stop는 선택 run만 정리한다. Seek는 authoring preview에서 state를 평가하되 보상·teleport·전투 소환을 다시 실행하지 않는다.

다른 Area에서 목록은 보여도 쿠크 맵 자산이 없으면 원점에 강제 생성하지 않는다. 필요한 Area와 승인된 이동 경로를 안내한다. 새로운 Level enum, direct Change_Level, F2~F5 기능키를 만들지 않는다.

현재 시퀀스 뷰어에는 트리거/맵 시퀀스/보스 패턴이라는 typed 목록 경계가 있다. 두 기능을 unsupported target ID로 억지로 보내지 말고 새 cinematic start/stop을 실제 command sink와 Server validator까지 연결한다. 단순히 한글 label 두 줄을 추가한 것을 실행 기능 완료로 처리하지 않는다.

## G17. 적용 단계를 나눠서 구현한다

### 첫 단계: source audit 확정

앵콜 23group, 마무리 B 23group을 package/export reference로 대조한다. Director/animation/visibility/toggle/fade/slomo와 알려지지 않은 class 수를 출력한다. old JSON의 decodedPartial track을 최종 데이터로 쓰지 않는다. duplicate export name은 package+index+outer로 구분한다.

### 두 번째: 배우 리소스와 누락 확인

05/쿠크 model과 필요한 clip을 매핑한다. wp2의 원본 idle 준비 여부를 해결한다. _02 미연결 actor와 작은 쿠크 parent/scale을 확인한다. raw PSK/PSA는 out/외부 추출 폴더에, 최종 WModel/texture만 Resources 기존 최상위 폴더에 둔다.

### 세 번째: 카메라와 고정 배우

앵콜 parent chain과 마지막 B 공간을 먼저 맞춘다. full effect 없이 정적 참조 배우와 카메라 구도를 사용자가 확인한다. 마지막 camera roll/작은 FOV를 포함한다. 이 단계는 전체 연출 시각 완료가 아니다.

### 네 번째: animation/visibility

clip trim과 sourceStart를 반영한 베이크/샘플을 연결한다. hidden actor 시간 평가와 30933ms 원자적 교체를 확인한다. 몸체와 무기의 소켓 위치가 프레임마다 맞는지 수치 검사한다.

### 다섯 번째: fake UI와 screen glass

진짜 결과 함수와 분리된 UI owner를 만든다. 원본 material을 조사하여 필요한 screen overlay 경로를 연결한다. 12500/13533/15433 단계와 UI/text draw order를 먼저 닫는다. 이미지가 있다는 이유로 screen material shader까지 복원됐다고 하지 않는다.

### 여섯 번째: particle/음성/후처리

spark/break/magic/소멸 occurrence를 현재 EffectV2 resource로 조인한다. 원본 emitter 미복원 항목은 별도 이름과 상태로 표시한다. 음성·자막은 source cue와 원문을 확인 후 추가한다. UI 원본 녹화 음성을 대충 잘라 재사용하는 것으로 source cue 연결을 대체하지 않는다.

### 일곱 번째: Server lifecycle

fake clear가 실제 clear를 건드리지 않는지, 빙고 transition과 최종 result가 중복 실행되지 않는지 연결한다. Shared/Server/Client를 한 기능 변경 단위로 닫는다. 클라이언트가 서버에 clip path를 보내는 우회는 사용하지 않는다.

### 여덟 번째: 제품 전투 전이에 연결

F1 시각 재생 성공 후 3관문 종료/빙고 종료의 서버 상태에 연결한다. combat despawn과 cinematic proxy lifetime 경합, 마지막 death latch와 true result의 차이를 확인한다. 기존 Debug_ActivateGate를 제품 transaction인 것처럼 재사용하지 않는다.

### 아홉 번째: 다시 재생/중단/실패 회귀

앵콜 2회, 마지막 2회, 중간 Stop, 타임라인 끝 직전 Stop, resource 실패, 4인 중 1인 disconnect, F6 전환, Level 이탈을 검사한다. 다른 마리오/미로/갈고리/빙고 데이터를 원복하거나 지우지 않는다.

## G18. 저장·프로젝트 등록·Drive 전달 지침

authoring 로드는 parse→validate→stage→commit이다. JSON 문법뿐 아니라 asset ID, clip, duplicate slot, finite Transform, valid quaternion, duration/key 제한을 검증한다. 실패한 항목의 이유를 보존하고 기존 정상 문서/Scene을 유지한다.

Data authoring의 해당 cutscene ID만 upsert한다. 전체 worldsequences.json을 작은 새 문서로 바꾸면 기존 맵 동작이 사라진다. published Runtime JSON은 publisher만 교체한다.

새 C++ 파일이 실제 필요하면 해당 프로젝트 Default/*.vcxproj에 ClInclude/ClCompile, *.vcxproj.filters에 실제 물리 폴더와 같은 책임 Filter를 함께 등록한다. 기존 필터를 재배치하지 않는다. 새 Data JSON은 Client 96.DataFiles의 None 항목으로만 등록한다. 새 generic 렌더 pass가 필요하면 Engine, LostArk 컷신 정책은 Client, 게임 결과는 Server에 둔다.

이 문서는 전체 C++ 교체 코드를 제공하는 문서가 아니다. 구현자는 실제 현재 파일의 변경을 보존한 G별 최종 코드와 정확한 project/filter 블록을 적용 전에 준비해야 한다. 존재하지 않는 helper나 JSON field를 만들어 넣고 실제 consumer를 연결하지 않은 상태로 종료하면 안 된다.

Drive 전달 후보:
1. 추가/재cook한 wp2 idle 또는 고정 배우 baked animation의 WModel.
2. 원본 균열 재질에서 복원한 texture/필요 mesh.
3. 새 EffectV2 정의가 참조하는 particle용 mesh/texture.
4. 새 Sound 파일과 fake clear용 UI 물리 아트가 생겼다면 그 파일.

기존 모델/UI를 그대로 재사용하면 불필요하게 전부 다시 배포하지 않는다. 실제 바뀐 Resources-relative ID와 물리 경로만 목록으로 준다. binary는 Git force-add하지 않는다. 원본 UPK/PSK/PSA와 임시 frame BMP는 제품 리소스 배포에 넣지 않는다.

## G19. 검증 명령과 사용자 확인 절차

다음은 구현 후 사용할 명령이다. 이번 계획서 작성으로 제품 빌드/publish를 실행했다는 뜻이 아니다.

```powershell
Set-Location -LiteralPath 'C:/Users/USER/source/졸업팀폴/LostArk'
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Validate -Scope Area
```

검증 성공 후:
```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish -Scope Area
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
git diff --check
```

Shared 변경이 있으면 현재 NetworkProtocolHarness를, Server 전이 변경이면 Server contract test를 기능 범위에 맞춰 별도로 실행한다. 일반 camera 튜닝마다 FullDiagnostic을 강제하지 않는다. JSON/XML parse와 최소 compile, 실제 입력/저장/재생 확인을 구분한다.

사용자 실행:
1. 맵툴 변경을 저장하고 Client를 종료한 상태에서 빌드·필요 publisher를 실행한다.
2. 공유 Server를 해당 담당 PC에서 실행한다. 현재 endpoint 192.168.0.14:7777을 유지한다. not-listening이라고 localhost로 복원하지 않는다.
3. 현재 PC는 client 판정이므로 사용자가 Debug x64 Client를 Ctrl+F5로 실행한다.
4. Lobby→KoukuSaydon의 서버 승인 진입 후 F1→시퀀스 뷰어→쿠크세이튼에서 새 항목을 선택한다. 새 항목은 구현 목표이지 지금 추가 완료 상태가 아니다.
5. 앵콜을 재생한다. fake UI는 보이나 ReturnButton/아이템 보상/전투 clear가 발동하지 않는지 확인한다.
6. 1차 균열, 2차 균열, 파편 전환, 얼굴 근접, 암전과 빙고 handoff를 확인한다.
7. 마지막 컷신은 빙고 공간 기준으로 재생하고 배우가 30933ms에 교체되는지, 무기가 손/바닥 역할에 맞는지, 41067ms 소멸과 마지막 결과 인계가 맞는지 확인한다.
8. Stop/정상 완료 후 이동·공격·F6를 확인한다. fake UI/검정/DOF/fog가 남으면 실패다.
9. 실제 4인에서 startTick/종료/결과 상태가 같고 1인 이탈이 다른 사람을 영구 lock하지 않는지 확인한다.

### 수치 검사 항목

- known Director/animation key count, 원본 InterpLength 보존.
- 앵콜 화면 opacity/type constant transition 시각 보존.
- hidden actor의 중간 pose와 정상 재생 pose 일치.
- source trim 후 유효 clip 구간 길이 양수.
- 30933ms 교체가 한 프레임 내 동시에 적용.
- 05/쿠크 socket 및 actor scale 정상, wp2 missing clip의 silent fallback 없음.
- 30/60/144fps 동일 scene time 결과 일치.
- 오래된 run 메시지 거부, 같은 start/stop 두 번 처리해도 한 번만 적용.
- 앵콜 전/후 actual raid-clear latch/보상 수치 변화 없음.
- 진짜 결과 재생/중단/재접속으로 중복 reward 또는 victory rollback 없음.
- 종료 후 이 run 소유 handle 0, 다른 effect/world actor는 보존.

## G20. 증상별 진단표

| 증상 | 먼저 확인 | 해결 방향 |
|---|---|---|
| 앵콜에서 바로 돌아가기 버튼 | 진짜 RaidClear 호출/원본 UI 시간 재사용 | fake UI view와 clock/command 분리 |
| 균열이 엠블럼 뒤에만 보임 | 월드 효과와 UI draw order | screen overlay 합성 순서 연결 |
| 문구만 깨끗하게 남음 | RenderRaidClearText 별도 후행 draw | 텍스트도 fake overlay owner 아래 정렬 |
| 보스가 카메라와 멀어짐 | cam→actor hardAttach 누락 | parent chain/world bake 중 한 방식만 적용 |
| 보스 두 개 겹침 | _02 미연결 group을 임의 spawn | binding 확인, actor visibility ownership |
| 마지막에 거대한 쿠크가 겹침 | 쿠크2 scale 0.3/base 누락 | source instance scale/parent 적용 |
| 30933ms에 잠깐 사라짐 | visibility/camera Update 순서 불일치 | 동일 scene time으로 일괄 state commit |
| 새로 나온 보스가 첫 자세로 시작 | hidden actor clock 정지 | 절대 scene time으로 clip sample |
| 무기가 이상하게 튐 | wp2 idle이 없어서 index0 fallback | 실제 clip cook 또는 정적 등가 확인 |
| 앵콜 후 진짜 클리어 처리됨 | raw DEAD latch를 전체 결과로 사용 | 서버 phase/result와 fake presentation 분리 |
| 마지막 컷신 중 보스 사라짐 | combat despawn이 proxy/run까지 정리 | cinematic owner 독립 수명 |
| 원본과 카메라 기울기 다름 | roll 누락 | 공통 camera pose up 지원 |
| 검정 화면에서 공격받음 | 빙고 AI/입력 unlock 시각 앞섬 | approved handoff/reveal 뒤 전투 시작 |
| C 변형 쓰니 다른 바닥에 있음 | B/C source 혼합 | B 빙고 공간 정의 일괄 사용 |
| 끝나도 검정/안개 남음 | 원본 마지막 fade/fog 상태를 무기한 유지 | finish overlay owner/reveal/restore |

## G21. 구현자에게 전달하는 최종 작업 지시

이 문서의 원본과 실제 파일을 먼저 읽으세요. 앵콜은 SCENE07A/interpdata_23, 빙고판 마지막은 SCENE01B/interpdata_0가 기본안입니다. SCENE01C는 별도 공간 변형이므로 섞지 마세요.

가짜 클리어는 진짜 결과 함수가 아닙니다. UI 아트는 재사용해도 Return/보상/clear latch를 호출하면 안 됩니다. FX_MI.fx_d_brokenglass_01_tr과 opacity/type key가 원본에 있으므로, 무조건 world particle 하나로 구현하지 말고 UI까지 포함한 화면 합성 순서를 설계하세요.

배우의 LookInfo07→실물05, 여러 actor slot, hidden 중 animation sample, 작은 쿠크 scale0.3, 30933ms 교체를 보존하세요. 현재 WP_MN_RPCT_05에는 원본 wp2의 idle_normal_1이 없으므로 없는 animation을 다른 clip으로 조용히 대체하지 마세요.

서버의 전투 결과와 컷신 재생을 분리하세요. 앵콜은 빙고로 진행하고, 진짜 마무리는 이미 확정된 결과를 연기합니다. 사망 actor가 없어졌다고 컷신이 끊기지 않도록 run 소유 presentation을 준비하고, 재생 완료를 reward 신호로 사용하지 마세요.

기존 WorldSequence/Composition/CModel/CMaterial/typed command/publisher 경로를 확장하세요. JSON 필드나 interface만 만들어 놓고 완료했다고 하지 마세요. parser/writer/actual consumer/rollback/test/project 등록을 한 기능 단위로 닫으세요.

Client/UI를 에이전트가 자율 실행하거나 캡처해서 visual PASS로 기록하지 마세요. 첨부 영상과 수치 근거를 사용하고 실제 화면은 사용자가 직접 확인합니다. 구현, publish, compile, 수치 검증, 사용자 화면 확인, Drive 리소스 전달을 분리해서 보고하세요.

## G22. 이번 조사 작업의 완료 범위

두 첨부 영상의 오프라인 프레임 분석, 세 원본 package 재열람, active Director와 주요 timeline/화면 재질/weight 일부 확인, 현재 WModel clip 목록과 누락 무기 clip, 기존 UI와 서버/클라이언트 경계 조사를 수행했다.

제품 C++/JSON/Resources를 이번 요청으로 변경하지 않는다. build/publish/실제 게임 재생은 실행하지 않았다. 조사 도구·raw 결과·이 계획서와 결과 문서만 작성했다. 아래 부록은 구현 시 원본 누락을 줄이기 위한 참고 자료이며 각 필드의 추출 한계는 본문의 안내를 따른다.

## G23. 부록 A — Group/배우/카메라/효과 연결 목록

이 부록의 행 수는 spawn할 몬스터 수가 아니다. Director, 소리, 라이트, named variable, 미연결 group도 각각 한 Group이다. 순서는 원본 InterpGroups 순서(아래 1-based)다. GameObject 저장 ID를 이 순번으로 만들지 않는다.

배우 이름·초기 Transform·resource 연결은 기존 cutscene.v2 목록과 이번 원본 재열람을 함께 참고한 inventory다. 알려진 track 개수는 이번 raw의 disabled=false 항목 수이며, 이것만으로 같은 property를 쓰는 복수 track의 최종 적용 우선순위까지 확인한 것은 아니다. 원본 group+export/outer를 보존한다. 같은 interptrackmove 이름도 다른 outer에서 반복된다.

(0,0,0)인 미해결 초기값을 월드 spawn 좌표로 복사하지 않는다. LookInfo actor/component default와 시간 0 MoveTrack을 평가해야 한다. null actor는 '눈에 안 보이는 새 보스를 생성하라'는 뜻이 아니다. 특히 앵콜의 쿠크세이튼_02/pung는 연결을 확인하기 전 임의 spawn하지 않는다.

### SCENE07A — 23개 Group

| 순서 / group | actor / 종류 | resource | base / hardAttach | instance scale | 초기 위치(m) / known track 수 |
|---|---|---|---|---|---|
| 1. cam | cameraactor_31 / cameraactor | — | cameraactor_32 / true | 1, 1, 1 | -9.6056, 12.2606, 950.0125 / 3 |
| 2. None | 미연결 / 비배우 | — | — / false/없음 | — | 미연결/비공간 / 3 |
| 3. 쿠크라이트 | pointlightmovable_0 / pointlightmovable | — | efskeletalmeshactorlookinfomat_0 / false/없음 | 1, 1, 1 | -8.3, 15.7987, 948.733 / 3 |
| 4. 쿠크세이튼_02 | 미연결 / 비배우 | — | — / false/없음 | — | 미연결/비공간 / 3 |
| 5. spark0 | emitter_9 / emitter | par_c_ring_001 | efskeletalmeshactorlookinfomat_0 / false/없음 | 2, 2, 2 | -12.4012, 10.0401, 952.8052 / 2 |
| 6. spark | emitter_3 / emitter | par_q_colorpaper_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 1, 1, 1 | -12.4012, 10.0401, 952.8052 / 2 |
| 7. disappear2 | emitter_2 / emitter | par_q_rpct_exp_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 1, 1, 1 | -12.4012, 10.0401, 952.8052 / 1 |
| 8. 쿠크세이튼_03 | efskeletalmeshactorlookinfomat_0 / efskeletalmeshactorlookinfomat | mn_rpct_05_sk | cameraactor_31 / true | 1, 1, 1 | -12.2938, 8.4526, 952.7009 / 6 |
| 9. pung | 미연결 / 비배우 | — | — / false/없음 | — | 미연결/비공간 / 1 |
| 10. move | emitter_5 / emitter | par_y_cmdgr_03-1_spawn_01_loc_int | efskeletalmeshactorlookinfomat_0 / false/없음 | 1, 1, 1 | -12.2938, 8.4526, 952.7009 / 3 |
| 11. break | emitter_10 / emitter | par_e_shot_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.6, 0.6, 0.6 | -12.4012, 10.0401, 952.8052 / 2 |
| 12. break2 | emitter_11 / emitter | par_e_shot_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.6, 0.6, 0.6 | -12.4012, 10.0401, 952.8052 / 2 |
| 13. break3 | emitter_12 / emitter | par_g_icebomb_01_pr | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.6, 0.6, 0.6 | -12.4012, 10.0401, 952.8052 / 2 |
| 14. break4 | emitter_13 / emitter | par_g_icebomb_01_pr | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.6, 0.6, 0.6 | -12.4012, 10.0401, 952.8052 / 2 |
| 15. break5 | emitter_15 / emitter | par_e_shot_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.6, 0.6, 0.6 | -12.4012, 10.0401, 952.8052 / 2 |
| 16. break6 | emitter_16 / emitter | par_e_shot_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.6, 0.6, 0.6 | -12.4693, 10.0626, 952.8716 / 2 |
| 17. break7 | emitter_1 / emitter | par_e_shot_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.5, 0.5, 0.5 | -12.4012, 10.0401, 952.8052 / 2 |
| 18. break8 | emitter_4 / emitter | par_e_shot_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.6, 0.6, 0.6 | -12.4012, 10.0401, 952.8052 / 2 |
| 19. break9 | emitter_6 / emitter | par_e_shot_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.6, 0.6, 0.6 | -12.4012, 10.0401, 952.8052 / 2 |
| 20. break10 | emitter_7 / emitter | par_e_shot_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.6, 0.6, 0.6 | -12.4012, 10.0401, 952.8052 / 2 |
| 21. break11 | emitter_8 / emitter | par_e_shot_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.4, 0.4, 0.4 | -12.4012, 10.0401, 952.8052 / 2 |
| 22. break12 | emitter_14 / emitter | par_e_shot_01 | efskeletalmeshactorlookinfomat_0 / false/없음 | 0.8, 0.8, 0.8 | -12.4012, 10.0401, 952.8052 / 2 |
| 23. sound | 미연결 / 비배우 | — | — / false/없음 | — | 미연결/비공간 / 0 |

### SCENE01B — 23개 Group

| 순서 / group | actor / 종류 | resource | base / hardAttach | instance scale | 초기 위치(m) / known track 수 |
|---|---|---|---|---|---|
| 1. 세이튼_1 | efskeletalmeshactorlookinfomat_4 / efskeletalmeshactorlookinfomat | mn_rpct_05_sk (component 재확인) | — / false/없음 | 1, 1, 1 | 기록 0; Move/LookInfo 확인 / 6 |
| 2. cam01 | cameraactor_0 / cameraactor | — | — / false/없음 | 1, 1, 1 | 1.2911, 1.9755, 1145.2497 / 7 |
| 3. None | 미연결 / 비배우 | — | — / false/없음 | — | 미연결/비공간 / 4 |
| 4. cam02 | cameraactor_21 / cameraactor | — | — / false/없음 | 1, 1, 1 | 2.4364, 2.0155, 1145.0613 / 7 |
| 5. 쿠크 | efskeletalmeshactorlookinfomat_1 / efskeletalmeshactorlookinfomat | mn_rpcz_00_sk (component 재확인) | — / false/없음 | 1, 1, 1 | 기록 0; Move/LookInfo 확인 / 5 |
| 6. wp2 | efskeletalmeshactor_0 / efskeletalmeshactor | wp_mn_rpct_05_sk (별도 actor) | — / false/없음 | 1, 1, 1 | 기록 0; Move/LookInfo 확인 / 3 |
| 7. cam03 | cameraactor_1 / cameraactor | — | — / false/없음 | 1, 1, 1 | -1.8182, 0.3855, 1147.0645 / 4 |
| 8. cam04 | cameraactor_23 / cameraactor | — | — / false/없음 | 1, 1, 1 | 0.7821, 0.5155, 1146.3942 / 3 |
| 9. cam05 | cameraactor_24 / cameraactor | — | — / false/없음 | 1, 1, 1 | 3.5863, 1.2855, 1144.5804 / 3 |
| 10. cam06 | cameraactor_2 / cameraactor | — | — / false/없음 | 1, 1, 1 | -0.5443, 0.5455, 1148.432 / 3 |
| 11. 쿠크세이튼 | efskeletalmeshactorlookinfomat_0 / efskeletalmeshactorlookinfomat | mn_rpct_05_sk (component 재확인) | — / false/없음 | 1, 1, 1 | 기록 0; Move/LookInfo 확인 / 4 |
| 12. 쿠크2 | efskeletalmeshactorlookinfomat_3 / efskeletalmeshactorlookinfomat | mn_rpcz_00_sk | efskeletalmeshactorlookinfomat_4 / false/없음 | 0.3, 0.3, 0.3 | 2.0632, 1.5701, 1144.2329 / 5 |
| 13. cam07 | cameraactor_3 / cameraactor | — | — / false/없음 | 1, 1, 1 | 0.004, 1.1655, 1147.2184 / 3 |
| 14. cam08 | cameraactor_4 / cameraactor | — | — / false/없음 | 1, 1, 1 | 1.0259, 1.1655, 1146.1748 / 4 |
| 15. magic | emitter_0 / emitter | par_q_magicshow_01 | — / false/없음 | 3.5, 3.5, 3.5 | 2.8847, 0.0611, 1144.3798 / 1 |
| 16. magic0 | emitter_1 / emitter | par_q_magicshow_01 | — / false/없음 | 4, 4, 4 | 2.8847, 0.0611, 1144.3798 / 1 |
| 17. cam01_1 | cameraactor_5 / cameraactor | — | — / false/없음 | 1, 1, 1 | 2.1873, 0.6818, 1145.2061 / 3 |
| 18. fx | emitter_2 / emitter | par_mp_deathmon_01_cine | efskeletalmeshactorlookinfomat_4 / false/없음 | 0.3, 0.3, 0.3 | 2.2079, 1.1368, 1144.512 / 1 |
| 19. cam04_1 | cameraactor_6 / cameraactor | — | — / false/없음 | 1, 1, 1 | 0.7821, 0.5155, 1146.3934 / 3 |
| 20. 펑 | emitter_6 / emitter | par_q_rpct_exp_01 | — / false/없음 | 1, 1, 1 | 2.8441, 1.3103, 1144.3798 / 1 |
| 21. shaft | spotlightmovable_0 / spotlightmovable | — | — / false/없음 | 1, 1, 1 | 12.5771, 4.8173, 1136.0287 / 3 |
| 22. sound | 미연결 / 비배우 | — | — / false/없음 | — | 미연결/비공간 / 0 |
| 23. g_fog | seqvar_named_0 / seqvar_named | — | — / false/없음 | 1, 1, 1 | 기록 0; Move/LookInfo 확인 / 0 |

### SCENE01C를 B와 혼용하지 않기 위한 차이표

카메라 배열/배우 이름이 같아도 initial pose와 일부 track이 다르다. 아래 표는 대표적인 공간 좌표와 환경 Group 차이를 보여준다. C에 B 전체를 평행이동한 것과 정확히 같다고 단정하지 않는다.

| 항목 | SCENE01B | SCENE01C |
|---|---|---|
| cam01 초기 좌표(m) | 1.2911, 1.9755, 1145.2497 | 5.8277, 3.2855, 935.4484 |
| cam02 초기 좌표(m) | 2.4364, 2.0155, 1145.0613 | 6.949, 3.3255, 935.1488 |
| cam03 초기 좌표(m) | -1.8182, 0.3855, 1147.0645 | 2.9113, 1.6955, 937.5593 |
| cam04 초기 좌표(m) | 0.7821, 0.5155, 1146.3942 | 5.4333, 1.8255, 936.6373 |
| cam05 초기 좌표(m) | 3.5863, 1.2855, 1144.5804 | 8.0463, 2.5955, 934.5574 |
| cam06 초기 좌표(m) | -0.5443, 0.5455, 1148.432 | 4.3131, 1.8555, 938.7954 |
| cam07 초기 좌표(m) | 0.004, 1.1655, 1147.2184 | 4.7398, 2.4755, 937.5339 |
| cam08 초기 좌표(m) | 1.0259, 1.1655, 1146.1748 | 5.6545, 2.4755, 936.3951 |
| cam01_1 초기 좌표(m) | 2.1873, 0.6818, 1145.2061 | 6.7153, 1.9918, 935.3173 |
| cam04_1 초기 좌표(m) | 0.7821, 0.5155, 1146.3934 | 5.4333, 1.8255, 936.6366 |
| 쿠크2 초기 좌표(m) | 2.0632, 1.5701, 1144.2329 | 6.4965, 2.8801, 934.3609 |
| magic 초기 좌표(m) | 2.8847, 0.0611, 1144.3798 | 7.3284, 1.3711, 934.4266 |
| fx 초기 좌표(m) | 2.2079, 1.1368, 1144.512 | 6.6678, 2.4468, 934.6245 |
| shaft 초기 좌표(m) | 12.5771, 4.8173, 1136.0287 | 16.1555, 6.1273, 925.1656 |
| 마지막 Group | g_fog / seqvar_named_0 | d_light / pointlightmovable_0 |

B에는 g_fog named binding이 있고 C에는 d_light가 있다. C의 23개 Group이 B와 의미까지 동일한 것은 아니다. B의 g_fog는 글로벌 fog 변수를 해석해야 하며 seqvar_named_0 자체를 3D 모델로 spawn하지 않는다.

## G24. 부록 B — 재추출한 전체 활성 AnimationControl 키

아래는 선택한 두 InterpData의 disabled=false AnimationControl 키다. time/start/end 단위는 ms. rate는 배속, L은 loop, R은 reverse, RM은 해당 key의 rootMotion flag다. 원본 clip 이름 그대로이며 MN_RPCT_05에는 rpct00_, MN_RPCZ_00에는 rpcz00_ 등 실제 이름 매핑을 확인한 뒤 연결한다.

활성 key라고 해서 그 시간에 배우가 보인다는 뜻은 아니다. visibility와 별개이며 가중치가 0일 수도 있다. 빈 clip key 배열인 fc1도 별도 weight/skel-control과의 관계를 지우지 않는다.

### SCENE07A

| group / slot / track | time | clip | start / end offset | rate | L / R / RM |
|---|---|---|---|---|---|
| 쿠크세이튼_02 / a / interptrackanimcontrol_20 | 0 | idle_normal_1 | 0 / 0 | 1 | 1 / 0 / 0 |
| 쿠크세이튼_02 / a / interptrackanimcontrol_20 | 9333 | att_battle_12_06 | 1000 / 0 | 1 | 0 / 0 / 0 |
| 쿠크세이튼_02 / a / interptrackanimcontrol_20 | 12533 | att_battle_1_01 | 0 / 0 | 1 | 1 / 0 / 0 |
| 쿠크세이튼_02 / a / interptrackanimcontrol_20 | 16167 | att_battle_6_03 | 0 / 0 | 0.7 | 0 / 0 / 0 |
| 쿠크세이튼_02 / b / interptrackanimcontrol_21 | 7833 | att_phase1_1_06 | 0 / 0 | 1 | 0 / 0 / 0 |
| 쿠크세이튼_02 / b / interptrackanimcontrol_21 | 11500 | att_battle_1_01 | 0 / 0 | 1 | 0 / 0 / 0 |
| 쿠크세이튼_02 / b / interptrackanimcontrol_21 | 14000 | att_battle_1_01 | 0 / 0 | 0.7 | 0 / 0 / 0 |
| 쿠크세이튼_03 / a / interptrackanimcontrol_20 | 0 | idle_normal_1 | 0 / 0 | 1 | 1 / 0 / 0 |
| 쿠크세이튼_03 / a / interptrackanimcontrol_20 | 9333 | att_battle_12_06 | 1000 / 0 | 1 | 0 / 0 / 0 |
| 쿠크세이튼_03 / a / interptrackanimcontrol_20 | 12533 | att_battle_1_01 | 0 / 0 | 1 | 1 / 0 / 0 |
| 쿠크세이튼_03 / a / interptrackanimcontrol_20 | 14533 | att_battle_13_01 | 0 / 0 | 1 | 0 / 0 / 0 |
| 쿠크세이튼_03 / a / interptrackanimcontrol_20 | 19667 | att_battle_13_02 | 900 / 1000 | 0.4 | 0 / 0 / 0 |
| 쿠크세이튼_03 / a / interptrackanimcontrol_20 | 20667 | att_battle_13_02 | 900 / 1000 | 0.4 | 0 / 0 / 0 |
| 쿠크세이튼_03 / a / interptrackanimcontrol_20 | 21667 | att_battle_13_02 | 900 / 1000 | 0.4 | 0 / 0 / 0 |
| 쿠크세이튼_03 / a / interptrackanimcontrol_20 | 22667 | att_battle_13_02 | 900 / 1000 | 0.4 | 0 / 0 / 0 |
| 쿠크세이튼_03 / b / interptrackanimcontrol_21 | 7833 | att_phase1_1_06 | 0 / 0 | 1 | 0 / 0 / 0 |
| 쿠크세이튼_03 / b / interptrackanimcontrol_21 | 11500 | att_battle_1_01 | 0 / 0 | 1 | 0 / 0 / 0 |
| 쿠크세이튼_03 / b / interptrackanimcontrol_21 | 14000 | att_battle_1_01 | 0 / 0 | 0.7 | 0 / 0 / 0 |
| 쿠크세이튼_03 / b / interptrackanimcontrol_21 | 18533 | att_battle_13_02 | 0 / 0 | 0.8 | 0 / 0 / 0 |
| 쿠크세이튼_03 / b / interptrackanimcontrol_21 | 20167 | att_battle_13_02 | 900 / 1000 | 0.4 | 0 / 0 / 0 |
| 쿠크세이튼_03 / b / interptrackanimcontrol_21 | 21167 | att_battle_13_02 | 900 / 1000 | 0.4 | 0 / 0 / 0 |
| 쿠크세이튼_03 / b / interptrackanimcontrol_21 | 22167 | att_battle_13_02 | 900 / 1000 | 0.4 | 0 / 0 / 0 |
| 쿠크세이튼_03 / b / interptrackanimcontrol_21 | 23667 | att_battle_13_02 | 900 / 1000 | 0.4 | 0 / 0 / 0 |
| 쿠크세이튼_03 / fc1 / interptrackanimcontrol_112 | — | clip key 없음 | — | — | — |

### SCENE01B

| group / slot / track | time | clip | start / end offset | rate | L / R / RM |
|---|---|---|---|---|---|
| 세이튼_1 / c / interptrackanimcontrol_2 | 24125 | walk_normal_1 | 0 / 0 | 1 | 1 / 0 / 0 |
| 세이튼_1 / a / interptrackanimcontrol_0 | 3000 | evt2_atpain01 | 0 / 11362 | 0.54696 | 0 / 0 / 0 |
| 세이튼_1 / a / interptrackanimcontrol_0 | 4167 | evt2_atpain01 | 638 / 0 | 0.910718 | 0 / 0 / 0 |
| 세이튼_1 / a / interptrackanimcontrol_0 | 18167 | evt2_atpain02_loop | 0 / 0 | 1 | 1 / 0 / 0 |
| 세이튼_1 / a / interptrackanimcontrol_0 | 31042 | idle_normal_1 | 0 / 0 | 1 | 1 / 0 / 0 |
| 세이튼_1 / b / interptrackanimcontrol_1 | 0 | dead_1 | 0 / 0 | 1 | 0 / 0 / 0 |
| 세이튼_1 / b / interptrackanimcontrol_1 | 5333 | evt2_atpain01 | 0 / 3068 | 1 | 0 / 0 / 0 |
| 세이튼_1 / b / interptrackanimcontrol_1 | 12967 | evt2_atpain01 | 7009 / 2922 | 1.088191 | 0 / 0 / 0 |
| 세이튼_1 / b / interptrackanimcontrol_1 | 16333 | evt2_atpain01 | 8928 / 0 | 1 | 0 / 0 / 0 |
| 세이튼_1 / b / interptrackanimcontrol_1 | 23433 | att_battle_25_06_end | 0 / 0 | 0.426705 | 0 / 0 / 0 |
| 세이튼_1 / b / interptrackanimcontrol_1 | 25433 | evt2_grp01 | 0 / 0 | 1 | 0 / 0 / 0 |
| 쿠크 / c / interptrackanimcontrol_2 | 2200 | idle_battle_1 | 0 / 0 | 1 | 0 / 0 / 0 |
| 쿠크 / a / interptrackanimcontrol_0 | 0 | idle_battle_1 | 0 / 0 | 0.666991 | 1 / 0 / 0 |
| 쿠크 / b / interptrackanimcontrol_1 | 15700 | att_battle_1_01 | 0 / 0 | 1 | 0 / 0 / 0 |
| 쿠크 / b / interptrackanimcontrol_1 | 31467 | att_battle_3_04 | 0 / 0 | 1 | 0 / 0 / 0 |
| wp2 / a / interptrackanimcontrol_2 | 0 | idle_normal_1 | 0 / 0 | 1 | 1 / 0 / 0 |
| 쿠크세이튼 / a / interptrackanimcontrol_2 | 933 | idle_normal_1 | 0 / 0 | 1 | 1 / 0 / 0 |
| 쿠크세이튼 / b / interptrackanimcontrol_5 | 25417 | evt2_grp01 | 0 / 0 | 1 | 0 / 0 / 0 |
| 쿠크세이튼 / b / interptrackanimcontrol_5 | 36867 | att_battle_25_01 | 0 / 0 | 0.709708 | 0 / 0 / 0 |
| 쿠크2 / c / interptrackanimcontrol_5 | 27250 | att_battle_2_07 | 0 / 0 | 1 | 0 / 0 / 0 |
| 쿠크2 / a / interptrackanimcontrol_3 | -625 | evt2_rpcz_idle_01 | 0 / 0 | 1 | 1 / 0 / 0 |
| 쿠크2 / a / interptrackanimcontrol_3 | 28917 | evt2_rpcz_jump_start_01 | 0 / 84 | 1 | 0 / 0 / 0 |
| 쿠크2 / a / interptrackanimcontrol_3 | 29917 | evt2_rpcz_idle_01 | 0 / 0 | 1 | 1 / 0 / 0 |
| 쿠크2 / b / interptrackanimcontrol_4 | 27542 | att_battle_3_06 | 0 / 0 | 1 | 0 / 0 / 0 |
| 쿠크2 / b / interptrackanimcontrol_4 | 29625 | evt2_rpcz_jump_loop_01 | 0 / 0 | 1 | 0 / 0 / 0 |

앵콜의 23667ms clip key와 23667/24167ms weight key는 23333ms InterpLength 바깥이다. raw에서 삭제하지 않지만 정상 scene window 밖에서는 재생하지 않는다. 영상이 더 짧다고 원본 window를 무조건 이 후속 키까지 늘리지 않는다.

## G25. 부록 C — 원본 AnimationControl float weight 곡선

상세 raw는 C:/Users/USER/source/졸업팀폴/LostArk/out/EncoreFinalPlan/animation_weights.json 이다. 위치/시간만 있던 기존 추출본을 완전한 A/B 혼합 정보로 오해하지 않도록 별도로 재열람했다.

각 숫자는 원본 floattrack에서 읽은 값이다. 빈 배열은 weight=0이 아니라 직렬화된 키를 찾지 못한 상태다. AnimTree/default/current control 경로를 확인해야 한다. B 슬롯이 항상 1처럼 보이더라도 최종 혼합이 normalized average인지, 순차 blend인지, bone mask 포함인지 확인 전 임의로 (A+B)/2를 하지 않는다.

time은 ms이며 tangent는 원본 시간 축(초) 기준으로 읽힌 scalar 미분값이다. Hermite 계산의 구간 길이를 ms로 넣을 경우 tangent 단위를 맞추거나 초 단위 평가 후 sample한다. 1.170188을 raw 단계에서 1로 잘라내지 않는다. 실제 AnimTree clamp 동작은 별도 검증한다.

### SCENE07A / 쿠크세이튼_02 / slot a / export 74

interptrackanimcontrol_20.

| time(ms) | value | arrive | leave | interpolation |
|---|---|---|---|---|
| 0 | 1 | 0 | 0 | cim_linear |
| 7833 | 1 | 0 | 0 | cim_curveautoclamped |
| 8000 | 0 | 0 | 0 | cim_curveautoclamped |
| 9333 | 0 | 0 | 0 | cim_curveautoclamped |
| 9667 | 1 | 0 | 0 | cim_curveautoclamped |
| 11500 | 1 | 0 | 0 | cim_curveautoclamped |
| 12000 | 0 | 0 | 0 | cim_curveautoclamped |
| 12533 | 0 | 0 | 0 | cim_curveautoclamped |
| 13000 | 1 | 0 | 0 | cim_curveautoclamped |
| 14000 | 1 | 0 | 0 | cim_curveautoclamped |
| 14533 | 0 | 0 | 0 | cim_curveautoclamped |
| 16167 | 0 | 0 | 0 | cim_curveautoclamped |
| 16767 | 1 | 0 | 0 | cim_curveautoclamped |

### SCENE07A / 쿠크세이튼_02 / slot b / export 75

interptrackanimcontrol_21.

직렬화된 weight key 없음. default/AnimTree 의미 확인 대상.

### SCENE07A / 쿠크세이튼_03 / slot a / export 77

interptrackanimcontrol_20.

| time(ms) | value | arrive | leave | interpolation |
|---|---|---|---|---|
| 0 | 1 | 0 | 0 | cim_linear |
| 7833 | 1 | 0 | 0 | cim_curveautoclamped |
| 8000 | 0 | 0 | 0 | cim_curveautoclamped |
| 9333 | 0 | 0 | 0 | cim_curveautoclamped |
| 9667 | 1 | 0 | 0 | cim_curveautoclamped |
| 11500 | 1 | 0 | 0 | cim_curveautoclamped |
| 12000 | 0 | 0 | 0 | cim_curveautoclamped |
| 12533 | 0 | 0 | 0 | cim_curveautoclamped |
| 13000 | 1 | 0 | 0 | cim_curveautoclamped |
| 14000 | 1 | 0 | 0 | cim_curveautoclamped |
| 14533 | 0 | 0 | 0 | cim_curveautoclamped |
| 15500 | 0 | 0 | 0 | cim_curveautoclamped |
| 16100 | 1 | 0 | 0 | cim_curveautoclamped |
| 18533 | 1 | 0 | 0 | cim_curveautoclamped |
| 19200 | 0 | 0 | 0 | cim_curveautoclamped |
| 19667 | 0 | 0 | 0 | cim_curveautoclamped |
| 20167 | 1 | 0 | 0 | cim_curveautoclamped |
| 20667 | 0 | 0 | 0 | cim_curveautoclamped |
| 21167 | 1 | 0 | 0 | cim_curveautoclamped |
| 21667 | 0 | 0 | 0 | cim_curveautoclamped |
| 22167 | 1 | 0 | 0 | cim_curveautoclamped |
| 22667 | 0 | 0 | 0 | cim_curveautoclamped |
| 23167 | 1 | 0 | 0 | cim_curveautoclamped |
| 23667 | 1 | 0 | 0 | cim_curveautoclamped |
| 24167 | 0 | 0 | 0 | cim_curveautoclamped |

### SCENE07A / 쿠크세이튼_03 / slot b / export 78

interptrackanimcontrol_21.

| time(ms) | value | arrive | leave | interpolation |
|---|---|---|---|---|
| 0 | 1 | 0 | 0 | cim_curveautoclamped |

### SCENE07A / 쿠크세이튼_03 / slot fc1 / export 76

interptrackanimcontrol_112.

| time(ms) | value | arrive | leave | interpolation |
|---|---|---|---|---|
| 0 | 1 | 0 | 0 | cim_curveautoclamped |

### SCENE01B / 세이튼_1 / slot c / export 73

interptrackanimcontrol_2.

직렬화된 weight key 없음. default/AnimTree 의미 확인 대상.

### SCENE01B / 세이튼_1 / slot a / export 71

interptrackanimcontrol_0.

| time(ms) | value | arrive | leave | interpolation |
|---|---|---|---|---|
| 3000 | 0 | 0.27027 | 0 | cim_curveauto |
| 4067 | 1 | 0.082873 | 0.082873 | cim_linear |
| 7667 | 1.170188 | 0 | 0 | cim_curveauto |
| 8433 | 0 | -0.013091 | -0.013091 | cim_linear |
| 18500 | 0 | -0.013091 | -0.013091 | cim_linear |
| 19167 | 1 | 0 | 0 | cim_linear |
| 23800 | 1 | 0 | 0 | cim_linear |
| 24600 | 0 | 0 | 0 | cim_linear |
| 31042 | 0 | 0 | 0 | cim_curveauto |
| 31667 | 1 | 0 | 0 | cim_curveauto |

### SCENE01B / 세이튼_1 / slot b / export 72

interptrackanimcontrol_1.

직렬화된 weight key 없음. default/AnimTree 의미 확인 대상.

### SCENE01B / 쿠크 / slot c / export 76

interptrackanimcontrol_2.

직렬화된 weight key 없음. default/AnimTree 의미 확인 대상.

### SCENE01B / 쿠크 / slot a / export 74

interptrackanimcontrol_0.

직렬화된 weight key 없음. default/AnimTree 의미 확인 대상.

### SCENE01B / 쿠크 / slot b / export 75

interptrackanimcontrol_1.

직렬화된 weight key 없음. default/AnimTree 의미 확인 대상.

### SCENE01B / wp2 / slot a / export 77

interptrackanimcontrol_2.

직렬화된 weight key 없음. default/AnimTree 의미 확인 대상.

### SCENE01B / 쿠크세이튼 / slot a / export 78

interptrackanimcontrol_2.

직렬화된 weight key 없음. default/AnimTree 의미 확인 대상.

### SCENE01B / 쿠크세이튼 / slot b / export 79

interptrackanimcontrol_5.

직렬화된 weight key 없음. default/AnimTree 의미 확인 대상.

### SCENE01B / 쿠크2 / slot c / export 82

interptrackanimcontrol_5.

| time(ms) | value | arrive | leave | interpolation |
|---|---|---|---|---|
| 29125 | 1 | 0 | 0 | cim_curveauto |
| 29292 | 0 | 0 | 0 | cim_curveauto |

### SCENE01B / 쿠크2 / slot a / export 80

interptrackanimcontrol_3.

직렬화된 weight key 없음. default/AnimTree 의미 확인 대상.

### SCENE01B / 쿠크2 / slot b / export 81

interptrackanimcontrol_4.

직렬화된 weight key 없음. default/AnimTree 의미 확인 대상.

## G26. 부록 D — 빼먹기 쉬운 원본 신호 전체 재생표

effect toggle/visibility/event를 같은 '한 번 실행 함수'로 합치지 않는다. visibility는 지속 상태, ON/OFF는 emitter 상태, TRIGGER는 원본 toggle action 의미를 확인할 단발 요청이다. 첫 0ms 및 Seek 처리 규칙은 G12를 따른다.

### SCENE07A

| group / track | 종류 | 원본 키 |
|---|---|---|
| None / interptrackevent_0 | interptrackevent | 0: fakeui; 12500: s1; 13533: s1; 15433: s1 |
| spark0 / interptracktoggle_5 | interptracktoggle | 12333: etta_off; 12500: etta_trigger; 12833: etta_off; 13333: etta_off; 13533: etta_trigger; 13867: etta_off; 15167: etta_off; 15433: etta_trigger; 16000: etta_off |
| spark / interptracktoggle_0 | interptracktoggle | 12333: etta_off; 12500: etta_trigger; 12833: etta_off; 13333: etta_off; 13533: etta_trigger; 13867: etta_off; 15167: etta_off; 15433: etta_trigger; 16000: etta_off |
| move / interptracktoggle_1 | interptracktoggle | 20233: etta_trigger; 20500: etta_off; 20700: etta_trigger; 20867: etta_off |
| break / interptracktoggle_4 | interptracktoggle | 15433: etta_trigger |
| break2 / interptracktoggle_5 | interptracktoggle | 15433: etta_trigger |
| break3 / interptracktoggle_6 | interptracktoggle | 15433: etta_trigger |
| break4 / interptracktoggle_7 | interptracktoggle | 15433: etta_trigger |
| break5 / interptracktoggle_8 | interptracktoggle | 15433: etta_trigger |
| break6 / interptracktoggle_9 | interptracktoggle | 15433: etta_trigger |
| break7 / interptracktoggle_1 | interptracktoggle | 15433: etta_trigger |
| break8 / interptracktoggle_2 | interptracktoggle | 15433: etta_trigger |
| break9 / interptracktoggle_4 | interptracktoggle | 15433: etta_trigger |
| break10 / interptracktoggle_5 | interptracktoggle | 15433: etta_trigger |
| break11 / interptracktoggle_6 | interptracktoggle | 15433: etta_trigger |
| break12 / interptracktoggle_7 | interptracktoggle | 15433: etta_trigger |

### SCENE01B

| group / track | 종류 | 원본 키 |
|---|---|---|
| 세이튼_1 / interptrackvisibility_3 | interptrackvisibility | 0: evta_show (evtc_always); 30933: evta_hide (evtc_always) |
| None / interptrackevent_0 | interptrackevent | 2067: shk; 40900: shk |
| 쿠크 / interptrackvisibility_2 | interptrackvisibility | 0: evta_hide (evtc_always); 12967: evta_show (evtc_always); 19400: evta_hide (evtc_always) |
| wp2 / interptrackvisibility_2 | interptrackvisibility | 0: evta_hide (evtc_always); 1600: evta_show (evtc_always); 26333: evta_hide (evtc_always) |
| 쿠크세이튼 / interptrackvisibility_0 | interptrackvisibility | 0: evta_hide (evtc_always); 30933: evta_show (evtc_always); 41067: evta_hide (evtc_always) |
| 쿠크2 / interptrackvisibility_1 | interptrackvisibility | 0: evta_hide (evtc_always); 28542: evta_show (evtc_always); 30933: evta_hide (evtc_always) |
| magic / interptracktoggle_0 | interptracktoggle | 0: etta_off; 32500: etta_off; 36900: etta_on; 40800: etta_off; 43267: etta_off |
| magic0 / interptracktoggle_0 | interptracktoggle | 0: etta_off; 32500: etta_off; 36900: etta_off; 40800: etta_off; 43267: etta_off |
| fx / interptracktoggle_0 | interptracktoggle | 0: etta_on; 12933: etta_off |
| 펑 / interptracktoggle_0 | interptracktoggle | 40917: etta_on; 49042: etta_off |
| shaft / interptracktoggle_1 | interptracktoggle | 0: etta_off; 37000: etta_on; 42375: etta_off |

원본 particle toggle이 없거나 전부 OFF인 경우 자동으로 ON하지 않는다. class default/actor initial active와 해당 event의 실제 연결을 조사한다. 데이터가 존재한다는 사실과 이 Matinee에서 화면에 나타난다는 사실은 다르다.
