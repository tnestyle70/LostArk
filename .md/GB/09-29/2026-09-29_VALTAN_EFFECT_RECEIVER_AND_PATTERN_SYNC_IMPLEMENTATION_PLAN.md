# 발탄 장판 수신 표면·패턴 표현 동기화 구현 계획

## G00. 현재 저장본과 적용 경계

`codex/valtan-all-effects-editing`의 기존 미커밋 변경과 사용자가 저장한 split 데이터를 보존한다.
이번 요청은 현재 디스크 저장본 기준 수정·게시를 이미 승인했다. 교체 직전 원문을 다시 읽고
stable ID와 대상 필드만 병합하며 원문 백업·hash 재확인·원자 교체를 수행한다. Client의 미저장
draft, 실행 중 Server 메모리와 파일 게시를 구분한다. 자동 UI 실행·Reload·프로세스 종료는 하지 않는다.

## G01. Shader_EffectDecalReceiver와 공통 데칼 경로

V1은 일부 native ID에만 actor 제외를 적용한다. 등장 연출의3852/3856과 피자의5184/5185/5198이
빠져 있어 skinned 발탄 위에도 투영한다. 기존 G-buffer marker/skinned bit와 actor program 판별을
공유 HLSLI로 옮기고 V1/V2 projected decal에 적용한다. V2는 기존 Target_PickPos를 바인딩한다.
메시·sprite·trail의 정상 공간 표현, 원본 색·alpha·depth와 팀장 rendering option은 보존한다.
새 HLSLI는 Client 프로젝트와 filters의 None으로 등록한다. FX 컴파일·GPU 수치 검사와 실제
사용자 화면 판정을 구분한다.

## G02. 420620 occurrence 방향

stage001의 붉은 영역은 STEP_07 root/follow에 붙고, 직전 피자 경고는 arena.center.target-follow를
쓴다. 경고 root는 회전해도 내부 ground decal의 snapshot attachment가11초 방향을 고정한다.
18개 ground occurrence는 기존 detail yaw에 source basis를 합치고 snapshot attachment를 꺼
mutable root를 소비한다. 시작 방향은 보존한다. stage001 cue도 같은 고정 arena target yaw를
소비하되 local yaw90으로 기존 body/source basis를 보존한다.
착지 붉은 notify014는 원본 emission 전방+X, 장판 shader5213는 projector -Z가 전방이다.
projector 축만 비교하지 않고11개 해당 occurrence의 snapshot source basis를-90에서0으로
보정해 붉은 발광의 위치·전방을 sector에 맞춘다. 본체 충격 notify013과 V2는 보존한다.
stage004의 부채꼴은 B_Root/b_root의 세 occurrence다. 실제 설치 모델에서 local Z가 world -Y라
socket roll+90이 world 반시계90도다. 무기본 요소를 보존하며 같은 asset을 참조하는 피자·3시·9시
지형 파괴에 함께 적용한다. yaw 필드에 임의90도를 넣지 않는다.

## G03. 6방향 후 전멸의 피해 시각

대상은 displayName이 `6방향 후 전멸 패턴`인 VALTAN_FLOOR_WIPE_130이다.
SECOND_SMASH의 피해 offset0을500ms로 옮기고 stage500ms를1000ms로 늘려 hit를 stage 안에 둔다.
500ms animation은 HOLD_LAST_POSE로 유지하고 기존 두 effect cue의99/101ms와 transform,
1500ms recovery를 보존한다. SIX_PIZZA 착지 피해250ms는 이 요청의 대상이 아니다.
Server root-motion 생성물도 기존500ms 움직임을 유지한1000ms hold tail로 맞춰
stage duration과 마지막 sample 계약을 함께 검증한다.

## G04. 돌의 실제 HIT와 시각 파괴

ownerHitChain은 이미 collider 내부0ms·나머지1500ms를 구분한다. BossCatalog가 Armed와 Hit에
같은 Off.full을 연결하고 armedEffectOwnsTerminal=true로 처리해 시각 파괴만 동시에 시작한다.
피자·3시·9시 row의 stopActiveOnArmed와 armedEffectOwnsTerminal을 false로 바꾸고, 같은 asset이며
terminal 소유가 없는 Armed에서는 표시를 시작하지 않는다. 실제 HIT에서 기존 active를 정리하고
전체 파괴 애니메이션을 시작한다. Product·Preview·seek 모두 같은 정책을 소비한다.

## G05. PublishCandidate의 최신 저장본 보존

원인 재현: 저장된9시 COMBO_STEP_11(index12) duration7870ms를 projector가 생성하고 receipt에 기록한 뒤,
`v2 == repository_v2`만으로 Encounter를 이전 Product2130ms로 교체한다. 이는 저장 후 별도 patch가
없는 정상 Publish 요청에도 발생한다. 이 무조건 교체를 제거하고 기존 semantic 동일성 helper로만
bytes를 재사용한다. 최종 선택된 Product에 receipt를 동기화해 숫자 표현만 다른 경우도 일치시킨다.
source freshness·writer lock·candidate rollback과 provenance 검증은 유지한다.

현재 저장된 피자 V2 landing의 CLIP_OCCURRENCE479ms는 사용자 저작값으로 보존한다.
기존 정확한 timing receipt를 source clock까지 확장해479ms binding 전체와 기존250ms hit를
각각 검증한다. 지연 전멸은 기존1ms sound와 변경된500ms hit·animation 전체를 기존 정확한
sound timing receipt로 기록한다. 새 허용 범위는 해당 occurrence와 payload뿐이다.

추가 요청의 `3회 구르기 후 돌진`은 현재 displayName `3회 땅 치기 후 돌진`의
VALTAN_DASH_CHARGE다. 방금 저장한 source dash Effect bytes와 CHARGE cue를 보존하고
최종172개 presentation artifact의 동일 hash 포함을 확인한다.

## G06. 검증과 게시

실제 후보 생성에서 최신 저장 Stage와 receipt의 일치를 확인하고 변경 JSON parse, source/product
projection, clip·hit 정합성 및 관련 native/GPU 검증을 수행한다. Valtan·Gameplay·Composition의
해당 publisher로 게시한 뒤 정상 Debug Product 증분 빌드를 사용한다. EXE/DLL 점유로 링크가
실패할 때만 사용자에게 해당 프로세스 종료를 안내한다. 새 C++ 파일은 계획하지 않는다.
최종 RESULT에는 설치·게시·컴파일·실행 중 메모리·사용자 화면 확인 상태를 나눠 기록한다.

## G07. 입장·사망 엔딩의 HUD 표시

MainApp의 기존 cinematic UI router suppression에 발탄의 실제 entrance/finale 재생 상태를
연결한다. 기존 쿠크 전용 함수 이름을 공통 이름으로 바꾸고 Level의 읽기 전용 상태만 소비한다.
HUD·boss HP sprite·text·창 입력은 같은 기존 gate를 따르며 저장 visibility를 수정하지 않는다.
종료·Stop·실패·레벨 전환 뒤 gate를 다시 계산해 원래 UI로 복귀한다. ImGui 개발 도구와
연출 자막은 유지하고 일반 전투 카메라에는 이 숨김을 적용하지 않는다.
Level Render에서 router 밖에 그리는 이름표·말풍선·상태 text에도 같은 조건을 적용한다.
source 종료 뒤 camera handoff까지 숨김이 유지되도록 camera owner가 suppression 상태를
소유하고 End_CinematicCameraOverride에서 해제한다. 새 카메라 owner 시작 시 입장/엔딩
여부를 다시 계산해 이전 상태가 일반 전투 연출에 남지 않게 한다.
