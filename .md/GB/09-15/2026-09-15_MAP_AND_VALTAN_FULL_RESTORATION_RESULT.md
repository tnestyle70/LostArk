# 맵·발탄 원본 입력과 표현 복원 결과

## G25-V. 09-16 도끼·돌·4방향 물보라·포탈의 원본 재질과 공간 입력

### 현재 반영 범위

쿠크의 원본 MIC/MaterialMap/VF/VS/PS → native material/shader → 기존 Effect 재생 경로를
사용했다. 아래는 소스·데이터 반영 및 비UI 수치 검증이다. 원작 화면과의 최종 일치나 실행 중
Client 적용 완료를 의미하지 않는다. G24-V의 35개 action 복원본은 현재 **39개**다.
Product presentation/V2의 full.restore cue는 계속 0개이며 기존 문서 바이트를 보존했다.

| 대상 | 실제 변경·검토 |
|---|---|
| 도끼 전기 | 원본 전기2995와 texture/dynamic/additive dispatch를 대조했다. 별도 6방향 피자420620/stage004의 누락된 Atk_08_05/08_03·조명·ZoomBlur 총13요소를 추가했다. `6방향 후 전멸 패턴`은 별도의420630/stage002/003/004/005/007이다. |
| 본 부착·시간 | Full Restore bone-follow에 실제 owner 배율1.4를 유지하고 arena snapshot은1을 유지한다. 실제 설치 무기/골격901표본에서 기존 최대1.762499m 오차와 수정 basis 일치를 확인했다. 원본 Anim playMs와 previewWallMs를 분리하여 5개 loop의 조기 pose 고정을 수정했다. 조건부 전환 시간은 preview 정책으로 구분한다. |
| 돌·파편 | 기존 ground-roar/six-pizza/struggling의 active/explode 6문서에 정확히 일치하는 원본 mesh/MIC2391/2452를 연결했다. masked LocalVF의 RGBA prefix 누락으로 모든 돌 픽셀이 clip되던 셰이더를 수정했다. 동적 소멸·발광 입력을 전달하는 기존 mesh-particle carrier를 사용한다. |
| 4방향 공격 | stage007의 원본25 color/12 distortion shader, 기존29 material permutation과66 DDS를 재대조했다. 기존240 material 요소의 parameter/texture bytes가 일치했다. 파편2383의 같은 masked-prefix 결함을 수정했다. 별도 int FRotator를 읽어20notify/244요소 중209회전을 교정했다. 원본 위치·크기·시간은 유지했다. 직접 GroundEffect 부채꼴4개를 추가하여 현재248요소다. |
| 포탈·돌진 | entry stage000/exit stage004와 source portal을 등록했다. 기존 stage001/006의20요소 중 각각19개는 실제 생성되고1개는 원본 spawn rate0/burst없음이다. 본 로컬 좌표를 UE world축으로 중복 변환한6notify를 수정했다. stage003에 직접 GroundEffect 원2개를 추가했다. |

### 원본 크기와 shader 연결

- 원형 바닥2614는 원본 PS `6b8e8fca5028ea449cbe6a1d5aebb3c6`의115명령과 기존
  LocalDecal VS/CB0 prefix를 사용한다. 지름3.5/5.5m, 두 번째 전방 offset2.9m,
  시작.65/.75초, 수명.5/1초, projection depth6m를 실제 소비자에서 확인했다.
- 부채꼴2615는 원본 PS `c390f9f83fce2b4791421e47db4cdcc6`의145명령을 사용한다.
  원본 SkillDecal2001/SkillEffect42062402/04/06/08의 반경10m·각도80도,
  방향0/180/270/90도, 시작.1/1.1/2.1/3.1초를 보존한다. native UV +V와 실제 projector
  local -Z의 차이만 해당4요소에서 변환한다. 임의 inner 채움 곡선을 추가하지 않았다.
- 물보라2380/굴절2381의 원본 signed distortion 출력은 실제 dispatch의 MRT로 전달된다.
  일반 detail distortionIntensity=0을 근거로 이 native 경로까지 비활성이라고 해석하지 않는다.
  전기2995도 원본 alpha0 뒤 dispatch의 opaqueCoverage1을 거쳐 additive RGB가 살아 있다.
- 돌은 설치52정점 WModel/preScale.01과 authored scale.6 기준 bounding size가
  root1에서 .7107×2.6016×1.0529m, 실제 owner1.4에서 .9950×3.6422×1.4740m다.
  matching source의2초 이동 돌 emitter와 현재5/19.5초 정지형 저작 수명은 동일하지 않다.

### 실행한 검증과 근거

- 실제 설치 body/donor 골격·원본 socket, 120Hz CEffectDocumentCodec/CEffectPlayback 검사:
  포탈/바닥/4방향과 추가 도끼7문서 **2,411,487 검사 통과**. Full Restore root identity와 named
  anchor owner1.4를 분리했다. 4방향의244 기존 요소와4개 fan이 생성되며 자연 잔여 수명도 확인했다.
  측정 중심·matrix 축 길이는 mesh 정점 AABB나 화면 크기와 구분한다.
- 돌 실제 재생 **433,082 검사 통과**. 원본 shader 함수 GPU5 draw에서 수정 전 전체 clip,
  수정 후 HDR 청록색 발광, Dynamic.W 발광 차이[3,22.5,12], Dynamic.X/alpha 소멸을 확인했다.
- 수정 Tool Helpers/Playback/Valtan CPP 최소 Debug 컴파일과 metadata/source timing 검사 통과.
  현재39개 index가26패턴에 연결되고 FOUR는 Att_Battle_19_01/stage007을 사용한다.
  원본833ms charge clip은 wall3900ms까지 같은 pose loop로 진행한다. 패턴 아래 Full Restore의
  두 버튼도 동일한 원본 경로로 통일하여 Product 속도·반복·준비 상태가 섞이지 않게 했다.
- 근거: `out/ValtanAxeRestore20260916`의 source timing/actual weapon/FourDirection receipt,
  `out/ValtanPriorityRestore20260916/FourDirectionNative`의 material_texture_receipt와
  shader_restore_receipt, `out/ValtanPortalRestore20260916`의 native/fan/notify-axis/consumer
  기록, `out/ValtanSummonedStone20260916/stone-restoration.receipt.json`.

### 미완료 경계

1. 돌 주변의15개 수작업 ring/sprite는 복사 원본까지 sourceMaterialPath/recipe/원본 식별자가
   없다. 임의 DDS 유사성으로 source MIC를 배정하지 않았다. 정지형 돌의 원작 생성 owner와
   수명·배치는 아직 확정되지 않았으며 원본 이벤트 전체 복원으로 표기하지 않는다.
2. 요청 패턴에 포함된 PawnMaterialParam3notify의 원본 색 곡선은 찾았으나 body/armor/weapon
   대상 enum과 activation/restore 시간 의미가 미확정이다. source particle/light/post와 별개인
   배우 재질 시간 변화까지 완료했다고 주장하지 않는다.
사용자가 복원 범위를 여기서 마치고 컴파일 확인까지 요청했으므로 위 미확정 범위를 더 확장하지 않았다.

사용자 확인 경로는 빌드 후 `F1 → Effect Tool V1 → All Effects → Valtan → 패턴 →
Full Restore → Open Editor → Play`다. 도끼 전멸 패턴과 중앙이동 후6방향 피자는 서로 다른
패턴임을 유지한다. 현재 Client/Server는 종료 상태이며 새 실행본의 화면 확인은 사용자 단계다.

### 정식 제품 빌드 종료

2026-09-16 10:34 KST, `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`
**PASS / 실제 컴파일·링크 에러0건**. Engine → Shared → Server → Client가 모두 성공했고,
Client 최종 링크, Engine.dll 및 변경 CSO 배포를 완료했다. Engine OBJ1, Client OBJ167·CSO7이
갱신되었다. 기존 C4819/C4267 및 shader 경고는 남아 있으며 경고0을 주장하지 않는다.
`Client/Bin/Debug/Client.exe`는10:34, 배포 Engine.dll은10:30:57, Server.exe는10:31:05 산출물이다.
Engine producer와 Client 배포 DLL SHA-256 일치를 확인했다. Client나 UI는 실행하지 않았다.

변경 범위 JSON213개/XML2개 parse, focused source metadata13검사와 scoped diff 검사 통과.
추가 임시 UI-route scaffold의 overload 추출 실패는 제품 컴파일 결과와 구분하며 그 별도
검사의 성공을 주장하지 않는다. 실제 두 UI 버튼은 검증된 동일 source 재생 경로를 호출하고
현재 Product 빌드에 포함됐다. 데이터 publisher·광역 runtime diagnostics는 실행하지 않았다.
정본 결과: `out/BuildPipeline/runs/20260916T013400956Z-debug-product.json`, 상세 로그와
산출물 hash는 `out/ValtanPriorityRestore20260916/product-build.log`, `final-validation.json`.

## G24-V. 09-16 발탄 Full Restore 전체 클립 재생·맵 로더 수정

### 구현 상태

- `.restore` suffix만으로 player recovery Sequencer를 선택하던 8개 분기를 발탄과 구분했다.
  Play, Restart, element Solo, 필터는 기존 World Preview를 사용한다. Play Group도 선택한
  stable Element ID와 숨김 provider만 같은 애니메이션 시간으로 재생한다.
- `Data/Effects/ValtanFullRestoreAnimations.json`은 설치된 35개 복원본 각각의 원본
  action/stage/clip/time 연결이다. 기존 source receipt와 34개 legacy actionbindings가 일치하며,
  legacy 색인에 없던 400440/stage000도 원본 Att_Battle_11_01 / 1800ms로 등록했다.
  35 Effect JSON은 다시 생성하거나 덮어쓰지 않았다. builder는 이후 설치에서도 이 연결을
  함께 갱신하고 기존 authored 편집과 metadata를 보존한다. 프로젝트 None/96.DataFiles에 등록했다.
- Full Restore 패턴 행은 SourceActionIds와 실제 ClipOccurrences를 함께 비교한다. 실제
  C++ helper와 현행 presentation 입력에서 35개가 26패턴에 대응한다. 2페이즈 4방향 공격
  VALTAN_SEQUENCE_FOUR에는 420624/stage007 하나만 남고 mesh_att_battle_19_01을 사용한다.
  action 번호만 보고 다른 stage 7개를 나열하던 G23-V 구현은 대체했다.
- 패턴에서 Open Editor는 해당 occurrence를 포함한 기존 authoring timeline에 연결하고
  t=0에서 정지한다. Play Effect + Animation/편집기의 Play는 같은 animation/effect clock을
  사용한다. EXISTING AUTHORED EFFECTS에서 복원본을 직접 열어도 원본 단일 clip을 연결한다.
- 새 preview runtime은 만들지 않았다. 기존 Valtan preview timing에 editor-only 표식을 두어
  Product cue 저장 권한과 분리했다. root identity/ARENA_ABSOLUTE 1/sourceStart 0을 사용하므로
  문서에 이미 있는 notify 지연·root -90도·크기를 중복 적용하지 않는다. Product full.restore
  cue는 계속 0개다. 같은 문서를 다시 여는 경우 active draft를 디스크로 덮어쓰지 않는다.

### 맵 입장 실패

20/3760 표시는 멈춤이 아니라 29.8초 뒤 발생한 CLIENT_LOAD_FAILED였다. 처음 실패한
VALTAN_NATIVE_CE735E61C7EFFC177741의 subspecular-only 재질은 catalog와 shader에서
지원하지만 CModel guard가 거부했다. 이 guard를 일치시킨 뒤 전체 모델을 검사하자 기존에
가려져 있던 grass flicker와 체인 재질 경로 오류도 확인됐다.

- Model.cpp: positive subspecular texture/direct specular off 조합 허용(109개 asset).
- Model.cpp: 이미 shader/parser가 지원하는 grass emissive flicker 허용(3개 asset).
- 체인 WModel 2개: 없는 legacy PNG 6경로를 실제 설치된 원본 DDS로 교정(13개 variant).
  형상 바이트와 파일 크기는 같고 원본은 out/ValtanLoader20260916/resource-backup에 보존했다.
  대상 geometry ID는 VALTAN_NATIVE_BF2BC10FBA77106C57D4, VALTAN_NATIVE_BBDCD1D1FFED60110BE5다.
- Loader.cpp: 실패 시 Area/asset ID/Resources-relative model path/count를 상태에 유지한다.
  맵 범위, 배치, 가시성은 변경하지 않았다.

### 실행한 검증

- 설치 모델·재질을 실제 CModel/Create_MaterialVariant로 읽는 비UI 검사: **3760/3760 성공**.
  잘못된 specular 입력 2종은 계속 거부했다. 화면이나 visual fidelity 검사는 아니다.
- 수정 Model/Loader와 Effect Tool CPP 최소 Debug 컴파일 성공. Engine 검증 DLL은 out 아래에
  격리했다. 기존 SDK C4828 경고만 남았다.
- 실제 C++ source index parser와 목록 join: 35 source/35 joined/26 patterns, FOUR stage007 1개.
  source clock 0/1.170/2.254/3.224/4.220초가 동일한 effect sample로 변환됨을 확인했다.
- 실제 BossCatalog가 사용하는 146-clip AnimSet에 35개 모두 이름이 존재한다. native clip 길이와
  원본 action wall budget은 구분한다. 원본 Anim notify의 loop flag 9개를 보존했다.
  4000ms loop/900ms native pose는 반복, 2100ms window/2067ms pose는 마지막 33ms 종료
  pose 유지를 기존 CActionPresentationTimeline 실제 함수로 확인했다.
- builder metadata 보존·중간 실패 rollback·concurrent edit·원본 loop payload 등 8개 검사 통과. JSON/XML parse와
  scoped git diff --check 통과. 기존 All Effects contract는 38개 중 36개 통과이며 2개 실패는
  G23-V의 변경 전 baseline에서도 동일하다.
- 근거: out/ValtanFullRestoreBrowser20260916의 clip_probe, model-animation-validation.json,
  animation_metadata_validation.json, final-contract.log 및 최소 compile 로그;
  out/ValtanLoader20260916/verification-summary.json, full-scope-final.json,
  resource-repair.receipt.json.

### 제품 반영·사용자 확인

실행 중 Client/Server를 임의 종료하지 않았다. 현재 제품 링크·Engine.dll 배포는 사용자 종료를
기다리는 상태이며, 소스·out 검증 성공을 실행 중 Client 반영으로 기록하지 않는다. 빌드 뒤
`F1 → Effect Tool V1 → All Effects → Valtan → 패턴 → Full Restore → Open Editor → Play`로
확인한다. 화면의 최종 표시·방향·크기와 발탄 실제 입장은 사용자가 판정한다.

## G23-V. 이전 진입점 구현 이력

이 단계의 action-only 목록과 standalone 재생은 G24-V의 정확한 clip 연결로 대체했다.

## G00. 현재 상태

진행 중인 결과다. 사용자가 카메라 복구 성공을 확인했고, 실제
`C:/Users/user/Desktop/로스트아크_렌더링`의 비교 PNG 13장을 전부 열람했다.
이번 변경의 Client 실행·화면 캡처·visual PASS는 수행하지 않았다. 추가 재질·렌더링·오라의
통합 Product build와 사용자 화면 확인은 아래 검증 상태와 구분한다.

우선순위는 발탄 전투 공간/본체, Character Select, 베른과 발탄 이펙트, 쿠크다.
같은 `pattern-3` 작업 디렉터리의 기존 Kouku/Effect 미커밋 변경을 보존한다.
카메라·렌더링의 앞 단계는 09-14 대응 PLAN/RESULT를 따른다.

사용자의 "잠깐 끊고 갈까" 요청으로 현재 상태에서 작업을 일시 중단했다.
추가 조사·제품 수정·빌드를 시작하지 않는다. 재개 시 첫 작업은 아래 후속 증분 build와
휠윈드 실제 OBJ 검증이며, Character Select 중앙8개 MIC의 native shader 해석과 발탄 하늘525의
custom lighting 연결,528의 engine-owned 환경광 입력은 미해결 조사 지점으로 보존했다.

## G01. 카메라와 캐릭터 크기

| 맵 | 저장 수평 FOV @16:9 | 거리 | 구분 |
|---|---:|---:|---|
| Character Select | 50° | 16m | 원본 공통 isometric CDO |
| Bern | 55° | 16m | 사용자 비교값. Source baseline은50° |
| Valtan | 55° | 18m | 원본 PS 카메라 영역 |
| Kouku | 50° | 19m | 원본1관문 영역. 2·3관문은 시험 적용 |

FOV 아래 Character size와 Reset size를 구현했다. 기존 catalog presentation scale에
0.25~4배를 곱하며 body/equipment/socket의 같은 presentation root에 적용한다.
네 Level의 생성·class 교체·camera profile 적용이 이를 소비한다. Server Transform,
충돌·공격 반경을 수정하지 않는다. source/before camera preset은 크기 입력을 보존한다.

기존 v1 JSON의 optional characterSizeMultiplier는 생략 시1이며 새 저장은 값을 포함한다.
숫자 범위·외부 파일 변경·잘못된 문서에서 기존 draft/파일을 보존한다. 1은 기존 catalog 기준이며
원작 게임이 추가로 적용하는 최종 actor scale을 확정한 값이 아니다.

현재 ArenaCameraProfile.cpp를 별도 컴파일해 실제 저장·로드 경로96검사 PASS.
근거: `out/FullMapRestoration20260915/camera-profile-check.log`.

## G02. 발탄 전투 공간

원본 StaticMeshComponent effective MIC와 native suffix를 대조해 바닥·바위443배치의
재질 입력을 연결했다. 기존7개에서436개를 추가했다. source MIC rock02 252,
rock04 27, rock05 164이며 source static set과 vertex COLOR/UV1/RNM을 구분했다.
동일 이름의 서로 다른 package texture는 별도 Resources 상대 경로로 보존한다.

원본 shadow234개는 DOM GUID `4ba587b9fa985e4b91c324a665a0ef33`에 연결된다.
SDF G8의 전체 native mip payload와 component 좌표를 운반한다. 원본으로 확정되지 않은
shadow transfer width0.05는 PROJECT_ADAPTER 경계다. 기존22 point light의 baked GUID와
해당 stone 입력은 교집합이 없으므로 광원 전체를 임의 비활성화하지 않았다.

중앙 Deploy A/B는 원본 geometry에서 각각25,819/34,306정점과2/3 UV 채널 및 정점 색을
회수했다. 두 모델의 바닥/균열 material slot을 BossCatalog의 named surface에 연결했다.
source crack은 원본 static option에 따라 sourceFlags133이며 별도 MIC emissive는 없다.
기존 중복 emissive overlay를 native surface에 다시 더하지 않는다.

ActorCatalog는 해당 정적 surface row를 CMapAssetCatalog의 동일 parser로 검증하고,
DeployPropObject는 기존 MapAssetRenderUtils/CModel/CMaterial을 소비한다. placement에
속하는 bakedLighting과 비지원 draw/cull은 actor row에서 거부한다. 별도 모델 경로를 추가하지 않았다.

새 geometry의 접선 교차 검증에서 UE→glTF의 반사 부호 누락을 찾아 설치 전에 수습했다.
glTF W=−native, 최종 runtime W=native가 맞다. 새75 geometry의152,609정점 W를 교정했고
나머지 position/N/UV/T.xyz/COLOR/index는 전량 보존했다. 기존7 geometry는 원본과 일치하여 유지했다.
근거는 `Valtan/native-tangent-basis-audit.json`, `existing-seven-tangent-audit.json`,
`tangent-repair-install.json`이며 모두 현재 out/FullMapRestoration20260915 아래에 있다.

MapAssetCatalog.cpp 최소 컴파일과 실제 새 Deploy4개 row/거절 시 기존값 보존 등20검사 PASS.
맵 Area Validate/Publish/Check PASS. 난간은 원본20,440정점과4개 material slot을 연결했고,
PS export527 구름은 source forward program59와 별도 MIC variant를 연결했다.
현재 map catalog717, material446, placement lighting444개다. PS export525/528 하늘의
원본 shader와 lighting channel은 추가 조사 중이며 전체 맵 완료가 아니다.

## G03. 발탄 본체·크기·이펙트

정상·유령·갑옷·도끼는 실제 설치 WModel 정점, mesh offset, skeleton hierarchy,
CModel preScale과 socket basis를 계산했다. 정상 bind 높이는 약3.000266m,
유령은2.863072m다. 서로 다른 파일의 raw 단위와 preScale을 임의 통일하지 않는다.
원본 NPC ModelSize140의 최종 소비자는 미확정이다. 이후 사용자가1.4배가 맞다고 명시해
정상/유령 BossCatalog presentationScale을1.4로 적용했다. native CPU 소비자 입증과 사용자
확정 배율을 구분한다. ClientReplication의 정상/유령 desc가 이를 읽으며 body/armor/weapon과
기본 오라의 동일 owner root에 적용한다.
캐릭터도 bind pose와 실제 서 있는 animation/actor scale을 구분한다.

유령 program84의 source master는 BLEND_Translucent다. NONBLEND ordered coverage에서
제외하고 기존 native character forward pass10에 연결했다. source varying/광원/안개/bloom을
사용한다. 원본 post-render depth와 approximate sort까지 동일 구현한 것은 아니다.

LookInfo 기본 particle은 normal default07, ghost default08이다. 정상 척추1부착,
유령 척추·양손·양팔5부착을 ActorCatalog.defaultParticles와 실제 본 이름/TRS로 연결했다.
새2 asset의8 emitter에서 원본 loops0을 보존했다. 기존 Effect_Playback의 owner-sustained
옵션은 이 boss 기본 오라에만 사용한다. 매프레임 전체 Seek/주기 reset을 제거하고 기존 고정 step
Update로 재생하며 죽음·숨김·pool 반환·normal/ghost 교체·release에서 handle을 정리한다.
새 native material2372만 추가했고 기존 공유 material/함수 변경을 보존했다.

실제 Product OBJ39개를 링크한30초 probe에서 움직이고 회전하는 root를 사용했다.
정상5 emitter와 유령3 emitter가20초 뒤에도 모두 발생하며 peak/capacity는 각각25/32,
12/19다. 동일 문서의 유한 재생 종료, loop1 입력의 지속 옵션 거절, 재설정 시 옵션 초기화도 PASS.
이 검사는 catalog/codec/playback CPU 경로이며 실제 CValtan lifecycle과 GPU 화면 판정은 아니다.
근거: `ValtanActor/aura_product_obj_probe.receipt.json`.

갑옷 색상 조사는 LookInfo의 body/armor/axe8개 slot과 MIC override를 대조했고, 원본 UPK에서
다시 추출한26개 texture의 base mip RGBA가 설치 DDS와 전부 일치했다. 원본 텍스처 자체에
황갈색 성분이 있으므로 색을 임의 제거하지 않았다. 이후 사용자가 원작도 금색이 맞다고 정정했고
비교 근거를 `발탄아레나_비교2_FOV55.png`라고 지정했다. 해당 이미지를 다시 열람했다.
금색 자체는 결함 조사에서 제외하며 갑옷의 원본 texture/MIC 색상 값을 제거하거나 무채색으로
변경한 작업은 없다. 최종 반사 밝기·청록 조명·안개·오라의 일치는 사용자 화면 비교가 남아 있다.
source action의 PawnMaterialParam/PawnMaterialChange/부위 분리는 full restore의 별도 범위다.
회오리420633 stage2/4의 활성 TransColor notify는 이번 재질 연결만으로 복원되지 않는다.

휠윈드420633의 기존9개 element 중8개 material에 source native 연결을 반영했다.
신규5개 프로그램과3개 distortion companion을 추가했고 기존1,188개 native 함수와
1,143개 material row 및 해당 effect의 material 외 필드가 보존됨을 대조했다.
이 추가 변경은 아래 첫 Product build 이후이므로 후속 빌드와 실제 OBJ 검증이 남아 있다.
high-jump는 아직 원본 native material 연결이 빠진 부분이 있다.
진입, 2페이즈 점프/붉은 하늘, 휠윈드, 도끼 변화, 에테르, 모든 패턴 occurrence는 후속 복원 중이다.
인벤토리 수집만으로 제품 연결 또는 화면 복원 완료로 표시하지 않는다.

## G04. 맵별 렌더링과 Benchmark

네 맵 WorldInfo/CDO/chain/volume의 활성 override를 resolve했다. 이름에 epic이 있는 chain도
실제 tone enum은 UE3 customizable이며 기존 Hable과 같은 함수가 아니다. native CPU packing과
원본 DXBC로 ToneScale/Range/Toe, highlights/midtones/shadows/colorize/desaturation 및 LUT 순서를 확인했다.

optional quality/region sourcePostProcess를 기존 RenderingProfileService와 Engine quality에
연결했다. source profile에서 이전 display desaturation을 중복 적용하지 않는다.
활성 LUT가 없는 경우 neutral 입력을 사용하고 override=false의 serialized LUT 이름을 켜지 않는다.
발탄 LUT는 원본 PF_A8R8G8B8, sRGB=false,256×16,16³의 실제 pixels다.

RenderingProfiles는22개/revision43으로 게시했다. 네 기존 base ID에 원본 입력을 반영하고
각 맵 before/source 별도 profile을 보존했다. Bern5, Valtan2 convex part, Kouku11 환경 영역을
원본 범위·priority·활성 값으로 연결했다. Valtan은 원본 안개/DOM 입력과 LUT를 사용한다.
runtime SH로 입증되지 않은 Lightmass 환경색을 runtime ambient로 임의 대체하지 않았다.

Benchmark는 네 맵 Before / Restored source profile / Return to entry를 제공한다.
source tone과 LUT도 비교 조건 fingerprint에 포함하며 기존 소유권·외부 변경 보존을 유지한다.
이 비교가 DOF, light shaft, 환경 particle, 물·폭포까지 자동 복원하는 것은 아니다.

LUTBlend의 원본 intermediate format은 A8R8G8B8이다. float CPU atlas 비교와8bit GPU
출력을 구분한다. 원본 DXBC immediate 0x322bcc77은1e-8이며 disassembly의 표시0을
literal0으로 옮기면 어두운 영역의 LUT quantization이 달라진다. 현재 기존 Deferred 경로에
GPU LUT bake를 기존 Deferred에 연결했다. 기존 pass0~27을 보존하고 원본 FLUT<1>/<2>에
대응하는28/29를 추가했다. 최종100조건/409,600 RGBpixels에서 LUT와 tone/lookup 최대오차0,
실패0, 비유한0이다. 실제20고유 map/region grading,16 stress,4 LUT crossfade,60 tone조건을
포함한다. 실제 ScopedSourceLutState 본문의 성공/early-return 실패 시 GPU state 복구와
전체 fx_5_0 effect 최소 컴파일도 PASS. Client 화면 비교는 아니다.

## G05. Character Select와 쿠크·베른의 남은 범위

사용자는 비교 화면이 현재 로스트아크에서 직접 촬영한 화면이라고 확인했다.
Character Select의 설치 원본 LV_LOBBY_CLASSSELECT_SL00을 새로 읽어803개 component와
55개 unique mesh를 다시 추출했다. placement property error와 unresolved placement는0이며
현재 배치 ID의 누락·추가도0이다. 전체 object 중 별도 native class2개는 generic tagged parser
밖이므로 전체 object 해석 성공으로 확대하지 않는다. package SHA256은
`55191fbeb0ebf2228c030a7807e7db287144ead3182a7921809f84f529c2d33c`다.

새55개 geometry와 현재63개 catalog variant의 모든 material submesh에서 방향 있는
삼각형 Position/Normal/UV0가 일치했다.6개 variant의 raw index 차이는 Assimp 정점 병합이며
semantic triangle mismatch는0이다. 중앙 FLOOR12/BRIDGE01E도 동일하다. UV1의 누락과
material/lightmap의 최종 출력은 이 기본 geometry 일치와 별개의 미해결 범위다.

중앙 원판2개와 다리8개의 원본/현재 world triangle을301×301 grid로 대조했다.
겹침28,448 sample에서 기존 Y 보정 때문에 위아래 관계가 바뀐 sample은0이다.
이는 CPU geometry 비교이며 alpha/culling/동적 표시를 고려한 화면 판정은 아니다.
반면 두 EFMotionStaticMeshActor의 현재 transform이 원점/항등인 확정 결함을 발견했다.
원본 위치 약(-785.61,-142.859,183.92),(-758.376,-142.859,211.255)m와 원본 회전·배율로
authoring을 수정하고 기존 publisher의 Publish/Check를 통과했다. 변경2행 외801행과 기존
Y9건은 byte 보존했다. 근거: `CharacterSelect/motion-placement-restoration.receipt.json`.
장식의 회전 운동은 정적 위치 복구와 별도 범위다.

중앙 문양 차이의 최종 원인은 미확정이다. 관련14 package와 별도 SL01 조사에서 교체할
중앙 asset의 확정 근거는 아직 없다. SL00에 독립 Decal class는 없지만 ParticleSystemComponent
10개와 동적 actor가 있으며 별도 표시 조건까지 조사해야 한다. UE4 혼합·리뉴얼·추출 불가능을
현재 원인으로 단정하지 않는다. 근거: `CharacterSelect/fresh-summary.json`,
`fresh-geometry-comparison.json`, `fresh-placement-comparison.json`, `central-layer-geometry.json`.

쿠크의 원본 PS/SL source placement2,951개는 위치/배율과 일치했다. 최대 위치 차이는
0.00008m, scale차이는4.055e-7이다. 추가 SCENE01A79개는 지원 Actor resolver 밖이므로
이 비교에 포함하지 않았다. 원형 바닥 FLOOR08/FLOOR08A의 실제1026정점·3186index는 새
원본 추출과 vertex/index payload가 전부 일치했고 교체하지 않았다. 원본이없는 COLOR0를
새로 만들어 넣지 않았다. 최종 preScale 후 최대 좌표 오차는9.03e-7m다.
근거: `Kouku/source-placement-scale.json`, `floor-native-parity.json`.

따라서 비교한 쿠크 원형 바닥을 일괄 확대할 근거는 없다. 2·3관문 카메라, 부착 actor 크기,
원본 광원/RNM과 실제 viewport 조건을 별도 대조한다. 베른 도서관·물·나무·폭포·창문 빛,
쿠크 전체 map material은 원본 후처리 연결과 별도로 남아 있다.

## G06. 빌드와 사용자 확인

- 카메라 실제 parser/save96검사: PASS.
- 공유 static surface parser 실제20검사와 최소 컴파일: PASS.
- Valtan Area/RenderingProfiles publisher: PASS.
- Debug Product build: PASS,421,324ms. 근거 `out/BuildPipeline/runs/20260914T171400350Z-debug-product.json`.
- 위 build 이후 휠윈드 native material 추가: 소스 반영, 후속 증분 build 대기.
- GPU LUT 원본 DXBC 대조100조건/409,600 RGBpixels 및 state 복구: PASS.
- 기본 오라의 실제 Product OBJ30초 지속 재생 probe: PASS. GPU/Client 화면 검사는 아니다.
- 이번 변경의 사용자 화면 판정: 대기. 카메라 복구 성공만 사용자가 확인했다.

build 완료 뒤 사용자가 Server/Client를 갱신해 F1 Player Follow Camera와 Rendering Workbench의
Benchmark를 확인한다. Client나 UI를 에이전트가 실행하거나 visual PASS를 대신 기록하지 않는다.
Git 제외 Resources의 추가 경로는 위 범위와 대응 설치 기록을 함께 공유해야 한다.

## G07. 사용자 밝기 회귀 보고에 따른 기본 렌더링 복구

사용자는 Character Select부터 전체 화면이 과도하게 밝아졌다고 보고했으며 쿠크에서도
밝고 뿌연 바닥을 첨부했다. HUD에 비해 맵의 어두운 무늬·색 대비가 낮아진 모습이다.
첨부 화면을 분석했으며 직접 Client 캡처나 최종 visual PASS를 수행하지 않았다.

`579d9b90`에서 기존14개 profile 중 네 맵의 기본값이 바뀌었다. 쿠크는 Bloom threshold
2.74→0.7, intensity0.5→0.9, exposure0.73→1, gamma1.905→2.2, Source UE3 tone 활성과
11개 환경영역 후처리가 함께 적용됐다. Character Select도 기존 배율과 tone 경로가 바뀌었다.
개별 원본식의 수치 검증을 현재 맵 전체의 활성 후처리 승인으로 확대했던 것이 회귀 범위다.

Character Select/Bern/Valtan/Kouku의 기본 profile을 `579d9b90^`
(`16d62e74c16185f1f70e467135a6b9621a46314e`) 객체로 복구하고 revision44로 게시했다.
기존14개 객체는 이전값과 같으며, 별도 before/source8개 및 미변경18개 profile 텍스트를
보존했다. globalQuality, 쿠크 별도 scene6개, geometry·재질·카메라는 변경하지 않았다.
source shader는 선택 profile에서만 활성화되므로 공통 shader를 다시 수정하지 않았다.

Authored Validate / Publish / Runtime Validate, 양쪽 profile·globalQuality 일치,
이전 객체 및 미변경 profile 보존, `git diff --check`를 통과했다. 근거는
`out/RenderingBrightnessRegression20260915/profile-restore.receipt.json`이다.
Rendering 데이터 수정에는 EXE 빌드가 필요하지 않다. 실행 중 Client에는 F1 → Tools →
Rendering Workbench → Authoring Pipeline → Reload Runtime을 사용자가 실행한다.
옛 catalog를 가진 Client에서 Save를 먼저 누르지 않는다. 화면 복구 확인은 대기 상태다.

## G08. 쿠크 3관문 바닥의 직접 반사 입력 단절 교정

2026-09-15 사용자 보고에 따라 3관문 두 FLOOR08/FLOOR08A를 대조했다. 두 재질 행은
09-07 복구를 포함한 ecdc681f와 같고 authored/runtime 재질·배치가 일치한다. 실제 WModel,
D/N/S/reflection 파일과 UV가 존재하며 D alpha는 위 바닥 전부255, 아래 바닥의
clip 통과 비율은98.89%다. 전체 바닥을 버리는 alpha나 texture 누락은 확인되지 않았다.

3관문 전용 파란 SPOT은 diffuse에 빛의 RGB·brightness를 전달하지만 specular는0이다.
scene.g3.dark의 방향광도 diffuse/ambient/specular가0이다. 기존 선택 바닥 marker1은
재질 RGB 반사율에 legacy light.specular를 곱하므로 해당 직접 반사가0이 되는 것을 확인했다.
Engine Shader_Deferred의 Resolve_MaterialSpecularLight가 marker1만 light.diffuse RGB를
선택하도록 수정했다. Directional/Point/Spot 소비자가 공유하며 marker0/2와 다른 native
family의 계산은 유지한다. catalog에서 family1/2는 해당 쿠크 바닥 두 행뿐이다.

Phong lobe, attenuation, shadow, 바닥 texture/geometry, 사용자 저장 RenderingProfiles와
maplights는 이 수정에서 변경하지 않았다. 원본 Blinn 조명식을 복원한 변경이 아니며,
전체 바닥 검정 현상의 모든 원인 또는 최종 반사 외관을 확인했다고 주장하지 않는다.

소스 diff 검사는 통과했다. 정상 Debug Product 빌드를 시작했으나 사용자가 직접 빌드하겠다고
지시하여 에이전트가 시작한 빌드 프로세스 트리만 중지했다. Client/Server를 종료하지 않았다.
따라서 새 shader의 Product 배포·GPU 검증·사용자 화면 확인은 이 기록 시점에 미완료다.
Valtan Deploy admission C++ 수정도 최종 링크 확인이 남아 있다. PR 게시 전 이 경계를 갱신한다.

## G09. 반사 교정의 GPU 검증과 전체 검정 증상의 구분

사용자 빌드 이후 Engine/Client의 Shader_Deferred.cso는 SHA256
`53da24048aaff403c0f2cf024f32e97ed4f04dcc3b9fc5039813334efd659d00`으로 일치한다.
Client PID51268과 Server PID49132는2026-09-15 11:23:55에 시작했으며 에이전트는
이 프로세스를 실행·종료·조작하지 않았다. 제품 프로젝트를 재빌드하지 않고 기존
PointLightFalloffContractHarness만 BuildProjectReferences=false로 컴파일·실행했다.

기존 GPU fixture에 marker0/1/2 × Directional/Point/Spot × legacy specular0/양수 ×
단일/batch의36조건을 추가했다. diffuse/specular RGB를 독립 비교하며 현재 제품 CSO는
36조건 실패0, 기존 receiver6조건 및 하네스 전체 통과다. 수정 직전 c65b2cf4^의 shader를
out에서만 재컴파일한 비교에서는 marker1 specular의12조건·36 RGB만 실패하고 diffuse와
marker0/2는 통과했다. 제품 CSO를 구버전으로 바꾼 적은 없다. 보존된 더 오래된 CSO는
현재 SourceGroup ABI로 생성부터 실패하므로 반사 회귀 증거로 사용하지 않았다.

하네스의 오래된 END==18 가정은 실제 유지 경계 SOURCE_LIGHT_MASK==18 검사로 교정했다.
기존 하네스 vcxproj의 runtime 배포에 Shader_Deferred_SourceGroup*.cso를 포함하여
제품 CShader가 실제 읽는 dependency를 준비했다. 새 프로젝트는 추가하지 않았다.
근거: `out/KoukuFloorSpecular20260915/regression.receipt.json`과 before/after 로그.

실제 설치 메시의 위쪽 삼각형 중심582개, mip0 D/N 표본 및 저장된 .6 SPOT을 CPU로
대조했다. 모든 표본에서 기본색 조명은 양수다. 반사의 최대 감색까지 적용한 상부 바닥
청색 출력 하한 추정은0.227~0.305, 하부는0.017~0.278이다. 렌더링 가시성·실제 제출 상태·
픽셀 커버리지·fog/Bloom은 포함하지 않은 수치 진단이다. 직접 반사0만으로 바닥 전체가
검어진 원인을 확정할 수 없다는 근거이며 최종 화면 PASS가 아니다.
근거: `out/KoukuFloorSpecular20260915/floor-diffuse-lighting-audit.json`.

추가 확인된 실행 경로: F1 Rendering Workbench의 Typed Effect Lights는
CPresentation_Manager의 모든 transient light를 차단한다. 맵 광원도 이 경로를 사용하므로
옵션off이면 .6 enabled=true여도 제출되지 않는다. 3관문의 방향광/ambient는0이어서
이 조건에서는 기본색과 직접 반사가 모두 소실될 수 있다. MapLight 상태 문자열은
S_FALSE suppression을 별도 집계하지 않으므로 현재 코드의 submitted 문구만으로 실제
조명 제출을 확정하면 안 된다. 사용자 당시 옵션 상태는 확인되지 않아 사건 원인으로
확정하지 않았다. 별도 저작 light preview도 runtime 문서를 덮을 수 있으나 관문 commit은
Composition preview를 정리하므로 지속 누출을 확인한 것은 아니다.

사용자에게 새 EXE의 바닥 전체/무늬/반사 상태를 요청했다. 그 관찰 전에는 전용광원의
brightness·백색광·profile·재질을 추가 변경하지 않는다. 기존 반사 수정과 실제 전체검정
증상의 원인 및 해소 여부는 계속 분리해서 기록한다.

## G09. 사용자 3관문 바닥 복구 확인과 원인 판정 경계

2026-09-15 사용자가 이어진 3관문 바닥 조사 중 `복구됐어`라고 확인했다.
이 관찰을 3관문 바닥 표시 복구의 사용자 확인으로 기록한다. 에이전트의 화면 캡처나
시각 판정은 없으며, 추가 제품 변경과 실행 중 Client 상태 조회는 중단했다.

확정된 코드 결함은 marker1 바닥 반사가 map SPOT의 0인 legacy specular를
소비한 연결이다. 해당 producer는 b5634787(09-07), 바닥 재질 RGB 소비는
ecdc681f(09-08)부터 존재하므로 오늘 PR390에서 새로 도입했다고 단정하지 않는다.
반사가 0이어도 diffuse는 별도로 남는다. 실제 WModel/TRS/normal을 이용한
16,992점 CPU 검사에서 변경 카메라의 화면 내 상면 diffuse는 모두 양수였다.
근거: out/ValtanAdmission20260915/kouku_camera_light_probe.json 및 동일 이름 .py.
오늘 추가된 Benchmark 복귀/입구 마커가 G3 광원을 제거하는 경로는 확인되지 않았다.
Typed Effect Lights OFF는 맵 SPOT도 억제하지만 당시 토글 상태는 확보하지 못했다.
따라서 사용자 화면 복구와 반사 입력 교정은 확인하되, 과거 전체 검정 화면의
단일 발생 조건까지 확정한 것으로 확대하지 않는다.

### G09 사용자 복구 확인

2026-09-15 Typed Effect Lights의 맵 광원 차단 경로를 안내한 다음 사용자가
“굿, 복구됐어”라고 확인했다. 사용자 관찰에 따른 해당 바닥 증상의 복구 확인이다.
사용자가 마지막으로 조작한 값이나 스위치 전후 상태를 별도로 명시하지 않았으므로
어느 한 조작만으로 발생 원인 전체가 확정됐다고 확대하지 않는다. 재질 파일은 정상,
직접 반사 입력 단절은 GPU 재현·교정 확인, Typed Effect Lights의 맵 광원 차단은
코드에서 확인된 경로로 구분한다. 에이전트 화면 조작·캡처·대리 시각 판정은 없다.

사용자는 이어서 “아 typed effect lights 끄니까 이렇게 나오네”라고 스위치off에 따른
증상 재발을 보고했다. 앞선 전체 검정 증상은 맵 광원까지 공유하는 transient light
스위치의 적용 범위와 연결된다. 이는 별도로 검증한 직접 반사 입력0 결함과 구분한다.

## G10. 사용자 재현으로 전체 검정 화면의 직접 원인 확인

G09 이후 사용자가 `아 typed effect light 끄니까 이렇게 나오네`라고 재현을 확인했다.
이번 3관문 전체 검정 화면의 직접 원인은 Rendering Workbench의 Typed Effect Lights
OFF다. MainApp checkbox가 CPresentation_Manager의 transient lights 전체를 억제하고,
CMapLightPresentationRuntime의 맵 광원도 Add_TransientLight를 사용하므로 유일한
청색 SPOT까지 제출되지 않는다. G3 dark profile은 방향광/환경광도 0이어서 바닥의
기본색과 문양을 비출 빛이 사라진다. 텍스처나 모델 삭제가 아니며 A/B 품질 초기화는
이 별도 메모리 토글을 켜지 않는다. 기본값은 true이고 Save/Publish 대상이 아니다.

marker1 반사광 입력 결함은 별도로 존재하지만 이번 전체 검정의 직접 원인으로
취급하지 않는다. 오늘 PR390의 새로운 광원 삭제나 카메라 회귀로도 취급하지 않는다.
사용자 관찰로 원인이 확정되어 추가 제품 수정/Client 조작은 수행하지 않았다.


## G11. Character Select 로더 23/127과 후속 재질 로딩 오류

사용자 실행 `Client/Bin/Debug/Diagnostics/client-session-87156.jsonl`의 두 Character
Select 입장은 Server 승인 후 `loading.target-resource-load / CLIENT_LOAD_FAILED /
Map: model prototypes 23/127`로 실패했다. 현재 scope의 24번째 정의는
`MAP_94E1EF196C1A_LV_MODULE_MESH03_512_OVR_75EF6B0F204F_FR_E0FED3CFC174`다.
기존 Loader.cpp 자체의 수정은 없었으며 이번 변경은 Loader가 호출하는 Model/Material과
Character Select의 후속 Level 활성화에 있다.

### 반영한 변경

- Model/Material은 PBR의 명시 surfaceDiffuse/surfaceNormal/detailNormal/ORM을 검사한다.
  비어 있는 dummy 기본 normalPath 때문에 정상 PBR을 거부하던 검사를 교정했다.
  기존 기본 D/N/S를 실제 소비하는 family와 Resources 경계 검사는 유지했다.
- Character Select Ready_Lights는 Loader의 기존 optional ambient 계약에 맞춰 게시
  mapeffects 문서가 없을 때 진입한다. 경로 조회 오류와 존재하는 손상 문서는 실패하며,
  Bern/Valtan의 기존 strict Load 호출은 변경하지 않았다.
- SourceMaterials의 텍스처 5개는 확장자만 DDS이고 실제 내용은 TGA였다. 이 파일을
  사용하는 모델 51개와 PBR 교정 후 실패한 51개가 정확히 일치했다. 실제 DDS로 무손실
  변환·설치했고, 다섯 파일 모두 크기와 RGBA 픽셀이 전후 완전히 같다. 원본 백업과
  stage/전후 hash는 `out/CharacterSelectLoader20260915/texture-repair.receipt.json`에 있다.

### 실행한 검증

기존 DLL로 실제 CModel 생성 실패와 `dummy_material_0 / required material input is
absent`를 debugger 출력에서 재현했다. 수정 DLL은 같은 모델을 생성했다. 다음 실패군은
`PBR input load failed`와 실제 TGA 헤더로 분리했다. DDS 교정 후 current catalog의
동일 scope 127개를 `CModel::Create -> Create_MaterialVariant`와 수명 유지 경로로
검사해 **127 성공 / 0 실패**를 얻었다. 빈 PBR normal 및 Resources 밖 경로 두 사본은
계속 거부됐다. headless probe 초기 버전의 COM 미초기화 실패는 제품 결함으로 기록하지
않으며, 실제 Loader와 동일한 COM MTA 초기화 후 결과만 사용한다.

Engine Debug Build(Material.cpp/Model.cpp 컴파일·DLL 링크)와 Client Debug ClCompile은
오류 0으로 끝났다. 기존 인코딩·크기 변환 경고는 남아 있다. 최종 Client 링크·runtime
DLL 배포는 실행 중 Client를 사용자가 종료한 뒤 진행해야 한다. 현재 생성/설치 파일
hash와 시각은 `out/CharacterSelectLoader20260915/verification.receipt.json`에 기록했다.
사용자 실제 Character Select 입장·화면 확인은 아직 하지 않았다.

### 문양 조사와 적용 상태

같은 FLOOR12를 쓰는 외딴 원판은 11개이며 각각 ARCH01A 한 개와 짝이다. 원판의
normal 픽셀은 중앙 흰 원판과 같지만 ARCH01A는 실제 UV/원본 diffuse 대조에서
참고 이미지 중앙의 8각 별, 파란 삼각 칸, 금속 점, 사각/마름모 타일에 대응한다.
이 조각은 현재 중앙에 없다. 별 후보의 원본 transform/실물/UV 진단은
`out/CharacterSelectFloorHypothesis20260915/central-star-candidate.json`과
`central-star-source-uv.png`에 있다. 이 로딩 교정 단계 이후 중앙 별 설치는 G12에서 진행했다.

현재 흰 원판과 갈색 원판의 겹침 표본 중 97.85%에서 갈색이 위이며 중앙 평면 간격은
1.848421cm다. 실제 가림 구조의 수치 증거이지 누락된 모든 장식의 원인 확정은 아니다.
물방울·삼중 곡선·외곽 장식은 각각 원본 메시/재질을 대조 중이다. 후보 발견·식별·
설치·사용자 화면 판정을 구분하며, 조사만으로 전체 바닥 복원 완료라고 쓰지 않는다.

## G12-V. 발탄 신규 Effect 제품 연결 철회와 검증용 문서

사용자는 이번 full restore 연결을 아직 육안 검증하지 않았으며, 검증 후 항목별로
제품 승격을 지정하겠다고 정정했다. `CurrentWiringWithScreenPost`와 `CurrentWiring`의
두 typed canonical transaction을 역순 conditional inverse로 되돌렸다. 현재 값이 당시
after와 같은 cue만 previousCues로 복귀하고, 이번에 삭제했던 V2 binding 44개만 stable ID로
복원했다. 외부 미커밋 변경을 baseline 전체 덮어쓰기로 교체하지 않았다.

현재 제품의 `effect.valtan.action.*.full.restore` cue는 **0개**다. V2 binding은 **102개**이며
전체 JSON이 최초 `CurrentWiring/v2.baseline.json`과 일치한다. 이 결과는 이전 작업 중간의
40/56구간 연결 기록을 현재 상태로 해석하지 않도록 정정하는 것이다. 역전 증거는
`out/ValtanPriorityRestore20260915/CurrentWiring.inverse/commit_receipt.json`과
`CurrentWiringWithScreenPost.inverse/commit_receipt.json`이다.

검증용 128개 Effect 문서와 catalog는 보존했다. 최신 `Effect_Playback`,
`Effect_DocumentCodec_PortableRuntime`, `Effect_ArtistMaterial`을 별도 OBJ로 컴파일해
실제 `Load / Validate_Drawable / Stage_Document`로 **128 성공, 0 실패**를 확인했다.
원본 FreezeMovement와 collision event는 실제 PhysX static ground·원본 particle을 사용한
74개 검사에서 bounce 수, 접촉 위치·회전 동결, 색·크기·수명 진행, 같은 occurrence의
파편 발생 및 종료를 확인했다. 이는 Client 화면이나 최종 시각 품질 검사가 아니다.
원본 modulate portal은 기존 Alpha/Add 및 V2 Multiply를 보존한 별도 native Multiply pass로
교정했다. portal 2개는 out 후보이며 설치 128개에 포함하지 않는다. 이 후보까지 합친 실제
Load/Validate_Drawable/Stage_Document는 130개 통과했다. 실제 D3D WARP 116개 검사에서
Dst×Src, HDR, RT0 alpha와 RT1/2 보존 및 기존 blend control을 확인했고, 잘못된 shape/profile을
거절하는 admission 19개도 통과했다. 제품 cue와 catalog에는 portal을 추가하지 않았다.
세부 근거는 `out/ValtanPriorityRestore20260915/ModulateProbe/validation_receipt.json`이다.

원본 FilmNoise/ZoomBlur 4개는 fresh MIC/shader map 및 native program 입력을 대조하고
기존 native ScreenPost carrier의 실제 enable을 연결한 검증용 문서다. 2,412개 CPU 검사에서
source 시간 창의 ScreenPost 생성·종료를 확인했다. ScreenPost 자료 생성은
`Tools/EffectPipeline/build_valtan_screen_post_library.py`, 근거는
`out/ValtanPriorityRestore20260915/ScreenPostLibrary/`에 있다. GPU 화면 동등성은 미확인이다.

## G13-V. Sequence의 발탄 선택과 Follow Camera 거리·각도

`CSequencerTool`의 Sequence Boss 목록에서 Valtan을 제외하던 조건을 제거하고 기존
`CValtanActionWorkbench`로 연결했다. 기존 canonical Pattern/Stage와 8개 lane을 같은
선택·draft·Save owner로 연다. 별도 제품 문서를 복제하거나 신규 full restore cue를
자동 추가하지 않았다. MainApp의 preview owner 회수·활성 조건도 Sequence target을 처리한다.
이는 쿠크 전용 World/Logic 연출을 발탄으로 복제한 기능이나 모든 native 컷씬 연결 완료가 아니다.

Follow Camera에 `Camera distance (m)`, `Camera pitch (deg)`, `Camera yaw (deg)`를
노출했다. `Set_OrbitAroundFocus`는 기존 주시점을 보존하면서 실제 eye offset을 재계산한다.
거리만 변경해도 확대·축소 구도가 달라지며 FOV, 캐릭터 배율, follow response는 보존한다.
실패하면 profile 전체를 보존하고 기존 JSON pose/lens와 Save freshness 경계를 사용한다.
저장돼 있던 네 맵의 카메라 JSON은 이번 조절 기능 추가로 변경하지 않았다.

실제 profile 및 session OBJ를 연결한 비UI 검사 **56개**가 통과했다. 발탄/쿠크 관문/Character/
Object 전환의 실제 owner 호출과 draft 보존, 거리 변화의 eye 이동, pitch/yaw 변경의 focus 유지,
잘못된 값 거절과 복귀를 확인했다. 변경 MainApp/SequencerTool/ArenaCameraProfile TU 컴파일도
통과했다. 근거는 `out/FullMapRestoration20260915/IsolatedCompile/session_camera_probe.log`다.

추가 호출자 검토에서 Sequence의 Boss를 바꿔도 공유 Benchmark 창이 활성 상태여서 발탄이
계속 재생되는 정지 누락을 교정했다. `CValtanActionWorkbench::On_WorkbenchDeactivated`는
세션 교체 전에 Master와 원본 Sequence의 실제 재생을 정지하고 pending owner claim을 지운다.
기존 `Stop_ValtanCompositionPattern`은 각각의 원래 preview model을 사용해 두 playlist를
정리한다. 모델 소멸 시에도 상태를 초기화하며 Workbench draft·선택·loop 옵션은 보존한다.
두 변경 TU를 추가 컴파일했고 호출자/일시정지/source-only/모델 소멸 경로를 읽기 검토했다.
이 정지 교정에 대해 Client 입력·재생을 직접 실행한 검증은 하지 않았다.

`F1 → Open Action Workbench → Sequence → Boss: Valtan`과
`F1 → Player Follow Camera → Camera map: KoukuSaydon`이 사용자 확인 경로다.
현재 Client/Server는 사용자가 실행 중이므로 이 새 C++의 제품 EXE/DLL 링크·배포는 하지 않았다.

## G14-V. 원본 발탄 컷씬 카메라의 현재 준비 범위

설치 원본 PS의 streaming package 참조에서 SCENE02A/02A01/02A02/04A/06A/07A 여섯 패키지를
resolve하고 실제 export와 Matinee/actor/animation/Director를 읽었다. 총 18개 Matinee가 있으며
그중 Director가 있는 9개를 투영했다. 하나는 상속된 빈 cut track으로 camera가 없고, 나머지
8개에서 runtime 크기 제한에 맞춘 **41개 shot 조각, 1,600 keyframe** 후보를 생성했다.
source 눈금·순서·group/actor ID와 원본 파일 hash를 함께 보존했다.

SCENE06A Matinee53/InterpData113은 원본 24.707817초이며 실제 camera는 23.385742초에
DirGroup으로 반환된다. 현재 제품의 19.867초 등장 카메라는 기존 영상 참고 저작본이고,
이 native 후보로 교체하거나 보스 연출·이펙트까지 통합 재생한 상태가 아니다.
원본 DirGroup cut은 플레이어 시점 복귀이며 CameraActor 누락으로 처리하지 않는다.
이 의미는 [UE3 Matinee Track Reference](https://docs.unrealengine.com/udk/Three/MatineeTrackReference.html)의
Director Track과 대조했다. 기존 공용 camera projector에서 빈 cut track과 DirGroup 반환을 처리했다.

원본 카메라 후보의 clock, 64-key 제한, 유한값, 직교 basis, lens와 플레이어 복귀 구간을
검사한 **6,640개 수치 검사**가 통과했다. 실제 쿠크 2관문 진입 원본으로 변경 전 projector와
비교해 기존 7개 shot의 모든 값이 동일함을 확인했다. 발탄의 플레이어 시점 반환 3구간에는
shot을 만들지 않는다. 근거는 `native-camera-check.json`이며 후보 검사와 제품 재생은 구분한다.

같은 여섯 scene에서 ParticleSystemComponent 127개 발생, 서로 다른 원본 PS 62개를 추적했다.
현재 발탄 검증용 library에 같은 source identity로 존재하는 것은 16개이며, 나머지 46개의
원본 closure도 추출했다. 이 추가 자료에는 emitter 388개와 MIC 185개가 있고 shape는
sprite 331, mesh 42, light 2, ribbon 11, decal 2다. 이 46개의 native 재질 투영·runtime 문서
등록·원본 scene 발생 연결은 아직 하지 않았다. 각 원본 actor, Matinee group/track, instance parameter와
auto-activate 상속 여부를 `native-scene-effect-inventory.json`에 보존했다. 현재 128개 문서의
admission 성공을 이 46개 컷씬 PS까지 복원됐다는 의미로 확대하지 않는다.

근거는 `out/FullMapRestoration20260915/ValtanSequences/scene-inventory.json`,
`actor-camera-inventory.json`, `native-camera-candidates.json`이다. actor·Effect·material parameter·
무드의 native 동시 재생 연결은 후속 구현 범위이며 원본 트랙 추출만으로 full restore 완료로 세지 않는다.
파란 에테르 구슬은 현재 76줄 검정 구슬과 동일 asset으로 확인되지 않았고, 원본 대비 크기 복원이
완료됐다고 판정하지 않았다.


## G12. 원격 조립의 중앙 별을 식별하고 실제 배치

사용자가 나머지 장식도 하나씩 찾아 적용하도록 요청했다. 원격1036 원판과1040 별의
관계를 중앙 흰 원판336에 옮기는 group 행렬로 별의 비율과 방향을 보존했다. 별의
실제 UV와 원본 diffuse는 작은 마름모 타일4개, 파란 삼각 칸, 금속 점, 사각/마름모
중첩 외곽까지 참고 이미지와 대응한다. 이름 ARCH01A만으로 건축 아치로 분류하면 이
후보를 놓칠 수 있다. 별을 찾은 것이 전체 문양의 식별 완료를 뜻하지는 않는다.

기존803개 배치의 모든 행과 원격11쌍을 그대로 보존하고 `editor:LV_LOBBY_CLASSSELECT_SL00:1`
한 개를 추가했다. editor ID1은 기존 MapTool의 ID domain과 발급 형식을 따른다.
최종 position은(-772.022176277,-142.886223812,197.538195165), quaternion은
(-.5,.5,-.5,.5), scale은 전축2.25428036849다. 원판 assembly 관계를 보존한 뒤 현재
앞면이 다리에 묻히지 않도록 Y3.0731624cm를 추가했다. 삼각형 교차1433쌍의 높이
극값과5mm grid174,436표본에서 추가 후 앞면 가림0, 최소 약1cm 간격이다. 이 추가
높이는 현재 조립의 가림 보정이며 원본 중앙에 저작된 위치라고 주장하지 않는다.

기존 WModel과 원본과 SHA가 같은 D/N을 재사용하고 원본 PBR variant
`MAP_EB1DC1086BB5_BG_VOL_COMMON_ARCH01A_SM_FEIYI_OVR_145D4AFC46B8`를 등록했다.
`diffuseBrightness=0`은 MIC chain0의 명시 scalar 값으로, shader 기본값1을 잘못0으로
대체한 것이 아니다. reflection texture1개를 원본 hash 확인 후 Resources에 설치했다.
원격11쌍의 기존 material은 바꾸지 않았다.

Area publisher Validate/Publish/Check가 모두 성공했다. Imported catalog175개,
Authoring/runtime placement804개이며 runtime 기존803행은 byte 내용 기준 보존된다.
실제 MapPlacementDocument::Read로804개를 읽고 같은 scope+Background predicate로
782배치/128모델을 얻었다. 기존 CModel 생성과 MaterialVariant 경로로 새 별을 포함해
**128 성공 / 0 실패**였다. current runtime과 probe snapshot SHA도 일치한다.
127개 전단 결과는 before-star 파일로 보존했다. 추가 C++ 변경·Client 실행·화면 캡처는 없다.

설치 receipt는 `out/CharacterSelectFloorHypothesis20260915/central-star-install.receipt.json`,
배치/재질 근거와 가림 계산은 같은 폴더의 central-star-* 문서, 최종 모델 생성 증거는
`out/CharacterSelectLoader20260915/probe/after-star-verification-summary.json`에 있다.
현재 실행 중인 Client의 종료 후 새 EXE/DLL 최종 빌드·배포와 사용자 입장 확인은 남아 있다.
물방울4개·삼중 곡선·외곽 장식은 원본 package의 미사용 mesh까지 개별 대조 중이며
아직 미식별 항목을 비슷한 형상으로 대체하지 않았다.

## G15-V. 원본 Modulate 포털과 source 충돌 Event 검증

### 실제 완료 상태

원본 `fx_mn_rpbf_00_n.par_n_rpbf_potal_02_01`의 native 2553 재질
`fx_m_mi_05.fx_mi.fx_a_aura_01_1_mo`는 단면 sprite / `blend_modulate`다.
원본 PS `2f6061c6f5abb34494c18bd49d6e7be2`의 출력 선언은 `o0.xyzw` 하나다.
기존 render profile의 0~4 번호와 문자열은 유지하고 단면 Multiply profile과 native
particle pass 5를 추가했다. `Has_ArtistMaterialContract`를 만족하는 source sprite만
해당 profile을 통과한다. 일반 sprite·mesh나 다른 native program으로 바꾼 후보는 거부한다.

원본 material은 particle alpha를 RGB 계수의 지수로 계산한다. 따라서 원본 함수의
결과를 emission gain 없이 RT0에 전달하고 `DstColor / Zero`로 곱한다.
RT0 alpha와 RT1 distortion·RT2 bloom은 쓰지 않는다. 기존 Alpha/Add 및 V2 Multiply
합성식은 보존했다. 기존 1,358개 native 함수와 1,263개 program 행이 그대로인 것을
검사하고 2553만 추가했다. native installer도 동일한 profile/case를 생성한다.

`Load -> Validate_Drawable -> Stage_Document`의 실제 C++ OBJ 검사 결과는
**130 성공 / 0 실패**다. 구성은 catalog에 이미 있던 검증용 **128개**와 새 포털의
neutral/action **out-only 후보 2개**다. 새 후보는 각각
`effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_potal_02_01`과
`effect.valtan.action.420624.stage000.full.restore`다. 포털 후보를 Catalog·Composition에
등록하지 않았으며 현재 제품 full restore cue는 **0개**, V2 102개 binding은 최초
baseline JSON과 동일하다.

### 실행한 검증

| 검증 | 결과와 실제 범위 |
|---|---|
| C++ 최소 컴파일 | Codec, DetailIo, Validation, ArtistMaterial, MaterialBinding, Tool_Helpers 6개 TU 성공 |
| 원본 포털 GPU | 실제 product shader group 2496의 pass를 D3D11 WARP에서 draw/readback. 원본 함수 3회와 기존 Alpha/Add/V2 Multiply 및 source-alpha control 12회, 총 15 draw / 116 검사 성공 |
| 합성 경계 | 알파 0·0.5·1, HDR destination/source factor, RT0 alpha·RT1/RT2 보존, emissive 7.5배 입력에도 factor가 변하지 않는 것 확인 |
| admission 음성 검사 | 19 검사 성공. 기존 ID·token 보존, source 비활성·mesh shape·mesh kind·다른 material program 거부 및 원본 portal particle evaluation 확인 |
| 원본 충돌 | 실제 PhysX static ground에서 74 검사 성공. 5 bounce 뒤 6번째 Kill/FreezeMovement, 위치·회전·orbit 동결, 색·크기·수명 진행 및 최종 소멸 |
| Collision Event | 원본 파편 generator 2개에서 각 4회 collision event, native receiver peak 14개, 종료 후 live 0. 팔별 source occurrence namespace로 다른 팔 receiver와 분리 |
| 문서·도구 | 130 JSON과 vcxproj/filters XML parse, 변경 Python compile, git diff --check 성공 |

검증 근거는 `out/ValtanPriorityRestore20260915/ModulateProbe/validation_receipt.json`,
`gpu.log`, `admission.log`, `all_documents.log`와 기존
`CollisionProbe/validation_receipt.json`, `collision.log`다. GPU 검사는 제어한 texture와
원본 parameter/formula를 사용한 오프스크린 수치 검사이며 Client 또는 장면 화면 캡처가 아니다.

### 남은 범위

- 새 EXE/DLL 통합 빌드·배포와 사용자 화면 판정은 별도다. 현재 실행 중인 Client에 이 소스 수정이 적용됐다고 기록하지 않는다.
- 포털 2개는 out-only이며 도구의 미저장 draft/freshness 확인 후 등록할 수 있다. 제품 cue 승격은 사용자가 지시하기 전까지 하지 않는다.
- THREE 원본 baked edge 3개는 원본 sample에서 폭이 0이다. 강제 폭을 발명하지 않았다. baked history는 stable ID 정렬과 실제 마지막 sample 안의 clamp를 지켜 codec을 통과한다.
- 현재 stage의 원본 branch가 여러 개인 THREE/CROSS는 임의 branch를 선택하지 않았다. `PlayDecalEffect` 직접 notify는 현재 Particle/Trails builder의 완료 범위에 포함하지 않는다.
- 원본 unit/socket 수치, 문서 admission, D3D 합성 검증은 원작 장면의 visual fidelity 승인을 대신하지 않는다.

## G16-V. 사용자 VS 빌드 확인과 미설치 후보 구분

후속 상태 점검에서 Client/Server가 종료돼 있어 미설치 후보의 최신 병합을 준비했다.
이후 별도 Visual Studio 빌드가 실행 중임을 확인해 authoring/project/runtime 변경과
publisher를 실행하지 않았다. 사용자 VS 빌드는 Engine과 Client를 컴파일·링크·배포했고,
Client EXE는 2026-09-15 15:59:41 KST, Engine DLL은 15:45:17 KST에 갱신됐다.
현재 변경과 관련한 16개 실제 TU의 OBJ가 source보다 최신인 것을 확인하고 EXE/DLL 및
로그 hash를 `out/FullMapRestoration20260915/VSBuildAfterReview/verification.json`에 기록했다.
이 확인은 사용자 VS 빌드 결과이며 정본 Product runner를 새로 실행했다는 뜻이 아니다.
Server는 기존 바이너리이며 이번 확인으로 Server 재빌드 성공을 주장하지 않는다.

빌드 직후 사용자 환경에서 Client/Server가 다시 실행됐다. 따라서 다음 자료는 준비 상태이며
아직 설치하지 않았다. 현재 실행 중 편집의 보존을 확인한 다음 최신 저장본으로 다시 합쳐야 한다.

- 쿠크 Composition 805 기준: G3 바주카 WORLD/P35 4구간과 본체·심지 폭탄 V1 두 리소스의
  통합 후보 8파일. `out/KoukuBazookaBombInstall20260915/receipt.json`.
- 발탄 overlay 922배치/305 variant/387 material/새 리소스29개.
  `out/ValtanArenaMaterial20260915/OverlayInstall/stage.json`.
- 발탄 포털 검증용 문서2개와 Catalog/project 등록 후보. 기존 제품 cue 0개를 유지한다.
  `out/ValtanPriorityRestore20260915/ModulateProbe/prepare_portal_registration.py`.

폭탄의 기존 심지 문서는 sprite element 3개와 ModelCue 0개다. 원본 본체는 별도 NPC 모델이며
파티클 복원만으로 생성되지 않는다. 빙고 WORLD 경로가 같은 순수 심지를 본체에 붙이므로
기존 심지 문서에 본체를 넣으면 중복 생성된다. 신규 전체 폭탄은 각각 불꽃 element 3개와
애니메이션 ModelCue 1개로 대기/낙하를 구성했다. 준비와 등록·제품 발생 연결을 구분한다.


## G13-CS. Character Select 전체 Area 배치 로딩

**현재 상태: 전체 범위는 제품 소스에 반영돼 있으며 사용자16:08 빌드에도 포함됐다. 새 재질·조명 복원은 아직 후보이며 미게시다.**
아래 수치는 전체 범위의 구조·모델 생성 검증이며 전체 재질 완료를 뜻하지 않는다.
`out/CharacterSelectFullMap20260915/material-completion-gate.json`을 따른다.

`Client/Private/LevelRegistry.cpp`의 Character Select 중앙 범위를 기존
`MakeFullMapScope()`로 교체했다. Loader와 Level이 동일 descriptor를 소비하며 전체
804배치/135사용모델이 대상이다. 이전 중앙 범위는782배치/128사용모델이며 원격 원판11/별11을
제외했다. 전체 범위와 같은 컴파일 검증본은
`out/CharacterSelectFullMap20260915/LevelRegistry.full-area.candidate.cpp`에 보존했다.
배치·재질·조명·숨김 플래그는 이 범위 후보에서 변경하지 않았다.

### 실행한 검증

- 기존 실제 CModel/Create_MaterialVariant headless probe에서 전체 catalog175/175,
  실제 배치 사용모델135/135 성공, 실패0. 추가 사용모델7개가 모두 통과했다.
- 실제 MapPlacementDocument로 읽은804배치와 같은 문서의 runtime snapshot SHA 일치.
  종전128개 검사 결과는 보존했다. 이번 full-area 카탈로그 실행은 Level 생성이나
  사용자 화면 검증이 아니며 placementCount0 출력은 해당 model-only probe mode다.
- 변경 LevelRegistry TU 격리 컴파일 exit0. OBJ/PDB는 out에만 생성했고 기존 Engine 헤더의
  C4828 경고 외 오류는 없다. project/filter에 이미 등록돼 있어 추가 등록이 없다.
- 관련 diff whitespace 검사 성공. 근거는
  `out/CharacterSelectLoader20260915/probe/full-area-verification-summary.json`,
  `out/CharacterSelectFullMap20260915/levelregistry-compile-receipt.json`이다.

### 다른 맵의 범위 실측

발탄8shard13,184배치, 쿠크3,369배치, 베른23shard50,017배치 모두 현재 full scope를
통과하며 excludedAssetGroupId가 없다. 베른 Landscape42개도 포함한다. 모든 mapset의
선언 수와 실제 행 수가 일치한다. 세 맵은 이번 변경 전부터 전체 범위여서 코드를 바꾸지 않았다.
저작 visible=false는 각각310/523/1,850배치이고 쿠크는 컷씬 finale22/arena461을 추가로
초기 숨김 처리한다. 이 숫자들은 서로 중복할 수 있어 더하지 않는다. 범위 로드 성공을
처음부터 전부 표시되거나 원작 전체 리소스가 복원됐다는 뜻으로 보고하지 않는다.

### 배포와 사용자 관찰

사용자는 중앙 별 장식이 나타났지만 별만 검게 보인다고 서면 확인했다. 형상 표시 확인이며
재질 복원 완료가 아니다. 검정 재질은 원격 _02MIC와 실제 조명 소비를 별도로 조사한다.
사용자는 현재 Client를 계속 사용 중이라고 답했다. 에이전트는 전체 범위를 제품 EXE에
링크·배포하지 않았지만, 후속 검사에서 사용자16:08 빌드의 LevelRegistry.obj에 전체 범위
소스 SHA256이 포함되고 같은 OBJ가 Client 링크 입력에 포함된 것을 확인했다. 당시 EXE는
16:08:11, OBJ는16:08:09, 링크 기록은16:08:12에 갱신됐다. 새 재질은 이 빌드에 없다.

재질 완료를 기다리며 제품 소스를 중앙 범위로 되돌린 보류 처리는 취소했다. 사용자
재빌드에서 이미 포함된 전체 로딩이 사라지지 않도록 검증한 전체 범위 소스와 정확히 같은
바이트로 유지했다. Client/Server를 종료·조작하지 않았고 Test 진입 실패를 별도로
재현하거나 해결했다고 기록하지 않는다.


## F1 도구 버튼 Open 중복 수정

`Client/Private/MainApp.cpp`의 Effect Tool V1/V2 및 World Level Tool 버튼 이름에서
중복 `Open ` 접두어 세 곳을 제거했다. 기존 공통 버튼 함수가 표시 상태에 따라
`Open ` 또는 `Hide `를 한 번 붙인다. 도구 동작과 draft는 변경하지 않았다.
원본 인코딩/개행을 보존했고 해당 MainApp TU 격리 컴파일(exit 0), diff whitespace
검사를 통과했다. 근거는 `out/F1ToolLabels20260915/MainApp.log`다.
실행 중 Client는 유지했다. 현재 EXE 링크·배포와 사용자 화면 확인은 미실행이다.

## G14-CS. 추가 배치의 원본 재질·조명 복원 후보와 적용 조건

이 절은 G18-CS 게시 이전의 조사·후보 상태다. 현재 가까운 배치와 재질 입력의 실제
게시 상태는 아래 G18-CS를 따른다.

사용자는 추가로 로드할 모든 배치의 재질까지 복원한 뒤 전체 맵을 열도록 요청했다.
따라서 원격 원판11개·별11개와 이미 중앙에 옮긴 별1개를 함께 검사한다. 현재 단계는
원본 입력 확보와 out 후보 검증이며, 재질 완료·제품 게시·EXE 적용 완료가 아니다.
검게 보이는 별을 기본 D/N 재질로 바꾸는 임시 후보는 폐기하고 원본 `_02` MIC를 유지했다.

### 실제 누락과 확보한 입력

| 입력 | 원본 대조 결과 | 현재 설치·후보 상태 |
|---|---|---|
| 원판·별 표면 | 두 `_02` MIC의 D/N/detail/ORM/2D reflection 10 binding, 8 unique DDS의 원본 일치 확인 | 원본 scalar/vector/switch를 유지한 후보. diffuseBrightness0도 원본 활성 값이며 임의로1로 바꾸지 않음 |
| 구운 조명 | 원격22개 모두 StaticMeshComponent native suffix에 Type2 RNM 존재 | 원판11개의 기존 입력은 원본 일치. 별11개와 중앙 복제본의 RNM12행을 후보에 추가 |
| RNM texture | 6 atlas pair의12파일 원본 대조. 별의10×10영역과 원판의30×30영역 구분 | 기존 Resources 파일 재사용. 배치마다 UV scale/bias와 RGB decode scale 보존 |
| 환경 반사 | 원격22개 모두 HDR02 override와120도, 원판·별별 tint 확인 | HDR02 원본6면×9mip를 out DDS로 추출,54 payload hash 왕복 일치. 현재 제품의 HDR01 전역 값으로 대체하지 않음 |
| 환경광 계수 | HDR02에 직렬화된 IncidentLightingSH9의 RGB27개 확보 | native7-vector packing 및 engine 환경광 상수와의 연결은 미확정 |
| 반사 계산표 | Engine.PBRPreintegratedGF owner와 native 생성 함수 export 확인 | 원본 payload/생성식 미확보. 현재 프로젝트 적분 LUT를 원본이라고 표시하지 않음 |

원본 component의 `lightEnvironment=null`은 RNM이 없다는 뜻이 아니었다. native suffix를
끝까지 읽고 배치의 LightMapType에 맞는 shader를 골라야 한다. 중앙 복제본의 조명 후보는
원격1040의 atlas·UV·배율을 함께 옮긴 것이며 중앙 위치에서 새로 구운 조명은 아니다.

### 후보의 데이터 범위

`out/CharacterSelectStarBlack20260915/Remote23Candidate/`에는 기존 Authoring/Imported를
기준으로 새 별 RNM variant6개와 배치 재연결12개를 준비했다. Catalog175→181,
material213→219, placementLighting369→381이며 전체804배치의 ID·transform은 그대로다.
기존11원판 RNM과 기존369조명행도 보존했다. 후보 생성은 제품 파일 교체가 아니다.

최신 재검증에서 소스4파일과 후보4파일의 hash가 모두 일치했다. 23배치의 표면 texture
115연결과 RNM46연결을 원본에 결합하고, 신규 별 variant6개를 실제 CModel로 생성해
6/6 성공과 material override 각1개를 확인했다. probe 전용 out catalog에 publisher와
같은 v5 material 선언을 적용했으며 제품 catalog를 잠시 교체하지 않았다.
근거는 같은 후보 폴더의 `source-restoration-compact-receipt.json`이다.

환경 후보와 근거는 `out/FullMapRestoration20260915/CharacterSelect/EnvironmentAudit/`에
있다. source tint의 alpha를 shader additive floor로, import AdjustBrightness2를 추가
runtime 배율로 해석할 근거는 아직 없다. 현재 PS skylight와22배치의 baked GUID가
일치하지 않아 hemisphere를0 또는0.15로 강제할 근거도 확보하지 못했다.
HDR02의 minimumRoughness는0.050000000745다. RNM만 준비한 후보의 기존값0.04는
환경 통합 시 원본값으로 함께 바꿔야 하며, 이 부분까지 반영된 후보로 보고하지 않는다.

### 원본 Pixel Shader와의 수식 검증

실제 RNM permutation의 별 shader `fb32088fb3d8d343a3d1db1c4deb5d1a`와 원판 shader
`82f66791c7d2d249b9c993b51ca86845`를 추출했다. 기존 간접광 계산에는 원본 SH 곱,
반사·확산 에너지 분배, hemisphere 및 extra ambient 항이 빠져 있었다.
`SourcePBRIndirectCandidate.hlsli`에 이 수식을 복구하고 D3D11 WARP에서 원본 DXBC와
동일 입력을 실행했다. 별64조건·원판64조건, 총8,192 pixels에서 실패0/nonfinite0,
최대 절대 오차4.47035e-08이다. 기존 indirect algebra는 별64조건 모두 불일치했다.

이 검사는 통제한 texture/constant를 사용한 수식 동등성 검사다. 실제 원본 LUT,
native 환경광 입력이나 최종 Client 화면의 동일성을 증명하지 않는다. 원본 CB 행과
입력 계약은 `pbr-lightmap-shader-contract.md`, 실행 결과는
`pbr-indirect-verification-summary.json`에 기록했다. 제품 HLSL/API는 추가하지 않았다.

### 실제 정점과 직업별 무대 대조

원본 VS와 현재 제품 VS 본문을23배치/9,091정점으로 WARP stream-output 실행했다.
UV1·atlas UV 오차는0, 위치 오차는1.49e-08m이며 source+Z→runtime+Y 방향을 확인했다.
일반 정점 법선 차이는 최대0.803도다. 별의 거의0인 tangent3개가 UModel/WModel에서
정규화되어 최대33.819도 차이가 났지만 연결된 윗면 면적은0.001195%다. 이것을 별 전체의
검정 원인으로 단정하지 않았다. 세부 원인과 검증 조건은
`out/CharacterSelectStarBlack20260915/VertexNative/vertex-contract.md`에 있다.

원격11쌍은 원본 PS의 `PLAYER_CLASS_*` Matinee→카메라와 프리뷰 캐릭터 위치에
대응한다. 단순 보관·교체용 청크로 단정했던 설명은 교정한다. 현재 중앙에 옮긴1040은
HUNTER, 참고 이미지의 UI와 연관된 FIGHTER는1054다. 원본 위치·재질·카메라 대조는
`StarOriginalAssignment/original-star-assignment-summary.md`에 기록했다.

PS와16개 streaming package의17,660 sequence 객체·584 material track에서는 별의
재질 교체나 중앙 이동 지시를 찾지 못했다. 원격11별의 직접 `_02` MIC 지정은 확인했으나
참고 화면의 실제 위치·MIC까지 증명한 것은 아니다. 각 원격 별10m반경의 SL00 static
배치는 원판과 별2개뿐이다. 다른 streaming 또는 native 생성 객체의 부재로 일반화하지
않는다. 물방울4개·돌출된 삼중 곡선은 계속 미식별이며 추가22개 로드를 그 복원으로 세지 않는다.

### 미완료와 현재 적용 상태

원본 BRDF lookup payload/생성식, SH9의 native packing, 환경 tint alpha·회전의 binding,
hemisphere/ambient owner가 아직 확정되지 않았다. 원본 VS의 UV·방향 전달은 검사했으나
native packed tangent의 완전 보존에는 위 차이가 남는다. 원본 EFEngine DLL은 보호된 정적 image여서 export 주소만으로
반사표 생성 함수를 복구하지 못했다. DLL이나 원작 게임을 실행하지 않았다.

사용자가 지정한 `C:/Users/user/Desktop/로스트아크_렌더링`을 조사했다. 파일13개는 모두
PNG이며 수치 dump나 lookup texture가 없다. `캐릭터선택_비교2_FOV55.png`와
`CharacterSelect.png`를 열람해 원본 중앙 별의 밝은 금속 표면·청회색 홈과 주변 문양을
비교 기준으로 확인했다. 화면 이미지만으로 원본 엔진의 반사 계산표나 환경 상수를
정확하게 역산했다고 주장하지 않는다.

전체 범위는 사용자 빌드의 기존 기능을 유지하도록 `LevelRegistry.cpp`에 반영했다.
새 재질 publisher·제품 빌드는 수행하지 않았다. 원본 재질까지 복원한다는 요청의 완료
조건은 아직 충족하지 못했다. 현재 실행 중 Client/Server와 저작 작업을 종료·조작하지 않았다.
추가 입력의 정확한 연결 후 기존 CModel→CMaterial, Area publisher, 동일 Level scope
경로로 적용해야 하며 완전 복원 또는 사용자 visual PASS로 기록하지 않는다.


## 쿠크 폭탄·바주카 실제 등록 후 상태

사용자가 미저장 쇼타임 편집을 Save하고 등록 동안 편집 중지를 확인했다.
최신 저장본815에서816으로 해골 폭탄 전체/낙하 V1 리소스2개와 G3 바주카를
등록했다. World1848→1849, 기존 presentation resource181개와 사용자 패턴 편집을
보존했다. 바주카는 최신 P35 source notify를 재결합한5개 WORLD 구간이다.
Map/Composition Publish·Check 및 KoukuSaydon owner의 Product/Map/World/Gameplay
게시가 모두 통과했고 설치 검증31개 실패0이다. 근거는
`out/KoukuBazookaBombInstall20260915/verification.json`, `owner-publish.log`다.
폭탄은 본체1 ModelCue와 심지3요소, scale3의 전체/낙하 리소스이며 자동 전투 spawn
추가는 아니다. Effect Tool Refresh Index로 전체 Effect를 선택해 확인한다.
바주카의 기존 Level World cache 갱신에는 쿠크 재입장이 필요하며 Composition Reload
후 새 Complete Play로 서버에 새 revision을 제출한다. Server 재시작은 필요하지 않다.
Client/Server를 종료·실행·조작하지 않았고 새 EXE 빌드는 수행하지 않았다.
최종 화면·방향·크기 판정은 사용자 확인 대상이며 발탄의 별도 승격 보류는 유지한다.

### 쇼타임 Save 복구와 최종818 반영

사용자 새 Duration은 Save 재시도 후817에 정상 저장됐으며37.685초/16.05초 값과
전체 사용자 draft를 백업했다.818에는 해당 Duration의2초 주기·이동속도50% 서버
타게팅 및 바주카5구간을 합쳤고 Product/World/Gameplay 명시 발행은 PASS다.
그룹 XYZ 회전 및 Client/Server 코드 최소 컴파일도 완료됐다. 에이전트는 제품 빌드나
Client/UI 조작을 수행하지 않았다. 상세 근거와 사용자 빌드·화면 경계는
[쇼타임 G20 결과](../09-12/2026-09-12_KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT.md#g20-통합-사용자-duration-보존과-플레이어별-쇼타임-연결)에 둔다.

## G18-CS. 중앙 인근 11쌍 배치와 재질 입력 게시

사용자는 원격 바닥이 실제로 보인다고 확인한 뒤 검은 표면·점 같은 반사를 지적했다.
이후 목적지를 실제 Bern이 아닌 Character Select 중앙 섬 근처로 확정하고 직접
EXE를 빌드하겠다고 했다. 이 변경은 사용자 요청의 가까운 배치와 확인한 재질 입력을
실행용 데이터에 게시했다. 원작과 완전히 동일한 재질·그림자 또는 visual PASS는 아니다.

### 게시한 범위

- 원격 원판11개와 별11개, 총22개의 position만 중앙 동쪽(+X)으로 옮겼다. 중앙에서
  60~102.956m, 중심 간격14m의4×3 배열이며 마지막 칸 하나는 비운다. 전체804배치의
  stable ID·quaternion·scale·visible은 보존했다. 중앙에 추가한 별1개는 이동하지 않았다.
- 바닥 상면은 Y=-142.7024386으로 맞췄다. 기존 높은 중앙 바닥보다5cm 높으며 pair마다
  같은 평행이동을 적용해 원판·별의 원래 상대 높이와 방향을 보존했다.
- 첫 원판1036의 pivot은(-712.017422,-143.197882833,183.537832)이다. 전체 배열은
  X=-712.017422~-670.017422, Z=183.537832~211.537832에 있다.
- 별의 원본 `_02` MIC/RNM variant6개와 조명12행을 연결했다. 원격 별11개와 중앙 별1개가
  대상이다. catalog175→181, material213→219, placementLighting369→381이다.
- 원판11개와 별12개의12개 material variant에 원본 HDR02 cube/RGB tint와 minimum
  roughness0.050000000745를 연결했다. 원본6면×9mip DDS54 payload는 변경 없이 사용한다.
  기존 원판 RNM11개와 원래 조명369행은 보존했다. diffuseBrightness0도 원본대로 유지한다.

정본은 해당 Area의 Imported catalog, Authoring placement/material과 MapCatalog다.
기존 Area publisher로 Validate/Publish/Check를 실행해4파일을 게시했다. Runtime
placement SHA256은 f4f0bf05411ce64d40f95e48c5c85a0048f63dcfc9d9229faa974f55e24b574e다.
이번 변경은 C++/HLSL을 추가하지 않았고 EXE/DLL 빌드·교체도 수행하지 않았다.

### 실제 검사

- 전체804배치의58개 실제 WModel 기하로 위치를 검사했다. 조립체 사이 최소 간격7.242m,
  기존 일반 기하와 보수적 최소 간격36.193m이며 기존 삼각형과1m 이내 접촉 후보는0이다.
  하늘 구체 shell은 최소41.265m 떨어져 있고 검사한 중앙 시점의165개 시선과 교차하지 않았다.
  전체 카메라 각도의 가시성을 증명하는 검사는 아니다.
- 게시본22좌표를 검증한 layout과 다시 대조하고,23배치의 material/RNM/environment 연결을
  확인했다. Area publisher의 LF 정규화 후 모델·재질 입력은 실제 CModel probe와 동일하다.
- 변경된12개 variant를 실제 CMapAssetCatalog→CModel/CMaterial로 생성해12/12 성공했다.
  cube와 BRDF의 실제 SRV dimension 검사도 이 생성 경로에서 통과했다.
- 대표 원판1036의19,904표본과 별1040의14,889표본에서 환경 없는 현재 PBR 계산을
  재현했다. 정확한 RGB0 비율은 원판94.036%, 별98.072%였고 HDR02를 연결한 뒤 각각
  0%,0.00672%였다. 밝기 중앙값은0에서0.017709/0.039768로 증가했다.
- 실제 texture에서 얻은128개 입력의 D3D11 WARP 산술 검사는 nonfinite0/negative0,
  최대 CPU 오차1.49012e-08이다. 통제 카메라·CPU texture sampling 기반이며 전체
  raster/shader/postprocess나 사용자 화면과의 동일성을 검사한 것이 아니다.

### 남은 재질·화면 경계

현재 프로젝트 BRDF lookup과 기존 간접광 수식을 유지한다. 원본 engine lookup,
SH9→packed SH7, hemisphere/ambient owner는 아직 복구하지 못했다. 원본120도를
현재 shader sin/cos 규약으로 변환했으며 native binder 부호는 미확정이다. component
alpha1을 additive floor로 사용하지 않고 기존 프로젝트 floor0을 유지했다.

환경 반사가 넓은 검정 영역에 기여하는 것은 수치로 확인했으나 기존 강한 반사점은
남는다. 모자이크·반짝임 완전 제거 또는 원본 참고 이미지와 일치한다고 보고하지 않는다.
이동한 정적 RNM은 원래 조립체의 tile이며 새 위치에서 다시 구운 조명·그림자가 아니다.
미식별 물방울·곡선 장식의 복원과11쌍의 이동도 구분한다.

현재 Client52080/Server40364는 사용자가 실행한 상태로 유지했다. Client 재시작 후
Lobby→Character Select에서 새 데이터를 읽는다. F1→World Level Tool 검색창에
`map:12666232865287822104`를 넣고 해당 행의 `Focus Origin (F)`를 누르면 첫 원판으로
카메라를 맞출 수 있다. 배열은 기존 Server navigation 밖의 시각 검사용 배치이며
플레이어 이동 범위를 확장하지 않았다. 사용자가 EXE 빌드·재실행과 화면 판정을 담당한다.

배치 근거는 `out/CharacterSelectFullMap20260915/NearbyLayout/nearby-layout.json`,
게시·최종검증은 `out/CharacterSelectStarBlack20260915/NearbyRepair/published-receipt.json`,
재질 수치는 같은 상위 폴더의 `ActualEnvironment/partial-environment-assessment.md`에 있다.

## G19-CS. 중앙 바닥 교체 UI와 원본 축소 mip 복구

사용자가 직접 빌드하기로 한 상태에서 F1 교체 기능과 재질 축소 입력을 반영했다.
이 절의 완료는 소스·리소스 반영과 아래 범위의 검증이다. EXE/DLL 링크·설치와 Client
화면 조작은 수행하지 않았다. G18-CS의 원본 BRDF·SH·hemisphere 미복구 경계도 유지한다.

### 11개 선택과 중앙 배치

- F1의 `Arena Camera / Player` 바로 아래 `Character Select Floor Swap`을 추가했다.
  `Floor`에서11개 직업 무대 중 하나를 고르고 `Apply floor`를 누르면 원판과 별이 함께
  중앙에 배치된다. `Restore original floor`는 교체 전 중앙 원판·별로 되돌린다.
- 이11개는 같은 원판 geometry와 같은 별 geometry, 같은 두 원본 MIC를 사용한다.
  배치별 RNM의 atlas6쌍·UV·RGB decode scale은 서로 다르다. 동일20,000표본/원판의
  flat-normal RNM 평균 휘도도0.64505~1.74483으로 달랐다. 서로 다른 모델11종도,
  조명까지 같은 완전 복제11개도 아니다.
- 조립체의 source floor→중앙 floor 행렬을 별에도 적용해 상대 위치·방향·크기를
  보존한다. 기본 높이 보정은+0.2m다. 실제 기하31,102표본에서 다른 바닥보다1cm
  위에 놓이는 최소 보정0.19154553m를 확인했다. 모든 시점의 무겹침을 증명하지 않는다.
- 추가 XYZ offset과 yaw를 조절할 수 있다. 기본 `Center floor environment`는 기존
  중앙 원판의 cube/BRDF/tint/회전/minimum roughness를 재사용한다. `Source stage
  environment`는 원래 HDR02 입력을 유지하고 현재 shader 방향식에 맞춰 조립체 yaw를
  환경 각도에서 뺀다. 이 부호는 현재 프로젝트 shader 기준이며 원본 native binder
  규약을 새로 확정한 것은 아니다.
- 현재 직접광은 새 world position을 소비한다. 원본 RNM tile은 조립체와 함께 보존하며
  중앙 위치에서 다시 구운 그림자가 아니다. 환경 선택만으로 모든 원본 환경광 계수가
  복구되는 것도 아니다. UI에 실제 적용한 값과 이 한계를 표시한다.
- 교체는 현재 맵 방문 동안 유지한다. 원래 가까운11쌍 전시와 저작 배치는 유지하며,
  맵 재진입은 저장된 기본 배치를 읽는다. Server navigation·충돌·플레이어 위치는
  수정하지 않는다. 캐릭터 customization 중에는 적용·원복을 거부한다.

목록 정본은 `Data/Rendering/Authored/CharacterSelectFloorSwap.json`이다.11쌍의 stable
source ID와 중앙336/기존 별1의 숨김 대상, 임시 배치9000001/9000002, 기본 offset을
선언한다. Client의96.DataFiles 아래 None 항목으로 등록했다. Debug Level이 프로젝트
Data root에서 읽으며 누락·손상은 해당 UI 상태에 표시하고 Level 입장을 막지 않는다.
목록 변경 중 활성 preview가 있으면 먼저 원복하도록 거부한다.

### 기존 런타임 경로와 실패 처리

기존 `Level_CharacterSelect -> MapPlacementRuntime -> CMapAssetObject -> CModel ->
CMaterial` 경로를 사용한다. 원판·별2개를 숨긴 상태로 준비한 뒤 중앙2개의 visibility를
변경한다. 실패하면 이전 객체·선택 ID·설정·visibility를 보존한다. 선택별 material은
기존 `CModel::Create_MaterialVariant`로 분리하고 component lookup도 같은 model을
가리킨다. 원래 모델 prototype이나 전시11쌍의 material을 바꾸지 않는다.

검토 중 empty override를 material variant로 전달해 원본 override가 소실되는 경계와,
Layer 삽입 뒤 entry 문자열 할당 실패 시 신규 객체가 남는 경계를 교정했다. 필요한
entry/status는 commit 전에 준비한다. 이전 preview는 숨긴 후 제거한다.

### 모자이크·반짝임 입력 검사와 설치

관련8개 DDS는 mip0만 있었고 축소용 원본 mip74개가 빠져 있었다. 원본 패키지와
기존 LostArk v7 UModel decode 경로로 각각 회수해 총82mip를 설치했다. mip 수는
11/11/11/10/11/7/10/11이며 총 bytes는4,459,520→5,945,832다. 기존8개 mip0의 BC
압축 payload는 모두 byte 동일하고 색공간·normal 형식도 유지했다. 원작 설치 패키지는
수정하지 않았다. 범용 Crunch decode는 CRC가 맞아도 원본 mip0와 달라 채택하지 않았다.

8개 경로는 SL00 material139행/catalog64개가 공유하므로 축소 입력 개선은 새22배치에만
한정되지 않는다. 기존 최고 해상도 색·무늬를 교체하거나 normal/roughness를 임의로
약하게 만든 변경은 아니다. 파일별 경로·hash·백업은
`out/CharacterSelectStarBlack20260915/ShaderMosaic/installed-source-mips.json`에 있다.

현재 source를 동일 FXC 옵션으로 컴파일한 `Shader_VtxMeshBinary`,
`Shader_VtxMeshMapInstance`, `Shader_Deferred`3개는 설치 CSO와 byte 동일했다.
디스크의 오래된 shader 문제는 이번 검사에서 재현되지 않았다. 실행 중 GPU shader를
읽어 대조한 검사는 아니다. mip 누락은 축소 aliasing에 기여하는 실제 입력 결함이지만
사용자가 본 모든 모자이크·강한 반사점이 사라졌다고 기록하지 않는다.

### UI 접기와 문자 표시

`Arena Camera / Player`, `Character Select Floor Swap`, `Player Follow Camera`,
`Kouku UI Preview (Debug)`, `Mario Controls (Debug Jump)`,
`Find the Real Saydon - Sight Collider`, `Clown`을 독립 CollapsingHeader로 정리했다.
패널을 닫아도 기존 시야 collider 상태를 원복하지 않는다. 요청한 깨진 한글 항목은
`Return to Start`, `Find the Real Saydon - Sight Collider` 등 ASCII로 바꿨다.
기존 font owner의 Korean range/Malgun fallback은 유지했다. 프로젝트 전체의 모든
한글 encoding·glyph 표시를 고친 것으로 확대하지 않는다.

### 검증과 빌드 경계

| 검사 | 결과와 범위 |
|---|---|
| 변경 CPP4개 최소 컴파일 | MainApp/Level_CharacterSelect/MapPlacementRuntime/MapAssetObject 오류0. out OBJ만 생성. MainApp의 기존 C4828 경고329개는 유지 |
| 실제 Level 함수·JSON·변환 | production Level 본문과 실제 CDataJson/MapAssetCatalog/MapPlacementDocument/DirectXMath, 설치804배치로441 checks PASS.11쌍×2환경22 apply, RNM byte 보존, 상대 행렬 최대 오차0.000183105 |
| 교체·원복 실패 주입 | production Replace/Clear 본문37/37 PASS,200회 교체 후 원복. create/visibility/remove 소비자는 stub이며 실제 Layer/GPU 실행과 구분 |
| 실제 CModel material 생성 | base12개, Source/Center variant24개 성공. 잘못된 cube12건 거부 후 기존 material 보존 |
| DDS 설치 뒤 모델 검사 | 영향을 받는12개 variant CMapAssetCatalog→CModel 생성 성공 |
| 실제 D3D11 DDS/SRV |8개 texture/SRV mip 수 일치. 전체82개 GPU subresource readback BC bytes가 설치 원본 mip와 동일 |
| 환경 방향식 | 현재 HLSL에1024방향×7 yaw 대조, 최대 오차5.55112e-16 |

Level/변환 근거는 `out/CharacterSelectFloorSwap20260915/runtime-implementation-receipt.json`,
`input-transform-receipt.json`, `material-variant-probe-result.json`이다. UI와 transaction은
`out/CharacterSelectFloorSwapUi20260915/ui-implementation-receipt.json` 및
`transaction-probe/result.json`, mip/GPU/CSO는 위 ShaderMosaic 폴더의 영수증을 따른다.
위 수치 검증을 제품 EXE 링크·실제 Client 장면·사용자 visual PASS로 대신 기록하지 않는다.

사용자가 Debug EXE를 빌드하고 Client를 재실행한 뒤 Lobby→Character Select→F1에서
`Character Select Floor Swap`을 펼친다. 기본 중앙 환경으로11개를 순서대로 Apply하고
별의 검정·축소 반짝임·바닥 교차를 확인한 후 Source 환경과 비교할 수 있다. Resources는
이미 교체했지만 실행 중 로드한 GPU texture는 자동으로 갱신되지 않는다. 원본 BRDF/SH/
hemisphere와 미식별 물방울·곡선 장식, 최종 화면 일치는 계속 미완료다.

최종 readback에서 runtime6파일·UI2파일·DDS8개의 hash가 각 검증본과 일치했다.
새 JSON11항목과 project/filter XML의 각1개 등록을 확인했고 scoped `git diff --check`는
오류0이다.18:31 KST의 Client/Server/빌드 프로세스는 모두 없었으며 설치 Client.exe는
사용자의17:24 빌드 상태다. 새 F1 기능은 사용자의 다음 Debug 빌드에 포함된다.
`out/CharacterSelectFloorSwap20260915/final-readback-receipt.json`에 기록했다.

## G20-CS. 사용자 화면 재검토, 원판 높이·큰 바닥·밝기 비교

### 사용자 관찰과 원인

G19 후 사용자 첨부에서 검은 원판·별, 사라진 큰 링과 회색 빈 곳이 계속됨을 확인했다.
따라서 G19의 수치/생성 검증을 사용자 화면 복원 완료로 취급하지 않는다. 원판과 별을 함께
0.20m 올린 설정은 큰 링을 가리는 회귀였다. 사용자 additional -0.17은 실제+0.03이며
실제 삼각형501×501 XZ 감사에서 큰 링482의 상향6,284표본을 모두 덮는다. 원판을 원래
높이로 되돌리면 같은 링의74.4%가 원판 위에 남는다. 나머지는 bevel/교차 부분이다.

원판336의 폭15.23359m와 작은 링439의 폭7.00377m 비율은 첨부와 맞아 단순 scale 문제의
근거는 없었다. 별은 bridge 위로 별도 높이 보정이 필요하다. preset offset=[0,0,0]으로
변경하고 `starOffsetMeters=[0,0.030731623924803,0]`를 별에만 적용했다. 11개 별의 최상단은
기존 중앙 clone 높이와 최대1.07052e-7m 차이다. 추가 XYZ/yaw는 조립체 전체에 적용한다.

### 큰 바닥488 표시와 환경 입력

`LV_LOBBY_CLASSSELECT_SL00:export:488`, stable ID10084275097207127157은 폭20.406m의
실제 바닥이다. 원판 바깥 상향표면142.31m²가 있으며 `LV_MODULE_MESH03_512`라는 이름으로
분류돼 visible=0이었다. 새 원본 가시성 추출은 actor/component·CDO를 따라 이 배치를
visible로 확인했다. EFNavMeshTerrain=true는 바닥 숨김의 근거가 아니다.

해당 한 배치의 visible을1로 바꾸고 transform은 유지했다. 위치는
(-772.018594,-143.037832,197.537422), scale=(4.00083828,0.102707773,4.00083828)이다.
asset ID는 `MAP_94E1EF196C1A_LV_MODULE_MESH03_512_OVR_75EF6B0F204F_FR_1863F638A2D5`다.
원본 floor17_02 재질과 atlas421 RNM을 보존하고 원본 override HDR01 cube를 연결했다.
HDR01은6면×8mip의48개 source payload 일치가 확인됐다. BRDF LUT는 기존 프로젝트 수치
적분본이며, 미기록 angle0/minimumRoughness0.04 적용은 프로젝트 근사로 남긴다.

488이 쓰는 normal BM01_03_n_old와 ambientreflection_10a의 누락 mip을 공용 추출기로
각10/8단계 회수·설치했다. mip0 압축 byte는 이전 원본과 동일하다. SourceMaterial/RNM
확인, 가시성/환경 변경, mip 설치와 실제 CModel1/1 성공 기록은
`out/CharacterSelectStarBlack20260915/Module488/`에 있다. Area Validate/Publish/Check는
804배치·4문서로 통과했다. 전체 hidden 배치를 일괄 활성화하지 않았다.

### 검정과 F1 비교 스위치

원격 무대 MIC의 diffuseBrightness0은 실제 직렬화된 값이고, 원본 uniform expression과
DXBC도 이 값을 바탕색에 곱한다. RNM/SH를 올리거나 Source/Center 환경만 바꿔도 바탕색0은
남는다. 실제 texture 입력192조건의 원본 DXBC 수치 대조는 최대1.91e-6 차이, 실패0이었다.
원작의 실행 중 MIC 변경 주체/값은 아직 확인하지 못했다. 부모/메시 기본 MIC와 원격 무대
MIC는 여러 값·shader family가 달라 단순 교체하지 않았다.

사용자가 승인한 `Compare shader default brightness (1)`을 F1의 Character Select Floor
Swap에 추가했다. 기본 OFF는 원본0이며 ON 후 Apply는 preview 원판·별의 복사 재질만1로
바꾼다. Reset은 OFF이고 적용된 상태를 별도 표시한다. 원본11쌍·catalog·공유 shader를
바꾸지 않는다. 실제 원본 밝기1 복원으로 기록하지 않는다.

| 검사 | 실제 결과 |
|---|---|
| MainApp/Level 최소 CPP 컴파일 | 각각 오류0, out OBJ만 생성. 기존 C4828 경고 유지 |
| production Level/804배치/JSON |1,148 checks PASS.11쌍×2환경×OFF→ON→OFF의66 apply, source RNM·나머지 surface byte 보존 |
| 실제 CModel material variant |12base×2환경×3상태72개 생성 성공. missing cube12건 거부, 원본/이전 설정 보존 |
| 밝기·기하·자료 | `G20Brightness/brightness-implementation-receipt.json`, `LayerAudit/height-correction-receipt.json`, `G20Black/` 원본 수치 근거 |

EXE/DLL 제품 링크·Client 실행·화면 캡처는 하지 않았다. 사용자는 새 Debug 빌드 후
Lobby→Character Select→F1→Character Select Floor Swap에서 Reset adjustments를 누르고
바닥을 선택해 Apply한다. 밝기1 체크 후 다시 Apply해 비교한다. 원판과 장식이 만나는
가장자리, 큰 바닥488의 실제 범위·재질과 물방울/곡선 장식의 원본 동일성은 사용자 확인이
남아 있다. 현재 무대를 모두 원작과 똑같이 복원했다고 결론내리지 않는다.

## G21. 공통 추출·WModel·source material 경로 수정

### 연결한 공통 경로

- 원본 placement schema3는 actor/component→archetype/CDO의 숨김·가시성 근거와
  navigation 참여를 따로 보존한다. LV_MODULE/nav/water/FX 이름은 진단 힌트만 남기고
  visible을 결정하지 않는다. 최초 이름 기반4분류는07-30의852d81a 추출 workflow에
  함께 추가된 규칙이었다. 특정 팀원이나 원작 엔진의 숨김 의도를 원인으로 단정하지 않는다.
- 실제 SL00 재추출은803 source배치, visible779/hidden24, property/unresolved 오류0이다.
  현재 저작804개는 이 원본803개와 editor 추가1개의 차이다. source488은 visible이다.
  schema3는 map variants, scene, Bern inventory·shard로 이어진다. v1/v2는 unrecorded로
  구분하고 명시적인 legacy preview 외에는 근거가 있다고 취급하지 않는다.
- Bern/map variant cook이 기존 geometry helper를 호출해 glTF의 실제 추가 UV·COLOR0·
  tangent.w를 WModel에 보존한다. source/cook/file hash와 최종 채널을 검사한다.
  UPK→glTF 동일성, native pivot과 shader 연결은 자동 인증하지 않으며 없는 채널을 만들지 않는다.
- 공용 `extract_source_map_material_parameters.py`는 full source path, MIC 부모 상속,
  child override와 명시된0/false/null, native static set·parameter provenance를 보존한다.
  기본값이 직렬화되지 않았으면 unresolved로 남긴다. numbered FName 및 v868의
 29byte normal parameter는 도구 전용 adapter로 읽고 공유 parser 전역은 변경하지 않는다.
- `extract_ue3_texture_mips.py`는 redirect와 원본 BC1/BC3/BC5 mip chain을 회수한다.
  원본 package는 변경하지 않으며 mip0/hash/전체 chain 확인 후 출력만 교체한다.
- `extract_source_map_component_lighting.py`는 원본 component의 RNM·GUID·환경 태그와
  원본 tail을 공용 형식으로 읽는다. SL00은RNM799/LOD없음3/vertex lightmap 미지원1이며
  malformed failure0이다. 기존799 RNM 전수와 원격22개·488의환경/GUID가 일치했다.
  미지원563은 원문/이유와 PARTIAL_UNSUPPORTED로 남기며 조명 없음으로 바꾸지 않는다.
  미기록 instance 환경 태그와 외부 TextureCube의 아직 읽지 않은 minimumRoughness를 구분한다.
- `build_source_map_materials.py`는 기존 mapmaterials v2와 placementLighting을 생성한다.
  실제 slot/MIC·texture 색공간/mip/hash·원본 RNM/환경 근거를 검사하며 unknown branch,
  잘못된 값과 null texture를 자동 fallback으로 숨기지 않는다. native expression 근거가
  있는 null fallback만 source path/parameter/index/hash를 맞춰 사용한다.
- scene은 compiler 결과와 입력·Resources를 재검증하고 같은 compiler로 재생성해 대조한다.
  legacy source-only field는 slot/MIC/정확한 필드 coverage만 보완한다. 모든 scalar나
  render flag를 family 지원만으로 일괄 승인하지 않는다. staged output 승격 실패는 이전
  catalog/placement/material/receipt를 보존한다. 배포는 기존 Area publisher다.
  RNM은 실제 WModel TEXCOORD1, vertex-color overlay는 COLOR0를 요구하며 기존 geometry
  parser로 payload를 읽어 검사한다. 메타데이터나 이름만으로 채널 존재를 대신 인증하지 않는다.

### 실제 검증과 남은 범위

공용 resolver의 실제 floor17_02/arch01a_02/base PBR3개 추출과 unit6개, mip 도구의
redirect10mip 전체 byte 일치와 unit6개를 확인했다. 잘못된 mip0/없는 material을 섞은
실패는 기존 output을 보존했다. Bern 공통 cook/reader41tests와 geometry helper12tests,
실제 converter→WINT1.4 fixture의 UV1/UV2/COLOR0/tangentW 보존이 통과했다.

큰 바닥488과 별 재질2개를 새 공용 compiler로 생성한 결과는 설치된 material row와
모든 필드가 동일하다. 해당 결과의 실제 CMapAssetCatalog→CModel2/2 생성과 scene의
동일 compiler 재생성·pinned input/resource 검증도 통과했다. 근거는
`out/CharacterSelectStarBlack20260915/SharedMaterialCompiler/`와
`out/CharacterSelectFloorSwap20260915/PipelineAudit/`에 있다. 제품 재질을 새 사본으로
재설치하거나 다른 Area의 미저장 authoring을 덮어쓰지 않았다.

원본 graph의 끊긴 edge와 동적 MIC/scene SH·hemisphere/native BRDF는 미완료다. 특히
source hit_color는 native uniform에 남으므로 unrelated switch가 OFF라는 이유만으로
미사용 처리할 수 없다. 새 도구가 모르는 field는 계속 실패 근거로 남긴다. 이 변경은
지원되는 입력의 공유 추출·연결과 재발 방지이며 모든 원작 shader/모든 맵의 시각 복원
완료를 뜻하지 않는다. 공통 명령·입력·지원 범위는 Tools README와 팀 Area guide에 반영했다.

최종 공통 검사는 LevelPlacementExtractor의관련9suite105tests, Bern 전체41tests,
geometry helper12tests가 통과했다. componentLighting을 실제 compiler 입력에 연결해
source배치3개의RNM ID/pair/scale/bias도 재검증했다. editor clone의 RNM을 source ID로
재해석한 첫fixture는 의도대로 거부됐고 실제 source 행만 포함한 출력으로 수정했다.
이는 검증fixture 조정이며 제품 clone/RNM 삭제가 아니다. 상세 최종 hash와 범위는
`out/CharacterSelectStarBlack20260915/SharedMaterialCompiler/final-validation.json`에 기록한다.

출력과 receipt 두 번째 승격 실패 시 부분 commit이 남던 저장 경계도 공용
`source_extraction_io.write_pair`로 연결했다. mip/parameter/component/compiler 성공 경로가
같은 stage/rollback을 사용하며 parser 실패의 receipt-only 정책은 유지한다. 새5tests는
두 번째 stage·replace 실패와 이전 output 부재의 원복을 검사했다. 실제4종 출력 SHA는
변경 전후 동일하다. 이는 보고된 I/O 오류의 rollback이며 두 파일의 OS crash 원자성을
주장하지 않는다. 마지막 상태 확인에서 Client35788/Server49472는 사용자의19:37 실행으로
유지 중이었다. 새 EXE 링크는 하지 않았으며 사용자가 Client 종료 후 Debug 빌드를 수행한다.

## G22. 사용자 후속 화면과 마지막 팔각별 보정

### 확인된 큰 바닥과 별의 변경

사용자 첨부2269a5ba 화면에서 큰 바닥이 표시되고 중앙 팔각별은 검게 남았다. 채워진 영역은
G20의 source488 visible 복구와 일치한다. 현재 저작의488은 원본 transform 한 개이며,
11쌍은 이전 동쪽 전시 배열이다. F1도 한 쌍씩만 중앙 preview를 만든다.
이름 기반 숨김은852d81a(2026-07-30 `feat: add map tool editor workflow`)의 추출 도구
분류였다. Git author는tnestyle70이며 이 기록만으로 실제 지시자나 의도를 확정하지 않는다.

사용자는 팔각별을 이번에 마지막으로 수정하고 다음 화면에서도 검으면 제거하라고 지시했다.
이에 실제 배치가 사용하는 정확한 ARCH01A _02 재질6행의 diffuseBrightness만0→1로
바꿨다. 중앙 clone1개·원격별11개와 기존 F1 preview가 이 값을 소비한다. 배치되지 않은
legacy base행은0으로 유지한다. 원본 MIC0과 shader 기본값1의 구분 및 원본 추출 근거는
그대로 보존한다. 이 변경은 프로젝트 저작 보정이며 원작의 동적 MIC값 복원 주장이 아니다.

다른 material field, DDS, RNM, sourceMaterial, geometry와 transform은 이 별 보정에서
변경하지 않았다. F1의 OFF/적용 설명은 authored brightness로 수정했다. 비교 ON은 계속
원판과 별을1로 만든다. 실제 CMapAssetCatalog→CModel에서6개 별의 brightness1과6개
원판의 기존0을 읽었고,12base/72재질variant 생성·12missing-cube 거부와 기존material
보존을 확인했다. MainApp 최소 컴파일은오류0이며 제품 EXE/DLL은 링크하지 않았다.
근거는 `out/CharacterSelectStarBlack20260915/G22Star/`다.

### 아직 진행 중인 범위와 사용자 확인

왼쪽 아래 사각 부조의 원본 mesh 동일성·배치, 외곽 첨탑 material 입력과488을 포함한
navigation 재베이크는 아래 후속 결과에서 각각 기록한다. 위 모델 생성 검사를 Client
표시 성공으로 취급하지 않는다. 새 빌드에서 별이 계속 검다는 사용자 확인이 오면 제거한다.

### 좌하단 사각 부조의 식별과 네 번째 배치

첨부 부조는 `bg_elg_filenysusm_a.mesh.bg_elg_filenysusm_statue01f_sm_dodo`의 앞판이다.
실제 앞판6삼각형 UV와 원본 D/N 부조가 사진과 대응하고 설치 geometry는 추출 receipt와
같다. 원본 SL00에는383/391/393 세 배치만 있으며 네 번째 숨김 배치는 없었다.
따라서 source383을 ring439 중심에서Y180도 돌려 신규 editor ID2로 배치했다.
위치는(-772.00625,-142.68,200.986836), scale=(-1,1,1)이며 기본 카메라 yaw135도에서
좌하단이다. 기존 세 장식과 같은 원본 메시·재질을 재사용하므로 세 개를 제거하지 않았다.

앞판2,823표본에서 기존 바닥/링에 묻힌 지점0이고 최소 링 위 여유는1.439cm다.
source383의 RNM atlas/UV/RGB scale을 새 editor ID에 복사했다. 이는 원본 조각을 사용한
대칭 저작 보완이며, 원본에 없는 네 번째 placement나 독립 baked lighting을 발견한
것으로 기록하지 않는다. 현재 authoring은805배치·사각 부조4개다.

이 장식의 D/N/S mip chain을 각각11/11/10단계로 회수했다. mip0 압축 byte는 기존과
같고 실제 D3D11 WARP SRV/staging readback32/32mip가 일치했다. STATUE01F의 기존
RNM variant2개도 실제 CModel 생성에 성공했다. 원본 texture32mip 설치와 geometry/
가림/카메라 방향은 `out/CharacterSelectFloorSwap20260915/G22Decoration/`에 기록한다.

### 새 바닥의 네비게이션 게시

원본488과 네 번째 부조를 포함한805배치에서 실제 `CNavGridBaker`로318대상·484,542
삼각형을 베이크했다. 0.5m셀에서 기존2,176개를 전부 보존하고192개(48m²)를 추가해
2,368walkable이다. 새488지지면1,288셀은 모두 보행 가능하며 연결 component는1개다.
장식의 낮은 경사면 때문에 막힌21셀은 실제488높이를 v3 navpaint에 기록했다.
추가192셀 전부를 player 반경0.45m/높이1.8m와 실제 삼각형 SAT로 대조했고 최대step
0.6m 위의 장애물 교차0이다. 현재 실제 최대 연결step은0.47402954m다.
새 바닥 바깥에 분리된 외곽38셀은 차단했고 원격11쌍을 보행 영역으로 연결하지 않았다.

공식 publisher에 optional `-AreaId`를 연결해 CS만 Validate/Publish했다. 생략한 기존
전 Area 동작과 unknown ID 무쓰기 거부를 보존했다. player4/disabled Valtan spawn
검사와 기존ContractTest가 통과했고 Client/Server navgrid가 동일한 bytes로 게시됐다.
타Area runtime hash 변경은0이다. 실행 중 Client/Server는 종료하지 않았으며 재시작해야
새 navigation을 사용한다. 근거는 `out/CharacterSelectFloorSwap20260915/G22Navigation/`다.

### 외곽 첨탑350배치의 재질·RNM 연결

첨부의 어두운 세로 조각은 BG_GDOGODS_PILLAR01B/C 두 모델이다. 원본 atlas의 균열과
테두리 무늬가 대응하며 현재350배치는 legacy D/N 경로만 사용하고 mapmaterials 행이 없었다.
원본 PBR MIC `bg_gdogods_b.mat.bg_gdogods_pillar01_mi_kjs`와15 RNM atlas pair를28개
기존 CModel material variant로 연결했다. geometry/transform은 그대로이고 catalog209개,
placement805개다. source scalar·native null texture fallback·원본 금속mask를 보존했다.

금속mask는 양쪽 tint 같음·saturation1·reflection OFF·외부 배율 같음의 검증된 조건에서
기존 nonmetallic/metallic brightness로 대수 변환한다. 조건이 다르면 compiler는 거부한다.
steady emissive는 `emissive.flicker.mode=none`으로 명시했다. 기존 sourceBgFlicker의3을
PBR 전용 steady로 사용하고 기존 PBR0/1/2의 nested·BG0/1/2 의미와 ABI를 보존한다.
parser→기존 map surface binder→공용 HLSL과 publisher/compiler를 함께 갱신했다.

원본 D/N/P6개 경로에 각각11/11/10mip를 회수했고 emissive6mip·반사10mip와 미설치
RNM atlas685/686의4개 texture를 연결했다. 총12개 Resources 변경이며 원본 mip0는
보존했다. exact null emissive는 native expression1의 t_tds_specular04다. 원본 AO0도
보존되어 간접 diffuse가0인 조건은 남는다. 이번 입력 연결이 화면을 원작처럼 밝게 만든다는
뜻은 아니며 source SH/동적 MIC/전체 조명 복원 완료로 확대하지 않는다.

실제 CModel28개에서 RNM·발광·steady3·AO0·metallicBrightness1.2를 확인했고 같은
모델의 기존mode0 variant28개 및 mode 없는 JSON1개도 통과했다. 실제 제품 발광 함수와
원본 금속mask 대수식을 실제 texture 표본으로 대조한 GPU128표본의 최대 오차는 각각
7.92e-9/2.98e-8, nonfinite0이었다. 이는 원본 PS 전체 실행이나 Client 화면 판정이 아니다.
관련 compiler13tests와 CPP2개 최소 컴파일도 통과했다. 근거는 G22OuterMaterials에 있다.

### 통합 검증과 사용자가 미룬 최종 게시

공식 publisher Validate의 정규화 출력4개를 out에 stage해 별6·부조2·큰 바닥1·첨탑28,
총37개 실제 CModel 생성과 RNM/재질 getter 검사를 모두 통과했다. 첫 통합 probe는
authoring catalog v4를 runtime으로 사용해 material declaration을 놓쳤고, 후속은 공식
publisher가 만든 v5 정규화 출력으로 수정했다. 제품 파일을 우회 수정한 것은 아니다.
shader include의 실제 소비자인 MeshBinary/MapInstance/AnimMeshBinary3개도 out FXC
컴파일 오류0이다. 기존 FxCompile read tlog가 공용 include를 모두 추적하므로 project 등록
추가는 필요 없다. 제품 CSO·EXE·DLL 설치나 실행은 하지 않았다.

초기 Area Publish는 실행 중 파일의 Move→rollback 단계 잠금으로 실패했고 기존 runtime
mapmaterials hash를 보존했다. 사용자는 Client 종료를 기다리지 말고 다른 세션 작업 뒤
한 번에 게시·빌드하겠다고 정했다. 따라서 소스·Resources·nav 게시까지 완료하고, 새
805배치/209catalog 및 material runtime 게시와 새 빌드는 사용자 후속 단계로 남긴다.
현재 runtime map은 이전804배치/181catalog이며 이 작업의 최종 재질이 적용됐다고 말하지 않는다.
나중에 현행 authoring을 공식 게시기로 검증·게시하는 명령은 다음과 같다. 별도 EXE 실행,
프로세스 종료, 자동 빌드 훅을 추가하지 않았다.

```powershell
powershell -ExecutionPolicy Bypass -File out/CharacterSelectStarBlack20260915/G22Final/Publish-CharacterSelect.ps1
```

Server navigation은 이미 새 파일로 게시됐어도 실행 중 Server는 이전 메모리 상태다.
새 Client 빌드와 Server/Client 재시작 후 사용자가 별의 마지막 보정·좌하단 장식·이동을
확인한다. 다음에도 별이 검다는 사용자 확인이 오면 별을 제거한다.

최종 통합 readback은 `out/CharacterSelectStarBlack20260915/G22Final/final-validation.json`에
기록했다.209catalog/805배치·별6재질·부조4배치·첨탑350배치와 공식 publisher의 v5
정규화 출력,37개 실제 CModel,Client/Server nav bytes를 대조했다. 별도 게시 스크립트의
PowerShell parse, 기존 project/filter XML2개와 변경 JSON parse, scoped diff 검사를 통과했다.
Navigation의 AreaId 생략 전체5Area/쿠크4region Validate도 통과해 기존 호출을 보존했다.

## G26. 발탄·베른 맵 shadow 비용, 발탄 밝기와 베른 진입 — 2026-09-16

### 확인한 병목과 소스 반영

사용자 저장본 `Client/Bin/ProfilerCaptures/profiler_20260916_104620_729_frame12_67768_0.json`은 12프레임 중 유효 GPU 8프레임의 평균이 142.518ms다. Shadow 112.852ms가 약79%이며 NonBlend 10.944ms, Lights 4.999ms, Render.Blend 0.038ms다. CPU 평균140.627ms이나 세부 scope 261318개가 누락돼 CPU 세부 합계를 병목 증거로 쓰지 않았다. 마지막 카운터는 draw8223, instanced4929, map placement13184다. 복원 이펙트의 입자 렌더링이 주 병목이라고 단정할 근거는 없었다.

`Uses_OpaqueShadowPass`가 source material을 사용하는 완전 불투명 DEFERRED 정적 맵 표면만 선별한다. PBR3/4는 alphaMasked가 없을 때, SOURCE_SPECULAR_OPAQUE5, SOURCE_OVERLAY7/SOURCE_BG8은 mask flag64가 없을 때 허용한다. source off, fade, masked, 식생/캐릭터와 미확인 vertex 변형은 기존 경로를 유지한다. MapStaticBatchObject는 기존 depth-only pass21~23을 재사용하고, MapAssetObject는 binary shader의 새 pass20~22를 사용한다. 허용된 mesh는 Bind_ShadowMaterial과 재질 texture/RNM 계산을 생략한다. geometry, preScale, world/projection 곱셈 순서, cull mode와 draw count는 유지했다. 이는 두 맵이 공유하는 비용 절감이며 실제 FPS 상승률은 아직 측정하지 않았다.

변경 CPP3개 개별 컴파일과 binary/instance 전체 FX 컴파일이 통과했다. 실제 전체 셰이더를 사용한 headless D3D11 WARP의 3264조건/835584 depth값은 기존 경로와 불일치0, nonfinite0, D3D오류0이다. 실제 두 맵의 family7/8 flag65조합, source on/off, fade, alpha, 3개 cull과 반전·비균일·shear basis를 포함한다. authored material coverage는 발탄10160배치, 베른16182배치에서 하나 이상 적용 가능하지만 light/frustum culling 전 집계이므로 실제 draw 절감 수로 보고하지 않는다. 증거는 `out/MapShadowPerf20260916/{capture_summary.json,source_material_coverage.json,verification.json,result.json}`이다.

### 발탄 밝기

RenderingProfiles revision49→50에서 활성 `scene.valtan.cool-low-key.v1`만 exposure1→0.73, base/두 region bloom threshold→2.74, intensity→0.5로 조정하고 공식 publisher의 Validate/Publish를 통과했다. 원본 LUT와 source tone 식, 비교용 source-rendering profile, 다른21개 profile/전역품질/광원은 유지했다. LUT 표본의 검정 출력은0이며 단독 white floor를 확인하지 못했다. 이 변경은 사용자가 보고한 밝기·번짐을 줄이는 프로젝트 튜닝이지 원본 값 복원이나 장면 픽셀 검증이 아니다. 증거는 `out/MapFrameAndValtanBrightness20260916/brightness-profile-change.json`이다. 실행 중 Client의 메모리 profile을 자동 reload하지 않았다.

### 베른 진입과 bootstrap 소비

`Client/Bin/Debug/Diagnostics/client-session-61448.jsonl`은 Server 승인 뒤 약250.295초에 target-level-create가 실패한 기록이다. `loading.complete`는 실패 원인이 아니라 activation 요청 이름이었다. 실제 MapEffect91배치/MapLight315개는 parse에 통과했지만, installed Resources를 사용하는 Product 준비에서는 물보라 `par_d_fallsplash_w1_001`과 `par_d_fallsplash_w3_001`의 `particlemodulevelocitycone`이 거절돼 9/11개만 준비됐다. Loader가 격리한 실패를 Bern Initialize의 필수 ambient transaction이 거절하는 재현 가능한 차단점이다. 이전 로그가 모든 Initialize 원인을 구분하지 않으므로 다른 실패가 전혀 없었다고 주장하지 않는다.

기존 codec/spawn carrier에 exact VelocityCone의 scalar angle/velocity, 유효 direction과 emitter-space 모드를 연결했다. 원본85~100cm/s와 direction(0,0,1)을 한 번만 좌표·단위 변환한다. angle0~10을 degree로 해석한 수치 재구성이며 원작 random 알고리즘/원본 실행 코드 동일성까지 확인한 것은 아니다. world-space velocity, 추가 owner scale, vector 분포와 zero direction은 계속 거절한다. Bern Initialize는 실제 실패 단계의 recovery를 먼저 기록하고 기존 first-recovery 소비자가 generic MainApp detail의 덮어쓰기를 막는다.

별도로 Client presentation reader의8192행 제한을 기존 Server/publisher32768행과 같은 Shared 상수로 통일했다. 실제23555행 bootstrap의 admission 실패를 제거하고, 엄격한 unsigned 파싱 및 선언 행 수와 payload 일치를 검사한다. 이 결함을 Bern map-effect 실패와 동일한 원인으로 기록하지 않는다.

Client CPP4개·Server CPP1개·Shared CPP 개별 컴파일과 publisher PowerShell parse가 통과했다. 실제11/11 ambient가 준비·10초 CPU 재생에 통과했으며, cone velocity 표본6449개의 속도와 각도가 입력 범위 안이었다. 2배 크기/90도 회전에서도 기존 root basis가 한 번 적용됨을 확인했다. 미지원8개 입력 거절, 실제23555행/32768행과 malformed·초과·부족 bootstrap14개 검사가 통과했다. 상세 근거는 `out/BernEntry20260916/RESULT_NOTES.md`와 해당 폴더의 native 결과다.

### 빌드·화면 경계

에이전트는 실행 중 Client/Server를 종료하거나 제품 EXE/DLL을 교체하지 않았다. 사용자가 현재 Product 빌드를 직접 수행 중이다. 위 기록은 최소 컴파일·shader/depth/데이터 검증이며, 새 제품 빌드 성공·실제 베른 입장·개선 후 FPS·밝기 및 원작 시각 일치의 완료 기록이 아니다. 사용자 새 profiler를 같은 위치·설정으로 저장하면 기존 프레임과 비교할 수 있다. 작은 오망성과 독립 진입 포탈은 09-13 RADIAL MOTION RESULT의 후속 절에서 별도로 기록한다.

## G27. 발탄 후속 캡처의 정적 depth 재사용과 local light 비용

`profiler_20260916_155102_918_frame26_62472_0.json`의 CPU frame 평균은85.231ms, frame interval은86.565ms(11.552fps), 유효 GPU22개 평균은86.746ms다. Shadow54.724ms(63.1%), NonBlend20.001ms, Lights4.570ms이며 shadow VS11,602,859회다. 기존 G26 이후 shadow PS는 약573만→98만으로 줄었으나 geometry 재제출은 그대로였다. camera-visible601/placement13184로 컬링은 이미 작동한다. CPU scope403405개 누락 때문에 GPU elapsed만으로 순수 GPU 연산과 CPU 제출 지연의 비중을 확정하지 않는다.

### 공통 소스 반영

Engine의 기본 거절 `Try_GetStaticShadowRevision`을 통해 완전 불투명·비 morph인 기존 map batch만 정적 depth 캐시에 참여한다. light view/projection, source-material 설정, ordered weak owner/control block과 instance revision이 같을 때 이전 정적 depth를 복사하고 동적 caster를 이어 그린다. transform·visible·bounds 변경, 구성 변경, source 모드, scene replacement와 실패는 캐시를 무효화한다. 추가 texture 생성 실패는 기존 매 프레임 경로를 유지하며 weak owner는 객체 수명을 연장하지 않는다. 캐시 copy가 그림자 해상도·depth format·geometry·sampling을 바꾸지 않는다.

fallback MapAssetObject는 실제 light volume 밖이면 shadow 제출을 생략한다. morph와 source-character43의 vertex displacement는 bounds 검사에서 제외한다. Deferred local light는 최종 attenuation이 정확히0인 pixel에서 뒤의 재질/BRDF 계산을 생략한다. 실제 광원 개수·범위·순서와 map visibility는 유지한다. Engine 정본과 Client shader 사본을 함께 변경했다.

### 수치 검증

- 실제 Renderer의 Render_Shadow/Ready_Shadow_Resources 본문을 추출한 headless WARP 비교:97검사,557056 depth값, 비배경538210값, bitwise 차이0, D3D오류·경고0. cold/hit,동적 ghost,revision·light·source·scene 변경,추가/삭제/순서,실패/재시도,queue append,weak lifetime 및 같은주소의 다른control block을 포함한다. 실제 맵 대신 통제한 caster를 썼으므로 실제 맵 FPS 증거는 아니다.
- 실제 map 함수6개 기반 admission/culling:45검사 실패0. mutable morph·masked/fade·source off·revision overflow와 보수적6평면 경계를 포함한다. 설치 발탄 asset/material 기준 캐시 후보3384batch·8711배치·3615submesh·14,197,926indices는 light culling 전 집계이며 실제 절감량은 아니다.
- 실제 전체 Deferred baseline/candidate FX를 hardware와 WARP에서 각각2176조건·163,931,136 FP16 channel 비교:차이0,nonfinite0,D3D오류0. 통제한1280×720·22point·9쌍 표본의 좁은범위 조건6.083→0.709ms는 shader fixture 결과이고 제품 Lights4.570ms나 FPS에 곧바로 환산하지 않는다.
- Renderer/MapStaticBatchObject/MapAssetObject 개별 Debug 컴파일과 전체 Deferred FX 컴파일 통과. 제품 빌드는 아래 통합 빌드 절에서 별도 기록한다.

근거는 `out/ValtanPerf20260916/{CaptureAnalysis,ShadowCache,ShadowAdmission,LightEarlyOut}`다. 실제 사용자 후속 캡처는 아직 없으며 새 FPS를 측정한 것으로 보고하지 않는다.

## G28. 베른 직접 입장과 Create Character transaction

Bern만 pending/created identity가 없으면 admission 이전에 실패했다. CharacterSelectionState의 pending Bern identity 우선은 유지하고, pending이 없을 때 Valtan/Kouku의 기존 created→AUDITION 선택을 공유하도록 수정했다. Lobby와 F1은 Server 승인 경로를 그대로 사용한다. 선택 class가 없으면 기존 Lance Master 기본값, created identity가 없으면 Test-<PID>를 사용한다. 직접 입장을 created identity로 만들지 않으며 실제 Bern Change_Level 이후 pending commit과 실패 cancel은 그대로다.

실제 CharacterSelectionState.cpp와 현재 Shared Debug.lib를 사용한23검사 실패0, Level_Lobby 최소 Debug 컴파일 통과다. 초기 입장,선택class,pending우선,잘못된nickname,취소,commit,기존created재진입과 미지원world의출력보존을 확인했다. UTF-8-noBOM/CRLF와 무관한 dirty 변경을 보존했다. AGENTS 및 팀핸드북의 direct Bern 계약을 함께 갱신했다. 근거는 `out/BernDirectEntry20260916/`이며 실제 Client 화면 입장은 사용자 확인 영역이다.
## G29. Character Select 최적화의 네 레벨 연결

Character Select/Bern/Valtan/Kouku의 Loader map 준비, registry scope, MapPlacementRuntime,
MapStaticBatchObject/MapAssetObject, Model/Material/Shader, player animation, Effect playback 및
local light 실제 호출자를 대조했다. 배치·camera/light 컬링·GPU LOD·동일 instance 업로드 생략,
geometry/texture/shader 입력 공유, pose/palette 및 Effect 평가 재사용은 공통 경로다. 이를
Character Select에만 허용하는 레벨 조건은 발견하지 못했다. 감사 근거는
`out/ValtanPerf20260916/CaptureAnalysis/common-level-optimization-audit.md`다.

실제 누락은 Level_Loading의 선택 player Effect 사전 준비 조건이었다. Bern도 기존
Queue_ProductCues_Priority와 로딩 worker의 target set에 포함하고 Effects 진행 표시와
BERN label을 연결했다. boss별 준비 조건, optional 실패 격리, epoch/cancel과 activation gate는
유지했다. 기존 Bern은 Character spawn 뒤 공통 incremental worker로 등록하므로 이는
입장 후 cold preparation을 로딩 단계로 옮긴 변경이며, 측정한 첫 스킬 지연 수치는 없다.

Bern의 far-plane 여유/hysteresis와 맵별 scope·재질 admission·LOD threshold를 유지했다.
revision50의 기본 Bern 및 Kouku profile은 shadow가 꺼져 있으므로 G27의 캐시가 이 설정에서
실행되지 않는 것은 연결 누락이 아니다. Character Select와 Valtan은 켜져 있다. 공통 최적화가
연결돼도 맵의 배치 수·재질·광원 부하는 달라 실제 FPS가 같아지는 것은 아니다.

## G30. 베른 Map Effect admission 거절 수정

후속 `client-session-62472.jsonl` generation4는 Server 승인 뒤185.238초에 bern.map-effect admission이 실패했다. Initialize는 Change_Level 이전에 실행되므로 current가 LOADING인 상태에서 BERN 소유 descriptor를 제출했고 Spawn_LevelPlacement의 active-level 검사가 거절했다. 이 검사를 통과시켜도 queued Spawn의 SOURCE_LOOP 조건이 level-owned를 금지해 즉시 Spawn_Immediate와 불일치하는 두 번째 거절이 있었다.

MapEffectPresentationRuntime은 임시 admission probe만 current==target 또는 current==LOADING을 허용하며 그 current layer에 잠깐 생성한다. 모든 probe는 반환 전에 Stop하며 실제 m_iLevelIndex와 활성화 후 spawn의 target은 유지한다. 잘못된 두 번째 sample 등 중간 실패에도 앞서 생성한 후보를 정리한다. 다른 활성 level의 요청과 실제 target-owned spawn의 LOADING 중 제출은 계속 거절한다. queued SOURCE_LOOP는 즉시 경로와 같은 world-root/NATURAL/비 external/비 character 및 level-owned/no-boss 또는 boss-owned/boss 조건으로 정렬했다.

변경2CPP 최소 컴파일 통과. 실제 Build_WorldRoot/Spawn_LevelPlacement/Probe_WorldEffectAdmissions와 두 source-loop predicate를 추출한 검증152개 실패0, owner/policy 조합128개 일치다. 실제 게시된 Bern91행의 LOADING/BERN admission과 전probe정리, foreign level·invalid root·queue/clone/seek 실패·중간 sample 실패를 검사했다. clone backend는 fixture이므로 실제 GPU attach, Client Bern 입장 또는 FPS 성공 증거로 확대하지 않는다. 근거는 `out/ValtanPerf20260916/BernAdmission/`이다.

### G27~G30 통합 빌드와 남은 화면 확인

사용자가 미저장 편집을 저장하고 Client62472/Server51828을 종료했다고 확인한 뒤 실행 중 제품 프로세스가 없음을 검사했다. 공식 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`로 Engine→Shared→Server→Client가 모두 PASS다. 첫 빌드는354.597초, Bern 사전 준비를 추가한 마지막 증분 빌드는8.281초로 성공했고 마지막에는 Level_Loading OBJ1개와 Client binary1개만 갱신했다. receipt는 `out/BuildPipeline/runs/20260916T071531345Z-debug-product.json` 및 `20260916T071624844Z-debug-product.json`이다. 기존 C4819/FXC 경고는 남지만 컴파일·링크 오류는 없다.

Engine DLL, Deferred CSO와 새 GameObject SDK 헤더의 원본/배포 SHA-256 일치를 확인했다. Client.exe 최종 수정 시각은2026-09-16 16:16:24 KST이며 Server는 현재 소스로 up-to-date라 기존 산출물을 재사용했다. 관련 project/filter XML5개 parse와 변경 범위 diff-check도 통과했다. 일반 Product 경로대로 별도 Data publish나 광역 runtime 진단은 실행하지 않았다.

Client와 Server를 자율 실행하지 않았고 둘 다 종료 상태다. 사용자는 Server와 새 Client를 실행해 F1 Level Navigation→Bern 직접 입장, Character Select→Create Character→Bern, 세 맵의 같은 위치 profiler를 확인한다. 실제 Bern 입장·최종 시각 결과·개선 FPS는 아직 사용자 확인 전이며 이 기록을 화면 PASS로 사용하지 않는다.