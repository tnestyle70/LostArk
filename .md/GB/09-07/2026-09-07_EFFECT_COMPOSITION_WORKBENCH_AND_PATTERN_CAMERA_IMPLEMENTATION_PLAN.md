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

## G17. 저장 GROUP의 Append 진입점

2026-09-08 사용자가 `boss.kouku.ball.smoke` GROUP은 Preview되지만 Append가 보이지 않는다고 보고했다.
통합 Effect Sequencer는 이미 `V2_GROUP + stable ID`를 단일 occurrence로 저장하고 기존 V2 group runtime을
호출한다. 실제 누락은 Saved Effects tree의 사용되지 않는 PREVIEW/APPEND command 버튼과 쿠크 Action
Workbench의 Effect source 직접 Append다. Current Effect의 Append도 긴 child 목록 아래에 배치돼 있다.

- `EffectAuthoringResourceTree.cpp`: 선택한 saved resource에 Preview/Append를 표시해 기존 typed command를 보낸다.
- `Effect_Tool.cpp`, `Effect_Tool_Workspace.cpp`: Current Effect의 Append를 child 목록 위로 옮기고 선택 draft의 수명을 사용한다.
- `KoukuSaydonActionWorkbench.h/.cpp`: Effect source를 기존 resource와 typed asset ID로 재사용하거나 candidate에
  등록한 뒤 occurrence까지 검증해 한 번 commit한다. GROUP을 leaf로 풀거나 Effect 파일을 재생성하지 않는다.
- 기존 사용자 occurrence의 offset/rotation/scale/timing과 이미 등록한 resource 설정은 유지한다.

새 C++/project/filter 등록, 별도 Effect runtime, nested group 편집 확장은 없다. 빌드는 전체 요청의 통합
Product 단계에서 확인한다. UI 입력·저장·재로드 및 실제 시각 결과는 사용자가 수행한다.

### G17-1. 쿠크 저장 GROUP 17개와 실제 사용처

아래 표는 2026-09-08 현재 source와 runtime의 대조 결과다. 새 데이터 연결은 전체 패턴 변경 담당자가
Composition의 기존 dirty 변경을 보존해 순차 적용한다. 시간은 현재 저장한 ordered Stage 합계의 ms다.

| GROUP | 기존 사용처 또는 연결 위치 | 보존할 경계 |
|---|---|---|
| `boss.kouku.card.{clober,dia,heart,spade}.{red,black}` 8개 | `Card_Asset(PLAYER_SNAPSHOT)`이 Server 문양·색에 맞는 1개를 선택해 플레이어 머리 위에 지속 재생 | Pattern에 8개를 추가하지 않는다. `CLUB→clober`, `DIAMOND→dia` stable ID를 유지한다. |
| `boss.kouku.disarm` | Pattern 1의 presentation 1~21에 이미 leaf 단위로 연결 | 앞뒤 방패의 별도 offset/yaw, 별/연기/바닥의 5개 위치와 10684ms 구간을 유지한다. GROUP 추가는 중복이다. |
| `boss.kouku.find.heart` | Pattern 2의 presentation 1/2/3, 시작 6767/13167/19567, 각각 2400ms | 기존 `b_effectroot`, yaw 90°, followBoss=true 유지 |
| `boss.kouku.find.star` | Pattern 5의 presentation 1/2/3, 시작 6166/12566/18966, 각각 934ms | 기존 `b_effectroot`, yaw 90°, followBoss=false 유지 |
| `boss.kouku.dance` | Pattern 6 댄스 구간 0~26800의 4색 바닥 | BOSS root를 시작 위치로 잡고 followBoss=false, bone 없음. 마지막 패턴 전환 clip은 제외한다. |
| `boss.kouku.dance.clap` | Pattern 6 양팔모으기 STAGE 590 시작 23300, 2000ms | BOSS root/followBoss=true. leaf에 손 높이 offset이 있으므로 추측한 bone을 더하지 않는다. |
| `boss.kouku.find.core` | 진짜 Pattern 2 3900~23967 및 가짜 Pattern 5 1833~23900의 공통 구체로 연결 가능 | 진짜에만 추가해 퍼즐 답을 노출하지 않는다. 두 Pattern의 기존 heart/star는 별도 cue로 유지한다. |
| `boss.kouku.medusa.blue`, `boss.kouku.medusa.red` | Pattern 11 `대형세이튼_파1빨2`의 두 attack Stage: 0~4000 / 6667~10667 | BOSS root/followBoss=true. 현재 DRAFT의 visual 저작 연결이며 Server judgement 구현을 뜻하지 않는다. |
| `boss.kouku.ball.smoke` | 공 Object Motion 종료 endpoint에서 GROUP 한 번, 2000ms | 보스 고정 위치의 Pattern Effect로 생성하지 않는다. 여러 공은 동일 effect ID를 각 공 끝에서 참조한다. |

smoke의 6색 Texture는 leaf position `(0,2.3,1.35)`, spread Particle는 child+leaf position `(0,4,2)`를
이미 가진다. 공 중심과 6색 smoke 중심을 맞추려면 Object Effect row offset `(0,-2.3,-1.35)`에서
조정하며 원본 leaf를 다시 만들지 않는다. 최종 위치·방향·모양의 판정은 사용자 관찰로 한다.

## G18. WORLD owner 정상 완료와 취소 구분

최종 공 10개 기준 마지막 공은 WORLD 시작 1481ms + 마지막 생성 offset 2700ms + 이동 1700ms + smoke 2000ms로
7881ms까지 필요하다. actor bundle 애니메이션이 7400ms에 정상 완료될 때 기존 `STOP_OWNER`는
남은 WORLD 재생까지 지운다. Shared에 기존 operation 값 0~3을 유지한 `FINISH_OWNER = 4`를
추가하고 같은 owner identity packet 형식을 사용한다. Server의 member/all-run 정상 완료만
FINISH를 보내며 취소·실패·restart는 기존 STOP을 유지한다. Client의 실제 WORLD consumer는
FINISH에서 이미 시작한 재생을 각 authored 수명까지 유지하고 STOP에서 즉시 정리한다.
protocol은 69로 올리고 Server/Client를 함께 재빌드·재시작한다. 기존 packet identity와 field layout은 유지한다.

Shared `PacketMessages.h/cpp`, Server `GameRoom.h/cpp`의 기존 함수 구간만 변경한다. 새 파일이나
프로젝트 등록은 없다. 기존 NetworkProtocolHarness의 world-motion 사례에서 operation prefix,
member/epoch round trip, 잘못된 owner identity와 placement 거절을 확인한다. 컴파일·실행은
통합 담당자가 수행하고 이 하위 작업은 source diff와 `git diff --check`만 확인한다.

## G20. Cinematic Camera Tool의 쿠크 Area 저장 연결

현재 CCameraTool은 Valtan source만 읽는다. 사용자 요청의 `카메라_2관문_세이튼 등장`을 같은
Cut List와 Capture Pos에서 편집하도록 Source에 Kouku Area를 연결한다. PATTERN_ONLY shot의
cameraTrack를 기존 cue/keyframe 편집기가 소비하고, 저장은 기존 Area parser와 MapTool의
CAS 원자 저장을 사용한다. AUTO shot과 기존 Valtan source·추적·게임플레이는 유지한다.

CameraTool.h/cpp는 source 선택과 draft/preview adapter를 소유하고 Level_KakulSaydonArena.h/cpp는
기존 shot→cue 변환, 변경된 pattern shot staging, 검증 뒤 Area source 저장 경계를 제공한다.
새 파일·project/filter 등록·Camera runtime은 없다. 이름은 UTF-8 displayName이고 shotId는 기존
camera.kouku.pattern.N 계약이다. 카메라 진입·복귀 시간과 activation, AUTO/마리오 shot은 보존한다.
독립 편집기의 미저장 draft나 디스크 baseline이 달라지면 저장을 거절하고 기존 상태를 보존한다.

통합 담당자가 source shot과 bundle 공통 Camera occurrence를 연결하고 최소 컴파일·publish를
수행한다. 이 하위 작업은 UI/Client 실행 없이 변경 diff와 실제 save/preview caller를 확인한다.

## G21. 사용자 재생 확인 뒤 등장 높이와 공 생성 구간 조정

사용자는 공 생성과 smoke 재생을 확인했고, 쿠크 상승량과 마지막 생성 위치를 조정하도록 요청했다.
원본 root 최대 상승량은 17.846225m(5433.333ms)다. Pattern 8에 animationRootVerticalScale=0.8을
지정해 14.276980m로 낮추고, Server의 바닥 Y=10.56 및 이동 구간 1870~5780ms는 유지한다.
원본 애니메이션의 수직 복귀는 약 6100ms이며 수평 도착 5780ms와 구분한다.

기존 10번째 공은 4181ms에 (7.802724,10.56,322.765918)에서 생성돼 수평 도착점과 5.664945m
차이가 난다. WORLD occurrence 시작을 1481에서 3080ms로 옮기고 생성 구간을 3000ms로 지정한다.
10개·300ms 간격·공 이동 1700ms·smoke 2000ms는 유지한다. 마지막 생성은 5780ms 도착점이고,
마지막 공의 smoke는 7480~9480ms다. 정상 완료의 WORLD tail 보존 경로를 그대로 소비한다.

Area camera.kouku.pattern.1을 카메라_2관문_세이튼 등장으로 등록하고 bundle.1의 공통 Camera로
0~7400ms 연결한다. 진입 1200ms·hold 6200ms·복귀 1200ms와 SMOOTHSTEP을 사용한다.
초기 wide pose는 eye=(-11.085,40.56,339.87), lookAt=(6.915,15.56,321.87), FOV=60으로 두고
사용자가 Cinematic Camera Tool의 Capture Pos에서 최종 구도를 조정한다. 기존 AUTO/마리오 shot과
134 revision의 조커찾기 무기 앵커 변경은 보존한다. 새 C++ 파일이나 project/filter 등록은 없다.

검증은 관련 컴파일, Map/Composition/Gameplay publish, 수정 JSON/XML parse와 scoped diff check를
실행한다. 카메라 진입·복귀와 조정 높이의 화면 확인은 사용자에게 남긴다.

## G22. 사용자가 저장한 P1/P2 카메라 높이 적용

사용자가 Area camera revision70에 camera.scene.auto.1/auto.2를 저장했다. Eye Y는 각각
25.3147125244/25.3114490509이며 두 Pos 모두 높은 시야의 목표다. 처음 플레이어 시점은
저장 key가 아니라 재생 시작 때 취득하며, 마지막 플레이어 시점도 현재 Follow pose로 계산한다.
사용자가 정한 두 Eye/LookAt/FOV와 key ID는 그대로 보존한다. 진입1200ms, Camera box7400ms,
복귀1200ms를 유지하고 Map domain publish로 서버 재생의 Client 카메라 입력까지 반영한다.
이는 데이터 배포 작업이며 C++/프로젝트/엔진 변경이나 재빌드는 필요하지 않다.

입력은 Follow 상태의 Composition Camera가 gameplay gate를 막지 않는지 실제 caller를 확인한다.
현재 화면 VIEW/PROJ의 우클릭·스킬 aim ray, typed Server command까지 검토하고 직접 UI 검증은
하지 않는다. source/runtime JSON parse·동일성, 기존 Bundle 연결과 scoped diff check를 확인한다.

## G23. 쿠크 Camera Tool의 단일 Pos

사용자가 선택한 P1 한 개를 cameraTrack.keyframes에 남긴다. durationMs는 Camera box의 기존
7400ms를 유지하고 단일 key의 timeMs는 0이다. Level의 기존 Area parser와 Composition publisher의
Area track validator는 1..64개를 허용하며 두 개 이상일 때만 마지막 key가 durationMs와 같아야 한다.
기존 Sample_Cue는 한 key를 처음부터 끝까지 고정 pose로 반환한다. Valtan document와 제품
controller의 최소 두 key 검사는 유지하고 별도 카메라 재생 경로는 만들지 않는다.

Camera Tool의 Kouku Source는 한 Pos로 Capture/선택/Go To Scene/Start/Save를 허용한다. Duration
변경 때도 단일 key의 0ms를 유지하며 static shot adapter도 P1만 보여준다. 두 개 이상인 기존
track은 그대로 읽고 편집한다. 사용자 P1의 Eye/LookAt/FOV와 ID, 진입·복귀 시간은 변경하지 않는다.
원본 P2 삭제와 publish는 통합 담당자가 순차 수행한다. 기존 Valtan 계약 검사와 Area track의
단일 key·잘못된 시작 시각·빈 목록·두 key 종료 경계 검사를 기존 test 파일에서 실행한다.


## G24. 댄스 장판 방향과 사용자 박수 배치·PRODUCT 선택 (2026-09-08)

최신 사용자 저장본의 PATTERN_6 stage/Logic/박수 occurrence를 보존하면서 boss.kouku.dance의
presentation.4에 rotationDegrees [0,90,0]을 설정한다. GROUP/leaf 원본을 회전시키지 않고 Pattern
occurrence의 Y축 회전만 바꾼다. 장판0~26800ms와 사용자 박수 presentation.6~19 총14개의 각
시작 시각·2000ms 수명을 유지한다. 앞선 G17/G19의 박수 한 개 연결은 이전 기준점이다.

P6을 PRODUCT로 설정하고 기존 authored 순서의 playAllPatternIds에 반영한다. Patterns 목록에는
선택 Pattern의 Set Pattern to PRODUCT 버튼을 기존 Set_PatternAuthoringStatus로 연결한다.
검증 실패는 기존 draft를 보존하고 성공 뒤 Save가 필요함을 표시한다. 기존 CAS Save와
Map/Composition/Gameplay publish·Server 재시작을 실제 반영 경계로 유지한다. 버튼·저장 API의
상세 구현/검증은 Gate Pattern Bundle PLAN/RESULT G21을 정본으로 사용한다.

변경은 기존 Composition JSON과 Workbench의 목록 UI이며 새 Effect asset·C++ 파일·project/filter
등록은 없다. JSON parse, 장판 rotation 외 사용자 값 보존과 박수14개 유지, 해당 domain publish 및
최소 Product 컴파일을 확인한다. 실제 장판 방향·박수 겹침/타이밍은 사용자가 아레나에서 판정한다.


## G25. Collider Box Detail의 즉시 geometry Preview (2026-09-08)

Position/Rotation/Scale과 Width/Height/Depth 또는 Radius 편집은 현재 표시 clock에서 즉시
Collider wire에 반영한다. resource의 HalfExtents/Radius는 유지하고 box Scale로 환산한다.
편집값은 Detail에 남으며 Apply/Save 이전에는 Composition draft, generation과 파일을 바꾸지 않는다.
시간·Bone·Logic 등 다른 미적용 필드는 geometry Preview에 섞지 않는다.

Workbench의 KOUKU_COLLIDER_GEOMETRY_PREVIEW_REQUEST는 stable Pattern ID와 occurrence를
전달하고 Request_ColliderGeometryPreview가 원본 occurrence의 세 geometry 배열만 덮어쓴다.
같은 occurrence의 연속 drag를 하나로 합치며 active Pattern 및 Bundle member에는 새 재생·Seek를
요청하지 않는다. inactive일 때만 현재 cursor에서 기존 paused Pattern Preview를 한 번 준비한다.
끝 cursor는 duration을 유지한다. Cancel/선택 동기화는 현재 적용된 draft geometry로 복구하고,
새 Preview/계층 전환 Reset은 폐기되는 session의 오래된 overlay request를 함께 버린다.

MainApp은 Pattern Preview 준비 뒤 geometry queue를 소비한다. PresentationPlayer는 정확한
Pattern/member·occurrence·Collider resource를 확인하고 해당 wire만 갱신한다. 기존 actor, clock,
Effect/SFX handle과 다른 session은 유지한다. follow=false Collider는 처음 확정한 anchor basis와
placement scale을 보관해 drag 중 움직이는 actor를 새 anchor로 취득하지 않는다.

수정 범위는 기존 Workbench H/CPP, MainApp, PresentationPlayer H/CPP와 기존 focused native
harness다. 새 schema·C++ 파일·project/filter 등록은 없다. running/paused와 Bundle child, cold/end
cursor, invalid 값 보존, 연속 편집 합치기, Cancel/선택 복구와 authoring 무변경을 기존
--kouku-preview-transport-contract에서 검사한다. Product 빌드 후 실제 wire 즉시 반영은 사용자가 확인한다.


## G27. V2 Effect Box Detail의 Sequencer 기준 즉시 P/R/S 편집 (2026-09-08)

Effect Box Detail의 Position/Rotation/Scale은 실제 Pattern 또는 active Bundle member의 현재
clock과 actor/bone/WORLD anchor에 즉시 반영한다. Detail Preview도 같은 owner 경로를 사용한다.
독립 resource 목록의 Preview만 기존 자원 미리보기로 남긴다. Effect 편집값은 stable Pattern ID와
occurrence ID별로 보관해 선택을 바꿔도 유지하고, Save 후보에 geometry 세 배열만 모아 검증한 뒤
기존 CAS 저장이 성공하면 commit한다. 미적용 timing/Bone/Logic은 자동 저장하지 않는다. 잘못된 값과
외부 파일 변경은 기존 파일을 보존하며 명시 Revert는 해당 box geometry를 취소한다.

기존 Collider geometry request를 COLLIDER/EFFECT 공통 typed request로 확장한다. PresentationPlayer는
실제 session의 선택 row만 갱신한다. Effect는 기존 V2 GROUP/LEAF handle, clock, pause, snapshot을
유지하고 새 pivot sampler로 같은 age의 기존 child 객체를 다시 계산해 월드 공간 particle birth도 갱신한다.
이미 렌더 큐에 들어간 객체를 교체하지 않아 연속 drag 도중 빈 프레임이 생기지 않게 한다.
전체 Pattern/Bundle과 무관한 Effect/Sound를 재시작하지 않는다. frozen anchor는 처음 위치를 보존하고,
bone/WORLD history에는 편집 geometry 전의 resolved anchor와 WORLD scale을 기록한다. sampler가
새 geometry를 합성하며 기록되지 않은 과거를 현재 actor 위치로 위장하지 않는다.

V2 Object의 pivot scale 소비와 trail의 과거 pivot sampling도 같은 경로에서 확인한다. 기존 C++와
기존 focused native 검사만 수정하며 신규 runtime, schema, project/filter 항목은 없다. Workbench의
running/paused/Bundle/선택전환/복수 box Save·reload/invalid·CAS 보존, V2의 동일 age 재계산과 실제
runtime 컴파일을 확인한다. 최종 Product EXE 교체는 실행 프로세스 종료 뒤 수행하며 화면의 위치·
크기·회전과 이펙트 모양은 사용자가 직접 판정한다.

Stop/paused seek는 같은 실행의 활성 Effect row와 raw anchor history를 보존하고 V2 절대 clock으로
되감는다. 과거 시점은 이미 기록한 표본을 사용하며 history에 역순으로 append하지 않는다. Reset과
새 Pattern 실행은 기존 Stop_Session으로 완전히 해제한다. WORLD anchor 준비를 기다린 Effect는
anchor가 생긴 뒤 기존 GROUP/LEAF 경로로 재시도한다.

## G28. 캐릭터 Effect Sequencer와 Composition 조작 통일 (2026-09-09)

사용자는 기존 Effect Sequencer를 Action Workbench와 같은 ImGui 구성으로 확장하고,
Animation·Effect·Collider·Sound·Camera·Screen Post를 한 시간 커서에서 편집·재생하도록 요청했다.
맨 위 ruler 전체를 드래그하면 노란 playhead와 모든 트랙의 표본 시점이 함께 이동한다.
Effect 행은 개별 element로 펼치지 않고 기존 typed effect resource occurrence를 표시한다.
다른 세션 변경의 조율은 사용자가 맡는다고 확인했으며 기존 dirty 내용은 보존한다.

`CEffectAuthoringSequencer`가 문서와 단일 clock을 계속 소유한다. 기존 `CompositionTimeline`의
ruler/box/edge gesture와 `CompositionResourceTree`를 재사용하고, Composition Resources와
Box Detail 창을 Effect Tool별 ID로 구분한다. 고정 여섯 lane에 겹치는 occurrence만 추가 subrow로
배치한다. 선택·검색·드래그마다 전체 Effect JSON을 다시 검사하지 않는다. 목록은 명시 Refresh와
처음 열린 catalog의 metadata만 읽고, 실제 Append/Play에서 선택 자원의 기존 owner가 검증한다.

현재 모델의 실제 clip 이름을 Animation resource로 제공한다. 저장된 skillbinding sequence는
가져오는 원본이며 수정한 animation occurrence는 별도 sequence 문서에 저장한다. 단일 CModel의
활성 animation 구간 겹침은 거절하며 sourceStart/sourcePlay/playRate/loop와 wall timing을 분리한다.
원본 skillbindings나 Server gameplay timing을 이 편집이 암묵적으로 바꾸지 않는다.

Sound는 Resources-relative asset ID와 occurrence별 FMOD handle을 사용한다. 기존
Play_SoundCue/Pause/Seek/Stop 경계를 연결하여 여러 사운드를 같은 clock에서 재생하고, 멈춤·
되감기·loop·자원 해제에서 자신의 handle만 정리한다. Collider는 Action Workbench의 saved
presentationResources 형상 정의와 `CHitAreaWire`를 재사용해 root/bone에 연결한다. BOX/CIRCLE/
SECTOR의 크기·위치·회전·발생구간을 저장하고 표시하며 combat 판정은 기존 Server 권위를 유지한다.
Screen Post는 기존 V2 SCREEN_POST leaf를 별도 lane에 놓고 기존 Effect runtime으로 재생한다.
Camera는 기존 camera key·원본 recovery camera·presentation override 소유권을 유지한다.

`.effectsequence.json` v4는 customAnimation/animationRows/soundRows/colliderRows와 effect의
screenPost 분류를 추가하고 v1~3을 계속 읽는다. 모든 추가 행은 stable occurrence ID, 유한 값,
시간 범위, Resources 상대 경로와 named bone을 검사한다. Load는 parse/validate/stage/commit,
Save는 기준 파일 변경 거절과 atomic write를 유지한다. 실패한 후보로 현재 문서를 부분 교체하지 않는다.

기존 `EffectAuthoringSequencer.h/.cpp`를 확장하고 표시·codec·presentation을 같은 class의
`_Timeline.cpp`, `_Resources.cpp`, `_Tracks.cpp`, `_Presentation.cpp`로 나눈다. 네 CPP는
Client.vcxproj와 filters에 실제 물리 위치를 등록한다. V1 목록 조회는 기존
EffectAuthoringResourceTree의 read-only metadata 함수를 재사용한다. 새 병렬 runtime은 없다.
실제 clock/저장 왕복·잘못된 입력 보존·동시 사운드 handle 정리를 필요한 기존 검사 방식으로
확인하고 최소 C++ compile 및 Debug Product를 순차 실행한다. Client 조작·화면·청각 판정은 사용자에게 인계한다.


### G28 새 CPP의 책임과 전체 코드

새 CPP 네 개는 독립 런타임이 아니라 `CEffectAuthoringSequencer`의 같은 멤버 상태를 사용한다. 아래 코드는 기존 class의 선언과 실제 project/filter 등록을 소비한다. 코드 전문은 화면 검증 증거가 아니다.

#### EffectAuthoringSequencer_Timeline.cpp

`Select_TimelineRow`는 stable ID 선택을 소유한다. `Apply_AnimationRow`, `Apply_EffectRow`, `Apply_SoundRow`, `Apply_ColliderRow`는 편집 후보를 검증하고 선택 occurrence만 교체한다. Animation 실패는 이전 배열과 pause/dirty를 복구하고, Effect/Sound는 새 객체·채널 준비 실패 때 이전 것을 유지한다. `Remove_SelectedRow`와 `Duplicate_SelectedRow`는 종류별 실제 자원을 정리·복사한다. `Render_Sequencer`는 toolbar와 공통 ruler/box gesture를 표시하고 드래그 종료 시 한 번 적용한다. UI의 preview timing과 저장된 행을 분리한다.

<details>
<summary>EffectAuthoringSequencer_Timeline.cpp 전체 코드</summary>

```cpp
#include "imgui.h"
#include "EffectAuthoringSequencer.h"
#include "CompositionTimeline.h"
#include "EffectEditingSession.h"
#include "Effect_Object.h"
#include "GameInstance.h"
#include "RuntimeAssetRoot.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>

namespace Client
{
namespace
{
constexpr std::uint32_t LIMIT_MS = 600000u;
template<class Rows> auto FindRow(Rows& rows, const std::string& id)
{
    return std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.id == id; });
}
bool FinitePosition(const float3_t& value)
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) &&
        std::abs(value.x) <= 100000.f && std::abs(value.y) <= 100000.f && std::abs(value.z) <= 100000.f;
}
}

void CEffectAuthoringSequencer::Select_TimelineRow(const TRACK_KIND kind, const std::string& id)
{
    m_SelectedTrack = kind; m_SelectedRowId = id; m_BoxDetailOpen = true;
    m_SelectedEffect = kind == TRACK_KIND::EFFECT || kind == TRACK_KIND::SCREEN_POST ? id : "";
    m_SelectedCamera = kind == TRACK_KIND::CAMERA ? id : "";
}

bool CEffectAuthoringSequencer::Apply_AnimationRow(const CLIP& row)
{
    if (m_UseKouku) { m_Status = "Edit the saved boss composition in Action Workbench."; return false; }
    auto candidate = m_CustomAnimation ? m_AnimationRows :
        (Selected_Sequence() ? Selected_Sequence()->clips : std::vector<CLIP>{});
    auto found = FindRow(candidate, row.id);
    if (found == candidate.end()) { m_Status = "The selected Animation occurrence is unavailable."; return false; }
    *found = row;
    if (!Validate_AnimationRows(candidate)) return false;
    auto previous = m_AnimationRows; const bool wasCustom = m_CustomAnimation, wasDirty = m_Dirty;
    m_AnimationRows = std::move(candidate); m_CustomAnimation = true;
    if (!Refresh_AnimationTiming())
    {
        const auto error = m_Status; m_AnimationRows = std::move(previous); m_CustomAnimation = wasCustom;
        Refresh_AnimationTiming(); m_Dirty = wasDirty; m_Status = error; return false;
    }
    m_BoxDetailDraft.reset(); m_Dirty = true; m_Status = "Animation occurrence applied."; return true;
}

bool CEffectAuthoringSequencer::Apply_EffectRow(const EFFECT_ROW& value)
{
    auto found = FindRow(m_Effects, value.id);
    if (found == m_Effects.end()) { m_Status = "Append this preview before editing its occurrence."; return false; }
    if (!FinitePosition(value.offset)) { m_Status = "Effect offset must be finite and within 100000 m."; return false; }
    auto candidate = value;
    candidate.v1.reset(); candidate.v2 = 0u; candidate.history.reset(); candidate.anchorHistory.reset();
    candidate.sampledAge = candidate.recordedAge = -1.f;
    float4x4_t root = m_WorldRoot;
    if (m_Active && !Resolve_Root(root)) return false;
    if (!Stage_Row(candidate, root) || (m_Active && !m_Transient && !Sample_Row(candidate, root)))
    { Release_Row(candidate); return false; }
    Release_Row(*found); *found = std::move(candidate); m_Dirty = true;
    m_BoxDetailDraft.reset(); Preserve_ClockDuringAuthoring(); m_Status = "Effect occurrence applied."; return true;
}

bool CEffectAuthoringSequencer::Apply_SoundRow(const SOUND_ROW& value)
{
    auto candidate = m_Sounds; auto found = FindRow(candidate, value.id);
    if (found == candidate.end()) { m_Status = "The selected Sound occurrence is unavailable."; return false; }
    const auto index = static_cast<std::size_t>(found - candidate.begin());
    *found = value;
    if (!Validate_SoundRows(candidate)) return false;
    auto staged = value; staged.handle = 0u; staged.sampledAge = -1;
    auto& sound = CGameInstance::Get();
    if (m_Active && !m_Transient && !staged.muted && ClockMs() >= staged.startMs &&
        std::uint64_t(ClockMs()) < std::uint64_t(staged.startMs) + staged.durationMs)
    {
        const auto path = CRuntimeAssetRoot::Resolve(std::filesystem::path(
            std::u8string(staged.assetId.begin(), staged.assetId.end())));
        std::uint32_t sourceDuration = 0u;
        if (path.empty() || !sound.Get_SoundDurationMs(path.wstring(), sourceDuration))
        { m_Status = "Sound source preparation failed; the previous occurrence is preserved."; return false; }
        const auto age = staged.sourceStartMs + ClockMs() - staged.startMs;
        if (age < sourceDuration)
        {
            staged.handle = sound.Play_SoundCue(path.wstring(), staged.volume, age, true);
            if (!staged.handle)
            { m_Status = "Sound channel preparation failed; the previous occurrence is preserved."; return false; }
        }
        staged.sampledAge = age;
    }
    if (m_Sounds[index].handle) sound.Stop_SoundCue(m_Sounds[index].handle);
    m_Sounds[index] = std::move(staged);
    if (m_Sounds[index].handle) sound.Pause_SoundCue(m_Sounds[index].handle, m_Paused);
    m_BoxDetailDraft.reset(); m_Dirty = true; m_Status = "Sound occurrence applied."; return true;
}

bool CEffectAuthoringSequencer::Apply_ColliderRow(const COLLIDER_ROW& value)
{
    auto candidate = m_Colliders; auto found = FindRow(candidate, value.id);
    if (found == candidate.end()) { m_Status = "The selected Collider occurrence is unavailable."; return false; }
    *found = value;
    if (!Validate_ColliderRows(candidate) || !Validate_Anchor(value.anchorSlotId, m_ModelRoot, m_UseKouku)) return false;
    m_Colliders = std::move(candidate); m_BoxDetailDraft.reset(); m_Dirty = true; m_Status = "Collider occurrence applied."; return true;
}

bool CEffectAuthoringSequencer::Remove_SelectedRow()
{
    if (m_SelectedRowId.empty()) return false;
    switch (m_SelectedTrack)
    {
    case TRACK_KIND::ANIMATION:
    {
        if (m_UseKouku) return false;
        auto candidate = m_CustomAnimation ? m_AnimationRows :
            (Selected_Sequence() ? Selected_Sequence()->clips : std::vector<CLIP>{});
        auto row = FindRow(candidate, m_SelectedRowId); if (row == candidate.end()) return false;
        auto previous = m_AnimationRows; const bool wasCustom = m_CustomAnimation, wasDirty = m_Dirty;
        candidate.erase(row); m_AnimationRows = std::move(candidate); m_CustomAnimation = true;
        if (!Refresh_AnimationTiming())
        {
            const auto error = m_Status; m_AnimationRows = std::move(previous); m_CustomAnimation = wasCustom;
            Refresh_AnimationTiming(); m_Dirty = wasDirty; m_Status = error; return false;
        }
        break;
    }
    case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST:
    {
        auto row = FindRow(m_Effects, m_SelectedRowId); if (row == m_Effects.end()) return false;
        Release_Row(*row); m_Effects.erase(row); break;
    }
    case TRACK_KIND::SOUND:
    {
        auto row = FindRow(m_Sounds, m_SelectedRowId); if (row == m_Sounds.end()) return false;
        if (row->handle) CGameInstance::Get().Stop_SoundCue(row->handle);
        m_Sounds.erase(row); break;
    }
    case TRACK_KIND::COLLIDER:
    {
        auto row = FindRow(m_Colliders, m_SelectedRowId); if (row == m_Colliders.end()) return false;
        m_Colliders.erase(row); break;
    }
    case TRACK_KIND::CAMERA:
    {
        auto row = FindRow(m_CameraRows, m_SelectedRowId); if (row == m_CameraRows.end()) return false;
        m_CameraRows.erase(row);
        if (m_Active) { float4x4_t root; if (Resolve_Root(root)) Sample_Camera(ClockMs(), root); }
        break;
    }
    }
    m_SelectedRowId.clear(); m_SelectedEffect.clear(); m_SelectedCamera.clear();
    m_BoxDetailDraft.reset(); m_Dirty = true; m_Status = "Occurrence removed."; return true;
}

bool CEffectAuthoringSequencer::Duplicate_SelectedRow()
{
    const auto nextStart = [](std::uint32_t start, std::uint32_t duration)
        { return duration <= LIMIT_MS && start <= LIMIT_MS - duration && start + duration <= LIMIT_MS - duration; };
    switch (m_SelectedTrack)
    {
    case TRACK_KIND::ANIMATION:
    {
        if (m_UseKouku) break;
        auto candidate = m_CustomAnimation ? m_AnimationRows :
            (Selected_Sequence() ? Selected_Sequence()->clips : std::vector<CLIP>{});
        auto found = FindRow(candidate, m_SelectedRowId);
        if (found == candidate.end() || !nextStart(found->startMs, found->durationMs)) break;
        auto row = *found; row.id = CEffectEditingSession::New_Id("animation.occurrence."); row.startMs += row.durationMs;
        candidate.push_back(row); if (!Validate_AnimationRows(candidate)) return false;
        auto previous = m_AnimationRows; const bool wasCustom = m_CustomAnimation, wasDirty = m_Dirty;
        m_AnimationRows = std::move(candidate); m_CustomAnimation = true;
        if (!Refresh_AnimationTiming())
        {
            const auto error = m_Status; m_AnimationRows = std::move(previous); m_CustomAnimation = wasCustom;
            Refresh_AnimationTiming(); m_Dirty = wasDirty; m_Status = error; return false;
        }
        Select_TimelineRow(TRACK_KIND::ANIMATION, row.id); m_Dirty = true; return true;
    }
    case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST:
    {
        auto found = FindRow(m_Effects, m_SelectedRowId);
        if (found == m_Effects.end() || m_Effects.size() >= 256u || !nextStart(found->startMs, found->durationMs)) break;
        auto row = *found; row.id = CEffectEditingSession::New_Id("effect.occurrence."); row.startMs += row.durationMs;
        row.v1.reset(); row.v2 = 0u; row.history.reset(); row.anchorHistory.reset(); row.sampledAge = row.recordedAge = -1.f;
        float4x4_t root = m_WorldRoot;
        if (m_Active && !Resolve_Root(root)) return false;
        if (!Stage_Row(row, root) || (m_Active && !m_Transient && !Sample_Row(row, root))) { Release_Row(row); return false; }
        Select_TimelineRow(m_SelectedTrack, row.id); m_Effects.push_back(std::move(row)); m_Dirty = true; return true;
    }
    case TRACK_KIND::SOUND:
    {
        auto found = FindRow(m_Sounds, m_SelectedRowId);
        if (found == m_Sounds.end() || !nextStart(found->startMs, found->durationMs)) break;
        auto row = *found; row.id = CEffectEditingSession::New_Id("sound.occurrence."); row.startMs += row.durationMs;
        row.handle = 0u; row.sampledAge = -1;
        auto candidate = m_Sounds; candidate.push_back(row); if (!Validate_SoundRows(candidate)) return false;
        m_Sounds = std::move(candidate); Select_TimelineRow(TRACK_KIND::SOUND, row.id); m_Dirty = true; return true;
    }
    case TRACK_KIND::COLLIDER:
    {
        auto found = FindRow(m_Colliders, m_SelectedRowId);
        if (found == m_Colliders.end() || !nextStart(found->startMs, found->durationMs)) break;
        auto row = *found; row.id = CEffectEditingSession::New_Id("collider.occurrence."); row.startMs += row.durationMs;
        auto candidate = m_Colliders; candidate.push_back(row); if (!Validate_ColliderRows(candidate)) return false;
        m_Colliders = std::move(candidate); Select_TimelineRow(TRACK_KIND::COLLIDER, row.id); m_Dirty = true; return true;
    }
    case TRACK_KIND::CAMERA:
    {
        auto found = FindRow(m_CameraRows, m_SelectedRowId);
        if (found == m_CameraRows.end() || !nextStart(found->startMs, found->cue.iDurationMs)) break;
        auto row = *found; row.id = CEffectEditingSession::New_Id("camera.row."); row.cue.strCueId = row.id;
        row.startMs += row.cue.iDurationMs;
        auto candidate = m_CameraRows; candidate.push_back(row); if (!Validate_CameraRows(candidate)) return false;
        m_CameraRows = std::move(candidate); Select_TimelineRow(TRACK_KIND::CAMERA, row.id); m_Dirty = true; return true;
    }
    }
    m_Status = "The selected occurrence cannot be duplicated at its end time."; return false;
}

void CEffectAuthoringSequencer::Render_Sequencer(const char* title)
{
    ImGui::SetNextWindowSize({1180.f, 420.f}, ImGuiCond_FirstUseEver);
    const bool expanded = ImGui::Begin(title, nullptr, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("Composition Resources", nullptr, &m_ResourcesOpen);
            ImGui::MenuItem("Box Detail", nullptr, &m_BoxDetailOpen);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    if (expanded)
    {
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) m_Interaction = true;
        if (ImGui::Button("Play")) { if (m_ClockMs >= DurationMs()) m_ClockMs = 0; Play(); }
        ImGui::SameLine(); if (ImGui::Button(m_Paused ? "Resume" : "Pause")) Pause(!m_Paused);
        ImGui::SameLine(); if (ImGui::Button("Restart")) { m_ClockMs = 0; Play(); }
        ImGui::SameLine(); if (ImGui::Button("Stop")) { Stop(); m_ClockMs = 0; }
        ImGui::SameLine(); ImGui::Checkbox("Loop", &m_Loop);
        ImGui::SameLine(); if (ImGui::Button("Refresh Effects")) Refresh_Effects();
        ImGui::SameLine(); if (ImGui::Button("Resources")) m_ResourcesOpen = true;
        ImGui::SameLine(); if (ImGui::Button("Details")) m_BoxDetailOpen = true;
        ImGui::SetNextItemWidth(255.f); ImGui::InputText("Sequence ID", m_SequenceId, sizeof(m_SequenceId));
        ImGui::SameLine(); if (ImGui::Button("Save")) Save_Sequence();
        ImGui::SameLine(); if (ImGui::Button("Load")) Load_Sequence();
        ImGui::SameLine(); if (ImGui::Button("Revert")) Load_Sequence(true);
        ImGui::SameLine(); if (ImGui::Button("New"))
        {
            if (m_Dirty) m_Status = "Save or Revert this sequence before creating another.";
            else
            {
                Stop(); m_Effects.clear(); m_CameraRows.clear(); m_Sounds.clear(); m_Colliders.clear();
                m_AnimationRows.clear(); m_CustomAnimation = false; m_SelectedSequence.clear(); m_UseKouku = false;
                m_SelectedCamera.clear(); m_SelectedEffect.clear(); m_SelectedRowId.clear(); m_BoxDetailDraft.reset();
                m_ClockMs = 0; m_NextEffectOrdinal = 1u;
                std::snprintf(m_SequenceId, sizeof(m_SequenceId), "%s", CEffectEditingSession::New_Id("effect.sequence.").c_str());
                m_SequenceBaseline.clear(); m_PersistedSequenceId.clear(); m_SequenceExisted = false; m_Dirty = true;
            }
        }
        if (m_Dirty) { ImGui::SameLine(); ImGui::TextDisabled("Unsaved"); }
        int clock = static_cast<int>(ClockMs());
        ImGui::SetNextItemWidth(300.f);
        if (ImGui::SliderInt("Time", &clock, 0, static_cast<int>(DurationMs()), "%d ms")) Seek(clock);
        ImGui::SameLine(); ImGui::SetNextItemWidth(130.f); ImGui::SliderFloat("Zoom", &m_Zoom, 10.f, 300.f, "%.0f px/s");
        ImGui::SameLine(); if (ImGui::Button("Duplicate")) Duplicate_SelectedRow();
        ImGui::SameLine(); if (ImGui::Button("Remove")) Remove_SelectedRow();
        const auto firstLine = m_Status.substr(0, m_Status.find('\n'));
        ImGui::TextUnformatted(firstLine.c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", m_Status.c_str());

        struct BOX final
        {
            TRACK_KIND kind; std::string id, label; std::uint32_t start, duration;
            bool muted, editable;
        };
        std::array<std::vector<BOX>, 6> lanes;
        auto add = [&](TRACK_KIND kind, const std::string& id, const std::string& label,
            std::uint32_t start, std::uint32_t duration, bool muted, bool editable = true)
        { lanes[static_cast<std::size_t>(kind)].push_back({kind, id, label, start, duration, muted, editable}); };
        if (m_UseKouku)
            for (const auto& clip : m_Kouku.Rows())
                add(TRACK_KIND::ANIMATION, clip.memberId + "." + clip.occurrenceId, clip.runtimeClip,
                    clip.startMs, clip.durationMs, false, false);
        else
        {
            const auto* sequence = Selected_Sequence();
            const auto* clips = m_CustomAnimation ? &m_AnimationRows : (sequence ? &sequence->clips : nullptr);
            if (clips) for (const auto& clip : *clips)
                add(TRACK_KIND::ANIMATION, clip.id, clip.label.empty() ? clip.clipName : clip.label,
                    clip.startMs, clip.durationMs, clip.muted);
        }
        auto addEffect = [&](const EFFECT_ROW& row, bool transient)
        {
            const auto resource = std::find_if(m_CompositionResources.begin(), m_CompositionResources.end(),
                [&](const auto& entry) { return entry.key == row.key; });
            const auto label = resource == m_CompositionResources.end() ? row.key.strStableId : resource->label;
            add(row.screenPost ? TRACK_KIND::SCREEN_POST : TRACK_KIND::EFFECT, row.id,
                transient ? "Preview / " + label : label, row.startMs, row.durationMs, row.muted, !transient);
        };
        for (const auto& row : m_Effects) addEffect(row, false);
        if (m_Transient) addEffect(*m_Transient, true);
        for (const auto& row : m_Colliders) add(TRACK_KIND::COLLIDER, row.id, row.label, row.startMs, row.durationMs, row.muted);
        for (const auto& row : m_Sounds) add(TRACK_KIND::SOUND, row.id, row.label, row.startMs, row.durationMs, row.muted);
        const auto& cameras = m_Transient ? m_TransientCameraRows : m_CameraRows;
        for (const auto& row : cameras) add(TRACK_KIND::CAMERA, row.id, row.label, row.startMs, row.cue.iDurationMs, row.muted, !m_Transient);
        constexpr float labels = 160.f, rowHeight = 29.f;
        const auto canvasMs = (std::min)(LIMIT_MS, (std::max)(10000u, DurationMs() + 1000u));
        const float width = (std::max)(ImGui::GetContentRegionAvail().x - 5.f, labels + canvasMs * m_Zoom * .001f + 30.f);
        ImGui::SetNextWindowContentSize({width, 0.f});
        if (ImGui::BeginChild("Timeline", {0.f, 0.f}, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
        {
            const auto origin = ImGui::GetCursorScreenPos(); auto* draw = ImGui::GetWindowDrawList();
            ImGui::TextDisabled("0  Timeline");
            CompositionTimeline::DrawRuler(draw, {origin.x + labels, origin.y}, {origin.x + width, origin.y + rowHeight}, canvasMs, m_Zoom);
            ImGui::SetCursorScreenPos({origin.x + labels, origin.y});
            ImGui::InvisibleButton("Timeline cursor", {width - labels, rowHeight});
            if (ImGui::IsItemActive())
            {
                const auto ms = static_cast<std::uint32_t>((std::clamp)(
                    (ImGui::GetIO().MousePos.x - origin.x - labels) * 1000.f / m_Zoom, 0.f, float(DurationMs())));
                if (!m_Active || ms != ClockMs()) Seek(ms);
                m_Interaction = true;
            }
            const char* names[] = {"Animation", "Effect", "Collider", "Sound", "Camera", "Screen Post"};
            const ImU32 colors[] = {IM_COL32(68,125,177,230), IM_COL32(173,107,48,230), IM_COL32(65,171,165,230),
                IM_COL32(130,176,83,230), IM_COL32(118,85,184,230), IM_COL32(190,90,144,230)};
            float y = origin.y + rowHeight;
            for (std::size_t lane = 0; lane < lanes.size(); ++lane)
            {
                auto& boxes = lanes[lane];
                std::stable_sort(boxes.begin(), boxes.end(), [](const auto& a, const auto& b) { return a.start < b.start; });
                std::vector<std::uint32_t> rowEnds;
                std::vector<std::size_t> positions;
                for (const auto& box : boxes)
                {
                    std::size_t row = 0;
                    while (row < rowEnds.size() && rowEnds[row] > box.start) ++row;
                    if (row == rowEnds.size()) rowEnds.push_back(0u);
                    rowEnds[row] = box.start + box.duration; positions.push_back(row);
                }
                const auto rowCount = (std::max)(std::size_t{1}, rowEnds.size());
                const float height = rowHeight * static_cast<float>(rowCount);
                draw->AddRectFilled({origin.x, y}, {origin.x + width, y + height},
                    lane % 2 ? IM_COL32(31,34,41,255) : IM_COL32(38,41,49,255));
                draw->AddLine({origin.x, y}, {origin.x + width, y}, IM_COL32(64,68,76,255));
                draw->AddText({origin.x + 8.f, y + 6.f}, colors[lane], names[lane]);
                if (boxes.empty()) draw->AddText({origin.x + labels + 8.f, y + 6.f}, IM_COL32(115,118,127,255), "Add from Composition Resources");
                for (std::size_t i = 0; i < boxes.size(); ++i)
                {
                    const auto& box = boxes[i];
                    const bool dragging = m_DragRowId == box.id && m_DragTrack == box.kind;
                    const auto startMs = dragging ? m_DragPreviewStartMs : box.start;
                    const auto durationMs = dragging ? m_DragPreviewDurationMs : box.duration;
                    const float top = y + positions[i] * rowHeight + 2.f;
                    const float left = origin.x + labels + startMs * m_Zoom * .001f;
                    const float right = (std::max)(left + 6.f, left + durationMs * m_Zoom * .001f);
                    CompositionTimeline::DrawBox(draw, {left, top}, {right, top + rowHeight - 5.f},
                        box.muted ? IM_COL32(73,75,81,200) : colors[lane],
                        m_SelectedTrack == box.kind && m_SelectedRowId == box.id, box.label.c_str(), box.editable, box.editable);
                    ImGui::SetCursorScreenPos({left, top}); ImGui::PushID(static_cast<int>(lane)); ImGui::PushID(box.id.c_str());
                    ImGui::InvisibleButton("Occurrence", {right - left, rowHeight - 5.f});
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%u ms / %u ms%s", box.label.c_str(), startMs, durationMs, box.muted ? " / Muted" : "");
                    if (ImGui::IsItemActivated())
                    {
                        Select_TimelineRow(box.kind, box.id); m_Interaction = true;
                        if (box.editable)
                        {
                            m_DragRowId = box.id; m_DragTrack = box.kind; m_DragMouseX = ImGui::GetIO().MousePos.x;
                            m_DragStartMs = m_DragPreviewStartMs = box.start; m_DragDurationMs = m_DragPreviewDurationMs = box.duration;
                            m_DragWasPaused = m_Paused; Pause(true);
                            m_DragKind = static_cast<int>(CompositionTimeline::HitBoxGesture(m_DragMouseX, left, right, 6.f, true, true));
                        }
                    }
                    if (box.editable && dragging && ImGui::IsItemActive())
                    {
                        const auto delta = static_cast<std::int64_t>(std::llround((ImGui::GetIO().MousePos.x - m_DragMouseX) * 1000. / m_Zoom));
                        std::int64_t start = m_DragStartMs, duration = m_DragDurationMs;
                        if (m_DragKind == static_cast<int>(CompositionTimeline::BoxGesture::MOVE))
                            start = (std::clamp)(start + delta, std::int64_t{0}, std::int64_t{LIMIT_MS - m_DragDurationMs});
                        else if (m_DragKind == static_cast<int>(CompositionTimeline::BoxGesture::TRIM_START))
                        { start = (std::clamp)(start + delta, std::int64_t{0}, std::int64_t{m_DragStartMs} + m_DragDurationMs - 1); duration = m_DragStartMs + m_DragDurationMs - start; }
                        else duration = (std::clamp)(duration + delta, std::int64_t{1}, std::int64_t{LIMIT_MS - m_DragStartMs});
                        m_DragPreviewStartMs = static_cast<std::uint32_t>(start); m_DragPreviewDurationMs = static_cast<std::uint32_t>(duration);
                    }
                    ImGui::PopID(); ImGui::PopID();
                }
                y += height;
            }
            const float cursorX = origin.x + labels + float(m_ClockMs) * m_Zoom * .001f;
            draw->AddLine({cursorX, origin.y}, {cursorX, y}, IM_COL32(255,222,90,255), 2.f);
            draw->AddTriangleFilled({cursorX - 6.f, origin.y}, {cursorX + 6.f, origin.y}, {cursorX, origin.y + 8.f}, IM_COL32(255,222,90,255));
            ImGui::SetCursorScreenPos({origin.x, y}); ImGui::Dummy({width, 2.f});
        }
        ImGui::EndChild();
        if (!m_DragRowId.empty() && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            if (m_DragPreviewStartMs != m_DragStartMs || m_DragPreviewDurationMs != m_DragDurationMs)
            {
                switch (m_DragTrack)
                {
                case TRACK_KIND::ANIMATION:
                {
                    const auto* sequence = Selected_Sequence();
                    const auto* clips = m_CustomAnimation ? &m_AnimationRows : (sequence ? &sequence->clips : nullptr);
                    if (clips)
                    {
                        auto found = FindRow(*clips, m_DragRowId);
                        if (found != clips->end())
                        {
                            auto row = *found; bool valid = true;
                            if (m_DragKind == static_cast<int>(CompositionTimeline::BoxGesture::TRIM_START))
                            {
                                const auto sourceDelta = static_cast<std::int64_t>(std::llround(
                                    (std::int64_t(m_DragPreviewStartMs) - row.startMs) * double(row.playRate)));
                                const auto sourceStart = std::int64_t(row.sourceStartMs) + sourceDelta;
                                const auto sourcePlay = row.sourcePlayMs ? std::int64_t(row.sourcePlayMs) - sourceDelta : 0;
                                valid = sourceStart >= 0 && sourceStart <= LIMIT_MS &&
                                    (!row.sourcePlayMs || (sourcePlay > 0 && sourcePlay <= LIMIT_MS));
                                if (valid) { row.sourceStartMs = static_cast<std::uint32_t>(sourceStart); row.sourcePlayMs = static_cast<std::uint32_t>(sourcePlay); }
                            }
                            if (valid) { row.startMs = m_DragPreviewStartMs; row.durationMs = m_DragPreviewDurationMs; Apply_AnimationRow(row); }
                            else m_Status = "Animation trim extends outside its source window; the occurrence is preserved.";
                        }
                    }
                    break;
                }
                case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST:
                { auto found = FindRow(m_Effects, m_DragRowId); if (found != m_Effects.end()) { auto row = *found; row.startMs = m_DragPreviewStartMs; row.durationMs = m_DragPreviewDurationMs; Apply_EffectRow(row); } break; }
                case TRACK_KIND::SOUND:
                {
                    auto found = FindRow(m_Sounds, m_DragRowId);
                    if (found != m_Sounds.end())
                    {
                        auto row = *found; const auto sourceStart = std::int64_t(row.sourceStartMs) +
                            (m_DragKind == static_cast<int>(CompositionTimeline::BoxGesture::TRIM_START) ?
                                std::int64_t(m_DragPreviewStartMs) - row.startMs : 0);
                        if (sourceStart >= 0 && sourceStart <= LIMIT_MS)
                        {
                            row.sourceStartMs = static_cast<std::uint32_t>(sourceStart);
                            row.startMs = m_DragPreviewStartMs; row.durationMs = m_DragPreviewDurationMs; Apply_SoundRow(row);
                        }
                        else m_Status = "Sound trim extends before its source; the occurrence is preserved.";
                    }
                    break;
                }
                case TRACK_KIND::COLLIDER:
                { auto found = FindRow(m_Colliders, m_DragRowId); if (found != m_Colliders.end()) { auto row = *found; row.startMs = m_DragPreviewStartMs; row.durationMs = m_DragPreviewDurationMs; Apply_ColliderRow(row); } break; }
                case TRACK_KIND::CAMERA:
                {
                    auto candidate = m_CameraRows; auto found = FindRow(candidate, m_DragRowId);
                    if (found != candidate.end())
                    {
                        for (auto& key : found->cue.Keyframes) key.iTimeMs = static_cast<std::uint32_t>(
                            std::llround(double(key.iTimeMs) * m_DragPreviewDurationMs / found->cue.iDurationMs));
                        found->startMs = m_DragPreviewStartMs; found->cue.iDurationMs = m_DragPreviewDurationMs;
                        if (Validate_CameraRows(candidate))
                        { m_CameraRows = std::move(candidate); m_Dirty = true; if (m_Active) Sample(); }
                    }
                    break;
                }
                }
            }
            m_DragRowId.clear(); Pause(m_DragWasPaused);
        }
    }
    ImGui::End();
    if (m_ResourcesOpen) Render_CompositionResources();
    if (m_BoxDetailOpen) Render_BoxDetail();
    Render_Colliders();
}
}
```

</details>

#### EffectAuthoringSequencer_Resources.cpp

`Refresh_CompositionResourceInventory`는 명시적 갱신의 metadata 목록을 구성하고 `Render_CompositionResources`는 공통 resource tree의 선택을 typed Append 명령으로 전달한다. 실제 모델 clip, V1/V2 Effect, saved geometry Collider, Sound 상대 ID와 Camera capture를 기존 소비자에 연결한다. `BOX_DETAIL_DRAFT`는 적용 전 편집 상태이며 shared owner를 함수 안에서 유지해 Apply가 멤버를 reset해도 참조가 유효하다. `Render_BoxDetail`은 종류별 필드를 보여 주고 검증된 후보만 class의 Apply 함수로 전달한다.

<details>
<summary>EffectAuthoringSequencer_Resources.cpp 전체 코드</summary>

```cpp
#include "imgui.h"
#include "EffectAuthoringSequencer.h"
#include "EffectAuthoringResourceTree.h"
#include "EffectEditingSession.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>

namespace Client
{
namespace
{
constexpr std::uint32_t MAX_RESOURCE_MS = 600000u;
constexpr const char* RESOURCE_TABS[] = { "Animation", "Effect", "Collider", "Sound", "Camera", "Screen Post" };

std::string ResourcePathUtf8(const std::filesystem::path& path)
{
    const auto text = path.generic_u8string();
    return std::string(text.begin(), text.end());
}

bool ResourceMatches(const std::string& text, const std::string& query)
{
    return query.empty() || std::search(text.begin(), text.end(), query.begin(), query.end(),
        [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); }) != text.end();
}

std::vector<std::string> ResourceSegments(const std::string& category)
{
    std::vector<std::string> result;
    std::size_t start = 0;
    while (start < category.size())
    {
        const auto end = category.find('/', start);
        if (end != start) result.push_back(category.substr(start, end - start));
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return result;
}

template<class Rows, class Kind>
void ReadAnimationRows(Rows& rows, const Kind animationKind)
{
    const auto model = CAnimationTargetService::Resolve_Model();
    if (!model) return;
    const auto asset = CAnimationTargetService::Resolve_AssetName();
    std::set<std::string> names;
    for (std::uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
    {
        const auto* name = model->Get_AnimationName(i);
        if (!name || !*name) continue;
        typename Rows::value_type row; row.kind = animationKind; row.id = row.label = name;
        row.category = asset;
        float position = 0.f, duration = 0.f;
        const float rate = model->Get_AnimationTickPerSecond(i);
        if (!names.insert(row.id).second) row.status = "The model contains an ambiguous clip name.";
        if (!model->Get_AnimationProgress(i, position, duration) || !std::isfinite(duration) || duration <= 0.f ||
            !std::isfinite(rate) || rate <= 0.f || duration / rate > MAX_RESOURCE_MS * .001f)
            row.status = "The model clip has invalid duration metadata.";
        else row.durationMs = static_cast<std::uint32_t>(std::ceil(duration / rate * 1000.f));
        rows.push_back(std::move(row));
    }
}

bool ReadColliderDefinitions(std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE>& rows,
    std::string& error)
{
    const auto path = CKoukuSaydonCompositionDocument::Resolve_Path();
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec || size > 16u * 1024u * 1024u)
    { error = "Saved Collider composition is unavailable or exceeds 16 MiB."; return false; }
    std::ifstream input(path, std::ios::binary);
    if (!input) { error = "Cannot read saved Collider composition."; return false; }
    const std::string bytes((std::istreambuf_iterator<char>(input)), {});
    if (input.bad()) { error = "Saved Collider composition read failed."; return false; }
    KOUKU_SAYDON_COMPOSITION_DOCUMENT document;
    // Reuse the source codec without admitting the unrelated boss gameplay graph.
    if (!CKoukuSaydonCompositionDocument::Parse_Text(bytes, document, error)) return false;
    for (auto& resource : document.PresentationResources)
        if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
            rows.push_back(std::move(resource));
    return true;
}

bool DetailMs(const char* label, std::uint32_t& value, bool& commit, const int minimum = 0)
{
    int edit = static_cast<int>(value);
    const bool changed = ImGui::DragInt(label, &edit, 10.f, minimum, MAX_RESOURCE_MS, "%d ms");
    commit |= ImGui::IsItemDeactivatedAfterEdit();
    if (changed) value = static_cast<std::uint32_t>((std::clamp)(edit, minimum, int(MAX_RESOURCE_MS)));
    return changed;
}

bool DetailVector(const char* label, float3_t& value, bool& commit, const float minimum = 0.f,
    const float maximum = 0.f)
{
    const bool changed = ImGui::DragFloat3(label, &value.x, .01f, minimum, maximum);
    commit |= ImGui::IsItemDeactivatedAfterEdit();
    return changed;
}
}

struct CEffectAuthoringSequencer::BOX_DETAIL_DRAFT final
{
    TRACK_KIND kind = TRACK_KIND::EFFECT;
    std::string id;
    CLIP animation;
    EFFECT_ROW effect;
    SOUND_ROW sound;
    COLLIDER_ROW collider;
    bool dirty = false;
};

bool CEffectAuthoringSequencer::Refresh_CompositionResourceInventory()
{
    std::vector<RESOURCE_ENTRY> staged;
    std::string problems;
    bool complete = true;
    const auto preserve = [&](TRACK_KIND kind, const std::string& error)
    {
        complete = false;
        if (!problems.empty()) problems += "\n";
        problems += std::string(RESOURCE_TABS[static_cast<std::size_t>(kind)]) + ": " + error;
        for (const auto& previous : m_CompositionResources)
            if (previous.kind == kind) staged.push_back(previous);
    };
    ReadAnimationRows(staged, TRACK_KIND::ANIMATION);
    m_ResourceModelGeneration = CAnimationTargetService::Resolve_TargetGeneration();

    std::vector<CEffectAuthoringResourceTree::RESOURCE> authored;
    std::string error;
    if (CEffectAuthoringResourceTree::Read_V1Inventory(authored, error))
        for (const auto& source : authored)
        {
            RESOURCE_ENTRY row; row.kind = TRACK_KIND::EFFECT;
            row.key = {source.eKind, source.strAssetId}; row.id = "authored:" + source.strAssetId;
            row.label = source.strDisplayName.empty() ? source.strAssetId : source.strDisplayName;
            row.category = "Saved Effects"; row.status = source.strStatus;
            staged.push_back(std::move(row));
        }
    else
    {
        complete = false; problems += "Saved Effects: " + error;
        for (const auto& previous : m_CompositionResources)
            if (previous.kind == TRACK_KIND::EFFECT && previous.key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT)
                staged.push_back(previous);
    }
    std::vector<EFFECT_V2_RESOURCE_SUMMARY> typed;
    error.clear();
    if (CEffectV2Catalog::Get().Read_Inventory(typed, error))
        for (const auto& source : typed)
        {
            RESOURCE_ENTRY row;
            row.kind = source.eKind == EFFECT_V2_RESOURCE_KIND::LEAF && source.strCategory == "ScreenPost" ?
                TRACK_KIND::SCREEN_POST : TRACK_KIND::EFFECT;
            row.key = {source.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? EFFECT_RESOURCE_OWNER_KIND::V2_GROUP :
                EFFECT_RESOURCE_OWNER_KIND::V2_LEAF, source.strResourceId};
            row.id = std::string(source.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "group:" : "effect:") + source.strResourceId;
            row.label = source.strDisplayName.empty() ? source.strResourceId : source.strDisplayName;
            row.category = source.strCategory; row.status = source.strStatus; row.durationMs = source.iDurationMs;
            staged.push_back(std::move(row));
        }
    else
    {
        complete = false; if (!problems.empty()) problems += "\n"; problems += "Effect catalog: " + error;
        for (const auto& previous : m_CompositionResources)
            if ((previous.kind == TRACK_KIND::EFFECT || previous.kind == TRACK_KIND::SCREEN_POST) &&
                previous.key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT) staged.push_back(previous);
    }

    std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> colliders;
    error.clear();
    if (ReadColliderDefinitions(colliders, error))
        for (const auto& source : colliders)
        {
            RESOURCE_ENTRY row; row.kind = TRACK_KIND::COLLIDER; row.id = source.strResourceId;
            row.label = source.strDisplayName.empty() ? row.id : source.strDisplayName;
            row.category = source.strShape; row.collider = source; row.durationMs = source.iDurationMs;
            if (source.strColliderKind != "GEOMETRY") row.status = "This resource requires its Action Workbench gameplay Logic.";
            staged.push_back(std::move(row));
        }
    else preserve(TRACK_KIND::COLLIDER, error);

    std::vector<RESOURCE_ENTRY> sounds;
    const auto resourceRoot = CRuntimeAssetRoot::Get_ResourceRoot();
    const auto soundRoot = CRuntimeAssetRoot::Resolve("Sound");
    std::error_code ec;
    if (!soundRoot.empty())
    {
        std::filesystem::recursive_directory_iterator it(soundRoot,
            std::filesystem::directory_options::skip_permission_denied, ec), end;
        for (; !ec && it != end; it.increment(ec))
        {
            if (!it->is_regular_file(ec)) continue;
            auto extension = it->path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return char(std::tolower(c)); });
            if (extension != ".wav" && extension != ".ogg" && extension != ".mp3") continue;
            RESOURCE_ENTRY row; row.kind = TRACK_KIND::SOUND;
            row.id = ResourcePathUtf8(it->path().lexically_relative(resourceRoot));
            if (CRuntimeAssetRoot::Resolve(std::filesystem::path(std::u8string(row.id.begin(), row.id.end()))).empty()) continue;
            row.label = ResourcePathUtf8(it->path().filename());
            row.category = ResourcePathUtf8(it->path().parent_path().lexically_relative(soundRoot));
            if (row.category == ".") row.category.clear();
            sounds.push_back(std::move(row));
        }
    }
    if (soundRoot.empty() || ec) preserve(TRACK_KIND::SOUND, ec ? ec.message() : "Sound resource root is unavailable.");
    else staged.insert(staged.end(), std::make_move_iterator(sounds.begin()), std::make_move_iterator(sounds.end()));

    RESOURCE_ENTRY camera; camera.kind = TRACK_KIND::CAMERA; camera.id = "camera.capture.current";
    camera.label = "Current camera view"; camera.category = "Capture"; camera.durationMs = 1000u;
    staged.push_back(std::move(camera));
    std::stable_sort(staged.begin(), staged.end(), [](const auto& a, const auto& b)
    {
        if (a.kind != b.kind) return a.kind < b.kind;
        if (a.category != b.category) return a.category < b.category;
        if (a.label != b.label) return a.label < b.label;
        return a.id < b.id;
    });
    m_CompositionResources = std::move(staged); m_ResourcesLoaded = true;
    m_ResourceStatus = std::move(problems);
    Rebuild_CompositionResourceTrees();
    return complete;
}

void CEffectAuthoringSequencer::Rebuild_CompositionResourceTrees()
{
    for (std::size_t family = 0; family < m_ResourceTrees.size(); ++family)
    {
        m_ResourceTrees[family] = {};
        m_ResourceQueries[family] = m_ResourceSearch[family].data();
        for (std::size_t i = 0; i < m_CompositionResources.size(); ++i)
        {
            const auto& row = m_CompositionResources[i];
            if (static_cast<std::size_t>(row.kind) != family) continue;
            const auto& query = m_ResourceQueries[family];
            if (!ResourceMatches(row.label, query) && !ResourceMatches(row.id, query) && !ResourceMatches(row.category, query)) continue;
            InsertResourceTree(m_ResourceTrees[family], ResourceSegments(row.category), i);
        }
        FinalizeResourceTree(m_ResourceTrees[family]);
    }
}

void CEffectAuthoringSequencer::Render_CompositionResources()
{
    if (!m_ResourcesOpen) return;
    const std::string title = "Composition Resources###EffectCompositionResources." + std::to_string(reinterpret_cast<std::uintptr_t>(this));
    ImGui::SetNextWindowSize({470.f, 570.f}, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title.c_str(), &m_ResourcesOpen)) { ImGui::End(); return; }
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) m_Interaction = true;
    if (!m_ResourcesLoaded) Refresh_CompositionResourceInventory();
    if (ImGui::Button("Refresh Resources")) Refresh_CompositionResourceInventory();
    ImGui::SameLine(); ImGui::TextDisabled("Append at %u ms", ClockMs());
    if (m_ResourceModelGeneration != CAnimationTargetService::Resolve_TargetGeneration())
    {
        // A new rig only changes native clips; the saved catalogs keep their cache.
        std::erase_if(m_CompositionResources, [](const auto& row) { return row.kind == TRACK_KIND::ANIMATION; });
        ReadAnimationRows(m_CompositionResources, TRACK_KIND::ANIMATION);
        m_ResourceModelGeneration = CAnimationTargetService::Resolve_TargetGeneration();
        m_SelectedResourceIds[static_cast<std::size_t>(TRACK_KIND::ANIMATION)].clear();
        Rebuild_CompositionResourceTrees();
    }
    if (ImGui::BeginTabBar("CompositionResourceTabs"))
    {
        for (std::size_t family = 0; family < m_ResourceTrees.size(); ++family)
        {
            if (!ImGui::BeginTabItem(RESOURCE_TABS[family])) continue;
            m_ResourceTab = static_cast<int>(family);
            ImGui::SetNextItemWidth(-1.f);
            if (ImGui::InputTextWithHint("##ResourceSearch", "Search name, category or source...",
                m_ResourceSearch[family].data(), m_ResourceSearch[family].size())) Rebuild_CompositionResourceTrees();
            const float footer = ImGui::GetFrameHeightWithSpacing() * 3.f;
            if (ImGui::BeginChild("ResourceList", {0.f, (std::max)(120.f, ImGui::GetContentRegionAvail().y - footer)}, ImGuiChildFlags_Borders))
                RenderResourceTree(m_ResourceTrees[family], [&](std::size_t index)
                {
                    const auto& row = m_CompositionResources[index];
                    ImGui::PushID(row.id.c_str());
                    if (ImGui::Selectable(row.label.c_str(), m_SelectedResourceIds[family] == row.id)) m_SelectedResourceIds[family] = row.id;
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s%s%s", row.key.Is_Valid() ? row.key.strStableId.c_str() : row.id.c_str(),
                        row.status.empty() ? "" : "\n", row.status.c_str());
                    ImGui::PopID();
                });
            ImGui::EndChild();
            const auto selected = std::find_if(m_CompositionResources.begin(), m_CompositionResources.end(), [&](const auto& row)
                { return static_cast<std::size_t>(row.kind) == family && row.id == m_SelectedResourceIds[family]; });
            ImGui::BeginDisabled(selected == m_CompositionResources.end() || !selected->status.empty());
            if (ImGui::Button("Add / Append"))
            {
                bool added = false;
                switch (selected->kind)
                {
                case TRACK_KIND::ANIMATION: added = Append_Animation(selected->id); break;
                case TRACK_KIND::EFFECT:
                    added = Append(selected->key, selected->key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT ? 0u : selected->durationMs);
                    break;
                case TRACK_KIND::SCREEN_POST: added = Append(selected->key, selected->durationMs, true); break;
                case TRACK_KIND::SOUND: added = Append_Sound(selected->id, 0u); break;
                case TRACK_KIND::COLLIDER: added = Append_Collider(selected->collider); break;
                case TRACK_KIND::CAMERA:
                {
                    CAMERA_ROW row; row.id = CEffectEditingSession::New_Id("camera.row."); row.label = "Camera";
                    row.startMs = (std::min)(ClockMs(), MAX_RESOURCE_MS - 1000u); row.cue.iDurationMs = 1000u;
                    row.cue.strCueId = row.id; row.modelRelative = m_ModelRoot;
                    if (Capture_CameraKey(row, 0u))
                    {
                        auto staged = m_CameraRows;
                        for (auto preview : m_TransientCameraRows)
                        {
                            preview.id = CEffectEditingSession::New_Id("camera.row."); preview.cue.strCueId = preview.id;
                            staged.push_back(std::move(preview));
                        }
                        staged.push_back(row);
                        if (Validate_CameraRows(staged) && Commit_TransientPreview())
                        {
                            // Promotion owns its new camera identities. Append to that
                            // committed list rather than overwriting it with this draft.
                            m_CameraRows.push_back(row); m_Dirty = added = true;
                            Select_TimelineRow(TRACK_KIND::CAMERA, row.id);
                        }
                    }
                    break;
                }
                }
                if (added) m_BoxDetailOpen = true;
            }
            ImGui::EndDisabled();
            if (selected != m_CompositionResources.end() && !selected->status.empty()) ImGui::TextWrapped("%s", selected->status.c_str());
            if (family == 0 && !CAnimationTargetService::Resolve_Model()) ImGui::TextDisabled("Select a character or boss in Model View.");
            if (!m_ResourceStatus.empty()) { ImGui::TextDisabled("Some resources could not refresh."); if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", m_ResourceStatus.c_str()); }
            if (!m_Status.empty())
            {
                const auto end = m_Status.find_first_of("\r\n");
                ImGui::TextUnformatted(m_Status.data(), m_Status.data() + (end == std::string::npos ? m_Status.size() : end));
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", m_Status.c_str());
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void CEffectAuthoringSequencer::Render_BoxDetail()
{
    if (!m_BoxDetailOpen) return;
    const std::string title = "Box Detail###EffectCompositionDetail." + std::to_string(reinterpret_cast<std::uintptr_t>(this));
    ImGui::SetNextWindowSize({440.f, 470.f}, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title.c_str(), &m_BoxDetailOpen)) { ImGui::End(); return; }
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) m_Interaction = true;
    if (m_SelectedTrack == TRACK_KIND::CAMERA)
    {
        m_SelectedCamera = m_SelectedRowId;
        const auto previous = m_SelectedCamera;
        Render_CameraEditor();
        if (previous != m_SelectedCamera) Select_TimelineRow(TRACK_KIND::CAMERA, m_SelectedCamera);
        ImGui::End(); return;
    }
    if (m_SelectedRowId.empty()) { ImGui::TextDisabled("Select a timeline box."); ImGui::End(); return; }
    if (!m_BoxDetailDraft || m_BoxDetailDraft->id != m_SelectedRowId || m_BoxDetailDraft->kind != m_SelectedTrack || !m_BoxDetailDraft->dirty)
    {
        auto draft = std::make_shared<BOX_DETAIL_DRAFT>(); draft->kind = m_SelectedTrack; draft->id = m_SelectedRowId;
        bool found = false;
        const auto copy = [&](const auto& rows, auto& destination)
        {
            const auto row = std::find_if(rows.begin(), rows.end(), [&](const auto& value) { return value.id == draft->id; });
            if (row != rows.end()) { destination = *row; found = true; }
        };
        switch (draft->kind)
        {
        case TRACK_KIND::ANIMATION:
            if (m_CustomAnimation) copy(m_AnimationRows, draft->animation);
            else if (const auto* sequence = Selected_Sequence()) copy(sequence->clips, draft->animation);
            break;
        case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST: copy(m_Effects, draft->effect); break;
        case TRACK_KIND::SOUND: copy(m_Sounds, draft->sound); break;
        case TRACK_KIND::COLLIDER: copy(m_Colliders, draft->collider); break;
        case TRACK_KIND::CAMERA: break;
        }
        if (!found) { ImGui::TextDisabled("The selected box is no longer available."); ImGui::End(); return; }
        draft->effect.v1.reset(); draft->effect.v2 = 0; draft->effect.snapshot.reset();
        draft->effect.history.reset(); draft->effect.anchorHistory.reset();
        draft->sound.handle = 0; draft->sound.sampledAge = -1;
        m_BoxDetailDraft = std::move(draft);
    }
    const auto draftOwner = m_BoxDetailDraft;
    auto& draft = *draftOwner;
    ImGui::TextUnformatted(RESOURCE_TABS[static_cast<std::size_t>(draft.kind)]);
    ImGui::TextDisabled("%s", draft.id.c_str());
    bool changed = false, commit = false;
    ImGui::PushItemWidth(-1.f);
    switch (draft.kind)
    {
    case TRACK_KIND::ANIMATION:
    {
        auto& row = draft.animation; ImGui::TextWrapped("%s", row.clipName.c_str());
        changed |= DetailMs("Start", row.startMs, commit); changed |= DetailMs("Duration", row.durationMs, commit, 1);
        changed |= DetailMs("Source start", row.sourceStartMs, commit); changed |= DetailMs("Source play", row.sourcePlayMs, commit);
        changed |= ImGui::DragFloat("Playback rate", &row.playRate, .01f, .01f, 100.f); commit |= ImGui::IsItemDeactivatedAfterEdit();
        if (ImGui::Checkbox("Loop", &row.loop)) changed = commit = true;
        if (ImGui::Checkbox("Mute", &row.muted)) changed = commit = true;
        break;
    }
    case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST:
    {
        auto& row = draft.effect; ImGui::TextWrapped("%s", row.key.strStableId.c_str());
        changed |= DetailMs("Start", row.startMs, commit); changed |= DetailMs("Duration", row.durationMs, commit, 1);
        if (draft.kind == TRACK_KIND::EFFECT)
        {
            changed |= DetailVector("Offset (m)", row.offset, commit);
            if (Render_AnchorChoice("Bone / socket", row.anchorSlotId)) changed = commit = true;
        }
        if (ImGui::Checkbox("Mute", &row.muted)) changed = commit = true;
        break;
    }
    case TRACK_KIND::SOUND:
    {
        auto& row = draft.sound; ImGui::TextWrapped("%s", row.assetId.c_str());
        changed |= DetailMs("Start", row.startMs, commit); changed |= DetailMs("Duration", row.durationMs, commit, 1);
        changed |= DetailMs("Source start", row.sourceStartMs, commit);
        changed |= ImGui::SliderFloat("Volume", &row.volume, 0.f, 1.f); commit |= ImGui::IsItemDeactivatedAfterEdit();
        if (ImGui::Checkbox("Mute", &row.muted)) changed = commit = true;
        break;
    }
    case TRACK_KIND::COLLIDER:
    {
        auto& row = draft.collider; ImGui::TextWrapped("%s", row.label.c_str());
        ImGui::TextDisabled("%s", row.resource.strShape.c_str());
        changed |= DetailMs("Start", row.startMs, commit); changed |= DetailMs("Duration", row.durationMs, commit, 1);
        changed |= DetailVector("Offset (m)", row.offset, commit); changed |= DetailVector("Rotation (degrees)", row.rotation, commit);
        changed |= DetailVector("Scale", row.scale, commit, .001f, 10000.f);
        if (Render_AnchorChoice("Bone / socket", row.anchorSlotId)) changed = commit = true;
        if (ImGui::Checkbox("Debug Render", &row.debugRender)) changed = commit = true;
        if (ImGui::Checkbox("Mute", &row.muted)) changed = commit = true;
        break;
    }
    case TRACK_KIND::CAMERA: break;
    }
    ImGui::PopItemWidth();
    draft.dirty |= changed;
    if (ImGui::Button("Apply")) commit = true;
    ImGui::SameLine();
    if (ImGui::Button("Revert")) { m_BoxDetailDraft.reset(); ImGui::End(); return; }
    if (commit && draft.dirty)
    {
        bool applied = false;
        switch (draft.kind)
        {
        case TRACK_KIND::ANIMATION: applied = Apply_AnimationRow(draft.animation); break;
        case TRACK_KIND::EFFECT: case TRACK_KIND::SCREEN_POST: applied = Apply_EffectRow(draft.effect); break;
        case TRACK_KIND::SOUND: applied = Apply_SoundRow(draft.sound); break;
        case TRACK_KIND::COLLIDER: applied = Apply_ColliderRow(draft.collider); break;
        case TRACK_KIND::CAMERA: break;
        }
        if (applied) draft.dirty = false;
    }
    if (draft.dirty) ImGui::TextDisabled("Unapplied values");
    ImGui::BeginDisabled(draft.dirty);
    if (ImGui::Button("Duplicate")) { if (Duplicate_SelectedRow()) m_BoxDetailDraft.reset(); }
    ImGui::SameLine();
    if (ImGui::Button("Remove")) { if (Remove_SelectedRow()) m_BoxDetailDraft.reset(); }
    ImGui::EndDisabled();
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
    ImGui::End();
}
}
```

</details>

#### EffectAuthoringSequencer_Tracks.cpp

`Parse_AdditionalRows`는 v4의 Animation/Sound/Collider 행을 임시 배열에 파싱하고 종류별 validator를 호출한다. v1~3은 빈 추가 트랙으로 읽는다. `Write_AdditionalRows`는 stable ID와 source/wall 시간, 전체 Collider resource를 최대 유효 숫자 정밀도로 저장한다. `Commit_TransientPreview`는 현재 preview의 Effect와 Camera를 함께 검증해 영구 목록으로 승격한다. `Refresh_AnimationTiming`은 현재 clock을 보존하며 기존 모델 preview와 Effect history를 새 timing에 연결한다.

<details>
<summary>EffectAuthoringSequencer_Tracks.cpp 전체 코드</summary>

```cpp
#include "EffectAuthoringSequencer.h"
#include "DataJson.h"
#include "EffectEditingSession.h"
#include "Effect_Object.h"
#include <cmath>
#include <limits>
#include <ostream>

namespace Client
{
namespace
{
constexpr std::uint32_t TRACK_MAX_MS = 600000u;
constexpr std::size_t TRACK_MAX_ROWS = 256u;
bool Track_Text(const DATA_JSON_VALUE& object, const char* key, std::string& result)
{
    const auto* value = object.Find(key);
    if (!value || !value->Is_String()) return false;
    result = value->Get_String(); return true;
}
bool Track_Bool(const DATA_JSON_VALUE& object, const char* key, bool& result)
{
    const auto* value = object.Find(key);
    if (!value || !value->Is_Boolean()) return false;
    result = value->Get_Boolean(); return true;
}
bool Track_Number(const DATA_JSON_VALUE& object, const char* key, double& result)
{
    const auto* value = object.Find(key);
    if (!value || !value->Is_Number() || !std::isfinite(value->Get_Number())) return false;
    result = value->Get_Number(); return true;
}
bool Track_Ms(const DATA_JSON_VALUE& object, const char* key, std::uint32_t& result)
{
    double value = 0.0;
    if (!Track_Number(object, key, value) || value < 0.0 || value > TRACK_MAX_MS || std::floor(value) != value) return false;
    result = static_cast<std::uint32_t>(value); return true;
}
bool Track_Vector(const DATA_JSON_VALUE& object, const char* key, std::array<double, 3u>& result)
{
    const auto* value = object.Find(key);
    if (!value || !value->Is_Array() || value->Get_Array().size() != 3u) return false;
    for (std::size_t i = 0u; i < result.size(); ++i)
    {
        const auto& component = value->Get_Array()[i];
        if (!component.Is_Number() || !std::isfinite(component.Get_Number()) || std::abs(component.Get_Number()) > 100000.0) return false;
        result[i] = component.Get_Number();
    }
    return true;
}
bool Track_Vector(const DATA_JSON_VALUE& object, const char* key, float3_t& result)
{
    std::array<double, 3u> value{};
    if (!Track_Vector(object, key, value)) return false;
    result = {static_cast<float>(value[0]), static_cast<float>(value[1]), static_cast<float>(value[2])}; return true;
}
void Track_WriteVector(std::ostream& out, const float3_t& value)
{ out << '[' << value.x << ", " << value.y << ", " << value.z << ']'; }
}

bool CEffectAuthoringSequencer::Parse_AdditionalRows(const DATA_JSON_VALUE& document, const std::uint32_t version,
    std::vector<CLIP>& animations, bool& customAnimation,
    std::vector<SOUND_ROW>& sounds, std::vector<COLLIDER_ROW>& colliders)
{
    animations.clear(); sounds.clear(); colliders.clear(); customAnimation = false;
    if (version < 4u) return true;
    const auto* animationValues = document.Find("animationRows");
    const auto* soundValues = document.Find("soundRows");
    const auto* colliderValues = document.Find("colliderRows");
    if (!Track_Bool(document, "customAnimation", customAnimation) ||
        !animationValues || !animationValues->Is_Array() || animationValues->Get_Array().size() > TRACK_MAX_ROWS ||
        !soundValues || !soundValues->Is_Array() || soundValues->Get_Array().size() > TRACK_MAX_ROWS ||
        !colliderValues || !colliderValues->Is_Array() || colliderValues->Get_Array().size() > TRACK_MAX_ROWS ||
        (!customAnimation && !animationValues->Get_Array().empty()))
    { m_Status = "Invalid v4 track arrays or custom animation mode; current timeline preserved."; return false; }
    for (const auto& value : animationValues->Get_Array())
    {
        CLIP row; double rate = 0.0;
        if (!Track_Text(value, "occurrenceId", row.id) || !Track_Text(value, "displayName", row.label) ||
            !Track_Text(value, "memberId", row.memberId) || !Track_Text(value, "clipName", row.clipName) ||
            !Track_Ms(value, "startMs", row.startMs) || !Track_Ms(value, "durationMs", row.durationMs) ||
            !Track_Ms(value, "sourceStartMs", row.sourceStartMs) || !Track_Ms(value, "sourcePlayMs", row.sourcePlayMs) ||
            !Track_Number(value, "playRate", rate) || rate <= 0.0 || rate > 1000.0 ||
            !Track_Bool(value, "loop", row.loop) || !Track_Bool(value, "muted", row.muted))
        { m_Status = "Invalid Animation occurrence; current timeline preserved."; return false; }
        row.playRate = static_cast<float>(rate); animations.push_back(std::move(row));
    }
    for (const auto& value : soundValues->Get_Array())
    {
        SOUND_ROW row; double volume = 0.0;
        if (!Track_Text(value, "occurrenceId", row.id) || !Track_Text(value, "displayName", row.label) ||
            !Track_Text(value, "assetId", row.assetId) || !Track_Ms(value, "startMs", row.startMs) ||
            !Track_Ms(value, "durationMs", row.durationMs) || !Track_Ms(value, "sourceStartMs", row.sourceStartMs) ||
            !Track_Number(value, "volume", volume) || std::abs(volume) > 1000.0 || !Track_Bool(value, "muted", row.muted))
        { m_Status = "Invalid Sound occurrence; current timeline preserved."; return false; }
        row.volume = static_cast<float>(volume); sounds.push_back(std::move(row));
    }
    for (const auto& value : colliderValues->Get_Array())
    {
        COLLIDER_ROW row; std::string kind;
        const auto* resource = value.Find("resource");
        if (!Track_Text(value, "occurrenceId", row.id) || !Track_Text(value, "displayName", row.label) ||
            !Track_Ms(value, "startMs", row.startMs) || !Track_Ms(value, "durationMs", row.durationMs) ||
            !Track_Vector(value, "offset", row.offset) || !Track_Vector(value, "rotationDegrees", row.rotation) ||
            !Track_Vector(value, "scale", row.scale) || !Track_Text(value, "anchorSlotId", row.anchorSlotId) ||
            !Track_Bool(value, "muted", row.muted) || !Track_Bool(value, "debugRender", row.debugRender) ||
            !resource || !resource->Is_Object() || !Track_Text(*resource, "resourceId", row.resource.strResourceId) ||
            !Track_Text(*resource, "displayName", row.resource.strDisplayName) ||
            !Track_Text(*resource, "kind", kind) || kind != "COLLIDER" ||
            !Track_Text(*resource, "assetId", row.resource.strAssetId) ||
            !Track_Text(*resource, "resourceKind", row.resource.strResourceKind) ||
            !Track_Text(*resource, "defaultAnchorKind", row.resource.strDefaultAnchorKind) ||
            !Track_Ms(*resource, "durationMs", row.resource.iDurationMs) ||
            !Track_Text(*resource, "shape", row.resource.strShape) ||
            !Track_Text(*resource, "colliderKind", row.resource.strColliderKind) ||
            !Track_Vector(*resource, "halfExtents", row.resource.HalfExtents) ||
            !Track_Number(*resource, "radiusM", row.resource.fRadiusM) ||
            !Track_Number(*resource, "halfAngleDegrees", row.resource.fHalfAngleDegrees))
        { m_Status = "Invalid Collider occurrence or resource; current timeline preserved."; return false; }
        row.resource.eKind = KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER;
        colliders.push_back(std::move(row));
    }
    return Validate_AnimationRows(animations) && Validate_SoundRows(sounds) && Validate_ColliderRows(colliders);
}

void CEffectAuthoringSequencer::Write_AdditionalRows(std::ostream& out) const
{
    const auto precision = out.precision(std::numeric_limits<double>::max_digits10);
    const auto quote = [&](const std::string& value) { out << '"' << CDataJson::Escape(value) << '"'; };
    out << ",\n  \"customAnimation\": " << (m_CustomAnimation ? "true" : "false") << ",\n  \"animationRows\": [";
    for (std::size_t i = 0u; i < m_AnimationRows.size(); ++i)
    {
        const auto& row = m_AnimationRows[i];
        out << (i ? ",\n" : "\n") << "    {\"occurrenceId\": "; quote(row.id);
        out << ", \"displayName\": "; quote(row.label); out << ", \"memberId\": "; quote(row.memberId);
        out << ", \"clipName\": "; quote(row.clipName);
        out << ", \"startMs\": " << row.startMs << ", \"durationMs\": " << row.durationMs
            << ", \"sourceStartMs\": " << row.sourceStartMs << ", \"sourcePlayMs\": " << row.sourcePlayMs
            << ", \"playRate\": " << row.playRate << ", \"loop\": " << (row.loop ? "true" : "false")
            << ", \"muted\": " << (row.muted ? "true" : "false") << '}';
    }
    out << "\n  ],\n  \"soundRows\": [";
    for (std::size_t i = 0u; i < m_Sounds.size(); ++i)
    {
        const auto& row = m_Sounds[i];
        out << (i ? ",\n" : "\n") << "    {\"occurrenceId\": "; quote(row.id);
        out << ", \"displayName\": "; quote(row.label); out << ", \"assetId\": "; quote(row.assetId);
        out << ", \"startMs\": " << row.startMs << ", \"durationMs\": " << row.durationMs
            << ", \"sourceStartMs\": " << row.sourceStartMs << ", \"volume\": " << row.volume
            << ", \"muted\": " << (row.muted ? "true" : "false") << '}';
    }
    out << "\n  ],\n  \"colliderRows\": [";
    for (std::size_t i = 0u; i < m_Colliders.size(); ++i)
    {
        const auto& row = m_Colliders[i]; const auto& resource = row.resource;
        out << (i ? ",\n" : "\n") << "    {\"occurrenceId\": "; quote(row.id);
        out << ", \"displayName\": "; quote(row.label);
        out << ", \"startMs\": " << row.startMs << ", \"durationMs\": " << row.durationMs << ", \"offset\": "; Track_WriteVector(out, row.offset);
        out << ", \"rotationDegrees\": "; Track_WriteVector(out, row.rotation); out << ", \"scale\": "; Track_WriteVector(out, row.scale);
        out << ", \"anchorSlotId\": "; quote(row.anchorSlotId);
        out << ", \"muted\": " << (row.muted ? "true" : "false") << ", \"debugRender\": " << (row.debugRender ? "true" : "false");
        out << ", \"resource\": {\"resourceId\": "; quote(resource.strResourceId);
        out << ", \"displayName\": "; quote(resource.strDisplayName); out << ", \"kind\": \"COLLIDER\", \"assetId\": "; quote(resource.strAssetId);
        out << ", \"resourceKind\": "; quote(resource.strResourceKind); out << ", \"defaultAnchorKind\": "; quote(resource.strDefaultAnchorKind);
        out << ", \"durationMs\": " << resource.iDurationMs << ", \"shape\": "; quote(resource.strShape);
        out << ", \"colliderKind\": "; quote(resource.strColliderKind);
        out << ", \"halfExtents\": [" << resource.HalfExtents[0] << ", " << resource.HalfExtents[1] << ", " << resource.HalfExtents[2]
            << "], \"radiusM\": " << resource.fRadiusM << ", \"halfAngleDegrees\": " << resource.fHalfAngleDegrees << "}}";
    }
    out << "\n  ]";
    out.precision(precision);
}

bool CEffectAuthoringSequencer::Commit_TransientPreview()
{
    if (!m_Transient) return true;
    if (m_Effects.size() >= TRACK_MAX_ROWS)
    { m_Status = "The sequence already has 256 Effect rows."; return false; }
    auto cameras = m_CameraRows;
    for (auto row : m_TransientCameraRows)
    {
        row.id = CEffectEditingSession::New_Id("camera.row."); row.cue.strCueId = row.id;
        cameras.push_back(std::move(row));
    }
    if (!Validate_CameraRows(cameras)) return false;
    auto row = std::move(*m_Transient); row.id = CEffectEditingSession::New_Id("effect.occurrence.");
    Select_TimelineRow(row.screenPost ? TRACK_KIND::SCREEN_POST : TRACK_KIND::EFFECT, row.id);
    m_Effects.push_back(std::move(row)); m_CameraRows = std::move(cameras);
    m_Transient.reset(); m_TransientCameraRows.clear(); m_SelectedCamera.clear(); m_Dirty = true;
    m_Status = "Preview and camera rows appended. Save sequence preserves this arrangement.";
    return true;
}

bool CEffectAuthoringSequencer::Refresh_AnimationTiming()
{
    const auto clock = ClockMs(); const bool active = m_Active, paused = m_Paused;
    const auto reset = [](EFFECT_ROW& row)
    {
        row.history.reset(); row.anchorHistory.reset(); row.recordedAge = row.sampledAge = -1.f;
        if (row.v2) { CEffectV2Runtime::Stop_Group(row.v2); row.v2 = 0u; }
    };
    for (auto& row : m_Effects) reset(row);
    if (m_Transient) reset(*m_Transient);
    m_Dirty = true;
    if (!active) return true;
    // An Effect-only preview may not have acquired the model clock yet.
    if (!Begin_Model()) return false;
    if (!Seek(clock)) return false;
    Pause(paused); m_SkipNextPlaybackDelta = true;
    return true;
}
}
```

</details>

#### EffectAuthoringSequencer_Presentation.cpp

`Validate_SoundRows`는 안전한 Resources 상대 경로와 실제 음원 길이를, `Validate_ColliderRows`는 같은 Composition geometry 정의와 transform/time을 검사한다. `Append_Sound`는 선택 음원을 paused 채널로 먼저 준비하고 성공한 뒤 문서에 추가한다. `Append_Collider`는 현재 root/bone anchor를 검증한다. `Sample_Sounds`는 단일 sequence clock으로 occurrence별 FMOD handle을 시작·seek·pause하고 자연 종료 채널을 매 프레임 재생성하지 않는다. `Stop_Sounds`는 자신의 handle만 해제한다. `Render_Colliders`는 현재 활성 geometry를 기존 `CHitAreaWire`에 전달하며 Server damage를 만들지 않는다.

<details>
<summary>EffectAuthoringSequencer_Presentation.cpp 전체 코드</summary>

```cpp
#include "EffectAuthoringSequencer.h"
#include "GameInstance.h"
#include "HitAreaWire.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <set>

namespace Client
{
namespace
{
constexpr std::uint32_t PRESENTATION_MAX_MS = 600000u;
constexpr std::size_t PRESENTATION_MAX_ROWS = 256u;

bool Presentation_Text(const std::string& text, std::size_t maximum, bool empty = false)
{
    return (empty || !text.empty()) && text.size() <= maximum &&
        text.find('\0') == std::string::npos;
}

bool Presentation_Time(std::uint32_t start, std::uint32_t duration)
{
    return duration > 0u && duration <= PRESENTATION_MAX_MS &&
        start <= PRESENTATION_MAX_MS - duration;
}

bool Presentation_Vector(const float3_t& value, float minimum, float maximum)
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) &&
        value.x >= minimum && value.x <= maximum && value.y >= minimum && value.y <= maximum &&
        value.z >= minimum && value.z <= maximum;
}

std::filesystem::path Sound_Path(const std::string& assetId)
{
    if (!Presentation_Text(assetId, 1024u) || !assetId.starts_with("Sound/") ||
        assetId.find(':') != std::string::npos || assetId.find('\\') != std::string::npos)
        return {};
    const auto relative = std::filesystem::path(std::u8string(assetId.begin(), assetId.end()));
    if (relative.is_absolute() || relative.has_root_path() ||
        std::any_of(relative.begin(), relative.end(), [](const auto& part) { return part == ".."; }))
        return {};
    const auto dot = assetId.find_last_of('.');
    if (dot == std::string::npos) return {};
    auto extension = assetId.substr(dot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
    if (extension != ".wav" && extension != ".ogg" && extension != ".mp3") return {};
    const auto path = CRuntimeAssetRoot::Resolve(relative);
    std::error_code error;
    return !path.empty() && std::filesystem::is_regular_file(path, error) && !error ? path :
        std::filesystem::path{};
}

bool Collider_Resource(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource)
{
    if (resource.eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER ||
        resource.strColliderKind != "GEOMETRY" || !resource.strAssetId.empty() ||
        !CEffectV2Document::Is_ValidEffectId(resource.strResourceId) ||
        !Presentation_Text(resource.strDisplayName, 512u) ||
        !Presentation_Text(resource.strResourceKind, 128u, true) ||
        !Presentation_Text(resource.strDefaultAnchorKind, 128u, true) ||
        (resource.strShape != "BOX" && resource.strShape != "CIRCLE" && resource.strShape != "SECTOR") ||
        !Presentation_Time(0u, resource.iDurationMs) ||
        !std::isfinite(resource.fRadiusM) || resource.fRadiusM < .001 || resource.fRadiusM > 10000.0 ||
        !std::isfinite(resource.fHalfAngleDegrees) || resource.fHalfAngleDegrees < .001 ||
        resource.fHalfAngleDegrees > 180.0)
        return false;
    return std::all_of(resource.HalfExtents.begin(), resource.HalfExtents.end(), [](double value)
        { return std::isfinite(value) && value >= .001 && value <= 10000.0; });
}

// The composition player's geometry mapping is retained: meters become the
// existing hit-area wire's centimetres; BOX height stays a presentation volume.
HIT_AREA_SHAPE Collider_Wire(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource,
    const float3_t& scale)
{
    HIT_AREA_SHAPE shape;
    if (resource.strShape == "BOX")
    {
        const double halfWidth = resource.HalfExtents[0] * scale.x;
        const double halfLength = resource.HalfExtents[2] * scale.z;
        shape.fBoxHalfHeightM = static_cast<float>(resource.HalfExtents[1] * scale.y);
        shape.iAreaType = 2;
        shape.iAreaRange = static_cast<int32_t>((std::min)(halfLength * 200.0, 1000000000.0));
        shape.iAreaAngle = static_cast<int32_t>((std::min)(halfWidth * 200.0, 1000000000.0));
        shape.iAreaOffsetX = -shape.iAreaRange / 2;
    }
    else
    {
        shape.iAreaType = resource.strShape == "CIRCLE" ? 1 : 3;
        shape.iAreaRange = static_cast<int32_t>((std::min)(
            resource.fRadiusM * (std::max)(scale.x, scale.z) * 100.0, 1000000000.0));
        shape.iAreaAngle = static_cast<int32_t>(resource.fHalfAngleDegrees * 2.0);
    }
    return shape;
}
}

bool CEffectAuthoringSequencer::Validate_SoundRows(const std::vector<SOUND_ROW>& rows)
{
    if (rows.size() > PRESENTATION_MAX_ROWS)
    { m_Status = "A sequence supports at most 256 Sound occurrences."; return false; }
    std::set<std::string> ids;
    for (const auto& row : rows)
    {
        if (!CEffectV2Document::Is_ValidEffectId(row.id) || !ids.insert(row.id).second ||
            !Presentation_Text(row.label, 512u, true) || !Presentation_Time(row.startMs, row.durationMs) ||
            row.sourceStartMs > PRESENTATION_MAX_MS - row.durationMs ||
            !std::isfinite(row.volume) || row.volume < 0.f || row.volume > 4.f)
        { m_Status = "Invalid Sound identity, timing, source trim or volume: " + row.id; return false; }
        const auto path = Sound_Path(row.assetId);
        std::uint32_t sourceDuration = 0u;
        if (path.empty() || !CGameInstance::Get().Get_SoundDurationMs(path.wstring(), sourceDuration))
        { m_Status = "Sound asset is missing, unsafe or cannot be decoded: " + row.assetId; return false; }
        if (row.sourceStartMs >= sourceDuration)
        { m_Status = "Sound source trim starts at or beyond its natural end: " + row.id; return false; }
    }
    return true;
}

bool CEffectAuthoringSequencer::Validate_ColliderRows(const std::vector<COLLIDER_ROW>& rows)
{
    if (rows.size() > PRESENTATION_MAX_ROWS)
    { m_Status = "A sequence supports at most 256 Collider occurrences."; return false; }
    std::set<std::string> ids;
    for (const auto& row : rows)
        if (!CEffectV2Document::Is_ValidEffectId(row.id) || !ids.insert(row.id).second ||
            !Presentation_Text(row.label, 512u, true) || !Collider_Resource(row.resource) ||
            !Presentation_Time(row.startMs, row.durationMs) ||
            !Presentation_Vector(row.offset, -100000.f, 100000.f) ||
            !Presentation_Vector(row.rotation, -100000.f, 100000.f) ||
            !Presentation_Vector(row.scale, .001f, 10000.f) ||
            !Presentation_Text(row.anchorSlotId, 128u))
        { m_Status = "Invalid geometry Collider resource, transform, timing or anchor: " + row.id; return false; }
    // Load validates named anchors against its staged model selection. This
    // structural check must not accidentally use the previously selected model.
    return true;
}

bool CEffectAuthoringSequencer::Append_Sound(const std::string& assetId, std::uint32_t durationMs)
{
    SOUND_ROW row;
    std::uint32_t ordinal = 1u;
    do { row.id = "sound.occurrence." + std::to_string(ordinal++); }
    while (std::any_of(m_Sounds.begin(), m_Sounds.end(), [&](const auto& value) { return value.id == row.id; }));
    row.assetId = assetId;
    row.label = assetId.substr(assetId.find_last_of('/') + 1u);
    row.startMs = ClockMs();
    if (!durationMs)
    {
        const auto path = Sound_Path(assetId);
        if (path.empty() || !CGameInstance::Get().Get_SoundDurationMs(path.wstring(), durationMs))
        { m_Status = "Sound source duration is unavailable: " + assetId; return false; }
    }
    row.durationMs = durationMs;
    auto staged = m_Sounds; staged.push_back(row);
    if (!Validate_SoundRows(staged)) return false;
    auto& sound = CGameInstance::Get();
    if (m_Active)
    {
        const auto path = Sound_Path(assetId);
        if (path.empty())
        { m_Status = "Sound source preparation failed; the current timeline is preserved."; return false; }
        row.handle = sound.Play_SoundCue(path.wstring(), row.volume, row.sourceStartMs, true);
        if (!row.handle)
        { m_Status = "Sound channel preparation failed; the current timeline is preserved."; return false; }
        row.sampledAge = row.sourceStartMs;
    }
    if (!Commit_TransientPreview())
    {
        if (row.handle) sound.Stop_SoundCue(row.handle);
        return false;
    }
    m_Sounds.push_back(std::move(row));
    if (m_Sounds.back().handle) sound.Pause_SoundCue(m_Sounds.back().handle, m_Paused);
    Select_TimelineRow(TRACK_KIND::SOUND, m_Sounds.back().id);
    m_Dirty = true;
    Preserve_ClockDuringAuthoring();
    m_Status = "Appended Sound at " + std::to_string(ClockMs()) + " ms.";
    return true;
}

bool CEffectAuthoringSequencer::Append_Collider(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource)
{
    COLLIDER_ROW row;
    std::uint32_t ordinal = 1u;
    do { row.id = "collider.occurrence." + std::to_string(ordinal++); }
    while (std::any_of(m_Colliders.begin(), m_Colliders.end(), [&](const auto& value) { return value.id == row.id; }));
    row.resource = resource; row.label = resource.strDisplayName;
    row.startMs = ClockMs(); row.durationMs = resource.iDurationMs;
    row.anchorSlotId = m_DefaultAnchorSlotId;
    auto staged = m_Colliders; staged.push_back(row);
    if (!Validate_ColliderRows(staged) || !Validate_Anchor(row.anchorSlotId, m_ModelRoot, m_UseKouku)) return false;
    if (!Commit_TransientPreview()) return false;
    m_Colliders.push_back(std::move(row));
    Select_TimelineRow(TRACK_KIND::COLLIDER, m_Colliders.back().id);
    m_Dirty = true;
    Preserve_ClockDuringAuthoring();
    m_Status = "Appended Collider at " + std::to_string(ClockMs()) + " ms.";
    return true;
}

void CEffectAuthoringSequencer::Stop_Sounds()
{
    for (auto& row : m_Sounds)
    {
        if (row.handle) CGameInstance::Get().Stop_SoundCue(row.handle);
        row.handle = 0u; row.sampledAge = -1;
    }
}

bool CEffectAuthoringSequencer::Sample_Sounds(bool forceSeek)
{
    if (!m_Active || m_Transient) { Stop_Sounds(); return true; }
    const auto clock = ClockMs();
    auto& sound = CGameInstance::Get();
    for (auto& row : m_Sounds)
    {
        if (row.muted || clock < row.startMs || std::uint64_t(clock) >= std::uint64_t(row.startMs) + row.durationMs)
        {
            if (row.handle) sound.Stop_SoundCue(row.handle);
            row.handle = 0u; row.sampledAge = -1;
            continue;
        }
        const auto sourceAge = row.sourceStartMs + (clock - row.startMs);
        const bool reposition = forceSeek || (row.sampledAge >= 0 && sourceAge < row.sampledAge);
        if (row.sampledAge >= 0 && !reposition)
        {
            // A finished FMOD channel is deliberately not reborn each tick.
            // Only a new interval, Restart/Loop or explicit Seek can recreate it.
            if (row.handle) sound.Pause_SoundCue(row.handle, m_Paused);
            row.sampledAge = sourceAge;
            continue;
        }
        const auto path = Sound_Path(row.assetId);
        std::uint32_t sourceDuration = 0u;
        if (path.empty() || !sound.Get_SoundDurationMs(path.wstring(), sourceDuration))
        { m_Status = "Sound occurrence cannot resolve its source: " + row.id; return false; }
        if (sourceAge >= sourceDuration)
        {
            if (row.handle) sound.Stop_SoundCue(row.handle);
            row.handle = 0u; row.sampledAge = sourceAge;
            continue;
        }
        if (row.handle && sound.Is_SoundCueActive(row.handle))
        {
            sound.Pause_SoundCue(row.handle, true);
            sound.Seek_SoundCue(row.handle, sourceAge);
            sound.Pause_SoundCue(row.handle, m_Paused);
        }
        else
        {
            if (row.handle) sound.Stop_SoundCue(row.handle);
            row.handle = sound.Play_SoundCue(path.wstring(), row.volume, sourceAge, m_Paused);
            if (!row.handle)
            { m_Status = "Sound occurrence could not start: " + row.id; return false; }
        }
        row.sampledAge = sourceAge;
    }
    return true;
}

void CEffectAuthoringSequencer::Render_Colliders()
{
    if (!m_Active || m_Transient) return;
    float4x4_t root;
    if (!Resolve_Root(root)) return;
    const auto clock = ClockMs();
    for (const auto& row : m_Colliders)
    {
        if (row.muted || !row.debugRender || clock < row.startMs ||
            std::uint64_t(clock) >= std::uint64_t(row.startMs) + row.durationMs) continue;
        EFFECT_ROW anchorRow; anchorRow.anchorSlotId = row.anchorSlotId;
        float4x4_t anchor;
        if (!Resolve_RowPivot(anchorRow, root, anchor)) continue;
        matrix_t basis = XMLoadFloat4x4(&anchor);
        bool valid = !XMMatrixIsNaN(basis) && !XMMatrixIsInfinite(basis);
        for (std::size_t axis = 0; valid && axis < 3u; ++axis)
        {
            const auto length = XMVectorGetX(XMVector3LengthSq(basis.r[axis]));
            valid = std::isfinite(length) && length >= .000001f;
            if (valid) basis.r[axis] = XMVectorSetW(XMVector3Normalize(basis.r[axis]), 0.f);
        }
        if (!valid) { m_Status = "Collider anchor has a singular transform: " + row.id; continue; }
        // Scale is already baked into Collider_Wire. Keep the occurrence's
        // rotation/offset and sampled anchor without scaling dimensions twice.
        const matrix_t placed = XMMatrixRotationRollPitchYaw(XMConvertToRadians(row.rotation.x),
            XMConvertToRadians(row.rotation.y), XMConvertToRadians(row.rotation.z)) *
            XMMatrixTranslation(row.offset.x, row.offset.y, row.offset.z) * basis;
        if (row.resource.strShape != "BOX" &&
            XMVectorGetX(XMVector3LengthSq(XMVectorSetY(placed.r[2], 0.f))) < 1e-12f)
        { m_Status = "Collider footprint needs a nonvertical forward axis: " + row.id; continue; }
        float4x4_t world; XMStoreFloat4x4(&world, placed);
        // Preview geometry only; no collision object, damage or Server state is created.
        CHitAreaWire::Draw(world, Collider_Wire(row.resource, row.scale), 0xff40dfff);
    }
}
}
```

</details>


## G29. 선택 element의 Sequencer Solo (2026-09-10)

차원술사 Alt V full restore의 개별 element를 기다리지 않고 원래 등장 시점부터 검토한다. 기존 Solo와 선택 element용 Timeline Solo 버튼은 Sequencer에 임시 effect 행 하나를 만들고 해당 element의 원본 시작 시각으로 이동해 정지한다. 수백 개의 행을 펼치거나 저장 문서를 변경하지 않는다.

호출은 Effect Tool 선택/버튼 → Preview_Element → 기존 Stage_Row/Seek/Sample_Row → 기존 V1 occurrence factory → CEffectObject로 이어진다. 임시 행은 실제 문서 asset ID와 element stable ID를 따로 유지한다. factory는 현재 draft를 반영한 문서를 선택 element 하나와 필요한 비표시 model-cue anchor로 투영한다. 원본 start delay와 source clock을 보존하고 행 시작은 0으로 둬 지연이 이중 적용되지 않게 한다. 준비 또는 최초 샘플 실패는 기존 재생을 보존한다.

| 파일 | 변경 책임 |
|---|---|
| Client/Public/EffectAuthoringSequencer.h, Client/Private/EffectAuthoringSequencer.cpp | 임시 element scope, factory 인자, 원본 시각에서 정지하는 transaction, refresh/play 유지 |
| Client/Private/EffectAuthoringSequencer_Tracks.cpp | 선택 element 미리보기의 저장행 자동 승격 차단 |
| Client/Private/EffectAuthoringSequencer_Timeline.cpp, EffectAuthoringSequencer_Resources.cpp | 임시 element 한 행과 이름, 읽기 전용 행 정보, 저장행 표시 복귀 |
| Client/Public/Effect_Tool.h, Client/Private/Effect_Tool.cpp, Effect_Tool_Workspace.cpp | 기존 Solo 및 Timeline Solo 버튼 연결, draft/anchor를 유지하는 단일 element document projection |

Append는 전체 문서 저장행의 의미를 유지한다. element 미리보기를 전체 문서인 것처럼 저장하지 않으며 Stop 또는 전체 Preview로 빠져나온 뒤 Append한다. 전체 재생, 기존 저장 sequence, 제품 skill cue와 effect JSON은 변경하지 않는다. 기존 C++ 파일만 수정하므로 vcxproj/filter 등록은 필요 없다.

검증은 관련 기존 source-contract 검사, 변경 Client C++의 최소 컴파일과 scoped diff check로 한정한다. Client 실행 중에는 ClCompile까지만 진행하고 링크/배포는 실행 종료 확인 뒤 수행한다. 한 행 표시, 4초 등장 시점 이동, 스크럽/Play 및 전체 재생 복귀의 최종 화면 확인은 사용자가 수행한다.
