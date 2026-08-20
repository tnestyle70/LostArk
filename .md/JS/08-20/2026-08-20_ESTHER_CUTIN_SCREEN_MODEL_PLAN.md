# 2026-08-20 에스더 컷인 화면 모델 연출 PLAN

작성자: JS · branch `feature/esther-cutin` (`91f34920` 위)
선행: `../08-15/2026-08-15_ESTHER_GAUGE_AND_SILIAN_SUMMON_RUNTIME_RESULT.md`

## 목표

발탄에서 Ctrl+Z로 소환된 실리안이 `esther.strike` 액션(베기 클립
`npc_att_battle_7_01`)에 들어가는 순간, 원작 에스더 컷인처럼 **화면 우하단 HUD 위에
같은 베기 애니메이션을 재생하는 3D 모델 연출**을 띄우고 클립이 끝나면 사라지게 한다.

## 실측 (2026-08-20)

- 트리거 지점: `ClientReplication.cpp` NPC action 처리(1102~1124행)가
  `NpcCatalog.json`의 `actionClips`(현재 `NPC_59030`의 `esther.strike` 하나)로 클립을
  전환한다. 이 스위치 성공 프레임이 컷인 시작 시점과 정확히 일치한다.
- HUD는 `CHUDRuntimeView`가 ImGui **foreground draw list**로 그리고, 실제 래스터라이즈는
  `CImGuiLayer::EndFrame()`에서 일어난다. 따라서 HUD 위에 모델을 그리려면
  `CMainApp::Render()`의 `EndFrame()` 이후, `RenderCombatHUDText()` 이전에 백버퍼로
  직접 렌더해야 한다.
- 백버퍼에는 `CGraphic_Device`가 메인 DSV를 상시 바인딩한다. 그 시점의 depth는 월드
  잔존값이므로 컷인 직전 `ClearDepthStencilView(D3D11_CLEAR_DEPTH)` 후 사용한다(이후
  프레임에는 depth 소비자가 없어 안전).
- `Shader_VtxAnimMeshBinary.hlsl`의 4개 패스는 전부 G-buffer(MRT5)/그림자 출력이라
  단일 RTV 백버퍼에 못 쓴다. 끝에 forward 패스(인덱스 4)를 추가한다. 조명은
  `Shader_VtxMeshPreview.hlsl`의 hemisphere+NdotL+rim 공식을 스킨드로 이식.
- ANIM `CModel`은 `LocalBounds`를 계산하지 않는다(`Ready_Meshes`가 NONANIM만 누적).
  카메라 프레이밍은 튜닝 상수로 두고 사용자가 육안 조정한다.
- `CModel` 클론은 bones/animations를 깊은 복사한다(`CModel(const CModel&)`) — 월드
  소환수와 컷인 모델이 같은 클립을 독립 커서로 동시 재생 가능.
- 모델/셰이더 프로토타입은 이미 준비돼 있다: Valtan 로더가
  `CNpcPresentationAssetService::Ensure_Prototypes(level, "NPC_59030")`를 선실행하고,
  `Prototype_Component_Shader_VtxAnimMeshBinary`는 레벨별 등록.

## 계약

| 계층 | 계약 |
|---|---|
| Data | 변경 없음 (`NpcCatalog.json actionClips`가 이미 클립 정본) |
| Shared/Server | 변경 없음 (스냅샷 `strActionId`가 이미 트리거 제공) |
| Shader | `Shader_VtxAnimMeshBinary.hlsl`에 `PS_MAIN_SCREEN_CUTIN` + `pass ScreenCutin`(인덱스 4, 단일 SV_TARGET0 forward). 기존 패스 0~3 인덱스 불변 |
| ViewModel | `CCombatHUDViewModel`에 `HUD_ESTHER_CUTIN_REQUEST{generation, archetypeId, clip}` + `Apply_EstherCutinAction`. `Reset_RuntimeState`에서 초기화. Replication→UI 기존 경계 재사용 |
| Replication | NPC actionClips 스위치 성공 시에만 `Apply_EstherCutinAction(archetype, clip)` 호출(1회/액션 변화, 매 프레임 아님) |
| Client 신규 | `CEstherCutinPresentationService`(정적 서비스, Release-safe, 상태는 cpp 내부 소유 — `CEffectPresentationService`/`CNpcPresentationAssetService` 패턴): `Render(device, context)` 한 진입점에서 generation 소비 → 현재 레벨에서 CModel/CShader 클론 → `Set_Animation(clip, loop=false)` → `ImGui::GetTime()` 기반 자체 dt로 `Play_Animation`(CHUDRuntimeView와 같은 render-driven 타이밍), 끝나면 해제. 우하단 서브 뷰포트(1280×720 기준 상수 스케일) + depth clear + `Begin(4)` + `Bind_DeferredMaterialInputs` 재사용. 레벨 전환 시 즉시 해제. 클론/클립 실패는 컷인만 조용히 생략(fail-closed, 월드 연출 비영향) |
| MainApp | 소유 멤버 없음. `Render()`의 ImGui 블록 종료 직후·`RenderCombatHUDText()` 전에 `CEstherCutinPresentationService::Render(m_pDevice, m_pContext)` 한 줄 |
| 등록 | `Client.vcxproj`/`.filters`에 `EstherCutinPresentationService.h/.cpp` 추가 |
| 검증 | Client x64 Debug 빌드. 육안(사용자): 발탄 게이지 충전 → Ctrl+Z → 소환수 베기 시작과 동시에 우하단 컷인 재생·종료 시 소멸, HUD/텍스트 z순서, 종료 후 뷰포트 복원(텍스트 위치 정상) |

## 결정

1. 컷인은 **모델만** 재생(배경 이펙트/슬라이드 인·아웃/사운드는 후속). 요청 범위 그대로.
2. 렌더투텍스처 대신 백버퍼 직접 서브 뷰포트 렌더 — 알파 discard로 모델 픽셀만 남아
   HUD 위 투명 합성과 동일한 결과, RT/합성 쿼드 없이 최소 구현.
3. 트리거는 catalog `actionClips`에 매핑된 액션 전부(현재는 esther.strike뿐) — 추후
   웨이/바훈투르 추가 시 데이터만으로 컷인이 함께 붙는다.
4. 카메라/배치 상수는 `EstherCutinPresentationService.cpp` 상단에 모아 두고 사용자 육안 튜닝으로 확정.
5. MainApp이 컷인 객체를 소유하지 않는다(사용자 결정 2026-08-20). 정적 프레젠테이션
   서비스로 두고 MainApp은 프레임 seam 호출만 담당. Update 메서드도 없음 —
   CHUDRuntimeView처럼 Render 내부 자체 타이밍.
