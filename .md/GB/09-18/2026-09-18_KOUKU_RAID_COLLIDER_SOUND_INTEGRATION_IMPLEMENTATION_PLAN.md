# 쿠크세이튼 전체 재생·피해 판정·사운드 연결 구현 계획

## 작업 기준

2026-09-18 사용자가 저장한 관문별 Pattern Flow, 1마리오 1~4페이즈 Parent,
카드 주사위 속박과 피자·마리오 위치 Trigger를 기존 Server 실행 경로에 연결한다.
편집 중 저장본은 계속 바뀌므로 revision 1748은 조사 기준일 뿐 교체 기준이 아니다.
설치 후보를 검증한 뒤 최신 저장본을 stable ID와 변경 필드로 병합한다.

## G00. 현재 동작과 변경 경계

피해는 Server fixed tick의 CombatObject/Logic 판정이 소유하고 Client 이펙트는
같은 객체의 pose와 lifetime을 표현한다. 사운드는 기존 SoundCueCatalog와 SOUND
occurrence를 사용한다. 개별 particle에 collider를 붙이거나 별도 재생기를 만들지 않는다.
카드미로의 망원경 담당자·사냥꾼 역할과 Mario lane 이동은 이미 있으므로 이를 확장한다.
현재 Complete Play의 시퀀스와 Flow 진행은 요청 Client에 묶여 있으므로 관문 진행과
시퀀스 시작 시각을 Server가 소유하도록 연결한다.

## G01. 스킬별 피해 템플릿

원본 Action/SkillEffect의 범위·시간·offset을 출처와 함께 추출한다. 정적 영역은 기존
COLLIDER와 ENTER_AREA, 투사체와 추적 과녁은 CombatObject 인스턴스에 붙는 피해
템플릿을 사용한다. 한 발당 한 판정 객체이며 visual의 파티클 수와 무관하다.
normal damage, 최대 HP 비율, 즉사를 구분하고 원본에서 입증되지 않은 피해량은
PROJECT_TUNED로 기록한다. 원본 geometry를 복원하지 못한 항목은 coverage에 남긴다.
빠른 투사체는 이전 pose부터 현재 pose까지 검사하고 충돌 판정 뒤 contact despawn한다.

## G02. 원본 사운드

Action clip notify와 시퀀스 InterpTrackAkEvent의 실제 원본 시각을 사용한다.
Wwise Random은 한 variant를 선택하고 Layer/동시 Play는 함께 재생한다. 필요한
layer·delay·gain은 원본 PCM으로 오프라인 합성하고 단일 무변조 WAV는 재사용한다.
soundEvent stable ID로 기존 catalog를 조회하며 occurrence seed를 고정해 seek 결과가
바뀌지 않게 한다. 미해석 switch/state와 control-only event는 누락 사유를 기록한다.

## G03. 카드 주사위와 위치 Trigger

속박 lifetime 동안 생존 참가자 중 한 명을 Server가 고르고 나머지만 속박한다.
1인은 속박하지 않는다. 카드 projectile은 같은 비속박 대상을 사용한다. 종료·취소 시
이 occurrence가 소유한 속박만 해제한다. 중복 Trigger와 Duration이 대상을 재추첨하지
않도록 동일 역할의 활성 구간을 합친다. 피자 시작 위치는 typed boss teleport로 연결한다.
카운터 unavailable은 저장본의 typed definition과 follow-up의 Gate/body 계약을 모두
검증하며, 잘못된 참조를 숨기기 위해 PRODUCT 검증을 완화하지 않는다.

## G04. 마리오 1~4페이즈

각 Parent의 실제 접촉 창에 설정한 MARIO_ENTER stage를 사용한다. 마지막 tick의
접촉은 패턴 완료 뒤에도 commit하고, 완료 정리가 먼저 큐를 버리지 않게 한다.
솔로는 마리오 복귀 후, 파티는 진입자와 별도로 나머지 인원이 2페이즈를 진행한다.
비진입 인원은 저작된 아이언메이든 위치 기준으로 배치하고 3~4인은 한 명만 속박한다.
복귀 지점은 진입자에 pin해 0키 복귀와 자연 출구가 같은 위치를 사용한다.
보스는 사용자가 지정한 (5.96, 1.30, 950.59)에서 중앙을 보고 무력화 성공까지 진행한다.
1페이즈 실패와 즉사 칼날은 기존 Server 즉사 RESULT 계약으로 연결한다.

## G05. 카드미로와 관문 진행

카드미로 포탈은 처치한 해당 문양의 위치에 생성한다. 망원경 시야 확대는 담당자에게만
적용하며 나머지 N-1명은 각자 포탈로 중앙에 돌아온 뒤 전원이 2관문으로 복귀한다.
Gate2 Flow는 이 완료 조건 뒤 피자로 이어진다. 단독 패턴의 반대 보스는 Idle,
Bundle은 포함된 보스들이 함께 재생한다.
Release에서는 1관문 진입 콜라이더가 동일한 Server raid 준비를 시작하고, Debug Complete Play도
같은 경로를 사용한다. 고정 참가자 1~4명 모두의 revision/리소스 준비 ACK가 끝난 뒤
공통 Server 시각으로 시퀀스·실제 참가자 스폰·현재 저장 Flow를 진행한다.
2026-09-18 추가 요청에 따라 보스 사망 후 자동 10초 이동은 제거한다. main의
클리어/MVP·관문 투표 UI에서 전원 진행 승인하면 다음 관문 시퀀스를 시작하고,
재시작 승인하면 현재 관문 시퀀스를 다시 시작한다. 마지막 3관문은 퇴장/재시작 UI를 유지한다.
컷씬 종료 전에는 전투를 시작하지 않으며 관전자·연출 중 UI 명령은 서버에서 거절한다.
Server orchestrator는 기존 audition 실행과 world sequence broadcast를 재사용한다.
새 C++ TU를 만들면 해당 .vcxproj와 .filters에 함께 등록한다.

## G06. 검증과 반영

Client/Server 최소 컴파일, 실제 catalog parse/project/publish 후보 검증, 1~4인
카드미로·마리오·속박·피해 판정·관문 상태 전이 계약을 실행한다. ServerPlayer layout
변경이 있으므로 과거 object 파일 재링크로 실행 성공을 판정하지 않는다.
설치 직전 hash/revision을 다시 확인하고 백업·원자 교체·자기 변경 rollback을 사용한다.
실행 중 도구의 Reload와 Server 재시작, 실제 Client 화면 확인은 별도로 기록한다.
사용자가 직접 하는 화면 검증을 headless 수치 검증으로 대신 완료 처리하지 않는다.

## G07. 기존 폴더 선택형 얇은 ZIP 배포

2026-09-19 최종 추가 요청은 기존 ResourceDelivery 방식의 얇은 실행 ZIP이다.
Release EXE/DLL/CSO와 양쪽 게시 DataFiles, Client가 직접 읽는 이번 변경의 필수
Data JSON과 함께 바뀌는 EffectCatalog가 참조하는 authored JSON 전체를 포함한다.
전체 Data를 무조건 복사하지 않으며 Resources·사운드 미디어는 넣지 않는다.
다른 리소스는 이미 팀에서 공유받았다는 사용자 확인을 적용한다. 이번 신규 WAV 18개는
`NEW_SOUND_PATHS.txt`의 상대 경로만 별도로 전달한다.
기존 LostArk 폴더 선택 wrapper, 설치 전 검증·기존 파일 백업과 no-build Client 바로가기를
재사용하고 새 배포 체계를 확장하지 않는다. 최종 빌드·게시 파일의 hash manifest를
검증한 뒤 ZIP을 전달하며 Client를 에이전트가 자동 실행하지 않는다.

### G07 추가 반영: DataFiles 포함 배포 복구

2026-09-19 후속 요청으로 EXE 전용 ZIP을 전체 실행 배포본에서 제외한다. 최신 main과
실제 저장본의 일치를 확인하고 Client/Server owner를 순차 게시한 뒤, 양쪽 DataFiles와
같은 protocol의 Release 실행 파일을 함께 포장한다. Bern·Character Select 지역 Navigation과
발탄 blocker의 Client/Server 일치를 확인한다. 기존 얇은 배포의 직접 소비 Data 보충분과
폴더 선택 설치 흐름을 재사용하며 배포 파일 목록·설치 설명·RESULT를 최종 ZIP에 맞춘다.

### G07 추가 반영: 흰 창 종료와 catalog 참조 누락

전체 EffectCatalog와 변경된 authored JSON 일부만 전달하는 방식은 수신 PC의 기존
Data 상태에 의존한다. 현재 loader가 시작 중 검사하는 모든 `authoringPath`와 optional
`screenOverlayPresentationPath`를 목록에서 수집하고 크기·hash 검증 후 같이 전달한다.
Release는 초기화 단계·HRESULT·상세 실패와 종료 사유를 기록한다. 기존 폴더 선택 실행기는
시작 직후 실패를 감지해 해당 실행의 로그를 보여준다. 설치 검증 성공, 프로세스 생존,
사용자의 실제 Lobby 진입 확인을 구분한다.

## G08. ESC 커서 선택과 Composition 게시 계약 보정

ESC 옵션의 열린 콤보가 클릭 소비 플래그를 먼저 세워 자기 항목 선택까지 차단하는 순서를
`SystemOptionWindowView.cpp`에서 바로잡는다. 팝업이 선택 입력을 처리한 뒤 같은 프레임의
하위 UI 클릭을 차단한다. 커서뿐 아니라 같은 콤보 경로를 쓰는 옵션의 선택도 보존한다.
Release Product를 정상 증분 빌드하고 새 Client EXE를 배포본에 포함한다.

WorldSequence v3의 기존 `colliderTracks`와 `loopFullPresentation` 계약이 Map publisher와
Client에만 연결되고 Composition publisher 검증에서 누락된 부분을 맞춘다. 필드를 삭제하거나
unknown-field 거부를 풀지 않고, 기존 shape·시간·결합 제한을 검사한 뒤 원본을 게시한다.
실제 저장본 전체 검증과 잘못된 collider/loop 입력의 거부를 확인한다.


## G09. Bern 군단장 레이드의 쿠크 입장 투표

2026-09-19 후속 요청은 Bern의 기존 군단장 레이드 UI에서 쿠크를 선택하고 입장하는
제품 경로를 완성하는 것이다. 기존 PROPOSE → Server 전원 수락 투표 → typed world transfer를
유지하며 UI가 직접 Level을 전환하거나 Server 승인을 우회하지 않는다.
`CRaidEntryPreviewView`는 수신한 투표 target을 별도로 보존해 선택 탭과 무관하게
`RAID_DEFS`의 해당 레이드 이름을 확인창에 표시한다. 알 수 없는 target을 발탄으로
대체하지 않으며 수락·거절·취소 시 기존 proposal lifecycle과 함께 표시 target을 정리한다.
실제 함수의 비시각 입력·문구·intent 검증과 Client 최소 컴파일을 수행하고 최종 화면은
사용자가 확인한다. Server의 파티 target 제한 보정은 같은 입장 수직 슬라이스로 검증한다.

## G10. 4인 검증용 Flow 재설정과 Release ZIP 갱신 (2026-09-19)

사용자가 P29 내려치기 크래시를 확인하고 현재 디스크 저장본의 Flow 교체와 publish를
명시 승인했다. 패턴 정의는 보존하며 stable entry ID와 다음 순서만 변경한다.

- GATE1: P1,P2,P6,P7,P47,P48,P58,P78,P79,P80,P81,P82,P83.
- GATE2: B1(P8+P9),B2(P10+P11),B3(P12+P13),B6(P15),B7(P17),P21,
  B10(P23),B9(P24),P27,P85,P86,P87,B4(P28),P25.
- GATE3: P88,P91,P92,P93,P52,P46,P66,P76,P35. 각 Mario Parent의 기존
  P33 2페이즈 → 성공 P42 후속 연결을 유지한다. 독립 P33 행은 중복 생성하지 않는다.

각 행 완료 뒤 1000ms, 마지막 행은 0ms를 사용한다. 마지막 패턴 완료는 보스 사망으로
처리하지 않으며 기존 Idle 대기와 사용자의 처치 후 클리어·전원 투표 계약을 유지한다.
교체 직전 최신 hash를 재확인하고 백업·원자 교체한다. 공식 KoukuSaydon owner로 게시해
저장본·Encounter raidGates·Server bootstrap의 순서와 revision을 대조한다.

B2 중 Lobby의 Server entry failed 보고는 구조화된 recovery와 실제 소비 경로에서
원인을 조사한다. 재현된 결함만 수정하고 필요한 Release 최소 컴파일·focused 검증을
수행한다. 최종 동일 revision의 Data와 runtime을 기존 Full ZIP 설치기로 포장하고
무결성을 검사한다. IP 10.16.127.103은 유지하며 Client/UI 실행·4인 화면 판정은 사용자가 한다.

추가 승인된 쇼타임 연출 P76은 P66 다음, P35 직전에 두며 중간 대기는 0ms로 한다. 기존 5000ms 연출의 두
animation을 원래 시작 시각대로 stage로 분리해 게시 규칙을 충족시키며 카메라·음향·
clip과 blend 시간은 보존한다. 실패 이유를 숨기는 admission 완화는 하지 않는다.

Release의 Kouku lifecycle 소비가 MainApp의 Debug guard 안에 있어 제품 레이드 owner의
메시지가 누적되는 결함을 수정한다. 기존 service include와 Update만 제품 경로로 옮기고
저작 UI의 Debug 경계는 유지한다. CLIENT_INVALID_SERVER_RESPONSE/WSA10055 관측과
해당 누적 결함의 focused 재현을 구분하며, 실제 4인 B2 재생은 사용자 검증으로 남긴다.


## G11. 컷씬 게시·UI 복원과 실패 원인 진단 (2026-09-19)

사용자의 최신 지시에 따라 저장된 미게시 CUTSCENE P73/P74/P75/P77에 현재
occurrence의 종료 시각을 포함하는 parent duration만 추가한다. P75에 없는 배우는
새로 추정해 생성하지 않는다. G10의 최신 GATE3와 함께 source revision 1753으로
공식 게시하며, 실제 product available 목록과 Client presentation을 확인한다.

실제 컷씬 시간트랙 소유 또는 Server cinematic pending일 때 제품 UI의 그리기와
입력을 공통 억제하고 종료 시 기존 visible/open 상태로 복귀한다. 일반 전투 follow/static
카메라는 컷씬으로 분류하지 않는다. 저작 도구와 연출 자체의 KakulFade overlay는 유지한다.
배경만 검게 되는 원인은 미확정이므로 scene/light 값을 임의로 바꾸지 않는다. 기존
Client session JSONL에 컷씬 경계/샷 변경과 map visibility, scene profile, light/camera
소유, fade alpha를 기록해 다음 사용자 재현의 근거를 확보한다.

Client NetworkManager의 15개 bounded 결과/알림 큐 overflow에 queue 이름, 현재 깊이,
한도, 실제 packet type을 남긴다. 실패 정책과 cap은 유지한다. V1/V2 이펙트의 준비·
admission·draw 실패와 Engine frame failure를 Release에서도 bounded 파일에 기록한다.
신규 header-only `Client/Public/EffectFailureDiagnostic.h`는 Client.vcxproj 및 filters의
기존 물리 폴더 항목에 등록하며 새 TU는 추가하지 않는다. 기존 C++ 인코딩을 보존한다.
Server의 기존 RoomPerf에 UTC epoch 시각, 실제 gate/flow/member pattern, combat object
수, 이전 outer-loop tick 지연/reset을 보강하고 기존 Diagnostics 경로에 bounded 파일로
보존한다. 서버 timeout과 송신/객체 상한은 근거 없이 늘리지 않는다.

최종 변경의 Debug/Release 제품 최소 컴파일, JSON/XML parse, diff check, 기존 focused
lifecycle 검증을 적용한다. 이미 통과한 검증은 관련 코드 변경이 없는 한 반복하지 않는다.
실제 4인 지연·컷씬 배경·효과 GPU 표시는 사용자가 최종 확인한다.


## G12. Release 4인 실측 결함 수정과 UI PR 통합 (2026-09-19)

현재 4인 로그의 원본과 SHA를 보존하고 실제 실패를 수정한다. P28 진입 시 이미 설정하는
Server 권위 MAZE area HUD를 미로 대기 판정에 포함해 미로 복귀 전 P25가 소모되지 않게 한다.
Sequence P1/P4의 WORLD 6행씩만 카메라·scene 끝까지 연장하고 revision65로 공식 게시한다.
화면상 영구 검은 배경과 확인된 2,852ms 무대 공백은 구분한다.

Kouku의 4인 동시·정상 재사용 꼬리 중첩과 P48 카드 수명을 실측해 해당 Level의 admission
예산을 정한다. 다른 Level과 Engine 실제 light/provider 한계는 보존한다. Bern 폭포 sprite는
회전한 local particle과 비균일 owner가 만드는 shear를 허용하고 billboard에 필요한 축 길이와
origin만 사용한다. NaN/Inf 거부와 기존 TRS 결과, 실제 source occurrence를 수치 검증한다.

UI PR #413의 원본을 현재 브랜치에 통합한다. 이름표·MVP clip과 cinematic guard를 결합하고
새 G 키캡 아트와 JSON을 배포에 포함한다. Native Win32 메뉴는 창 생성에서 제외하며 client
영역 계산도 메뉴 없는 상태로 일치시킨다. endpoint는 사용자 최신 지정192.168.0.14를 유지한다.

기존 Release session JSONL에 최초 terminal reason/detail, 메모리와 main-pump 지연, 실제
Lobby 실패 문구 표시 edge를 추가한다. Server closure는 PID/build/event와 partial send bytes를
남긴다. 로그를 위해 timeout이나 protocol 실패 정책을 임의 완화하지 않는다.
최종 Release Product 빌드와 focused 비시각 검증 후 새 v4 ZIP을 만든다. 화면과 실전4인
성공 여부는 사용자가 판정하며 모든 종료 원인이 제거됐다고 확대해서 기록하지 않는다.

## G13. Bern LAN 송신 지연과 Alt+V 누락의 실측 후속 수정 (2026-09-19)

v4 실행 중 Server PID53060의 Bern session 4/6/8/10/12가 05:08~05:16 KST에
WSA10060으로 종료됐다. 각 마지막 송신은 약250ms이고 reliable rejection과 snapshot
drop은0이다. 마지막 .43 연결은6,143개 frame을 보낸 뒤 끊겼으므로 최초 접속 주소나
입장 승인 실패로 분류하지 않는다. 호스트의 EXE와 설치 Data는 v4 manifest와 일치한다.
원격 PC의 수신 정체와 무선망 지연 중 어느 쪽인지는 Client 로그와 대조한다.

`Server/Public/ClientSession.h`, `Server/Private/ClientSession.cpp`의 session socket을
nonblocking으로 설정한다. `Send_All`은 성공한 바이트만 offset에 반영하고
WSAEWOULDBLOCK에서는100ms 단위 readiness 대기를 수행한다. 사용자가 명시적으로
요청한 대로 송신 대기 시간만으로 연결을 종료하지 않는다. 실제 socket 오류와 FIN,
명시적인 Stop의 처리와 bounded join은 유지한다. 동일 socket을 사용하는 `Receive_Frame`도
read readiness 대기를 소비한다. 송신 stall/recovered 이벤트는 offset·frame 크기·peer·session·
마지막 수신 시각·대기 시간을 남기고 bounded 파일과 중복 제한을 사용한다.
메모리 상한과 reliable 순서, snapshot coalescing을 보존하며 무제한 queue나 reliable drop으로
가리지 않는다. 기존 SessionTransport 검증에 일시 수신 중단 후 정확한 재개와 Stop을 추가한다.

Alt+V는 접속 종료와 별도로 수정한다. Client508의 차원술사2050540은 자연 종료가 남은
반복 사용분으로 owner mesh/draw 한도를 넘었고, 원격 도화가31930은 Bern의 scene 한도로
거절됐다. 또31930의 local-only visibility가 현재 document에 없는 source element를 지목해
예산을 통과한 재생도 rollback됐다. 사용자가 전체 이펙트에 동일한 처리를 요청했으므로
Level/owner/local/remote에 따른 whole-effect admission 상한과 prewarm의 같은 임의 ceiling을
제거한다. 비용은 진단으로 유지하고 uint overflow, 문서·리소스 유효성, prepared clone 계약은
보존한다. 전체1167개 catalog의 실제 연결된 visibility sidecar를 검사하고 존재하지 않는
Artist9개와 Lance clip1 6개 참조만 정리한다. 효과 요소나 원본 수명은 삭제하지 않는다.

Engine `Presentation_Manager`의 provider256/light384/post64/overlay64는 초기 reserve로만
사용하고 유효한 제출을 개수로 거절하지 않는다. `Light_Manager`는 기존 shader의400개
constant-buffer ABI를 유지하며 CPU 배열400개를 채워 순서대로 여러 batch를 그린다.
scene directional shadow·static channel·source receiver와 type run 순서를 batch 사이에도
보존한다. Map/Kouku/Debug preview의 선행 light truncation도 제거한다. 임의 개수 초과가
전역 frame failure가 되는 경로를 없애며 실제 GPU/device 오류를 성공으로 위장하지 않는다.
Renderer 로그에 UTC를, Client session 로그에 실패 stage/HRESULT/deviceRemovedReason을
남겨 네트워크 종료와 실제 렌더러 실패를 분리한다.

실행 중 사용자가 추가로 보고한 '카드·일부 장판만 보이고 일반 보스 효과가 누락되지만
데미지는 들어오는' 현상은 독립적으로 추적한다. Server collider와 Client 표시 경로가
분리되어 있으므로 정상 데미지를 정상 표시의 증거로 쓰지 않는다. 실제 published Product
reader와 V1/V2 분기, pattern revision·clock·occurrence를 대조한다. 공용 Effect의 외부
transform history와 follow owner/model/bone/root 실패 중 Debug 출력만 하던 제거 경로는
기존 Release 파일 진단에 asset·occurrence·handle·level·시각·원인으로 기록한다.
정상 lifetime·Level 전환·Stop은 실패로 기록하지 않으며 fault 인스턴스마다 한 번만 남긴다.

전체 Product staging의 실제 재현에서 P73 CAMERA 행의 빈 worldSequenceInstanceId가
예외를 발생시켰다. named World Object에만 sequence identity를 연결하고 CAMERA/SOUND의
고정 WORLD 좌표가 사용하지 않는 필드로85개 패턴 전체를 거부하지 않게 한다.
실제 WORLD 소비자의 필수 ID와 named World의 sequence 검사는 유지한다. 쇼타임 총구의 independent source-boss
패턴은 animation 목록이 없지만 원본 Effect에는 SourceModelPreview가 있다. 이 경우 기존
source-preview sampler에 Effect-local clock을 전달하고 정상 일반 pattern timeline은 유지한다.

`Tools/Network/Collect-RuntimeDiagnostics.ps1`은 각 PC의 최신 session·effect·renderer 로그와
실행 파일 hash, 실행 중 프로세스 경로/시작 시각, endpoint 및 설치 영수증을 로컬 ZIP으로
모은다. 기존 로그와 프로세스를 변경하거나 업로드하지 않는다. C++ 신규 TU가 없으므로
프로젝트·filters 등록은 바꾸지 않는다. 실제 socket 및 effect 경계 검증, 최소 컴파일,
JSON/PowerShell parse와 diff check 후 RESULT에 빌드와 사용자 화면 검증을 구분한다.

### G13 배포 경계 정정

사용자는 Resources를 팀에서 별도로 관리하므로 PNG를 포함해 한 파일도 새 ZIP에 넣지
말라고 명시했다. 새 배포본은 기존 source 폴더를 변경하는 설치기를 제거하고 EXE/DLL/CSO,
`Client/Bin/DataFiles`, `Server/Bin/DataFiles`, 실제 직접 소비하는 `Data`를 정식 상대 경로에
둔다. ChangedData 폴더나 중첩 runtime ZIP을 만들지 않는다. 최상위 실행기는 압축 해제한
배포본의 EXE와 Data를 사용하고, 사용자가 고른 기존 LostArk 폴더에서는 Resources 경로만
읽는다. Server도 배포본의 실행 파일과 DataFiles를 사용한다. 압축 파일 내 경로·manifest와
실제 빌드 hash, no-launch 사전검증을 대조하며 Resources나 사용자 저장소를 설치·덮어쓰지 않는다.

## G14. Release 시퀀스 준비 진단과 명시적 종료 세션 정리 (2026-09-19)

사용자 실행 Client44800은 Product revision1753의85개 패턴 로드에 성공했지만,
raid epoch1이 PREPARING에서 ABORTED로 전환됐다. 기존 로그에는 phase만 있고 준비 실패의
정확한 사유가 없었다. Client MainApp의 준비 단계, Action/Sequence local·pinned revision,
composition ID와 gameplay revision, 실패 문자열을 기존 session JSONL에 기록한다.
Server는 raid 종료 사유를 기존 bounded room 진단 경로의 heartbeat에 기록하고
epoch·ready mask·참가자 수·Sequence ID와 revision을 포함한다. 실패 사유를 얻기 위해
revision 검증을 제거하거나 READY를 강제로 보내지 않는다.

실제 Release reader와 설치된 데이터를 사용한 Action/Sequence 로드·9개 Sequence 확장,
Server의 실제 진입 trigger 경로→READY→공통 cinematic 시작을1인·4인으로 확인한다.
이 검사는 준비와 서버 상태 전이를 검증하며 화면 표시나 사용자 사건의 원인을 대신하지 않는다.

250ms 송신 종료 제거 뒤 명시적인 ROOM_FULL 종료 요청까지 무기한 대기할 수 있는 경계를
닫는다. 최초 Request_Close_After_Flush부터2초 동안 terminal 응답을 보내고, 기한 뒤에는
이미 종료하기로 한 세션만 정리한다. 정상 세션의 송신 정체에는 이 기한을 적용하지 않는다.
반복 종료 요청은 기한을 연장하지 않고 최초 reason/context와1회 종료 callback을 보존한다.
실제 비수신 소켓, 정상 drain, 활성 연결의2초 초과 대기·정확한 재개를 확인한다.

새 C++ TU는 없으며 project/filters 등록은 바꾸지 않는다. 사용자 Release 빌드와 중복으로
정본 빌드를 실행하지 않는다. 최종 산출물에 변경 소스가 반영됐는지 확인한 뒤 v6 ZIP을
별도로 만들고 v5를 보존한다. Resources·PNG·ChangedData 제외와 기존 폴더에서 Resources만
읽는 실행 계약을 유지한다.
