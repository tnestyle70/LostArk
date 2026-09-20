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
