# 차원술사 체험 모드 스킬 바인딩·Effect Resource Index 구현 계획

## 1. 현재 실제 반영 상태와 이번 경계

정본은 `origin/main`과 동일한 다음 두 문서다.

- `Data/Balance/PlayerSkills.json`: `(characterClass, inputSlot) -> skillId/effectId`
- `Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json`:
  `skillId -> 실제 CModel clip sequence`

현재 체험 모드 11슬롯 중 `A/S/D/T/ALT_V`만 `effectId`가 있고,
`Q/W/E/R/F/V`는 `effectId`가 비어 있다. 기존 Effect 추출 문서의 옛 `inputSlot` 이름을 근거로
새 슬롯에 자동 재배치하지 않는다. 예를 들어 기존 `2050190 R`을 현재 정본의 `2050180 R`로
추측 연결하면 애니메이션과 Effect가 다시 어긋난다.

이번 경계는 각 슬롯 폴더에 바이너리 복사본을 만드는 것이 아니다. 정본 Effect 문서가 실제 참조하는
Resources-relative Mesh/Texture/Model asset ID와 Material identity를 중복 제거한 JSON index로 만든다.
Effect Tool 동적 Resource UI는 이 index를 소비하는 다음 수직 슬라이스로 남긴다.

## 2. 파일과 역할

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_skill_effect_resource_index.py` | PlayerSkills → skillbindings → Authored Effect 순서로만 슬롯을 resolve하고 JSON index 생성 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/test_skill_effect_resource_index.py` | 추측 연결 금지, asset 중복 제거, Material identity, 물리 리소스 확인, JSON-only 출력 회귀 |
| 추가 | `C:/Users/user/Desktop/LostArk/Data/Effects/ResourceIndex/DimensionMaster/DimensionMaster.skill-effect-resource-index.json` | 11슬롯과 per-slot manifest 경로의 class index |
| 추가 | `C:/Users/user/Desktop/LostArk/Data/Effects/ResourceIndex/DimensionMaster/<SLOT>/DimensionMaster.<SLOT>.effect-resources.json` | 슬롯의 animation clips, effect binding 상태, exact resource/material 참조 |

`Client/Bin/Resources`의 `.wmodel/.dds`는 읽기 전용 존재 검증 입력이며 이 변경에 복사하거나 Git에
추가하지 않는다. C++ 파일과 project/filter 등록 변경도 없다.

## 3. 생성기 계약

### `build_resource_index`

한 줄 책임: 체험 모드 슬롯을 두 정본과 Authored Effect 문서에 순서대로 결합한다.

```text
PlayerSkills class/inputSlot 유일성 검증
→ skillbindings skillId 유일성 검증
→ effectId가 있을 때만 Authored/<effectId>.effect.json resolve
→ effectAssetId 동일성 검증
→ modelCues[].modelAssetId 수집
→ elements[].resources[].assetId 수집
→ elements[].material의 source/profile/template identity 수집
→ asset ID별 occurrence/slot/element/cue 역참조 집계
→ optional Resources root에서 PRESENT/MISSING 판정
→ class index와 11개 slot manifest를 모두 메모리에서 완성
```

`effectId`가 비어 있으면 같은 `skillId` 이름의 파일이 있어도 찾지 않는다. 상태는 다음 중 하나다.

```text
INDEX_READY
MISSING_CANONICAL_SKILL
MISSING_ANIMATION_BINDING
MISSING_EFFECT_BINDING
MISSING_EFFECT_DOCUMENT
MISSING_RUNTIME_RESOURCE
```

### `collect_effect_contract`

한 줄 책임: 하나의 완성 Effect 문서에서 실행 리소스와 Material identity를 손실 없이 집계한다.

- `.wmodel`, `meshModel`은 `mesh`; Model Cue는 `model`이다.
- `.dds/.png/.tga/.bmp`는 `texture`다.
- 같은 asset은 한 행이며 `occurrenceCount`, `elementIds`, `modelCueIds`, `slotIds`로 모든 사용처를 보존한다.
- Material은 `templateId/sourceMaterialPath/profileId/parentMaterialPath/runtimeShaderProfileId/
  semanticStatus` 조합으로 식별한다.
- 절대 경로, drive-qualified ID, `..` 탈출, 대소문자 충돌은 fail-closed한다.

### `write_resource_index`

한 줄 책임: 완성된 문서를 임시 파일에 기록한 뒤 `os.replace`로 manifest 단위 원자 교체한다.

출력 폴더에는 JSON만 생긴다. Mesh/Texture/Material payload를 복제하지 않는다.

## 4. 현재 정본에서 예상되는 결과

| 슬롯 | skillId | clip 정본 | effectId 상태 |
|---|---:|---|---|
| Q | 2050100 | `nailstrike_01` | 누락 |
| W | 2050120 | `dimensionalleap_01~03` | 누락 |
| E | 2050160 | `overslash_01~05` | 누락 |
| R | 2050180 | `foldcut` | 누락 |
| A | 2050210 | `willowrend` | 연결됨 |
| S | 2050220 | `momentaryrift` | 연결됨 |
| D | 2050240 | `telekinesisthrust_01/04` | 연결됨 |
| F | 2050230 | `chronorecoil` | 누락 |
| T | 2050500 | `dimensionprison` | 연결됨; Summon Model Cue 포함 |
| V | 2050520 | `timewave` | 누락 |
| ALT_V | 2050540 | `super_timewave` | 연결됨 |

따라서 이 index 완료가 11개 Effect 복원 완료를 뜻하지 않는다. 먼저 누락 6슬롯의 원본 Notify/Cascade를
현재 skillId와 clip sequence 기준으로 다시 materialize하고 `PlayerSkills.effectId`를 명시해야 한다.

## 5. 적용·검증

```powershell
python Tools/LevelPlacementExtractor/build_skill_effect_resource_index.py `
  --player-skills Data/Balance/PlayerSkills.json `
  --skill-bindings Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json `
  --authored-root Data/Effects/Authored `
  --repository-root . `
  --resource-root Client/Bin/Resources `
  --character-class DIMENSIONMASTER `
  --slots Q,W,E,R,A,S,D,F,T,V,ALT_V `
  --output-root Data/Effects/ResourceIndex/DimensionMaster

Set-Location Tools/LevelPlacementExtractor
python -m unittest test_skill_effect_resource_index.py
```

완료 증거는 11개 slot manifest, 내부 asset 중복 0, bound Effect의 physical missing 0, 두 번째 생성의
`changedFileCount=0`이다. `--require-complete`는 누락 6슬롯이 실제로 연결되기 전까지 의도대로 exit 1이다.
