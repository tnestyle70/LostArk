# 2026-08-28 발탄 레이드 클리어 화면 RESULT

## 0. 요청과 범위

전 세션에서 `epicgatecommonclear_i6a.dds` 추출/배치를 하다 만 상태(`Data/UI/RaidClear/RaidClear_Layout.json`
untracked 초안, `UI/RaidClear/*.png` 미배치)를 이어서 완성해 달라는 요청. 사용자와 두 가지 범위를
확정했다.

- 텍스트는 초안대로 헤드라인(`RaidClear_TitleTextBox`) 하나만 반영한다 (부제/관문 설명/시간 텍스트는
  추가하지 않는다).
- 발탄 처치 감지 후 실제 화면 표시까지 이번 세션에서 구현한다.

## 1. 실측 결과

- 실제 소스: `EFUI_EPICGATECOMMONCLEAR` 패키지(`RYVJ3DRWJD5ZORDPBBPID4RZA.upk`)의
  `epicgatecommonclear.gfx`. 전 세션이 이미 umodel로 텍스처/`.gfx`까지는 뽑아 뒀고
  (`C:\Users\...\Desktop\EpicGateClear_Extracted\`), 이번 세션에서 `ffdec-cli -export script`로
  실제 AS3(302개 클래스)를 추가로 뽑아 실제 배치 데이터를 확인했다.
- 실제 드라이버 클래스: `ark.ui.epicGateCommonClear.EpicGateCommonClearFrame`. `changeDifficultyImageType`
  (101~106/301/302/401/501/502/601)별로 `result_<type>` 자식 MovieClip 하나만 보이고 나머지는 숨기는
  구조 — 원래 "군단장 던전(에픽 게이트)" 종류별 클리어 팝업 하나를 재사용하는 공용 문서다.
- 이번 프로젝트가 쓸 변형은 **`result_101`** (문자 358)로 확정했다. `SymbolClassTag`에서 이 캐릭터만
  유일하게 `epicGateCommanderClearSuccess_SetNN` 명명 규칙을 벗어난 `EpicGateCommonClearSuccessSet101`
  이름을 갖고 있어, 특정 군단장 전용이 아니라 패키지 자체 이름("COMMON" clear)에 대응하는 기본/범용
  변형이라고 판단했다. 사용자가 첨부한 실제 화면 스크린샷(주황 뿔 + 월계관 크레스트, "던전 클리어"
  흰 헤드라인)과도 일치한다.
- `result_101`(char 358) 내부 실제 named child 배치(twips, 1920x1080 authoring canvas 기준)를
  `ffdec -swf2xml` 덤프에서 직접 파싱해 확인했다:
  - `clearTF`(char354, `DefineEditTextTag`): `initialText="[$]commander.dungeon_clear"`,
    `fontClass=$YoonGasiIIM`, 흰색. placement `translate(12650,11564)` scale `1.3`, bounds
    `Xmin=-40,Xmax=19960,Ymin=-40,Ymax=1800` twips. 이 로케일 키가 실제 화면의 "던전 클리어" 헤드라인이다.
  - `title`(char122, desc), `gateClear`(char82, 관문 설명), `onceRewardDesc_lb`(char81, 보상 설명),
    `timeTF`(char79, 클리어 시간), `titleBG`(char78) 도 실좌표까지 확인했지만 사용자 결정에 따라
    이번 구현에는 반영하지 않는다 (필요해지면 이 RESULT의 실측 좌표를 그대로 쓰면 된다).
  - "typeSymbol"(AS3의 `resultMc.getChildByName("typeSymbol")`) 은 result_101 안에서 별도 이름으로
    찾지 못했다 — 이 변형은 배지 아이콘 전환 없이 처음부터 굳어진 아트로 보인다. 즉 Emblem의 정확한
    실좌표(트레이스 기반)는 확보하지 못했고, 전 세션이 이미 실제 화면 스크린샷을 보고 잡아 둔
    좌표(551.5,210,177,200 in 1280x720)를 그대로 유지했다 — 스크린샷과 대조해 합리적이었다.
  - char75(depth1, 배경 shape, 전체 309프레임 내내 고정 위치 (0,0))가 BgFlash에 대응한다.
- `RaidClear_TitleTextBox`만 clearTF의 실측값으로 갱신했다: twips→px(÷20)→1920x1080 canvas
  center(x≈1280, y≈635.4 native)를 계산했으나, 이 문서 자체가 화면 최상위 원점에서 별도 오프셋으로
  배치될 가능성(예: 좌측 보상 패널과 균형 맞추는 배치)이 있어 절대 x값은 신뢰하지 않았다. 대신
  **실측 상대값**(박스 높이 `(1800-(-40))/20*1.3*0.6667≈79.75px`, Emblem 하단(y=410)과의 실제 간격이
  전 세션이 이미 잡아 둔 y=430과 맞아떨어짐)만 반영해 `x:440→410, y:430(유지), width:400→460,
  height:50→80`로 보정했다 (화면 중앙 x=640 기준 재정렬, Emblem과 명확한 세로 간격 유지).
- 폰트: `Font_YoonGasiIIM`이 이미 프로젝트에 등록되어 있고(`RenderDeadSceneText`가 이미 사용 중),
  실제 clearTF의 `fontClass=$YoonGasiIIM`과 정확히 일치한다 — 새 폰트 자산 불필요.

## 2. 발탄 처치 감지 계약 (신규 Shared/Server 계약 없음)

Server가 이미 보스를 포함한 모든 `ServerWorldEntity`에 대해 `iCurrentHp==0`이 되면
`eAction=SERVER_ENTITY_ACTION::DEAD`로 세팅하고(`ServerCombatHitRuntime.cpp:103-188`, `BOSS` kind도
같은 분기를 통과), `GameRoom.cpp`의 snapshot 빌더가 이를 `WORLD_ENTITY_ACTION::DEAD`로 그대로
복제한다. `WORLD_BOOTSTRAP_KIND::BOSS`는 despawn 타이머(`iDeadDespawnMs`) 대상에서 제외돼 있어
(`GameRoom.cpp:10230-10243`), 발탄이 죽은 뒤에도 world entity가 사라지지 않고 `eAction==DEAD`가
계속 관측 가능하다. Client `CClientReplication`은 `WORLD_ENTITY_KIND::BOSS` 스냅샷마다
`VALTAN_PRESENTATION_STATE.eAction`을 채우고, `CCombatHUDViewModel::Apply_Boss`를 거쳐
`Get_Boss().eAction`으로 이미 노출돼 있었다. **따라서 새 Shared 필드나 Server 판정을 추가하지
않고, 이미 복제되는 이 값을 그대로 트리거로 재사용했다.**

## 3. 구현

Death Scene(`CLevel_ValtanArena::Update_DeadScene/Render_DeadScene` +
`CMainApp::RenderDeadSceneText`)과 완전히 동일한 아키텍처 패턴을 그대로 따랐다 — Level이 소유하는
`CHUDRuntimeView` + `HUD_xxx_TEXT_RECTS`를 통한 위치 전달 + `EndFrame()` 이후 실제 한글 텍스트
드로우 분리.

- `Data/UI/RaidClear/RaidClear_Layout.json`: `RaidClear_TitleTextBox`의 `rect`를 위 실측 보정값으로
  교체 (`x:410,y:430,width:460,height:80`). BgFlash/Emblem 슬롯은 유지.
- `Client/Bin/Resources/UI/RaidClear/RaidClearBgFlash.png`,
  `Client/Bin/Resources/UI/RaidClear/RaidClearEmblem.png` (신규): 전 세션이 이미 만들어 둔
  `EpicGateClear_Extracted/FINAL/dungeon_clear_bg_flash.png`/`dungeon_clear_emblem.png`를
  JSON이 참조하는 경로로 복사.
- `Client/Public/CombatHUDViewModel.h`: `HUD_RAIDCLEAR_TEXT_RECTS`(`isValid`/`fTitleX,Y,Width,Height`)
  구조체와 `Set_RaidClearTextRects`/`Get_RaidClearTextRects`를 `HUD_DEADSCENE_TEXT_RECTS`/
  `Set_DeadSceneTextRects` 바로 옆에 추가.
- `Client/Public/Level_ValtanArena.h`: `Update_RaidClear(f32_t)`/`Render_RaidClear()` 선언과
  `m_pRaidClearView`/`m_bRaidClearWasBossDead`/`m_fRaidClearElapsedSeconds` 멤버를
  `m_pDeadSceneView` 바로 옆에 추가.
- `Client/Private/Level_ValtanArena.cpp`:
  - 생성자에서 `m_pRaidClearView`를 `L"UI/RaidClear/RaidClear_Layout.json"`로 초기화.
  - `Update()`에서 `Update_DeadScene()` 바로 뒤에 `Update_RaidClear(fTimeDelta)` 호출.
  - `Render()`에서 `Render_DeadScene()` 바로 뒤에 `Render_RaidClear()` 호출.
  - `Update_RaidClear`: `CCombatHUDViewModel::Get().Get_Boss()`의 `isValid && eAction==DEAD`를
    edge-trigger로 감지해 `m_fRaidClearElapsedSeconds`를 0부터 카운트한다. 실제
    `EpicGateCommonClearFrame`의 `startFrame=90`/`holdFrame=296`(소스 40fps)을 그대로 초/단위로
    환산해 `REVEAL=2.25s`/`HOLD=7.4s`(총 9.65s) 동안만 보이고, 그 뒤에는 즉시(페이드아웃 없이) 숨긴다
    — 실제 AS3의 `onUpdateFrameCheck`도 마지막 프레임에서 그냥 `visible=false`로 끊는다. `REVEAL`
    구간에서는 `Set_SlotAlpha`로 BgFlash/Emblem을 0→1 페이드인만 적용했다 (프레임 단위 reveal
    애니메이션 309프레임 재현은 범위 밖). `RaidClear_TitleTextBox`는 항상 `Set_SlotVisible(false)`로
    숨기고(마커 전용) 실좌표만 `HUD_RAIDCLEAR_TEXT_RECTS`로 내보낸다.
  - `Render_RaidClear`: `m_pRaidClearView->Render("Default", 0)` 호출.
- `Client/Public/MainApp.h`, `Client/Private/MainApp.cpp`: `RenderRaidClearText()` 추가
  (`RenderDeadSceneText()` 바로 옆). `CGameInstance::Draw_Text(TEXT("Font_YoonGasiIIM"), L"던전 클리어", ...)`
  로 실제 로케일 문자열을 그린다. 호출은 `RenderDeadSceneText();` 바로 뒤 (`EndFrame()` 이후 텍스트
  드로우 목록)에 추가.
- `Client/Private/HUDLayoutTool.cpp`: `g_Documents[]`에 `{ "Raid Clear", "UI/RaidClear/RaidClear_Layout.json",
  "UI/RaidClear/", false }`를 `Dead Scene` 바로 뒤에 추가 — F1 → UI 탭에서 authoring/preview 가능.
- `Client/Default/Client.vcxproj`, `Client.vcxproj.filters`: `Data/UI/RaidClear/RaidClear_Layout.json`을
  `96.DataFiles\UI` 필터에 `None` 항목으로 등록 (기존 `BossUI.json` 항목 바로 옆).

## 3.1 Debug O키 재배정 (테스트 편의)

매번 실제로 발탄을 잡아 확인하기 번거롭다는 사용자 요청으로, 기존 Debug O키(로컬 플레이어를
즉시 죽여 사망 화면을 테스트하던 `Handle_DebugKillSelf` 트리거)를 레이드 클리어 오버레이
트리거로 교체했다. 사망 화면은 이미 완성돼 있어 두 번째 키를 새로 만들지 않고 대체했다.

- `Client/Public/Level_ValtanArena.h`, `Client/Private/Level_ValtanArena.cpp`:
  `Update_DebugKillSelfKey()`/`m_bDebugKillSelfKeyDown` → `Update_DebugRaidClearKey()`/
  `m_bDebugRaidClearKeyDown`로 rename. 함수 본문은 `m_PlayerController.Request_DebugKillSelf()`
  호출 대신 `m_fRaidClearElapsedSeconds = 0.f`를 직접 세팅해 `Update_RaidClear`의 표시 타임라인을
  강제로 시작시킨다 — 실제 Server 커맨드 왕복 없는 순수 Client 로컬 테스트 편의 기능.
- `Update_RaidClear`의 기존 "`!isBossDead`면 즉시 `elapsed=-1`로 리셋" 분기를 제거했다. 이 분기가
  남아 있으면 실제로 죽지 않은 발탄 상태에서 디버그 키로 강제 트리거한 다음 프레임에
  `isBossDead=false`가 다시 이 값을 지워버려 화면이 뜨지 않았다. edge-trigger(진짜 사망 감지)만
  남기고, 표시 종료는 순수하게 `elapsed >= RAIDCLEAR_TOTAL_SECONDS` 자연 만료로만 처리하도록
  단순화했다 — 실제 발탄 사망 트리거와 디버그 강제 트리거가 같은 코드 경로를 공유한다.
- `Request_DebugKillSelf`의 Shared/Server/PlayerCommandSink 계약 자체는 삭제하지 않았다 (더 이상
  어떤 키에서도 호출되지 않지만, 독립적인 Server 권위 디버그 커맨드이므로 이번 요청 범위(O키
  재배정)를 넘어서는 삭제는 하지 않았다).

## 3.2 Emblem 크기 보정 (사용자 실제 화면 대조)

사용자가 O키로 실제 화면에 띄워 보고 "던전 클리어" 텍스트는 실제 게임과 일치하지만 문양(Emblem)이
훨씬 작다고 지적했다. `RaidClear_Emblem`을 `{x:551.5,y:210,width:177,height:200}` →
`{x:516,y:130,width:248,height:280}`로 40% 확대했다 (원본 PNG 실제 비율 357:403≈0.886을 유지,
가로/세로 모두 중앙 640 정렬과 Title 위 20px 간격은 그대로 보존). 이 값도 실측 트레이스가 아니라
사용자의 실제 화면 대조에 의한 육안 보정이므로, 여전히 어긋나면 F1 → UI Layout Tool의
"Raid Clear" 탭에서 직접 드래그/리사이즈해 조정 가능하다 (이 문서가 이미 authoring 가능하도록
등록돼 있다).

## 3.3 실제 배치 데이터 재추적 (Emblem 실크기, 글로우 이펙트 복구)

사용자가 "추출할 때 배치데이터 다 가져온 거 아니야?"라고 재질문해서, `epicgatecommonclear.gfx`
`result_101`(char358)의 309프레임 전체를 프레임 단위로 다시 걸었다 (`ffdec -swf2xml` 전체 덤프를
`ElementTree`로 파싱, `ShowFrameTag` 경계마다 프레임 카운터를 증가시키며 모든 `PlaceObject2/3Tag`의
`characterId`/`className`/`matrix`/`colorTransform`/`blendMode`를 수집).

**실제로 확인된 것:**
- `clearTF`/`title`/`gateClear`/`onceRewardDesc_lb`/`timeTF`/`titleBG`(1절에서 이미 실측)는
  각각 독립된 named placement였지만, **Emblem은 별도 이름의 `typeSymbol` 배치가 없다**는 1절의
  결론이 틀렸다 — 이름이 없을 뿐 실제 shape가 존재했다. frame117에 `char357`(depth17, scale=1.5,
  일반 alpha blend, 가산 아님)가 새로 배치되는데, 이 sprite가 감싸는 `char356` shape의
  `shapeBounds`가 정확히 **7140×8060 twips = 357×403px** — 이미 크롭해 둔
  `RaidClearEmblem.png`(357×403)과 **정확히 일치**한다. bitmapId 체인을 끝까지 따라가면
  `bitmapId=355` → `DefineSubImage characterID=355`(`imageId=6`, region
  `(359,602)-(716,1005)`=357×403) → `DefineExternalImage2 imageID=6` → 실제 파일명
  **`epicGateCommonClear_I6A.tga`** — 사용자가 애초에 지목했던 그 `epicgatecommonclear_i6a.dds`
  파일이 실제로 Emblem의 정본 소스임이 배치 데이터로 직접 확인됐다.
  실제 최종 크기 = 357×403(native px) × sprite357의 실제 배치 scale(1.4999695) = 535.5×604.5
  (1920×1080 authoring canvas 기준) → 이 프로젝트의 1280×720 기준으로 ×0.66667 하면 **정확히
  357×403** (1.5 × 2/3 = 1.0으로 정확히 상쇄되기 때문에 원본 크롭 픽셀 그대로가 실제 크기다).
  기존 두 번의 육안 보정(200→280)은 둘 다 실제 크기(403)보다 한참 작았다.
- 밝기/글로우 부족 지적도 실제로 원인이 있었다. frame99~139 구간에 `className` 참조로만 배치되는
  (로컬 `characterId` 없음 — export한 `.gfx` 자체엔 이 텍스처가 없다는 뜻) 진짜 추가 레이어들이
  있었다: `circleCoreActive_shine`(원형 발광, depth4, scale 최대 7.0, 가산블렌드),
  `effect2_particleLoopingEffect`(반짝임 파티클, 홀드 구간 내내 루프), `smeltEffect_lineEffect`
  (거울대칭 광선 줄기 2개, 가산블렌드), `smeltEffect_particleLightingEffecct`(라이트닝 파티클,
  필터 있음), `effect2_avtive02`. 전부 `ImportAssets2Tag`로 참조되는 외부 공유 이펙트 패키지
  (`../EFUI_Effect/effect2.swf`, `../EFUI_Effect/effect.swf`) 소속이라 `epicgatecommonclear.gfx`
  단독 추출로는 안 보였다.
- `EFUI_Effect.upk`(실제 해시 `OVSG0AOVVOALD62YWMZZMW.upk`)를 다시 umodel로 열어
  `effect.gfx`/`effect2.gfx`를 export하고 ffdec로 재decompile해 위 5개 클래스의 실제 위치를
  확인했다. 이 중 **`circleCoreActive_shine`만 실제로 완전히 복구**했다: 실제 characterId는
  2033(96프레임 sprite)이며, 그 안에서 2프레임마다 새 캐릭터로 바뀌는 방식으로 **48장의 서로 다른
  100×100 실제 텍스처**(`coreEffectSet_shineLooping_0000`~`_0047`, 파일
  `Effect_I7F0.nopack.tga`~`Effect_I7C1.nopack.tga`)를 직접 순환 재생한다 — 셰이프에 크롭된
  아틀라스 조각이 아니라 각 프레임이 완전히 독립된 `DefineExternalImage2`라서, umodel
  `-export -dds`로 48개 texture2d를 그대로 뽑아 그레이스케일 십자성(별) 모양의 실제 발광 플립북
  임을 눈으로 확인했다. `effect2_particleLoopingEffect`/`smeltEffect_lineEffect`/
  `smeltEffect_particleLightingEffecct`/`effect2_avtive02`는 이번 세션에서는 추가로 추적하지 않았다
  (범위/시간상 `circleCoreActive_shine` 하나로 밝기/글로우 지적의 핵심을 우선 해결).
- `result_101`(sprite358) 안에서 emblem(char357, frame117)/circleCoreActive_shine(className,
  frame124)/clearTF(frame0, 상시)의 실제 배치 X좌표를 각각 독립적으로 twips→px→1280 스케일
  변환했더니 **셋 다 project 좌표계 x≈851~853에서 중심이 맞아떨어졌다** (emblem 851.6,
  clearTF 853.3, circleCoreActive_shine 853.35) — 이 셋이 서로 다른 프레임/다른 트레이스
  경로로 얻은 값인데도 우연이라기엔 너무 정확히 일치해서, `result_101` 자체의 로컬 좌표계 안에서는
  실제로 화면 중앙(x=640)이 아니라 x≈853(66.6% 지점)에 정렬되어 있다는 뜻이다. 다만 이 스프라이트
  트리를 감싸는 상위 UI 매니저가 `EpicGateCommonClearFrame`(char394) 자체를 화면 원점에 정확히
  어떤 오프셋으로 배치하는지는 이 `.gfx` 파일만으로는 알 수 없다 (`epicgatecommonclear.gfx`는
  독립 무비이며, 이를 실제 스테이지에 올리는 상위 UI 프레임워크 코드는 이 파일 밖에 있다). 사용자가
  실제 화면에서 대체로 중앙에 가깝게 보인다고 한 것과 절대 좌표가 안 맞을 수 있는 지점이라, **실제
  트레이스 값(폭/높이, 서로 간의 상대 위치)은 그대로 쓰되 수평 중심만 화면 중앙(x=640)으로 재정렬**
  했다 — 세 요소 모두 같은 x=640 중심을 공유하도록 재배치했다(상대적 Y 간격/겹침은 실측값 그대로).
- 최종 실측 기반 rect (1280×720, 전부 x=640 중심): `RaidClear_CoreShine`
  `{x:406.7,y:95.3,width:466.7,height:466.7}` (신규 슬롯, 48프레임 애니메이션, `additive:true`,
  `fps:20` — effect.gfx가 96 SWF프레임/40fps로 2프레임당 1장을 유지하므로 실제 고유 프레임 재생
  속도는 20fps), `RaidClear_Emblem` `{x:461.5,y:149.8,width:357,height:403}`,
  `RaidClear_TitleTextBox` `{x:206.7,y:383.7,width:866.7,height:79.7}`.

**`CHUDRuntimeView` 엔진 쪽 실제 버그 하나 발견/수정**: `Set_SlotAlpha()`가 `Slot.Layers.empty()`면
그냥 스킵하도록 되어 있어서, `RaidClear_CoreShine`처럼 `layers:[]`+`animation.frames`만 쓰는
플립북 슬롯에는 알파를 전혀 적용할 수 없었다 (반환값 `false`를 아무도 확인하지 않아 조용히
무시됐을 뿐). `Client/Public/HUDRuntimeView.h`에 `HUD_SLOT::fAnimationAlpha`(기본 1.f)를
추가하고, `Set_SlotAlpha()`가 `Layers[0].vTint[3]`와 별개로 이 값도 항상 세팅하도록,
`Render()`의 AnimationFrames 그리기 경로(`Client/Private/HUDRuntimeView.cpp`)가 하드코딩된
`IM_COL32(255,255,255,255)` 대신 `Slot.fAnimationAlpha`를 반영한 알파를 쓰도록 고쳤다. 기존
소비자(`DeadScene_Effect` 등)는 `Set_SlotAlpha`를 호출한 적이 없어 `fAnimationAlpha` 기본값
1.f 그대로 동작 — 하위 호환 깨짐 없음.

## 3.4 나머지 4개 이펙트 전부 실추출 (사용자 재요청: "다 가져오라니까")

3.3절에서 시간상 미룬 `smeltEffect_lineEffect`/`effect2_particleLoopingEffect`/
`smeltEffect_particleLightingEffecct`/`effect2_avtive02`를 사용자 요청으로 전부 마저 추출했다.
5개 전부 같은 패턴(className 참조, `EFUI_Effect.upk`의 `effect`/`effect2` 무비 소속, SWF
40fps에서 2프레임당 1장씩 유지하는 실제 프레임별 개별 텍스처 — 아틀라스 크롭이 아니라 각 프레임이
자기 자신의 `DefineExternalImage2`)이라 같은 방식(umodel `-export -dds`로 프레임 수만큼 개별
texture2d export → PIL로 PNG 변환 → 육안 확인)으로 처리했다.

- `smeltEffect_lineEffect` → 실제 char 1132~1200(69프레임, `Effect_I46C`~`Effect_I4B0`), 200×100,
  좌우로 뻗는 흰색 수평 광선 줄기(grayscale). `result_101`에 depth13(그대로)/depth14(scaleX 부호
  반전으로 좌우 미러) 두 벌 배치되므로, PNG 자체를 `PIL.Image.transpose(FLIP_LEFT_RIGHT)`로 미리
  뒤집은 사본을 만들어 `RaidClear_lineLeft`/`RaidClear_lineRight` 두 슬롯에 각각 연결했다
  (`CHUDRuntimeView`의 AnimationFrames 경로엔 런타임 flipX가 없어서, 엔진을 더 건드리지 않고
  에셋을 미리 구워 뒤집는 쪽을 선택했다).
- `effect2_particleLoopingEffect` → char 412~472(61프레임, `effect2_I19C`~`effect2_I1D8`),
  250×180, 반짝이는 작은 파티클(spark) 무리, 홀드 구간 내내 루프.
- `smeltEffect_particleLightingEffecct` → char 889~968(80프레임, `Effect_I379`~`Effect_I3C8`),
  400×300, **컬러(청록색) 번개 줄기** — grayscale이 아닌 실제 색상 텍스처로 확인된 유일한 레이어.
- `effect2_avtive02` → char 1283~1323(41프레임, `effect2_I503`~`effect2_I52B`), 200×200, 소용돌이
  치는 흰색 링(swirl ring, 중심에 밝은 코어) — 실제 exportName은 `hotimeActiveEffect`. 사용자가
  말한 "원형 글로우"에 `circleCoreActive_shine`(별 모양 발광)보다 오히려 더 부합하는 형태.

**실제 배치 데이터 기반 project-space rect** (전부 `result_101` 로컬 좌표를 twips→px→
×0.66667 변환 후, 1.1절의 공유 수평 중심 보정(`shift_project = 640 − 853.3 ≈ −213.3`)을 그대로
적용 — 5개 전부 clearTF/Emblem/CoreShine과 같은 좌표계에서 나온 값이라 같은 상수로 이동):

| 슬롯 | x | y | width | height |
|---|---|---|---|---|
| `RaidClear_avtive02` | 413.6 | 124.2 | 440 | 440 |
| `RaidClear_particleLooping` | 389.3 | 152.7 | 500 | 360 |
| `RaidClear_particleLighting` | 346.7 | 149.8 | 586.7 | 440 |
| `RaidClear_lineLeft` | 270.2 | 286.5 | 312 | 100 |
| `RaidClear_lineRight` | 703.1 | 286.5 | 312 | 100 |
| `RaidClear_Emblem`(재계산) | 459.8 | 149.8 | 357 | 403 |

`RaidClear_Layout.json`을 이 9개 슬롯(BgFlash, avtive02, CoreShine, particleLooping,
particleLighting, lineLeft, lineRight, Emblem, TitleTextBox) 전체로 다시 작성했다 — 슬롯 배열
순서는 `result_101`의 실제 SWF depth 순서(1<3<4<7<8<13<14<17<19)를 그대로 따라 뒤에서 앞으로
그려지게 했다. 신규 flipbook PNG 368장은 `Client/Bin/Resources/UI/RaidClear/{avtive02,
CoreShine,particleLooping,particleLighting,lineLeft,lineRight}/`에 저장했고, 전부 JSON의
`animation.frames` 경로와 대조해 366장 전부 실존을 확인했다 (누락 0). `Level_ValtanArena.cpp`의
`Update_RaidClear`는 이 8개 페이드 대상 슬롯(TitleTextBox 제외)을 배열로 묶어 반복문으로
`Set_SlotVisible`/`Set_SlotAlpha`를 적용하도록 정리했다 — 5개 슬롯 추가로 개별 호출을 나열하면
너무 길어져서, 반복되는 부분을 배열+for로 묶었다(로직 자체는 3.3절과 동일, 대상만 늘어남).

**리소스 git 정책 관련 정정**: 지난 턴에서 "ItemUpgrade가 V1 Effect 예외로 git/LFS 추적된다"고
말한 건 `CLAUDE.md` 본문의 서술을 그대로 옮긴 것이었는데, 사용자가 실제로는 그런 예외를 준 적이
없다고 확인해 줬다. 직접 `git ls-files Client/Bin/Resources/UI/ItemUpgrade/`로 대조해보니 실제로
추적되는 파일이 0개였다 — 즉 `CLAUDE.md`의 그 문장은 실제로 실행된 적 없는 문서 따로/실제 따로
상태였다. 이 프로젝트는 리소스를 코드 push 시 별도로 압축해 Drive로 공유하는 방식이라 이번 RaidClear
Resources(BgFlash/Emblem/CoreShine 등 총 370여 장)도 git에는 안 잡히는 게 정상이며 문제 없다.
`CLAUDE.md`/`AGENTS.md`는 팀 정본 문서라 임의로 고치지 않았다 — 실제 관행과 다르다는 사실만
여기 RESULT에 기록해 둔다.

## 3.5 전체 0.75배 축소 (사용자: "너무 크다")

O키로 확인한 사용자가 전체적으로 너무 크다고 해서, `RaidClear_BgFlash`(전체화면 배경 플래시라
축소 대상에서 제외)를 뺀 나머지 8개 슬롯(avtive02/CoreShine/particleLooping/particleLighting/
lineLeft/lineRight/Emblem/TitleTextBox)의 rect를 **Emblem 자기 자신의 중심점
`(640, 351.3)`을 공통 축소 기준점**으로 각 슬롯의 폭/높이는 ×0.75, 중심 위치는
`anchor + (center-anchor)*0.75`로 재계산해 한 번에 축소했다. 개별 슬롯을 각자의 중심으로
줄이지 않고 공통 기준점 하나로 줄인 이유는, 그래야 lineLeft/lineRight의 좌우 대칭이나
emblem-이펙트 간 겹침 비율 같은 서로 간의 상대적 배치가 그대로 유지되면서 구도 전체가 균일하게
작아지기 때문이다. TitleTextBox도 같은 비율로 줄어들어(높이 79.7→59.78) `RenderRaidClearText()`가
`rects.fTitleHeight`에 비례해 폰트 스케일을 잡으므로 텍스트 크기도 자동으로 같이 줄어든다.

## 3.6 클리어 사운드 연결

`epicgatecommonclear.gfx` 자체(AS3/`DefineSound`/`StartSoundTag` 전부)에는 사운드가 없다 —
스캘폼 UI 무비는 보통 내장 오디오 없이 native 쪽에서 별도로 소리를 재생한다. 대신 이미 이
프로젝트가 캐릭터 사운드 파이프라인(`.md/TJ/08-24/2026-08-24_CHARACTER_SOUND_EVENT_PIPELINE_RESULT.md`)
에서 써 온 원본 리소스 풀(`D:\로아 리소스\Sound\UI\System\`)에서 실제 후보를 찾았다:
`sys_content_unlock_epic_gate_commander1__33466782.wav`
(에픽게이트 커맨더 콘텐츠 해금음 — 이름은 일치하지만 "해금" 의미라 매번 뜨는 클리어와는 결이 다름)과
`sys_raid_success1__457395004.wav`(범용 레이드 성공음). 발탄 전용 사운드는 리소스 풀에 없었다
(카멘/카제로스/오르드의 "1차 처치" 전용음만 있고, 이건 월드퍼스트 전용이라 매치 안 됨). 사용자가
`sys_raid_success1__457395004.wav`를 선택해 `Client/Bin/Resources/Sound/UI/System/`에 배치했다
(원본 리소스 풀과 같은 상대 경로 유지).

`CLevel_ValtanArena`에 `Trigger_RaidClear()`를 신규 추가해 `m_fRaidClearElapsedSeconds = 0.f`
설정과 `CGameInstance::Get().Play_Sound()` 호출을 한 곳으로 묶었다 — 실제 발탄 사망 edge-trigger
(`Update_RaidClear`)와 디버그 O키(`Update_DebugRaidClearKey`) 둘 다 이 함수를 호출하도록 바꿔서,
두 경로가 서로 다른 사운드 재생 로직을 갖지 않게 했다 (전에는 O키가 `m_fRaidClearElapsedSeconds`를
직접 세팅해서 사운드가 안 나가는 상태였음). `CRuntimeAssetRoot::Resolve` + `CGameInstance::Play_Sound`
패턴은 `CMainApp::Play_UIButtonClickSound()`와 동일하게 맞췄다.

## 3.7 씬 전환 시 DeadScene/RaidClear 순간 노출 수정

사용자 보고: 로딩이 끝나고 발탄 맵으로 들어갈 때(또는 다른 레벨 전환 시) 사망 화면/레이드 클리어
화면 같은 오버레이가 한 프레임 정도 화면에 떴다가 사라지면서 전환된다.

원인: `CHUDRuntimeView`의 슬롯은 `Set_SlotVisible()`을 한 번도 호출하지 않으면 기본값이
**보임**(`bForceHidden=false`)이다. `Render_DeadScene()`/`Render_RaidClear()`는 자체 early-return
가드 없이 매 프레임 무조건 `m_pDeadSceneView->Render(...)`/`m_pRaidClearView->Render(...)`를
호출하고, 실제로 숨기는 일은 오직 `Update_DeadScene()`/`Update_RaidClear()`가 매 프레임
`Set_SlotVisible(false)`를 호출해야만 일어난다. 레벨이 막 activate된 프레임에는 엔진의 Update→
Render 순서상 이전 레벨의 `Update()`가 이미 끝난 뒤 `Change_Level`이 새 레벨을 activate하므로,
새로 만들어진 `CLevel_ValtanArena`는 자기 자신의 `Update()`가 한 번도 실행되지 않은 채 그 프레임의
`Render()`를 맞는다 — 그 한 프레임 동안 두 뷰의 모든 슬롯(RaidClear의 화면 전체를 덮는 BgFlash
포함)이 authored 기본값(보임)으로 그려진다.

(참고로 Bern의 `Render_ValtanEntryModal()`은 같은 `CHUDRuntimeView` 패턴이지만 자체
`if (!m_isValtanEntryModalOpen) return;` early-return 가드가 있어서 이 버그가 없다 — 그래서
이번 수정 범위는 DeadScene/RaidClear로 한정했다.)

수정: `CLevel_ValtanArena::Initialize()`에서 `m_pDeadSceneView`/`m_pRaidClearView`를
생성한 직후, 각 뷰가 소유한 모든 슬롯에 `Set_SlotVisible(false)`를 즉시 호출해 뷰가 로드되는
순간부터 숨김 상태로 시작하게 했다. `RaidClear_FADING_SLOTS` 배열을 `Update_RaidClear` 함수
내부의 지역 static에서 파일 스코프 anonymous namespace(파일 상단, `Report_InitFailure` 바로
아래)로 옮겨 `Initialize()`와 `Update_RaidClear()` 양쪽에서 재사용하도록 정리했다.

## 3.8 배경 어둡게 (사용자: "뒤에 화면이 좀 어두워져야 하지 않나?")

실제 AS3(`ARKFrame::setModalState`)를 확인해보니 `EpicGateCommonClearFrame`은 base class의
모달 알파 상수를 `MODAL_TRANSPARENT`(0.65, Dead Scene류가 쓰는 값)가 아니라
`MODAL_TRANSPARENT_DUNGEON = 0`으로 오버라이드한다 — 즉 **원본은 이 팝업 뒤를 실제로 어둡게
하지 않는다** (모달 rect 자체는 클릭 차단용으로 존재하지만 알파 0이라 안 보임). 다만 이 오버레이가
화면 전체를 덮는 밝은 가산 글로우/플래시 아트이고 클릭 차단이 필요한 다른 목적도 없어서, 실제
수치보다는 가독성을 우선해 `Render_DeadScene()`과 동일한 방식(`IM_COL32(0,0,0,160)` 전체화면
dim rect, `isShowing`일 때만)으로 어둡게 만들었다 — 원본과 다르다는 점은 주석에 남겨뒀다.

## 4. 검증 상태

- 자동 검증: 없음. 이번 변경은 Debug 빌드를 아직 실행하지 않았다
  (`No build/token waste` 방침 — 빌드는 사용자가 원할 때 직접/요청 시 수행).
- 수동 검증: 없음. Client 실행과 화면 판정은 사용자 전용 경계이므로 에이전트가 대신 수행하지
  않았다. 실제 확인은 Client 빌드 후 Valtan 진입 → 발탄 HP를 0으로 만든 뒤(Debug라면 Boss Tool의
  damage-event 경로나 실제 전투로) BgFlash+Emblem이 페이드인하고 "던전 클리어" 헤드라인이 뜨는지,
  약 9.65초 뒤 사라지는지 사용자가 직접 확인해야 한다.
- 새 Shared 필드가 없으므로 `NetworkProtocolHarness`/`Server.exe --contract-test`에 해당 변경 없음.
- `git diff --check`, JSON parse는 아직 실행하지 않았다.

## 5. 남은 항목 / 알려진 갭

- (3.3절로 대체) Emblem 실좌표/실크기는 이제 트레이스로 확보했다 (357×403, 화면 중앙 x=640
  기준 재배치). BgFlash(char75)의 실제 shapeBounds/원본 크롭 비율은 여전히 대조하지 않았다.
- (3.4절로 대체) `result_101`이 참조하는 5개 EFUI_Effect 글로우/파티클 클래스
  (`circleCoreActive_shine`/`effect2_particleLoopingEffect`/`smeltEffect_lineEffect`/
  `smeltEffect_particleLightingEffecct`/`effect2_avtive02`) 전부 실제 프레임 텍스처까지 복구
  완료.
- 페이드인만 구현했고 309프레임짜리 실제 reveal 애니메이션(각 레이어가 정확히 몇 프레임째 나타나고
  어떤 스케일 곡선으로 커지는지)은 재현하지 않았다 — 9개 슬롯 전부를 같은 타이밍으로 동시에
  페이드인시키는 것으로 근사했다. 실제로는 avtive02(frame139)가 나머지(frame117~124)보다 늦게
  등장하는 등 미세한 순서 차이가 있고, particleLighting은 frame124에 real filter(흐림 효과로
  추정)가 있는데 이건 재현하지 않았다(원본 텍스처 자체에 이미 발광이 어느 정도 baked돼 있어 큰
  차이는 없을 것으로 예상).
  `title`/`gateClear`/`onceRewardDesc_lb`/`timeTF`/`titleBG`의 실좌표는 위 1절에 남겨 뒀으니
  나중에 부제/시간 표시가 필요해지면 재사용 가능.
- `RAIDCLEAR_TOTAL_SECONDS`(9.65초) 동안 보스가 계속 `eAction==DEAD`로 남아 있어야 하는데, 이는
  Server가 BOSS를 despawn하지 않는다는 현재 동작(`GameRoom.cpp:10230`)에 의존한다 — 이 동작이
  바뀌면(예: 보스도 despawn하도록 바뀌면) `Get_Boss().isValid`가 먼저 꺼져서 화면이 예정보다 일찍
  사라질 수 있다.
