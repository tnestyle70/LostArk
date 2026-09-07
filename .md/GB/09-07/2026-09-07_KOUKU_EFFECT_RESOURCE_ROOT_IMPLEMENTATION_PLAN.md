# 2026-09-07 KoukuSaydon Effect 리소스 경로 통합 구현 계획서

## G00. 목표와 실측

사용자가 옮긴 `Client/Bin/Resources/Effect/KoukuSaydon`을 쿠크 Effect 물리 정본으로 사용한다.
현재 폴더에는 `Meshes`, `Textures`, `Screen`, `WorldObjects`가 있으며 이전 두 Effect 폴더는 없다.
이 작업은 물리 파일을 이동하지 않고 저장된 참조 경로와 경로 소비자만 변경한다.
Character/UI/Sound 폴더와 원본 Unreal package 이름, effectId/objectId는 유지한다.

`Screen/fx_d_symbol_100_ycl.dds`는 09-06 커튼 ScreenPost 작업에서 설치한 원본 texture다.
V2 leaf `boss.kouku.curtain_1`이 소비한다. 공과 칼날은 WorldSequence의 Object Resources가
같은 Effect 물리 폴더의 WModel과 texture를 참조한다.

## G01. 저장 문서와 소비자

`Data/Effects/V2/Authored`의 슬롯, 쿠크 Area WorldSequence의 model/diffuse asset ID,
ResourceIntake의 runtimeRoot/effectPhysicalRoot를 새 상대 경로로 수정한다.
WorldSequence revision을 올리고 기존 Map publisher로 runtime 문서를 생성한다.
V2는 기존 direct-authored 경로를 유지하며 별도 복사본을 만들지 않는다.

`Client/Private/MainApp.cpp`의 쿠크 리소스 목록 prefix와 관련 기존 검사도 현재 Effect 경로에 맞춘다.
공용 `Scan_PhysicalResources`는 Effect 폴더를 재귀 검색하므로 기존 scanner를 그대로 사용한다.
새 C++ 파일이나 project/filter 등록은 필요하지 않다. 현재 리소스 인계 문서의 경로도 갱신한다.

## G02. 검증과 종료

Git 추적 코드·Data·도구와 배포된 runtime 문서를 대소문자·역슬래시 표기를 포함해 조사한다.
수정한 JSON은 parse하고 Resources 상대 경로의 파일 존재와 WModel material 참조를 확인한다.
기존 Map publish/check, 관련 Effect/Map 검사, 필요한 Debug 컴파일과 `git diff --check`를 수행한다.
과거 추출 기록의 당시 경로는 현재 runtime 참조와 구분한다.

사용자가 승인한 범위로 commit/push, PR 생성·merge 후 로컬 main을 fast-forward 동기화한다.
Resources binary는 Git에 추가하지 않는다. Client/UI 실행과 최종 시각 확인은 사용자가 수행한다.
