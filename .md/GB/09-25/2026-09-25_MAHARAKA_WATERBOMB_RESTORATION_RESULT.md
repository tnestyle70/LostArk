# 마하라카(워터밤) 맵 원본 복원 RESULT

작성 시작: 2026-09-25. 대상 Area `LV_OCN_EVENTIS_MHP` (Lobby `Maharaka` → `LEVEL::MAHARAKA`).
이 문서는 단계마다 누적 기록한다. 사용자의 화면 확인 전에는 "원작 일치/PASS"라고 쓰지 않는다.
에이전트는 Client와 게임 UI를 실행하지 않았고 수치·파일·게시 검사만 했다.

## 1. 기준선: 베른·발탄·쿠크는 어떤 레이어로 복원됐나

각 Area는 다음 레이어를 가진다(`Client/Bin/DataFiles/Map` 실측).

| Area | mapmaterials | maplights | mapwater | mapeffects | 기타 |
|---|---|---|---|---|---|
| 베른 `LV_BER_BERNCASTLE` | 23,153행 | 있음 | 있음 | 있음 | 23 shard, 50,017 placement, RNM·정적 그림자, 환경 영역 5 |
| 발탄 `LV_LUT_HEARTRB_ED` | 있음 | 있음 | 없음 | 있음 | camerashots, worldsequences, deploy |
| 쿠크 `LV_LUT_MIDNIGHTC_ED` | 1,732행 | 있음 | 없음 | 없음 | RNM 2,364, 환경 영역 11, camerashots, worldsequences |
| 마하라카 `LV_OCN_EVENTIS_MHP` (작업 전) | **없음** | **없음** | **없음** | **없음** | mapassets/mapplacements만 |

복원 절차(문서 근거: `Tools/LevelPlacementExtractor/README.md`, `.md/GB/렌더링이펙트복원V2.md` 공통 절차,
`.md/GB/09-11/2026-09-11_{BERN_MATERIAL_RESTORE,KOUKU_FULL_MAP_MATERIAL_RESTORE,BERN_LIGHTING_GEOMETRY_IMPLEMENTATION}_RESULT.md`):
placement schema3(source visibility) → WModel 추가 채널 cook → source material 파라미터/mip 추출 →
component 조명(RNM, ShadowMap2D) → `mapmaterials` 컴파일 → scene 빌드 → Area publisher.
베른 조명·재질 복원에 쓰인 개별 스크립트(ShadowMap2D 해독 등)는 저장소에 남아 있지 않다(확인: Tools 전체 grep).
`build_source_map_materials.py`의 입력 manifest를 만드는 orchestrator도 저장소에 없다.

## 2. 마하라카의 기존 상태 (작업 전 실측)

- 데이터: Imported `mapassets`(398 asset)·`mapplacements`(3,839)·`renderprofiles`(300행, emissive 0)·receipt 2개, Authoring `mapplacements`.
  `Client/Bin/DataFiles/Map`에는 `mapassets`, `mapplacements`만 있다. 게시 해시: mapassets `8d38065a…`, mapplacements `3d56e5ab…`.
- Resources: `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP` 382 폴더/706 MB, 랜드스케이프 `..._LAND` 16조각. 설치된 DDS 2,494개는 전부 mip0만 있다.
- placement 추출은 schema3(actor/component/archetype/CDO source visibility)로 이미 수행됐다: SL01 3,823(visible 3,822/hidden 1), PS 244(visible 1), LAND01 82(visible 82).
  원본 y>50000cm(섬) 범위 규칙으로 SL01 3,820 + LAND01 2 + PS 1 + 랜드스케이프 16조각을 채택했다(레이싱 쪽·숨김 컬링 박스 제외).
- 2026-09-19 RESULT가 스스로 적은 한계(이번 요청과 일치): 재질 완전 복원 0/382(`materialComplete`), 물이 Opaque, 조명 없음, 이미터 105·포인트라이트 32·모션 액터·데칼·폴리지 1,053 컴포넌트·컷신 미이식, 워터팡 아레나 존(57011) 데이터 미이식.
- 환경: Level 진입 profile은 `scene.development.neutral.v1`(중립: diffuse 0.8, ambient 0.25, 안개 없음). 원작 태양/하늘/안개/후처리가 없다.

### 2-1. 이전 스테이징(Fork B, 09-19)의 존재

`C:\LostArkExtract\LV_OCN_EVENTIS_MHP_20260919\Restore` 아래 스테이징 산출물이 있고 **저장소에는 적용된 적이 없다**.
스테이징 시점 live 해시(mapassets `8d38065a…`, mapplacements `3d56e5ab…`)가 오늘 live 해시와 같아 그대로 적용 가능하다(확인함).
내용: 재질 lane 교정(placeholder 묶임 제거), 원본 mip 체인 1,342 DDS, 물 10행 + renderMode Water, 점광원 32 + 스포트 1, 섬 식생 812 인스턴스/8 asset, 자체 모션 65행, 배치 4,651/자산 406.

## 3. 원본 레벨 전수 조사 (읽기 전용, 이번 작업에서 실측)

도구: `out/MaharakaRestore20260925/tools/dump_level_classes.py`, `dump_env_details.py`(저장소 UPK 리더 재사용, 원본 패키지는 읽기만).
산출물: `out/MaharakaRestore20260925/audit/level_classes.json`, `env_details.json`.

| 원본 package | export 수 | 주요 구성 |
|---|---:|---|
| `LV_OCN_EVENTIS_MHP_PS` (persistent) | 1,109 | StaticMeshActor 243, BSP 브러시 42/볼륨(PathBlocking 28, AreaName 7 등), **DominantDirectionalLight 1, ExponentialHeightFog 1, EFEnvironmentInfoData/Volume 1, WorldInfo(후처리·Lightmass), WindDirectionalSource 1, InstancedFoliageActor 1**, ShadowMap2D 240 |
| `LV_OCN_EVENTIS_MHP_SL01` (섬) | 8,977 | StaticMeshComponent 3,826, StaticMeshActor 1,489, StaticMeshCollectionActor 24, EFMotionStaticMeshActor 59(+모션 LocationCycle 53/RotationCyclic 12), InterpActor 12, **Emitter 105, PointLight 32, SpotLight 1, SkyLight 1**, EFTranslucentVolume 6, Decal 2, EFSkeletalMeshActor 1, Matinee 1, ShadowMap2D 3,027 |
| `LV_OCN_EVENTIS_MHP_LAND01` | 15,033 | Landscape 46조각, **InstancedStaticMeshComponent 1,053**, StaticMeshComponent 82, DecalComponent 23, ShadowMap2D 10,152, InstancedFoliageSettings 22 |
| `LV_OCN_EVENTIS_MHP_SL02` | (미조사) | 레이싱 트랙 쪽으로 추정. 조명 17개 |

component 조명 evidence(`Restore/lighting/components.json`): SL01 3,823 중 **UNSUPPORTED_NATIVE_LAYOUT 3,027(ShadowMap2D)**, RNM 725, NO_LOD 71.
따라서 팀 공통 source-material compile(`build_source_map_materials.py`)은 ShadowMap2D 해독기(저장소에 없음) 없이는 통과하지 못한다.

### 3-1. 원본 환경 값 (실제 패키지에서 추출)

| 항목 | 원본 값 | 근거 |
|---|---|---|
| 태양 방향 | pitch −51.04°, yaw 209.14° → Client 좌표 방향 (−0.5492, −0.7776, 0.3061) | DominantDirectionalLight_0 rotation. 변환식 (x, z, −y)는 베른(pitch −40°, yaw 120°)이 베른 profile 방향 [−0.383, −0.643, −0.663]을 재현해 검증 |
| 태양 밝기/색 | brightness 1.4, color (255,254,203) → diffuse (1.400, 1.395, 1.115) | EFEnvironmentInfoData `ddl_override` (섬 환경 볼륨 override). 컴포넌트 자체 밝기는 836 KB 네이티브 payload 때문에 파서 미해독 |
| 하늘빛 | SkyLight brightness 0.3, lightcolor (255,230,221), lowercolor (223,236,255) | SL01 SkyLightComponent |
| Lightmass 환경색 | (242,244,255) | WorldInfo lightmasssettings |
| 안개(기본) | density 0.1, heightFalloff 0.7, maxOpacity 0.2, startDistance 1600cm, opposite (119,213,255)×0.5, inscattering (126,246,255)×0.5 | ExponentialHeightFogComponent |
| 안개(환경 볼륨 override) | density 0.1, heightFalloff 3.0, maxOpacity 0.2, start 1600, opposite (122,182,255)×2.0, inscattering (195,101,245) | EFEnvironmentInfoData, 볼륨 위치 = 섬 중심(74.9, 87.0, −984.3 m) |
| 후처리 | bloom threshold 0.4(볼륨 0.5), scale 1, tint (255,178,159), scene_colorize (1,1,1.15), midtones (0.9,1,1), AO power 1.2, DOF far blur 0.4, colorgrading LUT는 외부 package 참조(−90) | WorldInfo DefaultPostProcessSettings, EFEnvironmentInfoVolume settings |

프로젝트 adapter 규칙(베른에서 역산해 검증): diffuse = color/255 × brightness, ambient = env color/255 × intensity(베른 (205,223,255)×0.9가 profile [0.7235,0.7871,0.9]를 정확히 재현), bloom tint = 색/255, fog start = cm/100.
색 byte→선형 변환 자체는 "project adapter"이며 원본 CPU packing 확인이 아니다(기존 베른 문서와 동일한 경계).

## 4. 레이어별 결과 (작업 후)

| 레이어 | 결과 | 근거 |
|---|---|---|
| 재질 lane 교정 + mip 체인 | **완료(설치·게시)** | 섬 382 variant 재쿠킹본 설치, DDS 2,494개 중 1,342개가 원본 mip 체인. placeholder 묶임 emissive 384→4, opacity 77→0(2 lane), specular 277→23(Fork B `recook/verify.json` 수치이며 이번 작업에서 재측정하지 않음). 소유 영수증 CAS 통과 |
| 물 | **완료(게시)** | `mapwater` 10행(`SOURCE_MATERIAL_EXACT`, 원본 부모 재질 chain 보존), 카탈로그 renderMode Water 10개, 참조 텍스처 전부 존재. `MapAssetCatalog`가 자동 로드 |
| 점광원 33 | **완료(게시 + C++ 연결)** | `maplights` 33개(POINT 32 + SPOT 1, 원본 SL01 인스턴스). Development 셸 `Ready_Lights`/`Update`가 MAHARAKA만 제출 |
| 식생 812 | **완료(게시)** | 원본 InstancedStaticMesh 디코드, 섬 범위 812개, 신규 asset 8종(`Map/LV_OCN_EVENTIS_MHP_FOLIAGE`). 배치 3,839→4,651, asset 398→406 |
| 자체 모션 65 | **완료(문서 + C++ 연결)** | `mapmotions` 65행(EFActorMotion LocationCyclic/RotationCyclic). runtime이 직접 읽고 MAHARAKA만 샘플링 |
| 환경(태양·하늘빛·안개·후처리·LUT) | **완료(게시 + C++ 연결)** | `scene.maharaka.source-rendering.v1`, LUT `Map/Lighting/Maharaka/lv_ocn_dookyis_lut.dds`, LevelRegistry의 MAHARAKA profile id 교체 |
| RNM/ShadowMap2D 조명, `mapmaterials` 재질 컴파일 | **미완료(막힘)** | 아래 8장 |
| 이미터 105, 데칼 25, 번역 볼륨 6, 스켈레탈 1, Matinee 1 | **미완료(인벤토리만)** | 아래 8장 |
| 환경 볼륨 region(brush 해독), 워터팡 아레나 존(57011) | **미완료** | 아래 8장 |

## 5. 실행 내역

1. 백업(`out/MaharakaRestore20260925/backup/`): live Data(Imported/Authoring/MapCatalog/RenderingProfiles/LevelRegistry/Level_Development), DataFiles Map 2개, 기존 Resources `Map/LV_OCN_EVENTIS_MHP` 3,254개 727,814,022 bytes(원본과 파일 수·바이트 수 일치).
2. Resources 설치: `build_map_material_variants.py install` 두 번. 섬 706→910 MB(382 variant), 식생 폴더 신규 13 MB(9 variant), LAND 22 MB 불변. 각 Area의 소유 영수증 CAS 검증 통과(3,253 / 50 파일).
3. 문서 적용(`tools/apply_docs.py`, compare-and-swap + 원자 교체): Imported mapassets·mapplacements·landscape-merge receipt·build receipt·renderprofiles(307행), Authoring mapplacements·maplights·mapwater, DataFiles mapmotions, MapCatalog 마하라카 행(placementCount 4,651, assetCount 406, water/lights pair 추가. diff는 이 행 7줄 추가/3줄 삭제뿐).
4. 게시: `Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP` Validate → Publish → Check 모두 exit 0(placement 4,651, 출력 4개). 게시 해시가 스테이징 README 기록과 네 개 모두 정확히 일치: mapassets `cabd4281`, mapplacements `1b169867`, mapwater `312651b1`, maplights `3d69ad79`.
5. 환경 profile: `Data/Rendering/Authored/RenderingProfiles.json`에 profile 1개 추가(+47줄, revision 77→78), `Publish-RenderingProfiles.ps1` Validate/Publish PASS(runtime +146줄/−1줄, 다른 profile 변경 없음).
6. C++(ASCII·CRLF 유지, 앵커 패치): `LevelRegistry.cpp`(MAHARAKA profile id), `Level_Development.h`(light runtime 멤버), `Level_Development.cpp`(`Ready_Lights` 원본 조명 로드, `Update`의 자체 모션·조명 제출, `Initialize`의 자체 모션 로드). 모두 `LEVEL::MAHARAKA`일 때만 동작하고 Training/Map Editor 경로는 바뀌지 않는다.
7. Debug Product 빌드 PASS(393초; Server 91초, Client 298초), 실행 데이터 검사 36건 누락/손상 0. 이 빌드는 다른 fork가 작업 트리에 둔 서버·Client 변경도 함께 컴파일했다.
8. 팀원 전달 목록: 저장소 루트 `Resource_Distribution_2026-09-25_MaharakaRestore.txt` + `Copy_ResourceDistribution_2026-09-25_MaharakaRestore.ps1`(3,306 파일, 958 MB; `-ReplaceAreaFolders`로 이전 마하라카 폴더 정리). 복사 스크립트는 실행하지 않았다(PowerShell parse 오류 0).

## 6. 원본 환경 값 → profile 대응 (근거는 3-1절과 gotchas 2026-09-25 절)

| 항목 | 값 |
|---|---|
| light.direction | (−0.5492, −0.7776, 0.3061) |
| light.diffuse | (1.400, 1.395, 1.115) |
| light.ambient | (0.949, 0.957, 1.000) — Lightmass 환경색 (242,244,255) × 기본 intensity 1.0 |
| fog | color (0.957, 1.427, 2.0), density 0.1, heightFalloff 3.0, topHeight 19.06 m, start 16 m, maxOpacity 0.2, inscattering (0.765, 0.396, 0.961), lightDirection = −태양, w = cos 45° |
| 후처리 | bloom threshold 0.5(섬 볼륨) / scale 1.0 / tint (1, 0.698, 0.624), midtones (0.9, 1, 1), colorize (1, 1, 1.15), toneScale 1 / range 8 / toe 1, LUT 256×16(항등 대비 평균 편차 16/255) |

추정·근사로 표시할 것: 섬 환경 볼륨 override를 base에 넣었다(brush 미해독), `environmentintensity` 미직렬화를 기본 1.0으로 해석, 태양 컴포넌트 자체 밝기(836 KB 네이티브 payload)는 미해독이라 override 값 사용, 색 byte→선형 변환은 project adapter, AO power 1.2·DOF 값은 profile 필드가 없어 미반영.

## 7. 검증 수치와 한계

- 카탈로그 406 asset 전부 모델 파일 존재, placement 4,651 전부 알려진 asset 참조, 렌더 모드 Opaque 395 / Water 10 / Sky 1.
- 재질 커버리지(설치 receipt): textureDependencyClosure 382/382, textureSlots 361/382(21 미완성), **materialComplete 0/382**, sourceOnlyUnsupported 382. 즉 각 재질의 원본 파라미터 전체를 런타임이 읽지 못하는 상태는 그대로다(09-19 결과와 동일). placeholder 묶임 잔존: emissive 4, specular 23, normal 41, material 71.
- mip 없는 텍스처 28종은 원본 패키지에서 못 찾음(엔진 placeholder/FX 텍스처 등) 그대로 mip0.
- 이 검증은 파일·해시·문서 계약이다. C++ 문서 소비(Water/조명/모션 로드)와 화면은 실행하지 않았다. 원본 화면과의 일치 여부는 판정하지 않는다.

## 8. 막힌 항목과 이어서 할 작업 (근거 포함)

1. **`mapmaterials` 재질 컴파일과 RNM/그림자 조명** — SL01 3,823 중 3,027개가 ShadowMap2D(evidence 미구현), RNM 725, NO_LOD 71.
   - ShadowMap2D는 193바이트 tagged struct로 전부 해독됨(texture ref, coordinateScale/Bias, lightGuid). 그림자 그림은 Crunch 압축 G8 `ShadowMapTexture2D` 52개(SL01)/1,113개(LAND01) payload에 있어 **Crunch 해독기**가 필요하다(저장소에 없음).
   - `build_source_map_materials.py`의 입력 manifest를 만드는 orchestrator도 저장소에 없다. 다음 fork는 (a) Crunch 해독기, (b) component 조명 evidence 확장, (c) 입력 manifest orchestrator, (d) 컴파일→scene→publish 순서로 진행한다. 베른 방식(원본 재질 23,153행)과 같은 수준이 되려면 이 항목이 핵심이다.
2. **이미터 105개**: 16종 particle template(glow 18, wave 17, fall 13, geyser 9, lutwater 9, ocn_wave 9, waterfall 6 …), 좌표는 `audit/emitters_decals.json`. 현재 Effect 문서가 있는 template은 2종(`par_a_h_waterwave_001` 5개, `par_d_fallmist_w3_002` 2개)뿐이라 나머지는 Effect 복원이 선행되어야 한다.
3. **데칼 25개**(SL01 2 + LAND01 23, 섬 범위 11): map decal runtime 소비자가 없다.
4. **환경 볼륨 region**: `EFEnvironmentInfoVolume` brush(BSP Model/Polys) 해독기가 없어 region bounds/planes를 만들지 못했다.
5. EFTranslucentVolume 6, EFSkeletalMeshActor 1, Matinee 1, WindDirectionalSource 1: 소비자/변환기 없음. SL02(레이싱 트랙, 조명 17)는 범위 밖.
6. 워터팡 아레나 존(57011): 프롭 118, NPC 22, 트리거 28은 별도 gameplay/Deploy 작업.
7. translucent 비-ocean 변형 24종은 여전히 opaque, 다중 슬롯 ocean asset은 publisher가 자산당 water 1행만 허용, landscape 채도는 원본 화면 없이 판정 불가(09-19 기록 유지).

## 9. 사용자 확인 경로와 주의

- 새 Debug 빌드의 Client를 다시 시작하고 `Lobby → Maharaka`(Server의 마하라카 승인 경로는 바뀌지 않았다). 확인할 것: 바다 표면이 물로 보이는지(이전에는 바다 텍스처가 섬 전체에 보임), 파랑·노랑 얼룩이 사라졌는지, 식생 추가, 원본 점광원 33개, 회전/흔들리는 소품, 태양 방향·하늘빛·안개·색보정. F1 Rendering Workbench에서 profile 값을 조절할 수 있다. 화면 판단은 사용자 몫이다.
- Resources 변경은 Git 비추적이다. 팀원은 위 배포 목록을 받아야 하고, Data/C++는 같은 commit이어야 한다. `.mapassets`/`.mapplacements`는 Git LFS 대상이라 수동 병합하지 않는다(Area 담당 1명).
- 롤백: `out/MaharakaRestore20260925/backup/`의 Data/DataFiles/Resources 백업과 `tools/apply_docs.py`의 역순으로 되돌릴 수 있다.
- 이 작업으로 protocol, Server gameplay, 네비게이션, 다른 Area의 데이터·Resources·RenderingProfiles 항목은 바뀌지 않았다.
