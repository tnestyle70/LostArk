# 발탄 4연속·추적 도끼 타격 판정 구현 계획

## G01. 현재 효과와 Server 타격 연결

사용자는 저장 후 Client를 종료했고 현재 저장본 기준 수정·적용을 승인했다. 다른 저작 필드와 Sound/Animation은 보존한다. Client/UI는 실행하지 않는다.

현재 FOUR_SLASH는 SLASHES 1790/2560/3330ms와 SPIN 600ms에 같은 CONE 9m/110도만 판정한다. SPIN의 1640ms 검격과 3000ms 원형 ring 효과에는 별도 판정이 없다. HIGH_JUMP LAND는 900ms의 500% 공격력 판정이며 사용자가 요청한 최대 HP 50%와 다르다. 그 DamageProfile은 개별 추적 도끼에도 공유되어 있으므로 전역 profile을 바꾸지 않는다.

현재 저장 효과를 기준으로 SLASHES ribbon 시작 1667/2221/3008ms, SPIN 기존 접촉 600ms 및 네 번째 ribbon 1640ms와 ring 3000ms를 연결한다. 기존 stage 길이·분기·Animation·Sound·Effect cue 값은 바꾸지 않는다. 실제 착지 cue는 sourceStart155ms/playbackOffset2194ms이고 최종 impact notify는2240ms이므로 LAND 접촉은201ms다. 큰 빨간 경고 원의 외경17.5m로 반경8.75m를 사용한다.

ring은 설치 fm_d_ring_008.wmodel의 XZ 반경40..79.88996, modelPreScale0.01, 두 mesh emitter StartSize8/9, cue worldScale1.5로 얻은 union을 gameplay 내경반경4.8m/외경반경10.8m로 정한다. 파티클 alpha나 GPU 표시 범위와 gameplay envelope는 구분한다.

## G02. Shared·Source·Product 계약

Shared/Public/Gameplay/AttackHitTemplate.h의 기존 ATTACK_HIT_TEMPLATE를 재사용한다. source hit.contacts와 product stage attackContacts는 optional이며, 없으면 기존 stage 타격 동작을 보존한다. 각 contact는 TIMED, repeatCount1, endMs0, repeatIntervalMs0이며 기존 hit schedule과 크기·순서·atMs가 일치해야 한다. 별도 timer나 runtime을 만들지 않는다. ACTIVE_WINDOW·CAPTURE·portal sweep과 조합하지 않는다.

Validate_StageAttackContacts는 lifetime과 각 schedule offset의 일치를 공통 검증한다. Python은 기존 Kouku combat_hit_templates.validate_hits를 import해 같은 primitive·damage·push 계약을 검증하며 source 원문을 수정하지 않는다. projection 시 기존 기본값으로 정규화한다. PowerShell은 기존 New-KoukuAttackHitRows를 재사용해 PATTERNATTACKHIT/STAGE row를 생성한다.

## G03. Server 소비

GameplayCatalog는 BOSS_PATTERN_STAGE_DEFINITION::AttackContacts를 소유한다. PATTERNATTACKHIT의 STAGE role은 stage action stable ID에 연결하며 기존 Kouku trigger role은 유지한다. 최종 카탈로그 검증은 schedule·duration·activation·player response를 함께 검사하고 실패 시 load를 거절한다.

ValtanBrain의 기존 due-hit loop가 contact index를 ApplyPatternHit에 전달한다. contact shape는 기존 ServerCombatGeometry와 CombatCollision으로 판정한다. MAX_HP_PERCENT는 쿠크처럼 플레이어 최대 HP에서 raw damage를 계산하고 defense/counter를 우회하되 기존 shield·invulnerability·damage replication을 사용한다. Shared forcePush/riseHeight/pushRange/pushMs를 ServerCombatHitRuntime로 전달한다. Boss origin과 stage-origin anchor는 기존 authority를 유지한다. 예측용 contact risk와 Client collider mirror도 같은 contact를 읽는다.

쿠크 KAKULSAYDON_G1_PATTERN_119 / logic137 노란 원·도넛은 maxHP10%, forcePush=true, pushRange1m, riseHeight2m, pushMs1200이다. FOUR_SLASH의 새 contact는 이 계약을 사용한다. HIGH_JUMP LAND만 maxHP50%를 사용한다. 개별 추적 도끼는 기존 profile을 보존한다.

## G04. Client 저장과 표시

ValtanPatternTree.cpp/.h의 source→master→view→save와 BalanceTool 재저장은 contact 값을 보존한다. VALTAN_STAGE_VIEW::AttackContacts는 읽기 전용 서버 판정 mirror다. current/selected stage collider 표시가 해당 contact shape를 소비하도록 연결한다. Client는 damage를 판정하지 않는다. 이 범위는 독립 agent가 맡고 동일 Shared schema를 사용한다.

## G05. 검증과 적용

out/ValtanContacts20260928/before와 baseline.json으로 기존 dirty 상태를 보존한다. 데이터는 stable pattern/stage와 hit 필드만 변경하고 적용 직전 hash를 확인한다. G14 exact receipt는 변경된 hit offset만 갱신하고 Sound/Animation 원문은 보존한다. 새 연락처 계약 malformed/repeat/window/schedule mismatch를 거절하는 Python·Shared 검증과 실제 ServerBrain fixture에서 circle/ring hole/outside/50%/push/legacy를 검사한다. Client roundtrip과 최소 TU compile은 담당 agent가 확인한다. 전체 Release build와 공식 domain publish·PR merge는 root가 실행한다. 화면 판정은 사용자가 한다.

새 C++ 파일은 추가하지 않으며 프로젝트 등록은 필요 없다. 결과 문서는 실제 실행한 검증만 기록한다.
