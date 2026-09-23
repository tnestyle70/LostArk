# Gameplay profile 반복 로드와 실패 보존 결과

## 실제 결함

out/ReleaseRaidTools20260924/debug-kouku-support-surface.log에서 첫 supplemental catalog admission은 통과하고 두 번째부터 Skill buff row is duplicated가 발생했다. 같은 파일을 다른 객체에 처음 읽는 경로는 정상이고 두 테스트가 공통으로 같은 CGameplayCatalog 객체를 반복해서 로드한다. fixture에 buff를 추가한 것이 아니다.

PR454의 m_SkillBuffsBySkillId, m_SkillBuffById, m_DamageProfileById가 기존 Load_BootstrapBytes의 clear/rollback에서 빠졌다. 따라서 두 번째 load는 이전 buff와 충돌하고 damage formula emplace는 이전 값을 남길 수 있다. m_EmberProfiles는 clear만 하여 실패 시 이전 profile을 복원하지 않았다. 실제 재로드 소비자의 문제이며 validator 완화로 우회하지 않았다.

## 수정

기존 LOAD_ROLLBACK의 committed 플래그를 네 profile map과 공유한다. 이전 map을 이동 보관한 뒤 비운 map에 parse하고 모든 검증 성공 때만 새 값을 유지한다. 실패 시 네 map의 이전값을 함께 복원한다. 기존 SKILLBUFF 중복 검증과 authored 수치는 그대로다.

SupportSurface fixture의 무관한 연쇄 실패 뒤에는 이전에 성공한 Showtime pattern을 summon으로 사용하면서 빈 PatternSpawns.back()이 호출됐다. 준비 조건에 정확한 SUMMON_PATTERNS 종류 및 10개의 spawn을 포함해, 최초 admission 실패를 기록하고 추가 assert로 검사를 중단하지 않게 했다.

## 검증

- 같은 published Retail bootstrap을 같은 catalog에 두 번 Load, 변경된 damage formula/buff duration/ember gain의 정상 교체, late invalid row 및 실제 duplicate buff row의 이전 revision/네 map 보존, 실패 뒤 authored baseline 복귀 검사를 추가했다. Debug/Release 공통이다.
- 기존 overlap의 두 실패 기대값은 바꾸지 않았다. 첫 로드는 통과하고 rise 변형과 baseline 복귀가 실패하던 같은 재로드 원인이다.
- P81/P118 actual counter→groggy의 새 검사는 최초 Debug 로그에서 모두 PASS. 기존 큰 Debug 블록 밖에 독립 fixture로 이동하여 Release도 같은 counter 검사를 실제 실행한다.
- 변경 C++ git diff --check, 전처리 균형, UTF-8/기존 CRLF 유지: PASS.
- Debug/Release Product: root 실행 PASS (`out/BuildPipeline/runs/20260923T224833539Z-debug-product.json`, `20260923T224850148Z-release-product.json`).
- 실제 Retail profile 반복 Load/변경값 교체/두 실패 rollback/baseline 복귀 공통 검사는 양 구성 모두 PASS. 최종 support-surface는 Debug 307 PASS / failures 0 (`out/ReleaseRaidTools20260924/debug-kouku-support-surface-verified.log`), Release 28 PASS / failures 0 (`release-kouku-support-surface.log`).
- Object-overlap: Debug 660 PASS / failures 0 (`debug-kouku-object-overlap-final.log`), Release 628 PASS / failures 0 (`release-kouku-object-overlap.log`).
- 중간 support-surface 실행의 counter WORLD cleanup 네 실패는 독립 fixture 이동 때 이전 support setup이 부여하던 run epoch 초기화를 누락한 원인이었다. 공통 counter room에 실제 admission과 같은 nonzero epoch 1을 추가했고 최종 양 구성 재컴파일/재실행에서 모두 통과했다. 기대값과 runtime은 변경하지 않았다.

Client/UI는 실행하지 않았다. Server native 검사는 통합 담당 root가 실행했다. 최초 support-surface의 후반 catalog 연쇄 실패와 assert는 위 수정 전 실패 이력이며, 최종 성공 근거는 verified 로그다.
