# 2026-08-28 Valtan Product Effect unlink transaction 결과

## 완료 상태

`VALTAN_FOUR_SLASH / RECOVERY`의 아래 Product 연결 하나를 제거했다.

```text
cue.valtan.carrier-v1.attack.four-slash.recovery.clip-01
-> effect.valtan.carrier-v1.attack.four-slash.recovery.clip-01
```

- split 정본 `Data/Valtan/Valtan.presentation.json`: RECOVERY `effectCues`가 빈 배열이다.
- generated `Valtan.patterneffectcues.json`: 동일 binding 한 행만 제거됐다.
- FOUR_SLASH의 `SLASHES / active.clip-01`, `SPIN / active.clip-02`는 유지됐다.
- `VALTAN_SEQUENCE_FOUR`가 공유하는 active clip-01 연결도 유지됐다.
- EffectCatalog 행과 recovery authored Effect 파일은 보존됐다.
- 사용자가 Effect Tool에서 저장한 recovery authored Effect 내부 변경은 되돌리지 않았다.

## 구조 수정

- unlink publisher 순서를 `PublishV2 -> ValidateV2`로 교정했다.
- live managed cue 집합이 migration policy ledger의 부분집합일 수 있게 했다.
- sealed legacy Effect cue 검증을 ordinal에서 stable `bindingId` 조회로 바꿨다.
- current split에서 unlink된 cue를 V1 migration fixture가 다시 만들지 않게 했다.
- Effect Tool의 기존 성공 경로는 child 종료 후 `Refresh_ValtanPatternTree()`와 `Refresh_DataFiles()`를 다시
  호출하므로, 이후 UI Delete도 publish 성공 뒤 새 연결 상태를 재파싱한다.

## 검증 결과

- Product unlink tests: 7/7 PASS
- All Effects contract: 17/17 PASS
- Effect Tool saved rows: 32 tests PASS, 7 skipped
- Valtan pattern master v2: 50/50 PASS
- Valtan pattern tree: 21/21 PASS
- Effect source validation: 196 direct sources, generated artifact 0, PASS
- PublishV2 재실행: `changed=0`, artifacts 7
- ValidateV2: `ok=true`, errors 0, projected artifacts 9
- `git diff --check`: PASS
- unlink stage/backup 및 child process 잔존 없음

## 수동 확인 경계

실행 중 Client에서는 All Effects의 `Refresh`를 누르거나 새 Client를 시작해야 FOUR_SLASH가 Effect 2개로
표시된다. 실제 Server 재생은 Server 재시작 후 Valtan Arena에 다시 진입해 사용자가 판정한다.
