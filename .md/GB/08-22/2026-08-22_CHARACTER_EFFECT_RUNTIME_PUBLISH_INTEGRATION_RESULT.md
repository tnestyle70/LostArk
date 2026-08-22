# 캐릭터 Effect authored → sealed runtime 통합 결과

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

Client Debug/Release와 신규 창술사 Q/A focused harness는 뒤따르는 통합 commit까지 포함한 최종
branch 기준으로 다시 실행한다. Client/UI는 자율 실행하지 않았으며 first-pixel과 원본 색감 판정은
사용자 수동 검증 전까지 `PENDING`이다.

## 5. 다음 국소 복원 경계

광범위한 캐릭터 전수 수정은 여기서 멈춘다. 다음 문서는 워로드 F `17140`, V `17170`,
풀배럴 캐넌 T `17240`과 도화가 A/S 및 이미 반영된 스킬을 다음 열로 먼저 고정한다.

```text
실행 순서 → 원본 emitter/source occurrence → Product element stable ID
→ carrier/mesh → texture role별 DDS → child/parent material
→ stored family → 실제 effective family → 화면 역할 → 증거 수준
```

그 표에서 하나의 작은 occurrence cohort를 선택해 `carrier → family → shader/renderer →
publish → build → 사용자 화면 판정`을 끝까지 닫고, 검증된 공식만 같은 family에 확장한다.
