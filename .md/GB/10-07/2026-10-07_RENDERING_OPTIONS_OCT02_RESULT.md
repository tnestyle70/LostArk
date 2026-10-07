# 10월 2일 렌더링 옵션 복원 결과

## G00. 반영한 옵션

기준은 현재 ancestry의 KST 2026-10-02 마지막 first-parent commit
`a0ff0185cc7fea171835cde15777c0f520d9d0ae`(11:28:49)다.
`Data/Rendering/Authored/RenderingProfiles.json`과 정본 publisher가 만든
`Client/Bin/DataFiles/Rendering/RenderingProfiles.runtime.json`은 revision을 제외하고
이 기준의 저장 옵션과 구조적으로 완전히 같다.

실제 변경은 `scene.bern.neutral-day.v1.fog.enabled` false→true 하나다.
revision은 최신 저장본 91에서 92로 올렸다. 나머지 전역/profile/region의
SSAO·Bloom·FXAA·노출·감마·조명·그림자·환경 입력은 원래 기준과 같았다.
특히 쿠크 Mario1~4의 기존 FXAA OFF를 보존했다.

10월 4일의 안개 OFF 요청과 적용 기록은 당시 RESULT에 보존하고,
이번 사용자의 명시적인 10월 2일 옵션 복원 요청에 따라 ON으로 돌렸다.
안개가 다시 켜지는 변경이며 성능 개선을 확인했다는 뜻이 아니다.

## G01. 보존한 구현과 조사 결과

무비/NPC 포즈 재사용, 오클루전·거리 컬링, LOD, instancing·lighting bank,
정적 그림자 캐시, CPU/particle 최적화와 Release 노란 FPS 표시는 그대로다.
이번 변경에서 C++/헤더/HLSL·카메라·무비·맵 재질·Resources는 수정하지 않았다.

새 SSGI/SSR/Horizon AO/SSR 추가 필터는 기본 OFF다. SSAO12샘플과 PCF3×3,
shadow map2048, Release 원본 texture mip0 및 기존 Engine 품질 기본값은 기준과 같다.
Debug의 낮춘 텍스처 누락 기본값은 기존 최적화로 유지했다.
세션 A/B는 시작 시 비활성이며 저장 옵션의 자동 재설정 경로로 바꾸지 않았다.

LightResources, maplights, Imported renderprofiles와 Data/UI도 기준 대비 변경이 없었다.
그 사이 추가된 foliage wind·landscape baked lighting·재질 복원은 옵션 저장값과 다른
코드/데이터 기능이다. 이를 통째로 과거 문서로 덮어쓰지 않았다.
개인 UserSettings는 Git 기준값이 없으므로 보존했다. 현재 Release의 texture0,
AA0·SSAO0·Bloom1·gamma50은 원본 품질과 프로필 값을 증폭 없이 사용한다.

## G02. 검증과 적용 경로

- 후보의 정본 publisher Validate PASS: 중복 키·필드·값·profile/region 계약 검사.
- 최신 source/runtime hash 재확인과 source writer lock, 백업, 원자 교체 후 Publish PASS.
- 게시 runtime을 SourcePath로 지정한 정본 Validate PASS.
- Python JSON 구조 비교: source/runtime 동일, 10월 2일 baseline과 revision 외 동일,
  이전 HEAD에서 fog.enabled/revision 외 변경 없음.
- Engine/Client 코드·shader·카메라 변경 없음, `git diff --check` PASS.
- 데이터만 바뀌어 제품 재빌드는 하지 않았다. 앞선 Release FPS 빌드는 그대로 사용한다.

준비·적용 receipt와 백업·게시 로그는
`out/RenderingOptions20261002Restore20261007/{prepared.json,applied.json,publish.log}`에 있다.
Release/Debug 실행 폴더 안의 별도 DataFiles override가 없음을 확인했으므로
기존 `Find_RuntimeCatalog`는 게시한 공용 `Client/Bin/DataFiles/Rendering` 파일을 소비한다.

## G03. 사용자 확인과 미검증

Client/Server/UI를 자동 실행하거나 캡처하지 않았다. 새로 실행하면 게시한 설정을 읽는다.
실행 중 프로세스의 메모리 draft/세션 A/B 상태가 자동 Reload됐다고 주장하지 않는다.
실제 화면·FPS·프레임 드랍 원인은 미측정이다. 이번 작업은 설정 복원이며 10월 2일의
shader/material 구현이나 전체 화면/성능을 그대로 재현한 것은 아니다.
