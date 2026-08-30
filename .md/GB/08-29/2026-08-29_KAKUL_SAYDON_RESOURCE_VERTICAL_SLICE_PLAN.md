# 2026-08-29 쿠크세이튼 리소스 추출 및 물리 반영 실행 계획

## 1. 목표

쿠크세이튼 관련 추출 원본과 현재 물리 리소스를 `C:/Users/user/Desktop/LostArk` 원본 checkout 기준으로 다시 실측하고, 다음 다섯 잔여 항목을 기존 추출·복원 파이프라인으로 마감한다.

1. `MN_RPCZ_00` Action 문서가 요구하지만 현재 WModel에 없는 27개 clip의 실제 원본 존재 여부와 skeleton identity를 전수 확인한다.
2. `WP_MN_RPCT_07`, `WP_MN_RPCT_08`의 package export를 다시 확인하고, 원본 animation이 있으면 exact skeleton 기준으로 cook하며 없으면 mesh/material closure와 canonical zero-animation 상태를 명시한다.
3. 맵의 `textureSlotsIncomplete=37`을 실제 파일 누락, authored null, UModel helper slot, 현재 CMaterial이 표현하지 않는 source-only lane으로 분해한다.
4. 맵 geometry를 MapCatalog의 `runtimeAssetRoot=Map/LV_LUT_MIDNIGHTC_ED`와 같은 물리 경로로 무손실 정리한다.
5. catalog 292개, placement 2,951개, install receipt, Resources-relative identity, JSON과 publisher/harness를 재검증한다.

## 2. 정본 identity와 물리 경로

- Area와 map asset의 stable identity는 `LV_LUT_MIDNIGHTC_ED`다.
- `KakulSaydon`은 사람이 읽는 collection alias다. Area ID, catalog ID 또는 map runtime root를 대체하지 않는다.
- 최종 map geometry root는 `Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED`다.
- 사용자가 관리하는 character/effect/sound collection은 각각 다음 물리 root를 유지한다.
  - `Client/Bin/Resources/Character/KakulSaydon/<canonical-package>`
  - `Client/Bin/Resources/Effect/KakulSaydon/...`
  - `Client/Bin/Resources/Sound/KakulSaydon/<canonical-bank>`
- `Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/Sound`는 만들지 않는다.
- JSON과 catalog의 runtime asset ID는 항상 `Client/Bin/Resources` 상대 POSIX 경로다.

## 3. 소유권과 금지 경계

- 다른 세션의 미커밋 C++, JSON, project/filter 변경을 보존한다.
- reset, checkout, 대규모 정리, 기존 사용자 파일 덮어쓰기를 하지 않는다.
- 새 Client/Server runtime, 별도 model loader, Effect V2 청산, ImGui 제품 UI를 만들지 않는다.
- Client/UI를 실행하거나 조작하지 않는다.
- 원본에 없는 animation을 이름 복제, bind pose, 잘못된 skeleton donor 또는 추측 retarget으로 만들지 않는다.
- unsupported material lane을 diffuse/emissive로 거짓 remap하지 않는다.
- `Client/Bin/Resources/**`의 이번 쿠크 payload는 Git add/commit하지 않고 tracked/staged 0을 유지한다.

## 4. 실행 계약

### 4.1 Animation

1. Action XML/LOA의 explicit clip name과 UModel이 export한 모든 PSA clip을 대소문자 비의존 exact name으로 비교한다.
2. 게임 package metadata에서 `AnimSequence`, `AnimSet`, import dependency를 확인한다.
3. donor 후보가 있더라도 bone name, count, order가 exact하지 않으면 cook하지 않는다.
4. exact source가 없으면 해당 이름은 추출 가능한 animation asset이 아니라 Action 상태 참조라는 evidence로 닫는다.
5. weapon package에 animation export가 0개면 0개가 canonical임을 기록하고, 가능한 mesh/material WModel만 기존 ActorX → FBX → ModelAssetConverter 경로로 cook한다.

### 4.2 Map material

1. final material catalog의 hydration gap과 physical texture dependency를 먼저 확인한다.
2. 37개 asset의 모든 material slot을 다음으로 분류한다.
   - authored null override
   - UModel empty helper material
   - named texture parameter가 없는 procedural/additive material
   - exact source texture는 있으나 CMaterial lane이 없는 source-only auxiliary role
3. exact resolved texture는 물리 closure와 receipt에 보존한다.
4. 물리 texture dependency closure와 runtime material semantic fidelity를 별도 상태로 기록한다.

### 4.3 Canonical map relocation

1. 기존 `Map/KakulSaydon/LV_LUT_MIDNIGHTC_ED` 파일 set의 상대 경로, 크기, SHA-256을 계산한다.
2. `Map/LV_LUT_MIDNIGHTC_ED`로만 이동하고 pre/post file set 차이가 0인지 확인한다.
3. exact-empty `Map/KakulSaydon` alias만 제거한다.
4. MapCatalog 292개 model path와 2,951개 placement asset ID를 실제 파일에 대조한다.

## 5. 검증

- 모든 변경 JSON parse
- `Tools/KakulSaydonPipeline/test_kakul_world_admission.py`
- `validate_kakul_world_admission.py --require development-geometry-preview`
- `Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Validate`
- 같은 publisher의 `-Mode Check`
- 기존 WModel geometry harness와 ModelAssetConverter focused tests
- WModel `info`, animation name uniqueness, skeleton identity 검사
- map install receipt path/size/SHA identity 검사
- Kakul 관련 Resources의 Git tracked/staged count 0
- `git diff --check`

## 6. 완료 판정

- 물리 map path와 MapCatalog root가 일치하고 catalog/placement missing reference가 0이다.
- 37개 항목은 실제 추출 누락 0과 source/runtime 표현 경계를 분리해 증명한다.
- `MN_RPCZ_00`과 weapon animation은 exact source 기준으로만 cook 또는 canonical-not-authored 판정을 갖는다.
- 구현 상태, 자동 검증, 사용자 수동 시각·청각 검증을 RESULT에서 분리한다.
- Product level admission, navigation/gameplay/server/client runtime 연결은 이번 extraction-only 마감의 완료 조건이 아니다.
