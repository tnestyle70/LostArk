# 2026-08-30 Camera Tool 컷 관리와 베른 입장 컷신 결과

브랜치: `codex/camera-tool-cut-management` (origin/main 기반 5 커밋)

## 구현 완료

### 1. Camera Tool 컷 단위 관리 (f6da9eb8, 6f599c34, 8dde5e9e)

- 왼쪽 패널에 `Cut Management` 추가: 컷 이름 입력, 패턴/스테이지 바인딩 선택,
  `Death (clear) cut` 대상 선택, `New Cut`, 확인 팝업이 있는 `Delete Cut`.
- 새 컷은 빈 Pos List로 시작한다. `Capture Pos`가 현재 카메라를 P1..PN으로
  순서대로 추가하고 전체 시간을 균등 재배치한다. 저장·재생은 시작/끝 2개
  미만이면 안내 문구와 함께 fail-closed로 막는다.
- 편집 첫 화면을 단순 흐름(Capture Pos / Delete Pos / Start / Stop, Pos List,
  선택 pos의 Eye/LookAt/FOV/구간 속도/LookAt Dummy)으로 재구성했다. 기존
  전체 편집기(Duration, 보간, Easing, Tracking, Shake, Key Time, 중간 삽입)는
  접힌 `Advanced (full editor)` 아래로 이동했으며 기능 제거는 없다.
- 문서 검증 규칙을 툴에서 미러링한다: stable ID 문자셋, 스테이지당 컷 1개,
  컷 32개/키프레임 64개 상한, death 컷 1개, 마지막 일반 컷 삭제 금지.

### 2. 베른 입장 컷신 (2ff79a24, f5020749)

- 신규 정본 `Data/Encounters/Bern/BernEntranceCamera.json`
  (`lostark.level-entrance-camera` v1): 패턴에 묶이지 않는 단일 카메라 cue.
  검증 규칙은 발탄 카메라 문서의 death cue와 동일하게 fail-closed다.
- `CLevel_Bern`이 입장 직후 이 cue를 정확히 한 번 재생한다. 기존
  `CValtanCinematicCameraController::Sample_Cue` 공용 제품 샘플러와
  `SERVER_CINEMATIC` 우선순위 presentation override를 재사용하며 두 번째
  카메라 런타임을 만들지 않았다. 종료·스킵·레벨 파기 모두 같은 경로로
  팔로우 카메라를 복원한다.
- 재생 중에는 기존 follow-disabled 계약이 gameplay command 제출을 막는다.
- ESC press edge로 스킵한다. 문서 누락/손상은 컷신만 격리하고 레벨 진입을
  막지 않는다.

## 자동 검증 (실행함)

- `cl /Zs` 문법 검사: CameraTool.cpp, Level_Bern.cpp 각 변경 시점마다 PASS
- `python Tools/ValtanPipeline/test_valtan_camera_tool_contract.py`: PASS
- CRLF/인코딩 무결성: bare LF 0, U+FFFD 0, 기존 한글 주석 보존 확인
- `git diff --check`: PASS
- Client x64 Debug 전체 빌드: 사용자가 VS에서 수행해 성공 (01:32 빌드)

## 사용자 실행 확인 (서면)

- 컷 생성/이름/삭제/Pos List 동작: "내가 말한것들은 어느정도 잘 나오는거같아"
- 등속+CATMULL_ROM 재생: "훨씬 부드러워졌어"
- 베른 입장 컷신 자동 재생과 ESC 스킵은 로컬 실행으로 확인 중이다.

## 남은 경계 (이 PR 범위 아님)

- Camera Tool은 발탄 문서 전용이라 베른 입장 cue의 툴 내 직접 편집은 없다.
  현재 편집은 JSON 직접 수정이다.
- 발탄 컷신(등장/109/클리어)의 클라이언트 측 ESC 스킵은 미구현.
- 베른 입장 컷신의 "최초 1회만 재생" 정책 없음(입장마다 재생).
- 사용자 로컬 워킹트리의 `ValtanCinematicCamera.json` 저작 데이터와 카메라
  계약 테스트의 float32 허용오차 교정은 별도 커밋 대상으로 남아 있다
  (툴 Save가 float32로 재직렬화하면 기존 정확일치 pin이 깨지는 문제).
