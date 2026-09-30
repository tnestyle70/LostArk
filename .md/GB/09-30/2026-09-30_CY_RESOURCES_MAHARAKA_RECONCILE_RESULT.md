# 2026-09-30 마하라카 리소스 누락 점검·CY_Resources 보충 RESULT

빌드·게임 실행·코드/Data 수정·git 쓰기 명령은 하지 않았다. `Client/Bin/Resources`는 읽기만 했고, `CY_Resources`에는 추가 복사만 했다(기존 파일 덮어쓰기·삭제 없음). 복사 스크립트나 배포 목록 txt는 만들지 않았다.

## 1. 왜 누락이 생겼나

- 팀원 오류: 마하라카 입장 시 `Catalog validation failed`, 그리고 마하라카 NPC가 보이지 않음.
- 마지막 배포 목록은 `Resource_Distribution_2026-09-25_MaharakaMaterials.txt`(09-25 21:53)다. 그 뒤 09-26~09-30에 만들어진 리소스가 어떤 배포 목록에도 없었고, 다시 만든 `CY_Resources`에도 없었다.
- 앞선 수집 작업은 "이번 브랜치 시작 커밋(23008e0e) 이후 바뀐 데이터가 참조하는 것"만 기준으로 삼았다. 09-26~09-27에 만들어졌지만 한 번도 배포되지 않은 파일(ITR_02453_SK 모델, FLOOR01A/B 모델, NPC 152개, 지형 타일 등)을 "이미 있던 팀 Drive 팩"으로 잘못 분류했다. 기준을 "마지막 배포 이후 생성/변경 + 어떤 예전 목록에도 없음"으로 바로잡았다.

## 2. 단계별 결과

### 이전에 이미 복사된 것(이 작업 전)
- `Map/LV_OCN_EVENTIS_MHP/MAP_65096D72C5C9_ITR_02453_SK` 11개(사용자 지시로 다른 작업이 복사).
- `Character/NPC/Maharaka` 전체 152개 + `Character/SourceMaterials/efmaster_material_prologue` 4개(마하라카 NPC 22 archetype의 모델·텍스처 폐쇄). 156개, 180MB.

### A. 마하라카 맵 카탈로그 (이번에 180개, 58.31MB 복사)
- 참조 파일 6,612개를 수집했다(카탈로그 모델 409개의 폴더 전체 6,503개 + 모델 내부 텍스처 1,189개 + 재질/물/월드시퀀스 문서의 경로 109개, 미해결 0).
- 결과 분류: `CY`에 이미 있음 20, 예전 목록에 있고 그 뒤 변경 없음 3,356, 마지막 목록보다 오래됐고 어떤 목록에도 없음 3,056(전부 09-19 변환기 중간 산출물 `.bin/.gltf/converter.*.txt/geometry.*.json`, 게임이 읽지 않음), **복사 후보 180**.
- 복사한 180개: `Map/LV_OCN_EVENTIS_MHP` 35(FLOOR01A/B 모델 각 8개, 나중에 추가된 노멀맵 텍스처 포함), `Map/LV_OCN_EVENTIS_MHP_LAND` 48(지형 타일), `Map/LV_OCN_EVENTIS_MHP_SOURCE_MATERIALS` 1, `Map/Lighting` 92, `Sound/Maharaka` 4. 전부 sha1 검증.

### B. 캐릭터·이펙트·사운드·UI (이번에 132개, 60.35MB 복사)
- `Character/*/AnimSets/*_WaterGunAnimSet.wmodel` 7, `Character/Maharaka/WaterGun` 4, `Effect/Maharaka/WaterGun·Waterpang` 9, `Effect/Bern/AnchorMarker` 3, `Map/LV_BER_BERNCASTLE` 9(섬 모델 ISL_00072 1 + SourceMaterials 8), `Sound/Maharaka` 22, `UI/HUD` 62(Voyage 58 + Waterpang 4), `UI/Interact` 2(Icon_lever, Icon_move), `UI/Loading` 2(Maharaka, Sea_0), `UI/Minimap` 12.
- 사용자가 지운 잔여물(`BernHarbor.png`, `Maharaka_0.png`, `Loading_Background_Sea_1/2.png`)은 복사하지 않았다.

### `CY_Resources` 현재
- 479개 파일, 309.9MB. 폴더별: `Character/Artist·DimensionMaster·GuardianKnight·GunSlinger·LanceMaster·Slayer·Warlord` 각 1(AnimSets), `Character/Maharaka` 4, `Character/NPC` 152, `Character/SourceMaterials` 4, `Effect/Bern` 3, `Effect/Maharaka` 9, `Map/Lighting` 92, `Map/LV_BER_BERNCASTLE` 9, `Map/LV_OCN_EVENTIS_MHP` 46, `Map/LV_OCN_EVENTIS_MHP_LAND` 48, `Map/LV_OCN_EVENTIS_MHP_SOURCE_MATERIALS` 1, `Sound/Maharaka` 26, `UI/HUD` 62, `UI/Interact` 2, `UI/Loading` 2, `UI/Minimap` 12.

## 3. 팀원 409개 vs 이 PC 385개 차이의 정체

- 실제 차이가 아니다. `origin/main`의 마하라카 `mapassets`(Data 정본과 게시본 모두)를 `git lfs smudge`로 읽어 이 PC 파일과 비교했더니 sha256이 완전히 같다(정본 `d2a27971…`, 게시본 `c52af567…`).
- 카탈로그의 고유 모델은 409개이고 구성은 `Map/LV_OCN_EVENTIS_MHP` 385 + `Map/LV_OCN_EVENTIS_MHP_FOLIAGE` 8 + `Map/LV_OCN_EVENTIS_MHP_LAND` 16이다. 이전의 385는 첫 묶음만 센 숫자였다.
- 409개 모두 이 PC에 존재한다. 이 PC에 없는 모델 참조는 0.

## 4. 이 PC에 없는 참조(복사 불가)

Maharaka 범위의 문서(UI 레이아웃, 카탈로그, Maharaka/Bern/ship wake 이펙트 문서, 사운드 카탈로그)를 훑어 이 PC에 파일이 없는 경로를 찾았다. 313개이고 우리 작업물이 아니다.
- `Sound/KoukuSaton` 174, `Sound/Vehicle` 106, `Sound/Valtan` 13 (`CharacterSoundCatalog.json`이 참조; 팀원 커밋으로 main에 들어온 사운드)
- `UI/HUD` 11(`HudBuffIcons.json`, `HUD_Layout.json`), `UI/ClassSelect/Common` 6, `UI/CharacterSelect` 2, `UI/Inventory` 1
- 이건 팀장/팀원 Drive 리소스라 이 PC에서 복사할 수 없다. 이 PC에도 없으므로 우리 배포로 생긴 누락이 아니다. 이전 스캔에서 본 `Effect/Vehicle/Aufstehen`·`FullRestore` 메시도 같은 성격이다.

## 5. 검증 표(C)

마하라카 범위 6,908개 파일(맵 6,612 + 물총·이펙트·UI·섬·닻·NPC 추가분)을 재스캔했다.

| 상태 | 개수 |
|---|---|
| `CY_Resources`에 있음 | 479 |
| 예전 배포 목록에 있고 그 뒤 변경 없음(근거: `Resource_Distribution_0806_0807`~`2026-09-25_MaharakaMaterials` 목록들) | 3,356 |
| 마지막 목록보다 오래됐고 목록에 없는 변환기 중간 산출물(게임이 읽지 않음) | 3,056 |
| 마지막 목록보다 오래됐고 목록에 없는 런타임 파일 | 17 |
| **실패 후보(마지막 배포 이후 생성됐는데 `CY`에 없음, 또는 이 PC에 없음)** | **0** |

- 17개 런타임 파일은 `UI/Minimap/Maps/BernCastle.png`, `KakulSaydonArena.png`, `ValtanArena.png`, `btn_*.png` 등 09-05에 만들어진 미니맵 기본 이미지다. 베른·발탄·쿠크가 이미 동작하는 리소스라 이미 팀원에게 있다고 본다.
- 한계: "예전 목록에 있고 마지막 배포보다 오래됐으면 팀원이 갖고 있다"는 가정에 근거했다. 팀원 PC를 직접 확인한 것은 아니다.

## 6. 범위 밖 미배포 목록(D, 복사하지 않음)

Maharaka 범위에서 참조가 확인되지만 마지막 배포 이후 생성됐고 `CY`에 없는 다른 영역 파일이다.
- `Character/GuardianKnight` 154, `Character/LanceMaster` 22, `Character/SourceMaterials` 24, `Character/KoukuSaton` 2, `Character/Valtan` 1
- `Effect/KoukuSaydon` 62, `Effect/Valtan` 1, `Effect/Warlord` 2
- `Sound/Asther` 31, `Sound/KoukuSaton` 9, `Sound/Mario` 36, `Sound/Valtan` 9
- `UI/Durability` 20, `UI/Repair` 4, `UI/Customizing` 3, `UI/Common` 2, `UI/HeadStatus` 2, `UI/Shop` 2, `UI/SystemMenu` 2
- 이 표 밖에도, 참조 여부를 이번에 확인하지 않은 미배포 파일이 남아 있다(이전 전체 스캔에서 `Character/Artist` 267, `DimensionMaster` 270, `Warlord` 269, `Map/LV_LOBBY_CLASSSELECT_SL03_FOLIAGE` 48, `Effect/Valtan` 23 등). 사용자가 다음에 정한다.

## 7. 팀원에게 전달할 내용

- `CY_Resources` 안의 폴더를 팀원의 `Client\Bin\Resources\` 아래에 같은 경로로 덮어 넣는다. 추가만 있고 기존 파일 삭제는 없다. 총 479개, 309.9MB.
- 마하라카 입장 오류와 NPC 미표시는 위 내용으로 해결되어야 한다. 그러나 팀원 PC에서 실행해 확인한 것은 아니다. 그래도 같은 오류가 나면 팀원 화면의 다음 누락 경로를 알려달라.
- 지형(`_LAND`) 48개와 라이트맵 92개가 이번 A단계에서 처음 나왔다. 이전에는 이 둘이 빠져 있었다.
