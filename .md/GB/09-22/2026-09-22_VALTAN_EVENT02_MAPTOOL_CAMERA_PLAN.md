# 발탄 Event_02 벽·기둥 파괴 Map Tool 카메라 등록 계획

## 목표

- 첨부 영상의 근접 포효 뒤 상공으로 빠지는 5.5초 카메라를 원본 데이터로 복구한다.
- 기존 `발탄 포효`(2페이즈 시작의 하늘·붉은 구름 연출)는 변경하지 않는다.
- Map Tool의 Camera 목록에서 별도 컷신으로 선택하고, 기존 2페이즈 맵 시퀀스와 같은 시계로 재생할 수 있게 한다.

## 확인된 원본

- 레벨 패키지: `LV_LUT_HEARTRB_ED_SCENE02A`
- Matinee: `efseqact_matinee_20` / InterpData export 104
- 이벤트: `Event_02`
- 카메라: Kismet `SetCameraTarget373`가 직접 선택하는 `CameraActor20`, 그룹 `c1`
- 길이: 5,500ms
- 원본 벽·기둥 애니메이션 시작: 약 1,300ms

Director 트랙이 없는 장면이므로 Director만 요구하는 일반 추출 경로로는 누락된다. 이미 제품 좌표로 변환된 `VALTAN_ARENA_BREAK_109`의 `IMPACT_HOLD` 원본 구간 600ms 이후, `WIDE_REVEAL`, `RECOVERY`를 이어 붙여 Event_02의 0~5,500ms 카메라 104키를 복원한다.

## 구현

1. `source.phase2-wall-destruction.cut01` 샷을 CameraShots 저작 데이터에 추가한다.
2. `editor.cutscene.valtan.phase2-wall-destruction` 컷신을 추가한다.
3. 컷신은 `world.sequence.instance.valtan.source-preview.phase2`와 연결한다.
4. 동기화 도구는 원본 큐의 키 개수·시간 범위·단조 증가와 기존 `editor.cutscene.valtan.roar` 보존을 검사한다.
5. Map Authoring publisher의 `CameraShots` 범위로 검증하고 런타임 데이터를 게시한다.

## 검증

- 동기화 검사: 104키, 0~5,500ms
- `Publish-MapAuthoring.ps1 -Mode Validate -Scope CameraShots`
- `Publish-MapAuthoring.ps1 -Mode Publish -Scope CameraShots`
- `git diff --check`
- 사용자가 Map Tool → Camera에서 새 컷신을 선택해 화면을 최종 확인한다.

## 범위 경계

이번 변경은 원본 카메라 등록이다. 연결된 기존 2페이즈 월드 시퀀스의 발탄·효과·사운드는 같은 시간축으로 재생하지만, 벽 조각 모델 자체를 새로 저작하거나 추가하지 않는다.
