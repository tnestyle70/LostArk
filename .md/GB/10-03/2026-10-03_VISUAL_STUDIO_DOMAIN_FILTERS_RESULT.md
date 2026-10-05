# Visual Studio 도메인 필터 정리 결과

## G01. main 렌더링 동기화

시작 작업 트리는 clean이었고 브랜치는 `codex/character-slot-state-restore`, HEAD는
`96a06a6d`였다. fetch한 `origin/main`은 `c7de2091`이며 PR505·507의 렌더링·Profiler 변경을
포함했다. 새 `codex/rendering-architecture-atlas`에서 `3c6ebbd1`로 병합했고 conflict는0개다.
캐릭터 슬롯 저장·복원 commit은 보존했다. 원격 main을 변경하거나 기존 작업을 reset하지 않았다.

통합한 범위는36파일이며 authored Data와 게시 DataFiles/Resources payload는 바뀌지 않았다.
기존 shader 입력664개 중 `Shader_Deferred.hlsl`의 Engine/Client 두 사본만 내용과 시각이
변경됐고 나머지662개는 비교 시점에 동일했다. 신규 `Shader_ScreenSpaceLighting.hlsl` 두 사본은
이 기존 파일 수에 포함되지 않는다. timestamp를 되돌리거나 tracking을 조작하지 않았다.
로컬 증거: `out/CodeAtlas/sync-before.json`, `shaders-before.json`, `sync-result.json`.

기간이 만료된 팀 LAN sync는 실행하지 않았다. endpoint와 개인 debugger 설정도 변경하지 않았다.

## G02. 실제 적용한 필터

| 프로젝트 | 최종 파일 항목 | 필터 선언 | 미분류 |
|---|---:|---:|---:|
| Engine |269|82|0|
| Client |2917|369|0|
| Server |146|27|0|
| Shared |29|10|0|
| 합계 |3361|488|0|

Engine의 기존 System/Utility 구조와73개 필터/GUID는 유지하고 shader98개를 역할별로 분류했다.
PCH2개는98.Default에 둔다. Client는 Tools와 실제 runtime Presentation, DataAccess,
Data 정본, shader를 나눴다. 같은 basename의 H/CPP는 같은 domain이다. Server는 Room과
판정·AI·boss·경제·generation 및 tests를 구분했다. Shared는 transport/protocol/gameplay/revision을 나눴다.

Client의 기존 필터에 잘못 중첩된 Effect `None`2개는 올바른 ItemGroup 항목으로 고쳤다.
`gate2.intro.lights` 항목의 자식으로 숨어 있던 albion cross/frontthree electric impact다.
프로젝트에는 이미 등록되어 있어 새 runtime 등록이나 파일 생성이 아니다.

기존 Client 프로젝트·필터의 `Data/UI/Bern/` layout2개는 실제 파일이 없었다.
현재 `RaidEntryPreviewView.cpp:262,265`와 HUDLayoutTool이 소비하는 `Data/UI/RaidEntry/`의
같은 JSON으로 `None Include`를 교정했다. `.md/TEAM/NPC_OWNER_HANDOFF.md`의 옛 경로1개도
함께 교정했다. C++ 컴파일 목록·옵션, source/header, 데이터 내용, 물리 폴더는 바꾸지 않았다.

독립 검토에서 이름 기반 분류를 교정했다. `AreaLightAuthoringSession`은 Tools의 authoring,
실제 NPC pose에 쓰이는 `KoukuSaydonAnimationBlend`는 Presentation에 둔다.
`AnimationSkillBindingDocument`와 공용 cue 문서는 runtime consumer를 고려해 분류했다.
`ArtistNativeSelectedGroup3968`은 실제 Kouku include이므로 Kouku,2304는 World/Kouku 혼합이므로
Shared다. selected wrapper51개와 공통/dispatch를 포함한118개 분류 근거를 확인했다.

## G03. 실행한 검증

| 검사 | 결과와 범위 |
|---|---|
| Debug Product Build |PASS,799,798ms,정상 Build·SkipBuild=false|
| Engine 단계 |620,337ms,OBJ23·PCH0·CSO30·binary2 쓰기|
| Shared/Server 단계 |411ms/532ms,기존 결과 재사용|
| Client 단계 |175,214ms,OBJ159·PCH0·배포 CSO30·binary2 쓰기|
| runner runtime 입력 검사 |missing0,invalid0; 전체 runtime domain의 내용·실행 보증은 아님|
| 프로젝트/필터 XML |네 프로젝트와 filters parse,최종 item 다중집합 일치|
| 구조 |중복 item·GUID·필터명·미선언/부모누락·미분류·중첩 item·누락 물리 경로0|
| H/CPP 및 Data |동일 stem의 필터 분리0,96.DataFiles의 non-None 또는 비Data 항목0|
| 등록 보존 |필터 외 item metadata 보존,컴파일/헤더/shader 등록·옵션 무변경;None 경로2개만 교정|
| 문서 근거 |소스 링크213개의 파일 존재·행 범위 확인;심볼과 핵심 흐름은 실제 코드로 별도 조사|
| JSON 및 whitespace |생성한 파일 색인·BuildDomains·UI layout2개 parse,git diff --check|

빌드 증거는 `out/BuildPipeline/runs/20261002T221727726Z-debug-product.json`과
`out/CodeAtlas/product-debug.log`다. 빌드 시작 시 기존 VS18 Insiders/amd64 MSBuild18.9,
v14314.44.35207,Windows SDK10.0.26100.0과 x64/Native64Bit host를 유지했다.
위 CSO30개는 Engine producer의 출력과 Client 배포 사본으로 중복 컴파일60개가 아니다.
출력 수는 크기/mtime 변경 기준이며 CPU 순수 compiler 시간이나 성능 개선 배율이 아니다.
기존 C4819·FXC warning은 남아 있다.

빌드는 main의 C++/HLSL 병합을 검증하기 위해 수행했다. 그 뒤의 필터 최종 분류와
UI `None`2개 경로 교정은 컴파일 입력을 바꾸지 않아 재빌드하지 않았다.
구조 상세 증거는 `out/CodeAtlas/final-filter-validation.json`,
`engine-filter-validation.json`, `server-shared-filter-verification.json`이다.
현재 Debug 실행 파일은 `Client/Bin/Debug/Client.exe`, `Server/Bin/Debug/Server.exe`다.

## G04. 학습 문서와 범위

- [Visual Studio 코드 지도](2026-10-03_VISUAL_STUDIO_CODE_ATLAS.md)
- [전체 필터 트리](2026-10-03_VISUAL_STUDIO_FILTER_TREE.md)
- [3361개 등록 파일 색인](2026-10-03_VISUAL_STUDIO_FILE_INDEX.json)
- [Workbench와 Sequencer](2026-10-03_ACTION_WORKBENCH_CODE_GUIDE.md)
- [렌더링·물·스킬 shader·Profiler](2026-10-03_RENDERING_SHADER_PROFILER_CODE_GUIDE.md)
- [WintersEngine과 Unreal 비교](2026-10-03_WINTERS_UNREAL_COMPARISON_CODE_GUIDE.md)
- [전체 XML 정본 PLAN](2026-10-03_VISUAL_STUDIO_DOMAIN_FILTERS_PLAN.md)

WintersEngine은 실제 source/filter를 읽었으며 수정·빌드하지 않았다. 최초 조사 때 없었던
Unreal 소스는 이후 사용자 로그인과 다운로드 요청에 따라 공식 저장소에서 clone했다.
`C:/Users/tnest/Desktop/UnrealEngine`의 release commit은
`396c9f059903aed5fec78ecd3d437a40c6415368`이고 `Build.version`은 5.8.3이다.
깊이1 clone으로 현재 추적 파일224,004개 checkout이 완료됐으며 작업 트리는 clean이다.
과거 Git 이력과 Setup의 외부 binary dependencies는 다운로드하지 않았다.
소스 디렉터리의 파일 길이 합계는 `.git` 포함3,558,249,571byte(약3.31GiB)였다.
`git fsck --connectivity-only`도 exit0으로 완료됐다.

이 소스의 Sequencer/MovieScene·AssetRegistry·package Save/Cook·RDG/RHI·shader compiler·
Trace/Insights·GC·Camera·World·BehaviorTree를 세 코드 가이드의 실제 경로·행과 연결했다.
이것은 소스 조사이며 Unreal 엔진 빌드·실행·시각 품질·성능 비교 결과가 아니다.
추가 조사 반영 후 다섯 문서의 절대 로컬 링크401개의 파일 존재·행 범위 오류0과
`git diff --check` 통과를 확인했다. 문서만 변경하여 추가 컴파일은 수행하지 않았다.
이 추가 조사 문서는 조사 당시 미커밋 상태로 보존했으며, 2026-10-05 main 동기화에서
별도 문서 커밋으로 전달한다. 당시 검증 범위와 위의 조사 수치는 그대로 유지한다.

## G05. 미실행과 후속 작업

VS/Client/Server를 자율 실행·조작하거나 화면을 캡처하지 않았다. 필터의 실제 VS 표시,
새 SSGI/SSR 화질·FPS·A/B,skill/물 occurrence의 사용자 화면 판단은 미검증이다.
Release Product,광역 regression,새 runtime publish는 수행하지 않았다.

문서는 핵심 API·소유 상태·자료구조·저장 계약과 알고리즘을 이해하기 위한 첫 지도다.
모든 함수·변수 전수 설명,Unreal 추가 기능 대조,새 영상·SRT·자기소개서·최종 포트폴리오 및
200쪽 PDF는 완료로 기록하지 않는다. 다음 설명은 새 필터에서 한 기능의 실제 코드·데이터·
도구 조작을 함께 따라가는 단위로 이어간다.

## G06. 커밋과 전달

현재 사용자 작업 폴더의 필터·문서 commit은 `3b675113`이다. 기존 캐릭터 기능이 PR에
섞이지 않도록 `origin/main c7de2091` 기준의 관리 worktree에 이 변경만 옮겼고,
`codex/visual-studio-domain-filters`의 `415bd889`로 push했다. 네 filters, Client의
`None` 경로2개, 문서만 포함하며 그 branch에는 별도 캐릭터 기능 commit이 없다.

GitHub connector의 draft PR 생성은 `403 Resource not accessible by integration`으로
거절되어 PR은 생성되지 않았다. 이것은 코드 검증 실패나 자동 승인 검토 거절이 아니라
GitHub 연동의 해당 API 권한 제한이다. 원격 branch는 정상적으로 존재한다.
[main과 필터 branch 비교 및 PR 작성](https://github.com/tnestyle70/LostArk/compare/main...codex/visual-studio-domain-filters)

## G07. Unreal 실행 환경 인계

공식 unrealengine.com 다운로드 페이지에서 받은 런처 설치 파일은
`C:/Users/tnest/Downloads/EpicGamesLauncherInstaller.exe`에 있다. 크기264,283,136byte,
file version2026.0709.1935.0이며 Authenticode 검사는 Valid/Epic Games Inc.였다.
실행 요청은 보냈으나 도구가 targetable window를 확인하지 못했다. 설치 성공으로 기록하지 않는다.
이후 사용자가 Launcher와 Unreal 에디터를 직접 다운로드·설치하겠다고 하여 설치 조작을 중단했다.
사용자가 진행하는 설치 프로세스를 닫거나 다시 실행하지 않았다.

`Setup.bat`·`GenerateProjectFiles.bat`·Unreal 소스 빌드는 수행하지 않았다.
에디터 설치 완료·실행 성공·도구 화면 비교는 아직 확인하지 않았다. 사용자 목표인10월30일
지원에 맞춘 영상·기술소개서 준비 흐름과 쿠크 뿅망치 촬영 후보는 코드 지도 G10에 반영했다.
