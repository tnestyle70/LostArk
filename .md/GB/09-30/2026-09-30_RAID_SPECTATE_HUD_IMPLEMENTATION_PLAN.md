# Raid 관전·HUD·미니게임 타이머 구현 계획

## G00. 현재 상태와 변경 경계

기준 브랜치 `codex/release-regression-20260929`의 기존 미커밋 변경을 보존한다.
관전 버튼은 두 Arena에 표시되지만 클릭 동작이 없다. 카메라는 local character에 고정되어 있고
ALT_V Effect camera도 locally-controlled character만 허용한다. 관전은 Server entity ID를 선택하는
Client presentation 상태로 추가하고 gameplay 입력·Server authority는 바꾸지 않는다.

## G01. ClientReplication과 Arena 카메라

`ClientReplication.h/.cpp`에 관전 entity ID와 선택·조회 함수를 둔다. roster 순서의 살아 있는
인간 player만 다음 대상으로 선택한다. 사망한 현재 대상은 다른 player로 자동 이동하지 않으며,
같은 ID가 부활하면 다시 따라간다. local player가 부활하면 자기 시점으로 돌아온다.
두 Arena의 `Update_DeadScene`과 camera binding·Kouku area shot 소비자를 연결한다.
Effect camera는 camera follow target과 일치하는 replicated character를 소비한다.

## G02. Server deadline을 소비하는 제품 타이머

Shared `PLAYER_SNAPSHOT::iKoukuMinigameEndTick`은 absolute Server tick이며 0은 비활성이다.
Writer/Reader와 protocol version을 함께 갱신한다. Server 담당 변경은 입장 시 90초 deadline,
탈출 시 제거와 timeout 사망을 소유한다. HUD는 snapshot tick과 deadline 차이로 표시하고
프레임 간 보간만 Client에서 한다. Mario/maze 일반 HUD 숨김과 제품 타이머 text를 분리해 Release에도
표시한다. 경고 소수점은 같은 크기·기준선의 한 문자열로 그린다.

## G03. 기존 HUD consumer 수정

MainApp HP fill의 끝부터 shield의 흰 fill이 시작하도록 rect를 계산한다. Esther portrait는 매 raid
진입 시 해당 raid 순서로 명시해 이전 Kouku texture가 Valtan에 남지 않게 한다. Gate3 안내는
leader 여부와 무관하게 동일 Server-authoritative 이동 대기 상태를 표시한다.

## G04. 검증

기존 C++ 파일의 byte encoding과 CRLF를 보존한다. 새 C++ 파일은 없으므로 project/filter 등록은
변경하지 않는다. 관련 protocol roundtrip와 변경 TU 최소 컴파일, diff check를 실행하고 실제 수행한
증거만 RESULT에 남긴다. 제품 통합 빌드·링크는 root가 수행한다. Client/UI 화면은 사용자가 확인한다.

## G05. 배틀 아이템 수량·재사용 대기와 피해 감소 표시

포션과 배틀 아이템은 기존 `CGameRoom::Handle_UseItem`에서 같은 Server inventory stack을
소비한다. Client 장착 슬롯의 item ID는 소비 후에도 유지한다. 현재 배틀 아이템의 30초 재사용
대기는 Server에만 있고 HUD에 없으므로, `ItemCooldownEndTicks`를 item catalog의 skill ID로
기존 `PLAYER_SNAPSHOT::Cooldowns`에 투영한다. Client catalog는 skill ID와 cooldownMs를
읽고 quick slot은 Server inventory의 잔량과 cooldown 초를 표시한다. 기존 16개 상한은 실제
LanceMaster의 stance별 쿨다운 22개와 아이템 4개를 담지 못하므로 protocol 125에서 32로 확장한다.

Server가 보내는 `DAMAGE_REDUCED`와 `CRITICAL_DAMAGE_REDUCED`만 피해 숫자 아래
흰색 `피해 감소`를 숫자의 72% 크기로 그린다. critical 숫자의 노란색과 기존 fade를 보존한다.

## G06. 일반 incoming 피해의 ±10% 변화

추가 요청의 고정 정책은 WorldToPlayer의 최종 방어·buff 계산값을 평균으로 하는 ±10% 정수
균등 표본이다. ServerPlayer의 내부 serial과 tick/entity로 server-only deterministic 표본을
만들고 shield 흡수 전에 한 번만 적용한다. outgoing 피해·무력화·저작 광기 증가량과 JSON은
변경하지 않는다. 즉사/wipe는 항상 표본을 건너뛰며 damage event는 실제 HP 차감량을 보존한다.
기존 BattleItems contract suite 끝에 범위·평균·동일tick 다양성·재현성·shield·즉사 검증을 추가한다.

## G07. 보호막 0폭 이후 다시 표시되는 UI transform

사용자 테스트의 검은 보호막은 흰 texture/tint 문제가 아니라 quad 폭이 복원되지 않는 경로다.
MainApp이 shield=0에서 Set_SlotRect 폭을 0으로 설정하면 CUI_Sprite의 기존 Scale이 RIGHT를
0으로 만든다. 이후 Scale은 기존 RIGHT를 normalize하므로 양수 폭에서도 0이 남는다.
CUI_Sprite::Apply_Transform에서 현재 rect와 viewport로 RIGHT/UP/LOOK을 직접 재구성한 뒤
기존 회전·위치를 적용한다. 3D CTransform의 공통 Scale, shield 비율·배치, 이미지·shader는 보존한다.
실제 기존/수정 함수와 DirectXMath로 0→양수, 소진→재부여, 회전·viewport 변경을 검증한다.
제품 빌드는 실행 중인 Client/Server의 출력 잠금 해제 후 수행하며 Client/UI는 사용자가 확인한다.

## G08. Mario 타인 월드 UI 제외

사용자 답변에 따라 Mario camera subject의 UI와 파티·에스더·90초 타이머는 유지한다.
ClientReplication의 read-only audience는 replicated iMarioStage 1~4와 camera subject entity ID를
사용한다. 같은 공간의 다른 플레이어도 머리 위 HP·보호막·이름·말풍선·광기·상태 문구에서
제외한다. 적 체력바와 본인이 준 피해·받은 피해는 보존하고 타인의 전투 문구만 제외한다.
이미 생성된 피해·상태 문구도 owner ID를 보존하여 관전 대상 변경 시 즉시 같은 정책으로
다시 판정한다. snapshot·파티 roster·캐릭터 모델·게임 입력은 필터하지 않는다.
제품 실행파일과 게시 데이터는 실행 중인 사용자 테스트를 보존하며 변경하지 않는다.
