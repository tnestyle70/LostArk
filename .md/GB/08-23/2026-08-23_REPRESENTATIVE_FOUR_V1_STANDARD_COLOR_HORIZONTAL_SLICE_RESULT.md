# 2026-08-23 대표 4스킬 V1 StandardColor 수평 슬라이스 결과

기준 main: `79b5ba042d9351198e517ff27c464fb1a1538190`

작업 branch: `codex/representative-four-v1-material-slice`

최종 화면 판정자: 사용자

## 0. 결론

정본 `C:\Users\user\Desktop\LostArk`에 대표 4스킬의 V0/V1 병렬 audition 문서와 Debug/Release
Client 실행물을 준비했다. 기존 V0 Product cue는 바꾸지 않았고 V1은 Effect Tool에서만 명시적으로
열고 재생할 수 있다.

이번 변경이 닫은 것은 `stable occurrence -> typed packet -> registry -> immutable catalog/prewarm ->
prepared document -> 기존 Sprite/Mesh/Decal actual draw` 실행 배선이다. 131개 occurrence를 한 종류의
`StandardColorV1/opcode 1` 근사식으로 연결했으며, skill/class ID renderer switch는 추가하지 않았다.

이번 변경이 닫지 않은 것은 원본 material graph의 계산식이다. 131개 전부
`PROJECT_TUNED_APPROX`이고 formal `SOURCE_EXACT`는 `0`이다. 따라서 사용자 화면 검증은 “원작 복원
PASS”가 아니라, 이 공용 근사식이 실제 4스킬에서 어느 요소까지 보존하고 어느 요소를 평탄화하는지
판정하는 audition이다.

## 1. 생성한 V1 비교 문서

| 입력 | V0 | V1 | occurrence |
|---|---|---|---:|
| 차원술사 R | `effect.dimensionmaster.skill.2050180.unified` | `effect.dimensionmaster.skill.2050180.v1.unified` | 10 |
| 도화가 A | `effect.artist.skill.31460.unified` | `effect.artist.skill.31460.v1.unified` | 18 |
| 창술사 D | `effect.lancemaster.skill.34110.unified` | `effect.lancemaster.skill.34110.v1.unified` | 88 |
| 워로드 R clip2 | `effect.warlord.skill.17110.clip2.unified` | `effect.warlord.skill.17110.clip2.v1.unified` | 3 |
| 워로드 R clip3 | `effect.warlord.skill.17110.clip3.unified` | `effect.warlord.skill.17110.clip3.v1.unified` | 12 |

합계는 Sprite 98 + Mesh 25 + Decal 8 = 131이다. V1은 V0의 element stable ID, composition,
carrier, timing과 resource identity를 보존한 별도 문서다.

도화가 F control은 기존 `effect.artist.skill.31470.unified`다. 이번 변경은 Artist F Full35를
수정하거나 대표 4스킬의 Product cue를 교체하지 않았다.

## 2. 실제로 닫힌 runtime 계약

- Registry: Program 1 / Layout 6 / Descriptor 131 / Binding 131을 추가했다.
- 전체 runtime catalog: 150 entries / 134 bindings / 5 registry-bound audition effects다.
- Adapter는 skill ID가 아니라 `carrier × render profile × layout × MRT`로 선택한다.
- Sprite 4종, Mesh 2종, Decal 2종의 generic compiled Adapter 8종을 실제 draw 경계에 연결했다.
- registry packet과 inline mirror는 ordered field, lane/scalar/vector/sampler와 float32 bit까지 비교한다.
- Binding이 있는 occurrence의 packet, carrier, adapter 또는 draw 계약 불일치는 fail-closed다.
- registry generation/revision은 catalog에서 prewarm target과 prepared document까지 전달한다.
- MeshParticle SubUV current/next atlas transform과 blend를 기존 CModel draw에 전달한다.
- Binding 0인 기존 V0 문서는 기존 draw path를 유지한다.
- Product 문서에 StandardColor V1이 들어가거나 audition marker/binding/seal을 제거하면 publisher가
  거부한다. six playable owner inventory에 Artist, DimensionMaster, GunSlinger, LanceMaster, Slayer,
  Warlord를 모두 포함했다.

## 3. Artist F와 이번 V1의 정확한 차이

Artist F 현재 Product는 17 elements와 12종 backend/opcode를 사용한다. Track A의 S6/M3/D14는 그중
3개 기존 typed inline packet을 Registry packet으로 옮기고 두 packet의 모든 float bit가 같은지를
증명한 migration이다. 이 `exact dual-resolve`는 source shader 전체가 exact하다는 뜻은 아니다.

이번 131개 V1도 inline/registry bit identity는 증명하지만 비교 기준이 새로 만든 StandardColor V1
packet이다. V0 sourceProfile이나 원작 shader packet과 비교한 것이 아니다.

현재 공용 식은 다음 경계만 계산한다.

```text
rgb   = selected base radiance × carrier color × optional emissive intensity
alpha = selected coverage × carrier lifetime alpha × optional dissolve
RT1   = zero distortion
```

원본의 UV panning, noise/distortion, fresnel, dynamic parameter, scalar/vector/static switch, scene input,
mesh WPO와 occurrence별 material graph는 평가하지 않는다. 따라서 이 화면이 평탄하거나 사각형으로
보이면 adapter 배선 실패가 아니라 material equation cohort가 아직 열리지 않은 것이 우선 원인이다.
Valtan ledger도 같은 한 식으로 묶으면 동일한 문제가 생기므로 이번 증거를 Valtan source-exact proof로
승격하지 않는다.

## 4. 자동 검증

- Python registry/materializer focused tests: `29/29 PASS`
- deterministic materializer check: `occurrences=131 fallback=11 changed=no`
- publisher validate: `150 entries / 134 bindings / 5 audition effects PASS`
- `Test-EffectPipeline.ps1`의 publisher/admission/rollback fixture와 GunSlinger/Slayer Product
  promotion negative: PASS
- Debug focused runtime harness: PASS
  - `actualBindingCount=134`
  - `representativeV1RegistryBindingCount=131`
  - `coveredCompiledAdapterCount=8`
  - `actualCompiledAdapterDrawCount=8`
  - `representativeV1ActualDrawCount=6`
  - Mesh SubUV Binding 0/1 candidate/material/draw/validation `5/5/5/5`
- Release focused runtime harness, `ExpectedBindingCount=134`: PASS
  - prewarm binding/resolved `134/134`
  - compiled Adapter covered/actual draw `8/8`
  - representative V1 actual draw `6`
  - catalog revision / registry generation `1/1`, rollback validated
  - Artist F Full35 attempted/submitted/suppressed/failed `25/22/3/0`, committed
  - Lance BA1 floor 0.5207s attempted/submitted/suppressed/failed `1/0/1/0`, committed
  - Lance BA1 tick32 attempted/submitted/suppressed/failed `1/1/0/0`, committed
- Client x64 Debug build/link: PASS
- Client x64 Release build/link/runtime deploy: PASS

Focused runtime harness는 131개 전부를 GPU draw한 것이 아니다. 131개 전부 resolve/prewarm을 검사하고,
대표 6개 V1 draw와 Artist F golden draw를 합쳐 compiled Adapter 8종의 실제 carrier/pass/state/MRT 경계를
검사한다.

전체 pipeline script의 후속 global Python suite는 `177 tests / 1 failure / 6 errors / 1 skipped`로
exit 1이었다. 이번 V1 focused 경로가 아니라 최신 main의 세 기존 evidence 불일치다. Artist F raw
authored 문서의 CRLF/LF 종속 byte assertion(canonical JSON과 17 elements는 동일), Warlord 17140 WPO
receipt stale 1건, Valtan Entrance/Arena84 gap invariant stale 4건이다. 이 RESULT는 전체 suite를
PASS라고 기록하지 않는다.

## 5. 실행물 identity

| artifact | bytes | SHA-256 |
|---|---:|---|
| `Client/Bin/Debug/Client.exe` | 25,995,264 | `BE60028DE42B44D4BB3FDDFBA9EFAC5FB643F791124B9C772465E8C0802CCD57` |
| `Client/Bin/Release/Client.exe` | 4,405,760 | `C82D1910A38467E4C32D5954DCAE22BFA9ED551B4C1C32E84F7810672AD885DA` |
| `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json` | 245,063 | `40714E31DA674B8A0A61421E2E085549A45C54EBB40058E9549FA1306A366BCA` |

## 6. 사용자 Effect Tool A/B 절차

1. `C:\Users\user\Desktop\LostArk\Framework.sln`을 연다.
2. Visual Studio 구성을 `Debug | x64`로 선택한다.
3. launch profile `Server + Client`를 선택하고 `Ctrl+F5`로 실행한다.
4. Lobby에서 `F1`을 누르고 `Effect Tool`을 클릭한다. `Effect Tool v2`가 아니다.
5. `All Effects` 창의 `Character / Boss`에서 해당 캐릭터를 선택한다.
6. 검색창에 위 표의 정확한 V0 asset ID를 넣고, 일치하는 `Skill | Input ...` 행을 펼친다.
7. `Saved Unified Effects`에서 같은 ID를 펼치고 `Play Saved Effect`를 누른다.
8. 같은 카메라에서 검색창을 V1 asset ID로 바꾸고 같은 방식으로 `Play Saved Effect`를 누른다.
9. 한 element만 보려면 먼저 `Open Saved Effect`를 누른 뒤 `Effect Tool > Current Effect`에서 해당
   element의 `Solo`를 누른다. 전체 family는 `Play Family`를 사용한다.

대표 4스킬 V1은 Product에 연결하지 않았으므로 캐릭터의 R/A/D 키만 눌러서는 V1이 재생되지 않는다.
키 입력은 계속 V0 Product다. 도화가 F 기존 control만 확인하려면 Artist 캐릭터에서 F를 눌러도 되지만,
대표 V1 비교는 반드시 위 Effect Tool 경로를 사용한다.

## 7. 화면 판정 뒤의 분기

- 근사식이 충분한 element는 stable descriptor/binding과 generic Adapter를 유지한다.
- 평탄화·사각형·소실이 보이는 element는 45 child material path / 27 parent path를 ShaderMap과 static
  permutation 기준으로 cohort화한다.
- equation이 같은 occurrence만 Program/Layout을 공유하고, noise/dissolve/emissive/distortion/fresnel,
  dynamic ABI, mesh WPO 등 필요한 typed opcode를 category별로 추가한다.
- 손튜닝을 선택해도 skill-ID C++ switch를 만들지 않고 stable occurrence data Binding에
  `PROJECT_TUNED_APPROX` Program/Descriptor를 연결한다.

사용자 visual PASS는 아직 선언하지 않았다. 이 결과 문서는 실행 준비와 자동 계약만 완료한 상태다.
