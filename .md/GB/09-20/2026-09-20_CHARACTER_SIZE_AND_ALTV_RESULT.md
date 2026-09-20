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
