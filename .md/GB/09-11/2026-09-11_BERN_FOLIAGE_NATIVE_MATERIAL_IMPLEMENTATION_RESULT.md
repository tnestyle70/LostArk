# Bern foliage·grass 및 정적 monster native 표면 구현 결과

## 구현 상태

기존 CModel → CMaterial → MapAssetRenderUtils 경로에 foliage family 9와 grass family 10을 연결했다. source mask의 R/B/A 역할, 원본 tint·saturation·normal·specular·transmission·alpha 0.3333·emissive 및 RNM 함수를 native PS에서 대조했다. 정적 개별 draw와 MapInstance batch draw에 같은 MRT depth marker와 transmission 입력을 연결했다. 새 helper HLSL은 Engine/Client 프로젝트와 filters의 기존 shader include 항목으로 등록했다. 새 C++ 파일은 없다.

원본 정적 monster MIC 5종은 native program 80~83으로 연결했다. Base/Light 4쌍 및 Baked 4종을 원본 DXBC의 연산 순서로 옮겼다. 기존 source-character 상수 배열의 상단 32~58에 Baked material rows를 패킹하며, 새 C++ 상수 배열이나 별도 모델 런타임을 만들지 않았다. 원본 statefx sampler의 U clamp/V wrap과 나머지 texture의 native gamma·주소를 유지한다. static map의 B 부호와 source 좌표 `(x,-z,y)*100`은 해당 80~83에만 적용된다.

RNM이 이미 포함된 native monster pixel에는 project ambient 근사항이 중복되지 않도록 했다. 80~83의 원본 Light PS가 쓰지 않는 auxiliary UV.w에 실제 per-pixel RNM 사용 여부를 기록하므로, 같은 material의 RNM 없는 instance는 기존 ambient를 유지한다.

8개 assets/60개 source placements의 native monster RNM을 위해 같은 optional bakedLighting, UV1, average/directional texture, instance scale/bias를 소비한다. SOURCE_CHARACTER 전체에 무조건 허용하지 않고 80~83에만 해당 검증을 연다. Map 담당자가 같은 sourceMaterial 행에 배치별 RNM을 조인한다. 기존 0~32 material descriptor와 연산 입력은 변경하지 않았다.

## 자동 검증

- foliage/grass 기존 17개 native permutation의 Base/Baked/Light × 7 fixture: 357 cases, 365,568 pixels, 비유한 0, 상대 오차 1e-4 초과 0. `out/BernFoliageNative20260911/gpu_results.csv`.
- instanced foliage 추가 16 MIC의 새 grass permutation: 21 cases, 21,504 pixels, 비유한 0, 상대 오차 1e-4 초과 0. 나머지 15 MIC는 기존 native PS 조합을 재사용한다. `out/BernFoliageNative20260911/instance_extension_audit.json`, `instance_probe/gpu_results.csv`.
- monster Base/Light 4쌍: 72 cases, 73,728 pixels, 비유한 0, 상대 오차 1e-3 초과 0. `out/BernMonsterNative20260911/gpu_results.csv`.
- monster native Baked 4종: 36 cases, 36,864 pixels, 비유한 0, 상대 오차 1e-3 초과 0. `out/BernMonsterNative20260911/baked_probe/gpu_results.csv`.
- monster strict Configure 5행: 정확한 texture mask, 필수 parameter 누락·추가 거부, 유한 값, 실패 시 기존 입력 유지 확인. 상단 Baked packing 추가 후 다시 컴파일·실행했다.
- 위 WARP는 원본 MIC parameter와 합성 texture/조명/시야 fixture를 같은 원본 DXBC 및 제품 HLSL에 넣은 수치 비교다. Client를 실행하거나 화면을 캡처하지 않았다. 실제 모든 DDS texel에 대한 육안 품질 판정은 아니다.
- 실제 Deferred Directional PS를 컴파일하고 material/pixel RNM bit 네 조합을 WARP에서 확인했다. 4 cases, 4,096 pixels, 비유한·예상 ambient 불일치 0. `out/BernMonsterNative20260911/ambient_rnm_gpu.csv`.
- 최종 127행 JSON 유한 값 및 물리 texture 경로, 변경 프로젝트 XML 4개 parse를 확인했다.
- `git diff --check` 통과. Product 전체 컴파일과 catalog publish는 root가 통합 실행한다. 이 문서는 아직 그 결과를 대신 PASS로 기록하지 않는다.

## 데이터 인계

`out/BernFoliageNative20260911/material_rows.json`은 원본 sourceMaterial별 family 9/10 입력을, `out/BernMonsterNative20260911/material_rows.json`은 native monster 5개 입력을 소유한다. 원본 full identity와 선택한 texture gamma를 Resources 상대 경로로 전달한다. 추가 foliage16과 원본 DDS 11개 설치를 완료했다. 최종 행은 foliage 117개·grass 10개, 합계 127개다. 기존 111개 원본 입력의 의미를 유지한 같은 writer로 합쳐 Map 담당자에게 전달했다. 전체 texture 설치 목록은 1,477개다. Resources는 Git에 넣지 않는다.

## 남은 경계

원본 wind VS 2종의 선택은 확인했지만 CPU scene wind·mesh bounds setup이 닫히지 않아 움직임을 추정 구현하지 않았다. 원본 skyLight의 skybox-only lighting channel을 캐릭터나 나무 전체로 확장하지 않는다. scene ambient와 RNM·조명 배치의 최종 연결·publisher 검증은 환경/맵 담당자의 결과와 root 통합 결과를 함께 확인해야 한다. GPU probe에서 native masked early-return을 옮긴 80/82 함수에는 FXC X4000 경고가 남지만 검증한 visible fixture의 비유한 값은 없다. 사용자 화면 확인은 미실시다.

Renderer의 기존 source-character light loop는 실제 draw가 등록한 material count가 0이면 실행하지 않는다. 요청한 추가 보수 cull은 instanced foliage 완결을 먼저 처리하도록 root가 우선순위를 조정했으므로 이 변경에서 새 cull 구현을 완료했다고 주장하지 않는다.

## BG8 분기 19 MIC 추가 완료

비-overlay 19 MIC에서 원본 subspecular/rimlight/specular saturation/UV panning을 기존 family8에 연결했다. 신규 flag나 family 없이 optional sourceSubspecular[2], sourceRimlight[4], sourceSpecularSaturation, sourcePanning[2]를 사용한다. Map 담당자가 같은 catalog/publisher에 `out/BernBgBranches20260911/material_rows.json`을 병합한다.

Subspecular는 원본 Base/Baked에만 더해지는 독립 시선 반사항이다. chain01은 direct specular off와 subspecular on을 함께 사용하므로 source specular color/intensity를 보존하고 direct output만 분리했다. Rim은 view-dependent RGB를 BG8의 빈 MRT6 RGB에 기록하고 원본처럼 geometric normal 뒷면 입사량·shadow·light color를 한 번 곱한다. 원본 palette나 전체 ambient를 임의로 밝히지 않는다. emissive가 없어도 panning의 native time을 바인딩한다.

- 원본 19 MIC × Base/Baked/Light × 7 fixture: 399 cases, 실패 0, 최대 상대 차이 3.441e-7. `out/BernBgBranches20260911/gpu_results.json`. alpha threshold, normal, front/back light, 두 시야, 독립 subspecular, 실제 MIC parameter와 RNM을 대조했다.
- UV panning은 비균일 8×8 texture와 native wrap/linear sampler로 16개 시각의 Base/Baked/Light를 별도로 대조했다. 48 cases, 실패 0, 최대 상대 차이 2.534e-7. `panning_gpu_results.json`.
- 실제 제품 Deferred의 Resolve_MapSourceSpecularLight를 컴파일해 MRT6 rim/packed geometric normal 소비를 WARP로 확인했다. 4 cases, 4,096 pixels, 비유한·예상 값 불일치 0. `deferred_rim_gpu.csv`.
- 19개 JSON 행의 유한 값·물리 texture 참조를 확인했다. 새 C++ 파일은 없고 프로젝트 등록 변경도 없다. C++ 전체 빌드와 catalog publish의 최종 결과는 root/Map 담당자의 통합 결과로 확인한다.

같은 원본 signed-distance shadow helper를 native monster80~83의 geometry에도 연결했다. Binary의 별도 staticShadowUV varying을 NativeGeometry에 전달하고 해당 프로그램의 MRT4 alpha에만 occlusion을 기록한다. CMaterial의 native base bind가 Bind_StaticShadow를 호출한다. 기존0~32 alpha0은 유지한다. 이 연결은 환경 담당자의 원본 G8 texture·UV·exponent와 명시적인 PROJECT_ADAPTER bias/scale를 소비하며 원본 CPU penumbra가 확정됐다는 뜻은 아니다. 원본 exe/dll 정적 조사에서 보호된 disk section 때문에 bias/scale를 확정하지 못한 근거는 `out/BernShadowCpu20260911/RESULT.md`에 보관했다.

환경 담당자의 per-placement static shadow channel 추가에 맞춰 native80~83도 PickPos.W에 `EncodeMapStaticShadowChannel`을 적용한다. 기존 캐릭터의 W와 alpha는 건드리지 않는다.

## Ice·vertexblend·wet 8 MIC와 interpactor 1 MIC

`SOURCE_SNOWICE_OPAQUE=11`, `SOURCE_VERTEXBLEND_OPAQUE=12`, `SOURCE_WET_OPAQUE=13`을 기존 CModel/CMaterial/MapAssetRenderUtils와 Binary·MapInstance MRT 경로에 연결했다. 새 `MODEL_SOURCE_SPECIAL_PARAMETERS`는 ice 색·mask blending·시차·발광, vertex A/R/G/B 네 층의 UV·normal·diffuse·specular, wet의 normal·opacity·specular power를 소유한다. CMaterial은 실제 선택한 texture만 로드하고 `Bind_SourceSpecialSurface`를 통해 같은 CModel의 mesh material에 바인딩한다. 원본 SRGB와 linear normal을 보존하며, 필수 texture 누락·비활성 texture 입력·Resources root 이탈·비유한 값을 CModel이 거부한다. 새 C++ 파일은 없다. 새 `Shader_SourceSpecialSurface.hlsli`는 Engine/Client 프로젝트와 filters에 등록했다.

Ice는 원본 core/outer fresnel 색과 environment texture의 반사색을 합성하며, mask·시차 UV·독립 emissive를 보존한다. Vertexblend는 raw diffuse와 tint/intensity를 각각 원본 순서대로 혼합하고 세 normal blend 뒤 base normal을 더한다. Wet은 vertex R·diffuse A로 normal·반사·specular power를 혼합한다. 세 표면은 resolved per-pixel specular power를 MRT depth Z에 기록한다. 같은 RNM average/directional lighting과 static-shadow UV를 소비한다. Deferred는 원본처럼 ice의 cap 없는 specular, wet의 shadow 이전 cap 2, vertexblend의 shadow 이후 cap 2와 독립 back-facing rim을 구분한다.

`bg_base_interpactor_msk`의 실제 Base/Baked/Light PS는 기존 BG8 normal·diffuse·specular·alpha 0.3333 경로와 동치여서 family를 추가하지 않았다. 이 소재의 원본 wind VS는 기존 foliage와 같은 CPU scene wind 미확정 경계로 남긴다.

- 실제 제품 special helper와 원본 8 MIC × Base/Baked/Light × 14 fixture: 336 cases, 실패·비유한 0, 최대 상대 차이 2.823e-7. `out/BernSpecialSurface20260911/actual_gpu_results.json`.
- 같은 실제 helper에서 비균일 8×8 diffuse/normal/mask/environment texture로 UV·parallax·layer tiling을 추가 대조했다. 144 cases, 실패 0, 최대 상대 차이 2.180e-7. `pattern_gpu_results.json`.
- interpactor BG8 원본 Base/Baked/Light: 21 cases, 실패 0, 최대 상대 차이 3.726e-9. `interpactor_probe/gpu_results.json`.
- 실제 제품 `Resolve_MapSourceSpecularLight`의 family11/12/13 × front/back × shadow 1/0.25: 12 cases, 12,288 pixels, 비유한·RGB cap/rim 소비 불일치 0. `deferred_special_gpu.csv`.
- 실제 Deferred `PS_MAIN_DIRECTIONAL`과 MapInstance `PS_MAIN` 최소 컴파일 통과. 기존 native80/82의 X4000 early-return 경고 외에 MapMaterialSurface의 EvaluateMapSurface/EvaluateMapSourceIndirectLighting에서 X4000 두 건을 확인했다. 단순한 로컬 배열 명시 초기화로 없어지지 않아 근거 없는 제품 변경은 하지 않았으며, 최종 통합 경고 조사는 root에게 인계했다. 수치 검증의 finite 결과와 compiler warning 상태를 구분한다.
- 프로젝트 XML 4개 parse와 변경 소스 `git diff --check` 통과. special9 행은 `out/BernSpecialSurface20260911/material_rows.json`으로 Map 담당자에게 전달했다. catalog/publisher와 Product 전체 컴파일은 root/Map 담당자의 통합 검증 결과로 기록한다.

추가 foliage strict parser 검증에서 ivy 두 MIC의 비활성 `use_specular_texture` switch를 발견했다. 부모 `use_specular=false`라 원본과 helper 모두 이 texture를 소비하지 않는다. 두 행의 sourceFlags를 25→17로 정규화하고 specularTexture를 제거했다. 127행 writer가 이후에도 비활성 normal/specular texture를 내보내지 않도록 했다. 활성 texture·GPU 연산의 의미는 유지한다.

남은 helper 소재 중 navmesh cul01 326개와 cul02 10개는 원본 component `hiddengame=true`가 명시된 336개 배치다. `out/BernMaterialAudit20260911/helper_visibility_handoff.json`으로 확인했으며 Map 담당자가 비가시성을 보존한다. black 227개와 depth-modulate shadow 34개는 root의 기존 native forward 경로로 인계했다. 이 둘의 구현 완료나 전체 시각 복원 완료를 이 문서의 special8 검증에 포함하지 않는다. 사용자 Client/UI 실행과 화면 확인은 수행하지 않았다.
