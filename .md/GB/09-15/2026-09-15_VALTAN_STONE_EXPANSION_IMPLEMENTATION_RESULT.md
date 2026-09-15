# 발탄 정적 석재·중앙 소품 원본 입력 확장 결과

## G00. 실제 설치 범위

기존 09-08 석재7배치와 09-11 중앙 circle1을 보존하면서 원본 floor01의24배치·rock02의419배치,
합443배치에 실제 component MIC·색·UV1·RNM을 연결했다. 새436행을 더했으며 source MIC04는27,
MIC05는164, MIC02는252배치다. MIC02의 native static set에서 vertex paint가 꺼진 것을 확인하고
기존 overlay의 world-up 분기로 연결했다. 다른 MIC 값을 일괄 복사하지 않았다.

중앙 Deploy A/B와 난간은 원본 LOD0 material array를 읽고 새 native geometry와 실제 재질8행을
ActorCatalog에 연결했다. source527의 구름은 같은 모델을 쓰던525와 별도 asset으로 분리해,
원본 native static set이 기존 program59와 일치하는 실제 molding 투명 재질로 연결했다.

| 현재 데이터 | 확인 수치 |
|---|---:|
| Valtan catalog | 717 assets |
| Map material override | 446행: 석재443 + circle2slot + 구름1 |
| RNM placement | 444: 석재443 + circle1 |
| Native directional static shadow | 234행 |
| 전체 placement | 13,184, stable ID·source ID·TRS·visibility 보존 |
| 이번 placement asset 참조 변경 | 437: 새 석재436 + 구름527의1 |
| 이번 연결 Resources 집합 | 221파일,49,328,801 bytes: WModel77 + DDS144 |

Data 정본은 Valtan mapassets/mapmaterials/mapplacements/deployassets, MapCatalog의 Valtan count와
BossCatalog의 해당 modelMaterialOverrides8행이다. 기존 다른 ActorCatalog 값과 다른 에이전트의
Valtan 기본 particle·scale 변경은 보존했다. runtime Map 출력은 기존 Area publisher가 생성했다.

## G01. geometry와 재질 근거

A는25,819정점, B는34,306정점이며 각각 UV2/3개와 native BGRA를 보존했다. 두 모델의 slot0은
rock04 overlay, slot1은 원본 crack-floor BG다. crack는 emissive가 없고 normal·saturation·specular를
사용한다. 같은 base/static 계약의 기존 source BG와 대조해 `sourceFlags=133`을 명시했다.
root가 native material 연결 모델에서 이전 crack-emission overlay를 차단하는 소비자를 연결했다.
다른 static component의 RNM을 합성 Deploy에 복사하지 않았다.

난간20,440정점/61,884index/UV2개에는 native COLOR가 없다. 실제4slot은 detail-normal BG,
simple BG, 방향 overlay, detail-normal/subspecular BG이며 서로 다른 MIC·15 native texture를
각각 선택했다. 구름527은4정점/6index/UV2개와 source material
`lv_lut_heartrb.mat.lv_rad_redsanddst_deco03_mi_khk2`를 사용한다. 실제 engine-static SHA
`d27d8653f5fd28a7133054cd8db284feff42b25165a6e70f75161ceb6e6eb472`가 기존 program59와 일치한다.
CMapAssetObject의 translucent→BLEND→pass3와 source binder/forward PS 소비를 확인했다.

## G02. 원본 접선 부호 오류의 교정

초기 신규 cook에서 UE packed normal.W를 그대로 glTF tangent.W로 옮긴 오류를 발견했다.
UE→glTF(X,Z,Y)와 glTF→runtime(X,Y,-Z)는 각각 determinant -1이다. 준비 glTF W는 native sign의
음수여야 하며 최종 runtime W는 native sign과 같아야 한다. native UV 미분으로 얻은 종법선과
cross(N,T)를 독립 비교했다. rock3,252corner 및 floor33,045 nondegenerate corner가 이 기준과
일치했다. floor의 degenerate UV triangle6개는 이 부호 판정에서 제외했다.

이번 신규 stone/A/B75파일의152,609정점 W를 교정하고 재설치했다. 전후 position/N/T.xyz/UV0/
COLOR/UV1/UV2/index는 모두 그대로였다. 기존 정상 석재7파일은 native 비교에서 이미 정답이므로
변경하지 않았다. 난간과 구름은 교정 후에 cook했다. UModel이 native half 0을3.0517578125e-5로
내보낸 구름 UV0 세 정점도 native 값으로 교정했다. 다른 오차를 숨기기 위해 허용 오차를 넓히지 않았다.
이전 단계 receipt의 geometry SHA는 최종 `final-installed-resource-inventory.json`이 대체한다.

## G03. RNM·shadow·광원과 텍스처 경계

234 native shadow의 LightGuid는 원본 DominantDirectionalLight의
`4ba587b9fa985e4b91c324a665a0ef33`과 같다. 현재 source point light22개의 LightGuid/LightmapGuid와
443석재의 baked GUID 교집합은0이다. 기존7배치 중 원본 shadow가 있는6개에 누락된 shadow 입력만
추가했고 기존 재질 상수·색은 바꾸지 않았다. shadow exponent2 및 penumbraWidth.05는 기존
PROJECT_ADAPTER 계약이며 원작 CPU penumbra 설정 복원으로 표현하지 않는다.

이번 DDS144개 중37개 G8 static-shadow는 원본 mip7~11개를 모두 보존했다. 나머지107개 surface/RNM
DDS는 UModel이 내보낸 mip0만 설치돼 있다. 원본 compressed texture 하위 mip 전체를 복원했다고
기록하지 않는다. native SRGB 여부와 normal/RNM의 linear 데이터는 별도로 확인했다.

## G04. 실행한 검증

- 실제 source native index와 glTF corner 대응, cooker topology/channel 검사 및 최종 WModel parse 통과.
- 기존7 native tangent 비교와 신규75 W 교정 전후 다른 채널 보존 통과.
- 전체13,184 placement를 이전 snapshot과 대조해 asset 참조 외 stable ID·TRS·visibility 보존 확인.
- Valtan Area `Validate → Publish → Check` 통과.7출력,13,184배치이며 마지막 placement SHA는
  `0f1815b04645bf4adbe843f414d0a226f6689493698ebb5b35a2b0848c586547`이다.
- source527 row에 처음 추가한 unsupported castsShadow field를 publisher가 거절했다. 기존 native
  contract대로 제거한 뒤 위 검증을 통과했다. translucent queue는 shadow draw를 제출하지 않는다.

실측 자료는 `out/FullMapRestoration20260915/Valtan`의 stone/deploy/rail/cloud install receipt,
`native-tangent-basis-audit.json`, `existing-seven-tangent-audit.json`, `tangent-repair-install.json`,
`final-placement-preservation.json`, `final-installed-resource-inventory.json`이다. 새 광역 하네스는
추가하지 않았다. 공용 C++/shader/Product 빌드는 root 통합 작업이 소유한다.

## G05. 아직 연결하지 않은 범위

PS525 sky-cinema와 PS528 sky는 서로 다른 원본 shader를 사용한다.528의 native2,208정점/
11,904index/UV2개 및 lv_sky_0043_d texture를 out candidate로 준비했다. native MIC는 MLM_Custom이며
모든 feature switch가 꺼졌지만, 이것만으로 unlit이라고 단정할 수 없다. 실제 shader-cache의
NoLightmap/SH/direct 정책과 Skybox lighting channel 소비를 조사 중이며 제품에는 아직 적용하지 않았다.
527의 원본 component translucentSortPriority=-5 및 원작 전체 alpha/depth/fog 합성도 현재 공통
renderer adapter와 구분한다. 이 문서는 석재 이외 전투공간 전체 object·원경·원본 CPU 조명·후처리
복원이 끝났다고 주장하지 않는다.

Client/UI 실행·조작·캡처와 visual PASS는 수행하지 않았다. 사용자 화면 비교는 수치 검증과 별개다.

## G06. 원본 바닥 재질의 Deploy staging 실패 교정

사용자가 `overlay:BG_RAD_VALTAN_A:bg_rad_valtan_floor01a_sm`에서 입장 rollback을 보고했다.
설치 A/B는 모두 mesh2개이며 mesh1은 `bg_rad_valtan_crack_floor01_mi_lsj`, emissive 경로는
빈 문자열이다. BossCatalog의 정확한 model/material binding은 `bg-source-opaque-masked`이고
참조 texture 누락은0이다. 기존 cook receipt와 설치 SHA도 일치했다.

`CDeployPropObject::Initialize`가 native surface에도 legacy EMISSIVE texture를 요구한 것이
이 입력의 실패 조건이다. 기존 `Should_RenderDeferredEmissiveOverlay`와 동일하게 native
surface를 구분하고 LEGACY surface에만 기존 필수 검사를 유지했다. native surface는 기존
MapAssetRenderUtils/CModel/CMaterial 경로로 그린다. public header와 shader는 변경하지 않았다.

deploy flag를0으로 바꾸는 시도는 Map Effect의 파괴 바닥 owner 검증에서 거절됐다. 해당 두 행은
즉시 원복했고 catalog·placement·파괴 상태·Resources는 기존 입력을 유지했다. Area publisher의
Validate 및 Publish/Check를 통과했다. publish는 Git checkout의 CRLF를 정본 LF로 정규화했으며
runtime의 의미상 Git diff는 없다. 전체13,184배치 SHA도 이전값과 같다.

실제 변경 CPP를 동일 VS Insiders/v143 14.44 x64 Debug 도구로 독립 컴파일해 통과했다.
`out/ValtanAdmission20260915/compile-deploy.log`, `installed-model-audit.json`에 근거를 남겼다.
Product 링크는 사용 중인 Client/Server 종료를 기다리므로 아직 실행하지 않았다. 기존 프로세스를
임의 종료하거나 수정 EXE 적용·실제 입장 성공·visual PASS로 기록하지 않는다.
