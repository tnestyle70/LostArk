# SSGI 절반 해상도 gather 구현 계획

## G00. 목표와 현재 실측

현재 SSGI는 marker3 MapPBR만 대상으로 전체 해상도에서 4/8/16 ray와 ray당 8 depth step을 계산한다.
사용자가 요청한 FPS 우선 custom quality layer로 절반 해상도 gather와 depth/normal bilateral resolve를
추가한다. 원작 복원이나 Lumen·DXR 구현으로 표시하지 않는다. 현재 기본 전체 해상도와 저장 옵션을 보존한다.

원본 및 설치 UE5 조사 결과는 out/CharacterSelectRenderingAudit20261004/audit.md에 구분했다.
UE source는 scene/cache/render graph 결합을 참고하는 설계 근거이며 코드 복사 대상이 아니다.

## G01. 헤더와 renderer 상태

Engine/Public/Engine_RenderTypes.h의 RENDER_QUALITY_SETTINGS 끝에 bSSGIHalfResolution=false를 추가한다.
false는 기존 전체 해상도, true는 SSGI가 켜졌을 때만 절반 해상도 gather를 선택한다. SSR은 그대로다.

Engine/Public/Renderer.h는 half gather FP16 texture/RTV/SRV와 실제 폭·높이를 소유한다.
Ready_SSGIHalfTarget은 ceil(width/2), ceil(height/2)로 local resource를 모두 만든 후 교체한다.
렌더링 전에 실제 scene 크기를 대조하므로 resize 후 이전 크기를 사용하지 않는다. 준비 실패는 기존
resource와 scene을 보존하며 실패를 반환한다. OFF와 full 모드에서는 추가 texture를 만들지 않는다.

## G02. shader와 렌더 호출

Engine/Client 양쪽 Shader_ScreenSpaceLighting.hlsl에 gather/resolve pass를 기존 두 pass 뒤에 추가한다.
Gather는 2x2 영역의 고정 대표 픽셀을 사용하고 기존 ray 수/반경/원본 HDR radiance를 소비한다.
출력은 incident radiance RGB와 대표 view depth다. Resolve는 주변 네 gather 값의 공간 weight에
full-resolution depth와 normal 유사도를 곱하고, 다른 receiver/불연속 depth/normal/비정상 값을 거부한다.
최종 albedo와 metallic은 현재 full-resolution 픽셀에서 곱하여 재질 경계의 색을 섞지 않는다.
지원 대표 표본이 없으면 추가 기여만 0이며 기존 scene/bloom은 보존한다.

CRenderer::Render_ScreenSpaceLighting은 half 선택 때만 작은 viewport의 gather 뒤 원래 크기 resolve를
실행한다. 기존 scene/bloom scratch commit과 출력/viewport/SRV 복원 경계 안에서 수행한다.
성공 후에만 scene/bloom copy가 일어난다. Render.SSGI.GatherHalf, Render.SSGI.ResolveHalf를
별도 GPU scope로 측정하고 공통 copy 비용은 기존 scope를 유지한다. SSR은 원래 radiance를 읽는다.

## G03. 옵션과 A/B 연결

root가 RenderingProfileService의 whitelist 끝에 SSGI_HALF_RESOLUTION을 연결하고 측정 JSON의 명시 필드,
session 기본 false, snapshot/rollback, fingerprint와 Workbench full/half 단일 비교를 맡는다. 저작 정본을
자동 변경하지 않는다. 효과 설명에는 marker3, 추가 가산 GI, 화면 밖 정보/temporal 미지원 경계를 표시한다.

## G04. 검증과 남은 경계

변경 Engine TU를 Debug compile하고 양쪽 shader 동일성 및 fx_5_0/각 PS compile을 확인한다.
기존 screen-space WARP fixture를 재사용해 기존 full/SSR 출력 보존, half gather/resolve의 실제 기여,
상수 입력 보존, depth/normal/receiver 경계, 홀수 크기와 1픽셀, 비정상 입력 격리, bloom delta를 검사한다.
OFF는 render 분기 이전 반환을 유지한다. 실제 FPS 절감/화질 판정은 사용자가 동일 카메라의 Workbench에서
측정한다. 정식 Product build는 root가 조율한다. 새 C++/HLSL 파일이 없으므로 project/filter 추가는 없다.
