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
