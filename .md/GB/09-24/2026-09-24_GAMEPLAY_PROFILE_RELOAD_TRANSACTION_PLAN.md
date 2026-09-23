# Gameplay profile 반복 로드와 실패 보존 계획

## G00 — 실측 원인

Retail 게시 후 같은 CGameplayCatalog에 두 번째 Load를 수행하면 Skill buff row is duplicated로 실패한다. 첫 load와 실제 공굴리기 counter 검사는 정상이다. parser는 기존 map을 LOAD_ROLLBACK으로 비우고 복원하지만 PR454의 buff-by-skill, buff-by-ID, damage formula map이 대상에 빠졌다. Ember map은 clear만 있어서 실패 시 이전 profile이 사라질 수 있다.

## G01 — 공통 transaction 보정

Load_BootstrapBytes의 기존 rollback.committed를 네 map의 수명과 공유한다. 다음 load 시작 시 이전 값은 stage 밖에 보관하고 비운 map으로 parse한다. 성공 시 새 map을 유지하고 어떤 parse/validation 실패라도 네 map을 함께 복원한다. 중복 row 거절이나 실제 수치는 변경하지 않는다.

## G02 — 검증과 테스트 실패 격리

기존 support-surface catalog fixture에서 같은 byte 재로드, 변경된 damage/buff 소비, late invalid row 뒤 revision과 이전 profile 유지, 이전 baseline 복귀를 검증한다. summon fixture는 admission 실패 시 빈 PatternSpawns.back()을 호출하지 않도록 readiness에 실제 열 개 spawn 계약을 포함한다. overlap의 두 실패도 같은 두 번째 load 원인이므로 기대값은 유지한다. root가 Debug/Release Product 및 관련 native CLI를 재실행한다. 신규 C++ 파일은 없다.
