# 10월 2일 렌더링 옵션 복원 구현 계획

## G00. 기준과 실제 차이

사용자 요청은 Git 10월 2일의 렌더링 옵션으로 돌아가되 무비·오클루전 컬링 등 최적화는
유지하는 것이다. 현재 ancestry에서 KST 2026-10-02 마지막 first-parent commit은
`a0ff0185cc7fea171835cde15777c0f520d9d0ae`(11:28:49)다.
작업 시작 HEAD는 `28c8c98856de479b95e453147bd9d6684e86fb24`, 작업 트리는 clean이다.

RenderingProfiles 저작/게시본의 실질 차이는 `scene.bern.neutral-day.v1.fog.enabled`의
true→false 한 필드와 revision 90→91이다. 전역·profile·region의 나머지 품질값,
LightResources, maplights와 Imported renderprofiles는 해당 기준과 같다.
10월 4일 사용자의 안개 OFF 요청은 당시 RESULT에 남기고 이번 명시적 날짜 복원 요청을 적용한다.

## G01. stable profile의 한 필드 복원

최신 `Data/Rendering/Authored/RenderingProfiles.json`을 읽어 stable profile ID로 해당
fog.enabled만 true로 바꾼다. revision은 과거로 낮추지 않고 92로 증가시킨다.
후보를 `out/RenderingOptions20261002Restore20261007`에서 publisher Validate로 검사한다.
기존 파일 백업과 교체 직전 hash 확인 후 원자 교체하고 정본 publisher로 runtime을 게시한다.
게시 전 runtime hash도 재확인하며 실패하면 자신의 source 변경만 rollback한다.

`Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Publish`가
`Client/Bin/DataFiles/Rendering/RenderingProfiles.runtime.json`을 생성한다.
JSON을 통째로 과거 파일로 되돌리거나 생성물을 수동 편집하지 않는다.

## G02. 최적화와 설정 경계

무비/NPC 포즈 재사용, instancing·lighting bank·LOD, 정적 그림자 캐시,
오클루전·거리 컬링과 CPU/particle 최적화 코드는 유지한다.
SSGI·SSR·Horizon AO·SSR 추가 필터는 현재 기본 OFF이고 SSAO12샘플/PCF3×3는 기존과 같다.
Release 텍스처 기본 mip0은 10월 2일의 원본 sampler 품질과 같다.
개인 UserSettings와 새 무대 배치·재질 복구·Resources는 날짜가 있는 옵션 저장본과 별개이므로
복원 대상으로 섞지 않는다. 새로운 C++/HLSL과 프로젝트/filter 등록 변경은 없다.

## G03. 검증과 사용자 확인

publisher Validate/Publish, 중복 키·JSON parse, baseline과 revision을 제외한 구조 동등성,
변경 필드 제한과 `git diff --check`를 확인한다. 데이터만 바뀌므로 제품 재컴파일은 필요 없다.
기존 Release 실행 파일과 노란 FPS는 유지한다. Client/UI는 자동 실행하지 않으며
재실행 후 실제 안개·FPS와 프레임 드랍 원인은 사용자가 확인한다.
