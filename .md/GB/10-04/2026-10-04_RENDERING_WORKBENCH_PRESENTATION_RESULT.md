# Rendering Workbench 촬영 흐름 결과

## G00. 구현

Workbench를 `Restoration`, `Technique A/B`, `Measure / Analyze`, `Saved Settings` 네 탭으로 분리했다.
기본 화면은 여섯 단계와 이전/다음 버튼, 원래 화면 복귀만 제공한다. 수치44개·기법 사전·
픽셀 진단은 기본으로 접으며 저작 저장 기능은 별도 탭에 있다. 기존 ANSI TU인 MainApp의 새 탭 문구는 ASCII, 기존 UTF-8 Benchmark의 상세 설명은 한국어로 유지한다. UI 크기는 현재 ImGui frame
height와 가용 너비를 사용한다. Client를 실행해 실제 창/DPI를 확인한 것은 아니다.

시연은 기본 재질(근사) → 원본 재질 → 환경광·baked → 그림자·공간 효과 → Tone·LUT → 현재
설정이다. 모든 단계는 처음 보관한 실효 값에서 구성하고 차이 필드만 임시 소유한다. 단계
이동에서 저장된 FXAA OFF·노출·감마·원본 재질 입력을 임의로 바꾸지 않는다. 간편 A/B는
13개 기법 중 하나만 선택해 기존 recipe에 연결한다. 표준 recipe는34→35개다.

기존 Service whitelist 끝에 `material.sourceMaterials.enabled`를 추가했다. 같은 transaction의
적용·rollback·종료 복원을 사용하며 Data schema나 새 renderer owner를 만들지 않았다.
재질 OFF는 map의 기존 fallback과 diffuse가 유효한 SourceCharacter program1~5,8~17,21~32의
기존 textured deferred branch에 연결한다. hair6/7/18/19/20·forward·미확인 native program은
유지한다. 현재 WModel·텍스처·형상을 유지하므로 최초 임포트 EXE의 정확한 재현으로 표시하지 않는다.

단계에서 기법 비교로 전환할 때 흐려진 현재 화면을 A로 다시 읽지 않고 Original을 사용한다.
수동으로 새 A를 채택했던 세션이 Original로 돌아가면 experiment ID를 새로 발급해 이전 측정의
A가 새 비교의 기준으로 섞이지 않게 했다. 선택 필드만 common fingerprint에서 제외하고 full
fingerprint는 material selector를 계속 기록한다. 옛 픽셀 진단과 새 세션의 중복 owner도 거부한다.

## G01. 추가 원본 복구와 조사

실제 Character Select Movie의 WARLORD 소품·머리카락 native700/701에서 primitive opacity
prefix 누락을 수정했다. 대상·수치·shader identity의 근거는
[해당 결과](2026-10-04_MOVIE_WARLORD_HAIR_PREFIX_RESULT.md)를 따른다.

현재 native128×32 BRDF와 SH 입력 복원이 연결된 범위를 확인해 Workbench의 오래된
전면 미복구 안내를 교정했다. 프로젝트 근사와 원본 입력을 구분했다.
공식 DX11 전환은 UE4 이식 증거가 아니며, UModel export·LPK archive·ShaderMap·runtime binding
및 현재 미보유 raw 자료의 경계는 [원본 조사 결과](2026-10-04_RENDERING_SOURCE_EVIDENCE_RESULT.md)에 기록했다.

사용자가 후속으로 제시한 베른 `(98.7,49.1,-103.4)`의 녹색 지형 늘어짐과 발탄 입구 돌 표면은
별도 원인 조사 대상이다. 이 문서의 Workbench/primitive prefix 검증으로 해당 지형이 고쳐졌다고
판정하지 않는다. Bern의 렌더링 튜닝·밝기·GI 옵션은 변경하지 않았다.

## G02. 검증

- 실제 Benchmark/Service 본문과 public header 기반 session 검사 **173 checks / 0 failures**.
  단계 왕복·직접 점프·한 기법 전환·원래 OFF 보존·LUT/PP 조건·실패 보존·rebase ID 분리를 확인했다.
  `out/RenderingPresentation20261004/ui/session_receipt.json`.
- 실제 Service read/validate/stage/apply/restore **144 checks / 0 failures**. renderer setter별
  적용/복원 실패와 재시도, profile/level/region/Video 변경, 비소유 필드 보존을 확인했다.
  `out/RenderingPresentation20261004/service/service-verification.json`.
- 실제 fallback policy/material bind/combat bind **953 checks / 0 failures**. 확인 program27개,
  미지원22개, diffuse override·shared shader reset·hit glow·bind failure를 확인했다.
  `out/RenderingPresentation20261004/dispatch/receipt.json`.
- 실제 condition fingerprint **73 checks / 0 failures**. 선택 material selector만 비교에서
  제외하고 비선택 debug/환경/후처리 조건은 유지했다. `out/RenderingPresentation20261004/fingerprint/run.log`.
- Benchmark, MainApp, Service, DeferredMaterialRenderUtils, MapAssetRenderUtils 총5개 변경 TU
  Debug 독립 컴파일 통과. 기존 포함 헤더의 인코딩 경고는 남아 있다.
- native700/701 WARP 검사220개와 실제 Group640 FX 컴파일 통과는 별도 prefix RESULT에 기록했다.

앞의 C++ 세션/dispatch 검사는 renderer/model/user 경계를 fixture로 사용했다. 실제 Client 외형이나
촬영 화면 검증으로 대신하지 않는다. 저장 프로필·camera·effect·composition·mapplacement의
사용자 변경을 이 작업에 섞지 않았다. 새 C++ 파일이나 project/filter 등록 변경은 없다.

## G03. 제품 반영 상태

사용자가 저장·종료한 뒤 최초 Debug Product 빌드는 PASS했다.
`out/BuildPipeline/runs/20261003T202441505Z-debug-product.json`에 기록됐다.
MainApp 새 문구의 ANSI TU 호환 수정을 포함한 두 번째 Debug 빌드는 map instance FXC 작업에서
MSB6006과 pipe EOF timeout을 기록했다. 같은 시간 다른 작업의 공통 map shader·Engine 변경이
진행 중이었으며, 오류가 그 변경의 문법 오류라는 근거는 없다. 최종 빌드와 Release는 아직 완료되지
않았다. 사용자 Client/UI 실행·화면 검증은 수행하지 않았다.

## G04. Bern 기본 Fog OFF

사용자의 명시적 요청으로 Bern 기본 진입 프로필 `scene.bern.neutral-day.v1`의 `fog.enabled`를
false로 변경했다. RenderingProfiles revision은90→91이다. 원본 비교용 두 Bern 프로필과 지역
밀도·색·높이, 다른 맵·FXAA·노출·감마는 보존했다. 기존 환경 영역 갱신의 scene enable gate가
지역 안개에도 적용된다. 배 탑승 전용 presentation fog나 사용자의 이후 임시 변경은 별도 경계다.

최신 원본 hash 재확인·백업·원자적 교체 후 `Publish-RenderingProfiles.ps1 -Mode Publish` PASS.
원본/게시 JSON parse 및 의미 동치 확인, 변경은 위 bool과 revision 두 필드뿐이다.
`out/RenderingPresentation20261004/fog/receipt.json`에 source 전후 hash를 보존했다.
데이터 게시 완료이며 실행 중 메모리 Reload나 사용자 화면 확인 완료를 뜻하지 않는다.
