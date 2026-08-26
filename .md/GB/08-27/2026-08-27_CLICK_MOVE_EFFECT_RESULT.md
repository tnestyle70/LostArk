# 2026-08-27 우클릭 이동 클릭 이펙트 RESULT

## 0. 요청과 범위

사용자 요청: 우클릭 이동 시 화살표가 모여 땅으로 사라지는 실제 로스트아크 클릭 이펙트 재현.
전투와 무관한 클라이언트 전용 커서 이펙트이며, `Data/Effects` Product Effect Catalog(스킬/보스
전용, GPU prepared-cache/prewarm/budget 연동)에 억지로 넣지 않고 `CSkillGroundTargetPreview`와
같은 성격의 독립 경량 GameObject로 구현하기로 사용자와 합의했다.

## 1. 실측 결과

- 실제 자산 소재: `ark.ui.cursorEffect.CursorEffectFrame` (`cursoreffect.gfx`, 패키지
  `OVSG0AAS7EM7OVVOALDR2Y.upk`). `CursorEffectFrame.handleClickStage()`가 우클릭 시
  `mouseX`/`mouseY`에 pooled MovieClip을 배치하고 `gotoAndPlay(2)`로 재생한다.
- 이 패키지의 실제 클릭 이펙트 3종을 전부 실측 확인했다. 사용자가 기억하는 "정삼각형 꼭짓점의
  화살표 3개가 모여 사라지는" 모양은 이 중 어디에도 없다.
  - `CursorEffect_Default_0` (symbol8, 12프레임): 실제 콘텐츠 있음 — **링 확대 + ADD 가산발광**
    펄스. depth3(characterId4, 링)이 scale 1.0→0.6→0.7→0.8→0.9→1.0으로 튀었다 커지고,
    depth5(characterId7, blendMode=ADD, 글로우)가 scale 0.2→1.0으로 같이 커진 뒤 둘 다
    별도 알파 곡선으로 페이드아웃한다.
  - `CursorEffect_Default_1` (symbol1, 25프레임): 전부 빈 `ShowFrameTag`만 있는 플레이스홀더.
  - `cursorEffect_00` (symbol4): 1프레임 정적 심볼 — Default_0의 링 코어 도형 그 자체.
  - SWF `frameRate="40.0"` 확인 — 12프레임 = 0.3초 실제 재생 시간.
- 실제 색상(비트맵 fill)은 복구하지 못했다. `cursoreffect_i5.tga`(DefineExternalImage2,
  256x128 아틀라스)는 Lost Ark 자체 크런치(crunch) 압축이며, 자체 컴파일한 `crn_decomp.h`
  디코더와 독립 `texture2ddecoder` 패키지 양쪽 모두 스크램블 노이즈만 산출했다. 별개의
  두 번째 텍스처(`voyagedestination` 커서 아이콘, 완전히 다른 asset)에서도 동일하게
  실패해 재현성 있는 실패로 확인했다 — 프로젝트 엔진 자체 crunch 우회 API도 없음
  (`EFEngine.dll` export 스캔 결과 crunch 관련 exported 함수 없음).
- 결론: **실제 모션/타이밍(스케일 곡선, 알파 페이드 곡선, 가산발광 레이어)은 실측 그대로
  사용**, 실제 픽셀 색상만 플레이스홀더(그레이스케일 마스크 + 연두색 tint)로 대체했다.

## 2. 구현

Product Effect Catalog을 쓰지 않고 `CSkillGroundTargetPreview`와 동일한 패턴(독립
`CGameObject`, `CVIBuffer_Rect` 쿼드, `CGameInstance::Add_RenderObject(RENDERGROUP::BLEND)`)
을 그대로 재사용했다.

- `Client/Public/ClickMoveEffect.h`, `Client/Private/ClickMoveEffect.cpp` (신규):
  `CClickMoveEffect`. 실측 키프레임 테이블(`g_Keyframes`, 10개 샘플, 1/40초 간격)을
  선형보간해 링/글로우 스케일·알파를 매 프레임 계산한다. `Play(worldPosition)`으로
  재생을 시작하고 `TOTAL_DURATION_SECONDS`(0.225초) 경과 시 자동 비활성화한다.
- `Client/Bin/ShaderFiles/Shader_VtxClickMoveGlow.hlsl` (신규): 글로우 레이어 전용, 기존
  `Shader_VtxSkillGroundTargetPreview.hlsl`와 동일 grayscale-coverage 셰이더에 blend state만
  기존 `BS_Additive`(SrcAlpha/One)로 교체. 링 레이어는 기존 `Shader_VtxSkillGroundTargetPreview.hlsl`
  프로토타입(`CSkillGroundTargetPreview::SHADER_TAG`)을 그대로 재사용한다(새 셰이더 불필요).
- `Client/Bin/Resources/Effect/UI/ClickMoveEffect/click_move_ring.dds`,
  `click_move_glow.dds` (신규, git-ignored): 그레이스케일 coverage 마스크 플레이스홀더.
  링은 실제 shapeBounds 비율(2080x2060 twips ≈ 104x103), 글로우는 실제 비율
  (760x740 twips ≈ 38x37, 링 대비 0.365x)에 맞춰 생성했다.
- `Client/Private/MainApp.cpp` (`Ready_Prototype_For_Static()`): `GLOW_SHADER_TAG` 셰이더와
  `CClickMoveEffect::PROTOTYPE_TAG` 프로토타입을 `CSkillGroundTargetPreview`와 같은 자리에 등록.
- `Client/Public/PlayerController.h`, `Client/Private/PlayerController.cpp`:
  `m_pClickMoveEffect` 멤버와 `Initialize_ClickMoveEffect(levelIndex)`를
  `m_pGroundTargetPreview`/`Initialize_TargetingPreview`와 동일한 형태로 추가.
  `Update()`의 `commandSink->Request_MoveGoal(...)` 성공 지점(우클릭 이동이 실제로
  서버에 제출된 그 프레임)에서 `m_pClickMoveEffect->Play(goal)`을 호출한다 — 서버 승인
  여부와 무관한 순수 클라이언트 커서 피드백이므로 스냅샷을 기다리지 않는다.
- `Client/Private/Level_Bern.cpp`, `Level_ValtanArena.cpp`, `Level_CharacterSelect.cpp`,
  `Level_Development.cpp`: `Initialize_TargetingPreview` 호출 바로 뒤에
  `Initialize_ClickMoveEffect(levelIndex)`를 추가해 4개 레벨 전부에 연결.
- `Client/Default/Client.vcxproj`, `Client.vcxproj.filters`: 신규 `ClickMoveEffect.h/.cpp`,
  `Shader_VtxClickMoveGlow.hlsl`을 `SkillGroundTargetPreview`와 같은 필터
  (`04. Network\00. PlayerController`, `97.ShaderFiles`)에 등록.

## 3. 검증 상태

- 자동 검증: 없음. 이 프로젝트에는 자동 테스트 스위트가 없고, 이번 변경은 Debug 빌드도
  아직 실행하지 않았다 (`No build/token waste` 방침 — 사용자가 직접 MSBuild 실행).
- 수동 검증: 없음. Client 실행/화면 판정은 사용자 전용 경계이므로 에이전트가 대신
  수행하지 않았다. 실제 확인은 Client 빌드 후 Bern/Valtan/CharacterSelect/Development
  아무 맵에서 우클릭 이동 시 링+글로우 마커가 뜨는지 사용자가 직접 확인해야 한다.
- `git diff --check`, JSON parse 등 텍스트 검증은 이 변경에 해당 사항 없음
  (JSON/schema 변경 없음).

## 4. 남은 항목 / 알려진 갭

- 실제 픽셀 색상은 여전히 미복구. Lost Ark의 crunch 비트스트림 변형을 디코딩할 수 있는
  도구를 찾으면 `click_move_ring.dds`/`click_move_glow.dds`를 실제 텍스처로 교체할 수 있다.
- 링/글로우 tint 색상(`RING_TINT`/`GLOW_TINT`, `ClickMoveEffect.cpp`)은 임의 선택(연두색)이며
  실제 게임 색상이 아니다. 사용자가 화면에서 보고 다른 색으로 조정 가능.
- `RING_BASE_DIAMETER = 6.0f`는 `PlayerSkillTargeting.json`의 `targetPreview.diameter` 관례를
  따른 추정치이며, 실제 화면에서 크기가 어색하면 조정이 필요하다.

## 5. 이 RESULT와 분리된 브랜치 처리

이 작업은 원래 `feature/valtan-death-screen-and-debug-kill` 브랜치 위에서 다른 미커밋
작업(죽음 화면)과 섞인 채로 진행됐다. 사용자 지적으로 `git worktree`를 이용해 클릭무브
관련 파일(신규 3개 + 기존 파일의 click-move 전용 hunk만)을 분리해 새 브랜치
`feature/click-move-effect`로 옮겼다. 원본 브랜치의 죽음 화면 미커밋 변경사항은
건드리지 않았다.
