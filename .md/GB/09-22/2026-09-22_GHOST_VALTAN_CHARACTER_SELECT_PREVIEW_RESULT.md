# 유령 발탄 Character Select 미리보기 재질 경로 복구 결과

대응 [구현 계획](2026-09-22_GHOST_VALTAN_CHARACTER_SELECT_PREVIEW_IMPLEMENTATION_PLAN.md). 기준은 `GB/collider-pattern-bug-fix`, HEAD `0ebd23cd1f04a6a65a09125e00ef2ac319ce0d97`의 기존 dirty 작업 폴더다. 사용자 첨부 이미지는 직접 열람했으며 Client 실행·조작·캡처는 하지 않았다.

## G01. 확인한 원인과 코드 반영

첨부 화면에서 머리와 상체는 매우 옅고 도끼와 손·발 일부는 진하게 남아 있다. `boss.valtan.ghost`의 `pBossArchetypeId`는 null이므로 Character Select는 `CBody_Valtan` 대신 `CharacterPreviewPanel::Select_Asset -> CPart_Body`로 ghost를 만든다. 설치된 모델·catalog는 동일해도 실제 render consumer가 다르다.

`CPart_Body::Resolve_TranslucentSourcePass`에 native84가 빠져 있었다. 이 때문에 모델의 3재질이 NONBLEND/pass0으로 그려지고 `Shader_SourceCharacterMaterial.hlsli`의 4×4 ordered coverage가 원본 alpha를 잘라냈다. 전투 `CBody_Valtan`에만 있던 BLEND/pass10 검증으로는 이 누락을 검출할 수 없었다.

`Client/Private/Part_Body.cpp`에 one-sided pass 상수와 native84 선택을 추가했다. 이 기존 resolver의 소비자들이 함께 바뀌므로 Initialize에서 translucent mesh를 인식하고 Late_Update에서 BLEND에 등록하며, 기본 opaque draw에서는 제외하고 Render_Translucent에서 scene lights/base/light/bone을 바인딩한 뒤 기존 pass10에 제출한다. native6/7/18/99의 pass9와 다른 프로그램의 기존 경로를 유지한다. 재질 opacity·색·조명·셰이더·모델·donor·데이터는 수정하지 않았다.

## G02. 실제 검증

- `Part_Body.cpp`를 현 Desktop include와 Debug `/c` 설정으로 컴파일했다. 성공, 제품 EXE 링크 없음. 기존 vcxproj와 filters 등록을 확인했다.
- 수정 전 파일의 resolver와 수정 후 실제 resolver를 추출해 컴파일한 routing probe는 native84 `0 -> 10`을 확인했다. 나머지 native0~108, null, legacy84의 기존 선택 보존까지 220 checks / failures0이다. 제품 객체 실행을 대신하는 테스트라고 기록하지 않는다.
- 실제 설치 ghost WModel, catalog의 3개 native84 재질/texture/parameter, 실제 cinematic donor(합산 290 clip)를 기존 `CModel`, material binding, 설치된 Engine DLL/CSO로 읽어 D3D WARP에서 비교했다. 모든 mesh의 material/light/bone/pass binding HRESULT가 성공했다.
- 모델 preTransform은 preview와 같은 `.01 * Y(-90°)`, world 위치는 party01 spawn 기준 preview offset `(2.5,0,0)`이다. saved `CharacterSelect.camera.json`의 offset/FOV와 현재 `scene.character-select.warm-high-key.v1`, 게시 maplights 4개(합계 5 lights)를 사용했다. reference viewport는 960×540, clip은 `mesh_idle_battle_1`, time0이다. 사용자 실행 메모리의 실제 카메라·편집 pose를 읽었다는 뜻은 아니다.

| mesh / material | 기존 pass0 covered | 수정 pass10 covered | 수정 pass10 colored | 수정 pass10 RGB max | nonfinite |
|---|---:|---:|---:|---:|---:|
| 0 / mn_rpbf_01-1_mi | 288 | 1346 | 1346 | 0.636512 | 0 |
| 1 / mn_rpbf_01_2-1_mi | 329 | 1821 | 1820 | 1.363281 | 0 |
| 2 / mn_rpbf_01_1-1_mi | 200 | 858 | 857 | 1.542374 | 0 |

covered는 alpha>1e-5, colored는 RGB max>1e-5다. 비교는 각 mesh를 분리한 수치이므로 3개 count를 합산해 화면 픽셀 수라고 부르지 않는다. pass0의 출력은 GBuffer diffuse, pass10은 alpha blend된 forward color이므로 두 RGB를 밝기 개선 배율로 비교하지 않는다. 기존 coverage가 원본의 낮은 alpha 영역을 버리는 경로와 수정된 연속 blend 경로를 구분한 증거다. background/depth/occlusion을 포함한 전체 Character Select 화면 검증은 아니다.

`git diff --check`는 변경 CPP와 전용 PLAN/RESULT 범위에서 통과했다. 빌드·routing probe의 기존 혼합 인코딩 헤더 warning은 해당 헤더를 재저장하지 않고 보존했다.

## G03. 백업과 남은 확인

백업 및 로컬 증거는 Git 제외 `out/GhostCharacterSelect20260922/`에 있다. `Part_Body.before.cpp`의 SHA-256은 `8E2478E8923EAB4457AA43A41A733255BCC7CC5A59477251869379CC1AAA57E9`, 수정 후는 `3AE4B97E88AE8F20D1826FB0E1592489F7F3F66E074DE1681A82B90CA62232F0`이다. 교체 직전 baseline hash를 재확인했고 UTF-8 no BOM/CRLF를 유지했다. `prepare_probe.py`, `preview_probe.cpp`, `routing-probe.log`, `mesh1-pass*.log`, `mesh2-pass*.log`와 tool stdout이 검증 증거다.

전체 제품 빌드·설치·commit/push는 이 하위 작업에서 하지 않았다. 배포 후 사용자가 Character Select의 Ghost Valtan을 다시 선택해 머리·상체의 이어지는 반투명 표시와 도끼 부착을 확인해야 한다. 원작 fidelity나 실행 중 Server의 phase3 표시까지 이번 probe로 승인하지 않는다.

## G04. 09-23 유령 발탄만 opaque로 표시하는 정책

사용자가 재강조한 대상은 **일반 발탄이 아니라 유령 발탄(native84)** 이다. 이전 G01의 pass10 연결은 원본의 낮은 alpha를 그대로 합성하므로 이번 요청의 불투명 표시를 충족하지 않는다. 현재 native84는 diffuse alpha와 cloud/rim 계산으로 opacity를 만든다. pass10은 opacity<=1e-4를 버리고 나머지도 alpha blend하며 깊이를 쓰지 않는다. native Base의 별도 luminance/opacity discard는 source[23].x가0인 현재 catalog에서 비활성이다. shader submission 성공과 몸체의 실질적인 표시를 구분한다.

현재는 native84만 새 pass16을 사용한다. 기존 native RGB와 광원 계산은 보존하고 alpha1, BS_Default, DSS_Default를 사용한다. Base84의 선택 인자 opaqueGhost 기본값은false여서 기존 source discard를 유지하며 pass16만 우회한다. source diffuse/RGBA 데이터, material parameter, 일반 발탄, Sea native88, hair/eye, 맵 청록색 masked 표면, pose afterimage는 변경하지 않았다. 이는 사용자 요청의 PROJECT_AUTHORED opaque 표시 정책이며 원본 ghost translucency 복원이라고 기록하지 않는다.

CBody_Valtan과 generic CPart_Body 모두 기존 NONLIGHT 그룹에 ghost를 제출한다. 이 그룹은 SceneHDR의 deferred 합성 뒤, scene snapshot과 BLEND 효과 전에 실행된다. 따라서 현재 opaque 장면 깊이를 읽고 ghost 깊이를 쓴 뒤 반투명 효과가 그 깊이를 소비한다. BLEND 큐 안에서 뒤늦게 opaque를 그려 이미 합성된 효과를 덮는 순서 문제를 피한다. 유령 shadow는 새 depth-only pass17을 사용해 packed diffuse alpha로 잘리지 않는 동일 silhouette를 기록한다. 일반 body/그 shadow와 ghost의 pose afterimage는 기존 경로를 유지한다. F1 진단은 NONLIGHT opaque pass16과 제출 mesh 수를 표시한다.

변경은 Body_Valtan.h/cpp, Part_Body.h/cpp, Shader_VtxAnimMeshBinary.hlsl, Shader_SourceCharacterForward.hlsli, Engine/Client Shader_SourceCharacterBaseGroup084.hlsli와 기존 generator build_vehicle_source_material.py의 native84/base 분기다. 새 파일이나 project/filter 항목은 없고 기존 등록을 확인했다. 기존 C++의 UTF-8 no BOM/CRLF를 보존했다.

### 실제 수치 검증

기존 headless probe를 현재 BossCatalog 3재질의 parameter/texture에서 다시 생성했다. 설치된 Ghost/MN_RPBF_02.wmodel와 cinematic donor290clip, .01 preScale·-90도 회전, saved Character Select 카메라, party01 기준 previewOffset2.5m, 현재 profile과 map light5개, 960×540, mesh_idle_battle_1/time0 조건이다. 이번에는 실제 DSV를 연결했고 전체3mesh를 함께 그렸다. 이전 G02의 mesh별 무DSV 수치를 이 표와 합산하지 않는다.

| 조건 | pass | covered / colored | alpha min~max | RGB sum | depth written | nonfinite |
|---|---:|---:|---|---:|---:|---:|
| 원본 opacity | 10 | 3591 / 3589 | .000101378~1 | 532.663983 | 0 | 0 |
| opaque 정상 | 16 | 12959 / 12959 | 1~1 | 6531.351787 | 12959 | 0 |
| opacity0 | 10 | 0 / 0 | 출력 없음 | 0 | 0 | 0 |
| opacity0 | 16 | 12959 / 12959 | 1~1 | 6531.351787 | 12959 | 0 |
| source[23].x=1 native cutout 활성화 | 16 | 12959 / 12959 | 1~1 | 6531.351787 | 12959 | 0 |
| 앞쪽 depth .5로 차폐 | 16 | 0 / 0 | 출력 없음 | 0 | 0 | 0 |
| opacity0 그림자 | 17 | color 출력 없음 | 해당 없음 | 0 | 12959 | 0 |

pass16의 RGBmax는2.147752762다. opacity0에서도 동일한 양수 RGB와 온전한 silhouette가 유지되는 것을 확인했다. D3D state readback은 pass10 blend1/depthWrite0, pass16·17 blend0/depthWrite1을 확인했다. native discard/opaque depth 및 RGB 수치 성공이 실제 사용자의 전투 화면·occlusion·suppression·원작 fidelity를 대신하지는 않는다.

Body_Valtan.cpp와 Part_Body.cpp는 Debug MSVC /c compile PASS다. SourceGroup084 FX /Od compile PASS이며 기존 다른 source program의 uninitialized/divide-by-zero 경고를 이번 ghost 수치의 성공으로 숨기지 않았다. standalone probe CSO를 제품에 설치하지 않았다. 전체 Product build·설치는 통합 작업 RESULT가 소유한다.

기존 partition 전체 unittest는2실패였다. 이번 변경 전 source84를 임시 corpus에 복원해도 동일하게 Base214/224/235/237과 dispatcher의 기존 drift가 발생했다. 이 무관 파일은 교정하지 않았다. 별도 검증에서 Base84 leaf는 수정 전후 모두 partition roundtrip 보존이며, 기존 native84의 실제193명령을 generator로 재생성해 optional false/opacity guard와 다른 program의 무변경을 확인했다. Engine/Client Base84 mirror는 byte-identical이다.

`git diff --check`는 이번 변경 범위 PASS다. 백업·source encoding/hash·컴파일·GPU·generator 근거는 Git 제외 out/GhostOpaque20260923에 있다. Client/UI 실행·조작·화면 캡처, 서버 변경, commit/push는 하지 않았다. 사용자 화면에서는 Ghost Valtan 몸체·도끼 가림과 불투명 silhouette를 직접 확인해야 한다.

필수 검증이 끝난 뒤 별도 O3 재컴파일과 pass10의 추가 source-mask/depth 조건 반복은 중단했다. 그 추가 실행을 PASS로 기록하지 않는다. 이미 완료된 FX84의 pass16·17 CShader::Begin과 실제 GPU state/depth/색 제출은 위 표의 증거다. 기본 FX와 전체 SourceGroup의 제품 산출물 갱신은 Product 빌드에서 함께 수행해야 한다.

### G04 후속 검토: 전체 variant admission 교정

독립 검토에서 pass17의 최초 hardcoded ProgramVariantPass=1이 모든 SourceGroup에서도1이어서 base shader 생성이 실패하는 결함을 발견했다. CShader::Stage_ProgramVariants는 BASE(1) pass에 대해 각 shard가 UNAVAILABLE(2)여야 한다고 검사한다. 통합 담당자가 기존 BINARY_ANIMATED_NATIVE_PASS_POLICY(base1/shard2)로 교정했다. 최초 direct SourceGroup084의 Begin17/GPU 성공은 이 전체 admission을 실행하지 않아 해당 결함을 검출하지 못했다. 위 표의 GPU 수치는 최초 정책 상태에서의 pass 연산 증거이며 최종 base FX 전체 생성 성공 증거가 아니다.

Product 배포 뒤 실행할 최소 out/GhostOpaque20260923/base_admission_probe.cpp와 Test-ProductGhostBaseAdmission.ps1을 준비했다. 실제 배포 Engine DLL과 base+14cohort CSO의 SHA256 동일 복사본을 격리 폴더에 두고, 정확한 base logical name으로 CShader::Create하여 전체 pass 이름·signature·policy·variable admission을 수행한다. base clone의 program84/pass16과 base-owned pass17의 blend/depth/PS 상태, direct shard84의 pass17 거절까지 확인한다. 모델/텍스처 decode와 draw, Client/UI 실행은 없으며 제품 파일을 교체하지 않는다. 이 준비 시점에는 아직 컴파일·실행하지 않았다. 실행과 최종 Product 상태는 통합 담당 RESULT가 기록한다.

### G04 최종 제품 admission 검증 (2026-09-23 11:24 KST)

사용자 Visual Studio 빌드가 끝난 뒤 실제 Client/Bin/Debug 배포본으로 위 probe를 컴파일·실행해
PASS했다. base CShader 생성과14개 cohort admission, base program84/pass16의
blend0/depthWrite1/PS 유효, base pass17의 blend0/depthWrite1/PS 없음, 직접 shard84의
pass16 성공·pass17 거절을 확인했다. 산출물 SHA256과 로그는
out/GhostOpaque20260923/admission-debug/{product-inputs.json,compile.log,probe.log}다.
이는 최초 direct-shard 검사의 빈틈이었던 최종 base 변형 등록까지 검증한 결과다.
windows0/draws0이며 사용자가 실행한 Client/Server의 종료·조작·화면 캡처는 하지 않았다.
현재 설치·실행본과 장시간 셰이더 빌드 비용의 별도 경계는 Character Select RESULT G11을 따른다.

사용자는 같은 실행본을 직접 확인한 뒤 “유령발탄 쪽 셰이더 수정에 의한 렌더링은 잘 나온다”고
판정했다. 유령 발탄의 이번 불투명 표시 요청은 사용자 화면 확인까지 완료했다. 캐릭터 선택
맵의 색·재질 복원 실패 판정과 구분하며, 모든 전투 상황·원작 fidelity의 포괄 승인으로 확장하지 않는다.

## G05. 유령 표시를 유지한 animated shader 컴파일 범위 축소

사용자가 성공 판정한 유령 발탄의 RGB·alpha1·depth-write 정책을 유지하면서 불필요한 native dispatcher 재컴파일을 줄였다. Shader_SourceCharacterForward.hlsli에서 이번 ghost helper 변경만 제거했고, 그 결과 이 파일은 git HEAD 및 첫 변경 전 백업과 같은 내용으로 돌아갔다. ghost entry 본문은 Shader_VtxAnimMeshBinary.hlsl의 shared Forward include 바로 아래로 이동했다. 실제 본문은 SOURCE_CHARACTER_PROGRAM_GROUP==84에서만 컴파일되며 base0와 다른 cohort는 unconditional discard stub이다. Base84(baseInput,true)와 Light84(lightInput)를 직접 호출한다. native Base84의 기본 false, 원본 pass10, pass16/17 state/index와 pass17 base1/shard2 정책은 유지했다.

기존 PLAN에 G05의 책임·호출 흐름·두 함수 전체를 먼저 기록한 뒤 tracked 두 파일의 SHA256을 후보 생성 당시 값과 다시 비교했다. 검토된 후보와 byte-identical하게 적용했고 UTF-8 no BOM/CRLF를 유지했다. 새 파일·프로젝트 등록·C++·data 변경은 없다. Git 제외 out/GhostCompileOptimize20260923의 candidate.patch, tracked-apply-guard.json, tracked-apply-receipt.json과 before-tracked-apply가 적용 근거다.

### G05 컴파일과 코드 크기 검증

대표 cohort001 FX /Od는9.758초, cohort084 FX /Od는78.847초에 PASS했다. 해당 ghost entry만 별도로 /O1 컴파일하여 현재 설치 Product /O1 FX에 들어 있던 PS와 비교했다.

| cohort | Product PS tokens / instruction slots | 후보 PS tokens / instruction slots | 후보 switch |
|---|---:|---:|---:|
| 001 | 33845 / 4260 | 29 / 4 | 0 |
| 084 | 39274 / 5069 | 5307 / 699 | 0 |

native84의29개 Light case가 제거됐고 non84는4개 명령의 discard stub이다. token 감소율은 각각99.91%,86.49%이며 전체 FX 빌드 시간이나 프레임 성능의 감소율이 아니다. 기존 일반/투명 PS는 계속 컴파일한다. shared Forward 복원 자체는 다음 실제 빌드에서 static FX를 한 번 invalidate한다. 이후 ghost 본문만 편집하면 이 공유 include를 통해 static cohort 전체에 전파되던 원인은 제거된다.

적용 뒤 허용된 추가 검증으로 base FX /Od를 out에서 한 번 컴파일해39.635초 PASS했다. 새 base와 대표001/084 후보, 나머지12개 실제 Product CSO 복사본으로 CShader::Create의14개 cohort admission을 PASS했다. base program84/pass16의 blend0/depthWrite1/PS 있음, base pass17의 blend0/depthWrite1/PS 없음, direct84 pass17 거절도 PASS다. base-admission 검증은 windows0/draws0이며 로그는 candidate-base-admission.log다.

### G05 실제 메시 WARP 비교

동일 설치 ghost3mesh WModel, cinematic donor290clip, 기존5광원 fixture, mesh_idle_battle_1/time0,960×540에서 Product Engine.dll 복사본을 사용했다. RGBA32F와 depth 전체를 읽어 현재 Product O1과 후보 Od를 비교했다.

| 항목 | 현재 Product와 후보 비교 |
|---|---|
| pass16 | 양쪽12959 covered/colored, alpha1, depth12959, nonfinite0 |
| pass16 전체 buffer | coverage·alpha 동일, depth bitwise 동일; 최대RGBA차이1.1920928955e-7, RMSE1.5834146134e-10 |
| pass17, 원본opacity0 | 양쪽color0/depth12959; 전체RGBA·depth bitwise 동일 |
| 기존 pass10 | 양쪽3591 covered/3589 colored, depth0, nonfinite0; coverage·depth 동일, 최대RGBA차이1.4305114746e-6, 최대alpha차이1.0579824448e-6 |

후보 base를 새로 컴파일하기 전에는 Product base로 위 비교를 했고, 새 base를 넣은 뒤 pass16/17 draw를 다시 실행해 같은 수치와 depth 동일성을 확인했다. 서로 다른 O1/Od 최적화 수준의 작은 float 차이를 완전한 RGB byte 일치로 표현하지 않는다. 원본 pass10 회귀도 동일 coverage와 허용한 수치 오차 안이다. 기록은 pixel-comparison.json, bytecode-comparison.json, product-inputs.json, full readback buffer와 각 probe 로그다.

전체 Product 빌드·설치, 다른12개 cohort 재컴파일, Client 실행·종료·UI 조작·사용자 화면 재판정은 하지 않았다. 새 source는 다음 승인된 제품 빌드에서 반영된다. 기존에 사용자가 성공 확인한 실행본을 이 최적화 산출물로 교체하지 않았다. 두 shader와 대응 PLAN/RESULT의 git diff --check 및 evidence JSON parse는 PASS다.
