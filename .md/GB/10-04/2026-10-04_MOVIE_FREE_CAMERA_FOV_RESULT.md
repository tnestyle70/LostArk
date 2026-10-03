# World Movie 자유 카메라 포즈 캡처 수정 결과

## G00. 확인한 원인

사용자 화면의 WARLORD intro cam05_a는 `classselect.warlord.intro.camera.2`이며94개 키와
HORIZONTAL FOV17도를 사용한다. Loop camera.2도 같은 FOV다. 16:9 수직 환산값9.610678도는
Movie runtime의1<FOV<179 범위에 있지만 Capture_ViewPose의 legacy10~120도 검사에 거절됐다.
MainApp의 free/follow/override 검사는 이미 통과한 뒤라 F6를 다시 누르는 것으로 해결되지 않는다.
F6 인계는 현재 Movie pose와 FOV를 보존한다.

## G01. 반영한 코드

CameraTool.cpp의 Capture_ViewPose는 runtime FOV 범위를 사용하고 projection, Eye/LookAt,
Up과 시선의 유한성·크기·평행 여부를 확인한다. 성공한 local candidate만 출력에 commit한다.
기존 거리10과 위치100000 제한은 유지하며 FOV를 clamp하지 않는다.
SequenceCameraEditor는 기존대로 Eye/LookAt/Up만 key에 반영하고 FOV·시간·보간을 유지한다.

독립 검토에서 Capture_CurrentPose가 strict 검증 전에 기존 키를 대입하던 경로도 확인했다.
검증을 대입 앞으로 이동해 Valtan 저작의10~120도 제한과 실패 시 기존 키 보존을 함께 유지했다.
CameraTool.h의 주석과 팀 인계 문서·gotchas를 현재 계약에 맞췄다.
새 C++/project/filter 등록, JSON·mapplacement·렌더링 옵션·Server·protocol 수정은 없다.

## G02. 실제 검증

- 실제 두 캡처 함수와 validator, POSE/KEYFRAME 선언을 추출한 DirectXMath CPU probe:
  75 checks, failures0, compile exit0. GameInstance의 matrix getter와 tracking 연결만 stub이다.
- 기존 HEAD의 실제 함수가 워로드 수직9.610677719도를 거부하는 것을 재현했다.
  수정본은 이 값과45/130도를 수용한다. 0/1/179.1/180/NaN, 무효 basis와 null matrix를 거부하고
  출력 포즈를 보존한다. legacy10~120도 제한과 strict 거부 시 키 필드 보존도 확인했다.
- 179도 입력의 float 행렬 왕복값은178.9999847도다. 실제 복원값에 runtime의 열린 구간 검사를
  적용해 수용하며, 입력 각도의 decimal 표기와 bit-identical 왕복을 가정하지 않는다.
- CameraTool.cpp를 실제 Debug 옵션과 MSVC14.44로 두 차례 독립 TU 컴파일했고 최종본 exit0이다.
  OBJ/PDB는 out에 두었고 실행 Client의 EXE는 이 단계에서 교체하지 않았다. 기존 C4819 경고가 있다.
- 기존 `Tools/ValtanPipeline/test_valtan_camera_tool_contract.py`는 무관한
  `VALTAN_GAMEPLAY_FOLLOW_LOOK_HEIGHT` 문자열 검사에서 실패했다. 변경 전 HEAD의 캡처 소스로도
  같은 실패를 재현했고 이 기존 검사나 Valtan Arena 코드는 변경하지 않았다.
- 두 C++ 파일의 UTF-8 BOM 없음·CRLF 유지와 `git diff --check`를 확인했다.

증거는 `out/CameraCaptureFov20261004/`의 `run.log`, `compile.log`, `provenance.json`,
`CameraTool.log`, `source-check.json`, `legacy-source-contract.txt`다.
native 검증은 실제 Client UI 조작·화면 확인을 대신하지 않는다.

## G03. EXE 반영 상태

검증 완료 시 Debug Client PID49400와 Server PID50224가 실행 중이었다.
사용자가 모두 종료했다고 답한 뒤 실제 두 프로세스가 없음을 확인하고 정식 Debug Product Build를
실행했다. Engine·Shared·Server·Client 전부 PASS/exit0이며 Client는42461ms, OBJ13개·EXE1개를
갱신했다. Engine/Shared/Server의 OBJ·binary와 CSO는 변경 없이 재사용했다.
증거는 `out/BuildPipeline/runs/20261003T174003757Z-debug-product.json`과
`out/CameraCaptureFov20261004/product-debug.log`다. receipt 파일명의 날짜는 UTC이며
실제 Client.exe 갱신 시각은2026-10-04 02:40 KST다.
런타임 파일·navigation·Item/Valtan catalog 검사는 통과했고 데이터 publish는 하지 않았다.
사용자가 편집한 camera JSON, Gate1 Composition과 워로드 배경 mapplacements의 SHA256은
빌드 전후 동일하다. 이번 반영은 Debug 구성이다. Client/UI 자동 실행·종료·Reload는 하지 않았다.
반영 후 사용자는 동일 Movie에서 F6 자유 시점을 선택하고 해당 key의 Use free cam pos를 확인한다.
