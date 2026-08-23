# 2026-08-23 Effect 수동 저작과 Character Select Bloom 결과

## 완료 범위

- Character Select의 scene Bloom multiplier를 Valtan과 같은 `0.85`로 맞췄다.
- Effect Detail을 수동 Element와 SourceRecipe Element의 실제 runtime 소유권 기준으로 분리했다.
  - 수동 Particle: spawn, lifetime, Start/End Size, motion을 직접 편집한다.
  - Source Mesh/Sprite/Decal carrier: Count/Size/Life/Speed/Rotation/Alpha multiplier를 편집한다.
  - Source Decal은 무시되던 `Detail.Decal.vSize` 대신 `Size x`와 실제 overlay인 Projection Depth만 표시한다.
- SourceRecipe Count 배율을 문서 particle budget과 portable event queue 상한에 반영했다.
- Tool preview duration이 source emitter delay/duration/loop와 scaled particle tail을 반영하도록 runtime 계산과 맞췄다.
- `Bloom Intensity`라는 혼동되는 이름을 `Emissive Intensity (HDR)`로 바꾸고 scene Bloom과의 차이를 표시했다.
- 다음 세 저작용 Canary의 UI, Tool 상태, Object/Renderer 실행 분기, 전용 shader/runtime/materializer/test/receipt/project 등록을 제거했다.
  - Exact Cooked Canary
  - Translated Glasshole02 Canary
  - Translated Valtan Core-Three Canary
- generic source material/profile, offline cooked/translation 근거, 공용 DDS/WModel, Artist F 전용 데이터와 제품 경로는 유지했다. 다만 공통 SourceScale budget/preview-tail 계산은 Artist F source element도 사용하므로, 전용 경로 보존과 별개로 31470 수동 smoke가 필요하다.

## 보호 경계

`Data/Effects/Authored/effect.valtan.carrier-v1.attack.four-slash.active.clip-01.effect.json`과 그 runtime catalog/sealed 문서는 먼저 병합된 Valtan 패턴 분리 작업의 정본이다. 이 작업에서는 해당 파일들을 재생성하거나 수정하지 않았다.

## 자동 검증

- Rendering profile Validate: PASS
- authored/runtime Rendering profile semantic parity: PASS
- Effect publisher Validate: PASS, 156 Effects / 171 material-program bindings
- EffectRenderContractHarness Debug: PASS, expected/actual 171 bindings
- focused Effect Tool/Valtan composition tests: PASS, 8 tests
- Canary live symbol/project reference audit: PASS
- Client C++ compile: PASS
- Client link: 실행 중인 `Client/Bin/Debug/Client.exe` 점유로 `LNK1104`; 사용자가 Client를 닫은 뒤 재링크가 필요하다.
- 전체 EffectPipeline PowerShell contract: PASS
- 전체 Python aggregate: 동시 세션이 갱신 중인 Artist F golden, Valtan clip-01 authored/runtime 및 historical inventory 불일치로 2 failure / 9 error. 이번 변경의 focused tests는 PASS했다.
- Effect project registration check: 동시 세션에서 추가한 대량 Effect evidence 파일이 아직 generated project group에 반영되지 않아 stale. 다른 세션 파일을 자동 재작성하지 않았다.
- `git diff --check`: PASS
- 관련 vcxproj/filters XML parse: PASS

## 수동 검증 대기

사용자가 새 Client 빌드에서 다음을 직접 확인해야 한다.

1. Effect Detail에서 세 Canary panel이 사라졌는지 확인한다.
2. Character Select 재진입 후 Valtan과 같은 Bloom halo 정책이 적용되는지 확인한다.
3. Source Decal에서 Size x와 Projection Depth가 반영되고 마지막 tail이 잘리지 않는지 확인한다.
4. 수동 Mesh/Sprite/Particle의 Start/End Size와 Emissive Intensity가 저장·재로드 뒤 유지되는지 확인한다.
5. Artist F의 기존 제품 표현이 유지되는지 확인한다.

자동 검증은 visual fidelity PASS를 대신하지 않는다.
