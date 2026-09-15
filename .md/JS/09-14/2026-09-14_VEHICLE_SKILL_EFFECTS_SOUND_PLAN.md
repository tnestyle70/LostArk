# 2026-09-14 탈것 스킬 파티클·사운드 PLAN

작성자: JS · 브랜치 `feature/vehicle-material-restoration`(커밋 `30f67113`) 위에서 이어서 작업한다.
선행: [탈것 스킬 RESULT](2026-09-14_VEHICLE_SKILLS_RESULT.md). 탈것 스킬 HUD는 UI 담당 브랜치 범위라 이 문서에서 다루지 않는다.

## G00. 목표와 사용자 결정

탑승 중 Space/Q/W/E 탈것 스킬에 원작 파티클과 사운드를 붙인다. 대상은 현재 로스터 6종 21스킬이다.

| 결정 | 내용 |
|---|---|
| 파티클 | 원본 V1 자동 복원. 팀장의 쿠크 원본 ParticleSystem 복원 파이프라인을 재사용해 게임 패키지에서 `full.restore` V1 문서를 만들고, 원작 notify 시각·소켓으로 탈것 본에 붙인다. 손으로 수치를 찍어 이펙트를 만들지 않는다 |
| 사운드 | 팀장이 추출해 둔 Wwise `Sound` 폴더(`<이벤트>__<wemID>.wav`)를 요청해 설치만 한다. Vehicle 뱅크가 없으면 그때 Wwise 추출 도구 작성으로 전환한다 |
| 순서 | 파티클이 가장 크다. 사운드는 원본 wav를 받는 즉시 짧게 끝나므로 파티클 G와 병렬로 둔다 |

## 1. 실측한 사실

### 1.1 원본 notify

`Tools/LevelPlacementExtractor/extract_action_effect_notifies.py`로 6종 Action `.loa`를 스펙의 skillId 단위로 다시 뽑았다
(`out/VehicleSkillEffects20260914/actions/<Vehicle>.action-effects.json`). 이전 `.animnotify` 집계는 같은 클립을 여러 스킬이 공유하는
action group(튜브·테르페이온)에서 다른 skillId의 헤더를 집어 오차가 있었으므로, 아래 숫자는 action id 기준 추출본이 정본이다.

| 탈것 | action/stage | Particle notify | 고유 ParticleSystem | 쿠크 라이브러리 보유 | AKEvent | 그 밖의 표현 notify |
|---|---|---:|---:|---:|---:|---|
| 황금 테르페이온 | 4 / 4 | 19 | 13 | 1 | 4 | PlaySkeletalMesh 3, MaterialParam 7, CancelParticles 2 |
| 고요한 별빛의 가호 | 4 / 9 | 8 | 4 | 0 | 5 | PlayCameraParticleEffect 1, DirectionalLight 1 |
| 레인보우 모코보드 | 1 / 1 | 1 | 1 | 0 | 1 | DefaultParticle 1 |
| 아우프슈텐-R | 4 / 4 | 40 | 13 | 4 | 4 | PlaySkeletalMesh 3, MaterialParam 7, Footstep 6, CancelParticles 3 |
| 바다 유니콘 튜브 | 4 / 7 | 17 | 13 | 0 | 7 | 없음 |
| 고대의 신화 | 4 / 6 | 24 | 19 | 5 | 11 | ViewShake 1, TrailGhost 1, PostProcessChain 1, DirectionalLight 2, MaterialParam 1 |
| 합계 | 21 / 31 | 109 | **61** (중복 제거) | 8 | 32 | |

- 쿠크 라이브러리에 이미 있는 8개: `fx_cm_00.dust.par_d_dust_001_pr`, `…_007_pr`, `fx_cm_01.distortion.par_mp_concavedis_z_01`,
  `fx_cm_02.light.par_mp_light_01`, `…_01_loop`, `…_05_l`, `fx_post.fx_par.par_c_filmnoise_01`, `…_zoomblur_02`
  (`Data/Effects/Authored/effect.kouku.source.<system>.effect.json`). 나머지 53개는 `FX_VT_*` 탈것 패키지와 공용 `FX_CM_00/02` 일부다.
- PlayParticleEffect payload에는 부착 이름과 파라미터 override가 들어 있다. 예: 신화 대시 `B_R_Wing_02`/`B_L_Wing_02` 두 소켓 동시 스트림,
  별빛 `FX_Dash_01`, 아우프슈텐 `FX_l_Engine_01`, 모코보드 `B_Body_02`, 튜브 `b_effectroot`.
- 한 notify가 스킨 변형별 ParticleSystem을 `CEFParticleDataModifier`로 함께 가진다(`EFDLVehi_MN_PMSDZ_00` → 기본, `-1`~`-4` → 변형).
  ~~우리 탈것 모델은 기본 스킨이므로 첫 기본 참조만 쓴다.~~ G01에서 틀린 전제로 확인했다. `EFTable_Vehicle.Model`이 스킨을 고른다
  (테르페이온 `MN_PMSTG_00-3`, 모코보드 `MN_PMSMK_00-4`, 아우프슈텐 `MN_PMSHE_00-2`, 튜브 `MN_PMSUT_00-6`, 별빛 기본, 신화 `MN_PMSDZ_00-1`).
  해당 스킨 modifier의 system을 쓰고, modifier가 없으면 기본 system을 쓴다. 테르페이온 `Gold` notify 2건은 기본 system 없이 `-3` 스킨에만 있다.
- 탈것 wmodel에 `B_*` 본은 소문자로 존재한다(신화 `b_r_wing_02`, `b_effectroot`, 별빛 `b_root`). `FX_*` 이름은 본이 아니라
  SkeletalMesh socket이므로 원본 socket 계약을 따로 뽑아야 한다.

### 1.2 원본 패키지와 기존 파이프라인

- 쿠크 복원 파이프라인(`Tools/EffectPipeline/build_kouku_gate1_full_restore.py` → `build_kouku_showtime_restore.py` →
  `build_kouku_all_source_effects.py`)은 `C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829`의 CanonicalSource(action JSON, particle-graph,
  Shared `engine`/`efgame` 패키지)를 전제로 한다. **이 PC에는 그 폴더가 없다.**
- 대신 게임 설치본 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC`의 원본 UPK를 `Tools/MoviePipeline/deobfuscate_names.py`
  이름 복원 + `extract_ue3_effect_material_closure.load_package`로 직접 읽을 수 있음을 확인했다. `FX_VT_PMSDZ_00`은 export 5045개,
  `particlesystem` 85개, `particlespriteemitter` 375개, TypeDataMesh 72개가 파싱됐다. `acquire()`에는 이미 `external_package_resolver`
  분기가 있어 graph JSON이 없을 때 UPK에서 레코드를 만든다. 남는 의존은 `source_package('Shared','engine'|'efgame')` 두 곳이다.
- 원본 notify는 `LookInfoAnim` 스테이지를 쓴다. 추출기의 `ANIMATION_NOTIFY_TYPES`에 없어 `animationClips`가 0개로 나오므로, stage 시간 누적은
  탈것 스킬 스펙의 클립 길이(`VehicleSkills.spec.json`)로 계산한다.
- native 재질 프로그램: 쿠크 도메인은 `2304..3711`을 예약했고 현재 874개가 `3620`까지 쓴다(`Client/Private/Effect_ArtistMaterial_Tables.inl`,
  `Shader_EffectArtistNativeDispatchKoukuNativeCases<64단위>.hlsli`). 남은 91개로는 탈것 신규 재질을 담는다고 보장할 수 없으므로
  탈것은 별도 64단위 구간을 새로 연다.
- 런타임 V1 재생은 `CEffectPresentationService::Spawn(EFFECT_SPAWN_DESC)`이다. 소유자 뷰 `EFFECT_OWNER_VIEW::Get_Model()`이
  캐릭터 body 모델만 돌려주므로(`Effect_PresentationService.cpp:195`) 탈것 본·소켓 부착은 새 소유자 모드가 필요하다.

### 1.3 사운드

- 원본 이벤트는 `S_Vehicle`, `S_Vehicle2~4` 뱅크의 AKEvent 32건이다(예: `S_Vehicle2.S_Vehicle_TrisionHorse_Skill1`,
  `S_Vehicle4.S_Vehicle_AncientDragon1_Dash1`).
- 게임에는 `WwiseAudioPackage/SOUND_VEHICLE`, `SOUND_VEHICLE_2026`, `SOUND_VEHICLE_2026_NONSTREAM` `.pck`가 있다. 이 PC에는 이벤트→wem 해석기와
  wem 디코더(vgmstream)가 없고, 기존 `build_sound_catalog.py`의 원본 경로 `D:\로아 리소스`도 없다.
- 캐릭터 재생 경로는 `CSoundCueCatalog::Find_Variants(class, event)` → `CGameInstance::Play_Sound`(`Character.cpp` `Update_SoundCues`)다.
  카탈로그 정본은 `Data/Sound/CharacterSoundCatalog.json`(`classes.<Class>.<event>: [wav…]`), 파일은 `Resources/Sound/<Class>/`.

## 2. 설계

```text
Action .loa ──extract_action_effect_notifies──▶ action-effects.json (skillId별)
                                                   │ PlayParticleEffect / AKEvent
게임 UPK ──(이름 복원 + load_package)──▶ source closure (PS → emitter → LOD → module → material/mesh/texture)
                                                   │ 기존 acquire / project / native program 생성 재사용
                                                   ▼
Data/Effects/Authored/effect.vehicle.<archetype>.<skillId>.<clipIndex>.full.restore.effect.json  (+ EffectCatalog 등록)
Resources/Effect/Vehicle/{Meshes,Textures,FullRestore}/…
                                                   │
Data/Actors/VehicleCatalog.json formatVersion 3: skills[].effectCues[] / soundCues[]
                                                   │
CCharacter VEHICLE_SKILL ─ action age ─▶ CEffectPresentationService::Spawn (소유자=캐릭터, 부착=탈것 모델)
                                    └─▶ CSoundCueCatalog("Vehicle") → Play_Sound
```

### 2.1 문서 단위

- **스킬 클립 한 개 = V1 문서 한 개.** 한 클립의 모든 Particle notify를 시각 offset 그대로 한 문서에 담는다(쿠크 `4219801` 불뿜기와 같은 단위).
  두 소켓 notify는 소켓별 독립 스트림 요소로 늘린다. 이미 라이브러리에 있는 8개 시스템도 문서 안에서는 같은 프로젝션으로 다시 투영한다
  (라이브러리 문서는 root 고정 미리보기 전용이라 부착·시각을 갖지 않는다).
- ID: `effect.vehicle.<archetypeId 소문자>.<skillId>.<clipIndex>.full.restore`. 예: `effect.vehicle.vehicle_ancient_myth.98520.0.full.restore`.
- 문서 수: 클립 기준 최대 31개(Particle notify가 없는 클립은 만들지 않는다).

### 2.2 데이터 계약 — `VehicleCatalog.json` formatVersion 3

skill 항목에 두 배열을 더한다. 둘 다 생성기가 쓰고 사람이 고치지 않는다.

```json
{
  "skillId": 98520,
  "inputSlot": "SPACE",
  "vehicleClips": ["npc_sk_dash"],
  "riders": [ { "characterClass": "WARLORD", "clips": ["wgl_ride_dragon_2_sk_dash"] } ],
  "effectCues": [
    { "clipIndex": 0, "effectAssetId": "effect.vehicle.vehicle_ancient_myth.98520.0.full.restore",
      "startMs": 0, "stopPolicy": "NATURAL" }
  ],
  "soundCues": [
    { "clipIndex": 0, "event": "S_Vehicle_AncientDragon1_Dash1", "startMs": 1 }
  ]
}
```

- `clipIndex`는 `vehicleClips` 인덱스, `startMs`는 그 클립 시작 기준. 문서 내부 요소가 notify별 지연을 이미 갖고 있으므로 문서 cue의 `startMs`는 보통 0이다.
- 부착 본·소켓·파라미터 override는 V1 문서 요소의 `actionCueAttachment`/`sourcePresentation`이 소유한다. 카탈로그에 중복 저장하지 않는다.
- Client `CActorCatalog`는 v2와 v3를 모두 읽는다. v3의 알 수 없는 effectAssetId는 해당 cue만 거부하고 탈것·스킬은 유지한다.

### 2.3 런타임

| 위치 | 변경 |
|---|---|
| `Client/Public/Effect_PresentationService.h` `EFFECT_SPAWN_DESC` | `bool_t bVehicleModelAnchors = false;` 추가. 켜지면 소유자 뷰의 모델·presentation root를 소유 캐릭터의 탈것 파츠로 바꾼다 |
| `Client/Private/Effect_PresentationService.cpp` `EFFECT_OWNER_VIEW` | `Get_Model()`·`Try_Get_PresentationRoot()`가 `bVehicleModelAnchors`면 `CCharacter::Get_VehicleModel()`·`Try_Get_VehicleRootMatrix()`를 쓴다. 탈것이 없으면 spawn 실패(캐릭터 body로 대체하지 않는다). `ACTIVE_EFFECT`에 같은 플래그를 보존해 follow 샘플도 같은 모델을 쓴다 |
| `Client/Public/Character.h` / `Character.cpp` | `Get_VehicleModel()`, `Try_Get_VehicleRootMatrix()` 공개. `Apply_NetworkAction`의 VEHICLE_SKILL 분기에서 `Update_VehicleSkillCues(skill, age)`를 호출해 이전 age~현재 age 사이에 시작하는 cue를 한 번씩 spawn(늦은 합류는 `fInitialSampleTimeSeconds`로 catch-up). 새 action start tick이면 발생 기록을 초기화 |
| 종료 | action이 끝나거나 하차하면 `CUE_END` 요소만 정리되고 `NATURAL` 잔여 파티클은 끝까지 재생한다. 하차로 탈것 파츠가 사라지면 follow 앵커 소실로 기존 경로가 해당 효과를 멈춘다 |
| 준비 | `CVehiclePresentationAssetService::Ensure_Prototypes` 성공 직후 그 탈것의 모든 effectAssetId를 `CEffectPresentationService::Queue_ProductTargets_Priority`에 넣는다. 전투 중 spawn은 준비된 캐시만 쓴다(기존 계약) |
| 사운드 | 같은 분기에서 `soundCues`를 `CSoundCueCatalog::Find_Variants("Vehicle", event)`로 찾아 `Play_Sound`. 로컬·원격 탑승자 모두 재생(캐릭터 스킬과 동일) |

Server·Shared는 바뀌지 않는다. 스킬 시작 tick과 탈것 ID는 이미 snapshot에 있다.

## 3. G 목록

| G | 내용 | 산출물 | 종료 증거 |
|---|---|---|---|
| G01 | 원본 closure 게이트. 게임 UPK resolver(이름 복원 인덱스)와 `engine`/`efgame` 물리 패키지 대체를 넣어 6종 61개 시스템의 closure를 뽑는다 | `Tools/VehiclePipeline/build_vehicle_skill_effects.py --acquire-only`, `out/VehicleSkillEffects20260914/{source_notifies,source_occurrences,first_lod_module_closure_validation,external_module_closure}.json` | 61개 시스템 모두 첫 LOD module 참조 누락 0, 해소 실패 목록 0 또는 원인별 목록. 원본 재질 수·렌더러 shape 집계 |
| G02 | socket 계약. 6종 SkeletalMesh의 socket(본 이름·offset·회전)을 원본에서 뽑는다 | `out/VehicleSkillEffects20260914/source_socket_contract.json` | notify가 참조한 모든 부착 이름이 본 또는 socket으로 해소 |
| G03 | native 재질 프로그램. G01 재질 목록으로 원본 셰이더를 복원하고 탈것 전용 64단위 구간을 등록한다. 생성기의 `--profile-domain`은 현재 `artist`/`kouku`만 받으므로 `vehicle`을 추가한다 | `generate_artist_native_runtime_shader.py --profile-domain vehicle` 계약, `Effect_ArtistMaterial_Tables.inl`, `Shader_EffectArtistNativeDispatchVehicleNativeCases*.hlsli`, dispatch/project/filter, installer 구간 확장 | 설치기 재실행 changed=False, Debug Product 빌드(CSO 포함) PASS |
| G04 | 프로젝션·설치. 클립별 V1 문서 생성, Resources 설치, EffectCatalog·`Client.vcxproj(.filters)` 96.DataFiles 등록 | `Data/Effects/Authored/effect.vehicle.*.full.restore.effect.json`, `Resources/Effect/Vehicle/…` | 문서 validator·Resources 실재 검사 PASS, `Validate-EffectSources.ps1` PASS, 원본 notify 109건 ↔ 문서 요소 분모 대조 |
| G05 | 카탈로그 v3. 생성기가 `effectCues`/`soundCues`를 쓰고 Client 파서가 읽는다 | `Data/Actors/VehicleCatalog.json`, `Client/Public/ActorCatalog.h`, `Client/Private/ActorCatalog.cpp` | JSON parse, v2 입력 호환, 잘못된 cue 단일 거부 |
| G06 | 런타임 연결. 탈것 앵커 소유자 모드, VEHICLE_SKILL cue 재생, 준비 큐 | `Effect_PresentationService.h/.cpp`, `Character.h/.cpp`, `VehiclePresentationAssetService.cpp` | Debug 빌드 PASS, 사용자 화면 확인(탈것별 Space/Q/W/E 파티클 위치·시각) |
| G07 | 사운드. 팀장 추출본에서 32건 이벤트 wav를 찾아 설치하고 카탈로그 `Vehicle` 클래스를 만든다 | `Tools/VehiclePipeline/build_vehicle_sound_catalog.py`, `Data/Sound/CharacterSoundCatalog.json`, `Resources/Sound/Vehicle/…` | 이벤트 매칭표(누락 0 또는 목록), 사용자 청취 확인 |
| G08 | 문서 | RESULT, `CLAUDE.md` 탈것 문단, `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`는 변경 없음 확인 | |

G01이 실패하면(원본 closure를 게임 UPK만으로 닫지 못함) G02 이후를 시작하지 않고 원인과 대안(팀장 CanonicalSource 전달 요청)을 보고한다.

## 4. 위험과 확인 지점

| 항목 | 확인 방법 |
|---|---|
| `engine`/`efgame` CDO를 게임 설치본에서 읽을 때 CanonicalSource와 다른 export가 섞일 수 있다 | G01에서 쿠크 기존 closure 하나(`4219801`)를 같은 resolver로 다시 뽑아 기존 `source_occurrences.json`과 비교 |
| 쿠크 파이프라인 모듈 전역(`source.SELECTED`, `library.TARGETS`) 치환 방식 | 탈것 생성기는 기존 모듈을 import하고 선택 목록만 주입한다. 쿠크 스크립트 파일은 수정하지 않는다 |
| AnimationTrail notify는 baked edge history가 필요하다 | G01 집계에 trail shape가 나오면 해당 요소만 제외 목록으로 보고 |
| 탈것 모델 preScale 0.0001과 원본 cm 단위 | 쿠크 Trails의 local scale 보정 사례처럼 부착 행렬 스케일을 G04에서 실측 기록 |
| PlaySkeletalMesh·MaterialParam·CameraParticle·PostProcess·DirectionalLight·ViewShake·TrailGhost | 이번 범위 밖. notify 목록은 evidence에 남긴다 |
| 사운드 추출본에 Vehicle 뱅크 없음 | G07 첫 단계에서 확인. 없으면 Wwise `.pck`/`.bnk` 해석 + vgmstream 디코드 도구 작성 계획으로 교체 |

## 5. 검증

| 검사 | 기준 |
|---|---|
| G01 closure | 61 시스템, 누락 0 |
| V1 문서 | 기존 validator(부착·native sprite·color space·module override) PASS |
| `Tools/EffectPipeline/Validate-EffectSources.ps1` | PASS |
| native 설치기 재실행 | changed=False |
| Debug Product 빌드 | Engine/Shared/Server/Client PASS |
| `git diff --check`, JSON/XML parse | PASS |
| 사용자 화면·청취 확인 | 탈것 6종 Space/Q/W/E 파티클 부착·시각, 사운드 |

## 범위 밖

- 탈것 스킬 HUD(UI 담당 브랜치), 스킨 변형 파티클(`-1`~`-4`), 은색 전투 랩터.
- ViewShake·PostProcessChain·DirectionalLight·PlaySkeletalMesh·MaterialParam·TrailGhost·Footstep notify.
- 튜브 추가 키 입력 분기.
