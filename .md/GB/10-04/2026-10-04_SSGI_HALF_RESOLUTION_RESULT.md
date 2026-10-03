# SSGI 절반 해상도 gather 결과

## G00. 구현 상태

Engine/Public/Engine_RenderTypes.h의 bSSGIHalfResolution=false와 Renderer의 실제 gather/resolve를 구현했다.
기본 false는 기존 전체 해상도 SSGI이며 SSGI OFF는 기존 조기 반환을 유지한다. SSR과 원본 RNM/IBL,
사용자가 저장한 렌더링 선택값은 변경하지 않았다. 이는 custom screen-space GI 실험이며 원작 복원,
Lumen, DXR 또는 화면 밖 GI 구현으로 표시하지 않는다. session/Workbench/capture 연결은 root의
동일 변경에서 통합하며 저작 profile JSON에 새 저장 경로를 만들지 않는다.

## G01. 실제 코드와 자원 경계

Engine/Client의 Shader_ScreenSpaceLighting.hlsl은 같은 내용이다. 기존 PS_SSGI와 PS_SSR entry는
그대로 보존했다. 새 pass2는 ceil(width/2)×ceil(height/2)의 RGBA16_FLOAT에 incident radiance RGB와
view depth를 쓴다. pass3은 주변 네 표본에 공간·depth·normal weight를 적용해 full-resolution
albedo/metallic으로 결합한다. marker3 이외, 비정상 값, depth/normal 불연속, 지원 표본 부재에서는
추가 기여가 없다. full-resolution 재질색을 마지막에 곱하므로 인접 검정/금속 재질의 색이 섞이지 않는다.
Bloom은 기존 base를 유지하고 추가 radiance의 bright-pass 차이만 더한다.

CRenderer::Ready_SSGIHalfTarget은 새 크기의 texture/RTV/SRV 준비가 모두 성공한 뒤 교체한다.
이 자원은 Renderer 소유의 lazy scratch이며 viewport가 바뀌면 다음 half 실행 전에 실제 크기를 다시
검사한다. OFF/full에서는 추가 할당을 하지 않는다. 준비 실패는 이전 자원을 보존하며 scene commit
전 실패한다. 기존 Render_ScreenSpaceLighting의 OM/viewport/SRV 복원과 두 pass 성공 후 HDR/bloom
copy 경계 안에서 실행한다. SSR은 GI 결과가 아닌 기존 immutable radiance를 계속 읽는다.

GPU scope는 Render.SSGI 아래 Render.SSGI.GatherHalf와 Render.SSGI.ResolveHalf를 추가했고,
Render.ScreenSpaceLighting.Copy는 유지했다. 짝수 크기의 gather 픽셀 수는 기존의 1/4이며 실제 프레임
시간에는 full-resolution resolve와 공통 copy도 포함된다. frame FPS 개선 비율은 아직 측정하지 않았다.

## G02. 실행한 검증

증거는 out/SSGIHalf20261004에 보존했다.

- Renderer.cpp Debug x64 개별 TU: /Od /RTC1 /JMC /Z7 /MDd compile PASS.
  기존 포함 헤더의 C4819 경고는 남았으며 파일 인코딩은 바꾸지 않았다.
- FXC /O1 fx_5_0, PS_SSGI, PS_SSR, PS_SSGI_GatherHalf, PS_SSGI_ResolveHalf compile PASS.
  Effects deprecated X4717 이외 새 shader 경고는 없다.
- 실제 D3D11 WARP: 64×64는 76검사, 65×65는 76검사, 1×1은 31검사, 총183검사 실패0.
  Gather RT는 실제 RGBA16_FLOAT, resolve 읽기 비교 RT는 RGBA32_FLOAT MRT0/MRT2다.
- 이전 shader snapshot과 현재 full SSGI/SSR를 동일 입력으로 각각 렌더해 scene/bloom bitwise 동일 확인.
- half gather의 실제 양의 광 기여, Bloom 증가와 alpha 보존, full-resolution 검정 albedo/metallic,
  marker0/1/2/4/5/6/14 제외, depth/normal 경계, NaN/Inf 표본, 지원 표본 부재, invalid ray budget,
  실제4/8/16 ray 출력의 finite 조건과 상수 incident 보존을 확인했다.
- 65×65에서 마지막 행/열의 UV 1ULP 범위 이탈을 발견해 새 half 경로만 canonical pixel-center clamp로
  교정했다. 기존 full entry는 유지했고 홀수 가장자리·1×1도 다시 통과했다.
- 변경 파일 CRLF와 Engine/Client shader 동일성, git diff --check를 확인했다.

자원 할당 실패 주입과 실제 CRenderer 전체 호출을 실행한 통합 테스트는 이번 픽셀 fixture 범위에
포함하지 않는다. 그 경계는 local stage/RAII/commit 코드 검토와 실제 TU compile로 확인했다.

## G03. 남은 검증 경계

정식 Debug/Release Product build 및 최종 통합은 root가 기록한다. Client/UI는 실행하지 않았다.
사용자는 같은 camera/scene에서 Workbench full/half A/B와 GPU scope를 측정해 실제 비용과 화질을
판정해야 한다. 고정 대표 표본을 놓친 얇은 표면은 추가 GI가 약해질 수 있고 history/denoise/화면 밖
정보는 없다. native character marker5와 Landscape14까지 소비 범위를 넓히지 않았다.


## G04. Workbench 연결과 최종 빌드

Service whitelist 끝에 `quality.ssgi.halfResolution`을 추가해 기존 필드 번호를 유지했다.
실험 필드는45개이며 Read/Apply/소유 필드 복원과 비교 fingerprint에 새 선택자가 연결된다.
`ssgi.resolution`은 SSGI가 켜진 A에서 full/half만 바꾸는 단일 변수 비교다. Technique A/B와
상세 recipe에서 선택하며, GI가 꺼진 A이면 이유를 표시한다. SSGI ON인 B를 새 A로 채택한 뒤
해상도를 비교하고 실험 종료 시 최초 장면으로 복귀한다. 저작/게시 rendering JSON은 변경하지 않았다.
Profiler capture의 `SSGI.halfResolution`과 gather/resolve scope 이름도 연결했다.

- 실제 Service 본문 transaction 검사101/0, 실제 Benchmark recipe 적용·원복9/0.
- 실제 fingerprint 검사90/0: 선택한 half 필드는 raw/named 공통 조건에서 제외하고 비선택 상태의
  half·GI ON·강도·반경·ray·SSR·gamma·material 조건은 유지한다.
- `out/SSGIHalf20261004/ui-validation-receipt.json`, `ui-service/service-verification.json`에 보존했다.
- 최종 Debug Product `out/BuildPipeline/runs/20261003T231005474Z-debug-product.json` PASS.
- 최종 Release Product `out/BuildPipeline/runs/20261003T231154821Z-release-product.json` PASS.
  두 빌드는 워로드V 수정과 현재 Workbench의14단계/16빠른비교/36recipe를 포함한다. Product 배포와
  필수 runtime/Navigation/Item/Valtan reward 확인도 통과했다. source/data publish나 Client 실행은 하지 않았다.
- 같은 primary 작업 폴더의 별도 Bern 최적화도 통합 빌드에 들어 있으므로 이 변경만의 격리 빌드가 아니다.
  별도 merge worktree에서 Benchmark/ProfileService/ProfilerTool/EffectPresentation/Renderer의 실제
  CPP5개를 다시 Debug 컴파일해 PASS했다(`merge-compile/receipt.json`). 격리 Product link 증거는 아니다.

최종 화면의 품질·FPS 및 실제 장면 A/B 비용은 미측정이다. Lumen·DXR·temporal GI는 구현하지 않았다.
원본 수식과 사용자 튜닝, 추가 screen-space GI를 구분하며 이 결과를 전체 원작 복원 완료로 쓰지 않는다.
