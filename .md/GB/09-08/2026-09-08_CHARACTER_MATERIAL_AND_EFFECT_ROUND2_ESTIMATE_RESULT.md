# 캐릭터 재질·차원술사 Q 2차 복구 구현 결과

## 현재 상태

사용자는 Q 전체의 유리 표시와 All Effects 첫 목록 자동 표시를 확인했고, Family별 재생에서
방향·위치·sprite 분포 차이와 Play All의 정지 후 끝 이동을 보고했다. 이 피드백을 기준으로
Q의 원본 기본값·기준축과 Sequencer 재생 제어를 추가 교정했다. 전체 원본 유사도와 새
Play All의 화면 결과는 아직 사용자 재확인 전이다. 성공한 Q2 비교본은 그대로 보존한다.

Q 공간·재생 교정 당시 C++ 4파일은 Debug 최소 컴파일을 통과했다. JSON 입력·49개 소스/수치 검사,
실제 Sequencer 멤버 함수 CPU 대조를 확인했다. GPU/모델 경계를 대역으로 둔 CPU 확인은
Client UI 실행 PASS가 아니다. 이후 Camera row와 Q native sprite basis를 포함하여
2026-09-08 21:22 KST Debug Product 빌드·링크·배포를 통과했다. Alt+V/W/R/BA의 전체
복구 문서와 새 Resources 제품 활성은 아직 미완료이며 이번 실행 확인 대상은 Q 전체다.

Character Select는 기존 복구 재질을 재사용하는 5배치를 추가해 현재 9 material / 29
placementLighting 행을 연결하고 기존 publisher Validate/Publish/Check를 통과했다.
맵 803배치 전체가 복구된 상태는 아니다. 캐릭터 22개 재질 및 추가 UV 모델은 설치·활성화했고,
실제 설치 경로의 모델·재질 검사 40개와 공통 load description 검사 18개를 통과했다.
9개 모델·23개 material slot에 적용되며 새 EXE의 실제 외형 확인은 남아 있다.

누적 원리·반복 결함·실제 커버리지는 [렌더링이펙트복원V2.md](../렌더링이펙트복원V2.md)에
정리하고 AGENTS/gotchas에서 연결했다. Q 이후 모든 클래스 V/Alt+V와 맵으로 확대하되,
V/Alt+V 전체 연결과 세부 구성 row는 여전히 미완료다. 아래 앞부분의 17~19시 결과는
그 당시 checkpoint이며 이 최신 상태와 마지막 후속 절을 우선한다.

## 차원술사 Q와 V1 도구

- `Data/Effects/Authored/effect.dimensionmaster.skill.2050100.restore.effect.json`을
  `이펙트_차원술사Q`로 생성했다. 기존 큐브·베기 무늬 두 element와 원본 recipe를 보존한다.
- 기존 Q `.unified`, Artist F authored, EffectCatalog, DimensionMaster.animevents는
  변경하지 않았다. 제품 Q는 `pc_sp_m_00_sk_sk_nailstrike_01` 0ms에서 기존 문서를 호출한다.
- SourceIndex는 대응하는 정상 PLAYER_SKILL `.unified`가 있을 때만 Catalog에 없는
  `.restore`를 편집기 문서로 추가한다. 경로·중복·원본 연결 실패는 해당 항목에 표시한다.
  제품 등록 수와 편집기 복구본 수를 구분하며 Catalog를 변경하지 않는다.
- V1 All Effects의 Product 아래 Recovery Effect를 표시하고 Play/Open Editor와 기존
  element/family/Play All tree를 연결했다. 최초 목록 갱신은 기존 `Refresh`를 사용한다.
- 전체 Play는 기존 CharacterPreviewPanel과 skillbindings에서 해당 클래스·스킬의 실제
  모델·클립 순서를 선택하고 EffectAuthoringSequencer의 같은 시계로 재생한다.
  새 캐릭터 로더나 별도 Effect runtime을 만들지 않았다.
- Model View의 root/bone anchor, Sequencer 재생·Seek를 V1/V2 owner에 연결했다.
  원본 SourceRecipe socket에는 바깥 occurrence bone을 이중 적용하지 않는다.
  sequence v2는 anchor를 저장하고 v1은 root로 읽는다.
- 복구본 Save As는 새 ID·표시명·Parent를 받는다. 기존 파일 덮어쓰기를 거부하고
  저장 실패 시 원본·편집 상태를 유지한다. 기존 V2 작업은 보존한다.

### 큐브의 원본 계산

MIC `fx_m_mi_j_00.fx_mi.fx_j_me_cubesample_01_04_tr`의 unclamped 분기,
PS `f548f39885bbfe42ac405ccd46dc2919`와 VS 입력을 대조했다.
`Shader_EffectCubeSampleScene.hlsli`를 기존 mesh shader에서 profile42로 선택한다.
화면 좌표 굴절, 시선 가중, 화면색 5차곱, caustic, UV 모서리 식을 이식했다.
MIC의 spec_str=1, edge_line=30, edge_str=2, color=(4,3.8,3,1)과 native uniform
기본값을 전달한다. caustic은 기존 `fx_j_caustic_tile_02.dds`를 사용한다.
원본 재질 identity와 분기가 맞는 문서에만 이 경로를 허용한다.

Renderer/GameInstance는 요청 프레임에 NONLIGHT 다음·BLEND 이전 SceneHDR를
별도 `Target_EffectSceneColor`로 복사한다. 같은 자원 읽기/쓰기 충돌을 피하고 기존
MRT를 다시 Begin해서 화면을 지우지 않는다. 복사 후 출력과 깊이 바인딩을 복구한다.

공식 확보와 원본 엔진 전체 일치는 구분한다. SceneHDR 시점·색 인코딩, engine prefix,
fog·보조 MRT 전체를 원본과 동일하게 구현한 것은 아니다. SceneHDR/opacity 1/fog identity는
현재 엔진의 입력 어댑터다. caustic의 생략된 sRGB class default도 아직 확정하지 않았다.

Q 베기 무늬도 PS `12959b8a47f91c4dab8a19b9de0871ed`의 계산을
`Shader_EffectSliceSceneDepth.hlsli`와 기존 particle shader의 profile43으로 연결했다.
flow 텍스처의 R, 원본 UV 회전·signed blade·radial 식, DynamicParameter.xyz,
particle alpha와 깊이 차에 따른 fade를 사용한다. Target_Depth.y=W/1000을 현재 단위로
복원하고 원본 cm로 변환한다. 원본의 별도 distortion=50 pass와 보조 MRT 전체는 미복원이다.
큐브·베기 두 요소의 원본 식 이식과 원본 엔진 전체 출력 일치는 구분한다.
근거는 `out/DimensionMasterQRestore20260908/`에 보존한다.

## 캐릭터 재질

모코코 3MIC, 창술사 장·단창 1MIC, 차원술사 body 10MIC와 무기 3MIC의 원본 프로그램을
확보했다. 17MIC에 대응하는 Base/Directional/VS/Baked 프로그램은 28개다.
ActorCatalog → PlayableCharacterAssetService → CModel/CMaterial → animated/socket
equipment → 기존 deferred 조명 소비를 확장한 코드 초안을 보존했다. 추가 개발은 중단했다.

원본 S.RGBA, 마스크·색상 variation, normal, BRDF·2D IBL과 프로그램별 uniform을 전달한다.
현재 모델에 빠진 차원술사 눈 UV1/UV2와 머리카락 UV1은 원본 geometry를 대조하여 기존
reader/vertex 경로에 운반하도록 작성했다. UV0를 복제하여 원본 입력으로 표시하지 않는다.
Lance 5/DimensionMaster 17의 모델별 override 22행은
`out/CharacterMaterialRestore20260908/pending_character_material_overrides.json`에 보존했다.
CharacterCatalog는 이번 작업 전 바이트와 동일하게 유지한다. 새 재질은 현재 활성화하지 않으므로
Q 모델 미리보기에 새 UV 후보가 필수 입력으로 추가되지 않는다.
shader는 기존 6 MRT에 표면 UV·tangent와 geometry normal용 2 MRT를 추가하고,
실제로 그린 CMaterial만 기존 light pass로 추가 계산한다. 원본 직접광 결과에는 이미
표면색이 들어 있으므로 다시 곱하지 않으며, 주변광은 원본 RT3 표면색을 거쳐 합성한다.
원본 time uniform은 기존 Renderer presentation clock을 전달한다. 새 시계는 만들지 않았다.
Engine/Bin/ShaderFiles의 SourceCharacterMaterial/Programs HLSLI가 공유 정본이고,
Engine Shader_Deferred FXC와 기존 SDK/Client 셰이더 복사가 이를 소비한다.

추가 UV 후보는
`out/CharacterMaterialRestore20260908/geometry/staged/Character/DimensionMaster/DimensionMaster_Character.wmodel`
에 준비했다. 눈 slot7은 얼굴00 geometry와 AV05 MIC의 조합이며 UV1/UV2를 보존한다.
머리 slot9는 UV1을 보존한다. 기존 34,612 vertices, 143,292 indices, 225 bones,
154 animations와 material payload는 유지하고 추가 채널만 붙인다.
캐릭터 복구를 후순위로 미뤘으므로 Resources 모델은 아직 교체하지 않았다.
추가 UV reader/transport와 shader wrapper는 이번 Product 빌드에 함께 컴파일됐지만
새 데이터 활성화·native decoder 실제 입력·GPU 화면 검증은 하지 않은 checkpoint다.
기존 cooker 검사 9개(새 추가 UV 검사 2개 포함), 후보 Python readback과 기존 payload 바이트
보존 검사를 통과했다. C++ decoder의 실제 읽기·GPU 입력·화면은 검증하지 않았다.
남은 재질 경계에는 engine cube/SH, 원본 sorted translucency, nonuniform scale TBN,
특수 계열의 pass 입력과 shadow 정책의 최종 확인이 있다. 완성된 캐릭터 재질로 보고하지 않는다.

## Character Select → 쿠크 종료 오류

`Client/Private/Level_CharacterSelect.cpp` 소멸자에서 map light provider의 Clear를
제거하고 shared_ptr만 reset한다. Update가 이미 프레임 큐에 제출한 provider는
Presentation_Manager가 해당 프레임을 처리할 때까지 유효한 문서를 유지한다.

수정 전에는 같은 Update 끝의 전환으로 Level이 소멸하면서 문서를 비웠다.
다음 Submit_FrameProviders가 준비되지 않은 provider를 호출해 E_FAIL을 반환하고
Client가 Render 실패로 종료하는 수명 경로를 확인했다.
`Client/Default/ClientExit.user.log` 마지막 기록은
`2026-09-08 16:36:02 reason=Render failed hr=0x80004005 level=1`이다.
RendererExit.user.log에도 `stage=Submit_FrameProviders hr=0x80004005`가 반복된다.
후자는 시각이 없으므로 같은 시각의 재현 로그라고 단정하지 않는다.
실제 수정 후 재진입은 아직 확인하지 않았다.

## Resources와 Drive

후순위 캐릭터 작업에서 새로 설치한 공유 폴더는 **`Resources/Character/SourceMaterials/`**다.
29개, 84,149,596 bytes이며 기존 47개는 픽셀 대조 뒤 현재 경로를 재사용했다.
주로 누락된 variation·피부·얼굴·환경반사 입력이다. 새 TGA는 원본 top-mip RGBA를
보존하고 CMaterial의 기존 CPU mip 경로를 사용한다. 신규 29개는 모두 TGA다.
재사용 47개 중 35개 TGA는 CPU mip, 차원술사 무기 DDS 12개는 기존 mip 11단계다.
원본 sRGB=false는 linear로, class default 생략은 프로젝트 해석임을 구분했다.
Drive ZIP 제작·업로드는 하지 않았다.

Q는 기존 `Resources/Effect/DimensionMaster/`의 모델·텍스처 4개를 재사용한다.
Q 신규 Resource는 없다. 위 신규 29개도 Q의 필수 공유 대상이 아니다. UV 후보의 설치 대상은
`Resources/Character/DimensionMaster/DimensionMaster_Character.wmodel`이며 기존 파일 교체다.
현재 미설치이므로 Drive 공유 완료본에 포함하지 않는다. 새 reader를 포함한 빌드와 함께 설치한다.
`out/` 조사 자료와 이전 중앙 링·무기 mip 공유 대상을 이번 신규 폴더에 섞지 않는다.

## 검증 상태와 사용자 경로

실행한 확인:

- 현재 diff·호출자·native shader/재질 입력과 기존 종료 로그 확인.
- Q 원본·Artist F·Catalog·animevents 해시 보존, restore 2element와 sourceRecipe
  15/12개 모듈 보존, 기존 Q Resource 4개 존재 확인.
- SourceIndex의 실제 Q source/skill/path 입력 확인, 기존 lazy metadata 검사 9개 통과.
- V1/Q 큐브/소멸자/해당 project/filter에 한정한 git diff --check 통과.
- 캐릭터 텍스처 픽셀 대조와 새 파일 설치, 재사용 DDS mip header 확인.
- Q Slice uniform rotation 4행·swizzle 6사례·depth gap 6사례 CPU 수치 대조.
- Engine/Client project와 filters XML 4개, 변경 JSON 2개 parse 및 신규 파일 등록 6개 정합.
- 원본 18 PS의 sample bias 121개가 모두 0임을 확인. 정수/hex literal·조건 비트 변환을
  검토하여 오역을 교정했다. 이것은 FXC/GPU 결과 검증이 아니다.

추가로 실행한 확인:

- Q MeshPreview/Particle FXC 개별 컴파일 성공. 이어서 공식 Debug Product 빌드 성공(exit 0).
- 공통 SourceCharacterAppend에서 미선택 분기의 uint 인덱스 underflow를 명시적인
  float4 조합으로 교정했다. C++ parameter의 min/max 매크로 충돌과 Part_Body의
  ModelAssetData 정의 include 누락도 교정했다. 새 캐릭터 override는 활성화하지 않았다.
- Client의 PrepareEngineSdk를 FxExport/FxCompile/ClCompile 앞에 연결했다.
  기존에는 Client FXC가 이전 HLSLI 사본을 먼저 읽었으므로, 같은 정상 배포 경로에서
  최신 Engine 정본을 먼저 복사하도록 순서를 바로잡았다.
- 빌드 결과는 `out/BuildPipeline/runs/20260908T085820822Z-debug-product.json`,
  최종 로그는 `out/DimensionMasterQRestore20260908/product-build-debug-retry3.log`다.
  기존 FXC 경고·코드 페이지 경고·외부 라이브러리 PDB 경고는 남아 있다.
  Product 명령은 runtime 데이터 publisher와 광역 진단을 실행하지 않았다.
- 빌드 후 project/filter XML 4개와 Q restore JSON parse, 해당 변경의 git diff --check를
  다시 확인했다. 필수 runtime 입력 누락은 Product 결과에서 0개다.
- 이후 사용자가 Visual Studio로 추가 빌드·실행을 진행했다. 그 빌드 중에는 Engine DLL과
  Client 배포 사본의 교체 시각이 일시적으로 달랐고, 완료 후 DLL·Deferred CSO 일치를
  재확인했다. 18:01 Client/Server 프로세스 실행을 확인했으며 화면 조작은 하지 않았다.
  따라서 17:58 정본 Product 결과와 이후 사용자 빌드를 같은 실행으로 기록하지 않는다.

미실행: UI 입력·Play/Play All·Save/Load·Seek,
CS→쿠크 진입, 사용자 시각 판정과 프레임 비용 측정.
정적 확인을 기능 실행 PASS나 visual PASS로 표시하지 않는다.

빌드 후 사용자 확인 경로는 F1 → Effect Tool → V1 Effect → All Effects → Refresh →
DimensionMaster의 Q → Product 아래 Recovery Effect `이펙트_차원술사Q` → Play/Play All이다.
모델·Q 애니메이션·root로 먼저 재생하고 Model View에서 필요한 anchor를 선택한다.

커밋·stage·push·병합을 하지 않았으며 다른 작업 변경과 빌드 산출물을 정리하지 않았다.

## Q 전체 10개와 19:14 실행 확인 checkpoint

- 전체본: `Data/Effects/Authored/effect.dimensionmaster.skill.2050100.full.restore.effect.json`.
  원본 Q 활성 cue의 10개 element이며 앞 2개는 성공한 `.restore`와 전체 dictionary가 동일하다.
- 전용 추가 원본 프로그램 8개는 `Shader_EffectDimensionMasterQNative.hlsli`와
  `Effect_DimensionMasterQMaterial.h`의 typed profile44~51로 연결했다. 재질 texture 이름,
  sampler/color space, uniform/DynamicParameter, SceneDepth 입력을 기존 carrier에 전달한다.
- 원본 swing mesh는 UV가 한 벌이다. 추가 UV1 모델 교체를 요구하지 않는다. shader가
  두 번째 UV를 요구하는 분기에는 마지막 UV stream을 반복하는 UE 호환 VF adapter를 사용한다.
  이 경계를 원본 LostArk VF 전체의 독립 검증 완료로 기록하지 않는다.
- All Effects의 Product 아래 `.restore`와 `.full.restore`를 함께 표시한다.
  확인 경로: F1 → Effect Tool → V1 → All Effects → Refresh → 차원술사 Q → Product →
  Recovery Effect → `이펙트_차원술사Q_전체` → Play.
- 이번 전체 Q 단계에서 설치한 Resource는 정확히 1개다:
  `Effect/DimensionMaster/Textures/FX_MASTERMATERIAL/fx_e_normal.dds` (8,320 bytes).
  실제 위치는 `Client/Bin/Resources/` 아래 같은 경로이며 Drive 공유 대상이다.
  Q 모델 교체는 없다. Alt+V용 5 DDS와 cube 모델은 아직 설치하지 않았다.

검증 증거:

- 18:49 정본 Product PASS: `out/BuildPipeline/runs/20260908T094928606Z-debug-product.json`.
- Q10 정적 검사: `out/DimensionMasterFullRestore20260908/q10_static.json`.
- C2664 수정 후 단일 CPP 컴파일 PASS:
  `out/DimensionMasterFullRestore20260908/q-build-fix-playback-compile.log`.
- 사용자 19:14 Client 링크/배포 성공 로그 보존:
  `out/DimensionMasterFullRestore20260908/user-client-build-191435.log`.
- 후속 Q10 JSON parse 및 기존 2개 정확 보존 재확인. Playback CPP/H의
  `git diff --check` 통과. 공유 작업 전체 diff의 미정리 항목까지 PASS로 기록하지 않는다.
- Client를 에이전트가 실행하거나 조작하지 않았다. 전체 Q 화면 확인은 대기다.

## Q 화면 피드백 후 입력 교정과 All Effects 자동 목록

사용자 첨부 이미지 1·2는 모작, 3은 원본이다. 이미지에서 큰 노란 직사각형과 화면을
덮는 유리/발광 띠를 관찰했다. 사용자는 유리 표현이 나오는 것을 확인했으나 전체 Q의
원본 유사도는 승인하지 않았다. 아래 변경은 그 피드백에 따른 후속 소스 수정이다.

실제 반영:

- full Q의 swing/crack 메시 2개에 modelPreScale=0.01을 추가했다. native positionstream과
  WModel 좌표는 cm이며 기존 누락값 1 때문에 정확히 100배 커졌다. 기존 cube의 0.01은 유지했다.
- Q45·49 native sprite에 활성 Dynamic 모듈이 없을 때만 Playback이 (1,1,1,1)을 공급한다.
  Q45의 기존 0은 UV를 (.5,.5)에 고정하고 opacity 지수를 0으로 만들었다. Q49는 particle
  alpha/fade를 무시했다. 원본 VS→PS varying과 native CDO, Epic Null Dynamic stream을
  근거로 공급값을 교정했으며 LostArk 실행 중 해당 stream을 직접 관측한 검증은 아니다.
  명시적인 Dynamic 모듈/곡선 및 기존 HLSLI 수식은 유지한다.
- full Q에 복사됐던 시험 SourceScale을 원본 1로 돌렸다. cube burst는 4×5.66의 반올림값
  23개에서 원본 4개로, 크기 2.81·수명 1.93·속도 .57도 원본 배율로 복귀했다. slice의
  시험 배율도 교정했다. 활성 cue006/007의 Client 좌표 (x,z,-y)×.01, 원본 .27초 시각과
  cube의 .05초 emitter delay를 반영했다. 기존 성공 Q2 `.restore` 파일은 바뀌지 않았다.
- All Effects의 첫 펼침에서 Product/authoring metadata를 한 번 검색한다. 복구본이
  같은 프레임의 Product 목록에 연결되며 Open/Play 전 전체 문서·GPU 리소스를 로드하지
  않는다. 초기 실패 시 매 프레임 반복하지 않고 수동 Refresh로 재시도한다.
- 이번 교정은 Resources 추가·교체가 없다. 앞 checkpoint의 fx_e_normal.dds 공유 요구는
  그대로이며 새 Drive 전달 파일은 없다. V/Alt+V와 구성 row 확장은 계속 미완료/보류다.

검증:

- Effect_Playback.cpp, Effect_Tool.cpp 각각 Debug ClCompile 성공(exit 0). 로그는
  `out/DimensionMasterQRestore20260908/q10-correction-playback-compile.log`와
  `q10-correction-tool-compile.log`다. 기존 포함 헤더의 C4819 경고는 남아 있다.
  최초 다중 SelectedFiles 명령은 MSBuild 인수 파싱 오류로 컴파일 전에 실패하여 단일 파일
  명령으로 각각 확인했다. Product 링크·EXE 교체는 실행하지 않았다.
- Q10 JSON은 원본 snapshot 대비 허용한 26경로만 변경됐고 모든 source module·재질·색·
  Resource 참조는 보존됐다. 참조 46건/서로 다른 실제 파일 29개 존재 확인. 기존 색공간·
  attachment·native sprite 검증 통과. `out/DimensionMasterQRestore20260908/q10_applied_correction_checks.json`.
- 기존 lazy metadata 테스트 9개 통과. exact revision 테스트는 7개 중 6개 통과,
  MainApp의 첫 Effect 생성에서 Open_ValtanAllEffectsWorkspace 호출을 기대하는 1개 오류.
  수정 전 Effect_Tool snapshot으로 같은 오류를 재현했다. MainApp은 이번 교정에서 수정하지
  않았으며 이 기존 호출 계약 불일치를 Q/자동 목록 검증 PASS로 포함하지 않는다.
- 이번 CPP와 팀 안내 문서의 git diff --check 및 PLAN/RESULT/변경 JSON의 공백·parse 확인.
  공유 작업 전체의 diff 또는 다른 세션 기능을 검증 완료로 기록하지 않는다.

실행 상태: 에이전트가 Client/Server/UI를 실행하지 않았다. 현재 EXE는 사용자가 빌드한
19:14:35 버전이며 이번 변경의 링크가 아직 필요하다. 새 EXE에서 사용자가 확인할 경로는
F1 → Effect Tool → V1 → All Effects → DimensionMaster → Q → Product →
Recovery Effect → `이펙트_차원술사Q_전체` → Play/Play All이다. 첫 표시에서 Refresh가
필요 없어야 하며 크기·사각형·배치의 실제 개선, Play/Seek/Save 결과와 시각 유사도는
사용자 확인 전이다. 코드/데이터 교정을 전체 원작 복원 완료로 표시하지 않는다.


## G19 후속 결과: Q의 Play All·공간 입력

사용자는 All Effects의 Refresh 없는 첫 목록 표시는 성공했다고 확인했다. Play All은
오류 문구 없이 약 2초 멈춘 뒤 끝으로 가는 느낌이라고 답했다. 첨부 sprite/mesh Family
이미지를 열람했으며 sprite streak 분산과 mesh 파편의 위치 차이를 관찰했다. 이를 특정
검은 fragment의 발생 element나 최종 원본 일치 승인으로 해석하지 않았다.

### 실제 코드·Data 반영

- Sequencer Play는 Preview의 transient row도 유지·재생한다. 성공 시 input interaction을
  요청하고 Play/Preview/Resume/Refresh 직후의 첫 advance를 제외한다. 정상 dt는 유지한다.
  원래 첫 dt 2.5초가 2초 수명의 끝으로 즉시 이동시켰고, 수정 경로는 0ms를 유지한 뒤
  다음 .016초에서 16ms로 진행했다. Loop는 현재 anchor를 유지한다.
- full recovery Play All은 버릴 COMPLETE WorldPreview를 먼저 만드는 중복 준비를 제거했다.
  현재 문서의 visible element/modelCue 수명으로 전체 duration을 계산해 Sequencer에
  한 번 준비한다. 현재 Effect의 Play All 버튼도 같은 active unified 경로로 연결했다.
  ParticleSystem/Detail/ModelCue draft 모두 실제 occurrence snapshot에 전달한다.
- 정확한 Q47 무텍스처 material만 `Procedural Glow (no texture)`로 표시한다. 기존 unbound는
  파일명을 찾지 못한 라벨이었고 Q47은 원본 texture sample 0인 절차적 원형 빛이다.
- full Q는 native/cooked vertex 비교에서 (x,z,-y)와 .01 단위를 보존하고 snapshot root
  source basis -90도를 10행에 넣었다. 캐릭터 모델 admission에만 있던 전방 정렬을
  Effect root에도 적용한다. source notify007 (70,25,80)cm는 owner yaw0에서
  (.25,.8,.7)m가 된다. 거리 축소나 임의 mesh 회전으로 고친 것이 아니다.
- 155개 분포를 원본과 대조했고 45개 구조 차이 중 실제 행동에 영향이 있는 8개 분포를
  교정했다. Mesh/Sprite StartRotation 5개, crack RotationRate metadata 1개,
  LocationDirect ScaleFactor 1개, Cylinder VelocityScale 1개다. 명시 LUT/곡선·재질·색은
  그대로이며 CDO/archetype 상속으로 빠진 필드만 채웠다.
- Cylinder의 명시 radialvelocity=true를 원본 CDO에서 추가하고 기존 Playback에 소비를
  연결했다. 원본 높이축을 속도에서 제외한 뒤 Client 좌표 변환을 적용한다. 생략/false의
  기존 동작은 유지한다. [Epic Cylinder 설명](https://dev.epicgames.com/documentation/en-us/unreal-engine/location-modules?application_version=4.27)은 의미 교차 확인용이며 원작 CPU 동일성의 증거가 아니다.

적용 fullQ SHA: `fd282b7e507ee5f7ae7c509eba4d70f0a7269e31368757054a20360e173668e5`.
직전 Q10 대비 정확 40경로(8분포의 29필드 + basis10 + literal1)만 바뀌었다.
Q2 비교본 `2aa68b55bc8b35f2c4814ad7a8eda55aaceadde54d3aaffcb2160d86963c5795`는 보존됐다.
원본 burst 합계15·기존 timing/loop/수명/Resource 참조는 이 단계에서 바뀌지 않았다.

### 검증과 남은 경계

- `out/DimensionMasterQRestore20260908/round3/`의 EffectAuthoringSequencer.cpp,
  Effect_Tool_Workspace.cpp, Effect_Tool.cpp, Effect_Playback.cpp 개별 compile.log 모두
  exit0. 기존 EngineSDK 헤더 C4819 경고는 남아 있다.
- `sequencer_comparison.json`: 실제 8멤버 함수 추출 CPU 대조의 10개 조건 통과.
  이전 preview 삭제·큰 dt skip·owner 누락을 재현했고, 수정의 preview 보존·첫 delta 제외·
  active preview/saved row 실패 보존·Loop anchor 유지를 확인했다. 모델/GPU 경계는 대역이다.
- `q_kinematics_verification.json`: source/data/numeric 49개 확인. 허용40경로,
  기존 색 공간/attachment/native sprite validator, 46참조/고유 Resource29개 존재,
  Q2 보존. Cylinder 수치는 현재 C++ 분기를 읽은 Python X/Y/Z × true/false6사례이며
  해당 CPU 함수 전체를 원작과 실행 대조한 것은 아니다.
- 기존 관련 source-contract 테스트35개는 변경 전/후 모두5실패+1오류, 동일 traceback6건.
  새 실패0이며 해당 기존 MainApp/refactor 계약 불일치를 수정 완료나 전체 테스트 PASS로
  표시하지 않는다. `existing_contract_tests_comparison.json`에 별도 기록했다.
- source axis 근거와 설치 mesh 세 개의 native vertex+UV 대조, 현재 회전/크기 행렬의
  대수 정합은 `q10_mesh_axis_audit.md/.json`과 `q10_mesh_axis_algebra.py`에 있다.

원본 난수열, bAdjustForWorldSpace의 CPU 식, bInheritParent, 일부 생략 Required duration과
원작 부가 MRT/화면 합성은 이번 교정으로 완료되지 않았다. 실제 UI Play All/Family 전환,
Seek/Loop/Save/Load와 새 화면의 위치·수량·색 결과는 사용자 확인 전이다.
원본 CDO 병합은 현재 Q JSON에 적용했으며 공통 importer SourceIndex의 자동 상속 병합은
아직 미구현이다. 다음 자동 변환이 저절로 수정됐다고 보고하지 않는다.

## G20 후속 결과: 누적 노트와 Character Select 동일 재질 확장

`.md/GB/렌더링이펙트복원V2.md`를 만들고 AGENTS의 작업 시작 문서와 gotchas의 관련
문서로 연결했다. 원본 재질·ABI·class-default·축·mip·조명·시계·Resources의 반복 결함과
현재 연결/미완료를 누적 관리한다. 이 문서의 V2는 새 Effect Tool/런타임을 뜻하지 않는다.

Character Select의 추가 5배치는 BRIDGE 442/444/458/474와 FLOOR13H 507이다.
원본 mesh·ordered MIC·atlas texture pair·환경 render input이 기존411/439와 동일해
기존 variant를 재사용하고 원본 배치별 atlas scale/bias와 decode 값을 연결했다.
기존9 material·24 lighting행을 보존하며 source/runtime 모두9/29행,803배치다.
전체 Transform·visibility·stable placement ID와 catalog/maplights는 보존됐다.

변경 파일은 해당 CS authoring의 mapmaterials.json/mapplacements와 runtime 동명2개다.
기존 `Publish-MapAuthoring.ps1`의 Validate/Publish/Check 성공과 네 파일 diff --check를
확인했다. 새 C++/shader/Resources는 없으며 참조한 기존15파일 bytes/hash를 보존했다.
근거: `out/DimensionMasterQRestore20260908/round3/cs5/connection.json`, before snapshot,
publisher3로그 및 `cs5_result_and_next.md`. 새5배치의 실제 화면은 사용자 확인 전이다.

이번 Q·CS5 교정의 신규/교체 Resources는 0이다. 이전 full Q의
`Effect/DimensionMaster/Textures/FX_MASTERMATERIAL/fx_e_normal.dds` 공유 경계는 유지한다.
캐릭터 Resource 설치는 별도 활성 결과에서 정확 경로·추가/교체를 기록한다.

이 단계에서 stage/commit/push/PR/merge나 Client/Server/UI 실행을 하지 않았다.
공유 dirty worktree 전체와 다른 세션의 검증 상태를 이번 결과에 포함하지 않는다.

## G20 캐릭터 활성 후속 결과

CharacterCatalog의 LANCE_MASTER 5행과 DIMENSIONMASTER 17행을 활성화했다.
9개 모델·23개 material slot의 원본 입력을 기존 CModel/CMaterial에 전달한다.
모코코 3재질, 창술사 장·단창 2재질, 차원술사 몸 10행·무기 7행이며 새 모델 런타임은 없다.

`Resources/Character/DimensionMaster/DimensionMaster_Character.wmodel`을
74,641,016→74,781,640 bytes로 교체했다. eye UV1/2와 hair UV1을 추가하고 기존 형상,
vertex/index·bone·bounds와 156개 비메시 section을 보존했다. 새 텍스처 설치는 0이며
이전에 설치한 Character/SourceMaterials를 소비한다. 이 WModel은 새 추가 UV reader가
포함된 EXE와 함께 Drive 공유해야 하며 ZIP/업로드는 하지 않았다.

기존 playable Model View도 EnsurePrototypes→CCharacter 경로에서 원본 재질을 소비한다.
bare CModel 경로라고 보았던 초기 가설은 실제 호출자로 기각했다. generic/V2 preview fallback은
새 공통 Build_ModelLoadDescription을 통해 같은 catalog 모델 소유자와 override를 사용한다.
불명확한 소유자·잘못된 경로는 기존 출력 desc를 보존하며 실패하고 파일명 추측을 하지 않는다.

- 실제 설치 경로의 C++/WARP 모델 생성·clone·재질 입력 검사 40/40, 추가 UV 140,576 bytes 일치.
- 공통 catalog helper의 실제 CPU 검사 18/18. 관련 preview/asset service 최소 C++ 컴파일 통과.
- 근거는 `out/CharacterMaterialRestore20260908/activation_installed.json`과
  `activation_readiness_current.md`. 모델·재질 입력 확인이며 실제 draw/육안 유사도 PASS가 아니다.
- 통합 Product 링크 및 사용자 외형 확인은 미완료다.

## G21/G22 실행 준비 중 상태

차원술사 전체 스킬의 원본 active 구성 대조와 Alt+V native shader 연결을 진행 중이다.
Alt+V Camera row의 저장/샘플/복귀 코드와 원본 preset을 추가했으며 최종 컴파일은 미완료다.
원본 SkillCam_DimensionMaster_01의 20~3700ms 활성 창, owner-local 위치·방향·up,
수평 FOV를 사용한다. preset은 유효 창 안의 448개 키를 저장한다.
카메라 원본 곡선 대조의 수치 검사는 통과했지만 전체 Client 재생을 입증하지 않는다.

BA4는 원본 clip·입력 창과 원본 발생 구성을 확인했다. 현재 Product Data는 아직 3단계이며
4개 source restore의 실제 material 연결이 완료되기 전에는 Catalog/animation만 부분 교체하지
않는다. Effect Sequencer의 단계별 MODEL_SEQUENCE와 선택 API는 추가했으며 최소 컴파일 전이다.

사용자가 Q 원본 이미지를 다시 제공해 Q 검토와 실행 준비를 우선하고 있다. 이 시점에는
새로운 Product EXE를 실행 가능하다고 보고하지 않는다. 다른 작업자가 빌드한 이전 EXE의
존재는 이 작업의 최신 camera/BA 변경이 링크됐다는 증거가 아니다.

## G23 Q 원본 재검토와 실행용 빌드 진행

Q47의 texture sample 0개와 중심 거리 기반 두 감쇠 항을 다시 확인했다. UV 공간의 원형 빛은
맞지만 원본 200×600cm, PSA_Velocity, pivotY=0.9, 수명·크기 곡선이 최종 모양을 결정한다.
원본 이미지 중앙의 모든 장식·검격이 이 한 sprite라는 판정은 하지 않는다.

Q43/45/46/47/48/51의 명시 velocity sprite를 원본 VS basis와 대조하여 renderer를 교정했다.
기존 camera 평면 X 정렬 대신 실제 3D motion이 local Y를 소유한다. native U/V와
ParticleRect의 UV sign을 대조해 row0=cross(toCamera,motion), row1=motion을 사용하고
기존 pivot/크기·나머지 profile을 유지했다. Q49 rectangle은 이 분기에 들어가지 않는다.

- 실제 Make_ParticleSpriteWorld와 Resolve_ParticleSpriteScale, DirectXMath CPU 검사
  197개/실패0. native UV 네 모서리 최대 오차 9.83025e-7m. 다양한 view/motion·roll,
  legacy 불변, zero/parallel fallback, nonfinite 실패와 출력 보존을 확인했다.
- 근거: `out/DimensionMasterQRestore20260908/round3/q_velocity_actual_cpu.result.json`,
  `q_velocity_actual_cpu.receipt.json`, `q_velocity_basis_review.json`.
- Q 실행 제어 8개 실제 멤버 함수를 현재 소스에서 다시 추출·컴파일했다. 첫 2.5s delta 제외,
  다음 16ms 진행, transient 재생·Loop anchor·실패 시 기존 활성 row 보존을 재확인했다.
  모델·GPU·camera owner는 대역이며 실제 UI 테스트가 아니다. `q_execution_sequencer_result.json`.
- Q2/Q10 JSON은 기존 SHA를 유지하며 이 교정에서 Resources 추가·교체는 없다.

사용자의 최신 우선순위는 Q·W·R이며 camera/Alt+V 연출과 BA 제품 활성은 뒤로 미뤘다.
W/R 후보의 원본 구성·module·크기 대조는 out에서 진행하고 제품 Data/Resources는 아직
교체하지 않았다. Q 실행용 정상 Product 빌드는 `round3/product-build.log`에 진행 중이며
완료·배포 결과가 나오기 전에는 실행 준비 완료로 보고하지 않는다.

## 21:22 KST Q 실행용 Product 빌드 완료

정상 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`
실행이 exit0으로 끝났다. Engine/Shared/Server/Client 컴파일·링크와 Engine shader/runtime
DLL 정상 배포를 완료했다. build receipt의 missingRuntimeInputs는 빈 배열이다.
이 검사는 모든 게임 Resource의 시각적 완전성이나 실행 admission PASS를 의미하지 않는다.

- `Client/Bin/Debug/Client.exe`: 2026-09-08 21:22:41 KST, 47,881,728 bytes.
- `Shader_VtxEffectMeshPreview.cso`: 21:17:28, `Shader_VtxEffectParticle.cso`: 21:21:59.
- `out/BuildPipeline/runs/20260908T122241983Z-debug-product.json`: 제품 네 프로젝트 PASS,
  전체 658,729ms. Client 608,468ms에는 두 큰 Effect shader 재컴파일이 포함된다.
- 변경 JSON6개와 project/filter XML2개 parse, Q2/Q10 기존 SHA 보존 확인.
- Q velocity actual CPU 197개와 현재 Sequencer 재생 제어 대조 통과.
- 관련 root 변경 파일의 `git diff --check` 통과. Renderer에는 이전 변경부터 존재한
  tab-only 줄 1개가 남아 있어 공유 전체 diff가 깨끗하다고 보고하지 않는다.
- 기존 shader X4000, 인코딩 C4819/C4828 및 third-party PDB LNK4099 경고는 남는다.

마지막 확인 시 Client/Server는 둘 다 꺼져 있었다. 에이전트가 실행·조작·캡처하지 않았다.
사용자는 Framework.slnLaunch의 Server + Client로 실행해 Character Select에 진입한 뒤
F1 → Effect Tool → All Effects → 차원술사 Q → Recovery Effect →
`이펙트_차원술사Q_전체`(10 elements) → Play All을 확인한다.
확인할 항목은 시작 구간이 건너뛰지 않는지, 중앙 검격과 빛의 위치·방향이 모이는지,
유리 파편의 크기·분포가 맞는지다. 최종 시각 결과는 아직 사용자 확인 전이다.

W95/R33의 후보는 `out/DimensionMasterWRRestore20260908/source_handoff.md`에 보존했다.
이전 Converted에서 달라진 W13개 발생의 ordered module을 현재 원본으로 교정했고,
8개 원본 메시의 P/UV 비교에 근거해 22 mesh element의 .01 단위를 설정했다.
shader의 실제 입력 연결은 다음 단위이며 현재 제품 W/R이 전체 복구됐다고 보고하지 않는다.
BA4 후보와 Alt+V 카메라 후속도 보존한 채 사용자의 Q 육안 관찰을 기다린다.

## Resources 목록 재확인: 쿠크 모델 설치 상태 정정

사용자의 누적 공유 목록 요청으로 현재 Resources의 쿠크 FLOOR08/FLOOR08A WModel을
`out/FloorMaterialRestore20260907/source_geometry_recovery.json`과 직접 대조했다.
두 파일 모두 beforeSha256과 일치하고 afterSha256과 다르다. 현재 재질 코드·Data 연결과
두 모델의 tangent/채널 교체본 설치는 별개이며, 현재 폴더를 복구 모델 공유 준비 완료로
안내하지 않는다. 누적 노트의 공유 표를 정정했다. 파일이 언제 교체됐는지는 판정하지 않았다.
이번 확인에서는 Resources 교체·빌드·Client 조작·ZIP·Drive 업로드를 하지 않았다.

## 사용자 화면 피드백과 Q/W 손튜닝 방향 검토

사용자는 현재 맵 표시가 깔끔하고 만족스럽다고 명시했다. 확인하지 않은 전체 803배치까지
승인한 것으로 확대하지 않는다. 새 Q 화면 두 장(40bae845, 5356f16f)과 성공했던 Q2 화면
한 장(196df995)을 직접 열람했다. 새 화면은 가로로 늘어난 밝은 선과 검은 띠/조각이 두드러지고,
Q2는 캐릭터 앞의 큰 사각 유리 덩어리가 중심이다. 이미지 한 장만으로 검은 픽셀의 특정
발생 element를 확정하지 않는다. 사용자는 원작 일치보다 성공한 핵심 표현의 손조립을 원한다.

실제 Q2 JSON의 cube는 count5.66/size2.81/life1.93/speed0.57 배율을 이미 사용한다.
Q10의 같은 항목은 모두1이다. 따라서 원본 수치 복귀와 사용자 선호 연출의 유지가 서로
다른 목표임을 확인했다. Q2/Q10을 덮어쓰지 않고 V1 + Sequencer에서 별도 Q/W 튜닝 문서를
만드는 방향이 적합하다. 이번 검토에서는 새 Effect 문서·코드·Resources를 만들지 않았다.

W 검토 조합은 기존 세로 검격 + Q42 사각 유리의 5배 크기, 위→아래 발생 후 방사형 확산이다.
Q42 몸체 색은 source material color가 소유하고 일반 particleColor는 주로 edge 항에 쓰인다.
Q43은 Voronoi Slice sprite, Q44는 swing mesh + BlackLineAura다. dustparticle tile01(Q48),
tile02(Q51)는 다른 재질의 입력이므로 그 이름만으로 초승달 검격이라고 판정하지 않는다.
초승달 윤곽에는 곡선 mesh/마스크와 UV가 필요하다. 기존 입력으로 첫 W 구성을 시작할 수 있다.

V1에는 Count/Size/Life/Speed 배율, Element Transform, 지연·색 조정이 있다. source recipe의
위치·속도 분포는 원본 모듈이 소유하며 일반 Particle Detail 일부는 읽기 전용이다. 자유로운
방사형/낙하 저작은 기존 재질과 source carrier 계약을 보존하면서 실제 소비되는 발생 설정을
연결해야 한다. 일반 Detail 값을 바꾸면 즉시 반영된다고 설명하거나 SourceRecipe를 끄는
방법으로 유리 재질 계약을 깨지 않는다. W/R 전체 native 후보 생성은 이번 검토에서 확대하지 않았다.

## 다음 작업 인계: 모코코와 차원술사 의상 밝기 차이

사용자는 다른 작업에서 계속 진행할 예정이며 현재 모코코가 차원술사 의상보다 밝고 생생하게
보인다고 보고했다. 현재 Catalog/실제 shader를 읽어 다음을 확인했다. 코드·Resources·EXE는
변경하지 않았고 별도 작업 생성/메시지 전송도 하지 않았다.

- `Character/LanceMaster/LanceMaster.wmodel`은 기본 몸·애니메이션 소유자다. 모코코는 별도
  `LanceMaster_Helmet_Mokoko.wmodel`/`LanceMaster_Upper_Mokoko.wmodel`의 avatar 부품2개다.
  현재 보이는 모코코를 사용상 본체라고 부를 수 있으나 Resource와 재질 소유자는 구분한다.
- 모코코 source override3개는 program1 classic-skin이다. 기본 LanceMaster.wmodel에는
  이번 source override가 없다. 이전 body LUT3경로 수정은 모코코 재질 복구와 별개다.
- 모코코가 추가로 소비한 SourceMaterials에는 원본 normal3개, dress mask와 공용 환경/상태
  입력이 있다. 전체29개가 전부 모코코용은 아니다. 모코코 두 WModel은 이번 활성에서 교체하지 않았다.
- 모코코는 material-owned `hdr07_1.tga`의 2D 반사와 ibl_exposer5/ibl_intensity1을 소비한다.
  차원술사 의상5재질은 program3 realPBR이며 원본 cube sample이 현재 shader6365행에서0이다.
  무기 program8/9도 동일한 engine cube/SH 미연결 경계를 갖는다. base/직접광 식 확보와
  장면 간접광·환경반사 연결 완료는 별개다.
- 대표 모코코/차원술사 의상의 normaltex_intensity는 둘 다1이며 normal sample과 계산은
  연결돼 있다. 동일 강도는 재질식 전체 동일성이나 normal 문제의 완전 배제를 뜻하지 않는다.
  원단의 고유색·마스크·roughness/metal과 미연결 장면 입력의 기여량은 아직 화면 측정 전이다.

다음 캐릭터 작업은 모코코와 같은 밝기를 강제하는 대신, 동일 조명에서 차원술사 의상 대표
한 슬롯의 basecolor/normal/직접광/주변광/반사를 구분하고 빠진 scene cube/SH 연결을 우선한다.
원본 장면 입력이 미확정이면 명시적인 PROJECT_AUTHORED 환경으로 비교하고 원본 복원으로
표시하지 않는다. CS material 전용 cube를 모든 캐릭터에 하드코딩하지 않는다. Q/W 손튜닝
방향은 직전 절 그대로이며 현재 실행 중인 성공 Q2/맵 및 실행 파일을 보존한다.

## BA·Q·W·E·R·A 두 복구본 반영 및 사용자 빌드 인계

최종 범위는 BA0~3와 Q/W/E/R/A다. S/D/T/F/V/Alt+V 신규 후보는 out의 deferred에
보관했고 이번 제품 목록에 추가하지 않았다. 맵은 이 작업에서 변경하지 않았다.
full은 원본 전체 구성의 비교본이고 튜닝은 사용자가 지정한 연출이다. A는 최종 첨부의
보라 초승달이며 이전 초록/직선 해석을 폐기했다. E는 마지막 지시에 따라 튜닝6개를 유지한다.

| 튜닝본 | 최종 구현 |
|---|---|
| BA0~3 | 보라 찌르기·초승달. 단계별2/1/1/2 element |
| Q | 성공Q2 두 element 보존 + 사각형 중심 방사형 유리1 emitter. 파편16개, 정상 단위의 size5 |
| W | 검은 세로 검격1 + 위→아래 순차 발생 유리1 emitter. 유리5개·size5·수직 목표속도 -.8m/s |
| E | 시작과 세 공격 구간을 핵심6개로 구성. 원본 전체 비교는 별도 full67 사용 |
| R | .3/.7/1.05초에 보라 검격1 + 보라 파편3개씩. 검격3개와 공통 파편 emitter1개, 총4 element |
| A | 실제 swing02 곡선 mesh로 짙은 보라 외곽과 백보라 안쪽 호. 2 emitter, 각각4회 발생 |

Q/W/E/R/A 튜닝은 `.tuning.restore`이고 기존 Product `.unified`를 덮어쓰지 않는다.
All Effects의 Recovery Effect에서 `_튜닝`/`_전체`를 선택해 Play All로 비교한다.
BA는 실제 `_01/_02/_03/_04` manual4단으로 PlayerSkills, skillbindings, root motion,
animevents와 Catalog를 연결했다. 동작 길이는1400/1500/1067/1700ms다. 기존 BA3개
unified는 내용 보존 후 기존 audition registry에 실제 old1→new3, old2→new0, old3→new2
소유권으로 분류했다. gameplay 데이터를 publish했으므로 실행 중 Server가 있으면 재시작이 필요하다.

### 전체본의 실제 연결 범위

| full | 원본 element | 실행 활성 | 보류 |
|---|---:|---:|---:|
| BA0~3 합계 | 75 | 62 | 13 |
| Q | 10 | 10 | 0 |
| W | 95 | 84 | 11 |
| E | 67 | 22 | 45 |
| R | 33 | 30 | 3 |
| A | 117 | 62 | 55 |

이 표는 JSON의 실행 활성 설정이며 화면 PASS 또는 모든 원본 shader의 동일성 수치가 아니다.
BA full에는 기존 texture/표준 재질 비교행도 포함된다. 미지원 Orbit link/options, 일부 빛과
미연결 재질의 원본 행은 삭제·위조하지 않고 실행 보류로 보존했다.
E 잔여13 MIC/12 PS,45발생은 out 준비까지이며 사용자의 우선 화면 확인 지시에 따라
제품에 추가하지 않았다. A도 남은55발생을 전체 복원 완료라고 표시하지 않는다.

WR 신규48 descriptor는 실제 중복5쌍을 공유한43 PS 계산을 사용한다. EA4 alias는 기존
Q/ALT/WR 계산을 재사용한다. 새 상수 배열은 없고 기존 V float4[32] 중 최대15행을 사용한다.
원본 named numeric869개, static switch430개와 texture95종을 대조했다. fog-neutral,
opacity1, scene-depth cm 변환 등 기존 장면 adapter의 한계는 원본 엔진 완전 동일성과 구분한다.

초기 A blackline4행의 UV1 보류 진단은 정정했다. swing02 원본이 UV0 한 채널인 것은 맞지만,
현재 mesh VS에 마지막 source UV 전달 adapter가 이미 있고 물리UV1 clip은 없다.
현 소비자를 확인하여 기존 Q44 alias4행을 활성화했다. 가짜 mesh 채널이나 새 Resource는 넣지 않았다.
최종 점검에서는 WR mesh6/sprite8 프로그램의 실제 tangent-view 계산 조건 누락을 발견해
기존 두 VS 조건에 정확14 ID만 추가했다. 새 식이나 새 프로그램을 더 만든 변경은 아니다.

### 검증과 빌드 상태

- 현재19문서(전체9, 튜닝9, 기존Q2 비교1),422 element의 실제 C++ codec
  Load→Save_Atomic→Load→serialize equality 통과. 정확한 Product/Recovery 소유권 연결도19개 통과.
- 관련 Python72검사 통과. JSON/XML parse와 프로젝트 등록, 참조 Resource192개 존재 확인.
  Q2/Q10 SHA는 각각2aa68b55…/fd282b7e…로 보존했다.
- W/R/A 실제 분포·출생 검사 통과: W5개, R9개, A8개 총22 particle. W 위치·하강 목표속도 오차0.
  W/E 압축의 출생 위치 오차 최대2.38419e-7m를 확인했고 Q51 검격은 emitter UV 시계 때문에
  서로 다른 타격을 한 emitter로 무리하게 합치지 않았다.
- Gameplay publisher Publish 통과. BA 전용 hit shape가 없는 기존 maximumRange fallback 경계는 유지한다.
- 첫 Debug Product의 Engine/Shared/Server/Client 컴파일·링크·정상 배포 성공:
  `out/BuildPipeline/runs/20260908T141848139Z-debug-product.json`.
- 이후 두 shader의 tangent-view 조건을 수정했다. 조건과 원본/C++ descriptor14/14 대조 및
  두 shader diff-check는 통과했다. 사용자가 직접 빌드하겠다고 요청하여 두 번째 증분 빌드는
  에이전트가 소유한 process tree만 확인 후 중지했다. **마지막 두 shader 수정 후 빌드는 미완료이며
  사용자가 Debug x64로 빌드할 상태다.** 첫 빌드 성공을 최신 두 수정의 컴파일 PASS로 승계하지 않는다.
- 변경 범위의 diff-check 통과. 공유 Renderer의 기존20290행 tab-only 공백1개는 보존하여
  저장소 전체 diff-check가 깨끗하다고 보고하지 않는다. 관련 없는 대규모 dirty 변경을 stage/commit하지 않았다.
- Client/Server/UI를 실행·조작·캡처하지 않았다. 사용자의 최종 외형과 조작감 확인은 아직이다.

검증 로그는 `out/DimensionMasterHandTuning20260908/`의 `final_inventory.json`,
`codec_check/result_final.txt`, `final_wra/numeric_verification.txt`, `python_tests.log`,
`gameplay_publish.log`, `product_build.log`, `product_build_final.log`에 있다.
마지막 로그의 중지는 사용자 빌드 인계이며 컴파일 오류 판정이 아니다.

### Resources와 확인 경로

이번 추가 Resource는 `Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_fragment_001_cl.dds`
1개(128² DXT1,8,320 bytes)다. 현재 `Client/Bin/Resources`에 정확 원본을 설치했다.
Drive 공유는 아직 하지 않았고 Git에 추가하지 않는다. 다른 참조 texture와 mesh는 기존 파일을 재사용했다.

사용자는 Visual Studio에서 Debug/x64로 빌드한 뒤 `Server + Client` profile로 Ctrl+F5를 실행한다.
Character Select → 차원술사 → F1 → Effect Tool → All Effects → 해당 스킬 → Recovery Effect에서
`이펙트_차원술사Q_튜닝` 등 `_튜닝`/`_전체`를 선택하고 Play All로 비교한다.
BA는 실제 LMB의 추가 클릭/hold4단도 확인한다. 이 시점에 새 스킬 범위를 더 늘리지 않는다.

## G27. F·V·Alt V 두 복구본과 카메라 반영, SceneCapture 원본 조사

사용자의 후속 요청으로 F/V/Alt V를 추가했으며9월9일 자정까지 같은 작업을 이어갔다.
이전 BA0~3/Q/W/E/R/A19문서의 bytes는 전부 보존했다. 새6문서는 기존 Recovery Effect의
`_전체`/`_튜닝`으로 연결하며 실제 gameplay의 F/V/Alt V Product effect 매핑은 유지한다.

### 실제 파일 구성

| 스킬 | 전체본 원본 구성 | 현재 활성/보류 | 손튜닝 |
|---|---:|---:|---|
| F2050230 |69 element|34/35|2 emitter, .7/1.4초 중앙 하늘색 큐브2개+주변 파편6개|
| V2050520 |42 element|42/0|2 emitter, .6909초 중앙 큐브1개+하늘색 확산 파편8개|
| Alt V2050540 |344 element+model cue1|322/22|2 emitter, .7초 보라 큐브1개→2~3.25초 하늘색 큐브6개→3.95초 파편4개|

활성은 현재 문서의 실행 설정이며 visual PASS나 원본 엔진의 모든 기여 복원 비율이 아니다.
F의 보류에는 미연결 재질, Orbit, LocationEmitter와 disabled CircleSurface를 현재 portable
검사가 거부하는 행이 있다. 원본 모듈을 삭제해 지원된 것처럼 만들지 않았다. Alt V22보류는
light6, material4(167×1/178×2/181×1), mesh material 선택5, defaultparticle2, Orbit1,
eventreceiver4다. 별도 PostProcessChain/조명/HidePawn/재질 애니메이션/음향/Shake 등 원본
notify는344분모 밖이며 이번 원본 전체 지원으로 표시하지 않는다.

새 effect 파일은 `Data/Effects/Authored/effect.dimensionmaster.skill.<ID>.full.restore.effect.json`
및 같은 stem의 `.tuning.restore.effect.json`이다. 표시명은 각각 `이펙트_차원술사F_전체`,
`이펙트_차원술사F_튜닝`과 V/AltV 대응 이름이다. 기존 SourceIndex의 정확한 `.unified` sibling
소유권을 소비하며 새 Product Catalog 항목이나 두 번째 런타임은 추가하지 않았다.

손튜닝3문서는 총6 emitter/14 burst group/28 particle이다. 중앙·순차 큐브는 성공 Q2의
size2.81, 파편은 정상 단위 위의 size5다. body material 하늘색=[.6,2.4,4,1],
보라=[2,.65,4,1]이며 source particle RGB는 중립1이다. 출생마다 root를 저장하는 기존
world-space particle 경로를 사용해 앞서 생성한 큐브가 이후 캐릭터 이동을 따라가지 않는다.
F/V/Alt 실제 파편 종료는 각각2.15/1.75/4.8초 이내이며 현재 clip2.3333/2.5333/5.1초에 대응한다.

### 재질 재사용과 단위 교정

F7 MIC는 기존 선택 PS/VS/VF 및 입력을 대조한 alias다. ALT113/163/104, WR210/232/259,
V54를 재사용한다. 새 PS 함수/HLSL 수정은0이며70 numeric+15 texture 입력을 연결했다.
4개 header(`Effect_DimensionMasterALTVMaterial.h`, `Effect_DimensionMasterWRMaterial.h`,
`Effect_DimensionMasterVMaterial.h`, `Effect_MaterialTemplate.h`)만 필요한 배열과 조회를 확장했다.
VS pivot 차이는 현재 carrier의 원본 .5/.5 소비를 대조했으며 서로 다른 수식을 이름만 보고
alias로 만들지 않았다. F vector field는 원본 module `fx_o_w_01.fx_o_vectorfield_02`의
기존 exact mapping `Effect/DimensionMaster/VectorFields/fx_o_w_01.b7a4be9b5da97572.wvectorfield`
를 연결했다. 새 vector field 파일은 없다.

최종 점검에서 V5 mesh의 modelPreScale 생략을 수정했다. 실제 WModel geometry bounds는
helix 약±50, sphere±100, crack 폭90/100의 cm 단위였고 source StartSize는 dimensionless다.
기존 CModel→Mesh pretransform에서 한 번 적용하는 `.01`을5행에 명시했다. 원본 StartSize,
모듈, texture, 시점과 geometry bytes는 보존했다. 기본1을 둬100배로 커지는 경로를 막았으며
손튜닝 size2.81/5와 별개다. 최종 V SHA256은
`1180652442d40a5e32d4c900cec1ffae50d9201d83cc523096662d554f86931e`다.

### Alt V 카메라와 큐브

기존 `EffectAuthoringSequencer`의 동일 model/effect 시계와 camera owner를 재사용한다.
full preset은 원본448키와 카메라 구간[20,3700)ms를 보존한다. 새 tuning preset은8키,
[0,4900)ms이며 .7초 보라 큐브를 비춘 뒤2~3.25초 하늘색 순차 큐브 때 뒤로 빠진다.
새 preset effect duration은 실제 clip에 맞춰5100ms다. 원본 full의 effect tail22001ms는
카메라 종료와 별개이며 원본 긴 particle 수명을 자르지 않는다.

새 파일은 `Data/Effects/Sequences/effect.dimensionmaster.skill.2050540.tuning.restore.effectsequence.json`이다.
기존 full preset과 함께 MODEL_ROOT, 정확 skill.2050540, root anchor/zero offset과 단일 effect
identity를 검사했다. Play는 preset을 읽고 Seek/Pause/Loop는 기존 clock을 사용한다.
camera 구간 밖과 Stop/도구 비활성은 기존 typed override를 해제한다. Camera row의 `M`으로
카메라만 mute해 비교할 수 있다. 이번 camera C++ 추가 변경은 없으며 실제 시점·복귀 외형은
사용자 확인 전이다. Source view/up/FOV와 튜닝값을 혼합하지 않았다.

Alt V 원본 animated cube는 기존 변환 결과를 실제 Resources에 설치했다. 원본51 joint,
weighted bone25, bind 최대오차1.073e-6 및5시각 pose 최대오차6.454e-8을 CPU로 확인했다.
344문서의 model cue1과27개 child occurrence는 기존 CModel/bone anchor 경로를 사용한다.
현재 model cue는 기본 diffuse 재질이며 원본 override까지 복구한 것은 아니다.

### 사용자 첨부의 큐브 안 화면: 원본 존재 확인, 이번 연결은 보류

첨부 `codex-clipboard-5cbcb6be-1710-4474-a666-e86111c5be64.png`를 직접 열람했다.
캐릭터 상체를 비추는 현재 시점과 별개로 큐브 면에는 맵 바닥과 캐릭터가 담겨 있다.
원본에는 enabled `CEFActionNotify_SceneCapture`, notify001,0~.1초가 실제로 있다.
초기 camera notify013의 `par_m_swp_tw_s1_camera_01` emitter18/31은 camera 앞.5m의
`fm_h_box_01_1`과 ALT178 `fx_m_me_swp_box_01`을 요구한다.2초 animated cube도 같은
override를 요구하지만 별도 발생이다.

ALT178은 `screencapture`를 world position+engine CB0[0..5]의 투영 좌표로 읽으며
usecamuv/use_meshtype=true, capture_centeruvtile1.15다. 실제 Parent lighting model은
**mlm_unlit, blend_masked**다. 기존 out의 PS pass 이름에 근거한 lit/non-unlit 설명을 교정한다.
캡처 view와6개 상수의 정확 binding은 미확정이며 capture RT 보관·해제, model cue override
소비가 현재 코드에 없다. Q42의 현재 frame HDR pixel-position 굴절과 동등하지 않다.

사용자가 카메라 병목이면 제외하도록 허용한 범위에 따라, 이번에는 기존 카메라와6문서까지
닫고 SceneCapture 연출은 구현하지 않았다. 추가 복원은 원본 capture view/행렬→RT 수명과
투영 입력→초기 masked/unlit mesh2개→animated model cue override 순서다. 새 별도 모델
런타임을 만들 필요는 없지만 현재 데이터만으로 켤 수 있는 옵션도 아니다.

### 검증과 빌드 상태

- 실제 C++ codec/SourceIndex:25문서883 element, override25, Load→Save_Atomic→Load→Serialize
  equality 및 exact owner join PASS. 이후 V 단위5행만 교정하고 V42행의 같은 검사를 다시 PASS.
- 기존19문서 hash 보존, 새6문서461 element, Resource 참조254개 누락0, JSON/XML parse와
  새7파일의 project/96.DataFiles 등록 각각1회 PASS. 기존45개 source validator test PASS.
- 실제 CEffectDistribution으로6 emitter/14 burst/28 particle을 검사했다. 위치 최대오차
  7.15256e-7m, 크기·수명 오차0. 색/alpha/회전/반경/속도/cap과 clip 종료 이내를 확인했다.
- camera 두 preset의 schema/시각/키/유효 basis 및 튜닝 줌아웃 수치 PASS. 기존 sampler/
  DurationMs/Stop/owner 해제/Level 카메라 제공 호출자를 코드로 확인했다. 이번 GPU/화면 실행은 아님.
- 새 alias4 header를 포함한 기존 codec 검사 실행파일의 최소 C++ 증분 compile/link PASS.
  이 작업에서 Product 빌드는 실행하지 않았다. 사용자의 빌드/실행을 중지하지 않았으며
 23:50:20 Server,23:50:22 Client 실행을 process로 확인했다. 그 EXE 이후 추가된 alias는
 사용자가 Debug/x64 증분 빌드 후 재실행해야 한다. 새 shader 함수는 없지만 빌드 소요를 단정하지 않는다.
- 변경 경로의 diff-check와 새 JSON whitespace 검사 PASS. 이전 Renderer의 무관한 tab-only
  줄을 지우거나 저장소 전체 dirty 상태를 정리하지 않았다. stage/commit/Client조작/화면 캡처 없음.

실제 로그는 `out/DimensionMasterFVAltRestore20260908/`의 `final_codec_result.txt`,
`v_unit_codec_result.txt`, `final_inventory.json`, `camera_data_validation.json`,
`source_tests.log`, `codec_compile.log`, `tuning_numeric/result.txt`, `full_codec_result.txt`와
`altv_full_handoff/altv_full_validation.json`에 있다. 세부 source 근거와 캡처 보류는
`tuning_handoff.md`, `fv_full_handoff.md`, `altv_full_handoff/README.md`를 따른다.

### Resources와 사용자 확인

추가2파일은 모두 `Client/Bin/Resources/Effect/DimensionMaster/Models/SK_SWP_CUB_00/` 아래다.
`sk_swp_cub_00_sk.wmodel`347,472 bytes와 `textures/sk_swp_cub_00_d.dds`1,048,704 bytes를
설치했다. Git에 추가하지 않았으며 Drive 전달은 아직이다. 다른 새6문서 참조는 기존 파일이다.

Debug/x64 빌드 후 Server+Client profile로 시작하고 Character Select→차원술사→F1→
Effect Tool→All Effects→Refresh→F/V/Alt V→Recovery Effect에서 `_전체`/`_튜닝`을 Play한다.
Alt V는 해당 복구본을 Play하면 대응 camera preset을 자동으로 읽는다. 전체본과 손튜닝의
크기·밀도·색, 카메라 구도/복귀와 실제 조작감은 사용자가 직접 확인한다. S/D/T와 E 잔여
프로그램 확장은 이번에 추가하지 않았다.

## G28~G29. 검격·crack 교정, Alt V 박스 중심본과 저작/컴파일 병목

09-09 사용자 요청을 반영했다. 현재 선택한 full을 원본 계산으로 고친 뒤 사용자가 핵심을
남기는 흐름이다. 사용자 W full20, E full64와 Screen Post4개를 지운 Alt V full340을
보존했다. 이 결과에서 `연결`은 source/data/codec/renderer 상태이며 사용자 외형 PASS가 아니다.

### 실제 변경

| 항목 | 반영 상태 |
|---|---|
| Q 튜닝 | 중앙 성공2행 보존. 주변1행은 fm_d_crack_032.wmodel / Q50이다. full Q mesh03 crack의 원본 size module4개와 SourceScale1, modelPreScale.01을 동일하게 적용했다. 이전2배/5배를 유지하지 않는다. |
| W/F/V 파편 | 파편행만 cube→실제 crack. W 노랑, F/V 하늘색이며 이전 발생·운동·W 사용자 transform을 보존했다. |
| R full30 | 원본 WR259 sprite3의 중복 generic UV speed를 제거하고 WR260/WR256/WR208의 중복 generic emissive를1로 교정했다. 위치·타이밍·SourceRecipe를 새 검격으로 조립하지 않았다. |
| A full104 | 원본 mesh4 MIC/16발생을 WR277~280에 연결했다. 새 PS3개가 MakeFlow/Swing/LinearFlow 원본 식을 소유한다. WR279의 MIC two-sided override와 추가 UV 반복·tangent-view 입력을 연결하고 WR208 중복 gain을 제거했다. 보라 초승달의 사용자 시각 판정은 남는다. |
| BA full4문서75행 | 기존 native27발생 연결과 중복 gain 교정. 실제 Open에서 실패한 base 누락 trail4행은 원본을 남긴 채 실행 보류하여 full visible59행이다. BA2/3를 포함한 실제 codec/Drawable 경로가 통과했다. 사용자 BA0 튜닝과4단 timing은 보존했다. |
| Alt V tuning | 원본 box25행+crack1행, CModel cue1개, 원본 camera448키. 기존8키 손배치 preset을 원본 camera로 교체했다. 원본 particle timing/attachment/module을 유지하고 body 투명도·파란 경계·gain·.01 geometry 단위를 저작 조정했다. |
| S full30 | 기존재질23발생과 SD320~323의 새4계산을 연결했다. native27, visible24이며 미지원 Orbit 등과 미연결3재질 행은 원본 보존/실행 보류다. 원본 vectorfield02를 기존 wvectorfield에 연결하고 MIC321/323의 two-sided override를 적용했다. |
| D full51 | 원본51행 문서와 등록 추가. exact 기존 native21발생과 기존 light2개가 연결되어 visible23이다. 나머지 재질/모듈은 실행 보류이며 D 전체 외형 복원 완료가 아니다. |
| F/V full | 기존 F69/native 관련 보류를 포함한 visible34와 V42/visible42를 재점검했다. F mesh15/V mesh5의 .01 단위와 현재 원본 구성을 보존했다. |

이전 R/A `.tuning.restore` 배치본을 새로 재조립하지 않았다. 이번 검격 수정 대상은 사용자가
선별 중인 R/A **full restore**다. 원본 전체 개수와 현재 사용자가 남긴 개수를 혼용하지 않는다.

### Alt V 장면 캡처와 남은 경계

원본 `fx_m_me_swp_box_01` PS `7a34bdb1e8c49f48b1f747c41f5c1880`의116 RT0 instruction을
ALT178로 연결했다. source의 `screencapture` 입력은 기존 renderer가 occurrence 시작에
`Target_SceneHDR`를 GPU texture로 한 번 복사해 보관한다. 새 공유 resource cache나 CPU
화면 이미지 저장을 만들지 않았다. 복사 시 OM target을 해제/복원하고 실패 시 기존 playback을
보존한다. 새 renderer로 Save refresh를 하더라도 이전 occurrence의 frozen capture를 유지한다.
같은 prepared resource를 재사용하며 숨긴 capture를 켤 때도 최초 capture를 준비한다.

원본 capture-view CB0[0..5]는 아직 해석되지 않았다. 따라서 현재 투영은 **프로젝트 cube-face
UV adapter**이고, 원본 masked RT0의 alpha0는 기존 alpha blend carrier의 particle alpha로
연결했다. source RGB/왜곡/clip 식은 유지한다. 후반 animated CModel은 원본 skeleton/clip과
child bone을 소비하되 기존 translucent presentation에 파란 tint/opacity를 적용한다.
**후반 skinned cube의 native capture material override는 미완료**다. 원작과 동일한 별도
capture camera/투영, 실제 손 구도·큐브 생성/소멸·투명도와 최종 fidelity는 사용자 확인 전이다.

### Effect Tool Open/Save

Read_V1Inventory가 전체206 JSON92,723,784bytes를 parse하던 목록 읽기를 파일별 첫4096bytes의
root metadata 읽기로 줄였다. BOM/escape/nested 값과 마지막 완결 root field를 처리하고,
긴 header는 filename 표시로 남긴다. 본문 validation은 실제 Open/Play codec가 계속 수행한다.
Attach_Saved는 저장된 row만 갱신한다. Save_Staged는 원본 충돌 검사를 유지한 뒤 실제
metadata 변화가 없으면 disk rewrite를 생략한다. 명시 Reload는 새 파일을 다시 찾는다.

실제 C++ CPU probe에서 같은206개 이름이 전부 일치했고 이전 전체 parse58,943.6ms,
새 metadata 읽기45.2939ms였다. 이는 CPU 목록 구성 실측이며 전체 UI가45ms라는 주장은 아니다.
metadata noop/write/external-conflict/row-update/Open/explicit-Reload와 header fixture5개를
확인했다. source JSON parse 실패를 정상 화면으로 숨긴다는 의미도 아니다.

Sequencer Save는 저장한 resource를 쓰는 occurrence만 현재 시점에 갱신한다. 모델 preview의
중복 재시작과 별도 WorldPreview stage를 피하고 camera/clock을 유지한다. 실패와 무관한
resource 저장도 동기 I/O 시간을 다음 playback delta에 넣지 않는다. 실제 UI Save 체감과
camera continuity는 사용자 확인 전이다.

### 긴 shader compile 원인과 수정

이전 Debug 결과에서 실제 SHEX hash가 같은 VS/PS가 mesh7회, particle5회 반복됐다.
상태가 다른 pass마다 `compile VS_MAIN/PS_MAIN`를 다시 선언한 구조다. 복원 재질들이 큰
공통 switch 안에 있으므로 작은 HLSLI 변경에도 그 전체가 반복 컴파일됐다. 두 entry HLSL을
global VertexShader/PixelShader 각1개로 만들고 기존 pass가 공유하도록 수정했다.
pass7/5의 이름·순서·blend/depth/cull·shader body는 그대로다.

| 실제 Debug /Od /Zi 검증 | 시간 | 이전 결과 크기 | 수정 결과 크기 | 실제 shader 수 |
|---|---:|---:|---:|---:|
| Effect mesh | 71,975ms | 98,340,893B | 14,157,678B | VS1+PS1 |
| Effect particle | 92,977ms | 74,244,262B | 15,286,927B | VS1+PS1 |

둘은 병렬로 실행했다. 새 재질 식을 추가한 상태에서도 결과 크기가 각각85.6%/79.4% 감소했다.
이 수치는 두 셰이더 검증이며 전체 Product 빌드 시간을 같은 비율로 단정하지 않는다.
기존 receipt의09-08 23:02 KST 빌드는 총987,761ms 중 Client944,569ms이고,21:11 KST는
총658,729ms 중 Client608,468ms다. 정확히30분인 단일 기록은 이번 조사에서 확인하지 못했다.
최초 별도 FXC는 실제 Debug 옵션과 달리 최적화 기본값으로 시작한 것이 확인되어, 명령줄과
PID가 이 작업의 out 파일임을 검증한2개 process만 중단했다. 실패/중단을 성공 시간에 섞지 않는다.

CSO는 소스 셰이더가 바뀌면 다시 만들어야 한다. 색/크기/배치/수명 같은 JSON 손튜닝만
바꿀 때에는 FXC가 필요하지 않다. 이번 결과는 out의 검증 bytecode이며 Product CSO/EXE를
수동 복사하거나 MSBuild tlog를 조작하지 않았다.

### 검증·현재 실행 준비

- 실제 codec27문서890행: Load→Save_Atomic→Load/serialize 동일성과 authored override24,
  exact SourceIndex owner join PASS. 이후 S 두-sided2행 변경은 S30 roundtrip을 다시 PASS.
- 실제 default parse/Load/Validate_Drawable: 수정 복구본26개 PASS, S vectorfield 수정 후 S1개
  PASS. S two-sided 최종 상태도 actual Drawable 재확인 PASS. 초반 scope 밖 canary3문서는
  별도 legacy module 오류가 있었으며 이번 수정 대상이나27개 복구본 결과에 포함하지 않았다.
- Q/Alt crack full 원본 크기 분포2,440샘플의 최대 차이0, 실제 CEffectDistribution 초기 범위
  .1~.2와 modelPreScale.01 일치. W/F/V 운동·수명 보존은 앞선 crack probe의3726샘플 차이0.
- 새 S/D/Alt core의 실제 사용 Resource51개 존재, 새 Resources0. 원본448 camera key byte값과
  original timing/attachment/module 보존, JSON/XML parse, 새 project 등록 각1회와 실제
  MSBuild project evaluation PASS. 작성 도중 ProjectReference 안에 들어간 중복 등록은 최종
  root ItemGroup으로 교정했고 GUID text-only 구조를 확인했다.
- 실제 ResourceTree/Sequencer/Tool/Renderer/Object 최소 C++ compile 및 codec helper link PASS.
  최신 capture visibility 분기도 Renderer compile PASS. FXC mesh/particle 두 파일 PASS.
  기존 Engine include C4828, Renderer C4805와 FXC X4000 경고는 남으며 이 변경의 compile 오류0.
- `git diff --check`에서 Renderer의 기존 tab-only 한 줄은 작업 시작 전부터 있던 것으로 별도
  기록한다. 그 외 이번 delta와 새 파일의 whitespace 검사, XML 구조 확인을 수행한다.
- Product 전체 빌드/Client/UI 실행·조작·화면 캡처/사용자 visual PASS 없음. 대규모 기존 dirty
  worktree의 unrelated 파일을 stage/commit/reset하지 않았다.

근거는 `out/DimensionMasterSlashFix20260909/`의 `ra_handoff.md`, `ba_drawable_and_a_native_fix.json`,
`q_exact_crack_size_result.json`, `alt_box_restore_result.json`, `sd_full_projection.json`,
`g29_structure_result.json`, `final_codec_result.txt`, `s_final_drawable.txt`, `numeric/exact_result.txt`,
`tool_check/result2.txt`, `capture/shared_shader_result.json`과 각 compile 로그다.

사용자는 Debug/x64 **일반 빌드** 후 현재 LAN server-host의 Server+Client profile로 실행한다.
Character Select→차원술사→F1→Effect Tool→All Effects→명시 Refresh에서 Q/R/A full 또는
해당 tuning을 선택한다. Alt V는 `effect.dimensionmaster.skill.2050540.tuning.restore`의
Play가 대응 원본 camera preset을 자동 연결한다. S/D는 full 항목이며 미연결 행이 남는다.
최종 Q 크기, R 보라 경계, A 초승달, Alt V 초기 capture/투명 반복 box/카메라 복귀와
Open/Save 체감은 사용자가 직접 확인해야 한다.


## G30. Debug 제품 빌드 완료와 ImGui 목록 추가 개선

09-09 사용자의 제품 빌드 승인 후 정본 Product runner를 실제 실행했다. 앞 절의 Product
미실행 표기는 G29 종료 시점이며, 현재 상태는 아래 두 빌드와 EXE/CSO 배포 완료다.

| 실행 | 전체 시간 | Client 단계 | 결과 |
|---|---:|---:|---|
| 02:11:52 KST Debug Product | 192,278ms | 190,700ms | Engine/Shared/Server/Client PASS, runtime 누락0 |
| 02:17:24 KST ImGui 수정 후 증분 Product | 13,649ms | 12,387ms | 네 프로젝트 PASS, runtime 누락0 |

첫 빌드는 공유 VS/PS로 수정한 두 native Effect shader를 실제 Product CSO로 생성했다.
제품 FXC는 순차 실행이며 mesh 완료02:13:00, particle 완료02:14:24다. 두 제품 CSO는
앞선 별도 Debug FXC와 실행 shader(SHEX)2개 hash가 각각 같다. 전체 파일의 바이트는
다르지만 크기는 동일하다(14,157,678B/15,286,927B). 따라서 이전의
shader7벌/5벌 중복 결과를 제품에서 계속 쓰는 상태가 아니다. 전체 clean build 시간이나
다른 소스 변경에서도 항상3분12초라는 의미는 아니다. 두 번째는 Effect_Tool.cpp 변경 후
필요한 C++/link를 수행했으며 FXC 결과와 timestamp가 그대로이고 shader compile 로그0이다.
ImGui 외부 라이브러리 소스는 두 빌드에서 불필요하게 재컴파일되지 않았다.

### Effect_Tool.cpp

Render_RecoveryEffectForProduct의 스킬 행 표시에서 sibling restore/full/tuning의 전체
JSON 자동 parse와 Drawable 검증을 제거했다. Play/Open Editor는 선택한 파일의 기존
검증 경로를 유지한다. 저장본 요소 목록만 열람하려면 해당 행 Show Elements를 사용하고,
이미 열람한 경우 Refresh Elements로 파일 freshness와 재시도를 처리한다. 현재 편집본
요소는 Current Effect에 계속 표시된다. 실패한 파일도 행과 오류를 유지한다.

Render_UnifiedEffectTree/Render_ActiveAuthoredEffectTree의 열린 Family에 ImGuiListClipper를
적용했다. 화면 밖 요소의 label/widget 생성을 생략하지만 전체 source 순서의 ordinal,
stable Element 선택/마킹/Load/Solo와 실제 timeline 재생은 유지한다. 효과 자체를
삭제하거나 sourceRecipe를 줄이는 변경이 아니다. 이 G는 기존 C++ 한 파일만 수정했으며
헤더·저장 schema·Resources payload·프로젝트 등록을 추가하지 않았다.

### S 세션 통합과 확인 경계

같은 checkout의 「차원술사 S 이펙트 전수조사」가 담당한 V63 원본 속도 정렬 수정은
Renderer 소스02:08:13, 실제 제품 obj02:14:38로 이번 제품 빌드에 포함됐다. 그 세션은
원본 축 대조 CPU865개와 최소 컴파일 PASS를 보고했다. 이 G에서는 S renderer나
재질식을 다시 수정하지 않았다. S 최종 외형 확인은 그 세션/사용자의 후속 검증이다.

변경 Effect Tool delta의 diff-check에서 공백 오류0, 복구 JSON27개와 Client project/filter
XML2개 parse PASS다. 기존 전체 dirty Renderer의 tab-only 공백은 이 G 변경 밖이다.
제품 빌드는 기존 문자 인코딩 C4819/C4828 및 FXC X4000 경고를 남겼고 오류는0이다.
Client/Server는 빌드 전 종료 상태였고 에이전트가 실행하지 않았다. 사용자 화면 조작,
스크롤/선택/Solo, Open/Save 체감과 Alt V/S의 시각 품질은 아직 PASS로 기록하지 않는다.

최종 Client.exe는 Client/Bin/Debug/Client.exe(02:17:37 KST)다. server-host 환경이므로
사용자는 Visual Studio Debug/x64 Server + Client profile로 실행하고 Character Select의
차원술사→F1→Effect Tool에서 확인한다. Effect JSON 손튜닝만 바꾸는 경우 CSO 재생성은
필요하지 않다. clean/rebuild와 중간 산출물 삭제를 반복 방법으로 권장하지 않는다.

근거: out/BuildPipeline/runs/20260908T171505116Z-debug-product.json,
20260908T171737881Z-debug-product.json, out/DimensionMasterSlashFix20260909/의
product_debug_build.log, product_imgui_incremental_build.log, imgui_delta.json,
product_build_final.json. 신규 Resources/Drive 전달 및 stage/commit/push는 없다.
