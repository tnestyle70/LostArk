# 쿠크 광기 수치와 마리오 입장 구현 계획

## G01. 실제 피해와 광기 계산

`ServerCombatHitRuntime::Apply_WorldToPlayer`는 이미 피해 비율을 광기로 바꾸지만 최대치
100에서 매 hit 정수 나눗셈을 하므로 1% 미만 피해가 누적되지 않는다. 보호막 이전 피해를
사용하고 쿠크 외 월드에서도 동작한다. 실제 HP 감소량을 읽고 소수 잔여량을 플레이어가
보존하며, 현재 room이 설정한 쿠크 policy 배율만 적용한다. 변신·부활·관문 리셋에서 잔여량을
초기화한다. 새 파일/프로젝트 등록이나 Shared packet 변경은 필요 없다.

## G02. 공·인형과 수치 editor

Retail profile의 `madness` stable row가 피해 배율, 공·인형 광기 배율, 기본 충전율과 반경을
소유한다. 기존 Balance Test의 scalar draft/CAS/publisher 경로에 Madness를 추가한다.
bootstrap row 확장 시 Shared/Server/Client/publisher/generation admission 버전을 37로 맞추고
v37의 base 4-column / Retail 12-column variant만 허용한다.
공·인형은 현재 damageable world cue와 시각 효과만 있고 충전 판정이 없다. 기존 cue의
생성/파괴/소유 보스 수명에서 살아 있는 플레이어의 근접 충전을 평가한다. 자연 pattern 종료
후에도 살아 있는 오브젝트는 충전하고, 파괴/취소/보스 사망 뒤 충전하지 않는다.

원본 LPK의 ZoneContentsGauge 3708100은 최대 100, 자동충전 0, hold 15000ms다.
SkillBuff 4219994는 1000ms마다 SkillEffect 421991716을 실행하며 200cm 원형과
421990117의 +10을 연결한다. 피해 비율의 정확한 원작 Server 계수와 인형 광기 계수는
확인되지 않았으므로 PROJECT_TUNED로 표시한다. 인형 근접 영역은 정확한 화염 mesh
피격 복원과 구분한다. 초기 특수 배율 2배는 사용자 요청에 따른 프로젝트 튜닝이다.

## G03. 마리오 승인

`Can_EnterMarioEntry`와 `Update_MarioControlState`의 Intro 진입 모두 이미 CLOWN인
살아 있는 플레이어만 허용한다. 일반 플레이어를 진입 과정에서 자동 변신시키지 않는다.
Debug 전용 이동은 기존 Debug 계약 안에서 변신을 명시적으로 준비할 수 있다.

## G04. 검증과 적용

기존 Server 계약에서 작은 피해 누적, 보호막/무적, 쿠크 외 월드, 배율, 공·인형의
파괴 수명, 일반인 거부/광대 입장을 확인한다. Balance scalar transaction 테스트에 Madness
저장·충돌 보존을 추가하고 publisher candidate validate, PowerShell parse, diff check를 수행한다.
실행 중 Client/Server는 자동 종료하지 않는다. authoring 기본값은 후보로 검증한 뒤
root가 한 번의 저장본 적용 승인을 받아 원자 교체하고 게시한다. Product 링크와
사용자 화면 확인은 구분한다.

## G05. 승인된 v37 게시와 Server 초기화 복구

사용자가 Server의 `Gameplay bootstrap header is invalid` 오류를 제시하고 준비된 Madness
반영·Gameplay Publish를 진행하도록 요청했다. 현재 저장본과 기존 후보의 차이는 madness
policy 한 개이며 Server 설치본은 v36/40699행, 현재 Shared 계약은 v37이다.
최신 Retail 저장본을 다시 읽어 해당 항목만 CAS·백업·원자 교체로 추가하고 공식
Publish-GameplayBalance의 Validate/Publish로 실행 데이터를 생성한다. 헤더 숫자를 직접
고치거나 reader의 버전 검사를 완화하지 않는다. 실제 Server catalog 초기화와 관련 native
검사로 수용 여부를 확인하며 Client/UI와 사용자 프로세스는 자동 실행·종료하지 않는다.
