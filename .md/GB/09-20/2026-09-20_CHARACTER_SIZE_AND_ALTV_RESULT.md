# 캐릭터 크기와 차원술사 Alt V 결과

## G00. 구현 상태

`ArenaCameraProfile`은 기존 map별 camera JSON에 optional `classSizeMultipliers`, `clownSizeMultiplier`, `marioSizeMultiplier`를 저장한다. 클래스는 stable 이름 여섯 개로 검사하며 잘못된 이름·개수·비유한 값·0.25~4 밖의 값은 거부한다. 이전 문서는 새 기본값을 사용한다. F1의 Player Follow Camera → Character Size에서 전체, 여섯 class, 광기 광대, Mario 광대를 조절하고 기존 Save/Reload로 유지한다. 카메라 preset은 크기 편집값을 보존한다.

기본 배율은 이번 요청 직전 catalog 모델을 기준으로 Artist 1.6, DimensionMaster 0.7, 광기 광대 0.7이다. 따라서 기존 catalog 값 1.5/1.05에 대해 유효 class 배율은 2.4/0.735다. Mario는 기존 1.5 m admission height에 1배를 유지한다. 로컬·원격 캐릭터와 새로 생성되는 캐릭터가 같은 active-map snapshot을 읽고 매 frame presentation root만 갱신한다. Server transform·collider·스킬 범위를 변경하지 않는다. `Apply_MarioPresentation`은 일반 광기 광대의 무기를 숨기며 Mario에서만 표시한다.

## G01. 차원술사 Alt V

어제의 Effect Detail에는 이미 방향별 속도·전체 수축시간·offset·회전·정사각형·model center 편집이 구현돼 있었다. full 312행, tuning 27행의 실제 ScreenPost에는 새 튜닝 값이 없어 0도 기본값으로 읽히고 있었다. 같은 패널에 `45 degree capture into cube` 프리셋과 각 축의 용도 설명을 추가했다.

두 문서의 `altv.authored.starting-scene-capture`에 `captureRotationDegrees=45`, `captureSquare=true`, `captureUseModelCenter=true`만 설치했다. 최신 bytes 확인·이전본 백업·원자적 교체를 사용했으며 다른 element, visibility, tint, cube opacity, 사용자 transform을 보존했다. 설치 manifest와 백업은 `out/CharacterSizeAltV20260920/altv_candidate_manifest.json`에 있다.

현재 native cube는 같은 고정 장면을 ALT178 material로 읽어 기존 TRANSLUCENT 합성을 사용한다. 이번 변경은 앞부분 project 2D capture의 저작 튜닝이다. 숨겨진 source camera mesh 18/31 복구, 원작 capture camera CB 해석, 액자 전체와 2D→3D UV/구도의 완전한 연속성은 이번 변경의 완료 항목이 아니다. 근거는 기존 `out/DimensionMasterArtist20260919/cube_source_audit.md`이며 실제 화면의 cube handoff는 사용자 확인이 필요하다.

## G02. 실행한 검증

- 변경 8개 Client TU의 현재 소스를 별도 out object로 Debug 컴파일 성공. 기존 include 인코딩 경고는 남아 있다. 로그: `out/CharacterSizeAltV20260920/compile.log`.
- 실제 `ArenaCameraProfile`/`DataJson` C++ 소비자를 isolated Data root에서 실행해 152개 검사 통과. 새 크기 저장·재로드, 이전 문서 기본값, 잘못된 class 이름·값, stale Save와 malformed Load의 기존 상태 보존 포함. 로그: `profile-check.log`.
- 기존 Codec dependency object와 현재 candidate 검사 프로그램을 연결해 두 ALT V 문서의 Load, Validate_Drawable, 세 필드 확인, serialize/parse 일치 통과. 로그: `candidate-check.log`. Renderer GPU draw 검사는 아니다.
- JSON 값 비교에서 두 문서는 요청한 세 capture 필드 외에 변경 없음. 변경 C++/header 대상으로 `git diff --check` 통과.

제품 링크·배포 검증은 상위 통합 작업에서 수행한다. Client/UI를 실행하거나 화면을 캡처하지 않았고 visual PASS를 기록하지 않는다.

## G03. 전 맵 자유 카메라 속도와 크기 Save — 2026-09-21

F1의 `Map Camera / Player`에서 Character Select, Bern, Valtan, KoukuSaydon, Development/Training, Maharaka의 자유 카메라 속도를 조절한다. Bern은 기존 getter를 사용하고, Development와 Maharaka를 함께 소유하는 `CLevel_Development`에는 현재 level ID를 대조하는 active-owner getter를 추가했다. 기존 camera weak reference를 반환하며 새 camera나 layer 탐색 경로는 만들지 않았다. Lobby와 Loading은 조절할 map camera가 없다. 발탄·쿠크의 session별 속도 보존, 다른 map의 방문별 속도, 0.1~400 m/s 범위와 Shift 배율은 기존 계약이다. Move Player는 계속 발탄·쿠크의 Server 승인 경로만 사용한다.

`Player Follow Camera → Character Size → Requested size defaults` 바로 아래에 `Save` 버튼을 추가했다. 현재 선택 map의 크기를 포함한 camera profile을 기존 stale 검사·임시 파일 검증·atomic replace로 저장한다. 저장 성공 후 활성 map의 기존 `Set_FollowCameraProfile`을 호출해 크기 snapshot도 바로 갱신한다. 매 frame 같은 snapshot을 읽는 로컬·원격 캐릭터에 다음 update부터 반영된다. 저장 후 적용은 follow/free 모드를 바꾸지 않고 진행 중인 presentation override도 유지한다. 비활성 map은 다음 입장에 적용하며 상태 메시지로 구분한다. 아래 `Save camera settings`도 같은 저장·적용 흐름을 사용한다. 실패 저장은 새 runtime 적용을 수행하지 않는다.

실제 `Data/Camera` 문서는 수정하지 않았다. 사용자 Save 클릭 때 선택 map 문서만 변경한다. Server transform·collider·damage 범위는 변경하지 않았다.

검증은 `out/CameraSizeControls20260921`에 보존했다.

- 실제 변경 `MainApp.cpp`, `Level_Development.cpp` 두 TU의 Debug/Release `/Zs` 검사 성공. VS18 Insiders MSVC14.44 x64와 현재 EngineSDK include를 사용했고 별도 `/utf-8` 옵션으로 제품 인코딩 조건을 바꾸지 않았다. 기존 외부/header 인코딩 경고는 남는다.
- 기존 profile 검사 프로그램을 현재 `ArenaCameraProfile.cpp`, `DataJson.cpp`, `ProjectDataRoot.cpp`와 다시 컴파일·링크하고 격리 Data root에서 152개 검사 성공. 크기 저장·재로드, 잘못된 값 거절, stale Save와 malformed Load의 기존 상태 보존, map 간 파일 격리를 포함한다. 첫 격리 `/Fo` 출력 경로 quoting 오류는 실행 전 수정했으며 성공 검사에는 포함하지 않는다.
- 관련 파일 `git diff --check` 성공. C++ UTF-8 BOM 없음·CRLF 유지, 기존 project/filter 등록 확인. 새 제품 파일은 없다.

이는 소스·최소 컴파일·실저장 계약 검증이며 제품 링크·설치와 실제 화면 판정은 상위 통합 작업 및 사용자 확인 단계다. Client/UI를 실행하지 않았다. 새 제품에서 F1 속도 조절 후 F6 이동, 크기 변경 후 바로 아래 Save, 재입장 시 같은 크기 복원을 사용자가 확인한다.
