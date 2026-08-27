# 2026-08-28 Valtan Boss Tool -> Effect Tool exact Product handoff 구현 계획

## 목표

Boss Tool에서 현재 선택한 발탄 stage의 실제 Product Effect occurrence를 stable ID로 Effect Tool에
전달한다. Effect Tool은 자기 joined tree와 direct-authored index에서 동일 tuple을 다시 검증한 뒤,
이미 존재하는 unified Effect editor와 Model View authoring timeline을 연다.

```text
Boss Tool selected pattern/stage/Product cue
-> patternId + stageId + cue occurrenceId + effectAssetId
-> CMainApp one-shot routing
-> Effect Tool fresh strict join
-> exact authored Effect open
-> clip-bound cue: complete Product animation timeline paused at t=0
-> STAGE_CLOCK cue: explicit standalone/static Valtan target
```

Model View를 새 정본이나 두 번째 재생기로 만들지 않는다. Effect element/material/resource/lifetime은
계속 `Data/Effects/Authored/*.effect.json`, cue owner/timing/attachment는 계속
`Data/Valtan/Valtan.presentation.json`이 소유한다.

## 현재 실측

- Effect Tool은 `Build_ValtanProductPreview -> Try_OpenValtanAuthoredEffect`로 exact clip occurrence와
  전체 pattern clock을 이미 동기화한다.
- 도넛처럼 `STAGE_CLOCK`인 cue는 `Try_OpenValtanStandaloneEffect`로 명시적으로 animation 없는
  저작 target을 사용한다.
- Boss Tool은 같은 joined tree에서 animation/Effect 연결을 표시하지만 Camera Tool deep-link만 있고
  Effect Tool 요청은 없다.
- MainApp도 Boss -> Camera one-shot 요청만 라우팅한다.
- 따라서 새 animation/effect runtime을 만드는 것이 아니라, exact Product identity handoff가 빠진 것이
  구조적 공백이다.

## G01. stable-ID open request와 Effect Tool exact resolve

수정 파일:

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`

public request는 `patternId`, `stageId`, `cueOccurrenceId`, `effectAssetId`만 전달한다. Effect Tool은
요청 때 tree와 direct-authored index를 갱신하고 pattern/stage/cue/effect tuple이 정확히 하나인지 검증한다.
clip-bound cue는 owner clip과 가능한 branch path를 exact resolve해 기존 Product preview를 열고,
`STAGE_CLOCK` cue는 기존 standalone 경로를 연다. Open은 Play를 자동 실행하지 않는다.

## G02. Boss Tool one-shot 요청과 MainApp routing

수정 파일:

- `Client/Public/BossTool.h`
- `Client/Private/BossTool.cpp`
- `Client/Private/MainApp.cpp`

Boss Tool의 선택 stage에 있는 각 Product cue 옆에 `Edit Linked Effect`를 제공한다. 버튼은 포인터,
배열 index, authored path를 넘기지 않고 stable tuple만 one-shot 요청으로 보관한다. MainApp은 기존
Camera routing과 같은 위치에서 Effect Tool을 lazy-create하고 요청을 전달한다.

## G03. 계약 검증과 인계 문서

수정 파일:

- `Tools/ValtanPipeline/test_valtan_boss_tool_contract.py`
- `Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py`
- `.md/TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md`
- `.md/TEAM/발탄인수인계서.md`

계약 테스트는 Boss Tool이 stable tuple만 제출하는지, MainApp이 Effect Tool로 라우팅하는지,
Effect Tool이 fresh tree/direct-authored index에서 unique tuple을 재검증하고 기존 synchronized/standalone
경로만 사용하는지 고정한다.

## 범위 밖

- 도넛에 검증되지 않은 `19_02/19_04` animation을 임의 복원하지 않는다.
- `NONE + STAGE_CLOCK`을 clip-bound cue로 자동 변환하지 않는다.
- Model View에 별도 저장 정본이나 두 번째 Effect 재생기를 만들지 않는다.
- Server gameplay, damage, wall navigation은 이번 Effect authoring handoff에서 변경하지 않는다.

## 검증

```powershell
python Tools/ValtanPipeline/test_valtan_boss_tool_contract.py
python Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py
python Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate
MSBuild.exe Client/Default/Client.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m /nologo
git diff --check
```

에이전트는 Client/UI를 실행·조작하지 않는다. 사용자는 빌드된 Debug Client에서 Boss Tool의 선택 stage
Product cue를 `Edit Linked Effect`로 열고, clip-bound cue는 Effect Tool Model View에 실제 전체 animation이
0초 pause 상태로 연결되는지, 도넛은 `no animation timeline` 상태가 명시되는지 직접 판정한다.
