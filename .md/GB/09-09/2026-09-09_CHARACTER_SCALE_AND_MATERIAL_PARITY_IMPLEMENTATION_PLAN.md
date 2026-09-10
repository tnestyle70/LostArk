# 캐릭터 외형 크기와 모코코 기준 재질 연결 구현 계획서

## G11. 2026-09-10 세 캐릭터 재질과 TJ 커스터마이징 통합

사용자가 차원술사·워로드·도화가의 본체, 머리, 장비, 무기를 모두 복구하고
`Downloads/TJ_Character (1)`과 통합한 공유 기준본을 요청했다. 기존 G10의
차원술사 우선 범위를 세 클래스로 확대한다. 변경 전 실측은
`out/CharacterPR352Audit20260910/`에 보존한다. PR 352 merge `146525de`와
현재 작업 트리의 재질 변경을 함께 소비하며 다른 기능의 미커밋 변경은 보존한다.

### 파일과 데이터 흐름

- TJ Artist 본체의 눈 UV1/UV2, 눈 본 연결과 눈 AO 가중치 수정을 보존한다.
  네 클래스의 기존 face morph 주소·위치·UV0 일치 검증은 통합 전후 유지한다.
- 차원술사는 현재 236개 본, 헤어 UV1, 본체 154개 clip과 Esther clip을 유지한다.
  TJ Customizing의 5개 clip은 채널 이름·index가 기존 본과 일치함을 검사한 뒤
  현재 skeleton hash로 연결한다. TJ의 225개 본체·UV1 없는 헤어로 되돌리지 않는다.
- `CharacterCatalog.json -> CActorCatalog -> CModel -> CMaterial`의 기존 재질
  override 경로에 원본 MIC의 parameter·texture와 정확한 source PS pair를 연결한다.
  PR 352 얼굴·눈 입력과 현재 별도 헤어 override를 함께 유지한다. 도화가 의상·붓·헤어는
  기존 family와 원본 PS가 다른 항목에 필요한 source program을 같은 경로에 추가한다.
- `Shader_SourceCharacterMaterial.hlsli`는 각 source program의 실제 varying 계약을
  연결한다. 워로드 legacy head program12 direct pass의 UV/light/view/position 배치가
  현재 기본 배치와 다른 결함을 수정한다. 기존 program을 다른 family로 치환하지 않는다.
- 차원술사 무기는 현재 19개 사용 material slot의 누락 texture가 0이다. 사용자 첨부
  Character Select 화면의 검은 무기를 기준으로 source program8/9의 실제 입력·출력을
  수치 검사하고, 확인된 미연결 입력을 기존 Renderer의 scene 환경 계약에 연결한다.
- 바이너리는 먼저 `out/CharacterPR352Audit20260910/merged-candidate/`에 생성한다.
  원본과 현재 설치본을 보존하고 구조·재질 로딩 검사가 성공한 후보만 설치·공유 대상으로 삼는다.
  WModel만으로 JSON 재질 매핑이나 shader 수정이 전달된다고 간주하지 않는다.

### 검증과 완료 경계

원본 SHA와 section/vertex/rig/clip 비교, 실제 CModel Create/Clone/Attach,
사용 material별 texture·추가 UV 검증, 원본 varying을 사용하는 WARP 수치 검사,
관련 Debug 빌드, JSON/XML parse와 diff check를 기록한다. 기존 도구·probe를 재사용하고
별도 제품 runtime이나 검증 프레임워크를 추가하지 않는다. 모델·texture의 전달 폴더와
Git 대상 코드·JSON을 함께 정리한다. Resources는 Git에 추가하지 않는다.
최종 화면은 사용자가 Character Select에서 세 클래스의 본체·머리·장비·무기와
커스터마이징·애니메이션을 직접 확인한다. 화면 미확인을 완전 복구 PASS로 기록하지 않는다.

## G10. 2026-09-10 차원술사 재질 복원 재개

사용자가 차원술사의 환경반사·재질 복원을 요청했다. 기존 17 override와 의상/피부/눈/무기
source program은 유지하고, G01의 실제 별도 헤어와 G06의 0으로 막힌 환경 cube를 연결한다.
RenderingProfile의 optional `environment`는 Resources 상대 cube ID, RGBM6 color/offset와
rotation/intensity를 소유한다. Renderer는 cube SRV를 먼저 stage하고 기존 quality/shadow/fog/light
교체가 성공한 뒤 함께 commit한다. 없는 environment는 명시적으로 이전 cube를 해제한다.
CMaterial의 기존 source base 바인딩이 Renderer의 환경을 소비하며 program 3/8/9의 원본
SampleLevel·방향·LOD·RGBM6 decode 계산을 유지한다. Engine 공용 struct/Renderer/GameInstance/
Material, Client RenderingProfileService, 기존 rendering publisher와 profile JSON만 확장한다.
새 C++ 파일과 별도 renderer·하네스는 추가하지 않는다.
별도 헤어는 body/equipment와 외부 AnimSet의236bone palette를 공유한다. 기존154+1+1개
animation의 key/event/clip은 유지하고 WANM trailer의 골격 참조 hash만 새 WSKL과 동기화한다.
mesh·skeleton·animation container를 각각 파싱한 수치 검사에 더해 실제 WModel decoder와
CModel Attach/Clone을 확인한다.

Character Select의 이미 추출·설치된 map cube와 해당 map의 프로젝트 선택 rotation/color를
같은 scene profile에 연결한다. 다른 씬은 정확 대응 입력을 확인한 경우만 연결한다.
원본 engine의 SH packing과 전역 probe 선택이 확인되지 않은 상태를 원작 환경광 전체 복구로
표현하지 않는다. 캐릭터 선택의 광택·헤어 화면은 사용자가 직접 판정한다.

작성일: 2026-09-09. 상태: **조사 완료 / 차원술사 1.5배 우선 적용 / 전체 재질 복구 계획**.

이 문서는 현재 여섯 playable character의 크기를 실측하고, 모코코 창술사를 비교 기준으로
외형 크기를 맞추는 변경과 차원술사·워로드·도화가의 누락된 재질 입력을 복구하는 범위를 정한다.
최초 요청은 계획과 조사였으며, 후속 요청에서 사용자가 **차원술사만 우선 1.5배 적용**을 명시했다.
따라서 G02~G04의 공통 연결 중 차원술사 1.5배에 필요한 부분을 먼저 구현한다.
다른 클래스 배율 변경, G01의 헤어 rig 복구, G05~G08의 재질·환경광 복구는 계획으로 남긴다.
아래 G 설명은 목표 계약이며 실제 구현·빌드 상태는 대응 RESULT에 기록한다.
이펙트 스킬 전체 복구나 Bern 조명 작업의 진행 상태를 이 문서의 구현 완료로 가져오지 않는다.

기존 [캐릭터 재질·이펙트 2차 계획](C:/Users/user/Desktop/LostArk/.md/GB/09-08/2026-09-08_CHARACTER_MATERIAL_AND_EFFECT_ROUND2_ESTIMATE_PLAN.md)의
G02 공통 재질 연결을 현재 실제 장비 소비자와 외형 scale까지 구체화한 후속 계획이다.
기존 문서의 과거 미구현 목록보다 해당 [RESULT](C:/Users/user/Desktop/LostArk/.md/GB/09-08/2026-09-08_CHARACTER_MATERIAL_AND_EFFECT_ROUND2_ESTIMATE_RESULT.md)와
이번 [실측 RESULT](C:/Users/user/Desktop/LostArk/.md/GB/09-09/2026-09-09_CHARACTER_SCALE_AND_MATERIAL_PARITY_RESULT.md)를 우선한다.

## G00. 비교 대상과 실측 기준

### 현재 크기

실제 Resources의 WModel 정점에 body inverse bind, 기본 battle idle의 첫 pose, skeleton hierarchy,
현재 loader의 단위 변환을 적용했다. 장비는 실제 렌더러처럼 body palette를 사용하고, 무기는
socket bone과 socket yaw를 적용했다. 기본 장착 상태와 숨겨진 body submesh를 반영했다.
Character/part의 추가 사용자 Transform은 identity인 생성 시점을 기준으로 한다.

| 캐릭터 | 현재 모델 단위 변환 | 무기 제외 높이 | 무기 제외 폭 | 모코코 높이까지의 외형 배율 |
|---|---:|---:|---:|---:|
| 모코코 창술사 | 0.0001 | 1.5001m | 0.4996m | 1.0000 |
| 도화가 | 0.0001 | 1.0385m | 0.4694m | 1.4444 |
| 워로드 | 0.0001 | 1.3155m | 1.0192m | 1.1403 |
| 차원술사, 헤어 제외 | 0.01 | 1.0682m | 0.2211m | 1.4043, 몸체 기준 참고값 |
| 건슬링어 | 0.0001 | 1.2481m | 0.4259m | 1.2019 |
| 슬레이어 | 0.0001 | 1.2392m | 0.8643m | 1.2106 |

높이는 해당 pose의 AABB이며 신체의 해부학적 키나 화면 픽셀 높이가 아니다.
모코코는 아바타 외곽을 포함한다. 모코코의 body 전체 rest 높이는 약 1.1221m이나,
아바타를 포함한 rest 높이는 1.4026m, battle idle 높이는 1.5001m다.
워로드는 이미 폭이 모코코의 약 두 배여서 높이를 맞춰도 같은 실루엣 크기가 되지는 않는다.
XYZ 비율을 유지하는 uniform scale을 적용하고 같은 높이를 초기 저작 기준으로 사용한다.

차원술사의 별도 헤어는 body palette 범위를 초과하는 index가 있어 측정에서 제외했다.
G01에서 이 불일치를 먼저 고친 뒤 동일한 식으로 헤어 포함 높이를 다시 계산한다.
따라서 1.4043을 차원술사 전체 아바타의 확정 배율로 저장하지 않는다.
사용자가 이후 직접 선택한 1.5배는 이 실측 비율과 별개의 외형 저작값이다.
이를 먼저 적용하고, 헤어 복구를 그 변경의 선행 조건으로 붙이지 않는다.

측정 정본은 [measurements.json](C:/Users/user/Desktop/LostArk/out/CharacterSizeAudit20260909/measurements.json)이며,
방법과 입력 파일 식별은 [측정 스크립트](C:/Users/user/Desktop/LostArk/out/CharacterSizeAudit20260909/measure_character_sizes.py)에 있다.
out 파일은 이번 조사 산출물이며 runtime 입력이나 새로운 배포 manifest가 아니다.

### 현재 재질 연결

| 범위 | catalog 모델 수 | 사용 material record 수 | source-character program 연결 | 현재 판정 |
|---|---:|---:|---:|---|
| 모코코 머리·몸 아바타 | 2 | 3 | 3 | program 1과 material-owned 2D IBL 연결 |
| 차원술사 body·장비·무기 | 6 | 19 | 18 | body의 숨겨진 헤어 포함. 실제 별도 헤어 1개는 일반 경로 |
| 도화가 body·장비·무기 | 8 | 21 | 0 | 일반 텍스처·조명 경로, 원본 program 미연결 |
| 워로드 body·장비·무기 | 9 | 14 | 0 | 일반 텍스처·조명 경로, 원본 program 미연결 |

이 분모는 catalog에 있는 모델들의 사용 material record 합계이며 한 프레임의 visible draw 수가 아니다.
차원술사의 17개 override 정의는 반복 MIC까지 포함해 18개 record에 적용된다.
도화가의 D/N/S 입력은 각각 21/20/16개, 워로드는 14/14/13개다. 현재 참조하는 물리 texture 누락은
두 클래스와 차원술사 모두 0개다. **재질 자체나 직접광 반사가 전혀 없는 상태는 아니다.**
원본 shader family의 추가 입력과 계산, scene 환경반사, 새 장비 소비자 연결이 불완전하다.

모코코의 `source.character.classic-skin.v1`은 `Character/SourceMaterials/efmaster_material_prologue/hdr07_1.tga`를
material 소유 2D IBL로 사용하며 IBL exposure 5, intensity 1과 피부/일반 표면 mask, Fresnel/rim 입력이 연결돼 있다.
이 수치나 2D texture를 세 클래스 전체에 복사하는 것은 각 원본 재질 복구가 아니다.

## G01. 차원술사 실제 헤어의 skeleton·UV·재질을 함께 연결

### 수정 위치와 책임

| 위치 | 이번 변경 책임 |
|---|---|
| `Character/DimensionMaster/Equipment/pc_sp_m_55_hair/head.wmodel` | 실제로 표시되는 헤어의 body rig 호환 정점 index와 원본 추가 UV 복구 |
| `Tools/ModelAssetConverter/`의 기존 변환·검증 경로 | 정점이 소비할 body skeleton의 이름·계층·bind 기준으로 cook 결과 검사 |
| `Client/Private/PlayableCharacterAssetService.cpp` | body와 skinned equipment의 palette 호환성을 실제 등록 단계에서 검사 |
| `Data/Actors/CharacterCatalog.json` | 실제 헤어 모델의 정확 material 이름에 source override 연결 |
| `Client/Private/Part_Equipment.cpp`, `Part_Body.cpp`, `DeferredMaterialRenderUtils.cpp`와 대응 H | body와 장비의 source material별 pass 선택을 기존 공통 경로로 맞춤 |
| `Engine/Private/Model.cpp` | 기존 source program/texture/extra UV 검사를 유지하고 실제 새 헤어 로드를 확인 |

Resources 경로는 모두 `Client/Bin/Resources/` 기준이다.

현재 `Logic_DimensionMaster.cpp`는 body submesh 9의 헤어를 숨기고 별도 equipment를 선택한다.
원본 program 7 override는 숨겨진 body 헤어에 있으며, 새 equipment에는 없다.
새 헤어는 WMESH minor 0/flags 31로 UV1/UV2가 없고 body는 minor 3/flags 415로 추가 UV가 있다.
`CModel::Apply_MaterialOverrides`는 program 7의 UV1, program 5의 UV1/UV2를 실제로 요구한다.
따라서 override만 복사하면 새 헤어의 model admission이 실패한다.

또한 별도 헤어에는 body palette 225개 범위를 넘는 index에 양수 weight를 가진 정점이
13,057개 있으며 최대 index는 235다. `WMeshReader`의 각 파일 자체 skeleton bounds 검사는 통과해도
`CPart_Equipment::Render_Pass`가 바인딩하는 body palette와는 맞지 않는다.
이는 실제 표현을 잘못 만들 수 있는 데이터 계약 불일치이며 이번 조사로 화면 증상이나 crash를 단정하지 않는다.

### 데이터와 함수 계약

새 runtime skeleton이나 두 번째 skinning 경로를 만들지 않는다. 기존 body rig를 정본으로 두고
헤어를 그 rig에 맞게 다시 cook한다. 정점 index를 정렬 순서나 가장 가까운 번호로 치환하지 않는다.
원본 bone 이름·계층·inverse bind와 실제 사용하는 weight를 연결하고, 필요 bone이 body rig에 없는
소스는 그대로 통과시키지 않는다. 필요한 경우 기존 body rig 및 그 rig를 쓰는 attachment/animation을
같은 변환 단위에서 재정합한다. 이때 기존 clip 이름·socket 이름·154개 clip 소비를 보존한다.

`CPlayableCharacterAssetService::Ensure_Registered`의 body 생성 이후, 장비 prototype commit 이전에
실제 body palette 호환성을 확인한다. 장비 자신의 bone 수와 index bounds만 검사하는 것으로 끝내지 않는다.
검사가 실패하면 해당 교체 candidate와 이번에 stage한 리소스를 정리하고, 살아 있는 character의
이전 장비·재질을 유지한다. 오류에 model asset ID와 bone 이름/불일치 종류를 남긴다.

`CPart_Equipment::Render_Pass`는 mesh별 `MODEL_SURFACE_FAMILY::SOURCE_CHARACTER`와 program을 읽어
body와 동일한 필요한 양면 pass를 선택한다. 현재 body는 program 6/7에 pass 6을 선택하지만,
equipment는 호출자가 넘긴 pass 0을 고정 사용한다. 일반 skinned 장비와 socketed weapon pass는
기존 의미를 유지하며, source hair의 main/shadow coverage가 동일 입력을 받게 연결한다.

### 반영 순서와 종료 증거

원본 헤어 rig·추가 UV 재구성 → body palette와 exact compatibility 확인 → fresh out cook →
실제 model/material 이름으로 override 구성 → main/shadow pass 연결 → CModel 생성·Clone·부착 확인 순서다.
기존 body hair를 다시 보이게 하거나 잘못된 index를 shader에서 clamp해 누락을 숨기지 않는다.
염색 mask, 숨김 mask, avatar head 교체 동작을 함께 보존한다.

종료 증거는 실제 새 헤어의 palette 위반 0, finite skinning, 필요한 UV 존재, source program 선택,
기존 socket/clip lookup 유지, 실패한 장비 교체 시 기존 appearance 보존이다.
이 단계가 끝나면 G00의 동일 CPU 측정으로 헤어 포함 높이와 최종 차원술사 배율을 산출한다.

## G02. CharacterCatalog가 외형 배율을 소유

### 수정 파일과 H 계약

- [ActorCatalog.h](C:/Users/user/Desktop/LostArk/Client/Public/ActorCatalog.h)의 `CHARACTER_ACTOR_ENTRY`에
  `presentationScale`을 추가한다. 단위는 무차원 uniform multiplier이며 finite, `0 < scale <= 100`을 요구한다.
  기존 boss entry의 presentation scale과 같은 뜻이다. asset의 cm/m 단위 변환값은 이 필드가 소유하지 않는다.
- [ActorCatalog.cpp](C:/Users/user/Desktop/LostArk/Client/Private/ActorCatalog.cpp)의 character parser는
  현재 format 4에 optional `presentationScale`을 추가하고 미지정 값은 명시 기본값 1을 사용한다.
  미등록 class나 손상된 값을 다른 class의 정상값으로 대체하지 않는다.
- [CharacterCatalog.json](C:/Users/user/Desktop/LostArk/Data/Actors/CharacterCatalog.json)의 차원술사 row에만
  `presentationScale: 1.5`를 명시한다. 다른 다섯 class는 기존 기본값 1을 유지한다.
  G00의 도화가 1.4444, 워로드 1.1403, 건슬링어 1.2019, 슬레이어 1.2106은 향후 비교용 제안이며 이번 적용값이 아니다.

`PlayableCharacterAssetService.cpp`의 차원술사 0.01/나머지 0.0001과 Y −90도는 유지한다.
차원술사의 100배 큰 입력 숫자는 다른 cook 단위 때문이며 실제로 100배 큰 캐릭터라는 뜻이 아니다.
무기의 model load는 identity이고 body socket을 통해 단위 변환을 이미 상속한다.

parser가 전체 candidate를 stage한 뒤 성공 시 catalog를 교체하는 현재 흐름을 유지한다.
잘못된 배율 때문에 이전 정상 catalog나 살아 있는 character를 지우지 않는다.
optional field 지원 reader와 JSON 변경을 같은 기능 commit에 포함하며 생성 bootstrap은 직접 수정하지 않는다.

종료 증거는 기존 미지정→1 호환 로드, 명시 scale 유지, 0/음수/NaN/상한 초과 거부와 기존 catalog 보존,
class별 body/장비/무기의 원래 asset unit 변환 유지다. 별도 scale 설정 파일은 추가하지 않는다.

## G03. CCharacter의 presentation root 하나로 body·장비·무기를 확대

### H 계약과 상태 수명

[Character.h](C:/Users/user/Desktop/LostArk/Client/Public/Character.h)에 다음 책임을 추가한다.

| 선언 | 소유하는 값과 수명 |
|---|---|
| `m_fPresentationScale` | 현재 character가 채택한 catalog 외형 배율. character instance 수명 |
| `m_PresentationRootMatrix` | `Scale(presentationScale) * gameplayWorld`로 갱신되는 안정된 멤버 행렬 |
| `Try_Get_PresentationRootMatrix` | part/animation/effect의 읽기 전용 visual root 조회. gameplay Transform을 반환하는 API와 분리 |
| `Update_PresentationRootMatrix` | 현재 승인된 gameplayWorld로 visual 행렬만 갱신하는 private 함수 |

부모 행렬 pointer는 local 임시값이나 vector element가 아닌 character 멤버를 가리킨다.
이 값은 Server entity pose, HP, movement, collision을 소유하지 않는다.

### CPP 호출 흐름

`CCharacter` 초기화에서 catalog 배율을 stage하고 최초 presentation root를 만든다.
body/기본 장비/weapon의 descriptor, 동적 equipment 교체가 전달하는 `pParentMatrix`를 모두 이 멤버로 바꾼다.
현재 기준점은 `Character.cpp`의 body 2813, equipment 2856, weapon 2886, 교체 equipment 1855 부근이다.

`CCharacter::Update`는 network transform/attachment 또는 local preview locomotion 반영이 끝난 뒤,
`__super::Update`가 part를 갱신하기 **전에** visual root를 갱신한다. 생성·teleport·class presentation 교체도
첫 draw 전에 같은 함수를 호출해 한 프레임의 옛 행렬이 섞이지 않게 한다.
socket matrix와 body palette에 scale을 다시 곱하지 않는다. 확대는 이 parent에서 정확히 한 번 적용한다.

현재 collider update는 gameplay Transform을 소비하므로 그대로 둔다.
Shared의 X/Z half extent 0.45m, Y 0.90m, center Y 0.90m, Server 이동 속도와 피격 범위를 변경하지 않는다.
UI의 보이는 크기를 키운다는 이유로 gameplay Transform 자체에 scale을 쓰지 않는다.

종료 증거는 body/교체 장비/무기의 상대 위치 유지, scale 1에서 기존 행렬 일치,
scale 변경 시 socket root 단일 상속, network pose와 collider/debug body 수치 불변이다.
점프와 animation root의 수직 이동 표현은 외형 parent 안에서 함께 확대되므로 사용자 동작 확인에 포함한다.

## G04. Animation Tool·Effect anchor·afterimage의 외형 root 연결

### 수정 파일과 함수 책임

| 파일/함수 | 변경 |
|---|---|
| `AnimationTargetService.cpp::Resolve_RootTransform` | scene/preview Character에서 새 presentation root를 반환 |
| 같은 파일의 bone anchor 및 historical pose 소비 | 기존 root 공유를 유지해 live/과거 pose가 같은 scale을 소비하도록 확인 |
| `Effect_PresentationService.cpp::EFFECT_OWNER_VIEW::Try_Get_PresentationRoot` | Character의 새 getter로 변경 |
| 같은 owner view의 `Try_Get_OwnerWorld` | gameplay pose 의미를 그대로 유지 |
| `CharacterPreviewPanel.cpp` | 현재 CPlayableCharacterAssetService→CCharacter 생성 경로의 결과를 소비하고 별도 scale을 중복 적용하지 않음 |

Animation Tool의 playable Model View는 현재 별도 raw model runtime이 아니라 같은 asset service와
CCharacter를 사용한다. `AnimationPreviewAssets.h`의 0.01 기본값을 이 경로에 다시 곱하지 않는다.

hand/weapon 부착 effect, trail, afterimage의 anchor는 확대된 외형과 일치해야 한다.
기존 world-size 또는 owner-unit 정규화 정책이 있는 effect는 그 정책을 유지한다.
ROOT/WORLD/target ground decal과 Server combat telegraph까지 일괄 확대하지 않는다.
각 descriptor의 anchor/follow/scale policy가 실제 최종 matrix에 적용되는 위치에서,
bone 위치는 presentation root를 사용하고 world 단위 길이·damage area는 기존 단위를 보존한다.
asset unit 정규화를 presentationScale로 다시 해석하거나 두 번 적용하지 않는다.

현재 `Resolve_Anchor`의 root 분기와 기본 `OWNER_RELATIVE` policy 때문에 getter만 교체하면
root effect 전체까지 확대될 수 있다. 반대로 `Resolve_SourceAnchors`는 policy를 먼저 적용한 root로
bone 위치를 계산하므로 scale 제거가 손의 위치까지 되돌릴 수 있다.
따라서 root/skill_target/world effect는 기존 pose·월드 크기를 유지하고, bone effect는 확대된 bone 위치와
해당 effect의 크기 정책을 각각 확인한다. 이 차이를 처리하는 데 필요한 기존 owner/policy 분기만 보강한다.

종료 증거는 scene/Model View의 같은 class·avatar·pose에서 같은 visual bounds,
손/무기 effect anchor 위치 일치, afterimage 과거 pose scale 일치, world-space 판정 크기 불변이다.
기존 Effect 실행 경로의 focused 입력으로 확인하며 새 전용 렌더러나 전수 admission 하네스를 추가하지 않는다.

## G05. 새 헤어 DDS의 축소용 mip 복구

확인된 물리 파일은 다음 세 쌍이다. 모두 현재 1024×1024, 1,048,704 bytes, header mipCount 0으로
실질적으로 mip 한 단계만 가진다.

| 대상 | Resources 상대 폴더 | 파일 |
|---|---|---|
| 도화가 | `Character/Artist/Equipment/pc_sp_00_hair/textures/` | `pc_sp_hair_00_d.dds`, `pc_sp_hair_00_n.dds` |
| 차원술사 | `Character/DimensionMaster/Equipment/pc_sp_m_55_hair/textures/` | `pc_sp_m_hair_55_d.dds`, `pc_sp_m_hair_55_n.dds` |
| 창술사 기본 헤어 | `Character/LanceMaster/Equipment/pc_ft_00_hair/textures/` | `pc_ft_00_hair_d.dds`, `pc_ft_00_hair_n.dds` |

모코코 head가 가리는 창술사 기본 헤어도 그 장비를 표시하면 같은 문제를 갖는다.
기존 source TGA와 차원술사 weapon DDS mip 복구는 이미 반영돼 있어 다시 전체 texture를 변환하지 않는다.

기존 texture 변환 도구를 사용해 source color space/normal encoding에 맞는 전체 mip을 생성한다.
1024→1 체인은 11단계이며 원본 mip0의 데이터와 opacity/normal 채널 의미를 보존한다.
normal mip은 원본 encoding을 복원해 벡터를 필터링·정규화한 뒤 같은 encoding으로 저장한다.
generic RGB averaging으로 normal이나 hair coverage를 바꾸지 않는다.

fresh out 출력에서 DDS header, 단계별 byte range, mip0 보존과 실제 SRV mip 수를 확인한 다음
위 여섯 파일만 설치한다. Resources는 Drive 전달 대상이며 Git force-add나 별도 Resource manifest를 만들지 않는다.
화면에서 먼 거리 반짝임이 개선됐다는 판정은 사용자가 한다.

## G06. scene 환경반사와 간접광을 기존 RenderingProfile→Renderer 경로에 연결

### 확인된 누락

차원술사 program 3 의상과 program 8/9 무기, 합계 13개 material record의 원본 texturecube sample이
현재 [Shader_SourceCharacterPrograms.hlsli](C:/Users/user/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_SourceCharacterPrograms.hlsli)의
6365/8977/9791 부근에서 `float4(0,0,0,0)`으로 대체돼 있다.
shader program 연결과 직접광/ORM 계산이 존재해도 이 환경반사 입력은 실제로 0이다.
[Shader_Deferred.hlsl](C:/Users/user/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl)의 현재 ambient도
원본 scene SH/cube가 완전히 연결된 결과가 아니다.

현재 source-character texture 슬롯은 Texture2D용이며 `CModel`은 source override에 map의
`hasEnvironmentCube`를 넣는 것을 거부한다. 모코코의 재질 소유 2D IBL과 scene 소유 cubemap은
입력의 소유자가 다르므로 이 검사를 풀고 모든 material에 같은 cube를 끼우는 방식으로 처리하지 않는다.

### 수정 파일과 typed 입력

- `Client/Public/RenderingProfileService.h`의 `SCENE_RENDERING_PROFILE`에 optional scene environment 입력을
  추가한다. Resources 상대 cubemap asset ID, 해당 원본의 decode/exposure 입력, SH 계수를 typed 값으로 소유한다.
  Engine은 class/level 이름을 모르고 generic render environment만 소비한다.
- `RenderingProfileService.cpp`의 parser/serializer와 `Data/Rendering/Authored/RenderingProfiles.json`,
  `Tools/RenderingPipeline/Publish-RenderingProfiles.ps1`을 같은 schema 변경으로 연결한다.
  기존 format 1은 environment 없음으로 명시 migration하며 과거 프로파일의 직접광·안개·노출을 보존한다.
- `Engine/Public/Renderer.h`, `Engine/Private/Renderer.cpp`와 기존 `GameInstance` typed 전달 경계에서
  scene environment SRV/constants의 stage·commit·수명을 소유한다. 별도 환경광 manager를 만들지 않는다.
- `Engine/Bin/ShaderFiles/Shader_SourceCharacterPrograms.hlsli`, `Shader_SourceCharacterMaterial.hlsli`,
  `Shader_Deferred.hlsl`에서 원본 sample 방향/LOD/decode와 SH 소비를 연결한다.
  Client 쪽 shader는 기존 Engine SDK/deploy 경로로 배포한다.

`CRenderingProfileService::Commit_Resolved`의 기존 light/shadow/fog/quality 교체에 environment를 포함한다.
새 scene의 모든 입력과 GPU resource를 먼저 stage하고, 한 입력이라도 실패하면 이전 environment와
light/shadow/fog/quality를 모두 유지한다. level 전환 뒤 이전 scene cube만 남는 부분 commit을 허용하지 않는다.
optional environment가 없는 기존 profile은 기존 경로로 로드하되 그 상태를 원본 간접광 복구 완료로 기록하지 않는다.

실제 출발 입력은 이미 존재한다. `Map/Lighting/CharacterSelect/lv_lut_valhatrond_04_hdr01.rgbm.cube.dds`는
현재 Resources에 설치된 128×128×6, 8 mips, RGBM6 cube다.
[lighting_binding_inputs.json](C:/Users/user/Desktop/LostArk/out/CharacterSelectRestore20260908/lighting_binding_inputs.json:823)의
environmentCube와 같은 파일 1531행의 `incidentLightingSH9PerChannel`에 추출 근거가 있다.
이는 Character Select map component의 확인된 입력이며 모든 캐릭터·world의 원본 환경 선택을 증명하지 않는다.
[기존 조사](C:/Users/user/Desktop/LostArk/out/CharacterSelectRestore20260908/lighting_binding_findings.md:59)에 남은
native t5 LUT identity, cb12/13, hemisphere22/23/24, SH9→shader seven-float4 packing을 확인하는 작업도 포함한다.
현재 project BRDF LUT를 native 원본이라고 표기하지 않는다.

이 실재 source에서 각 scene의 probe/cube/SH 선택과 decode 근거를 확인해 profile에 연결하는 작업까지 이 G에 포함한다.
임의 맵 cubemap을 Character Select·Bern·발탄·쿠크 전체의 원본이라고 지정하지 않는다.
원본 근거가 없는 조명값을 조정해야 할 때에는 해당 값은 프로젝트 저작값으로 구분한다.

종료 증거는 texturecube가 실제 cube SRV를 샘플함, 원본 mip/LOD와 decode 입력 전달,
finite 출력, 환경 선택 변경과 실패 시 전체 이전 profile 보존, 모코코의 기존 2D IBL 회귀 보존이다.
직접광 증폭이나 전역 exposure 증가로 cube 누락을 가리지 않는다.

## G07. 도화가·워로드의 실제 material slot 전체를 source family에 연결

### 수정 위치와 연결 단위

현재 catalog의 도화가 8모델/21사용 record, 워로드 9모델/14사용 record를 작업 분모로 둔다.
body, 실제 별도 헤어, 피부·눈·속눈썹, 의상, weapon/shield를 각각 원본 material identity로 조사한다.
같은 MIC가 반복되는 슬롯은 정의를 재사용하되 실제 표시되는 model 소비자가 연결돼야 완료다.
숨겨진 body에만 override를 붙이고 장착 중인 equipment를 빠뜨리지 않는다.

`CharacterCatalog.json`의 named override → `CActorCatalog::Build_ModelLoadDescription` →
`CPlayableCharacterAssetService` → `CModel` → `CMaterial::Bind_SourceCharacterInputs` →
`Part_Body/Part_Equipment` → geometry/G-buffer → `Renderer`의 source light pass를 기존대로 확장한다.

정확 MIC와 parent/static parameter, PS/VF, texture channel, sampler/color space를 원본 package/cache에서
연결한 다음 현재 program과 같은 계산인지 판정한다. 같으면 기존 program과 typed constant/texture 입력을
재사용하고, 다르면 기존 source-character shader/parameter 정의에 필요한 실제 family만 추가한다.
`Client/Public/SourceCharacterMaterialParameters.h`, `Engine/Public/BinaryAsset/ModelAssetData.h`,
`Engine/Private/Material.cpp`, `Engine/Private/Model.cpp`의 program 범위·입력 검사를 함께 갱신한다.
모코코 program 1을 class 전체의 fallback으로 쓰지 않는다.

추출 출발점은 [family_scope.json](C:/Users/user/Desktop/LostArk/out/CharacterMaterialReview20260908/family_scope.json),
[source_material_matches.json](C:/Users/user/Desktop/LostArk/out/CharacterMaterialReview20260908/source_material_matches.json)의
current slot→source MIC candidate→parent chain이다. 도화가 face의 실제 leaf는
`out/CharacterEquipmentExtraction/RawShared/PC_SP/PC_SP_00_FACE/mat/pc_sp_face_mi_high.props.txt`,
워로드 lower는 `out/CharacterEquipmentExtraction/RawShared/PC_WR/PC_WR_00/mat/pc_wr_00_lower_mi.props.txt`다.
기존 `out/CharacterMaterialRestore20260908/full_source_programs.py`와 `full_source_program_index.json`의
17 targets는 모코코·창술사 무기·차원술사이며 도화가/워로드 현재 일반 outfit의 exact PS/VF 완료 자료는 아니다.
그 extractor의 입력을 두 클래스의 실제 visible MIC/static-set/VF로 확장해 정확 permutation을 확보한다.
`pc_wr_av_036` 모코코 reference costume을 워로드의 현재 일반 의상 완료 실적으로 세지 않는다.

원본이 요구하는 추가 UV, skin/variation/roughness/normal/opacity 입력이 현재 모델에 없으면
G01과 같은 기존 cook 경로에서 필요한 해당 모델만 복구한다. D/N/S 숫자가 적다는 이유만으로 texture를
복제해 슬롯 수를 채우지 않는다. 원본에서 해당 texture가 선택되지 않는 재질은 정상적인 비사용으로 구분한다.

### 실패 보존과 검증

교체 candidate의 model, texture, shader program, extra UV와 source parameter를 모두 stage한 뒤
실제 character appearance를 교체한다. 잘못된 override 하나 때문에 기존 클래스 appearance를 삭제하지 않는다.
실패 사유는 model asset ID/material 이름/요구 입력까지 남기고 정상 이전 appearance를 유지한다.

각 material은 exact source mapping, 실제 CMaterial 바인딩과 draw pass 선택,
body/장비/socket 소비, alpha·shadow·염색 적용을 확인한다.
숫자 검증은 원본 shader와 같은 입력의 대표 조건에서 실시하고 실제 피부·금속·천·머리카락이
모코코와 같은 장면 조명에 자연스럽게 반응하는지는 사용자가 판정한다.
도화가·워로드의 스킬 Effect shader 복구는 캐릭터 표면 material의 복구 증거를 대신하지 않는다.

## G08. 투명 표면·그림자·장면 표현의 남은 경계

현재 native hair/eyelash는 ordered coverage로 sorted transparency를 근사한다.
원본과 동일한 투명 겹침이 필요한 material은 기존 alpha/blend와 shadow pass에 정확 입력을 연결하고,
원본 VF/PS가 요구하는 geometry/normal/coverage를 맞춘다. 새 캐릭터 전용 렌더러를 추가하지 않는다.
원본 scene SH/IBL, exposure/tonemapping, shadow PCF 차이는 캐릭터 material 연결과 별도로 기록한다.

이 G는 단순히 모든 slot에 source program이 들어갔다는 이유로 “원작과 완전히 동일”을 선언하지 않기 위한
남은 실제 표현 작업이다. 알파 가장자리·옆/뒤 면·머리와 속눈썹 겹침·그림자·노출을 동일 조건에서 확인하고,
차이가 shader 입력인지 scene setting인지 구분해 해당 기존 소유자에서 수정한다.
모코코도 자동 visual PASS 기준이 아니라 사용자가 선택한 비교 대상이다.

## G09. 구현 순서·프로젝트 등록·완료 판정

사용자 후속 지시에 따라 **G02→G03→G04의 차원술사 1.5배 외형 scale을 먼저 적용**한다.
root 일부만 바뀐 상태를 최종 반영본으로 인계하지 않는다. 기존 헤어 rig의 결함과 재질 상태는 이번 확대와 구분해 남긴다.
그다음 재질 복구 기능은 G01의 차원술사 실제 헤어 호환성 및 G05 mip 복구부터 진행한다.
G06 scene 입력과 G07 material mapping은 조사·작성은 병행할 수 있지만 최종 draw는 두 입력을 함께 소비해야 한다.
G08의 표현 차이와 사용자 확인을 마지막까지 RESULT에 구분한다.

새 C++ 파일은 현재 계획에 없다. 기존 H/CPP를 확장하므로 `.vcxproj/.filters` 신규 등록은 필요하지 않다.
실제 구현 중 새 shader include가 필요한 경우 Engine 정본과 Client 배포/프로젝트의 필요한 항목만 등록한다.
현재 존재하는 물리 폴더와 필터를 재배치하지 않는다. 기존 C++ 인코딩을 보존한다.

| 변경 | 필요한 확인 |
|---|---|
| CharacterCatalog/RenderingProfiles | 변경 JSON parse, 해당 parser/serializer round trip, 실패 시 이전 상태 유지 |
| body/장비/visual root | 변경 Client 최소 컴파일, 실제 CModel load/Clone/attachment 및 행렬 입력 확인 |
| source material/scene GPU binding | 변경 Engine/Client 최소 컴파일, 필요한 HLSL 컴파일, 실제 입력 바인딩·finite 출력 확인 |
| Resources | 지정 파일 존재, WModel/texture parse와 palette/UV/mip 수, 이전 정상 appearance 보존 |
| 최종 코드·데이터 | `git diff --check`, 실제 변경한 파일의 diff와 프로젝트 등록 확인 |
| 최종 제품 배포 | 기존 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`의 Product 순차 빌드·배포 |
| Client 화면 | 사용자가 직접 크기·부착·재질·장면 비교. 에이전트는 실행·조작·캡처·visual PASS를 하지 않음 |

일반 저장·컴파일에 전체 oracle이나 새 별도 하네스를 선행 조건으로 붙이지 않는다.
기존 focused 검사와 실제 소비자 검증으로 기능을 확인하고, 실행하지 않은 검사를 PASS로 쓰지 않는다.

사용자 화면 경로는 `Server + Client` profile에서 직접 `Ctrl+F5` → Lobby → Character Select →
도화가/차원술사/워로드 선택 순서다. 같은 camera와 RenderingProfile에서 모코코 창술사와 비교한다.
현재 저장된 Character Select 카메라는 Y 6m/Z 4m, pitch 55°, FOV 70°다.
카메라 zoom 변경과 외형 배율을 함께 바꿔 크기 차이를 숨기지 않는다.
F1의 Animation Tool에서 같은 클래스 Model View, 장비 교체, 기본 이동·공격·손/무기 Effect를 확인하고,
Server 승인 Bern/Valtan/Kouku 진입에서도 같은 visual root가 소비되는지 확인한다.

인계에는 바뀐 Resources 상대 asset ID와 `Client/Bin/Resources/` 실제 위치, Drive 전달 준비 여부를 기록한다.
Resource 교체·Drive 전달·사용자 화면 확인은 미실행이며, 차원술사 scale 코드의 실제 완료/빌드 상태는 RESULT를 따른다.
