# 캐릭터 재질·차원술사 Q 2차 복구 구현 결과

## 현재 상태

2026-09-09 G33 최신 checkpoint: Saved Skill Effects 가용 높이 확대·장문 안내 정리,
S 원본 splitline Null Dynamic 교정, R dust clean-core 선택 보강, 차원술사 full restore의
확정 차단149행 제거를 반영했다. S 손튜닝 cone/helix3행은 full에서 분리하고 기존 unified는
보존한다. Debug Product receipt `20260909T063739655Z-debug-product.json`이 이 변경을 포함한다.
Q tuning5행/금색 crack1회, A full62행/검격·보라색 crack4회, R tuning13행, S full26행,
F full43행이 현재 구성이다. 아래 앞선 수량과 빌드는 각 시점의 이력이며 이 checkpoint를 우선한다.
사용자가 이어서 요청한 Sequencer/Action Workbench 통합은 별도 기존 Composition PLAN/RESULT에서 진행한다.

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

## G32. 09-09 Solo·운동·검격·D/V mesh 복원 2차와 종료 자원 정리

사용자 요청으로 최신 main을 fast-forward pull해 PR344 포함 `94e90fd9`에서 시작했고,
깨끗한 worktree에서 `codex/dimensionmaster-effect-round2`를 만들었다. LAN 설정은
server-host, TCP7777 LocalSubnet 준비, 시작 당시 listener 없음이었다. Client/UI는 실행하거나
조작·캡처하지 않았다. 사용자의 마무리 요청 뒤 추가 복원 조사는 중단하고 아래 수정 검증만 수행한다.

### 실제 반영

- Solo는 선택 요소가 요구하는 model cue를 숨긴 pose 공급자로 보존한다. Alt V full 자식27개가
  부모 cue 삭제로 실패하던 문제를 수정했다. Update/Seek도 실제 stage한 필터·draft 문서를
  소비한다. Open Editor 다음 Solo가 기존 character/skill target을 준비하며 첫 준비 프레임의
  긴 delta가 짧은 효과의 수명을 소모하지 않게 했다. Play All은 기존 sequencer를 사용한다.
- LocalVectorField의 domain·translation·rotation·sample velocity를 원본 XYZ에서 Client
  `(x,z,-y)`로 함께 변환했다. CylinderSpin의 radialvelocity는 높이축 속도를 제외한다.
- S의 단일 Orbit4행은 원본 CDO의83-byte Options를 typed spawn-only 값으로 해석해 연결했다.
  raw/live 옵션·복수 LINK는 계속 거부한다. 네 행에 남던 hold execution도 해제했다.
- S e48은 V58, D의 `ad182…/85f2…`는 E259와 정확한 PS/MIC를 공유한다. offset-center와
  regular VS의 pivot `.5/.5` 동등성까지 확인한 뒤 기존 native material을 재사용했다.
- V58/65/67/69와 ALT80/81/82/127/129/169/185/191/194/195/196/200/201/202를 기존 native
  velocity basis에 연결했다. 여섯 source VS의 geometry 계산을 대조했고 원본 pivot·UV를
  보존한다. 원작 전체 CPU 난수열·packing의 동일성을 주장하지 않는다.
- V57/58/59/60/70/71/73/74/75의 MIC two-sided override를 descriptor와 A/V full14행에
  반영했다. 추가 S58도 같은 상태를 쓴다. parent만 읽은 생성 정보가 V70 반구를 단면으로
  지정하던 오류였으며 전체 blend/alpha를 바꾸지 않았다.
- D crack `7508b2684fd272292d7b`의 source materialIndex0/1을 각각 V66 LocalCrack과
  SD324 Ice로 연결했다. 원본 모델의286/565 triangles, 같은 particle motion·clock을 유지한다.
  optional `detail.mesh.sourceMaterialSlots`는 기존 material parser/validator와 CModel stage를
  재사용한다. 원본 MeshMaterial 경로·실제 슬롯 coverage를 확인하고 전부 준비한 뒤 commit한다.
  Tool/Playback도 effective slot 재질을 사용하며 사용하지 않는 primary defaultmaterial은 보존한다.
- Ice324는 원본 PS `046f090de8eb2f408bbb8debf92fd28f`의49 RT0 명령, 일반 texture3장과
  tangentView를 사용한다. 별도 SceneColor/Depth sample이나 다른 유리의 계산을 끼우지 않았다.
- V2 종료 시 target/group callback·snapshot과 source cache를 비우고 GPU cache·particle pool을
  마지막에 해제한다. loader join 및 Level/tool 객체 소멸 후 `CMainApp::Free`가 호출한다.
  전체 CRT 누수나 반복 플레이의 지속 증가를 재현·종료한 것은 아니다.

새 C++/HLSL 파일과 vcxproj/filter 등록은 없다. Engine은 원본 mesh materialIndex의 읽기 전용
`Try_GetSourceMaterialIndex`만 추가했다. 기존 C++ 인코딩과 사용자 full 선별 W20/E64/R30/A104,
Alt340 및 모든 기존 stable element ID를 보존했다. 제품 `.unified`는 변경하지 않았다.

### 자동 검증과 실제 한계

| 검사 | 실행 결과 |
|---|---|
| pull 직후 Debug Product | Engine/Shared/Server/Client compile·deploy PASS |
| D 슬롯 추가 전 Debug Product | 41,769ms, 네 프로젝트 PASS, missing runtime input0 |
| 최종 D 슬롯 포함 Debug Product | 232,702ms, Engine/Shared/Server/Client compile·deploy PASS, missing runtime input0. 20260909T032304606Z-debug-product.json |
| Release Product | 실행했으나 D 추가 전 시작한 장시간 optimized particle FXC를 마무리 요청 뒤 중단. Release 완료/PASS 아님 |
| Solo scope 실제 함수 | 28,411 assertions/실패0. baseline에서 Alt 부모 삭제 재현 |
| 준비 프레임 시계 실제 함수 | baseline2.5초 delta 건너뜀 재현, 수정 후0초→16ms |
| native velocity 실제 함수 | 3,023 checks/실패0, 최대 모서리 오차9.83e-7m |
| 운동 실제 코드 block/분포 | 359 checks/실패0, 이전 구현67사례 실패 재현 |
| S Orbit 실제 codec | 43 checks/실패0, 네 Solo 및 unsupported 입력 거부 |
| D 슬롯 실제 최신 codec | 18 checks/실패0. 왕복, primary 보존, 누락·중복·잘못된 슬롯/MIC/texture/profile/cull/형식 거부 |
| 실제 D CModel | 새 Engine DLL에서 source slots0/1 draw261/68 pixels, invalid index 출력 보존·실패한 load 뒤 clone68 pixels 유지, nonfinite0 |
| Ice324 | 원본49명령과 별도 수식108사례 일치, 최대오차4.44e-16. 현재 HLSLI의 최소 PS FXC PASS |
| V2 종료 실제 함수 CPU/COM | 소유 sentinel10/10 소멸, WARP SRV 참조12→1, cache empty·반복 release PASS |
| F1 V1/V2·resource facade | 기존25 tests PASS. 별도 entry/lazy 생성·visible/focus·typed open 경로 확인 |
| Tool metadata/clone/order | 기존31 tests PASS, 슬롯 admission/Detail 복사 실제 header580 checks/실패0 |
| shader closure·culling | 기존 fixture의 누락 UV1 선언을 교정한 Debug closure PASS(24 producers/23 consumers). 기존 compiled pass8 draw에서 역면 OneSided0→TwoSided1352 pixels, nonfinite0 |

D 슬롯까지 포함한 최종27 restore 문서의 Save→Reload canonical equality는27/27이었다.
최신 ABI로 재컴파일한 검사에서890개 중 visible704/hidden186이며 Tool이 허용한693개의 실제
Solo codec 검사가 통과했다. D crack1개가 추가됐고 Alt full27+tuning11의 숨긴 모델
anchor 의존성을 보존했다. 원시 전체 검사 exit1은 기존 비활성 Light11개 때문이며 그대로
보존했다. W/R/S 각1, A4, F2, D2는 baseline과 element 전체가 같고 codec은 허용하나 Tool이
거부하던 placeholder다. 최종 검사도 같은11개만 남아 새 회귀는0이다. 이11개를 새 회귀나
재생 완료로 기록하지 않는다. 변경 JSON4개와 기존 project/filter XML4개의 parse,
전체 변경의 `git diff --check`도 통과했다.

별도 확장 검사에서 Valtan 기존 기대값2개도 baseline부터 불일치했다. high-jump spawn count와
V1 lazy constructor의 옛 open 호출 기대이며 이번 기능 코드와 무관해 변경하지 않았다.
기존 C4828/C4819·FXC X4000/X3577·외부 PDB 경고는 남아 있다. 사용자 화면 확인과 장시간
Client 누수 검사는 하지 않았다. 원시 로그/부분 probe와 Product compile을 혼용하지 않는다.

### Resources와 사용자가 요청한 중첩 폴더 검토

이번 단계의 Resources 추가·교체는0이다. D Ice는 기존
`Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_environ_018.dds`(32,896B),
`fx_d_environ_035.dds`(32,896B), `FX_TEX_00/fx_a_atypical_043.dds`(8,320B)를 사용한다.
새 Drive 전달 payload나 ZIP은 없다. 같은 효과 Resources가 있는 팀 PC에는 코드/Data를 공유한다.

사용자가 물은 `Character/DimensionMaster/DimensionMaster/`는 이전 사본이다. 바깥의
74,781,640B Character.wmodel은 UV 복원 receipt.afterSha 및 실제 activation_installed와
일치하고, 안쪽74,641,016B는 복원 전 파일과 일치한다. 상대 경로112쌍 중111쌍은 동일하며
texture71개도 모두 같다. CharacterCatalog와 AnimationPreviewAssets는 바깥 asset ID를 쓰고,
ActorCatalog/PlayableCharacterAssetService/CharacterPreviewPanel은 그 정확한 경로를 resolve한다.
현재 resource root 환경 override나 Debug/Release 옆의 별도 Resources 폴더는 없다.
현재 정규 로딩이 안쪽 사본을 자동 선택한다는 근거는 없으며 폴더를 삭제·이동하지 않았다.
어제 실행한 EXE/수동 선택 상태까지 현재 파일 검사만으로 단정하지 않는다.

### 다음 사용자 확인

Visual Studio Debug/x64의 **Server + Client** profile을 Ctrl+F5로 시작한다.
Character Select에서 차원술사 선택 → F1 → **Effect Tool V1** → All Effects →
S/R/A/D/V/Alt V Recovery → Open Editor → Current Effect의 Solo/Play Family/Play All 순서다.
D crack은 하나의 element에서 두 재질이 함께 나오는지, Alt V 자식 Solo는 부모 모델을 숨긴
상태에서 재생되는지 확인한다. V2는 별도 F1 버튼에서 열고 각 창의 draft/닫기 상태를 확인한다.

S는30중29 visible, D는51중26 visible이다. 기존 inert Light와 보류 source 모듈은 별도로 남는다.
Alt V 후반 skinned native capture override와 원본 capture-view 행렬, live/multi-LINK Orbit,
CylinderSpin의 일부 방향 옵션은 이번에 완료하지 않았다. CameraOffset은 로컬 UE5와 방향 차이는
확인했지만 원작 UE3 실행 근거가 없고 핵심 S Shine/V 반구/crack/초기 box의 원인이 아니어서
변경하지 않았다. 최종 외형·색·투명도·사용자 visual PASS는 미완료다.

근거는 `out/DimensionMasterRound2_20260909/`의 memory_notes.md, motion_notes.md, tool_notes.md,
native source inventory/geometry proof, 각 CPU/COM/codec/FXC 로그, data delta audit 및
dimensionmaster_nested_resource_audit.json이다. 이 로컬 분석·빌드 산출물과 binary는 Git에 넣지 않는다.


## G33. DimensionMaster V1 선택·저장·재생과 Q/R/S/F 추가 복원 (2026-09-09)

### 시작 상태와 사용자 확인

이 절은 이미 main에 merge된 PR #345 뒤의 추가 작업이다. 시작 HEAD는
`591012dbebf7eeab0b660baec42852b9396e77d4`이며 `git fetch`로 원격을 확인했다.
LAN 설정은 server-host, TCP7777 LocalSubnet 준비 완료, endpoint는 not-listening이었다.
Client/UI는 실행하거나 조작하지 않았다. 사용자가 준 세 이미지는 직접 열람했다.

시작 시 Kouku 작업과 사용자 Effect 손튜닝을 포함한 dirty26파일의 원문을
`out/DimensionMasterRound3_20260909/initial_worktree/`에 보존한 뒤
`codex/dimensionmaster-tool-round3`에서 작업했다. 기존22파일은 byte 단위로 그대로이며,
기존 dirty 중 이번에 바뀐 것은 Q tuning/R tuning/S full/F full 네 문서다.
E full, R full, EffectResourceTree 및 Kouku 변경을 이번 구현으로 정리하지 않았다. A full은 마지막 사용자 정정 후 유리 색만 수정했다.

사용자가 말한 48 mesh와 mesh26/38은 A 전체 문서에서 확인됐다. 중간 답변을 Q4회 복사로 해석해 반영했으나, 사용자의 최종 정정으로 그 구성을 제거했다. **최종은 Q 금색 crack1항목/1회, A 검격과 보라색 crack4회**다. A의 승인된 초승달 shape/운동/시간은 보존하고 유리 색만 바꿨다.

### 구현 상태

| 대상 | 실제 변경 |
|---|---|
| V1 Data Files | typed resource tree 뒤의 조기 return을 제거해 기존 category/effect 검색, family/element 목록, Add Element를 복원했다. 별도 런타임은 만들지 않았다. |
| Ready/Solo 설명 | 문서 validation 결과를 개별 Solo 또는 화면 성공으로 표현하지 않는다. 선택 항목의 hidden/재질 hold/presentation admission 상태를 별도로 표시한다. |
| Shift/Ctrl 선택 | 코드 조사상 클릭마다 전체 Validate/Stage를 실행하지 않았다. stable ID mark의 매 프레임 반복 정리를 문서 교체 시점으로 옮기고, family 분류와 같은 항목 재선택의 중복 작업을 줄였다. |
| Apply/Save | 일반 비연결 preview의 Apply 성공 결과를 재사용하고, Save 뒤 중복 Stage/Refresh/예약 preview 재시작을 제거했다. Product·registry·Valtan 적용 경계는 유지한다. |
| Codec Save | 기존4인자 API를 유지하고 실제 저장한 canonical 문자열을 돌려주는5인자 API를 추가했다. 디스크 canonical이 byte 일치하면 불필요한 parse를 생략하고, 형식만 다른 경우 기존 semantic 비교를 사용한다. candidate Validate, 임시 파일 load/serialize 왕복, 외부 변경 거절, backup/rollback은 유지한다. |
| JSON DOM | MSVC Debug에서 noexcept가 아닌 map 포함 node를 vector가 배열 성장 때 깊게 복사하던 비용을 명시적 private move 성장11줄로 줄였다. 공개 타입/ABI·문법·오류·limit·실패 시 out 보존은 바꾸지 않았다. |
| Q tuning | 기존 cube/slice4행과 금색 crack1행, 총5행이다. crack은0.27초에15입자를 한 번 생성한다. A 검격이나4회 반복은 없다. 하나의 cylinder가 원형 전체에 분산하도록 X 양쪽 반구를 사용한다. |
| A full | 총104행/mesh48개를 유지한다. WR279/280 검격의0.25/0.60/0.90/1.30초 네 시점을 보존하고, 각 타격의 기존 q-local-crack/WR238 유리 두 층을 보라색으로 맞췄다. 별도의 Q51 glass-hole sprite4행에도 검은 중심 면 합성을 연결했다. 검격 shape/운동/개수는 그대로다. |
| R tuning | 기존3 dust slash에 검은 중심 면 합성을 추가했다. full의 swing3개, 보라색 crack6개를 조합하고 cube burst를 확산시켰다. 총13행. crack2의 원본 반전 yaw를 해제하고 X 반구까지 반대로 맞춰 양옆 source velocity가 상쇄되지 않게 했다. |
| S full | 기존27행을 보존하면서 crack2개의 반사/내부/외부 색을 금색으로 바꾸고, 사용자가 unified에서 저작한 cone 메인 검격과 helix2개를 연결해30행으로 만들었다. cone은 mesh를 쓰는 particle이며 sprite24개와 구분한다. |
| S 짧은 burst | cone의1ms emitter 구간을60Hz tick이 건너뛰던 문제를 고쳤다. 수명을 늘려 숨기지 않고 첫 eligible tick에서 한 번 생성하며 실제 particle life0.3초를 유지한다. |
| S CPU |500 sprite의 같은 vector-field 설정·행렬·field 조회를 element/tick마다 재사용한다. 입자 수나 RNG, 입자별 field sampling/curve는 보존한다. |
| F native 재질 |15mesh 중 기존1개 외에 WR263 broken2개, D source material slot crack2개, SD325–332 native8종을 쓰는10행을 연결했다. 다른 shader로 임의대체하지 않고 회수한 PS/VS/texture 입력을 기존 native 경로에 추가했다. |
| F 운동 | 첫 broken Orbit raw options를 실제 CDO/instance 근거의 typed spawn flags로 복구했다. 원본에서 꺼진 CircleSurface를 validator가 거절하던 조건만 수정했다. SD329의 다른 emitter 의존은 아래 별도 저작 경계로 처리했다. |
| Add Element slot material | 실제 source material slots를 쓰는 D/F crack은 사용하지 않는 primary hold가 아니라 유효 slot들의 실행 가능 여부로 복사 admission을 판단한다. FOLLOW/Trail/history 의존과 최종 canonical 왕복 검사는 유지한다. |

### 원본 복원과 저작 조정의 경계

R의 검은 줄은 Q51 glass-hole의 둥근 UV/투명 mask를 길게 늘린 상태에서 나타나는 구멍과
관련 있었다. 원본 PS의 alpha 식과 실제 texture를 조사했으며 무조건 transpiler 오류라고
판단하지 않았다. `effect.project-tuned.dimensionmaster-r-glasshole-solid-core.v1`은
기존 native aura 뒤에 검은 중심 면을 합성하는 **선택 항목의 저작 조정**이다. 최초 R3행에 적용한 뒤 A의 동일 MIC4행에도 연결했다. width0이면 기존
Q51과 같은 경로이며 현재 width0.38/softness0.12다. 원본 R 셰이더 회수 완료나 화면 일치로
표현하지 않는다.

S cone/helix는 사용자가 이미 만들어 놓은 저작 mesh를 full에 연결한 것이다. 조사한 S 원본
묶음은 crack mesh2개와 sprite24개 등이었고, 원작 메인 검격 전체가 반드시 sprite라고
확정한 것은 아니다. 기존 S crack의 두 초기 속도는 부호가 다르지만02 yaw가 다시 뒤집으므로
주 방향은 비슷하다. S는 그 운동을 유지했고 R에 복사한 여섯 crack만 실제 양방향으로 고쳤다.

F SD329 한 행의 enabled LocationEmitter는 `ppp`의 particle state를 필요로 한다.
원본 owner의 Orbit는 lookup header를 제외하고 읽으면 offset(-240,0,0)cm와
rotation rate(0,0,-4.5)turn/s다. 이를 원본 LocationEmitter와 동등하다고 속이지 않고,
해당 한 행을 **authored orbit-path tuning**으로 표시해 기존 startlocation curve에
600Hz/121점의 원형 출생 경로를 기록했다. 보간 오차는0.067cm 이하이나 원본 emitter 선택,
rotation/velocity 상속, RNG 순서 동등성을 주장하지 않는다. 재질·기존 크기·수명·50/sec 생성
입력은 유지하며 외부 owner 없이 Solo 저작이 가능하도록 만든 조정이다.

Life16은 **16초가 아니라 원본 입자 수명의16배**다. UI와 validator의 배율 상한이16이며,
원래 입자 수명 및 emitter timing과 구분한다. R 세행에서 배율16으로 생긴 입자가4초 뒤에도
남는 것을 확인했고 duration도7.2/7.4/7.75초로 계산됐다. 추가적인 조기 종료 결함은 찾지 못했다.

### 자동 검증 증거

| 검사 | 실제 결과 |
|---|---|
| 원래 Tool/V1/V2 검사 |56개 통과. Data Files entry·clone label·F1 독립 경로 포함 |
| canonical Save 비교 |변경 전/후 각16개 검사 통과. 외부 내용 변경, 삭제, 생성 충돌, invalid candidate 거절 및 실패 시 파일/출력 보존, 형식만 바뀐 JSON 허용 포함 |
| generic JSON 차등 |76개 입력의 성공/실패·전체 DOM 값·오류 bytes·실패 시 out 상태가 baseline과 완전히 같음. Unicode, -0, 숫자 token, duplicate key, byte/depth/value limit 및 배열 성장 포함 |
| 실제 Debug 저장3회 평균 |S30행1466.64→399.60ms, A104행5752.18→1612.20ms. preview GPU Stage를 포함한 UI 벽시계나 재실행 FPS로 표현하지 않음 |
| JSON 파싱 누적 할당 |A의 generic DOM 약1,074만→370만 allocation, 누적749.8→231.7MB. retained 메모리나 메모리 누수 감소 수치가 아님 |
| S CPU |500입자 동일90tick hash2656634922190783291, 다른23행15596061025277650640, 전체24행12412999347766807972 보존.500입자 약25.55→17.3ms, 전체24행 약28.3→19.7ms. 실제 Client FPS 측정 아님 |
| S cone/helix |1ms cone은 baseline에서생성0, 변경 후1. 양쪽 helix도각1, finite. 수명0.3초 입력 보존 |
| 기존 수명 끝 경계 |629개 경계 검사 중2개1ULP Update/Seek 차이는 실제baseline에서도 재현. raw exit1이며 이번 회귀0.629개 전부PASS라고 기록하지 않음 |
| A 네 타격 |A104행 전체를0초부터 끝까지 한 번 재생하는 수치 검사에서 대상16행 모두 각자 시간에 생성·finite. 이 검사는 마지막 색 수정 전이며 그 뒤에도 timing/운동은 보존 |
| R 검은 중심 면 |실제 focusedPS FXC 통과, WARP60경우/296검사 통과. native width0 경계 포함. 화면 fidelity PASS 아님 |
| F8 native |실제 제품header348검사 통과, 제품 include를 쓰는 PS8개 FXC 통과. 원본RT0 510instruction 순서/sample slot 보존,1026 source uniform 평가 finite, 공통VS 원본 byte 일치, DDS21개 존재 확인. 기존SD320–324 보존 |

각 시점의 Q/A/R/S/F 재생 sweep, Product Debug 빌드와 최종 diff 검사는 완료되는 실제 결과를 아래에
추가한다. Client/UI 입력·Save 체감·Play All FPS·원작 검격의 실제 화면 일치는 사용자 검증 전이다.

### Resources 및 실행 준비

이번 작업은 Resources binary 추가/교체가 없다. 기존
`Effect/DimensionMaster/Meshes/` 및 `Effect/DimensionMaster/Textures/`를 사용한다.
F 신규프로파일의21개 texture asset ID는 실제 JSON의 sourceProfile.textures에 기록됐고
모두 현재 Resources에서 확인했다. cone/helix/crack도 기존 입력이다. 새 Drive binary 전달물은 없다.

완료 빌드를 실행할 때는 server-host PC의 Debug/x64 **Server + Client** profile을 사용자가
Ctrl+F5로 시작한다. Character Select의 DimensionMaster에서 F1 → Effect Tool V1 → Data Files
또는 All Effects로 들어가 Q/R tuning restore와 S/F full restore를 다시 Load한다. Current Effect의
Solo → Play Family → Play All 순서로 확인하고 Q 네 group, R 검은 중심/양옆 파편, S cone/나선,
F 개별 mesh를 판단한다. V2는 F1의 별도 Open Effect Tool V2 entry를 사용한다.

증거는 `out/DimensionMasterRound3_20260909/`의 tool_notes.md, memory_notes.md,
save_probe, r_core_verification.json, f_native, runtime_diag 결과와 preservation_audit.json에 있다.
분석 산출물·compiled binary·Resources는 소스 변경에 포함하지 않는다.


### G33 중간 Q4회 구성의 CPU 검사와 첫 Product 빌드 (Q 최종 구성으로 사용하지 않음)

`runtime_diag_closeout_build.log` compile exit0, `runtime_diag_closeout.txt` run exit0.
당시 Q20/20, R13/13, F43/43 admitted 행이 Stage와 전체 수명 생성·finite 검사를 통과했다. Q20 구성은 이후 사용자 정정으로 제거했으므로 최종 Q 검증 증거가 아니다.
S30행에서는 inert light1개를 제외한29행이 통과했다. Q 전체를0초부터 재생한 경우에도
검격과 금색 crack의 네 타격이 실제 고정 tick0.28333/0.63333/0.93333/1.33333초에 생성됐다.
입력 시점0.27/0.62/0.92/1.32초와의 차이는60Hz 첫 eligible tick 때문이다.

F mesh15개는 모두 수치 생성되나 그중5개는 실제 캐릭터 bone anchor 대신 명시적인 numeric
fixture를 제공한 검사다. `FX_State_01 → bip001-spine2`, `FX_Buff_01 → b_root`,
`WP_SWM_M_1 → b_wp_swm_m_1`의 실제 bone 위치가 필요하며 numeric fixture를 Client 화면 PASS로 기록하지 않는다.
F의 나머지 hidden sprite21개는 이번 mesh 복원의 완료 범위에 포함하지 않는다.
F 두 source-material-slot crack의 Add Element clone이 모두 통과했고, slot1을 failClosed로
바꾼 경우 복사를 거절하며 기존 대상 문서를 보존했다. F329 곡선은 명시적인
`authoredModuleOverrides=true`로 Tool에서 수정 가능한 저작 상태를 표시한다.

첫 Debug Product compile/deploy는 exit0이다. Engine/Shared/Server/Client가 완료됐으며
receipt는 `out/BuildPipeline/runs/20260909T050529887Z-debug-product.json`이다.
기존 FXC X4000/X4008/X4717와 외부 DirectXTK PDB 경고는 남아 있다. 마지막 Renderer
SubUV 최적화 및 Codec copy helper 변경 후 증분 빌드는 별도로 기록한다. 이 빌드는 Client를
실행하거나 화면을 검증하지 않았다.


### G33 소스 동결과 사용자 빌드 인계

추가 Renderer SubUV 최적화는 실제 이전/현재 함수의19,153개 입력 결과가 완전히 같았다.
Debug500입자의 반복 설정 해석은3.10→0.011ms다. 입자 수·프레임 보간·flip·순서는 유지한다.
EffectAuthoringSequencer의 전진 재생은 이미 `Advance_PreviewWithTransformHistory`를 통해
새 fixed step만 진행하며 매 프레임 처음부터 Seek하는 결함은 현재 소스에 없다.

F BONE attachment의 live Solo에는 동기 animation timeline이 반드시 필요한 것은 아니다.
실제 caller를 끝까지 확인하면, Solo가 `Prepare_RecoveryPreviewTarget`으로 DimensionMaster와
F skill을 선택하고 current-pose bone을 공급한다. Play All은 같은 target에서 saved F sequence를
시작하고60Hz bone history를 기록한다. 처음의 strict Seek helper만 보고 timeline 없이는
Solo가 불가능하다고 판단했던 중간 조사 내용을 이 실제 호출 경로로 정정한다. 필요 시 사용자
수동 복구 항목은 Model View → Target → `DimensionMaster Character (154 clips)`다.

사용자가 직접 빌드를 시작하겠다고 확인한 뒤 모든 production source를 동결했다.
앞선 Product Debug 빌드는 통과했지만 마지막 SubUV/Codec 보완을 포함한 증분 Product 빌드는
**사용자 실행에 인계했으며 아직 성공으로 기록하지 않는다**. 병렬로 같은 Product 빌드를
실행하지 않는다. 변경 JSON4개와 stable ID 유일성, 기존 project/filter XML4개 parse,
전체 `git diff --check`는 통과했다. 기존22개 dirty 파일은 이번 작업이 덮어쓰지 않았다.

새 작업의 commit/push/PR은 아직 실행하지 않았다. 이전 PR #345 merge와 혼동하지 않는다.
남은 확인은 최종 사용자 빌드, 실제 Solo/Play All의 위치·검격 채움·S cone/helix와 frame time,
그리고 사용자의 화면 판단이다. 측정한 CPU/저장 개선을 실제 Client FPS나 visual PASS로
승격하지 않는다.


### G33 최종 Q/A 정정

사용자가 명확히 정정했다. Q는 crack 한 항목만 추가하는 것이며, 네 번의 검격/보라색 crack은
A에 해당한다. 잘못 추가했던 Q의 A 검격8행, 반복 crack6행, 추가 crack1행을 제거했다.
Q 최종은 기존 cube/slice4행 + 금색 crack1행 =5행, 금색 crack의 source burst는0초 한 건이며
Effect local start는0.27초다. 따라서 Q에서 검격/파편을 네 번 반복하는 구성은 없다.

A 최종104행은 원래 검격과 유리 타격 시간0.25/0.60/0.90/1.30초를 유지한다. 기존 두 유리층
(q-local-crack/WR238) 각각4행의 refle_color/in_color/out_color만 보라색 계수로 바꿨다.
기존 A 검격의 geometry·source motion·timing은 변경하지 않았다.
`out/DimensionMasterRound3_20260909/qa_final_correction.json`에 제거한 agent-created ID15개와
A의 실제 변경8행, 네 시점, 단일 Q burst를 기록했다. 이번 정정은 JSON만 바꿨으며 사용자
빌드 중 C++/shader 소스는 계속 동결했다. 앱에서 Q/A 문서를 다시 Load해야 기존 편집 cache와
혼동하지 않는다.


### G33 최신 Q/A 확인 및 0바이트 Engine DLL 복구

최종 Q/A 정정 후 기존 검사 실행 파일과 보존한 정상 Engine DLL로 최신 JSON만 다시 읽었다.
Q 전체5행은 모두 생성·finite이며 gold crack 한 행의 peak15, birth interval1,
첫 tick0.283333초를 확인했다. A104 전체 재생에서도 검격 두 family와 유리 두 family의
각4행, 총16개가 네 타격 시점에 모두 생성·finite였다. A의 HEAD 대비 실제 차이72개는
유리8행×색vector3개×RGB3 성분뿐이며 geometry/timing/motion 변경은0이다.
`qa_final_q.txt`와 `qa_final_a.txt`가 폐기된 Q20 검사 대신 현재 Q5/A4 증거다.

F의 실제 outer body도 별도 숫자 검사에서 CModel 생성/Clone이 성공했다.154clips/225bones이며
bip001-spine2, b_root, b_wp_swm_m_1 조회와 finite matrix 등13검사가 통과했다.
당시 배포된 정상 Debug Engine DLL의 out 복사본을 사용했으며 정확한 SHA/경계는
f_follow_bones_receipt.json에 기록했다. 이 검사도 Client/UI 실행이나 화면 확인은 아니다.

사용자 빌드 후 Engine/Bin/Debug 및 Client/Bin/Debug의 Engine.dll이 모두0바이트로 확인됐다.
Engine.log의 직접 오류는14:16:21 `LNK1114: Engine.lib를 덮어쓸 수 없음, 오류5`이다.
Client는 이전 EngineSDK import library로 링크할 수 있었고, vcxproj 배포는 DLL의 존재만
확인했으므로0바이트 DLL도 복사했다. 같은 시각 에이전트의 별도 검사 링크가 같은 Engine.lib를
읽고 있어 읽기 잠금 경합 가능성이 있다. 실제 잠금 소유자 추적 기록은 없으므로 확정 원인으로
단정하지 않는다. 사용자 실수나 VS18/VS17 toolset 불일치로 설명하지 않는다. 두 lastbuildstate는
v143/14.44.35207/SDK10.0.26100.0으로 같았다.

추가 probe compile/link/실행을 모두 중단하고, workspace 안의 정확한 두 DLL 경로와0바이트를
재확인한 뒤 그 두 실패 출력만 제거했다. 현재 소스에 대한 Product Debug 순차 빌드로 Engine을
다시 링크하고 Client로 배포한다. 정상 이전 DLL을 제품 경로에 덮어쓰는 우회는 하지 않는다.
관련 원본 로그와 SHA는 zero_dll_link_evidence에 보존했다. 이 복구 빌드의 최종 결과는 아래에
추가하며 Engine/Client source 변경으로 오인하지 않는다.


### G33 복구 완료 빌드

현재 최종 소스의 Debug Product compile/deploy는 exit0, Engine/Shared/Server/Client 모두 PASS,
missingRuntimeInputs0이다. 최종 receipt는
`out/BuildPipeline/runs/20260909T053148574Z-debug-product.json`이다. 이 빌드는 마지막 SubUV와
Codec 보완을 포함하므로 앞의 사용자 빌드 인계 미확인 상태를 대체한다.

Engine/Bin/Debug/Engine.dll과 Client/Bin/Debug/Engine.dll은 모두8,718,848B이며 SHA256
`6e67578e75661a3230268dde8fb182471212885462871df780dd1218ee8d2c12`로 일치한다.
두 DLL과 Client.exe(48,738,304B), Server.exe(12,739,072B)의 MZ/PE signature,
x64 machine, DLL/EXE flag, 모든 section의 파일 범위를 확인했다.
`out/DimensionMasterRound3_20260909/dll_recovery_verified.json`에 기록했다.
Client를 에이전트가 실행한 것은 아니며 실제 시작·화면·Play All FPS는 사용자가 확인한다.

최종 authoring 데이터는 **Q tuning5행/금색 crack1회**, **A full104행/검격과 보라색 유리4타격**,
R tuning13행, S full30행, F full66행이다. JSON5개 parse와 정상 Git 설정의 diff check를 통과했다.
제품 CPP/shader는 동결했고 새 commit/push/PR은 아직 실행하지 않았다.


### G33 A 검은 dustparticle 보강 누락 수정 완료

앞서 A의 색 수정만 완료한 시점에는 Q51 glass-hole sprite4행에 검은 중심 면 보강이 없었다.
사용자의 A/R 재확인 요청에서 이 누락을 확인해, A 6449356759fd8f684f81,
f23886dd0742331c9546, 99d30eff4c2c88e26aaf, a91256889932fbe55fef에 기존 R solid-core
runtime alias와 ProjectTuned scalar 두 개(width0.38/softness0.12)를 연결했다.
이 세 필드 변경을 되돌린 JSON이 변경 전과 완전히 같음을 확인했다. 원래 native texture,
색/동적 입력, 운동, 시간, WR279/280 초승달 mesh는 보존한다. 위의 A HEAD 대비 색72개만
변경됐다는 기록은 이 보강 전 시점의 증거이며 현재 적용 범위는 색8행+core4행이다.

기존 검사 EXE로 A4행의 focused Codec/Playback을 확인했다.4/4 생성, 각 birth interval1,
nonfinite0, exit0이다. A 전체104행을 다시 Load/Stage/재생한 검사에서도 기존 검격·유리
대상16/16 생성, nonfinite0, exit0이다. 최초 검사 실행은 PATH에서 의존 DLL을 찾지 못해
시작하지 못했고, 기존 정상 out DLL과 Client/Bin/Debug 의존 DLL 경로로 재실행했다.
새 컴파일·링크나 Client/UI 실행은 하지 않았다. 근거는 a_black_core_receipt.json,
a_black_core_runtime.txt, a_black_core_full_runtime.txt다.

제품 소스와 DLL은 직전 성공한 Debug Product 빌드 그대로이며 A 문서만 다시 Load하면 된다.
이는 전체 dustparticle shader의 일괄 수정 또는 원본 PS 복원 판정이 아니다. 같은 Q51을 쓰는
R tuning3행+A full4행의 선택 보강이며 사용자 화면에서 검은 면/광택/겹침 확인은 남아 있다.

### G33 S 원본 입력 추적·R 마지막 dust 보강·미지원 Solo 정리

`Effect_Tool.cpp`의 Saved Skill Effects는 고정190px 대신 남은 창 높이를 사용한다.
상시 장문 안내와 반복 진단을 제거했으며 검색·Add Element·Load·Save 계약은 보존한다.
상태 첫 줄과 전체 툴팁으로 실제 실패 원인을 계속 확인할 수 있다. 기존 목록/복사 검사28개가 통과했다.

S e49 V69 splitline은 shader가 있는데도 입력 Dynamic이0이라 원본 PS의
`floor(textureR+Dynamic.z)`가 alpha를0으로 만들었다. 원본 selected dynamic VF/VS와
Null Dynamic 기본값을 확인하여 기존 Q45/49 모듈 부재 분기에 V69만 추가했다.
원본 PS는 수정하지 않았다. 실제 현재 Playback의409입자 입력은 모두 finite다.
WARP972조건에서 이전 V69 117조건의 alpha0이 수정 후117조건 모두 양수로 바뀌었고,
나머지855조건의 PS 출력은 필드별로 동일했다. frame 전체가 bit-identical한 것은 아니다.
V53/64의68행에 최대1.43e-6 행렬 성분 차이/0.0157px 투영 면적 차이가 있었으며
PS 입력은 V69 Dynamic 외에 동일했다. V63 Shine은 수정 전에도 양의 alpha를 출력했다.
원본 live CPU stream과 사용자 화면 일치를 확보한 것은 아니다. 자세한 원본 근거는 S forensic RESULT에 둔다.

사용자 첨부 R 이미지에는 검은 중심 위 밝은 점·띠가 남았다. 기존 solid-core는 검은 바탕을
뒤에 채우므로 기존 noise의 밝은 RGB는 앞에 남는다. R tuning3행만 새 clean-core alias에 연결해
중심 coverage 안의 기존 RGB 기여를 줄였다. 원본 Q51·A의 기존 solid-core 계산은 유지한다.
60개 입력에서 기존/새 계산의507조건 검사, finite, alpha/외곽/fade 보존, 중심 RGB0을 확인했고
실제 PS wrapper FXC도 통과했다. 이는 프로젝트 저작 보강이며 native PS 복원 판정이 아니다.
사용자의 voronoi 검격과 세 행의 발생시간·운동은 보존했다.

full14개 최신 SHA를 쓰기 직전에 확인한 뒤 확정 차단149행(재질122, 비활성 Light24,
BA3 hidden/unbound3)을 제거했다. 사용자 숨김만 있고 미지원 증거가 없는 D1행은 보존한다.
S의 손튜닝 cone/helix3행은 별도로 full에서 분리했으며 기존 unified와 out 백업에 보존한다.
normal owner/FOLLOW/model cue와 남은 모든 element의 다른 필드는 그대로다.
정리 직전798행에서 현재646행이며 actual Codec/Playback으로 full14개와 R tuning의
15문서 모두 Load/Stage에 성공했다. 제거는 원본의 미지원 기능을 새로 구현했다는 뜻이 아니다.
근거는 `full_cleanup_applied.json`, `s_native_trace/validate_after.log`,
`s_native_ps_probe/before_after_receipt.json`, `r_clean_core_run.log`에 둔다.

최종 Debug Product compile/deploy exit0, Engine/Shared/Server/Client 모두 PASS,
missingRuntimeInputs0이다. receipt는 `out/BuildPipeline/runs/20260909T063739655Z-debug-product.json`.
Engine/Client의 Engine.dll은8,719,360B이며 SHA256
`08e1dacf3b81ad114eb44b5494f8a29690dded43e27973e12695374025994c62`로 일치한다.
Client.exe는48,782,336B다. 다른 세션의 선행 변경을 포함한 공유 작업 트리의 빌드이며,
이번 이펙트 작업에서 Engine 변경을 추가했다는 의미는 아니다. 기존 compiler/FXC/PDB 경고는 남아 있다.
Client/UI를 실행하거나 화면·청각·FPS를 대신 확인하지 않았다. 새 commit/push/PR도 아직 없다.


### G34 사용자 중단 시점: R 원본 검격·Q dust 출처와 복원 한계

사용자의 원본 복원 요청 뒤 추가 clean-core 강화는 전부 되돌렸고, 이어진 중단 요청에 따라
더 이상의 복원·실험을 멈췄다. 기존 R tuning의 clean-core 보강과 사용자의 Voronoi는 보존한다.
R tuning 세 slash는 Q nailstrike emitter13/Q51 glasshole을 복사한 저작 요소다. 원본 Q의
PSA_Velocity·center offsetY=.9·20×130cm·image flip을 R용 PSA_Rectangle·offset 해제·
260×35cm·flip 해제로 바꾼 입력이다. 이를 원본 R foldcut 검격의 직접 복원이라고 설명하지 않는다.

원본 R full WR259 sprite3·WR260 swing mesh3은 selected native 자료와 재질 파라미터25/40개가
일치하며 실제 Dynamic module을 보존한다. 현재 Codec/Playback을 컴파일한 60Hz trace에서
6행의 입자 표본2912개가 모두 유한했다. 현재 renderer에서 추출한 geometry 계산과 원본
fm_h_swing_05 UV/정점 및 modelPreScale을 소비했다. 같은 재질 packet·DDS·color·Dynamic을
WR259/260 PS에 넣은8736개 합성depth 검사에서 nonfinite0이고, 6행 모두 유효 시간에
alpha>0을 생성했다. 단, 이는 고정 카메라·identity root·UV grid·합성depth의 수치 검사다.
원본 live vertex stream, 실제 아레나 culling/pose/화면 또는 최종 검은 검격 fidelity 검증은 아니다.
근거는 out/DimensionMasterRound3_20260909/r_native_trace와 r_native_ps_probe/summary.json이다.

Q51은 원본 DXBC377108e10f08cc488de94c405e789871을 직접 실행해 현재 QNative51과
같은 입력120조건/491520픽셀을 비교했다. 검사기의 초기 VS register 순서와 native uniform
binding 오류를 먼저 고쳤으며 그 두 실패는 제품 결함 근거로 사용하지 않는다. 교정 후120조건의
alpha-positive pixel 수가 모두 같고 nonfinite0이다. 다만 상대 오차1e-3 기준2460픽셀이 차이나며
최대 alpha 오차.00119519, RGB RMS 최대.00105038이 남아 strict comparison exit1이다.
따라서 PS bit-exact PASS나 검은 줄 원인 규명으로 기록하지 않는다. 입력은 통제된 material/
UV/color/dynamic/scene 및 fog identity/opacity1이며 실제 원작 장면 확보와 구분한다.

원본 immediate32를 읽으면 현재 Q51의6자리 disassembly 기반 literal과10개 instruction,
11개 lane에서 float32 bit 차이가 있다. instruction38의 원본4.999999987e-7(0x350637bd)이
0.000000으로 손실된 사례가 포함된다. 원본 bits를 사용하는 후보는 out/r_q51_native_diff에만
준비했다. 사용자 중단에 따라 제품 미적용·미컴파일·미실행이며, 잔차나 검은 줄의 원인이라고
확정하지 않는다. 이 원본 상수 복구는 보강 mask와 다른 종류의 가능한 후속 복원이다.

불가능하다고 확정한 R 검격 element는 없다. selected PS/VS·재질·texture·module 자료가 있으므로
원본 입력/상수/발생 조합을 대조·수정할 경로가 있다. 현재 미완료인 원본 distortion/MRT와
장면/fog·실제 vertex 입력까지의 일치는 추가 renderer 연결 또는 실제 장면 증거가 필요하다.
미지원 Solo 정리는 현재 실행되지 않는 행을 full 목록에서 제거한 조치이며 원본 자료의 삭제나
영구 복원 불가능 판정이 아니다. S는 원본 Shine/주변 선 입력 연결과 V69 Null Dynamic 교정의
수치 결과까지 반영했지만, tuning cone·helix를 원본 S로 취급하거나 사용자 visual PASS로
승격하지 않는다. 최종 EXE 빌드 상태는 Composition Workbench RESULT G28의 최신 receipt를 따른다.

### G35 마무리 추가 요청: A 네 타격의 보라색 cube/crack

사용자가 제공한 원본 이미지3장과 서면 관찰에 따라 가로로 깨지는 검격 경계를 정상 표현으로
구분했다. 검은 중심 면과 보라색 외곽의 시간별 형성·소멸이 핵심이며, 주변 cube/crack은 별도
관찰 대상이다. 정지 이미지에서 실제 element 수나 타임라인 순서를 확정하지 않았고, 현재
Client 검격이 원작과 일치한다는 승인으로 기록하지 않는다. 추가 셰이더 보정은 하지 않았다.

A full에는 보라색 crack emitter4/24가 각각4행, 총8행 이미 존재했다. 기존62행은 그대로 두고
R tuning의 purple-glass-sequence에서 cube mesh/재질/크기/수명을 복사한4행을 추가했다.
현재 A는66행이며 새 ID는 authored.composition.dimensionmaster.a.hit-1/2/3/4-purple-cube다.
R donor의 세 burst를 각 새 행의 단발6개로 바꾸고 recipe와 spawn literal, maxParticles를
일치시켰다. 기존 R 자체와 사용자의 검격·crack 값은 바꾸지 않았다.

네 occurrence 시작은0.25/0.600000024/0.899999976/1.29999995초이며 emitter delay는 기존
crack과 같은0.100000001초다. A crack24의 transform/root attachment/localSpace 및 sphere→
cylinder-spin 생성 위치·초기 속도를 차용했다. R의 이후 수명·회전·fade와 속도 배율은 유지한다.
이는 A crack의 전 생애 가속/감속을 동일하게 복제했다는 뜻이 아니다. 원본 A cube emitter를
특정한 native 복원 판정과도 구분하는 사용자 요청의 저작 조합이다.

쓰기 직전 최신 bytes를 확인하고 기존 파일 내용에4행만 append했다. 기존62행 및 다른 top-level
필드 구조가 같고 R donor가 불변임을 확인했다. 적용 SHA256은
069cbf485cc7820c02729b05f5b3bd06a08615d76cadcb03ae02427edf8b16f4이며 JSON parse와 해당
파일 git diff --check가 통과했다. LF/CRLF Git 안내는 남지만 whitespace 오류는 없다.

기존 실제 Codec/Playback probe의 A 모드로 전체66행 Load/Stage 및3.67초/223step 재생을
확인했다. 기존 검격/crack 대상16/16 생성, 모든 frame particle World nonfinite0, exit0이다.
별도 out 입력에 새 cube4+기존 crack8의 원문 행을 보존해 기존 q 모드로 확인했다.12/12 생성,
cube각각 peak6, crack각각 peak10/12이며 모두 birth_intervals1이다. 세 종류의 첫 생성은
0.366667/0.716667/1.0/1.4초로 같고 처음 두 그룹은60Hz float 경계에서 약1frame 뒤에 관측됐다.
근거는 out/DimensionMasterRound3_20260909/a_cube4의 apply_receipt.json, full_a.log,
cube_crack_12.log다. 이 검사는 보존된14:10 실행 파일의 데이터 회귀 검사이며 최종 Product
동일 binary 또는 사용자 화면·FPS 검증으로 주장하지 않는다. Client/UI 실행·캡처는 하지 않았다.

### G36 R/A의 정상 검격과 S 메인 검격: 원본 입력 복구 및 실제 입력 연결

사용자의 추가 설명은 A의 가로 분열 자체를 없애라는 뜻이 아니다. 정상 검격이 형성된 뒤의
특수 처리와, 현재 R/A 공통 dust 재질이 처음부터 자글거리는 결함을 구분한다. 첨부한 R/S 원본
이미지 두 장을 열어 검은 내부·보라 경계와 S의 청색 중심·밝은 주변 선을 확인했다. 정지 이미지로
원본 element나 발생 시각을 단정하지 않았으며 사용자 화면 승인으로 기록하지 않는다.

**실제 반영된 연결과 시간 입력**

| 항목 | 변경과 현재 상태 |
|---|---|
| 실제 R 입력 | `2050180 → foldcut → effect.dimensionmaster.skill.2050180.full.restore`, 23개 요소 |
| 실제 S 입력 | `2050220 → momentaryrift → effect.dimensionmaster.skill.2050220.full.restore`, 26개 요소 |
| Catalog | 위 두 `DIRECT_AUTHORED_DOCUMENT`를 등록했다. `CProjectDataRoot`의 Data 원문을 읽는 기존 경로다. |
| R WR260 | 세 occurrence의 `sourceScale.lifeTime`을 16에서 1로, 마지막 occurrence의 size를 2에서 1로 돌렸다. 원본 `.25~.4초` 진행과 정규화된 Dynamic/size/alpha 시간이 다시 대응한다. |
| R/A Q51 | 원본 PS immediate32 11개 lane을 사용하는 10개 instruction을 교정했다. Q51 sprite의 원본 U/V/normal에 맞춰 tangentView.yz 부호도 교정했다. |
| R WR260 uniform | 편집 가능한 dissolve rotator의 sin/cos를 기존 CPU parameter builder에서 계산해 기존 packet의 미사용 row13에 전송했다. PS 본문은 유지하고 uniform prefix만 소비하도록 연결했다. |
| R WR259 정렬 | 원본 Required의 `psortmode_viewprojdepth`를 sprite 업로드에 연결했다. 한 요소의 입자를 camera clip-Z 큰 순서로 그리며 같은 깊이는 입력 순서를 유지한다. |
| S V63 | 네 핵심 sprite의 원본 native 수식·색·pivot·음수 size에 따른 UV 반전을 대조했다. 잘못된 기하 계산으로 재현되지 않아 추가 형상 보정은 하지 않았다. |

R tuning의 Q51 세 검격은 Q nailstrike에서 복사한 저작 요소이며 원본 R body의 직접 복원은 아니다.
원본 R full의 핵심은 WR208 spriteinvert 3개, WR259 sprite 3개, WR260 swing mesh 3개다.
WR208과 WR260에는 같은 PS 안에서 어두운 내부와 밝은 dissolve 경계를 계산하는 경로가 있다.
두 색을 반드시 별도 element 두 개로 만들어야 하는 구조가 아니다. 기존 unified/tuning 문서와
clean-core/solid-core 보강, 다른 작업에서 추가한 A cube 4개는 이번 연결 변경으로 덮지 않았다.

**확정한 texture 입력 차이와 복구 방법**

원본 Q51 normal과 S V63 noise는 `SampleBias(0)`으로 축소 mip을 선택하지만 기존 Resources
DDS에는 mip0만 있었다. 원본 UPK의 CRN/LZ4 mip payload를 기존 UModel LostArk v7의
동일 decoder로 회수했다. 별도 scratch package에 원본 mip 하나씩 노출하는 방식이며 원본
설치 패키지는 수정하지 않았다. generic Crunch decoder는 CRC가 맞아도 mip0 bytes가 달라
채택하지 않았다. 임의 downsample·재압축으로 만든 mip이 아니며 최고 해상도 압축 bytes는 같다.

| Resources 상대 asset ID | 설치한 원본 mip |
|---|---:|
| `Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_normal_bc5_09.dds` | BC5 512², 1→10 |
| `Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_009.dds` | BC1 128², 1→8 |
| `Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_014.dds` | BC1 128², 1→8 |
| `Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_021.dds` | BC1 128², 1→8 |
| `Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_caustic_tile_05.dds` | BC1 512², 1→10 |
| `Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_electric_008.dds` | BC3 128², 1→8 |
| `Effect/DimensionMaster/Textures/FX_TEX_04/fx_i_noise_03.dds` | BC1 512², 1→10 |
| `Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_environment_tile_02.dds` | BC1 512², 1→10 |
| `Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_cloud_021.dds` | BC1 128², 1→8 |
| `Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_030.dds` | BC1 256², 1→9 |
| `Effect/DimensionMaster/Textures/FX_TEX_05/fx_k_auratile_02.dds` | BC1 256², 1→9 |
| `Effect/DimensionMaster/Textures/FX_TEX_04/fx_i_atypical_03_1_ycl.dds` | BC1 512×256, 1→10 |

물리 위치는 모두 `Client/Bin/Resources/` 아래다. normal은 262,272→349,680B,
S noise 각 파일은 8,320→11,064B다. 기존 `Load_SourceTexture`의 DirectXTK loader는 파일에
들어 있는 mip을 모두 소비하므로 loader나 별도 resource manifest를 추가하지 않았다. Q51의
다른 mask `fx_d_atypical_094_ycl`은 원본이 명시적 LOD -1, 즉 mip0을 선택하므로 이번
SampleBias 입력 결함과 구분해 그대로 두었다. Drive 업로드나 Resources Git 추적은 하지 않았다.

위 마지막8개는 원본 R WR208/259/260 핵심9행의 추가 복구다. 원본 하위66mip을 회수했고,
실제 D3D11 resource/SRV의 mip 수와 BC1/BC3 sRGB 형식이 8/8 일치한다. R main trail007은
명시적 LOD -1이므로 그대로 두었다. 전체 교체는 12개 DDS, 원본 하위96mip이며 모두 mip0을
보존했다. 추가8개를 공유하는 저작 문서의 element 참조73개 중 R 외60개도 목록으로 기록했다.
차원술사 다른 스킬의 대안 문서와 창술사 Alt+V clip1의 auratile 참조가 포함된다. 모두 현재
실제 슬롯에서 활성이라는 뜻은 아니다. 원본 asset 교체가 공유 참조에 적용되는 범위다.

**실행한 수치·저장·재생 검증**

- 제품 QNative51과 원본 DXBC에 같은 PS 입력을 공급했다. 120조건/491,520픽셀에서 이전
  상대 오차 기준 초과 2,460픽셀이 0이 됐고 nonfinite 0이다. 원본 normal mip 설치 후에도
  다시 통과했다. 최대 절대 오차 RGB `9.54e-7`, alpha `2.12e-6`이며 bit-exact라는 뜻은 아니다.
- S V63도 원본 mip 설치 후 120조건에서 원본 PS와 RGB 차이 0, 최대 alpha 오차 `8.64e-7`,
  nonfinite 0 및 오차 기준 초과 0이다. 실제 D3D11 texture/SRV mip 수는 normal 10, noise 각 8이다.
- R WR208/259/260의 DXBC immediate32는 192개 lane 모두 현 disassembly의 float32 값과
  일치했다. Q51과 같은 epsilon 손실은 이 세 프로그램에서 재현되지 않아 수정하지 않았다.
- WR208은 실제 Playback 98 frame과 합성 60조건에서 현재 native PS와 원본 DXBC의 RGBA가
  일치했다. 실제 frame의 검은 내부 36,390표본과 밝은 경계 1,381표본을 포함하므로 빈 출력
  일치가 아니다. 실제 아레나 화면을 읽은 표본은 아니다.
- WR260의 기존 GPU sin/cos prefix는 actual 61 frame에서 원본 CPU uniform과 달라 오차
  기준 초과 72픽셀이 있었다. 같은 CPU uniform을 넣으면 raw PS 본문은 RGBA 차이 0이었다.
  CPU builder 연결 후 실제 builder의 packet으로 rotator 9값×61 frame, 총549조건을 비교해
  RGBA 최대 오차 0, 비유한 0, alpha coverage 549/549 일치를 확인했다. 유효출력 419조건,
  검은 내부 4,694표본·밝은 경계 20,749표본을 포함한다. 원본 uniform AST와 Windows ucrt
  float sin/cos가 기준이며 원본 게임 실행 중 CB를 캡처한 증거와는 구분한다.
- Q51 normal의 강제 mip0 샘플은 기존과 RG 차이 0이다. 128² 축소 조건의 원본 UV tile(0,6)에서
  이웃 R 값의 평균 차이는 `.0278999→.00863111`로 감소했다. 원본 filtering이 실제 입력에
  영향을 주는 증거이며 사용자 화면의 자글거림이 전부 해소됐다는 판정은 아니다.
- 원본 Q51 VS와 현재 rect의 좌표식을 대조했다. 21개 camera/rotation/scale 입력에서 교정한
  tangent 오차 0이다. 이 변경은 환경반사 UV에 쓰이며 alpha mask 변경은 아니다. 실제 제품
  `VS_MAIN`을 `vs_5_0`으로 최소 컴파일해 exit 0을 확인했다.
- 현재 소스와 SHA가 일치하는 실제 Codec/Playback 컴파일 객체로 R 23개와 S 26개의 Load,
  Validate_Drawable, scratch Save_Atomic 및 재Load canonical 일치, 전체 Stage를 통과했다.
  visible Solo 49/49의 Stage와 입자 생성이 성공했고 모든 재생 frame의 값이 유한했다.
  R 검사는 명시적인 identity anchor fixture 한 개를 사용했으므로 실제 bone 해석·화면과 구분한다.
- R260 세 occurrence의 관측 수명은 각각 `.336~.346 / .261~.339 / .280~.307초`다.
  시작 뒤 2/4/6.4초 Seek에서 모두 소멸해 이전 4~6.4초 유지가 남지 않음을 확인했다.
- 이 두 full 문서는 v13이다. v15 전용 projection을 요구하지 않고 direct document로 Stage한다.
  실제 projection=null Stage도 두 문서 모두 통과했다. 전체 Catalog 객체를 실행한 검사는 아니며
  Catalog→PresentationService→Object의 선택 경로는 현 코드로 별도 확인했다.
- 변경 JSON, project/filter XML과 1,553행 animevents를 parse했다. Catalog 182개 ID의 중복 0,
  R/S Resources 참조 각각 45/21개 누락 0, 실제 슬롯→clip→full asset 연결을 확인했다.

정렬은 원본 `Particle.Location`에 대응하는 World.translation을 View×Projection으로 바꾼
clip-Z를 사용한다. W, Z/W, quad 중심·pivot·cameraOffset을 사용하지 않는다. 원본 comparator에는
동일 깊이의 안정 순서 보장이 없어 같은 깊이는 입력 순서를 유지하는 strict comparator로 연결했다.
원본 R 한 호출의 emitter 순서 259→260→208과 현재 blend/depth/cull은 일치해 바꾸지 않았다.
서로 다른 호출 간 전역 정렬은 원본 근거가 충분하지 않아 변경하지 않았다. 기존 actual trace
64그룹 중 55그룹에서 생성 순서와 depth 순서 차이를 확인했다. 새 정렬 조건에 해당하는 현재
26개 sprite 요소에는 Orbit이 없어 key에 이미 합쳐진 Orbit offset 차이가 없다. renderer의
기존 코드와 인코딩을 보존한 부분 수정이며 out 전용 최소 C++ 컴파일은 exit 0이다.
현재 제품 helper 원문을 실제 particle 타입으로 컴파일한 66그룹/237입자 검사에서는 정렬 역전
165쌍이 0이 됐고, particle payload bytes와 전체 pointer 순열이 보존됐다. 같은 깊이, clip-Z 선택,
pivot/cameraOffset 독립성, 7개 적용 조건 및 4개 실패 이유도 확인했다. 근거는
`Shared/r_source_sort/probe_result.json`이며 실제 화면 비교로 확대하지 않는다.

수치 검사는 통제된 PS/geometry/depth 입력의 비교다. 원작 live vertex stream, 실제 camera·fog,
distortion/MRT 합성이나 최종 화면 전체의 일치 증거로 확대하지 않는다. 원본 S 첫 LOD 활성
발생에는 기존 손작업 helix/screw mesh가 없다. 이미지에서 보이는 주변 선을 특정 occurrence로
확정하는 일과 사용자의 최종 재생 확인은 남아 있다. 이번 49개에는 재생 실패로 제거할 요소가 없었다.

증거는 `out/DimensionMasterSRFocusedAnalysis20260909/` 아래
`Shared/q51_product_after_mips_verification.json`, `Shared/final_connection_verification.json`,
`Shared/q51_normal_mips/restore_receipt.json` 및 `dds_gpu_sampling.csv`,
`Shared/r_core_mip_restore/restore_receipt.json` 및 `actual_dds_srv.csv`, `S/source_mips/restoration_receipt.json`,
`S/v63_native_diff/mip_installation_sampling_result.json`, `S/runtime_validation/result_summary.json`과
`projection_result.json`에 있다. 기존 변경을 보존한 이번 적용 baseline은 `Shared/runtime_connection/`에 있다.
추가 PS/정렬 근거는 `S/wr208_native_diff/result_summary.json`,
`S/wr260_cpu_uniform_fixed_diff/result_summary.json`, `S/wr260_cpu_uniform_fix/actual_builder_packets.csv`,
`Shared/r_core_draw_order_audit.json`, `Shared/r_source_sort/renderer_compile.log`에 있다.

최소 셰이더 컴파일·구조 검사 후 최종 소스로 Debug Product 빌드를 시작했다. 직전 Product 성공
receipt `20260909T083046383Z-debug-product.json`은 이번 마지막 셰이더 변경 전 결과이므로
이번 변경의 최종 빌드 PASS로 사용하지 않는다. 새 빌드 결과는 아래 후속 기록으로 확정한다.
첫 후속 빌드는 확정된 정렬·CPU uniform 수정을 포함하기 위해 이 세션 소유 FXC만 중단했으며
그에 따른 MSB6006/exit 1은 `Shared/final_product_build.log`에 보존했다. 두 수정의 소스를
고정한 뒤 재시작한 빌드 로그는 `Shared/final_product_after_inputs.log`다.
Client/Server 및 UI는 실행하지 않았다. 시작 설정은 server-host이고 endpoint는 not-listening이다.
빌드 완료 후 사용자가 `Server + Client` profile에서 Ctrl+F5, Lobby→Character Select→차원술사로
들어가 R/S 및 A 비교 재생을 한다. Source/Resources 반영, 자동 검증과 사용자 visual 승인은 구분한다.

**사용자 질문에 따른 빌드 병목 조사 — 제품 설정 변경 없음**

직전 성공 Product는 50분14.853초였으며 shader 생성 시각 기준 Mesh 약18분, Particle 약31분이
차지했다. 두 번의 앞선 `/Od` Debug 실패는 X4505 임시 register4096 한도였고, 현재 두 파일만
`/Zi /O1 /T fx_5_0`으로 컴파일한다. `/O1`을 끄면 해결된다는 근거가 없으며 앞선 실제 실패와
반대되는 안내를 하지 않는다. 현 FXC는 한 작업 thread가 주 계산을 하고 두 큰 파일을 순차 처리한다.

MeshPreview/Particle 각각 unique include19개, 약6.3MB/10만7천 줄의 translation unit이며
원본 native PS 함수639개 정의를 포함한다. 보수적 텍스트 호출 graph는 Mesh336/Particle504개
native 함수에 도달하며 이는 최적화 후 instruction 수나 실제 pixel 실행 횟수와 다르다.
직전 완료 CSO의 PS 정적 instruction 수는 각각74,702/100,917, temp register17이었다.
Mesh7개/Particle5개 pass는 이미 VS/PS compile 객체를 공유하므로 pass 수만큼 같은 PS를 다시
컴파일하는 구조가 병목이라는 설명은 틀리다.

공용 native include의 작은 변경도 두 큰 shader 재생성을 요구한다. 다른 팀원의 fresh/clean/
rebuild 및 해당 include 변경도 같은 구조의 비용을 치르지만 정확한 시간은 PC별로 달라진다.
정상 증분 빌드에서 관련 shader/include/옵션이 같으면 무관한 C++·JSON·DDS 변경 때문에 이
두 shader를 다시 만들 필요는 없다. 완성된 EXE/CSO의 실행에는 컴파일이 필요 없다.

개선 후보는 재질군별 PS 분리와 공통 VS 유지, 지원되는 FXC 병렬 실행, compiler/include/옵션에
맞는 compiled output 재사용이다. 현재 복원 빌드는 유지하고 이 구조 변경은 구현하지 않았다.
임의로 최적화 옵션을 낮추거나 현재 빌드의 CSO를 오래된 파일로 대체하지 않는다. 정적 코드
규모만으로 실제 FPS/driver 생성 시간/시각 품질을 단정하지 않는다. 빌드 옵션·앞선 실패 근거는
Composition Workbench RESULT G28 및 `out/EffectSequencerComposition20260909/shader_pressure/`
의 `pressure_20260909T083106.json`에서 재확인했다.
10만7천 줄은 주석 포함 물리 소스이며 조건부 필터 후 비주석·비공백 줄은 각각67,837/67,956이다.
이번 include/call-graph 조사 원문은 `S/shader_compile_structure/README.md`와 `result.json`이다.

진행 중인 Particle FXC PID37108의 실행 환경도 읽기 전용으로 확인했다. 우선순위는 Normal,
affinity는 20 logical CPU 전체이며 명시적인 ProcessPowerThrottling 플래그는 없었다.
12.17초 표본에서 CPU 시간은 2.39초 증가했고 CPU 대기열0, 가용 메모리 약35GB,
page input/read0이었다. 해당 짧은 표본에는 시스템 전체 포화 근거가 없지만 FXC는 Job에
속하며 그 Job의 CPU rate 제한은 확인하지 못했다. 따라서 낮은 CPU 사용률의 환경적 원인은
미확정으로 남기며 소스 규모만으로 이번 PC의 실제 소요 시간 전부를 설명하지 않는다.
우선순위·affinity·전원·Job 설정과 현재 빌드는 변경하지 않았다. 측정 근거는 같은 보고서에 있다.

**G36 최종 복원 Product 성공 — 2026-09-09 19:22 KST**

고정 소스의 최종 Debug Product는 exit0, Engine/Shared/Server/Client 모두 PASS,
missingRuntimeInputs0이다. receipt는 `out/BuildPipeline/runs/20260909T102254939Z-debug-product.json`,
소요 시간은70분32.240초다. Mesh CSO는18:38:19, Particle CSO는19:21:52,
Client.exe는19:22:54에 생성됐다. EXE49,826,304B, Mesh29,453,593B, Particle30,793,734B다.
검토한9개 소스/데이터 SHA가 빌드 시작 기준과 같고 Engine DLL의 Engine/Client 배포 SHA도 일치했다.
기존 encoding/PDB 경고는 남지만 컴파일·링크 오류는 없다. Client/Server/UI는 실행하지 않았다.

사용자가 이어서 승인한 셰이더 분리·Play All 성능 개선의 변경 전 기준으로 이 결과와 EXE/CSO를
`out/EffectBuildPlayAllOptimization20260909/Baseline/final_restore_build.json` 및 `Binaries/`에
보존했다. 이후 구조 변경은 별도 성능 개선 IMPLEMENTATION_PLAN/RESULT가 소유하며,
이번70분 빌드를 그 구조 변경의 최종 PASS로 재사용하지 않는다. 사용자 visual 승인과 S 주변 선의
정확한 occurrence 대응, Resources12개 DDS의 Drive 전달은 여전히 별도 확인이다.

## G37. R 보라 경계·F1 D 세로 잘림·S 검격 후속 반영 — 2026-09-10

사용자는 D 세로 잘림을 **F1 Effect Tool full restore**, Alt V 중단을318개 전체 full 문서에서
확인했다고 답했다. 실제 R2050180/S2050220 full과 F1 D2050240 clip1 full을 각각 수정했다.
사용자 손튜닝 unified와 D gameplay binding은 보존했다. 다음은 소스·수치 검증 완료 상태이며
사용자 아레나의 최종 시각 승인은 아니다.

### R: 검은 본체와 보라색 보조층, 후처리의 구분

WR208/259/260 중심·dissolve 경계9행은 이전 G36 복구가 연결된 상태였다. 검은 내부와
경계는 이 검격 재질의 합성 결과이며 dust 한 장으로 전체를 설명할 수 없다. 이번에 확인한
직접 누락은 Body02 emitter109/110이 세 번씩 호출되는 **보라 additive 보조층6개**다.
이미 복구된 ALTV168/120과 MIC/PS/VS/VF·parameters·samplers가 같은 원본이라 기존 renderer로
연결했다. RGBNoise2·ZoomBlur1·point light1도 원본 occurrence를 회수해 R full을23→33행으로
확장했다. 기존23행의 parsed 구조는 모두 보존했다. R tuning의 Q51 purple-rim 보강은 별개다.

마지막 Solo 정리 전 R24행에도 body6/post3은 이미 없었다. 마지막 정리가 이9행을 없앴다고
단정하지 않으며, 해당 정리에서 없어진 R 요소는 light1이었다. 실제 DDS mip0와 원본 보존식의
정적 fixture에서 ALTV168/120의 보라 출력 texel16,378/38,192개를 확인했다. 이는 누락층의
색 기여 근거이며 전체 원작 장면의 합성·아레나 픽셀이나 모든 경계 문제의 단독 원인 판정이 아니다.

모든 ID는 `authored.source-particle.full-r.` 뒤 suffix다. Body02는 `fx_pc_swp_03.par_s_swp_foldcut_body_02`, post는 `fx_post.fx_par`, light는 `fx_cm_02.light` 아래 원본이다.

| ID suffix | 정확한 원본 emitter/notify | 시작·입자 수명 | 역할·보라 기여 | 이전 제외/이번 복구 |
|---|---|---|---|---|
| b5b3de36094d6e103c71 | Body02.particlespriteemitter_109 /019 | .5/.25초 | RGB-split ring; particle RGB(1.2,.8,2), additive 보라색 보조층 | 기존 재사용 ALTV168 연결. raw Orbit options 복구 |
| c4a013355bb86791e8c7 | Body02.particlespriteemitter_109.event_source-event-014 /028 | .7/.25초 | 같은 source ring 두 번째 호출 | 같은 ALTV168/원본 timing 유지 |
| 283806938c0aec209f01 | Body02.particlespriteemitter_109.event_source-event-018 /034 | 1.05/.25초 | 같은 source ring 세 번째 호출 | 같은 ALTV168/원본 timing 유지 |
| 8d501451826e59d002e5 | Body02.particlespriteemitter_110 /019 | .5/.2초 | emissive 무늬 sprite; RGB(6,2.5,10), additive 보라층 | 기존 재사용 ALTV120 연결 |
| 4a7ef8f50ea5988521bf | Body02.particlespriteemitter_110.event_source-event-014 /028 | .7/.2초 | 같은 source 무늬 두 번째 호출 | 같은 ALTV120/원본 timing 유지 |
| 16b2767c51274d8ce247 | Body02.particlespriteemitter_110.event_source-event-018 /034 | 1.05/.2초 | 같은 source 무늬 세 번째 호출 | 같은 ALTV120/원본 timing 유지 |
| 41426adbb82bc4fdd49a | fx_post.fx_par.par_j_rgbnoise_01.particlespriteemitter_11 /009 | .1/.45초 | 장면 R/B sample 좌표를 반대로 이동시키는 색수차. 보라 emissive를 자체 생성하는 재질 아님 | 기존 ALTV156 HDR scene/depth callback 연결 |
| 65edff9af0e5d770419a | 위 RGBNoise emitter.event_source-event-020 /036 | 1.1/.45초 | 두 번째 색수차; 채널 상수 R+.001/G0/B-.001, Dynamic powerx1~5 | 같은 ALTV156/원본 발생 유지 |
| 0476d3b62e94a1e2ac9e | fx_post.fx_par.par_j_zoomblur_01.particlespriteemitter_0 /037 | 1.1/.15초 | 원형 mask에서 중심 방향으로 장면을 다중 sample. Dynamic blur0→1.5→0; 자체 보라색 없음 | 기존 ALTV155 prepared post 연결 |
| 6f0a91b941d9dba6cb87 | fx_cm_02.light.par_mp_light_01.particlespriteemitter_2 /038 | 1.1/.35초 | point light, source constant RGB(1.5,.5,2), 주변 표면 조명. unlit 핵심 PS의 경계 색을 직접 만드는 층 아님 | 밝기/반경 입력이0이고 disabled라 제거됐던 항목. 실제 component/CDO로 typed light 복구 |

body109 MIC는 `fx_m_mi_j_00.fx_mi.fx_j_rgbsplit_01_01_ad`, PS d880c361f16370468cfdac8d4688d393, VS5825675b4ffbc840ad691ec56973cf7e이며 texture는 `Effect/DimensionMaster/Textures/FX_TEX_03/fx_e_ring_005.dds`다. 3개 위치 sample의 R 채널을 서로 다른 RGB 비율로 합치고 desaturation=-1.5/bright=.9와 particle color/alpha를 곱한다.

body110 MIC는 `fx_m_mi_o_00.fx_mi.fx_o_pa_ri_04_ad_2s`, PS096d7ee0efa1eb4bb2e91b406fe61913, VS2dd6d96a7e6c974fac82106409a5b9b8이다. `FX_TEX_03/fx_e_atypical_031.dds`를 emissive_desaturation1, emissive_power.1, 원본 Dynamic과 particle color/alpha로 계산한다. 위 두 source MIC의 현재 selected blend/cull은 additive one-sided이며 이름의 `_2s`만으로 뒤집지 않았다.

RGBNoise MIC는 `fx_m_mi_j_00.fx_mi.fx_j_po_rgbnoise_01_01_tr`, PS9f4cdbbbab89f745927fe4d13bb5e5a6. ZoomBlur MIC는 `fx_post.fx_mi.fx_c_pa_zoomblur_01_tr`, PS3fc4c0de7f119c49b1e0478e97872fc9. 이미 연결된 ALTV156/155 native 후처리 소비자를 사용하며 generic filmnoise 수식으로 대체하지 않는다. detail의 profile enum은 기존 native callback을 보관하는 carrier 설정이다.

109 Orbit의 raw options를 설치 UPK에서 회수해 offset spawn/update=false와 회전 CDO 옵션을
typed bool9개로 연결했다. Distribution=null인 회전은 잘못 상속한 cached 값을0으로 교정했다.
Light의 실제 brightness10/radius200cm/RGB(1.5,.5,2)/life.35초를 연결했다. Spawn rate0이어도
burst1은 살아 있으므로 rate0을 전체 비활성으로 해석하지 않는다. 원본 EF Size→radius 세부 갱신과
전체 attenuation 일치는 아직 입증하지 않았다. post life .45/.15초도 기존 typed clock에 연결해
임시1초 수명으로 Dynamic 곡선이 늘어지지 않게 했다.

### D: 본체의 size0/alpha0과 보조 dust의 UV 이중 이동

D full25행에서 실제 주검격 `fdb494d22f7cc4b36319`/fold 계열 WR259의 startsize와 alpha 분포가
비어 있었다. 원본은 bCanBeBaked=false이므로 빈 cooked LUT 자체가 손상은 아니며, 원본 객체의
값을 추가로 회수해야 했다. `DistributionVectorUniformRange`의 MaxHigh(90,50,0),
MaxLow(80,90,0), MinHigh(-90,50,0), MinLow(-80,90,0)cm를 복구했다. 원본은 먼저 두 범위
중 하나를 고르고 XYZ를 보간한다. min/max(-90..90) 하나로 합치면 존재하지 않는0폭을 생성한다.

기존 CEffectDistribution operation4에 prefix2+원본4벡터12값을 저장하고 Playback이 selector→XYZ
순서로 난수4개를 소비한다. class/path/component3/축고정없음/14개 유한값/정해진 chunk와 시간/
keys없음을 기존 Codec Validate 위임에서 검사한다. operation0~3와 별도 reconstructed source
projection의0~3 계약은 그대로다. alpha는 (0,0),(.172951713,1),(1,0)의 실제 cubic keys와
tangents를 회수했다. 원본 Required의 image-flipping은 기본false여서 제품false를 유지했다.

동일 결함의 보조 오라 `b2cf8ed335fe371059f4`/`f8652338309496891cd8`도 원본 X±104~116,
Y120~144cm의 두 범위로 교정했다. dust `ad1824…`/`85f2ab…` 두 행은 native 재질이 자체 UV를
이동하는데 generic UV speed(.6,.4)까지 더해 clamp alpha mask가 이동·잘리던 중복을0으로 고쳤다.
실제 DDS의 t=.5 경계 alpha 최대값1→.00607 변화가 중복 이동의 잘림 근거다. main Q51/SD shader를
근거 없이 수정하지 않았고 D의 SD320~332 소비는0이다. 기존 hidden `0fd0fc7aa0f6bebd715a`
줌블러1개도 사용자의 비활성 상태로 유지했다. 결과는25개 중24개 실행이며 새 Resource는 없다.

원본 분기 근거는 UE3 [UnDistributions.cpp](https://github.com/CodeRedModding/UnrealEngine3/blob/main/Development/Src/Engine/Src/UnDistributions.cpp)의
UDistributionVectorUniformRange::GetValue와 실제 설치 UPK 객체다. D 원본PS·5개 변경행·distribution
객체 receipt는 `out/DimensionMasterRDSAltV20260910/D/D_SOURCE_PATCH_RESULT.md`에 보존했다.

### S: 전체30요소 연결과 중심 검격만2배 조정

시작 SD320~323, 기존 sprite24+mesh2를 보존했다. V63 주검격4개의 원본 quad는 첫쌍 .90×4.05m,
후반쌍 .54×4.05m이며 실제 PS alpha>.05 footprint는 폭.26~.37m/길이2.12~2.47m였다.
사용자 허용 범위에서 이4행의 sourceScale.size만2로 바꿨다. 같은 mask 기준 폭.51~.73m/
길이4.24~4.94m이며 원본 크기 그대로의 복원이라는 뜻은 아니다. 원본 source 분포·색·timing·
pivot·회전·속도와 큰 원판은 보존했다.

누락 Light1/RGBNoise2/ZoomBlur1을 S 고유 발생과 수명으로 재연결해26→30행으로 만들었다.
RGBNoise ALTV156은0초/.3life와.393503초/.4life, ZoomBlur ALTV155는.39594초/.3life다.
Light는.39594초/.4life, 원본 brightness10/RGB(2,3,5), 기존 프로젝트 range3m/falloff2를 사용한다.
광원의 원본 radius/attenuation 전체 일치를 주장하지 않는다. 후처리는 R과 같은 원본 shader지만
S의 FX_State_01/bip001-spine2 follow는 기존 owner bone 경로가 제공한다.

### 자동 검증과 실행 인계

| 확인 | 실제 결과 |
|---|---|
| R/S 실제 Playback | 명시적 owner anchor fixture와60Hz0~6초.38,397개 출력, 신규14개 모두 출력, 비유한값0. R추가 body6 모두 size/alpha양수 |
| D 원본 분포·실제 재생 |75,638검사/실패0. selector/XYZ16,384seed·보조분포2,048샘플·잘못된입력12개 거부.22particle행+2post행 출력, 본체18샘플 중17alpha양수 |
| S 크기와 PS | 전후633샘플 중 V63 56개만2배/나머지577동일. PS56×depth3=168조건 비유한픽셀0. 색·시간·pivot·flip·velocity 보존 |
| 실제 Codec Save/Load/Solo | R33/S30/D25/AltV318 네 문서406행 Save왕복 동일. visible405/405 Solo통과, 실패·locked·unexpected0. 기존D hidden1은 의도적으로 보존 |
| Alt V 공용 회귀 |318요소·2,060 frame/state checkpoint CSV byte-identical. 성능 상세는 별도 G06 |
| Client 최소 컴파일·링크 | Client/Default/Client.vcxproj ClCompile 종료0, 별도 OutDir의 실제 제품 _Link 종료0. HLSL/Resource 변경은 없어 FX 재컴파일 없음 |

Codec probe의 exit1은 기존 hidden D1도 실패로 합치는 기존 probe 정책 때문이며 Save/Solo실패는0이다.
R/S 최초 누락은 검사에서 follow anchor를 제공하지 않은 결과였고, D 최초 flip 검사 기대는 원본
defaultfalse와 달라 바로잡았다. 제품에 identity anchor fallback이나 임의 image flip을 넣지 않았다.

새 EXE는 `out/DimensionMasterRDSAltV20260910/linked/Client.exe`, 50,576,384byte,
SHA256 `8fd3a16b75c7b4d66b9a27a0c33eb605f4a853fd2cf4038018a8aa708e17d557`다. 앞선3개 class full restore의 변경을 포함한 현재 Client 객체로 링크했다.
현재 Client PID55640/Server PID44208이 실행 중이므로 정식 `Client/Bin/Debug/Client.exe`는
교체하지 못했다. D의 operation4는 새 EXE가 필요하므로 기존 실행본의 JSON reload만으로 완료되지 않는다.
사용자가 Client 종료 후 Debug|x64의 Client를 빌드·Ctrl+F5로 재실행하면 현재 Server를 사용한다.
Server까지 종료한 경우 이 PC는server-host이므로 Server + Client profile을 사용한다.
F1 Effect Tool에서 R2050180 full, D2050240 clip1 full, S2050220 full, AltV2050540 full을 확인한다.
에이전트는 Client/Server/UI를 실행·종료·조작·캡처하지 않았으며 사용자 시각 승인은 남았다.

`out/DimensionMasterRDSAltV20260910/integration-result.json`과 각 subfolder 로그가 증거다.
변경 JSON parse·project XML parse와 최종 git diff 검사 결과는 아래 마지막 통합 확인에 기록한다.

최종 통합 확인: 변경 JSON과 Alt V 총4개 parse, 현재 Client project/filter XML2개 parse,
저장소 전체 git diff --check가 모두 성공했다. 검증한 제품 소스5개·변경 JSON3개의 SHA와
현재 파일이 일치하며 새 EXE SHA도 receipt와 같다. Resources 추가와 Git stage/commit/push는 없다.

## G38. DimensionMaster BA 목록·제품 연결과 A 높이 — 2026-09-10

### 반영한 데이터

`DimensionMaster.animevents`의 요청 슬롯을 BA 네 단계 full, Q tuning, W/E full, R tuning,
A/S/F/D/V/AltV full로 연결했다. 요청한 14개 aggregate 문서는 각각 실제 첫 clip의 0ms에 한 번
시작한다. W/E/D full 생성기가 `cue.globalTimeSeconds`를 detail timing에 저장하므로 분리 clip마다
같은 full을 다시 생성하지 않는다. 기존 분리 Product cue 25개를 14개로 바꾸고 header를 실제
1542행으로 계산했다. T 2050500의 기존 cue, source/orig·SOUND/HIT 행과 skillbindings,
PlayerSkills 및 combo 단계는 유지했다. EffectCatalog는 누락된 exact Product 13개만,
EffectResourceTree는 누락된 BA full 4개와 D/V full 2개만 추가했다.

A full은 두 번째 0.600000024초 발생의 Y=0.800000012를 기준으로 나머지 세 발생의 검격·crack·cube
등 함께 배치된 36요소 Y만 맞췄다. 기존 X/Z·rotation·scale과 source module은 유지했다.
sprite carrier 34개의 detail startDelaySeconds에는 각각 0.2초를 더했다. mesh cube의 발생 시간
0.25/0.600000024/0.899999976/1.29999995초는 유지한다. A 전체 요소 수는 66개다.
R tuning 사용자 저장 파일은 편집하지 않았으며 작업 시작 시 bytes SHA는
`419219079911aaa39fa366446c2a4387ee3893bc1bb0e8189b840d9677c25d4d`였다.

### BA 번호와 실제 원본 대조

기존 DisplayName과 내부 단계 이름은 0부터 시작한다. 아래 연결을 유지했으며 legacy audition의
이전 이름을 Product 단계로 오인해 재배치하지 않았다. 사용자가 목록의 BA1/BA2가 같았다고 기억한
부분은 실제 동일 normalatk_1_1 원본을 쓰는 아래 두 행과 일치한다. A를 축소 복제한 새로운 BA는 만들지 않았다.

| 기존 DisplayName | combo stage | 실제 animevent clip 끝 | Product full ID 끝 | 원본 핵심 구성 |
|---|---:|---|---|---|
| 이펙트_차원술사BA0_전체 | 0 | att_battle_1_01 | 2050010.ba0.full.restore | normalatk_0_1, cone/plane, scale 1 |
| 이펙트_차원술사BA1_전체 | 1 | att_battle_1_02 | 2050010.ba1.full.restore | normalatk_1_1, fm_h_swing_01/02/03, scale 1.1 |
| 이펙트_차원술사BA2_전체 | 2 | att_battle_1_03 | 2050010.ba2.full.restore | normalatk_1_1, 같은 swing 3종, scale 1.2 |
| 이펙트_차원술사BA3_전체 | 3 | att_battle_1_04 | 2050010.ba3.full.restore | normalatk_3_1 검격 2회와 core localcrack, swing scale 1.2 |

legacy `ba1.unified → ba3.restore`, `ba2.unified → ba0.restore`, `ba3.unified → ba2.restore`는
읽기 전용 출처를 가진 별도 audition 관계로 유지한다. 이 3개 donor는 CRLF checkout 때문에 저장된
sourceDocumentRawSha256와 달라 All Effects의 freshness gate에서 열기가 거부됐다. 각 파일을 LF로만
정규화하면 기존 hash와 정확히 일치했다. 기존 JSON 내용과 pin은 수정하지 않고 LF bytes를 복구했으며
`.gitattributes`에 `/Data/Effects/Authored/effect.dimensionmaster.skill.2050010.ba*.restore.effect.json text eol=lf`
한 줄을 추가했다. EOL 수정 3개의 JSON diff는 없고 기존 hash 검증도 완화하지 않았다.

### 실제 수행한 검증

- BA 기존 단계 회귀 검사 7개 PASS. Product full 연결 계약과 G33에서 이미 제거된 미지원 행 이후의
  15/12/7/25개 요소 수로 기존 검사를 갱신했다.
- 현재 Debug obj 기반 focused probe를 컴파일·링크했다. 실제 SourceIndex Build의 freshness,
  Open의 catalog provenance 검사, legacy 3개 Codec Load가 모두 PASS다.
- 실제 DimensionMaster 모델의 154개 clip으로 AnimationEffectCueDocument Load를 실행해 15개 cue,
  unavailable 0개를 확인했다. 요청 14개 모두 실제 Catalog Capture/Stage와 Codec Save/Reload 후
  정규 Serialize 일치를 확인했다. 저장 대상은 out 사본이며 사용자 원본은 Save하지 않았다.
- BA full 4개는 Codec Load, GPU Stage, CPU Playback 300 tick에서 유한 transform을 확인했다.
  BA full 4개와 수정한 A full은 실제 DimensionMaster 모델/bone을 사용한 offscreen WARP Render를
  각각 75 frame 실행했다. draw failure와 nonfinite pixel은 0이었다. 256×256 float target의 최대
  비영 픽셀 수는 BA0 2515, BA1 425, BA2 519, BA3 1623, A 7002였다. 이 수치는 제출 경로 확인이며
  색·모양·높이의 사용자 육안 승인이 아니다. 이미지 저장이나 화면 캡처는 하지 않았다.
- 변경 JSON parse와 해당 파일의 git diff --check를 통과했다. 제품 C++ 변경·전체 제품 빌드·
  Client/Server/UI 실행·stage/commit/push는 수행하지 않았다. 쿠크 props의 이전 dirty 작업은 보존했다.

증거는 `out/DimensionMasterFullRestore20260910/`의 `data_changes.json`, `line_ending_repair.json`,
`ba-source-comparison.log`, `ba-tests.log`, `product-audit.log`, `ba0..3.draw.log`, `a.draw.log`와
`before/` bytes backup에 있다. 이전 probe의 리소스 root/CSO 설정 누락에 따른 실패는 환경 교정 후
재실행했으며, 누락으로 잘못 판단했던 Resources는 수정하거나 복제하지 않았다.

### 남은 실행 경계

위 결과는 데이터와 focused runtime 경로까지다. 실행 중인 Client의 기존 catalog/Effect instance에는
자동 소급 적용하지 않았다. 사용자의 저장·종료 뒤 root가 정식 실행 파일과 전체 데이터 연결을
통합하고, 사용자가 직접 All Effects의 기존 BA0~3_전체와 제품 평타 1~4타, A 및 요청 슬롯을 확인해야 한다.


## 2026-09-10 최종 Product 통합 확인

사용자 마지막 Save/종료 뒤 최신 원본에 통합하고 관련 publisher와 정규 Debug Product 빌드를 완료했다.
Engine/Shared/Server/Client 컴파일·링크·EXE/DLL/셰이더 배포는 PASS이며 실행 입력 누락은0이다.
이 기록은 위의 Product 통합 대기 상태를 갱신한다. 세부 게시 revision·새 Server 검사·남은 사용자
화면 확인은09-10 KOUKU_PATTERN_EFFECT_ANCHOR_FEAR_AUTHORING_IMPLEMENTATION_RESULT의 G10에 있다.
빌드 근거: `out/BuildPipeline/runs/20260910T091016153Z-debug-product.json`. Client/UI 실행·캡처와 최종 육안 승인은 수행하지 않았다.
