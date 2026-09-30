# 콜로세움 매치 로딩 초상 구도 보정 RESULT (2026-09-30)

## 판정

- 사실: 원본 무비 colosseumloadings3의 texture_L_0/R_0는 렌더 타깃 자리이고, 30장 텍스처 중 캐릭터 일러스트가 없다. 즉 원본은 3D 아바타를 실시간 렌더한다.
- 추론: 시바견 인형탈·제복 코스튬이 개별 장착 상태로 표시되는 점도 같은 결론과 맞는다.
- 미확인: EFTable_CameraSetting의 초상 카메라 수치는 확인하지 못했다. 원본 카메라 값은 모른다.

## 우리 화면이 이상했던 이유 (코드 실측)

| 항목 | 이전 | 문제 |
|---|---|---|
| 카메라 | 거리 3.0m, 눈 1.05, 시선 0.95, FOV 35° | 전신이 작게 나옴 (다른 창의 전신 초상 값을 그대로 씀) |
| 무기 | 숨김 마스크 없음 | 창이 세로로 솟음 |
| 렌더 타깃 | 373×324 (슬롯과 동일 종횡비) | 문제 없음 |
| 포즈 | 전환 프레임의 살아 있는 애니메이션 | 제어 불가 (아래 미적용) |

## 적용

`Client/Private/MainApp.cpp` `CMainApp::Render_ColosseumTransferPortrait`
- 허리 위 클로즈업: 시선 높이 1.30×scale, 눈 높이 1.32×scale, 거리 = 1.3×scale / (2·tan(FOV/2)), FOV 35°. scale은 `Get_PresentationScale()`이라 직업별 키에 자동으로 맞는다.
- 캡처 직전 `Set_WeaponPartsVisible(false)`: 무기·방패 숨김, 코스튬 파츠 유지. 베른 캐릭터는 이 프레임 다음 레벨 전환으로 파괴된다.
- 로그 `[Colosseum.MatchLoading] portrait=3d dist= eye= look= fov= scale= rt=`

상수 위치: 같은 함수 안 `Camera.` 대입 6줄.

## 미적용과 사유

- idle 안정 포즈: 포즈는 다음 Update에 반영되는데 전환은 그 Update에서 일어나 캡처 프레임에 강제할 수 없다.
- 임시 조명: 초상은 월드 디퍼드 경로를 쓰고 Data/Rendering/**은 수정 금지라 미적용. 필요하면 characterSelectLight 방식의 transient light를 별도 검토.
- 배경 광채: 레이아웃상 WedgeGlow(2,3)가 Portrait(4,5)보다 앞 순서라 뒤에 그려져 있어 변경 없음.
- 우측 샘플 2D 톤: 생략.
- 6직업 표: scale 비례식이라 상수표를 두지 않았다. 실제 scale은 로그로 확인한다.

## 검증

- MainApp.cpp `cl /Zs` rc=0. CRLF 유지, Level_Loading·JSON 미수정.
- 미실행: 빌드, 화면 확인(사용자).
