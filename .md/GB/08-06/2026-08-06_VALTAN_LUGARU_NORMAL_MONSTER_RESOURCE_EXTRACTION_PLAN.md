# 발탄 루가루·일반 몬스터 원본 리소스 추출 계획

## 1. 목표

원본 `EFTable_Npc.db`와 각 LookInfo가 직접 가리키는 리소스만 사용해 다음 네 NPC의 런타임 모델을 만든다.

| 역할 | NPC ID | Model | Action group |
|---|---:|---|---|
| 일반 몬스터 | 480001 | `EFDLChar_MN_PADD_01.MN_PADD_01` | `MN_PADD_00` |
| 일반 몬스터 | 480002 | `EFDLChar_MN_SJFC_00-4.MN_SJFC_00-4` | `MN_SJFC_00-4` |
| 일반 몬스터 | 480003 | `EFDLChar_MN_0019_05.MN_0019_05` | `MN_0019_00` |
| 통솔자 루가루 | 480005 | `EFDLChar_MN_RPRS_02.MN_RPRS_02` | `MN_RPRS_00` |

`480001~480003`은 발탄 콘텐츠 `ClassifyIndex=3705100`에서 루가루 앞에 연속으로 정의되고 `Grade=2`인 렌더 가능한 일반 몬스터다. `480004`는 `Grade=5` 엘리트, `480006`은 다른 NPC인 루카스이므로 이번 범위에서 제외한다. `MN_RPBF_02`처럼 이름만 비슷한 발탄 이펙트·모델도 근거로 사용하지 않는다.

## 2. 정본 추적 규칙

각 대상은 다음 연결이 모두 성립해야 채택한다.

```text
EFTable_Npc ID
-> Model / OriginalActionObjectGroupName
-> LookInfo의 SkeletalMesh / AnimSet / MaterialInstance
-> UModel package / group / object
-> MaterialInstance .props.txt의 실제 texture parameter
-> ActorX PSK/PSA
-> ModelAssetConverter .wmodel
```

- 이름 유사도나 파일명 추측으로 메시·머티리얼·텍스처를 연결하지 않는다.
- UModel은 `-nameresolve`와 `-groups`를 사용해 원본 package/group/object 계층을 보존한다.
- MaterialInstance의 Diffuse/Normal/Specular/Emissive 슬롯은 `.props.txt` 근거가 있을 때만 remap한다.
- Skeletal ActorX는 Blender 중간 변환을 사용하고 정적 메시용 `--pretransform`을 사용하지 않는다.
- 원본 UPK, PSK, PSA, FBX, Blend와 조사 로그는 `.codex_tmp` staging에만 둔다.

## 3. 런타임 설치 계약

검증을 통과한 파일만 다음 폴더에 둔다.

```text
Client/Bin/Resources/Character/Monster/
  NPC_480001_MN_PADD_01/
  NPC_480002_MN_SJFC_00_4/
  NPC_480003_MN_0019_05/
  NPC_480005_MN_RPRS_02/
```

각 폴더에는 `.wmodel`과 그 모델이 실제 참조하는 `textures/*.dds`만 포함한다. 원본 조사 파일과 변환 중간 파일은 넣지 않는다.

## 4. 검증

1. NPC DB 행과 LookInfo의 exact reference를 결과 문서에 기록한다.
2. UModel 출력이 exact group/object를 포함하는지 확인한다.
3. ActorX 변환 시 PSK/PSA skeleton compatibility와 animation clip 수를 확인한다.
4. `ModelAssetConverter --info`로 모델 형식·본·애니메이션·머티리얼을 확인한다.
5. `.wmodel`의 모든 텍스처 참조가 설치 폴더 안의 실제 DDS로 해소되는지 검사한다.
6. 런타임 폴더에 raw source나 추측으로 채택한 리소스가 없는지 검사한다.

## 5. 이번 범위 밖

- Monster 제품 런타임/catalog/schema와 Server authority 구현
- NPC 배치 및 전투 AI
- 루가루의 이펙트·사운드·스킬 연동
- 발탄 보스 또는 루카스 리소스 재추출

이번 작업은 정확한 원본 리소스 payload 확보까지만 완료한다.
