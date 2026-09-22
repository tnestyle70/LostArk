# Clown·MAZE·핑·화염파동 통합 결과

## G00. 현재 반영 단계

사용자의 후속 “실제 반영해줘” 승인을 받아 최신 디스크 revision2193에 이전 검증 후보23파일을 실제 설치했다. `out/ClownMerged20260922/applied-manifest.json`에 백업·hash·설치 결과를 기록했다. Clown Q/W/E, 공유 MAZE Q/LMB, Ctrl 표식,14그룹 화염파동과9개 모델/텍스처가 설치됐고 사용자 P58/P100~103 및 back_shot 변경을 보존했다. 아래 후보 검증 항목은 설치 전 확보한 증거다. 후속 카드비·무력화·오디오 통합은 `2026-09-22_KOUKU_GATE1_PATTERN_RESTORATION_RESULT.md`에서 설치·게시·제품 빌드를 구분한다. Client/UI를 자율 실행하거나 미저장 draft를 Reload하지 않았다.

## G01. Ctrl 핑 입력과 표식

`CPlayerController`의 Ctrl 새 press는 핑 대기를 만든다. Ctrl을 누른 채 다음 물리 좌클릭을 하면 navigation에서 높이를 확인한 위치에 핑을 3초 표시한다. 대기 중 과녁은 실제 Character의 기존 head anchor를 따라간다. Ctrl 해제·다른 키/우클릭·UI·focus·capture는 대기를 취소하며, 소비한 클릭은 release까지 평타·MAZE LMB·ground-target confirm과 mouse swap의 이동 입력에서 제외한다. Ctrl+Z/X/C Esther 입력은 유지한다.

기존 `CClickMoveEffect`와 `CEffectPresentationService`의 Level preparation/spawn/update/stop을 재사용한다. 새 네트워크 packet이나 gameplay authority는 없고 로컬 표시다. source texture `fx_l_symbol_07_cl`/native2813와 `fx_l_symbol_21_cl`/native2800를 재사용한다. 하얀색·고정 위치·크기·시간 및 3×3 atlas index4는 프로젝트 저작값이다. 원본 DDS와 shader 식은 바꾸지 않는다.

후보는 `out/KoukuMarkers20260922/manifest.json`이며 신규 Resources는 없다. 실제 source validator의 색 공간/module/attachment/native sprite 검사를 통과했다. 실제 Product codec 및 playback 검사 303개가 failures0이다. ping peak1, reticle peak2이며 transform/capacity가 유효했다. 이 검사는 GPU 표시 성공이나 사용자 화면 판정이 아니다.

독립 검토에서 raw LB가 버튼별 UI 차단을 우회하는 조건을 발견해 filtered LB와 같은 프레임 UI claim을 추가했다. 실제 입력 gate·Update 입력 블록·Esther 함수 본문을 사용한 native console 검사 35개가 통과했다. 기록은 `out/PingInputReview20260922/result.log`다.

## G02. Effect Tool 삭제

비활성 preview 편집이 불필요한 source bone/GPU 준비를 요구하지 않도록 수정했다. 마지막 Solo Family 삭제와 실패 rollback도 보존한다. 실제 함수 본문 probe는 이전6실패에서 수정 후15조건 모두 통과했다. 상세·기존 검사 실패와 사용자 수동 경로는 [삭제 결과](2026-09-22_EFFECT_ELEMENT_DELETE_RESULT.md)를 따른다. 사용자의 Trail JSON은 삭제하지 않았다.

## G03. 화염파동 후보

기존10지점·260요소에서 4행2/3/4/5개, 총14그룹·392요소로 구성했다. 가로 간격5→3.5m, 행 간격4.33→3.031m다. 기존 요소의 stable ID를 보존하고 source FireWave의 native2873 지면 화염과 native2874 ring을 추가했다. WandDecal 착지 섬광과 실제 불바닥은 별개 source다.

그룹은 `manual.flame-wave.rN.cN`이며 기존 Group Center/Save Changes를 소비한다. 실제 Codec/Playback219,335검사와 production helper의 그룹 이동·저장·재로드426검사가 failures0이다. 28개 새 ground element의 중심 XZ 오차는4e-7m 이하, 지면 offset은0.04m다. 원본 disabled notify는 켜지 않았다. 수치 결과를 billboard 이후 최종 화면 법선·밝기 검증으로 표현하지 않는다.

후보는 `out/KoukuFlameWave20260922/manifest.json`이다. 사용자 편집된 P58의 position offset과5,352ms는 보존하며, 기본 수명의 P49/P59 및 리소스 정의에는9,100ms tail을 반영할 patch를 준비했다.

## G04. 컴파일과 남은 검증

PlayerController, ClickMoveEffect, Effect_Tool_Editing, CharacterModelWorkbench의 실제 수정 CPP4개는 MSVC x64 Debug scratch compile을 통과했다. 처음 두 시도의 Windows SDK include/response-file 줄바꿈 오류를 수정한 뒤 성공했다. 이는 정본 Product 링크·배포의 대체 결과가 아니다. 로그는 `out/KoukuMarkers20260922/compile/compile.log`다.

추가로 Engine Model.cpp·Shader.cpp의 실제 수정 TU2개와 WorldSequenceDocument.cpp의 native109 소비자 scratch compile을 통과했다. Product 실행 파일과 DLL은 교체하지 않았다.

## G05. Clown·MAZE 설치 전 후보 검증

Clown POLYMORPH의 기존 Q/W/E clip에 각각 새 Effect asset을 연결한다. Q는 기존 폭탄 WModel·native 폭발과 투척 궤적을 조합한 프로젝트 저작이고, W는 기존 공+원본 sk02 요소, E는 원본 선물상자 WModel+white burst를 사용한다. MAZE의 Q10요소/LMB2요소를 공유 저작 문서에 추가해 전 캐릭터가 소비하며 Tool 라벨도 Q와 LMB로 구분했다. 신규 Effect5개를 Client project/filter의96.DataFiles에 등록했다.

현재 SourceCharacter native109 header를 소비하는 실제 loader·playback으로 Clown3개+MAZE2개의11,254검사가 failures0이다. W/E의 b_wp_1과 bip001-r-finger11은 설치된 Clown WModel의 해당 clip을60Hz로 샘플해 실제 combined matrix×preScale0.012053×yaw−90로 검증했다. synthetic bone 대체 결과가 아니다. GPU 표시와 최종 이미지 일치는 사용자 확인 전이다.

독립 검토에서 Light109 입력 ABI 분기 누락을 발견해109에만 legacy UV/light/view/position 배치를 적용했다. 기존84~108의 source program 본문은 유지한다. 새 선물상자5파일과 기존 폭탄의 Effect 경로 사본4파일은 out의 candidate/Resources9파일로만 준비했다. Git binary 추가·라이브 설치·Drive 업로드는 없다.

통합 후보는 `out/ClownMerged20260922/merge-manifest.json`의23파일이다. 실행 대기 스크립트 `out/apply_clown_reviewed_candidates_20260922.py`는 기본적으로 out만 쓰며, 명시적인 `--apply`는 승인 뒤에만 호출한다. 적용 시 stable ID·기존 field값·현재 hash를 확인하고 백업·파일별 원자 교체·자기 변경만 rollback한다. 사용자가 추가 저장한 P58/P100과 back_shot_dust_atk_02의 무관한 변경을 덮어쓰지 않는다. 현재 후보 revision은2189지만 후속 적용 시 이 숫자를 고정해서 덮어쓰지 않는다.

초기 검증 시점에는 사용자 편집 대기로 설치·publish·Product Build를 보류했다. 후속 승인에 따른 현재 설치 상태는 G00과 통합 RESULT를 따른다.

## G06. 대기 전 최종 정적 검증

실제 FxCompile wrapper3개 `Shader_Deferred_SourceGroup084`, `Shader_VtxAnimMeshBinary_SourceGroup084`, `Shader_VtxMeshBinary_SourceGroup084`를 현행 소스로 `fxc /T fx_5_0` scratch compile하여 모두 exit0으로 확인했다. SourceCharacter 함수의 기존 계열 X4000 potentially uninitialized 경고는 있으나 compile error는 없다. 이 CSO는 out에만 있고 제품의 배포 CSO를 대신하거나 timestamp를 조정하지 않았다.

통합 후보 JSON14개와 변경 project XML2개 parse, `git diff --check`가 통과했다. C++ 파일의 기존 CRLF와 UTF-8 BOM 없음 상태를 유지했다. 사용자 편집 파일2개의 Git LF→CRLF 안내는 있었으나 해당 파일을 변경하거나 줄바꿈을 정리하지 않았다. Product 전체 링크·배포는 사용자의 “반영 대기” 요청으로 미실행이다.

Clown source action·재질·본 실측·Resources 상대 경로 상세는 [Clown POLYMORPH/MAZE 결과](2026-09-22_CLOWN_POLYMORPH_MAZE_EFFECT_RESULT.md)를 따른다. W의 원본 공 낙하 곡선은 해당 occurrence만1/14로 줄인 프로젝트 조정이며 원본 source Effect는 변경하지 않았다. 최종 W 후보 갱신 뒤 통합 후보를 out에서 다시 생성하고 일치 여부를 확인했다.

독립 메모리 검토에서도23파일 병합, P58/P100 JSON token 보존, stable ID 순서 변경·무관 필드 보존과 같은 duration 필드 충돌 거절을 확인했다. Effect10문서의 Resources dependency closure는 모두 존재한다. 실제 설치·실패 주입은 수행하지 않았으며 최종 적용 전 최신 저장본을 다시 확인한다.
