# 2026-08-23 Effect Tool direct-authored audition index 회귀 수정 구현 계획서

기준 main: `f2878943e066eb7816fbd6f5f671910e38efe452`

작업 branch: `codex/effect-tool-direct-authored-audition-index-fix`

최종 화면 판정자: 사용자

## 목표

Debug F1 `Effect Tool -> All Effects`에서 캐릭터 스킬 전체가 `Saved 0`으로 표시되고 Product cue 옆
Open Editor가 사라지는 회귀를 수정한다. Valtan runtime tree, Product cue 재생, 기존 direct-authored
저장 계약은 바꾸지 않는다.

## 현재 실측

- `Data/Effects/EffectCatalog.json`에는 캐릭터 direct-authored 문서가 존재한다.
- direct-authored 166행 중 160행은 기본 3필드, 1행은 screen overlay 4필드, 대표 V1 5행은 audition
  identity를 포함한 7필드다.
- `CEffectDirectAuthoredSourceIndex::Build`는 direct-authored 행의 필드 수를 3 또는 4로만 허용한다.
- 첫 7필드 V1행에서 catalog 전체 refresh가 실패한다. 시작 시 보존할 이전 index가 없으므로 모든
  캐릭터 skill이 `Saved 0`이 된다.

## 변경

### `Client/Private/Effect_DirectAuthoredSourceIndex.cpp`

- 기존 3필드 direct-authored 행과 4필드 screen-overlay 행을 그대로 허용한다.
- publisher와 동일한 정확한 7필드 audition shape와 screen overlay가 함께 있는 8필드 shape를
  추가 허용한다.
- `runtimeAdmission=REGISTRY_BOUND_AUDITION_ONLY`,
  `fidelityClass=PROJECT_TUNED_APPROX`, 서로 다른 source Effect ID, 소문자 SHA-256 형식을 검증한다.
- source Effect ID는 같은 catalog의 non-audition direct-authored 문서여야 한다.
- Player/Boss owner가 source와 target에서 다르면 해당 행만 격리한다.
- 누락·부분 metadata·임의 필드는 계속 catalog-level fail-closed다.

### `Tools/EffectRenderContractHarness`

- production `Effect_DirectAuthoredSourceIndex.cpp`를 하네스에 등록한다.
- 대표 네 스킬의 V0/V1 10문서를 실제 `EffectCatalog.json`과 `Data/Effects/Authored`에서 stage한다.
- 10개 ID가 모두 PLAYER_SKILL owner로 source index에 admission되는지 Debug/Release에서 검사한다.

## 검증

1. JSON/XML parse와 `git diff --check`
2. `Publish-Effects.ps1 -Mode Validate`
3. EffectRenderContractHarness Debug/Release build와 실행
4. Client Debug/Release build
5. PR 병합 후 정본 `C:\Users\user\Desktop\LostArk`을 최신 main으로 fast-forward
6. 사용자가 Debug `Server + Client`로 `F1 -> Effect Tool -> All Effects`를 열어 캐릭터 Saved count와
   Product cue 옆 Open Editor를 확인

에이전트는 Client/UI를 실행하거나 화면 PASS를 선언하지 않는다.
