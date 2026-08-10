# Artist F 31470 Format-3 Reconstructed Program Publisher 결과

날짜: 2026-08-11

브랜치: `codex/artist-f-format3-publisher-v1`

기준 commit: `eacb58bda2315e858c562677bbf38c17d5d3e785`

## 결론

Artist F 31470의 frozen R2 reconstructed runtime candidate를 별도 Product-false payload로 Effect
runtime catalog에 내장했다. source catalog는 102행, 생성된 runtime catalog는 format 3,
Effect 102행/Component 555행이며 신규 payload는 정확히 1개다.

이 결과는 실행 authority 승격이 아니다. 신규 outer entry, embedded candidate와 publication receipt는
모두 `sourceExact=false`, `runtimeExecution=false`, `product=false`를 보존한다. generic
`IMMUTABLE_COMPILED_IR` 17-key 계약은 수정하지 않았고 Artist 31470용 Authored/Assembly/Compiled
artifact나 generic compiler receipt도 생성하지 않았다. PlayerSkills `effectId`, animevent, C++,
Playback, Presentation, Renderer는 수정하지 않았다.

독립 P1 재감사에서 source row의 `effectAssetId`만 `effect.artist.skill.31471`로 바꾸어도 prepare가
hard-pin한 31470 runtime entry를 내보내던 identity substitution을 재현했다. publisher는 이제 exact
3-key reconstructed source row의 ID를 예약된 31470으로 먼저 제한하고, prepare 반환 ID도 source ID와
exact-equal인지 확인한다. 31471 real-publisher regression은 publish를 거부하며 입력 source와 기존
runtime bytes를 유지하고 temp/backup residue를 남기지 않는다.

두 번째 독립 P1 재감사에서는 source reconstructed row에 같은 owner key를 두 번 기록하면
PowerShell `ConvertFrom-Json`이 마지막 값만 남겨 3-key row처럼 보이게 하던 duplicate-key laundering을
재현했다. 모든 file-backed PowerShell authority JSON은 이제 `ConvertFrom-Json` 전에 embedded C# strict
scanner를 통과한다. scanner는 regex가 아니라 JSON object/array/string/number/literal 전체를 파싱하고,
escape를 decode한 object key를 ordinal set으로 보존한다. 동일 raw key와 `\u0065ffectAssetId`, mixed
primitive token을 지난 deeper array/object의 `owner`/`\u006fwner`가 모두 write/backup 전에 거부된다.

최종 consolidated 감사에서는 예약된 31470 ID가 legacy/generic payload로 우회될 수 있는 runtime/source
dispatch와 public reconstructed validator의 optional current-tool 검증을 재현했다. Python runtime catalog와
PowerShell source catalog는 이제 payload dispatch 전에 31470을 reconstructed kind로 전역 예약한다.
public receipt/entry validator는 항상 현재 tool dependency 3개의 exact canonical SHA를 대조하며 이를 끄는
default/public 인자가 없다. exact legacy, valid generic runtime, valid generic source fallback과 tool row
각 3개를 개별 coordinated reseal한 direct call이 모두 거부된다. generic compiled-IR 경로는
asset-agnostic이고 isolated reconstructed branch만 Artist 31470을 고정한다는 module 설명도 교정했다.

## 실제 생성 결과

| 항목 | 결과 |
|---|---|
| source catalog | `Data/Effects/EffectCatalog.json`, 102 Effects |
| runtime catalog | `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json` |
| runtime shape | format 3, 102 Effects, 555 Components |
| runtime catalog bytes | `26,255,931` |
| runtime catalog SHA-256 | `bf0807ec1b4d975c988ed7e8bb204c6b1713218968be76ea6accb6340e714d29` |
| reconstructed payload count | `1` |
| candidate bytes | `15,072,141`, UTF-8/no-BOM/LF-only |
| candidate raw SHA-256 | `72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849` |
| candidate Git blob | `345ab15bbb76648a650eaa854f18c4cd63cb1556` |
| program SHA-256 | `618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b` |
| reconstructed link SHA-256 | `74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2` |
| receipt self SHA-256 | `5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3` |
| outer publishReceipt SHA-256 | `92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94` |

runtime entry는 합의한 exact ordered outer 10 keys, link 16 keys, receipt 25 keys와 tool row 4 keys를
그대로 가진다. `candidateUtf8Json`을 다시 encode한 bytes의 count/raw SHA와 파싱한 program identity를
모두 재검증했다. runtime catalog 전체에서 `reconstructedRuntimeProgramPath`가 존재하지 않으므로
제품 catalog 소비자는 외부 candidate 파일 I/O를 요구하지 않는다.

## publication tool identity

tool dependency hash domain은 `TRACKED_SOURCE_EOL_CANONICAL_TEXT`다. UTF-8 BOM은 거부하고 LF/CRLF
checkout 차이만 LF로 정규화한 SHA는 다음과 같다.

| role | path | SHA-256 |
|---|---|---|
| candidate builder | `Tools/EffectPipeline/build_artist_31470_reconstructed_runtime_program.py` | `5c207e04952971adb553249540e336ba3ad065719e438a9892c6850d2c989c4e` |
| catalog validator | `Tools/EffectPipeline/build_effect_derived_artifact.py` | `5407c3d0983c3aaf4bf085904ef8d7b5f3e9119ae448703ff7e8f612a1c144fb` |
| publisher | `Tools/EffectPipeline/Publish-Effects.ps1` | `ee4a12cf5cbd63bc9af6b0af18ca37da7631a4b0b6ed1465c95bf99fb9be8825` |

receipt의 tool row를 함께 바꾸고 다시 봉인하는 공격도 current tracked tool identity 대조에서 거부한다.
receipt self digest는 첫 24 keys를, outer digest는 self digest를 포함한 full 25 keys를 결합한다.

## 구현 내용

- candidate 한 파일만 `.gitattributes`의 `text eol=lf`로 고정했다. `git ls-files --eol`은
  `i/lf w/lf attr/text eol=lf`이고 HEAD blob과 worktree raw blob이 모두 `345ab15...`다.
  `core.autocrlf=true` 별도 checkout도 frozen byte count/raw SHA/LF-only를 유지한다.
- Python publisher adapter가 frozen candidate bytes와 builder commit/tree/blob, 13개 input artifact
  projection, resource binding, program identity를 검증한 뒤 exact link/receipt/entry를 만든다.
- source catalog에 exact 3-key Artist 31470 reconstructed row를 추가하고 reserved source ID와 prepare
  output ID를 이중 결합했다. source/runtime dispatch도 같은 ID의 legacy/generic kind를 선제 거부한다.
- PowerShell publisher가 reconstructed row를 별도 strict path로 stage하고 mixed format-3 catalog 전체를
  검증한 뒤 atomic replace한다. generic/legacy payload 경로는 그대로 유지한다.
- `Read-JsonDocument`가 strict UTF-8/no-BOM decode와 duplicate-preserving full JSON scanner를 공통
  boundary로 사용하므로 source catalog, Authored, Assembly, Component, prepared entry와 staged runtime
  catalog 모두 같은 duplicate-key 정책을 적용받는다.
- public reconstructed receipt/entry validator가 tool dependency 3개의 current tracked identity를 항상
  검사한다.
- tracked authoring JSON의 source file identity를 UTF-8/no-BOM LF-canonical text SHA로 통일했다.
  binary/resource/raw artifact hash는 raw bytes 계약을 유지한다.
- focused audit의 Python test count, reserved reconstructed ID, mandatory current tools, strict
  duplicate-key/full-walk, clean-checkout LF, reconstructed source ID와 Product-false 판정을 24-test 계약으로
  갱신했다.

## 자동 검증

| 검증 | 결과 |
|---|---|
| `prepare-reconstructed-runtime-entry` 및 Python unit | PASS, `Ran 24 tests`, `OK` |
| focused `Test-EffectDerivedArtifactPublisher.ps1` | PASS, tests=24, reserved-ID/current-tools/duplicate-key/full-walk/clean-checkout/source-id/product=false/rollback=true |
| legacy `Test-EffectPipeline.ps1` | PASS, LF/CRLF parity, BOM/semantic mutation reject, 기존 publish/rollback |
| actual `Publish-Effects.ps1 -Mode Publish` | PASS, 102 Effects/555 Components, 70.9s |
| actual `Publish-Effects.ps1 -Mode Validate` | PASS, 102 catalog entries, 67.4s |
| independent runtime JSON/digest check | PASS, exact 10/16/25/4 order, raw/program/link/receipt hashes, false gates |
| candidate checkout/index identity | PASS, exact attr plus autocrlf=true clean checkout, 15,072,141 bytes, LF-only, no BOM, frozen SHA/blob |
| atomic residue scan | PASS, temp/backup residue 0 |
| Python/PowerShell/JSON parse | PASS |
| `git diff --check` | PASS |

unit fault coverage에는 candidate CRLF/BOM/raw/program/resource/asset mutation, coordinated payload reseal,
tool SHA coordinated reseal, candidate source 삭제 후 embedded validation, source 31471 identity substitution,
plain/escaped owner duplicate와 deeper mixed-token duplicate traversal, publish prepare/commit fault 두 지점의
기존 output 보존과 residue 0이 포함된다. reserved 31470의 legacy/generic runtime, valid generic source
fallback과 tool dependency 3개 각각의 coordinated reseal direct-call 거부도 포함된다.

## 전체 ProjectAudit

정본 Resources 경로를 지정한 consolidated corrective 이후 전체 ProjectAudit는 306.3초 후 exit 1, 외부 baseline
11건으로 끝났다.
이번 lane의 `effect.derived-artifact-publisher`는 최종 재실행에서 PASS했으며 실패 목록에서 제거됐다.
남은 항목은 다음과 같다.

1. `projects.data-source-visibility`
2. `effect.g09-authoring-world-runtime-boundary`
3. `effect.g09-cross-document-contract`
4. `effect.artist-31470-reconstructed-source-capability`
5. `effect.artist-31470-material-texture-runtime-binding`
6. `effect.artist-31470-exact-dds-runtime-deployment`
7. `effect.artist-31470-wmodel-geometry-contract`
8. `effect.artist-31470-reconstructed-runtime-program`
9. `effect.wfx-component-assembly`
10. `effect.representative-authored-readiness`
11. `effect.four-class-authored-clip-product-exact101`

대표적인 실제 원인은 project/filter 수 불일치, 미빌드 WModel harness, 기존 DDS/WFX/representative
traceback, Artist 31210 manifest/binding stage count 불일치다. 이 publisher 변경의 source/runtime
102행, receipt, rollback 또는 focused audit 실패는 아니다.

## 남은 경계

현재 자동 구현 상태는 catalog publication까지 완료다. integration 적용 뒤 별도 clean worktree에서
builder `--check`, Debug/Release reconstructed parser 4/4와 legacy 7/7을 다시 통과하기 전에는 R2 전체
complete가 아니다. Product 실행과 runtime 눈 검증도 아직
완료가 아니다. catalog consumer가 이 exact Product-false payload를 typed Program으로 parse한 뒤에도
실행 gate는 false여야 하며, 별도 execution admission 결정 없이는 Artist F 스킬에 연결하거나 Renderer로
제출하면 안 된다. 이후 consumer/Playback/Renderer 브랜치가 frozen 3개 tool SHA와 outer/link/receipt
identity를 hard-pin하고 Debug/Release harness를 통과한 다음에만 인게임 F 눈 검증 단계로 넘어간다.
