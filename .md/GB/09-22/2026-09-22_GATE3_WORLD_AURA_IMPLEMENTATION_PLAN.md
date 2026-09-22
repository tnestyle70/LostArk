# G00. Gate3 진입·리스폰 오라와 자동 입장 구현 계획

## 목표와 현재 실측

사용자 이미지1·3의 둥근 사각형 진입오라는 원본 Zone37081 Prop300010의 EFDLProp_ITR_00279가 참조하는 BFX_Low_01.Etc.Par_G_ITR_T_EF_On/GoOff다. 원본 UE 위치[-2220.617676,-95459.429688,2559]를 기존 cm→m/Z 변환으로[-22.20617676,25.59,954.59429688]에 둔다. 이미지2 검·기어·황금 원형은 Prop300009의 EFDLProp_ITR_10175가 참조하는 FX_ITR_10175.Par_G_Waiting_01이며[-11.9999292,25.59,964.54328125]다. 이름으로만 추측하지 않고 DeployData→Prop DB→LookInfo→ParticleSystem→TypeData geometry를 연결했다.

기존 Client Update_GateProgress는 3관문 입장 버튼을 typed Request_GateProgressPropose(ENTER_GATE3)로 제출하며 Server GameRoom_GateProgress가 현재 roster·관문 상태를 검증한다. 준비 deck 범위는 Shared KoukuArenaReadyAreas.h에 있고 WAIT_ENTRY에서 이동까지 막는 기존 분기가 있다.

## G01. 원본 Effect와 라이브러리 등록

기존 importer/Cascade sourceRecipe와 native material shader 번역 공정을 재사용해 대기/활성 진입오라와 리스폰오라 문서를 추가한다. 기존 Effect 문서는 덮어쓰지 않는다. World 카테고리에 월드 | 진입오라, 월드 | 리스폰오라로 노출한다. 원본 mesh는 Resources/Effect/World 아래 설치하고, texture는 exact source identity가 이미 설치된 Effect-relative 경로를 재사용하거나 기존 native 복원 경로에 설치하며 원본 emitter loop·색·TRS를 보존한다. Shared Material header/shader 편집은 다른 담당 변경을 보존한다.

## G02. trigger와 countdown

진입오라 원본 크기/배치를 근거로 Shared trigger를 정의한다. Client의 실제 replicated player 위치에서 진입을 감지하고 대기/활성 문서를 바꾼다. 10초 연속 체류 후 기존 typed ENTER_GATE3 경로로 제출한다. 문구는 기존 제품 font 렌더 경로로 흰색 안내와 노란색 초 수를 그린다. 이탈·사망·상태변경·실패는 countdown을 취소하고, 전송은 한 번만 한다. Server 권위 이동과 기존 party consent를 유지한다. WAIT_ENTRY는 준비 deck 이동만 허용한다.

## 소유 파일과 검증

오라 담당은 신규 Effect/복원 도구, 기존 catalog와 native shader/material/project 등록을 최소 변경한다. 통합 담당은 Client Level_KakulSaydonArena H/CPP·ClickMoveEffect, Server 준비영역 이동 경계, Shared KoukuArenaReadyAreas와 countdown을 구현한다. 새 C++ 파일은 계획하지 않는다. source projection의 JSON parse, 모든 resource 존재, exact material/runtime descriptor, 실제 playback finite/draw count를 기존 focused 진단으로 확인한다. Product 컴파일은 통합 담당이 수행한다. Client/UI는 실행하지 않으며 최종 화면 판정은 사용자에게 남긴다.
