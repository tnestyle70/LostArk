# 2026-09-28 마하라카 워터팡 — 낙사 부활, 회전 워터캐논, 큰 모코모코 물벼락 RESULT

브랜치 `codex/maharaka-map-restoration`. 커밋·빌드 전 상태다. Visual Studio(devenv)가 열려 있어
빌드를 돌리지 않았다. Client/UI는 실행하지 않았고 화면 판정은 하지 않았다.

## 1. 원본 근거 (읽기 전용 조사)

조사 산출물: `C:\Users\USER\.claude\jobs\46aea322\tmp\watercannon_research\`.
사용자 제공 영상 `모코모코 워터캐논.mp4`(12.6초) 0.5초 간격 프레임을 함께 대조했다.

| 항목 | 원본 값 | 근거 |
|---|---|---|
| 안내 문구 | se_announce_52 "모코모코 워터캐논이 회전합니다." / 53 "…빠르게 회전합니다." / 51 "모코모코 어트렉션에서 물벼락이 쏟아집니다." | `EFTable_GameMsg.db` (UTF-8) |
| 문구 → 행동 | 52/53 SceneEvent 바로 다음 SendAISignal(actor 3, AS_020/030, AS_040/050) | 57011 `TriggerMapData.loa` |
| 워터캐논 | actor 3 = NPC 570911 `MN_ISMP_00-1`, (75.044, -984.307), yaw 223.59 | 57011 `DeployData.loa` |
| 큰 모코모코 | actor 22 = NPC 570941 `MN_ISMP_00`, (81.836, -991.514), 중앙을 향함 | 같은 파일 |
| 물줄기 판정 | SkillEffect 422560330: AreaType 2, 1500cm, OffsetX -750 → 중앙 관통 ±7.5m 직선(반대 방향 2줄기), 폭 70cm | `EFTable_SkillEffect.db` |
| 반복 판정 | 투사체 `CEFSequenceSummonsProjectileFixArea` ActionTimer 0.4초 | `Projectile\<id>.loa` |
| 밀림 | Push 500~520cm / 535~555ms, HitType 4, FallDown | 422560330 |
| 회전 속도 | 한 바퀴 19.2초(회전) / 9.5초(빠르게), 시계/반시계 플래그 | 투사체 꼬리 float (추론: 초/회전) |
| 예고 | 사각 데칼 1103 1.5초 (4225613~16 계열) | Action LOA |
| 방향 선택 | 단계마다 50/50 시계·반시계 | 트리거 6000~6009 |
| 물벼락 4225601 | 도넛 데칼 2.3초, t=2.3 판정. 3~4m 밀림 200~220/242~262ms, 4~7m 띄우기 200~220cm/1814~1834ms/높이 200, 전부 FallDown, 피해 행 없음 | SkillEffect 422560101~108 |

확정 못한 것: 신호→스킬 AI 대응(서버 전용 AI 표), 초기 물줄기 방향(+0°/+90° 중 +90° 채택),
조건 기반 타이머의 절대 시각, 경기 종료 조건, 판정 행 피해 계수(870~1130)의 기준값.

## 2. 구현

### 2.1 낙사 → jump1/jump2 즉시 부활 (앞 요청)
- 경기 시작 이후 워터팡 영역(`WaterpangEntry`) 위에서 받은 넉백은 아레나 가장자리를 넘을 수 있다.
- 지지면이 사라지면 기존 FALLING(1.5초, 피격 루프 표현) 후 사망 대신 낙하 XZ에 가까운
  jump1/jump2 위치(네비 투영)에서 NONE으로 부활한다. 사망 UI 없음.
- jump 트리거는 원래 `triggerOnce=false`라 무제한 재사용된다.

### 2.2 공용 일정·수치 계약
`Shared/Public/Gameplay/MaharakaWaterpangContract.h`에 원본 수치, 경기 일정표,
`Sample_MaharakaWaterpangEvent`를 둔다. 서버와 클라이언트가 같은 경기 시작 틱으로 같은 결과를
계산하므로 프로토콜 변경이 없다. 시계/반시계는 시작 틱 해시로 결정한다.

일정(경기 시작 기준 초, PROJECT 일정 — 순서·속도·물벼락 120/150초는 원본 체인 라벨):
캐논 20/65/127/157(15초), 202/247(20초), 빠르게 300(15초), 347/377(10초), 물벼락 120/150/340,
417초부터 40초마다 빠르게 10초 반복(트리거 1700 주기).

### 2.3 서버 판정
`CGameRoom::Update_MaharakaWaterpangHazards`(GameRoom_PartyWorld.cpp)가 매 틱 호출된다.
- 캐논: 발사 구간에 직선 박스(±7.5m, 반폭 0.35m + 몸 반경) 안이면 물줄기 방향 바깥으로
  5.1m / 545ms 강제 밀림 + 넘어짐. 플레이어별 0.4초 재판정.
- 물벼락: t=69틱(2.3초)에 모코모코 기준 90° 부채꼴 3~7m. 3~4m는 2.1m/252ms 밀림,
  4~7m는 2.1m/1824ms 높이 2m 띄우기. 모두 넘어짐.
- 피해 0: 밀림·넘어짐만. 피해 0은 숫자 이벤트를 만들지 않는다.

### 2.4 클라이언트
`CMaharakaWaterpangPresentation`이 같은 샘플러로 진행 중 이벤트의 원본 안내 문구를 화면 상단에 표시한다.

## 3. 변경 파일

- Shared/Public/Gameplay/MaharakaWaterpangContract.h (ASCII, LF)
- Server/Public/ServerPlayer.h, Server/Public/GameRoom.h
- Server/Private/GameRoom_PlayerSimulation.cpp, GameRoom_PartyWorld.cpp
- Client/Public/MaharakaWaterpangPresentation.h, Client/Private/MaharakaWaterpangPresentation.cpp

파일별 기존 인코딩·줄끝을 유지했다. 새 C++ 파일과 vcxproj/filters 변경은 없다. `git diff --check` 통과.

## 4. 남은 단계

1. 빌드(VS 종료 후 Product Debug 1회).
2. 연출: 원본 파티클 `FX_MN_ISMP_00`(Attk03 WaterBomb/Orb, WaterGround, Face, WaterFinish),
   사각/도넛 데칼, 캐논 `att_battle_3_01/02/03/04`·모코모코 `att_battle_1_01` 클립, Water2 사운드는
   아직 설치·연결되지 않았다. 현재 물줄기는 화면에 보이지 않고 판정·밀림·알림만 동작한다.
3. 사용자 화면 확인: 경기 시작 20초 뒤 첫 알림, 밀림·낙하·jump 부활.

## 5. 사용자 확인 후 버그 수정 (2026-09-28 16:5x)

사용자 확인: 물대포 → 낙하까지 동작. 두 가지 결함 보고.

### 5.1 부활 후 낙하(피격) 자세가 남고 그 자세로 이동
- 원인 1: `Client/Private/Character.cpp` Apply_NetworkAction은 새 action이 NONE이면 이전 action 목록
  (INTERACTION/SKILL/TRIGGER_MOVE/…/GRABBED/VEHICLE_SKILL)에서만 IDLE/RUN으로 복귀하는데 FALLING이 없었다.
  워터팡 jump 부활(FALLING→NONE)은 HIT 루프를 유지했다.
- 원인 2: 물대포 피격 KNOCKDOWN이 `m_eKnockdownStep`(FALLING/LANDING/DOWN)을 남긴 채 FALLING으로 넘어가고,
  FALLING 분기는 DEAD 분기와 달리 이를 초기화하지 않았다. `Commit_Locomotion`은 단계가 NONE이 아니면
  RUN/IDLE 전환을 막아 부활 후 이동 중에도 자세가 바뀌지 않았다.
- 1차 수정(철회): 공용 FALLING 분기와 공용 복귀 목록을 직접 고쳤다. 사용자 지적대로 이는 잘 동작하는 기존
  경로(쿠크 1·3관문은 낙사를 막아 둔 곳, 2관문·빙고는 FALLING→DEAD)를 바꾸는 방식이라 되돌렸다.
- 최종 수정: 공용 분기 두 곳은 원본과 동일하게 복구하고, DEAD 분기 바로 뒤에 마하라카 전용 예외
  `else if (이전 action == FALLING && 현재 Level == LEVEL::MAHARAKA)`를 추가했다. 이 분기만 넘어짐 단계를
  NONE으로 비우고 IDLE/RUN으로 복귀한다. 다른 Level의 FALLING 처리는 변경 전과 같다. UTF-8/CRLF 유지.

### 5.2 jump1/jump2가 반대편 먼 도착점으로 이동
- 원인: `Tools/MapPipeline/configure_maharaka_waterpang_entry.py:45-47`이 `jump{n}` 목적지를 이름이 짝인
  `jump{n}_1` 마커 위치로 복사한다. 사용자 저작 마커 jump1_1/jump2_1이 서로 반대편 기둥 앞에 놓여 있어
  jump1(66.61,-992.67)→(78.84,-980.06)이 중앙을 가로지른 17.5m, jump2도 같은 형태였다.
- 수정: `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json` revision 274→275. jump1/jump2 movePlayer 목적지와
  jump1_1/jump2_1 마커 위치를 맞바꿈(스크립트 짝 규칙 유지). 결과 jump1→(71.03,23.10,-988.02) 6.42m,
  jump2→(78.84,23.23,-980.06) 6.26m, 둘 다 중앙에서 약 5.5m의 가까운 쪽 무대. 다른 행 변경 없음.
- 게시: `Publish-WorldGameplay.ps1 -Mode Validate/Publish -WorldId MAHARAKA` 성공(41 placements).
  `Server/Bin/DataFiles/World/MAHARAKA.worldbootstrap`은 헤더 revision과 jump1/jump1_1/jump2/jump2_1 4행만 변경,
  `Client/Bin/DataFiles/World/MAHARAKA.npcpresentation.json`은 바이트 동일.

### 검증
- 두 파일 백업(`C:\Users\USER\.claude\jobs\46aea322\tmp\revive_fix_backup`) 대비 diff가 의도한 줄만 표시.
- `git diff --check` 통과. 빌드는 하지 않았다(Character.cpp 변경 → Client 재빌드 필요, 데이터는 Server 재시작 필요).

## 6. 공격 연출 복원·연결 (FX_MN_ISMP_00 파티클, 예고 데칼, 공격 모션)

드라이버 `Tools/EffectPipeline/build_maharaka_waterpang_source_effects.py`(신규, Inanna/vehicle 코호트 재사용).
증거·후보·로그는 `out/MaharakaWaterpangFX20260928/`. 빌드·Client 실행은 하지 않았다. 화면 판정은 사용자 몫이다.

### 6.1 원본 사슬과 정정
- 4225601(큰 모코모코) 한 스테이지 6초, 4225615(캐논 시계) 시작1.5초/루프24초/끝1초를 원본 Action LOA에서 추출.
- `FX_Turn01~04`는 소켓이 아니라 **notify 이름**이다. 0.5초 간격 `WaterBomb_800_01` 버스트는 루트 스냅샷으로
  정상 해석된다(앞 조사 보고의 "소켓 없음"은 오독).
- 물줄기 본체는 투사체 422560315의 `Par_G_ISMP_Attk03_WaterBomb_Loop_800_01`(로컬 변환 항등, 배율1).
  요소들이 원본 Y축(투사체 전방의 오른쪽) ±7.7m에 놓여 판정 박스의 AreaOffsetAngle 90°와 일치한다.
- PlayParticleEffect 페이로드 byte47을 공용 디코더가 "enabled"로 읽어 물줄기(007)·지면(014)이 빠지던 것을,
  이 코호트에서만 비활성으로 해석하지 않도록 했다(MN_ISMP_00 1,500건 중 약1,400건이 0, 원본 영상에 물줄기 보임).
  의미 자체는 미확정으로 남긴다(`sourceByte47Semantic=UNRESOLVED_NOT_DISABLE`).
- 예고: SkillDecal 1103 → GroundEffect `GR_Mon_Rectangle_cond_01`, 재질 `fx_o_de_condsquare_02_01_tr`(쿠크 3607과 동일 MIC),
  활성색 (0.01,0.1,1.0,3.0) 파랑. SkillDecal 1006 → `GR_Mon_Donut_behit_01`, 재질 `fx_o_de_behitmondonut_02_01_tr`
  (부모가 쿠크 도넛과 달라 새 프로그램), 활성색 (1,0,0.03,1.5).
  PlayDecalEffect 꼬리 float를 쿠크 named timing 순서로 읽어 채움 1.3초(캐논)/2.0초(모코모코), 페이드 0.1/0.2초 (순서는 추론).

### 6.2 설치한 것
- native 프로그램 3621~3679 중 33개(스프라이트26·메쉬6·데칼1): 기존 3584/3648 그룹 파일에만 추가, 새 셰이더 파일 없음.
  변경: `Client/Bin/ShaderFiles/Shader_EffectKoukuNativeGroup3584/3648.hlsli`, dispatch 3개, `Shader_EffectArtistNativePrograms.hlsli`,
  `Client/Private/Effect_ArtistMaterial_Tables.inl`. 보류 1개(3633 `bfx_d_pa_smoke_01_03_tr`: TEXCOORD7 스프라이트 어댑터 없음, 이미터 5개 제외).
- 도넛 GroundEffect PS `cb2536fe…`를 `generate_artist_native_runtime_shader.py`의 검토된 GroundEffect 목록에 추가
  (같은 LocalDecal VS `51afa7d0…`, CB0[0..3] assert로 검증).
- Effect 문서 10개(Data/Effects/Authored, v13) + EffectCatalog 10행:
  `effect.maharaka.waterpang.mokomoko.att_battle_1_01.full.restore`(33요소),
  `effect.maharaka.waterpang.cannon.att_battle_3_01/3_04.full.restore`(9/17),
  `...cannon.att_battle_3_02.part0~3.full.restore`(349/338/338/366, 파티클 예산 8192 때문에 FX_Turn 6초 구간별 분할),
  `...cannon.jet.full.restore`(26), `...cannon.telegraph`(사각 15×0.9m), `...mokomoko.telegraph`(도넛 3~7m, 90°).
- WorldSequence(LV_OCN_EVENTIS_MHP) revision 6→7: 템플릿·인스턴스 5개
  `world.sequence.instance.maharaka.waterpang.attack.{mokomoko, cannon.start, cannon.loop.cw, cannon.loop.ccw, cannon.end}`.
  도입 HOLD 자세 그대로, 클립 att_battle_1_01 / 3_01 / 3_02(시계) / 3_03(반시계) / 3_04 + 해당 V1 이펙트 트랙(소켓 부착은
  `Sample_ObjectEffectAttachments`가 클립 포즈로 해석). `Publish-MapAuthoring -Scope WorldSequences` Validate/Publish/Check 통과.
- Resources 신규 4개: `Effect/Maharaka/Waterpang/Meshes/fm_h_tornado_01_2.wmodel`,
  `Effect/Maharaka/Waterpang/Textures/fx_tex_02/fx_d_electric_011_1.dds`, `.../fx_tex_04/fx_h_waterline_01_1.dds`,
  `.../fx_tex_05/fx_m_bubble_003.dds`. 문서가 참조하는 closure 79개 전체(`out/MaharakaWaterpangFX20260928/resource_ids.txt`)를
  `OneDrive/바탕 화면/CY_Resources/`에 같은 상대 경로로 복사했다.

### 6.3 연결 (서버 일정과 동기)
- `Client/Private/MaharakaWaterpangPresentation.cpp` `Update_Attacks`: 도입이 끝난 뒤 `Sample_MaharakaWaterpangEvent`(서버와 같은
  경기 시계)로 배우별 인스턴스·오프셋을 정한다. 물벼락 이벤트는 모코모코 attack, 캐논 이벤트는 start(0~1.5초)→loop.cw/ccw
  (`bClockwise`)→end, 그 외는 도입 HOLD. `Show_Actor`가 이전 인스턴스를 멈추고 새 인스턴스를 해당 ms로 Seek한다.
- 사각 예고: 캐논 이벤트 0~45틱, 루트 yaw = `fCannonYawDegrees`(데칼 길이축 +Z). 물줄기: 발사 구간 동안
  `Spawn_LevelPlacement`(소유자 유지 루프) 후 매 프레임 `Update_WorldRoot`, 루트 yaw = `fCannonYawDegrees - 90`
  (스냅샷 기저 −90°가 원본 Y축을 루트 +X로 보냄 → 서버 방향과 일치). 도넛: 물벼락 0~69틱, 모코모코→캐논 방향.
- `Client/Private/Level_Loading.cpp`: 마하라카 진입 로딩 때 WorldSequence V1 목록을 모아 기존 로더 준비 작업에 넣는다
  (발탄과 같은 경로, 경기 중 20MB 문서 준비 방지).

### 6.4 미해결·주의
- 도넛 부채꼴의 중심축(로컬 +Z 가정)과 사각·물줄기 방향은 수치 추론이다. 화면에서 물줄기와 판정이 어긋나면 알려 달라.
- 사운드(Water1/Water2 AkEvent)는 연결하지 않았다. PawnMaterialParam(PetEmotion) 표정 파라미터도 미연결.
- 연기 스프라이트 1종(3633) 보류.
- `Client.vcxproj/.filters`: 셰이더 설치기가 파일 전체를 CRLF로 재저장해 바이트 해시가 바뀌었다. git diff 기준 내용 변화는
  없다(보이는 차이는 다른 세션이 이미 넣은 줄). 원래 줄끝 복구는 자동 권한 분류기가 거부해 수행하지 않았다.
- 저장소 전체 `Validate-EffectSources.ps1`은 기존 문서 `effect.kouku.gate1.blade-dance.circle.impact`에서 먼저 실패한다(선행 문제).
  이번 10개 문서는 같은 검증 함수로 개별 검사 통과, 참조 리소스 79개 존재 확인.

### 6.5 사용자 확인 경로
VS 종료 후 Product Debug 빌드(셰이더·Client·Server) → Server 재시작 → 마하라카 입장 → 워터팡 입장 트리거 →
카운트다운 10초와 도입 뒤 20초에 첫 회전(파란 사각 예고 1.5초 → 두 줄기 회전, 캐논 회전 모션),
120초·150초에 큰 모코모코 물벼락(도넛 예고 2.3초 → 얼굴·물줄기·지면 튀김). 물줄기 위치가 판정과 맞는지,
모코모코/캐논 모션 전환이 튀지 않는지 확인한다.

## 7. jump3 오르기 뒤 클라이언트만 물속에 남은 현상 (조사·진단 추가)

사용자 보고(18:14): jump3(물 → 무대, 일반 TRIGGER_MOVE)에서 G를 누르자 카운트다운이 떴지만 캐릭터는 물속에 남았다.

### 7.1 코드·데이터로 확인한 사실
- jump3 행과 서버 bootstrap 행은 이번 작업에서 바뀌지 않았다. 바뀐 것은 jump1/jump2 도착점뿐이다.
- 워터팡 영역 navgrid(88×88, 0.25m, 원점 64/-995)는 영역 전체가 walkable이며 풀 20.48m, 무대 22.4m다.
  jump3 출발 셀 20.48, 도착 셀 22.365. 시작 트리거(Y 21.95~23.35)는 무대 높이에서만 판정되므로
  카운트다운은 서버가 플레이어를 무대에 올렸다는 뜻이다.
- 클라이언트 적용 경로: ClientReplication::Apply_PlayerSnapshot → Character::Apply_LocalMoveSnapshot
  (canPredictMove=false인 TRIGGER_MOVE는 RESET 후 스냅샷 보간) → 착지 후 RESET으로 서버 위치 적용 →
  Update_LocalMovePrediction의 지면 샘플은 Y만 바꾼다. 이 경로에서 오늘 바뀐 코드는 없다(마하라카 FALLING 분기는
  이전 action이 FALLING일 때만, 네비 영역 선택은 Y만 영향).
- 워터팡 이펙트 준비는 로딩 단계(18:12:38~18:13:27, 20MB 루프 문서 4개 각 약 23초)에 끝났고 레벨 시작은 18:13:29 이후,
  G는 약 18:13:54다. 준비가 경기 프레임에서 돌지는 않았다. 다만 로딩이 약 80초로 길어졌다.
- 클라이언트 세션 로그상 18:13:29~18:14:29 사이 메인 루프 멈춤이 1회 더 있었으나 길이가 기록되지 않았다.

결론: 코드와 로그만으로는 서버 위치(무대)와 화면 위치(물)가 갈라진 지점을 증명하지 못했다.

### 7.2 다음 실행에서 원인을 가르는 진단 로그 추가
- `Client/Private/ClientReplication.cpp` `Trace_MaharakaLocalPlayer`: 마하라카 로컬 플레이어의 서버 action 경계마다,
  그리고 서버가 NONE인데 화면 위치가 서버와 1.5m 넘게 떨어져 있으면(초당 1회, 경계당 최대 16회)
  `EffectFailure.user.log`에 `channel=maharaka.localplayer`로 서버 좌표·화면 좌표·canPredict·moveSeq를 남긴다.
- `Client/Private/Character.cpp` `Apply_LocalMoveSnapshot`: 마하라카에서 스냅샷이 IGNORED 처리되고 화면과 1.5m 넘게
  떨어져 있으면 `channel=maharaka.localmove.ignored`를 남긴다.
- 표현 상태는 바꾸지 않는다. 판독법: action=2(TRIGGER_MOVE) 경계 뒤 action=0 경계의 server 좌표가 무대(Y≈22.4)인데
  visual이 풀이면 클라이언트 적용 문제, server 좌표가 풀이면 서버가 착지 후 다시 내렸다는 뜻이다.
  `ignored` 줄이 이어지면 로컬 예측 스냅샷 거부가 원인이다. 서버 콘솔의 `[Trigger] Fire Trigger=jump3 ... Source=KEY`도 함께 본다.
- 인코딩·줄끝 유지(UTF-8, 해당 구간 CRLF), `git diff --check` 통과. 빌드·실행은 하지 않았다.

## 8. 워터캐논 2 영상: 물줄기 고정, 모코코 머리 이중 상 (2026-09-28)

사용자 영상 `워터캐논 2.mp4`(12.8초, 1920×1032)를 0.1~0.5초 간격으로 확대해 봤다. 4~9초 동안 큰 물줄기는
한 방향으로 고정돼 있고, 캐논 머리 옆에 솜브레로·분홍 분사구까지 있는 두 번째 상이 같은 높이에서 약 0.05 화면폭
떨어져 보인다. 이벤트가 끝난 12.3초 이후에는 머리가 하나뿐이다.

### 8.1 물줄기가 돌지 않은 원인
- 두 문서의 모든 요소가 스냅샷 루트 부착(`follow=false`, 기저 −90°)이다. `Effect_Playback.cpp` 4725~4731은
  요소가 처음 시작할 때 한 번만 `ActionRootWorld`를 잡고, 8282~8291이 그 뒤 계속 그 값을 부모로 쓴다.
- 물줄기(`cannon.jet`) 26요소는 0초에 시작해 24초 산다. `Update_WorldRoot`로 매 프레임 루트를 돌려도
  첫 방향에 고정됐다.
- 루프 버스트(`att_battle_3_02.part0~3`, 큰 물줄기)는 WorldSequence 루프 인스턴스의 V1 트랙이다. 루트가 캐논
  오브젝트의 고정 회전(쿼터니언 yaw 약 224°)이라 0.5초마다 새로 나와도 같은 방향이었다.

### 8.2 머리가 두 번 보인 원인
- 모델·배치 원인이 아님을 먼저 확인했다.
  - MN_ISMP_00-1은 서브메쉬 3개가 전부 `b_ismp_root`에 붙은 머리 하나다. 클립 중 모자 중심 이동은 수 cm다.
  - 서버 NPC actor188, 도입 HOLD, 공격 복제본, 중앙 기둥 `MAP_MHP_RECON_CENTRAL_PILLAR`는 모두 (75.05, −984.32)다.
  - 효과 문서에 ModelCue·잔상·모코코 텍스처가 없다.
- 이 cohort의 native 프로그램 20개는 별도 distortion pass로 RG 오프셋을 누적한다(ONE/ONE 가산).
  - 목록: 3622·3624~3626·3629·3630·3635·3636·3638·3640·3643~3649·3651~3653.
  - 머리 둘레 구 3636은 프레넬³×20이고, 물줄기 요소 수백 개가 머리 주변에 겹친다.
- 엔진 resolve는 RG를 ±0.05 UV로 자를 뿐이다(`Shader_Deferred.hlsl` 1304). 포화된 오프셋이 약 96px 옆의
  머리 화소를 끌어와 두 번째 머리를 만들었다. 영상의 두 상 간격과 이 한계가 일치한다.
- gotchas "노이즈 왜곡과 Decal 수신 표면을 구분한다"와 같은 부류다.

### 8.3 수정
- 루프 버스트 (`Client/Private/MaharakaWaterpangPresentation.cpp`)
  - `Initialize`가 `objectWorldPostTransform`을 연결한다. 기존 콜백이 있으면 먼저 적용한다.
  - `attack.cannon.loop.cw/ccw`에만 캐논 중심 기준 `T(−c)·RotY(δ)·T(c)`를 준다.
  - δ = 부호 × 360° × 경과ms / (한 바퀴 초 × 1000)이고, 서버 `fCannonYawDegrees`와 같은 속도·방향이다.
  - 회전 상태는 `shared_ptr` 값으로 캡처한다(effect provider 수명 규약).
  - WorldSequence가 이 변환을 모델과 이미 태어난 V1 입자에 매 프레임 적용하므로, 버스트 선 전체가 판정선과 함께 돈다.
  - 캐논 모델도 함께 돌아서, 머리 본의 원래 회전에 캐논 회전이 더해져 보인다.
- 물줄기
  - 문서: `effect.maharaka.waterpang.cannon.jet.full.restore` 26요소의 부착을 끄고 기저를 0으로 했다. 살아 있는 루트를 따른다.
  - 코드: 사라진 기저 −90°를 `JET_ROOT_YAW_OFFSET_DEGREES`로 옮겨 −90 → −180으로 바꿨다.
  - 결과: 월드 방향은 이전 첫 프레임과 같고, 이제 매 프레임 서버 각도를 따른다.
- 이중 상
  - 위 20개 프로그램의 dispatch 출력을 수신 격리 BA 채널로 옮겼다(`float4(0,0,xy−zw)`).
  - 대상 파일: `Client/Bin/ShaderFiles/Shader_EffectArtistNativeDispatchKoukuNativeCases3584Part2.hlsli`,
    `...Cases3648.hlsli`.
  - 원본 색·왜곡 식은 그대로다. 같은 깊이 평면의 배경만 굴절하고, 캐릭터나 깊이가 다른 머리는 끌려오지 않는다.
  - 쿠크 2461·2587·3682와 같은 계약이며, 범위 밖 프로그램은 바꾸지 않았다.
- 재생성 경로: `Tools/EffectPipeline/build_maharaka_waterpang_source_effects.py`
  - `install_native` 뒤 `route_receiver_isolated_distortion(first, last)`를 호출한다.
  - `project`에서 jet 문서에 `follow_live_root`를 적용한다.
  - 두 함수 모두 멱등이다. 설치본을 다시 적용해도 해시가 같고, 백업 문서에서 jet 결과를 바이트 동일하게 재현한다.

### 8.4 검증과 경계
- 인코딩·줄끝을 유지했다: 셰이더 ASCII/CRLF, 소스 ASCII/LF, 문서 UTF-8/LF, jet JSON 왕복 동일.
- 셰이더 두 파일은 `git diff --check`를 통과했다. 나머지 새 파일은 끝 공백이 없다.
- 부착을 끈 요소는 C++ 검증 규칙(`Effect_DocumentCodec_Validation.cpp` 423, 비활성이면 기저 0)을 충족한다.
- 빌드·실행은 하지 않았다. HLSL 변경이 있어 Product 빌드에서 해당 FX가 다시 컴파일된다. 화면 판정은 사용자 몫이다.
- 확인할 것
  - 발사 중 큰 물줄기와 물줄기가 판정선과 함께 도는지(파란 사각 예고는 발사 전이라 돌지 않는다)
  - 캐논 머리가 하나로만 보이는지
  - 물줄기 주변 바닥 굴절이 남아 있는지

## 9. 굵은 물줄기와 얇은 물줄기가 90° 어긋남 (스크린샷 194432)

사용자 관찰: 회전 중 굵은 물줄기 사이로 연한 물줄기가 직각으로 하나 더 보였고, 실제로 밀린 쪽은 연한 줄이었다.

### 9.1 식별
- 두 연출은 같은 원본 이미터 세트이며 요소가 모두 효과 로컬 Z축(±7.7m)에 놓인다.
  - 버스트(`att_battle_3_02.part0~3`): 0.5초마다 세트 하나. part0에만 요소 349개, 무지개 텍스처 24회 참조.
  - jet(`cannon.jet`): 24초짜리 세트 하나(요소 26개, 무지개 2회).
- 따라서 굵은 줄 = 버스트, 연한 줄 = jet으로 판단했다(밀도 근거, 추론).
- jet 루트는 서버 각도에서 직접 만든다(`Ground_Root(..., fCannonYawDegrees - 180)`, 기준 보정 0). 그래서 판정선과 같은 축이고, "연한 줄에 밀렸다"는 관찰과 맞는다.

### 9.2 원본 축 판단
- 소환 SkillEffect(Key 12)와 판정 박스 422560330이 각각 AreaOffsetAngle 90을 가진다.
  둘을 합하면 180°이고, 양방향 직선에서는 캐논 방향 +0°와 같다.
- 원본 액션의 FX_Turn 버스트(액터 루트 스냅샷, 공용 변환 경로)가 화면에서 판정선과 직각, 즉 캐논 방향 축에 있다.
- 결론: 원래 축은 캐논 방향 +0°(mod 180)다. 이전의 +90°는 미확정 선택이었고 틀렸다.
- 영상 두 개에서 머리 뼈가 돌기 때문에 액터 정면을 화면에서 따로 읽을 수는 없었다. 영상은 "한 줄"이라는 사실만 뒷받침한다.

### 9.3 수정
- `Shared/Public/Gameplay/MaharakaWaterpangContract.h` `Sample_MaharakaWaterpangEvent`:
  `fCannonYawDegrees = CANNON_YAW + 90 + turn` → `CANNON_YAW + turn`.
- 이 값 하나로 서버 판정선(`GameRoom_PartyWorld.cpp` 1565~1566), 사각 예고와 jet(`MaharakaWaterpangPresentation.cpp` 317, 321)이 함께 90° 옮겨 굵은 물줄기 축에 선다.
- 이펙트 문서, 빌더, WorldSequence는 바꾸지 않았다. 프로토콜 변경은 없고 **Server와 Client를 모두 다시 빌드**해야 한다.

### 9.4 수치 대조 (축, mod 180°)
| 조건 | t(s) | 서버 | 사각 예고 | jet | 버스트(관찰 축) |
|---|---|---|---|---|---|
| 19.2s 시계 | 0 | 43.59 | 43.59 | 43.59 | 43.90 |
| 19.2s 시계 | 5 | 137.34 | 137.34 | 137.34 | 137.65 |
| 19.2s 시계 | 14.9 | 142.97 | 142.97 | 142.97 | 143.28 |
| 9.5s 반시계 | 2.5 | 128.85 | 128.85 | 128.85 | 129.16 |
| 9.5s 반시계 | 9.6 | 39.80 | 39.80 | 39.80 | 40.11 |

0.31° 차이는 오브젝트 HOLD 쿼터니언(−136.10°)과 원본 캐논 yaw(−136.41°)의 차이다.

### 9.5 남은 불확실성
- 행렬만으로 계산하면(요소 Z × 기준 보정 −90° × 오브젝트 yaw × 회전) 버스트는 예전 판정선 위에 와야 한다. 관찰과 맞지 않으며, 코드에서 그 90° 차이의 출처는 찾지 못했다.
- 그래서 "굵은 줄 = 버스트" 판단이 틀렸다면(굵은 줄이 jet이라면) 빌드 후에도 두 줄이 직각으로 남는다.
  그때는 계약을 되돌리고, 버스트 인스턴스에 +90° 회전을 주고, jet 오프셋을 보정해야 한다.

### 9.6 확인할 것
- Server와 Client를 다시 빌드한 뒤 발사 중 물줄기가 한 줄로만 보이는지 확인한다.
- 파란 사각 예고가 그 줄과 같은 방향인지 확인한다.
- 밀리는 것이 눈에 보이는 굵은 물줄기인지 확인한다.

## 10. 물줄기가 여러 갈래로 보임 (스크린샷 200432) — 원인 규명과 한 줄 통일

사용자 관찰: Client.exe 19:57 빌드(9절 계약 변경 포함)에서 물줄기가 기둥을 지나는 세 줄(6갈래)로 보였다.
원작은 한 줄(두 갈래)이고, 밀리는 것도 그 줄이어야 한다. 이번에는 실제 런타임 식으로 다시 검증했다.

### 10.1 물줄기를 그리는 소스와 실제 루트
- 원본 위치 분포 형식은 `[Min, Max, min.xyz, max.xyz]`이다. jet(`cannon.jet`, Loop_800 26요소)과
  버스트(`att_battle_3_02.part0~3`의 WaterBomb_800_01 스프라이트·헬릭스/십자 메시)는 **모든 팔이 원본 ±Y** 위에 있다.
  원본→클라이언트 축 `(x,y,z)→(x,z,−y)`(`Effect_Playback.cpp` 320~323)로 둘 다 이펙트 로컬 Z 한 줄이다.
  헬릭스 메시의 두 번째 회전(seeded, 원본 Y 0~1회전)은 roll 90° 뒤 자기 긴 축 둘레 자전이라 줄 방향을 바꾸지 않는다
  (`UE3_EulerDegreesToClientRotation` 337~364, 설치 WModel 긴 축 = 클라이언트 Y 0.75m).
  `WaterBomb_800_01_01`(6초)은 상자 범위 무작위 속도의 물보라이고, Orb는 머리 뼈를 따르는 구다. 둘 다 직선이 아니다.
- 루트는 정확히 두 가지다.
  - jet: `Ground_Root(S + JET_ROOT_YAW_OFFSET(−180))`, 부착 해제(기준 보정 0) → 축 S.
  - 버스트: `Local × RotY(−90 스냅샷 기준) × RotY(오브젝트 yaw O = −136.10°) × post`
    (`Effect_Playback.cpp` 8306~8313, `WorldSequencePlayer_Objects.cpp` 1486~1592, post는 모든 입자에 곱해짐 9566~9626).
- 9절 계약(S = 223.59 + δ)과 8절 post(δ만)로는 버스트 축이 133.90 + δ, jet/서버/사각 예고가 43.59 + δ다.
  **모든 시각에서 89.69° 어긋난 두 줄**이다(10.4 수치).

### 10.2 9절의 모순 설명
- 9.5에서 "행렬로는 버스트가 예전 판정선 위인데 관찰은 다르다"고 한 모순은 실행 파일과 데이터의 세대 차이다.
  jet 문서는 실행 시 `Data/Effects/Authored`에서 바로 읽힌다. 8절이 문서를 기준 보정 0으로 바꾼 뒤, 오프셋 −180이 들어간
  exe가 빌드되기 전(스크린샷 194432, 19:44)에는 옛 오프셋 −90으로 돌았다. 그래서 jet = S − 90으로 버스트와 직각이었다.
- 9절은 이를 서버 축 오류로 보고 계약을 −90° 옮겼다. 새 exe(19:57, 오프셋 −180)에서는 오히려 jet·판정선이 버스트와 직각이 되었다.

### 10.3 수정
- `Shared/Public/Gameplay/MaharakaWaterpangContract.h`: `fCannonYawDegrees = CANNON_YAW + 90 + turn`으로 되돌림(9절 철회).
  **Server와 Client를 모두 다시 빌드**해야 한다. 프로토콜 변경은 없다.
- `Client/Public/WorldSequencePlayer.h`, `Client/Private/WorldSequencePlayer_Objects.cpp`: 선택 필드
  `TARGET_SET::objectEffectPostTransform`을 추가했다. 설정되면 World Object에 붙은 이펙트만 이 변환을 쓰고 모델은 그대로다.
  설정하지 않으면 기존 `objectWorldPostTransform`을 그대로 써서 쿠크 등 기존 소비자의 동작은 같다.
- `Client/Private/MaharakaWaterpangPresentation.cpp`, `Client/Public/MaharakaWaterpangPresentation.h`:
  - 캐논 모델 회전(8절 `objectWorldPostTransform`)을 제거했다. 원작처럼 몸체는 서 있고 머리는 클립 뼈만 돈다(이중 회전 제거).
  - 루프 인스턴스의 이펙트에만 held pivot 기준 `RotY(S + 90 − O)`를 준다. O와 pivot은 `Initialize`가
    CW/CCW 루프 템플릿 첫 키에서 읽는다(순수 yaw·두 템플릿 동일 검사, 어긋나면 공격 연출 비활성).
    이 값은 `Update`가 월드 샘플 전에 같은 서버 이벤트 샘플로 갱신한다. 결과 축 = −90 + O + (S + 90 − O) = S.
  - jet(S − 180), 사각 예고(데칼 길이 = 로컬 Z, `Shader_VtxEffectDecal.hlsl` 126~127), 서버 판정선이 모두 S다.
  - 진단 `Trace_JetAxes`: 발사 중 1초마다 `Client/Default/EffectFailure.user.log`에
    `channel=maharaka.waterpang.axis`로 server / jet / bursts / telegraph 축(mod 180), 모델 정면 축, effectTurn을 남긴다.
    jet은 실제로 넘긴 루트 행렬, 버스트는 실제 콜백이 돌려준 post 행렬로 계산한다.
- 이펙트 문서·빌더·WorldSequence 데이터는 바꾸지 않았다(재게시 없음).

### 10.4 수치 검증 (런타임 식 재현, `C:\Users\USER\.claude\jobs\46aea322\tmp\wp_axis_sim.py`)
문서의 실제 요소(스프라이트 팔, 메시 팔, 긴 헬릭스 축 3개 자전값)와 게시된 WorldSequence 쿼터니언으로 계산했다.

| 조건 | t(s) | 수정 전 서버=jet=예고 | 수정 전 버스트 | 수정 후 서버 | 수정 후 jet·버스트·예고 |
|---|---|---|---|---|---|
| 19.2s 시계 | 0 | 43.59 | 133.90 | 133.59 | 133.59 |
| 19.2s 시계 | 5 | 137.34 | 47.65 | 47.34 | 47.34 |
| 19.2s 시계 | 14.9 | 142.97 | 53.27 | 52.97 | 52.97 |
| 9.5s 반시계 | 2.5 | 128.85 | 39.16 | 38.85 | 38.85 |
| 9.5s 반시계 | 9.6 | 39.80 | 130.11 | 129.80 | 129.80 |

- 수정 전 최대 편차 89.70°, 수정 후 모든 소스·시각에서 0.00°.
- `cl /Zs` 구문 검사: MaharakaWaterpangPresentation.cpp, WorldSequencePlayer_Objects.cpp, Level_Development.cpp 모두 rc=0,
  오류·새 경고 없음(기존 C4828 제외). 산출물 없음. 빌드·실행은 하지 않았다.
- 인코딩: 새 줄은 각 앵커의 줄끝(CRLF/LF)을 따랐고 비ASCII 추가 없음. `git diff --check` 통과.

### 10.5 남은 불확실성
- 코드상 루트가 둘뿐이라 수치로 재현되는 것은 직각 두 줄이다. 스크린샷을 확대하면 뚜렷한 계열도 세로 물기둥과
  바닥 물 띠 둘이었다. 사용자가 그은 세 번째 줄에 대응하는 별도 소스는 코드·데이터에서 찾지 못했다(추정: 두 계열의 일부).
  다음 실행의 `maharaka.waterpang.axis` 줄에서 bursts가 server와 같고 modelFacing이 시간에 따라 변하지 않으면
  루트 불일치는 해소된 것이다. 그래도 여러 줄이 보이면 해당 시각 로그와 스크린샷으로 남은 소스를 좁힌다.
- 원작 기준의 절대 방향(캐논 정면 +90)은 원본 스냅샷 기준 보정(−90) 관례에 따른 것이다. 영상은 "한 줄"만 뒷받침한다.

### 10.6 사용자 확인
Server와 Client를 다시 빌드하고 Server를 재시작한 뒤 워터팡 회전 구간을 본다.
- 발사 중 물줄기가 기둥을 지나는 한 줄(두 갈래)로만 보이는지
- 캐논 몸체는 서 있고 머리만 도는지
- 파란 사각 예고가 그 줄과 같은 방향인지
- 밀리는 것이 눈에 보이는 그 물줄기인지

## 11. 반대로 도는 연한 물줄기 (스크린샷 204023) — 전체 요소 인벤토리와 한 줄 고정

사용자 관찰(Client.exe 20:37 빌드, 10절 포함): 굵은 물줄기는 서버선과 함께 도는데, 캐논을 지나는 연한
반투명 물줄기(두 갈래)가 반대 방향으로 따로 돈다. 스크린샷 시각의 `maharaka.waterpang.axis` 로그는
server=jet=bursts=88.59°였고 각도가 줄어드는 반시계 이벤트였다(로그는 행렬 계산값이며 입자 실측이 아니다).

### 11.1 연한 줄을 그린 요소 (확정)
- 문서: `effect.maharaka.waterpang.cannon.att_battle_3_02.part0~3.full.restore` (시계·반시계 루프 인스턴스가 같은 문서를 쓴다)
- notify: part0 `notify-006`, part1 `notify-021`, part2 `notify-034`, part3 `notify-047`
  (원본 `Par_G_ISMP_Attk03_WaterBomb_800_01_01`, 각 6초 구간)
- 요소: 각 notify의 `particlespriteemitter_7`, `particlespriteemitter_9` — 합계 8개.
  - 부착: 오브젝트 루트 스냅샷(`root`, 기준 −90°), 월드 공간 입자, 재질 `fx_d_pa_master_01_004_tr`, 알파 0.1~0.2(연함).
  - 초당 30개, 수명 0.25~0.3초, `particlemodulevelocityoverlifetime`(binworldspace) ×20 → 한 입자가 5~6m를 날아간다.
- 원인: 첫 번째 `particlemodulevelocity.startvelocity`가 상수가 아니라 이미터 시간 곡선이다
  (샘플 100개, 7번 timeScale 6.617·start −6.042, 9번 timeScale 10.987·start −0.077).
  크기 약 101cm/s 벡터가 6초에 원본 Z축 둘레로 −181° 돈다(7번 t=0 (4,−98) → t=3 (98,3) → t=6 (−6,98)).
  9번은 매 순간 7번의 정반대(t=3 (−100,−5))라 둘이 기둥을 지나는 한 줄(두 갈래)을 만든다.
  클라이언트 좌표로는 요소 로컬에서 약 +30°/s(시계)이고, 그 위에 서버 회전 post가 곱해진다.
  - 시계 이벤트: 월드 약 +48.75°/s (서버선 +18.75°/s보다 빠르게 같은 방향)
  - 반시계 이벤트: 월드 약 +11.25°/s (서버선 −18.75°/s와 **반대 방향**) — 스크린샷과 일치
  - 반시계 루프도 시계용(4225615) 문서를 쓰므로 곡선 방향이 뒤집히지 않는다.
- 10절 시뮬레이션이 놓친 이유: 위치 모듈(팔)만 계산하고 속도 곡선을 보지 않았다.
- 버린 가설(수치 근거): 머리 뼈 추종(Orb) 요소 36개는 머리 둘레 반경 1m 구면 방사·수직 속도라
  수평 방향이 없고 긴 줄을 만들 수 없다. `orientationaxislock`(epal_rotate_z) 메시 298개는 카메라 고정 정렬
  (`psma_meshfacecamerawithlockedaxis`)이 없어 루트를 따른다. `psa_velocity` 스프라이트는 native 경로
  (행1=속도)로 정렬되고 속도에 post가 곱해진다(`Effect_Playback.cpp` 9605). 문서 `particleSystem.yawOffsetDegrees`는 모두 0.

### 11.2 수정
- `Tools/EffectPipeline/build_maharaka_waterpang_source_effects.py`: `pin_spray_to_stream_axis`(멱등)를 추가하고
  `project()`에서 루프 part 문서마다 호출(고정 개수 2 assert). 시간 곡선(`lookupTableTimeScale>0`)인
  `startvelocity`만 대상이며, 각 샘플을 원본 ±Y(물줄기 축)로 바꾸되 부호는 이미터 시간 0의 방향,
  크기는 샘플별 값을 유지한다(7번 → −Y, 9번 → +Y, 두 갈래 유지). 포터블 요약 `initialVelocityMin/Max`도 같은 값으로 맞춘다.
  무작위 범위(±5, timeScale 0) 분포는 바꾸지 않는다.
- `Data/Effects/Authored/...att_battle_3_02.part0~3.full.restore.effect.json`: 위 함수를 적용. 감사 결과 각 문서에서
  정확히 요소 2개의 `sourceRecipe.modules[3]`(속도 곡선)과 `detail.particle.initialVelocityMin/Max`만 바뀌었고
  두 번 적용해도 바이트 동일. JSON 형식은 빌더 출력과 바이트 동일한 직렬화. 이펙트 문서는 실행 시 직접 읽히므로 게시 단계는 없다.
- `Client/Private/MaharakaWaterpangPresentation.cpp` `Trace_JetAxes`: 진단 줄에 `spray`(고정된 물보라 축),
  `serverSpin`, `headSpin`(3_02=cw, 3_03=ccw)을 추가했다. 동작 코드는 바꾸지 않았다.
- WorldSequence·Server·Shared·리소스 변경 없음. 원본 백업: `C:\Users\USER\.claude\jobs\46aea322\tmp\spray_pin_backup`.

### 11.3 전체 요소 시뮬레이션 (`wp_full_sim.py`, 런타임 식 재현)
대상: 캐논 이벤트 중 살아 있는 문서 7개(start 3_01, loop part0~3, end 3_04, jet)의 요소 **1,443개 전부**와 사각 예고.
조건: 시계/반시계 × 보통 19.2초/빠르게 9.5초, 발사 0/2.5/5/9.6/14.9초. 방향 운반자별로 서버선과의 최대 편차(°, mod 180).

| 소유 | 운반자 | 원본 이미터 | 요소 | 수정 전 | 수정 후 |
|---|---|---|---|---|---|
| loop | 위치 팔 | WaterBomb_800_01 | 1274 | 0.00 | 0.00 |
| loop | 메시 긴 축 | WaterBomb_800_01 | 392 | 0.00 | 0.00 |
| loop | 속도 | WaterBomb_800_01 | 490 | 0.00 | 0.00 |
| loop | 속도(시간 곡선) | **WaterBomb_800_01_01** | 8 | **89.11 (선 밖)** | 0.00 |
| jet | 위치 팔 | WaterBomb_Loop_800_01 | 24 | 0.00 | 0.00 |
| jet | 메시 긴 축 | WaterBomb_Loop_800_01 | 6 | 0.00 | 0.00 |
| jet | 속도 | WaterBomb_Loop_800_01 | 8 | 0.00 | 0.00 |
| start/loop/end | 머리 뼈 추종 Orb | Orb_Start/Loop/End | 36 | 방향 없음(중심) | 동일 |
| end | 노즐 물보라 | WaterFinish_01 | 6 | 머리 방향 ≤1.2m | 동일 |
| 사각 예고 | 데칼 길이축 | GR_Mon_Rectangle | 1 | 0.00 | 0.00 |

(요소 수는 조건·시각에 걸쳐 방향 운반자가 잡힌 고유 요소 수다. 한 요소가 위치·속도 둘 다 가지면 양쪽에 센다.)

- 머리 회전: 설치 `MN_ISMP_00-1.wmodel`의 `b_ismp_root`를 샘플링했다. 3_02는 24초에 +720°(시계, 가감속, 평균 30°/s),
  3_03은 −720°(반시계). 시계 이벤트는 3_02, 반시계는 3_03 인스턴스라 **방향은 서버와 같다.** 속도는 클립 값이라 서버(18.75/37.9°/s)와 다르다.
  캐논 몸체는 스케일 [1,1,1]·순수 yaw라 반전이 없고 10절 이후 돌지 않는다.
- 남긴 것: `WaterFinish_01`(끝 1초만, 소켓 FX_01 머리 앞 0.5m·위 0.9m, 반경 30cm 실린더, 30~50cm/s·0.5~0.8초)은
  캐논 축에서 최대 약 1.2m인 노즐 물방울이다. 7.5m 물줄기 같은 선이 아니고 원본도 FX_01 노즐에 붙는다. 물줄기가 끝난 뒤에만 나온다.

### 11.4 검증
- `cl /Zs` 구문 검사: `MaharakaWaterpangPresentation.cpp` rc=0, 기존 C4828 외 오류·경고 없음. 산출물 없음.
- 문서 단위 검증 함수(`validate_effect_sources.py`의 색공간·모듈 override·부착 방향·native 스프라이트 옵션) 통과.
  v15 runtimeExtensions 검사는 이 v13 문서에서 원본(백업)부터 같은 결과로 실패하는 비대상 검사다.
- 인코딩: 문서 UTF-8/LF, 빌더 UTF-8/LF(기존 비ASCII 12바이트 유지), 소스 ASCII/LF. `git diff --check` 통과.
- 빌드·실행은 하지 않았다. 화면 판정은 사용자 몫이다.

### 11.5 사용자 확인
Client를 다시 빌드한다(서버·데이터 게시 불필요, 이펙트 문서는 다음 실행에서 읽힌다).
- 시계·반시계, 보통·빠르게 회전 모두에서 물줄기가 기둥을 지나는 한 줄(두 갈래)로만 보이고 연한 줄이 따로 돌지 않는지
- 물줄기가 끝나는 1초 동안 머리 노즐 주변 물방울만 보이는지
- `Client/Default/EffectFailure.user.log`의 `maharaka.waterpang.axis` 줄에서 server=jet=bursts=spray이고 serverSpin=headSpin인지

## 12. 큰 모코모코 물벼락(4225601) 사운드·이펙트 전수 연결

사용자 요청: 물벼락도 사운드와 이펙트를 전부 원작대로 연결하되, 캐논 물줄기 때처럼 일부만 맞추는 실수 없이
전수 검증할 것. 빌드·Client 실행은 하지 않았다. 화면·소리 판정은 사용자 몫이다. 커밋 없음.

### 12.1 전수 인벤토리 (6초 행동 중 그리거나 재생하는 것 전부)
- 이펙트 문서 `effect.maharaka.waterpang.mokomoko.att_battle_1_01.full.restore` 33요소
  (WorldSequence `attack.mokomoko` 인스턴스의 V1 트랙, 모코모코 오브젝트 포즈·클립 att_battle_1_01에 붙음):
  - Face_01 4개(0.592초, FX_01 추종), Attk01_Water_01 13개(2.293초, FX_01 추종: 메시 3·스프라이트 10),
    Attk01_WaterGround_01 10개(2.390초, 루트 스냅샷), WaterFinish_01 6개(5.072초, FX_01 추종).
  - 문서에서 빠진 보류 이미터 1개: WaterFinish_01 `particlespriteemitter_6`(연기, 프로그램 3633,
    `bfx_d_pa_smoke_01_03_tr`). 원인: 동적 파라미터 VF(TEXCOORD7) 스프라이트 운반자 어댑터 없음. 캐논 보류분과 같은 원인.
- 도넛 예고 `effect.maharaka.waterpang.mokomoko.telegraph` 1요소(데칼 파티클, 프로그램 3654), 0~2.3초.
- 사운드 AKEvent 2개: Cast1(0초), Shot1(2.2초). 모코모코(MN_ISMP_00)는 Water1.
- 도입 HOLD 인스턴스(`intro15.mokomoko`)는 효과·사운드 트랙이 없다(재질 트랙만). 공격 중에는 `Show_Actor`가 HOLD를 멈춘다.
- 서버 판정: 69틱(2.3초) 모코모코→캐논 방향 ±45°, 3~4m 밀림/4~7m 띄우기(변경 없음).

### 12.2 발견한 결함과 원인 (런타임 식으로 계산)
시뮬레이션 `C:/Users/USER/.claude/jobs/46aea322/tmp/moko_final.py`
(오브젝트 쿼터니언 → `b_ismp_root` 결합행렬 × PreTransform → 소켓 S·RollPitchYaw·T → 요소, 스냅샷은 RotY(기저)×루트).
1. **추종 23요소가 1/100 크기**: WorldSequence 오브젝트 모델은 `XMMatrixScaling(modelPreScale=0.01)`로 만들어지고
   (`WorldSequencePlayer_Objects.cpp` 636), 결합행렬 끝에 그 PreTransform이 곱해진다(`Model.cpp` 661~681).
   MN_ISMP_00은 x100 루트 노드가 없는 cm WModel이라 뼈 기저 행 길이가 0.01이다. 미터 소켓·요소 오프셋이
   그대로 1/100로 줄어 얼굴·물줄기·종료가 머리 중심 1~6cm 안에 뭉쳐 사실상 보이지 않았다(수정 전 표 SCALE 0.0100).
   쿠크 리그는 x100 루트 기저가 있어 같은 식이 맞는다(`KoukuSaydonPresentationPlayer.cpp` 784~789 주석).
2. **FX_01 소켓 회전 축 오류**: 원본 소켓 RelativeRotation Roll=-16384는 UE **X축** 회전인데 공용 소켓 계약
   (`build_esther_inanna_source_effects.socket_contract`)이 이를 `rotationDegrees.z`(DirectX Z축 roll)에 넣었다.
   크기만 고치면 물줄기가 -144°~-27°로 흩어진다. 후보 7개 비교에서 **뼈 X축 +90°만** 물 요소 전부를
   캐논 쪽 3.6~6.0m에 떨어뜨렸다(`moko_socketrot.py`). Y 미러 뼈 공간에서 UE X축 -90°는 +90°다.
3. **지면 튀김이 옆으로 90°**: 루트 스냅샷 요소의 기저 -90°는 캐릭터 루트(+Z 정면)용이다. 이 오브젝트는
   DeployData yaw(225.88°)를 +X 정면 메시에 그대로 쓰므로 +X가 원본 정면이다. 기저 -90°로 4.5m 튀김이 -90.8°에 있었다.
4. **도넛 부채꼴이 뒤쪽**: 프로그램 3654는 `|atan2(u-.5,v-.5)| < pi*angle`(+V 중심)이고 LocalDecal의
   `decalUV.y = 0.5 - local.z/size`라 +V는 데칼 로컬 -Z다(`Shader_VtxEffectDecal.hlsl` 138~140). 데칼 파티클
   월드는 루트 yaw를 쓰므로(`Effect_Playback.cpp` 9062~9071) 루트 +Z를 캐논 쪽으로 둔 기존 코드는 부채꼴을 반대편에 그렸다.
   반경은 thickness 0.4286×0.5×14m=3.0m ~ 7.0m, angle 0.25 → ±45°로 서버와 같다(파라미터 표 `ARTIST_PARAMETERS_3654`).
5. **사운드 소비자 없음**: G07이 원본 미디어 6개와 `CharacterSoundCatalog.json`의 `Maharaka` 클래스를 이미 설치했지만
   클라이언트에서 이 클래스를 재생하는 곳이 없었다. Wwise 재확인: `SOUND_MOB4`, Cast1 → [884290698, 1068667522, 883323926],
   Shot1 → [170625403, 224393252, 605718293](각 Play 1개 → RanSeqCntr 동일 가중치 3개), 카탈로그 파일과 일치.

### 12.3 수정
- `Tools/EffectPipeline/build_maharaka_waterpang_source_effects.py`: `bind_mokomoko_object_rig`(멱등, 모코모코 문서만,
  (23,10) assert) 추가 후 `project()`에서 호출. FX_01 추종 요소는 소켓 배율·위치 ×100(차량 `WING_BONE_BASIS_INVERSE`와
  같은 규약), 회전 (0,0,90) → (90,0,0)(단일 축 원본만 허용, pitch/yaw 있으면 assert). 루트 스냅샷 10개는 기저 0.
  공용 `socket_contract`와 다른 코호트는 바꾸지 않았다.
- `Data/Effects/Authored/effect.maharaka.waterpang.mokomoko.att_battle_1_01.full.restore.effect.json`: 빌더로 재생성·설치.
  후보 모드에서 바뀌는 파일이 이 문서 1개뿐임을 확인했고, 33요소 모두 `actionCueAttachment`만 바뀌었다(다른 필드 동일).
  원본 백업: `C:/Users/USER/.claude/jobs/46aea322/tmp/moko_backup`.
- `Client/Private/MaharakaWaterpangPresentation.cpp`/`.h`:
  - 도넛 루트 yaw = 서버 부채꼴 중심 + 180°.
  - `Update_WaterfallSounds`: 물벼락 이벤트 시계로 Cast1(0ms, 창 3133ms)·Shot1(2200ms, 창 3800ms)을 캐스트마다 1회,
    `CSoundCueCatalog::Find_Variants("Maharaka", ...)`의 3개 중 무작위(직전 변형 회피), 늦은 입장은 지난 만큼 건너뛰고
    (`Play_SoundCue` ageMs), 창이 지나면 재생하지 않는다. Shot은 `endMs=3800`으로 6.0초에서 끊는다(G07 규칙).
    이벤트 종료·발생 번호 변경·Stop·저작 모드 전환(`Stop_AttackEffects`)에서 정지. 엔진 사운드는 2D이며 기존 워터팡 도입 효과음과 같은 정책이다.
  - `Trace_Waterfall`: 캐스트마다 판정 틱(69)에 `EffectFailure.user.log`의 `maharaka.waterpang.waterfall` 한 줄
    (서버 부채꼴 중심, 도넛 축, 오브젝트 정면, 4.5m 지면 튀김 거리·각도, Cast/Shot 변형과 시작 age).
- 리소스: 새 파일 없음. 이번에 소비하게 된 Water1 WAV 6개
  (`Sound/Maharaka/S_MOB_MOCOCOWATER1/mococowater1_attack01_{cast1__884290698,cast1__1068667522,cast1__883323926,
  shot1__170625403,shot1__224393252,shot1__605718293}.wav`)를 `CY_Resources`에 같은 경로로 복사(해시 동일).

### 12.4 전수 검증 (`moko_final.py`, 서버 중심 -43.30°, 모델 +X -44.12°)
| 계열 | 요소 | 수정 전 | 수정 후 |
|---|---|---|---|
| Face_01 | 4 | 기저 0.01, 머리 중심 1cm | 기저 1.0, 노즐 0.15~0.81m(얼굴 광원) |
| Water_01 노즐 플래시 | 2 (e10, e18) | 기저 0.01 | 노즐 0.05/0.20m |
| Water_01 물줄기 | 11 | 기저 0.01, -144°~-27° | 끝점 3.62~6.00m, -32.8°~+31.2°(중간값) |
| WaterGround_01 큰 튀김(e9, 8m) | 1 | 4.50m **-90.8°** | 4.50m -0.8°, 높이 22.41(무대) |
| WaterGround_01 발밑 링·물결 | 9 | 2개 -90.8° | 0~2.3m 전방(-0.8°), 22.36~22.88 |
| WaterFinish_01 | 6 | 기저 0.01 | 노즐 0.00~0.34m |
| 도넛 예고 | 1 | 부채꼴 중심 **+136.7°**(반대편) | -43.30°(서버 중심과 일치), 3~7m, ±45° |
- 수정 전 33개 중 26개 불일치 → 수정 후 0개. 물 스프라이트 속도·위치 무작위 범위의 극값 조합까지 계산하면
  노즐에서 1.5m 넘게 가는 점은 모두 2.50~7.00m, 최대 ±42.1°(`moko_extremes.py`). 서버 부채꼴(3~7m, ±45°) 안이다.
  2.5~3m 값은 노즐에서 출발한 공중 물방울이다. 발밑 링은 원본대로 루트 기준(0~2.3m)이며 안전 구역이다.
- 시각: 물줄기 2.293초, 지면 2.390초, 도넛 0~2.3초, 서버 판정 2.3초(69틱), Shot 2.2초.
- 도구 검사: 문서 단위 검증 4종(색공간·모듈 override·부착 방향·native 스프라이트 옵션) PASS, 참조 리소스 30개 존재.
  `cl /Zs`(MaharakaWaterpangPresentation.cpp) rc=0, 기존 C4828 외 경고·오류 없음. `git diff --check` 통과.

### 12.5 미해결 (추측으로 채우지 않음)
- PawnMaterialParam 2개(`PetEmotion`/`opacity_intensity`, 0~6초): 값 블록이 비정렬 가변 레이아웃이라
  `CEFActionNotify_PawnMaterialParam` 클래스 필드 구조(시간·값·enum 1/2 의미)가 없으면 증명할 수 없다. 도입의
  `opacity_intensity` 소비자(materialTracks, 1=웃는 얼굴)는 있으나 공격 notify 값을 거기에 옮기려면 곡선 의미가 필요하다. 연결하지 않았다.
- 보류 연기 이미터 1개(3633): TEXCOORD7 스프라이트 운반자 어댑터 필요.
- 지면 notify의 원본 -200cm는 루트 기준 수직축으로 해석했다. 모코모코 오브젝트 루트는 G04의
  PROJECT_FRAME_CONTACT_ADAPTER로 원본 액터(20.56m)보다 3.79m 높은 24.36m라, 결과 높이 22.36~22.41m가 무대(22.4m)와 맞는다.
- 캐논 문서(3_01/3_04/part0)의 FX_01 추종 요소(Orb 등 37개)도 같은 0.01 기저·소켓 축 문제를 가진다. 이번 범위(물벼락) 밖이라 바꾸지 않았다.
- 공용 소켓 계약이 UE Roll을 Z축에 넣는 문제는 다른 코호트(인나나·차량 등)의 roll 있는 소켓에도 해당할 수 있다. 확인하지 않았다.

### 12.6 사용자 확인
Client만 다시 빌드한다(Server·데이터 게시 불필요, 이펙트 문서와 사운드 카탈로그는 실행 시 읽힌다).
경기 시작 120초·150초·340초에 큰 모코모코 물벼락:
- 0~2.3초 붉은 도넛 예고가 모코모코 앞(아레나 중앙 쪽) 90° 3~7m에 보이는지
- 0초 Cast 소리, 0.6초 얼굴 빛, 2.2초 Shot 소리, 2.3초 머리 노즐에서 앞쪽 3~7m로 물줄기, 2.4초 앞 4.5m 바닥 물 튀김
- 물줄기가 떨어진 곳에 서 있으면 밀리거나 띄워지는지(판정과 화면 위치 일치)
- 5.1초 노즐 물방울, 6초에 Shot 소리가 끊기는지
- `Client/Default/EffectFailure.user.log`의 `maharaka.waterpang.waterfall` 줄에서 sector≈donutAxis≈modelFacing(±1°),
  groundSplash≈4.50m/±1°, cast/shot 변형 이름과 시작 age가 찍히는지

## 13. 공격 연출을 팀 방식(도구 편집 데이터)으로 전환 (예약 작업 #17)

사용자 요청: 워터캐논·물벼락 연출의 재생 시점·배치·회전·사운드를 C++ 하드코딩에서 빼서
이펙트 작업자가 저작 도구에서 보고 고칠 수 있는 데이터로 옮긴다. 빌드·실행은 하지 않았다. 커밋 없음.

### 13.1 선택한 도구 경로와 근거
- 공격 연출은 이미 WorldSequence `attack.*` 인스턴스(클립 + V1 이펙트 트랙)였고, 코드에 남은 것은
  사각·도넛 예고, 투사체 jet, 물벼락 사운드와 그 위치·각도·시간이었다. WorldSequence 템플릿은
  이펙트 트랙(위치·회전·배율·뼈·추종·루프)과 사운드 트랙(에셋·시작·구간·음량)을 이미 표현한다.
- World Object Tool은 쿠크 Area 전용(`WorldObjectTool.cpp` 43 `AREA_ID`)이다. 마하라카에서 OBJECT_RESOURCE
  배우를 편집·미리보기하는 기존 도구는 MapTool → Camera → 컷신 "World 배우" 편집기다
  (`MapTool_CameraShots.cpp` Render_CutsceneActorSection: 앵커·키·클립·사운드·자막, 저장은 카메라+World
  저작본 한 트랜잭션). 카메라 컷 없는 미리보기 컷신은 팀이 이미 쓰는 형식이다(바닥 흔들림·붕괴·복구).
- 따라서 새 런타임 없이: 공격 인스턴스를 미리보기 컷신으로 등록하고, 이 편집기에 이펙트 행 편집만 추가했다.
  서버 판정 시계는 그대로 공용 계약 샘플러이고, Client는 단계별 stable instance 선택·서버 시계 seek·
  loop instance 이펙트 회전(서버 각도)만 한다.

### 13.2 코드에서 데이터로 옮긴 것
| 이전 코드 | 데이터(WorldSequence 템플릿) |
|---|---|
| 사각 예고 `Ground_Root(C, S)` 0~45틱 | `attack.cannon.start` / `fx.cannon.start.telegraph`: 0~1500ms, inherit off, 회전 (0,313.59,0), 오프셋 (-0.006,0,0.013) |
| jet `Ground_Root(C, S-180)` 매 프레임, 문서 부착 해제 | `attack.cannon.loop.cw/ccw` / `fx.cannon.loop.*.jet`: 0~24000ms, loopEffectToDuration, 트랙 yaw 180 + 원본 스냅샷 기준 -90 |
| 도넛 `Ground_Root(M, F+180)` 0~69틱 | `attack.mokomoko` / `fx.mokomoko.telegraph`: 0~2300ms, inherit off, 회전 (0,136.698035,0), 오프셋 (-0.00004,-0.689361,0.000375) |
| Cast1/Shot1 재생 코드 | `attack.mokomoko` 사운드 트랙 `sound.mokomoko.cast1` 0/3133ms, `sound.mokomoko.shot1` 2200/3800ms, 음량 1 |
- `MaharakaWaterpangPresentation.cpp/.h`: Ground_Root·예고/jet/도넛 스폰·사운드 코드와 멤버를 제거했다.
  진단 `maharaka.waterpang.axis` / `.waterfall`은 이제 템플릿 트랙 데이터와 문서 스냅샷 기준으로 각 트랙의 축을 계산해
  `tracks=[id=axis ...]`, `sounds=[...]`로 남긴다.
- 게시: WorldSequences revision 7→8(트랙 6개 추가), CameraShots 5→6(미리보기 컷신 5개 추가).
  `Publish-MapAuthoring.ps1 -Scope WorldSequences/CameraShots` Validate·Publish·Check 모두 exit 0.
  저작본·게시본 대조에서 추가 행과 revision 외 변경 없음.

### 13.3 원본 값 수정 재검토
- jet 스냅샷 해제(`follow_live_root`): 제거. jet 문서는 원본 스냅샷 부착(기준 -90)으로 돌아갔다.
  회전은 굵은 물줄기와 같은 이펙트 회전(Build_PresentationFrame)이 맡는다.
- 물보라 속도 곡선 고정(`pin_spray_to_stream_axis`): 유지. 원본은 머리 회전을 따라 도는 물보라라
  "물줄기 한 줄" 요구와 충돌한다(11절). 사용자 요구로 인한 의도적 변경이다.
- 머리 리그 바인딩: 엔진 적응(cm 모델 × modelPreScale 0.01, 소켓 축)이라 유지하고 캐논에도 적용(13.5).
- 사운드: 원본 RanSeqCntr 3변형 무작위 → 트랙 하나에 첫 변형 고정(`__884290698`, `__170625403`).
  WorldSequence 사운드 트랙은 에셋 하나라서다. 변형 교체는 트랙 assetId 수정으로 한다(MapTool은 시간·음량만 편집).

### 13.4 작업자 편집 방법
Debug Client → 마하라카 → F1 → Map Tool → Camera → 컷신 `워터팡 / 공격 · …(공격 연출 미리보기)` 5개 중 선택 →
Play/Pause/시간 → "World 배우"에서 인스턴스 선택 → 키·클립 / Effects(시작·길이·오프셋·회전·배율·뼈·추종·회전 상속·루프·맞춤,
Apply Effect) / Sounds(시작·길이·음량, Apply Sound) → "컷신 저장 (카메라 + World 저작본)" →
`powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Scope WorldSequences -Mode Publish`.
- MapTool 미리보기에는 서버 회전이 없어 loop 물줄기가 시작 방향에 머문다(경기에서는 서버 각도로 돈다).
- 이펙트 행 추가·삭제·리소스 교체는 이 편집기 범위 밖이다(JSON 또는 빌더 시드).
- 빌더 `--sequences`는 없는 stable ID 행만 추가하고 편집된 행을 덮지 않는다(재실행 0건 확인).
- 공용 변경: `MapTool_CameraShots.cpp`/`MapTool.h`에 이펙트 행 편집 섹션(모든 Area의 컷신 배우에 추가되는 편집 UI, 런타임 영향 없음).

### 13.5 캐논 머리 추종 요소 37개
설치 `MN_ISMP_00-1.wmodel` `b_ismp_root` 결합 행렬 × PreTransform 0.01의 기준 길이가 3_01/3_02/3_04 모든 시점에서 0.01000
(`cannon_basis.py`). 빌더 `bind_object_rig`(모코모코와 같은 규약, 루트 스냅샷 기준은 유지)를 3_01(9), part0(11), 3_04(11+FX_01 6)에 적용.
필드 감사: 부착 `socketLocalTransform`만 변경(3_01 27값, part0 33값, 3_04 75값), 결과 기준 배율 0.01 → 1.0.

### 13.6 동일성 검증 (`team_path_equiv.py`, 이전 코드 식 대 게시 데이터 + 런타임 식)
| 대상 | 범위 | 회전 차이 | 위치 차이 |
|---|---|---|---|
| 사각 예고 (1요소) | 0~44틱 | 0.00000° | 0.00001m |
| jet 26요소 × {시계,반시계} × {보통15·20초, 빠르게15·10초} | 모든 발사 틱 | 0.00001° | 0.01431m |
| 도넛 예고 (1요소) | 0~68틱 | 0.00012° | 0.00002m |
| Cast1 / Shot1 | 시작·구간 | 0ms / 0ms | - |
- 시간 창: 예고 트랙 1500/2300ms가 이전 45/69틱 조건과 모든 틱에서 같다(검사). jet·루프는 같은 instance 시계.
- jet 위치 1.43cm: 이전 중심은 계약 상수 (75.044, -984.307), 지금은 굵은 물줄기와 같은 오브젝트 중심 (75.05, -984.32).
- 의도적 차이: jet의 월드 공간 스프라이트 8개(수명 ≤0.6초)는 이전에 뿜어진 시점 각도에 남아 최대 11.25°(빠르게 22.74°)
  뒤처졌고, 이제 굵은 물줄기처럼 현재 서버선 위에 머문다. 나머지 18요소는 로컬 공간이라 동일.
- 문서: 모코모코·part1~3·사각/도넛 예고 바이트 동일, 바뀐 4개는 부착 필드만.
- 빌더 재실행: 이펙트 0건, 시퀀스·컷신 이미 시드됨.
- `cl /Zs`: MaharakaWaterpangPresentation.cpp, MapTool_CameraShots.cpp EXIT 0, C4828 외 경고·오류 0. 소스 ASCII/LF,
  MapTool 파일은 추가 줄 CRLF(기존 LF 줄 129개·비ASCII 유지). `git diff --check` 통과.

### 13.7 사용자 확인
Client를 다시 빌드한다(Server 변경 없음, 데이터는 게시됨).
- 경기: 사각 예고·jet·굵은 물줄기가 한 줄로 서버선과 함께 돌고, 물벼락 도넛·Cast/Shot 소리가 이전과 같은 시각에 나오는지.
- 로그: `maharaka.waterpang.axis`의 `tracks=[fx.cannon.loop.*.part0..3=… fx.cannon.loop.*.jet=…]` 값이 모두 `server`와 같은지.
- 캐논 시작·끝의 머리 주변 구슬·끝 물방울이 이제 실제 크기(1m급)로 보이는지.
- MapTool → Camera → 공격 미리보기 컷신에서 Effects/Sounds 행이 보이고 Apply·저장·Reload 후 값이 유지되는지.

## 14. 물벼락을 아레나 전체로, 맞으면 한 번에 낙사 (스크린샷 223544)

사용자 요청: 물벼락 범위가 무대 일부(모코모코 앞 90° 3~7m)에만 나온다 → 아레나 전체를 덮고, 맞으면
한 번에 날아가 낙사(→ jump1/jump2 부활)해야 한다. 빌드·실행 없음, 화면 확인 없음, 커밋 없음.

### 14.1 원본 근거 (읽기 전용)
- `MN_ISMP_00.Action.loa`에 경기장 전용 변형이 있다. 4225612 "아레나_모코코 물벼락_물벼락 쏟아내기",
  4225617 "아레나_중앙_모코코 물벼락". 같은 6초 클립 `Att_Battle_1_01`이고 파티클은 전용
  `Par_G_ISMP_Arena_Attk01_Water_01` / `Par_G_ISMP_Arena_Attk01_WaterGround_01`이다(4225601은 일반판).
- 4225612 판정(SkillEffect 4225612xx): 모코모코 앞 3.2m(AreaOffsetX 320)를 원점으로 180° 반원.
  2.30초 1.2~2.5m, 2.46초 2.5~5.9m 두 파동. 202/206은 HitType 5 띄우기 580~600cm / 1814~1834ms /
  높이 200cm, FallDown. 피해 행 없음. 4225617은 앞 5.92m 중심 반경 2.9m 원(2.45초).
- 예고 데칼: 4225612 PlayDecalEffect → SkillDecal **1106** `GR_Mon_Donut_cond_01`(4225601은 1006 behit).
  data3.lpk `10_EF_PARTICLE_SOUND_DATA_GROUND_EFFECT_GR_Mon_Donut_cond_01.loa`: 재질
  `FX_O_De_CondMonDonut_02_01_Tr`, 활성색 [0.01, 0.1, 1.0, 3.0](파랑). 창 2.3초, 채움 2.0초.
- 원본 한 번의 공격은 모코모코 쪽 절반만 덮는다(모코모코는 중심에서 9.9m). "아레나 전체 + 무조건 낙사"는
  사용자 규칙(PROJECT)이다. 원본 AI가 4225612/4225617 중 무엇을 언제 쓰는지는 서버 AI 표라 미확인.
- 무대 실측(WaterpangEntry navgrid): 중심 (75.044, -984.307) 반경 약 7.45m 원판, 높이 22.30~22.43,
  주변 물 20.48. jump1(-134.8°)/jump2(+43.7°) 방향으로만 같은 높이 통로가 13.4m까지(각 ±7~9°).

### 14.2 서버 (권위, 공유 계약)
- `MaharakaWaterpangContract.h`: 부채꼴 상수(INNER/MIDDLE/OUTER/HALF_ANGLE/PUSH_M/PUSH_MS) 제거,
  `DECK_RADIUS_M 7.5`, `DECK_MIN_Y_M 21.5`, `PIER_HALF_ANGLE_DEGREES 12`, `WATERFALL_SECOND_HIT_TICK 74`,
  `WATERFALL_ORIGIN_FORWARD_M 3.2`, `WATERFALL_INNER_WAVE_M 2.5`, `WATERFALL_LAUNCH_M 5.9`,
  `WATERFALL_EXIT_RADIUS_M 9` 추가. 프로토콜 변경 없음(Server·Client 모두 재빌드).
- `GameRoom_PartyWorld.cpp` 물벼락 분기: 무대(반경 7.5m, y ≥ 21.5) 위 전원. 원점 2.5m 안은 69틱(2.30초),
  나머지는 74틱(2.46초). 아레나 중심에서 바깥으로(대포 위면 모코모코 정면 방향) 탄도 발사, 높이 2m,
  1824ms. 거리 = max(5.9m, 끝점이 중심 9m 밖이 되는 거리). 끝점 방위가 jump 통로 ±12° 안이면 방향을
  2°씩 더 돌린다. 발사가 실제로 무장되면 `bWaterpangLaunch`를 켠다. 발사 중에는 물대포·물벼락 판정 제외.
- `ServerPlayer.h` `bWaterpangLaunch` 추가. `GameRoom_PlayerSimulation.cpp`: 이 발사는 bounded가 아니며,
  무대·통로·물 바닥 어디에도 착지하지 않고 비행 종료 시 `Begin_PlayerFall` + `bWaterpangFall` →
  기존 45틱 낙하 뒤 가까운 jump 박스 부활. 낙하 시작·사망 시 플래그 해제.
- 원본과 다른 점(PROJECT): 원본은 캐스터 기준으로 밀고 반원만 덮는다. 여기서는 전 무대를 덮고 아레나 중심
  기준 바깥으로 밀어(대포 기둥 충돌 회피) 무조건 무대를 벗어나게 한다. 원본 띄우기 수치(5.9m·2m·1.824초)는 유지.

### 14.3 연출 (팀 방식 데이터 + 원본 복원)
- 빌더 `build_maharaka_waterpang_source_effects.py`:
  - mokomoko 프로필 action 4225601 → **4225612**. 문서 ID 동일. 바닥 notify는 제외(`mokomoko_actions`).
    얼굴·물·종료 22요소(머리 추종). 연기 1개는 기존과 같은 이유로 보류.
  - `ground` 프로필 신설: 4225612 바닥 notify만 담은 루트 문서
    `effect.maharaka.waterpang.mokomoko.ground.att_battle_1_01.full.restore`(17요소).
  - `seat_arena_columns`: 경기용 모코모코 자세가 원본보다 높아(과거 PROJECT_FRAME_CONTACT_ADAPTER) 토네이도
    물기둥 밑면이 무대 1.1~1.3m 위에 떴다. 소켓 위쪽 축으로 1.10m 내려 두 기둥은 무대(22.45), 원본상 20cm
    높은 기둥은 22.65에 놓았다(원본 상대 배치 유지).
  - 예고: 데칼 1106 cond 도넛(쿠크 템플릿과 같은 재질, 설치된 프로그램 3601) 파랑, 두께 0·각도 1 = 무대 전체
    원판 반경 7.5m, 아레나 중심.
  - 공격 시퀀스 행: 예고 트랙을 아레나 중심으로 이동, 바닥 튀김 트랙 `fx.mokomoko.ground.0..5` 추가
    (모코모코 자신의 방위부터 60°마다, 같은 9.9m 거리에서 중심을 향함 = 각 배치가 원본 한 번 공격의 구성,
    루트 높이는 가장 낮은 튀김 스프라이트가 무대 위 2cm).
- 재질: 새 조합은 경기장 큰 물웅덩이 `bfx_d_pa_circ_01_06_tr` 스프라이트 1개 → 프로그램 **3655**
  (Group3648 + 디스패치 3648에만 추가). 첫 실행에서 새 배정이 기존 3621과 겹쳐 `--native-first 3655`로
  배정하고 설치는 `--native-first 3621 --native-last 3679`로 했다. 재질 표·셰이더는 3655 추가뿐, 다른 줄 삭제 0.
  `Client.vcxproj`/`.filters`는 설치 전후 바이트 동일(설치기가 다시 쓰는 경우를 대비해 비교).
  동시 진행 중인 물총 fork의 4940/4941 행은 그대로 보존.
- 메쉬 굽기: 변환기가 한글 경로를 못 열어 ASCII 임시 경로로 변환 후 빌더 legacy 경로에 두었다.
  새 Resources: `Effect/Maharaka/Waterpang/Meshes/fm_d_wave_010.wmodel`, `fm_h_tornado_01_1.wmodel`.
  참조 31개 모두 존재. CY_Resources에 위 2개와 `Effect/KoukuSaydon/FullRestore/Meshes/fm_m_ring_001.wmodel` 복사.
- 설치: 바뀐 파일 = 모코모코 문서, 바닥 문서(신규), 예고 문서, EffectCatalog(행 1개 추가, 삭제 0).
  캐논 문서는 바이트 동일. 재실행 변경 0건.
- WorldSequences 저작 revision 8→9(CAS, 예고 트랙은 이전 시드값일 때만 변경), `Publish-MapAuthoring.ps1
  -Scope WorldSequences` Validate/Publish/Check 모두 exit 0. 게시본 변경은 모코모코 공격 템플릿뿐.
- 로딩: `Try_CollectPreparedAreaV1EffectTargets`가 시퀀스 참조를 모으므로 새 바닥 문서도 로딩 중 준비된다(코드 변경 없음).
- 클라이언트 진단 `maharaka.waterpang.waterfall` 문구를 무대 전체·파동 틱·원점(3.2m) 기준으로 갱신.

### 14.4 검증 (수치)
- 낙사 격자(`fallout_grid.py`, 서버 코드와 같은 식·30Hz): 무대 2,791칸 전부 발사 → 착지 없이 낙하 → 부활
  **2,791/2,791 (100%)**. jump1 1,392 / jump2 1,399. 파동 2.30초 203칸, 2.46초 2,588칸. 발사 거리
  5.90~9.11m, 비행 55틱, 끝점 반경 9.00~13.40m, 끝점이 통로·jump 박스 위인 경우 0(초안은 26칸 실패 →
  끝점 기준 회피로 수정).
- 연출 전수(`arena_sim.py`, 12절 런타임 합성식): 모코모코 22요소는 노즐 1.5m 이내 또는 무대 반경 안(3.9~6.3m),
  물기둥 밑면 22.45/22.45/22.65. 바닥 6배치 × 17요소 모두 무대 위 스폰(높이 ±0.8m). 예고 원판 무대 칸 100%,
  큰 물웅덩이 69.2%, 물보라 포함 95.7%. 넓은 물보라 2종(`_5`, `_6`)은 원본 한 번 공격(배치 0)과 똑같이 테두리
  밖으로 3.5~3.75m 번진다(원본 모양이라 실패로 세지 않음). 실패 0.
- `cl /Zs`: GameRoom_PartyWorld.cpp, GameRoom_PlayerSimulation.cpp, MaharakaWaterpangPresentation.cpp rc=0,
  C4819/C4828 외 경고·오류 0. `git diff --check` 통과. 새 JSON 3개 파싱 통과.
- 저장소 전체 `Validate-EffectSources.ps1`은 기존 쿠크 `blade-dance.circle.impact`(v15 carrier 없음)에서 먼저
  멈춘다(이번 변경과 무관, 워터팡 문서까지 도달하지 않음).

### 14.5 남은 것
- 원본 AI가 실제 경기에서 4225612/4225617 중 무엇을 쓰는지(서버 AI 표) 미확인. 전체 무대 판정·배치 6곳은 PROJECT.
- 더 이상 어떤 문서도 쓰지 않는 프로그램: 3654(behit 도넛), 4225601 전용이던 물 재질 프로그램들. 설치기에
  제거 기능이 없어 남겨 두었다(무해, 셰이더 코드만 남음).
- 사운드는 12절 그대로(Cast1/Shot1). 얼굴 재질 파라미터는 여전히 미연결.

### 14.6 사용자 확인 (Server + Client 재빌드, Server 재시작 필요)
경기 시작 120초·150초·340초 물벼락:
1. 0~2.3초 파란 원판 예고가 무대 전체에 깔리는지.
2. 2.3초 전후 모코모코 물기둥이 무대 위에 붙어 떨어지고, 무대 둘레 6곳에서 물웅덩이·물보라가 터지는지.
3. 무대 어디에 서 있든 맞으면 바깥으로 날아가 물에 떨어지고, 1.5초 뒤 가까운 jump 기둥에서 대기 자세로 서는지.
4. 날아가는 방향이 jump 통로·기둥 위로 떨어지지 않는지. 로그 `maharaka.waterpang.waterfall`의 `hit=deck(r<=7.5m,waves@69/74)`.

## 15. Debug F1 즉시 실행 버튼 (물벼락 / 워터캐논)

F1 → `Map Camera / Player`의 물총 `Fire 1`~`6` 줄 바로 아래에 `물벼락`, `워터캐논` 버튼과 결과 상태 문구를
추가했다. Debug 전용이며 빌드·실행은 하지 않았다.

### 동작 경로
- 버튼(`Client/Private/MainApp.cpp` 11156·11159): 기존 typed 명령 `C2S_DEBUG_WORLD_PLAYBACK`(PLAY_SEQUENCE,
  world MAHARAKA, 대상 `maharaka.waterpang.debug.waterfall` / `.cannon`)을 레벨의 command sink
  (`Level_Development.h` 51 `Get_DebugCommandSink`)로 보낸다. 응답 `S2C_DEBUG_WORLD_PLAYBACK_RESULT`를 받아
  버튼 옆에 `accepted` / `rejected (countdown/intro running)` / `disabled (Release Server)` 등을 표시한다.
- Server(`GameRoom_PartyWorld.cpp` 448–462): 마하라카에서 위 두 ID만 허용한다.
  `Start_MaharakaWaterpangDebugEvent`(1694~)는 `iStartTick = 현재 틱 + 6`인 `S2C_WORLD_SEQUENCE_PLAY`를 방 전원에게
  보내고 `m_MaharakaWaterpangDebugEvent`에 보관한다. Release 빌드는 `DISABLED`를 돌려준다(`#ifdef _DEBUG`).
  경기 카운트다운·도입 컷신 동안(예약 시작 + 첫 예약 이벤트 20초 전)은 도입 배우·카메라와 겹치므로 거절한다.
- 판정: `Update_MaharakaWaterpangHazards`는 `Sample_MaharakaWaterpangNow`(1664)를 쓴다. 강제 이벤트가 진행 중이면
  그것을, 아니면 경기 일정을 샘플한다. 경기가 없어도 강제 이벤트 판정은 돈다. 넉백 이탈·낙사 허용
  (`GameRoom_PlayerSimulation.cpp` 1331)도 강제 이벤트 + 2초 꼬리 동안 켜진다.
- 계약(`MaharakaWaterpangContract.h`): 샘플 완성부를 `Complete_MaharakaWaterpangSample`로 분리해 일정과 강제 이벤트가
  같은 식을 쓴다. `Sample_MaharakaWaterpangDebugEvent`는 종류별 이벤트(캐논 = 일반 회전 15초 발사)를 시작 틱 기준으로
  샘플하고, occurrence = `0x80000000 | 시작 틱`, 시계/반시계는 시작 틱 해시로 Server·Client가 같게 정한다.
- Client(`MaharakaWaterpangPresentation.cpp`): `Accept`(388)가 강제 ID를 받으면 `m_Forced`로 저장하고,
  `Sample_Now`가 강제 이벤트를 먼저 샘플한다(알림·예고·공격 인스턴스·jet 회전·사운드 모두 기존 `Update_Attacks` 경로).
  경기가 없으면 도입 인스턴스를 끝 자세(HOLD)로 세워 캐스트를 경기 위치에 두고(카운트다운·도입 카메라 없음), 이벤트가
  끝나면 `End_ForcedOnly`(438)로 원래 배치·NPC로 되돌린다. 준비 실패 5초는 강제 이벤트만 버리고 경기 연출은 막지 않는다.
- 다시 누르기: 새 occurrence가 되어 `Show_Actor`가 같은 인스턴스라도 멈췄다가 새로 재생한다(이전 이펙트·사운드 정리).
- 경기 예약이 오면 Server(650)와 Client(`Accept`) 모두 강제 이벤트를 끝낸다. 방이 비면 초기화(`GameRoom_WorldEntities.cpp`
  365). 늦게 들어온 플레이어는 입장 초기 패킷으로 진행 중 강제 이벤트를 같은 시계로 받는다(`GameRoom_Admission.cpp` 314).
- 물총 무장(`isWaterpangArmed`)은 경기 기준 그대로이며 강제 이벤트로 바뀌지 않는다.

### 프로토콜
새 메시지·필드 없음. 기존 `C2S_DEBUG_WORLD_PLAYBACK`, `S2C_DEBUG_WORLD_PLAYBACK_RESULT`, `S2C_WORLD_SEQUENCE_PLAY`를
그대로 쓴다(ID 형식·월드 검사 통과 확인). 버전은 물총 작업의 118 그대로.

### 검증
- `cl /Zs`: Server Debug 4파일·Release 1파일, Client 3파일(MainApp 포함) 오류 0, C4828 외 경고 0.
- 계약 헤더를 직접 컴파일한 동등성 검사(`tmp/waterpang_debug_zs/forced_equiv.cpp`): 강제 캐논(525틱)·물벼락(180틱)이
  예약 행 0·2와 모든 틱에서 종류·경과·길이·발사 구간(450틱)·회전량·물벼락 두 파동 틱(69/74)이 같다. 시작 전 비활성,
  누를 때마다 다른 occurrence, 같은 시작 틱은 같은 방향(200회 cw 97 / ccw 103). ALL PASS.
- 파일별 인코딩·줄끝 유지, `git diff --check` 통과.

### 사용자 확인
Server와 Client를 다시 빌드·재시작 → 마하라카 → F1 → `Map Camera / Player` → `물벼락` 또는 `워터캐논`.
경기 없이도 알림, 예고 장판, 공격 모션, 물줄기 회전, 사운드, 밀림·낙사가 바로 나와야 한다. 카운트다운·도입 중에는
`rejected (countdown/intro running)`가 정상이다. 진행 중 다시 누르면 처음부터 다시 시작해야 한다.
