# Bone.cpp Debug 최적화 설정 결과

## G01. 실제 반영

`Engine/Default/Engine.vcxproj`의 Bone.cpp 단일 항목에 Debug|x64 조건의 MaxSpeed,
Default BasicRuntimeChecks, ProgramDatabase, SupportJustMyCode=false,
PrecompiledHeader=NotUsing, 빈 ForcedIncludeFiles를 적용했다.
C++·헤더·수식·ABI·데이터·렌더링 품질 설정은 수정하지 않았다.
전체 교체 블록과 디버깅상의 차이는 [PLAN](2026-10-02_BONE_DEBUG_OPTIMIZATION_PLAN.md)에 있다.

## G02. 실행한 검증

MSBuild18 Insiders의 -getItem:ClCompile 평가를 전후 비교했다. Debug는 위6개 metadata만 바뀌었고
Release Bone metadata98개는 동일했다. Debug CRT, _DEBUG 등 매크로와 FP 설정도 유지됐다.
XML parse, 단일 ClCompile 등록, UTF-8 BOM 없음·CRLF 보존, git diff --check가 통과했다.
기존 native intermediate 검사3개와 build profile 검사2개가 통과했고 독립 리뷰에서도 변경 블록과
metadata 보존을 확인했다. 현재 checkout에 ProjectAudit 엔트리가 없어 해당 검사는 실행하지 않았다.

원본 프로젝트, 전후 평가 및 검증 로그는
`C:/Users/tnest/Desktop/LostArkTransfer/Sync-20261002/Recovery-20261002-1315`에 있다.

- Engine.before-bone-debug.vcxproj
- Bone-Debug-Metadata-Before.json / Bone-Debug-Metadata-After.json
- Bone-Debug-Source-Verification.json
- Movie-Artist-PoseDuplication.json

## G03. 후속 실제 컴파일과 미확인 사항

2026-10-02 정상 프로젝트에서 Bone.cpp를 Debug/Release 각각 ClCompile하여 오류0을 확인했다.
Movie 공통5개 TU의 양 구성 총10개 실제 컴파일, 정상 SDK export도 성공했고 기존 runtime
산출물298개는 SHA·크기·시각을 유지했다. `Movie-Integrated-ClCompile-Selected-Receipt.json` 참조.

이번 /O2 설정만으로 Release의 반복 샘플링이 줄어들지는 않는다. 별도로 Debug/Release 공통
channel sample 재사용을 구현하고 실제 v143·WModel local/combined/inverse-bind palette 검사를
통과했다. [공통 구현 결과](2026-10-02_MOVIE_ANIMATION_SAMPLE_REUSE_RESULT.md)를 따른다.

과거 idle queue24808은 확장된 Guide/Nav 변경을 위해 중지했다. 새 검증 입력을 고정하는
GuideNavMovie-Build-Ready.json과 Continue-GuideNavMovie-Debug-Then-Release.ps1을 준비한다.
새 표준 Product link·DLL 배포와 사용자 동일 조건 Movie FPS 비교는 아직 완료가 아니다.
현재 실행 중인 기존 Debug Client/Server에는 새 설정·공통 알고리즘이 적용되지 않았다.
