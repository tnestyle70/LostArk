# Character Select Effect 준비와 All Effects 목록 복구 결과

완료일: 2026-08-17

기준: `origin/main` `d176c6e8`, `codex/effect-binary-preload-all-effects`

## 완료 상태

- All Effects의 Saved Unified 목록 정본을 폐기된 FourClass Track-A batch/runtime Product tree에서 source
  `EffectCatalog.json`의 `DIRECT_AUTHORED_DOCUMENT_V13` row로 교체했다.
- production source-index helper가 stable Effect ID를 `PlayerSkills.json` owner와 join하고 canonical
  Authored path/physical scan을 transactionally 검증한다. 상위 catalog 오류와 중복은 이전 index를
  보존하고, owner/path/scan 단일 row 오류는 그 row만 격리한다.
- 목록 렌더와 Source Presets는 cache-only document/projection lookup을 사용한다. JSON decode는 사용자가
  특정 Saved Effect의 Open 또는 Play를 누를 때만 수행한다.
- Character Select Loading은 선택 class cue를 queue 앞에 등록하고 선택 target과 기존 background queue가
  모두 terminal이 된 뒤에만 activation을 요청한다. registration 실패도 current catalog revision과 global
  queue drain을 확인한 뒤에만 fail-open한다.
- Character Select 내부 Server 승인 class 변경도 최신 local snapshot generation을 stage하고 새 class Effect
  준비/global drain 뒤에 character replacement를 commit한다. 준비 중 입력은 막고 기존 presentation을
  유지하며 replacement 실패는 Lobby 복귀로 bounded 처리한다.
- Product prepared record가 immutable catalog document를 shared ownership한다. Playback/Renderer attach는
  exact revision/document/projection identity를 사용해 전체 document copy와 Product signature 재검증을
  반복하지 않는다. revision 0 Tool/edit stage의 owned-copy 계약은 유지한다.

## 실측 결과

- source direct-authored unified: 101개
- PlayerSkills owner join: 101/101
- runtime Product membership: 99개
- Lance loading cue: 41개, unique 41개, 현재 null-token admission 41개
- invisible prune은 source/runtime identity를 삭제하지 않았으며 목록 공백의 원인이 아니었다.

Winters의 `.wfx`는 binary가 아니라 text JSON loader다. 현재 Product spawn도 `Find_Loaded()`만 사용하므로
새 binary runtime은 추가하지 않았다. 이번 변경은 실제 interactive overlap과 spawn document copy를 먼저
제거했다. DDS/WModel/D3D 생성 비용은 binary로 없어지지 않는다.

## 자동 검증

- `ClientFrontendHarness` x64 Debug build: PASS
- `--effect-loading-prewarm-fast`: 8 PASS, failures 0
  - queue yield/coalesce/failure/revision
  - priority 및 normal/registration-failure global drain gate
  - production All Effects source index 101개와 unsafe row/rollback
  - Lance cue 41개와 runtime membership 99개/cache-only lookup
  - immutable prepared attach, foreign identity/stale handle 거부, incremental rollback
- Client x64 Debug build: PASS, 0 errors
- Client x64 Release build: PASS, 0 errors
- `Publish-Effects.ps1 -Mode Validate`: PASS, catalog 99개/visual programs 13개
- project/filter XML parse: PASS
- `git diff --check`: PASS

## 남은 수동 경계

에이전트는 Client/UI를 실행하지 않았다. 사용자가 Client를 `Ctrl+F5`로 실행해 다음을 확인한다.

1. Lobby에서 Character Select 진입 후 Lance LMB 첫 사용
2. Character Select 안에서 다른 class 선택 후 준비 상태와 첫 스킬
3. F1 Effect Tool의 All Effects Saved Unified 목록, 특정 row Open/Play
4. 1 client와 4 client의 Loading 체감

한 target 내부의 JSON/validation/GPU 준비는 아직 한 main-thread frame 단위다. 따라서 큰 문서는 Loading
화면 한 frame을 길게 만들 수 있지만 Character Select의 클릭 가능한 frame과는 겹치지 않는다. 네 Client
process는 OS file cache 외에는 Effect/GPU cache를 공유하지 않는다.
