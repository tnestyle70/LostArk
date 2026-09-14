# 2026-09-14 쿠크 1관문 `Stage1_wall` 원본 설치 데이터 복원 — RESULT

사용자 요청: 원본 게임 데이터에서 광장 줄무늬 벽이 "어떻게 설치되어 어떻게 처리되는지"를 찾아 내 맵의
`Stage1_wall`(World Sequence `world.sequence.instance.2` → MAP placement 3)에 그대로 적용한다. 메쉬 추출이
아니라 배치·크기·움직임 데이터가 대상이다.

## W-R1. 원본 데이터 확정 (사실)

### 벽이 아닌 것

- Deploy 프롭 `EFDLProp_ITR_10175`(def 300009, actor 268435474, UE (4760, 8464, 26.1))는 벽이 아니다.
  `umodel -list "*ITR_10175*"`가 여는 패키지(`XFH2RIN92C707DZF0IHRECD0.upk`)에는 `particlesystem par_g_waiting_01`
  하나뿐이고 mesh/animset이 없다. `Prop_zone37081` 행에 `sys.battle_station.move_waiting_desc`가 붙어 있으며
  같은 ITR_10175 + ITR_00279 쌍이 관문마다 반복된다(actor 268435471/474/478/480/849). 집합 대기 마커다.
  앞선 세션에서 "원본 문"이라고 한 것은 오판이었다.
- DeployData 레코드의 `+0x0c..+0x14`는 int32 FRotator(pitch, yaw, roll)다(마커 268435474 yaw 8192 = 45°,
  레버 268435857 yaw 51200 = 281.25° ↔ 팀장 deploy placement 2의 yaw −79.8°). 클라 yaw ≈ UE yaw.

### 벽인 것 — SCENE03A 컷신 113(광장폭죽) matinee 7, group `b01`

원본 파일 `B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8T6.upk`(SCENE03A). Kismet `37081_113`은 트리거 unit 131
"서커스 광장_서커스 광장 입구 이동해 팝업연출 재생"(Condition_VolumeProp @ UE (3289.7, 7119.9))이 재생한다.

| 항목 | 원본 값 (UE) | 클라 변환 (`build_maptool_scene.convert_*`) |
|---|---|---|
| 앞판 `interpactor_206` (base=204, hard attach, relative yaw 180) | mesh `lv_lut_midnightc.mesh.lv_lut_midnightc_floor02a_sm`, material override `lv_lut_midnightc.mat.bg_rad_koukusaton_floor21c_mi_pcs` | 줄무늬(보라·청록·크림, 별) 면. 플레이어 쪽을 향함 |
| 뒤판 `interpactor_204` | mesh `lv_lut_midnightc.mesh.bg_rad_koukusaton_floor18_sm_hht`, override 없음 | 누런 종이 면 |
| 위치 | (4544, 8016, 48) cm | (45.44, 0.48, −80.16) |
| 회전 | pitch 0, yaw −45°, roll −90° | quat (0.653281482, −0.27059805, 0.27059805, 0.653281482) — SL01 정적 금틀(row `…:export:684`)과 동일 |
| 크기 | drawscale 0.4 × drawscale3d (1.2, 1, 1) | (0.48, 0.4, 0.4) |
| 이동 트랙 | postrack 고정, eulertrack roll −90 → +90, 0 → 3.0009 s, `cim_curveautoclamped`, 양끝 tangent 0 | 로컬 −X축 회전 a(u)=180·(3u²−2u³)°, u=t/3000 |
| 종료 | 트랙 끝에서 정지(hide 없음). 바닥선 아래로 매달린 상태 | 마지막 키 `visible=false`로 동일 결과 |

메쉬 실측(wmodel): floor18 로컬 X ±1612, Y −210..93, Z −2385..0 cm → 벽 너비 16.5 m, 높이 9.54 m, 원점이
아래 모서리. floor02a 로컬 Z 0..2100 → 높이 8.4 m. 피벗은 광장 바닥 높이 0.48 m의 아래 모서리이고,
회전 180°이므로 벽은 플레이어 반대편(집합 마커 쪽, +X/−Z)으로 넘어가 1.5 s에 평평해진 뒤 바닥 아래로 내려간다.
원본이 "뒤에 공간이 있는 것처럼" 보이는 이유가 이것이다. 사용자 버전은 90°에서 멈추고 메쉬(FLOOR02 31.9 m
슬래브)·피벗·크기가 달라 지면을 뚫었다. 회전축 방향(로컬 −X)은 사용자 키와 원본이 같았다.

## W-R2. 적용한 변경

1. `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapplacements`
   - placement 3(`editor:…:3`, `Stage1_wall` 대상): 에셋은 사용자의 줄무늬 `MAP_8E8B6551B293_LV_LUT_MIDNIGHTC_FLOOR02_SM_OVR_CE0955453885`
     유지, 위치·회전·크기를 원본 값으로 교체. (FLOOR02는 FLOOR02A의 Z-반전 쌍둥이라 같은 quat로 같은 월드 형상)
   - placement 419(`editor:…:419`) 신규: 원본 뒤판 `MAP_55069666EA53_BG_RAD_KOUKUSATON_FLOOR18_SM_HHT`(override 없음),
     같은 위치·회전·크기. 헤더 count 3368 → 3369.
2. `…/LV_LUT_MIDNIGHTC_ED.worldsequences.json` (revision 1764 → 1765)
   - template `sequence.LV_LUT_MIDNIGHTC_ED.1`(`Stage1_wall`): durationMs 2600 → 3000, LINEAR, 트랙 `object`와
     `backing` 각 61키(50 ms), 회전 키 (−sin(a/2), 0, 0, cos(a/2)), 마지막 키만 visible=false.
   - instance `world.sequence.instance.2`: 바인딩 `object → MAP 3`, `backing → MAP 419`.
   - 다른 항목은 바이트 그대로. 툴 형식 재직렬화가 기존 항목을 정확히 재현하는지 먼저 확인한 뒤 해당 범위만 치환했다
     (`tmp/splice_stage1_wall_world.py`). 전체 문서 writer는 MapTool 1764 저장이 추가한 `mapMaterialBindings.unlit`를
     몰라 사용하지 않았다.

적용하지 않은 것: 원본 앞판 재질 `floor21c_mi_pcs`(보라·청록)로 쿡된 FLOOR02/FLOOR02A 에셋이 catalog에 없다.
사용자 에셋의 `floor21b_mi_pcs`(빨강·파랑 줄무늬, 텍스처 `floor21a_d_pcs`)를 그대로 썼다. 원본과 같은 색을 원하면
재질 override 에셋 쿡이 별도로 필요하다. 컷신 113의 다른 그룹(폭죽 f01/f02, 커튼 c01, 바위 m01~, 먼지)은 대상 밖.

## W-R3. 검증

- 자동: placement 행 float32 `%.9g` 포맷, 헤더 count, 새 id 미충돌(`Allocate_EditorPlacementId`는 미사용 최소 id를
  고르므로 419와 충돌 없음), 키 61개/첫 키 0/마지막 키 = durationMs/quat 길이 1, 문서 재파싱 float32 동일성.
- publisher: 아래 W-R4 참조.
- 수동(사용자): Map Tool → World Sequence → Placed Instances → `Stage1_wall` 재생. 기대: 벽이 광장 바닥선을 축으로
  천천히 시작해 뒤로 넘어가 1.5초에 평평, 3초에 바닥 아래로 사라짐. 줄무늬 앞판 뒤에 누런 종이판이 함께 넘어감.
  주의: Map Tool을 이미 열어 둔 상태라면 이전 revision(1764)이 메모리에 있으므로 맵 재진입 후 확인하고, 그 전에
  툴에서 Save하면 이 변경을 덮어쓴다.

## W-R4. publish 결과 (2026-09-14 12:42~12:52)

- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Publish`: exit 0, PlacementCount 3369, FileCount 8,
  runtime `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.mapplacements` sha256 `4527d345…c242f4` (12:48:09).
- 같은 명령 `-Mode Check`: exit 0, PlacementCount 3369, FileCount 8.
- 런타임 확인: `mapplacements`에 행 3(`FLOOR02_SM_OVR_CE0955453885`, 45.44/0.48/−80.16, quat 0.65328151/−0.270598054/0.270598054/0.65328151,
  scale 0.48/0.4/0.4)과 행 419(`FLOOR18_SM_HHT`, 같은 transform) 존재. `worldsequences.json` revision 1765,
  `sequence.LV_LUT_MIDNIGHTC_ED.1` durationMs 3000, 트랙 object/backing 각 61키, instance.2 바인딩 `object→3`, `backing→419`.
- 사용자 화면 확인은 아직 없다(W-R3 수동 항목). Git: 두 authoring 문서와 LFS runtime 출력은 아직 커밋하지 않았다.

## W-R5. 연결 시퀀스 `circus_finale` 동기화와 마리오 마커 대상 정정 (2026-09-14 13:30)

사용자 관찰: Map Tool에서 `Stage1_wall`은 앞판·뒤판이 함께 넘어가지만, `circus_finale`로 재생하면 앞판만 내려가고
누런 뒤판(placement 419)이 서 있는 채로 남는다.

- 원인(사실): placement 3을 바인딩하는 시퀀스는 `world.sequence.instance.2`(`Stage1_wall`)와
  `world.sequence.instance.circusfinale`(`sequence.LV_LUT_MIDNIGHTC_ED.circus_finale`, 21010 ms) 두 개다. `circus_finale`은
  419를 바인딩하지 않았고, `obj01`(→3) 키는 이전 저작값(0→6050 ms에 90°, 7150 ms에 숨김)이었다.
- 변경: `obj01` 키를 `Stage1_wall`과 같은 0→3000 ms 180° hermite 낙하 61키 + 21010 ms 숨김 키(총 62)로 교체하고,
  같은 키의 `obj24` 트랙을 추가해 `MAP_PLACEMENT 419`에 바인딩했다. 나머지 22개 트랙과 전체 길이 21010 ms는 그대로다.
  worldsequences revision 1765 → 1766. `circus_finale`을 재생하는 Gameplay 트리거 `1Stage_Final`, Composition
  `kakulsaydon.g1.world.14`, `KoukuSaydonArena.sequencer.json`, camerashots는 인스턴스 ID만 참조하므로 수정하지 않았다.
  Server World publisher도 인스턴스 ID만 검사하므로 재게시하지 않았다.
- 게시: `Publish-MapAuthoring.ps1 -Scope WorldSequences -Mode Publish` exit 0, runtime sha256 `ebd27f4f…a56d92`, runtime에서
  obj01 = obj24, 앞 61키 = `Stage1_wall` 키 일치 확인.

마리오 입장 마커(`CLevel_KakulSaydonArena::Load_EntranceTriggerMarkers`) 대상 정정: 사용자 정정 요청에 따라 맵 애니메이션을
재생하는 `playSequence` 트리거 17개를 빼고, 플레이어를 옮기는 `movePlayer` 트리거 18개만 남겼다. 팀장의 원래 5개
(`jump.1~3` movePlayer, `paper.1~2` playSequence)는 그대로다. 배열 40 → 23. 파일 UTF-8 BOM·CRLF 보존, `git diff --check` 통과.
C++ 컴파일은 사용자 VS 빌드로 확인한다(미실행).

## W-R6. PR #384에서 누락된 `피날레_맵` 움직임만 재반영 (2026-09-14)

사용자는 화면의 책 근처에서 재생하는 `피날레_맵` row 하나를 지정했고, 최종 확인에서 벽이 아래로 넘어가도록 수정한 움직임을 받으라는 뜻으로 확정했다. Sequence Composition의 `kakulsaydon.g1.world.14` → `world.sequence.instance.circusfinale`가 대상이다. 별도 Boss Composition에서 같은 worldId는 쿠크 트럼펫이므로 수정하지 않았다.

누락 원인은 PR #384 충돌 해결에서 완성 Sequence·Effect 튜닝 보존을 World 문서 전체에도 적용한 것이다. main `904303a9`의 MAP placement 3·419는 수정 PR과 이미 같지만, `circus_finale`에는 이전 90도 앞판 키만 있고 뒤판 연결은 없었다. Composition과 instance의 위치 offset 자체는 양쪽 모두 `[0,0,0]`이다. 첨부 이미지에서 보이는 거대한 책의 원인까지 이 비교로 확정하지는 않는다.

- 원본 수정 `fb9a8bb99e7b2c0ed8ed4aaed90aad5098bed4c2`의 W-R5 변경만 선택했다. `circus_finale.obj01`의 기존 6키를 62키로 교체하고 같은 키의 `obj24`와 `obj24 → MAP_PLACEMENT 419` 바인딩을 추가했다. 0~3000ms에 180도 회전하고 3000ms부터 숨긴다. 전체 template 길이는 21010ms 그대로다.
- 다른 22개 트랙·기존 23개 바인딩, resource 448개·다른 template 250개·다른 instance 306개의 원문 바이트가 모두 유지됐다. revision만 1847 → 1848로 증가했다. 별도 `Stage1_wall`, MAP 배치, 카메라, 두 Composition 및 이펙트·UV 파일은 편집하지 않았다. P4의 사용자 저작 길이 19959ms도 그대로다.
- 설치 직전 원문 일치를 확인했다. JSON 전체가 기대한 선택 변경과 동일하고 donor 키·바인딩도 동일하다. 키 시간은 0~3000ms 50ms 간격과 21010ms, quaternion 길이 검증도 통과했다. authoring 13,238,360 bytes로 16MiB 한도 이내이며 SHA-256은 `0a6b08b99852d5db30094a30e83a464b82da27deb61af2b64348684e2a7a1975`다.
- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences` Validate → Publish → Check가 모두 exit 0으로 끝났고 해당 runtime 파일 1개를 게시했다. runtime SHA-256은 `ee2d913eff3fe194f3de968841caf96edf478e4fe6d0359724bbd58cbb55c991`이다. authoring/runtime JSON과 줄바꿈을 정규화한 바이트가 동일하고 `git diff --check`도 통과했다. 선택 반영 helper·백업·receipt와 게시 로그는 `out/FinaleMapRestore20260914/`에 있다.

C++·셰이더·Resources 변경이 없어 재컴파일하지 않았다. 실행 중 Client PID 3948과 Server PID 46360은 유지했다. 별도 Gate1 Composition은 사용자 저장이 계속 들어오고 있어 커밋에서 제외하며, 기존의 Composition 생성물 3개도 그대로 남긴다. Client/UI를 실행·조작하거나 화면을 캡처하지 않았다.

사용자 확인: 미저장 맵·시퀀스 편집을 먼저 보존한 상태에서 F1 → Map Tool → World Sequence → Reload 후 `circus_finale` 또는 기존 Sequence의 `피날레_맵`을 재생한다. Reload는 미저장 맵·시퀀스 편집을 버리므로 편집 중이면 먼저 보존해야 한다. 기대 움직임은 앞판·뒤판이 함께 1.5초에 평평해지고 3초에 아래로 사라지는 것이다. 실제 화면과 책 잔존 여부는 아직 사용자 판정 전이다.
