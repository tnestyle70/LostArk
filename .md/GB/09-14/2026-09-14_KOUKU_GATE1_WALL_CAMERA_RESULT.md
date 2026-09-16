# 2026-09-14 쿠크 1관문 진입 전 벽 연출 카메라 — RESULT

계획서: `2026-09-14_KOUKU_GATE1_WALL_CAMERA_PLAN.md` (W01~W08). 이 문서는 구현 완료, 미완료(사용자 결정 대기), 사용자 화면 확인을 나눠 적는다.

## R1. 원본 카메라 출처 (조사 완료)

판정: **별도 게임 카메라 의미 확인**. 벽 Matinee에 직접 붙은 카메라는 없다.

- 벽 `37081_113` SCENE03A Matinee7(export 370 / InterpData 862, 9.518초) 체인에는 카메라·시네마틱 액션이 없다. 트리거 유닛 131의 액션도 VolumeProp 32, SceneEvent 37081_113, Prop despawn·spawn뿐이다.
- 같은 레벨의 `EFMatineePathNodeVolume_0`(export 329)이 Touch되면 `#1719 SetCameraTarget`이 `cameraactor_22`로 2.5초 blend 전환하고, UnTouch되면 `#1717`이 플레이어 카메라로 1.5초 blend 복귀한다(`Kismet_SCENE03A.txt` 24·26·45줄).
  - 볼륨 속성은 `tlinkmatinee_matinee = EFSeqAct_Matinee_2`, `matineepathnode = 327`이다. 경로는 327 → 328, 38.9m다.
  - `cameraactor_22`는 Matinee2 "마티니 카메라01"의 cm01이다.
- 벽을 시작하는 EFLocalTrigger_2, 서버 VolumeProp 32, 벽 b01은 모두 이 볼륨 안에 있다.
- cm01은 dmy에 hard attach되어 있다.
  - 시간: 5.004초
  - 이동: 월드 eye가 UE (1079, 5353, 1262) cm에서 (3765, 7241, 509) cm로 내려오며 벽 정면에 다가간다.
  - FOV: 수평 50°에서 70°로 바뀐다.
- 프로젝트 좌표 변환(`build_gate2_intro_composition.world_pose`, 16:9 FOV 변환)으로 샘플한 후보는 `tmp/gate1wall/cam01_candidate_keys.json`에 있다.
  - 키 72개, 샷 2개(0~2512, 2512~5004 ms)로 나뉜다.
  - 5004ms 끝 pose는 eye (37.650, 5.086, −72.411), 전방 (0.709, −0.043, −0.704), FOV Y 43.0이다.
  - 조사 fork가 별도로 계산한 월드값과 같다(변환 일치이며 원작 재생 의미의 증명은 아니다).
- 미확인:
  - 볼륨이 Matinee2를 플레이어 경로 진행도로 구동하는지와 그 매핑. 스크립트·exe에서 해당 클래스 코드를 찾지 못했다.
  - 볼륨 밖에서 113이 시작될 때의 카메라.
- 참고 영상 `벽넘어가는거 .mp4`, `벽넘어가는거(내거).mp4`는 이 PC의 OneDrive·바탕 화면·동영상·다운로드·문서에 없어 비교하지 않았다.

## R2. 현재 연결 대상 (조사 완료, 설치 대상 미결정)

| Pattern | 벽 WORLD | CAMERA 박스 → 샷 | 샷 소비자 |
|---|---|---|---|
| `KAKULSAYDON_G1_PATTERN_2` 연출_1관문 피날레 (21010ms) | `.world.1` 0~21010 | `.presentation.1` → `kakulsaydon.g1.presentation.44` → `1Stage.finale` 0~21010 | P2 박스 + Area AUTO(`circusfinale`) |
| `KAKULSAYDON_G1_PATTERN_4` 1관문_통합_시퀀스 (61662ms) | `.world.14` 0~19959 | `.presentation.23` → `kakulsaydon.g1.presentation.45` → `kouku.gate1.authored.finale` 0~12258, 다음 포탈 `.presentation.24` 12258~16658 | P4 박스 하나 |

두 샷의 설정은 같다.
- 정지 eye (38.8578, 4.65774, −75.0091), lookAt (46.3628, 4.30222, −81.6082)
- FOV Y 60, blendIn 6000ms, hold 3000ms, blendOut 1500ms, cameraTrack 없음

원본 cm01의 끝 pose와 비교하면 eye가 약 2.9m 떨어져 있고, 보는 방향은 약 3° 이내, FOV는 60과 43이다. 박스·샷 작성자는 모두 tnestyle70이다(`c67a47b2`, `3fc23750`). 카메라 데이터는 아직 바꾸지 않았다.

## R3. W07 편집 연결 (C++ 구현, 빌드 전)

변경 파일은 `Client/Private/KoukuSaydonActionWorkbench.cpp`, `Client/Public/KoukuSaydonActionWorkbench.h`, `Client/Private/Level_KakulSaydonArena.cpp`, `Client/Public/Level_KakulSaydonArena.h`다. 코드 전문은 계획서 W08에 있다.

- Box Detail `Open Composition Camera`가 Pattern·occurrence 문맥을 창에 넘긴다.
- 창은 박스 시작·길이와 "shot T = Sequence 시작+T"를 표시한다. 현재 미리보기 시각의 shot 로컬 시각과 선택 키의 Sequence 시각도 표시한다.
- `Pause Sequence at this position`은 기존 PAUSE transport를 쓴다. 실행 중이면 Seek 후 정지하고, 미리보기가 없으면 멈춘 상태로 연다.
- 샷 소비자(Composition Pattern·Bundle 박스, Area AUTO)를 나열한다. 소비자가 2개 이상이면 `Edit this shared shot for every consumer above` 체크 전까지 편집 위젯을 비활성화한다(창·Box Detail 모두).
- `Make dedicated shot for this box`는 샷을 PATTERN_ONLY로 복제(`camera.kouku.pattern.N`)하고 새 리소스(`kakulsaydon.g1.presentation.N`)로 해당 occurrence만 바꾼다. Composition 커밋이 실패하면 미저장 복제 샷을 제거한다.
- 트랙이 있는 샷의 Box Detail 캡처는 `Replace whole track with current view...` 확인 팝업으로 분리했다. 트랙 샷의 WORLD→PLAYER 앵커 전환(트랙 삭제)은 비활성화했다.
- `Play Camera`에 카메라 단독 툴팁과 저장 대상 안내를 추가했다.
- Map Tool 카메라 편집기로 넘기는 경로는 만들지 않았다. Workbench 창에서 키 시각·Pos·pitch/yaw/roll·FOV·보간·easing·blend를 편집하고, 세 도구(Workbench·Map Tool·Cinematic Camera Tool)가 같은 `camerashots.json`을 같은 기준본 CAS로 저장한다.

자동 검사:
- 두 cpp 격리 구문 검사(`cl /Zs /std:c++20 /DUNICODE`, Engine/Public 우선 include): 오류 0, 종료 0. 경고는 검사 명령이 강제한 `/utf-8`에서 나온 C4828뿐이다.
- CRLF·BOM 보존, `git diff --check` 통과, 적용 스크립트 앵커는 1회씩 일치했다.
- 실행하지 않음: 제품 빌드·링크, 편집 동작의 실제 입력 확인.

어느 쪽이 실제로 재생되는지 (코드·데이터 확인):
- **실제 게임 진행:** Gameplay 트리거 `1Stage_Final`(triggerBox (27.8, 0.45, −67.7), triggerOnce)이 Server `playSequence` → `world.sequence.instance.circusfinale`를 보낸다. Client `Find_ActiveCameraShot`은 재생 중인 인스턴스와 `sequenceInstanceId`가 같은 AUTO 샷을 고른다(`Level_KakulSaydonArena.cpp` 3524~3530줄). 게시된 런타임 `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.camerashots.json`(revision 84)의 `1Stage.finale`(AUTO, circusfinale, 정지)이 쓰인다.
- **Sequencer의 Complete Play(`Play Sequence`):** Gate의 `enterCombatOnFinish` Pattern 정확히 하나만 재생한다(`KoukuSaydonActionWorkbench.cpp` `Request_CompleteSequencePlay`). GATE1에서는 P4 `1관문_통합_시퀀스`만 해당한다. P2는 목록에서 직접 고를 때만 재생된다.
- **두 곳 모두 설치:** 가능하다. 같은 트랙을 `kouku.gate1.authored.finale`(P4 전용)과 `1Stage.finale`(P2 박스 + 실제 게임 AUTO)에 넣는다. 게임 AUTO에 반영하려면 Save Camera 뒤 Area 게시(`Publish-MapAuthoring.ps1 -Scope Area`)가 필요하다.

사용자 결정(2026-09-14):
- 시간 매핑은 참고 영상을 먼저 비교한 뒤 정한다(설치 보류).
- 포탈 박스는 그대로 둔다.
- 설치 대상은 "두 곳 모두 가능한지"를 물었고, 위 사실로 답했다.

## R4. 남은 것 (사용자 결정 대기)

1. 설치 대상: P4 전용 샷 또는 P2 공유 샷(게임플레이 AUTO까지 바뀜, 또는 새 편집기의 전용 샷 분리 사용).
2. 원본 cm01 궤적의 Sequence 시간 매핑. 원본이 증명하지 않으므로 승인이 필요하다.
   - 경로 진행도 가정으로 벽 시작을 Matinee2 약 2.29초에 맞추는 안
   - 5.004초 전체를 벽 시작과 함께 재생하는 안
   - 영상 기반 재현(영상 파일 필요)
3. 포탈 박스(12258ms, blend 0)의 정지 eye가 새 끝 pose와 약 2.9m·FOV 17° 다르다. 후속 연출이라 변경하지 않았다.

## R5. 사용자 확인 경로 (빌드 후)

1. VS에서 Client를 빌드하고 실행한다.
2. KoukuSaydon → F1 → Action Workbench → Composition Actions → Sequence에서 `1관문_통합_시퀀스`(또는 결정한 Pattern)를 고른다.
3. CAMERA 박스 `카메라_통합_피날레`를 선택하고 Box Detail의 `Open Composition Camera`를 누른다.
4. 확인할 것:
   - 창에 `Played by 1`과 박스 시작·길이가 나오는지
   - 키를 고르고 `Pause Sequence at this position`을 누르면 벽과 함께 그 시각에 멈추는지
   - Pos나 FOV를 조금 바꾸면 멈춘 화면에 바로 반영되는지
5. `Save Camera`(`Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json`) → `Reload Cameras` → 같은 시각을 다시 확인한다. 끝나면 원래 값으로 되돌리고 Save한다.
6. 공유 샷 확인: P2 `연출_1관문 피날레` 박스를 열면 `Played by 2`와 편집 잠금이 나와야 한다. `Make dedicated shot for this box` 뒤에는 Sequence Save가 필요하다.

화면 판정은 아직 없다.
