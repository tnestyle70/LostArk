# 쿠크 패턴 방향·판정·카드미로 연출 구현 계획

## 현재 실측과 범위

Composition revision 2025를 기준으로 조사했다. P13 조커 찾기의 retarget이 큰 세이튼의 yaw를 남기지만 P86/P87은 위치만 복원한다. P21 레이저는 시작 retarget이 없고 모델·빔·판정은 local +X를 전방으로 쓴다. P25의 MAP 폭발 13개와 보스 기준 피해 Sector 12개의 위치·각도·시각이 다르다. P23 팡파레의 회전 Sector와 고정 BOX 밀림도 서로 다른 영역이다.

P77 카드미로는 WORLD26으로 별도 쿠크를 만든다. 사용자가 추가한 logic112는 아직 triggerKind가 없는 정의다. 플레이어 소멸 이펙트 4개와 hide trigger의 시각도 다르다.

## G01 방향과 충돌

- P86/P87은 기존 resetBossYawDegrees로 배치 yaw 226.5°를 복원한다.
- P21은 준비 stage에서 Server가 선택한 플레이어를 향하고 모델 +X와 collider 전방을 일치시킨다. Client preview도 같은 기준을 사용한다.
- P25는 기존 시작 reset 및 0ms 중앙 이동을 사용해 yaw 306.5°로 시작한다. 피해 Sector는 각 MAP 폭발의 실제 변환·시각에 연결한다.
- P23/P85는 실제 이펙트에 대응하는 피해창에 전용 강제 밀림 결과를 연결한다. 다른 패턴의 공용 결과는 보존한다.

## G02 카드미로의 기존 배우와 플레이어

기존 G2 Kouku의 CModel에 동일 골격의 연출 animation set을 Attach하고 P77의 일반 animation occurrence로 소비한다. 별도 WORLD 배우는 비활성화한다. 위치는 기존 Server boss motion/teleport 계약을 사용한다.

새 trigger CARD_MAZE_STAGE_PLAYERS는 같은 패턴의 MAP EFFECT occurrence ID 1~4개를 순서대로 참조한다. Authoring 좌표를 복제하지 않고 projector가 현재 이펙트 위치를 Product playerEntryPositions로 해석한다. Server는 roster와 모든 목적지를 먼저 검증한 뒤 일괄 이동하고 동일 roster 순서로 기존 hide trigger를 적용한다. 소멸 이펙트 시각과 hide 시각을 일치시킨다.

수정 경로는 Composition H/CPP·Workbench → projector·GameplayBalance publisher → GameplayCatalog·Kouku LogicRuntime 및 기존 boss animation admission이다. 기존 파일만 확장하므로 vcxproj/filter의 새 C++ 등록은 필요 없다.

## G03 노란시선

원본 spotlight V1은 보존하고 별도 표시 이름 `대형 세이튼 | 노란시선`으로 재사용한다. 설치된 MN_RPCT_06의 좌우 눈 본과 애니메이션을 실측하여 두 occurrence를 연결한다. 본 scale을 재곱하지 않는 기존 Effect pivot을 사용한다. 최종 밝기와 화면 인상은 사용자 확인으로 구분한다.

## 반영과 검증

마무리 중 사용자가 추가로 보고한 로딩 화면의 ImGui·폰트 잔류도 확인한다. `LEVEL::LOADING`에서 기존 runtime text 숨김과 ImGui 최종 제출·도구 자막 경계가 일치하도록 최소 수정한다. 로딩 외 도구의 열림 상태와 정상 컷씬 자막은 보존하고, 기존 `BeginFrame/CancelFrame` 수명주기를 사용한다.

데이터는 out의 stable ID 필드 패치로 준비한다. 최종 교체 전 최신 저장본에 재병합하고 hash 재확인·백업·원자 교체를 유지한다. 무관한 편집을 보존한다.

변경 C++ 최소 컴파일, 실제 Server 논리와 V1 재생을 사용하는 CPU 검증, JSON parse 및 publisher 검증, git diff --check를 수행한다. 실제 반영 후 domain publish와 필요한 Product Debug build를 수행한다. Client 실행과 아레나 화면 검증은 사용자가 직접 한다.
