# 2026-08-20 에스더 컷인 화면 모델 연출 RESULT

작성자: JS · branch `feature/esther-cutin`
PLAN: `2026-08-20_ESTHER_CUTIN_SCREEN_MODEL_PLAN.md`

## 1. 구현 완료

PLAN 계약대로 Client 전용으로 구현했다. Data/Shared/Server 변경 없음.

- `Shader_VtxAnimMeshBinary.hlsl`: `PS_MAIN_SCREEN_CUTIN` + `pass ScreenCutin`(인덱스 4,
  단일 SV_TARGET0 forward, hemisphere+NdotL+rim+emissive, `g_CutinLightDirection` 기본값).
  기존 패스 0~3 인덱스 불변.
- `CCombatHUDViewModel`: `HUD_ESTHER_CUTIN_REQUEST{iGeneration, strArchetypeId,
  strClipName}` + `Apply_EstherCutinAction`/`Get_EstherCutinRequest`,
  `Reset_RuntimeState`에서 초기화.
- `ClientReplication.cpp`: NPC actionClips 클립 전환 성공 시(액션 변화당 1회)
  `Apply_EstherCutinAction(archetype, clip)` 게시. 현재 매핑은 `NPC_59030`의
  `esther.strike → npc_att_battle_7_01` 하나.
- 신규 `CEstherCutinPresentationService`(Public/Private, vcxproj/filters
  `02.GameObjects\04. UI` 등록): 정적 서비스, 상태는 cpp 익명 namespace 소유.
  `Render(device, context)` 단일 진입점에서 generation 소비 → 현재 레벨의
  `Prototype_Component_Shader_VtxAnimMeshBinary`와 NPC 모델 프로토타입 클론(클론은
  본·애니메이션 깊은 복사라 월드 소환수와 독립 재생) → `ImGui::GetTime()` 자체 dt로
  `Play_Animation`, 클립 종료·레벨 전환·드로우 실패 시 해제. 우하단 서브 뷰포트
  (1280×720 기준 X730 Y220 W520 H460 스케일) + 바인딩된 DSV depth clear + `Begin(4)`.
  클론/클립 실패는 컷인만 생략하고 월드 연출은 비영향.
- `CMainApp`: 소유 멤버 없음. `Render()`에서 `m_pImGuiLayer->EndFrame()` 직후·
  `RenderCombatHUDText()` 전에 `CEstherCutinPresentationService::Render(m_pDevice,
  m_pContext)` 한 줄 — HUD가 ImGui foreground라 EndFrame 이후여야 HUD 위에 그려진다.

사용자 구조 결정 반영: MainApp이 컷인 객체를 소유하는 초안(멤버 뷰 + Update 직접
호출)을 폐기하고 정적 프레젠테이션 서비스 + Render 내부 자체 타이밍으로 재작성.
새 코드에 설명 주석 없음.

## 2. 자동 검증 (실행함)

- Client x64 Debug 빌드 exit 0 (`Client.exe` 생성, 에러 0,
  `Shader_VtxAnimMeshBinary.cso` 컴파일 성공 — FXC 경고는 기존과 동일한
  deprecated-effects뿐).
- `git diff --check` clean. 삭제한 초안 파일(`EstherCutinView.*`) 잔존 참조 0.
- protocol/contract 하네스는 이번 변경이 Shared/Server 무변경이라 재실행하지 않음.

## 3. 수동 검증 — 사용자 확인 완료 (2026-08-20)

로컬 Server+Client에서 발탄 진입 → Ctrl+Z 반복 확인:

1. 소환수 베기 시작과 동시에 우하단 컷인 등장, 클립 종료 시 소멸 — **확인**.
2. 전체 인상 "나쁘지 않음" — **확인**. 각도·셰이더 미세 튜닝은 사용자가 후속 진행.
3. 확인 중 반영한 튜닝 3회:
   - look 방향: 초기값이 뒤통수(+180° 오차) → yaw `-15 → 195 → 205`(정면 + 화면
     왼쪽 사선), 카메라 눈 X `0.6 → -0.6`.
   - 컷인 클론에 `Enable_RootMotionSuppression("b_root", -1)`(전축 잠금) — 클립의
     Y 상승 이동량 때문에 모델이 뷰포트 밖으로 나갔다 돌아오는 문제 제거.
     `b_root`는 NPC 애니셋(NP_LRSA_00)에도 존재함을 바이너리 스캔으로 확인.
   - 뷰포트 520×460@(730,220) → 640×640@(760,180) → **800×800@(680,100)** —
     우/하로 화면 밖 오버플로 허용(모델 일부 잘림 의도).
4. 진입 시 "preparing Product Effects" 대기는 이 기능과 무관한 기존 계약
   (`CLevel_Loading::Advance_TargetEffectPreparation`, 프레임당 1타깃 프리웜).
   prepared 캐시는 프로세스 수명이라 같은 실행 내 재진입은 빠름 — 조사만 하고
   수정하지 않음(사용자 결정).

## 4. 남은 경계 (후속)

- 원작의 배경 버스트/슬라이드 인·아웃/전용 사운드 없음 — 모델 애니메이션만.
- 각도(`CUTIN_MODEL_YAW_DEGREES`/`CUTIN_CAMERA_*`)와 컷인 조명/셰이더 톤은 사용자
  후속 튜닝 예정.
- 다음 작업(사용자 지시, 2026-08-20): **에스더 슬롯 2 웨이 · 슬롯 3 바훈투르 추가.**
  08-15 추출 RESULT §4의 "같은 절차 재적용" — 원본 체인 실측(EpicSkill →
  CommonAction → 소환 NPC/모델/애니그룹/일격 클립) → umodel 추출 → body+animset
  2클립 쿠킹 → `NpcCatalog.json` 등록(+`actionClips`의 `esther.strike`) → Server
  `CEstherSkillSystem` roster에서 슬롯 2·3 UNSUPPORTED 해제 + contract test.
  컷인은 actionClips 기반이라 데이터 등록만으로 함께 붙는다.
