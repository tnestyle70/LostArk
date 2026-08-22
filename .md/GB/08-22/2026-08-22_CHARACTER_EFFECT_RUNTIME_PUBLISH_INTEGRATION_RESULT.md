# 캐릭터 Effect authored → sealed runtime 통합 시도 결과

> 최종 판정: `NOT MERGED`. 아래 publisher 결과는 후보 브랜치 내부에서 성공했지만,
> 정본 Debug 통합 회귀의 `ClientFrontendHarness`가 46건 실패해 main 승격을 중단했다.

## 1. 이번 통합 경계

2026-08-22 `codex/effect-family-conquest`의 캐릭터 Effect authored 문서를 정본
publisher 한 번으로 `Client/Bin/DataFiles/Effect` runtime catalog에 반영했다. 이 변경은
새 이펙트를 손튜닝하는 단위가 아니라, 이미 구현·검증된 authored 결과가 실제 Client가 읽는
content-addressed sealed 문서로 선택되게 만드는 통합 단위다.

발탄 authored/candidate/cue/binding은 수정하지 않았다. `origin/main`에 이미 병합된 발탄 runtime
행은 입력으로만 소비했다.

## 2. 실제 runtime 선택 변경

`EffectCatalog.runtime.json`은 205개 Effect를 전부 검증한 뒤 한 번에 교체됐다. 기존 catalog와
비교했을 때 authored document identity가 바뀐 Effect는 18개다.

| 대상 | 변경된 runtime Effect |
|---|---|
| 도화가 | `31210.ba1`, `31210.ba4`, `31420`, `31460`, 신규 `31490`, `31910`, `31950` |
| 차원술사 | `2050120.clip3`, `2050210`, `2050230` |
| 창술사 | `34560.clip3`, `34630.clip1~4`, `34650.clip1` |
| 워로드 | `17240.ba1`, `17240.ba3` |

워로드 풀배럴 캐넌은 이 교체로 다음 authored 상태를 실제 runtime이 선택한다.

- BA1: 구버전의 `screenPost/RGBNoise` 한 행이 제거된 24행 문서
- BA2: 기존 6행 문서 유지
- BA3: particle 8행과 진짜 LocalDecal 4행을 가진 12행 문서

도화가는 이 교체로 다음 authored 상태를 실제 runtime이 선택한다.

- A `31460`: 검격 8행의 SpriteWave UV-noise 기여가 `0`인 18행 문서
- S `31420`: 기존 한 행과 grass body/tip 두 행을 가진 3행 문서
- R `31210`: BA1 symbol 오염 원복, BA4 true LocalDecal 한 행 추가
- D `31490`: 기존 56행과 BLACK_TIGER_STROKE 12행을 가진 신규 catalog row
- V `31910`: donor 복원 5행과 target-attractor 계약
- T `31950`: source ribbon 한 행의 CascadeRibbon typed projection

E `31480`과 F `31470`은 publish 전부터 authored와 runtime sealed 문서가 semantic-equal이므로
이번 catalog 교체 대상이 아니다.

## 3. 생성물

| 파일 | 결과 |
|---|---|
| `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json` | 205 Effect의 direct sealed-document catalog |
| `Client/Bin/DataFiles/Effect/EffectVisualPrograms.runtime.json` | 17 program, 135 row, SHA-256 `0efa773900da495869f71b4f3d0bba787455ca3fb10eb790c4ca432762191054` |
| `Client/Bin/DataFiles/Effect/Authored/*.effect.json` | 새 content identity가 필요한 immutable sealed 문서 17개 추가 |

publisher는 authored 문서를 직접 덮어쓰지 않고 새 SHA-256 identity의 immutable 문서를 만든 뒤,
모든 입력 검증이 성공한 경우에만 catalog를 교체했다. 실패 시 기존 catalog를 유지하는
`parse → validate → stage → commit` 경계는 그대로 사용했다.

## 4. 실행한 검증

| 명령 | 결과 |
|---|---|
| `Publish-Effects.ps1 -Mode Validate` | 205 Effects, visual-program sidecar PASS |
| `Publish-Effects.ps1 -Mode Publish` | 205 Effects, 0 Components, sidecar publish PASS |
| visual-program artifact check | 17 programs, 135 rows, `productMutation=false` PASS |
| catalog old/new identity 비교 | 정확히 18 Effect mapping 변경 확인 |
| sealed target cardinality parse | Artist `4/69/3/18/68/46/23`, Warlord T `24/6/12`; BA1 Post 0, BA3 Decal 4 PASS |

Client Debug build/link는 통과했지만 아래 정본 회귀에서 하네스가 실패했다. Release 통합 회귀와
신규 창술사 Q/A focused harness는 완료하지 않았다. Client/UI는 자율 실행하지 않았으며
first-pixel과 원본 색감 판정도 실행하지 않았다.

## 5. 종료 판정과 보존 위치

정본 통합 명령은 다음 순서까지 진행됐다.

| 단계 | 결과 |
|---|---|
| Engine/Shared/Server/Client Debug build/link | PASS |
| Balance/World/Navigation Validate | PASS |
| Effect 205개 및 Data project registration | PASS |
| Artist F material oracle / Rendering profile / NetworkProtocolHarness | PASS |
| 기본 `ClientFrontendHarness` | **FAIL, 46 failures** |

대표 실패군은 DimensionMaster source material admission과 semantic module 보존, legacy GPU
occurrence, source overlay CAS/round-trip, Valtan placement source contract였다. 따라서
`Publish-Effects` 통과만으로 제품 회귀가 닫혔다고 판단하지 않았고 PR #141은 main에 병합하지
않았다. 후보 코드·데이터·publisher 결과는 다음 복구 가능한 위치에 보존한다.

```text
branch               codex/effect-family-conquest
implementation head  10e18dea50d3915f0bfa71484ea3a711b38c8dd8
PR                   #141 (closed without merge)
base                 main@7fb8f8139f62657914228070ebe2a9860287b577
```

사용자 결정에 따라 이 광역 복원 방향은 여기서 종료한다. 새 방향은 최신 main에서 새 범위와
새 완료 판정자를 먼저 정한 뒤 시작하며, 이 브랜치를 통째로 cherry-pick하거나 부분 결과를
이미 main에 반영된 것으로 간주하지 않는다.

## 6. 폐기한 다음 국소 복원 경계

광범위한 캐릭터 전수 수정은 여기서 멈춘다. 아래 항목은 당시 제안이었으나 방향 재설정으로
실행하지 않았다. 워로드 F `17140`, V `17170`,
풀배럴 캐넌 T `17240`과 도화가 A/S 및 이미 반영된 스킬을 다음 열로 먼저 고정한다.

```text
실행 순서 → 원본 emitter/source occurrence → Product element stable ID
→ carrier/mesh → texture role별 DDS → child/parent material
→ stored family → 실제 effective family → 화면 역할 → 증거 수준
```

그 표에서 하나의 작은 occurrence cohort를 선택해 `carrier → family → shader/renderer →
publish → build → 사용자 화면 판정`을 끝까지 닫고, 검증된 공식만 같은 family에 확장한다.
