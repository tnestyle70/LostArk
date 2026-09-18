# 쿠크 불뿜기 Effect 본 회전 anchor 결과

## G00. 요청과 원인

요청은 `3관문_세이튼_공통 불뿜기`(`kakulsaydon.effect.315861c9051370ef0dbe`, asset
`effect.kouku.gate3.firebreath.shared`)를 boss pivot이 아니라 세이튼의 입/머리 본에 붙여서
머리와 함께 회전시키는 것이다. 조커찾기 뿅망치 Collider가 `BOSS -> WEAPON -> Bone`으로
본에 붙는 것과 같은 방식을 Effect에도 쓰자는 것이다.

실측한 원인은 두 가지였고, 흔히 생각하는 "bone 칸이 없다"가 아니었다.

1. **bone 자체는 이미 가능했다.** presentation occurrence는 `bone`/`boneTarget`을 이미 갖고
   있고, Box Detail의 `Bone target` / `Bone` 콤보가 EFFECT 박스에도 이미 나온다. 실제로
   `KAKULSAYDON_G1_PATTERN_11.presentation.8/9/10`(`boss.kouku.medusa.laser`)이 이미
   `bone="bip001-head"`로 저장돼 있다.
2. **회전이 막혀 있었다.** `Make_Pivot`이 `Resolve_TargetPivot`에 `PIVOT_ROTATION::TARGET_YAW`를
   **상수로** 넘겼다. 이 모드는 본에서 **위치만** 가져오고 회전 basis는 `View.YawBasis`
   (= boss root)로 덮어쓴다. 기존 UI 설명문도 그렇게 적혀 있었다.
   `"Named bones follow their position; rotation uses boss facing and this box's rotation."`

`CEffectV2Object::PIVOT_ROTATION`에는 `BONE`이 이미 있고 V2 binding은 쓰고 있었다.
쿠크 composition occurrence만 그 선택지를 저장·전달할 수 없었다.

## G01. 계약: occurrence `boneRotation`

| 항목 | 값 |
|---|---|
| JSON key | `boneRotation` (presentation occurrence optional) |
| 값 | `"TARGET_YAW"`(기본, 기존 동작), `"BONE"` |
| C++ | `std::string strBoneRotation = "TARGET_YAW";` |
| 저장 | 기본값이면 쓰지 않는다. 기존 문서와 Product bytes가 그대로다. |
| 허용 | `"BONE"`은 resource kind `EFFECT` + `anchorKind == "BOSS"` + `bone` 비어 있지 않음 |
| 거부 | COLLIDER/LIGHT/SOUND/CAMERA, WORLD/MAP/PLAYER anchor, bone 없음, bundle common row |

`"BONE"`을 EFFECT로 제한한 이유는 Collider의 서버용 Bone 궤적이 publish 때 구워지기 때문이다.
Collider의 회전 basis를 바꾸면 Server 판정과 Client 표시가 어긋난다. 이번 범위에 넣지 않았다.

`boneTarget`은 BODY/WEAPON 둘 다 허용한다. `Make_Pivot` 한 곳만 거치므로 preview와 제품
재생, V1/V2, anchor history(입자 birth 소급)가 같은 basis를 쓴다.

## G02. 변경 파일

| 파일 | 위치 | 인코딩 |
|---|---|---|
| `Client/Public/KoukuSaydonCompositionDocument.h` | `KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE::strBoneRotation` | ASCII, CRLF |
| `Client/Private/KoukuSaydonCompositionDocument.cpp` | `Has_Properties` 목록, `Read_PresentationOccurrence`, pattern 검증, bundle common 검증, pattern writer, bundle writer | ASCII, CRLF(+기존 bare LF 2 보존) |
| `Client/Private/KoukuSaydonPresentationPlayer.cpp` | `Validate_EffectAnchor`, `Read_Occurrence`(Product patternbindings reader), `Make_Pivot`, `Preview_PresentationGeometry` | ASCII, CRLF |
| `Client/Private/KoukuSaydonActionWorkbench.cpp` | `Same_ColliderGroupFrame`, `Set_FixedWorldEffectPlacement`, `Copy_PresentationPlacement`, `Same_PresentationPlacement`, `Valid_PresentationPlacement`, `Render_...Anchor`(불변식 1줄 + 체크박스 + 설명문 + bone 해제 3곳 + Copy Collider anchor 정규화) | UTF-8 no BOM, CRLF, 기존 non-ASCII 201 byte 보존 |
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | `PRESENTATION_OCCURRENCE_DEFAULTS`, `_validate_presentation_occurrences`, 기본값 제외 집합 2곳 | ASCII, CRLF |
| `Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py` | 새 test 1개 | UTF-8, CRLF/LF 혼재 보존 |

새 C++ 파일이 없고 `.vcxproj`/`.filters`는 바꾸지 않았다.
`MainApp.cpp`, `WorldObjectTool*`, `MapTool*`, `Client.vcxproj`는 다른 세션 소유라 건드리지 않았다.

핵심 한 줄은 `Make_Pivot`이다.

```cpp
if (!CEffectV2Object::Resolve_TargetPivot(view, box.strBone,
    box.strBoneRotation == "BONE" ? CEffectV2Object::PIVOT_ROTATION::BONE :
    CEffectV2Object::PIVOT_ROTATION::TARGET_YAW, anchor)) return false;
```

`Make_Pivot`의 `sampledBasis` 출력이 `effectAnchorHistories`에 기록되고, 그 history를
`Effect_PivotSampler` → `Make_ResolvedEffectPivot` → `Effect_V1TransformProvider`가 다시 읽는다.
따라서 이 한 곳만 바꾸면 live pivot, 기록된 anchor history, V1 `Update_WorldRoot`/`Seek_WorldRoot`,
V2 group/leaf가 모두 같은 basis를 쓴다.

## G03. 실제 사용할 본 이름

`Client/Bin/Resources/Character/KoukuSaton/*.wmodel`의 skeleton section을 직접 디코드해 확인했다.

| 모델 | 본 수 | 머리 | 입 |
|---|---|---|---|
| `MN_RPCT_05` | 168 | `bip001-head` (idx 10, parent `bip001-neck`) | `bip001-mouth` (idx 17, parent `bip001-head`) |
| `MN_RPCT_06` | 84 | `bip001-head` (idx 10) | `bip001-mouth` (idx 17) |

`bip001-mouth`는 `Is_StableId`(ASCII `[A-Za-z0-9_.-]`)를 통과한다.

대상 박스는 두 패턴에 4개다.

| 패턴 | actorProfileId | occurrence | start/duration | 현재 offset / rotation |
|---|---|---|---|---|
| `KAKULSAYDON_G1_PATTERN_27` (GATE2) | `MN_RPCT_06` | `.presentation.2/.3/.4` | 2366·2347·2348 / 5500 | [2.6, 1.9, -1.4] / [0, 125.15, 0] |
| `KAKULSAYDON_G1_PATTERN_81` (GATE1) | `MN_RPCT_05` | `.presentation.1` | 3002 / 4089 | [1.25, 2.15, 0.25] / [0, 90, 0] |

원본 근거도 확인했다. `C:/LostArkExtract/.../ActionNameSources/MN_RPCT_06.action-effects.json`의
`action-4221809`(`대형 세이튼_불뿜기_B`) notify payload에서 cast 파티클
`Par_X_RPCT_FireCast_01_01_LOC_INT`는 소켓 **`FX_Mouth_01`**, breath 파티클
`Par_X_RPCT_FireBreath_01_02_LOC_INT`는 **`FX_Prj_01`**에 붙는다. 원작도 입 기준이다.

## G04. 실행한 검증

| 검사 | 명령 / 방법 | 결과 |
|---|---|---|
| 인코딩 보존 | 편집 전후 BOM/CRLF/bare LF/non-ASCII byte 수 비교 | 5개 파일 모두 동일 |
| whitespace | `git diff --check -- <변경 5파일>` | 경고 없음(기존 LF/CRLF 정규화 안내 2건만) |
| 격리 컴파일 | `out/KoukuBoneRotation20260918/compile.cmd` (vcvars 18/Insiders, 10.0.26100.0, `-vcvars_ver=14.44`) | exit 0, `error C` 0건, OBJ 4개 생성 |
| 컴파일 대상 | `KoukuSaydonCompositionDocument.cpp`, `KoukuSaydonPresentationPlayer.cpp`, `KoukuSaydonPresentationPlayer_LogicPreview.cpp`, `KoukuSaydonActionWorkbench.cpp` | 전부 성공 |
| 컴파일 경고 | 926건 전부 C4828이며 출처는 `EngineSDK/Inc/Level.h`, `Collider.h` | 이번 변경 파일 유래 0건 |
| projector 계약 | `python -B -m unittest Tools.KoukuSaydonPipeline.test_project_kouku_saydon_composition -k bone_rotation` | OK |
| 회귀 격리 | 이번 hunk만 되돌린 shadow namespace portion과 현재 파일로 같은 module 전체 2회 실행 | 229 test 기준 broken 95 / 95, **이번 변경 귀책 0건** |

`out/KoukuBoneRotation20260918/`에 rsp, cmd, log, exit, OBJ를 격리해 남겼다.

같은 module의 나머지 95건은 이번 변경과 무관하다. 두 갈래다.

- 다른 세션이 `project_kouku_saydon_composition.py`에 커밋하지 않은 `BOSS_TRACK_TARGET`
  `followSpeedScale` 허용과 Summon patternSpawns 확장을 넣었는데 대응 test를 아직 안 고쳤다.
  예: `test_rotate_only_duration_rejects_template_and_outcome_values`.
- 여러 test가 사용자의 Workbench가 계속 저장 중인
  `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` 실물을 읽는다. 실행 간에 revision이
  올라가 실패 집합이 흔들린다(관측: 1593 → 1612).

따라서 **failure 개수 비교는 이 저장소에서 의미가 없다.** hunk 격리 후 이름 집합을
비교해야 한다. 위 "회귀 격리" 행이 그 방법이다.

## G05. 실행하지 않은 것

- Product 빌드(`Invoke-BuildAndRegression.ps1`). 실행 중 `Client.exe`가 출력물을 점유한다.
- publish(`Publish All Patterns`).
- Client/Server 실행과 화면 판정.
- 데이터 반영. `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`에 쓰지 않았다.
  네 박스는 지금도 `bone: ""`, `boneRotation` 없음이다.

데이터에 쓰지 않은 이유는 그 파일을 사용자의 실행 중 Workbench가 소유하고 있기 때문이다.
작업 중 revision이 1593에서 1612까지 올라갔다. `AGENTS.md`의 편집 중 데이터 반영 절차대로,
최종 저장본 기준 반영 승인 전에는 덮어쓰지 않는다. Box Detail에서 직접 고르는 편이
어차피 필요한 offset·rotation 재조정과 한 번에 끝난다.

## G06. 사용자 반영 순서

1. Workbench의 미저장 draft를 Save한다.
2. Client를 종료한다(EXE 링크 잠금 때문이며 데이터 반영 조건은 아니다).
3. `powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`
4. Client 실행 → F1 → Action Workbench → Boss KoukuSaydon.
5. 해당 패턴의 불뿜기 박스를 고르고 Box Detail에서 순서대로 조작한다.
   1. `Anchor` = `Boss`
   2. `Bone target` = `BODY`
   3. `Bone` = `bip001-mouth` (해당 패턴 모델을 preview 중이어야 목록이 열린다)
   4. 새 체크박스 `Rotate with this bone`을 켠다
   5. `Position`/`Rotation`을 다시 맞춘다
   6. Apply → Save
6. `Publish All Patterns` 후 Complete Play 또는 Play Isolated로 확인한다.

## G07. 반영 시 반드시 알아야 할 경계

- **기존 offset/rotation은 그대로 쓸 수 없다.** [2.6, 1.9, -1.4]와 yaw 125.15는 boss root 기준
  값이다. anchor가 입 본으로 바뀌면 기준 frame과 원점이 달라진다. 0 근처에서 다시 잡는 편이 빠르다.
- **입 본이 실제로 움직이는지는 모델마다 다르다.** `MN_RPCT_06`의 `bip001-mouth`는 해당
  클립에서 실제로 articulate한다. `MN_RPCT_05` 쪽은 별도로 확인해야 한다. 머리 회전만 필요하면
  `bip001-head`가 더 안정적이다.
- **이미 방출된 입자는 소급 회전하지 않는다.** asset의 emitter 25개 중 16개가 world space라
  태어날 때 root를 고정한다. 머리를 돌리면 **새로 나오는** 불길이 따라 돌고 이미 날아간 것은
  제자리에 남는다. 원작과 같은 거동이다.
- **본 이름 오타는 publish에서 안 걸린다.** projector와 Product parser는 EFFECT row의 BODY 본이
  대상 WModel에 실재하는지 검사하지 않는다(Server Collider bone track만 검사한다).
  런타임에서 `Make_Pivot`이 실패해 해당 박스만 `Presentation bone/pivot is unavailable`로 격리된다.
- **`BONE`은 EFFECT 전용이다.** Collider에 켜려면 Server bone 궤적 굽기까지 같이 맞춰야 한다.
- 이 변경은 Shared protocol, Server, bootstrap을 바꾸지 않는다.

## G08. 같이 갱신할 문서

- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`: `boneTarget` 문단 아래 `boneRotation` 한 문단
  (허용 조합, TARGET_YAW/BONE 의미, EFFECT 한정 이유)
- `.md/GB/gotchas.md`: "쿠크 presentation 박스의 bone anchor는 기본이 위치 전용이다. 회전까지
  따라가려면 `boneRotation=BONE`을 켜고 offset/rotation을 본 frame에서 다시 잡는다."

## G09. 2026-09-18 원본 socket 근거와 부착 수치의 추가 정정

공통 불뿜기를 P27 입 본에 배치할 때 기준으로 삼는 실제 breath notify는
`action-4221809/stage-001/notify-010` 및 stage-003의 같은 notify이며 `FX_Prj_01`을 선택한다.
RPCT06에 `FX_Mouth_01`은 실제로 존재하지만 G03의 cast용 별도 socket이므로 이를 breath notify의
선택값으로 설명하지 않는다. 두 socket 모두 원본에서 `bip001-head`에 붙는다. P81 breath는
`FX_Prj_02`를 선택한다. 원본 PSK와 실제 설치 WModel을 대조하여 mouth-local 위치·회전을 계산한
최신 값과 적용·미검증 경계는 [왼손 트레일과 입 본 부착 실측 RESULT](2026-09-18_KOUKU_TRAIL_MOUTH_REPAIR_RESULT.md)를 따른다.
이전 G05의 미반영 상태와 G06의 수동 입력 절차는 당시 기록으로 보존하며, 현재 저장 수명과
최종 반영 상태의 정본으로 사용하지 않는다. 수치 부착 검증은 실제 Preview/Server Play 화면
확인 완료를 뜻하지 않는다.
