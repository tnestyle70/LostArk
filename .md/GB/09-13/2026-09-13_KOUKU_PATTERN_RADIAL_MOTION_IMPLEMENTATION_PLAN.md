# 쿠크 도넛 확장·서커스공 분열·손 트레일 구현 계획

## G00. 요청과 현재 근거

사용자 첨부 첫 영상 장면은 백스텝 후 세 갈래 전격, 두 번째는 1관문 쓰리투원투하의 공 낙하·분열과 노란 원형/도넛이다. 추가 첨부는 재물의식에서 오각형을 그리는 손 트레일이다. 정지 이미지의 색·형태는 진단 입력이며 속도와 궤적의 최종 증거로 쓰지 않는다.

현재 발탄 도넛은 manual sprite의 Transform scale과 Linear Lerp로 커진다. 쿠크 native3601의 inner/thickness는 반경 마스크 입력이지만 현재 warning 문서에 그 값의 시간 변화는 없다. 원본 shader의 시간 입력은 caustic UV에 연결된다. 원본 엔진이 inner를 증가시켰다는 호출 근거가 없는 상태에서 임의 곡선을 원본 복원값으로 기록하지 않는다.

현재 원본 Action4219806에는 큰 공 Projectile421980602와 후속 작은 공421980603~607 연결이 있다. 원본을 재사용할 수 있으므로 공 생성·분열을 새 Server 로직으로 합성하는 조건은 아직 성립하지 않는다. 실제 대상 선택과 damage는 기존 Server 소유이며 독립 Effect의 미리보기 배치와 구분한다.

시작 기준은 main e6f19ec806c7f41eacc37553a4d919cb17163d18, clean worktree다. 기능 브랜치는 codex/kouku-donut-ball-motion이다. LAN 동기화는 server-host, TCP7777 방화벽 준비, endpoint not-listening이었다.

## G01. 기존 도넛을 보존하는 확장 버전

`Tools/EffectPipeline/build_kouku_expanding_warning_groups.py`는 현재 쇼타임·칼날댄스의 두 도넛 warning을 읽고 별도 asset ID의 확장 후보를 만든다. 서커스공421980613의 원형1개·도넛2개 예고도 같은 원본 SkillDecal 재질과 실제 반경으로 조립하고 두 도넛의 확장본을 추가한다. 원본 native material/SourceRecipe와 fade는 유지한다. Transform의 XZ만 작은 시작 배율에서1로 보간해 중심과 projector 깊이를 보존한다. LocalDecal이 지원하는 Detail Life를 root 보간 시계로 사용해 fade-out 전에 확장을 끝내며 원본 입자 수명과 fade는 따로 유지한다.

편집 소비자는 기존 Effect Detail의 Transform/Timing/Linear Lerp이며 새 shader나 runtime을 만들지 않는다. 확대본은 프로젝트 크기 보간임을 이름·RESULT에 명시한다. 기존 복원본과 사용자가 저장한 패턴·애니메이션·발생 배치는 그대로 보존한다.

## G02. 백스텝 후 세 갈래 전격

`build_kouku_backstep_electric_group.py`는 MN_RPCT_07 Action4219962의 backstep와 후속 전격 stage, 세 발사 SkillEffect와 실제 Projectile을 대조한다. 현재 P46의4219921 단독 backstep에는 발사 notify가 없으므로 기존 사용자 animation을 자동 덮어쓰지 않는다. enabled3발과 disabled2발을 raw header로 구분한다. 원본 emitter·native material·속도·수명은 보존하며 원본 target-origin과 Grenade arc 미해독 부분은 독립0초·-30/0/+30도 직선 편집용 배치로 명시한다.

## G03. 공 낙하와 후속 분열

`build_kouku_circus_ball_groups.py`는 Action4219806 → 큰 공 → 후속 작은 공의 원본 callback과 occurrence TRS를 해석한다. 원본2자식·6세대·낙하/반동 곡선과 크기를 사용한다. nested CDO의 StartSize·LocationDirect.ScaleFactor에서 미직렬화된 lookup 기본값만 복구하고 disabled 모듈은 운영 projection에서 제외하되 원본 증거에 남긴다. 최대 수명1.5초를 분열 시각으로 선택한 것은 편집 정책이며 원본 거리 도달/수명 종료의 우선순위를 복원한 것으로 기록하지 않는다.

## G04. 재물의식의 손 트레일

`build_kouku_ritual_hand_trail.py`는 작은 오망성의 별 그리기 요소와 실제 손의 Cascade Ribbon2개·sprite1개를 구분한다. 원본 startcontrol/b_wp_1과 설치 CModel의 골격·preScale·clip으로 궤적을 확인한다. 추가 요청에 따라 기존 작은 오망성 drawing8을 보존하고 gray floor5를 제거한 뒤 star_shot7을1.536232초에 연결한다. 원본 action에서 star_cast/star_shot notify가 disabled라는 사실은 유지하며 사용자 요청 독립 조합으로 기록한다. 기존 재생성기도 gray floor를 다시 추가하지 않게 수정한다.

## G05. 검증·등록·실행 준비

후보는 기존 CEffectDocumentCodec의 load/Save roundtrip, CEffectPlayback의 시각 샘플·seek, 실제 리소스 staging으로 검증한다. 도넛은 회전·이동된 root에서 XZ 직경 증가, Y 깊이와 중심 고정을 검사한다. 이동·분열·손 부착은 실제 CPU 위치와 수명, 필요한 실제 CModel을 사용한다. 새 검증 체계를 만들지 않고 기존 out probe를 재사용한다.

검증된 새 Authored 문서만 설치하고 기존 Catalog·EffectResourceTree·Composition의 전역 presentationResources에 append한다. 새 Data JSON은 Client project의96.DataFiles None 및 기존 filters에 등록한다. 기존 pattern occurrence와 사용자의 이름·TRS는 구조/byte 비교로 보존한다. JSON/XML parse와 git diff --check를 확인하고 C++/HLSL 변경이 필요해진 경우 해당 Product Build로 반영한다. Client/UI 실행·조작·화면 캡처는 수행하지 않으며 실행 준비와 수동 선택 경로를 RESULT에 기록한다.

## G06. 추가 요청: 쇼타임 조준점 본체의 미표시

추가 첨부의 빨간 고정 조준점과 흰색·주황색 추적 조준점은 원본에서 각각4개/3개 Sprite와 native2484/2485가 구성한다. 완성 조준점 한 장의 texture를 사용하는 구조가 아니다. 원본 texture3종, source Required/axis-lock, 실제 CPU particle과 최종 정사각형 크기는 이미 연결돼 있다. 고정형은 EPAL_Z가 활성이고 추적형은 axis-lock이 disabled여서 CAMERA_SQUARE다.

WARP의 실제 shader2432/DDS/parameter 검사에서 native2484의 색 출력이 기존 SrcAlpha/One 합성으로 전부0이 됨을 재현했다. 같은 입력의 합성만 One/One으로 바꾸면1904~1948pixels가 생성된다. 원본 native2484는 blend_additive이며 PS가 coverage를 RGB에 미리 곱하고 A=0을 출력한다. 설치된 distortion 동반 dispatch의 early return이 일반 native 경로의 additive coverage=1 처리를 빠뜨렸다. native2485는 기존 상태에서 비영 출력이므로 같은 보정을 하지 않는다.

원본 PS의 수식·Distortion 출력과 기존 material/texture를 보존하고 설치된2484 dispatch의 scene alpha만 기존 SrcAlpha/One carrier에 맞는1로 전달한다. 원본 fade는 RGB에 이미 반영돼 있으므로 유지된다. 재생성기도 동일 계약을 사용한다. 신규 texture·manual sprite를 합성해 원본으로 등록하지 않는다. 기존과 수정된 실제 FXC 결과를 같은 WARP 수치 입력으로 비교하고2485의 불변 및2484의 원본 One/One RGB 결과 일치를 확인한다.

실행 중 Client/Server와 미저장 편집은 유지한다. source와 격리 out의 컴파일·검증까지 먼저 완료하며, Product runner의 실행 중 출력 보호 조건이 해제된 뒤에만 정본 Product Build를 수행한다. 현재 실행 인스턴스에 새 CSO가 자동 적용됐다고 기록하지 않는다.

## G07. 작은 오망성 폭발의 색 합성 누락

사용자의 최종 범위는 Play All의 잘못된 보스 모델 요구와 작은 오망성 폭발만이다. 다른 후보 데이터와
Anchor UI 변경은 보류한다. 작은 오망성은 drawing8 뒤 source star_shot7이 실제 존재하며 native2811
mesh5, native2812 sprite1, native2310 왜곡1의 DDS와 mesh도 존재한다. 실제 CPU에서 shot7 전부
발생하고 양의 색/alpha를 가지므로 요소 누락이나 Spawn 보정으로 우회하지 않는다.

`Shader_EffectArtistNativeDispatchKoukuNativeCases2752.hlsli`의2811/2812는 원본 PS가 coverage를
이미 RGB에 곱하고 A=0을 출력하는 additive 재질이다. distortion 동반 early return이 native A=0을
그대로 SrcAlpha/One carrier에 보내 색이 소거된다. 두 case의 SceneColor alpha만1로 맞추고 원본
RGB 수식, fade, distortion,2811 mesh와2812 sprite carrier, drawing8 및 실제 Data는 보존한다.
재생성기의 nativeBlend별 coverage 처리는 G06에서 이미 수정했으므로 전체 shader를 재생성하지 않는다.

실제 두 carrier의 Debug FXC를 격리 out으로 컴파일한다. 실제 material packet/DDS와 CPU의 색·dynamic
입력으로 기존/수정본 및 One/One 대조를 수치 검사한다. 같은 group의 다른 재질과 왜곡 출력은
보존돼야 한다. Client/UI와 제품 CSO는 실행 중에 교체하지 않으며 사용자 화면 확인은 별도다.

## G08. 원형·도넛의 고정 경계와 확장 요소 연결

사용자가 원형과 도넛 모두 발탄의 예고 장판처럼 경계를 고정하고 채움 영역이 시간에 따라 커지도록 요청했다. 발탄은 물리 요소 세 개지만 쿠크 native3600/3601은 한 요소 안에서 경계와 채움을 함께 계산한다. 동일 native를 세 번 복제하면 경계가 중복 합성된다. 현재 쿠크의 고정 `inner`에 기존 named material parameter curve를 연결하고, 이전 확장 후보의 전체 XZ 확대는 해제한다. 실제 패턴에서 사용하는 기본 warning/impact도 같은 변경에 포함한다.

현재 재질·텍스처 연결과 원본 반경 자료를 재사용하고 기존 Effect document의 `SourceTransformTrack.materialParameterTracks`를 사용한다. 원형의 `inner`는 0→1, 도넛은 `thickness`(내경/외경)→1로 진행하며 projector와 고정 경계는 유지한다. source 시간 원점과 시작 지연을 함께 반영하고 성장 종료를 fade-out 이전에 맞춘다. 효과별 색·반경·fade와 후속 폭발은 보존한다. 원본 재질 수식 재사용과 프로젝트가 저작한 시간 곡선, 기존 neutral engine texture 대체 입력은 구분한다.

변경 소유자는 기존 `build_kouku_expanding_warning_groups.py` 및 관련 warning Authored 문서다. 쇼타임의 실제 합성 warning/impact와 같은 계열의 현재 소비 문서를 대조해 고정된 예고가 남지 않게 한다. 새로운 Effect ID나 C++/shader는 기존 계약으로 처리할 수 없는 경우에만 추가하고, 새 JSON이 필요하면 Catalog/Tree 및 Client의 `96.DataFiles` None 등록을 함께 연결한다.

실제 codec의 Load/Save/Reload와 production CPU playback으로 시작·중간·성장 종료의 고정 경계/성장 반경·중심·깊이·alpha를 확인한다. 기존 폭발 블록과 무관한 사용자 편집은 보존 비교한다. 변경 JSON/XML parse와 `git diff --check`를 실행하며 Client/UI 실행과 최종 시각 판정은 사용자가 직접 한다.


## G09. 쇼타임 공 낙하의 원본 크기·경로 상속 (2026-09-14)

사용자가 지정한 `effect.kouku.gate3.showtime.ball.drop`의5요소는 원본 MissileDrop이다.
그 가운데 숨김 provider38의 LocationDirect.ScaleFactor와 공 mesh4의 StartSize는 원본에
Distribution=None만 저장돼 Engine CDO의 dimensionless1을 상속해야 하지만 현재 빈 lookup이다.
기존 서커스공 G03과 같은 원인인지 실제 두 source module/CDO를 먼저 대조한다.

`build_kouku_showtime_restore.py`의 해당 leaf 생성에 이 두 nested default만 복구하고,
별도 stage-only 옵션은 현재 저작본의 ID·TRS·원본 material·clock·provider 링크를 보존한다.
사용자의 meshY0/나머지Y1과1.25배는 원본 복구와 분리하고 임의 원복하지 않는다.
원본 낙하 curve가 실제 실행되면 새 속도나 smoke를 합성하지 않는다. 실제 Codec/Playback의
시간별 공 위치·크기와 이전 시각에 방출된 smoke의 위치를 비교하고 native WModel 정점의
전후 bounds와 cm→m·scale의 1회 적용을 기록한다. 후보 검증 뒤 최신 저장 SHA와 종료 상태를 확인한 CAS로 Authored에 적용한다. Client/UI 실행과 최종 화면 판정은 사용자가 한다.


## G10. 저주의식 왼손 trail과 원본 UV1 전달

사용자 요청은 저주의식의 흰 손 궤적과 무지개 별을 원본 재질로 재생하고 쿠크세이튼 왼손에 붙이는 것이다. 현재 세 요소 중 ribbon 두 개는 원본 PS `59a22eeec5a51f439595f929dddfe8bf`/distortion `e924ddbcfb7336408af5883ef3ddbf89`를 이미 사용한다. 원본 beam/trail VS는 입력 TEXCOORD0 네 성분을 그대로 넘기지만 현재 번역 adapter가 zw를 0으로 채운다. PS의 폭 방향 중앙 마스크가 z=0에서 항상 0이므로 CPU point 수만으로 표시 성공을 판정할 수 없다.

기존 generator에서 검증된 두 VS의 TEXCOORD0 전달을 `input.uv + input.uv1`로 연결한다. 기존 Trail carrier의 runtimeUV는 (거리/tilingDistance, 폭 0~1)이므로 해당 UV1은 (폭, 거리/tilingDistance)로 전달하여 원본 폭 마스크와 세로 타일 축을 충족한다. 기존 pixel 식/재질 scalar/texture/sampler와 다른 VF는 보존한다. 설치된 두 material program 2836/3007의 본문과 distortion 전달만 동일하게 갱신하고 shader를 정상 Product 증분 빌드한다.

원본 hand notify의 b_wp_1/startcontrol은 근거로 보존하고 사용자 지정 왼손은 원본에 존재하는 fx_l_hand_01 socket과 실제 설치 모델 bip001-l-hand에 연결한다. 원본 왼손 socket의 15cm X offset과 설치 bone frame Rx(-90)을 사용한다. 실제 CModel 27_01 clip과 왼손 본의 궤적을 수치 검증한다. 독립 저주의식 Pattern에 clip과 source notify 시각을 함께 등록한다. 새 C++ 파일이나 project 소스 등록은 없으며 새 resource가 생기는 경우 Catalog/Tree/Composition 및 Data None 등록을 함께 stage한다.

검증은 원본 VS/PS 입력과 두 셰이더 generated body 일치, 변경 JSON codec/actual CModel playback, 마지막 저장 SHA를 기준으로 한 설치, 해당 domain publish, Product shader build이다. 실제 화면은 사용자가 판정한다.


## G11. 기분나빠·쇼타임 폭탄·앵콜 블랙홀빔의 원본 연결

기분나빠 브레스는 공유 화염으로 교체하지 않고 action4219917/31_01/notify008의 원본15요소를 보존한다. Distribution=None만 저장된 정확 module instance의 누락된 nested CDO lookup10곳을 복구한다. 기존 분신 이동 Pattern들은 보존하고 새 독립 Pattern에 5167ms clip과 1989ms부터3000ms 브레스를 연결한다. 단독 EffectTool sourceModelPreview는 같은 clip의 sourceStart1989ms부터 시작한다.

앵콜 블랙홀빔은 action4219983의35_01/35_04 한 번 준비·발사, 원본5notify를 사용한다. 양눈 빔 notify를 실제 좌우 눈 본에 적용하고 slot별 group을 유지한다. 요청 이름은 빙고 | 앵콜세이튼 | 블랙홀빔이다. 새 effectAssetId를 Catalog/Tree/Composition과 Client Data None에 등록하고 실제 clip과 Effect를 가진 독립 Pattern을 추가한다. 기존 광역 source leaf나 전역 본 basis는 수정하지 않는다. emitter 자체 기간이 원본 notify보다 길어 spawning이 넘어가거나 원본 nested CDO lookup이 빠진 경우 신규 asset occurrence에만 근거가 있는 투영을 적용한다.

### G11-01. 쇼타임 해골 폭탄의 원본 심지 연결

대상은 기존 `world.object.kouku.bingo_bomb`가 사용하는 MN_RHCN_01 animated 모델과
기본 idle/Respawn 모션 두 개다. 실제 WModel의 D/N/S와 BossCatalog의 원본 native
program30/7 texture 입력을 검사하며, 현재 사용자의 scale2와 모든 인스턴스 TRS를 보존한다.
원본 NPC480712 -> SpawnAction4223107 -> Spark notify -> FX_01 socket 연결을 재사용한다.

기존 `build_kouku_showtime_restore.py`에 stage-only `--stage-bomb-fuse` 모드를 추가한다.
원본 SaprkLoop source leaf 3요소를 복사해 별도 `effect.kouku.gate3.showtime.bomb.fuse`를
만들고, 원본 notify의 2초 창 동안 심지 불꽃을 반복시키는 저작 정책을 명시한다. 원본 leaf,
material/source recipe의 분포/색/크기/velocity와 다른 리소스는 바꾸지 않는다.

FX_01은 b_body+[20,0,52.1496]cm다. 이 설치 FBX bone의 X/Z basis와 실제 PSK/스킨
정점의 대응을 확인해, World effectTrack의 bone=b_body, position=[.2,0,-.521496],
rotation=[-90,0,0]으로 좌표 변환을 한 번만 적용한다. 실제 CModel pose에서 심지 중심,
owner scale/회전과 연기/불꽃의 상하축을 검사한다. Source 본 복구나 모델 전체 회전 변경으로
기존 사용자의 폭탄 외형/애니메이션을 바꾸지 않는다.

generator는 현재 원본 bytes/hash와 baseline, 후보, 정확히 두 template의 국소 변경을 out에
기록한다. live Data/Resources/Catalog/Tree/Composition은 통합 담당자의 CAS 대상으로 남긴다.
검증은 원본·설치 mesh/재질 입력, 실제 CModel 및 Codec/Playback, JSON parse와 diff check다.
Client/UI 실행·캡처와 원본 전체 frame 시각 일치 판정은 하지 않는다.


## G12. 작은 오망성의 원본 광주·파편 동반 폭발 — 2026-09-16

현재 작은 오망성은 cast8+star_shot7만 가진15요소다. 이전 G07의 native2811/2812 additive 교정은 현재 설치 셰이더에도 존재하므로 같은 수정을 반복하지 않는다. 사용자 첨부 원작의 광주·파편에 대응하는 원본 down_lighting_Atk_02와 Atk_09_01 system을 실제 Action4219932의 활성 notify에서 가져온다. 전자는 현재 P34가 쓰는4219911에서도 활성이고, 후자는4219911에서는 비활성이나4219932에서는 활성이다. 두 action은 같은27_01 clip과 같은 발생 시간을 가진다. 원본 action flag는 수정하지 않고 작은 오망성 독립 library의 범위를 확장한다.

기존 build_kouku_ritual_hand_trail.py에 burst-only 후보 경로와 source impact 결합 함수를 추가하고, 기존 slam/mario 생성기도 같은 함수를 소비한다. 사용자 cast/shot15요소의 ID·TRS·시계·재질을 보존하고 원본1.993970초 광주11요소와2.051467초 폭발12요소만 추가한다. 회색바닥·precast·손 부착·ScreenPost는 이 변경에 합치지 않는다. 원본 notify scale2와 offset, native leaf 재질, nested CDO와 notify stop window를 대조한다.

Effect candidate와 P34의 전용 resource/occurrence duration 두 필드를 함께 준비한다. 현재3초창에서 사라지는 원본 입자 꼬리는 실제 Playback 종료 시각에 맞춰 보존하며 portal2998ms 시작, animation4667ms와 기존 패턴 시간을 바꾸지 않는다. 실행 중 편집이 있으므로 정본은 즉시 덮어쓰지 않고 사용자 저장·종료 후 source SHA를 비교해 설치한다. 변경 C++/셰이더/신규JSON은 없으므로 project/filter 등록은 필요 없다.

실제 Codec roundtrip·Product CPU/GPU resource stage·120Hz Playback의 발생, 유한값, 종료, 되감기와 기존15요소 보존을 검증한다. GPU resource stage는 화면 출력이나 visual fidelity 판정이 아니며 최종 화면은 사용자가 확인한다.

## G13. 큰 오망성과 독립된 마리오 진입 포탈 — 2026-09-16

첨부 두 이미지의 붉은 수직 기둥·주황 불꽃 링은 기존 center.portal의 바닥7요소와 구분한다. 실제 연결은 NPC480650의 Action4222001에서15초에 호출하는 Effect422200109, SkillBuff4219930 `KoukuSaton_Magic_Pillar`의5초 창과 ParticleSoundNew buff FX의 `fx_mn_rpct_07_v.Par_V_RPCT_light_line_01_LOC_INT`다. 이8요소는 붉은 cylinder, 불꽃ring, magic-circle2개와 sprite4개를 가진다.

신규 `effect.kouku.gate3.mario.entry.portal.full.restore`를 표시 이름 `진입 포탈`로 별도 등록한다. 독립 library의 시작은 Buff 활성 시점을0초로 두며 원래 전체 마리오 action의15초 시계를 변경하지 않는다. 기존 center.portal7과 큰/combined 오망성은 유지한다. source Required와 module/CDO, 원본 MIC/native shader 입력, buff TRS/배율 및 실제 모델 preScale을 대조하고 emitter의 발생 창과 particle tail을 따로 계산한다.

현재8요소의 재질 중 누락된3개 native shader는 기존 생성기·registry·carrier 경로에 연결하며 별도 renderer를 만들지 않는다. 새 Effect JSON의 Catalog/Tree 및 Client의96.DataFiles None/project filter를 최소 추가한다. 독립 검증용 shader는 out에서 컴파일하고 실행 중 Client의CSO/EXE는 교체하지 않는다. Composition presentation resource 등록은 최신 편집을 보존하는 후보로 준비하고 현재 사용자 편집 중 정본에 외부 쓰기를 하지 않는다.

원본 연결·8요소/재질/resource closure, 실제 Codec/CPU Playback·GPU resource 준비와 필요한 shader/C++ 개별 컴파일, JSON/XML/diff 검사를 수행한다. 원본 입력 복구·설치·제품 빌드 및 사용자 화면 판정은 RESULT에서 구분한다.

## G14. 기분나빠 브레스의 전체 재생 길이4279ms — 2026-09-16

사용자가 저장한 `effect.kouku.gate3.clone.breath`의 현행15요소를 기준으로 변경한다. 표시 timeline은3초지만 지속rate13개가 대부분1초 후 방출을 멈춰, 뒤에 같은 문서를 duplicate해 붙이면 첫burst가 반복된다. 원래2개 burst는 한 번만 유지하고 지속13개의 emissionDuration을 `4.279 - startDelay - emitterDelay - afterImage - 원래 최대particle수명`으로 늘린다. 실제 lifeScale을 포함하며 Required.emitterduration도 같은 방출 창으로 맞춘다. 입자 고유수명·속도·색·곡선은 유지하므로 자연스러운 꼬리를 포함한 전체 길이가4279ms다.

detail timing의 수명과 sourceModelPreview playMs는4.279초/4279ms로 맞춘다. 기존 sourceStart1989ms, clip31_01, playRate와 HOLD_LAST_POSE를 보존하며 원래clip이 끝난 뒤에는 기존 마지막 자세 유지 정책을 쓴다. 사용자 bloom·그룹·TRS·재질과 입자수/loop1을 변경하지 않는다. 생성기 `build_kouku_clone_breath_group.py`의 현재 저장본 변환 모드로 같은 정책을 재현하고 후보 hash/CAS로 Effect 정본만 반영한다. 실행 중 Composition의 resource/occurrence 시간은 별도 후보로 보관한다.

실제 Codec/Playback에서1초 이후의 연속 방출, 마지막 입자 종료 시점·전체4279ms, 되감기 동일성과 변경 필드 외 보존을 검증한다. 최신 Effect 저장본이 바뀌면 오래된 후보를 덮어쓰지 않는다. 현재 요청의 지속시간은 프로젝트 저작값이며 원본 action notify 길이로 표시하지 않는다.


## G15. 반복 패턴의 왼손 trail과 occurrence 수명 — 2026-09-16

현재 P33의 세 번째 Effect는 원본 저주의식27_01 미리보기와3.174864초의 loop0 emission을 가진다. 실제 마리오2페이즈는29_02를16회 배치하며 마지막 animation end는27994ms, Effect start는3035ms다. 원본 Effect는 P61에서도 사용하므로 공용 asset의 clip·수명을 덮어쓰지 않는다. 선택한 Pattern/occurrence를 Effect Tool의 임시 미리보기 문맥으로 전달하고 실제 animation row와 시작 시각을 기존 model sampler에 연결한다.

Composition occurrence에 loopEffectToDuration을 선택적으로 저장한다. 이 값은 V1 Effect의 source loop0 emitter만 occurrence 창까지 원래 속도로 유지하고 fitEffectToDuration의 시간 늘이기와 동시 적용하지 않는다. 기존 EffectPlayback/EffectObject/PresentationService에 유한 sourceLoopEndSeconds를 연결하며 원본 pointLife, emitterPeriod, finite loop와 다른 occurrence는 보존한다. 편집 UI에서 마지막 animation end까지 수명을 맞출 수 있게 하고 P33의 해당 occurrence는24959ms 후보로 준비한다. 사용자 편집 중 Composition 정본은 쓰지 않는다.

현재 native2836/3007의 원본 pixel body와 UV1 전달을 먼저 대조한다. 실제29_02 CModel·왼손 본·socket15cm·원본 ribbon local position을 사용하여 손과 geometry의 거리를 확인하고, shader 또는 위치 계산은 원인이 확인된 범위만 교정한다. 첨부 이미지의 떨어진 선은 관찰값이며 shader 원인이나 visual PASS의 근거로 대신하지 않는다.

검증은 기존 Codec/Playback의 loop0 late seek·유한 종료·되감기·finite loop 보존, 실제 본 sampling, projector roundtrip/거부 계약과 변경 C++ 격리 컴파일이다. 새 C++ 파일은 없으며 project/filter 등록도 추가하지 않는다. 실행 중 Client/Server와 공유 산출물을 보존하고 정상 Product build·publish와 사용자의 최종 화면 판정은 별도 상태로 기록한다.


### G15-01. 흰 Ribbon의 손끝 부착과 곡선

실제29_02 CModel 비교에서 white3007의 source StartLocation+1m가 바닥쪽 궤적을 만들었다. 사용자가 지정한 왼손 socket adaptation에서는 이 module lookup과 editable initial position만0으로 맞추며 원본 leaf/dump는 보존한다. Detail.Transform -1m 상쇄는 SPU 측정 원점을 바꿔 입자발생과 손끝 gap이 악화되므로 사용하지 않는다. 원본27_01 preview와 source timing은 이 후보에서 유지한다.

원본 bClipSourceSegement=true는 마지막 입자와 현재 손 사이의 연결을 생략한다. 현행0.75m SPU는1초 loop경계의 누적초기화 때문에 zerooffset뒤에도 최대1.509m gap을 보였다. 이번 사용자 손끝 정책은 white3007에만 명시false를 저작한다. 기존 native ribbon playback이 false인경우 방출활성창에 현재 source origin을 geometry head로 연결하고 particle/RNG/SPU 자체는 바꾸지 않는다. 기본/원본true는 그대로이며 끝난뒤 head제거·point cap·되감기를 검증한다.

Renderer는 Kouku native CascadeRibbon과 실제 source tangent flags에만 chord-limited Hermite를 연결한다. 최대25subdivision/512controlpoint, 정확한 끝점,0길이 및 긴span fallback, 기존 width/color/dynamic/UV를 보존한다. 이는 원본 CPU tangent 수식의 exact 회수가 아니라 source 설정을 소비하는 프로젝트 보간이다. 원본 pixel body는 변경하지 않는다.
