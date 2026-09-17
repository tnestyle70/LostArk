# 세이튼 서커스 공 World / V1 통합 구현 계획

## G00. 원작과 요청값

원작 Action 4219806의 Projectile 421980602~421980607은 6세대이며, 앞의
5세대마다 두 end-child callback을 갖는다. 최신 사용자 정정에 따라 5회 분열,
1→2→4→8→16→32, 총 63공을 사용한다. 원작 random 360도 대신 부모 진행 방향에
90도를 더한 축의 ±45도는 USER_AUTHORED로 기록한다. 원작 최대 수명 1.5초와
세대별 scale/range는 그대로 사용하고, source bounce 곡선의 첫 bounce만 선택하여
그 수명에 맞추는 것은 요청에 따른 PROJECT_AUTHORED 시간 재배치다.

## G01. 파일과 소비자

`Tools/EffectPipeline/build_saydon_circus_world.py`가 원작 projectile contract,
설치 V1 leaf와 Mario StripedBall World 정의를 읽어 `out/SaydonCircusWorld20260917`
후보를 생성한다. WorldSequence의 기존 objectMotion.emissions와 model-less group의
motionInstanceIds를 사용한다. NEXT는 단일 공만 허용하는 기존 계약을 유지한다.
CModel과 기존 CMaterial을 재사용하며 피해 판정과 네트워크 형식은 추가하지 않는다.

World timeline 하나의 그룹이 여섯 STOP motion을 재생한다. 각 motion의 emission
birth delay가 0/1500/3000/4500/6000/7500ms이고 playback speed를 함께 소비한다.
각 세대 공의 종점이
다음 세대 두 공의 원점이다. effectTracks는 공 위치를 따르는 상단광과 지면에 남는
충격을 같은 clock에서 재생하고, 끝난 공의 잔여 FX 수명은 기존 PresentationSpan이
보존한다. 원작 VividFracture Ball_04/Exp_02~04와 Showtime 총구 leaf를 V1 후보로
묶는다. 모든 source mesh/material recipe와 source 위치 관계의 변경 경계를 기록한다.

## G02. 저장과 적용 경계

후보 WorldSequence, EffectCatalog/EffectResourceTree entries, Composition world
정의 등록 proposal만 생성한다. Live Data, 설치 DataFiles, 사용자가 편집 중인
Composition과 빈 P83 stage/timeline은 변경하지 않는다. 실제 등록은 root의 semantic
merge 단계가 담당한다. 기존 Showtime WORLD 총구는 stable resource ID로 재사용한다.

## G03. 검증

실제 CWorldSequenceDocument Load/Save/Load와 production Sample_Track,
Sample_ObjectWorld, Resolve_ObjectMotion 본문을 격리 CPU probe에서 실행한다.
63공/5분열, 부모 종점-자식 원점, 각도, 단일 bounce, scale, group/STOP/seek,
효과 trigger/tail, ground/upper anchor를 확인한다. V1은 최신 codec/playback ABI로
parse/drawable/roundtrip/stage를 확인한다. JSON parse와 git diff --check를 실행한다.
Client/UI 실행, 제품 빌드, 화면 fidelity 판정은 하지 않는다.

## G04. 단일 Composition 그룹 소비

`Client/Private/MainApp.cpp`의 World 목록과 `KoukuSaydonActionWorkbench`는 그룹
objectId를 sequenceInstanceId로 보존하고 전체 effect tail을 포함한 duration으로
하나의 occurrence를 만든다. 이 부분은 root가 담당한다. 구조화된 저장 형식의 새
필드는 없고, `bMotionGroup`은 툴 목록의 일시적인 분류다.

`Client/Private/Level_KakulSaydonArena.cpp`는 해당 ID를 enabled member ID로
해석한다. 기존 CWorldSequencePlayer 하나에 모든 member를 prepare한 뒤 play하며,
부분 실패는 그 cue의 Stop_All로 정리한다. preview seek와 Server snapshot clock은
Seek_AllToMs로 공유하고, Stop/owner 종료도 같은 cue 소유권을 유지한다.

모든 세대가 같은 출발 원점을 사용하도록 최초 성공한 source(0) matrix를 shared
optional에 보존한다. pending anchor는 실패로 반환하고 다음 sample에서 재시도한다.
부모 이후의 보스 이동이 자식 원점을 바꾸지 않는다. 여러 member가 동시에 보이는
그룹의 단일 effect pivot은 모호하므로 거절한다.

publisher의 reference/placement/encounter/collider 입력은 같은 그룹을 검증한다.
WORLD anchor, 단일 실제 model, STOP/LOOP, 최대 32 member, enabled member 하나 이상을
요구한다. 이 확장은 시각 그룹이며 member collider/combatBody/walkable은 거절한다.
Server는 한 owned cue에 stable group ID와 aggregate duration을 그대로 전달한다.
publisher/Server 집중 검증은 server_patterns가 담당한다.

새 C++ TU가 없으므로 project/filter 등록은 필요 없다. MainApp/Workbench/Level TU
집중 컴파일, 실제 World codec/production sampling과 helper fault injection, publisher
focused tests로 마감한다.

## G04. Composition에서 World 그룹 하나를 Append

NEXT의 기존 단일 emission 계약은 유지한다. 여섯 STOP motion을 기존 model-less
Object의 motionInstanceIds로 묶고, Composition의 sequenceInstanceId가 그 group
objectId를 가리키도록 기존 resolver를 확장한다. Server는 같은 한 World cue와 clock을
전달한다. Level의 기존 CWorldSequencePlayer가 member motion들을 stage/play/seek/stop한다.

MainApp 목록 수집은 그룹의 유효한 member, 전체 presentation tail, emission 합계와
placement 지원을 계산한다. Workbench는 그룹을 default playback으로 Append하고 전체
재생 길이로 box 및 필요한 Pattern lifetime을 연장한다. 개별 Motion 편집과 그룹 편집을
구별하며 실패 시 기존 draft를 보존한다. 기존 C++ 3파일(MainApp, Workbench, 그 H)만
수정하므로 새 project/filter 항목은 없다. 그룹의 목록·Append·저장과 기존 단품 동작은
집중 CPU 검증하며 전체 Client 실행은 하지 않는다.

## G05. World 모션 정의 수용량

최종 병합 문서는 기존 256개 template에 필요한 12개를 추가하여 268개다.
WorldSequence는 vector와 JSON array, stable string ID를 사용하며 고정 256칸 배열이나
8-bit template count를 전송하지 않는다. 기존 유한 상한을 512개로 늘리고 C++ 공통 상수,
Map publisher, Composition source validator, 컷신 후보 생성기의 상한을 일치시킨다.
instance 2048개, template당 track 32개와 key 256개, 문서 16 MiB는 그대로 유지한다.
World Tool과 Effect Composition resolver는 같은 C++ 상수를 사용한다.

전체 268개 문서의 실제 codec Load/Validate/Save/재Load 동등성과 기존 행 보존을 검사한다.
추가로 512개 허용, 513개 거절 및 거절 시 이전 문서 보존을 실제 codec과 publisher 함수로
검사한다. source 변경은 기존 H와 Python/PowerShell 파일에 한정하며 새 제품 TU는 없다.
