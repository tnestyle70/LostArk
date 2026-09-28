# 발탄 Play Pattern의 마지막 동작과 이펙트 누락 수정 계획

## G00. 현재 증상과 조사 범위

`6방향 후 전멸`은 Preview에서 마지막 내려찍기와 이펙트가 보이지만 Play Pattern에서는
전멸 피해만 적용되고 표현이 누락된다. `점프 후 플레이어 추적 도끼`도 Preview의 빨간
장판과 마지막 폭발이 Play Pattern에서 보이지 않는다. 현재 저작본, 게시본, 실행 시 고정한
generation 및 Client의 실제 소비자를 대조해 Product 승격 누락과 실행 수명 문제를 구분한다.

## G01. 전멸 이후 마지막 stage 완주

현재 `VALTAN_FLOOR_WIPE_130`은 SECOND_SMASH 시작에 피해를 적용하고 500ms 동작 뒤
1500ms RECOVERY로 이어진다. 이펙트의 시작은 99ms와 101ms다. `ValtanBrain.cpp`는
정식 순차 실행만 타겟 사망 이후 완주를 허용해 stable-ID Play Pattern은 다음 tick에
NO_VALID_TARGET로 종료될 수 있다. 같은 패턴에서 마지막 피해가 확정된 이후의 완주를
실행 진입 방식과 무관하게 보존한다. HIGH_JUMP도 LAND201ms 최종 타격 이후 같은 조건으로
LAND3200ms와 RECOVERY400ms를 완주한다. 피해 이전 타겟 소실, 모든 플레이어 퇴장과 명시 Stop은
기존 종료 경계를 유지한다. 피해 시각과 사용자 애니메이션·이펙트 데이터는 변경하지 않는다.

기존 `ServerGameplayContractTests_ValtanResetlessNext.cpp`의 실제 명령과 room tick 검증에
마지막 플레이어가 전멸 피해로 죽는 경우를 추가한다. SECOND_SMASH와 RECOVERY의
수명, snapshot action 및 COMPLETED lifecycle을 검사해 단순 생존 fixture의 누락을 보완한다.

## G02. 추적 도끼와 게시 상태

현재 게시 generation의 171개 artifact는 디스크 저장본과 hash가 모두 같다. HIGH_JUMP의
AIRBORNE 경고와 LAND 폭발도 Product에 존재한다. 두 cue는 root/follow이며 Server는
발탄을 9m 올린다. 경고의 decal depth는 6m다. 폭발 source2240ms는 offset2194ms와
cue155ms에 의해 LAND201ms에 발생하지만 실제 지상 도착은267ms다. snapshot attachment
요소는 생성 시 공중 위치를 고정한다. Preview는 같은 Server 이동을 적용하지 않는다.

기존 target snapshot은 매 snapshot 현재 플레이어 위치이므로 시작 때 고정한 실제 착지점과
다르다. Shared의 `PATTERN_LANDING_SNAPSHOT { isValid, fPositionX/Y/Z }`를 기존
WORLD_ENTITY_SNAPSHOT에 추가하고 Server가 active leap의 fLeapLandingXYZ를 복제한다.
codec·finite/state 검사와 protocol version을 함께 갱신한다. Client는 같은 pattern sequence의
착지 pose를 기존 V1 world-root 경로에서 소비한다. 새 renderer나 Client 착지 추측은 없다.

명시적인 `pattern.landing.snapshot`/snapshot anchor만 추가한다. 저작 validator, Python
projection, Product parser, Workbench 선택과 Preview에 동일 계약을 연결한다. Preview는
기존 편집용 착지 기준을 소비한다. 경고와 LAND 두 cue의 anchor/follow만 바꾸는 후보를
별도 준비·검증하고, 편집 중 데이터의 최종 교체는 저장 기준 확인 후 수행한다.
사용자가 착지 수정 대상을 추적 도끼로 정정했으므로 SIX_PIZZA의 착지 STEP_03은 기존
root/follow를 보존한다. 월드 착지점 변경은 HIGH_JUMP의 두 cue에만 적용한다.

## G02-1. 중앙 피자 장판

SIX_PIZZA_106의 STEP_01 경고는 11초에 표시하지만 3.4초의 STEP_04 포효 컷씬에서
`Set_CinematicPresentationSuppressed`가 Product의 `Stop_BossOwner`를 호출해 먼저 삭제한다.
Preview는 삭제 대신 숨김을 사용해 이 경고가 남는다. `bPreserveBossActionTail`로 식별한
명시적 유한 cue만 Product 컷씬 동안 숨기고 기존 시계를 유지하며 끝나면 표시를 복구한다.
일반 몸체 이펙트는 기존 정리 경계를 유지하고 명시 Stop·owner 정리는 모두 종료한다.

Preview의 약0.7초 공백은 원본 세 notify의 개별 수명을 옮긴 결과다. 사용자가 무공백 표시를
명시했으므로 첫 표시11초부터 현재 cue의 종료19.033초까지 노랑·빨강·중앙 장판을 연속으로
유지하는 후보를 준비한다. 색·형태·크기와 실제 공격 시점은 보존하며 중복 notify는 겹쳐
생성하지 않는다. 원본 시각이 현재 제품의 공백을 요구하는 근거인 것처럼 설명하지 않는다.

3시와9시 지형 파괴의 COMBO_STEP_10에는 같은 source-warnings를 참조하는 별도 V1 cue가
명시적으로 연결돼 있다. 사용자 요청으로 `cue.valtan.terrain-3.combo.warning`과
`cue.valtan.terrain-9.combo.warning`만 제거한다. 다른 V1 타격·지형 파괴 cue는 보존한다.

## G03. 사용자가 저장한 마지막 이펙트와 카운터

최신 Save는 TRIPLE_COUNTER의 FAIL_3에 추가한 STAGE_CLOCK891ms의
`cue.valtan.composition.valtan_triple_counter.fail_3.01`이다. source-warnings와 착지 cue를
최종 병합하기 직전에 다시 읽은 최신 저장본에서 이 cue를 보존한다. 실제 게시 실패는 새
독립 cue와 무관하며 기존 FAIL_3 V2 smash-03의 저장 startMs1169와 템플릿900의 불일치다.
메모리에서 독립 cue만 제외해도 실패하고 기존 V2 시각만900으로 바꾸면 통과하는 것으로
원인을 분리했다. 사용자 저장값1169와891은 유지한다. 기존 occurrence allowlist에 exact
effectTimingOverride(templateEffect/bindingId/stageStartMs)를 지원해 해당 binding의
1169ms만 승인된 변경으로 검증한다. 범용 EFFECT waiver, template 전역 변경, 자동900복원은
하지 않는다. scope/resource/anchor/중복/다른 시각의 실패 검증을 보존한다.

COUNTER_1/2/3의 기존1800ms counter window와 COUNTER_HIT→GROGGY_FOLLOWUP은 유지한다.
`PlayerSkills.json`의 inputSlot Q/W/E/R로 카운터 방향 예외를 결정하고 실제 direct/projectile
hit까지 typed bool로 전달한다. 투사체는 생성 당시 값을 고정한다. 정확한 TRIPLE_COUNTER의
해당 세 stage에서만 앞180도 source 방향 검사를 생략하고 실제 명중·거리·window 검사는
유지한다. 다른 패턴과 다른 key의 기존 방향 계약은 보존한다. 기존 isCounterSuccess damage
event가 파란색 카운터 문구를 표시하며 Client에서 별도 성공 판정을 만들지 않는다.
후속 요청에 따라 TRASH의 STEP_07/RETRY_WINDUP_02/03도 포함한다. 이 패턴의 앞쪽 local
circle은 정면 최대 도달 거리를 유지한 방사형 범위로 검사한다. 두 패턴의 실제 COUNTER
guard도 같은 방향 예외를 받으며, 무피해 guard 성공은 Server의 counter-only damage event로
파란 카운터 문구를 전달한다. 일반 타격에 다른 key의 예외를 추가하지 않는다.

## G03-1. 추적 도끼 사운드의 지연과 조절

반복 추적 도끼는 Server HIT_PULSE에 연결한 combat-object sound cue를 사용한다. 현재
피해와 V2 visual hit의 기준은 모두1200ms이며 사운드는 수신 때 WAV의0ms부터 재생한다.
연결된 ProjExp1 네 WAV는 -48dB 첫 신호가 약61~312ms로 달라 원음 도입부만으로도
체감 지연이 달라진다. 투척 시작·반복 충격·본체 착지 cue를 구분해 변경 대상을 확인한다.

combat-object cue에도 기존 pattern sound와 같은 optional playbackOffsetMs(기본0)를
연결한다. Document의 parse/save, Product 검증, 실제 재생, 도구 편집과 미리듣기가 같은
값을 소비한다. WAV 앞부분을 건너뛰는 값이며 Server hit 시각을 변경하는 값으로 설명하지
않는다. 해당 도끼 cue만 우선50ms(60fps 기준3프레임) 보정하고 다른 cue와 원음은 보존한다.
변경 대상이 본체 pattern cue로 확인되면 이미 있는 pattern sound의 시간 편집 계약을 사용한다.
파서 roundtrip·누락 시0·잘못된 값 거부와 실제 소비 경로를 검증하며 청감 판정은 사용자가 한다.

## G03-2. 피자 추적 종료와 회전 속도

SIX_PIZZA 장판은 현재 cue 종료19.033초의1초 전인18.033초에 마지막 방향을 고정한다.
모아치기 CHARGE/CHARGE_2는 검격 STEP03 시작2.833초의1초 전인1.833초, 지형3/9는
COMBO_STEP11 시작20.183초의1초 전인19.183초를 기준으로 추적을 종료한다. 뒤 stage에
진입하며 같은 추적이 다시 켜지지 않게 한다. SIX 후반 별도 돌진 stage의 동작은 보존한다.

Showtime의 rotate-only 계약은 고정 각속도가 아니라 Shared `KoukuTargetTracking`의
남은 tick 기반 회전 비율이다. 이 helper를 기본 동작이 유지되는 선택적 배율로 확장하고
해당 발탄 추적만1/3을 사용한다. stage aim의 종료 시각·배율은 optional 데이터로 저장해
publisher→bootstrap→Server→Preview가 함께 소비한다. Client Product는 Server yaw를
사용하고 장판 cue도 종료1초 전의 world root를 고정한다. 타임라인 저장 뒤 검증에서
오래된 stage 범위가 통과하지 않도록 경계를 검사한다.

## G04. 검증과 완료 경계

최소 관련 Server/Client 검증과 정상 Debug Product 증분 컴파일을 수행한다. 새 C++ 파일은
현재 계획에 없어 프로젝트와 filters 등록이 필요 없다. 변경 JSON/XML이 있으면 parse하고
`git diff --check`를 확인한다. 기존 다른 작업과 바훈투르 무적 문구 변경을 보존한다.
Client/UI를 자동 실행하지 않으며 실제 화면 확인과 실행 중 프로세스의 갱신은 결과에 분리한다.
