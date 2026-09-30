# 2026-09-30 CY_Resources 배포 리소스 수집 RESULT

요청: 이번 작업에서 추가·변경돼 Drive로 배포할 리소스를 `C:/Users/USER/OneDrive/바탕 화면/CY_Resources`에 전부 모으고, 빠진 것이 없는지 검증한다.
이 fork는 리소스 복사와 검증만 했다. 코드·Data·게시·`Client/Bin/Resources` 원본은 건드리지 않았고, 빌드와 Client/Server 실행도 하지 않았다. 복사 스크립트와 목록 파일은 만들지 않았다.

## 결론

- **이번 작업이 만든 리소스는 전부 `CY_Resources`에 들어 있고, 이번 fork가 추가로 복사한 것은 0개다.** 앞서 부모가 복사한 20개를 포함해 현재 124개 파일(54.2 MB)이 있다.
- `CY_Resources`의 124개는 `Client/Bin/Resources`의 같은 경로 파일과 바이트(sha1)가 모두 같다. 내용이 다른 파일 0개, 원본에 없는 파일 0개다.
- 이번 작업의 Data·코드가 참조하는 리소스 216개(wmodel 안 텍스처 포함)는 전부 `Client/Bin/Resources`에 존재한다. 이 중 100개는 `CY_Resources`에 있고, 나머지 116개는 이번 작업 이전부터 있던 팀 Drive 팩 리소스(선행 리소스)다.
- 사용자가 이 작업 중에 커밋 4개를 올렸다(브랜치 `codex/maharaka-sea-island-waterpang-0930`). 그래서 "이번 작업의 변경"은 작업 시작 기준 커밋 `23008e0e`와 현재 작업 트리의 차이로 정의했다(변경된 추적 파일 142개).

## 방법

1. `git diff 23008e0e`(커밋 + 미커밋)와 미추적 파일에서 변경·추가된 Data(효과 문서, VehicleCatalog, HUD_Layout, MinimapAreas, 지도 재질·자산)와 코드(추가된 줄)의 리소스 경로 리터럴을 수집했다.
2. 수집한 wmodel은 UTF-16 문자열로 텍스처 경로를 읽어 폴더 상대·`textures`·`SourceMaterials` 규칙으로 해석해 텍스처까지 폐쇄했다. 해석 실패 0개.
3. 효과 문서는 우리 계열만 포함했다: `effect.maharaka.*`, `effect.vehicle.ship.wake.*`, `effect.bern.anchor.*`, `effect.bern.harbor.*`. (`effect.vehicle.*` 전체나 `effect.bern.*` 전체를 쓰면 팀원의 Aufstehen·Terpeion·환경 이펙트가 섞여 들어와서 좁혔다.)
4. 참조 경로마다 (i) `Client/Bin/Resources`에 존재하는지, (ii) `CY_Resources`에 있는지를 확인하고, 없는 것은 수정 시각과 출처로 분류했다.
5. 빌더·설치 스크립트(변경·추가된 Tools 20개)가 출력하는 리소스 경로도 별도로 대조했다. 여기서 나온 미포함 9개는 모두 Kouku 팩의 `FullRestore/Meshes`이며 우리 문서는 참조하지 않는다.

## 참조 폐쇄 검증

| 항목 | 개수 |
|---|---:|
| 참조된 리소스 경로(텍스처 포함) | 216 |
| `Client/Bin/Resources`에 없음 | 0 |
| `CY_Resources`에 있음(바이트 동일) | 100 |
| `CY_Resources`에 없음 = 선행 리소스 | 116 |
| 그중 9월 28일 이후 수정 + `CY_Resources`에 없음 | 23 (전부 아래 Kouku/Valtan 팩) |

**누락 0.** 이번 작업이 새로 만든 파일 중 참조되는데 `CY_Resources`에 없는 것은 없다.

## 이번 작업 결과로 판정한 것 (CY_Resources에 있음)

| 그룹 | 파일 | 근거 |
|---|---:|---|
| `Character/*/AnimSets/*_WaterGunAnimSet.wmodel` (Artist, DimensionMaster, GuardianKnight, GunSlinger, LanceMaster, Slayer, Warlord) | 7 | CharacterCatalog `waterGunAnimationSetModels`, 09-28 23:08 생성, 부모가 복사 |
| `Character/Maharaka/WaterGun/ITR_02164` | 4 | 물총 모델, 결과 문서 WATERGUN_RESULT |
| `Effect/Maharaka/WaterGun`, `Effect/Maharaka/Waterpang` | 9 | 워터캐논·물벼락·물총 이펙트 문서가 직접 참조 |
| `Effect/Vehicle/Ship` | 3 | 배 물살 이펙트 문서 |
| `Effect/Bern/AnchorMarker` | 3 | 닻 마커 문서(`fm_b_halfsphere_003.wmodel`, `fx_d_symbol_035.dds`, `fx_d_line_005.dds`) |
| `Map/LV_BER_BERNCASTLE` (섬 모델 `MAP_0D832EC70895_ISL_00072_SK` 1 + `SourceMaterials` 9) | 10 | 섬 자산 카탈로그와 재질 행이 참조 |
| `Sound/Maharaka/WaterGun` | 12 | 물총 발사 소리 |
| `UI/HUD/Voyage`, `UI/HUD/Waterpang` | 62 | HUD_Layout 슬롯이 참조 |
| `UI/Loading` (`Loading_Background_Maharaka.png`, `Loading_Background_Sea_0.png`) | 2 | Level_Loading, MainApp이 참조 |
| `UI/Minimap` (`Maps/*` 11개 + `marker_island.png`) | 12 | MinimapAreas가 참조 |
| 합계 | 124 | 총 54.2 MB |

## 복사하지 않은 것과 사유

### A. 선행 리소스: 우리 결과물이 참조하지만 이번 작업 이전부터 있던 팀 Drive 팩 (받는 쪽에 이미 있어야 함)

- **Kouku/Valtan/직업 이펙트 텍스처 (116개 중 대부분)**
  - `Effect/KoukuSaydon/Textures` 83, `Effect/KoukuSaydon/FullRestore` 7, `Effect/Valtan/Textures` 4, `Effect/Esther/*` 6, `Effect/Warlord/Textures` 4, `Effect/Artist/Textures` 1, `Effect/DimensionMaster/Textures` 1, `Effect/World/Textures` 1.
  - 참조하는 것: 워터캐논·물벼락·물총 이펙트 문서와 닻 마커의 텍스처(예: `fx_m_wave_001.dds`, `fx_d_noise_009.dds`).
  - 근거: 원본 파일은 9월 3~25일에 생성됐고, 그중 23개(Kouku 22 + Valtan 1)는 9월 29일 19:52~19:53의 일괄 재설치 묶음이 수정 시각만 갱신한 것이다. CLAUDE.md도 `Effect/KoukuSaydon`을 팀장 Drive 팩으로 규정한다. 사용자가 이전에 발탄·쿠크 이펙트를 `CY_Resources`에서 지우게 한 방침과도 일치한다.
- **섬 재질이 쓰는 클래스 선택 공용 재질 2개**
  - `Map/LV_LOBBY_CLASSSELECT_SL12/SourceMaterials/8b35b0d8e8e3_normal.dds` (144 B)
  - `Map/LV_LOBBY_CLASSSELECT_SL12/SourceMaterials/8675f5bc7bb9_ambientreflection_02.dds` (131 KB)
  - 섬의 `SLOT_003_isl_00072_mi` 재질이 detailNormal과 reflection으로 참조한다. 09-25에 생성된 기존 클래스 선택 자산이다. **받는 쪽에 이 두 파일이 없으면 섬 재질 로드가 실패할 수 있으므로 특히 확인이 필요하다.**
- **UI 선행 5개** (수정 시각 08-30~09-19, 이번 작업 이전 것)
  - `UI/Common/White1x1.png` (78 B): 새 HUD 슬롯이 참조
  - `UI/Interact/Icon_check.png` (2.2 KB): `InteractKeyPromptView.cpp`에 이번에 추가된 `case 4`가 참조
  - `UI/Interact/Icon_godown.png`, `Key_G.png`, `ShowFx_0.png`: `InteractKey_Layout.json`이 참조(파일 자체는 이번에 안 바뀜)
- `UI/Loading/Loading_Background_Prologue.png` (1.1 MB, 09-14): `MainApp.cpp`의 기존 등록 목록에 있는 것으로 이번 작업이 만든 것이 아니다.

### B. 이번 작업과 무관하다고 판정한 것 (참조하는 변경 파일이 없음)

이 그룹을 참조하는 Data·코드 파일(CharacterCatalog, SquareHoleInstruments, BossCatalog, CharacterSoundCatalog, SL00·HEARTRB worldsequences, ItemCatalog, Data/UI/Interact)은 모두 작업 시작 기준 커밋 이후 변경이 0건이다. 파일들의 수정 시각 대부분은 9월 29일 19:52~19:53의 일괄 재설치 묶음 또는 다른 팀원 작업 시간대다.

| 그룹 | 파일 | 근거 |
|---|---:|---|
| `Effect/Artist·DimensionMaster·Esther·LanceMaster·Warlord·World` 메시류 | 26 | 19:52~19:53 묶음, 참조 문서는 팀원의 classselect·환경 이펙트 |
| `Effect/KoukuSaydon` 111, `Effect/Valtan` 48 | 159 | 팀장 Drive 팩 |
| `Character/IT_DK_CART_00·IT_FT_PIPA_00·IT_SP_HARP_00·IT_WR_HORN_00` | 16 | `Data/Actors/SquareHoleInstruments.json`(미변경)가 참조, 09-29 17:08 |
| `Character/LanceMaster`(시네마틱 외형 등), `Character/GuardianKnight` WallClimb, `Character/SourceMaterials`, `Character/Valtan/Ghost` | 34 | CharacterCatalog·BossCatalog(미변경)가 참조 |
| `Map/LV_LOBBY_CLASSSELECT_SL03_FOLIAGE` | 48 | 참조 0건, 09-29 20:32(클래스 선택 작업) |
| `Sound/Character` 4, `Sound/CharacterSelect` 6, `Sound/Valtan` 9 | 19 | SL00·HEARTRB worldsequences와 CharacterSoundCatalog(미변경)가 참조 |
| `UI/Common` 8, `UI/HeadStatus` 1, `UI/Items` 4, `UI/Shop` 2, `UI/SystemMenu` 2 | 17 | 기존 뷰 코드가 참조, 이번 diff에 없음 |
| `UI/Interact/Icon_lever.png`, `Icon_move.png` | 2 | 저장소 어디에서도 참조하지 않음 |

### C. 참조되지 않는 잔여물 (복사 안 함, 삭제도 안 함)

- `UI/Minimap/Maps/BernHarbor.png`, `Maharaka_0.png`: 사용자가 지운 미니맵 항목의 이미지, 참조 0건.
- `UI/Loading/Loading_Background_Sea_1.png`, `Sea_2.png`: 사용자가 일부러 삭제한 로딩 이미지. `Client/Bin/Resources`에는 아직 있지만 코드가 더 이상 참조하지 않는다(재빌드 후 삭제해도 됨).
- 제거한 축소 섬(v2/v3)의 별도 산출물은 `Client/Bin/Resources`에 남아 있지 않다. 새 섬은 `MAP_0D832EC70895_ISL_00072_SK` 하나만 참조한다.

## 사용자가 결정할 항목

1. **선행 리소스를 배포에 함께 넣을지**: A 그룹 중 작고 이번 작업이 직접 의존하는 것은 넣는 편이 안전하다. SL12 재질 2개(131 KB), `UI/Interact` 4개(약 13 KB), `UI/Common/White1x1.png`(78 B). 받는 쪽이 이미 팀장 팩을 갖고 있으면 필요 없다.
2. Kouku/Valtan 팩 텍스처는 기존 방침대로 제외했다. 바꾸려면 알려달라.
3. `Sea_1`, `Sea_2`를 `Client/Bin/Resources`에서 언제 지울지(재빌드 후 삭제 가능).

## 참고 산출물

- 참조 전체 목록과 출처: `C:/Users/USER/.claude/jobs/46aea322/tmp/cy_closure.json`
- 스크립트: 같은 폴더의 `cy_closure.py`, `cy_classify.py`, `cy_coverage.py`
- 커밋하지 않았다.
