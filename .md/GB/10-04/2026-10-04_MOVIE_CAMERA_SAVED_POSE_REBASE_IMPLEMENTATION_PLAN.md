# 저장한 Movie 카메라 첫 구도 기준 이동 경로 보정 계획

## G00. 현재 저장본과 대상 실측

사용자는 방금 저장한 워로드·창술사 구도를 기준으로 Movie 이동을 고쳐 달라고 요청했다.
현재 `Data/Camera/ClassSelection.cinematics.json`의 SHA256은
`6b73a904d0ff3a04bee6287e0642ab3a8602c3ef82ed7fd3fbbb464ea621bd6a`다.
HEAD의 기존 설치본과 비교하면 WARLORD Intro Camera2·4·5의 첫 0ms 키에서 Eye·LookAt·Up만
변경됐다. 다른 변경은 DIMENSIONMASTER 제외 목록 8개 추가이며 사용자 변경으로 보존한다.

Intro2는 첫 24ms에 Eye 4.6503m·시선 32.00도, Intro4는 560ms에 3.4637m·49.37도,
Intro5는 8ms에 3.0258m·38.74도만큼 기존 경로로 돌아간다. 첫 키 뒤가 새 구도를 따르지
않는 데이터 불연속이다. C++의 WORLD 좌표계나 source clock 처리는 현재 계약과 일치한다.

LANCE_MASTER Intro/Loop는 HEAD와 완전히 같다. 09-30 G23에서 Intro2~4와 Loop2~4의
546키를 이미 보정했고 현재 후속 컷의 첫 구간 이동은 0.01~0.044m/source-sec다.
새 기준 키가 없으므로 창술사에는 같은 변환을 다시 적용하지 않는다.

## G01. 기준·변환·보존 계약

09-26 WORLD_MOVIE_EFFECT_EDITOR_RESULT의 G21/G23 방식을 재사용한다. 현재 저장본을
후보의 기반으로 복사하고 HEAD는 직전 설치된 대상 컷의 곡선 기준으로만 사용한다.
WARLORD Intro2·4·5의 새 첫 키 3개를 exact 보존하고 각 컷의 이전 첫 basis에서 새 첫
basis로 상대 Eye 위치·시선 벡터·Up을 회전한다. Eye는 새 첫 위치를 기준으로 평행이동한다.
시선 길이에만 새/이전 첫 키의 일정 비율을 적용하며 경로 거리·시간·FOV는 확대하지 않는다.

같은 컷의 Loop2·4·5는 각각 자기 기존 첫 키·곡선·FOV·시각을 입력으로 같은 새 Intro
기준 구도에 맞춘다. 총 6컷 307키 중 사용자 기준 3키를 제외한 304키의 pose를 바꾼다.
첫 키·모든 stable ID·timeMs·FOV·cut·curve/easing·source metadata·clock·repeat·hold·
다른 컷·다른 클래스와 제외 목록 등 비카메라 필드는 유지한다. 기존 hard cut은 유지하며
독립 컷 사이에 새 블렌딩이나 임의의 연속 경로를 넣지 않는다.

## G02. 후보와 설치 소유권

이 작업은 `out/MovieCameraSavedPose20261004/`에 정확한 저장 bytes `before.json`, HEAD
기준 snapshot, `candidate.json`, stable class/camera/key ID별 `stable-field-patches.json`,
대상 6행 입력 의존 hash와 검증 결과를 만든다. 추적하는 새 도구나 C++ 파일을 만들지 않는다.
제품 소스·프로젝트/filter·Server·renderer 설정은 변경하지 않는다.

상위 작업이 최종 반영 직전 최신 저장본을 다시 읽고 대상 필드와 의존 행이 동일한지 검사한다.
무관 변경은 최신 저장본에 유지하고 같은 대상 필드가 바뀌면 재검토 전 교체하지 않는다.
Windows 원자 교체·실제 교체된 bytes 백업·실패 시 자기 변경 rollback을 사용한다.
카메라-only source 수정은 기존 LoadScenes/Save 계약으로 다음 진입에 사용되며 World publish는
필요하지 않다. 실행 중 도구의 미저장 draft를 외부에서 바꾸거나 자동 Reload하지 않는다.

## G03. 검증과 사용자 확인

현재 제품 CClassSelectionPresentation parser와 Recovery/Valtan camera sampler를 사용하는
CPU 검사에서 후보를 읽고 대상 컷 전체를 sample한다. 예상 회전/이동과 runtime float 오차,
첫 키 다음 이동, FOV·시간·기준 키 보존을 확인한다. 전체 JSON의 실제 diff가 pose 912필드만
포함하는지 비교하고 무관 사용자 제외 목록과 모든 repeat/hold 값을 검증한다.

후보 검증과 설치 검증을 RESULT에서 분리한다. Client/UI 실행·조작·종료·화면 캡처와
자동 Save/Publish/Reload는 하지 않는다. 사용자는 기존 WORLD 편집기의 Reload saved movie
후 워로드 Play All로 구도와 기존 hard cut을 확인하며 최종 화면 판단은 별도로 남는다.

## G04. cam05_a 추가 저장분 반영

통합 반영 뒤 사용자가 다시 저장한 `classselect.warlord.intro.camera.2`의 첫 0ms 키만
새 기준으로 삼는다. 최신 저장본 SHA256은
`91f04aae027f38bdb5de569a95b93fa66353baa94e62145a24000095cd7b713f`다.
직전 설치본과 현재 main의 대상 행이 같음을 대조한 뒤, 그 직전 곡선에서 새 첫 구도로
추가 회전·평행이동을 계산한다. 최초 원본 기준의 이전 보정량을 다시 누적하지 않는다.

Intro2 94키와 Loop2 95키 중 저장한 Intro 첫 키 전체를 유지하고 나머지 188키의
Eye·LookAt·Up 564개 vector만 바꾼다. 다른 컷·창술사·FOV·시간·curve·repeat·사용자 제외 목록은
유지한다. 이전 증거 폴더를 덮지 않고 `out/MovieCam05Refine20261004`에서 후보·실제 sampler를
검증하며 `out/MovieCam05Apply20261004`에서 대상 2행 guard·백업·원자 교체를 수행한다.
카메라 데이터만 바뀌므로 제품 재빌드나 EXE 종료는 요구하지 않는다.
