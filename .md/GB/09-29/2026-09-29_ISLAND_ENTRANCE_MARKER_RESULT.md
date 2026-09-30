# 2026-09-29 마하라카 입항 표식(닻 마커) + 입구 프롬프트 RESULT

빌드와 Client/Server 실행은 하지 않았다(리드가 마지막에 한 번 빌드). 화면 판정은 사용자 몫이다.

## 1. 원본 체인 (증명됨)

```text
EFTable_VoyageAnchorVolume.PrimaryKey (45행: 1032000~1032016, 1032051, 1032113~1032145)
  == EFTable_Prop.PrimaryKey
  -> 1032000~1032016  Model EFDLProp_ITR_10297.ITR_10297  (항구 입항 구역)
  -> 1032051, 1032113~1032145  Model EFDLProp_ITR_10118.ITR_10118  (이벤트 섬 구역)
  -> LookInfo/Prop/EFDLProp_ITR_10297.ITR_10297.loa (data4.lpk)
  -> CEFParticleData  ParticleSystem'FX_ITR_10297.Par_D_ITR_10297_DockingVolume'  bone B_Root
  -> 패키지 Packages/XFH2RIN92C70ERDF0IHRECD0.upk (UModel -list로 확인, 이미터 7개)
```

ITR_10118은 `FX_BS_03.mark.Par_D_SafeZone_01/04`(안전지대 표식)이라 항구 닻 마커가 아니다. 마하라카 항구용은 ITR_10297의 DockingVolume이다.
(스크린샷과의 1:1 대조는 하지 않았다. "항구 입항 구역 = ITR_10297" 은 테이블/LookInfo 근거이고, 금색 닻 모양이 이 시스템이라는 최종 판정은 화면 확인이 필요하다.)

## 2. 복원 산출물

- 빌더: `Tools/EffectPipeline/build_anchor_marker_source_effects.py` (배 물살 빌더와 같은 드라이버 재사용, LookInfo 블록을 실제 PlayParticleEffect 헤더 뒤에 붙이는 합성 액션, 마지막에 월드 루트로 재바인딩)
- 문서: `Data/Effects/Authored/effect.bern.anchor.marker.marker.full.restore.effect.json` (요소 7개) + `Data/Effects/EffectCatalog.json` 1행
- native 셰이더 프로그램 4969~4972 (기존 bucket 4928 파일 3개 + `Effect_ArtistMaterial_Tables.inl`만 변경, 새 파일·vcxproj 변경 없음)
- 리소스 2개 (신규): `Client/Bin/Resources/Effect/Bern/AnchorMarker/Textures/fx_tex_02/{fx_d_line_005,fx_d_symbol_035}.dds`
  CY_Resources(바탕 화면) 미러 + 전달 스크립트 `Copy_ResourceDistribution_2026-09-29_AnchorMarker.ps1`
  (`fx_m_wave_001.dds`는 쿠크 텍스처 재사용, 설치본 존재)

## 3. 요소 전수 검증 (7/7)

| # | 이미터 | 재질 | 프로그램 | 수명 | 크기(m) | 위치 | 방향 | 공간 |
|---|---|---|---|---|---|---|---|---|
| 1 | sprite_14 | dockingvolume_01_ad (가산) | 4969 | 10s 루프 | 25 x 0.25 | (0, 0, +12.5) | axislock Z | local |
| 2 | sprite_2 | dockingvolume_01_ad | 4969 | 10s | 25 x 0.25 | (-12.5, 0, 0) | axislock Z + rotation | local |
| 3 | sprite_0 | dockingvolume_01_ad | 4969 | 10s | 25 x 0.25 | (0, 0, -12.5) | axislock Z + rotation | local |
| 4 | sprite_1 | dockingvolume_01_ad | 4969 | 10s | 25 x 0.25 | (+12.5, 0, 0) | axislock Z + rotation | local |
| 5 | sprite_21 | ripple_01_ad + fx_d_line_005 | 4970 | 10s | 25 x 25 | (0, 0.05, 0) | axislock Z | local |
| 6 | sprite_22 | circ_01_02_ad | 4971 | 10s | 5 x 0.01 | (0, 0.05, 0) | axislock Z | local |
| 7 | sprite_23 | master_01_072_tr + fx_d_symbol_035 | 4972 | 5s | 1.36 x 0.01 | (0, 1.6, 0) 상승 속도 | 빌보드(축 고정 없음) | local |

- 25 m 정사각 테두리 네 변(1~4) + 바닥 파문(5) + 중앙 원(6) + 떠오르는 닻 심볼(7)로 구성된다.
- 7개 모두 visible true, authoringApproximate false, 지연된(deferred) 이미터 0, native 입력 제외 0, 소스 재질 실패 0.
- 부착: 시스템이 뼈(B_Root)가 아니라 프롭 원점에 있으므로 7개 전부 `follow:false`, 슬롯 `root`, 소켓 항등, 기준 yaw 0으로 재바인딩(`worldRootElements: 7`). 회전 보정 없음(프롭 원점은 항등).
- 확인하지 않은 것: 축 고정(epal_z) 이미터가 실제 화면에서 수평으로 놓이는지, 닻 심볼 모양. 화면 확인 필요.

## 4. 런타임 연결

- `Client/Private/Level_Bern.cpp` / `Client/Public/Level_Bern.h`
  - Initialize: `CInteractKeyPromptView` 생성(`pEntry->pMapAreaId`), 마커 target을 `Queue_ProductTargets_Priority`로 로딩 중 준비.
  - Update: 프롬프트 갱신(서버가 offer한 trigger id, 입장 연출 중엔 숨김) + `Update_AnchorMarker`.
  - Render: `Render_Text()`.
  - `Update_AnchorMarker`: 뷰어 문서에 dock 트리거가 있을 때만, 그 트리거 위치·yaw에 `Spawn_LevelPlacement`(Level 소유, owner-sustained 루프)로 마커를 띄운다. 준비가 안 됐으면 0.5초마다 최대 40회 재시도. 좌표를 코드에 넣지 않았으므로 서버가 제안하지 않는 곳에는 마커가 생기지 않는다.
  - 소멸: 소멸자 `Clear_AnchorMarker` (+ Level 소유 루트라 레벨 이탈 정리 경로도 적용).
- `Client/Public/InteractKeyPromptView.h` / `.cpp`: `Try_Get_DockPoint()` 조회 함수 1개 추가(표시 전용).
- `Tools/WorldPipeline/Publish-WorldGameplay.ps1`: 뷰어 문서 출력 대상에 `BERN` 추가. 게시 결과 `Client/Bin/DataFiles/World/LV_BER_BERNCASTLE.viewer.world.json`(신규).
- 진단: Bern 진입 시 `Client/Default/EffectFailure.user.log` 채널 `AnchorMarker.Bern`
  (`placed asset=... pos=(x, y, z) yaw=.. attempts=N` / `spawn waiting: ...` / `prepare isolated: ...` / `activation failed: ...`).
- 프롬프트 문구: `마하라카 썸머 캠프 [G]` (interactAction `dock:마하라카 썸머 캠프`, 이전 데이터의 "썰머 캐프" 오타 수정).
- 다른 레벨 영향 없음: 변경은 `CLevel_Bern`과 Bern 데이터/뷰어 출력에 한정. 마하라카·발탄·쿠크 코드 경로는 바뀌지 않았다.

## 5. 데이터 (fork A 좌표 FINAL_V2 반영)

- `island_dock.json`의 `status: FINAL_V2`를 확인한 뒤 진행했다. 이 파일에는 `dockTrigger` 키가 없고 `shipStopPoint (417.25, -480.0)`, 부두 끝 x=420.25, 섬 중심 (440, -480)가 있다.
  트리거는 이전 규약(배 정지 지점 중심, 바다 높이 10.95)과 같게 **중심 (417.25, 10.95, -480), half (7, 4, 7), yaw 0**으로 뒀다. 이것은 내가 정한 값이며 fork A가 dockTrigger로 확정한 값이 아니다(정지 지점에서 부두 끝까지 3 m라 트리거 안에서 배가 서고 부두 쪽으로도 4 m 여유).
- `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json` revision 924 -> 925, `island.dock.to.maharaka` 추가(interact-gated, changeLevel `MAHARAKA`, 착지 배치 미지정 = 마하라카 기본 스폰). CRLF/배열 서식 유지, 바이트 단위 삽입, 읽은 뒤 변경 감지 검사 통과.
  백업: 작업 tmp `entrance_backup\Gameplay.world.json.before`.
- 게시: `Publish-WorldGameplay.ps1 -WorldId BERN` Validate/Publish 성공(68 placements). `BERN.worldbootstrap` diff는 헤더(922 -> 925, 67 -> 68)와 새 행 1개뿐:
  `island.dock.to.maharaka triggerBox ... 417.25 10.95 -480 ... changeLevel 1 MAHARAKA`.
  (헤더의 922는 커밋본 기준이며 924 게시 이력은 이전 작업)

## 6. 코드 경로 추적 (실행은 하지 않음)

- 배 탑승 중 G: `CPlayerController::Update` -> `Submit_InteractIfOffered`(탑승 여부로 막지 않음; Esther 슬롯만 `isMounted`로 막음) -> `Request_InteractTrigger`. 서버 `Activate_Interact`는 탑승을 보지 않고, 배 정지 지점이 트리거 안이므로 changeLevel(MAHARAKA)이 발화한다(이전 결과 문서 fork 2, `ServerTriggerSystem.cpp`).
- 프롬프트: 서버가 boxed trigger를 offer -> `CCombatHUDViewModel::Get_InteractPromptTriggerId()` -> `CInteractKeyPromptView`가 뷰어 문서에서 같은 placement id를 찾아 `마하라카 썸머 캠프 [G]` 표시, 벗어나면 숨김.
- 마커: dock 트리거가 뷰어 문서에 있고 target 준비가 끝난 프레임에 Spawn -> Commit -> Update_WorldRoot, 이후 유지. 레벨 이탈/소멸자에서 Stop.

## 7. 게임에서 확인할 순서

1. Server + Client 재빌드 후 베른 입장, 로그에서 `AnchorMarker.Bern placed ... pos=(417.25, 10.95, -480.00)` 확인.
2. 배를 타고 (417, -480) 부근으로 이동: 바다 위에 25 m 사각 금색 테두리, 바닥 파문, 중앙에서 떠오르는 닻 심볼이 보여야 한다.
3. 트리거 안에서 `마하라카 썸머 캠프 [G]`가 머리 위에 뜨는지, 벗어나면 사라지는지.
4. G -> 마하라카로 이동(로딩 후). 마하라카에서 `베른으로 돌아가기` G로 복귀.
5. 정지 지점에서 프롬프트/마커가 부두와 겹쳐 어색하면 트리거 위치(shipStopPoint 기준)만 조정.

## 8. 남은 것 / 불확실

- 컴파일하지 않았다. `Level_Bern.cpp`, `InteractKeyPromptView.cpp` 변경은 코드 리뷰(git diff, 파일 인코딩 CRLF/UTF-8 유지)만 했다. 빌드에서 include/시그니처 오류가 나면 이 두 파일부터 본다.
- 금색 닻 모양 최종 판정, 축 고정 이미터의 실제 바닥 눕힘, 마커 크기(25 m)와 작은 섬 배치의 조화는 화면 확인 필요.
- 마커 위치는 dock 트리거를 따른다. 섬/정지 지점이 다시 움직이면 트리거만 옮기면 마커도 따라간다.
- 대사 이름은 문자열 규칙(`dock:<이름>`)이며 32자 제한이 있다.
- `Validate-EffectSources.ps1` 전체 실행은 기존의 무관한 쿠크 문서에서 먼저 실패하므로 하지 않았다. 새 문서는 빌더의 material/attachment 검증과 JSON parse만 통과.
