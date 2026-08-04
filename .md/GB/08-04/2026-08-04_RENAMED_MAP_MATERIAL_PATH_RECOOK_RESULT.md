# 2026-08-04 renamed map material path recook RESULT

## 결과

Character Select와 Training 맵의 WModel material path를 최종 runtime namespace 기준으로
재쿠킹했다. 사용자가 제시한 foliage WModel의 base texture는 이제 다음 경로를 저장한다.

```text
Resource/Map/CHARACTERSELECTMAP/MAP_0B39DAEA62CA_LV_ATM_LOGHILL_FOLIAGE02_SM/textures/bg_anh_foliage_briwreath01_da_ksy.dds
```

## 원인

최초 추출은 원본 Area ID로 정상 수행됐지만, 이후 runtime 폴더를 직관적인 이름으로 바꿀 때
WModel 내부 material path를 다시 굽지 않았다. `CMaterial`은 존재하지 않는 이전 Area ID의
texture를 로드하다 실패했고 Loader worker에서 modal message가 표시됐다.

## 반영

- `CHARACTERSELECTMAP`: 55개 WModel, 167개 texture reference 재쿠킹
- `TRAININGMAP`: 302개 WModel, 752개 texture reference 재쿠킹
- 합계: 357개 WModel, 919개 texture reference
- `CMaterial` texture 실패 시 실제 경로를 debugger에 출력하고 원래 HRESULT를 보존
- Loader worker에서 `CMaterial` 생성 실패 modal 제거
- ProjectAudit에 WModel 내부 texture namespace, 경로 탈출, 실제 파일 존재 검사 추가

원본 WModel backup은 다음 로컬 경로에 보존했다.

```text
C:/Users/user/Desktop/LostArk/.codex_tmp/MapMaterialBackups-2c81522d0d6341a3a9c226bb6c6b868a
```

## DataFiles 계약

새 DataFiles 문서는 필요하지 않다. 다음 기존 문서가 원본 Area ID를 유지하며 최종 runtime
namespace의 model path를 가리킨다.

```text
LV_LOBBY_CLASSSELECT_SL00.mapassets      55 rows
LV_LOBBY_CLASSSELECT_SL00.mapplacements 803 rows
LV_SHS_RCARENA_D.mapassets              302 rows
LV_SHS_RCARENA_D.mapplacements          7856 rows
```

## 검증

- staged recook: 357/357 PASS
- material texture reference: 919/919 exact namespace and file resolution PASS
- Debug Engine/Server/Client build and all contract harnesses PASS
- Release Engine/Server/Client build and all contract harnesses PASS
- ProjectAudit map/loader checks PASS
- 전체 ProjectAudit 종료 코드는 폐기 대상으로 정리한 기존 asset-pack lock의 stale inventory
  한 건 때문에 FAIL: actual `10158 files / 5393658360 bytes`
- 실제 Lobby -> Character Select 및 Test 진입 수동 화면 확인은 사용자 실행 확인이 남아 있다.

## Valtan 회색 fallback 추가 복구

같은 canonical material path 계약으로 `LV_LUT_HEARTRB_ED`의 회색 fallback 자산 18종을
추가 복구했다. 파일명 유사도로 추측하지 않고 UModel이 추출한 Material Instance
`.props.txt`의 diffuse, normal, specular 슬롯을 모델별로 읽어 재쿠킹했다.

- OCASTLE tile floor: 5종
- PAP common tree root: 6종
- LUCASTLE street floor: 4종
- ATT common floor: 2종
- KARLAJAVIL floor vent: 1종
- 결과: WModel 18개, DDS 72개, 총 90개 runtime payload 파일

모든 texture reference는 다음 계약으로 저장했다.

```text
Resource/Map/LV_LUT_HEARTRB_ED/<AssetId>/textures/<Texture>.dds
```

18개 WModel의 WMSH geometry section은 교체 전과 SHA-256이 같고, 정점·인덱스·스케일은
바뀌지 않았다. 설치 뒤 `floor` 또는 `treeroot` 이름을 가진 explicit fallback 모델은
0개다. 교체 전 자산은 다음 로컬 경로에 보존했다.

```text
.codex_tmp/map_material_backup_20260804_019fad33
C:/_tex/backup_before_install
```

이 90개 파일은 `Client/Bin/Resources`의 Git 제외 payload다. 현재
`Data/AssetPacks.lock.json`이 가리키는 immutable pack에는 아직 포함하지 않았으며,
다른 로컬 payload와 분리해 `Snapshot -> Verify -> Publish -> Hydrate`를 완료하기 전에는
팀 배포 완료로 처리하지 않는다.
