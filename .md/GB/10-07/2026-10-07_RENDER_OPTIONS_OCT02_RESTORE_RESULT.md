# 2026-10-07 렌더링 옵션의 10월 2일 값 복원 결과

> 최신 상태: 같은 날 사용자의 데스크탑 Git 동기화 요청으로 Bern fog는 다시 OFF로 설정하고
> revision 93으로 게시했다. 아래 revision 92의 fog ON은 이전 단계의 이력이다.
> 최종 상태와 무비 시작 지연 조사 범위는 `2026-10-07_DESKTOP_RENDER_SYNC_RESULT.md`를 따른다.

## 완료 범위

사용자가 요청한 Git 10월 2일 기준은 `a0ff0185cc7fea171835cde15777c0f520d9d0ae`다.
`Data/Rendering/Authored/RenderingProfiles.json`에서 `scene.bern.neutral-day.v1.fog.enabled`만
`false`에서 `true`로 복원하고 revision은 91에서 92로 올렸다. 공식 Rendering publisher로
`Client/Bin/DataFiles/Rendering/RenderingProfiles.runtime.json`을 생성했다.

`Client/Private/UserSettingsDocument.cpp`의 Debug 텍스처 기본값만 3에서 0으로 복원했다.
Release 기본값 0, 명시 저장값 우선, 사용자가 선택하는 품질 기능과 sampler 구현은 유지했다.
기존 `UserSettingsContractHarness.cpp`의 Debug 기대값과 문구를 이에 맞췄다.
CLAUDE, gotchas, 렌더링이펙트복원V2 및 해당 harness README의 현재 기본값 설명도 교정했다.

무비·오클루전·거리 컬링·LOD·batch 최적화, 신규 Workbench 실험 기능과 개인 저장 설정은
이번 복원에서 변경하지 않았다. 다른 세션의 GPU 선택·showcase 수정은 별도 작업이다.

## 실행한 검증

- 구현 전에 대응 PLAN에 두 CPP의 최종 전문과 변경 JSON 블록을 기록했다.
- 후보 JSON Validate exit 0, 최신 hash 재확인 및 원본 백업 후 원자 교체 성공.
- 설치 정본 Validate exit 0, 공식 publisher Publish exit 0.
- 독립 읽기 전용 리뷰: authored/runtime JSON 의미값 일치, revision 92.
- 기준 commit과 비교하면 revision 90→92 외 기존 29개 profile과 모든 옵션값이 동일하다.
- 개인 UserSettings.json의 SHA-256은 전후
  `AC4A6C8CBFC7B80FF3AA09A7DB3DB97154BDB2682709400210FD56E66EA0747A`로 동일하다.
- `git diff --check` 통과. 이번 범위 밖 기존 문서에는 Git의 LF→CRLF 안내가 있다.
- 원본·후보·준비 hash·원자 백업은 Git 제외 `out/RenderOptionsOct02Restore20261007`에 보존했다.

## 별도 진단과 미검증 경계

확인 당시 Debug Client PID 3504의 높은 3D 사용량은 LUID
`0x00000000_0x00017055`의 AMD Radeon 840M이었다. RTX 4050 Laptop의 LUID는
`0x00000000_0x000196d1`이다. DXGI 조회 근거는 `out/FrameDrop20261007/dxgi-adapters.log`다.
정확히 일치하는 PDB를 이용한 읽기 전용 프레임 번호 표본은 5.339887초 동안 51프레임,
약 9.55 FPS였다. 이 관측은 이번 옵션 복원이나 다른 세션의 GPU 수정 효과를 측정한 것이 아니다.

사용자가 다른 세션에서 반영·빌드 중이라고 알려 이 세션에서는 Product 및 계약 fixture를
빌드하거나 실행하지 않았다. 다른 세션의 빌드 성공을 이 문서의 검증으로 대신하지 않는다.
실행 중 Client의 Reload, GPU 변경 후 재실행과 FPS·화면 확인도 이 세션에서는 수행하지 않았다.
게시 파일 변경은 이미 실행 중인 메모리 설정의 자동 갱신을 뜻하지 않는다.
