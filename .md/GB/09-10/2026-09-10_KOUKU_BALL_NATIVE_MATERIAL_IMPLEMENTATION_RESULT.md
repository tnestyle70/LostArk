# 쿠크 공 원본 PBR 재질 연결 결과

## 구현

공 `mn_rhcn_00_mi`의 원본 monster PBR Base/Light pair를 기존 SourceCharacter program 21로 연결했다. 원본 MIC의 41개 parameter와 N/D/S/IBL/black/E/state/LUT 8개 texture를 optional World Object materialProfile로 저장·검증하고 기존 CModel → CMaterial 경로가 소비한다. static World Object 렌더에서 generic reset 뒤 native material을 실제로 bind한다. 기존 program 1~20과 공 model/transform은 보존했다.

공의 S는 일반 specular 색이 아닌 원본 PBR mask다. roughness power 3, metalicness power 0.2, `hdr07_1` IBL, `brdf_beckmann_spec` LUT 및 emissive 시간식을 복구했다. 모든 texture는 기존 Resources에 있었으며 binary 변경·추가·Git 추적은 하지 않았다. 쿠크 4개 map light는 이미 활성화되어 있어 값을 증폭하지 않았다. BG 바닥과 공은 다른 원본 family이므로 바닥 재질을 복사하지 않았다. 전체 맵의 미연결 material family 복원은 이번 공 입력 연결과 별도다.

## 실행한 검증

- 변경 C++ MapAssetRenderUtils, WorldSequencePlayer_Objects, WorldSequenceDocument, Engine Model 개별 컴파일 성공. 기존 CP949 header를 UTF-8 compile한 C4828 경고가 있었고 컴파일 오류는 없었다.
- 실제 WorldSequenceDocument 코드로 Load → material override 생성(program 21, Base mask 127/Light mask 223, 기존 물리 texture 8개) → Save → Reload 동등성 통과. 잘못된 상대 경로, 중복 texture expression, 누락 parameter, 추가 parameter를 모두 거부하고 이전 document 보존.
- 실제 publisher 함수의 유효 profile 수용 및 같은 4개 잘못된 입력 거부 통과. PowerShell AST parse 성공.
- MeshBinary/Deferred FX 전체 컴파일 성공. 기존 family와 동일한 FXC X4000 계열 경고가 있으므로 warning-free 판정은 하지 않았다.
- 원본 native DXBC Base/Light와 production program 21 HLSL을 WARP에서 같은 synthetic 입력으로 비교: 18 case, 73,728 pixel, nonfinite 0, 상대오차 1e-3 초과 0. IBL 0 → 0.8 입력에서 Base RGB 누계 0.311838 → 4.102로 반사 응답 확인. 이는 수치 검증이며 화면 fidelity 판정이 아니다.
- 담당 코드 `git diff --check` 성공.

## 통합 및 남은 확인

공 정본 merge payload는 `out/KoukuBallMaterial20260910/ball-material-profile.patch.json`이다. 사용자가 저장 중인 worldsequences를 광역 저장하지 않고 root에게 objectId와 materialProfile만 전달했다. 최신 사용자 Save 뒤 root가 payload를 합치고 publisher와 Product 빌드를 완료해야 한다. 실제 CModel texture 생성/Client 화면 재생은 Product 실행에서 확인할 부분으로 남긴다. Client/UI 실행·캡처·visual PASS는 수행하지 않았다.


## 2026-09-10 최종 Product 통합 확인

사용자 마지막 Save/종료 뒤 최신 원본에 통합하고 관련 publisher와 정규 Debug Product 빌드를 완료했다.
Engine/Shared/Server/Client 컴파일·링크·EXE/DLL/셰이더 배포는 PASS이며 실행 입력 누락은0이다.
이 기록은 위의 Product 통합 대기 상태를 갱신한다. 세부 게시 revision·새 Server 검사·남은 사용자
화면 확인은09-10 KOUKU_PATTERN_EFFECT_ANCHOR_FEAR_AUTHORING_IMPLEMENTATION_RESULT의 G10에 있다.
빌드 근거: `out/BuildPipeline/runs/20260910T091016153Z-debug-product.json`. Client/UI 실행·캡처와 최종 육안 승인은 수행하지 않았다.
