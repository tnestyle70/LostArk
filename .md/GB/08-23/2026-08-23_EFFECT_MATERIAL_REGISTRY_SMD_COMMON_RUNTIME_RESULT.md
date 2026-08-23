# 2026-08-23 Effect Material Registry S/M/D 공용 Runtime 결과

기준 branch: `codex/effect-v1-visual-matrix`

기준 main: `d6b27084`

최종 화면 판정자: 사용자

## 1. 완료 범위

공용 registry가 실제로 허용하는 compiled tuple을 다음 세 개로 고정했다.

| carrier | backend/opcode | pass/state | golden element |
|---|---|---|---|
| SpriteParticle | `runtimeMaterialV2/6` | pass 1, alpha two-sided | `sprite.2b3dc6842507e910` |
| MeshParticle CModel | `runtimeMaterialV2/3` | pass 1, alpha two-sided | `mesh.062366ee9f9655d3` |
| LocalDecal Projector | `localDecal/14` | pass 3, alpha one-sided | `decal.f3b5c3b63b4a7e34` |

- base registry는 compiled Adapter 세 개만 소유한다.
- Artist F의 Program/Layout/Descriptor/Binding은
  `Fragments/artist-f-golden.material-program-fragment.v1.json`이 소유한다.
- 세 Binding은 모두 `INLINE_MIRROR_REQUIRED`다.
- `REGISTRY_AUTHORITATIVE`, opcode 17/19/23+와 증명되지 않은 state variant는 admission하지 않는다.
- Binding이 없는 occurrence는 기존 V0 경로를 유지한다.
- carrier, renderer profile 또는 inline packet이 불일치하면 catalog generation을 commit하지 않고 이전
  generation을 유지한다.

## 2. 실제 runtime 연결

기존 `EFFECT_MATERIAL_EXECUTION_DESC`와 `Effect_DocumentRenderer` draw 경로를 그대로 사용한다.
새 renderer나 두 번째 material runtime은 만들지 않았다.

```text
base + domain fragment merge
-> Program/Layout/Descriptor/Adapter/Binding validate
-> exact Program x Layout ABI receipt validate
-> authored inline packet float32 bit-exact compare
-> immutable catalog/registry stage and commit
-> existing Sprite/Mesh/Decal carrier draw
-> actual pass/state/MRT/SRV receipt
```

LocalDecal은 `Target_Depth`의 실제 raw SRV가 pixel shader `t14`에 bind됐는지까지 조회한다.
Mesh와 Decal은 같은 Artist F occurrence를 Binding 0 inline과 Binding 1 registry 두 모드로 각각 실제
draw했다.

## 3. 자동 검증

### Python, schema, publisher

- registry builder tests: `15/15 PASS`
- direct-authored runtime tests: `6/6 PASS`
- `Publish-Effects.ps1 -Mode Validate`: `145 entries / 3 bindings PASS`
- publish 산출물: Program 3 / Layout 3 / Descriptor 3 / Adapter 3 / Binding 3
- JSON/XML parse: PASS
- 공용 fragment의 `.vcxproj`와 `.filters` exact registration: PASS
- `git diff --check`: 오류 없음

### Debug

- Engine x64 Debug: PASS
- `UpdateLib.bat Debug`: PASS
- EffectRenderContractHarness x64 Debug build/run, `ExpectedBindingCount=3`: PASS
- Client x64 Debug: PASS

### Release

- Engine x64 Release: PASS
- `UpdateLib.bat Release`: PASS
- EffectRenderContractHarness x64 Release build/run, `ExpectedBindingCount=3`: PASS
- Client x64 Release build/link/runtime deploy: PASS

Release의 첫 전체 빌드는 C 드라이브 여유 공간이 0이 되어 PDB 갱신이 실패했으나, 재생성 가능한
intermediate를 정식 Clean한 뒤 같은 소스로 재실행해 최종 exit code 0과 `Client.exe` 링크 및 runtime
DLL 배포를 확인했다.

## 4. Binding 0 대 Binding 1 실행 증거

| carrier | pass | draw | Binding 0 world FNV | Binding 1 world FNV | 결과 |
|---|---:|---:|---:|---:|---|
| Mesh CModel | 1 | 1 | `1415597879498542891` | `1415597879498542891` | packet/snapshot/draw 동등 |
| LocalDecal | 3 | 1 | `11436688068424083471` | `11436688068424083471` | packet/snapshot/draw 동등 |

Sprite golden은 기존 bound actual draw, pass 1, packet/snapshot receipt를 유지했다. Debug와 Release
모두 catalog carrier/profile-invalid reload rollback과 세 compiled Binding을 동일하게 검증했다.

## 5. 이번 PR 밖의 상태

- Valtan 669 ledger, Valtan fragment와 pattern Binding은 이 공용 PR에 포함하지 않는다.
- masked Mesh opcode 19의 pass 7 fixed-state 검사는 actual bound CModel admission 증거가 아니다.
- opcode 17 distortion, opcode 23+, WPO, Glass/MRT, Ribbon과 Presentation은 별도 capability PR이다.
- `Test-EffectPipeline.ps1` 전체 baseline에는 이 변경과 무관한 기존 5개 실패가 남아 있다. Warlord
  17140 WPO receipt, Valtan source inventory gap 및 그에 종속된 carrier/family/portal-rush 검사다.
- 전체 `Sync-EffectDataProject.ps1 -Check`는 main에 누적된 과거 Effect Data 등록 누락까지 349개를
  한꺼번에 요구하므로 이번 공용 PR에는 그 광범위한 정리를 넣지 않았다. 이번에 추가한 Artist F
  fragment만 정확히 한 번 등록했다.
- Client를 에이전트가 실행하거나 화면을 판정하지 않았다. Product visual PASS는 사용자의 실제
  Server+Client 관찰 후에만 기록한다.

## 6. 다음 단계

공용 PR 병합 뒤 최신 main에서 Valtan 전용 branch를 만든다.

```text
Valtan 669 전수 ledger 정본화
-> 휠윈드 / 돌진 / 도넛 pattern occurrence join
-> S6/M3/D14 exact eligible row만 Binding
-> Descriptor 또는 ABI가 안 닫힌 row는 fail-closed blocker 유지
-> Debug Server + Client 실행 준비
-> 사용자가 실제 세 pattern을 관찰
-> 결과를 다음 opcode/capability 우선순위로 환류
```
