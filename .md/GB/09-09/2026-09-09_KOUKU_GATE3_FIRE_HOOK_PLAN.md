# G00. 쿠크 3관문 외곽 불 회전·갈고리 대각선 통과 — Claude 구현 지시서

작성 기준: 2026-09-09 현재 작업 트리. 이 문서는 구현 계획이며 적용 완료 보고서가 아니다.
전달 대상: 이 저장소를 직접 읽고 수정할 구현 에이전트.
저장소: C:/Users/USER/source/졸업팀폴/LostArk
사용자 전달본: C:/Users/USER/OneDrive/바탕 화면/갈고리.txt
영상: C:/Users/USER/OneDrive/바탕 화면/아재패턴.mp4 — 0초부터 5초까지만 참고한다.

## G00-1. 이번에 구현할 것과 구현하지 않을 것

구현할 것은 정확히 두 가지다.

1. 3관문 전투장 외곽에 있는 불 모양 판 메시를 회전시킨다. 기존 28개 배치는 보존하고, 연출 중에만 추가 불 48개를 외곽에 만들어 영상처럼 밀도를 높인다.
2. 사슬이 붙은 갈고리가 나타나 월드에 고정된 대각선 경로를 따라 지나가고, 반대편 외곽에서 사라지게 한다. 서로 평행한 3개 경로에 3차례씩, 총 9개를 시간차로 보낸다.

현재 목표는 두 움직임을 실제 서버 승인 재생 경로에 연결해 사용자가 런타임에서 확인하는 것이다. QTE 키 입력, 아재패턴 성공/실패, 플레이어 잡기·끌고 가기, 즉사, 화상 피해, 무력화 수치, HP 조건 자동 발동, 플레이어 카메라 변경은 추가하지 않는다. 이 문서의 갈고리는 시각 오브젝트다. 피해 판정까지 구현했다고 보고하면 안 된다.

첫 구현은 별도 3관문 시각 테스트 패턴으로 만든다. 사용자는 F1에서 그 패턴을 재생한다. 기존 보스 패턴을 임의로 바꾸거나 맵에 들어왔다는 이유만으로 발동시키지 않는다. 후속으로 실제 아재패턴에 연결할 때에는 이 문서에서 만든 WORLD occurrence를 그 패턴의 필요한 시각에 옮기면 된다. 본 작업에서 아직 없는 아재패턴 전체를 만들어서는 안 된다.

## G00-2. 영상 관찰과 수치의 신뢰도

0~5초에는 외곽의 불 그림이 있는 판 형태와 사슬이 달린 여러 갈고리가 보인다. 플레이어와 카메라 이동이 섞여 있고 중간부터 영상 상단에 '2배' 표시가 나온다. 따라서 영상 1초를 원작 게임 1초라고 가정해서 속도를 산출하지 않는다. 5초 이후의 QTE 진행을 구현 근거로 사용하지 않는다.

확인된 사실: 아래에 적은 불 메시·텍스처·갈고리 파일·클립·현재 배치 ID·런타임 호출 경로.
프로젝트 초깃값: 추가 불 48개, 반경 19.5m, 불 회전 60도/초, 갈고리 48m/7초, 3개 평행 경로, 18초 테스트 길이. 이것은 원본에서 추출한 수치가 아니다. 사용자가 재생 후 조절할 시작값이다.

불 회전은 우선 '각 불 판의 위치를 유지한 채 Y축으로 제자리 회전'으로 구현한다. 불의 위치가 경기장 중심을 공전하는 것과 구분해야 한다. 영상만으로 정확한 원본 회전축·각속도·공전 여부를 확정하지 않았다. 이번 지시서의 기본 동작은 제자리 회전이고, 마지막 튜닝 절차에서 사용자에게 확인받는다. 공전이라고 추측해 경기장 바닥이나 전체 배경을 돌리지 않는다.

# G01. 실제 리소스와 배치 — 이 이름을 사용한다

## G01-1. 불은 메시와 텍스처가 모두 있다

원본 package/object 계열은 BG_RAD_KOUKUSATON_F / bg_rad_koukusaton_deco24*_sm_khb다.
실제 불 그림 텍스처 bg_rad_koukusaton_deco24_d_khb.dds를 열어 영상의 종이 불 형태와 대조했다. 기타 치는 광대인 DECO02B나 호박 장식 DECO15를 불로 착각하지 않는다. fm_d_rpct_07이라는 이름도 이번 불의 확정 근거가 아니다.

재사용할 Map Assets stable asset ID:

| 용도 | asset ID |
|---|---|
| 추가 불 D | MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB |
| 추가 불 E | MAP_B71A2EC9D778_BG_RAD_KOUKUSATON_DECO24E_SM_KHB |
| 추가 불 F | MAP_7AC8BB3D2FEE_BG_RAD_KOUKUSATON_DECO24F_SM_KHB |

예를 들어 D의 실제 파일은 다음이다.
C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB/MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB.wmodel

E와 F도 같은 폴더 규칙이다. 정확한 모델 상대 경로는 다음 카탈로그의 해당 ID 행에서 읽는다.
C:/Users/USER/source/졸업팀폴/LostArk/Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapassets

실제 열람한 대표 텍스처:
C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/MAP_B698A4C3F7BE_BG_RAD_KOUKUSATON_DECO24_SM_KHB/textures/79e70292f4fc_bg_rad_koukusaton_deco24_d_khb.dds

세 변형의 기존 WModel에 들어 있는 재질을 그대로 사용한다. diffuseTextureAssetId는 빈 문자열로 두어 내장 CMaterial을 사용한다. 임의의 노이즈 텍스처, 단색 빨강, 범용 불 파티클로 교체하지 않는다. 현재 static WorldSequenceObject는 TWO_SIDED 재질 경로로 렌더링한다. 모든 맵 오브젝트의 컬링이나 블렌딩을 변경할 필요가 없다.

중요한 크기 계약: Map Loader는 기존 Map WModel을 0.01 pretransform으로 읽는다. 따라서 새 World Object 리소스에도 modelPreScale=0.01을 사용한다. 카탈로그의 배치 기본 scale 1,1,1만 보고 modelPreScale=1이라고 쓰면 100배 커질 수 있다. 추가 불의 occurrence scale은 이 변환이 적용된 이후의 배수다.

원본 glTF bounds 참고(이미 미터로 추출된 값): D 높이 약 1.743m·폭 약 2.253m, E 높이 약 1.693m·폭 약 1.827m, F 높이 약 0.853m·폭 약 4.494m. 이는 원본 glTF bounds이며 런타임 적용 완료나 화면 크기 PASS가 아니다.

## G01-2. 갈고리도 실제 설치 파일과 9개 클립이 있다

원본 NPC 480710: Comment1='갈고리', Model=EFDLChar_MN_UMAX_00.MN_UMAX_00, ModelSize=115.
현재 물리 파일:
C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources/Character/KoukuSaton/MN_UMAX_00/MN_UMAX_00.wmodel
조사 시 크기: 1,428,240 bytes. 같은 폴더의 textures도 함께 필요하다.

기존 리소스 ID: world.object.kouku.hook
기존 모델 상대 ID: Character/KoukuSaton/MN_UMAX_00/MN_UMAX_00.wmodel
현재 animated=true, modelPreScale=0.01, scale 약 1.15,1.15,1.15다. 이 부모 리소스를 다시 만들거나 기본 모션을 덮어쓰지 않는다.

| 실제 클립 이름 | 실제 길이 | 이번 사용 |
|---|---:|---|
| Hook_idle_normal_1 | 2초 | 매달린 모습. 반복 재생, 경로 이동은 Transform만 사용 |
| Hook_idle_battle_1 | 2초 | 보존, 사용하지 않음 |
| Hook_respawn_1 | 약 1.333초 | 보존, 첫 버전에서는 사용하지 않음 |
| Hook_att_battle_1_01 | 7초 | 자체 root 이동이 있어 이번 합성 경로에 중복 사용하지 않음 |
| Hook_att_battle_2_01 | 1.5초 | 내려치기, 보존 |
| Hook_att_battle_3_01 | 1.5초 | 올라가기, 보존 |
| Hook_att_battle_4_01 | 1초 | 보존 |
| Hook_att_battle_4_02 | 1초 | 보존 |
| Hook_att_battle_4_03 | 1초 | 보존 |

idle 시작 프레임의 CPU skin bounds에 현재 preScale와 resource scale을 적용하면, 갈고리와 사슬 전체는 대략 폭 0.805m·높이 9.394m·두께 0.226m다. 갈고리 머리만의 크기가 아니라 사슬까지 포함한 전체 높이다. 이 bounds를 보고 플레이어보다 갈고리 머리가 9m라고 설명하지 않는다. 시작 프레임 최저점은 root보다 약 0.135m 위다. occurrence Y=1.2이면 최저점은 대략 1.335m가 된다. 실제 스킨 재생·카메라 외형은 사용자 확인 대상이다.

이번에는 원본 갈고리 몸체/사슬/재질을 사용하되, 이동 경로는 프로젝트가 저작한다. 원본 Spawn 좌표나 정확한 대각선 속도를 찾았다고 주장하지 않는다. 이번에 살핀 37081 DeployData의 직접 NPC 배치에서 480710 행은 찾지 못했다. 그것만으로 맵 스크립트 소환까지 없다고 단정할 수 없다.

## G01-3. 기존 28개와 관문 경계

현재 Authoring mapplacements에서 source가 LV_LUT_MIDNIGHTC_ED_SL05:export:로 시작하고 DECO24 계열인 외곽 불 28개를 확인했다. G04의 생성 코드는 이 28개를 ID로 추출해 existingFirePlacements 목록을 함께 출력한다. 28개가 아니거나 다른 시퀀스/자체 모션이 이미 소유하고 있으면 중단한다. 범위를 넓혀 다른 관문의 DECO24를 같이 고르지 않는다.

대표 기존 ID: 16440694042354962789 → SL05 export613 → 위치(6.7254858,1.36000001,924.852295).
현재 중앙 바닥 placement12451899878577673092의 XZ는 (0,942.080017)이다. 이것을 이번 경로의 기준 중심으로 사용한다. 원점(0,0,0)이나 플레이어의 현재 위치를 중심으로 사용하지 않는다.

현재 Get_DebugGates()에서 1관문과 3관문 이동 좌표가 둘 다 (-2.45,1.32,945.17)이다. 따라서 '그 좌표에 있으면 3관문', 'Kouku 맵에 있으면 3관문'이라고 판단하면 안 된다. Gate3 보스/패턴 ID로 재생을 제한한다.

# G02. 구현 구조와 수정 파일

## G02-1. 재생 흐름

사용자가 F1에서 3관문 시각 테스트 패턴 선택
→ 기존 Action Workbench/보스 도구의 Server-approved 패턴 재생 명령
→ Server가 패턴 시작 tick과 WORLD occurrence를 확정
→ 기존 S2C_WORLD_SEQUENCE_PLAY
→ CClientReplication::Consume_WorldSequencePlays
→ CLevel_KakulSaydonArena::Consume_OwnedWorldCue
→ CWorldSequencePlayer가 해당 instance를 재생
→ 기존 CWorldSequenceObject / CModel / CMaterial 또는 Map placement transform 갱신.

새 HookManager, FireManager, 별도 ParticleSystem, 로컬 몬스터 AI, 새 packet, 새 Level, 새 Shader를 만들지 않는다. 모델과 재질 경로를 서버에 넣지 않는다. 서버에는 기존 stable ID와 재생 시간/배치 계약만 전달된다.

## G02-2. 실제 변경 범위

| 구분 | 저장소 아래 위치 | 변경 내용 |
|---|---|---|
| 신규 저작 도구 | Tools/KoukuSaydonPipeline/author_gate3_fire_hook_fragments.py | G04 전체 코드. 기존 데이터를 읽어 추가할 완전한 JSON 블록을 stdout으로 출력. 원본 자동 덮어쓰기 없음 |
| 수정 | Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json | 불 리소스3·시퀀스 template5·instance5 추가 |
| 수정 | Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json | WORLD 정의5·Gate3 시각 테스트 패턴1 추가 |
| 기존 코드 재사용 | Client/Private/Level_KakulSaydonArena.cpp 및 WorldSequencePlayer.cpp | 수정 없음. 유한 durationMs WORLD occurrence의 기존 자동 복원·소멸 경로 재사용 |
| 자동 생성 | Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json | 기존 composition projector로 생성. 수동 편집 금지 |
| 자동 생성 | Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json | 같은 projector로 생성. 수동 편집 금지 |
| 자동 생성 | Client/Bin/DataFiles/Map 및 Server/Bin/DataFiles | 기존 publisher/domain owner만 사용 |
| 기록 | .md/GB/09-09/2026-09-09_KOUKU_GATE3_FIRE_HOOK_RESULT.md | 실제 적용/자동 검사/사용자 화면 확인을 분리 |

위 표의 절대 경로는 C:/Users/USER/source/졸업팀폴/LostArk/에 표의 위치를 붙인 것이다. H/CPP는 추가·수정하지 않으며 vcxproj/filters 변경도 필요 없다. Python은 빌드 대상 C++ 파일이 아니다. 기존 두 저작 JSON의 프로젝트 등록도 재사용한다.

Map Assets 카탈로그·기존 mapplacements·Navigation·Gameplay triggerBox·기존 mapmotions·플레이어/보스 전투 수치는 수정하지 않는다. 추가 불의 위치는 static MapTool 배치가 아니라 Composition WORLD occurrence의 placement로 저장된다. 런타임 연출이 끝나면 추가 불은 사라지고 기존 28개는 원래 모습/위치로 남는 구조다.

# G03. 움직임 명세와 함정

## G03-1. 기존 불 28개

새 instance: world.sequence.instance.kouku.g3.fire_hook.existing_fire
MAP_PLACEMENT binding28개를 한 template에 둔다. 현재 최대 track32개 이내다.
positionOffset=(0,0,0), scaleMultiplier=(1,1,1)을 모든 key에서 유지한다. 기존 position·크기·yaw는 baseline으로 보존한다.
Y축 회전 각도는 60도/초. 18초 동안 3바퀴다. 1.5초 간격으로 90도 차이의 quaternion key를 넣는다.

중요: 처음0도와 끝360도 두 key만 넣으면 quaternion의 최단 경로 보간으로 '안 도는 것처럼' 보일 수 있다. 반드시 0→90→180→270→360처럼 중간 key를 둔다. 생성 코드가 이 값을 계산한다. degrees를 quaternion 필드에 직접 넣지 않는다.

이 mixed/map 시퀀스에는 motionEnd=LOOP를 넣지 않는다. 현재 문서 검증은 STOP 외 완료 정책을 single OBJECT_RESOURCE binding에만 허용한다. 18초 테스트 한 번을 재생하고 종료한다. 무한 반복은 필요하지 않다.

## G03-2. 추가 불 48개

3종의 새 Object Resource가 이미 있는 D/E/F WModel을 참조한다. 각 리소스에 single-object 모션 하나를 둔다. 같은 모션을 여러 WORLD occurrence에서 각기 다른 placement로 재생한다. resourceId를 48개 복제하지 않는다.

중심 C=(0,942.080017), 반경 R=19.5, 개수 N=48.
각 i에 대해 a=2πi/N, x=Cx+R cos(a), z=Cz+R sin(a), y=0.05.
초기 면이 중심을 바라보도록 yaw=180-a(degrees)로 둔다. 해당 원본 평면은 local X가 얇고 local Z가 가로폭이므로 모델의 정면을 local Z라고 가정하지 않는다. 최종 정면 방향은 사용자 화면으로 확인한다.
D/E/F를 순서대로 사용하고, E만 -60도/초, D/F는 +60도/초로 움직인다. 이것은 단조롭게 한 장처럼 보이지 않도록 정한 초기 저작값이지 원본 난수 규칙이 아니다.
D/E occurrence scale=(1.2,1.5,1.2), F=(0.7,2.5,0.7). F는 원래 낮고 넓어 서로 다른 배수를 쓴다. 각 occurrence가 같은 resource scale을 다시 100배 곱하면 안 된다.

상한: 기존28+추가48=불76개, 갈고리최대9개. 객체가 시간에 따라 계속 누적되지 않는다. 새 개수는 FIRE_COUNT 상수 하나와 생성 결과 occurrence 수를 함께 바꾼다. 새 고정 타이머에서 매 프레임 Clone하지 않는다.

## G03-3. 갈고리의 8초 수명

새 instance: world.sequence.instance.kouku.g3.fire_hook.hook_diagonal
기존 부모 world.object.kouku.hook를 참조하고 부모 defaultMotionInstanceId는 보존한다.

| 오브젝트 나이 | local position | 의미 |
|---:|---|---|
| 0ms | (0,3,0) | 출발점 바깥쪽 상공에 생성 |
| 400ms | (0,0,0) | 매달린 높이까지 내려옴 |
| 7400ms | (0,0,48) | 7초 동안 앞으로48m 직진 |
| 7999ms | (0,3,48) | 도착점 바깥쪽 상공으로 퇴장 |
| 8000ms | (0,3,48), visible=false | 종료 및 제거 |

공중에 있는 사슬 끝이 아니라 갈고리 몸체 root의 경로다. 직진 구간에서 Y는 고정한다. 중력·포물선·플레이어 추적을 넣지 않는다. appearance/exit는 요청된 등장/퇴장을 재현하기 위한 프로젝트 합성 이동이다. 원본 Respawn/Up 애니메이션을 복구했다고 표현하지 않는다.

Animation은 Hook_idle_normal_1을 loop=true로 샘플링한다. velocity, acceleration, angularVelocity, revolution은 모두0이다. Transform key 하나가 이동 권한을 가진다. Hook_att_battle_1_01과 동시에 경로를 더하지 않는다.

occurrence yaw=45도이며 local +Z 이동이 world XZ 대각선으로 변환된다. 화면 좌표로 x+=speed,y+=speed 하지 않는다. 카메라를 돌려도 월드 경로는 바뀌지 않아야 한다. 다른 대각선이 필요하면 WORLD occurrence yaw와 시작점/종점만 바꾸고, 카메라 View의 right/forward를 이동 방향으로 사용하지 않는다.

평행 3개 경로 간격4m. d=(√0.5,√0.5), n=(√0.5,-√0.5).
lane 0/1/2의 lateral은 -4/0/+4m.
S=C-24d+lateral*n, E=C+24d+lateral*n. Y=1.2.
모든 occurrence.scale=(1,1,1)이므로 local48m가 world48m다. 나중에 갈고리 크기를 바꾸려고 occurrence.scale을 바꾸면 경로 거리에도 배수가 걸릴 수 있다. 시각 크기만 바꿀 때는 별도 부모 리소스의 모델 배수를 저작하는 방법을 사용하고 기존 공유 부모를 무턱대고 바꾸지 않는다.

3회 wave ×3개 lane =9개. 시작 시각은 500+wave*2400+lane*300ms. 마지막 시작은5900ms, 마지막 종료는13900ms. 불 연출은18000ms에 끝난다. 이번에선 rand()를 사용하지 않는다. 동일한 Server 재생에 클라이언트마다 다른 갈고리가 나오면 안 된다.

## G03-4. Gate3 전용 패턴을 만드는 이유

현재 파일명에 Gate1이라고 쓰여 있어도 Composition은 여러 관문의 물리 보스를 묶어 관리한다. 새 Gate3 전용 JSON 런타임을 만들지 않는다.
새 pattern.actorProfileId는 MN_RPCT_05다. 원본 참조 animation.profileId는 MN_RPCT_07을 사용할 수 있지만 물리 actorProfileId=MN_RPCT_07은 현재 검증기가 거부한다. 현재 resolve 함수가07을05로 매핑한다.
gateId=GATE3, targetBossPlacementId=boss.kakulsaydon.g3.saydon을 정확히 지정한다.
보스는 이 시각 테스트 동안3초짜리 rpct00_idle_normal_1을6단계 이어서18초 대기한다. 저작 원본의 action0/stage-003/animation-000을 사용하며 referenceRevision은 실제 참조 파일에서 읽는다. 가짜 revision이나 임의 공격 clip을 만들지 않는다.

WORLD occurrence는 기존불1+추가불48+갈고리9=58개다. 현재 패턴당 최대128개 이내다. 각 template의 objectMotion.count는1을 유지한다. count를48로 올리면 서로 다른 WORLD placement가 아니라 같은 emitter 계산에 복수 객체가 생기는 것이므로 요구한 원형 배치를 대신하지 못한다.

현재 Composition 계약상 playAllPatternIds는 PRODUCT 패턴 전부를 저작 순서대로 포함해야 한다. 따라서 새 테스트 패턴도 이 배열 끝에 들어가며 전체 PRODUCT 재생 목록에 포함된다. 이는 HP 조건이나 관문 입장 자동 발동을 추가하는 것이 아니다. 이 배열에서만 빼서 숨기면 validator가 거부한다. 단독 확인에는 반드시 위 한글 패턴만 선택해 Complete Play (Server)를 사용한다.

# G04. JSON 블록 생성 도구와 적용 순서

## G04-1. 도구 역할·함수·변수

신규 파일 위치: C:/Users/USER/source/졸업팀폴/LostArk/Tools/KoukuSaydonPipeline/author_gate3_fire_hook_fragments.py
이 도구는 read-only다. 현재 catalog/placements/worldsequences/composition/actionreference를 읽고, 추가할 완전한 JSON 블록을 stdout에 출력한다. 제품 런타임이 읽는 새 설정 파일이나 Resource manifest가 아니다.

AREA/PREFIX/DISPLAY는 대상 영역·이번 작업 stable ID 접두어·한글 테스트 이름이다.
DURATION/HOOK_DURATION/FIRE_COUNT/CENTER/RADIUS/FIRE_ASSETS는 초기 저작값이다.
key()는 미터 단위 local offset과 Y각도를 실제 quaternion xyzw로 변환한다.
template()은 현재 WORLD_SEQUENCE_TEMPLATE 스키마 전체 항목을 만든다.
instance()는 WORLD anchor의 STOP instance를 만든다.
build()는 원본 확인→중복/소유권 검사→기존불→추가불→갈고리→Gate3 패턴 순서로 블록을 만든다.
add_world()는 현재 nextWorldOrdinal에서 stable WORLD ID를 발급한다.
cue()는 occurrence ID와 유한 수명, 선택적인 occurrence placement를 발급한다.
어떤 검사가 실패해도 원본 파일은 바뀌지 않는다. 이미 동일 패턴이 있으면 새 ID로 중복 생성하지 않고 중단한다.

조사 시 예상 출력: resources3/templates5/instances5/worlds5/patterns1/occurrences58. 현재 상태라면 pattern18, 다음 ordinal19가 된다. 나중에 사용자가 다른 패턴을 추가했다면 재실행 시 현재 ordinal을 사용해야 한다. 이 문서의 숫자18을 강제로 재사용하지 않는다.

아래 G04-2는 새 Python 파일의 전체 코드다. 실행 결과 전체 JSON의 worldSequenceAdditions.objectResources/templates/instances를 대응 배열 맨 끝에 추가하고, compositionAdditions.worlds/patterns를 대응 배열 맨 끝에 추가한다. rootUpdates는 해당 루트 필드만 변경한다. 전체 원본 파일을 재정렬하거나 기존 배열을 새 배열로 교체하지 않는다.

실행 예:
python Tools/KoukuSaydonPipeline/author_gate3_fire_hook_fragments.py --repository-root .
python은 이 PC에서 실제 사용 가능한 인터프리터로 지정한다. 조사에 사용한 인터프리터는 C:/Users/USER/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe다.

구현자는 출력 JSON을 확인한 후 apply_patch로 저작본의 정확한 배열에 추가한다. 출력 최상위 schema=lostark.plan-fragments-only 문서 전체를 worldsequences.json에 덮어쓰면 안 된다. 출력은 삽입 블록 묶음이지 런타임 문서가 아니다.

## G04-2. 새 저작 도구 전체 코드

아래 부록 A에 전체 코드를 수록했다. 파일로 그대로 옮긴 뒤 위 명령으로 실행한다.

# G05. 기존 배치의 종료 복원 — 코드 추가 없이 기존 수명 계약 사용

파일:
C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/Level_KakulSaydonArena.cpp
C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/WorldSequencePlayer.cpp

실제 호출 흐름은 Consume_OwnedWorldCue → Play(...,play.iDurationMs,placement) → Update의 owned cue Seek → CWorldSequencePlayer::Update(0,targets)다.

중요한 기존 분기: CWorldSequencePlayer::Update는 Apply_Instance 결과가 FAILED이거나 active.durationMs != 0이면 Stop_Instance(id,targets,true)를 호출한다. Stop_Instance는 placementBaselines를 복원하고 OBJECT_RESOURCE 객체를 해제한 다음 active instance를 지운다. Level은 Has_ActiveInstances()==false를 확인하고 cue owner를 제거한다.

이 계획의 기존불 WORLD occurrence는 durationMs=18000이고 갈고리도8000이므로 모두 이 유한 수명 분기를 사용한다. 별도의 ID 하드코딩 복원 조건, 새 cleanup 함수, Level CPP 수정은 필요 없다. 끝 key를 프레임이 건너뛰어도 유한 duration의 기존 Stop_Instance(...,true)가 baseline을 되돌린다.

주의: durationMs=0인 무제한 재생의 FINISHED는 m_Held에 마지막 pose를 보존할 수 있다. 이를 위 유한 duration WORLD 재생과 혼동하지 않는다. F1 단독 instance 미리보기는 이 경로와 수명 옵션이 다를 수 있으므로 그쪽에서는 Stop을 명시적으로 눌러 복원한다.

STOP_OWNER/새 run epoch/레벨 종료의 기존 Stop_All(targets,true)를 그대로 사용한다. 같은 기존불28개를 두 owner가 동시에 조작하지 않도록 동시 중첩 재생은 기존 ownership 검사를 따른다. Replay는 기존 owner를 취소하고 새 epoch로 시작해야 한다.

최종 구현 범위에 새 H/CPP, 기존 H/CPP 수정, Shared packet 변경은 없다. 따라서 존재하지 않는 변경 후 C++ 전문이나 프로젝트 등록을 억지로 추가하지 않는다. 수정은 저작 JSON 두 파일과 필요한 생성물이며, 신규 Python의 생략 없는 전체 코드는 부록 A다.

# G06. 저장·publish·빌드·사용자 확인 순서

## G06-1. 적용 전 보존

AGENTS.md/CLAUDE.md/현행 팀 문서를 다시 읽는다. git status --short와 현재 branch를 확인한다. main에 직접 작업하지 않는다. 다른 세션의 수정, 카메라 컷신, 마리오/카드미로/빙고 데이터를 되돌리지 않는다. Client나 맵툴이 편집 중이면 사용자가 저장·종료한 뒤 최종 파일 병합을 한다. 과거 대화에서 종료했다는 사실로 지금도 종료됐다고 추정하지 않는다.

이번 계획에는 런타임 자율 실행/클릭/스크린샷 권한이 없다. 에이전트는 구조 검사와 빌드까지만 하고 실제 화면 판정은 사용자가 한다.

## G06-2. 데이터 생성과 검사

1. G04 Python을 실행해 현재 원본에 맞는 블록을 얻는다. duplicate/placement count 오류를 무시하지 않는다.
2. 저작 worldsequences와 composition의 두 파일에만 블록을 병합한다. mapplacements와 model binary는 바꾸지 않는다.
3. G05의 기존 유한 duration 종료 경로를 확인하고, 생성 occurrence의 durationMs를0으로 바꾸지 않는다.
4. 기존 Map publisher로 world sequence의 binding/모델/시각/key 계약을 검사한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Validate
python Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode validate
```

두 번째 명령의 validate가 기존 생성 문서와 새 저작본의 차이를 보고하면, 데이터 오류와 '아직 publish하지 않은 차이'를 구분한다. generated 문서를 손으로 고쳐 통과시키지 않는다. 생성자는 기존 publisher다.

5. worldsequence를 publish한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish
python Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode publish
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
```

현재 KoukuSaydon 도메인 소유자는 koukusaydon.product와 gameplay.balance를 발행한다. 앞선 직접 projector publish와 도메인 내부 호출은 기존 구조를 따르며 새 publisher를 만들지 않는다. world.gameplay는 별도 명령으로 갱신한다. 실제 Server bootstrap 갱신과 재시작 전에는 서버 반영 완료라고 하지 않는다.

6. 본 계획대로 저작 데이터만 적용하면 C++ 소스 변경이 없으므로 이 기능 때문에 새 컴파일은 필수가 아니다. 현재 EXE가 기존 WORLD 경로를 지원하는 최신 빌드인지 확인한다. 다른 미빌드 변경이나 EXE 갱신이 필요할 때만 기존 Debug|x64 빌드 경로를 사용한다. 새로운 protocol version이나 광역 harness를 만들지 않는다. 저장소 정본 runner를 이용할 경우:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
git diff --check
```

빌드가 JSON publish를 자동으로 모두 처리한다고 가정하지 않는다. source 수정/publish/Server 재시작/Client 시작은 서로 다른 단계다.

## G06-3. 사용자 런타임 확인 경로

1. LAN 설정 스크립트의 role이 client면 공유 서버 담당자에게 갱신된 Server 데이터 적용과 재시작을 요청한다. 이 PC 조사 시 role=client, 192.168.0.14:7777 not-listening이었다. 연결이 안 된다고 임의로127.0.0.1로 바꾸지 않는다.
2. 사용자가 현재 설정에 맞는 Client 프로젝트 또는 server-host의 Server+Client profile을 Ctrl+F5로 시작한다.
3. Lobby → KoukuSaydon → F1 Developer Tools → KoukuSaydon Arena의 3관문 선택.
4. F1 Tools → Open Action Workbench → Patterns by Gate의 Gate에서 GATE3(3관문)를 선택한다. Model View는 All로 두어 필터 때문에 목록이 숨지 않게 한다.
5. 한글 이름 '3관문_외곽불회전_갈고리대각선_시각테스트'를 선택하고 'Complete Play (Server)'를 누른다. 버튼이 비활성이면 패턴 PRODUCT 여부, 미저장 변경, publish 진행 여부를 먼저 확인한다. Local preview만 성공한 것으로 런타임 완료라 하지 않는다.
6. F1 창을 닫고18초를 본다. 기존 불28개가 제자리에서 돌고 추가 불48개가 외곽을 채워야 한다. 갈고리는3개 평행 경로를 따라9개가 시간차로 지나가야 한다.
7. 종료하면 추가 불/갈고리는 없어지고 기존 불의 position/quaternion/scale/visibility는 재생 전 값으로 돌아와야 한다. 같은 패턴을3회 재생해 개수 누적이 없어야 한다.
8. 패턴 중간 Stop, 다른 패턴 선택, 관문 재선택, 맵 퇴장도 각각 확인한다. 기존 불이 회전 중간각으로 남거나 다른 맵에 갈고리가 남으면 미완료다.

World Object Tool의 individual Play는 각 새 child motion의 재질·모양·움직임을 분리 확인하는 용도다. 플레이어 앞 미리보기가 켜져 있으면 저장된 경기장 좌표와 다른 위치에서 보일 수 있다. 전체 원형 배치와9개의 갈고리 타이밍은 Composition 패턴 재생으로 확인한다. 새 패턴은 F1 Sequence Viewer의 단일 instance Play와 같지 않다. 별도 UI나 자동 묶음 메뉴를 추가했다고 보고하지 않는다.

# G07. 검증과 문제별 조치

구현자가 기록할 최소 수치 검증:

- fire existing binding28, 추가 resource3, template5, instance5, WORLD 정의5, occurrence58.
- 추가 WORLD 중 불48개는 시작0/끝18000, 갈고리9개는 시작500~5900/각 수명8000. 마지막13900ms에 모든 갈고리 종료.
- 각 기존 불 key quaternion 길이≈1, 시간은 엄격 증가, 처음0ms/끝18000ms, 위치 offset0·scale1 유지.
- 세 경로의 직진 구간 길이는48m, 직진 시간7초, 속도약6.857m/s. 경로 간 수직 거리4m.
- 부모 hook의 기존 defaultMotion, 기존 네이티브 패턴, 기존 card/mario sequence는 변경 전과 동일.
- 수정 JSON parse, 해당 publisher 성공, git diff --check 결과를 실제 로그로 기록. C++ 변경이 없으면 컴파일은 변경 없음/미실행으로 기록하고, EXE 갱신 때문에 실행했다면 실제 결과를 기록.
- '시각 테스트'이므로 HP·잡힘·사망 판정은 연결하지 않았음을 결과에 명시.

화면 문제와 우선 점검:

| 증상 | 먼저 확인할 것 |
|---|---|
| 불이100배 큼 | modelPreScale=0.01 누락, scale 중복 |
| 불이 안 돎 |0도/360도 두 key만 사용했는지, 새 world instance와 기존 placement binding이 맞는지 |
| 불이 튀거나 제멋대로 흔들림 | 같은 placement가 mapmotions와 world sequence에서 동시에 갱신되는지 |
| 불이 중앙을 가림 | RADIUS와 해당 모델 local width/높이, occurrence scale. 카메라를 바꿔 감추지 말 것 |
| 전체 배경/바닥이 회전 | 불28개가 아닌 부모/바닥 placement를 선택한 오류 |
| 일부 판이 얇아짐 | 회전한 평면을 옆에서 보는 기하학적 결과와 백페이스/재질 오류를 구분 |
| 불이 회색·검은 사각형 | 실제 CMaterial diffuse 경로/파일, 두꺼운불로 교체하기 전 기존 모델과 비교 |
| 갈고리가 두 배 빠름 | native 이동 clip과 Transform 이동 중복 또는 placement scale/재생속도 중복 |
| 갈고리가 다른 방향으로 이동 | 모델이 아니라 local+Z 경로에 적용되는 occurrence yaw45와 월드 시작점 확인 |
| 갈고리 머리·사슬이 바닥에서 어긋남 | 전체 bounds와 머리 높이를 혼동했는지. root Y=1.2와 원본 scale1.15 확인 |
| 카메라를 움직이면 경로가 변함 | 카메라 축을 사용하는 잘못된 경로 구현. 월드 S/E로 고정 |
| 단독 미리보기만 보이고 패턴은 안 보임 | composition projection, WORLD 참조, Server domain publish/재시작, pinned revision |
| 종료 후 기존불 각도가 남음 | WORLD durationMs가18000인지, 기존 Stop_Instance(...,true) 분기가 실행되는지 |
| 원본과 속도/방향이 다름 | 초기 튜닝값임을 확인하고 변경. 영상2배 표시를 원본 속도라고 오해하지 말 것 |

최초 테스트 후 사용자에게 '불은 제자리에서 돌아야 하는지, 원형 경계를 따라 위치도 돌아야 하는지'를 묻고 눈으로 맞춘다. 공전이 필요하다고 확인되면 추가 불 단일 OBJECT_RESOURCE motion의 revolution을 활용하되 현재 center/pivot/offset 식을 읽고 수정한다. 기존28 MAP_PLACEMENT는 objectMotion의 revolution이 적용되는 종류가 아니므로 그 필드만 켜서는 움직이지 않는다. 본 초안의 모든 위치는 제자리 회전 기준이다.

# G08. 리소스 배포·인계·완료 보고

이 계획의 기본 구현은 설치된 WModel과 텍스처를 재사용한다. 신규 mesh/texture 추출·cook은 필요하지 않다. 파일이 실제 누락된 팀원 PC에는 아래 폴더를 팀장이 Drive로 전달한다.

- Resources/Character/KoukuSaton/MN_UMAX_00/ 전체(모델+textures).
- Resources/Map/LV_LUT_MIDNIGHTC_ED/MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB/ 전체.
- Resources/Map/LV_LUT_MIDNIGHTC_ED/MAP_B71A2EC9D778_BG_RAD_KOUKUSATON_DECO24E_SM_KHB/ 전체.
- Resources/Map/LV_LUT_MIDNIGHTC_ED/MAP_7AC8BB3D2FEE_BG_RAD_KOUKUSATON_DECO24F_SM_KHB/ 전체.
- 기존28개가 참조하는 다른 DECO24 변형은 이미 맵 실행에 필요하므로 기존 맵 Resource 배포에 포함되어 있어야 한다.

Resources 경로의 실제 물리 루트는 C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources다. 이 파일들은 Git에 force-add하지 않는다. 새 manifest/lock/hash 배포 체계를 만들지 않는다. 이번 계획 작성 중에는 Resource 파일을 바꾸거나 Drive에 전송하지 않았다.

Claude의 최종 보고는 네 부분으로 분리한다: 적용한 파일/데이터, 실제 통과한 자동 검사, 사용자가 확인해야 하는 화면, 의도적으로 범위 밖인 잡힘·피해·QTE. 단순 JSON 생성이나 코드 작성만으로 원본처럼 복원 완료라고 하지 않는다.

계획 작성 단계에서 실제 수행한 검증: 영상0~5초 열람, 리소스/배치/클립 읽기, 데이터 생성 코드 실행, 생성된 후보를 메모리에서 기존 Composition validator와 publishable join에 대입하여 통과. 제품 source/runtime publish, Client 빌드·실행, 실제3관문 화면 확인은 하지 않았다. 본문 초기속도/밀도/축은 사용자 visual 승인 전이다.

# 부록 A. author_gate3_fire_hook_fragments.py 전체 코드

이 아래에는 생략 없는 코드가 이어진다.

```python
"""Read-only authoring fragment generator. Prints JSON; never edits game data."""
from __future__ import annotations
import argparse
import copy
import json
import math
from pathlib import Path
import shlex
import sys

AREA = "LV_LUT_MIDNIGHTC_ED"
PREFIX = "world.sequence.instance.kouku.g3.fire_hook"
DISPLAY = "3관문_외곽불회전_갈고리대각선_시각테스트"
DURATION = 18000
HOOK_DURATION = 8000
FIRE_COUNT = 48
CENTER = (0.0, 942.080017)
RADIUS = 19.5
FIRE_ASSETS = (
    "MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB",
    "MAP_B71A2EC9D778_BG_RAD_KOUKUSATON_DECO24E_SM_KHB",
    "MAP_7AC8BB3D2FEE_BG_RAD_KOUKUSATON_DECO24F_SM_KHB",
)

def read_json(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))

def key(ms, position=(0, 0, 0), yaw=0.0, visible=True):
    a = math.radians(yaw) * 0.5
    return {"timeMs": ms, "positionOffset": list(position),
            "rotationQuaternion": [0, math.sin(a), 0, math.cos(a)],
            "scaleMultiplier": [1, 1, 1], "visible": visible}

def template(sid, name, duration, tracks, angular_y=0.0, animations=None):
    return {"sequenceId": sid, "displayName": name, "category": "KoukuGate3",
            "durationMs": duration, "interpolation": "LINEAR", "tracks": tracks,
            "animationTracks": animations or [], "effectTracks": [],
            "objectMotion": {"velocity": [0, 0, 0], "acceleration": [0, 0, 0],
                             "angularVelocityDegrees": [0, angular_y, 0],
                             "revolutionDegreesPerSecond": [0, 0, 0],
                             "revolutionOffset": [0, 0, 0], "count": 1,
                             "intervalMs": 0, "spreadDegrees": 0, "seed": 1}}

def instance(iid, tid, bindings):
    return {"instanceId": iid, "templateId": tid, "enabled": True,
            "startDelayMs": 0, "playbackSpeed": 1, "bindings": bindings,
            "anchorKind": "WORLD", "position": [0, 0, 0],
            "motionEnd": "STOP", "nextMotionId": ""}

def binding(slot, kind, target):
    return {"slotId": slot, "targetKind": kind, "targetId": target}

def build(root: Path):
    ws_path = root / f"Data/Maps/Authoring/{AREA}/{AREA}.worldsequences.json"
    cp_path = root / "Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json"
    ws, cp = read_json(ws_path), read_json(cp_path)
    catalog_path = root / f"Data/Maps/Imported/{AREA}/{AREA}.mapassets"
    catalog = {r[0]: r for r in
               (shlex.split(line) for line in catalog_path.read_text(encoding="utf-8-sig").splitlines()[1:])}
    placement_path = root / f"Data/Maps/Authoring/{AREA}/{AREA}.mapplacements"
    placements = [shlex.split(line) for line in placement_path.read_text(encoding="utf-8-sig").splitlines()[1:]]
    existing = [r for r in placements if "DECO24" in r[4]
                and r[1].startswith(f"{AREA}_SL05:export:")
                and -40 < float(r[5]) < 40 and 910 < float(r[7]) < 970]
    if len(existing) != 28:
        raise ValueError(f"Expected the audited 28 SL05 fire placements, found {len(existing)}. Re-audit before applying.")
    existing.sort(key=lambda r: int(r[0]))
    if any(p.get("displayName") == DISPLAY for p in cp["patterns"]):
        raise ValueError("The plan pattern already exists. Tune its saved IDs; do not generate a duplicate.")
    if any(i["instanceId"].startswith(PREFIX) for i in ws["instances"]):
        raise ValueError("Plan sequence IDs already exist; inspect and merge by ID.")
    motion_path = root / f"Data/Maps/Authoring/{AREA}/{AREA}.mapmotions.json"
    motions = read_json(motion_path).get("motions", []) if motion_path.is_file() else []
    original_ids = {r[0] for r in existing}
    if original_ids.intersection(str(m["placementId"]) for m in motions):
        raise ValueError("An existing fire already has a self-motion owner; do not drive it twice.")
    for i in ws["instances"]:
        if any(b["targetKind"] == "MAP_PLACEMENT" and b["targetId"] in original_ids for b in i["bindings"]):
            raise ValueError(f"Fire placement already bound by {i['instanceId']}; ownership review required.")
    resources, templates, instances, worlds, cues = [], [], [], [], []
    pattern_number = cp["nextPatternOrdinal"]
    pid = f"KAKULSAYDON_G1_PATTERN_{pattern_number}"
    world_number = cp["nextWorldOrdinal"]

    def add_world(name, iid, object_id=None):
        nonlocal world_number
        wid = f"kakulsaydon.g1.world.{world_number}"
        world_number += 1
        row = {"worldId": wid, "displayName": name, "sequenceInstanceId": iid,
               "positionOffset": [0, 0, 0], "anchorKind": "NONE",
               "anchorPosition": [0, 0, 0], "companionEffectResourceId": ""}
        if object_id:
            row["objectResourceId"] = object_id
        worlds.append(row)
        return wid

    def cue(wid, start, duration, placement=None):
        row = {"occurrenceId": f"{pid}.world.{len(cues) + 1}", "worldId": wid,
               "startMs": start, "durationMs": duration, "playbackSpeed": 1}
        if placement is not None:
            row["placement"] = placement
        cues.append(row)

    iid = PREFIX + ".existing_fire"
    tid = "sequence.kouku.g3.fire_hook.existing_fire"
    tracks = []
    binds = []
    for index, row in enumerate(existing):
        slot = f"fire.{index + 1}"
        # 90-degree intervals avoid quaternion shortest-path full-turn collapse.
        tracks.append({"slotId": slot,
                       "keys": [key(t, yaw=t * 0.06) for t in range(0, DURATION + 1, 1500)]})
        binds.append(binding(slot, "MAP_PLACEMENT", row[0]))
    templates.append(template(tid, "3관문_기존외곽불28개_제자리회전", DURATION, tracks))
    instances.append(instance(iid, tid, binds))
    cue(add_world("3관문_기존외곽불_28개", iid), 0, DURATION)

    fire_worlds = []
    for index, asset in enumerate(FIRE_ASSETS):
        model_id = catalog[asset][2]
        if not (root / "Client/Bin/Resources" / model_id).is_file():
            raise ValueError(f"Missing physical fire model: {model_id}")
        suffix = ("d", "e", "f")[index]
        object_id = f"world.object.kouku.g3.outer_fire.{suffix}"
        iid = PREFIX + f".extra_fire_{suffix}"
        tid = f"sequence.kouku.g3.fire_hook.extra_fire_{suffix}"
        resources.append({"objectId": object_id, "displayName": f"3관문_외곽불_{suffix.upper()}",
                          "modelAssetId": model_id, "anchorKind": "WORLD",
                          "diffuseTextureAssetId": "", "modelPreScale": 0.01,
                          "animated": False, "scale": [1, 1, 1],
                          "sequenceInstanceId": "", "defaultMotionInstanceId": iid})
        templates.append(template(tid, f"3관문_추가불_{suffix.upper()}_제자리회전", DURATION,
                                  [{"slotId": "object", "keys": [key(0), key(DURATION)]}],
                                  angular_y=(60 if index != 1 else -60)))
        instances.append(instance(iid, tid, [binding("object", "OBJECT_RESOURCE", object_id)]))
        fire_worlds.append(add_world(f"3관문_추가외곽불_{suffix.upper()}", iid, object_id))
    for i in range(FIRE_COUNT):
        angle = 2 * math.pi * i / FIRE_COUNT
        x, z = CENTER[0] + RADIUS * math.cos(angle), CENTER[1] + RADIUS * math.sin(angle)
        # Local face normal is X. Face toward the centre before local Y rotation.
        yaw = -math.degrees(angle) + 180
        scale = (1.2, 1.5, 1.2) if i % 3 != 2 else (0.7, 2.5, 0.7)
        cue(fire_worlds[i % 3], 0, DURATION,
            {"position": [x, 0.05, z], "rotationDegrees": [0, yaw, 0], "scale": list(scale)})

    hook = next(r for r in ws["objectResources"] if r["objectId"] == "world.object.kouku.hook")
    if not (root / "Client/Bin/Resources" / hook["modelAssetId"]).is_file():
        raise ValueError("Installed hook model is missing.")
    iid, tid = PREFIX + ".hook_diagonal", "sequence.kouku.g3.fire_hook.hook_diagonal"
    keys = [key(0, (0, 3, 0)), key(400), key(7400, (0, 0, 48)),
            key(7999, (0, 3, 48)), key(8000, (0, 3, 48), visible=False)]
    animation = {"slotId": "object", "startMs": 0, "clipName": "Hook_idle_normal_1",
                 "playbackRate": 1, "loop": True, "holdLastFrame": True,
                 "displayName": "갈고리_매달린자세_경로이동은Transform만"}
    templates.append(template(tid, "3관문_갈고리_등장대각선통과퇴장", HOOK_DURATION,
                              [{"slotId": "object", "keys": keys}], animations=[animation]))
    instances.append(instance(iid, tid, [binding("object", "OBJECT_RESOURCE", hook["objectId"])]))
    hook_world = add_world("3관문_갈고리_대각선", iid, hook["objectId"])
    unit = math.sqrt(0.5)
    for wave in range(3):
        for lane in range(3):
            lateral = (lane - 1) * 4.0
            start = [CENTER[0] - 24 * unit + lateral * unit, 1.2,
                     CENTER[1] - 24 * unit - lateral * unit]
            cue(hook_world, 500 + wave * 2400 + lane * 300, HOOK_DURATION,
                {"position": start, "rotationDegrees": [0, 45, 0], "scale": [1, 1, 1]})

    reference = read_json(root / "Data/Animation/Reference/KoukuSaydon/MN_RPCT_07.actionreference.json")
    action = next(a for a in reference["actions"] if a["sourceActionId"] == 0)
    source_stage = next(s for s in action["stages"] if s["stageId"] == "stage-003")
    source_slot = next(s for s in source_stage["slots"] if s["slotId"] == "animation-000")
    if source_slot["runtimeClip"] != "rpct00_idle_normal_1" or source_slot["playMs"] != 3000:
        raise ValueError("The audited Gate 3 idle reference changed; re-audit.")
    stages = []
    for number in range(1, 7):
        stages.append({"stageId": f"STAGE_{number}", "actionId": f"{pid}.stage.{number}",
                       "stageKind": "ACTIVE", "durationMs": 3000,
                       "animationOccurrences": [{"occurrenceId": f"{pid}.animation.{number}",
                            "profileId": "MN_RPCT_07", "sourceActionId": 0,
                            "sourceStageId": "stage-003", "sourceSlotId": "animation-000",
                            "referenceRevision": reference["referenceRevision"],
                            "runtimeClip": source_slot["runtimeClip"], "startOffsetMs": 0,
                            "sourceStartMs": 0, "playMs": 3000, "playRate": 1, "endPolicy": "EXACT"}]})
    pattern = {"patternId": pid, "actorProfileId": "MN_RPCT_05", "gateId": "GATE3",
               "targetBossPlacementId": "boss.kakulsaydon.g3.saydon", "displayName": DISPLAY,
               "authoringStatus": "PRODUCT", "category": "MECHANIC", "nextStageOrdinal": 7,
               "nextAnimationOrdinal": 7, "nextLogicOccurrenceOrdinal": 1,
               "nextSummonOccurrenceOrdinal": 1, "nextWorldOccurrenceOrdinal": len(cues) + 1,
               "nextSceneProfileOccurrenceOrdinal": 1, "nextPresentationOccurrenceOrdinal": 1,
               "stages": stages, "logicOccurrences": [], "summonOccurrences": [],
               "worldOccurrences": cues, "sceneProfileOccurrences": [], "presentationOccurrences": [],
               "resetBossToSpawn": False}
    return {"schema": "lostark.plan-fragments-only", "notRuntimeInput": True,
            "sourceRevisions": {"worldSequences": ws["revision"], "composition": cp["revision"]},
            "worldSequenceAdditions": {"objectResources": resources, "templates": templates, "instances": instances},
            "compositionAdditions": {"worlds": worlds, "patterns": [pattern]},
            "rootUpdates": {"worldSequenceRevision": ws["revision"] + 1,
                            "compositionRevision": cp["revision"] + 1,
                            "nextPatternOrdinal": pattern_number + 1, "nextWorldOrdinal": world_number,
                            "playAllPatternIds": cp["playAllPatternIds"] + [pid]},
            "existingFirePlacements": [{"placementId": r[0], "source": r[1], "assetId": r[4],
                                        "position": [float(x) for x in r[5:8]]} for r in existing]}

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-root", type=Path, required=True)
    args = parser.parse_args()
    print(json.dumps(build(args.repository_root.resolve()), ensure_ascii=False, separators=(",", ":"), allow_nan=False))
```


# G09. 2026-09-10 첨부 영상의 외곽 불 색·면 방향 수정

이 절은 위의 초기 불 방향 설명을 대체한다. 갈고리 및 패턴 19는 수정하지 않는다.

실제 설치된 D/E/F WMSH는 모두 X 두께 0.01625m, 넓은 축 Z다. D/E 40개의 yaw에서 90도를 빼고 D/E CW/CCW 4개 motion의 revolutionOffset을 [0,0,12.6]에서 [12.6,0,0]으로 동시에 바꾼다. 회전축 중심, 반경12.6, 속도±24도/초, 수60, 크기와 현재 바닥 높이는 유지한다. Composition revision231→232, sequence revision426→427.

세 WModel의 WMA2 slot0에는 실제 diffuse가 아니라 t_tds_specular04가 emissivePath에 들어 있다. 텍스처 픽셀은 노랑/보라 반사광이며 현재 PS_MAIN은 그 RGB를 발광 버퍼에 더한다. 아래 좁은 수리 도구는 이 520바이트 경로 필드만 비운다. 원본 게임 파일이나 DDS, geometry/UV/다른 material slot을 수정하지 않는다. 이는 관찰된 오접속 수정이며 원본 material equation 전체 복원 주장도, visual PASS도 아니다.

적용 파일: Tools/KoukuSaydonPipeline/repair_gate3_fire_materials.py 신규, 기존 author_gate3_fire_hook_fragments.py의 FIRE_ASSETS 축·측정 주석·FIRE_ORIGIN_Y 동기화, 저장 Composition40 yaw와 WorldSequence4 offset. 새 C++/project/filter 등록 없음.

## G09-1. 신규 수리 도구 전체 코드

```python
#!/usr/bin/env python3
"""Remove the observed reflection-lookup-as-emission error from three fire assets.

This is a narrowly scoped repair of installed WMA2 metadata, not an original
material reconstruction. Geometry, UVs, diffuse, normal and specular are preserved.
Run without --apply to inspect. Originals are retained under out before writes.
"""
import argparse
import hashlib
from pathlib import Path
import struct

ASSETS = {
    'MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB':
        ('SLOT_000_bg_rad_koukusaton_deco24_mi_khb',
         'textures/79e70292f4fc_bg_rad_koukusaton_deco24_d_khb.dds'),
    'MAP_B71A2EC9D778_BG_RAD_KOUKUSATON_DECO24E_SM_KHB':
        ('SLOT_000_bg_rad_koukusaton_deco24a_mi_khb',
         'textures/cd58264db472_bg_rad_koukusaton_deco24a_d_khb.dds'),
    'MAP_7AC8BB3D2FEE_BG_RAD_KOUKUSATON_DECO24F_SM_KHB':
        ('SLOT_000_bg_rad_koukusaton_deco24a_mi_khb',
         'textures/cd58264db472_bg_rad_koukusaton_deco24a_d_khb.dds'),
}
BAD_EMISSION = 'textures/b2378a8f80d6_t_tds_specular04.dds'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def corrected(data, material_name, diffuse):
    require(len(data) > 176, 'Truncated WModel')
    require(struct.unpack_from('<4sHHII', data) ==
            (b'WINT', 1, 0, 0, len(data) - 16), 'Unexpected WModel header')
    require(struct.unpack_from('<4sIII4I', data, 16) ==
            (b'WMOD', 2, 0, 0, 0, 0, 0, 0), 'Unexpected model section count')
    sections = [struct.unpack_from('<IIQQ40s', data, 48 + i * 64) for i in range(2)]
    require(sorted(s[0] for s in sections) == [1, 2], 'Unexpected sections')
    section = next(s for s in sections if s[0] == 2)
    start, size = 16 + section[2], section[3]
    require(start + size == len(data), 'Material section is not last')
    require(struct.unpack_from('<4sHHII', data, start) ==
            (b'WINT', 1, 0, 0, size - 16), 'Unexpected material header')
    require(struct.unpack_from('<4sI', data, start + 16) == (b'WMA2', 2),
            'Expected two WMA2 material records')
    require(size == 24 + 2 * (76 + 9 * 520), 'Unexpected WMA2 layout')
    record = start + 24
    require(struct.unpack_from('<I', data, record)[0] == 0, 'Wrong material index')
    require(data[record + 12:record + 76].split(b'\0')[0].decode() == material_name,
            'Material identity differs')
    paths = record + 76
    decode_path = lambda offset: data[offset:offset + 520].decode('utf-16-le').split('\0')[0]
    require(decode_path(paths) == diffuse, 'Diffuse identity differs')
    emission_offset = paths + 3 * 520
    emission = decode_path(emission_offset)
    require(emission in ('', BAD_EMISSION), 'Unexpected emissive path; inspect instead of overwriting')
    result = data[:emission_offset] + bytes(520) + data[emission_offset + 520:]
    require(len(result) == len(data), 'Container size changed')
    return result, emission


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository-root', type=Path, required=True)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    root = args.repository_root.resolve(strict=True)
    resource_root = root / 'Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED'
    staged = []
    for asset, (material, diffuse) in ASSETS.items():
        path = resource_root / asset / (asset + '.wmodel')
        require(path.resolve().is_relative_to(resource_root.resolve()), 'Resource escaped root')
        original = path.read_bytes()
        replacement, emission = corrected(original, material, diffuse)
        staged.append((path, original, replacement))
        print(f'{asset}: emissive={emission or "<empty>"}; '
              f'{"repair" if original != replacement else "already repaired"}')
    if not args.apply:
        return
    backup_root = root / 'out/KoukuGate3FireMaterialBackup'
    backup_root.mkdir(parents=True, exist_ok=True)
    # Stage all originals before touching any resource. A concurrent edit aborts.
    for path, original, replacement in staged:
        require(path.read_bytes() == original, f'Concurrent resource edit: {path}')
        if original != replacement:
            backup = backup_root / (path.stem + '.' + hashlib.sha256(original).hexdigest() + '.wmodel')
            if backup.exists():
                require(backup.read_bytes() == original, 'Existing backup differs')
            else:
                backup.write_bytes(original)
    written = []
    try:
        for path, original, replacement in staged:
            require(path.read_bytes() == original, f'Concurrent resource edit: {path}')
            if original == replacement:
                continue
            written.append((path, original))
            path.write_bytes(replacement)
            require(path.read_bytes() == replacement, f'Readback failed: {path}')
    except BaseException:
        for path, original in reversed(written):
            path.write_bytes(original)
        raise
    print(f'Applied {len(written)} material repairs; backups: {backup_root}')


if __name__ == '__main__':
    main()
```

## G09-2. 검증 및 사용자 확인

모든 입력의 identity 검사 후 수리하며 원본은 out/KoukuGate3FireMaterialBackup에 보존한다. 수리 재실행은0개 변경이어야 한다. 데이터는 기존 KoukuSaydon publisher로 배포하고 JSON 의미 diff에서 갈고리/다른 패턴/모션 불변을 검사한다. C++/셰이더 변경이 없어 추가 컴파일은 필요하지 않다. Client 자율 실행·캡처는 하지 않는다. 사용자가 Client/Server 재시작 후 3관문_외곽불회전_갈고리대각선_시각테스트를 재생해 붉고 노란 불벽과 공전 모양을 판단한다.

# G10. 2026-09-10 — 불꽃을 앞뒤 간격이 있는 세 줄로 재배치

사용자 새 영상 3관문 불불.mp4에서 수정된 재질의 느낌은 맞지만 한 줄처럼 보인다는 피드백을 받았다.
원본 아재패턴.mp4와 비교하여 넓은 면을 유지하되 반경이 다른 세 줄로 분리한다.
이번 숫자는 원본 데이터 추출값이 아닌 영상 피드백에 맞춘 프로젝트 저작값이다.

불60개를 D/E/F 각각20개로 유지한다. D는 반경12.6m/시작0도, E는11.7m/6도,
F는10.8m/12도다. 줄 간 반경 간격0.9m, 각 줄 시작시 불꽃 간18도다.
종류별20개를 배치 순서대로 번호0~19에 대응한다.
position=(radius*cos(angle), 기존Y, 942.08+radius*sin(angle)), yaw=-angle.
E/F CW/CCW 네 motion의 revolutionOffset만 해당 반경으로 같이 갱신한다.
최외곽을 확장하지 않으며 불의 수·재질·scale·height·수명·회전속도±24도/초·갈고리는 보존한다.

author_gate3_fire_hook_fragments.py의 RADIUS 선언 바로 아래 추가할 정본:

```python
FIRE_RING_LAYOUT = {"d": (12.6, 0.0), "e": (11.7, 6.0), "f": (10.8, 12.0)}
```

FIRE_ASSETS 순회에서 offset 계산:

```python
radius = FIRE_RING_LAYOUT[suffix][0]
offset = (radius, 0, 0) if outward == "x" else (0, 0, radius)
```

기존 for i in range(FIRE_COUNT) 루프의 theta/x/z 계산을 아래로 교체:

```python
world_index = i % len(fire_worlds)
suffix = FIRE_ASSETS[world_index // 2][1]
radius, phase = FIRE_RING_LAYOUT[suffix]
slot_in_row = (i // len(fire_worlds)) * 2 + world_index % 2
degrees = slot_in_row * (360.0 / (FIRE_COUNT // len(FIRE_ASSETS))) + phase
theta = math.radians(degrees)
x = CENTER[0] + radius * math.cos(theta)
z = CENTER[1] + radius * math.sin(theta)
```

저장 패턴18의 fire occurrence position.x/z와 rotationDegrees.y만 위 식으로 갱신한다.
Composition revision232→233, WorldSequences427→428. 다른 패턴 및 갈고리 의미 diff0,
60개×721시점에서 세 반경 유지와 법선 방사방향 일치를 검사한 뒤 KoukuSaydon owner publisher로 배포한다.
C++/Shader/Resources 변경 없음. 실행 화면 판정은 사용자가 같은 패턴을 재생해서 한다.
