# 6개 클래스 장비 리소스 추출·분류 결과

작성일: 2026-09-01

대상 클래스: 차원술사, 도화가, 창술사, 워로드, 슬레이어, 건슬링어

대응 계획서: `2026-09-01_SIX_CLASS_EQUIPMENT_EXTRACTION_AND_LOADOUT_IMPLEMENTATION_PLAN.md`

## 1. 실제 완료 상태

이번 작업에서 완료한 것은 다음 범위다.

- 설치된 LostArk KR 원본 package에서 여섯 클래스의 class/shared/weapon SkeletalMesh 분모를 실측했다.
- LostArk 전용 UModel로 class/weapon 13 batch와 shared family를 glTF/DDS로 추출했다.
- broad batch 중단 뒤 package/object 격리 재시도로 추출 가능한 object를 모두 채웠다.
- 남은 11개에 네 종류의 최소 복구 경로를 적용하고 동일 decoder 차단임을 확인했다.
- dependency export를 제거한 정확한 7,475행 source inventory를 생성했다.
- 2,159개 canonical package/visual-set으로 class별 정리했다.
- 각 object에 part role, slot hint, locale/quality, 저장 후보 종류, runtime readiness를 기록했다.
- 이미 Cook된 호환 WModel 중 6 class, 12 visual set, 17 part를 curated source association과 함께
  self-contained 대표 staging pack으로 산출했다.
- 여섯 class별 신규 shoulder raw 후보를 하나씩 pack에 포함하고 정규화 대기 카테고리로 기록했다.
- 장비 교체·해제, Server random reward, inventory instance, per-class loadout의 구현 계획서를 작성했다.
- 현재 회원/account 저장 경로가 없음을 코드로 확인하고 durable repository 계획을 추가했다.
- 대표 12 visual set, 17 part를 stable slot과 named part로 승격한
  `Data/Actors/EquipmentPresentationCatalog.json`을 생성했다.
- 여섯 class의 툴 전용 초기 선택을 `Data/Actors/EquipmentLoadoutPresets.json`에 분리했다. 이 문서는
  `authoringOnly: true`이며 회원/account 저장 계약이 아니다.
- admission의 runtime closure 73개를 전부 해시 검증하고 새 `Character/<Class>/Equipment` 디렉터리만
  stage/promote하는 `Publish-CharacterEquipment.ps1`과 rollback harness를 구현했다.
- 검증된 runtime closure 73개를 실제 정본 Resources의 여섯 class `Equipment` 디렉터리로 Publish하고,
  재검증에서 여섯 디렉터리가 모두 identical 상태임을 확인했다.
- 기존 공용 `CCharacterPreviewPanel`을 재사용하는 F1 `Equipment Authoring Tool`을 추가했다. 여섯 class와
  `HEAD/SHOULDER/UPPER/LOWER/HANDS/WEAPON`을 선택하고 대표 set을 장착·해제·초기화할 수 있다.
- Catalog와 preset은 strict parse/validate/stage/commit을 사용한다. visual set의 모든 model/socket 검증과
  part clone이 성공한 뒤에만 prefix part group과 툴 draft를 교체하며 실패하면 이전 외형과 draft를 유지한다.
- `CharacterSpec`에 기본 presentation slot을 명시해 선택 set의 `occupiedSlots`만 기본 파츠와 교체한다.
  HEAD-only, WEAPON-only와 복합 outfit이 서로 비점유 기본 외형을 지우지 않는다.
- 툴 preset은 `Data/Actors/EquipmentLoadoutPresets.json`에 sibling temp write, 재파싱, atomic replace로
  저장한다. 이 문서는 계속 `authoringOnly: true`이며 회원정보 저장이 아니다.

완료하지 않은 범위는 다음과 같다.

- master skeleton 재바인딩과 attachment-ready WModel bulk cook
- 신규 shoulder 6개의 normalized WModel cook
- item/reward JSON 변경
- Shared/Server 제품 inventory·장착 명령과 replicated Client 장비 snapshot 구현
- 인증된 account/character repository와 재접속 영구 저장
- 사용자의 수동 화면 검증

따라서 이번 요청의 “6캐릭터 대표 산출, 실제 Resources 반영, F1 툴에서 슬롯별 교체와 툴 preset 저장”은
구현됐다. 다만 이것은 preview-only 저작·표현 슬라이스다. 레이드 보상, 제품 Inventory UI, Server
소유권과 회원 캐릭터 영구 저장을 포함한 최종 runtime 수직 슬라이스는 아직 완료가 아니다.

## 2. 추출 도구와 원본

원본 package root:

```text
C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages
```

사용 도구:

- Gildor LostArk 전용 UEViewer/UModel v7, build 1544 fix2
- UModel archive SHA-256:
  `97EE9E856B0F678E84E115889DFD50A9B515110F3814B8CC0A0DCF8FB65A5878`
- UModel executable SHA-256:
  `B9573CDCBB7E9D26DBF60A0E3AF47FB5AF8543140873DA8483C26D58CF40B249`
- SDL2 2.32.10 x64 archive SHA-256:
  `6CF9706EEFD0A4A06DC764007934D428AFAF029FABDD408A9E646048C91E18FB`
- SDL2.dll SHA-256:
  `B37740A72A7A9706216DF9F0134894BB7A850B356FD149398C67D874CBCFACB4`

로컬 도구 위치:

```text
out/CharacterEquipmentExtraction/Tools/UEViewerLostArk_runtime/umodel_lostark_v7.exe
```

공식 참고:

- `https://www.gildor.org/smf/index.php?topic=3055.0`
- `https://www.gildor.org/en/projects/umodel`
- `https://github.com/gildor2/UEViewer`

## 3. 정확한 source 분모

| 클래스 | exact source union | package | SkeletalMesh | Texture2D | 압축 원본 |
|---|---|---:|---:|---:|---:|
| 창술사 | `PC_FLM* + (PC_FT* - PC_FT_M*) + WP_WFLM*` | 436 | 1,518 | 2,622 | 1.34 GiB |
| 건슬링어 | `PC_GDH_F* + PC_GN_F* + WP_WGDH*` | 369 | 1,408 | 1,354 | 1.02 GiB |
| 슬레이어 | `PC_WBK_F* + PC_WR_F* + WP_WWBK*` | 408 | 1,220 | 1,107 | 0.97 GiB |
| 도화가 | `PC_SDM* + (PC_SP* - PC_SP_M*) + WP_WSDM*` | 387 | 1,361 | 1,554 | 1.15 GiB |
| 워로드 | `PC_WGL* + (PC_WR* - PC_WR_F*) + WP_WWGL*` | 443 | 1,514 | 3,704 | 1.47 GiB |
| 차원술사 | `PC_SWP_M* + PC_SP_M* + WP_WSWP_M*` | 148 | 454 | 610 | 0.39 GiB |
| 합계 | unique class source union | 2,191 | 7,475 | 10,951 | 약 6.34 GiB |

이 분모에는 base body, face, hair, apparel, weapon, FX/helper와 locale/high variant가 포함된다.
package만 존재하고 SkeletalMesh가 없는 material-only tail은 모델 수에 포함하지 않았다.

## 4. raw 추출 결과

### 4.1 class/weapon batch

13개 class/weapon batch는 모두 stderr 0 bytes이고 `Exported done/total`이 일치했다.

| 클래스 | raw glTF | raw bytes | scope |
|---|---:|---:|---|
| 창술사 | 163 | 592,295,826 | `PC_FLM*`, `WP_WFLM*` |
| 건슬링어 | 195 | 530,009,887 | `PC_GDH_F*`, `WP_WGDH*` |
| 슬레이어 | 114 | 420,515,483 | `PC_WBK_F*`, `WP_WWBK*` |
| 도화가 | 96 | 389,039,429 | `PC_SDM*`, `WP_WSDM*` |
| 워로드 | 145 | 362,514,725 | `PC_WGL*`, `WP_WWGL*` |
| 차원술사 | 454 | 1,831,009,433 | `PC_SP_M*`, `PC_SWP_M*`, `WP_WSWP_M*` |

성공 receipt 예:

```text
Artist.PC_SDM.stdout.log             Exported 1136/1136
DimensionMaster.PC_SP_M.stdout.log   Exported 2870/2870
Gunslinger.PC_GDH_F.stdout.log       Exported 1039/1039
LanceMaster.PC_FLM.stdout.log        Exported 1213/1213
Slayer.PC_WBK_F.stdout.log           Exported 968/968
Warlord.PC_WGL.stdout.log            Exported 1276/1276
```

### 4.2 shared family

- `PC_GN_F`: 1,213 glTF, `Exported 9636/9636`
- `PC_SP`: 1,629 glTF, `Exported 12146/12146`
- `PC_FT`: broad 중단 뒤 object retry로 1,349/1,351 class-selected mesh 확보
- `PC_WR_F`: broad 중단 뒤 object retry로 1,097/1,106 class-selected mesh 확보
- `PC_WR` male: 1,362/1,362 확보

raw 출력 전체는 dependency를 포함해 다음 크기다.

```text
files: 93,063
glTF: 7,863
bytes: 30,793,114,057 (28.678 GiB)
```

이 7,863 glTF는 dependency export가 섞인 물리 파일 수다. 제품 source 분모는 class allowlist로 거른
7,475행이며 둘을 같은 숫자로 해석하지 않는다.

## 5. 최종 coverage와 차단 11개

```text
source denominator: 7,475
extracted:          7,464
blocked:               11
coverage:          99.8528%
product runtime admitted: 0
validated staging parts: 17
pending normalization raw parts: 6
```

차단 object:

| 클래스 | package | object |
|---|---|---|
| 창술사 | `PC_FT_AV_247` | `pc_ft_av_247_dress_sk` |
| 창술사 | `PC_FT_AV_247` | `pc_ft_av_247_head_sk` |
| 슬레이어 | `PC_WR_F_AV_220A` | `pc_wr_f_av_220a_face2_sk` |
| 슬레이어 | `PC_WR_F_AV_220A` | `pc_wr_f_av_220a_head_sk` |
| 슬레이어 | `PC_WR_F_AV_220A` | `pc_wr_f_av_220a_lower_sk` |
| 슬레이어 | `PC_WR_F_AV_220A` | `pc_wr_f_av_220a_upper_sk` |
| 슬레이어 | `PC_WR_F_AV_220B` | `pc_wr_f_av_220b_face2_sk` |
| 슬레이어 | `PC_WR_F_AV_220B` | `pc_wr_f_av_220b_head_sk` |
| 슬레이어 | `PC_WR_F_AV_220B` | `pc_wr_f_av_220b_lower_sk` |
| 슬레이어 | `PC_WR_F_AV_220B` | `pc_wr_f_av_220b_upper_sk` |
| 슬레이어 | `PC_WR_F_AV_220B` | `pc_wr_f_av_220b-1_head_sk` |

격리 복구 감사 경로:

```text
out/CharacterEquipmentExtraction/BlockedRecoveryAudit_20260901
```

11개 각각에 최소 glTF, 최소 PSK, 최소 MD5Mesh, 물리 UPK 직접 지정 네 경로를 실행했다.
44/44회 모두 exit 1, model output 0, 같은 `Serializing behind stopper`였다. 세 UPK는 설치 manifest의
size/MD5와 일치했다. format, material, animation, resolver 문제가 아니라 현재 UModel의 해당
`FStaticLODModel3` binary layout 미지원으로 판정했다.

## 6. 전체 분류 산출물

재현 명령:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/CharacterEquipmentPipeline/New-CharacterEquipmentSourceInventory.ps1
```

생성 파일:

| 파일 | 내용 | 크기 |
|---|---|---:|
| `character-equipment-source-inventory.csv` | object 단위 7,475행 + header | 3,431,443 bytes |
| `character-equipment-source-inventory.json` | 같은 object와 readiness 전체 | 약 9.5 MB |
| `character-equipment-source-inventory.md` | 클래스별 package 2,159행 사람이 읽는 표 | 329,020 bytes |
| `character-equipment-visual-sets.json` | canonical visual set과 필수 role | 약 2.7 MB |
| `character-equipment-extraction.receipt.json` | tool hash, coverage, blocked/recovery receipt | compact JSON |

공통 출력 폴더:

```text
out/CharacterEquipmentExtraction/Inventory/Generated
```

source inventory의 저장 키는 `character.<class>.<exact-package-family>`다. `Raw` 또는 `RawShared`를
ID에 넣지 않는다. 차원술사 `PC_SP_M_*` 129 package, 364 glTF는 두 scope에서 SHA-256까지 모두 같아
하나의 canonical set으로만 기록했다.

## 7. 클래스별 부품·저장 후보

`의상`, `외형/base`, `무기`, `제외`는 서로 겹치지 않는다. `수동 검토`는 head/combined/비표준 이름
후보 중 role/coverage를 눈으로 저작해야 하는 부분집합이다.

| 클래스 | canonical visual set | 의상 후보 | 외형/base 후보 | 무기 후보 | 수동 검토 | 제외 |
|---|---:|---:|---:|---:|---:|---:|
| 창술사 | 428 | 740 | 732 | 40 | 445 | 6 |
| 건슬링어 | 360 | 644 | 647 | 116 | 421 | 1 |
| 슬레이어 | 401 | 512 | 659 | 45 | 389 | 4 |
| 도화가 | 385 | 667 | 675 | 18 | 408 | 1 |
| 워로드 | 437 | 721 | 721 | 65 | 442 | 7 |
| 차원술사 | 148 | 186 | 210 | 52 | 123 | 6 |

### 7.1 바로 저장 구조로 만들 수 있는 단위

다음은 runtime-ready가 아니라 authored catalog 후보로 만들 수 있는 단위다.

- `upper`, `lower`, `arm/glove`, `shoulder`, `helmet`: 개별 skinned equipment part
- `dress/robe/skirt/cape/cloak`: 하나의 compound visual set과 coverage slot
- `hair`, `face`: raid item과 분리한 appearance preset part
- `head/haed`: helmet인지 hair/deco인지 수동 coverage 판정이 필요한 head candidate
- `WP_*`: source package 하나를 다중 part weapon visual set 하나로 저장

클래스별 무기 필수 member:

| 클래스 | 한 item/visual set에 묶을 역할 |
|---|---|
| 창술사 | `L + S` |
| 건슬링어 | `H + L + S` |
| 슬레이어 | `MAIN` |
| 도화가 | `MAIN` |
| 워로드 | `MAIN + SHIELD(SD)` |
| 차원술사 | `E + L + P + S` |

비테스트 weapon package는 위 필수 역할을 모두 갖는다. 다만 numbered/high/backup 추가 member는
publisher가 임의 선택하지 않고 authored member list를 요구해야 한다.

### 7.2 자동 admission 금지

- `PC_SWP_M_00/pc_swp_fx01_sk`: 차원술사 body가 아닌 FX rig
- `WP_WSWP_M_00_TEST`
- `PC_FT_CREATION`
- `PC_WR_F_AV_142_TEST`
- `PC_WR_AV_163A_UPPER_TEST`, `PC_WR_AV_163A-1_UPPER_TEST`
- 이름의 `test`, `old`, `bk`, `hairtest`
- extractor 차단 11개
- role이 `HEAD_OR_HAIR` 또는 manual coverage인 상태에서 authored 결정이 없는 object

## 8. 왜 raw 추출물은 Resources에 복사하지 않았는가

원본 glTF를 현재 `ModelAssetConverter`에 직접 넣는 것은 geometry 크기만 맞고 body palette가 맞지 않았다.

| 클래스 | raw glTF joints | 현재 runtime body/part cooked bones | 판정 |
|---|---:|---:|---|
| 창술사 | 221 | 224 | direct WModel은 222로도 불일치 |
| 건슬링어 | 236 | 239 | 불일치 |
| 도화가 | 236 | 239 | 불일치 |
| 슬레이어 | 238 | 245 | 불일치 |
| 워로드 | 215 | 218 | 불일치 |
| 차원술사 의상 예 | 159 | current body 225 | subset이며 그대로 장착 불가 |

`CMesh::Bind_Resource`는 bone 이름 remap 없이 palette index를 직접 사용한다. 따라서 `skeleton=yes`나
bounds 일치만으로 admission하면 애니메이션 시 의상이 잘못 변형될 수 있다. 기존 창술사 파츠와 같은
방식으로 class master skeleton에 재바인딩해야 한다. admission은 모든 positive-weight blend index가
body의 같은 index/name을 가리키는지와 source inverse bind/rebind 결과를 검증해야 한다. 기존 valid
part에도 weight가 전혀 없는 synthetic source-mesh node 이름 한 개는 body와 다르므로 전체 skeleton
hash 단순 동등 비교를 사용하면 안 된다.

그래서 다음을 지켰다.

- raw glTF/DDS는 모두 `out/CharacterEquipmentExtraction`에 유지했다.
- direct-cook probe도 `out/.../Probe`에만 유지했다.
- `Client/Bin/Resources`에는 raw, source, 불완전 WModel을 복사하지 않았고, admission을 통과한 runtime
  closure 73개만 이후 publisher로 승격했다.
- `Resources/LostArk` wrapper를 만들지 않았다.
- Drive 관리 Resources 물리 payload에는 여섯 Equipment 디렉터리를 반영했지만 Git index에는 넣지 않았다.

## 9. 1차 대표 staging pack

### 9.1 산출 위치와 범위

재현 명령:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File Tools/CharacterEquipmentPipeline/New-CharacterEquipmentRepresentativePack.ps1 `
  -Overwrite
```

처음 만드는 빈 경로에서는 `-Overwrite`를 빼도 된다. 기존 pack 재생성은 `-Overwrite`를 명시하며 기존
폴더는 같은 Admission root의 timestamp backup으로 먼저 이동한다. `-Overwrite`가 없으면 파일 복사 전에
fail-fast한다.

실제 출력:

```text
out/CharacterEquipmentExtraction/Admission/representative-20260901-curated
```

이 폴더에는 curated raw source glTF/buffer를 둔 `Source`, 계획된 Resources-relative ID 구조로 WModel과 실제
texture closure를 둔 `Runtime`, hash와 admission 판정을 둔
`character-equipment-runtime-admission.json`이 있다.

```text
state:                 STAGED_VALIDATED_NOT_PROMOTED
source association:    CURATED_NOT_BUILD_PROVENANCE
classes:               6
visual sets:           12
parts:                 17
runtime closure files: 73
pending raw parts:     6
source files:          46
total files:           121
total bytes:           86,928,104
```

| 클래스 | Cook 의상 상태 | 즉시 쓸 카테고리 | 무기 set | 정규화 대기 |
|---|---|---|---:|---:|
| 차원술사 | `NO_DETACHABLE_COOKED_APPAREL` | 없음 | 1 | shoulder 1 |
| 도화가 | `READY_ALTERNATIVE` | `APPAREL_HEAD` | 1 | shoulder 1 |
| 창술사 | `READY_ALTERNATIVE` | `APPAREL_HEAD`, `APPAREL_OUTFIT` | 1 | shoulder 1 |
| 워로드 | `BASELINE_PART_ONLY` | `APPAREL_SHOULDER` | 1 | shoulder 1 |
| 슬레이어 | `READY_ALTERNATIVE` | `APPAREL_HEAD` | 1 | shoulder 1 |
| 건슬링어 | `READY_ALTERNATIVE` | `APPAREL_HEAD`, `APPAREL_UPPER` | 0 | shoulder 1 |

| 클래스 | 카테고리 | 상태 | 대표 visual set | 파츠 |
|---|---|---|---|---:|
| 창술사 | `APPAREL_HEAD` | `READY_ALTERNATIVE` | `character.lance_master.mokoko_pleasant.head` | 1 |
| 창술사 | `APPAREL_OUTFIT` | `READY_ALTERNATIVE` | `character.lance_master.mokoko_pleasant.outfit` | 1 |
| 건슬링어 | `APPAREL_HEAD` | `READY_ALTERNATIVE` | `character.gunslinger.default_variant_01.head` | 1 |
| 건슬링어 | `APPAREL_UPPER` | `READY_ALTERNATIVE` | `character.gunslinger.default_variant_01.upper` | 1 |
| 슬레이어 | `APPAREL_HEAD` | `READY_ALTERNATIVE` | `character.slayer.default_variant_01.head` | 1 |
| 도화가 | `APPAREL_HEAD` | `READY_ALTERNATIVE` | `character.artist.default_variant_01.head` | 1 |
| 워로드 | `APPAREL_SHOULDER` | `BASELINE_PART` | `character.warlord.default_00.shoulder` | 1 |
| 창술사 | `WEAPON_SET` | `BASELINE_WEAPON` | `character.lance_master.wp_wflm_00` | 2 |
| 슬레이어 | `WEAPON_SET` | `BASELINE_WEAPON` | `character.slayer.wp_wwbk_03` | 1 |
| 도화가 | `WEAPON_SET` | `BASELINE_WEAPON` | `character.artist.wp_wsdm_09` | 1 |
| 워로드 | `WEAPON_SET` | `BASELINE_WEAPON` | `character.warlord.wp_wwgl_04` | 2 |
| 차원술사 | `WEAPON_SET` | `BASELINE_WEAPON` | `character.dimensionmaster.wp_wswp_m_06` | 4 |

skinned 7 part는 positive-weight bone index/name과 inverse bind가 현재 class body와 일치했고 최대 delta는
0이었다. socketed weapon 10 part는 skeleton/animation을 포함하지 않으며 필요한 body socket이 실제로
존재했다. WModel 내부 material을 읽어 참조된 texture만 closure에 포함했다.

이 결과는 “모코코와 같은 제품 호환성 기준”으로 기존 Cook 자산을 재검증하고 별도 target ID로 묶은
admission staging이다. raw object와 existing WModel은 각각 검증한 뒤 사람이 저작한 association으로
묶었지만, 과거 Cook receipt나 geometry derivation이 없어 exact build provenance는 증명하지 않는다.
신규 raw glTF를 master skeleton으로 재바인딩해 Cook한 결과도 아니다.
대표 pack 생성 시점에는 `Client/Bin/Resources`, `Data/Actors/CharacterCatalog.json`, 제품 item catalog를
변경하지 않았다. 후속 Equipment authoring 구현에서 별도 `EquipmentPresentationCatalog.json`과
`EquipmentLoadoutPresets.json`을 추가했으며, 기존 `CharacterCatalog.json`과 item catalog는 그대로다.

### 9.2 다음 신규 외형 raw 후보

새 의상 파츠로 이어갈 첫 후보는 class마다 shoulder 하나로 고정했다.

| 클래스 | exact source object | raw joints / body | 현재 상태 |
|---|---|---:|---|
| 창술사 | `PC_FLM_01.pc_flm_01_shoulder_sk` | 207 / 224 | raw 완전, WModel 없음 |
| 건슬링어 | `PC_GDH_F_01.pc_gdh_f_01_shoulder_sk` | 236 / 239 | raw 완전, WModel 없음 |
| 슬레이어 | `PC_WBK_F_01.pc_wbk_f_01_shoulder_sk` | 238 / 245 | raw 완전, WModel 없음 |
| 도화가 | `PC_SDM_01.pc_sdm_01_shoulder_sk` | 236 / 239 | raw 완전, WModel 없음 |
| 워로드 | `PC_WGL_01.pc_wgl_01_shoulder_sk` | 215 / 218 | raw 완전, WModel 없음 |
| 차원술사 | `PC_SWP_M_02.pc_swp_m_02_shoulder_sk` | 159 / 225 | raw 완전, WModel 없음·combined body 제약 |

모두 1 mesh/1 primitive/1 material이고 raw joint 이름은 body의 subset이며 unknown/duplicate joint는 0이다.
glTF, buffer, material props, 참조 DDS도 존재한다. 그러나 현재 저장소와 PC에는 모코코 때 사용한
master-skeleton normalizer가 없어 직접 `ModelAssetConverter`로 Cook하면 palette index가 달라질 수 있다.
따라서 이 여섯 후보는 pack의 `Source/candidate.*`에 glTF/buffer를 함께 산출하되
`RAW_EXTRACTED_NOT_EQUIP_READY`와 `MASTER_SKELETON_NORMALIZATION_REQUIRED`로 유지하고 Resources에는 넣지
않았다. 차원술사는 `COMBINED_BODY_COVERAGE_REQUIRED`도 함께 기록했다.

### 9.3 현재 저장 위치와 회원정보 경계

이번 대표 팩은 위 `out/.../Admission/representative-20260901-curated`에만 저장돼 있다. 회원정보나 Server DB에
저장된 것은 없다. 현재 코드는 접속마다 process-local `sessionId`를 만들고, class/nickname도 process
memory로만 보존한다. `CServerPlayer` inventory도 in-memory `itemId + quantity` vector라 퇴장 또는 Server
재시작 뒤 사라지며, nickname은 identity가 아니다.

계획한 durable 저장은 인증된 `accountId + characterId`를 key로 하고 inventory item instance,
class별 slot loadout, pending raid reward, revision을 SQLite transaction 하나로 저장한다. 첫 물리 기본값은
repository/Data/Resources 밖인 `C:\ProgramData\LostArkTeam\Server\CharacterState.sqlite3`이다. 실제 저장값은
`itemId`, `itemInstanceId`, stable slot이고 model path나 `visualSetId`는 저장하지 않는다. 현재는 인증
provider와 repository 모두 미구현이므로 이 경로에 DB 파일을 생성하지 않았다.

## 10. 자동 검증

실행 완료:

```text
New-CharacterEquipmentSourceInventory.ps1
  SourceObjects=7475, Extracted=7464, Blocked=11, VisualSets=2159

generated JSON parse
  extraction receipt OK
  source inventory OK
  visual sets OK
  entries=7475, admitted=0

CSV row count
  7476 lines = header + 7475 entries

Windows PowerShell 재실행
  SourceObjects=7475, Extracted=7464, Blocked=11, VisualSets=2159

Publish-ItemCatalog.ps1 -Mode Validate
  Item catalog Validate succeeded: 13 items.

Publish-ValtanClearRewards.ps1 -Mode Validate
  Valtan clear rewards Validate succeeded: 6 items.

representative pack unit tests
  19개 중 18 PASS, Windows symlink 권한 fixture 1건 skipped
  Admission root 밖 output/overwrite 요청 거부 PASS
  nested output, case-only target collision, legacy WMSH flag/index mismatch 거부 PASS
  기존 output 재실행은 copy 전에 exit 1, 새 work directory 0 PASS

New-CharacterEquipmentRepresentativePack.ps1
  classes=6, sets=12, parts=17, closure=73
  state=STAGED_VALIDATED_NOT_PROMOTED
  sourceAssociation=CURATED_NOT_BUILD_PROVENANCE

representative receipt/file audit
  121 files, 86,928,104 bytes
  runtime closure 73 + source closure 46 hash 일치
  skinned 7 part weighted palette delta 0
  socketed 10 part socket/no-skeleton/no-animation PASS
  선택된 body/part WModel unique 23개 legacy header/range parse PASS

C++ runtime decoder isolated diagnostic
  representative Runtime의 WModel 17개 모두 decode traversal 완료, 개별 rejection 0
  static=10, skinned=7, hasBounds=17, multiSubmesh=6
  harness가 전체 Resources 고정 분모 2,586개를 요구하므로 isolated corpus valid=0/exit 1이며 PASS로 기록하지 않음

Resources top-level audit
  Character, Deploy, Effect, Fonts, Map, Sound, UI 정확히 7개
  Character/<Class>/Equipment product directory 6개 생성, raw Source directory 0개

git diff --check
  exit 0

untracked authored file whitespace audit
  대표 selection/tool/test/PLAN/RESULT trailing whitespace 0, final newline PASS

Equipment catalog/preset focused tests
  3 PASS
  catalog schema/version, 6 class, 12 set, 17 part, stable primary/occupied slot PASS
  Logic_* 정본 socket/yaw와 창술사 stance, 도화가 hiddenMeshMask=2 PASS
  authoringOnly preset의 class/slot/reference/점유 충돌 검증 PASS

Character equipment publisher focused tests
  7 PASS
  runtime closure 73개 hash/size, Resources-relative Character/... 안전 경로 PASS
  Resources 최상위 7폴더 계약, target collision 선검증 PASS
  동일 hash 재실행 no-op, raw Source 복사 금지 PASS
  두 번째 디렉터리 promote 중간 실패 주입 후 첫 promote rollback PASS

CharacterEquipmentPipeline 전체 unittest discovery
  40개 중 39 PASS, Windows symlink 권한 fixture 1건 skipped

Equipment Authoring Tool/runtime focused contract
  11 PASS
  six-class authored default slot mapping과 occupied-slot mask 전달 PASS
  HEAD-only/WEAPON-only/compound outfit의 비점유 기본 파츠 보존 계약 PASS
  preset/catalog visual-first reload와 실패 시 last-good draft/visual 보존 계약 PASS
  공용 preview 재사용, F1 explicit owner, 프로젝트/필터/Data 등록 PASS

실제 정본 경로 Publish 사전 검증
  AdmissionRoot=C:\Users\user\Desktop\LostArk\out\CharacterEquipmentExtraction\Admission\representative-20260901-curated
  ResourceRoot=C:\Users\user\Desktop\LostArk\Client\Bin\Resources
  RuntimeClosureFiles=73, EquipmentDirectories=6
  NewEquipmentDirectories=6, ExistingIdenticalEquipmentDirectories=0
  Result=VALIDATED, Resources mutation=0

실제 정본 Resources Publish와 사후 검증
  Publish RuntimeClosureFiles=73, EquipmentDirectories=6
  새 Character/<Class>/Equipment 디렉터리 6개 promote PASS
  post-Validate NewEquipmentDirectories=0
  post-Validate ExistingIdenticalEquipmentDirectories=6
  다른 hash collision=0, raw Source publish=0
```

Engine Debug x64 focused build와 Client Debug x64 compile/link는 성공했고 `Client.exe`를 생성했다.
최신 `origin/main` rebase 뒤 정본 명령
`Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`도 exit 0으로 완료했다. build-domain
gate와 Map/World/Navigation/Destruction/Balance/Item/Reward publisher, Engine/Shared/Server/Client compile/link,
Product compiled-shader closure가 모두 PASS했다. WARP readback은 V1/V2 각각 1,352 pixels였고 제품 build
receipt와 run evidence를 생성했다. 컴파일 오류는 없었으며 기존 C4819/C4828 인코딩 및 DirectXTK PDB
경고만 남았다.

## 11. 수동 검증 상태

- Client 실행: 미실행
- UI 조작: 미실행
- 스크린샷 생성: 미실행
- visual fidelity: 미판정
- manual first pixel / visual PASS: 사용자 판정 필요

현재 LAN sync는 이 PC를 `server-host`로 판정했지만 TCP 7777 LocalSubnet 방화벽 규칙이 없거나 오래돼
관리자 PowerShell 실행이 필요하다. 코드·데이터 검증은 계속했지만 Client를 자율 실행하지 않았다.

## 12. 다음 구현 시작점

다음 G는 계획서의 G00이다.

1. class master skeleton normalizer와 실패 harness를 만든다.
2. 위 shoulder 6개를 정규화 Cook하고 기존 대표 staging과 같은 admission receipt를 만든다.
3. 차원술사 base body/garment를 분리한다.
4. item 정의, item instance, random reward, equip/unequip protocol과 atomic presentation 교체를 연결한다.
5. 사용자가 Equipment Tool의 class/slot별 장착·해제 결과를 직접 육안 판정한다.
6. 인증된 account identity가 정해지면 durable repository와 restart/reconnect 복원을 닫는다.

11개 blocked object는 parser가 갱신될 때까지 별도 backlog로 유지한다. 다른 object로 대체하거나 누락을
성공 처리하지 않는다.
