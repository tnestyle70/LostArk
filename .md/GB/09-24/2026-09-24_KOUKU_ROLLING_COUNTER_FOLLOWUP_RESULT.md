# 쿠크 공굴리기 카운터 후속 연결 결과

## 완료한 변경

현재 source의 P81(1관문 세이튼)과 P118(3관문 쿠크세이튼)은 logic.85 COUNTER_WINDOW가 endsPatternOnSuccess=true인데 occurrence의 onSuccessLogicIds가 모두 비어 있었다. Server는 카운터 성공을 정상 판정하고 원래 패턴을 종료하지만 삽입할 후속이 없으므로 다음 flow의 플레이어 추적으로 넘어갔다. Client 표시 문제가 아니라 authored 성공 간선 누락이다.

Composition revision 2236→2237에서 P81.logic.1에 기존 logic.7→P4를, P118.logic.1에 기존 logic.57→P42를 연결했다. 두 후속은 각각 같은 관문·boss placement의 기존 무력화 start/loop/end이며 1167+1667+1333=4167ms다. 기존 RESULT/FOLLOWUP_PATTERN → Apply_KoukuLogicOutput → member scheduler 경로를 사용하며 보편적인 빈 결과를 임의 무력화로 바꾸지 않았다. 원래 다음 pursuit와 그 transition은 유지한다.

원본 backup은 out/KoukuRollingCounter20260924/Composition.before.json이다. 최신 hash 확인 후 stable occurrence ID의 두 필드와 revision만 원자적으로 교체했고, 의미 비교로 나머지 field 보존을 확인했다. 생성물은 root publisher가 source revision 2237로 게시했고 최종 native 검사는 해당 published runtime을 소비했다.

## 검증

- JSON parse 및 두 성공 간선의 stable ID 확인: PASS.
- 의미 비교에서 두 occurrence field와 revision 이외 동일: PASS.
- 변경 C++/JSON git diff --check: PASS.
- ServerGameplayContractTests_KoukuSupportSurface.cpp에 실제 published P81/P118 COUNTER_WINDOW를 읽는 검사를 추가했다. 같은 boss/gate, 실제 counter verdict, 한 tick 뒤 기존 groggy 삽입, 원래 pursuit 보존, 공 WORLD owner만 종료, 무력화 세 단계 4167ms 완료를 검증한다. 빈 결과 counter의 기존 정상 종료 검사도 유지한다.
- Debug/Release Product: PASS (`out/BuildPipeline/runs/20260923T224833539Z-debug-product.json`, `20260923T224850148Z-release-product.json`).
- 최종 `--kouku-support-surface-contract-test`: Debug 307 PASS / failures 0 (`out/ReleaseRaidTools20260924/debug-kouku-support-surface-verified.log`), Release 28 PASS / failures 0 (`release-kouku-support-surface.log`). published P81/P118의 성공 판정·groggy 삽입·pursuit 보존·공 WORLD owner 정리·4167ms 완료 모두 통과했다.
- 공통 fixture의 run epoch 초기화 누락은 실제 admission과 같은 epoch 1 초기화로 보정했고 양 구성에서 재검증했다. 소스의 성공 간선과 runtime은 추가 변경하지 않았다.

## 남은 확인

Client/UI는 실행하지 않았다. 사용자 화면에서 공굴리기 counter 성공 직후 무력화 시작/유지/복귀를 확인해야 한다. build/native 성공과 실제 화면 확인을 구분한다.

## 통합 검증 중 재로드 결함 분리

최초 Debug 실행에서 P81/P118의 실제 counter, 후속 삽입, 공 종료, 4167ms 무력화 완료 검사는 모두 PASS했다. 이후 같은 실행의 supplemental catalog 재로드가 실패하여 전체 실행은 미완료였다. 원인과 수정은 [Gameplay profile 재로드 결과](2026-09-24_GAMEPLAY_PROFILE_RELOAD_TRANSACTION_RESULT.md)에 기록한다. counter 검사는 Debug 전용 블록 밖으로 이동하여 Release에서도 같은 실제 source를 검사한다. 최종 양 구성 전체 실행은 위 verified 로그에서 failures 0으로 완료했다.

## 공통 fixture 보정

독립 counter room을 만들면서 이전 Debug support room의 iRoomAuditionEpoch=1 초기화를 함께 옮기지 않았다. 각 WORLD cue는 run epoch 1인데 room epoch는 0이어서 Stop_KoukuWorldOwner의 미승인 run 거절에 걸렸다. 실제 Evaluate_KoukuSaydonPatternAudition은 staged run에 nonzero epoch를 발급하고 cleanup은 양 구성 공통이다. fixture만 같은 admission 상태로 맞추며 공·sibling·pending mechanic 검증을 그대로 유지했다.
