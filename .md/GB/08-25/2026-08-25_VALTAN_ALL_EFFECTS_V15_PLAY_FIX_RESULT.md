# 2026-08-25 Valtan All Effects v15 Play 최소 수정 결과

## 1. 결론

최신 `origin/main`의 Effect Tool과 All Effects를 유지한 채, codec이 정상 지원하는 Valtan authored v15
문서를 Play identity gate에서 다시 v13과 비교해 거절하던 중복 판정을 제거했다. `Data/Valtan` 다섯 split
source와 현재 animation/effect Product는 수정하지 않았다. 도화가 A와 Effect 구조 정리·삭제도 포함하지 않았다.
코드·데이터 자동 검증과 Client Debug/Release 전체 compile/link를 완료했다. 사용자 육안 검증은 별도 대기 상태다.

| 구분 | 판정 |
|---|---|
| 기준 commit | `b643d3507aec88ff19c3daed015d5bec2142fdfb` (`origin/main`) |
| Valtan runtime set validate | PASS, managed pattern 7 / managed cue 15 / combat object 2 |
| Valtan runtime set 회귀 | PASS, 37/37 |
| Animation Tool Valtan master | PASS, 7/7 |
| Valtan pattern tree | PASS, 16/16 |
| Valtan Balance Tool | PASS, 22/22 |
| All Effects Valtan saved-row 회귀 | PASS, 29/29 |
| Effect source closure | PASS, direct 175 / resource 1,025 / generated artifact 0 |
| Effect project registration | PASS, files 2,349 / filters 219 |
| 변경 translation unit Debug/Release | PASS, 오류 0 |
| Client 전체 compile/link | PASS, x64 Debug/Release 오류 0 |
| 사용자 visual fidelity | 대기 — 에이전트가 대신 판정하지 않음 |

## 2. 원인

현재 Product 연결은 다음과 같이 이미 닫혀 있었다.

```text
Data/Valtan split source
  -> Valtan publisher의 pattern/animation/Effect Product projection
  -> EffectCatalog.json의 DIRECT_AUTHORED_DOCUMENT row
  -> Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json (version 15)
```

`CEffectDocumentCodec::Load`, catalog와 direct source index는 이 v15 문서를 수용한다. 그러나
`CEffect_Tool::Resolve_DirectAuthoredEditablePath`가 Load 성공 뒤에도
`Document.iLoadedFormatVersion == EFFECT_AUTHORING_FORMAT_VERSION`을 추가로 요구했다.
`EFFECT_AUTHORING_FORMAT_VERSION`은 13이므로 실제 Valtan 420633 문서만 All Effects Play 직전에
거절됐다. Player v13 문서는 같은 비교를 통과해 정상으로 보였기 때문에 Valtan에만 무표시처럼 나타났다.
Valtan의 `Open Editor`는 이 resolver를 거치지 않고 exact authored path를 직접 로드하므로 차단 대상이
아니었다.

## 3. 변경

- `Client/Private/Effect_Tool.cpp`
  - codec Load 뒤의 v13 equality를 제거했다.
  - Tool은 catalog row와 embedded `effectAssetId` 일치만 추가 확인한다.
  - Open/Play 공용 resolver 상태 문구에서 v13 전용 표현을 제거하고 중립화했다.
- `Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py`
  - 최종 Play identity gate가 codec version 계약을 복제하지 않는지 검사한다.
  - Valtan Play가 `Try_PlayUnifiedEffect`를 거쳐 이 identity gate에 도달하는지 검사한다.
  - 실제 `effect.valtan.pattern.420633.active` catalog row가 direct authored이고 문서 version 15이며
    embedded ID가 일치하는지 검사한다.

codec은 기존대로 supported version과 runtime-extension 구조를 검사하고 native v14 source contract를 거절한다.
따라서 이번 변경은 version 검증을 없앤 것이 아니라 한 곳의 codec 계약으로 단일화한 것이다.

## 4. Data/Valtan 정본 검증

`Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate` 결과는 다음과 같다.

```text
managedPatterns     7
legacyPatterns     26
combatObjects       2
projectedArtifacts  9
worldMembers       30
sourceRevision      297420d0ddbbaef04b76d827d5e7eb70db5b15910cdd07a10b0958f40bb2939a
```

현재 데이터가 이미 정합하므로 `Data/Valtan`, `Data/Animation/Authored/Valtan`,
`Data/Effects/EffectCatalog.json`과 authored Effect JSON은 다시 쓰지 않았다.

## 5. 자동 검증

```text
Test-ValtanTuningRuntimeSet.ps1                    PASS, 37/37
test_animation_tool_valtan_pattern_master.py       PASS, 7/7
test_valtan_pattern_tree_contract.py                PASS, 16/16
test_valtan_balance_tool_contract.py                PASS, 22/22
test_effect_tool_valtan_saved_rows.py               PASS, 29/29

Validate-EffectSources.ps1
  directSourceCount              175
  unboundReferenceCount          265
  sourceBytes             73,288,798
  resourceFileCount            1,025
  resourceBytes           73,709,096
  generatedArtifactCount          0

Sync-EffectDataProject.ps1 -Check                   PASS, files=2349 / filters=219
Effect_Tool.cpp x64 Debug ClCompile                 PASS, 오류 0
Effect_Tool.cpp x64 Release ClCompile               PASS, 오류 0
Client x64 Debug compile/link                       PASS, 오류 0
Client x64 Release compile/link                     PASS, 오류 0
git diff --check                                    PASS
```

변경은 기존 함수의 boolean 조건 하나를 줄이는 것으로 새 symbol이나 link dependency를 만들지 않는다.
해당 translation unit 단독 compile에 더해 clean worktree에서 Client x64 Debug/Release 전체 compile/link를
각각 완료했다. 기존 C4819와 외부 DirectXTK PDB 경고는 있었지만 새 compile/link 오류는 없었다.

## 6. 사용자 수동 검증

에이전트는 Client/UI를 실행·조작하거나 visual PASS를 대신 판정하지 않았다. PR을 받은 뒤 사용자가 다음을
직접 확인한다.

1. 필요하면 `git lfs pull`을 실행한다.
2. Visual Studio에서 `Server + Client` profile을 실행한다.
3. Lobby에서 Server 승인을 거쳐 Valtan에 진입한다.
4. `F1 -> Effect Tool -> All Effects -> Valtan`에서 `effect.valtan.pattern.420633.active`를 Open Editor로
   확인한 뒤 `Play Saved Effect`를 누른다.
5. 420633 휠윈드의 발탄 animation과 Effect가 함께 보이는지 확인한다.
6. 실제 Valtan pattern 재생에서도 같은 cue가 보이는지 확인한다.

사용자의 서면 관찰 전에는 manual visual fidelity를 PASS로 기록하지 않는다.

## 7. 제외 범위

- 도화가 A 재생 문제
- Effect Tool / All Effects 제거 또는 재구성
- Effect 폴더·검증 도구·하네스 삭제
- Valtan 패턴 수치, animation sequence, effect authored 값 재튜닝
- main 또는 다른 작업 worktree의 정리
