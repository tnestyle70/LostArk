# LostArk merge 회귀 방지 정본

### 이동하는 원본 이펙트는 그룹 이름·반복·거리 방출을 함께 확인한다

- UE Matinee의 FName 연결은 대소문자를 구분하지 않는다. `Pc01tr`/`pc01tr` 같은 표시 차이로
  actor Move를 놓친 뒤 빈 key를 정상 정지 상태로 저장하지 않는다. 정확한 occurrence binding을 검증한다.
- SpawnPerUnit·world-space ribbon은 실제 emitter 위치 이력이 필요하다. core 몇 개가 출력되거나
  finite 검사에 통과했다는 사실만으로 이동·잔광 복원 완료를 판단하지 않는다.
- Required의 임시 emitterLoopCount 1을 원본 값으로 유지하지 않는다. 완전한 CDO 체인에서
  loop 0을 확인한 경우 원본 Toggle 종료와 함께 복구하고 KillOnDeactivate/Completed를 보존한다.
- SourceTransformTrack가 움직이면 VelocityInheritParent도 document root가 아닌 해당 emitter의
  실제 world 속도를 사용한다. birth simulation basis와 source scale을 한 번씩 적용하고, 기존
  source track 없는 root/local/bone 경로를 함께 대조한다.
- 원본 CONSTANT 위치 도약은 Director의 즉시 camera cut과 같은 시각인지 먼저 조사한다.
  독립 Play All의 카메라 없는 재생 차이를 임의 평활·clamp로 숨기지 않는다. 상세 근거와 화면 미확인
  범위는 [금빛 이동 축포 결과 G07](09-11/2026-09-11_KOUKU_PLAYER_ANCHOR_RAINBOW_FIREWORKS_IMPLEMENTATION_RESULT.md)에 둔다.

### Effect Play All의 scene player 오류는 생성 전 등록 경계부터 확인한다

- `Enter an arena with a scene player before Play All`은 Effect factory·particle simulation·shader
  이전의 scene player 조회 실패다. 카메라와 이동이 정상이어도 `Resolve_SceneCharacter` 등록은 별개다.
- 실제 local player의 생성·class replacement commit을 소유한 `CClientReplication`이 scene target도
  연결한다. remote actor나 실패한 교체로 target을 덮지 않고 despawn/reset/destructor는 자기 캐릭터만 해제한다.
- 오류를 숨기려고 임의 world origin, preview 캐릭터, 첫 Layer 오브젝트를 플레이어로 대신 선택하지 않는다.
  Effect의 root attachment·shader·월드 좌표를 바꾸기 전에 실제 호출자가 어느 단계에서 거절됐는지 구분한다.
- 원본 독립 festival과 시퀀스용 authored festival은 시작 시각이 다르다. 전자는 0초, 후자는 현재
  시퀀스의 33.40897초부터 발생한다. 기존 MAP 배치와 원본 발생 시각을 Play All 편의를 위해 덮지 않는다.
- 다색 방사형 fireworks와 festival을 이름만으로 같은 문서로 취급하지 않는다. 원본 ParticleSystem과
  Matinee·주변 소품·현재 occurrence를 대조한다. 원본 MAP 좌표를 다른 카메라/무대의 시퀀스에 옮길 때는
  source actor 대응이나 공통 변환을 먼저 확인하며, element 내부 회전·크기를 root에 중복 적용하지 않는다.
  실제 검사와 사용자 화면 판정은 [플레이어 앵커·폭죽 결과](09-11/2026-09-11_KOUKU_PLAYER_ANCHOR_RAINBOW_FIREWORKS_IMPLEMENTATION_RESULT.md)의 G06에서 구분한다.

### 클릭 이동 예측의 위치·높이·회전·카메라를 각각 검증한다

- Client가 waypoint 직선을 예측하면 Server 일반 MOVE도 위치를 같은 목표 방향으로 진행해야 한다.
  몸의 제한 회전을 이동 벡터에 다시 적용하면 반대 클릭에서 곡선 이동과 ACK 되감김이 생긴다.
- 같은 navgrid 파일만으로 경로 일치를 보장하지 않는다. 기존 publisher의 맵별 navpolicy도 제품
  Loader와 Character 경로 요청까지 연결한다. 기본 step을 일괄 상향하거나 월드 상수를 복제하지 않는다.
- NavGrid 선분 검사는 실제 셀 경계 통과 순서를 구분한다. 떨어진 두 경계 통과를 한 대각선으로
  합치면 지나지 않는 장애물 때문에 우회한다. 정확한 모서리·막힌 내부 경계의 차단은 보존한다.
- 경로를 단축한 먼 waypoint의 Y를 미리 보간하지 않는다. XZ로 전진한 현재 발밑 지면을 읽는다.
- ACK 오차를 고정 80ms에 줄이면 오차가 커질수록 표시 보정 속도가 커진다. 일반 연속 오차에는
  속도에 따른 보정 시간을 적용하고 실제 teleport·강제 상태의 권위 전환은 따로 처리한다.
- 최단 yaw 보간의 소유자는 Character 한 곳이다. ACK마다 helper도 yaw를 보간하면 위치 오차가
  회전까지 늦춘다. 실제 Character 소비자와 반복 ACK로 확인하고 helper에 완성 pose만 넣어 검증하지 않는다.
- camera profile의 followResponse 0은 즉시 추적이다. 입력 반응과 카메라 감쇠를 분리하고 SPACE/스킬
  handoff, cinematic override와 복귀도 확인한다. CPU 검사 성공을 사용자 조작감 판정으로 기록하지 않는다.
- 실제 변경과 검증 범위는 [클릭 이동 결과](09-11/2026-09-11_CHARACTER_ACTION_COMPOSITION_AND_RESPONSIVENESS_RESULT.md)를 따른다.

## 0. 모든 세션의 사용자 전용 화면 검증 경계

- 세션 시작 시 `AGENTS.md`, `CLAUDE.md`, 이 문서, 있으면 `gotchas.local.md`,
  `.md/TEAM/README.md`, 대응 PLAN/RESULT를 먼저 읽는다.
- Artist F, Effect Tool, Character Select와 Client 시각 결과는 사용자만 직접 조작하고 최종 visual fidelity를 판정한다.
- 에이전트는 Client나 UI를 자율적으로 실행·조작하지 않고 화면 캡처·스크린샷 생성을 하지 않으며,
  visual fidelity를 대신 판정하지 않는다.
- 사용자가 대화에 첨부한 스크린샷이나 이미지 분석을 요청하면 에이전트는 반드시 열람·분석해
  관찰된 결함과 가능한 occurrence 진단을 보고한다.
- 에이전트는 빌드와 구조화된 로그·수치 진단, 실행 준비까지만 수행하고 사용자가 직접 누를 경로를
  보고한 뒤 멈춘다. 사용자의 서면 판정 전에는 first pixel, eye smoke, visual PASS를 기록하지 않는다.
- 일반적인 완성·복원 요청과 기존 캡처 파일의 존재는 Client/UI 자율 실행·조작이나 화면 캡처를 허가하지 않는다.

이 문서는 merge, pull, rebase와 충돌 해결에서 이미 닫힌 다른 작성자의 계약을 되살리거나
지우는 회귀를 막는 공용 체크리스트다. 날짜별 실패 로그는 대응 RESULT에 기록한다.

렌더링·캐릭터/무기 재질·이펙트 복원의 원본 입력, 기본값 상속, 좌표·재생 시계와
Resources 경계는 [렌더링이펙트복원V2.md](렌더링이펙트복원V2.md)를 함께 읽는다.
해당 분야의 재사용 원리와 실제 연결 범위는 그 문서에서 갱신하고 여기에는 복제하지 않는다.

### 맵 그림자 최적화의 보존 조건

- camera frustum을 shadow caster에 적용하지 않는다. 실제 light view/projection을 사용하고
  frame provider 뒤에 instance를 준비한다. light 변경 없이 실패한 upload도 다음 호출에서 재시도한다.
- 단순 shadow pass와 opaque null-PS 선택은 surface family뿐 아니라 source-material 활성 설정도
  함께 확인한다. 설정이 꺼진 경우 기존 legacy diffuse alpha를 보존한다.
- 장비 pose cache는 source pointer만으로 판정하지 않는다. owner 수명과 source/destination
  revision을 함께 검사해 동일주소 재할당·같은 프레임 포즈 변경을 반영한다.
- `Render.Shadow` GPU elapsed를 순수 GPU 실행시간으로 단정하거나 fixture 개선율을 사용자 FPS로
  환산하지 않는다. 현재 연결과 검증 경계는 [렌더링 복원 가이드](렌더링이펙트복원V2.md)를 따른다.

### 조명·애니메이션 성능 변경의 merge 경계

- Deferred shader는 Engine 정본과 Client 사본을 함께 유지한다. instance record stride·최대 개수·pass index를 한쪽만 복원하지 않는다. 조명 정렬과 shader 내부 합산으로 기존 FP16 순서를 바꾸지 않는다.
- CChannel key는 공유되고 cursor는 CAnimation clone의 상태다. Effect model cue의 pose 재사용을 일반 캐릭터 전체로 넓히지 않는다. 객체별 uniform cache도 공유 CShader Effect의 다른 caller 변경을 놓친다.
- 구체적인 소유·소비 경계는 [렌더링 복원 가이드](렌더링이펙트복원V2.md)의 조명·Alt+V 항목, 수치 예외와 실제 FPS 확인은 [성능 결과](09-12/2026-09-12_MAP_CHARACTER_RENDER_PERFORMANCE_RESULT.md)를 따른다.

## 1. 동기화 전 상태 고정

다음 증거를 먼저 남긴다.

```text
git status --short
git diff --name-only --diff-filter=U
git branch --show-current
git rev-parse HEAD
git fetch --prune
git rev-list --left-right --count HEAD...origin/main
```

- dirty worktree를 정리한다는 이유로 다른 작성자의 변경을 reset, checkout, clean하지 않는다.
- 동기화가 필요하면 추적·미추적 파일을 포함한 이름 있는 safety stash를 만들고, 원격 반영 후
  같은 stash를 복원한다. stash 적용 충돌도 아래 파일 역할 기준으로 다시 해결한다.
- `ours` 또는 `theirs`를 파일 전체에 일괄 적용하지 않는다. 실제 소유자, 호출자, 데이터 정본,
  실패 소비자와 대응 PLAN/RESULT를 읽고 계약별로 합친다.

## 2. 충돌 해결 불변식

### DimensionMaster 이름과 런타임 계약

- 활성 코드·공유 enum·catalog·Loader·HUD·Server profile·spawn·Animation Tool의 class 이름은
  `DimensionMaster`/`DIMENSIONMASTER` 계약을 유지한다.
- 이전 `Dimensionist` 이름은 명시적으로 보존한 역사 문서나 외부 원본 식별자가 아니면 되살리지 않는다.
- 이름 불일치를 임시 fallback으로 숨기지 않는다. 정의와 실제 소비 경로를 같은 변경에서 맞춘다.
- DimensionMaster의 Server skill 계약은 `Q W E R A S D F T V ALT_V`와 LMB `2050010` automatic
  3-stage(`1500/1067/1700ms`)로 닫혔고 `ALT_V`는 `2050540`이다. merge에서 LMB 4단 수동
  window, 이전 candidate-only `2050550` 또는 Z 슬롯을 되살리지 않는다.
  candidate-only Effect는 별도의 admitted effect 계약 없이 제품 런타임에 활성화하지 않는다.

### 공용 Character Preview

- Model Preview, Animation Tool, Effect Tool이 공유하는 Character Preview Panel 계약을 유지한다.
- 충돌 해결 과정에서 툴별 두 번째 character loader, pivot owner, preview runtime을 만들지 않는다.
- project와 filters 등록, 실제 include/caller, 모델·무기 part 경로를 함께 확인한다.

### Effect Tool 재작성 경계

- Effect Tool reboot의 정본은 G0/G1에서 승인한 `Effect_AuthoringDocument`와 최소 ImGui document
  경계다. 현재 G 계획이 삭제한 레거시 `Effect_AssetIO`, `Effect_Runtime`,
  `Effect_ParticleSimulator`, `Effect_ResourceCatalog`, `Effect_Types`, 전용 Effect shader와
  생성된 `.effect` 후보 파일을 merge가 다시 살리지 않게 한다.
- 추출 원본과 증거 자료는 저작 데이터와 구분한다. Source Catalog/Extracted/참고 PNG처럼 계획이
  보존하기로 한 원본은 레거시 런타임 삭제와 함께 지우지 않는다.
- G1 Active Document는 메모리 저작 단위이며, Element 종류 radio 선택만으로 Document를 변경하지 않는다.
  G2의 Add Element 이후에만 Element가 Document에 들어간다.
- Effect asset ID와 resource ID는 `Client/Bin/Resources` 기준 상대 안정 ID다. 절대 경로,
  drive-qualified 경로, `..` 탈출 경로를 저장 계약으로 되살리지 않는다.
- 제품 Effect는 `Data/Effects/EffectCatalog.json`과 `Data/Effects/Authored/*.effect.json`만 직접 읽는다.
  `Client/Bin/DataFiles/Effect`, hash seal, VisualPrograms sidecar, Effect publisher를 merge나 복구 과정에서
  다시 만들지 않는다. Editor Save는 파일 저장과 다음-spawn Product activation을 한 transaction으로 처리하고,
  activation 실패 시 compare-and-swap으로 이전 파일과 prepared target을 모두 보존한다.

### Bone/socket scale은 transform 계층별로 검증

쿠크 MN_RPCT_05처럼 ActorX/FBX로 쿠킹한 골격은 rest/animation root basis에 이미100이
들어갈 수 있다. 실제 세 공격 clip의 combined basis100에 CModel preScale .017이 적용되면
socket 입력을 받을 배율은1.7이다. 여기에 cm→m 역보정이라고100을 다시 곱하면170이 되어
sprite 크기와 socket offset이 모두100배 커진다. `Build_SourceAnchorWorlds`는 이 중복 곱을
제거한다. bone translation과 animation scale은 유지하고, 캐릭터용 변환을 다른 쿠킹 모델에
그대로 복사하지 않는다. 원본 Size의 .01 변환, mesh modelPreScale, 골격 basis는 각각 확인한다.

finite/count 검사와 synthetic scale1.7을 넣은 CPU probe는 이170배 오류를 검출하지 못한다.
설치된 모델의 실제 bone 행렬과 local socket이 만드는 원점/축 길이를 함께 확인한다.
원본 Required/CDO를 해석한 후 `bUseLocalSpace`가 생략됐다면 legacy importer의 미확인
true fallback을 복구본에 복사하지 않는다. 기본 false는 출생 시점의 월드 transform을 유지하며,
notify의 bone-follow와는 다른 상태다. 명시 true/false와 null distribution은 임의로 덮지 않는다.
근거와 실행 범위는 [쿠크 두 패턴 결과](09-11/2026-09-11_KOUKU_GATE1_TWO_PATTERN_FULL_RESTORE_IMPLEMENTATION_RESULT.md)에 둔다.

- model prototype admission scale을 `CModel::Get_BoneMatrix()`의 combined socket scale로 간주하지 않는다.
  Artist는 admission `0.0001`과 rig `sdm` root `100`이 합성되어 `b_wp_1` combined basis가 `0.01`이다.
- Artist Effect anchor는 combined basis `0.01`을 exact tolerance로 검사한 뒤 3x3에만 `x100`을 적용한다.
  translation, animation rotation, asset orientation과 owner world는 보존한다.
- 관찰한 임의 scale 자동 정규화, `0.0001 또는 0.01` 동시 허용, raw fallback은 금지한다. 다른 class/asset에
  확대할 때는 `prototypeAdmissionScale`, `rigRootScale`, `combinedAnchorScale`, reciprocal을 manifest가 소유한다.
- 회귀는 synthetic matrix만으로 닫지 않는다. 실제 model을 제품 pretransform으로 로드해 bind pose와 대상
  animation pose의 named bone combined scale, 정규화 결과, 잘못된 scale fail-close를 Debug·Release에서 검사한다.

### ObjectManager layer-map AV는 Effect 오류와 분리

- `std::map<..., shared_ptr<CLayer>>::_Find_lower_bound` 내부 AV만으로 missing layer tag, map insertion,
  Effect clone 또는 shader를 원인으로 확정하지 않는다. key 비교 전 map tree-state 주소에서 fault면 manager/map
  lifetime, ABI, out-of-bounds 또는 선행 메모리 손상을 full dump로 구분한다.
- authored animation Effect cue와 balance `effectId` fallback은 별도 경로다. `effectId=""`만 보고 cue가 없다고
  결론내리지 말고 실제 `.animevents`의 published `effectref=asset` row와 runtime caller를 함께 확인한다.
- 위험 경로를 queue로 옮긴 뒤 같은 RVA가 재현되면 그 변경을 root fix로 기록하지 않는다. Client 전용 full
  LocalDump의 전체 stack과 matching EXE/DLL/PDB identity가 확보되기 전에는 `OPEN`을 유지한다.

### Effect 준비 성공과 첫 GPU draw 성공은 별도 gate

- Catalog parse, typed Program 검사, DDS/SRV/sampler 준비와 prepared-cache commit이 PASS해도 실제
  `Bind -> Begin -> Draw`는 아직 한 번도 실행하지 않았을 수 있다. Effect 종료 회귀는 제품과 같은
  `CEffectObject`를 layer에 넣고 실제 fixed-step occurrence가 활성화된 시점까지 진행한 뒤 첫 draw의
  `attempted/submitted/suppressed/failed/committed`를 검사한다.
- native-v14 Artist 문서를 format-13 runtime drawable로 낮출 때 `Renderer.eType`과
  `Renderer.eSourceSpace`를 지우지 않는다. 35개 element는 stable element/emitter ID로 Program의
  renderer/source-space와 다시 exact join하고, aggregate family count만 맞는 것은 증거로 쓰지 않는다.
- 반대로 기존 v3~v13 문서에는 `SourceRecipe.bEnabled=true`이면서 `Renderer==END`인 정상 저작 문서가
  존재한다. 이 경로는 kind와 geometry binding으로 legacy family를 결정한다. typed Artist 규칙을
  legacy에 역적용하거나 `SourceRecipe.bEnabled`를 이유로 GPU occurrence를 제거하지 않는다. 2026-08-12
  기준 회귀 분모는 authored 18문서, particle/decal GPU occurrence 1,300개다.
- Effect 하나의 frozen input, packet denominator, profile, local resource identity 위반은 명시적인
  `LOCAL_CONTRACT`로 기록하고 해당 object만 격리한다. D3D Map/draw, device removal, OOM, global render
  target과 presentation capacity 실패는 `GLOBAL_RUNTIME`으로 전파한다. `E_FAIL` 값만 보고 둘을 추론하지 않는다.
- 첫 draw 회귀는 최소 Artist Full35와 legacy Lance BA1을 함께 태운다. Artist만 검사하면 legacy
  renderer fallback 퇴행을, Lance만 검사하면 35행 typed renderer join 퇴행을 놓친다. 화면 모양은 이
  자동 gate가 녹색인 뒤 사용자가 별도로 판정한다.

### 요청한 키 슬롯의 실제 Effect 연결 확인

F 번개를 Alt+V로 옮기는 요청은 키 이름만으로 기존 패치 함수를 재사용하지 않는다.
워로드 F17140의 native446 mesh 번개를 V17170에만 추가했던 패치는 Alt+V17250을 수정하지
않았다. `PlayerSkills → skillbindings → animevents → 실제 두 clip effect ID`를 확인하고,
같은 번개 요청은 F의 geometry/WPO/Dynamic/수명/root snapshot을 함께 재사용한다.
색상은 실제 native 입력을 바꾸고, 다른 sprite 복제나 표시 이름 변경을 번개 연결로 기록하지 않는다.
상세는 [워로드 결과](09-09/2026-09-09_WARLORD_ASVF_FULL_RESTORE_IMPLEMENTATION_RESULT.md)의 Alt+V 항목을 따른다.

### 원본 월드 좌표 입력과 이동량 기반 생성

거미카운터2349의 world-offset 재질은 VS가 전달한 월드 위치와 원본 PS의 alpha 입력을 함께
복구해야 한다. 미해석 register를0으로 초기화하면 원본 텍스처가 있어도 장판이 투명해진다.
같은 재질의 donor 이름만 맞추지 않고 실제 permutation·VS/PS 입출력·emitter 역행렬을 확인한다.
native ribbon2346도 material admission, point RGBA/width/dynamic, renderer vertex 소비를 함께 닫는다.

Rate/Burst0인 source emitter는 SpawnPerUnit 유무를 먼저 확인한다. 거미 돌진의2345/2347/2348은
저속.1m/s CPU 입력에서0개였지만10m/s에서 모두 생성됐다. 원본 거리 단위가 누적되기 전에
검사를 끝내거나 임의 burst를 추가하지 않는다. 자세한 증거는
[거미카운터 결과](09-11/2026-09-11_KOUKU_SPIDER_COUNTER_SOURCE_EFFECT_IMPLEMENTATION_RESULT.md)에 둔다.

### 패턴 Effect 추가 후 실제 Product 편입까지 확인

Kouku publisher의 exit0은 모든 저장 패턴이 Product에 들어갔다는 뜻이 아니다. projector는
잘못된 패턴을 unavailable로 격리한 뒤 정상 패턴을 배포할 수 있다. 거미카운터 후반 준비 clip에
원본 먼지의 긴 Effect 수명을 그대로 더하면 `presentation occurrence exceeds the Pattern lifetime`로
기존 패턴과 그 bundle이 빠진다. Effect 내부의 원본 시간은 유지하고 새 occurrence의 종료는
기존 패턴 창 안에서 정한다. 기존 animation/logic을 이펙트 때문에 연장하지 않는다.
대상 patternId의 unavailableReason, generated patternbindings의 실제 Effect asset ID와
occurrence 수까지 확인한 뒤 연결 완료로 기록한다. 전체 count·JSON parse만으로 대체하지 않는다.

### Effect Tool Solo와 선택 그룹의 재생 시계

복원 문서 Solo가 object를 만들었다는 이유로 재생 완료로 처리하지 않는다. 실제 소유자인
`CEffectAuthoringSequencer::Preview_Element`가 마지막에 paused=true로 commit하면 사용자가
Sequencer Play를 한 번 더 눌러야 한다. Solo/Play Group은 준비와 첫 샘플이 성공한 뒤 즉시
재생 상태로 commit하고, 선택ID는 기존 Shift/Ctrl 표시 집합을 사용한다.

선택 그룹은 원본의 min start부터 선택 element의 실제 max end까지 같은 시계를 반복한다.
긴 원본 문서·애니메이션·숨은 provider의 수명을 반복 종료로 쓰지 않는다. 필요한 provider와
model cue anchor는 함께 준비하되, group 반복 여부는 임시 preview row가 소유한다.
Stop/문서 전환으로 임시 재생을 정리하고 저장된 sequence의 Loop 설정을 덮어쓰지 않는다.

### Artist F 수동 검증 경계

- 화면의 최종 판정자는 사용자다. 에이전트는 Client HWND나 Effect Tool을 자율적으로 실행·조작하거나
  직접 캡처하지 않는다.
- 사용자가 첨부한 실행 화면이나 이미지를 분석해 달라고 요청하면 반드시 열람·분석한다. 분석 결과는
  occurrence별 진단·리뷰 입력으로 사용하되 최종 visual PASS나 단독 admission 증거로 승격하지 않는다.
- 자동 증거는 compile, structured diagnostic, resource/shader/draw 수치에 한정한다. 에이전트가 직접 만든
  캡처나 자동 클릭 결과를 구현·리뷰·완료 증거로 사용하지 않는다.
- 에이전트는 Server CMD와 Client 준비 상태, 정확한 수동 클릭 경로만 전달한다. 사용자의 관찰 결과를
  받은 뒤에만 occurrence별 결함과 튜닝 작업을 이어간다.

### Debug Client `abort()` 팝업

- `Client/Bin/Debug/Client.exe` 실행 직후 Microsoft Visual C++ Runtime Library의
  `abort() has been called`가 발생하면 창 핸들이나 흰 배경의 Win32 창이 존재한다는 이유로
  시작 성공으로 처리하지 않는다. Lobby 첫 렌더 프레임이 보여야 시작 smoke 성공이다.
- 확인된 사례에서는 `CMainApp::Ready_Fonts()`가
  `Client/Bin/Resources/Fonts/161ex.spritefont`를 요구했지만 실제 파일이
  `Client/Bin/Resources/Fonts/Fonts/161ex.spritefont`에 있어 한 단계 중첩돼 있었다.
  DirectXTK `BinaryReader`가 `0x80070002` 파일 없음 오류를 기록한 뒤 `SpriteFont` 생성자에서
  C++ 예외 `0xE06D7363`을 던졌고, 처리되지 않은 예외가 `std::terminate()`와 `abort()`로 끝났다.
- `CCustomFont::Initialize()`의 `make_unique<SpriteFont>()`는 실패 시 `HRESULT`를 반환하기 전에
  예외를 던질 수 있다. 이 경계는 2026-08-05에 `std::exception`과 알 수 없는 예외를 포착하고
  생성 중 객체를 정리한 뒤 `E_FAIL`을 반환하도록 수정됐다. merge에서 이 catch를 제거하면
  `CCustomFont::Create()`의 `FAILED(Initialize())` 경계를 건너뛰고 `abort()`가 재발한다.
- 복구 전에는 `Fonts/Fonts` 중첩 여부와 다음 필수 파일이 `Resources/Fonts` 바로 아래에 있는지
  확인한다: `161ex.spritefont`, `YG760.spritefont`, `YG330.spritefont`,
  `YoonGasiIIM.spritefont`, `BMKkubulim.spritefont`. 임의 fallback 경로를 추가하지 않고
  immutable resource pack의 올바른 구조로 Hydrate한다.
- 제품 오류 표시는 font 생성 예외를 파일별 로드 경계에서 포착해 실패 경로를 보존하고 기존
  Client 초기화 실패 메시지 경계로 전달한다. `catch (...)`는 C++ 표준 예외가 아닌 DirectX 경계도
  process abort로 빠지지 않게 하는 마지막 변환이며 성공으로 삼지 않고 반드시 `E_FAIL`을 반환한다.
  기본 폰트나 다른 디렉터리로 자동 fallback해 정상 시작으로 위장하지 않는다.
- 수정 후에는 Debug Client를 다시 빌드하고 사용자가 아무 입력 없이 실행해 Lobby 렌더와 `abort()` 부재를
  눈으로 확인한다. 에이전트는 공유된 결과와 종료 후 잔류 Client process 부재, resource pack의 `Verify`
  결과를 별도로 확인한다.

### 프로젝트·데이터 등록

- 물리 C++ 파일, `.vcxproj`, `.vcxproj.filters`의 등록을 세트로 비교한다.
- 삭제한 manifest나 생성물을 project가 계속 등록하지 않는지 확인한다.
- Git 관리 `Data` 원본은 Client 프로젝트의 `96.DataFiles` 아래 `None`으로만 노출한다.
- `Client/Bin/Resources`는 팀장이 관리하는 runtime 입력이다. 존재하지 않는 asset pack lock이나 immutable manifest를 새 완료 조건으로 만들지 않는다.

## 3. 병합 후 변경 범위 확인

실행하지 않은 항목을 PASS로 쓰지 않는다. 아래는 문제별 확인 지점이며 매 변경마다 전부 수행하는 체크리스트가 아니다.

```text
1. conflict marker와 unmerged path 0개
2. Dimensionist/DimensionMaster 잔류를 활성 코드·데이터와 역사/원본 자료로 분류
3. Effect 레거시 파일·symbol·project 등록 0개, G1 파일·등록 존재
4. Character Preview 공용 경로와 project/filter 등록 확인
5. 변경 JSON과 XML parse
6. 변경한 코드의 Debug compile/link
7. 사용자가 직접 수행한 Character Select 재진입 또는 Effect Tool 수동 smoke의 서면 결과
8. 데이터 배포가 필요한 경우 변경한 domain의 publisher, 재현 중인 오류에 필요한 작은 검사
9. git diff --check
10. 잔류 Client/Server process와 listener 확인
```

개인 PC 경로, 실행 중인 세션 메모, 임시 예외는 `.md/GB/gotchas.local.md`에만 기록하고
Git에 커밋하지 않는다.

## 12. Effect 복원에서 비싸게 배운 것 (2026-08-17 실측)

### 12.1 데이터가 맞아도 담을 그릇이 없으면 화면에 안 나온다

Track A는 원본 추출과 매핑에 성공했다. 실패는 그 다음에 났다.
`Data/Effects/Authored` 3,400 element 실측이다.

```text
source 소유 1,909 element 의 Detail 적용률
  scale / maxParticles / particleLife / startSize / timingLife   100%
  anchor 48%   position 39%
  rotation 1%   color 0%   velocity 0%
```

정적인 축은 전부 정확히 들어갔다. 안 들어간 것은 **시간에 따라 변하는 축과 방향 축**이고,
이유는 추출 실패가 아니라 `Detail` 스키마에 그 개념이 없어서다.

`sourceRecipe.modules` 인구조사가 어디에 정보가 남았는지 말해준다.

```text
particlemodulesizemultiplylife     2259   수명에 따른 크기
particlemodulecolorscaleoverlife   1982   수명에 따른 색·알파
particlemodulelocation             1343   스폰 형태
particlemoduleparameterdynamic     1361
particlemodulecolor                1324
particlemodulevelocity              580   초기 속도
```

**교훈**: 추출이 끝났다고 복원이 끝난 것이 아니다. 저작 스키마가 그 축을 표현할 수 있는지를
추출 전에 확인한다. 표현할 수 없으면 추출한 값은 문서에 남아도 화면에 오지 않는다.

### 12.2 재생 소유권이 저작 수치를 무효로 만든다

`sourceRecipe.enabled`가 true면 재생이 원본 모듈을 따라가고 저작 `Detail`은 무시된다.
`Effect_Playback.cpp:669` 외 6곳이 그 게이트다.

```text
Detail.Transform    게이트 없음   -> 크기·위치 튜닝은 먹는다
Detail.Particle     게이트 있음   -> particle 수 튜닝은 안 먹는다
```

"어떤 스킬은 되고 어떤 스킬은 안 된다"의 정체가 이것이다. 스킬 차이가 아니라 **건드린 축의 차이**다.
튜닝이 안 먹으면 먼저 이 플래그를 본다.

### 12.3 제품이 보는 문서와 저작할 수 있는 문서가 달랐다

같은 스킬에 문서가 네 갈래였다.

```text
.unified                 sourceRecipe 소유   <- runtime catalog 99개 중 98개가 이것
.effect.json             저작 소유           <- catalog 에 없다
.authored-baseline       저작 소유           <- catalog 에 없다
.restoration-candidate   저작 소유           <- catalog 에 없다
```

저작 가능한 문서는 화면에 나오지 않고, 화면에 나오는 문서는 저작이 무시됐다.
**저작 전에 그 문서가 runtime catalog 에 실려 있는지 확인한다.**

### 12.4 중복 판정의 기준을 바꾸면 답이 뒤집힌다 (2026-08-17 재실측으로 교정)

이 절은 원래 "Track A import 가 source occurrence 당 element 를 만들어 같은 시각 요소가
5~6번씩 들어갔다"고 기록했고 `element 7,861 -> 3,042`, `210.9 MB -> 84.9 MB`를 근거로 삼았다.
**그 수치는 잘못된 signature 의 산물이며 중복은 실재하지 않았다.** 되돌리기 `413e4e36`
이후 같은 corpus 8,219 element 를 세 기준으로 다시 셌다.

```text
binding (slotId, assetId) 만          4,759   57.9%
binding + transform                   4,003   48.7%
element 전체 (id/displayName 제외)        6    0.1%
```

되돌린 규칙이 합쳤을 4,471쌍을 열어보면 detail.particle 11,855, sourceNode 4,449,
detail.timing 2,847, detail.transform 1,386, 심지어 kind 109 가 서로 다르다. 한 텍스처를
여러 위치·크기·입자수로 배치하는 것이 이펙트 구성 방식이므로 binding 기반 판정은 공간
구조를 파괴한다. 워로드 17030 이 21 -> 9 로 줄고 손튜닝 하나가 사라진 것이 그 결과다.

`NO_RESOURCES` 일괄 삭제도 틀렸다. light 44 중 28, screenPost 56 중 39 는 텍스처를
바인딩하지 않는 것이 정상이다.

**교훈**: 일괄 삭제 전에 무엇을 동일성의 기준으로 삼았는지 먼저 쓰고, 그 기준을 한 단계
엄격하게 바꿨을 때 답이 얼마나 달라지는지 재본다. 두 수치의 차이가 크면 기준이 틀린 것이다.
남은 진짜 중복 6개도 additive 로 겹쳐 그려지므로 지우면 그 element 밝기가 절반이 된다.

### 12.4.1 소유권 flip 은 축별 게이트가 아니다

`sourceRecipe.enabled = false` 는 `Effect_Playback.cpp` 28개 지점에서 시뮬레이터 전체를
갈아탄다. 따라서 이식 도구가 "해석 못 하는 모듈은 건드리지 않는다"고 해도 flip 이후에는
그 모듈이 실행되지 않으므로 보존이 아니라 삭제다.

```text
source 소유 particle 4,609
   모든 모듈이 저작 스키마로 표현 가능      109   2.4%
   최소 한 축을 잃음                     4,500  97.6%
   주요 손실: parameterdynamic 3,058, cameraoffset 1,713, rotation 1,614,
              meshrotation 1,161, orientationaxislock 1,023, subuv, orbit, acceleration
```

그리고 `Detail.Color.multiply` 와 `Detail.Transform` 은 source 소유 element 에서도 이미
합성되어 먹는다(`Effect_Playback.cpp` ~4996 의 `ElementColor * Particle.vColor`). 막혀 있던
것은 `Detail.Particle` 축뿐이다. 그래서 소유권을 내리는 대신 저작 배율을 원본 결과 위에
곱하는 `Detail.Particle.sourceScale` 을 넣었다. 축이 더 필요하면 flip 이 아니라 같은 방식으로
하나씩 추가한다. 상세는
`.md/GB/08-17/2026-08-17_EFFECT_SOURCE_TRIM_AND_DEDUP_CORRECTION_RESULT.md`.

### 12.5 원본 동일의 비용 단위는 스킬 수가 아니라 exact program과 ABI다

도화가 F가 가장 높은 화면 완성도를 낸 것은 generic profile 하나에 맡긴 결과가 아니라, stable
occurrence와 resource 역할을 고정하고 다음 translated/typed 셰이더들을 실제 carrier에 연결했기 때문이다.

```text
Shader_Artist31470RuntimeMaterial.hlsli         34 sample
Shader_Artist31470Active003RibbonMaterial.hlsli  2
Shader_Artist31470Active011OuterMaterial.hlsli   4
Shader_Artist31470Active022DecalMaterial.hlsli   1
Shader_Artist31470Diagnostic.hlsli               6
```

`g_SourceTexture0..6`을 실제로 샘플링하는 것은 이 파일들과 decal adapter 뿐이고,
표준 경로 `Shader_EffectCommon.hlsli`는 이름 있는 5개만 샘플링한다.

**교훈**: "원본과 동일"은 element나 스킬마다 셰이더 한 벌을 요구하지 않는다. equation이 같은
occurrence는 translated HLSL program과 renderer adapter를 재사용하고 texture·CB·sampler 차이는
exact descriptor가 소유한다. equation이 다르면 새 program, VF/pass/scene/RT topology가 다르면 새
adapter가 필요하다. 도화가 F에서 사람이 쓴 전용 파일은 이 경계를 처음 증명한 선례이지,
스킬별 renderer 복제를 정본으로 만든 근거가 아니다. 현재 공정은
[`EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md`](../TEAM/EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md)를
따른다.

### 12.6 판정자 없는 목표를 세우지 않는다

`100% 복원`, `원본과 동일`은 판정할 oracle 이 없으면 완료를 선언할 수 없는 목표다.
그런 목표는 진척감을 없애고, 그 자리를 커밋 수와 문서 수 같은 대체 지표가 채운다.

목표를 쓰기 전에 **무엇을 보면 끝났다고 판정하는가**를 한 줄로 먼저 쓴다.
그 판정자가 없으면 목표를 바꾼다. 화면에 변화 없는 작업이 이틀 연속이면 경보로 취급한다.
## Valtan source carrier를 system-wide mesh로 합치면 100배 WModel과 carrier 붕괴가 함께 난다

- 발탄 ParticleSystem 하나에 mesh emitter가 있다는 이유로 같은 system의 모든 emitter에
  `meshModel`을 복사하면 안 된다. emitter별 원본 carrier가 정본이며 Sprite, Mesh, Decal,
  Light를 각각 보존해야 한다.
- `build_valtan_stage_effects.py` 계열의 clip aggregate는 source 감사 자료일 뿐 V1 Product 입력이
  아니다. reviewed occurrence와 `carrierKey + sourceOrder + rendererShape`가 exact join된 행만
  candidate element가 될 수 있다.
- `Effect/Valtan/Meshes/**/*.wmodel`을 사용하는 exact Mesh carrier는
  `detail.mesh.modelPreScale=0.01`을 반드시 가진다. 런타임 기본값 `1.0`을 쓰면 같은 WModel이
  100배로 렌더링된다.
- Sprite/Decal/Light carrier에는 `meshModel`과 `modelPreScale`을 넣지 않는다. exact resource와
  portable runtime closure가 닫힌 Sprite/Mesh/Decal은 source material identity를 보존한 채
  `effect.standard + alpha_two_sided_depth_read` 공통 RT0로 손튜닝 시작점을 만들 수 있다. Light,
  ScreenPost, resource/adapter 미해석 행은 임의 quad/mesh로 위장하지 않고 `BLOCKED_REQUIRED`로
  남긴다. 공통 RT0 승격은 family 복원이나 `V1_COMPLETE`를 뜻하지 않는다.
- 회귀 검증은 후보 전체에 대해 `rendererShape=mesh <=> meshModel 1개 + modelPreScale 0.01`과
  `rendererShape!=mesh => meshModel 0개`를 함께 검사해야 한다.
- 발탄 materialization receipt가 전체 `EffectCatalog.json` 해시를 봉인하면 다른 캐릭터가 catalog
  행 하나를 추가하는 것만으로 발탄 검증이 실패한다. receipt는 `effect.valtan.` slice와 catalog
  formatVersion만 봉인하고, 전체 catalog 보존은 publisher가 담당해야 한다. 그래야 병렬 캐릭터
  복원과 발탄 exact carrier 검증이 서로의 Product를 지우거나 재봉인하지 않는다.

## Valtan Product presentationScale 정본은 1.0이고 10배 대상은 본체 HP다

- 2026-08-28 사용자 정정에 따라 `Data/Actors/BossCatalog.json`의
  `BOSS_VALTAN.presentationScale`은 `1.0`이 정본이다. `10.0`은 HP 10배 요청을 시각 배율에 잘못 적용한 값이므로
  다시 복원하지 않는다.
- Server 권위 정본 `Data/Balance/BossProfiles.json`의 일반 `BOSS_VALTAN.maximumHp`는 `600000`,
  `maximumHealthBars`는 `160`이다. 종속 `BOSS_VALTAN_GHOST.maximumHp`는 `60000`을 유지한다.
- Server `collisionRadius: 1.4`와 공격 hit geometry는 scale-one gameplay 기준을 유지한다.
- `GAMEPLAY_FOOTPRINT`는 owner basis scale을 제거한 뒤 authored world scale을 적용하므로,
  Effect footprint 보정이나 미세 scale drift 허용을 이유로 boss presentationScale을 바꾸지 않는다.
- `test_valtan_model_view_composition.py`가 `1.0`과 Server body radius `1.4`를 함께 고정하고,
  gameplay balance/Server 계약 테스트가 본체 HP `600000`을 고정한다.

## Valtan strict join과 Effect Tool에서 재발시키지 않을 경계

### Boss Pattern/All Effects/Composition 목록이 함께 사라지면 공용 graph admission부터 본다

Boss Tool, Effect Tool의 `All Effects -> Valtan`, Composition Patterns는 서로 별개의
패턴 catalog를 갖지 않는다. 셋 모두 `Valtan.gameplay.json`,
`Valtan.presentation.json`, generated `ValtanEncounter.json`, rotation, Effect binding,
BossCatalog/combat-object를 strict join한 동일한 canonical Pattern tree에서 목록을 투영한다.
따라서 아래와 같은 화면은 Effect resource나 ImGui category 자체의 문제가 아니라 공용 graph
admission 실패의 연쇄 증상일 가능성이 가장 높다.

```text
No Valtan pattern inventory was staged
0 canonical patterns
canonical Valtan graph did not load
live only; outside All Effects list / UNKNOWN ACTION
Save/Restart button disabled
```

Server는 이미 publish된 bootstrap으로 계속 패턴을 실행할 수 있지만 Client Tool의 새 graph만
거부될 수 있다. 이때 `live only`와 `UNKNOWN ACTION`은 “Server에 패턴이 없다”는 뜻이 아니라,
현재 Server snapshot의 `patternId/actionId`를 Client의 admitted tree에서 resolve하지 못했다는
뜻이다. 화면의 마지막 증상부터 고치지 말고 `Graph reload failed:` 뒤 최초 strict-join 오류를
먼저 고친다.

한 field가 세 화면을 모두 비운 구조적 이유도 함께 기억한다.

- split parser는 모든 managed Pattern을 한 transaction으로 join하고 한 Stage 오류에서 전체 candidate를
  rollback한다. partial authoring tree를 정상 Product처럼 보여 주지 않는 것은 맞지만, 기존 구현은
  generated Product 표시보다 strict split join을 먼저 실행했다.
- last-good snapshot 보존은 같은 process에서 한 번 이상 성공한 뒤에만 가능했다. process restart 직후
  첫 load가 실패하면 Boss Tool, All Effects, Composition 모두 보존할 snapshot이 없어 0개가 됐다.
- 세 Tool이 한 process-level snapshot을 공유하지 않고 각각 같은 tree를 reload했다. 그래서 하나의
  source 오류가 세 곳에서 서로 다른 빈 화면과 버튼 비활성화로 반복 노출됐다.
- Boss Tool과 Effect Tool은 첫 자동 load 실패 뒤 `attempted` 상태가 남는다. 파일을 고친 뒤에도 explicit
  `Refresh/Retry Graph Load` 전에는 다시 시도하지 않아 “수정했는데 여전히 0개”처럼 보일 수 있다.
- reference/legacy/live-only Pattern 하나가 Complete Play 목록에 없는 것은 정상 필터다. 이 경우 graph
  자체는 loaded이고 해당 row만 `[live only; outside All Effects list]`다. 이번처럼 graph 전체가
  `did not load`인 경우와 혼동하지 않는다.

이번 작업과 인접 세션에서 실제 확인한 원인은 다음과 같다.

| 최초 오류/증상 | 실제 원인 | 재발 방지 |
|---|---|---|
| `split gameplay defaultNextActionId drifted: VALTAN_CATCH_BREATH/STEP_02` | Catch 성공/실패를 `ANY_PLAYER_GRABBED -> STEP_03`, `TIMEOUT -> terminal`로 바꾸면서 author script와 generated Product는 갱신했지만 split gameplay의 호환 필드 `defaultNextActionId=STEP_03`이 남았다. strict reader는 명시적 TIMEOUT target인 `null`을 기본 edge로 요구하므로 전체 graph를 거부했다. | branch를 바꾸는 writer는 `defaultNextActionId`를 독립 입력으로 받지 않고 `TIMEOUT.nextActionId`, ordered fallthrough, terminal 순으로 derive한다. `VALTAN_CATCH_BREATH/STEP_02`의 source는 반드시 `defaultNextActionId: null`이어야 한다. author script, split source, generated Encounter를 같은 transaction/revision으로 닫고 negative drift fixture를 실행한다. |
| `split gameplay SET_PLAYER_BIND event is invalid` | Bind event의 exact typed 계약과 source가 달랐다. ENTER는 `heightM=5`, `durationMs=stage duration`, EXIT는 `heightM=0`, `durationMs=0`이어야 하며 unknown/extra field도 거부한다. 한 event의 실패가 split master 전체를 거부했다. | event schema, author script, split source, projection과 runtime consumer를 같은 변경으로 수정한다. parser를 느슨하게 하거나 legacy event로 fallback하지 않는다. |
| `split gameplay SET_PLAYER_SILENCE event is invalid`와 `Encounter stage action lifetime is not closed: VALTAN_SILENCE_SLOT` | `SILENCE_APPLY`는 100ms Stage에서 침묵 5000ms를 한 번 설정하는 deadline-latched 계약이다. Publisher와 Server는 `ENTER only`, `duration >= Stage`, pattern 밖 deadline 만료를 정본으로 사용했지만 Client strict reader는 `duration == Stage`를, Product reference reader는 paired `EXIT`를 요구했다. strict와 fallback이 동시에 실패해 All Effects에는 `EXISTING AUTHORED EFFECTS`와 별도 Area `INDEPENDENT EFFECT`만 남고, Composition/Boss 목록과 Arena presentation admission까지 연쇄 차단됐다. | Client strict source reader와 Product fallback reader가 각각 deadline-latched Silence를 승인하게 하고, Publisher/Server와 동일한 truth table을 focused/native parity 회귀로 고정한다. Silence는 lifetime closure set에 넣지 않는다. actual current JSON을 compiled `ValtanPatternAuditionServiceHarness`로 로드해 ENTER 5000ms 승인, EXIT/value 0/duration < Stage 거부, rollback을 검사한다. Python publisher PASS만으로 Client admission PASS라고 결론내리지 않는다. |
| `master independent Effect did not resolve to one Product owner/document` | V2 도끼를 보이게 하려는 변경에서 independent Effect master row를 추가했지만 exact Product cue/combat-object owner 또는 authored Effect 문서가 정확히 하나로 join되지 않았다. runtime binding 변경과 authoring ownership 변경을 섞어 전체 tree를 깨뜨린 사례다. | 단순 runtime visual 교체는 runtime binding만 바꾼다. independent row는 실제 Product owner, cue timing, authored document가 모두 존재할 때만 추가한다. V1/V2 편집 도구의 목록 소유권을 Product pattern owner로 위조하지 않는다. |
| `BOSS_VALTAN combat-object visual identity is invalid or duplicated` | BossCatalog의 `combatObjectVisuals`에 같은 stable `combatObjectArchetypeId`가 중복되었거나 빈 `clientVisualId/effectAssetId`가 들어갔다. | catalog/projection은 archetype당 visual row 정확히 하나를 보장한다. 기존 row를 교체할 때 append하지 말고 stable ID로 replace하며 combat-object source와 exact join한다. |
| `Valtan scripted-sequence Product parity drifted` 또는 `0 canonical patterns` | saved Boss audition Flow의 occurrence order와 automatic Product rotation order를 하나의 동일 order로 오인해 exact-equal 비교했다. 서로 다른 소유자를 한 revision처럼 묶으면서 전체 tree가 fail-close했다. | saved Flow reference는 Boss Tool audition order를, Product rotation은 자동 전투 order를 각각 소유한다. schema/ID 존재는 함께 검증하되 두 order의 equality를 요구하지 않는다. legacy inline sequence만 기존 parity를 유지한다. |
| old Client에서 새 motion/schema를 연 뒤 모든 목록과 Play가 차단됨 | Data는 새 `{kind, retargetDelayMs, speedMps, distanceM}` 계약인데 실행 중 EXE는 구 parser였다. build가 실행 중 Server/Client의 출력 잠금에서 멈췄는데도 새 EXE로 오인했다. | build 호출 성공 여부가 아니라 Client EXE timestamp/receipt와 실제 strict graph load를 확인한다. 실행 중 EXE의 `LNK1104/MSB302x`는 compile과 link를 분리해 보고하며 old EXE로 새 Data를 검증하지 않는다. |
| Save Flow validation 실패 후 Restart 비활성화 | Save adapter가 합법적인 cross-pattern `COUNTER_HIT -> GROGGY`를 구형 same-pattern/local-action 규칙으로 거부하거나, 첫 candidate가 pending인 동안 두 번째 Save를 유실했다. | Counter success는 local action 또는 cross-pattern target 중 정확히 하나를 허용하고 TIMEOUT은 local failure edge로 검증한다. pending 중 두 번째 Save는 latest deferred candidate로 보존하고 첫 exact terminal 뒤 제출한다. 저장 성공, Server-active revision, 현재 실행 revision을 별도 상태로 표시한다. |
| 첫 Save는 되지만 두 번째 Save/Restart가 계속 막힘 | Apply A 결과 packet을 놓치면 Server가 이미 A를 active로 사용해도 Client transaction이 `UNCONFIRMED`에 남아 deferred B를 영구 대기시켰다. | 성공 packet을 추측해 `COMMITTED`로 만들지 않는다. 현재 연결/월드가 명시한 `ServerActiveRevision == immutable A`일 때만 `ALREADY_ACTIVE`로 reconcile하고, 그 exact A를 base로 queued B를 제출한다. 다른 revision, 다른 world의 관측, concurrent transaction에서는 계속 fail-close한다. |
| source commit 뒤 `COMMIT_SUCCEEDED_REOPEN_FAILED`, Flow는 clean인데 Save/Restart 재개 버튼 없음 | source CAS는 이미 성공했지만 editor reopen/Product publish/apply가 뒤에서 실패했다. Flow dirty flag는 clean이므로 Save 버튼은 비활성이고, 같은 source를 다시 쓰지 않고 post-commit 단계만 재시도할 typed 경로가 없었다. | durable committed revision과 당시 draft generation을 별도 보존한다. `Retry Product Publish / Apply`는 newer edit가 없을 때 그 exact revision을 reopen하고 Product publish/apply만 계속하며 source writer를 다시 호출하지 않는다. 새로운 edit가 있으면 retry가 이를 버리지 않고 거부한다. |
| Save Flow 직후 Lobby fallback과 `Server entry failed` | graph와 별개인 Server process 사망이었다. candidate artifact SHA-256 함수가 1 MiB local stack buffer를 만들었고 1 MiB Server thread stack의 함수 진입에서 `0xC00000FD` stack overflow가 발생했다. Client의 Lobby 문구는 그 뒤 연결 대상 Server가 사라진 후속 증상이다. | 해시 chunk는 bounded size를 유지하되 heap storage를 사용한다. 사용자가 수동 종료한 경우와 Server crash/fallback을 process exit code, dump/structured recovery state로 분리한다. `Server entry failed`만으로 graph 오류라고 결론내리지 않는다. |
| `Canonical Save validation failed; every source/Product owner was preserved` | 편집 source 한 곳만 바뀌고 이를 참조하는 generated Product, owner join 또는 publisher receipt가 아직 이전 revision이었다. validator가 reject한 것은 정상 rollback이며 파일이 저장되지 않았다는 뜻일 수 있다. | Save는 `parse -> validate -> stage -> source/Product CAS -> project -> post-validate -> commit` 한 transaction으로 수행한다. 생성물을 직접 고치거나 validator를 우회하지 않고 첫 stable ID/field 오류를 해결한다. |

현재 `author_valtan_phase_two_mechanics.py --mode Validate`와
`Project-ValtanPatternMaster.ps1 -Mode Validate`가 branch target 존재만 검사하면
`defaultNextActionId` drift를 놓치는 false negative가 생길 수 있었다. authoring helper와 focused
fixture에서 다음 불변식을 직접 검사한다.

```text
explicit TIMEOUT exists  -> defaultNextActionId == TIMEOUT.nextActionId
no TIMEOUT, next stage   -> defaultNextActionId == ordered next action
no TIMEOUT, terminal     -> defaultNextActionId == null/absent
```

목록 표시와 mutation admission도 분리한다. generated Product가 정상이라면 fresh launch에서도
Product pattern/action 목록은 `PRODUCT_ONLY / READ ONLY`로 표시한다. split authoring strict join이
실패하면 Save, Complete Play, Restart, Repeat, Next와 source mutation은 계속 fail-close하고, 화면에는
실패한 `patternId/stageId/field`를 그대로 남긴다. reload 실패 시 last-good display snapshot을 지우지
않는다. Product-only 표시를 authoring 성공으로 승격하거나 서로 다른 generation을 섞지 않는다.

첫 load가 source/Product writer의 짧은 lock 구간과 겹친 경우는 semantic invalid와 다르게 처리한다.
`Create/Project transaction is active`, Win32 sharing violation, admission 뒤 generation change는
transient failure다. All Effects와 Boss Tool은 last-good 또는 read-only Product fallback을 유지한 채
0.25초 간격으로 다시 admission을 시도한다. `attempted=true`만 남겨 fresh process를 영구 0 rows로
고정하지 않는다. 반대로 stable ID/schema/owner/join 오류는 자동 무한 재시도하지 않고 최초 오류를
그대로 표시한다.

Effect V2 closure에는 owner lane이 둘이다. animation의
`Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`에서 reachable한 group/leaf뿐 아니라,
`BossCatalog.json`의 `BOSS_VALTAN.combatObjectVisuals[].effectV2Group`도 포함한다. 점프 도끼의
`boss.valtan.axe`는 후자다. native receipt test가 bindings만 expected closure로 계산하면 정상 도끼
group을 extra artifact로 오판한다. runtime loader와 회귀 fixture가 두 owner lane을 같은 집합으로
계산해야 한다. `SET_PLAYER_SILENCE` stage-action 오류를 Effect 파일 오류로 오인해 V2 binding을
삭제하거나 Arena admission을 느슨하게 만들지 않는다.

Effect V2 validator를 독립 fixture에서도 재사용할 때 `Data/Actors/BossCatalog.json`은 optional
owner lane이다. 실제 제품 저장소처럼 문서가 존재하면 schema/version/Valtan visual owner를 끝까지
strict 검증하지만, Effect V2만 만든 격리 fixture에 문서가 없으면 빈 owner 집합으로 처리한다.
파일을 무조건 열면 제품 Effect는 정상인데 모든 Effect V2 단위 테스트가 `FileNotFoundError`로
무너져 Core가 compile 전에 중단된다. 반대로 제품 저장소의 손상된 BossCatalog를 optional이라는
이유로 건너뛰면 안 된다. 부재 허용, 정상 owner admission, 존재하지만 invalid인 문서의 fail-close를
세 개의 회귀 경로로 유지한다.

### 변경한 기능만 확인하고 writer 작업을 중복 실행하지 않는다

일상 수정은 필요한 compile/link 뒤 사용자가 대상 아레나에서 변경한 동작을 확인한다.
Animation 행을 추가·저장했다면 해당 보스·패턴·행의 저장/재로드/재생을 확인한다. 이 작업을
다른 보스의 PatternTree, 다른 family, 전체 oracle 또는 광역 회귀 실행의 선행조건으로 묶지 않는다.
광역 진단은 사용자가 요청할 때만 실행하며 미실행 자체를 완료·커밋 차단 사유로 쓰지 않는다.

명시 publish나 진단이 같은 writer를 사용하는 경우에는 중복·병렬 실행하지 않는다.
경합으로 발생한 `CANONICAL_TRANSACTION_BUSY`는 실제 기능 결함과 구분한다.
저장 실패 시 기존 항목을 보존하고 재시도가 필요한지 해당 기능의 상태로 알린다.

### Valtan Composition 확장에서 함께 유지할 저작·투영 계약

- `Data/Valtan/Valtan.gameplay.json`, `Valtan.presentation.json`,
  `Valtan.combatobjects.json`이 split 저작 정본이고 `Data/Encounters/Valtan/*`와 Client/Server
  bootstrap은 투영 생성물이다. source를 바꾼 직후 이전 Product와 다른 것은 정상 중간 상태이므로
  Save 전에 물리 Product parity를 요구하지 않는다. candidate를 메모리에서 project·validate한 뒤
  source/Product를 같은 CAS transaction으로 commit하고 post-validate한다. author helper가 같은 행을
  재생성한다면 helper도 함께 갱신한다. 밸런스 수치 변경은
  `2026-08-05.balance-provenance.receipt.json`의 해당 field를 `PROJECT_TUNED`로 동기화하고 정본
  publisher로 bootstrap을 다시 만든다. generated JSON/bootstrap/receipt를 결과 맞추기용으로 직접
  편집하지 않는다.
- Composition Save의 참가자는 Save edge에서 고정한 dirty owner뿐이다. Pattern/Collider만 dirty이면
  clean Pattern Sound에 `Can_Commit...Generation`, editor document load, candidate staging을 요구하지
  않고, clean Effect V2에도 draft `Prepare/Accept`를 호출하지 않는다. 다만 두 clean 물리 문서는
  candidate Product에 대한 read-set dependency로 writer 안에서 계속 strict 검증한다. dirty Sound/V2는
  exact baseline/candidate와 draft generation/revision을 stage하고, 모든 참가자의 parse·validate가 끝난
  뒤 한 번 commit하며, 실패하면 byte-exact rollback하고 성공 뒤 exact candidate로 reopen한다.
  Camera는 typed 저장 owner가 연결되기 전까지 Composition의 read-only/deep-link 경계이며 이 Save에
  빈 sidecar나 추측한 persistence를 추가하지 않는다.
- Effect V2 binding은 header를 먼저 읽어 validator를 dispatch한다. formatVersion 1만 legacy
  compatibility validator로 보내고, formatVersion 2는 exact binding pipeline과 resource read-set으로
  검증한다. v2 문서를 v1 helper에 넣어 실패시킨 뒤 schema를 완화하지 않는다. 구현이 source bytes를
  동일하게 보존하면서 copy 대신 `std::move`를 사용하도록 바뀌었으면 `*out = source` 같은 문자열을
  고정한 낡은 oracle을 고친다. CAS baseline, candidate equality, rollback이라는 의미 계약을 production
  코드의 불필요한 copy보다 우선한다.
- Pattern total duration은 Stage를 생성하는 명령이 아니며 부족한 시간을 숨은 `WAIT` Stage로 채우거나
  선택을 그 Stage로 이동시키지 않는다. Stage 추가·삭제·후속 선택은 stable Stage/action ID를 쓰는
  명시적 topology 명령만 허용한다. Animation Replace/Append도 `clips.size()==3` 같은 개수 gate를
  되살리지 않는다. HOLD 축약은 `start -> one-or-more loop -> end -> optional untagged tail` 역할을
  검증하고 fixed edge/tail을 보존한 채 loop 예산만 Stage clock에 맞춘다. 양의 loop 시간을 보장할 수
  없으면 draft를 바꾸기 전에 거부한다.
- split source가 소유하는 counter Pattern은 legacy compatibility row나 reference-only reaction layer와
  동시에 gameplay authority를 소유하지 않는다. active split owner가 Stage, `counterProxy`,
  `boss.flag.counterable` ENTER/EXIT, `COUNTER_HIT` 성공 branch와 `TIMEOUT` 실패 branch를 함께 소유하며,
  retired Pattern ID는 dangling 0건이어야 한다. Core rotation Pattern을 Details에 보이게 하려고
  `manualAuditions`나 DERIVED row로 승격하지 않는다. manual inventory admission과 counter gameplay
  ownership은 별도 계약이다.
- combat-object `spawnSchedule.firstOffsetMs`는 source에서 Product의 `firstSpawnOffsetMs`, publisher/
  bootstrap, Server scheduler, Client local preview와 Composition timeline까지 손실 없이 전달한다.
  offset이 0이면 첫 spawn을 즉시 소비하고, 0보다 크면 emitted count를 0에서 시작해 Stage-local
  clock이 offset에 도달하기 전에는 생성·표시하지 않는다. `StageStart + firstOffsetMs`가 authored
  world time이며 count/interval만 보존하고 offset을 0으로 기본화하지 않는다.
- combat-object visual이 `effectV2Group`과 V1 `effectAssetId/hitEffectAssetId`를 함께 가지면 V2는
  Product live owner이고 V1은 editor/legacy preview fallback이다. live에서 둘을 동시에 spawn해
  폭발을 이중 재생하지 않되, Server semantic presentation event는 Sound cue를 계속 구동한다. 하나의
  V2 group을 여러 combat-object archetype이 참조하는 것은 정상 재사용이며 group definition ID의
  중복과 visual reference 재사용을 같은 오류로 취급하지 않는다. Workbench는 선택된 live group과
  object-local lifetime/offset을 표시하고 fallback을 Product owner처럼 라벨링하지 않는다.
- `Client/Bin/Resources`는 Git/LFS 정본이 아니라 팀장 Drive가 전달하는 물리 runtime 입력이다.
  Ghost Valtan은 catalog에 Resources-relative `Character/Valtan/Ghost/MN_RPBF_02.wmodel`과
  `MN_RPBF_02_AnimSet.wmodel`만 저장하고, 두 WModel과 참조 DDS closure는 같은 물리 경로로 수동
  전달한다. worktree 삭제, clone, `git lfs pull`로 이 파일들이 복구된다고 가정하거나 force-add하지
  않는다. 누락 시 model-view/FullDiagnostic 차단을 코드 회귀와 구분하고 Drive 전달 prerequisite와
  exact 상대 경로를 보고한다.
- 임시 worktree, fixture, baseline 비교 폴더 안에 실제 `Client/Bin/Resources`를 가리키는 junction이나
  symlink를 만들지 않는다. `git worktree remove --force`, `Remove-Item -Recurse`, `rm -rf`는 연결된
  Git 비추적 Drive 폴더까지 순회해 원본 pack을 지울 수 있다. 별도 tree에서 물리 Resources가 필요하면
  `LOSTARK_RESOURCE_ROOT`/`LOSTARK_SHARED_ASSET_ROOT`로 실제 루트를 읽기 전용 지정하거나 검사에 필요한
  최소 파일만 복사한다. worktree 삭제 전에는 reparse point를 전수 확인하고, 발견된 link는 target을
  순회하지 않는 unlink 명령으로 먼저 분리한다.
- `Client/Bin/Resources` 자체, 그 하위 일곱 폴더, 그리고 이를 포함하는 상위 폴더는 어떤 정리·복원 요청에서도
  `rm -rf`, `Remove-Item -Recurse`, `git clean -x`로 지우지 않는다. 정리 대상은 `.vs`, `EngineSDK`, `out`,
  `x64`, 구성별 Bin 산출물처럼 재생성 가능한 폴더로 한정하고, 삭제 명령의 경로가 Resources를 포함하는지
  실행 전에 경로를 출력해 확인한다. 2026-09-03에 실제로 Drive pack 전체가 삭제된 사고가 있었다.
- 이 절의 source/Product validate, native harness, Product/Core/FullDiagnostic PASS는 화면 품질 PASS가
  아니다. Effect 반복·위치·색, collider wire, Ghost body와 Sound timing의 최종 판정은 0절의 사용자
  전용 경계를 그대로 적용하며, 사용자의 서면 관찰 전에는 first pixel, eye smoke, visual PASS를
  기록하지 않는다.
- encounter Stage에 `verticalOffsetM` 같은 optional field를 추가할 때 정본 gameplay publisher만
  고치면 끝나지 않는다. 같은 `ValtanEncounter.json`을 strict하게 다시 읽는
  `Publish-ValtanWorldDestruction.ps1` 같은 보조 publisher도 exact property set과 의미 검증을 같은
  변경 단위에서 갱신해야 한다. 그렇지 않으면 focused/source validation은 모두 통과해도 Product
  pre-build에서 `missing or unknown fields`로 중단된다. 보조 reader에서 단순히 unknown field를
  무시하지 말고 정본과 동일하게 finite/range, non-zero, active `bossResponse`, pattern/stage motion
  배타 조건을 검증하며, 이 consumer를 포함한 negative 회귀와 Product build를 완료 증거로 남긴다.
- 새 Client translation unit을 추가하면 `.vcxproj`/`.filters`뿐 아니라 Product build가 고정하는 native
  source inventory oracle도 함께 갱신한다. 실제 컴파일 오류와 stale exact-count/source-list 실패를
  구분하고, inventory 기대값을 약화하거나 새 파일을 빌드에서 빼서 통과시키지 않는다.

### Pattern/Effect schema evolution은 consumer closure matrix로 닫는다

`Stage.verticalOffsetM` 추가 뒤 실제 Client에서 확인된 연쇄 실패는 데이터 한 줄의 오류가 아니었다.
Python source validator와 projector는 새 필드를 승인했지만 Client split reader는 필수 nullable
`motion:null`을 active motion으로 오판했고, 그 오류를 고친 뒤에는 generated Product의
`SPAWN_COMBAT_OBJECT_VOLLEY.firstSpawnOffsetMs`를 모르는 read-only encounter fallback이 드러났다.
다시 그 오류를 고치자 publisher/Server만 알던 `SUPPRESS_INTER_STEP_PURSUIT`와 삼각 portal의 exact
3-point 규칙이 다음 Client reader drift로 나타났다. strict admission은 첫 실패에서 멈추므로 앞의
오류 하나가 뒤의 모든 누락을 가린다.

본질적인 원리는 다음과 같다.

- 새 field/event/effect parameter는 한 JSON 객체의 optional key가 아니라
  `source -> join/project -> generated Product -> secondary publisher -> Client source reader ->
  Client Product fallback -> bootstrap/version -> Server runtime -> preview/tool` 전체의 schema evolution이다.
- exact-property fail-close consumer는 서로 다른 목적의 로컬 schema를 가진다. primary publisher PASS나
  generated JSON diff만으로 다른 native reader의 admission을 증명할 수 없다.
- optional value는 `absent`, `present null`, `present value`를 구분한다. nullable 필드의 포인터 존재만으로
  기능이 활성화됐다고 판정하지 않는다. `motion:null`은 no motion이고 non-null object만 active motion이다.
- 시간 field는 저장/parse만의 계약이 아니다. `firstSpawnOffsetMs`는 Server spawn clock, Client preview,
  Composition과 Animation Tool lane 시작 시각까지 같은 Stage-local clock을 사용해야 한다. UI에서 0으로
  되돌리면 데이터는 맞아도 저작자가 잘못된 타임라인을 보게 된다.
- 새 필드를 실제로 소비하는 reader와 실행 경로만 확인한다. Animation/Effect 표현 수정 때문에
  관계없는 PatternTree, 보스, family의 검증을 함께 요구하지 않는다.

아래 표는 원인 조사 시 사용하는 소비 경계 참고표다. 작업마다 모든 행을 수행하거나 `N/A` 문서를
만드는 절차가 아니다. 변경한 값이 저장된 뒤 해당 아레나의 의도한 실행에 도달하는 경로를 확인한다.

| 경계 | 관련된 경우 확인할 내용 |
|---|---|
| owner/writer/helper | stable owner ID, required/optional/null 의미, 단위와 clock, save/reload/CAS rollback |
| source schema | exact key set, type/range, cross-field owner·배타 조건, generator 재실행 시 보존 |
| join/project | source 의미를 generated Product의 exact field/row로 무손실 투영하고 provenance/revision 동기화 |
| secondary publisher | 같은 Encounter/Effect 문서를 strict하게 읽는 모든 publisher의 key set과 의미 검증 동기화 |
| Client strict source reader | 현재 split source 전체를 실제 C++ reader로 읽고 in-memory view에 보존 또는 의도적 validate-and-discard |
| Client Product fallback | freshly projected Product 전체를 별도 fixture로 읽고 source reader와 같은 truth table 유지 |
| bootstrap/admission | row layout 변경 시 format version, generation admission, exact field count를 같은 변경에서 갱신 |
| Server | catalog parser, fixed-tick 실행, terminal/restore/rollback, late join snapshot 의미 확인 |
| presentation/tools | live runtime, local preview, Workbench/Animation/Effect Tool timeline·label·selection에서 동일 단위/offset 사용 |
| tests | positive survival, wrong type/range, missing owner, conflicting field, wrong trigger/stage, last-good 보존 |

Effect V2의 slot/parameter/group timing 변경은 해당 occurrence의 실제 저장값과 실행 시간을 확인한다.
문제가 재현되면 그 occurrence의 Document/Catalog/Runtime 경로를 따라가며 필요한 작은 검사만 사용한다.
전체 catalog·Valtan·WorldDestruction 진단을 매 수정의 완료 조건으로 연결하지 않는다.

오류 메시지는 `v4 field is invalid` 하나로 motion/action/branch 실패를 뭉개지 않는다. 최소
`document kind + patternId + stageId + field/action family + 위반 predicate`를 남기고, 실패 시 이전 admitted
tree/reference를 보존한다. 지원하지 않는 실행 항목은 정상값으로 위장하지 않고 그 항목에 오류를
표시한다. 오류 항목 때문에 다른 정상 패턴·family의 목록, 편집 또는 재생 상태를 초기화하지 않는다.

### Effect 세대를 한 화면에 합칠 때 backend catalog를 다시 직접 순회하지 않는다

V1 authored 문서는 `elements[]` 전체가 하나의 원자적 composition이고, V2는 leaf와 ordered group을
분리해 저장한다. 두 저장 형식을 하나로 보이게 한다는 이유로 V1 element를 V2 leaf처럼 펼치거나,
같은 V1 문서를 배치 수만큼 복제하지 않는다. 공용 authoring resource 계약은
`EFFECT_RESOURCE_KEY { ownerKind, stableId }`와 immutable `CEffectResourceCatalog` snapshot이며,
`V1_DOCUMENT`와 `V2_GROUP`은 같은 `Groups`, `V2_LEAF`는 `Leaves`에 표시한다. 이는 무손실 group
승격이지 V1 JSON을 불완전한 V2 field로 변환하는 migration이 아니다.

- All Effects는 `CEffectResourceCatalog` facade의 owner-kind과 stable ID를 소비해 V1/V2를 한
  화면에 표시한다. Action Composition Workbench는 현재 `V1 Pattern Effects`,
  `V2 Authored Effects`, `V2 Effect Groups`를 각각 명시적 owner lane으로 유지한다. 단일 writer가
  없는데 facade snapshot을 공유한다고 기록하거나 backend을 혼합하지 않는다.
- 선택 identity는 해당 backend의 owner kind와 stable ID를 함께 보존한다. V1 document append는
  기존 exact clip cue writer로, V2 leaf/group append는 typed stage binding writer로 dispatch한다.
  현재 코드에는 V1/V2/Camera/Catalog를 하나로 묶는 canonical mutation coordinator가 없으므로,
  이를 기존 저장 경로의 완료 계약으로 가정하지 않는다.
- owner refresh 또는 facade join이 실패하면 이전 snapshot은 표시용으로 유지하되 append/save는
  `STALE PRESERVED / READ ONLY`로 막는다. 실패한 새 owner와 이전 다른 owner를 섞어 새 snapshot처럼
  표시하지 않는다.
- Ground Roar 4방향 배치는 V1 active/explode 문서를 24/4 element로 복사하는 문제가 아니다. active
  6개와 explode 1개인 원본 atomic group을 유지하고 Server combat-object volley가 boss-relative
  `radiusM=4.9497475`, `startAngleDegrees=45`, `angleStepDegrees=90`의 root 네 개를 만든다.
  boss yaw 0도 기준 각 root는 X/Z `(3.5,3.5)`, `(3.5,-3.5)`, `(-3.5,-3.5)`,
  `(-3.5,3.5)`이며 boss yaw를 따라 함께 회전한다. element 복제와 root instancing을 동시에 적용하면
  16배 occurrence가 생기므로 회귀가 두 계약을 함께 검사해야 한다.
- 피해 없는 boss-relative four-rock owner를 새로 추가할 때는 `GameRoom`의 off-navigation exact owner
  집합도 같은 변경에서 갱신한다. `FIXED_AREA + direction NONE + hits=[] + presentation pulse + count=4`
  의미 조건과 `(combatObject, pattern, action)` 튜플을 모두 만족할 때만 authored root가 navgrid 밖에
  놓이는 것을 허용한다. 모든 visual object를 포괄 허용하거나 좌표를 project/clamp하지 않는다.
- `firstSpawnOffsetMs`가 있는 volley는 ENTER 테스트만으로 검증되지 않는다. 실제
  `Apply_BossPatternScheduledSpawnWave`에서 due 직전 no-op, due tick atomic spawn, 다음 tick 중복 없음과
  damage hit 주입 시 live/pending lifecycle 0개인 strict reject를 함께 검사한다. 이 경로의 실패는
  room을 not-ready로 만들고 다음 정상 입력에서 session FIN으로 이어져 Client에는
  `Valtan replication observed a disconnected Server session.`, Lobby에는 공통
  `Server entry failed.`로 보일 수 있다.

Save/Restart 진단에서는 한 문장인 `SAVED`를 다음 상태로 나눠 확인한다.

```text
SOURCE_COMMITTED -> EDITOR_REOPENED -> CANDIDATE_PUBLISHED
                 -> APPLY_PENDING -> SERVER_ACTIVE -> FLOW_RESTART_ADMITTED
```

- 앞 단계 성공은 뒤 단계 성공을 뜻하지 않는다. source commit 성공 뒤 reopen 실패라면 source를 다시
  쓰지 않고 post-commit retry를 제공한다.
- `UNCONFIRMED`는 실패도 성공도 아니다. exact Server-active revision 관측 전에는 다음 revision을
  제출하지 않는다.
- Restart는 saved Flow content, latest candidate, Server-active Product revision, presentation generation을
  각각 exact 비교한다. 버튼을 억지로 활성화하거나 이전 candidate로 fallback하지 않는다.

`Restart Pattern`과 `Restart Flow`는 같은 명령이 아니다. 과거 Boss Verification의
`Restart Saved Pattern (Fresh Arena)`는 내부에서 `Restart_SavedFlow(true)`를 호출했기 때문에,
saved slot이 하나일 때는 실제로 `Restart Flow`와 완전히 같은 Flow packet과 arena reset을 사용했다.
이 one-slot alias가 두 기능을 같은 것으로 보이게 만든 원인이므로 다시 만들지 않는다.

| Tool 명령 | wire/runtime | reset 범위 | 재생 범위 |
|---|---|---|---|
| `Play Selected Pattern (Keep Arena)` | `PLAY_PATTERN_ID` | Valtan boss-only reset. 현재 wall/floor/prop/collision/Nav를 유지하고 교체되는 boss-source combat object만 취소하며 player-source object는 유지 | 선택한 Pattern 하나를 첫 Stage부터 재생. saved Flow, Next, Wait는 소비하지 않음 |
| `Restart Active Pattern (Keep Arena)` | `RESTART_PATTERN_ID` exact predecessor CAS | 같은 boss-only reset. 현재 arena를 유지하고 교체되는 boss-source combat object만 취소 | 이 Tool이 소유한 exact ACTIVE/COMPLETED Pattern occurrence 하나를 첫 Stage부터 교체 재생 |
| `Restart Saved Flow (Fresh Arena)` | `C2S_DEBUG_VALTAN_PATTERN_FLOW_START` | world destruction과 encounter prop을 포함한 authoritative arena reset | disk의 전체 saved `scriptedSequence`를 Pattern 01부터 시작하고 saved order/Next/Wait를 끝까지 소비 |

따라서 single Pattern 버튼에 `Fresh Arena`를 쓰거나, Flow 버튼을 `Pattern Restart`라고 부르지 않는다.
Server 회귀에서는 Pattern ID branch가 `Reset_ValtanBossOnlyAuditionState`만 호출하고
`Reset_ValtanAuditionState`를 호출하지 않는지, Flow start branch는 destruction/prop preflight 뒤
`Reset_ValtanAuditionState`를 호출하는지를 함께 고정한다.

목록/Save/Restart/Arena 실행을 바꿨다면 새 Debug EXE에서 사용자가 변경한 경로를 확인한다.
저장한 animation occurrence가 해당 패턴에 반영되는지, 기존 정상 항목이 유지되는지를 본다.
다른 도구의 목록 개수, 전체 native harness, 전 보스 publisher 결과를 매 수정의 필수 조건으로 요구하지 않는다.
목록 표시와 실제 재생은 구분해 보고하고, 실행하지 않은 동작은 확인했다고 기록하지 않는다.

### `serverMotion`의 takeoff stage는 이름이 아니라 ordered entry다

- `takeoffStartMs/takeoffEndMs`의 소유자는 `stageId == "TAKEOFF"`가 아니라 `entryActionId`와 일치하는
  첫 ordered stage다. `travelStageId`는 그보다 뒤의 고유 stable stage여야 한다.
- `VALTAN_SIX_PIZZA_106`의 첫 stage는 정본 `STEP_01`이다. validator를 통과시키려고 이를 `TAKEOFF`로
  개명하면 Effect, Camera, Product occurrence join을 함께 깨뜨린다.
- 한 pattern의 strict join 실패로 전체 All Effects inventory가 rollback되는 것은 정상 fail-close다. partial
  inventory나 legacy fallback으로 숨기지 말고 오류에 `patternId`와 실제 stage/action identity를 남긴다.

### Map Effect catalog 등록과 entry-required Product 준비는 다른 gate다

- catalog 행, authored Effect 파일, `.mapeffects.json` world row가 모두 있어도 portable `sourceRecipe` codec
  admission 또는 prepared Product commit이 실패할 수 있다. catalog 재등록만 반복하지 않는다.
- entry-required Map Effect에 실패도 `settled`로 보는 optional prewarm 정책을 적용하면 Level activation 뒤
  `Map Effect world target is absent...`라는 후속 증상만 남는다. 최초 `incremental prewarm failed`의 asset ID와
  codec 오류를 Level 진입 실패 원인으로 보존한다.
- 일반 Particle source recipe는 Required 1개, Lifetime 1개 이상, Spawn 1개를 요구한다.
  `particlemodulecolorscaleoverlife`는 color와 alpha distribution을 모두 가져야 한다. validator 약화나
  `sourceRecipe.enabled=false`로 입장을 통과시키지 않는다.

### 선언/정의가 일치하는 `LNK2019`는 실제 provider obj를 확인한다

1. 같은 working tree의 다른 MSBuild, CL, FXC, linker와 publisher를 먼저 멈춘다.
2. 선택한 `Configuration|Platform`의 evaluated `IntDir/OutDir`를 확인한다.
3. 참조하는 consumer obj가 아니라 provider obj에 `dumpbin /symbols`로 정의 심볼이 있는지 검사한다.
4. 심볼이 없으면 해당 translation unit만 강제 재컴파일한 뒤 최종 link를 한 번 수행한다. broad output 삭제나
   두 번째 전체 빌드를 겹치면 `.tlog` 잠금과 서로 다른 시점의 obj 혼합을 만든다.
5. `LNK1104`, `MSB3021`, `MSB3027`이 EXE/DLL을 가리키면 실행 중 출력물 잠금이다. compile 성공과 link
   차단을 분리하고, 현재 실행 중인 EXE는 새 obj가 반영되지 않은 이전 바이너리라고 보고한다.

### publisher의 `exit code 1`은 최초 오류가 아니다

- MSBuild가 출력한 마지막 `명령이 종료되었습니다(코드: 1)`은 wrapper 결과다. 그 앞의 첫 terminating
  error에서 domain, phase, stable ID와 path를 확보한다.
- `Publish-GameplayBalance -Mode Publish`는 destination 교체 전에 Valtan strict validation도 실행한다.
  먼저 `-Mode Validate`로 source/schema/join 실패를 분리하고 통과한 뒤에만 destination lock, promotion,
  rollback을 조사한다.
- Server contract-test의 기대값 실패는 pre-build publisher 실패와 별도 단계다. 생성 bootstrap 직접 수정,
  validation skip, 실행 중 Server/Client 위에 publisher/build 반복 실행으로 숨기지 않는다.

### All Effects의 `Delete Effect`는 소유권에 따라 의미가 다르다

- `[PRODUCT]` 행 삭제는 선택한 Pattern의 exact cue 연결만 split `Valtan.presentation.json`에서 제거하고
  candidate source CAS 뒤 Product를 투영한 다음 단일 `Validate` postcondition을 실행한다. 이전 Product parity를
  source Save 전에 요구하면 새 source와 이전 Product의 정상 drift를 실패로 오인한다. 공유 `EffectCatalog` 행과 authored Effect 파일, 다른 Pattern
  연결은 보존한다.
- `DRAFT_ATTACHED`만 sidecar row와 deterministic `Effects/Authored/<effectId>.effect.json` 파일을 함께
  삭제할 수 있다. Product catalog/cue 참조가 있으면 파일 삭제를 거부한다.
- Draft 삭제는 sidecar CAS를 먼저 commit하고 파일을 같은 handle에서 canonical compare-delete한다. 파일
  삭제가 실패하면 sidecar를 CAS rollback한다. unsaved 편집이 있거나 선택 이후 cue/baseline이 바뀌면 삭제하지 않는다.
- generated `Valtan.patterneffectcues.json`을 직접 편집하지 않는다. Product 연결 변경의 정본은 split
  presentation이고 Server 재생 판정은 publish 후 Server 재시작·Arena 재진입 뒤에 한다.

#### 삭제 직전에는 캐시가 아니라 정본을 다시 잠그고 읽는다

- `CEffectCatalog::Find()`나 현재 화면의 Pattern tree는 표시용 snapshot이다. 사용자가 확인 modal을 보는 동안
  다른 publisher가 Effect를 Product에 등록할 수 있으므로, 이 캐시만 보고 authored 파일을 삭제하면 안 된다.
- Draft 생성·삭제의 destructive preflight는 Effect catalog, split gameplay/presentation, Encounter/rotation,
  animation binding/cue/alias/stage-Effect, BossCatalog/combat-object까지 `CValtanPatternTree`가 소비하는 complete
  source read set을 read handle로 열어 concurrent write/delete를 막은 상태에서 catalog와 Product graph를 새로
  parse한다. 어느 하나라도 parse/lock에 실패하면 파일을 보존하고 Refresh를 요구한다.
- modal을 열 때 복사한 `kind + patternId + effectAssetId + cueIds + alias`와 확인 순간의 선택이 하나라도 다르면
  삭제하지 않는다. render-frame vector pointer가 아니라 stable identity만 확인 대상으로 보존한다.

#### publisher를 소유한 child process는 timeout으로 죽이지 않는다

- Product unlink는 source commit 뒤 원자적 Product projection + `Validate` postcondition과 실패 시 source/Product rollback을
  수행한다. managed cue scale-policy migration 표는 현재 cue의 허용 정책 ledger이며 live cue 전체 개수를
  고정하지 않는다. sealed legacy Effect cue도 전역 배열 ordinal이 아니라 stable `bindingId`로 검증한다. 이 child를 180초
  timeout에서 `TerminateProcess`하면 PowerShell의 catch/finally가 실행되지 않아 source/Product가 반쪽 상태로
  남을 수 있다.
- Effect Tool은 unlink process를 비동기로 시작하고 매 frame exit만 poll한다. 180초는 경고 기준일 뿐 종료 기준이
  아니다. 작업 중에는 All Effects 편집을 잠그고, Client가 닫혀도 process handle만 닫아 child가 commit 또는
  rollback을 끝내게 한다.
- exit 0에서만 unlink 성공으로 표시한다. nonzero나 process observation 실패에서는 disk를 다시 읽되 보존 여부를
  추측하지 않고 최초 publisher 오류를 확인하게 한다. child가 아직 실행 중일 수 있는 observation failure에서는
  All Effects를 다시 열어 두지 않는다. 잠금을 유지해 두 번째 publisher transaction이 겹치는 것을 막는다.

#### atomic replace backup은 post-commit 검증 뒤에만 지운다

- `File.Replace`가 성공한 직후 실행되는 byte verification도 실패할 수 있다. replace 완료 flag를 verification 뒤에
  세우거나 backup을 `finally`에서 무조건 지우면 이미 바뀐 source를 baseline으로 복구할 수 없다.
- replace 직후 commit flag와 recovery backup path를 먼저 기록하고, replacement bytes를 다시 확인한 뒤에만 backup을
  삭제한다. post-replace 실패는 보존한 backup/CAS rollback으로 baseline을 복원하며, 복구까지 실패하면 backup의
  정확한 경로를 오류에 남긴다.

### Model View target 교체와 Create auto-open은 저장 transaction과 분리한다

- Character Select의 Model View가 같은 `Valtan` asset을 다시 publish해도 target generation은 바뀔 수 있다.
  asset 이름과 포인터만 비교하면 synchronized sequence가 generic update에서 비워진 뒤 다시 stage되지 않는다.
- Pattern Draft는 매 frame generic synchronized update보다 먼저 generation mismatch를 확인하고, 동일 Pattern의
  ordered timeline을 새 generation에 다시 stage한다. target 교체를 문서 unload나 빈 sequence의 정상 종료로
  처리하지 않는다.
- `Create Effect`의 durable 성공 경계는 authored Effect 파일과 `DRAFT_ATTACHED` sidecar가 모두 CAS commit된
  시점이다. 그 뒤 Model View 준비나 auto-open이 실패해도 두 파일을 rollback하지 않는다. `files remain
  committed`와 preview 실패 원인을 분리해 보고하고, 사용자가 `Open Editor`로 재시도할 수 있게 한다.
- 실행 중 `Client.exe`는 이 새 source를 반영하지 않은 이전 바이너리다. 사용자의 tuning 세션을 강제 종료하거나
  그 위에 link하지 말고, 종료 신호 뒤 한 번 재빌드한 새 EXE에서 target replacement와 Delete UI를 확인한다.

### 오래된 worktree의 PatternTree 전체 파일로 All Effects를 덮어쓰지 않는다

- `피자 패턴 바닥 이펙트 가이드` 작업에서 확인된 회귀처럼, 최신 strict join 위에 오래된 worktree 파일을
  통째로 덮으면 개별 Effect 문제가 아니라 Valtan tree 전체 admission 실패로 나타난다.
- `Boss Tool`이나 Pattern Flow를 추가할 때 과거 계획서·stash·별도 worktree의
  `ValtanPatternTree.h/.cpp` 또는 `Effect_Tool.cpp` All Effects 본문을 전체 복사하지 않는다. 현재 working copy의
  strict join은 누적 계약이며 `partDamagePolicy`, `counterProxy`와 transactional previous-tree 보존 중 하나라도
  사라지면 `F1 -> Effect Tool -> All Effects -> Valtan` tree 전체가 fail-close로 비어 보일 수 있다.
- 작업 전후 no-touch diff를 비교하고 다음 focused contract를 함께 실행한다.
  `python -m unittest Tools.ValtanPipeline.test_valtan_pattern_tree_contract Tools.EffectPipeline.test_effect_tool_valtan_all_effects_contract`
- reload 실패는 기존 admitted tree를 지우지 말고 exact parse/join 오류를 표시한다. 자동 검증 뒤에도 사용자가 새
  Client에서 All Effects의 Valtan 28개 Pattern과 Stage tree가 실제로 열리는지 확인해야 visual PASS다.

### gameplay source가 없는 encounter에 공용 Dataset/runtime부터 만들지 않는다

- 모델, animation chain, 추출 asset 폴더가 존재하는 것과 Server-authoritative encounter source/Product가
  존재하는 것은 다른 계약이다. 현재 Valtan은 기존 canonical source/Product 경로와
  Tool/Server의 직접 reader를 소비하며, 별도 `ENCOUNTER_DATASET` registry나 공용
  `BossPatternGraphRuntime`을 완료 계약으로 두지 않는다.
- `KAKULSAYDON`은 public logical ID이고
  `KoukuSaton`은 실제 animation/resource 저장 alias이므로 spelling을 통일한다는 이유로 raw asset을 바꾸거나,
  존재하지 않는 Kakul gameplay source/Product 경로를 catalog에 추가하지 않는다.
- 향후 공용 Dataset/runtime을 도입하려면 새 encounter source/Product를 먼저 publish한 뒤
  실제 두 소비자 이상을 한 변경 단위에 이전한다. invalid absolute/`..` path, identity mismatch,
  missing/duplicate stage, action+pattern dual branch target, follow-up depth 32 경계를 같은 변경 단위의 native
  contract와 structural oracle로 닫는다. registry/helper만 먼저 만들면 Tool 목록은 생겨도 Save/Restart가 별도 정본을
  참조하는 두 번째 경로가 다시 만들어진다.

### Kouku Scene Profile의 blendMs에 Effect 페이드 길이 검증을 적용하지 않는다

- Composition의 Scene Profile `blendMs` 저장 범위는 `0..600000`이고 해당 box의 `durationMs`와 독립적이다.
  기존 Product는 이 예약 메타데이터를 `SCENE_PROFILE.fadeInMs`로 투영한다. Scene Profile은 현재
  profile의 exposure, bloom, map-light multiplier를 즉시 적용하며 blendMs는 어두움의 강도가 아니다.
- 진짜 세이튼 찾기의 `durationMs=24127`, `blendMs=600000` 저장본에 일반 Effect의
  `fadeInMs <= durationMs` 검사를 적용하면 Client Product 전체 staging이 실패한다. 그 결과 무력화의
  정상 Effect 연결까지 함께 재생되지 않는다. Effect 파일이나 occurrence가 삭제된 현상과 구분한다.
- Client reader는 `SCENE_PROFILE`의 blend 메타데이터만 canonical과 같은 `0..600000` 범위를 허용한다.
  일반 Effect는 occurrence 길이와 fade-in/out 합계 검사를 유지한다. 손상된 Scene Profile row는
  해당 pattern/occurrence와 오류를 표시하고 그 row만 격리해 정상 Effect를 계속 로드한다.
- 검증에는 현재 저장본의 600000 투영 보존, 600001 거부, 일반 Effect의 duration 초과 fade 거부를 함께
  확인한다. Scene Profile의 어두움은 multiplier로 조절하며 큰 blendMs를 삭제하거나 임의 축소해 우회하지 않는다.

### 독립 Effect 편집에 전체 보스 graph admission과 live preview를 요구하지 않는다

- 새 Workbench의 Tree/목록은 metadata만 읽고 손상 항목을 따로 표시한다. 선택 문서 parse, 선택 closure의 Play stage, CPU Save와 domain Product publish의 검증 경계를 구분한다. 목록·Save 성공을 Valtan 전체 Reload_BossValtan으로 가로막지 않는다.
- 새 문서 Create는 clock·Solo/Mute·모델 참고·anchor history를 초기화한다. 공용 Resource UI를 재사용할 때 기존 Effect Tool의 type/slot/bindings를 저장·복구해야 미저장 슬롯을 바꾸지 않는다.
- 기존 V2 leaf editor도 Load 때 표시 이름과 실제 source bytes를 보관한다. 새 Workbench에서 저장한 leaf를 예전 preview로 Save하면 이름을 지우거나 최신 파일을 덮을 수 있으므로 파일 기준본이 바뀌면 거부한다. 이름이 같은 것과 bytes가 같은 것은 별개다.
- Revert는 재로드를 임시 session에서 성공한 뒤 교체한다. saved leaf가 삭제되거나 손상됐다는 이유로 현재 dirty draft부터 Reset하지 않는다. view에서 world anchor를 capture한 경우도 Dirty로 표시한다.
- 모델 참고 actor는 root motion이 꺼진 고정 root이므로 첫 비영 시점 Play를 위해 알려진 0초 root를 기록할 수 있다. 실제 Product에서 과거 Server 위치를 같은 방식으로 만들어 넣으면 안 된다. Product는 실제로 기록한 root history를 사용하고 누락·불연속 구간은 해당 Effect 실패로 격리한다.
- 외부 Sequencer clock의 group은 일반 Runtime Advance에서 중복 증가하지 않는다. 1ms emitter와 1.7초 입자 수명은 별개다. Deactivate 시 고정 step 잔여 구간의 마지막 방출을 처리하고, timeline 끝의 잔여 수명 만료도 요청한 age로 반영한다. group/box 끝을 늘려 방출 횟수를 임의로 늘리지 않는다.

### 패턴 Camera 복귀는 저장한 플레이어 시작 좌표로 돌아가지 않는다

- Camera shot은 목표 eye/lookAt/FOV와 진입·유지·복귀 시간을 저장한다. 시작 pose는 재생 시 취득하고 복귀 목표는 이동 중인 player follow pose를 매 프레임 계산한다. 복귀 끝에서 예전 override 이전 pose를 복원하면 마지막 프레임에 튄다.
- Authoring Preview가 저장한 shot을 바로 읽는 것과 Complete Play가 published shot을 읽는 것을 구분한다. Map publish 후 새 run은 runtime shot snapshot을 갱신해야 한다. Camera overlap 검사는 visible box뿐 아니라 복귀 tail을 포함한다.
- 진짜 세이튼과 같은 Spot Light를 가짜에 적용할 때 Server owner boss ID와 같은 archetype으로 대상을 제한한다. 이미 같은 asset을 직접 재생 중인 가짜에 중복 light를 만들지 않으며 despawn/row 종료 때 follower handle을 정리한다.

### Sequencer 카메라 조회에서 실패한 문서를 매 프레임 다시 파싱하지 않는다

- 카메라 shot/keyframe 수가 늘 때 byte 한도뿐 아니라 JSON value 한도도 실제 전체 문서로 검사한다.
  publisher가 받은 문서를 Client만 낮은 value 한도로 거부하면 컷신이 follow 시점에 머물 수 있다.
- Timeline의 길이·row·복귀 tail 조회는 같은 카메라를 여러 번 찾는다. 실패한 최초 저작 로드는 Level이
  기억하고, `Composition Camera → Reload Cameras`로만 재시도한다. 재로드 실패는 이전 shot과 baseline을 보존한다.
- 입력 파일의 실제 parser 성공, 반복 Ensure에서 read/parse 추가 호출이 없는지, 사용자 FPS 측정은 구분한다.
  재현·검증은 [Sequencer 카메라 로드 결과](09-12/2026-09-12_SEQUENCER_CAMERA_LOAD_PERFORMANCE_RESULT.md)를 따른다.

### ImGui root 위젯 ID에 빈 draft의 표시 이름을 그대로 쓰지 않는다

- Effect Composition Workbench를 처음 열면 group name과 stable ID가 모두 비어 있다. 이 값으로 `Selectable("")`를 그리면 Timeline child window의 root ID와 충돌해 `Cannot have an empty ID at the root of a window` assertion이 발생한다.
- 빈 draft는 생성/열기 안내를 그리고 반환한다. 정상 부모 row에는 `###EffectCompositionGroup`처럼 표시 이름과 독립된 위젯 ID를 사용한다. 손상·삭제된 Pattern을 참조하는 Character anchor도 표시 이름이 비어 있을 수 있으므로 member ID scope와 `###AnchorMember`를 사용한다.
- 컴파일과 문서 parse만으로 첫 창 렌더의 ImGui assertion을 검증했다고 기록하지 않는다. Client 창 재열기 확인은 사용자가 직접 한다.

### Timeline lane과 per-lane cache의 크기를 함께 유지한다

- 새 lane을 render order에만 추가하면 고정 크기의 cache가 남아 첫 timeline 렌더에서 범위를 벗어난다. `TIMELINE_LANE::COUNT`로 cache 크기를 정하고 표시 순서의 실제 항목 수도 static_assert로 대조한다. cache는 표시 ordinal 대신 lane 값으로 조회하며 `Pack_TimelineSubrows`도 같은 크기를 소비한다.
- Valtan Workbench의 WORLD index7 / cache7 결함은 빈 draft 위젯 ID 오류와 별개다. assertion을 끄거나 WORLD를 생략해 숨기지 않는다. 실제 packing 함수의 8-lane·겹침·빈 lane 검사와 focused compile은 사용자 창 재열기 검증과 구분한다.

### Effect panel 재사용 시 저장·미리보기 owner를 구분한다

- Parent/tree 저장과 Effect body 저장은 다른 owner다. native V2 leaf Open을 항상 group으로 감싸 Save하면 뜻하지 않은 group 원본이 생긴다. native leaf 저장은 같은 ID/파일을 유지하고 group 확장은 명시적인 생성 명령으로만 한다.
- 현재 draft의 preview 검증이 실패했을 때 saved 문서로 fallback하면 잘못된 편집 내용 대신 이전 Effect가 재생된다. 현재 draft가 소유한 key의 실패는 그대로 표시하고 snapshot 교체를 취소한다.
- Play All/Family/Element는 preview이고 Append만 저장 sequence occurrence를 만든다. Preview 버튼 처리에 Append를 재사용하면 Play할 때마다 저장 행이 쌓인다.
- Patterns by Gate의 Create Parent/Bundle은 메모리 변경이다. 전체 Composition Save 이전에는 EXE 종료 후 보존을 보장하지 않으며 트리 옆에 Saved/Unsaved와 Save를 표시한다.
- Parent의 runtime 전개와 편집 진입은 따로 확인한다. backing timeline이 없는 기존 Parent도 상단 Append Pattern에서 첫 자식과 timeline을 하나의 candidate로 생성해야 하며 실패한 시도는 folder·ordinal·draft를 보존한다. Details에 함수가 연결됐다는 이유만으로 상단 버튼이나 Parent 선택 후 sequencer가 연결됐다고 기록하지 않는다.
- ImGui Rename의 한영 입력은 공통 Win32 IME context를 유지해야 한다. caret callback에서 WantVisible에 따라 context를 분리하지 않는다. 일반 InputText는 확정 WM_CHAR만 표시하므로 OS 조합창이 필요하고, 채팅/닉네임의 직접 그리는 조합 문자열과 구분한다. backend가 DefWindowProc를 호출했다면 처리 완료를 반환해 Client와 분리 viewport에서 기본 처리를 반복하지 않는다.
- World Object model/texture는 기존 Effect domain scan 밖에 있을 수 있다. 저장 Object resource에서 확인한 상대 ID와 file kind를 동일 resource binder에 전달해야 목록만 보이고 Bind가 거부되는 상태를 피할 수 있다.


### Product 빌드와 같은 import library를 읽는 probe 링크를 겹치지 않는다

Windows에서 Engine.lib를 쓰는 제품 링크와 같은 파일을 입력으로 여는 별도 probe 링크가
겹치면 LNK1114/오류5와 같은 공유·접근 실패가 발생할 수 있다. 사용자에게 소스 동결과
빌드 인계를 했으면 Product 빌드뿐 아니라 같은 import library를 읽는 out 검사 compile/link도
중단한다. 원본 로그에 잠금 소유자 정보가 없으면 동시 실행만으로 특정 프로세스를 확정하지 않는다.

Engine 링크 실패는0바이트 Engine.dll을 남길 수 있고, DLL 존재만 검사하는 Client 배포는 그
파일을 복사할 수 있다. EXE 링크 성공만으로 실행 준비 완료라고 판단하지 말고 Engine 원본과
Client 배포 DLL의 유효 크기/PE 형식·일치 여부도 확인한다. 실패 출력은 크기와 정확한 workspace
경로를 확인한 뒤 필요한 파일만 재생성하며, 사용자 Client를 자동 실행하지 않는다.


### 쿠크 컷신의 모델·재질·조명·곡선 연결

- 보스 무기에서 정적 World Object를 만들면 같은 mesh/slot/D/N/S여도 새 modelAssetId에는 원래 catalog의 native 재질이 자동 적용되지 않을 수 있다. 실제 원본 MIC가 같은지 확인해 `materialSourceModelAssetId`를 전달하고 IBL/BRDF까지 검사한다. 재생성 때 이 참조를 버리지 않는다. geometry의 반전 bake는 환경 반사 복구가 아니다. 상세 절차는 복원 V2의 오브젝트 공통 절차를 따른다.
- 같은 mesh와 diffuse가 있어도 움직이는 BG8 모델은 static shader와 다른 skinned shader를 쓴다. 공유 MapMaterialSurface 평가와 모든 바인딩을 실제 skinned draw까지 연결하고, 정적 RNM/static shadow를 움직이는 모델에 복사하지 않는다. UNBAKED receiver는 기존 baked bit로 정적 맵 중복 조명을 제외한다.
- Matinee InterpGroup만 세면 부모에 부착된 맵 소품을 빠뜨린다. source actor의 base/basebonename/relative pose와 component material override까지 조사한다. source transparent override를 범용 diffuse 슬롯으로 표시하지 않는다.
- native Move/Camera를 일정 간격으로만 줄이면 급격한 이동을 놓칠 수 있다. 원본 곡선 대비 위치·회전·FOV 오차를 측정하고 저장 key 상한을 넘으면 연속 resource로 분할한다. Director 컷 수와 저장 resource 수는 다를 수 있다.
- 인접 Effect 구간은 start/end를 runtime float32로 변환한 뒤 duration=end-start로 만든다. start와 double 차이 duration을 따로 변환하면 경계에서 두 광원이 겹칠 수 있다. RGB Hermite는 기존 cubic distribution으로 보존한다.
- 생성형 popup book을 쓰는 Preview는 이전 Deploy7도 보이는지 확인한다. borrowed state와 applied state를 함께 기록하고 Stop에서 현재 상태가 여전히 적용값일 때만 복구한다.
- 정상 맵 애니메이션을 합칠 때 template의 키 길이와 occurrence의 표시 수명을 구분한다. MAP은 원래 마지막 키를 유지하고 OBJECT_RESOURCE는 명시적 HOLD instance를 사용한다. source Matinee의 다른 책 clip·배치·배우를 일부만 섞지 않는다.
- 연출용 MapLight 사본은 기존 provider 문서를 stage/validate한 뒤 소유하고, 실제 WORLD cue 수명으로 활성화한다. WORLD Seek/Stop 이후 같은 프레임에 provider 하나만 제출한다. Stop에서 Renderer에 이미 등록된 provider를 Clear하지 않으며, 같은 포인터의 authoring 문서가 변경될 때도 기존 사본을 무효화한다.
- RenderingProfiles의 JSON number는 C++ reader와 같은 float32 변환·범위 순서로 검사한다. `0.100000001` 같은 9자리 저장값을 double로 확장한 float32 하한과 직접 비교하지 않는다. 벡터 reader의 별도 원문 double 범위와 near/far의 float32 비교는 유지한다.


### 카드 variant의 cooked slot 이름과 native texture identity를 구분한다

- `MN_RHOC_00-1.wmodel`의 slot 이름은 일반 카드와 같은 `mn_rhoc_00_mi`지만 원본 조커 MIC와 D/N/S는 `mn_rhoc_00-1`이다. BossCatalog의 모델별 override를 만들 때 slot 이름만으로 일반 카드의 sourceMaterial·texture를 복사하지 않는다. 실제 modelAssetId, 원본 MIC와 설치 WModel texture를 함께 대조한다.
- program 26은 native expression 1의 diffuse·alpha를 직접 샘플한다. 일반 diffuse override로 바꿔도 잘못된 native slot은 남는다. 조커는 cooked materialName·family·36개 parameter를 유지하고 기존 variant texture 0=N(linear), 1=D(srgb), 2=S(srgb)를 연결한다. JSON·실제 입력 검증과 사용자 화면 확인을 구분하며 [조커 결과](09-12/2026-09-12_KOUKU_JOKER_NATIVE_TEXTURE_RESULT.md)를 따른다.

### v15 trail/ribbon만 있는 projection에 LocalDecal을 강제하지 않는다

- document-owned v15 `ADAPTER_PACKET_V1`은 정상 supplemental-only projection일 수 있다. `Get_AdmittedRows()`의 LocalDecal 수가 0이어도 유효한 `Get_AdmittedSupplementalElements()`가 있으면 준비를 허용하고 실제 typed adapter 수 일치는 계속 검사한다.
- 내려찍기·거미카운터처럼 v15 전체 문서만 미표시이고 v13 연기는 표시되면 크기를 임의 보정하기 전에 catalog/projection → 실제 GPU resource prepare → renderer attach/clone 실패를 확인한다. CPU playback과 단일 shader draw만 통과해도 이 경계에서 전체 effect가 차단될 수 있다.

### 쿠크 독립 Effect와 원본 폭죽 event의 소유자를 구분한다

- 독립 Effect Resource는 원본 `sourceModelPreview`의 actor·clip·Source In 시계를 사용한다. 빈 synthetic Pattern의 Animation만 조회하면 첫 fixed-step에서 source anchor가 실패한다. 실제 모델과 bone/clip을 함께 준비하고 마지막 pose를 입자 tail까지 유지한다. 독립 Preview의 정지 pose 정책을 Product history 누락에 적용하지 않는다. [검증 결과](09-12/2026-09-12_KOUKU_RESOURCE_SOURCE_PREVIEW_RESULT.md)를 따른다.
- root/camera-only Effect의 Play All에 저장된 boss pattern을 요구하지 않는다. 실제 source bone이 필요한 draft만 CNpc/CModel을 준비하고, 독립 재생은 실제 플레이어 root를 임시 캡처한다. 이 과정에서 저장된 sequence의 모델·anchor·dirty 상태나 occurrence를 바꾸지 않는다.
- Action Workbench와 Sequencer Benchmark의 MAP Effect는 고정 월드 위치다. WORLD object 참조나 follow/bone과 혼합하지 않는다. V1 전체 문서도 기존 Append 경로로 같은 anchor를 소비한다.
- `EPET_Death` 폭죽은 로켓 수명 종료 위치·속도로 기존 bounded event queue에 넣는다. 생성0-age와 마지막 부분 step을 수명 끝까지 적분하고 이미 소비한 spawn을 다시 보내지 않는다. 숨은 `ERM_None`는 같은 문서의 location/event 소비자와 원본 출처가 있을 때만 허용한다.
- launch 종료와 후속 폭발 tail은 다르다. rate0·burst없음인 event receiver의 수신창만 원본 event chain으로 계산하고, 입자 수명이나 원본 발사 횟수를 늘려 재생 시간을 확보하지 않는다. reserve budget을 실제 동시 생존 상한으로 검사한다.
- 원본 Action에 ParticleSystem 이름이 없어도 SkillEffect가 Projectile을 거쳐 호출할 수 있다. 사진과 비슷한 Ray/Ready 이름만으로 격자를 단정하지 말고 fixed-area placement·signed Rotator·Timer와 CEF 파라미터를 함께 추적한다.
- native ID를 추가할 때 descriptor/C++ 상한과 Mesh/Particle/Decal/Trail HLSL dispatch 범위를 함께 검사한다. 새 ID가 분기를 지나지 못하면 컴파일에 성공해도 RGB가 0이다.
- 원본 PS의 material 상수만으로 엔진 prefix가 채워지지 않는다. 외부 opacity/color와 MacroUV를 실제 소비 lane까지 연결한다. MacroUV 중심은 현재 ParticleSystem occurrence가 소유하고 world-space 입자의 고정 birth 위치와 다르다. 원본 world radius에는 occurrence scale을 다시 곱하지 않는다.
- 독립 사각형 shader draw의 RGB 0은 시선각/Fresnel/dissolve mask와 Dynamic 입력을 구분해서 진단한다. finite/alpha 성공을 실제 표시 성공으로 대신 기록하거나 임의 alpha 보정으로 덮지 않는다.

### World 표시는 opacity와 재생창을 원본 의미로 구분한다

- `par_b_picking_01`의 mesh particle alpha는 -1~+1 UV 이동 입력이다. 음수 alpha를 일반 opacity로 clamp/cull하면 피킹 표시의 앞부분이 사라진다. 원본 PS의 실제 소비 lane을 먼저 확인한다.
- Required emitterLoops 생략은 UE3 기본 0인 반복일 수 있다. `par_i_movetrack_01`의 9개 반복 emitter와 3개 초기 pulse를 모두 loops1로 바꾸지 않는다. bounded document tail과 Tool의 반복창은 구분하며, 반복 중 일반 Seek의 pause 부작용이나 자원 재준비 때문에 멈추지 않게 한다.
- mesh의 `PSA_TypeSpecific + MeshFaceCameraWithLockedAxis + EPAL_Rotate_Z`는 sprite billboard와 별도 source mode다. 원본 TypeData 회전·preScale·mesh basis를 실측하고 native instance/scalar 양쪽에서 같은 camera-facing transform을 사용한다.
- 이름·정지 이미지·비슷한 색만으로 특정 보스 관문의 실제 variant 선택을 확정하지 않는다. source 구조 대응과 level/script 직접 참조 여부, 사용자 화면 판정을 나눠 기록한다.

### Profiler 숫자의 계측 분모를 유지한다

- GPU timestamp가 Update 전~Present 뒤를 감싸면 GPU 실행 사이 CPU 공급 공백을 포함할 수 있다. CPU와 GPU frame ms가 같다는 사실만으로 GPU 연산 포화를 단정하지 않는다. pass timestamp·Present CPU·copy 횟수를 별도로 측정한다.
- `PSInvocations`는 셰이더 호출 수이며 셰이더 내부 ALU 연산 수가 아니다. 전체값을 특정 패스 비용으로 해석하지 않는다. GPU scope의 `pipelineValid`가 true인 PS/VS 통계만 사용하고 미선택·미지원 값을 0회 실행으로 해석하지 않는다. 인스턴싱으로 draw 제출이 줄어도 겹친 픽셀 수가 자동으로 줄지는 않는다.
- Long Operations는 임계값 이상인 완료 호출만 표시한다. main만 보인다는 사실로 전체 프로세스에 worker가 없다고 단정하지 않으며 부모·자식 시간을 합산하지 않는다. worker 계산 합계와 main join 대기는 서로 다른 지표다.
- `Particle.Simulate`의 spawn/update 동명 scope, 부모·자식 inclusive 시간, 여러 playback의 fixed-step 호출 합계를 구분한다. 고정 스텝 따라잡기와 동기 Save JSON의 프레임 간섭도 기록한다.
- JSON의 counter 키 존재는 실제 writer 존재를 뜻하지 않는다. 확장 전 renderSubmissions/texture 계열의 writer 없는 0값을 작업량 0의 증거로 쓰지 않는다. 새 renderSubmissions는 실제 enqueue에 연결되며 미연결 texture counter는 N/A로 표시한다. occurrence별 SceneColor refresh도 실제 copy 함수 안에서 계측해야 초기 snapshot만 보이는 누락을 막는다.

### ImGui backend에 Engine 헤더를 넣을 때 Debug new 매크로를 격리한다

- `Engine_Defines.h`는 `_DEBUG`에서 `new`를 CRT debug allocation 매크로로 바꾼다. 이를 ImGui backend에 그대로 유입하면 `IM_NEW`의 placement-new가 C2226 `ImNewWrapper` 오류로 깨진다. backend의 Engine 헤더 include 전후에 `push_macro("new")` / `pop_macro("new")`로 진입 시 상태를 복원한다. Engine 전체의 debug allocation 설정을 끄지 않는다.
- 비-Debug standalone compile 성공은 이 경계를 검증하지 않는다. 실제 Debug Engine compile과 SDK 복사 뒤 Client compile/link를 확인한다. 새 profiler enum이 Client에서 없다고 표시되면 `Engine/Public/Profiler.h`와 `EngineSDK/Inc/Profiler.h`의 일치 및 선행 Engine build 성공부터 확인한다.
- 비동기 저장의 완료 Poll을 ImGui 창 Render에만 두면 저장 중 창을 닫았을 때 joinable worker와 비활성 Save 상태가 남는다. 저장 owner의 Update에서 창 가시성과 무관하게 결과를 회수한다.

### 한 target의 긴 준비와 GPU readback은 분배 횟수만 제한해도 main을 멈춘다

- 프레임당 Effect target 한 개만 처리해도 target의 parse/decode/resource 준비가 13초면 그 프레임이 13초 멈춘다. 기존 EffectLoadPreparationJob의 worker stage/result/ACK를 runtime에서도 사용하고 main은 결과 commit을 담당한다. scope는 실제 stage와 ACK 대기를 분리한다.
- runtime job에서 Loading owner로 넘어갈 때 기존 worker의 협력 취소·drain을 확인한다. 실패 job은 terminal receipt와 원인 문자열을 보존하며 매 프레임 같은 target을 다시 시작하지 않는다.
- GPU timestamp interval에 CPU 명령 공급 공백이 포함될 수 있다. 전체 화면 PickPos readback처럼 Copy 뒤 즉시 Map하는 동기 경로는 CPU wait와 bytes를 따로 확인한다. 마우스 한 점을 얻으려고 viewport 전체를 복사하지 않는다. 현재 요청의 좌표/target/RowPitch/no-hit 계약을 유지한다.
- Profiler panel 자체의 raw sample 정렬·집계가 캡처를 교란할 수 있다. self-time 계산을 바꿀 때 실제 캡처의 thread별 nesting, zero duration, orphan와 frame window를 비교한다. 집계 함수 가속 배율을 게임 FPS 배율로 보고하지 않는다.
- Save JSON은 최근 최대 1200프레임의 독립 파일을 추가한다. 완료되지 않은 긴 scope는 종료 전까지 집계에 없고, 이미 기록된 frame history도 무제한은 아니다. 저장 이름이 같아도 기존 파일을 덮어쓰지 않는다.
- pause한 Profiler 패널도 아직 pending인 GPU 결과가 회수될 때까지는 갱신해야 한다. 이후 변경 없는 history의 반복 집계를 중단하며 Capture/Reset/frame-window 변경은 즉시 반영한다.

### 공유 mesh의 포즈와 파티클 병렬 작업 경계

- `CModel` clone은 mesh geometry를 공유하고 bone pose는 각자 소유한다. skin palette cache도 모델에 두고 combined pose가 갱신될 때 무효화한다. WModel의 전체 skeleton palette와 Assimp의 mesh별 bone subset/offset을 같은 것으로 취급하지 않는다. frame number만으로 cache를 고정하면 한 프레임 안의 secondary motion·명시적 pose 변경을 놓친다.
- 파티클 병렬화는 각 emitter의 particle/RNG 상태를 한 작업이 소유하고, 불변 준비 데이터만 공유한다. emitter 간 spawn provider와 portable event/death-event queue는 기존 순서를 유지한다. worker 결과를 모두 회수하기 전에 다음 fixed step, event, trail, frame rebuild를 진행하지 않는다.
- worker 수 자체를 성능 개선으로 기록하지 않는다. Debug checked iterator의 경합, 작은 작업의 제출·join 비용을 실제 serial/parallel 동일 결과 비교로 확인하고, compiler 최적화 효과와 알고리즘·병렬화 효과를 분리한다.
- 모델 worker의 협력 취소는 진행 중인 한 binary decode를 즉시 중단하지 못할 수 있다. 정상 레벨 전환은 요청을 보존하고 프레임을 진행하며 준비·큰 자원 해제가 끝난 뒤 전환한다. main에서 service를 먼저 파괴해 bounded join timeout을 정상 전환에도 발생시키지 않는다. worker registry와 owner는 thread를 시작하기 전에 준비하고 immutable authoring/catalog 입력은 main에서 캡처한다.

### 소환 모델의 뒤틀림·정지와 여러 material section을 구분한다

- glTF→WModel의 bone matrix 일치만으로 PSA→glTF 변환이 맞다고 판정하지 않는다. root는 유지하고 mesh hierarchy의 child quaternion을 conjugate해야 하며, 실제 원본 PSA와 여러 frame의 pose를 대조한다. 기존 geometry/weights/skeleton이 정상이어도 animation rotation만 잘못될 수 있다.
- PSA와 mesh joint 순서가 다를 때 skin JOINTS나 IB를 재배열하지 않는다. 명시적인 unique-name remap으로 animation channel target만 연결하고 누락·중복 이름은 거부한다.
- 말의 section0~3은 한 골격의 재질 조각이다. section 하나의 local rotation만 바꾸면 조립이 깨진다. 원본 모델 basis와 한 동물의 공통 transform을 확인한다.
- ModelCue의 holdLastFrame=false는 반복 재생이 아니다. loop는 별도 입력으로 검증하고 animation time만 감싼다. cue 이동 시간을 fmod로 감싸면 매 회전마다 시작 위치로 되돌아간다.
- loop 시 뒤로 튀면 원본 root의 수평 displacement와 cue velocity를 따로 실측한다. 원본 수평 이동을 반복 초기화하는 문제를 quaternion 재변환이나 cue 방향 반전으로 가리지 않는다. 명시적인 ModelCue root-motion suppression은 기존 CModel을 재사용하고 source Y 점프와 원본 binary를 보존한다. optional 필드와 cache·rollback 계약은 [렌더링 복원 정본](렌더링이펙트복원V2.md)의 ModelCue 항목을 따른다.

### 제품 맵 배치의 LFS 병합 충돌과 parser 이유를 구분한다

- `Map: product load scope`는 진행 단계다. 실패 시 `Read_Placements`의 실제 parser 이유와 Area ID를 `CLoader::Get_ActiveStatus()`에 보존해 `Recover_FromFailure`의 세션 진단 JSON까지 전달한다. scope 결과에 배치가 없는 경우도 명시적인 이유를 남긴다.
- 실행 `.mapplacements`는 `LOSTARK_MAP_PLACEMENTS` 헤더의 게시 출력이어야 한다. LFS pointer나 conflict marker가 남은 파일의 header 거부를 resource 누락이나 GPU 문제로 오판하지 않는다. 같은 Area의 worldsequences도 확인하고, 통합한 `Data/Maps/Authoring` 정본으로 Area publisher와 Check를 수행한다. 생성물을 직접 편집하거나 parser를 완화하지 않으며 일반 C++ 빌드를 publisher로 간주하지 않는다.
- 충돌 제거·게시·컴파일 성공과 최종 Client의 arena 진입은 별도 증거다. 당시 원인과 게시 후 정상 상태는 [PR360 통합 결과](09-11/2026-09-11_PR360_WORLD_OBJECT_RESOURCE_MERGE_IMPLEMENTATION_RESULT.md)에 구분한다.

### 스킬 애니메이션만 동작하고 한 직업의 모든 Effect가 없으면 animevents 전체 로드를 확인한다

- `.animevents`의 헤더 총행수는 원본 참고 event와 제품 `effectref=asset` 행을 모두 포함한다. clip cue 병합·삭제 뒤 실제 행 수를 갱신하지 않으면 parser가 문서 전체를 거부한다. 개별 Effect JSON·catalog 존재만 확인하면 이 실패를 놓친다. 통합 도구에서 최종 행 수를 산출하고 실제 Product prewarm 및 설치 모델 clip/bone을 사용한 cue Load를 검사한다.
- Artist와 LanceMaster ALT V에서 같은 결함이 재발했다. Lance의 선언3139/실제3136을 맞춘 뒤43cue가 정상 admission됐으며, 이미 실패한 prepared 문서는 Client 재시작으로 다시 읽는다. 헤더 검사를 완화하거나 이 데이터 수정에 EXE/Server 재빌드를 요구하지 않는다. [상세 원인과 검증](09-11/2026-09-11_TIGER_HORSE_ANIMATION_AND_LANCEMASTER_ALTV_RESULT.md#g07-창술사-전체-이펙트-미출력의-실제-로더-회귀)을 따른다.

### Effect Tool에서 읽힌 큰 문서도 제품 준비 경로를 확인한다

- Lance ALT V의 20,049,144-byte full 문서는 Codec의 64MiB 한도에는 들어왔지만 Product Catalog의 별도 16MiB 한도에서 거부됐다. standalone Codec/Renderer Stage만으로 스킬 제품 준비를 검증하지 않는다. 실제 Catalog request와 `Stage_LoadingProductTarget`을 연결해 확인한다.
- 저작·제품 문서는 `CEffectDocumentCodec::MAXIMUM_DOCUMENT_BYTES`의 64MiB 상한과 bounded Load를 공유한다. catalog index의 16MiB 한도와 JSON depth/value/identity 검사는 별도 계약이다. 임의 minify로 현재 파일만 통과시키면 F1 Save 후 재발할 수 있다. [실제 실패와 교정 결과](09-11/2026-09-11_TIGER_HORSE_ANIMATION_AND_LANCEMASTER_ALTV_RESULT.md#g08-alt-v-제품-로더의-16mib--64mib-불일치)를 따른다.

### 손 부착 창이 돌아가면 source TypeData 회전 누락을 먼저 구분한다

- MeshRotation distribution의 quarter-turn과 TypeData의 degree 회전은 별개다. Lance V/ALT V source pitch=-90이 typed detail에서 빠져 있으면 실제 +Y 메시가 손본 -Z로 향한다. `[roll,pitch,yaw]`를 기존 `sourceTypeDataRotationDegrees`에 한 번 투영하고 local/socket 회전이나 offset을 임의로 덧붙이지 않는다.
- 같은 `fm_x_flm_gdr_01`/dragon을 쓰는 T34650도 두 full 문서의 typed pitch가 누락/0이었다. V/ALT V 교정이 다른 스킬의 같은 mesh 행까지 자동 적용되는 것은 아니므로 실제 요청 슬롯의 모든 clip을 대조한다. 사용자가 확정한 Transform 위치는 source 회전 복구와 별도 필드로 보존한다.
- source import scale 보정은 방향을 회전시키지 않지만 기존 방향·offset 오류를 크게 드러낼 수 있다. 실제 손본 pose와 설치 mesh vertex의 world 결과로 크기·원점·방향을 따로 비교한다. synthetic axis만 finite라는 검사로 실제 손 부착이 맞다고 기록하지 않는다.

### Composition 재생 거부와 첫 프레임 준비 지연은 별도로 확인한다

- 저장 Composition revision과 게시된 Pattern revision이 다르면 빈 DRAFT만 추가됐어도 Complete Play는 거부된다. 초안을 버리거나 revision 검사를 완화하지 말고 공식 publisher를 사용한다. 깨끗한 편집기의 cached revision이 아닌 실제 저장본을 비교하며 미저장 편집과 게시 중 상태는 보호한다.
- WORLD cue마다 같은 문서를 다시 읽고 전체 검증하면 첫 Play에 동일 비용이 누적된다. 같은 요청의 독립 player는 검증을 한 번 공유하되 모든 문서 복사를 준비한 뒤 교체한다. per-instance 리소스 검증과 시계는 유지한다.
- WORLD Effect는 worldId와 실제 sampled pivot이 필요하다. 맵 절대좌표 조명이나 화면 fade에 빈 WORLD를 저장하면 anchor 대기에서 생성에 도달하지 못한다. 이런 Effect는 MAP을 명시한다.
- mouse_click LocalDecal의 시작 공백은 birth와 화면 마스크를 구분한다. Life 조절은 burst 시간과 shader의 첫 파동 위상을 바꾸지 않는다. source alpha를 opacity로 단정하지 말고 실제 DDS와 native mask를 대조한다. [수정·검증 결과](09-12/2026-09-12_KOUKU_PLAYBACK_AND_WORLD_MARKER_IMPLEMENTATION_RESULT.md).

### Native effect 생성물은 모델 전용 include와 원본 pass 상수 범위를 함께 검사한다

- `ARTIST_NATIVE_MODEL_ONLY`에서 기본 함수만 제외하고 distortion companion을 포함하면, 모델 파일의 선언보다 먼저 scene-depth texture를 참조해 실제 FXC X3004가 발생한다. companion도 동일 MODEL_ONLY guard를 갖게 하고 생성기와 설치 결과를 함께 수정한다. texture 선언을 앞으로 옮겨 경계를 우회하지 않는다.
- native cohort 확장 시 CB0 material 행뿐 아니라 원본 CB2 pass 상수의 선언·실제 읽기 범위도 확인한다. 고정 4행 scratch는 CB2[4]/CB2[6]를 사용하는 원본에서 X3504를 만든다. viewport 값은 실제 render target 크기, override 값은 검증된 기존 scene 계약으로 공급한다.
- 파티클·메시 컴파일만으로는 MODEL_ONLY include 회귀가 드러나지 않는다. 같은 include를 소비하는 `Shader_VtxAnimMeshBinary`도 FXC로 확인한다. [교정 결과](09-12/2026-09-12_KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT.md)를 따른다.

- 왜곡 PS의 CB1 참조를 `projection`으로 치환했다면 실제 선언과 carrier 입력도 연결한다. 쿠크 `2d8c822c...`는 TEXCOORD5의 source world cm를 한 번 투영한다. screen clip 값을 다시 투영하거나 Trail에 0 행렬을 전달하지 않는다.
- native texture index 9를 쓰는 프로그램은 열 번째 SRV가 필요하다. generated sample helper만 늘리지 말고 renderer staging 배열, bind, screen-post snapshot·mask와 독립 모델 shader 선언까지 같은 상한을 적용한다.
- native 재질 추가 때 기존 descriptor의 상한만 늘려 하나의 mega-switch에 누적하지 않는다. 생성기에서 64 ID 구간별 물리 HLSLI·carrier FX·dispatch·실행 표·project/filter를 함께 갱신하고 원본 함수/guard/ID를 보존한다. 같은 내용은 다시 쓰지 않아 증분 tracking을 유지한다. VS/PS를 패스 간 공유해도 한 PS의 수백 재질 최적화 비용은 남는다.
- IDE와 runner의 Visual Studio/toolset/SDK/host architecture가 다르면 소스 변경 없이도 전체 재컴파일될 수 있다. 현재 `lastbuildstate`만으로 지난 재빌드 원인을 확정하지 말고, runner의 toolchain·전후 state와 선택 실행의 diagnostic 로그를 비교한다. 출력 timestamp 조작이나 강제 skip으로 감추지 않는다.
- 같은 FX의 여러 pass가 같은 entry/profile을 사용하면 `CompileShader` 결과를 공유한다. pass 이름·순서·render state와 서로 다른 entry는 유지한다. 정적/애니메이션 CModel shader도 이 검사를 포함하며, 컴파일 표현식 수 감소와 실제 FX 생성·pass/input layout 검증을 구분한다.
- 증분 측정은 같은 MSBuild와 완전히 같은 인자를 반복한다. 같은 디렉터리라도 `OutDir`의 slash 표기가 달라 `/Fo` 문자열이 바뀌면 FXC command tracking이 전체를 다시 컴파일할 수 있다. 그런 실행은 no-change 결과로 보고하지 않고 별도 재빌드로 기록하며, CSO 내용과 수정 시각 및 실제 FXC 실행 수를 함께 확인한다.

### 시퀀스 목록 표시·소스 검증을 실제 Play 준비와 혼동하지 않는다

- Composition은 Data 원본의 새 WORLD ID를 참조할 수 있지만 Level은 게시된 Area 문서, World Object Tool은 저장 문서의 cache를 사용한다. 저작 revision만 올리고 실행용 `.worldsequences.json`과 `.camerashots.json`을 게시하지 않으면 row는 보여도 Play 준비에서 거부된다. 같은 Area publisher의 Publish와 Check를 수행하고 새 Client에서 동일 WORLD/Camera ID와 revision을 확인한다. 일반 C++ 빌드는 이 배포를 대신하지 않는다. 미저장 Tool 문서를 자동 reload하거나 누락 ID를 건너뛰지 않는다.
- `World Object model admission failed`는 파일 부재만 뜻하지 않는다. 실제 CModel decoder와 material/texture 준비 이유를 구분한다. WMSH submesh를 줄일 때는 같은 submesh의 bounds도 함께 줄이고 bone tail과 나머지 section은 보존한다. 2관문 Table은 4개 중 2개 mesh만 남기면서 bounds 4개를 유지해 80-byte trailing payload로 거부됐다. decoder 검사를 완화하거나 파일 이름만 바꾸어 해결하지 않는다.
- 새 연출은 기존 row까지 포함해 occurrence의 ID·enabled·중복·시간, 실제 model/material/animation 준비, Camera/Effect/SceneProfile 참조를 검사한다. 이 결과와 사용자가 Client에서 Play해 확인한 카메라·연출·전투 결과는 별도 완료 상태로 기록한다. 원본 Fade/카메라/배우/Effect가 여러 문서에 나뉘어 있다는 사실을 SceneProfile 하나에 원본 전체가 들어 있다는 설명으로 바꾸지 않는다.
- Complete Play는 관문별 입장 Sequence뿐 아니라 저장 Pattern Flow와 같은 source revision의 Server Product가 필요하다. Python Product projection만 게시하면 Server bootstrap은 이전 revision일 수 있다. 최종 revision을 명시한 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision <revision>`로 관련 Product/Map/World/Balance를 함께 게시하고 세 관문의 Flow target을 확인한다. F1 관문 spawn 성공을 저장 Flow 존재의 증거로 쓰지 않는다.
- 이전 EXE용 Data 사본을 본 작업으로 합칠 때 양쪽의 새 Pattern이 같은 ordinal ID를 할당할 수 있다. 기준본과 양쪽 저장본을 세 방향으로 비교하고 현재 항목을 덮어쓰지 않는다. 충돌한 신규 항목은 미사용 stable ID와 내부 action/occurrence 참조를 함께 재배정하며 사용자 clip·시간·loop 값은 보존한다.


### Object 그룹 편집은 생성 행과 판정 참조를 함께 저장한다

- WORLD emission의 개수·순서·Delay를 바꿀 때 indexed Collider와 전용 Logic의 참조를 함께 갱신한다. shared hold/result 의존성, 모호한 WORLD, dirty Composition은 저장 전에 거부하고 기존 draft를 보존한다. 슬롯 번호를 stable 저장 ID로 새로 승격하지 않는다.
- 카드 준비 풀은 동일 Area/revision, model/preScale/material 및 device 범위에서만 공유한다. Stop/완료 반환과 문서 교체 시 token 무효화가 함께 있어야 하며, 첫 입장뿐 아니라 Object Save 뒤 reload에서도 다시 준비한다. 실제 GPU/FPS 검증과 CPU 준비 성공을 구분한다.
- WModel이 이미 30Hz이고 FLOAT weights가 정상이어도 child quaternion conjugate 누락은 별개다. 원본 PSA 전체 clip 회전을 대조하고 기존 geometry/skeleton/material과 위치·scale·시간 키를 보존해 교정한다. 괴기스러운 인형은 말·호랑이와 같은 원인으로 확인됐다. [인형·외곽불 결과](09-12/2026-09-12_KOUKU_DOLL_FIRE_REPAIR_RESULT.md).
- WINT minor 증가로 정적 mesh 속성이 추가돼도 내장 WMA2 레이아웃이 유지될 수 있다. repair 도구는 실제 구조와 material identity를 검사해 지원 버전을 명시하고 임의 byte offset 교체로 우회하지 않는다. 외곽불 D/E/F의 잘못된 emissive 입력 제거는 원본 native 불 재질 전체 복원과 구분한다.


### 렌더링 hot path는 실제 소비 입력과 큐 수명을 함께 보존한다

- source family별 재질 준비를 줄일 때 PS의 family 분기 앞 공통 처리도 검사한다. `Shader_VtxMeshBinary`의 opaque/shadow presentation dither는 source BG에도 `g_Opacity`를 읽는다. native 재질이 raw UV를 쓴다는 이유로 opacity까지 생략하면 소품이 잘못 사라진다. source on/off, diffuse override와 직전 shader 상태를 실제 MRT/depth로 비교한다.
- per-draw 진단 목록은 닫힌 도구에서도 문자열 검색·삭제·할당 비용을 만들 수 있다. 실제 UI 조회가 있는 동안만 수집하고 level 변경·만료와 재열기 동작을 유지한다.
- list 렌더 큐를 capacity 재사용 vector로 바꾸면 callback append가 iterator/reference를 무효화할 수 있다. index로 순회하고 객체 수명은 queue의 shared_ptr로 보존한다. sorted BLEND의 snapshot 순서와 실패/pass 종료 clear를 별도로 유지한다.
- shader instruction/SRV 감소와 CPU Draw 제출 단축을 GPU pixel 실행 단축으로 간주하지 않는다. 같은 입력의 작은·넓은 면적을 각각 비교하고 실제 게임 프레임 결론은 사용자 캡처로 판단한다. [맵·캐릭터 성능 결과](09-12/2026-09-12_MAP_CHARACTER_RENDER_PERFORMANCE_RESULT.md).

### 패턴 Effect 분리는 실제 소비자·간접 원본·수명을 확인한다

- JSON parse와 자체 field 검사만으로 v15 authored 문서 admission을 대신하지 않는다. 비어 있어도 필수인 `runtimeExtensions` 누락은 실제 CEffectDocumentCodec에서 거부된다. 독립 그룹은 그 codec과 Playback roundtrip·seek를 통과해야 한다.
- 본체의 disabled notify를 켜서 부족한 폭발을 보충하지 않는다. SkillEffect → NPC → Action의 간접 원본에 실제 십자 연출이 있을 수 있다. 알비온의 4방향은 raw FRotator를 사용하고, MIC permutation의 texture index는 해당 MIC cooked texture 배열과 join한다.
- source `bKillOnDeactivate`의 metadata 존재를 runtime 소비 완료로 기록하지 않는다. 예고 종료와 긴 입자 tail을 구분해 해당 원본 occurrence의 가시 구간을 유지한다.
- level-owned Effect를 network combat object에 연결하면 보스 weak pointer의 자동 정리를 기대할 수 없다. natural expire와 Stop/사망/despawn의 즉시 취소, 늦은 snapshot의 object 자체 pinned revision을 함께 확인한다. [패턴 그룹 결과 G08](09-12/2026-09-12_KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT.md).

### 이펙트의 90도 오차와 크기 차이는 원본 occurrence별로 분리한다

- 같은 원본 재질을 쓴다고 geometry의 기본 면·긴 축까지 같지는 않다. 실제 WModel 정점과 preScale을 먼저 읽고, TypeData mesh pre-rotation → StartRotation → notify/local TRS → socket·부모 basis → 독립 그룹 전방 변환의 실제 합성 결과를 확인한다. 렌더러 enum만 검사하면 실제 meshModel 바인딩으로 선택되는 mesh carrier를 놓칠 수 있다.
- sourceRecipe에 원본 `pitch=-90`이 남아 있어도 `detail.mesh.sourceTypeDataRotationDegrees`에 투영되지 않으면 그 pre-rotation은 소비되지 않는다. 자동 보정 함수가 존재한다는 이유로 모든 asset이 적용된다고 간주하지 말고 asset admission 조건을 읽는다. 쿠크 hoop의 누락은 창술사 창과 같은 증상이지만 별도 occurrence에서 확인해 보정했다.
- `FRotator`의 65536 정수 단위, StartRotation의 1회전 단위, TypeData의 degree를 섞지 않는다. `[roll,pitch,yaw]`를 위치 벡터처럼 `(x,z,-y)`로 바꾸거나 degree에 다시 360을 곱하지 않는다. 기존 UE3 Euler basis 변환을 사용하고 mesh의 pre-rotation과 particle rotation을 각각 한 번 적용한다.
- 독립 그룹을 +Z 전방으로 맞출 때 source +X의 yaw 보정을 socket snapshot basis, element TRS, particleSystem yaw에 중복 적용하지 않는다. 원본 Projectile의 yaw와 이미 적용된 원본 위치를 합성한 뒤 링 중심·분사구·법선을 대조한다. 한 emitter의 빠진 pitch를 전체 시스템 yaw로 덮으면 정상 sprite와 잔불까지 돌아간다.
- 크기의 cm→m와 WModel preScale, StartSize, notify scale, 골격 basis100·CModel scale, 사용자 확대는 각각 다른 입력이다. mesh 크기를 바꾸려고 입자 위치·속도까지 임의로 나누지 않는다. 같은 화염포를 두 그룹에서 재사용하면 원본·배율·방향 설정의 동등성을 확인하되 element ID별 난수 표본 차이는 허용한다.
- 무기 부착의 원점·반경이 맞아도 회전은 틀릴 수 있다. 원본 PSK와 설치 WModel의 geometry basis, 실제 body bone에 합성한 세 축과 정점을 함께 대조한다. 쿠크 WP05는 identity 손 소켓에서도 설치 geometry의 Y/Z 교환 때문에 catalog의 X축 preRotation -90도가 필요하다. 이 값은 해당 모델의 실측 결과이며 다른 무기·소켓에 일괄 적용하지 않는다.
- UE3 socket의 bone-local 위치·FRotator를 설치 골격에 그대로 복사하지 않는다. UE→PSK export mirror, 원본 bind, 설치 bind와 particle의 좌표계를 함께 합성한다. RPCT05는 원본·설치 bone 이름이 같아도 FX_Prj_03의 Y 부호와 회전 기저가 달랐다. 총구 검증은 같은 실제 clip 시점의 source 발생과 runtime 발생을 비교하며, proxy socket 원점과 입 정점이 다르다는 이유만으로 다른 본이나 추측 offset을 넣지 않는다.
- 실제 Playback의 finite·seek 성공만으로 방향·크기 또는 GPU 표시를 승인하지 않는다. 설치 geometry와 실제 재생 행렬의 축·중심·속도 및 필요한 본 샘플을 확인하고, 화면 크기·색·밀도는 사용자 확인으로 남긴다. 이번 수직 hoop·확대 화염포의 범위와 수치는 [패턴 그룹 결과 G09](09-12/2026-09-12_KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT.md)에 기록한다.


### Rendering Benchmark의 품질 저장·게시·적용 경계

- 스킬별 bloom 값은 Full Restore 문서 root `bloomIntensity`다. source group·skill ID가 같아도 단계별 문서는 별개이며 catalog만 순회하면 Authored 전체 탐색에서 보이는 미등록 문서를 놓친다. 원본 RGB/Emissive 수정과 별도 bloom 기여 조절을 구분한다.
- 투명 이펙트의 bloom은 같은 alpha/additive/occlusion 계약을 유지한다. 이미 합성된 SceneColor를 화면 왜곡 단계에서 다시 추출하면 다른 문서가0으로 억제한 bloom이 살아날 수 있으므로 가중치가 적용된 bloom 입력을 해당 화면 연산으로 함께 운반한다.
- root 필드를 추가한 뒤에는 새 codec과 같은 바이너리로 저장한다. 구 v13 Client는 unknown root를 무시해 열 수 있지만 Save 때 새 필드를 지울 수 있다. 현재 입력 전체를 보존하는 roundtrip과 단계별 독립값을 검사한다. [스킬별 Bloom 결과](09-12/2026-09-12_EFFECT_PER_SKILL_BLOOM_RESULT.md).

- 선택 Level 품질은 해당 base profile의 qualityOverride다. 연출용 scene profile에 그 값을 복사해 고정하면 기본 Bloom을 저장·게시해도 연출 진입 때 예전 값으로 돌아간다. 같은 값의 연출 복사본은 제거해 Level을 상속하고 light/fog/environment·연출 multiplier는 유지한다. 새 scene duplicate가 qualityOverride를 지우는 기존 규칙과 맞춘다.
- Save Authored, Publish Runtime, Reload Runtime은 서로 다른 단계다. 게시기 CLI 성공을 실행 중 catalog 갱신이나 UI 버튼 실패 원인 해소로 기록하지 않는다. 현 Publish_Runtime은 표준 출력을 수집하지 않고 UI thread에서 동기 대기하므로 실제 실패 원인과 무응답을 구분할 정보가 부족하다. [Bloom 게시·조사 결과 G09](09-12/2026-09-12_KOUKU_SOURCE_SEQUENCE_RESTORE_RESULT.md).

### Composition 게시와 타임라인 입력을 함께 잠그지 않는다

- 실행 중 Composition의 외부 Resource 등록은 LastGood와 디스크를 다르게 만든다. Save 거절 후 Reload/재시작 전에 미저장 Pattern·Bundle·staged placement를 보존한다. 자동 병합은 revision 증가와 기존 Resource prefix를 바꾸지 않은 신규 append만 허용하고, ID payload 충돌·다른 외부 편집은 기존 파일과 draft를 보존하며 거절한다.
- Effect placement의 anchor/follow/bone/world와 TRS는 같은 staged 값이어야 한다. Anchor만 바꾼 경우에도 Dirty·Preview·Play·Save·선택 왕복을 검사한다. MAP의 `[0,0,0]`은 해당 관문 중앙이 아니다. 정본 boss placement의 절대 월드 좌표 또는 명시한 사용자 좌표를 저장하고, SourceModelPreview가 없는 Resource 단독 미리보기에도 선택 Pattern의 actor/gate를 전달한다.

- `Publish All Patterns`는 여러 domain을 비동기로 게시한다. 프로세스가 살아 있다는 이유로 박스 선택·scrub·초안 편집까지 막으면 수 분 동안 Sequencer가 멈춘 것처럼 보인다. 메모리 초안 편집은 계속 허용하고, publisher가 읽는 원본을 쓰는 Save와 중복 Publish는 완료까지 제한한다.
- 게시 완료는 Workbench 초안 Reload가 아니다. 게시 도중 만든 초안·선택·커서를 유지하고 미저장 변경은 이후 Save/Publish로 반영한다. 실행 중 프로세스와 실제 완료 로그, 사용자의 입력 복구 확인을 구분한다. [Parent 타임라인 결과 G04](09-12/2026-09-12_KOUKU_PARENT_PATTERN_TIMELINE_IMPLEMENTATION_RESULT.md).

### Composition 게시의 반복 입력 비용

- Pattern/Bundle 후보마다 큰 World Sequence와 전체 WModel vertex·animation key를 다시 decode하면 같은 입력을 수십 번 처리한다. 한 게시 실행 안에서 JSON, 모델과 pose를 재사용하고, timing 검사는 clip metadata만, 본 sampling은 필요한 clip key만 읽는다. 특정 보스 이름에 특례를 두지 않으며 전체 geometry가 필요한 기존 reader 호출은 기본 full decode를 유지한다.
- cache 수명은 호출 안으로 제한한다. 출력 교체 전후 원본 bytes와 모델 hash를 다시 확인하고 변경되면 기존 rollback을 수행한다. 길이·mtime만 같다고 동일 입력으로 판정하지 않는다. PowerShell 원문 비교는 culture 비교인 `-cne` 대신 `StringComparison.Ordinal`을 사용한다.
- projector 직접 변환 시간과 Product/Map/World/Balance 전체 게시 시간은 다르다. 같은 고정 입력의 전후 bytes와 단계별 실측을 함께 기록하고, 한국어 표시명이나 Resources 전체 hash를 측정 없이 원인으로 단정하지 않는다. 실제 imported 도구도 domain fingerprint에 포함한다. [Parent 타임라인 결과 G06](09-12/2026-09-12_KOUKU_PARENT_PATTERN_TIMELINE_IMPLEMENTATION_RESULT.md).

- GPU LOD는 원본 geometry와 현재 draw 재질을 함께 검사한다. CModel material variant는 CMesh를 공유하므로 load-time opaque admission만으로 masked/교체 재질까지 LOD를 적용하지 않는다. indirect index 수는 CPU 추정치를 실제값처럼 기록하지 않고 별도 LOD0 상한 counter를 사용한다. 작은 draw는 dispatch 손익 검증 후 기존 direct 경로를 유지한다.
- light quad의 clip distance는 같은 VS 위치/UV를 유지해도 clipping 이후 interpolation 정밀도 차이를 만들 수 있다. coverage 누락과 FP16 출력 차이, clip-disabled control 및 사용자 visual 판정을 구분한다. 상세 근거는09-12 맵·캐릭터 성능 RESULT의G12–G14를 따른다.

- Composition Patterns의 편집 대상과 Resources의 추가할 Pattern은 별도 session 선택이다. 공용 Pattern Tree를 재사용해도 Resources의 Gate/Parent/Bundle/leaf 탐색은 대상·커서·preview를 바꾸면 안 된다. 실제 Append에서 현재 target/source를 다시 검사하고 잘리는 source 수명을 표시한다. [Parent 타임라인 결과 G07](09-12/2026-09-12_KOUKU_PARENT_PATTERN_TIMELINE_IMPLEMENTATION_RESULT.md).

## C++와 셰이더 빌드 입력 경계

- PCH에는 게임·저작·재질 표를 넣지 않는다. 기본 PCH와 charset/최적화 옵션이 다른 CPP는 PCH와 forced include를 함께 제외하고, 분리 CPP에는 원래 파일 옵션을 보존한다.
- Engine_Defines는 Assimp/DirectXTK/FX11/DirectInput/Engine_Struct의 우회 include가 아니다. 실제 완전 타입을 쓰는 CPP에 해당 헤더를 연결한다. Client에서 WinSock2는 Windows/D3D/DirectXTK보다 먼저 읽고, lean Windows 입력에서 RPC 헤더는 전역 using namespace std보다 먼저 읽는다.
- 생성 native material의 큰 표는 Private owner에서 한 번 컴파일한다. public inline 함수가 사용하는 작은 상수까지 Private로 이동하지 않는다. generator는 native_material_tables.py를 통해 읽기·저장을 하고 같은 bytes는 다시 쓰지 않는다.
- CPP를 분리하면 기존 source 검사도 등록된 same-owner CPP와 Private _Internal.h를 읽어야 한다. 다음 함수의 물리 순서를 기준으로 현재 함수 범위를 추정하지 않는다. cpp_source_domains.py와 Tools/Build/README.md의 소비 경계를 사용한다.
- 무변경 빌드의 OBJ/PCH/CSO 쓰기 0은 증분 처리 확인이다. 공통 셰이더의 큰 최적화 작업이나 cache 없는 빌드까지 해결한 증거로 쓰지 않는다. 세부 구조와 측정은 09-12/2026-09-12_PROJECT_BUILD_ISOLATION_IMPLEMENTATION_RESULT.md에 있다.
