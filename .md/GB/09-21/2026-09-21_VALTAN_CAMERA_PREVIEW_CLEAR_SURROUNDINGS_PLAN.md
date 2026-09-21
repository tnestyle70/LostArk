# Valtan Camera Preview Clear Surroundings Plan

## Goal

MapTool의 Camera Shots 도구에서 `Reload Shots` 옆 버튼으로 카메라를 가리는
발탄이 서 있는 중앙 Arena 바닥 위를 둘러싼 벽·성벽·절벽 계열을 미리보기 동안 숨긴다. 발탄
최후와 발탄 스킬 연출(버러지)의 카메라·월드 배우·이펙트·Arena 바닥은 유지한다.

## Contract

- 버튼은 `Hide Arena Outer Walls`와 `Restore Arena Outer Walls`를 토글한다.
- 숨김은 authoring `Visible` 값, 저장 문서, runtime stage suppression과 별개인
  camera-preview overlay다.
- 대상 구조물은 컷신 World Sequence와 Server destruction 상태가 Deploy visibility를
  다시 적용해도 매 frame camera-preview overlay가 다시 숨긴다. Restore 전에는
  컷신 중에도 보이지 않는다.
- 대상은 이름·반경으로 추측하지 않는다. Valtan Deploy runtime의 145개 stable placement
  중 floor destruction group이 소유한 바닥·레일 6개만 보존하고, 나머지 139개를 숨긴다.
  여기에는 `outerwall109` 54개뿐 아니라 중앙 Arena 위 벽 계열 77개와 destruction group이
  없는 구조물 8개도 포함한다. water, stage, actors와 Server destruction 상태는 대상이 아니다.
- MapTool을 닫거나 authoring Level이 바뀌면 overlay를 해제한다.

## Validation

- Deploy 정본 145개와 floor/rail 6개를 대조해 숨김 대상이 정확히 139개인지 확인한다.
- Debug Client C++ compile을 실행한다. 실행 중인 Client가 있으면 링크/교체하지 않는다.
- MapTool 화면 확인은 사용자가 `Hide Arena Outer Walls` → 컷신 재생 →
  `Restore Arena Outer Walls` 순서로 확인한다.
