# 베른·Character Select 원본 렌더링 입력과 Benchmark 결과

## G00. 완료 범위

원본 설치 패키지20개에서 렌더링 관련 객체714개를 선택해 WorldInfo, 클래스 기본값,
postprocess chain, 환경 영역, 조명·Lightmass·LUT·vignette 정보를 대조했다. 선택 객체의 tagged
property 미해독은0개다. 이것은 모든 native CPU 코드나 원본 프레임을 복구했다는 뜻이 아니다.

기존 Rendering Workbench의 Benchmark에 Bern/Character Select의 `Before`,
`Restored inputs (adapter)`, `Return to entry`를 연결했다. 기존14개 profile과 globalQuality는
의미상 그대로이며, 명시적인 qualityOverride를 가진 비교 profile4개를 추가했다. 시작 scene은
기존 profile을 사용하고 사용자가 버튼으로 비교한다. 카메라의 원본값 시작 적용은 별도
[카메라 결과](2026-09-14_GAMEPLAY_CAMERA_RESTORATION_RESULT.md)에 기록했다.

이번 후보는 원본 블룸·채도 입력을 현재 renderer에서 구현한 adapter다. 원본 Customizable
tone mapper의 CPU 상수 변환은 미복구라 현재 Hable을 유지했다. 원본 전체 렌더링이나 도서관의
최종 시각적 복원이 완료됐다고 판정하지 않는다. Client/UI 실행·조작·캡처는 하지 않았다.

## G01. 발견된 원본 차이

| 입력 | Bern | Character Select |
|---|---|---|
| World chain | efpostprocess.postprocesschain.defaultscenepostprocess | bg_pcselect02.postprocesschain.charactercloseupscene |
| Bloom Scale / Threshold | 1.25 / 0.4 | 0.9 / 0.7 |
| Bloom Tint, byte RGB | 255,192,157 | 255,255,255 |
| Desaturation | 0.1 | 0 |
| Highlights | .95,.97,.95 | 1,1,1 |
| Midtones | 1,1.05,.85 | 1,1,1 |
| Tone Scale / Range / Toe | 1 / 8 / 1 | 1 / 8 / .5 |
| Image grain | .02 | .2 |
| AO power / radius | 2.5 / 70cm | 2 / 25cm |
| DOF | 활성 | 비활성 |

Bern WorldInfo는 PS export1559, SHA256
`1494866ea6e94e2a679ebfeba8c2018e7703cb972406ba52ee5d639bc0303d56`이다.
CS는 SL00에만 WorldInfo를 찾으면 안 된다. parent LV_LOBBY_PS의 streaming ref179가 SL00을
참조하고 WorldInfo1101은 BG_PCSELECT02의 CharacterCloseupScene을 사용한다. BG_PCSELECT02
chain591은 Uber1736, AO2, vignette MaterialEffect22, material 미할당23으로 구성된다. 해당 패키지 SHA는
`e2a381a1f98214a98e748d82aad3dcbec7ca4ffe6ebc2ce46dbb61ec8d5c265d`이다.
현재 CS의 대략 focus(-772,-141,197)와 eye는 parent의14개 volume AABB 모두 밖이어서
다른 캐릭터 선택 배경 영역의 LUT/tone/AO를 SL00에 대입하지 않았다.

WorldInfo 및 Volume은 클래스 CDO와 struct 기본값을 상속한다. 현재 source의 네 mapped field는
World/지역 모두 해당 override=true임을 확인했다. CS tone/grain override=false이므로 chain의
toe.5/grain.2를 World struct의 toe1/grain0으로 덮지 않는다. Bern World toe override=true는1이다.

원본 지역 LUT `lv_att_heartkt.tex.lv_att_heartkt_lut_01`은 실제256×16 PF_A8R8G8B8,
sRGB=false/noMip 텍스처다. 그러나 해당 지역의 LUT override=false이므로 제품 후보에서 활성화하지 않았다.
World LUT override=true인 곳의 실제 LUT 배열은 비어 있다. 값의 존재와 실제 적용을 구분했다.

## G02. 밝기 중첩 조사와 실제 수정

`CLight_Manager::Add_Light`는 append가 아니라 scene light 교체다. 환경 갱신·재진입 자체가
방향광을 매번 누적하지 않는다. Bern maplights315개는 ID중복0이고 authoring/runtime이 일치한다.
그중285개는 RNM에 구워진 원본 광원이라 SOURCE_CHARACTER 수신기이며30개만 ALL이다.

실제 결함은 native monster carrier를 쓰는 일부 정적 배경이다. SOURCE_CHARACTER가 marker5인
모든 행을 허용해 RNM을 가진 배경도 구운 source light를 다시 받을 수 있었다. 기존
`g_SourceMapMonsterBakedEnabled`, program80..83, 해당 픽셀의 RNM flag를 모두 확인하도록
`Reject_LightReceiver`를 고쳤다. 실제 Bern 대상은31 material행/60 placementLighting이며
drop_devilstone/drop_abilitystone/wp_np_lrrt3 계열이다. 일반 벽 전체의 중첩을 고쳤다는 뜻은 아니다.
ALL/UNBAKED 및 일반 캐릭터·Effect 수신은 유지한다.

다음 세 가지는 별도 미해결 밝기 원인 후보다.

- **태양광 제외 영역:** Bern dominant actor82는 ExcludeVolumes=[80]을 명시한다.
  DDLExcludeVolume_1의 runtime bounds는 min(-42.52,-13.48,81.75998),
  max(277.47999,91.48,401.75998)m다. 현재 LIGHT_DESC/GPU에는 이 입력이 없다.
  원본의 primitive/channel 선택 의미가 미확정이라 임의 AABB 안의 픽셀을 모두 어둡게 만들지 않았다.
- **환경광 입력:** 현재 Bern ambient(.723529,.787059,.9)는 Lightmass EnvironmentColor
  (205,223,255)×Intensity.9에서 온 adapter다. 원본 bake authoring 입력을 runtime SH와
  동일시할 근거는 없다. source shader와 RNM이 실제로 어떤 환경 기여를 갖는지 CPU packing까지 필요하다.
- **안개와 구운 조명 합성:** deferred final은 fog(direct)+emissive이며 emissive에는 RNM·반사·진짜
  발광이 같이 담긴다. 이 버퍼 전체를 fog로 감쇠하면 정상 발광도 바뀌므로 의미를 분리하기 전에 수정하지 않았다.

## G03. 제품 연결 계약

Engine RENDER_QUALITY_SETTINGS에 optional color adjustment의 기본값 white/0을 추가했다.
`Renderer::Apply_RenderQualitySettings`는 finite·범위를 검증하고 final shader의 raw values를 binding한다.
블룸 tint는 최종 bloom 합성에 한 번 곱하며, desaturation은 Hable/gamma 뒤 display-space에서
RGB와 .299R+.587G+.114B 회색을 보간한다. desaturation0은 기존 경로를 그대로 반환한다.
원본 LUT의 packed color grading과 같다고 주장하지 않는다. 후보 tint는 source byte RGB/255의
명시적 adapter이며 원본 FColor→linear의 CPU 경로까지 복구한 값은 아니다.

quality JSON의 optional `colorAdjustment`는 bloomTint:[R,G,B,1](RGB0..1),desaturation0..1이다.
기존 profile이 이 블록을 생략하면 항등값이다. globalQuality/qualityOverride의 parse, validate,
serialize와 publisher에 같은 계약을 연결했다.

Bern source 후보의5개 convex 환경 영역은 원본 priority와4개 PP입력을 사용한다.

| source volume / 현 region suffix | Priority | Bloom scale / threshold | Tint byte RGB | Desaturation |
|---|---:|---|---|---:|
| 0 / 106 | 0 | .9 / .7 | 255,255,255 | .2 |
| 1 / 107 | 0 | .5 / .7 | 255,255,255 | .25 |
| 2 / 108 | 1 | .9 / .7 | 255,255,255 | .2 |
| 3 / 109 | 2 | .9 / .5 | 255,168,128 | .1 |
| 4 / 110 | 2 | .9 / .5 | 255,168,128 | .1 |

큰 priority를 먼저 선택하고 같은 값이면 작은 기존 convex AABB를 선택한다. 실제 containment는
AABB 후 plane 검사다. fog/light와 기존 blendTimeIn/Out을 공유해4개 PP값을 보간한다.
이탈 시 m_EffectiveQuality 기본값으로 복귀한다. 지역 PP가 없는 기존 profile은 quality를 건드리지 않는다.
지역 bloom도 profile bloomIntensityMultiplier를 한 번 적용하고 최종0..16 범위를 검증한다.
검토에서 발견한 multiplier 우회 결함을 수정했다. GPU 입력 성공 후 transition cursor를 commit하며
quality/fog/light 거절 시 이전 입력과 경과 상태를 보존한다.

Benchmark는 실제 활성 renderer의 exposure, bloom, tint, desaturation, fog, directional/ambient를
표시한다. `Activate_Profile`만 사용해 비교하고 자동 저장·배포하지 않는다. 도구를 닫을 때는 같은
Level/quality owner/마지막 적용 profile 조건이 모두 맞을 때만 entry profile로 돌아간다.
외부 scene/Level 변경 또는 성공한 Reload는 소유권만 해제해 새 상태를 보존한다.
Capture 중 profile 전환을 막고, 전환 프레임은 Capture를 막는다. 기존 재질 A/B는 별도 section이다.
fingerprint에 source fog, 환경 cube/색/회전, light receiver/shadow channel, 새 color adjustment를 포함했다.

## G04. 미지원 원본 경계

원본 Customizable tone/LUT shader가 Hable과 다른 점은 [선행 native 후처리 결과](../09-11/2026-09-11_KOUKU_SOURCE_POSTPROCESS_IMPLEMENTATION_RESULT.md)와
[Epic UE3 Color Grading 문서](https://docs.unrealengine.com/udk/Three/ColorGrading.html)를 대조했다.
원본 authored Scale/Range/Toe 및 Highlights/Midtones/Shadows→GPU상수 변환은 여전히 미확정이다.
문서 예제곡선을 LostArk 원본 상수로 대체하지 않았다.

따라서 이번에 원본값을 알아낸 것과 제품에 연결한 항목은 다르다. tone curve, LUT color packing,
image grain, 원본 SSAO/FXAA buffer·연산, DOF, vignette material, light shaft, sun exclusion,
환경 particle/먼지/창문 광선과 실제 화면 검증은 남아 있다. 창문 흰빛이나 도서관 질감은 후처리
preset 하나로 완성됐다고 말할 수 없다. 재질·광원·구운 조명·공기·최종 합성의 층을 분리해 추적한다.

## G05. 검증 증거

- source 조사:20package SHA, 선택714객체, 선택 tagged parse 미해독0.
- 실제 `Reject_LightReceiver` 본문을 C++ mock texture로 실행해288,000조건 검사, 실패0.
  변경32조건은 SOURCE_CHARACTER+실제 native baked map pixel에 한정됐고287,968조건은 보존됐다.
- Rendering publisher의 legacy/new 계약·negative·출력 보존23조건 통과. multiplier0과 최종 강도
  범위 초과 반례도 포함한다. 실제 제품 RenderingProfileService/DataJson OBJ를 링크한
  parser/serializer 검사52개 통과. 최종 소스보다 product OBJ가 최신임을 SHA/mtime으로 확인했다.
- 실제 Apply_CameraEnvironment 함수 본문 검사29개 통과. priority·보간·exit·multiplier0/비항등·
  실패 rollback을 확인했다. renderer binding은 mock이며 실제 GPU transaction 검사가 아니다.
- 실제 Benchmark6개 함수 본문에서24시나리오/146assertion 통과. 프로필 전환·복귀·도구 닫힘·
  외부 Level/scene/quality owner·Reload·실패 보존·캡처 잠금을 검사했다. ProfileService/ImGui 일부는 mock이다.
- 실제 현재/HEAD의 Sanitize_HDR/Tonemap_Hable/Resolve_FinalLDR 본문을 추출해 WARP GPU에서
  180검사/737,280픽셀을 검증했다. 실패·nonfinite·범위초과0. Tint1/Desaturation0 및 Bloom off의
  tint 변화는 이전 출력과 bit mismatch0이다. 완전 탈색 채널차≤5.96e-8, 중간 탈색 휘도오차≤6.20e-8.
  창·swapchain·이미지 출력 없이 실행했으며 제품 전체 프레임/native LostArk 동일성 검사는 아니다.
- 원본 후보4개 추가 후18profile Validate/Publish 성공. 전체 시작 profile/globalQuality 보존을 대조했다.
- Debug Product 통합 빌드 성공,240.995초. receipt는
  `out/BuildPipeline/runs/20260914T144440331Z-debug-product.json`이며 Client.exe는
  2026-09-14 23:44:39 KST에 생성됐다. Engine/Shared/Server/Client와 shader 배포 완료.
  기존 C4819/C4828, shader X4000/X3571/X4717 경고가 있으며 실패는 없다.
- Camera JSON 및 Client 프로젝트/filters XML parse, 새 Bern/Valtan None 각1개 등록 검사 성공.
  JSON 개행 혼용으로 최초 diff검사에서 whitespace가 검출돼 해당 JSON의 개행만 LF로 정리했다.
  최종 `git diff --check` 성공. authoring/runtime profile 재publish 성공.

원본 세부 증거는 `out/RenderingRestoration20260914/`의 rendering_source_handoff,
resolved_postprocess_candidates, profile-restoration-mapping, Bern.sun-exclusion JSON이다.
수신기와 service 검사 근거는 `out/BernLightingRuntimeAudit20260914/`에 저장한다.
팀에 남는 source 요약은 [원본 수치 receipt](2026-09-14_BERN_RENDERING_SOURCE_RECEIPT.json)이며
20개 package SHA/상속된 활성 입력/미지원 경계와 제품 매핑을 포함한다.

## G06. 사용자가 직접 확인할 경로

Server와 최신 Debug Client를 직접 실행해 Lobby에서 Bern 또는 Character Select로 들어간다.
F1 → Tools → Rendering Workbench → 오른쪽 Rendering Workbench → Benchmark → Rendering restoration에서
`Before`와 `Restored inputs (adapter)`를 번갈아 누른다. Bern은 같은 위치/카메라에서 실외, 실내,
창문 앞을 각각 비교하고 영역 이동 뒤 blend가 끝난 값을 확인한다. `Return to entry` 또는 도구 닫기로 복귀한다.
카메라는 별도의 Player Follow Camera에서 Before restoration/Source baseline을 비교한다.
창문이 더 하얀지, 목재/벽 디테일이 살아 있는지, 캐릭터와 완성된 Effect의 색·범위가 적절한지는
사용자의 화면 관찰로 판단해야 한다. 자동 검증은 그 판정을 대신하지 않는다.
