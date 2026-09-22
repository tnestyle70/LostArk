# 쿠크 2관문 클리어 포탈 조명 조사

## G00. 요청과 현재 경계

사용자는 동일한 정지 시각에서 카메라를 보라색 포탈 안으로 옮기면 배우가 밝게 보이고,
바깥에서는 포인트 라이트를 추가해도 어둡다고 보고했다. 원인 조사와 수정을 요청했다.
두 첨부 이미지의 자막 `cin.37081_31_03`으로 Sequence `KAKULSAYDON_G1_PATTERN_5`
(`2관문_클리어`)의 8,942~10,442ms 구간을 찾았다.

현재는 **포탈 전체 소실의 WMSH 계약 위반 재현·수리 설치·실제 문서 resource staging 통과,
사용자 화면 확인 전**이다. G05~G08은 최초 수정 당시의 조사 기록이며, 당시 제품 모델 로드를
검사하지 못한 결함과 후속 교정은 G09에 기록한다. 사용자에게 같은 시각·카메라·라이트를
유지하고 포탈 이펙트만 숨기는 비교를 요청했다. 사용자는 실행 후 결과를 알려주겠다고
답했다. 이후 받은 두 이미지는 같은 장면의 바깥/안쪽 비교이며 포탈 OFF 결과라고 간주하지 않았다.
추가 조사와 검증은 아래 G05 이후에 기록한다. 사용자의 `전체다 반영해` 승인 후
파생 Resources 하나와 저작 JSON의 meshModel binding 한 곳을 처음 반영했다.
제품 C++/HLSL 및 별도 게시 데이터는 변경하지 않았으며, 생성용 Python 도구와 문서를 추가했다.

## G01. 실제 연결 확인

- 배우는 `world.object.kouku.gate2.clear.largesaydon`과
  `world.object.kouku.gate2.clear.kouku`다. 각각 Character의 `MN_RPCT_06.wmodel`,
  `MN_RPCZ_00.wmodel`을 사용하며 material source도 같은 Character 모델이다.
  `CWorldSequenceObject -> Bind_DeferredMaterialInputs -> CModel/CMaterial`의
  source character geometry/light 경로를 소비한다. Map lightmap에 고정된 캐릭터 경로가 아니다.
- Pattern 5의 저장된 `sceneProfileOccurrences`는 비어 있다. 기존 Client 세션 기록의
  해당 컷 scene은 `scene.kakulsaydon.g1.base.v1`이다. 현재 이 profile과 해당 G2 region의
  directional diffuse는 0.8, specular는 0.5다. 저장본에서 방향광이 꺼져 있다는 근거가 없다.
  스크린샷 순간의 모든 미저장 UI 설정까지 확정한 것은 아니다.
- `Collect_FrameLights`가 추가한 조명은 transient light로 제출된다.
  `Resolve_LocalLight`는 거리 감쇠와 재질의 직접광을 계산한다. 이 경로에는 포탈 메시를
  추적해 외부 포인트 라이트를 차단하는 연산이 없다.
- Renderer는 NonBlend/G-buffer → Lights → Combined → Blend 순서다.
  포탈에는 alpha 합성 재질이 있다. `BS_EffectAlpha`는 SrcAlpha/InvSrcAlpha를 사용하므로
  뒤의 조명 결과를 약화시킬 수 있다. 실제 스크린샷의 주원인인지는 포탈 표시 비교가 필요하다.

## G02. 실제 모델과 저장된 광원의 범위

Sequence occurrence `KAKULSAYDON_G1_PATTERN_5.presentation.38`은
`kakulsaydon.g1.presentation.65 -> light.runtime.7`을 9,085~12,085ms에 사용한다.
리소스 위치 `(3.55710793, -4.48999977, 322.069733)`와 occurrence offset
`(-7.150000095, -10.199999809, -9.550000191)`을 합친 광원 위치는
`(-3.592892165, -14.68999958, 312.51973281)`이다.
range는 15.1000004, falloff는 0.860000014, brightness는 30.9500008이다.

9.5초의 실제 설치된 WANM을 채널별로 sample하고 inverse bind × combined pose,
modelPreScale, WorldSequence quaternion/position을 적용했다. actor root만으로
수광 범위를 판단하면 거대 세이튼의 골격 이동을 놓친다.

| 배우 | 정점 수 | 광원으로부터 정점 거리 최소~최대 | 반경 안 정점 비율 |
|---|---:|---:|---:|
| 거대 세이튼 | 82,833 | 7.708~19.883m | 53.04% |
| 작은 쿠크 | 30,106 | 5.350~6.873m | 100% |

전체 정점의 기하학적 범위 검사이며 화면에 보이는 픽셀의 비율은 아니다.
거대 모델의 범위 부족은 확인됐지만, 작은 쿠크까지 검게 보이는 현상의 단독 설명이 아니다.
계산과 출력은 `out/KoukuPortalLighting20260922/sample_actor.py`,
`actor-light-range.log`에 있다.

## G03. 실제 배포 재질의 GPU 수치 검사

기존 LightEarlyOut의 offscreen fixture를 out에 복사해 현재 128-byte light record에
맞췄다. 현재 BossCatalog의 실제 4개 material parameter를 제품
`SourceCharacterMaterial::Configure`로 packing하고 해당 Resources DDS를 원래
sRGB/linear 설정으로 바인딩했다. 현재 배포된 `Shader_Deferred_SourceGroup017.cso`,
`Shader_Deferred_SourceGroup025.cso`의 point light pass를 사용했다.

| 재질 | native program | 구 밖 camera distance 20의 조명 출력 픽셀 | 구 안 camera distance 3의 조명 출력 픽셀 |
|---|---:|---:|---:|
| mn_rpct_05_mi | 26 | 3,751 | 4,096 |
| mn_rpct_05-1_mi_loc_int | 21 | 3,559 | 3,960 |
| mn_rpct_05-2_mi | 21 | 3,658 | 4,096 |
| mn_rpcz_00_mi | 26 | 3,232 | 3,750 |

각 64×64 synthetic G-buffer에서 point range 15.1, falloff 0.86, brightness 3을
사용하고 bounds flag를 off/on했다. 16개 조합 모두 직접광 출력이 양수였으며
출력 RGB의 non-finite는 0이었다. bounds off/on의 출력 집계도 일치했다.
이는 실제 재질이 추가 point light를 지원한다는 증거이며, 전체 컷신의 GPU frame,
포탈 합성 결과 또는 사용자 화면 PASS를 뜻하지 않는다.

초기 probe에서 program21을 group025로 호출해 0을 얻은 것은 검사 도구의 잘못된
variant 선택이었다. 제품과 동일하게 program별 group을 선택하도록 고친 최종 실행은
exit 0이다. 제품 결함으로 기록하지 않는다. 임시 도구 컴파일에는 기존 Engine 헤더의
문자 집합 경고가 있었고 제품 파일의 인코딩은 변경하지 않았다.

증거는 `out/KoukuPortalLighting20260922/gpu/point_probe.cpp`, `compile.log`,
`point_probe.log`다. Client/UI 자율 실행·조작·캡처, 제품 Build/Publish는 하지 않았다.
조사 마지막 확인 시 Client PID39020과 Server PID43032가 각각
`Client/Bin/Debug/Client.exe`, `Server/Bin/Debug/Server.exe`에서 실행 중이었다.

## G04. 다음 수정 판단

초기 조사에서는 포탈 OFF 비교를 기다렸다. 후속 조사는 실제 element의 셰이더·원본 입력·
설치 geometry·배우 pose·카메라를 연결해 원인을 확인했다. 공통 조명이나 재질을 바꾸지 않는다.

## G05. 검은 원통 합성의 원인 확정

새 이미지의 자막 `장르를 바꾸는 거야. 로맨스?`는 10,991~13,991ms다.
이 구간에는 초기 blackcircle3337/hole3338/cone2832가 이미 종료됐고, 아래 요소가 활동한다.

- 문서: `effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.1.effect.json`.
- stable element: `kouku.action.233d0d5adbba2933178bf071`.
- 원본: `par_q_darkfield_01.particlespriteemitter_75`, native3330.
- 실제 메시: `Effect/KoukuSaydon/FullRestore/Meshes/fm_b_cylinder_002.wmodel`.
- 시각: 약8.257~17.191초. 원본 unlit/translucent/one-sided를 현재도 사용한다.

`ArtistNative3330`의 RGB는 emissionGradient × particleColor.rgb + selectionColor.rgb다.
두 색이 모두0이므로 결과는 검정이다. 원본 Color 모듈의 null distribution뿐 아니라
실제 cooked class default StartColor0/StartAlpha1도 대조했다. 누락된 흰색 기본값이 아니다.
alpha는 depthGapCm/50의 saturate와 view-normal 항의 곱이며 최대1이다.
초과/음수 alpha에 의한 HDR blend 결함은 발견하지 못했다.

검정 alpha1이 `SrcAlpha/InvSrcAlpha`로 합성되면 이전 캐릭터 색은0이 된다.
조명 계산 뒤에 적용되므로 추가 라이트를 높여도 가려진다. 포탈은 Shadow queue를
통해 외부 빛을 차단하는 것이 아니다. Directional의 위치는 조명 계산 기준이 아니며
방향·surface normal·shadow가 중요하다. 이 조사에서는 방향광 OFF 근거가 없다.

실제 원통은 반지름8.75m, 길이262.5m이고 법선/winding은 바깥쪽이다. 원래 camera는
축에서 약17.9m 떨어져 있다. 실제 설치 WModel skin 및 camera key를 보간해 광선을 검사했다.

| 시각 | 세이튼 정점 앞의 원통 교차 | 쿠크 정점 앞의 교차 |
|---|---:|---:|
| 9.5초 | 97.12% | 100% |
| 12초 | 97.13% | 100% |

안쪽 비교 eye에서는 뒷면 컬링으로 교차가0이다. 12초 쿠크 앞벽과 배우 간격은4.19~6.15m로
0.5m depth fade보다 크다. 이 수치는 화면 픽셀 점유율이 아니다.
원본 source transform·material cull도 현재와 일치했다. 원본 mesh glTF는 현재 경로에 없어
원작 엔진의 winding/전체 화면까지 확정하지 않았다. 수정은 PROJECT_TUNED 배경막 정책이다.

## G06. 수정 후보와 검증

`Tools/KoukuSaydonPipeline/prepare_gate2_clear_portal_backdrop.py`는 기본 실행에서 후보만 만든다.
`--apply`는 최신 저장본 stable element의 mesh binding만 병합하고 기존 공유 원본을 유지한다.
후보는 `out/KoukuPortalLighting20260922/candidate/`에 있다.

파생 ID는 `Effect/KoukuSaydon/FullRestore/Meshes/fm_b_cylinder_002_gate2_clear_inward.wmodel`이다.
11,592bytes이며 triangle winding을 안쪽으로 바꾸고 local X/Z만2배로 넓힌다. 보라색
이펙트 메시·shader·색·시간·조명은 수정하지 않는다. WModel material section과 기존
normal/tangent/UV는 그대로다. 이 새 Resources는 사용자 승인 후 제품 경로에 설치했다.

winding만 바꾼 초기 후보는 작은 쿠크 가림0이지만 세이튼4~6%가 먼 벽 밖으로 나가 남았다.
최종 반지름17.5m 후보는8.257~17.191초의31개 시각(0.5초 간격 및 camera 전환±1ms 포함),
배우62개 pose에서 원통 앞벽의 가림 정점0을 확인했다. 실제 최대 배우 반경은10.9264m였다.
먼 배경벽도 남는다.15.757초 거대 세이튼의3,966개 광선은 원래 열린 원통 끝으로 빠지므로
모든 화면 방향의 배경이 완전히 검정이라고 보장하지 않는다. 전체 연속 프레임·화면 검증은 아니다.

배포된 `Shader_VtxEffectMeshKouku3328.cso`를 사용하는 offscreen WARP 검사에서도 기존
one-sided 앞면과 native3330이 배경 RGB0.25/2/20을 모두0으로 덮는 것을 재현했다.
삼각형 순서를 뒤집은 앞벽은 같은 입력을 그대로 보존했다. alpha strength0.2 비교도
예상 배경×0.8과 일치했다.12개 조합 PASS/exit0이며 화면 캡처나 Client 실행은 하지 않았다.

기존 Sep21 제품 codec probe로 후보 문서 Parse_Value/Validate_Drawable을 실행해
perDoc.ok=true를 확인했다. 후보용 Resources에61개 기존 asset을 읽기용 hardlink로
연결해 검사했고 원본 파일은 수정하지 않았다. 첫 검사의 texture invalid는 resource root를
설정하지 않은 fixture 오류이며 실제 root를 연결한 재실행이 통과했다.
ModelAssetConverter info에서도2sections,0animations,static mesh/material을 확인했다.
이 검사는 실제 CModel decode를 수행하지 않았고 WGEO payload digest·bounds 검증 실패를
놓쳤다. 당시 이 결과를 제품 resource staging 성공의 근거로 사용한 판단은 잘못이었다.

기존 native admission은 meshModel 하나의 유효 경로를 요구하며 이 profile의 경로를
원본 이름으로 고정하지 않는다. `ResourceStaging -> CRuntimeAssetRoot -> CModel`과
기존3330 shader/texture binding을 그대로 소비한다. `sourceRecipe`의 원본 objectpath는
출처로 보존한다. EffectCatalog의 DIRECT_AUTHORED_DOCUMENT를 읽으므로 별도 publisher나
Composition 변경은 필요하지 않다. 메모리 캐시 Reload는 별도다.

증거: `portal_geometry.*`, `portal_inward_candidate.*`, `portal_radial_candidate.*`,
`gpu/portal_probe.*`, `candidate/codec-result.json`, `candidate/receipt.json`.

## G07. 최종 반영 경계

AGENTS.md의 편집 중 데이터 반영 규칙에 따라 준비·검증을 먼저 끝냈다.
사용자가 `전체다 반영해`로 승인한 뒤 `--apply`를 실행했다. 원본 mesh와 최신 문서를
재확인하고 JSON 백업·hash 비교·atomic replace를 거쳐 디스크 반영을 완료했다.
다른 mesh를 선택한 실제 필드 충돌은 없었고 무관한 사용자 변경은 보존했다.
Client/UI 자율 실행·Reload·종료, Product 빌드, commit/push와 Drive 업로드는 하지 않았다.

## G08. 승인 후 실제 설치와 최종 확인

- 새 파일: `Client/Bin/Resources/Effect/KoukuSaydon/FullRestore/Meshes/fm_b_cylinder_002_gate2_clear_inward.wmodel`, 11,592bytes.
- 파생 파일 SHA256: `6d2e9d058dc42e8b0779492e24992f65490363bc5b18dc3968eae95771499dba`.
- 공유 원본 SHA256은 `226868c59498af8d33ec6be625db3bae2f4b1ee17055dee238bc261da907c96d`로 유지됐다.
- 저작 JSON 변경은 해당 stable element의 assetId 한 줄이다. SourceRecipe·native profile·
  source texture·parameter·배우·카메라·광원·이펙트 시각은 보존됐다.
- 최신 JSON SHA256: `81b0cf4579a700eed10a64ed5c7604fa67975e85e8b077b018f038bc7add1b4a`.
- 승인 직전 저장본 백업: `out/KoukuPortalLighting20260922/candidate/document-before-4b86e5495a19e895239cdd283ce08323f0c8bd17d60d46cc14d61400ef1ab35e.json`.
- 실제 설치 Resources root와 실제 저작 JSON으로 동일 codec Parse_Value/Validate_Drawable을
  다시 실행해 `perDoc.ok=true`를 확인했다. 출력은 `installed-codec-result.json`이다.
- 독립 바이트 대조에서26정점·72인덱스의 변경120bytes가 X/Z 확대와 winding 반전에만
  한정됨을 확인했다. 다른 vertex lane·material·section bytes는 동일하고 소비자는 하나다.
- Python syntax와 대상 diff whitespace 검사를 통과했다.

데이터 변경이므로 EXE/HLSL 빌드는 필요하지 않다. 실행 중인 기존 occurrence와 cached 문서를
자동 갱신하지 않았으며 사용자 화면 판정은 아직 수행하지 않았다. 현재 UI의 `Load Saved`는
Tool 문서/preview만 갱신하고, Product hot reload는 실제 수정 후 Save에서만 호출된다.
clean 상태에서 Save는 비활성이므로 단순 Load Saved→Play를 Product 적용 절차로 안내하지 않는다.
사용자가 다른 편집을 저장한 뒤 Client를 재실행하여 컷신을 새로 재생하면 설치본을 읽는다.
Server 재시작은 필요하지 않다. 파일 반영 자체는 Client 실행 중에 이미 완료했다.
Resources는 Git 제외 새 파일이므로 다른 PC에는 위 asset을 기존 팀 Drive 경로로 별도 전달해야 한다.

## G09. 포탈 전체 소실 재현과 WMSH 수리

최초 반영 뒤 사용자가 포탈 전체 소실을 보고했다. 현재 Resources에는 파생 파일이 있으므로
단순 파일 누락이 아니었다. 현재 저장본의 대상 요소는 visible=true, native3330,
`alpha_one_sided_depth_read`, mesh preScale=.01을 유지하고 있었다. 저장된 start는
8.139344431459904초, life는8.935094833374023초였다. 이 최신 시간은 이번 수리에서 보존했다.

`prepare_gate2_clear_portal_backdrop.py`의 기존 `make_backdrop`은 vertex X/Z와 index를
바꾼 뒤 원본의 embedded bounds와 WGEO payloadSha256/metadataSha256을 그대로 두었다.
제품 `CWMeshReader::ValidateGeometryMetadata`가 이를 거부한다.
`Effect_DocumentRenderer_ResourceStaging.cpp`의 `CModel::Create` 실패가
`Effect_DocumentRenderer_PreparedDocument.cpp`의 `Build_PreparedDocument` 실패로 전달되어,
해당 원통뿐 아니라 같은 문서의 포탈 요소 전체가 준비되지 못한다. cull/pass 적용 전의 실패다.

현재 설치 Engine.dll, 현재 WMeshReader 소스와 D3D11 WARP 장치로 실제 파일을 읽었다.
별도로 현재 프로젝트에 등록된 Client Debug object만 묶은 console probe에서 실제
`CEffectDocumentRenderer::Stage_Document`를 호출했다. 창·swapchain·Client 실행은 없었다.

| 입력 | CWMeshReader / CModel | 실제 91-element 문서 staging |
|---|---|---|
| 공유 원본 cylinder | 성공 / 성공 | 원본 복구는 불필요하여 별도 검사하지 않음 |
| 최초 파생 설치본 | 실패 / 실패: geometry payload SHA-256 불일치 | 실패: 해당 파생 CModel load failed |
| SHA만 고친 음성 대조 | 실패 / 실패: embedded bounds와 정점-derived bounds 불일치 | 수행하지 않음 |
| bounds와 SHA를 함께 고친 후보 | 성공 / 성공 | 성공 |
| 승인 후 실제 설치본 | 성공 / 성공 | 성공 |

수정 도구는 입력 payload/metadata digest를 먼저 확인하고, 파생 정점의 min/max/center/radius,
payloadSha256, geometryToolSha256, metadataSha256을 갱신한다. 나머지 source digest는
입력 계보로 유지하며 원본 geometry와의 동일성 인증으로 사용하지 않는다. evidenceFlags의
원본 정확성 인증 bit는0이고 이 파생은 계속 PROJECT_TUNED다. 알려진 최초 손상 SHA 또는
동일 생성 결과만 교체를 허용한다. 최신 원본 hash·교체 대상 bytes를 확인하고 JSON/mesh
백업과 atomic replace를 사용하며 후속 실패 시 자기 mesh 교체를 rollback한다.

최초 손상본 대비 변경은105bytes이며 embedded bounds와 위3개 digest에만 한정된다.
26개 정점·72개 index·UV/normal/tangent·material section은 최초 파생과 완전히 같다.
따라서 이번 수리는 반경·winding·shader·cull/pass·색·배우·조명·카메라·시간을 다시 바꾸지 않는다.
최초 합성 삼각형 WARP는 one-sided/pass의 제한된 근거로만 남고 실제 파생 파일 로드 근거를
대체하지 않는다. 이번 headless 검증도 전체 컷신 draw나 사용자 화면 PASS가 아니다.

사용자의 `발탄 돌이랑 같이 전부 다 수정하고 빌드까지 돌려줘. exe 종료했어` 승인 후 설치했다.

- 수리 설치 파생 SHA256: `5620d75d1031ac09cc48ca960a3f39b0496fb5edc3786a967118b368d1d96ec1`.
- 공유 원본 SHA256: `226868c59498af8d33ec6be625db3bae2f4b1ee17055dee238bc261da907c96d`, 변경 없음.
- 저작 JSON SHA256: `81b0cf4579a700eed10a64ed5c7604fa67975e85e8b077b018f038bc7add1b4a`, 전후 동일.
- 손상본 백업: `out/KoukuPortalLighting20260922/candidate/mesh-before-6d2e9d058dc42e8b0779492e24992f65490363bc5b18dc3968eae95771499dba.wmodel`.
- 증거: `out/KoukuPortalLoader20260922/before.log`, `digest-only.log`, `candidate.log`,
  `installed.log`, `stage-before.log`, `stage-candidate.log`, `stage-installed.log`,
  `repair-audit.json`, `installed-receipt.json`.

Python syntax와 대상 diff whitespace 검사도 통과했다. 독립 probe 컴파일의 기존 헤더
문자집합 경고와 Debug object의 EDITANDCONTINUE 무시 경고는 기록했으며 제품 소스를
바꾸지 않았다. 통합 Engine/Shared/Server/Client Debug Product 빌드와 배포는 exit 0으로
완료했다. receipt는 `out/BuildPipeline/runs/20260922T011037720Z-debug-product.json`이며
missing/invalid runtime input은 0이다.
사용자 재실행 뒤 화면 판정과 다른 PC의 수리된 Resources 전달은 아직 별도다.
