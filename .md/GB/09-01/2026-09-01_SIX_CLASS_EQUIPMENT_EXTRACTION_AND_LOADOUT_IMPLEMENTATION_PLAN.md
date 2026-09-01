# 6개 클래스 장비 리소스 정규화·레이드 보상·인벤토리 장착·계정 저장 구현 계획서

작성일: 2026-09-01

문서 종류: 구현 계획서

대상 클래스: 차원술사, 도화가, 창술사, 워로드, 슬레이어, 건슬링어

현재 구현 상태: 원본 추출과 source inventory, 기존 Cook 자산 기반 6-class 대표 admission pack,
검증된 runtime closure의 실제 Resources 승격, F1 Equipment Authoring Tool의 preview-only 슬롯 교체까지
완료. 복합 슬롯 의상과 기본 외형의 정합화는 아래 G03A/G03B 계획 확정 뒤 미구현이며,
Server item/reward/inventory와 account durable 저장도 미착수

대응 결과서: `2026-09-01_SIX_CLASS_EQUIPMENT_EXTRACTION_AND_LOADOUT_RESULT.md`

이번 PR은 최종 Server 권위 흐름 전체가 아니라 그 앞단의 검증 가능한 저작·표현 슬라이스를 닫는다.
1차 대표 12 visual set/17 named part에 가시 외형 보강분을 더한 현재 구현 상태는 22 visual set/36 named part다.
G03A는 같은 36 physical part를 유지한 채 분리된 모코코 두 set을 한 owner로 합쳐 21 visual set으로 만들고,
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

### 2026-09-01 복합 슬롯 의상과 독립 무기 Tool 정합화 계획

물리 모델의 개수와 논리 장비 슬롯 수를 같은 것으로 취급하지 않는다. 현재 모코코는 `head` WModel과
`dress` WModel이 각각 별도 visual set으로 갈라져 있지만, 이번 제품 정책에서는 두 물리 part를 하나의
`APPAREL_FULLBODY` owner로 묶는다. owner의 primary는 `UPPER`, footprint는
`HEAD/SHOULDER/UPPER/LOWER/HANDS`이고 `WEAPON`은 포함하지 않는다. 따라서 Tool에서 모코코를 한 번 장착한
뒤에도 무기 owner만 독립적으로 교체·해제할 수 있어야 한다.

현재 Tool의 primary-slot array와 `occupiedSlots` mask는 catalog에 올라온 set끼리의 충돌은 처리한다.
그러나 창술사 모코코가 catalog와 별개로 `Logic_LanceMaster.cpp`의 `AVATAR_HEAD/AVATAR_ARMOR` 기본 파츠에도
항상 생성된다. `AVATAR_ARMOR`의 존재가 모든 기본 방어구를 숨기고 presentation slot은 `UPPER` 하나뿐이라
다음 문제가 남는다.

사용자 스크린샷에서 선택한 `character.warlord.default_00.shoulder`는 `BASELINE_PART`이며 현재 워로드 기본
shoulder와 같은 reference 모델이다. 따라서 그 행을 Equip해도 외형 차이가 없는 것이 현재 데이터상 정상이고,
행만 선택한 뒤 Save하면 loadout 자체가 바뀌지 않아 저장 결과도 달라지지 않는다.

| 현재 동작 | 문제 |
|---|---|
| 모코코 catalog set 장착 | baked-in 동일 모델을 숨기고 다시 clone하므로 외형 변화가 없다. |
| `LOWER` 단독 장착 | baked-in armor는 `UPPER`로만 표시돼 남아 있고 새 하의와 겹친다. |
| `UPPER` 단독 장착 | baked-in armor 전체는 사라지지만 기본 `LOWER/HANDS/SHOULDER`가 계속 숨을 수 있다. |
| `Unequip All` 또는 Reset | 표준 기본 방어구가 아니라 baked-in 모코코로 돌아간다. |
| 현재 모코코 선택 | 머리와 몸통이 서로 다른 owner라 사용자가 두 번 장착해야 하며 “전신 의상 한 벌”이라는 보상 의미와 다르다. |
| slot 화면 | 보조 점유 slot은 owner를 찾지만 후보 목록은 primary slot 기준이라 전신 set의 영향 범위를 바로 비교하기 어렵다. |
| `BASELINE_PART/BASELINE_WEAPON` 선택 | admission 참고 자산도 일반 후보처럼 보여 현재 기본과 같은 모델을 다시 골라 “눌러도 안 바뀜”으로 보일 수 있다. |
| 후보 행 선택 뒤 Save | 행 선택은 candidate/diagnostic만 바꾸고 loadout에는 아직 장착하지 않으므로 Save가 그 candidate를 저장하지 않는다. UI가 이 경계를 설명하지 않는다. |
| preset 저장 | 현재는 primary 위치만 저장하는 방향은 맞지만 Tool과 presentation service가 slot 해석을 각각 구현한다. |

G03A/G03B는 다음 불변식을 먼저 고정한 뒤 구현한다.

1. `occupiedSlots`는 primary를 포함하는 중복 없는 전체 footprint이며 물리 `parts[]` 개수와 독립이다.
2. loadout은 set 또는 item instance를 primary slot에 정확히 한 번만 소유한다. secondary slot은 저장 row가
   아니라 resolver가 만든 `ownerPrimarySlot` 파생값이다.
3. 새 set footprint와 하나라도 겹치는 기존 owner는 set 전체가 교체 대상이다. 겹치지 않는 owner는 그대로
   남는다. 따라서 4/5-slot outfit 교체는 armor owner만 제거하고 `WEAPON` owner는 보존한다.
4. secondary slot에서 해제해도 resolver가 primary owner를 찾아 set 전체를 제거한다.
5. apparel category는 `WEAPON`을 점유할 수 없고 `WEAPON_SET`은 `WEAPON`만 점유한다.
   `APPAREL_FULLBODY`는 primary `UPPER`와 다섯 apparel slot 전체를 요구한다. 전신 의상과 무기 독립성을
   catalog validation에서 보장한다.
6. 현재의 standalone 모코코 head set은 제거하고 기존 outfit stable ID가 `head + dress` 두 part를 소유하게
   한다. catalog 장비를 `CHARACTER_SPEC` 기본 파츠에 중복 하드코딩하지 않는다. 빈 loadout과 `Unequip All`은
   class 표준 기본 외형이며 모코코는 Tool/loadout을 통해서만 장착한다.
7. body hide는 고정 class bit mask만으로 결정하지 않는다. 현재 보이는 기본 파츠와 새 visual set의 stable
   body material coverage를 합성한 뒤 part group과 같은 transaction에서 commit한다.
8. Tool은 장착 전에 `점유할 슬롯`, `전체 해제될 owner`, `유지될 owner`를 표시한다. 실제 preview 적용이
   성공한 뒤에만 draft와 dirty 상태를 commit한다.
9. Save/Reload는 primary selection만 기록하고 parse -> validate -> stage preview -> commit 순서를 유지한다.
10. 제품 Server 저장은 `{classId, primarySlot, itemInstanceId}`만 보존한다. slot 의미의 정본은
    `ItemCatalog`이고 Server는 여기서 footprint를 다시 resolve한다. Client 전용
    `EquipmentPresentationCatalog`가 같은 `itemId/classId`의 footprint를 반복하면 publisher가 ItemCatalog와
    exact-equality를 강제하며 Server는 model·visual catalog를 읽지 않는다.

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

`dress`와 전신 outfit은 물리적으로 upper/lower를 억지 분할하지 않는다. item 정의는 `primarySlot`과
primary를 포함하는 전체 `occupiedSlots[]`를 소유하고, 장착 상태는 primary slot에 item instance를 한 번만
저장한다. secondary slot owner는 매 validate/load 때 footprint로 재계산해 다른 slot과의 충돌을 Server와
Client가 같은 방식으로 검증한다. hair와 face는 appearance preset이고 초기 raid reward slot이 아니다.

### 4.2 `Data/Items/ItemCatalog.json`

현재 formatVersion 2의 `category`만으로는 장비를 판정할 수 없다. 다음 revision은 기존 potion/material을
보존하면서 장비 행에 다음 semantic field를 추가한다.

- `itemKind`: `CONSUMABLE`, `MATERIAL`, `CURRENCY`, `EQUIPMENT`
- `equipmentCategory`: equipment에만 필요한 `APPAREL_HEAD`, `APPAREL_SHOULDER`, `APPAREL_UPPER`,
  `APPAREL_LOWER`, `APPAREL_HANDS`, `APPAREL_OUTFIT`, `APPAREL_FULLBODY`, `WEAPON_SET`
- `primarySlot`: 저장 owner가 놓이는 위 stable enum ID
- `occupiedSlots`: primary를 포함해 복합 garment가 점유하는 전체 slot의 중복 없는 목록
- `compatibleClasses`: 여섯 `CHARACTER_CLASS_ID`의 명시 목록
- `rarity`: UI 표시용 stable enum
- `maxStack`: equipment는 1이지만 동일 itemId의 서로 다른 instance 보유는 허용

Server bootstrap에는 itemId, stack, itemKind, equipment category, primary slot, occupied slots,
compatibleClasses와 publisher가 확정한 `equipmentSemanticRevision`만 publish한다. display name, icon, model path,
material path는 Server에 보내지 않는다. category/primary/occupied/class compatibility가 바뀌면 semantic revision도
바뀌어 기존 durable row를 새 footprint로 조용히 재해석하지 못하게 한다.

### 4.3 `Data/Actors/EquipmentPresentationCatalog.json`

새 Client presentation 정본은 `itemId + classId`를 `visualSetId + parts[]`에 매핑한다.

각 visual set은 다음을 소유한다.

- `visualSetId`: `character.<class>.<curated-set-id>`
- `itemId`, `classId`, `categoryId`, `primarySlot`, `occupiedSlots`: `categoryId`는 Client 표기 이름이지만
  product row에서는 ItemCatalog의 `equipmentCategory`와 정확히 같아야 한다.
- `parts[].partId`: set 내부 stable ID
- `parts[].modelAssetId`: `Character/<Class>/Equipment/<AdmittedVariantDir>/<Part>.wmodel` 형식의
  Resources-relative ID. 하나의 logical owner가 기존 162-file closure의 서로 다른 admitted variant directory에
  있는 part를 함께 참조할 수 있으며 visualSetId와 물리 directory 이름은 동일 ID가 아니다.
- `parts[].partRole`: skinned garment, main weapon, shield, stance weapon 등
- `parts[].attachmentMode`: `SKINNED` 또는 현재 runtime enum과 같은 `SOCKETED`
- `parts[].socketBoneId`, `socketYawDegrees`: socket mode에서만 필수
- `parts[].requiredStance`: 한 part가 표시될 stance 하나 또는 모든 stance를 뜻하는 `null`
- `bodyCoverage`: `INHERIT_BASELINE`, `EXPLICIT materialIds[]`, `NONE` 중 하나인 set-level body hide 계약

formatVersion 2에는 현재 loader/service가 소비하지 않는 `visibleStances`와 `materialProfileId`를 추가하지 않는다.
여러 stance에 각각 다른 메시가 필요한 무기 세트는 지금처럼 같은 set의 stance별 part를 여러 개 둔다.

formatVersion 2 parser는 visual set row의 `itemId`를 `null|string`로만 받는다. visual row에는 별도
`authoringOnly` bool을 만들지 않고 `itemId: null` 자체가 Tool preview-only 의미이며 메모리에서는 빈 문자열로
정규화한다. product admission row는 비어 있지 않은 string을 요구하고 duplicate `(itemId, classId)`를 거부한다.
`visualSetId`도 catalog 전체에서 유일해야 한다. `bodyCoverage`는 mode별 exact-field object다. `INHERIT_BASELINE`과 `NONE`은 `mode`만,
`EXPLICIT`은 `mode`와 중복 없는 비어 있지 않은 `materialIds`만 가져야 하며 unknown field를 거부한다. G03A에서
재생성하는 나머지 20개 preview set도 `itemId: null`을 명시한다. apparel set은 admission evidence에 맞춘
`bodyCoverage: { "mode": "INHERIT_BASELINE" }`, `WEAPON_SET`은 반드시
`bodyCoverage: { "mode": "NONE" }`로 publish해 일부 row만 구형 의미로 남지 않게 한다.

제품 publish에서는 같은 `itemId + classId`의 `equipmentCategory`, `primarySlot`, `occupiedSlots`가
`ItemCatalog`와 정확히 같아야 한다. `EquipmentPresentationCatalog`는 visual mapping을 위한 Client 데이터이며
slot 의미를 독자적으로 변경할 수 없다. 현재 preview-only set에 아직 itemId가 없다면 G03A에서는 authoring
footprint로만 사용하고, G01 product admission 때 itemId join을 완료하기 전에는 Server 보상/저장 대상으로
승격하지 않는다.

body coverage는 vector index나 raw bit mask를 저장하지 않는다. JSON catalog load는 schema, mode, stable material
ID 문자열과 중복만 검증하며 `CModel`을 만들지 않는다. publisher가 물리 Resources의 class body WModel에서 각
material ID가 정확히 한 submesh에 존재하고 32-bit mask 범위 안인지 admission 전에 검증한다. runtime은 active
Character body model에 대해 preview stage마다 exact-one resolve를 다시 수행한다. publisher 실패는 catalog/data를
교체하지 않고, runtime 실패는 기존 live visual/draft/body mask를 유지한다.

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
- `S2C_INVENTORY_SNAPSHOT`: state revision, equipment semantic revision, item instances, active-class primary-owner loadout
- `S2C_EQUIPMENT_PRESENTATION_SNAPSHOT`: netEntityId, class, state/semantic revision, primary slot별 itemId/instanceId
- `S2C_ITEM_COMMAND_RESULT`: request sequence, typed result, committed revision
- `S2C_RAID_REWARD_RESULT`: immutable roll, auto-claim/pending-full 상태

Owner에게만 전체 inventory를 보내고, room broadcast에는 현재 장착 itemId와 slot만 보낸다.
model asset ID, Prototype tag, pointer, vector index는 wire에 넣지 않는다.

### 5.2 Server player-owned state

`CServerPlayer`가 다음 process-session state를 소유한다.

- inventory item instances
- 여섯 class별 primary `slot -> itemInstanceId` loadout과 tick-local derived secondary owner map
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

`C2S_UNEQUIP_SLOT`은 primary와 secondary를 모두 받을 수 있다. Server는 현재 ItemCatalog footprint로
secondary slot의 primary owner를 먼저 resolve하고 owner item 하나를 원자적으로 해제한다. snapshot과
durable row에는 같은 instance를 secondary slot마다 복제하지 않는다.

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
Client/Bin/Resources/Character/<Class>/Equipment/<VisualSetId>/Textures/<TextureFile>
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

`RaidRewardService`는 `CGameRoom`이 실제로 소유하고 clear tick에서 호출한다. 호출자가 없는 선행 manager를
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

### G03A. 복합 슬롯 owner resolver와 Equipment Tool 분기 통합

G03A와 G03B는 설명 순서만 나눈 하나의 수직 슬라이스이며 같은 commit과 같은 완료 gate로 구현한다.
G03A의 catalog v2가 이미 `bodyCoverage`를 요구하므로 resolver/Tool만 먼저 merge하거나 완료 처리하지 않는다.
한 branch 안에서 `catalog v2 parse -> owner transition -> body/default visibility stage -> part group/body/loadout commit ->
Tool diagnostics/save` 순서로 연결하고, G03B의 실제 Character visual transaction까지 통과해야 사용자에게
“장착·해제가 보인다”고 안내한다.

G03A는 새 Manager를 만들지 않는다. slot enum과 footprint 전이만 소유하는 pure Shared 계약을 두고,
Equipment Tool과 미래 Server item runtime이 각자의 stable owner ID를 같은 전이 함수에 투영한다.
Shared resolver는 `visualSetId`, model path, Client pointer를 알지 않으며 primary slot과 occupied mask만 다룬다.

현재 curated data의 모코코 정책은 이 단계에서 하나로 확정한다. 기존
`character.lance_master.mokoko_pleasant.outfit` stable ID가 `head + dress` 두 physical part를 함께 소유하고,
category는 `APPAREL_FULLBODY`, primary는 `UPPER`, occupied slots는
`HEAD/SHOULDER/UPPER/LOWER/HANDS`다. standalone `character.lance_master.mokoko_pleasant.head` set은 제거한다.
현재 checked-in preset은 모두 `null`이므로 writer migration은 없지만, 외부 authoring preset이 제거된 head ID를
참조하면 조용히 outfit으로 바꾸지 않고 reload를 거부한다. 기존 WModel/texture 162-file Resources closure는
그대로 재사용하므로 binary를 다시 옮기지 않는다. catalog 집계는 22 set/36 part에서 21 set/36 part가 된다.

`EquipmentPresentationCatalog.json` formatVersion 2에서 기존 두 Mokoko row를 교체할 완전한 visual-set row는
다음과 같다. 현재 authoring preview에는 product item이 없으므로 `itemId`를 명시적 `null`로 두며 G01 product
admission 전에는 reward 대상이 아니다.

```json
{
  "visualSetId": "character.lance_master.mokoko_pleasant.outfit",
  "itemId": null,
  "classId": "LANCE_MASTER",
  "categoryId": "APPAREL_FULLBODY",
  "catalogStatus": "READY_ALTERNATIVE",
  "primarySlot": "UPPER",
  "occupiedSlots": [
    "HEAD",
    "SHOULDER",
    "UPPER",
    "LOWER",
    "HANDS"
  ],
  "bodyCoverage": {
    "mode": "INHERIT_BASELINE"
  },
  "parts": [
    {
      "partId": "head",
      "partRole": "HEAD",
      "attachmentMode": "SKINNED",
      "modelAssetId": "Character/LanceMaster/Equipment/mokoko_pleasant_head/head.wmodel",
      "socketBoneId": null,
      "socketYawDegrees": 0.0,
      "requiredStance": null,
      "hiddenMeshMask": 0
    },
    {
      "partId": "dress",
      "partRole": "COMBINED_APPAREL",
      "attachmentMode": "SKINNED",
      "modelAssetId": "Character/LanceMaster/Equipment/mokoko_pleasant_outfit/dress.wmodel",
      "socketBoneId": null,
      "socketYawDegrees": 0.0,
      "requiredStance": null,
      "hiddenMeshMask": 0
    }
  ]
}
```

위 row의 생성 정본인 `RepresentativeCharacterEquipment.json`에서는 기존 Mokoko 두 set을 다음 한 set으로
교체한다.

```json
{
  "visualSetId": "character.lance_master.mokoko_pleasant.outfit",
  "classId": "LANCE_MASTER",
  "curatedVariantId": "mokoko_pleasant_fullbody",
  "catalogStatus": "READY_ALTERNATIVE",
  "primarySlot": "UPPER",
  "coverageSlots": [
    "HEAD",
    "SHOULDER",
    "UPPER",
    "LOWER",
    "HANDS"
  ],
  "masterBodyAssetId": "Character/LanceMaster/LanceMaster.wmodel",
  "bodyCoveragePolicy": "INHERIT_BASELINE",
  "parts": [
    {
      "partId": "head",
      "sourceClassId": "WARLORD",
      "sourcePackageName": "PC_WR_AV_036",
      "sourceObjectName": "pc_wr_av_036-1_head_sk",
      "expectedSourceRole": "HEAD_OR_HAIR",
      "existingModelAssetId": "Character/LanceMaster/LanceMaster_Helmet_Mokoko.wmodel",
      "targetModelAssetId": "Character/LanceMaster/Equipment/mokoko_pleasant_head/head.wmodel",
      "partRole": "HEAD",
      "attachmentMode": "SKINNED",
      "modelKind": "ANIM_SKINNED"
    },
    {
      "partId": "dress",
      "sourceClassId": "LANCE_MASTER",
      "sourcePackageName": "PC_FT_AV_036",
      "sourceObjectName": "pc_ft_av_036-1_dress_sk",
      "expectedSourceRole": "APPAREL_COMBINED",
      "existingModelAssetId": "Character/LanceMaster/LanceMaster_Upper_Mokoko.wmodel",
      "targetModelAssetId": "Character/LanceMaster/Equipment/mokoko_pleasant_outfit/dress.wmodel",
      "partRole": "COMBINED_APPAREL",
      "attachmentMode": "SKINNED",
      "modelKind": "ANIM_SKINNED"
    }
  ]
}
```

같은 selection 문서의 현재 11개 `primarySlot: "WEAPON"` row는 기존
`bodyCoveragePolicy: "INHERIT_BASELINE"`을 모두 `bodyCoveragePolicy: "NONE"`으로 명시 교체한다. generator가
weapon을 조용히 NONE으로 보정하지 않으며 validator가 `WEAPON_SET + NONE`, `materialIds` 없음만 허용한다.
pipeline test는 11개 weapon row 전부와 최종 presentation weapon set 전부가 NONE인지 집계해 누락 한 건도
실패시킨다.

수정·추가 파일은 다음과 같다.

| 구분 | 저장소 상대 경로 | 역할 |
|---|---|---|
| 수정 | `Tools/CharacterEquipmentPipeline/RepresentativeCharacterEquipment.json` | 모코코 head/dress를 한 fullbody visual set으로 저작하는 선택 정본 |
| 수정 | `Tools/CharacterEquipmentPipeline/new_character_equipment_representative_pack.py` | `APPAREL_FULLBODY`와 multi-part set 생성·집계 |
| 수정 | `Tools/CharacterEquipmentPipeline/publish_character_equipment.py` | selection/admission/presentation category·footprint와 Resources closure 검증 |
| 수정 | `Tools/CharacterEquipmentPipeline/Publish-CharacterEquipment.ps1` | 명시 admission/resource root의 schema·physical validation과 rollback 유지 |
| 수정 | `Tools/GameplayPipeline/Publish-ItemCatalog.ps1` | ItemCatalog semantic과 presentation item join exact-equality 검증 |
| 수정 | `Data/Actors/EquipmentPresentationCatalog.json` | fullbody 1 owner, 2 parts, 5-slot footprint로 publish |
| 검증 | `Data/Actors/EquipmentLoadoutPresets.json` | primary-only format 유지, removed standalone head ID fail-close |
| 추가 | `Shared/Public/Gameplay/EquipmentLoadoutContract.h` | stable slot enum, mask, primary owner projection과 pure stage 결과 선언 |
| 추가 | `Shared/Private/Gameplay/EquipmentLoadoutContract.cpp` | footprint 검증, owner projection, conflict 수집, any-slot unequip 전이 구현 |
| 수정 | `Shared/Default/Shared.vcxproj` | 새 Shared CPP/H 등록 |
| 수정 | `Shared/Default/Shared.vcxproj.filters` | `Gameplay` filter 등록 |
| 수정 | `Client/Public/EquipmentPresentationCatalog.h` | Client 전용 slot enum 제거, visual loadout/resolution view 선언 |
| 수정 | `Client/Private/EquipmentPresentationCatalog.cpp` | catalog row를 Shared footprint로 변환하고 category/slot 불변식 검증 |
| 추가 | `Client/Public/EquipmentVisualLoadoutPolicy.h` | stable visualSetId와 Shared footprint를 결합하는 pure primary-selection stage 선언 |
| 추가 | `Client/Private/EquipmentVisualLoadoutPolicy.cpp` | weapon A→B identity를 포함한 primary 문자열 전이 구현 |
| 추가 | `Client/Public/EquipmentAuthoringPresetCodec.h`, `Client/Private/EquipmentAuthoringPresetCodec.cpp` | candidate와 분리된 primary-only deterministic preset serialization |
| 수정 | `Client/Public/EquipmentAuthoringTool.h` | primary selection draft와 staged impact/resolution 함수 선언 |
| 수정 | `Client/Private/EquipmentAuthoringTool.cpp` | affected-slot 목록, 영향 진단, equip/unequip/save/reload 분기 통합 |
| 수정 | `Client/Public/EquipmentPresentationService.h` | validated loadout resolution 소비 계약 반영 |
| 수정 | `Client/Private/EquipmentPresentationService.cpp` | Tool과 별도 mask 계산을 제거하고 같은 resolution으로 part stage |
| 수정 | `Tools/CharacterSelectIsolationHarness/Private/ClientPresentationPrimitiveContractTests.cpp` | pure resolver의 모코코 전환 행렬 실행 |
| 수정 | `Client/Default/Client.vcxproj`, `Client/Default/Client.vcxproj.filters` | 새 visual policy H/CPP 물리 경로 등록 |
| 수정 | `Tools/CharacterSelectIsolationHarness/Default/CharacterSelectIsolationHarness.vcxproj`, `.filters` | 같은 pure policy CPP를 harness binary에 컴파일해 identity 전이 실행 |
| 수정 | `Tools/CharacterEquipmentPipeline/test_equipment_authoring_tool_contract.py` | Tool이 Shared resolver만 소비하고 baked Mokoko가 없는지 계약 검증 |
| 수정 | `Tools/CharacterEquipmentPipeline/test_equipment_catalogs.py` | apparel/weapon footprint와 primary 포함 규칙 검증 |
| 수정 | `Tools/CharacterEquipmentPipeline/test_new_character_equipment_representative_pack.py` | 21-set/36-part fullbody 생성과 part-role 검증 |
| 수정 | `Tools/CharacterEquipmentPipeline/test_publish_character_equipment.py` | exact body material/resource closure와 staged publish rollback 검증 |

G03A/G03B는 Git 제외 `out` admission과 팀 물리 Resources를 모든 팀원의 공용 build graph 필수 입력으로 만들지
않으므로 `Tools/Build/BuildDomains.json`을 수정하지 않는다. tracked fixture unittest는 아래 명시 명령으로,
현재 worktree의 실제 admission/Resources exact 검증은 mandatory parameter를 모두 준 publisher 명령으로 실행한다.

`EquipmentLoadoutContract`의 runtime 값은 다음 의미만 가진다.

- `EQUIPMENT_SLOT_ID`: `HEAD, SHOULDER, UPPER, LOWER, HANDS, WEAPON`의 명시적 stable numeric enum
- `EQUIPMENT_SLOT_MASK`: 여섯 stable slot bit만 허용하는 ephemeral projection 값
- `EQUIPMENT_OWNER_FOOTPRINT`: primary slot과 primary bit를 포함한 occupied mask
- `EQUIPMENT_SLOT_PROJECTION`: 각 slot이 default인지, primary owner인지, 어느 primary에 covered됐는지 나타내는
  stage 결과
- `EQUIPMENT_LOADOUT_TRANSITION`: 제거할 primary slot mask, 추가할 candidate primary와 최종 projection

mask는 wire나 durable DB의 저장 정본이 아니다. preview-only Tool은 presentation catalog의 stable slot 목록을,
제품 Server는 ItemCatalog의 stable slot 목록을 검증한 뒤 한 tick/한 transaction에서만 mask를 만든다. Shared는
owner의 실제 ID를 소유하지 않으며 Tool은 primary 위치의 `visualSetId`, Server는 primary 위치의
`itemInstanceId`와 projection을 결합한다.

pure resolver의 함수 책임은 다음과 같이 고정한다.

1. `Build_EquipmentSlotMask`는 primary 포함, 중복, invalid/high bit, empty footprint를 거부한다.
2. `Project_EquipmentSlotOwners`는 primary owner 목록 전체를 검증하고 두 footprint가 한 slot이라도 겹치면
   실패하며 기존 output을 바꾸지 않는다.
3. `Stage_EquipEquipmentOwner`는 candidate와 겹치는 모든 기존 primary owner를 transition의 remove mask에
   넣고, 겹치지 않는 owner를 보존한 최종 projection을 만든다.
4. `Stage_UnequipEquipmentSlot`은 primary/secondary 어느 slot이든 projection에서 owner primary를 찾아
   owner 하나 전체를 제거한다. default slot은 typed `ALREADY_DEFAULT` 결과다.
5. 입력 owner set 자체가 invalid하면 자동 수리하지 않고 실패해 호출자의 기존 draft/state를 보존한다.

catalog validator는 resolver 호출 전에 semantic footprint를 닫는다.

- 모든 set은 primary를 occupied에 정확히 한 번 포함하며 occupied는 nonempty/unique다.
- `WEAPON_SET`은 primary `WEAPON`, occupied 정확히 `{WEAPON}`이다.
- `WEAPON_SET`의 body coverage는 정확히 `NONE`이며 material ID를 가질 수 없다.
- 모든 `APPAREL_*`은 `WEAPON`을 점유할 수 없다.
- slot-specific apparel의 primary는 category와 일치한다.
- `APPAREL_OUTFIT`은 primary `UPPER`, occupied는 `{SHOULDER, UPPER, LOWER, HANDS}`의 UPPER를 포함한
  두 개 이상 subset이며 `HEAD/WEAPON`을 포함하지 않는다.
- `APPAREL_FULLBODY`는 primary `UPPER`, occupied 정확히
  `{HEAD, SHOULDER, UPPER, LOWER, HANDS}`이며 parts에 skinned head와 combined apparel 역할이 모두 있다.
- product set은 ItemCatalog의 같은 `itemId/classId`와 category/primary/occupied가 exact-equal해야 한다.

`Tools/CharacterEquipmentPipeline/new_character_equipment_representative_pack.py`의 기존
`derive_category_id` 전체는 다음 블록으로 교체한다. JSON `coverageSlots` 중복 검증은 set으로 바꾸기 전에
`validate_selection_document`가 수행하고, fullbody 두 part 역할 검증도 같은 함수에서 이어서 수행한다.

```python
APPAREL_SLOTS = frozenset({"HEAD", "SHOULDER", "UPPER", "LOWER", "HANDS"})
OUTFIT_SLOTS = frozenset({"SHOULDER", "UPPER", "LOWER", "HANDS"})
FULLBODY_SLOTS = APPAREL_SLOTS


def derive_category_id(primary_slot: str, coverage_slots: Iterable[str]) -> str:
    coverage = frozenset(coverage_slots)
    require(primary_slot in ALLOWED_SLOTS, f"unknown primary slot: {primary_slot}")
    require(coverage and primary_slot in coverage, "coverage must contain primary")
    if primary_slot == "WEAPON":
        require(coverage == {"WEAPON"}, "weapon must occupy only WEAPON")
        return "WEAPON_SET"
    require("WEAPON" not in coverage, "apparel cannot occupy WEAPON")
    if primary_slot == "UPPER" and coverage == FULLBODY_SLOTS:
        return "APPAREL_FULLBODY"
    if (primary_slot == "UPPER" and coverage.issubset(OUTFIT_SLOTS)
            and len(coverage) > 1):
        return "APPAREL_OUTFIT"
    require(coverage == {primary_slot}, "slot apparel must occupy only its primary")
    return f"APPAREL_{primary_slot}"
```

fullbody의 `parts` parse 직후에는 다음 검증 블록을 추가한다.

```python
if category_id == "APPAREL_FULLBODY":
    require(len(parts) == 2, f"fullbody must contain two parts: {visual_set_id}")
    roles = {(part.get("partRole"), part.get("attachmentMode")) for part in parts}
    require(
        roles == {("HEAD", "SKINNED"), ("COMBINED_APPAREL", "SKINNED")},
        f"fullbody roles differ: {visual_set_id}",
    )
```

#### G03A-1. `EquipmentLoadoutContract.h` 전체 코드

파일: `Shared/Public/Gameplay/EquipmentLoadoutContract.h`

```cpp
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace LostArk::Shared
{
	enum class EQUIPMENT_SLOT_ID : std::uint8_t
	{
		HEAD = 0u,
		SHOULDER = 1u,
		UPPER = 2u,
		LOWER = 3u,
		HANDS = 4u,
		WEAPON = 5u,
		END = 6u
	};

	using EQUIPMENT_SLOT_MASK = std::uint32_t;

	inline constexpr std::size_t EQUIPMENT_SLOT_COUNT =
		static_cast<std::size_t>(EQUIPMENT_SLOT_ID::END);
	inline constexpr EQUIPMENT_SLOT_MASK EQUIPMENT_SLOT_MASK_ALL =
		(1u << EQUIPMENT_SLOT_COUNT) - 1u;

	[[nodiscard]] constexpr bool Is_ValidEquipmentSlot(
		const EQUIPMENT_SLOT_ID slot) noexcept
	{
		return slot < EQUIPMENT_SLOT_ID::END;
	}

	[[nodiscard]] constexpr EQUIPMENT_SLOT_MASK EquipmentSlotMask(
		const EQUIPMENT_SLOT_ID slot) noexcept
	{
		return Is_ValidEquipmentSlot(slot) ?
			1u << static_cast<std::uint32_t>(slot) : 0u;
	}

	struct EQUIPMENT_OWNER_FOOTPRINT final
	{
		EQUIPMENT_SLOT_ID ePrimarySlot = EQUIPMENT_SLOT_ID::END;
		EQUIPMENT_SLOT_MASK iOccupiedSlotsMask = 0u;

		friend bool operator==(
			const EQUIPMENT_OWNER_FOOTPRINT&,
			const EQUIPMENT_OWNER_FOOTPRINT&) = default;
	};

	enum class EQUIPMENT_SLOT_OCCUPANCY : std::uint8_t
	{
		DEFAULT,
		PRIMARY,
		COVERED,
		END
	};

	struct EQUIPMENT_SLOT_PROJECTION_ENTRY final
	{
		EQUIPMENT_SLOT_OCCUPANCY eOccupancy =
			EQUIPMENT_SLOT_OCCUPANCY::DEFAULT;
		EQUIPMENT_SLOT_ID eOwnerPrimarySlot = EQUIPMENT_SLOT_ID::END;

		friend bool operator==(
			const EQUIPMENT_SLOT_PROJECTION_ENTRY&,
			const EQUIPMENT_SLOT_PROJECTION_ENTRY&) = default;
	};

	struct EQUIPMENT_SLOT_PROJECTION final
	{
		std::array<EQUIPMENT_SLOT_PROJECTION_ENTRY,
			EQUIPMENT_SLOT_COUNT> Slots{};
		EQUIPMENT_SLOT_MASK iOccupiedSlotsMask = 0u;
		EQUIPMENT_SLOT_MASK iPrimarySlotsMask = 0u;

		friend bool operator==(
			const EQUIPMENT_SLOT_PROJECTION&,
			const EQUIPMENT_SLOT_PROJECTION&) = default;
	};

	enum class EQUIPMENT_LOADOUT_RESULT : std::uint8_t
	{
		SUCCESS,
		ALREADY_DEFAULT,
		INVALID_SLOT,
		EMPTY_FOOTPRINT,
		TOO_MANY_OCCUPIED_SLOTS,
		DUPLICATE_OCCUPIED_SLOT,
		PRIMARY_NOT_OCCUPIED,
		INVALID_OCCUPIED_MASK,
		TOO_MANY_OWNERS,
		DUPLICATE_PRIMARY_OWNER,
		OVERLAPPING_OWNER,
		END
	};

	struct EQUIPMENT_LOADOUT_TRANSITION final
	{
		EQUIPMENT_SLOT_MASK iRemovedPrimarySlotsMask = 0u;
		EQUIPMENT_SLOT_MASK iPreservedPrimarySlotsMask = 0u;
		bool bHasAddedOwner = false;
		EQUIPMENT_OWNER_FOOTPRINT AddedOwner{};
		EQUIPMENT_SLOT_PROJECTION FinalProjection{};

		friend bool operator==(
			const EQUIPMENT_LOADOUT_TRANSITION&,
			const EQUIPMENT_LOADOUT_TRANSITION&) = default;
	};

	[[nodiscard]] EQUIPMENT_LOADOUT_RESULT Build_EquipmentSlotMask(
		EQUIPMENT_SLOT_ID primarySlot,
		std::span<const EQUIPMENT_SLOT_ID> occupiedSlots,
		EQUIPMENT_OWNER_FOOTPRINT& outFootprint) noexcept;

	[[nodiscard]] EQUIPMENT_LOADOUT_RESULT
		Validate_EquipmentOwnerFootprint(
			const EQUIPMENT_OWNER_FOOTPRINT& footprint) noexcept;

	[[nodiscard]] EQUIPMENT_LOADOUT_RESULT Project_EquipmentSlotOwners(
		std::span<const EQUIPMENT_OWNER_FOOTPRINT> owners,
		EQUIPMENT_SLOT_PROJECTION& outProjection) noexcept;

	[[nodiscard]] EQUIPMENT_LOADOUT_RESULT Stage_EquipEquipmentOwner(
		std::span<const EQUIPMENT_OWNER_FOOTPRINT> owners,
		const EQUIPMENT_OWNER_FOOTPRINT& candidate,
		EQUIPMENT_LOADOUT_TRANSITION& outTransition) noexcept;

	[[nodiscard]] EQUIPMENT_LOADOUT_RESULT Stage_UnequipEquipmentSlot(
		std::span<const EQUIPMENT_OWNER_FOOTPRINT> owners,
		EQUIPMENT_SLOT_ID selectedSlot,
		EQUIPMENT_LOADOUT_TRANSITION& outTransition) noexcept;

	[[nodiscard]] EQUIPMENT_LOADOUT_RESULT Stage_UnequipAllEquipment(
		std::span<const EQUIPMENT_OWNER_FOOTPRINT> owners,
		EQUIPMENT_LOADOUT_TRANSITION& outTransition) noexcept;

	[[nodiscard]] EQUIPMENT_SLOT_ID Get_EquipmentOwnerPrimarySlot(
		const EQUIPMENT_SLOT_PROJECTION& projection,
		EQUIPMENT_SLOT_ID slot) noexcept;

	[[nodiscard]] const char* Get_EquipmentLoadoutResultText(
		EQUIPMENT_LOADOUT_RESULT result) noexcept;
}
```

#### G03A-2. `EquipmentLoadoutContract.cpp` 전체 코드

파일: `Shared/Private/Gameplay/EquipmentLoadoutContract.cpp`

```cpp
#include "Gameplay/EquipmentLoadoutContract.h"

LostArk::Shared::EQUIPMENT_LOADOUT_RESULT
LostArk::Shared::Build_EquipmentSlotMask(
	const EQUIPMENT_SLOT_ID primarySlot,
	const std::span<const EQUIPMENT_SLOT_ID> occupiedSlots,
	EQUIPMENT_OWNER_FOOTPRINT& outFootprint) noexcept
{
	if (!Is_ValidEquipmentSlot(primarySlot))
		return EQUIPMENT_LOADOUT_RESULT::INVALID_SLOT;
	if (occupiedSlots.empty())
		return EQUIPMENT_LOADOUT_RESULT::EMPTY_FOOTPRINT;
	if (occupiedSlots.size() > EQUIPMENT_SLOT_COUNT)
		return EQUIPMENT_LOADOUT_RESULT::TOO_MANY_OCCUPIED_SLOTS;

	EQUIPMENT_SLOT_MASK occupiedMask = 0u;
	for (const EQUIPMENT_SLOT_ID slot : occupiedSlots)
	{
		if (!Is_ValidEquipmentSlot(slot))
			return EQUIPMENT_LOADOUT_RESULT::INVALID_SLOT;

		const EQUIPMENT_SLOT_MASK slotMask = EquipmentSlotMask(slot);
		if (0u != (occupiedMask & slotMask))
			return EQUIPMENT_LOADOUT_RESULT::DUPLICATE_OCCUPIED_SLOT;
		occupiedMask |= slotMask;
	}

	if (0u == (occupiedMask & EquipmentSlotMask(primarySlot)))
		return EQUIPMENT_LOADOUT_RESULT::PRIMARY_NOT_OCCUPIED;

	EQUIPMENT_OWNER_FOOTPRINT staged{};
	staged.ePrimarySlot = primarySlot;
	staged.iOccupiedSlotsMask = occupiedMask;
	outFootprint = staged;
	return EQUIPMENT_LOADOUT_RESULT::SUCCESS;
}

LostArk::Shared::EQUIPMENT_LOADOUT_RESULT
LostArk::Shared::Validate_EquipmentOwnerFootprint(
	const EQUIPMENT_OWNER_FOOTPRINT& footprint) noexcept
{
	if (!Is_ValidEquipmentSlot(footprint.ePrimarySlot))
		return EQUIPMENT_LOADOUT_RESULT::INVALID_SLOT;
	if (0u == footprint.iOccupiedSlotsMask)
		return EQUIPMENT_LOADOUT_RESULT::EMPTY_FOOTPRINT;
	if (0u != (footprint.iOccupiedSlotsMask & ~EQUIPMENT_SLOT_MASK_ALL))
		return EQUIPMENT_LOADOUT_RESULT::INVALID_OCCUPIED_MASK;
	if (0u == (footprint.iOccupiedSlotsMask &
		EquipmentSlotMask(footprint.ePrimarySlot)))
	{
		return EQUIPMENT_LOADOUT_RESULT::PRIMARY_NOT_OCCUPIED;
	}
	return EQUIPMENT_LOADOUT_RESULT::SUCCESS;
}

LostArk::Shared::EQUIPMENT_LOADOUT_RESULT
LostArk::Shared::Project_EquipmentSlotOwners(
	const std::span<const EQUIPMENT_OWNER_FOOTPRINT> owners,
	EQUIPMENT_SLOT_PROJECTION& outProjection) noexcept
{
	if (owners.size() > EQUIPMENT_SLOT_COUNT)
		return EQUIPMENT_LOADOUT_RESULT::TOO_MANY_OWNERS;

	EQUIPMENT_SLOT_PROJECTION staged{};
	for (const EQUIPMENT_OWNER_FOOTPRINT& owner : owners)
	{
		const EQUIPMENT_LOADOUT_RESULT validation =
			Validate_EquipmentOwnerFootprint(owner);
		if (EQUIPMENT_LOADOUT_RESULT::SUCCESS != validation)
			return validation;

		const EQUIPMENT_SLOT_MASK primaryMask =
			EquipmentSlotMask(owner.ePrimarySlot);
		if (0u != (staged.iPrimarySlotsMask & primaryMask))
			return EQUIPMENT_LOADOUT_RESULT::DUPLICATE_PRIMARY_OWNER;
		if (0u != (staged.iOccupiedSlotsMask & owner.iOccupiedSlotsMask))
			return EQUIPMENT_LOADOUT_RESULT::OVERLAPPING_OWNER;

		staged.iPrimarySlotsMask |= primaryMask;
		staged.iOccupiedSlotsMask |= owner.iOccupiedSlotsMask;

		for (std::size_t slotIndex = 0u;
			slotIndex < EQUIPMENT_SLOT_COUNT; ++slotIndex)
		{
			const EQUIPMENT_SLOT_ID slot =
				static_cast<EQUIPMENT_SLOT_ID>(slotIndex);
			const EQUIPMENT_SLOT_MASK slotMask = EquipmentSlotMask(slot);
			if (0u == (owner.iOccupiedSlotsMask & slotMask))
				continue;

			EQUIPMENT_SLOT_PROJECTION_ENTRY& entry =
				staged.Slots[slotIndex];
			entry.eOccupancy = slot == owner.ePrimarySlot ?
				EQUIPMENT_SLOT_OCCUPANCY::PRIMARY :
				EQUIPMENT_SLOT_OCCUPANCY::COVERED;
			entry.eOwnerPrimarySlot = owner.ePrimarySlot;
		}
	}

	outProjection = staged;
	return EQUIPMENT_LOADOUT_RESULT::SUCCESS;
}

LostArk::Shared::EQUIPMENT_LOADOUT_RESULT
LostArk::Shared::Stage_EquipEquipmentOwner(
	const std::span<const EQUIPMENT_OWNER_FOOTPRINT> owners,
	const EQUIPMENT_OWNER_FOOTPRINT& candidate,
	EQUIPMENT_LOADOUT_TRANSITION& outTransition) noexcept
{
	EQUIPMENT_SLOT_PROJECTION current{};
	EQUIPMENT_LOADOUT_RESULT result =
		Project_EquipmentSlotOwners(owners, current);
	if (EQUIPMENT_LOADOUT_RESULT::SUCCESS != result)
		return result;

	result = Validate_EquipmentOwnerFootprint(candidate);
	if (EQUIPMENT_LOADOUT_RESULT::SUCCESS != result)
		return result;

	std::array<EQUIPMENT_OWNER_FOOTPRINT,
		EQUIPMENT_SLOT_COUNT> finalOwners{};
	std::size_t finalOwnerCount = 0u;

	EQUIPMENT_LOADOUT_TRANSITION staged{};
	staged.iPreservedPrimarySlotsMask = current.iPrimarySlotsMask;

	for (const EQUIPMENT_OWNER_FOOTPRINT& owner : owners)
	{
		const EQUIPMENT_SLOT_MASK ownerPrimaryMask =
			EquipmentSlotMask(owner.ePrimarySlot);
		if (0u != (owner.iOccupiedSlotsMask &
			candidate.iOccupiedSlotsMask))
		{
			staged.iRemovedPrimarySlotsMask |= ownerPrimaryMask;
			staged.iPreservedPrimarySlotsMask &= ~ownerPrimaryMask;
			continue;
		}
		finalOwners[finalOwnerCount++] = owner;
	}

	finalOwners[finalOwnerCount++] = candidate;
	EQUIPMENT_SLOT_PROJECTION finalProjection{};
	result = Project_EquipmentSlotOwners(
		std::span<const EQUIPMENT_OWNER_FOOTPRINT>(
			finalOwners.data(), finalOwnerCount),
		finalProjection);
	if (EQUIPMENT_LOADOUT_RESULT::SUCCESS != result)
		return result;

	staged.bHasAddedOwner = true;
	staged.AddedOwner = candidate;
	staged.FinalProjection = finalProjection;
	outTransition = staged;
	return EQUIPMENT_LOADOUT_RESULT::SUCCESS;
}

LostArk::Shared::EQUIPMENT_LOADOUT_RESULT
LostArk::Shared::Stage_UnequipEquipmentSlot(
	const std::span<const EQUIPMENT_OWNER_FOOTPRINT> owners,
	const EQUIPMENT_SLOT_ID selectedSlot,
	EQUIPMENT_LOADOUT_TRANSITION& outTransition) noexcept
{
	if (!Is_ValidEquipmentSlot(selectedSlot))
		return EQUIPMENT_LOADOUT_RESULT::INVALID_SLOT;

	EQUIPMENT_SLOT_PROJECTION current{};
	EQUIPMENT_LOADOUT_RESULT result =
		Project_EquipmentSlotOwners(owners, current);
	if (EQUIPMENT_LOADOUT_RESULT::SUCCESS != result)
		return result;

	const EQUIPMENT_SLOT_PROJECTION_ENTRY& selected =
		current.Slots[static_cast<std::size_t>(selectedSlot)];
	if (EQUIPMENT_SLOT_OCCUPANCY::DEFAULT == selected.eOccupancy)
		return EQUIPMENT_LOADOUT_RESULT::ALREADY_DEFAULT;

	const EQUIPMENT_SLOT_ID removedPrimary = selected.eOwnerPrimarySlot;
	const EQUIPMENT_SLOT_MASK removedPrimaryMask =
		EquipmentSlotMask(removedPrimary);

	std::array<EQUIPMENT_OWNER_FOOTPRINT,
		EQUIPMENT_SLOT_COUNT> finalOwners{};
	std::size_t finalOwnerCount = 0u;
	for (const EQUIPMENT_OWNER_FOOTPRINT& owner : owners)
	{
		if (owner.ePrimarySlot == removedPrimary)
			continue;
		finalOwners[finalOwnerCount++] = owner;
	}

	EQUIPMENT_SLOT_PROJECTION finalProjection{};
	result = Project_EquipmentSlotOwners(
		std::span<const EQUIPMENT_OWNER_FOOTPRINT>(
			finalOwners.data(), finalOwnerCount),
		finalProjection);
	if (EQUIPMENT_LOADOUT_RESULT::SUCCESS != result)
		return result;

	EQUIPMENT_LOADOUT_TRANSITION staged{};
	staged.iRemovedPrimarySlotsMask = removedPrimaryMask;
	staged.iPreservedPrimarySlotsMask =
		current.iPrimarySlotsMask & ~removedPrimaryMask;
	staged.FinalProjection = finalProjection;
	outTransition = staged;
	return EQUIPMENT_LOADOUT_RESULT::SUCCESS;
}

LostArk::Shared::EQUIPMENT_LOADOUT_RESULT
LostArk::Shared::Stage_UnequipAllEquipment(
	const std::span<const EQUIPMENT_OWNER_FOOTPRINT> owners,
	EQUIPMENT_LOADOUT_TRANSITION& outTransition) noexcept
{
	EQUIPMENT_SLOT_PROJECTION current{};
	const EQUIPMENT_LOADOUT_RESULT result =
		Project_EquipmentSlotOwners(owners, current);
	if (EQUIPMENT_LOADOUT_RESULT::SUCCESS != result)
		return result;
	if (0u == current.iPrimarySlotsMask)
		return EQUIPMENT_LOADOUT_RESULT::ALREADY_DEFAULT;

	EQUIPMENT_SLOT_PROJECTION finalProjection{};
	const EQUIPMENT_LOADOUT_RESULT finalResult =
		Project_EquipmentSlotOwners(
			std::span<const EQUIPMENT_OWNER_FOOTPRINT>{},
			finalProjection);
	if (EQUIPMENT_LOADOUT_RESULT::SUCCESS != finalResult)
		return finalResult;

	EQUIPMENT_LOADOUT_TRANSITION staged{};
	staged.iRemovedPrimarySlotsMask = current.iPrimarySlotsMask;
	staged.FinalProjection = finalProjection;
	outTransition = staged;
	return EQUIPMENT_LOADOUT_RESULT::SUCCESS;
}

LostArk::Shared::EQUIPMENT_SLOT_ID
LostArk::Shared::Get_EquipmentOwnerPrimarySlot(
	const EQUIPMENT_SLOT_PROJECTION& projection,
	const EQUIPMENT_SLOT_ID slot) noexcept
{
	if (!Is_ValidEquipmentSlot(slot))
		return EQUIPMENT_SLOT_ID::END;

	const EQUIPMENT_SLOT_PROJECTION_ENTRY& entry =
		projection.Slots[static_cast<std::size_t>(slot)];
	if (EQUIPMENT_SLOT_OCCUPANCY::DEFAULT == entry.eOccupancy ||
		!Is_ValidEquipmentSlot(entry.eOwnerPrimarySlot))
	{
		return EQUIPMENT_SLOT_ID::END;
	}
	return entry.eOwnerPrimarySlot;
}

const char* LostArk::Shared::Get_EquipmentLoadoutResultText(
	const EQUIPMENT_LOADOUT_RESULT result) noexcept
{
	switch (result)
	{
	case EQUIPMENT_LOADOUT_RESULT::SUCCESS:
		return "success";
	case EQUIPMENT_LOADOUT_RESULT::ALREADY_DEFAULT:
		return "already default";
	case EQUIPMENT_LOADOUT_RESULT::INVALID_SLOT:
		return "invalid slot";
	case EQUIPMENT_LOADOUT_RESULT::EMPTY_FOOTPRINT:
		return "empty footprint";
	case EQUIPMENT_LOADOUT_RESULT::TOO_MANY_OCCUPIED_SLOTS:
		return "too many occupied slots";
	case EQUIPMENT_LOADOUT_RESULT::DUPLICATE_OCCUPIED_SLOT:
		return "duplicate occupied slot";
	case EQUIPMENT_LOADOUT_RESULT::PRIMARY_NOT_OCCUPIED:
		return "primary slot is not occupied";
	case EQUIPMENT_LOADOUT_RESULT::INVALID_OCCUPIED_MASK:
		return "occupied mask contains an unsupported bit";
	case EQUIPMENT_LOADOUT_RESULT::TOO_MANY_OWNERS:
		return "too many primary owners";
	case EQUIPMENT_LOADOUT_RESULT::DUPLICATE_PRIMARY_OWNER:
		return "duplicate primary owner";
	case EQUIPMENT_LOADOUT_RESULT::OVERLAPPING_OWNER:
		return "owner footprints overlap";
	default:
		return "unknown equipment loadout result";
	}
}
```

#### G03A-3. Shared project/filter 등록 블록

`Shared/Default/Shared.vcxproj`의 기존 `CombatCollisionContract` 항목 바로 뒤에 각각 추가한다.

```xml
<ClInclude Include="..\Public\Gameplay\EquipmentLoadoutContract.h" />
<ClCompile Include="..\Private\Gameplay\EquipmentLoadoutContract.cpp" />
```

`Shared/Default/Shared.vcxproj.filters`의 같은 항목 바로 뒤에 추가한다. 기존
`Public\Gameplay`, `Private\Gameplay` filter를 사용하므로 새 filter GUID는 만들지 않는다.

```xml
<ClInclude Include="..\Public\Gameplay\EquipmentLoadoutContract.h">
  <Filter>Public\Gameplay</Filter>
</ClInclude>
<ClCompile Include="..\Private\Gameplay\EquipmentLoadoutContract.cpp">
  <Filter>Private\Gameplay</Filter>
</ClCompile>
```

#### G03A-4. native resolver harness 교체 블록

`Tools/CharacterSelectIsolationHarness/Private/ClientPresentationPrimitiveContractTests.cpp`의 include에는
`Gameplay/EquipmentLoadoutContract.h`, `EquipmentVisualLoadoutPolicy.h`,
`EquipmentAuthoringPresetCodec.h`, `<array>`를 추가한다. anonymous namespace에서
`VerifyPartyTransferNotice` 바로 앞에 다음 함수를 넣는다.

```cpp
	bool VerifyCompositeEquipmentLoadout()
	{
		using namespace LostArk::Shared;
		using Slot = EQUIPMENT_SLOT_ID;
		using Footprint = EQUIPMENT_OWNER_FOOTPRINT;
		using Result = EQUIPMENT_LOADOUT_RESULT;
		using Occupancy = EQUIPMENT_SLOT_OCCUPANCY;

		const std::array<Slot, 5u> fullbodySlots{
			Slot::HEAD,
			Slot::SHOULDER,
			Slot::UPPER,
			Slot::LOWER,
			Slot::HANDS
		};
		const std::array<Slot, 1u> weaponSlots{ Slot::WEAPON };
		const std::array<Slot, 1u> lowerSlots{ Slot::LOWER };
		const std::array<Slot, 1u> headSlots{ Slot::HEAD };

		Footprint fullbody{};
		Footprint weapon{};
		Footprint lower{};
		Footprint head{};
		if (!Require(
			Result::SUCCESS == Build_EquipmentSlotMask(
				Slot::UPPER, fullbodySlots, fullbody) &&
			Result::SUCCESS == Build_EquipmentSlotMask(
				Slot::WEAPON, weaponSlots, weapon) &&
			Result::SUCCESS == Build_EquipmentSlotMask(
				Slot::LOWER, lowerSlots, lower) &&
			Result::SUCCESS == Build_EquipmentSlotMask(
				Slot::HEAD, headSlots, head) &&
			0x1fu == fullbody.iOccupiedSlotsMask &&
			0x20u == weapon.iOccupiedSlotsMask,
			"Mokoko fullbody/weapon footprints were not stable"))
		{
			return false;
		}

		Footprint unchanged = weapon;
		const std::array<Slot, 2u> duplicateSlots{
			Slot::HEAD, Slot::HEAD
		};
		const std::array<Slot, 1u> missingPrimary{ Slot::LOWER };
		if (!Require(
			Result::DUPLICATE_OCCUPIED_SLOT ==
				Build_EquipmentSlotMask(
					Slot::HEAD, duplicateSlots, unchanged) &&
			unchanged == weapon &&
			Result::PRIMARY_NOT_OCCUPIED ==
				Build_EquipmentSlotMask(
					Slot::HEAD, missingPrimary, unchanged) &&
			unchanged == weapon,
			"invalid footprint partially replaced the caller output"))
		{
			return false;
		}

		const std::array<Footprint, 2u> fullbodyAndWeapon{
			fullbody, weapon
		};
		EQUIPMENT_SLOT_PROJECTION projection{};
		if (!Require(
			Result::SUCCESS == Project_EquipmentSlotOwners(
				fullbodyAndWeapon, projection) &&
			0x3fu == projection.iOccupiedSlotsMask &&
			(EquipmentSlotMask(Slot::UPPER) |
				EquipmentSlotMask(Slot::WEAPON)) ==
				projection.iPrimarySlotsMask &&
			Occupancy::COVERED ==
				projection.Slots[
					static_cast<std::size_t>(Slot::HEAD)].eOccupancy &&
			Slot::UPPER ==
				projection.Slots[
					static_cast<std::size_t>(Slot::HEAD)].
						eOwnerPrimarySlot &&
			Occupancy::PRIMARY ==
				projection.Slots[
					static_cast<std::size_t>(Slot::UPPER)].eOccupancy &&
			Occupancy::PRIMARY ==
				projection.Slots[
					static_cast<std::size_t>(Slot::WEAPON)].eOccupancy,
			"fullbody and independent weapon projection was incorrect"))
		{
			return false;
		}

		EQUIPMENT_LOADOUT_TRANSITION weaponSwap{};
		if (!Require(
			Result::SUCCESS == Stage_EquipEquipmentOwner(
				fullbodyAndWeapon, weapon, weaponSwap) &&
			EquipmentSlotMask(Slot::WEAPON) ==
				weaponSwap.iRemovedPrimarySlotsMask &&
			EquipmentSlotMask(Slot::UPPER) ==
				weaponSwap.iPreservedPrimarySlotsMask &&
			0x3fu == weaponSwap.FinalProjection.iOccupiedSlotsMask &&
			Slot::UPPER == Get_EquipmentOwnerPrimarySlot(
				weaponSwap.FinalProjection, Slot::HEAD),
			"weapon replacement removed or changed Mokoko fullbody"))
		{
			return false;
		}

		EQUIPMENT_LOADOUT_TRANSITION lowerEquip{};
		if (!Require(
			Result::SUCCESS == Stage_EquipEquipmentOwner(
				fullbodyAndWeapon, lower, lowerEquip) &&
			EquipmentSlotMask(Slot::UPPER) ==
				lowerEquip.iRemovedPrimarySlotsMask &&
			EquipmentSlotMask(Slot::WEAPON) ==
				lowerEquip.iPreservedPrimarySlotsMask &&
			(EquipmentSlotMask(Slot::LOWER) |
				EquipmentSlotMask(Slot::WEAPON)) ==
				lowerEquip.FinalProjection.iOccupiedSlotsMask &&
			Slot::END == Get_EquipmentOwnerPrimarySlot(
				lowerEquip.FinalProjection, Slot::HEAD) &&
			Slot::WEAPON == Get_EquipmentOwnerPrimarySlot(
				lowerEquip.FinalProjection, Slot::WEAPON),
			"partial apparel replacement did not evict fullbody as one owner"))
		{
			return false;
		}

		EQUIPMENT_LOADOUT_TRANSITION coveredUnequip{};
		if (!Require(
			Result::SUCCESS == Stage_UnequipEquipmentSlot(
				fullbodyAndWeapon, Slot::HANDS, coveredUnequip) &&
			EquipmentSlotMask(Slot::UPPER) ==
				coveredUnequip.iRemovedPrimarySlotsMask &&
			EquipmentSlotMask(Slot::WEAPON) ==
				coveredUnequip.iPreservedPrimarySlotsMask &&
			EquipmentSlotMask(Slot::WEAPON) ==
				coveredUnequip.FinalProjection.iOccupiedSlotsMask,
			"covered-slot unequip did not remove its complete owner"))
		{
			return false;
		}

		EQUIPMENT_LOADOUT_TRANSITION weaponUnequip{};
		if (!Require(
			Result::SUCCESS == Stage_UnequipEquipmentSlot(
				fullbodyAndWeapon, Slot::WEAPON, weaponUnequip) &&
			EquipmentSlotMask(Slot::WEAPON) ==
				weaponUnequip.iRemovedPrimarySlotsMask &&
			EquipmentSlotMask(Slot::UPPER) ==
				weaponUnequip.iPreservedPrimarySlotsMask &&
			0x1fu == weaponUnequip.FinalProjection.iOccupiedSlotsMask,
			"weapon unequip removed fullbody apparel"))
		{
			return false;
		}

		EQUIPMENT_LOADOUT_TRANSITION unequipAll{};
		if (!Require(
			Result::SUCCESS == Stage_UnequipAllEquipment(
				fullbodyAndWeapon, unequipAll) &&
			(EquipmentSlotMask(Slot::UPPER) |
				EquipmentSlotMask(Slot::WEAPON)) ==
				unequipAll.iRemovedPrimarySlotsMask &&
			0u == unequipAll.FinalProjection.iOccupiedSlotsMask &&
			0u == unequipAll.FinalProjection.iPrimarySlotsMask,
			"unequip all retained a primary owner"))
		{
			return false;
		}

		const std::array<Footprint, 2u> overlappingOwners{
			fullbody, head
		};
		const EQUIPMENT_SLOT_PROJECTION committedProjection = projection;
		if (!Require(
			Result::OVERLAPPING_OWNER == Project_EquipmentSlotOwners(
				overlappingOwners, projection) &&
			projection == committedProjection,
			"overlapping preset partially committed its projection"))
		{
			return false;
		}

		const Footprint highBit{
			Slot::HEAD,
			EquipmentSlotMask(Slot::HEAD) |
				(1u << EQUIPMENT_SLOT_COUNT)
		};
		const std::array<Footprint, 1u> highBitOwners{ highBit };
		if (!Require(
			Result::INVALID_OCCUPIED_MASK ==
				Project_EquipmentSlotOwners(highBitOwners, projection) &&
			projection == committedProjection,
			"unsupported occupied bit replaced committed projection"))
		{
			return false;
		}

		const std::array<Footprint, 2u> duplicatePrimaryOwners{
			head, head
		};
		std::array<Footprint, EQUIPMENT_SLOT_COUNT + 1u> tooManyOwners{};
		const std::array<Footprint, 1u> invalidSlotOwner{
			Footprint{ Slot::END, EquipmentSlotMask(Slot::HEAD) }
		};
		const std::array<Footprint, 1u> emptyOwner{
			Footprint{ Slot::HEAD, 0u }
		};
		if (!Require(
			Result::DUPLICATE_PRIMARY_OWNER ==
				Project_EquipmentSlotOwners(duplicatePrimaryOwners, projection) &&
			projection == committedProjection &&
			Result::TOO_MANY_OWNERS ==
				Project_EquipmentSlotOwners(tooManyOwners, projection) &&
			projection == committedProjection &&
			Result::INVALID_SLOT ==
				Project_EquipmentSlotOwners(invalidSlotOwner, projection) &&
			projection == committedProjection &&
			Result::EMPTY_FOOTPRINT ==
				Project_EquipmentSlotOwners(emptyOwner, projection) &&
			projection == committedProjection,
			"invalid owner collection changed the committed projection"))
		{
			return false;
		}

		EQUIPMENT_LOADOUT_TRANSITION untouched{};
		untouched.iRemovedPrimarySlotsMask = EQUIPMENT_SLOT_MASK_ALL;
		const EQUIPMENT_LOADOUT_TRANSITION untouchedBefore = untouched;
		if (!Require(
			Result::ALREADY_DEFAULT == Stage_UnequipEquipmentSlot(
				std::span<const Footprint>{}, Slot::LOWER, untouched) &&
			untouched == untouchedBefore &&
			Result::OVERLAPPING_OWNER == Stage_EquipEquipmentOwner(
				overlappingOwners, lower, untouched) &&
			untouched == untouchedBefore,
			"default/invalid transition mutated caller state"))
		{
			return false;
		}

		return true;
	}
```

같은 anonymous namespace에 visual ID 교체를 실제 production policy로 호출하는 다음 함수를 추가한다.

```cpp
	bool VerifyVisualEquipmentOwnerIdentity()
	{
		using namespace LostArk::Shared;
		using Slot = EQUIPMENT_SLOT_ID;
		using Footprint = EQUIPMENT_OWNER_FOOTPRINT;

		const Footprint fullbody{ Slot::UPPER, 0x1fu };
		const Footprint weapon{ Slot::WEAPON, 0x20u };
		Client::EQUIPMENT_VISUAL_SELECTIONS current{};
		current[static_cast<std::size_t>(Slot::UPPER)] =
			"character.lance_master.mokoko_pleasant.outfit";
		current[static_cast<std::size_t>(Slot::WEAPON)] =
			"character.lance_master.weapon.a";
		const std::array<Client::EQUIPMENT_VISUAL_OWNER_REF, 2u> owners{
			Client::EQUIPMENT_VISUAL_OWNER_REF{
				current[static_cast<std::size_t>(Slot::UPPER)], fullbody },
			Client::EQUIPMENT_VISUAL_OWNER_REF{
				current[static_cast<std::size_t>(Slot::WEAPON)], weapon }
		};
		const Client::EQUIPMENT_VISUAL_OWNER_REF candidate{
			"character.lance_master.weapon.b", weapon
		};

		Client::EQUIPMENT_VISUAL_LOADOUT_STAGE staged{};
		std::string error;
		if (!Require(
			Client::Stage_EquipVisualOwner(
				current, owners, candidate, staged, error) &&
			staged.selections[static_cast<std::size_t>(Slot::UPPER)] ==
				current[static_cast<std::size_t>(Slot::UPPER)] &&
			staged.selections[static_cast<std::size_t>(Slot::WEAPON)] ==
				candidate.visualSetId &&
			EquipmentSlotMask(Slot::WEAPON) ==
				staged.transition.iRemovedPrimarySlotsMask &&
			EquipmentSlotMask(Slot::UPPER) ==
				staged.transition.iPreservedPrimarySlotsMask,
			"weapon visual ID swap changed the fullbody owner"))
		{
			return false;
		}

		Client::EQUIPMENT_AUTHORING_LOADOUTS loadouts{};
		loadouts[0] = current;
		const std::string savedBefore =
			Client::Serialize_EquipmentAuthoringPresets(loadouts);
		const std::string selectedCandidateId(candidate.visualSetId);
		const std::string candidateOnly =
			Client::Serialize_EquipmentAuthoringPresets(loadouts);
		if (!Require(
			!selectedCandidateId.empty() && savedBefore == candidateOnly,
			"candidate row selection changed the serialized preset"))
		{
			return false;
		}
		loadouts[0] = staged.selections;
		const std::string equipped =
			Client::Serialize_EquipmentAuthoringPresets(loadouts);
		if (!Require(
			equipped != savedBefore &&
			std::string::npos != equipped.find(candidate.visualSetId) &&
			std::string::npos == equipped.find(
				"character.lance_master.weapon.a") &&
			std::string::npos != equipped.find(
				"character.lance_master.mokoko_pleasant.outfit"),
			"equipped weapon was not the only serialized owner change"))
		{
			return false;
		}

		Client::EQUIPMENT_VISUAL_LOADOUT_STAGE untouched{};
		untouched.selections[0] = "sentinel";
		const Client::EQUIPMENT_VISUAL_LOADOUT_STAGE before = untouched;
		Client::EQUIPMENT_VISUAL_SELECTIONS invalid = current;
		invalid[static_cast<std::size_t>(Slot::HEAD)] =
			current[static_cast<std::size_t>(Slot::UPPER)];
		if (!Require(
			!Client::Stage_EquipVisualOwner(
				invalid, owners, candidate, untouched, error) &&
			untouched.selections == before.selections &&
			untouched.transition == before.transition,
			"invalid secondary selection partially changed visual stage output"))
		{
			return false;
		}

		return true;
	}
```

같은 파일의 runner 조건은 다음 완전한 블록으로 교체한다.

```cpp
	if (!VerifyMousePressOwnership() || !VerifyIndependentLoopedSound() ||
		!VerifyReplicatedPartyHealth() ||
		!VerifyCompositeEquipmentLoadout() ||
		!VerifyVisualEquipmentOwnerIdentity() ||
		!VerifyPartyTransferNotice())
	{
		return 1;
	}
```

#### G03A-5. visualSetId primary-selection policy 전체 코드

Shared resolver는 opaque item/visual ID를 소유하지 않는다. Tool이 같은 footprint의 weapon A와 B를 실제 문자열로
교체하는 경계를 native test로 실행하기 위해 다음 pure Client policy를 추가한다. 이 파일은 ImGui, JSON,
Character, D3D를 include하지 않는다.

`Client/Public/EquipmentVisualLoadoutPolicy.h` 전체 코드는 다음과 같다.

```cpp
#pragma once

#include "Gameplay/EquipmentLoadoutContract.h"

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace Client
{
	using EQUIPMENT_VISUAL_SELECTIONS = std::array<std::string,
		LostArk::Shared::EQUIPMENT_SLOT_COUNT>;

	struct EQUIPMENT_VISUAL_OWNER_REF final
	{
		std::string_view visualSetId;
		LostArk::Shared::EQUIPMENT_OWNER_FOOTPRINT footprint{};
	};

	struct EQUIPMENT_VISUAL_LOADOUT_STAGE final
	{
		EQUIPMENT_VISUAL_SELECTIONS selections{};
		LostArk::Shared::EQUIPMENT_LOADOUT_TRANSITION transition{};
	};

	bool Is_VisualOwnerAlreadySelected(
		const EQUIPMENT_VISUAL_SELECTIONS& currentSelections,
		const EQUIPMENT_VISUAL_OWNER_REF& candidate) noexcept;

	bool Stage_EquipVisualOwner(
		const EQUIPMENT_VISUAL_SELECTIONS& currentSelections,
		std::span<const EQUIPMENT_VISUAL_OWNER_REF> currentOwners,
		const EQUIPMENT_VISUAL_OWNER_REF& candidate,
		EQUIPMENT_VISUAL_LOADOUT_STAGE& outStage,
		std::string& outError);

	bool Stage_UnequipVisualSlot(
		const EQUIPMENT_VISUAL_SELECTIONS& currentSelections,
		std::span<const EQUIPMENT_VISUAL_OWNER_REF> currentOwners,
		LostArk::Shared::EQUIPMENT_SLOT_ID selectedSlot,
		EQUIPMENT_VISUAL_LOADOUT_STAGE& outStage,
		std::string& outError);

	bool Stage_UnequipAllVisualOwners(
		const EQUIPMENT_VISUAL_SELECTIONS& currentSelections,
		std::span<const EQUIPMENT_VISUAL_OWNER_REF> currentOwners,
		EQUIPMENT_VISUAL_LOADOUT_STAGE& outStage,
		std::string& outError);
}
```

`Client/Private/EquipmentVisualLoadoutPolicy.cpp` 전체 코드는 다음과 같다.

```cpp
#include "EquipmentVisualLoadoutPolicy.h"

#include <utility>

namespace
{
	using Footprint = LostArk::Shared::EQUIPMENT_OWNER_FOOTPRINT;
	using Result = LostArk::Shared::EQUIPMENT_LOADOUT_RESULT;
	using Slot = LostArk::Shared::EQUIPMENT_SLOT_ID;

	bool Build_CurrentFootprints(
		const Client::EQUIPMENT_VISUAL_SELECTIONS& selections,
		const std::span<const Client::EQUIPMENT_VISUAL_OWNER_REF> owners,
		std::array<Footprint,
			LostArk::Shared::EQUIPMENT_SLOT_COUNT>& outFootprints,
		std::size_t& outCount,
		std::string& outError)
	{
		if (owners.size() > LostArk::Shared::EQUIPMENT_SLOT_COUNT)
		{
			outError = "Visual owner count exceeds the stable slot count.";
			return false;
		}

		std::size_t selectedCount = 0u;
		for (const std::string& selection : selections)
		{
			if (!selection.empty())
				++selectedCount;
		}
		if (selectedCount != owners.size())
		{
			outError = "Primary selections and resolved owners differ.";
			return false;
		}

		std::array<bool, LostArk::Shared::EQUIPMENT_SLOT_COUNT>
			seenPrimary{};
		std::array<std::string_view,
			LostArk::Shared::EQUIPMENT_SLOT_COUNT> seenIds{};
		std::array<Footprint,
			LostArk::Shared::EQUIPMENT_SLOT_COUNT> stagedFootprints{};
		std::size_t stagedCount = 0u;

		for (const Client::EQUIPMENT_VISUAL_OWNER_REF& owner : owners)
		{
			if (owner.visualSetId.empty())
			{
				outError = "Resolved visual owner ID is empty.";
				return false;
			}
			const Result validation =
				LostArk::Shared::Validate_EquipmentOwnerFootprint(
					owner.footprint);
			if (Result::SUCCESS != validation)
			{
				outError = std::string("Resolved visual footprint is invalid: ") +
					LostArk::Shared::Get_EquipmentLoadoutResultText(validation);
				return false;
			}

			const std::size_t primary = static_cast<std::size_t>(
				owner.footprint.ePrimarySlot);
			if (seenPrimary[primary] || selections[primary] != owner.visualSetId)
			{
				outError = "Resolved visual owner does not match its primary selection.";
				return false;
			}
			for (std::size_t index = 0u; index < stagedCount; ++index)
			{
				if (seenIds[index] == owner.visualSetId)
				{
					outError = "Resolved visual owner ID is duplicated.";
					return false;
				}
			}
			seenPrimary[primary] = true;
			seenIds[stagedCount] = owner.visualSetId;
			stagedFootprints[stagedCount++] = owner.footprint;
		}

		LostArk::Shared::EQUIPMENT_SLOT_PROJECTION projection{};
		const Result projectionResult =
			LostArk::Shared::Project_EquipmentSlotOwners(
				std::span<const Footprint>(
					stagedFootprints.data(), stagedCount), projection);
		if (Result::SUCCESS != projectionResult)
		{
			outError = std::string("Resolved visual owners overlap: ") +
				LostArk::Shared::Get_EquipmentLoadoutResultText(
					projectionResult);
			return false;
		}

		outFootprints = stagedFootprints;
		outCount = stagedCount;
		return true;
	}

	void Clear_RemovedPrimarySelections(
		Client::EQUIPMENT_VISUAL_SELECTIONS& selections,
		const LostArk::Shared::EQUIPMENT_SLOT_MASK removedMask)
	{
		for (std::size_t index = 0u;
			index < LostArk::Shared::EQUIPMENT_SLOT_COUNT; ++index)
		{
			const Slot slot = static_cast<Slot>(index);
			if (0u != (removedMask & LostArk::Shared::EquipmentSlotMask(slot)))
				selections[index].clear();
		}
	}
}

bool Client::Is_VisualOwnerAlreadySelected(
	const EQUIPMENT_VISUAL_SELECTIONS& currentSelections,
	const EQUIPMENT_VISUAL_OWNER_REF& candidate) noexcept
{
	if (candidate.visualSetId.empty() ||
		!LostArk::Shared::Is_ValidEquipmentSlot(
			candidate.footprint.ePrimarySlot))
	{
		return false;
	}
	return currentSelections[static_cast<std::size_t>(
		candidate.footprint.ePrimarySlot)] == candidate.visualSetId;
}

bool Client::Stage_EquipVisualOwner(
	const EQUIPMENT_VISUAL_SELECTIONS& currentSelections,
	const std::span<const EQUIPMENT_VISUAL_OWNER_REF> currentOwners,
	const EQUIPMENT_VISUAL_OWNER_REF& candidate,
	EQUIPMENT_VISUAL_LOADOUT_STAGE& outStage,
	std::string& outError)
{
	if (candidate.visualSetId.empty())
	{
		outError = "Candidate visual set ID is empty.";
		return false;
	}
	const Result candidateValidation =
		LostArk::Shared::Validate_EquipmentOwnerFootprint(
			candidate.footprint);
	if (Result::SUCCESS != candidateValidation)
	{
		outError = std::string("Candidate visual footprint is invalid: ") +
			LostArk::Shared::Get_EquipmentLoadoutResultText(
				candidateValidation);
		return false;
	}

	std::array<Footprint,
		LostArk::Shared::EQUIPMENT_SLOT_COUNT> footprints{};
	std::size_t footprintCount = 0u;
	if (!Build_CurrentFootprints(currentSelections, currentOwners,
		footprints, footprintCount, outError))
	{
		return false;
	}
	for (const EQUIPMENT_VISUAL_OWNER_REF& owner : currentOwners)
	{
		if (owner.visualSetId == candidate.visualSetId &&
			owner.footprint != candidate.footprint)
		{
			outError = "Candidate visual set ID resolves to a different footprint.";
			return false;
		}
	}

	LostArk::Shared::EQUIPMENT_LOADOUT_TRANSITION transition{};
	const Result result = LostArk::Shared::Stage_EquipEquipmentOwner(
		std::span<const Footprint>(footprints.data(), footprintCount),
		candidate.footprint, transition);
	if (Result::SUCCESS != result)
	{
		outError = std::string("Visual equip transition failed: ") +
			LostArk::Shared::Get_EquipmentLoadoutResultText(result);
		return false;
	}

	EQUIPMENT_VISUAL_LOADOUT_STAGE staged{};
	staged.selections = currentSelections;
	Clear_RemovedPrimarySelections(
		staged.selections, transition.iRemovedPrimarySlotsMask);
	staged.selections[static_cast<std::size_t>(
		candidate.footprint.ePrimarySlot)] = candidate.visualSetId;
	staged.transition = transition;
	outStage = std::move(staged);
	outError.clear();
	return true;
}

bool Client::Stage_UnequipVisualSlot(
	const EQUIPMENT_VISUAL_SELECTIONS& currentSelections,
	const std::span<const EQUIPMENT_VISUAL_OWNER_REF> currentOwners,
	const Slot selectedSlot,
	EQUIPMENT_VISUAL_LOADOUT_STAGE& outStage,
	std::string& outError)
{
	std::array<Footprint,
		LostArk::Shared::EQUIPMENT_SLOT_COUNT> footprints{};
	std::size_t footprintCount = 0u;
	if (!Build_CurrentFootprints(currentSelections, currentOwners,
		footprints, footprintCount, outError))
	{
		return false;
	}

	LostArk::Shared::EQUIPMENT_LOADOUT_TRANSITION transition{};
	const Result result = LostArk::Shared::Stage_UnequipEquipmentSlot(
		std::span<const Footprint>(footprints.data(), footprintCount),
		selectedSlot, transition);
	if (Result::SUCCESS != result)
	{
		outError = std::string("Visual unequip transition failed: ") +
			LostArk::Shared::Get_EquipmentLoadoutResultText(result);
		return false;
	}

	EQUIPMENT_VISUAL_LOADOUT_STAGE staged{};
	staged.selections = currentSelections;
	Clear_RemovedPrimarySelections(
		staged.selections, transition.iRemovedPrimarySlotsMask);
	staged.transition = transition;
	outStage = std::move(staged);
	outError.clear();
	return true;
}

bool Client::Stage_UnequipAllVisualOwners(
	const EQUIPMENT_VISUAL_SELECTIONS& currentSelections,
	const std::span<const EQUIPMENT_VISUAL_OWNER_REF> currentOwners,
	EQUIPMENT_VISUAL_LOADOUT_STAGE& outStage,
	std::string& outError)
{
	std::array<Footprint,
		LostArk::Shared::EQUIPMENT_SLOT_COUNT> footprints{};
	std::size_t footprintCount = 0u;
	if (!Build_CurrentFootprints(currentSelections, currentOwners,
		footprints, footprintCount, outError))
	{
		return false;
	}

	LostArk::Shared::EQUIPMENT_LOADOUT_TRANSITION transition{};
	const Result result = LostArk::Shared::Stage_UnequipAllEquipment(
		std::span<const Footprint>(footprints.data(), footprintCount),
		transition);
	if (Result::SUCCESS != result)
	{
		outError = std::string("Visual unequip-all transition failed: ") +
			LostArk::Shared::Get_EquipmentLoadoutResultText(result);
		return false;
	}

	EQUIPMENT_VISUAL_LOADOUT_STAGE staged{};
	staged.selections = currentSelections;
	Clear_RemovedPrimarySelections(
		staged.selections, transition.iRemovedPrimarySlotsMask);
	staged.transition = transition;
	outStage = std::move(staged);
outError.clear();
	return true;
}
```

candidate row와 실제 저장 입력을 native test에서 분리하기 위해 preset serialization도 pure codec으로 뺀다.
`Client/Public/EquipmentAuthoringPresetCodec.h` 전체 코드는 다음과 같다.

```cpp
#pragma once

#include "EquipmentVisualLoadoutPolicy.h"

#include <array>
#include <cstddef>
#include <string>

namespace Client
{
	inline constexpr std::size_t EQUIPMENT_AUTHORING_CLASS_COUNT = 6u;
	using EQUIPMENT_AUTHORING_LOADOUTS = std::array<
		EQUIPMENT_VISUAL_SELECTIONS, EQUIPMENT_AUTHORING_CLASS_COUNT>;

	std::string Serialize_EquipmentAuthoringPresets(
		const EQUIPMENT_AUTHORING_LOADOUTS& loadouts);
}
```

`Client/Private/EquipmentAuthoringPresetCodec.cpp` 전체 코드는 다음과 같다.

```cpp
#include "EquipmentAuthoringPresetCodec.h"

#include "DataJson.h"

#include <array>
#include <string_view>

namespace
{
	constexpr std::array<std::string_view,
		Client::EQUIPMENT_AUTHORING_CLASS_COUNT> CLASS_IDS{
		"LANCE_MASTER", "GUNSLINGER", "SLAYER",
		"ARTIST", "DIMENSIONMASTER", "WARLORD"
	};
	constexpr std::array<std::string_view,
		LostArk::Shared::EQUIPMENT_SLOT_COUNT> SLOT_IDS{
		"HEAD", "SHOULDER", "UPPER", "LOWER", "HANDS", "WEAPON"
	};

	void Append_Quoted(std::string& output, const std::string_view value)
	{
		output.push_back('"');
		output += Client::CDataJson::Escape(value);
		output.push_back('"');
	}
}

std::string Client::Serialize_EquipmentAuthoringPresets(
	const EQUIPMENT_AUTHORING_LOADOUTS& loadouts)
{
	std::string output;
	output.reserve(2048u);
	output += "{\n";
	output += "  \"schema\": \"lostark.equipment-loadout-presets\",\n";
	output += "  \"formatVersion\": 1,\n";
	output += "  \"authoringOnly\": true,\n";
	output += "  \"slots\": [\n";
	for (std::size_t slot = 0u; slot < SLOT_IDS.size(); ++slot)
	{
		output += "    ";
		Append_Quoted(output, SLOT_IDS[slot]);
		output += slot + 1u == SLOT_IDS.size() ? "\n" : ",\n";
	}
	output += "  ],\n";
	output += "  \"classPresets\": [\n";
	for (std::size_t classIndex = 0u;
		classIndex < CLASS_IDS.size(); ++classIndex)
	{
		output += "    {\n";
		output += "      \"classId\": ";
		Append_Quoted(output, CLASS_IDS[classIndex]);
		output += ",\n";
		output += "      \"slotSelections\": {\n";
		for (std::size_t slot = 0u; slot < SLOT_IDS.size(); ++slot)
		{
			output += "        ";
			Append_Quoted(output, SLOT_IDS[slot]);
			output += ": ";
			const std::string& selection = loadouts[classIndex][slot];
			if (selection.empty())
				output += "null";
			else
				Append_Quoted(output, selection);
			output += slot + 1u == SLOT_IDS.size() ? "\n" : ",\n";
		}
		output += "      }\n";
		output += classIndex + 1u == CLASS_IDS.size() ?
			"    }\n" : "    },\n";
	}
	output += "  ]\n";
	output += "}\n";
	return output;
}
```

Tool은 기존 inline writer를 삭제하고 `Save_Presets`에서 오직 `Serialize_EquipmentAuthoringPresets(m_Loadouts)`를
호출한다. codec은 candidate ID를 입력으로 받지 않으므로 row 선택만으로 저장 문자열이 바뀔 수 없다. temporary
write 뒤 기존 strict parser로 재parse/validate하는 단계는 유지한다.

Client project에는 H/CPP를 각각 `Public`, `Private` filter에 등록한다. Character Select harness는 위 CPP를 자기
`ClCompile` 항목으로 등록해 Client executable을 link하지 않고 같은 production function을 실행한다.

```xml
<!-- Client/Default/Client.vcxproj -->
<ClInclude Include="..\Public\EquipmentVisualLoadoutPolicy.h" />
<ClCompile Include="..\Private\EquipmentVisualLoadoutPolicy.cpp" />
<ClInclude Include="..\Public\EquipmentAuthoringPresetCodec.h" />
<ClCompile Include="..\Private\EquipmentAuthoringPresetCodec.cpp" />

<!-- Client/Default/Client.vcxproj.filters -->
<ClInclude Include="..\Public\EquipmentVisualLoadoutPolicy.h">
  <Filter>02.GameObjects\00. Character</Filter>
</ClInclude>
<ClCompile Include="..\Private\EquipmentVisualLoadoutPolicy.cpp">
  <Filter>02.GameObjects\00. Character</Filter>
</ClCompile>
<ClInclude Include="..\Public\EquipmentAuthoringPresetCodec.h">
  <Filter>03. Tools\03. UI</Filter>
</ClInclude>
<ClCompile Include="..\Private\EquipmentAuthoringPresetCodec.cpp">
  <Filter>03. Tools\03. UI</Filter>
</ClCompile>

<!-- Tools/CharacterSelectIsolationHarness/Default/CharacterSelectIsolationHarness.vcxproj -->
<ClCompile Include="..\..\..\Client\Private\EquipmentVisualLoadoutPolicy.cpp" />
<ClCompile Include="..\..\..\Client\Private\EquipmentAuthoringPresetCodec.cpp" />
<ClCompile Include="..\..\..\Client\Private\DataJson.cpp" />

<!-- CharacterSelectIsolationHarness.vcxproj.filters의 Private ItemGroup -->
<ClCompile Include="..\..\..\Client\Private\EquipmentVisualLoadoutPolicy.cpp">
  <Filter>Private</Filter>
</ClCompile>
<ClCompile Include="..\..\..\Client\Private\EquipmentAuthoringPresetCodec.cpp">
  <Filter>Private</Filter>
</ClCompile>
<ClCompile Include="..\..\..\Client\Private\DataJson.cpp">
  <Filter>Private</Filter>
</ClCompile>
```

#### G03A-6. Client H 계약 전체 코드

`Client/Public/EquipmentPresentationCatalog.h`는 다음 전체 코드로 교체한다.

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "EquipmentVisualLoadoutPolicy.h"
#include "Gameplay/EquipmentLoadoutContract.h"
#include "Network/PacketMessages.h"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

using EQUIPMENT_SLOT_ID = LostArk::Shared::EQUIPMENT_SLOT_ID;
inline constexpr size_t EQUIPMENT_SLOT_COUNT =
	LostArk::Shared::EQUIPMENT_SLOT_COUNT;

enum class EQUIPMENT_ATTACHMENT_MODE
{
	SKINNED,
	SOCKETED,
	END
};

enum class EQUIPMENT_BODY_COVERAGE_MODE
{
	INHERIT_BASELINE,
	EXPLICIT,
	NONE,
	END
};

struct EQUIPMENT_BODY_COVERAGE final
{
	EQUIPMENT_BODY_COVERAGE_MODE mode =
		EQUIPMENT_BODY_COVERAGE_MODE::END;
	std::vector<std::string> materialIds;
};

struct EQUIPMENT_PRESENTATION_PART final
{
	std::string partId;
	std::string partRole;
	EQUIPMENT_ATTACHMENT_MODE attachmentMode =
		EQUIPMENT_ATTACHMENT_MODE::END;
	std::string modelAssetId;
	std::string socketBoneId;
	f32_t socketYawDegrees = 0.f;
	LostArk::Shared::PLAYER_STANCE_ID requiredStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	uint32_t hiddenMeshMask = 0u;
};

struct EQUIPMENT_VISUAL_SET final
{
	std::string visualSetId;
	std::string itemId;
	LostArk::Shared::CHARACTER_CLASS_ID classId =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string categoryId;
	std::string catalogStatus;
	LostArk::Shared::EQUIPMENT_OWNER_FOOTPRINT footprint{};
	EQUIPMENT_BODY_COVERAGE bodyCoverage{};
	std::vector<EQUIPMENT_PRESENTATION_PART> parts;
};

struct EQUIPMENT_RESOLVED_VISUAL_LOADOUT final
{
	LostArk::Shared::CHARACTER_CLASS_ID classId =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::array<const EQUIPMENT_VISUAL_SET*,
		EQUIPMENT_SLOT_COUNT> ownerSetsByPrimary{};
	LostArk::Shared::EQUIPMENT_SLOT_PROJECTION slotProjection{};
};

class CEquipmentPresentationCatalog final
{
public:
	bool_t Load(std::string& outError);

	const std::vector<EQUIPMENT_VISUAL_SET>& Get_Sets() const
	{
		return m_Sets;
	}

	const EQUIPMENT_VISUAL_SET* Find_Set(
		std::string_view visualSetId) const;
	std::vector<const EQUIPMENT_VISUAL_SET*>
		Find_ByClassAndOccupiedSlot(
			LostArk::Shared::CHARACTER_CLASS_ID classId,
			EQUIPMENT_SLOT_ID slot) const;
	bool_t Resolve_Loadout(
		LostArk::Shared::CHARACTER_CLASS_ID classId,
		const EQUIPMENT_VISUAL_SELECTIONS& selections,
		EQUIPMENT_RESOLVED_VISUAL_LOADOUT& outResolved,
		std::string& outError) const;

private:
	std::vector<EQUIPMENT_VISUAL_SET> m_Sets;
};

const char_t* To_String(EQUIPMENT_SLOT_ID slot);

NS_END
```

`Client/Public/EquipmentAuthoringTool.h`는 다음 전체 코드로 교체한다.

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "EquipmentAuthoringPresetCodec.h"
#include "EquipmentPresentationCatalog.h"
#include "EquipmentPresentationService.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CCharacterPreviewPanel;
class CCharacter;

class CEquipmentAuthoringTool final
{
private:
	static constexpr size_t CLASS_COUNT = 6u;
	static constexpr size_t SLOT_COUNT = EQUIPMENT_SLOT_COUNT;
	using LOADOUT = EQUIPMENT_VISUAL_SELECTIONS;
	using CLASS_LOADOUTS = EQUIPMENT_AUTHORING_LOADOUTS;

	struct EQUIPMENT_LOADOUT_IMPACT final
	{
		LOADOUT stagedLoadout{};
		LostArk::Shared::EQUIPMENT_LOADOUT_TRANSITION transition{};
		std::vector<std::string> removedOwnerIds;
		std::vector<std::string> preservedOwnerIds;
	};

public:
	CEquipmentAuthoringTool(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::shared_ptr<CCharacterPreviewPanel> pPreviewPanel);
	~CEquipmentAuthoringTool();

	void Render(bool_t isInputOwner);
	void On_LevelChanged();

private:
	void Render_ClassSelector();
	void Render_SlotList();
	void Render_VisualSetList();
	void Render_SelectedSetDiagnostics() const;
	void Render_Actions();

	bool_t Select_Class(size_t classIndex);
	bool_t Apply_CurrentLoadout();
	bool_t Commit_CurrentLoadout(
		const LOADOUT& stagedLoadout,
		const std::string& successStatus);
	bool_t Equip_SelectedSet();
	bool_t Unequip_SelectedSlot();
	bool_t Unequip_All();
	bool_t Stage_EquipImpact(
		const EQUIPMENT_VISUAL_SET& candidate,
		EQUIPMENT_LOADOUT_IMPACT& outImpact,
		std::string& outError) const;
	bool_t Stage_UnequipImpact(
		EQUIPMENT_SLOT_ID requestedSlot,
		EQUIPMENT_LOADOUT_IMPACT& outImpact,
		std::string& outError) const;

	bool_t Load_InitialDocuments();
	bool_t Reload_Catalog();
	bool_t Reload_Presets();
	bool_t Save_Presets();
	bool_t Load_PresetsFromPath(
		const std::filesystem::path& path,
		const CEquipmentPresentationCatalog& validationCatalog,
		CLASS_LOADOUTS& outLoadouts,
		std::string& outError) const;
	bool_t Validate_Loadouts(
		const CLASS_LOADOUTS& loadouts,
		const CEquipmentPresentationCatalog& catalog,
		std::string& outError) const;
	bool_t Validate_Loadout(
		size_t classIndex,
		const LOADOUT& loadout,
		const CEquipmentPresentationCatalog& catalog,
		std::string& outError) const;
	std::filesystem::path Preset_Path() const;
	std::filesystem::path Make_UniqueTemporaryPath(
		const std::filesystem::path& destination) const;

	void Synchronize_ClassFromPreviewTarget();
	bool_t Resolve_ActivePlayablePreview(
		size_t& outClassIndex,
		std::shared_ptr<CCharacter>& outCharacter) const;
	void Recompute_Dirty();
	const EQUIPMENT_VISUAL_SET* Selected_Set() const;

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	std::shared_ptr<CCharacterPreviewPanel> m_pPreviewPanel;
	CEquipmentPresentationCatalog m_Catalog;
	CEquipmentPresentationService m_PresentationService;
	CLASS_LOADOUTS m_Loadouts{};
	CLASS_LOADOUTS m_SavedLoadouts{};

	size_t m_iSelectedClass = 0u;
	size_t m_iSelectedSlot = 0u;
	std::string m_strSelectedVisualSetId;
	std::string m_strStatus =
		"Equipment catalog and preview-only presets have not been loaded.";
	uint64_t m_iObservedPreviewGeneration = 0u;
	uint64_t m_iLastPreviewApplyAttemptGeneration = 0u;
	bool_t m_bInitialDocumentsAttempted = false;
	bool_t m_bInitialDocumentsLoaded = false;
	bool_t m_bWasInputOwner = false;
	bool_t m_bHasPreviewApplyAttemptGeneration = false;
	bool_t m_bPreviewApplyFailed = false;
	bool_t m_bCatalogLoaded = false;
	bool_t m_bPresetsLoaded = false;
	bool_t m_bDirty = false;
	bool_t m_bPreviewInputEnabled = false;
	bool_t m_bShowBaselineReferences = false;
};

NS_END
```

`Client/Public/EquipmentPresentationService.h`는 다음 전체 코드로 교체한다.

```cpp
#pragma once

#include "Client_Defines.h"
#include "EquipmentPresentationCatalog.h"

#include <string>
#include <unordered_set>

NS_BEGIN(Client)

class CCharacter;

class CEquipmentPresentationService final
{
public:
	CEquipmentPresentationService(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

	bool_t Apply_Preview(
		CCharacter& character,
		const EQUIPMENT_RESOLVED_VISUAL_LOADOUT& resolved,
		std::string& outError);
	void On_LevelChanged();

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	uint32_t m_iPrototypeLevelIndex = ETOUI(LEVEL::END);
	std::unordered_set<std::string> m_AdmittedModelAssetIds;
};

NS_END
```

#### G03A-7. 기존 Client CPP 함수별 교체 계약

아래 함수는 선언만 추가하고 기존 구현을 추측하게 두지 않는다. 각 함수는 local staged 값만 만들고 표의 마지막
commit 지점 전에는 member/live Character를 바꾸지 않는다.

| 파일·함수 | 완전한 처리 순서 | 성공 commit / 실패 불변식 |
|---|---|---|
| `EquipmentPresentationCatalog.cpp::Load` | root exact fields `schema/formatVersion/slotIds/visualSets`와 formatVersion 2 확인 → slotIds exact order 확인 → 각 9-field set에서 nullable itemId, class/category/status/primary/raw occupied/bodyCoverage/parts parse → raw occupied 배열을 dedupe하기 전에 `Build_EquipmentSlotMask` → category/body mode/part 8-field exact validation → unique visualSetId와 non-null `(itemId,classId)` 검사 → staged vector 전체 교차 검증 | 마지막에만 `m_Sets = move(staged)`; parse/schema/semantic 실패 시 기존 generation 유지 |
| `Find_ByClassAndOccupiedSlot` | class가 같고 `footprint.iOccupiedSlotsMask & EquipmentSlotMask(slot)`인 set만 수집 → visualSetId stable sort | read-only; invalid slot은 빈 목록 |
| `Resolve_Loadout` | selections의 nonempty entry마다 exact set lookup/class match/entry index==set primary 확인 → owner set pointer와 footprint 수집 → `Project_EquipmentSlotOwners` → secondary entry가 nonempty이거나 set ID가 중복되면 실패 | local `EQUIPMENT_RESOLVED_VISUAL_LOADOUT` 완성 뒤에만 out 교체; 실패 시 out 불변 |
| `EquipmentAuthoringTool.cpp::Load_PresetsFromPath` | JSON exact parse → 전달받은 `validationCatalog`로 여섯 class primary-only selections를 validate/resolve | member `m_Catalog`를 참조하지 않음; startup은 staged catalog, normal reload/save reparse는 committed catalog를 명시 전달; 실패 시 out 불변 |
| `EquipmentAuthoringTool.cpp::Load_InitialDocuments` | catalog를 local object로 parse → preset을 `Load_PresetsFromPath(path, stagedCatalog, stagedLoadouts, outError)`로 local loadouts에 parse/validate → 둘이 모두 성공한 경우에만 catalog, draft, saved baseline을 한 번에 document-only commit | live Character를 호출하지 않음; input owner가 없어도 문서를 읽을 수 있음; 하나라도 실패하면 기존 document/draft/saved/live visual 유지 |
| `Render`의 최초 로드·preview owner 전이 | 첫 Render에서 input owner와 무관하게 `Load_InitialDocuments`를 정확히 한 번 시도하고 attempted를 기록 → `isInputOwner: false→true` 또는 current `previewGeneration != lastAttemptGeneration`인 프레임에 current draft를 live preview에 한 번 적용 → 시도한 generation은 성공/실패 모두 별도로 기록 | 성공 때만 observed generation 갱신; 같은 실패 generation은 자동 재시도하지 않고 `Retry Preview`를 유지; 새 generation·owner 재획득·명시 retry는 다시 시도하며 document/draft/saved/dirty는 실패 시 보존 |
| `EquipmentAuthoringTool.cpp::Stage_EquipImpact` | current loadout을 catalog resolve → primary owner pointer로 `EQUIPMENT_VISUAL_OWNER_REF[]` 생성 → `Stage_EquipVisualOwner` → staged selections를 catalog로 재-resolve → transition remove/preserve primary mask를 stable visualSetId 목록으로 변환 → default/body diagnostics 계산 | 완성한 `EQUIPMENT_LOADOUT_IMPACT`만 out에 move; Tool member 불변 |
| `Stage_UnequipImpact` | 같은 current resolve/ref 생성 → `Stage_UnequipVisualSlot(selectedSlot)` → staged selections 재-resolve → removed/preserved/restore diagnostics 생성 | default slot은 `ALREADY_DEFAULT` status이고 out/member 불변 |
| `Equip_SelectedSet` / `Unequip_SelectedSlot` / `Unequip_All` | 위 stage 함수만 호출 → staged loadout full validation → `Commit_CurrentLoadout` | 직접 selection 문자열을 clear/set하지 않음 |
| `Commit_CurrentLoadout` | active playable preview와 selected class 일치 확인 → staged loadout catalog resolve → `PresentationService.Apply_Preview` → 성공 뒤 selected class `m_Loadouts` 교체 → `Recompute_Dirty` → status | apply 실패 시 loadout/saved/dirty와 live visual 유지; status만 exact failure reason으로 갱신 가능 |
| `Render_SlotList` | resolved projection을 한 번 만들고 DEFAULT/PRIMARY/COVERED를 표시 | 렌더 중 state mutation 없음 |
| `Render_VisualSetList` / `Selected_Set` | selected slot을 occupied하는 class set만 조회 → baseline toggle gate → primary와 footprint 표시 | toggle OFF에서 hidden baseline selection 즉시 clear |
| `Render_SelectedSetDiagnostics` | candidate가 있으면 `Stage_EquipImpact`, 없고 occupied slot이면 `Stage_UnequipImpact`를 read-only 호출 → owner/default/body/save delta 출력 | stage 실패 이유를 표시하고 성공 impact는 표시하지 않음 |
| `Reload_Catalog` | 새 catalog object에 Load → 여섯 class current draft와 saved baseline을 모두 새 catalog로 validate/resolve → active playable selected class가 있으면 새 resolved preview를 먼저 apply → 성공 뒤 `m_Catalog = move(stagedCatalog)` | `m_bPresetsLoaded` 여부로 visual-first 적용을 건너뛰지 않음; saved ID 하나라도 orphan이면 전체 reload 실패, catalog/draft/saved/dirty/live visual 유지 |
| `Reload_Presets` | `Load_PresetsFromPath(path, m_Catalog, stagedLoadouts, outError)`로 local loadouts parse/validate → active preview 먼저 apply → 성공 뒤 `m_Loadouts`와 `m_SavedLoadouts`를 같은 staged 값으로 교체 | 실패 시 두 loadout/dirty/live visual 유지 |
| `Save_Presets` | 오직 `m_Loadouts` primary 문자열로 temporary JSON 생성 → `Load_PresetsFromPath(tempPath, m_Catalog, reparsed, outError)`로 재parse → byte 의미상 `reparsed == m_Loadouts` 확인 → `MoveFileEx(REPLACE_EXISTING|WRITE_THROUGH)` → 성공 뒤 saved baseline 갱신 | candidate ID와 derived secondary는 writer 입력이 아님; write/parse/replace 실패 시 destination/saved/dirty 유지 |
| `On_LevelChanged` | presentation service의 level-local prototype admission/cache만 해제 → observed generation, last attempted generation 유효 flag, `m_bWasInputOwner`, preview apply failure만 초기화 | 이미 검증된 catalog/draft/saved baseline과 initial-document 상태는 유지; 다음 playable preview owner 획득 때 같은 draft를 다시 적용 |
| `EquipmentPresentationService.cpp::Apply_Preview` | resolved primary owner set을 순회해 model admission batch, preview part desc와 set별 body coverage desc를 전부 stage → footprint/projection과 desc mask 일치 확인 → Character를 정확히 한 번 호출 | prototype registry는 additive cache일 수 있으나 Character part/body/default state는 한 call에서만 commit; 별도 Reset 함수 없음 |

`EquipmentPresentationCatalog.cpp`의 v2 part exact fields는
`partId/partRole/attachmentMode/modelAssetId/socketBoneId/socketYawDegrees/requiredStance/hiddenMeshMask` 여덟 개다.
`SOCKETED`는 nonempty socket, `SKINNED`는 null socket을 요구하고, stance는 `null` 또는 class가 지원하는 한 stable
stance다. `materialProfileId`, `visibleStances`, `SOCKET` 별칭은 허용하지 않는다.

Equipment Tool은 다음 한 흐름만 사용한다.

```text
현재 primary selections를 catalog footprint로 resolve
-> 선택 slot을 점유하는 모든 class set을 후보 목록에 표시
-> candidate footprint와 현재 projection으로 transition stage
-> 제거될 visualSetId, 점유할 slot, 유지될 owner를 diagnostics에 표시
-> Equip Selected에서 staged primary selections를 다시 full validate
-> CEquipmentPresentationService가 모든 part/body state를 stage
-> preview commit 성공 뒤 m_Loadouts와 dirty/status commit
```

최초 문서 로드와 live preview 적용은 같은 one-shot flag로 묶지 않는다. 현재 구현은 첫 `Render`에서
`m_bLoadAttempted = true`를 먼저 세우기 때문에, 그 순간 Tool이 input owner가 아니거나 playable preview가 아직
없거나 apply가 실패하면 이후 owner를 얻어도 자동 적용 기회가 사라진다. 새 상태 전이는 다음으로 고정한다.

```text
첫 Render
  -> m_bInitialDocumentsAttempted가 false면 catalog + preset을 local stage
  -> preset parser에는 member catalog가 아니라 staged catalog를 validationCatalog로 전달
  -> 둘 다 성공: documents/draft/saved만 commit, m_bInitialDocumentsLoaded = true
  -> 실패: 기존 상태 유지, exact error와 Retry Documents 노출

input owner false -> true 또는 preview generation 변경
  -> documents가 loaded일 때만 current selected-class draft를 live preview에 1회 apply
  -> 시도 직전에 current generation을 lastAttemptGeneration으로 기록
  -> 성공: observed generation도 갱신, m_bPreviewApplyFailed = false
  -> 실패: observed generation은 적용 완료로 기록하지 않고 m_bPreviewApplyFailed = true,
           Retry Preview 노출

Retry Documents
  -> 같은 two-document stage를 다시 수행
  -> 성공했고 현재 input owner이면 이어서 current draft apply

Retry Preview
  -> document를 다시 읽지 않고 현재 검증된 catalog + draft만 재적용
```

`Retry Documents`와 실패 status는 catalog-dependent widget disable scope 바깥에 둔다. 따라서 최초 JSON 오류가
있어도 Reload 계열 button까지 함께 비활성화되어 복구할 수 없는 UI가 되지 않는다. 반대로 `Equip Selected`,
`Unequip Slot`, `Unequip All`, `Save Presets`는 documents loaded와 input owner가 모두 true일 때만 활성화한다.
preview apply 실패는 자동 매 프레임 재시도하지 않는다. 실패 원인을 status에 보존하고 명시적 `Retry Preview`,
`currentGeneration != lastAttemptGeneration` 또는 owner 재획득 때만 다시 시도해 clone/I/O 오류가 매 프레임
반복되지 않게 한다. `m_iObservedPreviewGeneration`은 마지막 성공 generation,
`m_iLastPreviewApplyAttemptGeneration + m_bHasPreviewApplyAttemptGeneration`은 성공 여부와 무관한 마지막 시도
generation이므로 둘을 합치지 않는다. generation 값 0도 유효할 수 있어 별도 bool 없이 0을 sentinel로 쓰지 않는다.

`Render_SlotList`는 각 slot을 `<default>`, `[OWNER] visualSetId`,
`[COVERED BY UPPER] visualSetId` 중 하나로 표시한다. `Render_VisualSetList`와 `Selected_Set`은 primary slot이
같은 set만 찾는 현재 경로를 `selected slot을 occupiedSlots에 포함하는 set` 검색으로 바꾸고 각 행에 primary
slot을 함께 표시한다. 따라서 `HEAD`나 `LOWER`를 눌러도 모코코 fullbody를 검사·장착할 수 있지만 저장은
계속 `UPPER` primary 한 곳에만 남는다.

`Render_SelectedSetDiagnostics`에는 선택만 해도 다음 impact를 계산해 표시한다.

```text
Stored owner: UPPER
Will occupy: HEAD, SHOULDER, UPPER, LOWER, HANDS
Will remove: 현재 겹치는 distinct owner 목록
Will preserve: WEAPON을 포함한 비충돌 owner 목록
Default parts hidden: 새 footprint가 가릴 기본 part tag/slot
Default parts restored: 제거되는 owner 때문에 다시 보일 기본 part tag/slot
Body coverage: mode와 inherited slot 또는 explicit stable material ID
Result: staged primary-only loadout
```

catalog/coverage/clone stage가 실패하면 같은 diagnostics panel에 typed 단계와 exact 이유를 표시하고 위 impact를
성공 결과처럼 남기지 않는다. 이 표시는 logical owner 전이와 실제 default/body 외형 전이가 한 gate라는 사실을
Tool에서 사용자가 직접 확인하기 위한 것이다.

candidate의 visualSetId가 이미 그 primary의 current selection과 같으면
`Is_VisualOwnerAlreadySelected`가 true를 반환하고 transition을 remove+add로 표시하지 않는다.
diagnostics는 `Already equipped: no visual, draft, or save delta`를 표시하고 `Equip Selected`를 disable한다.
`Equip_SelectedSet`도 같은 guard를 다시 검사해 stale UI event를 no-op 처리한다. 이 분기는 footprint가 같은 다른
weapon B에는 적용되지 않으므로 A→B는 정상 교체된다.

`READY_ALTERNATIVE`는 기본 후보로 표시하고, 현재 외형과 같아 변화가 없을 수 있는 `BASELINE_PART`와
`BASELINE_WEAPON`은 `Show baseline/reference` 옵션을 켰을 때만 노출한다. 이것은 admission 상태를 숨기는
필터일 뿐 resolver 규칙을 바꾸지 않는다. toggle을 끄는 순간 현재 선택이 baseline/reference면 선택 ID를
비우고, `Selected_Set()`도 toggle이 꺼진 상태에서 baseline/reference ID를 반환하지 않는다. 숨겨진 stale
candidate를 `Equip Selected`로 적용할 수 없게 한다.

button 아래에는 다음 문구를 고정해 candidate 선택, 장착, 저장을 구분한다.

```text
Selecting a row only shows its impact. Equip Selected applies it to the live preview.
Save Presets persists only equipped primary owners.
```

Tool은 `m_Loadouts`와 마지막으로 파일에 확정된 `m_SavedLoadouts`를 별도로 소유한다. preset 최초 load와
`Reload Presets` 성공 때 둘을 같은 staged 값으로 commit하고, `Save Presets`의 temporary-file write와 atomic
replace가 성공한 뒤에만 `m_SavedLoadouts = m_Loadouts`로 갱신한다. 장착/해제 뒤 dirty는 수동 true가 아니라
`m_Loadouts != m_SavedLoadouts`로 다시 계산하므로 원래 저장 상태로 되돌리면 false가 된다. Save diagnostics는
absolute preset path, class별 `saved owner -> draft owner` primary delta와 `secondary slots are derived and not
stored`를 표시한다. 후보 행 선택만으로는 두 loadout 모두 바뀌지 않는다.

`Equip_SelectedSet`은 candidate와 겹치는 owner primary만 clear한다. 모코코 fullbody `0x1F`와 weapon
`0x20`은 겹치지 않으므로 모코코를 입은 상태에서 무기 교체 시 apparel selection은 그대로다. 반대로
`LOWER` item을 장착하면 모코코 outfit owner인 `UPPER` 한 건 전체가 제거되고 weapon은 유지된다.
`Unequip_SelectedSlot`도 기존 선형 탐색 대신 staged projection의 owner primary를 사용한다.

`Unequip All`은 모든 primary selection을 제거해 Character default로 돌아가는 유일한 초기화 명령으로 둔다.
동일 결과를 만드는 `Reset Character Default` button은 제거하고, `Reload Presets`는 마지막으로 저장한 draft를
복원한다. 어느 명령도 baked avatar를 다시 생성하지 않는다.

별도 reset call chain도 함께 제거한다. `EquipmentAuthoringTool.h/.cpp`의 `Reset_CharacterDefault`,
`EquipmentPresentationService.h/.cpp`의 `Reset_Preview`, `Character.h/.cpp`의 `Reset_EquipmentPreview`를 삭제하고,
빈 resolved loadout도 일반 `Apply_Preview -> Apply_EquipmentPreview` transaction으로 처리한다.
`Apply_EquipmentPreview`는 part가 비고 occupied mask가 0이면 preview group을 빈 group으로 교체하고 default
visibility/body coverage를 계산한 뒤 `m_isEquipmentPreviewActive = false`로 commit한다. 하나라도 있으면 true다.
따라서 초기화만 rollback 규칙을 우회하는 두 번째 렌더 경로가 남지 않는다. `On_LevelChanged`, 성공/실패 status와
Python contract test에서도 제거된 reset 문구·symbol을 갱신한다.

authoring preset은 `authoringOnly: true`를 유지한다. 현재 format에서는 `slotSelections`의 primary 위치에만
`visualSetId`를 쓰고 secondary 위치는 `null`이어야 한다. 후속 format revision에서 `equippedOwners[]`로
바꿀 수 있지만, 같은 PR에서 두 format writer를 병존시키지 않는다. Save는 Server command, item instance,
account revision을 만들지 않는다.

`Reload_Catalog`은 preset 최초 로드 여부와 무관하게 active playable preview와 manual draft가 있으면 staged
catalog로 loadout을 다시 resolve하고 visual apply가 성공한 뒤에만 catalog generation을 교체한다.
`Reload_Presets`도 staged preset + 현재 exact catalog로 preview를 먼저 적용한다. 두 경로 모두 staged catalog의
set pointer를 commit 전 상태에 보관하지 않으며 실패 시 catalog, draft, dirty, live visual을 함께 보존한다.

이 button의 범위는 `EquipmentPresentationCatalog.json`뿐이다. `CharacterCatalog.json` v4의
`defaultBodyCoverage`는 startup-only `CActorCatalog` 데이터이므로 Tool에서 저작하거나 hot reload하지 않는다.
해당 JSON을 바꾼 뒤에는 Client를 재시작해야 하며 Tool diagnostics에
`Body coverage source: CharacterCatalog v4 (Client restart required after edits)`를 표시한다. 불완전한 Actor catalog
staged reload를 Equipment Reload에 몰래 섞지 않는다.

G03A의 고정 oracle은 다음이다.

| 시작 상태 | 명령 | 종료 상태 |
|---|---|---|
| default | Mokoko fullbody 장착 | `UPPER` owner 하나가 다섯 apparel slot 점유, `WEAPON` default |
| Mokoko fullbody | weapon A 장착 | fullbody 유지, `WEAPON` owner A 추가 |
| fullbody + weapon A | weapon B 장착 | fullbody 유지, weapon A만 B로 교체 |
| fullbody + weapon | `WEAPON` 해제 | fullbody 유지, class 기본 무기 복원 |
| fullbody + weapon | `LOWER` item 장착 | fullbody 전체 제거, lower + weapon 유지 |
| fullbody + weapon | covered `HEAD` 또는 `SHOULDER` 해제 | fullbody 전체 제거, weapon 유지 |
| 여러 partial apparel + weapon | fullbody 장착 | 겹치는 apparel owner 전부 제거, fullbody + weapon 유지 |
| fullbody + weapon | Save 후 Reload | `UPPER`와 `WEAPON` primary만 저장되고 같은 projection 복원 |
| fullbody + weapon | `Unequip All` | 모든 selection 제거, 표준 class default 복원 |
| invalid overlapping preset | Reload | draft와 live preview 모두 이전 generation 유지 |
| middle part clone 실패 | Equip | transition, draft, dirty, live preview 모두 이전 상태 유지 |
| 첫 Render에 input owner 없음 | 이후 owner 획득 | documents는 이미 검증된 상태이고 current draft를 새 preview에 정확히 한 번 적용 |
| owner 획득 시 preview apply 실패 | `Retry Preview` | 문서를 재parse하지 않고 같은 draft를 재적용하며, 성공 전까지 applied generation으로 기록하지 않음 |
| 최초 catalog/preset load 실패 | 파일 교정 뒤 `Retry Documents` | Tool 재생성 없이 두 document를 다시 stage하고 성공 뒤에만 draft/saved를 commit |

위 표는 소스 문자열 검사가 아니라 pure resolver를 실제 호출하는 native executable test로 고정한다. G03A가
끝나기 전에는 창술사 단독 `LOWER/HANDS/SHOULDER`와 `APPAREL_FULLBODY`를 제품 지원으로 표시하지 않는다.

Shared resolver는 같은 footprint의 서로 다른 owner ID를 의도적으로 알지 못하므로 Tool layer harness가 별도로
`weapon A visualSetId -> weapon B visualSetId` 전환에서 `WEAPON` primary 문자열만 B로 바뀌고 fullbody
`UPPER` 문자열은 byte-for-byte 유지되는지 검사한다. 같은 테스트는 candidate 행 선택만 한 뒤 Save하면 파일이
바뀌지 않고, `Equip Selected` 성공 뒤 Save해야만 새 primary owner가 기록되는 경계도 검증한다.

G03A 중간 필수 증거는 representative pack 재생성 결과 21 set/36 part와 기존 162-file Resources closure 일치,
publisher Validate, equipment pipeline unittest와 native resolver transition harness다. 이 증거만으로 merge하거나
기능 완료 처리하지 않고, 바로 G03B의 Client visual transaction과 Product build까지 같은 변경에서 이어 간다.

### G03B. body coverage 합성과 part group 동시 commit

slot footprint는 어떤 기본 장비를 숨길지 결정하고, body coverage는 피부 body의 어느 submesh를 숨길지
결정한다. 두 값을 하나의 mask로 섞지 않는다. 현재 class별 `COVERED_BY_ARMOUR` 고정 index mask는 모든
의상이 같은 피부 coverage를 가진다고 가정하므로 부분 장비 조합의 일반 계약이 될 수 없다.

먼저 모코코의 이중 소유를 제거한다.

| 수정 파일 | 변경 |
|---|---|
| `Data/Actors/CharacterCatalog.json` | 창술사 기본 `equipmentModels`에서 Mokoko head/armor 두 항목 제거 |
| `Client/Private/Logic_LanceMaster.cpp` | `Part_15_Avatar_Head/Armor` 기본 spec 제거, 표준 5-part default 복원 |
| `Client/Private/PlayableCharacterAssetService.cpp` | 기본 preload의 Mokoko prototype 두 개 제거, 7개 admission을 5개로 정합화 |
| `Client/Public/CharacterSpec.h` | catalog 장비와 중복되는 `EQUIPMENT_SLOT_KIND`, 별도 Client slot enum과 고정 body mask 제거 |
| `Client/Private/Character.cpp` | `hasAvatarHead/hasAvatarArmor` 존재 기반 hide 제거, resolved occupied slot만 기본 파츠를 숨기게 변경 |

이 변경 뒤 빈 loadout, `Unequip All`, catalog 장비 해제는 모두 class 표준 기본 외형으로 돌아가며 catalog
모코코만 장착 외형을 소유한다.

그다음 `EquipmentPresentationCatalog` v2의 set-level `bodyCoverage`를 다음 세 mode로 고정한다.

- `INHERIT_BASELINE`: occupied apparel slot에 대응하는 기본 part의 authored stable body material ID를 합집합한다.
- `EXPLICIT`: 해당 visual set이 `materialIds[]`를 명시한다.
- `NONE`: body submesh를 숨기지 않는다는 의도적 저작값이다.

기존 `parts[].hiddenMeshMask`는 preview part 자신의 submesh hide 값으로 유지하며 body coverage로 재사용하지
않는다.

실물 WModel 감사에서 body submesh name은 클래스 안에서도 전부 중복이었다. 예를 들어 창술사 일곱 submesh는
모두 `pc_ft_00_sk`이므로 mesh name이나 `name + vector index`를 stable ID로 쓸 수 없다. 반면 기존
`CModel::Get_MaterialName(meshIndex)`가 반환하는 material name은 현재 여섯 body에서 submesh마다 유일했다.
따라서 추가 Engine API 없이 material ID를 authoring identity로 사용하고, runtime에서 정확히 한 mesh에
resolve된 경우에만 ephemeral bit mask를 만든다.

기존 `COVERED_BY_ARMOUR` index mask를 material ID로 옮긴 실측 baseline은 다음과 같다.

| class | default slot | body material IDs |
|---|---|---|
| Lance Master | HANDS / UPPER / LOWER | `pc_ft_01_arm_mi` / `pc_ft_01_upper_mi` / `pc_ft_01_lower_mi` |
| Gunslinger | HANDS / UPPER / LOWER | `pc_gn_f_00_arm_mi` / `pc_gn_f_00_upper_mi` / `pc_gn_f_00_lower_mi` |
| Slayer | HANDS / UPPER / LOWER | `pc_wbk_f_00_arm_mi` / `pc_wbk_f_00_upper_mi`, `pc_wr_f_basebody_upper_mi` / `pc_wbk_f_00_lower_mi` |
| Artist | HANDS / UPPER / LOWER | `pc_sp_01-1_arm_mi` / `pc_sp_01_upper_mi`, `pc_sp_av_base_body_mi` / `pc_sp_01-1_lower_mi` |
| Warlord | HANDS / UPPER / LOWER | `pc_wr_00_arm_mi` / `pc_wr_00_upper_mi`, `pc_wr_base_upper_mi` / `pc_wr_00_lower_mi` |
| Dimension Master | 없음 | combined body split 전 equipment/body coverage admission 금지 |

`publish_character_equipment.py`는 `CharacterCatalog.json`을 strict parse해 selection의 class와
`masterBodyAssetId`가 character row의 `networkClassId/bodyModel`과 일치하는지 확인한다. 그 body WModel의 material
table을 읽어 `defaultBodyCoverage` 전체 ID와 EXPLICIT visual-set ID가 각각 정확히 한 mesh에 존재하고 mesh count가
32 이하인지 검사한다. `INHERIT_BASELINE`은 occupied apparel slot의 JSON coverage를 사용하며 HEAD/SHOULDER의
의도적 빈 배열은 허용한다. mismatch는 admission receipt/catalog를 교체하지 않는다.

HEAD와 SHOULDER 기본 part는 body material coverage가 없다. 위 표는 C++ `Logic_*`에 복제하지 않고
`Data/Actors/CharacterCatalog.json` 각 character row의 `defaultBodyCoverage`가 유일한 machine-readable 정본으로
소유한다. publisher와 runtime `CActorCatalog`가 같은 필드를 소비한다.
현재 representative selection 21 set 중 apparel은 admission evidence에 맞춰 `INHERIT_BASELINE`, weapon은
`NONE`으로 publish한다. validator는 `WEAPON_SET + NONE`을 강제하고 apparel만 세 mode 중 하나를 허용한다. 신규
normalized apparel은 mode가 없으면 거부한다. raw index와 숫자 bit mask는 JSON, packet, durable
state에 쓰지 않는다. unknown ID, 한 ID가 0개/2개 이상 mesh에 매칭되는 경우와 32-bit 범위 초과는 preview
stage를 실패시키고 기존 body hide state를 유지한다.

변경 대상은 `Data/Actors/CharacterCatalog.json`, `Client/Public/ActorCatalog.h`,
`Client/Private/ActorCatalog.cpp`, `Client/Public/CharacterSpec.h`, `Logic_LanceMaster.cpp`, `Logic_GunSlinger.cpp`,
`Logic_Slayer.cpp`, `Logic_Artist.cpp`, `Logic_Warlord.cpp`, 장비가 0개여도 `CHARACTER_SPEC` aggregate가 바뀌는
`Logic_DimensionMaster.cpp`,
`Client/Public/EquipmentBodyCoveragePolicy.h`, `Client/Private/EquipmentBodyCoveragePolicy.cpp`,
`Client/Public/Character.h`, `Client/Private/Character.cpp`, `Client/Public/EquipmentPresentationCatalog.h`,
`Client/Private/EquipmentPresentationCatalog.cpp`, `Data/Actors/EquipmentPresentationCatalog.json`과
catalog/presentation tests다. `CModel::Get_MaterialName`을 그대로 소비하므로 Engine public 변경과
`UpdateLib.bat`은 필요 없다.

#### G03B-1. Character coverage 입력 계약

`CharacterSpec.h`는 별도 Client slot enum과 `EQUIPMENT_SLOT_KIND`를 제거하고 Shared stable slot을 직접 쓴다.
`EQUIPMENT_PRESENTATION_SLOT`, `EquipmentPresentationSlotMask`,
`EQUIPMENT_PRESENTATION_SLOT_MASK_ALL`도 삭제한다. `Character.cpp`, 여섯 `Logic_*`,
`PlayableCharacterPreviewContract.cpp`, `EquipmentPresentationService.cpp`와 대응 테스트의 모든 비교/mask 생성은
`LostArk::Shared::EQUIPMENT_SLOT_ID`, `EquipmentSlotMask`, `EQUIPMENT_SLOT_MASK_ALL`로 바꾼다. ordinal cast나
Client↔Shared switch 변환은 남기지 않는다.
`EQUIPMENT_PART_SPEC`에는 coverage string을 복제하지 않는다. 최종 필드 블록은 다음과 같다.

```cpp
struct EQUIPMENT_PART_SPEC
{
	const tchar_t* pPartTag;
	const tchar_t* pModelTag;
	uint32_t iHiddenMeshMask;
	bool_t isHidden;
	LostArk::Shared::EQUIPMENT_SLOT_ID ePresentationSlot =
		LostArk::Shared::EQUIPMENT_SLOT_ID::END;
};
```

`CHARACTER_SPEC`에서는 `iBodyHiddenMeshMask` 한 필드만 제거하고 나머지 순서를 유지한다. 이 aggregate 변경은
장비 배열이 없는 차원술사를 포함한 여섯 `Spec_*` initializer를 모두 컴파일 대상으로 만든다.

`CharacterCatalog.json`은 strict contract 변경이므로 `formatVersion`을 3에서 4로 올리고, 여섯 character row에
다음 exact-field object를 추가한다. v3을 optional coverage로 조용히 읽지 않으며 `ActorCatalog.cpp`, publisher
fixture와 tests가 v4만 받는다. 아래는 창술사 완전한
coverage object이며 다른 class는 위 표의 값을 같은 slot에 넣는다. 차원술사는 여섯 배열이 모두 비어 있어야
하고 combined body split 전 apparel admission을 거부한다.

```json
{
  "defaultBodyCoverage": {
    "HEAD": [],
    "SHOULDER": [],
    "UPPER": ["pc_ft_01_upper_mi"],
    "LOWER": ["pc_ft_01_lower_mi"],
    "HANDS": ["pc_ft_01_arm_mi"],
    "WEAPON": []
  }
}
```

`CHARACTER_ACTOR_ENTRY`에는
`std::array<std::vector<std::string>, LostArk::Shared::EQUIPMENT_SLOT_COUNT> defaultBodyCoverageBySlot`을 추가한다.
`ActorCatalog.h`는 `Gameplay/EquipmentLoadoutContract.h`와 `<array>`를 직접 include하고, `Character.cpp`는
`ActorCatalog.h`와 `EquipmentBodyCoveragePolicy.h`를 직접 include한다.
Actor catalog parser는 slot key exact set/order 독립, per-slot duplicate/empty string, class 중복과 WEAPON nonempty를
거부하고 staged character vector가 전부 유효할 때만 commit한다. `PlayableCharacterPreviewContract`는
`CActorCatalog::Find_Character(spec class)`가 존재하고, 각 default equipment part의 Shared slot이 유효하며,
coverage가 있는 apparel slot에는 적어도 한 default part가 있는지 검사한다. 이로써 C++ regex나 static string
mirror 없이 publisher와 live Character가 같은 JSON 정본을 소비한다.

#### G03B-2. pure body coverage resolver 전체 코드

body material union과 exact-one mask 생성을 Character 내부 private loop로 숨기지 않고 다음 pure policy로 분리한다.
실제 Character와 native harness가 같은 함수를 호출한다.

`Client/Public/EquipmentBodyCoveragePolicy.h` 전체 코드는 다음과 같다.

```cpp
#pragma once

#include "Gameplay/EquipmentLoadoutContract.h"

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	using EQUIPMENT_DEFAULT_BODY_COVERAGE = std::array<
		std::vector<std::string>, LostArk::Shared::EQUIPMENT_SLOT_COUNT>;

	enum class EQUIPMENT_PREVIEW_BODY_COVERAGE_MODE : std::uint8_t
	{
		INHERIT_BASELINE,
		EXPLICIT,
		NONE,
		END
	};

	struct EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC final
	{
		EQUIPMENT_PREVIEW_BODY_COVERAGE_MODE mode =
			EQUIPMENT_PREVIEW_BODY_COVERAGE_MODE::END;
		LostArk::Shared::EQUIPMENT_SLOT_MASK occupiedSlotsMask = 0u;
		std::vector<std::string> materialIds;
	};

	bool Stage_EquipmentBodyCoverageMask(
		const EQUIPMENT_DEFAULT_BODY_COVERAGE& defaultCoverage,
		LostArk::Shared::EQUIPMENT_SLOT_MASK visibleDefaultSlotsMask,
		LostArk::Shared::EQUIPMENT_SLOT_MASK occupiedSlotsMask,
		std::span<const EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC> selectedCoverages,
		std::span<const std::string_view> bodyMaterialNames,
		std::uint32_t& outHiddenMeshMask,
		std::string& outError);
}
```

`Client/Private/EquipmentBodyCoveragePolicy.cpp` 전체 코드는 다음과 같다.

```cpp
#include "EquipmentBodyCoveragePolicy.h"

#include <algorithm>

namespace
{
	using Mask = LostArk::Shared::EQUIPMENT_SLOT_MASK;
	using Slot = LostArk::Shared::EQUIPMENT_SLOT_ID;

	constexpr Mask APPAREL_SLOT_MASK =
		LostArk::Shared::EQUIPMENT_SLOT_MASK_ALL &
		~LostArk::Shared::EquipmentSlotMask(Slot::WEAPON);

	bool Append_UniqueMaterialIds(
		const std::vector<std::string>& source,
		std::vector<std::string_view>& inOutUnion,
		std::string& outError)
	{
		for (std::size_t index = 0u; index < source.size(); ++index)
		{
			if (source[index].empty())
			{
				outError = "Body coverage material ID is empty.";
				return false;
			}
			for (std::size_t previous = 0u; previous < index; ++previous)
			{
				if (source[previous] == source[index])
				{
					outError = "Body coverage material ID is duplicated in one list.";
					return false;
				}
			}
			const std::string_view id = source[index];
			if (inOutUnion.end() ==
				std::find(inOutUnion.begin(), inOutUnion.end(), id))
			{
				inOutUnion.push_back(id);
			}
		}
		return true;
	}
}

bool Client::Stage_EquipmentBodyCoverageMask(
	const EQUIPMENT_DEFAULT_BODY_COVERAGE& defaultCoverage,
	const Mask visibleDefaultSlotsMask,
	const Mask occupiedSlotsMask,
	const std::span<const EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC>
		selectedCoverages,
	const std::span<const std::string_view> bodyMaterialNames,
	std::uint32_t& outHiddenMeshMask,
	std::string& outError)
{
	if (0u != ((visibleDefaultSlotsMask | occupiedSlotsMask) &
		~LostArk::Shared::EQUIPMENT_SLOT_MASK_ALL))
	{
		outError = "Body coverage slot mask contains an unsupported bit.";
		return false;
	}
	if (0u != (visibleDefaultSlotsMask &
		LostArk::Shared::EquipmentSlotMask(Slot::WEAPON)) ||
		0u != (visibleDefaultSlotsMask & occupiedSlotsMask))
	{
		outError = "Visible default slots conflict with the selected footprint.";
		return false;
	}
	if (!defaultCoverage[static_cast<std::size_t>(Slot::WEAPON)].empty())
	{
		outError = "Default weapon cannot own body material coverage.";
		return false;
	}
	if (bodyMaterialNames.empty() || bodyMaterialNames.size() > 32u)
	{
		outError = "Body material table is empty or exceeds the 32-bit mask.";
		return false;
	}

	std::vector<std::string_view> requestedIds;
	for (std::size_t index = 0u;
		index < LostArk::Shared::EQUIPMENT_SLOT_COUNT; ++index)
	{
		const Slot slot = static_cast<Slot>(index);
		if (0u == (visibleDefaultSlotsMask &
			LostArk::Shared::EquipmentSlotMask(slot)))
		{
			continue;
		}
		if (!Append_UniqueMaterialIds(
			defaultCoverage[index], requestedIds, outError))
		{
			return false;
		}
	}

	Mask describedApparelMask = 0u;
	for (const EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC& coverage :
		selectedCoverages)
	{
		if (coverage.mode >= EQUIPMENT_PREVIEW_BODY_COVERAGE_MODE::END ||
			0u == coverage.occupiedSlotsMask ||
			0u != (coverage.occupiedSlotsMask & ~APPAREL_SLOT_MASK) ||
			0u != (coverage.occupiedSlotsMask & ~occupiedSlotsMask) ||
			0u != (describedApparelMask & coverage.occupiedSlotsMask))
		{
			outError = "Selected body coverage footprint is invalid or overlaps.";
			return false;
		}
		describedApparelMask |= coverage.occupiedSlotsMask;

		switch (coverage.mode)
		{
		case EQUIPMENT_PREVIEW_BODY_COVERAGE_MODE::INHERIT_BASELINE:
			if (!coverage.materialIds.empty())
			{
				outError = "Inherited body coverage cannot carry explicit IDs.";
				return false;
			}
			for (std::size_t index = 0u;
				index < LostArk::Shared::EQUIPMENT_SLOT_COUNT; ++index)
			{
				const Slot slot = static_cast<Slot>(index);
				if (0u == (coverage.occupiedSlotsMask &
					LostArk::Shared::EquipmentSlotMask(slot)))
				{
					continue;
				}
				if (!Append_UniqueMaterialIds(
					defaultCoverage[index], requestedIds, outError))
				{
					return false;
				}
			}
			break;
		case EQUIPMENT_PREVIEW_BODY_COVERAGE_MODE::EXPLICIT:
			if (coverage.materialIds.empty() ||
				!Append_UniqueMaterialIds(
					coverage.materialIds, requestedIds, outError))
			{
				if (outError.empty())
					outError = "Explicit body coverage is empty.";
				return false;
			}
			break;
		case EQUIPMENT_PREVIEW_BODY_COVERAGE_MODE::NONE:
			if (!coverage.materialIds.empty())
			{
				outError = "NONE body coverage cannot carry material IDs.";
				return false;
			}
			break;
		default:
			outError = "Body coverage mode is invalid.";
			return false;
		}
	}

	if (describedApparelMask != (occupiedSlotsMask & APPAREL_SLOT_MASK))
	{
		outError = "Selected body coverage does not describe every apparel slot.";
		return false;
	}

	std::uint32_t stagedMask = 0u;
	for (const std::string_view requestedId : requestedIds)
	{
		std::size_t matchCount = 0u;
		std::size_t matchedIndex = 0u;
		for (std::size_t index = 0u; index < bodyMaterialNames.size(); ++index)
		{
			if (bodyMaterialNames[index] == requestedId)
			{
				++matchCount;
				matchedIndex = index;
			}
		}
		if (1u != matchCount)
		{
			outError = "Body coverage material ID did not resolve exactly once: " +
				std::string(requestedId);
			return false;
		}
		stagedMask |= 1u << static_cast<std::uint32_t>(matchedIndex);
	}

	outHiddenMeshMask = stagedMask;
	outError.clear();
	return true;
}
```

Client project/filter와 Character Select harness에 이 H/CPP를 visual policy와 같은 방식으로 등록한다. harness는
default `UPPER/HANDS`가 보이는 lower-only 조합, fullbody+weapon, explicit/none, missing/duplicate material,
33-mesh overflow, overlapping coverage desc를 실행하고 실패 때 sentinel out mask가 유지되는지 검사한다.

```xml
<!-- Client/Default/Client.vcxproj -->
<ClInclude Include="..\Public\EquipmentBodyCoveragePolicy.h" />
<ClCompile Include="..\Private\EquipmentBodyCoveragePolicy.cpp" />

<!-- Client/Default/Client.vcxproj.filters -->
<ClInclude Include="..\Public\EquipmentBodyCoveragePolicy.h">
  <Filter>02.GameObjects\00. Character</Filter>
</ClInclude>
<ClCompile Include="..\Private\EquipmentBodyCoveragePolicy.cpp">
  <Filter>02.GameObjects\00. Character</Filter>
</ClCompile>

<!-- Tools/CharacterSelectIsolationHarness/Default/CharacterSelectIsolationHarness.vcxproj -->
<ClCompile Include="..\..\..\Client\Private\EquipmentBodyCoveragePolicy.cpp" />

<!-- CharacterSelectIsolationHarness.vcxproj.filters -->
<ClCompile Include="..\..\..\Client\Private\EquipmentBodyCoveragePolicy.cpp">
  <Filter>Private</Filter>
</ClCompile>
```

`ClientPresentationPrimitiveContractTests.cpp`에 `EquipmentBodyCoveragePolicy.h`를 include하고 다음 함수를
`VerifyVisualEquipmentOwnerIdentity`와 같은 runner에 등록한다.

```cpp
	bool VerifyEquipmentBodyCoveragePolicy()
	{
		using namespace LostArk::Shared;
		using Slot = EQUIPMENT_SLOT_ID;
		using Mode = Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_MODE;

		Client::EQUIPMENT_DEFAULT_BODY_COVERAGE defaults{};
		defaults[static_cast<std::size_t>(Slot::HANDS)] = { "hands" };
		defaults[static_cast<std::size_t>(Slot::UPPER)] = { "upper" };
		defaults[static_cast<std::size_t>(Slot::LOWER)] = { "lower" };
		const std::array<std::string_view, 3u> bodyMaterials{
			"hands", "upper", "lower"
		};

		const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC lower{
			Mode::INHERIT_BASELINE,
			EquipmentSlotMask(Slot::LOWER),
			{}
		};
		std::uint32_t mask = 0u;
		std::string error;
		if (!Require(
			Client::Stage_EquipmentBodyCoverageMask(
				defaults,
				EquipmentSlotMask(Slot::UPPER) |
					EquipmentSlotMask(Slot::HANDS),
				EquipmentSlotMask(Slot::LOWER),
				std::span<const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC>(
					&lower, 1u),
				bodyMaterials, mask, error) &&
			0x7u == mask,
			"lower-only coverage dropped visible upper/hands baseline"))
		{
			return false;
		}

		const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC fullbody{
			Mode::INHERIT_BASELINE, 0x1fu, {}
		};
		if (!Require(
			Client::Stage_EquipmentBodyCoverageMask(
				defaults, 0u, 0x3fu,
				std::span<const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC>(
					&fullbody, 1u),
				bodyMaterials, mask, error) &&
			0x7u == mask,
			"fullbody coverage was not independent from WEAPON"))
		{
			return false;
		}

		const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC noneHead{
			Mode::NONE, EquipmentSlotMask(Slot::HEAD), {}
		};
		if (!Require(
			Client::Stage_EquipmentBodyCoverageMask(
				defaults,
				EquipmentSlotMask(Slot::UPPER) |
					EquipmentSlotMask(Slot::LOWER) |
					EquipmentSlotMask(Slot::HANDS),
				EquipmentSlotMask(Slot::HEAD),
				std::span<const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC>(
					&noneHead, 1u),
				bodyMaterials, mask, error) &&
			0x7u == mask,
			"NONE head coverage changed visible default body coverage"))
		{
			return false;
		}

		const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC explicitHead{
			Mode::EXPLICIT,
			EquipmentSlotMask(Slot::HEAD),
			{ "upper" }
		};
		if (!Require(
			Client::Stage_EquipmentBodyCoverageMask(
				defaults,
				EquipmentSlotMask(Slot::UPPER) |
					EquipmentSlotMask(Slot::LOWER) |
					EquipmentSlotMask(Slot::HANDS),
				EquipmentSlotMask(Slot::HEAD),
				std::span<const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC>(
					&explicitHead, 1u),
				bodyMaterials, mask, error) &&
			0x7u == mask,
			"explicit head coverage did not merge with visible defaults"))
		{
			return false;
		}

		const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC missing{
			Mode::EXPLICIT,
			EquipmentSlotMask(Slot::HEAD),
			{ "missing" }
		};
		const std::array<std::string_view, 4u> duplicateMaterials{
			"hands", "upper", "upper", "lower"
		};
		std::array<std::string_view, 33u> tooManyMaterials{};
		tooManyMaterials.fill("unused");
		const std::array<Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC, 2u>
			overlapping{
				Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC{
					Mode::NONE, EquipmentSlotMask(Slot::UPPER), {} },
				Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC{
					Mode::NONE, EquipmentSlotMask(Slot::UPPER), {} }
			};
		for (const auto& [materials, descriptions, occupied] : std::array{
			std::tuple{
				std::span<const std::string_view>(bodyMaterials),
				std::span<const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC>(
					&missing, 1u),
				EquipmentSlotMask(Slot::HEAD) },
			std::tuple{
				std::span<const std::string_view>(duplicateMaterials),
				std::span<const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC>(
					&fullbody, 1u), 0x1fu },
			std::tuple{
				std::span<const std::string_view>(tooManyMaterials),
				std::span<const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC>(
					&fullbody, 1u), 0x1fu },
			std::tuple{
				std::span<const std::string_view>(bodyMaterials),
				std::span<const Client::EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC>(
					overlapping), EquipmentSlotMask(Slot::UPPER) }
		})
		{
			mask = 0xa5a5a5a5u;
			if (!Require(
				!Client::Stage_EquipmentBodyCoverageMask(
					defaults, 0u, occupied, descriptions,
					materials, mask, error) &&
				0xa5a5a5a5u == mask,
				"invalid body coverage changed the caller output"))
			{
				return false;
			}
		}

		return true;
	}
```

위 test가 `std::tuple`을 사용하므로 harness include에 `<tuple>`도 추가하고 runner에는
`!VerifyEquipmentBodyCoveragePolicy()`를 `VerifyVisualEquipmentOwnerIdentity` 다음에 넣는다.

`Character.h`는 catalog type을 직접 include하지 않고 `EquipmentBodyCoveragePolicy.h`의 renderer 입력을 사용해
다음 signature만 둔다.

```cpp
bool_t Apply_EquipmentPreview(
	const std::vector<EQUIPMENT_PREVIEW_PART_DESC>& parts,
	LostArk::Shared::EQUIPMENT_SLOT_MASK occupiedSlotsMask,
	const std::vector<EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC>& bodyCoverages,
	std::string& outError);
```

`EquipmentPresentationService.cpp`만 catalog mode를 위 renderer mode로 exact switch하고, set 하나당 desc 하나를
만든다. `occupiedSlotsMask`는 그 set footprint와 같아야 하며 전체 desc mask의 OR는 resolved projection의
apparel bit와 같아야 한다. weapon set은 body coverage desc를 만들지 않는다. unknown mode, duplicate material
ID, apparel mask 불일치, coverage desc에 WEAPON bit가 있으면 Character 호출 전에 실패한다.

`Character.cpp`는 두 번째 material resolver를 만들지 않는 thin adapter다. 현재 `isHidden`과 candidate footprint로
`visibleDefaultSlotsMask`를 만들고, `m_pBodyModel->Get_MaterialName(index)`의 `string_view` 배열과
`CActorCatalog::Find_Character(m_pSpec->eCharacterClass)->defaultBodyCoverageBySlot`을 조립한 뒤
`Stage_EquipmentBodyCoverageMask(defaultCoverage, visibleDefaultSlotsMask, occupiedSlotsMask, bodyCoverages,
bodyMaterialNames, stagedMask, outError)`를 정확히 한 번 호출한다. union, exact-one, 32-bit 검사는 pure policy만
소유한다. `Ready_PartObjects`는 `bodyDesc.iHiddenMeshMask = 0 -> body part Add -> m_pBodyModel 획득 -> 빈
loadout/default coverage exact-one resolve -> CPart_Body::Set_HiddenMeshes` 순서로 초기 mask를 설정하며, 실패하면
`E_FAIL`로 character 생성 전체를 중단한다. body를 0 mask로 생성한 뒤 resolve 실패를 무시하는 경로는 두지 않는다.

`EQUIPMENT_SLOT_KIND`와 `EQUIPMENT_PART_SPEC::eSlotKind`는 전부 제거한다. 모코코가 기본 spec에서 빠지면
남은 관계는 `isHidden`이 초기 default part의 표시 여부를, `ePresentationSlot`이 장비에 의해 가려지는 slot을
결정하는 것뿐이다. `CHARACTER_SPEC::iBodyHiddenMeshMask`도 제거한다. Character 생성 때 보이는 모든 default part의 material ID를
resolve해 초기 mask를 만들고, preview 때는 `CEquipmentPresentationService`가 선택된 각 set의 coverage mode와
footprint를 `EQUIPMENT_PREVIEW_BODY_COVERAGE_DESC`로 전달한다. `CCharacter::Apply_EquipmentPreview`는 clone 전에
다음 순서로 최종 mask를 계산한다.

1. candidate footprint가 점유하지 않고 `isHidden == false`인 default part가 하나 이상 있는 각 slot의
   `defaultBodyCoverageBySlot[slot]`을 추가한다.
2. `INHERIT_BASELINE` set은 그 set이 점유한 default slot의 material IDs를 추가한다.
3. `EXPLICIT` set은 catalog의 material IDs를 추가하고 `NONE`은 아무것도 추가하지 않는다.
4. 각 ID가 body model의 정확히 한 mesh material name에 대응하는지 검사한 뒤에만 bit mask를 만든다.

이렇게 해야 fullbody는 기존 전체 baseline coverage를 유지한다. lower-only나 head-only candidate 자체는
관계없는 torso/arm coverage를 추가하지 않지만, 계속 보이는 default UPPER/HANDS part의 baseline coverage는
최종 mask에 그대로 남는다. 빈 loadout과 `Unequip All`도 같은 default coverage resolver를 사용한다.

commit 순서는 다음으로 고정한다.

```text
visible default part coverage resolve
-> selected visual-set coverage resolve
-> 최종 body hide mask stage
-> 모든 preview part clone stage
-> preview part group replace
-> occupied-slot 기본 part visibility와 body hide mask commit
```

어느 단계든 실패하면 staged clone만 폐기하고 이전 part group, 기본 part visibility, body hide mask,
Tool draft와 dirty를 유지한다. model prototype batch는 현재 `Apply_EquipmentPreview`보다 먼저 등록되는
monotonic admission cache이므로 후단 실패 뒤 남을 수 있다. 이를 렌더 상태 commit으로 간주하지 않으며 이번
작업에서 unsafe unregister를 추가하지 않는다. pure native harness가 자동 증명하는 범위는 transition output과
body mask output의 실패 시 불변까지다. 실제 `CCharacter` part-group/default visibility commit과 middle clone 실패는
Product compile만으로 PASS 처리하지 않고, 사용자의 Tool smoke 및 명시적 실패 status 관찰 전에는 visual/rollback
검증 미완료로 기록한다.

G03A/G03B의 자동 종료 증거는 모코코 전신, 표준 4-part outfit, head-only, lower-only, weapon-only 조합의
owner projection과 pure body material mask를 native harness로 검증하고, publisher Validate, equipment pipeline
unittest, Shared→Client Product build와 `git diff --check`를 통과하는 것이다. default part 실제 draw, part-group
교체, middle clone rollback과 최종 pixel 판정은 사용자가 직접 수행하며 그 관찰 전에는 RESULT의 visual/manual
항목을 PASS로 기록하지 않는다.

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
  schemaVersion
  stateRevision
  equipmentSemanticRevision
  selectedClassId
  inventory[]: itemInstanceId, itemId, quantity, acquisitionTransactionId
  classLoadouts[classId][]: primarySlot, itemInstanceId
  pendingRaidRewards[]: transactionId, raidId, clearEpoch, rolledItemId, claimState
```

model path, `visualSetId`, source object, Prototype/Part tag, pointer, vector index는 저장하지 않는다.
재접속 시 instance의 `itemId + classId`를 저장된 `equipmentSemanticRevision`과 현재 `ItemCatalog`로 다시
resolve하고, Client는 승인된 itemId를 `EquipmentPresentationCatalog`에 join한다. 삭제·비호환 item 또는
footprint 의미 revision 불일치를 만나면 해당 loadout을 임의 대체하거나 secondary row를 추측하지 않고 typed
quarantine 결과와 함께 기존 durable row를 보존한다.

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
python -m unittest discover -s Tools/CharacterEquipmentPipeline -p "test_*.py"
powershell -ExecutionPolicy Bypass -File Tools/CharacterEquipmentPipeline/New-CharacterEquipmentRepresentativePack.ps1 `
  -SelectionPath Tools/CharacterEquipmentPipeline/RepresentativeCharacterEquipment.json `
  -OutputDirectory out/CharacterEquipmentExtraction/Admission/representative-20260901-visible-variants `
  -ResourceRoot Client/Bin/Resources `
  -Overwrite
powershell -ExecutionPolicy Bypass -File Tools/CharacterEquipmentPipeline/Publish-CharacterEquipment.ps1 `
  -Mode Validate `
  -AdmissionRoot out/CharacterEquipmentExtraction/Admission/representative-20260901-visible-variants `
  -ResourceRoot Client/Bin/Resources
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-ItemCatalog.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanClearRewards.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic
git diff --check
```

기존 G03의 Engine public header를 바꾸는 경우에는 `UpdateLib.bat` 뒤 Product Client까지 다시 빌드한다.
G03A/G03B의 현재 확정안은 기존 `CModel::Get_MaterialName`을 사용하므로 Engine public 변경이 없다.

## 8. 자동 검증 행렬

| 영역 | 정상 사례 | 반드시 실패할 사례 | rollback 증거 |
|---|---|---|---|
| source inventory | 7,475 exact denominator | wrong class prefix, Raw/RawShared duplicate ID | 이전 inventory output 유지 |
| normalizer | weighted palette index/name 일치 | unknown/duplicate joint, bad inverse bind | Resources 미승격 |
| presentation catalog | 6-class/multi-part set, Mokoko 2-part fullbody | missing model/socket/stance, apparel+WEAPON, category/footprint mismatch | 이전 catalog 유지 |
| composite-slot resolver | fullbody+independent weapon 전이 oracle | overlap preset, wrong primary, invalid/high bit, covered 중복 저장 | 입력 loadout/output 불변 |
| Equipment Tool | affected-slot 후보, owner/covered/evicted/preserved 표시 | removed set ID, staged catalog mismatch, middle clone 실패 | 이전 draft/dirty/live visual 유지 |
| item publisher | equipment semantic publish와 presentation exact join | bad slot/class/version/duplicate/category mismatch | bootstrap 미교체 |
| reward publisher | positive weighted class pool | zero/overflow/unknown item | reward bootstrap 미교체 |
| reward runtime | exactly-once auto claim | duplicate clear/claim, full inventory | pending result 고정, no reroll |
| equip runtime | owned compatible item | missing instance, wrong class/slot, stale revision | inventory/loadout revision 불변 |
| world transfer | empty/full state 보존 | target admission/send failure | source room/state 유지 |
| durable repository | restart/reconnect state 복원 | stale CAS, corrupt row, DB write failure | 이전 DB/room revision 유지 |
| Client presentation | all parts/default visibility/body coverage atomic commit | middle clone/skeleton/socket/body mesh resolve failure | 이전 visual generation/default visibility/body mask 유지 |
| replication | local/remote/late join 동일 | reordered/stale snapshot | 최신 generation 유지 |

## 9. 수동 화면 검증 경계

Client와 UI는 사용자가 직접 실행하고 시각 결과를 판정한다. 에이전트는 Client를 자율 실행하거나
스크린샷을 만들고 visual PASS를 기록하지 않는다.

G03A/G03B 자동 검증 뒤에는 사용자가 다음 Tool 경로를 먼저 판정한다.

1. `Server + Client` launch profile을 `Ctrl+F5`로 시작하고 Character Select에 진입한다.
2. F1 `Equipment Authoring Tool`에서 `Lance Master`와 active playable preview를 맞춘다.
3. `HEAD` 또는 `LOWER` slot에서 모코코 fullbody를 선택하고 impact의 owner `UPPER`, occupied 다섯 slot,
   preserved `WEAPON`을 확인한 뒤 `Equip Selected`를 누른다.
4. `WEAPON`에서 `READY_ALTERNATIVE` 무기를 장착하고 모코코가 유지된 채 무기만 바뀌는지 확인한다.
5. covered `LOWER`에서 `Unequip Slot`을 눌러 모코코 전체만 사라지고 선택 무기가 유지되는지 확인한다.
6. 다시 모코코와 무기를 장착해 `Save Presets -> Unequip All -> Reload Presets` 순서로 같은 외형과
   primary-only owner가 복원되는지 확인한다.
7. 걷기, 스킬, stance 전환에서 body 관통·누락과 무기 visibility를 확인한다.

G04/G05까지 구현한 뒤에는 다음 제품 흐름을 추가로 판정한다.

1. Lobby에서 승인된 class로 Valtan Arena에 진입한다.
2. 발탄 클리어 후 reward 결과가 class pool에서 하나만 확정되는지 확인한다.
3. Inventory의 장비 item을 각 equipment slot에 장착하고 해제 시 기본 appearance가 복원되는지 확인한다.
4. class를 바꿨다가 돌아와 per-class loadout이 복원되는지 확인한다.
5. Server를 정상 종료·재시작하고 같은 회원/캐릭터로 재접속해 inventory와 loadout이 복원되는지 확인한다.
6. 2인 이상에서 상대 장비 교체와 late join 외형이 같은지 확인한다.

최종 visual fidelity와 `manual first pixel`, occurrence 승인은 사용자의 서면 관찰이 있어야만 RESULT에
PASS로 기록한다.
