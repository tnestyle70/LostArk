# G01 스킬 선택 필드 호환성 / G02 Flow 순서 복구

## 실제 반영

- BalanceTool의 strict skill 객체에 rootMotionScale만 선택 필드로 허용했다.
- 존재하면 finite, 0 < scale <= 8, scale != 1을 Reload/ValidateDraft에서 검사한다.
- SKILL_EDIT의 optional로 존재 여부와 값을 보존하고 serializer 및 기존 round-trip
  진단에도 연결했다. 플레이어 스킬 원본은 수정하지 않았다.
- 오류에 skillId를 포함한다. Save Flow의 기존 원자적 저장/충돌 검사를 우회하지 않는다.
- 사용자가 제공한 214802/214813 두 스크린샷에서 70개 순서와 69개 대기를 복구했다.
  실제 gameplay scriptedSequence와 생성 PatternRotations에 기존 파이프라인으로 저장했다.
  33~36행 뒤 100ms, 62~68행 뒤 0ms, 나머지 1000ms이며 마지막은 End다.

## 실행한 검증

- VALIDATE_DRAFT ok=true / operationCount=1.
- COMMIT_CANONICAL_DRAFT ok=true / changedCount=3 / runtimeActivation=NOT_ACTIVATED.
- 저장된 gameplay sequence의 모든 필드를 recovery patch와 대조: 일치.
- 원본 스킬 114개 중 5개의 rootMotionScale=1.5 유지.
- focused optional field source contract unittest 1개 통과.
- valtan_tuning_pipeline.py validate: ok=true / errors=[].
- 독립 코드 검토: publisher 범위 일치와 필드 부재 보존 확인. 화면/실행 판정 아님.
- Debug Product 빌드: Engine/Shared/Server/Client 성공, Client 13개 OBJ 및 EXE 링크.
  out/BuildPipeline/runs/20260922T125350853Z-debug-product.json.
  Client.exe 로컬 시각 2026-09-22 21:53:48, 67742208 bytes.
- git diff --check 통과. 기존 C4819/LNK4099 경고는 남아 있다.

## 생성물과 미완료 경계

정본 저장 파이프라인이 gameplay, PatternRotations와 Valtan.rootmotion 생성물을 갱신했다.
rootmotion은 현재 발탄 정본/클립 기준 재투영 결과이며 수작업으로 이동 키를 수정하지 않았다.
전체 Gameplay bootstrap 게시 및 실행 중 Server 활성화는 수행하지 않았다.
발탄 파일 저장 성공과 서버 적용 성공을 혼동하지 않는다.
현재 실행 중이던 Client는 사용자가 종료했고 에이전트가 UI를 실행/조작하지 않았다.
실제 새 Client Reload/Save Flow 확인은 사용자 검증 대기다.
기존 round-trip native harness 전체는 실행하지 않았다. 빌드 및 focused source 검증과 구분한다.
발탄 전용 저장 소유자 분리, 자동 draft 백업은 이번에 구현하지 않았다.
Git commit/push는 하지 않았다.

## 사용자 확인

새 Debug Client로 발탄 진입 → F1 → 기존 Boss Tool의 Pattern Flow.
70 Patterns, 첫 발탄 등장 컷신, 마지막 3페이즈 발탄 사망과 Wait를 확인한다.
저장본을 읽으므로 순서를 다시 입력할 필요가 없다.
순서를 추가 편집한 뒤 Save Flow에서 저장 상태/게시 상태를 구분하여 확인한다.
