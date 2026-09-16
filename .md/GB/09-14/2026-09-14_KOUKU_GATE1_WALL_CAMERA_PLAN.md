# 1관문 진입 전 벽 연출 — 원본 카메라 조사·교정·기존 시퀀스 연결 계획

## W01. 이번 목표와 확인된 출발점

사용자 요청: 팀장이 연결한 1관문 진입 전 벽 넘어가는 연출에서 카메라를 원작에 맞춰 고치고 같은 시퀀스에서 재생되게 한다. **벽 동작 재제작이나 2관문 작업은 이 문서의 실행 대상이 아니다.**

실제 프로젝트 루트: `C:/Users/USER/source/졸업팀폴/LostArk`. 다른 checkout의 오래된 코드·JSON으로 설치하지 않는다. 이 문서는 조사 후 확인된 차이만 고치는 실행계획이며, 원본 카메라가 확인되기 전 임의 좌표 패치를 제시하지 않는다. 구체적 변경이 확정되면 이 문서에 대상 stable ID와 교체 가능한 전체 데이터 블록을 보강한 뒤 적용한다. 필요한 C++ 수정은 실제 함수/호출자/실패 소비자와 저장소 규칙의 코드 전문을 먼저 기록한다.

### 실제로 확인한 것

- 원본 벽은 `37081_113 / SCENE03A / Matinee7`, export 370 / InterpData 862, 길이 약 9.518479초의 `b01` 그룹이다.
- 해당 추출 JSON의 61개 그룹에는 **Director 트랙과 카메라 그룹이 없다.** 이는 이 Matinee에 카메라가 없다는 증거이지, 원작 게임이 별도 카메라를 사용하지 않았다는 증명은 아니다.
- 같은 SCENE03A의 `Matinee2 / 마티니 카메라01`에는 dmy/cm01, 약 5.003863초 이동과 FOV가 있다. 추출 요약에는 들어오는 Kismet 연결이 없다. **가까운 곳을 본다는 이유로 벽 연출의 정답으로 채택하지 않는다.**
- 현재 `1Stage.finale` 샷은 cameraTrack 없는 정지 eye/lookAt, FOV Y 60°, blendIn 6000ms, hold 3000ms, blendOut 1500ms, activation AUTO, `world.sequence.instance.circusfinale` 연결이다. `kouku.gate1.authored.finale`도 같은 정지 구도/블렌드이며 PATTERN_ONLY다. 따라서 원본 곡선을 직접 가진 샷이라고 볼 근거는 없다. 화면 이동이 정지 샷 진입 블렌드에서 생길 가능성을 검사한다.
- 기존 벽 복구 기록에서 Stage1_wall과 circus_finale의 앞·뒤판은 0~3000ms에 180°로 넘어가도록 수정됐다. 이전 90°/6050ms 동작으로 되돌리지 않는다. 이 기록의 마지막 숨김 처리와 원본 hide 없음은 동일하다고 단정하지 않으며 이번 카메라 작업에서는 보존한다.

## W02. 원본과 현재 진입점을 먼저 연결한다

읽을 근거:

- `C:/Users/USER/OneDrive/바탕 화면/쿠크1관문_연출_원본_20260913/01_컷신별_타임라인/1관문_113_광장폭죽_SCENE03A_matinee7_전체.json`
- 같은 폴더의 `1관문_미연결_카메라01_SCENE03A_matinee2_전체.json` 및 두 요약 파일.
- 원본 패키지 `B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8T6.upk`. 추출물과 실제 설치 패키지의 scene/export/트리거 identity를 대조한다.
- `C:/Users/USER/OneDrive/바탕 화면/벽넘어가는거 .mp4` 및 `벽넘어가는거(내거).mp4`. 이번 계획 작성에서는 재분석하지 않았으며 구현자가 첨부 녹화를 오프라인 비교한다.
- `C:/Users/USER/source/졸업팀폴/LostArk/.md/GB/09-14/2026-09-14_KOUKU_STAGE1_WALL_ORIGINAL_RESTORE_RESULT.md` W-R1~W-R6.

현재 연결 조사에서 다음 경로가 있다. 팀장이 최종 연결한 항목을 실제 참조로 확정하고, 모두를 일괄 바꾸지 않는다.

| 현재 경로 | 확인된 연결 |
|---|---|
| Map Tool의 Stage1_wall | `world.sequence.instance.2`, MAP 앞판3/뒤판419의 개별 벽 동작 |
| 묶음 맵 동작 circus_finale | `world.sequence.instance.circusfinale`, 앞판 obj01/뒤판 obj24, template 길이21010ms |
| Gameplay `1Stage_Final` | playSequence로 circusfinale 참조 |
| Sequence P2 `연출_1관문 피날레` | `kakulsaydon.g1.presentation.44` → `1Stage.finale`; 현재 총21010ms |
| Sequence P4 `1관문_통합_시퀀스` | `kakulsaydon.g1.presentation.45` → `kouku.gate1.authored.finale`; 첫 박스12258ms. 전체 길이는 사용자 편집값으로 재확인 |

Sequence 문서의 `kakulsaydon.g1.world.14`는 피날레 맵이지만, 다른 Boss Composition의 같은 문자열은 다른 의미일 수 있다. **문서 종류+ID**로 조회한다. `1관문_연출` 같은 표시명만 보고 다른 Pattern을 교체하지 않는다. 현재 저장본만으로 사용자가 누른 항목을 유일하게 식별할 수 없다면 패턴/박스 이름 한 가지만 확인하고 설치를 멈춘다. 원본 조사와 후보 작업은 계속할 수 있다.

## W03. 원본 카메라 출처를 확정하는 조사

1. `37081_113`의 TriggerMapData, RemoteEvent, Kismet의 진입/완료 및 같은 시각 병렬 동작에서 카메라 관련 action/volume/설정 참조를 추적한다. SCENE03A 밖의 참조나 게임 기본 follow camera 사용 여부도 확인한다. 이름 유사성으로 연결하지 않는다.
2. 원본에 직접 연결된 카메라 트랙이 있으면 source camera/group/활성 track, parent, 시작 지연, 원본 eye/orientation/FOV와 벽 b01의 시계를 함께 추출한다.
3. 카메라01 Matinee2는 실제 연결 또는 원본 재생 근거가 확인될 때만 채택한다. 비활성 Move나 FOV=0인 dummy를 실제 촬영 카메라로 쓰지 않는다.
4. 원작이 follow camera+이벤트 보간을 사용했다면 그 동작을 복구 대상으로 삼는다. 원본에 없는 Matinee 궤적을 만들어 원본 추출이라고 부르지 않는다.
5. 안전한 조사로 연결을 찾지 못하면 조사한 참조와 한계를 보고한다. 사용자가 원본 영상 기반 수동 재현을 승인한 뒤에만 별도의 보정값으로 작성한다. 원본 카메라 미확정을 가짜 좌표나 무한 재검색으로 숨기지 않는다.

판정표에는 `직접 원본 연결 / 별도 게임 카메라 의미 확인 / 영상 기반 승인 재현 / 근거 미확정`을 구분한다. 같은 파서의 두 결과 일치만으로 원작 의미를 증명하지 않는다.

## W04. 카메라만 후보로 수정한다

원본 벽 시작을 기준으로 녹화 시간을 맞춘다. 창·HUD·유튜브 바를 뺀 고정 viewport를 사용하고, 프레임마다 확대/crop을 바꿔 일치를 만들지 않는다. 9.518초 원본 연출, 3초 벽 움직임, 21.010초 circus_finale, 사용자 P4 전체 길이는 별개다. 전체 시퀀스를 원본 길이로 일괄 압축하지 않는다.

원본→현재 시각 매핑을 먼저 작성해 벽 시작/평평해지는 시점/완료와 카메라의 관계를 유지한다. 후속 팝업북·흡입·암전·포탈의 박스나 시각은 변경하지 않는다. 길이 충돌로 후속 연출 수정이 필요하면 선택지를 보고하고 승인받는다.

후보는 camera eye/lookAt/up/roll/FOV, 보간/접선, 진입·유지·복귀 및 박스 연결을 포함한다. UE→프로젝트 basis·cm→m·parent 변환은 한 번만 적용하고 원본 수평 FOV와 런타임 수직 FOV/실제 aspect를 구분한다. 정지 pose를 Capture하여 원본 이동 트랙을 지우지 않는다.

기존 `Tools/KoukuSaydonPipeline/build_gate2_intro_composition.py`의 source curve/world_pose/make_cameras 또는 현재 source camera 생성 경로를 재사용한다. 단, make_cameras는 활성 Director가 정확히 하나라고 가정하므로 **Director 없는113을 그대로 넣으면 안 된다.** 외부 카메라 출처가 확인된 경우 필요한 좁은 추출/후보 처리만 추가하고 전체 생성기 --install은 실행하지 않는다.

카메라 AUTO 영역과 Sequence PATTERN_ONLY의 동시 소유, 우선순위, 원래 follow/free 카메라와 블렌드/복귀를 검사한다. 원본과 다른 화면 이동이 6000ms 블렌드 때문인지 실제 runtime camera pose로 확인하고, 근거 없이 전역 blend를0으로 바꾸지 않는다. 선택된 경로에 필요한 샷만 교체하거나 전용 stable shot으로 분리한다. 공유 샷을 수정한다면 모든 소비자가 이번 범위인지 확인한다.

## W05. 파일과 보호 경계

| 파일(실제 루트 기준) | 허용되는 역할 |
|---|---|
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json` | 확인된 벽 연출 카메라의 키·타이밍·전용 샷 |
| `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` | 팀장 연결 항목의 CAMERA 참조/구간만. 대상 Pattern/occurrence를 먼저 확정 |
| `Tools/KoukuSaydonPipeline/`의 해당 기존 생성기 | 확인된 source camera 후보 생성과 선택 병합 |
| `Client/Private/KoukuSaydonActionWorkbench.cpp`, `MainApp.cpp`, 기존 cinematic camera 소비자와 Map Tool 카메라 편집기 | 재생 오류의 최소 수정 및 W07의 사용자 편집·저장 연결. 기존 편집기를 재사용 |
| World/placement/Gameplay 문서 | 이번에는 연결 확인과 전후 비교용. 벽 움직임·MAP3/419·트리거는 보존 |

전등·암전·재질·FX·2관문·다른 관문·후속 팝업북 수정은 제외한다. C++/모델/셰이더 신규 파일은 기본 계획에 없다. 신규 C++가 필요해지면 .vcxproj/.filters 등록과 영향 빌드까지 같은 변경에 포함한다.

## W06. 적용·검증·팀장 확인

현재 다른 작업의 미커밋 MainApp/Level_KakulSaydonArena_WorldObjects/헤더 변경을 보존한다. 원본과 후보를 별도 보관하고 사용자의 Save/draft 상태를 확인한 뒤 대상 baseline이 같은 경우에만 적용한다. 외부 저장이 바뀌면 다시 병합하고 전체 파일 복원으로 타 작업을 지우지 않는다. 두 계획은 같은 Camera/Composition을 사용할 수 있으므로 **동시 설치하지 않고 첫 작업 저장 완료 후 두 번째가 최신본을 읽는다.**

JSON parse, 실제 Camera/Sequence reader의 stable ID/키/구간/참조, 타 Pattern 불변, diff --check를 확인한다. 카메라가 바뀌므로 WorldSequences-only 게시로 끝내지 않는다. 아래 명령은 실제 프로젝트 루트에서 변경 데이터 준비 뒤 사용한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Publish
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Check
```

Area 게시의 타 layer 반영도 diff로 확인하고 팀장 작업을 예기치 않게 함께 적용하지 않는다. runtime DataFiles 직접 편집, 전체 재생성, shader 강제 제외는 금지다. 데이터만 바뀌면 EXE 빌드는 불필요하다. C++가 바뀐 경우에만 현재 Product 증분 Build를 사용하며 Rebuild/Clean/OBJ/PCH/tlog 삭제를 하지 않는다.

사용자가 F1 → Action Workbench → Composition Actions → Sequence에서 **W02에서 확정한 기존 항목**을 재생한다. 벽 시작 직전/0·1.5·3초의 상대 구도, 첫 시점과 후퇴/전환, 중간 Seek, Stop/Reset, 두 번째 Play, Save Camera/Sequence Save 후 재로드를 확인한다. 기존 후속 연출과 전투 카메라 복귀가 유지돼야 한다. Map Tool Stage1_wall 단독 Play는 카메라 전체 연결 검증의 대체물이 아니다.

Client/UI 실행·조작·화면 캡처는 사용자가 한다. 에이전트는 데이터/코드 및 필요한 컴파일·로그까지 준비한다. 검증 결과는 같은 이름의 RESULT에 원본 출처, 대상 Pattern/shot, source→Sequence 시각표, 변경값, 자동 검사, 사용자 판정 대기, 정확한 편집/저장 경로를 분리해 기록한다. 원본 카메라를 못 찾았으면 찾았다고 쓰지 않고, 영상 재현은 명확히 별도 표기한다.

## W07. 사용자 카메라 편집까지 연결한다 — 추가 필수 범위

이번 인계는 원본 카메라를 설치하고 재생시키는 것으로 끝나지 않는다. 사용자가 1관문 벽 연출을 선택한 뒤 해당 카메라를 직접 수정하고 저장하여 같은 전체 시퀀스에서 확인할 수 있어야 한다. 2관문만 편집 가능하게 만들고 1관문을 재생 전용으로 남기지 않는다.

Action Workbench의 기존 카메라 편집기를 우선 사용한다. 필요한 편집이 불가능하면 Map Tool의 카메라 편집기로 연결하고 부족한 기능을 구현한다. 둘 중 한 도구에서 실질적인 편집을 끝낼 수 있어야 한다는 요구이며, 두 도구에 독립 편집기와 저장 사본을 복제하라는 요구는 아니다. 양쪽 진입점이 있다면 동일한 Area/shot 정본과 저장 결과를 소비해야 한다. 단순히 “Map Tool에서 하세요”라고 안내하고 연결되지 않은 상태로 종료하지 않는다.

### 편집 동작과 보호 조건

1. W02에서 확정한 벽 연출의 CAMERA 박스를 선택하면 해당 stable shot ID를 가진 편집 대상을 연다. 표시명이 비슷한 다른 샷이나 첫 번째 샷을 대신 선택하지 않는다. Map Tool로 이동할 때도 Area, shot, 선택 키와 시퀀스 발생 구간을 명시적으로 전달한다.
2. 카메라 키의 시각·위치·바라보는 방향/대상·FOV·roll/up, 보간 및 진입/복귀 설정을 실제 저장 형식에 맞춰 편집할 수 있어야 한다. CAMERA 박스의 시작·길이·샷 참조는 Sequence 편집에서 변경한다. 클립/샷 로컬 시각과 전체 시퀀스 시각을 함께 표시하여 혼동을 막는다.
3. 키 하나의 위치를 현재 뷰로 바꾸는 작업이 전체 cameraTrack을 정지 포즈 하나로 덮어쓰면 안 된다. 기존 Capture 동작이 전체 트랙 교체라면 명확한 별도 동작으로 구분하고, 선택 키만 수정하는 경로를 제공한다.
4. 전체 벽 연출을 Pause/Seek하여 구도를 본 상태에서 편집하고, Apply 후 같은 시각을 다시 평가하거나 전체 Play를 다시 시작하여 확인할 수 있어야 한다. 편집기 전환 때문에 연결을 잃거나 벽이 사라진 단독 카메라 미리보기만 남는 상태는 완료가 아니다. 서로 다른 preview owner를 동시에 강제 실행하지 말고 기존 transport의 안전한 중단·재평가 경계를 사용한다.
5. 공유 샷의 수정이 다른 연출에 영향을 주면 소비자 목록을 표시하고 명시적 선택을 받는다. 현재 발생만 수정하려는 경우 기존 데이터 계약에 맞춰 전용 stable shot을 만들고 해당 참조만 바꾼다. 벽의 3초180도 동작, MAP3/419 및 후속 연출은 유지한다.

### 저장·재로드·완료 검사

카메라는 해당 Area camerashots 정본, CAMERA 박스는 기존 Sequence Composition 정본에 저장한다. Map Tool과 Workbench에 별도 임시 정본을 만들지 않는다. Save Camera와 Sequence Save가 각각 무엇을 저장하는지 표시하고, 수정 안 된 문서까지 오래된 메모리 사본으로 덮어쓰지 않는다. 외부 Save/draft 충돌 시 기존 데이터를 유지하고 이유를 보여 준다. 저장된 변경은 필요한 publisher 검증/게시 및 reload를 거쳐 전체 Sequence에 반영하며, 실시간 hot reload가 지원되지 않으면 Stop → Save → Reload → 같은 시각 Seek/Play 절차를 제공한다.

자동 검증은 선택 ID 전달, 선택 키 수정 시 나머지 키 불변, Save/Reload 왕복, 잘못된 ID/저장 실패 시 기존 상태 보존, 타 Pattern·벽 동작 불변을 포함한다. 사용자는 카메라 키 하나를 소폭 수정 → 전체 연출에서 차이 확인 → Save/Reload → Client 재실행 후 유지 확인 → 원래 값 복구를 수행한다. 정확한 버튼 경로와 저장 파일을 RESULT에 적는다. 버튼 존재나 빌드 성공만으로 편집 검증을 PASS 처리하지 않는다.

편집 연결에 C++ 변경이 필요하면 이번 구현 범위에 포함하고 W06의 증분 빌드·검증을 적용한다. 실제 호출자/저장 소유자를 확인한 뒤 구체 패치를 작성한다. 원본 출처 확인, 전체 재생, 사용자 편집 가능, 저장 유지, 사용자 화면 판정을 각각 별도로 완료 보고한다.


## W08. 조사 확정 사항과 W07 편집 연결 코드 (2026-09-14 Claude)

### W02 결과 — 대상 후보 (사실)

- `KAKULSAYDON_G1_PATTERN_2` `연출_1관문 피날레`(21010ms): WORLD `.world.1`(`kakulsaydon.g1.world.14` 피날레_맵 0~21010) + CAMERA `.presentation.1` → 리소스 `kakulsaydon.g1.presentation.44` → 샷 `1Stage.finale`(0~21010). 박스는 tnestyle70 `c67a47b2`(09-10) 추가. 샷은 AUTO(`circusfinale` 인스턴스)라 게임플레이 트리거 재생도 소비한다.
- `KAKULSAYDON_G1_PATTERN_4` `1관문_통합_시퀀스`(61662ms): WORLD `.world.14`(피날레_맵 0~19959) + CAMERA `.presentation.23` → `kakulsaydon.g1.presentation.45` → 샷 `kouku.gate1.authored.finale`(0~12258), 뒤이어 포탈 `.presentation.24`(12258~16658, 샷 `kouku.gate1.authored.portal`, blend 0, 같은 정지 eye). 박스·샷 모두 tnestyle70 `3fc23750`(09-13). 이 샷의 소비자는 P4 박스 하나다.
- 두 샷의 eye/lookAt/FOV Y 60/blendIn 6000/hold 3000/blendOut 1500이 같고 cameraTrack이 없다. 저장본만으로 사용자가 누르는 항목을 하나로 정할 수 없어 설치 전 확인한다.

### W03 결과 — 원본 카메라 출처

판정: **별도 게임 카메라 의미 확인** (직접 Matinee 연결은 아님).

- 벽 Matinee7(#370) 체인에는 카메라 액션이 없다(`Kismet_SCENE03A.txt` 4·16·32·41줄). 트리거 유닛 131 액션도 VolumeProp 32 / SceneEvent 37081_113 / Prop despawn·spawn뿐이다.
- 같은 SCENE03A의 `#1738 Touch(EFMatineePathNodeVolume_0)`(maxtriggercount 0) → Touched `#1719 SetCameraTarget`(Cam Target cameraactor_22 = Matinee2 cm01, blend 2.5초), UnTouched `#1717 SetCameraTarget`(플레이어, blend 1.5초) (`Kismet_SCENE03A.txt` 24·26·45줄).
- 볼륨 export 329에 `tlinkmatinee_matinee = …EFSeqAct_Matinee_2`, `matineepathnode = 327`(경로 327 (1748,6147,6) → 328 (4882,8449,6), 38.9m). README의 "카메라01 미연결"은 Kismet 입력만 본 결과다.
- 벽을 시작하는 EFLocalTrigger_2 (3137,6935,49), 서버 VolumeProp 32 (3290,7120,−15), 벽 b01 (4544,8016,48)이 모두 볼륨 브러시 안이다.
- cm01은 dmy(0→5.004초 (1962,6235,11)→(4519,8020,11), yaw 45)에 hard attach, 상대 키 4개, 수평 FOV 50→70(3.492초). 월드 eye (1079,5353,1262)→(3765,7241,509), 벽 정면을 45.4m→11.9m로 내려오며 다가간다.
- 미확인: 볼륨이 Matinee2 시간을 플레이어 경로 진행도로 구동하는지(스크립트·exe에서 해당 클래스 코드를 찾지 못함), 진행도→시간 매핑, 볼륨 밖 시작 시 카메라. 경로 진행도 가정 시 EFLocalTrigger_2는 Matinee2 약 2.29초, VolumeProp 32는 약 2.54초.
- 참고 영상 `벽넘어가는거 .mp4`, `벽넘어가는거(내거).mp4`는 이 PC의 OneDrive·바탕 화면·동영상·다운로드·문서에서 찾지 못해 비교하지 않았다.

따라서 궤적 출처는 원본 cm01이지만 Sequence 시간축으로의 매핑은 원본이 증명하지 않는다. 매핑 방식과 설치 대상(P2/P4)은 사용자 승인 후 적용한다.

### W07 편집기 실측 — 이미 되는 것과 부족분

이미 되는 것: Box Detail `Open Composition Camera`가 박스의 stable shot ID로 창을 연다. 창에서 키별 도착 시각·Pos·pitch/yaw/roll(up)·FOV Y, Position path(Linear/Catmull-Rom), Segment easing, Entry blend/Return을 편집한다. `Capture Pos + Rot`는 같은 시각 키만 교체하거나 새 키를 추가한다. Sequence 미리보기는 매 프레임 `m_AuthoringCameraShots`(draft)를 샘플하므로(`Level_KakulSaydonArena.cpp` `Sample_CompositionCamera` preview 분기) Pause 상태에서도 편집이 같은 시각에 재평가된다. `Save_CameraShots`는 dirty 샷만 기준본에 병합해 CAS 원자 저장한다(외부 저장이 있으면 거절·draft 보존). Map Tool과 Cinematic Camera Tool도 같은 `Save_CameraShotDocumentAtomic` 기준본 검사를 쓴다.

부족분과 이번 변경:
1. 창이 어느 Sequence 박스에서 열렸는지 모른다 → Pattern/occurrence 문맥을 저장하고 박스 시작·길이, "shot T = Sequence 시작+T", 현재 미리보기 시각의 shot 로컬 시각을 표시한다. 선택 키에 `Pause Sequence at this position`(기존 PAUSE transport = Seek 후 정지, 미리보기가 없으면 기존 `Request_PatternPreview(…, startPaused)`)을 둔다.
2. 공유 샷 소비자 표시·보호가 없다 → Composition 박스(Pattern·Bundle)와 Area AUTO 소비자를 나열하고, 소비자가 2개 이상이면 `Edit this shared shot for every consumer above` 체크 전까지 편집 위젯을 비활성화한다. 박스 문맥이 있으면 `Make dedicated shot for this box`가 샷을 PATTERN_ONLY로 복제(`Duplicate_CameraShot`)하고 새 리소스로 이 occurrence의 참조만 바꾼다. Composition 커밋 실패 시 미저장 복제 샷을 되돌린다(`Discard_UnsavedCameraShot`).
3. Box Detail `Set Camera Pos / Capture view`가 트랙이 있어도 즉시 전체 트랙을 정지 포즈로 교체한다 → 트랙이 있으면 `Replace whole track with current view...` 확인 팝업으로 분리한다. 트랙 샷의 WORLD→PLAYER 앵커 전환(트랙 삭제)도 비활성화한다.
4. `Play Camera`는 카메라 단독 미리보기다 → 툴팁으로 명시하고 전체 장면은 1번 버튼을 쓰게 안내한다. 저장 대상 안내 문구를 추가한다.

Map Tool 카메라 편집기로의 전달은 추가하지 않는다. 위 편집이 Workbench 한 곳에서 끝나고 두 도구가 같은 정본·CAS를 쓰기 때문이다.

신규 C++ 파일·project/filter 변경은 없다. `KoukuSaydonActionWorkbench.cpp`(13,106줄)는 파일 전문 대신 교체 함수 전체 블록을 싣는다. 적용은 `tmp/w07/apply_w07.py`의 앵커 1회 일치·CRLF/BOM 보존 검사로 한다. 빌드는 사용자가 VS에서 수행한다.

#### `Client/Private/KoukuSaydonActionWorkbench.cpp` — 익명 namespace, `Camera_DefaultDuration` 바로 뒤에 추가

```cpp
	// Every owner that plays one stable shot: Composition boxes plus the Area AUTO trigger.
	std::vector<std::string> Camera_Consumers(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const CLevel_KakulSaydonArena::KAKUL_CAMERA_SHOT& shot)
	{
		std::vector<std::string> consumers;
		const auto collect = [&](const std::string& owner, const std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE>& rows) {
			for (const auto& row : rows)
				for (const auto& resource : document.PresentationResources)
					if (resource.strResourceId == row.strResourceId && resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA &&
						resource.strAssetId == shot.strShotId)
						consumers.push_back(owner + " / " + row.strOccurrenceId + " @ " + std::to_string(row.iStartMs) + " ms");
		};
		for (const auto& pattern : document.Patterns) collect(pattern.strDisplayName, pattern.PresentationOccurrences);
		for (const auto& bundle : document.Bundles) collect(bundle.strDisplayName, bundle.PresentationOccurrences);
		if (!shot.bPatternOnly)
			consumers.push_back(shot.strSequenceInstanceId.empty() ? std::string("Area AUTO camera box") :
				"Area AUTO with " + shot.strSequenceInstanceId);
		return consumers;
	}
```

#### 같은 파일 — `Render_CameraAuthoring` 전체 교체

```cpp
void Client::CKoukuSaydonActionWorkbench::Render_CameraAuthoring(const std::string_view shotId,
	const std::string_view patternId, const std::string_view occurrenceId)
{
	if (ImGui::Button("Open Composition Camera"))
	{
		m_strCameraWindowShotId = std::string(shotId); m_strCameraKeyId.clear(); m_bCameraWindowOpen = true;
		// A Sequence box opens the window with its clock; a Resource row opens the shot alone.
		m_strCameraWindowPatternId = std::string(patternId); m_strCameraWindowOccurrenceId = std::string(occurrenceId);
	}
	auto* level = CLevel_KakulSaydonArena::Get_Active();
	const auto* selected = Find_AuthoringCamera(shotId);
	if (!level || !selected) { ImGui::TextDisabled("Enter KoukuSaydon to edit this Area Camera shot."); return; }
	auto shot = *selected;
	const auto consumers = Camera_Consumers(m_Draft, shot);
	const bool locked = consumers.size() > 1u && m_strCameraSharedEditShotId != shot.strShotId;
	ImGui::PushID(shot.strShotId.c_str());
	ImGui::SeparatorText("Camera Shot / Area source");
	ImGui::TextDisabled("%s | %s", shot.strShotId.c_str(), shot.bPatternOnly ? "PATTERN_ONLY" : "AUTO (existing Area trigger)");
	if (locked)
		ImGui::TextWrapped("Shared by %zu consumers. Open Composition Camera to review them, then allow shared edits or make a dedicated shot for this box.", consumers.size());
	ImGui::BeginDisabled(locked);
	char name[129]{}; (void)Copy_Text(name, std::size(name), shot.strDisplayName);
	bool changed = ImGui::InputText("Camera name", name, std::size(name));
	if (changed) shot.strDisplayName = name;
	int entry = static_cast<int>(shot.iBlendInMs), hold = static_cast<int>(shot.iDefaultHoldMs), exit = static_cast<int>(shot.iBlendOutMs);
	if (ImGui::InputInt("Blend in ms", &entry)) { shot.iBlendInMs = std::clamp(entry, 0, 10000); changed = true; }
	if (ImGui::InputInt("Default hold ms", &hold)) { shot.iDefaultHoldMs = std::clamp(hold, 0, 600000); changed = true; }
	if (ImGui::InputInt("Return ms", &exit)) { shot.iBlendOutMs = std::clamp(exit, 0, 10000); changed = true; }
	int easing = shot.eTransitionEasing == VALTAN_CINEMATIC_CAMERA_EASING::LINEAR ? 0 : 1;
	if (ImGui::Combo("Transition", &easing, "LINEAR\0SMOOTHSTEP\0"))
	{ shot.eTransitionEasing = easing == 0 ? VALTAN_CINEMATIC_CAMERA_EASING::LINEAR : VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP; changed = true; }
	int anchor = shot.followsPlayer ? 1 : 0;
	// PLAYER framing drops the track, so a tracked shot keeps WORLD here.
	ImGui::BeginDisabled(shot.hasCameraTrack && !shot.followsPlayer);
	if (ImGui::Combo("Shot anchor", &anchor, "WORLD\0PLAYER\0"))
	{
		shot.followsPlayer = anchor == 1;
		if (shot.followsPlayer) { shot.hasCameraTrack = false; shot.vFollowEyeOffset = shot.vEye; shot.vFollowLookAtOffset = shot.vLookAt; }
		changed = true;
	}
	ImGui::EndDisabled();
	if (changed) (void)level->Update_CameraShot(shot, m_strStatus);
	ImGui::EndDisabled();
	ImGui::Text("Eye (%.3f, %.3f, %.3f)", shot.vEye.x, shot.vEye.y, shot.vEye.z);
	ImGui::Text("Look at (%.3f, %.3f, %.3f), FOV %.2f", shot.vLookAt.x, shot.vLookAt.y, shot.vLookAt.z, shot.fFovYDegrees);
	ImGui::BeginDisabled(locked);
	if (shot.hasCameraTrack)
	{
		ImGui::TextWrapped("This shot has a camera track. Edit one position in Composition Camera; replacing the whole track is a separate confirmed action.");
		if (ImGui::Button("Replace whole track with current view...")) ImGui::OpenPopup("Replace Camera Track##CameraAuthoring");
		if (ImGui::BeginPopupModal("Replace Camera Track##CameraAuthoring", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextWrapped("Every position of %s becomes one static pose captured from the current view.", shot.strShotId.c_str());
			if (ImGui::Button("Replace whole track"))
			{ (void)level->Capture_CameraShot(shot.strShotId, m_strStatus); ImGui::CloseCurrentPopup(); }
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}
	else if (ImGui::Button("Set Camera Pos / Capture view"))
		(void)level->Capture_CameraShot(shot.strShotId, m_strStatus);
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Save Camera"))
	{
		if (level->Save_CameraShots(m_strStatus)) m_bPresentationResourceRefreshRequested = true;
	}
	ImGui::TextWrapped("Set Camera Pos captures eye, lookAt and FOV together. PLAYER captures offsets from the local player. Save writes the Area source immediately; Preview needs no publish.");
	ImGui::TextWrapped("Append uses blend-in + default hold. The box end begins the separate return tail. Use F6 Follow before Play to keep gameplay movement and return to the moving player.");
	ImGui::PopID();
}
```

#### 같은 파일 — `Render_CameraWindow` 전체 교체와 `Make_DedicatedCameraShot` 추가

```cpp
void Client::CKoukuSaydonActionWorkbench::Render_CameraWindow()
{
	if (!m_bCameraWindowOpen) return;
	ImGui::SetNextWindowSize({ 660.f, 740.f }, ImGuiCond_FirstUseEver);
	const char* title = m_bSequenceWorkspace ? "Composition Camera###SequenceCompositionCamera" :
		"Composition Camera###ActionCompositionCamera";
	if (!ImGui::Begin(title, &m_bCameraWindowOpen)) { ImGui::End(); return; }
	auto* level = CLevel_KakulSaydonArena::Get_Active();
	if (!level)
	{ ImGui::TextWrapped("Enter KoukuSaydon to author its camera actions."); ImGui::End(); return; }
	if (ImGui::Button("Reload Cameras"))
		if (level->Reload_CameraShotAuthoring(m_strStatus)) m_bPresentationResourceRefreshRequested = true;
	if (!level->Ensure_CameraShotAuthoring(m_strStatus))
	{
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		ImGui::TextWrapped("Repair the Camera source, then use Reload Cameras to retry.");
		ImGui::End(); return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Create Camera Action")) ImGui::OpenPopup("Create Camera Action##Camera");
	if (ImGui::BeginPopupModal("Create Camera Action##Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText("Name", m_NewCameraActionName, std::size(m_NewCameraActionName));
		ImGui::TextWrapped("The current view becomes the first position. Capture later positions at their arrival times.");
		ImGui::BeginDisabled(!m_NewCameraActionName[0]);
		if (ImGui::Button("Create"))
		{
			std::string id;
			if (level->Create_CameraShot(m_NewCameraActionName, id, m_strStatus))
			{
				m_strCameraWindowShotId = id; m_strCameraKeyId.clear(); m_iCameraCaptureMs = 1000;
				m_strCameraWindowPatternId.clear(); m_strCameraWindowOccurrenceId.clear();
				m_strSelectedPresentationSourceId = std::to_string(static_cast<int>(KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)) + "::" + id;
				m_NewCameraActionName[0] = '\0'; m_bPresentationResourceRefreshRequested = true;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndDisabled(); ImGui::SameLine();
		if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Save Camera"))
		if (level->Save_CameraShots(m_strStatus)) m_bPresentationResourceRefreshRequested = true;
	const auto& shots = level->Get_CameraShots();
	const auto* selected = Find_AuthoringCamera(m_strCameraWindowShotId);
	if (ImGui::BeginCombo("Camera Action", selected ? selected->strDisplayName.c_str() : "Choose camera action"))
	{
		for (const auto& item : shots)
			if (ImGui::Selectable((item.strDisplayName + "###" + item.strShotId).c_str(), item.strShotId == m_strCameraWindowShotId))
			{
				m_strCameraWindowShotId = item.strShotId; m_strCameraKeyId.clear();
				m_strCameraWindowPatternId.clear(); m_strCameraWindowOccurrenceId.clear();
			}
		ImGui::EndCombo();
	}
	selected = Find_AuthoringCamera(m_strCameraWindowShotId);
	if (!selected) { ImGui::TextWrapped("%s", m_strStatus.c_str()); ImGui::End(); return; }
	auto shot = *selected;
	auto cue = CLevel_KakulSaydonArena::CameraShot_ToCue(shot);
	ImGui::PushID(shot.strShotId.c_str());
	ImGui::TextDisabled("%s | %zu positions | %u ms", shot.strShotId.c_str(), cue.Keyframes.size(), cue.iDurationMs);
	// Every edit below writes this stable shot, so name everything that plays it first.
	const auto consumers = Camera_Consumers(m_Draft, shot);
	const bool sharedShot = consumers.size() > 1u;
	const bool locked = sharedShot && m_strCameraSharedEditShotId != shot.strShotId;
	const auto* contextPattern = Find_Pattern(m_Draft, m_strCameraWindowPatternId);
	const auto* contextBox = contextPattern ? Find_PresentationBox(*contextPattern, m_strCameraWindowOccurrenceId) : nullptr;
	const auto* contextResource = contextBox ? Find_PresentationResource(m_Draft, contextBox->strResourceId) : nullptr;
	const bool boxContext = contextResource && contextResource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA &&
		contextResource->strAssetId == shot.strShotId;
	ImGui::Text("Played by %zu", consumers.size());
	for (const auto& consumer : consumers) ImGui::BulletText("%s", consumer.c_str());
	if (sharedShot)
	{
		bool allowShared = !locked;
		if (ImGui::Checkbox("Edit this shared shot for every consumer above", &allowShared))
			m_strCameraSharedEditShotId = allowShared ? shot.strShotId : std::string();
		if (boxContext && ImGui::Button("Make dedicated shot for this box") &&
			Make_DedicatedCameraShot(contextPattern->strPatternId, contextBox->strOccurrenceId, m_strStatus))
		{ ImGui::TextWrapped("%s", m_strStatus.c_str()); ImGui::PopID(); ImGui::End(); return; }
	}
	if (boxContext)
		ImGui::TextWrapped("Sequence box %s / %s starts at %u ms for %u ms. Shot time T plays at Sequence time %u + T ms.",
			contextPattern->strDisplayName.c_str(), contextBox->strOccurrenceId.c_str(), contextBox->iStartMs,
			contextBox->iDurationMs, contextBox->iStartMs);
	else ImGui::TextDisabled("Open from a Sequence CAMERA box to map shot time to Sequence time.");
	if (boxContext && m_PreviewState.bPlaying && m_PreviewState.strPatternId == contextPattern->strPatternId)
		ImGui::Text("Sequence preview %u ms%s = shot %lld ms", m_PreviewState.iClockMs, m_PreviewState.bPaused ? " (paused)" : "",
			static_cast<long long>(m_PreviewState.iClockMs) - static_cast<long long>(contextBox->iStartMs));
	if (shot.followsPlayer)
	{
		ImGui::TextWrapped("This action follows PLAYER. Capture a world path explicitly to replace that follow framing.");
		ImGui::BeginDisabled(locked);
		if (ImGui::Button("Convert to world camera path"))
		{
			VALTAN_CINEMATIC_CAMERA_POSE pose;
			if (CCameraTool::Capture_ViewPose(pose))
			{
				shot.followsPlayer = false; shot.hasCameraTrack = true;
				cue.Keyframes = {{ shot.strShotId + ".p1", 0u, pose.vEye, pose.vLookAt, pose.fFovYDegrees, pose.vUp, true }};
				shot.CameraTrack = cue; shot.vEye = pose.vEye; shot.vLookAt = pose.vLookAt;
				shot.fFovYDegrees = pose.fFovYDegrees;
				(void)level->Update_CameraShot(shot, m_strStatus);
			}
			else m_strStatus = "Current camera pose is unavailable; the action was preserved.";
		}
		ImGui::EndDisabled();
		ImGui::TextWrapped("%s", m_strStatus.c_str()); ImGui::PopID(); ImGui::End(); return;
	}
	bool changed = false;
	ImGui::BeginDisabled(locked);
	char name[129]{}; (void)Copy_Text(name, std::size(name), shot.strDisplayName);
	if (ImGui::InputText("Action name", name, std::size(name))) { shot.strDisplayName = name; changed = true; }
	int entry = static_cast<int>(shot.iBlendInMs), exit = static_cast<int>(shot.iBlendOutMs);
	if (ImGui::InputInt("Entry blend ms", &entry)) { shot.iBlendInMs = std::clamp(entry, 0, 10000); changed = true; }
	if (ImGui::InputInt("Return ms", &exit)) { shot.iBlendOutMs = std::clamp(exit, 0, 10000); changed = true; }
	int interpolation = static_cast<int>(cue.eInterpolation), easing = static_cast<int>(cue.eEasing);
	if (ImGui::Combo("Position path", &interpolation, "Linear\0Catmull-Rom\0"))
	{ cue.eInterpolation = static_cast<VALTAN_CINEMATIC_CAMERA_INTERPOLATION>(interpolation); changed = true; }
	if (ImGui::Combo("Segment easing", &easing, "Linear\0Smoothstep\0Hold\0"))
	{ cue.eEasing = static_cast<VALTAN_CINEMATIC_CAMERA_EASING>(easing); changed = true; }
	ImGui::InputInt("Capture arrival ms", &m_iCameraCaptureMs);
	m_iCameraCaptureMs = std::clamp(m_iCameraCaptureMs, 0, 120000);
	if (ImGui::Button("Capture Pos + Rot"))
	{
		VALTAN_CINEMATIC_CAMERA_POSE pose;
		if (!CCameraTool::Capture_ViewPose(pose)) m_strStatus = "Current camera pose is unavailable; the action was preserved.";
		else
		{
			const auto time = static_cast<std::uint32_t>(m_iCameraCaptureMs);
			auto found = std::find_if(cue.Keyframes.begin(), cue.Keyframes.end(), [time](const auto& key) { return key.iTimeMs == time; });
			if (found == cue.Keyframes.end() && cue.Keyframes.size() >= 64u) m_strStatus = "Camera position limit is 64.";
			else
			{
				VALTAN_CINEMATIC_CAMERA_KEYFRAME key;
				key.iTimeMs = time; key.vEye = pose.vEye; key.vLookAt = pose.vLookAt;
				key.fFovYDegrees = pose.fFovYDegrees; key.vUp = pose.vUp; key.hasUp = true;
				if (found != cue.Keyframes.end()) { key.strSceneId = found->strSceneId; *found = key; }
				else
				{
					for (std::uint32_t ordinal = 1u; ordinal <= 65u; ++ordinal)
					{
						key.strSceneId = shot.strShotId + ".p" + std::to_string(ordinal);
						if (std::none_of(cue.Keyframes.begin(), cue.Keyframes.end(), [&](const auto& row) { return row.strSceneId == key.strSceneId; })) break;
					}
					cue.Keyframes.push_back(key);
				}
				m_strCameraKeyId = key.strSceneId; changed = true;
				m_iCameraCaptureMs = (std::min)(120000, m_iCameraCaptureMs + 1000);
			}
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Stop camera preview")) { m_bPresentationPreviewRequestPending = false; level->Stop_CompositionCamera(true); }
	if (ImGui::BeginListBox("Positions", { -1.f, 150.f }))
	{
		for (const auto& key : cue.Keyframes)
		{
			const auto label = std::to_string(key.iTimeMs) + " ms / " + key.strSceneId;
			if (ImGui::Selectable(label.c_str(), m_strCameraKeyId == key.strSceneId)) m_strCameraKeyId = key.strSceneId;
		}
		ImGui::EndListBox();
	}
	auto key = std::find_if(cue.Keyframes.begin(), cue.Keyframes.end(), [&](const auto& item) { return item.strSceneId == m_strCameraKeyId; });
	if (key == cue.Keyframes.end() && !cue.Keyframes.empty()) { key = cue.Keyframes.begin(); m_strCameraKeyId = key->strSceneId; }
	if (boxContext && key != cue.Keyframes.end())
	{
		const auto sequenceMs = (std::min)(contextBox->iStartMs + key->iTimeMs, Pattern_DurationMs(*contextPattern));
		ImGui::Text("Selected position %u ms = Sequence %u ms", key->iTimeMs, contextBox->iStartMs + key->iTimeMs);
		// PAUSE seeks the running Sequence first; otherwise open that Sequence paused at the key.
		if (ImGui::Button("Pause Sequence at this position"))
		{
			if (m_PreviewState.bPlaying && m_PreviewState.strPatternId == contextPattern->strPatternId)
			{
				m_strCursorPatternId = contextPattern->strPatternId; m_iCursorMs = sequenceMs;
				m_iPendingSeekMs = sequenceMs; m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::PAUSE;
				m_strStatus = "Sequence paused at " + std::to_string(sequenceMs) + " ms; camera edits re-evaluate at this time.";
			}
			else (void)Request_PatternPreview(contextPattern->strPatternId, sequenceMs, m_strStatus, true);
		}
	}
	ImGui::BeginDisabled(locked);
	if (key != cue.Keyframes.end())
	{
		int time = static_cast<int>(key->iTimeMs);
		if (ImGui::InputInt("Arrival time ms", &time)) { key->iTimeMs = std::clamp(time, 0, 120000); changed = true; }
		const auto direction = XMLoadFloat3(&key->vLookAt) - XMLoadFloat3(&key->vEye);
		const auto forward = XMVector3Normalize(direction);
		float3_t look; XMStoreFloat3(&look, forward);
		const float pitch = std::asin(std::clamp(-look.y, -1.f, 1.f)), yaw = std::atan2(look.x, look.z);
		const auto basis = XMMatrixRotationRollPitchYaw(pitch, yaw, 0.f);
		const auto up = XMLoadFloat3(&key->vUp);
		const float roll = key->hasUp ? std::atan2(-XMVectorGetX(XMVector3Dot(up, basis.r[0])), XMVectorGetX(XMVector3Dot(up, basis.r[1]))) : 0.f;
		float3_t rotation{ XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll) };
		float3_t position = key->vEye;
		const bool positionChanged = ImGui::DragFloat3("Pos", &position.x, .05f);
		const bool rotationChanged = ImGui::DragFloat3("Rot / pitch yaw roll", &rotation.x, .1f);
		if (positionChanged || rotationChanged)
		{
			const auto edited = XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
			key->vEye = position;
			XMStoreFloat3(&key->vLookAt, XMLoadFloat3(&position) + edited.r[2] * XMVectorGetX(XMVector3Length(direction)));
			XMStoreFloat3(&key->vUp, edited.r[1]); key->hasUp = true; changed = true;
		}
		changed |= ImGui::DragFloat("FOV Y", &key->fFovYDegrees, .1f, 1.01f, 178.99f);
		if (key != cue.Keyframes.begin())
		{
			const auto& previous = *std::prev(key);
			const float distance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&key->vEye) - XMLoadFloat3(&previous.vEye)));
			const auto span = static_cast<int64_t>(key->iTimeMs) - previous.iTimeMs;
			ImGui::Text("Segment: %.3f units / %.3f s = %.3f units/s (straight-line average)", distance,
				span * .001, span > 0 ? distance * 1000.0 / span : 0.0);
		}
		ImGui::BeginDisabled(cue.Keyframes.size() <= 1u);
		if (ImGui::Button("Delete position"))
		{
			cue.Keyframes.erase(key); m_strCameraKeyId.clear(); changed = true;
			const auto firstTime = cue.Keyframes.front().iTimeMs;
			for (auto& row : cue.Keyframes) row.iTimeMs -= firstTime;
		}
		ImGui::EndDisabled();
	}
	if (cue.Keyframes.size() == 1u)
	{
		int duration = static_cast<int>(cue.iDurationMs);
		if (ImGui::InputInt("Static duration ms", &duration)) { cue.iDurationMs = std::clamp(duration, 1, 120000); changed = true; }
	}
	ImGui::EndDisabled();
	if (changed)
	{
		std::stable_sort(cue.Keyframes.begin(), cue.Keyframes.end(), [](const auto& left, const auto& right) { return left.iTimeMs < right.iTimeMs; });
		if (cue.Keyframes.size() > 1u) cue.iDurationMs = cue.Keyframes.back().iTimeMs;
		shot.hasCameraTrack = true; shot.CameraTrack = cue;
		shot.iDefaultHoldMs = cue.iDurationMs > shot.iBlendInMs ? cue.iDurationMs - shot.iBlendInMs : 0u;
		shot.vEye = cue.Keyframes.front().vEye; shot.vLookAt = cue.Keyframes.front().vLookAt;
		shot.fFovYDegrees = cue.Keyframes.front().fFovYDegrees;
		if (shot.iBlendInMs > cue.iDurationMs) m_strStatus = "Entry blend exceeds the camera action duration; the previous action was preserved.";
		else (void)level->Update_CameraShot(shot, m_strStatus);
	}
	// Re-read after validation so rejected edits cannot become preview or resource input.
	if (const auto* committed = Find_AuthoringCamera(m_strCameraWindowShotId))
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE resource;
		resource.eKind = KOUKU_SAYDON_PRESENTATION_KIND::CAMERA; resource.strResourceKind.clear();
		resource.strAssetId = committed->strShotId; resource.strDisplayName = committed->strDisplayName;
		resource.iDurationMs = Camera_DefaultDuration(resource);
		if (ImGui::Button("Play Camera")) Queue_PresentationPreview(resource);
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Camera only: World boxes of the Sequence do not play. Use Pause Sequence at this position for the full scene.");
		ImGui::SameLine();
		ImGui::BeginDisabled(!m_bHasDraft);
		if (ImGui::Button("Append Camera at Cursor")) { std::string status; (void)Append_PresentationSource(resource, status); }
		ImGui::EndDisabled();
	}
	ImGui::TextWrapped("F6 Free camera: place the view, then capture. Times set travel speed; rotation includes roll. Save Camera writes the Area source. Append and Composition Save keep the stable shot reference for Sequencer Play.");
	ImGui::TextWrapped("Save Camera writes Data/Maps/Authoring/<Area>/<Area>.camerashots.json. Sequence Save writes the CAMERA box start, length and shot reference in the Composition.");
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::PopID(); ImGui::End();
}

bool_t Client::CKoukuSaydonActionWorkbench::Make_DedicatedCameraShot(
	const std::string_view patternIdView, const std::string_view occurrenceIdView, std::string& outStatus)
{
	// The views may point into m_Draft, which Commit_Candidate replaces.
	const std::string patternId(patternIdView), occurrenceId(occurrenceIdView);
	auto* level = CLevel_KakulSaydonArena::Get_Active();
	if (!level || !m_bHasDraft)
	{ outStatus = m_strStatus = "Enter KoukuSaydon with a loaded Composition to split a Camera shot."; return false; }
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (!pattern || !pattern->strLoadError.empty())
	{ outStatus = m_strStatus = "The CAMERA box Pattern is unavailable; the Composition was preserved."; return false; }
	const auto box = std::find_if(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
		[&](const auto& row) { return row.strOccurrenceId == occurrenceId; });
	const auto* resource = box == pattern->PresentationOccurrences.end() ? nullptr : Find_PresentationResource(candidate, box->strResourceId);
	if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
	{ outStatus = m_strStatus = "Select an existing CAMERA box before splitting its shot."; return false; }
	if (candidate.iNextPresentationResourceOrdinal >= 1000000u)
	{ outStatus = m_strStatus = "Presentation resource IDs are exhausted."; return false; }
	auto dedicated = *resource;
	dedicated.strDisplayName = resource->strDisplayName + " / " + occurrenceId;
	std::string shotId;
	if (!level->Duplicate_CameraShot(resource->strAssetId, dedicated.strDisplayName, shotId, outStatus))
	{ m_strStatus = outStatus; return false; }
	if (dedicated.strDisplayName.size() > 255u) dedicated.strDisplayName = shotId;
	dedicated.strResourceId = "kakulsaydon.g1.presentation." + std::to_string(candidate.iNextPresentationResourceOrdinal++);
	dedicated.strAssetId = shotId;
	// Only this occurrence moves; every other box keeps the shared resource.
	box->strResourceId = dedicated.strResourceId;
	const auto resourceId = dedicated.strResourceId;
	candidate.PresentationResources.push_back(std::move(dedicated));
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "CAMERA box now uses a dedicated shot. Save Camera, then Sequence Save.", outStatus))
	{
		std::string ignored;
		(void)level->Discard_UnsavedCameraShot(shotId, ignored);
		return false;
	}
	if (m_PresentationBoxEdit.strOccurrenceId == occurrenceId) m_PresentationBoxEdit.strResourceId = resourceId;
	m_strCameraWindowShotId = shotId; m_strCameraKeyId.clear(); m_strCameraSharedEditShotId.clear();
	m_bPresentationResourceRefreshRequested = true;
	return true;
}
```

#### 같은 파일 — `Render_PresentationBoxDetails` 호출 한 줄

```cpp
	if (cameraBox) Render_CameraAuthoring(definition.strAssetId, patternId, occurrenceId);
```

#### `Client/Public/KoukuSaydonActionWorkbench.h`

```cpp
		void Render_CameraAuthoring(std::string_view shotId, std::string_view patternId = {}, std::string_view occurrenceId = {});
		void Render_CameraWindow();
		bool_t Make_DedicatedCameraShot(std::string_view patternId, std::string_view occurrenceId, std::string& outStatus);
```

```cpp
		std::string m_strCameraWindowShotId;
		// The Sequence CAMERA box that opened the window maps shot time to Sequence time.
		std::string m_strCameraWindowPatternId;
		std::string m_strCameraWindowOccurrenceId;
		// A shot played by several owners stays read-only until this names it.
		std::string m_strCameraSharedEditShotId;
		std::string m_strCameraKeyId;
```

#### `Client/Public/Level_KakulSaydonArena.h` — `Capture_CameraShot` 선언 뒤

```cpp
	bool_t Duplicate_CameraShot(std::string_view sourceShotId, std::string_view name, std::string& outShotId, std::string& outStatus);
	bool_t Discard_UnsavedCameraShot(std::string_view shotId, std::string& outStatus);
```

#### `Client/Private/Level_KakulSaydonArena.cpp` — `Save_CameraShots` 정의 바로 앞

```cpp
bool_t Client::CLevel_KakulSaydonArena::Duplicate_CameraShot(const std::string_view sourceShotId,
	const std::string_view name, std::string& outShotId, std::string& outStatus)
{
	if (!Ensure_CameraShotAuthoring(outStatus)) return false;
	const auto source = std::find_if(m_AuthoringCameraShots.begin(), m_AuthoringCameraShots.end(),
		[&](const auto& value) { return value.strShotId == sourceShotId; });
	if (source == m_AuthoringCameraShots.end()) { outStatus = "Camera shot was not found."; return false; }
	if (m_AuthoringCameraShots.size() >= CAMERA_SHOT_MAX_COUNT) { outStatus = "Camera shot limit is 128."; return false; }
	auto shot = *source;
	for (uint32_t ordinal = 1u; ordinal <= CAMERA_SHOT_MAX_COUNT + 1u; ++ordinal)
	{
		shot.strShotId = "camera.kouku.pattern." + std::to_string(ordinal);
		if (std::none_of(m_AuthoringCameraShots.begin(), m_AuthoringCameraShots.end(),
			[&](const auto& item) { return item.strShotId == shot.strShotId; })) break;
	}
	// The copy belongs to one Sequence box; the Area trigger keeps playing the source shot.
	shot.strDisplayName = std::string(name);
	shot.bPatternOnly = true;
	shot.strSequenceInstanceId.clear();
	if (shot.hasCameraTrack) shot.CameraTrack.strCueId = shot.strShotId;
	DATA_JSON_VALUE root; std::string ignored;
	(void)CDataJson::Parse(Camera_EmptyDocument(), root, ignored);
	auto fields = root.Get_Object(); fields["shots"] = DATA_JSON_VALUE::Array({ Camera_ShotJson(shot, nullptr) });
	std::vector<KAKUL_CAMERA_SHOT> validated;
	if (!Parse_CameraShots(Camera_JsonText(DATA_JSON_VALUE::Object(std::move(fields))), validated, outStatus)) return false;
	outShotId = shot.strShotId;
	m_DirtyCameraShotIds.insert(shot.strShotId);
	m_AuthoringCameraShots.push_back(std::move(shot));
	outStatus = "Dedicated Camera shot created in the draft. Save Camera writes it to the Area source.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Discard_UnsavedCameraShot(const std::string_view shotId, std::string& outStatus)
{
	std::vector<KAKUL_CAMERA_SHOT> saved;
	if (!Parse_CameraShots(m_strCameraAuthoringBaseline.empty() ? Camera_EmptyDocument() : m_strCameraAuthoringBaseline,
		saved, outStatus)) return false;
	if (std::any_of(saved.begin(), saved.end(), [&](const auto& shot) { return shot.strShotId == shotId; }))
	{ outStatus = "A saved Camera shot is never discarded here."; return false; }
	std::erase_if(m_AuthoringCameraShots, [&](const auto& shot) { return shot.strShotId == shotId; });
	m_DirtyCameraShotIds.erase(std::string(shotId));
	return true;
}
```

검증: 두 cpp 격리 구문 검사(`cl /Zs`), CRLF·BOM, `git diff --check`. 사용자 확인: W06 경로에서 P4(또는 확정 항목) CAMERA 박스 → Open Composition Camera → 소비자 목록·박스 시각 표시 확인 → 키 선택 → Pause Sequence at this position → Pos/FOV 소폭 수정이 같은 시각 화면에 반영 → Save Camera → Reload Cameras → 같은 시각 재확인 → 원래 값 복구·Save.
