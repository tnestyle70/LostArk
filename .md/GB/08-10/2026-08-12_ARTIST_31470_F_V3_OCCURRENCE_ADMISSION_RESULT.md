# Artist 31470 F V3 occurrence admission 결과

## 결론

V3는 35개 원본 occurrence를 모두 보이게 하는 복원 방식에서, 증거와 사용자 승인이 닫힌 행만
합성하는 fail-closed admission 방식으로 전환했다. 현재 정본에는 user-approved source-exact 행이
없으므로 Complete 기본 출력은 `0/35`다. Effect Tool과 nonProduct Artist F 검토 경로는 원본 본체
역할을 판정하기 위한 `#9/#10/#11` 세 행만 노출한다. 이 세 행도 복원 성공이 아니라
`V3_MAIN_REVIEW`다.

현재 분류는 다음과 같다.

```text
ADMITTED_ONLY       0/35
V3_MAIN_REVIEW      3/35  (#9,#10,#11; CONDITIONAL의 부분집합)
CONDITIONAL_REVIEW 13/35  (#3,#4,#9,#10,#11,#16,#17,#22,#23,#30,#31,#32,#34)
DIAGNOSTIC_ONLY    22/35
ALL_DIAGNOSTIC     35/35  (inventory/원인 진단 전용)
```

## 구현 상태

- `CEffectReconstructedSourceRuntimeFactory::Build_Document`에 네 visual scope를 추가했다.
- 기본 scope는 `ADMITTED_ONLY`이며 현재 visible occurrence가 0이다.
- Artist 31470의 Character Select/Effect Tool nonProduct 준비 경로만 `V3_MAIN_REVIEW`를 명시한다.
- Effect Tool 문구를 `Restore Full35`에서 `V3 Admission / Main Review (3)`으로 바꾸고
  `0 admitted / 13 conditional / 22 diagnostic-only`를 표시한다.
- all-35 document는 하네스와 명시 진단 scope에서만 유지한다.
- reconstructed decal의 World-owned projection shader 상수를 `{1,1}, depth=1`로 고쳤다.
  source size/depth가 이미 World에 있으므로 단위를 다시 곱하지 않는다.
- RuntimeMaterialV2 texture provider index는 `SourceTextures[7]`뿐 아니라 sampler `[5]`도 함께
  bounds-check한다.
- 화면 결함 owner, 35행 geometry/material/reason, 공통 admission schema와 수동 제작 전환 조건은
  대응 implementation PLAN의 `2026-08-12 V3 최종 복원 시도` 절에 기록했다.

## 자동 검증

### PASS

- ClientFrontendHarness x64 Debug build
- `--effect-reconstructed-gpu-material`
  - 실제 WARP GPU, 77 DDS/SRV/sampler stage
  - production ObjectManager layer Add와 blank descriptor clone
  - Artist actual GPU draw와 Lance 연속 GPU draw
  - `0 admitted / 3 V3 main / 13 conditional` exact visibility
  - fixed-time main review packet:

```text
1.059s -> {}
1.670s -> {9,10,11}
2.177s -> {11}
2.610s -> {}
```

- `--effect-reconstructed-decal`
  - #20~#22 source size/depth/yaw exactly once
  - reconstructed shader projection unit constants
  - signed mirror 유지, singular/nonfinite World 거부
  - legacy authored decal size/depth 불변
- `Test-EffectToolFinal.ps1`
- `Test-EffectRuntimePrewarm.ps1`
- `Test-Artist31470RuntimeMaterialV2.ps1`
- `Test-EffectRuntimeAuthority.ps1`
- Client x64 Debug build/link
- focused 변경 파일 `git diff --check`

빌드에는 기존 EngineSDK/code-page와 DirectXTK PDB warning이 있었지만 compile/link exit는 0이었다.

### 전체 ProjectAudit

`Invoke-ProjectAudit.ps1`은 실행했으며 exit 1이다. PASS로 기록하지 않는다. 현재 공유 worktree에 남은
대표 실패는 다음과 같다.

- reconstructed material policy 분모의 이전 기대값 `255`와 현재 `260` 불일치
- material render-resource approval의 이전 Program SHA
- reconstructed render-resource authority unit test API drift
- derived artifact의 이전 candidate/sidecar byte count
- exact DDS deployment audit
- WModelGeometryContractHarness 미빌드
- legacy Product cue exact projection
- reconstructed runtime program 이전 admission/denominator 기대값
- Artist 31210 manifest/binding stage count mismatch
- quality-workbench bloom, point-light falloff 정적 계약

이 항목들은 V3 fixed-time visibility와 실제 GPU/ObjectManager PASS를 무효화하지 않지만, 전체 저장소
검증 완료를 의미하지도 않는다. 후속 정본 동기화 단위에서 별도로 닫아야 한다.

## 수동 화면 검증 대기

에이전트는 Client/UI를 실행하거나 화면 fidelity를 승인하지 않았다. 사용자가 다음 경로를 직접 확인해야
한다.

```text
Client Debug 실행
Character Select -> Artist
F1 -> Effect Tool -> All Effects -> Artist
Skill | F | V3 Admission | Main Review + Diagnostics
Play V3 Main Review (3)
```

확인 기준:

1. 1.059초의 검은 판과 초기 보라 billboard가 없어야 한다.
2. 약 1.67초에는 #9/#10 core와 #11 outer만 짧게 보여야 한다.
3. 2.177초에는 #11 outer만 남고 페인트/바닥/흰 쐐기가 없어야 한다.
4. 2.610초에는 V3 occurrence 출력이 없어야 한다.
5. 세 본체 중 하나라도 흰색 또는 raw blue carrier 면으로 보이면 해당 행을 승인하지 않는다.

## V3 이후 결정

- #9/#10/#11이 역할과 형태를 보존하면 사용자가 occurrence별로 승인한 행만 다음 합성 revision에
  승격한다.
- 세 행도 실패하면 복원 시도를 종료한다. alpha/gain/threshold/scale 또는 임의 downward translation으로
  화면을 맞추지 않는다.
- 이후 본체는 사용자 목표인 위에서 아래로 내려오는 붓획 느낌을 기준으로 별도
  `MANUAL_TUNED` asset/revision으로 저작한다. 이 결과는 복원 성공 통계와 분리한다.
