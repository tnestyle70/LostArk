# 발탄 루가루·일반 몬스터 원본 리소스 추출 결과

## 1. 완료 상태

원본 NPC DB와 LookInfo가 직접 연결하는 네 NPC를 추출·쿠킹하고 다음 런타임 폴더에 설치했다.

```text
Client/Bin/Resources/Character/Monster/
  NPC_480001_MN_PADD_01/
  NPC_480002_MN_SJFC_00_4/
  NPC_480003_MN_0019_05/
  NPC_480005_MN_RPRS_02/
```

설치본에는 `.wmodel` 4개와 각 WModel이 실제 참조하는 DDS 18개만 있다. UPK, PSK, PSA, FBX, Blend, 조사 로그는 런타임 폴더에 넣지 않았다.

## 2. 대상 확정 근거

`EFTable_Npc.db`의 `ClassifyIndex=3705100` 행과 각 LookInfo를 다음 순서로 추적했다.

```text
NPC ID
-> Model / OriginalActionObjectGroupName
-> LookInfo SkeletalMesh / AnimSet / MaterialInstance
-> name-resolved physical UPK
-> UModel group/object
-> MaterialInstance .props.txt texture parameter
```

| 역할 | NPC ID | DB Model | LookInfo SkeletalMesh | LookInfo AnimSet |
|---|---:|---|---|---|
| 일반 몬스터 | 480001 | `EFDLChar_MN_PADD_01.MN_PADD_01` | `MN_PADD_01.Mesh.MN_PADD_01_SK` | `MN_PADD_00.Ani.MN_PADD_00_Ani` |
| 일반 몬스터 | 480002 | `EFDLChar_MN_SJFC_00-4.MN_SJFC_00-4` | `MN_SJFC_00.Mesh.MN_SJFC_00_SK` | `MN_SJFC_00.Ani.MN_SJFC_00_Ani` |
| 일반 몬스터 | 480003 | `EFDLChar_MN_0019_05.MN_0019_05` | `MN_0019_05.Mesh.MN_0019_05_SK` | `MN_0019_00.Ani.MN_0019_00_Ani` |
| 통솔자 루가루 | 480005 | `EFDLChar_MN_RPRS_02.MN_RPRS_02` | `MN_RPRS_02.Mesh.MN_RPRS_02_SK` | `MN_RPRS_00.Ani.MN_RPRS_00_Ani`, `MN_RPRS_00.Ani.MN_RPRS_00_Evt2_Ani` |

`480001~480003`은 같은 발탄 분류에서 루가루 앞에 연속으로 정의된 `Grade=2` 렌더 가능 NPC다. `480004`는 `Grade=5` 엘리트, `480006`은 루카스이므로 제외했다. 이름만 비슷한 `MN_RPBF_02` 계열도 채택하지 않았다.

name-resolve로 확정한 물리 패키지는 다음과 같다.

| 논리 패키지 | 물리 UPK |
|---|---|
| `MN_PADD_01` | `9G1MUXII1BZ6E7TEHH64S5P.upk` |
| `MN_PADD_00` | `9G1MUXII1BZZE7TEHH64SSP.upk` |
| `MN_SJFC_00-4` | `AH2NGPXC2C0023SF0VFCEC00.upk` |
| `MN_SJFC_00` | `9G1MFOWB1BZZE7WN7464SSP.upk` |
| `MN_0019_05` | `9G1MZZ6Q1BZYE7SS5164SLP.upk` |
| `MN_0019_00` | `9G1MZZ6Q1BZZE7SS5164SSP.upk` |
| `MN_RPRS_02` | `9G1M8U8F1BZDE7JTJW64SIP.upk` |
| `MN_RPRS_00` | `9G1M8U8F1BZZE7JTJW64SSP.upk` |

## 3. 정확한 머티리얼 연결

파일 이름을 추측하지 않고 메시 슬롯과 MaterialInstance `.props.txt`가 가리키는 텍스처만 remap했다.

| 대상 | 머티리얼 슬롯 | Base | Normal | Specular | Emissive |
|---|---|---|---|---|---|
| 480001 | `mn_padd_01_mi` | `mn_padd_00_d_loc_int.dds` | `mn_padd_00_n.dds` | 없음 | 없음 |
| 480001 | `mn_padd_01-1_mi` | `mn_padd_01_d.dds` | `mn_padd_01_n.dds` | 없음 | 없음 |
| 480002 | `mn_sjfc_00_mi` | `mn_sjfc_00_d.dds` | `mn_sjfc_00_n.dds` | `mn_sjfc_00_s.dds` | 없음 |
| 480003 | `mn_0019_05_mi` | `mn_0019_05_d_loc_int.dds` | `mn_0019_05_n.dds` | `mn_0019_05_s.dds` | 없음 |
| 480005 | `mn_rprs_02_mi` | `mn_rprs_02_d.dds` | `mn_rprs_02_n.dds` | `mn_rprs_02_s.dds` | `mn_rprs_02_e.dds` |
| 480005 | `mn_rprs_02-1_mi` | `mn_rprs_02-1_d.dds` | `mn_rprs_02-1_n.dds` | `mn_rprs_02-1_s.dds` | `mn_rprs_02-1_e.dds` |

원본에는 일부 Color Mask 또는 변형 Mask도 있지만 현재 WMA2/CMaterial의 해당 명시 슬롯이 없으므로 임의로 다른 슬롯에 넣지 않았다.

## 4. ActorX 처리와 보존 경계

UModel은 `-nameresolve -groups`와 exact package/group/object로 실행했다. ActorX는 다음 경로로 변환했다.

```text
exact PSK + exact PSA
-> Blender 3.0 ActorX importer
-> FBX
-> ModelAssetConverter
-> WModel
```

Skeletal 모델이므로 정적 메시용 `--pretransform`은 사용하지 않았다.

### 4.1 공용 AnimSet의 bone order

`MN_PADD_01 <- MN_PADD_00`과 `MN_RPRS_02 <- MN_RPRS_00`은 LookInfo가 명시한 정확한 조합이지만 PSK와 PSA의 bone 배열 순서가 달랐다. 두 경우 모두 bone 이름 집합과 개수는 완전히 같았다.

- PADD: 85개 bone, 동일 이름 집합, 배열 위치 83개 차이
- RPRS: 69개 bone, 동일 이름 집합, 배열 위치 11개 차이

동일한 exact bone 이름으로 PSA의 BONENAMES와 각 프레임의 Position/Rotation/Scale key 순서만 PSK 순서로 재배열했다. 각 bone의 원본 key byte가 재배열 후에도 동일함을 전 프레임 비교로 검증했다. 이름 유사 매핑이나 보간은 사용하지 않았다.

### 4.2 제외한 원본 sequence

한 FBX의 Scene FPS를 하나로 유지하고 원본 timing을 변조하지 않기 위해 실제 30fps가 아닌 6개 sequence를 제외했다.

| 대상 | 제외 sequence | 원본 FPS | 이유 |
|---|---|---:|---|
| 480001 | `run_battle_2` | 28.548387527 | 혼합 FPS |
| 480001 | `abn_earthquake_1` | 28.695652008 | 혼합 FPS |
| 480002 | `idle_battle_1` | 28.714286804 | 혼합 FPS |
| 480002 | `twistknockdown_land` | 28.064516068 | 혼합 FPS |
| 480003 | `att_battle_2_01` | 28.888889313 | 혼합 FPS |
| 480005 | `evt1_att_battle_05_01` | 28.790323257 | 혼합 FPS |

또한 공용 PSA를 외형 변형 PSK 순서로 맞춘 뒤 FBX가 단위 스케일 골반 채널 양끝에 2개 key를 추가한 두 보조 sequence는 strict key-count 검증을 통과시키기 위해 제외했다.

| 대상 | 제외 sequence | 원본 key / 변환 key | 값 범위 |
|---|---|---|---|
| 480001 | `standup_1` | 51 / 53 | 1.0에서 약 `0.0000002` 이내 |
| 480005 | `idle_normal_1_2` | 104 / 106 | 1.0에서 약 `0.0000003` 이내 |

원본 PSK/PSA는 `C:\LostArkExtract\MonsterSource_20260806`에 보존했다. 필수 idle, run, walk, attack, damage, dead와 루가루 `evt2_jump_1`은 최종 WModel에 포함된다.

## 5. 설치 결과

| Asset | 파일 수 | DDS | Animation | WModel bytes | WModel SHA-256 |
|---|---:|---:|---:|---:|---|
| `NPC_480001_MN_PADD_01` | 5 | 4 | 36 | 9,931,328 | `1D6AE04C1F8E0483B55414F144A274F7B902A8B3B40A6832D4C09C081BEC650E` |
| `NPC_480002_MN_SJFC_00_4` | 4 | 3 | 29 | 4,578,766 | `F8D5462633FB1D10EABA7791ABE10F0E72ACE351A156641CAB28DE2649E6E93F` |
| `NPC_480003_MN_0019_05` | 4 | 3 | 25 | 4,125,952 | `855F9DA56A579C4FD403BF260A5493A14B8C02C85CA883DAFF6CE79172D310A0` |
| `NPC_480005_MN_RPRS_02` | 9 | 8 | 91 | 21,196,924 | `F25B512571D3B38D1314459361F4809286E85F4563BC1B7C6DAB320D7FD7AE8D` |

총 22개 파일이며 전체 런타임 payload 크기는 46,913,162 bytes다.

## 6. 검증 결과

- 네 WModel 모두 `skeleton=yes`다.
- WModel animation 수와 Blender Action 수가 각각 `36/29/25/91`로 일치한다.
- PSK/PSA bone 이름·개수·순서 계약을 검증했다.
- 포함 sequence의 FPS는 모두 정확히 30.0이다.
- 포함 sequence의 frame count, Position/Rotation/Scale key와 bone 대응을 검증했다.
- 모든 명시적 texture remap은 원본 DDS와 설치 DDS의 SHA-256이 같다.
- WModel 텍스처 참조는 `textures/<file>.dds` 상대 경로다.
- 설치본과 외부 검증 패키지의 WModel/DDS SHA-256이 모두 같다.
- 런타임 네 폴더에는 `.wmodel`과 `.dds` 외 확장자가 없다.
- 네 모델의 `idle_normal_1` 시작·중간·끝 프레임을 렌더해 메시가 분리되거나 bone이 폭발하지 않는 것을 육안 확인했다.

프리뷰는 다음 외부 폴더에 보존했다.

```text
C:\LostArkExtract\MonsterPreview_20260806
```

## 7. 남은 경계

이번 작업은 정확한 원본 렌더 payload 확보까지만 완료했다. 다음 항목은 아직 구현하지 않았다.

- Monster 제품 runtime/catalog/schema
- Server authority spawn, 이동, 전투 AI와 stage trigger 연동
- MapTool의 실제 일반 몬스터·루가루 archetype 선택과 배치
- 원본 effect, sound, skill action 연결
- 제외한 혼합 FPS 및 FBX 재샘플 sequence를 한 WModel에 손실 없이 넣는 converter 확장

따라서 폴더에 모델이 설치됐다는 사실만으로 Server gameplay에서 자동 생성되지는 않는다. 후속 수직 슬라이스는 별도 PLAN과 harness 승인 후 진행해야 한다.
