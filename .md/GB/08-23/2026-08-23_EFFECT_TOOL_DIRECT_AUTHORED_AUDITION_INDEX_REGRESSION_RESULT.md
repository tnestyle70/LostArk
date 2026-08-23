# 2026-08-23 Effect Tool direct-authored audition index 회귀 수정 결과

기준 main: `f2878943e066eb7816fbd6f5f671910e38efe452`

작업 branch: `codex/effect-tool-direct-authored-audition-index-fix`

최종 화면 판정자: 사용자

## 결과

`F1 -> Effect Tool -> All Effects`에서 Valtan은 보이지만 모든 캐릭터 스킬이 `Saved 0`이 되고
Product cue의 Open Editor가 사라지던 회귀를 수정했다. Effect 데이터나 Product cue를 되돌리지 않았고,
direct-authored source index가 publisher의 registry-bound audition source shape를 읽도록 실제 소비자를
교정했다.

## 원인

- source `EffectCatalog.json`의 direct-authored 문서는 166개다.
- 기존 161개는 3필드 기본형 또는 screen overlay가 있는 4필드형이다.
- 대표 V1 5개는 audition identity quartet가 있는 7필드형이다.
- `CEffectDirectAuthoredSourceIndex::Build`는 3/4필드만 허용해 첫 V1 행
  `effect.artist.skill.31460.v1.unified`에서 catalog 전체를 malformed로 거부했다.
- 첫 refresh에는 보존할 이전 index가 없으므로 캐릭터 saved list가 비었고, 별도 runtime tree인 Valtan만
  계속 보였다.

## 구현

### source index

- 정확한 3/4/7/8필드 shape만 허용한다.
- audition metadata 네 필드는 all-or-none이다.
- `REGISTRY_BOUND_AUDITION_ONLY`, `PROJECT_TUNED_APPROX`, 서로 다른 source ID, 소문자 64자리
  SHA-256 형식을 검사한다.
- source ID는 같은 catalog의 non-audition direct-authored 행이어야 한다.
- Player skill, boss pattern, boss combat object에서 source와 audition target의 stable owner가 같아야 한다.
- shape가 잘못되면 catalog transaction 전체가 fail-closed하며 이전 index를 보존한다. owner가 다른
  개별 audition 행은 unavailable로 격리한다.

### 실행 회귀

`EffectRenderContractHarness`가 production `Effect_DirectAuthoredSourceIndex.cpp`를 직접 링크한다.
실제 source catalog와 Authored root를 사용해 다음 V0/V1 10개가 정확한 PLAYER_SKILL owner로 모두
admission되는지 검사한다.

- Artist A `31460`: V0/V1
- Dimension Master R `2050180`: V0/V1
- Lance Master D `34110`: V0/V1
- Warlord R `17110` clip2: V0/V1
- Warlord R `17110` clip3: V0/V1

부분 audition metadata fixture는 Build 실패와 기존 index 무변경을 함께 검사한다.

## 자동 검증

- `git diff --check`: PASS
- harness project/filter XML parse: PASS
- canonical `Publish-Effects.ps1 -Mode Validate`: PASS
  - catalog entries `156`
  - material-program bindings `171`
  - registry-bound audition effects `5`
- EffectRenderContractHarness x64 Debug build/run: PASS
  - direct-authored index regression: PASS
  - actual binding count `171/171`
  - representative V1 registry bindings `131`
  - representative V1 actual draws `6`
  - total compiled-adapter actual draws `8`
- EffectRenderContractHarness x64 Release build/run: PASS
  - Debug와 같은 index/binding/draw 계약 확인

## 사용자 화면 검증

PR 병합과 정본 Client build 뒤 사용자가 `C:\Users\user\Desktop\LostArk\Framework.sln`에서
x64 Debug `Server + Client`를 `Ctrl+F5`로 실행한다.

1. `F1 -> Effect Tool -> All Effects`를 연다.
2. class selector에서 `Dimension Master`를 선택한다.
3. `Skill | Input R | ... | #2050180`이 `Saved 2`인지 확인한다.
4. `Saved Unified Effects` 아래에서 다음 두 문서를 각각 펼쳐 `Open Saved Effect`와
   `Play Saved Effect`를 확인한다.
   - `effect.dimensionmaster.skill.2050180.unified`
   - `effect.dimensionmaster.skill.2050180.v1.unified`
5. Product cue `pc_sp_m_00_sk_sk_foldcut`를 펼쳤을 때
   `Editable Unified Effect | effect.dimensionmaster.skill.2050180.unified`가 보이고,
   `this Product cue has no exact direct-authored document` 오류가 사라졌는지 확인한다.
6. 대표 행도 확인한다.
   - Artist A `#31460`: `Saved 2`
   - Lance Master D `#34110`: `Saved 2`
   - Warlord R `#17110`: clip2/clip3 V0/V1 합계 `Saved 4`
7. Valtan selector와 pattern tree가 계속 표시되는지 회귀 확인한다.

에이전트는 Client/UI를 실행하지 않았고 화면 PASS를 선언하지 않는다.
