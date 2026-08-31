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

## 11. 2026-09-01 Product F 단일 유리 기준편

사용자는 여러 shard와 전체 그래프를 먼저 확장하지 않고, 실제 F 제품 경로에서 **유리 한 조각이
먼저 보이는지**를 기준으로 삼기로 결정했다. G1 audition 후보와 그 provenance/freshness 계약은 그대로
보존하고, 별도의 ordinary Product 문서
`effect.dimensionmaster.skill.2050230.single-glass-canary`를 추가했다.

### 11.1 고정된 한 조각 계약

- model: `Effect/DimensionMaster/Meshes/fm_a_broken_012.wmodel`
- topology audit: 1 submesh, 40 vertices, 28 triangles, position-weld 기준 연결된 한 조각
- element/particle/carrier: 각각 정확히 1개
- spawn: `maxParticles=1`, `burstCount=1`, `spawnRate=0`, local-space non-billboard
- F occurrence: `pc_sp_m_00_sk_sk_chronorecoil`, hit timing과 같은 `700ms`, root follow
- visible interval: particle life `1.4s`, document life `1.6s`
- material: 기존 compiled RuntimeMaterialV2 glass opcode `1004`
- packet: Coverage, BodyOpacity, Fresnel, Edge, Crack, Refraction, DistortionClamp, Emission과
  Body/Edge HDR tint의 bounded typed parameter만 사용

기존 `fx_m_glass_01.wmodel`의 24-island rigid cluster를 Product로 옮기지 않았다. 새 기준편은 한 번의
F 입력에 한 occurrence, 한 particle, position-weld 기준 한 연결 조각만 제출하므로 이후 수식·렌더러
변경의 원인을 단일 표본에서 분리할 수 있다.

### 11.2 수식과 renderer 결합

opcode `1004`는 mesh normal과 view direction에서 body transmission, Fresnel edge, crack mask,
bounded emission을 계산한다. 결과는 기존 두 출력 계약을 그대로 탄다.

```text
RT0 = straight-alpha glass radiance + coverage
RT1 = coverage가 적용된 bounded signed distortion
RT0/RT1 -> SceneHDR distortion resolve -> ScreenPost -> Bloom -> Final
```

따라서 Effect Tool의 surface parameter와 Rendering Workbench의 exposure/bloom/post parameter가 같은
고정 표본의 서로 다른 계층을 조절한다. 새 런타임이나 임의 HLSL graph를 추가하지 않았고,
`CModel -> CMaterial`과 compiled opcode 경로를 재사용한다. 후속 benchmark는 이 한 occurrence를 고정
camera/time/exposure에서 반복해 surface, distortion resolve, bloom/tone pass의 비용과 중간 결과를
분리하는 순서로 확장한다.

### 11.3 자동 검증과 정본 gate 상태

| 검증 | 결과 |
|---|---|
| mirror audition + Product single glass + F visual focused | `19 tests`, PASS |
| source + Product shader/WARP 포함 wider focused | `65 tests`, PASS |
| Effect source validator | PASS, direct source `174`, unbound reference `1`, resource files `994` |
| Debug direct Engine -> Shared -> Server -> Client compile | PASS, `Client.exe` link 완료 |
| Debug compiled shader closure/WARP | PASS, V1 `1352`, V2 `1352` pixels |
| Release direct Engine -> Shared -> Server -> Client compile | PASS, `Client.exe` link 완료 |
| Release compiled shader closure/WARP | PASS, V1 `1352`, V2 `1352` pixels |
| canonical Debug Product runner | FAIL/BLOCKED: `origin/main` Kakul exact-output assertion stale |
| 사용자 visual fidelity | PENDING |

canonical runner의 실패는 이번 diff가 아니다. `origin/main`의 `BuildDomains.json`은 Kakul map output을
`.mapassets/.mapplacements/.deployassets/.deployplacements` 네 개로 선언하고 publisher도 네 개를 실제로
만든다. 반면 같은 정본의 build-domain test는 예전 두 map output만 exact 비교한다. 해당 suite는
이 한 건 외 `17/18`이 통과했고 deploy fixture도 PASS했으며, single-glass gate에 도달하기 전에
중단됨을 확인했다. 범위 밖 Kakul 정본은 이번 glass commit에서 수정하지 않는다. 직접 compile과 CSO
closure는 보조 검증이며 canonical Product PASS로 대체 표기하지 않는다.

### 11.4 사용자 수동 확인

현재 PC는 LAN `server-host`이나 TCP 7777 LocalSubnet 방화벽 규칙이 누락돼 있다. Server+Client 수동
실행 전에 관리자 PowerShell에서 다음을 한 번 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
```

그 뒤 사용자가 직접 `Framework.sln`의 Server + Client profile을 실행하고 Character Select에서
차원술사를 선택한 뒤 F를 한 번 누른다. 약 `0.7s`에 캐릭터 앞·위쪽에서 청록색 깨진 유리 한 조각이
나타나 약 `1.4s` 동안 천천히 회전하는지가 첫 판정 대상이다.

다음 네 항목을 분리해 기록한다.

1. Shape: 둥근 card/검은 면이 아니라 각진 한 조각으로 읽히는가
2. Surface: body transparency와 밝은 Fresnel/crack edge가 함께 보이는가
3. Refraction: 배경 왜곡이 보이되 화면을 과도하게 흔들지 않는가
4. Timing: F의 700ms water hit와 결합되면서도 한 조각을 식별할 시간이 있는가

자동 검증은 occurrence와 renderer 계약까지만 보장한다. 위 네 항목의 `manual first pixel`과 visual
PASS는 사용자의 직접 관찰 전까지 계속 `PENDING`이다. compiled WARP는 opcode `1004`의 RT0/RT1
backend를 검증하지만 이 exact Product 문서의 실제 화면 first draw를 대신하지 않는다.

### 11.5 다음 확장 순서

첫 유리가 보인다는 판정 뒤에만 다음을 순서대로 진행한다.

1. 고정 camera/time/exposure와 A/B parameter snapshot
2. effect/distortion/bloom/tone pass별 GPU timestamp와 intermediate target 진단
3. 동일 surface packet을 공유하는 여러 독립 shard instance
4. 실제 결함이 확인된 경우에만 V1 depth soft fade, thickness proxy, rough refraction adapter
5. 같은 typed composition/carrier/curve 구조를 F 전체와 ALT_V 같은 복합 연출로 일반화

머신러닝은 renderer input, carrier, pass 계약을 대신하지 않는다. 충분한 고정 capture와 사용자 rating이
쌓인 뒤 bounded parameter 후보를 좁히는 offline 보조 최적화기로만 검토한다.
