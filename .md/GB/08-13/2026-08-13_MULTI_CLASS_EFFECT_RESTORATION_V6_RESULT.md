# 2026-08-13 멀티 클래스 이펙트 복원 V6 결과

## 2026-08-14 Authoring-first 구현 closeout

이제 Effect Tool의 주 경로는 Track A exact 복원 row를 고르는 화면이 아니라, 사용자가 resource와 stable
element를 직접 조합하는 authoring workbench다.

- resource browser는 `DimensionMaster`, `Artist`, `LanceMaster`, `Warlord`, `Valtan` 다섯 domain의
  `445 WModel + 2,234 DDS`를 읽는다.
- `Mesh`, `Sprite`, `MeshParticle`, `SpriteParticle`, `LocalDecal`, `Trail/Ribbon` 여섯 family에서
  `Create Effect`로 첫 element를 만들고 `Add Element`로 하나의 effect에 여러 element를 결합한다.
- All Effects는 `Active Effect -> Family -> stable Element` 트리다. element click은 편집 선택만 바꾸며
  `Solo`, `Play Group`, `Play All`이 재생 범위를 명시적으로 바꾼다.
- Effect Details는 transform, velocity, color, emissive, distortion, dissolve, UV, timing과 family별 값을
  편집한다. 현재 WModel/DDS named slot을 바로 바꾸고 authored `.effect.json`을 Save/Reload한다.
- editable slot과 별도로 선택 row의 exact `SourceMaterial`/Track A DDS evidence를 role, register, channel,
  sampler, hash와 함께 read-only로 표시한다.
- `Use Selected Element as Preset`과 generic starting copy는 WModel/DDS/Material/Detail을 보존한다. source
  occurrence identity, native renderer/adapter packet, source attachment와 source rotation lane은 제거하므로
  저장 결과가 native packet exact 보존을 주장하지 않는다.

Track A는 exact DDS/WModel identity, source 수치, material lane과 family carrier의 **evidence/starting
recipe**다. Track B authored `.effect.json`이 화면 완성의 정본이며 binary publish/packing과 draw/resource
공유는 후속 최적화다. Warlord와 Valtan은 같은 browser/schema를 쓰는 resource authoring domain까지만
열렸고 exact 복원 product content로 승격되지 않았다.

자동 검증은 다음 최신 denominator로 닫혔다.

| 검증 | Debug | Release |
|---|---:|---:|
| Client build/link | PASS | PASS |
| `--effect-authoring-fast` | failures `0` | failures `0` |
| `--effect-runtime-fast` | `23/23 PASS` | `23/23 PASS` |
| occurrence tuning | `10/10 PASS` | `10/10 PASS` |
| reconstructed material | `24/24 PASS` | `24/24 PASS` |

Python은 visual-program runtime `12/12`, corpus `10/10`, occurrence tuning `5/5`로 PASS했다. 사용자가
Effect Tool을 실제 조작한 visual fidelity와 Save/Reload UX 판정은 아직 PENDING이다.

방금 표시된 `Debug Assertion Failed` 창은 컴파일·링크 오류가 아니었다. Debug 실행은 해당 창 뒤에도
정상 종료/PASS했고, 당시 dialog의 Program/File/Line/Expression을 확보하지 못했으므로 exact assert 원인은
확정하지 않는다. focused harness는 Debug CRT assert/error report를 modal dialog 대신 stderr로 보내고
process error box를 억제하도록 바꿨으며, 같은 Debug authoring/material 회귀를 다시 실행해 PASS했다.
ProjectAudit는 shared dirty worktree에서 관련 도구가 기존부터 삭제된 상태라 실행하지 않았고, 이를
복원하거나 다른 검증으로 ProjectAudit PASS를 주장하지 않는다.

## 2026-08-14 최종 결론

V6에서 Track A를 원작 100% 자동 복원이나 모든 native Shader/VF/MRT 증명으로 확장하지 않는다.
Track A가 회수한 DDS/WModel identity, source module 수치, material lane, transform/timing과 family 의미는
**편집 가능한 stable Element의 초기 레시피**다. 화면 완성의 authoring 정본은 기존 Effect Tool에서
사용자가 편집하고 저장한 `Data/Effects/Authored/*.effect.json`이며, runtime catalog와 visual sidecar는
그 결과를 배포·prewarm·최적화하기 위한 산출물이다.

공통화한 것은 하나의 universal effect shader가 아니다. stable Element identity, 문서, Resource Library,
Effect Details, Save/Reload와 publish transaction이 공통 계약이다. `MeshParticle`, `SpriteParticle`,
`LocalDecal`, `CascadeRibbon`, `AnimationTrail`, `LightParticle`, `ScreenPost`는 각각의 geometry와 carrier
adapter를 유지한다.

## 실제 구현 상태

### visual-program corpus와 제품 게시

- visual program `13`: 차원술사·도화가·창술사 BA1~BA4 `12`개와 도화가 F adapter `1`개.
- renderer row `135`: MeshParticle `41`, SpriteParticle `69`, CascadeRibbon `4`, LocalDecal `4`,
  LightParticle `15`, ScreenPost `2`.
- supplemental row `5`: AnimationTrail `4`, 도화가 F CascadeRibbon `1`.
- renderer admission은 `72/135`, fail-closed는 `63/135`다. 세 BA는 admitted `70/133`,
  fail-closed `63/133`이다. family별 admitted/total은 Mesh `30/41`, Sprite `36/69`,
  LocalDecal `2/4`, CascadeRibbon `4/4`, Light `0/15`, ScreenPost `0/2`이며 supplemental은 `5/5` admitted다.
- catalog와 `EffectVisualPrograms.runtime.json`을 한 publisher transaction에서 validate하고 함께
  backup/commit/rollback한다. 한 파일만 교체된 상태는 허용하지 않는다.
- tracked 제품 산출물까지 실제로 게시했다.

| 산출물 | bytes | SHA-256 |
|---|---:|---|
| `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json` | 27,149,532 | `9638e18838abe4f80cee4adcfba52a61a775862d8ba9365e597c3073cc513a3d` |
| `Client/Bin/DataFiles/Effect/EffectVisualPrograms.runtime.json` | 3,863,062 | `3a16623a6ccd1686b57a16f96245dccc9635e63a16535f1ade44e1fb5634999a` |

visual sidecar의 source와 product payload는 동일하고 logical artifact SHA field는
`dfd1518bfb8efba77cb730aa1a68ca059855e12b36cc14e4144e0b615c95ff60`이다.

### 기존 Effect Tool의 stable Element authoring bridge

- 세 BA의 admitted `SOURCE_RECIPE_OVERLAY_V1` stable row를 기존 전체 Effect Details와 Resource Library에
  연결했다.
- row 선택은 편집 대상만 바꾸며 현재 All/Family/Solo 제출 범위를 암묵적으로 바꾸지 않는다.
- DDS, WModel, material parameter, module, timing, transform, color와 UV를 기존 full Details에서 편집한다.
- runtime/import projection은 immutable source로 연다. 첫 편집은 generic authored document copy를 stage한다.
- `Save As Authored Copy`가 `Data/Effects/Authored/<new-id>.effect.json`을 만들고, 그 뒤에는 기존 authored
  renderer와 Save/Reload 경로를 사용한다. generated/imported source를 직접 덮지 않는다.
- stable identity를 사용하며 family, UI row, vector index를 저장 ID로 사용하지 않는다.

도화가 F `ADAPTER_PACKET_V1`은 별도 경계다.

- `Play Core F`로 exact adapter projection이 현재 `CEffectObject`에 stage된 뒤 LocalDecal `#20/#21`과
  supplemental CascadeRibbon `#3`만 stable row로 열린다.
- exact adapter Details/ResourceBindings는 inspection-only다. ordinary Save As 또는 packet mutation은
  projector/VF/6-SRV 의미를 잃으므로 차단한다.
- 위치·회전·스케일은 stable occurrence `PROJECT_TUNED` Save/Reload로 영속한다.
- 선택한 Decal/Trail 하나는 명시적 **generic Authored starting copy**로 낮춰 저장할 수 있다. ordinary
  codec roundtrip과 drawable validation을 통과하고 Detail/Material/ResourceBindings를 보존하지만,
  native adapter packet을 보존했다고 주장하지 않는다. 저장 뒤에는 기존 Effect Tool의 full 편집과
  Save/Reload를 사용한다.
- 도화가 F Mesh `#4`, crack `#22`와 나머지 Core row는 이 3-row adapter bridge의 대상이 아니다.

## 자동 검증 증거

| 검증 | 결과 |
|---|---|
| 현재 tracked product dual output `--effect-runtime-fast` | PASS, failures `0` |
| visual-program runtime Python | `12/12 PASS` |
| visual-program corpus Python | `10/10 PASS` |
| occurrence tuning Python | `5/5 PASS` |
| derived artifact/publisher Python | `28/28 PASS` |
| source visual-program Playback/Trail admission C++ harness | `8/8 PASS` |
| occurrence selection/tuning/save rollback C++ harness | `10/10 PASS` |
| Effect authoring fast Debug/Release | 양 구성 failures `0` |
| reconstructed material Debug/Release | 양 구성 `24/24 PASS`, failures `0` |
| runtime fast Debug/Release | 양 구성 `23/23 PASS`, failures `0` |
| reconstructed decal Debug/Release | `13/13 PASS`, failures `0` |
| dual publisher | `102 Effects / 555 Components / 13 programs / 135 rows PASS` |
| Client x64 Debug build | PASS |
| Client x64 Release build | PASS |

Catalog assembler는 source authoring version을 보존하고, visual-program typed semantic identity는 부호만
다른 zero를 canonical zero로 정규화한다. identity 검사를 완화하거나 sidecar mismatch를 묵인하지 않는다.

### Lance BA1 Ribbon/Trail의 정확한 자동 경계

focused 검증에서 parser, token, particle/source simulation과 CPU의 두 점 이상
`CascadeRibbon`/`AnimationTrail` strip은 PASS했다. 그러나 WARP probe는 draw 전에 다음 Engine 초기화
실패로 중단됐다.

```text
Headless render probe Engine initialization failed HRESULT=0x80004005,
active=0 attempted=0 submitted=0
```

이는 shader 실패 증거도 GPU PASS 증거도 아니다. 더구나 현재 Lance CascadeRibbon projection의
material/resource는 `fallback-blocked` 경계를 보존한다. 따라서 검은 고리·UV·dissolve·alpha를 포함한
Ribbon 시각 품질이나 GPU 제출을 완료로 기록하지 않는다.

## Track A 종료선별 상태

| 경계 | 구현/자동 상태 | 사용자 시각 상태 |
|---|---|---|
| 도화가 F Mesh `#4` | cooked `ParticleModuleTypeDataMesh`의 `yaw=+90°`를 occurrence-owned carrier 값으로 한 번 적용하는 공용 계약 | forward-axis 판정 대기 |
| 도화가 F CascadeRibbon `#3` | typed CascadeRibbon 시작 레시피와 supplemental row 보존 | Ribbon 모양 판정 대기 |
| Lance BA1~BA4 CascadeRibbon | 실제 `ParticleModuleTypeDataRibbon`으로 Sprite 오분류 교정, CPU strip PASS | WARP 미도달·fallback-blocked material, 판정 불가/대기 |
| Lance BA1~BA4 AnimationTrail | animation `Trails` notify를 별도 family와 schedule로 보존, CPU strip PASS | 사용자 판정 대기 |
| LocalDecal `#20/#21` | 공용 projector + exact-role t0~t5 six-SRV bounded adapter | native FLocalDecal 아님, 사용자 판정 대기 |
| crack `#22` | 별도 원본 DDS occurrence 보존 | 사용자 판정 대기 |
| 세 캐릭터 BA 12단계 | 공용 visual-program artifact와 runtime join | animation timing/Combined 판정 대기 |

LocalDecal은 `native=false`인 bounded RT0 semantic adapter다. native FLocalDecal VF,
fog/custom-light CB, tangent parallax와 Target2~5 MRT exact replay는 Track A backlog로 남긴다. 자동 검증이나
문서만으로 LocalDecal 또는 Ribbon의 `visual PASS`를 선언하지 않는다.

## 확장성의 정확한 범위

schema, typed family resolver, adapter, catalog/sidecar transaction과 Effect Tool bridge는 class 이름,
skill ID, Artist order와 material path switch 없이 소비하도록 만들었다. 그러므로 다른 캐릭터와 Valtan이
같은 계약을 소비할 **구조**는 열렸다.

하지만 현재 product content denominator는 세 캐릭터 BA 12단계와 도화가 F adapter까지다. Warlord와
Valtan은 schema를 통과하는 fail-closed extension canary일 뿐 product admission 완료가 아니다.
Gunslinger, Slayer를 포함한 전체 캐릭터의 DDS/WModel/Element 연결도 완료로 주장하지 않는다. 다음 확장은
새 renderer를 만드는 일이 아니라 같은 authored schema에 content row와 resource를 공급하는 일이다.

## 사용자 검증과 남은 일

에이전트는 Client/UI를 실행·조작하거나 화면을 캡처하지 않았다. 아래 항목은 사용자의 서면
`APPROVED / RETUNE / BLOCKED` 판정 전까지 OPEN이다.

1. 도화가 F Mesh `#4` forward-axis.
2. 도화가 F CascadeRibbon `#3`.
3. 도화가 F LocalDecal `#20/#21`, crack `#22`와 Combined.
4. Lance BA1~BA4 Main Mesh, CascadeRibbon, AnimationTrail과 단계별 Combined.
5. 차원술사·도화가·창술사 BA 12단계의 실제 animation timing과 합성.
6. All/Family/Solo를 유지한 stable Element 선택과 BA full Effect Details 편집,
   `Save As Authored Copy`, Save/Reload의 실제 Tool 흐름.
7. 도화가 F의 `Play Core F -> #20/#21/#3 exact inspection -> Tune Transform`과 선택 Decal/Trail의
   generic Authored starting copy 생성·편집·Save/Reload 흐름.

Track A는 위 항목을 원작 exact로 더 파는 방향이 아니라 현재 evidence/starting recipe에서 동결한다.
사용자는 Track B에서 DDS/WModel과 각 family Element를 조합·튜닝해 완성한다. 이후 같은 schema에 다른
캐릭터와 Valtan content를 공급하고, 마지막으로 binary packing, draw submission, resource sharing,
buffer/instance reuse와 prewarm을 제품 성능 단계에서 닫는다.

## 수행하지 않은 것

- Client/UI 자율 실행·조작·캡처
- 사용자 대신 visual fidelity 판정
- LocalDecal native MRT exact 선언
- Lance Ribbon GPU/visual PASS 선언
- 전체 캐릭터·Warlord·Valtan product content admission 선언
- 도화가 adapter packet을 일반 `.effect.json`이 그대로 보존한다는 선언
- 사용자 assertion 때문에 중단한 GPU/WARP 경로의 재실행 또는 PASS 선언
- shared dirty worktree stage/commit/push

ProjectAudit는 shared dirty worktree에서 관련 도구가 기존부터 삭제된 상태이므로 복원하거나 완료
증거로 사용하지 않았다.
