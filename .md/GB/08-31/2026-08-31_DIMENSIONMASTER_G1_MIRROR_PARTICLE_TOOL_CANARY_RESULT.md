# 차원술사 G1 유리 거울 파티클 Effect Tool canary 구현 결과

기준일: 2026-08-31

대응 계획: `2026-08-31_DIMENSIONMASTER_GLASS_WATER_TWELVE_PATH_VISUAL_LAB_PLAN.md`

## 1. 결론

`origin/main`의 `a265b683`에서 분리한 독립 worktree와 정확한 브랜치
`DimensionMaster-Mirror-Particle`에 차원술사 F용 첫 번째 geometry oracle인 G1을 구현했다.
기존 Composition Work Bench 작업 폴더와 그 미커밋 변경은 수정하지 않았다.

G1은 `fx_m_glass_01.wmodel`의 24개 disconnected island를 한 rigid cluster로 재생한다. 따라서 이번
단계의 질문은 “개별 유리 조각 24개가 폭발하는가”가 아니라 “원형 billboard 대신 실제 각진 mesh
silhouette와 facet 조명이 유리로 읽힐 수 있는가”다. 사용자 Shape Oracle 승인이 나기 전에는 Product
F occurrence에 연결하지 않는다.

자동 검증은 데이터·렌더 패킷·GPU 출력·Tool admission의 안전성을 확인했다. 실제 유리 미감과 다음
G2 개별 shard bank 투자 여부는 사용자 육안 판정 대기 상태다.

## 2. 격리된 작업 위치

| 항목 | 값 |
|---|---|
| 원래 작업 폴더 | `C:\Users\user\Desktop\LostArk` |
| 원래 작업 브랜치 | `codex/composition-workbench` |
| 이번 독립 worktree | `C:\Users\user\Desktop\CodexWorkTree\LostArk-DimensionMaster-Mirror-Particle` |
| 이번 브랜치 | `DimensionMaster-Mirror-Particle` |
| 분기 기준 | `origin/main@a265b683` |

`Client/Bin/Resources`만 Git 제외 junction으로 팀 Drive runtime pack을 읽는다. 소스·Data·문서·빌드
출력은 이번 독립 worktree에 있다.

## 3. 구현된 canary

- stable effect ID:
  `effect.dimensionmaster.skill.2050230.mirror-particle-canary.unified`
- stable element ID:
  `project-tuned.glass-mirror-shards.2050230.01`
- runtime admission: `REGISTRY_BOUND_AUDITION_ONLY`
- provenance: `PROJECT_TUNED_APPROX`
- typed material opcode: `1004`
- mesh: `Effect/DimensionMaster/Meshes/fx_m_glass_01.wmodel`
- texture:
  `Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/fx_h_brokenglass_02_1.dds`
- particle: local-space mesh 1개, fixed seed `2050231`, non-billboard, revolution
  `[17, 41, 73]`, authored size `0.55 -> 1.10`

셰이더는 mesh world normal과 view direction으로 face transmission, Fresnel edge, thickness proxy,
crack texture, bounded emission을 계산한다. RT0는 straight-alpha radiance/coverage를 출력하고 RT1은
coverage가 적용된 작은 signed distortion만 출력한다. 원형 UV hemisphere normal을 새로 만드는
방식이 아니므로 외곽과 facet 방향은 실제 mesh가 결정한다.

## 4. Effect Tool 튜닝 계약

기존 raw material packet editor에 `-100000..100000` 수치를 늘어놓지 않고 opcode `1003/1004`에만
의미 단위 패널 `Project Tuned Surface`를 제공한다.

- Glass Tint
- Coverage / Edge
- Refraction
- Emission

각 값은 셰이더의 안전 범위로 제한된다. 공통 Carrier Color 및 color lerp는 project-tuned opcode에서
잠겨 있어 typed packet 밖의 중복 경로로 색을 바꿀 수 없다. `Apply to Current Effect (Unsaved)`로
stage한 뒤 Play/Audition하고, 유지할 값만 `Save Changes`로 후보 문서에 저장한다.

## 5. Product와 Tool의 소유권 격리

Product `EffectCatalog.json`에는 이번 candidate가 없고 기존 Product row와 byte diff가 없다. Product
loader와 Valtan artifact hash는 Tool 전용 `EffectAuditionCatalog.json`을 읽지 않으며, Product catalog에
`runtimeAdmission`이 섞이면 명시적으로 거부한다. 따라서 후보 문서가 누락·손상·stale 상태가 되어도
Product effect availability가 줄지 않으며, 현재 차원술사 F cue도 그대로 유지된다.

반대로 Tool의 direct authored source index는 Product와 audition registry를 함께 읽고 audition source의
raw SHA-256을 고정한다. 후보가 열린 뒤 행이 삭제되거나 registry를 이동하거나 Product로 재분류되거나
source path/hash가 달라지면 다음 동작을 fail-closed한다.

- 직접 load/reload
- `Play All`
- `Audition Selected`
- 공통 world preview 재시작
- `Save Changes`
- `Save As`는 freshness와 무관하게 exact audition ID 보존을 위해 사용 불가

구조상 정상인 stale 후보는 목록에서 사라지지 않고 잠긴 상태와 원인을 보여 준다. 하나의 stale
audition 때문에 나머지 Tool source index 전체가 실패하지 않는다.

## 6. 렌더 admission과 실패 경계

renderer는 opcode `1004`를 exact effect/element occurrence, mesh/texture, sampler, scalar/vector 개수,
parameter mask, finite 값, normal matrix, camera carrier가 모두 일치할 때만 허용한다. 이 검사는 최초
stage뿐 아니라 prepared-document와 resource-signature 재사용 전에도 다시 실행한다. 따라서
ColorOffset-only 수정처럼 resource signature를 바꾸지 않는 문서 mutation도 validator를 우회하지
못한다.

WARP probe는 정상 packet의 RT0/RT1 finite output을 확인하고 다음 invalid packet이 render target을
변경하지 않는 것을 확인한다.

- 잘못된 parameter mask
- scalar NaN
- zero normal matrix
- camera NaN

Release FXC가 일반 `isfinite(cameraPosition)` 검사를 제거하는 실측 실패가 있었기 때문에 opcode `1004`의
finite guard는 IEEE-754 exponent bit 검사로 고정했다. 최종 Release WARP probe에서 camera NaN을 포함한
네 invalid fixture가 모두 zero-output으로 닫히는 것을 다시 확인한다.

## 7. 자동 검증

| 검증 | 결과 |
|---|---|
| focused mirror/water/source/WARP unittest | `60 tests`, PASS |
| Visual Lab build gate | `14 tests`, PASS |
| Effect source validator | PASS, direct source `173`, unbound reference `0` |
| native audition harness | `29/29`, flow `13/13`, tuning `14/14`, Effect cue `11/11`, PASS |
| Debug Client targeted build | PASS |
| Debug compiled shader closure/WARP | PASS, V1 `1352`, V2 `1352` pixels |
| Debug Core 선행 회귀 | PASS, evidence `20260831T060634098Z-debug-core-3cae86a2.json`, Character Select failures `0` |
| Release Product 최종 현재 소스 | PASS, evidence `20260831T064742774Z-release-product-f18c3ad8.json` |
| Release compiled shader closure/WARP | PASS, V1 `1352`, V2 `1352`, camera NaN 포함 invalid RT0/RT1 `0` |
| Debug FullDiagnostic | NOT PASS: 외부 Drive pack의 Valtan Ghost WModel 2개 누락 |
| 사용자 visual fidelity | PENDING |

FullDiagnostic의 누락 리소스는 다음 두 파일이며 이번 canary가 새로 참조한 리소스가 아니다.

```text
Character/Valtan/Ghost/MN_RPBF_02.wmodel
Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel
```

## 8. runtime resource 실측

| 리소스 | 크기 | SHA-256 |
|---|---:|---|
| `Effect/DimensionMaster/Meshes/fx_m_glass_01.wmodel` | 20,400 bytes | `E20850266011A3FA569F05FA72A356D96AA5207556AF747087C530BB9315D2E5` |
| `Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/fx_h_brokenglass_02_1.dds` | 524,416 bytes | `92FCF09BA484267ACDEF99E97E4081E93F1F9262EFB193BA8FB2CAE42753A6A4` |

두 파일 모두 기존 팀 Drive resource pack을 재사용한다. 새 binary payload를 Git에 추가하거나 별도 전달할
필요는 없다. 물리 파일은 각각
`C:\Users\user\Desktop\LostArk\Client\Bin\Resources\Effect\DimensionMaster\Meshes\fx_m_glass_01.wmodel`과
`C:\Users\user\Desktop\LostArk\Client\Bin\Resources\Effect\DimensionMaster\Textures\FX_TEX_HIGH_03\fx_h_brokenglass_02_1.dds`에
있고, 독립 worktree의 junction 자체도 Git에 포함되지 않는다.

## 9. 사용자 수동 확인 경로

현재 PC는 LAN endpoint 기준 `server-host`이지만 TCP 7777 LocalSubnet 방화벽 규칙이 누락된 상태다.
Server+Client 수동 실행 전 관리자 PowerShell에서 다음을 한 번 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
```

그 뒤 사용자가 직접 다음 순서로 확인한다.

1. `Framework.sln`의 Server + Client profile을 실행한다.
2. F1 `Developer Tools`에서 기존 `Effect Tool`을 연다. `Effect Tool V2`가 아니다.
3. `Data Files` → Authoring Category `DimensionMaster` → `Refresh Index`를 누른다.
4. `effect.dimensionmaster.skill.2050230.mirror-particle-canary.unified`를 검색한다.
5. `Unassigned / Test Effects`에서 후보를 선택하고 `Load Saved Effect for Editing`을 누른다.
6. `Current Effect` → 대상 Element → `Project Tuned Surface`에서 수치를 조절한다.
7. `Apply to Current Effect (Unsaved)` → `Play All` 또는 `Audition Selected`로 확인한다.
8. 유지할 값만 `Save Changes`로 저장한다.

대체 진입은 `All Effects` → `Dimension Master` → exact candidate ID → `Saved Unified Effects` →
`Open Editor`다. F 키만 눌러서는 이번 G1이 나오지 않는다.

## 10. 남은 경계와 다음 승격 조건

- G1의 24개 island는 서로 따로 날아가지 않는 한 rigid cluster다.
- 원형이 깨지고 유리 외곽·facet으로 읽힌다는 사용자 Shape Oracle 판정이 먼저 필요하다.
- G1이 생존하면 G2에서 island별 pivot, velocity, quaternion을 가진 instanced shard bank를 구현한다.
- G1이 탈락하면 표면 상수 100개를 계속 조절하지 않고 G7 analytic polygon impostor 또는 다른
  carrier를 비교한다.
- Product F/W occurrence 연결, synchronized A/B, `USER_APPROVED` 표기는 별도 승인 후 변경 단위다.
- renderer의 wrong-parent/duplicate/reuse-mutation은 현재 Python/static contract와 실제 stage validator로
  막혀 있다. 이를 직접 호출하는 별도 native renderer document-staging negative harness는 후속 자동화
  보강 항목이다.
