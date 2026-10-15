# 보스·렌더링·이펙트 기술소개서와 실제 코드 학습 결과

2026-10-15. 최신 요청은 소개서의 구조를 먼저 완성하면서 P13의 실제 Python 코드, 수업23일차 모델·animation, 현재 PBR/SH/BRDF/IBL, 도화가 F 복원 과정을 한 줄씩 공부하는 것이다. 구조 리팩터링·패턴 수정 작업의 재개가 아니다. 사용자 직접 구현 범위를 유지하고 제품 C++·저작 JSON·게시 데이터·Resources·렌더링 옵션을 변경하지 않았다.

## 산출물

- [통합 학습 색인](C:/Users/tnest/Desktop/Interview/projects/lostark/technical/2026-10-15-boss-rendering-effects/README.md)
- [60페이지 소개서 사본](C:/Users/tnest/Desktop/Interview/out/2026-10-15-technical-deep-dive/기술소개서_보스_렌더링_이펙트_구조와근거.pptx)
- [구조·촬영 계획](C:/Users/tnest/Desktop/Interview/projects/lostark/technical/2026-10-15-boss-rendering-effects/00_소개서구조와촬영계획.md)
- [P13 실제 함수·행별 설명](C:/Users/tnest/Desktop/Interview/projects/lostark/technical/2026-10-15-boss-rendering-effects/01_P13_JSON부터실행정의까지.md)
- [수업23일차 비교](C:/Users/tnest/Desktop/Interview/projects/lostark/technical/2026-10-15-boss-rendering-effects/02_수업23일차에서현재렌더러까지.md)
- [현재 PBR·SH·IBL 전체 함수와 수식](C:/Users/tnest/Desktop/Interview/projects/lostark/technical/2026-10-15-boss-rendering-effects/03_PBR_SH_BRDF_IBL.md)
- [도화가 F 복원 기록·현재 연결](C:/Users/tnest/Desktop/Interview/projects/lostark/technical/2026-10-15-boss-rendering-effects/04_도화가F_이펙트복원.md)
- [이펙트 함수 전문](C:/Users/tnest/Desktop/Interview/projects/lostark/technical/2026-10-15-boss-rendering-effects/05_이펙트함수전문.md)
- [페이지별 발표 원고](C:/Users/tnest/Desktop/Interview/projects/lostark/technical/2026-10-15-boss-rendering-effects/06_페이지별발표원고.md)

## 현재 사실과 설명의 교정

1. 발탄과 쿠크는 공통 Room·TCP·표현 경계를 쓰지만 저작 schema와 전용 pipeline은 다르다. Data는 저작 정본, publish는 검증된 소비자별 실행본을 만드는 단계다.
2. Python이 Brain 메모리를 직접 만드는 구조가 아니다. Composition dict/list→projected Encounter→Gameplay bootstrap→CGameplayCatalog의 typed 정의→Room 실행 경계를 구분했다.
3. 한글 displayName은 표시 데이터다. stable ID 조회와 kind 분기가 동작을 선택한다. P13 첫 접촉은4858~5902ms, L22→L24/L26, 조커 identity는world.11이다.
4. 수업23일차에도 Assimp·본 보간·palette·기본 Deferred가 있었다. 현재의 WModel·native 재질 descriptor·색공간·환경 입력·효과 family/ABI 확장과 구분했다.
5. SH diffuse, RNM, specular cube, BRDF LUT, 색보정 LUT는 입력과 책임이 다르다. 현재 native BRDF128×32 RG16과 project BRDF128×128 RGBA16F를 분리했다. MapPBR 식을 모든 캐릭터 shader와 같은 식으로 설명하지 않았다.
6. 도화가 F 현재 cue는unified17개(particle14/decal2/trail1)다. 원본 active35개와full.restore33개를 현재 제품 사용 수량으로 섞지 않았다. raw DXBC 수식 비교와 현재 bounded HLSL 실행을 구분했다.
7. 사용자가 말한 셰이더50%는 코드 비중이다. 추적된 소스 확장자3545개 중 bytes 기준49.7484%, 물리 줄 수43.8101%였다. 생성·외부·Engine/Client 미러를 포함하며 GitHub Linguist 재현이나 개인 수작업 비율은 아니다.
8. 과거106개 이펙트 CPU HistoryUpdate53.13% 감소는 별도09-09 RESULT 근거다. Debug headless 각1회 합계이며 Client FPS/GPU 개선율로 표기하지 않았다.

## 실행한 검증

- 일일 branch/status/fetch 확인. 다수 동시 변경을 보존했다. 10월3일 만료 LAN 설정을 AllowExpired로 실행하지 않았다.
- 원본 PPT44쪽 SHA-256 `b47e0d7ea7b78848bd24ac105d8c398d1194be475705aca20231dd4aa0273808` 재확인·보존. 관련9~22·31쪽15개를 교체하고 부록16개를 추가했다. 무관한29개 slide XML과 기존의 다른 package part는 명시된 추가/수정 부품을 제외하고 보존했다.
- P13 실제load_json(use_cache=False), 기본 window/outcome/serialize를 AST로 추출 실행.21개 필드가 저장 Encounter와 일치. 중복 key 거절·입력 dict 불변·13개 전체함수와2개 발췌의 원문/hash·로컬링크 검사 PASS. bone bake/전체publisher는 미실행이다.
- 렌더링 입력 JSON/설치DDS header·hash·현재 함수 근거 조사. 도화가F 현재17개 execution 정보와6개 shader/ABI 함수 hash 대조.
- PPT는 `@oai/artifact-tool`로31개 편집 가능한 slide를 저작하고 기존 package에 병합했다. 최종60개 slide를 전부 이미지로 렌더했다. 새로 작성한 페이지를 개별 크기로 검토하고 보존 페이지는 전체 흐름을 스캔했다.
- 새31개 slide의 package/layout/font 검증: error0·warning0. 전체본 package/layout error0, 기존 보존 페이지의 경고23개(본문 밖 객체·기존 연결선) 별도 기록. 소스의 East Asian font inheritance26건 때문에 전체 reference-font 자동 검사는 해결되지 않았으며 원본을 고쳐 검사를 우회하지 않았다. 새 slide에 Malgun Gothic/Consolas를 명시하고 별도 font 검증을 통과했으며 원본 theme와29개 slide를 byte 보존했다.
- 최종PPT SHA-256 `3203aa7b212f49ad02464e6427a8039a3f308f2b696d72ee8bdb02224a620087`.
- 산출물·원문·기존검증 경계는 `out/TechnicalDeepDive20261015`와 학습자료의evidence에 기록했다. 검증 도구의 경고·실제 화면 검사·과거 RESULT의 검증을 서로 구분했다.

## 하지 않은 작업과 다음 실제 화면 자료

제품 빌드, 실제 게임·Client·ImGui 조작, runtime publish/Reload, 전체화면 촬영을 하지 않았다. 과거 성능 수치를 오늘 재측정하지 않았다. 원본11쪽 부분 이미지는 사본에서 구조도로 교체했다. P13저작/접촉결과, 같은카메라의복원단계A/B, 재질입력, 도화가F 전체재생은 사용자의 전체viewport 촬영 목록으로 남겼다. 추가 화면 없이 코드·수식·구조 설명이 완성된 현재 문서와 실제 게임 화면 검증은 구분한다. PowerPoint앱에서의 native 재생 검증은 수행하지 않았다.
