# 발탄 Event_02 벽·기둥 파괴 Map Tool 카메라 등록 결과

## 완료

- 기존 `발탄 포효`와 분리된 `발탄 벽·기둥 파괴 (원본 Event_02)` 컷신을 추가했다.
- 원본 5,500ms 카메라를 104개 키로 등록했다.
- 새 컷신을 `world.sequence.instance.valtan.source-preview.phase2`에 연결했다.
- CameraShots 저작본과 런타임 게시본에 같은 샷·컷신이 존재한다.
- 기존 `editor.cutscene.valtan.roar` 데이터는 변경하지 않았다.

## 변경 파일

- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.camerashots.json`
- `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.camerashots.json`
- `Tools/ValtanPipeline/sync_valtan_wall_destruction_maptool_camera.py`

## 자동 검증

- 동기화 검사: `ok: Valtan Event_02 Map Tool camera (104 keys, 0..5500 ms)`
- Map Authoring CameraShots Validate: 통과
- Map Authoring CameraShots Publish: 통과, 게시 SHA-256 `a76af0621031649c6aed7cd3263182deacae3417ed2700ba9df2678ecd3b41e1`
- 저작본/런타임의 새 샷과 컷신 구조 비교: 동일
- `git diff --check`: 통과

## 수동 확인

Client를 다시 빌드할 필요는 없다. 실행 중인 Client가 있다면 Camera 데이터 Reload 또는 Client 재실행 후 다음 항목을 확인한다.

1. F1 → Map Tool → Valtan Arena → Camera
2. `발탄 벽·기둥 파괴 (원본 Event_02)` 선택
3. 재생 시 0초 근접 포효, 약 1.3초부터 빠른 후퇴·상승, 5.5초 상공 와이드 구도가 나오는지 확인

사용자의 화면 확인 전에는 visual PASS로 판정하지 않는다.

## 남은 범위

- 이번 요청은 카메라 등록이므로 벽 조각 모델의 신규 맵 애니메이션 저작은 포함하지 않았다.
- 저장소 시작 규칙의 LAN endpoint 동기화는 카메라 작업과 무관한 네트워크/방화벽 변경 위험으로 실행 승인이 거부되어 수행하지 못했다.
