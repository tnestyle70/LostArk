# 2026-08-25 Valtan All Effects v15 Play 최소 수정 계획서

## 0. 목표와 완료 판정

최신 `origin/main`을 pull한 팀원이 별도 Effect pack이나 sidecar를 만들지 않고 다음 정본 연결을 그대로
소비할 수 있게 한다.

```text
Data/Valtan의 다섯 split source
-> Valtan publisher의 pattern/animation/Effect Product projection
-> EffectCatalog.json의 DIRECT_AUTHORED_DOCUMENT
-> Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json
-> Effect Tool / All Effects의 Play Saved Effect
```

이번 변경은 Effect Tool과 All Effects를 유지한다. 도화가 A, Effect 폴더 정리, 하네스 삭제, V0/V1 데이터
재구성은 포함하지 않는다. `Data/Valtan`과 현재 authored Effect 데이터가 이미 정합하면 데이터를 다시 쓰지 않는다.

코드 변경 완료 조건은 다음과 같다.

1. codec과 catalog가 이미 허용하는 authored v15 문서를 All Effects의 Play identity gate가 거절하지 않는다.
2. v13 Player/Valtan 문서의 기존 Play 경로와 Valtan Open Editor 직접 로드를 바꾸지 않는다.
3. native v14 source contract는 `CEffectDocumentCodec::Load`의 기존 runtime reject를 그대로 따른다.
4. 7개 managed pattern animation과 15개 managed Effect cue의 `Data/Valtan` join 검증이 통과한다.
5. Client Debug/Release가 컴파일·링크되고, 최종 시각 판정은 사용자가 직접 수행한다.

## 1. 실측한 원인

`origin/main`의 다음 두 authored Product 문서는 version 15다.

- `effect.artist.skill.31950.unified`
- `effect.valtan.pattern.420633.active`

`CEffectDocumentCodec::Load`, `CEffectCatalog`과 direct-authored source index는 v15를 정상 수용한다. 그러나
`CEffect_Tool::Resolve_DirectAuthoredEditablePath`만 Load 성공 뒤
`Document.iLoadedFormatVersion == EFFECT_AUTHORING_FORMAT_VERSION`을 다시 요구한다.
`EFFECT_AUTHORING_FORMAT_VERSION`은 13이므로 발탄 420633 정본은 Play 직전에만 거절된다. Valtan의
`Open Editor`는 이 resolver를 거치지 않고 정확한 authored path를 직접 로드하므로 원래부터 별도 경로다.

이중 version 판정을 없애고 codec의 runtime document 계약을 단일 판정자로 사용한다. Load 성공 뒤 Tool이
추가로 확인할 것은 embedded `effectAssetId`와 catalog row의 stable ID 일치뿐이다.

## 2. 수정 파일과 책임

| 경로 | 변경 |
|---|---|
| `Client/Private/Effect_Tool.cpp` | final Play identity gate의 v13 equality와 version-specific 문구 제거 |
| `Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py` | 실제 v15 Valtan Product와 Valtan Play 호출 연결 회귀 추가 |
| 대응 RESULT | 자동 검증과 사용자 수동 판정을 분리 기록 |

`Data/Valtan`, `Data/Animation/Authored/Valtan`, `Data/Effects/EffectCatalog.json`과 authored Effect JSON은
현재 main 정합성이 검증되는 한 수정하지 않는다.

## 3. 반영 코드

기존 `CEffect_Tool::Resolve_DirectAuthoredEditablePath`의 identity 판정은 다음 의미로 교체한다.

```cpp
EFFECT_DOCUMENT_DESC Document;
std::string Error;
Entry.bIdentityValid = CEffectDocumentCodec::Load(
    Entry.Path, Document, Error) &&
    Document.strEffectAssetId == strEffectAssetId;
```

`Load`가 supported version, v14 source-contract reject와 v15 runtime-extension 구조를 기존 codec 계약으로
검사한다. Tool은 같은 version 규칙을 복제하지 않는다.

## 4. 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate
python Tools/ValtanPipeline/test_valtan_pattern_master_v2.py
python Tools/ValtanPipeline/test_animation_tool_valtan_pattern_master.py
python Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py
python Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Sync-EffectDataProject.ps1 -Check
```

그 뒤 Client x64 Debug/Release compile·link와 `git diff --check`를 실행한다. 에이전트는 Client/UI를 실행하지
않는다. 사용자는 `Server + Client` profile로 직접 실행한 뒤 `F1 -> Effect Tool -> All Effects -> Valtan`에서
420633 휠윈드의 `Play Saved Effect`와 패턴 timeline을 확인한다.
