# 쿠크 렌더링 품질과 Pattern 삭제 구현 계획

작성일: 2026-09-19. 기존 09-07 바닥 복구 RESULT와 09-15 맵·발탄 복구 RESULT의 남은 직접광 경계를 이어서 조사한다. 기존 미커밋 조명·패턴 문서와 다른 세션 소유 코드는 보존한다.

## G01. 선택 바닥의 원본 직접 반사

`Engine/Bin/ShaderFiles/Shader_Deferred.hlsl`의 marker 1은 쿠크 SL05 FLOOR08/FLOOR08A의 원본 표면 RGB를 받지만 legacy Phong 반사각에 원본 power 30/100을 적용한다. 09-15 수정은 incident light를 legacy specular에서 diffuse RGB로 연결했으며 원본 Blinn lobe를 복원한 것은 아니다. SL04의 RNM 바닥은 BG8로 이미 별도 Blinn 경로이므로 이번 차이를 모든 관문이나 첨부 화면의 단독 원인으로 일반화하지 않는다.

기존 UPK와 shader cache에서 두 MIC의 실효 static set·LocalVF directional DXBC를 다시 읽고, half-vector와 RGB cap의 원본 연산을 확인한다. marker 1에만 같은 반사 계산을 연결하고 marker 0/2와 모든 다른 native family의 연산을 보존한다. diffuse·ambient·shadow·attenuation·재질 파라미터·Resources 파일은 변경 대상이 아니다. 두 표면은 같은 Blinn lobe를 갖지만 09b만 shadow 뒤 RGB cap 2를 갖는다. 현재 두 MIC의 흰 specular color × intensity 0.2에서는 texture·lobe·shadow가 포화 범위여서 cap 2에 도달하지 않는다. 이번 변경은 공통 lobe 복원으로 한정하고 cap용 GBuffer 확장은 하지 않는다.

기존 D3D11 focused fixture에서 정면 광원과 비정면 카메라 각도, 두 power, Directional/Point/Spot 및 single/batch를 비교한다. 회귀 대조군은 marker 0/2다. 실제 설치 shader를 root의 단일 Product 빌드로 생성한다. 자동 수치 증거와 사용자 화면 승인을 RESULT에서 구분한다. 신규 C++ 파일이나 프로젝트 등록은 없다.

## G03. Pattern 삭제와 연결 정리

`Client/Private/KoukuSaydonActionWorkbench.cpp`의 삭제 확인은 단순 Flow/Bundle/Parent 연결도 모두 필수 참조로 취급한다. 조사 시점의 93 Pattern 중 65개에 하나 이상의 연결이 있고, 다수는 Flow 또는 Bundle 연결만으로 삭제 버튼이 비활성화된다. 삭제 함수의 디스크 freshness 검사는 draft 편집까지 막지만 실제 파일 보호는 `CKoukuSaydonCompositionDocument::Save_Atomic`이 이미 소유한다.

삭제 확인에 함께 지울 연결을 표시한다. 사용자가 Delete from Draft를 누르면 대상 Pattern과 그 Pattern의 Flow 행, Bundle 멤버, Parent Pattern Box, Folder timeline 포인터를 하나의 candidate에서 정리한다. 영향을 받는 Parent/Bundle은 기존 DRAFT 전이 규칙을 적용한다. 다른 Pattern의 시간·리소스·필수 Logic/Summon target은 임의 변경하지 않는다. 필수 target이나 손상되어 안전하게 편집할 수 없는 참조는 구체적인 owner와 함께 계속 거절한다.

`Collect_PatternDeleteReferences`는 삭제 transaction이 정리할 연결과 사전 수정이 필요한 연결을 구분해 같은 UI/실행 조건에 사용한다. candidate 전체 Validate에 성공한 경우에만 기존 Commit_Candidate로 교체하고, Save는 기존 revision/hash 비교·백업·원자적 교체를 유지한다. authoring JSON의 실제 Pattern은 에이전트가 삭제하지 않는다. 기존 `ValtanPatternAuditionServiceHarness`의 실제 Workbench/Document 경로로 Flow 연결 삭제, Bundle/Parent 연결 정리, 필수 참조 거절, 실패 시 기존 draft 보존 및 Save/reload를 검사한다. 실제 버튼 조작은 사용자 확인으로 남긴다.

출처는 설치된 native shader cache의 exact static equality set과 LocalVF directional PS다. 09b shader ID는 526df319c686eb43b1c7d463fc688eda, 08b는 b8160ab1e4eed948b85f2389b3e8523b다. 입력·DXBC hash와 focused 검증 범위는 out/KoukuRenderingQuality20260919/verification.receipt.json에 보관하고 통합 빌드 및 실제 완료 판정은 root RESULT에서 합친다.

## G04. 사전 컴파일 shader closure의 기존 Modulate 계약

ProductEffectShaderWarpProbe는 particle pass를 모두 5개로 가정하지만 source family 7은 09-15부터 native 단면 sprite Modulate pass를 index 5에 추가해 6개다. runtime Select_Pass와 shader 선언을 정본으로 하여 ARTIST particle에만 6개와 정확한 MultiplyOneSidedDepthRead 이름, SourceModulatePS, RT0 Dst*Src와 alpha 보존·RT1/RT2 차단을 검사한다. 다른 family의 개수·순서·입력 layout·필수 constant 검사는 유지한다. 제품 source나 CSO는 수정하지 않고 Release Product closure를 다시 실행한다.
