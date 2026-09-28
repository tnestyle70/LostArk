# 2026-09-28 마하라카 워터팡 물총 RESULT

## 상태 요약

| 구분 | 상태 |
|---|---|
| 원본 데이터 체인·물총 모델·소켓 확인 | 완료 (추출 근거 아래) |
| 물총 모델·텍스처·7개 클래스 AnimSet 쿠킹·설치, CY_Resources 복사 | 완료 |
| 발사 이펙트 4종 source 복원·native program 설치·Authored/Catalog 설치 | 완료 |
| 49개 요소 배치·방향 수치 검증 | 완료(수치 진단). 화면 판정은 사용자 몫 |
| Server 판정 + protocol 118 복제 + Client 장착·무기 숨김·이동 애니 교체 | 소스 반영 완료, **빌드 미실행** |
| Debug F1 미리보기(무장 토글·발사 1~6)·진단 로그 | 소스 반영 완료, 빌드 미실행 |
| 스킬·PvP 판정 | 범위 밖(다음 단계) |
| 사용자 화면 확인 | 미실시 — visual PASS 아님 |

## 1. 원본 데이터 체인

- `EFTable_Prop` 15000/15002 "워터 프로MK-1"은 Model `EFDLProp_ITR_02164`, CommonActionIndex 54015, 스킬 56900/56910/56920, 기본 56930이다. MK-2(15010)는 `ITR_02165`, 57000 계열을 쓴다.
- CommonAction 54015 `BATTLE_RIFLE_Carry_S`(소총 들기)는 `PR_ITR_02164_Start_1`, 54016(내려놓기)은 `PR_ITR_02164_End_1`을 쓴다.
- LookInfo `EFDLProp_ITR_02164.loa`(data4.lpk)는 `CEFData_PartsMesh`이며 플레이어 소켓은 `SC_Prop3_01`이다. 7개 클래스 패밀리(PC_FT_00, PC_GN_F_00, PC_WR_F_00, PC_SP_00, PC_SP_M_00, PC_WR_00, PC_DL_00) 모두 이 소켓을 `bip001-prop3`, 오프셋 0으로 둔다. 설치된 7개 body wmodel에도 모두 `bip001-prop3`가 있다.
- 워터팡(마하라카) 액션은 `XmlData/Action/GADGET.loa`의 "[마하라카]" 변형이다.

| Action | 이름 | 클립 | 파티클(소켓 FX_Prj_01) | 사운드 |
|---|---|---|---|---|
| 56932 | 평타 | Att_1_01 | `FX_BS_07.Gadget.Par_L_WaterGun01_Sk_03` t=0.3921 | Cast1 t0, Shot1 t0.2 |
| 56902 | 연발 | Att_2_01 | `Par_L_WaterGun01_Sk_01` t=0.6867 | Shot1 t0.55 |
| 57032 | 범위 | Att_3_01 | `Par_L_WaterGun02_Sk_03` t=0.5931 | |
| 57002 | 한방 | Att_4_01 | `Par_L_WaterGun02_Sk_01` t=0.689 | |
| 56912 | 물폭탄 | Att_5_01 | 없음 | |
| 57012 | 물지뢰 | Att_6_01 | 없음 | |

- 사운드는 `S_Gadget.Gadget_WaterPistol1_*`이며 이번 작업에서는 연결하지 않았다(다음 단계인 스킬 연결 범위).
- 경기 시작 트리거 57011의 ContentsBuff 5700902→5700901은 Client 데이터에 없다(Server 전용).

## 2. 쿠킹된 에셋 (Client/Bin/Resources 상대 경로)

| 경로 | 크기 | sha256 앞 16자리 |
|---|---|---|
| `Character/Maharaka/WaterGun/ITR_02164/ITR_02164.wmodel` | 247,766 | cd83b417cfa5c436 |
| `Character/Maharaka/WaterGun/ITR_02164/textures/pr_wiwg_00_d.dds` | 524,416 | 25f8d072320888d3 |
| `.../textures/pr_wiwg_00_n.dds` | 1,048,704 | d3613d6a734e667c |
| `.../textures/pr_wiwg_00_s.dds` | 262,272 | 35aace88ce6b16c7 |
| `Character/<Class>/AnimSets/<Class>_WaterGunAnimSet.wmodel` × 7 | 4.6~5.4 MB | receipt 참조 |
| `Effect/Maharaka/WaterGun/Meshes/bfm_f_fountain_010.wmodel` | 17,616 | 2db99b5bb5551a41 |
| `Effect/Maharaka/WaterGun/Textures/fx_tex_04/fx_h_wood_decal_bump_01.dds` | 32,896 | 25f59d3486df988b |
| `Effect/Maharaka/WaterGun/Textures/fx_tex_high_00/fx_a_waterfalln_001.dds` | 262,272 | d48d0de5e16c9239 |

- 14개 파일 모두 `C:\Users\USER\OneDrive\바탕 화면\CY_Resources\`에 같은 상대 경로로 복사하고 바이트가 같은지 확인했다. 이펙트 문서가 참조하는 나머지 33개 텍스처는 기존 Resources에 이미 있다(누락 0건).
- 물총: umodel glTF → ModelAssetConverter(`--pretransform --no-auto-textures --scale 100`, 두 재질의 D/N/S remap). 서브메시 2개(0 = `itr_02164_01_mi` 불투명 본체, 1 = `itr_02164_02_mi` 반투명 탱크), 단위 cm, bounds x -16..61 / y -7.8..20.9 / z -14.6..8.8.
- AnimSet: `Tools/ActorXAssetCooker/build_waterpang_watergun_player_animations.py`. 클래스별 PSA의 `pr_itr_02164_*` 10개 클립을 클래스 중립 이름으로 바꿔 넣었고, 각 body의 skeleton이 바이트 단위로 보존되는지 assert한다.
  - 클립: `watergun_idle, watergun_run, watergun_start, watergun_end, watergun_att_1..6`.
  - GuardianKnight(PC_DL_00)의 PSA는 umodel로 새로 export했다.
- `Data/Actors/CharacterCatalog.json`: 7개 클래스의 `animationSetModels`에 각각 한 줄씩 추가했다. 이 목록은 클래스 로드마다 모두 읽힌다. **따라서 팀원 PC에 AnimSet 파일이 없으면 해당 클래스 로드가 실패하므로, Drive 전달이 필수다.**

## 3. 장착 실측

좌표 규칙:

- PSA→WModel 위치 변환은 (px, py, -pz), 즉 bone space는 Z가 반전된 UE 좌표다.
- 쿠킹된 물총 좌표는 UE (x, z, y)이고, 필요한 좌표는 (x, y, -z)다. 그래서 모델 pre-rotation으로 `XMMatrixRotationX(-90°)`를 쓴다(det +1).

스케일:

- prop3 combined bone scale은 6개 클래스가 100(body preScale 0.0001), DimensionMaster가 1(preScale 0.01)이다. 결과 basis는 모두 0.01이다.
- 따라서 cm 모델이 추가 스케일 없이 m로 맞춰진다. maze hammer와 같은 방식이다.

방향 검증(모델 +X = 액터 전방):

- 발사 프레임: 총 전방 오차 1~3°(att_3은 DimensionMaster -9.4°, Warlord -6.2°).
- idle: 7~13° 벗어남.
- pitch: ±10° 이내. 총 위쪽과 월드 위쪽의 차이: 0~15°.

## 4. 애니메이션

- 무장 중에는 `Resolve_LocomotionClip`이 IDLE/RUN을 `watergun_idle`/`watergun_run`으로 바꾼다. 두 클립이 body에 실제로 붙어 있을 때만 교체한다.
- 탈것 탑승 중에는 기존 탑승 클립이 우선한다.
- `watergun_att_1..6`은 다음 단계(스킬)용으로 모델에 탑재돼 있고, 지금은 Debug 미리보기만 재생한다.
- `watergun_start/end`는 탑재만 하고 무장 edge에서는 재생하지 않는다. 이동 상태머신과 충돌하지 않도록 하기 위해서다.

## 5. 발사 이펙트

- 빌더: `Tools/EffectPipeline/build_maharaka_watergun_source_effects.py`. 근거 루트는 Assimp가 한글 경로를 열지 못해 `C:/LostArkExtract/MaharakaWaterGunFX20260928`에 두었다.
- 실행 순서: stage-source → acquire ×4 → `--native`(program 14개, deferred 0) → `--install-native`(4940~4953, 그룹 4928) → `--project --install`.
- 설치 결과: `effect.maharaka.watergun.watergun_att_{1..4}.full.restore`. 요소 수 10/14/12/13, 길이 1325/1819/2794/2890 ms.
- 원본에서는 49개 요소 모두 `b_root` 소켓 `FX_Prj_01`을 따라간다. `bind_player_prop`가 이것을 모두 `bip001-prop3`로 바꾸고 socket을 position [0.6, 0, -0.03] m, rotation [180, 0, 0]으로 설정한다.
  - 위치: UE (60, 0, 3) cm를 Z 반전한 값이다.
  - 회전: 이펙트 드라이버의 Y 반전 basis와 플레이어의 Z 반전 bone basis 사이의 X축 180° 차이를 보정한다.
- `CEffectPresentationService::Requires_SourceBoneImportScaleNormalization`에 `effect.maharaka.watergun.` 접두사를 추가했다. 0.01 basis를 1로 정규화하는 경로이며, 7개 클래스 모두 0.01 검사를 통과한다.
- 요소별 방향: 속도 모듈이 있는 요소는 모두 emitter +X(총구 전방)로 방출하고(최소 x 10~150 cm/s), 시작 오프셋은 ±20 cm 이내다. 소켓 회전 X180은 +X를 유지하므로 방출 방향은 총 전방과 같다. 전체 표는 문서 끝에 있다.

## 6. 복제 설계 (protocol 118)

- Shared: `PLAYER_SNAPSHOT.isWaterpangArmed`(bool)를 추가했다. 와이어에서는 `isKnockbackAirborne` 바로 뒤의 U8이며, 읽을 때 1보다 크면 거부한다. `NETWORK_PROTOCOL_VERSION`은 117에서 118로 올렸다.
- Harness의 `== 117u` 검사 8곳도 118u로 바꿨다.
- Server: `GameRoom_Replication.cpp`의 스냅샷 생성에서 상태 없이 계산한다. 다른 fork가 수정 중인 시뮬레이션·Hazards 파일은 건드리지 않았다.
  - 무장 조건: `MAHARAKA` 월드이고, `m_MaharakaWaterpangIntro`가 있고, 인트로 `iStartTick`에 도달했으며, 다음 중 하나를 만족할 때다.
    - `bWaterpangFall`
    - `KNOCKDOWN`
    - `WaterpangEntry` 네비 영역 위에 서 있음
  - 따라서 경기 중 낙사·밀림 동안에는 총을 유지한다. 영역을 벗어나거나, 방이 비어 경기가 리셋되거나, 월드를 떠나면 해제된다.
- Client:
  - `ClientReplication`이 매 스냅샷마다 `CCharacter::Apply_WaterGunPresentation(isWaterpangArmed)`를 호출한다. 모든 참가자의 캐릭터에 적용되므로 다른 사람의 총도 보인다.
  - `Part_96_WaterGun`은 `CPart_Equipment`로 `bip001-prop3`에 소켓하고, `iHiddenMeshMask = 0x2`로 탱크를 숨긴다.
  - 무장 중에는 `Set_PartVisible` 규칙이 모든 weapon part(Warlord 방패, DimensionMaster 정적 무기 4개 포함)를 숨긴다. 해제하면 `Apply_NetworkStance`가 원래 무기를 되살린다.
  - 무장 edge에서 이펙트 target 4개를 priority queue에 등록한다.
- Server 외 경로의 기본값은 false이므로 다른 월드에는 영향이 없다. Server와 Client는 함께 빌드·재시작해야 한다.

## 7. Debug 미리보기·진단

- F1 → `Map Camera / Player`(Maharaka)에 `Water Gun preview arm` 체크박스와 `Fire 1`~`Fire 6` 버튼을 추가했다. Debug 전용이며 로컬 캐릭터에만 적용된다.
- Fire 1~4는 클립과 이펙트를 함께 재생하고, 5~6은 원본에 파티클이 없어서 클립만 재생한다.
- `EffectFailure.user.log` 채널 `maharaka.watergun`에 다음 사건을 기록한다. 현재 Level이 MAHARAKA일 때만 기록한다.
  - arm/disarm: class, local, clips, socketBasis, socketWorld
  - arm-failed
  - effect-queue-failed
  - preview-fire

## 8. 변경 파일

- Shared: `PacketType.h`, `PacketMessages.h`, `PacketMessages.cpp`
- Server: `GameRoom_Replication.cpp`
- Tools:
  - `NetworkProtocolHarness.cpp`
  - 신규 `Tools/ActorXAssetCooker/build_waterpang_watergun_player_animations.py`
  - 신규 `Tools/EffectPipeline/build_maharaka_watergun_source_effects.py`
- Client:
  - `Character.h/.cpp`, `ClientReplication.cpp`, `Level_Development.h`, `MainApp.cpp`, `Effect_PresentationService.cpp`
  - `Effect_ArtistMaterial_Tables.inl`
  - `Client/Bin/ShaderFiles/Shader_EffectArtistNativeDispatchKoukuNativeCases4928.hlsli`, `Shader_EffectArtistNativeSelectedGroup4928.hlsli`, `Shader_EffectKoukuNativeGroup4928.hlsli`
- Data:
  - `Data/Actors/CharacterCatalog.json`
  - `Data/Effects/EffectCatalog.json`
  - 신규 `Data/Effects/Authored/effect.maharaka.watergun.watergun_att_{1..4}.full.restore.effect.json`
- 문서: `CLAUDE.md`(protocol 118 한 문장)
- vcxproj·filters 변경 없음. 새 C++ 파일 없음.

검증:

- `git diff --check`: 통과
- 추가된 C++ 줄: 모두 ASCII
- 파일별 원래 인코딩·줄바꿈: 보존

## 9. 남은 경계·미해결

- 탱크(반투명 재질)는 숨긴 상태다. native 반투명 재질을 복원하지 않았다.
- 사운드와 실제 발사 판정(스킬·PvP)은 다음 단계다.
- start/end 클립은 무장 edge에서 재생하지 않는다.
- 미리보기에서 발사 클립이 끝나면 다음 이동 입력이 들어올 때 locomotion으로 돌아간다.
- 네비 영역 `WaterpangEntry`에 포함되지 않는 수영 구역에서 FALLING 없이 서 있으면 무장이 해제된다. 이 영역 판정은 다른 fork의 범위 수정 결과에 따라 달라질 수 있다.

## 10. 사용자 확인 절차

1. VS를 닫은 상태에서 Debug Product 빌드를 실행한다. Server와 Client 모두 protocol 118이어야 한다.
2. Server + Client를 실행하고 Lobby에서 `Maharaka`로 들어간다.
3. 워터팡 아레나에 들어가 10초 카운트다운이 끝난 뒤 확인한다.
   - 인트로 컷신 시작 시점에 모든 참가자의 손(prop3)에 물총이 보이는지
   - 직업 무기(방패·차원술사 무기 포함)가 사라졌는지
   - idle/run이 물총 자세로 바뀌는지
4. 아레나에서 나가거나 방을 비운 뒤 다시 들어가면 원래 무기로 돌아오는지 확인한다.
5. F1 → `Map Camera / Player` → `Water Gun preview arm`을 켜고 `Fire 1`~`4`를 누른다. 총구에서 물줄기가 전방으로 나가는지, 크기와 방향이 맞는지 판정한다.
6. 문제가 있으면 `Client/Default/EffectFailure.user.log`의 `maharaka.watergun` 행을 첨부한다.

## 부록 A. 49개 요소 검증표

소스 lookupTable 해석 결과이며, UE emitter local 기준이다. 런타임에서는 prop3 × socket(0.6, 0, -0.03) m, X180 적용 후 +X가 총 전방이 된다.

| 문서 | # | element id | localSpace | startLocation (cm, emitter) | startVelocity min -> max (cm/s) | 방향 판정 |
|---|---|---|---|---|---|---|
| att_1 | 1 | `bd614f63de5bdcb04bda` | True | [-5.0, 0.0, 0.0] | [10.0, -5.0, -5.0] -> [10.0, 5.0, 5.0] | +X 총구 전방 방출 |
| att_1 | 2 | `1e96f2b565e4808d2ca9` | True | [3.0, 0.0, 0.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_1 | 3 | `f088a844f1c612c40761` | False | primitive(구/실린더 표면) | [15.0, 0.0, 0.0] -> [20.0, 0.0, 0.0] | +X 총구 전방 방출 |
| att_1 | 4 | `5f5401446094b96b5027` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_1 | 5 | `6ee8ebed58e41522bc3b` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_1 | 6 | `a019ce22607b549f2538` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_1 | 7 | `0ec73b71510db27f0076` | False | primitive(구/실린더 표면) | [100.0, -60.0, -60.0] -> [150.0, 60.0, 60.0] | +X 총구 전방 방출 |
| att_1 | 8 | `9f3c356bf4603b7df392` | True | [0.0, 0.0, -10.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_1 | 9 | `a12bcbbb22ee4c7d8a55` | True | [-5.0, 0.0, 20.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_1 | 10 | `56866d8304c4e0aac633` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_2 | 1 | `83d5b3b420251de91c9c` | True | [10.0, 0.0, 0.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_2 | 2 | `c7f0f1d8da5aa969efd9` | True | [10.0, 0.0, 0.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_2 | 3 | `87a15ea78a2c09fe7019` | True | [-5.0, 0.0, 0.0] | [10.0, -5.0, -5.0] -> [10.0, 5.0, 5.0] | +X 총구 전방 방출 |
| att_2 | 4 | `1bfac8bc333d1d50e575` | True | [3.0, 0.0, 0.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_2 | 5 | `bf8fbf1d60e5ab2a818b` | False | primitive(구/실린더 표면) | [30.0, 0.0, 0.0] -> [40.0, 0.0, 0.0] | +X 총구 전방 방출 |
| att_2 | 6 | `6277ebc5ab10e47e4f73` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_2 | 7 | `d79ce8a53e68b29eb876` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_2 | 8 | `4a90f94b5e5ab5bebb06` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_2 | 9 | `70175392e6d76f9b2628` | False | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_2 | 10 | `b29f318bd4583dd5d4e3` | False | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_2 | 11 | `5fdec76b18ea7e7ad39b` | False | primitive(구/실린더 표면) | [150.0, -60.0, -60.0] -> [250.0, 60.0, 60.0] | +X 총구 전방 방출 |
| att_2 | 12 | `cabf04ff70148d72eab8` | False | primitive(구/실린더 표면) | [50.0, -100.0, -100.0] -> [250.0, 100.0, 100.0] | +X 총구 전방 방출 |
| att_2 | 13 | `b0b764db864f09950130` | True | [0.0, 0.0, -10.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_2 | 14 | `6815c17d98c45b2ee2a5` | True | [-5.0, 0.0, 20.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_3 | 1 | `be9a93a1d5ba7278c10b` | True | [10.0, 0.0, 0.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_3 | 2 | `32f81ec9183dabe37450` | True | [10.0, 0.0, 0.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_3 | 3 | `6dc37bf7817098ea9c26` | True | [-5.0, 0.0, 0.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_3 | 4 | `a8669b9b1aebb4296476` | False | primitive(구/실린더 표면) | [30.0, 0.0, 20.0] -> [40.0, 0.0, 50.0] | +X 총구 전방 방출 |
| att_3 | 5 | `54b5cc1c264fc69903dd` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_3 | 6 | `81aad1236210a95c832f` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_3 | 7 | `482a51ee31f71613dd47` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_3 | 8 | `de708dbc703876b6801b` | False | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_3 | 9 | `32a9e70beef1aadc7a54` | False | [-20.0, 0.0, 0.0] | [150.0, -150.0, -150.0] -> [250.0, 150.0, 150.0] | +X 총구 전방 방출 |
| att_3 | 10 | `db585deb9c600960dc44` | False | primitive(구/실린더 표면) | [50.0, -100.0, -100.0] -> [250.0, 100.0, 100.0] | +X 총구 전방 방출 |
| att_3 | 11 | `44fa64cb5acab6c4dd8b` | True | [0.0, 0.0, -10.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_3 | 12 | `b2423dea1289ca28f515` | True | [-5.0, 0.0, 20.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_4 | 1 | `ede7d9efd28e39d39461` | True | [10.0, 0.0, 0.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_4 | 2 | `3dc626dac9c120fdd158` | True | [10.0, 0.0, 0.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_4 | 3 | `1ffb7f5501319791154c` | True | [-5.0, 0.0, 0.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_4 | 4 | `4b39ba0f361fe0634829` | False | primitive(구/실린더 표면) | [30.0, 0.0, 20.0] -> [40.0, 0.0, 50.0] | +X 총구 전방 방출 |
| att_4 | 5 | `c646b929cfbe7693c8fa` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_4 | 6 | `ce20946252f9e83f4e1e` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_4 | 7 | `949fa79c457f832714a3` | True | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_4 | 8 | `06575e0929464fc9ff1f` | False | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_4 | 9 | `cb5d0886b3c6333ef120` | False | (0,0,0) 기본 | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_4 | 10 | `9753b4affa82fae70a1f` | False | [-20.0, 0.0, 0.0] | [150.0, -150.0, -150.0] -> [250.0, 150.0, 150.0] | +X 총구 전방 방출 |
| att_4 | 11 | `d65fa8a673bed53dcbb4` | False | primitive(구/실린더 표면) | [50.0, -100.0, -100.0] -> [250.0, 100.0, 100.0] | +X 총구 전방 방출 |
| att_4 | 12 | `294ab79a84c6e8f9b7bd` | True | [0.0, 0.0, -10.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |
| att_4 | 13 | `3df6edf6f0b073e0e93f` | True | [-5.0, 0.0, 20.0] | 없음(총구 고정 표현) | 총구 소켓 고정 |

## 부록 B. 발사 프레임 총구 배치 (7 클래스 × 4 클립)

| 클래스 | 클립 | prop3 bone scale | 총구 모델공간(m) | 총구 yaw vs 액터 전방 | pitch |
|---|---|---|---|---|---|
| Artist | watergun_att_1 | 100.0 | [0.537, 0.616, -0.065] | 3.3° | -2.0° |
| Artist | watergun_att_2 | 100.0 | [0.491, 0.652, -0.116] | 1.8° | 9.1° |
| Artist | watergun_att_3 | 100.0 | [0.458, 0.636, -0.103] | -0.4° | 3.1° |
| Artist | watergun_att_4 | 100.0 | [0.551, 0.604, -0.097] | 2.0° | 4.7° |
| DimensionMaster | watergun_att_1 | 1.0 | [0.498, 0.682, -0.183] | -1.6° | -1.0° |
| DimensionMaster | watergun_att_2 | 1.0 | [0.488, 0.685, -0.204] | -1.2° | 6.8° |
| DimensionMaster | watergun_att_3 | 1.0 | [0.396, 0.688, -0.222] | -9.4° | 3.3° |
| DimensionMaster | watergun_att_4 | 1.0 | [0.511, 0.593, -0.171] | -0.1° | 0.3° |
| GuardianKnight | watergun_att_1 | 100.0 | [0.535, 0.785, -0.133] | 1.4° | -2.1° |
| GuardianKnight | watergun_att_2 | 100.0 | [0.478, 0.793, -0.18] | 1.2° | 8.0° |
| GuardianKnight | watergun_att_3 | 100.0 | [0.418, 0.78, -0.161] | -2.0° | 2.3° |
| GuardianKnight | watergun_att_4 | 100.0 | [0.555, 0.735, -0.155] | 0.3° | 3.0° |
| GunSlinger | watergun_att_1 | 100.0 | [0.535, 0.785, -0.133] | 1.4° | -2.1° |
| GunSlinger | watergun_att_2 | 100.0 | [0.478, 0.793, -0.18] | 1.2° | 8.0° |
| GunSlinger | watergun_att_3 | 100.0 | [0.418, 0.78, -0.161] | -2.0° | 2.3° |
| GunSlinger | watergun_att_4 | 100.0 | [0.555, 0.735, -0.155] | 0.3° | 3.0° |
| LanceMaster | watergun_att_1 | 100.0 | [0.535, 0.801, -0.149] | 1.4° | -2.1° |
| LanceMaster | watergun_att_2 | 100.0 | [0.476, 0.839, -0.126] | 2.6° | 10.4° |
| LanceMaster | watergun_att_3 | 100.0 | [0.419, 0.794, -0.178] | -2.0° | 2.3° |
| LanceMaster | watergun_att_4 | 100.0 | [0.567, 0.749, -0.168] | 0.3° | 3.0° |
| Slayer | watergun_att_1 | 100.0 | [0.514, 0.798, -0.116] | 1.4° | -2.1° |
| Slayer | watergun_att_2 | 100.0 | [0.462, 0.794, -0.177] | 1.2° | 8.0° |
| Slayer | watergun_att_3 | 100.0 | [0.406, 0.791, -0.153] | -2.0° | 2.3° |
| Slayer | watergun_att_4 | 100.0 | [0.538, 0.741, -0.158] | 0.3° | 3.0° |
| Warlord | watergun_att_1 | 100.0 | [0.446, 0.798, -0.206] | 1.0° | -2.1° |
| Warlord | watergun_att_2 | 100.0 | [0.476, 0.779, -0.221] | 1.8° | 4.9° |
| Warlord | watergun_att_3 | 100.0 | [0.33, 0.789, -0.23] | -6.2° | 1.7° |
| Warlord | watergun_att_4 | 100.0 | [0.493, 0.698, -0.182] | 2.0° | 0.0° |

## 11. 물총 애니메이션 세트 마하라카 전용·선택 로드 (2026-09-29)

원인: 7개 `<Class>_WaterGunAnimSet.wmodel`이 `Data/Actors/CharacterCatalog.json`의 `animationSetModels`에 들어가 있었다.
`CPlayableCharacterAssetService::Prepare_Models`는 이 배열을 레벨과 무관하게 전부 필수 작업으로 읽고
(`PlayableCharacterAssetService.cpp` 수정 전 314~316행), 하나라도 없으면 `E_FAIL`로 캐릭터 로드 전체가 실패했다.
Drive 전달 파일이 없는 PC는 베른·발탄·쿠크·캐릭터 선택에서도 입장할 수 없었다.

수정:
- 카탈로그: 7개 경로를 선택 필드 `waterGunAnimationSetModels`로 옮겼다. `ActorCatalog`가 이 필드를 선택적으로 읽는다(없으면 빈 목록, 형식 오류는 기존처럼 거부).
- `Prepare_Models`: 요청 레벨이 `LEVEL::MAHARAKA`일 때만 이 목록을 작업에 넣는다. 파일이 없으면 작업에 넣지 않고, 로드·부착 실패는 해당 세트만 건너뛴다.
  건너뛴 세트는 `EffectFailure.user.log`의 `maharaka.watergun` `kind=animset-skipped class=... set=... reason=missing|load-failed|attach-failed`로 남고 캐릭터 로드는 계속된다.
  `Attach_AnimationSet`은 뼈대·클립 이름 검사를 끝낸 뒤에만 본체를 바꾸므로 실패해도 본체는 그대로다.
- `CCharacter::Apply_WaterGunPresentation`: 무장 요청이 와도 프로토타입 레벨이 마하라카가 아니거나 `watergun_idle/run` 클립이 없으면
  무장하지 않고 직업 무기를 유지한다(`kind=arm-skipped reason=animation-set-unavailable`). 물총 모델은 원래대로 무장 시점에만 지연 로드된다.

레벨별 경로:
- Bern/Valtan/Kouku/Character Select/Test(Development): `Ready_Character_Rendering`과 `CClientReplication`의 지연 로드가 각 레벨 인덱스로 `Prepare_Models`를 호출하므로
  `loadWaterGunSets=false`이고 물총 경로를 해석하거나 읽지 않는다. 서버가 마하라카 밖에서 무장 플래그를 켜지 않고, Debug 미리보기 체크박스도 마하라카에서만 보인다.
- Maharaka: `Loader.cpp`의 마하라카 로드와 다른 직업의 지연 로드 모두 `LEVEL::MAHARAKA`로 호출되어 세트를 붙인다.
- Maharaka + 파일 삭제: 존재 검사에서 빠지고 진단 한 줄만 남는다. 직업은 정상 생성되고 무장 요청은 무시된다.

참고: `CharacterActionWorkbench`의 저작 소스 목록은 `animationSetModels`만 보므로 물총 클립은 그 목록에 나오지 않는다.
검증: 인코딩·CRLF 유지, `git diff --check` 통과. 빌드·실행은 하지 않았다.

확인: Client 재빌드 후 (1) 물총 AnimSet 파일을 잠시 치운 PC에서 베른·캐릭터 선택 입장이 되는지, (2) 같은 상태의 마하라카에서 캐릭터가 뜨고
`animset-skipped` 로그가 남는지, (3) 파일을 되돌린 마하라카에서 워터팡 무장이 이전처럼 되는지.
