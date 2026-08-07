# 2026-08-07 차원술사 복원 워크벤치 결과서

## 1. 결론

차원술사 복원 방향을 다음 하이브리드 계약으로 확정했다.

```text
UPK/Cascade 자동 추출 baseline
→ 실제 Mesh/Texture/Decal/Trail/Particle/시간/Transform 보존
→ Effect Detail에서 finite Material profile과 named 값 튜닝
→ Authored Save/Reload
→ 별도 Assembly/WFX/Runtime Catalog publish
→ 원작 PNG 고정 조건 A/B
```

581 Particle를 하나씩 다시 만드는 방향이 아니다. 자동 추출 문서를 초안으로 유지하고,
화면을 결정하는 큰 visual layer와 Parent Material family를 우선 복원한다.

## 2. 구현 완료

### 2.1 복원 작업 화면

- `Effect Detail` 상단 `Restoration Session`에서 `Apply + Save Authored`,
  `Save Authored`, `Reload Saved`, `Restart Preview`를 한 흐름으로 제공한다.
- draft/Document/save/live preview 상태를 분리한다.
- non-drawable partial draft는 저장할 수 있지만 preview hidden과 publish blocked 이유를
  표시한다. 저장 성공을 GPU preview 성공으로 말하지 않는다.
- active skill은 현재 Authored tree를 먼저 표시하고 published Assembly/WFX는
  `Published Runtime Hierarchy (diagnostic)`로 분리한다.

### 2.2 Material 복원 편집

- Renderer가 지원하는 finite runtime shader profile만 선택할 수 있다.
- Imported에서 복사된 named scalar/vector/static switch 값을 Authored에서 편집한다.
- Dynamic Parameter semantic과 SubUV mode도 등록된 token 안에서 편집한다.
- 값을 바꾸면 `RECONSTRUCTED_PROFILE`로 표시하며 runtime exact를 주장하지 않는다.
- Parent/source Material identity와 parameter name/group은 provenance로 읽기 전용이다.
- Mesh/Base/Noise/Mask/Emissive/Dissolve는 기존 typed resource binding 경로를 유지한다.

### 2.3 저장과 동시 작업 보호

- unique temporary/backup 경로로 저장한다.
- 임시 파일 parse 뒤 전체 canonical serialize가 입력과 같은지 확인한다.
- Authored load 시 canonical baseline을 기억하고 저장 직전 disk 내용과 비교한다.
- 다른 세션/promotion이 파일을 바꾸면 stale save를 거부하고 외부 파일을 보존한다.
- New/Save As 대상이 세션 도중 생겨도 덮지 않는다.
- base11 자동 promotion은 직전 committed promotion receipt의 `promotedSha256`와 현재
  Authored SHA가 다르면 수동 override로 판단하고 시작 전에 거부한다.

### 2.4 Reference A/B 보조

- Screen Post를 Authored 변경 없이 preview에서 ON/OFF할 수 있다.
- Active Effect ID, Sample Time, selected emitter, Screen Post 상태를 한 줄에 표시한다.
- `Copy A/B Metadata`는 class와 pivot까지 복사한다.
- 카메라 transform/FOV/resolution 고정은 수동 캡처 절차로 남는다.

## 3. 자동 검증

| 검증 | 결과 |
|---|---|
| DimensionMaster base11 promotion Python tests | PASS, 9 tests, exit 0 |
| Effect pipeline fixture | PASS, exit 0 |
| Effect Tool final audit | PASS, exit 0 |
| Client x64 Debug build | PASS, exit 0 |
| ClientFrontendHarness x64 Debug build | PASS, exit 0 |
| ClientFrontendHarness 실행 | PASS, failures 0, exit 0 |
| ProjectAudit | PASS, 77 checks, exit 0 |

빌드 로그에는 기존 소스 인코딩 경고와 ClientFrontendHarness post-build의
`pwsh.exe` PATH 경고가 있었지만 MSBuild exit는 0이고 생성된 하네스 실행도 exit 0이다.
이 세션이 만든 Client/ClientFrontendHarness/MSBuild 프로세스는 종료 후 모두 정리했다.

ClientFrontendHarness에서 추가로 확인한 계약:

- v12 partial SourceRecipe draft atomic Save/Reload
- stale writer save 거부와 외부 파일 보존
- Q `2050100` stable emitter
  `fx_pc_swp_00.par_j_swp_nailstrike_00_1.particlespriteemitter_8`의 named Material 값 변경,
  `RECONSTRUCTED_PROFILE`, Save/Reload canonical 동일성
- 모든 현재 DimensionMaster Authored stage
- Assembly/WFX/Runtime Catalog identity와 failed reload rollback

`Test-EffectToolFinal.ps1` 실측은 `runtime-exact material=0`이다. 이 값은 완료가 아니라
현재 strict Material exact가 하나도 없다는 명시적 미완료 증거다.

## 4. 수동 GPU A/B

수행하지 않았다.

사용자가 직접 최신 EXE를 검증하기로 했으므로 이 세션이 띄운 Client smoke는 중단하고
프로세스를 정리했다. 따라서 다음 항목은 PASS로 기록하지 않는다.

- Client/Default 시작 smoke
- Q emitter live GPU 편집 반영
- Screen Post OFF/ON 캡처
- 원작 PNG와 고정 카메라 pixel A/B
- dissolve/alpha/card boundary 제거 확인

## 5. 아직 미완료

- Material `RUNTIME_EXACT`는 0이다.
- Authored Save는 Assembly/WFX/Runtime Catalog publish 또는 hot reload가 아니다.
- Tool 내부의 staged candidate build → validate → atomic promote → runtime reload transaction은
  아직 없다.
- Parent Material graph topology가 cooked UPK에 남았는지 representative raw probe가 필요하다.
- topology가 없으면 Parent family별 finite HLSL profile과 A/B로 복원해야 한다.
- stable element field mask를 가진 override sidecar merge는 후속 정밀 병합 단계다.
- 실제 Q 원작 pixel 복원은 시작하지 않았다. 이번 Q 검증은 authoring 데이터 보존 검증이다.
- 기존 `차원술사_S00~S06`은 2050550 자료이며 현재 S 2050220 A/B 근거가 아니다.
- 원작 PNG에는 effect ID/sample/camera/FOV/emitter/post를 묶은 manifest가 아직 없다.

## 6. 다음 세션 시작 순서

1. 현재 dirty worktree와 이 PLAN/RESULT를 먼저 다시 확인하고 다른 변경을 보존한다.
2. 사용자가 `Client/Default`에서 수행한 최신 EXE/Effect Tool 결과를 받는다.
3. Q 2050100의 위 stable emitter를 Solo하고 Screen Post OFF, 고정 카메라로 캡처한다.
4. blackline aura/local crack/glow의 큰 silhouette부터 finite Material 값과 typed resource를
   조정한다.
5. Apply + Save → Reload 값 동일성을 확인한다.
6. 기존 pipeline으로 Q candidate Assembly/WFX/catalog를 staging 검증한다. Client에서 현재
   publisher를 직접 실행하지 않는다.
7. 같은 Parent의 다른 Q layer와 다른 스킬이 함께 개선되는지 A/B한다.
8. Q checkpoint 뒤 S SpriteWave, W post 분리, T 핵심 4 Parent, E model material 순서로 간다.

## 7. 작업 트리 경계

대규모 dirty working tree에 다른 담당 변경이 함께 있다. 이 세션은 stage, commit, revert,
전체 정리를 하지 않았다. 빌드 산출물과 `.codex_tmp` audit report도 stage하지 않는다.
