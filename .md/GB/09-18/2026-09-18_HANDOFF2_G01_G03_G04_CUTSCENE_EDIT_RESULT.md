# 2026-09-18 HANDOFF2 G01·G03·G04 — 발탄 컷신 기준선 복구·같은 화면 편집·원본 정확도 RESULT

입력: "설명서 2 통합 수정 실행계획 — 현행 재검증·사용자 결정 반영"의 G01·G03·G04와 "클로드에게 그대로 전달할 작업 지시".
범위: F1 → Map Tool → 발탄 Area(`LV_LUT_HEARTRB_ED`) → Camera 의 컷신 5종(1관문 입장·발탄 등장·버러지·포효·최후).
불변: 발탄 deathCue(2090ms/3키), 전투 Camera Tool·패턴·수치·Encounter·조명, World 리소스/템플릿/인스턴스 개수 3/7/7, 네트워크·IDE 설정.
게시(Publish)·제품 빌드·Client 실행·화면 캡처·commit/push 는 하지 않았다.
기준선 백업: `out/Handoff2Baseline_G01G03G04_1330/` (편집 전 C++ 15개, `MapTool_Internal.h`, 저작 JSON 2개).
분석 스크립트·산출물: `out/Handoff2_G01G03G04/`.

---

## G01 — 기준선·실패 복구

### 기존 반영 (이번 작업 전 상태)

- `Ensure_WorldObjectPrototype` 는 `Add_Prototype` 실패를 전부 "이미 등록됨"으로 간주했다. 엔진 `CPrototype_Manager::Add_Prototype` 은 관리자 미초기화·레벨 범위 밖·null 원형·중복 태그 네 경우에 모두 `E_FAIL` 을 돌려준다(`Engine/Private/Prototype_Manager.cpp:23-34`).
- 컷신 세션의 World 인스턴스 소유가 bool 하나였다. 부분 실패 때 살아남은 인스턴스를 정지할 목록이 없었다.
- `Load_CameraShots` 는 파싱 도중 실패하면 이미 비운 draft 를 되돌리지 못했다.
- 컷 경계에 `cut.startMs == shot.trackDurationMs` 예외가 있었다. 이 조건은 경계 규칙과 무관한 우연한 비교였다.

### 이번 수정

1. **원형 중복과 실제 실패 구분** — `MapTool_Cutscenes.cpp:50-126`.
   - Kouku 소유 레벨과 범위 밖 레벨 인덱스는 먼저 거른다.
   - `CWorldSequenceObject::Create` 가 null 이면 생성 실패로 보고 멈춘다.
   - `Add_Prototype` 이 성공하면 "registered by Map Tool on Level N" 으로 기록한다.
   - 나머지 `E_FAIL` 은 앞의 세 원인이 이미 걸러진 뒤이므로 중복뿐이다. 이 경우 "already registered on Level N; reused" 로 기록한다.
   - 상태는 `m_strWorldObjectPrototypeStatus` 에 남는다.
2. **세션 소유 ID 와 준비 플래그 분리** — `m_CutsceneSessionInstanceIds` (`MapTool.h`).
   - `Prepare_EditorCutsceneWorld` 는 인스턴스를 하나씩 시작하면서 ID 를 기록한다. 하나라도 실패하면 `Release_EditorCutsceneWorld(true)` 가 이미 시작한 것까지 전부 정지한다.
   - 새 함수는 `Release_EditorCutsceneWorld`(`:244`), `Abandon_EditorCutscene`(`:263`), `Refresh_EditorCutsceneWorldDraft`(`:292`) 이다.
   - 정리 지점: 실패(`Seek` `:314-360`), Stop(`:386`), Area 전환(`MapTool_Area.cpp:689, 821`), Level 전환(`MapTool_Area.cpp:1323`, 레이어가 사라진 뒤라 `restore=false` 로 버림), F1 닫기(`MapTool.cpp:99`).
3. **`Load_CameraShots` 를 read → validate → stage → swap 순서로** — `MapTool_CameraShots.cpp:596`(`Parse_CameraShotDocument`), `:930-1013`.
   - 같은 Area 의 재로드가 실패하면 기존 샷·컷신을 유지한다. 상태: "Reload failed; the loaded shots and cutscenes are kept: …".
   - 다른 Area 에서 실패하면 비운다.
   - 성공하면 실행 중 세션을 정지하고 교체한다. 같은 Area 면 cutsceneId 로 선택을 유지한다.
4. **콤보 변경 시 이전 세션 정지** — `MapTool_CameraShots.cpp:1411`. 상태: "이전 컷신 미리보기를 정지했습니다.". 배우 선택과 편집 스냅숏도 함께 초기화한다.
5. **컷 소유 [start, end)** — `Find_CutsceneCutAt` `MapTool_Cutscenes.cpp:128-157`.
   - 마지막 컷의 끝 순간만 포함한다(`&cut == lastCut && timeMs <= endMs`).
   - `startMs == trackDurationMs` 예외는 삭제했다.

### 자동 검증 (명령·exit code)

| 명령 | 결과 |
|---|---|
| `python -B out/Handoff2_G01G03G04/g01_harness.py` | **exit 0, RESULT PASS**. 5 컷신의 모든 컷 시작·공유 경계(다음 컷 로컬 0)·마지막 컷 끝(자세 유지)·tail(카메라 반납: 입장 >23386, 버러지 >5736, 포효 >5000 ms) 확인. 현재 데이터에서 새 규칙은 이전 규칙과 결과가 같다 |
| 같은 하네스의 writer 왕복 (revision 2 문서) | float 필드 3059개, `to_chars` 최단 표기 왕복 불일치 **0**. 이전 `%.6g` 였다면 값이 바뀌었을 필드 1311개 (revision 1 문서 기준으로는 819개 중 359개) |
| 격리 /Zs 컴파일 `out/IsolatedCompile20260918g/build.bat` (14:33:53 시작, 모든 소스 수정 이후) | 9 TU 전부 **EXITCODE 0**, 오류 0. 경고는 기존 인코딩 경고 C4819 86줄·C4828 1160줄뿐. 로그 `out/IsolatedCompile20260918g/log.txt` |

부분 실패 정리(살아남은 인스턴스까지 정지)는 코드 경로로만 확인했고 실행하지 않았다.

### 사용자 확인 (미실시)

아래 "사용자 확인 절차"의 1~4.

### 미지원·추가 승인

없음.

### 내 실수

- `WorldSequenceToolPanel.h` 의 대상 구역이 원래 LF 만 쓰는 줄이라 CRLF 앵커가 0회 일치했다. LF 앵커로 다시 잡았다. 새 줄은 파일 다수인 CRLF 로 넣었고, 기존 lone LF 10줄은 그대로 남았다.
- `Switch_EditorArea` 앵커가 `Begin_EditorAreaSwitch` 와 같은 문자열이라 2회 일치했다. 고유한 `runtimeAttach` 조건 줄까지 넓혀 1회로 만들었다.
- 첫 격리 컴파일 로그(13:52:29 종료)가 마지막 소스 수정(13:51:21) 뒤의 소스를 컴파일했는지 증명할 수 없었다. 14:33 에 다시 돌렸다.

---

## G03 — 같은 화면 편집·저장

### 기존 반영

- 컷신 화면은 카메라 컷만 편집했다. World 배우는 게시본을 읽어 재생만 했다.
- 통합 Save 는 발탄 Area 에서도 쿠크 `WorldObjectTool` 문서가 dirty 면 그 문서를 저장할 수 있었다.

### 이번 수정

- **미저장 draft 미리보기** — `CWorldSequencePlayer::Replace_DocumentKeepingModels` (`WorldSequencePlayer_Objects.cpp:245-286`, `WorldSequencePlayer.h:112-116`).
  - `Set_Document` 와 같은 검증을 거친 뒤 모델 입력이 바뀌지 않은 모델은 재사용하고 문서만 교체한다.
  - 컷신 준비는 Area 의 World 패널 draft 가 있으면 그것을 쓴다. 출처 표시는 "저작 draft (미저장 변경 포함)" 또는 "저작본 (Data/Maps/Authoring, 저장된 상태)" 이다.
  - 패널 draft 가 없을 때만 게시본을 쓴다. 출처 표시: "게시본 … 저작 draft 없음".
- **배우 표와 상태** — `Render_CutsceneActorSection` `MapTool_CameraShots.cpp:1583-2015`.
  - 표 열: 인스턴스·템플릿·모델·상태.
  - 상태 값: "문서에 없음 (등록 실패)" / "등록됨 (정지)" / "활성 - 표시" / "활성 - 시작 전 (정상 숨김)" / "활성 - 키 visible=false (정상 숨김)" / "실패 (해제됨)" / "비활성" (`MapTool_CameraShots.cpp:1620-1645`).
  - 요약 줄은 "World 등록 N개 · 활성 M개" 로 등록 수와 활성 수를 나눴다(`:1488`).
  - 실패는 빨간 배너와 "World instance could not start: id - reason" 또는 "World actor failed at N ms: id - reason" 으로 보인다. reason 에는 player 가 붙인 자산·클립 ID 가 들어간다.
- **편집 가능한 항목**
  - 인스턴스: 앵커 위치 "(월드 절대, m)", startDelayMs, playbackSpeed.
  - 키: 시각(첫·끝 키는 잠금), offset "(앵커 기준, m)", yaw(순수 yaw 쿼터니언일 때만 편집, 아니면 읽기 전용), scale, visible.
  - 키 추가·삭제: "현재 시각에 키 추가"(직전 키 복사), "선택 키 삭제"(경계 키이거나 키가 2개 이하면 비활성).
  - 클립: clipName, startMs(슬롯 첫 행은 잠금), sourceStartMs, rate, loop, hold.
  - 편집은 항목이 활성이 아닐 때 스냅숏을 잡는다. 패널 문서 검증에 실패하면 ID 로 찾아 스냅숏으로 되돌린다. 성공하면 dirty 로 표시하고 미리보기를 다시 준비한다.
- **공유 템플릿** — 공유 중이면 경고를 띄운다. "이 배우 전용 템플릿으로 복제" 를 누르면 `<templateId>.own[N]` 으로 복제하고 검증에 실패하면 되돌린다(`:1775`).
- **짝 저장 `Save_CutsceneAuthoring`** — `MapTool_CameraShots.cpp:1220-1382`.
  - 두 문서를 모두 사전 검증한다. 카메라는 쓸 텍스트를 다시 파싱하고 `Validate_CameraShotDraft` 로 확인한다. World 는 패널 `Validate` 로 확인한다.
  - sequence 경로에 잠금을 걸고 `RecoverAuthoringTransactionUnderLock` 을 거친다.
  - 기준선 충돌을 검사한다(카메라 디스크 == 로드 기준선, 패널 `Matches_SequenceBaseline`).
  - 두 파일을 모두 백업한 뒤 카메라를 원자 저장하고, 이어서 `Save_SequenceChecked` 를 한다.
  - 뒤 단계가 실패하면 두 백업과 패널 기준선을 복원하고 dirty 를 유지한다.
  - 성공 상태: "저장했습니다 (저작본만, 게시 안 함): …".
  - 저장 줄에는 "미저장: 카메라 + World" 표시와 게시하지 않는다는 안내가 붙는다.
- **카메라 dirty 와 Reload**
  - dirty 판정은 로드한 정규화 텍스트와 현재 draft 의 Build 텍스트 비교다(`Is_CameraShotDraftDirty` `:1139`).
  - 카메라가 dirty 일 때 "Reload Shots" 는 두 번 눌러야 한다(`:2094-2098`).
- **카메라 writer** — `std::to_chars` 최단 float 표기로 바꿨다(`:1015-1030`). 이전 `%.6g` 는 값 자체를 바꿨다.
- **쿠크 문서 오저장 차단** — `MainApp.cpp:2687`. `objectDirty = koukuHosted && m_pWorldObjectTool && …`. `WorldObjectTool` 의 쿠크 AREA_ID 하드코딩에는 발탄을 연결하지 않았다.
- **WorldSequenceToolPanel** — `Matches_SequenceBaseline`·`Save_SequenceChecked` (`WorldSequenceToolPanel.cpp:487-546`), 선언 `WorldSequenceToolPanel.h:90-117`.
- 코드는 컷신 종류와 무관한 범용 경로다. 5종 모두 같은 표·저장 경로를 탄다(컷신별 코드 없음). 포효를 먼저 확인하는 순서는 사용자 확인 절차에 반영했다.

### 자동 검증

- 격리 /Zs 9 TU EXIT 0 (G01 표와 같은 실행).
- `git diff --check` (C++ 10개) **exit 0**.
- 인코딩 바이트 검증: 편집한 C++ 10개 모두 BOM 없음. CRLF 를 유지했고, `WorldSequenceToolPanel.h` 의 기존 lone LF 10줄은 그대로다. U+FFFD 개수는 기준선과 같다(`MapTool.h` 6, `MapTool.cpp` 38, 편집 전부터 있던 것).
- `MainApp.cpp` 는 /utf-8 없이 컴파일되므로 ASCII 만 추가했다.

### 사용자 확인 (미실시)

포효로 한 번 저장·다시 읽기 왕복 → 나머지 4종 재생. 아래 절차 5~7.

### 미지원·추가 승인

- worldsequences 의 C++ writer 는 `setprecision(9)` 이다. 사용자가 이 화면에서 처음 Save 하면 Python 이 쓴 숫자 텍스트가 9자리로 바뀌어 diff 가 커진다. float 값은 동일하다. writer 는 쿠크도 쓰므로 바꾸지 않았다.
- 카메라 파일도 첫 Save 에서 C++ Build 서식으로 바뀐다. 값은 왕복 불일치 0 이다.

### 내 실수

- `step5` 적용 전 dry run 에 무관한 assert(`replace("m_Camera","X")`)가 남아 있었고, `to_chars` 주석 문장이 깨져 있었다. 둘 다 적용 전에 고쳤다.

---

## G04 — 원본 정확도

### 기존 반영

- 카메라 13샷 91키는 원본 키를 옮긴 것이었다. 키 사이는 Map Tool 샘플러(균일 Catmull-Rom, FOV 선형)로 보간된다. 원본 Hermite 곡선이나 이동하는 부모 배우를 따르는지는 같은 T 에서 비교된 적이 없었다.
- World 배우 7템플릿은 `gen_valtan_worldsequences.py`(같은 job 의 이전 세션)가 만들었다.
  - 0.1s 포즈 표본을 선형 키로 옮겼다.
  - 슬롯 a/b/up_a/up_b 는 "시작 시각 순 한 줄" 로 합쳤다.
  - 그 뒤 발탄 본체·유령 키에 −90° 를 한 번 접었다.

### 이번 수정 — 카메라

**방법**
- 원본은 추출기의 `world_pose` 를 240Hz 로 평가했다(`g04_original_dense.py`). 부모 배우에 붙은 카메라까지 포함된다.
- Map Tool 샘플러와 같은 T 에서 eye(m)·전방(°)·up 포함 방향(°)·fovY(°)를 비교했다(`g04_camera_dense_compare.py`).
- 허용오차는 eye 0.05 m / 전방 0.5° / fovY 0.25° 이다.
- 초과한 샷은 원본에서 그 ms 의 포즈를 표본으로 다시 키를 박았다(`g04_camera_refit.py`). 키 한도는 64 (`MAX_CAMERA_KEYS`, `MapTool_CameraShots.cpp:109`).
- 저작 문서: revision 1 → 2, 키 91 → 411, 13샷·5컷신 개수 불변. 컷신 절은 바이트 그대로 되붙였다. 키 수가 늘어난 것은 원본 곡선을 따르기 위한 것이다.

**결과**: 이전은 revision 1, 이후는 revision 2 를 같은 독립 비교기로 잰 값이다.

| 샷 | 키 | 이전 eye / 전방 / 방향(up) / fovY | 이후 eye / 전방 / 방향(up) / fovY | 판정 |
|---|---|---|---|---|
| gate1 cut01 | 8→64 | 4.203 m / 0.00 / 0.00 / 0.00 | 0.139 / 0.00 / 0.00 / 0.00 | **한도 초과 남음** |
| entrance cut01 | 5→24 | 1.189 / 4.65 / 10.89 / 0.00 | 0.045 / 0.38 / 10.02 / 0.00 | 위치·전방 OK, roll 남음 |
| entrance cut02 | 12→64 | 1.947 / 2.75 / 20.24 / 0.00 | 0.211 / 1.40 / 20.09 / 0.00 | **한도 초과 남음**, roll 남음 |
| entrance cut03 | 2→6 | 0.174 / 0.00 / 0.01 / 0.00 | 0.033 / 0.00 / 0.01 / 0.00 | OK |
| finale cut01 | 2→6 | 0.564 / 0.43 / 0.44 / 0.00 | 0.043 / 0.03 / 0.03 / 0.00 | OK |
| finale cut02 | 4→13 | 0.082 / 2.37 / 2.37 / 0.00 | 0.013 / 0.41 / 0.41 / 0.00 | OK |
| finale cut03 | 23→48 | 0.285 / 1.50 / 1.52 / 0.00 | 0.044 / 0.43 / 0.43 / 0.00 | OK |
| finale cut04 | 2→13 | 1.145 / 0.59 / 0.59 / 0.00 | 0.047 / 0.02 / 0.02 / 0.00 | OK |
| trash cut01 | 6→12 | 0.087 / 2.00 / 2.00 / 0.00 | 0.046 / 0.35 / 0.36 / 0.00 | OK |
| trash cut02 | 13→64 | 0.293 / 3.09 / 3.39 / 2.15 | 0.071 / 0.51 / 0.56 / 0.21 | **한도 초과 남음** |
| roar cut01 | 6→29 | 0.388 / 5.48 / 5.48 / 0.00 | 0.040 / 0.50 / 0.50 / 0.00 | OK |
| roar cut02 | 5→53 | 0.430 / 0.72 / 0.89 / 33.19 | 0.050 / 0.09 / 0.13 / 0.16 | OK |
| roar cut03 | 3→15 | 0.046 / 5.39 / 5.39 / 0.00 | 0.005 / 0.40 / 0.40 / 0.00 | OK |

- 키 지점 자체의 오차는 전부 0.04 m 이하였다. 큰 오차는 모두 **키 사이**에서 났다. 원인은 원본 Hermite(접선×구간)와 이동 부모를 균일 Catmull-Rom 이 따르지 못한 것이다.
- 컷 시작 오프셋(−0.47~+0.40 ms)은 편집기 시계가 정수 ms 에서 시작하기 때문이다.
- 이전 판단 정정: 포효 cut03 의 "FOV 26°" 는 원본 컷 시작 3621.394 ms 이전의 틈을 잘못 평가한 artifact 였다. 원본을 max(원본 시작, t) 로 평가하도록 고쳤다.

**roll 서술 정정**
- 09-17 계획서 85행의 "`up`/`roll` 불가" 는 전투 `ValtanCinematicCameraDocument.cpp:508, 594` 의 정확 객체 검사에만 해당한다.
- Map Tool camerashots 는 키마다 선택적 `up` 을 읽고 쓴다(`MapTool_CameraShots.cpp:766-770, 1089`). 샘플러도 `up` 을 소비한다(`ValtanCinematicCameraController.cpp:69-92, 333-335`).
- 현재 발탄 문서의 `up` 키는 0개다. 즉 "엔진 미지원" 이 아니라 "키에 넣지 않은 것" 이다.
- 부호 있는 원본 roll: entrance cut01 −9.98°~+10.02°, cut02 +19.86°~+20.00°, 나머지 |roll| ≤ 0.04°.

### 이번 수정 — World 배우 (`g04_actor_refit.py`, 검증 `g04_actor_verify.py`)

원본은 게임 패키지에서 활성 Move/AnimControl 트랙을 직접 읽었다(`g04_actor_tracks_probe.py` → `g04_actor_tracks_probe.json`, 비활성 트랙 제외). 1 ms 격자에서 player 코드 경로(`Sample_Track`, `Find_AnimationTrackAt`, `Try_SampleAnimationTicks`)와 비교했다.

**yaw −90 한 번 검증 (수정 전후 동일)**
- 7템플릿 모든 보이는 키에서 키 yaw − 원본 UE yaw = −89.996° ~ −90.005° (재생성 후 −90.000°). **중복 적용 없음.**
- 근거: 본체·유령 모두 제품 `CBody_Valtan::Initialize` 의 `Rotation(0,-90,0)` 한 번(`Body_Valtan.cpp:50`, 유령도 같은 클래스 `Valtan.cpp:3913`). 루가루는 MonsterCatalog −90. `CWorldSequenceObject` 의 pre-transform 은 스케일뿐이다(`WorldSequencePlayer_Objects.cpp:476, 499`).

**원본과 대조해 확정한 변환 오류**

1. **13000→13100 ms 12 m 상승 = 변환 오류.**
   - 원본 마수군단장발탄 Move 는 0~13.0592 s 동안 바닥 12 m 아래(y 10.997)에 있다가 `cim_constant` 로 **순간** 바닥(y 23.086)에 붙는다. 상승 동작은 없다. 부착(base)도 없다.
   - 0.1 s 표본을 선형 보간해서 100 ms 상승이 생겼고, 13060 ms 에서 4.835 m 오차가 났다. 13000~13059 ms 동안 땅속 몸이 보였다(60 ms).
   - 수정: 본체는 13060 ms 에 바닥에서 나타나고 무채색은 같은 순간 숨는다(1 ms 숨김 다리 키).
2. **18.645 s 는 "가라앉고 솟는 교차" 가 아니다.** 마수군단장발탄과 발탄동기화가 같은 XY(차 0.5 mm)·같은 yaw 로 순간 교대한다(둘 다 `cim_constant`). 09-18 ACTOR_PLAYBACK RESULT 67행의 서술을 정정한다. 이음 없이 한 트랙으로 이었다(yaw 오차 6.09° → 0.42°).
3. **슬롯 단일 체인화의 오류.**
   - UE3 층 구조: up_a·up_b 위, a 위, b 는 weight 1 고정. 실효 weight 최대 클립을 원본 위상 그대로 이어 붙였다(`sourceStartMs` = 전환 순간의 원본 위상).
   - loop 클립에 중간 위상으로 들어가면 첫 wrap 에서 행을 나눈다. player 는 `[sourceStart, 끝]` 만 반복하기 때문이다(`WorldSequenceDocument.cpp:2074-2076`).
   - 확정 오류 목록:
     - 최후: `att_battle_5_01_start`(up_b) 는 weight 가 전 구간 0 인데 5566~7476 ms 동안 표시됐다.
     - 최후: `dead_1` 은 9.71 s 까지 weight 0 인데 8609 ms 부터 표시됐다. 실제 우세 시작은 10080 ms 이고, 그때 위상은 1.471 s 다.
     - 최후: 합계 weight 0 표시 3154 ms 였다.
     - 버러지: 우세 클립 `att_battle_17_loop`(b, ×0.3) 가 596~2480 ms 동안 빠져 있었다. 그 구간은 0.667 s 짜리 `att_battle_13_02` 의 마지막 프레임에 1.6 s 멈춰 있었다(weight 0 표시 1640 ms).
     - 늑대·등장·포효: 전환이 weight 교차가 아니라 페이드 시작 시각에 일어났다(각 86 / 11 / 178 / 216 / 9 ms 의 weight 0 표시).
4. **다른 `cim_constant` 계단도 같은 원인.**
   - 포효 6931 ms: 원본은 공중(y 85.2)에서 바닥(y 23.16)으로 순간이동한다. 기존 키는 100 ms 낙하 선을 그렸다(42.8 m 오차).
   - 흰늑대: 3567 ms 등장이 33 ms 늦었고, 4.5~4.83 s 의 180° 회전 곡선에서 8.55° 오차가 났다.
   - 검늑대: 10267 ms 복귀가 32 ms 늦었다.

**재생성 결과** (저작본 revision 1 → 2, 리소스/템플릿/인스턴스 3/7/7·인스턴스 앵커·템플릿의 다른 필드 불변, LF·BOM 없음 유지)

| 템플릿 | 키 | 클립 행 | 위치 최대 | 방향 최대 | 가시 불일치 | 클립 불일치(weight 0 표시) |
|---|---|---|---|---|---|---|
| gate1-entrance(흰늑대) | 18→44 | 5→5 | 3.013 m → 0.018 m | 8.55° → 0.33° | 33 → 1 ms | 719(86) → 0 ms |
| gate1-entrance.black-wolf | 11→13 | 3→3 | 0.037 → 0.017 | 0.00 → 0.00 | 32 → 1 | 84(11) → 0 |
| entrance(본체) | 24→19 | 8→10 | 4.835 → 0.003 | 6.09 → 0.42 | 60 → 0 | 1508(178) → 0 |
| entrance.colorless | 6→4 | 8→10 | 0.000 → 0.000 | 0.00 → 0.00 | 61 → 1 | 1825(216) → 0 |
| finale | 2→2 | 6→6 | 0 → 0 | 0 → 0 | 0 → 0 | 4153(3154) → 0 |
| trash | 4→3 | 2→3 | 0 → 0 | 0 → 0 | 2 → 0 | 1885(1640) → 0 |
| roar | 60→13 | 2→2 | 42.816 → 0.017 | 0.00 → 0.00 | 0 → 1 | 78(9) → 0 |

- 남은 가시 불일치 1 ms 는 순간이동 직전의 숨김 다리 키다. 1 ms 선분이 보이지 않게 일부러 둔 것이다.
- 새 체인 요약:
  - 등장: 0 idle_normal_1 → 3452 abn_groggy_1_loop(346) → 4439 루프 이어짐 → 5255 abn_groggy_1_end(105) → 11083 idle_battle_1(490) → 12698 idle_normal_1(639) → 15567 att_battle_5_01_end(508) → 19069 walk_normal_1(424, 원본 역재생) → 20069 idle_battle_1(513) → 22114 루프 이어짐.
  - 최후: 0 abn_groggy_1_start → 1828 abn_groggy_1_loop → 4805 abn_groggy_1_end(230) → 7416 abn_groggy_1_loop(255) → 7865 evt1_att_battle_5_01_end(272) → 10080 dead_1(1471).
  - 버러지: 0 att_battle_13_02 → 596 att_battle_17_loop(179) → 2481 att_battle_13_02-1(207).
  - 포효: 0 att_battle_12_04(2644) → 79 att_battle_12_05(78).

### 자동 검증 (명령·exit code)

| 명령 | 결과 |
|---|---|
| `python -B g04_camera_refit.py --write` | 13샷 기록, sceneId 고유·첫 키 0·끝 키 = duration·순증가·≤64 assert 통과 |
| `python -B g04_camera_dense_compare.py` (독립 재측정) | 위 표의 "이후" 값. 10/13 허용오차 안, 3샷 초과 |
| 같은 비교기를 기준선 revision 1 에 실행 | 위 표의 "이전" 값 (`g04_camera_errors_before.json`) |
| `python -B g04_actor_refit.py --write` | 7템플릿 기록. C++ 검증 규칙 미러 assert 통과(키 2~256·첫 0·끝 = duration·순증가, 체인 첫 0·순증가·start < duration·rate 0.05~8·sourceStart ≤ 클립 길이(loop 는 <)·트랙 합계 ≤ 32). 다른 필드 동일성 assert 통과 |
| `python -B g04_actor_verify.py` (쓴 파일 재측정) | **exit 0, RESULT PASS**. 위치 ≤ 0.018 m, 방향 ≤ 0.42°, 클립 불일치 0 ms, 키 yaw − UE yaw = −90.000° |
| `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Scope WorldSequences -Mode Validate` | **exit 0**. 런타임 `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.worldsequences.json` 은 12:09 revision 1 그대로(게시 안 함) |
| JSON 두 개 행 끝 공백·탭 | 0줄 |

클립 길이는 설치된 WModel 에서 읽었다(`clip_durations.json`: 본체·유령 AnimSet 146클립, 루가루 91클립). 원본 UE3 시퀀스 길이와 같다고 본 것은 추론이다(같은 PSA 에서 cook).

### 사용자 확인 (미실시)

아래 절차 3·4·8.

### 미지원·추가 승인 필요

- **카메라 64키 한도 초과 3샷**: gate1 cut01 (0.139 m), entrance cut02 (0.211 m / 전방 1.40°), trash cut02 (0.071 m / 0.51° / fov 0.21°). 한도 상향 또는 비균일·Hermite 샘플러 같은 계약 변경이 필요하다. 조용히 줄이지 않았다.
- **roll**: entrance cut01 ±10°, cut02 약 20°. `up` 키 경로는 이미 있으므로 추가만 하면 된다. 과거 "roll 생략" 결정을 바꾸는 일이라 별도 승인 후보로 둔다.
- **역재생**: 등장 `walk_normal_1` 은 원본 역재생(breverse)이다. 우세 구간 19069~20069 ms 동안 정방향으로 재생된다. 계약에 reverse·부호 있는 rate 필드가 없다(rate 0.05~8). 필요하면 문서·검증·player 에 필드를 추가해야 한다.
- **상체 슬롯**: 최후 `up_a`(abn_groggy_1_end ×0.6) 는 이름과 `H_Up` 스켈 컨트롤로 보아 상체 전용으로 **추정**된다. 패키지 컴포넌트에 `animtreetemplate` 가 없어 뼈 마스크를 데이터로 확인하지 못했다. player 에 뼈 단위 블렌딩이 없으므로 기존 체인처럼 전신으로 표시한다. 4805~7416 ms 는 전신 groggy_end, 7416~7865 ms 는 groggy_loop 복귀다(원본 상체 weight 그대로). 원본 하체는 이 구간에 groggy_loop 를 유지한다.
- **무채색 대역**: 원본은 `mn_rpbf_00_sk` + 무기 `wp_mn_rpbf_00_sk` 이고 저장소에 없다. 유령 몸체(program 84 반투명)로 대역한다. 보고만 한다.
  - 원본은 재질 파라미터 `dead` 로 사라지고 나타난다: 무채색 0~4.0 s 는 dead=1(보이지 않음), 4.0~5.57 s 에 나타남, 12.32~15.06 s 에 사라짐. 본체는 12.06~15.06 s 에 나타난다. 두 배우가 반씩 겹치는 시점은 약 13.56~13.69 s 다.
  - dead=1 = 사라짐 은 최후·등장 곡선의 일관성에서 나온 **추론**이다. 셰이더로 확인하지 않았다. 가시성 키를 이 추론으로 바꾸지 않았다.
- **무기**: 원본 발탄 배우는 모두 `b_wp_r_01` 본에 스켈레탈 부착물이 있다. 본체·최후는 `wp_mn_rpbf_01_sk`, 무채색은 `wp_mn_rpbf_00_sk` 다. 현재 World 배우는 무기가 없다. World 계약의 `attachmentBone` 은 collider 전용이다(`WorldSequenceDocument.h:231`). 오브젝트를 본에 붙이려면 계약 추가와 리소스 추가(3/7/7 변경)가 필요하다. 제품 자산 `Character/Valtan/ValtanWeapon.wmodel` 과 BossCatalog 의 무기 preScale·preRotation 은 있다.
- **반투명 경로(진단만, 코드 무변경)**
  - `CWorldSequenceObject` 는 program 84 메시를 NONBLEND 에서 빼고 BLEND 패스로 그린다(`WorldSequenceObject.cpp:18-24, 147-148, 178, 196-219`).
  - 실패는 `m_TranslucentRenderStatus` → `Get_RenderStatus()` 로 다음 샘플에 올라온다(`WorldSequencePlayer_Objects.cpp:976`). 그러면 G01 의 전체 해제와 "World actor failed at N ms" 배너로 이어진다. 추가 로그는 필요 없다고 판단했다.

### 내 실수

- 이전 세션 생성기(같은 job)의 실수를 이번에 원본으로 확인해 바로잡았다.
  - 0.1 s 표본을 `cim_constant` 계단 위로 선형 보간했다(12 m 상승, 42.8 m 낙하 선).
  - 슬롯을 시작 시각 순으로 합쳐 weight 0 클립을 보이게 했다.
  - 18.645 s 교대를 "가라앉고 솟음" 으로 잘못 적었다.
- 카메라 첫 원본 평가기는 부모에 붙은 카메라에서 100~200 m 잔차를 냈다. 강체 맞춤도 부분 해결에 그쳐, 추출기의 `world_pose` dense 평가로 바꿨다.
- 포효 cut03 "FOV 26°" 를 처음엔 실제 오차로 보았다. 원본 컷 시작 이전 구간의 평가 artifact 였다.
- bash heredoc 안의 작은따옴표로 스크립트 작성이 깨졌다(알려진 반복 gotcha). Write 도구로 다시 썼다.
- 배우 재생성 dry run 중 네 가지를 잡았다: 기준선 경로 오타, 무채색 교대 시각 조회 KeyError, 검늑대 첫 키 이전(prestart) 구간을 같은 행으로 묶어 생긴 3.067 s 위상 오차, 위치 오차를 player 가 숨긴 프레임까지 세어 과대(83 m)로 낸 지표. 모두 문서를 쓰기 전에 고쳤다.

---

## 변경 파일

| 파일 | 변경 (기준선 대비) |
|---|---|
| `Client/Public/MapTool.h` | +54 (398-412, 457-469, 812-814, 992-998, 1017-1032) |
| `Client/Public/WorldSequenceToolPanel.h` | +28 (90-117) |
| `Client/Public/WorldSequencePlayer.h` | +5 (112-116) |
| `Client/Private/MapTool_Cutscenes.cpp` | +205 −67 (50-126, 128-157, 160-446) |
| `Client/Private/MapTool_CameraShots.cpp` | +889 −102 (8-11, 596-1039, 1136-1528, 1580-2015, 2091-2104) |
| `Client/Private/MapTool.cpp` | +3 (97-99) |
| `Client/Private/MapTool_Area.cpp` | +9 (687-689, 819-821, 1321-1323) |
| `Client/Private/MainApp.cpp` | +4 −1 (2684-2687, ASCII) |
| `Client/Private/WorldSequenceToolPanel.cpp` | +60 (487-546) |
| `Client/Private/WorldSequencePlayer_Objects.cpp` | +42 (245-286) |
| `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.camerashots.json` | revision 2, shots 키만 (91→411), 컷신 절 바이트 동일, LF |
| `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json` | revision 2, 7템플릿의 `tracks[0].keys`·`animationTracks` 만, LF |

- 새 C++ 파일·vcxproj·filters 변경은 없다.
- fork A·C 소유 파일(deployplacements·destructionsimulation·ValtanWorldEvents·Gameplay.world·Navigation·Tools/WorldPipeline·Publish-MapAuthoring·NavigationPipeline, Bern·CharSelect 네비·NPC)은 편집하지 않았다. `Publish-MapAuthoring.ps1` 은 Validate 실행만 했다.

## 사용자 확인 절차 (화면 판정은 사용자)

1. `Client.exe` 를 종료한다. VS 에서 Debug|x64 **Build**(Rebuild 아님)를 한다. 바뀐 것은 Client 소스뿐이다. 에이전트 결과는 격리 /Zs 문법 검사까지라 **VS 증분 Build 없이는 완료가 아니다.**
2. 빌드 중에 Map Tool 을 열어 둔 Client 가 있었다면 그 세션에서는 Save 하지 않는다. 디스크 문서가 revision 2 로 바뀌어 기준선 충돌로 거절된다. 새로 실행하면 revision 2 를 읽는다.
3. Lobby → Valtan → F1 → Map Tool → 발탄 Area → Camera → 컷신 콤보 "발탄 포효" → Play.
   - 기대: 요약 "World 등록 1개 · 활성 1개", "배우 출처: 저작본 (Data/Maps/Authoring, 저장된 상태)", 배우 표 상태 "활성 - 표시".
   - 0.079 s 에 att_battle_12_05 로 바뀐다. 카메라는 5.0 s 뒤 반납된다.
   - 6.931 s 에 발탄이 공중에서 바닥으로 **즉시** 옮겨진다(이전의 내려오는 선 없음).
4. 실패 시 기대 표시: 빨간 배너와 "World instance could not start: <instanceId> - <이유>" 또는 "World actor failed at N ms: <instanceId> - <이유>". 이때 모든 배우가 해제되고 Resume 은 카메라만 이어 간다.
5. 포효 왕복: 배우 표에서 포효를 선택해 startDelayMs 를 0 → 100 으로 바꾼다.
   - "미저장: 카메라 + World" 표시를 확인하고 "컷신 저장 (카메라 + World 저작본)" 을 누른다.
   - 기대: "저장했습니다 (저작본만, 게시 안 함): …".
   - 이어서 Reload Shots 를 누르고 값이 유지되는지 확인한 뒤, 되돌리고 다시 저장한다.
   - 첫 Save 는 두 파일의 숫자 서식을 C++ writer 형식으로 바꾼다(값 동일).
6. 콤보를 다른 컷신으로 바꾸면 "이전 컷신 미리보기를 정지했습니다." 가 떠야 한다.
7. 나머지 4종 재생:
   - 발탄 등장: 13.06 s 에 본체가 바닥에서 바로 나타나고 무채색이 같은 순간 사라진다(12 m 솟음 없음). 19.07~20.07 s 걷기는 정방향이다(원본은 역재생, 미지원).
   - 버러지: 0.596 s 부터 att_battle_17_loop 가 느리게(×0.3), 2.481 s 부터 13_02-1.
   - 최후: 4.805 s groggy_end(전신), 7.416 s groggy_loop, 7.865 s evt1_att_battle_5_01_end, 10.08 s dead_1(클립 1.47 s 지점부터).
   - 1관문 입장: 3.567 s 흰늑대 등장, 7.5~10.27 s 검늑대.
8. 카메라: 입장 cut02·1관문 cut01·버러지 cut02 는 원본과 아직 0.07~0.21 m 차이가 난다(한도 초과). 입장 cut01·cut02 는 roll 이 없다(승인 후보).

게시(`-Mode Publish`)는 하지 않았다. 제품 런타임의 worldsequences 는 12:09 revision 1 그대로다. 게시 여부는 사용자가 결정한다.
