# 쿠크 Effect 박스 전체 회전 pivot 구현 계획

## G00. 요청과 현재 실측

### 대상 박스와 Effect 문서

아래 값은 2026-09-18 기준 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` revision 1409를 읽기 전용으로 확인한 결과다.
이 파일은 사용자의 Workbench가 계속 저장하므로 이번 작업에서 에이전트가 쓰지 않는다.

| 박스 | start / duration ms | rotationDegrees | anchor |
|---|---|---|---|
| `KAKULSAYDON_G1_PATTERN_15.presentation.9` | 4500 / 3167 | [-6.2, -18.05, 1.2] (사용자가 시도한 값) | BOSS, followBoss true, bone "", BODY |
| `KAKULSAYDON_G1_PATTERN_15.presentation.12` | 11667 / 3167 | [0, 0, 0] | 같음 |
| `KAKULSAYDON_G1_PATTERN_15.presentation.15` | 18834 / 2667 | [0, 0, 0] | 같음 |

세 박스는 모두 resource `kakulsaydon.g1.presentation.56`(V1_EFFECT `effect.kouku.gate2.4219776.stage2.full.restore`)을 쓴다.
같은 패턴의 logic.4/5/6(`kakulsaydon.g1.logic.31`, ENTER_AREA, `bossChargeDistanceM` 7, `chargeYawOffsetDegrees` 90)은 세 박스와 정확히 같은 시각(4500/11667/18834 ms)에 1167 ms 동안 실행된다.

Effect 문서(LF, version 15)에는 요소가 9개 있고, 두 그룹으로 나뉜다.

- **FX_State_01 그룹 (4개)**
  - 요소: particle 3개와 TRAIL `_2`(cascadeRibbonV1). TRAIL도 파티클 시뮬레이션 요소다.
  - 부착: `follow=true`, slot `FX_State_01`, bone `bip001-spine1`
  - 시작: startDelay 0.115242 s
  - 방출: 모두 spawn-per-unit이다.
- **root 그룹 (5개)**
  - 요소: `_19`, `_25`(local space), `_27`, `_28`, `_29`
  - 부착: `follow=false`, slot `root`, snapshot yaw -90
  - 시작: startDelay 0.19218 s

### 현재 합성 경로

Workbench Preview와 제품 재생은 같은 함수를 거친다.

1. **`Make_Pivot`** (`KoukuSaydonPresentationPlayer.cpp` L420-459)
   - `F = S·R(rotationDegrees)·T(positionOffset)·N(anchor)`
   - `N`은 boss root, 또는 bone의 TARGET_YAW pivot의 정규화 basis다.
2. **`Effect_PivotSampler`** (L477-501)
   - BOSS + followBoss이면 root history를 `iStartMs + t`에서 fixed step마다 다시 샘플한다.
   - 그 결과 `F_t`는 매 step 달라진다.
3. **`Build_SourceAnchorWorlds`** (L527-595)
   - `slot_t = socketLocal·bone_t·BoneRoot·inverse(ownerPivot)·RootWorld_t`
   - `BoneRoot·inverse(ownerPivot)`는 actor scale만 남는다. 따라서 `slot_t = socket·bone_t·S_actor·F_t`다.
4. **`Effect_V1TransformProvider`** (L748-767)
   - 호출 경로: `CEffectPresentationService::Seek_WorldRoot` → `Commit_ExternalTransformHistorySample` → `CEffectPlayback::Seek_WithTransformHistory` / `Update_WithTransformHistory`(L4099-4273)
   - fixed step마다 `m_SourceAnchorWorlds`와 `Step(RootWorld_t)`를 적용하고, 마지막에 `Rebuild_Frame(FinalSample.RootWorld)`를 부른다.
   - 렌더되는 root는 모두 이 provider가 소유한다. `row.pivot`은 spawn과 `Update_WorldRoot`의 placeholder일 뿐이다.
5. **`Effect_Playback.cpp`**
   - follow 요소의 부모는 slot world다(L7763-7769).
   - follow=false 요소는 localTime ≥ 0이 되는 첫 step에 `ActionRootWorld = RootWorld`를 한 번 저장한다(L4350-4360).
   - 입자는 태어날 때 `SpawnRootWorld = ElementWorld`를 고정한다(L4993). ribbon 점은 이렇게 고정된 birth에서 만들어진다.

### 회전 하나로 전체를 돌릴 수 없는 이유

- **FX_State_01 그룹**
  - R은 매 step 그 순간의 root를 중심으로 bone offset만 돌린다.
  - trail 점과 birth는 world snapshot이므로, 그 경로는 Server charge가 옮기는 7 m root 이동 방향과 같다.
  - R은 이 이동 방향 자체를 돌리지 못한다.
- **root 그룹**
  - 박스 시작 약 0.2 s에 캡처된 root 한 점을 중심으로 돈다.
  - 이 점은 FX_State_01 그룹의 pivot과 다르다.
  - 결과적으로 한 Effect 안에 pivot이 두 개 있고, 그중 하나는 계속 움직인다.
- **기존 조작도 같은 한계를 갖는다.**
  - Workbench의 multi-box `Group rotation`은 각 박스의 RotationDegrees/PositionOffset만 다시 쓴다. 결국 같은 `Make_Pivot`을 쓴다.
  - Effect Tool V1의 Element Group Rotation, Anchor Rotation, System Yaw는 모두 step마다 다시 적용되는 부모 기준이다.
  - FX_State_01 그룹은 runtime carrier(`_2`)가 있어 group rotation 자체가 비활성이다.
- **Workbench local preview로는 차이가 보이지 않는다.**
  - `CKoukuSaydonPreviewRootMotion::Allows_AutomaticMotion`이 charge logic이 있는 패턴의 root 이동을 끈다.
  - 그래서 root가 정지해 있고, 회전이 맞아 보인다. 어긋남은 Server 재생에서만 드러난다.
- **기본 의미는 바꾸지 않는다.** 이미 0이 아닌 rotation을 가진 V1 BOSS follow 박스가 62개 있으므로, 새 동작은 opt-in으로 둔다.

## G01. 계약: occurrence `transformPivot`

### 키와 허용 범위

| 항목 | 값 |
|---|---|
| JSON key | `transformPivot` (presentation occurrence optional) |
| enum 값 | `"LIVE_ANCHOR"` (기본, 현재 동작), `"CAPTURED_ANCHOR"` |
| C++ 필드 | `std::string strTransformPivot = "LIVE_ANCHOR";` |
| 저장 | 기본값이면 쓰지 않는다. 기존 문서, Product bytes, SHOWTIME/pursuit digest가 모두 그대로다. |
| 허용 | `CAPTURED_ANCHOR`는 EFFECT + `V1_EFFECT`/`V1_ELEMENT` + anchorKind `BOSS` + followBoss true에서만 허용한다. bone은 BODY/WEAPON 모두 허용한다. |
| 거부 | MAP/WORLD anchor, followBoss false, V2 GROUP/LEAF, Collider/Light/Sound/Camera, bundle common row, SHOWTIME template |
| formatVersion | composition 3과 patternbindings 1을 유지한다. fit/loop와 같은 optional key 선례를 따른다. |

### 수식과 followBoss 의미

- `P = S·R·T`는 occurrence의 Position/Rotation/Scale이다.
- `I_t`는 placement를 identity로 둔 anchor pivot이다. 같은 `Effect_PivotSampler`를 offset 0, rotation 0, scale 1인 박스 사본으로 샘플한다.
- `F_0 = P·I_0`는 박스 시작 시점의 placed pivot이다.
- `Q = inverse(I_0)·F_0 = inverse(I_0)·P·I_0`는 박스마다 고정된 값이다.
- step t에서 provider는 다음 두 값을 돌려준다.
  - `RootWorld'_t = I_t·Q`
  - `slot'_t = Build_SourceAnchorWorlds(root = I_t)·Q`

뜻은 이렇다. followBoss는 그대로 켜져 있어서 Effect는 boss root, bone 이동, 회전을 계속 따라간다.
다만 그렇게 따라가서 생긴 world 궤적 전체를 박스 시작 anchor frame으로 옮긴다. 그 frame 안에서 P를 한 번 적용하고 다시 world로 돌린다.
FX_State_01의 bone 경로, 그 경로에 이미 뿌린 trail·particle, 0.192 s에 찍힌 root snapshot, local/world space 입자가 모두 하나의 pivot을 공유한다.
그 pivot은 박스 시작 시점의 anchor 원점이다.
BOSS pivot이면 root, bone을 고른 박스면 그 bone pivot이다.

### 반드시 지킬 불변식

1. anchor가 박스 동안 움직이지 않으면 CAPTURED와 LIVE의 결과가 수치상 같다. 이유: `I_t = I_0`이므로 `I_0·Q = F_0`다.
2. P가 identity면(pattern 15의 .12/.15) 켜도 결과가 같다.
3. yaw만 있는 회전과 uniform scale은 캡처 시점 facing과 무관하다. Q는 캡처된 root 위치를 중심으로 한 world Y 회전과 등방 scale이 된다.
4. pitch/roll과 positionOffset은 박스 시작 순간 anchor facing의 축을 쓴다. 현재 LIVE도 매 step facing에 의존하므로 회귀는 아니다.
5. world gravity(`balwaysinworldspace`)와 camera billboard는 지금처럼 world/camera 축을 유지한다.
6. scale은 박스 시작 pivot을 중심으로 이동 경로 길이까지 늘리고 줄인다. 전체 회전의 정의상 정상이다.
7. 회전한 Effect의 끝은 boss의 실제 이동선에서 `7 m·sin(yaw)`만큼 벗어난다. "전체 Effect 회전"의 본질이므로 결과 문서에 명시한다.

### 박스 시작 facing에 대한 실측과 저작 지침

- **Server가 charge 시작 tick에 facing을 한 번에 바꾼다.** 이동을 캡처한 tick에 `boss.fYawDegrees = travel yaw + 90`으로 바로 설정한다(`KoukuSaydonLogicRuntime.cpp` L1158-1211).
- **Client 표시 facing은 늦게 따라온다.**
  - Client NPC는 `INTERPOLATION_DELAY_TICKS = 2`와 `TURN_DEGREES_PER_SECOND = 720`으로 보간한다(`Npc.h` L49-53, L132-190).
  - 반면 presentation clock은 가장 최근 snapshot tick을 쓴다(`KoukuSaydonPresentationPlayer.cpp` L2549-2553).
  - 그래서 박스 시작 순간의 표시 root는 charge 이전 facing과 이전 위치다. facing은 최대 약 0.35 s 동안 회전한다.
- **따라서 박스 시작 pivot의 위치는 돌진 시작점과 일치한다.** yaw 회전도 매번 같은 상대 각도로 적용된다(불변식 3).
- **pitch/roll의 축은 판마다 달라질 수 있다.** 기울임 축은 charge 이전 facing을 쓰고, 그 값은 이전 charge 대상에 따라 바뀐다.
- **사용자 조정 지침:** 이번 목표(Kouku 머리 쪽 방향)는 yaw로 맞춘다. pitch/roll은 0 또는 작은 값을 권장한다.
- **후속 작업 후보:** 기울임까지 판마다 같아야 하면 Server snapshot yaw로 capture frame을 만든다. 이번 범위에는 넣지 않는다.

### 이번 범위가 아닌 것

- WORLD anchor, MAP, V2 GROUP/LEAF의 전체 회전
- Effect Tool V1의 Kouku pattern context preview. 이 경로는 원래 occurrence rotation을 적용하지 않는다.
- Workbench local preview에서 charge 이동을 시뮬레이션하는 기능
- Server, Shared protocol, bootstrap 변경
- Effect 문서, `Effect_Playback.cpp`, `Effect_PresentationService.cpp`의 공통 runtime 의미 변경

## G02. Composition 문서 계약

### `Client/Public/KoukuSaydonCompositionDocument.h` (ASCII, CRLF, git clean)

`KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE`의 `bool_t bLoopEffectToDuration = false;` 바로 아래에 멤버를 추가한다.
`operator== = default`와 whole-struct 복사가 새 필드를 자동으로 다룬다.
이 헤더를 포함하는 TU 21개가 Product 빌드에서 다시 컴파일된다. 소스 수정은 필요 없다.

```cpp
		// LIVE_ANCHOR re-applies this placement around every sampled anchor. CAPTURED_ANCHOR
		// applies it once, in the anchor frame sampled at iStartMs, to the whole following V1 Effect.
		std::string strTransformPivot = "LIVE_ANCHOR";
```

### `Client/Private/KoukuSaydonCompositionDocument.cpp` (ASCII, CRLF, git clean, L1550-1551의 bare LF 두 줄 보존)

1. **`Read_PresentationOccurrence`** (L851-907)
   - `Has_Properties`의 optional 목록 끝 `"fitEffectToDuration", "loopEffectToDuration"` 뒤에 `"transformPivot"`을 추가한다.
   - 반환 chain의 `Read_PresentationText(value, "anchorKind", row.strAnchorKind) &&` 바로 아래에 다음을 추가한다.
     `Read_PresentationText(value, "transformPivot", row.strTransformPivot) &&`
   - key가 없으면 기본값 `LIVE_ANCHOR`를 유지한다.
2. **Pattern occurrence 검증** (`Validate`, L1794-1798)
   - `"Effect lifetime cannot stretch and loop simultaneously."` 검사 바로 아래에 추가한다.

```cpp
				if ((row.strTransformPivot != "LIVE_ANCHOR" && row.strTransformPivot != "CAPTURED_ANCHOR") ||
					(row.strTransformPivot == "CAPTURED_ANCHOR" && (resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
						(resource->strResourceKind != "V1_EFFECT" && resource->strResourceKind != "V1_ELEMENT") ||
						row.strAnchorKind != "BOSS" || !row.bFollowBoss)))
				{ outStatus = "Captured Effect transform requires a following BOSS V1 Effect: " + row.strOccurrenceId; return false; }
```

3. **Bundle common presentation 검증** (L1310-1330)
   - 실패 조건의 `row.fBrightnessMultiplier != 1. ||` 앞에 `row.strTransformPivot != "LIVE_ANCHOR" ||`를 추가한다.
   - Camera 전용 row는 기본값만 허용한다.
4. **Pattern writer** (L4595-4596)와 **bundle writer** (L4670-4671)
   - 각각 `loopEffectToDuration` 줄 바로 아래에 기본값이 아닐 때만 key를 쓰는 줄을 추가한다.
   - 들여쓰기는 pattern writer가 tab 3개, bundle writer가 공백 16칸으로, 기존 줄을 따른다.

```cpp
			if (row.strTransformPivot != "LIVE_ANCHOR") output << ", \"transformPivot\": \"" << CDataJson::Escape(row.strTransformPivot) << "\"";
```

같은 class가 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`도 읽고 쓰므로 한 번의 변경으로 두 문서가 함께 처리된다.
실행 중인 이전 Client는 이 key를 모른다. 새 binary가 저장하기 전에는 문서에 key가 생기지 않으므로 이전 binary가 Pattern을 격리하는 일은 없다.

## G03. 런타임 합성: `Client/Private/KoukuSaydonPresentationPlayer.cpp` (ASCII, CRLF, 다른 세션의 미커밋 변경 있음)

수학을 composition 계층에 두는 이유가 있다.
Effect playback과 service는 이미 provider가 준 RootWorld와 SourceAnchorWorlds만 소비한다.
provider만 바꾸면 bone-follow slot, root snapshot, 입자 birth, ribbon 점, local/world space가 한 번에 같은 변환을 받는다.
다른 owner(Character, Valtan, Effect Tool)가 쓰는 generic runtime은 건드리지 않는다.
`Build_SourceAnchorWorlds`는 Effect Tool(`EffectAuthoringSequencer.cpp` L1156)과 공유하므로 signature를 바꾸지 않는다.

### 1. 새 helper `Compose_CapturedEffectTransform`

익명 namespace 안의 `Effect_SourceClockRate` 정의 바로 아래, `Effect_V1TransformProvider` 바로 위에 둔다.
DirectXMath와 기존 `Valid_SourceMatrix`(L520)만 쓴다. 그래서 G06 probe가 이 함수 원문을 그대로 추출해 검증할 수 있다.

```cpp
// placedAnchor is Make_Pivot's S*R*T*A for the same anchor A. Post-multiplying an unplaced
// per-step world by inverse(A)*placedAnchor expresses it in A and applies S*R*T exactly once.
bool Compose_CapturedEffectTransform(const float4x4_t& unplacedAnchor,
    const float4x4_t& placedAnchor, float4x4_t& transform)
{
    const matrix_t anchor = XMLoadFloat4x4(&unplacedAnchor);
    const matrix_t placed = XMLoadFloat4x4(&placedAnchor);
    if (!Valid_SourceMatrix(anchor) || !Valid_SourceMatrix(placed)) return false;
    const matrix_t value = XMMatrixInverse(nullptr, anchor) * placed;
    if (!Valid_SourceMatrix(value)) return false;
    XMStoreFloat4x4(&transform, value);
    return true;
}
```

### 2. `Effect_V1TransformProvider` (L748-767)

함수 본문 맨 앞에 CAPTURED 분기를 추가한다. 기존 return 문은 LIVE 경로로 그대로 둔다.

```cpp
    if (box.strTransformPivot == "CAPTURED_ANCHOR")
    {
        auto unplacedBox = box;
        unplacedBox.PositionOffset = {0.0, 0.0, 0.0};
        unplacedBox.RotationDegrees = {0.0, 0.0, 0.0};
        unplacedBox.Scale = {1.0, 1.0, 1.0};
        return [unplaced = Effect_PivotSampler(unplacedBox, rootHistory, anchorHistory, exactRoot),
            placed = Effect_PivotSampler(box, rootHistory, anchorHistory, std::move(exactRoot)),
            sourceAnchors = std::move(sourceAnchors), sourceSecondsPerBoxSecond]
            (float seconds, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& output, std::string& error)
        {
            // Source particles keep their own clock; owner/bone history keeps the box clock.
            seconds /= sourceSecondsPerBoxSecond;
            output.SourceAnchorWorlds.clear();
            if (!unplaced || !placed)
            { error = "Captured Effect transform requires a following BOSS anchor."; return false; }
            float4x4_t startAnchor, startPlaced, anchor, transform;
            if (!unplaced(0.f, startAnchor, error) || !placed(0.f, startPlaced, error)) return false;
            if (!Compose_CapturedEffectTransform(startAnchor, startPlaced, transform))
            { error = "Captured Effect anchor at the box start is singular."; return false; }
            if (!unplaced(seconds, anchor, error)) return false;
            const matrix_t placement = XMLoadFloat4x4(&transform);
            XMStoreFloat4x4(&output.RootWorld, XMLoadFloat4x4(&anchor) * placement);
            if (sourceAnchors && !sourceAnchors(seconds, anchor, output.SourceAnchorWorlds, error)) return false;
            for (auto& anchorWorld : output.SourceAnchorWorlds)
                XMStoreFloat4x4(&anchorWorld.second, XMLoadFloat4x4(&anchorWorld.second) * placement);
            error.clear();
            return true;
        };
    }
```

- **입력 검증**
  - sampler가 비어 있으면(MAP 또는 followBoss false) 명시적으로 오류를 반환한다. 검증 계층이 이런 행을 미리 거부하므로 방어용이다.
  - 박스 시작 history가 없으면(세션이 박스 시작 뒤에 시작된 경우) 기존 LIVE와 같은 `Requested root time has not been recorded.` 실패를 반환한다.
- **실패 처리**
  - 호출자 `sampleOne`의 기존 처리(L2183-2190)가 handle을 멈추고 row를 failed로 격리한다. 다른 row는 유지한다.
- **source anchor**
  - `sourceAnchors`는 기존 `Make_SourceAnchorSampler` lambda다. 그 lambda가 CAMERA_VIEW 부착을 먼저 거부하므로(L722-724), Q를 곱하는 map에는 bone slot만 있다.
- **이후 단계**
  - 뒤이어 `Collect_TransformHistorySample`의 ModelCue anchor provider가 이미 Q가 적용된 RootWorld를 받는다.
- **비용**
  - 추가 비용은 step마다 pivot 샘플 2회와 4x4 역행렬 1회다. 새 heap 상태는 없다.

### 3. `Read_Occurrence` (L279-359, Product patternbindings reader)

`return box;` 바로 위에 추가한다. 이 한 곳이 Product Pattern(L1547), bundle common lane(L1632), targeted visual(L1163)을 모두 덮는다.

```cpp
    if (row.Find("transformPivot")) box.strTransformPivot = Text(row, "transformPivot");
    if (box.strTransformPivot != "LIVE_ANCHOR" && (box.strTransformPivot != "CAPTURED_ANCHOR" || kind != KIND::EFFECT ||
        (Field(row, "resourceKind").Get_String() != "V1_EFFECT" && Field(row, "resourceKind").Get_String() != "V1_ELEMENT") ||
        box.strAnchorKind != "BOSS" || !box.bFollowBoss))
        throw std::runtime_error("Captured Effect transform requires a following BOSS V1 Effect: " + box.strOccurrenceId);
```

### 4. `Validate_EffectAnchor` (L89)

첫 anchor/boneTarget/emission 검사(`"Invalid Effect anchor, bone or World emission index"`) 바로 아래에 추가한다.
Preview admission(L2741, L3094)과 live geometry preview(L4572)가 같은 규칙을 쓴다.

```cpp
    if ((box.strTransformPivot != "LIVE_ANCHOR" && box.strTransformPivot != "CAPTURED_ANCHOR") ||
        (box.strTransformPivot == "CAPTURED_ANCHOR" && (box.strAnchorKind != "BOSS" || !box.bFollowBoss)))
        return reject("Captured Effect transform requires a following BOSS anchor");
```

### 5. `Preview_PresentationGeometry` (L4527-4659)

- EFFECT 분기의 `edited.iWorldEmissionIndex = occurrence.iWorldEmissionIndex;` 바로 아래에 다음을 추가한다.
  `edited.strTransformPivot = occurrence.strTransformPivot;`
- 끝부분의 `box->Scale = occurrence.Scale;` 바로 아래에 다음을 추가한다.
  `if (resource->eKind == KIND::EFFECT) box->strTransformPivot = occurrence.strTransformPivot;`
- `anchorChanged`에는 넣지 않는다.
  - 모드가 바뀌면 기존 V1 분기(L4615-4620)가 `edited`로 만든 provider와 `bRebuildHistory=true`로 모든 fixed step을 다시 재생한다.
  - 다음 프레임부터는 `sampleOne`이 갱신된 `box`로 같은 provider를 만든다.

### 변경하지 않는 곳

- `Sample`, `sampleOne`의 spawn과 Seek 호출
- `Make_SourceAnchorSampler`, `Build_SourceAnchorWorlds`
- `Client/Public/KoukuSaydonPresentationPlayer.h`: 새 row 상태가 없다.
- `Effect_Playback.cpp`, `Effect_PresentationService.cpp`, `EffectV2_*`
- 금지 파일 `MainApp.cpp`: occurrence struct를 통째로 전달하므로(L2008) 수정할 필요가 없다.

## G04. Workbench Box Detail: `Client/Private/KoukuSaydonActionWorkbench.cpp` (UTF-8 no BOM, CRLF, 한글 포함, 다른 세션의 미커밋 변경 있음)

새 줄은 모두 ASCII다. 기존 non-ASCII byte 201개와 CRLF를 보존한다.

1. **`Same_ColliderGroupFrame`** (L1304-1311)
   - 반환식에 `a.strTransformPivot == b.strTransformPivot &&`를 추가한다.
   - 이 조건을 만족하는 박스끼리만 multi-box Group rotation을 함께 적용한다.
2. **`Set_FixedWorldEffectPlacement`** (L1338-1344)
   - `edit.strTransformPivot = "LIVE_ANCHOR";`를 추가한다.
   - MAP으로 전환하는 모든 경로(L1963, L10392, L10806, L11769, L11833, L11858)가 이 함수로 초기화된다.
3. **`Copy_PresentationPlacement`** (L1354-1368)
   - effect 분기 끝에 `target.strTransformPivot = source.strTransformPivot;`를 추가한다.
   - staging, Save 병합(L1519-1533), Revert(L12371), Synchronize(L4225)가 새 필드를 함께 옮긴다.
4. **`Same_PresentationPlacement`** (L1370-1378)
   - effect 비교에 `&& left.strTransformPivot == right.strTransformPivot`를 추가한다.
   - 변경이 감지되면 L12055가 geometry preview를 요청한다.
5. **`Valid_PresentationPlacement`** (L1380-1388)
   - `if (resource.eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT) return true;` 바로 아래에 추가한다.

```cpp
		if (value.strTransformPivot != "LIVE_ANCHOR" && (value.strTransformPivot != "CAPTURED_ANCHOR" ||
			(resource.strResourceKind != "V1_EFFECT" && resource.strResourceKind != "V1_ELEMENT") ||
			value.strAnchorKind != "BOSS" || !value.bFollowBoss)) return false;
```

6. **`Render_PresentationBoxDetails`**
   - 위치: `if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)` 블록의 첫 줄, `if (!Same_PresentationPlacement(placementBeforeControls, edit, true)) previewGeometry();` 바로 위다.
   - 이 위치는 Rotation/Scale, Anchor, Bone, Copy Collider anchor, Follow anchor 조작이 모두 끝난 뒤다. 그래서 같은 프레임의 anchor 변경을 정규화한 다음 한 번의 preview 요청으로 반영된다.

```cpp
		if (definition.strResourceKind == "V1_EFFECT" || definition.strResourceKind == "V1_ELEMENT")
		{
			if (edit.strAnchorKind == "BOSS" && edit.bFollowBoss)
			{
				bool captured = edit.strTransformPivot == "CAPTURED_ANCHOR";
				if (ImGui::Checkbox("Transform whole Effect from box-start anchor##PresentationBox", &captured))
					edit.strTransformPivot = captured ? "CAPTURED_ANCHOR" : "LIVE_ANCHOR";
				ImGui::TextWrapped("%s", captured ?
					"Position / Rotation / Scale move every trail, particle and root snapshot about this anchor at box start. Yaw is exact; pitch/roll use the boss facing at box start." :
					"Position / Rotation / Scale are re-applied around the live anchor at every step.");
			}
			else if (edit.strTransformPivot != "LIVE_ANCHOR")
			{
				edit.strTransformPivot = "LIVE_ANCHOR";
				m_strStatus = "Box-start transform needs a following Boss anchor; restored the live anchor.";
			}
		}
```

Apply(`Set_PresentationBox`, whole-struct 교체), Save(`Save_Atomic`), Revert는 추가 코드 없이 새 필드를 보존한다.
`KoukuSaydonActionWorkbench.h`는 변경하지 않는다.

## G05. Projector, publisher, Server

### `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` (ASCII, CRLF, git clean)

1. **`PRESENTATION_OCCURRENCE_DEFAULTS`** (L4461-4468)
   - `"worldEmissionIndex": 0,` 뒤에 `"transformPivot": "LIVE_ANCHOR",`을 추가한다.
   - `_keys`의 Pattern 검사(L4574)와 bundle 검사(L1597)가 key를 허용하게 된다.
2. **`_validate_presentation_occurrences`** (L4567-4669)
   - 루프 끝의 WEAPON boneTarget 검사 바로 아래에 추가한다.
   - `followBoss`의 boolean 검사가 이미 끝난 위치다.

```python
        pivot = normalized["transformPivot"]
        if not isinstance(pivot, str) or pivot not in {"LIVE_ANCHOR", "CAPTURED_ANCHOR"}:
            raise CompositionError("presentation transformPivot must be LIVE_ANCHOR or CAPTURED_ANCHOR")
        if pivot == "CAPTURED_ANCHOR" and (not effect or resources[box["resourceId"]].get("resourceKind") not in {"V1_EFFECT", "V1_ELEMENT"} or
                                           normalized["anchorKind"] != "BOSS" or not normalized["followBoss"]):
            raise CompositionError("CAPTURED_ANCHOR transformPivot requires a following BOSS V1 Effect")
```

3. **기본값 제외 집합**
   - `_project_presentation_occurrence`(L4791)와 scene-profile 투영(L4945)의 제외 집합 `{"brightnessMultiplier", "boneTarget", "fitEffectToDuration", "loopEffectToDuration"}`에 `"transformPivot"`을 추가한다.
   - 저장된 key만 그대로 복사되므로, key가 없는 행의 Product bytes와 pursuit/SHOWTIME `sha256` visual ID가 바뀌지 않는다.
4. **`_showtime_visual_template`** (L4818-4823)
   - 거부 조건 끝에 `or row.get("transformPivot", "LIVE_ANCHOR") != "LIVE_ANCHOR"`을 추가한다.
   - message는 `"...; random volleys also admit following BOSS rows without a captured transform pivot"`으로 바꾼다.
   - targeted visual은 MAP 행을 BOSS로 다시 쓰는 private 복사본을 쓰므로 CAPTURED를 받지 않는다.

### `Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py` (UTF-8, CRLF 5820줄 + bare LF 53줄 혼재)

`test_effect_source_loop_window_is_explicit_and_excludes_stretch`(L4192-4215) 바로 아래에 CRLF로 새 메서드를 추가한다.

```python
    def test_effect_transform_pivot_is_explicit_following_boss_v1_only(self):
        pattern_id = "KAKULSAYDON_G1_PATTERN_15"
        resource = dict(resourceId="effect.dash", kind="EFFECT", resourceKind="V1_EFFECT")
        row = dict(occurrenceId=pattern_id + ".presentation.9", resourceId="effect.dash",
                   startMs=4500, durationMs=3167)
        pattern = dict(patternId=pattern_id, nextPresentationOccurrenceOrdinal=10,
                       presentationOccurrences=[row])
        def validate():
            subject._validate_presentation_occurrences(pattern, {"effect.dash": resource}, 21501, {})
        validate()
        self.assertNotIn("transformPivot", subject._project_presentation_occurrence({"worlds": []}, pattern, row, resource))
        row["transformPivot"] = "CAPTURED_ANCHOR"
        validate()
        self.assertEqual("CAPTURED_ANCHOR",
                         subject._project_presentation_occurrence({"worlds": []}, pattern, row, resource)["transformPivot"])
        row["followBoss"] = False
        with self.assertRaisesRegex(subject.CompositionError, "CAPTURED_ANCHOR"):
            validate()
        row["anchorKind"] = "MAP"
        with self.assertRaisesRegex(subject.CompositionError, "CAPTURED_ANCHOR"):
            validate()
        del row["anchorKind"]
        row["followBoss"] = True
        resource["resourceKind"] = "V2_GROUP"
        with self.assertRaisesRegex(subject.CompositionError, "CAPTURED_ANCHOR"):
            validate()
        resource["resourceKind"] = "V1_EFFECT"
        row["transformPivot"] = "ROOT"
        with self.assertRaisesRegex(subject.CompositionError, "LIVE_ANCHOR or CAPTURED_ANCHOR"):
            validate()
```

### Publisher와 Server

- `Tools/Build/BuildDomains.json`의 `koukusaydon.product` domain은 projector를 tool로 등록하고 있다. projector가 바뀌면 receipt가 무효가 되어 다음 publish 때 다시 생성된다.
- `Invoke-BuildDomainOwner.ps1`, `Publish-WorldGameplay.ps1`, `Publish-GameplayBalance.ps1`는 presentation occurrence key를 읽지 않는다. `Publish-GameplayBalance.ps1`가 검사하는 `rotationDegrees`는 WORLD occurrence placement다.
- Server와 Shared는 patternbindings와 presentation occurrence를 참조하지 않는다. protocol, bootstrap, Server 재시작 조건은 바뀌지 않는다.

## G06. 에이전트 검증

Client/Server를 실행하거나 조작하지 않고, 화면도 캡처하지 않는다. Product 빌드와 publish도 하지 않는다. 산출물은 모두 `out/KoukuEffectWholeRotation20260918/`에 격리한다.

1. **편집 전 기준 기록**
   - 변경할 파일 6개(아래 요약 표)의 BOM, CRLF/bare-LF 수, non-ASCII byte 수를 Python으로 기록한다. 편집 뒤 같은 값인지 비교한다.
   - `git show HEAD:Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`를 `project_kouku_saydon_composition.before.py`로 저장한다.
2. **`git diff --check`**
   - 대상은 이번 변경 파일이다. 기존에 다른 세션이 만든 diff의 경고는 따로 구분한다.
3. **격리 컴파일**
   - `compile.cmd`는 `out/WorldLevelEdit20260917/compile.cmd`와 같은 vcvars 호출(`18\Insiders ... 10.0.26100.0 -vcvars_ver=14.44`)을 쓴다.
   - `compile.rsp`는 `compile-groupC.rsp`의 flag(`/MDd /D_DEBUG /DUNICODE /EHsc /utf-8 /std:c++20 /permissive- /bigobj /Od /Zi /FS /MP4`와 `/I Client\Public`, `/I EngineSDK\Inc`, `/I Engine\External\imgui`, `/I Shared\Public`, vcpkg include)를 복사한다. `/Fo`와 `/Fd`는 새 폴더를 가리킨다.
   - 대상 TU: `KoukuSaydonPresentationPlayer.cpp`, `KoukuSaydonCompositionDocument.cpp`, `KoukuSaydonActionWorkbench.cpp`, 헤더 소비자 확인용 `KoukuSaydonPresentationPlayer_LogicPreview.cpp`, `KoukuSaydonBossTool.cpp`
   - 기대: exit 0, 로그에 `error C` 없음
   - 금지 파일인 `MainApp.cpp`는 다른 세션이 편집 중이므로 대상에서 뺀다.
4. **행렬 합성 probe** (`transform_probe.cpp`, `probe.cmd`)
   - `extract_probe.py`가 제품 cpp에서 `Valid_SourceMatrix`와 `Compose_CapturedEffectTransform`의 원문을 그대로 추출해 `probe_extracted.inl`에 쓴다.
   - `Make_Pivot`의 BOSS/no-bone 합성(`S·RollPitchYaw·T·normalized root`)과 provider의 `I_t·Q`, `slot_t·Q`는 같은 DirectXMath 호출로 재현한다.
   - 판정은 모두 PASS여야 하고, exit 0이어야 한다.
     - **A. 정지 동등성**
       - 입력: yaw 37°, actor scale 1.7, rotation [-6.2,-18.05,1.2], offset [0.3,0.1,-0.4], uniform scale 1.2와 비등방 (1,2,0.5)
       - 기대: `I·Q`와 `P·I`의 최대 원소 차 ≤ 1e-4. `socket·bone·S_actor·I·Q`와 `socket·bone·S_actor·P·I`의 차도 ≤ 1e-4다.
     - **B. 결함 재현**
       - 입력: root가 1.167 s 동안 7 m 이동하고 facing이 0.35 s 동안 90° 회전한다. yaw는 -18.05, socket·bone은 고정이다.
       - 기대: facing이 안정된 0.5 s→1.167 s 구간에서 LIVE slot 원점의 변위 방향이 identity 경로와 같다. 각도 차 < 0.5°로, 회전이 이동 방향을 바꾸지 못함을 보인다.
     - **C. 전체 회전**
       - 기대: 모든 sample 시각에서 CAPTURED slot 원점과 0.192 s root snapshot 원점이 identity 원점을 박스 시작 root 원점 중심으로 yaw -18.05° 돌린 위치와 같다(위치 오차 ≤ 1e-4 m).
       - yaw-only `Q`는 charge 이전 facing 0°와 90°에서 같다(원소 차 ≤ 1e-5).
     - **D. 등방 scale 2**
       - 기대: 박스 시작 pivot에서의 거리가 2배가 된다.
     - **E. 기울임 경계 (INFO)**
       - pitch -6.2가 있으면 facing 0°/90°의 `Q`가 달라진다. G01의 경계를 수치로 기록하며, 실패로 치지 않는다.
5. **Composition roundtrip probe** (`composition_probe.cpp`)
   - 링크 대상: `KoukuSaydonCompositionDocument.cpp`, `KoukuSaydonAnimationActionDocument.cpp`, `DataJson.cpp`, `ProjectDataRoot.cpp`
   - `Data` 원본 대신 live composition을 한 번 읽어 저장한 `composition.snapshot.json`만 쓴다.
   - `Parse_Text`, `Serialize` 결과: 원본에는 `"transformPivot"`가 0회 나온다. .9를 CAPTURED로 바꾸면 1회 나오고, 다시 parse한 필드가 같다.
   - `Validate`: CAPTURED .9는 통과한다. `bFollowBoss=false`나 anchor MAP이면 새 메시지로 거부한다.
   - 링크 closure가 위 TU를 넘으면 실행하지 않고 미실행으로 기록한다.
6. **Python 계약**
   - 명령: `PYTHONPATH=. PYTHONIOENCODING=utf-8 python -B -m unittest Tools.KoukuSaydonPipeline.test_project_kouku_saydon_composition -k transform_pivot -k effect_source_loop_window`
   - 기대: 두 test 모두 OK
   - 가능하면 같은 module 전체도 실행한다. 빌드 runner와 겹치지 않게 한다.
7. **Product byte 안정성** (`projector_stability.py`)
   - live source JSON을 한 번 읽는다.
   - `before` module과 수정된 projector 각각에서 `_publication_session(final_check=False)` 안에 `prepare_publication` → `projected_outputs`를 실행한다.
   - `KoukuSaydonEncounter.json`과 `KoukuSaydon.patternbindings.json`의 bytes와 sha256이 같아야 한다.
   - module 경로 문제로 실행할 수 없으면 편집 전후 `--mode validate`의 exit code와 출력이 같은지로 대신한다.

미실행으로 남는 항목은 네 가지다. RESULT에 분리해 기록한다.

- Product 빌드
- Client/Server 실행
- publish
- 화면 판정

## G07. 사용자 반영 순서와 남은 경계

1. 다른 세션의 MainApp/WorldObjectTool 작업이 끝났는지 확인한다. Workbench의 미저장 draft는 Save한다.
2. Client를 종료한다. EXE 링크 잠금 때문이며, 데이터 반영을 위한 조건은 아니다.
3. `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`로 Product를 빌드한다. Server 소스는 바뀌지 않았다.
4. 순서대로 조작한다.
   1. Client를 실행한다.
   2. F1 → Action Workbench → Boss KoukuSaydon → `쿠크_거미카운터` → `.presentation.9`를 선택한다.
   3. Box Detail에서 `Transform whole Effect from box-start anchor`를 켠다.
   4. Rotation의 yaw로 Kouku 머리 방향을 맞추고, pitch/roll은 0부터 시작한다.
   5. Apply → Save.
   6. `.12`와 `.15`도 필요하면 같은 설정과 yaw를 넣는다.
5. `Publish All Patterns` 뒤 Server의 Complete Play 또는 Play Isolated로 charge 중 방향을 확인한다.
   - local preview는 charge 이동이 없어 LIVE와 똑같이 보이므로 판정에 쓰지 않는다.
   - 조정 반복은 Save → Publish → Server 재생 순서다.
6. 최종 visual PASS는 사용자의 서면 판정으로만 기록한다.

같은 변경 단위에서 갱신할 문서는 다음과 같다.

- `.md/GB/09-18/2026-09-18_KOUKU_EFFECT_WHOLE_ROTATION_PIVOT_RESULT.md`: 실제 diff, G06 실행 결과, 미실행 항목
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` L1689 `loopEffectToDuration` 문단 아래: `transformPivot` 한 문단(허용 조합, 수식, facing 경계, local preview 한계)
- `.md/GB/gotchas.md`와 `.md/GB/렌더링이펙트복원V2.md`: "follow Effect의 박스 rotation은 step마다 live anchor 기준이라 world snapshot 경로를 돌리지 못한다. 전체 회전은 `transformPivot=CAPTURED_ANCHOR`를 쓴다."와 Npc 보간·720°/s 회전 때문에 생기는 박스 시작 facing 경계

남은 경계는 다음과 같다.

- pitch/roll 축은 박스 시작 순간 Client에 표시된 facing을 따르므로 charge 박스에서 판마다 달라질 수 있다. 필요하면 후속으로 Server snapshot yaw capture frame을 만든다.
- 회전한 streak는 boss의 실제 이동선에서 벗어난다. 이 의미가 사용자 의도와 다르면, 예를 들어 streak가 몸을 따라가면서 내용만 기울어야 한다면, 다른 계약이 필요하다.
- Effect Tool V1의 pattern context preview는 occurrence rotation을 적용하지 않는다.
- WORLD anchor와 V2 Effect에는 적용하지 않는다.

## 변경 파일 요약

| 파일 | 함수/위치 | 인코딩·줄끝 |
|---|---|---|
| `Client/Public/KoukuSaydonCompositionDocument.h` | `KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE::strTransformPivot` | ASCII, CRLF |
| `Client/Private/KoukuSaydonCompositionDocument.cpp` | `Read_PresentationOccurrence`, `Validate`(pattern·bundle), pattern/bundle writer | ASCII, CRLF(+기존 bare LF 2) |
| `Client/Private/KoukuSaydonPresentationPlayer.cpp` | `Compose_CapturedEffectTransform`(신규), `Effect_V1TransformProvider`, `Read_Occurrence`, `Validate_EffectAnchor`, `Preview_PresentationGeometry` | ASCII, CRLF |
| `Client/Private/KoukuSaydonActionWorkbench.cpp` | `Same_ColliderGroupFrame`, `Set_FixedWorldEffectPlacement`, `Copy_PresentationPlacement`, `Same_PresentationPlacement`, `Valid_PresentationPlacement`, `Render_PresentationBoxDetails` | UTF-8 no BOM, CRLF |
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | DEFAULTS, `_validate_presentation_occurrences`, `_project_presentation_occurrence`, scene-profile 투영, `_showtime_visual_template` | ASCII, CRLF |
| `Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py` | 새 test 1개 | UTF-8, CRLF/LF 혼재 보존 |

새 C++ 파일은 없고 `.vcxproj`/`.filters`도 변경하지 않는다.
금지 파일 목록(MainApp*, WorldObjectTool*, MapTool*, MapAreaInventory*, Client.vcxproj/.filters)은 수정하지 않는다. `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`에도 쓰지 않는다.
