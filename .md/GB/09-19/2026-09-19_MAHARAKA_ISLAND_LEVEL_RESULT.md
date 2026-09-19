# 마하라카 파라다이스 섬 레벨 추가 RESULT

작성일: 2026-09-19
브랜치: `feature/maharaka-island-level` (origin/main `8f15a3c3`에서 분기, 미커밋)
대응 PLAN: [2026-09-19_MAHARAKA_ISLAND_LEVEL_PLAN.md](2026-09-19_MAHARAKA_ISLAND_LEVEL_PLAN.md)

## 1. 상태 요약

| 구분 | 상태 |
|---|---|
| 코드(Shared/Server/Client/Tools) | 반영 완료, 빌드 미실행 |
| 맵 에셋 cook·Resources 설치 | 완료 (변형 382종 + 섬 랜드스케이프 16조각) |
| Area 데이터(Imported/Authoring/MapCatalog) | 완료, `Publish-MapAuthoring -Mode Validate` PASS |
| World·Navigation 데이터 | 완료, 두 publisher Validate PASS |
| 게시(Publish) | 03:01 1차 게시(VS 종료 상태): 맵 3,839 placements, `MAHARAKA.worldbootstrap`·`MAHARAKA.npcpresentation.json` 신규(다른 월드 출력 해시 변화 없음), navgrid/navpolicy/navblockers Server·Client 양쪽. **이후 카탈로그(`mapassets` sha `8d38065a…`, render profile 300개 반영)를 다시 만들었고 이 재게시는 VS가 켜져 있어 미실행**이다. 게시된 `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapassets`는 아직 03:01 판(`bac8c3d0…`)이라 얼룩이 그대로 보인다 |
| Product 빌드 | **미실행** (사용자 실행 대상) |
| Server+Client 입장·화면 확인 | **미실행** (사용자 전용) |
| 팀 문서 | AGENTS/CLAUDE 로비·레벨 계약, Area 가이드 표, gotchas 갱신 |

## 2. 작업 중 브랜치 전환

작업 도중 다른 세션이 이 작업 폴더를 `main`으로 전환하고 `origin/main`을 fast-forward했다. 그때 미커밋 변경은
`stash@{0}`("backup before switching to main d2d3563b (Maharaka/EOL uncommitted, 2026-09-19)")에 백업됐다.

- `main`(8f15a3c3)에서 `feature/maharaka-island-level`을 만들었다. 같은 커밋이라 파일은 바뀌지 않는다.
- stash에서 마하라카 파일 19개의 변경분만 `git apply --3way`로 다시 적용했다. 18개는 충돌 없이 들어갔다.
- `PacketType.h`는 main의 protocol 93 설명과 충돌했다. main 93 설명을 보존하고 94를 이어 붙였다.
- stash의 `Framework.sln` 변경은 이 작업이 아니어서 적용하지 않았다. stash는 삭제하지 않았다.
- untracked 데이터 두 파일(`Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json`, `Data/Navigation/LV_OCN_EVENTIS_MHP.navgrid.json`)은 stash 대상이 아니라 그대로 남아 있었다.

## 3. 원본 범위와 제외

원본 레벨 `LV_OCN_EventIS_MHP_PS`는 섬(존 57009)과 레이싱 트랙(57010)을 함께 담는다. 섬은 원본 y 약 913~1071m,
레이싱은 y 0~350m 부근이다. 섬 범위 규칙은 원본 y > 50000cm이다.

| 레벨 | 원본 배치 | 포함 | 제외 사유 |
|---|---:|---:|---|
| `LV_OCN_EVENTIS_MHP_SL01` | 3,823 | 3,820 | `bg_tot_movillage_decoprop07f_sm_artree` 3개: 원본 glTF normal/tangent 평행, native parallel 증거 생성기 없음 |
| `LV_OCN_EVENTIS_MHP_LAND01` | 82 | 2 | 레이싱 쪽 80개. 포함 2개는 광장 중앙 수영장 `pool01/pool02` |
| `LV_OCN_EVENTIS_MHP_PS` | 244 | 1 | 숨김 컬링 박스 243개. 포함 1개는 하늘 돔 `lv_matte.mesh.sky_mirror_sm`(Bern·Valtan도 사용) |
| 랜드스케이프 LAND01 | 46조각 | 16조각 | 레이싱 쪽 30조각 |

제외 기록: `C:\LostArkExtract\LV_OCN_EVENTIS_MHP_20260919\admission.exclusions.json`

## 4. 산출물

### 저장소 데이터

| 파일 | 내용 | SHA-256 |
|---|---|---|
| `Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapassets` | 398 assets (LFS). render profile 300개 반영판. 03:01 1차 게시본은 `bac8c3d0…` | `8d38065aac78097edbfb948c3017b9a80d5188387ee5806ee0824f6daf4f52e2` |
| `Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.renderprofiles.json` | 프로필 300개: `emissiveIntensity=0` 299 + 하늘 돔 `Sky` 1 (sha 앞 16자 `55AB363BED5D0605`, 씬 빌드 receipt 입력과 일치) | |
| `Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapplacements` | 3,839 placements (LFS) | `3d56e5abb06adffc08b7a4c3dcc0081d0a13e7907f92902a82437659044691b2` |
| `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapplacements` | 위와 동일 (LFS) | `3d56e5abb06adffc08b7a4c3dcc0081d0a13e7907f92902a82437659044691b2` |
| `Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.build.receipt.json` | 씬 빌드 receipt (병합 전 382/3,823) | |
| `Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.landscape-merge.receipt.json` | 병합 receipt (base 해시 = 씬 빌드 출력) | |
| `Data/Maps/MapCatalog.json` | `LV_OCN_EVENTIS_MHP` single 행 | |
| `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json` | playerSpawn 4 (광장 20.48m) | |
| `Data/Navigation/LV_OCN_EVENTIS_MHP.navgrid.json` | 균일 160×160, cell 1m, origin (0, -1072), 높이 20.48 | |

씬 빌드 수치: 음수 scale 1,113, 반사 1,109, 원본 숨김 1(`source-hidden`), 숨김 헬퍼 0.

### Resources (Git 비추적, Drive 전달 대상)

- `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP/`: 변형 382종, 소유 영수증 `.lostark-area-install.receipt.json`
- `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP_LAND/Landscape/`: 섬 랜드스케이프 16조각
- 합계 3,360 파일, 715MB. 저장소 루트 `Resource_Distribution_2026-09-19_Maharaka.txt`와 `Copy_ResourceDistribution_2026-09-19_Maharaka.ps1 -Source <받은 Resources> -Destination <본인 Client/Bin/Resources>`

### 저장소 밖 작업 폴더

- `C:\LostArkExtract\LV_OCN_EVENTIS_MHP_20260919`: 배치(원본/승인 사본), 원본 메시·재질·텍스처, 인벤토리·재질 카탈로그, 씬·병합 스테이징, 로그
- `C:\LostArkExtract\MHPrt`: 변형 cook 출력과 런타임 매니페스트

## 5. 도구 수정과 원인

`Tools/LevelPlacementExtractor/build_map_material_variants.py` (쿠크 한 Area로만 쓰였던 경로)

1. 부모 재질 경로: `bg_evt_collabo_chair02_mi_psy`의 부모가 `zzzbg_simple_opa_inst`로만 적혀 cook이 거부됐다. 점 필수 검사를 비어 있지 않은 문자열 검사로 바꾸고 식별은 `contextual_candidate`의 정확히-하나 규칙에 맡겼다. 카탈로그의 정식 경로는 `BG_PRIVATE_PSY_01.zzzbg_simple_opa_inst` 하나다.
2. 텍스처 수화: `bg_anh_foliage_te_b49_mk_old` 등 9장이 UModel export에는 있지만 베이스 팩에는 복사되지 않아 후보 0개가 됐다. 수화 판정을 팩 `textures` 폴더 실존 기준으로 바꿨다. 수화 후 카탈로그는 재질 442, 텍스처 591, gap 0이다.
3. `cook --package-root`: 커밋 560741ac가 `args.package_root`를 참조하면서 인자를 정의하지 않아 `AttributeError`가 났다. 베이스 cook과 같은 선택 인자를 추가했다.
4. 범위 밖 override: `bg_lut_lucastle_houseinfloor02_sm_msj`는 슬롯 1개인데 배치 override가 3칸(0·1 null, 2 MIC)이다. UE3는 메시 요소 수만큼만 조회하므로 슬롯 1·2를 적용하지 않고 receipt `ignoredOutOfRangeOverrides`에 남긴다. 해당 변형 1개다.

운영상 발견: cook 출력 경로가 260자를 넘으면 geometry contract 임시 파일을 찾지 못한다(출력을 `C:\LostArkExtract\MHPrt`로 분리). 변형 install 폴더는 소유 영수증 CAS가 영수증 밖 파일을 거부하므로 랜드스케이프를 `--pack-name LV_OCN_EVENTIS_MHP_LAND`로 별도 폴더에 뽑았다.

## 6. 실행한 검증

| 검증 | 결과 |
|---|---|
| `python -B -m unittest discover -s Tools/LevelPlacementExtractor -p test_build_map_material_variants.py` | 15 tests OK (브랜치 재적용 후 재실행) |
| `Publish-WorldGameplay.ps1 -Mode Validate` | PASS, `Validated MAHARAKA: 4 placements` |
| `Publish-ServerNavigation.ps1 -Mode Validate -AreaId LV_OCN_EVENTIS_MHP` | PASS, 160×160, walkable 25,600 |
| `Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Validate` | PASS, single, 3,839 placements |
| 변형 inventory/hydrate/cook/install | inventory 382, 카탈로그 gap 0, cook 382/382(텍스처 슬롯 미완성 21), install 382 |
| 랜드스케이프 추출 | 46/46, 높이 대조 mismatch 0, 이음매 mismatch 0 |
| `merge_maptool_landscape.py` | 382+16 → 398 assets, 3,823+16 → 3,839 placements |
| `git diff --check` | clean |
| 변경 JSON parse | MapCatalog, Gameplay.world, navgrid.json, receipt 2개 OK |
| 변경 C++/ps1 인코딩·줄바꿈 | 기존과 동일(CRLF, U+FFFD 0), PowerShell 구문 오류 0 |

## 7. 남은 단계와 실행 순서

1. 게시는 09-19 03:01에 아래 세 명령으로 실행했다. 그 뒤 맵 카탈로그를 render profile 반영판으로 다시 만들었으므로 **맵 게시(첫 줄)만 다시 실행**하고 `-Mode Check`로 `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapassets` sha가 `8d38065a…`인지 확인한다. VS를 닫은 상태에서만 실행한다. 데이터 정본을 바꾸면 같은 명령을 다시 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Publish -AreaId LV_OCN_EVENTIS_MHP
```

2. Product 빌드(Engine → Shared → Server → Client). protocol이 94로 바뀌어 Server와 Client를 같이 빌드한다.
3. 02:26에 VS가 `main` 트리로 Client/Server를 다시 빌드했으므로(버튼 없음) 이 브랜치에서 Product 빌드를 다시 해야 한다. 이 작업본으로 빌드한 로컬 Server(127.0.0.1)와 Client로 Debug 로비 → `Maharaka`. `Client.vcxproj.user`는 127.0.0.1, 사용자 환경변수는 10.16.127.103(바로가기 실행용)으로 나눠 두었다. 팀 서버(192.168.0.14, main)는 MAHARAKA 월드가 없고 protocol이 달라 접속이 거절된다.
4. 선택: `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json`, `Data/Navigation/LV_OCN_EVENTIS_MHP.navgrid.json`를 VS `추가 > 기존 항목`으로 Client 프로젝트 `96.DataFiles`에 None 항목으로 노출한다(탐색용, 빌드 무관).

## 8. 범위 밖과 알려진 한계

- HUD: MainApp의 저작 HUD·아이템 강화·보스 게이지 표시 레벨 목록에 MAHARAKA를 넣지 않았다. 섬 표시·이동·복제와는 무관하다.
- Release: Release Lobby에는 스테이지 버튼이 없고 Character Select의 스테이지 진입에도 마하라카를 넣지 않았다.
- Navigation: 균일 높이 20.48m 격자라 섬의 낮은 해변(약 14m)이나 높은 구조물 위에서는 캐릭터가 떠 보이거나 묻혀 보일 수 있다. MapTool bake는 Map Editor Area 목록(하드코딩 5개)에 섬을 추가한 뒤 할 수 있다.
- 표현: 물·바다 메시는 Bern 전용 water presentation이 없어 일반 재질로 그린다. 이미터 105, 포인트라이트 32, 모션 액터 이동, 데칼, 폴리지 인스턴스(1,053 컴포넌트), 컷신은 옮기지 않았다.
- 재질(09-19 재실측으로 정정): 변형 382개 전부 원본 재질을 완전히 읽지 못한다(`materialComplete` 0/382). 원본이 선언한 값 중 런타임이 읽지 않는 것이 텍스처 1,112, scalar 2,387, renderFlag 2,475, vector 506, null 슬롯 23이다. 앞서 적은 "변형 21개만 텍스처 슬롯 미완성"은 텍스처 슬롯 채움 여부만 센 값이라 실제 상태보다 낙관적이었다. 쿠크와 같은 `geometry-preview-partial-material` 수준이며 원본 화면과 같다고 볼 수 없다. 이 Area는 `mapmaterials`가 없어 legacy 경로(wmodel 재질 슬롯 + 카탈로그 render profile)로만 그린다.
- 파랑·노랑 얼룩 제거(카탈로그 데이터만 수정, 게시 전): 변환기는 emissive가 없는 재질에도 자리표시자 `t_tds_specular04`(파랑·노랑 타원 두 개)를 emissive 슬롯에 묶는다. 변형 299개가 이것만 emissive로 갖고 있었고(진짜 emissive가 있는 19개는 유지), `Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.renderprofiles.json`에서 그 299개를 `emissiveIntensity=0`으로 껐다. 근거: `Shader_VtxMeshMapInstance.hlsl` PS_MAIN이 `vEmissive = g_EmissiveTexture * g_EmissiveIntensity`로만 emissive를 그리고, `MapAssetRenderUtils.cpp`의 `Bind_Material`이 `g_EmissiveIntensity`를 카탈로그 행의 `profile.emissiveIntensity`로 묶으며(정적 배치·개별 객체 모두 `desc.renderProfile = asset->renderProfile`), 카탈로그 열 순서(`opacity emissiveIntensity specular ...`)가 파서와 일치한다. 텍스처 파일 자체는 wmodel이 참조하므로 Resources에 남아 있고 화면에 기여만 하지 않는다.
- 알려진 남은 얼룩 1곳: `bg_ocn_etc_gate01_sm ... OVR_9B926B78308E`는 슬롯 0에 진짜 emissive(`gate01_em`)가, 슬롯 1에 자리표시자가 있다. render profile은 에셋(변형) 단위라 슬롯 단위로 끌 수 없어 그대로 두었다(배치 1개). 끄면 진짜 emissive가 함께 사라진다.
- 쿠크 Area(`LV_LUT_MIDNIGHTC_ED`)에는 같은 자리표시자 파일이 506개 설치돼 있다. 이번 작업에서 쿠크는 변경하지 않았다.
- 워터팡 아레나 존(57011) 데이터(프롭 118, NPC 22, 트리거 28)는 넣지 않았다. 프롭 12종은 현재 클라이언트에서 모델 정의가 지워졌다.
- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md` 7장 체크리스트의 `LevelCatalog.json`은 저장소에 없는 파일이다. 이번 작업에서 고치지 않았다.
