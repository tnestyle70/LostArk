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

## G06. 단일 g0 선택과 낙하 FX 원점·시계 정렬

최신 Composition1232의 P83.world.7은 동일 표시명의 내부 model donor(world32,
`world.object.kouku.saydon.circus.split.model`)를 선택하여 g0만 재생한다. 기존 group30은
six motion/63공을 이미 소유한다. MainApp의 일시 목록에서 기본 motion이 정확히 하나의
model-less group에 속한 donor에 group Append alias를 연결하고, Objects 목록에서는
실제 group 한 행만 선택하도록 한다. 개별 motion의 Logic/편집 목록은 유지한다.
Workbench Append는 해당 alias를 기존 group Append 경로로 보내 전체 수명을 사용한다.
P83의 기존 start/placement/anchor를 보존한 world30 연결과11500ms 수명, 필요한 Pattern
duration 연장만 guarded patch로 생성한다. 현재 P83의16045ms 수명은 충분하므로 그대로 둔다. 실제 clip의 시작/길이/재생속도는 보존한다.

V1 `rainbow.drop`의 현재 저장된 emitter30 위치[-.41,5.18,.66]와 공 mesh5의 위치0,
원작 LocationDirect 및0.8초 particle 수명을 실제 CPU playback으로 비교한다. 설치 모델의
pivot·scale로 공 중심과 상단을 측정하고 상단광의 상대 위치와 이동 시계를 공과 일치시킨다.
World의 `ball.upper`는 자체 DirectLocation을 뺀 기존 followObject provider를 계속 사용하며,
fitEffectToDuration에 의해 원본 짧은 particle이 motion보다 먼저 끝나는지 같이 검증한다.
원작 색·재질과 사용자 삭제한 요소·그 외 편집값은 유지한다. Engine 이동 runtime을 만들지 않는다.

변경 소유는 기존 MainApp.cpp, KoukuSaydonActionWorkbench H/CPP, builder와 필요한 WorldSequence
소비자다. 새 C++ TU는 없다. 현재 저장본 bytes/hash가 유지되는 out 후보만 만들고 root가
최종 등록한다. 실제 목록/Append 본문 집중 검사, 기존 World codec/샘플러, V1 particle 시간·위치
검사와 최소 TU compile을 수행한다. Client/UI 실행·캡처 및 제품 화면 판정은 하지 않는다.

## G07. 현재 저장본의 상단 무지개 재생과 표시 기준점 재확인

사용자가 수정한 최신 rainbow.drop과 ball.upper, World2036, Composition의 bytes/hash를
out/CardDiceScale20260917/ball에 보관한다. 공 mesh5와 환경 무지개 sprite30의 실제
EmitterDirect provider, 입자 출생·수명·위치와 원작 CameraOffset·PSA_Velocity·size curve를
함께 비교한다. 이전 G06과 같은 clock이라는 이유로 표시 문제를 해결됐다고 판단하지 않는다.
반대로 시각적 차이만으로 이미 같은 궤적에 별도 속도 보정을 추가하지 않는다.

기존 actual Codec/Playback 검사에 paired world 위치, SourceEmitterWorld, evaluated velocity,
normalized life 로그를 추가한다. 모든 probe는 CRT assertion/abort, Windows 및 Engine 오류
대화상자를 먼저 차단하고 첫 오류에서 중단한다. 제품 Client/UI 조작 없이 root가 허용한
1회 실행으로 검증한다. 위치와 clock이 이미 일치하면 native2843 material/quad의 실제 표시
소비자를 조사한 뒤 근거가 있는 최소 변경만 후보로 만든다. live 등록은 root가 담당한다.

G07의 최소 후보는 derived sprite30의 원작 위쪽 velocity를 다시 켜 PSA_Velocity의 수직축을
복구하고, 사용자 상단 부착을 위해 offsetcentery를0→1로 명시적으로 조정한다. 원본 binary의
signed-size CPU packing을 확정한 것이 아니므로 pivot 변경을 원작 복원으로 설명하지 않는다.
World split.g1..g5의 upper track만 제거하여 자식62공에는 환경 무지개를 반복하지 않는다.
첫 공 g0의 upper와 shot 전체, 모든 공·분열·충돌·폭발의 시각과 배치는 유지한다.
