# Boss → Effect exact Product 통합 결과

## 실제 구현

Boss Verification의 Pattern/Stage/Product cue 선택은 stable
`patternId/stageId/cueOccurrenceId/effectAssetId` 요청을 MainApp이 한 번 소비해
Effect Tool로 전달한다. 현재 Product tree와 직접 source index에서 exact tuple을
재검증한 뒤 기존 문서/preview 경로를 사용한다. unlink 작업이 진행 중이면 새 진입점도
기존 Open/Refresh와 동일하게 거부한다.

clip-bound는 전체 Pattern timeline의 t=0 pause, `STAGE_CLOCK`은 static Valtan
target으로 연결한다. 자동 재생이나 Server command는 없다. 미저장 문서는 기존
Save/Discard/Cancel을 거친다. source 문서와 사용자29-slot Flow는 그대로 보존했다.

## 자동 검증

- Boss/Effect/Flow/PatternTree focused suite: 115 tests, OK, 7 skipped.
- Valtan V2 전체 suite: 51 tests, OK.
- direct Effect source validator: 196 sources / 1028 resources, PASS.
- 97 world members / 135 unique placements와 삭제된 recovery cue의 migration 비복원: PASS.
- live Flow schema/admission과 초기28-slot in-memory seed fixture를 분리했다.
  사용자가 정상 Save한 순서를 테스트가 초기 순서로 강제하지 않는다.
- 해당 6개 변경 파일 `git diff --check`: PASS.

통합 C++ Debug/Release 빌드와 추가 세션 테스트의 최종 결과는
`2026-08-28_VALTAN_BERN_PARTY_INTEGRATION_RESULT.md`에 기록한다.

## 사용자 수동 검증

사용 경로는 `F1 → Boss Tool → Boss Verification → Pattern → Stage → Edit Linked
Effect`다. 열린 Model View Timeline에서 직접 `Play` 또는 `Restart + Play`를 누른다.
화면 재생·Effect fidelity·Save 후 육안 확인은 에이전트가 수행하거나 PASS로 기록하지 않았다.
