# 2026-08-07 Effect Tool Mesh 제작 레퍼런스 정합 결과서

## 1. 결론

레퍼런스 Tool의 핵심 제작 순서를 현재 v12 Effect 계약 위에 복구했다.

```text
Domain
→ Mesh thumbnail
→ Base / Noise / Mask / Emissive / Dissolve
→ Create Effect
→ Effect Detail live preview
→ Apply / Save / Reload
```

이번 작업은 기존 Cascade 실행기를 삭제하거나 다시 만드는 변경이 아니다. 신규 제작 전면은 Mesh 전용으로 바꾸고, 기존 Particle/Cascade 문서와 runtime은 Imported diagnostic으로 보존했다.

## 2. 구현 완료

### 2.1 Mesh Authoring Builder

- `Resources/Effect`의 실제 5개 Domain을 선택한다.
- DimensionMaster의 WModel 140개와 DDS 701개를 기존 clipped/lazy thumbnail로 탐색한다.
- Mesh는 `All`, Ring, Slash, Crack, Sphere, Cylinder, Helix, Box, Wave, Other 이름 힌트로 좁힐 수 있다.
- 이름 분류는 UI filter일 뿐이며 저장 정본은 Resources-relative asset ID다.
- Mesh, Base, Noise, Mask, Emissive, Dissolve 고정 slot 카드를 제공한다.
- Mesh와 safe Base가 모두 있기 전에는 `Create Effect`를 실행할 수 없다.
- normal/bump/blankwhite와 `_n` normal 표기는 Base에서 차단한다.
- 근거가 없는 normal data를 Noise에 연결하는 것도 차단한다.

### 2.2 원자적 Create Effect

Thumbnail 선택은 별도 builder draft만 변경한다. Create 성공 시에만 다음 Element 한 개를 active document 복사본에 추가하고 기존 `Try_CommitDocument`를 한 번 호출한다.

```text
kind                 MESH
template             effect.standard
useModelMaterial     false
sourceRecipe         disabled
meshModel            required
base                 required
noise/mask/emissive/dissolve optional
```

중복 Element ID, 다른 Domain/kind, unsafe Base, resource stage·save 실패는 active document와 preview를 보존한다. 성공하면 Authored Data File까지 원자 저장하고 새 Element를 선택한 뒤 preview를 0초부터 재생하며 Effect Detail을 연다. 이후 Detail 조절값은 기존 Apply/Save 경계를 사용한다.

### 2.3 Effect Detail 실행 의미

- Position/Rotation/Revolution/Scaling/Velocity/Color/UV/Timing Start 값은 체크박스 없이 입력 즉시 live draft preview에 반영된다.
- 각 `Lerp ...` 체크박스는 Start→End lifetime 보간만 활성화한다.
- Lerp를 OFF→ON 하면 preview가 0초부터 자동 재생되어 Rotation·Position·Scale·Velocity 변화가 보인다.
- playback 수식이 lifetime 중간에서 Position/Rotation/Scale/Velocity를 함께 평가하는 하네스를 추가했다.
- Emissive가 없으면 Bloom Intensity 입력은 비활성화된다.
- standard shader도 Base 색에 Bloom 값을 곱하지 않고 Emissive RGB에만 intensity를 적용한다.
- standard Noise는 RG를 사용해 Base/Mask/Emissive/Dissolve surface UV를 실제로 왜곡하며 R은 dissolve 보조값으로도 사용한다.
- Mask와 Dissolve는 R, Emissive는 RGB를 사용한다.
- PassName raw string 대신 기존 5개 stable Render Profile enum을 유지한다.

### 2.4 기존 경로 보존

- All Effects에서 빈 generic Element를 만드는 primary `Add Element`를 제거하고 `Time Reset All`, Delete, Clear All로 정리했다.
- Particle/Cascade 계층은 `Imported Cascade Diagnostics` 이름으로 보존했다.
- selected legacy Element의 resource 교체는 Advanced 영역에서 계속 가능하다.
- resource 변경 시 열린 Effect Detail draft가 있으면 Apply/Revert를 요구한다.
- atomic Save/Reload, stale writer 차단, publish 분리 계약은 변경하지 않았다.
- Warlord resource domain mapping 누락과 잘못된 `v8` New 성공 문구를 교정했다.

### 2.5 이름 기반 Create Effect 긴급 교정

- `Create Effect`의 기존 `active Data File 필수` 조건을 제거했다.
- Effect Name, Mesh, safe Base만 준비되면 새 v12 Authored 문서와 Mesh Layer를 함께 stage하고 `Data/Effects/Authored/<EffectName>.effect.json`에 원자 저장한다.
- 저장 성공 뒤 Data Files와 All Effects를 새로 고치며 생성된 Mesh Layer를 Effect Detail에서 즉시 선택한다.
- 같은 이름의 active Authored 문서라면 현재 Detail draft를 같은 transaction에 적용하고 새 Layer를 추가한다.
- 다른 writer가 같은 파일을 만들거나 stale baseline이 발견되면 기존 문서·GPU preview·builder 선택을 보존하고 저장을 거부한다.
- Data Files의 `New`는 빈 문서만 열며 Mesh/Base/Noise/Mask/Emissive/Dissolve 선택을 초기화하지 않는다.
- Create 성공 후에도 builder slot은 보존하고 Layer Name만 비워 동일 재료로 다음 Layer를 빠르게 만들 수 있다.
- 썸네일 클릭은 builder의 현재 slot에 즉시 bind된다. 기존 `Use Selected` 버튼은 키보드/명시 선택 경로로도 계속 사용할 수 있다.

## 3. 자동 검증

| 검증 | 결과 |
|---|---|
| Client x64 Debug build | PASS, exit 0 |
| ClientFrontendHarness x64 Debug build | PASS, exit 0 |
| `--effect-authoring-fast` | PASS, failures 0 |
| `--effect-executor-fast` | PASS, failures 0 |
| Effect Tool final audit | PASS, palette 2667 |
| Effect pipeline fixture | PASS |
| Client startup smoke | 12초 생존 PASS |
| `git diff --check` | PASS |

추가 회귀 항목:

- missing Mesh / missing Base / unsafe Base 거부
- manual Mesh drawable document canonical round trip
- atomic save/reload와 stale external writer 보존
- 3/30/60/144 FPS deterministic playback
- manual Mesh Position/Rotation/Scale/Velocity lifetime Lerp
- 기존 DimensionMaster authored/source semantics/VectorField 실행

## 4. 전체 ProjectAudit

전체 ProjectAudit은 77개 중 76개가 통과했다. 남은 한 건은 이번 Tool 변경이 아니라 현재 공유 작업트리의 기존 생성 데이터 불일치다.

```text
effect.wfx-component-assembly
WFX compile identity mismatch: effect.dimensionmaster.skill.2050120
```

이번 UI 슬라이스에서 불완전한 W authored 결과를 canonical Assembly/WFX로 재발행하지 않았다. 따라서 이 한 건을 숨기거나 Tool 완료로 오인하지 않는다.

## 5. 수동 GPU 검증

자동 검증은 닫았지만 실제 ImGui 조작과 GPU slot 결과는 아직 사용자 수동 판정 전이다.

권장 첫 검증:

1. Mesh Effect Authoring의 Effect Name에 사용하지 않은 test ID를 입력한다. Data Files의 New는 누르지 않는다.
2. DimensionMaster → Slash/Trail/Plane에서 carrier WModel 하나를 클릭한다.
3. Base slot을 고르고 DDS 하나를 클릭한 뒤 Create Effect 한다.
4. Effect Detail에서 Position/Rotation/Scaling 값을 바꿔 즉시 반영을 확인한다.
5. Lerp Rotation을 켜고 End 값을 바꿔 0초부터 회전하는지 확인한다.
6. Mask → Noise → Dissolve → Emissive 순서로 연결한다.
7. Bloom은 Emissive 연결 전에는 비활성, 연결 후에는 Emissive에만 작동하는지 확인한다.
8. Apply + Save Authored → Reload Saved 후 값과 layer가 동일한지 확인한다.

## 6. 명시적 한계

- Color Clip은 현재 v12 schema와 shader가 scalar final-alpha threshold다. 레퍼런스 화면의 RGBA 네 채널 clip으로 확정하려면 format/codec/renderer/HLSL을 함께 확장해야 하며 이번 Mesh builder transaction에 거짓 UI로 추가하지 않았다.
- Billboard는 현재 Sprite 전용 계약이므로 Mesh Detail에 거짓 체크박스를 추가하지 않았다.
- Summon 같은 `useModelMaterial=true` 완성형 skeletal WModel은 load/render를 보존하지만, 이번 신규 Create UI는 carrier Mesh `Effect Surface` 모드만 만든다.
- slot별 R/RG channel thumbnail과 source-evidence ranking badge는 다음 사용성 개선 범위다. 이번 버전은 모든 후보를 보존하고 설명으로 실제 채널 의미를 표시한다.
- 원작 PNG와의 스킬 복원은 이 Tool 기반을 사용한 다음 작업이며, 이번 결과는 Q/A/T 픽셀 완성을 뜻하지 않는다.

## 7. 변경 파일

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`
- `Client/Public/Effect_MaterialTemplate.h`
- `Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/ProjectAudit/Test-EffectToolFinal.ps1`
- `2026-08-07_EFFECT_TOOL_MESH_AUTHORING_REFERENCE_PARITY_IMPLEMENTATION_PLAN.md`

## 8. 수동 Mesh 생성 피드백 반영

- 사용자가 실제 carrier WModel을 생성해 Transform, Revolution, Velocity, Distortion, Lerp의 GPU 반영을 확인했다.
- 새 Mesh Builder draft의 기본 Scale을 `(0.01, 0.01, 0.01)`로 고정했다. 이 기본값은 신규 수제 Mesh에만 적용하며 기존 문서와 imported Cascade 값은 변환하지 않는다.
- 공통 상수 `EFFECT_MANUAL_MESH_DEFAULT_SCALE`과 authoring harness round-trip 검증을 추가해 생성 Scale 계약을 고정했다.
- A `2050210` 추출 문서의 4개 반복 hit 시작 시각은 `0.25 / 0.60 / 0.90 / 1.30초`다. 수제 A 조립은 이 네 시각을 초기 Start Delay로 사용하고, `fm_h_swing_01/02` 중심 검격 carrier와 trail/broken/ring 보조 layer를 PNG A/B로 단계적으로 추가한다.

재검증 결과:

- ClientFrontendHarness Debug build: PASS, exit 0
- Client Debug build: PASS, exit 0
- `--effect-authoring-fast`: PASS, failures 0
- Effect Tool final audit: PASS
- Client startup smoke: 12초 생존 PASS
- `git diff --check`: PASS
- 전체 ProjectAudit: 2건 실패. 현재 공유 작업트리의 `projects.data-source-visibility expected=548/project=547/filters=547`와 기존 `effect.wfx-component-assembly` 불일치이며, 이번 신규 Mesh Scale 계약과 관련된 Effect Tool audit/harness는 통과했다.
