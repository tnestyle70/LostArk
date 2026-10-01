# 발탄 160줄 체력 21억과 기존 피해 기준 분리

## G00. 현재 값과 요청 경계

활성 Retail 발탄은 maximumHp741285439/160줄이며 콜로세움은 이 값에서 참가자 HP와
줄 단위 피해 기준을 함께 가져온다. 단순 HP 증가는 새 콜로세움 경기와 발탄 줄 단위 피해도
증가시킨다. 발탄 본체만 maximumHp2100000000로 바꾸고160줄 및 기믹 진입 줄수는 유지한다.
유령 발탄197222731/40줄, 쿠크 및 player 수치는 유지한다.

## G01. 보스 프로필과 게시 계약

기존 보스 프로필에 optional `damageReferenceHp`를 추가한다. 미지정은 maximumHp를
사용하고 발탄 본체만741285439를 명시한다. `Retail.balanceprofile.json`의 기존 발탄 행만
변경하며 `Publish-GameplayBalance.ps1`가 검증한 BOSS 행의 optional 마지막 열로 게시한다.
기존11열은 계속 읽히며 새 열이 있는 입력은 uint32 양수로 검증한다. 새 wire 필드는 없다.
Client 및 검사 도구의 동일 bootstrap 소비자도 조사해 필요한 읽기 호환을 연결한다.

## G02. 실제 Server 소비자

보스 catalog와 live entity가 피해 기준 HP를 소유한다. Spawn, numeric migration, 유령
전환이 각각 해당 profile의 값을 반영한다. 기믹·실제 HP·HUD는 maximumHp와 기존160/40줄
계산을 그대로 쓴다. PlayerSkillSystem의 bossHealthBarDamage만 피해 기준 HP를 읽는다.
콜로세움 입장·용병 생성은 피해 기준 HP로 참가자 HP와 피해 기준을 고정한다.
현재 참가자 HP92660680과 모든 기존 PvP 피해 기준741285439를 유지한다.

## G03. 검증과 배포

구형/신형 BOSS 행, 잘못된 참조값, 실제 발탄 HP·줄 경계·줄 피해, 유령 전환,
콜로세움 인간/용병 HP와 damage reference, numeric migration을 기존 focused contract에서
검사한다. 변경 JSON parse·publisher 검증·git diff check와 정상 Release Product Build 뒤
F1 복구 및 Deploy 피킹 수정까지 포함한 새 portable ZIP을 만든다. 실제 플레이는 사용자 확인이다.
데이터 후보를 검증하고 최신 저장본의 해당 필드만 반영한다. 새 C++ 파일을 만들지 않고
기존 Server/Client/publisher 경계를 확장하므로 project/filter 등록은 필요하지 않다.
