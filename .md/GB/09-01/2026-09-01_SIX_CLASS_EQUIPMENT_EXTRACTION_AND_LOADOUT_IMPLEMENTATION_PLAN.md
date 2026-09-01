# 6개 클래스 장비 리소스 정규화·레이드 보상·인벤토리 장착·계정 저장 구현 계획서

작성일: 2026-09-01

문서 종류: 구현 계획서

대상 클래스: 차원술사, 도화가, 창술사, 워로드, 슬레이어, 건슬링어

현재 구현 상태: 원본 추출과 source inventory, 기존 Cook 자산 기반 6-class 대표 admission pack,
검증된 runtime closure의 실제 Resources 승격, F1 Equipment Authoring Tool의 preview-only 슬롯 교체까지
완료. Server item/reward/inventory와 account durable 저장은 미착수

대응 결과서: `2026-09-01_SIX_CLASS_EQUIPMENT_EXTRACTION_AND_LOADOUT_RESULT.md`

이번 PR은 최종 Server 권위 흐름 전체가 아니라 그 앞단의 검증 가능한 저작·표현 슬라이스를 닫는다.
1차 대표 12 visual set/17 named part에 가시 외형 보강분을 더한 최종 22 visual set/36 named part를
6개 stable slot으로 분류하고, 기존 공용 Character preview에서
장착·해제·원자 교체와 툴 전용 preset 저장을 확인할 수 있게 한다. `authoringOnly` preset은 회원정보,
Server inventory 또는 재접속 저장 계약으로 승격하지 않는다.

### 2026-09-01 가시 외형 보강 슬라이스

첫 대표 pack의 baseline weapon은 원래 기본 무기와 byte-identical하여 `Equip Selected`를 눌러도
외형 변화가 없었다. 이번 보강은 어제 추출한 raw 전체를 다시 감사한 뒤, 골격 정규화가 필요 없는
static/socketed weapon만 `ModelAssetConverter --pretransform --scale 100`으로 새로 Cook하여 여섯 클래스에
각각 하나의 `READY_ALTERNATIVE` 무기 세트를 추가한다. 각 Cook은 exact inventory source, 명시적 material
texture binding, converter hash와 인자, static/no-skeleton/no-animation 계약, texture closure, 기본 무기와
다른 geometry를 admission receipt에 남긴다.

추가 대상은 창술사 `WP_WFLM_07` L/S, 건슬링어 `WP_WGDH_TF01_05` H dual-pistol,
슬레이어 `WP_WWBK_F_TF01_07`, 도화가 `WP_WSDM_TF01_06`, 워로드 `WP_WWGL_18` main/shield,
차원술사 `WP_WSWP_M_TF01_07` E/L/P/S다. 기존 Cook 호환성이 검증된 창술사 표준 head/outfit,
도화가 lower, 워로드 hair도 별도 `READY_ALTERNATIVE` 의상 set으로 승격한다. 기본 preset은 모든 slot을
비워 두어 Reset 상태와 Equip 이후의 차이가 바로 보이게 한다.

기존 publisher의 12 set/17 part/73 file 고정 분모와 Equipment-directory 전체 충돌 정책은 제거한다.
receipt 내부 실제 count와 모델 closure를 상호 검증하고, 이미 배포된 managed 파일은 hash가 같을 때만
재사용하며 신규 파일은 전체 staging 검증 뒤 per-file 원자 승격한다. 중간 실패는 이번 호출에서 새로
승격한 파일을 전부 rollback한다. raw skinned apparel은 master-skeleton normalizer와 weighted palette
검증이 닫히기 전까지 Resources에 옮기지 않는다.

이 슬라이스의 종료 증거는 Python domain tests, 6개 신규 무기 WModel의 static decode와 texture closure,
각 클래스 기본 무기 대비 다른 vertex/hash, publisher Validate/Publish, Catalog/preset contract,
Client Product build와 `git diff --check`다. Client 실행과 시각 PASS는 사용자가 직접 확인한다.

## 0. 목표와 종료 상태

이 계획의 최종 목표는 다음 한 흐름을 Server 권위 수직 슬라이스로 닫는 것이다.

```text
LostArk 원본 SkeletalMesh
→ 클래스 master skeleton 기준 정규화와 WModel admission
→ stable item/visual-set catalog
→ Valtan clear의 class별 weighted random roll
→ Server 소유 item instance와 per-class loadout
→ Inventory UI의 장착/해제 명령
→ local/remote Character의 atomic part 교체
→ world transfer와 late join 재현
→ 인증된 account/character의 durable 저장과 재접속 복원
```

종료 시에는 다음이 모두 성립해야 한다.

- 여섯 클래스마다 기본 body, 의상 파츠, 외형 파츠, 무기 세트가 stable ID로 구분된다.
- raw glTF나 직접 변환한 호환 불명 WModel은 `Client/Bin/Resources`에 들어가지 않는다.
- skinned 의상은 모든 positive-weight blend index가 클래스 body의 같은 index/name으로 resolve되는
  1-mesh WModel로만 admission된다.
- 하나의 아이템이 여러 메시를 요구해도 하나의 `visualSetId`와 `parts[]`로 원자적으로 장착된다.
- 발탄 클리어 보상은 고정 전부 지급이 아니라 Server가 class pool에서 weighted roll한다.
- 장비는 고유 `itemInstanceId`를 갖고 중복 보유할 수 있다.
- 장착과 해제는 Inventory 소유권을 바꾸지 않고 per-class loadout reference만 바꾼다.
- 같은 플레이어가 class를 바꾸면 해당 class의 기존 loadout이 복원된다.
- 교체 중 하나의 model, texture, skeleton, socket 검증이라도 실패하면 이전 시각 상태를 유지한다.
- remote player와 late joiner도 Server snapshot 하나로 같은 장비 외형을 재현한다.
- 레이드 보상 획득과 장착 변경은 인증된 `accountId + characterId`의 durable revision에 원자적으로
  저장되며 Server 재시작과 재접속 뒤 복원된다.

사용자가 확인한 핵심 목표에 따라 계정 DB 저장과 재접속 영구 저장을 종료 조건에 포함한다. 현재
저장소에는 login/auth packet, account repository, DB adapter가 없으므로 process-session 장비 기능과
durable persistence를 서로 다른 G로 구현하되 최종 완료는 둘 다 통과해야 한다. 거래, 경매장, 강화
수치, 랜덤 옵션, 장비 능력치 적용, 염색 UI는 계속 범위 밖이다. 저장값에는 model path, PartTag,
Prototype tag, mesh/vector index를 넣지 않고 `itemId`, `itemInstanceId`, stable slot만 사용한다.

## 1. 현재 원본 추출 실측

### 1.1 정본 산출물

현재 추출 정본은 다음과 같다.

- raw 출력: `out/CharacterEquipmentExtraction/Raw`, `out/CharacterEquipmentExtraction/RawShared`
- 전체 source object 표: `out/CharacterEquipmentExtraction/Inventory/Generated/character-equipment-source-inventory.csv`
- 같은 표의 JSON: `out/CharacterEquipmentExtraction/Inventory/Generated/character-equipment-source-inventory.json`
- package별 저장 후보: `out/CharacterEquipmentExtraction/Inventory/Generated/character-equipment-visual-sets.json`
- 사람이 읽는 전체 package 표: `out/CharacterEquipmentExtraction/Inventory/Generated/character-equipment-source-inventory.md`
- 추출 receipt: `out/CharacterEquipmentExtraction/character-equipment-extraction.receipt.json`

`New-CharacterEquipmentSourceInventory.ps1`은 class allowlist와 exact package directory를 사용해
dependency export를 걸러낸다. `Raw`와 `RawShared` 같은 물리 scope는 ID가 아니며, 저장 후보의
canonical key는 `character.<class>.<exact-package-family>`다.

### 1.2 클래스별 분모와 교체 후보

아래 수치는 아이템 수가 아니라 SkeletalMesh object 수다. locale/high/numbered 변형과 base/helper를
포함하므로 이 숫자를 그대로 raid reward item 개수로 승격하지 않는다. `수동 검토`는 앞의 후보 수에
포함된 부분집합이다.

| 클래스 | source object | 추출 | 차단 | package/visual set | 의상 후보 | 외형·base 후보 | 무기 후보 | 수동 검토 | 제외 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 창술사 | 1,518 | 1,516 | 2 | 428 | 740 | 732 | 40 | 445 | 6 |
| 건슬링어 | 1,408 | 1,408 | 0 | 360 | 644 | 647 | 116 | 421 | 1 |
| 슬레이어 | 1,220 | 1,211 | 9 | 401 | 512 | 659 | 45 | 389 | 4 |
| 도화가 | 1,361 | 1,361 | 0 | 385 | 667 | 675 | 18 | 408 | 1 |
| 워로드 | 1,514 | 1,514 | 0 | 437 | 721 | 721 | 65 | 442 | 7 |
| 차원술사 | 454 | 454 | 0 | 148 | 186 | 210 | 52 | 123 | 6 |
| 합계 | 7,475 | 7,464 | 11 | 2,159 | 3,470 | 3,644 | 336 | 2,228 | 25 |

2,159개 visual set에는 추출이 전부 차단된 세 package도 포함된다. 실제 glTF가 하나 이상 있는
package-family는 2,156개다. `runtimeAdmittedCount`는 현재 0이다.

### 1.3 추출 완료와 차단 경계

- raw와 dependency 출력은 93,063 files, 7,863 glTF, 30,793,114,057 bytes다.
- class allowlist로 거른 정확한 원본 분모는 7,475 SkeletalMesh다.
- 7,464개는 glTF로 추출됐고 11개는 최신 LostArk UModel build 1544 fix2에서도
  `USkeletalMesh3::Serialize -> FStaticLODModel3 -> Serializing behind stopper`로 차단됐다.
- 최소 glTF, PSK, MD5Mesh, 물리 UPK 직접 지정 재시도도 11개 모두 exit 1, output 0이었다.
- 원본 UPK size/MD5는 설치 manifest와 일치하므로 로컬 설치 손상이 아니다.
- 차단된 것은 창술사 `PC_FT_AV_247` 2개와 슬레이어 `PC_WR_F_AV_220A/B` 9개다.

차단 object는 inventory에서 삭제하지 않고 `BLOCKED_SERIALIZATION`으로 보존한다. 새로운 LostArk
UModel 또는 해당 LOD layout parser가 나오기 전에는 incomplete visual set으로 제품 admission하지 않는다.

## 2. 클래스별 교체·저장 해석

### 2.1 창술사

- master/base family: `PC_FT_00`
- class apparel: `PC_FLM_*`
- 공유 여성 Fighter apparel/appearance: `PC_FT_*`, 단 `PC_FT_M_*` 제외
- 기본 장비 파츠 힌트: `arm`, `helmet`, `lower`, `shoulder`, `upper`
- hair/face preset: `PC_FT_*_HAIR`, `PC_FT_*_FACE`
- 무기 visual set: 같은 `WP_WFLM_*` package의 `L + S`
- 필수 예외: `PC_FT_AV_247`은 dress/head가 모두 차단됐으므로 저장 후보에서 격리한다.

창술사 기존 runtime body/part는 224 cooked bones다. raw upper는 221 joints이고 direct WModel은
222 bones라서 직접 장착할 수 없다. 모든 shared/class apparel을 창술사 master skeleton으로 재바인딩해
동일 224-entry palette와 weighted-index mapping을 만든 뒤에만 `CPart_Equipment`가 body palette를
공유할 수 있다.

### 2.2 건슬링어

- master/base family: `PC_GN_F_00`
- class apparel: `PC_GDH_F_*`
- 공유 Gunner female apparel/appearance: `PC_GN_F_*`
- 무기 visual set: 같은 `WP_WGDH_*` package의 `H + L + S`
- `PC_GN_F_00_HAIR`는 현재 원본에 없으므로 존재한다고 가정하지 않는다.

무기 package 36개는 H/L/S 필수 역할을 모두 갖지만 `WP_WGDH_13`, `WP_WGDH_20`,
`WP_WGDH_F_00`처럼 high/numbered 추가 변형이 있다. publisher는 하나를 임의 선택하지 않고 authored
`parts[]`가 어느 object를 쓸지 요구한다.

### 2.3 슬레이어

- master/base family: `PC_WR_F_00`
- class apparel: `PC_WBK_F_*`
- 공유 여성 Warrior apparel/appearance: `PC_WR_F_*`
- 무기 visual set: `WP_WWBK_*`의 main weapon
- 차단 family: `PC_WR_F_AV_220A`, `PC_WR_F_AV_220B`
- quarantine: `PC_WR_F_AV_142_TEST`, backup/high/old member

`PC_WR_F_AV_220A/B`는 face/head/lower/upper 9개가 전부 차단됐다. 다른 package의 외형을 대신 붙여
완료 처리하지 않는다. `WP_WWBK_14`는 같은 basename의 default/high 파일이 서로 다른 binary이므로
quality variant 선택을 저작해야 한다.

### 2.4 도화가

- master/base family: `PC_SP_00`
- class apparel: `PC_SDM_*`
- 공유 여성 Specialist apparel/appearance: `PC_SP_*`, 단 `PC_SP_M_*` 제외
- 무기 visual set: `WP_WSDM_*` main weapon

`HR`은 hair 약자가 아니다. `PC_SDM_HR_*`와 shared `HR` package 안의 helmet/upper/lower/dress는
의상 family로 분류하고, 실제 hair object만 appearance preset으로 분리한다.

### 2.5 워로드

- master/base family: `PC_WR_00`
- class apparel: `PC_WGL_*`
- 공유 남성 Warrior apparel/appearance: `PC_WR_*`, 단 `PC_WR_F_*` 제외
- 무기 visual set: 같은 `WP_WWGL_*` package의 `MAIN + SD(shield)`
- quarantine: `PC_WR_AV_163A_UPPER_TEST`, `PC_WR_AV_163A-1_UPPER_TEST`, backup member

shield object 이름은 `sd_03`, `03_sd`, `sd_tf01`처럼 위치가 일정하지 않다. suffix 추측 대신
authored `partRole: SHIELD`를 저장한다. `WP_WWGL_19`처럼 다섯 mesh가 있는 package도 member 선택이
명시되기 전에는 자동 admission하지 않는다.

### 2.6 차원술사

- master/base family: `PC_SP_M_00/pc_sp_m_00_sk`, raw 기준 222 joints
- 공유 남성 Specialist apparel/appearance: `PC_SP_M_*`
- class apparel: `PC_SWP_M_02`, `PC_SWP_M_03`, `PC_SWP_M_AV_137`, `PC_SWP_M_HR_00`
- 무기 visual set: 같은 `WP_WSWP_M_*` package의 `E + L + P + S`
- 금지: `PC_SWP_M_00/pc_swp_fx01_sk`는 body가 아니라 FX rig
- quarantine: `WP_WSWP_M_00_TEST`

`Raw/DimensionMaster/PC_SP_M_*` 129 package, 364 glTF는 `RawShared/PC_SP/PC_SP_M_*`와 SHA-256까지
동일하다. source path를 ID에 넣지 않고 하나의 canonical visual set으로만 저장한다.

현재 `CharacterCatalog.json`의 차원술사는 `equipmentModels: []`이고 의류가 body에 합쳐져 있다.
따라서 다른 클래스와 같은 장착 구조를 열기 전에 base body와 기본 garment를 분리하고, 기존 캐릭터
생성 실패 rollback을 유지하는 G가 반드시 선행돼야 한다.

## 3. source 분류와 제품 admission은 별개다

source inventory는 다음 역할만 판정한다.

| source 분류 | 의미 | 제품 저장 후보 |
|---|---|---|
| `BASE_BODY_COMPONENT` | 클래스 master와 기본 loadout 기준 | raid item이 아닌 Character base 계약 |
| `APPAREL_*` | upper/lower/hands/shoulder/helmet/dress 후보 | `EQUIPMENT_VISUAL_SET` |
| `FACE_APPEARANCE`, `HAIR_APPEARANCE` | 캐릭터 외형 preset 후보 | 초기 raid pool 제외 |
| `HEAD_OR_HAIR` | 이름만으로 helmet/hair/deco 구분 불가 | coverage 수동 저작 후 결정 |
| `WEAPON_*` | package 단위 다중 무기 파츠 | `EQUIPMENT_VISUAL_SET` |
| `FX_HELPER` | FX/shadow/grab/helper rig | 제품 장비 금지 |
| `QUARANTINED_VARIANT` | TEST/CREATION/old/backup | 자동 admission 금지 |
| `BLOCKED_SERIALIZATION` | extractor가 geometry를 읽지 못함 | parser 갱신 전 금지 |

`_loc_int`, `_usa`, `_chn`은 별도 item이 아니라 locale variant다. `mesh_high`, `_high`도 별도 item이
아니라 quality variant다. `lower1`, `lower-1`, `head-1`의 ordinal은 원문 그대로 보존한다.
`shouder`, `haed` 같은 오타는 alias로 role만 교정하고 source object ID는 바꾸지 않는다.

제품 admission은 아래를 모두 통과해야 한다.

1. source object가 class allowlist에 속한다.
2. weighted joint name이 class master skeleton의 subset이다.
3. class master로 재바인딩한 cooked WModel은 모든 positive-weight blend index에서 body의 같은
   index/name hash를 참조한다. 기존 valid part가 가진 unweighted synthetic source-mesh node 한 개는
   이름이 달라도 weight/socket/animation에서 미참조임을 증명한 경우에만 허용한다.
4. skinned part는 1 WModel = 1 mesh 계약을 만족한다.
5. inverse bind와 bind-pose bounds가 허용 오차 안에 있다.
6. socketed part는 socket bone과 yaw/scale profile이 존재한다.
7. visual set의 필수 member가 모두 있고 slot/coverage 충돌이 없다.
8. locale/quality variant가 정확히 하나 선택돼 있다.
9. 모든 material과 texture가 Resources-relative 경로로 닫힌다.
10. `..`, absolute path, drive-qualified path, `Resources/LostArk` wrapper를 거부한다.

## 4. 저장 계약

### 4.1 장비 슬롯

첫 수직 슬라이스의 gameplay equipment slot은 다음 여섯 개다.

- `HEAD`
- `SHOULDER`
- `UPPER`
- `LOWER`
- `HANDS`
- `WEAPON`

`dress`와 전신 outfit은 물리적으로 upper/lower를 억지 분할하지 않는다. item의 primary slot과
`coverageSlots[]`를 함께 저장해 다른 slot과의 충돌을 Server와 Client publisher가 같은 방식으로
검증한다. hair와 face는 appearance preset이고 초기 raid reward slot이 아니다.

### 4.2 `Data/Items/ItemCatalog.json`

현재 formatVersion 2의 `category`만으로는 장비를 판정할 수 없다. 다음 revision은 기존 potion/material을
보존하면서 장비 행에 다음 semantic field를 추가한다.

- `itemKind`: `CONSUMABLE`, `MATERIAL`, `CURRENCY`, `EQUIPMENT`
- `equipmentSlot`: 위 stable enum ID
- `coverageSlots`: 복합 garment가 점유하는 추가 slot
- `compatibleClasses`: 여섯 `CHARACTER_CLASS_ID`의 명시 목록
- `rarity`: UI 표시용 stable enum
- `maxStack`: equipment는 1이지만 동일 itemId의 서로 다른 instance 보유는 허용

Server bootstrap에는 itemId, stack, itemKind, slot, coverage, compatibleClasses만 publish한다.
display name, icon, model path, material path는 Server에 보내지 않는다.

### 4.3 `Data/Actors/EquipmentPresentationCatalog.json`

새 Client presentation 정본은 `itemId + classId`를 `visualSetId + parts[]`에 매핑한다.

각 visual set은 다음을 소유한다.

- `visualSetId`: `character.<class>.<curated-set-id>`
- `itemId`, `classId`, `primarySlot`, `coverageSlots`
- `parts[].partId`: set 내부 stable ID
- `parts[].modelAssetId`: `Character/.../*.wmodel` Resources-relative ID
- `parts[].partRole`: skinned garment, main weapon, shield, stance weapon 등
- `parts[].attachmentMode`: `SKINNED` 또는 `SOCKET`
- `parts[].socketBoneId`, `socketYawDegrees`: socket mode에서만 필수
- `parts[].visibleStances`: 창술사 L/S, 건슬링어 H/L/S 등
- `parts[].bodyCoverageMeshIds`: body mesh 이름 기반 hide 계약
- `parts[].materialProfileId`

body coverage는 vector index나 raw bit mask를 저장하지 않는다. 로드 시 stable body mesh ID를 현재
WModel mesh index로 resolve해 runtime mask를 만든다. 모르는 이름, 중복, 32-bit mask overflow는
catalog load를 실패시키고 기존 catalog를 유지한다.

### 4.4 `Data/Valtan/Valtan.clearrewards.json`

현재 formatVersion 1은 여섯 itemId를 순서대로 전부 지급한다. formatVersion 2는 class별 pool과
정수 weight, roll count를 소유한다.

- class마다 정확히 하나의 active pool
- `rollCount > 0`
- entry의 `weight > 0`
- entry item은 ItemCatalog의 equipment이며 해당 class와 호환
- 같은 pool 안의 duplicate itemId 거부
- weight 합의 `uint64_t` overflow 거부

RNG 결과는 Server room이 소유하며 Client가 seed, weight, 선택 index를 보내지 않는다.

## 5. Shared와 Server 권위 상태

### 5.1 item instance와 snapshot

equipment는 `itemId + quantity` stack만으로 표현하지 않는다. Server가 process 동안 유일한
`uint64 itemInstanceId`를 발급하고 0을 invalid로 둔다. consumable stack도 하나의 instance ID를
가지며 merge 뒤 기존 ID를 유지한다.

Shared packet 계약은 기존 `PacketType.h`, `PacketMessages.h/.cpp`의 inventory 경로를 확장한다.

- `C2S_EQUIP_ITEM`: request sequence, expected state revision, itemInstanceId
- `C2S_UNEQUIP_SLOT`: request sequence, expected revision, equipmentSlot
- `C2S_CLAIM_RAID_REWARD`: request sequence, reward transaction ID
- `S2C_INVENTORY_SNAPSHOT`: revision, item instances, active-class loadout
- `S2C_EQUIPMENT_PRESENTATION_SNAPSHOT`: netEntityId, class, revision, slot별 itemId/instanceId
- `S2C_ITEM_COMMAND_RESULT`: request sequence, typed result, committed revision
- `S2C_RAID_REWARD_RESULT`: immutable roll, auto-claim/pending-full 상태

Owner에게만 전체 inventory를 보내고, room broadcast에는 현재 장착 itemId와 slot만 보낸다.
model asset ID, Prototype tag, pointer, vector index는 wire에 넣지 않는다.

### 5.2 Server player-owned state

`CServerPlayer`가 다음 process-session state를 소유한다.

- inventory item instances
- 여섯 class별 `slot -> itemInstanceId` loadout
- pending raid reward transaction
- item state revision
- 마지막 처리 request sequence

장착한 item도 inventory 소유 목록에 남고 loadout이 instance를 참조한다. 따라서 장착/해제 자체는
가방 capacity를 바꾸지 않으며, 해제 시 빈 칸 부족으로 실패하는 모순이 없다.

class change는 기존 장비를 인벤토리로 밀어 넣지 않는다. 목표 class의 per-class loadout을 활성화하고
그 snapshot을 broadcast한다. source class loadout도 그대로 보존한다.

기존 transfer의 빈 `CarriedInventory`가 “전달된 빈 인벤토리”와 “전달 없음, 기본 potion 지급”을
구분하지 못하는 문제는 `optional<PLAYER_OWNED_ITEM_STATE>`로 바꾼다. inventory, loadouts, pending reward,
revision을 한 state로 stage하고 target room enter 성공 뒤에만 source를 commit한다.

### 5.3 raid reward transaction

Valtan death가 처음 commit되는 tick에 room-owned RNG가 플레이어별 pool을 한 번 roll한다.

1. clear epoch와 player를 기준으로 at-most-once latch를 확인한다.
2. weighted result를 immutable pending transaction으로 stage한다.
3. inventory capacity와 instance ID 발급을 검증한다.
4. 가능하면 inventory에 원자적으로 auto-claim한다.
5. full이면 reroll하지 않고 pending을 유지하고 typed result를 보낸다.
6. 사용자가 공간을 정리하고 claim하면 같은 결과만 commit한다.

`MAX_INVENTORY_ITEMS`는 현재 UI의 stable `Inventory_Slot_0..99`와 맞춰 100으로 통일한다. capacity,
duplicate request, stale revision, incompatible class, wrong slot, missing instance, already-equipped conflict는
모두 typed rejection이며 기존 inventory/loadout/reward latch를 유지한다.

## 6. Client 장착과 해제

### 6.1 command 경계

`CInventoryView`는 socket을 직접 호출하지 않는다. 새 `CInventoryCommandService`에 stable
itemInstanceId 또는 equipmentSlot을 제출한다. service가 request sequence와 expected revision을 붙여
`CNetworkManager`에 전달한다. UI는 Server snapshot/result만 view model에 반영한다.

screen-space hit test는 앞쪽 draw order 하나만 pointer를 소비하고, 소비한 frame에는 gameplay mouse
command를 막는다. drag/drop과 double-click 모두 같은 typed equip command로 수렴한다.

### 6.2 atomic presentation replacement

`CCharacter`는 `CEquipmentPresentationService`를 통해 다음 순서로 장착 snapshot을 적용한다.

1. `itemId + classId`로 presentation entry를 찾는다.
2. 필요한 모든 WModel prototype, material, texture, socket, stance part를 stage한다.
3. skinned part의 weighted palette signature가 현재 body와 호환되는지 검사한다. 전체
   `Get_SkeletonHash()` 단순 동등 비교는 기존 valid part의 unweighted synthetic node까지 달라
   오탐하므로 사용하지 않는다.
4. socket part의 bone 존재와 stance member completeness를 검사한다.
5. stable body mesh coverage를 runtime hide mask로 resolve한다.
6. 모든 part clone이 성공한 뒤 `CContainerObject`의 slot group을 한 번에 교체한다.
7. 새 body hide state와 generation을 같은 commit에서 적용한다.
8. 실패하면 staged clone만 폐기하고 이전 part/body hide state를 유지한다.

`CContainerObject`에는 generic protected stage/commit API를 추가한다. raw map을 Client가 직접 erase하거나
update loop 도중 부분 교체하지 않는다. 이 변경은 Engine public header 변경이므로 `UpdateLib.bat` 뒤
Client까지 검증한다.

logical Server loadout과 visual presentation은 분리한다. runtime resource 실패는 Server state를 되돌리거나
disconnect시키지 않는다. 해당 Character는 이전 visual generation을 유지하고 typed diagnostic을 남긴 뒤,
resource가 복구되면 같은 revision을 재적용할 수 있어야 한다.

### 6.3 spawn, remote, late join

- local player도 broadcast와 같은 equipment presentation snapshot을 소비한다.
- remote player는 packet의 itemId를 local presentation catalog로 resolve한다.
- late join initial bundle에는 현재 room player들의 equipment snapshot이 포함된다.
- class-change presentation transaction은 새 body를 stage한 뒤 목표 class loadout을 같은 generation에 적용한다.
- disconnect는 replicated equipment state와 staged generation을 정리한다.

## 7. G별 구현 순서

### G00. source inventory와 normalization admission

목표는 raw 추출물을 교체 가능한 WModel 후보로 바꾸되 안전하지 않은 파일을 Resources에 넣지 않는 것이다.

현재 추가된 파일:

- `Tools/CharacterEquipmentPipeline/New-CharacterEquipmentSourceInventory.ps1`

추가/변경할 파일:

- `Tools/CharacterEquipmentPipeline/Normalize-CharacterEquipmentGltf.ps1`
- `Tools/CharacterEquipmentPipeline/normalize_character_equipment_gltf.py`
- `Tools/CharacterEquipmentPipeline/test_normalize_character_equipment_gltf.py`
- `Tools/CharacterEquipmentPipeline/Publish-CharacterEquipment.ps1`
- `Tools/ModelAssetConverter/README.md`

normalizer는 glTF joint index를 이름으로 class master에 remap하고, 누락된 master bone을 bind pose로 확장하며,
JOINTS_0, inverse bind, vertex/bone/node scale을 일관되게 갱신한다. `--pretransform`은 사용하지 않는다.
output WModel은 `ModelAssetConverter` 기존 포맷을 사용한다.

admission을 통과한 파일만 다음 물리 위치에 배치한다.

```text
Client/Bin/Resources/Character/<Class>/Equipment/<VisualSetId>/<PartId>.wmodel
Client/Bin/Resources/Character/<Class>/Equipment/<VisualSetId>/Textures/...
```

Resources payload는 Git에 force-add하지 않는다. RESULT에는 필요한 상대 asset ID, 물리 폴더, Drive 전달
준비 여부를 기록한다.

G00 종료 증거:

- source inventory 7,475행과 class 분모 일치
- blocked 11개가 성공으로 위장되지 않음
- direct-cook mismatch fixture가 validator에서 실패
- normalized fixture가 body weighted-palette index/name mapping 및 1-mesh contract 통과
- missing joint, duplicate joint, inverse-bind mismatch, locale 중복, texture 누락 rollback 통과
- Resources top-level 폴더가 정확히 일곱 개 유지

### G00A. 기존 Cook 자산 기반 대표 admission pack

raw 전체 normalizer를 만들기 전에 현재 제품 경로에서 이미 Cook되고 body palette/socket 계약을
통과하는 소수 자산을 exact source object의 authored association evidence와 함께 묶는다. raw와 WModel은
각각 독립 검증하지만 과거 Cook receipt나 geometry derivation이 없어 exact build provenance를 주장하지
않는다. 이 단계는 새 raw glTF를 직접 Cook한 것으로 위장하지 않고, 다음 파일이 선택과 staging
transaction을 소유한다.

- `Tools/CharacterEquipmentPipeline/RepresentativeCharacterEquipment.json`
- `Tools/CharacterEquipmentPipeline/New-CharacterEquipmentRepresentativePack.ps1`
- `Tools/CharacterEquipmentPipeline/new_character_equipment_representative_pack.py`
- `Tools/CharacterEquipmentPipeline/test_new_character_equipment_representative_pack.py`

출력은 `out/CharacterEquipmentExtraction/Admission/representative-20260901-curated` 아래에만 만든다. exact glTF와
buffer는 `Source`, 선택한 WModel과 실제 material texture closure는 계획된 Resources-relative 구조를
보존한 `Runtime`에 stage한다. `character-equipment-runtime-admission.json`은 source family와 exact object,
별도 `curatedVariantId`, stable `visualSetId`, named part, weighted palette/socket/material 검증과 target asset ID를
기록한다. `sourceAssociation.state`는 `CURATED_NOT_BUILD_PROVENANCE`이며 receipt hash는 선택된 파일과
staging 검증 결과의 증거이지 raw에서 WModel로 변환된 역사 자체의 증거가 아니다. 별도 immutable
Resource pack/lock 계약도 아니다.

첫 대표 분모는 6 class, 12 visual set, 17 part다. 창술사 모코코 head/outfit, 건슬링어 head/upper variant,
슬레이어·도화가 head, 워로드 baseline shoulder, 창술사·슬레이어·도화가·워로드·차원술사 weapon assembly를
포함한다. 건슬링어
H/L/S 전체는 S의 canonical player path와 mode/socket swap이 닫히지 않아 첫 validated weapon set에서
제외한다. 차원술사 apparel은 combined body split 전에는 admission하지 않는다.

산출물 카테고리는 `READY_ALTERNATIVE`, `BASELINE_PART`, `BASELINE_WEAPON`으로 구분한다. 별도로 여섯 class의
shoulder raw 후보를 하나씩 `RAW_EXTRACTED_NOT_EQUIP_READY`로 pack의 Source closure에 넣고
`MASTER_SKELETON_NORMALIZATION_REQUIRED`를 기록한다. 차원술사 후보에는
`COMBINED_BODY_COVERAGE_REQUIRED`도 추가한다.

G00A 종료 증거:

- exact source inventory row와 선택 object가 유일하게 resolve됨
- skinned 7 part의 positive-weight palette index/name 및 inverse-bind delta 0
- socketed 10 part의 body socket과 no-skeleton/no-animation 계약 통과
- WModel 17개와 실제 texture closure 73개, pending raw source 6개를 self-contained staging에 복사
- path escape, duplicate stable ID/target, socket 누락, weighted mismatch fixture 거부
- `Client/Bin/Resources`와 `Data/Actors/CharacterCatalog.json` 미변경

### G01. item, presentation, reward 데이터 계약

변경할 파일:

- `Data/Items/ItemCatalog.json`
- `Data/Actors/EquipmentPresentationCatalog.json` 신규
- `Data/Valtan/Valtan.clearrewards.json`
- `Tools/GameplayPipeline/Publish-ItemCatalog.ps1`
- `Tools/ValtanPipeline/Publish-ValtanClearRewards.ps1`
- `Tools/CharacterEquipmentPipeline/Publish-CharacterEquipment.ps1`
- `Client/Default/Client.vcxproj`, `Client/Default/Client.vcxproj.filters`

신규 JSON은 Client 프로젝트의 `96.DataFiles` 아래 `None` 항목으로만 등록한다. build output `Content`나
프로젝트별 복사본을 만들지 않는다.

G01 종료 증거:

- 정상 여섯 class catalog publish
- invalid version, unknown item/class/slot, duplicate stable ID, incompatible class, missing part 거부
- multi-part weapon 필수 역할 누락 거부
- dress coverage collision 거부
- bad Resources path, material/texture closure 실패 시 runtime 문서 미교체
- reward zero weight, overflow, duplicate, non-equipment entry 거부

### G02. Shared item/equipment protocol과 Server state

변경할 파일:

- `Shared/Public/Network/PacketType.h`
- `Shared/Public/Network/PacketMessages.h`
- `Shared/Private/Network/PacketMessages.cpp`
- `Server/Public/ServerPlayer.h`
- `Server/Public/RoomCommand.h`
- `Server/Public/ServerTriggerSystem.h`
- `Server/Public/GameRoom.h`, `Server/Private/GameRoom.cpp`
- `Server/Public/ServerApp.h`, `Server/Private/ServerApp.cpp`
- `Server/Public/ItemCatalog.h`, `Server/Private/ItemCatalog.cpp`
- `Server/Public/ValtanClearRewards.h`, `Server/Private/ValtanClearRewards.cpp`
- `Server/Public/RaidRewardService.h`, `Server/Private/RaidRewardService.cpp` 신규
- `Shared/Default/Shared.vcxproj`, `Shared/Default/Shared.vcxproj.filters`
- `Server/Default/Server.vcxproj`, `Server/Default/Server.vcxproj.filters`

`RaidRewardService`는 `CGameRoom`이 실제로 소유하고 clear tick에서 호출한다. 미래용 placeholder manager를
만들지 않는다. 새 H/CPP는 project와 filter에 함께 등록한다.

G02 종료 증거:

- protocol encode/decode round trip
- invalid enum, zero instance ID, oversized vector/string, duplicate slot 거부
- 동일 seed fixture의 deterministic weighted result
- duplicate clear와 duplicate claim이 instance를 중복 생성하지 않음
- inventory full pending은 reroll되지 않음
- equip/unequip stale revision과 wrong class/slot rollback
- empty inventory transfer와 “carried 없음”이 구분됨
- Party2/Party4 transfer에서 inventory/loadout/pending state 보존

### G03. Client catalog와 atomic equipment presentation

추가할 파일:

- `Client/Public/EquipmentPresentationCatalog.h`
- `Client/Private/EquipmentPresentationCatalog.cpp`
- `Client/Public/EquipmentPresentationService.h`
- `Client/Private/EquipmentPresentationService.cpp`

변경할 파일:

- `Engine/Public/ContainerObject.h`, `Engine/Private/ContainerObject.cpp`
- `Client/Public/Part_Equipment.h`, `Client/Private/Part_Equipment.cpp`
- `Client/Public/Part_Body.h`, `Client/Private/Part_Body.cpp`
- `Client/Public/PlayableCharacterAssetService.h`, `Client/Private/PlayableCharacterAssetService.cpp`
- `Client/Public/Character.h`, `Client/Private/Character.cpp`
- `Client/Public/ClientReplication.h`, `Client/Private/ClientReplication.cpp`
- `Client/Public/ClientReplicationEvent.h`
- `Client/Default/Client.vcxproj`, `Client/Default/Client.vcxproj.filters`

G03 종료 증거:

- 같은 slot 새 set의 all-part commit
- 중간 part clone 실패 시 이전 generation과 body coverage 유지
- weighted palette/socket/stance/texture 실패 격리
- 차원술사 base split 뒤 garment 장착/해제 rollback
- 창술사 L/S, 건슬링어 H/L/S, 워로드 MAIN/SD, 차원술사 E/L/P/S completeness
- local/remote/late-join snapshot parity

### G04. Inventory UI와 typed command service

추가할 파일:

- `Client/Public/InventoryCommandService.h`
- `Client/Private/InventoryCommandService.cpp`

변경할 파일:

- `Client/Public/InventoryView.h`, `Client/Private/InventoryView.cpp`
- `Client/Public/ItemCatalog.h`, `Client/Private/ItemCatalog.cpp`
- `Client/Public/CombatHUDViewModel.h`
- `Client/Public/MainApp.h`, `Client/Private/MainApp.cpp`
- `Client/Public/NetworkManager.h`, `Client/Private/NetworkManager.cpp`
- `Data/UI/Inventory/InventoryUI.json`
- `Client/Default/Client.vcxproj`, `Client/Default/Client.vcxproj.filters`

`InventoryUI.json`에는 stable `Equipment_Slot_HEAD` 등 여섯 slot을 추가한다. 장착 중인 item은 bag에서도
계속 보이되 equipped marker를 표시한다. 다른 class item, covered slot conflict, pending Server result를
명확한 상태 메시지로 표시한다.

G04 종료 증거:

- screen-space frontmost hit 하나만 소비
- drag/drop과 double-click이 같은 command 생성
- consumed frame gameplay mouse command 차단
- Server reject 시 optimistic local swap 없음
- snapshot revision 역행 거부
- 장착/해제 뒤 Inventory와 Character view가 같은 instance ID를 표시

### G05. 인증된 account/character의 durable 저장

현재 제품에는 login/auth packet, account repository, DB adapter가 없다. Server는 접속마다 증가하는
`sessionId`를 발급하고 첫 `C2S_ENTER_WORLD`의 class/nickname만 사용하며, nickname은 display text라
회원 identity로 쓸 수 없다. 기존 `CServerPlayer::Inventory`도 room process memory에만 있어 퇴장이나
Server 재시작 뒤 복원되지 않는다. 따라서 session ID, nickname, socket peer를 저장 key로 승격하지 않는다.

추가할 최소 저장 경계:

- `Server/Public/ICharacterStateRepository.h`
- `Server/Public/CharacterPersistentState.h`
- `Server/Public/SqliteCharacterStateRepository.h`
- `Server/Private/SqliteCharacterStateRepository.cpp`
- repository schema migration과 restart/CAS harness
- `Server/Default/Server.vcxproj`, `Server/Default/Server.vcxproj.filters`

저장 root는 Git 저장소, `Data`, `Client/Bin/Resources`, build output 밖의 Server machine state로 둔다.
첫 구현의 기본값은 `C:\ProgramData\LostArkTeam\Server\CharacterState.sqlite3`이고 Server launch option으로
명시적 absolute path를 교체할 수 있게 한다. directory 생성, DB open, schema migration이 실패하면
process-session JSON이나 nickname fallback으로 우회하지 않고 durable equipment 기능을 fail-closed한다.
계정 password나 login token은 이 장비 DB와 JSON에 저장하지 않는다.

저장 aggregate의 key와 값은 다음과 같다.

```text
accountId + characterId
  stateRevision
  selectedClassId
  inventory[]: itemInstanceId, itemId, quantity, acquisitionTransactionId
  classLoadouts[classId][equipmentSlot]: itemInstanceId
  pendingRaidRewards[]: transactionId, raidId, clearEpoch, rolledItemId, claimState
```

model path, `visualSetId`, source object, Prototype/Part tag, pointer, vector index는 저장하지 않는다.
재접속 시 저장된 `itemId + classId`를 현재 `ItemCatalog`와 `EquipmentPresentationCatalog`로 다시 resolve한다.
삭제되거나 비호환인 item 정의를 만나면 해당 loadout을 임의 대체하지 않고 typed quarantine 결과와 함께
기존 durable row를 보존한다.

repository API는 `LoadCharacter(accountId, characterId)`와
`CompareAndSwapCharacter(expectedRevision, candidateState)` 두 transaction을 핵심으로 한다. 인증된 session이
stable account/character ID에 bind된 뒤에만 room admission을 시작한다. raid reward와 equip/unequip은 먼저
candidate aggregate를 검증하고 DB CAS commit에 성공한 뒤 room state와 snapshot을 commit한다. DB busy,
disk full, stale revision, process crash는 이전 revision을 유지하며 성공 packet을 보내지 않는다.

현재 저장소에 인증 provider가 없으므로 nickname이나 자동 생성 ID로 회원을 가장하지 않는다. 제품 연결 전
별도의 인증 admission이 stable `accountId`와 소유 `characterId`를 Server session에 제공해야 한다. provider가
정해지기 전 repository harness는 명시적 fixture ID만 사용하고 제품 durable 경로는 비활성 상태를 유지한다.

G05 종료 증거:

- Server restart와 재접속 뒤 inventory, 여섯 class loadout, pending reward가 같은 revision으로 복원
- duplicate clear/claim 재전송이 같은 transaction을 재사용하고 item instance를 중복 생성하지 않음
- concurrent stale CAS 하나만 성공하고 loser는 최신 snapshot을 다시 받음
- DB open/migration/corrupt row/disk failure에서 room state와 이전 DB revision 불변
- 다른 account/character의 itemInstanceId 장착 요청 거부
- logout/world transfer 도중 실패해도 durable commit과 room commit이 엇갈리지 않음

### G06. publisher, harness, build, 문서 종료

변경할 파일:

- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`
- `Tools/CharacterSelectIsolationHarness/Private/CharacterSelectIsolationHarness.cpp`
- `Tools/Build/BuildDomains.json`
- 대응 PLAN/RESULT와 public 계약이 실제로 바뀐 경우에만 팀 문서

검증 순서:

```powershell
pwsh -NoProfile -File Tools/CharacterEquipmentPipeline/New-CharacterEquipmentSourceInventory.ps1
powershell -ExecutionPolicy Bypass -File Tools/CharacterEquipmentPipeline/Publish-CharacterEquipment.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-ItemCatalog.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanClearRewards.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic
git diff --check
```

Engine public header를 바꾼 G03에서는 `UpdateLib.bat` 뒤 Product Client까지 다시 빌드한다.

## 8. 자동 검증 행렬

| 영역 | 정상 사례 | 반드시 실패할 사례 | rollback 증거 |
|---|---|---|---|
| source inventory | 7,475 exact denominator | wrong class prefix, Raw/RawShared duplicate ID | 이전 inventory output 유지 |
| normalizer | weighted palette index/name 일치 | unknown/duplicate joint, bad inverse bind | Resources 미승격 |
| presentation catalog | 6-class/multi-part set | missing model/socket/stance, slot collision | 이전 catalog 유지 |
| item publisher | equipment semantic publish | bad slot/class/version/duplicate | bootstrap 미교체 |
| reward publisher | positive weighted class pool | zero/overflow/unknown item | reward bootstrap 미교체 |
| reward runtime | exactly-once auto claim | duplicate clear/claim, full inventory | pending result 고정, no reroll |
| equip runtime | owned compatible item | missing instance, wrong class/slot, stale revision | inventory/loadout revision 불변 |
| world transfer | empty/full state 보존 | target admission/send failure | source room/state 유지 |
| durable repository | restart/reconnect state 복원 | stale CAS, corrupt row, DB write failure | 이전 DB/room revision 유지 |
| Client presentation | all parts atomic commit | middle clone/skeleton/socket failure | 이전 visual generation 유지 |
| replication | local/remote/late join 동일 | reordered/stale snapshot | 최신 generation 유지 |

## 9. 수동 화면 검증 경계

Client와 UI는 사용자가 직접 실행하고 시각 결과를 판정한다. 에이전트는 Client를 자율 실행하거나
스크린샷을 만들고 visual PASS를 기록하지 않는다.

자동 검증과 Server 준비가 끝난 뒤 사용자에게 다음 경로를 안내한다.

1. `Server + Client` launch profile을 `Ctrl+F5`로 시작한다.
2. Lobby에서 승인된 class로 Valtan Arena에 진입한다.
3. 발탄 클리어 후 reward 결과가 class pool에서 하나만 확정되는지 확인한다.
4. Inventory의 장비 item을 각 equipment slot에 장착한다.
5. 걷기, 스킬, stance 전환에서 body 관통, 누락, 무기 visibility를 확인한다.
6. 해제 시 기본 appearance가 복원되는지 확인한다.
7. class를 바꿨다가 돌아와 per-class loadout이 복원되는지 확인한다.
8. Server를 정상 종료·재시작하고 같은 회원/캐릭터로 재접속해 inventory와 loadout이 복원되는지 확인한다.
9. 2인 이상에서 상대 장비 교체와 late join 외형이 같은지 확인한다.

최종 visual fidelity와 `manual first pixel`, occurrence 승인은 사용자의 서면 관찰이 있어야만 RESULT에
PASS로 기록한다.
