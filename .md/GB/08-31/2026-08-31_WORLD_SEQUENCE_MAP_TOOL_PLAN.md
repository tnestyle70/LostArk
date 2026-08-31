# World Sequence MapTool Plan

## 1. Goal

MapTool에 정적 맵 오브젝트의 transform sequence와 Deploy ANIM 오브젝트의 named clip
sequence를 같은 `World Sequence` 저작 흐름에서 편집하는 기능을 추가한다. 한 번 만든
sequence template은 Area 안의 다른 호환 placement에 다시 적용할 수 있어야 하며,
저장된 template과 instance를 목록에서 검색하고 관리할 수 있어야 한다.

추가 입력은
`Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/_레버_ITR_02283`과
`_종이펼침_paperstage`의 UModel glTF/PSA source bundle이다. MapTool은 이 원본을 CModel이
바로 읽는 파일로 위장하지 않고, 원본 검증 -> existing `ModelAssetConverter` 조리 ->
Resources-relative WModel 등록 순서를 사용한다.

이번 변경의 완료 범위는 **MapTool authoring + 로컬 preview + authoring document**다.
Server interaction packet, replicated product playback, sequence 완료 시 navigation/collision
transaction은 별도 수직 슬라이스다. 툴 preview를 제품 런타임 완료로 표시하지 않는다.

## 2. Existing contracts to preserve

- Map placement 원본과 runtime은 `CMapPlacementDocument`와 `CMapPlacementRuntime`을 사용한다.
- Preview는 `CMapAssetObject::Set_PlacementTransform`,
  `CMapStaticBatchObject::Update_Instance`,
  `CMapPlacementRuntime::Set_RuntimeVisible`만 사용한다.
- Preview 중 `MAP_RUNTIME_PLACED_ENTRY::record`와 MapTool의 placement dirty state를 수정하지
  않는다. Stop, 탭 변경, Area 변경, 툴 닫기, 오류 시 baseline을 복원한다.
- 저장 파일은 source placement 문서와 같은 Area authoring 폴더에 둔다.
- 화면의 짧은 조작명은 영어, hover 도움말과 중요한 사용 설명은 한국어로 표시한다.
  stable ID와 저장 필드명은 영어를 유지한다.

## 3. Source intake and runtime asset

- raw bundle은 source/reference로만 탐색한다. glTF/PSA/PSK/.props를 CModel에 직접 넘기지
  않는다.
- UModel이 출력한 skinned glTF의 joint 목록과 PSA `BONENAMES`/hierarchy를 exact 검사한 뒤
  PSA transform key를 glTF animation channel로 stage하고, 기존 ModelAssetConverter로 WModel을 만든다.
- 조리 중간물은 Resources 밖 staging에 둔다. 검증된 WModel과 직접 material texture만
  요청된 Resources Area 하위에 commit한다.
- `evt2_paperstage_open01`, `go_on`, `go_off`, `on`, `off`는 WModel info에서 실제 clip
  이름과 길이를 재검사한 뒤만 저작 UI에 노출한다.

## 4. Authoring data

파일명은 `<AreaId>.worldsequences.json`, schema는 `lostark.world-sequences`다.
formatVersion 1 map-only 문서는 로드 호환하고, typed target/animation track을 저장하는 현재
문서는 formatVersion 2로 쓴다.

- `templates`: 재사용 가능한 sequence 정의
  - stable `sequenceId`, 표시 이름, category, duration, interpolation
  - 하나 이상의 target slot
  - slot별 정렬된 transform keyframe
  - keyframe은 time, position offset, relative quaternion, scale multiplier, visibility
- `instances`: Area placement에 적용한 배치
  - stable `instanceId`, `templateId`, enabled, start delay, playback speed
  - template slot과 `(MAP_PLACEMENT | DEPLOY_PLACEMENT, uint64 targetId)` typed binding
- `animationTracks`: Deploy ANIM slot의 named clip, playback rate, loop, hold-last-frame

로드와 저장은 parse -> exact schema validation -> reference validation -> stage -> commit을
따른다. 저장은 temporary file을 flush한 후 ReplaceFile/MoveFileEx로 교체한다. 잘못된 파일은
현재 Area와 현재 runtime을 유지하고 상태 문자열에 이유를 남긴다.

같은 Area의 map placement와 sequence는 Area별 sidecar lock을 잡은 상태에서만 읽고 저장한다.
툴이 Reload 때 읽은 두 원본의 byte baseline을 저장 직전 다시 비교해 stale editor의 덮어쓰기를
거부한다. 두 파일 저장 뒤에는 의도한 placement/document와 다시 읽은 내용이 같은지 확인하고,
어느 한쪽이라도 다르면 양쪽 백업으로 rollback한다. sequence 입력은 parse 전에 16 MiB로 제한한다.

## 5. MapTool integration

1. `TOOL_MODE::WORLD_SEQUENCE`와 `World Sequence` 모드를 추가한다.
2. 독립 `CWorldSequenceToolPanel`이 document, selection, dirty state, playback clock,
   preview baseline을 소유한다.
3. Area switch는 새 panel/document를 먼저 stage한 뒤 map placement runtime stage까지 성공한
   경우에만 함께 commit한다.
4. `Save_AllAuthoring`과 unsaved Area switch 경고에 sequence dirty state를 포함한다.
5. Sequence 모드의 viewport click은 일반 Map Asset placement로 fall-through하지 않는다.
6. Template list, instance list, Map Objects/Animated Props browser, new/duplicate/delete/apply, keyframe editor,
   play/stop/loop/time scrub, validate/save/reload를 제공한다.
7. Animated Props는 source bundle 상태, cooked WModel 상태, clip 목록을 분리해 보여 준다.
8. 조리된 Deploy ANIM asset은 stable asset ID로 catalog에 등록하고, project-authored
   placement를 생성/이동/회전/크기/삭제/저장할 수 있게 한다.

## 6. Preview rules

- Instance play 시작 시 binding된 placement의 baseline record를 복사한다.
- Template key를 시간으로 sample하고 relative transform을 baseline에 합성한다.
- visibility는 이전 key의 step 값, transform은 Linear 또는 Smooth Step 보간을 사용한다.
- batch mirror parity가 baseline과 달라지는 key는 document validation에서 거부한다.
- runtime 적용 하나라도 실패하면 즉시 모든 target을 baseline으로 rollback하고 playback을
  중단한다.
- Deploy ANIM preview는 시작 시 현재 clip/index/track position/pause/loop를 snapshot하고,
  scrub 동안 normal animation advance를 멈춘 뒤 Stop/탭 전환/Area 전환/오류에서 snapshot을
  복원한다.
- transform track은 map placement에만, animation track은 ANIM deploy placement에만 binding한다.

## 7. Validation

- C++ validator가 JSON schema/reference, duplicate ID/slot/key time, missing placement,
  invalid target kind/clip/quaternion/scale/playback speed를 거부하도록 구현
- source cooker가 glTF skin/joint와 PSA bone/hierarchy/key count/time/rate/non-finite를 엄격히 검사하고
  converter `info`의 skeleton/animation count/name/duration을 재검사
- 빠른 Python source/project integration guard로 validator, preview rollback, linked-save
  recovery, Area lock/stale-source CAS, bounded input, exact post-save verification,
  project/filter 등록과 UTF-8 compile option의 연결 누락 확인
- C++ 동작은 Debug Product build로 컴파일·링크하고, 실제 편집/저장/재로드/미리보기 시각
  흐름은 사용자가 Development MapTool에서 직접 확인
- Debug Product build
- `git diff --check`
- 최종 UI/시각 결과는 사용자가 Development MapTool에서 직접 판정

## 8. Known visual boundary

종이 펼침의 형상 변화는 static `book01a`의 rigid transform이 아니라 12-bone
`paperstage` + `evt2_paperstage_open01`이 담당한다. 따라서 원본 clip preview를 정본으로
사용하고 rigid hinge sequence로 위장하지 않는다. 이번 변경은 MapTool authoring/preview까지이며,
레버 click 패킷, Server 권위 OPENING/OPEN 상태, 동적 충돌과 navigation 개방은 별도 수직
slice에서 같이 닫는다.
