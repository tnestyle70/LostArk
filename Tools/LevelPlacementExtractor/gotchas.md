# Lost Ark 레벨·DeployData 추출 gotchas

이 파일은 추출기 옆에서 바로 확인하는 실행 체크리스트다. 전체 근거와 단계는
`C:/Users/user/Desktop/LostArk/.md/GB/07-30/맵추출파이프라인.md`가 정본이다.

## Static level

- ImportTable은 참조 증거이지 placement 증거가 아니다. Transform은 Actor/Component
  Export payload에서 읽는다.
- 일반 StaticMeshActor는 Actor Transform, StaticMeshCollectionActor는 Component
  Transform이 정본이다.
- `12,949`는 SL00~SL05, `13,091`은 PS까지 더한 수다.
- UE3 position cm은 Client에서 `(X,Z,-Y)*0.01`; signed `scale3D`는 보존한다.
- exact placement는 `Origin` anchor를 쓴다. `BottomCenter`는 수동 바닥 배치용이다.
- 동일 asset/Transform도 source actor/component, streaming level, visibility와 state를
  보기 전에는 중복 제거하지 않는다.

## DeployData gameplay Prop

- 파괴물은 PS/SL에 없을 수 있다. `DeployData.loa -> EFTable_Prop.db ->
  EFDLProp_*.loa -> TriggerMapData.loa`를 join한다.
- `StateOffActionId`는 `EFTable_GameAction.db -> EFTable_SkillEffect.db`까지 해석한다.
- 정적 `.mapplacements`와 Deploy placement를 합치지 않는다. 서로 다른 source layer다.
- 가까운 Deploy record를 중복 제거하지 않는다. 난이도/phase/restore 조건의 variant일
  수 있으므로 raw field와 raw record SHA-256을 보존한다.
- FracturedStaticMesh는 intact/fractured state lane, SkeletalMesh는 AnimModel lane,
  model 없는 actor는 trigger/volume lane이다.
- HeartRB 아레나 112개 중 시각 85개, fractured static 77개, skeletal 8개,
  TriggerMapData 안에서 deploy actor ID의 little-endian 4바이트 패턴이 발견된 레코드는
  111개다. 이것은 강한 binary occurrence 증거지만 특정 Trigger node·필드 참조를
  구조적으로 파싱한 결과는 아니다. 이 경우에도 임의 대체 asset을 배치하지 않는다.
- `ChangePropState`, `ChangePropProperty`, `Condition_HitProp`, `DestroyHitProp`,
  `SpawnProp`, `DespawnProp`, `Destruct/Restore*`의 정확한 자동 전환은 구조 파서가
  준비되기 전까지 확정하지 않는다. 현재 런타임의 상태 radio는 명시적인 debug selector다.

## Floor crack와 material

- 원본 crack MaterialInstance에는 exact diffuse/normal은 있지만 authored emissive가 없다.
  영상 근거 녹색 texture는 `VIDEO_MATCH_RECONSTRUCTION`으로 표시한다.
- diffuse에 녹색을 구워 넣지 않는다. 별도 WMat emissive slot과 G-buffer emissive target을
  사용한다.
- 약한 self-lit과 bloom halo는 다른 기능이다. intensity는 `CMapAssetObject` 전역 상수가
  아니라 Catalog v3의 asset별 render profile이 소유한다. crack 재구성 profile은 `0.35`를
  쓰지만 다른 authored emissive asset에 이 값을 전파하지 않는다. bloom은 post-process에서
  별도로 맞춘다.
- WMat v2를 다시 만들면 source pack의 `asset.manifest.json`과 `.complete.json`을 실제
  파일 hash로 동기화한다.

## Render profile과 phase layer

- `MRT_GameObject`에 emissive RT를 붙였다면 같은 MRT를 쓰는 모든 G-buffer writer가
  `SV_TARGET4`를 반드시 기록해야 한다. emissive가 없는 writer는 0을 써야 뒤쪽 발광이
  전경을 뚫고 남지 않는다.
- Catalog v3 render mode는 `Opaque`, `Alpha`, `Sky`, `Additive`다. `Sky`는 깊이를 읽되
  쓰지 않고, 안쪽 면을 보려면 `Front` cull을 선택한다. 반투명 구름은 `Alpha`와 UV
  panning, 효과 plane은 `Additive`를 쓴다.
- 쇠사슬 A/B, CloudPlane, sky mirror는 exact level placement와 exact texture다.
  spacehole/hugechaosgate plane은 exact particle texture를 쓰지만 plane topology와 수동
  전환 timing은 `VIDEO_MATCH_RECONSTRUCTION`이다.
- phase placement는 기본 저장 상태를 hidden으로 둔다. Baseline/SpaceHole/ChaosGate
  radio가 가시성만 전환하며 원작 TriggerMap/Matinee timing을 주장하지 않는다.

## WModel과 AnimModel

- 정적 mesh는 `ModelAssetConverter --pretransform --scale 100` 계약을 쓴다.
- skinned glTF에 `--pretransform`을 쓰면 skeleton/skin을 평탄화하거나 잃을 수 있다.
  `ITR_02326` 같은 AnimModel은 `--scale 100`만 사용하고 결과 WModel의 skeleton flag를
  확인한다.
- SkeletalMesh가 exact여도 glTF `animations`가 0이면 애니메이션 복구 완료가 아니다.
  `ITR_02326_Ani` AnimSet package가 별도로 확보되기 전에는 exact bind pose로만 표기한다.
- FracturedStaticMesh 77개는 LookInfo 직접 참조다. intact 77개는 같은 package의 sibling
  export에서 찾은 것이므로 `direct LookInfo reference`와 provenance를 분리한다.

## Build와 runtime smoke

- Debug/Release 산출물은 `Engine/Bin/<Configuration>`, `Client/Bin/<Configuration>`,
  `EngineSDK/lib/<Configuration>`으로 분리한다. `UpdateLib.bat`에는 Engine을 빌드한 것과
  같은 Configuration을 전달하며, Client도 반드시 같은 Configuration으로 빌드한다.
- AssetTest 자동 진입 순서는 초기 `Enter`로 Logo 진입 → `F2`로 asset loader → loader에서
  `Enter`다. F2 직후 바로 Enter만 보내면 잘못된 상태를 테스트할 수 있다.
- 프로세스 생존은 resource/prototype/G-buffer smoke PASS다. crack 밝기, 구름 흐름,
  sky의 화면 점유율과 phase 효과는 대상 카메라에서 별도 시각 QA해야 한다.

## Commit gate

1. parse
2. validate
3. stage
4. commit
5. 실패 시 새 layer만 rollback

빌드 성공과 화면 검증도 분리한다. 프로세스 생존은 resource/G-buffer smoke PASS일 뿐,
카메라가 대상에 있지 않다면 재질의 최종 밝기 PASS가 아니다.
