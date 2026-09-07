# 2026-09-07 World Object Tool 구현 계획서

> 현재 작업 상태: 사용자가 구현을 재개했다. 이번 반영은 Object 단일 목록, 카드/조커 Object Append와 개별 Map Transform 저장·Preview·Product 소비, Save 자동 적용에 한정한다. Boss Anchor·Dissolve·카드 Collider/Logic 자동 저작은 이번 범위가 아니며 사용자가 직접 연결한다. 다른 Codex 작업에 메시지를 보내지 않는다.

## G00. 목표와 현재 경로

사용자가 요청한 카드·조커카드·공·세토·칼날·갈고리·빙고폭탄·빙고·커튼·룰렛을
World Object Resources로 정리하고, 물리 모델과 texture 선택, 동작 상태 저작, 패턴 재사용을 연결한다.
기존 Effect resource 파일은 모델·texture의 입력으로 재사용한다.

현재 World는 Composition의 world definition/occurrence가 Area WorldSequence instance를 참조한다.
제품 재생은 Server의 sequence 시작 → ClientReplication → Level → CWorldSequencePlayer를 사용한다.
커튼은 기존 11개 Map placement, 룰렛은 placement 40에 연결되어 있다. 두 대상의 stable ID와 키는 보존한다.

새 도구는 기존 World Sequence source와 sampler를 확장한다. 카드의 들썩임·뒤집힘은 이름 있는
template/state로 저장하고 instance가 대상 object resource를 binding한다. 물리 파일을 상태마다 복제하지 않는다.

## G01. 문서와 데이터 정본

정본은 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`이다.
새 formatVersion 3은 `objectResources`, `OBJECT_RESOURCE` binding, template motion과 instance anchor를 추가한다.
기존 v1/v2 reader와 Map/Deploy bindings를 유지한다.

Object resource는 stable objectId, displayName, Resources-relative model/diffuse texture, model 단위 보정,
animated 여부와 기본 scale을 소유한다. 기존 커튼·룰렛은 원래 sequence instance의 alias로 목록에 등록한다.
표시 이름은 `월드오브젝트_카드`, `월드오브젝트_조커카드`, `월드오브젝트_공`, `월드오브젝트_세토`,
`월드오브젝트_칼날`, `월드오브젝트_갈고리`, `월드오브젝트_빙고폭탄`, `월드오브젝트_빙고`,
`월드오브젝트_커튼`, `월드오브젝트_룰렛`을 사용한다.

State는 기존 Transform key/animation track과 duration을 사용한다. motion은 속도·가속도·자전·공전,
생성 개수·간격·방향 분산·seed를 소유한다. 기본 생성 개수는 1이며 seed와 시각으로 샘플링해 seek와 재생을 맞춘다.
Instance는 WORLD/PLAYER anchor와 시작 위치를 소유한다. PLAYER transform은 ClientReplication의 읽기 전용 view다.

Load/Save/Publish는 기존 문서 검증과 원자적 저장을 확장한다. 외부 source 변경, 잘못된 경로·숫자·ID·binding은
기존 정상 문서를 보존한다. 물리 자산은 실제 패키지·material 근거로 확인하고 필요한 기존 cooker로 준비한다.
Resources binary는 Git에 추가하지 않는다.

## G02. World Object Tool

F1의 World 범주에서 `World Object Tool`을 연다. 화면 상단은 저장 Object Resources와 상태 목록,
가운데는 Object Detail·부품 slot·동작 timeline, 하단은 실제 Physical Resources 폴더 목록이다.
등록된 Effect 항목으로만 제한하지 않고 실제 상대 경로별 모델·texture를 표시한다.

타임라인은 `CompositionTimeline.h`의 ruler/box/gesture를 재사용한다. 키 이동·Transform·rotation·scale·visibility,
상태 duration, 속도·공전·복수 생성과 Play/Pause/Stop/Seek를 동일 source에 연결한다.
커튼·룰렛을 선택하면 원래 template의 tracks를 편집하므로 또 다른 애니메이션 사본을 만들지 않는다.

## G03. 실행과 패턴 연결

CWorldSequencePlayer가 OBJECT_RESOURCE binding을 기존 GameObject Prototype/Clone 경로로 생성한다.
새 CWorldSequenceObject는 CModel/CMaterial을 렌더링하는 표현 객체이며 시간·배치·동작 권한은 Player에 둔다.
생성/seek/종료/Level 퇴장과 실패 시 정리는 같은 Player가 수행한다. 모델 prototype은 공유하고 움직이는 clone만 둔다.

World Object Tool preview도 Level의 같은 Player 경로를 사용한다. 제품 sequence와 충돌하는 preview는 거부한다.
Action Workbench World 목록에 실제 sequence/state 목록을 공급하고 Append한다.
World box의 lifetime을 제품 호출과 preview 종료까지 연결한다. Effect는 기존 World anchor 조회 경계로 연결한다.
게임플레이 충돌·피해 수치를 object 렌더러에서 판정하지 않는다.

## G04. 파일과 등록

| 구분 | 경로 | 책임 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/WorldSequenceDocument.h` 및 Private CPP | v3 resource/motion/anchor, 검증·저장 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/WorldSequencePlayer.h` 및 Private CPP | dynamic object 생성·샘플·수명·pivot |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/WorldSequenceObject.h` 및 Private CPP | 기존 CModel의 물리 모델·texture 표현 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/WorldObjectTool.h` 및 Private CPP | Object/상태/slot/timeline/폴더 저작 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/PhysicalResourceCatalog.h` 및 Private CPP | Effect/World Object 공용 물리 폴더 검색 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/WorldSequencePlayer_Objects.cpp` | 기존 Player의 모델 준비·복수 생성·샘플·정리 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_KakulSaydonArena_WorldObjects.cpp` | Level의 World Object preview와 typed target view |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MainApp.h` 및 Private CPP | 도구 ownership과 F1 연결 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_KakulSaydonArena.h` 및 Private CPP | typed target view, preview·runtime reload |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp` 및 대응 H | 저장 Object/state 목록·Append |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/MapPipeline/Publish-MapAuthoring.ps1` 및 관련 validator | v3 publish |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | World box 수명·정본 join |

World box lifetime의 기존 Shared/Server/Client typed 전송도 같은 변경에서 연결한다.
`iDurationMs`를 WORLD cue에 추가하며 protocol 63을 사용한다. 0인 기존 직접 호출은 authored duration을 유지한다.
diffuse override는 DDS 로드 성공을 검사한 SRV를 기존 공용 CModel material 바인딩에 전달한다.
OBJECT_RESOURCE는 단일 상태에서 최대 128개, Player 전체에서 최대 1,024개로 제한하고 초과는 실패로 정리한다.
기존 Effect WORLD anchor는 살아 있는 단일 object pose를 소비하며 복수 입자별 effect 이벤트 저작은 이번 범위가 아니다.
새 H/CPP는 Client.vcxproj와 기존 물리 폴더에 대응하는 filters에 등록한다. 기존 sequence JSON의 None 등록을 재사용한다.
public 계약 변경은 CLAUDE와 팀 Animation/Area 사용서에 반영하고 검증 이력은 대응 RESULT에 기록한다.

## G05. 검증과 종료 기준

변경한 sequence/Composition publisher 검사, 정상 저장 왕복과 실패 보존, 실제 resource 경로 확인,
관련 기존 Python 검사, 필요한 native 검사와 Debug Product 빌드를 수행한다. JSON/XML parse와 diff check를 확인한다.
무관한 광역 진단을 기본 조건으로 추가하지 않는다.

사용자는 Server + Client profile의 Ctrl+F5로 시작하고 Lobby → KoukuSaydon → F1 → World Object Tool에서
목록·slot·키·저장·재로드와 아레나 재생을 확인한다. 에이전트는 Client/UI 실행·캡처와 최종 화면 판정을 하지 않는다.

## G06. 사용자 검증 후 독립 패널·앵커·Play 수정

2026-09-07 첨부 Action Workbench 화면을 기준으로 Object Resources는 왼쪽, Object Sequencer는 아래,
Object Detail은 오른쪽의 독립 ImGui 창으로 분리한다. 기존 SequencerTool의 첫 배치·Windows 메뉴·
Reset Window Layout 방식을 따르며 사용자가 창을 이동·크기 조절할 수 있게 한다. F1 Tools 목록에서 연다.

Resource에 optional `anchorKind`(WORLD/PLAYER)를 저장한다. UI는 Map/Character로 표시하고 Create와
Resources 분류가 같은 값을 사용한다. 필드가 없던 v3 리소스는 Map으로 읽고 기존 instance anchor는 보존한다.
리소스 anchor 변경은 연결된 상태에 적용하고 새 상태가 그 anchor를 사용한다. 배치 alias는 Map에 고정한다.
기존 문서 parser/Save/동등성 검사와 Map publisher에 같은 계약을 연결한다.

저장된 모델 상태가 최초 플레이어 spawn에 고정돼 현재 관문에서 멀리 생성되는 문제를 해결한다.
기본 저작 preview는 현재 캐릭터 앞쪽에서 재생하고 명시 선택으로 저장 Map 위치에서 재생한다.
이 preview offset은 저장 위치를 바꾸지 않으며 배치된 커튼·룰렛에는 적용하지 않는다.
생성 개수·위치와 모델/texture/anchor/render 실패 상태를 Object Sequencer에 표시한다.
새 clip 없는 animated 모델도 rest pose의 bone combined matrix를 준비한다.

Player Follow Camera의 위치·각도·FOV·응답값 편집은 기존 Level setter로 즉시 반영하고
Save만 영구 JSON 저장을 수행한다. 다른 맵이나 카메라 연출 중에는 해당 카메라를 덮어쓰지 않는다.
사용자가 이미 저장한 WorldSequence·RenderingProfiles dirty 원본을 보존한다.

## G07. 설치된 World Object 원본 모션 조사와 상태 저장

10종의 실제 WModel과 기존 배치 sequence를 조사하고, 원본 Action과 연결되는 카드 들썩임·뒤집힘을
기존 animationTracks로 저장한다. 빈 카드의 기존 card_hop/card_flip stable ID를 유지하되 합성 Transform을
제거하고 원본 bone clip을 적용한다. 조커 카드에는 별도 상태를 추가하고 사용자가 요청한 표시명을 사용한다.

세토·갈고리·빙고폭탄을 포함해 설치된 native clip은 선택 모델의 Animation Resources 목록으로 표시한다.
기존 CWModelDecoder::Read_AnimationCatalog를 사용하고 클립별 상태를 자동 생성하지 않는다.
왼쪽 Create Object에서 이름과 빈 패턴을 만들고, 아래 원본 모델·Animation 선택 → Append → Save로
필요한 모션만 저장한다. 요청된 카드 4개와 대표 이동·등장
모션은 미리 저장하며, 원본 clip 이름·재생률·길이를 보존하고 제자리 보행과 root bone 이동 돌진을 구분한다.
native root 이동에 임의 Velocity를 더하지 않는다. 클립이 없는 공·칼날·빙고는 기존 Motion/Transform 경로를
사용하며 커튼·룰렛의 배치 track도 유지한다. 새 물리 리소스나 별도 애니메이션 런타임은 만들지 않는다.

기존 World Object Tool과 Action Workbench의 상태 목록·World Append가 소비하는 authoring JSON을 수정하고
기존 Map publisher로 runtime 문서에 반영한다. 원본 리소스·기존 배치·무관한 상태의 동등성, 실제 clip 존재,
duration/track 계약과 저장·재로드를 확인한다. 원본 클립 목록과 Append의 WorldObjectTool H/CPP는
기존 프로젝트 등록을 사용한다. 최소 Product 컴파일 후 Client 실행과 실제 애니메이션 방향·크기
확인은 사용자가 수행한다.

## G08. 부모 Object·자식 Motion 및 판정 결과 재생

Object Resource는 모델·텍스처·기본 크기·Anchor를 소유한다. 기존 template/instance는 같은 Object를
참조하는 독립 저장 Motion으로 사용하며 새 중복 목록을 만들지 않는다. 부모 선택은 child ID를 비우고
Detail에 공통 설정과 연결 Motion 목록만 보인다. 자식을 고르면 같은 Detail과 Sequencer가 그 자식만
편집한다. Create Object는 부모만, Create Motion은 자식만 생성하고 Append Clip은 자식이 있을 때만
허용한다. 원본 clip 목록과 저장 Motion 목록의 이름과 역할을 구분한다.

자식 instance의 optional motionEnd는 STOP/HOLD/LOOP/NEXT이며 NEXT의 nextMotionId는 같은
Object를 참조하는 enabled 자식 ID여야 한다. 미지정 문서는 기존 STOP이다. NEXT cycle과 과도한 체인은
거절하고 Save/Reload/publisher에 같은 검증을 연결한다. 카드/조커의 기본 3-clip 연속 모션은 별도 자식에
복제 보존하고 기존 기본 ID는 idle LOOP, 들썩임은 NEXT idle, 뒤집기는 HOLD로 정리한다.

Result의 PLAY_WORLD_OBJECT_MOTION은 targetWorldInstanceId와 motionInstanceId만 저장한다.
Server가 판정을 확정하면 기존 WORLD sequence typed event에 대상 ID를 담고 Client Level이 현재
활성 대상의 motion을 교체한다. 같은 CWorldSequenceObject/CModel과 target placement를 유지한다.
누락·다른 Object의 모션·비활성 target은 기존 재생을 보존하며 거부하고 local 판정으로 우회하지 않는다.
고정 packet layout 변경은 protocol 65 및 Shared/Server/Client 재빌드를 같은 단위로 수행한다.

현재 Composition에는 뿅망치 전용 collider/window와 카드 World cue가 없다. 기존 player-vs-region
판정과 실제 hammer-vs-card 판정을 구분해서 실제 소비자가 있는 Result 경로에 연결한다. 새 판정의
저장 필드와 실제 overlap은 구현 전에 current Server geometry 계약으로 확인한다. Client physics가
gameplay 판정 권위를 갖지 않는다.

WorldSequencePlayer는 단일 elapsed clock으로 LOOP/HOLD/NEXT를 sample하고, 기존 target의 CModel을
재생성하지 않고 motion ID 및 상대 시각만 바꾼다. Tool Seek도 같은 함수로 샘플한다. 새 CPP 파일이나
프로젝트 등록은 제안하지 않는다. 기존 관련 publisher/protocol tests, 최소 Product 컴파일, JSON parse,
diff check 및 Server 시작을 확인하고 실제 화면은 사용자에게 남긴다.

## G09. 전체 World Catalog와 저장 후 즉시 목록 갱신

Composition Resources의 World Catalog는 Object Tool의 저장된 전체 Object/Motion inventory를 표시한다. 기존 Composition World alias 편집을 유지하고 Append 시에만 stable instance ID에 대응하는 World definition/occurrence를 생성한다. 비활성 Motion과 자식 없는 부모도 목록에 표시하되 배치·재생은 막고 이유를 표시한다.

MainApp은 저장 성공 generation을 소비해 다음 frame에 목록을 갱신한다. Save와 Publish 실패는 이전 저장 snapshot을 유지한다. PhysicalResourceCatalog의 같은 scanner를 시간 예산으로 나눠 진행하여 큰 물리 폴더 검색이 Save/Publish 위젯 표시를 막지 않게 한다. 별도 worker나 runtime catalog 정본을 만들지 않는다. 기존 파일의 인코딩·등록을 보존하고 필요한 Client compile/link 및 저장 정본 보존을 확인한다. 실제 버튼 표시 시간과 Append 배치는 사용자 확인이다.

## G10. Object를 Append하고 연결 상태를 전환하는 단일 목록

### 최신 사용자 결정과 목표

Composition Resource는 **Object 자체를 선택하여 Append**한다. Idle/들썩임/뒤집힘처럼 같은 Object의 상태를 별개의 배치 대상으로 펼치지 않는다. 앞서 제안했던 동작 Element 선택 후 Append는 이 Object 수명/상태 계약으로 대체한다. `World Catalog`, `Saved World Object states`, `Composition World aliases`를 별도 목록과 선택 단계로 노출하지 않는다.

Object Tool은 모델·기본 크기·Anchor와 그 Object에 속한 animation 상태를 만드는 정본이다. 사용자가 Object Tool에서 상태별 clip 또는 clip chain을 저장하고 **기본 상태**를 지정한다. Composition Resource에는 동일 Object 목록이 표시되고, Append된 Box Detail에서 그 Object에 연결된 상태와 animation clip을 확인한다. 실제 clip 편집은 Object Tool로 연결하며 두 도구가 같은 animation 정의의 복사본을 각각 저장하지 않는다.

조커찾기의 Object 정의와 배치는 다음과 같다.

| 정의 | 저장할 상태 | Pattern에 생성할 instance |
|---|---|---|
| 월드오브젝트_빈카드 | idle(기본), 들썩임, 뒤집힘 | 6개 |
| 월드오브젝트_조커카드 | idle(기본), 들썩임, 뒤집힘 | 1개 |

이는 Object 정의 2종과 독립 카드 배치 7개다. animation 상태마다 새 카드나 WORLD 박스를 만드는 구조가 아니다. 각 카드는 생성부터 제거까지 stable occurrence ID와 Transform을 유지하고 current state/animation만 교체한다.

Object 분류는 **Map**과 **Anchor**다. Anchor에는 **Character**와 **Boss**가 있다. 분류는 리소스 파일 폴더와 무관하며, 재생 중 배치가 무엇을 기준으로 유지되는지를 뜻한다.

| 분류 | Pattern 재생 중 기준 | 박스가 저장할 Transform |
|---|---|---|
| Map | 월드에 고정 | 독립 절대 위치·회전·크기 |
| Anchor / Character | 해당 Character | Character 기준 상대 위치·회전·크기 |
| Anchor / Boss | 해당 Pattern/Bundle member의 Boss | Boss 기준 상대 위치·회전·크기 |

카드는 **Map**을 사용한다. 플레이어는 Preview 및 Append의 초기 위치를 정하는 참고 대상일 뿐, 생성 후 카드를 끌고 다니는 Anchor가 아니다. 현재 Character 적용 범위는 기존 계약처럼 살아 있는 복제 Character들이며, 단독 Resource Preview에서는 로컬 플레이어 하나를 사용한다. Boss 실행은 Pattern의 targetBossPlacementId/해당 run member가 소유한다. 리소스에 GameObject 포인터나 현재 Boss 위치를 저장하지 않는다.

### 현재 실제 코드와 저장본

- Object Tool은 현재 `WORLD/PLAYER`를 Map/Character로 보여 준다. Boss Anchor는 아직 없다.
- 부모 `WORLD_SEQUENCE_OBJECT_RESOURCE`는 모델·텍스처·기본 크기·Anchor를 소유하고, 자식 `WORLD_SEQUENCE_TEMPLATE/INSTANCE`가 상태별 재생을 소유한다. 일반 Object에는 명시적 default state 필드가 아직 없다.
- 현재 카드/조커카드는 각각 기본 상태 `LOOP`, 들썩임 `NEXT → 기본 상태`, 뒤집힘 `HOLD`로 저장되어 있다. 기존 state ID/clip을 새로 복제할 필요는 없다.
- `Apply_ObjectMotion`은 이미 생성된 같은 WorldSequenceObject/CModel의 motion을 교체하는 소비자다. 다만 Object를 Append하여 default state로 시작하는 UI/저장 계약과 상태 종료 후 dissolve 수명 연결은 이번 계획에서 구현해야 한다.
- Composition revision116의 월드_룰렛은 `anchorKind=BOSS_SPAWN`, source anchor `[-0.319, 1.9, 737.531]`, offset `[0, .58, 0]`이다. 코드가 이 저장값으로 `boss spawn - source anchor + offset`을 계산한다. 룰렛이라는 이름만 보고 위치를 정하는 하드코딩도, 움직이는 Boss를 계속 따라가는 live Anchor도 아니다.
- 커튼과 룰렛은 현재 Object Resource가 기존 맵 sequence를 가리키는 항목이다. 커튼 11개 부품과 룰렛 지면/판정은 그룹 상대 배치를 보존해야 한다.

### 소유 데이터와 변경 위치

Object 정의는 stable object ID, 표시 이름, 모델, Map/Anchor 종류, **기본 상태 ID**, **연결 상태 목록**을 소유한다. 각 상태는 stable state ID, 기존 saved motion/clip chain 참조, 재생률·종료 후 상태 및 선택적인 종료 연출을 소유한다. 숫자/클립/기본 상태를 Composition Box에 복제하지 않는다. 기존 motion/sequence ID를 내부 참조로 재사용한다.

`WorldObjectTool.h/cpp`의 `Render_Resources/Render_ObjectDetail/Render_Detail/StateIds`와 `WorldSequenceDocument.h/cpp`가 Object 및 상태 저작을 맡는다. Workbench의 `Render_WorldResources`는 저장된 **Object 목록 하나**, `Render_WorldBoxDetails`는 해당 Object의 연결 상태/clip 조회와 독립 배치 편집을 맡는다. `MainApp::RefreshWorldObjectResources`는 동일 저장 snapshot/generation을 전달한다. 두 번째 Catalog 파일을 추가하지 않는다.

기본 상태가 없거나 연결 clip이 잘못된 Object는 목록에 남기고 해당 행에 오류를 표시한다. 실행할 수 없는 Object의 Append/Preview만 막으며 정상 Object 목록을 비우지 않는다. Object Tool의 미저장 편집과 저장 snapshot은 구분하고 Save 성공 시 Workbench 목록을 다음 frame에 갱신한다.

**종료 증거:** 두 도구의 Object ID·이름·분류가 같고, 카드 Object 한 번 Append로 한 박스만 생긴다. Box Detail에서 idle/들썩임/뒤집힘 clip 연결과 default state가 보인다. 상태를 바꿔도 새 카드/박스가 생기지 않는다.

## G11. Object Preview와 Map 초기 배치

현재 Preview는 `Object Tool → Level → CWorldSequencePlayer`에서 같은 clip/Transform/physics sampler를 사용하며 저장 문서를 바꾸지 않는다. 앞으로 Object 자체를 Preview하면 **플레이어 기준의 임시 위치에 Object를 한 번 생성하고 default state를 재생**한다. Object Tool과 Box Detail에서 연결 상태를 골라 Preview 중인 같은 Object의 들썩임/뒤집힘도 시험할 수 있다. 상태 시험은 authoring/debug 입력이며 Server gameplay 접촉 성공으로 기록하지 않는다.

Map Object는 Preview 시작 순간 플레이어 근처의 임시 월드 프레임을 사용한다. Anchor Character/Boss도 단독 Resource Preview에서는 플레이어를 임시 anchor로 사용한다. 이 임시 anchor는 저장되지 않는다. Pattern Preview는 그 Pattern에 저장된 각 카드의 실제 월드 위치와 전체 시계를 사용한다.

`Try_Get_AuthoringPreviewPlacement`의 복제 로컬 Character 위치를 공통 초기 위치 공급자로 재사용한다. 플레이어가 없거나 좌표가 유효하지 않으면 기존 미리보기를 보존하고 실패 이유를 표시한다. 카메라 위치나 원점으로 대체하지 않는다.

커튼·룰렛처럼 MAP/DEPLOY 부품 그룹인 Object는 공통 기준점을 player 근처로 옮기되 부품의 상대 Transform을 유지한다. Stop, 다른 Object 선택, 창 종료, Level 전환에서 기존 위치·회전·크기·visibility를 복원한다. 실제 지면/Server 판정 자료를 Preview 위치로 저장하지 않는다.

수정 대상은 WorldObjectTool, MainApp의 Resource Preview 라우팅, Level_KakulSaydonArena_WorldObjects, WorldSequencePlayer, KoukuSaydonPresentationPlayer다. 새 Preview 전용 모델 런타임을 만들지 않는다.

**종료 증거:** 카드 default idle과 상태 전환을 두 도구에서 같은 플레이어 위치로 확인하고, 커튼·룰렛 group Preview 종료 시 원래 배치를 복원한다. 실제 보이는 크기·방향·연출은 사용자 확인이다.

## G12. 카드 7개의 생성·상태·종료를 한 수명으로 연결

### Append와 Box Detail

Composition에서 빈카드 Object를 6번, 조커카드 Object를 1번 Append한다. 각각 새 stable occurrence ID와 플레이어 기준 초기 위치를 가진다. 카드들은 Map이며, Box Detail에서 서로 다른 월드 Transform을 지정한다. Start를 0으로 맞추면 Pattern 시작에 생성된다. 이후 플레이어 이동과 무관하게 저장된 월드 위치를 유지한다.

Box Detail에는 Object 이름/ID, Map/Anchor, Transform, Start/Lifetime, default state, 연결 상태와 그 clip, 상태 종료 정책이 보인다. animation 연결을 수정할 때는 Object Tool의 같은 Object를 연다. Pattern box는 object ID와 자신의 배치/수명을 저장하며 animation 정의를 복제하지 않는다. 기존 독립 모션 Append 문서는 호환 reader에서 Object와 초기 상태로 해석하여 이전 재생 의미와 occurrence ID를 보존한다.

### 서버 Trigger와 상태 전환

전체 조커찾기 Duration은 제한시간·성공·Timeout과 카드 7개의 최대 수명을 소유한다. Collider 접촉은 그 기간 안의 짧은 타격 Trigger 창에서 Server가 평가한다. Duration 성공을 카드 animation 한 번 재생했다는 이유만으로 확정하지 않는다.

| 현재 카드 상태/입력 | 같은 카드에 적용할 결과 |
|---|---|
| 생성 | default idle 반복 |
| idle에서 가장자리 타격 | 들썩임 → 끝나면 idle |
| idle 또는 들썩임에서 중앙 타격 | 뒤집힘 |
| 뒤집힘 완료, dissolve 없음 | 뒤집힌 상태 유지 |
| 뒤집힘 완료, dissolve 있음 | 마지막 자세 유지하며 dissolve → 제거 |
| Duration 종료/Stop | 남은 카드와 연출을 owner 단위로 정리 |

중앙/가장자리 겹침은 기존 Contact Group/Priority로 중앙을 우선한다. 서버 결과는 **맞은 카드 occurrence + 그 Object에 속한 상태 ID**를 전달한다. generic 상태 역할을 공유하는 경우 빈카드/조커카드 정의가 각각 자신의 clip을 resolve하며, 다른 Object의 raw clip을 재생하지 않는다. Object에 없는 상태는 저장/실행 오류로 표시한다.

뒤집히거나 소멸 중인 카드는 추가 들썩임/뒤집힘 타격으로 되돌리지 않는다. 카드의 상호작용 완료 상태는 Server의 현재 run/occurrence가 소유하고 다음 Pattern/run에 섞이지 않는다. 들썩임의 종료 후 idle 복귀는 같은 Object의 저장된 next-state 계약을 사용한다. 중앙 타격은 들썩임 재생 중에도 뒤집힘으로 전환할 수 있다.

조커 카드의 지정된 중앙 접촉이 검색 성공 조건이면 해당 Result가 전체 Duration의 EXTERNAL_SIGNAL에 성공을 전한다. 빈카드가 뒤집힌 것은 그 카드의 상태 변경이며 전체 검색 성공으로 취급하지 않는다. 마지막 tick 접촉/Timeout 우선순위와 기존 성공 Pattern/전멸 Result 경로를 유지한다.

### Dissolve와 생성/제거 시각

Dissolve는 선택적인 **카드 상태 종료 연출**이다. 별도 카드 Object를 Append하지 않으며, 이미 뒤집힌 카드의 마지막 자세에서 진행한다. 길이와 종료 후 제거 정책은 Object Tool의 해당 상태에 저장한다. 모델용 WorldObject의 현재 clip 전환만으로 dissolve까지 지원한다고 가정하지 않는다. 기존 CModel/CMaterial 렌더 경로의 instance 단위 파라미터를 연결하고, WorldSequencePlayer가 해당 카드만 정리해야 한다.

서버는 뒤집힘/소멸 중 카드의 접촉을 이미 종료하고, Client는 복제된 상태 시작 tick과 정의의 clip/dissolve 길이로 표현한다. 투명해졌는지를 Client에서 보고 성공/제거 권위를 판단하지 않는다. 전체 Duration 종료·성공 분기·Stop은 남은 모든 카드의 owner 정리를 수행한다.

### 실제 소비자와 호환

기존 `CWorldSequencePlayer::Apply_ObjectMotion`이 같은 target Object의 상태 재생을 담당한다. Object default-state/occurrence 수명 계약을 Source → projector/publisher → Server catalog/LogicRuntime/GameRoom → Shared cue → Client Level/WorldSequencePlayer에 연결한다. CONTACT의 원은 실제 저장된 각 카드의 월드 중심을 사용하고 WORLD Collider는 정확한 occurrence를 선택한다. Client의 위치를 Server의 판정 정답으로 보내지 않는다.

기존 `worldId → sequenceInstanceId`, definition offset/BOSS_SPAWN, state 단독 WORLD 박스는 호환 reader로 해석한다. 기존 파일을 열자마자 다시 저장하지 않으며, 명시 Save migration에서 Object/initial-state/배치/시각의 의미와 stable occurrence ID를 보존한다. 별도 alias UI는 제거하되 데이터 참조를 먼저 삭제하지 않는다.

커튼·룰렛도 같은 Object 선택/Append 흐름을 사용한다. 기존 맵 부품 그룹을 여러 occurrence가 동시에 덮어쓰는 방법으로 독립 배치를 흉내 내지 않는다. 필요한 clone은 기존 CModel/Map/WorldSequence 생성 경로를 재사용하고 룰렛의 지면/판정도 같은 occurrence transform을 소비해야 한다. 이 연결 전에는 해당 항목의 독립 Append가 완료됐다고 처리하지 않는다.

**종료 증거:** Object 정의 2종에서 카드 7개 생성, idle 시작, 서로 다른 위치 저장·재로드, 가장자리 한 카드만 들썩임→idle, 중앙 한 카드만 뒤집힘, optional dissolve/제거, 조커 접촉만 검색 성공, Timeout·Stop 정리까지 기존 focused 검사와 사용자 아레나 확인으로 검증한다.

## G13. Save 하나로 저장·목록 갱신·실행 자료 생성

현재 Save는 authoring JSON의 검증·stage·readback·CAS·원자 교체를 하고, Publish는 저장본을 runtime 자료로 생성한 뒤 다음 재생용 캐시를 갱신한다. 기능상 필요한 검증/변환은 유지하되 **사용자 버튼은 Save 하나로 통합**한다. `Publish Area`, `Publish Product`를 별도 작업 단계로 요구하지 않는다.

Save 흐름은 다음과 같다.

1. 편집 문서의 ID/path/숫자/binding을 검증하고 기존 원자 저장을 수행한다. 잘못된 입력과 외부 저장 충돌이면 draft와 기존 저장본을 보존한다.
2. 저장 generation을 올려 Object Tool과 Workbench의 공통 목록을 즉시 갱신한다. Resource Preview는 이 저장본의 Object/default state를 바로 사용할 수 있다.
3. 기존 domain publisher를 뒤에서 실행하여 변경한 WorldSequence 및 이를 소비하는 실행 자료를 생성한다. UI는 파일 스캔이나 외부 프로세스 종료를 동기 대기하지 않는다.
4. 생성·검증 성공 후 다음 Preview/재생용 문서를 stage하여 교체한다. 진행 중인 재생은 자신이 시작한 revision을 유지하고 다음 재생부터 새 revision을 사용한다.
5. 결과는 `저장 중`, `저장됨 · 반영 중`, `저장·반영 완료`, `저장됨 · 반영 실패`로 표시한다. 반영 실패를 성공으로 숨기거나 저장된 정상 편집본을 버리지 않는다. 같은 Save 버튼으로 실패한 반영을 다시 시도한다.

Object Tool Save는 무관한 맵 조명/카메라/배치 전체를 매번 다시 게시하지 않는다. 현재 `Publish-MapAuthoring.ps1`은 Area 전체를 다루므로 같은 publisher 안에 WorldSequence용 제한 scope를 추가한다. 별도 World runtime writer를 만들지 않는다. Object/상태 변경이 실제 CONTACT/지면/Pattern 생성 자료에 영향을 주면 해당 domain의 기존 검사·생성도 연결하며 무관한 전체 빌드/광역 진단은 Save 선행조건으로 두지 않는다.

Action Workbench의 Save도 Pattern 배치 저장과 적용 가능한 Product 생성까지 같은 사용자 동작으로 연결한다. 미완성 DRAFT는 저장 가능해야 하며, 실행할 수 없는 이유는 해당 Pattern에 표시한다. 목록 표시와 편집을 전체 Product 검증 실패에 종속시키지 않는다.

**현재 서버 적용 경계:** 서버는 시작/진입한 gameplay revision을 pin한다. 파일을 저장했다는 사실만으로 현재 실행 중인 전투의 데이터가 바뀌지는 않는다. 이번 계획의 즉시성은 Save 후 목록·Resource/Pattern Preview와 실행 자료 생성까지다. 서버가 새 revision을 아직 받지 않은 경우 실행 UI에 재시작 필요 상태를 표시하며 `현재 서버 반영 완료`라고 쓰지 않는다. 재시작 없는 Server 적용은 revision stage·room tick commit·진행 중 run 유지·Client 동기화가 모두 필요한 별도 변경으로 분리한다. Publish 버튼을 숨기는 것으로 hot reload를 구현했다고 처리하지 않는다.

수정 대상: `CWorldObjectTool::Save_Source/Start_Publish/Poll_Publish/Render_Toolbar`, `CKoukuSaydonActionWorkbench::Save/Publish_Product/Poll_PublishProcess`, 기존 Map/Kouku/Gameplay publisher와 MainApp 저장 generation 소비 경계다.

**종료 증거:** Save 한 번으로 다음 frame 목록 갱신, 게시 버튼 없이 다음 Preview에서 수정한 Object 상태 재생, 생성 실패/외부 충돌 시 정상 저장·runtime 보존, 불필요한 Area 파일 미변경을 확인한다.

## G14. 구현 순서와 파일 경계

| 순서 | 실제 변경 단위 | 주된 파일 |
|---|---|---|
| 1 | Object 단일 목록·default/연결 상태와 Map/Anchor/Character/Boss 분류 | WorldObjectTool.h/cpp, WorldSequenceDocument.h/cpp, 기존 Composition resource tree, KoukuSaydonActionWorkbench.h/cpp, MainApp.cpp |
| 2 | 동일 Resource Preview와 Map group 복원 | WorldObjectTool.cpp, MainApp.cpp, Level_KakulSaydonArena.h/cpp/_WorldObjects.cpp, WorldSequencePlayer.h/cpp/_Objects.cpp |
| 3 | occurrence별 Append/Transform와 기존 문서 호환 | KoukuSaydonCompositionDocument.h/cpp, Workbench, PresentationPlayer, 기존 native editor tests |
| 4 | 저장한 배치·Anchor·접촉의 실제 Product 소비 | projector/publisher, GameplayCatalog, LogicRuntime/Brain/GameRoom, Shared PacketMessages/PacketType, Client Level/replication |
| 5 | Save 하나로 자료 적용·상태 표시 | Object Tool/Workbench Save coordinator, 기존 publisher와 cache generation 소비 |

Engine/Public 변경이 필요하면 기존 범용 변환/클론 책임에 한정하고 Product SDK 배포를 검증한다. 새 C++ 파일이 실제로 필요한 경우에만 해당 프로젝트와 filters에 등록한다. 현재 계획에서는 두 번째 Catalog 파일, 별도 모델 런타임, 별도 테스트 하네스를 만들지 않는다.

구현을 재개할 때는 다른 작업의 최신 변경을 먼저 확인하고 중단 전에 작성한 부분 변경을 재검토한다. 이 문서 자체는 구현 재개·빌드·배포 지시가 아니다.

## G15. 현재 중단 상태와 이후 검증

사용자가 다른 작업을 우선하라고 지시한 시점에 내부 구현 작업을 중단했다. 그 전에 occurrence placement codec/Workbench 일부, Shared/Server placement 필드 일부, Preview 기준점 처리 일부가 working tree에 작성되어 있다. 이 추가 변경은 통합 빌드/실행을 확인하지 않았고 이번 단일 목록/Anchor Boss/Save 통합이 구현되었다고 기록하지 않는다. 중단 뒤 코드를 되돌리거나 새 빌드·데이터 publish를 하지 않았다. 기존 다른 담당자 변경도 보존한다. 중단 지시 이후 다른 작업에 메시지를 보내지 않는다.

이전 G09까지의 검증 기록과 이번 미완성 변경을 구분한다. 최초 요청 당시 이미 완성된 CONTACT/Bone 지원의 결과는 Gate Pattern Bundle RESULT G10/G11이며, 새 World 배치 구조의 완료 증거로 재사용하지 않는다.

구현 재개 후 검증은 기존 경로만 사용한다.

- 기존 native editor에서 동일 Object의 다중 Append와 default/연결 상태 전환, 개별 Transform/Anchor Save→Reload, 기존 문서 의미 보존과 실패 rollback.
- 기존 WorldSequence/Composition projector tests에서 Object/state 참조·Map/Boss/Character anchor·카드 target 위치·Save 적용 자료 검증.
- Shared wire 변경 시 기존 NetworkProtocolHarness의 WORLD 왕복/잘못된 입력 보존, 기존 Server object/bundle 계약에서 정확한 occurrence 대상과 revision 경계.
- 변경 JSON/XML/PowerShell parse, `git diff --check`, 최소 컴파일. 최종 배포는 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`.
- Client/UI는 에이전트가 실행·조작·캡처하지 않는다. 사용자가 두 도구의 같은 목록, 모든 Resource Preview, 개별 카드 배치, Save 후 표시와 실제 아레나 재생을 확인한다.

## G16. 재개 범위와 초기 상태 저장 계약

이번 요청은 Composition Resource의 부모 Object 단일 선택 → Append → Box Detail의 연결 상태·clip 확인과 독립 Map Transform → Save 및 기존 publisher 자동 적용이다. Object의 defaultMotionInstanceId를 새 Append의 초기 상태로 사용한다. Composition world definition은 objectResourceId와 선택 당시 sequenceInstanceId를 저장하여 부모 참조와 박스 초기 상태를 함께 보존한다. Object 기본 상태를 이후 변경해도 기존 박스의 초기 상태를 자동 교체하지 않는다. 미완성 DRAFT와 사용자의 Collider/Logic 배치는 자동 승격·생성하지 않는다.

G16 구현 및 자동 검증 결과는 대응 RESULT G10에 기록했다. G10~G15의 Boss Object Anchor·Dissolve 등 확장 계획은 이번 완료 범위에 포함하지 않는다.


## G17. 배치 편집 즉시 갱신과 뿅망치 Collider 고정 장착

2026-09-08 사용자 실행에서 Position 숫자 변경은 저장되지만 보이는 카드가 움직이지 않는다고 확인했다.
Workbench의 다른 Preview 재생 중 요청 생략을 제거하고, 실제 편집 occurrence가 현재 재생 중인 경우에만
기존 clock과 Object를 유지한다. 단독 배치 Preview도 원본 occurrence ID로 draft 배치를 찾아 동일 객체에
적용한다. 다른 재생이나 아직 생성되지 않은 카드 편집은 선택 Pattern의 배치 Preview로 전환한다.
MainApp의 기존 typed request, PresentationPlayer, Level WorldSequencePlayer 경로를 확장한다.

Collider BOX의 height는 저장되지만 HitAreaWire가 바닥 네 변만 그리는 것을 확인했다. 기존 skill wire와
Server XZ 판정은 유지하고 Composition BOX만 높이가 있는 12개 모서리를 그린다. 사용자의 최신 요청에
따라 조커찾기의 중앙/가장자리 세 Collider는 현재 크기를 유지한 채 망치 끝면과 양옆에 고정 장착한다.
Logic 연결은 사용자가 저작한다. 새 런타임/새 C++ 파일 없이 기존 Bone anchor와 publish 소비자를 사용한다.

기존 editor 계약 검사에 재생 중 위치 편집 요청 누락 재현을 포함하고, 변경 Client 최소 컴파일 및
EXE 링크/배포, 관련 JSON parse와 diff check를 수행한다. Client 화면은 사용자가 직접 확인한다.

G17 재개 경계: 사용자가 뿅망치 고정 장착을 중단하고 카드 Position 수정을 계속 진행하도록 지정했다.
망치 Anchor/Logic/사용자 배치 데이터는 수정하지 않는다. 이미 작성한 Collider BOX 높이 표시 소스는
보존하며, 이번 검증·배포의 필수 목표는 카드 Position이 정확한 Preview 객체에 즉시 전달되는 것이다.
