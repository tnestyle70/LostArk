# 쿠크·발탄 연출과 오브젝트 복구 구현 계획

## G00. 기준선과 작업 경계

`GB/koukubugfix-bingo`의 기존 게시·로딩 최적화와 사용자의 저작 데이터를 보존한다.
2026-09-20 요청의 렌더링, 카드 배정, 무기·오브젝트, 크기·캡처 튜닝, 발탄 연출을
기존 Engine/Shared/Server/Client 경로에서 연결한다. Client/UI 실행과 화면 판정은 사용자가 한다.
별도 렌더러·클라이언트 전투 판정을 만들지 않는다.

## G01. 렌더링 소비 경로

`RenderingProfileService`와 `MainApp`은 활성 카메라의 환경을 계산한다.
1관문 실효 노출 2와 복구된 LUT의 중간톤 증폭을 분리하고 해당 구역의 노출 후보를 만든다.
카드는 V2 leaf의 base color가 scene bloom에 직접 기록되는 경로이므로 기존 emissive gain과
별도의 scene bloom 기여를 같은 V2 codec·shader·Detail로 연결한다.
연출 fog는 활성 카메라 소유권에 한정해 끄고, 2관문 camera3에는 캐릭터 수신 directional만 복구한다.

## G02. Server 카드 수명

`CKoukuSaydonLogicRuntime::Assign_EncounterCard`는 이미 배정된 문양 mask를 제외한
문양에서 무작위 선택하고 색은 독립 선택한다. `CGameRoom::Apply_KoukuGateEntryCard`가
방의 다른 참가자 문양을 전달하고 유효한 기존 배정은 유지한다.
`Begin_KoukuRaidCinematic`은 이전 관문 카드 상태를 정리한다. 전투 진입의 기존 spawn 경로는
snapshot 전에 배정을 완료한다. 1~4인·반복 tick의 중복 금지와 기존 상태 유지를 검증한다.

## G03. World Object 저작과 실제 소비자

뿅망치는 저장된 Object resource·Boss BODY bone·World occurrence를 대조한다.
Object Tool Map Position의 preview override를 확인하고 중앙 앵커 이동은 전체 emission을
동일한 기준으로 이동시킨다. 칼날 일반/즉사 Effect·Collider는 각 motion 정본을 유지한다.
실제 끌림·피해는 Server의 기존 패턴 audition 경로를 사용한다.
빙고 해골과 폭탄은 원본 수명·flip·심지·폭발을 World effect track으로 연결한다.

확인된 `(1.25,0,0) / (0,180,0) / (2,2,2)`는 쿠크 휠윈드 Motion에만 적용한다.
카드미로 컷씬은 별도 Motion instance를 사용해 이 수치를 상속하지 않는다. F1의 휠윈드
Transform은 같은 Object draft의 해당 Motion만 편집하고 기존 Save/Publish 경로를 사용한다.
연출의 실제 World actor 골격을 BOSS anchor resolver에 연결해 전투 NPC의 다른 pose를
사용하던 망치 부착을 교정한다. 설치된 파생 WModel의 실제 CModel pose를 수치 검증한다.

앵콜 F1 회전은 최신 Gameplay placement의 yaw만 안전하게 병합해 저장한다. 모델과 지팡이는
3관문 Saydon과 동일한 catalog 경로를 사용한다. 기존 3관문 일반 공격 pattern ID는 Encore
대상으로 Server audition에 제출하며, 3관문 전용 map/layout/mechanic은 원래 관문에서 재생한다.

## G04. 플레이어 크기와 차원술사 캡처

기존 `ARENA_CAMERA_PROFILE`의 presentation scale과 ImGui Save/Reload 경로에 전체·class·
광기광대·Mario 크기를 노출한다. 광기광대는 무기를 숨기고 Mario만 기존 뿅망치를 표시한다.
ALT V의 기존 ScreenPost edge speed·duration·rotation·destination과 큐브 model cue를
어제 저장본과 대조해 복구하며, 재질 소비와 사용자 튜닝 패널을 연결한다.
카드미로 플레이어 망치의 Pos/Rotation/Size는 별도 맵별 F1 profile 필드로 저장하며 실제
직업별 손 본 기준을 사용한다. LMB/Q는 여섯 직업의 원본 골격별 native clip을 연결하고,
Server 타격 시점과 기존 typed command를 사용한다. 이펙트 저작 슬롯은 사용자가 채운다.

## G05. 발탄

부활 패턴의 잘못된 source action join을 원본 Action15/Respawn_1과 대조한다.
누락된 source Effect와 full restore를 기존 catalog/cue 경로에 연결한다.
연출별 카메라·world sequence·effect를 실제 pattern consumer에 연결하고 ghost 모델 생성 실패를
수치·생성 상태로 구분한다. 삼각형은 반지름 9m에서 13.5m로 확대하며 기존 이동 시간을 보존한다.

## G06. 교체·검증

데이터 후보는 최신 디스크의 stable ID/변경 필드로 병합한다. 실행 중 저작 데이터의 최종 교체는
저장본 기준 승인 뒤 hash 재확인·백업·원자 교체를 사용한다. 뿅망치 스크린샷 수치 반영은
사용자가 명시적으로 요청한 범위다. 미저장 draft를 자동 Reload하지 않는다.
변경한 JSON/XML parse, 최소 컴파일, 관련 기존 계약 검사와 `git diff --check`를 실행한다.
Product 빌드는 한 번 취합하며 실제 프로세스 점유로 막힌 링크는 미완료로 기록한다.
RESULT는 소스 반영, 게시, 빌드, 사용자의 화면 확인을 구분한다.

## G07. Stage 진행과 row 수명 분리

Stage 합계는 보스의 다음 stage/pattern 진행 시각이다. 이펙트·사운드·World·Logic·Summon의
최종 종료 시각은 별도 timelineDurationMs로 게시하며, 긴 row 때문에 마지막 stage를 늘리던
Python projector와 C++ preview 확장을 제거한다. Full lifetime 편집도 stage를 변경하지 않는다.
Client는 자연 완료한 presentation 세션의 원래 시작 시각과 핸들을 유지해 남은 row만 재생한다.
Server는 이전 pattern의 정의 revision과 판정 ledger를 별도 수명 동안 유지한다. 명시 Stop,
중단, 보스 사망, 관문 전환은 잔여 세션을 취소하며 다음 pattern의 상태를 이전 row가 덮어쓰지 않는다.
긴 소리/이펙트와 짧은 stage, 연속 pattern, 강제 중단을 실제 소비자 계약으로 검증한다.

## G08. Server 시작 데이터와 접속 복구

2026-09-20 사용자 실행에서 MAHARAKA worldbootstrap 누락으로 Server가 listener 생성 전에 종료했다. LAN debugger 설정은 Client 192.168.0.14, Server 0.0.0.0으로 일치한다. 기존 전체 World publisher 및 MAHARAKA 전용 Navigation publisher로 현재 디스크 정본을 게시하고, 실제 Server의 모든 world 초기화와 192.168.0.14:7777 TCP 도달을 확인한다. Client/UI 실행은 사용자가 한다.

Product는 compile-only 정책을 유지하되 시작에 필요한 여섯 world와 navigation의 누락을 빠짐없이 알리도록 기존 BuildDomains 필수 출력 계약과 Product 점검을 맞춘다. 누락 검출을 기존 build-pipeline 검사로 확인하며, C++/shader 변경 없는 준비 검사 수정 때문에 제품 전체를 재컴파일하지 않는다. 사용자 검증 가이드는 요구사항별 실제 반영, 남은 범위, 정확한 패널과 검증 순서를 별도 결과의 읽기 경로로 제공한다.

## G09. 사용자 화면 회귀 교정

1관문 캐릭터 간접광은 기본값0의 별도 조명 입력으로 profile/region부터 native character pass까지 연결한다. 기존 맵과 무관한 material 수식은 보존한다. Mario는 별도 source-rendering profile을 사용하므로 G1 alias 노출 상속과 구분하여 fog·tone·LUT·bloom 소비를 확인하고 범위별 후보를 만든다.

`KoukuSaydonBossTool`의 Complete Play는 선택 Pattern/Bundle/Flow의 실제 V1/V2/World/모델 준비가 끝난 뒤 기존 typed Server audition을 요청하도록 한다. 실패와 대기를 구분하며 일부만 준비된 상태로 재생하지 않는다.

첨부3의 휠윈드 수치는 P24 World occurrence placement다. 기존 Object Motion의 손 offset (.3,0,0)/quaternion(-.5,-.5,.5,.5)을 복구하고 P24 placement(1.25,0,0)/(0,180,0)/(2,2,2)를 유지한다. F1 바로가기 역시 같은 Workbench World box를 편집·저장하도록 바꿔 공유 Object와 중복 보정을 막는다. 카드미로 플레이어/연출 망치는 각자의 기존 부착을 유지한다.

주사위의 실제 카드 visual 연결과 spawnIntervalMs를 추적하고 요청한 네 장 간격을2배로 늘린다. 머리 위 카드의 V2 bloom 수정과 별도 원본 V1 카드의 bloom 소비를 구분한다. 거미 회전은 root snapshot·본 부착·원본 mesh basis를 실제 occurrence별로 대조하여90도 변환의 중복/누락만 교정한다.

G1 카메라는 원본 book track37800ms보다 긴41703ms row 때문에 마지막 pose를 유지하는 경로와 MainApp의 강제 camera return을 교정한다. 카메라 track 종료와 World/효과 tail의 수명을 분리하고 follow pose로 연속 blend한다. ALT V는 cube root가 발밑인 상태를 screen 축소 목표로 삼는 경로를 교정하고 Effect Detail의 저장값이 실제 capture/cube 위치·크기를 제어하게 한다. 빙고 검증은 기존 G06에 정확한 Pattern/Object 이름과 실행 순서를 보강한다.

각 후보는 별도 out 경로에서 준비·검증한다. 최신 저장본 승인 이후에만 stable ID/필드별 병합·백업·원자 교체·domain publish를 진행하고 Product Build를 한 번 취합한다. 사용자 화면 확인과 수치·컴파일 결과를 분리한다.


## G10. Complete Play 실사용 거절의 P78 수명 보완

사용자가 Complete Play - Sequences + Pattern Flow에서 P78 Logic lifetime 오류를 확인했다. 원인은 카드 간격을2→4초로 바꾸면서 기존 명시적 row 수명 필드 `durationMs`를 함께 저장하지 않은 통합 데이터 수정이다. Stage합17165ms, Logic끝22114/22980/22981ms이며 source에 durationMs가 없어 Python 개별 pattern admission이 P78을 Unavailable로 격리했다. 게시 전체 성공은 이 패턴과 G1 raid gate의 게시 성공을 의미하지 않았다.

현재 계약을 바꾸지 않고 P78에 `durationMs: 22981`을 추가한다. Stage, clip, 네 카드시각, 사용자 offset은 바꾸지 않는다. source revision을 증가시키고 기존 projector와 Gameplay publisher만 사용한다. 후보 전후의 public prepare_publication 및 실제 C++ BossTool Reload/Validate_PatternFlow로 P78, G1~G3 흐름, sequence raid gate의 연결을 확인한다. Python helper나 C++ validation의 상한·명시 lifetime 검사를 제거하지 않는다. Client/UI는 실행하지 않으며 runtime Server 메모리와 디스크 게시를 구분한다.


## G11. 거미 카운터 몸체 전방 보정

추가 사용자 설명은 FX 자체90도가 아니라 플레이어를 향해 이동하면서 몸체가180도 반대로 향한다는 것이었다. 실제 설치MN_RPCZ_00의rpcz00_att_battle_6_03을CModel에서0/50/100%샘플하면 얼굴은지속+X다. 현재 Server의atan2(dx,dz)+90은+X를이동반대로 돌리므로 기존 per-charge저작필드 `kakulsaydon.g1.logic.31.chargeYawOffsetDegrees`만−90으로 바꾼다. 이 정의를 쓰는 P15 Logic4/5/6세 돌진에만 적용된다. 플레이어target과world-space7m이동벡터, Stage1167ms, Counter/Fear및FX자료는보존한다. 사용자가 원인확인후수정을명시적으로요청하여 이전의 사용자직접수정보류를 해제했다.

원본클립forward를월드이동벡터로보내는bodyyaw와이동방향은분리하고 native-axis변환은한번만한다. 양쪽눈/입본의실제좌표와8방향목표를대조하여 기존+90의뒤쪽정렬과−90의앞쪽정렬을수치검증한다. 범용Server코드가+90자체를거부하도록바꾸지않으며 다른nativebasis를쓰는pattern과effectrotation은변경하지않는다.

최종 Gameplay 게시에서 발견된 음수 yaw 직렬화 오류도 같은 변경으로 수정한다. 기존 −360..360 허용 검사와 Server parser는 유지하고, `PATTERNLOGICCHARGE`의 yaw 출력만 기존 `Format-InvariantSignedFloat`로 바꾼다. 거리는 음수 금지를 유지한다. 실제 게시 코드로 부호·경계·잘못된 입력을 검증한 뒤 정식 게시를 다시 실행한다.


## G12. 쿠크 관문 조명과 반복 재생 회귀

실효 노출1과 원본 LUT 단일 적용을 유지한다. 현재 사용자 요청에 따라 실제 base/G1/G3/source-rendering profile의 기본 directional과 관문 환경영역 directional/specular를 before-restoration 저장 기준으로 복구한다. 기존 캐릭터 독립 간접광과 map ambient, LUT, bloom을 보존하며 G2 camera3만 character receiver로 덮던 임시 경로를 제거한다. 카드미로와 의도적인 blackout, Mario 영역 및 comparison profile은 보존한다. 시퀀스와 전투가 같은 관문 환경 입력을 소비하도록 한다. 이는 사용자가 선택한 프로젝트 조명 복구이며 원본 DDL exclusion의 완전 재현이나 과거 exposure2의 pixel 일치로 기록하지 않는다.

카드미로는 머리 위/발밑 광기 HUD 중 발밑 게이지와 Q 오른쪽 LMB 슬롯 표시를 숨기고 Server Q만500으로 바꾼다. LMB/Mario 피해와 미로 표적 규칙은 유지한다. G2 saved flow는 camera를 포함하는 기존 P77로 연결한다. P25 피자 10소환의 실제 admission을 확인하고 쇼타임의 +X 원본 전방과 목표 yaw 오프셋, 레이저/휠윈드/팡파레 root motion을 발생 단위로 교정한다.

Sequence animation이 없는 경우 별도 idle boss를 중복 표시하지 않는다. G1 sound 저장 revision의 Reset 소비자와 Complete Play 소비자를 일치시키고 시퀀스 종료에서 같은 책/조명을 복원한 즉시 다시 준비하는 동기 작업을 조사해 준비와 commit을 분리한다. 기존 runtime/Server 권위와 실패 시 보존 경계를 사용한다.

현재 Client/Server 프로세스는 없고 Visual Studio만 열려 있다. 최종 데이터는 최신 디스크 stable ID별 필드 병합, hash 재확인, backup/atomic replace 뒤 domain publisher로 반영한다. 다른 작업의 dirty 변경은 유지하며 자동 commit하지 않는다. 기능별 최소 컴파일과 기존 contract 검사, JSON parse, diff check를 완료하고 Product Build를 취합한다. Client/UI와 마지막 화면 비교는 사용자가 한다.

## G13. 3관문 입장과 빙고 전투 연결

Release 통합 진행은 G2 종료 연출과 G3 입장 연출의 중복 연결을 제거하고 G3 입장만 재생한다. 마지막 네 효과 위치를 서버 플레이어 도착 위치로 사용한다. G3 입장 완료는 별도 WAIT_ENTRY 상태로 복제하고 전투 입력을 막은 채 HUD의 기존 재시작 버튼을 ‘3관문 입장’으로 표시한다. 기존 typed 투표가 승인되면 전원 목표 navigation을 먼저 검증하고 원래 전투 스폰으로 이동해 전투를 시작한다. 이후 버튼은 ‘재시작’이며 G1/G2 자동 진행은 유지한다.

빙고 Complete Play는 기존 G3 블랙홀빔·메두사·기분나빠·3방향 화염·십자화염 폭발 다섯 패턴을 반복한다. 시작 시 일반 해골 두 칸을 배치한다. 5초마다 머리 폭탄 표식을 예약하고 5초 뒤 현재 타일에 폭탄을 놓으며 3초 뒤 중심과 상하좌우를 변환한다. 일반 해골은 재폭발 시 빨간 해골이 되고 두 해골 모두 광기 초당3을 적용한다. 전체 parent의 50000ms 빙고_바닥_망치생성 row는 10초마다 겹치지 않는 두 평행 경로를 뽑고 3초 UV 경고 뒤 망치를 이동시킨다. 끝 collider의 즉사와 갈고리 마지막 지점의 이동 잠금 해제는 Server 권위로 처리한다. 기존 로직·World cue·snapshot 소비 경로를 확장한다.

쇼타임·카드미로의 원본 사운드를 연결하고 원본 GameMsg의 한국어 자막을 typed sequence row와 제품 글꼴 렌더러까지 연결한다. 빙고 입장 유리 파손 이펙트는 추출과 resource 준비만 하며 다른 작업자의 카메라·애니메이션 저작은 수정하지 않는다. 로컬 Mario가 선택하는 source-rendering profile의 FXAA를 끈다. 빛번짐 원인이 FXAA라고 단정하지 않고 해당 설정 변경과 사용자 화면 확인을 구분한다.

추가 C++ 파일 없이 기존 문서·runtime·프로젝트 등록을 사용한다. schema 변경은 저장/다시 읽기 및 잘못된 입력 보존, Server 변경은 실제 room 계약, 클라이언트/서버 프로토콜 변경은 양쪽 컴파일로 검증한다. 최신 디스크 기반 필드 병합과 domain publish를 거쳐 Release Product 빌드로 취합한다.

## G14. Rendering Workbench 실시간 비교

사용자가 직접 렌더링을 비교하도록 Recovered map materials 위에 방향광, 노출0.5/1/2배, LUT grading, FXAA와 Bloom 전환을 둔다. 실제 코드의 노출 배율과 LUT 적용 횟수를 구분해 표시한다. 비교는 저장 문서를 바꾸지 않는 세션 override이며 region·pattern·Mario의 품질 계산 뒤 한 번만 적용한다. 다음 프레임 region 보간 전에 원래 값을 복원하여 배율 누적과 비교값의 저장을 방지한다. 닫기·초기화·Level 전환에서 복원하고 명시 Save는 기존 저작 draft만 저장한다. 모델만 밝게 하는 별도 LUT 경로는 만들지 않는다.

## G15. 앵콜과 발탄 연출 사운드·자막 보완

추가 사용자 요청으로 앵콜 입장 SCENE07A와 마지막 SCENE01B의 원본 AkEvent/Stop/Fade, 발탄 PS와 SCENE02A/02A01/02A02/04A/06A/07A의 원본 트랙을 설치된 bank 및 현재 데이터와 대조한다. 이미 설치된 전체 음원은 재사용하고 누락 layer/variant만 추출한다. 원본 Play와 Stop/Pause/Resume를 구분하며, 임의 BGM을 대체 음원으로 사용하지 않는다.

쿠크 마지막 P75/P9는 기존 51,185ms 음성 두 layer를 재사용하고 잘린 재생 행을 보완한다. 원본 BGM의 10ms Play delay, 48.974121초 Stop과 2초 페이드를 후보에 반영한다. 빙고 사망은 boss iteration을 끝낸 다음 기존 P9 CLEAR 시퀀스를 한 번 재생한 뒤 최종 clear를 복제한다. 앵콜 입장 카메라·애니메이션은 다른 작업자의 범위로 유지하고, 원본 사운드·자막 resource 및 시간표를 같은 저작 catalog와 인계 문서에 준비한다. 새 불완전 입장 시퀀스를 Raid에 삽입하지 않는다.

발탄 원본 자막은 일반/상단6행과 늑대별 말풍선2행이다. 실제 연출 transform과 시간이 이미 있는 기존 WorldSequence template에 optional subtitleTracks/soundTracks를 추가하고 document codec, Map publisher, World player 및 MainApp 폰트까지 연결한다. 말풍선은 stable actor slot의 실제 visible 모델 상단 위치를 화면에 투영한다. 기존 preview에서도 같은 데이터를 읽고 모델이 없거나 숨겨지면 해당 말풍선만 생략한다. 다른 Level의 새 재생기를 만들지 않는다.

World sound는 기존 Engine SoundCue handle을 사용하며 Pause/Seek/속도/명시Stop을 같은 owner가 제어한다. 자연 visual 완료 뒤 이미 시작한 소리의 tail은 유지하되 카메라와 이동 잠금 시간을 음원 길이만큼 늘리지 않는다. 발탄 BGM은 기존 CValtan Music owner를 교정하고 World에 중복 재생하지 않는다. 발탄 두 늑대의 source preview는 연결하되 아직 없는 제품 진입 카메라 흐름까지 새로 만들지는 않는다.

데이터는 원본 hash, stable ID patch와 별도 resource manifest를 만들고 실제 codec/시각·음원 시간 검증 후 병합한다. WAV는 무음 장치로 decode/admission을 확인하고 사용자 오디오 장치나 Client 화면을 자동 재생하지 않는다. Source 추출, runtime 연결, 최종 화면·청취 여부를 구분해 RESULT에 기록한다.


## G16. 쇼타임 중앙 복귀의 높이와 노란 장판 고정

쇼타임 P35의55.869초 logic.17은 logic.47 중앙이동의 BOSS_TELEPORT_XZ를 소비한다. 이 계약은 authoring Y를 사용하지 않아 원래 위치의 공중 높이가 남는다. 같은 쇼타임39.209초 logic.63 및 다른XZ이동은 기존높이보존을 유지해야 하므로 공통XZ종류를바꾸지않는다.

logic.47은 P38.logic.1도 공유하므로 기존47을 보존한다. 기존 mechanic 경로에 BOSS_TELEPORT_GROUNDED를 추가하고 동일 목적지의 새 stable logic kakulsaydon.g1.logic.100(nextLogicOrdinal 100→101)를 만들어 P35.logic.17의 참조만 전환한다. 현재Server navigation의목적지바닥을검증한뒤보스높이와 animation root기준높이를같은delta로commit한다. 목적지 navigation 또는 높이 검증 실패는 위치와 root 기준을 모두 보존한다. Grounded 성공 시 해당 Pattern에만 바닥 root 기준 정책을 유지해 다음 Stage의 origin Y를 navigation 바닥에서 캡처한다. 원본 마지막 점프는 보존하고 지면 아래 값은 clamp하며, Pattern 종료/새 실행에서는 정책을 해제한다. C++Composition codec/저작선택, projector, Gameplay publisher/catalog, 기존Server logic consumer까지같은종류를연결하며새packet이나별도이동runtime을만들지않는다. source후보는최신Data의stable ID/변경필드만병합하고정식Composition/Gameplay publish한다. 새C++파일추가는없다.

사용자가확인한대상은쇼타임도중노란바닥장판이다. 초기생성world위치와방향을보존하고그이후player/boss이동추적만제거한다. 확인된 노란 부채꼴은 effect.kouku.gate3.showtime.sector.warning.shot의 yellow outline이며 billboard=false다. 해당 BOSS occurrence99/648/700/716의 followBoss만 false로 바꾸고, 같은 리소스의 기존 MAP occurrence755 및 주황색 경고10행은 보존한다. 전체 particle의 billboard/local-space나 공통shader를 바꾸지 않는다. 기존spawn-anchor snapshot경로를재사용하며원본내부UV/particle회전animation은요청대상이아니다.

검증은실제소비자에서기존공중Y→navground와다음root tick유지,실패시기존위치보존,기존XZ높이보존및쇼타임−90추적유지,장판발생시점의TRS동일/이후boss·player이동독립을확인한다. 진행중Product빌드후변경된최종소스로필요한증분빌드와Server계약을실행한다. Client/UI와최종화면판정은사용자가수행한다.


## G17. 룰렛 발판 위 플레이어의 서버 높이 보존

Server Refresh_KoukuSupportSurfaces는 이미 룰렛 범위 안의 정지/이동 플레이어 높이를 올리고 표면 종료 시 원래 지면으로 내린다. 현재 snapshot.canPredictMove=true 상태에서 Client CCharacter::Update_LocalMovePrediction이 정적 navigation ground.y를 다시 써서 서버 높이를 덮는 것이 누락 원인이다.

기존 GameRoom_Replication snapshot 생성에서 실제 활성 support 범위 안인 플레이어만 canPredictMove=false로 보낸다. 기존 snapshot XYZ 보간을 사용하며 Server 이동 입력과 canMove를 유지한다. 범위를 나가거나 표면이 종료되면 즉시 원래 prediction policy로 복귀한다. Client Transform 직접 보정, navigation 복사본, Shared wire, 새 Data는 추가하지 않는다. 기존 support 범위/시계 판정을 재사용하고 Mario/갈고리/다른 강제 이동의 기존 상태를 바꾸지 않는다.

실제 outbound snapshot 검사는 발판 안/밖, 소멸, 정지/이동, 입력 가능 상태 보존을 다룬다. Server와 Client의 소비자 경로를 함께 읽고 필요한 Server 증분빌드를 최종 Product에 포함한다. 실제 룰렛 시각 높이와 이동 감각은 사용자 화면에서 확인한다.

## G18. 최종 검토에서 확인한 카드 생성 알림 분기

Server의 pursuit.started는 이미 생성된 카드의 lifecycle marker다. ClientReplication의 pursuit 분기가 이를 contactVisualId의 폭발 cue로 전달해 정상 생성에도 false와 오류 status를 반환한다. Kouku Level은 이때 debug log를 남긴 뒤 update를 계속하므로 Complete Play의 종료 원인은 아니다.

기존 live owner·정확한 object·pinned revision 검증을 보존하면서, 정확한 started HIT_PULSE/repeat0 및 유효한 시각·pose만 lifecycle marker로 수락한다. 시작 알림에서 폭발을 재생하지 않고 실제 contact는 기존 Play_TargetedCombatContact로 전달한다. 알 수 없는 hit ID와 잘못된 소유자는 계속 거부한다. 실제 분기 소비자와 focused Debug/Release 컴파일로 확인한다. 사용자 실행 중 EXE는 교체하지 않고 별도 출력 경로에서 링크하며, 배포된 Client에 반영하려면 사용자가 현재 검증을 끝낸 뒤 다음 빌드를 실행한다.

## G20. 사운드 리소스 검색과 목록 렌더링 비용

사용자는 1관문 피날레의 두 원본 소리와 현재 WAV를 요청했고 Sound resource 창을 열면 2.8fps가 된다고 보고했다. 기존 Render_PresentationResources는 Effect만 cache하고 Sound는 매 프레임 전체 inventory에서 구조체를 복사하고 모든 Selectable을 제출한다. 검색 입력도 Effect에만 있다.

기존 Workbench H/CPP의 Sound 목록에 별도 session cache와 검색 문자열을 둔다. inventory 갱신은 source cache를 무효화하고, Created 목록은 draft generation과 검색 변경에서만 갱신한다. stable resource/source ID 선택을 보존하고 ImGuiListClipper로 보이는 행만 제출한다. 검색은 표시 이름·asset 경로·resource ID의 부분 일치를 쓰고, 현재 선택 경로와 검색 결과 수를 표시한다. 기존 Preview/Create/Append 소비자를 재사용한다. 새 C++ 파일·프로젝트 등록·Data 변경·publish는 없다. G19 Play 시간 회귀는 별도 인계 상태로 유지한다.

검증은 변경 TU의 Debug/Release 최소 컴파일과 기존 소비자 검토, 실제 설치 목록 규모 및 cached idle frame의 복사/행 제출 감소를 확인한다. 사용자 Client/UI를 실행하지 않으며 실제 2.8fps 개선 수치는 사용자가 같은 창에서 확인한다. 원본 이벤트 추출 근거와 연결 WAV를 대조해 두 소리의 역할을 별도 결과로 기록한다.

### G20 추가: 사운드 바 앞·뒤 재생 구간 자르기

현재 presentation bar의 양쪽 edge는 시작/길이만 바꾼다. SOUND 왼쪽 edge도 source offset을 바꾸지 않아 WAV 처음부터 다시 시작하는 것이 구간 자르기와 다른 동작이다. 이미 저장·projector·Player에 연결된 `soundSourceStartMs`를 재사용하여 SOUND 왼쪽 trim에서는 timeline start/source start를 함께 옮기고 끝 시각은 보존한다. 오른쪽 trim은 시작점 두 개를 보존하고 길이만 변경한다. WAV 앞0·최소1ms·pattern/source 끝에서 clamp하며, 바 가운데 이동은 source offset을 유지한다. Box Detail에는 Source In/Out ms와 원본 이벤트/경로를 표시한다. 원본 WAV와 사용자 저장본은 자르거나 덮어쓰지 않는다.

## G21. 일반 Sequencer Play의 사운드 시계 누적 오차

사용자는 앞뒤를 자르고 이동·저장한 사운드가 일반 Play에서 약 2초 어긋난다고 보고했다. 현재 Sequence의 실제 SOUND 저장값과 Workbench의 ordinary Play 문서 전달을 확인한다. Sound Detail의 숫자는 Apply 후 Save, timeline drag는 release 후 Save가 문서 저장 경계다. 사용자 저작 파일은 읽기만 하고 임의로 시간이나 잘라낸 구간을 보정하지 않는다.

`CKoukuSaydonPresentationPlayer::Update`는 double 밀리초를 누적하지만 `Sample_BundlePreview`와 MainApp을 거친 `Sample_Preview`는 매 프레임 `Preview_ClockMs()`의 정수 표시값을 다시 내부 double에 대입한다. 일반 재생의 분수 밀리초가 사라져 정상 속도로 진행하는 FMOD와 시퀀서가 벌어진다. 내부 clock을 소유한 일반 sample은 소수부를 보존하고, 실제 capture 경계 이동·외부 animation/server clock·명시 Seek는 기존 지정 시각을 소비하도록 기존 Player만 수정한다. 새 C++ 파일·프로젝트 등록·schema·Data·WAV 변경은 없다.

검증은 production clock 소비자 본문으로 24/30/60/120/144fps 및 가변 프레임 재생의 수정 전 누적 오차와 수정 후 오차, 외부 clock·Seek·capture 경계의 정확한 이동을 대조한다. 실제 Workbench/문서 객체의 격리 저장본에서 start/duration/soundSourceStart 세 값을 함께 바꿔 Save→Reload→일반 Play 전달을 반복한다. Debug/Release 최소 컴파일과 diff 검사를 수행하고 실행 중 Client의 EXE/DLL은 교체하지 않는다. 장치 청취와 사용자 UI 결과, G19의 별도 server/local 전환 및 stale scrub 문제는 이 수치 검증과 구분한다.

## G22. Save 뒤 이전 Preview의 사운드가 남는 경로

사용자가 증상을 사운드 바가 없는 시점의 재생으로 명확히 했다. G21의 누적 시계 문제는 별도 수정이며 이 증상의 해결 근거로 사용하지 않는다. 현재 Save는 디스크와 Workbench draft를 저장하지만 실행 중 immutable Preview 문서를 교체하지 않는다. 바 이동·삭제·trim 후 Save만 하면 이전 문서의 SOUND가 계속 들릴 수 있고, 기존 scrub/Resume도 그 문서를 재사용한다.

Workbench H/CPP의 기존 snapshot에 draft generation을 함께 기록한다. Save 성공 후 변경된 local preview가 남았으면 기존 STOP transport로 소리를 포함한 이전 preview를 중단하고 cursor를 보존한다. 다음 일반 Play는 기존 최신 문서 staging을 사용한다. Scrub/Resume은 generation이 다른 snapshot에 SEEK/RESUME만 보내지 않고 기존 pattern/bundle preview request를 다시 만든다. 검증 실패 시 이전 문서를 계속 발음시키지 않도록 STOP을 요청한다. 실패한 Save는 기존 draft와 저장본을 보존하며 성공으로 표시하지 않는다. Server-follow와 Complete Play의 immutable pin은 local 갱신 대상이 아니다.

MainApp의 공용 Resource Resume도 Workbench의 typed Resume 경계를 소비한다. 새로운 local Play의 actor/world staging 실패 시 같은 owner의 기존 preview를 즉시 일시 정지하고 같은 frame의 기존 STOP transport로 정리한다. 실패 status를 보존하고 실패한 문서의 generation을 정상 재생으로 간주해 이전 소리를 Resume하지 않는다. 새 C++ 파일이나 별도 audio runtime은 추가하지 않는다.

검증은 현재 Workbench 객체의 이동·trim·삭제→Save→STOP, 다음 Play의 최신 구간, stale scrub/Resume과 실패 보존, 변경 없는 preview의 연속 Resume 및 Server 경계를 포함한다. 실제 SOUND active gate/cleanup/Stop 본문은 앞·뒤·삭제·이동한 bar 밖에서 handle이 없는지 별도 검증한다. Client UI·오디오 장치 재생 없이 최소 Debug/Release 컴파일·전체 Client 링크를 수행한다. 사용자 요청에 따라 실행 중 EXE는 안전하게 이전 image를 백업한 뒤 다음 실행 경로에 수정본을 설치하며, 현재 process가 갱신됐다고 설명하지 않는다.

## G23. 아레나 입장·3관문 대기 BGM과 전투 이동 연결

사용자가 지정한 2nd circus 음원은 설치된 `Sound/KoukuSaton/S_BGM_COMMANDERRAID/bgm_midnightc_ed_m12_ready_terrace_2ndcircus__559227263.wav`다. 원본 Wwise event 248306555의 media559227263과 실제 WAV 128.135604초가 일치한다. 기존 `m02_scene_movetocircus` 이동 연출음과 구분하고 원본 반복 BGM의 설치 WAV를 재사용한다.

최초 KoukuSaydon 아레나 진입 대기에는 이 BGM을 재생하고, 입장 시퀀스 재생이 시작되면 중단한다. 3관문 입장 시퀀스가 끝나 마지막 arrival Logic의 나무 판자 지점에서 Server가 WAIT_ENTRY를 확정하면 처음부터 다시 재생한다. 좌상단 기존 '3관문 입장' 명령이 승인되어 COMBAT으로 전환할 때 중단한다. 기존 typed 관문 명령·투표와 Server의 `Enter_KoukuRaidCombat(3)` 목적지 사전 검증·전원 이동·실패 보존을 재사용한다. Client가 직접 플레이어 위치를 바꾸거나 클릭만으로 이동 성공을 가정하지 않는다.

Level의 기존 music 채널과 Server raid 상태 소비 경계에서 재생·종료 수명을 관리한다. 같은 대기 상태를 매 frame 읽어도 BGM을 다시 시작하지 않으며, 시퀀스 중 재발음·거절된 입장 후 소실·Level 퇴장 후 누출을 방지한다. Debug 시퀀스 미리보기의 시작도 기존 성공한 preview 진입 경계에서 처리한다. 사용자 Sequence SOUND 행·잘라낸 구간·World arrival 좌표는 수정하지 않는다. 새 C++ 파일·프로젝트 등록·별도 오디오 런타임은 추가하지 않는다.

검증은 실제 BGM 파일·원본 event 근거, 기존 3관문 arrival/WAIT_ENTRY/전투 이동 소비자, 반복 update·시퀀스 시작·WAIT_ENTRY 재생·승인/거절·퇴장 상태를 확인한다. 변경 TU의 Debug/Release 최소 컴파일과 Client 링크·설치, diff 검사를 수행한다. Client 실행·화면 이동과 실제 청취는 사용자가 확인한다.

### G23 추가: 일반 Play와 공간 이동의 같은 대기 동작

사용자가 일반 Play에도 같은 동작을 적용하고 ImGui 아레나 이동에 '3관문 입장 전 공간'을 추가하도록 확정했다. 기존 Return to Start와 같은 PlayerController의 typed Server teleport에 마지막 arrival slot0 위치를 전달한다. 승인 후 Server-replicated local player가 시작 공간 또는 3관문 판자 공간에 있을 때 기존 BGM을 재생하며, 공간을 나가거나 실제 시퀀스 재생이 시작되면 중단한다. local Preview owner 종료·교체·자연 완료에서 suppression을 해제하고 실제 도착 위치로 다시 판단한다. 재생 중·일시 정지 중에는 판자 도착만으로 BGM이 켜지지 않는다.

일반 Play는 Server Raid WAIT_ENTRY를 만들지 않으므로 기존 RESTART를 보내면 잘못된 현재 관문을 시작할 수 있다. 기존 GATE_PROGRESS_KIND에 명시 ENTER_GATE3를 추가하고 Client 확인창·투표창·Level→IPlayerCommandSink→Server 관문 투표로 전달한다. active raid에서는 기존 GATE3 WAIT_ENTRY와 immutable epoch를 검증하고 `Enter_KoukuRaidCombat(3)`을 사용한다. nonraid에서는 실제 판자 공간의 proposer와 기존 파티 leader/전원 동의, 승인 직전 위치와 roster를 검증하고 기존 관문 활성화·Server teleport 소비자로 연결한다. 전원 목적지와 boss 생성·정리를 사전 검사한 뒤 commit하여 실패 시 이전 위치·보스를 보존한다. 일반 Play를 Client에서 제품 raid 상태로 위장하지 않는다.

공간 판정은 Client와 Server가 같은 공유 상수·유한 좌표 범위를 소비한다. 구 peer가 새 enum을 읽지 못하므로 protocol95→96을 같은 변경으로 적용하고 Shared·Server·Client를 함께 컴파일한다. 기존 H/CPP를 확장하며 새 C++ 런타임 파일은 없다. shared header를 별도로 둘 경우 해당 프로젝트의 ClInclude와 기존 물리 폴더 filter만 추가한다. 변경 enum 왕복·범위 거절, 파티 입장·거절·목적지/생성 실패 보존, 실제 위치에 따른 BGM 재생·종료를 focused 검증한다. Data 및 사용자 timeline은 수정하지 않는다.

### G23 공유 공간 판정 헤더

`Shared/Public/Gameplay/KoukuArenaReadyAreas.h`는 판자 공간의 X/Y/Z 범위와 시작 공간의 XZ 거리·높이를 판정한다. Server의 3관문 입장 승인과 Client의 BGM·버튼 표시가 같은 함수를 읽는다. Y 범위가 판자 아래의 전투 공간을 제외하며, NaN·무한대는 범위 비교에서 거절된다. 기존 Shared 프로젝트에 ClInclude와 Gameplay filter를 등록하고 XML parse와 실제 헤더를 포함하는 경계 검사를 실행한다. 별도 구현 CPP는 없다.

```cpp
#pragma once

namespace LostArk::Shared
{
    // Authored Gate 3 arrival deck, including its walkable fence footprint.
    // Height separates the deck from the combat floor below the same XZ area.
    inline bool Is_KoukuGate3EntryTerrace(const float x, const float y, const float z) noexcept
    {
        return x >= -30.f && x <= -5.f && y >= 23.5f && y <= 28.f && z >= 947.f && z <= 972.f;
    }

    // Authored player.spawn.kakul.party01 and its initial waiting platform.
    inline bool Is_KoukuArenaStartArea(const float x, const float y, const float z) noexcept
    {
        const float dx = x - 3.29f, dz = z + 10.69f;
        return dx * dx + dz * dz <= 100.f && y >= 5.64f && y <= 11.64f;
    }
}
```
