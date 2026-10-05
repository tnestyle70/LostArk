# 차원술사 무비 헤어 표시 수정 결과

## G00. 확인한 원인

사용자는 무비에서 정수리·뒤통수가 비어 보이며 정상 선택창·체험하기 머리를 사용하도록 요청했다. 설치된 정상 HEAD와 무비 a753.p0는 같은 hair55다. 16,956정점·60,927인덱스이며 basis를 맞춘 position/UV0/UV1 오차0, normal/tangent 오차1.19e-7, 양의 weight→bone 이름 집합 불일치0이다. diffuse/normal TGA와 DDS의1024² RGBA도 byte exact다. 정상 머리 파일 경로만 복사하면236본/no clip과171본/Intro·Loop의 차이 때문에 무비 animation을 잃으므로 geometry 파일을 재교체하지 않았다.

실제 결함은 소비자에 있었다. CPart_Equipment는 NONBLEND에서 masked core/pass6를 그렸지만 CWorldSequenceObject는 같은 program7을 건너뛰고 depth-readonly forward/pass9만 그렸다. 또한 SourceCharacterBase7은 caller의 row26 masked switch를 source[26].x=0으로 다시 덮어 native discard를 끄고 있었다. 기존9월29일 장착 prepass도 이 shader 상수를 놓쳤다. 모발 자체의 투명 texel은 정상이며 제거할 별도 투명 오브젝트로 취급하지 않는다. 이전 shell 제외 변경과 이번 hair depth 결함을 구분한다.

## G01. 소스 반영

DeferredMaterialRenderUtils의 Render_SourceHairMaskedMesh가 기존10개 hair program의 native masked/alpha-cut 선택과 pass6 제출을 소유한다. Part_Equipment와 WorldSequenceObject가 이 함수를 같이 호출한다. world/bone transform과 기존 forward 렌더는 유지한다. 지원하지 않는 eyelash6·ghost84/88·shell702 등은 S_FALSE로 유지한다. CMaterial은 변경하지 않으며 성공·실패 뒤 임시 mode와 base 상수를 원복한다.

Engine/Client Shader_SourceCharacterBaseGroup001.hlsli는 program7의 source[26].x 강제0만 제거했다. 기본 재질 row26=0인 forward 출력은 동일하다. native alpha threshold의 기존 식을 그대로 사용한다. 새 C++·project/filter·schema·Data·Resources·렌더링 옵션 변경은 없다. 무관한 dirty 변경은 보존하고 대규모 dirty worktree이므로 stage/commit하지 않았다.

## G02. 실행한 검증

| 항목 | 실제 결과 | 근거 |
|---|---|---|
| 설치된 정상/무비 geometry·texture 대조 | PASS, 위 오차와 픽셀 동일성 | out/DimensionMasterMovieHair20261005/installed_hair_parity_receipt.json |
| 실제 helper·resolver 추출 CPU fixture |23cases/113checks PASS; 지원 분기, bind·Begin·draw 실패, cleanup·material 불변 | 같은 경로 cpp/helper_fixture_receipt.json |
| 실제 native program7 WARP depth/readback |17,952checks, failures0; alpha256종×3scenario | numeric-receipt.json |
| 투명/불투명 깊이와 뒤 물체 차폐 |282discard/486depth-write, 뒤 물체282visible/486blocked | 같은 WARP fixture |
| forward 회귀 | 이전/수정 raw와 alpha blend 최대 절대차0 | 같은 WARP fixture |
| 표준 shader consumer 격리 컴파일 | fx_5_0 /O1 exit0, CSO1,014,904bytes | consumer_compile.log·final-receipt.json |
| 변경 CPP3개 격리 컴파일 | Debug·Release 모두 exit0 | compile/Debug 및 compile/Release의3개log |
| 읽기 리뷰 |6개 WIP파일에서 추가P0/P1 없음; 제품/화면PASS가 아님 | 협업 리뷰, c999f7e91205330c5d3f18862da94289f6fa7232 기준dirty |
| diff/인코딩 | scoped diff-check PASS, 기존UTF-8 no BOM·CRLF 유지 | source-receipt.json |

WARP의 기존 native threshold는0.26298~0.399969였다. alpha0은 depth1을 유지하며, alpha101/255는 해당 기본 scenario에서 discard, alpha102/255 및255/255는 depth0.2를 기록했다. 실제 함수의 결과를 검사했으며 전체 장면의 최종 화면과 같다고 주장하지 않는다. 창·Client·UI를 실행하거나 화면을 캡처하지 않았다.

## G03. Debug·Release 제품 빌드와 배포

최초 공식 runner 시도는 실행 중인 Debug Client25312/Server25272 때문에 preflight에서 멈췄다. 사용자가 종료를 알린 뒤 표준 제품 프로세스가 없음을 확인하고 Debug→Release 순서로 정본 Invoke-BuildAndRegression.ps1을 실행했다. 기존 dirty 변경은 보존했으며 Clean/Rebuild·tracking 조작·자동 프로세스 종료·publish는 하지 않았다.

| 구성 | Product 컴파일·링크·배포 | 실행 시간 | 정본 결과 |
|---|---|---:|---|
| Debug x64 | PASS, 오류0 |115.831초 | out/BuildPipeline/runs/20261004T205210430Z-debug-product.json |
| Release x64 | PASS, 오류0 |131.488초 | out/BuildPipeline/runs/20261004T205435642Z-release-product.json |

두 구성 모두 Engine→Shared→Server→Client의 정상 증분 Build와 런타임 DLL·shader 배포를 완료했다. Debug는 OBJ101개/CSO2개/제품binary1개, Release는 OBJ100개/CSO2개/제품binary1개가 실제 갱신됐으며 유효한 기존 출력은 재사용했다. source hash6개는 빌드 중 불변이며 Engine/SDK/Client HLSLI mirror도 exact다.

Debug Client.exe는2026-10-05 05:52:08 KST, Release는05:54:33 KST에 갱신됐다. Engine.dll의 producer/Client 배포본 hash는 구성별 exact다. 두 구성의 Shader_VtxAnimMeshBinary_SourceGroup001.cso는 각각1,014,904bytes이며 SHA2562a3df838d6214558398aef596b16a0e6c710c27044e8de627897acf5545ec064로 격리 검증 consumer와 같다. 설치 근거는 out/DimensionMasterMovieHair20261005/product-install-receipt.json이다.

기존 인코딩 경고(C4819/C4828), native shader 경고(X4000/X4008/X4717), 외부 DirectXTK PDB 경고(LNK4099)는 남아 있다. 오류0을 경고0으로 보고하지 않는다. Product runner의 필수 runtime 파일·Navigation 참조·Item/Valtan reward 검사도 통과했으며 다른 domain을 publish하지 않았다.

두 구성의 Test-CompiledShaderClosure.ps1 -Modules Product도 PASS다. 각256개 활성 producer·171개 Client consumer를 확인했고 기존 Product Effect WARP V1/V2가 각각1,352pixels를 냈다. 근거는 debug-shader-closure.log와 release-shader-closure.log다. 전체 작업 트리 git diff --check도 exit0이며 기존 별도 문서들의 LF→CRLF 안내만 출력됐다.

## G04. 사용자 화면 확인

새 Debug 또는 Release에서 캐릭터 선택→차원술사→미리보기의 class movie를 재생하고 정수리·뒤통수·모발 가장자리를 기존 선택창/체험하기와 비교한다. 무비의 움직이는 머리·얇은 끝부분이 자연스러운지 최종 판정은 사용자가 직접 한다. 에이전트는 Client/UI를 실행하거나 screenshot을 생성하지 않았고, 수치 검사를 최종 visual PASS로 기록하지 않았다. 현재 프로세스를 다시 실행해야 새 EXE/CSO를 소비한다.
