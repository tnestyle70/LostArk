# 카드미로 망원경·바닥 문양 수정계획서

작성일: 2026-09-14. 개정 R2. 요청 범위: 런타임 중앙 망원경 표시, 실제 카드병사 및 같은 경로를 쓰는 플레이어·출구 문양 표시.

## G00. 현재 파일과 작업 경계

분석 정본은 `C:/Users/USER/source/졸업팀폴/LostArk`이다. 이 문서는 **수정 제안**이며 실제 C++·JSON·리소스에는 반영하지 않았다. 현재 다른 작업의 변경이 있으므로 전문을 무조건 덮어쓰지 않는다. 부록의 기준 해시와 현재 diff를 확인하고 기능 브랜치에서 이번 변경만 반영한다.

R2는 원본 입력 확보 → 실제 소비자 연결 → 변경 범위 검증 → 증분 빌드 → 사용자 화면 확인까지 실패할 때의 조치도 고정한 인계 문서다. **문서만으로 실행 결과가 완벽하다고 보장하지 않는다.** ‘문양 완료’, ‘망원경 완료’, ‘사용자 화면 확인 대기’를 따로 기록한다. 망원경 입력이 미확보이면 전체 작업 완료로 보고하지 않는다.

### 구현 전 반드시 고정할 것

1. 현재 checkout, 브랜치, dirty diff, Client 실행 파일 경로·구성을 기록한다. 계획서의 코드 전문과 최신 파일이 다르면 이번 변경만 옮기고 다른 작업을 덮어쓰지 않는다. main에서 바로 구현하거나 사용자의 변경을 자동 stash/reset하지 않는다.
2. 작업 중인 Tool draft가 있으면 Save·종료를 사용자에게 요청한다. Reload/Publish/재시작으로 draft를 잃게 하지 않는다. 빌드나 설치기를 먼저 실행하지 않는다.
3. 아래 네 leaf/8 group의 실제 ID·DDS·생성 경로와 바닥 두 ID가 현재도 일치하는지 확인한다. 달라졌다면 기존 계획 숫자를 강제하지 말고 변경 근거를 확인해 해당 부분을 갱신한다.
4. 망원경 입력의 `LookInfo 확보 / mesh 확보 / material·texture 확보 / cooked 검증 / 초기 표시 pose 확인 / 게시 / 제품 표시` 상태를 각각 기록한다. 참조 문자열을 찾은 것과 모델 파일을 확보한 것을 섞지 않는다.
5. 전역 재질 복원·조명·암전은 제외한다. 신규 망원경이 기존 지원 경로에서 표시되기 위한 자체 texture/material 연결만 포함한다. 새 native family 구현까지 필요하면 현재 지원으로 완성했다고 우회하지 않고 그 경계를 보고한다.

### 이번 개정에서 바로잡은 사항

- Deploy는 SL03/미로 활성화 조건으로 생성되는 것이 아니라 Area 전체 로드다.
- ANIM의 빈 clip은 bind pose 보장이 아니라 모델 로더 기본 clip의 0초 pose일 수 있다.
- STATIC Deploy는 Animated Props 목록에 나오지 않는다. 종류에 맞춰 확인·편집 방법을 분리한다.
- runtime F1을 다시 여는 것만으로 새 asset prototype과 배치를 읽었다고 볼 수 없다.
- 새 영구 하네스와 광역 validator를 일반 구현·컴파일 선행조건으로 추가하지 않는다. 이번 입력과 호출 경로 검증에 집중한다.

이번 계획에서 바꾸지 않는 항목은 Server 역할 배정, Q 상호작용, 몬스터 스폰·피격·처치, 문양 정답, 네비게이션, 카메라, 전등·암전·재질 복구다. 망원경의 원작 대기/작동 애니메이션 복원도 이번 ‘오브젝트가 보이게 하기’와 별개다.

### 코드와 데이터에서 확인한 내용

| 대상 | 확인된 사실 | 이번 처리 |
|---|---|---|
| 중앙 망원경 | `Gameplay.world.json`의 `cardmaze.telescope`는 `claimCardMazeTelescope`를 보내는 triggerBox이며 모델을 생성하지 않는다. 09-08 인계 문서에도 시각 모델 미추가로 기록되어 있다. | 기존 Deploy 소품 경로에 원본 모델과 시각 배치만 추가한다. |
| 원본 중앙 소품 | Deploy actor `0x10000102`, Prop `378134`, 참조 `EFDLProp_ITR_10073.ITR_10073`. 원본 배치를 변환하면 약 `(0, 0, 1351.68)`, 회전 quaternion `(0,0,0,1)`, scale 1이다. | 참조를 따라 실물 에셋을 확보한다. 다른 망원경을 임의 대체하지 않는다. |
| 문양 파일 | `cardmaze.symbol.*` 4개와 `cardmaze.mark/exit.*` 8개가 있으며 DDS 네 개도 존재한다. | 기존 ID·텍스처·색·대상 선택·수명을 보존한다. |
| 현재 렌더 방식 | 네 leaf는 `Decal`. `CEffectV2Object::Render_Decal()`이 `Target_Depth`와 `Target_Normal`을 사용한다. | 이 네 leaf만 기존 Texture 평면으로 전환한다. |
| 미로 바닥 | placement `10296705976280178153`, 아래 G03의 asset ID. Alpha/forward 경로이며 해당 경로는 문양이 요구하는 G-buffer depth/normal을 쓰지 않는다. | 바닥 재질을 opaque로 바꾸지 않는다. 문양의 receiver 의존성을 제거한다. |
| 재시도 | `Sync_MazeMark()`는 같은 asset ID에서 handle이 0이 되어도 다시 생성하지 않는 구조다. | 카드미로 8개 그룹만 제한된 재시도를 추가한다. |

바닥 데이터와 셰이더 경로의 불일치는 강한 원인 근거다. GPU 캡처로 모든 누락 사례를 확정한 것은 아니다. 바닥 샘플 5지점에서 해당 Alpha 표면만 발견됐으며, 사용자 런타임 육안 확인을 최종 통과 기준으로 남긴다.

원본 증거:

- `C:/Users/USER/OneDrive/바탕 화면/쿠크1관문_연출_원본_20260913/00_원본파일/테이블/Prop_zone37081_배치소품.csv`: `PrimaryKey=378134`의 Model 참조.
- 분석 checkout의 `.codex_tmp/kakul_lever_probe/DeployData_37081.loa`: `CEFDeployActor_Prop` offset `0x02C907`. 위치 필드 `+0x14`, 회전 `+0x20`, actor ID `+0x44`, scale `+0x4C`, definition `+0x78`을 기존 추출기 형식으로 읽었다.
- `Tools/LevelPlacementExtractor/extract_deploydata_props.py`와 `build_maptool_scene.py`: 기존 좌표 변환 계약. 단, 앞 스크립트에는 zone 37051 전용 source ID가 있으므로 **그대로 실행해 쿠크 데이터 전체를 덮어쓰면 안 된다**.

## G01. Deploy catalog·placement — 망원경 시각 모델

### 입력과 선행 조건

정확한 참조 ID는 찾았지만 `ITR_10073` cooked 모델/LookInfo 실물은 조사한 리소스 폴더에서 확보하지 못했다. 따라서 **모델 확보 전에 catalog 행을 먼저 추가하거나 publish하지 않는다**. 이 단계가 막혀도 G02~G04 문양 수정은 독립적으로 진행할 수 있다.

1. 원본의 `EFDLProp_ITR_10073.ITR_10073` LookInfo를 해석하고, 그 안에 명시된 실제 StaticMesh/SkeletalMesh 및 재질·텍스처 참조만 추출한다. LookInfo 이름을 곧바로 mesh 이름이라고 가정하지 않는다.
2. 기존 ModelAssetConverter로 `CModel -> CMaterial` 호환 `.wmodel`을 만든다. 원본 scale·좌표 변환은 기존 Deploy 변환을 사용하며, scale 100을 중복 적용하지 않는다. 기존 Valtan 전용 `build_deployprop_runtime.py`를 쿠크 catalog 전체에 그대로 실행하지 않는다.
3. `ModelAssetConverter info`에서 model/material-version, skeleton 유무, mesh 수, texture 참조를 확인하고 CModel 로드 검증을 거친다. 모든 참조가 Resources 내부에 있어야 한다.
4. 원본 실물을 구할 수 없으면 정확한 누락 참조를 결과에 기록하고 망원경 단계만 중단한다. 투명 프록시나 엉뚱한 모델을 추가하고 ‘망원경 완료’로 보고하지 않는다.

### 소유자와 경로

- asset ID: **신규 제안** `DEPLOY_ITR_10073`.
- Resources 상대 경로: `Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/DEPLOY_ITR_10073/DEPLOY_ITR_10073.wmodel`.
- 물리 경로: `C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/DEPLOY_ITR_10073/`.
- 정의: `Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.deployassets`.
- 배치: `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.deployplacements`.
- 소비자: 기존 `CDeployPropCatalog` → 기존 Level Deploy 생성 → `CDeployPropObject` → `CModel/CMaterial`.

### 추가할 완전한 데이터 행

catalog v3의 현재 4개 행은 유지한다. 리소스 검증이 끝난 뒤 header의 개수만 4→5로 바꾸고 아래 두 행 중 **converter가 보고한 모델 종류에 대응하는 한 행만** 추가한다. 이 분기는 구현자 취향이 아니라 검증된 asset 형식으로 결정한다.

STATIC 모델:

```text
"DEPLOY_ITR_10073" STATIC "Card maze telescope ITR_10073" "Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/DEPLOY_ITR_10073/DEPLOY_ITR_10073.wmodel" "Prototype_Component_Model_DEPLOY_ITR_10073" "" "" 1 0 "SOURCE_REFERENCE Prop 378134 EFDLProp_ITR_10073.ITR_10073; initial visual pose only" "" ""
```

ANIM 모델:

```text
"DEPLOY_ITR_10073" ANIM "Card maze telescope ITR_10073" "Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/DEPLOY_ITR_10073/DEPLOY_ITR_10073.wmodel" "Prototype_Component_Model_DEPLOY_ITR_10073" "" "" 1 0 "SOURCE_REFERENCE Prop 378134 EFDLProp_ITR_10073.ITR_10073; initial visual pose only" "" ""
```

빈 clip 역할은 현재 parser와 `Apply_LogicalAnimation()`이 허용한다. 그러나 `Engine/Private/Model.cpp`의 `Ready_BinaryModel()`은 animation이 있으면 `defaultAnimationName` 또는 index 0을 선택한다. 따라서 빈 role은 **원작 idle 재생도, bind pose 보장도 아니다. 기본 클립의 0초 정지 pose일 수 있다.** Deploy INTACT 상태에서는 그 animation 시간이 진행되지 않는다.

위 ANIM 행은 기본 pose가 목표로 하는 펼쳐진 중앙 장치 모습과 맞을 때만 채택한다. 모델 info의 clip 목록과 기본 clip을 확인하고, 원본 LookInfo의 초기 상태 참조를 따라 해당 pose를 확인한다. 접힌/다른 형태라면 확인된 실제 clip의 0초 pose가 맞는 경우에만 intact role에 그 이름을 넣는다. 원하는 모양이 clip 중간에만 존재한다면 위 빈 role 행으로 완료 처리하지 않는다. 원본 초기 상태 pose를 해당 모델에 한정해 구워 연결하거나 현재 Deploy의 pose 선택 기능을 근거와 함께 보강한 뒤 문서를 갱신한다. 확인하지 않은 `idle/on/off` 이름, ANIM→STATIC 위장, 모든 소품의 clip0 강제 변경은 금지한다.

배치 v2는 현재 6개 행, ID `1,2,3,4,5,7`이다. header 개수 6→7과 함께 다음 행을 추가한다. 적용 시 ID 8이 이미 생겼다면 기존 행을 덮어쓰지 말고 프로젝트의 placement ID 발급 절차로 충돌 없는 ID를 발급한다.

```text
8 0 0 "cardmaze.telescope.visual" "DEPLOY_ITR_10073" 0 0 1351.68 0 0 0 1 1 0 0 0 PROJECT_AUTHORED
```

이 배치는 원본 위치를 참고한 **프로젝트 시각 배치**다. gameplay trigger ID `cardmaze.telescope`와 분리하고, source actor/definition 필드에는 0을 유지한다. 원본 provenance인 척하지 않는다. 원본 actor와 Prop ID는 위 증거에 보존한다. trigger 중심 `(0.28,-0.01,1351.65)`과의 기준점 거리는 약 0.28m이며 trigger 반경 안이다. **모델 전체 bounding box가 반경 안이라는 의미는 아니다.** 원본 mesh pivot·bounds를 변환해 바닥 접점과 Q 접근 공간을 확인하고, 상호작용 영역을 모델 중심으로 임의 이동하지 않는다.

### 생성·실패·저장

모델/텍스처 검증 → catalog/placement 한 쌍 stage → publisher Validate → Publish 순서다. 모델 누락 상태에서 runtime catalog만 선반영하면 Area 전체가 로드 실패할 수 있다. 생성 실패 시 신규 clone·레이어 등록을 정리하고 기존 맵을 보존한다. 현재 도구가 실패 시 빈 Area로 전환한다면 실패 보존까지 확인하기 전 완료 처리하지 않는다.

Area publisher는 같은 Area의 다른 저작 문서도 함께 게시할 수 있다. Publish 직전에 현재 미게시 mapplacements/worldsequences/materials/lights 변경을 확인하고, 이번 작업과 무관한 다른 작업자의 변경을 임의로 함께 게시하지 않는다. 충돌하면 해당 저장 상태와 게시 범위를 확인받고, 그동안 문양 데이터·C++ 검증처럼 독립 가능한 부분은 진행한다. 전체 Area 출력을 옛 계획의 복사본으로 교체하지 않는다.

실제 기존 경로는 다음과 같다.

```text
Loader::Ready_DeployPropArea
  → CDeployPropRuntime::Ensure_AreaPrototypes(runtime deployassets 전부)
  → CActorCatalog::Build_ModelLoadDescription → CModel::Create(preScale 0.01)
Level_KakulSaydonArena의 DeployCommit
  → m_DeployRuntime.Load_Area → Load(runtime deployplacements 전부)
  → Layer_DeployProps → CDeployPropObject
Area 종료
  → m_DeployRuntime.Clear
```

이 경로에는 MapLoadScope 인자가 없다. **Area의 Deploy 전체가 상주**하며 망원경도 Area 로드 때 생성된다. 미로 위치에서 일반 카메라/클리핑으로 보이는 것이지 SL03 활성화에 맞춰 별도 생성되는 것이 아니다. 이 작업에서 새 scope/전역 생성기/Server spawn을 추가하지 않는다. 현재 초기 숨김 ID는 다리 1·4, 보스 5, 책 7이며 신규 제안 ID 8은 포함되지 않는다. 정상 INTACT 상태로 Q 전부터 보이고, 재입장 시 1개여야 한다.

**확인·편집 UI는 모델 종류별로 다르다.**

- ANIM + PROJECT_AUTHORED: F1 → Map Tool → World Sequence → Animated Props (Deploy ANIM)의 기존 Transform 편집 → Apply Transform → Save Animated Props. 현재 패널의 ANIM 필터가 맞는지 확인한다.
- STATIC: 위 Animated Props 목록에 보이지 않는 것이 현재 정상이다. 이번 범위에서는 `.deployplacements`의 해당 시각 배치 행 수정 → Publish → 재진입으로 조정한다. World Destruction의 Deploy Props 목록에서 존재는 확인할 수 있으나 읽기 전용 위치 표시를 Transform 편집기라고 안내하지 않는다. UI에 노출하려고 STATIC을 ANIM으로 속이지 않는다.

런타임에서 F1은 Level 소유 Deploy에 attach한다. 창을 닫았다 여는 것만으로 신규 prototype/저작 배치를 다시 읽지 않는다. Reload Animated Props는 활성 preview 중 거부될 수 있고 신규 모델 prototype admission을 대체한다고 보장할 수 없으므로, **신규 모델 Publish 후 최종 확인은 Client 재실행**으로 고정한다. 기존 모델의 배치만 바꾸는 반복은 publish 후 정상 재입장/검증된 Reload 경로를 사용한다.

## G02. Effect leaf — 투영 Decal에서 바닥 평면으로

수정 파일은 `Data/Effects/V2/Authored/cardmaze.symbol.heart/spade/club/diamond.effectv2.json` 네 개다. G06에 네 파일의 제안 전문을 둔다.

| 필드 | 제안 값 | 이유 |
|---|---|---|
| effectType | Texture | G-buffer receiver를 요구하지 않는 기존 SPRITE 경로 |
| rotation start/end | `[90,-90,0]`, lerp false | XY→XZ 변환 후 기존 Decal의 yaw와 UV 축을 유지. 아래 수치 검사 후 실제 시인성은 사용자 확인 |
| scale start/end | `[2,2,1]`, lerp false | 기존 decal 2×2m와 같은 기본 크기 |
| billboard | false 유지 | 카메라를 따라 서지 않음 |
| depthTest | true 유지 | opaque 물체 뒤에서는 기존 depth test를 유지 |
| softFadeDistance | 0 유지 | 다시 scene-depth receiver에 의존하지 않음 |
| color/texture/alpha/loop/lifetime | 현재 값 유지 | 색·문양·수명 변경을 섞지 않음 |

기존 `decal` 설정 블록은 schema 호환을 위해 남겨도 Texture 렌더에 사용되지 않는다. 새 HLSL, 별도 V1 경로, ImGui 화면 문양은 추가하지 않는다.

회전·크기 근거: `VIBuffer_Rect`의 정점은 x/y ±0.5, UV는 `(x+0.5, 0.5-y)`이다. scale `(2,2,1)` 후 X축 +90° 회전은 로컬 `(2x,0,2y)`를 만들고 yaw -90°를 이어 적용한다. 기존 Decal은 같은 yaw를 역변환한 local 좌표에서 `(local.x/2+0.5, 0.5-local.z/2)`를 샘플하므로 네 모서리와 중앙의 UV가 일치한다. `XMMatrixRotationRollPitchYaw` 및 row-vector `Scale * Rotation * Pivot` 순서를 기준으로 검사한다. 소스 Effect의 yaw -90°를 pitch 값으로 잘못 옮기지 않는다.

G02는 ‘원작이 반드시 Texture 평면이었다’는 복원 주장이 아니다. 현재 평평한 미로 바닥에서 기존 문양을 표시하기 위한 프로젝트 수정이다. 다른 Decal을 일괄 Texture로 바꾸지 않는다.

유지할 계약:

- `cardmaze.mark.*`와 `cardmaze.exit.*` 8개 group ID 및 자식 변환은 그대로 유지한다.
- `groundPivot()`의 translation-only와 `world Y + 0.02m`를 보존한다. 병사 크기/yaw가 문양을 확대·회전시키지 않는다.
- `Play_Group`의 productOwned, duration 0 및 종료 정리를 유지한다. group 문서의 1000ms만 보고 1초짜리로 바꾸지 않는다.
- `Collect_KoukuMazeTargets()`가 고르는 실제 4종 `MONSTER_KOUKU_CARD_*`가 대상이다. 별도 행진 장식 36개/세토에 문양을 무조건 붙이는 작업이 아니다.
- HUNTER, 탈출·전이·사망·미로 종료 필터를 유지한다. 망원경 담당에게 무조건 문양을 주도록 역할 규칙을 바꾸지 않는다.

높이는 실재하는 대상 root를 확인한다. 병사/플레이어/출구 각각의 pivot Y가 해당 위치 바닥과 근접하는지 확인하고, snapshot root가 지면과 크게 다르면 ‘재시도’나 크기 확대 문제가 아니다. 현재 바닥 높이 약 -0.01m와 유효한 root에 +0.02m를 적용하는 기본안을 검증하되, 다른 컷신/빙고/미로 외 맵의 Y를 전역 보정하지 않는다.

## G03. Renderer·MapAssetObject — 미로 바닥만 먼저 그리기

Texture 전환만으로 끝내지 않는다. 기존 Alpha 정렬은 객체 원점의 카메라 거리 기준이라 큰 바닥이 문양보다 나중에 그려질 수 있다.

**문양을 모든 Alpha 뒤에 그리는 방안은 사용하지 않는다.** 그러면 투명 카드 벽 위에 문양이 비칠 수 있다. 이번 제안은 확인된 미로 바닥만 priority -1, 나머지 전부 0이다.

### 파일·선언·함수 책임

| 파일 | 변경 위치와 책임 |
|---|---|
| `Engine/Public/BlendSortKey.h` 신규 | `BLEND_SORT_KEY`는 우선순위와 정규화된 거리만 소유. `BlendSortBefore()`는 priority 오름차순, 동일 priority 거리 내림차순. 게임 ID를 모른다. |
| `Engine/Public/GameObject.h` | `Render_Group` 다음에 `Get_BlendSortPriority() const` 기본 0 추가. 모든 기존 객체의 동작을 보존한다. |
| `Engine/Private/Renderer.cpp` | `Render_Blend()`만 교체. queue → 우선순위/거리 수집 → stable sort → Render_Group → 실패 기록·queue 정리. 카메라 없음도 같은 우선순위 정렬. |
| `Client/Public/CardMazeVisualPolicy.h` 신규 | `IsCardMazeFloorReceiver()`는 아래 두 stable ID가 모두 일치할 때만 true. Client가 게임별 예외를 소유한다. |
| `Client/Public/MapAssetObject.h` | Render_Group 선언 다음에 우선순위 override 선언. |
| `Client/Private/MapAssetObject.cpp` | Render_Group 정의 직전에 override 정의. 해당 바닥 -1, 그 외 0. |

정확한 대상:

```text
placementId = 10296705976280178153
assetId = MAP_3C514C107BAB_LV_OCN_FORGOTTENIS_PLANE01_SM_OVR_017DC7A6977C
```

이 Alpha 바닥은 현재 `MapPlacementRuntime` batch 대상에서 제외되므로 placement 객체의 override가 실제 사용된다. 다른 배치/복제된 바닥에 자동 확대 적용하지 않는다. 미래에 이 ID가 달라지면 아래 실제 입력 검사에서 불일치를 보고하고 근거를 갱신한다. 정책 단독 검사만으로 실제 맵 데이터의 ID까지 검증하는 것은 아니다.

거리 없는 객체/비정상 거리는 기존처럼 FLT_MAX, 카메라 없음은 모두 동률로 취급한다. stable sort로 일반 객체의 같은 거리 제출 순서를 보존한다. scratch는 render thread 소유이고 non-owning 포인터는 그 프레임 queue 수명 안에서만 쓴다. 첫 렌더 오류를 보존하고 queue와 scratch를 정리한다. 시간복잡도 O(N log N), 기존 정렬과 동일하다.

재질·opacity·depth write·HLSL은 수정하지 않는다. **투명 벽의 완벽한 픽셀 단위 가림을 새로 보장하는 변경은 아니다.** 기존 object-origin 거리 정렬의 한계는 남는다. 실제 벽 근처 가림 테스트에서 결함이 남으면 별도 renderer 작업으로 보고하고, depthTest를 끄거나 모든 문양을 최상단에 그려 통과시키지 않는다.

## G04. PresentationPlayer — 같은 문양의 실패 후 복구

`Client/Public/CardMazeVisualPolicy.h`의 `CARD_MAZE_MARK_RETRY`는 generation, nextAttemptMs, attempts를 소유한다. 모두 Client presentation/update thread 전용이다. 시간은 `steady_clock` 밀리초이고 서버 게임 시간이나 컷신 재생 시간과 분리한다. generation 변경/새 occurrence는 예산을 초기화하며, 정상 handle 발급만으로 시도 횟수를 초기화하지 않는다.

`KoukuSaydonPresentationPlayer.h`의 기존 `CARD`에 이 상태를 추가한다. 기존 asset ID와 handle의 의미는 바꾸지 않는다. `Sync_MazeMark()` 호출자는 그대로 유지한다.

함수 흐름:

1. 8개 cardmaze mark/exit ID만 새 경로로 보낸다. Bingo 및 다른 ID는 현재 함수 본문을 그대로 사용한다.
2. asset 변경이면 old handle 정리 후 새 상태를 시작한다.
3. Effect cache generation을 관찰한다. 살아 있는 효과를 단순 generation 변경 때문에 먼저 없애지 않는다.
4. 살아 있는 handle은 pivot만 갱신. terminal failure가 있으면 이유를 기록하고 정리한 뒤 1초 뒤 재시도한다.
5. 각 mark occurrence와 cache generation마다 최초 1회 + 재시도 최대 2회, 시도 간 최소 1초. 같은 asset을 쓰는 병사들도 각자 예산을 가진다. 실패가 계속되면 예산 소진 상태를 유지한다.
6. 재시도할 때 `GROUP:<정확한 asset ID>`의 성공/실패 캐시만 제거한다. 모든 효과 캐시를 매 프레임 비우지 않는다.
7. 기존 `Ensure_EffectResource` → snapshot Find_Group → Play_Group 순서를 사용한다. load 실패 이유와 runtime Last_Error를 상태 메시지에 남긴다.
8. 예산 소진 후에는 다음 미로 진입/asset 변경 또는 정식 Effect cache generation 갱신으로 다시 시도한다. 살아 있는 다른 문양은 유지한다.

이 코드는 실패 문자열을 보존하지만 단일 공용 상태줄의 모든 덮어쓰기 문제까지 해결하지는 않는다. CModel/texture 로드 오류 로그와 함께 확인한다.

재시도는 Load/Spawn 및 `Consume_GroupFailure`가 전달하는 terminal failure에 대한 복구다. 성공 handle이 존재한다는 사실은 GPU 표시 성공이 아니다. Render_Group 실패나 Alpha=0·가림은 재시도 횟수를 늘려 해결하지 말고 기존 `RendererExit.user.log`/Effect Last_Error와 실제 입력을 조사한다. `Acquire_Texture`는 실패한 DDS를 성공 cache에 저장하지 않으므로, 파일 복구 후 다음 허용 시도에서 다시 로드할 수 있다. 이미 살아 있는 효과를 강제 파괴하는 자동 hot reload는 이 계획에 포함하지 않는다.

## G05 준비. 프로젝트 등록

신규 H 2개는 아래처럼 등록한다. 기존 폴더/필터를 재배치하지 않는다. 새 하네스 프로젝트·영구 validator·테스트 전용 CPP는 제품 변경에 추가하지 않는다. 이번 계획 작성의 단독 정책 검사는 out 아래에만 격리한다.

`Engine/Default/Engine.vcxproj`: 기존 `..\Public\Engine_RenderTypes.h` ClInclude 바로 다음:

```xml
<ClInclude Include="..\Public\BlendSortKey.h" />
```

`Engine/Default/Engine.vcxproj.filters`: 같은 header 항목의 닫는 ClInclude 다음:

```xml
<ClInclude Include="..\Public\BlendSortKey.h">
  <Filter>99.Defines</Filter>
</ClInclude>
```

`Client/Default/Client.vcxproj`: 기존 `..\Public\KoukuSaydonPresentationPlayer.h` ClInclude 바로 다음:

```xml
<ClInclude Include="..\Public\CardMazeVisualPolicy.h" />
```

`Client/Default/Client.vcxproj.filters`: 같은 header 항목의 닫는 ClInclude 다음:

```xml
<ClInclude Include="..\Public\CardMazeVisualPolicy.h">
  <Filter>04. Network</Filter>
</ClInclude>
```

기존 JSON 네 파일은 이미 등록되어 있어 추가하지 않는다. 신규 shader/일반 Client CPP 파일은 없다. 추가 H는 각각 실제 Renderer 및 MapAssetObject/PresentationPlayer에서 소비하며 테스트만을 위한 인터페이스가 아니다.

## 검증·적용 순서와 완료 조건

### 구현자가 수행할 자동 검증

실제 실행 checkout에서 수행한다. 망원경 리소스가 확보되지 않았으면 망원경 catalog/placement는 아직 적용하지 않고 문양 변경만 검증한다.

```powershell
Set-Location -LiteralPath 'C:/Users/USER/source/졸업팀폴/LostArk'
git diff --check
```

이번 네 leaf/8 group만 기존 resolver로 확인한다. 검증 코드를 제품 코드에 새로 추가하는 명령이 아니다. 적용 후 실제 파일을 읽는다.

```powershell
$mazePython = 'C:/Users/USER/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
@'
from pathlib import Path
import sys
root = Path.cwd()
sys.path.insert(0, str(root / 'Tools/EffectToolV2'))
import effect_v2_binding_pipeline as pipeline
suits = ('heart', 'spade', 'club', 'diamond')
authored = {f'cardmaze.symbol.{s}': root / f'Data/Effects/V2/Authored/cardmaze.symbol.{s}.effectv2.json' for s in suits}
groups = {f'cardmaze.{kind}.{s}': root / f'Data/Effects/V2/Groups/cardmaze.{kind}.{s}.effectv2group.json' for kind in ('mark', 'exit') for s in suits}
groups = {key: (path, pipeline.read_json(path)) for key, path in groups.items()}
for effect_id, path in authored.items():
    document = pipeline.read_json(path)
    assert document['effectId'] == effect_id
    assert document['effectType'] == 'Texture'
    p = document['params']
    assert p['rotation']['start'] == [90, -90, 0] and not p['rotation']['lerp']
    assert p['scale']['start'] == [2, 2, 1] and not p['scale']['lerp']
    assert p['depthTest'] and not p['billboard'] and p['softFadeDistance'] == 0
    assert p['lifetime'] == 0 and p['loop']
for group_id in groups:
    leaves, span = pipeline._resolve_group(group_id, authored, groups, require_v2=True, resource_root=root / 'Client/Bin/Resources')
    assert len(leaves) == 1 and span == 1000, group_id
print('Card maze inputs: 4 leaves / 8 groups validated; GPU visibility not tested')
'@ | & $mazePython -B -
if ($LASTEXITCODE -ne 0) { throw 'Card maze input validation failed' }
```

G01의 catalog/placement를 변경한 경우에만 모델 입력을 갖춘 뒤 같은 Area의 publisher로 구조를 검증한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Validate
if ($LASTEXITCODE -ne 0) { throw 'Deploy/map authoring validation failed' }
```

실제 바닥 ID 검사도 별도로 수행한다. 다음은 저작/런타임 양쪽에서 정확히 한 행이며 해당 asset ID를 참조하는지 확인한다. 이 검사 실패를 정책 단독 검사 성공으로 덮지 않는다.

```powershell
$mazeFloorId = '10296705976280178153'
$mazeFloorAsset = 'MAP_3C514C107BAB_LV_OCN_FORGOTTENIS_PLANE01_SM_OVR_017DC7A6977C'
$mazePlacementFiles = @(
    'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapplacements',
    'Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.mapplacements'
)
foreach ($mazePlacementFile in $mazePlacementFiles) {
    $mazeFloorRows = @(Get-Content -LiteralPath $mazePlacementFile | Where-Object { $_ -match ('^' + $mazeFloorId + ' ') })
    if ($mazeFloorRows.Count -ne 1 -or -not $mazeFloorRows[0].Contains('"' + $mazeFloorAsset + '"')) {
        throw "Card maze floor identity changed: $mazePlacementFile"
    }
}
```

전체 Effect validator나 광역 하네스는 일반 컴파일의 선행조건으로 붙이지 않는다. 명시적으로 확장 진단을 수행했다면 기존의 무관한 실패와 이번 기능 실패를 분리한다. 변경 project/filter XML parse, DDS 존재, 회전·크기, 그룹 참조 및 카드미로 ID 외 변경 없음도 확인한다.

실제 수정 경로에서 확인할 정책은 우선순위/거리 정렬, 동률 안정성, 카메라 없는 입력, 정확한 두 ID 가드, 8개 그룹 필터, 1초 제한, 3회 예산, generation 복구다. 수치·단독 정책 검사 성공을 GPU draw/실제 리소스 생성/Level rollback 성공으로 확대 해석하지 않는다. 새 하네스 프로젝트를 추가하거나 별도 하네스 실행 여부로 일반 빌드·커밋을 막지 않는다.

Engine public header가 바뀌므로 Engine과 Client를 일치시켜야 한다. Product 증분 빌드 경로를 사용한다. **Clean/Rebuild, 모든 shader 강제 재컴파일, out 전체 삭제를 하지 않는다.** 신규 shader 변경은 없지만 공용 header 영향으로 C++ 재컴파일이 발생할 수 있어 ‘몇 분에 끝난다’고 보장하지 않는다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

실행 중 Client가 링크 산출물을 점유하면 사용자가 Save·종료한 후 빌드한다. 빌드 도구의 Engine/SDK 연결을 따르고 별도 수동 복사 경로를 중복 실행하지 않는다. Release 배포 시 같은 소스의 Release 회귀도 수행한다.

망원경 모델이 검증되고 G01 데이터가 적용된 경우에만:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Check
```

`Client/Bin/DataFiles/Map` 생성물을 손으로 편집하지 않는다. Effect V2는 ProjectDataRoot의 `Data/Effects/V2`를 읽으므로 임의의 runtime Effect 복사본을 따로 만들지 않는다.

### 사용자가 확인할 런타임 절차

Client 작업 디렉터리는 `Client/Default`. 같은 빌드의 평소 VS 실행 경로로 로비 → 쿠크 → 카드미로에 들어간다. 이 검증은 **MapTool의 CardMiro_Play만 누르는 것이 아니라 실제 런타임 미로 진입·중앙 Q 시작** 기준이다.

| 확인 상황 | 기대 결과 |
|---|---|
| 미로 입장, Q 전 | 중앙 모델 1개가 보이며 기존 Q 상호작용 위치가 유지됨 |
| 실제 미로 시작 | 하트·스페이드·클럽·다이아 병사에 맞는 바닥 문양이 보임 |
| 병사 이동/회전 | 발밑에서 따라가고 병사 yaw/크기에 끌려 돌거나 커지지 않음 |
| 플레이어/망원경 담당 | 기존 HUNTER 역할 조건대로만 표시. 담당자 규칙을 바꾸지 않음 |
| 1초 이상/미로 진행 중 | 문양이 유지되고 새로 나타날 때마다 중복 생성되지 않음 |
| opaque 물체·카드 벽 근처 | 바닥 덮어쓰기가 없고 벽 가림에 새 회귀가 없는지 확인 |
| 처치·전이·탈출·미로 종료 | 해당 문양이 사라지고 출구 문양 조건도 기존대로 동작 |
| 나가기·재입장 | 망원경과 문양이 중복 없이 다시 생성됨 |
| F1 MapTool 편집 | 기존 Area와 6개 Deploy 배치가 유지되며 신규 모델도 찾을 수 있음 |

마지막 행의 목록/편집 위치는 G01의 ANIM·STATIC 구분을 따른다. 기존 6개라는 숫자는 계획 기준값이며 다른 작업자가 정상 추가한 배치를 줄이라는 지시가 아니다.

### 기대와 다를 때의 조사 순서

| 증상 | 먼저 확인할 실제 경계 | 금지하는 우회 |
|---|---|---|
| 망원경 catalog 자체가 안 열림 | resource path·model kind·모든 texture·catalog/placement 한 쌍, publish 상태 | 모델이 없는데 존재한다고 등록, fallback 회색을 원작 완료로 보고 |
| 모델이 있지만 Q 전 보이지 않음 | Loader admission, Layer_DeployProps clone, 초기 INTACT, 기본 clip0 pose, 위치·bounds | Server 역할/맵 전체 가시성 강제 변경 |
| STATIC이 Animated Props에 없음 | 해당 목록의 ANIM 필터. STATIC 배치는 파일+publish로 조정 | 모델 종류를 ANIM으로 위장 |
| 모든 문양이 없음 | 실제 런타임 미로 활성/대상 snapshot → 대상 수집 → group handle/실패 → layer → Render_Blend | 장식 36개에 전부 생성, gameplay 활성 조건 삭제 |
| 특정 문양만 없음 | 해당 suit ID·leaf·DDS·mask R/alpha·실패 상태 | 전체 색/alpha/shader 일괄 변경 |
| handle 정상인데 안 보임 | 실제 pivot, 최종 scale/rotation, Alpha 입력, floor 우선순위, opaque DSV, draw 실패 | 재시도 횟수 무한 증가, depthTest false |
| 벽 위로 문양이 비침 | marker priority가 0인지, 바닥만 -1인지, 기존 투명벽 정렬 | 모든 marker 최후 렌더 강제 |
| 재시작해도 이전 값 | EXE 구성·경로, ProjectDataRoot/ResourceRoot, publish 경로, 새 prototype 로드 | 설치기 재실행으로 로컬 수정본 덮어쓰기 |

미검증 경계를 없애기 위해 소스를 넓게 바꾸는 것이 아니라, 위 순서로 실패 단계를 좁힌 뒤 이번 대상의 최소 수정으로 해결한다. 새 family/전역 occlusion 알고리즘 등 범위 확장이 필요하면 필요한 근거와 변경 범위를 먼저 보고한다.

실패 검증은 별도 임시 테스트 데이터에서 수행하고 공유 실물 리소스를 삭제하지 않는다: 없는 DDS/잘못된 group ID/손상 문서/망원경 모델 누락/중복 placement ID/루트 밖 경로를 거부해야 한다. 실패한 문양은 1초 제한과 총 3회 예산을 지키고, 다른 정상 문양과 맵은 유지해야 한다. 모델 누락 상태의 publish는 실패하며 이전 runtime 파일이 보존되어야 한다. 중간 promote 실패를 실험할 필요가 있다면 실행 폴더가 아닌 격리 복사본에서만 기존 publisher failure-injection을 사용한다.

### R2 작성 중 실제 수행한 검증

- 제안 leaf 네 개에서 effectType/rotation/scale 외 필드가 현재 원본과 동일한지 비교했다.
- 제안 leaf 복사본과 현재 group 8개를 기존 `effect_v2_binding_pipeline._resolve_group`에 넣어 실제 DDS 참조와 1000ms group 기본 길이를 검증했다. 8개 모두 통과했다.
- 저작/런타임 mapplacements 양쪽에서 해당 바닥의 두 stable ID 일치를 확인했다.
- 2×2m 평면의 네 모서리+중앙에서 기존 Decal과 새 Sprite UV를 비교했다. 최대 오차는 약 `5.56e-17`, plane Y 오차는 부동소수점 허용 범위였다.
- 제안 `BlendSortKey.h`와 `CardMazeVisualPolicy.h` 전문을 out에 격리해 MSVC x64/C++20으로 컴파일·실행했다. 우선순위·안정 정렬·카메라 없음·정확한 ID·8그룹·재시도 예산·generation·overflow의 단독 정책 검사 결과 `failures=0`이었다.
- 위 단독 검사는 제품 Renderer/PresentationPlayer 전체 파일의 빌드가 아니다. 새 Client 링크·실제 GPU 표시·망원경 모델 확보·사용자 화면 확인은 아직 하지 않았다.

검증 산출물은 계획 작성 worktree의 `out/CardMazeDiagnosis/r2_check/`에만 있다. 이것을 제품 프로젝트 등록이나 다른 PC 배포 요건으로 만들지 않는다.

### 인계와 상태 표기

- 이번 문서: 현재 데이터/호출 경로 조사와 코드 제안. 구현·게임 빌드·최종 화면 확인은 아직 수행하지 않았다.
- 독립 비평 반영: marker 최상단 렌더 방안 폐기, 바닥 두 ID 제한, 카메라-null 정렬 유지, Bingo 제외, 빈 clip은 초기 pose라는 경계, 모델 검증 선행.
- 구현 결과는 `.md/GB/09-14/2026-09-14_KOUKU_CARD_MAZE_VISUAL_FIX_RESULT.md`에 실제 수행한 자동/수동 결과를 분리해 기록한다. 망원경 입력 미확보와 문양 수정 완료를 한 상태로 합치지 않는다.
- 새 generic Engine BLEND 우선순위 public 계약과 배치 입력 추가는 실제 반영 후 해당 팀 안내 문서에 필요한 부분만 기록한다.
- 리소스 인계는 `DEPLOY_ITR_10073` 폴더와 필요한 텍스처 상대 경로를 전달한다. 확인하지 않은 원본 일치·원작 idle·육안 PASS를 기록하지 않는다.

다음 G05/G06 전문은 구현 시 대조하는 코드 정본이다. 현재 다른 작업 변경이 계속되는 저장소이므로 부록 해시가 달라졌다면 최신 파일에 이번 변경 구간만 재적용한다.

## G05. 기존 H/CPP 적용 후 전문

아래는 실제 실행 checkout의 현재 파일을 보존한 **제안 코드**다. 아직 소스에 반영하거나 컴파일하지 않았다. 최신 작업과 비교하지 않고 파일 전체를 덮어쓰지 않는다. 해시가 달라졌으면 변경 구간만 재조정한다.

### Engine/Public/GameObject.h

대상: `C:/Users/USER/source/졸업팀폴/LostArk/Engine/Public/GameObject.h`

```cpp
#pragma once

#include "GameInstance.h"

NS_BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CPrototype
{
public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC
	{

	}GAMEOBJECT_DESC;
protected:
	CGameObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CGameObject();

public:
	shared_ptr<CComponent> Get_Component(const wstring_t& strComponentTag);

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Post_Physics_Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_Group(RENDERGROUP group);
	// Lower values draw first within BLEND; equal values retain distance order.
	virtual int32_t Get_BlendSortPriority() const { return 0; }
	virtual HRESULT Render_DeferredOverlay();
	virtual HRESULT Render_Shadow();

protected:
	map<const wstring_t, shared_ptr<CComponent>>		m_Components;

protected:
	shared_ptr<class CTransform>			m_pTransformCom = { nullptr };


	template<typename T>
	HRESULT Add_Component(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, const wstring_t& strComponentTag, shared_ptr<T>& pOut, void* pArg = nullptr)
	{
		if (nullptr != Find_Component(strComponentTag))
			return E_FAIL;

		shared_ptr<CComponent> pComponent = dynamic_pointer_cast<CComponent>(CGameInstance::Get().Clone_Prototype(iPrototypeLevelIndex, strPrototypeTag, pArg));
		if (nullptr == pComponent)
			return E_FAIL;

		m_Components.emplace(strComponentTag, pComponent);

		pOut = dynamic_pointer_cast<T>(pComponent);

		return S_OK;
	}
protected:
	CComponent* Find_Component(const wstring_t& strComponentTag);

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
	void Free();

};

NS_END
```

### Engine/Private/Renderer.cpp

대상: `C:/Users/USER/source/졸업팀폴/LostArk/Engine/Private/Renderer.cpp`

```cpp
#include "Renderer.h"
#include "BlendSortKey.h"
#include <algorithm>
#include <cfloat>
#pragma push_macro("new")
#undef new
#include "DirectXTK/DDSTextureLoader.h"
#pragma pop_macro("new")
#include "Engine_RenderTypes.h"
#include "Engine_VertexTypes.h"
#include "Material.h"
#include "Render_OutputContract.h"
#include "Profiler.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <typeinfo>

namespace
{
	void WriteRendererFailure(
		const char* stage,
		const HRESULT result,
		const char* objectType = nullptr)
	{
#ifdef _DEBUG
		std::ofstream output(
			"RendererExit.user.log",
			std::ios::binary | std::ios::app);
		if (!output)
			return;

		output << "stage=" << (nullptr == stage ? "unknown" : stage)
			<< " hr=0x" << std::hex << std::uppercase
			<< static_cast<unsigned long>(result) << std::dec;
		if (nullptr != objectType)
			output << " object=" << objectType;
		output << '\n';
#else
		UNREFERENCED_PARAMETER(stage);
		UNREFERENCED_PARAMETER(result);
		UNREFERENCED_PARAMETER(objectType);
#endif
	}
}
#include "GameInstance.h"
#include "Presentation_Manager.h"
#include "Transform.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

namespace
{
	constexpr uint32_t DEFERRED_PASS_SCENE_RESOLVE =
		ETOUI(DEFERRED::SCENE_RESOLVE);
	constexpr uint32_t DEFERRED_PASS_RGB_NOISE =
		ETOUI(DEFERRED::PRESENTATION_RGB_NOISE);
	constexpr uint32_t DEFERRED_PASS_ZOOM_BLUR =
		ETOUI(DEFERRED::PRESENTATION_ZOOM_BLUR);
	constexpr uint32_t DEFERRED_PASS_FILM_NOISE =
		ETOUI(DEFERRED::PRESENTATION_FILM_NOISE);
	constexpr uint32_t DEFERRED_PASS_TEXTURED_OVERLAY =
		PRESENTATION_TEXTURED_OVERLAY_PASS_INDEX;
	constexpr uint32_t DEFERRED_PASS_CHROMATIC_ABERRATION =
		ETOUI(DEFERRED::PRESENTATION_CHROMATIC_ABERRATION);

	static_assert(8u == DEFERRED_PASS_SCENE_RESOLVE);
	static_assert(9u == DEFERRED_PASS_RGB_NOISE);
	static_assert(10u == DEFERRED_PASS_ZOOM_BLUR);
	static_assert(11u == DEFERRED_PASS_FILM_NOISE);
	static_assert(14u == DEFERRED_PASS_TEXTURED_OVERLAY);
	static_assert(12u == ETOUI(DEFERRED::SSAO_RAW));
	static_assert(13u == ETOUI(DEFERRED::SSAO_BLUR));
	static_assert(15u == DEFERRED_PASS_CHROMATIC_ABERRATION);
	static_assert(16u == ETOUI(DEFERRED::SPOT));
	static_assert(17u == ETOUI(DEFERRED::PRESENTATION_DISPLAY_OVERLAY));
	static_assert(18u == ETOUI(DEFERRED::SOURCE_LIGHT_MASK));
	static_assert(19u == ETOUI(DEFERRED::SOURCE_DIRECTIONAL));
	static_assert(20u == ETOUI(DEFERRED::SOURCE_POINT));
	static_assert(21u == ETOUI(DEFERRED::SOURCE_SPOT));
	static_assert(22u == ETOUI(DEFERRED::END));

	bool_t IsFiniteInRange(const f32_t fValue, const f32_t fMinimum,
		const f32_t fMaximum)
	{
		return std::isfinite(fValue) &&
			fValue >= fMinimum && fValue <= fMaximum;
	}

	bool_t IsValidRenderQualitySettings(
		const RENDER_QUALITY_SETTINGS& Settings)
	{
		return
			IsFiniteInRange(Settings.fSSAORadius, 0.01f, 8.f) &&
			IsFiniteInRange(Settings.fSSAOBias, 0.f, 1.f) &&
			IsFiniteInRange(Settings.fSSAOIntensity, 0.f, 4.f) &&
			IsFiniteInRange(Settings.fSSAOPower, 0.1f, 8.f) &&
			IsFiniteInRange(Settings.fSSAODistanceFade, 1.f, 1000.f) &&
			Settings.fSSAOBias < Settings.fSSAORadius &&
			Settings.fSSAODistanceFade >= Settings.fSSAORadius &&
			IsFiniteInRange(Settings.fBloomThreshold, 0.f, 64.f) &&
			IsFiniteInRange(Settings.fBloomSoftKnee, 0.f, 1.f) &&
			IsFiniteInRange(Settings.fBloomIntensity, 0.f, 16.f) &&
			IsFiniteInRange(Settings.fBloomScatter, 0.25f, 4.f) &&
			IsFiniteInRange(Settings.fExposure, 0.01f, 32.f) &&
			IsFiniteInRange(Settings.fWhitePoint, 1.f, 64.f) &&
			IsFiniteInRange(Settings.fGamma, 1.f, 3.f) &&
			IsFiniteInRange(Settings.fFXAASubpixel, 0.f, 1.f) &&
			IsFiniteInRange(Settings.fFXAAEdgeThreshold, 0.0312f, 0.333f) &&
			IsFiniteInRange(Settings.fFXAAEdgeThresholdMin, 0.0156f, 0.0833f);
	}
}

CRenderer::CRenderer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{

}

CRenderer::~CRenderer()
{
}

HRESULT CRenderer::Initialize()
{
	float2_t		vViewportSize = CGameInstance::Get().Get_ViewportSize();
	m_vShadowTexelSize = float2_t(
		1.f / static_cast<f32_t>(m_iShadowMapSize),
		1.f / static_cast<f32_t>(m_iShadowMapSize));
	m_iBloomWidth = max(1u, static_cast<uint32_t>(vViewportSize.x) / 2u);
	m_iBloomHeight = max(1u, static_cast<uint32_t>(vViewportSize.y) / 2u);
	m_vBloomTexelSize = float2_t(
		1.f / static_cast<f32_t>(m_iBloomWidth),
		1.f / static_cast<f32_t>(m_iBloomHeight));
	m_iSSAOWidth = max(1u, static_cast<uint32_t>(vViewportSize.x) / 2u);
	m_iSSAOHeight = max(1u, static_cast<uint32_t>(vViewportSize.y) / 2u);
	m_vSSAOTexelSize = float2_t(
		1.f / static_cast<f32_t>(m_iSSAOWidth),
		1.f / static_cast<f32_t>(m_iSSAOHeight));

	/* For.Target_Diffuse */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Diffuse"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R8G8B8A8_UNORM, float4_t(1.f, 1.f, 1.f, 0.f))))
		return E_FAIL;

	/* For.Target_Normal */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Normal"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_UNORM, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* For.Target_Shade */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Shade"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Depth */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Depth"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(1.f, 1.f, 1.f, 0.f))))
		return E_FAIL;

	/* For.Target_Specular */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Specular"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Material input RGB is separate from accumulated light specular. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_MaterialSpecular"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

    // Source character direct lighting needs UV and its full tangent basis.
    if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_CharacterSurface"), vViewportSize.x, vViewportSize.y,
        DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))) ||
        FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_CharacterGeometry"), vViewportSize.x, vViewportSize.y,
        DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;

	/* For.Target_PickPos */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_PickPos"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Emissive */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Emissive"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_SceneHDR */
	/* Scene colour before tone mapping. FP16 so values above 1 survive. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SceneHDR"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	// One shared full-resolution contribution target for all effects, never one per skill.
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SceneBloom"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Refractive effects read this snapshot while SceneHDR remains the output. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_EffectSceneColor"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_EffectSceneBloom"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Signed RG offsets written by distortion-capable effect shaders. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Distortion"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Half-resolution ambient occlusion: raw estimate plus bilateral resolve. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SSAORaw"), m_iSSAOWidth, m_iSSAOHeight,
		DXGI_FORMAT_R16_FLOAT, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SSAOBlur"), m_iSSAOWidth, m_iSSAOHeight,
		DXGI_FORMAT_R16_FLOAT, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* Half-resolution bloom chain. R11G11B10 preserves positive HDR energy. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_BloomExtract"), m_iBloomWidth, m_iBloomHeight,
		DXGI_FORMAT_R11G11B10_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_BloomPing"), m_iBloomWidth, m_iBloomHeight,
		DXGI_FORMAT_R11G11B10_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_BloomResult"), m_iBloomWidth, m_iBloomHeight,
		DXGI_FORMAT_R11G11B10_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(Ready_Shadow_Resources()))
		return E_FAIL;
	if (FAILED(Ready_Bloom_DSV()))
		return E_FAIL;
	if (FAILED(Ready_SSAO_DSV()))
		return E_FAIL;
	if (FAILED(Ready_ScenePostTargets(
		static_cast<uint32_t>(vViewportSize.x),
		static_cast<uint32_t>(vViewportSize.y))))
	{
		return E_FAIL;
	}



	/* MRT_GameObject */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Diffuse"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Normal"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Depth"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_PickPos"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Emissive"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_MaterialSpecular"))))
		return E_FAIL;
    if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_CharacterSurface"))))
        return E_FAIL;
    if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_CharacterGeometry"))))
        return E_FAIL;

	/* MRT_LightAcc */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Shade"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Specular"))))
		return E_FAIL;

	/* MRT_SceneHDR */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SceneHDR"), TEXT("Target_SceneHDR"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SceneHDR"), TEXT("Target_Distortion"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SceneHDR"), TEXT("Target_SceneBloom"))))
		return E_FAIL;

	/* Half-resolution bloom ping-pong targets. */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_BloomExtract"), TEXT("Target_BloomExtract"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_BloomPing"), TEXT("Target_BloomPing"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_BloomResult"), TEXT("Target_BloomResult"))))
		return E_FAIL;

	/* Half-resolution SSAO raw and bilateral targets. */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SSAORaw"), TEXT("Target_SSAORaw"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SSAOBlur"), TEXT("Target_SSAOBlur"))))
		return E_FAIL;

	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Deferred.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;
	
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(vViewportSize.x, vViewportSize.y, 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix,
		XMMatrixOrthographicLH(vViewportSize.x, vViewportSize.y, 0.f, 1.f));

#ifdef _DEBUG
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Diffuse"), 150.f, 150.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Normal"), 150.f, 450.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Shade"), 450.f, 150.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Specular"), 450.f, 450.f, 300.f, 300.f)))
	//	return E_FAIL;
#endif

	return S_OK;
}

HRESULT CRenderer::Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<CGameObject> pRenderObject)
{
	if (nullptr == pRenderObject ||
		eRenderGroupID >= RENDERGROUP::END)
		return E_FAIL;

	m_RenderObjects[ETOUI(eRenderGroupID)].push_back(std::move(pRenderObject));
    if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
    {
        switch (eRenderGroupID)
        {
        case RENDERGROUP::PRIORITY: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsPriority); break;
        case RENDERGROUP::SHADOW: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsShadow); break;
        case RENDERGROUP::NONBLEND: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsNonBlend); break;
        case RENDERGROUP::BLEND: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsBlend); break;
        default: break; // Other queues are not included in these four counters.
        }
    }

	return S_OK;
}

HRESULT CRenderer::Apply_MaterialRenderSettings(const MATERIAL_RENDER_SETTINGS& settings)
{
	if (static_cast<uint32_t>(settings.eDebugView) >= static_cast<uint32_t>(MATERIAL_DEBUG_VIEW::END))
		return E_INVALIDARG;
	m_MaterialRenderSettings = settings;
	return S_OK;
}

HRESULT CRenderer::Apply_RenderQualitySettings(
	const RENDER_QUALITY_SETTINGS& Settings)
{
	if (!IsValidRenderQualitySettings(Settings))
		return E_INVALIDARG;

	m_RenderQualitySettings = Settings;
	return S_OK;
}

namespace
{
	bool_t IsValidHeightFogSettings(const HEIGHT_FOG_SETTINGS& Settings)
	{
		const auto finite = [](const f32_t value) { return std::isfinite(value); };
		return finite(Settings.vColor.x) && finite(Settings.vColor.y) &&
			finite(Settings.vColor.z) && finite(Settings.vColor.w) &&
			Settings.vColor.x >= 0.f && Settings.vColor.y >= 0.f &&
			Settings.vColor.z >= 0.f &&
			finite(Settings.fDensity) && Settings.fDensity >= 0.f &&
			finite(Settings.fHeightFalloff) && Settings.fHeightFalloff > 0.f &&
			finite(Settings.fTopHeight) &&
			finite(Settings.fStartDistance) && Settings.fStartDistance >= 0.f &&
			finite(Settings.fMaximumOpacity) &&
			Settings.fMaximumOpacity >= 0.f && Settings.fMaximumOpacity <= 1.f &&
			finite(Settings.fDriftSpeed) && Settings.fDriftSpeed >= 0.f &&
			finite(Settings.fDriftHeightAmplitude) &&
			Settings.fDriftHeightAmplitude >= 0.f &&
			finite(Settings.fDriftDensityAmplitude) &&
			Settings.fDriftDensityAmplitude >= 0.f &&
			finite(Settings.fCoveragePercent) &&
			Settings.fCoveragePercent >= 0.f &&
			Settings.fCoveragePercent <= 1.f &&
			finite(Settings.fWindDirectionX) &&
			finite(Settings.fWindDirectionZ) &&
			finite(Settings.fWindSpeed) && Settings.fWindSpeed >= 0.f &&
			finite(Settings.fPatchScale) && Settings.fPatchScale > 0.f &&
			finite(Settings.fPatchSoftness) &&
			Settings.fPatchSoftness > 0.f &&
			Settings.fPatchSoftness <= 0.5f &&
            finite(Settings.vInscatteringColor.x) && Settings.vInscatteringColor.x >= 0.f &&
            finite(Settings.vInscatteringColor.y) && Settings.vInscatteringColor.y >= 0.f &&
            finite(Settings.vInscatteringColor.z) && Settings.vInscatteringColor.z >= 0.f &&
            finite(Settings.vFogLightDirection.x) && finite(Settings.vFogLightDirection.y) &&
            finite(Settings.vFogLightDirection.z) && finite(Settings.vFogLightDirection.w) &&
            Settings.vFogLightDirection.w >= -1.f && Settings.vFogLightDirection.w <= 1.f &&
            (!Settings.bSourceExponential ||
                Settings.vFogLightDirection.x * Settings.vFogLightDirection.x +
                Settings.vFogLightDirection.y * Settings.vFogLightDirection.y +
                Settings.vFogLightDirection.z * Settings.vFogLightDirection.z > 0.000001f);
	}
}

HRESULT CRenderer::Stage_RenderEnvironment(const wstring_t& cubePath,
    const float4_t& color, const float4_t& rotationIntensity,
    RENDER_ENVIRONMENT_STATE& outState, bool_t forceReload) const
{
    const float values[] = {color.x, color.y, color.z, color.w,
        rotationIntensity.x, rotationIntensity.y, rotationIntensity.z, rotationIntensity.w};
    for (float value : values) if (!std::isfinite(value)) return E_INVALIDARG;
    if (color.x < 0.f || color.y < 0.f || color.z < 0.f || color.w < 0.f ||
        color.x > 64.f || color.y > 64.f || color.z > 64.f || color.w > 64.f ||
        rotationIntensity.z < 0.f || rotationIntensity.z > 64.f ||
        rotationIntensity.w != 0.f ||
        std::abs(rotationIntensity.x * rotationIntensity.x +
            rotationIntensity.y * rotationIntensity.y - 1.f) > .001f) return E_INVALIDARG;
    RENDER_ENVIRONMENT_STATE staged;
    staged.strCubePath = cubePath;
    staged.vColor = color;
    staged.vRotationIntensity = rotationIntensity;
    if (!cubePath.empty() && !forceReload && cubePath == m_RenderEnvironment.strCubePath &&
        m_RenderEnvironment.pCube)
    {
        // Quality edits and mood changes in the same scene retain its staged cube.
        staged.pCube = m_RenderEnvironment.pCube;
    }
    else if (!cubePath.empty())
    {
        if (FAILED(CreateDDSTextureFromFileEx(m_pDevice.Get(), cubePath.c_str(), 0,
            D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
            DDS_LOADER_DEFAULT, nullptr, staged.pCube.GetAddressOf()))) return E_FAIL;
        D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
        staged.pCube->GetDesc(&desc);
        if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBE ||
            desc.TextureCube.MipLevels == 0u ||
            (desc.Format != DXGI_FORMAT_BC1_UNORM && desc.Format != DXGI_FORMAT_BC2_UNORM &&
             desc.Format != DXGI_FORMAT_BC3_UNORM && desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM &&
             desc.Format != DXGI_FORMAT_B8G8R8A8_UNORM))
            return E_INVALIDARG;
    }
    outState = std::move(staged);
    return S_OK;
}

void CRenderer::Commit_RenderEnvironment(const RENDER_ENVIRONMENT_STATE& state)
{
    m_RenderEnvironment = state;
}

RENDER_ENVIRONMENT_STATE CRenderer::Get_RenderEnvironment() const
{
    return m_RenderEnvironment;
}

HRESULT CRenderer::Bind_HeightFog(CShader* shader) const
{
    if (!shader) return E_INVALIDARG;
	const uint32_t iFogEnabled = m_HeightFogSettings.bEnabled ? 1u : 0u;
	const float2_t vFogWind(
		m_HeightFogSettings.fWindDirectionX,
		m_HeightFogSettings.fWindDirectionZ);
    const uint32_t sourceModel = m_HeightFogSettings.bSourceExponential ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_iSourceExponentialFog", &sourceModel, sizeof(sourceModel))) ||
        FAILED(shader->Bind_RawValue("g_vFogInscatteringColor", &m_HeightFogSettings.vInscatteringColor, sizeof(float4_t))) ||
        FAILED(shader->Bind_RawValue("g_vFogLightDirection", &m_HeightFogSettings.vFogLightDirection, sizeof(float4_t))) ||
		FAILED(shader->Bind_RawValue("g_iHeightFogEnabled",
			&iFogEnabled, sizeof(iFogEnabled))) ||
		FAILED(shader->Bind_RawValue("g_vHeightFogColor",
			&m_HeightFogSettings.vColor, sizeof(float4_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDensity",
			&m_HeightFogSettings.fDensity, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogFalloff",
			&m_HeightFogSettings.fHeightFalloff, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogTopHeight",
			&m_HeightFogSettings.fTopHeight, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogStartDistance",
			&m_HeightFogSettings.fStartDistance, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogMaximumOpacity",
			&m_HeightFogSettings.fMaximumOpacity, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDriftSpeed",
			&m_HeightFogSettings.fDriftSpeed, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDriftHeight",
			&m_HeightFogSettings.fDriftHeightAmplitude, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDriftDensity",
			&m_HeightFogSettings.fDriftDensityAmplitude, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fFogCoverage",
			&m_HeightFogSettings.fCoveragePercent, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_vFogWindDirection",
			&vFogWind, sizeof(vFogWind))) ||
		FAILED(shader->Bind_RawValue("g_fFogWindSpeed",
			&m_HeightFogSettings.fWindSpeed, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fFogPatchScale",
			&m_HeightFogSettings.fPatchScale, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fFogPatchSoftness",
			&m_HeightFogSettings.fPatchSoftness, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fPresentationClock",
			&m_fPresentationClock, sizeof(m_fPresentationClock))))
	{
		return E_FAIL;
	}
    return S_OK;
}

HRESULT CRenderer::Apply_HeightFog(const HEIGHT_FOG_SETTINGS& Settings)
{
	if (!IsValidHeightFogSettings(Settings))
		return E_INVALIDARG;

	m_HeightFogSettings = Settings;
	return S_OK;
}

void CRenderer::Advance_PresentationClock(f32_t fTimeDelta)
{
	if (!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
		return;

	/* Wrapping keeps the drift phase exact after long sessions instead of
	   letting float precision quantise the sine input. */
	constexpr f32_t CLOCK_WRAP_SECONDS = 3600.f;
	m_fPresentationClock = fmodf(
		m_fPresentationClock + fTimeDelta, CLOCK_WRAP_SECONDS);
}

HRESULT CRenderer::Draw()
{
    // Priority sky and later forward water share this frame clock.
    CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	auto FailFrame = [this, &Presentation](
		const char* stage, const HRESULT hResult) -> HRESULT
	{
		WriteRendererFailure(stage, hResult);
        CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);
		m_bSceneColorSnapshotRequested = false;
        m_bSceneEnvironmentReplaced = false;
		Presentation.Clear_Frame();
		for (auto& RenderGroup : m_RenderObjects)
			RenderGroup.clear();
		return FAILED(hResult) ? hResult : E_FAIL;
	};
	CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
	CProfilerScope drawScope(pProfiler, "Render.Draw");
	CProfilerGpuScope gpuDrawScope(pProfiler, "Render.Draw");
	HRESULT hResult = S_OK;
	{
		CProfilerScope scope(pProfiler, "Render.SubmitFrameProviders");
		hResult = Presentation.Submit_FrameProviders();
	}
	if (FAILED(hResult))
		return FailFrame("Submit_FrameProviders", hResult);
	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	if (vViewportSize.x <= 0.f || vViewportSize.y <= 0.f)
	{
		return FailFrame("Viewport", E_INVALIDARG);
	}
	hResult = Ready_ScenePostTargets(
		static_cast<uint32_t>(vViewportSize.x),
		static_cast<uint32_t>(vViewportSize.y));
	if (FAILED(hResult))
		return FailFrame("Ready_ScenePostTargets", hResult);
	{
		CProfilerScope scope(pProfiler, "Render.Shadow");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Shadow", true);
		hResult = Render_Shadow();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Shadow", hResult);
	{
		CProfilerScope scope(pProfiler, "Render.NonBlend");
		CProfilerGpuScope gpuScope(pProfiler, "Render.NonBlend", true);
		hResult = Render_NonBlend();
	}
	if (FAILED(hResult))
		return FailFrame("Render_NonBlend", hResult);
	if (m_RenderQualitySettings.bSSAOEnabled)
	{
		CProfilerScope scope(pProfiler, "Render.SSAO");
		CProfilerGpuScope gpuScope(pProfiler, "Render.SSAO", true);
		hResult = Render_SSAO();
		if (FAILED(hResult))
			return FailFrame("Render_SSAO", hResult);
	}
	{
		CProfilerScope scope(pProfiler, "Render.Lights");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Lights", true);
		hResult = Render_Lights();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Lights", hResult);

	/* Scene colour is accumulated in FP16 so lighting and effect values above 1 */
	/* survive until tone mapping. Sky/background join the same target, or the   */
	/* final blit would overwrite them with black.                               */
	hResult = CGameInstance::Get().Begin_MRT(TEXT("MRT_SceneHDR"));
	if (FAILED(hResult))
		return FailFrame("Begin_MRT_SceneHDR", hResult);
	HRESULT hSceneResult = S_OK;
	HRESULT hEndSceneResult = S_OK;
	{
		CRenderOutputContractScope SceneOutputScope(
			RENDER_OUTPUT_CONTRACT::
			SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION,
			m_pContext.Get());
		CProfilerScope scope(pProfiler, "Render.SceneHDR");
		CProfilerGpuScope gpuScope(pProfiler, "Render.SceneHDR");
		hSceneResult = Render_Priority();
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_Combined();
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_NonLight();
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_SceneReplacements();
		if (SUCCEEDED(hSceneResult) && m_bSceneColorSnapshotRequested)
		{
			hSceneResult = Capture_SceneColorSnapshot();
		}
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_Blend();

		/* Always restore the back-buffer/DSV pair after entering the HDR MRT. */
		hEndSceneResult = CGameInstance::Get().End_MRT();
	}
	if (FAILED(hSceneResult) || FAILED(hEndSceneResult))
		return FailFrame(
			FAILED(hSceneResult) ? "Render_Scene" : "End_MRT_SceneHDR",
			FAILED(hSceneResult) ? hSceneResult : hEndSceneResult);

	{
		CProfilerScope scope(pProfiler, "Render.ScreenPosts");
		CProfilerGpuScope gpuScope(pProfiler, "Render.ScreenPosts");
		hResult = Render_ScreenPosts();
	}
	if (FAILED(hResult))
		return FailFrame("Render_ScreenPosts", hResult);

	if (m_RenderQualitySettings.bBloomEnabled)
	{
		CProfilerScope scope(pProfiler, "Render.Bloom");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Bloom");
		hResult = Render_Bloom();
		if (FAILED(hResult))
			return FailFrame("Render_Bloom", hResult);
	}

	/* The one and only place tone mapping and gamma are applied. */
	{
		CProfilerScope scope(pProfiler, "Render.Final");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Final");
		hResult = Render_Final();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Final", hResult);

	{
		CProfilerScope scope(pProfiler, "Render.DisplayOverlays");
		CProfilerGpuScope gpuScope(pProfiler, "Render.DisplayOverlays");
		hResult = Render_DisplayOverlays();
	}
	if (FAILED(hResult))
		return FailFrame("Render_DisplayOverlays", hResult);

	/* UI is authored in display space, so it stays out of the HDR target. */
	{
		CProfilerScope scope(pProfiler, "Render.UI");
		CProfilerGpuScope gpuScope(pProfiler, "Render.UI", true);
		hResult = Render_UI();
	}
	if (FAILED(hResult))
		return FailFrame("Render_UI", hResult);

#ifdef _DEBUG
	{
		CProfilerScope scope(pProfiler, "Render.Debug");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Debug");
		hResult = Render_Debug();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Debug", hResult);
#endif

	m_bSceneColorSnapshotRequested = false;
    m_bSceneEnvironmentReplaced = false;
	Presentation.Clear_Frame();
	return S_OK;
}

#ifdef _DEBUG

HRESULT CRenderer::Add_DebugComponent(shared_ptr<CComponent> pDebugComponent)
{
	m_DebugComponent.push_back(pDebugComponent);
	return S_OK;
}

#endif

HRESULT CRenderer::Render_Priority()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.Priority");
    CProfilerGpuScope gpuScope(profiler, "Render.Priority");
	// Index iteration permits callbacks to append without retaining vector
	// iterators or element references; the queue still owns every object.
	for (size_t renderIndex = 0; renderIndex < m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)].size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)][renderIndex].get();
		if (nullptr != pRenderObject)
			pRenderObject->Render_Group(RENDERGROUP::PRIORITY);
	}

	m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_Shadow()
{
	auto& ShadowObjects =
		m_RenderObjects[ETOUI(RENDERGROUP::SHADOW)];
	if (nullptr == m_pShadowDSV || nullptr == m_pShadowSRV ||
		0u == m_iShadowMapSize)
	{
		ShadowObjects.clear();
		return E_FAIL;
	}

	D3D11_VIEWPORT OriginalViewports[
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	UINT iViewportCount = _countof(OriginalViewports);
	m_pContext->RSGetViewports(&iViewportCount, OriginalViewports);

	if (FAILED(CGameInstance::Get().Begin_DepthOnly(m_pShadowDSV)))
	{
		ShadowObjects.clear();
		return E_FAIL;
	}

	HRESULT hRenderResult = S_OK;
	if (CGameInstance::Get().Is_ShadowLightEnabled())
	{
		SetUp_ViewportDesc(m_iShadowMapSize, m_iShadowMapSize);
		for (size_t renderIndex = 0; renderIndex < ShadowObjects.size(); ++renderIndex)
		{
			CGameObject* const pRenderObject = ShadowObjects[renderIndex].get();
			if (nullptr != pRenderObject &&
				FAILED(pRenderObject->Render_Shadow()))
			{
				WriteRendererFailure(
					"Render_Shadow_Object",
					E_FAIL,
					typeid(*pRenderObject).name());
				hRenderResult = E_FAIL;
				break;
			}
		}
	}
	ShadowObjects.clear();

	const HRESULT hEndResult =
		CGameInstance::Get().End_DepthOnly();
	if (0u < iViewportCount)
		m_pContext->RSSetViewports(iViewportCount, OriginalViewports);
	else
		m_pContext->RSSetViewports(0u, nullptr);

	return FAILED(hRenderResult) || FAILED(hEndResult) ? E_FAIL : S_OK;
}

HRESULT CRenderer::Render_NonBlend()
{
    CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);
	/* Diffuse + Normal */
	if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_GameObject"))))
		return E_FAIL;

	auto& NonBlendObjects =
		m_RenderObjects[ETOUI(RENDERGROUP::NONBLEND)];
	auto& DeferredOverlayObjects =
		m_RenderObjects[ETOUI(RENDERGROUP::DEFERRED_OVERLAY)];
	for (size_t renderIndex = 0; renderIndex < NonBlendObjects.size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = NonBlendObjects[renderIndex].get();
		if (nullptr != pRenderObject)
			pRenderObject->Render_Group(RENDERGROUP::NONBLEND);
	}

	/* Deferred overlays must run after every opaque object while the complete
	   game-object MRT, including Target_Emissive, is still bound. */
	for (size_t renderIndex = 0; renderIndex < DeferredOverlayObjects.size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = DeferredOverlayObjects[renderIndex].get();
		if (nullptr != pRenderObject)
			pRenderObject->Render_DeferredOverlay();
	}

	NonBlendObjects.clear();
	DeferredOverlayObjects.clear();

	if (FAILED(CGameInstance::Get().End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_SSAO()
{
	if (nullptr == m_pSSAODSV || 0u == m_iSSAOWidth || 0u == m_iSSAOHeight)
		return E_FAIL;

	D3D11_VIEWPORT OriginalViewports[
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	UINT iViewportCount = _countof(OriginalViewports);
	m_pContext->RSGetViewports(&iViewportCount, OriginalViewports);
	SetUp_ViewportDesc(m_iSSAOWidth, m_iSSAOHeight);

	HRESULT hResult = Render_SSAOPass(
		TEXT("MRT_SSAORaw"), DEFERRED::SSAO_RAW);
	if (SUCCEEDED(hResult))
	{
		hResult = Render_SSAOPass(
			TEXT("MRT_SSAOBlur"), DEFERRED::SSAO_BLUR);
	}

	if (0u < iViewportCount)
		m_pContext->RSSetViewports(iViewportCount, OriginalViewports);
	else
		m_pContext->RSSetViewports(0u, nullptr);

	return hResult;
}

HRESULT CRenderer::Render_SSAOPass(
	const wstring_t& strMRTTag, const DEFERRED ePass)
{
	if (DEFERRED::SSAO_RAW != ePass && DEFERRED::SSAO_BLUR != ePass)
		return E_INVALIDARG;
	if (FAILED(CGameInstance::Get().Begin_MRT(strMRTTag, m_pSSAODSV)))
		return E_FAIL;

	HRESULT hResult = S_OK;
	HRESULT hBindAO = S_OK;
	if (DEFERRED::SSAO_BLUR == ePass)
	{
		hBindAO = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_SSAORaw"), m_pShader, "g_SSAOTexture");
	}

	if (FAILED(hBindAO) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Normal"), m_pShader, "g_NormalTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(
            TEXT("Target_CharacterGeometry"), m_pShader, "g_CharacterGeometryTexture")) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vSSAOTexelSize", &m_vSSAOTexelSize,
			sizeof(m_vSSAOTexelSize))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAORadius", &m_RenderQualitySettings.fSSAORadius,
			sizeof(m_RenderQualitySettings.fSSAORadius))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAOBias", &m_RenderQualitySettings.fSSAOBias,
			sizeof(m_RenderQualitySettings.fSSAOBias))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAOIntensity", &m_RenderQualitySettings.fSSAOIntensity,
			sizeof(m_RenderQualitySettings.fSSAOIntensity))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAOPower", &m_RenderQualitySettings.fSSAOPower,
			sizeof(m_RenderQualitySettings.fSSAOPower))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAODistanceFade", &m_RenderQualitySettings.fSSAODistanceFade,
			sizeof(m_RenderQualitySettings.fSSAODistanceFade))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_CameraViewMatrix",
			CGameInstance::Get().Get_Transform(D3DTS::VIEW))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_CameraProjMatrix",
			CGameInstance::Get().Get_Transform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_ProjMatrixInverse",
			CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Begin(ETOUI(ePass))) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		hResult = E_FAIL;
	}

	/* Balance Begin_MRT even when a shader bind or draw fails. */
	if (FAILED(CGameInstance::Get().End_MRT()))
		hResult = E_FAIL;

	return hResult;
}

HRESULT CRenderer::Render_Lights()
{
	/* Shade */
	if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_LightAcc"))))
		return E_FAIL;

	HRESULT hResult = S_OK;
    const uint32_t noSourceCharacter = 0u;
	const bool_t bShadowEnabled =
		CGameInstance::Get().Is_ShadowLightEnabled();
	const uint32_t iSSAOEnabled =
		m_RenderQualitySettings.bSSAOEnabled ? 1u : 0u;
	HRESULT hBindAO = S_OK;
	if (0u != iSSAOEnabled)
	{
		hBindAO = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_SSAOBlur"), m_pShader, "g_SSAOTexture");
	}
    if (FAILED(m_pShader->Bind_RawValue("g_SourceCharacterProgram", &noSourceCharacter, sizeof(noSourceCharacter))) ||
        FAILED(m_pShader->Bind_RawValue("g_SourceCharacterRow", &noSourceCharacter, sizeof(noSourceCharacter))) ||
        FAILED(m_pShader->Bind_Matrix("g_SourceCharacterViewMatrix", CGameInstance::Get().Get_Transform(D3DTS::VIEW))) ||
        FAILED(m_pShader->Bind_Matrix("g_SourceCharacterProjMatrix", CGameInstance::Get().Get_Transform(D3DTS::PROJ))) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Emissive"), m_pShader, "g_EmissiveTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterSurface"), m_pShader, "g_CharacterSurfaceTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterGeometry"), m_pShader, "g_CharacterGeometryTexture")) ||
        FAILED(hBindAO) ||
		FAILED(m_pShader->Bind_Texture(
			"g_LightDepthTexture", m_pShadowSRV)) ||
		FAILED(CGameInstance::Get().
			Bind_ShadowLight_LightingResources(m_pShader)) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vShadowTexelSize", &m_vShadowTexelSize,
			sizeof(m_vShadowTexelSize))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iSSAOEnabled", &iSSAOEnabled, sizeof(iSSAOEnabled))) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_MaterialSpecular"), m_pShader, "g_MaterialSpecularTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_PickPos"), m_pShader, "g_GeometricNormalTexture")) ||
		FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInverse", CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInverse", CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_RawValue("g_vCamPosition", CGameInstance::Get().Get_CamPosition(), sizeof(float4_t))) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(CGameInstance::Get().Render_Lights(
			m_pShader, m_pVIBuffer, bShadowEnabled)))
	{
		hResult = E_FAIL;
	}

    // The ordinary pass skips marker-5 pixels. Only materials submitted by
    // visible mesh draws get a source light pass; prototypes never enter here.
    const uint32_t sourceCount = CMaterial::Get_SourceCharacterFrameCount();
    if (SUCCEEDED(hResult) && sourceCount != 0u)
    {
        // Effect outlines own the scene stencil. Only this separate DSV receives
        // the marker-5 mask, and the original light targets are restored on failure.
        ID3D11RenderTargetView* rawTargets[2]{};
        ComPtr<ID3D11DepthStencilView> originalDepth;
        m_pContext->OMGetRenderTargets(2u, rawTargets, originalDepth.GetAddressOf());
        ComPtr<ID3D11RenderTargetView> targets[2];
        for (uint32_t index = 0u; index < 2u; ++index) targets[index].Attach(rawTargets[index]);
        D3D11_TEXTURE2D_DESC targetDesc{};
        if (targets[0])
        {
            ComPtr<ID3D11Resource> resource;
            ComPtr<ID3D11Texture2D> texture;
            targets[0]->GetResource(resource.GetAddressOf());
            if (resource && SUCCEEDED(resource.As(&texture))) texture->GetDesc(&targetDesc);
        }
        bool_t useSourceMask = false;
        if (targetDesc.SampleDesc.Count == 1u &&
            S_OK == Ready_SourceLightMask(targetDesc.Width, targetDesc.Height))
        {
            m_pContext->ClearDepthStencilView(m_pSourceLightMaskDSV.Get(), D3D11_CLEAR_STENCIL, 1.f, 0u);
            m_pContext->OMSetRenderTargets(0u, nullptr, m_pSourceLightMaskDSV.Get());
            HRESULT maskResult = m_pShader->Begin(ETOUI(DEFERRED::SOURCE_LIGHT_MASK));
            if (SUCCEEDED(maskResult)) maskResult = m_pVIBuffer->Render();
            useSourceMask = SUCCEEDED(maskResult);
            if (!useSourceMask)
            {
                m_iSourceLightMaskFailedWidth = targetDesc.Width;
                m_iSourceLightMaskFailedHeight = targetDesc.Height;
                WriteRendererFailure("SourceLightMask_DrawFallback", maskResult);
            }
            m_pContext->OMSetRenderTargets(2u, rawTargets,
                useSourceMask ? m_pSourceLightMaskDSV.Get() : originalDepth.Get());
        }
        for (uint32_t index = 0u; SUCCEEDED(hResult) && index < sourceCount; ++index)
        {
            if (FAILED(CMaterial::Bind_SourceCharacterLight(m_pShader, index)) ||
                FAILED(CGameInstance::Get().Render_Lights(m_pShader, m_pVIBuffer, bShadowEnabled,
                    LIGHT_RECEIVER::SOURCE_CHARACTER, useSourceMask))) hResult = E_FAIL;
        }
        m_pContext->OMSetRenderTargets(2u, rawTargets, originalDepth.Get());
    }
    if (FAILED(m_pShader->Bind_RawValue("g_SourceCharacterProgram", &noSourceCharacter, sizeof(noSourceCharacter))) ||
        FAILED(m_pShader->Bind_RawValue("g_SourceCharacterRow", &noSourceCharacter, sizeof(noSourceCharacter))))
        hResult = E_FAIL;
    CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);

	/* Always restore the back buffer even when a light bind or draw fails. */
	if (FAILED(CGameInstance::Get().End_MRT()))
		hResult = E_FAIL;

	return hResult;
}

HRESULT CRenderer::Render_Combined()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.Combined");
    CProfilerGpuScope gpuScope(profiler, "Render.Combined");
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Specular"), m_pShader, "g_SpecularTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Emissive"), m_pShader, "g_EmissiveTexture")))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	/* Height fog reuses the world position the combine step can rebuild from
	   the depth target, so it needs the same inverse matrices and camera the
	   lighting pass already binds. */
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInverse",
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInverse",
			CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_RawValue("g_vCamPosition",
			CGameInstance::Get().Get_CamPosition(), sizeof(float4_t))) ||
		FAILED(Bind_HeightFog(m_pShader.get()))) return E_FAIL;

	if (FAILED(m_pShader->Begin(ETOUI(DEFERRED::COMBINED))))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_NonLight()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.NonLight");
    CProfilerGpuScope gpuScope(profiler, "Render.NonLight");
	HRESULT hFirstFailure = S_OK;
	for (size_t renderIndex = 0; renderIndex < m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)].size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)][renderIndex].get();
		if (nullptr != pRenderObject)
		{
			const HRESULT hResult = pRenderObject->Render_Group(RENDERGROUP::NONLIGHT);
			if (FAILED(hResult) && SUCCEEDED(hFirstFailure))
			{
				WriteRendererFailure(
					"Render_NonLight_Object",
					hResult,
					typeid(*pRenderObject).name());
				hFirstFailure = hResult;
			}
		}
	}

	m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)].clear();

	return hFirstFailure;
}

HRESULT CRenderer::Refresh_SceneColorSnapshot()
{
	if (CRenderOutputContract::Get_Active() !=
			RENDER_OUTPUT_CONTRACT::SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION ||
		!CRenderOutputContract::Matches_ActiveRenderTargets(m_pContext.Get()))
		return E_INVALIDARG;
	return Capture_SceneColorSnapshot();
}

HRESULT CRenderer::Capture_SceneColorSnapshot()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope snapshotScope(profiler, "Render.SceneColorSnapshot");
	const auto sourceSRV = CGameInstance::Get().Get_RT_SRV(TEXT("Target_SceneHDR"));
	const auto snapshotSRV = CGameInstance::Get().Get_RT_SRV(TEXT("Target_EffectSceneColor"));
	if (!sourceSRV || !snapshotSRV)
		return E_FAIL;
	ComPtr<ID3D11Resource> sourceResource, snapshotResource;
	sourceSRV->GetResource(sourceResource.GetAddressOf());
	snapshotSRV->GetResource(snapshotResource.GetAddressOf());
	ComPtr<ID3D11Texture2D> sourceTexture, snapshotTexture;
	if (!sourceResource || !snapshotResource || sourceResource.Get() == snapshotResource.Get() ||
		FAILED(sourceResource.As(&sourceTexture)) || FAILED(snapshotResource.As(&snapshotTexture)))
		return E_FAIL;
	const auto bloomSnapshotSRV = CGameInstance::Get().Get_RT_SRV(TEXT("Target_EffectSceneBloom"));
	ComPtr<ID3D11Resource> bloomSnapshotResource;
	ComPtr<ID3D11Texture2D> bloomSnapshotTexture;
	if (!bloomSnapshotSRV) return E_FAIL;
	bloomSnapshotSRV->GetResource(bloomSnapshotResource.GetAddressOf());
	if (!bloomSnapshotResource || FAILED(bloomSnapshotResource.As(&bloomSnapshotTexture))) return E_FAIL;
	D3D11_TEXTURE2D_DESC sourceDesc{}, snapshotDesc{};
	sourceTexture->GetDesc(&sourceDesc);
	snapshotTexture->GetDesc(&snapshotDesc);
	if (sourceDesc.Width != snapshotDesc.Width || sourceDesc.Height != snapshotDesc.Height ||
		sourceDesc.Format != snapshotDesc.Format || sourceDesc.MipLevels != snapshotDesc.MipLevels ||
		sourceDesc.ArraySize != snapshotDesc.ArraySize ||
		sourceDesc.SampleDesc.Count != snapshotDesc.SampleDesc.Count ||
		sourceDesc.SampleDesc.Quality != snapshotDesc.SampleDesc.Quality)
		return E_INVALIDARG;

	// Copy outside the output binding, then restore all MRTs and the same DSV.
	// Begin_MRT would clear the accumulated scene, so it must not be used here.
	ID3D11RenderTargetView* outputs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	ComPtr<ID3D11DepthStencilView> depth;
	m_pContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
		outputs, depth.GetAddressOf());
	ComPtr<ID3D11Resource> activeSceneResource;
	if (outputs[0]) outputs[0]->GetResource(activeSceneResource.GetAddressOf());
	if (activeSceneResource.Get() != sourceResource.Get())
	{
		for (auto* output : outputs)
			if (output) output->Release();
		return E_INVALIDARG;
	}
	m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
	ID3D11ShaderResourceView* emptySRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, emptySRVs);
    HRESULT result = E_FAIL;
    {
        CProfilerGpuScope copyScope(profiler, "Render.SceneColorCopy");
        result = CGameInstance::Get().Copy_RT_Resource(
            TEXT("Target_SceneHDR"), snapshotTexture);
        if (SUCCEEDED(result))
            result = CGameInstance::Get().Copy_RT_Resource(TEXT("Target_SceneBloom"), bloomSnapshotTexture);
    }
    if (SUCCEEDED(result) && profiler)
    {
        profiler->Add_Counter(EProfilerCounter::SceneColorCopies, 2u);
        // Both named targets are created as RGBA16F. Count the copied logical
        // payload, not read+write bus traffic or driver allocation overhead.
        uint64_t pixels = 0;
        uint32_t width = sourceDesc.Width;
        uint32_t height = sourceDesc.Height;
        for (uint32_t mip = 0; mip < sourceDesc.MipLevels; ++mip)
        {
            pixels += static_cast<uint64_t>(width) * height;
            width = (std::max)(1u, width / 2);
            height = (std::max)(1u, height / 2);
        }
        profiler->Add_Counter(EProfilerCounter::SceneColorCopyBytes,
            pixels * 16u * sourceDesc.ArraySize * sourceDesc.SampleDesc.Count);
    }
	m_pContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
		outputs, depth.Get());
	for (auto* output : outputs)
		if (output) output->Release();
	return result;
}

HRESULT CRenderer::Render_Blend()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.Blend");
    CProfilerGpuScope gpuScope(profiler, "Render.Blend", true);
    HRESULT hFirstFailure = S_OK;

    struct BLEND_ENTRY final
    {
        Engine::BLEND_SORT_KEY key;
        CGameObject* object;
    };
    // Render-thread-only scratch; the owning queue is not reordered.
    static vector<BLEND_ENTRY> sorted;
    sorted.clear();
    const auto& queue = m_RenderObjects[ETOUI(RENDERGROUP::BLEND)];
    sorted.reserve(queue.size());
    const float4_t* const cameraPosition = CGameInstance::Get().Get_CamPosition();
    const vector_t camera = cameraPosition ? XMLoadFloat4(cameraPosition) : XMVectorZero();
    for (const auto& owner : queue)
    {
        CGameObject* const object = owner.get();
        if (!object) continue;
        f32_t distanceSquared = FLT_MAX;
        if (cameraPosition)
        {
            const auto transform = dynamic_pointer_cast<CTransform>(
                object->Get_Component(g_strTransformComTag));
            if (transform)
            {
                const vector_t delta = transform->Get_State(STATE::POSITION) - camera;
                distanceSquared = XMVectorGetX(XMVector3LengthSq(delta));
                if (!std::isfinite(distanceSquared)) distanceSquared = FLT_MAX;
            }
        }
        sorted.push_back({ { object->Get_BlendSortPriority(), distanceSquared }, object });
    }
    std::stable_sort(sorted.begin(), sorted.end(),
        [](const BLEND_ENTRY& lhs, const BLEND_ENTRY& rhs)
        {
            return Engine::BlendSortBefore(lhs.key, rhs.key);
        });
    for (const auto& entry : sorted)
    {
        const HRESULT result = entry.object->Render_Group(RENDERGROUP::BLEND);
        if (FAILED(result) && SUCCEEDED(hFirstFailure))
        {
            WriteRendererFailure("Render_Blend_Object", result, typeid(*entry.object).name());
            hFirstFailure = result;
        }
    }
    m_RenderObjects[ETOUI(RENDERGROUP::BLEND)].clear();
    sorted.clear();
    return hFirstFailure;
}

HRESULT CRenderer::Render_ScreenPostPass(
	ComPtr<ID3D11ShaderResourceView> pSourceSRV,
	ComPtr<ID3D11RenderTargetView> pDestinationRTV,
	const uint32_t iPassIndex,
	ComPtr<ID3D11ShaderResourceView> pBloomSourceSRV,
	ComPtr<ID3D11RenderTargetView> pBloomDestinationRTV,
	const PRESENTATION_SCREEN_POST_DESC* pPostDesc)
{
	if (nullptr == pDestinationRTV || nullptr == pBloomDestinationRTV)
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fSceneBloomIntensity", &m_RenderQualitySettings.fBloomIntensity, sizeof(float))) ||
		FAILED(m_pShader->Bind_RawValue("g_fEffectBloomThreshold", &m_RenderQualitySettings.fBloomThreshold, sizeof(float))) ||
		FAILED(m_pShader->Bind_RawValue("g_fEffectBloomSoftKnee", &m_RenderQualitySettings.fBloomSoftKnee, sizeof(float))))
		return E_FAIL;
	if (nullptr != pSourceSRV)
	{
		ComPtr<ID3D11Resource> pSourceResource;
		ComPtr<ID3D11Resource> pDestinationResource;
		pSourceSRV->GetResource(&pSourceResource);
		pDestinationRTV->GetResource(&pDestinationResource);
		if (nullptr == pSourceResource || nullptr == pDestinationResource ||
			pSourceResource.Get() == pDestinationResource.Get())
		{
			return E_FAIL;
		}
	}

	ID3D11ShaderResourceView* pNullSRVs[
		D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(
		0u, _countof(pNullSRVs), pNullSRVs);
	ID3D11RenderTargetView* destinations[3] = {
		pDestinationRTV.Get(), nullptr, pBloomDestinationRTV.Get() };
	m_pContext->OMSetRenderTargets(3u, destinations, nullptr);
	const float4_t vClear{};
	m_pContext->ClearRenderTargetView(destinations[0], &vClear.x);
	m_pContext->ClearRenderTargetView(destinations[2], &vClear.x);

	if (nullptr != pPostDesc && nullptr != pPostDesc->pMaterial)
	{
		PRESENTATION_SCREEN_POST_MATERIAL_INPUT Input;
		Input.pSceneColor = pSourceSRV;
		Input.pSceneBloom = pBloomSourceSRV;
		Input.pSceneDepth = CGameInstance::Get().Get_RT_SRV(TEXT("Target_Depth"));
		Input.World = m_WorldMatrix;
		Input.View = m_ViewMatrix;
		Input.Projection = m_ProjMatrix;
		if (nullptr == Input.pSceneColor || nullptr == Input.pSceneBloom || nullptr == Input.pSceneDepth ||
			FAILED(pPostDesc->pMaterial->Bind(Input)) ||
			FAILED(m_pVIBuffer->Bind_Resources()) || FAILED(m_pVIBuffer->Render()))
			return E_FAIL;
		return S_OK;
	}
	if (nullptr == pPostDesc)
	{
		if (FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_SceneHDR"), m_pShader,
				"g_SceneHDRTexture")) ||
			FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_Distortion"), m_pShader,
				"g_DistortionTexture")) ||
			FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_SceneBloom"), m_pShader, "g_SceneBloomTexture")))
		{
			return E_FAIL;
		}
	}
	else
	{
		if (nullptr == pSourceSRV || nullptr == pBloomSourceSRV ||
			FAILED(m_pShader->Bind_Texture("g_PostBloomTexture", pBloomSourceSRV)) ||
			FAILED(m_pShader->Bind_Texture(
				"g_PostProcessTexture", pSourceSRV)) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationTime", &pPostDesc->fSampleTimeSeconds,
				sizeof(pPostDesc->fSampleTimeSeconds))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationIntensity", &pPostDesc->fIntensity,
				sizeof(pPostDesc->fIntensity))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationSecondaryIntensity",
				&pPostDesc->fSecondaryIntensity,
				sizeof(pPostDesc->fSecondaryIntensity))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationFrequency", &pPostDesc->fFrequency,
				sizeof(pPostDesc->fFrequency))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_iPresentationSeed", &pPostDesc->iRandomSeed,
				sizeof(pPostDesc->iRandomSeed))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_vPresentationTint", &pPostDesc->vTint,
				sizeof(pPostDesc->vTint))))
		{
			return E_FAIL;
		}
	}

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Begin(iPassIndex)) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CRenderer::Render_SceneReplacements()
{
    const auto& posts = CPresentation_Manager::Get().Get_ScreenPosts();
    const bool hasReplacement = std::any_of(posts.begin(), posts.end(), [](const auto& post) {
        return post.pMaterial && post.pMaterial->Replaces_SceneBeforeBlend();
    });
    if (!hasReplacement) return S_OK;
    auto color = CGameInstance::Get().Get_RT_SRV(TEXT("Target_SceneHDR"));
    auto bloom = CGameInstance::Get().Get_RT_SRV(TEXT("Target_SceneBloom"));
    if (!color || !bloom) return E_FAIL;
    ID3D11RenderTargetView* saved[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
    ID3D11DepthStencilView* depth = nullptr;
    m_pContext->OMGetRenderTargets(_countof(saved), saved, &depth);
    HRESULT result = S_OK;
    for (const auto& post : posts)
    {
        if (!post.pMaterial || !post.pMaterial->Replaces_SceneBeforeBlend()) continue;
        result = Render_ScreenPostPass(color, m_pScenePostRTVs[0], 0u,
            bloom, m_pSceneBloomPostRTVs[0], &post);
        if (FAILED(result)) break;
        ID3D11ShaderResourceView* clear[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
        m_pContext->PSSetShaderResources(0u, _countof(clear), clear);
        m_pContext->OMSetRenderTargets(0u, nullptr, nullptr);
        const auto copy = [this](ID3D11ShaderResourceView* source, ID3D11ShaderResourceView* destination) {
            ComPtr<ID3D11Resource> sourceResource, destinationResource;
            source->GetResource(&sourceResource); destination->GetResource(&destinationResource);
            ComPtr<ID3D11Texture2D> sourceTexture, destinationTexture;
            if (!sourceResource || !destinationResource || sourceResource.Get() == destinationResource.Get() ||
                FAILED(sourceResource.As(&sourceTexture)) || FAILED(destinationResource.As(&destinationTexture))) return E_FAIL;
            D3D11_TEXTURE2D_DESC a{}, b{};
            sourceTexture->GetDesc(&a); destinationTexture->GetDesc(&b);
            if (a.Width != b.Width || a.Height != b.Height || a.Format != b.Format ||
                a.ArraySize != b.ArraySize || a.MipLevels != b.MipLevels ||
                a.SampleDesc.Count != b.SampleDesc.Count || a.SampleDesc.Quality != b.SampleDesc.Quality) return E_FAIL;
            m_pContext->CopyResource(destinationResource.Get(), sourceResource.Get());
            return S_OK;
        };
        result = copy(m_pScenePostSRVs[0].Get(), color.Get());
        if (SUCCEEDED(result)) result = copy(m_pSceneBloomPostSRVs[0].Get(), bloom.Get());
        if (FAILED(result)) break;
    }
    ID3D11ShaderResourceView* clear[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
    m_pContext->PSSetShaderResources(0u, _countof(clear), clear);
    m_pContext->OMSetRenderTargets(_countof(saved), saved, depth);
    for (auto* target : saved) if (target) target->Release();
    if (depth) depth->Release();
    return result;
}

HRESULT CRenderer::Render_ScreenPosts()
{
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	ComPtr<ID3D11RenderTargetView> pOriginalRTV;
	ComPtr<ID3D11DepthStencilView> pOriginalDSV;
	m_pContext->OMGetRenderTargets(
		1u, pOriginalRTV.GetAddressOf(), pOriginalDSV.GetAddressOf());
	D3D11_VIEWPORT OriginalViewports[
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	UINT iViewportCount = _countof(OriginalViewports);
	m_pContext->RSGetViewports(&iViewportCount, OriginalViewports);
	SetUp_ViewportDesc(m_iScenePostWidth, m_iScenePostHeight);

	HRESULT hResult = Render_ScreenPostPass(
		nullptr, m_pScenePostRTVs[0], DEFERRED_PASS_SCENE_RESOLVE,
		nullptr, m_pSceneBloomPostRTVs[0]);
	const vector<PRESENTATION_SCREEN_POST_DESC>& ScreenPosts =
		Presentation.Get_ScreenPosts();
	size_t composedPostCount = 0u;
	for (size_t iPost = 0u;
		SUCCEEDED(hResult) && iPost < ScreenPosts.size(); ++iPost)
	{
		const PRESENTATION_SCREEN_POST_DESC& Post = ScreenPosts[iPost];
		if (Post.pMaterial && Post.pMaterial->Replaces_SceneBeforeBlend()) continue;
		const PRESENTATION_SCREEN_POST_PLAN_STEP Step =
			Build_PresentationScreenPostPlanStep(composedPostCount++);
		uint32_t iPassIndex = {};
		switch (Post.eProfile)
		{
		case PRESENTATION_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_RGB_NOISE;
			break;
		case PRESENTATION_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_ZOOM_BLUR;
			break;
		case PRESENTATION_SCREEN_POST_PROFILE::FILM_NOISE_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_FILM_NOISE;
			break;
		case PRESENTATION_SCREEN_POST_PROFILE::CHROMATIC_ABERRATION_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_CHROMATIC_ABERRATION;
			break;
		case PRESENTATION_SCREEN_POST_PROFILE::PREPARED_MATERIAL:
			iPassIndex = 0u;
			break;
		default:
			hResult = E_FAIL;
			continue;
		}
		hResult = Render_ScreenPostPass(
			m_pScenePostSRVs[Step.iSourceTarget],
			m_pScenePostRTVs[Step.iDestinationTarget],
			iPassIndex, m_pSceneBloomPostSRVs[Step.iSourceTarget],
			m_pSceneBloomPostRTVs[Step.iDestinationTarget], &Post);
	}
	const vector<PRESENTATION_SCREEN_OVERLAY_DESC>& ScreenOverlays =
		Presentation.Get_ScreenOverlays();
	size_t hdrOverlayCount = 0u;
	for (size_t iOverlay = 0u;
		SUCCEEDED(hResult) && iOverlay < ScreenOverlays.size(); ++iOverlay)
	{
		const PRESENTATION_SCREEN_OVERLAY_DESC& Overlay =
			ScreenOverlays[iOverlay];
		if (Overlay.bDisplaySpace)
			continue;
		const PRESENTATION_SCREEN_POST_PLAN_STEP Step =
			Build_PresentationScreenOverlayPlanStep(
				composedPostCount, hdrOverlayCount++);
		ComPtr<ID3D11ShaderResourceView> pSourceSRV =
			m_pScenePostSRVs[Step.iSourceTarget];
		ComPtr<ID3D11RenderTargetView> pDestinationRTV =
			m_pScenePostRTVs[Step.iDestinationTarget];
		if (nullptr == pSourceSRV || nullptr == pDestinationRTV ||
			nullptr == Overlay.pTexture)
		{
			hResult = E_FAIL;
			break;
		}
		ComPtr<ID3D11Resource> pSourceResource;
		ComPtr<ID3D11Resource> pDestinationResource;
		ComPtr<ID3D11Resource> pOverlayResource;
		pSourceSRV->GetResource(&pSourceResource);
		pDestinationRTV->GetResource(&pDestinationResource);
		Overlay.pTexture->GetResource(&pOverlayResource);
		if (nullptr == pSourceResource || nullptr == pDestinationResource ||
			nullptr == pOverlayResource ||
			pSourceResource.Get() == pDestinationResource.Get() ||
			pOverlayResource.Get() == pDestinationResource.Get())
		{
			hResult = E_FAIL;
			break;
		}

		ID3D11ShaderResourceView* pNullSRVs[
			D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
		m_pContext->PSSetShaderResources(
			0u, _countof(pNullSRVs), pNullSRVs);
		ID3D11RenderTargetView* destinations[3] = { pDestinationRTV.Get(),
			nullptr, m_pSceneBloomPostRTVs[Step.iDestinationTarget].Get() };
		m_pContext->OMSetRenderTargets(3u, destinations, nullptr);
		const float4_t vClear{};
		m_pContext->ClearRenderTargetView(destinations[0], &vClear.x);
		m_pContext->ClearRenderTargetView(destinations[2], &vClear.x);
		if (FAILED(m_pShader->Bind_Texture("g_PostProcessTexture", pSourceSRV)) ||
			FAILED(m_pShader->Bind_Texture("g_PostBloomTexture", m_pSceneBloomPostSRVs[Step.iSourceTarget])) ||
			FAILED(Render_ScreenOverlay(Overlay)))
			hResult = E_FAIL;
	}
	m_iScenePostFinalTarget = PresentationScreenCompositionFinalTarget(
		composedPostCount, hdrOverlayCount);

	ID3D11ShaderResourceView* pNullSRVs[
		D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(
		0u, _countof(pNullSRVs), pNullSRVs);
	ID3D11RenderTargetView* pOriginal = pOriginalRTV.Get();
	m_pContext->OMSetRenderTargets(
		nullptr == pOriginal ? 0u : 1u,
		nullptr == pOriginal ? nullptr : &pOriginal,
		pOriginalDSV.Get());
	if (0u < iViewportCount)
		m_pContext->RSSetViewports(iViewportCount, OriginalViewports);
	else
		m_pContext->RSSetViewports(0u, nullptr);
	Presentation.Clear_ScreenPosts();
	return hResult;
}

HRESULT CRenderer::Render_ScreenOverlay(
	const PRESENTATION_SCREEN_OVERLAY_DESC& Overlay)
{
	const uint32_t iCoverageChannel =
		static_cast<uint32_t>(Overlay.eCoverageChannel);
	const uint32_t iFilter = static_cast<uint32_t>(Overlay.eFilter);
	const uint32_t iAddress = static_cast<uint32_t>(Overlay.eAddress);
	if (FAILED(m_pShader->Bind_RawValue("g_fEffectBloomIntensity", &Overlay.fBloomIntensity, sizeof(Overlay.fBloomIntensity))) ||
		FAILED(m_pShader->Bind_Texture(
			"g_PresentationOverlayTexture", Overlay.pTexture)) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fPresentationTime", &Overlay.fSampleTimeSeconds,
			sizeof(Overlay.fSampleTimeSeconds))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vPresentationOverlayPosition", &Overlay.vPosition,
			sizeof(Overlay.vPosition))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vPresentationOverlayScale", &Overlay.vScale,
			sizeof(Overlay.vScale))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fPresentationOverlayRotationDegrees",
			&Overlay.fRotationDegrees,
			sizeof(Overlay.fRotationDegrees))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fPresentationOverlayAngularVelocityDegreesPerSecond",
			&Overlay.fAngularVelocityDegreesPerSecond,
			sizeof(Overlay.fAngularVelocityDegreesPerSecond))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vPresentationOverlayUvDriftPerSecond",
			&Overlay.vUvDriftPerSecond,
			sizeof(Overlay.vUvDriftPerSecond))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vPresentationOverlayTint", &Overlay.vTint,
			sizeof(Overlay.vTint))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fPresentationOverlayAlpha", &Overlay.fAlpha,
			sizeof(Overlay.fAlpha))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iPresentationOverlayCoverageChannel",
			&iCoverageChannel, sizeof(iCoverageChannel))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iPresentationOverlayFilter", &iFilter,
			sizeof(iFilter))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iPresentationOverlayAddress", &iAddress,
			sizeof(iAddress))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Begin(Overlay.bDisplaySpace ?
			ETOUI(DEFERRED::PRESENTATION_DISPLAY_OVERLAY) :
			DEFERRED_PASS_TEXTURED_OVERLAY)) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CRenderer::Render_DisplayOverlays()
{
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	HRESULT result = S_OK;
	for (const auto& overlay : Presentation.Get_ScreenOverlays())
	{
		if (!overlay.bDisplaySpace)
			continue;
		// No scene SRV is sampled: alpha blend directly over the final display.
		if (FAILED(m_pShader->Bind_Texture("g_PostProcessTexture", nullptr)) ||
			FAILED(Render_ScreenOverlay(overlay)))
		{
			result = E_FAIL;
			break;
		}
	}
	ID3D11ShaderResourceView* nullSrvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(0u, _countof(nullSrvs), nullSrvs);
	Presentation.Clear_ScreenOverlays();
	return result;
}

HRESULT CRenderer::Render_Bloom()
{
	D3D11_VIEWPORT OriginalViewports[
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	UINT iViewportCount = _countof(OriginalViewports);
	m_pContext->RSGetViewports(&iViewportCount, OriginalViewports);

	SetUp_ViewportDesc(m_iBloomWidth, m_iBloomHeight);

	HRESULT hResult = Render_BloomPass(
		TEXT("MRT_BloomExtract"),
		m_pSceneBloomPostSRVs[m_iScenePostFinalTarget],
		DEFERRED::BLOOM_EXTRACT);
	if (SUCCEEDED(hResult))
	{
		hResult = Render_BloomPass(
			TEXT("MRT_BloomPing"), nullptr,
			DEFERRED::BLOOM_BLUR_H);
	}
	if (SUCCEEDED(hResult))
	{
		hResult = Render_BloomPass(
			TEXT("MRT_BloomResult"), nullptr,
			DEFERRED::BLOOM_BLUR_V);
	}

	/* Restore every viewport exactly as it was before the half-resolution pass. */
	if (0 < iViewportCount)
		m_pContext->RSSetViewports(iViewportCount, OriginalViewports);
	else
		m_pContext->RSSetViewports(0, nullptr);

	return hResult;
}

HRESULT CRenderer::Render_BloomPass(const wstring_t& strMRTTag,
	ComPtr<ID3D11ShaderResourceView> pSourceSRV, DEFERRED ePass)
{
	if (FAILED(CGameInstance::Get().Begin_MRT(strMRTTag, m_pBloomDSV)))
		return E_FAIL;

	HRESULT hResult = S_OK;
	HRESULT hBindSource = S_OK;
	if (nullptr != pSourceSRV)
		hBindSource = m_pShader->Bind_Texture(
			"g_PostProcessTexture", pSourceSRV);
	else if (DEFERRED::BLOOM_BLUR_H == ePass)
		hBindSource = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_BloomExtract"), m_pShader,
			"g_PostProcessTexture");
	else if (DEFERRED::BLOOM_BLUR_V == ePass)
		hBindSource = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_BloomPing"), m_pShader,
			"g_PostProcessTexture");
	else
		hBindSource = E_FAIL;
	if (FAILED(hBindSource) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vBloomTexelSize", &m_vBloomTexelSize,
			sizeof(m_vBloomTexelSize))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fBloomThreshold", &m_RenderQualitySettings.fBloomThreshold,
			sizeof(m_RenderQualitySettings.fBloomThreshold))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fBloomSoftKnee", &m_RenderQualitySettings.fBloomSoftKnee,
			sizeof(m_RenderQualitySettings.fBloomSoftKnee))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fBloomScatter", &m_RenderQualitySettings.fBloomScatter,
			sizeof(m_RenderQualitySettings.fBloomScatter))) ||
		FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Begin(ETOUI(ePass))) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		hResult = E_FAIL;
	}

	/* Keep render-target state balanced even if binding or drawing failed. */
	if (FAILED(CGameInstance::Get().End_MRT()))
		hResult = E_FAIL;

	return hResult;
}

HRESULT CRenderer::Render_Final()
{
	const uint32_t materialView = static_cast<uint32_t>(m_MaterialRenderSettings.eDebugView);
	if (FAILED(m_pShader->Bind_RawValue("g_MaterialDebugView", &materialView, sizeof(materialView))) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterSurface"), m_pShader, "g_CharacterSurfaceTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterGeometry"), m_pShader, "g_CharacterGeometryTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Specular"), m_pShader, "g_SpecularTexture")))
		return E_FAIL;
	const uint32_t iBloomEnabled =
		m_RenderQualitySettings.bBloomEnabled ? 1u : 0u;
	const uint32_t iFXAAEnabled =
		m_RenderQualitySettings.bFXAAEnabled ? 1u : 0u;
	const float2_t vInverseSceneSize = {
		1.f / static_cast<f32_t>((max)(1u, m_iScenePostWidth)),
		1.f / static_cast<f32_t>((max)(1u, m_iScenePostHeight)) };

	if (FAILED(m_pShader->Bind_Texture(
		"g_SceneHDRTexture",
		m_pScenePostSRVs[m_iScenePostFinalTarget])))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_BloomResult"), m_pShader, "g_BloomTexture")))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue(
			"g_iBloomEnabled", &iBloomEnabled, sizeof(iBloomEnabled))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fToneMapExposure", &m_RenderQualitySettings.fExposure,
			sizeof(m_RenderQualitySettings.fExposure))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fToneMapWhitePoint", &m_RenderQualitySettings.fWhitePoint,
			sizeof(m_RenderQualitySettings.fWhitePoint))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fToneMapGamma", &m_RenderQualitySettings.fGamma,
			sizeof(m_RenderQualitySettings.fGamma))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iFXAAEnabled", &iFXAAEnabled, sizeof(iFXAAEnabled))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fFXAASubpixel", &m_RenderQualitySettings.fFXAASubpixel,
			sizeof(m_RenderQualitySettings.fFXAASubpixel))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fFXAAEdgeThreshold", &m_RenderQualitySettings.fFXAAEdgeThreshold,
			sizeof(m_RenderQualitySettings.fFXAAEdgeThreshold))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fFXAAEdgeThresholdMin", &m_RenderQualitySettings.fFXAAEdgeThresholdMin,
			sizeof(m_RenderQualitySettings.fFXAAEdgeThresholdMin))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vInverseSceneSize", &vInverseSceneSize,
			sizeof(vInverseSceneSize))))
	{
		return E_FAIL;
	}

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Begin(ETOUI(DEFERRED::FINAL))))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_UI()
{
	for (size_t renderIndex = 0; renderIndex < m_RenderObjects[ETOUI(RENDERGROUP::UI)].size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = m_RenderObjects[ETOUI(RENDERGROUP::UI)][renderIndex].get();
		if (nullptr != pRenderObject)
			pRenderObject->Render_Group(RENDERGROUP::UI);
	}

	m_RenderObjects[ETOUI(RENDERGROUP::UI)].clear();

	return S_OK;
}

HRESULT CRenderer::Ready_SourceLightMask(uint32_t width, uint32_t height)
{
    if (width == 0u || height == 0u) return S_FALSE;
    if (width == m_iSourceLightMaskFailedWidth && height == m_iSourceLightMaskFailedHeight)
        return S_FALSE;
    if (m_pSourceLightMaskDSV && width == m_iSourceLightMaskWidth && height == m_iSourceLightMaskHeight)
        return S_OK;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = desc.ArraySize = 1u;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count = 1u;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11DepthStencilView> depth;
    HRESULT result = m_pDevice->CreateTexture2D(&desc, nullptr, texture.GetAddressOf());
    if (SUCCEEDED(result)) result = m_pDevice->CreateDepthStencilView(texture.Get(), nullptr, depth.GetAddressOf());
    if (FAILED(result))
    {
        m_iSourceLightMaskFailedWidth = width;
        m_iSourceLightMaskFailedHeight = height;
        WriteRendererFailure("SourceLightMask_ResourceFallback", result);
        return S_FALSE;
    }
    m_pSourceLightMaskDSV = std::move(depth);
    m_iSourceLightMaskWidth = width;
    m_iSourceLightMaskHeight = height;
    return S_OK;
}

HRESULT CRenderer::Ready_Shadow_Resources()
{
	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = m_iShadowMapSize;
	TextureDesc.Height = m_iShadowMapSize;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags =
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> pStagedTexture;
	if (FAILED(m_pDevice->CreateTexture2D(
		&TextureDesc, nullptr, pStagedTexture.GetAddressOf())))
		return E_FAIL;

	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc{};
	DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Texture2D.MipSlice = 0;
	ComPtr<ID3D11DepthStencilView> pStagedDSV;
	if (FAILED(m_pDevice->CreateDepthStencilView(
		pStagedTexture.Get(), &DSVDesc, pStagedDSV.GetAddressOf())))
		return E_FAIL;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 1;
	ComPtr<ID3D11ShaderResourceView> pStagedSRV;
	if (FAILED(m_pDevice->CreateShaderResourceView(
		pStagedTexture.Get(), &SRVDesc, pStagedSRV.GetAddressOf())))
	{
		return E_FAIL;
	}

	m_pShadowDepthTexture = std::move(pStagedTexture);
	m_pShadowDSV = std::move(pStagedDSV);
	m_pShadowSRV = std::move(pStagedSRV);
	return S_OK;
}

HRESULT CRenderer::Ready_Bloom_DSV()
{
	ComPtr<ID3D11Texture2D> pDepthStencilTexture = { nullptr };

	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = m_iBloomWidth;
	TextureDesc.Height = m_iBloomHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (FAILED(m_pDevice->CreateTexture2D(
		&TextureDesc, nullptr, pDepthStencilTexture.GetAddressOf())))
	{
		return E_FAIL;
	}

	if (FAILED(m_pDevice->CreateDepthStencilView(
		pDepthStencilTexture.Get(), nullptr, m_pBloomDSV.GetAddressOf())))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CRenderer::Ready_SSAO_DSV()
{
	ComPtr<ID3D11Texture2D> pDepthStencilTexture = { nullptr };

	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = m_iSSAOWidth;
	TextureDesc.Height = m_iSSAOHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (FAILED(m_pDevice->CreateTexture2D(
		&TextureDesc, nullptr, pDepthStencilTexture.GetAddressOf())))
	{
		return E_FAIL;
	}

	if (FAILED(m_pDevice->CreateDepthStencilView(
		pDepthStencilTexture.Get(), nullptr, m_pSSAODSV.GetAddressOf())))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CRenderer::Ready_ScenePostTargets(
	const uint32_t iWidth, const uint32_t iHeight)
{
	if (0u == iWidth || 0u == iHeight)
		return E_FAIL;
	if (m_iScenePostWidth == iWidth && m_iScenePostHeight == iHeight &&
		nullptr != m_pScenePostTextures[0] &&
		nullptr != m_pScenePostTextures[1] &&
		nullptr != m_pScenePostRTVs[0] &&
		nullptr != m_pScenePostRTVs[1] &&
		nullptr != m_pScenePostSRVs[0] &&
		nullptr != m_pScenePostSRVs[1] &&
		nullptr != m_pSceneBloomPostSRVs[0] && nullptr != m_pSceneBloomPostSRVs[1])
	{
		return S_OK;
	}

	ComPtr<ID3D11Texture2D> StagedTextures[2];
	ComPtr<ID3D11RenderTargetView> StagedRTVs[2];
	ComPtr<ID3D11ShaderResourceView> StagedSRVs[2];
	ComPtr<ID3D11Texture2D> StagedBloomTextures[2];
	ComPtr<ID3D11RenderTargetView> StagedBloomRTVs[2];
	ComPtr<ID3D11ShaderResourceView> StagedBloomSRVs[2];
	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1u;
	TextureDesc.ArraySize = 1u;
	TextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	TextureDesc.SampleDesc.Count = 1u;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags =
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	for (size_t iTarget = 0u; iTarget < 2u; ++iTarget)
	{
		if (FAILED(m_pDevice->CreateTexture2D(
				&TextureDesc, nullptr,
				StagedTextures[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateRenderTargetView(
				StagedTextures[iTarget].Get(), nullptr,
				StagedRTVs[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateShaderResourceView(
				StagedTextures[iTarget].Get(), nullptr,
				StagedSRVs[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, StagedBloomTextures[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateRenderTargetView(StagedBloomTextures[iTarget].Get(), nullptr, StagedBloomRTVs[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateShaderResourceView(StagedBloomTextures[iTarget].Get(), nullptr, StagedBloomSRVs[iTarget].GetAddressOf())))
		{
			return E_FAIL;
		}
	}

	for (size_t iTarget = 0u; iTarget < 2u; ++iTarget)
	{
		m_pScenePostTextures[iTarget] = std::move(StagedTextures[iTarget]);
		m_pScenePostRTVs[iTarget] = std::move(StagedRTVs[iTarget]);
		m_pScenePostSRVs[iTarget] = std::move(StagedSRVs[iTarget]);
		m_pSceneBloomPostRTVs[iTarget] = std::move(StagedBloomRTVs[iTarget]);
		m_pSceneBloomPostSRVs[iTarget] = std::move(StagedBloomSRVs[iTarget]);
	}
	m_iScenePostWidth = iWidth;
	m_iScenePostHeight = iHeight;
	m_iScenePostFinalTarget = 0u;
	XMStoreFloat4x4(&m_WorldMatrix,
		XMMatrixScaling(static_cast<f32_t>(iWidth),
			static_cast<f32_t>(iHeight), 1.f));
	XMStoreFloat4x4(&m_ProjMatrix,
		XMMatrixOrthographicLH(static_cast<f32_t>(iWidth),
			static_cast<f32_t>(iHeight), 0.f, 1.f));
	return S_OK;
}

void CRenderer::SetUp_ViewportDesc(uint32_t iWidth, uint32_t iHeight)
{
	D3D11_VIEWPORT			ViewPortDesc;
	ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
	ViewPortDesc.TopLeftX = 0;
	ViewPortDesc.TopLeftY = 0;
	ViewPortDesc.Width = static_cast<f32_t>(iWidth);
	ViewPortDesc.Height = static_cast<f32_t>(iHeight);
	ViewPortDesc.MinDepth = 0.f;
	ViewPortDesc.MaxDepth = 1.f;

	m_pContext->RSSetViewports(1, &ViewPortDesc);
}

#ifdef _DEBUG

HRESULT CRenderer::Render_Debug()
{
	for (auto& pDebugComponent : m_DebugComponent)
	{
		if (nullptr != pDebugComponent)
			pDebugComponent->Render();
	}
	m_DebugComponent.clear();

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	//if (FAILED(CGameInstance::Get().Render_MRT(TEXT("MRT_GameObject"), m_pShader, m_pVIBuffer)))
	//	return E_FAIL;

	//if (FAILED(CGameInstance::Get().Render_MRT(TEXT("MRT_LightAcc"), m_pShader, m_pVIBuffer)))
	//	return E_FAIL;

	return S_OK;
}

#endif

unique_ptr<CRenderer> CRenderer::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CRenderer>(new CRenderer(pDevice, pContext));

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CRenderer");
		return nullptr;
	}

	return pInstance;
}
```

### Client/Public/MapAssetObject.h

대상: `C:/Users/USER/source/졸업팀폴/LostArk/Client/Public/MapAssetObject.h`

```cpp
#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "MapAssetCatalog.h"
#include "MapLoadScope.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CMapAssetObject final : public CGameObject
{
public:
	/* Narrowly scoped runtime treatments for the masked Valtan proxy planes.
	   NONE remains the invariant for every ordinary map asset. */
	enum class PRESENTATION_VORTEX_PROFILE : uint32_t
	{
		NONE = 0u,
		DARK_APERTURE = 1u,
		RED_RING = 2u,
		RED_CLOUD_DISC = 3u,
		END
	};

	struct MAP_ASSET_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		uint32_t prototypeLevelIndex = ETOUI(LEVEL::DEVELOPMENT);
		uint64_t placementId = {};
		std::string assetId;
		std::string assetGroupId;
		std::wstring modelPrototypeTag;
		float3_t position = {};
		float4_t rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
		float3_t signedScale = float3_t(1.f, 1.f, 1.f);
		bool_t applyBottomCenter = false;
		bool_t visible = true;
		MAP_ASSET_RENDER_PROFILE renderProfile;
		Engine::MODEL_BAKED_LIGHTING_INSTANCE bakedLighting;
		MAP_FRUSTUM_CULLING_POLICY frustumCulling{};
		/* Only set when the catalog resolved a water row for this asset. */
		bool_t hasWaterProfile = false;
		MAP_ASSET_WATER_PROFILE waterProfile;
	};

private:
	CMapAssetObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CMapAssetObject();

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Group(RENDERGROUP group) override;
	virtual int32_t Get_BlendSortPriority() const override;
	virtual HRESULT Render_Shadow() override;

	uint64_t Get_PlacementId() const { return m_iPlacementId; }
	const std::string& Get_AssetId() const { return m_AssetId; }
	const float3_t& Get_Position() const { return m_vPlacementPosition; }
	const float4_t& Get_RotationQuaternion() const { return m_vRotationQuaternion; }
	const float3_t& Get_SignedScale() const { return m_vSignedScale; }
	bool_t Is_Visible() const { return m_bVisible; }
	bool_t Is_Mirrored() const { return m_bMirrored; }
	void Set_PlacementTransform(const float3_t& position,
		const float4_t& rotationQuaternion, const float3_t& signedScale);
	void Set_Visible(bool_t visible) { m_bVisible = visible; }
	void Set_PresentationOpacityMultiplier(f32_t multiplier);
	void Set_PresentationVortexProfile(
		PRESENTATION_VORTEX_PROFILE profile,
		f32_t strength);

private:
	uint64_t m_iPlacementId = {};
	std::string m_AssetId;
	std::string m_AssetGroupId;
	float3_t m_vPlacementPosition = {};
	float4_t m_vRotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	float3_t m_vSignedScale = float3_t(1.f, 1.f, 1.f);

	bool_t m_bApplyBottomCenter = false;
	bool_t m_bVisible = true;
	bool_t m_bMirrored = false;
	//Frustum Culling을 위한 멤버 변수 추가 
	bool_t m_bHasLocalCullBounds = false;
	bool_t m_bHasWorldCullBounds = false;
	float3_t m_vLocalCullCenter = {};
	f32_t m_fLocalCullRadius = {};
	float3_t m_vWorldCullCenter = {};
	f32_t m_fWorldCullRadius = {};
	MAP_FRUSTUM_CULLING_POLICY m_FrustumCulling{};
	MAP_FRUSTUM_RUNTIME_STATE m_FrustumState{};

	MAP_ASSET_RENDER_PROFILE m_RenderProfile;
	Engine::MODEL_BAKED_LIGHTING_INSTANCE m_BakedLighting;
	bool_t m_bHasWaterProfile = false;
	MAP_ASSET_WATER_PROFILE m_WaterProfile;
	/* Runtime presentation may fade a placement without mutating the authored
	   catalog profile shared by every occurrence of the asset. */
	f32_t m_fPresentationOpacityMultiplier = 1.f;
	PRESENTATION_VORTEX_PROFILE m_ePresentationVortexProfile =
		PRESENTATION_VORTEX_PROFILE::NONE;
	f32_t m_fPresentationVortexStrength = 0.f;
	f32_t m_fElapsedTime = {};

	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };

private:
	MAP_ASSET_RENDER_PROFILE Get_MaterialRenderProfile(uint32_t meshIndex) const;
	HRESULT Ready_Components(uint32_t prototypeLevelIndex,
		const std::wstring& modelPrototypeTag);
	HRESULT Bind_ShaderResources(
		const struct MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot);
	HRESULT Bind_ShadowShaderResources();
	HRESULT Bind_PresentationVortexShaderResources(
		PRESENTATION_VORTEX_PROFILE profile,
		f32_t strength);
	HRESULT Reset_PresentationVortexShaderResources();
	/* Pushes the authored water parameters and clears them again, so an
	   ordinary asset drawn later through the same shared FX11 effect cannot
	   inherit another placement's water values. */
	HRESULT Bind_WaterShaderResources(bool_t bEnabled);
	//Frustum Culling
	void Ready_CullBounds();
	void Update_WorldCullBounds();

	float3_t Compute_WorldOrigin(const float3_t& placementPosition,
		const float4_t& rotationQuaternion, const float3_t& signedScale) const;
	MAP_ASSET_RENDER_PROFILE Get_EffectiveRenderProfile() const;

public:
	static unique_ptr<CMapAssetObject> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### Client/Private/MapAssetObject.cpp

대상: `C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/MapAssetObject.cpp`

```cpp
#include "MapAssetObject.h"
#include "CardMazeVisualPolicy.h"

#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Profiler.h"

#include "MapAssetRenderUtils.h"

#include <algorithm>
#include <cmath>

namespace
{
    RENDERGROUP MaterialRenderGroup(const MAP_ASSET_RENDER_PROFILE& profile)
    {
        switch (profile.renderMode)
        {
        case MAP_ASSET_RENDER_MODE::DEFERRED: return RENDERGROUP::NONBLEND;
        case MAP_ASSET_RENDER_MODE::BACKGROUND: return RENDERGROUP::PRIORITY;
        case MAP_ASSET_RENDER_MODE::TRANSLUCENT:
        case MAP_ASSET_RENDER_MODE::ADDITIVE:
        case MAP_ASSET_RENDER_MODE::WATER: return RENDERGROUP::BLEND;
        }
        return RENDERGROUP::END;
    }
}

MAP_ASSET_RENDER_PROFILE CMapAssetObject::Get_MaterialRenderProfile(uint32_t meshIndex) const
{
    auto profile = Get_EffectiveRenderProfile();
    const auto* surface = m_pModelCom->Get_MaterialSurface(meshIndex);
    if (!surface || m_fPresentationVortexStrength > 0.f) return profile;
    switch (surface->renderMode)
    {
    case Engine::MODEL_SURFACE_RENDER_MODE::INHERIT: break;
    case Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED: profile.renderMode = MAP_ASSET_RENDER_MODE::DEFERRED; break;
    case Engine::MODEL_SURFACE_RENDER_MODE::TRANSLUCENT: profile.renderMode = MAP_ASSET_RENDER_MODE::TRANSLUCENT; break;
    case Engine::MODEL_SURFACE_RENDER_MODE::BACKGROUND: profile.renderMode = MAP_ASSET_RENDER_MODE::BACKGROUND; break;
    case Engine::MODEL_SURFACE_RENDER_MODE::ADDITIVE: profile.renderMode = MAP_ASSET_RENDER_MODE::ADDITIVE; break;
    case Engine::MODEL_SURFACE_RENDER_MODE::WATER: profile.renderMode = MAP_ASSET_RENDER_MODE::WATER; break;
    }
    switch (surface->cullMode)
    {
    case Engine::MODEL_SURFACE_CULL_MODE::INHERIT: break;
    case Engine::MODEL_SURFACE_CULL_MODE::CULL_BACK: profile.cullMode = MAP_ASSET_CULL_MODE::CULL_BACK; break;
    case Engine::MODEL_SURFACE_CULL_MODE::CULL_FRONT: profile.cullMode = MAP_ASSET_CULL_MODE::CULL_FRONT; break;
    case Engine::MODEL_SURFACE_CULL_MODE::TWO_SIDED: profile.cullMode = MAP_ASSET_CULL_MODE::TWO_SIDED; break;
    }
    profile.castsShadow = profile.castsShadow && surface->castsShadow;
    return profile;
}

CMapAssetObject::CMapAssetObject(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CMapAssetObject::~CMapAssetObject()
{
}

HRESULT CMapAssetObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapAssetObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	//매개 변수로 받은 Arg를 MAP_ASSET_DESC*로 캐스팅해서 초기화 데이터 채워넣기
	const MAP_ASSET_DESC desc = *static_cast<MAP_ASSET_DESC*>(pArg);

	const vector_t quaternion = XMLoadFloat4(&desc.rotationQuaternion);
	const float quaternionLength = XMVectorGetX(XMVector4Length(quaternion));

	if (0 == desc.placementId || desc.assetId.empty() ||
		desc.modelPrototypeTag.empty() ||
		!std::isfinite(quaternionLength) || quaternionLength < 0.000001f ||
		std::abs(desc.signedScale.x) < 0.000001f ||
		std::abs(desc.signedScale.y) < 0.000001f ||
		std::abs(desc.signedScale.z) < 0.000001f)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)) ||
		FAILED(Ready_Components(
			desc.prototypeLevelIndex, desc.modelPrototypeTag)))
		return E_FAIL;

	m_iPlacementId = desc.placementId;
	m_AssetId = desc.assetId;
	m_AssetGroupId = desc.assetGroupId;
	m_bApplyBottomCenter = desc.applyBottomCenter;
	m_bVisible = desc.visible;
	m_RenderProfile = desc.renderProfile;
	m_BakedLighting = desc.bakedLighting;
	m_FrustumCulling = desc.frustumCulling;
	m_bHasWaterProfile = desc.hasWaterProfile;
	m_WaterProfile = desc.waterProfile;
	m_fPresentationOpacityMultiplier = 1.f;
	Set_PresentationVortexProfile(
		PRESENTATION_VORTEX_PROFILE::NONE, 0.f);

	/*CModel이 로드하며 만든 local AABB를 한 번만 bounding sphere로 변환한다.*/
	Ready_CullBounds();

	Set_PlacementTransform(
		desc.position, desc.rotationQuaternion, desc.signedScale);
	return S_OK;
}

void CMapAssetObject::Update(f32_t fTimeDelta)
{
	m_fElapsedTime += fTimeDelta;
	if (m_RenderProfile.renderMode == MAP_ASSET_RENDER_MODE::BACKGROUND)
	{
		const float4_t* cameraPosition = CGameInstance::Get().Get_CamPosition();
		if (nullptr != cameraPosition)
			m_pTransformCom->Set_State(
				STATE::POSITION, XMLoadFloat4(cameraPosition));
	}
}

void CMapAssetObject::Late_Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	Engine::CProfiler* pProfiler =
		CGameInstance::Get().Get_Profiler();

	//Visible 여부와 Frustum 통과 여부와 무관한 전체 placement 개수
	//MapAssetObject가 Update를 돌면서 Profiler의 Counter 증가 시키기 O(N)
	if (nullptr != pProfiler)
	{
		pProfiler->Add_Counter(
			Engine::EProfilerCounter::MapPlacements);

		pProfiler->Add_Counter(
			Engine::EProfilerCounter::MapFallbackObjects);
	}

	if (!m_bVisible)
		return;
    bool queued[static_cast<size_t>(RENDERGROUP::END)]{};
    for (uint32_t mesh = 0; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        const auto profile = Get_MaterialRenderProfile(mesh);
        const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
        if (surface && surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
            surface->sourceCharacter.program >= 38u && surface->sourceCharacter.program <= 43u)
            CGameInstance::Get().Request_SceneColorSnapshot();
        const auto group = MaterialRenderGroup(profile);
        if (group == RENDERGROUP::END) continue;
        queued[static_cast<size_t>(group)] = true;
        if (profile.renderMode == MAP_ASSET_RENDER_MODE::DEFERRED && profile.castsShadow &&
            CGameInstance::Get().Is_ShadowLightEnabled())
            queued[static_cast<size_t>(RENDERGROUP::SHADOW)] = true;
    }
    for (size_t group = 0; group < std::size(queued); ++group)
        if (queued[group]) CGameInstance::Get().Add_RenderObject(static_cast<RENDERGROUP>(group),
            static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CMapAssetObject::Render()
{
    // Direct callers submit the same disjoint subsets as the renderer queues.
    for (const auto group : { RENDERGROUP::PRIORITY, RENDERGROUP::NONBLEND, RENDERGROUP::BLEND })
        if (FAILED(Render_Group(group))) return E_FAIL;
    return S_OK;
}

int32_t CMapAssetObject::Get_BlendSortPriority() const
{
    // This single translucent receiver must precede ordinary world sprites.
    // Do not promote every marker above transparent walls or alter materials.
    if (Client::IsCardMazeFloorReceiver(m_iPlacementId, m_AssetId))
        return -1;
    return 0;
}

HRESULT CMapAssetObject::Render_Group(RENDERGROUP group)
{
	/* Late_Update may already have queued this object when a presentation cue
	   hides it. Re-check at draw time so the previous frame cannot leak through. */
	if (!m_bVisible || m_fPresentationOpacityMultiplier <= 0.f ||
        CGameInstance::Get().Is_SceneEnvironmentReplaced())
		return S_OK;

	MAP_CAMERA_CULL_SNAPSHOT cameraSnapshot{};
	const bool_t hasCameraSnapshot =
		CMapAssetRenderUtils::Capture_CameraCullSnapshot(cameraSnapshot);
	const bool_t background = group == RENDERGROUP::PRIORITY;
	MAP_FRUSTUM_CULL_DECISION cullDecision{};
	if (!background && m_bHasWorldCullBounds && hasCameraSnapshot &&
		CMapAssetRenderUtils::Evaluate_FrustumVisibility(
			m_FrustumCulling,
			cameraSnapshot,
			m_AssetId,
			m_AssetGroupId,
			m_iPlacementId,
			m_vWorldCullCenter,
			m_fWorldCullRadius,
			m_FrustumState,
			cullDecision) &&
		!cullDecision.shouldRender)
	{
		return S_OK;
	}

	if (Engine::CProfiler* profiler =
		CGameInstance::Get().Get_Profiler())
	{
		profiler->Add_Counter(
			Engine::EProfilerCounter::MapVisibleInstances);
	}

    const float4_t worldCullSphere(m_vWorldCullCenter.x, m_vWorldCullCenter.y,
        m_vWorldCullCenter.z, m_fWorldCullRadius);
	HRESULT renderResult = Bind_ShaderResources(
		hasCameraSnapshot ? &cameraSnapshot : nullptr);
	if (SUCCEEDED(renderResult))
	{
        for (uint32_t meshIndex = 0; SUCCEEDED(renderResult) && meshIndex < m_pModelCom->Get_NumMeshes(); ++meshIndex)
        {
            auto presentationProfile = Get_MaterialRenderProfile(meshIndex);
            if (MaterialRenderGroup(presentationProfile) != group) continue;
            const bool_t bWater = presentationProfile.renderMode == MAP_ASSET_RENDER_MODE::WATER && m_bHasWaterProfile;
            if (presentationProfile.renderMode == MAP_ASSET_RENDER_MODE::WATER && !m_bHasWaterProfile)
                presentationProfile.renderMode = MAP_ASSET_RENDER_MODE::TRANSLUCENT;
            const uint32_t passIndex = CMapAssetRenderUtils::Select_Pass(presentationProfile, m_bMirrored);
            presentationProfile.opacity *= m_fPresentationOpacityMultiplier;
            renderResult = Bind_WaterShaderResources(bWater);
            if (FAILED(renderResult)) break;
			if (FAILED(
				CMapAssetRenderUtils::Bind_Material(
					m_pModelCom, m_pShaderCom, meshIndex,
					presentationProfile, m_fElapsedTime, nullptr, m_AssetId, &m_BakedLighting,
                    m_bHasWorldCullBounds ? &worldCullSphere : nullptr)) ||

				FAILED(m_pShaderCom->Begin(passIndex)) ||

				FAILED(m_pModelCom->Render(meshIndex)))
			{
				renderResult = E_FAIL;
				break;
			}
		}
	}

	/* CShader clones share one FX11 effect. Reset even after a failed bind or
	   draw so a later character, prop or ordinary map asset cannot inherit the
	   presentation-only branch. */
	const HRESULT resetResult = Reset_PresentationVortexShaderResources();
	const HRESULT waterResetResult = Bind_WaterShaderResources(false);
	if (FAILED(renderResult))
		return renderResult;
	return FAILED(resetResult) ? resetResult : waterResetResult;
}

HRESULT CMapAssetObject::Render_Shadow()
{
	constexpr uint32_t STATIC_SHADOW_PASS_BASE = 12u;
	if (!m_bVisible || m_fPresentationOpacityMultiplier <= 0.f ||
        CGameInstance::Get().Is_SceneEnvironmentReplaced())
		return S_OK;
    if (!m_RenderProfile.castsShadow) return S_OK;
    if (FAILED(Bind_ShadowShaderResources())) return E_FAIL;

	for (uint32_t iMesh = 0;
		iMesh < m_pModelCom->Get_NumMeshes(); ++iMesh)
	{
        auto presentationProfile = Get_MaterialRenderProfile(iMesh);
        if (presentationProfile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED || !presentationProfile.castsShadow) continue;
        const uint32_t iCullPass = CMapAssetRenderUtils::Select_Pass(presentationProfile, m_bMirrored);
        if (iCullPass > 2u) return E_UNEXPECTED;
        presentationProfile.opacity *= m_fPresentationOpacityMultiplier;
		if (FAILED(CMapAssetRenderUtils::Bind_ShadowMaterial(
				m_pModelCom, m_pShaderCom, iMesh,
				presentationProfile, m_fElapsedTime)) ||
			FAILED(m_pShaderCom->Begin(
				STATIC_SHADOW_PASS_BASE + iCullPass)) ||
			FAILED(m_pModelCom->Render(iMesh)))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

void CMapAssetObject::Set_PresentationOpacityMultiplier(
	const f32_t multiplier)
{
	/* A corrupt presentation value must fail closed instead of reaching the
	   shader as NaN. Valid callers may only attenuate authored opacity. */
	m_fPresentationOpacityMultiplier = std::isfinite(multiplier) ?
		(std::clamp)(multiplier, 0.f, 1.f) : 0.f;
}

void CMapAssetObject::Set_PresentationVortexProfile(
	const PRESENTATION_VORTEX_PROFILE profile,
	const f32_t strength)
{
	const bool_t validProfile =
		PRESENTATION_VORTEX_PROFILE::NONE == profile ||
		PRESENTATION_VORTEX_PROFILE::DARK_APERTURE == profile ||
		PRESENTATION_VORTEX_PROFILE::RED_RING == profile ||
		PRESENTATION_VORTEX_PROFILE::RED_CLOUD_DISC == profile;
	if (!validProfile || !std::isfinite(strength))
	{
		m_ePresentationVortexProfile =
			PRESENTATION_VORTEX_PROFILE::NONE;
		m_fPresentationVortexStrength = 0.f;
		return;
	}
	const f32_t boundedStrength =
		(std::clamp)(strength, 0.f, 1.f);
	if (PRESENTATION_VORTEX_PROFILE::NONE == profile ||
		boundedStrength <= 0.f)
	{
		m_ePresentationVortexProfile =
			PRESENTATION_VORTEX_PROFILE::NONE;
		m_fPresentationVortexStrength = 0.f;
		return;
	}
	m_ePresentationVortexProfile = profile;
	m_fPresentationVortexStrength = boundedStrength;
}

MAP_ASSET_RENDER_PROFILE CMapAssetObject::Get_EffectiveRenderProfile() const
{
	MAP_ASSET_RENDER_PROFILE profile = m_RenderProfile;
	if (!std::isfinite(m_fPresentationVortexStrength) ||
		m_fPresentationVortexStrength <= 0.f)
	{
		return profile;
	}
	if (PRESENTATION_VORTEX_PROFILE::DARK_APERTURE ==
		m_ePresentationVortexProfile)
	{
		/* Additive blending cannot remove light. The aperture uses the same
		   authored texture/opacity but must alpha-blend a dark procedural color. */
		profile.renderMode = MAP_ASSET_RENDER_MODE::TRANSLUCENT;
	}
	else if (PRESENTATION_VORTEX_PROFILE::RED_RING ==
		m_ePresentationVortexProfile)
	{
		profile.renderMode = MAP_ASSET_RENDER_MODE::ADDITIVE;
	}
	else if (PRESENTATION_VORTEX_PROFILE::RED_CLOUD_DISC ==
		m_ePresentationVortexProfile)
	{
		/* The cloud discs must retain their dark burgundy body. Additive blending
		   would turn the source texture into the bright blue/red square that this
		   presentation profile is specifically meant to suppress. */
		profile.renderMode = MAP_ASSET_RENDER_MODE::TRANSLUCENT;
	}
	return profile;
}

void CMapAssetObject::Set_PlacementTransform(const float3_t& position,
	const float4_t& rotationQuaternion, const float3_t& signedScale)
{
	vector_t quaternion = XMQuaternionNormalize(XMLoadFloat4(&rotationQuaternion));

	if (XMVectorGetW(quaternion) < 0.f)
		quaternion = XMVectorNegate(quaternion);

	XMStoreFloat4(&m_vRotationQuaternion, quaternion);

	m_vPlacementPosition = position;
	m_vSignedScale = signedScale;
	m_bMirrored = signedScale.x * signedScale.y * signedScale.z < 0.f;

	const float3_t worldOrigin = Compute_WorldOrigin(
		position, m_vRotationQuaternion, signedScale);

	const matrix_t world = XMMatrixScaling(
		signedScale.x, signedScale.y, signedScale.z) *
		XMMatrixRotationQuaternion(quaternion);

	m_pTransformCom->Set_State(STATE::RIGHT, world.r[0]);
	m_pTransformCom->Set_State(STATE::UP, world.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, world.r[2]);
	m_pTransformCom->Set_State(
		STATE::POSITION, XMVectorSet(worldOrigin.x, worldOrigin.y, worldOrigin.z, 1.f));

	//bottom-center 보정을 포함한 최종 world 행렬을 사용해야 실제 렌더 위치와 bounds가 일치한다.
	Update_WorldCullBounds();
	m_FrustumState = {};
}

HRESULT CMapAssetObject::Ready_Components(
	uint32_t prototypeLevelIndex,
	const std::wstring& modelPrototypeTag)
{
	if (FAILED(__super::Add_Component(
		prototypeLevelIndex,
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		TEXT("Com_Shader"), m_pShaderCom)) ||
		FAILED(__super::Add_Component(
			prototypeLevelIndex, modelPrototypeTag,
			TEXT("Com_Model"), m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapAssetObject::Bind_ShaderResources(
	const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot)
{
	matrix_t world = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	world.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	const matrix_t inverseTranspose =
		XMMatrixTranspose(XMMatrixInverse(nullptr, world));
	float4x4_t storedInverseTranspose{};
	XMStoreFloat4x4(&storedInverseTranspose, inverseTranspose);
	if (FAILED(m_pTransformCom->Bind_ShaderResource(
		m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(m_pShaderCom->Bind_Matrix(
			"g_WorldInvTransposeMatrix", &storedInverseTranspose)) ||
		FAILED(nullptr != cameraSnapshot ?
			CMapAssetRenderUtils::Bind_CameraCullSnapshot(
				m_pShaderCom, *cameraSnapshot) :
			(FAILED(CGameInstance::Get().Bind_Transform(
				m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
			 FAILED(CGameInstance::Get().Bind_Transform(
				m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)) ?
			 E_FAIL : S_OK)) ||
		FAILED(Bind_PresentationVortexShaderResources(
			m_ePresentationVortexProfile,
			m_fPresentationVortexStrength)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapAssetObject::Bind_PresentationVortexShaderResources(
	const PRESENTATION_VORTEX_PROFILE profile,
	const f32_t strength)
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	const bool_t validProfile =
		PRESENTATION_VORTEX_PROFILE::NONE == profile ||
		PRESENTATION_VORTEX_PROFILE::DARK_APERTURE == profile ||
		PRESENTATION_VORTEX_PROFILE::RED_RING == profile ||
		PRESENTATION_VORTEX_PROFILE::RED_CLOUD_DISC == profile;
	const PRESENTATION_VORTEX_PROFILE boundedProfile =
		validProfile && std::isfinite(strength) && strength > 0.f ?
		profile : PRESENTATION_VORTEX_PROFILE::NONE;
	const f32_t boundedStrength =
		PRESENTATION_VORTEX_PROFILE::NONE == boundedProfile ? 0.f :
		(std::clamp)(strength, 0.f, 1.f);
	const uint32_t rawProfile =
		static_cast<uint32_t>(boundedProfile);

	/* Strength is written first. Even if the profile write fails, resetting
	   strength to zero disables both procedural shader branches. */
	const HRESULT strengthResult = m_pShaderCom->Bind_RawValue(
		"g_PresentationVortexStrength",
		&boundedStrength, sizeof(boundedStrength));
	const HRESULT profileResult = m_pShaderCom->Bind_RawValue(
		"g_PresentationVortexProfile",
		&rawProfile, sizeof(rawProfile));
	return FAILED(strengthResult) || FAILED(profileResult) ? E_FAIL : S_OK;
}

HRESULT CMapAssetObject::Reset_PresentationVortexShaderResources()
{
	return Bind_PresentationVortexShaderResources(
		PRESENTATION_VORTEX_PROFILE::NONE, 0.f);
}

HRESULT CMapAssetObject::Bind_WaterShaderResources(const bool_t bEnabled)
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	/* Disabled is the identity water: no fresnel, no distortion, no reflection
	   and a flat normal, so the water passes cannot tint or bend anything if
	   they are ever reached without a row. */
	const MAP_ASSET_WATER_PROFILE identity{};
	const MAP_ASSET_WATER_PROFILE& profile =
		bEnabled ? m_WaterProfile : identity;

	/* The auxiliary textures are declared by the water document but the
	   runtime does not own an SRV for them yet, so both flags stay zero and
	   the shader falls back to the model's own diffuse and normal. */
	constexpr uint32_t hasDetailNormalTexture = 0u;
	constexpr uint32_t hasReflectionTexture = 0u;
	const f32_t elapsedTime = bEnabled ? m_fElapsedTime : 0.f;

	struct WATER_SCALAR_BINDING final
	{
		const char_t* pName;
		f32_t value;
	};
	const WATER_SCALAR_BINDING scalars[]
	{
		{ "g_WaterOpacity", profile.opacity },
		{ "g_WaterOpacityPower", profile.opacityPower },
		{ "g_WaterFresnelIntensity", profile.fresnelIntensity },
		{ "g_WaterFresnelPower", profile.fresnelPower },
		{ "g_WaterScreenDistortionIntensity", profile.screenDistortionIntensity },
		{ "g_WaterNormalIntensity", profile.normalIntensity },
		{ "g_WaterDetailNormalIntensity", profile.detailNormalIntensity },
		{ "g_WaterReflectionIntensity", profile.reflectionIntensity },
		{ "g_WaterDiffuseTiling", profile.diffuseTiling },
		{ "g_ElapsedTime", elapsedTime },
	};
	struct WATER_VECTOR_BINDING final
	{
		const char_t* pName;
		const float4_t* pValue;
	};
	const WATER_VECTOR_BINDING vectors[]
	{
		{ "g_WaterDiffuseColor", &profile.diffuseColor },
		{ "g_WaterReflectionColor", &profile.reflectionColor },
		{ "g_WaterNormalTilingPanning", &profile.normalTilingPanning },
		{ "g_WaterDetailNormalTilingPanning",
			&profile.detailNormalTilingPanning },
		{ "g_WaterReflectionTilingPanning", &profile.reflectionTilingPanning },
	};

	HRESULT firstFailure = S_OK;
	const auto record = [&firstFailure](const HRESULT result)
	{
		if (FAILED(result) && SUCCEEDED(firstFailure))
			firstFailure = result;
	};

	for (const WATER_SCALAR_BINDING& binding : scalars)
	{
		record(m_pShaderCom->Bind_RawValue(
			binding.pName, &binding.value, sizeof(binding.value)));
	}
	for (const WATER_VECTOR_BINDING& binding : vectors)
	{
		record(m_pShaderCom->Bind_RawValue(
			binding.pName, binding.pValue, sizeof(*binding.pValue)));
	}
	record(m_pShaderCom->Bind_RawValue(
		"g_HasDetailNormalTexture",
		&hasDetailNormalTexture, sizeof(hasDetailNormalTexture)));
	record(m_pShaderCom->Bind_RawValue(
		"g_HasReflectionTexture",
		&hasReflectionTexture, sizeof(hasReflectionTexture)));
	if (bEnabled)
	{
		record(CGameInstance::Get().Bind_CamPosition(
			m_pShaderCom, "g_vCamPosition"));
	}
	return firstFailure;
}

HRESULT CMapAssetObject::Bind_ShadowShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(
		m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}
	return S_OK;
}

void CMapAssetObject::Ready_CullBounds()
{
	m_bHasLocalCullBounds = false;
	m_bHasWorldCullBounds = false;
	m_vLocalCullCenter = {};
	m_fLocalCullRadius = 0.f;
	m_vWorldCullCenter = {};
	m_fWorldCullRadius = 0.f;

	if (nullptr == m_pModelCom ||
		!m_pModelCom->Has_LocalBounds())
		return;

	const float3_t& minimum =
		m_pModelCom->Get_LocalBoundsMin();
	const float3_t& maximum =
		m_pModelCom->Get_LocalBoundsMax();
	//예외처리 <- 근데 무슨 경우?
	if (!std::isfinite(minimum.x) ||
		!std::isfinite(minimum.y) ||
		!std::isfinite(minimum.z) ||
		!std::isfinite(maximum.x) ||
		!std::isfinite(maximum.y) ||
		!std::isfinite(maximum.z) ||
		maximum.x < minimum.x ||
		maximum.y < minimum.y ||
		maximum.z < minimum.z)
	{
		return;
	}

	m_vLocalCullCenter = float3_t(
		(minimum.x + maximum.x) * 0.5f,
		(minimum.y + maximum.y) * 0.5f,
		(minimum.z + maximum.z) * 0.5f);

	const f32_t extentX =
		(maximum.x - minimum.x) * 0.5f;
	const f32_t extentY =
		(maximum.y - minimum.y) * 0.5f;
	const f32_t extentZ =
		(maximum.z - minimum.z) * 0.5f;

	const f32_t radius = std::sqrt(
		extentX * extentX +
		extentY * extentY +
		extentZ * extentZ);

	if (!std::isfinite(radius))
		return;

	//점이나 얇은 mesh가 쉽게 잘리지 않도록 최소 반지름
	m_fLocalCullRadius =
		radius > 0.05f ? radius : 0.05f;

	m_bHasLocalCullBounds = true;
}

void CMapAssetObject::Update_WorldCullBounds()
{
	m_bHasWorldCullBounds = false;

	if (!m_bHasLocalCullBounds ||
		nullptr == m_pTransformCom)
		return;

	const matrix_t world =
		XMLoadFloat4x4(
			m_pTransformCom->Get_WorldMatrixPtr());

	const vector_t worldCenter =
		XMVector3TransformCoord(
			XMLoadFloat3(&m_vLocalCullCenter),
			world);

	float3_t storedWorldCenter{};
	XMStoreFloat3(
		&storedWorldCenter,
		worldCenter);
	//vector의 길이 가지고 오기. row 0 1 2의 x y z의 길이 구하기
	const f32_t scaleX =
		XMVectorGetX(
			XMVector3Length(world.r[0]));
	const f32_t scaleY =
		XMVectorGetX(
			XMVector3Length(world.r[1]));
	const f32_t scaleZ =
		XMVectorGetX(
			XMVector3Length(world.r[2]));
	//가장 큰 스케일 기준으로 컬링
	f32_t maximumScale = scaleX;
	if (scaleY > maximumScale)
		maximumScale = scaleY;
	if (scaleZ > maximumScale)
		maximumScale = scaleZ;
	//worldRadius scale 보정
	const f32_t worldRadius =
		m_fLocalCullRadius *
		maximumScale *
		1.02f + 0.05f;
	
	if (!std::isfinite(storedWorldCenter.x) ||
		!std::isfinite(storedWorldCenter.y) ||
		!std::isfinite(storedWorldCenter.z) ||
		!std::isfinite(worldRadius) ||
		maximumScale < 0.000001f)
	{
		return;
	}

	m_vWorldCullCenter = storedWorldCenter;
	m_fWorldCullRadius = worldRadius;
	m_bHasWorldCullBounds = true;
}

float3_t CMapAssetObject::Compute_WorldOrigin(const float3_t& placementPosition,
	const float4_t& rotationQuaternion, const float3_t& signedScale) const
{
	float3_t worldOrigin = placementPosition;
	if (m_bApplyBottomCenter && m_pModelCom->Has_LocalBounds())
	{
		const float3_t& minimum = m_pModelCom->Get_LocalBoundsMin();
		const float3_t& maximum = m_pModelCom->Get_LocalBoundsMax();
		const vector_t localAnchor = XMVectorSet(
			(minimum.x + maximum.x) * 0.5f,
			minimum.y,
			(minimum.z + maximum.z) * 0.5f,
			1.f);
		const matrix_t transform = XMMatrixScaling(
			signedScale.x, signedScale.y, signedScale.z) *
			XMMatrixRotationQuaternion(
				XMQuaternionNormalize(XMLoadFloat4(&rotationQuaternion)));
		float3_t anchorOffset{};
		XMStoreFloat3(&anchorOffset,
			XMVector3TransformCoord(localAnchor, transform));
		worldOrigin.x -= anchorOffset.x;
		worldOrigin.y -= anchorOffset.y;
		worldOrigin.z -= anchorOffset.z;
	}

	return worldOrigin;
}

unique_ptr<CMapAssetObject> CMapAssetObject::Create(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CMapAssetObject>(
		new CMapAssetObject(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
		return nullptr;
	return pInstance;
}

shared_ptr<CPrototype> CMapAssetObject::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CMapAssetObject>(new CMapAssetObject(*this));
	if (FAILED(pInstance->Initialize(pArg)))
		return nullptr;
	return pInstance;
}
```

### Client/Public/KoukuSaydonPresentationPlayer.h

대상: `C:/Users/USER/source/졸업팀폴/LostArk/Client/Public/KoukuSaydonPresentationPlayer.h`

```cpp
#pragma once
#include "CardMazeVisualPolicy.h"

#include "Client_Defines.h"
#include "KoukuSaydonCompositionDocument.h"
#include "KoukuSaydonPreviewRootMotion.h"
#include "Network/PacketMessages.h"
#include "HitAreaWire.h"
#include <array>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace Engine { class CModel; }
namespace Client
{
class CNpc;
class CWorldSequencePlayer;
class CWorldSequenceDocument;
class CCharacter;
class CRenderingProfileService;
class CLightResourceCatalog;
class EFFECT_V2_CATALOG_SNAPSHOT;
class EFFECT_V2_PIVOT_HISTORY;
struct EFFECT_V2_TARGET;
struct EFFECT_V2_TARGET_VIEW;
struct ANIMATION_MODEL_TARGET_VIEW;
struct EFFECT_DOCUMENT_DESC;

struct KOUKU_BOSS_PRESENTATION_VIEW final
{
    std::weak_ptr<CNpc> pNpc;
    LostArk::Shared::WORLD_ENTITY_SNAPSHOT Snapshot;
    std::uint32_t iServerTick = 0;
    LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
    std::string strArchetypeId;
};
struct KOUKU_CARD_PRESENTATION_VIEW final
{
    std::weak_ptr<CCharacter> pCharacter;
    LostArk::Shared::PLAYER_SNAPSHOT Snapshot;
};
struct KOUKU_MAZE_TARGET_VIEW final
{
    std::weak_ptr<CNpc> npc;
    std::uint32_t entityId = 0u;
    std::string archetypeId;
};

// The Server supplies identity and time. This owner only samples presentation
// resources and releases its own effects, audio and temporary scene/camera state.
class CKoukuSaydonPresentationPlayer final
{
public:
    CKoukuSaydonPresentationPlayer(ComPtr<ID3D11Device> device,
        ComPtr<ID3D11DeviceContext> context, CRenderingProfileService& profiles);
    ~CKoukuSaydonPresentationPlayer();
    bool Reload_Product(std::string& status, std::uint32_t expectedSourceRevision = 0u);
    using WORLD_EMISSION_ANCHOR = std::function<bool_t(f32_t, float4x4_t&)>;
    static WORLD_EMISSION_ANCHOR Make_WorldEmissionAnchor(
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
        const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world,
        const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& occurrence);
    bool Resolve_ProductWorldEmissionAnchor(std::uint32_t sourceRevision, std::string_view patternId,
        std::string_view occurrenceId, WORLD_EMISSION_ANCHOR& out) const;
    void Set_LightResources(const CLightResourceCatalog* catalog) { m_pLightResources = catalog; }
    std::size_t Light_SkippedByBudget() const;
    void Update(float dt, const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
        const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players);
    bool Begin_Preview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, bool ownClock,
        std::uint32_t clockMs, bool paused, std::string& status);
    bool Begin_BundlePreview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const std::string& bundleId, std::uint32_t clockMs, bool paused, std::string& status,
        const CWorldSequenceDocument* sourceDocument = nullptr, bool automaticRootMotion = true,
        bool externalWorldPreview = false);
    // Optional Effect Workbench reference: existing actors and model sampler,
    // with all Pattern presentation/WORLD disabled and an external master clock.
    bool Begin_ModelReferencePreview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const std::string& selectionId, bool bundle, std::uint32_t clockMs, bool paused, std::string& status);
    void Sample_ModelReferencePreview(std::uint32_t clockMs, bool paused);
    bool Resolve_ModelReferenceTarget(const std::string& memberId,
        EFFECT_V2_TARGET& target, EFFECT_V2_TARGET_VIEW& view) const;
    // Source sockets use the same CNpc/CModel as the selected animation target.
    static bool Resolve_SourceAnchorWorlds(const EFFECT_DOCUMENT_DESC& document,
        const EFFECT_V2_TARGET_VIEW& view, const float4x4_t& root,
        std::unordered_map<std::string, float4x4_t>& anchors, std::string& error);
    static bool Sample_SourceAnchorWorlds(const EFFECT_DOCUMENT_DESC& document,
        const EFFECT_V2_TARGET_VIEW& view, const float4x4_t& root, float seconds,
        std::unordered_map<std::string, float4x4_t>& anchors, std::string& error);
    using V1_SOURCE_ANCHOR_SAMPLER = std::function<bool(float, const float4x4_t&,
        std::unordered_map<std::string, float4x4_t>&, std::string&)>;
    bool Preview_IsModelReference() const { return m_bModelReferencePreview; }
    std::uint64_t Preview_Generation() const { return m_iPreviewGeneration; }
    bool Preview_IsBundle() const { return !m_PreviewBundleId.empty(); }
    // MainApp resolves this before WORLD/model sampling, for either clock owner.
    bool Resolve_PreviewCaptureClock(std::uint32_t requestedMs, std::uint32_t& effectiveMs);
    void Sample_Preview(std::uint32_t clockMs, bool playing, bool paused,
        const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model);
    void Set_PreviewPivot(const float4x4_t& pivot,
        const std::shared_ptr<Engine::CModel>& model);
    void Pause_Preview(bool paused);
    void Seek_Preview(std::uint32_t clockMs);
    void Stop_Preview();
    void Reset();
    // Called after ImGui NewFrame; sampling never draws from a loader/update thread.
    void Render_Debug() const;
    void Refresh_ColliderAuthoring(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, std::uint64_t generation);
    // Only the selected Collider/Effect placement changes; clocks and unrelated cues remain live.
    bool Preview_PresentationGeometry(const std::string& patternId,
        const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence);
    bool Consume_CompletedPreview(std::string& patternId);
    // Terminal WORLD failure is separate from natural completion and consumed once.
    bool Consume_FailedPreview(std::string& patternId, std::string& status);
    bool Preview_OwnsClock() const { return m_bOwnPreviewClock; }
    bool Preview_IsColliderResource() const { return m_bColliderResourcePreview; }
    bool Preview_Playing() const { return m_bPreviewPlaying; }
    bool Preview_HasActiveWorldBox(std::string_view occurrenceId) const;
    bool Preview_Paused() const { return m_bPreviewPaused; }
    std::uint32_t Preview_ClockMs() const { return static_cast<std::uint32_t>(m_fPreviewClockMs); }
    std::uint32_t Preview_DurationMs() const { return m_iPreviewDurationMs; }
    const std::string& Preview_PatternId() const { return m_PreviewPattern.strPatternId; }
    const std::string& Status() const { return m_strStatus; }
private:
    struct PLAYING_ROW final
    {
        KOUKU_SAYDON_PRESENTATION_KIND kind = KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
        std::uint32_t effectHandle = 0;
        std::uint64_t v1EffectHandle = 0;
        std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> effectPivotHistory;
        V1_SOURCE_ANCHOR_SAMPLER sourceAnchorSampler;
        std::uint64_t soundHandle = 0;
        float lastAge = -1.f;
        float startMs = 0.f;
        std::uint32_t cameraDurationMs = 0u;
        float4x4_t pivot{};
        std::string assetId;
        float3_t cameraOffset{};
        HIT_AREA_SHAPE wire{};
        float4x4_t placementAnchor{};
        std::array<double, 3u> placementAnchorScale{1.0, 1.0, 1.0};
        bool hasPlacementAnchor = false;
        KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE lightBox;
        float lightWeight = 1.f;
        bool failed = false;
        std::string failureStatus;
        bool waitingForAnchor = false;
        bool debugRender = true;
    };
    struct EFFECT_ANCHOR_HISTORY final
    {
        std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> samples;
        float recordedSeconds = -1.f;
        float4x4_t recordedPivot{};
        bool missingSinceSample = false;
    };
    struct SESSION final
    {
        std::string key;
        float lastClockMs = -1.f;
        std::map<std::string, PLAYING_ROW> rows;
        std::uint32_t runEpoch = 0;
        std::string memberId;
        std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> rootHistory;
        float rootRecordedSeconds = -1.f;
        float4x4_t rootRecordedPivot{};
        // Pattern time, recorded before each following bone/WORLD cue starts.
        std::map<std::string, EFFECT_ANCHOR_HISTORY> effectAnchorHistories;
        std::map<std::string, std::shared_ptr<CWorldSequencePlayer>> previewWorlds;
    };
    struct PRODUCT_PATTERN final
    {
        KOUKU_SAYDON_COMPOSITION_DOCUMENT document;
        KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
        std::uint32_t durationMs = 0;
        std::map<std::string, WORLD_EMISSION_ANCHOR> worldEmissionAnchors;
    };
    struct PRODUCT_BUNDLE final
    {
        PRODUCT_PATTERN common;
        std::vector<std::string> patternIds;
    };
    struct BUNDLE_PREVIEW_MEMBER final
    {
        std::string memberId;
        std::uint32_t offsetTicks = 0, durationMs = 0;
        KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
        std::shared_ptr<CNpc> actor;
        SESSION session;
        std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE> animations;
        std::map<std::string, float3_t> worldOffsets;
        std::uint32_t initialAnimation = 0;
        float initialTicks = 0.f;
        float initialYawDegrees = 0.f;
        float3_t initialPosition{};
        std::unique_ptr<CKoukuSaydonPreviewRootMotion> rootMotion;
        std::map<std::string, float> stageFacingYawDegrees;
    };
    void Sample_BundlePreview();
    bool Prepare_PreviewEffects();
    void Fail_Preview(std::string status);
    bool Sample_BundlePreviewFacing(BUNDLE_PREVIEW_MEMBER& member, double localMs);
    void Refresh_WorldPlacementAuthoring(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document);
    void Release_BundlePreviewMembers(std::vector<BUNDLE_PREVIEW_MEMBER>& members);
    struct CARD final
    {
        std::string assetId;
        std::uint32_t handle = 0;
        // Used only by the eight cardmaze mark/exit groups.
        CARD_MAZE_MARK_RETRY mazeRetry;
    };
    void Sync_MazeMark(CARD& mark, const std::string& asset, const float4x4_t& pivot);
    void Update_FearPresentation(float dt, const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players);
    void Update_MazeMarks(const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players);
    void Sample(SESSION& session, const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, float clockMs, bool paused,
        const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model,
        const ANIMATION_MODEL_TARGET_VIEW* weaponView = nullptr);
    void Stop_Session(SESSION& session);
    bool Ensure_EffectResource(const std::string& kind, const std::string& asset,
        std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>& snapshot);
    void Restore_Scene();
    void Refresh_SharedPresentation();
    void Collect_FrameLights();
    struct FRAME_LIGHT_PROVIDER;
    std::shared_ptr<FRAME_LIGHT_PROVIDER> m_LightProvider;
    const CLightResourceCatalog* m_pLightResources = nullptr;
    std::vector<float4x4_t> m_LightPlayerPivots;
    struct LIGHT_BOSS_FOLLOWER final
    {
        std::uint32_t entityId = 0u;
        std::weak_ptr<CNpc> npc;
    };
    std::map<std::uint32_t, std::vector<LIGHT_BOSS_FOLLOWER>> m_LightBossFollowers;
    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_Context;
    CRenderingProfileService& m_Profiles;
    std::map<std::string, std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>> m_EffectResources;
    std::map<std::string, std::string> m_EffectResourceFailures;
    std::uint64_t m_iEffectCacheGeneration = 0u;
    std::set<std::string> m_QueuedV1Effects;
    std::uint64_t m_iV1CatalogRevision = 0u;
    std::map<std::string, PRODUCT_PATTERN> m_FearPresentations;
    SESSION m_FearSession;
    std::string m_strCompletedFearKey;
    std::map<std::string, PRODUCT_PATTERN> m_Product;
    std::map<std::string, PRODUCT_BUNDLE> m_ProductBundles;
    SESSION m_ProductBundleSession;
    std::uint32_t m_iProductSourceRevision = 0u;
    std::uint32_t m_iProductReloadRunEpoch = 0u;
    std::set<std::string> m_MissingProductPatterns;
    std::map<std::uint32_t, SESSION> m_BossSessions;
    std::map<std::uint32_t, SESSION> m_MarioEntrySessions;
    std::map<std::uint32_t, CARD> m_Cards;
    std::map<std::uint32_t, CARD> m_MazeExits;
    std::map<std::uint32_t, CARD> m_MazePlayerMarks;
    std::map<std::uint32_t, CARD> m_MazeTargetMarks;
    /* One floor decal per painted bingo cell, keyed by cell index. */
    std::map<std::int32_t, CARD> m_BingoMarks;
    /* Keyed by the Server's bomb slot, so a mark turning into a planted
    bomb replaces the same entry instead of leaving two on screen. */
    std::map<std::int32_t, CARD> m_BingoBombs;
    std::map<std::string, bool> m_ColliderDebugOverrides;
    std::uint64_t m_iColliderAuthoringGeneration = UINT64_MAX;
    bool m_bProductLoaded = false, m_bProductAttempted = false;
    std::string m_strScenePrevious, m_strSceneOwner, m_strSceneApplied;
    bool m_bSceneUsed = false, m_bCameraUsed = false;
    SESSION m_PreviewSession;
    std::string m_PreviewBundleId;
    std::vector<BUNDLE_PREVIEW_MEMBER> m_BundlePreviewMembers;
    KOUKU_SAYDON_COMPOSITION_DOCUMENT m_PreviewDocument;
    KOUKU_SAYDON_COMPOSITION_PATTERN m_PreviewPattern;
    float4x4_t m_PreviewPivot{};
    std::weak_ptr<Engine::CModel> m_PreviewModel;
    std::string m_strCompletedPreviewPatternId;
    std::string m_strFailedPreviewPatternId, m_strFailedPreviewStatus;
    bool m_bOwnPreviewClock = false, m_bPreviewPlaying = false, m_bPreviewPaused = false;
    bool m_bPreviewClockAwaitingFirstUpdate = false;
    bool m_bPreviewPreparationQueued = false;
    std::vector<std::string> m_PreviewPreparationTargets;
    bool m_bPreviewCaptureClockHeld = false;
    bool m_bPreviewCaptureBoundarySampled = false, m_bPreviewCaptureAllowed = true;
    std::uint32_t m_iPreviewCaptureResumeMs = 0u, m_iPreviewCaptureBoundaryMs = 0u;
    std::uint32_t m_iPreviewCaptureWaitFrames = 0u;
    bool m_bPreviewPivotReady = false;
    bool m_bColliderResourcePreview = false;
    bool m_bModelReferencePreview = false;
    bool m_bBundleWorldExternal = false;
    std::uint64_t m_iPreviewGeneration = 0u;
    double m_fPreviewClockMs = 0;
    std::uint32_t m_iPreviewDurationMs = 0;
    std::string m_strStatus;
};
}
```

### Client/Private/KoukuSaydonPresentationPlayer.cpp

대상: `C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp`

```cpp
#include <WinSock2.h>
#include "imgui.h"
#include "Engine_RenderTypes.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "KoukuSaydonAnimationBlend.h"

#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "DataJson.h"
#include "EffectV2_Catalog.h"
#include "Effect_PresentationService.h"
#include "Effect_Catalog.h"
#include "EffectCompositionModelPreview.h"
#include "Effect_Playback.h"
#include "NetworkManager.h"
#include "EffectV2_Object.h"
#include "EffectV2_Runtime.h"
#include "GameInstance.h"
#include "CombatHUDViewModel.h"
#include "Level_KakulSaydonArena.h"
#include "LightResourceCatalog.h"
#include "Presentation_Manager.h"
#include "Model.h"
#include "Npc.h"
#include "WorldSequencePlayer.h"
#include "ProjectDataRoot.h"
#include "RenderingProfileService.h"
#include "RuntimeAssetRoot.h"
#include "Transform.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iterator>
#include <optional>
#include <set>
#include <stdexcept>

namespace
{
using namespace Client;
using RESOURCE = KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE;
using OCCURRENCE = KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE;
using KIND = KOUKU_SAYDON_PRESENTATION_KIND;
constexpr std::uint32_t MAX_TIMELINE_MS = 600000u;

bool Validate_EffectAnchor(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const OCCURRENCE& box, std::string& status)
{
    const auto reject = [&](const char* reason) {
        status = std::string(reason) + ": " + box.strOccurrenceId;
        return false;
    };
    const auto stableId = [](const std::string& value) {
        return !value.empty() && value.size() <= 128u && value != "." && value != ".." &&
            std::all_of(value.begin(), value.end(), [](const unsigned char c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
            });
    };
    if ((box.strAnchorKind != "BOSS" && box.strAnchorKind != "WORLD" && box.strAnchorKind != "MAP") ||
        (box.strBoneTarget != "BODY" && box.strBoneTarget != "WEAPON") ||
        (!box.strBone.empty() && !stableId(box.strBone)) || box.iWorldEmissionIndex > 127u)
        return reject("Invalid Effect anchor, bone or World emission index");
    if (box.strAnchorKind == "MAP" && (box.bFollowBoss || !box.strBone.empty() ||
        box.strBoneTarget != "BODY" || !box.strWorldId.empty() || !box.strWorldOccurrenceId.empty()))
        return reject("MAP Effect requires a fixed position without a bone or World dependency");
    if (box.strAnchorKind == "WORLD")
    {
        if (box.strWorldId.empty() || std::none_of(document.Worlds.begin(), document.Worlds.end(),
            [&](const auto& world) { return world.strWorldId == box.strWorldId; }))
            return reject("Effect needs an existing World anchor");
    }
    else if (!box.strWorldId.empty())
        return reject("Only a WORLD Effect can name a World anchor");
    if (box.strBoneTarget == "WEAPON" && (box.strAnchorKind != "BOSS" || box.strBone.empty()))
        return reject("WEAPON Effect requires a boss anchor and an explicit weapon bone");
    if (!box.strWorldOccurrenceId.empty())
    {
        const auto owner = std::find_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
            [&](const auto& world) { return world.strOccurrenceId == box.strWorldOccurrenceId; });
        const auto world = owner == pattern.WorldOccurrences.end() ? document.Worlds.end() :
            std::find_if(document.Worlds.begin(), document.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == owner->strWorldId; });
        if (!stableId(box.strWorldOccurrenceId) || world == document.Worlds.end())
            return reject("Effect needs an existing World occurrence in the same Pattern");
        if (box.strAnchorKind == "WORLD")
        {
            if (box.strWorldId != owner->strWorldId)
                return reject("Effect World occurrence must match its World anchor");
        }
        else if (world->strCompanionEffectResourceId != box.strResourceId ||
            std::any_of(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
                [&](const auto& other) {
                    if (other.strOccurrenceId == box.strOccurrenceId || other.strAnchorKind == "WORLD" ||
                        other.strWorldOccurrenceId != box.strWorldOccurrenceId) return false;
                    return std::any_of(document.PresentationResources.begin(), document.PresentationResources.end(),
                        [&](const auto& resource) {
                            return resource.strResourceId == other.strResourceId && resource.eKind == KIND::EFFECT;
                        });
                }))
            return reject("Effect companion needs one matching World box/resource in the same Pattern");
    }
    return true;
}

std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> WorldPlacementFromOccurrence(
    const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& box)
{
    if (!box.Placement) return {};
    const auto& value = *box.Placement;
    return CWorldSequencePlayer::OBJECT_PLACEMENT{
        {float(value.Position[0]), float(value.Position[1]), float(value.Position[2])},
        {float(value.RotationDegrees[0]), float(value.RotationDegrees[1]), float(value.RotationDegrees[2])},
        {float(value.Scale[0]), float(value.Scale[1]), float(value.Scale[2])}};
}

std::uint32_t Camera_ReturnMs(const RESOURCE& resource, const bool authoring)
{
    if (resource.eKind != KIND::CAMERA) return 0u;
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level) return 0u;
    if (authoring) { std::string status; if (!level->Ensure_CameraShotAuthoring(status)) return 0u; }
    const auto& shots = authoring ? level->Get_CameraShots() : level->Get_PublishedCameraShots();
    const auto found = std::find_if(shots.begin(), shots.end(), [&](const auto& shot) { return shot.strShotId == resource.strAssetId; });
    return found == shots.end() ? 0u : found->iBlendOutMs;
}


const DATA_JSON_VALUE& Field(const DATA_JSON_VALUE& row, const char* key)
{
    const auto* value = row.Find(key);
    if (!value) throw std::runtime_error(std::string("Missing presentation field: ") + key);
    return *value;
}

std::string Text(const DATA_JSON_VALUE& row, const char* key, bool allowEmpty = false)
{
    const auto& value = Field(row, key);
    if (!value.Is_String() || value.Get_String().size() > 512u ||
        (!allowEmpty && value.Get_String().empty()) ||
        value.Get_String().find('\0') != std::string::npos)
        throw std::runtime_error(std::string("Invalid presentation string: ") + key);
    return value.Get_String();
}

double Number(const DATA_JSON_VALUE& row, const char* key, double low, double high)
{
    const auto& value = Field(row, key);
    if (!value.Is_Number() || !std::isfinite(value.Get_Number()) ||
        value.Get_Number() < low || value.Get_Number() > high)
        throw std::runtime_error(std::string("Invalid presentation number: ") + key);
    return value.Get_Number();
}

std::uint32_t UInt(const DATA_JSON_VALUE& row, const char* key,
    std::uint32_t low, std::uint32_t high)
{
    const double value = Number(row, key, low, high);
    if (std::floor(value) != value)
        throw std::runtime_error(std::string("Presentation integer required: ") + key);
    return static_cast<std::uint32_t>(value);
}

std::array<double, 3u> Vector(const DATA_JSON_VALUE& row, const char* key,
    double low, double high)
{
    const auto& value = Field(row, key);
    if (!value.Is_Array() || value.Get_Array().size() != 3u)
        throw std::runtime_error(std::string("Presentation vector required: ") + key);
    std::array<double, 3u> result{};
    for (size_t index = 0; index < result.size(); ++index)
    {
        const auto& part = value.Get_Array()[index];
        if (!part.Is_Number() || !std::isfinite(part.Get_Number()) ||
            part.Get_Number() < low || part.Get_Number() > high)
            throw std::runtime_error(std::string("Invalid presentation vector: ") + key);
        result[index] = part.Get_Number();
    }
    return result;
}

KIND Read_Kind(const std::string& kind)
{
    if (kind == "EFFECT") return KIND::EFFECT;
    if (kind == "SOUND") return KIND::SOUND;
    if (kind == "CAMERA") return KIND::CAMERA;
    if (kind == "COLLIDER") return KIND::COLLIDER;
    if (kind == "LIGHT") return KIND::LIGHT;
    if (kind == "SCENE_PROFILE") return KIND::SCENE_PROFILE;
    throw std::runtime_error("Unsupported presentation kind: " + kind);
}

RESOURCE Read_Resource(const DATA_JSON_VALUE& row)
{
    RESOURCE resource;
    resource.strResourceId = Text(row, "resourceId");
    resource.strDisplayName = resource.strResourceId;
    resource.eKind = Read_Kind(Text(row, "kind"));
    resource.strAssetId = Text(row, "assetId", resource.eKind == KIND::COLLIDER);
    resource.strResourceKind = Text(row, "resourceKind", resource.eKind != KIND::EFFECT);
    if (row.Find("elementId")) resource.strElementId = Text(row, "elementId", true);
    resource.iDurationMs = UInt(row, "resourceDurationMs", 1u, MAX_TIMELINE_MS);
    resource.strShape = Text(row, "shape");
    resource.HalfExtents = Vector(row, "halfExtents", 0.001, 100000.0);
    resource.fRadiusM = Number(row, "radiusM", 0.001, 100000.0);
    resource.fHalfAngleDegrees = Number(row, "halfAngleDegrees", resource.strShape == "REVERSE_SECTOR" ? 0.0 : 0.001, 180.0);
    const bool v1 = resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT";
    if ((resource.eKind == KIND::EFFECT && resource.strResourceKind != "GROUP" && resource.strResourceKind != "LEAF" && !v1) ||
        (resource.strShape != "BOX" && resource.strShape != "SECTOR" && resource.strShape != "REVERSE_SECTOR" && resource.strShape != "CIRCLE"))
        throw std::runtime_error("Invalid presentation resource type: " + resource.strResourceId);
    if (resource.eKind == KIND::EFFECT &&
        !CEffectV2Document::Is_ValidEffectId(resource.strAssetId))
        throw std::runtime_error("Invalid Effect identity: " + resource.strAssetId);
    const auto stableLightId = [](const std::string& id)
    {
        return !id.empty() && id.size() <= 128u && id != "." && id != ".." &&
            std::all_of(id.begin(), id.end(), [](unsigned char c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'; });
    };
    if ((resource.strResourceKind == "V1_ELEMENT" && !stableLightId(resource.strElementId)) ||
        (resource.strResourceKind != "V1_ELEMENT" && !resource.strElementId.empty()))
        throw std::runtime_error("Invalid Effect element identity: " + resource.strElementId);
    if (resource.eKind == KIND::LIGHT && (!resource.strResourceKind.empty() ||
        !stableLightId(resource.strAssetId)))
        throw std::runtime_error("Invalid Light resource identity: " + resource.strAssetId);
    if (resource.eKind == KIND::SOUND &&
        (resource.strAssetId.rfind("Sound/", 0u) != 0u ||
         CRuntimeAssetRoot::Resolve(resource.strAssetId).empty()))
        throw std::runtime_error("Invalid Sound asset ID: " + resource.strAssetId);
    if (resource.eKind == KIND::COLLIDER && !resource.strAssetId.empty())
        throw std::runtime_error("Collider presentation cannot name an asset.");
    return resource;
}

OCCURRENCE Read_Occurrence(const DATA_JSON_VALUE& row, std::uint32_t durationMs, KIND kind)
{
    OCCURRENCE box;
    box.strOccurrenceId = Text(row, "occurrenceId");
    box.strResourceId = Text(row, "resourceId");
    box.iStartMs = UInt(row, "startMs", 0u, durationMs);
    box.iDurationMs = UInt(row, "durationMs", 1u, durationMs);
    if (box.iDurationMs > durationMs - box.iStartMs)
        throw std::runtime_error("Presentation occurrence exceeds its pattern.");
    box.PositionOffset = Vector(row, "positionOffset", -100000.0, 100000.0);
    box.RotationDegrees = Vector(row, "rotationDegrees", -100000.0, 100000.0);
    box.Scale = Vector(row, "scale", 0.001, 100000.0);
    // SCENE_PROFILE projects its reserved blendMs metadata into fadeInMs.
    // Its authoring range is the whole timeline, independent of this box's
    // lifetime; applying Effect envelope bounds here rejects every Product.
    box.iFadeInMs = UInt(row, "fadeInMs", 0u,
        kind == KIND::SCENE_PROFILE ? MAX_TIMELINE_MS : box.iDurationMs);
    box.iFadeOutMs = UInt(row, "fadeOutMs", 0u, box.iDurationMs);
    if (kind != KIND::SCENE_PROFILE &&
        box.iFadeInMs + box.iFadeOutMs > box.iDurationMs)
        throw std::runtime_error("Presentation fades exceed the occurrence.");
    box.fDissolveStart = Number(row, "dissolveStart", 0.0, 1.0);
    box.fDissolveEnd = Number(row, "dissolveEnd", 0.0, 1.0);
    if (box.fDissolveStart > box.fDissolveEnd)
        throw std::runtime_error("Presentation dissolve interval is reversed.");
    box.fVolume = Number(row, "volume", 0.0, 1.0);
    if (row.Find("brightnessMultiplier")) box.fBrightnessMultiplier = Number(row, "brightnessMultiplier", 0.0, 16.0);
    const auto& follow = Field(row, "followBoss");
    if (!follow.Is_Boolean()) throw std::runtime_error("followBoss must be Boolean.");
    box.bFollowBoss = follow.Get_Boolean();
    if (const auto* debug = row.Find("debugRender"))
    {
        if (!debug->Is_Boolean()) throw std::runtime_error("debugRender must be Boolean.");
        box.bDebugRender = debug->Get_Boolean();
    }
    box.strBone = Text(row, "bone", true);
    if (row.Find("boneTarget")) box.strBoneTarget = Text(row, "boneTarget");
    if (box.strBoneTarget != "BODY" && box.strBoneTarget != "WEAPON")
        throw std::runtime_error("Presentation boneTarget is unsupported.");
    if (box.strBone.size() > 128u) throw std::runtime_error("Presentation bone is too long.");
    if (row.Find("anchorKind")) box.strAnchorKind = Text(row, "anchorKind");
    if (row.Find("worldId")) box.strWorldId = Text(row, "worldId", true);
    if (row.Find("worldOccurrenceId")) box.strWorldOccurrenceId = Text(row, "worldOccurrenceId", true);
    if (row.Find("worldEmissionIndex")) box.iWorldEmissionIndex = UInt(row, "worldEmissionIndex", 0u, 127u);
    if (box.strAnchorKind != "BOSS" && box.strAnchorKind != "WORLD" &&
        !(kind == KIND::LIGHT && (box.strAnchorKind == "MAP" || box.strAnchorKind == "PLAYER")) &&
        !(kind == KIND::EFFECT && box.strAnchorKind == "MAP"))
        throw std::runtime_error("Presentation anchorKind is unsupported.");
    if (kind == KIND::LIGHT && ((box.strAnchorKind != "WORLD" && !box.strWorldId.empty()) ||
        box.Scale != std::array<double, 3u>{1.0, 1.0, 1.0} ||
        (box.strAnchorKind != "BOSS" && !box.strBone.empty()) ||
        (box.strAnchorKind == "PLAYER" && !box.bFollowBoss)))
        throw std::runtime_error("Invalid Light anchor, scale or bone.");
    if (kind == KIND::EFFECT && box.strAnchorKind == "MAP" &&
        (box.bFollowBoss || !box.strBone.empty() || box.strBoneTarget != "BODY" ||
            !box.strWorldId.empty() || !box.strWorldOccurrenceId.empty()))
        throw std::runtime_error("MAP Effect requires a fixed position without a bone or World occurrence.");
    if (box.strAnchorKind == "WORLD" && box.strWorldId.empty())
        throw std::runtime_error("WORLD presentation anchor needs a worldId.");
    if (box.strBoneTarget == "WEAPON" && ((kind != KIND::COLLIDER && kind != KIND::EFFECT) || box.strAnchorKind != "BOSS" || box.strBone.empty()))
        throw std::runtime_error("WEAPON bone target needs a Boss Collider/Effect and a named weapon bone.");
    return box;
}

struct PRESENTATION_WINDOW final
{
    KIND kind;
    std::int64_t startSubtick = 0, endSubtick = 0;
    std::string owner;
};
bool Admit_PresentationWindow(std::vector<PRESENTATION_WINDOW>& windows,
    KIND kind, double startMs, double endMs, const std::string& owner)
{
    const auto start = static_cast<std::int64_t>(std::llround(startMs * 30.0));
    const auto end = static_cast<std::int64_t>(std::llround(endMs * 30.0));
    for (const auto& window : windows)
        if (window.kind == kind && window.owner != owner &&
            start < window.endSubtick && window.startSubtick < end) return false;
    windows.push_back({ kind, start, end, owner });
    return true;
}

std::uint32_t Pattern_Duration(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
{
    std::uint64_t duration = 0;
    for (const auto& stage : pattern.Stages) duration += stage.iDurationMs;
    for (const auto& row : pattern.PresentationOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    for (const auto& row : pattern.SceneProfileOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    for (const auto& row : pattern.WorldOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    return static_cast<std::uint32_t>((std::min)(duration, std::uint64_t(MAX_TIMELINE_MS)));
}

bool Make_Pivot(const OCCURRENCE& box, const float4x4_t& root,
    const std::shared_ptr<Engine::CModel>& model, float4x4_t& result,
    const ANIMATION_MODEL_TARGET_VIEW* weaponView = nullptr, float4x4_t* sampledBasis = nullptr)
{
    float4x4_t anchor = root;
    if (!box.strBone.empty())
    {
        EFFECT_V2_TARGET_VIEW view;
        if (box.strBoneTarget == "WEAPON")
        {
            if (!weaponView) return false;
            view.pModel = weaponView->Model;
            view.BoneRoot = weaponView->BoneRoot;
            view.YawBasis = weaponView->TargetRoot;
        }
        else if (box.strBoneTarget == "BODY")
        {
            view.pModel = model;
            view.BoneRoot = root;
            view.YawBasis = root;
        }
        else return false;
        if (!view.pModel || !view.pModel->Has_Bone(box.strBone.c_str())) return false;
        if (!CEffectV2Object::Resolve_TargetPivot(view, box.strBone,
            CEffectV2Object::PIVOT_ROTATION::TARGET_YAW, anchor)) return false;
    }
    matrix_t basis = XMLoadFloat4x4(&anchor);
    for (size_t axis = 0; axis < 3u; ++axis)
    {
        if (XMVectorGetX(XMVector3LengthSq(basis.r[axis])) < 0.000001f) return false;
        basis.r[axis] = XMVector3Normalize(basis.r[axis]);
    }
    if (sampledBasis) XMStoreFloat4x4(sampledBasis, basis);
    XMStoreFloat4x4(&result,
        XMMatrixScaling(float(box.Scale[0]), float(box.Scale[1]), float(box.Scale[2])) *
        XMMatrixRotationRollPitchYaw(XMConvertToRadians(float(box.RotationDegrees[0])),
            XMConvertToRadians(float(box.RotationDegrees[1])), XMConvertToRadians(float(box.RotationDegrees[2]))) *
        XMMatrixTranslation(float(box.PositionOffset[0]), float(box.PositionOffset[1]), float(box.PositionOffset[2])) * basis);
    return true;
}

// Recorded resolved anchors contain WORLD scale but no editable occurrence geometry.
bool Make_ResolvedEffectPivot(const OCCURRENCE& box, const float4x4_t& anchor,
    float4x4_t& output)
{
    auto placed = box;
    placed.strBone.clear();
    const matrix_t basis = XMLoadFloat4x4(&anchor);
    for (size_t axis = 0u; axis < 3u; ++axis)
    {
        const float scale = XMVectorGetX(XMVector3Length(basis.r[axis]));
        placed.PositionOffset[axis] *= scale;
        placed.Scale[axis] *= scale;
    }
    return Make_Pivot(placed, anchor, {}, output);
}

CEffectV2Object::PIVOT_SAMPLER Effect_PivotSampler(const OCCURRENCE& box,
    const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& rootHistory,
    const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& anchorHistory)
{
    if (box.strAnchorKind == "MAP" || !box.bFollowBoss) return {};
    const bool resolved = box.strAnchorKind == "WORLD" || !box.strBone.empty();
    return [box, history = resolved ? anchorHistory : rootHistory, resolved]
        (float seconds, float4x4_t& output, std::string& error)
    {
        if (!history) { error = "Effect anchor history is unavailable."; return false; }
        float4x4_t recorded;
        if (!history->Sample(box.iStartMs / 1000.f + seconds,
            recorded, error)) return false;
        if (resolved ? Make_ResolvedEffectPivot(box, recorded, output) :
            Make_Pivot(box, recorded, {}, output)) return true;
        error = "Recorded Effect anchor cannot form its authored pivot.";
        return false;
    };
}

using SOURCE_ATTACHMENTS = std::vector<EFFECT_ACTION_CUE_ATTACHMENT_DESC>;
using SOURCE_BONES = std::unordered_map<std::string, float4x4_t>;
using ANCHOR_ANIMATION = KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE;

SOURCE_ATTACHMENTS Source_Attachments(const EFFECT_DOCUMENT_DESC& document, const std::string& elementId = {})
{
    SOURCE_ATTACHMENTS result;
    for (const auto& element : document.Elements)
    {
        const auto& attachment = element.ActionCueAttachment;
        if (element.bVisible && (elementId.empty() || element.strElementId == elementId) &&
            attachment.bEnabled && attachment.bFollow && attachment.strModelCueId.empty())
            result.push_back(attachment);
    }
    return result;
}

bool Valid_SourceMatrix(const matrix_t& value)
{
    const float determinant = XMVectorGetX(XMMatrixDeterminant(value));
    return !XMMatrixIsNaN(value) && !XMMatrixIsInfinite(value) &&
        std::isfinite(determinant) && std::abs(determinant) > 1.e-12f;
}

bool Build_SourceAnchorWorlds(const SOURCE_ATTACHMENTS& attachments,
    const EFFECT_V2_TARGET_VIEW& view, const float4x4_t& root, const SOURCE_BONES& bones,
    SOURCE_BONES& anchors, std::string& error)
{
    SOURCE_BONES staged;
    float4x4_t ownerPivot;
    const bool needsBones = std::any_of(attachments.begin(), attachments.end(), [](const auto& value)
        { return value.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW; });
    matrix_t delta = XMMatrixIdentity();
    if (needsBones)
    {
        if (!CEffectV2Object::Resolve_TargetPivot(view, "", CEffectV2Object::PIVOT_ROTATION::TARGET_YAW, ownerPivot) ||
            !Valid_SourceMatrix(XMLoadFloat4x4(&ownerPivot)) || !Valid_SourceMatrix(XMLoadFloat4x4(&root)))
        { error = "Kouku source attachment owner root is unavailable or singular."; return false; }
        delta = XMMatrixInverse(nullptr, XMLoadFloat4x4(&ownerPivot)) * XMLoadFloat4x4(&root);
    }
    for (const auto& attachment : attachments)
    {
        if (attachment.strRuntimeAnchorSlotId.empty())
        { error = "Kouku source attachment has no runtime slot."; return false; }
        matrix_t anchor;
        if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW)
        {
            const auto* camera = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
            if (!camera) { error = "Kouku source camera anchor is unavailable."; return false; }
            anchor = XMLoadFloat4x4(camera);
        }
        else
        {
            const auto bone = bones.find(attachment.strRuntimeBoneName);
            if (bone == bones.end())
            { error = "Kouku source bone is unavailable: " + attachment.strRuntimeBoneName; return false; }
            matrix_t raw = XMLoadFloat4x4(&bone->second);
            // Both Kouku/Saydon cooked rigs already carry a 100x root basis.
            // CModel applies the actor pre-scale (G1 0.017 -> 1.7, G2 Kouku
            // 0.012053 -> 1.2053). Metre-based offsets consume that basis
            // directly; preserve animated scale/translation without another x100.
            if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::BONE)
                anchor = raw * XMLoadFloat4x4(&view.BoneRoot);
            else if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW)
            {
                float4x4_t normalizedBone, yawAnchor;
                XMStoreFloat4x4(&normalizedBone, raw);
                if (!CEffectPlayback::Build_OwnerYawBoneAnchorWorld(normalizedBone, view.BoneRoot, view.YawBasis, yawAnchor))
                { error = "Kouku source owner-yaw anchor is invalid."; return false; }
                anchor = XMLoadFloat4x4(&yawAnchor);
            }
            else { error = "Unsupported Kouku source attachment orientation."; return false; }
        }
        const auto& local = attachment.SocketLocalTransform;
        matrix_t world = XMMatrixScaling(local.vScale.x, local.vScale.y, local.vScale.z) *
            XMMatrixRotationRollPitchYaw(XMConvertToRadians(local.vRotationDegrees.x),
                XMConvertToRadians(local.vRotationDegrees.y), XMConvertToRadians(local.vRotationDegrees.z)) *
            XMMatrixTranslation(local.vPosition.x, local.vPosition.y, local.vPosition.z) * anchor;
        if (attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW) world *= delta;
        if (!Valid_SourceMatrix(world))
        { error = "Kouku source attachment matrix is invalid: " + attachment.strRuntimeAnchorSlotId; return false; }
        float4x4_t value;
        XMStoreFloat4x4(&value, world);
        const auto [found, inserted] = staged.emplace(attachment.strRuntimeAnchorSlotId, value);
        if (!inserted)
            for (size_t r = 0u; r < 4u; ++r) for (size_t c = 0u; c < 4u; ++c)
                if (std::abs(found->second.m[r][c] - value.m[r][c]) > 0.0001f)
                { error = "Conflicting Kouku source attachment slot: " + attachment.strRuntimeAnchorSlotId; return false; }
    }
    anchors = std::move(staged);
    error.clear();
    return true;
}

bool Sample_SourceBones(const std::shared_ptr<CModel>& model,
    const std::vector<ANCHOR_ANIMATION>& animations,
    std::span<const KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> blendWindows, float sampleMs,
    const std::vector<std::string>& names, SOURCE_BONES& bones, std::string& error)
{
    if (names.empty()) return true;
    if (!model) { error = "Kouku source animation model was released."; return false; }
    const ANCHOR_ANIMATION* animation = nullptr;
    const ANCHOR_ANIMATION* previous = nullptr;
    for (const auto& value : animations)
        if (sampleMs >= (value.iPoseStartMs == UINT32_MAX ? value.iStartOffsetMs : value.iPoseStartMs))
        { previous = animation; animation = &value; }
    // A leading Effect samples the scheduled clip's first pose before playback starts.
    if (!animation && !animations.empty()) animation = &animations.front();
    if (!animation) { error = "Kouku source attachment has no animation at its requested time."; return false; }
    const auto clipIndex = [&](const std::string& name) {
        uint32_t found = UINT32_MAX;
        for (uint32_t index = 0u; index < model->Get_NumAnimations(); ++index)
            if (const auto* value = model->Get_AnimationName(index); value && name == value)
            { if (found != UINT32_MAX) return UINT32_MAX; found = index; }
        return found;
    };
    const uint32_t index = clipIndex(animation->strRuntimeClip);
    float cursor = 0.f, duration = 0.f;
    if (index == UINT32_MAX || !model->Get_AnimationProgress(index, cursor, duration))
    { error = "Kouku source animation clip is unavailable: " + animation->strRuntimeClip; return false; }
    const float age = (std::max)(0.f, sampleMs - animation->iStartOffsetMs);
    const bool loop = animation->strEndPolicy == "LOOP_TO_WINDOW";
    const float elapsed = (std::min)(age, float(animation->iPlayMs));
    const float tps = model->Get_AnimationTickPerSecond(index);
    double sourceMs = 0.0;
    if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(animation->iSourceStartMs,
        animation->iSourceEndMs, elapsed, animation->fPlayRate, duration * 1000.0 / tps, loop, sourceMs))
    { error = "Kouku source animation range is outside the native clip."; return false; }
    const float ticks = float(sourceMs * tps / 1000.0);
    std::vector<uint32_t> indices;
    for (const auto& name : names)
    {
        const auto bone = model->Find_BoneIndex(name.c_str());
        if (bone < 0) { error = "Kouku source bone is unavailable: " + name; return false; }
        indices.push_back(static_cast<uint32_t>(bone));
    }
    std::vector<float4x4_t> sampled(indices.size());
    bool sampledPose = false;
    CModel::ANIMATION_TRANSITION_POSE logicPose;
    bool logicActive = false;
    if (!CKoukuSaydonAnimationBlend::Sample_Pose(*model, blendWindows, sampleMs, logicPose, logicActive, error)) return false;
    if (logicActive) sampledPose = model->Sample_AnimationTransitionBoneCombinedMatrices(logicPose, indices, sampled);
    else if (previous && animation->iBlendInMs && age < animation->iBlendInMs)
    {
        CModel::ANIMATION_TRANSITION_POSE pose;
        pose.sourceIndex = clipIndex(previous->strRuntimeClip);
        pose.targetIndex = index; pose.targetTicks = ticks;
        pose.durationSeconds = animation->iBlendInMs * .001f;
        pose.elapsedSeconds = age * .001f; pose.playRate = animation->fPlayRate;
        float previousDuration = 0.f;
        if (pose.sourceIndex == UINT32_MAX || !model->Get_AnimationProgress(pose.sourceIndex, cursor, previousDuration))
        { error = "Kouku source animation blend clip is unavailable."; return false; }
        const float previousTps = model->Get_AnimationTickPerSecond(pose.sourceIndex);
        double previousMs = 0.0;
        if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(previous->iSourceStartMs,
            previous->iSourceEndMs, previous->iPlayMs, previous->fPlayRate,
            previousDuration * 1000.0 / previousTps, previous->strEndPolicy == "LOOP_TO_WINDOW", previousMs))
        { error = "Kouku source animation blend range is outside the native clip."; return false; }
        pose.sourceTicks = float(previousMs * previousTps / 1000.0);
        sampledPose = model->Sample_AnimationTransitionBoneCombinedMatrices(pose, indices, sampled);
    }
    else sampledPose = model->Sample_AnimationBoneCombinedMatrices(animation->strRuntimeClip.c_str(), ticks, indices, sampled);
    if (!sampledPose) { error = "Kouku source animation pose sample failed."; return false; }
    for (size_t i = 0u; i < names.size(); ++i) bones.emplace(names[i], sampled[i]);
    return true;
}

CKoukuSaydonPresentationPlayer::V1_SOURCE_ANCHOR_SAMPLER Make_SourceAnchorSampler(
    const RESOURCE& resource, const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
    const std::shared_ptr<CModel>& model, const float4x4_t& ownerRoot, uint32_t startMs)
{
    const auto document = CEffectCatalog::Find_Loaded(resource.strAssetId);
    if (!document) return [](float, const float4x4_t&, SOURCE_BONES&, std::string& error)
        { error = "Prepared Kouku source Effect document is unavailable."; return false; };
    auto attachments = Source_Attachments(*document, resource.strElementId);
    if (attachments.empty()) return {};
    std::vector<ANCHOR_ANIMATION> animations;
    uint32_t stageStart = 0u;
    for (const auto& stage : pattern.Stages)
    {
        for (auto animation : stage.AnimationOccurrences)
        {
            if (animation.iPoseStartMs == UINT32_MAX)
                animation.iPoseStartMs = stageStart + (animation.strOccurrenceId == std::min_element(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
                [](const auto& a, const auto& b) { return a.iStartOffsetMs != b.iStartOffsetMs ?
                    a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; })->strOccurrenceId ? 0u : animation.iStartOffsetMs);
            animation.iStartOffsetMs += stageStart; animations.push_back(std::move(animation));
        }
        stageStart += stage.iDurationMs;
    }
    std::stable_sort(animations.begin(), animations.end(), [](const auto& a, const auto& b)
        { return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; });
    std::vector<std::string> names;
    for (const auto& attachment : attachments)
        if (attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
            std::find(names.begin(), names.end(), attachment.strRuntimeBoneName) == names.end())
            names.push_back(attachment.strRuntimeBoneName);
    // A resource with no saved model animation previews the selected pose.
    // Product/Pattern timelines still require their authored animation history.
    SOURCE_BONES frozenBones;
    std::string frozenError;
    const bool freezeResourcePose = animations.empty() && (pattern.strPatternId == "preview.kouku.resource" ||
        pattern.strPatternId == "preview.kouku.resource.actor");
    if (freezeResourcePose)
        for (const auto& name : names)
        {
            if (!model || !model->Has_Bone(name.c_str()))
            { frozenError = "Selected Resource Preview model has no source bone: " + name; break; }
            float4x4_t value;
            XMStoreFloat4x4(&value, model->Get_BoneMatrix(name.c_str()));
            frozenBones.emplace(name, value);
        }
    return [attachments = std::move(attachments), animations = std::move(animations), names = std::move(names),
        frozenBones = std::move(frozenBones), frozenError = std::move(frozenError), freezeResourcePose,
        blendWindows = pattern.AnimationBlendWindows, weakModel = std::weak_ptr<CModel>(model), ownerRoot, startMs]
        (float seconds, const float4x4_t& root, SOURCE_BONES& anchors, std::string& error)
    {
        if (!std::isfinite(seconds) || seconds < 0.f) { error = "Invalid Kouku source anchor sample time."; return false; }
        if (std::any_of(attachments.begin(), attachments.end(), [](const auto& value)
            { return value.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW; }))
        { error = "Kouku Product camera-view source attachment requires recorded camera history."; return false; }
        EFFECT_V2_TARGET_VIEW view;
        view.pModel = weakModel.lock(); view.BoneRoot = ownerRoot; view.YawBasis = ownerRoot;
        if (freezeResourcePose)
        {
            if (!frozenError.empty()) { error = frozenError; return false; }
            return Build_SourceAnchorWorlds(attachments, view, root, frozenBones, anchors, error);
        }
        SOURCE_BONES bones;
        return Sample_SourceBones(view.pModel, animations, blendWindows, startMs + seconds * 1000.f, names, bones, error) &&
            Build_SourceAnchorWorlds(attachments, view, root, bones, anchors, error);
    };
}

EFFECT_FIXED_STEP_TRANSFORM_PROVIDER Effect_V1TransformProvider(const OCCURRENCE& box,
    const float4x4_t& frozenPivot, const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& rootHistory,
    const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& anchorHistory,
    CKoukuSaydonPresentationPlayer::V1_SOURCE_ANCHOR_SAMPLER sourceAnchors)
{
    return [sampler = Effect_PivotSampler(box, rootHistory, anchorHistory), frozenPivot, sourceAnchors = std::move(sourceAnchors)]
        (float seconds, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& output, std::string& error)
    {
        output.RootWorld = frozenPivot;
        output.SourceAnchorWorlds.clear();
        if (sampler && !sampler(seconds, output.RootWorld, error)) return false;
        if (sourceAnchors && !sourceAnchors(seconds, output.RootWorld, output.SourceAnchorWorlds, error)) return false;
        error.clear();
        return true;
    };
}

// A centered WORLD circle is a ground-plane radius proxy. Uniform model
// scale changes its radius; mesh roll/pitch and self-spin do not tilt the proxy.
bool Is_CenteredWorldCircle(const RESOURCE& resource, const OCCURRENCE& box)
{
    return resource.eKind == KIND::COLLIDER && resource.strShape == "CIRCLE" &&
        box.strAnchorKind == "WORLD" && box.strBone.empty() &&
        std::all_of(box.PositionOffset.begin(), box.PositionOffset.end(), [](const double v) { return v == 0.0; }) &&
        std::abs(box.Scale[0] - box.Scale[1]) <= .0001 && std::abs(box.Scale[0] - box.Scale[2]) <= .0001;
}

void Flatten_CenteredWorldCircle(const RESOURCE& resource, const OCCURRENCE& box, float4x4_t& anchor)
{
    if (!Is_CenteredWorldCircle(resource, box)) return;
    const matrix_t world = XMLoadFloat4x4(&anchor);
    const float sx = XMVectorGetX(XMVector3Length(world.r[0]));
    const float sy = XMVectorGetX(XMVector3Length(world.r[1]));
    const float sz = XMVectorGetX(XMVector3Length(world.r[2]));
    // Keep legacy nonuniform upright colliders on their existing path. The
    // gameplay publisher rejects nonuniform spinning circle sources.
    if (!std::isfinite(sx) || sx <= 0.f || std::abs(sx - sy) > .0001f || std::abs(sx - sz) > .0001f) return;
    matrix_t upright = XMMatrixIdentity();
    upright.r[3] = world.r[3];
    XMStoreFloat4x4(&anchor, upright);
}

HIT_AREA_SHAPE Collider_Wire(const RESOURCE& resource, const OCCURRENCE& box)
{
    HIT_AREA_SHAPE shape;
    if (resource.strShape == "BOX")
    {
        const double halfWidth = resource.HalfExtents[0] * box.Scale[0];
        const double halfLength = resource.HalfExtents[2] * box.Scale[2];
        shape.fBoxHalfHeightM = static_cast<float>(resource.HalfExtents[1] * box.Scale[1]);
        shape.iAreaType = 2;
        shape.iAreaRange = static_cast<int32_t>((std::min)(halfLength * 200.0, 1000000000.0));
        shape.iAreaAngle = static_cast<int32_t>((std::min)(halfWidth * 200.0, 1000000000.0));
        shape.iAreaOffsetX = -shape.iAreaRange / 2;
    }
    else
    {
        shape.iAreaType = resource.strShape == "CIRCLE" ? 1 : 3;
        shape.iAreaRange = static_cast<int32_t>((std::min)(
            resource.fRadiusM * (std::max)(box.Scale[0], box.Scale[2]) * 100.0, 1000000000.0));
        shape.iAreaAngle = static_cast<int32_t>(resource.fHalfAngleDegrees * 2.0);
        if (shape.iAreaType == 3)
        {
            shape.fSectorRadiusXM = static_cast<float>(resource.fRadiusM * box.Scale[0]);
            shape.fSectorRadiusZM = static_cast<float>(resource.fRadiusM * box.Scale[2]);
            shape.fSectorAngleDegrees = static_cast<float>(resource.fHalfAngleDegrees * 2.0);
            shape.bReverseSector = resource.strShape == "REVERSE_SECTOR";
        }
    }
    return shape;
}

#ifdef _DEBUG
void Draw_LightWire(const LIGHT_DESC& light)
{
    if (light.eType == LIGHT::DIRECTIONAL) return;
    auto& game = CGameInstance::Get();
    const matrix_t view = XMLoadFloat4x4(game.Get_Transform(D3DTS::VIEW));
    const matrix_t projection = XMLoadFloat4x4(game.Get_Transform(D3DTS::PROJ));
    auto* viewport = ImGui::GetMainViewport();
    auto* draw = ImGui::GetBackgroundDrawList(viewport);
    const auto project = [&](fvector_t world, ImVec2& out)
    {
        const vector_t v = XMVector3TransformCoord(world, view);
        if (XMVectorGetZ(v) <= .1f) return false;
        const vector_t p = XMVector3TransformCoord(v, projection);
        out = {viewport->Pos.x + (XMVectorGetX(p) * .5f + .5f) * viewport->Size.x,
            viewport->Pos.y + (.5f - XMVectorGetY(p) * .5f) * viewport->Size.y};
        return std::isfinite(out.x) && std::isfinite(out.y);
    };
    const auto line = [&](fvector_t a, fvector_t b)
    { ImVec2 pa{}, pb{}; if (project(a, pa) && project(b, pb)) draw->AddLine(pa, pb, IM_COL32(255, 224, 80, 220), 1.5f); };
    const vector_t origin = XMLoadFloat4(&light.vPosition);
    const bool spot = light.eType == LIGHT::SPOT;
    for (int ring = 0; ring < (spot ? 1 : 3); ++ring)
    {
        const vector_t direction = spot ? XMVector3Normalize(XMLoadFloat4(&light.vDirection)) :
            ring == 0 ? XMVectorSet(0.f, 1.f, 0.f, 0.f) : ring == 1 ? XMVectorSet(1.f, 0.f, 0.f, 0.f) : XMVectorSet(0.f, 0.f, 1.f, 0.f);
        const vector_t up = std::abs(XMVectorGetY(direction)) > .99f ? XMVectorSet(0.f, 0.f, 1.f, 0.f) : XMVectorSet(0.f, 1.f, 0.f, 0.f);
        const vector_t right = XMVector3Normalize(XMVector3Cross(direction, up));
        const vector_t forward = XMVector3Normalize(XMVector3Cross(right, direction));
        const float angle = spot ? std::acos(std::clamp(light.fSpotOuterCos, -1.f, 1.f)) : 0.f;
        const float radius = spot ? light.fRange * std::sin(angle) : light.fRange;
        const vector_t center = spot ? origin + direction * (light.fRange * std::cos(angle)) : origin;
        for (int i = 0; i < 48; ++i)
        {
            const float a = XM_2PI * float(i) / 48.f, b = XM_2PI * float(i + 1) / 48.f;
            const vector_t p = center + (right * std::cos(a) + forward * std::sin(a)) * radius;
            line(p, center + (right * std::cos(b) + forward * std::sin(b)) * radius);
            if (spot && i % 12 == 0) line(origin, p);
        }
    }
}
#endif

std::string Card_Asset(const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
{
    using namespace LostArk::Shared;
    // Roulette retains its overhead card; maze assignment now lives on the floor.
    const MECHANIC_CARD_SYMBOL cardSymbol = snapshot.eMechanicCardSymbol;
    const MECHANIC_CARD_COLOR cardColor = snapshot.eMechanicCardColor;
    const char* symbol = nullptr;
    switch (cardSymbol)
    {
    case MECHANIC_CARD_SYMBOL::HEART: symbol = "heart"; break;
    case MECHANIC_CARD_SYMBOL::SPADE: symbol = "spade"; break;
    case MECHANIC_CARD_SYMBOL::CLUB: symbol = "clober"; break;
    case MECHANIC_CARD_SYMBOL::DIAMOND: symbol = "dia"; break;
    default: return {};
    }
    const char* color = nullptr;
    switch (cardColor)
    {
    case MECHANIC_CARD_COLOR::RED: color = "red"; break;
    case MECHANIC_CARD_COLOR::BLACK: color = "black"; break;
    default: return {};
    }
    return std::string("boss.kouku.card.") + symbol + "." + color;
}
}

struct Client::CKoukuSaydonPresentationPlayer::FRAME_LIGHT_PROVIDER final : Engine::IPresentationProvider
{
    struct FRAME_LIGHT final { LIGHT_DESC desc; bool debugRender = false; };
    std::vector<FRAME_LIGHT> lights;
    std::size_t skippedByBudget = 0u;
    HRESULT Submit_Presentation() override
    {
        auto& presentation = CPresentation_Manager::Get();
        const auto used = presentation.Get_TransientLights().size();
        constexpr auto capacity = CPresentation_Manager::TRANSIENT_LIGHT_CAPACITY;
        const std::size_t remaining = used < capacity ? capacity - used : 0u;
        const auto count = (std::min)(remaining, lights.size());
        skippedByBudget = lights.size() - count;
        // Validation is finished before this provider joins the frame transaction.
        presentation.Register_ProviderSubmissionExpectation(lights.size(), count, 0u, 0u);
        for (std::size_t i = 0u; i < count; ++i)
            if (FAILED(presentation.Add_TransientLight(lights[i].desc))) return E_FAIL;
        return S_OK;
    }
};

std::size_t Client::CKoukuSaydonPresentationPlayer::Light_SkippedByBudget() const
{
    return m_LightProvider ? m_LightProvider->skippedByBudget : 0u;
}

void Client::CKoukuSaydonPresentationPlayer::Collect_FrameLights()
{
    if (!m_LightProvider) m_LightProvider = std::make_shared<FRAME_LIGHT_PROVIDER>();
    m_LightProvider->lights.clear();
    if (!m_pLightResources) return;
    const auto collect = [&](const SESSION& session, bool preview, std::uint32_t bossId = 0u)
    {
        for (const auto& [id, row] : session.rows)
        {
            if (row.kind != KIND::LIGHT || row.failed || row.waitingForAnchor || row.lightWeight <= 0.f) continue;
            if (!m_FearSession.rows.empty() && &session != &m_FearSession) continue;
            const auto* resource = preview ? m_pLightResources->Find_Resource(row.assetId) :
                m_pLightResources->Find_RuntimeResource(row.assetId);
            if (!resource)
            {
                m_strStatus = "Light resource unavailable: " + row.assetId + "; other rows preserved.";
                continue;
            }
            const auto append = [&](const float4x4_t& pivot, const float anchorHeight)
            {
                LIGHT_DESC desc{};
                std::string status;
                if (!CLightResourceCatalog::Try_BuildAnchoredLightDesc(*resource, row.lightBox.strAnchorKind,
                    pivot, anchorHeight, row.lightWeight, desc, status))
                { m_strStatus = "Light occurrence isolated: " + id + "; " + status; return; }
                m_LightProvider->lights.push_back({desc, row.debugRender});
            };
            if (row.lightBox.strAnchorKind == "PLAYER")
            {
                // No present character is a temporary empty target set, never a sticky row failure.
                for (const auto& root : m_LightPlayerPivots)
                {
                    float4x4_t pivot;
                    if (Make_Pivot(row.lightBox, root, nullptr, pivot)) append(pivot, root._42);
                }
            }
            else
            {
                append(row.pivot, row.placementAnchor._42);
                // A Server-owned same-body actor shares its owner's following
                // spotlight. Gaze clones retain their own transform and animation;
                // the original occurrence still owns timing, fade and brightness.
                if (preview || resource->eType != LIGHT::SPOT ||
                    row.lightBox.strAnchorKind != "BOSS" || !row.lightBox.bFollowBoss) continue;
                const auto followers = m_LightBossFollowers.find(bossId);
                if (followers == m_LightBossFollowers.end()) continue;
                for (const auto& follower : followers->second)
                {
                    const auto own = m_BossSessions.find(follower.entityId);
                    const bool hasOwnLight = own != m_BossSessions.end() &&
                        std::any_of(own->second.rows.begin(), own->second.rows.end(),
                            [&](const auto& entry)
                            {
                                const auto& candidate = entry.second;
                                return candidate.kind == KIND::LIGHT && candidate.assetId == row.assetId &&
                                    candidate.lightBox.strAnchorKind == "BOSS" && !candidate.failed &&
                                    !candidate.waitingForAnchor && candidate.lightWeight > 0.f;
                            });
                    if (hasOwnLight) continue;
                    const auto npc = follower.npc.lock();
                    float4x4_t pivot;
                    if (npc && npc->Get_Transform() && Make_Pivot(row.lightBox,
                        *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model(), pivot))
                        append(pivot, npc->Get_Transform()->Get_WorldMatrixPtr()->_42);
                }
            }
        }
    };
    for (const auto& [id, session] : m_BossSessions) collect(session, false, id);
    for (const auto& [id, session] : m_MarioEntrySessions) collect(session, false, id);
    if (m_bPreviewPlaying) collect(m_PreviewSession, true);
    for (const auto& member : m_BundlePreviewMembers) collect(member.session, true);
    collect(m_FearSession, false);
    if (!m_LightProvider->lights.empty())
        if (FAILED(CPresentation_Manager::Get().Add_FrameProvider(m_LightProvider)))
            m_strStatus = "Light frame provider registration failed.";
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_SourceAnchorWorlds(
    const EFFECT_DOCUMENT_DESC& document, const EFFECT_V2_TARGET_VIEW& view,
    const float4x4_t& root, std::unordered_map<std::string, float4x4_t>& anchors, std::string& error)
{
    const auto attachments = Source_Attachments(document);
    SOURCE_BONES bones;
    for (const auto& attachment : attachments)
    {
        if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW ||
            bones.contains(attachment.strRuntimeBoneName)) continue;
        if (!view.pModel || !view.pModel->Has_Bone(attachment.strRuntimeBoneName.c_str()))
        { error = "Selected Kouku model has no source bone: " + attachment.strRuntimeBoneName; return false; }
        float4x4_t value;
        XMStoreFloat4x4(&value, view.pModel->Get_BoneMatrix(attachment.strRuntimeBoneName.c_str()));
        bones.emplace(attachment.strRuntimeBoneName, value);
    }
    return Build_SourceAnchorWorlds(attachments, view, root, bones, anchors, error);
}

bool Client::CKoukuSaydonPresentationPlayer::Sample_SourceAnchorWorlds(
    const EFFECT_DOCUMENT_DESC& document, const EFFECT_V2_TARGET_VIEW& view,
    const float4x4_t& root, const float seconds, SOURCE_BONES& anchors, std::string& error)
{
    if (!document.SourceModelPreview) return Resolve_SourceAnchorWorlds(document, view, root, anchors, error);
    if (!std::isfinite(seconds) || seconds < 0.f)
    { error = "Source model animation time must be finite and nonnegative."; return false; }
    const auto attachments = Source_Attachments(document);
    std::vector<std::string> names;
    for (const auto& attachment : attachments)
        if (attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
            std::find(names.begin(), names.end(), attachment.strRuntimeBoneName) == names.end())
            names.push_back(attachment.strRuntimeBoneName);
    std::vector<ANCHOR_ANIMATION> animations;
    for (const auto& source : document.SourceModelPreview->Animations)
    {
        ANCHOR_ANIMATION animation;
        animation.strRuntimeClip = source.strRuntimeClip;
        animation.iStartOffsetMs = source.iStartOffsetMs; animation.iSourceStartMs = source.iSourceStartMs;
        animation.iPoseStartMs = source.iStartOffsetMs;
        animation.iPlayMs = source.iPlayMs; animation.fPlayRate = source.fPlayRate; animation.strEndPolicy = source.strEndPolicy;
        animations.push_back(std::move(animation));
    }
    SOURCE_BONES bones;
    return Sample_SourceBones(view.pModel, animations, {}, seconds * 1000.f, names, bones, error) &&
        Build_SourceAnchorWorlds(attachments, view, root, bones, anchors, error);
}

Client::CKoukuSaydonPresentationPlayer::CKoukuSaydonPresentationPlayer(
    ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context,
    CRenderingProfileService& profiles)
    : m_Device(std::move(device)), m_Context(std::move(context)), m_Profiles(profiles)
{
    XMStoreFloat4x4(&m_PreviewPivot, XMMatrixIdentity());
}

Client::CKoukuSaydonPresentationPlayer::~CKoukuSaydonPresentationPlayer()
{
    Reset();
}

Client::CKoukuSaydonPresentationPlayer::WORLD_EMISSION_ANCHOR
Client::CKoukuSaydonPresentationPlayer::Make_WorldEmissionAnchor(
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
    const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world,
    const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& occurrence)
{
    if (!pattern.BossMotion || occurrence.Placement || world.strAnchorKind != "BOSS_SPAWN") return {};
    KOUKU_SAYDON_COMPOSITION_PATTERN sampled;
    sampled.BossMotion = pattern.BossMotion;
    return [sampled = std::move(sampled), world, startMs = occurrence.iStartMs](
        const f32_t birthMs, float4x4_t& out)
    {
        std::array<double, 3u> position{};
        double yaw = 0.0;
        if (!Sample_KoukuSaydonBossMotion(sampled, double(startMs) + birthMs, position, yaw)) return false;
        for (std::size_t axis = 0; axis < position.size(); ++axis)
            position[axis] += world.PositionOffset[axis] - world.AnchorPosition[axis];
        XMStoreFloat4x4(&out, XMMatrixRotationY(XMConvertToRadians(float(yaw))) *
            XMMatrixTranslation(float(position[0]), float(position[1]), float(position[2])));
        return true;
    };
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_ProductWorldEmissionAnchor(
    const std::uint32_t sourceRevision, const std::string_view patternId,
    const std::string_view occurrenceId, WORLD_EMISSION_ANCHOR& out) const
{
    if (sourceRevision != m_iProductSourceRevision) return false;
    const auto pattern = m_Product.find(std::string(patternId));
    if (pattern == m_Product.end()) return false;
    const auto anchor = pattern->second.worldEmissionAnchors.find(std::string(occurrenceId));
    out = anchor == pattern->second.worldEmissionAnchors.end() ? WORLD_EMISSION_ANCHOR{} : anchor->second;
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Reload_Product(
    std::string& status, const std::uint32_t expectedSourceRevision)
{
    // Restart keeps its admitted in-memory Product even after another publish.
    if (expectedSourceRevision && m_bProductLoaded && expectedSourceRevision == m_iProductSourceRevision)
    { status = "KoukuSaydon presentation already matches the admitted source revision."; return true; }
    m_bProductAttempted = true;
    try
    {
        const auto path = CProjectDataRoot::Resolve(
            "Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json");
        std::error_code error;
        const auto bytes = std::filesystem::file_size(path, error);
        if (error || bytes > 16u * 1024u * 1024u)
            throw std::runtime_error("KoukuSaydon Product presentation is missing or oversized.");
        std::ifstream input(path, std::ios::binary);
        const std::string text{ std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
        if (!input || input.bad() || text.size() != bytes)
            throw std::runtime_error("KoukuSaydon Product presentation read failed.");
        DATA_JSON_VALUE root;
        std::string parseStatus;
        if (!CDataJson::Parse(text, root, parseStatus) || !root.Is_Object())
            throw std::runtime_error("KoukuSaydon Product parse failed: " + parseStatus);
        if (Text(root, "schema") != "lostark.kouku-saydon-pattern-bindings" ||
            UInt(root, "formatVersion", 1u, 1u) != 1u ||
            Text(root, "bossArchetypeId") != "BOSS_KAKULSAYDON_G1_KOUKU")
            throw std::runtime_error("KoukuSaydon Product presentation header is incompatible.");
        const auto sourceRevision = UInt(root, "sourceRevision", 1u, UINT32_MAX);
        if (expectedSourceRevision && sourceRevision != expectedSourceRevision)
            throw std::runtime_error("KoukuSaydon presentation source revision mismatch: requested " +
                std::to_string(expectedSourceRevision) + ", published " + std::to_string(sourceRevision) +
                ". Previous presentation is preserved; publish the matching Product on this Client.");
        const auto& patterns = Field(root, "patterns");
        if (!patterns.Is_Array() || patterns.Get_Array().size() > 4096u)
            throw std::runtime_error("Product patterns must be a bounded array.");
        std::map<std::string, PRODUCT_PATTERN> staged;
        std::size_t isolatedLights = 0u;
        std::size_t isolatedSceneProfiles = 0u;
        std::string isolatedSceneStatus;
        for (const auto& value : patterns.Get_Array())
        {
            PRODUCT_PATTERN item;
            item.pattern.strPatternId = Text(value, "patternId");
            if (value.Find("gateId")) item.pattern.strGateId = Text(value, "gateId");
            if (value.Find("targetBossPlacementId")) item.pattern.strTargetBossPlacementId = Text(value, "targetBossPlacementId");
            if (value.Find("actorProfileId")) item.pattern.strActorProfileId = Text(value, "actorProfileId");
            item.durationMs = UInt(value, "durationMs", 1u, MAX_TIMELINE_MS);
            if (const auto* windows = value.Find("animationBlendWindows"))
                if (!CKoukuSaydonAnimationBlend::Read_ProductWindows(*windows, item.pattern.AnimationBlendWindows, parseStatus))
                    throw std::runtime_error(parseStatus);
            if (const auto* animations = value.Find("sourceAnchorAnimations"))
            {
                if (!animations->Is_Array() || animations->Get_Array().empty() || animations->Get_Array().size() > 4096u)
                    throw std::runtime_error("Product source anchor animations require a bounded nonempty array.");
                KOUKU_SAYDON_COMPOSITION_STAGE stage;
                stage.iDurationMs = item.durationMs;
                for (const auto& source : animations->Get_Array())
                {
                    ANCHOR_ANIMATION animation;
                    animation.strRuntimeClip = Text(source, "runtimeClip");
                    animation.iStartOffsetMs = UInt(source, "startOffsetMs", 0u, item.durationMs - 1u);
                    animation.iPoseStartMs = source.Find("stageStartMs") ? UInt(source, "stageStartMs", 0u, animation.iStartOffsetMs) : animation.iStartOffsetMs;
                    animation.iSourceStartMs = UInt(source, "sourceStartMs", 0u, MAX_TIMELINE_MS);
                    if (source.Find("sourceEndMs")) animation.iSourceEndMs = UInt(source, "sourceEndMs", 0u, MAX_TIMELINE_MS);
                    animation.iPlayMs = UInt(source, "playMs", 1u, MAX_TIMELINE_MS);
                    animation.iBlendInMs = UInt(source, "blendInMs", 0u, 1000u);
                    animation.fPlayRate = float(Number(source, "playRate", 0.01, 100.0));
                    animation.strEndPolicy = Text(source, "endPolicy");
                    if (animation.strEndPolicy != "EXACT" && animation.strEndPolicy != "HOLD_LAST_POSE" &&
                        animation.strEndPolicy != "LOOP_TO_WINDOW")
                        throw std::runtime_error("Unsupported source anchor animation end policy.");
                    if (!stage.AnimationOccurrences.empty() &&
                        animation.iStartOffsetMs <= stage.AnimationOccurrences.back().iStartOffsetMs)
                        throw std::runtime_error("Source anchor animations must have distinct increasing pattern times.");
                    stage.AnimationOccurrences.push_back(std::move(animation));
                }
                item.pattern.Stages.push_back(std::move(stage));
            }
            if (value.Find("animationRootVerticalScale"))
                item.pattern.fAnimationRootVerticalScale = Number(value, "animationRootVerticalScale", 0.0, 1.0);
            if (const auto* source = value.Find("bossMotion"))
            {
                KOUKU_SAYDON_BOSS_MOTION motion;
                motion.iStartMs = UInt(*source, "startMs", 0u, item.durationMs - 1u);
                motion.iEndMs = UInt(*source, "endMs", motion.iStartMs + 1u, item.durationMs);
                motion.StartPosition = Vector(*source, "startPosition", -100000.0, 100000.0);
                motion.EndPosition = Vector(*source, "endPosition", -100000.0, 100000.0);
                motion.fYawDegrees = Number(*source, "yawDegrees", -360.0, 360.0);
                if (std::abs(motion.StartPosition[1] - motion.EndPosition[1]) > 0.0001)
                    throw std::runtime_error("Product bossMotion must keep its ground height.");
                item.pattern.BossMotion = motion;
            }
            if (const auto* anchors = value.Find("worldEmissionAnchors"))
            {
                if (!item.pattern.BossMotion || !anchors->Is_Array() || anchors->Get_Array().size() > 4096u)
                    throw std::runtime_error("Product WORLD emission anchors require a bounded bossMotion owner.");
                for (const auto& anchor : anchors->Get_Array())
                {
                    KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION world;
                    world.strAnchorKind = "BOSS_SPAWN";
                    world.PositionOffset = Vector(anchor, "positionOffset", -100000.0, 100000.0);
                    world.AnchorPosition = Vector(anchor, "anchorPosition", -100000.0, 100000.0);
                    KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE occurrence;
                    occurrence.strOccurrenceId = Text(anchor, "occurrenceId");
                    occurrence.iStartMs = UInt(anchor, "startMs", 0u, item.durationMs - 1u);
                    if (!item.worldEmissionAnchors.emplace(occurrence.strOccurrenceId,
                        Make_WorldEmissionAnchor(item.pattern, world, occurrence)).second)
                        throw std::runtime_error("Duplicate Product WORLD emission anchor.");
                }
            }
            const auto& boxes = Field(value, "presentationOccurrences");
            if (!boxes.Is_Array() || boxes.Get_Array().size() > 4096u)
                throw std::runtime_error("Product presentationOccurrences must be a bounded array.");
            std::set<std::string> ids;
            for (const auto& box : boxes.Get_Array())
            {
                try
                {
                RESOURCE resource = Read_Resource(box);
                OCCURRENCE occurrence = Read_Occurrence(box, item.durationMs, resource.eKind);
                if (occurrence.strAnchorKind == "WORLD")
                {
                    // The Product pins the published sequence identity; it never
                    // reopens the editable source composition during gameplay.
                    const std::string sequenceId = Text(box, "worldSequenceInstanceId");
                    const auto world = std::find_if(item.document.Worlds.begin(), item.document.Worlds.end(),
                        [&occurrence](const auto& value) { return value.strWorldId == occurrence.strWorldId; });
                    if (world == item.document.Worlds.end())
                    {
                        KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION definition;
                        definition.strWorldId = occurrence.strWorldId;
                        definition.strSequenceInstanceId = sequenceId;
                        item.document.Worlds.push_back(std::move(definition));
                    }
                    else if (world->strSequenceInstanceId != sequenceId)
                        throw std::runtime_error("Conflicting WORLD presentation anchor identity.");
                }
                if (!ids.insert(occurrence.strOccurrenceId).second)
                    throw std::runtime_error("Duplicate Product presentation occurrence.");
                auto old = std::find_if(item.document.PresentationResources.begin(),
                    item.document.PresentationResources.end(), [&resource](const auto& row)
                    { return row.strResourceId == resource.strResourceId; });
                if (old == item.document.PresentationResources.end())
                    item.document.PresentationResources.push_back(std::move(resource));
                else
                {
                    // Scene resourceDurationMs is the individual projected box duration.
                    resource.iDurationMs = old->iDurationMs;
                    if (resource != *old) throw std::runtime_error("Conflicting Product resource identity.");
                }
                item.pattern.PresentationOccurrences.push_back(std::move(occurrence));
                }
                catch (const std::exception& error)
                {
                    const auto* kind = box.Find("kind");
                    if (!kind || !kind->Is_String()) throw;
                    if (kind->Get_String() == "LIGHT")
                    {
                        ++isolatedLights;
                        OutputDebugStringA((std::string("[KoukuSaydonPresentationPlayer] Isolated LIGHT row: ") + error.what() + "\n").c_str());
                    }
                    else if (kind->Get_String() == "SCENE_PROFILE")
                    {
                        ++isolatedSceneProfiles;
                        const auto* id = box.Find("occurrenceId");
                        isolatedSceneStatus = item.pattern.strPatternId + "/" +
                            (id && id->Is_String() ? id->Get_String() : "<missing occurrenceId>") +
                            ": " + error.what();
                        OutputDebugStringA((std::string("[KoukuSaydonPresentationPlayer] Isolated SCENE_PROFILE row: ") +
                            isolatedSceneStatus + "\n").c_str());
                    }
                    else throw;
                }
            }
            const std::string key = item.pattern.strPatternId;
            if (!staged.emplace(key, std::move(item)).second)
                throw std::runtime_error("Duplicate Product pattern identity.");
        }

        std::map<std::string, PRODUCT_BUNDLE> stagedBundles;
        if (const auto* bundles = root.Find("bundles"))
        {
            if (!bundles->Is_Array() || bundles->Get_Array().size() > 4096u)
                throw std::runtime_error("Product bundles must be a bounded array.");
            for (const auto& value : bundles->Get_Array())
            {
                PRODUCT_BUNDLE item;
                auto& common = item.common;
                common.pattern.strPatternId = Text(value, "bundleId");
                common.pattern.strGateId = Text(value, "gateId");
                common.durationMs = UInt(value, "durationMs", 1u, MAX_TIMELINE_MS);
                const auto& members = Field(value, "members");
                const auto& boxes = Field(value, "presentationOccurrences");
                if (!members.Is_Array() || members.Get_Array().empty() ||
                    members.Get_Array().size() > LostArk::Shared::MAX_KOUKUSAYDON_BUNDLE_MEMBERS ||
                    !boxes.Is_Array() || boxes.Get_Array().size() > 4096u)
                    throw std::runtime_error("Product bundle members/common rows are invalid.");
                std::set<std::string> memberIds, targets, boxIds;
                std::vector<PRESENTATION_WINDOW> globalWindows;
                for (const auto& box : boxes.Get_Array())
                {
                    auto resource = Read_Resource(box);
                    if (resource.eKind != KIND::CAMERA && resource.eKind != KIND::SCENE_PROFILE)
                        throw std::runtime_error("Product bundle common lane has a non-global resource.");
                    auto occurrence = Read_Occurrence(box, common.durationMs, resource.eKind);
                    if (!boxIds.insert(occurrence.strOccurrenceId).second)
                        throw std::runtime_error("Product bundle has duplicate common occurrences.");
                    (void)Admit_PresentationWindow(globalWindows, resource.eKind, occurrence.iStartMs,
                        double(occurrence.iStartMs) + occurrence.iDurationMs + Camera_ReturnMs(resource, false), "common");
                    const auto previous = std::find_if(common.document.PresentationResources.begin(),
                        common.document.PresentationResources.end(), [&](const auto& row)
                        { return row.strResourceId == resource.strResourceId; });
                    if (previous == common.document.PresentationResources.end())
                        common.document.PresentationResources.push_back(std::move(resource));
                    else
                    {
                        resource.iDurationMs = previous->iDurationMs;
                        if (resource != *previous) throw std::runtime_error("Conflicting bundle resource identity.");
                    }
                    common.pattern.PresentationOccurrences.push_back(std::move(occurrence));
                }
                for (const auto& member : members.Get_Array())
                {
                    const auto memberId = Text(member, "memberId");
                    const auto patternId = Text(member, "patternId");
                    const auto targetId = Text(member, "targetBossPlacementId");
                    const auto actorId = Text(member, "actorProfileId");
                    const auto offsetMs = UInt(member, "startOffsetMs", 0u, MAX_TIMELINE_MS);
                    const double offset = std::ceil(double(offsetMs) * 30.0 / 1000.0) * 1000.0 / 30.0;
                    const auto child = staged.find(patternId);
                    if (!memberIds.insert(memberId).second || !targets.insert(targetId).second ||
                        child == staged.end() || child->second.pattern.strGateId != common.pattern.strGateId ||
                        child->second.pattern.strTargetBossPlacementId != targetId || child->second.pattern.strActorProfileId != actorId)
                        throw std::runtime_error("Product bundle has duplicate or inconsistent child targets.");
                    for (const auto& row : child->second.pattern.PresentationOccurrences)
                        for (const auto& resource : child->second.document.PresentationResources)
                            if (resource.strResourceId == row.strResourceId &&
                                (resource.eKind == KIND::CAMERA || resource.eKind == KIND::SCENE_PROFILE) &&
                                !Admit_PresentationWindow(globalWindows, resource.eKind, offset + row.iStartMs,
                                    offset + row.iStartMs + row.iDurationMs + Camera_ReturnMs(resource, false), memberId))
                                throw std::runtime_error("Product bundle global presentation windows overlap.");
                    item.patternIds.push_back(patternId);
                }
                const auto key = common.pattern.strPatternId;
                if (!stagedBundles.emplace(key, std::move(item)).second)
                    throw std::runtime_error("Duplicate Product bundle identity.");
            }
        }

        std::map<std::string, PRODUCT_PATTERN> stagedFear;
        if (const auto* presentations = root.Find("fearPresentations"))
        {
            if (!presentations->Is_Array() || presentations->Get_Array().size() > 4096u)
                throw std::runtime_error("Fear presentations must be a bounded array.");
            for (const auto& value : presentations->Get_Array())
            {
                PRODUCT_PATTERN item;
                item.pattern.strPatternId = Text(value, "presentationId");
                item.durationMs = UInt(value, "durationMs", 1u, MAX_TIMELINE_MS);
                const auto append = [&](RESOURCE resource, std::uint32_t startMs, const std::string& anchor)
                {
                    OCCURRENCE box;
                    box.strOccurrenceId = item.pattern.strPatternId + ":" + resource.strResourceId;
                    box.strResourceId = resource.strResourceId;
                    box.iStartMs = startMs;
                    box.iDurationMs = item.durationMs - startMs;
                    box.strAnchorKind = anchor;
                    box.bDebugRender = false;
                    item.document.PresentationResources.push_back(std::move(resource));
                    item.pattern.PresentationOccurrences.push_back(std::move(box));
                };
                const auto sceneId = Text(value, "sceneProfileId", true);
                if (!sceneId.empty())
                {
                    RESOURCE scene;
                    scene.strResourceId = "fear.scene";
                    scene.eKind = KIND::SCENE_PROFILE;
                    scene.strResourceKind.clear();
                    scene.strAssetId = sceneId;
                    scene.iDurationMs = item.durationMs;
                    append(std::move(scene), 0u, "BOSS");
                }
                const auto delayMs = UInt(value, "effectDelayMs", 0u, item.durationMs - 1u);
                if (const auto* effect = value.Find("effectResource"); effect && !effect->Is_Null())
                {
                    auto resource = Read_Resource(*effect);
                    if (resource.eKind != KIND::EFFECT) throw std::runtime_error("Fear effectResource must be EFFECT.");
                    append(std::move(resource), delayMs, "BOSS");
                }
                else if (delayMs) throw std::runtime_error("Fear delay requires an Effect resource.");
                if (const auto* light = value.Find("lightResource"); light && !light->Is_Null())
                {
                    auto resource = Read_Resource(*light);
                    if (resource.eKind != KIND::LIGHT) throw std::runtime_error("Fear lightResource must be LIGHT.");
                    append(std::move(resource), 0u, "PLAYER");
                }
                const auto key = item.pattern.strPatternId;
                if (!stagedFear.emplace(key, std::move(item)).second)
                    throw std::runtime_error("Duplicate Fear presentation identity.");
            }
        }

        // Only a fully staged replacement may stop the old running presentation.
        for (auto& [id, session] : m_BossSessions) Stop_Session(session);
        m_BossSessions.clear();
        for (auto& [id, session] : m_MarioEntrySessions) Stop_Session(session);
        m_MarioEntrySessions.clear();
        m_Product = std::move(staged);
        Stop_Session(m_FearSession);
        m_FearSession.key.clear();
        m_strCompletedFearKey.clear();
        m_FearPresentations = std::move(stagedFear);
        Stop_Session(m_ProductBundleSession);
        m_ProductBundles = std::move(stagedBundles);
        m_iProductSourceRevision = sourceRevision;
        m_MissingProductPatterns.clear();
        m_bProductLoaded = true;
        Refresh_SharedPresentation();
        status = "Loaded KoukuSaydon presentation for " + std::to_string(m_Product.size()) + " Product patterns.";
        if (isolatedLights) status += " Isolated invalid LIGHT rows: " + std::to_string(isolatedLights);
        if (isolatedSceneProfiles) status += " Isolated invalid SCENE_PROFILE rows: " +
            std::to_string(isolatedSceneProfiles) + ". " + isolatedSceneStatus;
        m_strStatus = status;
        return true;
    }
    catch (const std::exception& error)
    {
        status = error.what();
        m_strStatus = status;
        OutputDebugStringA(("[KoukuSaydonPresentationPlayer] " + status + "\n").c_str());
        return false;
    }
}

bool Client::CKoukuSaydonPresentationPlayer::Ensure_EffectResource(
    const std::string& kind, const std::string& asset,
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>& snapshot)
{
    const auto generation = CEffectV2Runtime::Cache_Generation();
    if (generation != m_iEffectCacheGeneration)
    {
        m_EffectResources.clear();
        m_EffectResourceFailures.clear();
        m_iEffectCacheGeneration = generation;
    }
    const std::string key = kind + ":" + asset;
    if (const auto found = m_EffectResources.find(key); found != m_EffectResources.end())
    { snapshot = found->second; return true; }
    if (const auto failed = m_EffectResourceFailures.find(key); failed != m_EffectResourceFailures.end())
    { m_strStatus = failed->second; return false; }
    EFFECT_V2_RESOURCE_KIND resourceKind;
    if (kind == "GROUP") resourceKind = EFFECT_V2_RESOURCE_KIND::GROUP;
    else if (kind == "LEAF") resourceKind = EFFECT_V2_RESOURCE_KIND::LEAF;
    else { m_strStatus = "Unsupported Effect owner: " + kind; return false; }
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> staged;
    if (!CEffectV2Catalog::Get().Load_ResourceSnapshot(resourceKind, asset, staged, m_strStatus))
    {
        m_strStatus = "Effect resource " + asset + ": " + m_strStatus;
        m_EffectResourceFailures.emplace(key, m_strStatus);
        return false;
    }
    m_EffectResources.emplace(key, staged);
    snapshot = std::move(staged);
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Stop_Session(SESSION& session)
{
    for (auto& [id, row] : session.rows)
    {
        if (row.effectHandle) CEffectV2Runtime::Stop_Group(row.effectHandle);
        if (row.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({row.v1EffectHandle});
        if (row.soundHandle) CGameInstance::Get().Stop_SoundCue(row.soundHandle);
    }
    session.rows.clear();
    session.rootHistory.reset();
    session.effectAnchorHistories.clear();
    session.rootRecordedSeconds = -1.f;
    session.lastClockMs = -1.f;
}

void Client::CKoukuSaydonPresentationPlayer::Sample(SESSION& session,
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, float clockMs, bool paused,
    const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model,
    const ANIMATION_MODEL_TARGET_VIEW* weaponView)
{
    if (!std::isfinite(clockMs) || clockMs < 0.f) return;
    if (session.lastClockMs >= 0.f &&
        (clockMs < session.lastClockMs - 0.5f || clockMs > session.lastClockMs + 150.f))
    {
        // Same occurrence seek retains observed actor history; a new Server run
        // calls Stop_Session separately and never borrows the previous run.
        auto history = session.rootHistory;
        auto anchorHistories = std::move(session.effectAnchorHistories);
        const auto recordedSeconds = session.rootRecordedSeconds;
        const auto recordedPivot = session.rootRecordedPivot;
        Stop_Session(session);
        session.rootHistory = std::move(history);
        session.effectAnchorHistories = std::move(anchorHistories);
        session.rootRecordedSeconds = recordedSeconds;
        session.rootRecordedPivot = recordedPivot;
    }
    if (!session.rootHistory) session.rootHistory = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
    const float rootSeconds = clockMs / 1000.f;
    if (rootSeconds >= session.rootRecordedSeconds)
    {
        const float dx = pivot._41 - session.rootRecordedPivot._41;
        const float dy = pivot._42 - session.rootRecordedPivot._42;
        const float dz = pivot._43 - session.rootRecordedPivot._43;
        const bool jump = session.rootRecordedSeconds >= 0.f && dx*dx + dy*dy + dz*dz > 2500.f;
        std::string historyStatus;
        if (session.rootHistory->Record(rootSeconds, pivot, jump, historyStatus))
        { session.rootRecordedSeconds = rootSeconds; session.rootRecordedPivot = pivot; }
        else m_strStatus = std::move(historyStatus);
    }
    const auto resolveWorldPivot = [&](const OCCURRENCE& box, float4x4_t& anchor) {
        const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
            [&box](const auto& value) { return value.strWorldId == box.strWorldId; });
        if (world == document.Worlds.end()) return false;
        if (!session.previewWorlds.empty())
        {
            const CWorldSequencePlayer* selected = nullptr;
            for (const auto& [id, player] : session.previewWorlds)
                if ((box.strWorldOccurrenceId.empty() || id == box.strWorldOccurrenceId) &&
                    player->Is_Playing(world->strSequenceInstanceId))
                { if (selected) return false; selected = player.get(); }
            return selected && selected->Try_GetSequencePivot(world->strSequenceInstanceId, anchor, box.iWorldEmissionIndex);
        }
        const auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level) return false;
        if (session.runEpoch)
            return level->Try_GetOwnedCompositionWorldPivot(session.runEpoch, session.memberId,
                world->strSequenceInstanceId, box.strWorldOccurrenceId, anchor, box.iWorldEmissionIndex);
        return level->Try_GetCompositionWorldPivot(world->strSequenceInstanceId, anchor, box.strWorldOccurrenceId, box.iWorldEmissionIndex);
    };
    // Observe future anchored cues as well as active ones. Their first emission
    // can then interpolate the real samples bracketing the box start even when
    // a render frame crosses that start by more than one fixed step.
    for (const auto& box : pattern.PresentationOccurrences)
    {
        if (!box.bFollowBoss || (box.strAnchorKind != "WORLD" && box.strBone.empty()) ||
            clockMs > double(box.iStartMs) + box.iDurationMs) continue;
        const auto resource = std::find_if(document.PresentationResources.begin(),
            document.PresentationResources.end(), [&box](const auto& row)
            { return row.strResourceId == box.strResourceId && row.eKind == KIND::EFFECT; });
        if (resource == document.PresentationResources.end()) continue;
        auto& history = session.effectAnchorHistories[box.strOccurrenceId];
        if (!history.samples) history.samples = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
        // Rewinding retains observed poses; it never appends fabricated history.
        if (rootSeconds < history.recordedSeconds) continue;
        auto sampledBox = box;
        float4x4_t anchor = pivot;
        auto anchorModel = model;
        float3_t anchorScale{1.f, 1.f, 1.f};
        if (box.strAnchorKind == "WORLD")
        {
            if (!resolveWorldPivot(box, anchor)) { history.missingSinceSample = true; continue; }
            const matrix_t matrix = XMLoadFloat4x4(&anchor);
            anchorScale = {XMVectorGetX(XMVector3Length(matrix.r[0])),
                XMVectorGetX(XMVector3Length(matrix.r[1])), XMVectorGetX(XMVector3Length(matrix.r[2]))};
            sampledBox.strBone.clear();
            anchorModel.reset();
        }
        float4x4_t unusedPivot, basis;
        if (!Make_Pivot(sampledBox, anchor, anchorModel, unusedPivot, weaponView, &basis))
        { history.missingSinceSample = true; continue; }
        float4x4_t recorded;
        XMStoreFloat4x4(&recorded, XMMatrixScaling(anchorScale.x, anchorScale.y, anchorScale.z) *
            XMLoadFloat4x4(&basis));
        const float dx = recorded._41 - history.recordedPivot._41;
        const float dy = recorded._42 - history.recordedPivot._42;
        const float dz = recorded._43 - history.recordedPivot._43;
        const bool discontinuity = history.recordedSeconds >= 0.f &&
            (history.missingSinceSample || dx*dx + dy*dy + dz*dz > 2500.f);
        std::string historyStatus;
        if (history.samples->Record(rootSeconds, recorded, discontinuity, historyStatus))
        {
            history.recordedSeconds = rootSeconds;
            history.recordedPivot = recorded;
            history.missingSinceSample = false;
        }
        else m_strStatus = "Effect anchor history " + box.strOccurrenceId + ": " + historyStatus;
    }
    std::set<std::string> active;
    const auto sampleOne = [&](const RESOURCE& resource, const OCCURRENCE& box)
    {
        if (clockMs < box.iStartMs || clockMs >= double(box.iStartMs) + box.iDurationMs) return;
        active.insert(box.strOccurrenceId);
        auto [found, inserted] = session.rows.try_emplace(box.strOccurrenceId);
        PLAYING_ROW& row = found->second;
        if (row.failed) return;
        struct ROW_FAILURE_GUARD
        {
            PLAYING_ROW& row;
            const std::string& status;
            ~ROW_FAILURE_GUARD() { if (row.failed && row.failureStatus.empty()) row.failureStatus = status; }
        } failureGuard{row, m_strStatus};
        const float age = (clockMs - box.iStartMs) / 1000.f;
        row.kind = resource.eKind;
        row.debugRender = box.bDebugRender;
        row.startMs = float(box.iStartMs);
        row.cameraDurationMs = box.iDurationMs;
        row.assetId = resource.strAssetId;
        row.cameraOffset = { float(box.PositionOffset[0]), float(box.PositionOffset[1]), float(box.PositionOffset[2]) };
        if (resource.eKind == KIND::LIGHT)
        {
            row.lightBox = box;
            row.lightWeight = float(box.fBrightnessMultiplier);
            if (box.iFadeInMs) row.lightWeight *= (std::min)(1.f, (clockMs - box.iStartMs) / box.iFadeInMs);
            if (box.iFadeOutMs) row.lightWeight *= (std::min)(1.f, (box.iStartMs + box.iDurationMs - clockMs) / box.iFadeOutMs);
        }
        if (resource.eKind != KIND::CAMERA && (inserted || box.bFollowBoss || row.waitingForAnchor) &&
            !(resource.eKind == KIND::LIGHT && box.strAnchorKind == "PLAYER"))
        {
            OCCURRENCE placedBox = box;
            float4x4_t anchor = pivot;
            auto anchorModel = model;
            if ((resource.eKind == KIND::LIGHT || resource.eKind == KIND::EFFECT) && box.strAnchorKind == "MAP")
            { XMStoreFloat4x4(&anchor, XMMatrixIdentity()); anchorModel.reset(); }
            if (box.strAnchorKind == "WORLD")
            {
                if (!resolveWorldPivot(box, anchor))
                {
                    // A WORLD may not have its first sampled pose yet. Retry
                    // presentation anchors next frame instead of hiding this box forever.
                    row.waitingForAnchor = resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT || resource.eKind == KIND::LIGHT;
                    row.failed = !row.waitingForAnchor;
                    m_strStatus = "WORLD presentation anchor unavailable: " + box.strWorldId;
                    return;
                }
                const matrix_t worldMatrix = XMLoadFloat4x4(&anchor);
                // Lights use metre offsets and retain their source shape, independent of model scale.
                for (size_t axis = 0; resource.eKind != KIND::LIGHT && axis < 3u; ++axis)
                {
                    const float scale = XMVectorGetX(XMVector3Length(worldMatrix.r[axis]));
                    placedBox.Scale[axis] *= scale;
                    placedBox.PositionOffset[axis] *= scale;
                }
                Flatten_CenteredWorldCircle(resource, box, anchor);
                placedBox.strBone.clear();
                anchorModel.reset();
            }
            if (!Make_Pivot(placedBox, anchor, anchorModel, row.pivot, weaponView,
                (resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT || resource.eKind == KIND::LIGHT) ? &row.placementAnchor : nullptr))
            {
                row.waitingForAnchor = resource.eKind == KIND::LIGHT;
                row.failed = !row.waitingForAnchor;
                m_strStatus = "Presentation bone/pivot is unavailable: " + box.strOccurrenceId;
                return;
            }
            if (row.waitingForAnchor)
                m_strStatus = "Presentation WORLD anchor ready: " + box.strWorldId;
            row.waitingForAnchor = false;
            if (resource.eKind == KIND::COLLIDER)
                row.wire = Collider_Wire(resource, placedBox);
            if (resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT)
            {
                for (size_t axis = 0u; axis < 3u; ++axis)
                    row.placementAnchorScale[axis] = placedBox.Scale[axis] / box.Scale[axis];
                row.hasPlacementAnchor = true;
            }
        }
        if (resource.eKind == KIND::EFFECT && box.bFollowBoss &&
            (box.strAnchorKind == "WORLD" || !box.strBone.empty()))
        {
            const auto history = session.effectAnchorHistories.find(box.strOccurrenceId);
            if (history != session.effectAnchorHistories.end()) row.effectPivotHistory = history->second.samples;
        }
        if (inserted || (resource.eKind == KIND::EFFECT && !row.effectHandle && !row.v1EffectHandle))
        {
            switch (resource.eKind)
            {
            case KIND::EFFECT:
            {
                if (resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT")
                {
                    const auto catalogRevision = CEffectCatalog::Get_RuntimeRevision();
                    if (catalogRevision != m_iV1CatalogRevision)
                    { m_QueuedV1Effects.clear(); m_iV1CatalogRevision = catalogRevision; }
                    std::vector<std::string> targets{resource.strAssetId};
                    if (m_QueuedV1Effects.insert(resource.strAssetId).second)
                    {
                        std::vector<std::string> admitted;
                        if (!CEffectPresentationService::Queue_ProductTargets_Priority(targets, admitted, m_strStatus))
                        { m_QueuedV1Effects.erase(resource.strAssetId); row.failed = true; break; }
                    }
                    const auto preparation = CEffectPresentationService::Get_ProductCuePreparationProbe(targets);
                    if (preparation.iFailedCount || preparation.iUnavailableCount)
                    {
                        row.failed = true;
                        m_strStatus = "V1 Effect preparation failed: " + resource.strAssetId + "; " +
                            CEffectPresentationService::Get_ProductCuePreparationFailure(resource.strAssetId);
                        break;
                    }
                    if (!preparation.bCatalogRevisionCurrent || !preparation.bSettled) break;
                    row.sourceAnchorSampler = Make_SourceAnchorSampler(resource, pattern, model, pivot, box.iStartMs);
                    EFFECT_LEVEL_PLACEMENT_SPAWN_DESC spawn;
                    spawn.iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
                    spawn.strPlacementId = "kouku:" + session.key + ":" + box.strOccurrenceId;
                    spawn.strEffectAssetId = resource.strAssetId;
                    spawn.strElementId = resource.strElementId;
                    spawn.RootWorld = row.pivot;
                    spawn.fInitialSampleTimeSeconds = age;
                    spawn.bExternallySampled = true;
                    EFFECT_WORLD_ROOT_HANDLE handle;
                    if (!CEffectPresentationService::Spawn_LevelPlacement(spawn, handle, m_strStatus))
                    {
                        m_strStatus = "V1 Effect spawn rejected: " + resource.strAssetId + " | " + box.strOccurrenceId + "; " + m_strStatus;
                        row.failed = true;
                    }
                    else row.v1EffectHandle = handle.iValue;
                    break;
                }
                std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
                if (!Ensure_EffectResource(resource.strResourceKind, resource.strAssetId, effects))
                { row.failed = true; break; }
                EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                playback.PivotWorld = row.pivot;
                playback.fInitialAgeSeconds = age;
                playback.fDurationSeconds = box.iDurationMs / 1000.f;
                // Workbench Fade 0 retains the leaf envelope via the runtime's -1 sentinel.
                playback.fFadeInSeconds = box.iFadeInMs > 0u ? box.iFadeInMs / 1000.f : -1.f;
                playback.fFadeOutSeconds = box.iFadeOutMs > 0u ? box.iFadeOutMs / 1000.f : -1.f;
                playback.fDissolveOutStart = box.iFadeOutMs > 0u ? float(box.fDissolveStart) : -1.f;
                playback.fDissolveOutEnd = box.iFadeOutMs > 0u ? float(box.fDissolveEnd) : -1.f;
                // Server/preview occurrence age owns this lane. Layer Update and
                // MainApp's general Effect clock cannot advance it a second time.
                playback.bProductOwned = true;
                playback.bExternalClock = true;
                playback.PivotSampler = Effect_PivotSampler(box, session.rootHistory, row.effectPivotHistory);
                if (resource.strResourceKind == "GROUP")
                {
                    const auto* group = effects->Find_Group(resource.strAssetId);
                    if (group)
                    {
                        // A particle composition owns emission intervals and tail;
                        // stretching it to the box would manufacture extra births.
                        if (std::any_of(effects->Get_Documents().begin(), effects->Get_Documents().end(),
                            [](const auto& leaf) { return leaf.eType == EFFECT_V2_TYPE::PARTICLE; }))
                            playback.fDurationSeconds = -1.f;
                        row.effectHandle = CEffectV2Runtime::Play_Group(*group,
                            effects, playback, m_Device, m_Context);
                    }
                }
                else if (resource.strResourceKind == "LEAF")
                {
                    const auto* leaf = effects->Find_Document(resource.strAssetId);
                    // Particle boxes include the living particles after emission ends.
                    // Keep the source emitter lifetime/loop; the box still owns final cleanup.
                    if (leaf && leaf->Desc.eShape == CEffectV2Object::SHAPE::PARTICLE)
                        playback.fDurationSeconds = -1.f;
                    row.effectHandle = CEffectV2Runtime::Play_Leaf(resource.strAssetId,
                        effects, playback, m_Device, m_Context);
                }
                if (!row.effectHandle)
                {
                    row.failed = true;
                    m_strStatus = "Effect occurrence unavailable: " + resource.strAssetId + "; " + CEffectV2Runtime::Last_Error();
                }
                break;
            }
            case KIND::SOUND:
            {
                const auto path = CRuntimeAssetRoot::Resolve(resource.strAssetId);
                if (!path.empty()) row.soundHandle = CGameInstance::Get().Play_SoundCue(
                    path.wstring(), float(box.fVolume), static_cast<std::uint32_t>(age * 1000.f));
                if (!row.soundHandle)
                {
                    row.failed = true;
                    m_strStatus = "Sound occurrence unavailable: " + resource.strAssetId;
                }
                break;
            }
            case KIND::LIGHT: break;
            case KIND::COLLIDER: break;
            case KIND::CAMERA: break;
            case KIND::SCENE_PROFILE:
                if (!m_Profiles.Has_Profile(resource.strAssetId))
                {
                    row.failed = true;
                    m_strStatus = "Scene profile unavailable: " + resource.strAssetId;
                }
                break;
            default:
                row.failed = true;
                m_strStatus = "Unsupported occurrence resource: " + resource.strResourceId;
                break;
            }
        }
        if (row.v1EffectHandle)
        {
            const bool previewSession = &session == &m_PreviewSession || std::any_of(
                m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
                [&](const auto& member) { return &session == &member.session; });
            const bool captureSample = previewSession && m_bPreviewCaptureClockHeld;
            CEffectPresentationService::Set_ScreenPostCaptureAllowed({row.v1EffectHandle},
                !captureSample || m_bPreviewCaptureAllowed);
            if (!CEffectPresentationService::Update_WorldRoot({row.v1EffectHandle}, row.pivot) ||
                !CEffectPresentationService::Seek_WorldRoot({row.v1EffectHandle}, age,
                    Effect_V1TransformProvider(box, row.pivot, session.rootHistory, row.effectPivotHistory, row.sourceAnchorSampler),
                    captureSample) || (captureSample && FAILED(
                        CEffectPresentationService::Commit_WorldRootCaptureSample({row.v1EffectHandle}))))
            {
                CEffectPresentationService::Stop_WorldRoot({row.v1EffectHandle});
                row.v1EffectHandle = 0u;
                row.failed = true;
                m_strStatus = "V1 Effect occurrence lost its admitted handle after spawn/attach or rendering failure: " +
                    box.strOccurrenceId + ". " + CEffectPresentationService::Get_Status() +
                    " Correct the resource and restart or seek Preview to retry.";
                return;
            }
        }
        if (row.effectHandle)
        {
            CEffectV2Runtime::Set_GroupPivot(row.effectHandle, row.pivot);
            (void)CEffectV2Runtime::Sample_Group(row.effectHandle, age, paused, m_Device, m_Context);
            std::string failure;
            if (CEffectV2Runtime::Consume_GroupFailure(row.effectHandle, failure))
            {
                CEffectV2Runtime::Stop_Group(row.effectHandle);
                row.effectHandle = 0;
                row.failed = true;
                m_strStatus = std::move(failure);
            }
        }
        if (row.soundHandle) CGameInstance::Get().Pause_SoundCue(row.soundHandle, paused);
        row.lastAge = age;
    };
    for (const auto& box : pattern.PresentationOccurrences)
    {
        const auto resource = std::find_if(document.PresentationResources.begin(),
            document.PresentationResources.end(), [&box](const auto& row)
            { return row.strResourceId == box.strResourceId; });
        if (resource != document.PresentationResources.end()) sampleOne(*resource, box);
    }
    for (const auto& sceneBox : pattern.SceneProfileOccurrences)
    {
        const auto profile = std::find_if(document.SceneProfiles.begin(), document.SceneProfiles.end(),
            [&sceneBox](const auto& row) { return row.strSceneProfileId == sceneBox.strSceneProfileId; });
        if (profile == document.SceneProfiles.end()) continue;
        RESOURCE resource;
        resource.eKind = KIND::SCENE_PROFILE;
        resource.strResourceId = profile->strSceneProfileId;
        resource.strAssetId = profile->strRenderingProfileId;
        OCCURRENCE box;
        box.strOccurrenceId = sceneBox.strOccurrenceId;
        box.strResourceId = sceneBox.strSceneProfileId;
        box.iStartMs = sceneBox.iStartMs;
        box.iDurationMs = sceneBox.iDurationMs;
        box.iFadeInMs = sceneBox.iBlendMs;
        sampleOne(resource, box);
    }
    for (auto row = session.rows.begin(); row != session.rows.end();)
    {
        if (active.contains(row->first)) { ++row; continue; }
        if (row->second.effectHandle) CEffectV2Runtime::Stop_Group(row->second.effectHandle);
        if (row->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({row->second.v1EffectHandle});
        if (row->second.soundHandle) CGameInstance::Get().Stop_SoundCue(row->second.soundHandle);
        row = session.rows.erase(row);
    }
    session.lastClockMs = clockMs;
}

void Client::CKoukuSaydonPresentationPlayer::Restore_Scene()
{
    if (m_bSceneUsed && !m_strScenePrevious.empty() &&
        m_Profiles.Get_ActiveProfileId() == m_strSceneApplied)
    {
        std::string status;
        if (!m_Profiles.Activate_Profile(m_strScenePrevious, status))
        {
            // Keep the owned previous profile until restoration succeeds or an
            // external profile supersedes it; a transient failure is not a release.
            m_strStatus = status;
            return;
        }
    }
    m_bSceneUsed = false;
    m_strScenePrevious.clear();
    m_strSceneOwner.clear();
    m_strSceneApplied.clear();
}

void Client::CKoukuSaydonPresentationPlayer::Refresh_SharedPresentation()
{
    Collect_FrameLights();
    const PLAYING_ROW* scene = nullptr;
    const PLAYING_ROW* camera = nullptr;
    std::string sceneOwner;
    std::string cameraOwner;
    bool cameraPreview = false;
    std::set<KIND> conflictingGlobals;
    std::map<KIND, std::string> globalOwners;
    const auto inspectGlobals = [&](const SESSION& session)
    {
        if (!m_ProductBundleSession.runEpoch || session.runEpoch != m_ProductBundleSession.runEpoch) return;
        for (const auto& [id, row] : session.rows)
            if (!row.failed && (row.kind == KIND::CAMERA || row.kind == KIND::SCENE_PROFILE))
            {
                const auto [owner, inserted] = globalOwners.emplace(row.kind, session.key);
                if (!inserted && owner->second != session.key) conflictingGlobals.insert(row.kind);
            }
    };
    inspectGlobals(m_ProductBundleSession);
    for (const auto& [id, session] : m_BossSessions) inspectGlobals(session);
    if (!conflictingGlobals.empty())
        m_strStatus = "Bundle global presentation owner conflict; overlapping Camera/Scene rows isolated.";
    const auto choose = [&](const SESSION& session, bool product)
    {
        const PLAYING_ROW* localScene = nullptr;
        const PLAYING_ROW* localCamera = nullptr;
        std::string localOwner;
        std::string localCameraOwner;
        for (const auto& [id, row] : session.rows)
        {
            if (row.failed || (product && conflictingGlobals.contains(row.kind))) continue;
            if (row.kind == KIND::SCENE_PROFILE && (!localScene || row.startMs >= localScene->startMs))
            {
                localScene = &row;
                localOwner = session.key + ":" + id;
            }
            if (row.kind == KIND::CAMERA && (!localCamera || row.startMs >= localCamera->startMs))
            {
                localCamera = &row;
                localCameraOwner = session.key + ":" + id + ":" +
                    std::to_string(product ? session.runEpoch : m_iPreviewGeneration);
            }
        }
        if (localScene) { scene = localScene; sceneOwner = std::move(localOwner); }
        if (localCamera) { camera = localCamera; cameraOwner = std::move(localCameraOwner); cameraPreview = !product; }
    };
    // Stable entity/id order resolves overlapping presentation; preview owns the final choice.
    for (const auto& [id, session] : m_BossSessions) choose(session, true);
    choose(m_ProductBundleSession, true);
    for (const auto& member : m_BundlePreviewMembers) choose(member.session, false);
    if (m_bPreviewPlaying) choose(m_PreviewSession, false);
    // A local player's temporary fear owns the scene until its replicated end.
    choose(m_FearSession, false);
    if (!scene) Restore_Scene();
    else if (m_strSceneOwner != sceneOwner || m_strSceneApplied != scene->assetId)
    {
        const std::string previous = m_Profiles.Get_ActiveProfileId();
        std::string status;
        if (m_Profiles.Activate_Profile(scene->assetId, status))
        {
            if (!m_bSceneUsed) m_strScenePrevious = previous;
            m_bSceneUsed = true;
            m_strSceneOwner = std::move(sceneOwner);
            m_strSceneApplied = scene->assetId;
        }
        else m_strStatus = status;
    }
    if (auto* level = CLevel_KakulSaydonArena::Get_Active())
    {
        const bool cameraEnabled = level->Is_CompositionCameraEnabled();
        if (camera && cameraEnabled)
        {
            if (level->Sample_CompositionCamera(camera->assetId, camera->lastAge, camera->cameraOffset,
                cameraOwner, camera->cameraDurationMs, cameraPreview))
                m_bCameraUsed = true;
            else
            {
                m_strStatus = "Camera shot unavailable or interrupted: " + camera->assetId;
                if (m_bCameraUsed) level->Stop_CompositionCamera();
                m_bCameraUsed = false;
            }
        }
        else if (m_bCameraUsed)
        {
            level->Stop_CompositionCamera(!cameraEnabled);
            m_bCameraUsed = false;
        }
    }
    else m_bCameraUsed = false;
}

void Client::CKoukuSaydonPresentationPlayer::Update_FearPresentation(float dt,
    const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players)
{
    const auto stopFear = [this]() { Stop_Session(m_FearSession); m_FearSession.key.clear(); };
    const auto localId = CNetworkManager::Get().Get_LocalEntityId();
    const auto player = std::find_if(players.begin(), players.end(), [&](const auto& view)
        { return view.Snapshot.iNetEntityId == localId; });
    const auto* arena = CLevel_KakulSaydonArena::Get_Active();
    if (!arena || !localId || player == players.end() || !player->Snapshot.iCurrentHp ||
        player->Snapshot.eAction != LostArk::Shared::PLAYER_ACTION_STATE::FEAR)
    { stopFear(); m_strCompletedFearKey.clear(); return; }
    const auto& snapshot = player->Snapshot;
    const auto key = "fear:" + snapshot.strFearPresentationId + ":" + std::to_string(snapshot.iActionStartTick);
    // A locally completed clock must not restart from the last, slightly older
    // Server snapshot while the expiry snapshot is still in flight.
    if (m_strCompletedFearKey == key) { stopFear(); return; }
    const auto rejectFear = [&](const std::string& reason)
    { stopFear(); m_strCompletedFearKey = key; m_strStatus = reason; };
    const auto presentation = m_FearPresentations.find(snapshot.strFearPresentationId);
    if (presentation == m_FearPresentations.end())
    { rejectFear("Fear presentation unavailable: " + snapshot.strFearPresentationId); return; }
    float seconds = 0.f;
    if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(arena->Get_PresentationServerTick(),
        snapshot.iActionStartTick, 30.f, seconds))
    { stopFear(); return; }
    if (m_FearSession.key != key) { stopFear(); m_FearSession.key = key; }
    const float clockMs = m_FearSession.lastClockMs < 0.f ? seconds * 1000.f :
        (std::max)(seconds * 1000.f, m_FearSession.lastClockMs + dt * 1000.f);
    const auto durationTicks = snapshot.iFearEndTick - snapshot.iActionStartTick;
    const float durationMs = (std::min)(float(presentation->second.durationMs), durationTicks * (1000.f / 30.f));
    if (clockMs >= durationMs) { stopFear(); m_strCompletedFearKey = key; return; }
    // Do not darken the scene until its declared character lights are usable.
    // Gameplay FEAR remains Server-owned even if this presentation is isolated.
    for (const auto& resource : presentation->second.document.PresentationResources)
    {
        if (resource.eKind == KIND::SCENE_PROFILE && !m_Profiles.Has_Profile(resource.strAssetId))
        { rejectFear("Fear scene profile unavailable: " + resource.strAssetId); return; }
        if (resource.eKind != KIND::LIGHT) continue;
        const auto* light = m_pLightResources ? m_pLightResources->Find_RuntimeResource(resource.strAssetId) : nullptr;
        if (!light) { rejectFear("Fear character light unavailable: " + resource.strAssetId); return; }
        if (m_LightPlayerPivots.empty()) { stopFear(); return; }
        for (const auto& pivot : m_LightPlayerPivots)
        {
            LIGHT_DESC desc{}; std::string reason;
            if (!CLightResourceCatalog::Try_BuildLightDesc(*light, pivot, 1.f, desc, reason))
            { rejectFear("Fear character light rejected: " + resource.strAssetId + "; " + reason); return; }
        }
    }
    float4x4_t pivot; XMStoreFloat4x4(&pivot, XMMatrixIdentity());
    Sample(m_FearSession, presentation->second.document, presentation->second.pattern,
        clockMs, false, pivot, nullptr);
}

void Client::CKoukuSaydonPresentationPlayer::Update(float dt,
    const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
    const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players)
{
    if (!std::isfinite(dt) || dt < 0.f) return;
    m_LightPlayerPivots.clear();
    m_LightBossFollowers.clear();
    for (const auto& view : bosses)
    {
        if (!view.iOwnerBossNetEntityId || !view.Snapshot.iCurrentHp || view.pNpc.expired() || view.strArchetypeId.empty()) continue;
        const auto owner = std::find_if(bosses.begin(), bosses.end(), [&](const auto& candidate)
        {
            return candidate.Snapshot.iNetEntityId == view.iOwnerBossNetEntityId &&
                !candidate.iOwnerBossNetEntityId && candidate.Snapshot.iCurrentHp && !candidate.pNpc.expired() &&
                candidate.strArchetypeId == view.strArchetypeId;
        });
        if (owner != bosses.end()) m_LightBossFollowers[view.iOwnerBossNetEntityId].push_back(
            {view.Snapshot.iNetEntityId, view.pNpc});
    }
    for (const auto& view : players)
    {
        const auto character = view.pCharacter.lock();
        if (character && character->Get_Transform())
            m_LightPlayerPivots.push_back(*character->Get_Transform()->Get_WorldMatrixPtr());
    }
    if (!m_bProductAttempted) { std::string status; (void)Reload_Product(status); }
    const auto* arena = CLevel_KakulSaydonArena::Get_Active();
    const auto* run = arena ? &arena->Get_KoukuBundleState() : nullptr;
    using RUN_STATE = LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE;
    const bool runLive = run && run->iRunEpoch &&
        (run->eState == RUN_STATE::PENDING || run->eState == RUN_STATE::ACTIVE || run->eState == RUN_STATE::PATTERN_COMPLETED);
    m_ProductBundleSession.runEpoch = runLive && !run->strBundleId.empty() ? run->iRunEpoch : 0u;
    if (runLive && m_iProductReloadRunEpoch != run->iRunEpoch)
    {
        m_iProductReloadRunEpoch = run->iRunEpoch;
        if (auto* level = CLevel_KakulSaydonArena::Get_Active())
        { std::string cameraStatus; if (!level->Reload_PublishedCameraShots(cameraStatus)) m_strStatus = cameraStatus; }
        if (run->iPinnedSourceRevision != m_iProductSourceRevision)
        { std::string status; (void)Reload_Product(status, run->iPinnedSourceRevision); }
    }
    if (runLive && !run->strBundleId.empty())
    {
        const auto product = m_ProductBundles.find(run->strBundleId);
        float seconds = 0.f;
        if (product == m_ProductBundles.end() || run->iPinnedSourceRevision != m_iProductSourceRevision)
        {
            Stop_Session(m_ProductBundleSession);
            m_strStatus = "Replicated bundle Product/source revision is unavailable: " + run->strBundleId;
        }
        else if (CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
            (std::max)(run->iServerTick, arena->Get_PresentationServerTick()), run->iCommonStartTick, 30.f, seconds))
        {
            const auto key = "bundle-product:" + std::to_string(run->iRunEpoch) + ":" + run->strBundleId;
            if (m_ProductBundleSession.key != key)
            { Stop_Session(m_ProductBundleSession); m_ProductBundleSession.key = key; }
            const float clock = m_ProductBundleSession.lastClockMs < 0.f ? seconds * 1000.f :
                (std::max)(seconds * 1000.f, m_ProductBundleSession.lastClockMs + dt * 1000.f);
            float4x4_t pivot; XMStoreFloat4x4(&pivot, XMMatrixIdentity());
            Sample(m_ProductBundleSession, product->second.common.document, product->second.common.pattern,
                clock, false, pivot, nullptr);
        }
        else Stop_Session(m_ProductBundleSession);
    }
    else Stop_Session(m_ProductBundleSession);
    std::set<std::uint32_t> liveMarioEntries;
    if (runLive && run->iPinnedSourceRevision == m_iProductSourceRevision)
        for (const auto& member : run->Members)
        {
            if (member.strMarioEntryPatternId.empty()) continue;
            const auto root = m_Product.find(member.strMarioEntryPatternId);
            if (root == m_Product.end()) continue;
            float seconds = 0.f;
            if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
                (std::max)(run->iServerTick, arena->Get_PresentationServerTick()), member.iMarioEntryStartTick, 30.f, seconds)) continue;
            liveMarioEntries.insert(member.iBossNetEntityId);
            auto& session = m_MarioEntrySessions[member.iBossNetEntityId];
            const auto key = std::to_string(run->iRunEpoch) + ":" + member.strMemberId + ":" + std::to_string(member.iMarioEntryStartTick);
            if (session.key != key) { Stop_Session(session); session.key = key; }
            session.runEpoch = run->iRunEpoch; session.memberId = member.strMemberId;
            const auto pivot = XMMatrixRotationY(XMConvertToRadians(member.fMarioEntryYawDegrees)) *
                XMMatrixTranslation(member.fMarioEntryX, member.fMarioEntryY, member.fMarioEntryZ);
            float4x4_t storedPivot; XMStoreFloat4x4(&storedPivot, pivot);
            // Keep the authored entry frame while child animations run. Its
            // lifetime and terminal stop come only from persistent Server state.
            const float clock = (std::min)(seconds * 1000.f, float(member.iMarioEntryHoldMs));
            Sample(session, root->second.document, root->second.pattern, clock,
                seconds * 1000.f >= member.iMarioEntryHoldMs, storedPivot, nullptr);
        }
    for (auto it = m_MarioEntrySessions.begin(); it != m_MarioEntrySessions.end();)
        if (liveMarioEntries.contains(it->first)) ++it;
        else { Stop_Session(it->second); it = m_MarioEntrySessions.erase(it); }
    std::set<std::uint32_t> liveBosses;
    for (const auto& view : bosses)
    {
        if (runLive && run->iPinnedSourceRevision != m_iProductSourceRevision) continue;
        const auto npc = view.pNpc.lock();
        const auto product = m_Product.find(view.Snapshot.strPatternId);
        if (m_bProductLoaded && !view.Snapshot.strPatternId.empty() && product == m_Product.end() &&
            m_MissingProductPatterns.insert(view.Snapshot.strPatternId).second)
        {
            m_strStatus = "Snapshot pattern has no Product presentation: " + view.Snapshot.strPatternId;
            OutputDebugStringA(("[KoukuSaydonPresentationPlayer] " + m_strStatus + "\n").c_str());
        }
        if (!npc || !npc->Get_Transform() || product == m_Product.end() || !view.Snapshot.iCurrentHp) continue;
        float seconds = 0.f;
        if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(view.iServerTick,
            view.Snapshot.iPatternStartTick, 30.f, seconds)) continue;
        const auto id = view.Snapshot.iNetEntityId;
        if (liveMarioEntries.contains(id) && std::any_of(run->Members.begin(), run->Members.end(), [&](const auto& member) {
            return member.iBossNetEntityId == id && member.strMarioEntryPatternId == view.Snapshot.strPatternId;
        })) continue;
        liveBosses.insert(id);
        SESSION& session = m_BossSessions[id];
        session.runEpoch = 0u; session.memberId.clear();
        if (runLive)
            for (const auto& member : run->Members)
                if (member.iBossNetEntityId == id)
                { session.runEpoch = run->iRunEpoch; session.memberId = member.strMemberId; break; }
        const std::string key = std::to_string(id) + ":" + view.Snapshot.strPatternId + ":" +
            std::to_string(view.Snapshot.iPatternSequence) + ":" + std::to_string(view.Snapshot.iPatternStartTick);
        if (session.key != key) { Stop_Session(session); session.key = key; }
        // Interpolate between snapshots without rewinding/restarting effects every network tick.
        const float clock = session.lastClockMs < 0.f ? seconds * 1000.f :
            (std::max)(seconds * 1000.f, session.lastClockMs + dt * 1000.f);
        ANIMATION_MODEL_TARGET_VIEW weaponView;
        const bool hasWeapon = npc->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
        Sample(session, product->second.document, product->second.pattern,
            (std::min)(clock, float(product->second.durationMs)), false,
            *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model(), hasWeapon ? &weaponView : nullptr);
    }
    for (auto session = m_BossSessions.begin(); session != m_BossSessions.end();)
    {
        if (liveBosses.contains(session->first)) { ++session; continue; }
        Stop_Session(session->second);
        session = m_BossSessions.erase(session);
    }
    std::set<std::uint32_t> liveCards;
    for (const auto& view : players)
    {
        const auto character = view.pCharacter.lock();
        const std::string asset = Card_Asset(view.Snapshot);
        if (!character || !character->Get_Transform() || asset.empty()) continue;
        const auto id = view.Snapshot.iNetEntityId;
        liveCards.insert(id);
        CARD& card = m_Cards[id];
        if (card.assetId != asset)
        {
            if (card.handle) CEffectV2Runtime::Stop_Group(card.handle);
            card = {};
            card.assetId = asset;
            std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
            if (Ensure_EffectResource("GROUP", asset, effects))
            {
                if (const auto* source = effects->Find_Group(asset))
                {
                    EFFECT_V2_GROUP group = *source;
                    group.iDurationMs = 0u;
                    for (auto& child : group.Children)
                    {
                        child.vOffset.z = 0.f;
                        child.LocalTransform.vTranslation.z = 0.f;
                    }
                    EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                    XMStoreFloat4x4(&playback.PivotWorld, XMMatrixIdentity());
                    playback.fDurationSeconds = 0.f;
                    playback.bProductOwned = true;
                    card.handle = CEffectV2Runtime::Play_Group(group, effects,
                        playback, m_Device, m_Context);
                }
                if (!card.handle) m_strStatus = "Assigned card effect unavailable: " + asset;
            }
        }
        if (card.handle)
        {
            const matrix_t world = XMLoadFloat4x4(character->Get_Transform()->Get_WorldMatrixPtr());
            vector_t position = world.r[3];
            const auto body = character->Get_BodyModel();
            if (body && body->Has_Bone("bip001-head"))
            {
                position = (body->Get_BoneMatrix("bip001-head") * world).r[3];
                position = XMVectorSetY(position, XMVectorGetY(position) + 0.65f - 2.f);
            }
            // Both card leaves already contribute +2m Y. Keep their billboard vertical.
            float4x4_t pivot;
            XMStoreFloat4x4(&pivot, XMMatrixTranslationFromVector(position));
            CEffectV2Runtime::Set_GroupPivot(card.handle, pivot);
            std::string failure;
            if (CEffectV2Runtime::Consume_GroupFailure(card.handle, failure))
            {
                CEffectV2Runtime::Stop_Group(card.handle);
                card.handle = 0;
                m_strStatus = std::move(failure);
            }
        }
    }
    for (auto card = m_Cards.begin(); card != m_Cards.end();)
    {
        if (liveCards.contains(card->first)) { ++card; continue; }
        if (card->second.handle) CEffectV2Runtime::Stop_Group(card->second.handle);
        card = m_Cards.erase(card);
    }
    Update_MazeMarks(players);
    Update_FearPresentation(dt, players);
    if (m_bPreviewPlaying && m_bOwnPreviewClock && m_bPreviewPivotReady)
    {
        // Begin runs after this Update; the next delta includes synchronous WORLD
        // preparation. Arm the new clock once without charging that setup time.
        if (!Prepare_PreviewEffects())
        { m_bPreviewClockAwaitingFirstUpdate = true; Refresh_SharedPresentation(); return; }
        const bool advanceClock = !m_bPreviewClockAwaitingFirstUpdate;
        m_bPreviewClockAwaitingFirstUpdate = false;
        if (advanceClock && !m_bPreviewPaused && !m_bModelReferencePreview && !m_bPreviewCaptureClockHeld)
            m_fPreviewClockMs += double(dt) * 1000.0;
        if (!m_bPreviewPaused && !m_bModelReferencePreview && m_fPreviewClockMs >= m_iPreviewDurationMs)
        {
            if (m_bColliderResourcePreview)
            {
                m_fPreviewClockMs = m_iPreviewDurationMs - 1u;
                Pause_Preview(true);
            }
            else
            {
                // MainApp applies this frame's Pause/Seek/Stop before consuming completion.
                // Keep the real clock and borrowed actors alive until that decision.
                m_fPreviewClockMs = m_iPreviewDurationMs;
                m_strCompletedPreviewPatternId = Preview_IsBundle() ? m_PreviewBundleId : m_PreviewPattern.strPatternId;
            }
        }
        // Bundle members own their independent WORLD players, sampled before effects.
        if (Preview_IsBundle() && m_strCompletedPreviewPatternId.empty()) Sample_BundlePreview();
        // MainApp samples single-pattern WORLD first, then its presentation.
    }
    Refresh_SharedPresentation();
}

bool Client::CKoukuSaydonPresentationPlayer::Begin_Preview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, bool ownClock,
    std::uint32_t clockMs, bool paused, std::string& status)
{
    const auto duration = Pattern_Duration(pattern);
    if (!pattern.strLoadError.empty() || !duration)
    {
        status = "Presentation preview needs a valid finite pattern.";
        return false;
    }
    for (const auto& box : pattern.PresentationOccurrences)
    {
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&box](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (resource == document.PresentationResources.end())
        {
            status = "Preview names an unknown presentation resource: " + box.strResourceId;
            return false;
        }
        if (resource->eKind == KIND::EFFECT && !Validate_EffectAnchor(document, pattern, box, status)) return false;
    }
    if (pattern.strPatternId == "preview.kouku.resource" && pattern.WorldOccurrences.empty() &&
        pattern.PresentationOccurrences.size() == 1u)
    {
        const auto& box = pattern.PresentationOccurrences.front();
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (resource->eKind == KIND::EFFECT &&
            (resource->strResourceKind == "V1_EFFECT" || resource->strResourceKind == "V1_ELEMENT"))
        {
            // A Resource has its own animation origin. Prepare its saved actor
            // through the existing single-member model/presentation owner.
            const auto effect = CEffectCatalog::Find(resource->strAssetId);
            if (!effect) { status = CEffectCatalog::Get_Status(); return false; }
            if (effect->SourceModelPreview)
            {
                CEffectCompositionModelPreview source;
                if (!source.Select_SourceEffect(*effect)) { status = source.Status(); return false; }
                auto stagedDocument = document;
                auto sourcePattern = source.Get_Document().Patterns.front();
                sourcePattern.strPatternId = pattern.strPatternId + ".actor";
                sourcePattern.PresentationOccurrences = pattern.PresentationOccurrences;
                // The Effect window owns lifetime; the native final pose is held
                // across particle tails by the same CModel source sampler.
                sourcePattern.iDurationMs = duration;
                sourcePattern.Stages.front().iDurationMs = duration;
                auto& animations = sourcePattern.Stages.front().AnimationOccurrences;
                std::erase_if(animations, [&](const auto& animation) { return animation.iStartOffsetMs >= duration; });
                for (auto& animation : animations)
                {
                    animation.iPoseStartMs = animation.iStartOffsetMs;
                    animation.iPlayMs = (std::min)(animation.iPlayMs, duration - animation.iStartOffsetMs);
                }
                const auto sourceId = sourcePattern.strPatternId;
                const auto gateId = sourcePattern.strGateId;
                stagedDocument.Patterns.push_back(std::move(sourcePattern));
                KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
                bundle.strBundleId = pattern.strPatternId; bundle.strGateId = gateId;
                bundle.Members.push_back({pattern.strPatternId + ".member", sourceId, 0u});
                stagedDocument.Bundles.push_back(std::move(bundle));
                if (!Begin_BundlePreview(stagedDocument, pattern.strPatternId, clockMs, paused, status)) return false;
                status = m_strStatus = "Effect Resource source model and animation ready: " + resource->strAssetId;
                return true;
            }
        }
        const bool bossLight = resource->eKind == KIND::LIGHT && box.strAnchorKind == "BOSS";
        if (bossLight && (pattern.strActorProfileId.empty() || pattern.strGateId.empty() || pattern.strTargetBossPlacementId.empty()))
        { status = "Boss Light Preview requires a selected Pattern with an exact Gate and boss target."; return false; }
        if ((resource->eKind == KIND::EFFECT || bossLight) && !pattern.strActorProfileId.empty() &&
            !pattern.strGateId.empty() && !pattern.strTargetBossPlacementId.empty())
        {
            // Reuse the selected boss actor owner for a Resource without source
            // animation metadata. Its existing idle pose needs no invented clip.
            auto stagedDocument = document;
            auto sourcePattern = pattern;
            sourcePattern.strPatternId = pattern.strPatternId + ".actor";
            sourcePattern.iDurationMs = duration;
            const auto sourceId = sourcePattern.strPatternId;
            stagedDocument.Patterns.push_back(std::move(sourcePattern));
            KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
            bundle.strBundleId = pattern.strPatternId; bundle.strGateId = pattern.strGateId;
            bundle.Members.push_back({pattern.strPatternId + ".member", sourceId, 0u});
            stagedDocument.Bundles.push_back(std::move(bundle));
            if (!Begin_BundlePreview(stagedDocument, pattern.strPatternId, clockMs, paused, status)) return false;
            status = m_strStatus = "Resource selected boss ready: " + resource->strAssetId +
                " | " + pattern.strGateId + " | " + pattern.strTargetBossPlacementId;
            return true;
        }
    }
    // Stage first. A rejected preview request preserves the active session.
    auto stagedDocument = document;
    auto stagedPattern = pattern;
    Stop_Preview();
    m_PreviewDocument = std::move(stagedDocument);
    m_PreviewPattern = std::move(stagedPattern);
    m_PreviewSession.key = "preview:" + pattern.strPatternId;
    m_iPreviewDurationMs = duration;
    m_fPreviewClockMs = (std::min)(clockMs, duration);
    m_bOwnPreviewClock = ownClock;
    m_bPreviewClockAwaitingFirstUpdate = ownClock;
    m_bPreviewPlaying = true;
    m_bPreviewPaused = paused;
    m_bColliderResourcePreview = pattern.strPatternId == "preview.kouku.resource" &&
        pattern.PresentationOccurrences.size() == 1u &&
        std::any_of(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&pattern](const auto& resource) { return resource.eKind == KIND::COLLIDER &&
                resource.strResourceId == pattern.PresentationOccurrences.front().strResourceId; });
    m_EffectResources.clear();
    m_EffectResourceFailures.clear();
    status = "Presentation preview ready: " + pattern.strPatternId;
    m_strStatus = status;
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Release_BundlePreviewMembers(
    std::vector<BUNDLE_PREVIEW_MEMBER>& members)
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    for (auto& member : members)
    {
        if (member.rootMotion)
        {
            member.rootMotion.reset();
            if (member.actor) (void)member.actor->Apply_NetworkState(member.initialPosition, member.initialYawDegrees);
        }
        else if (member.actor && member.actor->Get_Model())
            (void)member.actor->Get_Model()->Set_RootMotionVerticalScale(1.f);
        Stop_Session(member.session);
        if (level)
        {
            const auto targets = level->Get_CompositionWorldTargets();
            for (auto& [id, player] : member.session.previewWorlds) player->Stop_All(targets, true);
            level->Release_CompositionPreviewActor(member.actor);
        }
        else if (member.actor)
            CGameInstance::Get().Remove_GameObject_from_Layer(ETOUI(LEVEL::KAKULSAYDON_ARENA),
                L"Layer_KoukuCompositionPreview", member.actor);
    }
    members.clear();
}

bool Client::CKoukuSaydonPresentationPlayer::Begin_BundlePreview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, const std::string& bundleId,
    std::uint32_t clockMs, bool paused, std::string& status, const CWorldSequenceDocument* sourceDocument,
    const bool automaticRootMotion, bool externalWorldPreview)
{
    const auto bundle = std::find_if(document.Bundles.begin(), document.Bundles.end(),
        [&](const auto& value) { return value.strBundleId == bundleId; });
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level || bundle == document.Bundles.end() || !bundle->strLoadError.empty() ||
        bundle->Members.empty() || bundle->Members.size() > LostArk::Shared::MAX_KOUKUSAYDON_BUNDLE_MEMBERS)
    { status = "Bundle preview requires an active arena and a valid nonempty bundle."; return false; }
    if (externalWorldPreview && (bundle->Members.size() != 1u || bundle->Members.front().iStartOffsetMs))
    { status = "Level WORLD preview requires one Pattern on the common clock."; return false; }
    std::vector<BUNDLE_PREVIEW_MEMBER> staged;
    std::vector<CWorldSequencePlayer*> stagedWorldPlayers;
    KOUKU_SAYDON_COMPOSITION_PATTERN common;
    common.strPatternId = bundleId;
    common.PresentationOccurrences = bundle->PresentationOccurrences;
    common.SceneProfileOccurrences = bundle->SceneProfileOccurrences;
    std::uint64_t duration = Pattern_Duration(common);
    std::set<std::string> targetsUsed, memberIds, placementBindings;
    const auto targets = level->Get_CompositionWorldTargets();
    const auto& sequences = sourceDocument ? *sourceDocument : level->Get_WorldSequenceDocument();
    const auto fail = [&](const std::string& reason)
    { Release_BundlePreviewMembers(staged); status = reason; return false; };
    std::vector<PRESENTATION_WINDOW> globalWindows;
    for (const auto& row : common.SceneProfileOccurrences)
        (void)Admit_PresentationWindow(globalWindows, KIND::SCENE_PROFILE, row.iStartMs,
            double(row.iStartMs) + row.iDurationMs, "common");
    for (const auto& box : common.PresentationOccurrences)
    {
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (resource == document.PresentationResources.end() ||
            (resource->eKind != KIND::CAMERA && resource->eKind != KIND::SCENE_PROFILE))
            return fail("Bundle common lane only accepts Camera or Scene Profile resources.");
        (void)Admit_PresentationWindow(globalWindows, resource->eKind, box.iStartMs,
            double(box.iStartMs) + box.iDurationMs + Camera_ReturnMs(*resource, true), "common");
    }
    for (const auto& sourceMember : bundle->Members)
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == sourceMember.strPatternId; });
        if (source == document.Patterns.end() || !source->strLoadError.empty() ||
            source->strGateId != bundle->strGateId || source->strTargetBossPlacementId.empty() ||
            !targetsUsed.insert(source->strTargetBossPlacementId).second ||
            !memberIds.insert(sourceMember.strMemberId).second || sourceMember.strMemberId.empty())
            return fail("Bundle has an invalid, duplicate, or cross-Gate target/member.");
        const double offset = std::ceil(double(sourceMember.iStartOffsetMs) * 30.0 / 1000.0) * 1000.0 / 30.0;
        for (const auto& row : source->SceneProfileOccurrences)
            if (!Admit_PresentationWindow(globalWindows, KIND::SCENE_PROFILE, offset + row.iStartMs,
                offset + row.iStartMs + row.iDurationMs, sourceMember.strMemberId))
                return fail("Bundle global Scene Profile windows overlap.");
        for (const auto& box : source->PresentationOccurrences)
        {
            const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                [&](const auto& value) { return value.strResourceId == box.strResourceId; });
            if (resource == document.PresentationResources.end()) return fail("Unknown child presentation resource.");
            if (resource->eKind == KIND::EFFECT && !Validate_EffectAnchor(document, *source, box, status))
                return fail(status);
            if ((resource->eKind == KIND::CAMERA || resource->eKind == KIND::SCENE_PROFILE) &&
                !Admit_PresentationWindow(globalWindows, resource->eKind, offset + box.iStartMs,
                    offset + box.iStartMs + box.iDurationMs + Camera_ReturnMs(*resource, true), sourceMember.strMemberId))
                return fail("Bundle global presentation windows overlap.");
        }
        staged.emplace_back();
        auto& member = staged.back();
        member.memberId = sourceMember.strMemberId;
        member.offsetTicks = static_cast<std::uint32_t>((std::uint64_t(sourceMember.iStartOffsetMs) * 30u + 999u) / 1000u);
        member.pattern = *source;
        if (!CKoukuSaydonCompositionDocument::Try_ResolveAnimationBlendWindows(document, *source,
            member.pattern.AnimationBlendWindows, status)) return fail(status);
        member.durationMs = Pattern_Duration(*source);
        if (!member.durationMs) return fail("Empty child pattern cannot be previewed.");
        duration = (std::max)(duration, (std::uint64_t(member.offsetTicks) * 1000u + 29u) / 30u + member.durationMs);
        if (duration > MAX_TIMELINE_MS) return fail("Bundle preview exceeds the timeline duration limit.");
        if (!level->Create_CompositionPreviewActor(*source, member.actor, status)) return fail(status);
        const auto model = member.actor->Get_Model();
        if (!CKoukuSaydonAnimationBlend::Validate_ModelWindows(*model, member.pattern.AnimationBlendWindows, status))
            return fail(status);
        member.initialAnimation = model->Get_CurrentAnimIndex();
        const auto& initialRoot = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
        member.initialYawDegrees = XMConvertToDegrees(std::atan2(initialRoot._31, initialRoot._33));
        member.initialPosition = {initialRoot._41, initialRoot._42, initialRoot._43};
        float ignored = 0.f;
        model->Get_AnimationProgress(member.initialAnimation, member.initialTicks, ignored);
        std::uint32_t stageStart = 0u;
        for (const auto& stage : source->Stages)
        {
            for (auto box : stage.AnimationOccurrences)
            {
                bool found = false;
                for (std::uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
                    if (const auto* name = model->Get_AnimationName(i); name && box.strRuntimeClip == name)
                    { found = true; break; }
                if (!found || !box.iPlayMs || !std::isfinite(box.fPlayRate) || box.fPlayRate <= 0.f)
                    return fail("Bundle child animation is unavailable: " + box.strRuntimeClip);
                box.iPoseStartMs = stageStart + (box.strOccurrenceId == std::min_element(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
                [](const auto& a, const auto& b) { return a.iStartOffsetMs != b.iStartOffsetMs ?
                    a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; })->strOccurrenceId ? 0u : box.iStartOffsetMs);
                box.iStartOffsetMs += stageStart;
                member.animations.push_back(std::move(box));
            }
            stageStart += stage.iDurationMs;
        }
        std::sort(member.animations.begin(), member.animations.end(), [](const auto& a, const auto& b)
            { return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; });
        if (automaticRootMotion && !member.animations.empty() &&
            CKoukuSaydonPreviewRootMotion::Allows_AutomaticMotion(document, *source))
        {
            member.rootMotion = std::make_unique<CKoukuSaydonPreviewRootMotion>();
            if (!member.rootMotion->Prepare(model, member.animations,
                    float(source->fAnimationRootVerticalScale), status) ||
                !member.rootMotion->Begin_Suppression())
                return fail("Bundle child root motion: " + status);
        }
        for (const auto& box : source->WorldOccurrences)
        {
            if (externalWorldPreview) continue;
            const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == box.strWorldId; });
            if (world == document.Worlds.end()) return fail("Bundle child WORLD resource is missing.");
            const auto* instance = sequences.Find_Instance(world->strSequenceInstanceId);
            if (!instance) return fail("Bundle WORLD sequence is missing: " + world->strSequenceInstanceId);
            if (!level->Can_StartCompositionWorld(world->strSequenceInstanceId, status, &sequences)) return fail(status);
            for (const auto& binding : instance->bindings)
                if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE &&
                    !placementBindings.insert(std::to_string(static_cast<int>(binding.targetKind)) + ":" + binding.targetId).second)
                    return fail("Bundle WORLD members share a mutable map/deploy target.");
            auto player = std::make_shared<CWorldSequencePlayer>();
            const auto [entry, inserted] = member.session.previewWorlds.emplace(box.strOccurrenceId, player);
            if (!inserted) return fail("Bundle WORLD occurrence is duplicated: " + box.strOccurrenceId);
            stagedWorldPlayers.push_back(entry->second.get());
            float3_t offset(float(world->PositionOffset[0]), float(world->PositionOffset[1]), float(world->PositionOffset[2]));
            if (!box.Placement && world->strAnchorKind == "BOSS_SPAWN")
            {
                const auto& pivot = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
                offset.x += pivot._41 - float(world->AnchorPosition[0]);
                offset.y += pivot._42 - float(world->AnchorPosition[1]);
                offset.z += pivot._43 - float(world->AnchorPosition[2]);
            }
            member.worldOffsets.emplace(box.strOccurrenceId, offset);
        }
        member.session.key = "bundle-preview:" + bundleId + ":" + member.memberId;
    }
    if (!stagedWorldPlayers.empty() &&
        !CWorldSequencePlayer::Set_DocumentBatch(sequences, targets, stagedWorldPlayers, status)) return fail(status);
    for (auto& member : staged)
        for (const auto& box : member.pattern.WorldOccurrences)
        {
            if (externalWorldPreview) continue;
            const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == box.strWorldId; });
            auto& player = *member.session.previewWorlds.at(box.strOccurrenceId);
            if (!player.Prepare_InstanceResources(world->strSequenceInstanceId, targets))
                return fail("Bundle WORLD " + box.strOccurrenceId + ": " + player.Get_Status());
            if (!player.Validate_ObjectPlacement(world->strSequenceInstanceId, WorldPlacementFromOccurrence(box), status))
                return fail(status);
            const auto worldSpan = player.Get_InstanceElapsedSpanMs(
                world->strSequenceInstanceId, box.fPlaybackSpeed, box.iDurationMs);
            if (worldSpan <= 0.f) return fail("Bundle WORLD has no finite presentation span.");
            duration = (std::max)(duration, (std::uint64_t(member.offsetTicks) * 1000u + 29u) / 30u +
                box.iStartMs + static_cast<std::uint64_t>(std::ceil(worldSpan)));
            if (duration > MAX_TIMELINE_MS) return fail("Bundle WORLD tail exceeds the timeline duration limit.");
        }
    // All models, clips and WORLD inputs are prepared before replacing the live preview.
    auto stagedDocument = document;
    Stop_Preview();
    m_PreviewDocument = std::move(stagedDocument);
    m_PreviewPattern = std::move(common);
    m_PreviewBundleId = bundleId;
    m_bBundleWorldExternal = externalWorldPreview;
    m_BundlePreviewMembers = std::move(staged);
    m_PreviewSession.key = "bundle-preview:" + bundleId + ":common";
    m_iPreviewDurationMs = static_cast<std::uint32_t>(duration);
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_bOwnPreviewClock = m_bPreviewPlaying = m_bPreviewPivotReady = true;
    m_bPreviewClockAwaitingFirstUpdate = true;
    m_bPreviewPaused = paused;
    XMStoreFloat4x4(&m_PreviewPivot, XMMatrixIdentity());
    if (!externalWorldPreview) Sample_BundlePreview();
    if (!m_bPreviewPlaying) { status = m_strStatus; return false; }
    Refresh_SharedPresentation();
    status = m_strStatus = "Bundle preview ready: " + bundleId;
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Begin_ModelReferencePreview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, const std::string& selectionId,
    const bool isBundle, const std::uint32_t clockMs, const bool paused, std::string& status)
{
    KOUKU_SAYDON_COMPOSITION_DOCUMENT reference;
    reference.iRevision = document.iRevision;
    reference.strAreaId = document.strAreaId;
    KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
    if (isBundle)
    {
        const auto source = std::find_if(document.Bundles.begin(), document.Bundles.end(),
            [&](const auto& value) { return value.strBundleId == selectionId; });
        if (source == document.Bundles.end() || !source->strLoadError.empty())
        { status = "Saved model-reference Bundle is missing or invalid."; return false; }
        bundle = *source;
    }
    else
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == selectionId; });
        if (source == document.Patterns.end())
        { status = "Saved model-reference Pattern is missing."; return false; }
        // Runtime-only grouping lets one Pattern use the same staged actor path.
        // It is neither a second saved Pattern nor a writable authoring document.
        bundle.strBundleId = "effect.model.reference:" + selectionId;
        bundle.strGateId = source->strGateId;
        bundle.Members.push_back({selectionId, selectionId, 0u});
    }
    bundle.PresentationOccurrences.clear();
    bundle.SceneProfileOccurrences.clear();
    for (const auto& member : bundle.Members)
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == member.strPatternId; });
        if (source == document.Patterns.end() || !source->strLoadError.empty())
        {
            status = "Model-reference child is missing or invalid: " + member.strPatternId;
            if (source != document.Patterns.end()) status += ". " + source->strLoadError;
            return false;
        }
        auto& pattern = reference.Patterns.emplace_back(*source);
        pattern.BossMotion.reset();
        pattern.fAnimationRootVerticalScale = 1.0;
        for (auto& stage : pattern.Stages) stage.bRetargetOnEnter = false;
        pattern.LogicOccurrences.clear();
        pattern.SummonOccurrences.clear();
        pattern.WorldOccurrences.clear();
        pattern.SceneProfileOccurrences.clear();
        pattern.PresentationOccurrences.clear();
    }
    const std::string bundleId = bundle.strBundleId;
    reference.Bundles.push_back(std::move(bundle));
    if (!Begin_BundlePreview(reference, bundleId, clockMs, paused, status, nullptr, false)) return false;
    m_bModelReferencePreview = true;
    status = m_strStatus = "Model reference ready. Master cursor owns time; actors stay at authored spawn (no Server movement replay).";
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Sample_ModelReferencePreview(
    const std::uint32_t clockMs, const bool paused)
{
    if (!m_bModelReferencePreview || !m_bPreviewPlaying) return;
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_bPreviewPaused = paused;
    Sample_BundlePreview();
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_ModelReferenceTarget(
    const std::string& memberId, EFFECT_V2_TARGET& target, EFFECT_V2_TARGET_VIEW& view) const
{
    if (!m_bModelReferencePreview || !m_bPreviewPlaying) return false;
    const BUNDLE_PREVIEW_MEMBER* selected = nullptr;
    if (memberId.empty() && m_BundlePreviewMembers.size() == 1u)
        selected = &m_BundlePreviewMembers.front();
    else
        for (const auto& member : m_BundlePreviewMembers)
            if (member.memberId == memberId) { selected = &member; break; }
    if (!selected || !selected->actor) return false;
    auto stagedTarget = EFFECT_V2_TARGET::From_Npc(selected->actor);
    stagedTarget.strArchetypeId = std::string(CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
        selected->pattern.strTargetBossPlacementId));
    EFFECT_V2_TARGET_VIEW stagedView;
    if (!CEffectV2Object::Resolve_TargetView(stagedTarget, stagedView)) return false;
    target = std::move(stagedTarget);
    view = std::move(stagedView);
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreviewFacing(
    BUNDLE_PREVIEW_MEMBER& member, const double localMs)
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    const auto player = level ? level->Get_LocalCharacter() : nullptr;
    const auto root = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
    float yaw = member.initialYawDegrees;
    std::vector<float> rowYaws(member.animations.size(), yaw);
    std::uint32_t stageStartMs = 0u;
    for (const auto& stage : member.pattern.Stages)
    {
        if (localMs < stageStartMs) break;
        if (stage.bRetargetOnEnter)
        {
            auto [sample, inserted] = member.stageFacingYawDegrees.try_emplace(stage.strStageId, yaw);
            if (inserted && player && player->Get_Transform())
            {
                float3_t stagePosition{root._41, root._42, root._43};
                if (member.rootMotion)
                {
                    float3_t preceding;
                    if (!member.rootMotion->Sample_Displacement(stageStartMs, rowYaws, preceding)) return false;
                    stagePosition = {member.initialPosition.x + preceding.x,
                        member.initialPosition.y + preceding.y, member.initialPosition.z + preceding.z};
                }
                const auto& target = *player->Get_Transform()->Get_WorldMatrixPtr();
                const float dx = target._41 - stagePosition.x, dz = target._43 - stagePosition.z;
                if (std::isfinite(dx) && std::isfinite(dz) && dx * dx + dz * dz > .000001f)
                {
                    sample->second = XMConvertToDegrees(std::atan2(dx, dz));
                    // Match the Server's measured catalog +X face/hammer forward for Big Saydon.
                    if (CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
                        member.pattern.strTargetBossPlacementId) == "BOSS_KAKULSAYDON_G2_BIG_SAYDON")
                        sample->second -= 90.f;
                }
            }
            // Reuse each stage's first facing on backward/forward scrubs.
            yaw = sample->second;
        }
        for (size_t i = 0u; i < member.animations.size(); ++i)
            if (member.animations[i].iPoseStartMs >= stageStartMs &&
                member.animations[i].iPoseStartMs < stageStartMs + stage.iDurationMs)
                rowYaws[i] = yaw;
        stageStartMs += stage.iDurationMs;
    }
    float3_t position{root._41, root._42, root._43};
    if (member.rootMotion)
    {
        float3_t displacement;
        if (!member.rootMotion->Sample_Displacement((std::clamp)(localMs, 0.0, double(member.durationMs)),
            rowYaws, displacement)) return false;
        position = {member.initialPosition.x + displacement.x,
            member.initialPosition.y + displacement.y, member.initialPosition.z + displacement.z};
    }
    return member.actor->Apply_NetworkState(position, yaw);
}

bool Client::CKoukuSaydonPresentationPlayer::Prepare_PreviewEffects()
{
    if (!m_bPreviewPreparationQueued)
    {
        std::set<std::string> targets;
        const auto collect = [&](const auto& pattern) {
            for (const auto& box : pattern.PresentationOccurrences)
                for (const auto& resource : m_PreviewDocument.PresentationResources)
                    if (resource.strResourceId == box.strResourceId && resource.eKind == KIND::EFFECT &&
                        (resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT"))
                        targets.insert(resource.strAssetId);
        };
        collect(m_PreviewPattern);
        for (const auto& member : m_BundlePreviewMembers) collect(member.pattern);
        m_PreviewPreparationTargets.assign(targets.begin(), targets.end());
        if (!m_PreviewPreparationTargets.empty())
        {
            std::vector<std::string> admitted;
            std::string status;
            if (!CEffectPresentationService::Queue_ProductTargets_Priority(m_PreviewPreparationTargets, admitted, status))
            { Fail_Preview("Preview Effect preparation could not queue: " + status); return false; }
        }
        m_bPreviewPreparationQueued = true;
    }
    if (m_PreviewPreparationTargets.empty()) return true;
    const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(m_PreviewPreparationTargets);
    // Failed targets are reported by their own occurrence. Preparation time
    // never consumes a short Effect window on the authoring clock.
    if (probe.bCatalogRevisionCurrent && probe.bSettled) return true;
    m_strStatus = "Preparing preview Effects; timeline is held at " + std::to_string(Preview_ClockMs()) + " ms.";
    return false;
}

void Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreview()
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level || !m_bPreviewPlaying || !Prepare_PreviewEffects()) return;
    std::uint32_t effectiveMs = Preview_ClockMs();
    if (!Resolve_PreviewCaptureClock(effectiveMs, effectiveMs))
    { const auto error = m_strStatus; Fail_Preview(error); return; }
    m_fPreviewClockMs = effectiveMs;
#ifdef _DEBUG
    if (m_bBundleWorldExternal)
    {
        std::string worldStatus;
        if (!level->Debug_SampleCompositionWorldPreview(m_PreviewBundleId, true, effectiveMs, worldStatus))
        { Fail_Preview(worldStatus); return; }
    }
#endif
    const auto targets = level->Get_CompositionWorldTargets();
    for (auto& member : m_BundlePreviewMembers)
    {
        const double localMs = m_fPreviewClockMs - double(member.offsetTicks) * 1000.0 / 30.0;
        const auto model = member.actor->Get_Model();
        (void)model->Set_RootMotionVerticalScale(member.rootMotion ? 0.f : float(member.pattern.fAnimationRootVerticalScale));
        const auto sampleMs = static_cast<float>((std::clamp)(localMs, 0.0, double(member.durationMs)));
        if (!Sample_BundlePreviewFacing(member, localMs))
        { Fail_Preview("Bundle root-motion sample or actor target is unavailable."); return; }
        std::array<double, 3u> bossPosition{};
        double bossYaw = 0.0;
        if (Sample_KoukuSaydonBossMotion(member.pattern, sampleMs, bossPosition, bossYaw))
            (void)member.actor->Apply_NetworkState(
                {float(bossPosition[0]), float(bossPosition[1]), float(bossPosition[2])}, float(bossYaw));
        const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* animation = nullptr;
        const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* previousAnimation = nullptr;
        for (const auto& box : member.animations)
            if (sampleMs >= box.iPoseStartMs && localMs >= 0.0)
            { previousAnimation = animation; animation = &box; }
        if (!animation && !member.animations.empty()) animation = &member.animations.front();
        std::uint32_t animationIndex = member.initialAnimation;
        float ticks = member.initialTicks;
        if (animation)
        {
            for (std::uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
                if (const auto* name = model->Get_AnimationName(i); name && animation->strRuntimeClip == name)
                { animationIndex = i; break; }
            float oldTicks = 0.f, clipTicks = 0.f;
            model->Get_AnimationProgress(animationIndex, oldTicks, clipTicks);
            const float tps = model->Get_AnimationTickPerSecond(animationIndex);
            const float elapsed = (std::max)(0.f, sampleMs - animation->iStartOffsetMs);
            const float age = (std::min)(elapsed, float(animation->iPlayMs));
            double sourceMs = 0.0;
            if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(animation->iSourceStartMs,
                animation->iSourceEndMs, age, animation->fPlayRate, clipTicks * 1000.0 / tps,
                animation->strEndPolicy == "LOOP_TO_WINDOW", sourceMs))
            { Fail_Preview("Animation source range is outside the native clip."); return; }
            ticks = float(sourceMs * tps / 1000.0);
        }
        model->Set_Animation(animationIndex, false, 0.f);
        model->Set_AnimPaused(true);
        model->Set_AnimTrackPosition(animationIndex, ticks);
        model->Update_Animation(0.f);
        CModel::ANIMATION_TRANSITION_POSE logicPose;
        bool logicActive = false;
        std::string blendStatus;
        if (!CKoukuSaydonAnimationBlend::Sample_Pose(*model, member.pattern.AnimationBlendWindows,
            sampleMs, logicPose, logicActive, blendStatus) ||
            (logicActive && !model->Set_AnimationTransitionPose(logicPose)))
        { Fail_Preview("Logic animation blend failed: " + blendStatus); return; }
        if (!logicActive && animation && animation->iBlendInMs && previousAnimation)
        {
            CModel::ANIMATION_TRANSITION_POSE pose;
            pose.targetIndex = animationIndex; pose.targetTicks = ticks;
            pose.durationSeconds = animation->iBlendInMs * .001f;
            pose.elapsedSeconds = (sampleMs - animation->iStartOffsetMs) * .001f;
            pose.playRate = animation->fPlayRate;
            for (uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
                if (previousAnimation->strRuntimeClip == model->Get_AnimationName(i)) { pose.sourceIndex = i; break; }
            float cursor = 0.f, sourceEnd = 0.f;
            if (pose.sourceIndex == UINT32_MAX || !model->Get_AnimationProgress(pose.sourceIndex, cursor, sourceEnd))
                m_strStatus = "Animation blend source clip is unavailable.";
            else
            {
                const float previousTps = model->Get_AnimationTickPerSecond(pose.sourceIndex);
                double previousMs = 0.0;
                if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(previousAnimation->iSourceStartMs,
                    previousAnimation->iSourceEndMs, previousAnimation->iPlayMs, previousAnimation->fPlayRate,
                    sourceEnd * 1000.0 / previousTps, previousAnimation->strEndPolicy == "LOOP_TO_WINDOW", previousMs))
                { Fail_Preview("Animation blend source range is outside the native clip."); return; }
                pose.sourceTicks = float(previousMs * previousTps / 1000.0);
                if (!model->Set_AnimationTransitionPose(pose)) m_strStatus = "Animation blend pose admission failed.";
            }
        }
        for (const auto& box : member.pattern.WorldOccurrences)
        {
            if (m_bBundleWorldExternal) continue;
            const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == box.strWorldId; });
            auto& player = member.session.previewWorlds.at(box.strOccurrenceId);
            auto worldTargets = targets;
            worldTargets.bossAnchor = [&member](const std::string& archetype, const std::string& bone,
                CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status)
            {
                if (archetype != CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(member.pattern.strTargetBossPlacementId) ||
                    !member.actor || !member.actor->Get_Transform())
                { status = "World Object Boss anchor does not match this preview actor: " + archetype; return false; }
                return CWorldSequencePlayer::Resolve_BossBoneAnchor(member.actor->Get_Model(),
                    *member.actor->Get_Transform()->Get_WorldMatrixPtr(), bone, out, status);
            };
            worldTargets.objectEmissionAnchor = Make_WorldEmissionAnchor(member.pattern, *world, box);
            const auto span = player->Get_InstanceElapsedSpanMs(world->strSequenceInstanceId,
                box.fPlaybackSpeed, box.iDurationMs);
            if (localMs < box.iStartMs || localMs >= double(box.iStartMs) + span)
            { player->Stop_All(worldTargets, true); continue; }
            if (!player->Is_Playing(world->strSequenceInstanceId) &&
                !player->Play(world->strSequenceInstanceId, worldTargets, box.fPlaybackSpeed,
                    member.worldOffsets.at(box.strOccurrenceId), box.iDurationMs, WorldPlacementFromOccurrence(box)))
            {
                Fail_Preview("Bundle WORLD failed: " + box.strOccurrenceId + ": " + player->Get_Status());
                return;
            }
            if (!player->Seek_InstanceToMs(world->strSequenceInstanceId, float(localMs - box.iStartMs), worldTargets))
            {
                Fail_Preview("Bundle WORLD failed: " + box.strOccurrenceId + ": " + player->Get_Status());
                return;
            }
        }
        member.actor->Synchronize_WeaponPose();
        ANIMATION_MODEL_TARGET_VIEW weaponView;
        const bool hasWeapon = member.actor->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
        if (localMs < 0.0 || localMs >= member.durationMs) Stop_Session(member.session);
        else Sample(member.session, m_PreviewDocument, member.pattern, sampleMs, m_bPreviewPaused,
            *member.actor->Get_Transform()->Get_WorldMatrixPtr(), model, hasWeapon ? &weaponView : nullptr);
    }
    Sample(m_PreviewSession, m_PreviewDocument, m_PreviewPattern,
        float(m_fPreviewClockMs), m_bPreviewPaused, m_PreviewPivot, nullptr);
    if (m_bPreviewCaptureClockHeld && Preview_ClockMs() == m_iPreviewCaptureBoundaryMs)
        m_bPreviewCaptureBoundarySampled = true;
}

void Client::CKoukuSaydonPresentationPlayer::Set_PreviewPivot(
    const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model)
{
    m_PreviewPivot = pivot;
    m_PreviewModel = model;
    m_bPreviewPivotReady = true;
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_PreviewCaptureClock(
    std::uint32_t requestedMs, std::uint32_t& effectiveMs)
{
    effectiveMs = requestedMs;
    if (!m_bPreviewPlaying) return true;
    if (!Prepare_PreviewEffects()) { effectiveMs = Preview_ClockMs(); return m_bPreviewPlaying; }
    const bool wasHeld = m_bPreviewCaptureClockHeld;
    if (wasHeld) requestedMs = m_iPreviewCaptureResumeMs;
    if (!CPresentation_Manager::Get().Are_ScreenPostsEnabled())
    {
        m_bPreviewCaptureClockHeld = false;
        m_bPreviewCaptureBoundarySampled = false;
        m_bPreviewCaptureAllowed = true;
        m_iPreviewCaptureWaitFrames = 0u;
        effectiveMs = requestedMs;
        if (wasHeld) m_PreviewSession.lastClockMs = -1.f;
        return true;
    }
    std::optional<std::uint32_t> firstCaptureMs;
    struct CAPTURE_OWNER { const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern; SESSION* session; std::uint32_t offsetMs; };
    std::vector<CAPTURE_OWNER> owners{{&m_PreviewPattern, &m_PreviewSession, 0u}};
    for (auto& member : m_BundlePreviewMembers)
        owners.push_back({&member.pattern, &member.session,
            static_cast<std::uint32_t>((std::uint64_t(member.offsetTicks) * 1000u + 29u) / 30u)});
    for (const auto& owner : owners)
    for (const auto& box : owner.pattern->PresentationOccurrences)
    {
        const auto boxStartMs = owner.offsetMs + box.iStartMs;
        if (requestedMs < boxStartMs ||
            double(requestedMs) >= double(boxStartMs) + box.iDurationMs) continue;
        const auto resource = std::find_if(m_PreviewDocument.PresentationResources.begin(),
            m_PreviewDocument.PresentationResources.end(), [&](const auto& value) {
                return value.strResourceId == box.strResourceId; });
        if (resource == m_PreviewDocument.PresentationResources.end() || resource->eKind != KIND::EFFECT ||
            (resource->strResourceKind != "V1_EFFECT" && resource->strResourceKind != "V1_ELEMENT")) continue;
        const auto document = CEffectCatalog::Find(resource->strAssetId);
        if (!document) continue; // Existing admission owns missing-document failures.
        for (const auto& element : document->Elements)
        {
            if (!element.bVisible || element.eKind != EFFECT_ELEMENT_KIND::SCREEN_POST ||
                !element.Detail.ScreenPost.bEnabled || element.Detail.ScreenPost.eStatus !=
                    EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE || element.Detail.ScreenPost.eProfile !=
                    EFFECT_SCREEN_POST_PROFILE::SCENE_COLLAPSE_CAPTURE_V1 ||
                (!resource->strElementId.empty() && resource->strElementId != element.strElementId)) continue;
            const double delayMs = double(element.Detail.Timing.fStartDelaySeconds) * 1000.0;
            if (!std::isfinite(delayMs) || delayMs < 0.0 || delayMs >= box.iDurationMs) continue;
            // Float seconds can put an authored integer millisecond a fraction
            // of a microsecond above itself; retain that cursor before rounding up.
            const auto captureMs = static_cast<std::uint32_t>(boxStartMs + std::ceil(delayMs - 0.001));
            const double captureEndMs = double(boxStartMs) + delayMs +
                double(element.Detail.Timing.fLifeTimeSeconds) * 1000.0;
            if (captureMs > requestedMs || double(requestedMs) >= captureEndMs) continue;
            const auto row = owner.session->rows.find(box.strOccurrenceId);
            if (row != owner.session->rows.end())
            {
                if (row->second.failed)
                {
                    m_strStatus = row->second.failureStatus;
                    return false;
                }
                const HRESULT captureResult = CEffectPresentationService::Get_ScreenPostCaptureResult(
                    {row->second.v1EffectHandle}, element.strElementId);
                if (FAILED(captureResult))
                {
                    m_strStatus = "Scene capture failed: " + box.strOccurrenceId + " / " +
                        element.strElementId + "; HRESULT=" + std::to_string(captureResult);
                    return false;
                }
                if (CEffectPresentationService::Has_CapturedScreenPost(
                    {row->second.v1EffectHandle}, element.strElementId)) continue;
            }
            if (!firstCaptureMs || captureMs < *firstCaptureMs) firstCaptureMs = captureMs;
        }
    }
    m_bPreviewCaptureClockHeld = firstCaptureMs.has_value();
    if (firstCaptureMs)
    {
        if (!wasHeld) m_iPreviewCaptureResumeMs = requestedMs;
        const bool sameBoundary = wasHeld && m_iPreviewCaptureBoundaryMs == *firstCaptureMs;
        m_bPreviewCaptureAllowed = sameBoundary && m_bPreviewCaptureBoundarySampled;
        if (!sameBoundary)
        {
            m_bPreviewCaptureBoundarySampled = false;
            m_iPreviewCaptureWaitFrames = 0u;
        }
        m_iPreviewCaptureBoundaryMs = *firstCaptureMs;
        effectiveMs = *firstCaptureMs;
        if (++m_iPreviewCaptureWaitFrames > 120u)
        {
            m_strStatus = "Scene capture did not receive a renderable frame at " +
                std::to_string(effectiveMs) + " ms within 120 preview updates. "
                "Check the active WORLD and Screen Presentation Post, then restart Preview.";
            return false;
        }
    }
    else
    {
        effectiveMs = requestedMs;
        m_bPreviewCaptureBoundarySampled = false;
        m_bPreviewCaptureAllowed = true;
        m_iPreviewCaptureWaitFrames = 0u;
    }
    if (wasHeld || m_bPreviewCaptureClockHeld)
    {
        // A render boundary is one continuing occurrence, not a new playback.
        // Keep its capture and observed anchors across the deferred cursor jump.
        m_PreviewSession.lastClockMs = -1.f;
        for (auto& member : m_BundlePreviewMembers) member.session.lastClockMs = -1.f;
        m_strCompletedPreviewPatternId.clear();
    }
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Sample_Preview(std::uint32_t clockMs,
    bool playing, bool paused, const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model)
{
    if (!playing) { Stop_Preview(); return; }
    if (!m_bPreviewPlaying) return;
    Set_PreviewPivot(pivot, model);
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_bPreviewPaused = paused;
    ANIMATION_MODEL_TARGET_VIEW weaponView;
    const bool hasWeapon = CAnimationTargetService::Resolve_Model() == model &&
        CAnimationTargetService::Resolve_ModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
    Sample(m_PreviewSession, m_PreviewDocument, m_PreviewPattern, float(m_fPreviewClockMs),
        paused, pivot, model, hasWeapon ? &weaponView : nullptr);
    // Resolve_PreviewCaptureClock checks the actual active capture occurrence.
    // Unrelated failed rows remain isolated, just as they do during ordinary
    // playback; a later scene capture must not turn them into a sequence stop.
    Refresh_SharedPresentation();
    // The next Engine Late_Update can now cull and submit this WORLD/camera.
    // The first boundary render is pass-through; only the next may latch it.
    if (m_bPreviewCaptureClockHeld && clockMs == m_iPreviewCaptureBoundaryMs)
        m_bPreviewCaptureBoundarySampled = true;
}

void Client::CKoukuSaydonPresentationPlayer::Pause_Preview(bool paused)
{
    m_strCompletedPreviewPatternId.clear();
    m_bPreviewPaused = paused;
    if (!paused && m_bPreviewPlaying && m_iPreviewDurationMs && m_fPreviewClockMs >= m_iPreviewDurationMs)
        Seek_Preview(0u);
    for (const auto& member : m_BundlePreviewMembers)
        for (const auto& [id, row] : member.session.rows)
        {
            if (row.effectHandle) CEffectV2Runtime::Set_GroupPaused(row.effectHandle, paused);
            if (row.soundHandle) CGameInstance::Get().Pause_SoundCue(row.soundHandle, paused);
        }
    for (const auto& [id, row] : m_PreviewSession.rows)
    {
        if (row.effectHandle) CEffectV2Runtime::Set_GroupPaused(row.effectHandle, paused);
        if (row.soundHandle) CGameInstance::Get().Pause_SoundCue(row.soundHandle, paused);
    }
}

void Client::CKoukuSaydonPresentationPlayer::Seek_Preview(std::uint32_t clockMs)
{
    m_bPreviewCaptureClockHeld = false;
    m_bPreviewCaptureBoundarySampled = false;
    m_bPreviewCaptureAllowed = true;
    m_iPreviewCaptureWaitFrames = 0u;
    for (const auto& [id, row] : m_PreviewSession.rows)
        CEffectPresentationService::Set_ScreenPostCaptureAllowed({row.v1EffectHandle}, true);
    m_strCompletedPreviewPatternId.clear();
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    if (m_fPreviewClockMs >= m_iPreviewDurationMs) Pause_Preview(true);
    // Stop/paused scrubbing must retain the observed birth history and a
    // frozen Effect's original anchor. The V2 external clock handles rewind.
    const auto prepare = [&](SESSION& session) {
        session.lastClockMs = -1.f;
        for (auto row = session.rows.begin(); row != session.rows.end();)
        {
            if (row->second.kind == KIND::EFFECT && !row->second.failed)
            { ++row; continue; }
            if (row->second.effectHandle) CEffectV2Runtime::Stop_Group(row->second.effectHandle);
        if (row->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({row->second.v1EffectHandle});
            if (row->second.soundHandle) CGameInstance::Get().Stop_SoundCue(row->second.soundHandle);
            row = session.rows.erase(row);
        }
    };
    prepare(m_PreviewSession);
    for (auto& member : m_BundlePreviewMembers) prepare(member.session);
    if (Preview_IsBundle()) Sample_BundlePreview();
    // MainApp samples single-pattern WORLD at the new clock before recreating these cue handles.
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Fail_Preview(std::string status)
{
    const std::string patternId = Preview_IsBundle() ? m_PreviewBundleId : m_PreviewPattern.strPatternId;
    Stop_Preview();
    m_strFailedPreviewPatternId = patternId;
    m_strFailedPreviewStatus = std::move(status);
    m_strStatus = m_strFailedPreviewStatus;
}

bool Client::CKoukuSaydonPresentationPlayer::Consume_FailedPreview(std::string& patternId, std::string& status)
{
    if (m_strFailedPreviewPatternId.empty()) return false;
    patternId = std::move(m_strFailedPreviewPatternId);
    status = std::move(m_strFailedPreviewStatus);
    m_strFailedPreviewPatternId.clear();
    m_strFailedPreviewStatus.clear();
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Consume_CompletedPreview(std::string& patternId)
{
    if (m_strCompletedPreviewPatternId.empty()) return false;
    patternId = std::move(m_strCompletedPreviewPatternId);
    Stop_Preview();
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Stop_Preview()
{
#ifdef _DEBUG
    if (m_bBundleWorldExternal)
        if (auto* level = CLevel_KakulSaydonArena::Get_Active()) level->Debug_StopCompositionWorldPreview();
#endif
    m_bBundleWorldExternal = false;
    m_strCompletedPreviewPatternId.clear();
    m_strFailedPreviewPatternId.clear();
    m_strFailedPreviewStatus.clear();
    ++m_iPreviewGeneration;
    m_bModelReferencePreview = false;
    Release_BundlePreviewMembers(m_BundlePreviewMembers);
    m_PreviewBundleId.clear();
    Stop_Session(m_PreviewSession);
    m_bPreviewPlaying = false;
    m_bOwnPreviewClock = false;
    m_bPreviewClockAwaitingFirstUpdate = false;
    m_bPreviewPreparationQueued = false;
    m_PreviewPreparationTargets.clear();
    m_bPreviewCaptureClockHeld = false;
    m_bPreviewCaptureBoundarySampled = false;
    m_bPreviewCaptureAllowed = true;
    m_iPreviewCaptureResumeMs = m_iPreviewCaptureBoundaryMs = m_iPreviewCaptureWaitFrames = 0u;
    m_bPreviewPaused = false;
    m_bPreviewPivotReady = false;
    m_bColliderResourcePreview = false;
    m_PreviewModel.reset();
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Sync_MazeMark(
    CARD& mark, const std::string& asset, const float4x4_t& pivot)
{
    const bool isCardMaze = IsCardMazeMarkGroup(asset);
    if (!isCardMaze)
    {
        if (mark.assetId != asset)
        {
            if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
            mark = {}; mark.assetId = asset;
            std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
            if (Ensure_EffectResource("GROUP", asset, effects))
                if (const auto* group = effects->Find_Group(asset))
                {
                    EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                    playback.PivotWorld = pivot;
                    playback.fDurationSeconds = 0.f;
                    playback.bProductOwned = true;
                    mark.handle = CEffectV2Runtime::Play_Group(*group, effects, playback, m_Device, m_Context);
                }
            if (!mark.handle) m_strStatus = "Maze floor mark unavailable: " + asset;
        }
        if (mark.handle)
        {
            CEffectV2Runtime::Set_GroupPivot(mark.handle, pivot);
            std::string failure;
            if (CEffectV2Runtime::Consume_GroupFailure(mark.handle, failure))
            {
                CEffectV2Runtime::Stop_Group(mark.handle); mark.handle = 0u;
                m_strStatus = "Maze floor mark failed: " + asset + ": " + failure;
            }
        }
        return;
    }
    if (mark.assetId != asset)
    {
        if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
        mark = {};
        mark.assetId = asset;
    }
    const auto generation = CEffectV2Runtime::Cache_Generation();
    const auto nowMs = static_cast<std::uint64_t>(std::chrono::duration_cast<
        std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    mark.mazeRetry.ObserveGeneration(generation);
    if (mark.handle)
    {
        CEffectV2Runtime::Set_GroupPivot(mark.handle, pivot);
        std::string failure;
        if (!CEffectV2Runtime::Consume_GroupFailure(mark.handle, failure)) return;
        CEffectV2Runtime::Stop_Group(mark.handle);
        mark.handle = 0;
        mark.mazeRetry.Defer(nowMs);
        m_strStatus = "Maze floor mark failed: " + asset + ": " + failure;
        return;
    }
    if (!mark.mazeRetry.TryBegin(nowMs)) return;
    const std::string resourceKey = "GROUP:" + asset;
    if (mark.mazeRetry.attempts > 1u)
    {
        // Only this resource is read again, at most twice per mark occurrence/generation.
        m_EffectResourceFailures.erase(resourceKey);
        m_EffectResources.erase(resourceKey);
    }
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
    if (!Ensure_EffectResource("GROUP", asset, effects))
    {
        const std::string reason = m_strStatus;
        m_strStatus = "Maze floor mark unavailable: " + asset + ": " + reason;
        return;
    }
    const auto* group = effects ? effects->Find_Group(asset) : nullptr;
    if (!group)
    {
        m_strStatus = "Maze floor mark group missing from snapshot: " + asset;
        return;
    }
    EFFECT_V2_GROUP_PLAYBACK_DESC playback;
    playback.PivotWorld = pivot;
    playback.fDurationSeconds = 0.f;
    playback.bProductOwned = true;
    mark.handle = CEffectV2Runtime::Play_Group(*group, effects, playback, m_Device, m_Context);
    // A handle may fail on the next frame; do not reset the attempt budget here.
    if (!mark.handle)
        m_strStatus = "Maze floor mark spawn failed: " + asset + ": " + CEffectV2Runtime::Last_Error();
}

void Client::CKoukuSaydonPresentationPlayer::Update_MazeMarks(
    const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players)
{
    using namespace LostArk::Shared;
    const auto suitName = [](MECHANIC_CARD_SYMBOL suit) -> const char*
    {
        switch (suit)
        {
        case MECHANIC_CARD_SYMBOL::HEART: return "heart";
        case MECHANIC_CARD_SYMBOL::SPADE: return "spade";
        case MECHANIC_CARD_SYMBOL::CLUB: return "club";
        case MECHANIC_CARD_SYMBOL::DIAMOND: return "diamond";
        default: return nullptr;
        }
    };
    const auto groundPivot = [](const float4x4_t& world)
    {
        // Translation only: character scale/yaw must not resize or rotate the symbol.
        float4x4_t pivot;
        XMStoreFloat4x4(&pivot, XMMatrixTranslation(world._41, world._42 + .02f, world._43));
        return pivot;
    };
    std::set<std::uint32_t> livePlayers, liveTargets, liveExits;
    bool mazeActive = false;
    for (const auto& view : players)
    {
        const auto& s = view.Snapshot;
        if (!s.iCurrentHp || s.eCardMazeRole == CARD_MAZE_ROLE::NONE || (s.CardMaze.flags & 8u)) continue;
        mazeActive = true;
        // The telescope owner never carries a suit marker, even with an old Debug snapshot.
        if (s.eCardMazeRole != CARD_MAZE_ROLE::HUNTER) continue;
        const char* suit = suitName(s.eCardMazeSuit);
        if (!suit) continue;
        if (!(s.CardMaze.flags & 2u) && !s.CardMaze.transferStartTick)
            if (auto character = view.pCharacter.lock(); character && character->Get_Transform())
            {
                livePlayers.insert(s.iNetEntityId);
                Sync_MazeMark(m_MazePlayerMarks[s.iNetEntityId], std::string("cardmaze.mark.") + suit,
                    groundPivot(*character->Get_Transform()->Get_WorldMatrixPtr()));
            }
        if ((s.CardMaze.flags & 4u) && !s.CardMaze.transferStartTick)
        {
            liveExits.insert(s.iNetEntityId);
            float4x4_t pivot;
            XMStoreFloat4x4(&pivot, XMMatrixTranslation(s.CardMaze.exitX, s.CardMaze.exitY + .02f, s.CardMaze.exitZ));
            Sync_MazeMark(m_MazeExits[s.iNetEntityId], std::string("cardmaze.exit.") + suit, pivot);
        }
    }
    if (const auto* arena = CLevel_KakulSaydonArena::Get_Active(); mazeActive && arena)
    {
        std::vector<KOUKU_MAZE_TARGET_VIEW> targets;
        arena->Collect_KoukuMazeTargets(targets);
        for (const auto& target : targets)
        {
            const char* suit = nullptr;
            if (target.archetypeId == "MONSTER_KOUKU_CARD_HEART") suit = "heart";
            else if (target.archetypeId == "MONSTER_KOUKU_CARD_SPADE") suit = "spade";
            else if (target.archetypeId == "MONSTER_KOUKU_CARD_CLUB") suit = "club";
            else if (target.archetypeId == "MONSTER_KOUKU_CARD_DIAMOND") suit = "diamond";
            const auto npc = target.npc.lock();
            if (!suit || !npc || !npc->Get_Transform()) continue;
            liveTargets.insert(target.entityId);
            Sync_MazeMark(m_MazeTargetMarks[target.entityId], std::string("cardmaze.mark.") + suit,
                groundPivot(*npc->Get_Transform()->Get_WorldMatrixPtr()));
        }
    }
    const auto removeStale = [](auto& marks, const auto& live)
    {
        for (auto i = marks.begin(); i != marks.end();)
        {
            if (live.contains(i->first)) { ++i; continue; }
            if (i->second.handle) CEffectV2Runtime::Stop_Group(i->second.handle);
            i = marks.erase(i);
        }
    };
    /* The bingo board. Both masks are room state the Server owns, so this
       only chooses which of the two authored decals sits on each painted
       cell and drops the ones the Server has cleared. */
    {
        const auto& board = CCombatHUDViewModel::Get().Get_BingoBoard();
        std::set<std::int32_t> liveBingo;
        for (std::int32_t cell = 0; cell < LostArk::Shared::KOUKU_BINGO_CELL_COUNT; ++cell)
        {
            const std::uint32_t bit = 1u << cell;
            if (0u == (board.iWhiteMask & bit)) continue;
            liveBingo.insert(cell);
            float4x4_t pivot;
            XMStoreFloat4x4(&pivot, XMMatrixTranslation(
                LostArk::Shared::Kouku_BingoCellCenterX(cell), .02f,
                LostArk::Shared::Kouku_BingoCellCenterZ(cell)));
            Sync_MazeMark(m_BingoMarks[cell],
                (0u != (board.iRedMask & bit)) ? "bingo.skull.red" : "bingo.skull.white",
                pivot);
        }
        for (auto i = m_BingoMarks.begin(); i != m_BingoMarks.end();)
        {
            if (liveBingo.contains(i->first)) { ++i; continue; }
            if (i->second.handle) CEffectV2Runtime::Stop_Group(i->second.handle);
            i = m_BingoMarks.erase(i);
        }
    }
    /* The bingo bomb's mark. It rides one named carrier and the Server can
       cancel it mid-flight, so it stays a followed state rather than a
       timeline. Height and size live in the authored document, so the pivot
       here is only the ground point under its carrier. The planted half is a
       World Sequence the Server names the moment it plants. */
    {
        const auto& bombBoard = CCombatHUDViewModel::Get().Get_BingoBoard();
        std::set<std::int32_t> liveBombs;
        for (std::uint8_t index = 0u; index < bombBoard.iBombCount; ++index)
        {
            const auto& bomb = bombBoard.Bombs[index];
            const char* bombAsset = nullptr;
            float4x4_t bombPivot;
            if (LostArk::Shared::BINGO_BOMB_PHASE::MARKED == bomb.ePhase)
            {
                std::shared_ptr<CCharacter> carrier;
                for (const auto& view : players)
                    if (view.Snapshot.iNetEntityId == bomb.iCarrierNetEntityId)
                    { carrier = view.pCharacter.lock(); break; }
                if (!carrier || !carrier->Get_Transform()) continue;
                const float4x4_t& carrierWorld =
                    *carrier->Get_Transform()->Get_WorldMatrixPtr();
                XMStoreFloat4x4(&bombPivot, XMMatrixTranslation(
                    carrierWorld._41, carrierWorld._42, carrierWorld._43));
                bombAsset = "bingo.bomb.mark";
            }
            /* PLANTED falls through: dropping the slot out of liveBombs is
               what takes the mark off the carrier, and the sequence has already
               started where it stood. */
            else continue;
            const std::int32_t slot = static_cast<std::int32_t>(index);
            liveBombs.insert(slot);
            Sync_MazeMark(m_BingoBombs[slot], bombAsset, bombPivot);
        }
        for (auto i = m_BingoBombs.begin(); i != m_BingoBombs.end();)
        {
            if (liveBombs.contains(i->first)) { ++i; continue; }
            if (i->second.handle) CEffectV2Runtime::Stop_Group(i->second.handle);
            i = m_BingoBombs.erase(i);
        }
    }
    /* The bingo hammer is authored now: four World Sequence templates, one
       per sweep direction, and one instance per anchor. The Server names the
       instance when it rolls the anchor, so there is nothing to pose here. */
    removeStale(m_MazePlayerMarks, livePlayers);
    removeStale(m_MazeTargetMarks, liveTargets);
    removeStale(m_MazeExits, liveExits);
}

void Client::CKoukuSaydonPresentationPlayer::Reset()
{
    m_LightPlayerPivots.clear();
    m_LightBossFollowers.clear();
    m_iProductReloadRunEpoch = 0u;
    Stop_Session(m_FearSession);
    m_FearSession.key.clear();
    m_strCompletedFearKey.clear();
    m_QueuedV1Effects.clear();
    for (auto& [id, session] : m_BossSessions) Stop_Session(session);
    m_BossSessions.clear();
    for (auto& [id, session] : m_MarioEntrySessions) Stop_Session(session);
    m_MarioEntrySessions.clear();
    Stop_Session(m_ProductBundleSession);
    for (const auto& [id, card] : m_Cards)
        if (card.handle) CEffectV2Runtime::Stop_Group(card.handle);
    m_Cards.clear();
    for (const auto& [cell, mark] : m_BingoMarks)
        if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
    m_BingoMarks.clear();
    for (const auto& [slot, bomb] : m_BingoBombs)
        if (bomb.handle) CEffectV2Runtime::Stop_Group(bomb.handle);
    m_BingoBombs.clear();
    for (const auto& [id, exit] : m_MazeExits)
        if (exit.handle) CEffectV2Runtime::Stop_Group(exit.handle);
    m_MazeExits.clear();
    for (auto* marks : { &m_MazePlayerMarks, &m_MazeTargetMarks })
    {
        for (const auto& [id, mark] : *marks)
            if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
        marks->clear();
    }
    m_ColliderDebugOverrides.clear();
    Stop_Preview();
    Restore_Scene();
}

bool Client::CKoukuSaydonPresentationPlayer::Preview_HasActiveWorldBox(const std::string_view occurrenceId) const
{
#ifdef _DEBUG
    if (occurrenceId.empty() || !m_bPreviewPlaying || m_bModelReferencePreview) return false;
    const auto contains = [&](const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const SESSION& session,
        const double clockMs, const bool bundle)
    {
        const auto box = std::find_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
            [&](const auto& value) {
                return value.strOccurrenceId == occurrenceId && clockMs >= value.iStartMs &&
                    clockMs < double(value.iStartMs) + value.iDurationMs;
            });
        if (box == pattern.WorldOccurrences.end()) return false;
        if (!bundle)
        {
            const auto* level = CLevel_KakulSaydonArena::Get_Active();
            return level && level->Debug_HasVisibleCompositionWorldBox(occurrenceId);
        }
        const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
            [&](const auto& value) { return value.strWorldId == box->strWorldId; });
        const auto player = session.previewWorlds.find(box->strOccurrenceId);
        float4x4_t pivot;
        return world != m_PreviewDocument.Worlds.end() && player != session.previewWorlds.end() &&
            player->second->Try_GetObjectPivot(world->strSequenceInstanceId, pivot);
    };
    if (contains(m_PreviewPattern, m_PreviewSession, m_fPreviewClockMs, false)) return true;
    for (const auto& member : m_BundlePreviewMembers)
        if (contains(member.pattern, member.session,
            m_fPreviewClockMs - double(member.offsetTicks) * 1000.0 / 30.0, true)) return true;
#endif
    return false;
}

void Client::CKoukuSaydonPresentationPlayer::Refresh_WorldPlacementAuthoring(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document)
{
#ifdef _DEBUG
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level || !m_bPreviewPlaying || m_bModelReferencePreview) return;
    const auto targets = level->Get_CompositionWorldTargets();
    const auto refresh = [&](KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, SESSION& session, const bool bundle)
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == pattern.strPatternId; });
        const bool placementPreview = pattern.strPatternId == "preview.kouku.resource";
        if (!placementPreview && (source == document.Patterns.end() || !source->strLoadError.empty())) return false;
        bool changed = false;
        for (auto& box : pattern.WorldOccurrences)
        {
            // Synthetic placement previews still own the authored stable occurrence IDs.
            const auto owner = placementPreview ? std::find_if(document.Patterns.begin(), document.Patterns.end(),
                [&](const auto& value) {
                    return value.strLoadError.empty() && std::any_of(value.WorldOccurrences.begin(), value.WorldOccurrences.end(),
                        [&](const auto& row) { return row.strOccurrenceId == box.strOccurrenceId; });
                }) : source;
            if (owner == document.Patterns.end()) continue;
            const auto edited = std::find_if(owner->WorldOccurrences.begin(), owner->WorldOccurrences.end(),
                [&](const auto& value) { return value.strOccurrenceId == box.strOccurrenceId && value.strWorldId == box.strWorldId; });
            if (edited == owner->WorldOccurrences.end() || box.Placement == edited->Placement) continue;
            const auto placement = WorldPlacementFromOccurrence(*edited);
            std::string status;
            bool applied = false;
            if (bundle)
            {
                const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
                    [&](const auto& value) { return value.strWorldId == box.strWorldId; });
                const auto player = session.previewWorlds.find(box.strOccurrenceId);
                if (world != m_PreviewDocument.Worlds.end() && player != session.previewWorlds.end())
                {
                    applied = player->second->Validate_ObjectPlacement(world->strSequenceInstanceId, placement, status);
                    if (applied && player->second->Is_Playing(world->strSequenceInstanceId))
                    {
                        applied = player->second->Set_ObjectPlacement(world->strSequenceInstanceId, placement, targets);
                        if (!applied) status = player->second->Get_Status();
                    }
                }
            }
            else applied = level->Debug_SetCompositionWorldPlacement(box.strOccurrenceId, placement, status);
            if (!applied)
            {
                m_strStatus = "WORLD placement preview kept its previous pose: " + box.strOccurrenceId + "; " + status;
                continue;
            }
            box.Placement = edited->Placement;
            changed = true;
        }
        return changed;
    };
    (void)refresh(m_PreviewPattern, m_PreviewSession, false);
    bool bundleChanged = false;
    for (auto& member : m_BundlePreviewMembers)
        bundleChanged |= refresh(member.pattern, member.session, true);
    if (bundleChanged) Sample_BundlePreview();
#endif
}

void Client::CKoukuSaydonPresentationPlayer::Refresh_ColliderAuthoring(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, std::uint64_t generation)
{
    if (m_iColliderAuthoringGeneration == generation) return;
    m_iColliderAuthoringGeneration = generation;
    m_ColliderDebugOverrides.clear();
    for (const auto& pattern : document.Patterns)
        for (const auto& box : pattern.PresentationOccurrences)
            m_ColliderDebugOverrides.emplace(box.strOccurrenceId, box.bDebugRender);
    if (!m_bPreviewPlaying) return;
    Refresh_WorldPlacementAuthoring(document);
    bool changed = false;
    for (auto& box : m_PreviewPattern.PresentationOccurrences)
    {
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&box](const auto& value) { return value.strResourceId == box.strResourceId && value.eKind == KIND::COLLIDER; });
        if (resource == document.PresentationResources.end()) continue;
        const auto old = std::find_if(m_PreviewDocument.PresentationResources.begin(), m_PreviewDocument.PresentationResources.end(),
            [&box](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (old != m_PreviewDocument.PresentationResources.end() && *old != *resource)
        { *old = *resource; changed = true; }
        for (const auto& pattern : document.Patterns)
        {
            const auto source = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
                [&box](const auto& value) { return value.strOccurrenceId == box.strOccurrenceId; });
            if (source == pattern.PresentationOccurrences.end()) continue;
            auto staged = *source;
            if (m_bColliderResourcePreview) staged.iStartMs = 0u;
            if (box != staged) { box = std::move(staged); changed = true; }
            break;
        }
    }
    if (changed)
    {
        m_PreviewDocument.Worlds = document.Worlds;
        if (m_bColliderResourcePreview && !m_PreviewPattern.Stages.empty())
            m_PreviewPattern.Stages.front().iDurationMs = m_PreviewPattern.PresentationOccurrences.front().iDurationMs;
        m_iPreviewDurationMs = Pattern_Duration(m_PreviewPattern);
        m_fPreviewClockMs = (std::min)(m_fPreviewClockMs, double(m_iPreviewDurationMs - 1u));
        Stop_Session(m_PreviewSession);
    }
}

bool Client::CKoukuSaydonPresentationPlayer::Preview_PresentationGeometry(
    const std::string& patternId, const OCCURRENCE& occurrence)
{
    if (!m_bPreviewPlaying || m_bModelReferencePreview) return false;
    if (m_bColliderResourcePreview)
    {
        const auto owner = std::find_if(m_PreviewDocument.Patterns.begin(), m_PreviewDocument.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == patternId; });
        if (owner == m_PreviewDocument.Patterns.end() ||
            std::none_of(owner->PresentationOccurrences.begin(), owner->PresentationOccurrences.end(),
                [&](const auto& value) { return value.strOccurrenceId == occurrence.strOccurrenceId &&
                    value.strResourceId == occurrence.strResourceId; })) return false;
    }
    const auto finite = [](const auto& values, const double minimum, const double maximum) {
        return std::all_of(values.begin(), values.end(), [=](const double value) {
            return std::isfinite(value) && value >= minimum && value <= maximum;
        });
    };
    if (!finite(occurrence.PositionOffset, -100000.0, 100000.0) ||
        !finite(occurrence.RotationDegrees, -36000.0, 36000.0) ||
        !finite(occurrence.Scale, 0.001, 10000.0)) return false;
    const auto update = [&](KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, SESSION& session) {
        if (pattern.strPatternId != patternId && !m_bColliderResourcePreview) return false;
        const auto box = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
            [&](const auto& value) { return value.strOccurrenceId == occurrence.strOccurrenceId &&
                value.strResourceId == occurrence.strResourceId; });
        if (box == pattern.PresentationOccurrences.end()) return false;
        const auto resource = std::find_if(m_PreviewDocument.PresentationResources.begin(), m_PreviewDocument.PresentationResources.end(),
            [&](const auto& value) { return value.strResourceId == box->strResourceId &&
                (value.eKind == KIND::COLLIDER || value.eKind == KIND::EFFECT); });
        if (resource == m_PreviewDocument.PresentationResources.end()) return false;
        auto edited = *box;
        edited.PositionOffset = occurrence.PositionOffset;
        edited.RotationDegrees = occurrence.RotationDegrees;
        edited.Scale = occurrence.Scale;
        bool anchorChanged = Is_CenteredWorldCircle(*resource, edited) != Is_CenteredWorldCircle(*resource, *box);
        if (resource->eKind == KIND::EFFECT)
        {
            edited.strAnchorKind = occurrence.strAnchorKind;
            edited.bFollowBoss = occurrence.bFollowBoss;
            edited.strBone = occurrence.strBone;
            edited.strBoneTarget = occurrence.strBoneTarget;
            edited.strWorldId = occurrence.strWorldId;
            edited.strWorldOccurrenceId = occurrence.strWorldOccurrenceId;
            edited.iWorldEmissionIndex = occurrence.iWorldEmissionIndex;
            if (!Validate_EffectAnchor(m_PreviewDocument, pattern, edited, m_strStatus)) return false;
            anchorChanged = edited.strAnchorKind != box->strAnchorKind || edited.bFollowBoss != box->bFollowBoss ||
                edited.strBone != box->strBone || edited.strBoneTarget != box->strBoneTarget ||
                edited.strWorldId != box->strWorldId || edited.strWorldOccurrenceId != box->strWorldOccurrenceId ||
                edited.iWorldEmissionIndex != box->iWorldEmissionIndex;
        }
        const auto active = session.rows.find(box->strOccurrenceId);
        if (anchorChanged)
        {
            // A new anchor owns a new occurrence playback. Other rows and the
            // session's observed boss history retain their current clock/state.
            if (active != session.rows.end())
            {
                if (active->second.effectHandle) CEffectV2Runtime::Stop_Group(active->second.effectHandle);
                if (active->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({active->second.v1EffectHandle});
                session.rows.erase(active);
            }
            session.effectAnchorHistories.erase(box->strOccurrenceId);
            *box = std::move(edited);
            return true;
        }
        if (active != session.rows.end() && active->second.hasPlacementAnchor &&
            !active->second.failed && !active->second.waitingForAnchor)
        {
            // Frozen rows retain their first anchor; following rows retain the
            // current sampled anchor and all prior particle birth transforms.
            auto placed = edited;
            placed.strBone.clear();
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                placed.PositionOffset[axis] *= active->second.placementAnchorScale[axis];
                placed.Scale[axis] *= active->second.placementAnchorScale[axis];
            }
            float4x4_t pivot{};
            if (!Make_Pivot(placed, active->second.placementAnchor, nullptr, pivot)) return false;
            if (resource->eKind == KIND::EFFECT && active->second.effectHandle &&
                !CEffectV2Runtime::Rebuild_GroupPlacement(active->second.effectHandle, pivot,
                    Effect_PivotSampler(edited, session.rootHistory, active->second.effectPivotHistory),
                    m_Device, m_Context))
            {
                m_strStatus = "Effect geometry preview failed: " + CEffectV2Runtime::Last_Error();
                return false;
            }
            if (resource->eKind == KIND::EFFECT && active->second.v1EffectHandle &&
                (!CEffectPresentationService::Update_WorldRoot({active->second.v1EffectHandle}, pivot) ||
                 !CEffectPresentationService::Seek_WorldRoot({active->second.v1EffectHandle},
                    (std::max)(0.f, active->second.lastAge), Effect_V1TransformProvider(edited, pivot,
                        session.rootHistory, active->second.effectPivotHistory, active->second.sourceAnchorSampler), true)))
            {
                m_strStatus = "V1 Effect geometry preview lost its active handle: " +
                    CEffectPresentationService::Get_Status();
                return false;
            }
            active->second.pivot = pivot;
            if (resource->eKind == KIND::COLLIDER)
                active->second.wire = Collider_Wire(*resource, placed);
        }
        else if (active != session.rows.end())
        {
            if (active->second.effectHandle) CEffectV2Runtime::Stop_Group(active->second.effectHandle);
            if (active->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({active->second.v1EffectHandle});
            session.rows.erase(active);
        }
        box->PositionOffset = occurrence.PositionOffset;
        box->RotationDegrees = occurrence.RotationDegrees;
        box->Scale = occurrence.Scale;
        return true;
    };
    for (auto& member : m_BundlePreviewMembers)
    {
        if (!update(member.pattern, member.session)) continue;
        const double localMs = m_fPreviewClockMs - double(member.offsetTicks) * 1000.0 / 30.0;
        if (member.actor && localMs >= 0.0 && localMs < member.durationMs)
        {
            ANIMATION_MODEL_TARGET_VIEW weaponView;
            const bool hasWeapon = member.actor->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
            Sample(member.session, m_PreviewDocument, member.pattern, float(localMs), m_bPreviewPaused,
                *member.actor->Get_Transform()->Get_WorldMatrixPtr(), member.actor->Get_Model(),
                hasWeapon ? &weaponView : nullptr);
        }
        return true;
    }
    if (!update(m_PreviewPattern, m_PreviewSession)) return false;
    // MainApp samples the single-pattern owner later in this same frame with its
    // actual animation target and weapon view. Its displayed clock is unchanged.
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Render_Debug() const
{
#ifdef _DEBUG
    if (m_LightProvider)
        for (const auto& light : m_LightProvider->lights)
            if (light.debugRender) Draw_LightWire(light.desc);
    const auto draw = [this](const SESSION& session, bool preview)
    {
        for (const auto& [id, row] : session.rows)
            if (!row.failed && !row.waitingForAnchor && row.kind == KIND::COLLIDER)
            {
                const auto override = m_ColliderDebugOverrides.find(id);
                const bool visible = !preview && override != m_ColliderDebugOverrides.end() ? override->second : row.debugRender;
                if (visible) CHitAreaWire::Draw(row.pivot, row.wire, 0xff40dfff);
            }
    };
    for (const auto& [id, session] : m_BossSessions) draw(session, false);
    for (const auto& [id, session] : m_MarioEntrySessions) draw(session, false);
    if (m_bPreviewPlaying) draw(m_PreviewSession, true);
    for (const auto& member : m_BundlePreviewMembers) draw(member.session, true);
#endif
}
```

### Engine/Public/BlendSortKey.h

대상: `C:/Users/USER/source/졸업팀폴/LostArk/Engine/Public/BlendSortKey.h`

```cpp
#pragma once
#include <cstdint>

namespace Engine
{
    struct BLEND_SORT_KEY final
    {
        std::int32_t priority = 0;
        float distanceSquared = 0.f;
    };
    // Caller normalizes non-finite distances before constructing the key.
    inline bool BlendSortBefore(const BLEND_SORT_KEY& lhs, const BLEND_SORT_KEY& rhs)
    {
        if (lhs.priority != rhs.priority) return lhs.priority < rhs.priority;
        return lhs.distanceSquared > rhs.distanceSquared;
    }
}
```

### Client/Public/CardMazeVisualPolicy.h

대상: `C:/Users/USER/source/졸업팀폴/LostArk/Client/Public/CardMazeVisualPolicy.h`

```cpp
#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include <string_view>

namespace Client
{
    inline bool IsCardMazeFloorReceiver(std::uint64_t placementId, std::string_view assetId)
    {
        return placementId == 10296705976280178153ull &&
            assetId == "MAP_3C514C107BAB_LV_OCN_FORGOTTENIS_PLANE01_SM_OVR_017DC7A6977C";
    }
    inline bool IsCardMazeMarkGroup(std::string_view asset)
    {
        constexpr std::array<std::string_view, 8> ids = {
            "cardmaze.mark.heart", "cardmaze.mark.spade", "cardmaze.mark.club", "cardmaze.mark.diamond",
            "cardmaze.exit.heart", "cardmaze.exit.spade", "cardmaze.exit.club", "cardmaze.exit.diamond"
        };
        for (const auto id : ids) if (asset == id) return true;
        return false;
    }
    struct CARD_MAZE_MARK_RETRY final
    {
        std::uint64_t generation = (std::numeric_limits<std::uint64_t>::max)();
        std::uint64_t nextAttemptMs = 0;
        std::uint32_t attempts = 0;

        void ObserveGeneration(std::uint64_t value)
        {
            if (generation == value) return;
            generation = value;
            attempts = 0;
            nextAttemptMs = 0;
        }
        bool TryBegin(std::uint64_t nowMs)
        {
            if (attempts >= 3u || nowMs < nextAttemptMs) return false;
            ++attempts;
            Defer(nowMs);
            return true;
        }
        void Defer(std::uint64_t nowMs)
        {
            constexpr auto maximum = (std::numeric_limits<std::uint64_t>::max)();
            nextAttemptMs = nowMs > maximum - 1000u ? maximum : nowMs + 1000u;
        }
    };
}
```

## G06. Effect leaf 교체 전문

8개 group은 수정하지 않는다. 아래 네 파일만 교체한다.

### Data/Effects/V2/Authored/cardmaze.symbol.heart.effectv2.json

```json
{
  "schema": "lostark.effect-v2",
  "formatVersion": 1,
  "effectId": "cardmaze.symbol.heart",
  "effectType": "Texture",
  "slots": {
    "mesh": "",
    "base": "Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47.dds",
    "noise": "",
    "mask": "Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47.dds",
    "emissive": "",
    "dissolve": ""
  },
  "params": {
    "position": {
      "start": [
        0,
        0,
        0
      ],
      "end": [
        0,
        0,
        0
      ],
      "lerp": false
    },
    "rotation": {
      "start": [
        90,
        -90,
        0
      ],
      "end": [
        90,
        -90,
        0
      ],
      "lerp": false
    },
    "scale": {
      "start": [
        2,
        2,
        1
      ],
      "end": [
        2,
        2,
        1
      ],
      "lerp": false
    },
    "velocity": {
      "start": [
        0,
        0,
        0
      ],
      "end": [
        0,
        0,
        0
      ],
      "lerp": false
    },
    "colorOffset": [
      0,
      0,
      0,
      0
    ],
    "colorOffsetEnd": [
      0,
      0,
      0,
      0
    ],
    "colorOffsetLerp": false,
    "colorMul": [
      1,
      0,
      0,
      0.6
    ],
    "colorMulEnd": [
      1,
      1,
      1,
      0
    ],
    "colorMulLerp": false,
    "colorClipChannel": "Alpha",
    "colorClip": 0,
    "rimColor": [
      1,
      1,
      1,
      1
    ],
    "rimPower": 3,
    "rimIntensity": 0,
    "ghostAlpha": 0,
    "outlineWidth": 0,
    "outlineColor": [
      1,
      1,
      1,
      1
    ],
    "bloomIntensity": 1,
    "distortionIntensity": 0,
    "uvStart": [
      0,
      0
    ],
    "uvSpeed": [
      0,
      0
    ],
    "uvTileCount": [
      1,
      1
    ],
    "noiseStrength": 0,
    "noiseScale": 1,
    "noisePan": [
      0,
      0
    ],
    "dissolveStart": 1,
    "dissolveInEnd": 0,
    "dissolveSoftness": 0.1,
    "dissolveWarp": false,
    "maskWarp": true,
    "alphaInEnd": 0,
    "alphaOutStart": 1,
    "scaleInEnd": 0,
    "scaleOutStart": 1,
    "blend": "Alpha",
    "billboard": false,
    "depthTest": true,
    "softFadeDistance": 0,
    "lifetime": 0,
    "loop": true,
    "playRate": 1,
    "meshPreScale": 0.01,
    "animationClip": "",
    "animationLoop": true,
    "colorTexturesSRGB": true,
    "particle": {
      "maxParticles": 256,
      "spawnRate": 20,
      "burstCount": 0,
      "lifetime": [
        0.5,
        1
      ],
      "spawnShape": "Point",
      "spawnRadius": 0.5,
      "spawnInnerRadius": 0,
      "spawnExtents": [
        0.5,
        0.5,
        0.5
      ],
      "spawnArcDegrees": 360,
      "velocityMode": "Cone",
      "velocityMin": [
        -0.5,
        1,
        -0.5
      ],
      "velocityMax": [
        0.5,
        2,
        0.5
      ],
      "speedRange": [
        1,
        2
      ],
      "coneAngleDegrees": 30,
      "acceleration": [
        0,
        -1,
        0
      ],
      "drag": 0,
      "sizeStart": [
        0.2,
        0.2
      ],
      "sizeEnd": [
        0,
        0
      ],
      "rotationRange": [
        0,
        0
      ],
      "spinRange": [
        0,
        0
      ],
      "colorStart": [
        1,
        1,
        1,
        1
      ],
      "colorEnd": [
        1,
        1,
        1,
        0
      ],
      "alignment": "Camera",
      "localSpace": true,
      "tileColumns": 1,
      "tileRows": 1,
      "subUVOverLife": true,
      "randomSeed": 1,
      "meshRotationMin": [
        0,
        0,
        0
      ],
      "meshRotationMax": [
        0,
        0,
        0
      ],
      "meshSpinMin": [
        0,
        0,
        0
      ],
      "meshSpinMax": [
        0,
        0,
        0
      ]
    },
    "decal": {
      "size": [
        2,
        2
      ],
      "depth": 0.3,
      "edgeFade": 0,
      "normalCutoff": 0.5
    },
    "trail": {
      "maxPoints": 64,
      "pointLifetime": 0.35,
      "sampleInterval": 0.01666667,
      "minDistance": 0.01,
      "startWidth": 0.2,
      "endWidth": 0,
      "tilingDistance": 0,
      "edgeMode": "CenterlineCamera",
      "edgeOffset": [
        0,
        1,
        0
      ],
      "fadeWithAge": true
    },
    "screenPost": {
      "profile": "ZoomBlur",
      "intensityStart": 2,
      "intensityEnd": 0,
      "intensityLerp": true,
      "secondaryIntensity": 0,
      "frequency": 1,
      "tint": [
        1,
        1,
        1,
        1
      ],
      "randomSeed": 1
    }
  },
  "parts": []
}
```

### Data/Effects/V2/Authored/cardmaze.symbol.spade.effectv2.json

```json
{
  "schema": "lostark.effect-v2",
  "formatVersion": 1,
  "effectId": "cardmaze.symbol.spade",
  "effectType": "Texture",
  "slots": {
    "mesh": "",
    "base": "Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_1.dds",
    "noise": "",
    "mask": "Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_1.dds",
    "emissive": "",
    "dissolve": ""
  },
  "params": {
    "position": {
      "start": [
        0,
        0,
        0
      ],
      "end": [
        0,
        0,
        0
      ],
      "lerp": false
    },
    "rotation": {
      "start": [
        90,
        -90,
        0
      ],
      "end": [
        90,
        -90,
        0
      ],
      "lerp": false
    },
    "scale": {
      "start": [
        2,
        2,
        1
      ],
      "end": [
        2,
        2,
        1
      ],
      "lerp": false
    },
    "velocity": {
      "start": [
        0,
        0,
        0
      ],
      "end": [
        0,
        0,
        0
      ],
      "lerp": false
    },
    "colorOffset": [
      0,
      0,
      0,
      0
    ],
    "colorOffsetEnd": [
      0,
      0,
      0,
      0
    ],
    "colorOffsetLerp": false,
    "colorMul": [
      0,
      0,
      1,
      0.6
    ],
    "colorMulEnd": [
      1,
      1,
      1,
      0
    ],
    "colorMulLerp": false,
    "colorClipChannel": "Alpha",
    "colorClip": 0,
    "rimColor": [
      1,
      1,
      1,
      1
    ],
    "rimPower": 3,
    "rimIntensity": 0,
    "ghostAlpha": 0,
    "outlineWidth": 0,
    "outlineColor": [
      1,
      1,
      1,
      1
    ],
    "bloomIntensity": 1,
    "distortionIntensity": 0,
    "uvStart": [
      0,
      0
    ],
    "uvSpeed": [
      0,
      0
    ],
    "uvTileCount": [
      1,
      1
    ],
    "noiseStrength": 0,
    "noiseScale": 1,
    "noisePan": [
      0,
      0
    ],
    "dissolveStart": 1,
    "dissolveInEnd": 0,
    "dissolveSoftness": 0.1,
    "dissolveWarp": false,
    "maskWarp": true,
    "alphaInEnd": 0,
    "alphaOutStart": 1,
    "scaleInEnd": 0,
    "scaleOutStart": 1,
    "blend": "Alpha",
    "billboard": false,
    "depthTest": true,
    "softFadeDistance": 0,
    "lifetime": 0,
    "loop": true,
    "playRate": 1,
    "meshPreScale": 0.01,
    "animationClip": "",
    "animationLoop": true,
    "colorTexturesSRGB": true,
    "particle": {
      "maxParticles": 256,
      "spawnRate": 20,
      "burstCount": 0,
      "lifetime": [
        0.5,
        1
      ],
      "spawnShape": "Point",
      "spawnRadius": 0.5,
      "spawnInnerRadius": 0,
      "spawnExtents": [
        0.5,
        0.5,
        0.5
      ],
      "spawnArcDegrees": 360,
      "velocityMode": "Cone",
      "velocityMin": [
        -0.5,
        1,
        -0.5
      ],
      "velocityMax": [
        0.5,
        2,
        0.5
      ],
      "speedRange": [
        1,
        2
      ],
      "coneAngleDegrees": 30,
      "acceleration": [
        0,
        -1,
        0
      ],
      "drag": 0,
      "sizeStart": [
        0.2,
        0.2
      ],
      "sizeEnd": [
        0,
        0
      ],
      "rotationRange": [
        0,
        0
      ],
      "spinRange": [
        0,
        0
      ],
      "colorStart": [
        1,
        1,
        1,
        1
      ],
      "colorEnd": [
        1,
        1,
        1,
        0
      ],
      "alignment": "Camera",
      "localSpace": true,
      "tileColumns": 1,
      "tileRows": 1,
      "subUVOverLife": true,
      "randomSeed": 1,
      "meshRotationMin": [
        0,
        0,
        0
      ],
      "meshRotationMax": [
        0,
        0,
        0
      ],
      "meshSpinMin": [
        0,
        0,
        0
      ],
      "meshSpinMax": [
        0,
        0,
        0
      ]
    },
    "decal": {
      "size": [
        2,
        2
      ],
      "depth": 0.3,
      "edgeFade": 0,
      "normalCutoff": 0.5
    },
    "trail": {
      "maxPoints": 64,
      "pointLifetime": 0.35,
      "sampleInterval": 0.01666667,
      "minDistance": 0.01,
      "startWidth": 0.2,
      "endWidth": 0,
      "tilingDistance": 0,
      "edgeMode": "CenterlineCamera",
      "edgeOffset": [
        0,
        1,
        0
      ],
      "fadeWithAge": true
    },
    "screenPost": {
      "profile": "ZoomBlur",
      "intensityStart": 2,
      "intensityEnd": 0,
      "intensityLerp": true,
      "secondaryIntensity": 0,
      "frequency": 1,
      "tint": [
        1,
        1,
        1,
        1
      ],
      "randomSeed": 1
    }
  },
  "parts": []
}
```

### Data/Effects/V2/Authored/cardmaze.symbol.club.effectv2.json

```json
{
  "schema": "lostark.effect-v2",
  "formatVersion": 1,
  "effectId": "cardmaze.symbol.club",
  "effectType": "Texture",
  "slots": {
    "mesh": "",
    "base": "Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_2.dds",
    "noise": "",
    "mask": "Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_2.dds",
    "emissive": "",
    "dissolve": ""
  },
  "params": {
    "position": {
      "start": [
        0,
        0,
        0
      ],
      "end": [
        0,
        0,
        0
      ],
      "lerp": false
    },
    "rotation": {
      "start": [
        90,
        -90,
        0
      ],
      "end": [
        90,
        -90,
        0
      ],
      "lerp": false
    },
    "scale": {
      "start": [
        2,
        2,
        1
      ],
      "end": [
        2,
        2,
        1
      ],
      "lerp": false
    },
    "velocity": {
      "start": [
        0,
        0,
        0
      ],
      "end": [
        0,
        0,
        0
      ],
      "lerp": false
    },
    "colorOffset": [
      0,
      0,
      0,
      0
    ],
    "colorOffsetEnd": [
      0,
      0,
      0,
      0
    ],
    "colorOffsetLerp": false,
    "colorMul": [
      0,
      1,
      0,
      0.6
    ],
    "colorMulEnd": [
      1,
      1,
      1,
      0
    ],
    "colorMulLerp": false,
    "colorClipChannel": "Alpha",
    "colorClip": 0,
    "rimColor": [
      1,
      1,
      1,
      1
    ],
    "rimPower": 3,
    "rimIntensity": 0,
    "ghostAlpha": 0,
    "outlineWidth": 0,
    "outlineColor": [
      1,
      1,
      1,
      1
    ],
    "bloomIntensity": 1,
    "distortionIntensity": 0,
    "uvStart": [
      0,
      0
    ],
    "uvSpeed": [
      0,
      0
    ],
    "uvTileCount": [
      1,
      1
    ],
    "noiseStrength": 0,
    "noiseScale": 1,
    "noisePan": [
      0,
      0
    ],
    "dissolveStart": 1,
    "dissolveInEnd": 0,
    "dissolveSoftness": 0.1,
    "dissolveWarp": false,
    "maskWarp": true,
    "alphaInEnd": 0,
    "alphaOutStart": 1,
    "scaleInEnd": 0,
    "scaleOutStart": 1,
    "blend": "Alpha",
    "billboard": false,
    "depthTest": true,
    "softFadeDistance": 0,
    "lifetime": 0,
    "loop": true,
    "playRate": 1,
    "meshPreScale": 0.01,
    "animationClip": "",
    "animationLoop": true,
    "colorTexturesSRGB": true,
    "particle": {
      "maxParticles": 256,
      "spawnRate": 20,
      "burstCount": 0,
      "lifetime": [
        0.5,
        1
      ],
      "spawnShape": "Point",
      "spawnRadius": 0.5,
      "spawnInnerRadius": 0,
      "spawnExtents": [
        0.5,
        0.5,
        0.5
      ],
      "spawnArcDegrees": 360,
      "velocityMode": "Cone",
      "velocityMin": [
        -0.5,
        1,
        -0.5
      ],
      "velocityMax": [
        0.5,
        2,
        0.5
      ],
      "speedRange": [
        1,
        2
      ],
      "coneAngleDegrees": 30,
      "acceleration": [
        0,
        -1,
        0
      ],
      "drag": 0,
      "sizeStart": [
        0.2,
        0.2
      ],
      "sizeEnd": [
        0,
        0
      ],
      "rotationRange": [
        0,
        0
      ],
      "spinRange": [
        0,
        0
      ],
      "colorStart": [
        1,
        1,
        1,
        1
      ],
      "colorEnd": [
        1,
        1,
        1,
        0
      ],
      "alignment": "Camera",
      "localSpace": true,
      "tileColumns": 1,
      "tileRows": 1,
      "subUVOverLife": true,
      "randomSeed": 1,
      "meshRotationMin": [
        0,
        0,
        0
      ],
      "meshRotationMax": [
        0,
        0,
        0
      ],
      "meshSpinMin": [
        0,
        0,
        0
      ],
      "meshSpinMax": [
        0,
        0,
        0
      ]
    },
    "decal": {
      "size": [
        2,
        2
      ],
      "depth": 0.3,
      "edgeFade": 0,
      "normalCutoff": 0.5
    },
    "trail": {
      "maxPoints": 64,
      "pointLifetime": 0.35,
      "sampleInterval": 0.01666667,
      "minDistance": 0.01,
      "startWidth": 0.2,
      "endWidth": 0,
      "tilingDistance": 0,
      "edgeMode": "CenterlineCamera",
      "edgeOffset": [
        0,
        1,
        0
      ],
      "fadeWithAge": true
    },
    "screenPost": {
      "profile": "ZoomBlur",
      "intensityStart": 2,
      "intensityEnd": 0,
      "intensityLerp": true,
      "secondaryIntensity": 0,
      "frequency": 1,
      "tint": [
        1,
        1,
        1,
        1
      ],
      "randomSeed": 1
    }
  },
  "parts": []
}
```

### Data/Effects/V2/Authored/cardmaze.symbol.diamond.effectv2.json

```json
{
  "schema": "lostark.effect-v2",
  "formatVersion": 1,
  "effectId": "cardmaze.symbol.diamond",
  "effectType": "Texture",
  "slots": {
    "mesh": "",
    "base": "Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_3.dds",
    "noise": "",
    "mask": "Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_3.dds",
    "emissive": "",
    "dissolve": ""
  },
  "params": {
    "position": {
      "start": [
        0,
        0,
        0
      ],
      "end": [
        0,
        0,
        0
      ],
      "lerp": false
    },
    "rotation": {
      "start": [
        90,
        -90,
        0
      ],
      "end": [
        90,
        -90,
        0
      ],
      "lerp": false
    },
    "scale": {
      "start": [
        2,
        2,
        1
      ],
      "end": [
        2,
        2,
        1
      ],
      "lerp": false
    },
    "velocity": {
      "start": [
        0,
        0,
        0
      ],
      "end": [
        0,
        0,
        0
      ],
      "lerp": false
    },
    "colorOffset": [
      0,
      0,
      0,
      0
    ],
    "colorOffsetEnd": [
      0,
      0,
      0,
      0
    ],
    "colorOffsetLerp": false,
    "colorMul": [
      2,
      1,
      0,
      0.6
    ],
    "colorMulEnd": [
      1,
      1,
      1,
      0
    ],
    "colorMulLerp": false,
    "colorClipChannel": "Alpha",
    "colorClip": 0,
    "rimColor": [
      1,
      1,
      1,
      1
    ],
    "rimPower": 3,
    "rimIntensity": 0,
    "ghostAlpha": 0,
    "outlineWidth": 0,
    "outlineColor": [
      1,
      1,
      1,
      1
    ],
    "bloomIntensity": 1,
    "distortionIntensity": 0,
    "uvStart": [
      0,
      0
    ],
    "uvSpeed": [
      0,
      0
    ],
    "uvTileCount": [
      1,
      1
    ],
    "noiseStrength": 0,
    "noiseScale": 1,
    "noisePan": [
      0,
      0
    ],
    "dissolveStart": 1,
    "dissolveInEnd": 0,
    "dissolveSoftness": 0.1,
    "dissolveWarp": false,
    "maskWarp": true,
    "alphaInEnd": 0,
    "alphaOutStart": 1,
    "scaleInEnd": 0,
    "scaleOutStart": 1,
    "blend": "Alpha",
    "billboard": false,
    "depthTest": true,
    "softFadeDistance": 0,
    "lifetime": 0,
    "loop": true,
    "playRate": 1,
    "meshPreScale": 0.01,
    "animationClip": "",
    "animationLoop": true,
    "colorTexturesSRGB": true,
    "particle": {
      "maxParticles": 256,
      "spawnRate": 20,
      "burstCount": 0,
      "lifetime": [
        0.5,
        1
      ],
      "spawnShape": "Point",
      "spawnRadius": 0.5,
      "spawnInnerRadius": 0,
      "spawnExtents": [
        0.5,
        0.5,
        0.5
      ],
      "spawnArcDegrees": 360,
      "velocityMode": "Cone",
      "velocityMin": [
        -0.5,
        1,
        -0.5
      ],
      "velocityMax": [
        0.5,
        2,
        0.5
      ],
      "speedRange": [
        1,
        2
      ],
      "coneAngleDegrees": 30,
      "acceleration": [
        0,
        -1,
        0
      ],
      "drag": 0,
      "sizeStart": [
        0.2,
        0.2
      ],
      "sizeEnd": [
        0,
        0
      ],
      "rotationRange": [
        0,
        0
      ],
      "spinRange": [
        0,
        0
      ],
      "colorStart": [
        1,
        1,
        1,
        1
      ],
      "colorEnd": [
        1,
        1,
        1,
        0
      ],
      "alignment": "Camera",
      "localSpace": true,
      "tileColumns": 1,
      "tileRows": 1,
      "subUVOverLife": true,
      "randomSeed": 1,
      "meshRotationMin": [
        0,
        0,
        0
      ],
      "meshRotationMax": [
        0,
        0,
        0
      ],
      "meshSpinMin": [
        0,
        0,
        0
      ],
      "meshSpinMax": [
        0,
        0,
        0
      ]
    },
    "decal": {
      "size": [
        2,
        2
      ],
      "depth": 0.3,
      "edgeFade": 0,
      "normalCutoff": 0.5
    },
    "trail": {
      "maxPoints": 64,
      "pointLifetime": 0.35,
      "sampleInterval": 0.01666667,
      "minDistance": 0.01,
      "startWidth": 0.2,
      "endWidth": 0,
      "tilingDistance": 0,
      "edgeMode": "CenterlineCamera",
      "edgeOffset": [
        0,
        1,
        0
      ],
      "fadeWithAge": true
    },
    "screenPost": {
      "profile": "ZoomBlur",
      "intensityStart": 2,
      "intensityEnd": 0,
      "intensityLerp": true,
      "secondaryIntensity": 0,
      "frequency": 1,
      "tint": [
        1,
        1,
        1,
        1
      ],
      "randomSeed": 1
    }
  },
  "parts": []
}
```

## 부록. 계획 작성 기준 소스

| 실제 checkout 상대 경로 | SHA-256 | 감지 인코딩 |
|---|---|---|
| Engine/Public/GameObject.h | `35c60626e59e3e44b1085070327298cf896969676971b4291bc710eb8adf6c6b` | UTF-8 |
| Engine/Private/Renderer.cpp | `2a22c7195e71834948cd40f28af1d3efba4b7a3844bc9a68a3d5dd6ba973f7eb` | UTF-8 |
| Client/Public/MapAssetObject.h | `1ee9fd21649b2ca12222db6449040e34ae320bcff175bd05eef4ff4e1d556b3a` | UTF-8 |
| Client/Private/MapAssetObject.cpp | `173c56d9e7f090826088955b1978b50d80e834e230f6c0a17cbdc4ad7cefb151` | UTF-8 |
| Client/Public/KoukuSaydonPresentationPlayer.h | `7594f8a2675bed1c7afb8f345ee39060003e9f6bc4119192151a5ff3e7f78bc2` | UTF-8 |
| Client/Private/KoukuSaydonPresentationPlayer.cpp | `de92b4a2c3430b6fb203a35cba9b5a1ad232afd79a99e57f8e7665ee12a95cdf` | UTF-8 |
| Data/Effects/V2/Authored/cardmaze.symbol.heart.effectv2.json | `f0f0fbe73ab834fa2f46296f2678a254fad097ef3e95fa5bcc196f7b96c3c1eb` | UTF-8 |
| Data/Effects/V2/Authored/cardmaze.symbol.spade.effectv2.json | `0a9017038a783431e3e468094f99407cd3e7067a1950bc4b8ca16cf001fbbf06` | UTF-8 |
| Data/Effects/V2/Authored/cardmaze.symbol.club.effectv2.json | `1c83752cb69f024e6e0593add362aa1db9688c9b28c6cbde3eea4a5ec2cb2bf4` | UTF-8 |
| Data/Effects/V2/Authored/cardmaze.symbol.diamond.effectv2.json | `2a2f3a737cc5ebde39ab4d68da20a7f32c3b851c6cb6acd21ad2a9ff91cd1a49` | UTF-8 |

이 해시는 다른 세션 변경을 덮어쓰지 않기 위한 소스 비교 자료이며, 리소스 배포의 manifest/lock 요건이 아니다.
