# 2026-09-26 마하라카 source material build 실행 결과

`LV_OCN_EVENTIS_MHP`의 source material build를 실행했다. **G4 보존 관문에서 멈췄고 정본에
쓰지 않았다.** build 자체는 완주했으나, 산출물이 이미 존재하는 행을 재생산할 뿐 새 material
정의를 만들지 않으며, 그대로 적용하면 물 10행을 잃는다.

## 0. 한 줄 결론

**이 Area의 source material build는 내용 기준으로 no-op이다.** 컴파일 출력 319행이 전부
현재 329행과 바이트 동일하고 신규 0행이며, 만들지 못한 10행이 정확히 물 10행이다.
조율자의 전제였던 "이 build가 382행의 material 정의를 만들고 530 variant를 같은
transaction에서 admit한다"는 **성립하지 않는다.**

## 1. 관문 결과

| 관문 | 결과 | 수치 |
|---|---|---|
| G1 전체 백업 | **통과** | 15파일, sha256 목록 기록 |
| G2 입력 준비 | **통과** | 자산 241/382 바인딩, 슬롯 319, 텍스처 361, propertyError 0 |
| G3 compiler 실행 | **통과** | 319 슬롯 staged, exit 0 |
| G4 보존 대조 | **실패 → 정지** | 출력 319행 = 기존 행과 100% 동일, 신규 0, 누락 10(= 물 10행) |
| G5 정본 반영·게시 | **미실행** | G4 미통과로 지시대로 중단 |
| G6 사후 검증 | **미실행** | 동일 |

## 2. 입력이 이미 존재했다 — 09-19 staging

`rebuild_all.sh`(`C:/LostArkExtract/LV_OCN_EVENTIS_MHP_20260919/Restore/`)를 읽어 확인했다.
그 스크립트는 A install → B scene → C landscape merge → D water → E 문서 복사로 끝나며
**source material build 단계가 아예 없다.** `sourceMaterialBuild: null`은 사고가 아니라
09-19 파이프라인이 그 단계를 포함하지 않은 결과다.

그런데 그 단계에 필요한 입력 7개는 **전부 그 staging에 있었다.**

| 어댑터 인자 | 실제 파일 | 크기 |
|---|---|---|
| `--parameters` | `Restore/params/parameters.json` | 28 MB |
| `--runtime-manifest` | `MHPr2/manifests/map_material_runtime_assets.json` | 1.9 MB |
| `--mip-catalog` | `Restore/recook/admitted.mip-catalog.json` | — |
| `--mip-results` | `Restore/mips/batch.results.json` | 108 KB |
| `--mip-chains` | `Restore/mips/chains/` | 421 MB, receipt 1,122개 |
| `--asset-inventory` | `admitted.inventory.json` | 396 KB |
| `--imported-catalog` | `Imported/LV_OCN_EVENTIS_MHP.mapassets` | — |

즉 mip 체인은 09-19에 이미 복구돼 있었다(README: 561 텍스처 복구, DDS 1,342개가 원본 체인 보유).
직전 작업이 "mip 체인 미복구"라고 적은 것은 **lightmap 136개**에 대한 것이고, material
텍스처의 체인은 별건으로 이미 존재한다.

## 3. G2 — 어댑터 결과

```
assets bound: 241 of 382 (partial 17 left on the legacy path, geometry-dropped 2);
slots 319; textures 361
```

141개가 legacy 경로에 남았고 이유가 `slot_report.json`에 있다. 상위 이유는
foliage vertex wind 바인딩 미검증 41, `bg_base_trn_depthtest` 미지원 terminal 17,
`texture_normal` NULL 16, `specialresource.mat.ocean_trn` 미지원 terminal 12,
정규화되지 않은 static switch 등이다.

**어댑터는 조명을 전혀 싣지 않는다.** 실측으로 확인했다 — 출력의 `placementLighting`이
**0행**이고 모든 슬롯의 `lightingEvidence`가 **`source-absent`** 하나뿐이다. 이는 그 파일
`:259`·`:297`의 하드코딩과 정확히 일치한다. 지시대로 원본(미추적 타 세션 파일, sha256
`f7dd9bc2635ce0c8`)은 고치지 않았다.

## 4. G4 — 멈춘 이유 (핵심)

컴파일 출력과 현재 정본을 `(assetId, materialName)`으로 대조했다.

| 항목 | 값 |
|---|---|
| 컴파일 출력 행 | **319** |
| 현재 정본 행 | 329 |
| 기존 329행 중 출력에 존재 | **319** |
| 출력에만 존재(신규) | **0** |
| 기존에만 존재(출력이 못 만듦) | **10** |
| 겹치는 319행의 값 완전 동일 | **319 / 319** |
| 값이 다른 행 | **0** |

그리고 만들지 못한 10행이 **정확히 water family 10행**이다(출력에 0/10 존재).

해석은 이렇다. 현재 329행 = 컴파일러가 재생산 가능한 **319행** + 손으로 적용한 **물 10행**.
즉 이 build는 이미 디스크에 있는 것을 다시 만든다. `sourceMaterialBuild: null`은
**내용 공백이 아니라 provenance 공백**이었다.

그대로 정본에 쓰면 329 → 319가 되어 물 10행이 사라진다. G4 조건("기존 329행이 값까지
전부 살아 있어야 한다")을 만족하지 못하므로 정지했다.

## 5. 조명은 왜 여전히 막히는가

| 항목 | 값 |
|---|---|
| 조명 payload 고유 `assetId` | 990 |
| 기존 + 어댑터 바인딩으로 커버 | **78 (4.8%)** |
| 여전히 material 행 없음 | **912** |
| 그중 `_RNM_` 접미 | **883** |
| 그중 `.mapassets` catalog가 admit함 | **29** |
| 그중 catalog에도 없음 | **883** |
| 어댑터 슬롯에 포함된 `_RNM_` ID | **0** |

진짜 병목은 source material build가 아니다. **`.mapassets` catalog가 883개 `_RNM_<hex>`
variant ID를 admit해야 한다.** 예:
`MAP_02CF669C500B_BG_PAP_ETC_FESTIVALFOOD01_SM_ARTREE_OVR_CE15388F52A1_RNM_1DBD1EDADF2B`.

그 admission은 `build_maptool_scene.py`의 일이고 컴파일러의 일이 아니다. 컴파일러는
이미 존재하는 자산의 재질을 컴파일하며 variant catalog 항목을 만들 수 없다. 어댑터 입력
(`admitted.inventory.json`, runtime manifest)에 `_RNM_` ID가 하나도 없다는 것이 그 증거다.

조명을 4.8%에서 올리려면 선결 조건이 **`_RNM_` variant 883개의 catalog admission**이며,
그 과정에서 물 10행 보존 문제(`render_profile_text()`가 `Water`를 못 만든다)를 함께 풀어야 한다.

## 6. 실행 상태 구분

- **확인한 원본:** 09-19 `rebuild_all.sh` 단계 구성, 입력 7개 실재, mip chain receipt 1,122개
- **실제 구현:** 어댑터·컴파일러 실행 스크립트. **도구·제품 코드 무변경**
- **live 설치:** 없음. `Client/Bin/Resources`에 한 바이트도 쓰지 않았다
- **게시:** **없음.** publisher를 한 번도 실행하지 않았다
- **빌드/자동검사:** 빌드 0회. 회귀 3종 **전부 exit 0**(20 / 10 / 13 tests)
- **사용자 수동확인:** 없음. 화면에 나타난 변경이 없다
- **미완료:** 조명 행 반영(5절의 catalog admission 선결), 물 10행 재생산 경로, MIC 15종,
  legacy 경로에 남은 141자산

## 7. 무변경 증거

- 정본·게시본 **15파일 전부 G1 백업과 sha256 동일**(변경 0)
- `mapmaterials.json` sha256 `8ad7ecdb3eb926c1`, materials 329, placementLighting 0
- Resources `Map/LV_OCN_EVENTIS_MHP` 384자산 / 6,323파일 / wmodel 384 유지
- Resources `Map/Lighting/Maharaka` 137파일 유지, LUT 16,512 bytes 존재
- 어댑터가 engine-default DDS를 쓴 곳은 staging `C:/LostArkExtract/SMBnew` **3파일**이며
  live Resources가 아니다
- `build.receipt.json`의 `sourceMaterialBuild`는 **여전히 `null`**이다. 적용하지 않았으므로
  임의로 채우지 않았다

## 8. 다음 한 걸음

이 build를 적용 가치가 있게 만들려면 두 가지가 먼저다.

1. **`_RNM_` variant 883개를 catalog가 admit하게 한다.** 그래야 조명 payload의 56%(B 범주
   530행)와 나머지가 bind 대상이 된다. `build_maptool_scene.py` 경로이고, 베른이 `_RNM_`
   15,483행을 base wmodel로 admit한 방식이 선례다.
2. **물 10행을 재생산 가능하게 한다.** `render_profile_text()`가 `Water`를 내지 못하는
   결함을 고치거나, 컴파일 출력에 물 10행을 병합하는 경로를 확정한다. 지금은 컴파일러가
   그 10행을 만들지 못하므로 어떤 재생성도 baseline 보존 장치 없이는 손실이다.

이 두 가지가 풀리기 전에는 source material build를 정본에 적용할 이유가 없다. 내용이
늘지 않고 물 10행만 위험해진다.

## 산출물

`out/MaharakaContinuation_20260926_193455/SourceMaterialBuild/` 22파일
(`backup/` 15, `backup-manifest.json`, `inputs/inputs.json`, `inputs/slot_report.json`,
`compiled/` 2, `adapter.log`, `compiler.log`).
