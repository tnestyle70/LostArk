# LostArk merge 회귀 방지 정본

### 발탄 복원본은 실제 source clip과 함께 재생한다

- `.restore` suffix는 player skill identity가 아니다. 발탄 Full Restore는 원본 action/stage의
  정확한 clip occurrence와 연결하고 일반 Play/Restart/Solo/Group에서 같은 clock을 유지한다.
- action 하나에 여러 공격이 있으므로 SourceActionIds만으로 패턴 목록에 모두 넣지 않는다.
  ValtanFullRestoreAnimations 정본의 clip과 현재 ClipOccurrences를 함께 비교한다.
- 원본 notify delay/root basis 회전은 문서에 포함된다. preview cue에 같은 값을 더하지 않는다.
  메모리 내 editor source preview를 Product 연결 또는 제품 승격으로 처리하지 않는다.
- 맵 prototypes 진행 수치가 멈추면 구조화된 실패 로그의 모델부터 CModel로 재현한다.
  catalog/shader가 허용하는 subspecular-only·grass flicker를 모델 guard가 다르게 거부하지
  않는지 확인한다. 한 모델 수정 후 전체 admission에서 뒤에 가려진 실패도 확인한다.
- embedded material PNG가 없으면 실제 원본 DDS와 대응을 확인해 그 필드만 교정한다.
  모델·재질 입력 오류를 맵 scope 축소나 배치 숨김으로 우회하지 않는다.
- 근거: [맵·발탄 G24-V](09-15/2026-09-15_MAP_AND_VALTAN_FULL_RESTORATION_RESULT.md).

### full restore의 누락과 사용자 삭제를 이력으로 구분한다

- 원본 활성 발생기가 legacy에 있고 full에 없으면 최초 full 생성 커밋과 generator의 제외
  조건을 확인한다. native 재질 미연결로 빠진 항목을 사용자 삭제로 오인해 영구 보존하지 않는다.
- cylinder나 sphere 메시도 PS가 화면 좌표·종횡비·SceneDepth를 읽을 수 있다. 원본 PS/VS,
  uniform binding과 실제 읽는 varying을 확인하고 viewport 및 깊이 단위를 기존 carrier에
  연결한다. 사용하지 않는 UV/vertex color나 legacy의 미해결 texture를 임의 대체하지 않는다.
- 등록 뒤 원본 PS 비교와 함께 실제 Product Catalog Stage 및 발생기 재생까지 확인한다.
  [워로드 Alt+V 결과 G17](09-09/2026-09-09_WARLORD_ASVF_FULL_RESTORE_IMPLEMENTATION_RESULT.md)을 따른다.

### native 배경 메시의 조명 입력도 carrier에서 전달한다

- 원본 material PS가 존재하고 mesh/resource stage가 성공해도 ambient/sky 입력을 0으로 초기화한 채
  넘기면 diffuse 표면은 검게 출력될 수 있다. 실제 native PS 소비 항과 Mesh carrier의 입력을 함께 확인한다.
- Lance ALT V static Mesh782~800은 committed scene ambient를 명시적으로 전달한다. 원작 환경조명의
  값 복원과 현재 장면 입력 연결을 구분하며, 없는 skylight owner를 임의 상수로 채우지 않는다.
- V/ALT V dragon 23행은 사용자 요청으로 원본 `localSpace=true`를 선택 복구했다. 다른 particle의
  world-space 요청과 손본·socket·사용자 Transform은 유지한다.
- 근거: [창술사 G13/G14 결과](09-11/2026-09-11_TIGER_HORSE_ANIMATION_AND_LANCEMASTER_ALTV_RESULT.md#g13-v-alt-v-dragon의-local-space-선택-복구--2026-09-15).

### 원본 색 패스와 별도 distortion 패스를 따로 닫는다

- `bUsesDistortion` 재질은 기본 색 PS 일치만으로 복구되지 않는다. 선택 shader map의 별도
  distortion PS, 실제 serialized uniform binding과 VF 입력을 확인한다. binding 배열의
  겹치는 후보를 추정 선택하지 않고 같은 shader class의 검증된 직렬화 경계와 대조한다.
- 원본 distortion discard는 해당 왜곡 기여만 제거한다. 기본 색까지 clip하지 않는다.
  원본 양/음 누적 RGBA와 현재 signed XY MRT, scene-depth 단위 변환은 명시적으로 연결한다.
- 동일 원본 DXBC와 후보 PS를 강도·particle dynamic·시선·가림 조건으로 비교하고 실제
  0이 아닌 기여도 포함한다. 수치 일치를 사용자 화면 복원 완료로 대신 기록하지 않는다.
- 근거: [차원술사 V distortion 결과](09-15/2026-09-15_DIMENSIONMASTER_T_UNLIT_V_RECTANGLE_IMPLEMENTATION_RESULT.md#g06-유리-재질의-별도-원본-distortion-pass-연결).

### localSpace의 이동 정책과 본·카메라 부착은 구분한다

- 캐릭터 이펙트의 `localSpace`를 일괄 해제하면 손에 붙어야 할 무기와 `camera_view` 입자도
  생성 당시 world에 남는다. 실제 skillbinding/cue/asset → Catalog DIRECT/reconstructed →
  원본 Required `buselocalspace` → Playback 소비자를 확인하고 필요한 모델·카메라 요소만 복구한다.
- 앵커가 갱신된다는 사실만으로 기존 입자가 따라온다고 판단하지 않는다. 같은 살아 있는 입자의
  위치를 앵커 이동 전후로 비교한다. 사용자 요청으로 world-space를 택한 다른 이펙트는 유지한다.
- camera sprite의 local axis 조건을 풀기 전에 socket·UE camera basis를 대조한다. 잘못된
  회전은 법선을 camera up으로 만들어 옆면 소실을 일으킬 수 있다.
- 특정 소환 모델의 조명 제외는 exact cue의 기존 mask/depth를 유지한 emissive 출력 정책으로
  제한한다. 모든 캐릭터·메시에 unlit을 전파하지 않는다.
- 근거: [차원술사 T/V 결과](09-15/2026-09-15_DIMENSIONMASTER_T_UNLIT_V_RECTANGLE_IMPLEMENTATION_RESULT.md).

### Sequence의 일반 Play와 전투 재생은 종료 의도를 따로 보존한다

- `enterCombatOnFinish`는 Complete Play에서 고를 입장 Sequence의 표시다. 일반 Play를
  자동으로 Complete로 바꾸지 않는다. Complete에서 일반 Play로 전환할 때는 기존 전투
  연결을 새 미리보기 시작 전에 취소한다. 뒤늦은 취소가 새 미리보기를 중지하면 안 된다.
- Sequence 이동은 불변 Pattern의 시계와 occurrence/run ID로 한 번만 제출하고, Pause와
  scrub으로 플레이어를 이동시키지 않는다. 공유 응답 큐는 request ID의 원래 소비자에게
  분배하며 다른 도구의 응답을 pop한 뒤 버리지 않는다.
- 도착 승인 전 완료 이벤트는 보존하고, 실패·시간 초과 때 전투로 넘어가지 않는다.
  이미 승인된 도착 위치를 후속 Gate의 기본 teleport로 덮어쓰지 않는다.
- 근거: [Sequence 결과 G26](09-14/2026-09-14_KOUKU_SEQUENCE_PLAYBACK_EDITOR_IMPLEMENTATION_RESULT.md).

### 원본 정적 메시의 접선 부호는 두 좌표계 변환을 함께 계산한다

- UE packed TangentZ.W의 부호를 glTF tangent.w에 그대로 복사하지 않는다.
  UE→glTF `(x,z,y)`는 반사이므로 glTF W는 원본 부호의 음수다. 공통 cooker의
  glTF→runtime `(x,y,-z)`에서 다시 반사되어 최종 runtime W는 원본 부호와 같아진다.
- 원본 T/N과 `B=cross(N,T)*W`, 비퇴화 UV 미분으로 얻은 B를 독립 대조한다.
  최종 WModel의 T/B/N과 position/UV/COLOR/index를 검사하고 정상 모델까지 일괄 반전하지 않는다.
- 상세 교정·설치 범위는 [맵·발탄 결과](09-15/2026-09-15_MAP_AND_VALTAN_FULL_RESTORATION_RESULT.md)에 둔다.

### 유령 재질과 기본 부착 오라는 원본 blend·수명 계약을 소비한다

- Valtan source program84의 master는 BLEND_Translucent다. opaque/deferred의 ordered coverage로
  대신하면 픽셀 소실이 발생한다. 기존 native character forward pass를 사용하며 source의
  post-render depth와 approximate sort까지 재현했는지는 별도 기록한다.
- LookInfo 기본 particle은 action notify와 다른 입력이다. 실제 본 이름·local TRS와 owner
  생존 수명을 연결하고 normal/ghost 교체·죽음·숨김·release에서 handle을 정리한다.
- source EmitterLoops의 생략/0을 Python `value or 1`로 유한1회로 바꾸지 않는다.
  지속 오라는 기존 Effect_Playback의 owner-sustained 실행으로 처리하며 매프레임 전체 Seek나
  duration마다 강제 reset으로 입자 이력을 버리지 않는다. 기존 유한 효과의 종료 정책은 보존한다.

### 원본 카메라·후처리 값은 축·상속·실제 소비자를 함께 확인한다

- 원본 수평 FOV를 DirectX 수직 FOV에 그대로 넣지 않는다. 16:9 변환과 현재 viewport aspect를
  구분한다. 카메라 pose까지 바뀌면 캐릭터와 지면의 투영 비율이 달라져 전체 Effect scale로 보정하지 않는다.
- WorldInfo/PostProcessVolume의 저장값은 클래스 기본값과 override flag까지 합쳐야 한다.
  LUT 이름이 있어도 override=false이면 켜지 않는다. Character Select는 SL00만 보지 않고
  LV_LOBBY_PS의 실제 CharacterCloseupScene chain을 확인한다.
- Lightmass EnvironmentColor는 bake 입력이다. RNM 위에 더하는 runtime ambient와 동일하다고
  단정하지 않는다. SOURCE_CHARACTER 광원은 marker5라는 이유로 baked map monster 표면을 재조명하면
  안 된다. material baked binding과 실제 픽셀 RNM flag를 함께 검사한다.
- 원본 tone/LUT packing과 Hable exposure/임의 gamma는 다른 계산이다. 원본 chain 이름에
  epic이 들어 있어도 실제 enum과 CPU 분기를 확인한다. LUTBlend의 native A8R8G8B8과
  float 중간 출력, CPU pow와 GPU 명령 정밀도를 구분한다. disassembly에0으로 표시된
  상수도 DXBC bit를 확인한다. 현재 pow floor의0x322bcc77은1e-8이다.
  native shader 수치 동치와 사용자 화면 판정을 구분한다. 미지원 태양광 제외영역·안개/발광
  합성과 입력 출처는 대응 09-14/09-15 렌더링 RESULT에 둔다.

### Cooked distribution의 range header를 방향 XYZ로 읽지 않는다

- `lookupTable`의 앞 2개 값은 값 범위 header다. 실제 vector payload와 `componentCount`,
  `lookupTableChunkSize`, operation을 `CEffectDistribution`과 같은 규칙으로 읽는다.
  배열의 첫 음수만 보고 X 방향이라고 판단하면 이미 +Z인 불뿜기에 90도를 중복 적용한다.
- 세이튼 `Fire_01/02`의 실제 주 속도는 UE -Y이며 기존 `(x,z,-y)` 변환 뒤 Client +Z다.
  shared neutral frame은 yaw0을 사용한다. 원본 본 부착의 basis, occurrence 회전,
  사용자가 화면에서 지칭한 반시계 방향은 서로 다른 경계이므로 같은 degree로 대체하지 않는다.
- 크기 비교에서는 world-space acceleration을 단순 배율로 곱한 기대값에 포함하지 않는다.
  실제 particle position/velocity와 원본 local/world 정책을 사용해 방향·크기를 따로 검사한다.

### 화염이 조각만 남으면 native 입력과 합성을 분리한다

- 링 mesh와 주변 sprite가 함께 있는 효과는 주변 입자의 draw만으로 본체 출력을 판정하지 않는다.
  masked LocalVF의 engine row0은 particle RGBA일 수 있다. translucent용 opacity X=1을
  재사용하면 W=0으로 원본 mask가 전부 discard된다. 원본 PS/VS, serialized float4 wire,
  material-owned row와 기존 창술사 masked 교정을 대조하고 실제 particle color/alpha를 연결한다.
  mask 임계값 삭제나 상수 alpha1은 원본 fade를 보존하는 수정이 아니다.
- 같은 결함은 도화가 Alt+V 나비의 native536에도 있었다. dispatcher에서 opaque coverage를
  바꿔도 함수 내부 discard는 복구되지 않는다. material-owned row1..7과 unowned row0,
  정확한 PS/VS를 확인한 분기로 생성기와 설치 함수를 함께 고친다. [도화가 결과 G12](09-09/2026-09-09_ARTIST_CORE_FULL_RESTORE_IMPLEMENTATION_RESULT.md)를 따른다.
- additive+distortion의 early return은 원본 RGB에 포함된 opacity를 다시 곱하지 않아야 한다.
  하나를 고친 뒤 같은 sourceBlend·PS 출력·dispatch 계약의 설치 cohort 전체를 대조한다.
  이름에 fire가 있는지로 수정 대상을 결정하거나 translucent alpha까지1로 바꾸지 않는다.
- world-position 재질의 TEXCOORD5는 clip position과 구분한다. 원본이 emitter inverse를
  소비하면 기존 source cm WorldToLocal uniform까지 실제 renderer에서 공급한다.
  shader 수식만 고치고 CPU uniform binding을 빠뜨리지 않는다.
- 구현·수치·제품 빌드와 사용자 화면 판정은 [화염 재질 결과](09-14/2026-09-14_FIRE_MATERIAL_CLIPPING_RESULT.md)에 분리한다.

### Logic 표시명과 실제 실행 종류·발생 수명을 구분한다

랜덤 탐색·스폰이라는 이름이나 Timeline box만으로 Server 동작이 연결됐다고 판단하지 않는다.
Composition의 typed kind와 실제 배치 occurrence, projector의 mechanic trigger/logic window,
Server 소비자를 끝까지 확인한다. 이름 전용 TRIGGER/Summon은 실행을 만들지 않는다.
합본 Effect를 Server 객체로 옮길 때는 Timeline의 기존 직접 재생과 중복되지 않게 하고,
예고보다 짧은 occurrence가 폭발을 끊지 않도록 실제 Effect 지연·tail과 객체 수명을 비교한다.
무피해 Effect의 랜덤 위치를 구하려고 가짜 damage shape를 추가하지 않는다. 공용 nav resolver의
명시 간격을 사용하고 기존 발탄의 damage 직경 기반 기본값은 보존한다.
현재 감사·검증과 미적용 저장안은 [알비온 결과](09-14/2026-09-14_KOUKU_ALBION_RANDOM_VOLLEY_RESULT.md)에 둔다.

### 원본 Sprite의 형상 생성과 최종 blend를 함께 검사한다

- 원본 조준점은 완성형 이미지 한 장 대신 화살표·띠 texture와 radial UV shader로 구성될 수 있다. Elements의 `resources=[]`만으로 누락을 판정하지 말고 실제 `material.sourceProfile.textures`, source emitter와 native program을 대조한다.
- CPU 입자·양수 alpha·texture staging 성공은 최종 RGB 기여의 증거가 아니다. native additive PS가 opacity를 RGB에 이미 곱하고 alpha0을 내면 공통 SrcAlpha/One 합성에서 다시0이 된다. distortion 동반 dispatch의 early return도 일반 native의 source blend에 따른 coverage adapter를 유지해야 한다. translucent까지 alpha1을 강제하지 않는다.
- 같은 PS·DDS·parameter·시각에서 기존 합성과 source blend 대응을 비교하고, 확정된 program만 설치 교정한다. 원본 fade의 RGB 반영과 별도 distortion MRT 불변을 수치로 확인한다. Sprite2484뿐 아니라 Mesh2811/Sprite2812의 distortion early return에도 같은 결함이 있었으므로 실제 carrier별 출력까지 검사한다. [쇼타임 조준점 결과](09-13/2026-09-13_KOUKU_SHOWTIME_TARGET_BLEND_RESULT.md)와 [작은 오망성 G09](09-13/2026-09-13_KOUKU_PATTERN_RADIAL_MOTION_RESULT.md#g09-작은-오망성-폭발이-투명한-왜곡만-남는-문제)를 따른다.

### 원본 nested CDO와 독립 패턴의 시계를 구분한다

- RawDistribution 구조체의 instance가 `Distribution=0`만 직렬화했다고 CDO의 lookup table까지 지워진 것은 아니다. 원본 class default의 타입·Op·ChunkSize·lookup을 실제 instance delta와 합쳐 확인한다. StartSize fallback이나 LocationDirect의0배가 원본 낙하·바운스를 없앨 수 있다. null alpha/rate처럼 runtime이 이미 identity를 적용하는 경로는 실제 소비자를 먼저 읽는다.
- disabled 모듈은 raw 증거에 보존하고 독립 runtime projection에는 활성 모듈만 싣는다. 목록 개수 성공으로 codec의 family/cardinality 검사를 대신하지 않는다.
- 원본 Projectile의 자식 callback·수명·거리 값이 있어도 실제 종료 우선순위·targeting까지 해독한 것은 아니다. 편집용 생성 시계·방향은 이름과 RESULT에서 구분한다. 원본 action의 disabled visual system을 사용자 요청으로 독립 조합해도 enabled cue 복원으로 기록하지 않는다.
- LocalDecal의 크기 보간은 기존 Detail Life와 SourceRecipe의 입자 수명을 분리해 검사한다. Mesh Particle 전용 transformMotionDuration이나 Ring Fill을 강제로 넣지 않는다. XZ 직경·고정 중심·projector 깊이와 fade 끝을 실제 CPU 행으로 확인한다.
- 쿠크 원형/도넛 native3600/3601은 고정 경계와 채움을 한 draw에서 계산한다. 발탄의 물리3요소와 같다고 native를 중복하거나 projector 전체 scale을 키우면 경계 합성·위치가 달라진다. `inner`만 원형0 또는 도넛의 고정 내경 비율에서1까지 보간하고 원본 lifetime·fade·thickness·drawscale을 보존한다. sourceTimeOrigin과 start delay를 포함한 native packet의 실제 시각별 값, 끝값 도달 시 양수 alpha, 실제 패턴이 사용하는 warning/impact까지 확인한다.

실제 적용과 검증은 [도넛·분열·손 트레일 결과](09-13/2026-09-13_KOUKU_PATTERN_RADIAL_MOTION_RESULT.md)를 따른다.

### 이동하는 원본 이펙트는 그룹 이름·반복·거리 방출을 함께 확인한다

- UE Matinee의 FName 연결은 대소문자를 구분하지 않는다. `Pc01tr`/`pc01tr` 같은 표시 차이로
  actor Move를 놓친 뒤 빈 key를 정상 정지 상태로 저장하지 않는다. 정확한 occurrence binding을 검증한다.
- SpawnPerUnit·world-space ribbon은 실제 emitter 위치 이력이 필요하다. core 몇 개가 출력되거나
  finite 검사에 통과했다는 사실만으로 이동·잔광 복원 완료를 판단하지 않는다.
- Required의 임시 emitterLoopCount 1을 원본 값으로 유지하지 않는다. 완전한 CDO 체인에서
  loop 0을 확인한 경우 원본 Toggle 종료와 함께 복구하고 KillOnDeactivate/Completed를 보존한다.
- Object 불뿜기의 전체 Lifetime만 늘려도 Required의 1회 배출은 다시 살아나지 않는다. 준비·유지·종료
  notify와 실제 emission window를 나누고 원본 CDO loop, 입자 잔여 수명을 함께 측정한다. 준비 속도를
  보존하는 연장은 animation `sourceStartMs`로 원본 구간을 잇고 유지 구간만 조정한다. 보이는 모델과
  Effect 소켓 이력은 같은 source 시각이어야 한다. [인형 15초 결과 G12](09-13/2026-09-13_KOUKU_DOLL_VARIANTS_OBJECT_SEQUENCE_EFFECT_IMPLEMENTATION_RESULT.md#g12-09-14-object-인형의-실제-불뿜기-15초).
- SourceTransformTrack가 움직이면 VelocityInheritParent도 document root가 아닌 해당 emitter의
  실제 world 속도를 사용한다. birth simulation basis와 source scale을 한 번씩 적용하고, 기존
  source track 없는 root/local/bone 경로를 함께 대조한다.
- 원본 CONSTANT 위치 도약은 Director의 즉시 camera cut과 같은 시각인지 먼저 조사한다.
  독립 Play All의 카메라 없는 재생 차이를 임의 평활·clamp로 숨기지 않는다. 상세 근거와 화면 미확인
  범위는 [금빛 이동 축포 결과 G07](09-11/2026-09-11_KOUKU_PLAYER_ANCHOR_RAINBOW_FIREWORKS_IMPLEMENTATION_RESULT.md)에 둔다.

### Effect Play All의 scene player 오류는 생성 전 등록 경계부터 확인한다

- `Enter an arena with a scene player before Play All`은 Effect factory·particle simulation·shader
  이전의 scene player 조회 실패다. 카메라와 이동이 정상이어도 `Resolve_SceneCharacter` 등록은 별개다.
- 실제 local player의 생성·class replacement commit을 소유한 `CClientReplication`이 scene target도
  연결한다. remote actor나 실패한 교체로 target을 덮지 않고 despawn/reset/destructor는 자기 캐릭터만 해제한다.
  destructor가 이미 제거됐을 수 있는 Layer를 다시 조작하지 않게 한다.
- 오류를 숨기려고 임의 world origin, preview 캐릭터, 첫 Layer 오브젝트를 플레이어로 대신 선택하지 않는다.
  Effect의 root attachment·shader·월드 좌표를 바꾸기 전에 실제 호출자가 어느 단계에서 거절됐는지 구분한다.
- 원본 독립 festival과 시퀀스용 authored festival은 시작 시각이 다르다. 전자는 0초, 후자는 현재
  시퀀스의 33.40897초부터 발생한다. 기존 MAP 배치와 원본 발생 시각을 Play All 편의를 위해 덮지 않는다.
- 다색 방사형 fireworks와 festival을 이름만으로 같은 문서로 취급하지 않는다. 원본 ParticleSystem과
  Matinee·주변 소품·현재 occurrence를 대조한다. 원본 MAP 좌표를 다른 카메라/무대의 시퀀스에 옮길 때는
  source actor 대응이나 공통 변환을 먼저 확인하며, element 내부 회전·크기를 root에 중복 적용하지 않는다.
  실제 검사와 사용자 화면 판정은 [플레이어 앵커·폭죽 결과](09-11/2026-09-11_KOUKU_PLAYER_ANCHOR_RAINBOW_FIREWORKS_IMPLEMENTATION_RESULT.md)의 G06에서 구분한다.

### 클릭 이동 예측의 위치·높이·회전·카메라를 각각 검증한다

- Client가 waypoint 직선을 예측하면 Server 일반 MOVE도 위치를 같은 목표 방향으로 진행해야 한다.
  몸의 제한 회전을 이동 벡터에 다시 적용하면 반대 클릭에서 곡선 이동과 ACK 되감김이 생긴다.
- 같은 navgrid 파일만으로 경로 일치를 보장하지 않는다. 기존 publisher의 맵별 navpolicy도 제품
  Loader와 Character 경로 요청까지 연결한다. 기본 step을 일괄 상향하거나 월드 상수를 복제하지 않는다.
- NavGrid 선분 검사는 실제 셀 경계 통과 순서를 구분한다. 떨어진 두 경계 통과를 한 대각선으로
  합치면 지나지 않는 장애물 때문에 우회한다. 정확한 모서리·막힌 내부 경계의 차단은 보존한다.
- 경로를 단축한 먼 waypoint의 Y를 미리 보간하지 않는다. XZ로 전진한 현재 발밑 지면을 읽는다.
- ACK 오차를 고정 80ms에 줄이면 오차가 커질수록 표시 보정 속도가 커진다. 일반 연속 오차에는
  속도에 따른 보정 시간을 적용하고 실제 teleport·강제 상태의 권위 전환은 따로 처리한다.
- 최단 yaw 보간의 소유자는 Character 한 곳이다. ACK마다 helper도 yaw를 보간하면 위치 오차가
  회전까지 늦춘다. 실제 Character 소비자와 반복 ACK로 확인하고 helper에 완성 pose만 넣어 검증하지 않는다.
- camera profile의 followResponse 0은 즉시 추적이다. 입력 반응과 카메라 감쇠를 분리하고 SPACE/스킬
  handoff, cinematic override와 복귀도 확인한다. CPU 검사 성공을 사용자 조작감 판정으로 기록하지 않는다.
- 실제 변경과 검증 범위는 [클릭 이동 결과](09-11/2026-09-11_CHARACTER_ACTION_COMPOSITION_AND_RESPONSIVENESS_RESULT.md)를 따른다.

## 0. 모든 세션의 사용자 전용 화면 검증 경계

- 세션 시작 시 `AGENTS.md`, `CLAUDE.md`, 이 문서, 있으면 `gotchas.local.md`,
  `.md/TEAM/README.md`, 대응 PLAN/RESULT를 먼저 읽는다.
- Artist F, Effect Tool, Character Select와 Client 시각 결과는 사용자만 직접 조작하고 최종 visual fidelity를 판정한다.
- 에이전트는 Client나 UI를 자율적으로 실행·조작하지 않고 화면 캡처·스크린샷 생성을 하지 않으며,
  visual fidelity를 대신 판정하지 않는다.
- 사용자가 대화에 첨부한 스크린샷이나 이미지 분석을 요청하면 에이전트는 반드시 열람·분석해
  관찰된 결함과 가능한 occurrence 진단을 보고한다.
- 에이전트는 빌드와 구조화된 로그·수치 진단, 실행 준비까지만 수행하고 사용자가 직접 누를 경로를
  보고한 뒤 멈춘다. 사용자의 서면 판정 전에는 first pixel, eye smoke, visual PASS를 기록하지 않는다.
- 일반적인 완성·복원 요청과 기존 캡처 파일의 존재는 Client/UI 자율 실행·조작이나 화면 캡처를 허가하지 않는다.

이 문서는 merge, pull, rebase와 충돌 해결에서 이미 닫힌 다른 작성자의 계약을 되살리거나
지우는 회귀를 막는 공용 체크리스트다. 날짜별 실패 로그는 대응 RESULT에 기록한다.

렌더링·캐릭터/무기 재질·이펙트 복원의 원본 입력, 기본값 상속, 좌표·재생 시계와
Resources 경계는 [렌더링이펙트복원V2.md](렌더링이펙트복원V2.md)를 함께 읽는다.
해당 분야의 재사용 원리와 실제 연결 범위는 그 문서에서 갱신하고 여기에는 복제하지 않는다.

### 맵 그림자 최적화의 보존 조건

- camera frustum을 shadow caster에 적용하지 않는다. 실제 light view/projection을 사용하고
  frame provider 뒤에 instance를 준비한다. light 변경 없이 실패한 upload도 다음 호출에서 재시도한다.
- 단순 shadow pass와 opaque null-PS 선택은 surface family뿐 아니라 source-material 활성 설정도
  함께 확인한다. 설정이 꺼진 경우 기존 legacy diffuse alpha를 보존한다.
- 장비 pose cache는 source pointer만으로 판정하지 않는다. owner 수명과 source/destination
  revision을 함께 검사해 동일주소 재할당·같은 프레임 포즈 변경을 반영한다.
- `Render.Shadow` GPU elapsed를 순수 GPU 실행시간으로 단정하거나 fixture 개선율을 사용자 FPS로
  환산하지 않는다. 현재 연결과 검증 경계는 [렌더링 복원 가이드](렌더링이펙트복원V2.md)를 따른다.

### 라이트 Enabled와 새 광원이 무시되면 최종 관문 문서를 확인한다

- 색·밝기는 반영되는데 enabled 해제와 신규 광원이 무시되면 shader 캐시보다 먼저
  저작 문서 → 관문/Sequence override → 실제 제출 문서의 enabled를 대조한다.
- 3관문을 특정 청색 lightId 하나만 허용하는 방식으로 만들지 않는다. 기존 배경 제외와
  사용자가 추가한 광원의 enabled를 분리하고, 새 광원·삭제·off/on을 같은 변경 비교에 포함한다.
- 조명 RT clear, transient 목록 재수집, shader 입력 재설정 최적화와 구운 lightmap을 구분한다.
  관문 문서 오류를 고치기 위해 전체 shader 캐시나 광원 budget을 제거하지 않는다.

### 조명·애니메이션 성능 변경의 merge 경계

- Deferred shader는 Engine 정본과 Client 사본을 함께 유지한다. instance record stride·최대 개수·pass index를 한쪽만 복원하지 않는다. 조명 정렬과 shader 내부 합산으로 기존 FP16 순서를 바꾸지 않는다.
- CChannel key는 공유되고 cursor는 CAnimation clone의 상태다. Effect model cue의 pose 재사용을 일반 캐릭터 전체로 넓히지 않는다. 객체별 uniform cache도 공유 CShader Effect의 다른 caller 변경을 놓친다.
- 구체적인 소유·소비 경계는 [렌더링 복원 가이드](렌더링이펙트복원V2.md)의 조명·Alt+V 항목, 수치 예외와 실제 FPS 확인은 [성능 결과](09-12/2026-09-12_MAP_CHARACTER_RENDER_PERFORMANCE_RESULT.md)를 따른다.

## 1. 동기화 전 상태 고정

다음 증거를 먼저 남긴다.

```text
git status --short
git diff --name-only --diff-filter=U
git branch --show-current
git rev-parse HEAD
git fetch --prune
git rev-list --left-right --count HEAD...origin/main
```

- dirty worktree를 정리한다는 이유로 다른 작성자의 변경을 reset, checkout, clean하지 않는다.
- 동기화가 필요하면 추적·미추적 파일을 포함한 이름 있는 safety stash를 만들고, 원격 반영 후
  같은 stash를 복원한다. stash 적용 충돌도 아래 파일 역할 기준으로 다시 해결한다.
- `ours` 또는 `theirs`를 파일 전체에 일괄 적용하지 않는다. 실제 소유자, 호출자, 데이터 정본,
  실패 소비자와 대응 PLAN/RESULT를 읽고 계약별로 합친다.

## 2. 충돌 해결 불변식

### DimensionMaster 이름과 런타임 계약

- 활성 코드·공유 enum·catalog·Loader·HUD·Server profile·spawn·Animation Tool의 class 이름은
  `DimensionMaster`/`DIMENSIONMASTER` 계약을 유지한다.
- 이전 `Dimensionist` 이름은 명시적으로 보존한 역사 문서나 외부 원본 식별자가 아니면 되살리지 않는다.
- 이름 불일치를 임시 fallback으로 숨기지 않는다. 정의와 실제 소비 경로를 같은 변경에서 맞춘다.
- DimensionMaster의 Server skill 계약은 `Q W E R A S D F T V ALT_V`와 LMB `2050010` automatic
  3-stage(`1500/1067/1700ms`)로 닫혔고 `ALT_V`는 `2050540`이다. merge에서 LMB 4단 수동
  window, 이전 candidate-only `2050550` 또는 Z 슬롯을 되살리지 않는다.
  candidate-only Effect는 별도의 admitted effect 계약 없이 제품 런타임에 활성화하지 않는다.

### 공용 Character Preview

- Model Preview, Animation Tool, Effect Tool이 공유하는 Character Preview Panel 계약을 유지한다.
- 충돌 해결 과정에서 툴별 두 번째 character loader, pivot owner, preview runtime을 만들지 않는다.
- project와 filters 등록, 실제 include/caller, 모델·무기 part 경로를 함께 확인한다.

### Effect Tool 재작성 경계

- Effect Tool reboot의 정본은 G0/G1에서 승인한 `Effect_AuthoringDocument`와 최소 ImGui document
  경계다. 현재 G 계획이 삭제한 레거시 `Effect_AssetIO`, `Effect_Runtime`,
  `Effect_ParticleSimulator`, `Effect_ResourceCatalog`, `Effect_Types`, 전용 Effect shader와
  생성된 `.effect` 후보 파일을 merge가 다시 살리지 않게 한다.
- 추출 원본과 증거 자료는 저작 데이터와 구분한다. Source Catalog/Extracted/참고 PNG처럼 계획이
  보존하기로 한 원본은 레거시 런타임 삭제와 함께 지우지 않는다.
- G1 Active Document는 메모리 저작 단위이며, Element 종류 radio 선택만으로 Document를 변경하지 않는다.
  G2의 Add Element 이후에만 Element가 Document에 들어간다.
- Effect asset ID와 resource ID는 `Client/Bin/Resources` 기준 상대 안정 ID다. 절대 경로,
  drive-qualified 경로, `..` 탈출 경로를 저장 계약으로 되살리지 않는다.
- 제품 Effect는 `Data/Effects/EffectCatalog.json`과 `Data/Effects/Authored/*.effect.json`만 직접 읽는다.
  `Client/Bin/DataFiles/Effect`, hash seal, VisualPrograms sidecar, Effect publisher를 merge나 복구 과정에서
  다시 만들지 않는다. Editor Save는 파일 저장과 다음-spawn Product activation을 한 transaction으로 처리하고,
  activation 실패 시 compare-and-swap으로 이전 파일과 prepared target을 모두 보존한다.

### Bone/socket scale은 transform 계층별로 검증

쿠크 MN_RPCT_05처럼 ActorX/FBX로 쿠킹한 골격은 rest/animation root basis에 이미100이
들어갈 수 있다. 실제 세 공격 clip의 combined basis100에 CModel preScale .017이 적용되면
socket 입력을 받을 배율은1.7이다. 여기에 cm→m 역보정이라고100을 다시 곱하면170이 되어
sprite 크기와 socket offset이 모두100배 커진다. `Build_SourceAnchorWorlds`는 이 중복 곱을
제거한다. bone translation과 animation scale은 유지하고, 캐릭터용 변환을 다른 쿠킹 모델에
그대로 복사하지 않는다. 원본 Size의 .01 변환, mesh modelPreScale, 골격 basis는 각각 확인한다.

finite/count 검사와 synthetic scale1.7을 넣은 CPU probe는 이170배 오류를 검출하지 못한다.
설치된 모델의 실제 bone 행렬과 local socket이 만드는 원점/축 길이를 함께 확인한다.
원본 Required/CDO를 해석한 후 `bUseLocalSpace`가 생략됐다면 legacy importer의 미확인
true fallback을 복구본에 복사하지 않는다. 기본 false는 출생 시점의 월드 transform을 유지하며,
notify의 bone-follow와는 다른 상태다. 명시 true/false와 null distribution은 임의로 덮지 않는다.
근거와 실행 범위는 [쿠크 두 패턴 결과](09-11/2026-09-11_KOUKU_GATE1_TWO_PATTERN_FULL_RESTORE_IMPLEMENTATION_RESULT.md)에 둔다.

- model prototype admission scale을 `CModel::Get_BoneMatrix()`의 combined socket scale로 간주하지 않는다.
  Artist는 admission `0.0001`과 rig `sdm` root `100`이 합성되어 `b_wp_1` combined basis가 `0.01`이다.
- Artist Effect anchor는 combined basis `0.01`을 exact tolerance로 검사한 뒤 3x3에만 `x100`을 적용한다.
  translation, animation rotation, asset orientation과 owner world는 보존한다.
- 관찰한 임의 scale 자동 정규화, `0.0001 또는 0.01` 동시 허용, raw fallback은 금지한다. 다른 class/asset에
  확대할 때는 `prototypeAdmissionScale`, `rigRootScale`, `combinedAnchorScale`, reciprocal을 manifest가 소유한다.
- 회귀는 synthetic matrix만으로 닫지 않는다. 실제 model을 제품 pretransform으로 로드해 bind pose와 대상
  animation pose의 named bone combined scale, 정규화 결과, 잘못된 scale fail-close를 Debug·Release에서 검사한다.

### ObjectManager layer-map AV는 Effect 오류와 분리

- `std::map<..., shared_ptr<CLayer>>::_Find_lower_bound` 내부 AV만으로 missing layer tag, map insertion,
  Effect clone 또는 shader를 원인으로 확정하지 않는다. key 비교 전 map tree-state 주소에서 fault면 manager/map
  lifetime, ABI, out-of-bounds 또는 선행 메모리 손상을 full dump로 구분한다.
- authored animation Effect cue와 balance `effectId` fallback은 별도 경로다. `effectId=""`만 보고 cue가 없다고
  결론내리지 말고 실제 `.animevents`의 published `effectref=asset` row와 runtime caller를 함께 확인한다.
- 위험 경로를 queue로 옮긴 뒤 같은 RVA가 재현되면 그 변경을 root fix로 기록하지 않는다. Client 전용 full
  LocalDump의 전체 stack과 matching EXE/DLL/PDB identity가 확보되기 전에는 `OPEN`을 유지한다.

### Effect 준비 성공과 첫 GPU draw 성공은 별도 gate

- Catalog parse, typed Program 검사, DDS/SRV/sampler 준비와 prepared-cache commit이 PASS해도 실제
  `Bind -> Begin -> Draw`는 아직 한 번도 실행하지 않았을 수 있다. Effect 종료 회귀는 제품과 같은
  `CEffectObject`를 layer에 넣고 실제 fixed-step occurrence가 활성화된 시점까지 진행한 뒤 첫 draw의
  `attempted/submitted/suppressed/failed/committed`를 검사한다.
- native-v14 Artist 문서를 format-13 runtime drawable로 낮출 때 `Renderer.eType`과
  `Renderer.eSourceSpace`를 지우지 않는다. 35개 element는 stable element/emitter ID로 Program의
  renderer/source-space와 다시 exact join하고, aggregate family count만 맞는 것은 증거로 쓰지 않는다.
- 반대로 기존 v3~v13 문서에는 `SourceRecipe.bEnabled=true`이면서 `Renderer==END`인 정상 저작 문서가
  존재한다. 이 경로는 kind와 geometry binding으로 legacy family를 결정한다. typed Artist 규칙을
  legacy에 역적용하거나 `SourceRecipe.bEnabled`를 이유로 GPU occurrence를 제거하지 않는다. 2026-08-12
  기준 회귀 분모는 authored 18문서, particle/decal GPU occurrence 1,300개다.
- Effect 하나의 frozen input, packet denominator, profile, local resource identity 위반은 명시적인
  `LOCAL_CONTRACT`로 기록하고 해당 object만 격리한다. D3D Map/draw, device removal, OOM, global render
  target과 presentation capacity 실패는 `GLOBAL_RUNTIME`으로 전파한다. `E_FAIL` 값만 보고 둘을 추론하지 않는다.
- 첫 draw 회귀는 최소 Artist Full35와 legacy Lance BA1을 함께 태운다. Artist만 검사하면 legacy
  renderer fallback 퇴행을, Lance만 검사하면 35행 typed renderer join 퇴행을 놓친다. 화면 모양은 이
  자동 gate가 녹색인 뒤 사용자가 별도로 판정한다.

### 요청한 키 슬롯의 실제 Effect 연결 확인

F 번개를 Alt+V로 옮기는 요청은 키 이름만으로 기존 패치 함수를 재사용하지 않는다.
워로드 F17140의 native446 mesh 번개를 V17170에만 추가했던 패치는 Alt+V17250을 수정하지
않았다. `PlayerSkills → skillbindings → animevents → 실제 두 clip effect ID`를 확인하고,
같은 번개 요청은 F의 geometry/WPO/Dynamic/수명/root snapshot을 함께 재사용한다.
색상은 실제 native 입력을 바꾸고, 다른 sprite 복제나 표시 이름 변경을 번개 연결로 기록하지 않는다.
상세는 [워로드 결과](09-09/2026-09-09_WARLORD_ASVF_FULL_RESTORE_IMPLEMENTATION_RESULT.md)의 Alt+V 항목을 따른다.

### 원본 월드 좌표 입력과 이동량 기반 생성

거미카운터2349의 world-offset 재질은 VS가 전달한 월드 위치와 원본 PS의 alpha 입력을 함께
복구해야 한다. 미해석 register를0으로 초기화하면 원본 텍스처가 있어도 장판이 투명해진다.
같은 재질의 donor 이름만 맞추지 않고 실제 permutation·VS/PS 입출력·emitter 역행렬을 확인한다.
native ribbon2346도 material admission, point RGBA/width/dynamic, renderer vertex 소비를 함께 닫는다.

Rate/Burst0인 source emitter는 SpawnPerUnit 유무를 먼저 확인한다. 거미 돌진의2345/2347/2348은
저속.1m/s CPU 입력에서0개였지만10m/s에서 모두 생성됐다. 원본 거리 단위가 누적되기 전에
검사를 끝내거나 임의 burst를 추가하지 않는다. 자세한 증거는
[거미카운터 결과](09-11/2026-09-11_KOUKU_SPIDER_COUNTER_SOURCE_EFFECT_IMPLEMENTATION_RESULT.md)에 둔다.

### 패턴 Effect 추가 후 실제 Product 편입까지 확인

Kouku publisher의 exit0은 모든 저장 패턴이 Product에 들어갔다는 뜻이 아니다. projector는
잘못된 패턴을 unavailable로 격리한 뒤 정상 패턴을 배포할 수 있다. 거미카운터 후반 준비 clip에
원본 먼지의 긴 Effect 수명을 그대로 더하면 `presentation occurrence exceeds the Pattern lifetime`로
기존 패턴과 그 bundle이 빠진다. Effect 내부의 원본 시간은 유지하고 새 occurrence의 종료는
기존 패턴 창 안에서 정한다. 기존 animation/logic을 이펙트 때문에 연장하지 않는다.
대상 patternId의 unavailableReason, generated patternbindings의 실제 Effect asset ID와
occurrence 수까지 확인한 뒤 연결 완료로 기록한다. 전체 count·JSON parse만으로 대체하지 않는다.

### Effect Tool Solo와 선택 그룹의 재생 시계

복원 문서 Solo가 object를 만들었다는 이유로 재생 완료로 처리하지 않는다. 실제 소유자인
`CEffectAuthoringSequencer::Preview_Element`가 마지막에 paused=true로 commit하면 사용자가
Sequencer Play를 한 번 더 눌러야 한다. Solo/Play Group은 준비와 첫 샘플이 성공한 뒤 즉시
재생 상태로 commit하고, 선택ID는 기존 Shift/Ctrl 표시 집합을 사용한다.

선택 그룹은 원본의 min start부터 선택 element의 실제 max end까지 같은 시계를 반복한다.
긴 원본 문서·애니메이션·숨은 provider의 수명을 반복 종료로 쓰지 않는다. 필요한 provider와
model cue anchor는 함께 준비하되, group 반복 여부는 임시 preview row가 소유한다.
Stop/문서 전환으로 임시 재생을 정리하고 저장된 sequence의 Loop 설정을 덮어쓰지 않는다.

### Artist F 수동 검증 경계

- 화면의 최종 판정자는 사용자다. 에이전트는 Client HWND나 Effect Tool을 자율적으로 실행·조작하거나
  직접 캡처하지 않는다.
- 사용자가 첨부한 실행 화면이나 이미지를 분석해 달라고 요청하면 반드시 열람·분석한다. 분석 결과는
  occurrence별 진단·리뷰 입력으로 사용하되 최종 visual PASS나 단독 admission 증거로 승격하지 않는다.
- 자동 증거는 compile, structured diagnostic, resource/shader/draw 수치에 한정한다. 에이전트가 직접 만든
  캡처나 자동 클릭 결과를 구현·리뷰·완료 증거로 사용하지 않는다.
- 에이전트는 Server CMD와 Client 준비 상태, 정확한 수동 클릭 경로만 전달한다. 사용자의 관찰 결과를
  받은 뒤에만 occurrence별 결함과 튜닝 작업을 이어간다.

### Debug Client `abort()` 팝업

- `Client/Bin/Debug/Client.exe` 실행 직후 Microsoft Visual C++ Runtime Library의
  `abort() has been called`가 발생하면 창 핸들이나 흰 배경의 Win32 창이 존재한다는 이유로
  시작 성공으로 처리하지 않는다. Lobby 첫 렌더 프레임이 보여야 시작 smoke 성공이다.
- 확인된 사례에서는 `CMainApp::Ready_Fonts()`가
  `Client/Bin/Resources/Fonts/161ex.spritefont`를 요구했지만 실제 파일이
  `Client/Bin/Resources/Fonts/Fonts/161ex.spritefont`에 있어 한 단계 중첩돼 있었다.
  DirectXTK `BinaryReader`가 `0x80070002` 파일 없음 오류를 기록한 뒤 `SpriteFont` 생성자에서
  C++ 예외 `0xE06D7363`을 던졌고, 처리되지 않은 예외가 `std::terminate()`와 `abort()`로 끝났다.
- `CCustomFont::Initialize()`의 `make_unique<SpriteFont>()`는 실패 시 `HRESULT`를 반환하기 전에
  예외를 던질 수 있다. 이 경계는 2026-08-05에 `std::exception`과 알 수 없는 예외를 포착하고
  생성 중 객체를 정리한 뒤 `E_FAIL`을 반환하도록 수정됐다. merge에서 이 catch를 제거하면
  `CCustomFont::Create()`의 `FAILED(Initialize())` 경계를 건너뛰고 `abort()`가 재발한다.
- 복구 전에는 `Fonts/Fonts` 중첩 여부와 다음 필수 파일이 `Resources/Fonts` 바로 아래에 있는지
  확인한다: `161ex.spritefont`, `YG760.spritefont`, `YG330.spritefont`,
  `YoonGasiIIM.spritefont`, `BMKkubulim.spritefont`. 임의 fallback 경로를 추가하지 않고
  immutable resource pack의 올바른 구조로 Hydrate한다.
- 제품 오류 표시는 font 생성 예외를 파일별 로드 경계에서 포착해 실패 경로를 보존하고 기존
  Client 초기화 실패 메시지 경계로 전달한다. `catch (...)`는 C++ 표준 예외가 아닌 DirectX 경계도
  process abort로 빠지지 않게 하는 마지막 변환이며 성공으로 삼지 않고 반드시 `E_FAIL`을 반환한다.
  기본 폰트나 다른 디렉터리로 자동 fallback해 정상 시작으로 위장하지 않는다.
- 수정 후에는 Debug Client를 다시 빌드하고 사용자가 아무 입력 없이 실행해 Lobby 렌더와 `abort()` 부재를
  눈으로 확인한다. 에이전트는 공유된 결과와 종료 후 잔류 Client process 부재, resource pack의 `Verify`
  결과를 별도로 확인한다.

### 프로젝트·데이터 등록

- 물리 C++ 파일, `.vcxproj`, `.vcxproj.filters`의 등록을 세트로 비교한다.
- 삭제한 manifest나 생성물을 project가 계속 등록하지 않는지 확인한다.
- Git 관리 `Data` 원본은 Client 프로젝트의 `96.DataFiles` 아래 `None`으로만 노출한다.
- `Client/Bin/Resources`는 팀장이 관리하는 runtime 입력이다. 존재하지 않는 asset pack lock이나 immutable manifest를 새 완료 조건으로 만들지 않는다.

## 3. 병합 후 변경 범위 확인

실행하지 않은 항목을 PASS로 쓰지 않는다. 아래는 문제별 확인 지점이며 매 변경마다 전부 수행하는 체크리스트가 아니다.

```text
1. conflict marker와 unmerged path 0개
2. Dimensionist/DimensionMaster 잔류를 활성 코드·데이터와 역사/원본 자료로 분류
3. Effect 레거시 파일·symbol·project 등록 0개, G1 파일·등록 존재
4. Character Preview 공용 경로와 project/filter 등록 확인
5. 변경 JSON과 XML parse
6. 변경한 코드의 Debug compile/link
7. 사용자가 직접 수행한 Character Select 재진입 또는 Effect Tool 수동 smoke의 서면 결과
8. 데이터 배포가 필요한 경우 변경한 domain의 publisher, 재현 중인 오류에 필요한 작은 검사
9. git diff --check
10. 잔류 Client/Server process와 listener 확인
```

개인 PC 경로, 실행 중인 세션 메모, 임시 예외는 `.md/GB/gotchas.local.md`에만 기록하고
Git에 커밋하지 않는다.

## 12. Effect 복원에서 비싸게 배운 것 (2026-08-17 실측)

### 12.1 데이터가 맞아도 담을 그릇이 없으면 화면에 안 나온다

Track A는 원본 추출과 매핑에 성공했다. 실패는 그 다음에 났다.
`Data/Effects/Authored` 3,400 element 실측이다.

```text
source 소유 1,909 element 의 Detail 적용률
  scale / maxParticles / particleLife / startSize / timingLife   100%
  anchor 48%   position 39%
  rotation 1%   color 0%   velocity 0%
```

정적인 축은 전부 정확히 들어갔다. 안 들어간 것은 **시간에 따라 변하는 축과 방향 축**이고,
이유는 추출 실패가 아니라 `Detail` 스키마에 그 개념이 없어서다.

`sourceRecipe.modules` 인구조사가 어디에 정보가 남았는지 말해준다.

```text
particlemodulesizemultiplylife     2259   수명에 따른 크기
particlemodulecolorscaleoverlife   1982   수명에 따른 색·알파
particlemodulelocation             1343   스폰 형태
particlemoduleparameterdynamic     1361
particlemodulecolor                1324
particlemodulevelocity              580   초기 속도
```

**교훈**: 추출이 끝났다고 복원이 끝난 것이 아니다. 저작 스키마가 그 축을 표현할 수 있는지를
추출 전에 확인한다. 표현할 수 없으면 추출한 값은 문서에 남아도 화면에 오지 않는다.

### 12.2 재생 소유권이 저작 수치를 무효로 만든다

`sourceRecipe.enabled`가 true면 재생이 원본 모듈을 따라가고 저작 `Detail`은 무시된다.
`Effect_Playback.cpp:669` 외 6곳이 그 게이트다.

```text
Detail.Transform    게이트 없음   -> 크기·위치 튜닝은 먹는다
Detail.Particle     게이트 있음   -> particle 수 튜닝은 안 먹는다
```

"어떤 스킬은 되고 어떤 스킬은 안 된다"의 정체가 이것이다. 스킬 차이가 아니라 **건드린 축의 차이**다.
튜닝이 안 먹으면 먼저 이 플래그를 본다.

### 12.3 제품이 보는 문서와 저작할 수 있는 문서가 달랐다

같은 스킬에 문서가 네 갈래였다.

```text
.unified                 sourceRecipe 소유   <- runtime catalog 99개 중 98개가 이것
.effect.json             저작 소유           <- catalog 에 없다
.authored-baseline       저작 소유           <- catalog 에 없다
.restoration-candidate   저작 소유           <- catalog 에 없다
```

저작 가능한 문서는 화면에 나오지 않고, 화면에 나오는 문서는 저작이 무시됐다.
**저작 전에 그 문서가 runtime catalog 에 실려 있는지 확인한다.**

### 12.4 중복 판정의 기준을 바꾸면 답이 뒤집힌다 (2026-08-17 재실측으로 교정)

이 절은 원래 "Track A import 가 source occurrence 당 element 를 만들어 같은 시각 요소가
5~6번씩 들어갔다"고 기록했고 `element 7,861 -> 3,042`, `210.9 MB -> 84.9 MB`를 근거로 삼았다.
**그 수치는 잘못된 signature 의 산물이며 중복은 실재하지 않았다.** 되돌리기 `413e4e36`
이후 같은 corpus 8,219 element 를 세 기준으로 다시 셌다.

```text
binding (slotId, assetId) 만          4,759   57.9%
binding + transform                   4,003   48.7%
element 전체 (id/displayName 제외)        6    0.1%
```

되돌린 규칙이 합쳤을 4,471쌍을 열어보면 detail.particle 11,855, sourceNode 4,449,
detail.timing 2,847, detail.transform 1,386, 심지어 kind 109 가 서로 다르다. 한 텍스처를
여러 위치·크기·입자수로 배치하는 것이 이펙트 구성 방식이므로 binding 기반 판정은 공간
구조를 파괴한다. 워로드 17030 이 21 -> 9 로 줄고 손튜닝 하나가 사라진 것이 그 결과다.

`NO_RESOURCES` 일괄 삭제도 틀렸다. light 44 중 28, screenPost 56 중 39 는 텍스처를
바인딩하지 않는 것이 정상이다.

**교훈**: 일괄 삭제 전에 무엇을 동일성의 기준으로 삼았는지 먼저 쓰고, 그 기준을 한 단계
엄격하게 바꿨을 때 답이 얼마나 달라지는지 재본다. 두 수치의 차이가 크면 기준이 틀린 것이다.
남은 진짜 중복 6개도 additive 로 겹쳐 그려지므로 지우면 그 element 밝기가 절반이 된다.

### 12.4.1 소유권 flip 은 축별 게이트가 아니다

`sourceRecipe.enabled = false` 는 `Effect_Playback.cpp` 28개 지점에서 시뮬레이터 전체를
갈아탄다. 따라서 이식 도구가 "해석 못 하는 모듈은 건드리지 않는다"고 해도 flip 이후에는
그 모듈이 실행되지 않으므로 보존이 아니라 삭제다.

```text
source 소유 particle 4,609
   모든 모듈이 저작 스키마로 표현 가능      109   2.4%
   최소 한 축을 잃음                     4,500  97.6%
   주요 손실: parameterdynamic 3,058, cameraoffset 1,713, rotation 1,614,
              meshrotation 1,161, orientationaxislock 1,023, subuv, orbit, acceleration
```

그리고 `Detail.Color.multiply` 와 `Detail.Transform` 은 source 소유 element 에서도 이미
합성되어 먹는다(`Effect_Playback.cpp` ~4996 의 `ElementColor * Particle.vColor`). 막혀 있던
것은 `Detail.Particle` 축뿐이다. 그래서 소유권을 내리는 대신 저작 배율을 원본 결과 위에
곱하는 `Detail.Particle.sourceScale` 을 넣었다. 축이 더 필요하면 flip 이 아니라 같은 방식으로
하나씩 추가한다. 상세는
`.md/GB/08-17/2026-08-17_EFFECT_SOURCE_TRIM_AND_DEDUP_CORRECTION_RESULT.md`.

### 12.5 원본 동일의 비용 단위는 스킬 수가 아니라 exact program과 ABI다

도화가 F가 가장 높은 화면 완성도를 낸 것은 generic profile 하나에 맡긴 결과가 아니라, stable
occurrence와 resource 역할을 고정하고 다음 translated/typed 셰이더들을 실제 carrier에 연결했기 때문이다.

```text
Shader_Artist31470RuntimeMaterial.hlsli         34 sample
Shader_Artist31470Active003RibbonMaterial.hlsli  2
Shader_Artist31470Active011OuterMaterial.hlsli   4
Shader_Artist31470Active022DecalMaterial.hlsli   1
Shader_Artist31470Diagnostic.hlsli               6
```

`g_SourceTexture0..6`을 실제로 샘플링하는 것은 이 파일들과 decal adapter 뿐이고,
표준 경로 `Shader_EffectCommon.hlsli`는 이름 있는 5개만 샘플링한다.

**교훈**: "원본과 동일"은 element나 스킬마다 셰이더 한 벌을 요구하지 않는다. equation이 같은
occurrence는 translated HLSL program과 renderer adapter를 재사용하고 texture·CB·sampler 차이는
exact descriptor가 소유한다. equation이 다르면 새 program, VF/pass/scene/RT topology가 다르면 새
adapter가 필요하다. 도화가 F에서 사람이 쓴 전용 파일은 이 경계를 처음 증명한 선례이지,
스킬별 renderer 복제를 정본으로 만든 근거가 아니다. 현재 공정은
[`EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md`](../TEAM/EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md)를
따른다.

### 12.6 판정자 없는 목표를 세우지 않는다

`100% 복원`, `원본과 동일`은 판정할 oracle 이 없으면 완료를 선언할 수 없는 목표다.
그런 목표는 진척감을 없애고, 그 자리를 커밋 수와 문서 수 같은 대체 지표가 채운다.

목표를 쓰기 전에 **무엇을 보면 끝났다고 판정하는가**를 한 줄로 먼저 쓴다.
그 판정자가 없으면 목표를 바꾼다. 화면에 변화 없는 작업이 이틀 연속이면 경보로 취급한다.
## Valtan source carrier를 system-wide mesh로 합치면 100배 WModel과 carrier 붕괴가 함께 난다

- 발탄 ParticleSystem 하나에 mesh emitter가 있다는 이유로 같은 system의 모든 emitter에
  `meshModel`을 복사하면 안 된다. emitter별 원본 carrier가 정본이며 Sprite, Mesh, Decal,
  Light를 각각 보존해야 한다.
- `build_valtan_stage_effects.py` 계열의 clip aggregate는 source 감사 자료일 뿐 V1 Product 입력이
  아니다. reviewed occurrence와 `carrierKey + sourceOrder + rendererShape`가 exact join된 행만
  candidate element가 될 수 있다.
- `Effect/Valtan/Meshes/**/*.wmodel`을 사용하는 exact Mesh carrier는
  `detail.mesh.modelPreScale=0.01`을 반드시 가진다. 런타임 기본값 `1.0`을 쓰면 같은 WModel이
  100배로 렌더링된다.
- Sprite/Decal/Light carrier에는 `meshModel`과 `modelPreScale`을 넣지 않는다. exact resource와
  portable runtime closure가 닫힌 Sprite/Mesh/Decal은 source material identity를 보존한 채
  `effect.standard + alpha_two_sided_depth_read` 공통 RT0로 손튜닝 시작점을 만들 수 있다. Light,
  ScreenPost, resource/adapter 미해석 행은 임의 quad/mesh로 위장하지 않고 `BLOCKED_REQUIRED`로
  남긴다. 공통 RT0 승격은 family 복원이나 `V1_COMPLETE`를 뜻하지 않는다.
- 회귀 검증은 후보 전체에 대해 `rendererShape=mesh <=> meshModel 1개 + modelPreScale 0.01`과
  `rendererShape!=mesh => meshModel 0개`를 함께 검사해야 한다.
- 발탄 materialization receipt가 전체 `EffectCatalog.json` 해시를 봉인하면 다른 캐릭터가 catalog
  행 하나를 추가하는 것만으로 발탄 검증이 실패한다. receipt는 `effect.valtan.` slice와 catalog
  formatVersion만 봉인하고, 전체 catalog 보존은 publisher가 담당해야 한다. 그래야 병렬 캐릭터
  복원과 발탄 exact carrier 검증이 서로의 Product를 지우거나 재봉인하지 않는다.

## Valtan Product presentationScale 정본은 1.0이고 10배 대상은 본체 HP다

- 2026-08-28 사용자 정정에 따라 `Data/Actors/BossCatalog.json`의
  `BOSS_VALTAN.presentationScale`은 `1.0`이 정본이다. `10.0`은 HP 10배 요청을 시각 배율에 잘못 적용한 값이므로
  다시 복원하지 않는다.
- Server 권위 정본 `Data/Balance/BossProfiles.json`의 일반 `BOSS_VALTAN.maximumHp`는 `600000`,
  `maximumHealthBars`는 `160`이다. 종속 `BOSS_VALTAN_GHOST.maximumHp`는 `60000`을 유지한다.
- Server `collisionRadius: 1.4`와 공격 hit geometry는 scale-one gameplay 기준을 유지한다.
- `GAMEPLAY_FOOTPRINT`는 owner basis scale을 제거한 뒤 authored world scale을 적용하므로,
  Effect footprint 보정이나 미세 scale drift 허용을 이유로 boss presentationScale을 바꾸지 않는다.
- `test_valtan_model_view_composition.py`가 `1.0`과 Server body radius `1.4`를 함께 고정하고,
  gameplay balance/Server 계약 테스트가 본체 HP `600000`을 고정한다.

## Valtan strict join과 Effect Tool에서 재발시키지 않을 경계

### Boss Pattern/All Effects/Composition 목록이 함께 사라지면 공용 graph admission부터 본다

Boss Tool, Effect Tool의 `All Effects -> Valtan`, Composition Patterns는 서로 별개의
패턴 catalog를 갖지 않는다. 셋 모두 `Valtan.gameplay.json`,
`Valtan.presentation.json`, generated `ValtanEncounter.json`, rotation, Effect binding,
BossCatalog/combat-object를 strict join한 동일한 canonical Pattern tree에서 목록을 투영한다.
따라서 아래와 같은 화면은 Effect resource나 ImGui category 자체의 문제가 아니라 공용 graph
admission 실패의 연쇄 증상일 가능성이 가장 높다.

```text
No Valtan pattern inventory was staged
0 canonical patterns
canonical Valtan graph did not load
live only; outside All Effects list / UNKNOWN ACTION
Save/Restart button disabled
```

Server는 이미 publish된 bootstrap으로 계속 패턴을 실행할 수 있지만 Client Tool의 새 graph만
거부될 수 있다. 이때 `live only`와 `UNKNOWN ACTION`은 “Server에 패턴이 없다”는 뜻이 아니라,
현재 Server snapshot의 `patternId/actionId`를 Client의 admitted tree에서 resolve하지 못했다는
뜻이다. 화면의 마지막 증상부터 고치지 말고 `Graph reload failed:` 뒤 최초 strict-join 오류를
먼저 고친다.

한 field가 세 화면을 모두 비운 구조적 이유도 함께 기억한다.

- split parser는 모든 managed Pattern을 한 transaction으로 join하고 한 Stage 오류에서 전체 candidate를
  rollback한다. partial authoring tree를 정상 Product처럼 보여 주지 않는 것은 맞지만, 기존 구현은
  generated Product 표시보다 strict split join을 먼저 실행했다.
- last-good snapshot 보존은 같은 process에서 한 번 이상 성공한 뒤에만 가능했다. process restart 직후
  첫 load가 실패하면 Boss Tool, All Effects, Composition 모두 보존할 snapshot이 없어 0개가 됐다.
- 세 Tool이 한 process-level snapshot을 공유하지 않고 각각 같은 tree를 reload했다. 그래서 하나의
  source 오류가 세 곳에서 서로 다른 빈 화면과 버튼 비활성화로 반복 노출됐다.
- Boss Tool과 Effect Tool은 첫 자동 load 실패 뒤 `attempted` 상태가 남는다. 파일을 고친 뒤에도 explicit
  `Refresh/Retry Graph Load` 전에는 다시 시도하지 않아 “수정했는데 여전히 0개”처럼 보일 수 있다.
- reference/legacy/live-only Pattern 하나가 Complete Play 목록에 없는 것은 정상 필터다. 이 경우 graph
  자체는 loaded이고 해당 row만 `[live only; outside All Effects list]`다. 이번처럼 graph 전체가
  `did not load`인 경우와 혼동하지 않는다.

이번 작업과 인접 세션에서 실제 확인한 원인은 다음과 같다.

| 최초 오류/증상 | 실제 원인 | 재발 방지 |
|---|---|---|
| `split gameplay defaultNextActionId drifted: VALTAN_CATCH_BREATH/STEP_02` | Catch 성공/실패를 `ANY_PLAYER_GRABBED -> STEP_03`, `TIMEOUT -> terminal`로 바꾸면서 author script와 generated Product는 갱신했지만 split gameplay의 호환 필드 `defaultNextActionId=STEP_03`이 남았다. strict reader는 명시적 TIMEOUT target인 `null`을 기본 edge로 요구하므로 전체 graph를 거부했다. | branch를 바꾸는 writer는 `defaultNextActionId`를 독립 입력으로 받지 않고 `TIMEOUT.nextActionId`, ordered fallthrough, terminal 순으로 derive한다. `VALTAN_CATCH_BREATH/STEP_02`의 source는 반드시 `defaultNextActionId: null`이어야 한다. author script, split source, generated Encounter를 같은 transaction/revision으로 닫고 negative drift fixture를 실행한다. |
| `split gameplay SET_PLAYER_BIND event is invalid` | Bind event의 exact typed 계약과 source가 달랐다. ENTER는 `heightM=5`, `durationMs=stage duration`, EXIT는 `heightM=0`, `durationMs=0`이어야 하며 unknown/extra field도 거부한다. 한 event의 실패가 split master 전체를 거부했다. | event schema, author script, split source, projection과 runtime consumer를 같은 변경으로 수정한다. parser를 느슨하게 하거나 legacy event로 fallback하지 않는다. |
| `split gameplay SET_PLAYER_SILENCE event is invalid`와 `Encounter stage action lifetime is not closed: VALTAN_SILENCE_SLOT` | `SILENCE_APPLY`는 100ms Stage에서 침묵 5000ms를 한 번 설정하는 deadline-latched 계약이다. Publisher와 Server는 `ENTER only`, `duration >= Stage`, pattern 밖 deadline 만료를 정본으로 사용했지만 Client strict reader는 `duration == Stage`를, Product reference reader는 paired `EXIT`를 요구했다. strict와 fallback이 동시에 실패해 All Effects에는 `EXISTING AUTHORED EFFECTS`와 별도 Area `INDEPENDENT EFFECT`만 남고, Composition/Boss 목록과 Arena presentation admission까지 연쇄 차단됐다. | Client strict source reader와 Product fallback reader가 각각 deadline-latched Silence를 승인하게 하고, Publisher/Server와 동일한 truth table을 focused/native parity 회귀로 고정한다. Silence는 lifetime closure set에 넣지 않는다. actual current JSON을 compiled `ValtanPatternAuditionServiceHarness`로 로드해 ENTER 5000ms 승인, EXIT/value 0/duration < Stage 거부, rollback을 검사한다. Python publisher PASS만으로 Client admission PASS라고 결론내리지 않는다. |
| `master independent Effect did not resolve to one Product owner/document` | V2 도끼를 보이게 하려는 변경에서 independent Effect master row를 추가했지만 exact Product cue/combat-object owner 또는 authored Effect 문서가 정확히 하나로 join되지 않았다. runtime binding 변경과 authoring ownership 변경을 섞어 전체 tree를 깨뜨린 사례다. | 단순 runtime visual 교체는 runtime binding만 바꾼다. independent row는 실제 Product owner, cue timing, authored document가 모두 존재할 때만 추가한다. V1/V2 편집 도구의 목록 소유권을 Product pattern owner로 위조하지 않는다. |
| `BOSS_VALTAN combat-object visual identity is invalid or duplicated` | BossCatalog의 `combatObjectVisuals`에 같은 stable `combatObjectArchetypeId`가 중복되었거나 빈 `clientVisualId/effectAssetId`가 들어갔다. | catalog/projection은 archetype당 visual row 정확히 하나를 보장한다. 기존 row를 교체할 때 append하지 말고 stable ID로 replace하며 combat-object source와 exact join한다. |
| `Valtan scripted-sequence Product parity drifted` 또는 `0 canonical patterns` | saved Boss audition Flow의 occurrence order와 automatic Product rotation order를 하나의 동일 order로 오인해 exact-equal 비교했다. 서로 다른 소유자를 한 revision처럼 묶으면서 전체 tree가 fail-close했다. | saved Flow reference는 Boss Tool audition order를, Product rotation은 자동 전투 order를 각각 소유한다. schema/ID 존재는 함께 검증하되 두 order의 equality를 요구하지 않는다. legacy inline sequence만 기존 parity를 유지한다. |
| old Client에서 새 motion/schema를 연 뒤 모든 목록과 Play가 차단됨 | Data는 새 `{kind, retargetDelayMs, speedMps, distanceM}` 계약인데 실행 중 EXE는 구 parser였다. build가 실행 중 Server/Client의 출력 잠금에서 멈췄는데도 새 EXE로 오인했다. | build 호출 성공 여부가 아니라 Client EXE timestamp/receipt와 실제 strict graph load를 확인한다. 실행 중 EXE의 `LNK1104/MSB302x`는 compile과 link를 분리해 보고하며 old EXE로 새 Data를 검증하지 않는다. |
| Save Flow validation 실패 후 Restart 비활성화 | Save adapter가 합법적인 cross-pattern `COUNTER_HIT -> GROGGY`를 구형 same-pattern/local-action 규칙으로 거부하거나, 첫 candidate가 pending인 동안 두 번째 Save를 유실했다. | Counter success는 local action 또는 cross-pattern target 중 정확히 하나를 허용하고 TIMEOUT은 local failure edge로 검증한다. pending 중 두 번째 Save는 latest deferred candidate로 보존하고 첫 exact terminal 뒤 제출한다. 저장 성공, Server-active revision, 현재 실행 revision을 별도 상태로 표시한다. |
| 첫 Save는 되지만 두 번째 Save/Restart가 계속 막힘 | Apply A 결과 packet을 놓치면 Server가 이미 A를 active로 사용해도 Client transaction이 `UNCONFIRMED`에 남아 deferred B를 영구 대기시켰다. | 성공 packet을 추측해 `COMMITTED`로 만들지 않는다. 현재 연결/월드가 명시한 `ServerActiveRevision == immutable A`일 때만 `ALREADY_ACTIVE`로 reconcile하고, 그 exact A를 base로 queued B를 제출한다. 다른 revision, 다른 world의 관측, concurrent transaction에서는 계속 fail-close한다. |
| source commit 뒤 `COMMIT_SUCCEEDED_REOPEN_FAILED`, Flow는 clean인데 Save/Restart 재개 버튼 없음 | source CAS는 이미 성공했지만 editor reopen/Product publish/apply가 뒤에서 실패했다. Flow dirty flag는 clean이므로 Save 버튼은 비활성이고, 같은 source를 다시 쓰지 않고 post-commit 단계만 재시도할 typed 경로가 없었다. | durable committed revision과 당시 draft generation을 별도 보존한다. `Retry Product Publish / Apply`는 newer edit가 없을 때 그 exact revision을 reopen하고 Product publish/apply만 계속하며 source writer를 다시 호출하지 않는다. 새로운 edit가 있으면 retry가 이를 버리지 않고 거부한다. |
| Save Flow 직후 Lobby fallback과 `Server entry failed` | graph와 별개인 Server process 사망이었다. candidate artifact SHA-256 함수가 1 MiB local stack buffer를 만들었고 1 MiB Server thread stack의 함수 진입에서 `0xC00000FD` stack overflow가 발생했다. Client의 Lobby 문구는 그 뒤 연결 대상 Server가 사라진 후속 증상이다. | 해시 chunk는 bounded size를 유지하되 heap storage를 사용한다. 사용자가 수동 종료한 경우와 Server crash/fallback을 process exit code, dump/structured recovery state로 분리한다. `Server entry failed`만으로 graph 오류라고 결론내리지 않는다. |
| `Canonical Save validation failed; every source/Product owner was preserved` | 편집 source 한 곳만 바뀌고 이를 참조하는 generated Product, owner join 또는 publisher receipt가 아직 이전 revision이었다. validator가 reject한 것은 정상 rollback이며 파일이 저장되지 않았다는 뜻일 수 있다. | Save는 `parse -> validate -> stage -> source/Product CAS -> project -> post-validate -> commit` 한 transaction으로 수행한다. 생성물을 직접 고치거나 validator를 우회하지 않고 첫 stable ID/field 오류를 해결한다. |

현재 `author_valtan_phase_two_mechanics.py --mode Validate`와
`Project-ValtanPatternMaster.ps1 -Mode Validate`가 branch target 존재만 검사하면
`defaultNextActionId` drift를 놓치는 false negative가 생길 수 있었다. authoring helper와 focused
fixture에서 다음 불변식을 직접 검사한다.

```text
explicit TIMEOUT exists  -> defaultNextActionId == TIMEOUT.nextActionId
no TIMEOUT, next stage   -> defaultNextActionId == ordered next action
no TIMEOUT, terminal     -> defaultNextActionId == null/absent
```

목록 표시와 mutation admission도 분리한다. generated Product가 정상이라면 fresh launch에서도
Product pattern/action 목록은 `PRODUCT_ONLY / READ ONLY`로 표시한다. split authoring strict join이
실패하면 Save, Complete Play, Restart, Repeat, Next와 source mutation은 계속 fail-close하고, 화면에는
실패한 `patternId/stageId/field`를 그대로 남긴다. reload 실패 시 last-good display snapshot을 지우지
않는다. Product-only 표시를 authoring 성공으로 승격하거나 서로 다른 generation을 섞지 않는다.

첫 load가 source/Product writer의 짧은 lock 구간과 겹친 경우는 semantic invalid와 다르게 처리한다.
`Create/Project transaction is active`, Win32 sharing violation, admission 뒤 generation change는
transient failure다. All Effects와 Boss Tool은 last-good 또는 read-only Product fallback을 유지한 채
0.25초 간격으로 다시 admission을 시도한다. `attempted=true`만 남겨 fresh process를 영구 0 rows로
고정하지 않는다. 반대로 stable ID/schema/owner/join 오류는 자동 무한 재시도하지 않고 최초 오류를
그대로 표시한다.

Effect V2 closure에는 owner lane이 둘이다. animation의
`Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`에서 reachable한 group/leaf뿐 아니라,
`BossCatalog.json`의 `BOSS_VALTAN.combatObjectVisuals[].effectV2Group`도 포함한다. 점프 도끼의
`boss.valtan.axe`는 후자다. native receipt test가 bindings만 expected closure로 계산하면 정상 도끼
group을 extra artifact로 오판한다. runtime loader와 회귀 fixture가 두 owner lane을 같은 집합으로
계산해야 한다. `SET_PLAYER_SILENCE` stage-action 오류를 Effect 파일 오류로 오인해 V2 binding을
삭제하거나 Arena admission을 느슨하게 만들지 않는다.

Effect V2 validator를 독립 fixture에서도 재사용할 때 `Data/Actors/BossCatalog.json`은 optional
owner lane이다. 실제 제품 저장소처럼 문서가 존재하면 schema/version/Valtan visual owner를 끝까지
strict 검증하지만, Effect V2만 만든 격리 fixture에 문서가 없으면 빈 owner 집합으로 처리한다.
파일을 무조건 열면 제품 Effect는 정상인데 모든 Effect V2 단위 테스트가 `FileNotFoundError`로
무너져 Core가 compile 전에 중단된다. 반대로 제품 저장소의 손상된 BossCatalog를 optional이라는
이유로 건너뛰면 안 된다. 부재 허용, 정상 owner admission, 존재하지만 invalid인 문서의 fail-close를
세 개의 회귀 경로로 유지한다.

### 변경한 기능만 확인하고 writer 작업을 중복 실행하지 않는다

일상 수정은 필요한 compile/link 뒤 사용자가 대상 아레나에서 변경한 동작을 확인한다.
Animation 행을 추가·저장했다면 해당 보스·패턴·행의 저장/재로드/재생을 확인한다. 이 작업을
다른 보스의 PatternTree, 다른 family, 전체 oracle 또는 광역 회귀 실행의 선행조건으로 묶지 않는다.
광역 진단은 사용자가 요청할 때만 실행하며 미실행 자체를 완료·커밋 차단 사유로 쓰지 않는다.

명시 publish나 진단이 같은 writer를 사용하는 경우에는 중복·병렬 실행하지 않는다.
경합으로 발생한 `CANONICAL_TRANSACTION_BUSY`는 실제 기능 결함과 구분한다.
저장 실패 시 기존 항목을 보존하고 재시도가 필요한지 해당 기능의 상태로 알린다.

### Valtan Composition 확장에서 함께 유지할 저작·투영 계약

- `Data/Valtan/Valtan.gameplay.json`, `Valtan.presentation.json`,
  `Valtan.combatobjects.json`이 split 저작 정본이고 `Data/Encounters/Valtan/*`와 Client/Server
  bootstrap은 투영 생성물이다. source를 바꾼 직후 이전 Product와 다른 것은 정상 중간 상태이므로
  Save 전에 물리 Product parity를 요구하지 않는다. candidate를 메모리에서 project·validate한 뒤
  source/Product를 같은 CAS transaction으로 commit하고 post-validate한다. author helper가 같은 행을
  재생성한다면 helper도 함께 갱신한다. 밸런스 수치 변경은
  `2026-08-05.balance-provenance.receipt.json`의 해당 field를 `PROJECT_TUNED`로 동기화하고 정본
  publisher로 bootstrap을 다시 만든다. generated JSON/bootstrap/receipt를 결과 맞추기용으로 직접
  편집하지 않는다.
- Composition Save의 참가자는 Save edge에서 고정한 dirty owner뿐이다. Pattern/Collider만 dirty이면
  clean Pattern Sound에 `Can_Commit...Generation`, editor document load, candidate staging을 요구하지
  않고, clean Effect V2에도 draft `Prepare/Accept`를 호출하지 않는다. 다만 두 clean 물리 문서는
  candidate Product에 대한 read-set dependency로 writer 안에서 계속 strict 검증한다. dirty Sound/V2는
  exact baseline/candidate와 draft generation/revision을 stage하고, 모든 참가자의 parse·validate가 끝난
  뒤 한 번 commit하며, 실패하면 byte-exact rollback하고 성공 뒤 exact candidate로 reopen한다.
  Camera는 typed 저장 owner가 연결되기 전까지 Composition의 read-only/deep-link 경계이며 이 Save에
  빈 sidecar나 추측한 persistence를 추가하지 않는다.
- Effect V2 binding은 header를 먼저 읽어 validator를 dispatch한다. formatVersion 1만 legacy
  compatibility validator로 보내고, formatVersion 2는 exact binding pipeline과 resource read-set으로
  검증한다. v2 문서를 v1 helper에 넣어 실패시킨 뒤 schema를 완화하지 않는다. 구현이 source bytes를
  동일하게 보존하면서 copy 대신 `std::move`를 사용하도록 바뀌었으면 `*out = source` 같은 문자열을
  고정한 낡은 oracle을 고친다. CAS baseline, candidate equality, rollback이라는 의미 계약을 production
  코드의 불필요한 copy보다 우선한다.
- Pattern total duration은 Stage를 생성하는 명령이 아니며 부족한 시간을 숨은 `WAIT` Stage로 채우거나
  선택을 그 Stage로 이동시키지 않는다. Stage 추가·삭제·후속 선택은 stable Stage/action ID를 쓰는
  명시적 topology 명령만 허용한다. Animation Replace/Append도 `clips.size()==3` 같은 개수 gate를
  되살리지 않는다. HOLD 축약은 `start -> one-or-more loop -> end -> optional untagged tail` 역할을
  검증하고 fixed edge/tail을 보존한 채 loop 예산만 Stage clock에 맞춘다. 양의 loop 시간을 보장할 수
  없으면 draft를 바꾸기 전에 거부한다.
- split source가 소유하는 counter Pattern은 legacy compatibility row나 reference-only reaction layer와
  동시에 gameplay authority를 소유하지 않는다. active split owner가 Stage, `counterProxy`,
  `boss.flag.counterable` ENTER/EXIT, `COUNTER_HIT` 성공 branch와 `TIMEOUT` 실패 branch를 함께 소유하며,
  retired Pattern ID는 dangling 0건이어야 한다. Core rotation Pattern을 Details에 보이게 하려고
  `manualAuditions`나 DERIVED row로 승격하지 않는다. manual inventory admission과 counter gameplay
  ownership은 별도 계약이다.
- combat-object `spawnSchedule.firstOffsetMs`는 source에서 Product의 `firstSpawnOffsetMs`, publisher/
  bootstrap, Server scheduler, Client local preview와 Composition timeline까지 손실 없이 전달한다.
  offset이 0이면 첫 spawn을 즉시 소비하고, 0보다 크면 emitted count를 0에서 시작해 Stage-local
  clock이 offset에 도달하기 전에는 생성·표시하지 않는다. `StageStart + firstOffsetMs`가 authored
  world time이며 count/interval만 보존하고 offset을 0으로 기본화하지 않는다.
- combat-object visual이 `effectV2Group`과 V1 `effectAssetId/hitEffectAssetId`를 함께 가지면 V2는
  Product live owner이고 V1은 editor/legacy preview fallback이다. live에서 둘을 동시에 spawn해
  폭발을 이중 재생하지 않되, Server semantic presentation event는 Sound cue를 계속 구동한다. 하나의
  V2 group을 여러 combat-object archetype이 참조하는 것은 정상 재사용이며 group definition ID의
  중복과 visual reference 재사용을 같은 오류로 취급하지 않는다. Workbench는 선택된 live group과
  object-local lifetime/offset을 표시하고 fallback을 Product owner처럼 라벨링하지 않는다.
- `Client/Bin/Resources`는 Git/LFS 정본이 아니라 팀장 Drive가 전달하는 물리 runtime 입력이다.
  Ghost Valtan은 catalog에 Resources-relative `Character/Valtan/Ghost/MN_RPBF_02.wmodel`과
  `MN_RPBF_02_AnimSet.wmodel`만 저장하고, 두 WModel과 참조 DDS closure는 같은 물리 경로로 수동
  전달한다. worktree 삭제, clone, `git lfs pull`로 이 파일들이 복구된다고 가정하거나 force-add하지
  않는다. 누락 시 model-view/FullDiagnostic 차단을 코드 회귀와 구분하고 Drive 전달 prerequisite와
  exact 상대 경로를 보고한다.
- 임시 worktree, fixture, baseline 비교 폴더 안에 실제 `Client/Bin/Resources`를 가리키는 junction이나
  symlink를 만들지 않는다. `git worktree remove --force`, `Remove-Item -Recurse`, `rm -rf`는 연결된
  Git 비추적 Drive 폴더까지 순회해 원본 pack을 지울 수 있다. 별도 tree에서 물리 Resources가 필요하면
  `LOSTARK_RESOURCE_ROOT`/`LOSTARK_SHARED_ASSET_ROOT`로 실제 루트를 읽기 전용 지정하거나 검사에 필요한
  최소 파일만 복사한다. worktree 삭제 전에는 reparse point를 전수 확인하고, 발견된 link는 target을
  순회하지 않는 unlink 명령으로 먼저 분리한다.
- `Client/Bin/Resources` 자체, 그 하위 일곱 폴더, 그리고 이를 포함하는 상위 폴더는 어떤 정리·복원 요청에서도
  `rm -rf`, `Remove-Item -Recurse`, `git clean -x`로 지우지 않는다. 정리 대상은 `.vs`, `EngineSDK`, `out`,
  `x64`, 구성별 Bin 산출물처럼 재생성 가능한 폴더로 한정하고, 삭제 명령의 경로가 Resources를 포함하는지
  실행 전에 경로를 출력해 확인한다. 2026-09-03에 실제로 Drive pack 전체가 삭제된 사고가 있었다.
- 이 절의 source/Product validate, native harness, Product/Core/FullDiagnostic PASS는 화면 품질 PASS가
  아니다. Effect 반복·위치·색, collider wire, Ghost body와 Sound timing의 최종 판정은 0절의 사용자
  전용 경계를 그대로 적용하며, 사용자의 서면 관찰 전에는 first pixel, eye smoke, visual PASS를
  기록하지 않는다.
- encounter Stage에 `verticalOffsetM` 같은 optional field를 추가할 때 정본 gameplay publisher만
  고치면 끝나지 않는다. 같은 `ValtanEncounter.json`을 strict하게 다시 읽는
  `Publish-ValtanWorldDestruction.ps1` 같은 보조 publisher도 exact property set과 의미 검증을 같은
  변경 단위에서 갱신해야 한다. 그렇지 않으면 focused/source validation은 모두 통과해도 Product
  pre-build에서 `missing or unknown fields`로 중단된다. 보조 reader에서 단순히 unknown field를
  무시하지 말고 정본과 동일하게 finite/range, non-zero, active `bossResponse`, pattern/stage motion
  배타 조건을 검증하며, 이 consumer를 포함한 negative 회귀와 Product build를 완료 증거로 남긴다.
- 새 Client translation unit을 추가하면 `.vcxproj`/`.filters`뿐 아니라 Product build가 고정하는 native
  source inventory oracle도 함께 갱신한다. 실제 컴파일 오류와 stale exact-count/source-list 실패를
  구분하고, inventory 기대값을 약화하거나 새 파일을 빌드에서 빼서 통과시키지 않는다.

### Pattern/Effect schema evolution은 consumer closure matrix로 닫는다

`Stage.verticalOffsetM` 추가 뒤 실제 Client에서 확인된 연쇄 실패는 데이터 한 줄의 오류가 아니었다.
Python source validator와 projector는 새 필드를 승인했지만 Client split reader는 필수 nullable
`motion:null`을 active motion으로 오판했고, 그 오류를 고친 뒤에는 generated Product의
`SPAWN_COMBAT_OBJECT_VOLLEY.firstSpawnOffsetMs`를 모르는 read-only encounter fallback이 드러났다.
다시 그 오류를 고치자 publisher/Server만 알던 `SUPPRESS_INTER_STEP_PURSUIT`와 삼각 portal의 exact
3-point 규칙이 다음 Client reader drift로 나타났다. strict admission은 첫 실패에서 멈추므로 앞의
오류 하나가 뒤의 모든 누락을 가린다.

본질적인 원리는 다음과 같다.

- 새 field/event/effect parameter는 한 JSON 객체의 optional key가 아니라
  `source -> join/project -> generated Product -> secondary publisher -> Client source reader ->
  Client Product fallback -> bootstrap/version -> Server runtime -> preview/tool` 전체의 schema evolution이다.
- exact-property fail-close consumer는 서로 다른 목적의 로컬 schema를 가진다. primary publisher PASS나
  generated JSON diff만으로 다른 native reader의 admission을 증명할 수 없다.
- optional value는 `absent`, `present null`, `present value`를 구분한다. nullable 필드의 포인터 존재만으로
  기능이 활성화됐다고 판정하지 않는다. `motion:null`은 no motion이고 non-null object만 active motion이다.
- 시간 field는 저장/parse만의 계약이 아니다. `firstSpawnOffsetMs`는 Server spawn clock, Client preview,
  Composition과 Animation Tool lane 시작 시각까지 같은 Stage-local clock을 사용해야 한다. UI에서 0으로
  되돌리면 데이터는 맞아도 저작자가 잘못된 타임라인을 보게 된다.
- 새 필드를 실제로 소비하는 reader와 실행 경로만 확인한다. Animation/Effect 표현 수정 때문에
  관계없는 PatternTree, 보스, family의 검증을 함께 요구하지 않는다.

아래 표는 원인 조사 시 사용하는 소비 경계 참고표다. 작업마다 모든 행을 수행하거나 `N/A` 문서를
만드는 절차가 아니다. 변경한 값이 저장된 뒤 해당 아레나의 의도한 실행에 도달하는 경로를 확인한다.

| 경계 | 관련된 경우 확인할 내용 |
|---|---|
| owner/writer/helper | stable owner ID, required/optional/null 의미, 단위와 clock, save/reload/CAS rollback |
| source schema | exact key set, type/range, cross-field owner·배타 조건, generator 재실행 시 보존 |
| join/project | source 의미를 generated Product의 exact field/row로 무손실 투영하고 provenance/revision 동기화 |
| secondary publisher | 같은 Encounter/Effect 문서를 strict하게 읽는 모든 publisher의 key set과 의미 검증 동기화 |
| Client strict source reader | 현재 split source 전체를 실제 C++ reader로 읽고 in-memory view에 보존 또는 의도적 validate-and-discard |
| Client Product fallback | freshly projected Product 전체를 별도 fixture로 읽고 source reader와 같은 truth table 유지 |
| bootstrap/admission | row layout 변경 시 format version, generation admission, exact field count를 같은 변경에서 갱신 |
| Server | catalog parser, fixed-tick 실행, terminal/restore/rollback, late join snapshot 의미 확인 |
| presentation/tools | live runtime, local preview, Workbench/Animation/Effect Tool timeline·label·selection에서 동일 단위/offset 사용 |
| tests | positive survival, wrong type/range, missing owner, conflicting field, wrong trigger/stage, last-good 보존 |

Effect V2의 slot/parameter/group timing 변경은 해당 occurrence의 실제 저장값과 실행 시간을 확인한다.
문제가 재현되면 그 occurrence의 Document/Catalog/Runtime 경로를 따라가며 필요한 작은 검사만 사용한다.
전체 catalog·Valtan·WorldDestruction 진단을 매 수정의 완료 조건으로 연결하지 않는다.

오류 메시지는 `v4 field is invalid` 하나로 motion/action/branch 실패를 뭉개지 않는다. 최소
`document kind + patternId + stageId + field/action family + 위반 predicate`를 남기고, 실패 시 이전 admitted
tree/reference를 보존한다. 지원하지 않는 실행 항목은 정상값으로 위장하지 않고 그 항목에 오류를
표시한다. 오류 항목 때문에 다른 정상 패턴·family의 목록, 편집 또는 재생 상태를 초기화하지 않는다.

### Effect 세대를 한 화면에 합칠 때 backend catalog를 다시 직접 순회하지 않는다

V1 authored 문서는 `elements[]` 전체가 하나의 원자적 composition이고, V2는 leaf와 ordered group을
분리해 저장한다. 두 저장 형식을 하나로 보이게 한다는 이유로 V1 element를 V2 leaf처럼 펼치거나,
같은 V1 문서를 배치 수만큼 복제하지 않는다. 공용 authoring resource 계약은
`EFFECT_RESOURCE_KEY { ownerKind, stableId }`와 immutable `CEffectResourceCatalog` snapshot이며,
`V1_DOCUMENT`와 `V2_GROUP`은 같은 `Groups`, `V2_LEAF`는 `Leaves`에 표시한다. 이는 무손실 group
승격이지 V1 JSON을 불완전한 V2 field로 변환하는 migration이 아니다.

- All Effects는 `CEffectResourceCatalog` facade의 owner-kind과 stable ID를 소비해 V1/V2를 한
  화면에 표시한다. Action Composition Workbench는 현재 `V1 Pattern Effects`,
  `V2 Authored Effects`, `V2 Effect Groups`를 각각 명시적 owner lane으로 유지한다. 단일 writer가
  없는데 facade snapshot을 공유한다고 기록하거나 backend을 혼합하지 않는다.
- 선택 identity는 해당 backend의 owner kind와 stable ID를 함께 보존한다. V1 document append는
  기존 exact clip cue writer로, V2 leaf/group append는 typed stage binding writer로 dispatch한다.
  현재 코드에는 V1/V2/Camera/Catalog를 하나로 묶는 canonical mutation coordinator가 없으므로,
  이를 기존 저장 경로의 완료 계약으로 가정하지 않는다.
- owner refresh 또는 facade join이 실패하면 이전 snapshot은 표시용으로 유지하되 append/save는
  `STALE PRESERVED / READ ONLY`로 막는다. 실패한 새 owner와 이전 다른 owner를 섞어 새 snapshot처럼
  표시하지 않는다.
- Ground Roar 4방향 배치는 V1 active/explode 문서를 24/4 element로 복사하는 문제가 아니다. active
  6개와 explode 1개인 원본 atomic group을 유지하고 Server combat-object volley가 boss-relative
  `radiusM=4.9497475`, `startAngleDegrees=45`, `angleStepDegrees=90`의 root 네 개를 만든다.
  boss yaw 0도 기준 각 root는 X/Z `(3.5,3.5)`, `(3.5,-3.5)`, `(-3.5,-3.5)`,
  `(-3.5,3.5)`이며 boss yaw를 따라 함께 회전한다. element 복제와 root instancing을 동시에 적용하면
  16배 occurrence가 생기므로 회귀가 두 계약을 함께 검사해야 한다.
- 피해 없는 boss-relative four-rock owner를 새로 추가할 때는 `GameRoom`의 off-navigation exact owner
  집합도 같은 변경에서 갱신한다. `FIXED_AREA + direction NONE + hits=[] + presentation pulse + count=4`
  의미 조건과 `(combatObject, pattern, action)` 튜플을 모두 만족할 때만 authored root가 navgrid 밖에
  놓이는 것을 허용한다. 모든 visual object를 포괄 허용하거나 좌표를 project/clamp하지 않는다.
- `firstSpawnOffsetMs`가 있는 volley는 ENTER 테스트만으로 검증되지 않는다. 실제
  `Apply_BossPatternScheduledSpawnWave`에서 due 직전 no-op, due tick atomic spawn, 다음 tick 중복 없음과
  damage hit 주입 시 live/pending lifecycle 0개인 strict reject를 함께 검사한다. 이 경로의 실패는
  room을 not-ready로 만들고 다음 정상 입력에서 session FIN으로 이어져 Client에는
  `Valtan replication observed a disconnected Server session.`, Lobby에는 공통
  `Server entry failed.`로 보일 수 있다.

Save/Restart 진단에서는 한 문장인 `SAVED`를 다음 상태로 나눠 확인한다.

```text
SOURCE_COMMITTED -> EDITOR_REOPENED -> CANDIDATE_PUBLISHED
                 -> APPLY_PENDING -> SERVER_ACTIVE -> FLOW_RESTART_ADMITTED
```

- 앞 단계 성공은 뒤 단계 성공을 뜻하지 않는다. source commit 성공 뒤 reopen 실패라면 source를 다시
  쓰지 않고 post-commit retry를 제공한다.
- `UNCONFIRMED`는 실패도 성공도 아니다. exact Server-active revision 관측 전에는 다음 revision을
  제출하지 않는다.
- Restart는 saved Flow content, latest candidate, Server-active Product revision, presentation generation을
  각각 exact 비교한다. 버튼을 억지로 활성화하거나 이전 candidate로 fallback하지 않는다.

`Restart Pattern`과 `Restart Flow`는 같은 명령이 아니다. 과거 Boss Verification의
`Restart Saved Pattern (Fresh Arena)`는 내부에서 `Restart_SavedFlow(true)`를 호출했기 때문에,
saved slot이 하나일 때는 실제로 `Restart Flow`와 완전히 같은 Flow packet과 arena reset을 사용했다.
이 one-slot alias가 두 기능을 같은 것으로 보이게 만든 원인이므로 다시 만들지 않는다.

| Tool 명령 | wire/runtime | reset 범위 | 재생 범위 |
|---|---|---|---|
| `Play Selected Pattern (Keep Arena)` | `PLAY_PATTERN_ID` | Valtan boss-only reset. 현재 wall/floor/prop/collision/Nav를 유지하고 교체되는 boss-source combat object만 취소하며 player-source object는 유지 | 선택한 Pattern 하나를 첫 Stage부터 재생. saved Flow, Next, Wait는 소비하지 않음 |
| `Restart Active Pattern (Keep Arena)` | `RESTART_PATTERN_ID` exact predecessor CAS | 같은 boss-only reset. 현재 arena를 유지하고 교체되는 boss-source combat object만 취소 | 이 Tool이 소유한 exact ACTIVE/COMPLETED Pattern occurrence 하나를 첫 Stage부터 교체 재생 |
| `Restart Saved Flow (Fresh Arena)` | `C2S_DEBUG_VALTAN_PATTERN_FLOW_START` | world destruction과 encounter prop을 포함한 authoritative arena reset | disk의 전체 saved `scriptedSequence`를 Pattern 01부터 시작하고 saved order/Next/Wait를 끝까지 소비 |

따라서 single Pattern 버튼에 `Fresh Arena`를 쓰거나, Flow 버튼을 `Pattern Restart`라고 부르지 않는다.
Server 회귀에서는 Pattern ID branch가 `Reset_ValtanBossOnlyAuditionState`만 호출하고
`Reset_ValtanAuditionState`를 호출하지 않는지, Flow start branch는 destruction/prop preflight 뒤
`Reset_ValtanAuditionState`를 호출하는지를 함께 고정한다.

목록/Save/Restart/Arena 실행을 바꿨다면 새 Debug EXE에서 사용자가 변경한 경로를 확인한다.
저장한 animation occurrence가 해당 패턴에 반영되는지, 기존 정상 항목이 유지되는지를 본다.
다른 도구의 목록 개수, 전체 native harness, 전 보스 publisher 결과를 매 수정의 필수 조건으로 요구하지 않는다.
목록 표시와 실제 재생은 구분해 보고하고, 실행하지 않은 동작은 확인했다고 기록하지 않는다.

### `serverMotion`의 takeoff stage는 이름이 아니라 ordered entry다

- `takeoffStartMs/takeoffEndMs`의 소유자는 `stageId == "TAKEOFF"`가 아니라 `entryActionId`와 일치하는
  첫 ordered stage다. `travelStageId`는 그보다 뒤의 고유 stable stage여야 한다.
- `VALTAN_SIX_PIZZA_106`의 첫 stage는 정본 `STEP_01`이다. validator를 통과시키려고 이를 `TAKEOFF`로
  개명하면 Effect, Camera, Product occurrence join을 함께 깨뜨린다.
- 한 pattern의 strict join 실패로 전체 All Effects inventory가 rollback되는 것은 정상 fail-close다. partial
  inventory나 legacy fallback으로 숨기지 말고 오류에 `patternId`와 실제 stage/action identity를 남긴다.

### Map Effect catalog 등록과 entry-required Product 준비는 다른 gate다

- catalog 행, authored Effect 파일, `.mapeffects.json` world row가 모두 있어도 portable `sourceRecipe` codec
  admission 또는 prepared Product commit이 실패할 수 있다. catalog 재등록만 반복하지 않는다.
- entry-required Map Effect에 실패도 `settled`로 보는 optional prewarm 정책을 적용하면 Level activation 뒤
  `Map Effect world target is absent...`라는 후속 증상만 남는다. 최초 `incremental prewarm failed`의 asset ID와
  codec 오류를 Level 진입 실패 원인으로 보존한다.
- 일반 Particle source recipe는 Required 1개, Lifetime 1개 이상, Spawn 1개를 요구한다.
  `particlemodulecolorscaleoverlife`는 color와 alpha distribution을 모두 가져야 한다. validator 약화나
  `sourceRecipe.enabled=false`로 입장을 통과시키지 않는다.

### 선언/정의가 일치하는 `LNK2019`는 실제 provider obj를 확인한다

1. 같은 working tree의 다른 MSBuild, CL, FXC, linker와 publisher를 먼저 멈춘다.
2. 선택한 `Configuration|Platform`의 evaluated `IntDir/OutDir`를 확인한다.
3. 참조하는 consumer obj가 아니라 provider obj에 `dumpbin /symbols`로 정의 심볼이 있는지 검사한다.
4. 심볼이 없으면 해당 translation unit만 강제 재컴파일한 뒤 최종 link를 한 번 수행한다. broad output 삭제나
   두 번째 전체 빌드를 겹치면 `.tlog` 잠금과 서로 다른 시점의 obj 혼합을 만든다.
5. `LNK1104`, `MSB3021`, `MSB3027`이 EXE/DLL을 가리키면 실행 중 출력물 잠금이다. compile 성공과 link
   차단을 분리하고, 현재 실행 중인 EXE는 새 obj가 반영되지 않은 이전 바이너리라고 보고한다.

### publisher의 `exit code 1`은 최초 오류가 아니다

- MSBuild가 출력한 마지막 `명령이 종료되었습니다(코드: 1)`은 wrapper 결과다. 그 앞의 첫 terminating
  error에서 domain, phase, stable ID와 path를 확보한다.
- `Publish-GameplayBalance -Mode Publish`는 destination 교체 전에 Valtan strict validation도 실행한다.
  먼저 `-Mode Validate`로 source/schema/join 실패를 분리하고 통과한 뒤에만 destination lock, promotion,
  rollback을 조사한다.
- Server contract-test의 기대값 실패는 pre-build publisher 실패와 별도 단계다. 생성 bootstrap 직접 수정,
  validation skip, 실행 중 Server/Client 위에 publisher/build 반복 실행으로 숨기지 않는다.

### All Effects의 `Delete Effect`는 소유권에 따라 의미가 다르다

- `[PRODUCT]` 행 삭제는 선택한 Pattern의 exact cue 연결만 split `Valtan.presentation.json`에서 제거하고
  candidate source CAS 뒤 Product를 투영한 다음 단일 `Validate` postcondition을 실행한다. 이전 Product parity를
  source Save 전에 요구하면 새 source와 이전 Product의 정상 drift를 실패로 오인한다. 공유 `EffectCatalog` 행과 authored Effect 파일, 다른 Pattern
  연결은 보존한다.
- `DRAFT_ATTACHED`만 sidecar row와 deterministic `Effects/Authored/<effectId>.effect.json` 파일을 함께
  삭제할 수 있다. Product catalog/cue 참조가 있으면 파일 삭제를 거부한다.
- Draft 삭제는 sidecar CAS를 먼저 commit하고 파일을 같은 handle에서 canonical compare-delete한다. 파일
  삭제가 실패하면 sidecar를 CAS rollback한다. unsaved 편집이 있거나 선택 이후 cue/baseline이 바뀌면 삭제하지 않는다.
- generated `Valtan.patterneffectcues.json`을 직접 편집하지 않는다. Product 연결 변경의 정본은 split
  presentation이고 Server 재생 판정은 publish 후 Server 재시작·Arena 재진입 뒤에 한다.

#### 삭제 직전에는 캐시가 아니라 정본을 다시 잠그고 읽는다

- `CEffectCatalog::Find()`나 현재 화면의 Pattern tree는 표시용 snapshot이다. 사용자가 확인 modal을 보는 동안
  다른 publisher가 Effect를 Product에 등록할 수 있으므로, 이 캐시만 보고 authored 파일을 삭제하면 안 된다.
- Draft 생성·삭제의 destructive preflight는 Effect catalog, split gameplay/presentation, Encounter/rotation,
  animation binding/cue/alias/stage-Effect, BossCatalog/combat-object까지 `CValtanPatternTree`가 소비하는 complete
  source read set을 read handle로 열어 concurrent write/delete를 막은 상태에서 catalog와 Product graph를 새로
  parse한다. 어느 하나라도 parse/lock에 실패하면 파일을 보존하고 Refresh를 요구한다.
- modal을 열 때 복사한 `kind + patternId + effectAssetId + cueIds + alias`와 확인 순간의 선택이 하나라도 다르면
  삭제하지 않는다. render-frame vector pointer가 아니라 stable identity만 확인 대상으로 보존한다.

#### publisher를 소유한 child process는 timeout으로 죽이지 않는다

- Product unlink는 source commit 뒤 원자적 Product projection + `Validate` postcondition과 실패 시 source/Product rollback을
  수행한다. managed cue scale-policy migration 표는 현재 cue의 허용 정책 ledger이며 live cue 전체 개수를
  고정하지 않는다. sealed legacy Effect cue도 전역 배열 ordinal이 아니라 stable `bindingId`로 검증한다. 이 child를 180초
  timeout에서 `TerminateProcess`하면 PowerShell의 catch/finally가 실행되지 않아 source/Product가 반쪽 상태로
  남을 수 있다.
- Effect Tool은 unlink process를 비동기로 시작하고 매 frame exit만 poll한다. 180초는 경고 기준일 뿐 종료 기준이
  아니다. 작업 중에는 All Effects 편집을 잠그고, Client가 닫혀도 process handle만 닫아 child가 commit 또는
  rollback을 끝내게 한다.
- exit 0에서만 unlink 성공으로 표시한다. nonzero나 process observation 실패에서는 disk를 다시 읽되 보존 여부를
  추측하지 않고 최초 publisher 오류를 확인하게 한다. child가 아직 실행 중일 수 있는 observation failure에서는
  All Effects를 다시 열어 두지 않는다. 잠금을 유지해 두 번째 publisher transaction이 겹치는 것을 막는다.

#### atomic replace backup은 post-commit 검증 뒤에만 지운다

- `File.Replace`가 성공한 직후 실행되는 byte verification도 실패할 수 있다. replace 완료 flag를 verification 뒤에
  세우거나 backup을 `finally`에서 무조건 지우면 이미 바뀐 source를 baseline으로 복구할 수 없다.
- replace 직후 commit flag와 recovery backup path를 먼저 기록하고, replacement bytes를 다시 확인한 뒤에만 backup을
  삭제한다. post-replace 실패는 보존한 backup/CAS rollback으로 baseline을 복원하며, 복구까지 실패하면 backup의
  정확한 경로를 오류에 남긴다.

### Model View target 교체와 Create auto-open은 저장 transaction과 분리한다

- Character Select의 Model View가 같은 `Valtan` asset을 다시 publish해도 target generation은 바뀔 수 있다.
  asset 이름과 포인터만 비교하면 synchronized sequence가 generic update에서 비워진 뒤 다시 stage되지 않는다.
- Pattern Draft는 매 frame generic synchronized update보다 먼저 generation mismatch를 확인하고, 동일 Pattern의
  ordered timeline을 새 generation에 다시 stage한다. target 교체를 문서 unload나 빈 sequence의 정상 종료로
  처리하지 않는다.
- `Create Effect`의 durable 성공 경계는 authored Effect 파일과 `DRAFT_ATTACHED` sidecar가 모두 CAS commit된
  시점이다. 그 뒤 Model View 준비나 auto-open이 실패해도 두 파일을 rollback하지 않는다. `files remain
  committed`와 preview 실패 원인을 분리해 보고하고, 사용자가 `Open Editor`로 재시도할 수 있게 한다.
- 실행 중 `Client.exe`는 이 새 source를 반영하지 않은 이전 바이너리다. 사용자의 tuning 세션을 강제 종료하거나
  그 위에 link하지 말고, 종료 신호 뒤 한 번 재빌드한 새 EXE에서 target replacement와 Delete UI를 확인한다.

### 오래된 worktree의 PatternTree 전체 파일로 All Effects를 덮어쓰지 않는다

- `피자 패턴 바닥 이펙트 가이드` 작업에서 확인된 회귀처럼, 최신 strict join 위에 오래된 worktree 파일을
  통째로 덮으면 개별 Effect 문제가 아니라 Valtan tree 전체 admission 실패로 나타난다.
- `Boss Tool`이나 Pattern Flow를 추가할 때 과거 계획서·stash·별도 worktree의
  `ValtanPatternTree.h/.cpp` 또는 `Effect_Tool.cpp` All Effects 본문을 전체 복사하지 않는다. 현재 working copy의
  strict join은 누적 계약이며 `partDamagePolicy`, `counterProxy`와 transactional previous-tree 보존 중 하나라도
  사라지면 `F1 -> Effect Tool -> All Effects -> Valtan` tree 전체가 fail-close로 비어 보일 수 있다.
- 작업 전후 no-touch diff를 비교하고 다음 focused contract를 함께 실행한다.
  `python -m unittest Tools.ValtanPipeline.test_valtan_pattern_tree_contract Tools.EffectPipeline.test_effect_tool_valtan_all_effects_contract`
- reload 실패는 기존 admitted tree를 지우지 말고 exact parse/join 오류를 표시한다. 자동 검증 뒤에도 사용자가 새
  Client에서 All Effects의 Valtan 28개 Pattern과 Stage tree가 실제로 열리는지 확인해야 visual PASS다.

### gameplay source가 없는 encounter에 공용 Dataset/runtime부터 만들지 않는다

- 모델, animation chain, 추출 asset 폴더가 존재하는 것과 Server-authoritative encounter source/Product가
  존재하는 것은 다른 계약이다. 현재 Valtan은 기존 canonical source/Product 경로와
  Tool/Server의 직접 reader를 소비하며, 별도 `ENCOUNTER_DATASET` registry나 공용
  `BossPatternGraphRuntime`을 완료 계약으로 두지 않는다.
- `KAKULSAYDON`은 public logical ID이고
  `KoukuSaton`은 실제 animation/resource 저장 alias이므로 spelling을 통일한다는 이유로 raw asset을 바꾸거나,
  존재하지 않는 Kakul gameplay source/Product 경로를 catalog에 추가하지 않는다.
- 향후 공용 Dataset/runtime을 도입하려면 새 encounter source/Product를 먼저 publish한 뒤
  실제 두 소비자 이상을 한 변경 단위에 이전한다. invalid absolute/`..` path, identity mismatch,
  missing/duplicate stage, action+pattern dual branch target, follow-up depth 32 경계를 같은 변경 단위의 native
  contract와 structural oracle로 닫는다. registry/helper만 먼저 만들면 Tool 목록은 생겨도 Save/Restart가 별도 정본을
  참조하는 두 번째 경로가 다시 만들어진다.

### Kouku Scene Profile의 blendMs에 Effect 페이드 길이 검증을 적용하지 않는다

- Composition의 Scene Profile `blendMs` 저장 범위는 `0..600000`이고 해당 box의 `durationMs`와 독립적이다.
  기존 Product는 이 예약 메타데이터를 `SCENE_PROFILE.fadeInMs`로 투영한다. Scene Profile은 현재
  profile의 exposure, bloom, map-light multiplier를 즉시 적용하며 blendMs는 어두움의 강도가 아니다.
- 진짜 세이튼 찾기의 `durationMs=24127`, `blendMs=600000` 저장본에 일반 Effect의
  `fadeInMs <= durationMs` 검사를 적용하면 Client Product 전체 staging이 실패한다. 그 결과 무력화의
  정상 Effect 연결까지 함께 재생되지 않는다. Effect 파일이나 occurrence가 삭제된 현상과 구분한다.
- Client reader는 `SCENE_PROFILE`의 blend 메타데이터만 canonical과 같은 `0..600000` 범위를 허용한다.
  일반 Effect는 occurrence 길이와 fade-in/out 합계 검사를 유지한다. 손상된 Scene Profile row는
  해당 pattern/occurrence와 오류를 표시하고 그 row만 격리해 정상 Effect를 계속 로드한다.
- 검증에는 현재 저장본의 600000 투영 보존, 600001 거부, 일반 Effect의 duration 초과 fade 거부를 함께
  확인한다. Scene Profile의 어두움은 multiplier로 조절하며 큰 blendMs를 삭제하거나 임의 축소해 우회하지 않는다.

### 독립 Effect 편집에 전체 보스 graph admission과 live preview를 요구하지 않는다

- 새 Workbench의 Tree/목록은 metadata만 읽고 손상 항목을 따로 표시한다. 선택 문서 parse, 선택 closure의 Play stage, CPU Save와 domain Product publish의 검증 경계를 구분한다. 목록·Save 성공을 Valtan 전체 Reload_BossValtan으로 가로막지 않는다.
- 새 문서 Create는 clock·Solo/Mute·모델 참고·anchor history를 초기화한다. 공용 Resource UI를 재사용할 때 기존 Effect Tool의 type/slot/bindings를 저장·복구해야 미저장 슬롯을 바꾸지 않는다.
- 기존 V2 leaf editor도 Load 때 표시 이름과 실제 source bytes를 보관한다. 새 Workbench에서 저장한 leaf를 예전 preview로 Save하면 이름을 지우거나 최신 파일을 덮을 수 있으므로 파일 기준본이 바뀌면 거부한다. 이름이 같은 것과 bytes가 같은 것은 별개다.
- Revert는 재로드를 임시 session에서 성공한 뒤 교체한다. saved leaf가 삭제되거나 손상됐다는 이유로 현재 dirty draft부터 Reset하지 않는다. view에서 world anchor를 capture한 경우도 Dirty로 표시한다.
- 모델 참고 actor는 root motion이 꺼진 고정 root이므로 첫 비영 시점 Play를 위해 알려진 0초 root를 기록할 수 있다. 실제 Product에서 과거 Server 위치를 같은 방식으로 만들어 넣으면 안 된다. Product는 실제로 기록한 root history를 사용하고 누락·불연속 구간은 해당 Effect 실패로 격리한다.
- 외부 Sequencer clock의 group은 일반 Runtime Advance에서 중복 증가하지 않는다. 1ms emitter와 1.7초 입자 수명은 별개다. Deactivate 시 고정 step 잔여 구간의 마지막 방출을 처리하고, timeline 끝의 잔여 수명 만료도 요청한 age로 반영한다. group/box 끝을 늘려 방출 횟수를 임의로 늘리지 않는다.

### 패턴 Camera 복귀는 저장한 플레이어 시작 좌표로 돌아가지 않는다

- Camera shot은 목표 eye/lookAt/FOV와 진입·유지·복귀 시간을 저장한다. 시작 pose는 재생 시 취득하고 복귀 목표는 이동 중인 player follow pose를 매 프레임 계산한다. 복귀 끝에서 예전 override 이전 pose를 복원하면 마지막 프레임에 튄다.
- Authoring Preview가 저장한 shot을 바로 읽는 것과 Complete Play가 published shot을 읽는 것을 구분한다. Map publish 후 새 run은 runtime shot snapshot을 갱신해야 한다. Camera overlap 검사는 visible box뿐 아니라 복귀 tail을 포함한다.
- 진짜 세이튼과 같은 Spot Light를 가짜에 적용할 때 Server owner boss ID와 같은 archetype으로 대상을 제한한다. 이미 같은 asset을 직접 재생 중인 가짜에 중복 light를 만들지 않으며 despawn/row 종료 때 follower handle을 정리한다.

### Sequencer 카메라 조회에서 실패한 문서를 매 프레임 다시 파싱하지 않는다

- 카메라 shot/keyframe 수가 늘 때 byte 한도뿐 아니라 JSON value 한도도 실제 전체 문서로 검사한다.
  publisher가 받은 문서를 Client만 낮은 value 한도로 거부하면 컷신이 follow 시점에 머물 수 있다.
- Timeline의 길이·row·복귀 tail 조회는 같은 카메라를 여러 번 찾는다. 실패한 최초 저작 로드는 Level이
  기억하고, `Composition Camera → Reload Cameras`로만 재시도한다. 재로드 실패는 이전 shot과 baseline을 보존한다.
- 입력 파일의 실제 parser 성공, 반복 Ensure에서 read/parse 추가 호출이 없는지, 사용자 FPS 측정은 구분한다.
  재현·검증은 [Sequencer 카메라 로드 결과](09-12/2026-09-12_SEQUENCER_CAMERA_LOAD_PERFORMANCE_RESULT.md)를 따른다.

### ImGui root 위젯 ID에 빈 draft의 표시 이름을 그대로 쓰지 않는다

- Effect Composition Workbench를 처음 열면 group name과 stable ID가 모두 비어 있다. 이 값으로 `Selectable("")`를 그리면 Timeline child window의 root ID와 충돌해 `Cannot have an empty ID at the root of a window` assertion이 발생한다.
- 빈 draft는 생성/열기 안내를 그리고 반환한다. 정상 부모 row에는 `###EffectCompositionGroup`처럼 표시 이름과 독립된 위젯 ID를 사용한다. 손상·삭제된 Pattern을 참조하는 Character anchor도 표시 이름이 비어 있을 수 있으므로 member ID scope와 `###AnchorMember`를 사용한다.
- 컴파일과 문서 parse만으로 첫 창 렌더의 ImGui assertion을 검증했다고 기록하지 않는다. Client 창 재열기 확인은 사용자가 직접 한다.

### Timeline lane과 per-lane cache의 크기를 함께 유지한다

- 새 lane을 render order에만 추가하면 고정 크기의 cache가 남아 첫 timeline 렌더에서 범위를 벗어난다. `TIMELINE_LANE::COUNT`로 cache 크기를 정하고 표시 순서의 실제 항목 수도 static_assert로 대조한다. cache는 표시 ordinal 대신 lane 값으로 조회하며 `Pack_TimelineSubrows`도 같은 크기를 소비한다.
- Valtan Workbench의 WORLD index7 / cache7 결함은 빈 draft 위젯 ID 오류와 별개다. assertion을 끄거나 WORLD를 생략해 숨기지 않는다. 실제 packing 함수의 8-lane·겹침·빈 lane 검사와 focused compile은 사용자 창 재열기 검증과 구분한다.

### Effect panel 재사용 시 저장·미리보기 owner를 구분한다

- Parent/tree 저장과 Effect body 저장은 다른 owner다. native V2 leaf Open을 항상 group으로 감싸 Save하면 뜻하지 않은 group 원본이 생긴다. native leaf 저장은 같은 ID/파일을 유지하고 group 확장은 명시적인 생성 명령으로만 한다.
- 현재 draft의 preview 검증이 실패했을 때 saved 문서로 fallback하면 잘못된 편집 내용 대신 이전 Effect가 재생된다. 현재 draft가 소유한 key의 실패는 그대로 표시하고 snapshot 교체를 취소한다.
- Play All/Family/Element는 preview이고 Append만 저장 sequence occurrence를 만든다. Preview 버튼 처리에 Append를 재사용하면 Play할 때마다 저장 행이 쌓인다.
- Patterns by Gate의 Create Parent/Bundle은 메모리 변경이다. 전체 Composition Save 이전에는 EXE 종료 후 보존을 보장하지 않으며 트리 옆에 Saved/Unsaved와 Save를 표시한다.
- Parent의 runtime 전개와 편집 진입은 따로 확인한다. backing timeline이 없는 기존 Parent도 상단 Append Pattern에서 첫 자식과 timeline을 하나의 candidate로 생성해야 하며 실패한 시도는 folder·ordinal·draft를 보존한다. Details에 함수가 연결됐다는 이유만으로 상단 버튼이나 Parent 선택 후 sequencer가 연결됐다고 기록하지 않는다.
- ImGui Rename의 한영 입력은 공통 Win32 IME context를 유지해야 한다. caret callback에서 WantVisible에 따라 context를 분리하지 않는다. 일반 InputText는 확정 WM_CHAR만 표시하므로 OS 조합창이 필요하고, 채팅/닉네임의 직접 그리는 조합 문자열과 구분한다. backend가 DefWindowProc를 호출했다면 처리 완료를 반환해 Client와 분리 viewport에서 기본 처리를 반복하지 않는다.
- World Object model/texture는 기존 Effect domain scan 밖에 있을 수 있다. 저장 Object resource에서 확인한 상대 ID와 file kind를 동일 resource binder에 전달해야 목록만 보이고 Bind가 거부되는 상태를 피할 수 있다.


### Product 빌드와 같은 import library를 읽는 probe 링크를 겹치지 않는다

Windows에서 Engine.lib를 쓰는 제품 링크와 같은 파일을 입력으로 여는 별도 probe 링크가
겹치면 LNK1114/오류5와 같은 공유·접근 실패가 발생할 수 있다. 사용자에게 소스 동결과
빌드 인계를 했으면 Product 빌드뿐 아니라 같은 import library를 읽는 out 검사 compile/link도
중단한다. 원본 로그에 잠금 소유자 정보가 없으면 동시 실행만으로 특정 프로세스를 확정하지 않는다.

Engine 링크 실패는0바이트 Engine.dll을 남길 수 있고, DLL 존재만 검사하는 Client 배포는 그
파일을 복사할 수 있다. EXE 링크 성공만으로 실행 준비 완료라고 판단하지 말고 Engine 원본과
Client 배포 DLL의 유효 크기/PE 형식·일치 여부도 확인한다. 실패 출력은 크기와 정확한 workspace
경로를 확인한 뒤 필요한 파일만 재생성하며, 사용자 Client를 자동 실행하지 않는다.


### 쿠크 컷신의 모델·재질·조명·곡선 연결

- 부모 더미에 부착된 배경은 부모 자신의 Matinee group으로 샘플한다. `world_pose(..., None, parent, t)`는 Move 트랙을 누락시켜 움직이는 카드·받침을 저장 자세에 고정할 수 있다. 모델130개를 WORLD14묶음으로 연결한 경우 모델 수와 타임라인 항목 수를 구별하고, 완성 시퀀스에 추가할 때 기존 카메라·발생·다른 패턴을 보존한다. [2관문 배경 선택 반영 G16](09-13/2026-09-13_KOUKU_G12_CUTSCENES_CAMERA_MAP_RESULT.md).
- 보스 무기에서 정적 World Object를 만들면 같은 mesh/slot/D/N/S여도 새 modelAssetId에는 원래 catalog의 native 재질이 자동 적용되지 않을 수 있다. 실제 원본 MIC가 같은지 확인해 `materialSourceModelAssetId`를 전달하고 IBL/BRDF까지 검사한다. 재생성 때 이 참조를 버리지 않는다. geometry의 반전 bake는 환경 반사 복구가 아니다. 상세 절차는 복원 V2의 오브젝트 공통 절차를 따른다.
- 같은 mesh와 diffuse가 있어도 움직이는 BG8 모델은 static shader와 다른 skinned shader를 쓴다. 공유 MapMaterialSurface 평가와 모든 바인딩을 실제 skinned draw까지 연결하고, 정적 RNM/static shadow를 움직이는 모델에 복사하지 않는다. UNBAKED receiver는 기존 baked bit로 정적 맵 중복 조명을 제외한다.
- Matinee InterpGroup만 세면 부모에 부착된 맵 소품을 빠뜨린다. source actor의 base/basebonename/relative pose와 component material override까지 조사한다. source transparent override를 범용 diffuse 슬롯으로 표시하지 않는다.
- native Move/Camera를 일정 간격으로만 줄이면 급격한 이동을 놓칠 수 있다. 원본 곡선 대비 위치·회전·FOV 오차를 측정하고 저장 key 상한을 넘으면 연속 resource로 분할한다. Director 컷 수와 저장 resource 수는 다를 수 있다.
- 인접 Effect 구간은 start/end를 runtime float32로 변환한 뒤 duration=end-start로 만든다. start와 double 차이 duration을 따로 변환하면 경계에서 두 광원이 겹칠 수 있다. RGB Hermite는 기존 cubic distribution으로 보존한다.
- 생성형 popup book을 쓰는 Preview는 이전 Deploy7도 보이는지 확인한다. borrowed state와 applied state를 함께 기록하고 Stop에서 현재 상태가 여전히 적용값일 때만 복구한다.
- 정상 맵 애니메이션을 합칠 때 template의 키 길이와 occurrence의 표시 수명을 구분한다. MAP은 원래 마지막 키를 유지하고 OBJECT_RESOURCE는 명시적 HOLD instance를 사용한다. source Matinee의 다른 책 clip·배치·배우를 일부만 섞지 않는다.
- 연출용 MapLight 사본은 기존 provider 문서를 stage/validate한 뒤 소유하고, 실제 WORLD cue 수명으로 활성화한다. WORLD Seek/Stop 이후 같은 프레임에 provider 하나만 제출한다. Stop에서 Renderer에 이미 등록된 provider를 Clear하지 않으며, 같은 포인터의 authoring 문서가 변경될 때도 기존 사본을 무효화한다.
- RenderingProfiles의 JSON number는 C++ reader와 같은 float32 변환·범위 순서로 검사한다. `0.100000001` 같은 9자리 저장값을 double로 확장한 float32 하한과 직접 비교하지 않는다. 벡터 reader의 별도 원문 double 범위와 near/far의 float32 비교는 유지한다.


### 카드 variant의 cooked slot 이름과 native texture identity를 구분한다

- `MN_RHOC_00-1.wmodel`의 slot 이름은 일반 카드와 같은 `mn_rhoc_00_mi`지만 원본 조커 MIC와 D/N/S는 `mn_rhoc_00-1`이다. BossCatalog의 모델별 override를 만들 때 slot 이름만으로 일반 카드의 sourceMaterial·texture를 복사하지 않는다. 실제 modelAssetId, 원본 MIC와 설치 WModel texture를 함께 대조한다.
- program 26은 native expression 1의 diffuse·alpha를 직접 샘플한다. 일반 diffuse override로 바꿔도 잘못된 native slot은 남는다. 조커는 cooked materialName·family·36개 parameter를 유지하고 기존 variant texture 0=N(linear), 1=D(srgb), 2=S(srgb)를 연결한다. JSON·실제 입력 검증과 사용자 화면 확인을 구분하며 [조커 결과](09-12/2026-09-12_KOUKU_JOKER_NATIVE_TEXTURE_RESULT.md)를 따른다.

### v15 trail/ribbon만 있는 projection에 LocalDecal을 강제하지 않는다

- document-owned v15 `ADAPTER_PACKET_V1`은 정상 supplemental-only projection일 수 있다. `Get_AdmittedRows()`의 LocalDecal 수가 0이어도 유효한 `Get_AdmittedSupplementalElements()`가 있으면 준비를 허용하고 실제 typed adapter 수 일치는 계속 검사한다.
- 내려찍기·거미카운터처럼 v15 전체 문서만 미표시이고 v13 연기는 표시되면 크기를 임의 보정하기 전에 catalog/projection → 실제 GPU resource prepare → renderer attach/clone 실패를 확인한다. CPU playback과 단일 shader draw만 통과해도 이 경계에서 전체 effect가 차단될 수 있다.

### 쿠크 독립 Effect와 원본 폭죽 event의 소유자를 구분한다

- 독립 Effect Resource는 원본 `sourceModelPreview`의 actor·clip·Source In 시계를 사용한다. 빈 synthetic Pattern의 Animation만 조회하면 첫 fixed-step에서 source anchor가 실패한다. 실제 모델과 bone/clip을 함께 준비하고 마지막 pose를 입자 tail까지 유지한다. 독립 Preview의 정지 pose 정책을 Product history 누락에 적용하지 않는다. [검증 결과](09-12/2026-09-12_KOUKU_RESOURCE_SOURCE_PREVIEW_RESULT.md)를 따른다.
- root/camera-only Effect의 Play All에 저장된 boss pattern을 요구하지 않는다. 실제 source bone이 필요한 draft만 CNpc/CModel을 준비하고, 독립 재생은 실제 플레이어 root를 임시 캡처한다. 이 과정에서 저장된 sequence의 모델·anchor·dirty 상태나 occurrence를 바꾸지 않는다.
- Action Workbench와 Sequencer Benchmark의 MAP Effect는 고정 월드 위치다. WORLD object 참조나 follow/bone과 혼합하지 않는다. V1 전체 문서도 기존 Append 경로로 같은 anchor를 소비한다.
- `EPET_Death` 폭죽은 로켓 수명 종료 위치·속도로 기존 bounded event queue에 넣는다. 생성0-age와 마지막 부분 step을 수명 끝까지 적분하고 이미 소비한 spawn을 다시 보내지 않는다. 숨은 `ERM_None`는 같은 문서의 location/event 소비자와 원본 출처가 있을 때만 허용한다.
- launch 종료와 후속 폭발 tail은 다르다. rate0·burst없음인 event receiver의 수신창만 원본 event chain으로 계산하고, 입자 수명이나 원본 발사 횟수를 늘려 재생 시간을 확보하지 않는다. reserve budget을 실제 동시 생존 상한으로 검사한다.
- 원본 Action에 ParticleSystem 이름이 없어도 SkillEffect가 Projectile을 거쳐 호출할 수 있다. 사진과 비슷한 Ray/Ready 이름만으로 격자를 단정하지 말고 fixed-area placement·signed Rotator·Timer와 CEF 파라미터를 함께 추적한다.
- native ID를 추가할 때 descriptor/C++ 상한과 Mesh/Particle/Decal/Trail HLSL dispatch 범위를 함께 검사한다. 새 ID가 분기를 지나지 못하면 컴파일에 성공해도 RGB가 0이다.
- 원본 PS의 material 상수만으로 엔진 prefix가 채워지지 않는다. 외부 opacity/color와 MacroUV를 실제 소비 lane까지 연결한다. MacroUV 중심은 현재 ParticleSystem occurrence가 소유하고 world-space 입자의 고정 birth 위치와 다르다. 원본 world radius에는 occurrence scale을 다시 곱하지 않는다.
- 독립 사각형 shader draw의 RGB 0은 시선각/Fresnel/dissolve mask와 Dynamic 입력을 구분해서 진단한다. finite/alpha 성공을 실제 표시 성공으로 대신 기록하거나 임의 alpha 보정으로 덮지 않는다.

### World 표시는 opacity와 재생창을 원본 의미로 구분한다

- `par_b_picking_01`의 mesh particle alpha는 -1~+1 UV 이동 입력이다. 음수 alpha를 일반 opacity로 clamp/cull하면 피킹 표시의 앞부분이 사라진다. 원본 PS의 실제 소비 lane을 먼저 확인한다.
- Required emitterLoops 생략은 UE3 기본 0인 반복일 수 있다. `par_i_movetrack_01`의 9개 반복 emitter와 3개 초기 pulse를 모두 loops1로 바꾸지 않는다. bounded document tail과 Tool의 반복창은 구분하며, 반복 중 일반 Seek의 pause 부작용이나 자원 재준비 때문에 멈추지 않게 한다.
- mesh의 `PSA_TypeSpecific + MeshFaceCameraWithLockedAxis + EPAL_Rotate_Z`는 sprite billboard와 별도 source mode다. 원본 TypeData 회전·preScale·mesh basis를 실측하고 native instance/scalar 양쪽에서 같은 camera-facing transform을 사용한다.
- 이름·정지 이미지·비슷한 색만으로 특정 보스 관문의 실제 variant 선택을 확정하지 않는다. source 구조 대응과 level/script 직접 참조 여부, 사용자 화면 판정을 나눠 기록한다.

### Profiler 숫자의 계측 분모를 유지한다

- GPU timestamp가 Update 전~Present 뒤를 감싸면 GPU 실행 사이 CPU 공급 공백을 포함할 수 있다. CPU와 GPU frame ms가 같다는 사실만으로 GPU 연산 포화를 단정하지 않는다. pass timestamp·Present CPU·copy 횟수를 별도로 측정한다.
- `PSInvocations`는 셰이더 호출 수이며 셰이더 내부 ALU 연산 수가 아니다. 전체값을 특정 패스 비용으로 해석하지 않는다. GPU scope의 `pipelineValid`가 true인 PS/VS 통계만 사용하고 미선택·미지원 값을 0회 실행으로 해석하지 않는다. 인스턴싱으로 draw 제출이 줄어도 겹친 픽셀 수가 자동으로 줄지는 않는다.
- Long Operations는 임계값 이상인 완료 호출만 표시한다. main만 보인다는 사실로 전체 프로세스에 worker가 없다고 단정하지 않으며 부모·자식 시간을 합산하지 않는다. worker 계산 합계와 main join 대기는 서로 다른 지표다.
- `Particle.Simulate`의 spawn/update 동명 scope, 부모·자식 inclusive 시간, 여러 playback의 fixed-step 호출 합계를 구분한다. 고정 스텝 따라잡기와 동기 Save JSON의 프레임 간섭도 기록한다.
- JSON의 counter 키 존재는 실제 writer 존재를 뜻하지 않는다. 확장 전 renderSubmissions/texture 계열의 writer 없는 0값을 작업량 0의 증거로 쓰지 않는다. 새 renderSubmissions는 실제 enqueue에 연결되며 미연결 texture counter는 N/A로 표시한다. occurrence별 SceneColor refresh도 실제 copy 함수 안에서 계측해야 초기 snapshot만 보이는 누락을 막는다.

### ImGui backend에 Engine 헤더를 넣을 때 Debug new 매크로를 격리한다

- `Engine_Defines.h`는 `_DEBUG`에서 `new`를 CRT debug allocation 매크로로 바꾼다. 이를 ImGui backend에 그대로 유입하면 `IM_NEW`의 placement-new가 C2226 `ImNewWrapper` 오류로 깨진다. backend의 Engine 헤더 include 전후에 `push_macro("new")` / `pop_macro("new")`로 진입 시 상태를 복원한다. Engine 전체의 debug allocation 설정을 끄지 않는다.
- 비-Debug standalone compile 성공은 이 경계를 검증하지 않는다. 실제 Debug Engine compile과 SDK 복사 뒤 Client compile/link를 확인한다. 새 profiler enum이 Client에서 없다고 표시되면 `Engine/Public/Profiler.h`와 `EngineSDK/Inc/Profiler.h`의 일치 및 선행 Engine build 성공부터 확인한다.
- 비동기 저장의 완료 Poll을 ImGui 창 Render에만 두면 저장 중 창을 닫았을 때 joinable worker와 비활성 Save 상태가 남는다. 저장 owner의 Update에서 창 가시성과 무관하게 결과를 회수한다.

### 한 target의 긴 준비와 GPU readback은 분배 횟수만 제한해도 main을 멈춘다

- 프레임당 Effect target 한 개만 처리해도 target의 parse/decode/resource 준비가 13초면 그 프레임이 13초 멈춘다. 기존 EffectLoadPreparationJob의 worker stage/result/ACK를 runtime에서도 사용하고 main은 결과 commit을 담당한다. scope는 실제 stage와 ACK 대기를 분리한다.
- runtime job에서 Loading owner로 넘어갈 때 기존 worker의 협력 취소·drain을 확인한다. 실패 job은 terminal receipt와 원인 문자열을 보존하며 매 프레임 같은 target을 다시 시작하지 않는다.
- GPU timestamp interval에 CPU 명령 공급 공백이 포함될 수 있다. 전체 화면 PickPos readback처럼 Copy 뒤 즉시 Map하는 동기 경로는 CPU wait와 bytes를 따로 확인한다. 마우스 한 점을 얻으려고 viewport 전체를 복사하지 않는다. 현재 요청의 좌표/target/RowPitch/no-hit 계약을 유지한다.
- Profiler panel 자체의 raw sample 정렬·집계가 캡처를 교란할 수 있다. self-time 계산을 바꿀 때 실제 캡처의 thread별 nesting, zero duration, orphan와 frame window를 비교한다. 집계 함수 가속 배율을 게임 FPS 배율로 보고하지 않는다.
- Save JSON은 최근 최대 1200프레임의 독립 파일을 추가한다. 완료되지 않은 긴 scope는 종료 전까지 집계에 없고, 이미 기록된 frame history도 무제한은 아니다. 저장 이름이 같아도 기존 파일을 덮어쓰지 않는다.
- pause한 Profiler 패널도 아직 pending인 GPU 결과가 회수될 때까지는 갱신해야 한다. 이후 변경 없는 history의 반복 집계를 중단하며 Capture/Reset/frame-window 변경은 즉시 반영한다.

### 공유 mesh의 포즈와 파티클 병렬 작업 경계

- `CModel` clone은 mesh geometry를 공유하고 bone pose는 각자 소유한다. skin palette cache도 모델에 두고 combined pose가 갱신될 때 무효화한다. WModel의 전체 skeleton palette와 Assimp의 mesh별 bone subset/offset을 같은 것으로 취급하지 않는다. frame number만으로 cache를 고정하면 한 프레임 안의 secondary motion·명시적 pose 변경을 놓친다.
- 파티클 병렬화는 각 emitter의 particle/RNG 상태를 한 작업이 소유하고, 불변 준비 데이터만 공유한다. emitter 간 spawn provider와 portable event/death-event queue는 기존 순서를 유지한다. worker 결과를 모두 회수하기 전에 다음 fixed step, event, trail, frame rebuild를 진행하지 않는다.
- worker 수 자체를 성능 개선으로 기록하지 않는다. Debug checked iterator의 경합, 작은 작업의 제출·join 비용을 실제 serial/parallel 동일 결과 비교로 확인하고, compiler 최적화 효과와 알고리즘·병렬화 효과를 분리한다.
- 모델 worker의 협력 취소는 진행 중인 한 binary decode를 즉시 중단하지 못할 수 있다. 정상 레벨 전환은 요청을 보존하고 프레임을 진행하며 준비·큰 자원 해제가 끝난 뒤 전환한다. main에서 service를 먼저 파괴해 bounded join timeout을 정상 전환에도 발생시키지 않는다. worker registry와 owner는 thread를 시작하기 전에 준비하고 immutable authoring/catalog 입력은 main에서 캡처한다.

### 소환 모델의 뒤틀림·정지와 여러 material section을 구분한다

- glTF→WModel의 bone matrix 일치만으로 PSA→glTF 변환이 맞다고 판정하지 않는다. root는 유지하고 mesh hierarchy의 child quaternion을 conjugate해야 하며, 실제 원본 PSA와 여러 frame의 pose를 대조한다. 기존 geometry/weights/skeleton이 정상이어도 animation rotation만 잘못될 수 있다.
- PSA와 mesh joint 순서가 다를 때 skin JOINTS나 IB를 재배열하지 않는다. 명시적인 unique-name remap으로 animation channel target만 연결하고 누락·중복 이름은 거부한다.
- 말의 section0~3은 한 골격의 재질 조각이다. section 하나의 local rotation만 바꾸면 조립이 깨진다. 원본 모델 basis와 한 동물의 공통 transform을 확인한다.
- ModelCue의 holdLastFrame=false는 반복 재생이 아니다. loop는 별도 입력으로 검증하고 animation time만 감싼다. cue 이동 시간을 fmod로 감싸면 매 회전마다 시작 위치로 되돌아간다.
- loop 시 뒤로 튀면 원본 root의 수평 displacement와 cue velocity를 따로 실측한다. 원본 수평 이동을 반복 초기화하는 문제를 quaternion 재변환이나 cue 방향 반전으로 가리지 않는다. 명시적인 ModelCue root-motion suppression은 기존 CModel을 재사용하고 source Y 점프와 원본 binary를 보존한다. optional 필드와 cache·rollback 계약은 [렌더링 복원 정본](렌더링이펙트복원V2.md)의 ModelCue 항목을 따른다.

### 제품 맵 배치의 LFS 병합 충돌과 parser 이유를 구분한다

- `Map: product load scope`는 진행 단계다. 실패 시 `Read_Placements`의 실제 parser 이유와 Area ID를 `CLoader::Get_ActiveStatus()`에 보존해 `Recover_FromFailure`의 세션 진단 JSON까지 전달한다. scope 결과에 배치가 없는 경우도 명시적인 이유를 남긴다.
- 실행 `.mapplacements`는 `LOSTARK_MAP_PLACEMENTS` 헤더의 게시 출력이어야 한다. LFS pointer나 conflict marker가 남은 파일의 header 거부를 resource 누락이나 GPU 문제로 오판하지 않는다. 같은 Area의 worldsequences도 확인하고, 통합한 `Data/Maps/Authoring` 정본으로 Area publisher와 Check를 수행한다. 생성물을 직접 편집하거나 parser를 완화하지 않으며 일반 C++ 빌드를 publisher로 간주하지 않는다.
- 충돌 제거·게시·컴파일 성공과 최종 Client의 arena 진입은 별도 증거다. 당시 원인과 게시 후 정상 상태는 [PR360 통합 결과](09-11/2026-09-11_PR360_WORLD_OBJECT_RESOURCE_MERGE_IMPLEMENTATION_RESULT.md)에 구분한다.

### 스킬 애니메이션만 동작하고 한 직업의 모든 Effect가 없으면 animevents 전체 로드를 확인한다

- `.animevents`의 헤더 총행수는 원본 참고 event와 제품 `effectref=asset` 행을 모두 포함한다. clip cue 병합·삭제 뒤 실제 행 수를 갱신하지 않으면 parser가 문서 전체를 거부한다. 개별 Effect JSON·catalog 존재만 확인하면 이 실패를 놓친다. 통합 도구에서 최종 행 수를 산출하고 실제 Product prewarm 및 설치 모델 clip/bone을 사용한 cue Load를 검사한다.
- Artist와 LanceMaster ALT V에서 같은 결함이 재발했다. Lance의 선언3139/실제3136을 맞춘 뒤43cue가 정상 admission됐으며, 이미 실패한 prepared 문서는 Client 재시작으로 다시 읽는다. 헤더 검사를 완화하거나 이 데이터 수정에 EXE/Server 재빌드를 요구하지 않는다. [상세 원인과 검증](09-11/2026-09-11_TIGER_HORSE_ANIMATION_AND_LANCEMASTER_ALTV_RESULT.md#g07-창술사-전체-이펙트-미출력의-실제-로더-회귀)을 따른다.

### Effect Tool에서 읽힌 큰 문서도 제품 준비 경로를 확인한다

- Lance ALT V의 20,049,144-byte full 문서는 Codec의 64MiB 한도에는 들어왔지만 Product Catalog의 별도 16MiB 한도에서 거부됐다. standalone Codec/Renderer Stage만으로 스킬 제품 준비를 검증하지 않는다. 실제 Catalog request와 `Stage_LoadingProductTarget`을 연결해 확인한다.
- 저작·제품 문서는 `CEffectDocumentCodec::MAXIMUM_DOCUMENT_BYTES`의 64MiB 상한과 bounded Load를 공유한다. catalog index의 16MiB 한도와 JSON depth/value/identity 검사는 별도 계약이다. 임의 minify로 현재 파일만 통과시키면 F1 Save 후 재발할 수 있다. [실제 실패와 교정 결과](09-11/2026-09-11_TIGER_HORSE_ANIMATION_AND_LANCEMASTER_ALTV_RESULT.md#g08-alt-v-제품-로더의-16mib--64mib-불일치)를 따른다.

### 손 부착 창이 돌아가면 source TypeData 회전 누락을 먼저 구분한다

- MeshRotation distribution의 quarter-turn과 TypeData의 degree 회전은 별개다. Lance V/ALT V source pitch=-90이 typed detail에서 빠져 있으면 실제 +Y 메시가 손본 -Z로 향한다. `[roll,pitch,yaw]`를 기존 `sourceTypeDataRotationDegrees`에 한 번 투영하고 local/socket 회전이나 offset을 임의로 덧붙이지 않는다.
- 같은 `fm_x_flm_gdr_01`/dragon을 쓰는 T34650도 두 full 문서의 typed pitch가 누락/0이었다. V/ALT V 교정이 다른 스킬의 같은 mesh 행까지 자동 적용되는 것은 아니므로 실제 요청 슬롯의 모든 clip을 대조한다. 사용자가 확정한 Transform 위치는 source 회전 복구와 별도 필드로 보존한다.
- source import scale 보정은 방향을 회전시키지 않지만 기존 방향·offset 오류를 크게 드러낼 수 있다. 실제 손본 pose와 설치 mesh vertex의 world 결과로 크기·원점·방향을 따로 비교한다. synthetic axis만 finite라는 검사로 실제 손 부착이 맞다고 기록하지 않는다.

### Composition 재생 거부와 첫 프레임 준비 지연은 별도로 확인한다

- 저장 Composition revision과 게시된 Pattern revision이 다르면 빈 DRAFT만 추가됐어도 Complete Play는 거부된다. 초안을 버리거나 revision 검사를 완화하지 말고 공식 publisher를 사용한다. 깨끗한 편집기의 cached revision이 아닌 실제 저장본을 비교하며 미저장 편집과 게시 중 상태는 보호한다.
- WORLD cue마다 같은 문서를 다시 읽고 전체 검증하면 첫 Play에 동일 비용이 누적된다. 같은 요청의 독립 player는 검증을 한 번 공유하되 모든 문서 복사를 준비한 뒤 교체한다. per-instance 리소스 검증과 시계는 유지한다.
- UI의 World → Fixed position은 MAP·followBoss=false이며 맵 절대좌표를 사용한다. WORLD는 특정 World Object 부착이므로 worldId와 실제 sampled pivot이 필요하다. 빈 WORLD를 게시하면 Product reader가 전체 presentation replacement를 거절해 다른 정상 패턴의 Effect도 빠질 수 있다. 저작 codec과 publisher 모두 이 누락을 거절한다. 유효한 worldId가 있지만 해당 시각의 actor가 준비되지 않은 경우만 anchor 대기다.
- mouse_click LocalDecal의 시작 공백은 birth와 화면 마스크를 구분한다. Life 조절은 burst 시간과 shader의 첫 파동 위상을 바꾸지 않는다. source alpha를 opacity로 단정하지 말고 실제 DDS와 native mask를 대조한다. [수정·검증 결과](09-12/2026-09-12_KOUKU_PLAYBACK_AND_WORLD_MARKER_IMPLEMENTATION_RESULT.md).

### Native effect 생성물은 모델 전용 include와 원본 pass 상수 범위를 함께 검사한다

- `ARTIST_NATIVE_MODEL_ONLY`에서 기본 함수만 제외하고 distortion companion을 포함하면, 모델 파일의 선언보다 먼저 scene-depth texture를 참조해 실제 FXC X3004가 발생한다. companion도 동일 MODEL_ONLY guard를 갖게 하고 생성기와 설치 결과를 함께 수정한다. texture 선언을 앞으로 옮겨 경계를 우회하지 않는다.
- native cohort 확장 시 CB0 material 행뿐 아니라 원본 CB2 pass 상수의 선언·실제 읽기 범위도 확인한다. 고정 4행 scratch는 CB2[4]/CB2[6]를 사용하는 원본에서 X3504를 만든다. viewport 값은 실제 render target 크기, override 값은 검증된 기존 scene 계약으로 공급한다.
- 파티클·메시 컴파일만으로는 MODEL_ONLY include 회귀가 드러나지 않는다. 같은 include를 소비하는 `Shader_VtxAnimMeshBinary`도 FXC로 확인한다. [교정 결과](09-12/2026-09-12_KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT.md)를 따른다.

- 왜곡 PS의 CB1 참조를 `projection`으로 치환했다면 실제 선언과 carrier 입력도 연결한다. 쿠크 `2d8c822c...`는 TEXCOORD5의 source world cm를 한 번 투영한다. screen clip 값을 다시 투영하거나 Trail에 0 행렬을 전달하지 않는다.
- native texture index 9를 쓰는 프로그램은 열 번째 SRV가 필요하다. generated sample helper만 늘리지 말고 renderer staging 배열, bind, screen-post snapshot·mask와 독립 모델 shader 선언까지 같은 상한을 적용한다.
- native 재질 추가 때 기존 descriptor의 상한만 늘려 하나의 mega-switch에 누적하지 않는다. 생성기에서 64 ID 구간별 물리 HLSLI·carrier FX·dispatch·실행 표·project/filter를 함께 갱신하고 원본 함수/guard/ID를 보존한다. 같은 내용은 다시 쓰지 않아 증분 tracking을 유지한다. VS/PS를 패스 간 공유해도 한 PS의 수백 재질 최적화 비용은 남는다.
- IDE와 runner의 Visual Studio/toolset/SDK/host architecture가 다르면 소스 변경 없이도 전체 재컴파일될 수 있다. 현재 `lastbuildstate`만으로 지난 재빌드 원인을 확정하지 말고, runner의 toolchain·전후 state와 선택 실행의 diagnostic 로그를 비교한다. 출력 timestamp 조작이나 강제 skip으로 감추지 않는다.
- 같은 FX의 여러 pass가 같은 entry/profile을 사용하면 `CompileShader` 결과를 공유한다. pass 이름·순서·render state와 서로 다른 entry는 유지한다. 정적/애니메이션 CModel shader도 이 검사를 포함하며, 컴파일 표현식 수 감소와 실제 FX 생성·pass/input layout 검증을 구분한다.
- 증분 측정은 같은 MSBuild와 완전히 같은 인자를 반복한다. 같은 디렉터리라도 `OutDir`의 slash 표기가 달라 `/Fo` 문자열이 바뀌면 FXC command tracking이 전체를 다시 컴파일할 수 있다. 그런 실행은 no-change 결과로 보고하지 않고 별도 재빌드로 기록하며, CSO 내용과 수정 시각 및 실제 FXC 실행 수를 함께 확인한다.

### 시퀀스 목록 표시·소스 검증을 실제 Play 준비와 혼동하지 않는다

- Composition은 Data 원본의 새 WORLD ID를 참조할 수 있지만 Level은 게시된 Area 문서, World Object Tool은 저장 문서의 cache를 사용한다. 저작 revision만 올리고 실행용 `.worldsequences.json`과 `.camerashots.json`을 게시하지 않으면 row는 보여도 Play 준비에서 거부된다. 같은 Area publisher의 Publish와 Check를 수행하고 새 Client에서 동일 WORLD/Camera ID와 revision을 확인한다. 일반 C++ 빌드는 이 배포를 대신하지 않는다. 미저장 Tool 문서를 자동 reload하거나 누락 ID를 건너뛰지 않는다.
- `World Object model admission failed`는 파일 부재만 뜻하지 않는다. 실제 CModel decoder와 material/texture 준비 이유를 구분한다. WMSH submesh를 줄일 때는 같은 submesh의 bounds도 함께 줄이고 bone tail과 나머지 section은 보존한다. 2관문 Table은 4개 중 2개 mesh만 남기면서 bounds 4개를 유지해 80-byte trailing payload로 거부됐다. decoder 검사를 완화하거나 파일 이름만 바꾸어 해결하지 않는다.
- 새 연출은 기존 row까지 포함해 occurrence의 ID·enabled·중복·시간, 실제 model/material/animation 준비, Camera/Effect/SceneProfile 참조를 검사한다. 이 결과와 사용자가 Client에서 Play해 확인한 카메라·연출·전투 결과는 별도 완료 상태로 기록한다. 원본 Fade/카메라/배우/Effect가 여러 문서에 나뉘어 있다는 사실을 SceneProfile 하나에 원본 전체가 들어 있다는 설명으로 바꾸지 않는다.
- Complete Play는 관문별 입장 Sequence뿐 아니라 저장 Pattern Flow와 같은 source revision의 Server Product가 필요하다. Python Product projection만 게시하면 Server bootstrap은 이전 revision일 수 있다. 최종 revision을 명시한 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision <revision>`로 관련 Product/Map/World/Balance를 함께 게시하고 세 관문의 Flow target을 확인한다. F1 관문 spawn 성공을 저장 Flow 존재의 증거로 쓰지 않는다.
- 이전 EXE용 Data 사본을 본 작업으로 합칠 때 양쪽의 새 Pattern이 같은 ordinal ID를 할당할 수 있다. 기준본과 양쪽 저장본을 세 방향으로 비교하고 현재 항목을 덮어쓰지 않는다. 충돌한 신규 항목은 미사용 stable ID와 내부 action/occurrence 참조를 함께 재배정하며 사용자 clip·시간·loop 값은 보존한다.


### Object 그룹 편집은 생성 행과 판정 참조를 함께 저장한다

- WORLD emission의 개수·순서·Delay를 바꿀 때 indexed Collider와 전용 Logic의 참조를 함께 갱신한다. shared hold/result 의존성, 모호한 WORLD, dirty Composition은 저장 전에 거부하고 기존 draft를 보존한다. 슬롯 번호를 stable 저장 ID로 새로 승격하지 않는다.
- 카드 준비 풀은 동일 Area/revision, model/preScale/material 및 device 범위에서만 공유한다. Stop/완료 반환과 문서 교체 시 token 무효화가 함께 있어야 하며, 첫 입장뿐 아니라 Object Save 뒤 reload에서도 다시 준비한다. 실제 GPU/FPS 검증과 CPU 준비 성공을 구분한다.
- WModel이 이미 30Hz이고 FLOAT weights가 정상이어도 child quaternion conjugate 누락은 별개다. 원본 PSA 전체 clip 회전을 대조하고 기존 geometry/skeleton/material과 위치·scale·시간 키를 보존해 교정한다. 괴기스러운 인형은 말·호랑이와 같은 원인으로 확인됐다. [인형·외곽불 결과](09-12/2026-09-12_KOUKU_DOLL_FIRE_REPAIR_RESULT.md).
- WINT minor 증가로 정적 mesh 속성이 추가돼도 내장 WMA2 레이아웃이 유지될 수 있다. repair 도구는 실제 구조와 material identity를 검사해 지원 버전을 명시하고 임의 byte offset 교체로 우회하지 않는다. 외곽불 D/E/F의 잘못된 emissive 입력 제거는 원본 native 불 재질 전체 복원과 구분한다.


### 렌더링 hot path는 실제 소비 입력과 큐 수명을 함께 보존한다

- 정적 월드 표시를 다른 구역까지 늘릴 때는 화면 밖 occurrence의 sample·입자 준비 비용을 함께 검사한다.
  particle별 최종 clip 검사는 이미 실행한 CPU 재생·준비를 되돌리지 않는다. 숨긴 표시의 7초 시계와
  재진입 tail은 보존하고, 작은 카메라 왕복은 가시성 여유 영역으로 흡수한다.
- 최종 카메라 이후에 가시성을 확정하면 이미 끝난 자동 Late_Update의 제출을 그대로 기대하지 않는다.
  해당 owner만 기존 제출 함수를 명시 호출하고 자동 제출을 비활성화해 첫 표시 누락과 이중 제출을 함께 막는다.
  쿠크 marker의 실제 연결·수치 검증은 09-14 KOUKU_MARKER_VISIBILITY_PERFORMANCE RESULT를 따른다.

- source family별 재질 준비를 줄일 때 PS의 family 분기 앞 공통 처리도 검사한다. `Shader_VtxMeshBinary`의 opaque/shadow presentation dither는 source BG에도 `g_Opacity`를 읽는다. native 재질이 raw UV를 쓴다는 이유로 opacity까지 생략하면 소품이 잘못 사라진다. source on/off, diffuse override와 직전 shader 상태를 실제 MRT/depth로 비교한다.
- per-draw 진단 목록은 닫힌 도구에서도 문자열 검색·삭제·할당 비용을 만들 수 있다. 실제 UI 조회가 있는 동안만 수집하고 level 변경·만료와 재열기 동작을 유지한다.
- list 렌더 큐를 capacity 재사용 vector로 바꾸면 callback append가 iterator/reference를 무효화할 수 있다. index로 순회하고 객체 수명은 queue의 shared_ptr로 보존한다. sorted BLEND의 snapshot 순서와 실패/pass 종료 clear를 별도로 유지한다.
- shader instruction/SRV 감소와 CPU Draw 제출 단축을 GPU pixel 실행 단축으로 간주하지 않는다. 같은 입력의 작은·넓은 면적을 각각 비교하고 실제 게임 프레임 결론은 사용자 캡처로 판단한다. [맵·캐릭터 성능 결과](09-12/2026-09-12_MAP_CHARACTER_RENDER_PERFORMANCE_RESULT.md).

### 패턴 Effect 분리는 실제 소비자·간접 원본·수명을 확인한다

- JSON parse와 자체 field 검사만으로 v15 authored 문서 admission을 대신하지 않는다. 비어 있어도 필수인 `runtimeExtensions` 누락은 실제 CEffectDocumentCodec에서 거부된다. 독립 그룹은 그 codec과 Playback roundtrip·seek를 통과해야 한다.
- 본체의 disabled notify를 켜서 부족한 폭발을 보충하지 않는다. SkillEffect → NPC → Action의 간접 원본에 실제 십자 연출이 있을 수 있다. 알비온의 4방향은 raw FRotator를 사용하고, MIC permutation의 texture index는 해당 MIC cooked texture 배열과 join한다.
- source `bKillOnDeactivate`의 metadata 존재를 runtime 소비 완료로 기록하지 않는다. 예고 종료와 긴 입자 tail을 구분해 해당 원본 occurrence의 가시 구간을 유지한다.
- level-owned Effect를 network combat object에 연결하면 보스 weak pointer의 자동 정리를 기대할 수 없다. natural expire와 Stop/사망/despawn의 즉시 취소, 늦은 snapshot의 object 자체 pinned revision을 함께 확인한다. [패턴 그룹 결과 G08](09-12/2026-09-12_KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT.md).

### 이펙트의 90도 오차와 크기 차이는 원본 occurrence별로 분리한다

- 같은 원본 재질을 쓴다고 geometry의 기본 면·긴 축까지 같지는 않다. 실제 WModel 정점과 preScale을 먼저 읽고, TypeData mesh pre-rotation → StartRotation → notify/local TRS → socket·부모 basis → 독립 그룹 전방 변환의 실제 합성 결과를 확인한다. 렌더러 enum만 검사하면 실제 meshModel 바인딩으로 선택되는 mesh carrier를 놓칠 수 있다.
- sourceRecipe에 원본 `pitch=-90`이 남아 있어도 `detail.mesh.sourceTypeDataRotationDegrees`에 투영되지 않으면 그 pre-rotation은 소비되지 않는다. 자동 보정 함수가 존재한다는 이유로 모든 asset이 적용된다고 간주하지 말고 asset admission 조건을 읽는다. 쿠크 hoop의 누락은 창술사 창과 같은 증상이지만 별도 occurrence에서 확인해 보정했다.
- `FRotator`의 65536 정수 단위, StartRotation의 1회전 단위, TypeData의 degree를 섞지 않는다. `[roll,pitch,yaw]`를 위치 벡터처럼 `(x,z,-y)`로 바꾸거나 degree에 다시 360을 곱하지 않는다. 기존 UE3 Euler basis 변환을 사용하고 mesh의 pre-rotation과 particle rotation을 각각 한 번 적용한다.
- 독립 그룹을 +Z 전방으로 맞출 때 source +X의 yaw 보정을 socket snapshot basis, element TRS, particleSystem yaw에 중복 적용하지 않는다. 원본 Projectile의 yaw와 이미 적용된 원본 위치를 합성한 뒤 링 중심·분사구·법선을 대조한다. 한 emitter의 빠진 pitch를 전체 시스템 yaw로 덮으면 정상 sprite와 잔불까지 돌아간다.
- 크기의 cm→m와 WModel preScale, StartSize, notify scale, 골격 basis100·CModel scale, 사용자 확대는 각각 다른 입력이다. mesh 크기를 바꾸려고 입자 위치·속도까지 임의로 나누지 않는다. 같은 화염포를 두 그룹에서 재사용하면 원본·배율·방향 설정의 동등성을 확인하되 element ID별 난수 표본 차이는 허용한다.
- 무기 부착의 원점·반경이 맞아도 회전은 틀릴 수 있다. 원본 PSK와 설치 WModel의 geometry basis, 실제 body bone에 합성한 세 축과 정점을 함께 대조한다. 쿠크 WP05는 identity 손 소켓에서도 설치 geometry의 Y/Z 교환 때문에 catalog의 X축 preRotation -90도가 필요하다. 이 값은 해당 모델의 실측 결과이며 다른 무기·소켓에 일괄 적용하지 않는다.
- UE3 socket의 bone-local 위치·FRotator를 설치 골격에 그대로 복사하지 않는다. UE→PSK export mirror, 원본 bind, 설치 bind와 particle의 좌표계를 함께 합성한다. RPCT05는 원본·설치 bone 이름이 같아도 FX_Prj_03의 Y 부호와 회전 기저가 달랐다. 총구 검증은 같은 실제 clip 시점의 source 발생과 runtime 발생을 비교하며, proxy socket 원점과 입 정점이 다르다는 이유만으로 다른 본이나 추측 offset을 넣지 않는다.
- 실제 Playback의 finite·seek 성공만으로 방향·크기 또는 GPU 표시를 승인하지 않는다. 설치 geometry와 실제 재생 행렬의 축·중심·속도 및 필요한 본 샘플을 확인하고, 화면 크기·색·밀도는 사용자 확인으로 남긴다. 이번 수직 hoop·확대 화염포의 범위와 수치는 [패턴 그룹 결과 G09](09-12/2026-09-12_KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT.md)에 기록한다.


### Rendering Benchmark의 품질 저장·게시·적용 경계

- 스킬별 bloom 값은 Full Restore 문서 root `bloomIntensity`다. source group·skill ID가 같아도 단계별 문서는 별개이며 catalog만 순회하면 Authored 전체 탐색에서 보이는 미등록 문서를 놓친다. 원본 RGB/Emissive 수정과 별도 bloom 기여 조절을 구분한다.
- 투명 이펙트의 bloom은 같은 alpha/additive/occlusion 계약을 유지한다. 이미 합성된 SceneColor를 화면 왜곡 단계에서 다시 추출하면 다른 문서가0으로 억제한 bloom이 살아날 수 있으므로 가중치가 적용된 bloom 입력을 해당 화면 연산으로 함께 운반한다.
- root 필드를 추가한 뒤에는 새 codec과 같은 바이너리로 저장한다. 구 v13 Client는 unknown root를 무시해 열 수 있지만 Save 때 새 필드를 지울 수 있다. 현재 입력 전체를 보존하는 roundtrip과 단계별 독립값을 검사한다. [스킬별 Bloom 결과](09-12/2026-09-12_EFFECT_PER_SKILL_BLOOM_RESULT.md).

- 선택 Level 품질은 해당 base profile의 qualityOverride다. 연출용 scene profile에 그 값을 복사해 고정하면 기본 Bloom을 저장·게시해도 연출 진입 때 예전 값으로 돌아간다. 같은 값의 연출 복사본은 제거해 Level을 상속하고 light/fog/environment·연출 multiplier는 유지한다. 새 scene duplicate가 qualityOverride를 지우는 기존 규칙과 맞춘다.
- Save Authored, Publish Runtime, Reload Runtime은 서로 다른 단계다. 게시기 CLI 성공을 실행 중 catalog 갱신이나 UI 버튼 실패 원인 해소로 기록하지 않는다. 현 Publish_Runtime은 표준 출력을 수집하지 않고 UI thread에서 동기 대기하므로 실제 실패 원인과 무응답을 구분할 정보가 부족하다. [Bloom 게시·조사 결과 G09](09-12/2026-09-12_KOUKU_SOURCE_SEQUENCE_RESTORE_RESULT.md).

### Composition 게시와 타임라인 입력을 함께 잠그지 않는다

- 실행 중 Composition의 외부 Resource 등록은 LastGood와 디스크를 다르게 만든다. Save 거절 후 Reload/재시작 전에 미저장 Pattern·Bundle·staged placement를 보존한다. 자동 병합은 revision 증가와 기존 Resource prefix를 바꾸지 않은 신규 append만 허용하고, ID payload 충돌·다른 외부 편집은 기존 파일과 draft를 보존하며 거절한다.
- Effect placement의 anchor/follow/bone/world와 TRS는 같은 staged 값이어야 한다. Anchor만 바꾼 경우에도 Dirty·Preview·Play·Save·선택 왕복을 검사한다. MAP의 `[0,0,0]`은 해당 관문 중앙이 아니다. 정본 boss placement의 절대 월드 좌표 또는 명시한 사용자 좌표를 저장하고, SourceModelPreview가 없는 Resource 단독 미리보기에도 선택 Pattern의 actor/gate를 전달한다.

- `Publish All Patterns`는 여러 domain을 비동기로 게시한다. 프로세스가 살아 있다는 이유로 박스 선택·scrub·초안 편집까지 막으면 수 분 동안 Sequencer가 멈춘 것처럼 보인다. 메모리 초안 편집은 계속 허용하고, publisher가 읽는 원본을 쓰는 Save와 중복 Publish는 완료까지 제한한다.
- 게시 완료는 Workbench 초안 Reload가 아니다. 게시 도중 만든 초안·선택·커서를 유지하고 미저장 변경은 이후 Save/Publish로 반영한다. 실행 중 프로세스와 실제 완료 로그, 사용자의 입력 복구 확인을 구분한다. [Parent 타임라인 결과 G04](09-12/2026-09-12_KOUKU_PARENT_PATTERN_TIMELINE_IMPLEMENTATION_RESULT.md).

### Composition 게시의 반복 입력 비용

- Pattern/Bundle 후보마다 큰 World Sequence와 전체 WModel vertex·animation key를 다시 decode하면 같은 입력을 수십 번 처리한다. 한 게시 실행 안에서 JSON, 모델과 pose를 재사용하고, timing 검사는 clip metadata만, 본 sampling은 필요한 clip key만 읽는다. 특정 보스 이름에 특례를 두지 않으며 전체 geometry가 필요한 기존 reader 호출은 기본 full decode를 유지한다.
- cache 수명은 호출 안으로 제한한다. 출력 교체 전후 원본 bytes와 모델 hash를 다시 확인하고 변경되면 기존 rollback을 수행한다. 길이·mtime만 같다고 동일 입력으로 판정하지 않는다. PowerShell 원문 비교는 culture 비교인 `-cne` 대신 `StringComparison.Ordinal`을 사용한다.
- projector 직접 변환 시간과 Product/Map/World/Balance 전체 게시 시간은 다르다. 같은 고정 입력의 전후 bytes와 단계별 실측을 함께 기록하고, 한국어 표시명이나 Resources 전체 hash를 측정 없이 원인으로 단정하지 않는다. 실제 imported 도구도 domain fingerprint에 포함한다. [Parent 타임라인 결과 G06](09-12/2026-09-12_KOUKU_PARENT_PATTERN_TIMELINE_IMPLEMENTATION_RESULT.md).

- GPU LOD는 원본 geometry와 현재 draw 재질을 함께 검사한다. CModel material variant는 CMesh를 공유하므로 load-time opaque admission만으로 masked/교체 재질까지 LOD를 적용하지 않는다. indirect index 수는 CPU 추정치를 실제값처럼 기록하지 않고 별도 LOD0 상한 counter를 사용한다. 작은 draw는 dispatch 손익 검증 후 기존 direct 경로를 유지한다.
- light quad의 clip distance는 같은 VS 위치/UV를 유지해도 clipping 이후 interpolation 정밀도 차이를 만들 수 있다. coverage 누락과 FP16 출력 차이, clip-disabled control 및 사용자 visual 판정을 구분한다. 상세 근거는09-12 맵·캐릭터 성능 RESULT의G12–G14를 따른다.

- Composition Patterns의 편집 대상과 Resources의 추가할 Pattern은 별도 session 선택이다. 공용 Pattern Tree를 재사용해도 Resources의 Gate/Parent/Bundle/leaf 탐색은 대상·커서·preview를 바꾸면 안 된다. 실제 Append에서 현재 target/source를 다시 검사하고 잘리는 source 수명을 표시한다. [Parent 타임라인 결과 G07](09-12/2026-09-12_KOUKU_PARENT_PATTERN_TIMELINE_IMPLEMENTATION_RESULT.md).

## C++와 셰이더 빌드 입력 경계

- PCH에는 게임·저작·재질 표를 넣지 않는다. 기본 PCH와 charset/최적화 옵션이 다른 CPP는 PCH와 forced include를 함께 제외하고, 분리 CPP에는 원래 파일 옵션을 보존한다.
- Engine_Defines는 Assimp/DirectXTK/FX11/DirectInput/Engine_Struct의 우회 include가 아니다. 실제 완전 타입을 쓰는 CPP에 해당 헤더를 연결한다. Client에서 WinSock2는 Windows/D3D/DirectXTK보다 먼저 읽고, lean Windows 입력에서 RPC 헤더는 전역 using namespace std보다 먼저 읽는다.
- 생성 native material의 큰 표는 Private owner에서 한 번 컴파일한다. public inline 함수가 사용하는 작은 상수까지 Private로 이동하지 않는다. generator는 native_material_tables.py를 통해 읽기·저장을 하고 같은 bytes는 다시 쓰지 않는다.
- CPP를 분리하면 기존 source 검사도 등록된 same-owner CPP와 Private _Internal.h를 읽어야 한다. 다음 함수의 물리 순서를 기준으로 현재 함수 범위를 추정하지 않는다. cpp_source_domains.py와 Tools/Build/README.md의 소비 경계를 사용한다.
- 무변경 빌드의 OBJ/PCH/CSO 쓰기 0은 증분 처리 확인이다. 공통 셰이더의 큰 최적화 작업이나 cache 없는 빌드까지 해결한 증거로 쓰지 않는다. 세부 구조와 측정은 09-12/2026-09-12_PROJECT_BUILD_ISOLATION_IMPLEMENTATION_RESULT.md에 있다.

### 맵 연출의 원점·렌더 예산·실제 배우를 구분한다

- 클릭 배치는 source 좌표를 추측하지 않고 기존 Picking의 실제 표면 좌표를 사용한다. exact 요청 token·stable 선택 ID·편집 세대를 확인하고 최초 버튼 클릭과 gameplay 클릭을 분리한다. ImGui 다중 viewport에서는 같은 프로세스의 분리 창을 외부 포커스로 오인해 즉시 취소하지 않는다. MAP Effect 원점 표식은 emission 활성 시각과 독립적으로 표시한다.

- 같은 Level의 모든 Effect placement는 캐릭터 한 명의 owner 예산이 아니다. Level 집계는 scene hard 한도 안에서 승인하고 Character/Boss owner와 remote soft 한도는 유지한다. 단독 Play All 성공과 여러 문서 동시 spawn 승인은 별도로 검사하며 실패 asset/occurrence와 원인을 표시한다.
- 여러 source emitter를 묶은 문서의 공통 원점을 특정 도형의 중심으로 간주하지 않는다. 독립 회전이 필요한 부분만 중심 cue로 나누고 나머지 요소의 source 시각·좌표를 보존한다. sourceTransformTrack이 있는 local-space fixed-axis sprite는 잠금 축에 emitter basis를 한 번 적용하며 camera-facing/world-space/mesh까지 전역 회전시키지 않는다.
- MAP position은 절대 월드 미터이고 BOSS/WORLD offset은 대상 상대값이다. Use Player Position은 이동 완료 후 현재 좌표를 명시적으로 복사하는 편집 명령이다. 원본 map light의 절대 XZ를 대상 위치에 다시 더하지 않으며 원본 방향/range가 실제 대상 높이에 도달하는지 검사한다.
- 전투 보스와 World Sequence의 연출 배우는 별개다. WORLD Light는 같은 pattern의 명시적 world occurrence와 실제 샘플된 Deploy/Object/Map pivot을 사용하고, 아직 준비되지 않은 배우에 identity나 다른 보스를 대신 쓰지 않는다.
- 재사용하는 placed sequence의 animation track을 편집하면 이를 참조하는 모든 연출에 반영된다. 목록 별칭을 추가하는 것과 독립 Action Pattern을 만드는 것을 구분한다. Effect 방출 구간, particle tail을 포함한 재생 수명, 사용자가 정한 Box 창도 서로 다른 값이다.
- Parent의 fixedTimeline은 Kouku publisher chain 전체가 같은 optional boolean 계약으로 읽어야 한다. Product projection만 성공하고 다음 World 단계가 unknown field로 실패하면 전체 rollback되어 저장 revision과 게시 revision이 계속 다르다. freshness 검사를 삭제해서 해결하지 않는다.
- cooked Material의 graph가 비어 있어도 native shader map 부재를 뜻하지 않는다. 특수 엔진 재질은 MaterialMap 앞에 global shader 참조가 있을 수 있으므로 count0을 고정 가정하지 않는다. 실제 참조를 소비한 뒤 material GUID·static set/repeated set·VF·uniform trailer·물리 cache hash까지 확인한다. global radial-blur shader 참조를 해당 mesh의 BasePass shader로 선택하지 않는다.
- Matinee의 StaticMeshActor 재질 곡선은 승인된 native parameter 이름·타입·packing으로 연결하고 move와 같은 시계를 사용한다. source bUseQuatInterpolation이 true면 원본 endpoint quaternion slerp를 사용하며 Euler tangent 경로와 구분한다. 기존 optional field가 없는 문서의 계산·직렬화를 보존한다.
- source static MESH의 실제 blend_masked/one-sided는 원본 discard를 보존한 depth-write pass를 사용한다. 같은 parent의 Cascade particle이 depth-read를 쓴다는 이유로 정적 맵 가림막의 깊이 기록까지 생략하지 않는다. source component와 renderer kind에 한정해 바꾸고 기존 particle profile은 유지한다.

- Effect Box의 명시 Preview는 해당 박스 시작에서 재생하고, 배치 드래그의 geometry Preview는 현재 커서를 유지한다. Preview 단축을 위해 BOSS/WORLD 종속 시계를0으로 바꾸면 부착 위치가 달라진다. 문서 내부 StartDelay와 Composition 박스 시작은 별개이므로 실제 선택 asset의 두 시계를 함께 확인한다.
- 원본 맵 연출을 독립 Effect로 바꿀 때 공통 앞 대기를 제거하면 모든 요소 StartDelay에서 같은 값을 빼고 SourceTransformTrack의 SourceTimeOrigin에는 더한다. 상대 emission/native delay/수명과 원본 transform·alpha·material 곡선은 유지한다. source model cue·본·history가 있는 문서에는 이 공식을 일괄 적용하지 않는다. Effect Tool은 미적용 draft를 보존하고 기존 Apply/Save로 처리하며 생성기에도 같은 시간 정책을 반영한다.


### 쿠크 Effect 목록과 보스 선택의 정본

- Catalog만 목록으로 사용하면 설치된 Authored 문서가 숨고, Tree 이름만 검색하면 미분류 한글 문서가 검색되지 않는다. 목록은 Catalog·실제 헤더·트리 참조를 합치고 실제 로드는 선택 시 기존 codec으로 검사한다. 미등록 목록 노출을 Product admission으로 기록하지 않는다.
- 트리의 관문·패턴 분류는 탐색용이다. 원본이 여러 actor/관문에 사용돼도 첫 분류를 재생 보스로 삼지 않는다. sourceModelPreview 또는 실제 Composition resource 연결을 사용하고, 모호하면 사용자가 Model View에서 명시 선택한 보스·clip만 허용한다. 자동 Append가 만든 현재 선택을 사용자 선택처럼 사용하지 않는다.
- 애니메이션 미리보기의 무기를 특정 모델 이름 한 개로 제한하면 실제 NPC에는 있는 지팡이가 preview에서 빠진다. actor가 resolve한 BossCatalog weaponModel·native material·pre-transform을 동일한 실제 손 본에 연결하고 기존 무기의 크기와 bind/animation 동기화를 보존한다.

- World 트랙의 object anchor와 고정 월드 위치는 둘 다 좌표를 쓰지만 서로 다른 Transform 소비자다. UI의 World 안에서 Fixed position과 Follow world object를 구분하고, 기존 MAP/WORLD 저장 계약은 보존한다. worldId가 필수인 WORLD를 '(world position)'이라는 빈 선택으로 제공하지 않는다. 앵커 전환도 전체 staged occurrence 변경으로 처리해야 preview/Dirty/Save가 일치한다.
- 쇼타임 양손 총은 기본 무기를 대체하는 두 World resource다. 기본 무기 숨김은 실제 BODY와 표시 중인 총을 기준으로 합산하며, 한쪽 총 종료 때 무조건 visible=true로 바꾸지 않는다. 등록은 World 재생 수명에 묶고 clone을 pool에 반환하기 전에 해제한다. 같은 clone의 재사용이 이전 actor나 다른 Preview의 무기를 숨기면 안 된다.
- Sequence Effect Append에 resource의 WORLD 기본값만 복사하면 실제 worldId 없는 박스가 생성된다. 새 occurrence는 고정 MAP으로 시작하고 명시 선택한 World box만 연결한다. 기존 WORLD에서도 Player/Mouse 위치 버튼에 접근할 수 있어야 하며 피킹은 hit 성공 때만 MAP·절대좌표·참조 해제를 함께 반영한다. 기존 occurrence Preview에 초기 spawn 좌표를 다시 넣지 않는다.
- 원본 Projectile 복원에서 particle 시각과 track 시각을 중복 이동하지 않는다. 절대 source track key와 문서 startDelay가 같이 있으면 실제 CPU 위치·회전·종료를 샘플해 검증한다. 원본 최대거리로 만든 독립 미리보기 경로를 실시간 대상에 따라 결정된 원본 궤적으로 기록하지 않는다.
- 다수 투사체의 ribbon reserve 합계가 문서 예산을 넘으면 원본 TypeData와 운영 예약을 구분한다. 실제 point 수명·샘플 간격으로 충분한 예약을 계산하고 소스 레시피는 보존한다. 기본 예산을 전역 상향하거나 모든 emitter의 count/size를 줄여 우회하지 않는다.

### 쿠크 원본 애니메이션 이동의 Server 소유

- `b_root` translation을 Server로 옮길 때는 실제 skeleton 부모 basis와 BossCatalog preScale을 함께 적용한다. 다른 보스의 cm→m 상수나 preScale 하나만 복사하지 않는다. Source In을 기준으로 전체 궤적을 읽고 끝점이 0이라는 이유로 왕복·점프를 생략하지 않는다.
- 자동 root와 수동 BossMotion·돌진·teleport는 Pattern 안에서 중복 소유하지 않는다. 모델 및 본 궤적의 root 억제와 Server XYZ를 같이 연결하며, 현재 모델의 vertical scale을 소비하는 과거 source-bone query 때문에 같은 Pattern의 ownership도 일관되게 유지한다. 원본 clip에 없는 actor/script 이동은 추측하지 않는다.
- stage origin은 ENTER의 spawn reset·retarget 이후에 잡는다. collision이 XYZ를 함께 자른 뒤에는 최종 XZ의 실제 지면 높이와 잘린 원본 up을 구분한다. 지면 보정도 collision을 다시 확인하고 실패하면 위치 전체를 보존한다.
- 정수 ms로 원본 fractional key를 옮기는 양자화 오차와 표본 축약 오차는 별개다. 원본 turning key까지 대조하지 않고 시작·끝이나 축약된 표본만 검사하면 중간 이동이 지워져도 통과한다. 정상 설치 clip의 단위·속도·배율에서 실제 오차를 먼저 측정한다. WModel 분석은 Publish에만 연결하고 Save에 추가하지 않는다.

### 통합 Action Workbench의 대상·시계·저장 소유자

- Effect V1/V2는 독립 창과 visibility를 소유한다. Action target으로 다시 우회하지 않는다.
  독립 `Render()`가 이미 catalog/frame/native 복원을 수행하므로 Composition pane Begin/End를
  중복 호출하지 않는다. Character가 사용하는 Effect sequencer factory는 독립 창과 별개로 유지한다.
- 공통 창으로 합쳐도 Boss와 Sequence의 gate/selection은 각 세션에 보존한다. typed deep link는
  대상과 gate를 한 번에 선택한 뒤 exact occurrence를 선택한다. 이전 gate로 잠시 들어가면 다른
  문서에서 고른 박스가 hierarchy 선택으로 지워진다.
- 숨겨진 세션의 Save/Publish 완료 Poll은 계속 소비한다. category 변경은 실제 preview owner를
  정리하고 단일 model clock만 사용한다. pane wrapper만 제거하고 native Attach/Group update를
  빠뜨리면 버튼은 남아도 실제 모델·부착 편집이 사라진다.
- Effect sequence 저장과 제품 skillbindings / animevents / HitShapes 저장은 다른 owner다. 각 Save는
  정확한 source baseline을 검증하고 외부 편집 충돌에서 기존 초안과 파일을 보존한다.
- 피해·무력화·카운터를 세 collider로 나눌 때 HP 예산은 DAMAGE 행만 센다. zero-HP trait가 legacy
  카드미로 즉사나 push를 발생시키지 않게 실제 combat sink까지 확인한다. 새 bootstrap v34 배포와
  Server 재시작 전에 Combat 저장을 제품 적용으로 기록하지 않는다.
- bootstrap 형식 변경 뒤 EXE만 재빌드하면 기존 생성물과 헤더가 맞지 않아 시작이 거부될 수 있다.
  이를 저작 파일 초기화로 오판하지 않는다. 실제 설치 헤더와 코드 요구 버전을 확인하고, 미저장
  편집을 보호한 뒤 저장 정본의 Product projection과 정식 Gameplay Publish를 순서대로 적용한다.
  생성물 헤더 직접 수정이나 버전 검사 완화로 우회하지 않는다. 복구 보고는 저장본의 바이트 보존과
  종료된 프로세스의 미저장 메모리 초안을 구분한다.

- Local Animation Play의 몸체 root를 억제하면서 actor 이동을 연결하지 않으면 원본 backstep도 제자리다.
  실제 parent basis와 preScale로 suppression 전 root를 샘플하고 source crop/rate/loop 끝점 누적을
  같은 절대 clock으로 처리한다. 제자리 walk의 actor 이동은 별도 원본 actor track 근거로 저작한다.
- Logic 블렌드 중 CModel current clip은 target으로 먼저 바뀔 수 있다. 이것을 semantic action 진입으로
  판단하면 effect notify가 반복된다. action occurrence와 pose sample의 소유자를 분리하고, 블렌드 밖의
  기본 pose도 같은 Pattern clock을 사용해 fixed-tick 경계의 점프를 막는다.
- Box Set Group은 selected box 수가 1보다 크다. 단일 선택 전용 drag gate로 그룹 이동을 막지 않는다.
  공통 시간 delta와 linked Logic의 고유 ID를 검증한 뒤 한 번에 적용하며 표시 행도 그룹 전체 구간으로 예약한다.
- transient light 최대치와 실제 provider의 제출 상한을 공통 상수로 유지한다. 광원 탈락을 카메라
  frustum만의 문제로 단정하지 말고 provider 순서·남은 예산·실제 sphere 범위를 함께 확인한다.
- 매우 먼 far plane의 world corner 세 점으로 평면을 만들면 float 정밀도 손실로 가까운 광원도
  잘못 제거될 수 있다. homogeneous view-projection에서 직접 평면을 추출하고 local 판정에서는
  같은 covector 변환을 사용한다. 큰 far 값과 실제 카메라 방향·비균일 배율을 수치로 대조한다.
- Source SpawnPerUnit particle은 source origin이 고정되면 SpawnRate/Burst가 0인 채로 하나도 생성되지
  않을 수 있다. occurrence metadata 수와 실제 particle birth를 구분하고 source 본 이동 누락을 먼저
  확인한다. 검증한 본 변위는 사용자 TRS와 source basis 배율을 중복 적용하지 않는 기존 transform 경로로 넣는다.


### Object Effect·화면 companion·캡처 수축

- Object에 V1 Effect를 붙일 때 같은 모델을 가진 내장 ModelCue까지 그리면 인형이 중복된다.
  실제 Object CModel과 같은 pose clock을 외부 anchor로 제공하고 일치한 ModelCue만 대체한다.
  effect row의 TIME 시작과 followObject/bone, object scale과 modelPreScale을 각각 검증한다.
- 월드 alias의 MAP placement와 화면 ScreenPost companion은 별도 소비자다. World 행만 복사하거나
  preview하면 화면 커튼이 누락된다. 저장된 같은 alias association을 유지하고 external clock의
  Play/Seek/Stop을 같이 호출한다. scope 밖 화면 전용 fallback은 명시 companion에만 허용한다.
- 불투명 검정 바깥을 가진 장면 수축을 Blend 뒤에 합성하면 함께 재생한 포탈까지 덮는다.
  scene replacement만 Blend 전에 처리하고 MRT/depth 복원과 일반 post ping-pong 순서를 검증한다.
- 같은 scene capture를 샘플해도 카메라 앞 mesh와 animated cube가 같은 위치·크기인 것은 아니다.
  실제 설치 큐브 CModel의 현재 pose와 occurrence root/bounds로 수축 종착 transform을 구한다.
  캡처 해상도 축소는 표시 면적 축소가 아니다. HDR/Bloom은 같은 UV를 사용한다.
  검은 바깥은 1관문 도입 수축에만 적용하고 차원술사 Alt+V의 기존 배경은 유지한다.
- 코드 컴파일·후보 JSON 검증·원본 적용·runtime publish·새 EXE·사용자 화면 판정은 분리해 보고한다.
  실행 중 Product 출력과 미저장 draft를 guard 우회나 전체 Composition 덮어쓰기로 해결하지 않는다.
- Object Travel의 개별 visible 수명과 마지막 emission을 포함한 전체 재생 창은 다르다.
  Effect 첫 추가/마지막 삭제에서 ObjectSpan의 지연 가산 경로가 바뀌므로 개별 수명을 유지해
  키를 다시 구성한다. 생성 지연 변경과 Save/Load 뒤에도 마지막 생성물이 같은 수명을 갖는지 검사한다.
- 시각적인 칼날 X축 자전을 서버 ground-plane 원의 회전으로 적용하지 않는다. 중심 offset과
  균일 배율 조건을 확인하고 translation/visibility만 추적하며, 빠른 이동은 tick 사이 구간도 검사한다.
  숨김·Lifetime·Parent 주기 경계를 건너 이전 이동 경로를 다시 판정하지 않는다.
- 최소 TU 검증의 문자 집합 옵션은 실제 해당 파일의 Product compile 명령과 같아야 한다.
  UTF-8 no-BOM 소스를 CP949로 컴파일하는 기존 파일에 새 한글 literal을 넣으면 문자열 경계가
  깨질 수 있다. `/utf-8`을 추가한 격리 compile 성공으로 대체하지 말고 파일 인코딩과 프로젝트
  설정을 유지하면서 필요한 UTF-8 표시 문자열을 byte escape로 표현해 Product Build도 확인한다.
- 보스의 기본 아레나를 옮길 때 BOSS_SPAWN 상대 행과 NONE/World 절대 좌표를 구분한다.
  Gaze target으로 clone 반경을 계산하는 경로와 기존 MAP alias의 positionOffset도 같은 이동량을
  반영해야 한다. 원본 placement·행·시각을 보존하고 실제 nav의 target와 clone 위치를 검사한다.
- Kouku Product 회귀는 현재 Gate/placement admission과 실제 Prepare_KoukuAuditionTick →
  per-boss Update 순서를 사용한다. 시작 위치 복귀 직후와 첫 root motion 이후를 구분하고,
  PATTERN_COMPLETED의 마지막 stage와 전체 run COMPLETED의 stage 0을 혼동하지 않는다.
  저작 좌표·sequence 길이가 바뀌면 이전 고정 기대값을 제품 오류로 단정하지 말고 실제 정본과
  테스트 입력·호출 순서를 대조한다. bootstrap fixture도 지원 버전의 필수 열을 갖춰야 한다.


### Play All과 Composition의 일반 v15 Effect 준비

v15는 runtimeCarrier와 baked history가 없는 일반 mesh/sprite 문서도 허용한다. 파일 버전만으로
DocumentOwnedRuntimeProjection을 강제하면 `no admitted runtime carrier`로 Play All·Product worker
준비가 거절되고, 일반 Stage_Document를 쓰는 Family만 표시될 수 있다. Codec 검증 후
`CEffectDocumentCodec::Requires_DocumentOwnedRuntimeProjection`으로 실제 carrier/history를 검사하며
Tool factory·Catalog 직접 로드·worker·Debug 등록/교체와 이전 cache 검사에 같은 판정을 사용한다.
특수 carrier와 orphan history의 기존 검증은 유지한다. selector 통과만으로 전체 재생을 검증하지
말고 실제 문서 staging을 대조한다. [수정 결과 G13](09-13/2026-09-13_KOUKU_CINEMATIC_WORKBENCH_IMPLEMENTATION_RESULT.md).


### 2026-09-14 고정 화면 캡처와 저프레임 표시

- 넓게 늘어난 화면을 particle distortion만으로 단정하지 않는다. 현재 camera의 보간 FOV와 projection을 먼저 측정한다. 167~179도 FOV의 원근 확대는 작은 distortion MRT offset과 구분한다.
- occurrence Stage의 마지막 SceneHDR는 나중의 ScreenPost 시작 장면이 아니다. 고정 화면 전환은 resolved HDR/bloom 입력을 해당 렌더 시점에서 함께 캡처하고, scene 합성 뒤·HUD 앞에서 그린다. 캡처 실패와 아직 대기 중인 상태를 구분한다.
- Preview의 최종 커서가 Late_Update 뒤에 바뀌면 WORLD/camera뿐 아니라 Effect exact seek와 다음 culling frame까지 맞춰야 한다. ScreenPost A/B off 또는 실패 경로에서 capture 대기를 계속하지 않는다.
- Preview의 느린 프레임을 새 occurrence로 처리하지 않는다. 150ms 초과 sample에서 V1 handle을 지우면 고정 캡처도 소멸해 다음 resolver가 전환점으로 되돌아간다. 단일·bundle Preview는 기존 외부 시계로 진행하고 박스 종료·Stop·새 Preview에서 정리한다. 캡처 ready 검사뿐 아니라 ready 이후 저FPS sample의 handle 수명까지 검증한다.
- 큐브로 전환하는 화면 캡처는 ScreenPost의 다음 활성 frame이 온다는 전제에 의존하지 않는다. 마지막 활성 frame에 latch돼도 후속 ModelCue material이 같은 캡처를 직접 소비한다.
- source action4219903의 알비온 공중 자세 _24_03은 원본 root 상승이0이다. 연속 착지 _24_04/_24_05의 실제 하강 합을 앞 상승 run에 배분한다. 기존 source TRS·XZ·하강과 nav는 보존하며 사용자 저작 duration/clip window에 맞춰 계산한다.
- 원본 ancestor keys가 상수임을 이미 확인한 경우 scale100 quaternion 보간 행렬의 float noise를 다시 절대1e-5로 비교해 애니메이션 root sampling을 거절하지 않는다. 실제 ancestor key 변화·nonfinite는 계속 거절한다.


### Workbench의 반복 계산과 실패 리소스 이름 조회를 구분한다

- UI draw의 이름·상태 표시는 이미 로드된 view나 명시 선택 시 확보한 metadata를 사용한다. 성공만 cache하는 Catalog::Find를 매 프레임 호출하면 손상된 파일 하나가 반복 I/O를 만들 수 있다. 펼친 metadata → Find_Loaded → 저장 stable ID 표시를 사용하고, 실제로 확인한 이름만 Rename 대상으로 제공한다.
- 이전 camera 실패 cache가 남아 있는지와 실제 camera Load scope를 먼저 확인한다. 문자열·category·트리 계산 비용이 큰 경우를 실패한 로드의 재시도로 단정하지 않는다. kind/version 필터를 텍스트 검색 전에 적용하고 불변 메뉴 문자열은 매 frame 정규화하지 않는다. Composition Effect 목록은 inventory Refresh와 version/owner/search 변경 때만 필터·목록·분류 트리를 다시 만들고, Created 목록은 draft generation도 검사한다. category는 참조로 읽고 owner 결과를 재사용한다. 선택은 stable source ID로 조회하며 펼친 Element를 캐시 목록에 매 frame 누적하지 않는다. 검색·Locate·Refresh·이름 변경 때 캐시 갱신과 동적 duration 보존을 함께 검사한다.
- inclusive Composition Build 시간은 자식 비용을 포함한다. metadata 검색의 CPU 비교를 전체 UI 시간이나 실제 FPS 개선량으로 보고하지 않는다. pane별 계측과 사용자가 저장한 같은 조건의 profiler를 대조한다. [Workbench 결과 G17](09-13/2026-09-13_KOUKU_CINEMATIC_WORKBENCH_IMPLEMENTATION_RESULT.md).

### 짧은 발사 섬광과 움직이는 BG 불

- 40~70ms source particle은10FPS의100ms update 안에서 생성·소멸할 수 있다. box lifetime이나 emission delay와 개별 particle life를 구분하며, 모든 fixed step을 정상 실행해도 마지막 상태만 그리면 섬광이 보이지 않을 수 있다. scoped 표시 후보를 보존할 때 source simulation/lifetime은 그대로 두고 World·Color·Dynamic·SubUV·material sample time을 같은 substep으로 유지한다. 선언 순서와 GpuOccurrence의 contiguous row count까지 맞추며 Seek에는 적용하지 않는다.
- 정적 배경과 움직이는 오브젝트의 texture가 같아도 RNM/static-shadow 유무로 최종색이 달라진다. 이동 오브젝트에 정적조명 좌표를 복제하지 않는다. 사용자가 조명 독립 불을 요청한 경우 해당 material binding의 optional unlit만 사용해 BG surface RGB를 emission으로 출력하며 기본false와 비불 오브젝트는 유지한다. Engine surface public 구조가 바뀌면 Engine/Client를 함께 빌드한다.

### ScreenPost TexturedOverlay의 DDS coverage admission

- V2 TexturedOverlay는 현재 기본 A coverage를 사용한다. DXT1/BC1 이미지를 RGBA로 디코딩하면 alpha=1로 보여도 Presentation_Manager의 A 채널 format 허용 목록에는 BC1이 없어 submission이 거절될 수 있다. 픽셀 확인과 DDS/SRV format admission은 별개다.
- 단색 암전은 기존 intensity/tint를 보존하고 A가 명시적으로 있는 불투명 RGBA8 텍스처를 전용 asset ID로 연결한다. 다른 효과가 공유하는 BC1 원본을 덮어쓰거나 전역 검사를 제거하지 않는다. 추가 Resources는 다른 PC에도 전달한다.
- 통합 암전의 수정 범위와 사용자 화면 확인 경계는 09-12/2026-09-12_KOUKU_SOURCE_SEQUENCE_RESTORE_RESULT.md G10을 따른다.

### World Sequence 문서에 생성기로 행을 추가할 때

- 완성 Composition의 이펙트·UV·타이밍을 보존하는 병합에서도 별도 World Sequence의 승인된 맵 움직임을 파일 전체 `ours`로 누락시키지 않는다. 표시 WORLD row → sequence instance → template track·binding과 MAP placement를 함께 비교하고, 요청된 row의 변경만 선별한다. `피날레_맵`은 배치 3·419의 수정만 들어오고 `circus_finale`의 앞판 키·뒤판 바인딩이 빠지면 수정 배치와 이전 움직임이 섞인다. 같은 숫자 worldId도 Boss Composition과 Sequence Composition에서 의미가 다르므로 파일·displayName·instanceId를 함께 확인한다.
- 한 인스턴스는 같은 Object Resource를 한 번만 바인딩할 수 있다(`WorldSequenceDocument.cpp` Validate `boundTargets`, `Publish-MapAuthoring.ps1`의 같은 검사). 같은 모델을 여러 슬롯에 두려면 조각마다 Object Resource를 만든다. 이 규칙에 걸린 문서는 publisher만이 아니라 툴/Client 로드도 실패하므로 설치 전에 후보로 검사한다.
- 정본 `.worldsequences.json`은 python `json.dumps(indent=2)`(배열 한 줄에 한 값)로 쓰면 같은 내용이 툴 Save 형식(배열 한 줄, float32 9자리)보다 약 1.4배 크다. 2관문 소품 165개 설치 뒤 indent=2 형식은 16MiB reader 한도를 넘는다. 이 문서를 다시 쓰는 생성기는 `Tools/KoukuSaydonPipeline/build_gate_cutscenes_g12.py`의 `tool_document`(툴 Save와 같은 형식, 재파싱 float32 동일성 검증 포함)를 사용한다. minify는 툴이 재확장하므로 해결책이 아니다.
- Composition(`KoukuSaydonSequenceComposition.json`)의 `worlds` 등록 ID는 reader가 `kakulsaydon.g1.world.<n>`(n < `nextWorldOrdinal`) 또는 `world.kouku.gate2.intro.<x>`(인스턴스 `world.sequence.instance.kouku.gate2.intro.<x>`와 짝)만 받는다(`KoukuSaydonCompositionDocument.cpp` 1165행). 다른 이름을 등록하면 Composition 전체가 "not admitted"로 로드되지 않으므로 생성기는 `nextWorldOrdinal`을 소비해 ID를 발급한다.
- 원본 Matinee의 배우 `drawscale` float 트랙은 `build_source_sequences.actor_world`가 굽지 않는다. 배우가 커지거나 작아지는 컷(3관문 비행 6→2, 1→0.3)은 template `scaleMultiplier` 키로 따로 넣어야 한다. 근거는 09-13/2026-09-13_KOUKU_G12_CUTSCENES_CAMERA_MAP_RESULT.md G13-R3/R4.

### 탈것·NPC 재질의 UV와 shader cache 해석

- NPC 파이프라인으로 쿠킹한 skinned `.wmodel`은 1.0이라 UV1이 없다. 원본 MIC가 program 5/7/18/19를 쓰면 `CModel`이 "source character requires native extra UV channels"로 모델 전체를 거부하고, 탈것·NPC 표현이 소리 없이 격리된다. 원본 PSK의 EXTRAUVS 유무를 확인하고, set이 1개면 UE3 clamp 규칙대로 해당 submesh UV1=UV0를 `Tools/VehiclePipeline/cook_single_set_uv1.py`로 추가한다. extra set이 있으면 `cook_ocular_uv_channels.py`처럼 원본 채널을 join한다.
- UV1을 요구하지 않는 program도 셰이더 안에서 `v4.zw`(TexCoord[1])를 샘플할 수 있다. 별빛의 가호 외피 program 88은 panning 발광을 UV1로 읽어, UV1이 0이면 텍스처 한 점이 ×10 발광으로 칠해져 진한 파랑이 된다. 생성 셰이더의 `v4.zw/wz` 사용을 확인하고, PSK에 EXTRAUVS0가 있으면 `Tools/VehiclePipeline/cook_psk_extra_uv1.py`로 삼각형 단위 join한다.
- `build_npc.py`의 non-self-rigged 경로는 메시를 master armature에 rebind하므로 inverse bind가 master의 ref pose가 된다. 메시와 master의 본 translation이 다르면(모코보드 `b_body_00` 50cm vs `MN_PMSHB_00` 19.41cm) 메시만 그 차이만큼 떠서 좌석 본과 어긋난다. 메시 PSK와 master PSK의 REFSKELT를 비교하고, 다르면 같은 AnimSet으로 `master.selfRigged=true`(master file=메시 PSK) 재쿠킹한다.
- 생성 SourceCharacter 셰이더의 leading/trailing unowned cb0 행과 varying 배치는 program마다 다르다. 반투명 88은 cb0[0].w 엔진 opacity, cb0[18..20] sky light, cb0[21].x 반투명 모드를 쓰고, v5=fog·v6=tangent view·v7=up(program 18 배치)이다. 새 program 설치 후 0으로 남은 엔진 행과 `MakeSourceCharacterInput` 배치를 사용처 기준으로 대조한다.
- SourceCharacter program은 번호 구간별 CSO 변형(`*_SourceGroupNNN.hlsl`)으로 컴파일된다. 마지막 그룹 밖 번호를 `install`하면 `needs a registered CSO cohort`로 거부되고, 도구만 고치면 런타임 `CShader::Stage_ProgramVariants`가 그 program을 어떤 변형에도 배정하지 못한다. `Engine/Private/Shader.cpp` 범위, `native_shader_dispatch.py` 그룹 표, `Model.cpp` 상한, 변형 probe 반복 범위를 같은 변경에서 늘린다.

- UE3 static parameter set의 `FNormalParameter`는 FName 8 + CompressionSettings 1 + bOverride 4 + GUID 16 = 29바이트다. 공용 shader cache oracle은 32바이트로 읽어 normal 파라미터가 있는 MIC(예: 랩터 `monster_base_msk_high`)에서 `ShaderCache FName index is invalid`로 실패한다. `Tools/VehiclePipeline/build_vehicle_source_material.py`는 도구 안에서만 29바이트로 보정한다. NPC 파이프라인 쿠킹본의 재질 슬롯 이름은 LookInfo 교체 MIC 이름이 아니라 메시 기본 이름이므로 catalog `materialName`은 `rows … @슬롯이름`으로 지정한다.


### Character local space와 시작 화면 캡처의 소유 시점

- particle localSpace=false는 생성 때의 SpawnRootWorld를 보존한다. 저작 문서의 localSpace와 source notify/socket anchor를 구분하고, sourceRecipe·Read-only Reference를 임의 수정하지 않는다. CPU 합성 anchor에서 입자 World가 유지된 검증을 실제 bone 부착이나 GPU 외형 PASS로 기록하지 않는다.
- 화면 큐브 수축은 cinematic camera 적용 전에 Stage가 저장한 HDR/bloom pair를 첫 ScreenPost에 전달한다. 첫 Render에서 비어 있는 capture에 다시 live scene을 채우면 이미 움직인 카메라를 캡처한다. 중앙 수축은 destinationUV를 화면 중앙으로 유지하고 target model은 끝 크기만 제공한다. 포탈의 전환 시점 캡처와 혼동하지 않는다.
- Effect mesh가 useModelMaterial=false여도 CModel 생성은 WModel에 기록된 material texture를 읽는다. 파생 WModel을 Effect/Meshes에 옮길 때 embedded relative DDS도 hash와 함께 닫아야 한다. 뒤 Queued 성공 메시지로 앞선 실패 원인을 덮지 않으며 capture 실패는 해당 occurrence에서 판정한다.
- 재현·검증과 남은 화면 경계는 [캐릭터/아레나 결과](09-14/2026-09-14_CHARACTER_EFFECT_AND_ARENA_RECOVERY_IMPLEMENTATION_RESULT.md), [쿠크 재생 결과](09-14/2026-09-14_KOUKU_SEQUENCE_PLAYBACK_EDITOR_IMPLEMENTATION_RESULT.md)를 따른다.

### WORLD 앵커 검사는 실제 소비 종류와 두 Composition 정본을 함께 확인한다

- WORLD는 Camera shot의 기존 좌표 표기와 Effect/Light/Collider의 World Object 참조에서 함께 쓰인다. `worldId` 필수 검사를 모든 presentation kind에 적용하면 카메라까지 거절돼 통합 시퀀스가 재생 준비에서 중단된다. 실제 object transform을 소비하는 Effect/Light/Collider에만 필수 참조를 요구하고 Camera/Sound의 전역 재생 계약은 보존한다.
- 공용 CompositionDocument·Workbench 변경은 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`뿐 아니라 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`도 검증한다. 한쪽 정본의 컴파일·Summon roundtrip·Gameplay publish 성공으로 다른 정본의 Preview admission을 대신하지 않는다. 실제 저장 카메라를 Parse → Validate → Expand하고 Product reader/Python projection의 같은 kind 검사를 대조한다.
- 앵커 오류 화면은 우선 재생 준비의 검증 실패로 분류한다. CPU/GPU 병목으로 단정하지 말고 실패 occurrence의 실제 resource kind·world 참조·소비자를 확인한다. 카메라를 MAP으로 일괄 변환하거나 worldId 없는 Effect를 허용하는 방식으로 우회하지 않는다. [발생 원인과 검증 G11](09-14/2026-09-14_KOUKU_SEQUENCE_PLAYBACK_EDITOR_IMPLEMENTATION_RESULT.md).

### 창 포커스와 Map 조명 삭제 저장

- DirectInput의 BACKGROUND 수집은 외부 앱 타이핑까지 게임 입력으로 보낸다. FOREGROUND 장치와 raw getter의 창 검사, focus/read 실패 시 상태 초기화, 복귀 시 held 입력 release 대기를 함께 유지한다. 비활성 상태의 조준·hold 예약도 기존 취소 경로로 정리한다.
- 여러 Client의 사운드는 foreground PID 기준 FMOD master mute로 분리한다. 개별 volume이나 명시 pause를 덮으면 복귀 시 사용자 설정·시퀀스 시간이 깨진다. 같은 프로세스의 분리 도구 창과 다른 Client EXE를 구분한다.
- Map Light 삭제 후 기본 방향광으로 선택을 바꾸면 뒤 Save가 RenderingProfiles를 저장할 수 있다. Map 저장 도메인을 유지하고 선택과 무관한 Save Map Lights를 제공한다. 삭제 저장과 runtime 게시를 구분하고, `-Scope Lights`는 maplights 한 파일만 게시한다.

### 마리오 복귀 완료는 이동 종료로 판정한다 (2026-09-14)

`Clear_MarioControl`은 마지막 출구의 이동 시작에 실행되므로 `iMarioStage == 0`만으로 다음 전투를 시작하면 복귀 중에 2페이즈가 열린다. Server가 원래 참가자 identity와 terminal source placement를 유지하고, `Update_PlayerMotion` 종료 및 실제 3관문 착지를 확인한 뒤 복귀 완료로 소비한다. 랜덤 패턴의 마지막 tick에 제출된 입장도 post-update commit 뒤 판단해야 한다. Intro 안에서 0키로 복귀를 시작하면 stage가 이미 0이므로 terminal 이동 중 Intro의 자동 재진입을 차단한다. 미진입·복귀 먼저·복귀 지연·중복 요청·사망·퇴장을 별도로 검증한다.

### Object 원형 반경 편집과 기본 외곽불 중복

- Radial Offset을 현재 반경에 반복 Apply하면 같은 값이 계속 누적된다. 현재 반경의 직접 입력과 마지막 Save/Reload 대비 차이를 구분하고, UI 값 변경을 validate한 뒤 기존 Object Preview의 현재 clock에 전달한다. 저장된 orbit 반경과 실행 중 미저장 draft를 혼동해 원본 JSON을 덮어쓰지 않는다.
- 3관문 기본불은 별도 Gate Object player가 소유한다. Object Preview만 갱신하면 원래 불이 겹쳐 편집이 반영되지 않은 것처럼 보인다. 같은 instance/template의 Preview가 빌린 기본불만 잠시 숨기고 Stop 때 복원한다. 명시적으로 despawn하거나 교체한 owner는 Preview Stop/rollback에서 다시 생성하지 않는다.

### 마리오 0키의 일반 창 포커스와 실제 입력을 구분한다

DIK_0은 위쪽 숫자열이며 DIK_NUMPAD0과 다르다. 연결된 typed 요청이 있어도 ImGui의
WantCaptureKeyboard가 일반 편집창 포커스를 이유로 요청 전송을 차단할 수 있다.
명시적 복귀 단축키만 일반 포커스를 통과시키고 text·active widget·foreground·gameplay camera·
속박·Server state 검증은 유지한다. 전체 gameplay gate를 해제하거나 Client Transform을
직접 옮기지 않는다. [전후 입력·Server 검증](09-14/2026-09-14_KOUKU_MARIO_SERVER_PROGRESSION_IMPLEMENTATION_RESULT.md#g06-숫자열-0의-편집창-포커스-차단-수정).

### 캡처 대기 진단은 실제 재생 중의 상태로 판정한다

- `Queued admitted Effect for post-update layer commit`은 spawn 요청의 과거 성공 문구다. active 생성·capture 완료·현재 대기를 증명하지 않는다. Stop 뒤 남은 문자열만으로 멈춤 원인을 정하지 말고 owner/playing/capture state와 종료 직전 오류를 함께 확인한다.
- capture의120회 제한은 초가 아닌 preview update 횟수다. 경계에서 전체 Effect history를 재구성하면 한 update가 느려져 실제 대기가 길어진다. capture ready를 가정한 clock test나 synthetic Bind 성공을 실제 Bundle → 서비스 → Renderer 완료 증거로 대신하지 않는다.
- BC1(DXT1)은 불투명/1-bit alpha를 공급한다. alpha coverage를 BC2/BC3/BC7에만 허용하면 실제 BC1 흰색 DDS를 쓰는 V2 암전 overlay가 매 프레임 거부된다. authored coverage나 텍스처를 바꾸기 전에 실제 loader가 만든 SRV format과 Engine channel 검증을 대조한다.
- 명시적으로 격리된 LOCAL_PROVIDER_CONTRACT 실패는 그 provider의 light/post/overlay와 통계만 rollback한다. 전체 frame을 지우고 S_FALSE를 반환하면 Client는 계속 실행되지만 정상 capture도 Bind되지 않아 preview clock이 경계에서 영구 대기한다. 다른 provider의 정상 제출은 유지하고, 전역 실패와 budget 초과는 전체 rollback한다. [실제 DDS 재현과 수정 G13](09-14/2026-09-14_KOUKU_SEQUENCE_PLAYBACK_EDITOR_IMPLEMENTATION_RESULT.md).
- 1관문 장면 수축의 임시 visible=false 변경은 원본으로 되돌렸다. 캡처·검은 배경·수축 연출과 portal particle30개, 원본 fade 및 팝업북 timing을 유지한다.

### 외부 WORLD 미리보기는 실제 actor와 같은 시각의 본을 전달한다

단일 Bundle의 WORLD를 Level에 맡겨도 actor 소유자는 Bundle member다. 전역 Model View나
복제 보스로 다시 찾으면 같은 외형의 다른 BODY에 붙거나 anchor 대기에서 소품이 숨겨진다.
현재 actor의 본 resolver를 해당 sample에 명시 전달하고 actor/weapon pose→WORLD→Effect
순서를 유지한다. external member1/offset0, Level visibility/조명 정리와 기존 Model View 경계를
보존한다. waiting을 true로 반환하는 객체는 그 이유도 현재 preview 상태에 전달해야 한다.
inactive Animation backend의 정리 메시지가 WORLD의 실제 실패 이유를 덮지 않도록 한다.
[쇼타임 Play 결과](09-10/2026-09-10_KOUKU_HAND_PROPS_RESULT.md#g06-09-14-쇼타임-play와-총-resource-preview의-actor-연결).

optional 저작 필드를 추가해도 구 EXE의 strict shape parser에는 호환되지 않을 수 있다.
같은 문서의 무관한 object까지 로드가 막히므로 실행 중 프로세스의 parser와 전체 문서를 대조한다.
후보를 먼저 준비하고 새 제품 빌드 뒤에 게시한다. 먼저 게시했다면 사용자 변경을 보존하면서
해당 template/필드만 호환 형태로 복구하고 정상 publisher와 구 reader로 재확인한다.


### 캡처 축소의 끊김과 Stage 공백 편집 거절을 분리한다 (2026-09-14)

- 캡처 ready 이후 화면이 사라졌다 다시 나타나면 render race로 단정하지 않는다. Renderer 순서의 왜곡 → capture Color/Bloom 고정 → display-space overlay를 함께 확인한다. Kouku P4의 V2 fade가 20.5초에 화면 전체를 덮고 23.13초까지 걷히면 정상 frozen scene도 어둡게 보인다. 처음 0~2초 fade와 두 번째 전환은 구분해서 수정한다.
- V1 Composition 박스 종료만 당겨도 ScreenPost의 authored shrink seconds는 자동 변경되지 않았으므로 중간 크기에서 잘렸다. `Seek_WorldRoot`의 owning-box end age를 pending/active Object/Renderer까지 전달해 scene-collapse만 가용 시간으로 제한한다. particle age, freeze된 SRV, 차원술사 cube target 계약은 유지한다. user 선택은 capture 19,819 → popup 21,010ms다.
- Stage 길이를 줄이면 애니메이션 pose 경계는 움직이지만 절대 시간 ANIMATION_BLEND는 그대로여서 저장이 거절될 수 있다. Pattern 시작 offset 변경은 실제 source/target stable occurrence pair를 resolve해 기존 정책으로 검증한다. Stage 길이 변경은 비애니메이션 시간 보존 정책에 따라 blend를 retime하지 않고 충돌 시 candidate를 거절한다. validator를 완화하거나 사용자 draft를 버리지 않는다. 일반 Effect/Logic/WORLD의 독립 시간은 건드리지 않는다.
- 외부 JSON 편집 전 저작 도구의 미저장 draft와 LastGood/disk를 확인한다. clean 확인 뒤에도 직전 파일 bytes가 바뀌면 재조사하며 사용자 조명·패턴 수정 전체를 이전 snapshot으로 덮어쓰지 않는다.


### Parent timeline same-folder 거절 (2026-09-14)

Parent folder의 timelinePatternId와 해당 Pattern의 folderId는 같은 관계의 양쪽이다. 외부 데이터 편집으로 하나만 넣으면 기존 validator가 Parent를 격리하고 same-folder 오류가 난다. 이번 P1/P4 누락은 새 조명 저장 이전 rev43부터 있었으며 현 Stage_ParentTimeline/Serialize/Parse는 양쪽 관계를 정상 유지했다. 두 owner의 누락 folderId만 복구하고 actual codec 왕복과 Expand를 확인했다. 사용자 새 배치를 이전 snapshot으로 되돌리거나 same-folder 검사를 제거하지 않는다. Child의 resetBossToSpawn은 expansion이 거부하므로 부모의 동일 actor/placement를 확인하고 Parent 시작의 spawn reset을 사용한다. child 시작 시 teleport와 Parent 시작 위치는 다른 계약이다.

### World/Camera owner와 Composition 게시 검사의 계약 일치 (2026-09-14)

WorldSequenceDocument와 Map publisher에 새 effect track 필드가 추가됐는데 CompositionPipeline의 exact-field 검사를 갱신하지 않으면 정상 사용자 저장본도 publish에서 막힌다. optional followObject는 bool, bone은 빈 문자열 또는 유효 UTF-8 최대256bytes/제어문자 금지, resourceKind는 LEAF/GROUP/V1_EFFECT라는 현재 consumer 계약을 함께 유지한다. Camera Shot 목록 한도도 Arena/Map의128과 Composition이 일치해야 한다. 이번 source110개를 예전64 제한으로 거절하던 검증을128로 맞췄고129개는 계속 거부한다. 원본을 삭제하거나 unknown-field/경계 검사를 통째로 끄지 않는다. 실제 owner 문서의 검증·입력 보존과128/129 경계를 함께 확인한다.

### Parent 연결 복구 뒤 단독 Sequence 행의 위치 (2026-09-14)

유효한 folderId가 생긴 Parent owner는 Gate-root leaf와 폴더 child leaf에서 중복 표시하지 않는다. 기존 Render_PatternTree의 `[Parent]` 이름 행을 누르면 해당 timelinePatternId가 선택된다. 사용자에게 단독 Sequence 행이 그대로 있다고 안내하지 말고 실제 Parent 경로를 알려준다. 목록이 없어졌다는 보고는 저장본 존재/로드 오류/선택 Gate/Parent 행을 먼저 대조한다. JSON 검증 성공만으로 기존 목록 위치까지 유지됐다고 결론 내리지 않는다. 이 목록 표시는 발탄 패턴 oracle 검증과 별도다.


## 캡처 경계에서 같은 particle history를 반복 seek하지 않는다

Sequence capture hold는 같은 occurrence의 계속 재생이다. 모든 V1을 bRebuildHistory=true로 다시 Seek하면 대기 frame마다 원점부터 수백 fixed steps를 재생한다. normal external Update와 capture commit은 같은 first/rewind/large-seek/edit 판단을 사용하고, WORLD/animation 이후의 final sample만 기존 history에 이어 반영한다. 단순히 replay flag만 끄면 고정 간격 사이의 캡처(예4.26초)가 직전 display sample에 남을 수 있다. 고정 입자 계산과 정확한 마지막 display/root/source-anchor sample을 구분하며 final provider도 commit 전에 검증한다. zero delta에서도 같은 시각의 anchor 변경·실패를 소비한다. CPU 호출 감소를 최종 화면의 hitch-free 승인으로 기록하지 않는다.

포탈 왜곡의 emitter 종료와 살아 있는 particle tail의 종료는 다르다. Scene capture는 그 시점의 이미지를 고정하므로 포함된 왜곡도 보존된다. 전환용 입자와 캡처 화면을 별도 기존 V1 occurrence로 나누고, 앞뒤 연출이 한 source effect에 함께 있으면 후반 stable element IDs와 local clock을 보존한다. 임의로 전체 box를 줄여 후반 원본을 삭제하지 않는다. 페이드는 기존 display-space TexturedOverlay를 사용하면 capture/Bloom 합성 뒤에 적용되며 캡처 원본에 다시 구워지지 않는다.


V1_ELEMENT 선택은 renderer 제출 범위이며 전체 effect의 particle simulation/입장 예산을 자동 축소하지 않는다. 전환용 화면만 필요하면 전용 SCREEN_POST 요소를 기존 asset 경로에 분리해 실제 simulation 입력도 줄인다. runtimeCarrier가 있는 요소는 visible=false만으로 계속 저장할 수 없는 계약이 있으므로, 파생 subset은 stable ID·cross-reference closure를 검증하고 필요한 행만 보존한다. 원본을 변경하거나 codec의 visible 검사를 제거하지 않는다.

### 총 WORLD에 원본 BOSS 섬광을 다시 부착하지 않는다

Composition의 WORLD anchor를 고르는 것만으로 Effect 내부 actionCueAttachment와 notify TRS가 제거되지는 않는다. 원본 양손 섬광을 총에 붙이면 양손 본·내부 위치가 총 WORLD 위에 다시 합성된다. 총구용은 한 손 subset과 중립 내부 부착을 가진 기존 파생을 사용하고, 설치 WModel에서 측정한 총구 위치·방향을 WORLD local offset으로 설정한다. 원본 중첩을 상쇄하던 큰 offset을 파생으로 그대로 옮기지 않는다. WORLD scale과 이미 발생한 world-space 입자의 잔상도 구분한다. [손 소품 결과 G07](09-10/2026-09-10_KOUKU_HAND_PROPS_RESULT.md#g07-09-14-양손-발사-섬광과-한-손-총구-섬광).

Effect의 `anchorKind=WORLD` 참조는 부착할 대상이며 소유한 placement가 아니다. Effect 또는 Effect 그룹만 복제하면 기존 총을 참조하고, World 박스를 명시 복제했을 때만 부착 Effect를 새 World로 remap한다. `selectionGroupId`는 Effect에서 선택·시간 이동을 공유할 뿐 좌우 본·offset·rotation을 합치지 않는다. Collider의 공통 anchor 공간 제약은 별도로 유지한다.


### 수축 종료 뒤 별도 context 왜곡과 self-motion 가시성

- 수축 박스 종료가 전체 관련 Effect 종료는 아니다. 별도 context의 scene-color distortion mesh/particle tail을 실제 frame의 owner/element ID로 찾아 종료해야 한다. 캡처 SRV를 임의로 지우거나 암전 Profile을 덧대지 않는다. 전후 sample은 수축 종료 직후와 다음 WORLD 시작 전을 모두 포함한다.
- batch self-motion은 현재 runtime visibility를 보존해야 한다. 저장된 placement.visible로 instance 전체를 재구성하면 Level/Sequence가 숨긴 배치가 매 프레임 되살아난다. transform owner가 visibility owner를 덮지 않게 한다.
- 섬광 carrier의 첫 opacity/color key를 pre-roll 전체에서 평가하면 화면 밖 보관용 plane이 다른 카메라에서 드러날 수 있다. 실제 source activation/visibility/첫 flash 이동을 확인해 활성 구간만 좁히고 후반 source clock과 정상 carrier를 보존한다.
- book/map material 이름과 파일 존재만으로 복원 여부를 판정하지 않는다. 원본 MIC static switch와 실제 skeletal/static geometry 입력을 비교하고, 미지원 COLOR0와 해당 asset의 실제 색 손실을 구분한다.

### 본 그룹 위치와 총구 WORLD 위치의 좌표계

원본 V1의 Element를 본별로 묶을 때 본 이름만 같다고 동일 좌표계로 간주하지 않는다. follow/orientation, model cue, runtime anchor slot, socket basis와 transform owner를 함께 구분한다. 공통 local translation은 기존 S*R*T 뒤 본으로 전달되며, position lerp는 시작과 끝에 같은 delta를 더한다. source track과 master inheritance는 별도 owner다. 파생 한 손 WORLD용 총구 좌표를 원본 양손 본-local 위치에 그대로 넣지 않는다. Effect Tool Model Reference는 source actor/animation을 유지하고, source Effect의 정확한 사용 관계에서 지원 본 소품을 가진 유일한 Pattern을 Play All/Play Group에 자동 참조한다. 여러 후보는 명시 Pattern 선택을 요구한다. 양손 내부 본을 앞서 선택한 한 손의 단일 총 WORLD root에 중첩하지 않는다. [본 그룹 구현 결과](09-11/2026-09-11_EFFECT_TOOL_SOLO_AND_SELECTED_GROUP_PLAYBACK_RESULT.md).


### Effect 숫자 입력과 미설치 capture 후보

- bundled ImGui InputScalar/InputFloat 계열에는 EnterReturnsTrue를 전달하지 않는다. 입력창을 그리는 즉시 assert한다. 입력값 변경 반환으로 기존 stage/commit을 연결하거나 실제 지속 draft와 편집 종료를 함께 설계한다. InputText의 Enter 계약과 혼동하지 않는다.
- capture/왜곡 수정은 EXE 갱신만으로 적용되지 않는다. 후보의 sourceWritten·신규asset 존재·실제 occurrence 끝·게시 revision을 따로 확인한다. 삭제된 context를복구할 때는 원문복원과early/late 소유분할을 같은CAS에서 적용해야 잔류왜곡을 되살리지 않는다.
- 거절 경로 회귀검사는 호출 직전 실제상태와 비교한다. Mario Update가 현재위치의lane에재진입할 수 있으므로 이전에대입한NORMAL/0이 그대로라는가정을 두지 않는다. 구체근거는09-14 Sequence G24 RESULT에 둔다.


- MissileDrop 같은 nested RawDistribution의 Distribution=None을 숫자0으로 해석하지 않는다. 해당 module의 Engine CDO lookup을 확인한다. LocationDirect.ScaleFactor가 누락되면 경로 전체가0배가 되고, StartSize가 누락되면 generic fallback 크기로 줄어든다. 원본 분포 복구와 사용자 TRS 확대는 구분한다. 낙하 trail은 spawn-only 위치 복사와 매프레임 Direct 위치 복사의 소비자를 따로 검사한다. [공 낙하 G11 결과](09-13/2026-09-13_KOUKU_PATTERN_RADIAL_MOTION_RESULT.md)를 따른다.


### 2026-09-14: Ribbon·socket·nested CDO 확인

- Ribbon CPU point PASS 뒤에도 원본 VS의 UV 네 성분과 PS의 zw 소비를 확인한다. WaterRibbon UV1이0이면 edge mask도0이다.
- source socket의 cm→m 외에 설치 WModel 본 basis를 검증한다. 폭탄 심지 FX_01의 양의 source Z를 설치 b_body에 그대로 더하면 아래로 간다.
- Distribution=None만 저장된 RawDistribution은 원본 archetype/CDO cooked table을 확인한다. exact occurrence의 빈 필드만 복구하고 기존 저작 분포를 보존한다.
- 원리와 검증 경계는 [렌더링이펙트복원V2.md](렌더링이펙트복원V2.md)의 같은 날짜 항목, 개별 증거는 09-13 KOUKU_PATTERN_RADIAL_MOTION RESULT를 따른다.

- Composition Effect 그룹의 공간 이동에서 MAP는 고정 세계좌표라 멤버 시작 시간이 달라도 공통 delta를 적용할 수 있다. BOSS/WORLD는 실제 같은 anchor/bone/occurrence/emission/Follow 기준을 검사하며, frozen이면 시작 시점도 같아야 한다. 모든 후보를 먼저 검증하고 기존 staged geometry를 함께 갱신한다. 같은 시각 그룹 복제는 새 occurrence/group ID와 현재 미저장 위치를 함께 복사하고, 이미 실행 중인 preview의 문서 snapshot에도 새 ID를 다시 admit해야 한다. Pattern ID만 같다고 이전 snapshot을 재사용하면 복제본의 위치 편집이 무시된다. Save는 기존 Save_Atomic을 사용하며 이 editor metadata를 새 runtime 부모 Transform으로 소비하지 않는다.


### 새 Effect 목록 발견과 Composition 재생 catalog는 별도다

새 Authored의 metadata Discover/Refresh 성공을 runtime catalog Load 완료로 보지 않는다.
실행 중 Composition의 미저장 편집을 보존하고 외부 등록은 Authored·Catalog·Tree 범위에 둔다.
현재 Composition의 신규 asset 재생은 사용자 저장 후 Client 재시작으로 확인한다.
선택 clip에 사라짐 notify가 없으면 바로 앞 clip의 HidePawn·Light·particle 시각을 함께 찾는다.
root snapshot과 bone follow는 서로 다른 공간이므로 각 notify의 slot·socket·scale을 보존한다.
원본 앞 clip의 이펙트를 선택 clip root에 임의 중첩하지 않는다.
[쇼타임 사라지기 G17 결과](09-12/2026-09-12_KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT.md#g17-쇼타임-쿠크세이튼-사라지기-독립-effect-설치--2026-09-14).

### FixArea 예고는 필드 이름·영역 소비자·발생 단위로 읽는다

직사각형 AreaAngle은 기존 HitAreaWire의 halfWidth 계산과 역변환을 대조해 전체 폭인지
확인한다. 반폭으로 가정하거나 screenshot 비율로 크기를 정하지 않는다. FixArea footer의
시간은 원본 ScriptStruct의 필드 이름·타입·연결 순서로 읽는다. 고정 `len-164`는 다른
Timer의 sound 시각일 수 있으므로 DecalFillTime·Duration 대신 사용하지 않는다.
같은 particle이 세 번 보이면 timer·TRS·판정 영역으로 별도 source occurrence인지 확인하고
ID·상대 시작·위치를 각각 보존한다. emitter loop 변경으로 서로 다른 발생을 합치지 않는다.
독립 복제로 element ID를 새로 발급할 때는 기존 portable-copy의 `authored-copy:<원본 ID>`도
유지해 원본 RNG identity가 바뀌지 않게 한다. ID 고유성 성공만으로 particle 분포 보존을 판정하지 않는다.
Append의 기본 길이도 Detail 수명 합산 대신 기존 Playback의 source particle tail 계산을 재사용하며 이미 저장된 사용자 occurrence 길이는 보존한다.
원본 named timing과 프로젝트 보간 투영은 구분한다. 구체 수치는
[사각형 장판 G18 결과](09-12/2026-09-12_KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT.md#g18-쇼타임-사각형-예고와-3회-공습-폭발-등록--2026-09-14)에 둔다.

### 원본 재질 전환 시 생성 검사와 활성 후처리도 함께 확인한다

- Deploy의 native surface가 legacy 발광 overlay를 건너뛰면 Initialize도 같은 family를 기준으로
  검사한다. 원본에 없는 EMISSIVE texture를 필수로 요구하면 첫 바닥에서 Level staging이 실패한다.
  파괴 바닥의 overlay flag는 Map Effect owner 계약이므로 입장 우회를 위해 끄지 않는다.
- 원본 tone/LUT의 수치 일치는 현재 맵의 완성된 조명 출력과 화면 일치를 뜻하지 않는다. 기존
  활성 맵 profile의 노출·Bloom·gamma·region까지 한 번에 대체하지 않는다. 밝기 회귀는 이전
  활성 profile로 복구하고 source 비교용 profile과 재질·geometry 복원은 분리해 유지한다.

### 선택 바닥의 직접 반사와 광원 입력

쿠크 FLOOR08/FLOOR08A(marker1)의 RGB reflectance는 MapLight의 diffuse radiance로
조명한다. MapLight의 legacy specular는0이므로 이를 곱하면 재질이 정상이어도 직접
반사가 사라진다. marker0/2의 legacy specular 설정과 다른 native family는 유지한다.
원본 Phong/Blinn 차이, base-color reflection과 직접 반사, 사용자의 화면 판정은 구분한다.
근거: [3관문 조사와 반영 범위](09-15/2026-09-15_MAP_AND_VALTAN_FULL_RESTORATION_RESULT.md#G08).

### Typed Effect Lights는 현재 맵 광원도 차단한다

Rendering Workbench의 Typed Effect Lights는 Presentation_Manager의 전체 transient
광원 스위치다. 맵 SPOT도 같은 큐를 사용하므로 off이면 maplights의 enabled가 true여도
0개가 제출된다. 어두운 3관문에서는 바닥 기본색까지 사라질 수 있다. MapLight의
submitted 상태 문자열은 S_FALSE suppression을 구분하지 않으므로 실제 전체 Light count와
스위치 상태를 대조한다. 이 경로의 존재와 사용자의 실제 발생 원인 확인은 구분한다.

### Typed Effect Lights는 현재 맵 광원에도 적용됨

Rendering Workbench의 Typed Effect Lights는 CPresentation_Manager의 transient
light 제출 전체를 제어한다. CMapLightPresentationRuntime의 저작 맵 Point/Spot도
같은 Add_TransientLight를 사용하므로 OFF에서 함께 억제된다. 방향광/환경광이0인
쿠크3관문에서는 유일한 SPOT까지 사라져 바닥 재질이 없어진 듯 보인다. 텍스처나
크기를 되돌리기 전에 이 체크와 Last submitted Light를 확인한다. Selected Reference
A/B Start와 RenderingProfiles Save/Publish는 이 메모리 토글을 복구하지 않는다.
반사 입력0 결함과 전체 광원 억제 증상을 구분한다.

### Effect 비동기 완료 결과에서 YIELDED를 FIFO 불일치로 취급하지 않는다

Enqueue/Enqueue_Priority는 다른 target이 Loading owner에서 준비 중이어도 새 target을
추가하면 다음 frame 양보를 요청한다. 완료 결과를 먼저 Pop한 다음 Begin_LoadingFrame의
YIELDED/빈 ID를 identity 실패로 처리하면 정상 target이 영구 FAILED receipt로 남는다.
Advance_LoadingProductCuePreparation은 결과가 있을 때 pacing gate를 먼저 확인하고
YIELDED면 mailbox payload를 유지한다. READY의 exact ID/epoch/revision 검사는 유지하고
EPOCH_STAGE_COMPLETE는 마지막 target 소진 후 IDLE에서도 소비한다. 사용자 저장
Composition/Effect 손상으로 오진하거나 fail-closed 검사를 삭제해 우회하지 않는다.

### 쿠크 GroundEffect의 actor 수신과 source GBuffer row

- 깊이6m와 upward cutoff만으로는 보스의 위쪽 표면을 제외할 수 없다. 원본 projector 깊이나 장판색을 먼저 줄이지 말고 실제 receiver 경로와 Light 요소 유무를 구분한다.
- marker5의 depth.z는 source program이 아니라 프레임 material row다. Decal은 RGBA32_FLOAT PickPos.W low8 mantissa의 program ID를 정확한 Load로 읽는다. Static shadow exponent·bit22와 non-marker5의 map normal payload를 침범하지 않는다.
- native3600/3601/3602/3607의 program21/26 제외는 확인된 재질군 정책이며 범용 actor mask가 아니다. 같은 program의 다른 monster에도 적용되므로 Map/Actor catalog 실측과 Picking·Deferred reader 검사를 함께 유지한다. [수신 수정 결과](09-12/2026-09-12_KOUKU_SHOWTIME_WARNING_GROUPS_IMPLEMENTATION_RESULT.md#g03-노란-예고의-쿠크-표면-수신-제외--2026-09-15).


### 원본 overlay의 일반 광택과 추가 반사광은 독립 입력

`use_specular=false`라도 `use_subspecular=true`이면 specular texture/color를 제거하면 안 된다.
subspecular의 view dot은 원본 비정규화 mixed normal을 소비한다. 또한 방향 기반 overlay의
specular half-dot과 vertex-paint overlay의 half-dot은 서로 다른 normal을 사용할 수 있으므로
기존 floor 수식을 전역 교체하지 말고 exact native static permutation을 확인한다. Map parser의
새 family 입력을 허용했다면 Engine Model의 override 검증과 Material texture whitelist, static과
instanced binder/Deferred까지 함께 대조한다. parser 통과만으로 제품 지원을 판정하지 않는다.


### 명시 PBR 입력과 텍스처의 실제 파일 형식을 함께 검사한다

원본 material을 명시 surface 입력으로 교체했으면 Model override와 Material 생성도
같은 입력을 검사한다. WModel dummy 기본 normal이 비어 있다는 이유만으로 정상
PBR을 거부하거나, 우회용 dummy texture를 채우지 않는다. 실제 PBR D/N/detail/ORM,
Resources 경계와 기존 legacy 입력 검사는 유지한다.

확장자를 .dds로 바꾸는 것은 DDS 변환이 아니다. UModel이 TGA로 출력한 작은 기본
normal/white texture와 일반 normal도 실제 magic/decoder를 확인하고 픽셀을 보존해
변환한다. exists/hash만으로 설치 성공을 기록하지 않고, 실제 CModel 및 material variant
생성으로 해당 로딩 범위 전체를 확인한다. [Character Select 로딩 교정 G11](09-15/2026-09-15_MAP_AND_VALTAN_FULL_RESTORATION_RESULT.md#g11-character-select-로더-23127과-후속-재질-로딩-오류)에 원인과 검증을 기록했다.

### 원본 낙하와 지속 방패를 정지 ring 하나로 바꿀 때

동일 mesh/material의 낙하와 착지 이후 발생이 따로 있으면 처음부터 정지 ring을 켜지 않는다. 기존 stable ID·사용자 반경/수량은 보존하고 실제 source start·낙하곡선과 persistent span을 연결한다. 두 발생을 하나로 결합한 clock remap과 default adapter는 source raw값과 구분한다. 빈 LocationDirect ScaleFactor를0으로 곱해 원본 낙하를 지워서는 안 된다. camera bUseLocalSpace와 월드 잔상의 사용자선택을 구분하며 한 스킬의 모든 world-space 입자를 FOLLOW로 바꾸지 않는다. [워로드 Alt V G16](09-09/2026-09-09_WARLORD_ASVF_FULL_RESTORE_IMPLEMENTATION_RESULT.md#g16-2026-09-15-alt-v-배경낙하번개-재검토)을 따른다.

### Matinee의 Director 반환과 카메라 거리 조절

원본 Director track의 자기 Director group cut은 플레이어 시점 반환이다. 없는 CameraActor로
처리하거나 마지막 cinematic shot을 끝까지 늘리지 않고 해당 시간 구간을 비워 반환한다.
상속된 빈 cut track도 정상적인 무카메라 구간이다. 기존 scene 결과 보존과 shot key/clock 검사를
함께 한다. Follow Camera 거리를 조절할 때는 주시점을 유지한 채 실제 eye offset을 바꾼다.
거리 숫자만 변경하거나 같은 시점에 FOV·캐릭터 배율까지 바꾸면 거리 조절의 결과를 분리할 수 없다.
[발탄 카메라 결과](09-15/2026-09-15_MAP_AND_VALTAN_FULL_RESTORATION_RESULT.md#g14-v-원본-발탄-컷씬-카메라의-현재-준비-범위).

### 본체가 있는 이펙트의 모델 생성과 GBuffer 제출

심지·총구만 있는 순수 ParticleSystem에는 NPC/무기 본체가 포함되지 않을 수 있다.
기존 WorldObject가 본체를 생성해 쓰는 leaf에 모델을 추가하지 말고 실제 생성 owner와
독립 preview를 구분한다. ModelCue는 실제 설치 본·소켓·preScale·NPC 크기를 한 번만
소비하도록 측정하며 개별 emitter isolation이 standalone 모델을 숨기는 정책도 확인한다.

CModel 기반 OPAQUE/MASKED cue는 명시 Effect material이 없을 때 GBuffer를 쓴다.
캐릭터 shader의 opaque pass를 post-light Effect BLEND에서 호출하면 원본 material을
로드해도 scene lighting을 올바르게 받지 못한다. NONBLEND 제출 조건과 모델 draw의
분류를 같은 함수로 맞추고 투명 pass4,명시 Effect native7/8,기존 exact T11을 보존한다.
[쇼타임 폭탄과 분류 검사](09-11/2026-09-11_KOUKU_SHOWTIME_RESOURCE_INVENTORY_RESULT.md#15-09-15-해골폭탄-본체심지-및-cmodel-cue-조명-연결)에 현재 candidate/소스/빌드 경계를 기록했다.

### Native Modulate·collision event·baked history의 반복 방지

- 원본 Modulate의 alpha 의미는 PS 계산과 blend state를 함께 대조한다. RGB factor에 opacity가 이미 반영된 원본을 Alpha/V2 Multiply로 합성하지 않는다. 원본 출력 RT별 write를 확인하고 emission gain·bloom을 자동 적용하지 않는다. 기존 profile ID는 append-only로 보존한다.
- FreezeMovement는 위치뿐 아니라 회전과 orbit 이동도 멈춘다. direct-location update 뒤에도 동결 위치를 유지하고 색·크기·수명은 계속 진행해야 한다. 실제 world hit가 없는 synthetic particle count를 collision 성공으로 기록하지 않는다.
- Collision Event의 generator/receiver는 기존 bounded queue와 원본 접촉점을 공유한다. 같은 source PS를 여러 팔/owner에 복제할 때 event 이름을 source occurrence별로 분리한다. 미지원 상속·first/last-only 옵션을 묵살하지 않는다.
- baked history의 notify duration이 마지막 source sample보다 길면 실제 sample 끝까지 clamp한다. history ID 정렬 계약도 유지하며 원본 폭 0을 임의로 늘리지 않는다.
- native table을 추가한 뒤 실제 codec 검증은 해당 table을 소비하는 OBJ까지 재컴파일한다. 오래된 OBJ에서 생긴 metadata 오류를 validator 완화로 숨기지 않는다. NULL texture는 같은 MIC의 serialized referenced-texture index를 검증하며 parent 유사재질로 추측하지 않는다.
- 검증용 full restore catalog와 제품 cue는 별도다. 실행 중 저작 draft/freshness를 보존하고 승격 보류 요청 뒤에는 생성 문서·native 지원을 검증한 것을 전투 연결 완료로 기록하지 않는다.
- All Effects의 패턴 아래 `[FULL RESTORE]`는 원본 action별 복원 문서를 여는 로컬 preview다. source stage 번호를 제품 occurrence 번호로 취급하거나 PRODUCT cue로 자동 승격하지 않는다. 표시·검색은 기존 exact authored index를 사용하고 선택 시 기존 Product unlink 선택을 해제한다.

현재 발탄의 실제 130문서 admission, PhysX 74검사, D3D 116검사와 제품 cue 0의 근거는
[09-15 결과 G15-V](09-15/2026-09-15_MAP_AND_VALTAN_FULL_RESTORATION_RESULT.md#g15-v-원본-modulate-포털과-source-충돌-event-검증)에 있다.


### 맵 데이터 존재와 제품 로딩 범위를 따로 검증한다

- 원격 청크·장식이 Authoring/runtime placement에 있어도 `CLevelRegistry::MapLoadScope`가
  제외하면 일반 Level에서 생성되지 않는다. 카메라 이동과 frustum culling 검사는 이 누락을
  해결하지 못한다. Loader의 모델 집합과 Level의 배치 집합이 같은 범위를 쓰는지 대조한다.
- 전체 복원·전체 맵 요청에서는 전체 배치 수와 실제 로드 대상 수, 제외 stable ID와 이유를
  확인한다. Map Editor 전체 Area에서 찾은 객체를 제품 입장에도 있다고 보고하지 않는다.
- geometry·재질·placementLighting 보존과 실제 제품 대상 포함, EXE 배포, 사용자 화면 확인을
  구분한다. [맵·발탄 결과 G13-CS](09-15/2026-09-15_MAP_AND_VALTAN_FULL_RESTORATION_RESULT.md#g13-cs-character-select-전체-area-배치-로딩)를 따른다.

### 같은 Effect의 카메라와 본 배율을 한꺼번에 정규화하지 않는다

- 원본 StartSize가 맞아도 설치 combined bone의0.01과 메시 cm→m가 중복되면 실제 표시가100배 작아진다. actual WModel/clip의 basis와 최종 particle 행렬을 함께 측정한다.
- 같은 document에 본·카메라·발 anchor가 섞여 있으면 확인한 runtime anchor만 기존 정규화에 연결한다. 제품과 Tool은 같은 document/anchor 판단을 사용한다. slot으로 중복 제거하는 collector에 같은 slot의 상충하는 정책을 넣지 않는다.
- 설치 CSO hash가 재컴파일과 다르면 실행 코드와 디버그 정보의 차이를 분리한다. Artist512는 debug chunk만 달랐으며 나비 alpha 수정은 이미 배포돼 있었다. 픽셀 생존과 실제 배율·위치 문제를 서로 대신하지 않는다.
- 상세 실측과 남은 화면 확인은 [도화가 결과 G13](09-09/2026-09-09_ARTIST_CORE_FULL_RESTORE_IMPLEMENTATION_RESULT.md#g13-09-15-나비-실제-골격-배율과-solo-검토)에 기록한다.

### 방향성 장판은 생성 중심과 실제 quad의 방향을 함께 확인한다

원본 notify가60도 간격이어도 변환에서 FRotator가 누락되거나 axis-locked sprite가 emitter 회전을 방향에 소비하지 않으면 겹치거나 같은 방향으로 그려진다. 기존 source TRS와 billboard roll의 실제 소비를 대조하고 발생 수를 늘리지 않는다. 원본 각도·배치와 사용자 카메라/반경/높이 튜닝은 구분한다. 실제 모델 bounds의 시야 포함과 띠 장축·법선 검증은 [워로드 결과 G18](09-09/2026-09-09_WARLORD_ASVF_FULL_RESTORE_IMPLEMENTATION_RESULT.md)에 둔다.


### 별도 조각의 원본 조명은 native component payload까지 확인한다

- tagged properties에 lightEnvironment가 없다고 lightmap이 없는 것으로 판정하지 않는다.
  StaticMeshComponent의 native suffix를 끝까지 읽고 LightMapType, texture refs, GUID,
  UV scale/bias와 배치별 RGB scale을 회수한다. 별 같은 작은 조각도 RNM 영역을 가진다.
- material shader cache에 NoLightMap 프로그램이 있다는 사실은 해당 배치가 그 프로그램을
  사용한다는 증거가 아니다. 실제 component LightMapType에 맞는 shader permutation을 비교한다.
- CModel 생성 PASS는 파일·리소스 준비 성공이다. 누락된 RNM/환경 입력과 잘못된 조명 수식,
  검은 재질까지 검사했다고 보고하지 않는다. 원본 비교 shader fixture의 동일 입력과 실제
  원본 엔진이 공급한 입력의 동일성을 구분한다.
- 사용자 빌드와 에이전트 빌드는 따로 추적한다. 에이전트가 링크하지 않았더라도 사용자가
  중간에 빌드해 변경이 EXE에 들어갈 수 있다. 소스 보류·복귀 전에 OBJ의 source checksum,
  실제 링크 입력과 EXE 시각을 확인해 다음 빌드에서 이미 사용한 기능이 사라지지 않게 한다.
- diffuseBrightness0인 원본 PBR에서 환경 cube/lookup이 빠지면 RNM이 있어도 넓은 면은
  검정이고 일부 2D reflection만 점처럼 남을 수 있다. 실제 texture/UV 표본에서 RNM과
  환경 기여를 분리해 검사하며 원본 brightness를 임의로 올려 누락을 숨기지 않는다.
- 멀리 떨어진 조립체를 가까이 전시할 때는 pair에 같은 평행이동을 적용해 상대 높이와
  개별 lightmap tile을 보존한다. 하늘 구체의 큰 AABB 안에 있다는 사실과 shell 표면에
  겹치거나 가려진다는 사실을 구분한다. 현재 정적 tile을 새 위치의 조명 bake로 부르지 않는다.

### 원격 바닥의 축소 mip와 교체 preview

- LV_MODULE/nav/water/FX 이름 분류를 visibility로 사용하지 않는다. 실제 원판488도 이
  규칙으로 숨겨졌다. source actor/component의 HiddenGame/bHidden/bVisible과 archetype/CDO
  근거를 보존하고 navigation 참여와 분리한다. source schema3와 공용 scene 계약을 사용한다.
- 원판·별 조립체 전체를 올리면 더 위에 있어야 하는 큰 장식 링을 원판이 가릴 수 있다.
  같은 XZ의 실제 삼각형 높이를 비교하고 원판의 원래 층과 별의 bridge clearance를 분리한다.
  bounds 높이만으로 최소 offset을 정하지 않는다.
- 원본 MIC의 밝기0과 missing default는 다르다. 원본 uniform expression/DXBC에서 실제로
  밝기0을 소비하면 환경 cube를 연결해도 diffuse가 복구되지는 않는다. 진단용 기본값1은
  별도 preview variant로 대조한다. 사용자 요청으로 저작값을 보정할 때도 원본0을 보존하고 프로젝트 보정으로 기록하며 원작 실행 중 값으로 주장하지 않는다.
- 공유 source material compiler의 coverage는 slot/MIC/정확한 field 단위다. 모든 scalar나
  renderFlag를 일괄 지원 처리하지 않는다. PBR normalmap OFF처럼
  현재 carrier와 다른 active branch는 명시적으로 거부한다. RNM 색공간, normal linear,
  DDS 전체 payload/mip/hash와 runtime scalar 범위를 모델 생성 전에 확인한다. PBR steady는 명시 mode=none으로 연결하고 기존 mode 부재/0 descriptor의 nested와 BG0/1/2를 보존한다.

- DDS는 원본 mip0만 추출돼도 생성에 성공한다. 먼 거리에서 반사점이 모자이크처럼
  보이면 설치 DDS, 실제 texture/SRV의 mip 수, sampler와 설치 CSO를 각각 대조한다.
  같은 원본 mip0와 decode 경로를 먼저 확인한 뒤 하위 mip을 회수한다. 범용 Crunch의
  CRC 성공만으로 LostArk 원본 BC 데이터 일치를 주장하지 않는다.
- 같은 geometry/MIC를 사용하는 배치도 RNM atlas/UV/RGB scale이 다를 수 있다.
  교체할 때 record의 bakedLighting을 통째로 보존하고 원판과 별에 같은 조립체 변환을
  적용한다. 중앙 환경 선택은 새 위치에서 조명을 다시 굽는 기능이 아니다.
- 현재 map cube 방향식에서 source-local 환경을 유지하는 yaw 보정은 기존 각도에서
  조립체 yaw를 빼는 것이다. 원본 native binder의 규약까지 같은 것으로 일반화하지 않는다.
- preview material을 분리할 때 empty override를 새 variant로 전달하지 않는다. Layer
  삽입 뒤 던질 수 있는 entry/status 할당은 stage 전에 끝내고, 생성·visibility 실패와
  원복을 검사한다. 함수 소비자 stub 검사와 실제 CModel/GPU 생성 검사를 구분한다.

현재 구현·원본 mip 설치와 남은 화면 경계는
[맵 복원 결과 G19-CS](09-15/2026-09-15_MAP_AND_VALTAN_FULL_RESTORATION_RESULT.md#g19-cs-중앙-바닥-교체-ui와-원본-축소-mip-복구)에 둔다.

### Composition 저장 충돌과 서버 제어 Effect template

- 실행 중 외부 등록으로 Composition의 Pattern/World가 바뀌면 기존 편집기의 Save는
  `composition changed before save`로 거부된다. 미저장 draft가 JSON에 있다는 뜻이 아니다.
  baseline·등록본을 보존하고 호환되는 append-only resource 상태로만 복구한 뒤 사용자가
  실제 Save했는지 revision/신규 Logic/occurrence 시간을 재확인한다. freshness 검사는 유지한다.
- Server가 반복 생성하는 그룹은 원본 occurrence를 동시에 static 재생하지 않는다. 같은
  시각자료·상대 시간/TRS를 template로 소비하며, 별도 CombatObject ID로 플레이어별 수명을
  관리한다. 부모 반복 확장 시 fixed group·tracking reference를 함께 scope-remap하고,
  template가 잘릴 경우 수명이나 상대 시간을 조용히 바꾸지 않고 publish를 거부한다.
- 전용 CombatObject의 started marker는 기존 live identity/revision 검증 후 consume해야 한다.
  실제 시각 생성을 이미 spawn에서 수행했다면 marker로 같은 이펙트를 중복 생성하지 않는다.

- Encounter Pattern에 optional lane을 추가할 때 Gameplay 발행기와 함께 WorldPipeline의
  Get-EncounterProfiles strict allowed-key도 확인한다. World는 해당 보스의 optional 배열
  형태만 검증하고 실제 값·Client template join 계약은 Gameplay 발행기가 계속 소유한다.

### 노란 장판의 actor 수신과 실제 부채꼴 저작

- 같은 source material program이 움직이는 폭탄과 정적 Map에 쓰이면 program 전체를 차단하지 않는다. 실제 skinned writer의 표식과 receiver 소유 marker를 함께 확인한다. marker0/5의 bit8과 source program low8은 RGBA32_FLOAT로 왕복 검사하고, 다른 Map marker의 normal/RNM·shadow payload를 actor 표식으로 해석하지 않는다.
- shader가 지원하는 inner 채움 수식이 해당 원본 occurrence에서 사용됐다는 뜻은 아니다. 사용자가 채움이 없음을 확인했다면 해당 occurrence의 시작값을 고정하고 inner track만 제거한다. 다른 원형·도넛의 채움까지 같은 보정을 적용하지 않는다.

### ModelCue가 있는 Effect의 수명 연장

- Composition duration, ModelCue visible duration, source emitter emission window와 개별
  particle tail을 각각 확인한다. holdLastFrame은 clip 마지막 자세만 유지하며 ModelCue의
  종료 시각을 자동 연장하지 않는다. 원본 낙하2초를 사용자 존재시간까지 느리게 늘리면 안 된다.
- ModelCue가 있는 문서는 level 소유 무한 source-loop 경로의 대상이 아니다. 특정 존재시간
  연장에는 해당 cue와 원본 loops0 emitter의 유한 배출 구간만 맞추고 원본 속도·개별 수명을
  보존한다. 끝의 폭발은 source notify의 hide-gap/TRS/parameter를 대조해 별도 occurrence로
  연결하며, 폭발 tail로 사용자가 저장한 몸체 존재시간을 덮어쓰지 않는다.


### Kouku Stage 길이와 빈 tail의 게시

Stage 길이 편집은 Effect/Logic/World 등 비애니메이션 행의 시작·길이·fade·TRS를 바꾸는 명령이 아니다. `Set_StageDuration → Extend_PatternLifetimeForAuthoredLanes`는 선택 animation window만 제한하고 모든 저작 행의 마지막 끝까지 explicit Pattern 수명을 연장한다. 이미 명시한 lifetime은 줄이지 않는다. ANIMATION_BLEND 절대 시간이 새 pose 경계와 충돌하면 전체 편집을 거절하며 Logic을 조용히 retime하지 않는다. 별도 staged geometry는 Transform만 합쳐야 하며 이전 timing으로 새 clock을 덮지 않는다.

Stage 합을 늘리는 Add Stage, clip append/bind, action/cinematic append와 Start Offset도 commit 전에 같은 lifetime 확장을 적용한다. explicit duration이 Stage 합과 같던 Pattern에서 Stage만 추가하면 `Explicit Pattern duration is shorter than its Stages`로 정상 입력까지 거절된다. implicit duration만 있는 새 Pattern의 추가 성공으로 이 경로를 검증했다고 처리하지 않는다. 기존 긴 tail과 비애니메이션 행을 보존하며 600000ms 상한·실패 rollback은 유지한다. 재현과 수정 증거는09-14 Kouku Sequence Implementation RESULT G45를 따른다.

Codec의 저작 admission과 Product 게시를 구분한다. 마지막 빈 Stage의 Preview는 직전 animation playMs 끝을 hold하며, 같은 kind·retarget 없는 leaf tail을 publisher 파생 사본에서 직전Stage duration으로 합쳐 기존 holdAtWindowEnd에 연결한다. implicit clock은 동일30Hz tick 합을 요구하고, explicit lifetime은 기존 fixedTimeline 절대 경계로 끝 pose를 유지한다. Source Stage 삭제·임의idle/clip 대입·Product one-clip검증 해제로 숨기지 않는다. 실제 prepare_publication의 Pattern 포함, targeted template refs, native root curve의 tail 정지까지 확인한다. 세부와 증거는09-14 Kouku Sequence Implementation RESULT의 G29/G30-P에 있다.


### Effect preparation failure는 owner 해제 전에 정산한다

runtime worker의 structural failure target을 pending 상태로 owner부터 해제하면 priority enqueue가 front를 바꾸고 뒤늦은 front-only failure receipt가 거절되어 revision 전체가 멈출 수 있다. known target은 소유 상태에서 먼저 terminal commit하고 해제한다. 실제 identity 손상은 same-revision/requested-pending blocking failure로 Preview에 전달하며 timeout으로 ready 처리하지 않는다. ready 분기는 과거 held 문구를 지우되 occurrence 오류를 덮지 않는다. 근거: `.md/GB/09-14/2026-09-14_KOUKU_SEQUENCE_PLAYBACK_EDITOR_IMPLEMENTATION_RESULT.md` G31.


### 쇼타임 랜덤 낙하의 MAP/BOSS 혼합 기준점 (2026-09-15)

- Gameplay bootstrap의 부모·자식 행은 생성 함수의 append 순서만 확인하면 안 된다. 최종 정렬 키에서 `PATTERNSHOWTIMETARGETS`를 같은 encounter/pattern/trigger의 `PATTERNSHOWTIMERANDOM`보다 먼저 두고 자식 ordinal은 숫자 순서로 보존한다. 생성물의 행 개수·내용 검증에 더해 실제 `CGameplayCatalog` 전체 로드를 확인한다. `Test-GameplayBootstrapRowOrder.ps1`이 최종 정렬 함수의 다중 소유자·32개 순번과 기존 의존 순서를 검사한다.

여러 발사가 묶인 selection group을 통째로 MAP 템플릿으로 바꾸면 총구의 BOSS-local 위치가 랜덤 바닥 위치로 이동한다. 반복할 세트는 stable occurrence ID로 명시하고 MAP 기준점은 첫 MAP 예고에서 구한다. 총구는 같은 CombatObject source entity의 실제 CNpc root/model, 바닥 효과는 Server가 정한 root를 기존 Sample 경로에 전달한다. 두 세션은 같은 Server 생성 clock을 사용하고 실패/종료/Reset 때 함께 정리한다. 원본 총구가 아직 준비되지 않았다면 MAP root를 총구의 임시 보스로 사용하지 않는다.

random pool은 기존 플레이어 위치 fixed/tracking에 추가하는 optional 계약이다. Source codec/projector/Gameplay publisher/Server parser의 pool 범위·필수 필드를 일치시키고, 내용이 같은 A/B 세트의 visual hash 중복은 순서 배열에서 보존한다. Duration 종료 후 새 생성을 중단해도 이미 생성한 폭발/장판은 자기 수명을 마친다. 샘플러의 exact walkable 중심 검사는 모든 이펙트 정점이 원형 바닥 내부라는 보장이 아니다.

### 독립 전기 그룹의 전방과 원본 notify 위치

원본 notify yaw의 중앙값만 보고 전체 leaf를 돌리지 않는다. 설치 mesh의 실제 끝점에 CPU particle World와 modelPreScale을 적용해 주축을 확인하고, 새 독립 그룹에 필요한 배우 basis만 한 번 합성한다. 같은 basis는 notify translation에도 적용해야 한다. 비대칭 FRotator 각도는 반올림하지 않고 source byte offset과 실수 변환을 함께 남긴다. 배우 전방을 따라야 하는 새 library resource는 BOSS anchor가 실제로 그 yaw를 소비하는지도 확인한다. 원본 leaf와 다른 소비자의 각도는 유지한다. 알비온 근거는09-14 Sequence Implementation RESULT G35에 있다.

- 생성 위치에 남아야 하는 전기 효과는 `detail.particle.localSpace`와 바깥 occurrence의 `followBoss`를 함께 확인한다. 입자를 world-space로 바꿔도 움직이는 root에서 후속 입자를 계속 생성할 수 있다. BOSS anchor는 생성 시 방향을 제공하고, 추적을 끈 occurrence는 그때 만든 root를 유지한다. 원본 Required 모듈의 literal은 추출 근거로 보존하며 요청된 runtime override와 구분한다.
- V1의 `groupId` 저장과 `Group Center`의 실제 그룹 키는 별도 소비자다. root-local 수동 그룹은 기존 `manual.*` ID를 중심 편집 키에 포함해야 독립 이동할 수 있다. source 자동 그룹과 본별 그룹의 기존 묶음, source track·inheritance·범위 검증을 유지한다. 세부 적용은09-14 Sequence Implementation RESULT G37에 기록한다.

### 공중 등장과 원본 착지 곡선

알비온 `_24_03`은 고정 공중 자세다. 클립의 Y 자체를 상승 곡선으로 오인하거나 모든 animation의 속도·수직 배율을 바꾸지 않는다. 기존 후속 낙하로 계산한 정점 높이와 TRIGGER의 상승 시간을 분리하고, stage·animation clock은 유지한다. 순간이동은 기존 root 원점과 navigation 지면을 함께 갱신해야 다음 sample에서 돌아가지 않는다. SLAM은 trigger부터 현재 시각까지 원본 곡선의 누적 최저값을 사용한다. fixed tick이 실제 최저점을 건너뛰거나 원본 끝에서 미세한 반등이 있어도 지면에 머물러야 하며, Preview seek도 과거 프레임 상태 없이 같은 결과를 낸다.

플레이어 선택은 ID를 고정하고 등장은 해당 시점의 위치를 읽는다. 두 trigger를 같은 시각에만 시험하면 오래된 위치에 등장하는 결함을 놓친다. Preview는 등장 시각의 복제 위치를 별도로 고정하고 Server는 실제 살아 있는 플레이어·navigation·body collision을 검증한다. supplemental bootstrap 행은 해당 부모 뒤로 정렬하고 실제 전체 Catalog 로드를 확인한다.

Composition 외부 변경 때문에 Save가 충돌했다면 미저장 draft를 Reload로 버리거나 freshness 검사를 해제하지 않는다. 자기 변경의 before/after bytes가 확정된 경우 해당 field와 revision만 CAS 역변경해 저장 기준을 복구하고 사용자 Save 성공 후 최신본에 다시 적용할 수 있다. 이후 사용자 저장이 계속되면 예전 후보 설치는 거절하고 최신 편집을 보존해 다시 준비한다. 실제 사례는09-14 Sequence Implementation RESULT G38이다.

### 이전 Client와 새 Composition 필드, Object Save의 선행 저장

`Logic definition has unexpected properties`는 이름이나 로직 생성 자체가 아니라 실행 중인 codec이 모르는 JSON 필드일 수 있다. 새 typed Logic 필드를 설치하기 전에 기존 Client의 사용자 편집을 저장해야 한다. 이미 충돌하면 자기 변경의 before/after를 확인해 writer lock + CAS로 역변경하고, 사용자 Save 뒤 새 정의·배치 개수를 실파일에서 읽어 보존한다. 새 후보는 최신 저장본에 합친다. 역변경한 필드의 runtime 재게시나 완료를 임시 복구와 혼동하지 않는다.

`Is_Dirty || Is_PublishRunning`을 한 안내로 표시하면 사용자가 불필요하게 Publish를 누를 수 있다. 새 Logic 정의만 생성해도 dirty이며, 패턴에 배치했는지는 이 검사와 무관하다. 미저장 owner의 Save와 이미 실행 중인 Publish 대기를 각각 안내한다. 근거와 저장 복구 기록은 09-12 World Object Group RESULT의 마리오 Collider 절에 둔다.

### Effect 그룹 중심 회전과 쿠크 분신 행동 소유

Effect Tool 그룹 회전은 같은 anchor 안에서 공통 quaternion delta로 중심 기준 위치·방향과 선형 이동 끝점·속도를 함께 변환한다. 저장 기준은 각 Element Transform이며 별도 누적 UI 각도를 정본으로 만들지 않는다. 원본 회전 animation/revolution owner가 있으면 해당 편집을 거절한다. 실제 helper/codec 검사와 현재 커서 preview 갱신, 사용자 화면 판정을 구분한다.

네 방향 Pattern은 한 보스의 동시 action으로 겹쳐 넣지 않는다. CROSS_DIRECTION_CLONES는 parent clock을 보존한 채 선택된 child만 본체를 소유하고 나머지는 dependent Summon이다. 생성 admission의 ownerRunsFinale, 부모 종료 정리, cutoff deadline, Client child animation/Effect snapshot까지 연결한다. Summon 이름만 있는 박스는 네 방향을 추측하지 않는다. Summon definition의 CROSS_DIRECTION_CLONES 정책과 네 방향/종료 Stage 설정을 occurrence 하나가 실행하며, 이전 typed Logic의 summonOccurrenceId 연결도 같은 실행 window로 해석한다. No child Patterns 안내를 Play 실패로 혼동하지 말고 typed Summon 실행 여부를 확인한다. 여러 Parent의 재사용은 가능하나 기존 Animation host와 공유한 이름뿐인 Summon 정의를 일괄 typed로 바꾸지 않는다. 늦은 tick에 이미 끝난 duration을 새 분신 생성으로 되살리지 않는다. 실제 root 이동 곡선을 사용하고 전방/후방 이름을 임의 좌표축에 대응시키지 않는다.


### Play의 Logic 공간 샘플과 Local Space만 수정하는 후보

BOSS_TELEPORT_XZ는 이름이 아니라 typed 정의와 발생 시각으로 재생한다. Preview에서 목적지만 더하면 다음 root 샘플에 원위치로 돌아가므로 destination + D(t) - D(trigger)를 사용하고 animation Y·yaw를 유지한다. Effect 발생 위치와 particle birth도 같은 source clock sampler를 사용해야 짧은 순간이동의 보간과 첫 seek의 잘못된 frozen pivot을 피한다. 플레이어 추적은 ID만 고정하고 현재 위치를 계속 읽되 되감기는 이미 기록한 입력을 사용한다.

Local Space 필드만 수정할 때 JSON 전체 dump는 사용자 음수0(-0)을0으로 정규화할 수 있다. 실제 codec Serialize 비교가 차이를 검출한 경우 구조 비교만으로 동일하다고 처리하지 않는다. 최신 원본 bytes의 해당 boolean 토큰만 교체하고 source Required literal·사용자 TRS·그룹·시간이 모두 보존되는지 확인한다. 입자 Local Space 해제를 Composition BOSS follow 해제로 확장하지 않는다. 실제 범위와 수치 검사는09-14 Sequence Implementation RESULT G40/G41을 따른다.

### 발탄 Full Restore의 masked 재질·본 basis·별도 FRotator

원본 LocalVF masked PS가 CB0[0].rgba를 particle color/opacity로 읽으면 opacity X만1로
채우지 않는다. 실제 VS/PS와 unowned binding prefix를 확인하고 color RGBA를 전달한다.
정적 mesh carrier의 기본 dynamic0은 원본 dissolve/emission을 없앨 수 있으므로 실제
소비자가 source dynamic을 전달하는지 확인한다. native additive의 alpha0은 최종 dispatch의
opaqueCoverage까지 확인해야 하며 중간값만 보고 blend를 바꾸지 않는다.

arena snapshot의 크기1과 bone-follow의 실제 owner 배율은 별개다. 본 local notify와 socket의
좌표를 UE world축으로 다시 바꾸지 않는다. float 회전 필드가0이어도 같은 source payload의
별도 FRotator 정수가 유효할 수 있으므로 occurrence별 원본 단위·정확한 offset을 검증한다.
재질 이름에 의한 전체 회전/크기 보정은 금지한다. 직접 PlayDecalEffect는 particle 목록 밖에
있으므로 particle244/244 성공을 stage전체 복원 완료로 기록하지 않는다. source Anim 길이,
무조건 stage전환, 조건부 preview 범위와 NATURAL effect tail도 구분한다.
근거: 09-15 MAP_AND_VALTAN_FULL_RESTORATION_RESULT G25-V.

### Fixed-axis Sprite의 저작 회전

SourceRecipe axis-lock Sprite의 최종 billboard 면은 SourceTransformTrack이 없으면 Element/Group 회전을 소비하지 않을 수 있다. 선택적 sprite.followEmitterAxisRotation은 고정 축 Sprite에서 정규화한 emitter basis를 한 번 적용한다. Local Space는 현재 basis, World Space는 spawn 시점 basis다. 기본 false이며 camera/velocity billboard와 기존 Matinee/local 및 수동 billboard roll 보정을 전역 변경하지 않는다. source 좌표 변환을 재적용하거나 모든 Sprite billboard를 끄지 않는다. 새 bool이 기존 struct padding에 들어가도 구버전 OBJ 생성자는 초기화하지 않으므로 codec core와 소비자를 같은 헤더로 컴파일해 검사한다.

### 맵 shadow 비용과 Loader 완료 뒤 activation 실패

낮은 FPS를 복원 재질·입자 수만으로 추정하지 않는다. 유효 GPU scope와 누락 CPU 표본부터 구분한다. 완전 불투명 정적 맵 shadow는 검증된 source family/flags에서만 재질 바인딩과 pixel shader를 생략한다. masked/fade와 vertex 변형은 유지하고 실제 전체 shader의 depth·cull·basis parity로 검증한다. authored coverage를 실제 draw 절감이나 측정 FPS로 보고하지 않는다.

Loader가 effect 준비 실패를 격리해도 Level Initialize는 필수 ambient의 누락을 거절할 수 있다. `loading.complete`는 activation 요청 이름일 수 있으므로 실제 거절 단계의 상세 진단을 먼저 보존한다. bootstrap version뿐 아니라 행 수 상한도 Client·Server·publisher가 같은 Shared 계약을 소비해야 한다. 적용과 증거는09-15 MAP_AND_VALTAN_FULL_RESTORATION_RESULT G26을 따른다.

반복되는 정적 shadow geometry는 time-invariant depth 조건을 만족하는 batch만 캐시한다. owner/revision과 최종 light 행렬·source 모드가 모두 같아야 하며 화면 밖 caster도 light 범위 안이면 유지한다. weak owner의 control block까지 대조하고 scene replacement·실패·mutable morph는 기존 draw로 돌아간다. authored 적용 가능 개수와 실제 cache hit/FPS는 다르다. local light는 최종 감쇠0의 불필요한 재질 계산만 생략하고 출력 동일성을 확인한다. G27/G29가 해당 검증 근거다.

캐시 hit인데 shadow가 비싸면 미참여 batch와 개별 fallback을 구분한다. alpha-tested라는 이유만으로 매 프레임 변하는 것은 아니지만 BG parallax는 camera, panning/UV 이동은 time에 의존할 수 있다. 기존 alpha PS를 유지하고 해당 입력이 정적인 경우만 캐시한다. 외부 texture override는 내용 변이를 추적하지 않고 dynamic으로 제외한다. 개별 객체는 placement setter뿐 아니라 실제 Transform과 bounds·mesh별 cast/pass도 비교해야 하며, 배치 WorldInvTranspose 변경도 revision에 포함한다. G31은 이 누락과 후속 캡처를 다룬다.

Level 생성은 Change_Level 전이므로 ambient probe의 target level과 current LOADING이 다를 수 있다. probe만 현재 LOADING 소유로 잠깐 생성하고 모든 성공·실패 경로에서 제거한다. 실제 활성화 후 effect는 원래 target 소유를 유지한다. queued Spawn과 Spawn_Immediate의 SOURCE_LOOP owner 허용 조건이 다르면 첫 검사 수정 뒤 다음 단계에서 재거절된다. 두 경로를 함께 대조하고 active-level validation을 넓게 우회하지 않는다. Bern 직접 입장 identity는 pending 생성 우선, 이후 기존 created/audition을 사용하며 audition을 created로 commit하지 않는다. G28/G30에 구현 범위를 기록한다.

### 같은 animation의 동반 burst와 effect 수명

cast/shot의 emitter 수와 native shader 일치만으로 전체 폭발 복원을 판정하지 않는다. 동일 clip을 쓰는 source action들의 활성 notify를 비교하고, 조합 시 원래 action에서 비활성이던 system을 구분해 기록한다. notify emission 종료와 particle tail은 별개이며 Composition occurrence가 tail보다 짧으면 준비·재생 검사가 통과해도 화면에서 잘린다. 기존 사용자 TRS·시간을 보존하고 정확한 소비 occurrence의 수명만 수정한다. 작은 오망성의 근거는09-13 KOUKU_PATTERN_RADIAL_MOTION_RESULT G14다.

### Composition의 서로 다른 외부 수정과 미저장 draft

저장 기준본의 freshness를 없애는 대신 기준본·draft·디스크를 함께 비교한다. schema가 정한 stable ID 배열과 객체 필드는 겹치지 않는 변경만 병합하고, 같은 필드의 다른 값·삭제 대 수정·상충하는 순서는 경로와 함께 거절한다. 좌표·참조 순서 같은 비-ID 배열은 원자 값이다. revision은 최신 디스크 기준으로 한 번 증가하며 writer lock, temp validate/reopen와 byte CAS를 유지한다. 구버전 Client가 실행 중이면 새 소스만으로 이 정책이 적용되지 않는다. 열린 draft의 저장 복구는 자기 외부 변경의 exact before/after가 확인될 때만 역변경하고 사용자 Save 결과를 확인한다. 실행 중 정본을 반복 수정하지 않는다. 구현과 검증은09-14 Sequence RESULT G46을 따른다.

Parser가 invalid/orphan Pattern·Folder·Bundle을 원문 그대로 격리하는 문서에서는 Parse 성공만으로 병합을 승인하지 않는다. 각자 유효한 start와 duration도 합치면 window를 넘을 수 있다. 이미 격리된 동일 JSON만 보존하고, 병합 때문에 새로 격리된 항목은 실패로 처리한다.


### Source Sprite 단면·양면과 회전 옵션

화염링처럼 fixed-axis Sprite를 회전하면 기존 단면 back-face cull이 드러난다. 회전 오류와 컬링을 구분하고 Source material renderProfile/native descriptor를 임의 양면으로 바꿔 exact 검증을 깨지 않는다. 선택적 detail.sprite.twoSided는 기본false이며 검증된 Artist-registry native Alpha/Additive One Sided Sprite에서만 기존 양면 패스를 선택한다. 원본 blend/depth/material ID는 보존한다. mesh/decal/trail/compiled adapter/native-v14 source contract에는 적용하지 않는다. 새 bool이 struct padding에 들어가더라도 구 OBJ 생성자와 섞지 않고 codec core와 소비자를 같은 헤더로 빌드한다. 활성 편집 파일에는 최신 사용자 저장 SHA를 확인해 지정 필드만 치환하고 기준본을 보존한다.

### 패턴 간 선택 복사와 독립 창의 입력 소유

패턴을 바꾸면 timeline 선택은 지워지므로 clipboard는 원본 포인터가 아닌 세션 값 snapshot으로 보관한다. animation의 source stage/slot ID와 대상의 새 occurrence ID를 구분하고 World owner·내부 Animation Blend·Effect 그룹을 함께 remap한다. Paste는 기존 행의 clock을 이동하지 않고 전체 lifetime 뒤에 추가하며 모든 검증 뒤 한 번만 commit한다. 공유 정의가 변경됐으면 무조건 덮어쓰지 않는다. Patterns와 Sequencer는 서로 다른 ImGui root window이므로 timeline 내부 focus 검사만으로는 대상 패턴을 고른 직후 Paste할 수 없다. 각 pane의 focus를 수집하고 행 포인터 사용이 끝난 뒤 처리하며 텍스트 입력·popup·drag를 먼저 보호한다. 구현과 검증은 09-14 Sequence RESULT G48에 기록한다.


### 마리오 랜덤 후보와 현재 패턴의 시작 시각

완료 횟수 Logic의 후보는 서버가 중복 없이 선택하고 실제 PATTERN_COMPLETED만 누적한다. Parent Summon의 authored Stage가 비어 있어도 명시 lifetime과 기존 확장 결과로 실행 여부를 판단한다. stage 합보다 긴 explicit lifetime을 짧게 자르지 않는다. Success가 비어 있으면 마지막 완료에서 portal과 대기 entry를 정리하고 정상 종료하며, 후속 Success가 있는 체인의 실제 복귀 대기는 보존한다.

Bundle member의 최초 scheduled tick은 랜덤 child의 시작 tick이 아니다. 패턴 전환마다 실제 boss.iPatternStartTick을 복제하고 Sequencer는 해당 run/revision의 현재 member 시계만 읽는다. Stop 요청 대기 중에는 추적 상태를 버리지 않고 거절 시 원래 ACTIVE 상태로 돌아가야 한다. 자동 선택은 미적용 editor 입력을 잃게 하지 않으며 dirty 또는 활성 입력이 생기면 해당 실행의 선택 추적을 멈춘다. 적용·게시·제품 빌드 증거는 09-14 KOUKU_MARIO_SERVER_PROGRESSION RESULT G07에서 구분한다.

## 생존 Object의 반복 길이·피격 범위·종료 소유자

- Pattern 박스 duration을 HP 수명으로 재사용하지 않는다. UNTIL_DESTROYED는 Server의 개별 body/cue receipt가 소유하고 정상 완료된 run 밖에서도 boss 제거·사망·취소·퇴장을 정리해야 한다. 늦은 PLAY보다 exact STOP_CUE tombstone이 우선한다.
- 구형 공을 모델 AABB의 대각선으로 피격 원에 투영하면 반지름이 약1.414배 커진다. 원본 모델 bounds를 유지하고 ELLIPSOID의 실제 transform을 투영한다. preScale·resource scale·occurrence scale은 각각 한 번 적용한다.
- 전체 animation+Effect 반복의 주기는 저장한 창이다. burst/kill-on-deactivate 원본은 duration+particle life 추정값보다 실제 재생이 먼저 끝날 수 있으므로 그 추정값으로 반복 주기를 늘리지 않는다. stage 길이를 늘릴 때 기존 key·clip·Effect 시작과 속도를 자동 재분배하지 않는다.
- ParticleModuleMeshMaterial의 non-null 전체 section 배열은 TypeData bOverrideMaterial=true여도 Required보다 먼저 소비한다. bool=true를 이유로 거절하거나 원본값을 false로 변조하지 않는다. 실제 mesh section별 슬롯 경로·native 계약·전체 coverage는 계속 검사하고 null/누락 슬롯은 명시적으로 거절한다.

### 우클릭 hold 이동과 클릭 표식의 생성 주기를 분리한다

- 이동 목적지 재전송마다 `CClickMoveEffect::Play`를 호출하면 이전 handle을 Stop하고 새 표식을 생성해 hold 중 클릭이 반복된다. typed 이동 송신·예측·sequence는 유지하며 표식만 최초 물리 press의 성공한 송신에 연결한다.
- raw press 상태는 capture/Mario/타기팅의 early return 전에 갱신한다. 동일 player presentation rebind는 상태를 보존하고, Bern NPC의 명시 클릭은 기존 한 번의 표식을 유지한다.
- 구현·컴파일·사용자 확인은 [World marker 결과 G06](09-12/2026-09-12_KOUKU_PLAYBACK_AND_WORLD_MARKER_IMPLEMENTATION_RESULT.md#g06-2026-09-16-우클릭-hold의-클릭-표식-반복-생성-수정)에서 구분한다.


## 노이즈 왜곡과 Decal 수신 표면을 구분한다

- 캐릭터나 폭탄이 두 번 보일 때 객체 spawn 수만 조사하지 않는다. source SceneColor 샘플, 별도 distortion pass, 실제 dispatch와 최종 화면 resolve를 연결해 본다. 노란 장판의 Decal actor 배제는 화면 distortion에 자동 적용되지 않는다.
- 원본 pass가 존재해도 상수0일 수 있다. 이번 검토14개 중 실제 texture-dependent offset3개만 보호 채널로 옮겼으며 원본 색·크기·왜곡 식은 보존했다.
- signed offset BA를 추가하면 RT 형식뿐 아니라 blend write mask, alpha blend operation, coverage, fixed-function admission과 생성기를 함께 바꾼다. 일반RG나 BA=0 writer가 기존 누적을 지우면 안 된다.
- 이동된 UV의 중심 한 점만 검사하면 bilinear 이웃에서 actor 영상이 다시 섞인다. 현재/일반RG/BA 합성 footprint를 실제 필터 가중치로 검사하고, map marker의 packed payload와 actor bit를 구분한다. 경사면 깊이는 평면 기울기로 비교한다.
- actor 표식이 없는 정적 prop 내부까지 완전 차단했다고 쓰지 않는다. 실제 source·공통 pass·Engine resolve 수치 검증과 사용자의 화면 관찰은 별개다. 적용 범위와 증거는09-14 Sequence RESULT G50이다.


### 마리오 진입은 시각 창·접촉 원·실패 재시도를 함께 확인한다

포탈이 보이는 시각과 Logic 시작, MAP 위치와 BOSS_CURRENT root, solid boss/player 반경을 따로 대조한다. 플레이어 중심만 박스 안으로 요구하면 body collision에 막혀 영원히 들어갈 수 있다. 기존 Shared body-circle와 solver margin을 동일하게 사용하고 Y gate도 실제 위치로 조사한다. 일반 action을 취소하는 Mario 접촉은 목적지 검증 뒤 commit하며 retryable 실패를 inside 캐시에 고정하지 않는다. 같은 active move를 다시 시작하거나 다른 trigger의 once/interaction 정책으로 확장하지 않는다. 근거와 설치 경계는09-14 Mario progression RESULT G08이다.


### 반복 Pattern의 모델 시계와 source socket 시계를 함께 연결한다

Effect Tool 모델을 현재 Pattern clip으로 바꿔도 SourceModelPreview 기반의 별도 bone sampler가 옛 clip을 읽으면 손과 trail이 다시 분리된다. 모델 pose와 source anchor 모두 동일한 animation snapshot과 effect start offset을 사용한다. 공용 Effect의 SourceModelPreview를 특정 occurrence 때문에 저장 변경하지 않는다. 긴 박스에 맞추는 시간 stretching과 loop0 emission 연장은 다른 정책이며 동시 적용하지 않는다. duration clamp·late seek·되감기·끝난 뒤 tail과 기존 owner cleanup을 함께 확인한다. 원본1m local offset과 bone preScale도 실제 월드 거리로 확인한다.


### 게시 성공과 F1 목록 로드의 용량 계약을 함께 검사한다

Kouku Encounter가 root-motion/월드 연출을 포함해 커지면 publisher 성공 뒤 BossTool의 선행 byte 상한에서 거절될 수 있다. 파일 크기 제한뿐 아니라 `CDataJson`의 기본16MiB와 value/depth 제한도 같은 호출에 명시한다. 현재 F1 Encounter 계약은64MiB/4,000,000values/depth64이며 projector가 같은 조건을 게시 전에 검사한다. Load 실패로 Flow까지 읽지 못한 상태를 `No saved Pattern Flow`로 표시하지 않고 실제 오류와 마지막 정상 목록을 유지한다. 재발 검증은09-14 Sequence RESULT G51.

### 쿠크 Effect는 나오는데 보스 animation만 idle이면 binding root 계약을 확인한다

`KoukuSaydon.patternbindings.json`은 보스 Animation과 별도 PresentationPlayer가 함께 소비한다. publisher가 `targetedCombatVisuals` 같은 공용 optional section을 추가하면 두 reader의 root 허용 필드를 함께 갱신한다. Effect 소비 성공은 CNpc의 action binding 로드 성공을 보장하지 않는다. unknown field·schema·revision·clip 검증을 제거하지 말고 실제 게시 문서로 기존 엄격 reader 호환 검사를 실행한다.

Complete Play의 `target is not spawned`는 Parent/Summon 실행 전에 대상 보스가 없는 상태다. 다른 관문의 보스만 자동 생성하면 플레이어·맵·조명·HUD가 어긋나므로 기존 Gate 활성화의 spawn와 이동 승인을 기다린 뒤 저장 revision을 고정한 audition을 제출한다. Flow가 없는 새 session에 과거 `Level changed` 사유를 Flow 결과로 복사하지 않는다.

### 공유 Effect의 원본 애니메이션과 Pattern 선택을 구분한다

Effect를 원본 Resource 목록에서 열었는데 다른 동작이면 SourceModelPreview와 설치 clip을 먼저 비교한 뒤 Workbench의 선택 provider를 조사한다. 같은 Effect를 여러 Pattern이 사용하므로 마지막 편집 선택을 Open/Play 때 자동 소비하면 정상 저장 원본이 덮여 보인다. 기본은 저장 source이며 occurrence preview는 명시적으로 선택한 값 snapshot이다. 모델 pose와 bone sampler에 같은 snapshot/start/duration을 전달하고 성공한 문서 교체에서만 초기화한다. 선택 실패·로드 취소는 기존 상태를 보존한다. 해당 Effect를 특정 Pattern에 맞춰 재저장하는 우회는 하지 않는다.

### Trail의 폭 축 연속성과 단면 winding을 함께 검사한다

Trail이 꼬이거나 끊길 때 tick이나 shader부터 바꾸지 않는다. 설치 모델의 실제 궤적, 현재 sample cadence, camera와 tangent의 cross, 이웃 폭 축의 부호와 triangle winding을 함께 비교한다. 폭 축을 연속화하면서 단면 재질의 front/back을 바꾸면 일부 구간이 사라질 수 있다. 카메라 평행·왕복·중복점의 축과 완전퇴화 구간의 연결도 검사한다. baked AnimationTrail은 EdgePairs가 원본 geometry이며 centerline Points가 비어 있을 수 있으므로 centerline tessellation을 적용하지 않는다. 수치 검사와 사용자 GPU 화면 판정은 구분한다. 구현과 개별 증거는09-16 KOUKU_PATTERN_CLEANUP_AND_TRAIL_IMPLEMENTATION_RESULT에 둔다.
### 정적 맵 캐시 밖의 Deploy 그림자와 GPU elapsed 해석

맵 shadow cache hit만으로 정적 장면 전체가 재사용된다고 판단하지 않는다. MapStaticBatchObject/MapAssetObject 외의 DeployPropObject처럼 같은 Render_Shadow 큐를 사용하는 소품도 별도로 확인한다. 파괴 가능한 소품은 intact STATIC, actual world/model, opaque presentation 및 시간·카메라 독립 alpha 입력을 검증한 때만 기존 depth 캐시에 참여하고 destruction/fade/animation/physics/debris/suppression/morph/texture override에는 기존 경로를 유지한다. source pass를 유지하며 camera 밖 shadow caster는 최종 light volume으로만 제외한다.

GPU timestamp의 Shadow elapsed에는 CPU 명령 공급 공백이 포함될 수 있다. CPU NonBlend와 실제 draw/VS/PS 및 완전한 CPU 표본을 함께 읽고, enqueue 수를 실제 draw 수로 쓰지 않는다. 계측 예산이 차면 자식보다 늦게 종료하는 부모 scope도 사라질 수 있으므로 main root/pass 여유를 보존한다. detail 누락이 있으면 parent inclusive는 유효해도 SelfMs를 정확한 exclusive 비용이라고 보고하지 않는다. 안개는 별도 추정 대신 실제 포함 패스의 시간을 먼저 대조한다. [G34 결과](09-15/2026-09-15_MAP_AND_VALTAN_FULL_RESTORATION_RESULT.md)에 적용 및 검증 범위를 기록한다.

### 모델·이펙트의 병렬 준비와 등록 순서를 구분한다

서로 다른 모델·이펙트의 immutable 입력 준비는 제한된 공통 작업 예산으로 중첩할 수 있지만 Prototype registry와 Effect queue의 main commit까지 병렬화하지 않는다. Effect 후보는 먼저 끝난 순서가 아니라 원래 FIFO로 등록하고, 앞 target의 ACK 뒤 worker에서 현재 prepared catalog와 병합한다. main의 generation 검사를 제거하지 않는다. 새 session 최초 admission과 full replacement/clear를 구분해 병렬 sibling은 보존하고 A→B→A의 오래된 후보는 거부한다. 후보 개수와 미ACK 결과도 제한하며 큰 교체 자원은 worker가 해제한다.

실행 중 EXE와 수정된 소스는 별개다. 개별 compile을 Product 배포나 실제 FPS 개선으로 보고하지 않는다. headless 실패 주입 검사는 CRT assertion/abort와 Windows 오류 대화상자를 로그로 돌린 뒤 실행한다. 검사 프로그램의 실패 창을 실행 중 Client 결함으로 오인하지 않도록 process 경로·시각을 함께 확인한다. 구현과 검증 경계는 [Cold loading 결과 G04~G06](09-16/2026-09-16_COLD_MAP_LOADING_IMPLEMENTATION_RESULT.md)에 둔다.
