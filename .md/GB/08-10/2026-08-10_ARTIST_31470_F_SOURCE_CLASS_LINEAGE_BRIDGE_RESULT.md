# 2026-08-10 Artist 31470 F Source Class Lineage Bridge Result

기준 commit은 `ccb60a519d1c9e2fb955652e862561b9b670438e`다.

## 완료 상태

v14 transitional 문서는 기존 normalized class 규칙을 계속 사용할 수 있다. 반면
`exactSourceClass`와 `aliasId`가 있는 receipt-bound 문서는 원본 class를 소문자화만 하고
`efparticlemodule` prefix 또는 `_seeded` suffix를 제거하지 않는다.

Artist F fixture에서 exact custom/seeded class는 26 occurrence, 13 raw class family다. 이
행들을 legacy normalized class로 바꾸면 Codec load가 실패한다. unknown exact class의 실행
승인 여부는 Compiler/handler receipt가 별도로 판정하며 Codec이 표준 class로 세탁하지 않는다.

## 검증

- ClientFrontendHarness x64 Debug build: PASS
- ClientFrontendHarness x64 Release build: PASS
- `Test-EffectSourceClassLineage.ps1` Debug: PASS
- `Test-EffectSourceClassLineage.ps1` Release: PASS
- paired field 누락, exact class mismatch, alias-only, blank exact class: 모두 reject
- receipt-bound custom class의 legacy normalization mutation: reject
- PowerShell parse와 `git diff --check`: PASS

이미지·육안 검증은 수행하지 않았다. 이 변경은 Source payload, runtime executor 또는 Product
admission을 열지 않는다.
