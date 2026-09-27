# 2026-09-26 Resources Area install 파괴적 동기화 안전장치 RESULT

`Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP`의 기존 382 자산이 삭제된 사고의 재발을 막는 작업이다.
도구 안전장치만 구현했고 Resources·Data·DataFiles·게시본은 건드리지 않았다.

## 1. 확인한 원본 — 무엇이 어떤 조건에서 지우는가

`Tools/LevelPlacementExtractor/build_map_material_variants.py::install_runtime_area`는
**파일 단위 동기화가 아니라 Area 디렉터리 전체 교체**다. 코드 근거는 다음 순서다.

| 줄 | 코드 | 역할 |
|---|---|---|
| 2131 | `previous_receipt = validate_owned_area(destination)` | receipt 없으면 raise, 있으면 전 파일 CAS 검증 |
| 2146~2180 | `stage.mkdir()` 후 manifest의 asset만 stage로 복사 | **manifest에 없는 자산은 stage에 존재하지 않는다** |
| 2221 | `os.replace(destination, backup)` | 기존 Area 전체를 backup으로 이동 |
| 2225 | `os.replace(stage, destination)` | manifest 자산만 든 stage를 Area 자리에 올림 |
| 2240 | `base.bounded_rmtree(backup, map_root)` | **commit 후 backup 삭제 = 기존 자산 소멸** |

즉 삭제는 "누락 자산 정리" 같은 별도 단계가 아니라 **전체 교체의 부산물**이다.
`validate_owned_area`(2030~2063)는 소유권과 파일 해시만 확인하고 **규모 축소를 검사하지 않는다.**
그래서 자산 2종 manifest로 실행하면 382종이 CAS를 통과한 뒤 그대로 사라진다.

사고 당시 manifest는 `assetCount: 2`였다
(`out/.../Scene03bInstall/stage.runtime-manifest.json`, 자산
`MAP_167F91F6940F_..._FLOOR01A_SM_LNH_OVR_0636FCF08A2E`와 `MAP_AF1951C8B827_..._FLOOR01B_SM_LNH`).
install이 쓴 receipt는 `assetCount: 2 / ownedFiles: 14`이며
`out/.../Scene03bInstall/backup/stale-install-receipt.json`에 보존돼 있다.

**증명하지 못한 한 가지:** 삭제 직전 Area에 있었던 이전 receipt의 내용이다. 그 파일은 backup으로
옮겨진 뒤 `bounded_rmtree`로 함께 지워졌다. 코드 경로상 `validate_owned_area`가 통과해야 교체가
일어나므로 382 자산을 선언한 receipt가 존재했다고 **추론**하지만, 그 파일을 직접 확인하지는 못했다.

## 2. 감사 — 같은 패턴이 또 있는가

`ownedFiles` / `validate_owned_area` 방식의 **Area 전체 소유 모델은 저장소에서 이 파일 하나뿐**이다
(`grep -rln "ownedFiles\|INSTALL_RECEIPT_NAME\|validate_owned_area" Tools --include=*.py` → 본체와 그 테스트).

| 대상 | 삭제 범위 | 위험도 | 판정 |
|---|---|---|---|
| `build_map_material_variants.py::install_runtime_area` | Area 디렉터리 전체 | **높음** | 사고 발생 지점, 이번에 수정 |
| `CompositionPipeline/composition_pipeline.py:4170~4171` | `row["relativePath"]` **파일 단위**, 대상은 `output_root`(publisher 산출물) | 낮음 | 디렉터리 prune 없음, Resources 아님 |
| `EffectPipeline/*`, `ValtanPipeline/*`, `WorldPipeline/*`의 `os.replace(staged[path], path)` 계열 | 파일 단위 atomic 교체 + backup 복원 | 낮음 | 정상 atomic write 패턴 |
| 약 95개 파일의 `temporary.unlink()` / `rmtree(stage)` | 자기 임시·staging 경로 | 낮음 | 정상 정리 |

**같은 위험을 가진 다른 Area가 실재한다.** receipt 소유 Area를 실측했다.

| Area | receipt assetCount | ownedFiles |
|---|---|---|
| `LV_LUT_MIDNIGHTC_ED` (쿠크) | **292** | **2,554** |
| `LV_OCN_EVENTIS_MHP_FOLIAGE` | 9 | 50 |

`install`의 `--expect-variants` 기본값이 292로 쿠크에 맞춰져 있다. 수정 전에는 짧은 manifest로
쿠크에 install을 돌리면 2,554 파일이 같은 방식으로 사라졌다.
`LV_OCN_EVENTIS_MHP`는 사고 후 parent가 stale receipt를 제거해 현재 receipt가 없다(= 지금은
`validate_owned_area`가 raise하므로 install 자체가 막힌다).

감사 범위는 `Tools/**/*.py`다. C#/PowerShell/C++ 경로는 보지 않았으므로 "없다"고 단정하지 않는다.

## 3. 실제 구현

`Tools/LevelPlacementExtractor/build_map_material_variants.py` — +2,635 bytes, LF·ASCII·무BOM 유지,
새 주석은 전부 영문. 오늘 먼저 들어간 texture package 가드 수정(`parts) not in (1, 2)`)은 보존했다.

1. **임계값 상수**(`INSTALL_RECEIPT_NAME` 옆) — `PRUNE_ABSOLUTE_LIMIT = 10`,
   `PRUNE_FRACTION_LIMIT = 0.20`. 근거를 주석으로 남겼다. 절대 상한은 일상적인 자산 은퇴를
   사람이 눈으로 확인할 수 있는 크기로 제한하고, 면적 비율 상한은 자산이 몇 개뿐인 작은 Area가
   큰 Area 기준 상한 때문에 통째로 비워지는 것을 막는다. manifest/Area 불일치는 **둘 다** 넘긴다.
2. **`_receipt_asset_ids(receipt)`** — 검증된 receipt의 `ownedFiles` 경로에서 최상위 자산
   디렉터리를 복원한다. 경로가 `<ASSET_ID>/...` 형태임을 실제 receipt로 확인했다.
   `None`이면 빈 집합(신규 설치).
3. **`prune_missing: bool = False`** 매개변수와 **`--prune-missing`** CLI 플래그, dispatch 연결.
4. **가드 본체** — staging 직후, 파괴적 `os.replace` **이전**에 실행한다.
   - 버려질 자산이 있고 플래그가 없으면 → **거부**하고 수와 목록을 오류에 담는다.
   - 플래그가 있어도 `> 10` 또는 `> 20%`면 → **거부**하고 상한과 수를 출력한다.
5. **receipt 기록** — `installPrune` 블록에 `previousAssetCount`, `preservedAssetCount`,
   `droppedAssetCount`, `droppedAssets`, `pruneMissingRequested`를 남긴다. 조용한 prune이 불가능해진다.
   `schemaVersion`은 **올리지 않았다**. 올리면 기존 쿠크 receipt(schemaVersion 1)가
   `validate_owned_area`에서 거부되기 때문이다.

상한을 넘는 prune에는 두 번째 우회 플래그를 두지 않았다. 그게 상한의 목적이다. 정말 필요하면
작은 배치로 나누거나 사람이 의도적으로 Area를 정리해야 한다.

## 4. live 설치 / 게시

**없다.** Resources·`Data/**`·`Client/Bin/DataFiles/**`에 한 바이트도 쓰지 않았다.
모든 검사는 `tempfile.TemporaryDirectory()` 안에서 했다.

## 5. 빌드 / 자동검사

빌드 0회(python 도구 작업), publisher 0회.

- `test_build_map_material_variants.py` → **20 tests OK**(기존 15 + 신규 5). 패치 직후 기존 15개만으로도 OK를 먼저 확인했다.
- 신규 검사 5개: 플래그 없으면 보존+보고 / 플래그+소량이면 삭제 및 receipt 기록 / 절대 상한 초과 거부 / 작은 Area의 면적 상한 초과 거부 / 신규 추가·갱신 경로 불변.
- 절대 상한과 면적 상한을 **분리해** 검사했다. 60자산에서 11개 삭제는 면적 상한(12) 안이지만 절대 상한(10)에 걸리고, 5자산에서 2개 삭제는 절대 상한 안이지만 면적 상한(1)에 걸린다.

**사고 형태 재현 검사(임시 폴더, 382 → 2):**

```
installed before: 382
prune_missing=False -> REFUSED: install would drop 380 of 382 installed assets ...
prune_missing=True  -> REFUSED: install prune of 380 of 382 ... exceeds the safety cap (10 absolute, 20% of the area)
installed after: 382 | preserved: True
```

**정상 경로 불변 검사** — 오늘 성공한 2종 install을 같은 manifest·같은 runtime root
(`C:/LostArkExtract/Scene03bCook_20260926/Runtime/runtime`)로 **임시** Resources에 재실행했다.
`schemaVersion`, `areaId`, `runtimeManifestSha256`, `assetCount`, `admissionState`,
`runtimeCoverage`, `ownedFiles` **7개 필드 전부 오늘 receipt와 동일**하다. 추가된 것은
`installPrune`(삭제 0) 뿐이다.

## 6. 사용자 수동확인

없다. 화면에 나타나는 변경이 없다. 게임 실행·조작·캡처를 하지 않았다.

## 7. 미완료와 다음 조사 위치

- **감사 범위가 `Tools/**/*.py`에 한정된다.** PowerShell publisher와 C++/C# 경로에 Resources를
  지우는 코드가 있는지는 보지 않았다. "없다"고 단정하지 않는다.
- **사고 직전 receipt의 내용을 확인하지 못했다**(1절). 382 자산 receipt가 있었다는 것은 코드
  경로에 근거한 추론이다.
- **복구본의 바이트 동일성은 여전히 미보장**이다. 09-19 cook 산출물로 복구했으므로 그 사이
  수동 교체가 있었다면 되살리지 못했다. 이번 작업으로 달라지지 않는다.
- **쿠크 Area는 지금도 receipt 소유 상태**다. 이번 가드가 짧은 manifest 실행을 막지만, 쿠크
  게시본을 재생성할 계획이 있으면 그 전에 이 가드의 상한이 적절한지 다시 판단해야 한다.
- 이번 가드는 **install 경로만** 막는다. 사람이 손으로 Resources를 지우는 것은 막지 않는다.
- 범위 밖 관찰 한 줄: `gotchas.md`의 기존 줄 "57011의 300004는 모델 없는 collision Prop다"는
  오늘 G02 2차가 `EFTable_Npc.Model = MN_KZDW_02-1`로 정정한 내용과 어긋난다. 내 범위가 아니라
  건드리지 않았다.

## 8. 변경 파일

| 파일 | 변경 | 비고 |
|---|---|---|
| `Tools/LevelPlacementExtractor/build_map_material_variants.py` | +2,635 bytes | 가드·상수·헬퍼·CLI·receipt |
| `Tools/LevelPlacementExtractor/test_build_map_material_variants.py` | +5,533 bytes | `InstallPruneGuardTests` 5개 |
| `.md/GB/gotchas.md` | +892 bytes, **9줄 추가만** | 맨 끝 append, 기존 바이트 prefix 동일 확인 |

백업은 전부 `out/MaharakaContinuation_20260926_193455/ResourceGuard/backup/`에 있다.
패치 스크립트 3개는 같은 폴더의 `patch_install_prune_guard.py`,
`patch_prune_guard_tests.py`, `append_gotcha.py`다.
