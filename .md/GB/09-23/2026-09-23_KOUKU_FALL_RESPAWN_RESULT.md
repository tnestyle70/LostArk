# 쿠크 낙사면·관문 부활·Albion 상승 결과

## G00. 코드 반영

`Begin_PlayerFall`/`Update_PlayerFall`의 기존 Server 상태 전이를 유지하고, 쿠크에서만 기준 지지면
아래5m 통과를 사망 기준으로 사용한다. 발탄은 기존45틱 deadline을 유지한다. 비행 도중 아래
배경 바닥을 추적하는 경우도 낙사면을 먼저 검사한다. 강제 재피격으로 공중에서 다음 포물선이
시작돼도 최초 `fKnockbackSupportY`를 유지하므로 낙사면이 공중으로 올라가지 않는다.

쿠크에서 이탈이 허용된 일반 수평 넉백은 현재 발탄의 `Trace_ForcedSurface`를 재사용한다.
실제 벽·몸체 충돌이 확정한 직선 구간의 물리 지지를 검사하며, 보행 마스크 때문에 먼저 멈추는
기존 별도 쿠크 경계 추정을 제거했다. Gate1·Gate3는 항상 펜스를 적용하고 Mario는 전용 경로를 유지한다.
다른 세션의 발탄 지지면 함수 본문과 발탄 이동 분기는 보존했다.

부활 버튼의 기존 `C2S_REVIVE_PLAYER` 소비자는 쿠크에서 현재 승인 raid/audition/관문 상태를
resolve하고 `KOUKU_GATES`의 기존 전투 진입점을 navigation에 검증해 이동한다. 판정이 없으면
기존 player spawn을 사용하며, 목적지가 없거나 검증에 실패하면 HP·위치를 바꾸지 않는다.
사망 좌표 근처 투영은 하지 않는다. 성공하면 기존 HP/자원/상태 복구와 함께 비행 플래그도 정리한다.
발탄의 `boss.valtan.center` 부활은 그대로다.

## G01. Albion 기존 피해 경로 확장

Shared `ATTACK_HIT_TEMPLATE`에 `fRiseHeightM`/`iPushMs`를 추가했다. JSON 계약은
`riseHeightM`/`pushMs`이며 둘 다0 또는 높이0초과100m이하+비행100~5000ms를 요구한다.
Server parser는 기존25열과 신규27열 `PATTERNATTACKHIT`를 모두 받는다.
CombatObjectRuntime의 TIMED/CONTACT 두 소비자가 기존 world-hit과 PlayerSkillSystem에
수평0·지정 높이·지정 시간의 bounded 강제 ballistic 반응을 전달한다. 별도 피해 runtime은 없다.
Client·projector·emitter·저작 데이터 반영은 root의 통합 작업으로 따로 검증한다.

## G02. 검증 상태

`ServerGameplayContractTests_KoukuOverlap.cpp`의 기존 runner에 낙사면 경계, 옛 timer와의 구분,
비행/연속피격 기준 유지, 실제 네 관문 navigation 부활, 목적지 실패 보존, 실제 P39 bootstrap의
27열 admission, 기존25열 유지, TIMED/CONTACT 피해1회·3m 정점·1.2초 착지 검사를 추가했다.
기존 edge 낙사 검사의 timer 기대값은 실제5m 횡단으로 갱신했다.

통합 Release 빌드 후 실제 `Server/Bin/Release/Server.exe`를 headless로 실행했다.
`--kouku-object-overlap-contract-test`는 failures0, `--valtan-arena-support-contract-test`는
failures0으로 통과했다. 로그는 `out/kouku-object-overlap-20260923.log`와
`out/valtan-arena-support-20260923.log`다. 첫 실행에서 확인된 테스트 fixture의 CRLF 열 삽입
오류와 bounded 이동 검사에 잘못 남은 arena-exit 플래그를 교정한 뒤 overlap 전체를 다시 통과했다.
실제 소비자의 낙사·지지면 판정을 완화하지 않았다. Client/UI 실행·프로세스 종료·
저작/게시 데이터 교체는 이 담당 작업에서 하지 않았다.

## G03. 유지되는 경계

낙사면은 메커니즘 시작의 실제 지지/비행 기준 높이에 상대적이다. 새 맵 배치 collider나 wire 필드는
추가하지 않았다. 기존 발탄 지지면의 셀 절반 표본/대각선 모서리 및 non-walkable 착지 후 보행
문제는 해당 발탄 RESULT의 미완료 경계를 그대로 유지한다. 화면의 상승·낙사·부활 판정은 사용자 확인 대상이다.

사용자의 후속 변경으로1·3관문 모두 낙사를 금지한다. 강제 이동의 지지 높이를 검사하고
낮은 배경층으로 내려가는 step을 막는다. 이 두 관문에서는 void 자동 낙하도 시작하지 않는다.
기존 FALLING 상태가 들어오면 HP를 보존하여 검증된 현재 관문 시작점으로 복귀시킨다.

## G04. 해머·도넛·장판 후보와 실제 소비자 검증

저장 revision2234/SHA256 `34c2efa5e6064282a1cc3fd91b272e1e8bd64497c1a23a716d944da18fd2a956`에서
`out/KoukuCollider20260923/bugfix-hammer-rise/field-patch.json`와 같은 폴더의
`KoukuSaydonComposition.candidate.json`를 준비했다. 독립 후보는2235이며 원본은 쓰지 않았다.

P13의 .50~.53 및 P86 .15/.16, P87 .21~.23을 같은 WEAPON `b_rpct_01`의 중심에
반경3.5m·반높이1.25m Cylinder로 연결했다. 크기는 이전 검증 BOX의 긴 XZ 반길이3.5m와
반높이1.25m를 기준으로 잡은 초기 튜닝값이며 사용자 화면의 최종 크기 판정을 대신하지 않는다.
P86은1700/3533ms, P87은1566/2200/2866ms에100ms 판정을 사용한다. P13의 기존 판정 시간과
카드 뒤집기 presentation/Logic/resource29/30은 그대로 보존하고 중복 .54만 제거했다.

공유 피해 Result98을 바꾸지 않고 Result150을 만들어 P23/P79/P119/P122의 요청된12개
도넛·장판 contact에만 수직3m/1200ms/수평0 반응을 연결했다. P39의 기존 Logic72 fixedHits에도
같은 높이/시간을 넣었다. 공격 템플릿이 없는 다른 Albion 정의에 새 공격을 만들지는 않았다.

실제 후보 P13/P39/P86/P87/P23의 선택 draft projector와 canonical emitter가 통과했다.
각 gameplay.rows를 `LOSTARK_KOUKU_DRAFT_TEST_ROWS`로 넘겨 Release의
`--kouku-draft-contract-test`를 실행했고 모두 failures0이다. 준비/실행 로그는 위 후보 폴더의
`p<N>-preparation.log`, `p<N>-native.log`에 있다. P39 준비 admission은 `p39/admission.json`이다.

이 검사에서 기존 publisher/native parser가 BOSS_CURRENT bone track을 OBJECT_CONTACT에만
허용하여 플레이어 ENTER_AREA 피해를 막던 실제 연결 누락을 확인했다. 허용 범위를 ENTER_AREA까지
확장하고 기존 exact trigger clock·양 끝 key·identity baseline·회전/크기 고정 검증을 동일하게
적용했다. 기존 Server bone transform과 player contact 소비자를 그대로 사용한다. 일반 WORLD나
다른 Trigger 종류의 허용 범위를 넓히지 않았다.

통합 후보와 실제 원본 교체/publish는 root가 다른 field patch와 병합하여 별도 수행한다.
