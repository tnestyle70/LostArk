# 마리오 색 공 미표시·맵 재질 조사 및 수정 실행계획

작성: 2026-09-14. 대상: 1~4마리오의 색 공 표시와 사용자가 지적한 맵 재질.

## G00. 현재 반영 상태와 인계 범위

이 문서는 **원인을 확정한 뒤 해당 경로만 수정하는 실행계획**이다. 현재 미표시의 최종 원인은 미확정이다. 검증되지 않은 C++ 교체 코드를 정답으로 제시하는 구현 명세가 아니다. 구현자는 G01~G03에서 원인을 재현한 다음, 이 문서를 갱신해 실제 수정 대상 H/CPP의 적용 후 전체 코드와 데이터 변경 블록을 보존하고 구현한다. 현재 코드 스냅샷을 통째로 덮어쓰지 않는다.

사용자는 예전에는 보이던 빨강·파랑 등의 공이 현재 플레이와 자유시점 양쪽에서 보이지 않는다고 보고했다. 팀장과 사용자는 마리오 공·맵 재질 복원에 문제가 있다고 보고했다. **사용자 보고를 단순히 맵 미리보기라서 생긴 정상 현상으로 처리하지 않는다.** 조명·재질 변경과 발생 시점의 연관성은 조사 출발점이지, 조명이 공을 지웠다는 확정 증거는 아니다.

조사 기준 저장소는 `C:/Users/USER/source/졸업팀폴/LostArk`다. 계획을 작성한 Codex worktree `C:/Users/USER/.codex/worktrees/7395/LostArk`의 제품 소스는 현재 실제 저장소보다 오래될 수 있으므로 구현 기준으로 사용하지 않는다. 다른 PC에서는 실제 사용자가 실행하는 프로젝트 경로를 확인한 후 상대 경로를 대응시킨다.

현재 실제 저장소는 main이며 다수의 타 작업 변경이 있다. 자동 reset/stash/checkout/commit/push, 전체 파일 덮어쓰기, 강제 리소스 설치를 하지 않는다. 카드미로 문양·렌더 순서 수정, 컷신·카메라, 다른 관문의 조명과 authored sequence를 보존한다. 팀 브랜치 절차를 따르되 사용자의 dirty 작업을 임의로 이동시키지 않는다.

### 확인된 사실

| 항목 | 확인 결과 | 이 결과만으로 확정할 수 없는 것 |
|---|---|---|
| 색 공 모델 | RedStarBall/BlueBall/YellowBall WModel 존재 | 실제 실행 EXE에서의 draw 성공 |
| 원본 색 공 배치 | runtime mapplacements에 117개, 모두 기본 visible=0 | 플레이 중 runtime Visible도 0인지 여부 |
| 배치 시퀀스 | 4개 stage × 3개 case, 총 12개 존재 | 실행 중 document가 성공적으로 commit됐는지 |
| instance/template/MAP target 연결 | 조사한 runtime 문서의 해당 ID 누락 없음 | 모델·Deploy·효과를 포함한 전체 런타임 admission 성공 |
| 재질 연결 | 네 공 모두 `source.character.monster-be5bc0ded311.v1`, program 25 | 설치 CSO와 현재 코드의 일치, 실제 GPU 결과 |
| 텍스처 | 네 공의 native 재질이 참조하는 필수 파일 존재 | SRV 생성·바인딩 성공 |
| 모델 재질 이름 | 네 WModel 모두 `mn_ppcc_00_mi` 문자열 확인 | 모델 로더가 적용한 최종 CMaterial 상태 |
| 색 diffuse alpha | d1/d2/d3와 줄무늬 c의 최상위 mip alpha 최소·최대가 모두 255 | 다른 마스크·draw 단계의 동작 |
| variation 입력 | 네 공의 mask_variation_visible=[0,0,0,0], null.dds는 RGBA 모두 0 | 실제 바인딩이 이 파일·상수를 사용했는지 |
| 렌더 경로 변화 | 9/11 `359412c4`에서 native source 재질을 static batch 대상에서 제외 | 이 변경 자체가 결함이라는 결론 |

따라서 텍스처가 투명하게 저장됐다는 설명, 데이터가 삭제됐다는 설명은 현재 입력과 맞지 않는다. 반대로 Visible 조건만으로 재질·렌더 결함을 배제해서도 안 된다.

### 사용자 첨부 재현 입력

- `C:/Users/USER/OneDrive/사진/Screenshots/스크린샷 2026-09-14 140854.png`: 붉은 달 배경, 줄무늬 공 3개가 보이는 장면.
- `C:/Users/USER/OneDrive/사진/Screenshots/스크린샷 2026-09-14 140904.png`: 카드 구조물 맵.
- `C:/Users/USER/OneDrive/사진/Screenshots/스크린샷 2026-09-14 140911.png`: 매달린 카드 길과 줄무늬 공이 보이는 장면.

이 스크린샷만으로 단계 번호, 서버 snapshot 값, 흑백 카드 재질의 오류 여부를 단정하지 않는다. 원본 마리오 영상/추출 자료와 같은 장소를 맞춘다. 이전에 전달된 2·3관문 진입 컷신 영상은 마리오 맵 재질의 정답 영상이 아니다.

## G01. 실제 EXE·리소스·저작/배포 입력 고정

수정 대상 파일을 정하기 전에 다음을 읽고 작업 시작 상태를 기록한다.

1. 실제 저장소의 AGENTS.md, CLAUDE.md, gotchas, TEAM README, 재질 복원 V2와 관련 PLAN/RESULT를 읽는다.
2. `git status --short`, branch, 현재 diff를 확인한다. fetch 등 규칙상 작업이 권한 문제로 실패하면 성공한 것으로 쓰지 않는다.
3. 사용자가 실행한 Client 경로·구성·작업 디렉터리와 연결한 Server의 버전을 확인한다. 바로가기 이름이나 EXE timestamp 하나로 최신 빌드를 판단하지 않는다. 설치기를 실행해 로컬 결과를 배포 ZIP으로 되돌리지 않는다.
4. 실제 소비하는 Resources root와 DataFiles root를 코드/실행 정보로 확인한다. 진단 기록에 절대 경로를 남기되 JSON에는 Resources 상대 ID만 유지한다.
5. 관련 저작 문서와 runtime 출력을 의미 단위로 비교한다. 줄바꿈 차이를 재질 변경으로 집계하지 않는다.
6. 기준 스냅샷은 out 아래의 이번 작업 전용 폴더에 보존한다. 이미 있는 백업을 덮어쓰지 않는다. 이 스냅샷은 팀 Resources 배포 manifest나 hash admission 조건이 아니다.

### 핵심 파일

아래 경로는 실제 저장소 루트 기준이다. 이 목록은 조사 범위이며 모든 파일을 수정하라는 지시가 아니다.

| 경로 | 소유하는 정보/책임 |
|---|---|
| `Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapassets` | MarioProps의 asset ID, 모델, 기본 render profile |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapplacements` | 배치 ID, TRS, authored visibility |
| 같은 폴더의 `.worldsequences.json` | 단계별 3종 원본 배치, 표시 키, 줄무늬 공 motion |
| 같은 폴더의 `.mapmaterials.json` | asset/slot별 native 재질·상수·텍스처 |
| 같은 폴더의 `.maplights.json` | 맵 광원과 수광 대상 |
| `Data/Rendering/Authored/RenderingProfiles.json` | 후처리·환경·지역 설정 |
| `Client/Bin/DataFiles/Map/`의 대응 문서 | 게시된 실행 입력; 직접 편집 금지 |
| `Data/Actors/BossCatalog.json` | 모델 ID별 공유 원본 재질 override |
| `Client/Private/Level_KakulSaydonArena_WorldObjects.cpp` | 마리오 배치 선택·공 표시·reload |
| `Client/Private/WorldSequencePlayer.cpp` | document 준비·commit·시퀀스 표시 적용·완료 pose |
| `Client/Private/MapPlacementRuntime.cpp` | 개별/배치 렌더 생성과 runtime visibility |
| `Client/Private/MapAssetObject.cpp`, `MapAssetRenderUtils.cpp` | cull·render group·재질 bind·draw |
| `Client/Public/SourceCharacterMaterialParameters.h` | native family의 program/상수/입력 mask |
| `Engine/Private/Material.cpp`, `Renderer.cpp` | SRV·원본 재질 bind·재질별 조명 패스 |
| `Engine/Bin/ShaderFiles/Shader_SourceCharacterBaseGroup025.hlsli` | program 25의 실제 pixel discard·출력 |
| `Server/Private/GameRoom_KoukuPlayerCommands.cpp` | 서버 단계 진입·배치 case 선택 |
| `Server/Private/GameRoom_Replication.cpp`, `Client/Private/CombatHUDViewModel.cpp` | stage/layout/pop mask 전달 및 소비 |

기존 결과 문서는 `.md/GB/09-09/2026-09-09_MARIO_ORIGINAL_PLACEMENT_RESULT.md`, `.md/GB/09-11/2026-09-11_KOUKU_BOSS_PROP_NATIVE_MATERIAL_IMPLEMENTATION_RESULT.md`, `.md/GB/09-11/2026-09-11_KOUKU_MAP_LIGHTING_IMPLEMENTATION_RESULT.md`를 참고한다. 오래된 문서의 프로토콜 번호나 프로그램 설명을 현재 코드보다 우선하지 않는다. 특히 결과 문서의 마리오 program 예시는 현재 packing의 program 25와 대조해서 읽는다.

## G02. 색 공 하나의 표시부터 draw까지 원인 분리

### 실제 호출과 상태 소유자

`Update_MarioControlState / Begin_MarioStageObjects`
→ Server snapshot `iMarioStage`, `iMarioLayoutVariant`, `iMarioPoppedBallMask`
→ `CCombatHUDViewModel`
→ `Update_MarioLayoutPresentation`
→ `CWorldSequencePlayer::Play / Update / Apply_Instance`
→ `CMapPlacementRuntime::Set_RuntimeVisible`
→ `CMapAssetObject::Late_Update / Render_Group`
→ `CMapAssetRenderUtils::Bind_Material`
→ `CMaterial::Bind_SourceCharacter`
→ 실제 셰이더·메시 draw.

클라이언트가 서버의 단계·case를 임의로 만들어 공을 켜는 우회를 금지한다. 기본 숨김 117개를 전부 true로 바꾸면 서로 다른 case가 겹치므로 금지한다.

### 대표 배치

| 대상 | asset ID | 1마리오 case 1의 대표 placement ID |
|---|---|---|
| 빨강 | `MAP_MARIO_RED_STAR_BALL` | `15849624984520486748` |
| 파랑 | `MAP_MARIO_BLUE_BALL` | `12454274333077724228` |
| 노랑 | `MAP_MARIO_YELLOW_BALL` | `12984898520043161401` |
| 비교용 줄무늬 | `MAP_MARIO_STRIPED_BALL` | `9440738451765240953` |

이는 조사 시점에 실제 존재한 ID다. 실행 case가 2/3이거나 다른 stage라면 **선택된 instance의 bindings에서** 대표 ID를 다시 구한다. case 1을 강제로 적용해서 실행 진단을 왜곡하지 않는다.

### 진단 수집

기존 로그/디버거로 아래 값을 같은 재현에서 수집한다. 정보가 부족하면 해당 함수에 Debug 한정·대상 ID 한정의 최소 진단을 추가한다. 에이전트가 Client/UI를 직접 실행·조작하거나 캡처하지 않는다. 코드·빌드 준비 후 사용자가 평소 경로로 재생하며 로그를 얻는다. 매 프레임 전체 맵을 덤프하거나 새 전역 진단 런타임/영구 하네스 프로젝트를 만들지 않는다.

- 실제 stage/layout, HP, pop mask 및 선택 instance ID.
- `Load_PreparedArea`의 성공/실패와 원문 status. 단순 JSON 파싱 성공을 대체 증거로 쓰지 않는다.
- `Play` 반환값과 **다음 Update의 Apply 성공 여부**. Play 성공만으로 표시 성공으로 기록하지 않는다.
- 대표 배치의 authored visible, runtime Visible, suppression, object/batch 중 실제 생성 경로.
- 위치·signedScale·실제 모델 bounds, frustum 판정과 presentation opacity, scene environment replacement 상태.
- 실제 CMaterial의 family/program, texture mask와 준비 결과, 선택된 render group/pass.
- `Bind_ShaderResources`, `Bind_Material`, `CShader::Begin`, `CModel::Render` 중 처음 실패한 단계와 HRESULT.
- SRV/상수/셰이더가 정상이라도 draw 제출만으로 GPU pixel 성공을 선언하지 않는다. 모두 성공인데 안 보이면 실제 입력을 사용하는 격리된 headless GPU 진단 또는 사용자 재현 비교로 discard/depth를 분리한다. 기존 진단 경로를 우선 사용하고 새 진단 프로젝트를 자동 생성하지 않는다. 기존 경로로 확인할 수 없으면 사용자에게 범위를 좁힌 재현을 요청한다.

### 원인 판정과 수정 조건

| 재현 증거 | 수정할 부분 | 금지되는 대응 |
|---|---|---|
| stage/layout이 미설정 | 진입·서버 case 선택·snapshot 소비 중 실제 실패 경계 | 클라이언트 임의 stage 부여 |
| document 준비/commit 실패 | 실패한 항목과 admission/load 경계 | 검증 전체 제거, unrelated 데이터 삭제 |
| Play/Apply 실패 또는 reload 후 표시 소유 상태 불일치 | 배치 재생 수명과 실패/재준비 상태 | 매 프레임 재생 초기화 |
| 표시=true이나 cull/bounds가 잘못됨 | 실제 개별 렌더 bounds/TRS 계산 | 전체 맵 culling 해제·공 위치 임의 이동 |
| bind/pass 실패 | 해당 program·shader·입력의 불일치 | 광원 세기나 알파를 임의 상향 |
| 실제 shader 입력으로 discard/depth 문제 재현 | 원본 MIC/텍스처/상수/좌표/깊이 계약 | 공 전체를 unlit/항상 앞에 그리기로 변경 |
| 조명·후처리 단계에서만 결과 손실 | 해당 지역·receiver·합성 입력 | 관문 전체 조명/후처리 초기화 |

프로그램 25의 초반 discard는 variation mask와 diffuse alpha를 사용한다. 조사한 설치 최상위 입력은 null=0, variation=0, diffuse alpha=1이므로 해당 조건만으로 전체 공이 버려지지는 않는다. **이는 CPU 입력 검사이며 실제 셰이더 실행의 정상 판정은 아니다.** 원본 material family/program을 문서 표 하나만 보고 25→26으로 바꾸지 않는다.

## G03. 색 공 표시 복구 시 지켜야 할 재생 계약

`Update_MarioLayoutPresentation`의 `m_strMarioLayoutInstance`는 서버가 선택한 표시 묶음이고, `m_bMarioLayoutStarted`/`m_bMarioLayoutFailed`는 해당 묶음의 시작/실패 상태다. 현재 같은 desired ID에서는 started/failed일 때 Play를 다시 요청하지 않는다. 이 정책의 존재만으로 이번 원인이라고 단정하지 않는다.

재현이 이 경로를 가리킬 경우 다음을 구현 명세에 반영한다.

- started는 단순 Play 호출 여부와 혼동하지 말고 document/대상 revision과 실제 재생 소유 상태에 맞춰 관리한다.
- 정상 STOP은 1초 뒤 마지막 visible=true pose를 유지한다. 정상 완료를 장애로 오판해서 매 프레임 재시작하지 않는다.
- 실패한 Apply, 명시적 Stop/Restore, 성공한 reload를 구분한다. 다시 준비된 입력에 대해서만 제한된 재시도를 허용한다. 고장 난 입력을 매 프레임 재시도하지 않는다.
- 같은 서버 stage/layout을 유지하는 reload에서도 이미 파괴된 공의 pop mask를 보존·재적용한다. 재생 재시작으로 파괴된 공을 부활시키지 않는다.
- layout 교체·퇴장·사망 시 이전 case의 표시와 suppression 소유권을 정리한다. 새 case의 마스크와 다른 플레이어의 서버 상태를 섞지 않는다.
- 모델·재질 복구 실패 시 다른 관문, 기존 정상 배치·저작 draft를 삭제하거나 초기화하지 않는다.

수정할 G의 원인이 결정되면 이 PLAN에 실제 파일·함수·변수·실패 정리 책임과 적용 후 전체 H/CPP를 기록한다. 새 C++ 파일을 만드는 경우에만 프로젝트와 filters 등록을 함께 명시한다. 아직 불필요한 공개 enum/packet/schema를 설계하지 않는다.

## G04. 마리오 맵 재질 복원

색 공이 다시 보이는 것과 맵 재질의 원작 일치는 별도 완료 항목이다. 색 공 원인만 고치고 전체 마리오 재질 복원 완료로 보고하지 않는다.

1. 사용자 첨부 각 장소에서 문제로 지적된 카드 바닥·측면·배경 등의 **placement ID/asset ID/material slot**을 확인한다. 사진의 흑백·검은 공간을 곧바로 누락 텍스처로 분류하지 않는다.
2. 같은 원작 장소의 영상/추출된 actor material array·MIC override를 찾는다. 현재 소스와 모델 slot에 대응시키고, 원작에 없는 임의의 색/무늬를 넣지 않는다. 원본이 없으면 해당 표면의 원작 일치 판정은 사용자 참고 입력이 필요하다고 결과에 명시한다.
3. MIC 상속·static switch·native program·uniform/texture index·sampler·색 공간을 비교한다. diffuse만 붙이는 것은 원본 재질 복원이 아니다.
4. geometry의 UV0/UV1/필요 UV, vertex color, tangent/normal, 단위와 재질 slot을 확인한다. 재질 재추출로 현재 배치·카메라·애니메이션을 재생성하지 않는다.
5. 정적 맵의 RNM·static shadow·lightmap scale/bias와 해당 placement 연결을 확인한다. 움직이는 공에 정적 맵의 구운 조명을 복사하지 않는다.
6. 광원의 receiver, native character 직접광과 baked BG의 중복 수광 여부, IBL/환경·fog·exposure를 분리한다. 원본과 현재 renderer adapter의 한계도 기록한다.
7. 저장 문서→publisher 출력→실제 로더→CMaterial→GPU 입력 순서로 손실 지점을 수정한다. 정확한 대상이 아닌 공통 Engine 조명이나 카드미로 재질을 함께 조정하지 않는다.

범위별 A/B가 필요하면 한 번에 하나의 입력만 바꾸고 비교가 끝나면 실험용 값을 제거한다. 재질 family 우회·culling 해제·opacity 강제값은 출시 수정으로 남기지 않는다. `bUseSourceMaterials`/Recovered map materials 체크가 실제 SOURCE_CHARACTER 분기도 끄는지 코드로 먼저 확인한다. 현재 native bind 분기는 일반 source BG의 체크와 다르므로 체크를 껐다는 사실만으로 원본 공 재질을 제외한 A/B라고 판단하지 않는다.

### 필요한 공 리소스

Resources 상대 경로이며 실제 위치는 확인된 `Client/Bin/Resources/` 아래다.

- `Map/LV_LUT_MIDNIGHTC_ED/MarioProps/RedStarBall/RedStarBall.wmodel`
- `Map/LV_LUT_MIDNIGHTC_ED/MarioProps/BlueBall/BlueBall.wmodel`
- `Map/LV_LUT_MIDNIGHTC_ED/MarioProps/YellowBall/YellowBall.wmodel`
- 비교 기준 `Map/LV_LUT_MIDNIGHTC_ED/MarioProps/StripedBall/StripedBall.wmodel`
- 각 모델 폴더의 기존 textures 및 native 재질이 참조하는 `Character/SourceMaterials/Kouku/mn_ppcc_00/`, `Character/SourceMaterials/Kouku/efmaster_material_prologue/`의 해당 DDS.

원본 파일이나 수정한 DDS가 필요할 때만 해당 파일을 Drive 인계 목록에 적는다. Resources를 Git에 force-add하거나 전체 Resources를 덮어쓰지 않는다.

## G05. 검증·배포와 회귀 방지

### 데이터 변경

프로젝트 루트에서 실행하는 현재 publisher 명령이다. 수정하지 않은 domain은 실행하지 않는다.

```powershell
# 재질/맵 배치가 바뀐 경우 먼저 검증
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Validate

# 관련 저작 변경 전체를 검토한 뒤에만 게시
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Check
```

WorldSequences만 수정했다면 위의 Scope를 WorldSequences로 바꾼다. 재질/신규 배치 수정은 그 scope만으로 게시되지 않는다. Area publish는 같은 Area의 다른 작업까지 내보낼 수 있으므로 현재 dirty 조명·배치·카메라·sequence와 사용자의 미저장 draft를 먼저 보존하고 게시 범위를 확인한다. 출력 DataFiles를 직접 편집하지 않는다. RenderingProfiles 또는 Server 입력을 실제 수정한 경우 해당 문서에서 정한 publisher도 별도로 사용한다.

### 빌드

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
git diff --check
```

데이터만 바뀌면 저장·publish·재로드/재진입 경로로 확인하며 C++ 빌드를 요구하지 않는다. C++/HLSL이 바뀌면 현재 VS 설치와 동일한 toolset/구성의 정상 증분 Product Build를 사용한다. Clean/Rebuild, OBJ/PCH/CSO 삭제, 강제 전체 shader skip은 하지 않는다. Client 종료는 사용자가 Save 후 수행하며 실행 중인 Client를 임의 종료하지 않는다. Server/Shared를 수정했다면 실제 연결하는 서버까지 같은 변경을 반영·재시작해야 한다.

### 사용자 재생 확인표

| 확인 | 기대 결과 |
|---|---|
| 1·2·3마리오 각각 진입 | 선택된 case의 색 공 9개가 의도된 위치에 존재; 이미 파괴된 것은 제외 |
| 4마리오 진입 | 선택된 case의 색 공 12개가 의도된 위치에 존재; 이미 파괴된 것은 제외 |
| case 1/2/3 | 선택된 한 case만 표시, 미선택 case 숨김 |
| 진입 직후 및 1초 이후 | 정상 STOP 후에도 공이 유지됨 |
| 색 공 파괴·재진입 | 서버 파괴 상태와 reset 정책 유지, 임의 부활·중복 없음 |
| 줄무늬 공 | 기존 배치·크기·bounce 유지 |
| 자유시점 | 현재 표시된 같은 공이 각도 변화만으로 부당하게 사라지지 않음 |
| 퇴장·사망·stage 전환 | 이전 case의 잔상/중복·누적 없음 |
| 허용된 Save/Reload와 재진입 | 배치 표시·suppression·재질 입력의 일관성 유지 |
| 맵 재질 | 원본과 대응한 문제 표면의 재질/UV/조명 오류가 개선됐다고 사용자가 확인 |
| 인접 기능 | 카드미로 문양, 컷신·암전·카메라, 다른 관문 조명에 회귀 없음 |

모든 case를 확인하기 위해 Server 난수 정책을 영구 고정하지 않는다. 기존 검증/Debug 경로가 있으면 그 경로를 사용하고 사용자 재생과 구분해 기록한다. 자동 검사와 실제 화면 판정을 혼용하지 않는다.

## G06. 결과물과 클로드 완료 보고

수정 전에 독립 비평으로 원인 가정·소유권·영향 범위를 점검하고 실제 코드로 재확인한다. 최종 변경의 정확한 H/CPP 전문 및 JSON 교체 블록은 이 PLAN에 보존한다. 새로운 조사용 전역 프레임워크는 만들지 않는다.

RESULT에는 다음을 구분해서 기록한다.

1. 최초 실패 지점과 실제 증거. 재질 복원 변경과의 관계가 증명됐는지, 단지 시점상 연관인지.
2. 색 공 미표시의 수정 파일·함수·데이터와 정상 표시까지의 연결.
3. 맵 재질의 대상 slot별 수정 내역. 확인하지 않은 표면을 완료에 포함하지 않는다.
4. 보존한 배치·TRS·카메라·조명·게임플레이, 실험용 우회 제거 여부.
5. 실제 실행한 최소 컴파일/파싱/publisher 결과와 실제 실행 파일·Resources 경로.
6. 사용자가 확인한 장면과 아직 확인하지 않은 장면.
7. 남은 문제, 필요한 원본/사용자 입력, 다른 PC로 전달할 정확한 변경 리소스.

원인이 확인되지 않은 상태로 “공은 기본 숨김이라 정상”, “조명이 원인”, “재질 복원 완료”라고 종료하지 않는다. 실행 중 증거가 없으면 진단 가능한 수정본/로그 수집 위치까지 준비하고 필요한 사용자 재생 한 번을 구체적으로 요청한다. 검증되지 않은 원인에 맞춰 무관한 수정으로 진행하지 않는다.
