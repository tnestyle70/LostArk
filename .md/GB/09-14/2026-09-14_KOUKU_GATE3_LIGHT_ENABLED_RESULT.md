# 쿠크 3관문 조명 Enabled와 추가 광원 반영 결과

## G00. 확인한 원인

`Client/Private/Level_KakulSaydonArena.cpp::Prepare_GateMapLights`는 3관문에서
`light.enabled = light.lightId == enabledId`로 모든 광원을 다시 설정하고 `.6`을
강제로 켰다. 그래서 청색 `.6`의 색·밝기는 반영돼도 enabled=false는 무시되고,
백색 `.7`은 source에서 true여도 관문 문서에서 false가 됐다.
`Same_MapLightSource`에는 enabled·RGBA·개수 비교가 이미 있어 변경 감지 누락이 아니다.

Engine의 `CPresentation_Manager::Submit_FrameProviders`는 transient 목록을 비우고
각 provider를 다시 제출한다. `Renderer::Render_Lights`의 MRT 시작은 조명 RT를 clear하고,
`Light_Manager`는 현재 광원 수만 GPU에 업로드·draw한다. Spotlight는 ONE+ONE 가산이다.
`CShader::Bind_RawValue`의 동일 값 재설정 생략은 64바이트 이하 입력에만 적용되며
112바이트 light record 배열이나 조명 결과 저장이 아니다. Engine·shader 최적화는 변경하지 않았다.

## G01. 실제 소스 변경

변경 함수는 `Prepare_GateMapLights` 하나다. 3관문에서는 기존 배경 그룹
`source_baked_character`, `source_direct`와 기존 1·2관문 저작 광원 `.1`~`.5`만
비활성화하고 `.6` 및 새 광원은 원본 enabled·색·밝기·위치를 보존한다.
`.6`이 삭제돼도 3관문 문서를 만들 수 있다. 1관문의 popup 필수 광원과 강제 활성화는 유지한다.
상위 stage/commit과 실패 시 이전 관문 문서 보존 경로는 그대로다.

기존 브랜치 `codex/kouku-donut-ball-motion`, HEAD
`e6f19ec806c7f41eacc37553a4d919cb17163d18`의 미커밋 작업을 보존했다.
작업 시작 snapshot과 비교해 해당 함수 밖 bytes는 동일하다. UTF-8 BOM 없음과
기존 혼합 줄바꿈도 보존했다. 사용자 JSON·Scene Profile·Engine 소스는 이 작업에서 쓰지 않았다.
새 C++ 파일이나 public API가 없으며 기존 vcxproj와 filters의 해당 CPP 등록을 확인했다.
자동 stage/commit/push는 하지 않았다.

## G02. 실행한 검증

수정된 실제 `Level_KakulSaydonArena.cpp`를 제품과 같은 Debug 문자 집합 설정의
`/MDd /D_DEBUG /Od /RTC1 /std:c++20`으로 별도 out OBJ에 컴파일해 exit 0을 확인했다.
기존 include의 C4819 경고는 남아 있다. 실행 중 제품 EXE/DLL/PDB를 교체하지 않은
최소 TU 컴파일이며 최종 Product 링크 성공 증거로 사용하지 않는다.
로그: `out/KoukuGate3LightEnabled20260914/compile.log`.

구/신 실제 관문 helper와 변경 없는 popup·source 비교 함수를 추출하고 현재
`MapLightDocument.cpp`, `DataJson.cpp`를 직접 컴파일한 native 검사에서
14개 시나리오, 148개 assertion, 실패 0을 확인했다. 청색/백색 on/off 4조합,
새 `.8` on/off, `.6` 삭제, 색·위치·밝기 변경, source 변경 감지와 원본 보존을 검사했다.
1·2관문은 5개 문서에서 구/신 결과가 동일하다. 이전 helper의 강제 on/off와 삭제 실패도
같은 입력으로 재현했다. provider의 문서 저장 부분만 대역이며 GPU 제출·화면 검사가 아니다.
증거는 같은 out 폴더의 `native_regression_result.json`, `native_regression_receipt.json`,
`native_regression_run.log`와 재현 스크립트 `build_native_regression.ps1`이다.

전체 `git diff --check`와 변경 파일의 집중 검사, 관련 maplights JSON 두 개 parse,
기존 Client project/filter XML parse가 통과했다. 변경 함수 밖 보존 및 source SHA256은
`out/KoukuGate3LightEnabled20260914/source-check.json`, 이번 작업만의 diff는
같은 폴더의 `session-change.diff`에 기록했다.

## G03. 실행 반영과 사용자 확인 대기

조사 시 저작 maplights는 122개이며 청색 `.6`과 백색 `.7`이 모두 enabled=true다.
runtime maplights는 121개이며 `.7`이 없다. 백색은 brightness=55.7000008,
청색은 5.94999981로 저장돼 있고 이 값을 임의 변경하지 않았다.
저작 preview를 끄거나 재시작하면 published 파일을 읽으므로 새 백색광에는 Publish도 필요하다.

사용자는 종료 질문에 `수정중이야 아직`이라고 답했다. 따라서 현재 Client/Server와
저장·실행 데이터를 유지하고 Product 링크·배포·Map Publish를 실행하지 않았다.
조사 시 Server PID 36372는 `Server/Bin/Debug/Server.exe`, Client PID 46256은
`Client/Bin/Debug/Client.exe`를 사용했고 LAN은 server-host/192.168.0.14:7777 reachable이었다.

편집 저장·종료 통보 뒤 최신 source와 실행 프로세스·동시 빌드를 재확인하고,
기존 `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish` 및 Check,
`Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`로 게시·링크·배포한다.
오래된 후보 JSON으로 최신 사용자 편집을 덮지 않는다.

사용자 화면 확인은 새 Client의 Lobby → KoukuSaydon → 기존 3관문 선택 →
F1 Rendering Workbench의 Map 목록에서 두 광원을 각각 off/on하는 순서다.
`Preview authored map lights`를 켜서 편집값을 확인하고, 필요한 값을 Save Light →
Publish Light로 저장·게시한다. 청색 off/백색 on, 청색 on/백색 off, 둘 다 on/off를
직접 확인한다. Client/UI 실행·조작·캡처와 시각 PASS는 수행하지 않았다.
