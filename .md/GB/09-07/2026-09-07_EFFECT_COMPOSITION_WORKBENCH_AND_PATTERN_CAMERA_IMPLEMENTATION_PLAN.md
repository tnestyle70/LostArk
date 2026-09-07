# Effect Composition Workbench·패턴 Camera 구현 계획

작성일: 2026-09-07

문서 종류: 구현 계획서. 사용자 구현 승인 후 Effect Composition Workbench·Camera 구현에 사용한 설계다. 실제 완료·검증과 남은 경계는 같은 날짜의 대응 RESULT가 정본이다.
기준은 `codex/kouku-gate-pattern-bundles`의 현재 작업 사본이다. 선행 미커밋 변경, Composition v3/revision 87,
Gate·폴더·묶음·자식 관계, protocol 66을 보존한다. 이전 문서의 revision 86/단일 보스 가정을 재사용하지 않는다.

## G00. 목표와 현재 구현 경계

새 F1 도구의 이름은 **Effect Composition Workbench**다. 기존 Effect Resources의 목록·폴더·썸네일·검색과
Type별 resource slot 선택을 유지하고, Effect 생성·편집·저장을 모델 애니메이션 선택과 분리한다.
모델과 저장 Pattern은 같은 시계에서 비교하는 선택적 참고 대상이다. Effect는 그 자체로 생성하고 저장·재생할 수 있다.
기존 codec·renderer·runtime은 계속 사용하며, 새 창을 위해 세 번째 Effect 실행 경로를 만들지 않는다.

| 현재 코드에서 확인한 것 | 이번에 바꿀 부분 |
|---|---|
| V1/V2 Effect 문서는 애니메이션 없이 생성할 수 있다. V2의 effect 모델 자체 animationClip도 optional이다. | 모든 Create/Save/독립 Play에서 대상 캐릭터·Pattern·Clip을 요구하지 않는다. |
| V2 `Save_Document()`는 활성 preview 객체의 Params를 읽어 저장한다. preview가 없으면 Save를 거절한다. | 편집 Document가 정본, preview는 Document를 소비하는 결과가 되도록 소유권을 바꾼다. |
| V2 leaf/group에 stable ID, child 시작·지속 시간·offset과 stop이 있다. | 이를 독립 창과 element Timeline으로 편집한다. 새 composition 파일 형식을 중복 생성하지 않는다. |
| 통합 EffectResourceCatalog는 owner별 Load/Save/Preview capability를 가진다. 현재 구현의 필터는 Valtan 중심이다. | 선택 domain에 맞게 기존 목록을 확장하고 동일 브라우저를 새 도구에서 소비한다. |
| Particle + mesh slot, seed, 방향/속도 범위, spin, lifetime, world-space 입자는 이미 있다. | 생성 시각별 이동 anchor를 샘플링하고 재생·seek가 같은 결과를 내도록 보완한다. |
| Kouku Camera Create는 기존 shot을 Composition resource로 등록한다. | 이름으로 새 shot을 만들고 현재 카메라 pose를 캡처·저장하는 authoring을 추가한다. |
| Kouku Composition Camera는 pose를 즉시 override하고 즉시 해제한다. | 진입 → 유지 → 현재 Player Follow Camera 복귀를 동일 전환 경로로 연결한다. |

현재 파일 근거:

- [Effect_Tool_V2.cpp:1215](C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool_V2.cpp:1215): preview 의존 Save.
- [EffectV2_Document.h:151](C:/Users/user/Desktop/LostArk/Client/Public/EffectV2_Document.h:151): group child 저장 계약.
- [EffectResourceCatalog.h:20](C:/Users/user/Desktop/LostArk/Client/Public/EffectResourceCatalog.h:20): 기존 owner/capability 경계.
- [EffectV2_Object.cpp:1122](C:/Users/user/Desktop/LostArk/Client/Private/EffectV2_Object.cpp:1122): 입자 생성 시 world 변환.
- [Level_KakulSaydonArena.cpp:2117](C:/Users/user/Desktop/LostArk/Client/Private/Level_KakulSaydonArena.cpp:2117): 현재 Camera 직접 override.

위 링크의 구현 기준은 현재 작업 사본이며, 예전 Effect G06 문서는 설명 형식 참고로만 사용한다.

## G01. Effect 정의·배치·참고 애니메이션의 저장 책임

| 단위 | 소유할 값 | 소유하지 않는 값 |
|---|---|---|
| Effect asset | 이름·stable ID·element 구성·resource slots·재질·렌더 설정·자체 수명 | 특정 보스 Pattern의 Stage/클립 목록 |
| Element/Emitter | child/element ID·resource 참조·시작 시간·방출 구간·개별 입자 수명·local transform | 보스의 Server 위치·피해 판정 |
| Pattern occurrence | Effect ID·시작 시각·활성 구간·대상 anchor·follow/stop 정책·해당 배치의 offset | Effect 원본의 texture/material 복사본 |
| Model Animation 참고 | 선택 target/pattern/bundle ID와 소스 revision·읽기 전용 clip 행 | 별도 저장한 애니메이션 복제본 |

기존 정본을 사용한다.

- V2 단일 효과: `Data/Effects/V2/Authored/<id>.effectv2.json`.
- V2 조합: `Data/Effects/V2/Groups/<id>.effectv2group.json`.
- 기존 V1 복합 효과: 현재 `Data/Effects/Authored`의 원본 문서와 기존 codec/runtime.
- 쿠크 패턴의 호출·배치: 기존 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`.
- 발탄의 호출·배치: 기존 split presentation과 해당 writer/publisher.

신규 Create의 기본은 현재 쿠크가 소비하는 V2 조합이다. 이름과 첫 element Type을 입력하면 메모리에
조합과 첫 element를 만들고 그 element를 선택한다. resource가 아직 없으면 편집은 가능하고 preview만
필요한 slot을 안내한다. 유효한 저장의 필수 resource 검사는 유지한다.

한글 표시 이름은 ID와 분리한다. V2 leaf/group의 현재 구조에는 별도 이름이 충분하지 않으므로 optional
`displayName`을 기존 codec·catalog·serializer에 함께 추가한다. stable ID는 생성 시 발급하고 이름 변경으로 바뀌지 않는다.

`Save Effect`는 편집 중인 Effect body만 저장한다. `Append to Pattern`은 저장된 Effect ID를 현재 Pattern에
참조로 배치하고 Pattern을 Dirty로 만든다. Pattern Save는 기존 Action Workbench writer가 처리한다.
참고 Pattern 선택만으로 binding을 만들거나 Save Effect가 애니메이션 목록을 덮어쓰지 않는다.

이미 여러 Pattern에서 쓰는 Effect를 직접 수정하면 그 참조 전체에 영향을 준다. 첫 구현은 stable ID를 Detail에 표시하고 새 Effect/Element 생성으로 별도 ID를 발급한다. 사용처 역조회·Duplicate·Make Unique는 이번 구현에 포함하지 않는다.

새 창의 편집 상태는 `CEffectEditingSession`이 소유한다. 기존 창의 resource/detail UI를 함수로 재사용하되 기존 슬롯 선택·preview 상태는 보존한다. 새 창은 읽었던 디스크 기준본과 비교해 다른 창에서 바뀐 파일의 Save를 거부한다. 기존 V1/V2 창의 session을 전부 이전하지는 않는다. Preview 종료·대상 교체·Level 변경이 Document를 지우지 않는다.
여러 V2 파일을 저장하는 경우 현재의 파일별 atomic write만으로 전체 성공을 표시하지 않는다. 변경한 leaf와
group의 기준본 확인, stage, 검증, 순서 있는 교체와 실패 rollback을 하나의 Save로 처리한다. 실패 시 기존
파일과 dirty 상태를 보존하며, 저장 후 catalog 갱신/preview 재생성 실패는 저장 실패와 구분한다.

기존 V1 효과는 원본 document/element를 기존 renderer로 편집·재생한다. V1의 material recipe를 V2 일반
슬롯으로 자동 변환하지 않는다. V1을 V2 group child로 넣는 혼합 실행과 nested group은 현재 지원되지 않는다.
새 Workbench는 V2 leaf/flat group을 편집한다. V1 복합 문서는 기존 Effect Tool에서 계속 편집하며, nested group과 V1 혼합 편집은 새 창에서 지원하지 않는다.

**G01 종료:** 모델 없는 Create → 필수 slot 연결 → Save → 재로드 → 독립 Play가 닫히고, preview 실패와
동시 저장 실패가 정상 원본을 잃게 하지 않는다. 기존 Effect의 ID·body와 다른 Pattern 연결을 보존한다.

## G02. 창 배치와 선택 동작

```text
┌ Effect Resources ───────────┬ Scene / Preview ─────────┬ Effect Detail ─────────┐
│ Create Effect              │ 현재 선택 Effect         │ 선택 element의 튜닝    │
│ Type · Name · Create       │ 단독 / 모델과 함께 보기 │ 필요한 항목만 표시     │
│                            │                          ├ Model Animation ───────┤
│ 기존 저장 리소스 트리       │                          │ None / Character / Boss│
│ 기존 Type별 resource slot  │                          │ Gate · Pattern · Bundle│
│ 기존 물리 폴더·썸네일·검색  │                          │ 실제 ordered clip 목록 │
├ Effect Sequencer ──────────┴──────────────────────────┴───────────────────────┤
│ Play · Pause · Restart · Loop · Seek             하나의 master cursor          │
│ Model / 쿠크           [clip A][clip B────────][clip C]                    │
│ Model / 거대세이튼          [clip D───────────────][clip E]                 │
│ Effect / 선택 조합      [활성 구간────────────────────────────]             │
│   Element / 공         [방출 구간──────────] ··· 잔여 개체 수명              │
│   Element / 섬광          [짧은 burst]                                     │
│   Element / 파편           [방출─────] ······· 잔여 개체 수명               │
└──────────────────────────────────────────────────────────────────────────┘
```

Action Workbench/Object Tool처럼 F1에서 여는 독립 창을 제공한다. ImGui의 창 크기·위치 저장을 사용하며 별도 Reset Layout 명령은 추가하지 않는다.
리소스를 다른 목록 형태로 재설계하지 않는다. 기존 domain·폴더·썸네일·선택 slot 바인딩을 유지한다.
오른쪽에는 Effect Detail과 Model Animation 탭을 두고 같은 영역에서 전환한다.

부모 Effect 선택 시 전체 수명과 anchor 설정을, 아래 Sequencer에는 element 목록을 보인다. element 선택 시 해당 Type의 Detail과 slot이
열린다. 원본 model/texture 클릭은 선택 slot의 후보만 바꾸고 다른 element를 암묵적으로 생성하지 않는다.
`Add Element`, `Append Saved Effect`, `Bind Resource`의 책임을 구분한다.

모델 참고의 기본은 `None`이다. 보스 선택은 현재 저장한 Gate → 폴더 → 묶음/자식 구조와 target을 그대로
읽는다. 쿠크·거대세이튼·앵콜 세이튼을 이름 문자열로 추측하지 않고 기존 actor/placement/model 관계를 사용한다.
묶음 선택이면 member offset을 포함한 각 actor 행을, 자식 선택이면 그 자식의 전체 ordered clip을 표시한다.
빈 DRAFT는 빈 상태와 이유를 보이며 임의 clip을 추가하지 않는다. 묶음 offset은 저작 ms와 현재 30Hz tick 올림 실행 시각을 함께 표시한다.

Model Animation 행은 먼저 읽기 전용 참고로 연결한다. 실제 clip 추가·삭제·순서/Stage 변경은 원래
Action Workbench의 같은 Pattern으로 이동해 수정한다. 이 창에서 복제된 clip sequence를 별도 저장하지 않는다.
Effect mesh 자체의 animation clip은 캐릭터 참고 애니메이션과 구분해 선택 element Detail에 둔다.

## G03. 공통 시계와 element lifetime

Timeline/group/occurrence 시각은 ms이며 기존 leaf lifetime, particle lifetime, trail pointLifetime은 초 단위를 유지한다. UI 경계에서만 변환한다. 프레임 표시는 안내다.
공통 시계는 Pattern/member offset을 현재 실행과 동일하게 30Hz tick 올림한 시각으로 변환한 뒤 Effect occurrence/child offset을 합산한다.
같은 cursor에서 모델 pose와 Effect 상태를 샘플하며 여러 창의 Render/Update가 각자 시간을 증가시키지 않는다.

Particle/Trail에는 방출과 잔여 수명을 구분한다. Mesh/Texture/Decal/ScreenPost에는 기존 element lifetime/종료 의미를 유지한다.

- 방출 시작: 처음 생성할 시각.
- 방출 구간: 새 입자를 계속 만들 수 있는 기간.
- 개별 lifetime: 이미 생성한 공·파편 각각이 살아 있는 기간.

Particle/Trail Timeline의 진한 막대는 방출 구간, 연한 꼬리는 입자/point가 정리될 때까지의 구간이다. 방출 종료와
즉시 삭제를 혼동하지 않도록 기존 Deactivate/Kill을 `방출 중지 후 수명 유지`/`즉시 정리`로 보여준다.
group의 전체 제한이 꼬리를 자르면 잘린 종료 위치를 표시한다. Pause·Seek·재시작·Loop에서 같은
seed와 같은 root 이동 표본을 쓰며 결과가 반복 가능해야 한다.

`CompositionWorkbenchSession`의 pane 흐름과 `CompositionTimeline`의 DrawRuler/DrawBox/HitBoxGesture를 재사용한다. element ID와 occurrence ID가 편집
대상을 식별하며 vector index를 저장 ID로 사용하지 않는다. Solo/Mute는 기본적으로 preview 선택이고,
저장 enabled 변경과 구분해 원치 않는 제품 변경을 막는다.

## G04. 쿠크 이동 경로를 따라 공 생성

공의 뼈 애니메이션을 찾는 문제와 방출 방식은 별개다. 현재 모델은 Particle의 mesh slot에 연결할 수 있다.

| 설정 | 권장 의미 |
|---|---|
| Type | Particle + Mesh |
| Mesh | 기존 `Effect/KoukuSaydon/Meshes/wp_mn_rhcn_00/mesh/fm_d_rhcn_00.wmodel` |
| Base texture | 기존 `Effect/KoukuSaydon/Textures/MN_RHCN_00/tex/mn_rhcn_00_d.dds` |
| 생성 위치 | 각 birth time의 쿠크 root 위치 + local offset |
| 생성 방향 | 이동 방향 + 저작 yaw/pitch 범위의 재현 가능한 랜덤 값 |
| 생성 후 기준계 | WORLD. 생성된 공은 이후 쿠크 이동에 끌려가지 않음 |
| 시간차 | spawn rate/interval 또는 저작 burst 시각 |
| 위아래 이동 | 초기 수직 속도·가속도와 개별 lifetime |
| Random seed | Effect seed와 stable child/occurrence ID·반복 회차로 결정 |

현재 V2는 world-space로 생성된 입자 위치·속도를 고정하는 부분을 이미 지원한다. 보완할 것은
`t_i`마다 root transform을 얻는 것이다. 한 프레임 동안 세 개가 태어났으면 현재 위치 하나에 몰아 만들지
않고 세 birth time의 위치에서 각각 생성한다. 낮은 FPS와 seek도 같은 생성 스케줄을 소비해야 한다.

첫 구현의 birth transform은 기록된 actor root 위치·회전이다. 이동 궤적의 접선 방향을 별도로 계산하는 방향 mode는 추가하지 않는다. 명시적 discontinuity를 보간하지 않으며 현재 caller는 50m 초과 위치 jump를 불연속으로 감지한다. 작은 순간이동을 모두 감지하는 계약은 아니다.

기존 V1의 transform-history provider를 참고해 V2 runtime에 birth-time sampler를 연결한다. root history는
runtime/session 입력이며 Effect 원본에 플레이어·보스의 실제 world 좌표를 대량 저장하지 않는다. 첫 구현은 root anchor이며 bone은 같은 birth time의 clip/bone sampler가 있을 때만 허용한다.

현재 Kouku Bundle Preview는 배치 위치에 actor를 만들고 root motion을 suppress한 채 clip/time을 샘플한다.
전체 animation clip을 표시하는 것만으로 Server navigation 이동 경로가 재현되지 않는다. 첫 적용은 실제
Server 패턴 재생의 timestamp root 표본을 기록하고 그 구간에서 Effect를 되감아 튜닝하도록 한다.
기록되지 않은 과거/미래는 명시적으로 사용할 수 없다고 표시한다. 별도 저작 이동 경로를 추가할 경우에도
Effect에 가짜 clip 이동을 넣지 않고 같은 transform sampler 입력으로 연결한다.

검증에는 직선·곡선 이동, 30/60/낮은 FPS, 한 프레임의 여러 birth, seek/restart와 seed 동일성을 포함한다.
정지 후 낙하·소멸, anchor 삭제, teleport 경계도 확인한다. 모델이 없는 단독 테스트에서는 고정 root로 재생한다.

현재 여러 World Object emission을 각각 후속 Effect anchor로 지정하는 계약은 없다. 하나의 emitter가 공을
여러 개 생성하는 이번 요구는 위 경로로 처리한다. 각 공이 다시 효과를 방출해야 할 때는 별도의 안정적인
개별 instance 식별과 실제 consumer를 추가해야 한다.

## G05. Effect Detail과 고난도 복원

기본 Detail에는 선택 element를 실제로 바꾸는 항목을 모은다.

| 묶음 | 편집 항목 |
|---|---|
| 생성 | Type, Count/Rate/Burst, seed, 생성 shape·범위 |
| 위치·움직임 | root/bone, 생성 후 follow 여부, local offset, 방향·속도·가속도·drag, 회전·spin |
| 시간 | 시작, 방출 구간, 개별 lifetime, stop 정책 |
| 형태·색 | 크기/색/alpha over life, UV/SubUV, dissolve, emissive |
| Render | 해당 renderer와 호환되는 material/render profile, blend/depth/cull, distortion 입력 |
| Resource Slots | Type에 필요한 mesh·base·noise·mask·emissive·dissolve와 기존 material 슬롯 |

Render Pass는 사용자가 조절하되 raw pass index를 임의로 저장하지 않는다. 기존의 안정적인 render/material
profile을 선택하고 실제 renderer가 지원하는 조합만 보여준다. 선택한 pass의 label과 적용 상태를 표시한다.
발광/투명/깊이/왜곡을 구분하고, 해당 shader가 소비하지 않는 값을 정상 적용처럼 보여주지 않는다.

원본 recipe, material program, 원시 모듈과 복원 근거는 Advanced/Source 아래에 유지한다. 기본 창에서
숨기는 것과 저장 데이터에서 삭제하는 것은 다르다. 기존 V1의 고급 곡선·원본 재질은 그대로 round-trip한다.

창술사 유리 파편 복원은 UI 개편과 별도의 렌더링 작업으로 진행한다. 정확한 skill/occurrence를 먼저 특정하고
그 emitter·geometry·texture·material·실제 pass 연결을 확인한 뒤 필요한 공용 renderer 기능만 보완한다.
파편 texture가 있다는 사실만으로 원작 발생 위치·개수·재질을 확정하지 않는다. 현재 Effect 코드에서 PhysX
충돌·반발 소비는 확인되지 않았으므로 단순 포물선과 실제 충돌 simulation을 구분한다.
공중 방출·낙하에는 현재 velocity/acceleration을 사용하고, 충돌/튕김이 필요한 대표 파편이 확인되면
기존 범용 물리 경로에 이어 구현한다. UI만 개편한 상태를 유리 파편 복원 완료로 기록하지 않는다.

## G06. Camera Create·Save와 패턴 왕복 전환

사용자가 생각한 동작은 세 단계가 맞다. 다만 세 개의 고정 world position을 저장하지 않는다.

```text
현재 보이는 Player Follow Camera
       ↓ 진입 시간
저장한 Zoom-out 목표 pose에서 유지
       ↓ 패턴/카메라 유지 구간 종료 → 복귀 시간
그때의 Player Follow Camera
```

출발 pose는 재생 순간에 한 번 취득한다. 복귀 목표는 이동 중인 플레이어와 현재 FollowCameraProfile로
매 프레임 계산한다. 과거 시작 위치를 저장해 복귀에 쓰면 이동한 플레이어를 놓친다.
카메라 pose에는 position 외에 lookAt/FOV도 필요하다. `Set Camera Pos`는 현재 view의 이 값들을 함께
캡처하므로 사용자가 숫자 세트를 일일이 입력할 필요는 없다.

현재 공용 Camera Tool의 `New Cut/Capture Pos/Save`는 Valtan 정본에 연결돼 있다. Kouku의 Camera 탭은
Area에 이미 저장된 shot을 등록할 뿐이다. pose capture와 기존 Area shot 전환기를 재사용해 다음 흐름을 만든다.

```text
Action Workbench → Camera → Create Camera
이름: 2관문_세이튼등장 → Create
카메라를 원하는 시점으로 배치 → Set Camera Pos
진입 시간 / 기본 유지 시간 / 복귀 시간 → Save Camera
패턴 또는 묶음 선택 → 저장 Camera 선택 → Append → Play
```

| 저장 위치 | 책임 |
|---|---|
| Area `.camerashots.json` | stable shotId, 표시 이름, 목표 eye/lookAt/FOV, blendInMs/defaultHoldMs/blendOutMs, 보간 방식, 활성화 방식 |
| Composition Camera occurrence | shotId 참조, 시작과 유지 종료 시각, 해당 배치의 offset |
| `Data/Camera/KoukuSaydon.camera.json` | 평상시 Player Follow Camera 설정 |
| Level transition state | 이번 실행의 시작 pose·현재 pose·run/occurrence owner·복귀 진행도. 저장하지 않음 |

단순 zoom-out의 기본은 고정 WORLD pose다. 필요하면 기존 player-relative eye/lookAt offset을 선택할 수
있게 하지만 BOSS anchor가 이미 작동한다고 표시하지 않는다. 현재 Kouku Camera consumer는 일반 occurrence의
anchor/rotation을 전부 소비하지 않으므로 지원하는 모드만 UI와 저장 계약에 열어 둔다.

신규 Pattern 전용 shot에는 명시 활성화 모드를 추가해 기존 Area box/World Sequence 자동 선택에서 제외한다.
새 shot 저장 때문에 맵의 다른 위치에서 카메라가 자동으로 켜지면 안 된다. 기존 shot은 이전 활성화 방식을 유지한다.

기본 보간은 요청대로 LINEAR다. 기존 bounded sampler의 smoothstep은 선택 옵션으로 재사용하되 선형이라고
부르지 않는다. 위치·lookAt·FOV를 같은 진행도로 보간한다. 진입 도중 중단되면 현재 표시 pose에서 복귀한다.

`durationMs = blendInMs + holdMs`, `returnStart = occurrence.startMs + durationMs`, `finalEnd = returnStart + blendOutMs`다. Append의 hold는 shot.defaultHoldMs로 초기화하고 blendInMs가 durationMs를 넘으면 거부한다.

**시간의 의미:** Camera 박스의 끝은 고정 시점 유지 종료이며 복귀가 시작되는 지점으로 정한다. 별도 연한
복귀 꼬리가 `blendOutMs`를 표시한다. 패턴 종료 후 돌아오라는 요구를 위해 정상 Pattern 완료가 이 꼬리를
즉시 삭제하지 않도록 Level의 전환 상태가 짧은 복귀를 끝까지 소유한다. 화면에는 진입·유지·복귀와 총 시간을
각각 표시한다. 자식·묶음 Camera 충돌 검사에도 실제 복귀 점유 구간을 반영한다.

새 패턴이 시작돼 다른 Camera를 인계받으면 현재 표시 pose를 새 진입점으로 사용한다. 명시적 사용자 중지의
복귀, F6 free camera의 즉시 권한 반환, Level 종료/대상 소멸의 정리는 서로 구분한다. 같은 프레임의 여러
Camera writer가 경쟁하지 않도록 기존 owner/priority와 run/occurrence 식별을 사용한다.

Kouku gameplay 입력은 FollowEnabled를 유지한 채 presentation override만 사용하면 계속 받을 수 있다.
Valtan full cinematic의 `Set_FollowEnabled(false)`/target 제거 코드는 그대로 옮기지 않는다.
엔진 `End_PresentationOverride()`는 시작 때의 pose를 복구하므로 정상 복귀 완료 시 현재 Follow pose로
끊김 없이 넘기는 종료 계약도 함께 보완한다. 기본 End의 기존 동작은 유지하고 명시적인 handoff 경로만 추가한다.

Save Camera는 기존 MapTool의 Area writer/validator를 재사용한다. 저장한 authoring shot을 목록과 Preview에 즉시 반영하며 일반 Preview에 Map publish를 요구하지 않는다. 새 shot staging 실패는 이전 문서와 camera를 유지한다. Product는 명시 publish 후 안전한 run 경계 reload 또는 Area 재진입으로 published snapshot을 갱신한다.

**G06 종료:** 이름으로 shot 생성 → pose capture → Save → Camera Append → Pattern Save → 저작 Play에서 진입·유지·복귀. Product는 Map publish와 runtime snapshot 갱신 뒤 실제 Complete Play를 확인한다. 유지·복귀 중 플레이어 이동 입력, 재시작·중단·F6·Level 종료,
마지막 프레임의 과거 pose 복원 방지와 겹친 owner 정리가 함께 확인돼야 한다.

## G07. 파일과 구현 순서

다음은 구현에 사용한 변경 단위다. 실제 추가 파일과 검증 결과는 RESULT에 기록한다.

| 파일 | 변경 책임 |
|---|---|
| `Client/Public/EffectCompositionWorkbench.h`, `Client/Private/EffectCompositionWorkbench.cpp` 신규 | 독립 패널, 선택·명령, Effect/Model 행과 하나의 cursor |
| `Client/Public/EffectEditingSession.h`, `Client/Private/EffectEditingSession.cpp` 신규 | 기존 편집 코드에서 분리한 document owner·dirty·저장 기준본·선택 element; 기존/새 창이 함께 소비 |
| 기존 `Effect_Tool.h/.cpp`, `Effect_Tool_V2.h/.cpp` | Resource/Slot/Detail 편집 명령 재사용, preview 의존 저장 제거, 공통 session 연결 |
| `EffectResourceCatalog.h/.cpp` | 기존 catalog의 domain 범위와 표시 이름·실제 지원 capability |
| `EffectV2_Document.h/.cpp`, `EffectV2_Catalog.cpp` | 이름·저장 transaction·검증, 기존 leaf/group 원본 보존 |
| `EffectV2_Object.h/.cpp`, `EffectV2_Runtime.h/.cpp` | birth-time root sampler, fixed clock·seed·seek와 잔여 lifetime |
| 기존 `Effect_Playback.cpp`, `Effect_DocumentRenderer.cpp` | V1 편집/재생 재사용. 검증된 기능의 호출 연결이 필요한 부분만 변경 |
| `KoukuSaydonActionWorkbench.h/.cpp`, `ValtanActionWorkbench.h/.cpp` | 저장 Pattern/bundle의 읽기 전용 timeline 제공과 typed Append 명령, Camera authoring 진입 |
| `KoukuSaydonPresentationPlayer.h/.cpp`, `Level_KakulSaydonArena.h/.cpp` | 기존 actor/WORLD/Effect preview 시계와 Camera의 정상 종료·복귀 tail |
| `MapTool.h/.cpp` | 기존 Area camera-shot 저장/검증 재사용과 저장본 목록/Preview 갱신 |
| `CameraTool.h/.cpp`, `ValtanCinematicCameraController.h/.cpp` | pose capture·전환 sampler 공용 부분 재사용, Kouku Area 저장 backend 연결 |
| `Engine/Public/Camera.h`, `Engine/Private/Camera.cpp` | 기존 End 복원 동작을 유지하는 명시적 현재 Follow pose handoff |
| `MainApp.h/.cpp`, `Client.vcxproj`, `Client.vcxproj.filters` | F1 등록·수명·Level 전환과 신규 H/CPP의 정식 등록 |
| `Tools/EffectToolV2`, `Tools/MapPipeline`, `Tools/KoukuSaydonPipeline`의 기존 validator/test | 변경한 이름·생성 시각·Camera 활성화/복귀 수명·원본 보존 검사 |

물리 폴더가 정본이며 Workbench·EditingSession·ModelPreview·WorldResource의 새 여덟 H/CPP 파일은 Client의 기존 debug tool 계열 project/filter에 정확한 Include/Compile로
등록한다. 기존 필터를 재배치하지 않는다. 새 runtime 경로·보스별 renderer·임시 catalog 복사본은 만들지 않는다.

실행 순서는 다음으로 정한다.

1. G01/G02의 최소 수직 슬라이스: 모델 없는 생성·slot·Detail·Save/Load·독립 Play와 새 패널.
2. G03의 element Timeline, 같은 cursor와 선택적 전체 Pattern/묶음 Model Animation 행.
3. G04의 공 emitter와 이동 기록 재생을 실제 쿠크 사례에 연결.
4. G06의 Camera Create/Capture/Save/Append와 Player Follow 왕복 전환. Effect renderer 수정과 독립 수행 가능.
5. G05의 정확한 창술사 파편 occurrence를 첫 고난도 복원 대상으로 튜닝. 새 UI의 실사용을 통해 필요한 재질/renderer만 보완.

Pattern 제작은 기존 Action Workbench에서 계속하고 Effect 복원은 새 창에서 진행한다. 둘이 공유하는 것은
Effect ID·Pattern ID·대상/시간 참조다. 각 도구의 저장이 다른 소유자의 데이터를 통째로 덮어쓰지 않는다.

## G08. 검증과 구현 상태

구현 때 필요한 최소 검증은 다음이다.

- 기존 Effect ID·resource path·material recipe 보존과 독립 Create/Save/Reload.
- 선택 slot만 변경, 잘못된 resource/pass·손상 문서·stale Save 실패 시 이전 document와 preview 유지.
- Timeline 시작·종료·방출 중지/잔여 수명, seed·FPS·seek 재현성과 이동 anchor 표본.
- Gate/모델/묶음 교체 시 올바른 ordered clip과 member offset, 빈 DRAFT 표시.
- Camera resource 저장/Map publish/Composition Append, 입력 유지·정상 복귀·중단과 충돌 검사.
- 변경 JSON/XML parse, `git diff --check`, 변경 기능 최소 컴파일 및 최종 Debug Product.
- existing focused Effect/Map/Composition 검사를 확장하고 새 전용 대형 하네스나 admission 체계를 추가하지 않음.

사용자 승인 후 새 Workbench, CPU draft, World Object 모션 가져오기, 모델 참고 시계, birth-time sampler, Camera 왕복 전환과 가짜 세이튼 Spot Light 상속을 구현했다. 검증 결과는 [대응 RESULT](2026-09-07_EFFECT_COMPOSITION_WORKBENCH_AND_PATTERN_CAMERA_RESULT.md)에 기록한다. Client/UI 실행·조작·캡처는 하지 않으며 실제 아레나 재생과 시각 품질 판정은 사용자가 수행한다.

## G09. 구현 승인 후 추가 요구 — 조회 병목과 World Object resource

2026-09-07 사용자 승인으로 전체 구현에 착수한다. Tree/목록은 경로·ID·표시 이름 metadata만 조회하고, 선택 문서는 항목별 parse, Play는 선택 closure의 resource 검증, Save는 변경 문서의 구조와 stale 기준본 검사, Product는 해당 publisher 검증으로 분리한다. Tree가 Valtan 전체 graph/oracle/resource admission의 성공을 기다리지 않는다. 손상 항목은 오류 상태로 남기고 정상 항목을 유지한다. 프레임별 재스캔·자동 실패 재시도·전체 썸네일 생성도 제거한다.

`2관문_세이튼등장_공파편` 같은 UTF-8 표시 이름은 ASCII stable ID와 분리한다. Resource에 World Object 카테고리를 추가해 저장된 `월드오브젝트_공`의 model/base texture/pre-scale을 현재 element의 slot으로 연결한다. 모델/텍스처를 복사하거나 World Object의 저장 모션을 덮어쓰지 않는다. 저장한 `쿠크_세이튼등장_동시` 묶음/자식의 전체 clip을 Model Resource에서 선택하고 쿠크 actor를 character anchor로 사용한다.

## G10. 최초 창 열기의 empty ImGui ID 회귀 수정

사용자가 첫 Open에서 `Cannot have an empty ID at the root of a window` assertion을 보고했다. 빈 EditingSession은 group의 name/ID가 모두 비어 있는데 Timeline이 이를 `Selectable` label로 제출한 것이 직접 원인이다. 빈 draft는 sequencer 작성 안내만 그리고 반환한다. 생성·Load 후 부모 row에는 표시 이름과 독립적인 `###EffectCompositionGroup` ID를 부여한다. Revert로 저장 전 draft를 버린 경우도 같은 빈 상태 경로를 사용한다. source 변경 뒤 Client 최소 compile/link, 기존 관련 focused test와 diff 검사를 확인한다. Client 창 재열기는 사용자가 직접 한다.

## G11. 사용자 화면 기준 재구성 — 구현 전 확정

기존 독립 Workbench의 큰 창은 진입점에서 제거한다. Effect Tool / Effect Detail / Model View의 기존 panel과 renderer를 재사용하고 Data Files는 Effect Resource로 교체한다. 기존 V1/V2 원본과 사용자가 저장한 Object·Light·Composition은 보존한다.

- Effect Resource는 V1/V2 루트, 사용자 지정 Category/Parent/Effect 이름과 stable ID를 표시한다. 트리 메타데이터만 `Data/Effects/EffectResourceTree.json`에 CAS 저장한다. 목록 조회는 개별 파일 metadata만 읽고 Product 전체 validate나 GPU load를 호출하지 않는다. 잘못된 항목은 그 행에 표시한다.
- V1은 기존 Current Effect, Element, Resource slots와 Detail tuning을 그대로 소비한다. V2는 기존 CPU EditingSession과 같은 slot/detail renderer를 두 패널 안에 연결한다. Create/Save는 애니메이션을 요구하지 않는다.
- World Object resource는 저장 parent의 model/base texture와 자식 Motion을 읽는다. V1 resource binding과 V2 motion importer를 각각 원래 owner로 연결하며 저장 원본을 변경하지 않는다.
- Model View는 공유 CharacterPreviewPanel/CModel, Character skillbindings, Kouku Pattern/Bundle, Valtan Product clip sequence를 읽는다. 별도 Effect Sequencer가 model animation과 effect occurrence를 동일 clock으로 sample한다. Preview/Append는 선택한 typed effect를 기존 V1 CEffectObject 또는 V2 runtime에 전달한다.
- 선택 model source ID와 effect occurrence의 authoring 저장은 `Data/Effects/Sequences/<stableId>.effectsequence.json`을 사용한다. Area Composition이나 boss pattern 정본을 덮어쓰지 않는다.
- 기존 EffectCompositionWorkbench의 별도 runtime ownership은 진입점에서 제거한다. MainApp은 Effect Tool 한 owner로 open/hide/level cleanup을 연결한다.
- Patterns by Gate 바로 위에 전체 Composition Save와 Saved/Unsaved 상태를 추가한다. Parent 생성은 메모리 draft이며 Save 성공 뒤에만 재실행 보존을 안내한다.

신규 ResourceTree, Sequencer, V2Pane H/CPP는 Client 프로젝트와 filters에 등록한다. 해당 C++ 최소 컴파일, 변경 XML/JSON parse와 diff check를 수행하며 Client/아레나 화면은 사용자가 직접 확인한다. Nav는 대응 Roulette Walkable Surface PLAN을 따른다.


## G12. Render Pass의 실제 의미와 Effect Detail 품질 설계

### G12-1. 현재 Render Pass가 바꾸는 것

목표는 Effect Detail에서 선택한 값이 화면의 어떤 계산을 바꾸는지 설명하고, 재질 복원과 이번 편집기 개편의 완료 범위를 구분하는 것이다. 이 절은 설계 설명이며 새로운 셰이더나 렌더링 기능의 구현 완료 기록이 아니다. 기준은 [렌더링 감사 RESULT](2026-09-07_RENDERING_MATERIAL_PIPELINE_AUDIT_RESULT.md)와 현재 V1/V2 소스다. 감사 문서의 과거 개수나 조명 수치를 현재 실행 프레임의 측정값으로 옮기지 않는다.

현재 V2의 `Render Pass` 콤보에 나오는 Alpha, Additive, Opaque, Multiply는 주로 **배경과 Effect 색을 합치는 blend 방식**이다. 콤보는 `Params.eBlend`를 바꾸고, `CEffectV2Object::Render()`가 Depth Test와 함께 실제 셰이더 pass를 결정한다. 일반적으로 pass는 셰이더, 출력 대상, blend, depth, culling을 묶은 실행 단계이므로 콤보 이름만 보고 모든 항목을 바꾼다고 이해하면 안 된다. [콤보와 depth 입력](/C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool_V2.cpp:4520), [실제 pass 선택](/C:/Users/user/Desktop/LostArk/Client/Private/EffectV2_Object.cpp:1790).

아래 식에서 S는 Effect의 선형 RGB, D는 이미 그려진 배경, a는 Effect alpha다. HDR 값은 1보다 클 수 있다. 따라서 Additive는 빛의 기여를 더하는 데 적합하지만, 물체 표면을 표현하는 만능 선택은 아니다.

| 현재 선택 | V2 RGB 합성의 의미 | 확인할 점 |
|---|---|---|
| Alpha | `S × a + D × (1 − a)` | 텍스처 alpha와 mask, 겹침 순서가 함께 결과를 결정 |
| Additive | `S × a + D` | 검은 RGB는 색을 더하지 않지만 밝은 입자가 겹치면 쉽게 과도하게 밝아짐 |
| Opaque | blend 없이 S를 기록 | 이 Effect 경로에서 depth write도 사용하지만 표면 조명 모델을 추가하지 않음 |
| Multiply | `D × (S × a + 1 − a)` | V2 Multiply 전용 픽셀 셰이더가 먼저 RGB에 alpha를 곱하는 계약까지 포함 |

이 식은 [V2 blend state](/C:/Users/user/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli:134)와 Multiply 픽셀 출력의 조합이다. 특히 Opaque를 골라도 현재 V2 일반 객체는 `RENDERGROUP::BLEND`에 제출된다. G-buffer에 normal과 표면 재질을 기록하는 일반 모델의 opaque 경로로 자동 이동하지 않는다. [V2 queue 제출](/C:/Users/user/Desktop/LostArk/Client/Private/EffectV2_Object.cpp:1583).

### G12-2. 재질과 합성 상태를 분리하는 Detail 구조

제안하는 Detail은 위에서부터 **Identity/Type → Material → Blend/Depth → Motion/Lifetime → 진단** 순서로 읽히게 한다. Type은 Mesh, Sprite, Particle, Decal, Trail처럼 무엇을 그리는지 정한다. Material은 그 표면이나 입자의 색을 어떻게 계산하는지 정한다. Blend는 계산한 색을 장면에 어떻게 합치는지 정한다. 이 세 항목을 하나의 품질 단계처럼 표시하지 않는다. Type 변경으로 원본 V1 recipe를 V2 공통 파라미터로 변환하는 동작도 넣지 않는다.

Material의 사용자용 후보 이름은 Unlit, Lit, Refraction으로 설명할 수 있다. Unlit은 장면 광원에 의한 표면 명암보다 texture/tint/emissive 계산이 중심이다. Lit은 normal과 실제 광원, 표면 반사 계산이 연결되어야 한다. Refraction은 장면 색을 어디에서 읽고 어떻게 굴절시킬지까지 필요한 기능이다. 내부 계약에서는 Refraction을 Lit과 배타적인 물리 법칙으로 만들지 않고, 사용할 표면 모델과 scene-color 의존성을 구분한다. 사용자에게는 실제 renderer가 지원하는 조합만 preset으로 제시한다.

현재 V2 공통식에는 base/noise/mask/emissive/dissolve와 mesh normal 기반 rim, scene-depth soft fade, distortion 출력이 있다. 그러나 normal texture와 roughness/metallic을 사용하는 범용 Lit 표면 계산은 없다. 기존 distortion 출력도 그 자체로 두께, 굴절률, 환경 반사까지 갖춘 유리 재질을 의미하지 않는다. 따라서 이번 UI 개편에서 Lit이나 Refraction 버튼만 추가하고 지원된 것처럼 저장하지 않는다. 필요한 확장은 기존 CModel/CMaterial 및 Effect shader 소비 경계에서 따로 구현한다. [현재 픽셀 계산](/C:/Users/user/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli:219).

Queue는 일반 불투명 표면, ground mark, 투명 합성, distortion, screen post의 실행 순서와 관련된다. 임의의 숫자 하나로 모든 객체를 재배치하기보다 재질 preset이 허용하는 queue를 표시하고, 필요한 정렬 보정만 같은 범위에서 제공하는 것이 맞다. Depth Test는 다른 geometry 뒤에서 가려지는지, Depth Write는 뒤에 그려질 geometry의 가림 판단에 영향을 주는지의 차이다. 투명 파편의 depth write를 무조건 켜면 뒤쪽 파편이 사라질 수 있고, 끄면 내부 겹침 정렬 문제가 남을 수 있다. Cull과 양면 표시도 얇은 조각의 geometry 및 원본 state에 맞춰 선택한다.

### G12-3. Alpha와 밝기 손잡이의 소유권

Alpha convention은 별도 계약으로 보여야 한다. 현재 V2 Alpha는 straight alpha 입력을 기대한다. 이미 RGB에 alpha가 곱해진 premultiplied texture를 같은 방식으로 합치면 가장자리 밝기가 달라진다. 반대로 straight 입력을 premultiplied 방식으로 해석해도 밝은 테두리가 생길 수 있다. 향후 convention 선택을 열려면 texture 해석, 픽셀 출력, blend factor를 함께 연결해야 한다. 이름만 바꾸거나 모든 RGB에 alpha를 한 번 더 곱하지 않는다. Multiply의 현재 전처리는 그 pass에 한정된 의도적인 계산이다.

HDR에서는 Effect의 발광량과 장면의 노출을 분리한다. Effect는 자신의 tint, opacity, emissive 기여와 수명별 변화를 소유한다. 장면 Rendering Profile은 exposure, tone mapping, gamma와 전역 Bloom 합성을 소유한다. 현재 renderer도 Effect를 SceneHDR에 합친 뒤 Screen Post, Bloom, Final 순서로 처리한다. Effect마다 별도 exposure override를 넣어 어두운 Effect를 보정하면 같은 장면 안의 상대 밝기가 무너지고, scene 전환이나 Bloom 설정 변경 때 결과가 다시 흔들린다. [장면 합성과 후처리 순서](/C:/Users/user/Desktop/LostArk/Engine/Private/Renderer.cpp:405).

현재 V2 `BloomIntensity`는 emissive texture의 RGB 배수로 사용된다. 전역 Bloom 강도와 동일한 값이 아니다. 향후 Detail에서는 이를 Emissive Gain처럼 실제 역할에 맞춰 설명하고, 활성 Scene의 실효 exposure/Bloom은 읽기 전용 참고값으로 보여주는 구성이 적절하다. 빛을 내지 않는 공 표면이 너무 어두운 상황에서 emissive를 올리는 조작은 표면 조명 복원이 아니다. 반대로 폭발 빛의 HDR 기여가 필요한 상황을 모든 색을 0~1로 제한하는 방식으로 해결하지 않는다.

### G12-4. 창술사 유리 파편과 쿠크 공에 적용할 순서

창술사 유리 파편은 먼저 정확한 skill과 occurrence를 지정하고, 해당 emitter의 생성 위치·시각·개수·속도·수명과 원본 material family를 연결해야 한다. geometry만 유리 모양이어도 원본이 사용하는 coverage, 반사색, Fresnel, 왜곡, crack texture와 particle color 조합이 다르면 다른 효과가 된다. 기존 V1에는 source recipe와 제한된 material family 실행, 특정 프로젝트 조정 유리 계산이 있으므로 그 경로를 먼저 조사한다. 다른 캐릭터용 glass profile이 존재한다는 사실을 창술사 파편의 원본 복원 증거로 사용하지 않는다.

이 경우 V1의 원본 material path, sampler, scalar/vector, source profile 및 execution tuple을 그대로 보존한다. 실제 V1 Detail도 source/execution이 소유한 Render Profile을 읽기 전용으로 취급한다. 공통 V2의 네 blend 값에 억지로 맞추면 원본 계산과 출력 계약이 사라질 수 있다. 필요한 기능은 해당 family의 기존 renderer에 이어 붙이고, 지원되지 않은 요소는 정확히 표시한다. [V1 profile 소유권](/C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp:7521), [V1 pass/state 검증](/C:/Users/user/Desktop/LostArk/Client/Private/Effect_DocumentRenderer.cpp:7127).

쿠크 공은 검증된 model/base texture와 저장 Motion을 이용해 MeshParticle로 출생시키고, 이동 중인 anchor의 출생 시점 transform을 사용한다. world-space 입자는 태어난 뒤 원래 위치와 속도를 유지하고, 각 입자의 seed·방향·회전·수명으로 위아래 움직임을 구성한다. 이 운동 경로는 이번 작업에서 재사용하는 기능이다. 공의 표면이 원본처럼 빛을 받게 만드는 Lit 재질, 실제 바닥 충돌과 반발, 원본 유리 굴절 계산은 각각 별도 기능이다. 생성 궤적이 맞다는 결과를 재질이나 충돌 복원까지 완료됐다고 확대하지 않는다.

### G12-5. 비용과 완료 판단

품질 조절에는 살아 있는 particle 수뿐 아니라 화면을 덮는 면적, 투명 겹침, texture sample 수, submesh draw 수, distortion/full-screen pass 비용을 함께 본다. 작은 공 수십 개와 화면 전체를 덮는 연기 수십 개는 같은 비용이 아니다. Particle Max는 발생 횟수가 아니라 동시 생존 용량이며, 수명과 방출 간격이 그 용량을 결정한다. 먼저 대표 occurrence에서 기존 RenderingBenchmark의 CPU/GPU 시간과 draw·PS invocation을 확인하고, 목표 기기와 해상도에 맞는 예산을 정한다. 아직 측정하지 않은 고정 ms나 입자 수를 보장값으로 제시하지 않는다.

이번 구현의 범위는 기존 창 배치와 Current Effect 흐름, V1/V2 native 저장 보존, 이름과 parent 메타데이터, 공용 slot/detail, 선택 snapshot 및 명시적 재생 연결이다. Material model 신규 구현, queue 재설계, premultiplied 지원, 범용 Lit/Refraction, 전역 tone-map 변경은 이 절의 후속 설계다. 그 작업의 완료에는 shader와 데이터의 실제 소비, 잘못된 조합의 실패 보존, 필요한 최소 빌드 및 동일 카메라·시각·장면 조건에서 사용자의 비교가 필요하다. 문서 작성이나 컴파일 성공만으로 원본 시각 품질을 PASS로 기록하지 않는다.

## G13. 사용자 추가 승인: 유리·에너지·검격 재질의 실제 authoring 경로

도화가 F와 기존 양호한 도화가 Effect는 유지한다. 차원술사 Q/W/E/F, 워로드 번개/충격, 창술사 검격의 부족한 표현을 원본 재질 입력과 현재 실행 수식으로 대조한다. 기존 V1 source profile 및 finite execution을 V2로 자동 변환하거나 기존 사용자 저장본을 새 preset으로 덮어쓰지 않는다.

이번 수직 구현은 기존 V2 renderer에 optional Material Model을 연결한다. 기본 UNLIT는 기존 수식을 보존한다. GLASS는 Mesh/MeshParticle에 geometric/tangent normal, 별도 normal 및 reflection texture, Fresnel, body/edge opacity와 기존 distortion target을 통한 굴절을 제공한다. ENERGY는 원본 texture/noise의 흐름, core/edge 분리와 발광을, SLASH는 원본 geometry/texture를 유지한 UV reveal·soft edge·dissolve edge를 제공한다. 새 재질 수식은 project-authored 기능이며 원본 ABI나 최종 visual fidelity 완료로 기록하지 않는다.

CPU typed Material Params → JSON optional params.material → immutable snapshot → CEffectV2Object shader binding → 기존 Effect shader family → Effect Detail 튜닝·저장·재생을 한 경로로 연결한다. Normal/Reflection slot은 기존 slot index를 바꾸지 않고 뒤에 추가한다. normal은 선형 데이터, reflection은 색 입력 정책을 사용한다. 별도 셰이더 파일은 사용되는 HLSLI family로 작성하고 기존 shader compile/deploy 경로와 필요한 project/filter 항목을 등록한다. GLASS의 sprite/decal/trail/screenpost 조합처럼 구현되지 않은 조합은 잘못된 효과로 조용히 저장하거나 다른 재질로 대체하지 않는다.

화면 합성 순서와 HDR/Exposure/Tonemap/Bloom은 기존 Rendering Profile/Renderer 권위를 사용한다. 재질이 장면 render target을 읽고 동시에 쓰지 않으며 굴절은 기존 distortion 누적 및 후속 resolve를 소비한다. 바뀐 material 저장값의 round-trip, 기존 문서의 기본값, 잘못된 enum/범위/조합 거부, C++ 및 실제 HLSL 컴파일을 확인한다. 원작과의 시각 비교는 사용자 확인을 남긴다.


## G14. 원본 재질 계산 조사 — 기존 Effect 반영은 이번 구현 범위 제외

사용자는 마지막에 범위를 **툴 확장만**으로 한정했다. 아래는 이후 사용자가 재질을 직접 선택·튜닝할 때 사용할 읽기 전용 분석이며, 기존 차원술사·워로드·창술사 Effect 생성, shader 적용, saved JSON 또는 animation binding 변경 계획이 아니다. `Shader_EffectCommon.hlsli`의 기존 Profile 29와 F의 opcode 1004도 변경하지 않는다. G13의 optional V2 재질은 기본 UNLIT를 보존하는 툴 기능이며 기존 V1 스킬에 자동 적용하지 않는다.

### G14-1. 현재 문서가 사용하는 경로

`Data/Balance/PlayerSkills.json`의 차원술사 Q/W/E/F는 각각 `2050100 / 2050120 / 2050160 / 2050230`이다. 현재 Q는 2행, W clip2는 8행, W clip3는 13행 중 11행 visible, E clip4는 10행 중 8행 visible, F 본문은 8행이다. E의 세 중간 animation clip은 같은 clip4 Effect를 참조한다. F animation event는 F 본문 외에 W clip3(450ms), W clip2(590ms), water-burst와 single-glass-canary(각 700ms)를 참조한다. 이 구성은 사용자 저장 상태이며 자동으로 원본 전체 행으로 되돌리지 않는다.

현재 W의 `authored.source-particle.40e1b48e2f0f88dcfeff1549`는 `effect.ue3.glasshole-02.v1` → `Effect_DocumentRenderer.cpp`의 source profile 29 → `Shader_EffectCommon.hlsli`의 bounded 계산을 소비한다. material.execution 필드가 없다는 사실만으로 셰이더 누락이라 판단할 수 없다. Q의 cubesample과 E의 hole/watertrail은 서로 다른 parent family다. F single-glass의 opcode 1004는 검증된 두 occurrence에만 허용되는 별도 project-tuned 실행이며 Q/W/E에 적용된 범용 유리 재질이 아니다.

Material Parameters는 이미 source scalar/vector를 authoring override로 저장한다. 반면 Source Material Profile 패널은 compiler 결과를 읽기 전용으로 보여준다. 따라서 현재 부족한 계산은 UI 슬라이더를 추가하는 것만으로 채울 수 없다.

### G14-2. Glasshole 변수와 실제 계산의 대응

현재 sourceProfile은 `aura_texture`, `cracknormal_tex`, `in_hole_texture` 3개를 소비한다. 저장소에서 제거된 Imported corpus는 다시 설치하지 않고 Git `4e013459^`의 exact-map, uniform-evaluation, texture-closure receipt를 읽었다. 해당 증거와 `Shader_Ue3Glasshole02.hlsli`의 translated 식을 대조한 결과는 다음과 같다. 역사적 receipt의 source exact 표시는 당시 추출 증거의 속성이며 현재 runtime의 완전 복원 판정이 아니다.

| 현재 packed 값 | 원본 증거와 소비 식 | 이번 처리 |
|---|---|---|
| `curve_power` / `main_ucoord` | scalar expression 41/38 → CB0[18].y/[17].z. 반지름 power와 polar 각도를 원본 t4 dust 및 t5 environment 좌표에 사용 | 현재 3개 texture에는 이 두 lane이 없으므로 aura에 대신 연결하지 않음 |
| `uvnoise_utile/vtile/pan` | expression 43/44/46 → CB0[18].w/[19].x/[19].z. 마지막 항목은 pan×time이며 같은 dust/environment UV에 사용 | 누락 texture를 다른 입력으로 대체하지 않음 |
| `main_v_coord` | 선택된 원본 permutation은 `use_dynamic_vcoord=true`. 해당 이름의 scalar expression 없음 | 항상 동작하는 공통 V-offset으로 임의 구현하지 않음 |
| `edge_crack_desaturation` | expression 55 → CB0[21].w. outer radiance를 `lerp(rgb, dot(rgb,[.3,.59,.11]), value)`로 처리한 뒤 inner-hole 색을 더함 | 의미는 확인했지만 기존 Effect 변경 금지에 따라 Profile29에 반영하지 않음 |
| `in_hole_height` | expression 32 → CB0[16].x. `(height×.5−.25) × normalize(cameraToParticle).xy`를 inner UV에 더함 | 현재 Profile29 함수의 view-vector 입력 확장 없이 단순 UV 이동으로 대체하지 않음 |

원본은 material texture 7개와 SceneDepth, camera vector, fog, native MRT/VF를 사용한다. 현재 Profile29의 3-texture 재구성은 이 전체 ABI와 같지 않다. 후속 확장 시 필요한 texture 역할과 view/depth 입력을 먼저 연결하고, 확인된 원본 식과 project-authored 근사를 구별해야 한다. 이번에는 HLSL·C++·Effect 문서 수정이 없다.

### G14-3. 다른 클래스의 현재 저장본과 재사용 경계

워로드 F `effect.warlord.skill.17140.unified`는 현재 4행이다. `mesh_particle_6`의 electric WModel은 존재하지만 material execution/source profile이 없는 현재 사용자 mesh particle이다. 기존 opcode 22의 전기 RT0 함수는 과거 두 stable ID에 한정되고 당시 canary JSON은 현재 없다. 그 함수는 원본 WPO vertex 이동까지 구현한 것이 아니다. 과거 RESULT의 56행을 현재 데이터처럼 설명하거나 새 효과로 복원하지 않는다.

창술사 D `34110`은 현재 2행, F `34150`은 3행이다. F의 dragon mesh, lensflare, ring/trail을 서로 다른 재질로 보존한다. 과거 74/168행 family 복원 문서는 참고 증거이며 현재 사용자 저작 구성과 다르다. 도화가 F `31470`은 현재 17행 모두 실제 inline execution을 가지며 사용자가 양호하다고 확인했으므로 유지한다. 재사용할 것은 source identity → family packet → 기존 renderer dispatch → typed tuning/override라는 계약이지 도화가 Effect나 shader 식의 일괄 복사본이 아니다.

읽기 전용 실측은 `out/EffectCompositionWorkbench/source-family-current-audit.json` 및 `glasshole-historical-equation-evidence.json`에 보관했다. 조사한 11개 현재 Effect 문서에서 실제로 참조하는 WModel/DDS 파일의 누락은 없었다. 파일 존재 확인은 GPU draw나 사용자 화면 판정이 아니다. 이 절은 후속 계산 근거를 보존하며 기존 Effect 변경 및 visual PASS를 기록하지 않는다.

## G15. 최종 사용자 범위 제한

툴과 선택 가능한 셰이더 기능만 확장한다. 실제 Effect 생성, 기존 Effect 수식 변경, 스킬·Pattern binding 생성/교체를 수행하지 않는다. 기존 W Profile29의 계산 변경도 이번 구현에서 제외한다. 새 GLASS/ENERGY/SLASH는 기본 UNLIT 결과를 보존하고 사용자가 직접 선택·튜닝·저장할 때만 사용된다. Data/Effects JSON 392개 기준본의 해시를 보존 검사하며 사용자 Effect를 새 preset으로 덮어쓰지 않는다. 앞선 Nav 및 Patterns by Gate Save 요청은 그대로 구현한다.

## G16. 최종 마감 범위

사용자가 패턴 제작을 우선하고 작업 확대를 멈추도록 요청했다. 추가 Material family/새 shader/관련 publisher 확장은 이번 EXE에서 제외하고 설계만 보존한다. 기존 Effect panel 재사용·Parent tree·Model/Effect Sequencer·World resource·Pattern Save와 승인된 Nav 수정까지만 최종 컴파일하고 배포한다. 기존 스킬 Effect와 JSON은 생성하거나 변경하지 않는다. G13의 실제 재질 수직 구현은 미구현/후속으로 전환한다.
