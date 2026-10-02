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

WintersEngine은 실제 source/filter를 읽었으며 수정·빌드하지 않았다. Unreal 소스 부재는
사용자가 확인했다. Unreal 부분은 Epic 공식 문서/API를 근거로 비교하고 private repository
접근·계정 연결 절차를 안내했다. 사용자 GitHub 권한을 확인하거나 clone을 수행하지 않았다.

## G05. 미실행과 후속 작업

VS/Client/Server를 자율 실행·조작하거나 화면을 캡처하지 않았다. 필터의 실제 VS 표시,
새 SSGI/SSR 화질·FPS·A/B,skill/물 occurrence의 사용자 화면 판단은 미검증이다.
Release Product,광역 regression,새 runtime publish는 수행하지 않았다.

문서는 핵심 API·소유 상태·자료구조·저장 계약과 알고리즘을 이해하기 위한 첫 지도다.
모든 함수·변수 전수 설명,Unreal 로컬 소스 대조,새 영상·SRT·자기소개서·최종 포트폴리오 및
200쪽 PDF는 완료로 기록하지 않는다. 다음 설명은 새 필터에서 한 기능의 실제 코드·데이터·
도구 조작을 함께 따라가는 단위로 이어간다.
