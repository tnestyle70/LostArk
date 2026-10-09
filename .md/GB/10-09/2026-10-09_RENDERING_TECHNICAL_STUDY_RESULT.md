# LostArk 렌더링 기술소개서 해설 작성 결과

2026년 10월 9일. 기준 HEAD `45faba4ba0806cbfbcfd02b213404fb98901b961`, 작업 브랜치 `codex/server-iocp-job-comparison`의 현재 소스·저장 데이터를 읽었다. 기존 Engine/Public/GameInstance.h 변경과 IOCP 문서들은 보존했다.

## 작성한 내용

[렌더링 기술소개서 해설](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_RENDERING_TECHNICAL_STUDY.md)에 다음 내용을 정리했다.

- PPT 11~14쪽의 현재 텍스트와 개념·범위 교정.
- 현재 프레임의 deferred/forward 합성 순서, PBR 직접광과 ORM 입력.
- native SH·RNM·RGBM cube·128×32 BRDF lookup 및 실제 계수의 계산 예.
- Bern/Valtan/Kouku/Character Select의 저장값과 재질별 적용 범위.
- SSAO 차폐의 의미, 실제 4/8/12 표본과 half/5×5 bilateral 경로.
- Bern water41 실제 입력·부분식·scene snapshot, source water42 카메라 입력 복구 사례.
- 차원술사 Dynamic 기본값·Q CubeSample·tone/LUT, Valtan Trail UV의 복구 사례.
- 현재 선택형 Horizon AO/SSGI/SSR과 향후 DXR/Lumen의 구분.
- 구조적 연산량·복사 payload 계산, 기존 raw capture의 재집계와 비A/B 한계.
- PPT 11~14쪽에 옮길 수 있는 발표 문장과 코드 학습 순서.

구현을 제안하는 전체 코드 PLAN이 아니라 현재 구현의 설명 자료다. 새 GI/RT 구현이나 PPT 편집은 하지 않았다.

## 직접 확인한 자료

원본 PPT는 `C:/Users/tnest/Desktop/Interview/기술소개서/기술소개서.pptx`다. ZIP의 presentation 관계와 slide XML에서 순서를 확인했다. 전체 37쪽이며 11~14쪽은 텍스트 초안이고 해당 슬라이드에 직접 연결된 image 관계는 없다. PowerPoint를 실행하거나 화면을 렌더·캡처한 것은 아니다.

읽기 전후 PPT SHA-256:

```text
6cc18b3ebccba9a4bea2a9098120a4c40749fe486b7318af63b41f43958c4975
```

RenderingProfiles authoring/runtime JSON은 revision93으로 파싱 결과가 동일하다. 4개 Area material 문서를 읽어 material/placementLighting/sourceIndirect 행을 집계했다. 실제 load scope나 가시 draw 수로 해석하지 않았다.

기존 2026-10-07 profiler raw 390프레임을 다시 읽었다. GPU valid386/pending4, nearest-rank P50/P95와 pass duration 합산을 확인했다. 원본 캡처의 Debug·840M·export metadata·가변 workload 한계를 본문에 유지했다. 새 성능 측정을 실행한 것은 아니다.

## 문서 검증

- PBR/IBL, SSAO/GI/비용, 물/이펙트의 3개 독립 조사와 검토를 통합했다.
- review에서 지적한 native roughness의 공간 미분, normal 내적 saturate, 물 program별 Baked 지원, Trail 검증 범위, TLAS 구성 설명을 교정했다.
- 본문 47개 로컬 링크의 파일 존재와 지정 행 범위를 확인했다.
- Markdown fence 쌍과 후행 공백, UTF-8 읽기를 확인했다.
- 문서 범위 `git diff --check`와 신규 문서의 `git diff --no-index --check -- NUL <문서>`에서 whitespace 결함은 없었다. no-index의 차이 존재 exit1과 LF→CRLF 안내는 whitespace 결함과 구분했다. 종료 시 전체 worktree 검사는 이번 범위 밖 `Engine/Private/GameInstance.cpp:64`의 후행 공백을 보고했다. 해당 변경은 수정하지 않았다.
- SSAO 가정 표본, 물 depth/opacity, 1080p 후보 반복·복사량 산술을 Python으로 재계산했다.

최초 작업 당시 근거는 Git 제외 `out/RenderingTechnicalNote20261009`에 `pptx_inspection.json`, `structural_calculations.json`, `capture_reanalysis.json`, `document_validation.json`과 분야별 조사 메모로 보관했다. 2026-10-09 통합 전 재확인 시 이 폴더는 현재 디스크에 없었다. 위 검증은 이 RESULT에 남은 당시 기록이며, 원시 조사 자료를 이번에 다시 대조하거나 검증을 재실행한 결과로 취급하지 않는다.

## 남은 검증과 변경하지 않은 범위

원본 PPT, 제품 C++/HLSL, Data/Resources, 게시 runtime, 사용자 렌더링 설정을 변경하지 않았다. 문서만 추가했으므로 제품 컴파일·shader 컴파일·publisher·Client/UI 실행과 새 화면 판정을 하지 않았다. 기존 WARP·native parity 결과는 해당 RESULT의 과거 증거로 인용했으며 이번 재실행 성공으로 기록하지 않았다.

작업 중 Server IOCP와 BossToolTests rename 등 다른 범위의 변경이 worktree에 추가된 것을 종료 시 확인했다. 이 세션은 그 파일들을 정리·stage·commit하거나 되돌리지 않았다.

원작과 동일 장면의 최종 화면 비교, 통제된 실제 GPU A/B, water/Glasshole/SSGI/SSR의 개별 추가 ms, DXR 또는 Unreal 이관 구현은 완료 항목이 아니다. 문서의 향후 순서와 비용 표 기준으로 별도 진행해야 한다.
