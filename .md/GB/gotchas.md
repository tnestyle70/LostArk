# LostArk merge 회귀 방지 정본

## 날짜 기준 옵션 복원과 명시적인 OFF 정본

과거 commit의 값과 현재 사용자가 유지하라고 지정한 예외를 함께 확인한다.
베른 `scene.bern.neutral-day.v1.fog.enabled=false`와 쿠크 Mario1~4의 FXAA OFF를
날짜 복원·merge·publish로 다시 켜지 않는다. revision은 과거로 낮추지 않고 증가시키며,
최신 저장본의 요청 필드만 병합하고 정본 publisher로 실행 데이터를 갱신한다.

## 같은 헤어의 무비와 장착 경로 차이

- 정상 장착과 무비가 다른 머리처럼 보여도 먼저 basis를 맞춘 정점·UV·weight와 디코딩한 texture alpha를 대조한다. 동일한 hair55를 경로만 교체하면 무비 골격·clip을 잃을 수 있다.
- 반투명 헤어는 forward만 연결하면 카드 사이의 깊이를 기록하지 못한다. CPart_Equipment와 CWorldSequenceObject는 공용 Render_SourceHairMaskedMesh로 native masked core를 먼저 그리고 기존 forward edge를 유지한다. 지원하지 않는 eyelash/ghost 등의 재질을 불투명으로 바꾸지 않는다.
- caller가 native masked 상수를 설정했어도 shader 내부에서 같은 값을 다시0으로 덮으면 discard가 무효다. program7의 source[26].x는 draw 입력을 소비하며, 실제 alpha0의 depth 미기록과 core 차폐를 검사한다. masked mode와 상수는 실패 경로에서도 복원한다. [헤어 수정 결과](10-05/2026-10-05_DIMENSIONMASTER_MOVIE_HAIR_RESULT.md).

## 프레임 CPU 작업의 병렬화 경계

- final camera 이전 Update에서 맵 가시성 job을 시작하지 않는다. frame provider가 최종 camera/light를 반영한 뒤 Layer 후보에서 owner snapshot → 큰 CPU 작업 묶음 → 동기 join → 원래 순서 GPU commit을 사용한다.
- worker는 per-batch staging만 변경한다. global camera capture·surface diagnostics·immediate context Map/Apply/Draw·렌더 큐는 owner에 둔다. 정규화 plane 검사처럼 숨은 전역 cache도 thread-local 또는 immutable 입력으로 바꾸고 검사한다. Profiler의 main-only WorkScope를 worker로 옮겨0으로 계측하지 않는다.
- 객체마다 job을 제출하지 않는다. 실제 설치 batch 크기 분포와 cache hit·owner stage·dispatch·join을 포함한 순이득으로 최소 비용을 정한다. 프러스텀/거리 grace와 hysteresis는 GPU upload 성공 때 함께 commit하고 실패 재시도에서 같은 프레임을 두 번 진행하지 않는다.
- 공통 pool의 nested callback은 caller/worker 양쪽 TLS guard로 직렬 처리하고 자기 pool의 lock/join을 기다리지 않는다. 외부 동시 제출은 직렬화한다. 이미 실행한 callback은 예외 뒤에도 모두 join되며 partial 쓰기는 소비자의 staging 책임이다. 작업에서 다른 스레드의 같은 pool 호출을 기다리는 순환 의존을 만들지 않는다.
- NPC·이펙트 전체 Update는 순수 CPU 함수가 아니다. network/collider/sound/event/provider/렌더 상태를 포함한 함수를 통째로 병렬 루프에 넣지 않는다. 작업 분산은 계산량 제거와 다르며 fixture의 wall time을 실FPS 향상으로 바꾸어 보고하지 않는다.

## Bern 가림·거리 컬링의 보수성과 검증

- AABB는 가려지는 대상의 검사 범위다. 열린 창·틈이 있는 모델의 AABB를 꽉 찬 occluder로 대신 그리지 않는다. 현재 제출할 원본 LOD0의 불투명 삼각형만 가림 근거로 사용하며 masked·opacity·wind·morph·다른 선택 LOD는 occluder에서 제외한다. 변형 없는 masked BG는 occludee로 허용할 수 있다.
- CPU와 D3D의 front face·mirror·cull을 실제 GPU 깊이와 비교한다. 같은 viewport 픽셀 중심과 triangle farthest-depth, 확장 query bounds로 보수성을 유지한다. 낮은 해상도 중심 샘플의 꽉 찬 셀 추정은 작은 틈을 메울 수 있고, 모든 삼각형의 안쪽 축소는 공유 내부 edge에 인위적 틈을 만든다.
- 가림 결과는 view/projection·viewport·설정·weak owner·geometry/visibility revision과 descriptor 전체가 같을 때만 재사용한다. visible payload에 대응하는 tight world AABB는 upload 성공과 함께 commit한다. queue 준비 실패는 원본을 보존하고 실제 제외 counter는 commit 뒤 기록한다. 입력 삼각형 budget과 rasterized 삼각형은 서로 다르다.
- Distance culling은 LOD와 별개다. 작은 정적 소품의 거리와 보수적 투영 크기 조건을 함께 검사하며 near plane/invalid 입력은 표시한다. 같은 aspect의 resize도 pixel 조건을 바꾸므로 viewport를 cache key에 넣는다. 거리·pixel hysteresis는 TRS/표시/설정 변경에 맞춰 무효화한다.
- 컬링 CPU 비용·원본 source draw 상한·후속 instancing 이후 실제 draw·게임 FPS는 다른 수치다. 저장 카메라에서 distance 제외가0이면 그 사실을 기록하고 성능 향상을 주장하지 않는다. 작은 파생 청크와 atlas는 현재 Bern 기본 경로에서 생성하지 않는다. [구현·검증 결과](10-04/2026-10-04_BERN_SPATIAL_CHUNK_HLOD_RESULT.md).

## 정적 청크 병합과 HLOD 검증 경계

- WModel 파일을 하나로 만들거나 material 이름이 같은 것만으로 한 draw가 되지 않는다. 활성 shader 입력·texture·pass·cull 상태의 호환성을 검사하고 실제 병합 vertex/index buffer를 제출해야 한다. RNM atlas와 static shadow의 서로 다른 원본 입력은 source별 payload와 lighting bank로 보존한다.
- 공간 분할 없이 멀리 떨어진 모든 동일 asset을 합치면 bounds가 커져 culling 이득을 잃는다. Bern은 원본 instancing부터 32m XZ origin cell로 나누고 geometry bounds를 별도로 보존한다. GPU 메모리·준비 시간과 현재 카메라의 실제 draw도 함께 측정한다.
- 파생 청크는 placement/picking/shadow의 새 정본이 아니다. TRS·visible·stage/camera suppression은 공유 claim을 무효화하고 원본 제출로 복귀한다. source와 chunk의 활동 상태는 같은 frame 결정으로 고정한다. 원본 material 편집을 추가할 때도 같은 invalidation을 연결한다.
- 합친 근거리 shader의 동치 검증과 원거리 geometry 오차 검증, 실제 게임 FPS는 서로 다른 증거다. synthetic triangle/callback 감소를 게임 FPS 개선율로 옮겨 쓰지 않는다. [청크/HLOD 결과](10-04/2026-10-04_BERN_SPATIAL_CHUNK_HLOD_RESULT.md)를 따른다.
- 원본 batch/mesh 수는 실제 카메라의 절감 draw가 아니다. 원본 instance culling·mesh LOD·lighting-bank instancing을 함께 대조하고, 병합 전체 geometry가 원본보다 증가하는 후보를 자동으로 사용하지 않는다. 같은 프레임의 source와 chunk는 같은 선택을 소비해야 한다. 정적 batch의 Update를 제거할 때는 material 시간·shadow 시간·가시성 grace와 authoring Reload의 legacy 경로를 함께 보존한다.
- 생성 하한과 실제 선택 하한은 같은 상수를 사용한다. 최소3개 원본 draw를 요구하면서2개 member 청크를 생성하면 어느 카메라에서도 선택되지 않는 geometry·GPU buffer·frame 검사만 남는다. source member 수는 절감 draw의 상한일 뿐이므로 생성 필터를 통과한 뒤에도 카메라·LOD·기존 instancing 검사를 유지한다.

## 동일 형상 인스턴싱과 CPU 계산 공유

- geometry/asset 이름 정렬만으로 실제 재질 호환 그룹이 이어지지 않는다. 준비 단계에서 실제 CModel/CMaterial predicate로 cohort를 구분하고 지원 외 객체의 슬롯을 보존한다. 저장 JSON의 일부 필드만 복제한 key를 최종 호환 판정으로 사용하지 않는다. runtime의 LOD·claim·시간·mirror 검사는 계속 필요하다.
- lighting bank의8개 제한은 고유한 조명 텍스처 조합 수다. 모든 submesh의 RNM average/directional/static-shadow 조합이 같은 source만 슬롯을 공유하며 source batch 개수와 shader bank size를 혼용하지 않는다. 조합 하나의 ordinary pass에서는 원본 instance payload를 그대로 유지한다.
- 공간 셀은 culling 단위이며 반드시 draw 단위일 필요는 없다. 셀별로 보이는 instance만 모으되 각 mesh의 LOD·claim·mirror·material·시간을 검증한다. 같은 geometry라도 RNM이나 서로 다른 활성 표면 입력이 있으면 해당 입력을 bank로 보존하거나 별도 draw를 유지한다.
- identical prefix를 먼저 그려 작은 lighting bank를 쪼개지 않는다. 같은 재질 A/A 뒤 호환 RNM 변형 B가 오면 기존 A/A/B 한 draw를 두 draw로 늘리는 회귀가 생긴다. identical prefix는 길이와 무관하게 조명 조합 하나다. 기존 bank가 다음 호환 변형까지 확장할 수 있으면 bank 경로에 양보한다.
- multi-mesh 결합은 draw 전에 모든 mesh의 compatibility와 LOD를 확인한다. 첫 mesh가 그려진 뒤 실패하면 원본 전체를 다시 그리지 않는다. source-draw 계측은 결합하지 않은 입력 수이며 과거 제품 대비 절감량이나 FPS가 아니다. 청크 선택의 원본 비용 하한도 확장된 instancing 기준으로 함께 갱신한다.
- 조명 bank의 전체 후보 admission을 없애고 현재 mesh 검증만 남기지 않는다. 반대로 이미 admission된 후보를 각 mesh에서 다시 전체 순회하면 material 검사가 mesh 수의 제곱으로 늘어난다. 현재 mesh의 geometry·transform·실제 CMaterial 입력을 매번 검증하는 binder를 사용하고, draw 수·GPU 시간·CPU 검사 비용은 따로 보고한다.
- shader cache의 재사용 이득은 실제 호출자의 변경 입력·row 전환·기존 중복 제거를 포함해 검증한다. 모든 바인딩을 고정한 microbenchmark만으로 공통 setter에 추가 비교·복사 비용을 넣지 않는다. shared Effect의 Clone·부분 write·실패·직접 write 무효화도 별도로 검사한다.
- NPC 공유는 cooked channel 내용과 정확한 track time이 같을 때 local sample 계산만 재사용한다. 각 actor의 clock·blend·root suppression·최종 bone palette와 Server authority는 유지한다. 채널 동등성 fixture와 실제 실행의 reuse hit/FPS를 구분한다.
- 정적 환경 particle-root 역행렬은 행렬 전체 bit가 같은 경우 재사용할 수 있다. moving root·NaN·signed zero·worker별 독립성을 확인하고, 해당 연산 microbenchmark를 전체 이펙트/FPS 개선으로 환산하지 않는다.

## 클래스 무비의 숨김 목록과 동일 형상 복제본

- 회색 저해상도 껍질이 다시 보이면 기존 stable ID의 제외 여부와 같은 WModel 형상을 쓰는 다른 occurrence를 각각 확인한다. native opacity 복원으로 이전의 잘못된 zero 입력에 가려졌던 복제본이 드러날 수 있다. 이를 shader 전체 opacity를 다시 끄는 방식으로 고치지 않는다.
- 실제 머리카락과 보조 전신 shell을 설치 geometry·재질 program·timeline opacity로 구분한 뒤, 사용자가 제거를 요청한 정확한 movie occurrence만 제외한다. 원본 숨김 목록과 다른 클래스 편집은 유지한다. [실측과 적용 결과 G09](10-04/2026-10-04_BERN_SPATIAL_CHUNK_HLOD_RESULT.md#g09-차원술사-클래스-무비의-머리-껍질).

## 캐릭터 선택의 마지막 서버 응답과 빈 가방 이동

- HUD의 마지막 inventory를 복사한 즉시 연결을 닫으면 직전에 보낸 구매·장착·강화 결과가
  수신되지 않아 유실된다. room FIFO의 typed capture 응답을 기다리고 sequence·세대·world·
  player/entity/class를 확인한 뒤 local character ID에 저장한다. 이후의 낡은 HUD capture로
  이 값을 다시 덮어쓰지 않는다. 실패/timeout도 기존 슬롯을 유지한다.
- Bern 복원 요청을 보낸 프레임부터 성공 응답까지 gameplay/economy 입력을 막는다. 새
  admission은 계속 허용해야 거부 뒤 다른 캐릭터를 선택할 수 있다. sequence1 고정이나
  슬롯 index·닉네임을 저장 identity로 사용하지 않는다.
- 입장 직후 fTimeDelta에는 앞선 loading/activation 시간이 들어갈 수 있다. 방금 보낸 복원 요청의 timeout에 이 값을 더하지 않고 steady_clock으로 실제 대기를 측정한다. Lobby 복귀를 슬롯 손상으로 단정하기 전에 entry.accepted, main-pump.stall, character.restore-timeout의 시각을 비교한다. 실제5초 제한과 실패 시 슬롯 보존은 유지한다. [수정 근거 G10](10-04/2026-10-04_BERN_SPATIAL_CHUNK_HLOD_RESULT.md#g10-다른-캐릭터-생성-후-첫-캐릭터의-베른-재입장).
- 강화 UI의 전역 itemId map은 같은 직업 캐릭터 사이에 섞인다. 서버 inventory 항목의 단계와
  내구도를 복원·장착·해제에서 함께 보존하고, 선택창은 생성 외형 뒤 저장된 아바타를 적용한다.
- 선택 복귀의 Loading 완료는 공통 prototype 준비만 뜻하지 않는다. 실제 채워진 슬롯의 class 모델·생성 외형·장착 아바타 성공까지 기다린 뒤 창을 열고 active local character ID로 원래 카드를 선택한다. 재시도는 새 Loading 생성 후 이전 Loading 소멸 순서일 수 있으므로 준비 owner를 확인해 이전 소멸자가 새 준비를 취소하지 않게 한다. 실패와 취소는 저장 로스터를 변경하지 않는다.
- inventory.empty()는 fresh 입장 판정이 아니다. 이미 플레이한 빈 가방/0 재화도 명시적인
  carried state로 전달해야 월드 이동에서 초기 지급으로 바뀌지 않는다.
- 구현과 실행 검증은 [캐릭터 슬롯 결과](10-03/2026-10-03_CHARACTER_SLOT_STATE_RESTORE_RESULT.md)를 따른다.

## 입장 승인 뒤 이전 Level의 복제 이벤트 소비 금지

- ENTER_ACCEPTED가 world inbound generation을 바꿔도 비동기 모델 준비 취소 때문에
  이전 Level이 몇 프레임 더 살아 있을 수 있다. 전환 요청 직후 반환 한 번으로는
  목적지의 PLAYER_SPAWNED 유실을 막지 못한다.
- CClientReplication은 Initialize에서 소유 세대를 캡처하고 Update의 큐 소비와
  후속 준비 전에 일치 여부를 확인한다. world ID만 비교하면 같은 월드 재입장을
  구분하지 못한다. Reset은 소유 세대를 해제하며 disconnect 정리는 유지한다.
- 캐릭터 없이 자유 카메라로 남는 증상은 카메라 강제 전환이나 로컬 캐릭터 복제로
  가리지 말고 승인 → 초기 spawn → 새 Level 소비 순서를 먼저 확인한다.

## 강화 창의 월드 텍스트 가림 영역

- 강화 창은 중앙 ItemUpgrade_PanelBg가 아닌 전체 ItemUpgrade_WindowBg rect를
  WINDOW_ITEM_UPGRADE occluder로 등록한다. 이름표는 WORLD층, 강화 자체 문구는
  같은 WINDOW_ITEM_UPGRADE층을 사용하므로 창 밖 이름표와 창 안 문구는 유지한다.
- 대기·성공·실패 화면의 배경 범위도 비교한다. 중앙 패널만 사용하면 좌우 목록과
  결과 화면 위에 가이드 이름표가 그려진다.

## 콜로세움 승패 배너의 결과 identity와 마지막 프레임

- 로컬 PlayerId/NetEntityId를 서버 참가자 팀과 조인하지 못한 상태를 패배로 간주하지 않는다.
  정확한 팀 확인까지 결과 UI를 보류하며 패배는 확인된 상대 팀 승리일 때만 표시한다.
- 같은 원본 무비의 승리/패배라도 프레임 수는 다를 수 있다. 콜로세움은 40fps에서 승리 139,
  패배/무승부 140프레임이다. 마지막 키의 시각에 한 프레임 길이를 더해 끝 프레임을 보존한다.
  결과별 배너 종료와 전체 참가자의 공통 컷신 시작 시각을 구분해 양팀의 연출 시계가 갈리지 않게 한다.
  상세 적용/검증은 10-01 COLOSSEUM_MATCH_FLOW RESULT G06을 따른다.

## 워터팡 효과·NPC 복원 경계

- `.restore`만으로 player skill preview를 선택하지 않는다. Maharaka World 효과의
  실제 WorldSequence model/bone, player 총구의 prop bone, 독립 투사체 root를 구분한다.
- source emitter의 좌표 변환과 GADGET 발사 offset을 owner·emitter 양쪽에 중복 적용하지 않는다.
- 원본 PSA clip 추가는 설치 WModel의 변환 basis부터 같은 idle의 전체 bone/key로 실측한다.
  UModel-glTF 주민의 position(x,z,y), root quaternion(-x,-z,-y,w), 나머지(x,z,y,w)을
  다른 ActorX 모델의 기본값으로 전파하지 않는다. 추가 clip 외 기존 section은 byte를 보존한다.
- native deferred0·shader 컴파일을 원본 VS 변형·화면 성공으로 대신하지 않는다.
  워터팡 program5275의 bubble 변형은 원본 VS16~23의 UV·time·DynamicParameter와
  normal 방향을 복원한 family7/group5248/profile5275 전용 경로다. CPU 동치 검증과
  shader 컴파일은 실제 GPU 화면 검증과 구분한다. 다른 profile에 같은 변형을 전파하지 않는다.
- ERM_None/Point처럼 원본에서 그리지 않는 provider emitter는 effect.standard와
  disabled source material로 보존한다. 비어 있는 sourceMaterialSlots 배열을 기록하지 않는다.
  codec load 실패를 발사/수명/카메라 문제로 단정하지 말고 EffectFailure 로그부터 확인한다.
- 물총 Q의 MK2 3갈래는 같은 총구 원점에서 owner yaw에 0/+30/-30도를 더한다.
  발사 offset을 회전한 각 ray에 다시 적용하거나 Speed와 MaxDistance 열을 바꾸지 않는다.

### Movie 카메라 기준 키와 정적 배경의 소유자

공통 view pose 캡처에 Valtan 저작 문서의 수직 FOV10~120도 제한을 재사용하지 않는다.
Movie의 수평17도는16:9에서 수직9.610678도이며 runtime에서 유효하다. 캡처는 runtime의
1<FOV<179와 finite/nondegenerate basis를 검사하고, 저장 소비자가 자기 문서 제한을 검증한다.
Use free cam pos는 Eye/LookAt/Up만 복사하므로 선택 key의 FOV를 보정하거나 덮어쓰지 않는다.

같은 공통 편집기를 써도 capture callback을 전달하지 않으면 버튼이 나타나지 않는다.
Model root 행을 일괄 금지하는 대신 해당 owner가 현재 preview root의 역행렬로 Eye/LookAt은
좌표, Up은 방향으로 변환한 후보를 반환한다. determinant에 큰 고정 epsilon을 쓰면 정상적인
작은 model preScale까지 거부하므로 singular/nonfinite와 최종 basis/문서 유효성을 검사한다.
월드 포즈를 model-relative key에 그대로 넣거나 캡처 FOV로 저장 키의 lens를 덮어쓰지 않는다.

Use free cam pos가 저장하는 LookAt 거리는 원본 키와 다를 수 있다. 첫 Eye만 바꾸거나
첫 LookAt만 긴 벡터로 두면 다음 키 보간에서 구도가 급히 돌아간다. 수정 전·후 camera basis의
회전을 Eye 상대 경로와 시선·Up에 함께 적용하며 시선 벡터 길이만 한 컷에서 일관되게 맞춘다.
이 길이 비율로 이동 경로를 확대하지 않는다. 서로 다른 컷은 기존 cut 정책을 유지하므로
컷 안의 연속성과 Loop 끝→첫 컷의 구도 차이를 별도로 측정한다.
같은 컷을 다시 보정할 때는 최초 원본 대신 직전 실제 설치본을 기준으로 새 저장분을 비교한다.
이미 보정된 뒤 키에 첫 보정량을 다시 더하지 않으며, 변경하지 않은 기준 키와 컷은 유지한다.

Movie WORLD picking은 WorldSequence object를 검사한다. backgroundAreaId로 별도 로드한
map placement는 그 목록에 없으므로 미선택을 삼각형 picking 결함으로 단정하지 않는다.
하늘 이름이라도 무한 구체가 아닐 수 있으며 실제 WModel 외곽·배치와 원본 shader를 확인한다.
배경은 기존 MapTool에 stable Area로 고정해 빌리고 class 전환이 저장 대상을 바꾸지 않게 한다.
변경은 mapplacements에서 저장·게시하며 Movie JSON에 별도 map override를 만들지 않는다.
존재하지 않고 편집하지도 않은 optional WorldSequences는 placement Save로 생성하지 않는다.
상세 검증과 화면 경계는09-26 WORLD_MOVIE_EFFECT_EDITOR RESULT G21·G22를 따른다.

### 본 부착 그룹의 재생 선택과 실제 전방 축

Group by anchor에는 비활성 Light/ScreenPost 자리표시자도 포함될 수 있다. 그룹의 문서·편집
대상은 유지하되 Play Group의 재생 선택에서는 명시적으로 enabled=false인 presentation만
제외한다. 단일 Solo, 누락 ID, 숨김 carrier, 활성화된 잘못된 presentation을 정상으로 취급하지
않으며, 재생 대상이 없으면 기존 preview를 보존한다. 화면의 presentation admission 메시지만
보고 문서 전체가 drawable=false이거나 live draft 전달이 끊겼다고 단정하지 않는다.

source 전방과 Element Euler만으로 검격 방향을 판단하지 않는다. 실제 설치 모델의 preTransform과
해당 animation의 bone basis까지 합친다. Guardian 리벤지 스피어의 기존 source +X가 월드 -Y로
변환되는 원인은 정적인 b_root basis였으며, 독립 그룹 socket에서 그 회전을 상쇄했다. 본의
전체 clip 안정성과 owner yaw별 전방을 측정한 뒤 기존 BONE 경로를 사용하고, sourceRecipe에
금지된 OWNER_YAW admission을 우회하지 않는다. 수치와 화면 판정은 별도다. 상세는
09-30 GUARDIANKNIGHT_REVENGE_SPEAR RESULT G04를 따른다.

### 기믹의 성공 안전존과 일반 무적의 즉사 우회를 구분한다

일반 즉사가 실드·개인 무적·시간 정지를 관통하더라도 같은 Pattern에서 정확 인원을
충족한 INVULNERABILITY_ZONE까지 건너뛰면 성공한 파1빨2가 사망한다. 현재 실행의
성공 집합은 generic lethal hit보다 먼저 결과를 차단하고 다른 Pattern이나 종료 이후에
보호를 남기지 않는다. 50% 피해·FEAR 테스트만으로 즉사 보호를 검증하지 않으며 실제
게시 원·threshold·세 타격과 인원 부족/초과를 함께 검사한다. 근거는09-21
KOUKU_SAFE_ZONE_CHARGE_CUTSCENE_RESULT G07에 둔다.

진입 컷신의 HUD 숨김은 캐릭터 숨김이 아니다. 기존 Character presentation owner를
본체·장비·탈것·그림자의 draw까지 연결하고 source와 camera exit blend의 수명을 함께
소비한다. 정상 종료·실패·취소·이탈은 표시를 복구하며 전투 camera의 정책은 유지한다.
### snapshot 뒤에도 유지해야 하는 비행 포즈 보정

Object Update 뒤 Level의 snapshot 적용이 skeleton을 다시 설치할 수 있다. 서버가 소유한
고도와 중복되는 원본 body lift를 제거할 때 frame Update 한 곳에서만 local bone을 보정하지
않는다. 비행 mount 포즈 설치 경로를 하나로 모으고 frame·snapshot에서 모두 보정한다.
같은 시각의 실제 설치 CModel을 raw pose → 보정 → snapshot 재설치 순서로 측정하여
몸체·seat 높이를 대조한다. rider 자체의 root motion과 혼동해 두 모델에 일괄 보정하지 않는다.

투척 아이템의 조준은 기존 지면 스킬 renderer/geometry를 재사용하되 item ID와 request sequence를
별도로 소유한다. 조준 시작·취소에서 서버 명령을 보내지 않고 확정 클릭만 기존 item sink로 제출한다.
취소된 마우스 hold가 다음 frame 이동으로 새는지, UI/focus에서 누른 숫자키가 복귀 시 새 edge가
되는지 확인한다. 원본 texture 한 장의 재사용은 원본 다층 material/particle 전체 복원과 구분한다.

### 패턴의 회전·충돌 종료와 cinematic 입력 분리

패턴 target/aim이 NONE이어도 선택 직전 nearest-target FacePoint나 Stage 중앙 이동의
FacePoint가 회전을 다시 만들 수 있다. 고정 부채꼴은 원본 경고와 hit의 같은 yaw를 유지하고,
개별 패턴과 복합 패턴 안의 같은 동작을 각각 검사한다. 복합 패턴 전체 추적을 끄지 않는다.

보존된 ONCE Effect tail은 일반 Stage 종료와 다르게 처리된다. 이동을 따라가는 돌진 aura는
원본 lifetime을 줄이지 않고 현재 CHARGE stage의 남은 시간을 cue 종료로 사용한다.
벽 충돌 GROGGY·정상 완료·패턴 교체·죽음의 승인 action 전환에서 pending과 tail을 정리한다.
다른 원본 이펙트의 보존 tail은 유지한다. 명시 TIMEOUT branch가 event-only GROGGY
건너뛰기를 우회하는지도 확인한다. 같은 deadline의 WALL_CONTACT는 정상 timeout보다 우선한다.

cinematic camera 활성만으로 모든 player visibility/input/HUD를 함께 끄지 않는다.
전투 중 대형 등장 camera는 기존 Server 승인 이동을 유지하며, DANCE는 플레이어 HUD와
보스 HUD의 표시 조건을 구분한다. 실제 raid phase와 camera owner를 모두 대조한다.
구체적 적용·검증은 09-29 RAID_MOVIE_INTEGRATION 및 연결된 발탄·쿠크 RESULT를 따른다.

### Movie 파생 모델 교체와 시간별 재질 트랙

Movie의 보이지 않는 삼각형이 빨간 강조로 선택되면 기본 MIC opacity만 보지 않는다.
현재 material curve가 덮어쓴 값, native program의 primitive opacity 입력, draw filter와
별도 highlight pass를 확인한다. Warlord selection-native-702는 named `op`와 별도로
source row0.x를 곱하며, packer가 소유하지 않는 primitive opacity의 identity가 누락되면
최종 alpha가 0이다. 이 입력만 해당 program adapter에서 복구하고 명시적인 `op=0`이나
다른 program의 상수를 전역적으로 1로 바꾸지 않는다.

장면 Pick의 highlight ID만 갱신하면 World Model의 Details 선택은 바뀌지 않는다.
stable ID와 새 선택 이벤트를 기존 row 편집기로 전달하고 미반영 draft는 보존한다.
한 키만 위치를 바꾸면 다음 키에서 원래 경로로 복귀하므로 소품 전체 위치 조정에는 현재
phase의 track 위치에 같은 이동량을 적용한다. Intro/Loop를 임의로 함께 수정하지 않는다.

Movie 모델을 일반 catalog donor로 바꾸면 WorldSequences의 model/materialSource 연결뿐 아니라
ClassSelection.cinematics의 Intro/Loop materialTracks도 실제 사용 mesh의 materialName·family에
맞춰야 한다. 이전 이름은 override rejected, 이름만 바꾼 이전 program은 mismatch로 재생을 막는다.
정상 donor 외형을 선택한 교체는 기본 parameters도 donor와 대조하고 기존 곡선·시간·제외 목록은
보존한다. 준비 성공만으로 완료하지 말고 재생의 Set_ObjectMaterialConstants까지 확인한다.
발생 근거와 복구 검증은 `09-27/2026-09-27_WORLD_MOVIE_HAIR_GUARDIAN_EYES_IMPLEMENTATION_RESULT.md` G11에 둔다.

### Release 선로딩과 실행 직전 재준비

쿠크 Release 입장에서 준비한 WORLD 모델·clone pool을 첫 Server PREPARING에서 다시
Load_Area하여 비우면 선로딩 효과가 사라진다. Loader의 검증된 WORLD를 공통 Complete Play
준비에 연결하고, 같은 선택·source revision·catalog generation의 완료 상태를 재사용한다.
V1은 queued가 아니라 실제 settled/prepared여야 하며 초기화 중 worker를 기다리는 무한
루프를 만들지 않는다. 저작 reload, 다른 revision과 명시 취소는 기존 검사를 유지한다.
성공한 명시 WORLD reload에서는 아직 사용하지 않은 관문 준비 객체와 조명도 함께 폐기한다.
실패한 로드와 활성 cinematic이 빌린 환경은 유지한다.
발탄 Release의 이미 준비된 V2/WORLD 검증에도 리소스마다 한 프레임 대기를 추가하지 않는다.
Character Select는 현재 roster 전체를 이미 선로딩하므로 Server 승인과 교체 검사를 지워
속도를 개선한 것으로 처리하지 않는다. 초기 준비·서버 왕복·실제 화면 시작 시간은 구분한다.

### Server entry failed와 reliable 송신 큐 초과

Lobby의 Server entry failed는 일반 복구 문구다. 승인 timeout으로 단정하지 말고 Server의
connection.closed reason과 queuedFrames/queuedBytes, 해당 시각의 Room 상태를 확인한다.
작은 combat event도 개수 상한을 먼저 채울 수 있다. 현재 송신 큐는 연결당 4096 frame/8 MiB로
제한하며 reliable FIFO와 snapshot 병합·reserve를 유지한다. 일시적인 WSAEWOULDBLOCK을
입장 승인 timeout이나 이미 종료된 연결의 drain timeout으로 혼동하지 않는다. 큐 확대는
일시 정체 허용량의 증가이며 원격 Client 정체 원인이 해결됐다는 증거가 아니다.
09-29 Release 시퀀스 준비 RESULT에 실제 카드 패턴의 동시 종료와 회귀 검증을 기록한다.

### World 구슬의 파괴 원인·표현 수명과 추적 정지

World SOURCE_LOOP의 입장 검사는 실제 지원 carrier와 같아야 한다. portable Cascade Ribbon은
TRAIL kind이지만 source particle simulation과 지속 방출을 사용한다. sprite/mesh-only 검사로
정상 Ribbon을 거부하지 말고 기존 typed ribbon admission을 사용한다. map parser/publish 성공을
실제 Spawn → Commit → Update clone/attach 성공으로 대신하지 않는다. 실패 이유는 Stop이
status를 초기화하기 전에 복사해 effectAssetId와 stable placementId를 함께 보고한다.

같은 destruction mutation을 돌진과 외곽 전체 파괴가 공유할 수 있다. mutation 이름만으로
전체 구슬을 제거하지 말고 실제 stage binding과 성공한 commit을 구분한다. 벽 위 구슬은
기존 SOURCE_LOOP로 유지하고 Server snapshot이 없는 동안 로컬로 생성·획득하지 않는다.
착지점은 바닥 높이뿐 아니라 벽 파괴 뒤 플레이어가 접근 가능한 nav 연결과 다른 collider를
검사한다. World Effect와 Server pickup bootstrap은 같은 게시 transaction에서 교체한다.

추적 종료는 Stage의 정수 시계로 판정하고 다음 Stage 진입의 FacePoint가 고정 방향을 다시
덮어쓰지 않는지 확인한다. Preview도 같은 fixed tick 회전 helper를 사용한다. 충격음 지연은
cue 시점과 WAV 내부 무음 길이를 분리해 측정하며 playback offset을 gameplay hit 이동으로
설명하지 않는다. 구체적인 검증은 09-28의 Play Pattern·World Ether RESULT를 따른다.

### 연출 Effect 편집은 원래 장면 시계를 유지한다

WorldSequence의 Effect 목록만 Open하면 배우 TRS·본·발생 시점이 사라진다. 연출 단위 편집은
같은 WorldSequencePlayer를 사용하고 Element preview만 기존 world-root renderer에 교체한다.
여러 occurrence가 같은 Effect를 쓰면 저장 owner는 하나임을 표시한다. Solo로 숨길 때 모델
애니메이션과 source history를 함께 중단하지 않으며, 실패한 교체는 이전 preview를 보존한다.

Valtan Full Restore의 Solo는 Complete와 같은 source-aware sequencer로 먼저 보내야 한다.
선택에서 제외한 형제 trail의 baked-edge history가 남은 문서를 먼저 stage하면 unreferenced history 검증에서 실패한다. 기존 sequencer의 선택 projection과 source target을 재사용하고, 남은 trail에 필요한 history는 보존한다.
전투 오브젝트 Solo의 Element source 시작은 패턴 시각으로 변환한 뒤 애니메이션과 함께 seek한다.
여러 Element의 일회 그룹 재생도 첫 Element만 넘기지 않으며 반복은 선택 시작점으로 돌아간다.
선택 projection은 참조되지 않는 sibling baked history만 제거한다. baked trail/light와 실제
본 follow carrier를 같은 집합으로 세지 않으며, 실제 follow 집합의 bone/socket 계약을 검증한다.
마지막 Element 삭제를 지원할 때는 저장 guard뿐 아니라 Product 준비와 sequence loop/fit도
빈 body를 처리해야 한다. 모든 실행 row와 runtime extension이 비어 있고 Codec 구조 검증을
통과한 authored source만 무표시 상태로 허용하며, 잘못된 nonempty 문서를 허용하지 않는다.

### 발탄 Preview와 Play Pattern의 장판·마지막 동작 차이

게시된 cue가 존재해도 전멸 직후 NO_VALID_TARGET 종료가 마지막 애니메이션과 늦은 cue를
삭제할 수 있다. 최종 타격의 사망 fixture에서 남은 동작·RECOVERY·COMPLETED와 snapshot을
검사하고 생존자를 과도한 HP로 유지한 테스트만으로 완주를 확인하지 않는다.
도약 중 root/follow의 실제 높이와 decal depth 및 snapshot particle의 생성 위치를 대조한다.
Server가 시작에 고정한 착지점은 현재 target pose와 다르므로 protocol의 landing snapshot을
사용한다. 새 snapshot 계약이 nonzero pattern 시작 tick을 요구하면 기존 action tick뿐 아니라
실제 BeginPattern/완료/사망의 pattern tick 기록·정리까지 연결하고 전체 packet encode를 확인한다.
이전 Stage의 유한 지연 경고가 중간 source 컷씬의 Stop_BossOwner에 지워지는지도
확인하며, 명시적 tail만 숨김·시계 유지하고 일반 몸체 이펙트 및 Stop 정리는 보존한다.
다른 패턴에서 보이는 경고는 잔상으로 단정하기 전에 canonical/Product의 stable cue 참조를
대조한다. 원본 notify별 공백을 제품의 연속 장판 요구로 오인하지 않는다.

### 새 Animation 패턴 생성은 기존 promotion을 다시 생성하지 않는다

Create New Pattern이 과거 intake로 기존 promotion까지 재생성하면 이후 편집한 stage duration,
animation 및 camera가 서로 다른 시계가 된다. 새 요청은 현재 canonical 기존 pattern을 그대로
보존하고 새 pattern만 추가한다. 전체 lineage·join·Product validation은 유지하고 잘못된 기존
camera를 자르거나 검사를 건너뛰지 않는다. 명시적인 promotion refresh의 재생성 동작과 Create의
보존 동작을 구분해 회귀 검사한다.

### Effect 게시 일치는 배열 index가 아니라 stable binding ID로 비교한다

원본 cue 배열은 편집 저장 순서이고 Product reader는 action/clip/start/occurrence 순으로
정렬한다. 시작 시각 편집으로 순서가 바뀌면 같은 index 비교는 정상 게시도 변경으로 오판해
F1 inventory와 Server Play를 막는다. 동일 stage의 stable binding ID로 찾아 전체 저장 field를
비교하고 개수·중복·누락·실제 field 변경은 계속 거부한다. 이 상태에서 Save/Publish 반복이나
source 배열 재정렬을 복구 절차로 요구하지 않는다. native strict reader의 전체 inventory 성공과
실제 cue 값이 다른 negative case를 함께 확인한다.

### V2 게시 검증은 현재 decoder와 저작 라이브러리를 함께 따른다

`CEffectV2Object::Acquire_Texture`의 DDS/WIC 경로가 PNG를 지원해도 Python 검증이 DDS만
허용하면 쿠크 DJ의 정상 PNG 때문에 발탄 `Save & Publish` 전체가 중단된다. 일반 corpus와
typed binding validator의 texture 허용 형식을 함께 맞추고 mesh WModel·상대 경로·실물 검사는
유지한다. V1으로 교체한 에스더·쿠크의 이전 V2 leaf/group은 저작 라이브러리에 보존될 수 있다.
모든 문서와 실제 binding/group/Independent 참조를 검증하되 모든 저작본의 역방향 runtime
binding을 요구하거나 이를 통과시키려고 가짜 Independent 등록을 하지 않는다.

Encounter Stage에 optional `attackContacts`를 추가하면 gameplay뿐 아니라 같은 Stage를 읽는
지형 파괴 publisher의 exact field 검사도 함께 맞춘다. 지형 게시기는 nonempty 배열과 Stage
identity만 확인하고 contact geometry·pulse·damage 의미는 기존 Gameplay/Server 검증을 유지한다.

피자 target-follow의 공통 yaw180과 개별 cue의 원본 방향 보정은 별개다. 한 조합의 빨강과 노랑
관계가 정상인 채 전체 방향만 반대면 그 occurrence의 LocalTransform만 보정한다. once cue의
명시 tail 종료는 `sourceEndMs`와 `cue_end`를 함께 저장하고 Product 및 generation에 같은 값이
실리는지 확인한다. 공통 root yaw나 Server target/hit yaw를 함께 바꾸지 않는다.

### 컷신의 일시 숨김으로 미래 Effect 예약을 삭제하지 않는다

Local Pattern Preview의 장기 V1 인스턴스에는 아직 발생하지 않은 delayed emitter도 들어 있다.
컷신 진입에서 `Stop_BossOwner`로 삭제하면 pause/seek가 재구성할 때만 미래 이펙트가 보인다.
local preview는 해당 boss의 externally-sampled cue만 숨기고 동일 인스턴스·clock을 유지한다.
컷신 중 새 cue도 숨긴 상태로 시작하며 종료 시 표시한다. 실제 Stop과 Product 정리는 구분한다.

### 소품 이펙트는 LookInfo의 이벤트·모델·재질을 함께 복원한다

움직이는 돌 파편의 이름만으로 고정 소품의 원본을 대신하지 않는다. Prop row에서 LookInfo를
찾고 모델 두 재질·원본 애니메이션·material action과 Particle 부모 이벤트/지연을 함께 대조한다.
ParticleSystem 자체의 emitter delay와 부모 notify delay는 서로 다르며 각각 한 번만 적용한다.
MeshMaterial 모듈의 명시적 override는 기존 `sourceMaterialSlots`로 연결해 portable recipe와
native material의 일치를 유지한다. 단순 Required 치환이나 지원 검증 삭제로 통과시키지 않는다.
새 조합은 필요한 runtime extension만 가져오고, 실제 codec에서 UTF-8 표시명 길이와 미참조
baked-edge history까지 검사한다. 원본 비활성 notify를 켠 조합은 원본 자료 기반 저작으로 표시한다.
Full Restore animation index는 원본 action/stage/clip ID만 담는다. 파생 sector·조합은 일반
Effect catalog/resource tree에 등록하고, 설치 뒤 실제 검색 metadata reader까지 검증한다.
Effect 전환과 Sound의 성공은 분리한다. 새 Armed visual을 생성한 뒤 Sound만 실패했을 때도
기존 active visual은 종료해야 하며, 소리 실패는 별도 진단으로 유지한다.

### Sound Save의 저장 값과 파생 stage metadata를 구분한다

Pattern Sound의 stage index/duration은 편집·재생에서 계산하는 값이며 JSON 저장 계약이 아니다.
Save 후 storage-only 재읽기와 runtime metadata가 붙은 draft를 구조체 전체로 비교하면 파일 저장은
성공해도 owner 채택이 실패한다. 그 결과 Workbench source pin이 오래돼 Sound 행이 사라질 수 있다.
동일 draft generation과 직렬화한 저장 후보·실제 디스크 bytes를 비교하고 실패 시 기존 draft를 유지한다.
Preview 요청도 자기 잠금을 현재 dirty 상태로 갱신한다. 다른 창의 Render에만 맡기면 저장 전
잠금이 남으므로 동일 owner 예외와 다른 도구의 미저장 보호를 각각 유지한다.

### All Effects의 한국어 검색은 미게시 Pattern에서도 원본을 찾는다

발탄 Source Save 뒤 strict Product join이 실패해도 실제 Authored Effect의 검색·편집은 유지한다.
Resource/Existing Authored 목록은 ID·경로뿐 아니라 저장 표시명과 source의 모든 정확한 Pattern
연결 이름을 검색한다. 공유 Effect를 첫 owner의 표시명 하나로만 검색하면 다른 Pattern에서
사용하는 원본이 사라진 것처럼 보인다. 검색 metadata는 source admission에서 따로 검증·교체하고,
실패하면 이전 metadata를 보존한다. 이 metadata로 Product/Server 권한을 승인하지 않으며,
Open Editor는 기존 exact path·stable ID·미저장 전환 검증을 사용한다.

### 한 Stage의 서로 다른 타격은 같은 pulse 시계의 contact로 보존한다

검격·도넛처럼 도형과 피해가 다른 타격을 particle bounds나 단일 shape로 합치지 않는다.
optional `hit.contacts`는 기존 pulse의 count/order/atMs와 정확히 일치시켜 source·Product·
Server와 Client 저장을 함께 연결한다. 별도의 damage timer를 추가하면 중복 타격이 생긴다.
`PATTERNATTACKHIT/STAGE` bootstrap은 owner Stage 뒤로 정렬해야 하며 실제 전체 정렬 결과를
native Catalog로 읽어 검증한다. 구조체에 vector를 추가한 fixture는 ABI를 소비하는 모든 TU를
다시 컴파일한다. 타격 시각을 고쳐 일반 허용오차 안에 들어온 Sound의 exact 예외는 제거한다.
구현·검증은 [4연속·착지 판정 결과](09-28/2026-09-28_VALTAN_FOUR_SLASH_TRACKING_AXE_COLLIDER_RESULT.md)에 있다.

### Sequencer 호출 위치와 Effect·Sound의 재생 구간을 분리한다

ONCE Effect의 시작은 실제 Animation/Stage 안에서 검증하되 종료를 유한 Animation 끝으로
clamp하지 않는다. 500ms Animation에 붙인 긴 Effect를 조금 줄일 때 약401ms로 축소되는
회귀가 생긴다. 앞trim은 resource `playbackOffsetMs`까지 전달해야 앞부분이 다시 재생되지
않는다. optional offset의 부재와 명시0을 구분하고 Full Restore의 기존 원점을 보존한다.
Detail transform은 UI 임시값에서 끝내지 않고 typed draft와 같은 cursor의 Preview까지
연결한다. Save는 그 draft를 직렬화한다.

Composition publisher도 ONCE/CUE_END의 sourceClock.endMs를 보존하고 명시 종료를
occurrence 누적 시작·sourceStartMs·playRate로 stage clock에 변환한다. stage 끝 clamp를
재사용하면 native 저장은 성공해도 게시 시 잔여 수명이 잘린다. stage 초과 허용은
EFFECT_V1/ONCE/CUE_END에 한정하고 시작 범위와 다른 정책의 검증은 유지한다.

Sound의 WAV 구간은 Animation clip rate와 별개다. 앞뒤 편집은 offset/duration으로 저장하고,
다른 Stage로 이동할 때 actual Stage/action/clip join을 함께 바꾼다. stable ID에 들어 있는
생성 당시 clip 이름을 현재 owner로 다시 해석하면 복제·이동이 막힌다. 잘못된 실제 join은
계속 거부한다. 혼합 V1/Sound 그룹 편집은 두 owner 모두 rollback하는 transaction을 사용한다.
원본 library 검색 alias 복구는 asset 삭제/복원이나 gameplay cue 연결 변경과 구분한다.

### 발탄 ground warning도 기존 actor receiver 제외를 공유한다

native2614 확장원과 native2599 추적 도끼 원본02는 기존 쿠크 native3600군과 같은 volume
decal 경로를 쓴다. projector 높이를 낮추거나 Bloom을 바꾸지 않고 기존 GBuffer marker0/5의
actor bit8 및 source skin/equipment 필터에 정확한 native family를 연결한다. 정적 Map과
다른 decal profile은 유지한다. FXC·WARP 수신 분류 검증을 실제 Arena 화면 확인으로 대신하지
않는다. 구현·검증·설치 상태는 [Composition G29 결과](09-09/2026-09-09_VALTAN_COMPOSITION_AUTHORING_PARITY_RESULT.md)를 따른다.


### 컷씬의 Effect는 선택된 Pattern 박스와 별도로 추적한다

`Source cinematic`이 활성화된 장면은 선택된 takeoff 등의 Pattern cue와 별도 WorldSequence
`effectTracks`를 재생할 수 있다. 화면 모양이나 선택 박스만 보고 body ID를 단정하지 않는다.
실제 route → instance → template → effect track → asset ID를 추적한다. All Effects와 Resources는
같은 `ValtanCinematicEffectLibrary`를 사용하며, 공유 body와 서로 다른 재생 occurrence를 구분한다.
수정한 Bloom 등 body 값은 그대로 유지하고 목록 노출 때문에 asset을 복제하지 않는다.
Append는 표시된 Animation 대상을 자동 제안하되 사용자 dropdown 지정을 우선한다.

### Sequencer 연속 Seek는 다음 Late_Update 전에 해당 owner를 준비한다

- ImGui Render 중 Seek가 기존 객체를 지운 뒤 V1 spawn을 다음 프레임 pending commit까지
  미루면 그 객체는 Late_Update를 한 번도 통과하지 못하고 다음 Seek에서 제거될 수 있다.
  local boss preview owner만 기존 spawn 경로로 즉시 commit·follow anchor·절대 age sample한다.
- 직접 Seek는 앞 Stage에서 시작되어 아직 살아 있는 NATURAL 또는 명시적 tail도 복원한다.
  loop ONCE의 `cue_end`는 누적 source 시각으로 자른 tail이며, Stage 뒤까지 남는 경우만
  boss-action 종료 보존 flag를 사용한다. 명시 종료·owner reset·level cleanup은 유지한다.
- Source Save 완료 뒤 Product graph와 모든 V2 payload를 다시 로드하지 않는다. 최신 source
  revision 검증과 dirty-owner CAS는 유지하고 Product 갱신은 명시 Publish 경계에서 수행한다.
- Arena preset의 Server admission은 Pattern source/Product 일치와 별개다. 미게시 Pattern
  편집이 벽 복원·지형 preset을 잠그지 않으며 실제 session/world/상태 검증은 Server가 한다.
- 수치·저장·컴파일 증거는 [Composition 결과 G25~G27](09-09/2026-09-09_VALTAN_COMPOSITION_AUTHORING_PARITY_RESULT.md)를 따른다.


### Composition의 저장된 이펙트도 Preview 준비와 실제 수명을 확인한다

- Source Save로 추가한 V1 cue는 게시된 Product의 선행 준비 목록에 없을 수 있다. local Preview는
  선택한 Stage 경로의 실제 cue ID를 기존 preparation queue에 넣고 준비 완료 후 시작한다.
  준비 중 선택·draft·catalog 변경, 창 비활성화와 Reset은 예약 재생을 취소한다. 준비 실패를
  보이지 않는 재생 성공으로 처리하지 않는다. saved view의 generation과 live draft를 혼용하지 않는다.
- Effect box의 끝은 cue 시작점이나 Stage 끝이 아니라 기존 Playback의 요소·model cue·owner
  control과 particle/ribbon tail을 포함한 수명으로 계산한다. V2는 leaf/group의 rate와 stop policy를
  적용하며 무한 반복은 유한 자연 수명으로 위장하지 않는다. Preview tail과 Server Stage는 별도다.
- 다른 Stage로 drag할 때 기존 Stage 끝에 clamp하면 반복 입력이 unchanged가 된다. drop 위치의
  실제 Stage·clip occurrence를 찾아 owner와 source clock을 함께 변경하고 stable binding ID 및
  무관한 설정을 유지한다. 유효하지 않은 위치·stale draft는 이전 값을 보존한다.
- loop clip의 ONCE cue는 Stage 안에서 누적된 source clock을 사용할 수 있다. native clip 한 주기만
  검사하면 두 번째 주기 이후 cue가 누락된다. Composition에 독립 Append한 Full Restore는 cue
  시작 때 document age 0으로 재생하며 원본 clip 정렬용 source seek와 구분한다.

### 같은 리소스 표시명은 같은 source view로 만든다

- 공통 이름 helper만 사용해도 입력 view가 다르면 표시명이 달라진다. 발탄의 strict Product
  tree는 legacy alias를 포함하므로 Composition의 authoring view와 첫 owner가 다를 수 있다.
  All Effects는 실제 선택·재생용 strict tree를 유지하고 표시 map만 같은 admission 아래의
  authoring view에서 만든다. 두 실제 소비자 전체 inventory의 ID별 표시명을 대조한다.
- Source Save 뒤 animation 길이를 변경했다면 기존 rootmotion과의 duration도 게시 전에
  대조한다. 오래된 생성물은 현재 Encounter/Bindings로 기존 생성기를 실행해 갱신하며,
  사용자 animation을 되돌리거나 gameplay 게시의 검증 조건을 완화하지 않는다.

### 한글 UI 문자열은 실제 제품 컴파일의 문자 설정으로 검증한다

- UTF-8 파일에 추가한 narrow 한글 literal은 CP949 제품 컴파일에서 깨질 수 있다.
  최소 컴파일만 `/utf-8`로 강제하면 실제 빌드 실패를 숨긴다. 파일별 인코딩과 기존
  프로젝트 설정을 유지하고 새 표시 문자열은 UTF-8 byte escape 등 기존 설정과 무관하게
  동일 byte가 되는 방식으로 전달한다. ImGui에 전달할 UTF-8을 CP949로 바꾸지 않는다.
- 병렬 native probe의 개별 TU 컴파일은 `/Fo`뿐 아니라 `/Fd`도 전용 out 경로로 지정한다.
  제품 `vc143.pdb`를 공유하면 정규 빌드와 충돌해 C1041이 발생한다.
- 개별 TU 검사와 제품 Build의 source/execution charset 차이를 확인하고 최종 제품 빌드를
  통과한 뒤 성공으로 기록한다. 표시명 native 검사는 실제 byte와 검색·제목 일치도 확인한다.

### 본 부착 이펙트 그룹 복제는 runtime anchor도 독립화한다

- manual group ID만 바꾸고 runtime anchor slot을 공유하면 복제본의 socket 편집이 원본과
  충돌하거나 같은 그룹으로 다시 합쳐진다. 원본 source slot·실제 bone은 유지하고 선택된
  요소만 새 manual group 및 고유 runtime slot으로 분리한다. attached grouping key에도
  manual group ID를 포함한다. slot별 import-scale 정책이 달라지는 분리는 거부한다.
- source transform track이 있는 요소는 기존 Detail TRS 제한을 해제하지 않는다. 기존
  Anchor Position/Rotation으로 이동 궤적까지 변환하고 시작 시각을 옮길 때는 delay 증가분만큼
  sourceTimeOrigin을 반대로 이동해 원본 motion/material phase를 보존한다.
- runtime carrier 또는 선택 밖 transform owner/dependent가 있는 불완전 그룹은 수정하지
  않고 실패시킨다. 복제에는 기존 codec의 stable ID·내부 참조 remap을 사용한다.

### 스킨 NPC의 머리 위 UI는 raw bind bounds를 머리 좌표로 쓰지 않는다

- raw skinned vertex bounds는 inverse bind·현재 pose 적용 후 표시되는 몸체와 basis/크기가
  다를 수 있다. 실제 head bone의 combined matrix와 표시 world를 연결하고 preScale를
  중복 적용하지 않는다. bone 없는 모델의 fallback과 non-finite 실패 처리를 구분한다.
- 카메라 이동에 따라 체력바가 보스에서 밀리면 X/Y offset부터 바꾸지 않는다. anchor 월드
  위치와 실제 머리의 깊이, 현재 frame의 projection·최종 UI 변환을 함께 대조한다.
- 쿠크의 root 근처 bind anchor와 실제 head 수치·수정은
  [HUD 결과 G04 후속](09-27/2026-09-27_KOUKU_AUDIO_MARIO_HUD_POLISH_IMPLEMENTATION_RESULT.md)를 따른다.

### Composition Pause는 이펙트 객체의 시계까지 멈춰야 한다

- 모델과 timeline cursor만 멈춰서는 V1/V2 Effect layer의 frame delta가 멈추지 않는다.
  local authoring occurrence는 시작 age 주입뿐 아니라 매 sample의 절대 timeline age와
  layer의 자체 진행 차단을 함께 연결한다. pending spawn도 같은 age를 받아야 한다.
- Stage를 넘어 남는 NATURAL tail은 Stage-local age가 아닌 전체 sequence clock에 붙인다.
  새 Stage의 age가 0이 되는 것을 역 seek로 판단해 owner 전체를 지우지 않는다.
- 현재 cursor의 Pause/Resume는 기존 handle과 tail을 유지한다. 실제 seek/reset과 구분하고
  재생 target generation·owner 검증은 유지한다. Server 제품 clock은 local preview와 분리한다.
- 발탄의 적용 범위와 컴파일·clock 검증은
  [Composition 결과 G15](09-09/2026-09-09_VALTAN_COMPOSITION_AUTHORING_PARITY_RESULT.md)를 따른다.

### 헤더 변경 후 일부 도구만 접근 위반이면 OBJ 배치와 의존성을 확인한다

- 같은 클래스의 소비자 OBJ가 서로 다른 시점의 헤더로 컴파일되면 정상 null 검사도 잘못된
  멤버를 읽는다. 소스에 방어 코드를 추가하기 전에 fault symbol, 해당 OBJ의 생성 시각과
  멤버 offset, CL read tracking의 실제 헤더 dependency를 대조한다.
- PCH만 추적하고 해당 클래스 헤더가 누락된 unit은 증분 빌드에서 남을 수 있다. 결함을 확인한
  산출물만 백업·제거한 뒤 정상 Product 빌드로 재생성하고 헤더 추적과 실제 소비 경로를 검증한다.
  전체 clean이나 무관한 shader/PCH 재생성을 진단 대신 사용하지 않는다.
- CL.read block에 source CPP 한 줄만 남아 있는 경우도 같다. 인자를 추가한 함수의 이전 ABI
  링크 오류가 나오면 같은 변경 header의 include closure와 OBJ 시각을 함께 검사해 다른
  오래된 소비자까지 좁힌다. 재생성 뒤 해당 header가 CL.read에 실제 기록됐는지 확인한다.
- 발탄 local Preview는 Server 보스 미스폰 상태에서도 자체 presentation을 생성한다. 미스폰을
  종료의 원인으로 단정하지 않는다. 재현된 Debug 배치 불일치와 원래 WER의 확인 범위는
  [Preview 종료 조사 결과](09-27/2026-09-27_VALTAN_PREVIEW_CRASH_RESULT.md)를 따른다.

### 바닥의 삼각형 교차 무늬는 실제 배치의 공면부터 확인한다

- 사용자 좌표의 XZ를 덮는 placement와 설치 WModel의 정점·인덱스, preScale 및 저장 TRS를 함께
  측정한다. 다른 UV의 두 평면이 같은 깊이에 겹치면 정상 texture도 삼각형 단위로 깨져 보인다.
- 같은 맵이라도 진입로 바닥과 파괴 아레나 균열은 다른 geometry다. asset 이름이나 과거 재질
  복원 기록만으로 전역 shader·AA·조명 값을 바꾸지 않는다.
- 겹친 연결 바닥은 stable placement ID의 필요한 transform 필드만 수정하고 인접 면의 교차도
  확인한다. Placements 범위로 게시한 뒤 수치상 깊이 분리와 사용자의 화면 판정을 구분한다.
- 좌표 `(47, 10, -63.77)`의 배치 43/44와 1 cm 분리 근거는
  [발탄 진입로 결과](09-27/2026-09-27_VALTAN_FLOOR_TEXTURE_RESULT.md)에 기록했다.


### 개인 로컬 F5 선택은 LAN 동기화에서도 유지해야 한다

- `.vcxproj.user`의 host만 로컬로 고치면 다음 세션의 LAN sync가 팀 주소로 덮어쓴다.
  명시적 개인 테스트는 `Sync-TeamLanEndpoint.ps1 -EndpointMode Local`로 저장한다.
  기본 Saved는 선택을 보존하고, `-EndpointMode Team`만 공유 endpoint로 되돌린다.
  VS가 설정을 캐시하면 Reload/재시작 뒤 `Server + Client`를 선택한다. Client만 시작하고
  localhost 연결 실패를 Server 데이터 오류로 판단하지 않는다.

### 제품 Area의 MapTool 연결과 Camera 목록은 별도 등록이다

- MapCatalog와 camerashots가 있어도 MapTool의 editor registry와 runtime target resolver에
  제품 Level이 없으면 NO MAP AREA / Catalog NOT READY가 된다. 마하라카가 이 경우였다.
  기존 Level 소유 placement/batch/catalog/camera를 빌리고 self-motion pause/rebase까지
  연결한다. 화면 문구만 바꾸거나 Test로 강제 전환하지 않는다.
- Camera의 일반 컷신 목록과 `통합 컷신 편집`은 다른 소비자다. 후자는 발탄·쿠크
  Composition 세션 전용이다. 마하라카 worldsequence/camerashots는 전자에서 확인한다.

### 발탄 Composition Preview의 Stage 간 수명과 Full Restore 진입

- Resources는 All Effects와 같은 물리 inventory를 소비한다. runtime catalog admission만으로 목록을
  만들거나 여러 Pattern에 속한 Full Restore를 Common에만 남기면 실제 저장 자원이 사라진 것처럼 보인다.
  Product cue이기도 한 Full Restore는 두 경로를 유지하고 V1/V2 종류를 명시한다. 분리본 clip mapping의
  optional variantId는 action/stage/ID 일치와 token을 검증하며 실패 시 이전 index를 보존한다.
- 원본 Sequence의 서버 재생 owner와 Sequencer의 현재 Pattern 선택은 다르다. 실제 V2 누락을 판정하기
  전에 source owner로 이동한 Pattern의 Build_Timeline을 확인한다. Server active revision 관찰은
  현재 Source와 일치하는 admission이 아니므로 관찰값만으로 "active on this revision"을 표시하지 않는다.
- Full Restore의 Pattern 분류는 clip 이름만 비교하지 않는다. 원본 action 또는 실제 Product cue 연결과
  clip 이름을 함께 확인한다. 같은 이름의 clip을 공유하는 휠윈드 계열을 다른 source action으로 묶지 않는다.
- Composition Resources에서 바로 Open Editor할 때도 Effect Tool의 catalog/source metadata를 준비한다.
  All Effects를 먼저 열었던 세션에서만 Play All이 되는 상태를 정상으로 판정하지 않는다.
- combat object는 생성 Stage가 끝나도 살아 있다. 선택 Preview branch의 누적 Stage clock과 spawn offset으로
  object age를 계산하고, 이전 Stage의 돌·timed terminal visual을 유지한다. CONTACT hit를 임의 timed 폭발로
  취급하지 않으며 Product와 같은 natural visual tail을 사용한다.
- Pause/seek 대상 Effect는 기존 externally sampled handle을 사용한다. 별도 wall clock으로 진행시키지 않고
  역 seek·Stop·complete·대상 제거 때 handle을 정리한다. stage/action/clip-qualified Sound도 같은 clock과
  managed handle을 사용하고 paused 상태의 draft generation 변경까지 반영한다.
- Preview는 선택한 Logic 분기와 표현을 재현한다. 실제 player target·counter 결과·cover 피해 판정은
  Play Pattern의 Server 권위다. Source Save, Publish와 Server의 active revision 일치를 구분한다.

### World Movie의 V1 편집과 camera live 적용

- Movie Effect를 V1에서 열 때 원본 WORLD 배우·카메라·시계의 owner를 유지한다. 일반 Effect의
  별도 모델 preview나 Element Solo 시계를 함께 실행하지 않는다. 실제 Movie class/phase/Effect ID로
  연결하고 창 전환·편집 종료·Level 이탈의 임시 override 정리를 확인한다.
- Element 초안은 기존 Effect renderer로 먼저 준비하고 활성·pending occurrence 모두 성공한 뒤
  같은 handle의 인스턴스를 교체한다. 준비 실패는 이전 재생과 Product catalog를 보존한다.
  Product Save·임시 preview·다음 spawn을 동일한 완료 상태로 기록하지 않는다.
- prepared Effect의 Build_ResourceSignature는 Document 주소와 stable asset ID를 함께 사용한다.
  caller 문서를 prepare한 뒤 내용이 같은 복사본을 attach하면 signature가 달라 거절된다.
  불변 shared Document를 먼저 만들고 같은 객체를 prepare부터 active/pending attach까지 유지한다.
  이 주소는 process 내부 cache identity이며 저장 ID가 아니다. prepare 통과를 attach 성공으로 대신하지 않는다.
- Movie A의 Solo/Play Group 뒤 B로 전환할 때 isolation ID만 지우면 A의 filtered target이 남는다.
  새 문서 검증 후 이전 A의 전체 draft를 먼저 복원하고, 실패하면 isolation과 문서 선택을 유지한다.
  같은 asset의 Load Saved/Discard도 필터 없는 전체 저장본으로 복원한다. New/Save As는
  End Movie Editing 뒤에 수행하며, End는 임시 target 복원 후 isolation ID와 filter를 초기화한다.
- camera pose/FOV만 바뀌면 같은 Movie 시각에서 camera만 다시 평가하고 배우·Effect를 재시작하지
  않는다. camera box 시작/길이를 바꿀 때는 기존 전체 Apply 경로를 유지한다. key의 source 시각과
  현재 보간 sample, source horizontal FOV와 적용 vertical FOV를 구분한다.

### 스킨 모델 호버의 빈 공간 판정과 배경 덮임

- pose 전체 AABB는 broad phase일 뿐이다. 몸 사이 빈 공간도 선택되면 현재 palette로 변형한
  삼각형의 실제 교차와 표시 중인 body/part 수명을 확인한다. CPU triangle picking은 알파 텍스처
  구멍까지 읽는 GPU pixel picking과 구분한다.
- mesh draw 직후 depth-write 없이 그린 hull은 뒤의 바닥 draw에 덮일 수 있다. 최종 deferred
  overlay에서 visible silhouette을 임시 stencil bit로 표시하고 바깥쪽 픽셀만 그린 뒤 그 bit만
  반환한다. 공유 depth와 다른 stencil bit, 노란 hit와 LUT를 변경하지 않는다. 구현/검증은
  09-25 KOUKU_RESULT_TUNING의 G23에 기록한다.

### 쿠크 광기 충전과 특수 오브젝트 판정

- 광기 게이지는 최초 gate-progress 수신 전/관문 0에서 숨긴다. 보스 관문 없는 MARIO snapshot과 Server 승인된 F1 player-only 진입의 본인은 표시를 허용한다. Return to Start 뒤 이전 관문값이 남으므로 각 플레이어 Server snapshot 위치에 `Is_KoukuArenaStartArea`도 적용한다. 발판 검사만으로는 최초 jump.2부터 조기 표시된다. 타인 gauge의 `isValid = Health.Has_Madness()`에서도 같은 표시 조건을 적용한다.
- 광기 게이지 높이 정본은 `madness.feetOffsetMeters`이며 발 Transform origin에 더한다. Save와 다음 실행 Load_Config의 ProjectDataRoot 일치를 확인한다. float 1.3의 JSON 값 `1.2999999523162842`는 정상이며 화면 픽셀 offset과 구분한다.


- 최대 100인 게이지를 hit마다 정수 나눗셈하면 1% 미만 피해가 모두 소실된다. 실제 HP 전후 차와 소수 잔여량을 누적하고 변신·부활·관문 reset에서 잔여량을 지운다. 보호막 흡수량을 HP 피해로 세지 않는다.
- World object의 시각 effect나 파괴 HP가 있다는 사실은 공격·광기 판정의 구현을 뜻하지 않는다. 실제 cue의 생성·취소·파괴·보스 소유 수명과 Server overlap 소비자를 확인한다. 원본에서 확인한 반경·주기·충전량과 프로젝트 배율을 구분한다.
- Mario 중 timed clown의 만료 tick을 0으로 지우면 복귀 후 영구 광대가 될 수 있다. 입장은 기존 CLOWN만 허용하고 만료 tick은 퇴장까지 보존한다. [광기 구현 계획](09-24/2026-09-24_KOUKU_MADNESS_TUNING_IMPLEMENTATION_PLAN.md).

### 구조체 padding의 새 필드와 이전 Debug 복사자

- 새 필드가 기존 alignment padding을 채우면 sizeof가 같아도 이전 OBJ의 inline copy ctor/operator=는 새 필드를 복사하지 않는다. parser TU만 최신인지 확인하지 말고 간접 소비자와 실제 COMDAT 복사 구현을 대조한다. 저장 JSON에 없는 optional Source In 값이 비정상이라면 데이터를 0으로 덮거나 수명 validation을 풀기 전에 이 경로를 확인한다.
- 확인된 Debug Animation_Tool.obj는 header 추적이 빠져 있었으며 이전 산출물과 tracking을 보존한 뒤 해당 OBJ 하나만 정상 재컴파일했다. 새 CL.read의 Composition header 포함과 복사자 일치를 확인한다. 컴파일 복구는 실행 중 EXE 교체가 아니므로 링크·사용자 재시작을 구분한다. [복구 결과](09-24/2026-09-24_KOUKU_SEQUENCE_SOURCE_IN_ABI_RESULT.md).

### 단독 쿠크 보스 처치 뒤 관문 전이

- Kill Boss는 HP 사망 경로를 사용하지만 standalone 보스에 raid owner가 없으면 legacy Advance_Gate의 spawn/teleport가 intro를 생략한다. ADVANCE/RESTART 투표도 Begin_KoukuRaidPreparation을 사용해 전원 READY 뒤 cinematic과 authored arrival로 진입한다. 명시 ENTER_GATE3의 deck→전투 입장은 별도 계약이다.
- 다음 cinematic commit 직전에 이전 audition·Bingo·CardMaze owner를 정리하고 Mario stage cursor를 1로 초기화한다. 준비 실패 전에는 기존 상태를 보존한다. [구현 계획](09-24/2026-09-24_KOUKU_KILL_GATE_SEQUENCE_IMPLEMENTATION_PLAN.md).

### Loading 완료 프레임의 이전 UI와 새 Level 렌더 큐

- Loading에서 runtime UI Update 전체를 생략하면 그 안의 비활성 Level hide 분기도 실행되지 않는다. STATIC에 소유된 Lobby 이미지와 캐릭터 선택 창은 Loading 진입 정리에서 명시적으로 숨긴다.
- 이전 Level의 Late_Update 뒤 activation하고 즉시 Render하면 새 Level은 아직 렌더 큐를 만들지 않았고, Loading 소멸자가 기존 chrome까지 숨겨 이전 STATIC UI가 노출된다. 전환은 다음 MainApp Update 시작에 소비하여 새 Level의 Update/Late_Update와 첫 Render를 같은 프레임으로 묶는다. 초기화 실패 rollback과 사용자 화면 확인은 별도다.
- 근거: [로딩 잔상 결과](09-24/2026-09-24_LOADING_LOBBY_FLASH_RESULT.md).

### 발탄 원본 연출은 MapTool 개방 전에 배우 Prototype이 필요하다

- WorldSequence 문서와 모델 파일이 있어도 `CWorldSequenceObject::PROTOTYPE_TAG`를
  해당 Level에 등록하지 않으면 Prewarm Clone이 실패한다. Valtan Loader가 등록하고
  MapTool은 이를 재사용한다. MapTool에서 성공했다는 사실은 Boss 경로의 생성 성공
  증거가 아니다. 원본 배우 준비 실패 시 Camera만 계속 표시하지 않는다.
- Workbench 후반 샘플링은 자신의 Effect world-root를 같은 프레임 commit하고
  원본 사운드에 Timeline pause도 전달한다. pending Effect seek 자체는 허용되므로
  commit 시점 차이를 영구 사운드 누락 원인으로 단정하지 않는다.

### 새 클래스 추가 뒤 기존 Camera 크기 저장본의 이행

classSizeMultipliers의 필수 키 수만 늘리면 기존 여섯 클래스 Camera JSON을 읽지 못하고 빈 source baseline 때문에 Save도 외부 변경으로 거절된다. 새 GuardianKnight 키만 없는 유효한 이전 문서는 기존 여섯 값을 보존하고 새 값 1로 이행한다. 다른 키 누락·unknown·손상값은 계속 거부하고 다음 Save에서 일곱 이름을 저장한다. 초기값 변경은 실제 map 저장본과 함께 확인하며 freshness 검사를 제거하지 않는다. 재현과 네 저장본 반영은 [크기 결과 G04](09-20/2026-09-20_CHARACTER_SIZE_AND_ALTV_RESULT.md#g04-2026-09-22-크기-save-회귀-복구)에 기록한다.


### 보스 연출 클립의 Character 계약과 로딩 초기화 rollback

- BossCatalog의 bodyModel·animationSetId는 Client ActorCatalog가 허용하는 `Character/.../*.wmodel`이어야 한다. 같은 골격의 Map 전용 연출 모델을 직접 열 수 있다는 이유로 catalog ID에 넣으면 전체 catalog 초기화가 실패해 다른 Level 입장도 막힌다. publisher의 donor 지원도 같은 경로 계약으로 검증한다.
- 연출 클립은 기존 Character WModel에 WANM section만 병합한다. skeleton/rest basis 일치와 이름 충돌을 확인하고 기존 geometry·material·skeleton·clip section을 보존한다. animation index 0이나 단일 클립 모델을 가정하지 말고 이름으로 찾는다. 재생성 도구도 canonical Character 경로를 반환해야 한다.
- 모델 단독 로드·Server publisher 성공만으로 Client 입장을 검증했다고 기록하지 않는다. 실제 Client ActorCatalog reader와 WorldSequence reader를 확인한다. 연출 이관은 실제 CModel 본 행렬 대조와 Resources 배포 목록을 함께 남긴다.
- Loading UI를 Layer에 먼저 등록한 뒤 Loader 초기화가 실패하면 Level이 교체되지 않아 Lobby 글자와 orphan Loading 배경이 겹칠 수 있다. 생성한 instance의 sprite만 hide/remove하고 최초 실패 source/detail을 보존한다. 실패 cleanup에 전체 Level 자원 clear를 사용하지 않는다.
- ImGui ViewportsEnable 상태에서는 취소한 프레임도 EndFrame 뒤 UpdatePlatformWindows를 호출해야 다음 NewFrame이 유효하다. 로딩 UI를 강제로 Cancel하는 우회로 폰트·모델 실패를 가리지 않는다. 취소 시 RenderPlatformWindowsDefault draw는 필요 없다.
- 이번 연결 범위와 근거: [Character 연출 이관 결과](09-21/2026-09-21_KOUKU_CHARACTER_CINEMATICS_RESULT.md), [리소스 목록](09-21/2026-09-21_KOUKU_CHARACTER_CINEMATICS_RESOURCES.md).


### 게시 데이터의 Git 전달과 bootstrap schema 불일치

- `Client/Bin/DataFiles`, `Server/Bin/DataFiles`의 게시 snapshot은 정본·publisher·소비 schema와 같은 PR로 전달한다. 수신 PC에서 전체 publish나 navigation bake를 반복하는 것을 기본 절차로 두지 않는다. 생성물 수동 편집 금지는 Git 전달 금지와 다른 규칙이다. 신규 출력도 일반 `git add`에 포함하며 Resources·컴파일 산출물·staging/rollback·로컬 cache는 계속 제외한다.
- `World simulation failed to initialize ... Item bootstrap header is invalid`이면 실제 `Items.bootstrap` header와 실행한 Server reader를 먼저 대조한다. v2의4열 ITEM 데이터와 v4의7열 reader가 함께 전달된 사례가 있었다. 같은 수신본의 Valtan `ClearRewards.bootstrap`도 v1 데이터와 v2 reader가 달랐다. 오래된 게시 데이터를 새 코드와 합치면 pull/build 성공만으로 실행 준비가 끝나지 않는다. 각각 `Publish-ItemCatalog.ps1 -Mode Publish`, `Publish-ValtanClearRewards.ps1 -Mode Publish`로 현재 정본을 재게시하고 갱신된 bootstrap을 같은 PR에 포함한다. header 숫자만 수정하거나 schema 검사·필수 로드를 우회하지 않는다.
- 정본 Product runner는 Items·Valtan ClearRewards의 `CheckPublished`로 현재 정본의 전체 생성 행과 게시본을 읽기 전용 비교한다. `invalidRuntimeInputs`와 `runtimeDataChecks`를 확인하며, compile PASS를 모든 domain의 실행 준비 완료로 설명하지 않는다.
- `Process gameplay generation failed to initialize ... Gameplay bootstrap header is invalid`이면
  Shared `GAMEPLAY_BOOTSTRAP_FORMAT_VERSION`과 실제 `Gameplay.bootstrap` header·행 수를
  대조한다. Madness v37 코드에 v36 게시본이 남아 같은 오류가 발생했다. 승인된 최신 저작본을
  공식 `Publish-GameplayBalance.ps1 -Mode Publish -BalanceProfile Retail`로 게시하고 실제
  Server catalog load와 bounded headless 초기화를 확인한다. 파일 존재·빌드 성공만으로
  Gameplay generation 호환성을 확인한 것으로 안내하지 않는다.
- publisher·reader schema 변경자는 대응 출력을 갱신하고 실제 consumer에서 읽히는지 확인한다. 특정 domain 오류를 전체 재게시나 Clean/Rebuild로 우회하지 않는다. Git 수신, C++/shader 빌드, 데이터 게시, 실행 중 메모리 reload/Server 재시작과 사용자 화면 확인은 별도 단계다.

### 쿠크 이펙트 상한·컷씬 UI·배경 진단

- V1 scene/owner reservation과 V2 GROUP의 Mesh/Decal/Engine provider cap은 다른 경로다.
  occurrence 종류를 확인하고 queue overflow, spawn 거절, draw 실패, Server timeout을 구분한다.
- Release Renderer 실패 기록을 Debug guard 안에 두지 않는다. 파일은 bounded 기록하고
  같은 시각의 Client session과 Server RoomPerf를 비교한다. 상한 확대는 실제 도달 증거 뒤에 한다.
- 컷씬 UI 숨김은 widget visible/open을 덮어쓰지 않고 공통 렌더/입력 억제로 처리한다.
  연출 자체의 fade는 보존하고 종료·실패·Level 이탈 때 억제를 해제한다. normal combat
  follow/static 카메라를 컷씬으로 분류하지 않는다. 숨긴 damage event는 이후 재생하지 않는다.
- 배경만 검고 배우·이펙트가 보이면 전체화면 fade로 단정하지 않는다. 컷씬 경계의
  map visibility, scene profile, light/camera owner를 기록해 재현 근거를 확보한다.


### Release 쿠크 레이드의 lifecycle 소비와 Lobby 복구 문구

- 제품 쿠크 레이드도 기존 audition result/lifecycle packet을 사용한다. packet 이름의
  DEBUG 접두사나 저작 UI의 Debug 경계로 제품 수신 소비자까지 감싸지 않는다.
  MainApp은 NetworkManager dispatch 다음에 Kouku audition service를 모든 구성에서
  갱신한다. 소비되지 않은 bounded lifecycle 큐를 용량 확장으로 숨기지 않는다.
- Lobby의 `Server entry failed.`는 복구 공통 문구다. 같은 PID의 session diagnostic에서
  reason·WSA 오류·recovery source를 읽고 Server의 같은 시각 종료와 대조한다.
  WSA10055와 Release 소비 누락은 확인해도 packet별 증거 없이 특정 패턴의 admission
  실패로 단정하지 않는다. 근거는 쿠크 통합 RESULT의 2026-09-19 Flow 재설정 항목이다.

### 차원술사 탑승 AnimationSet의 import 배율

- AnimationSet은 skeleton hash·이름·부모가 같아도 armature의 import 배율이 다를 수 있다.
  `Attach_AnimationSet`은 animation만 복사하므로 target body의 rest·기존 clip과 donor의
  root key를 함께 실측한다. 차원술사 body 1 / Ride 100은 지정한 6개 donor의 root scale만
  100으로 나눠 교정하며 전역 Character·차량 역배율로 우회하지 않는다.
- 실제 body의 mesh·inverse bind로 skin bounds를 검사하고, 교정하지 않는 필드의 byte 보존과
  재실행 무변경을 확인한다. Resources는 기존 Drive 경계로 별도 전달하며 실행 중 Client의
  메모리까지 갱신됐다고 표현하지 않는다. 근거는 [탈것 결과 G09](../JS/09-14/2026-09-14_VEHICLE_ADDITIONS_RESULT.md)를 따른다.
- 본인만 정상이고 다른 PC에서 같은 탑승자가 100배 커지면 관찰 PC가 선택한 Resources의
  `Character/DimensionMaster/AnimSets`를 비교한다. 원격 캐릭터도 관찰 PC의 donor를 사용하며
  Release 실행 ZIP·Git pull은 별도 Resources 교정본을 전달하지 않는다. 정상 donor에 다시
  전역 0.01을 곱하지 말고 기존 교정 6개를 같은 상대 경로로 전달한다. 설치 도구는
  `Tools/VehiclePipeline/Install-DimensionMasterRiderScalePatch.ps1`이며 알 수 없는 파일은 보존한다.

### 실행 ZIP의 DataFiles와 옵션 팝업 클릭 소비

- Git pull과 Product Build는 runtime publish를 대신하지 않는다. 특히 KoukuSaydon owner에는
  Navigation/Composition이 없으므로 전체 실행 배포 전 Client/Server owner의 결과를 확인한다.
  EXE/DLL/CSO와 양쪽 Bin/DataFiles를 함께 포장하고 region manifest가 참조하는 파일까지
  검증한다. 직접 읽는 Data JSON은 같은 commit 또는 검증된 보충분으로 전달한다.
- `EffectCatalog.json`을 보충할 때는 모든 direct-authored `authoringPath` JSON도 함께 검증·
  전달한다. Client 초기화는 변경된 effect뿐 아니라 catalog 전체의 참조 파일 존재를 검사한다.
  catalog 전체와 일부 authored JSON만 전달하면 이전 Data를 가진 PC에서 흰 창 뒤 종료될 수 있다.
  실제 실패 원인은 같은 PID의 `ClientStartup.user.log`/`ClientExit.user.log`로 확인하며,
  설치기 headless PASS를 Client 시작 성공으로 기록하지 않는다.
- WorldSequence에 기존 runtime 필드를 추가할 때 Map publisher·Client codec뿐 아니라
  Composition의 엄격한 source validator도 같은 계약을 소비해야 한다. `colliderTracks`와
  `loopFullPresentation`을 unknown으로 거절하는 경우 필드를 제거하지 않고 타입·시간·shape·
  binding 제약을 일치시킨다.
- 열린 옵션 콤보는 popup 항목이 입력을 먼저 처리한 뒤 하위 UI의 같은 클릭을 차단한다.
  공용 `Is_Clicked`가 소비 플래그를 검사하는데 popup 처리 전에 그 플래그를 세우면
  커서 preset을 포함한 모든 combo 선택이 막힌다. popup 선택과 바깥 클릭 차단을 함께 확인한다.
- 세부 증거는 [쿠크 통합 결과의 배포 재수정](09-18/2026-09-18_KOUKU_RAID_COLLIDER_SOUND_INTEGRATION_RESULT.md)을 따른다.

### Sprite 축 회전과 source 곡선의 기본값

- Source sprite의 EPAL_Rotate_X/Y/Z는 일반 camera billboard와 다르다. 원본 축 회전을 편집하려면 Billboard를 유지하고 명시적인 axis-follow 옵션으로 emitter 축을 변환한다. 옵션을 끄면 기존 동작을 유지하며 local space는 현재 root, world space는 출생 root를 사용한다. camera와 축이 평행할 때도 finite basis와 실제 quad의 앞면을 검사한다.
- 비어 있는 nested RawDistribution을 0으로 단정하지 않는다. 해당 class의 CDO 상속을 확인하고 손 본 basis와 root snapshot의 preScale·크기를 따로 측정한다. 서로 다른 notify의 궤적을 연결할 때 해독하지 못한 원본 방향 설정을 복원 완료로 표현하지 않고 원본 곡선 기반 저작 보정과 구분한다.
- sector 메시의 실제 삼각형 coverage와 native alpha mask의 최종 pixel coverage는 별도다. 화면에 빈 구간이 보여도 emitter 복제·회전으로 채우기 전에 mesh/UV/material 입력을 확인한다.

### Orbit event 위치와 피자 layer 경계

- Orbit으로 이동한 입자의 종료 폭발은 frame과 동일한 offset 평가 및 root 변환을 사용한다. 원래 emitter 위치 또는 offset을 두 번 적용한 위치로 발사하지 않는다. 회전된 root에서도 실제 마지막 입자 위치와 event 위치를 대조한다.
- sector mesh가 만든 빈 각도를 전체 이펙트의 공통 mask로 해석하지 않는다. halfcylinder·sprite·출생 영역 제한과 material opacity/flow mask를 각각 확인한다. 일부 mesh의 빈 각도만 측정해 모든 layer에 정확90도 안전 영역이 적용됐다고 기록하지 않는다.

### 공중 착지의 stage 소유권과 native 시각

- 공중 높이 제어를 다른 보스 clip에 재사용할 때 stage 시작과 animation startOffsetMs를 합친 실제 창을 확인한다. source clip 이름 대신 설치 모델의 남은 하강량으로 admission한다. 타깃 착지는 XZ를 고정해 native lateral root sway가 선택 위치를 바꾸지 않게 하되, 착지 source Stage가 끝나면 다음 Stage의 native root를 재개하여 후속 상승까지 지면에 고정하지 않는다.

### Effect V1 carrier admission과 삭제 시 재생 선택 정합

- CASCADE_BEAM_V1은 기존 Playback의 직접 native 경로다. runtimeCarrier가 존재한다는 이유만으로 Ribbon/baked 전용 projection을 강제하지 않는다. 혼합 문서는 Beam을 보존하며 변환이 필요한 carrier만 projection한다. codec Parse만 성공한 것을 Catalog/Play All 성공으로 기록하지 않는다.
- 선택 요소 삭제는 문서·legacy isolation·Sequencer transient previewElementIds를 하나의 staged 변경으로 맞춘다. 삭제 전 ID로 새 문서를 preview하는 오류를 missing ID 검증 완화로 숨기지 않는다. stage 실패는 선택과 문서까지 보존한다.
- 부착 그룹의 전체 방향은 공유 socket 원점에서 수정한다. Element 평균 중심 회전과 혼동하지 않는다. native recipe/localSpace/각 요소 offset은 보존하고 같은 runtime slot의 서로 다른 socket 정의를 만들지 않는다.

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

### 화면 픽셀 수와 맵의 투영 배율을 구분한다

- DPI 선언이 없는 1280×720 Client가 150% 배율 모니터에서 1920×1080 크기로 보일 수 있다.
  설정 상수나 캡처 외곽 크기만으로 내부 해상도를 판정하지 않고 EXE manifest, 실제 HWND DPI,
  client rect와 swapchain/viewport/RT 크기를 함께 확인한다. 같은 종횡비·카메라에서 전체 화면을
  같은 비율로 확대하면 맵이 차지하는 정규화 비율은 그대로이며 픽셀 선명도 문제를 분리한다.
- PerMonitorV2와 실제 physical client 크기를 연결할 때 main buffer만 바꾸지 않는다. full-size
  MRT/HDR/scene post/source-light-mask depth와 half-size Bloom/SSAO/DSV/texel, viewport/화면
  projection을 stage한 뒤 한 번에 commit한다. 0 크기와 자원 준비 실패는 기존 자원을 유지한다.
- MRT가 공유한 RenderTarget wrapper는 유지하고 GPU 자원만 바꾼다. ResizeBuffers 전 context와
  저장된 backbuffer RTV 참조를 모두 해제하며 End_MRT의 이전 output 참조를 다음 frame까지
  붙잡지 않는다. 같은 해상도의 exclusive fullscreen 전환도 새 buffer 실현이 필요하다.
- 원본 카메라 volume의 값은 실제 구역 포함 여부와 함께 소비한다. 쿠크19m volume은 입구의
  Z[-86.428,1.409]m에 있고 세이튼 전장 Z737.53m는 밖이다. 09-14의 이를1관문 전체에 적용한
  해석을 전장 근거로 재사용하지 않는다. 바깥의 공통16m CDO 기준을 원작 최종 camera framing
  검증 완료로 확대하지 않는다. 맵 크기, 개별 Effect source/배율, 조명·tone/LUT는 따로 대조한다.
- 구현·수치·빌드와 사용자 화면 검증 경계는 [물리 해상도·카메라 결과](09-20/2026-09-20_NATIVE_RESOLUTION_DPI_AND_KOUKU_CAMERA_IMPLEMENTATION_RESULT.md)를 따른다.

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

- 발탄 native2614 원형의 inner는 row0/lane1이고 native2615 부채꼴은 row0/lane2다. 이름이 같아도 typed native binding의 실제 row/lane을 확인한다. source-clock key는 start delay와 `sourceTimeOriginSeconds`를 함께 반영하고, 독립화 시 둘을0 기준으로 맞춘다. shader 전체나 projector 크기를 바꾸지 않고 다른 native lane과 원본 fade를 보존한다.
- PlayDecalEffect의 optionalName `Decal`이 있는193-byte payload는 기존187-byte 형식보다 뒤쪽 field offset이6바이트 밀린다. raw 길이·문자열 길이·내용·source hash를 확인하지 않고 SkillDecal/SkillEffect/fade를 고정 offset으로 읽지 않는다. 부착된 sector 방향은 notify TRS·snapshot source basis·projector 전방을 함께 검사하며 전체 asset 회전이나 배율을 강제하지 않는다. 후보와 설치·GPU 표시를 구분한 검증은 [발탄 G16 결과](09-09/2026-09-09_VALTAN_COMPOSITION_AUTHORING_PARITY_RESULT.md#2026-09-28-g16-4방향-부채꼴-분리와-추적-도끼-큰-원형-후보)를 따른다.

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

### 콜로세움 인원 변경과 좌표 차원의 경계를 분리한다

- 인원 제한 변경은 좌표 차원을 바꾸지 않는다. Colosseum 팀 슬롯을4인으로 확장할 때
  카메라 XYZ 파서까지4회로 바뀌어3개짜리 JSON/출력 배열의 범위를 벗어났다. 입력 길이 검증과
  같은 실제 배열 크기로 순회하고, 팀 슬롯 검사와 실제 Client camera 파서 검사를 따로 실행한다.
  서버 입장 계약 PASS나 C++ 컴파일 성공을 Client 컷신 로드·화면 성공으로 기록하지 않는다.

### 클릭 이동 예측의 위치·높이·회전·카메라를 각각 검증한다

- prediction helper가 정한 유효 이동 시간을 Character에서 다시100ms로 자르지 않는다.
  ACK 전 follower와 ACK 후 표시 경로가 같은 시간을 소비해야 재클릭이 긴 frame의 속도를 바꾸지 않는다.
- 가까운 waypoint도 남은 실제 거리를 frame budget에서 뺀 뒤 소비한다. 2cm 도착 tolerance로
  무료 이동하면 corner 속도 상승과 helper가 아직 도착하지 않았는데 RUN이 끝나는 결함이 생긴다.
- Server의 새 이동 경로는 임시 vector에서 검색·정리한 뒤 commit한다. 실패한 retarget은
  기존 경로·index·goal/request를 보존하고 처리 sequence만 ACK한다. 실제 충돌·도착·상태 잠금의
  이동 종료와 경로 검색 거부를 구분한다. 재현과 수정은09-22 PROFILER RESULT의 G21에 둔다.
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

- 수만 draw에 raw CPU scope를 항상 기록하면 scope cap에 걸리고 측정 자체가 frame을 늘린다. 기본은 pass/counter이며 per-draw detail은 별도 비교 실험에서만 켠다. frame의 droppedCpuScopes와 window 합계는 누적 drop과 구별한다. cpuScopesWithinBudget는 overflow가 없었다는 뜻이며 모든 함수의 계측 완료를 뜻하지 않는다.
- Capture/Reset/detail 변경은 frame 경계에서 적용하고 pause 전 열린 worker scope가 재개 history에 들어오지 않도록 epoch를 확인한다. worker가 mutex를 기다린 뒤에도 epoch를 다시 확인한다. Reset 후 첫 frame interval은 이전 pause 시간을 포함하지 않는다. export metadata는 저장 시점 snapshot이며 과거 모든 frame의 설정 증거가 아니다.

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

### 통합·checkout과 shader 증분 캐시의 무효화를 구분한다

브랜치 이름은 compiler 입력이 아니다. 같은 tree로 전환해 working-tree byte와 mtime가 그대로라면
branch를 바꿨다는 사실만으로 FXC를 다시 실행할 이유가 생기지 않는다. 실제 동일-tree 전환의
C++/shader 입력 2,273개에서 SHA-256과 mtime가 모두 보존된 대조는
[Release 통합 결과](09-29/2026-09-29_RELEASE_RAID_PR_INTEGRATION_RESULT.md)를 따른다.
측정 파일 `out/ReleaseValidation20260929/branch-no-input-change.json`은 해당 실행의 로컬 증거이며,
다른 checkout에서도 파일이 보존된다고 대신 증명하지 않는다.

반면 서로 다른 tree 사이의 checkout, safety stash와 복원, 파일 전체 복사, 내용이 같은 생성물의
재저장은 디스크 파일을 다시 써서 mtime를 바꿀 수 있다. 기존 CSO가 최신 source 내용으로 만들어졌어도
include가 더 새 시각이면 MSBuild dependency tracking은 그 입력을 오래된 출력의 원인으로 판단할 수
있다. 이때 diagnostic에 적힌 입력 경로·수정시각과 Git reflog/보존 기록을 대조한다. 내용이 같은
공통 HLSLI까지 원인으로 지목됐다면 byte 동일성과 timestamp invalidation을 함께 기록한다.
실제로 바뀐 C++/헤더도 있을 수 있으므로 모든 재컴파일을 불필요했다고 확장하지 않는다.

`git diff`의 의미상 차이, Git index blob, working-tree bytes, mtime는 서로 다른 정보다.
`core.autocrlf`와 `.gitattributes` 때문에 index LF/working-tree CRLF가 될 수 있다.
`git ls-files --eol`과 실제 byte hash를 확인하고 “내용이 같다”가 어느 기준인지 명시한다.
EOL만 바뀌어도 byte hash와 tracking은 달라질 수 있다. 다른 작업 보존을 생략하거나 줄바꿈 설정을
임의로 바꾸는 대신, 통합 전 안전 snapshot을 남기고 필요한 변경만 현재 디스크 형식에 맞춰 적용한다.
작업 보존과 캐시 보존은 별도 목적이며, 재컴파일이 발생했다고 stash의 원본 데이터가 유실된 것은 아니다.

기존 출력이 있는 checkout과 새 worktree의 비용도 구분한다. worktree는 소스 격리이며 원본의
OBJ/PCH/CSO와 `.tlog`를 자동 공유하지 않는다. 유효 캐시가 없는 최초 빌드는 정상적으로 비용이 든다.
원본 checkout의 실제 report가 include 변경을 지목했다면 이를 막연히 cold-worktree 문제로 설명하지
않는다. 출처·도구체인·입력 대응이 불명확한 다른 디렉터리의 OBJ/CSO를 섞어 캐시를 만들지 않는다.

### 기존 CSO는 정상 tracking과 입력 대응을 보존해 재사용한다

- 통합 전에는 수정 예정 source/include와 공용 props·프로젝트 metadata, 활성 output/IntDir/tlog,
  현재 toolchain을 확인한다. source/include 변경이 없는 C++ 수정은 같은 경로의 정상 Product Build로
  필요한 OBJ와 링크만 갱신하고, 유효 CSO는 MSBuild가 재사용하도록 둔다.
- source/include bytes뿐 아니라 define/macro, entry/profile, compiler 옵션, SDK/도구 host,
  구성과 command의 출력 경로 표기까지 같아야 동일한 compiler 입력이라고 볼 수 있다. 같은 디렉터리를
  가리켜도 `/Fo` 등 command 문자열이 달라져 tracking이 무효화되는 경우를 별도 확인한다.
- `source mtime < cso mtime`만으로 올바른 출력을 보장하지 않는다. 빌드 도중 source가 바뀌었거나
  다른 구성의 CSO를 복사했을 수 있다. 기존 성공 결과의 입력/출력 대응과 tracking을 확인하고,
  배포에서는 활성 producer의 CSO와 실제 consumer 복사본의 존재·내용 hash도 확인한다.
- 생성기와 복사 단계는 결과 bytes가 같으면 다시 쓰지 않는다. 통합 후보를 안전하게 보존한 상태에서
  필요한 내용만 반영한다. 기록 없이 과거 source 시각을 복원하거나 CSO를 touch하고 tracking을 지워
  최신이라고 표시하는 것은 재사용이 아니다. 실제 shader 변경·누락은 정상 Build로 갱신한다.
- source가 바뀌지 않았다는 예상과 달리 FXC가 시작되면 또 다른 전체 Build/Clean/Rebuild를 추가하지
  않는다. 기존 diagnostic에서 최초 변경 include, command 차이, toolchain 전후, 출력/추적 누락을
  먼저 분류한다. 원인 확인을 위해 같은 비싼 작업을 무조건 다시 실행하지 않는다.

### FXC 병목은 작업 시간과 실제 compile 입력으로 설명한다

Product report의 Client 전체 시간은 FXC 시간과 다르다. binary/diagnostic log에서 FXC task의
시작·종료, C++ CL, Link를 구분하고, shader별 command 메시지와 object-save 성공 메시지가 있으면
그 사이의 경과를 함께 적는다. command→save 시간은 scheduling·다른 compiler와의 경합을 포함할 수
있으므로 개별 프로세스의 순수 CPU 시간 또는 optimizer 시간으로 부르지 않는다. 마지막 CSO 저장이
FXC task 종료 직전이라는 사실은 그 실행에서 기다린 마지막 shader를 보여주지만 함수별 원인은 아니다.
여러 FXC가 병렬로 실행되므로 shader별 경과의 합을 사용자 대기시간으로 계산하지 않는다.

실제 FXC task의 `MaxProcessCount`와 C++ `ProcessorNumber`를 따로 확인한다. 공용 props의 기본값만
읽어 해당 실행의 병렬도라고 보고하지 않는다. 상위 property나 명령의 override가 적용될 수 있고,
병렬 수를 늘리면 CPU·메모리 경합이 생길 수도 있다. CPU/메모리 측정 없이 최적 병렬 수나 병목 원인을
확정하지 않는다. outputChanges의 OBJ/CSO 수는 성공 출력의 크기·시각 변화이며 실패 시도 수나 source
내용 변화 수가 아니다. 두 실행 비교는 toolchain·command·실제 입력 범위·cache 상태도 함께 적는다.

`Shader_VtxAnimMeshBinary`와 `Shader_VtxMeshBinary`는 vertex shader 하나가 아니라 여러 VS/PS,
render state와 pass를 담은 `fx_5_0` effect다. 공통 HLSLI 하나의 수정은 여러 group wrapper를
재컴파일시키는 범위 문제이며, 기본 effect 한 개의 긴 compile 시간은 별도의 compiler workload다.
단순 줄 수·파일 크기·pass 수만으로 optimizer 시간이 비례한다고 판단하지 않는다.

### AnimMesh·Mesh 최적화는 기존 group 선택과 pass 공유부터 실측한다

- 기본 `SOURCE_CHARACTER_PROGRAM_GROUP=0`을 “모든 SourceCharacter group 포함”으로 해석하지 않는다.
  Base/Light leaf의 `!defined(GROUP) || GROUP == <선택값>` guard와 wrapper define을 실제로 평가한다.
  AnimMesh의 `ARTIST_NATIVE_MODEL_ONLY`는 일반 effect dispatch와 Kouku native group들을 이미
  제외하고, 비영 group의 `EFFECT_NATIVE_DECLARATIONS_ONLY`는 관련 함수 본문을 더 제한한다.
- 그래도 기본 AnimMesh의 model 경로는 Artist·Vehicle·Lance VA·ALTV 함수와 입력 변환을 함께
  포함한다. 좁은 모델 dispatch가 읽는 큰 native group 파일에 미사용 함수도 들어 있다면 실제 호출
  의존성을 기준으로 include 범위를 줄일 후보가 된다. 원시 include 줄 수는 전처리 후 source 크기나
  최종 GPU instruction 수와 다르며, 후보를 찾았다고 성능 개선을 확인한 것은 아니다.
- `PS_MAIN_EFFECT_MODEL_CUE_NATIVE`의 scene-read/bloom 보정은 같은 native 평가를 read mode별로
  여러 번 호출한다. 이 경로는 inline/분기 최적화 비용의 조사 후보지만, 색·투과·bloom 계산의 동등성
  증명 없이 호출을 합치거나 제거하지 않는다. compiler phase별 시간은 별도 계측 전까지 미확정이다.
- 기본 Mesh의 map-forward native 프로그램, foliage/stand wind, alpha/sky/water/shadow/outline 등
  여러 entry의 callgraph를 구분한다. group0의 비용과 group wrapper 전체 재컴파일 수를 섞지 않는다.
  무조건 모든 native material이 포함돼 느리다고 하지 말고 실제 전처리 조건과 reachable 함수를 본다.
- 두 기본 effect에는 동일 entry를 전역 `VertexShader`/`PixelShader` 변수로 compile한 뒤 여러 pass가
  공유하는 경로가 이미 있다. pass 수를 모두 중복 compile 횟수로 세지 않는다. 남은 inline 식도
  entry/profile/인수/전처리 조건이 완전히 같을 때만 추가 공유 후보로 삼는다.
- 물리 파일을 나누기만 하고 모든 consumer가 다시 전부 include하면 영향 범위는 줄지 않는다.
  기존 source group/cohort·carrier 선택과 생성기·등록·배포 경계를 함께 유지하면서 필요한 함수와
  공통 선언을 좁힌다. 별도 두 번째 runtime이나 누락 material fallback을 만들지 않는다.
- 후보 비교는 같은 Release toolchain과 격리된 실험 출력으로 측정한다. production output을 덮거나
  `/Od`·최적화 비활성화·FXC skip으로 시간을 줄여 완료하지 않는다. 공개 pass 이름/순서/상태,
  base/group admission 정책, reflection 변수·타입·배열 범위, input signature, texture/sampler와
  material ID·상수 ABI를 보존한다. 컴파일 후 실제 FX 생성·consumer 준비와 사용자 화면 판정을 구분한다.
  compiler 시간 감소와 실행 중 GPU frame-time 개선은 서로 다른 검증이다.

### publish 범위·runtime 내용 검증·완료 시간은 별도로 보고한다

Product compile/link PASS는 모든 runtime domain의 최신 generation 내용, 설치 파일 또는 패키지
전달 완료를 의미하지 않는다. runner가 기록한 파일 존재·일부 catalog `CheckPublished`·Navigation
검사 범위를 그대로 보고한다. 참조된 generation/manifest·schema·stable ID·실제 소비 경로는 해당
변경의 publisher/reader 검증과 전달 목록에서 확인한다. JSON parse 성공이나 파일명 존재만으로
내용 정합성을 대신하지 않는다. Core/FullDiagnostic, 별도 harness와 실제 Server/Client 동작도
각각 실행한 범위만 기록한다. `-SkipBuild`를 현재 source의 컴파일 증거로 쓰지 않는다.

먼저 변경을 C++/HLSL/데이터 domain으로 분리하고, 저작 source·publisher·소비 schema가 바뀐 domain만
명시적으로 생성·검증한다. owner 전체 publish는 여러 domain 최초 준비 또는 전체 배포 준비처럼
실제로 필요한 경우에 선택한다. UI/C++ 수정이나 compile 경고 해소를 위해 대형 map/projector/
navigation·gameplay publisher를 매번 모두 실행하지 않는다. 반대로 실제 바뀐 gameplay·presentation
출력을 빠뜨리고 EXE만 전달하지 않는다. publisher의 START/PASS/REUSED/lock-wait와 domain별 시간을
읽고, 독립 병렬 구간을 중복 합산하지 않는다. 실제 코드/내용 결함 때문에 필요한 게시 비용과 선택
범위가 넓어서 발생한 비용을 구분한다.

완료 예상은 “항상 몇 분”으로 고정하지 않는다. 기존 cache 재사용 여부, 새 FXC 실행 여부,
C++ 대상과 public header 파급, 게시 domain, ZIP I/O와 검증 범위를 확인한 뒤 측정 근거와 가정을
같이 알린다. FXC가 시작되어 가정이 깨지면 직전 동일 계열의 실측으로 예상을 수정한다. 데이터 통합,
컴파일/링크, domain 게시, generation 전달, 패키지 구성, 사용자 실제 화면·다인 플레이 확인은
별도 완료 항목이다. 이번 사건의 시간표·로그·미측정 경계는 위 Release 통합 RESULT에 둔다.

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

- LOD 생성 최소 삼각형 수를 낮춰도 재질·UV·경계 오차 조건에서 실제 축약되지 않을 수 있다. 실제 설치 geometry의 생성 성공률과 cold/warm 준비 비용을 함께 측정한다. 파생 index cache는 변환된 전 채널 정점·index·알고리즘 revision으로 검증하고, 실패 결과도 재사용하되 손상·쓰기 실패는 원본 생성으로 복귀한다. 생성 가능한 LOD 수와 화면에서 선택한 LOD draw 수는 다르다.
- Static mesh LOD는 원본 geometry와 현재 draw 재질을 함께 검사한다. CModel material variant는 CMesh를 공유하므로 load-time opaque admission만으로 masked/교체 재질까지 LOD를 적용하지 않는다. CPU가 이미 가진 scalar 화면 오차는 CPU에서 선택해 direct instanced draw로 제출하며 기존 0.25px 품질 경계를 유지한다. 선택된 direct index 수는 실제값을 기록하고 다른 indirect draw의 상한과 섞지 않는다. 넓게 분포한 instance를 하나의 world sphere로 감싸면 횡방향 폭이 가까운 깊이로 오인되어 LOD0에 묶일 수 있다. 각 visible instance의 보수적 view depth/XY envelope를 사용하고 finite·near·재질 실패는 원본으로 되돌린다.
- light quad의 clip distance는 같은 VS 위치/UV를 유지해도 clipping 이후 interpolation 정밀도 차이를 만들 수 있다. coverage 누락과 FP16 출력 차이, clip-disabled control 및 사용자 visual 판정을 구분한다. 상세 근거는09-12 맵·캐릭터 성능 RESULT의G12–G14를 따른다.

- Composition Patterns의 편집 대상과 Resources의 추가할 Pattern은 별도 session 선택이다. 공용 Pattern Tree를 재사용해도 Resources의 Gate/Parent/Bundle/leaf 탐색은 대상·커서·preview를 바꾸면 안 된다. 실제 Append에서 현재 target/source를 다시 검사하고 잘리는 source 수명을 표시한다. [Parent 타임라인 결과 G07](09-12/2026-09-12_KOUKU_PARENT_PATTERN_TIMELINE_IMPLEMENTATION_RESULT.md).

## C++와 셰이더 빌드 입력 경계

- PCH에는 게임·저작·재질 표를 넣지 않는다. 기본 PCH와 charset/최적화 옵션이 다른 CPP는 PCH와 forced include를 함께 제외하고, 분리 CPP에는 원래 파일 옵션을 보존한다.
- Engine_Defines는 Assimp/DirectXTK/FX11/DirectInput/Engine_Struct의 우회 include가 아니다. 실제 완전 타입을 쓰는 CPP에 해당 헤더를 연결한다. Client에서 WinSock2는 Windows/D3D/DirectXTK보다 먼저 읽고, lean Windows 입력에서 RPC 헤더는 전역 using namespace std보다 먼저 읽는다.
- 생성 native material의 큰 표는 Private owner에서 한 번 컴파일한다. public inline 함수가 사용하는 작은 상수까지 Private로 이동하지 않는다. generator는 native_material_tables.py를 통해 읽기·저장을 하고 같은 bytes는 다시 쓰지 않는다.
- CPP를 분리하면 기존 source 검사도 등록된 same-owner CPP와 Private _Internal.h를 읽어야 한다. 다음 함수의 물리 순서를 기준으로 현재 함수 범위를 추정하지 않는다. cpp_source_domains.py와 Tools/Build/README.md의 소비 경계를 사용한다.
- 무변경 빌드의 OBJ/PCH/CSO 쓰기 0은 증분 처리 확인이다. 공통 셰이더의 큰 최적화 작업이나 cache 없는 빌드까지 해결한 증거로 쓰지 않는다. 세부 구조와 측정은 09-12/2026-09-12_PROJECT_BUILD_ISOLATION_IMPLEMENTATION_RESULT.md에 있다.
- native leaf를 나눠도 모든 wrapper가 가변 설치 목록을 include하거나 공용 carrier의 최대 ID를 계속 수정하면 무관한 FX가 다시 컴파일된다. Artist Mesh/Particle은 자기 SelectedGroup과 실제 guarded dispatch ID를 사용하고 installer는 facade·공통 helper를 그대로 유지한다. 새 program의 body/case, 선택 include, runtime 등록을 함께 검사한다.
- SourceCharacter selector는 이전 draw의 값이 남을 수 있다. native ModelCue의 그룹 컴파일을 빼려면 pass annotation과 CShader의 기본 FX 선택을 함께 연결해야 한다. 입력 변수·기본값·pass/input-layout ABI는 유지하고, 해당 native pass를 SourceGroup에서 직접 호출하면 실패시킨다. 조건 없이 native PS만 제거하면 기존 cue가 보이지 않을 수 있다.
- 같은 profile·entry·argument의 compile 식만 전역 shader 객체로 공유한다. V2 macro의 VS 선언은 실제 VS 함수 뒤에 두고, 같은 common include를 읽는 Decal까지 사용하지 않는 프로그램을 컴파일하지 않는다. pass 상태·전처리 조건 보존과 실제 CSO 검증은 별도로 확인한다.

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
  생성 재질 헤더의 parameter/family key도 동일하다. 생성기가 UTF-8 bytes를 고정3자리 octal
  escape로 출력하게 하여 뒤따르는 숫자와 escape가 합쳐지지 않도록 한다. 현재 헤더만 고치지
  않고 일반 parameter·시간 parameter·family 생성 경로와 실제 런타임 lookup bytes도 검증한다.
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
- SourceCharacter program은 번호 구간별 CSO 변형(`*_SourceGroupNNN.hlsl`)으로 컴파일된다. `Engine/Public/SourceCharacterProgramRegistry.h`의 stable program/range와 `source_character_registration.py`의 분할 정책, Base/Light leaf, wrapper producer·배포를 함께 갱신한다. CModel/CShader는 이 등록을 소비하므로 도구만 고쳐 ID를 받아들이면 실제 변형이 빠진다. 모든 추가 범위를 무조건64-ID로 계산하지 않으며 큰84/1088 구간의 세분화 정책도 재등록 때 보존한다.

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
- Deploy 모델 파일과 texture가 모두 있어도 MapTool이 raw path overload로 생성하면
  ActorCatalog의 native 재질 override가 빠진다. intact/fractured 양쪽을 제품과 같은
  `Build_ModelLoadDescription`으로 생성한다. 이 차이를 제품 화면의 원인으로 단정하지 않는다.
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

### 추적 카드의 수명·문양·접촉 폭발

지속 객체의 Server 수명과 Effect 표시 반복 주기를 분리한다. 무한 추적 카드를 긴 유한 lifetime으로 흉내 내거나 native emitter의 emission 창과 particle tail 합계를 표시 반복 길이로 쓰지 않는다. source SubUV random은 같은 seed의 단일 입자에서 같은 문양을 반복할 수 있으므로 네 문양을 독립 Effect로 저작할 때는 원본 atlas 칸을 명시하고 실제 CPU particle의 subimage 값을 확인한다. 카드 삭제와 접촉 event가 같은 batch에 도착해도 폭발은 고정된 event 위치·정의로 독립 재생해야 한다.

### 몸체 잔상과 Trail, 진단 실행 파일의 ABI

몸체 윤곽은 source TrailGhost notify와 실제 골격 palette를 먼저 조사한다. centerline Trail의 폭이나 수명만 늘려 골격 잔상을 대신하지 않는다. 과거 pose는 불변 복사하며 live palette를 복원하고 숨김·모델 교체·순간이동에서 이력을 해제한다. 원본 notify 확인과 원작 shader/fade 복원은 별도다.

Effect 구조체가 바뀐 뒤 서로 다른 시점의 codec·DetailIo·Playback OBJ를 섞은 probe는 잘못된 필드값을 읽을 수 있다. 실제 JSON에 없는 Two Sided 등의 오류가 나오면 데이터 수정보다 현재 헤더로 종속 TU를 다시 컴파일해 재현한다. 개별 원인·검증은 [세이튼 카드·트럼펫 결과](09-17/2026-09-17_SAYDON_CARD_TRUMPET_CHARGE_IMPLEMENTATION_RESULT.md)에 남긴다.

### 원작 particle 수명과 패턴 유지 구간, FXC include

Effect timing의 표시 duration만 늘려도 native source particle 수명은 늘지 않는다. sourceRecipe의 원본 lifetime과 `detail.particle.sourceScale.lifeTime`, 실제 CPU 생존 입자와 절대 alpha cutoff를 함께 확인한다. 원작 값은 보존하고 사용자 유지 구간 override만 별도 기록한다. Effect 자체 tail과 Pattern owner의 종료도 다르므로 standalone 성공으로 제품 tail 전체가 재생된다고 판단하지 않는다.

FXC가 `#include`의 문자열 macro를 확장하지 못해 X1500을 내면 이후 profile 함수 미정의는 연쇄 오류일 수 있다. native 프로그램 내용을 바꾸기 전에 wrapper의 literal selected include와 common 입력 순서를 확인한다. 새 cohort 추가 시 기존 facade/cohort가 변하지 않는 분리 계약은 테스트를 약화해서 우회하지 않는다.

### 공용 모델 shader에 pass를 추가하면 파생 FX admission도 확인한다

`ProgramVariantPass`가 기본 FX에서 BASE(1)이면 모든 source group의 같은 pass는 UNAVAILABLE(2)여야 한다. 기본·파생 양쪽에 BASE를 쓰면 FXC와 Product Build가 성공해도 `CShader::Stage_ProgramVariants`가 실제 생성에서 거부하며 Level 입장의 character rendering 단계가 실패한다. 공용 pass 추가 시 기존 정책 macro를 사용하고 파생의 사용 불가 PS는 NULL로 유지한다. pass 수·이름·입력 signature·변수 ABI 검증을 약화하지 않는다.

컴파일 성공과 실제 `CShader::Create` 성공은 별개다. 설치 base와 현재 CShader 범위에 등록된 모든 group의 동일 빌드 CSO로 기존 WARP probe를 실행하여 admission, base-owned pass 선택, stale source selector 아래의 상수·bone 보존과 파생 직접 호출 거절을 확인한다. headless 검사는 실제 Client 입장·GPU 화면 판정과 구분한다. 원인과 증거는 [세이튼 입장 실패 수정 결과](09-17/2026-09-17_SAYDON_CARD_TRUMPET_CHARGE_IMPLEMENTATION_RESULT.md)의 G06에 둔다. 카드미로 picking pass25~27도 같은 계약을 사용하며, 현재 static mesh family는 base와 14개 group이다. 기본 FX만 검사하면 이 결함을 놓친다. 맵 shader 생성이 실패해 Loader가 Effect 작업을 취소한 경우 `already cancelled`는 후속 오류이므로 cancellation을 초기화하여 우회하지 않는다. 후속 수정과 실제 factory 검증은 [카드미로 결과 G04](09-24/2026-09-24_CARD_MAZE_FLOOR_NAVIGATION_RESULT.md)에 기록한다.


### 원본 Effect Tree만 등록하고 Catalog metadata를 빠뜨리지 않기

Effect Tool이 직접 읽는 Authored 파일과 Pattern이 CEffectCatalog로 읽는 경로는 등록 계약이 다르다. Tree에 V1을 공개할 때 exact DIRECT_AUTHORED_DOCUMENT metadata도 함께 stage하고 lazy payload validation은 유지한다. metadata 등록 성공과 payload/화면 성공을 구분한다. sync_kouku_effect_tree.py는 두 catalog의 기준 bytes와 Authored SHA를 확인하며 편집 중 등록을 강행하지 않는다.

EffectCatalog의 긴 JSON 줄에 이름이 남아 있는 것과 Pattern에서 재생하는 occurrence가 남아
있는 것은 다르다. 충돌 검토는 양쪽 parent/base의 stable asset ID와 기존 필드를 구조적으로
비교하고, resource 정의부터 현재 occurrence 참조까지 별도로 확인한다. 삭제된 재생 연결을
과거 RESULT의 설치 기록으로 복원하지 않으며, 재사용 가능한 library 정의를 참조 없이
남겼다는 이유만으로 삭제 실패나 병합 회귀로 판정하지 않는다.

### 선택한 플레이어 위치에 고정한 Effect 그룹과 airborne의 원점

`selectionGroupId` 자체는 재생 pivot이 아니다. Play 시점 고정 요청은 SELECT 시점 navigation ground XYZ를 저장하고 APPEAR와 fixed targeted visual이 같은 점을 소비해야 한다. MAP 멤버의 공통 원점은 XZ뿐 아니라 Y도 빼야 떠 있는 오프셋이 중복되지 않는다. 멤버의 원래 absolute 시작 시각은 유지하고 일반 Effect lane 중복 재생은 제외한다. 기존 Albion의 APPEAR 시점 추적은 optional SELECT 정책과 구분한다. transaction 실패는 이전 선택·좌표·시각 객체를 함께 보존한다.

### World 그룹 donor와 실제 표시 모델의 pivot을 구분한다

Object Collider의 원형 폭발은 기존 BOX를 표시만 둥글게 그리지 않고 optional shape=CYLINDER를 codec·편집·preview·Map publish·Server bake까지 연결한다. halfExtents의 X/Z는 같은 반경이며 비균등 scale은 큰 X/Z 축을 사용한다. 피해 시계와 이펙트 잔상 수명은 별개다. 짧은 폭발 창은 플레이어마다 한 번이고 독립 방출 창은 중첩되므로 화염의 repeatIntervalMs를 폭발에 복사하지 않는다. 최대 HP 비율 피해도 방어력 무시와 피해 감소 버프·보호막·무적 소비를 구분한다. 원작 공식 근거가 없는 조정값은 PROJECT_TUNED로 기록한다.

Complete Play의 WORLD group 해석에서 `colliderTracks`가 있다는 이유만으로 모션을 거절하지 않는다. 해당 트랙은 publisher가 Server pattern geometry로 bake하고 Client는 본 유효성 검사·Debug 표시만 소비한다. 실제 모델에 연결된 WORLD/STOP·LOOP 모션의 Collider metadata는 허용하되 walkableSurface·combatBody·중첩 group·잘못된 binding은 계속 거절한다. 준비 검사뿐 아니라 실제 Prepare/Play/Span/Pivot의 공통 해석을 함께 확인한다. 분열 공 재현과 검증은 [09-25 구현 결과 G09](09-25/2026-09-25_KOUKU_AUTHORING_REFINEMENT_IMPLEMENTATION_RESULT.md#g09-complete-play-분열-공-world-준비-거절-수정)에 기록한다.

model-less group과 내부 CModel donor에 같은 표시명을 붙이면 default g0를 Append해 전체 그룹처럼 오인할 수 있다. Objects 목록은 고유 owning group을 선택하고 개별 motion 편집과 구분한다. WorldSequence의 group ID를 instance ID로 보내야 기존 여섯 motion 확장이 실행된다. native FX mesh와 재사용 World 모델은 extent가 같아도 pivot은 다를 수 있으므로 실제 정점 중심을 각각 적용한다. 공에 붙는 상단광은 위치의 이동 소유자를 하나로 유지한다. LocationEmitterDirect가 최종 위치를 덮는 입자는 velocity가 PSA_Velocity의 방향 입력일 수 있으므로 일괄 비활성화하지 않는다. fitEffectToDuration의 기준에 emitter tail이 포함돼 live particle이 일찍 끝나는지도 확인한다.

### Trail 반복 무늬·빈 구간과 원본 방출 영역

Trail이 끊기거나 같은 무늬가 크게 반복되면 tick 증가 전에 원본 VS의 UV 전체 성분과
PS의 소비, TypeData TilingDistance의 cm→m 전달을 확인한다. baked history를 사용해
sourceRecipe가 꺼져 있어도 retained TypeData는 근거다. 누락된 저작 필드만 복구하며 명시한
0을 원본 값으로 덮어쓰지 않는다. native 수식 parity와 실제 vertex 축·화면 판정을 구분한다.

SpawnPerUnit은 개수만 세고 같은 tick 끝점에 모두 배치하면 중복점과 긴 공백이 생긴다.
world-space Ribbon은 실제 이동 구간의 거리 교차점에 생성하고 emitter loop의 나머지 거리를
보존한다. Reset은 이력을 비운다. 입자 수명·폭·난수·다른 family를 보존하며 이 결함을
100배 update로 숨기지 않는다.

primitive Cylinder의 positive/negative XYZ는 source height 축 배치 뒤, cm·owner 변환 전에
소비한다. 텍스처 이름만으로 둥근 검정 영역을 판단하지 않고 StartSize·velocity-facing·pivot·
alpha 경계 수식과 방출 분포를 함께 읽는다. 구현·검증은 [Trail 결과](09-17/2026-09-17_TRAIL_RIBBON_NATIVE_RESTORATION_IMPLEMENTATION_RESULT.md)와 세이튼 결과 G20에 둔다.

### 공급원 의존 Solo와 상단 sprite의 실제 표시 기준

Solo/Family/Group은 선택 요소의 LocationEmitter 및 LocationEmitterDirect(각 EF alias 포함)
활성 참조와 transform master를 재귀 보존한다. 공급원 bVisible을 끄면 simulation도 멈추므로
기존 submission element set으로 선택한 요소만 그린다. 전체/선택의 원본 시각·순서를 유지하며
실제 missing provider 검증을 제거하지 않는다. stale ID는 전체 Effect 재생으로 확대하지 않는다.

위치 공급원을 따라가는 입자의 World 중심 일치만으로 상단 부착을 완료 처리하지 않는다.
PSA_Velocity 방향, signed StartSize, image flip, noncentral pivot, lifetime size와 최종 quad
하단·상단을 함께 검사한다. 원본 CPU packing이 미확정일 때 파생 Effect의 pivot 조정은
사용자 배치 override로 기록하며 공용 shader의 원본 복원이라고 부르지 않는다.

### Albion preview의 대상 선택 조건은 Server와 일치시킨다

V1 FX 추가와 pattern Logic 상속을 혼동하지 않는다. APPEAR_PLAYER는 Server가 기존 선택이
없으면 등장 시 살아 있는 대상을 선택하므로 preview도 같은 fallback을 허용한다. 실제
SELECT_PLAYER의 SELECT 정책만 선택 지면을 고정한다. JUMP 선행과 native 하강 검증은
그대로 유지하며 오류를 숨기려고 사용자 삭제 Logic을 재삽입하지 않는다. 세이튼 결과 G25 참조.


### 카드 mesh와 고정축 sprite는 중심·owner 회전·반복 수명을 따로 확인한다

mesh/sprite 불일치는 detail 위치만 맞춰 끝내지 않는다. 실제 WModel의 평면·장축,
TypeData pre-rotation, 원본 StartLocation, sprite pivot와 camera offset을 분리해서
실측한다. sourceTransformTrack이 없는 local-space 고정축 sprite는 기존
followEmitterAxisRotation 소비 여부를 확인하고 필요한 occurrence에만 연결한다.
정상 world-space smoke나 같은 문서의 별도 폭발 레이어로 보정을 확장하지 않는다.

Required의 미직렬화 emitterloops는 상속/CDO와 native 기본값을 확인한다. 임시 loopCount=1로
고정하면 원본의 1초/2초 입자가 tracking duration 중 소멸하고 bounded-loop admission도
실패할 수 있다. 원본 입자 수명과 명시적인 유한 loop를 보존하며 반복 경계의 누락과
구입자·새 입자 중첩을 구분해 검증한다. shader 식이 원본과 같을 때 요청한 RGB 밝기 보정은
project-authored로 기록한다. 원본 world-space 잔상 emitter를 사용자 요청으로 숨겼다면
원본에 잔상이 없었다고 설명하지 않는다. 실제 수치·적용 상태는
[세이튼 카드 결과 G26](09-17/2026-09-17_SAYDON_CARD_TRUMPET_CHARGE_IMPLEMENTATION_RESULT.md)에 둔다.

공 낙하를 상승으로 변형할 때는 LocationDirect 위치 곡선과 velocity-facing 입력을 구분한다.
공 위치의 Direct provider를 유지한 채 꼬리 sprite의 pivot과 실제 공 하단 offset을 측정한다.
world-space 별은 매 tick 따라가는 direct follower가 아니라 spawn 시점 provider 위치를
받아야 기존 궤적이 남는다. 효과 복제 시 provider stable ID도 함께 remap하며 삭제된 요소를
source 전체 재생성으로 되살리지 않는다. 상승공 적용 수치는 같은 RESULT G27을 따른다.

### 실행 중 도구의 데이터 반영과 EXE 점유를 구분한다

Client 프로세스가 있다는 이유만으로 준비된 데이터 설치를 막지 않는다. 검증한 후보를
최종 반영할 때 한 번 받은 저장본 기준 승인을 사용하고, 최신 파일에 필요한 필드만 병합한다.
오래된 snapshot 전체 교체와 미저장 draft의 자동 reload는 다른 사용자의 편집을 잃게 할 수
있으므로 hash/revision·stable ID·freshness 검사는 유지한다. 이미 받은 승인을 종료 확인
질문으로 반복하지 않는다. 정본 절차는 [AGENTS의 편집 중 데이터 반영](../../AGENTS.md#편집-중-데이터-반영)이다.
파일 설치·publish 성공을 실행 중 메모리 갱신으로 보고하지 않으며 실제 EXE/DLL 링크 점유만
종료가 필요한 별도 사유다.

### 발탄 목록 읽기와 publisher 경합

`canonical Product read admission failed`는 상세 failure kind를 먼저 확인한다. WRITER_BUSY는
Product 손상 판정이 아니며 기존 snapshot을 보존하고 자동 재시도한다. 그동안 catalog·원본
animation index를 반복 parse하지 않는다. 독립 authored Effect 목록은 pattern Play admission과
분리한다. publisher의 독립 다른 boss 선검증은 발탄 writer 획득 전에 수행하고, 발탄 source
snapshot부터 출력 교체까지의 잠금은 유지한다. exact-save revision 재시도를 최신 세대로
바꾸거나 unpinned Product 읽기로 우회하지 않는다. 적용·검증 범위는
[발탄 트리 경합 결과](09-17/2026-09-17_VALTAN_EFFECT_TREE_PUBLISH_CONTENTION_RESULT.md)를 따른다.


### 전투 기본 spawn과 연출의 절대 이동 시작점을 분리한다

연출을 위해 boss placement를 옮겼다면 이후 BossMotion 연결 시 일반 관문 생성도 그
placement를 소비하는지 다시 확인한다. 기본 spawn은 해당 관문 중앙을 소유하고, 연출의
시작·도착·시각은 기존 BossMotion이 소유하도록 분리한다. Parent가 child 이동을 확장할 때
시각만 이동하고 절대 좌표를 유지하는지 검사하며 이미 정상인 연출 좌표를 spawn과 함께
덮어쓰지 않는다. World publisher의 디스크 출력과 실행 중 Server가 로드한 bootstrap은
별도 상태다. 적용·검증은 [Level Navigation·세이튼 spawn 결과 G02](09-17/2026-09-17_LEVEL_NAVIGATION_DEBUG_SAYDON_SPAWN_RESULT.md)에 둔다.

### 게시 네비게이션의 막힘과 바닥 미검출을 구분한다

게시 blocked 값만으로 NO_SURFACE라고 판정하지 않는다. source/paint의 descriptor와
전체 셀의 walkable·실제 저장 높이가 일치할 때만 원본 원인을 표시한다. authoring의
주변 셀 기반 표시 높이는 실제 baked 높이가 아니다. Auto 표시에서 detail과 겹치는 base는
겹친 부분만 빼고, 수동 Base 검사에서는 전체 base를 보존한다. 표시 예산으로 생략된 셀도
미베이크 구멍으로 설명하지 않는다. 중앙 좌표 몇 개의 성공으로 가장자리 bake를 정상
판정하지 않으며, 최상단 교차 방식은 넓은 Y 범위의 상부 기하를 포착할 수 있다.
표시 데이터·실행 중 Server 상태·사용자 화면 확인을 구분한다. 구현과 조사 범위는
[Level Navigation 결과 G01](09-17/2026-09-17_LEVEL_NAVIGATION_DEBUG_SAYDON_SPAWN_RESULT.md)에 둔다.

### 연결된 Trail 정점과 재질 coverage를 같은 성공으로 세지 않는다

삼각형 띠가 연결되어도 폭 마스크의 V에 길이 좌표를 주면 알파가 진행 방향을 잘라낸다.
원본 DDS의 실제 채널·축·wrap/clamp와 원본 PS의 소비 성분을 함께 확인한다. float2 하나를
복사하거나 `.yx`로 뒤집어서 원본 float4 UV 계약을 채웠다고 판단하지 않는다. 전체 길이의
정규화 좌표와 거리 반복 좌표는 서로 다른 입력이며, 유한 0~1 taper에 무제한 거리값을
넣으면 띠 중간에서 알파가 다시 0이 될 수 있다.

동일한 임의 입력으로 번역 PS와 원본 DXBC가 일치한 검사는 연산의 일치를 증명한다.
그 입력을 실제 carrier가 올바르게 만들었다는 증거는 아니다. 원본 CPU vertex packing을
회수하지 못했다면 VS passthrough, 재질 소비 범위와 추론을 구분한다. 실제 본 궤적 또는
baked 양쪽 edge를 업로드하고 길이별 alpha와 내부 공백을 검사한다. 자연스러운 끝 fade를
중간 단절로 세지 않으며 평균 밝기·정점 개수·finite 값만으로 완료 처리하지 않는다.

후처리 없는 raw material 출력에서 결함을 재현한 뒤 MRT blend, scene depth, distortion,
bloom/tonemap을 따로 조사한다. 한 단계의 반증으로 전체 장면의 가림까지 배제하지 않는다.
shader family 등록뿐 아니라 마지막 generated material/distortion 소비자에서 UV 성분이
버려지는지도 확인한다. 구체적인 비평·수정·검증 범위는
[Trail 결과 G05](09-17/2026-09-17_TRAIL_RIBBON_NATIVE_RESTORATION_IMPLEMENTATION_RESULT.md)를 따른다.

### 복제 Sprite의 외곽과 모자 부착의 기준 좌표

개별 검정 Sprite의 alpha가 원형이어도 복제·비등방 확대된 여러 Sprite의 합성 외곽은
공통 원이 아니다. 원본 요소 복구와 사용자가 요청한 effect-origin 원형 coverage를 구분하고,
마스크를 입자 중심이나 StartSize 기준으로 적용하지 않는다. 선택한 carrier만 opt-in하고
RT0·왜곡·Bloom의 coverage 순서를 함께 확인한다. 새 optional struct가 포함된 probe는
해당 header를 소비하는 객체를 모두 같은 ABI로 다시 컴파일한다.

장착물은 bone 이름만 맞추지 말고 실제 모델의 material identity와 bone basis를 확인한다.
WORLD anchor가 basis를 정규화하면 body preScale을 중복 적용하거나 빠뜨리지 않는다.
기본 머리 모자의 숨김 상태를 전역 bool로 공유하지 않고 실제 owner와 살아 있는 손 모자의
lease에 연결한다. 저장된 DURATION은 timing 존재와 PRODUCT 의미의 유효성을 별도로
검사한다. 이번 모자 유지 구간은 기존 ATTACHMENT_HOLD를 사용한다.

### World Object의 정적 모델은 follow 본이 없어도 pivot으로 부착한다 (2026-09-17)

- `공_튀기기`처럼 정적 mesh Object에 보스용 V1 문서(`runtimeBoneName: b_root`, follow)를 붙이면
  `World Object V1 source bone is unavailable`로 거절됐다. owner 경로는 없는 본을 건너뛰지만
  transform-history 재생은 follow slot이 반드시 `SourceAnchorWorlds`에 있어야 하므로 slot을 비우면 안 된다.
  `Sample_ObjectEffectAttachments`가 없는 본을 identity bone으로 채워 object pivot에 붙이고 preflight는
  debug note만 남긴다. 명시 `effect.bone`과 collider `attachmentBone`은 여전히 엄격하다.
- V1 effect track의 `positionOffset`은 Object 저작 scale(1.5)이 곱해진 기저에서 적용된다. V2 GROUP의
  metre 오프셋을 같은 월드 위치로 옮기려면 scale로 나눈다.

### native 재질 table shape와 sprite admission을 먼저 확인한다 (2026-09-17)

- `fx_k_pa_turbpa_06_tr`(native 3008)는 ribbon 요소로 먼저 복원돼 table shape가 `ribbon`이다.
  같은 MIC를 sprite 요소에 옮기면 `Native Artist requires its recovered material variant…`로 거절된다.
  원본이 sprite emitter여도 프로그램 table row의 shape가 다르면 admission되지 않으므로, 대체 재질을
  쓰거나 sprite table row를 새로 설치해야 한다. 별 선 smoke_tail은 strike의 `fx_m_pa_smoke_01_8_tr`(2992)로 대체했다.
- 후보 문서를 쓸 때 `Path.write_text`는 Windows에서 CRLF를 넣는다. Authored 문서는 LF이므로 bytes로 쓴다.
  CRLF가 섞이면 diff가 파일 전체가 되고 `git diff --check`가 통과해도 병합 충돌을 만든다.

### 격리 codec/playback probe는 resource root와 헤더 ABI를 맞춰야 한다 (2026-09-17)

- `codec_probe.exe`류는 `LOSTARK_RESOURCE_ROOT`가 없으면 `Is_SafeResourceAssetId`가 DDS 종류를 못 읽어
  `Effect source Material texture is invalid`로 실패한다. `Client/Bin/Debug`를 PATH에 넣고 resource root를
  지정한 뒤 실행한다.
- `Effect_AuthoringDocument.h`가 바뀐 뒤 옛 OBJ와 새 헤더로 링크한 probe는 `xmemory(983) null pointer` assert로
  CRT 대화상자에 멈춘다. 헤더 변경 시각 이후에 컴파일한 closure(`out/PizzaMaskRestoration20260917/abi`)와만 링크하고,
  probe에는 `SetErrorMode`·`_set_abort_behavior`·`_CrtSetReportHook`을 넣어 대화상자를 막는다.
- Python `validate_effect_sources.py`의 v15 baked history 규칙(`playbackClampSeconds < 마지막 표본 시각`)은
  발탄 420609 stage008/009의 HEAD 문서도 거절하고 `blade-dance.circle.impact`는 carrier가 없어 저장소 전체
  검증이 먼저 멈춘다. 변경 문서만 같은 module 함수로 검사하고 기존 실패는 RESULT에 구분해 적는다.

### Workbench가 저장 중인 Composition은 외부 publish가 CAS로 계속 실패한다 (2026-09-17)

- `Invoke-BuildDomainOwner -Owner KoukuSaydon`은 validation 전후 입력 hash를 비교해
  `Publication input changed during validation`으로 중단한다. 사용자가 Action Workbench에서 몇 분 간격으로
  Save하는 동안(rev 1330→1335) 세 번 모두 실패했다. 외부 세션은 재시도를 반복하지 말고, 저장이 끝난 뒤
  한 번 실행하거나 사용자가 Workbench의 `Publish All Patterns`로 게시하게 안내한다. 파일 자체의 필드 편집
  (바이트 보존 splice + revision +1)은 다음 Save에 그대로 유지됐다.

### 배치 제거는 행·baked lighting·참조 문서를 함께 지운다 (2026-09-17)

- 캐릭터 선택 스폰의 별 문양은 09-15에 추가한 editor 배치 `editor:LV_LOBBY_CLASSSELECT_SL00:1`이었다.
  `.mapplacements` 행과 header count, `.mapmaterials.json placementLighting`의 같은 sourcePlacementId,
  `CharacterSelectFloorSwap.json hiddenSourcePlacementIds`를 같이 지워야 `Publish-MapAuthoring -Scope Area`의
  dangling lighting 검사와 Debug Floor Swap 로드가 통과한다. `visible=0`로 숨기지 않는다.
- Area publish는 대기 중이던 다른 editor 행(editor:2 부조)도 함께 내보낸다. PR에 그 사실을 적는다.

### 사용자 저장 Effect 문서의 부분 필드 복구는 바이트 splice로 한다 (2026-09-17)

- `effect.kouku.common.spinning.card.throw`는 Tool이 CRLF로 저장한 11MB 문서다. 24개 `visible` 값만
  raw_decode span 안에서 교체하면 -24 bytes의 최소 diff가 되고 builder의 `write()` 재직렬화(LF)는 쓰지 않는다.

### JUMP(0 ms)만으로는 보스가 내려오지 않는다: clip 하강 착지는 SLAM이 소비한다 (2026-09-18)

- `ALBION_AIRBORNE` JUMP는 `airborneDurationMs 0`이어도 높이를 고정할 뿐이며 Stage native root motion의
  하강을 무시한다. 쿠크 훌라후프 P84에서 사용자가 재저작 중 SLAM box(logic.2)를 지우자 보스가 11 m에 머물렀다.
- 시작 높이에서 clip 하강으로 착지하려면 같은 clock에 SLAM box를 두고, `_start` Stage가 반복되면 각 Stage
  시작에 SLAM을 하나씩 둔다. 두 번째 SLAM은 직전 `12_end` 상승 높이(약 15.8 m)에서 정규화 하강한다.
  JUMP 높이는 실제 설치 clip의 하강량에 맞춘다(`12_start` 10.890836 m, `13_start` 11.191078 m).
- 저장본 전체 `validate_document`는 다른 미완성 draft(P32 세이튼_쇼타임의 stage 없는 presentation)에서 먼저
  실패한다. 후보 검증은 publisher처럼 `_publication_candidate(closure)` 단위로 한다. 적용 스크립트·Server 높이
  시뮬레이션·receipt는 `out/KoukuHoopDescent20260918/`에 있다.

### 발탄 제품 clip과 Full Restore의 Sprite 누락·본 배율을 함께 확인한다 (2026-09-18)

- 같은 action의 Full Restore에 원본 emitter가 있어도 실제 제품 cue가 carrier-v1 clip01/02를
  참조하면 제품 복원이 아니다. cue→asset ID→sourceNode→native material까지 대조한다.
- 긴 사전 생성 Sprite와 짧은 스윙 Trails를 시간·source emitter 기준으로 분리한다. 리본
  UV/색 수정만으로 제품에서 빠진 SpriteParticle이 생기지 않는다.
- source particle은 이미 m 단위다. Full Restore에서 본 부착 요소를 제품 ID로 옮길 때
  기존 source-bone scale-normalization helper의 적용 범위를 확인한다. 정상 발탄0.01과
  유령 발탄1 basis를 혼동해 전체 입자 크기100배를 저작하지 않는다. 기존 cue worldScale과
  원본 notify scale은 별도다. 두 제품 클립의 StartControl에만 기존 보정을 연결했다.
- 적용 범위와 실제 수치·검증·제품 빌드 경계는
  [4연속 Sprite 결과](09-18/2026-09-18_VALTAN_FOUR_SLASH_SPRITE_RESTORE_RESULT.md)를 따른다.


### 캡처된 장판 그룹과 Effect 회전 pivot (2026-09-18)

- selectedEffectGroupId는 Preview 전용이 아니다. publisher가 여러 occurrence를 단일
  selectedEffectVisualId template으로 바꾸므로 Server에 그룹 ID 필드가 없다는 이유로
  미지원으로 판단하지 않는다. 같은 시각 SELECT 정책과 APPEAR는 캡처 지면을 공유한다.
- 고정 장판에 BOSS anchor를 쓰면 공중 Y와 행별 시작 포즈가 섞인다. 기존 fixed template과
  captured ground를 사용하고 preview/ordinary presentation 중복을 제거한 소비 경로를 확인한다.
- captured root의 Element Transform도 기존 Playback이 소비한다. source track/carrier/inheritance
  owner는 그대로 제한하고 rotation pivot을 중심/원점/custom으로 지정한다. Pivot UI는 세션 상태,
  저장 정본은 결과 Element TRS다. source 고정축 sprite는 필요한 요소에만 기존
  followEmitterAxisRotation을 켜고 최종 quad까지 회전되는지 확인한다.
- 비둘기 builder의 후보와 실제 live track은 달랐다. 현재 저장 거리부터 측정하고 직선 시간
  단축으로 같은 속도의 선회/귀환에 시간을 배분한다. 경로 수정에서 저장 밝기를 덮어쓰지 않는다.

### Fixed-axis 장판의 내부 이미지 회전과 quad 회전 불일치

- native 재질이 SourceEmitterWorld 역행렬을 사용해도 fixed-axis sprite quad가 emitter
  회전을 소비한다는 뜻은 아니다. 실제 quad와 shader 좌표계를 함께 확인한다.
- 원본 EPAL_Z를 유지한 채 배치 회전이 필요한 해당 요소에만 followEmitterAxisRotation을
  연결한다. source pivot과 snapshot basis를 보존하고 unrelated sprite에 전파하지 않는다.
- 외부 authored 수정 뒤 Effect Tool의 Load Saved와 Product 캐시 갱신을 구분한다.
  Restart Preview/Refresh Resources만으로 새 파일을 읽었다고 판단하지 않는다.

### sourceTransformTrack의 빈 alphaScaleKeys는 기본 alpha1이 아니다

- alphaScaleKeys=[]은 Codec에서 값0인 optional 분포로 생성되어 Playback에서 기존
  입자 alpha를0으로 곱한다. 위치/회전만 저작하는 track은 alphaScaleKeys를 생략한다.
- 원본 opaque/fade 곡선을 유지하려고 빈 배열을 넣지 않는다. count/finite/quad 성공만으로
  표시를 판단하지 않고 실제 Color.w를 검사한다. Full Restore와 제품 carrier 경로를 구분한다.
- 420609 stage008/009 axe worms36요소의 수정·A/B·설치는09-18 Sprite 복원 RESULT G06에 기록했다.

## 발탄 Composition source clock과 저장 소비자 분리

- source sequence와 master Pattern은 preview owner가 다르다. source는 Pattern ID가 비어 있으므로
  공통 Play/Pause/Seek를 Pattern ID만으로 분기하지 않는다. CModel의 자유 재생과 authoring clock을
  동시에 켜지 않고 명시적 sample 한 경로만 pose를 쓴다.
- Source Save에서 Product 전체 완성을 요구하지 않는다. 반대로 Source Save를 분리하면서
  제품 V2/Sound reader를 authoring 파일에 남기면 미완성 draft가 재실행 때 활성화된다.
  게시된 snapshot과 명시적 local preview snapshot을 실제 소비자까지 구분한다.
- Stage를 줄일 때 기존 Sound/Effect뿐 아니라 Camera/SceneProfile/Light 끝과 마지막 Summon spawn도
  검사한다. Stage 간 drag는 source 삭제와 target 추가, dirty metadata를 같은 transaction으로 처리한다.
- 실행 가능한 범위와 결과는 [발탄 Composition 재개 결과](09-09/2026-09-09_VALTAN_COMPOSITION_AUTHORING_PARITY_RESULT.md)를 따른다.

- Windows PowerShell 5.1에서 Save job 결과를 `[IO.File]::Replace`로 교체할 때 null backup 인자는
  overload 변환으로 경로 오류를 만들 수 있다. 중간 canonicalCommitted receipt 뒤 마지막 receipt도 실제
  실행해 검사하고, job 소유의 명시적 sibling backup과 원자 교체를 사용한다.

## 쿠크 presentation 박스의 bone anchor는 기본이 위치 전용이다

- `Make_Pivot`이 `Resolve_TargetPivot`에 넘기던 `PIVOT_ROTATION::TARGET_YAW`는 본에서
  위치만 가져오고 회전 basis를 boss root로 덮는다. bone만 지정하면 이펙트가 그 본을 따라
  이동하되 함께 회전하지는 않는다. 회전까지 필요하면 occurrence의 `boneRotation`을 `BONE`으로
  둔다(EFFECT + BOSS anchor + 이름 있는 bone에서만 허용).
- anchor를 본으로 바꾸면 기존 `positionOffset`/`rotationDegrees`는 못 쓴다. 그 값은 boss root
  frame에서 잡은 것이라 본 frame에서 다시 잡아야 한다.
- world space emitter의 이미 방출된 입자는 소급 회전하지 않는다. 새로 나오는 입자만 따라 돈다.
- EFFECT row의 BODY bone 이름은 projector와 Product parser가 WModel 실재를 검사하지 않는다.
  오타는 publish를 통과하고 런타임에서 그 박스만 `Presentation bone/pivot is unavailable`로 격리된다.
- 세이튼 본체 본 이름은 `bip001-head`, `bip001-mouth`다(MN_RPCT_05 168본, MN_RPCT_06 84본).

### World Effect의 finite source는 bounded source loop0와 구분한다 (2026-09-18)

`Set_SourceLoopEndSeconds`는 source EmitterLoops=0 연장이며 모든 emitter가 finite이면 거절한다. World/Composition V1 Effect의 `loopEffectToDuration`은 finite source에 원본 prepared duration 단위 반복을 사용하고 follow provider에 반복 시작 나이를 더한다. 이를 빠뜨리면 Effect만 Object 시작 위치로 돌아간다. fit은 한 번 재생하는 source 시계를 느리게 만들므로 원래 속도 지속 재생 요구와 구분한다. WORLD 박스의 birth deadline 뒤 tail 허용과 부모 Pattern 종료는 별개다. Collider bake는 부모 종료를 명시적으로 받아 그 뒤 hit/track을 만들지 않아야 한다. source JSON을 loop0로 덮어쓰거나 prepared identity 검사를 완화하지 않는다. 근거는 09-18 KOUKU_WORLD_BLADE_REPAIR_RESULT에 기록한다.

### Composition 배우·방출 수명·게시 연결

- 도구의 Bone 목록은 해당 Composition Preview/Server CNpc의 typed model-target view에서 resolve한다. 별도 Animation Tool 전역 선택과 profile을 비교하는 것만으로는 실제 Preview 배우의 본을 찾을 수 없다.
- 모델의 정면은 Transform +Z라고 가정하지 않는다. 실제 설치 모델의 head/mouth basis와 clip을 측정하고, 목표 body yaw 보정과 그 몸의 전진 방향을 함께 고친다. 추적 수명을 늘리는 것이 회전 속도를 낮추지 않게 이동 추적과 시간 제한 회전의 계약을 구분한다.
- 외부 Effect/WORLD 박스의 길이만 늘려도 내부 emitter/template hidden key가 자동 연장되는 것은 아니다. source 방출·Object lifetime·occurrence cutoff를 각각 확인한다. finite Effect 반복은 source 원문을 바꾸지 않고 occurrence별 원래 재생속도와 follow 시계를 유지한다.
- 게시 성공 여부에 더해 요청한 patternId의 unavailableReason, 생성 트리거 및 Server bootstrap 행을 확인한다. 이름만 있는 DURATION/RESULT, 비어 있는 patternSpawns를 실행 가능한 기믹으로 설명하지 않는다.

### Sequencer의 긴 seek는 GPU·타임라인 UI보다 과거 root 재평가를 먼저 본다 (2026-09-18)

- Complete Play는 정상인데 특정 시점에서 정지해도 느리면 Effect.Service.Update/HistoryUpdate와 Animation.Channels.Sample을 함께 비교한다. exactRoot provider마다 0초부터 모든30Hz 회전 사건을 재실행하면, 긴 seek의60Hz Effect history 안에서 같은 과거를 중첩 재계산한다.
- Preview member별 yaw·animation별 yaw·follow offset checkpoint는 같은 시각의 Stage와 tracking 사건을 모두 처리한 뒤 저장한다. read-only 과거 sampling이 새로운 플레이어 관측을 만들거나 실제 actor pose를 변경해서는 안 된다. 미관측 미래 입력 실패를 유지한다.
- 위치/yaw 편집과 member 교체 때 checkpoint 및 최종 pose memo를 무효화한다. 최종 pose memo는16384개로 제한하고, 역방향은 가장 가까운 이전 checkpoint에서 재개한다. 정지 프레임의 동일 시점은 이미 계산한 pose를 쓴다.
- capture의 scope drop이 있으면 최초182초 프레임의 원인을 세부 수치로 꾸미지 않는다. 이번 사용자 capture에는 timeline ms가 없어44299/55637ms의 정확 대응은 사용자 관찰이다. 구현·수치 검증과 사용자 FPS 확인은 대응 KOUKU_PATTERN_RUNTIME_REPAIR_RESULT에서 구분한다.

- 단독 EffectAuthoringSequencer도 loopEffectToDuration의 finite/loop0 구분을 소비해야 한다. Composition만 고치면 공통 불25개 finite emitter가 단독 Preview에서 계속 거절된다. cycle을 되감을 때 provider의 owner/bone 시각에 cycle 시작을 다시 더한다.
- 컷신의 Animation lane이 비어 있어도 World sequence의 animationTracks와 설치 WModel clip을 먼저 확인한다. 이미 World 배우가 소유한 clip은 기존 WORLD 편집 경계로 투영하고 같은 보스 Animation을 중복 생성하지 않는다.
- WORLD Animation 표시 행을 시간 구간만으로 채우면 서로 다른 배우의 clip이 같은 행에 섞인다.
  World occurrence+slot별로 행을 고정하고 배우를 표시한다. 공백과 이웃은 같은 owner의 source
  구간·속도·끝 시각으로 판단하며 화면상의 빈칸만으로 validator를 풀거나 다른 clip을 밀지 않는다.
  편집 거절은 이웃 충돌과 Motion 범위 초과를 구분하고 사용자 dirty 문서와 고정 시계를 보존한다.
  같은 mesh를 쓰는 원본 actor도 별도 UObject·visibility·본 부착을 가질 수 있으므로 mesh 이름만으로 중복이라 판정해 삭제·병합하지 않는다.

### Show Navigation 오버레이는 main viewport background list에 명시적으로 그린다 (2026-09-18)

- `ViewportsEnable` 아래 인자 없는 `ImGui::GetBackgroundDrawList()`는 `CurrentWindow->Viewport`를 쓴다. 모든 tool 창 End 뒤의 current window는 암시적 `Debug##Default` 창이고, `imgui.ini`가 그 창을 자기 viewport(`0x16723995` = CRC32C `ImHashStr("Debug##Default")`)에 고정하면 platform window가 없어 아무도 렌더하지 않는다. `Drawn N`은 CPU 카운트라 정상으로 보인다.
- 오버레이는 항상 `GetBackgroundDrawList(ImGui::GetMainViewport())`를 넘긴다(`CHitAreaWire`와 같은 방식). Client 창이 화면 (0,0)에 있는 PC에서는 병합돼 재현되지 않으므로 '내 PC에서는 보인다'가 진단을 부정하지 않는다. `MainApp_WorldLevel.cpp:227`의 같은 패턴은 아직 남아 있다.
- 채움 quad는 near/far 사이이면서 한 side plane 너머 전부인 piece를 제외해야 frustum 거절 카운트와 draw work가 일치한다.

### 쿠크 timeline clipboard는 전체 Pattern snapshot이고 Paste는 Ctrl+D clone engine을 쓴다 (2026-09-18)

- Ctrl+C는 모든 lane의 선택을 ownership closure(hold·summon·group·region·companion·WORLD owner)로 닫고, 참조 정의(Logic/World/Summon/SceneProfile/PresentationResource)와 Pattern 값 전체를 snapshot으로 담는다. 부분 snapshot과 축소 remap을 따로 두면 lane을 늘릴 때마다 두 구현이 갈라진다.
- Paste는 `Clone_TimelineSelectionInto(PASTE_APPEND)`로 Duplicate와 같은 engine을 쓰되 새 row index와 usedGroupIds를 destination에서 취한다. source 기준으로 취하면 다른 Pattern에 붙일 때 ID가 충돌한다. 빈 placeholder Parent(15000ms, row 없음)는 0ms부터 배치한다.
- 삭제된 정의는 snapshot에서 복원하고 ordinal을 올리며, 변경된 정의는 `changed; copy again`으로 전체를 거절한다. Ctrl+D는 Box Detail 선택을 timeline 선택으로 유지해야 하며 engine 공유 뒤 Serialize 결과를 편집 전 baseline과 byte 비교한다.

### Mario 입장은 chain 없이도 되지만 네 소비자를 같이 풀고 데이터는 코드 뒤에 설치한다 (2026-09-18)

- ENTER_AREA→`MARIO_ENTER` admission은 projector, Client Save 규칙, Server catalog admission, publisher 네 곳에 있다. 한 곳만 completion chain 요구를 빼면 다른 곳이 P88 같은 부모를 거절한다. Gate 3·Collider region·sole Success 규칙은 유지한다.
- optional `marioStage` 0..4는 0이면 live counter, 1..4면 저작 단계이며 요청 test stage가 우선한다. 0이 아닐 때만 문서·projection·`PATTERNLOGICOUTCOME` 11번째 field로 실어 기존 행을 byte 동일하게 둔다. Server parser는 11-field 행을 FEAR로 단정하지 말고 kind로 FEAR(presentationId)와 MARIO_ENTER(stage) 를 구분한다.
- chain 없는 입장은 Client hold를 게시하지 않는다(Shared writer가 hold 0의 pattern ID를 거절하고 Client가 frozen session으로 바꾼다). 창 끝은 chain 없는 입장에서만 `startMs+durationMs`로 닫고, 같은 tick의 조기 완료와 queue된 entry는 Commit이 소비할 때까지 anchor를 유지한다.
- `marioStage` key가 있는 문서를 코드보다 먼저 설치하면 Workbench `Has_Properties`가 문서 전체를 거절하고 projector가 `unknown=[marioStage]`로 실패한다. 코드 빌드 → 설치 → Save/Publish → Server·Client 함께 재시작 순서를 지킨다.

### 쿠크/세이튼 rig는 model +X가 정면이고 root motion은 navgrid 높이 단차에서 멈춘다 (2026-09-18)

- MN_RPCZ_00·MN_RPCT_05는 model +X를 바라본다(눈/입 +X, 손 ±Z). Client는 scale-only pre-transform이므로 model +X = Transform Right = Server `lateral+`다. projector `lateral` 음수가 이미 시각 뒤 방향이며 부호를 뒤집지 않는다. `forward`는 side 축(model Z)이다.
- 2관문 쿠크 placement 옆 셀은 10.56/3.54/6.51/2.68m checkerboard이고 navpolicy는 1m 단차만 허용한다. recoil이 약 2m에서 멈추면 arena 가장자리로 단정하지 말고 F1 Show Navigation으로 셀 높이를 먼저 본다. bake 오선택이면 `.navpaint` v3 HEIGHT override, 실제 무대 단차면 데이터 유지.
- `Apply_StageRootMotion`의 navigation gate는 이제 tick segment를 1mm까지 bisect해 경계에 flush로 멈춘다. origin-relative sampling은 그대로라 곡선이 되돌아오면 origin+sample로 재개한다. 'partial XYZ commit 없음' 계약은 'last navigable point로 clamp'로 바뀌었다.

### World Object 자전과 동반 Effect·바닥 Collider를 분리한다 (2026-09-18)

- `Sample_ObjectWorld`의 key quaternion과 angularVelocity는 메시를 세우고 자전시킨다. 바닥 Collider는 이를 제외하므로 동반 Effect도 `inheritObjectRotation=false`일 때 같은 no-spin basis를 써야 한다. 이동 위치·scale·emission yaw·WORLD placement는 함께 유지하고, 기본 true로 다른 Object와 본 부착의 기존 표현을 보존한다.
- Effect의 `followObject`를 끄면 위치 갱신까지 멈춘다. 자전만 분리하려고 이 값을 끄거나 모든 Effect root에서 회전을 제거하지 않는다. WorldSequence native codec·Object Tool·Map publisher·Composition owner validator의 optional bool 지원을 함께 연결한다.
- 새 8개 칼날 group을 기존 LOOP 그대로 추가하면 P33의 일반 칼날24·갈고리30·즉사24가 기존64-window 한도를 초과한다. 원본 library는 보존하고, P33 전용 즉사8개를11초 한 번 재생하면 기존STAGGER1까지63개다. 개수·간격 축소나 parser 한도 확대로 우회하지 않는다.

- **4인 쿠크 이펙트 누락과 접속 종료를 분리한다.** Effect budget rejection은 Client
  presentation이며 같은 시각의 session terminal/Server queue·tick 근거 없이 서버 부하로
  단정하지 않는다. Release 소비자는 Debug guard 밖에서 매 프레임 알림을 drain한다.
  정상 액션 cooldown/표현 tail·동시 플레이어 수·카드 수명으로 누락을 검증한다.
  G13부터 Level/owner/remote whole-effect 개수 admission은 제거됐으며 G12 수치는 과거 값이다.
  유효성 검사와 실제 GPU 배열 크기를 임의 scene 예산과 혼동하지 않는다.400-light shader
  배열은 순서 보존 batch로 소비하며 provider/post/overlay 합계도 개수만으로 거절하지 않는다.
- **미로 entry도 대기다.** P28 전송 후 망원경 claim 전에는 role/runtime가 아직 NONE/INACTIVE다.
  권위 area HUD MAZE를 포함해 복귀 clear까지 Flow를 기다린다. 표시된 WAIT_MINIGAME만
  보고 이미 시작된 후속 audition timer까지 pause된 것으로 해석하지 않는다.
- **Sequence 무대 말단과 billboard affine basis를 확인한다.** 카메라/scene profile보다
  먼저 끝나는 WORLD lifetime은 배우만 남는 검은 공백을 만든다. 반면 비균일 parent 아래
  local 회전이 만드는 shear는 유효하다. quaternion을 쓰지 않는 billboard에서 TRS
  decomposition 성공을 강제하지 않고 축 길이/원점을 사용하며 finite 검사를 보존한다.

- **TCP 정체는 전송 실패가 아니다.** nonblocking WSAEWOULDBLOCK은 마지막 성공 byte부터
  readiness 후 재개한다.250ms 같은 경과 시간으로 session을 종료하거나 다음 frame을 먼저
  보내지 않는다. blocking SO_SNDTIMEO가 이미 낸 WSAETIMEDOUT을 안전한 would-block으로
  재해석하지 않는다. 실제 FIN/reset, reliable overflow와 명시 Stop은 별도 원인이다.
- **원격 local-only sidecar도 전체 효과 생성을 막을 수 있다.** stable element ID가 authored
  문서에서 사라졌는데 sidecar에 남으면 준비된 effect도 spawn rollback된다. catalog 전체의
  실제 연결을 검사하고 없는 참조만 정리한다. 예산 증가나 renderer 실패 무시로 가리지 않는다.
- **WORLD 좌표와 named World Object를 구분한다.** CAMERA/SOUND의 고정 WORLD 좌표는
  worldId가 없어도 정상이다. 이를 sequence identity join에 넣으면 컷씬 한 행 때문에 모든
  boss Product staging이 실패한다. 실제 named World만 sequence를 연결하고 EFFECT/LIGHT/
  COLLIDER의 필수 worldId 검사는 유지한다. 개별 row parse만으로 전체 staging을 대신하지 않는다.
- **독립 보스 효과의 명시 원본 애니메이션을 소비한다.** targeted source-boss에는 일반
  pattern animation lane이 없을 수 있다. 그때 문서의 SourceModelPreview를 기존 sampler로
  Effect-local clock에서 읽는다. 정상 pattern lane을 바꾸거나 현재 pose로 오류를 덮지 않는다.


### 쿠크 룰렛·World cue·마리오 연출 회귀 방지

- 동적 지지면을 교체할 때 source root-motion 보스도 이전/새 지면 높이 차를 한 번 받아야 한다. 곡선의 지면 상대 높이는 유지하고 새 지지면과 옛 actor Y로 음수 시작 offset을 만들지 않는다. 반복 시작의 spawn reset도 활성 지지면을 소비한다.
- charge 목적지가 walkable이어도 이동 경로는 막혀 있을 수 있다. 시작부터 목적지까지 `Has_LineOfSight`와 traversal을 함께 검사하고 막힌 경우 마지막 유효 지점까지만 이동한다.
- 비동기 V1 준비 중인 World cue는 수명 안에서 재시도한다. 뒤따르는 motion cue도 준비 중인 실제 birth를 기다리며 invalid resource와 준비 중 상태를 구분한다.
- Mario parent와 phase 2는 같은 run/member를 재사용한다. member 종료의 `iPatternSequence`를 전달·소비해 다음 패턴을 영구 차단하지 않는다. run 전체 종료와 특정 cue 파괴의 우선순위는 유지한다.
- Mario intro의 room broadcast는 모든 플레이어의 카메라 소유권을 뜻하지 않는다. 로컬 snapshot `iMarioStage`와 해당 intro를 대조하고 실제 선택된 timed camera만 입력을 막는다.

소스·집중 검사·Product 빌드·사용자 화면 확인은09-18 쿠크 패턴 재생 복구 RESULT G09 이후에서 구분한다.

### native animationTrail의 비활성 SourceRecipe도 carrier 계약이다

- `animationTrailBakedEdgeV1`에 recovered native material을 설치할 때 `SourceRecipe.enabled=false`라는 이유로 rendererShape를 무시하지 않는다. material admission은 typed carrier와 `rendererShape=animationTrail`을 함께 검사한다. 이전 sprite metadata가 남으면 GPU 이전에 전체 Effect가 거부될 수 있다.
- 실제 실패 element의 stable ID·native program·carrier를 대조하고 해당 field만 교정한다. validator를 느슨하게 하거나 shader alpha를 바꾸지 않는다. 재질 승인, product load-stage, 실제 edge playback과 사용자 GPU 표시를 따로 검증한다. 발탄420633의3개 오류와 근거는09-18 KOUKU_PATTERN_RUNTIME_REPAIR_RESULT G09에 기록했다.

### 쿠크 바닥·보스 표면·전투 선준비의 원본 대조

- 각진 보스를 낮은 LOD라고 단정하지 않는다. 설치 모델과 원본 LOD0의 모든 삼각형 위치·UV를 대조하고 정점 N/T가 면 법선으로 덮였는지 먼저 확인한다. source tangent.w도 보존해야 mirrored UV의 normal map 방향이 맞는다. WINT 1.5 후보는 기존 CModel decoder로 검증하며 골격·클립·재질·인덱스 보존과 화면 품질을 구분한다.
- 원본 LUT의 존재, volume의 실제 override BoolProperty, 참조 index를 함께 검사한다. 특정 맵 이름 whitelist로 다른 활성 LUT를 누락시키지 않는다. Kouku의 원본 volume 46/47은 LUT02/01 override가 실제 활성이다. LUT02는 중간 밝기를 올리므로 LUT 누락 하나로 과도한 밝기를 설명하지 않는다.
- alias에 qualityOverride가 없으면 현재 Level의 base quality를 상속한다. globalQuality만 읽어 실효값을 추정하지 말고 Get_ActiveLevelQuality와 profile multiplier, camera region까지 소비 순서대로 대조한다. 사용자가 방금 저장한 품질·조명은 최신 디스크 기준으로 보존한다.
- 원본 directional light의 excludevolumes와 Lightmass/character indirect 계수를 별도로 확인한다. character SH brightness를 맵 전체 uniform ambient로 곱하지 않는다. scene/camera region으로 directional을 끄는 것은 구역 단위 근사이며 receiver별 convex exclusion 완성으로 기록하지 않는다.
- native specular power를 복원해도 Phong/Blinn 수식이 다르면 반사 폭이 틀어진다. 실제 MIC의 원본 PS와 marker producer를 대조해 해당 carrier만 고친다. Kouku floor family1/2는 3관문 바닥이며 1·2관문의 BG8 RNM 원인을 대신하지 않는다.
- 쿠크 Release 선준비는 BossCatalog만으로 닫히지 않는다. 실제 published presentation, 사용하는 Sequence resource, enabled World effectTracks, Server가 선택하는 카드·공 target을 기존 V1/V2 준비 경로로 수집한다. Debug는 클래스·marker·BossCatalog의 기존 선준비를 유지하고 추가 전체 closure는 기존 lazy 경로로 처리한다. CSO 사전 컴파일과 JSON/texture/model GPU 준비, 발생별 instance allocation을 구분한다. Release 필수 준비 실패는 입장 실패로 처리하며 Client 실행 없이 무끊김을 확정하지 않는다. 이미 병렬인 V1 worker를 늘리기 전에 같은 corpus에서 설정별 처리 시간을 측정한다. 느린 V1 로그 일부의 interval을 V2/World 포함 전체 입장 시간으로 대신 기록하지 않는다.
- Pattern 삭제의 단순 배치/Flow/Bundle 참조는 확인창에서 설명한 뒤 같은 draft transaction으로 제거한다. Logic/Summon의 필수 타깃은 명시적으로 차단한다. 외부 저장은 draft 편집 자체를 막는 이유가 아니며 실제 Save의 CAS와 실패 시 보존은 계속 필요하다.
- Save와 Publish 시간 차이는 projection, Gameplay 검증·직렬화, owner lock을 나눠 측정한다. 큰 Encounter만 보고 PowerShell JSON이나 provenance 검증을 병목으로 단정하지 않는다. 반복 deepcopy, 동일 World 문서 digest, WModel·pose·root curve 재계산과 key별 불변조건 반복을 먼저 실측한다. memo는 한 publication의 pinned 입력·root에 묶고 ID 기반 key의 객체 수명을 유지한다. native freshness·최종 산출물 검증을 캐시로 대신하지 않으며 동일 입력의 정상 생성 결과를 byte 비교한다. 실측과 적용 범위는 [09-20 게시·이펙트 로딩 RESULT](09-20/2026-09-20_KOUKU_PUBLISH_AND_EFFECT_LOADING_RESULT.md)를 따른다.

근거와 실제 적용·검증 상태는09-19 KOUKU_RENDERING_QUALITY 및 KOUKU_BOSS_SOURCE_BASIS RESULT,
09-18 KOUKU_PATTERN_RUNTIME_REPAIR RESULT G10을 따른다.

### 새 섬 맵 추출에서 드러난 변형 도구 전제와 프로토콜 번호 (2026-09-19)

- `build_map_material_variants.py`는 쿠크 한 Area로만 검증됐었다. 마하라카 섬에서 네 전제가 깨졌다: 패키지 루트 부모 재질은 UModel이 이름만 적는다(`zzzbg_simple_opa_inst`), 베이스 추출기는 역할 텍스처만 팩에 복사하므로 "UModel이 내보냄"은 "팩에 있음"이 아니다, `cook`이 `--package-root`를 넘기면서 인자를 정의하지 않았다, 메시 슬롯 수를 넘는 component override가 있다(UE3는 조회하지 않는다). 새 Area마다 inventory `--expect-*`를 실측값으로 넘기고 첫 실패를 원인별로 닫는다. 세부는 09-19 MAHARAKA_ISLAND_LEVEL RESULT.
- cook 출력 경로에 64자 asset ID가 두 번 들어가 260자를 넘으면 geometry contract가 임시 파일을 못 찾는다. 출력 root를 짧은 경로로 둔다.
- 변형 install 폴더(`Map/<AreaId>`)는 소유 영수증 CAS가 영수증 밖 파일을 거부한다. 랜드스케이프는 `--pack-name`으로 별도 폴더(`Map/<AreaId>_LAND`, Bern은 `_T`)에 둔다. 추출기 기본 pack 이름은 Bern이다.
- 원본 glTF normal/tangent가 평행하면 `prove_native_static_parallel_basis.py`로 원본 package/serial hash와 모든 indexed vertex의 위치·UV·packed N/T를 대조한다. 원본에도 동일한 평행 basis가 있다는 증거가 일치할 때만 기존 native-parallel cook flag를 전달한다. 증거 없이 geometry contract를 완화하거나 배치를 숨기지 않는다.
- 변형 cook 산출물의 emissive 슬롯을 그대로 믿지 않는다. 변환기는 emissive가 없는 재질에도 자리표시자 `t_tds_specular04`(파랑·노랑 타원)를 emissive 슬롯에 묶는다. 마하라카에서는 변형 382개 중 299개가 이것만 갖고 있어 섬 전체에 얼룩이 나왔다. `mapmaterials`가 없는 Area는 legacy 경로라 `Shader_VtxMeshMapInstance.hlsl` PS_MAIN이 `emissive texture * g_EmissiveIntensity`만 그리고 그 값이 카탈로그 행의 render profile `emissiveIntensity`다. 진짜 emissive 변형은 남기고 자리표시자만 가진 변형만 `renderprofiles.json`에서 0으로 끈다. render profile은 에셋 단위라 한 변형 안에서 슬롯별로 다르게 켜고 끌 수 없다(진짜+자리표시자 혼합 변형은 그대로 둔다). 쿠크 Area에도 같은 자리표시자가 506개 설치돼 있다.
- WORLD_ID 추가처럼 wire를 바꾸는 작업은 병합 대상 main의 최신 `NETWORK_PROTOCOL_VERSION` 다음 번호를 쓴다. 브랜치마다 같은 번호를 다른 내용에 쓰면 번호 검사는 통과하고 패킷 해석이 어긋난다(09-19에 main 91·93과 작업본 91이 충돌).

### 트리거는 진입으로 발동하지 않고 G로 발동한다 (2026-09-19 정정)

- 처음 원인: `CServerTriggerSystem::RUNTIME_TRIGGER.hasFired`는 트리거 하나당 하나였다. 첫 플레이어가 발동하면 그 방의 모두에게 소진되고 `Initialize`나 방이 비는 초기화(`Reset_ReplayableArenaWhenEmpty`: Character Select·Valtan·Kouku만)에서만 풀렸다. Bern·수련장·마하라카 방은 서버를 다시 켜기 전까지 돌아오지 않는다. 저작 기본값이 `triggerOnce=true`라 게시된 활성 트리거 55개 중 40개가 이 상태였다. 지금은 `Initialize`가 `isTriggerOnce`를 지운다(`Set_HonourTriggerOnce(true)`는 테스트 옵트인). 새 트리거 종류를 넣을 때 "한 번만"을 `hasFired`로 다시 만들지 말 것.
- 정정된 요구: **밟기만 해서는 발동하지 않고 볼륨 안에서 G를 눌러야 발동한다.** 처음 작업은 진입 발동을 남겨 틀렸다. 기준은 `ServerTriggerSystem.cpp`의 `AUTO_ENTRY_RULES` 한 표다. 여기 있는 (월드, 종류, id 접두사)만 진입 발동이고(컷신 `playSequence`, Valtan 복도 웨이브·보스 시작. 같은 날 오후 정정으로 Kouku `Mario*` 이동 레인은 표에서 뺐다. 아래 마지막 항목) 나머지는 진입하면 `[ G ]` 제안만 하고 G가 실행한다. 새 종류의 트리거를 자동으로 발동시키고 싶으면 그 표에 행을 넣는 것이지 진입 경로에 분기를 더하는 것이 아니다. 저작 `requiresInteract`는 표보다 우선한다.
- **밟으면 알아서 이동하던 원인 후보**: Debug Valtan의 복도 지름길(`Build_ValtanStageBypassMove`, `Place_PlayerAtValtanAuditionBait`)은 `Stage_2`·`Stage_3`·`Stage_Boss`를 밟는 순간 플레이어를 옮겼다. 이 중 `Stage_2`는 웨이브(`spawn.valtan.stage03`)라 웨이브 대신 앞으로 나가는 이동이 됐고, 같은 날 밤에 `Stage_1`·`Stage_MiniBoss`처럼 지름길 표(`DESTINATIONS`)에서 뺐다. `Stage_3`도 저작 이동(절벽, 100.42/20.53/-86.95)을 그대로 실행하도록 표에서 뺐다. 지금 지름길은 `Stage_Boss`뿐이다. 웨이브 트리거를 지름길 표에 넣지 말 것. 진입이 곧 발동이던 시절에는 저작 트리거를 G 전용으로 바꿔도 이 Debug 이동이 남는다. 지금은 `Fires_OnEntry`가 지름길 트리거를 G 전용으로 만들고 `Run_Trigger`가 G에서 실행한다.
- 소환 트리거는 래치와 별개로 `CSpawnGroupRuntime::Activate`(DORMANT 전용) 때문에 한 번만 됐다. 재발동은 `Activate_Repeat`를 쓴다. 진행 중 그룹을 다시 활성화하지 않고, 끝난 그룹은 그 그룹의 몬스터가 0마리일 때만 다시 시작한다.
- G 요청: Client는 Server가 제안한 박스 ID를, 제안이 없으면 `INTERACT_TRIGGER_HERE_ID`(`@here`)를 보낸다. Server는 어느 쪽이든 그 플레이어의 **현재 위치로 볼륨을 다시 판정**하고, 진입 발동 대상 박스는 건드리지 않는다. 안정 ID 패턴에 없는 문자라 실제 ID와 겹치지 않고 wire 모양이 같아 프로토콜 번호는 그대로다.
- G 안내를 화면에 그리는 표시는 다른 팀원이 작업 중이라 Bern·Valtan의 `[ G ]` 텍스트는 넣지 않는다(2026-09-19에 우리가 추가했던 것을 제거했다). 제안 상태(`CCombatHUDViewModel::Get_InteractPromptTriggerId`)는 모든 방의 HUD 뷰모델에 그대로 저장되며 지금 이를 그리는 곳은 Kouku 레벨의 기존 표시뿐이다. G 입력과 서버 안내는 유지한다.
- 진입 발동 대상 박스만 "막힌 진입 재시도"를 쓴다. 스킬·피격으로 바쁠 때 진입 edge를 소비하지 않고 다음 틱에 다시 시도한다. 이동 중(`TRIGGER_MOVE`)은 제외해 순간이동 연쇄를 막는다.
- **플레이어가 스스로 움직이는 트리거는 전부 G다 (2026-09-19 오후 정정).** `AUTO_ENTRY_RULES`에는 컷신(`playSequence`)·카드미로 망원경·Valtan 웨이브·보스 시작만 남는다. 마리오 안의 뛰어내리기·올라가기·건너가기와 마지막 출구, Kouku `jump.*`, Valtan 시작 지점과 나머지 `movePlayer`는 G로 발동한다. 마리오 입장 자체는 `Mario*_Intro` 컷신과 `Update_MarioControlState`의 OBB 판정이 하므로 그대로 자동이다. 새 이동 트리거가 밟으면 발동해야 한다면 그 표에 행을 넣는 것이지 진입 경로에 분기를 더하는 것이 아니다.
- **G 경로도 방 소유 진입 핸들러를 거친다.** 마리오 lane 박스는 `CGameRoom::Begin_MarioTriggerMove`가 stage 일치·권한 잠금·접촉 행동 중단과 마지막 출구의 복귀 좌표(`Resolve_MarioReturnDestination`)를 맡는다. G가 이것을 우회하면 마지막 출구가 저작 좌표(`[-2,1.3,942]`)로 보낸다. `Activate_Interact`/`Activate_Here`가 `moveEntry`를 받고 `Run_KeyTrigger`가 진입 경로와 같은 핸들러를 호출한다. 방 소유 진입이 더 생기면 `Handle_InteractTrigger`(`GameRoom_PartyWorld.cpp`)에도 넘길 것.
- **G 키캡은 원본 `requiresInteract` 상자 위에서만 그린다.** 서버 제안 id로 키캡을 그리던 `CInteractKeyPromptView::Update(..., strOfferedId)`와 `Is_Showing()`은 제거했다(2026-09-19, 다른 팀원이 G 표시를 작업 중). 뷰는 원래대로 `<Area>.viewer.world.json`의 `requiresInteract` 트리거 박스 10m 안에서 Kouku 레벨만 쓴다.
- **발탄 트리거 표식은 이동(`movePlayer`) 트리거에만 띄운다 (2026-09-19).** 처음 구현이 활성 트리거 9개 전부에 띄워서 웨이브·보스와 도착 쪽 박스에도 표식이 떴다. `Stage_MiniBoss_Spawn`(웨이브)은 `Stage_MiniBoss` 이동의 목적지 (50.87, 10.14, -81.02)와 좌표가 같아 도착점에 표식이 생겼다. `CLevel_ValtanArena::Load_TriggerMarkers`가 이벤트가 정확히 하나이고 `MOVE_PLAYER`인 박스만 고른다. 목록이 아니라 종류 기준이므로 MapTool에서 이동 트리거를 추가하면 표식이 자동으로 생기고, 도착 쪽에 이동 트리거를 두면 그곳에도 뜬다. 쿠크는 원작 제작자가 고른 id 목록을 그대로 쓰며 이 규칙과 무관하다.
- **발탄 `Stage_Boss`는 보스를 시작하고, 발동한 플레이어를 `Stage_Boss_ArenaEntry` 박스 중심으로 보낸다 (2026-09-19).** `CServerTriggerSystem::Run_Action`이 이 트리거만 특수 처리한다(`Place_PlayerAtValtanArenaEntry`). 보스 시작은 이미 떠 있으면 거절되지만 그 뒤에 온 플레이어도 보낸다(8인 레이드가 한 명씩 걸어 들어가지 않게). 목적지 XZ는 게시된 `Gameplay.world.json`의 ArenaEntry 박스 위치이고, 높이는 방이 `Set_GroundSampler`로 넘긴 네비 바닥이다. 박스 Y는 손으로 저작한 값이라 바닥에서 떠 있을 수 있다(2026-09-19 저장본은 25.73, 바닥 22.84). 이동 시간과 호는 ArenaEntry 박스가 저작한 이동(0.8초, 0)과 같다. Debug의 159 bar 벽 돌진 유도 지점은 더 이상 `Stage_Boss`가 부르지 않고 패턴 audition(`GameRoom_ValtanAudition.cpp`)만 쓴다. ArenaEntry 박스를 MapTool로 옮기면 착지도 따라가지만 `Publish-WorldGameplay.ps1`로 게시해야 서버가 새 위치를 읽는다. 스킬·피격으로 바쁜 플레이어는 보내지지 않고 보스 시작만 일어난다(밖으로 나갔다 다시 들어오면 보내진다).
- **트리거 위치 표식(`effect.world.move_destination`)은 그 레벨의 로딩이 미리 준비해야 뜬다.** `CClickMoveEffect::Queue_LevelResources`가 Kouku와 Valtan에서만 이 Effect를 큐에 넣는다. 다른 레벨에 표식을 붙이려면 이 조건에도 그 레벨을 넣어야 `Spawn_LevelPlacement`가 준비된 target을 찾고, 없으면 그 표식은 조용히 retired 된다.
- **새로 구운 몬스터·캐릭터 wmodel은 clip을 30 t/s로 다시 표현해야 한다 (2026-09-19 쿠크 몬스터 4종에서 재발)**: 쿠킹 파이프라인은 clip을 1000 t/s로 저장하는데, 엔진은 저장된 rate를 무시하고 `CAnimation`의 `COOKED_TICK_RATE`(30)로 재생한다(`Engine/Public/Animation.h:24`, `Engine/Private/Animation.cpp:51-52`). 그대로 쓰면 걷기·공격이 33배 느려 굳은 듯 미끄러진다. 쿠킹 직후 `python Tools/ActorXAssetCooker/retime_wmodel_ticks.py --wmodel <경로> --ticks-per-second 30 --expect-ticks-per-second 1000`를 실행하고, 모든 clip의 rate가 30인지와 clip 길이(초)가 원본과 같은지 측정한다. clip 이름이 wmodel에 있는지만 본 것은 검증이 아니다. 이 절차는 09-10 마리오 때 한 번 고쳤지만 그 RESULT에만 있어 쿠크 몬스터 4종(NPC_480701~480704)에서 다시 빠졌다. 그 4종은 같은 날 밤 다시 표현했고 근거는 `2026-09-19_KOUKU_NORMAL_MONSTERS_RESULT.md` 끝 절에 있다.


### Stage 진행과 이펙트 수명·복구 소비자 (2026-09-20)

- 긴 Effect/Sound/World row를 수용하려고 마지막 Stage를 늘리면 clip 종료 뒤 보스가 정지해 기다린다. Stage 합계와 row lifetime을 별도로 게시하고 자연 완료 occurrence의 원래 시작 tick·definition revision·판정 ledger·재생 핸들을 유지한다. 명시 Stop과 자연 FINISH를 같은 cleanup으로 처리하지 않는다. 다음 sequence의 clock을 이전 이펙트에 적용하지 않으며 독립 tail에도 자체 종료·강제 취소가 필요하다. 자동 RaidFlow Entry 전환은 같은 epoch를 유지하고 수동 Play/Restart와 구분한다.
- `Full lifetime` 편집이 Stage를 바꾸는 구현과 설명을 함께 제거한다. Stage 축소/확대가 다른 row를 자르거나 이동시키지 않는지 실제 Save/reopen/preview expansion으로 검사한다. 늦은 World/Logic/Summon은 다음 보스의 sequence를 가져오지 않고 태어난 occurrence의 소유권을 사용한다.
- 과광은 recovered material이라는 이유만으로 prebaked 중복이라고 단정하지 않는다. base exposure × scene alias, source LUT, 실제 shader 분기, light receiver와 baked flag를 따로 읽는다. V2 base-color bright-pass는 emissive slot의 bloomIntensity와 다른 소비자이므로 해당 leaf의 sceneBloomScale로 조절한다.
- 맵 과광을 줄인 뒤 캐릭터만 어두워지면 native character의 실제 ambient 수식도 확인한다. 현재 adapter는 `direct diffuse × ambient`라 diffuse0 구역에서는 저장된 ambient가 있어도 기여0이다. 원본 CharacterLit/ShadowedIndirectBrightness를 map ambient에서 제거한 상태와 캐릭터 간접광 복원을 구분한다. 전역 노출 복원이나 모든 native 재질의 밝기 변경으로 숨기지 않고 수신 대상별 입력을 검토한다. world baseline×원본 계수의 uniform 근사는 원본 SH 복원이 아니다.
- finite World Effect의 재발생 주기에 particle tail까지 포함하면 방출이 끊긴다. emission cadence마다 새 cycle을 시작하고 이전 tail은 자기 prepared duration까지 겹쳐 유지한다. native infinite emitter와 일반 one-shot의 수명은 바꾸지 않는다.
- 연출 중 BOSS bone 부착은 실제 표시되는 파생 World actor의 CModel pose를 찾아야 한다. 다른 전투 NPC의 동명 bone이나 synthetic anchor 성공은 화면의 손 부착 검증이 아니다. 같은 뿅망치 모델을 써도 휠윈드와 카드미로의 occurrence Transform은 분리한다.
- 공포 화면 얼굴은 거미 보스 머리가 아니라 collider hit → buff → Darkness/Fear → screen ParticleSystem 연결을 추적한다. source SizeOverLife는 보존하고 반복 주기를 요청에 맞춰 추가했다면 그 cadence만 PROJECT_TUNED로 기록한다.
- 빙고 보드의 경계·뒤집힘은 보드 actor의 normal-to-mark/mark/mark-bingo 원본에서 찾는다. 이름이 비슷한 boss skull projectile을 보드 원본으로 대신하지 않는다. source flip의 마지막 경계 burst까지 재생한 뒤 무한 유지 모션으로 전환한다.

### 2026-09-20: 캡처와 row 소유권의 실제 소비자

- 같은 captured SRV를 연결한 성공과 UV 구도 성공은 별개다. 정규화 WModel UV를 임시 world-position으로 바꿀 때 native zoom/U·V offset까지 계산해 중앙의 중복 이동을 실측한다. Color/Bloom은 한 crop helper를 공유한다. ALT V의 좌표 adapter와 화면 평면 액자 제어는 PROJECT_TUNED이며 미해독 원본 camera CB 복원으로 기록하지 않는다.
- Stage 종료는 다음 동작으로의 전이다. 늦은 row는 born pattern/sequence/start tick와 immutable catalog를 보유하며, 실제 primary 생존은 별도로 검증한다. dependent Summon admission, clone liveness, counter/shield와 GC pin까지 같은 소유권을 사용해야 한다. BOSS_CURRENT만 살아 있는 위치·방향을 읽고 BOSS_SPAWN/stage origin은 고정한다. 자동 RaidFlow 다음 Entry는 같은 epoch로 넘기되 명시 Play/Restart/Stop과 관문 변경은 취소한다.
- PlayParticleEffect의 base ParticleSystem이 null이면 첫 문자열을 base TRS로 해석하지 않는다. model-specific CEFParticleDataModifier를 타입과 byte provenance로 분리한다. Full Restore는 명시 cue asset ID와 실제 animation clip join으로 연결한다.

- 원본 연출 모델은 Scene component Materials override를 LookInfo와 mesh 기본 MIC보다 우선한다. 같은 source family 이름으로 native 재질을 대체하지 않고 실제 ShaderCache 함수·constant packing·Light varying ABI까지 대조한다. Actor64 dead MIC의 Light 입력은 UV=v2, light=v3, view=v5, position=v6이다.

### Server 기동 준비와 Product 컴파일 결과

- Server는 접속할 월드 하나만이 아니라 시작 시 등록된 여섯 world와 각 navigation을 초기화한다. MAHARAKA를 당장 플레이하지 않아도 `MAHARAKA.worldbootstrap` 또는 `LV_OCN_EVENTIS_MHP.navgrid`가 없으면 listener 생성 전에 종료한다. `server connection failed`에서 IP를 변경하기 전에 Server 초기화 오류를 확인한다.
- 특정 encounter만 Publish한 결과와 Server 전체 준비를 구분한다. Product는 compile/deploy 경로이고 데이터 게시나 실제 Server 시작 검증을 자동 수행하지 않는다. 전체 world 정본은 `Publish-WorldGameplay.ps1 -Mode Publish -WorldId ALL`, 빠진 navigation은 `Publish-ServerNavigation.ps1 -Mode Publish -AreaId <실제 AreaId>`로 게시한다. 누락 산출물을 직접 작성하거나 다른 world 파일로 대체하지 않는다. 정상 초기화와 실제 endpoint listener/TCP 도달을 따로 확인한다.

### 2026-09-20 재검토: 별도 조명 입력·Box/Motion·capture 중심
- native character ambient를 직접광색에 곱하면 G1처럼 directional0인 장면에서 몸체가 검어진다. 선택적 sourceCharacterAmbient는 직접광과 독립이며 기본0, native map은 제외한다. source SH와 uniform 근사를 구분한다.
- profile raw quality와 active multiplier를 혼동해 부모 multiplier를 재상속하지 않는다. V1 explicit effect bloom은 scene intensity와 선택 관계다. LUT 런타임 경로는 기존 `Map/Lighting/KoukuSaydon`을 사용한다.
- 휠윈드 Pattern World Box의 TRS를 손 Object Motion에 다시 쓰지 않는다. saved Motion→Box→live bone 순서로 합성한다. 고정 Pattern을 편집하는 quick panel Preview는 현재 선택이 아니라 원 소유 Pattern ID를 전달해야 한다.
- ALT V captureUseModelCenter는 실제 cube의 첫 pose 월드 중심을 사용한다. 그 중심이 발밑이면 capture도 내려간다. screen Transform을 bounds로 덮지 말고 이미지·액자·cube의 같은 camera rig에 적용한다.
- Complete Play는 큐 등록 수가 아니라 actual prepared/current/failed0을 확인한 뒤 typed start/READY를 보낸다. Debug 준비 제한과 시작 후 gameplay 시간은 분리한다.
- 카메라 source 끝과 긴 World 행 끝을 구분한다. 완료 row guard는 Area 재획득을 막되 늦은 Seek의 기존 camera lease도 인계·복귀해야 한다. 상세 검증은09-20 RAID_PRESENTATION_REPAIR_IMPLEMENTATION_RESULT G09.
- Stage보다 늦게 끝나는 Logic/World/Summon 행을 늘릴 때 기존 명시 `pattern.durationMs`도 확인한다. Stage 길이는 애니메이션 진행, 명시 수명은 남은 행 보유를 담당한다. 검증기의 행 범위 검사를 느슨하게 하거나 Stage를 늘려 누락을 숨기지 않는다. 개별 패턴이 Unavailable로 격리돼도 전체 Publish는 성공할 수 있으므로, 변경 패턴의 inventory와 실제 `Prepare_PatternFlow` 허용 결과까지 확인한다.
- 돌진 이동과 머리 방향이 반대라면 Effect에 보정 회전을 먼저 넣지 않는다. 실제 설치 CModel의 해당 clip 전방과 Server의 world 이동 벡터·body yaw를 따로 측정한다. Kouku 거미는 +X 모델 전방에 −90도 보정이 맞으며 기존 +90은 역방향이었다. 수정 시 body-local 피해 영역과 별도 카운터 방향 조건도 함께 검사한다.
- yaw처럼 음수를 허용하는 계약은 validator와 Server parser뿐 아니라 bootstrap 숫자 formatter도 signed 경로여야 한다. `PATTERNLOGICCHARGE`의 yaw는 기존 `Format-InvariantSignedFloat`, 거리는 nonnegative formatter를 사용한다. 허용 범위 안의 음수와 양수 모두 실제 행 생성으로 검증한다.

### 쿠크 기본 방향광 복구와 시퀀스 소비자 (2026-09-20)

- directional RGB를 복구할 때 scene LIGHT_DESC의 기본 receiver=ALL까지 확인한다. map light의 UNBAKED만으로 scene directional의 baked 중복을 막지는 못한다. Scene Profile light.receiver와 region.receiver는 기존 GPU 수광 계약을 사용하고, 정상 카드미로/Mario 영역의 명시ALL을 보존한다. 노출1에서 직접광을 복구한 결과와 과거 노출2의 전체 HDR 결과를 동일하다고 기록하지 않는다.
- 피자 소환은 서버 spawn10 성공만 확인하지 않는다. ClientReplication dependent archetype admission까지 실제 G2_KOUKU와 owner entity를 연결해야 한다. stationary Showtime도 Saydon의 +X 전방을 moving pursuit와 같은 -90도 yaw로 계산한다.
- Complete Play 활성 guard가 로컬 Reset/Play를 소비하기 전에 return/continue하면 Save가 성공해도 편집 재생은 이전 서버 소유 상태에 막힌다. 명시 사용자 Reset과 내부 선택의 Stop_Preview를 구분하고 기존 typed STOP 뒤 최신 로컬 요청을 소비한다. 시퀀스 종료의 책/맵 준비 비용과 HUD 표시 bool 변경을 구분한다.

- Rendering Workbench의 임시 노출/LUT/FXAA/Bloom 비교값은 다음 프레임 camera-region 보간 전에 원래 품질로 복원해야 한다. 비교된 현재 exposure에 다시 배율을 곱하면 프레임마다 밝기가 누적된다. 새 profile commit은 이전 복원 snapshot을 폐기하고, 닫기/Level 변경은 비교 옵션을 해제한다. Save는 catalog만 직렬화하며 임시 renderer 값을 저작값으로 역수집하지 않는다.


### 연출 원본 음성·자막과 재생 시계

- WAV가 이미 원본 전체 layer/길이를 담고 있어도 SOUND 행이 짧으면 끝이 잘린다. 누락 판단은 bank Event→Action→media 목록과 설치 WAV, 실제 occurrence 시작·끝을 함께 대조한다. Stop event와 시작 지연·fade도 재생 계약에 포함한다.
- World 연출의 sound는 actor별로 시작하지 않고 instance별 stable soundTrackId로 시작한다. 배우가 여러 명이면 같은 음성을 중복 재생하지 않도록 한다. sound tail은 visual/camera/전투 잠금 수명과 분리하고 자연 완료 때만 보존한다. Stop/Seek/Level 정리는 owner handle을 종료한다.
- 외부 시계로 World를 매 프레임 샘플링하는 편집기/연출은 PLAYING의 continuous seek와 사용자의 discontinuous scrub을 구분한다. 매 프레임 paused=true 또는 기본 discontinuous Seek를 적용하면 원본 사운드가 계속 멈추거나 재생성된다.
- JSON source가 codec상 유효해도 pretty-print로 16MiB 문서 경계를 넘을 수 있다. 기존 compact World 문서의 저장 스타일을 보존하고 실제 publisher를 통과시킨다. 개별 source extraction·설치·게시·실제 화면/청취 확인을 구분한다.


- 고정 장판의 생성 위치를 유지하려면 BOSS anchor를 MAP으로 바꾸기 전에 follow 정책을 확인한다. 기존 BOSS pivot의 생성 시점 snapshot과 followBoss=false 경로는 위치·회전·크기를 보존한다. exact asset의 내부 transform/attachment도 끝단 decal world까지 확인하고 정상인 다른 색/shape 행은 바꾸지 않는다.
- 동적 발판의 Server Y가 맞아도 Client 정적 navigation 기반 이동 예측이 매 frame 덮을 수 있다. Server support 포함 판정과 snapshot.canPredictMove, Character의 예측/보간 분기를 같이 확인한다. 입력 command 송신과 로컬 예측 허용은 별개 계약이다.
- 세부 navgrid를 재베이크해도 Client의 CNavigation 예측이 기본 격자만 읽으면 이동 경계와 높이가 어긋난다. detail 소비자와 snapshot.canPredictMove를 함께 확인한다. 최상단 삼각형 baker는 머리 위 장식·의자·접힌 종이도 선택할 수 있으므로 셀 크기 축소만으로 완료하지 않고 실제 바닥과 머리 공간을 실측한다. 복층 아레나는 실제 입장점의 연결 성분이 전투 바닥에만 머무는지 확인하며, 하부를 상판 높이로 메우지 않는다. 쿠크 5구역의 근거는 `09-21/2026-09-21_KOUKU_FINE_NAVIGATION_RESULT.md`를 따른다.
- XZ teleport는 기존 높이를 유지하는 계약이다. 바닥 착지가 필요한 한 occurrence만 별도 typed policy로 분리하고 공유 logic definition의 다른 소비자를 확인한다. 현재 root Y만 보정하면 다음 Stage가 잔여 offset을 origin으로 캡처할 수 있으므로 실제 마지막 curve와 Stage/Pattern 종료 높이까지 검사한다.

### 2026-09-20 Sound 리소스 목록과 구간 편집

- 대형 Sound 목록은 매 프레임 resource 구조체를 복사하거나 화면 밖 Selectable을 모두 제출하지 않는다. source는 Refresh/검색 변경, Created는 draft generation/검색 변경에서만 재구성하고 가시 행만 그린다. 파일 길이는 선택한 WAV만 확인하고 실패도 cache해 반복 I/O를 막는다. physical inventory의 임시3000ms를 실제 WAV 길이로 clamp하면 긴 음원을 편집할 수 없어진다.
- 사운드 바 왼쪽 trim은 timeline start와 soundSourceStartMs를 같은 양만큼 변경해야 WAV 앞부분이 잘린다. timeline start만 바꾸면 동일한 처음 부분을 늦게 재생한다. 오른쪽은 길이, 가운데 이동은 timeline 위치만 변경한다. Source In/Out 저장·Play 소비자와 별도로 서버/로컬 preview 전환의 이전 문서 재생 문제를 검증한다.
- Effect의 앞 edge trim도 occurrence `effectSourceStartMs`를 바꾼다. 가운데 이동은 source-in을 보존한다. source asset의 emitter delay를 지워 모든 사용처를 바꾸지 않는다. source clock에 offset을 더했으면 anchor history에는 그 offset을 빼고, 잘린 앞구간의 pre-roll은 occurrence의 생성 basis를 사용한다. 최초 출력은 authored delay와 fixed-step의 실제 생성 sample을 구분한다.
- Sound 그룹 허용은 UI뿐 아니라 C++ codec·Python projector까지 함께 연결한다. Effect+Sound 일괄 이동은 같은 delta와 각 source-in·fade·volume을 보존하며 Collider의 공유 판정 창은 중복 이동하지 않는다.
- 피해 Collider의 자동 Trigger는 명시 `colliderDamageContactRole`과 반복 조건으로 선택한다. 표시 이름을 무시한 전체 ENTER_AREA 동등 검색은 무관한 잡기 정의를 재사용한다. 피해 전용 window만 역할을 다시 연결하고 실제 hold·비피해 결과·Fail/Timeout 기믹은 보존한다.
- 누적 재생 시계에 정수 ms 표시값을 매 프레임 되쓰지 않는다. double로 dt를 더해도 표시값을 다시 대입하면 소수부가 소실되어 60Hz·50초에서 타임라인이 2초 늦어질 수 있다. 내부 clock의 분수부를 보존하고 외부 clock·명시 Seek·capture 경계 이동만 지정 시각을 적용한다. 저장·재로드 성공과 오디오/시각 시계 동기화는 별도로 검증한다.
- Save의 디스크 성공은 실행 중 Preview snapshot 갱신을 뜻하지 않는다. local snapshot의 draft generation을 추적하고 변경 저장 뒤 이전 재생을 STOP하며 다음 Play는 최신 문서를 사용한다. Scrub/Resume도 stale 문서에 transport만 보내지 않는다. 준비 요청을 최신으로 표시한 뒤 실제 admission이 실패하면 같은 Pattern뿐 아니라 다른 Pattern 교체도 STOP으로 정리한다. pending Play→edit→Save와 실패→Resume를 함께 검사한다. 부모 바 밖의 음원은 자식 Pattern 및 Server/product owner도 구분하고 임의로 모두 mute하지 않는다.
- **마리오 lane의 `rightSign`은 그 lane을 비추는 follow 카메라의 화면 오른쪽에 손으로 맞춘 값이라, 어긋나면 ←/→가 반대로 움직인다 (2026-09-20 `Mario4_Tigger_2 -> Mario4_Tigger_5`)**: Client는 화면 기준 LEFT/RIGHT만 보내고 Server가 `Configure_MarioRail`에서 `normalize(출구 위치 - 이동을 마친 위치) * MARIO_LANES의 rightSign`(`GameRoom_Internal.h`)을 레일 오른쪽으로 삼는다. 그 자리를 비추는 shot(Client는 박스가 겹치면 우선순위가 가장 높은 것, 동률이면 목록 앞을 고르고 `플레이어 위치 + eyeOffset`을 월드 오프셋으로 적용한다)의 화면 오른쪽과 부호가 반대면 입력이 뒤집힌다. 화면 오른쪽은 왼손 좌표계에서 `(fz, -fx)`이고 `(fx, fz)`는 `lookAtOffset - eyeOffset`의 수평 성분이다. lane을 추가하거나 카메라 shot·트리거 위치를 옮기면 모든 lane에서 (→ 방향 · 화면 오른쪽) 내적이 양수인지 수치로 다시 확인한다(`out/MarioDirectionCheck/mario_direction_check.py`, 방법은 `2026-09-20_KOUKU_MARIO4_DIRECTION_INVERT_RESULT.md` 3절). 서버 계약 테스트의 lane 표(`ServerGameplayContractTests_DebugTeleport.cpp`)는 서버 표의 복사본이라 함께 고쳐야 하며, 이 lane은 17행짜리 테스트 표에 없어 부호가 한 번도 단언되지 않았다.
- **쿠크 컷신 중 다른 무대 구역이 보이던 원인과 격리 규칙 (2026-09-20)**: 쿠크 맵의 배치 3,369개는 서로 떨어진 무대 구역 17곳(60m 연결 거리로 묶은 군집, 원본 sub-level `SL01~SL05`·`SCENE01A`와 대응)에 흩어져 있는데 제품 로딩은 전체 scope이고 카메라 far가 `max(2000, span*8)`라 컷신 카메라가 다른 구역을 그대로 그렸다. 컷신 중 주변을 숨기는 범용 장치는 없었고(`KAKUL_ARENA_HIDDEN_PLACEMENT_IDS`와 게이트 오브젝트 처리는 특정 연출 전용) 원본에서도 주변 숨김 규칙은 확인하지 못했다(앵콜 컷신 SCENE07A의 `ToggleHidden`은 컷신 출연 액터를 컷신 동안만 보이게 한다). 지금은 `Is_CinematicPresentationActive()`가 true인 동안 카메라와 전방 40m/100m 표본점이 속한 구역(AABB+80m)만 그리고(로컬 플레이어는 기준점이 아니다) 나머지 구역의 배치는 `CMapPlacementRuntime::Set_RuntimeSuppressed` 오버레이로 숨긴다. 논리 표시(`Set_RuntimeVisible`)는 건드리지 않으므로 팝업북 아레나 교체 같은 기존 로직과 값이 충돌하지 않고, 복원은 오버레이 해제 하나다. 지금 재생 중이거나 자세를 유지 중인 시퀀스가 소유한 배치(`CWorldSequencePlayer::Collect_OwnedPlacements`)와 스케일 100 이상 배경물(5개)은 숨기지 않는다. 지킬 것: 새 무대를 다른 무대에서 60m 안에 두면 한 구역으로 합쳐져 함께 보인다. 구역에서 300m 이상 떨어진 컷신 카메라(2관문 입장 후반 shot, 지하 y -100)는 구역 배치를 전부 숨기고 시퀀스 소유 대상만 그린다. Deploy 소품 7개와 NPC·보스·플레이어 엔티티는 이 규칙의 대상이 아니다. 2차 수정(같은 날): 처음에는 로컬 플레이어 위치와 모든 시퀀스의 바인딩(814개)을 유지·면제 기준에 넣었는데, 플레이어가 다른 무대(2관문 SL03)에 서 있는 채로 F1 Workbench 1관문 팝업북 미리보기를 재생하자 그 무대의 배치 92개가 400m 밖 먼 조각으로 남았다(로그 `stageSuppressed=2402` = 유지 구역 팝업북 아레나 + SL03). 유지 구역은 카메라 기준점으로만 정하고, 면제는 재생 중인 시퀀스의 것으로 좁힌다. 검증 하네스가 "플레이어는 컷신 무대에 있다"고 가정하면 이런 누수를 놓치므로 플레이어를 다른 무대에 둔 경우를 항상 함께 계산할 것. 로그 `stageKept=x,z|x,z`가 유지 구역 중심이다. 근거와 검증 수치는 `.md/GB/09-20/2026-09-20_KOUKU_CUTSCENE_HIDE_SURROUNDINGS_RESULT.md` 10절에 있다.
- **컷신 항목(패턴) 하나를 Boss 탭과 Sequence 탭에 추가하는 절차와 함정 (2026-09-20 `앵콜컷신`)**: 두 탭은 별개 문서다. Boss 탭은 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`, Sequence 탭은 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`이고, 같은 컷신도 각 문서에 패턴이 따로 있으며(ID 접두사만 다르다) 둘 다 `camerashots.json`의 shot과 `Data/Effects/V2`의 페이드 이펙트를 참조한다. 게이트별 트리는 `KoukuSaydonActionWorkbench.cpp`의 `Render_PatternTree`가 gateId와 folder/bundle 유무로 만들며, 폴더 없는 패턴은 게이트 바로 아래 `표시이름 [Actor]`로 나온다. `Tools/KoukuSaydonPipeline/build_gate_cutscenes_g12.py`는 CONFIGS 전체를 다시 만들고 id가 지금 문서와 어긋나 있어(id=8이 현재는 `1관문_연출`) 새 컷신을 위해 다시 돌리지 않는다. 컷신 하나만 추가할 때는 `Tools/KoukuSaydonPipeline/build_encore_cutscene.py`처럼 기존 `빙고_최종엔딩씬` 패턴을 복제하고 쓰기 직전에 baseline을 다시 확인한 뒤 원자 교체한다. Composition 두 문서는 파이썬 indent=2 + CRLF로 다시 써도 바이트가 같지만 `camerashots.json`은 실수 표기가 섞여 있어 텍스트 삽입으로만 고친다. 자막·가짜 클리어 UI·깨진 유리·배우 굽기(slot a/b 밖의 skelcontrolstrength 무시)는 이 파이프라인이 표현하지 못하므로, 패턴이 생겼다고 그 요소가 들어간 것이 아니다. 근거와 미포함 목록은 `2026-09-20_KOUKU_ENCORE_COMPOSITION_ENTRY_RESULT.md`에 있다.
- **원본 컷신 배우(Matinee 그룹)를 World Sequence로 옮길 때 조용히 어긋나는 세 가지 (2026-09-20 앵콜컷신 세이튼)**: ① `build_source_sequences.actor_world`가 쓰는 `base.source_times`는 `cim_constant` 계단을 `round(ms)`와 `ms-1`에서 `world_pose(ms/1000)`로 샘플한다. 원본 키 시각이 7833.34ms처럼 정수 ms 바로 위면 `round`가 내림해 그 시각에는 아직 옛 값이 나오고, 새 값은 다음 200ms 경계에 도착해 배우가 계단 대신 167ms 동안 약 44m를 미끄러진다(앵콜 후보에서 실측). 계단 다음의 첫 정수 ms(`floor(t)+1`)를 키에 더한다(`build_encore_cutscene.add_step_keys`). 기존 baker는 고치지 않았고 다른 컷신에 같은 미끄러짐이 있는지는 확인하지 못했다. ② baker의 `rotationQuaternion`은 scipy가 w<0 부호로 줄 수 있는데 Composition/World Sequence 검증기는 정규화된 w≥0을 요구한다(`must be normalized with non-negative w`). q와 -q는 같은 회전이므로 부호를 뒤집어 저장한다. ③ Boss 문서 `worlds[].worldId`는 `kakulsaydon.g1.world.<N>`(N<nextWorldOrdinal)만 투영기 `_validate_catalog`가 받는다(`world.kouku.gate2.intro.*`만 예외). `world.kouku.bingo.encore.saydon`을 넣으면 `project_kouku_saydon_composition.py --check`가 `worldId must use kakulsaydon.g1.world.<N> below nextWorldOrdinal`로 실패했다(Sequence 문서 선례의 `world.kouku.*` 형식을 그대로 복사하면 안 된다). 또 baked wmodel 폴더에는 원본 `textures/`가 함께 있어야 한다. `write_clip`이 복사하지만 후보 Resources root에서 구우면 빠지므로 설치 때 같이 옮긴다. 근거: 09-20/2026-09-20_KOUKU_ENCORE_COMPOSITION_ENTRY_RESULT.md 10절.
- **베른 castle↔castle.2 / library↔library.2 왕복 트리거는 예외적으로 밟자마자 발동하고, 화면이 어두워진 뒤에 이동한다 (2026-09-20).** 플레이어가 스스로 움직이는 movePlayer 상자는 기본이 G키 대기라서 `AUTO_ENTRY_RULES`에 `{BERN, MOVE_PLAYER, "castle"}`, `{BERN, MOVE_PLAYER, "library"}` 행이 있어야 G 없이 발동한다(id 접두어라 `castle.2`, `library.2`도 포함, Bern의 다른 movePlayer 상자는 계속 G). 도착 자리가 짝 상자 안이라 서버가 새로 밟은 것으로 보고 곧바로 되쏘는 무한 반복을 막으려고, Bern에서만 `Evaluate_Entries`가 방금 착지한 플레이어를 그 틱의 진입 발동에서 뺀다(`m_TriggerMoveInFlight`로 TRIGGER_MOVE가 끝난 틱을 안다. 나갔다 다시 들어와야 발동). 어두워진 뒤 이동은 서버가 `TriggerMove.fHoldSeconds`(Shared `BERN_TRAVEL_HOLD_MS`=500)만큼 제자리에 묶고 그 뒤 이동시키며, 클라는 `CSongCastGaugeView`의 전역 암전을 Bern 레벨에서만 TRIGGER_MOVE 동안 `BERN_TRAVEL_FADE_OUT_MS`(300)로 검게 만든다. 쿠크 아레나의 속도 휴리스틱 암전은 그대로이며 Bern 레벨에서만 켜지므로 이중 암전은 없다. 스퀘어홀은 TRIGGER_MOVE가 아니라 SQUAREHOLE_SONG 액션이라 겹치지 않는다. 이동 이벤트의 `durationSeconds`는 게시기 최소값 0.05이고 targetPosition의 Y는 짝 자리의 서버 네비 높이를 썼다(트리거 상자 Y와 최대 0.13m 차이). 프로토콜 94는 그대로지만 Shared 헤더가 바뀌므로 Server와 Client를 함께 빌드·재시작한다. 게시기 검증은 스크래치 루트(`Publish-WorldGameplay.ps1`의 repoRoot는 스크립트 상위 2단계)에 `Data` junction을 만들고 후보 파일만 복사해 저장소를 건드리지 않고 할 수 있다. 이때 junction은 `cmd /c rmdir`로만 지운다(`Remove-Item -Recurse`는 링크 대상까지 지울 수 있다). 자세한 검증 범위는 `.md/GB/09-20/2026-09-20_BERN_CASTLE_LIBRARY_TRAVEL_RESULT.md`.
- **쿠크 앵콜컷신은 프로젝트 배우가 원본보다 1.7배 커서 카메라를 배우 루트 기준 1.7배 멀리 둔다 (2026-09-20).** 원본 SCENE07A의 배우 `쿠크세이튼_03`(export 24)과 컴포넌트(export 224)에는 DrawScale/Scale 속성이 없어 메시를 기본 크기(wmodel 단위 × 0.01)로 그린다. 프로젝트 몸체 `MN_RPCT_05`는 `bodyModelPreScale 0.017`이라 구운 배우가 1.7배 크다. 원본 카메라(위치·시선·FOV 29.395°)는 맞았고 발이 화면 아래 끝에 오는 것도 원본 영상과 같았다. 틀어진 것은 머리·몸통이 화면 위로 나가는 것뿐이었다. 세계를 배우 루트 A 기준으로 균등 배율하면 화면이 같으므로 배우를 줄이지 않고 `eye' = A + k(eye - A)`, `lookAt' = A + k(lookAt - A)`(k=1.7, up·FOV 유지)로 카메라 키를 배우 위치 키 시각에 맞춰 다시 만든다(`Tools/KoukuSaydonPipeline/fit_encore_camera_to_actor_scale.py`, 26키, 멱등이며 `--ratio`로 재조정). 다른 컷신에 배율을 옮기기 전에 그 배우의 원본 Actor/Component에 DrawScale이 있는지 UPK 속성(`Tools/.../encore_upk_props.py` 방식)으로 먼저 본다. 09-18 쇼타임의 1.4167(=0.017/(0.01×1.2))은 그 배우에 drawscale 1.2가 명시돼 있었기 때문이다. 배율은 이론값(0.017/0.01), 클로즈업에서 원본의 빨간 코가 화면 중앙에 오는 조건(눈 높이 2.36m ÷ 카메라 축 높이 1.373m = 1.72), 전신 구간 정점 범위 대조(1.4~1.8)가 겹쳐 정했다. 카메라 키는 shot당 최대 128개(`CAMERA_TRACK_MAX_KEYFRAMES`, CameraTool)이므로 배우 위치 키를 5mm 오차로 줄여 쓴다. 영상과 컷신의 시간 대응은 눈대중(+5.4초)이 아니라 프레임 차이 최대점으로 잡는다: 유리 파열 섬광 v=9.80초 = 컷신 15.433초로 오프셋 5.63초. 카메라를 옮기면 원본과 달리 배경이 움직이므로(원본은 카메라 고정, 배우가 카메라에 붙음) 배우 위치 키가 뛰는 8.167·20.4·20.933초에 카메라가 각각 약 2.1m·1.9m·0.6m 순간 이동해 배경이 튄다. 카메라는 원본보다 약 3m 뒤·위(높이 12.26m → 14.81~14.93m)로 가므로 천막·천장 지오메트리와 겹치는지는 화면에서 확인해야 한다. 화면 확인은 사용자 몫이며 이 항목은 수치 검증(스킨 메시 정점 투영, 등가성 NDC 차이 4e-4 이하)만 끝난 상태다. 근거: 09-20/2026-09-20_ENCORE_CAMERA_FIT_RESULT.md.

- **main과 병합할 때 두 브랜치가 같은 "다음 번호"를 쓰면 ID가 조용히 겹친다 (2026-09-20 쿠크 `PATTERN_94`, `world.40`).** 쿠크 Composition은 저장할 때 `nextPatternOrdinal`/`nextWorldOrdinal`을 올리므로, 같은 기준에서 갈라진 두 브랜치가 각자 새 패턴·월드를 만들면 서로 다른 내용이 같은 ID(`KAKULSAYDON_G1_PATTERN_94` 메두사공포 대 앵콜컷신, `kakulsaydon.g1.world.40` 뿅망치 대 앵콜 세이튼)를 갖는다. git은 텍스트 충돌만 보여 주므로 JSON을 ID 기준으로 비교해 "양쪽이 같은 ID를 추가했고 내용이 다름"부터 찾고, 한쪽을 새 번호로 옮기면서 그 ID를 참조하는 모든 파일(Composition, Encounter, patternbindings)을 함께 바꾼다. 생성 출력물(`KoukuSaydonEncounter.json`, `KoukuSaydon.patternbindings.json`)은 손으로 합치지 말고 병합된 Composition으로 재생성한다. 재생성 전에는 `sourceRevision`이 Composition `revision`보다 낮은 오래된 상태다.
- **병합 충돌 해결 때의 함정 세 가지 (2026-09-20).** (1) 해결 스크립트가 실패해도 `;`로 이어 둔 `git add`는 실행되어 충돌 마커가 남은 파일이 "해결됨"으로 스테이징된다. 해결 명령은 `&&`로만 잇고 스테이징 뒤 `git show :0:<파일> | grep -c "^<<<<<<<"`로 0개를 확인한다. (2) autocrlf 때문에 작업 사본은 CRLF, 인덱스 blob은 LF라 바이트 앵커를 쓰는 해결 스크립트는 줄끝부터 감지해야 한다. main의 `gotchas.md`처럼 `CR CR LF`로 저장된 파일은 main 원본 바이트를 그대로 두고 우리 줄만 같은 줄끝으로 붙인다. (3) `git status`가 내용이 같은 파일을 계속 M으로 표시할 수 있다. `git update-index --refresh` 뒤에도 남으면 blob 해시와 `cmp`로 같음을 확인한 뒤 `git checkout -- <파일>`로 정리한다.

### 쿠크 Composition 프로젝터 검증·테스트를 돌릴 때 (2026-09-20 앵콜 자막 작업)

- `project_kouku_saydon_composition.py`는 같은 폴더의 `raid_flow_projection`을 이름만으로 불러오고, 단위 테스트는 `Tools.KoukuSaydonPipeline...`로 불러온다. 테스트는 `Tools/KoukuSaydonPipeline`에서 `PYTHONPATH=<저장소 루트>`를 주고 실행한다. 루트에서만, 또는 폴더에서만 실행하면 `ModuleNotFoundError`로 수십 개가 오류가 되어 자막·데이터 문제로 오해하기 쉽다.
- `project_raid_gates`나 `projected_outputs`를 원본 JSON에 직접 호출하면 `GATE1 flow cannot admit unavailable KAKULSAYDON_G1_PATTERN_1`이 병합 전 버전에서도 난다. 생성 결과가 필요하면 `prepare_publication(source, root)`를 거쳐 `projected_outputs(document, root, inventory)`를 호출한다(실제 `_run`과 같은 경로, 디스크 쓰기 없음).
- 원본 Composition을 고친 뒤 `--mode validate`는 `projected Product is stale: ...KoukuSaydonEncounter.json`으로 끝난다. 생성 단계는 통과한 것이고 게시로 산출물을 갱신하라는 안내다. 자막 오류가 아니다.
- 프로젝터가 원본을 읽는 중에 같은 파일을 `git stash`나 편집으로 바꾸면 `Summon Pattern must be another same-Gate, same-actor Pattern` 같은 엉뚱한 오류가 난다(경합). 검증 도중에는 원본을 건드리지 않고, 다시 돌려 같은 오류가 나는지 확인한다.
- 쿠크 자막 배치는 `presentationOccurrences`에 `{occurrenceId, resourceId(subtitle.kouku.<GameMsg id>), startMs, durationMs, anchorKind: "MAP", followBoss: false}`이다. 자막 리소스는 이미 등록돼 있어도 배치가 없으면 화면에 안 나온다. 저장은 revision을 1 올리고 해당 패턴의 `nextPresentationOccurrenceOrdinal`을 함께 올린다.
### Debug JSON 로딩은 빈 컨테이너 생성과 전체 consumer ABI를 함께 본다

- 같은 /O2라도 /MDd와 Debug STL의 할당 비용은 남는다. DATA_JSON_VALUE의 모든 scalar에 string/vector/map/order를 생성하던 구조를 활성 payload만 생성하도록 바꿔 실제 119개 입력의 parse·digest·해제에서 약57~59% 감소를 확인했다. 전체 맵 입장이나 GPU 개선율로 확대하지 않는다. 상세 수치는 `09-21/2026-09-21_DEBUG_LOADING_CPU_RESULT.md`를 따른다.
- Debug는 같은 문서의 3worker 처리가 1worker보다 느릴 수 있다. thread 수를 늘리기 전에 parse/decode와 renderer 준비를 분리하고 같은 입력·할당량·순서 교대 시간을 비교한다. 기존 필수 준비 장벽이나 validation을 지워 시간을 줄이지 않는다.
- JSON value의 메모리 배치가 바뀌면 DataJson OBJ 하나만 기존 Client나 probe에 링크하지 않는다. public header를 소비하는 모든 TU를 정상 의존성 빌드로 다시 컴파일한다. /MDd와 /MD 또는 iterator ABI를 파일별로 혼합하지 않는다.
- `Effect.Prepare.Document/Metadata/Renderer/Commit`과 `V1.prepare.*`는 CPU 단계다. 부모 total과 자식 단계, 서로 병렬인 target 시간을 합산해 전체 진입 시간으로 표시하지 않는다.
- MSVC map은 move construction에서도 sentinel/proxy 할당이 남을 수 있다. payload owner를 분리하는 것만으로 속도 개선을 단정하지 않고, parser가 최종 owner 안에 직접 구성하는 경계까지 비교한다. 공개 값의 deep copy·이동 후 재사용·할당 실패 보존을 유지하며, 할당 감소와 wall time 감소는 별도 증거다. 10-04 실측과 재현 도구는 `10-04/2026-10-04_DEBUG_EFFECT_LOADING_IMPLEMENTATION_RESULT.md`를 따른다.
- Raid Publish의 원문 hash/generation 봉인이나 Python 검증 cache를 Client Effect parse cache와 혼동하지 않는다. 파싱 결과를 지속 산출물로 만들고 기존 Client consumer가 읽어야 실행 시 비용을 옮길 수 있다. GPU 객체와 현재 재생 owner/clock은 별도이며, Save·Publish·실행 중 Server 적용 완료도 구분한다.

### Complete Play는 저작 revision·최종 응답·수신 소비 순서를 함께 확인한다

- Action revision이 같아도 Sequence revision은 별도로 게시되어야 한다. 저장 Sequence와 Encounter raidGates, Server Gameplay.bootstrap RAIDGATE의 revision을 대조한다. 오래된 게시본의 거절을 timeout으로 오인하지 않도록 exact request ID의 최종 Server 응답을 보존한다. timeout은 승인·거절이 아니며 같은 session에서 미확정 START를 자동 재시도하지 않는다.
- raw queue 전체를 typed queue로 옮긴 뒤 소비자를 실행하면, raw 4096 한도보다 작은 lifecycle 64 한도가 정상 backlog를 연결 오류로 바꿀 수 있다. 한 Update의 dispatch를 제한하고, 다음 목적지 queue가 차면 FIFO head를 보존한 채 소비자에게 반환한다. reliable lifecycle을 버리거나 snapshot처럼 합치지 않는다.
- ENTER_ACCEPTED의 world reset 뒤 같은 수신 배치의 spawn/snapshot은 새 world 입력이다. session 종료의 전체 폐기와 world 전환의 typed state 정리를 구분해 검증한다. 기록된 queue overflow와 반복 이펙트 의심은 각각의 증거로 조사한다.
- 수정·재게시와 실제 화면 재생 완료를 구분한다. 근거와 실행 범위는 `09-21/2026-09-21_KOUKU_COMPLETE_PLAY_RECEIVE_RESULT.md`에 기록한다.

### Play Pattern 준비 결과와 공포의 조건부 사운드

- Play Pattern 버튼의 enqueue 안내는 Server 송신·승인 증거가 아니다. 일반 Pattern도 MainApp/BossTool의 Gate·리소스 준비 실패와 exact audition 상태를 Workbench에 전달한다. completion-chain/Mario 전용 local Preview 정책으로 명시적 Server Play 상태 추적을 제한하지 않는다.
- 게시 P15의 실제 Server admission·collider FEAR 성공과 실행 중 Client의 준비 실패를 구분한다. 사운드를 패턴 전체 timeline에 추가하면 회피한 플레이어도 듣는다. 피격자 전용 사운드는 FEAR result의 optional soundResourceId와 기존 local FEAR session으로 연결한다.
- 원본 FEAR buff에 AkEvent가 없으면 같은 거미 동작의 보이스 재사용을 원작 얼굴 사운드 복원이라고 기록하지 않는다. effectDelayMs에서 한 번 재생하고, 반복 얼굴 펄스마다 중첩시키지 않는다. 근거는 09-21 KOUKU_SPIDER_PLAY_AND_FEAR_SOUND_RESULT에 둔다.


### 쿠크 안전존·돌진·컷씬 파생 모델

- 안전존을 플레이어의 전역 무적 또는 지난 tick의 접촉 여부로 구현하지 않는다. 해당 Pattern의 현재 활성 Collider를 Result 이전에 모으고 같은 실행의 즉사·체력 비례 피해·공포만 차단한다. 표시 pulse는 입장 및 2초마다 Server가 발행한다. 공포와 무적이 같은 플레이어에게 표현될 수 있으므로 텍스트 dedup key는 owner만으로 공유하지 않는다.
- MAP 기준 폭발과 BOSS 기준 collider는 보스의 이전 yaw가 같을 때만 우연히 겹칠 수 있다. 실제 V1 mesh 변환·각 폭발 occurrence의 위치·시각·yaw를 측정해서 같은 기준으로 판정을 생성한다. 강제 밀림 거리만 늘려도 잘못 연결한 collider는 고쳐지지 않는다.
- 기존 배우에 컷씬 animation set을 붙일 때는 원본 WORLD 경로와 b_root 변위를 분리해 확인한다. 무대 밖 경로를 일반 root 이동에 맡기면 navigation에서 막힌다. 명시 bossMotion keys로 합성한 경우 body root 억제를 함께 적용하고 같은 skeleton·clip 충돌·실제 weighted bone world 위치를 확인한다. 정수 ms key의 보간 오차와 최종 화면 판정은 구분한다.
- 패턴 시작 위치 복원은 yaw 복원을 뜻하지 않는다. 고정 방향 찍기는 authored resetBossYawDegrees까지 지정하고 플레이어 조준은 실제 모델 전방(+X/+Z)과 Server retarget·시각·collider가 일치하는지 네 방향으로 검증한다.
- 네비게이션 이탈을 막으려고 `animationRootHorizontalScale`과 `bossChargeDistanceM`을 모두 0으로 만들면 휠윈드 이동 경로가 사라진다. 기존 target capture·navigation/body collision clamp를 유지하고 실제 원본 전방과 charge yaw를 각각 확인한다.
- `materialSourceModelAssetId`는 컷씬 파생 WModel의 geometry를 교체하지 않는다. 본체가 smooth normal/tangent 복구됐어도 worldsequences의 별도 modelAssetId를 전수 확인한다. Character WModel 전체를 복사하면 컷씬 전용 animation이 손실되므로 indexed corner 기준 basis만 복구하고 나머지 section bytes를 보존한다. 근거는09-21 KOUKU_SAFE_ZONE_CHARGE_CUTSCENE_RESULT를 따른다.


### 쿠크 밀침 방향과 낙사 경계

- BOSS_FORWARD는 body local+Z다. 콜라이더가 local+X인 레이저·바주카는 중앙+90도, 사선은 각자의 저작 yaw까지 포함해 Result 방향을 설정한다. 공유 Result의 수치를 직접 바꾸면 무관한 팡파레까지 바뀌므로 요청 접촉만 복제 Result로 연결한다.
- 높이 차이만으로 외곽 낙사를 판정하지 않는다. 명시된 arena-exit 밀림만 원본 바닥·runtime blocker·진행 방향의 내부 틈을 구분하고 collision 해결 후 최종 위치를 재검증한다. body slide가 안전 바닥이나 내부 장애물로 방향을 바꿀 수 있으므로 충돌 전 외곽 후보를 그대로 낙사에 사용하지 않는다. 실제 경계에 도달하지 않은 유한 거리 밀림을 즉사로 바꾸지 않는다.
- 강제 밀림은 기존 FEAR·다운·grace·밀림을 교체하는 명시 Result 정책이며 전역 충돌·사망·잡힘 제한을 제거하지 않는다. 같은 Pattern 안전존은 피해와 그 결과의 밀림을 함께 차단한다. 근거는09-21 KOUKU_SAFE_ZONE_CHARGE_CUTSCENE_RESULT의G06을 따른다.

### V2에서 V1으로 옮긴 ScreenPost의 실제 재생 계약

- V2 profile 이름과 시작 강도만 복사하면 V1에서 색수차가 거절되거나 fade가 사라질 수 있다. codec 토큰·typed profile·lifetime·강도 곡선·Engine 제출을 같이 확인하고 원래 요소와 occurrence는 유지한다. optional `intensityLerp=false` 기본값과 기존 source dynamic/alpha 우선순위는 보존한다. 문서 struct를 늘린 native 검증은 같은 헤더로 소비자 OBJ를 재컴파일하며, CPU 수치 성공을 GPU 화면 성공으로 기록하지 않는다.

### CubeSample의 밝은 배경과 선택적 재질 보정

- 배경을 읽는 CubeSample의 `5*C^5` 본체가 tone shoulder에 몰리면 Bloom 증폭만으로 면과 모서리 대비를 복구할 수 없다. 실제 blend 상태와 배경 샘플 식을 구분하고, 대응 skillbinding/cue가 쓰는 저작 문서와 Tool 비교본을 모두 확인한다.
- CubeSample의 선택적 `project_clarity_strength`는 기존 named scalar와 `SourceScalars0.w`를 사용한다. 생략/0은 기존 계산이며 유한한 0~1만 허용한다. Parse/Save가 통과하는 공통 MaterialValidation과 resource staging에서 검증하고 다른 재질에 전파하지 않는다.
- SceneColor를 보정하는 gain과 coverage는 Bloom/black 보조 평가에서도 실제 SceneHDR로 고정한다. 억제된 배경 Bloom을 HDR에서 다시 생성하지 않는다. 큰 HDR 값의 `lerp(큰 값, 작은 값, 1)`은 상쇄될 수 있으므로 보정 transmission은 가중합으로 계산한다. alpha clip은 원본 alpha로 판정한다.
- opt-off 동일성, 어두운 배경, 밝은 유색 HDR, alpha 끝값과 Bloom 운반 수치 검사는 실제 맵 화면의 선명도 승인과 구분한다. Q 적용 범위·검증·남은 단계는 `09-21/2026-09-21_DIMENSIONMASTER_Q_CUBE_CLARITY_IMPLEMENTATION_RESULT.md`를 따른다.
- 배경 의존 본체는 `F(black)`에서0이므로 본체 밝기 제한을 조정하거나 Bloom intensity만 올려도 자기 발광은 생기지 않는다. 독립 발광을 추가할 때는 세 scene read mode에서 같은 값을 더하고 `F(SceneBloom)-F(black)+Write_SceneBloom(F(black))`를 유지한다. 광도 압축만 하고 alpha를 높이면 밝은 배경을 어두운 면으로 더 많이 교체하는 회귀가 생긴다.
- 합성 보라색 입력의 shader 검사로 실제 Q의 금색을 검증했다고 기록하지 않는다. 실제 particle `(7,6,1,.6)`, MIC tint, 원본 DDS, face/edge UV, 활성 tone과 alpha 합성을 사용한다. `R>G>B`만으로 금색이 충분히 남았다고 판단하지 말고 백색화 정도도 비교한다. Character Select의 LUT OFF는 source tone OFF가 아니며 directional OFF도 baked 배경을 제거하지 않는다.
- unlit·자체 RGB·개별 Bloom은 최종 맵 tone/LUT 제외를 뜻하지 않는다. 보라색이 특정 맵에서 청색으로 바뀌면 실제 활성 region/LUT와 동일 HDR 입력의 후처리를 먼저 비교한다. 원본 추출이 검증된 LUT도 의도적으로 큰 색 회전을 만들 수 있으므로 재질 누락이나 잘못된 텍스처로 단정하지 않는다.
- 대상의 청색화를 줄이려고 최종 LUT를 통째로 해제하면 배경의 분위기와 대비도 함께 바뀐다. LUT 복구와 환경광 조정을 한 번에 섞지 않고 같은 카메라의 비교 기준을 보존한다. light receiver는 조명 수신 경계이며 합성 이후 LUT의 대상별 제외 기능이 아니다.
- 3D 투명 이펙트의 색을 보호할 때 이미 섞인 pixel 전체에서 LUT를 끄면 배경까지 바뀐다. 원본 SceneColor 투과와 자체 발광을 구분하고 depth·반투명 정렬·distortion·Bloom을 포함한 합성을 검토한다. 화면 overlay를 3D 유리의 우회 경로로 사용하지 않는다.

### 차원술사 유리의 coverage와 T의 환경 조명 경로

- 큰 유리 면이 불투명하게 보일 때 particle HDR 색만 보고 최종 pixel을 추정하지 않는다. 실제 native 함수·DDS·alpha와 설치 mesh의 view/normal을 대조한다. V 시작 native66의 세 occurrence는 기존 Fresnel 지수만0.2→0.5로 조정해 정면 coverage를 낮추고 grazing rim을 보존했다. 원본 MIC 오류 수정이나 전체 native family 보정으로 확대하지 않는다.
- T 소환체의 환경 조명 수신 여부는 최종 tone/LUT와 별개다. 정확한 ModelCue의 pass 선택과 NONBLEND/mask를 함께 확인한다. 현재 사용자 요청에 따라 T exact predicate의 pass11을 정상 pass0으로 복구했으며 전체 shader pass 재배열이나 다른 cue 변경은 하지 않았다. T 전체 particle full restore 완료와도 구분한다.

### 2026-09-21 쿠크 연출·빙고·크기 반영 경계

- Composition World ID는 첫 행의 접두사로 추측하지 않는다. `world.kouku.gate2.intro.*`는 동일 suffix의 `world.sequence.instance.kouku.gate2.intro.*`와 한 쌍인 source 전용 ID다. 새 저작 행은 `kakulsaydon.g1.world.<ordinal>`을 쓰고 nextWorldOrdinal·모든 occurrence 참조를 함께 갱신한다. Action뿐 아니라 Sequence도 Python 게시 진입점과 실제 Client codec으로 검증한다.
- Effect 고정 월드 좌표는 MAP anchor다. WORLD는 실재 World resource ID가 필요한 오브젝트 추적이다. 카메라의 WORLD 허용을 Effect에 그대로 옮기지 않는다.
- V1 World Object effect는 부모 scale을 보존한다. 원본 fixed-axis sprite의 XY 크기가 바닥면으로 재배치되므로 offset·최종 quad 폭/길이/높이를 실제 renderer 행렬로 검사한다. 필요한 요소에만 followEmitterAxisRotation을 적용한다.
- Native ScreenPost의 원본 재질 곡선은 authoring binding 검증과 GPU 제출 snapshot 양쪽에 연결해야 한다. 시간별 수치 검사와 FXC 성공을 실제 화면 확인으로 기록하지 않는다.
- Character Size는 선택 맵의 Camera profile에 저장된다. 다른 맵 값과 전체 배율을 함께 비교한다. 컷신의 카메라 preview 제한 때문에 크기 적용/Reload까지 막지 않으며, Development 진입에서도 크기를 명시적으로 초기화한다.
- 게시 도중 Product admission의 transaction lock 거부는 종료·재실행·재빌드로 우회하지 않는다. owner publish 완료 후 실제 CGameplayCatalog::Load_PublishedKoukuProduct 결과와 정확한 status를 확인한다.

### 빙고 Sequence 격리와 실행 중 WORLD 게시

- P10 앵콜은 BINGO + enterCombatOnFinish=true다. Server/publisher만 이를 허용하고 Client가 GATE1~3만 허용하면 행이 격리돼 0ms/0stage가 된다. MAP SOUND의 고정 음향 anchor도 source codec·Product reader·publisher에 함께 연결해야 하며 조명 오류 문구만으로 Light resource를 수정하지 않는다.
- 아레나 입장 뒤 WORLD를 게시하면 저장 Product와 Level의 이전 WORLD snapshot이 달라질 수 있다. 새 Complete Play 준비에서 idle base를 공식 Load 경로로 갱신하며 미저장 draft와 독립 cue는 보존한다. 같은 Area refresh에서 Loader의 AREA_LEAF CPU snapshot을 지우면 마리오 연기가 사라지므로 sequence 전용 cache와 구분한다.
- Client가 새 EXE로 시작됐는지, source row가 실제 reader에서 격리되지 않는지, 사용자가 컷씬을 본 결과를 구분한다. 수정·검증은 [빙고 재생 결과](09-21/2026-09-21_KOUKU_COMPLETE_PLAY_WORLD_AND_BINGO_RESULT.md)를 따른다.

### 컷씬 본 단위와 검증 후보의 실제 설치

- 원본 SkelControl translation의 cm와 설치 Character 본 로컬 단위를 구분한다. armature scale100을 실측한 본체에서는 해당 translation에0.01을 적용하고, 회전·scale·다른 clip은 보존한다. 얼굴이 늘어나는 현상을 애니메이션 누락이나 Character 경로 오류로 단정하지 않는다.
- 검증 도중 수정된 후보가 이전 통합 staging에 자동 반영되지는 않는다. 최종 검증한 파일 SHA와 설치 직전 staging·설치 후 파일 SHA를 연결한다. 형식 version 숫자만 바꾼 effect는 필수 root 누락으로 거부되며 admission 완화로 우회하지 않는다.
- 결과와 설치 증거는 `09-21/2026-09-21_KOUKU_COMPLETE_PLAY_WORLD_AND_BINGO_RESULT.md`의G06/G07을 따른다. 실제 화면·음향과 프레임 시간은 사용자 확인 전 자동 검사 성공으로 대신 기록하지 않는다.

### Debug Server 웨이브 트리거와 수동 재소환

- Kouku `Book1_Monsters`/`Book2_Monsters`와 Valtan `Stage_1`/`Stage_2`는 Debug Server에서 밟아도, G를 눌러도 시작하지 않는다. 대신 F1 `Normal Monster 1/2`(쿠크는 `KoukuSaydon Arena`의 `Bingo Board` 아래, 발탄은 아레나 안에서만 보이는 `Valtan Arena` 헤더)로 다시 소환한다. Release Server는 이전처럼 밟으면 시작한다. Debug에서 "밟았는데 안 나온다"를 트리거 데이터나 navigation 결함으로 조사하기 전에 빌드 구성을 확인한다.
- `Stage_MiniBoss_Spawn`(`spawn.valtan.stage02.miniboss`)은 이 네 개에 속하지 않아 Debug에서도 밟으면 시작한다. `Stage_2`가 시작하는 그룹은 `spawn.valtan.stage03`이며 G로 움직이는 `Stage_3` 이동 상자와 다른 기능이다.
- 억제는 컴파일 시 `_DEBUG`이고 `CGameRoom`이 `CServerTriggerSystem::Set_SuppressWaveMonsterTriggers(true)`를 Debug에서만 호출한다. 이 상자들의 제품 동작은 Release 빌드에서 확인한다. 무엇이 나오지 않았는지 볼 때 `[Trigger] Fire Trigger=...` 줄이 Server 콘솔에 찍혔는지가 첫 단서다.
- `C2S_DEBUG_RESUMMON_WAVE_MONSTERS`는 protocol 100에 추가됐다. protocol 99의 무적 구역 연출 펄스와 별개이므로, 두 기능을 함께 가진 Client와 Server는 같은 protocol 100으로 빌드해야 한다.
- 근거와 검증 범위는 `09-21/2026-09-21_DEBUG_WAVE_MONSTER_BUTTONS_RESULT.md`에 기록한다.

### MapTool 컷신에서 새 V1 월드 이펙트를 즉시 seek할 때

- MapTool은 MainApp의 일반 `Commit_PendingSpawns` 뒤에 컷신을 seek한다. 같은 editor frame에서 태어난 V1 world-root 이펙트를 다음 frame까지 pending으로 남긴 채 root/외부 시계 sample을 실패로 처리하면, 하나의 effect 실패가 컷신 배우 전체 해제로 확대된다. `TARGET_SET`의 명시적 editor-only flag로 해당 handle만 scoped commit한 뒤 seek하고, 게임 Level/전투 consumer의 post-update 경계를 전역으로 바꾸지 않는다. `Queued admitted Effect`는 resource/document 실패 증거가 아니라 commit 전 상태일 수 있다.

### V1 bounded source loop에 포함된 보조 light

원본 Cascade source가 `EmitterLoops=0` sprite/mesh/ribbon emitter와 같은 source visual program의 `LIGHT`/`light` carrier를 함께 가질 수 있다. bounded source-loop 검증은 반복 방출을 수행하는 admitted particle/ribbon carrier를 최소 하나 요구하되, 그 보조 light를 particle/ribbon이 아니라는 이유로 거절해서는 안 된다. 반대로 light만으로 loop0 조건을 만족시키면 안 된다. `effect.valtan.cinematic.trash.actor106.at667`이 이 경우이며, 원본 emitter count나 `loopEffectToDuration`을 변경해 우회하지 않는다.


### 투명 배경막을 통과할 때만 캐릭터가 밝아지는 경우

포인트 광원 추가가 보이지 않고 카메라가 이펙트 안으로 들어갈 때만 배우가 밝아지면,
재질의 조명 지원 여부와 조명 이후 alpha 합성을 구분한다. RGB0·alpha1인 unlit 막은
이미 계산된 배우 색을 덮으며 one-sided 외벽은 내부 camera에서 컬링된다. 원래 camera,
실제 skinned pose, effect geometry의 교차를 검사하고 emitter root나 광원 gizmo만으로 판단하지 않는다.
검은 원본 particle color가 CDO에서도0이면 흰색 기본값으로 바꾸지 않는다. 해당 occurrence의
배경막을 튜닝하면 외벽 방향과 배우 전체 포함 범위를 확인한다. 파생 WModel의 정점/index를
바꾸면 WGEO payload SHA, geometry bounds, 생성 도구 식별과 metadata SHA를 함께 재생성한다.
해시가 이전 payload를 가리키면 CWMeshReader가 거부하고, CModel 생성 실패가 같은 문서의
포탈 전체 staging 실패로 이어질 수 있다. reader의 무결성 검사를 완화해 우회하지 않는다.

JSON Parse/Validate_Drawable와 원시 geometry 광선 검사는 CModel 로드 검증이 아니다.
원본·기존 설치본·수정 후보를 실제 CWMeshReader/CModel과 해당 effect ResourceStaging까지
연결해 대조한다. 단일 삼각형의 GPU culling 검사를 포탈 전체 로드·표시 성공으로 기록하지
않는다. source 복원과 PROJECT_TUNED 수정, 설치 및 사용자 화면 판정을 구분한다.
이 누락으로 이전 파생 파일이 제품 로더에서 거부된 회귀와 수정 증거는
[쿠크 포탈 결과](09-22/2026-09-22_KOUKU_PORTAL_LIGHTING_RESULT.md)에 기록한다.

### 자막 Box Detail은 최종 화면 소비자까지 연결한다

occurrence의 offset/scale이 저장돼도 active row와 typed subtitle view가 전달하지 않거나
MainApp이 고정 값을 사용하면 Preview에는 반영되지 않는다. Kouku SUBTITLE은 높이1080 기준
화면 X/Y pixel offset(+Y 아래)과 Scale X의 균일 글자 배율을 같은 preview clock으로 전달한다.
Preview/Apply/Save, 종료·재시작·잘못된 입력의 이전 값 보존을 함께 검사한다. 글자 높이 요청과
최종 UILabelFont의 폭 맞춤·baked font snapping을 구분하고 실제 설치 폰트로 확인한다.

### Server Play deadline과 늦은 exact lifecycle

bounded raw FIFO를 사용하는 Client의 wall-clock deadline은 Server 거절 증거가 아니다.
느린 resource 준비나 main pump 뒤 exact receipt/lifecycle이 다음 batch에 남을 수 있다.
deadline에는 지연 안내를 표시하고 요청 소유권은 실제 typed 응답, 명시 Stop 또는 session
종료까지 보존한다. 요청 ID/scope/revision/epoch 검사는 유지한다. 승인 전 Stop은 epoch가
도착한 뒤 같은 run에 전달한다. ACTIVE부터 COMPLETED까지 같은 batch에서 처리한 경우
완료 뒤 presentation을 다시 시작하지 않는지 검사한다. 개별 검증은
[쿠크 lifecycle 결과](09-22/2026-09-22_KOUKU_PATTERN_LATE_LIFECYCLE_RESULT.md)에 둔다.


### 세이튼 카드와 망치 접촉 재생

- Local Preview의 연출 성공은 플레이어 피격 성공이 아니다. 접촉 넉백·영구 추적 카드가 있는 Pattern은 기존 Server audition으로 재생해 Play Pattern과 같은 판정을 사용한다. 짧은 Trigger의 birth는 박스 끝으로 누락시키지 않으며 자연 Pattern 종료와 room-owned projectile 종료를 구분한다.
- 피해 ENTER_AREA의 bone을 Client만 읽고 publisher가 무시하면 실제 망치와 서버 충돌이 수 m 어긋난다. OBJECT_CONTACT의 기존 WModel 본 bake를 피해/overlap에도 연결하고, 실제 타격 시각의 본 위치와 MAP 경고 footprint를 대조한다. 자동 원형의 추정 중심을 본 부착 성공으로 기록하지 않는다.
- ballistic 비행 높이는 gravity와 비행 시간에 의해 정해진다. 242ms는 정점이 약0.072m이므로 눈에 보이는 launch를 원하면 해당 Result의 시간만 조정한다. 기존 다른 패턴이 공유하는 Result까지 함께 바꾸지 않는다.
- 원본 카드 출력 sound는 action의 실제 반복 stage에 존재할 수 있다. 소리가 없는 다른 stage를 반복해 만든 Pattern은 원본 importer가 sound를 자동 복제하지 않는다. 저장 Trigger별 Sound Box와 원본 event를 연결한다.
- 회색 native mesh는 texture 누락뿐 아니라 원본 MRT의 diffuse/normal/specular가 forward carrier에서 소비되는지 확인한다. RT0 ambient만 번역된 native2893에는 원본 출력과 같은 basis의 직접광을 연결하며, RGB 배수만으로 원본 복원을 주장하지 않는다. 저작 intensity와 alpha를 보존하고 원본 shadow/SH 미복원·사용자 화면 검증 경계를 분리한다.

- **유령 native84 표시 검증은 alpha/coverage와 실제 RGB를 나눠야 한다.** 조명 없이 검은 반투명 mesh도 clear와 다른 픽셀을 만들므로 changed-pixel 성공은 표시 성공이 아니다. actual catalog model+donor+scene profile로 RGB 양수/finite를 확인하고, 별도로 CValtan dormant/GHOST_HIDDEN/body-window/cinematic-suppression 및 BLEND pass10의 HRESULT를 확인한다. local Workbench는 저장된 authoringPhase를 part 선택에 연결하고 교체 뒤 CModel/target generation을 함께 갱신한다. [검증과 남은 화면 경계](09-22/2026-09-22_VALTAN_EDITOR_VISIBILITY_RESULT.md).


### Bone Clips와 순차 timeline 소유권

- live 본 자세는 Set key 전까지 문서가 아니다. 미확정 TRS가 있을 때 본·클립·시간 변경이나 Undo가 Read_Key를 호출해 값을 지우지 않도록 선택을 잠그고 Set/Cancel/명시적 Discard만 허용한다. baked clip 길이 축소는 duration만 줄이지 않고 모든 본 key를 재샘플링해 범위와 시작·끝 자세를 함께 보존한다.
- 골격 overlay의 화면 클릭을 ImGui Render에서 뒤늦게 claim하면 이미 처리된 gameplay 입력을 막지 못한다. 실제 early input owner와 연결하지 않은 overlay는 표시/hover로 제한하고 본 선택은 panel에서 한다. bone combined matrix의 preScale과 actual BoneRoot도 한 번만 합성한다.
- 본 편집기의 Stop은 자신이 manual pose를 소유할 때만 pause한다. 비활성 본 패널이 매 frame 다른 Sequencer의 CModel 재생을 정지시키면 안 된다.
- authored animation은 native WModel section의 39-byte name 제한 대신 별도 stable ID를 쓰며 native clip과 index를 저장 ID로 공유하지 않는다. exact skeletonHash/bone/source clip DAG와 finite quaternion·시간 범위를 모두 검증한 후 전체 채널을 교체한다.
- 발탄 Bone Clips source는 prototype이 직접 읽지 않는다. PublishV2가 검증한 optional Published bone 문서와 generation closure를 사용한다. source 변경은 publisher CAS read set에 포함하고 실패 시 이전 Product를 보존한다.
- 발탄 순차 clip의 앞 trim은 독립 시작 offset이 아니다. sourceStart를 옮기고 길이를 줄여 후속 clip을 당기며 서버 stage 시간은 유지한다. preview donor는 BossCatalog의 Cinematic donor와 일치해야 entrance/phase2/finale/trash 4 clip이 빠지지 않는다.


### 2026-09-22 native sincos와 실제 sampling 비교

DXBC `sincos`는 두 destination을 쓰기 전에 source를 한 번 읽는다. `sincos r0.x, r1.x, r0.x`를 sine/cosine 순차 대입하면 cosine이 변경된 r0.x를 읽는다. generator는 sourceAngle 지역 상수를 먼저 만든다. 신규 Guardian/Sea program만 검토한 source 명령에서 재생성하고 기존 무관 program을 일괄 교체하지 않는다. 단색 1×1 shader 비교는 UV·sampler 오류를 증명하지 못하므로 화면 sampling 복원에는 공간적으로 변하는 texture, 실제 camera 행렬·깊이, 유색 출력과 non-finite 검사를 사용한다. 모션 블러 108조건 결과는 09-22 AncientSea/Guardian RESULT에 있으며 유령 발탄 전투 증상과 원인을 혼동하지 않는다.


### Live TrailGhost 대상과 첫 샘플

원본 TrailGhost의 emission duration이 sampling interval보다 짧을 수 있다. 누적 간격만 기다리면 source notify가 정상이어도 한 장도 나오지 않으므로 첫 실제 owner pose를 별도로 기록하고 child lifetime으로 종료한다. 모델 이름이 같아도 고정 갑옷 clone으로 현재 outfit을 대체하지 않는다. own/shared bone palette, 실제 class weapon socket, hidden mask와 only-local flag를 typed owner callback에서 읽는다. generic mount preview가 선택돼 있을 때 scene Character로 fallback하지 않는다. source enum NONE과 원본 rim/fade shader 의미가 미복구라면 PROJECT_AUTHORED projection이라고 구분한다. 실제 Guardian socket은 b_wp_1이며 없는 본의 identity 반환을 렌더 성공으로 기록하지 않는다.


- 탈것 lifetime FX는 스킬 종료와 소유 기간이 다르다. ambient MOUNT_END / spawn NATURAL을 main-thread 준비 큐와 같은 Character의 vehicle handle owner에 연결하고 mount commit 실패·동일 snapshot·해제·교체 경계를 확인한다.
- CModel clone이 CMaterial을 공유하는지 실물 두 clone과 prototype으로 확인한다. source constant/texture override 및 clear는 GPU texture/geometry 공유를 유지하며 material 상태만 copy-on-write로 분리해야 한다. 이름·행 수 검사만으로 타 인스턴스 보존을 증명하지 않는다.
- PostProcessChain의 material Opacity와 EFPPMESkillValue가 구동하는 engine opacity는 별개다. 원본 CB0 prefix와 native texture expression을 검증하고, 원본 재질 파라미터를 action fade로 덮어쓰지 않는다. scene brightness도 복원된 원래 light에서 계산해야 반복 누적 곱을 피한다.

### 후처리 material의 S_FALSE는 draw 생략이다

native screen-post가 퇴화한 camera projection을 S_FALSE로 격리하면 renderer도 그리기·장면 복사·ping-pong target 진행을 생략해야 한다. FAILED만 검사하면 Begin하지 않은 이전 shader state로 그릴 수 있다. 정상/생략/정상 및 생략-only를 실제 render method·CShader·VB로 검사하고 draw primitive 수, 최종 target, RGB 보존을 함께 확인한다. [통합 검증](09-22/2026-09-22_ANCIENT_SEA_GUARDIAN_INTEGRATION_RESULT.md).

### WModel 기본 재질 의존성과 source lifetime 0

- native material override의 JSON texture closure가 완전해도 CModel은 WModel의 기본 diffuse/normal을 먼저 로드한다. 실제 decoder의 material path와 native register texture를 합쳐 설치·SHA256을 검사하고, 전체 source material section의 CModel 생성/variant/원복을 실행한다. JSON parse 성공으로 실제 모델 로드 성공을 대신하지 않는다.
- 원본 Cascade lifetime 0은 임의 1ms 수명으로 바꾸지 않는다. 해당 원본 carrier의 normalized age 0을 유지하되 생성한 notify occurrence가 끝나면 입자를 정리한다. 다른 긴 cue가 같은 문서를 계속 살려도 원래 occurrence의 입자가 남지 않는지 실제 clip·bone으로 검증한다.


### Effect owner 제어는 저장 경로와 취소 원복까지 확인한다

- `ownerControls`를 codec에만 추가하면 component split/compile 또는 catalog assembly에서 사라질 수 있다. Effect 문서 → assembly → 문서 왕복과 최종 owner consumer를 함께 확인한다. control-only 문서는 가짜 particle을 넣지 않고 control 마지막 key까지 수명을 계산한다. v15는 빈 경우에도 정식 runtimeExtensions 객체가 필요하다.
- 재질·visibility 제어는 effect occurrence token과 action-start identity에 귀속한다. effect가 살아 있어도 action이 교체되면 이전 token을 해제한다. hide/reset/실패/owner 변경/소멸 경로도 자기 token을 제거하고 남은 제어를 재평가한다. 원복은 제어 전 실제 native constants 및 기존 stance/장비 visibility를 보존해야 하며 전체 override 초기화로 대체하지 않는다. 저장만 성공하거나 finite 값만 나온 검사를 실제 취소 원복의 증거로 사용하지 않는다.
- 범용 소비 계약은 [렌더링·이펙트 복원 V2의 OwnerControls](렌더링이펙트복원V2.md), 개별 검증은 [Guardian 결과](09-22/2026-09-22_GUARDIANKNIGHT_NATIVE_EFFECT_RESTORATION_RESULT.md)를 따른다.


### 원본 재질 packing이 커질 때 Debug 스택과 부분 색 변경

원본 native family를 generated Configure 한 함수에 계속 추가하면 Debug의 모든 분기 임시값이 같은 스택 프레임에 잡힐 수 있다. Shader 비교나 작은 최적화 probe 통과로 실제 Character 초기화 성공을 대신하지 않는다. 기본 1MB 스택의 Product 객체에서 실제 catalog/Character 소비를 검사하며, 계산식은 유지한 family별 call frame으로 분리한다. /STACK 증가로 생성기의 구조적 문제를 숨기지 않는다.

TransColor/BuffColor는 현재 native program의 실제 direct packing 및 후속 copy에서 생성한 register 연결만 바꾼다. catalog에 없는 이름을 추측하거나 다른 상수를 다시 채우지 않는다. AUTO 또는 source parameter가 없는 program은 변경하지 않는다. 기존 튜닝값·peer/prototype·중첩 owner 및 취소 원복을 실제 CModel로 검사한다. 상세 근거는 [Guardian owner control 결과](09-22/2026-09-22_GUARDIAN_OWNER_CONTROL_RESULT.md)를 따른다.


### Navigation 영역 목록과 게시 파일은 함께 전달

`.navregions`에 새 REGION을 추가하면 같은 게시 단위의 `<Area>.<Region>.navgrid`, `.navpolicy`, `.navblockers`와 Server의 `.navsurface`도 Git 전달 대상인지 확인한다. 목록만 추적하고 신규 출력이 빠지면 Server는 해당 영역을 방문하기 전에도 world 초기화에서 실패한다. Product의 기본 파일 존재 검사만으로 세부 영역 준비 완료를 판단하지 않는다. Product는 게시 목록 참조, grid header/정확한 byte 길이와 필수 sidecar header를 읽기 전용으로 확인하고 실패 경로를 runtimeDataChecks에 남긴다. 좌표·정책은 소비자와 같은 float32로 파싱해 큰 소수 원점의 roundtrip을 오탐하지 않는다. 이 검사는 셀별 높이·층 겹침·world admission이나 원본 bake를 대체하지 않는다. 누락은 해당 Area의 공식 publisher로 복구하고 생성물을 임의 작성하거나 목록에서 영역을 제거해 숨기지 않는다.


### 클래스 미리보기의 파츠 입장과 재질 소비자

stance 전용 IDENTITY 파츠는 교체 가능한 의상 slot을 소유하지 않아 END를 사용할 수 있다.
현재 허용 조합은 GuardianKnight의 IDENTITY + END + GUARDIANKNIGHT_DRAGON이다.
Model View admission과 장비 preview는 같은 검사를 사용하고 일반 장비의 END를 허용하는
식으로 검사를 완화하지 않는다.

generic preview의 CPart_Body와 Product boss의 CBody_Valtan은 같은 재질을 다른 경로로
그릴 수 있다. 실제 preview asset의 part type부터 확인한다. 유령 발탄 native84는 두 경로
모두 BLEND/pass10을 사용한다. pass0의 ordered alpha coverage나 Arena 조명 probe만으로
Character Select 표시를 검증했다고 기록하지 않는다.

### 돌의 마스크 유지 구간과 생성 간격

native dissolve는 Dynamic.X의 clamp와 실제 DDS·mesh UV를 함께 검사한다. 유지값을
clamp 상한보다 올려도 이미 잘리는 부분은 복구되지 않는다. 해당 occurrence의 scalar
교정은 authoringOverrides에 원본 compilerValue를 보존하며, 다른 재질이나 파편에 일괄
적용하지 않는다. 실제 UV별 clip 검사와 수명 종료의 draw 제거를 나눠 기록한다.

기존 고정 거리 생성기에 sourceRecipe를 붙일 때 source rate/burst 분기가 거리 생성을
가로채지 않는지 검사한다. 허용된 portable mesh carrier의 명시 간격만 기존 생성기를
사용하고 source 위치·이동 module과 섞지 않는다. 1.2배 외형 변경은 mesh 크기에만 적용하고
birth center·간격·Server cover와 폭발 시점은 유지한다.

### Ctrl 핑의 물리 클릭과 UI 소유권

Ctrl 핑은 물리 좌클릭을 사용하며 mouse-button swap과 독립이다. raw edge는 UI·focus·capture 분기 전부터 관찰하되, 실제 소비 시에는 전역 차단뿐 아니라 버튼별 filtered LB와 UIInputRouter의 같은 프레임 claim도 확인한다. 소비한 press는 release까지 이동·평타·MAZE LMB·ground-target confirm에서 제외한다. Ctrl+Z/X/C의 typed Esther 명령은 유지한다. 과녁과 핑의 native source texture 및 로컬 표시 검증은 [Clown·MAZE 결과](09-22/2026-09-22_CLOWN_MAZE_MARKERS_RESULT.md)를 따른다.

### 비활성 Effect 문서의 삭제와 preview 준비

CPU-only Open 문서의 Element 삭제·편집은 선택 모델·source bone·GPU 준비를 요구하지 않는다. 활성 preview만 stage 후 commit하며 실패하면 문서·선택·필터를 보존한다. 마지막 Solo Family 삭제 뒤 남은 문서가 있으면 COMPLETE로 조정하고, 실패 시 기존 family도 복원한다. 실제 코드 재현 범위는 [삭제 결과](09-22/2026-09-22_EFFECT_ELEMENT_DELETE_RESULT.md)에 기록한다.

### 화염파동 바닥과 수동 그룹 식별

발광만 남은 바닥은 bloom부터 바꾸지 말고 실제 source ground carrier가 문서에 있는지 확인한다. WandDecal 착지 섬광은 FireWave의 지면 고정 화염과 별개다. Element groupId가 있어도 Effect Tool 수동 그룹은 manual. 접두어를 요구하므로 독립 위치 편집을 의도한 파생 문서에서 이를 명시한다. 원본 disabled notify, 기존 사용자 occurrence offset·수명과 독립 저작 파생의 추가를 구분한다.

### 신규 SourceCharacter Light program의 입력 ABI

MN_PPPP_00 선물상자 native109는 Base와 Light가 다른 varying 배치를 쓴다. shader 함수와 material row만 추가하면 `MakeSourceCharacterInput`의 Light 2/3/5/6 분기에서 빠져 UV·조명 방향을 상수로 읽을 수 있다. 새 program은 원본 Base/Light DXBC 선언과 실제 input builder 양쪽을 대조하고 필요한 program만 해당 분기에 등록한다. 기존 program의 ABI를 통째로 바꾸지 않는다. Engine/Client mirror와 실제 FxCompile wrapper도 함께 확인한다.

### 원본 버프 수명·Beam2 carrier·백스텝 잔상

Source leaf 존재와 finite 성공만으로 제품 복원을 판단하지 않는다. 속박은 Server bound 상태의 실제 소비자·해제·사망·Reset까지 연결한다. 원본 particle lifetime이 30초를 넘을 수 있으므로 source recipe는 finite 120초, 수동 particle은 30초를 허용하며 UI와 Codec 범위, 기존 particle capacity를 함께 유지한다. Required EmitterLoops 생략값0을1로 가정하면 지속 방패가 중간에 꺼진다. 원본 반복을 복원하고 기존 loopEffectToDuration의 소유자 window로 종료한다.

Action occurrence를 source leaf로 보강할 때 material/runtimeCarrier만 복사하지 않는다. 원본 TypeData에 맞는 kind·rendererShape·Detail.Trail도 대조한다. Beam2를 particle/sprite로 남긴 채 carrier만 연결하면 drawable admission이 실패한다. native3008의 별 선 sprite 지원은 동일 원본 재질 ABI에 한정하며 기존 ribbon 경로를 바꾸지 않는다.

TrailGhost의 원본5ms는 float에서 .004999999888이므로 decoder 경계에 최소 float 오차만 허용한다. 실제 설치 모델·preScale·본·socket TRS로 검증하고, 흰 반투명 appearance의 PROJECT_AUTHORED 경계와 사용자 화면 판정을 분리한다. FX_Buff_01을 actor-ground translation으로 대응한 것은 실제 본 부착 검증이 아니다. 자세한 증거는 [주사위·무력화·백스텝 결과](09-22/2026-09-22_KOUKU_DICE_STAGGER_BACKSTEP_RESULT.md)를 따른다.


### EventReceiver와 LocationDirect의 위치 소유권

EventReceiverSpawn은 일반적으로 source event 위치를 상속하지만, 활성 LocationDirect가 emitter-local 절대 위치를 지정하면 같은 부모 높이를 다시 더해서는 안 된다. `Spawn_Particles`의 prepared recipe에 활성 `LOCATION_DIRECT`가 있는 경우에만 event origin을 0으로 시작하고, 일반 event receiver의 위치·속도 상속은 유지한다. 비활성 module 이름이나 문서 전체에 LocationDirect가 있다는 이유로 모든 event origin을 제거하지 않는다.

원본 낙하가 사라진 경우에는 기존 nested RawDistribution/CDO 절차를 먼저 적용한다. ScaleFactor 누락을 복구한 뒤 event 부모·자식 각각의 실제 world 위치도 비교해야 두 결함을 구분할 수 있다. 카드비에서는 원본 CDO 상속 복구와 자식의 중복 7m 제거를 별도로 검증했다. 동일 문서의 일반 입자와 LocationDirect가 없는 controlled event-receiver fixture도 이전 playback과 대조했다. 이 CPU 수치 성공을 GPU 발광·실제 화면 성공으로 기록하지 않는다. [카드비·DJ 결과](09-22/2026-09-22_KOUKU_CARDRAIN_DJ_EFFECT_RESULT.md)를 따른다.

랜덤 투하의 개별 사운드는 보스 Pattern의 고정 SOUND 한 행으로 대신하지 않는다. 원본 Projectile timer를 확인해 각 CombatObject birth 기준의 유한 MAP SOUND로 같은 targeted template에 포함한다. MAP EFFECT가 위치 원점을 소유하고 SoundCue는 기존 재생/seek/종료 경로를 사용한다. 같은 템플릿의 모든 투하가 한 변형만 고르지 않도록 targeted session identity도 variant 선택에 포함하며, 일반 Pattern의 기존 선택은 보존한다. 원본 카드비의 사운드1350ms와 카드 시작1500ms·충돌1650ms가 다르므로 임의로 같은 시점에 맞추지 않는다.

고정 플레이어 대상 공에도 같은 위치·birth clock의 SOUND가 필요하면 기존 SHOWTIME fixed
그룹의 소유자를 검사해 MAP EFFECT+MAP SOUND만 허용한다. SOUND-only·tracking Sound·일반
선택 그룹·Collider 혼합까지 넓히지 않는다. 사용자가 이미 편집한 Action 사운드는 원본 notify
시각이나 반복 개수와 다르다는 이유로 자동 보충하지 않는다. 보충할 Pattern을 명시적으로
제한하고 나머지는 기존 sound 행 전체가 같은지 확인한다.

파생 후보의 freshness는 후보 JSON과 manifest 해시만으로 닫히지 않는다. 원본 leaf TRS,
source recipe·재질·모델 등 실제 입력도 생성 시점 해시로 고정해 설치 직전에 확인한다.
baseline은 후보를 계산할 때 읽은 동일 bytes여야 한다. 계산 뒤 최신 파일을 다시 읽어 baseline
해시로 삼으면 동시 저장을 승인한 것처럼 잘못 처리해 새 편집을 덮어쓸 수 있다.

### Pattern Flow의 마지막 항목과 반복 소유권

순서가 저장되고 게시됐다는 사실만으로 순환 반복을 완료 처리하지 않는다. 전투 전용 Client Play_Flow와 Sequence 포함 Server RaidFlow의 마지막 완료 처리를 각각 확인한다. optional `patternFlows.loopStartEntryId`는 vector index가 아니라 같은 Flow의 stable entry ID다. 최초 전체 진행 뒤 마지막 wait를 지키고 지정 entry부터 반복하며, reorder 뒤에도 ID로 찾는다. 저장/게시/Server catalog는 잘못되거나 누락된 기점을 거부하며 첫 entry로 임의 대체하지 않는다. Tool에서 기점을 삭제하면 설정도 지운다. 미지정 시 Client는 GATE1, Server는 GATE1·BINGO만 기존 처음 반복을 유지한다. Server에서 index 0으로 돌아오는 명시적 기점도 continueRaid 조건에 포함하여 기존 epoch·게시 revision·잔여 row 소유권을 유지한다. 최초 epoch 0 입장과 반복 입장을 구분하고 Stop·거절·중단·관문 완료는 기존 종료 경로를 보존한다. 두 번째 주기 및 다른 관문 회귀를 검사한다.

Client codec의 문서 Load 성공은 모든 Pattern admission 성공을 뜻하지 않는다. malformed
Pattern을 quarantine하고 나머지 문서를 보존할 수 있으므로 Play Pattern 검사는 각 행의
quarantine 사유와 Expand 결과까지 확인한다. Stage optional 필드를 Python에 추가했다면
C++ struct·허용 property·strict 값 검증·Parse/Serialize·실제 Preview도 함께 연결한다.
`retargetTarget`의 RANDOM_ALIVE/NEAREST_ALIVE와 retargetOnEnter 종속성을 일치시키며,
부모 clip의 중간 파편에서 retargetOnEnter를 끄면 selector도 제거한다. 사용자 저장 필드를
지워서 admission 오류를 숨기지 않는다.


### Monster 공격 1회와 Full Restore preview identity

MonsterBrain의 공격 접촉은 공용 Apply_WorldToPlayer가 HP·damage event·hit reaction을 소유한다. 호출 전에 HP를 직접 차감하거나 같은 damage event를 추가하면 한 collider가 두 번 피해를 준다. 공격 push/down을 없앨 때는 대상 MonsterProfiles의 attackPushRangeM/attackPushMs/attackKnockdown/attackDownMs만 바꾸고 boss reaction 및 몬스터 자신의 hitKnockbackScale을 섞지 않는다.

Full Restore COMBO preview는 전체 Effect tree의 enrichment cache만으로 stage를 판단하지 않는다. 선택한 실제 모델의 skillbindings·animevents에서 effect ID와 clip window가 일치하는 유일한 stage를 조회한다. target 선택은 catalog reload를 할 수 있으므로 그 이전의 skill 포인터를 유지하지 않는다. 시작 실패뿐 아니라 재생 중 실패도 현재 Effect 목록에 이유를 남긴다.

ALT V source camera와 model actor를 따로 복원할 때 위치뿐 아니라 forward/up의 좌표 basis도 비교한다. 원본 줌아웃 키가 이미 있는데 camera만 다른 축이면 새 키를 추측해서 추가하지 않는다. 배경을 대체하는 실제 opaque carrier에 sceneBackdrop을 연결하고, 해당 활성 window만 기존 배경 숨김을 적용한다. finite camera 성공과 실제 구도 성공은 별개다. 개별 수치는 09-22 Guardian 재생·카메라 RESULT를 따른다.


### Interaction 준비 지연과 pending commit 사이의 취소

준비가 늦은 첫 Clown/MAZE action을 최초 snapshot 한 번만 검사하면 이펙트가 빠진다. 동일 Server action 안에서 현재 age와 실제 clip/playRate·Server lock·Effect 수명을 확인하며 재시도하고, queue admission 성공 후 중복 제출을 막는다. queue 이후 같은 프레임에 취소·사망·class/form 교체가 들어올 수 있으므로 opted-in weak PendingAdmission을 실제 commit에서도 확인한다. old Character의 외부 참조가 남아 있는 경우까지 고려하며 이미 active인 E의 자연 꼬리를 Stop_Owner로 함께 지우지 않는다. 기본 nullopt 호출자는 기존 동작을 유지한다. 09-22 Gate1 복원 RESULT의 후속 검증을 따른다.

### World prop의 상태 이름과 sustained source 오라

`On/GoOff/Off` 이름으로 색이나 지속 여부를 추측하지 않는다. 원본 DeployData→Prop DB→LookInfo→ParticleSystem의 상태 참조를 찾고 ColorOverLife·EmitterLoops·lifetime을 대조한다. Gate3 진입오라는 On이 녹색 지속, GoOff가 파랑 지속이고 Off는 종료1회다. 원본 source-loop는 zero lifetime/loop0 계약과 실제 owner-sustained 소비자까지 함께 검사한다. trigger bounds는 실제 WModel 정점×modelPreScale×StartSize 및 occurrence 회전으로 계산한다. source code/window 성공을 실제 GPU 표시 성공으로 기록하지 않는다. [Gate3 결과](09-22/2026-09-22_GATE3_WORLD_AURA_IMPLEMENTATION_RESULT.md).


### 원본 sprite의 carrier guard와 engine cbuffer prefix

Codec의 sourceRecipe 허용이나 CPU 입자 수만으로 복원 성공을 판단하지 않는다. 실제 선택된 shader의 function·dispatch·HasProfile 세 guard와 renderer carrier를 모두 대조한다. 동일 원본 PS를 ribbon과 sprite가 공유할 때는 검증한 material/PS/VF/VS 조합만 함께 허용하고 기존 carrier는 보존한다.

원본 material이 소유하지 않는 CB0 prefix를 항상 row0.X opacity로 가정하지 않는다. ParticleMacroUV는 원본 PS에 따라 center/scale 두 row 뒤 opacity가 별도 row에 있다. materialMap의 owned/unowned row와 실제 o0.w 명령으로 입력을 복원하고 최종 alpha를 강제하지 않는다. MacroUV는 실제 ParticleSystem occurrence 중심과 원본 world-space 반경을 사용한다. 같은 native를 공유하는 설치 문서 전체의 source literal·renderer predicate까지 연결한 뒤 실제 DDS pixel 출력과 opacity0 control을 검사한다.

source tail이 occurrence 끝에서 잘리는지는 실제 입자 종료 시점으로 판단한다. visual tail을 늘릴 때 stage/Logic 시계는 별도로 보존한다. 사용자 저장 window는 보수적인 document 상한으로 자동 확대하지 않는다. Tool의 실제 모델 잔상은 미리보기 외부시계로 pause/rewind/stop을 처리하고 Product 기본 시계를 보존한다. 개별 근거는09-22 Flame/Dice/Stagger/Backstep 결과에 둔다.

### Native carrier와 shader 입력 연결을 따로 확인한다

- 새 native ID가 설치돼도 Trail/Decal pixel dispatch 상한이 옛 범위면 generic 재질로
  빠진다. Native의 빈 generic resources가 흰 mask를 소비하면 큰 사각형이 생긴다.
  C++ validation, HLSL dispatch와 installer range를 함께 갱신한다.
- Masked LocalVF를 CB0[0].x opacity 계약으로 통일하지 않는다. 정확한 PS/VS와
  unowned prefix를 확인하면 CB0[0]에 particle RGBA가 필요한 permutation이 있다.
- Beamtrail UV0.zw는 source VS 출력과 runtime uv1 소비를 대조한다. tangentView가
  필요한 PS에는 실제 strip world/UV frame을 공급한다. 전역 alpha 보정을 하지 않는다.
- Identity-root CPU에서 bone-follow가 비었다는 사실은 Tool이 bone을 공급하지 않는다는
  증거가 아니다. 설치 WModel의 actual pose와 Tool anchor/time history를 별도 확인한다.

### IdentityParts 종료 정책과 skeletal actor의 중첩 payload

원본 IdentityParts의 MakeParts=false와 bExecuteNotifyEnd=true를 구간 전체 hide로 옮기지 않는다.
원본 실행 시점과 installed IDENTITY 파츠의 stance baseline을 함께 확인하고, 저작 playback의 구간
표시는 기존 owner token의 저장·복원 경로에서만 추가한다. preview의 requiredStance도 실제 편집용
CCharacter에 적용하고 Stop 때 복원하며 Server stance를 대신 변경하지 않는다.

source skeletal actor의 자식 material/particle payload에는 부모 actor와 비슷한 필드가 포함될 수 있다.
부모의 enclosing tail과 reflection으로 확인한 LoopCount/StartAnimTime/StartAnimTimeUseOnlyFirst를
구분하고 실제 설치 WModel의 본을 사용한다. 지원 bool의 0/1 차이로 전체 actor를 버리거나 nested
particle transform을 actor transform으로 쓰지 않는다. 신규 source native material의 color 선택이
완료돼도 별도 selected_distortion_programs 원본 pass의 생성·설치 여부를 함께 확인한다.

### 개별·그룹 회전과 실제 고정축 면

Element rotation이 shader 좌표에 전달돼도 fixed-axis sprite의 최종 quad가 같은 emitter basis를 소비하는지 확인한다. 명시적 회전 편집은 공통 pivot helper와 기존 followEmitterAxisRotation을 함께 사용하며, camera/velocity billboard의 정책은 유지한다. 단일 선택을 unrelated animated sibling이 막지 않게 하되 서로 다른 부모 공간의 raw 위치 평균을 Group center로 쓰지 않는다. camera-relative offset은 pivot 회전 기대값에 포함하지 않는다. paused preview는 새 문서·birth history를 현재 cursor까지 재생성하는지도 확인한다. 실제 팡파레·fog와 source mesh 검증은09-22 EFFECT_ROTATION_PIVOT_IMPLEMENTATION_RESULT를 따른다.

### 컷신 패턴 연결은 실제 Stage 입력과 최종 카메라 pose까지 검사한다

Map Tool actor clip과 기존 패턴 occurrence를 먼저 대조한다. 이름이나 source player 선택문이
존재하는 것만으로 연결 성공을 판단하지 않는다. 선택 조건에 사용하는 Stage ID가 실제 Server
snapshot index에서 채워지는지 확인한다. source 배우/FX/WAV 수명은 카메라의 마지막 cut과
다를 수 있으므로 카메라 반환이 source 정리 조건이 되어서는 안 된다. invocation-local camera
age에는 offset을 더해 action age를 복원한다. camera smoothing에서 snapshot age로 전환할 때
같은 occurrence의 시간이 역행하면 WAV까지 다시 seek될 수 있다. 기존 player elapsed로 경계를
보호하되 사용자 Local Timeline rewind는 허용한다. 절대월드 camera projection은 tracking/blend
적용 뒤에도 비교한다. BOSS_XZ가 남으면 raw key 검사가 통과해도 다른 구도가 된다.
09-22 `VALTAN_MAPTOOL_CUTSCENE_PATTERN_LINK_RESULT.md`의 G17을 따른다.

### Effect 모델 그림자의 native coverage와 카메라

skeletal ModelCue를 SHADOW group에 추가할 때 surface와 다른 animation clock이나 model clone을 만들지 않는다. 동일 evaluated frame의 pose/root/material track을 다시 사용한다. Shadow light의 View/Proj를 source material의 camera 입력으로 넘기면 view-dependent mask가 달라질 수 있으므로 raster light 행렬과 실제 scene camera 행렬을 분리한다. generic CDO CastShadow=true는 notify별 native override를 증명하지 않으며 별도 projectile/decal의 존재와 시점을 함께 조사한다. optional castsShadow 누락은 false로 유지해 다른 Effect의 표시를 바꾸지 않는다.
# Flow 저장과 공용 Balance loader의 선택 스킬 필드

Boss Tool의 Save Flow는 BalanceTool이 소유한 canonical writer를 사용한다.
발탄 source-manifest가 통과해도 BalanceTool::Reload의 PlayerSkills 파싱 실패로
writer admission이 거부될 수 있다. `invalid skill definition`이면 publisher의
선택 필드와 UI의 strict 객체 검사를 함께 대조한다. rootMotionScale은 optional로
읽기/범위 검증/직렬화까지 보존하며 unknown-field 검사를 제거하거나 스킬 필드를
삭제하지 않는다. 미저장 Flow를 확보하기 전에 Load Flow/종료를 안내하지 않는다.
09-22 VALTAN_FLOW_SAVE_SKILL_COMPAT_RESULT에 실제 복구와 빌드 증거를 기록했다.

HLSL의 invocation-local `static` 변수는 FX 외부 uniform이 아니다. `g_EffectSceneReadMode`처럼
shader 내부에서 초기화·전환하는 상태에 C++ Bind_RawValue를 추가하면 shader 컴파일이 성공해도
실제 FX reflection 바인딩이 실패한다. 제품 CSO로 CShader를 생성하고 base/cohort의 새 pass와
실제 uniform 바인딩을 함께 검증한다. FXC 실행 중 source가 바뀌면 완성 파일 timestamp만으로
최신 내용을 증명하지 말고 해당 입력 재컴파일과 새 심볼/실제 로더를 확인한다.

### PBR 간접광과 발광, 그림자의 거리 단위

RNM/IBL을 Emissive에 합쳐 저장하면 ambient SSAO와 직접광 shadow strength를 올려도 구운 바닥은
계속 밝다. PBR marker3만 기존 geometry RGB에 간접광을 분리하고 실제 emissive는 보존한다.
구운 정적 그림자에 shadow map을 그대로 한 번 더 곱하지 않는다. optional dynamicBakedStrength는
static cache보다 앞에 생긴 동적 caster만 판별하는 프로젝트 근사이며 cache 무효 시 비활성이다.
shadow depthBias는 정규화된 광원 깊이다. orthographic far-near를 곱한 실제 거리와 normalBias(m),
shadow 폭/해상도를 함께 확인한다. SSAO 반경과 FXAA 혼합을 RGB 색 보정으로 대신하지 않는다.
Deferred base와 모든 source cohort CSO를 함께 재컴파일한 뒤 실제 CShader 로더와 pixel 회귀로 확인한다.

공용 native family의 긴 else-if chain은 family별 lambda로 stack을 분리해도 MSVC C1061 중첩
한계를 넘을 수 있다. 유일한 nonzero program을 설정하는 첫 일치 계약을 확인해 guarded sibling
분기를 생성하고 마지막 unknown-family guard와 공통 validation/commit을 유지한다. header만
바꾸면 정본 installer와 named-vector/face reader의 분기 경계 탐색이 깨지므로 같이 수정한다.
135개 family의 본문 보존과 실제 실패 시 result 보존 검사는
[기본 의상 결과](09-22/2026-09-22_CHARACTER_EQUIPMENT_MATERIAL_RESTORE_RESULT.md)의 G09에 있다.

### Source trail 준비와 All Effects 편집의 실제 소비 경계

native animationTrail의 마지막 baked sample은 notify window보다 짧을 수 있다. document-owned
projection에서는 clamp=min(notify lifetime, sourceEnd)를 검증한다. Stage_Document만 직접 호출해
성공한 것은 제품 입장 준비 성공이 아니다. Create_DocumentOwnedRuntimeProjection부터 검사하고
일반 저작 trail의 duration equality, sourceRecipe.enabled 금지와 잘못된 clamp 거부는 유지한다.

preview clone은 Server snapshot이 없어도 presentation stance를 가진다. preview 종료 복원에
network stance getter를 쓰지 않으며 실제 scene Character는 편집 대상에서 제외한다. 다른 class의
animevents 실패로 전체 All Effects의 Product cues를 비우지 않는다. 실패 class만 보존하고 Saved
document 편집은 Product enrichment와 분리한다. `Document validation passed`는 오류가 아니다.

카메라 emitter의 source XY 화면 평면/-Z depth를 캐릭터 actor 전방 basis로 처리하면 near plane
뒤나 화면 아래로 내려갈 수 있다. 해당 camera occurrence의 socket과 fixed-axis 추종만 교정하고
원본 occurrence/time/scale는 보존한다. 원본 native camera 생성 코드를 확보하지 못한 adapter
보정은 원본 코드 복원과 구분한다. 확대 요청은 실제 세 클립의 bone/mesh 최소높이를 확인해
배율을 정하며, 과대한 임의 배율 때문에 생긴 지면 침범을 별도 가짜 본 보정곡선으로 덮지 않는다.

### SourceCharacter engine lookup의0 대체와 역수

원본 engine-owned BRDF sample이 미복구일 때0으로 반환하는 것만으로 안전하지 않다.
Guardian110/111의 e56633f Base는 lookup.y의 역수 후 곱셈 때문에0*Inf가 최종RGB NaN으로
전파됐다. shader identity와 원본 명령을 고정해 분모0만0기여로 처리하고 유효한lookup의 원본
DXBC 일치를 확인한다. NaN 제거를 원본 LUT 복원으로 기록하지 않는다. scene cube가 연결돼도
해당 shader의 color/rotation suffix CB rows가 실제 공급되는지 별도로 확인한다.

### 건슬링어·슬레이어 기본 의상 source 연결

CharacterCatalog에 본체/기본 장비 경로가 있어도 `modelMaterialOverrides`가 없으면 기존 단순 재질로
그려진다. 실제 WModel의 named slot과 원본 mesh의 Materials를 먼저 join한다. Slayer의
`pc_wr_f_face_mi`처럼 basename이 `PC_WR_F_FACE`와 `PC_WBK_F_00.mat_high`에 모두 있는 경우
실제 mesh의 package 참조를 사용한다. 머리·눈 native family를 연결할 때 원본 UV1/UV2도 필요하며,
같은 position/UV0의 hair seam은 원본 triangle corner로 구분한다. 임의 첫 vertex나 UV0 복제는 금지한다.

MIC native texture expression 수는 referenced texture table 수보다 클 수 있다. 여러 expression이
같은 source texture를 참조하기 때문이다. expression 개수로 table을 거절하지 말고 실제 fallback의
referencedTextureIndex 범위와 object identity를 검사한다. 기본 두 의상31재질의 설치·검증 경계는
`09-22/2026-09-22_CHARACTER_EQUIPMENT_MATERIAL_RESTORE_RESULT.md`를 따른다.

### Character Composition의 Product source와 저장 freshness

Character Product를 sequencer로 열 때 `.animevents`의 새 bytes만 baseline으로 잡고 이전에 cache한
cue를 stage하면 다음 Save가 다른 도구의 새 cue를 지운다. baseline으로 읽은 동일 bytes를 parse하여
stage하고, 기존 writer가 `Load_Events`로 다시 읽은 뒤에도 expected baseline을 비교한다. 저장된
`.effectsequence`는 이전 Product의 배치일 수 있으므로 Product 선택에 자동 overlay하지 않는다.
별도 배치 복원은 명시적인 Load Effect Sequence로 구분한다.

`.animevents`는 source clip별 정본이다. 한 action이 같은 clip을 반복할 때 occurrence 한쪽의 cue만
추가·삭제하면 runtime에서 모든 occurrence가 바뀐다. 겹치는 source window의 cue signature multiset을
비교하여 누락·추가·TRS·policy 차이까지 거절하고, 같은 cue 편집을 적용하거나 별도 authored clip을 쓴다.
서로 다른 source 범위는 그대로 보존한다. binding/cue의 여러 파일 저장 실패는 자기 변경만 baseline
조건으로 rollback하며 다른 writer의 최신 저장본을 덮어쓰지 않는다.

Composition의 cue 삭제·교체 범위를 previous+proposed binding 합집합으로 잡지 않는다. Animation 교체·삭제나
trim 축소로 빠진 원래 clip/window의 cue는 다른 스킬에서도 소비할 수 있다. proposed source window만 갱신하고
제거된 구간은 보존한다. 이전 binding은 rollback을 위해 유지하는 값이다.

interaction의 explicit `effectCues.endMs`를 재열기 시 native clip 길이로 다시 합성하면 Save 후 길이가
늘어난다. 원래 end가 없었던 기존 cue에만 duration을 보충하고, 유효한 CUE_END source end는 보존한다.

### 쿠크 복합 투사체의 수명과 표시 이름

공의 Action notify만 연결해 후속 Projectile과 death/timer 폭발을 빠뜨리지 않는다. 세 공의
receiver event 이름과 particle-system occurrence를 분리하고 마지막 위치를 event가 전달하는지
실제 Playback으로 확인한다. source 속도·높이는 원본 값이지만 저작 목적지는 프로젝트 값이다.
Effect 수명이 clip 뒤까지 이어지면 Pattern duration을 보충하며 애니메이션 속도를 늘리지 않는다.

리소스 표시 이름 `yellow-gaze`만으로 노란 경고라고 판단하지 않는다. source eye_01/eye_02,
원본 색과 설치된 양안 bone basis를 확인한다. 파란 구체도 G/U variant와 alpha·SceneColor를
대조한다. WModel의 raw cm 정점에 particle World만 적용한 크기는100배 오류가 날 수 있으므로
CModel modelPreScale를 포함한다. synthetic anchor의 성공은 실제 모델 부착 성공이 아니다.

나팔의 원형·방사 방향은 비슷한 asset 이름이나 등각 분할로 추정하지 않는다. Action의
직접 notify와 SkillDecal/SkillEffect 방향을 대조한다. compact CEFParticleData의 검증된
transform+40 int32 FRotator를 +28 float 벡터로 읽으면 회전이 모두0이 될 수 있다.
disabled notify도 제외해야 한다. 원본 occurrence 회전과 leaf의 MeshRotation turn·TypeData
pre-rotation을 구분하고 기존 좌표 변환을 한 번만 적용한다.

### Alt V 중앙 큐브와 submesh 내부 skin island

`altv.source.notify036.cube` 한 cue에는25개의 cube가 들어 있다. 한 submesh/하나의 material
바인딩을 중앙 표면 한 개로 해석하면 주변에 캡처 이미지가 반복된다. 실제 mesh의 skin
indices/weights를 세고 중앙 `b_cube_1_02`를 이름으로 resolve한다. 전체 출력색이나 alpha를
마스킹하면 주변 aura/edge까지 사라지므로 frozen Color/Bloom의 capture RGB항만 한정한다.

공용 static/skinned shader에 uniform을 추가하면 base와 모든 SourceGroup의 FX 변수 계약을 함께
확인한다. FXC 실행 중 source가 바뀌면 이전 입력으로 만든 CSO의 수정 시각이 source보다
늦을 수 있다. mtime만으로 최신이라고 판단하지 않고 입력·출력 기록과 실제 FX reflection을
대조한다. 파일 존재·개별 FX 생성과 일반 shader closure PASS도 전체 base/variant ABI 일치를
보증하지 않는다. 실제 제품 CShader의 전체 group 생성을 검사하고, 실행 중 compiler의 대상이
아닌 것으로 확인한 stale 생성물만 백업 후 재생성한다. Loader의 Effect already-cancelled는
앞선 맵 shader 실패의 후속 취소일 수 있으므로 최초 Shader stage/HRESULT부터 확인한다.

### 선택 장비 source 재질과 native UV

기본 캐릭터의 source material override는 EquipmentPresentationService의 단순 경로 로드에
자동 적용되지 않는다. 선택 장비도 ActorCatalog의 MODEL_ASSET_LOAD_DESC를 소비해야 한다.
머리·fur의 native PS가 추가 UV를 읽으면 원본 GPU-skin VS와 WModel submesh별 채널을 함께
검사한다. 원본 position/UV0가 같은 seam은 normal·triangle corner로 확정하며 같은 geometry의
`_old` mesh는 LookInfo가 가리킨 export로 구분한다. source에 없는 채널을 복사로 만들어 내지
않는다. program7의 두 Base/Light two-tone 상수가0인 경우만 추가 UV 없이 정확한 원본 색이
가능하며, 이후 그 분기를 활성화하는 변경은 같은 geometry에서 거절한다.


### Character Select 간접광과 장면 전체 색 진단

원본 PBR RNM 항을 albedo×lightmap만으로 축약하면 `(1-metallic)`과
`(1-reflectionBRDF)` 에너지 분배가 빠진다. 원본 PS 수식과 같은 입력으로 비교하고,
미연결 SH/hemisphere 계수는 이 확인된 수식 누락과 구분한다. 맵은 SH 곱을 생략하고
일부 native character는0 상수를 소비하므로 같은 환경 큐브를 바인딩해도 간접광은 같지 않다.

전체 장면의 황색 편향은 material debug만으로 확정하지 않는다. 최종 SceneHDR, tone 뒤,
grading 뒤를 같은 카메라에서 비교한다. 중립 LUT·감마 검사 통과는 조명/표면 입력까지
원본 동등함을 뜻하지 않는다. Benchmark의 수치는 실제 바인딩/설정이며 GPU 픽셀 샘플은
아니다. per-instance RNM 값이 없는 CPU snapshot을 RNM 미연결로 오판하지 않는다.


### 보이지 않는 환경 효과와 광원을 줄일 때의 범위 계약

이펙트 중심점만 frustum 밖이라고 simulation을 중단하지 않는다. 최종 camera와 전체 sprite
분포·수명·이동·크기를 감싸는 bound가 필요하다. 현재 정적 SOURCE_LOOP bound는 stationary
root와 rigid camera를 전제로 하며 scaled/sheared camera, 미지원 source module, mesh/trail/light/
screen/control/외부 anchor는 기존 경로를 유지한다. source size는 spawn 순서와 update 순서를
각각 포함해야 하고, 재질 PS 분기 수만으로 sprite VS의 WPO 존재를 추정하지 않는다.

LEVEL_ACTIVE 환경 표현의 offscreen pause는 입자·RNG 보존과 visual clock 정지다. 숨은 wall time을
몰아서 replay하거나 연속 재생과 같은 위상이라고 설명하지 않는다. 기존 draw distance의 Stop/
respawn은 별도다. externally sampled 이동 표식과 Server trigger·combat 시간은 계속 기존 계약을
사용한다. explicit render submission을 켠 대상은 최종 camera 경계에서 정확히 한 번만 제출한다.

화면 밖 POINT/SPOT도 영향 sphere가 화면 안 receiver를 비추면 남긴다. deferred 제출에서 생략한
광원을 원본 transient 목록이나 forward consumer에서 지우지 않는다. shadow caster에는 camera
culling 결과를 전파하지 않는다. 실제 적용과 검사는
[공통 성능 결과](09-22/2026-09-22_BERN_RELEASE_PROFILER_OPTIMIZATION_RESULT.md)의 G07을 따른다.

### 클래스 선택의 검은 팔각 바닥과 소개 씬

SL00의11개 floor/star 쌍은 정지 캐릭터 미리보기 자리이며 원본 diffuse brightness0은 정상 입력이다.
밝은 중앙 무대 장식이나 class 소개 배경으로 잘못 해석해 일괄 복원하지 않는다. 사용자가 이동한
배치와 원본 좌표를 구분한다. GuardianDragonHuman의 짧은 customization zoom과
SCENE01 ChangeClass9의 Matinee75→74/SL10 소개 연출은 다른 경로다.

검정 diffuse만으로 검정 무대를 보장하지 않는다. 현재 PBR은 diffuseBrightness 적용 뒤
lightbox 반사를 기본색에 더하고 직접광·환경광 specular를 별도로 계산한다. 선택창 임시
floor/star는 사용자 검정 표시 요청에 따라 기존 material variant의 diffuseBrightness,
reflectionIntensity, specularPBRIntensity를 함께0으로 둔다. source catalog나 원래11쌍의
재질을 덮어쓰지 않는다. 셰이더 계산 근거와 실제 화면 확인은 구분한다.

카메라 반복과 particle 재시작도 같은 동작이 아니다. source infinite emitter와 finite burst의
Lifetime0을 구분하고, 입자의 원본 나이와 owner 종료 창을 유지한다. 같은 이름의 skill FX보다
해당 PSC의 실제 template·redirector를 먼저 대조한다. 조사·후보·설치·화면 검증의 경계는
[가디언 선택 연출 결과](09-22/2026-09-22_GUARDIAN_CLASS_SELECTION_CINEMATIC_IMPLEMENTATION_RESULT.md)를 따른다.

### 클래스 선택 연출의 연속 재생과 도구 소유권

- Movie Play의 검증 대상은 현재 선택한 바닥 Category의 class다. 특정 Server class나 Guardian
  선택을 공통 전제로 두지 않는다. F1과 WORLD는 Level의 같은 선택·명령을 사용하며 Seek·Stop은
  실제 active class를 대상으로 한다. 카테고리 변경만으로 다른 영화나 Server class를 대신 선택하지 않는다.
- cinematic PSC의 source age와 phase source time은 다르다. 움직이는 root의 history는
  age→intro/loop phase를 역조회하고 held-age의 현재 root·parameter는 별도로 최종 frame에 적용한다.
  scalar/vector ParticleParameter는 raw Matinee curve를 먼저 보간한 뒤 DPM mapping을 적용한다.
  phase curve를 particle-relative-life distribution.keys에 넣거나 shared definition의 기본값을
  매 frame 바꾸지 않는다. PSC별 variant와 모든 phase의 typed 입력을 함께 검증한다.
  Effect codec의 bSourceContract는 native-v14 계약을 뜻하며 일반 source recipe 존재와 다르다.
  movie format13의 새 typed 입력은 reader·문서 validation·writer 전체에서 검사해야 한다.
  ordinary sourceRecipe도 공통 portable admission을 소비하므로 v15 runtimeExtensions 분기만
  검사하고 제외하지 않는다. 실제 EffectObject를 생성해 마지막 검증 gate까지 확인한다.
- scene별 `backgroundAreaId`는 Loader와 Level이 같은 full map scope로 소비한다. 동일 Area의
  prototype 중복 등록을 피하고, 이전 방문의 Map load cache를 준비 시작 시 무효화한다. optional
  배경 실패를 공통 presentation 실패로 승격하거나 level 전체 prototype을 지우지 않는다.
  WorldSequence의 공통 SL00 준비는 한 번 유지하고 활성 scene 배경만 표시한다.
- PSC InstanceParameters는 실제 `ParamType`의 Scalar/Vector 필드만 읽는다. 같은 struct의 비활성 union 값을 색으로 사용하지 않는다. 초기 typed 값과 Matinee setter를 함께 투영하고 이름이 같아도 distribution 타입이 다르면 원본 fallback을 유지한다.
- 배우의 mesh material slot과 component override, `ParentAnimComponent`, 숨김 part 및 SkelControlGroup을 원본 기준으로 추적한다. VS가 COLOR/UV를 출력한다는 사실만으로 필요 채널을 결정하지 않고 실제 Base/Light PS의 입력 사용까지 확인한다. 부모 포즈 복구와 별도 의상·머리카락 물리 재현은 다른 완료 항목이다.
- native Effect는 base와 distortion companion을 각각 컴파일한다. world projection의 CB 배열, C++ descriptor admission, shader dispatch, publisher, 공통 Decal/Trail 범위가 같은 program을 받아야 한다. 기존 함수 본문을 보존하는 append installer를 사용한다.
- OneLayer 재질은 이미 SceneColor를 합성한 결과를 반환할 수 있다. 원본 blend와 기존 OneLayer consumer를 함께 연결해 같은 SceneColor를 additive로 다시 더하지 않는다.
- 모델 Create/Clone 통과와 실제 Map object stage, cinematic Initialize/Play는 다른 경계다.
  rollback 상태에는 실패 단계·asset ID·source placement를 남긴다. headless native 검사는 Loader와
  동일한 COM 초기화가 필요하며, 창 없는 재생 성공은 GPU draw와 사용자 화면 판정을 대신하지 않는다.

- 원본 SL00의 11개 floor/star를 따로 옮겼다면 한 개 공통 offset으로 카메라를 옮기지 않는다.
  소개 연출 SL10의 원본 WORLD 좌표는 별도이며, 무대를 이동할 때에는 배우·배경·camera·FX·light
  전체에 같은 source/destination frame 변환을 적용해야 한다.
- intro에서 loop로 한 번 전환한 뒤 WorldSequence player와 PSC handle을 재사용한다.
  particle source age는 camera loop clock과 별도로 누적한다. Lifetime0 burst의 종료 horizon을
  현재 끝점과 같게 두면 float 반올림으로 장시간 반복 후 사라질 수 있으므로 다음 구간까지 연장한다.
- Workbench가 정리할 재생은 Level 포인터만으로 구분하지 않는다. 외부 Play/Seek마다 새 token을
  발급하고 내부 loop는 유지한다. slider float 끝점은 double duration을 초과할 수 있어 clamp한다.
  customization·전환·연결 상태의 재생 조건은 Level과 Workbench가 동일한 gate를 소비한다.

- WModel의 Python read/finite 검사만으로 설치 가능한 animation donor라고 판단하지 않는다.
  Engine WANM은 clip당 총 TRS key 1,000,000개를 제한한다. 긴 컷신을 모든 본에60Hz로 bake하면
  정상 정지 포즈도 이 상한을 넘을 수 있다. 같은 packed-float 값의 연속 구간 내부 키만 제거하고
  구간 양끝·시간·bone palette를 보존한 뒤 실제 CModel Create/Attach/Clone으로 admission한다.

- native map RNM program을 늘릴 때 MapAssetCatalog/publisher와 CModel override 검증만 바꾸지
  않는다. CMaterial::Initialize의 supportsBaked와 Bind_SourceCharacterInputs 소비자까지 같은
  program 범위로 맞추고, 실제 Loader의 CModel/Create_MaterialVariant로 설치 모델을 검사한다.
  catalog/placement parse 및 캐릭터 donor 검증 성공은 배경 모델 admission을 보증하지 않는다.

- WorldSequence의 Area 준비에서 optional Deploy 두 문서를 무조건 Load_Default하지 않는다.
  두 파일 모두 없으면 빈 대상, 한쪽 누락·손상은 실패이며 실제 DEPLOY_PLACEMENT binding은
  대상 검증을 유지한다. 정상 아레나 입장과 모델 admission 성공은 연출 Initialize/Play 성공의
  증거가 아니다. 준비 실패 이유를 Level에서 보존하여 Workbench에 전달한다.

### V2 ScreenPost PNG의 존재와 실제 decoder를 구분한다

V2 `Acquire_Texture`는 DDS와 WIC(PNG 등)를 확장자로 분기하고 두 경로에 같은 FORCE/IGNORE_SRGB 정책을 적용한다. 정상 PNG를 DDS 전용 loader에 전달하면 파일 존재 검사를 통과해도 prewarm은 실패한다. 확장자를 DDS로 바꾸거나 원본 PNG를 변환해 로더 누락을 숨기지 않는다. 설치 파일의 실제 decode·SRV 형식·ScreenPost alpha coverage admission까지 확인하며 JSON/CPU 재생 성공으로 대체하지 않는다. 구체적인 수정과 검증은 [카드비·DJ 결과](09-22/2026-09-22_KOUKU_CARDRAIN_DJ_EFFECT_RESULT.md)를 따른다.

### 병렬 native material ID 발급 뒤 병합

충돌 없는 숫자처럼 보여도 양쪽 branch가 같은 native program ID를 다른 material/PS/layout에 발급했는지 base와 양 parent를 비교한다. 다른 의미라면 한 cohort에 미사용 ID를 배정하고 authored runtimeShaderProfileId, C++ table, group HLSLI 함수, dispatch, selected switch를 함께 옮긴다. 같은 숫자의 main 효과까지 전역 치환하지 않으며 기존 material 식·texture·TRS와 지원 group 범위를 유지한다. PR449의 Guardian/Esther 충돌과 protocol104 결합 검증은 [병합 결과](09-22/2026-09-22_PR449_MAIN_MERGE_RESULT.md)에 기록한다.

### 쿠크 반복 Parent와 원본 이름·좌표의 독립 검증

표시 이름을 원본 action/clip identity로 간주하지 않는다. 레이저에 블랙홀 이름이 붙어 있어도
실제 source action·animation·notify와 설치 모델의 clip을 대조한 뒤 복원한다. 보스 기준 광선과
고정 월드 구체는 서로 다른 occurrence anchor를 쓰며, 중앙과 보스 시작 위치도 구분한다.
Saydon의 +X 전방은 world +Z heading과 90도 차이가 있다. 전체 effect asset을 돌리지 않고
실제 teleport/face-center 소비자와 해당 occurrence만 보정한다.

긴 Parent를 64-stage 배열에 펼치거나 유한 보드 시간을 반복 시작하는 것으로 순차 반복을
대신하지 않는다. 실제 자식 완료·카운터 후속·tail 뒤 다음 자식을 시작하고 Parent의 보드
소유권을 유지한다. 원래 대기 slot을 보존하도록 자식과 transition slot을 함께 삽입한다.
Mario 입장 Logic 시간을 바꾸면 연결된 collider occurrence 시간도 함께 맞춘다.
Release 검증은 Debug 전용 test body가 생략되지 않았는지 먼저 확인하고 pending mechanic
trigger commit까지 실제 tick 순서로 실행한다.


### 쿠크 생성 좌표의 정밀도와 리소스 없는 worktree

쿠크 본 콜라이더·오브젝트 갈고리의 계산 위치는 raw finite/bounds 검증 후 소수점9자리로
저장한다. Object의 refine/reduce는 raw 값을 사용하며 마지막 출력 위치·grip만 정규화한다.
저작값·scale·yaw·시간과 actual byte freshness 검사는 유지한다. 극히 작은 부동소수점 끝자리
차이를 원본 변경이나 패턴 손실로 오인해 전체 검증을 끄지 않는다.
Resources 없는 worktree에서는 모델 의존 패턴이 unavailable로 격리돼2개만 남을 수 있다.
Git clean 여부와 물리 Resources 준비는 다르므로 후보 patternInventory의 unavailableReason을
확인하고 의도하지 않은 패턴 감소를 그대로 게시하지 않는다.
통합 후 최신 저장본으로 projector와 Gameplay publisher를 다시 실행하며, 이전 main 기준
검증용 생성물로 진행 중인 쿠크 저작·게시 데이터를 덮어쓰지 않는다. 근거와 인계는
09-23/2026-09-23_KOUKU_PUBLISH_PRECISION_RESULT.md와 같은 주제 HANDOFF에 둔다.

### Release 쿠크 전체 준비와 추가 Resources 전달

새 Effect가 로컬의 기존 이미지·모델을 재사용한다는 사실만으로 배포 추가분을0개라고 판단하지
않는다. 이전 배포 Data와 현재 Pattern/Sequence/V1/V2/World의 참조를 비교하고 실제 전달 폴더의
상대 경로·SHA256, WModel 재질의 하위 texture까지 대조한다. 추가팩에 없는 오래된 기본 파일과
새 경로·갱신 파일을 구분한다. mtime만으로 파일 내용 변경이나 상대 PC 설치 상태를 확정하지 않는다.
Release 쿠크는 후반 관문 연출도 입장 전에 준비하므로 후반 Effect 누락으로 첫 입장이 실패할 수 있다.
Lobby의 `Server entry failed`는 로딩 복구에도 표시된다. 원격 상세 로그 없이 publisher hash 차이나
특정 파일을 첫 실패 원인으로 단정하지 않는다. 관련 범위와 보완 목록은09-23 Resources 감사에 둔다.


### Collider의 시작·종료 geometry와 실제 Server 소비

- Collider의 end position/size는 문서 필드 추가만으로 움직이지 않는다. Workbench의
  preview 복사·동등성·그룹 이동, 실제 preview 시간 평가, projector의 region motion과
  Server 높이·tick 사이 sweep을 같은 변경에서 연결한다. CYLINDER와 기존 평면 CIRCLE을
  구분하고 정적 저작본의 기본값을 보존한다.
- 반복 접촉은 Collider가 연결한 ENTER_AREA의 플레이어별 이력으로 처리한다. 일정 간격은
  창 끝에서 추가 타격하지 않으며, 기존 Logic 선택만으로 그 반복 정책을 덮지 않는다.
- 포물선과 낙사 허용은 별도 값이다. 상승 높이·수명으로 호를 만들고 낙사 비허용은 지상
  footprint의 navigation·collision을 유지한다. Gate1은 단독 Pattern과 Raid 모두 확인한다.
- BOSS에서 Follow를 끈 Collider는 생성 틱의 위치·방향을 Server도 고정해야 한다.
  평범한 overlap뿐 아니라 Mario entry 같은 특수 소비자도 새 anchor를 처리하는지 확인한다.
- 근거: [Collider 상세 결과](09-23/2026-09-23_KOUKU_COLLIDER_DETAIL_RESULT.md).

- Collider 상세의 Start/Lifetime·transform·anchor·종료값은 상단 설정 선택과 무관하게 유지한다.
  preset 항목 추가 시 label/shape만 추가하지 말고 실제 열거 개수도 확인한다. 비슷한 다른
  반복문을 전역 치환하지 않는다. 동일 시각 복제는 연결 Logic도 새 ID로 복제하고, 화면에서는
  복제된 Collider만 선택해 혼합 선택으로 인해 상세 편집이 사라지지 않게 한다.

### 유령 발탄의 coverage와 캐릭터 선택의 diffuse 환경 입력

- CBody_Valtan의 부활과 CPart_Body 미리보기만 pass16으로 고치면 마지막 컷신은 여전히
  CWorldSequenceObject의 pass10을 사용할 수 있다. native84 세 소비자의 NONLIGHT/pass16을
  함께 확인하고, 실제 source-preview.finale의 actor 모델·material까지 join한다.
- native84 유령 발탄은 원본 opacity0/discard, 공통 dither, forward alpha blend를 각각 확인한다.
  사용자 요청의 opaque 경로는 native84 전용이며 일반 발탄과 다른 반투명 재질에 전파하지 않는다.
  불투명 forward는 sorted BLEND 앞에서 depth-write하고 shadow silhouette도 같은 정책을 쓴다.
- animated FX에 base 전용 pass를 추가할 때 variant에는 UNAVAILABLE 정책을 선언한다.
  모든 파일에 BASE 정책을 넣으면 직접 shard 검사는 통과해도 전체 CShader variant admission이
  실패한다. 최종 제품 base FX로 전체 cohort 등록과 해당 pass의 dispatch를 확인한다.
- 하늘 mesh가 보인다는 사실과 cube의 specular/diffuse 기여를 구분한다. 캐릭터 선택은 원본
  확인된 정적 LUT owner가 비어 있고 원본 FLOOR12도 albedo를 saturate하므로 LUT 부재·clamp 자체를 오류로 단정하지 않는다.
- cubeDiffuse는 원본 cooked cube의 E/pi 적분 근사다. source SH9→native packed7 복원으로
  기록하지 않는다. 후속 native owner 복원과 구분하고 원본 RNM의 sky 기여 중복 가능성을
  남기고 단독 보기/gain0 및 before-restoration 프로필로 비교한다. GPU 수치와 사용자 화면 승인은 구분한다.
- 중성 albedo fixture에서 sky의 B>R이 나와도 실제 갈색 재질에 곱한 결과가 중립이라는 뜻은
  아니다. 실제 DDS와 material 값을 검증한다. 한 정적 owner의 LUT가 비어 있다는 사실을
  전체 camera/UI/volume 후처리 체인의 neutral 확정으로 확대하지 않는다.


### Collider의 Effect 기준 프레임과 임시 Server 재생

- 지연 생성 Collider가 fixed Effect를 따라야 하면 각 Collider 생성 시 boss 위치를 잡지 않는다.
  같은 Effect occurrence의 시작을 참조하고 Server/Client 모두 같은30Hz 시작 틱의 basis를 고정한다.
- 미저장 draft는 source revision이 같아도 내용이 다르다. 요청 sequence·gameplay rows SHA·승인 epoch로
  실행을 구분하고 시각 표현만 바뀐 동일 SHA도 새 실행에서 다시 읽는다. 이전 COMPLETED 응답을
  새 world의 승인으로 쓰지 않도록 world generation을 검사한다.
- 타임라인 Play/Play Preview/Play Bundle은 로컬 표현 시계, Play Pattern은 검증한 draft의 Server 실행을 소유한다.
  서버 Collider 검사 여부를 수평 밀림 거리로 제한하지 않는다. 수직 상승·피해 전용도 같은 Play Pattern 경로를 쓴다.
  서버 준비·재생 중 로컬 Play/Resume/scrub는 막고 Stop Pattern 뒤 전환한다. 서버 권위 시계를 ruler로 되감지 않는다.
- Publish 실패로 receipt가 rollback되면 변경 없는 Map도 다음에 cold 게시될 수 있다. 단계별
  action 시간과 fingerprint 시간을 분리하고, 없는 성공 receipt를 만들어 검사를 우회하지 않는다.

### 원본 PBR 간접광의 입력 owner와 이전 모드 보존

- 원본 SH9는 채널별48byte stride이며 native packer의7개 float4를 그대로 검증한다. cooked cube를 적분한 추가광과 원본 SH 색의 곱은 서로 다른 계산이다.
- 추가 hemisphere는 source SkyLight brightness만 보고 주입하지 않는다. 실제 primitive의 baked GUID와 lighting channel을 join한다. 이미 RNM에 구워진 하늘광은 다시 더하지 않는다.
- 재질별 cube override와 scene/view의 global fallback을 구분한다. 기존 연결이 없는 행에 현재 scene cube를 복사하는 것은 원본 선택의 증거가 아니다.
- sourceIndirect는 별도 cube/color/rotation/BRDF를 보존한다. native 모드를 끄면 이전 리소스와 식을 사용하며, 새 환경 연결의 legacyEnabled=false는 이전의 환경 없음 상태를 유지한다.
- native BRDF lookup과 color-grading LUT를 혼동하지 않는다. 실제 warm albedo와 원본 SH 곱이 덜 붉어져도 밝기는 낮아질 수 있다. 수식/owner 복원과 화면의 하얀색 복원 성공은 구분한다.
- 특정 native group의 전용 pass를 shared forward include에 추가하면 모든 static/animated cohort의 셰이더 컴파일이 늘어난다. 전용 animated carrier에서 compile-time group 분기를 사용하고 base/shard pass admission과 실제 GPU 출력을 함께 확인한다.


### 강제 이동의 보행 마스크와 물리 지지면 분리

- 보행용 walkable clamp를 피격 변위에 그대로 재사용하면 붕괴 구간에 도달하기 전에 멈춰 낙사를 차단한다. 실제 벽/몸체 sweep 결과와 물리 바닥의 지지 여부를 따로 평가한다.
- navigation 최대 단차 0은 무제한이다. 인접 셀에 낮은 배경 지형이 있으면 경로점 Y를 그대로 적용하지 말고 현재 높이에서 실제 이동 구간의 지지를 검증한다.
- surface=1, walkable=0 착지는 낙사가 아니어도 이후 보행 시작점 검사에서 끼일 수 있다. 강제 이동 수정에는 해당 착지에서 정상 보행으로 복귀하는 검증이 필요하다.
- 셀 절반 간격의 표본도 모서리를 짧게 통과하는 대각선 셀은 건너뛸 수 있다. 경계 전수 검증에는 셀 교차 추적이 필요하며 축 방향 테스트 성공을 전체 경계 성공으로 기록하지 않는다.

### 쿠크 낙사면과 연속 공중 피격의 기준 높이

- 강제 재피격의 새 포물선 launchY를 바닥 기준으로 사용하면 공중 연타마다 낙사면이 올라간다.
  개별 포물선 시작점과 연속 비행의 최초 지지 높이를 따로 유지한다.
- 아래 배경 nav면이 존재해도5m 낙사면보다 아래의 착지 후보로 사망 판정을 늦추지 않는다.
- 부활 위치는 사망 좌표 근처 투영이 아니라 현재 승인 관문의 기존 시작점에서 검증한다.
  후보 검증 전에 HP·위치를 바꾸지 않고, 성공 시 남은 비행 플래그를 함께 정리한다.

### 앵콜 clear UI와 낙사 변경을 통합할 때 (2026-09-24)

- Server의 G3 clear 대기와 Presentation Sequence의 lead-in은 서로 다른 시계다. 시간을 줄일 때 P10/P97, World/Camera, animation/sound source offset, 유리 source clock을 함께 이동하고, fake clear UI는 Debug 도구가 아닌 공통 Sample 경로에서 SceneHDR 후처리 전에 유지한다.
- 일시적인 CUILayoutRuntime는 숨김만으로 해제되지 않는다. Layer가 sprite를 소유하므로 Stop/실패/교체에서 기존 Release_Sprites를 호출한다. 선택적 scene UI의 font/draw 실패를 전체 world 프레임 실패로 전파하지 않는다.
- 일반1/3관문 낙사 금지와 Mario 내부 낙사는 적용 범위가 다르다. 실제 GATE3 상태의 Mario에서 중력→5m 낙사면→DEAD 복귀를 검사하고, timer deadline 한 번 호출을 물리 낙하 성공으로 간주하지 않는다.

### WORLD hook의 마지막 상승과 attachment 해제

- Baked grip이 있는 carrier는 마지막 XYZ 도착이 실제 하차점이라는 보장이 없다. 마지막
  연속 상승의 시작과 실제 Server navigation 바닥을 대조한 뒤 기존 attachment release를
  호출한다. 엄격한 상승 판정으로 같은 높이 수평 접근/대기를 해제점까지 되감지 않는다.
- Staggered bake의 sampling phase가 다르면 같은 clip도 local key 시간이 달라진다. 첫
  occurrence의 시간 하나를 전체 track의 기대값으로 복제하지 않고 실제 float32 소비값을
  확인한다. WORLD grip에 boss/root basis를 다시 적용하지 않는다.
- 반복 회귀 검증과 현재 수치는
  [09-24 hook 결과](09-24/2026-09-24_KOUKU_HOOK_ASCENT_RELEASE_IMPLEMENTATION_RESULT.md)에 둔다.

### 보행 이탈과 강제 이동의 바닥 판정 순서

- 보행용 바닥 이탈 helper를 넉백 시작에 무조건 호출하면 bounded push와 gate fence를
  건너뛰어 먼저 FALLING을commit할수있다. 명시적arena-exit의여부를먼저보존하고일반
  강제이동은기존swept collision/forced surface, bounded이동은기존clamp를사용한다.
- 실제게시source회귀는synthetic용빈catalog가아니라room이admit한Product generation을
  읽는다. source수치audit의PASS는native소비자가실제track을순회한PASS를대신하지않는다.

- Hook grip의XZ에물리바닥셀이없으면`Project_PointOnSameLevel`의기준Y도바닥근거가
  아니다. 기존player-spawn projection을fallback으로사용할때는동일navgrid/정확한
  walkable/수평거리/높이차를함께검증한다. 높이만0m나보스spawnY로대체하지않는다.


### Release F1의 서버 재생과 수치 profile

- Release F1을 열 때 ImGui 표시 guard만 제거하면 패턴이 시작되지 않는다. 같은 typed audition/flow의 Server 평가·fixed tick·lifecycle 송신과 Client preparation/응답 drain을 함께 연결한다. Map authoring이나 Debug 전용 로컬 preview의 guard까지 일괄 제거하지 않는다.
- Retail override가 있는 필드는 base 숫자만 저장해도 런타임이 바뀌지 않는다. 공용 Balance Test는 유효한 Retail 소유 field를 편집하고, 공식 Gameplay/World 및 조합 publisher는 기본 Retail을 유지한다. 저장 성공, publish 성공, 실행 중 Server 반영은 서로 다른 단계다.
- 공굴리기 counter window의 `endsPatternOnSuccess`만으로 무력화가 자동 삽입되지 않는다. occurrence의 성공 Logic → 같은 관문 groggy Pattern 결과 연결과 실제 published counter fixture를 함께 확인한다.


### Gameplay catalog의 새 profile lookup과 재로드

- 새 lookup map을 parser에 추가할 때 기존 Load rollback과 clear 대상도 함께 갱신한다. 첫 Load 성공만 확인하면 같은 객체의 두 번째 Load에서 duplicate buff나 이전 damage formula 잔존을 놓친다. profile별 lookup 두 개가 있으면 두 경로 모두 변경값을 소비하는지 확인한다.
- optional profile을 clear만 하고 rollback에서 빠뜨리면 malformed 후보가 이전 정상 profile을 지운다. 실제 published bootstrap의 반복 Load, 변경값 교체, 뒤쪽 invalid row 실패 후 이전 revision/값 보존까지 같은 검증에 둔다.
- 후속 fixture는 admission 실패 뒤 이전 catalog의 다른 종류 row를 예상 타입으로 접근하지 않는다. row 종류·필수 배열 개수를 확인한 뒤 front/back을 사용하여 첫 실패를 후속 assert가 가리지 않게 한다.


### 클래스 배치 변경 뒤 Debug OBJ의 헤더 의존성 누락

- Product compile/link PASS만으로 서로 다른 클래스 배치의 OBJ 혼합을 배제할 수 없다. 초기화 성공 직후 string/shared_ptr 접근 위반이 발생하면 정확한 EXE/PDB와 WER를 대조하고, 변경한 public header의 소비자 OBJ 시각 및 `CL.read.*.tlog`에 해당 헤더가 실제로 기록됐는지 확인한다.
- 강제 포함한 표준 라이브러리 PCH를 사용하는 TU라도 프로젝트 헤더가 PCH 안에 있다는 뜻은 아니다. 소스와 PCH만 기록된 불완전 tracking은 클래스 멤버 삭제·추가 때 재컴파일을 누락할 수 있다. 같은 설정의 정상 TU와 다른 configuration의 기록을 비교한 뒤 원인을 판단한다.
- 원인이 확인된 이전 OBJ만 보존·격리한 뒤 같은 toolchain의 정상 Product Build로 복구하고, 소비자별 헤더 추적과 현재 OBJ를 다시 확인한다. 출처를 확인하지 않은 전체 Clean/Rebuild나 tlog 삭제, 소스 timestamp 조작으로 정상 상태를 가장하지 않는다. CMainApp의 실제 사례와 실행/수동 확인 경계는 09-24 RELEASE_F1_RAID_TEST_RESULT의 Debug 종료 복구 기록을 따른다.


### 쿠크 전투 전환·공유 이펙트·피격 판정

- 연출 종료 프레임에 환경을 새로 붙이지 않는다. 준비한 gate 환경을 Sequence 시작 전에 baseline으로 잡고 성공 시 인계, 실패/세션 종료 시 이전 환경을 복원한다. clone prewarm 여부와 실제 프레임 시간은 별도로 검증한다.
- 피격 동작은 clip 종료나 지연 locomotion으로 Idle로 덮지 않는다. Server 공중 여부와 착지 edge를 소비하고 push-only 표시 변경이 기존 재접촉 grace까지 늘리지 않는지 확인한다.
- Resolve_CircleMove의 wasBlocked는 접촉 여부가 아니라 접선 이동도 실패했는지를 뜻한다. 접선 이동에 성공하면 false이므로 몸 내부 목적지 종료를 이것에만 묶으면 계속 몸 주위를 돈다. 실제 닿은 body와 목적지 내부를 함께 검사한다.
- Debug draw를 Release에서 제공할 때 Collider override, Component virtual, Bounding 파생형, Renderer/GameInstance 소비자까지 같은 ABI를 사용하고 EngineSDK→Client를 함께 Build한다.
- 투명도는 실제 occurrence가 참조하는 asset/carrier를 확인한다. MODEL 잔상의 brightness/alpha를 reflection으로 대신 조정하지 않는다. fixed-axis sprite의 본/owner 회전 누락은 해당 leaf의 followEmitterAxisRotation opt-in만 복원한다.
- 폭발 판정에 비행 입자·debris 전체 AABB를 쓰지 않는다. 폭발 source time과 중심, 바닥 mesh/ring을 분리해 측정한다. 같은 폭발의 중첩 primitive는 같은 Trigger창을 공유하여 중복 피해를 막는다.
- 한 보스 내의 칼날 count만 확인하면 Mario 전용 복제 template의 count를 놓친다. 실제 WORLD occurrence→instance→template을 따라가며 성공 종료는 자연 tail과 구분해 owner 단위로 정리한다.

- Parent child 중간 삽입은 stable occurrence ID 순서와 재생 시각 순서를 다르게 만든다. Product validation이 시간순이어도 bootstrap의 전체 natural-ID sort가 이를 뒤집을 수 있다. PATTERNPARENTCHILD는 parent 내부 startMs 순서를 사용하고 ID는 tie-breaker로만 쓴다. publisher 성공을 Server catalog admission 성공으로 대신하지 않는다.

## World Object 정리용 Parent와 Motion 부모 구분

- Object의 `motionInstanceIds`는 여러 Motion을 함께 재생하는 합성 리소스다. 목록 정리용
  Parent에 재사용하면 모델 binding과 재생 의미가 바뀐다. 조직 정보는 v3 `objectFolders`와
  Object `parentId`에만 저장하고 실제 모델·위치·개수·Motion ID는 보존한다.
- Ctrl 다중 선택 후 우클릭은 이미 선택된 행이면 집합을 유지한다. Shift 범위는 검색·펼침을
  반영한 현재 화면의 stable Object/Parent 행 순서로 계산하며 숨겨진 행을 섞지 않는다.
- 폴더와 Object를 함께 검증해 없는 부모·anchor 불일치·순환·64단계 초과를 막는다.
  hierarchy-only 검증은 모델 지정 전 draft를 허용하고 실제 Save는 기존 전체 검증을 유지한다.
- 선택한 상위와 자식을 일괄 이동할 때 최상위 선택만 바꿔 내부 소속을 보존한다.
  저장 codec/equality와 Map publisher를 함께 바꾸지 않으면 Save/readback이나 publish가 실패한다.
  검증·사용자 확인 상태는 `09-24/2026-09-24_WORLD_OBJECT_PARENT_SELECTION_RESULT.md`에 기록한다.

## 반투명 보행 바닥의 클릭 표면

- `Target_PickPos`는 불투명 MRT가 쓰므로 Alpha/BLEND 바닥이 화면에 보여도 그 아래 불투명
  geometry가 클릭될 수 있다. 유효한 depth hit가 있으면 현재 player Y 평면 fallback은 실행되지
  않는다. 올바른 nav bake와 잘못된 cursor XZ를 서로 구분한다.
- 이 target의 W는 frame 중 조명·그림자·decal의 기하 보조 값이기도 하다. 반투명 표면의
  XYZ만 deferred 중간에 덮지 않는다. 명시적인 PICKING 기여는 모든 frame consumer가 끝난
  뒤 실제 mesh/world/cull/depth로 target 하나만 no-clear 갱신한다. 다음 frame은 기존 MRT가
  초기화한다. 새 역할의 MRT를 Begin_MRT로 시작하면 원래 불투명 pick까지 clear될 수 있다.
- 늦은 pass에 현재 DSV를 그대로 쓰면 UI/Nav debug가 남긴 깊이로 바닥이 가려질 수 있다.
  불투명 렌더 직후의 depth를 별도 texture/DSV에 복사해 사용한다. 피킹의 depth write도 이
  복사본에만 남긴다. Final material debug view의 PickPos.W 소비까지 끝난 뒤 기록해야 한다.
- 카드미로는 기존 floor placement/asset의 정확한 쌍만 opt-in한다. alpha·반사·Nav·다른 투명
  소품은 바꾸지 않는다. 실제 화면 클릭·이동은 사용자 확인으로 남긴다.

## 공용 Sequencer clipboard와 owner 좌표 계약

- 복사 데이터는 분리된 저작 값 snapshot이다. runtime Effect/audio handle, mutable row pointer,
  vector index를 다른 tool로 넘기지 않는다. 전체 후보 검증 뒤 한 번에 commit하며 실패한 Copy는
  이전 clipboard를 보존한다. Resources 포커스에서 이전 timeline 선택을 복사하지 않는다.
- 같은 V1/V2 asset ID를 사용해도 좌표·clock·stop policy가 같다는 뜻은 아니다. 공통 kind는
  `V1_EFFECT/LEAF/GROUP`으로 통일한다. `follow=false`가 어떤 owner에서는 절대 MAP 좌표,
  다른 owner에서는 발생 시점의 Object 상대 좌표이므로 구분자 없이 서로 변환하지 않는다.
  현재 교차 owner import는 고정 snapshot을 거절하고 native 복사는 원래 정책을 보존한다.
  World의 inheritObjectRotation=false도 emission·placement 회전까지 제거하지 않으므로 WORLD
  rotation과 동일시하지 않는다. 교차 owner에서 표현 불가능하면 전체 후보를 거절한다.
- Object 전체 복제는 Motion/template/NEXT/default/binding ID를 함께 재발급해야 한다.
  model asset을 공유하는 것과 Motion 편집을 공유하는 것을 구분한다. 빈 Create Object 초안에
  붙여넣을 때에는 사용자가 정한 목적지 이름·ID·Parent를 유지한다.


## 쿠크 Duration·본 부착·직접 저작 이펙트 계약

- 재생 lifetime의 피해·광기는 하나의 반복 Duration과 연결된 Result로 저작한다. 같은 WORLD에 기존 자동 aura와 저작 Duration을 동시에 적용하지 않는다. 파괴 가능한 WORLD는 정확한 occurrence owner를 판정에 보존해 조기 파괴 시 결과 없이 종료한다.
- Composition occurrence는 해당 pattern의 next ordinal에서 정수 ID를 발급한다. 임의 문자열 suffix는 publisher가 거절한다.
- BONE collider는 local XYZ TRS를 실제 설치 모델의 전체 bone/socket basis와 합성한 뒤 마지막에 XZ center/yaw로 투영한다. 먼저 bone +Z를 평면화하면 수직인 mouth/b_root 축과 local rotation을 잃는다. Client와 Server bake의 순서를 같게 하고 offset/yaw를 이중 적용하지 않는다. 게시기와 Server의 BOSS_CURRENT track 검증도 sampled yaw를 허용해야 한다. identity yaw를 강제하지 않되 finite/normalized planar quaternion, visible, key unit scale, identity baseline과 exact contact clock 검증은 유지한다.
- ColorOverLife가 있는 carrier는 spawn alpha 수정만으로 최종 opacity를 판단하지 않는다. 최종 detail color multiplier와 움직이는 native Playback의 particle Color.w·birth history를 확인한다.
- 새 MODEL SourceCharacter program은 CModel 허용 범위, runtime shader group, Python group 표, Base/Light dispatch, CPU packing, exact Catalog override와 texture closure를 함께 연결한다. 기존 group 확장도 Client static/anim 및 Engine deferred shader의 정상 증분 컴파일이 필요하다.
- shield 흡수 이벤트는 HP 피해·DPS·stagger에 합산하지 않는다. 레이드 실패의 전원 전멸은 일반 HP 피해·개인 무적·shield·붙잡힘의 처리와 명시적으로 구분한다.
- 전체 수명의 본 track으로 bootstrap이 커지면 실제 track별 증가와 key 수를 먼저 측정한다. 제품 bootstrap의131,072행/64MiB와 선택 dependency closure만 보내는 Debug draft의16MiB는 별도 계약이다. 제품 전체 파일을 draft fixture에 넣어 전송 상한을 잘못 늘리지 않는다. Server·Client·publisher·Python의 bounded read, 정확한 행수·후행 행 거부를 함께 유지한다.
- 저작 Parent의 patternOccurrences가 Server ParentChildren로 그대로 남는다고 가정하지 않는다. 동일 actor의 병합 Parent는 하나의 fixed timeline과 Logic로 펼쳐지고 순차 Parent는 자식 ledger를 사용한다. 실제 게시 definition을 끝까지 읽어 두 형태를 구분하며, 제품 동작의 특수 Parent는 stable 참조로 연결한다. 실행 중 occurrence를 특수 Parent로 교체할 때는 현재 소유된 boss의 복사본에 Abort를 적용한 취소 후 상태를 먼저 admit하고, 성공한 뒤에만 live occurrence를 정리한다. 진행 중 live boss를 그대로 신규 admission에 넘기면 Busy로 거절되며, 실패를 피하려고 사전 검증 전에 원본을 취소해서도 안 된다. 테스트도 현재 저장 Parent와 자식 없는 독립 fixture를 혼합하지 않는다.

## 석재 색 차이의 원본 그림자·DDS 대조

- 파괴 바닥의 program7과 정적 석재를 구분한다. 동일 MIC/DDS라도 RNM이 없는 Deploy는
  source Base의 미연결 hemisphere/ambient와 함께 간접광이0일 수 있다. 현재 무베이크
  program7은 저장된 장면 방향광의 ambient를 한 번 소비하고, baked bit가 있으면 추가하지
  않는다. local/effect light에는 이 보정을 반복하지 않으며 direct shadow로 ambient를
  지우지 않는다. 이는 제품 장면 주변광 연결이고 원본 동적 LightEnvironment 복원은 아니다.
  MapTool의 descriptor 전달 수정만으로 제품 조명까지 고쳤다고 판정하지 않는다.
- MIC tint와 diffuse가 맞아도 component `ShadowMap2D` 및 placement atlas 좌표가 누락되면 직접광 색·밝기가 달라진다. 원형 바닥과 외곽처럼 같은 atlas를 쓰는 배치도 material의 `bakedLighting.staticShadow`와 placement의 `shadowCoordinateScale/Bias`를 각각 확인한다. 원본 참조가 확인된 연결만 복구하고 다른 재질을 같은 색으로 통일하거나 렌더 옵션으로 상쇄하지 않는다.
- `extract_source_map_component_lighting.py`의 `PARTIAL_UNSUPPORTED`는 shadow 없음 판정이 아니다. 현재 shadow record 지원 경계에서는 native prefix의 reference를 따라 원본 `ShadowMap2D` tagged properties의 texture·GUID·좌표를 확인한다.
- DDS top mip 대조는 legacy128/DX10 148바이트 header와 lower mip 추가를 분리한다. 고정128 offset 비교만으로 원본 압축 데이터 불일치를 선언하지 않는다. 발탄 적용 범위와 수치 근거는 [석재 RESULT G08](09-08/2026-09-08_VALTAN_ARENA_STONE_RESTORATION_RESULT.md#g08-2026-09-25-작은-원형-바닥-색-차이-재조사와-그림자-후보)에 있다.


### Class Selection 무비가 등록됐지만 Play가 실패할 때

- Level descriptor의 forward-declared member-function pointer ABI와 Loader/Registry 양쪽
  offset을 먼저 대조한다. 이 저장소의 callback은 일반 함수 포인터를 사용한다. 맵 파일 존재와
  Load 성공은 다르며 실제 CModel prototype, placement, movie Initialize/Play를 각각 검증한다.
- native background RNM cohort를 늘릴 때 Catalog/publisher/CModel/CMaterial admission과
  binding을 함께 확인한다. material 이름·실제 DDS/TGA magic·D3D texture 생성까지 추적한다.
  원본 TGA bytes를 `.dds` 이름으로 복사하면 hash 검사는 성공해도 실제 Play가 실패한다.
- source29byte FStaticNormalParameter, octal UTF-8 이름, 선택한 packer가 소비하지 않는
  LookInfo mask를 점검한다. 후보 리소스 생성·설치·실제 재생·사용자 화면 판정은 구분한다.
- reflected WORLD actor는 nonzero constant-sign scale과 실제 world determinant의 winding을
  같이 처리한다. MAP/DEPLOY binding의 기존 양수 계약에는 확장하지 않는다.
- movie clock4096key 상한은 source-time0.001ms 오차와 양끝 보존으로 admission한다.
  원본 LightColor의 cubic overshoot는 byte RGB 범위를 적용하고 일반 MIC vector는 보존한다.
- 반복/Stop/Seek/배속은 실제 다섯 클래스와 같은 F1/Workbench Play 함수로 검증한다.
  headless WARP의 모델·카메라·FX 수치 성공은 Client UI 클릭이나 최종 화면 성공을 뜻하지 않는다.

- movie 모델에 clip이 이미 내장돼 있으면 같은 WModel을 animationSet donor로 다시 붙이지 않는다.
  실제 WANM과 요청 clip을 대조한 자기 참조만 생략한다. 중복 clip 거부를 제거하지 않는다.
- native constant-only 재질은 exact program의 mask0 계약을 검사한다. 모델 전체에 임의 texture를 넣지 않는다.
- CDO/원본의 SubUVSelect는 X/Y tile 분포다. SubImageIndex scalar로 읽지 않는다. FreezeRotation은
  이동·수명을 멈추지 않는다. 실제 물리 접촉과 후속 tick으로 두 상태를 구분해 검사한다.
- MeshMaterial을 sourceMaterialSlots로 옮긴 뒤에는 codec와 실제 Stage가 같은 element 실행 판정을
  사용해야 한다. 원본 수신자 없는 이벤트는 local visual no-op이며 모듈 삭제나 dummy receiver로
  우회하지 않는다. 연결된 event cycle/queue 상한 검사는 그대로 유지한다.


### 쿠크 추적 접촉 종료와 특수 오브젝트 체력의 실제 입력

BOSS_TRACK_TARGET를 쓰는 공격 패턴도 플레이어와 접촉할 수 있다. 접촉을 모든 패턴의
완료로 바꾸면 공굴리기 카운터의 착지·피해·WORLD 종료 이전에 다음 Flow로 진행한다.
사용자가 지정한 플레이어 1초 추적 P104의 stable ID에만 접촉 종료를 허용한다. 같은
BOSS_TRACK_TARGET 사용이나 단독 창 구조를 근거로 P101·공굴리기 등 다른 패턴까지
확장하지 않는다. 다른 패턴은 접촉 중 이동만 멈추며 본래 종료 시계를 유지한다. P104 양성과
P101·공굴리기 음성을 각각 같은 접촉 위치로 검사한다.

카드미로 중앙 상자의 실제 체력은 MonsterProfiles 기본값뿐 아니라 활성 balanceprofile와
게시된 spawngroupsbootstrap까지 대조한다. 기본값500만 읽으면 Retail의587993 덮어쓰기를
놓친다. 요청값1000은 두 정본에서 일치시키고 실제 Server spawn·Q500 두 번과 LMB100을
확인한다. 생성된 bootstrap을 직접 편집하지 않는다.


### 팝업북·컷씬의 material binding 실패

animated WORLD가 정적 배경 material binder를 공유할 때 shader에 없는 foliage wind 입력을
비사용 reset까지 필수 바인딩하지 않는다. 실제 skinned/static CSO와 모델을 함께 검사하고
바람이 필요한 unsupported 입력의 거절은 유지한다. WORLD 실패로 Bundle 전체가 중지되면
그 이후 SOUND·카메라도 재생되지 않으므로 각각의 파일 누락이라고 먼저 판단하지 않는다.

### 선택한 플레이어 위치의 장판·투척과 독립 SOUND

Effect만 SELECT target에 연결하고 Collider를 기존 MAP 또는 BOSS 원점에 두면 화면과 실제
피해 위치가 분리된다. 선택 시점의 Server ground capture를 기존 combat object의 시각과
fixedHits가 함께 소비하게 하고 원래 hit 시각·반경·피해·밀림을 각각 대조한다. 같은 공격의
다른 관문·앵콜 복제본도 실제 Flow 사용처로 확인한다.

투척 compound를 target flight로 분리할 때는 기존 source transform·velocity·attachment의
고정 이동을 중복 적용하지 않는다. 비행 종료와 원본 particle tail을 구분하고, 착탄 시각의
오프셋·death event와 직접 burst의 중복 여부를 실제 Effect codec·Stage·Playback으로 확인한다.
새 Client header와 구형 OBJ archive를 섞은 격리 검사의 실패는 최신 Product OBJ로 원본과
후보를 함께 재검증하기 전까지 자산 결함으로 단정하지 않는다.

WORLD의 폭발 횟수를 줄여도 독립 Composition SOUND lane은 자동으로 줄지 않는다.
없어진 occurrence에 대응하는 SOUND ID·시각만 제거하고 남은 폭발의 소리는 유지한다.

### 전투 피격 강조의 실제 모델과 골격 경계

BOSS라는 network kind만 보고 CValtan을 호출하면 CNpc로 표시하는 쿠크 보스의 피격 표현이
빠진다. Server WORLD_OBJECT인 공·인형은 일반 WorldEntity snapshot에도 없으므로 기존
owned WORLD cue의 combat body NetEntityId로 DAMAGE_EVENT와 표시 모델을 연결한다.
별도 모델을 생성하거나 cue 문자열에서 entity ID를 추측하지 않는다. 배경 소품·큰 세이튼과
실제 공격 가능한 body를 구분하고 stop·owner 종료·late join의 수명을 함께 확인한다.

WModel raw vertex의 bind bounds에 preScale만 곱하면 원본 골격 basis의100배 확대를
빠뜨릴 수 있다. 호버·피킹에는 현재 inverseBind×combined palette와 같은 좌표계의 경계를
사용하고 actor root는 한 번만 적용한다. Reference bind bounds 의미를 바꾸지 않는다.
노란 Hit_Color rim과 SelectionColor RGB tint는 실제 바깥 외곽선 pass와 다르다.
공통 shader에 pass를 추가하면 base/source cohort의 pass index·ABI도 함께 빌드한다.
세부 구현·수치·사용자 화면 미확인은09-25 COMBAT_HIT_HOVER RESULT에 둔다.


### 클래스 무비의 실제 draw와 frame native material row 상한

WorldSequence/Effect의 Update·Seek·pose 수치가 성공해도 실제 World Object Render의 재질 바인딩은 실패할 수 있다. 클래스 선택 무비는 배경과 배우를 같은 프레임에 그린 뒤 render status와 intro→loop 수명을 함께 확인한다. Guardian 얼굴 native200과 DimensionMaster 무기 native902는 단독 바인딩은 성공했지만 기존 Engine frame registry의 257번째 재질에서 E_BOUNDS가 재현됐다. SourceCharacterRow는 R32G32B32A32_FLOAT Target_Depth.z에 저장하는 프레임 한정 정수이며 256개 ABI가 아니다. 임의 상한을 재도입하거나 실패 배우를 숨겨 재생 완료로 처리하지 않는다. 상세 구현·제품 반영 여부는 09-25 FOUR_CLASS_SELECTION_MOVIES RESULT를 따른다.

### Mario 저장 대상과 정상 기믹 실패의 수명

화면의 FXAA 비교 override와 저장 scene quality draft가 분리돼 있으면 Save/Publish 성공 뒤에도
매 프레임 override가 저장값을 덮어 보인다. checkbox의 실제 owner, 선택 Mario profile/region,
직렬화 문서, 게시본, 마지막 프레임 적용 순서를 함께 검사한다. 저장값이 false라는 사실만으로
사용자 조작 오류로 판단하지 않는다. profileId/regionId 3-way merge와 writer lease를 유지한다.

WORLD 기본 배치와 Pattern occurrence의 placement override는 서로 다른 소유자다. Action에서
연 WORLD 편집은 선택 pattern/occurrence stable ID를 보존해 실제 소비 override를 저장한다.
같은 object ID만 보고 기본값을 바꾸거나 다른 Pattern의 배치를 덮지 않는다.

Mario 입장자 부재는 저작된 전멸 결과를 적용할 정상 기믹 실패다. 이를 raid 실행 오류로
승격하면 cleanup이 보스와 Flow를 함께 제거한다. 현재 테스트 정책은 HP0을 유지한 채 다음
Flow와 반복을 계속하는 것이다. catalog/admission 오류는 별도 이유와 실패 수명으로 유지한다.

WORLD lane마다 전체 문서를 validate/copy하지 않는다. 선택 instance·group·NEXT·같은 resource의
전환 motion closure를 stage하고 실제 활성화 전에 clone pool을 준비한다. 부분 문서로 바꿀 때
span/endpoint 조회는 Set_Document 이후여야 한다. CPU subset 개선을 전체 화면 freeze 해결의
증거로 대신하지 않는다. 관문 전환의 실제 frame gap과 GPU 표시 결과는 별도로 확인한다.

광기와 같은 authored Result 수치는 source/Encounter만 확인하지 말고 최종 balanceprofile
overlay를 거친 Server bootstrap까지 확인한다. Retail의 전역0이 개별1/5%를 지우던 경로는
저작값 보존 sentinel -1로 구분한다. 명시0은 추가 광기가 없는 피해 verdict이며 최대HP피해0과
같은 validator 조건으로 묶지 않는다. shield/invulnerability 뒤 gauge만 오르지 않는지도 확인한다.

준비 단계에서 이미 읽은 presentation을 첫 combat bundle에서 archetype마다 재파싱하면
화면이 나오기 전 main thread가 멈춘다. exact source bytes·revision·캐시 존재와 canonical/draft
provenance가 모두 같을 때 검증 결과를 재사용한다. timestamp/size 일치만으로 승인하지 않고
retained run의 cache deep-copy도 피한다. 입력 변경 시 원래 stage/validate/commit을 유지한다.

### 원본 피격·호버·TrailGhost의 증거 경계

- monster Hit_Color는 원본 program별 Base 전용 row를 확인해 draw 사본만 변경한다.
  동일 번호를 Light에 쓰거나 shared CMaterial을 변경하면 다른 개체/조명에 상태가 샌다.
  native rim을 사용할 때 generic Fresnel 중복 가산을 끄고 다음 non-hit bind 복구를 확인한다.
- SelectionColor의 RGB tint, PPOutline 후처리, geometry extrusion은 같은 기능이 아니다.
  ColorOption의 적 빨강은 색 근거이며 width·RT·depth·blend의 근거를 대신하지 않는다.
  원본 DXBC texture swizzle을 그대로 읽고 sample destination 성분만 보고 채널을 추측하지 않는다.
- TrailGhost의 notify 발생 기간·생성 주기·child 수명·initial alpha hold·사용자 알파 배율은
  분리한다. 다음 발생의 설정으로 살아 있는 child를 바꾸지 않고 실제 관측한 pose만 보존한다.
  source field 복구와 native fade/shader 식 복구 여부도 별도로 기록한다.
- AKEvent가 있는 실제 피격 action/clip 시작과 Server damage 수신은 다르다. SoundSet의
  weapon/flesh slot을 식별하지 못한 채 모든 피해에 같은 음성을 붙이지 않는다.
  원본 HIRC의 reachable media 목록은 동시 재생이나 동일 확률 random 계약을 뜻하지 않는다.
  자세한 조사·구현·검증 범위는 09-25 COMBAT_HIT_HOVER_RESULT G09 이후를 따른다.

### 단일 Effect 그룹과 무관한 Collider Apply 실패

Apply의 singleton 정리는 해당 Pattern의 실제 Logic occurrence가 selectedEffectGroupId 또는
fixedSelectionGroupId로 참조하는 그룹을 보존해야 한다. 한 Effect만 가진 그룹도 위치·공격
소유권의 정상 대상이다. 전체 문서에서 이를 UI 편의 그룹으로 지우면 다른 Pattern의 변경도
`Selected Effect group is missing or has no members`로 거절되고 Dirty/Save가 활성화되지 않는다.
disabled occurrence도 보존 대상이며 미배치 catalog 정의는 실제 소유자가 아니다.
누락·빈 그룹 검증은 그대로 유지한다. 실제 P59 재현과 회귀는 09-25 KOUKU_RESULT_TUNING 결과 G12.

### 마리오 단독 입장자 사망과 앵콜 종료 경계

마리오 입장자 사망도 입장자 부재처럼 정상 기믹 실패다. 연결 해제와 묶어 runtime ABORT를
기록하면 raid cleanup이 살아 있는 보스까지 제거한다. HP0 귀환과 typed 부활은 기존 경로를
유지하고, 기믹 실패 완료 receipt를 실제 Raid가 소비한 뒤 같은 boss가 남는지 확인한다.

fixed update 중에는 현재 updateTick과 아직 commit되지 않은 m_iServerTick이 다르다.
컷씬 종료 판단과 다음 전투의 admission·commit에 같은 tick을 전달해야 한다. helper 내부에서
이전 tick을 다시 읽으면 정확한 종료 프레임만 거부된다. tick을 직접 조작한 helper 호출만으로
검증하지 말고 실제 room.Tick의 종료 직전·종료 경계를 통과시킨다. 상세 근거는 위 결과 G13.

양눈 attachment를 포함한 저장 Effect로 시선을 교체할 때 단안 occurrence 두 개의 bone/roll을
그대로 씌우지 않는다. boss root의 한 occurrence에서 기존 양눈 TRS를 소비하고 기믹 수명에만
loop를 건다. 조커 표적 과녁은 Server pattern target을, Ctrl 위치 핑은 별도 room broadcast를
소비한다. 로컬 입력 pending 표식과 전투 표적의 수명을 묶으면 다른 client가 같은 대상을
볼 수 없고 Ctrl만 눌러도 과녁이 나온다. 실제 연결과 검사 범위는 위 결과 G14.

### 빙고 반전 순서와 보상 수명

폭탄 중심+상하좌우 중 판 안의 모든 nonred 칸을 먼저 XOR한 뒤 가로·세로 줄을 한 번만
승격한다. 칸별 변경 도중 줄을 판정하면 같은 폭발이 완성 줄의 이웃 칸을 지우기 전에
빨간 줄이 잘못 확정된다. a2~a5 검정→a1 폭발은 a2 빈칸으로 빙고가 아니며, 이어 b2 폭발이
a2를 다시 채울 때 a행이 완성되는 예시를 실제 소비자 회귀로 유지한다. 대각선은 제외한다.

빨간 바닥의 영속 상태와 이미 보상받은 줄의 identity는 구분한다. 사용자 최종 규칙은 새1줄당
30초이며 같은 폭발에서 완성된 줄도 모두 사용 처리해 다음 폭발의 지연 보상으로 남기지 않는다.
일반 보스 패턴 교체와 Parent 재개는 board의 사용 기록을 지우지 않는다. 보상은 pinned
special 정의의 threshold/Result를 실제 완성 tick에 적용하고, 뒤늦은 Parent 판정이 같은
무적을 다시 적용해 만료 시각을 연장하지 않게 한다. 참가자와 생존 여부를 적용·소비 전에 확인한다.

블랙홀은 성공·실패 모두 폭발 tick의 유효 무적으로만 생존한다. 성공 분기를 일반 즉사로
처리하면 이미 만료된 줄 보상 이후에도 실드가 전멸을 막을 수 있으므로 player 판정을 공통화한다.
이난나는 승인된 같은 레이드 생존자에게30초 보호를 주고 Bingo만 이를 존중한다. 다른 기믹의
encounter wipe 규칙을 전역으로 약화하지 않는다. 상세 구현·검증은 위 결과 G15.

### 고정 플레이어 시점의 시퀀스 배우와 WORLD 파편

시퀀스 카메라만 멈추면 절대 WORLD 배우는 원본 공간에 남는다. 각 Client 진입 pose를 보존할
때는 authored view→held view의 변환을 배우의 최종 world에 적용하고 FOV 비율도 함께 맞춘다.
WORLD 입자의 birth root에 이 변환을 넣으면 이전 입자가 과거 카메라에 남는다. 원본 simulation과
history는 유지하고 완성된 현재 evaluated frame의 render 사본에만 현재 변환을 적용한다.
Client마다 다른 뷰는 Server 좌표·자산·shader에 쓰지 않는다. 카메라 행이 먼저 끝나도 Server
컷씬의 audio tail과 다음 gate commit까지 hold하며, F6 복귀 때 같은 캡처를 재사용한다.
수치 투영·history 불변과 최종 화면 확인을 구분한다. 실제 구현/검증은09-25 KOUKU_RESULT_TUNING G16.

### Publish의 WORLD 본 Collider 중복 계산

pattern closure와 bundle admission이 같은 WORLD 본 track을 반복 투영할 수 있다. 검증 횟수를
줄이기 전에 실제 native sampling의 중복 여부를 측정한다. 동일 게시 세션에서 root, sequence,
world/box/collider 전체 입력으로 계산 결과를 재사용하며 반환값을 격리한다. 외부 native 파일은
첫 계산 때 입력 snapshot에 등록하고 게시 직전 exact-byte freshness 검사를 유지해야 한다.
mutable 입력을 객체 identity만으로 캐시하거나 프로세스 간 결과를 무조건 재사용하지 않는다.
시간 개선은 동일 출력 hash와 함께 확인하고 Composition 단계 시간을 전체 UI Publish 시간으로
설명하지 않는다. 실제 구현과 측정 근거는09-25 KOUKU_RESULT_TUNING G20이다.

### WORLD 컷신 clip 분할과 원본 시계 불연속

원본 Matinee가 같은 clip의 source 위치를 되감는 경계는 baked animation에도 남을 수 있다.
timeline 박스만 나누고 분할 전후 sampling 동등성만 확인해서 동작을 복구했다고 설명하지
않는다. 경계 앞뒤 bone 회전·이동 및 원본 weight/control을 확인하고 native clip 편집과
구간 반복의 의미를 구분한다. sourceEndMs가 있는 WORLD 구간은 pose·부착 FX·본 Collider가
동일 범위를 사용해야 하며, 새 클립 교체가 다른 배우·camera/audio를 자동 이동하지 않는다.

원작 선택지를 추가할 때 이미 검증된 baked 구간의 key 값은 재샘플하지 않고 시간만
재기준화할 수 있다. 기존 continuous와 모든 model section을 보존하고 클립 목록 추가와
저장된 박스 start 변경을 구분한다. 음향 동기는 원본 event·자막 시각 및 각 배우의
timeline을 함께 대조하며 한 배우만 밀린 문제를 전체 음원 이동으로 덮지 않는다.

### 자식 시간 편집과 완전재생 Parent 참조

자식 lifetime과 이를 완전재생하는 Parent occurrence duration을 별도로 저장하면 자식만 바꾸는
편집은 전체 문서 검증에서 거절된다. 입력→candidate→검사→commit 경로에서 실제 바뀐 자식의
순차/loop 참조와 뒤쪽 시작을 함께 갱신한다. 기존 간격과 별도 고정 window는 보존하고 모든
변경이 유효할 때만 commit한다. 해당 오류를 publish 성능 문제로 단정하거나 검사를 삭제하지
않는다. 진단에는 실제 Parent/child ID와 두 시간을 포함하며 Save/Reload까지 검증한다.

분신 child의 Logic guard를 풀기 전에 Server의 분리된 배우 실행 경로가 그 Logic을 소비하는지
확인한다. 부모에서 실행한 회전과 특정 분신의 회전은 다른 상태 owner다. 배우별 ledger와
snapshot을 사용하고 접촉 판정보다 먼저 방향을 확정한다. 다른 분신·부모와 이미 종료한 분신에
상태가 전파되지 않는지 검증한다.


### WORLD 칼날 크기·경로와 포박 위치를 함께 확인

Object/Motion의 첫 scale key만 바꾸면 뒤의 baked scale key가 다음 프레임부터 값을
덮어쓴다. 일정 크기를 요청받으면 해당 motion의 전체 key와 실제 occurrence placement를
함께 확인한다. 공유 motion에 다른 시작점·yaw의 occurrence를 추가하면 도착점도 달라진다.
각 시작점을 보존하면서 하나의 감옥으로 모을 때는 해당 occurrence용 motion variant로
분리하고, emission yaw/offset → occurrence TRS 순서의 실제 소비 좌표로 도착점을 검증한다.

아이언 메이든의 표시 FX 위치와 MARIO_PHASE2_PLAYERS의 teleportPosition은 별도 저작
필드다. 표시만 옮기면 포박·마리오 복귀 위치는 남는다. 두 필드를 같은 위치로 맞추고
Server navigation 결과 및 WORLD ENTER_AREA 접촉/구출 성공 해제를 함께 확인한다.
칼날의 중앙 도착 시각을 새 사망 타이머로 만들지 않는다. 판정은 실제 collider 접촉이다.


### Summon child Logic 허용 조건은 모든 소비자에서 함께 변경

actor-local Logic을 추가하면 Client Composition validation, Python projection, canonical
KoukuBootstrapRows.ps1, Server Brain validation을 함께 대조한다. 앞의 두 검사만 통과해도
정식 Gameplay publish의 이전 guard가 새 Trigger를 거절할 수 있다. 짧은 BOSS_TRACK_TARGET은
34ms 이하·이동 추적 없음만 허용하고, 일반 Summon의 기존 Albion JUMP/SLAM 예외와
Cross Direction의 더 좁은 경계를 유지한다. 실제 최종 bootstrap 게시와 제품 로드를 완료
하기 전에는 데이터 반영 완료로 보고하지 않는다.

### 추적 Logic·조건부 폭탄·고정 예고의 소유권

BOSS_TRACK_TARGET이 회전한다고 별도 부채꼴 Effect까지 생성하는 것은 아니다. 실제 Effect
행을 유지하고 조건부 본체/폭발만 Duration에 stable occurrence ID로 연결한다. Server가
조건부 visual을 소유하면 정적 presentation에서 같은 행을 제외해야 무조건 폭발이나 중복
표시를 막을 수 있다. 파티 전멸 뒤에도 finite 폭발 tail은 자연 수명까지 유지하고 명시 Stop은 정리한다.

원본 부채꼴의 비균등 scale X/Z와 emitter/decal 전방을 조사하고 Server 타원 부채꼴에
반영한다. world 각도나 단일 반경으로 근사하지 않는다. 고정 사각 예고와 폭발은 먼저
생성된 예고의 보스 birth basis를 공유해야 한다. 각 행의 follow=false만으로는 생성 시각이
달라질 때 같은 위치를 보장하지 않는다. 서로 다른 원본 Effect의 180도 yaw 차이는 보존한다.

### 저작 schema 확장과 실행 중 구 Client의 Save

현재 저장 codec이 unknown field를 거부한다면 새 optional 필드도 구 실행 Client에는 호환되지
않는다. 그 상태에서 외부 최신본 병합은 Parse/Validate에서 막혀 미저장 draft를 Save하지
못할 수 있다. 필드 병합만 안전하다고 판단하지 말고 실행 중 소비자의 저장 schema도 확인한다.
새 필드 후보와 빌드를 준비한 뒤 사용자 draft Save 완료를 먼저 받아 최종 교체한다. 기존
저장본을 복구해야 하면 자기 설치 hash가 같은 경우에만 자기 변경을 회수하고 사용자 변경은
보존한다. 회수 뒤 같은 revision으로 재저장될 수 있으므로 publish freshness는 revision뿐
아니라 실제 source hash와 pattern 내용을 함께 대조한다.

### 같은 Effect asset ID의 레이저 collider 실측 재사용

asset ID와 occurrence TRS가 같아도 V1 내부 element 삭제·본별 회전·scale 변경으로 실제
레이저 크기가 달라질 수 있다. 이전 native CSV를 재사용하기 전에 authored Effect 내용,
설치 모델·본·preScale 및 animation 소비 경로를 대조한다. 변경됐으면 현재 source-direct
Effect를 실제 CModel 본과 particle world 평가로 다시 측정한다. primary shaft와 남아 있는
beam을 눈별로 분리하여 바닥 XZ의 BOX를 맞추고 피해창·음성·원래 Effect 자체는 보존한다.

### 선택적 Product 참조의 빈 값과 로드 실패 재시도

Projector가 기본값을 정규화해 만드는 targeted visual은 선택 필드의 빈 문자열도
직렬화할 수 있다. authoring validator와 Python projection 통과만으로 native
Product consumer 통과를 대신하지 않는다. 누락/빈 값은 미사용으로 같은 의미여야
하며 실제 참조의 kind·stable ID·시간·소유 관계는 계속 검증한다. 최종 게시본
전체를 runtime parser에 통과시키는 회귀가 필요하다.

Run 준비 성공 뒤에만 epoch를 기록하고 실패를 매 프레임 재시도하면 하나의
잘못된 optional field가 전 관문의 카메라/연출 문서 재파싱으로 확대된다.
Admission 대기와 확정된 parse 실패를 구분하고 같은 run/source/draft 실패는
보존한다. 새 실행 identity 또는 명시적 Reload/Reset으로만 무거운 재시도를 허용한다.
GPU frame timestamp는 CPU 제출 공백을 포함할 수 있으므로 전체 GPU ms만으로
VRAM 또는 shader 병목을 단정하지 않고 CPU scope·실제 render scope를 대조한다.

### Complete Play 준비와 활성 WORLD의 대기 계약

Level Update에서 Server WORLD 큐를 소비한 뒤 같은 프레임에 raid 준비가 호출될 수
있다. 특히 늦게 입장한 참가자의 정상 연출을 준비 실패로 회신하면 Server가 파티 전체를
취소한다. 활성 재생 때문에 문서 reload가 잠시 불가능한 경우는 `true/ready=false`로
기다리고 기존 WORLD Update를 계속한다. 실제 parse/revision 오류만 실패로 보낸다.
대기 중 파일을 매 프레임 다시 읽거나 현재 문서·모델 pool을 초기화하지 않는다.
활성 stable ID를 상태에 표시하며, 무한 반복·일시정지는 유한 연출 종료와 구분한다.


### Complete Play 준비 인원과 도구 자동 Open

개별 Pattern audition IDLE은 Server Raid PREPARING과 별개다. 전체 재생 진단은
ParticipantPlayerIds의 고정 roster와 iReadyMask를 읽어 준비 x/N·미준비 PlayerId를
표시한다. 로컬 READY 송신 성공과 Server 확인은 구분하며 확인 뒤 송신 대기 문구를
남기지 않는다. 상세 로컬 실패는 매 프레임 일반 대기 문구로 덮어쓰지 않는다.
다른 Client의 세부 리소스 단계는 기존 wire에 없으므로 추정하지 않는다.

F1 안의 Bingo Size 같은 embedded tuner 준비에 창을 여는 EnsureDebugTool을 그대로
호출하면 Action Workbench가 자동으로 열리고 focus를 가져간다. 내부 준비 호출은
bShowWindow=false를 사용하고 기존 창의 선택·입력 owner·preview를 보존한다. 준비 뒤
Hide로 되돌리는 방식은 Deactivate/Stop을 일으키므로 사용하지 않는다.


### 원본 마스터가 brightness 4.0 인 Landscape 레이어는 8bit bake 에서 형광 단색이 된다

UE3 Landscape 마스터는 HDR 조명 패스를 전제로 레이어 brightness 를 4.0 까지 저작한다.
`extract_ue3_landscape.py` 는 평범한 8bit albedo PNG 를 굽기 때문에 weightmap 이 그 레이어에
1.0 에 가까운 가중치를 주는 넓은 구역에서 텍셀 대부분이 같은 바이트 천장에 붙고, 질감이
사라진 형광 단색 덩어리가 된다. 마하라카 `lv_ocn_eventis_mhp_land_01_mi` 의 layer03
(노랑 tint) 과 layer04 (초록 tint) 가 그 사례다.

레이어별 hard clamp `min(1, max(0, x))` 는 이 증상을 고치지 못한다. clamp 는 포화된 색상비를
그대로 보존하므로 형광 색이 남는다. 실측으로 확인할 것: 255 클립 픽셀 비율과 노랑/초록
픽셀 비율을 타일별로 재고, clamp 전후가 같으면 그 수정은 실패다.
레이어별 headroom scale은 8bit clipping을 줄이는 표시용 근사일 뿐 원본 복원이 아니다.
레이어 사이의 상대 밝기를 변경하며, 클립 픽셀 0%도 올바른 레이어 혼합을 증명하지 않는다.
09-26 원본 재조사에서 `landscape_base`의 layer01은 AlphaBlend, layer02~07은 diffuse alpha를
높이 입력으로 사용하는 HeightBlend임을 확인했다. 기존 baker는 이를 일반 가중 평균으로
처리한다. 원본 static normal switch와 색공간·HDR 출력도 별도 확인해야 한다. 밝기 조절만
반복하거나 기존 베이크를 원본 shader 결과라고 기록하지 않는다.

지형 타일의 diffuse 소비 경로는 재질 문서가 아니다. `.wmodel` 안의 UTF-16LE 경로 문자열이
`textures/baked_diffuse.png` 를 직접 가리키고, `Engine/Private/Material.cpp` 가 확장자가
`.dds`/`.tga` 가 아니면 WIC 로 읽는다. `mapmaterials.json` 에는 LAND01 행이 아예 없다.
타일을 다시 구웠으면 Resources 물리 폴더의 그 PNG 를 실제로 교체해야 화면이 바뀐다.


### 물 자산은 renderMode=Water 여도 재질 surface 의 renderMode 가 이긴다

`.mapassets` 의 `renderMode=Water` 는 `mapwater.json` 행을 강제하지만, 실제 draw 는
`CMapAssetObject::Get_MaterialRenderProfile` 이 재질 surface 의 renderMode 로 덮어쓴 값으로
결정된다. `mapmaterials.json` 행이 `renderMode: translucent` 면 `bWater` 가 false 가 되어
레거시 물 패스(`fx_c_water_001` 거품을 수면 전체 diffuse 로 그림)를 타지 않고
`source.map.water-4x` program 을 탄다. 즉 그 자산의 `mapwater.json` 값은 불활성이다.
물 색을 고칠 때 어느 문서가 소비되는지 먼저 확정하고, 양방향 검사 때문에 `mapwater.json`
행 자체는 지우지 말 것.

`specialresource.mat.ocean_trn` 의 정확한 program 은 `source.map.water-42.v1` 이다.
이전 `Tools/LevelPlacementExtractor/author_ocean_water_rows.py` 는 "ocean_trn has no native
program in this project" 라는 틀린 전제로 `FAMILY = 'source.map.water-41.v1'` 근사를 썼고,
그 과정에서 `reflection_power`·`sky_*`·`fresnel_color` 를 버리고 `diffuse_color` 에
`sky_color*sky_intensity` 를, `reflection_color` 에 `fresnel_color` 를 바꿔 넣는다.
`fresnel_intensity` 가 0 인 수영장 물은 거의 흰 `reflection_color`(0.87,0.87,1.0) 와
`reflection_intensity` 20 이 감쇠 없이 들어가 카메라 방향으로 흰 번짐이 생긴다.
베른 `MAP_6D4F71329FE9_BG_SCD_RHD_FLOOR07_SM_KHB` 가 같은 `ocean_trn` props 파일
(`A643D1D45E9B`) 로 water-42 를 쓰는 것이 증거이며, water-42 의 텍스처 expressionIndex 순서는
`0 texture_normal / 1 detail_texture_normal / 2 texture_sky / 3 texture_reflection /
4 texture_fresnel / 5 texture_diffuse / 6 texture_diffuse_mask` 다.
09-26 재조사 수정본은 water-42 후보만 만들고 원본 30개 값과 7개 texture lane을 유지한다.
이전에 제외했던 river-rock 3종도 ocean_trn source chain이면 포함한다. 같은 부모 재질이어도
static wave/distortion/world-position 분기는 다를 수 있으므로 pixel 프로그램 연결만으로
vertex wave와 전체 원본 외형까지 복원됐다고 판정하지 않는다.

### SelfMotion 다중 행은 같은 배치를 한 번 초기화한 뒤 합성한다

한 placement의 MotionArr에 여러 회전/이동 축이 있을 수 있다. 매 행에서 원본 transform으로
되돌린 후 적용하면 앞 행의 결과가 사라진다. `Sample_SelfMotions`는 placement별 base를 한 번
준비하고 원본 행 순서대로 누적한 뒤 한 번 반영한다. Sequence가 관리하는 현재 visibility는
유지한다. 마하라카 게시본은 65행/59배치이며 6배치가 두 축을 가진다.

### 원본 테이블에서 정의가 없다는 것은 모델 삭제 증거가 아니다

정적 레벨 목록, Deploy NPC/Prop ID, LookInfo와 UPK mesh/AnimSet을 구분한다. 마하라카의
모코모코 `MN_ISMP_00`은 정적 메시 검색에서 누락됐지만 NPC 570910의 LookInfo와 실제
skeletal mesh 두 종류·애니메이션 10개 및 Deploy 좌표가 남아 있다. 키워드나 테이블 한 벌의
미발견을 전체 원본 삭제로 확대하지 않는다. 추출 성공과 런타임 재질·배치·동작 연결도 구분한다.


### Landscape는 레벨별 packed ShaderCache도 검색한다

공용 RefShaderCache에서 컴포넌트 static key를 못 찾아도 native 프로그램이 없는 것이 아니다.
마하라카는 별도 `sc_lv_ocn_eventis_mhp_land01` cache에서 설치 지형 16/16 key가 일치했다.
packed cache는 descriptor 수와 code blob 수가 다르므로 단순 1:1 parser를 쓰지 않는다.
원본 top UV는 section 좌표 * .1, 회전 후 tiling이며 HeightBlend는
`saturate(2*paint-1+diffuseAlpha)`다. diffuse와 normal의 layer별 blend/static enable도 다르다.
선택적 source-layer bake와 전체 HDR/RNM/GPU material 복원은 구분한다.

Prop 배치의 ID를 Npc 테이블에서 조회해 얻은 동명 번호는 소품의 모델이 아니다.
실제 EFTable_Prop와 LookInfo 연결을 사용한다. 57011의 300004는 모델 없는 collision Prop다.

`build_map_material_variants.py install`은 파일 단위 동기화가 아니라 Area 디렉터리 전체 교체다.
manifest에 없는 자산은 commit 때 `bounded_rmtree`로 사라진다. 2026-09-26에 자산 2종 manifest로
실행해 `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP`의 기존 382개가 전부 삭제됐고 09-19 cook
staging에서 복구했다. ownership receipt는 소유 확인만 하고 규모 축소를 막지 않았다.
이제 manifest가 기존 자산을 버리면 기본 거부이며 `--prune-missing`과 안전 상한
(절대 10개, 면적 20%)을 함께 통과해야 한다. receipt의 `installPrune`이 결정을 기록한다.
`LV_LUT_MIDNIGHTC_ED`(292자산)와 `LV_OCN_EVENTIS_MHP_FOLIAGE`(9자산)도 receipt 소유 Area이므로
같은 위험을 가진다. Resources는 Git 비추적 팀장 입력이라 삭제되면 git으로 되돌릴 수 없다.

위 Prop 항목 정정 (2026-09-26 원본 DB 실측). 57011의 300004는 모델 없는 collision Prop이 아니다.
`EFTable_Prop.db`의 Prop PrimaryKey 300004는 Model이 빈 문자열이지만, `EFTable_Npc.db`의
Npc PrimaryKey 300004는 Model이 `EFDLChar_MN_KZDW_02-1.MN_KZDW_02-1`이다.
57011의 Prop 레코드는 NPC ID를 들고 있으므로 이 zone에서는 Npc 테이블이 올바른 모델 출처다.
Prop 행이 비었다는 사실만으로 모델 부재라고 단정하지 말고 두 테이블을 모두 조회한다.
같은 이유로 57011의 Prop ID 필드는 ints_0x30_0x68[13]이며 그 값도 NPC 테이블 ID다.

## MainApp.cpp 한글 주석 끝의 공백은 지우면 안 된다

`git diff --check`가 `Client/Private/MainApp.cpp`의 한글 주석 5줄(7962, 7972, 7982, 8026, 8043)을
trailing whitespace로 경고한다. 이 공백은 **지우면 빌드가 깨진다.** 2026-09-27에 실제로 깨뜨렸다.

- 파일은 UTF-8(BOM 없음)인데 MSVC는 이 파일을 CP949로 읽는다. UTF-8 한글 한 자는 3바이트라
  줄 끝 바이트가 CP949 2바이트 짝에서 홀수로 남는다.
- 그 마지막 바이트(예: `0xA4`, `0x9C`, `0xB0`)가 뒤따르는 개행 `0x0A`와 짝을 이루면서 개행이
  주석에 먹힌다. 그러면 다음 줄이 `//` 주석의 연속으로 사라지고, 그 줄이 선언하던 변수가
  `C2065 선언되지 않은 식별자` / `C2737 const 개체를 초기화해야 합니다`로 터진다.
- 끝 공백 한 칸이 그 짝을 맞춰서 개행을 살려 준다. 장식이 아니라 기능이다.

**규칙:** 이 파일의 한글 주석 줄 끝 공백을 `git diff --check` 경고를 없애려고 지우지 않는다.
완료 보고에 그 5건 경고는 의도된 것으로 적는다. 인코딩 일괄 변환은 별도 합의 작업이다.

## .wmodel의 텍스처 경로는 UTF-16이라 ASCII 검색으로는 0건이 나온다

리소스 의존 closure를 계산할 때 `.wmodel`을 ASCII로 훑어 `.dds`가 0건이면 **모델이 텍스처를
안 들고 있다는 뜻이 아니다.** 2026-09-27에 이걸로 Drive 전달 목록에서 텍스처 727개를 빠뜨렸다.

- `MODEL_MATERIAL_DATA`(`Engine/Public/BinaryAsset/ModelAssetData.h`)의 `diffusePath`,
  `normalPath`, `specularPath` 등은 `std::filesystem::path`이고 Windows에서 `wchar_t`다.
  그래서 `.wmodel` 안에 **UTF-16LE**로 직렬화되어 있고 `WModelDecoder`가 그걸 채운다.
- 경로는 Resources 루트 상대가 아니라 **그 모델 폴더 상대**다. 예: `textures/<hash>_<name>.dds`.
  따라서 모델의 디렉터리에 붙여서 해석해야 한다.
- 확장자는 `.dds`뿐이 아니다. `Engine/Private/Material.cpp`의 `LoadTexture`가 `.dds` → DDS,
  `.tga` → 전용 리더, 그 외 → WIC로 보내므로 `.png`도 실제 런타임 입력이다.

**검색 패턴:** `(?:[ -~]\x00){3,}?(?:\.\x00)(?:d\x00d\x00s\x00|t\x00g\x00a\x00|p\x00n\x00g\x00)`

**규칙:** 맵·캐릭터 자산의 전달 목록은 `시각 기준 델타`가 아니라 **참조 closure**로 만든다.
게시 문서(catalog/mapmaterials/mapwater) + 카탈로그 + 각 모델 내장 경로까지 합쳐야 완전하다.
`converter.log.txt`, `.gltf`, `.bin`, 변환 영수증 `.json`은 런타임 입력이 아니므로 제외한다.
### 쿠크 Bingo preview와 포박 판정 경계

- Effect sourceModelPreview의 관문 정본은 `BINGO`다. legacy `ENCORE`는 codec decode에서 정규화한다. Effect Load/Drawable 성공만으로 재생 성공을 판단하지 말고 metadata → SourceProp → Composition model selection까지 검사한다.
- `isPatternBound`는 입력 잠금이다. 매틱 처리나 formation 취소에서 `isCombatReady=false`를 남기면 즉사 칼날을 포함한 피해 판정에서 제외된다. 검증 fixture도 readiness를 강제로 복구하지 말고 실제 room update를 거친다.
- WORLD JSON은 Client16MiB 제한이 있다. 필드 병합 시 숫자 vector를 원본처럼 한 줄로 보존하며, 전체 pretty-print 팽창을 데이터 증가로 오인하지 않는다.

### 원본 Matinee의 시간과 배우·부착 소유권

- Slomo가 있는 Matinee는 source 시각과 실제 경과 시각이 다르다. 카메라·WORLD·가시성·
  애니메이션·자막·cue를 하나의 적분/역변환으로 맞춘다. 오디오 샘플 속도와 Stop 시각은
  구분하고, 파티클 lifetime 꼬리를 Matinee 길이로 잘랐다고 원본 복원으로 기록하지 않는다.
- 동일 mesh의1/2 배우를 이름만 보고 합치지 않는다. Matinee variablelink→actor/component→
  visibility/baseBone을 확인한다. 새 부착물을 만들기 전에 World clone의 native hat/weapon
  자동 부착까지 확인한다. 재질 트랙은 원본 대상 component에만 적용한다.
- constant 변환 키는 정확한 소수 source 시각을 확인한다. 반올림 양쪽이 이전 자세를
  샘플하면 다음30Hz frame까지 가짜 이동을 만든다. 본 부착 reduction은 저장 frame뿐 아니라
  실제 재생 구간 중간 표본을 대조한다. full을 retime한 뒤 같은 결과에서 split을 잘라야 한다.
- `effectSourceTimeKeys`는 원본 Effect 내부 시각을 보존한다. 일반 occurrence와 SCENE_PROFILE
  투영 모두 absent일 때 빈 배열을 쓰지 않는다. Save 검증과 live anchor preview 검증이 같은
  fixed MAP 제약을 쓰고 V2 trim이 normalized lifetime을 압축하지 않도록 한다.
- WORLD authoring·Map publisher·Composition dependency validator는64 track/4096 key와
  optional materialTracks/BOX·CYLINDER를 같은 strict 계약으로 검증한다. 한 소비자만 오래된
  필드/키 상한을 유지하면 파일 저장은 성공해도 공식 게시가 막힌다.

### Complete Play의 선택적 Effect 시계와 전체 Product 준비

추적 폭탄은 occurrence 기본값을 채운 복사본을 targetedCombatVisuals에 투영한다.
기본값에서 뺀 필드도 복사본에 이미 있으면 다시 출력될 수 있다. 미사용
effectSourceTimeKeys=[]는 최종 occurrence 투영에서도 생략하고 reader는 absent와
같게 읽는다.1개 키·비배열·잘못된 순서/범위는 계속 거절한다. Animation binding과
Effect resource 준비만 성공해도 전체 presentation parser가 실패할 수 있으므로
새 READY 준비 identity에서 전체 native Product parser를 한 번 검증한다. 같은
source 실패를 매프레임 재파싱하지 않으며 기존 재생 cache를 보존한다.

방향성 warning은 mesh/root의 관습적인 전방을 그대로 쓰지 않는다. DDS의 문양
방향→quad UV→source StartRotation/axis lock→WORLD occurrence 회전으로 실제
전방을 구한 뒤 이동 곡선과 내적을 검사한다. 공유 asset 크기/재질은 유지하고
해당 occurrence 회전만 고친다.09-26 PLAYTEST_RECOVERY G07 결과에 수치를 둔다.


### 쿠크 게시 schema·고정 장판 높이·피해 소유

- 같은 공간의 고정 장판과 별도 공간의 플레이어 표식은 본 socket을 공통 정답으로 삼지
  않는다. 장판은 bone 없는 BOSS/followBoss=false로 발생 시점 root를 한 번 캡처하고,
  머리 표식은 actor translation에 고정 높이를 더한다. cold 입장에서 표시할 모든 색을
  product prewarm 목록에 포함해야 하며 이미 본 색만 보이는 warm-cache 결과로 완료하지 않는다.
- Mario 참가자 제외는 최초 random 선택과 기존 target 재조회 모두에 적용한다. 알비온의
  유효 대상이 없으면 보스의 navigation ground를 고정 표적으로 사용한다. 청취 범위는
  로컬 Server snapshot의 Mario 상태로 결정하고 Composition SOUND, WORLD soundTracks와
  종료 후 tail을 함께 정리한다. BGM 전체를 mute하거나 시각·Server pattern clock을 멈추지 않는다.
- 다른 플레이어 청취 격리는 Character의 일반 skill/vehicle과 Esther의 별도 action 소비자를
  모두 확인한다. 이미 시작한 음원은 owned handle로 중단하고 mute 중에도 cue cursor를
  소비한다. 로컬 Mario 상태를 remote action보다 먼저 반영하며 첫 remote snapshot의
  network-state 미준비를 청취 허용 조건으로 사용하지 않는다.
- 원본 Mario는 일반 광대 변신의 MN_RPCZ와 별도 Polymorph다. 실제 Polymorph→skill→action→
  notify를 먼저 확인한다. 다른 body에 retarget할 때 source weapon/socket과 설치 mesh의
  같은 정점을 대조하고 hand-frame cook, preScale, socket pitch를 함께 적용한다. source FX
  복원과 프로젝트 body/contact-time 재매핑, 사용자 화면 확인을 서로 구분한다.
- 성공 문구는 animation 이름·groggy 진입으로 추측하지 않는다. 일반 stagger gauge와
  쿠크 HP-threshold STAGGER_WINDOW의 성공 edge는 서로 다른 생산자다. 피해0 성공도
  DAMAGE_EVENT로 전달하고 실제 피해 숫자·DPS와 별도로 표시한다.
- 원본 진입 음원 조사에서는 메인 컷신 Matinee뿐 아니라 선행 trigger가 재생하는 별도
  Matinee/InterpData의 SoundTrack을 확인한다. 기존 WORLD motion에 귀속시켜 trigger와
  시작 시계가 같게 연결하고 컷신 전체에 임의로 붙이지 않는다.

- 공식 Kouku owner는 Gameplay보다 앞서 World encounter metadata도 읽는다. 새 optional
  pattern 필드는 양쪽 schema 경계에 연결해야 한다. trackBombs처럼 한 검증기에만 빠지면
  저작 저장은 성공해도 owner 전체가 rollback된다. 검증 자체를 제거하지 않는다.
- 같은 Effect가 Append에서는 보이고 group에서 숨으면 먼저 실제 occurrence Y와
  source local Y를 지면에 대조한다. STATIC MAP은 절대 배치이고 Showtime의
  Server-controlled template은 navigation ground에 상대 Y를 더한다. 두 값을 일괄
  변경하면 이미 지면 기준인 서버 그룹이 뜬다. group 이름만으로 carrier 결함을 가정하지 않는다.
- 망치 충돌은 모델의 진행축/가로축과 배율을 각각 확인한다. WORLD collider head와
  저장 Object scale이 Server typed geometry와 Debug/Release 표시의 같은 정본이다.
  새 필수 geometry를 연결할 때 timeline trigger뿐 아니라 entry idle의 자동 보드 시작도
  현재 pinned flow의 실제 BINGO_BOARD 정의를 소비해야 한다. 빈 synthetic trigger는
  보드 시작을 건너뛰어 원래 폭탄·망치 deadline까지 밀리게 한다.
- GRABBED는 일반 공격에서 제외된다. 잡기 전용 저작 피해만 허용할 때는 같은 boss ID,
  pattern sequence, BOSS_LEFT_HAND 및 활성 hold를 모두 검사하고 전역 면역을 풀지 않는다.
- 주사위는 동문양 피해 면제와 속박 해제가 별도 소비다. 실제 CombatObject 접촉에서
  owner/sequence가 일치하는 속박까지 해제해야 하며, 피해 0만 확인하면 구조상 누락을 놓친다.
  자유 1명 선정, 실제 이동 명령, 속박된 플레이어의 카드 가로막기와 다음 tick 해제 유지까지
  대조한다. 이 누락의 재현만으로 과거 전원 고정·GPU 이펙트 누락의 원인을 확정하지 않는다.
- 지속 source 링이 몇 초 뒤 꺼지면 Required EmitterLoops의 생략값을 완전한 CDO chain에서
  확인한다. 기본값0을 provisional1로 저장한 자산은 원본 근거가 있는 element만 복원한다.
  외부 시각으로 재생하는 occurrence는 기존 loop-to-duration과 bounded source end도 함께
  연결해야 한다. loopCount만0으로 고친 CPU 후보가 계속 사라지는 경우를 성공으로 기록하지 않는다.
- 랜덤 표적 Duration은 시작 cue의 선택과 종료 시 조준 확정을 나눠 연결한다. 같은 선택 ID의
  종료 위치를 ledger에서 한 번 저장하고, 이후 stage retarget이 덮지 않도록 occurrence로
  소유한다. 다음 선택·완료·취소·새 패턴 시작은 원래 yaw를 복구해야 한다. 사용자 타이밍과
  원본 무기 궤적을 바꾸지 않고 실제 다음 공격 stage까지 고정되는지 검사한다.

### ocean-42의 월드 위치와 카메라 상대 위치

원본 PS가 translated-world 입력에 origin을 더해 UV 위치를 복구하고 origin에서 그 위치를
빼서 시선을 만들 때, 절대 위치와 0 origin의 조합은 월드 원점을 카메라로 오인한다.
program 42의 baked/non-baked 입력은 source 축·cm 단위의 camera-relative 위치와 실제
camera origin을 함께 전달한다. 투영의 마지막 행도 같은 origin을 반영해 깊이를 보존한다.
높이나 Fresnel 색을 먼저 바꾸지 말고 원점에서 멀리 떨어진 같은 장면의 평행이동 불변성을 검사한다.
추출 DDS와 설치 DDS의 mipCount는 별도로 확인한다. 추출 receipt의 성공이 Resources에
원본 mip 체인이 설치됐다는 증거는 아니다. 09-27 MAHARAKA_MAP_RESTORATION_RESULT 참조.


### Landscape glTF와 최종 WModel의 좌표계를 따로 검사한다

Client `(UE X,Z,-Y)` 정점을 glTF에 그대로 기록하면 converter의 RH->LH 변환이
타일 local Z를 다시 뒤집는다. anchor가 정확해도 높이/painted layer가 엉뚱한 위치에
나타나 육지가 침수된다. 직렬화 시 position/normal/tangent Z, tangent handedness,
winding을 함께 RH로 변환하고 최종 WModel+placement를 원본 grid/height/collision과
대조한다. 기존 WModel과 triangle count/positions가 같다는 검사는 원본 일치 검사가 아니다.
마하라카 16개만 재cook했으며 기존 베른 설치 리소스에 일괄 보정을 전파하지 않았다.
09-27 MAHARAKA_MAP_RESTORATION_RESULT G06 참조.
- 클래스 무비의 머리카락이 사라지면 geometry 존재와 material texture 검사만으로 끝내지 않는다.
  원본 PS의 leading unowned CB prefix가 primitive opacity/environment를 곱하는지 확인한다.
  FT06 native600은 material pack 앞 cb0[0..1]의 Base 환경 배율과 Base/Light opacity를
  identity로 연결해야 한다. 실제 shader ID와 closure를 검사해 generator와 설치 shader를 함께 고친다.
  다른 native program의 비슷한 row에 같은 값을 일괄 주입하지 않는다.
- 탑승 ID와 비행 phase는 다르다. 드래곤 카메라는 GROUNDED를 제외한 비행 phase에서만 적용하고,
  공중 이동에 걷기 nav projection 또는 ground-height mirror collision을 적용하지 않는다.
  실제 고도의 충돌과 착륙 가능한 지면은 Server에서 따로 검사한다.
- 원작 본 파티클이 긴 잔상처럼 보이면 skeletal afterimage로 단정하지 않는다. 실제 occurrence의
  emitter lifetime·density·alpha를 확인하고 프로젝트 튜닝은 기존 sourceScale에 둔다.
  sourceRecipe를 덮어쓰거나 같은 asset의 무관한 emitter에 보정을 전파하지 않는다.
- 광원 배열 prefix만 setter에 보낼 때 모든 consumer가 count 안에서만 읽는지, count가0으로
  줄어든 뒤 stale tail을 읽지 않는지, 캐시가 byte length도 비교하는지 함께 확인한다.
  Effects11 setter bytes 감소를 실제 GPU cbuffer 업로드 감소나 FPS 개선으로 보고하지 않는다.
- 비행에 기존 평면 body sweep을 그대로 쓰면 deltaXZ=0인 수직 하강이 몸체를 통과할 수 있다.
  높이 변화가 있는 이동은 Y slab와 XZ 원형 진입·이탈 시간을 함께 검사하고, 몸체 전체를
  통과해 끝점이 다시 빈 공간인 경우도 검증한다. 평지 접선 미끄러짐 계약은 별도로 보존한다.
  착륙 시작 때의 nav 성공을 착륙 완료의 성공으로 사용하지 않는다. 동적 지형을 다시 검사하고,
  낮은 층 착륙이 취소될 때 비행 최대 높이 clamp가 현재 Y를 갑자기 낮추지 않는지도 확인한다.

- 눈 texture와 UV가 정상인데 흰색으로 덮이면 다른 class와 실제 native program·uniform·UV를
  대조하고, 최종 scene의 여러 광원을 함께 재현한다. 가디언 native5는 별도 고장 난 shader가
  아니라 정상 class와 같은 경로다. shadowfactor0.6의 최소 광량0.4가 광원마다 누적되는
  경우 반사 강도만0으로 낮추어도 밝기가 남는다. 가디언 일반/movie 눈에만 저장한
  shadowfactor1/tdspecular_intensity0.25는 사용자 요청 프로젝트 튜닝이며 원본 복원값과
  구별한다. donor 재설치도 이 두 저작값을 보존하고 전역 조명·exposure를 바꾸지 않는다.

- 원본 texture의 identity는 leaf 파일명이 아니라 package.object다. 같은 leaf를 전역 사전으로
  합치면 다른 package의 의상 피부 texture가 잘못 연결될 수 있다. material별 source-qualified
  ID에서 실제 byte/pixel까지 대조하고 Resources-relative 경로로 연결한다. GBResources 전달도
  새 모델·texture뿐 아니라 새 catalog가 참조하는 기존 공용 의존성의 closure까지 검사한다.

- native 재질의 함수 본문만 그룹으로 나누고 case를 공통 파일에 남기면 ID 한 개를 추가해도
  모든 FX가 다시 컴파일된다. SourceCharacter는 Base/Light leaf에 본문과 case를 함께 두고
  실제 generator의 재등록·수식 변경·no-op가 각각 필요한 파일만 쓰는지 검사한다.
- SourceCharacter CPU packing은 공개 헤더의 inline 구현으로 복귀시키지 않는다. 단일 CPP와
  `SourceCharacterMaterialParameters_Generated.inl`을 generator/publisher가 함께 소비한다.
  헤더 선언과 private 구현을 나눈 뒤 모든 원래 소비자의 링크·실제 상수 결과를 확인한다.
- shader 함수 본문을 조건부로 제외할 때 Texture/Sampler/cbuffer/default까지 같이 지우면
  CShader의 variant ABI가 달라진다. 모든 선언과 실제 호출 closure를 보존하고 전처리·CSO의
  pass/input/변수 계약을 따로 검사한다. 전처리 parser는 FXC의 함수명/괄호·case/colon 사이
  공백을 허용하며 기대 program 집합이 빈 채 PASS하지 않도록 nonempty/count 검사를 둔다.

### World Movie의 V1 Solo·Group과 원본 shader prefix

- `effect.classselect.*` Movie 편집은 실제 Element Solo 진입점과 두 Play All 창을 같은
  ClassSelectionPresentation owner로 연결한다. 전체 draft와 선택 target을 분리하고
  dependency provider는 simulation에 남기되 draw ID는 선택 element로 제한한다.
  PSC particle age, phase source time, Movie wall time을 구분해 구간을 구하며 외부
  Play/다른 phase Seek 뒤에는 Tool의 남은 선택 ID를 실제 owner 상태에 맞춘다.
- Original DXBC의 leading unowned cb0 row를 전부 0 또는 최종 `o0.w` 패턴만으로
  결정하지 않는다. primitive opacity가 중간 register에서 곱해지는 경우와 masked
  LocalVF가 row0 RGB/alpha를 함께 소비하는 경우를 구분한다. MIC row 소유 및
  exact PS/VS를 검증한 program의 generator와 설치 HLSLI를 함께 수정한다.
- source/texture/emitter 존재, finite CPU particle, shader 함수 출력, 실제 draw 제출과
  최종 화면은 다른 증거다. 정상 A와 Movie가 같은 MIC를 써도 occurrence clock과
  camera/frustum까지 비교하며 정상 A를 근거 없이 변경하지 않는다.

### Sequence 카메라 저장 대상과 타임라인 표시 행

- Object/World는 공통 box 그림만 공유해서는 Boss/Sequence와 같은 편집 UX가 되지 않는다.
  겹침 기준 행 배치를 공유하고 actor/slot을 partition한다. 표시 lane index는 저장 ID가 아니며
  접기는 객체 visibility나 재생을 바꾸지 않는다. 유한 animation의 실제 clip 구간과 hold tail을 구분한다.
- 스킬 타격 시간을 연출에 맞출 때 PlayerSkills.hitTimeMs만 바꾸면 실제 HitShapes의 DAMAGE/COUNTER/STAGGER clock이 남을 수 있다. 동일 stable skill/clip에서 effect occurrence 시작과 camera 종료를 대조하고 실제 Server shape clock·animevents·provenance를 함께 맞춘다. camera 시간을 Server runtime이 직접 읽는 우회는 추가하지 않는다.
- ALT V의 action arrangement와 product recovery effectsequence는 서로 다른 저장 owner다.
  preview camera를 편집할 때 원본 effect/camera ID·clip 시간 매핑을 보존하며 정본 편집은 원본 시간에서 한다.
  camera-only stable ID 병합·CAS 저장과 실제 product camera cache 갱신까지 확인한다.
- Movie camera cut은 phase 전체 coverage를 요구한다. 컷 순서를 바꿀 때 start를 다시 배치하고,
  경계 trim은 두 이웃 cut을 함께 검증한다. 키 하나의 변경은 촘촘한 원본에서 매우 짧게 보일 수 있으므로
  얼굴 구도 조정은 필요한 구간의 Eye/LookAt offset을 함께 사용한다.
- Save/Data 저작 정본, Publish/검증된 실행 데이터, 실행 중 소비자의 reload를 구분한다.
  Client 전용 카메라 수치를 Server에 복사하거나 local publish를 원격 Server 적용으로 표시하지 않는다.


### World Movie 위치 입력과 회색 plane 판정

- LocalVF의 TEXCOORD 번호만 보고 clip 위치를 전달하지 않는다. 원본 VS instruction/PS 소비를
  대조하여 world-cm varying과 source camera prefix를 연결한다. 1518/1523의 네 경로 수정과
  실제 clip 입력인 1509/1517 보존은 같은 회귀 검증에 둔다.
- StaticMeshComponent가 near/far PS를 쓴다는 이유로 DecalComponent CDO를 상속시키지 않는다.
  far=0의 `saturate(5*far/far)`를 1로 약분하는 것도 원본과 다르다. source draw-binding이
  미확정이면 해당 object를 미복원으로 남기고 다른 정상 shader에 강제값을 전파하지 않는다.


### Movie box 시간 이동과 저장 후 Publish 상태

- Effect body 이동은 visibility start/end만 옮기지 않는다. 기존 clock/root/parameter의 경계
  sample을 보존하고 구간별 시간 변환과 Hermite tangent를 함께 바꾼다. 원래 phase0에 붙은
  occurrence를 옮길 때 endpoint를 그냥 고정하면 새 시작점의 particle age가 달라진다.
- Camera 순서 이동은 컷 길이와 별개인 insertion point다. phase 끝으로의 이동을 기존 길이로
  막거나 insertion point를 정수로 반올림해 빈 구간으로 만들지 않는다. 실제 저장 컷들은
  기존 정수 duration을 누적해 phase를 덮으며 edge trim은 인접 두 컷을 함께 검증한다.
- World animation 첫 clip의 지연 시작은 현재 owner가 지원한다. 오래된 first-start=0 주석으로
  UI를 막지 않는다. MOTION_END Effect/Collider의 start=0 anchor 계약과 구분한다.
- Save에서 내가 World 파일을 썼는지만으로 Publish 필요 여부를 판단하지 않는다. 외부 World
  변경을 병합한 camera-only Save도 실제 게시본과 비교한다. Publish 직전 source freshness와
  완료 뒤 게시본 일치를 확인하며, 재시작한 Object editor는 저장된 연결 문서에서 게시 연계를
  복구한다. ReplaceFile 백업도 실제 baseline과 대조하여 freshness 확인 직후의 외부 수정을 보존한다.

### WRL ComPtr 주소 차용은 std::addressof 사용

`const ComPtr<T>* borrowed = &owner`는 C++ 객체 주소의 무해한 차용이 아니다. WRL의
`ComPtr::operator&`가 반환한 `ComPtrRef::operator T*`는 owner를 nullptr로 만들고 기존 COM
참조를 Release한다. 재질의 SRV 복사를 줄일 때는 `std::addressof(owner)` 또는 소유자를
보존하는 명시적 참조를 사용한다. API가 새 COM 출력을 쓰는 경우의 `GetAddressOf`/
`ReleaseAndGetAddressOf`와 구분한다. missing texture처럼 보여도 첫 Bind 전후의 SRV identity와
반복 Bind를 확인하며, shadow pass만이 아니라 같은 material의 일반 draw도 검증한다.
실제 Character Select 재현과 검증 근거는09-26 WORLD_MOVIE_EFFECT_EDITOR_RESULT의 G11을 따른다.

### Movie 검사에서 visibility와 카메라 소유권

- Inspector 기능이 있어도 Sequencer에서 진입할 수 있는지 확인한다. 선택 WORLD의 Mute/Solo와
  Delete/Restore는 같은 stable-ID owner를 사용하고, Delete 키는 focus·텍스트 입력·drag를 검사한다.
- 마지막 Element의 Mute는 빈 draw mask를 허용하되 전체 document와 Movie clock을 유지한다.
  저장형 Visible OFF는 nonempty all-hidden 문서로 검증하고, 비어 있거나 손상된 문서까지
  drawable 검증에서 허용하지 않는다. 실제 저장·재로드·stale writer 거부와 복원을 함께 확인한다.

- WORLD Solo/Mute/Delete 표시 제외는 draw gate로 처리한다. CWorldSequenceObject::Hide 또는
  authored visibility 변경으로 임시 격리하면 Try_GetObjectPivot/본 부착 소비자가 사라질 수 있다.
- 외부 샘플링 Effect의 임시 숨김은 Set_Visible(false)와 다르다. 후자는 owner controls/afterimage/
  overlay 상태를 정리하므로 draw 전용 flag를 queued spawn, active root, preview replacement까지 유지한다.
- Movie F6에서 Stop을 호출하지 않는다. Camera_Free의 follow requested 전환을 Movie owner가
  소비하고 End_PresentationOverrideAtCurrentPose로 pose/FOV를 인계한다. 자유 모드에서 매 frame
  base follow FOV를 복원하거나 Seek/Loop 때 camera override를 다시 잡지 않는다.
- Movie Delete는 scene별 excludedWorldObjectIds의 stable object ID만 저장한다. Intro/Loop에 실제
  바인딩된 리소스인지 검증하고 다른 Movie/전역 WModel을 삭제하지 않는다.

### Movie 물방울의 distortion 근거

- 기본 의상의 TGA 경로가 full mip를 만들더라도 같은 그림의 Movie DDS는 한 mip뿐일 수 있다.
  anisotropic sampler만으로 없는 mip가 생기지 않는다. 반사 lookup의 실제 SampleLevel 요청과
  SRV mip 수·mip0 픽셀·색공간을 대조한다. 정상 TGA와 동일한 입력이면 해당 Movie의 texture
  참조만 재사용하고 공용 DDS·shader·보스 재질을 덮어쓰지 않는다. 같은 증상이라도 환경 cube를
  쓰는 별도 program은 이 수정에 포함하지 않는다.
- 환경 cube에 mip가 충분해도 표면 normal/diffuse/ORM/color mask/emissive DDS는 한 레벨일 수
  있다. roughness 채널·색공간·cube LOD와 표면 sampler를 분리해 검사한다. 원본 BC mip가
  있으면 기존 UModel 추출 경로로 회수하고 mip0 byte-exact와 실제 SRV mip를 확인한다.
  이 복구를 밝기 옵션 변경이나 거칠기 강제 조정으로 대신하지 않는다. Guardian Movie 범위와
  화면 확인 경계는09-27 WORLD_MOVIE_HAIR_GUARDIAN_EYES RESULT G12를 따른다.

- 같은 맵의 반사 감사를 `mapmaterials.reflectionTexture`만으로 끝내지 않는다.
  WorldSequences의 materialProfile, NpcCatalog의 실제 사용 모델 override와 native water의
  TextureExpression/texture_sky까지 소비자를 따라간다. 정상 TGA는 같은 mip0·색공간을
  확인한 참조에만 재사용하고, 원본 압축 mip가 존재하는 DDS는 그 payload를 보존해 복원한다.
  내용 해시가 파일명에 포함되면 새 hash 경로로 연결하며 공통 단일 mip 파일을 덮어쓰지 않는다.
  맵 DDS만 전달하면 Character lookup TGA가 빠질 수 있으므로 최종 참조에서 GBResources를
  다시 대조한다. 마하라카의 적용 범위는09-30 SOURCE_LIGHTING RESULT 후속 절을 따른다.

Guardian Movie watersplash native4645~4647에는 원본 shader map에도 별도 distortion shader가
없다. UV distortion 파라미터와 SceneColor 굴절 pass를 혼동해 companion을 추가하지 않는다.
정확한 MIC static set·VF·shader ID를 먼저 대조하고, 미연결 WORLD crack과 particle 물방울을
같은 대상으로 취급하지 않는다. 수치 draw 성공은 사용자가 본 특정 프레임의 가려짐 판정과 다르다.

### 일반 헤어를 Movie 골격에 연결할 때

- determinant -1의 좌표 변환은 tangent handedness도 반전한다. 76-byte legacy 정점에는
  sign이 없어 reader가 +1로 복원하므로, 명시 sign을 가진 WINT 정점으로 바꾼 뒤 반전한다.
  위치·normal·tangent만 같아도 binormal이 반대일 수 있다. 실제 reader의 N/T/B를 대조한다.
- normal-map basis 결함을 머리 실루엣의 원인으로 단정하지 않는다. 정상 머리를 Movie 의상에
  조합할 때 geometry·재질을 함께 옮기고 본 이름·inverse bind·animated head-space를 검사한다.
  기존 Movie clip과 배우 ID는 유지하며 새 파생 WModel의 설치와 데이터 게시를 각각 확인한다.
- Movie 원본에서 weight가 없던 facial bone은 이름이 있어도 inverse bind가 새 donor 정점에
  맞지 않을 수 있다. donor의 weighted inverse bind를 좌표 변환해 매핑하고 Movie skeleton과
  clip은 유지한다. 파일 decode·본 수만 검사하지 말고 실제 skinning 후 원형 오차·전구간 크기를 대조한다.

- donor와 Movie의 본 수가 다르면 양의 weight가 사용하는 실제 본 이름부터 대조한다. 누락된
  본을 버리거나 head에 몰아 붙이지 않고 Movie clip prefix를 유지한 호환 파생 골격을 만든다.
- 좌표계 변환은 geometry와 local rest/inverse bind·normal·tangent·winding에 함께 적용한다.
  원래 본의 실제 animated combined matrix와 weighted rest skin 오차를 별도로 확인한다.
- 기본 헤어 변경은 stable defaultVisualSetId로 선택한다. 기존 배열을 재정렬하면 숫자로 저장된
  사용자 preset이 다른 헤어를 가리키므로 순서를 유지하고 명시적 선택은 그대로 복원한다.


## Guide AI: 인간 인원과 공간 접촉을 분리한다

- `m_Players`에는 session 없는 초대용/동행 가이드가 들어간다. 인간 인원·기믹·MVP는 `Is_Human()` 또는 `Count_HumanPlayers()`로 판단하고 actor 수를 인간 수로 사용하지 않는다.
- Guide를 보스 target에서 제외하기 위해 `isCombatReady=false`로 두면 피해도 차단된다. 실제 companion은 전투 가능 상태로 두고 target/gimmick eligibility만 분리한다.
- `Is_Judgeable`의 인간 전용 기믹 판정과 `Can_ReceiveSpatialContact`의 물리 피격 판정을 구분한다. Guide의 outside 기믹 판정을 건너뛰어도 `InsidePlayers` 이탈 latch는 지워야 재진입 갈고리/장판이 다시 작동한다.
- Guide skill 직접 적중뿐 아니라 projectile/combat-object의 `SERVER_PLAYER_TO_WORLD_HIT::bGuideSource`도 전파해야 counter/stagger/part/MVP 제외가 동일하게 적용된다.
- Guide Save에서 PowerShell 함수 결과의 singleton 배열을 scalar로 풀면 무관한 필드 수정이 충돌로 오판된다. `File.Replace` rollback의 null backup 인자는 PS5에서 빈 문자열로 바뀔 수 있으므로 실제 복구 fixture를 유지한다.
- Guide 신규 C++는 UTF-8 BOM 없음이며 한글 ImGui 문자열을 가진 TU는 프로젝트의 파일별 `/utf-8` 옵션을 유지한다. 기존 CP949 파일을 일괄 변환하지 않는다.
- Guide의 `GUIDE_STARTED`에는 박스가 없다. 공간 편집은 `콜라이더` 목록의 `SPACE_ENTER` 행을 선택해 별도 `Collider Detail`에서 수행하며, Show Debug는 현재 Area의 전체 draft 박스를 표시한다. 시작 인사에 상점 대사를 연결한 것을 상점 공간 트리거 생성으로 해석하지 않는다.
- 공간 대사를 promptId만으로 예약하면 이탈 후 다른 장소에서 발화한다. 발생 trigger ID·priority를 보존하고 미시작 예약의 현재 접촉을 재검사한다. 취소된 예약은 cooldown을 소비하지 않는다. 안내 시작·월드 귀환만 현재 접촉을 seed하고, Guide 재배치·부활은 기존 owner 접촉을 유지하여 실제 스퀘어홀 진입을 억제하지 않는다. 같은 대사의 중첩 박스는 실제 발생한 다른 출처를 보존해 한 박스 이탈 시 남은 박스의 안내가 유실되지 않게 한다.
- NPC anchor는 좌표·회전 복사 참조이며 NPC를 자동 추종하지 않는다. NPC 이동 뒤 Guide를 다시 Load하고 복사해야 게시할 수 있다. 잘못된 anchor를 도구 Load부터 거부하면 복구 UI가 막히므로 편집용 Validate는 경고, Save/Publish/CheckPublished는 거부로 구분한다. PowerShell helper는 동적 scope의 반복 변수와 충돌하지 않게 모드를 `$script:Mode`로 읽는다.
- NPC 배치를 비활성화할 때 상점 binding과 authored/viewer 미니맵 소비자도 확인한다. 서버 actor만 사라지고 정적 NPC 심볼이 남는 경우가 있으므로 미니맵은 enabled=false를 제외해야 한다.
- 전체 Size를 halfExtents로 바꿀 때 float의 `0.02 / 2`가 저장 schema 최소 `0.01`보다 작아질 수 있다. JSON double 영역에서 최소·최대값을 보정한다. 상세 창을 닫으면 그 창에서 시작한 one-shot picking도 취소한다.


### SourceCharacter program 번호의 독립 브랜치 충돌

같은 숫자의 Base/Light 함수가 충돌하면 이름만 보고 한쪽을 선택하지 않는다. family와 원본
shader identity를 비교하고 서로 다른 재질이면 한쪽을 사용하지 않는 번호로 옮긴다.
CPU packing, named-vector patch, Base/Light 함수와 dispatch, registry를 함께 바꾸고
기존 함수 본문·Engine/Client mirror를 대조한다. public header가 단일 CPP/generated INL로
분리된 브랜치에는 새 packing을 INL에 연결하며 거대한 inline header를 되살리지 않는다.
구체적인 병합 근거는 [PR 통합 결과](09-27/2026-09-27_MAIN_PR465_PR467_INTEGRATION_RESULT.md)에 둔다.

### 초상 요청 간 viewport와 forward coverage

여러 초상을 같은 G-buffer로 그릴 때마다 full-resolution viewport와 원래 DSV를 복구한다.
Begin/End_MRT는 viewport를 초기화하지 않으며 마지막 UI resolve는 작은 viewport와 null DSV를
남긴다. Character의 pass0은 source translucent hair/eyelash를 건너뛰므로 field의
NONLIGHT/BLEND pass도 초상에 연결해야 한다. 이들 픽셀은 G-buffer marker가 없으므로
초상 alpha를 depth marker만으로 만들지 말고 SceneHDR alpha-over coverage를 사용한다.
근거: `.md/GB/09-27/2026-09-27_PORTRAIT_AND_SHIP_REVIEW_FIX_RESULT.md`.

### 주사위 속박과 snapshot 전체 송신 중단

`isPatternBound`는 입력 이동·스킬 차단이고 `isCombatReady`는 카드 피격 가능 상태다.
두 상태가 동시에 true인 정상 속박을 wire validator가 거절하면 그 플레이어만이 아니라
방 전체 snapshot encode가 실패해 자유 플레이어도 멎고 속박 표시도 갱신되지 않는다.
속박 검증은 N명 중 N-1명 선택만 확인하지 말고 실제 Complete Play 진입 뒤 두 session의
snapshot tick 진행과 자유 플레이어의 실제 이동까지 확인한다. GUIDE_AI는 인간 인원에서 제외한다.
근거: `.md/GB/09-27/2026-09-27_KOUKU_HEALTH_STAGGER_BARS_IMPLEMENTATION_RESULT.md`.


### UI 게이지의 색 덮임과 반복 크기 조절

- 특정 UI 바만 색이 다르면 전역 rendering option보다 실제 PNG alpha, slot 생성 순서와
  같은 sort layer의 겹침을 먼저 확인한다. 불투명 보라 track이 주황 fill 뒤에 그려지면
  fill 수치와 texture가 정상이어도 보이지 않는다. 빈 영역이 필요한 바는 빈 배경과 fill만 표시한다.
- 크기와 위치를 따로 편집할 때는 원본 rect를 보관하고 공통 frame 중심 기준 scale 후
  X/Y offset을 적용한다. runtime rect를 다시 축소하면 반복 편집·Reload마다 크기가 누적된다.
  저장 배율도 최신 디스크의 필드별 병합·충돌·공유 lock과 원자 교체에 포함한다.
- 광대 얼굴의 작은 바는 플레이어 광기 게이지다. 보스 머리 위 HP의 앵커 결함으로 진단하기
  전에 stable slot과 presentation owner를 식별한다.
- 근거: `09-27/2026-09-27_KOUKU_AUDIO_MARIO_HUD_POLISH_IMPLEMENTATION_RESULT.md` G04 후속.

### 장판의 시작 앵커와 반복 사운드 수명

- 보스 위치에서 발생한 고정 장판은 `BOSS + followBoss=false`로 시작 pose를 보존한다.
  MAP으로만 바꾸면 local offset이 절대 맵 좌표가 된다. Collider는 해당 이펙트의
  `anchorPresentationOccurrenceId`를 공유해야 Client 표시와 Server `captureStartMs`가 일치한다.
- 사운드가 빠졌다고 애니메이션 notify만 검사하지 않는다. 원본 Projectile의 AkEvent와
  실제 저장 occurrence/group 내부 clock을 대조한다. 사각 그룹 앞에 미사일 시간을 넣을 때
  내부 폭발 clock 증가와 occurrence 시작 감소를 함께 적용해 사용자 폭발 시간을 유지한다.
- 시작음+지연된 반복음으로 구성된 원본 event 전체를 loop하면 시작음이 반복된다.
  각 native layer를 분리하고 반복 lane에만 World `loopToDuration`을 켠다. 수명뿐 아니라
  motion 교체/NEXT/외부 cutoff에서도 이전 loop를 정리하고 one-shot tail과 구분한다.
- Movie 헤어의 bind/rest·finite 성공은 포즈 중 머리 형태 보존을 뜻하지 않는다. 실제 움직임
  key 구간과 중간 시각을 head 기준으로 비교한다. 긴 마지막 hold만 샘플하면 변형을 놓친다.
  호환되지 않는 긴 모발 애니메이션을 대신해 head 추종형 외형을 선택한 경우에는 원본
  hair dynamics 복원과 구분하고, 정점·재질·Movie clip 보존과 실제 소비자 연결을 확인한다.
- 근거: `09-27/2026-09-27_KOUKU_AUDIO_MARIO_HUD_POLISH_IMPLEMENTATION_RESULT.md`.


### 편집 창 Present·정적 맵·baked trail의 반복 비용

- multi-viewport의 `Present(0,0)`은 VSync가0이어도 같은 device의 제출 queue에서 기다릴 수 있다. GPU elapsed와 CPU frame이 같다는 사실을 GPU 연산 포화로 해석하지 않는다. viewport별 ID/rect/HRESULT와 busy를 함께 기록하고, nonblocking secondary 제출은 순서를 순환해 특정 창의 지속적인 후순위 밀림을 막는다.
- 정적 그림자 cache hit라도 매 caster의 불변 surface/profile을 다시 해석하면 비용이 남는다. material admission만 stage에 준비하고 morph/override/transform/visibility/source option은 매번 확인한다. frustum 재사용은 최종 camera revision과 transform, reject hysteresis까지 일치해야 한다.
- baked trail의 점마다 같은 source module/distribution을 문자열로 찾지 않는다. 한 호출 동안 불변인 조회만 끌어올리고 per-point time/random/WORLD_SAMPLE 평가와 출력 bit 동등성을 검사한다.
- Effect Tool 단일 Product와 패턴 전체가 다르면 shader 종류를 바꾸기 전에 stable cue 목록과 각 asset의 visible element를 비교한다. 저작 Product와 원본 복원 Product의 동시 연결을 공통 asset 삭제로 해결하지 않는다.
- 근거: `09-27/2026-09-27_ARENA_WORKBENCH_PERFORMANCE_RESULT.md`.

### 전조·폭발 분리의 원본 근거와 owner-hit 시계

- 현재 손저작 전조·폭발의 element를 나누거나 조합한 결과는 `PROJECT_AUTHORED`다. 같은 mesh/material을 쓰거나 이름·이미지가 비슷하다는 사실로 원본 FullRestore라고 부르지 않는다. 원본 ParticleSystem과 실제 owner·호출 시각의 연결이 없으면 그 경계를 명시하고 저작값을 보존한다.
- owner-hit/지연 연쇄로 바꿀 때 active 안의 고정 시각 전조·폭발도 분리한다. 별도 event와 effect ID를 join하고 local preview는 stage age가 아닌 동일 global timeline clock으로 armed/hit·pause/seek를 평가한다. terminal에는 active/armed handle을 정리한다.
- 표시 수명을 늘릴 때 `detail.timing/particle`만 수정하면 sourceRecipe의 Required emitter duration 또는 Lifetime distribution이 먼저 끝날 수 있다. 실제 마지막 연쇄 시각 이후의 native sample까지 확인하고 owner terminal stop과 resource particle 수명을 구분한다.
- 근거: `09-27/2026-09-27_ARENA_WORKBENCH_PERFORMANCE_RESULT.md` G06.


### 2026-09-28 유령 발탄: 각진 면과 백색 반사를 투명도 문제로 묶지 않기

- native84/pass16은 원본 alpha를 우회해 가시성을 확보한 정책이다. 맵 청록 pixel mask를 공유한다는 뜻이 아니며, 실제 호출 경로에 없는 map discard를 수정하지 않는다.
- 설치 ghost WINT1.0의 모든 14,472 triangles에 face normal이 저장돼 있었다. 원본 glTF corner의 position/UV를 대응시킨 뒤 `restore_skinned_source_basis.py`로 NORMAL/TANGENT/sign만 복구한다. shader에서 normal을 normalize하거나 임의 평균내는 방식으로 원본 smooth basis를 대신하지 않는다. WINT1.5의 sign을 실제 native decoder/animated input까지 확인한다.
- native84 passValues[4]는 `specular * w + rgb` 계약이다. `(1,1,1,1)`은 피부 specular_color를0으로 해도 흰 반사를 남긴다. 해당 program의 중립값은 `(0,0,0,1)`이며 generator와 Engine/Client Base/Light mirror를 함께 맞춘다. 다른 native program의 상수는 별도 ABI 근거 없이 바꾸지 않는다.
- 원본 specular tint의 복원과 사용자가 요청한 피부 무반사 조정은 구분한다. 이번 skin slot RGB0은 PROJECT_AUTHORED이며 diffuse/cloud/rim을 끄는 설정이 아니다. 모델 복구·후보 GPU 검증·설치·사용자 화면 판정은 각각 기록한다.
- 근거와 적용 상태: [유령 발탄 결과 G07](09-22/2026-09-22_GHOST_VALTAN_CHARACTER_SELECT_PREVIEW_RESULT.md).

### V1 Effect Tool 그룹 표시의 반복 문자열 포맷

- 큰 source Effect를 열어 둔 것만으로 Authoring/Detail 창이 느려지면 저장·parse를 원인으로 추측하지 말고 각 Render 소비자를 추적한다. 동일 attachment의16float를 모든 Element에서 hexfloat로 포맷하는 비용은 stable ID 선형 검색보다 클 수 있다. member 검색만 제거한 후보와 실제 key 생성 비용을 분리해 측정한다.
- `Build_AttachmentElementGroups`는 호출 안의 typed attachment/manual group/inheritance identity와 member view를 재사용한다. 기존 문자열 key와 첫 등장 순서, 양수·음수0, 그룹 중심·회전 권한은 보존한다. 포인터와 string_view를 다음 frame이나 문서 교체 뒤까지 보관하지 않는다. Source/Detail 편집을 알지 못하는 cross-frame cache로 바꾸지 않는다.
- 작은 cinematic Effect와 수백 Element의 source Effect를 함께 비교한다. 선택 asset ID가 없는 캡처에 임의 fixture ID를 붙이거나 helper 국소 시간을 전체 FPS 개선으로 보고하지 않는다. 근거는 `09-27/2026-09-27_ARENA_WORKBENCH_PERFORMANCE_RESULT.md` G10이다.

### 같은 frame의 Timeline 변경과 Detail 초기화

- Sequencer가 typed owner를 바꾸고 Detail identity를 비웠어도 다른 pane은 frame 시작의 immutable view를 가지고 있을 수 있다. 뒤 pane이 구값으로 draft를 다시 채우면 다음 Save의 pending Detail 적용이 정상 trim을 되돌린다. mutation이 허용되고 owner generation이 바뀐 frame에서만 Detail용 local 최신 snapshot을 사용하며 다른 pane의 공유 포인터는 교체하지 않는다. idle 재복사, readonly 조회 차단, 실제 사용자 Detail 값의 무조건 초기화를 피한다.
- Animation clip 삭제의 Sound cascade를 V2까지 처리한 것으로 간주하지 않는다. exact V2 clip reference가 있으면 안정 ID·baseline·dirty/revision까지 복원 가능한 multiowner transaction이 필요하다. 현재 Valtan은 그 transaction 없이 V2를 일부 삭제하지 않고 binding ID와 함께 삭제를 선행 거절한다. Animation·Sound·V2의 변경0과 기존 자료 보존을 확인한다. 근거는 `09-09/2026-09-09_VALTAN_COMPOSITION_AUTHORING_PARITY_RESULT.md` G30이다.


### Composition seek와 실시간 fixed-step 예산을 구분하기

- 이전 stage 끝을 몇 번 샘플링하여 NATURAL tail을 재구성할 때 큰 delta를 일반 Effect Advance로 전달하지 않는다. 실시간 MAX_CATCH_UP_STEPS 제한은 입자 적분을 일부만 실행할 수 있는데 서비스의 요청 elapsed clock은 이미 목적 시각으로 이동해 입자 상태와 표시 시간이 어긋난다. Get_FixedStepClockSeconds도 남은 accumulator를 포함하므로 그 숫자만 비교하면 놓친다.
- Valtan의 명시 seek/reset 재구성은 AnimationTool → local boss → Sample_LocalBossPreview로 rebuild 의미를 전달하여 기존 Set_SampleTime/Seek를 사용한다. 일반 forward, 자연 stage 경계와 pause/resume는 증분 경로를 유지한다. 큰 실시간 frame을 모두 seek로 바꾸는 성능 우회는 피한다.
- 단독 Playback.Seek가 정상이어도 실제 AnimationTool/owner/service를 함께 확인한다. 같은 시점의 particle stable ID·count·world·alpha를 비교하고, target-follow 회전과 cinematic suppression은 별도로 측정한다. 근거는 `09-27/2026-09-27_ARENA_WORKBENCH_PERFORMANCE_RESULT.md` G13이다.


### Full Restore 반복 저작과 고정 root의 절대 시계

Full Restore의 원본 animationClips/clipDuration/source-stage receipt를 사용자 반복 길이로 덮어쓰지 않는다. standalone 편집용 반복은 검증된 optional authoredPreview(PROJECT_AUTHORED)로 따로 저장하고 해당 소비자만 opt-in한다. builder는 이 저작 override를 검증·보존하며 일반 pattern source matching은 원본 metadata를 계속 사용한다. 실제 패턴에 연결된 효과의 개수를 바꾸면 cue 시작, 반복 모델 phase, bodyVisibility를 함께 검사한다.

고정 world root를 외부 절대 시각으로 매frame 표본화할 때 매번 particle Seek를 호출하면 누적 age 전체를 반복 적분한다. 기존 fixed-step history provider의 첫표본/불연속 Seek와 정상 전진/hold 분기를 사용한다. 상수 root provider는 실제로 고정된 오브젝트에만 사용하고 움직이는 owner의 transform 역사를 대신하지 않는다.


### 2026-09-28 저작 V2와 Sound의 역할·시각 receipt
새로 연결한 V2 Library 그룹은 EffectRoles exact coverage에도 등록해야 한다. 역할 누락을 gameplay hit 추가나 저장한 cue 시각 원복으로 메우지 않는다. 공격 resource를 피해 없는 PROJECT_AUTHORED 연출에 재사용할 때는 shared 역할을 바꾸지 않고 검토한 binding/scope/resource/clock의 exact presentation-only receipt로 제한한다. 의도적으로 편집한 Sound와 contact 차이는 해당 scope의 hit offsets, Sound payload와 실제 wall clock를 결정하는 animation occurrence 전체를 함께 고정한다. Sound source 시각만 고정하면 playRate·선행 clip 길이 변경이 예외를 통해 새 timing drift를 숨긴다. unknown/stale receipt와 이후 drift는 계속 거절한다.

### 생성 돌과 폭발 파편의 복구 표면 연결

standing rock과 explosion debris가 다른 source material을 사용할 수 있다. 생성 돌을 복원해도 별도 hit 문서와 내용을 복사한 편집용 composite는 자동 갱신되지 않는다. 같은 표면을 요청받으면 실제 WModel geometry·UV/N/T·sampler와 material dynamic 채널을 대조한다. 기둥 mesh를 작은 파편에 통째로 치환해 크기·실루엣을 바꾸지 않고, 기존 파편 수명·탄도·색·저작 파동을 보존한 표면 연결과 원본 폭발 전체 복원 주장을 구분한다. 구체 적용과 native 비교는 09-28 VALTAN_PR_INTEGRATION RESULT에 기록한다.

### 발탄 Create 이후 STALE와 끝 자세 유지 구간의 Append

- 데이터 전용 Composition은 Animation preview의 `m_AssetName`을 보장하지 않는다. 생성 후
  intake 재조회는 명시적 Valtan source를 사용한다. 보조 preview 이름이 비었다고 정상 source
  commit의 created event를 삼키면 목록은 이전 revision에 남고 Append까지 읽기 전용이 된다.
- 편집 가능 여부는 검증한 source owner로 판단하고 Server 재생 준비는 Product inventory로
  판단한다. 빈 pattern은 실제 `animation.mode=NONE`으로 저장하고 clip을 꾸며 넣지 않는다.
- finite clip 합보다 Stage가 길면 끝 자세 유지 구간이다. Append는 clip 합 위치에서 시작하고
  Stage 길이는 기존값과 새 합 중 큰 값으로 유지한다. 이전 마지막 clip의 파생 hold 길이도
  다시 계산해야 timeline에서 실제 clip 사이에 가짜 빈 구간이 생기지 않는다.
- 일반 Collider는 Stage의 단일 geometry와 pulse schedule이다. Ctrl+C/V로 같은 geometry의
  시점을 복제할 때 피해·반응·anchor를 함께 비교하며 중복 시점과 범위 밖 시점은 전체 거부한다.

### 발탄 원본 시퀀스와 독립 Effect 저작 입력

- 원본 .clipcuts의 ms와 설치 CModel의 cooked tick/30 길이는 다를 수 있다. 예를 들어
  mesh_idle_battle_1은 원본2333ms와 runtime2233ms다. Append는 non-loop 한 항목을
  native window 안으로 제한하고 조정 내용을 표시한다. 원본의 중복 항목은 보존하며 파일명에
  _loop가 없는 긴 cut을 임의 반복으로 늘리지 않는다. 개별 clip Append 검사만으로 원본
  Sequence 전체 Append 성공을 대신하지 않는다. Boss 한 Stage의 clip 상한은256이며 reader,
  editor, source와 publisher가 같아야 한다. player skill 상한은 별개다.
- STAGE_CLOCK Effect는 clipOccurrenceId와 mappingBasis가 없는 정식 독립 invocation이다.
  기존 reader가 이를 지원해도 Add/Update/Save serializer가 clip mirror를 요구하면 실제 저작은
  막힌다. 빈 Stage의 Add→Box Detail→Ctrl+C/V→Save/reopen→Product reader를 함께 확인한다.
  stageEndMs는 optional이고 cue_end에만 수치가 필요하다. once tail의 끝은 Stage를 넘어도
  600000ms 이내로 보존한다. timing clock과 spatial anchor는 별도 계약이다.
- map anchor는 snapshot과 identity world root를 사용한다. Boss root snapshot을 Map으로
  표시하면 실제 배치 좌표가 달라진다. 기존 cue ID와 연결은 유지하고 사용자가 선택한 cue만
  변경한다. 여러 owner의 붙여넣기는 Balance/Sound/V2 draft transaction으로 한 번에 원복한다.

- Animation이 없는 seed의 Product 제외 판정은 독립 V1뿐 아니라 실제 Pattern/Stage/Action에
  연결된 V2 STAGE binding과 Collider도 포함한다. 마지막 content 제거 시 이전 Product 행을
  unmanaged legacy로 남기지 않고 기존 retirement 경로로 제거하며 Source seed는 유지한다.
- Stage topology 변경은 별도 PatternShake owner도 살핀다. Stage copy/delete와 Animation
  delete는 Shake의 clip/action ID와 원본 payload를 함께 처리하고 비활성 source 행을 버리지
  않는다. Sound/Shake/V2 baseline/candidate는 기존 canonical writer의 같은 CAS에 포함한다.
  source_manifest에도 Shake hash가 들어가야 Shake만 저장해도 revision이 바뀐다.
- 영역 선택/혼합 box 변경에서 UI 선택만 복원하면 충분하지 않다. 실패 시 Balance, Sound,
  V2, Shake와 dirty/detail 상태를 함께 원복한다. Animation은 기존 연속 slot 경계에서 상대
  간격을 보존하는 이동만 허용하며 Stage 재배치는 각 Stage 내부 시계를 변경하지 않는다.


### 발탄 Resources의 삭제된 대상과 Append 끝 위치

- 선택 Stage와 별도 cached ResourceTargetStageId가 달라지면 삭제된 STEP_01이 Resources를
  계속 잠글 수 있다. Append의 대상 정책은 화면에 명시하고 현재 Pattern에서 다시 resolve한다.
  선택 Stage 안 편집과 패턴 맨 끝 추가를 같은 Append 명령으로 섞지 않는다.
- Earlier/Later는 밀리초 delta를 기존 drag midpoint에 더하는 기능이 아니다. 선택 Animation의
  소속 Stage를 중복 제거하고 각 선택 구간을 인접한 미선택 Stage 너머로 한 칸 옮긴다.
  여러 이동은 단일 owner transaction으로 처리하고 실패 시 순서·세대·선택을 유지한다.
- timeline 버튼이 source transaction과 preview 갱신을 호출하면 그 프레임의 기존 Pattern/Stage
  포인터를 계속 그리지 않는다. mutation 뒤 다음 프레임에서 새 view를 조회한다.


### 기존 encounter Pattern과 manual audition의 Append를 구분하기

- 새 Pattern만 생성하는 native fixture로 기존 encounter Pattern의 Append를 검증했다고
  기록하지 않는다. manual audition의 새 Stage 삽입이 성공해도 정본 encounter의 C++ Save와
  Python writer는 같은 topology op를 거부할 수 있다. 실제 사용자 Pattern ID로 owner의
  Append → BuildValtanDraftPatch → SourceSave → split/reopen → Product를 이어서 확인한다.
- 정본의 마지막 finite Animation 목록은 기존 typed owner로 확장할 수 있다. Stage graph와
  Motion/World/Counter 참조를 바꾸는 manual topology admission을 일괄 완화하지 않는다.
  기존 Stage의 ENTER event·hit offset·clip ID는 그대로 유지하고 끝 시계만 연장한다.
- 마지막 Stage에 독립 Effect를 붙이며 길이를 늘리면 finite EXACT Animation의 end policy를
  HOLD_LAST_POSE로 맞춘다. 수명 증가만 저장하고 EXACT를 남기면 source validation이 거부한다.

### 발탄 Box 삭제·검색 drag·조건부 tail 표시의 소비자 일치

- 마지막 Animation Box 삭제를 manual audition에만 허용하면 기존 정본 Stage에서 삭제도
  재추가도 막힌다. non-WAIT의 Animation NONE, C++ draft admission, typed patch, Python
  writer와 재로드를 함께 확인한다. WAIT의 빈 Stage 계약은 별도로 유지한다.
- clip에 연결된 V1 cue를 먼저 삭제했어도 삭제 전 Stage/ProductCues snapshot으로 다음
  의존성을 검사하면 자기 변경을 dangling 참조로 오판한다. 같은 transaction 안에서 최신
  Stage와 cue 목록을 다시 읽고 검사한다. Sound·Shake·V2 실패 때도 앞선 삭제를 원복한다.
- ImGui `BeginDragDropSource()` 기본 경로는 직전 item의 stable ID가 필요하다. 검색 InputText가
  활성화된 프레임에 TextWrapped/TextDisabled 뒤에서 호출하면 ID 0 assert가 발생할 수 있다.
  Summon/Logic의 source는 기존 Copy Resource 버튼 바로 뒤에 연결한다. 전역 helper에
  SourceAllowNullID를 추가해 행 identity 문제를 우회하지 않는다.
- 저장된 후속 clip이 안 보이면 Product 누락과 선택한 preview 분기를 구분한다. 잡기 성공의
  ANY_PLAYER_GRABBED 뒤 Stage는 Normal/TIMEOUT 경로에서 의도적으로 빠진다. shared path
  resolver, 초기 Pattern 선택, 경로 메뉴, Play Selected Stage, V1/V2 Effect 진입을 같은
  Capture Success 경로로 연결하되 Server 분기를 선형 재생으로 바꾸지 않는다. 같은 Pattern에서
  사용자가 선택한 Normal은 자동으로 다시 성공 경로로 바꾸지 않는다.
- Stage motion/aim을 추가하면 source projection뿐 아니라 Product reference, strict 비교,
  source overlay, Balance draft/save와 Effect authoring admission까지 연결한다. nearest aim의
  center-follow는 owner Stage와 앞선/현재 center motion을 함께 검증한다. Pattern 전체의
  locked-random/snapshot 허용 조건을 느슨하게 만들어 다른 Stage에 추적이 번지게 하지 않는다.
- V1 Stage-clock cue의 `stageEndMs`는 Composition reader와 joined/detached 정규화에도
  보존한다. canonical Source/Product 검증만 통과해도 보조 reader가 필드를 모르거나 natural만
  허용하면 정상 게시를 막는다. `once`와 `cue_end`의 유한 종료 범위를 유지하고, natural의
  종료 없음 및 clip-clock의 별도 sourceEndMs 계약을 함께 검사한다. target-follow admission도
  동일한 Stage aim·앞선/현재 center motion 조건을 소비해야 한다.
- Effect V2 `CLIP_OCCURRENCE.startMs`를 occurrence 내부 상대 시각으로 바꾸지 않는다.
  native와 Pattern Sound는 원본 source 시각을 저장하고 `(startMs-sourceStartMs)/playRate`에
  앞선 clip의 wall 길이를 더한다. Python validator·legacy migration도 이 계약을 유지한다.
  source cut이 0인 예제만으로는 잘못된 상대 시계가 드러나지 않는다. nonzero cut의 시작
  포함/끝 제외, 이전 clip 누적 시각, 서로 다른 재생 배율과 실제 반복 펼침을 함께 확인한다.

### Valtan 원본 장판·포탈 재질과 native shader bucket

- 신규 source decal은 재질 이름이나 PS ID만으로 기존 adapter에 넣지 않는다. 실제 VS/PS, instruction hash, binding hash와 사용 CB 행·texture slot이 기존 adapter 계약과 일치하는지 검사한다. 사용하지 않던 padding 행을 읽기 시작한 경우도 거부한다.
- 새 native shader bucket을 추가하면 Mesh/Particle 선택 wrapper·실행 표·프로젝트뿐 아니라 공용 Decal/Trail의 bucket 상한도 함께 갱신한다. source HLSL 컴파일만으로 설치 CSO나 실제 장판 표시가 성공했다고 기록하지 않는다.
- 원본 boss Sound event가 설치 catalog에 없으면 유사 event로 복구 완료를 대신하지 않는다. 원본 Wwise Play graph의 layer·random weight·delay를 보존한 offline render와 stable event catalog merge를 확인한다.

- Gameplay publisher의 InputOverlayRoot 검증 성공만으로 최종 canonical publish 성공을
  선언하지 않는다. 설치 전 동일한 전체 후보에 Project-ValtanPatternMaster Validate의
  split/Product·clip-template·hit/presentation 검사와 Composition reader를 적용한다.
  검사용 전체 Data를 반복 복사하지 말고 read-only mirror와 분리한 변경 overlay를 사용한다.
  실제 게시 성공과 실행 중 Server의 재시작·사용자 화면 확인은 별도로 기록한다.

### Actor Catalog와 패턴 판독기의 서로 다른 입장 경계

BossCatalog combatObjectVisuals는 보스당 visual 정의 배열이며 현재 상한은32개다. 동시 생성
instance나 snapshot 최대 개수와 혼동하지 않는다. CActorCatalog·Valtan authoring·Gameplay
publisher에 같은 정의 상한을 적용한다. PatternTree가18개를 읽고 Server catalog가 통과해도
Loader가 먼저 부르는 전체 ActorCatalog Initialize의16개 제한이 남아 있으면 입장 전에 실패한다.
카탈로그 정의를 늘릴 때 전체5개 Actor catalog의 실제 Initialize를 실행하고 한도 바로 위 입력을
거부하는지도 확인한다. 상세 parse 실패를 포괄 contract mismatch 문구로 덮어쓰지 않는다.


### 발탄 새 데칼과 ARMED/HIT의 파괴 시계

- 새 source decal을 연결할 때 특정 native ID allowlist에 actor 제외를 추가하는 방식으로
  끝내지 않는다. V1/V2 공통 projected receiver가 GBuffer marker0/5의 skinned bit와 native
  actor 표식을 소비해야 한다. 정적 Map 및 sprite/mesh/trail과 source projection volume은
  별도 계약이다. shader compile·GPU 수치 검증과 사용자 화면 확인을 구분한다.
- ownerHitChain 내부0ms/외부1500ms는 준비 시작 차이다. 원본 전체 Off asset은 ARMED에서
  한 번 시작하고, hit.atMs의 준비 duration1820ms 뒤 피해·소리를 낸다. direct 바위도 준비를
  생략하지 않는다. armedEffectOwnsTerminal이 HIT 중복 시작을 막으며 HIT에서 준비를 다시
  시작하거나 source seek로 준비 구간을 자르지 않는다. Preview도 고정된 보스 yaw와 같은
  collider를 소비하며 현재 타겟을 다시 향해 판정을 돌리지 않는다.
- PublishCandidate에 추가 draft patch가 없다는 사실은 Product가 최신이라는 뜻이 아니다.
  저장한 split source에서 먼저 projection하고 의미가 같은 Product만 byte 재사용한다.
  provenance receipt는 최종 선택된 Product 값에 동기화한다. 재현·검증은09-29 발탄 결과를 따른다.
- 회전하는 피자 경고는 cue root뿐 아니라 내부 요소의 snapshot attachment도 확인한다.
  source basis를 한 번 보존한 뒤 mutable root를 소비해야 장판과 후속 붉은 영역이 같은
  sector yaw를 따른다. 본 부착의 반시계 보정은 설치 모델의 축으로 계산하되 중간 particle world만
  검사하지 않는다. 고정축 Sprite는 그 회전과 별개로 최종 quad를 다시 만들 수 있어 위치만 따라갈 수 있다.
  필요한 carrier에만 `followEmitterAxisRotation`을 연결하고 실제 bone import basis와 socket의 평면 보정을
  함께 검증한다. 원점·고정 yaw만 사용하지 말고 비영점 보스 위치에서 이동과 회전을 함께 바꿔 최종 quad의
  꼭짓점·텍스처 전방·법선이 보스 pivot을 따르는지 확인한다. source/notify 보정은 한 번만 적용한다.

### 같은 보스 entity의 부활 HP와 정지 장판

부활이 archetype 교체 없이 phase만 바꾸는 경우 HUD의 최대 줄수 profile도 phase를 소비해야 한다.
HP/maxHP는 snapshot을 사용하고 maxHP 전환을 일반 피격으로 처리하지 않는다. 부활 HP 검사는
Respawn ENTER가 아니라 실제 완료 경계에서 수행한다. 정지 PER_ALIVE_PLAYER SINGLE 장판은
플레이어 위치에 유효 surface가 있는지로 배치한다. walkable bit가 없다는 이유만으로 바닥 있는
위치를 실패시키지 않으며 RADIAL·이동·void 거부에는 같은 예외를 전파하지 않는다.
이펙트·사운드의 sourceStart는 stage local과 다를 수 있으므로 source clock에서 wall clock으로
변환한 뒤 범위를 검증한다. 원본과 저장된 PROJECT_AUTHORED 선택을 구분한다.

### Encounter optional 필드와 실제 Level 입장 파서

ValtanPatternTree 에디터와 Server Catalog가 새 optional 필드를 읽어도
Level_ValtanArena가 별도로 호출하는 CEncounterPatternReference::Load의 exact-object
검사가 오래된 상태면 매 입장마다 초기화가 실패한다. schema 확장 시 field 검색으로
모든 실제 소비자를 확인하고, 설치된 Product 및 실제 CProjectDataRoot로 해석한
전체 Encounter를 입장 파서에 통과시킨다. 이어지는 camera document Load와
controller Initialize도 검사한다. 에디터의 단일 함수 검사나 Server 입장 검증을
Client Level 전체 초기화 성공으로 기록하지 않는다. malformed 문서의 Load 실패가
기존 commit 상태를 보존하는지 함께 확인한다.

- Source→Product의 수동 owner 파생값은 publisher와 Client strict join이 동일해야 한다.
  manual repeatPolicy.limit가 저장되어 있어도 자동 전투의 maximumConsecutiveUses는0이다.
  원본을 억지로0으로 고치거나 parity를 끄지 말고 실제 전체 Tree와 Play inventory로 확인한다.
- Encounter stage 필드 확장은 Gameplay publisher뿐 아니라 World destruction publisher의
  exact-field 검사에도 적용한다. 공통 타입·범위 계약을 맞추고 실제 Full DataOnly 완료를
  확인한다. 개별 Gameplay/Composition 게시 성공을 전체 domain 게시 성공으로 대신하지 않는다.

### 보스 기믹 게이지와 갈고리 하차 직후 이동

- 움직이는 보스의 무력화 게이지는 고정 HUD rect+offset으로 붙이지 않는다. 작은 HP와 같은
  exact entity의 실제 머리 anchor와 최종 카메라 projection을 매 프레임 소비한다. 예전 화면
  고정 offset을 새 머리 기준 offset으로 재해석하지 않고 optional 새 키 기본0으로 분리한다.
- Space/root motion의 navigation clamp 뒤 body collision이 접선 이동을 만들 수 있다. 최종
  충돌 결과의 전체 경로도 지면 검증 후 commit하며, 거절된 tick은 마지막 안전 좌표를 보존한다.
  그 뒤 ground Y가 바뀌면 수직 보정 구간과 최종 player volume도 collision으로 재검증한다.
- 3관문 WORLD hook은 정상 ascent뿐 아니라 deadline·owner 소멸·중단도 공통 해제에서
  walkable floor와 충돌을 검사한 뒤 입력을 푼다. 근처 안전 바닥이 없으면 기존 관문 시작점의
  검증된 바닥으로 복귀하며, 그것도 없으면 attachment/input lock을 유지한다. 실제 Space
  입력의 발생 시각과 이 방어 경로 검증은 구분한다.
  cinematic의 일괄 action reset도 이 실패 lock을 지우면 안 된다. gate scope·owner를 바꾸기
  전에 현재 관문 기준으로 모든 hook 해제를 stage하고, 미확보 지면이면 진입을 거절한다.
- 검증 근거는 `09-29/2026-09-29_KOUKU_STAGGER_ANCHOR_HOOK_LANDING_RESULT.md`에 둔다.


### 평소 장착 무기와 공격 이펙트의 중복 모델

- 같은 무기를 작게 들고 공격 때 크게 띄우면 source mesh particle의 본체·overlay도 찾는다.
  geometry cook basis와 preScale·StartSize·socket scale·실제 body/root 축소를 모두 비교하고,
  기존 장착 모델을 요청 크기로 맞춘 뒤 중복 geometry만 비활성화한다. 타격 particle과 recipe는
  보존한다. CPU 치수 일치를 최종 손 부착·GPU 화면 확인으로 기록하지 않는다.
- 실제 적용·검증은 `09-29/2026-09-29_MARIO_HELD_HAMMER_RESULT.md`를 따른다.

### 색별 기믹 표식은 원본 variant의 재질·텍스처까지 대조한다

- base ParticleSystem과 색별 suffix를 전체 표식의 대체 관계로 가정하지 않는다.
  마리오 원본 Offering buff는 공통 광대 얼굴에 Color를 넣고 색별 도형을 동시에 재생한다.
  face를 흰색으로 고정하면 이 색상 override가 누락된다. 서버 random/snapshot 성공은
  실제 얼굴 색상을 증명하지 않는다. ColorStr 같은 추가 배율도 emitter별 소비자를 확인한다.
- 원본 buff/action의 동시 시스템·인스턴스 override와 emitter→MIC→texture를 추적한다. 원본 흰 마스크,
  HDR 색상값, HUD buff atlas 아이콘을 구분하고 PNG 추출만으로 게임 표시 수정을 완료 처리하지 않는다.
- 원본 추출 근거와 현재 미반영 경계는
  `09-29/2026-09-29_MARIO_MARKER_SOURCE_EXTRACTION_RESULT.md`에 기록한다.
- 목표색과 진행 count는 머리 표식 대상과 분리해 입장자 snapshot으로 전달한다. 진행 알림은
  일치 count 증가에만 시작하고 반복 snapshot·다른 색 공 파괴·진입 기준값으로 재생하지 않는다.
  UILabelFont의 SpriteBatch는 premultiplied alpha이므로 fade에서 RGB와 alpha를 함께 줄인다.

### 참가자별 Effect 차단은 실제 게시 occurrence까지 연결한다

저작 LogicOccurrences의 triggerKind로만 Client audience를 판정하면 Product reader가
해당 logic을 받지 않는 경로에서 조건이 영원히 false가 될 수 있다. 게시기가 정확한
occurrence의 audience 의미를 전달하고 실제 Product parser가 소비하는지 대조한다.
쿠크 Mario phase2의 suppressLocalMario는 커튼 occurrence 하나에만 적용하며
전체 패턴·Server gameplay·다른 파티원의 재생을 막지 않는다. 이미 생성된 핸들 종료,
늦은 참가 snapshot과 복귀 뒤 재생 방지까지 검사한다. bool 조건 fixture만 통과한 것을
Product 연결 완료로 기록하지 않는다. 근거는09-29 RAID_MOVIE_INTEGRATION_RESULT다.

### 주기 생성물의 접촉 소멸은 피해 숫자와 별도 reliable identity를 쓴다

서버 피해가 확정돼도 HUD의 transient damage history만 보고 공의 생성을 끝내면
피해 합산·흡수·snapshot 병합에서 소멸과 이펙트를 놓칠 수 있다. 기존 typed lifecycle에
stable instance와 birth tick을 실어 해당 참가자의 정확한 세대만 종료한다.
처음 관측한 참가 snapshot 시각을 실제 입장 시각으로 가정하지 않으며 이전 관측 경계를
사용해 먼저 도착한 접촉을 보존한다. 현재 공의 생존과 확정 접촉 Effect는 별도로 판단해
수명 마지막 tick의 정상 접촉을 놓치지 않는다. 늦은 Effect는 원본 sample age/수명을
사용하고 오래된 신호로 다음 공을 종료하지 않는다. 같은 clock/interval을 Server와
Client에서 공유하며 자세한 검증은09-29 RAID_MOVIE_INTEGRATION_RESULT를 따른다.


### 배틀 아이템의 무력화·보호·원본 리소스

- 쿠크 `STAGGER_WINDOW`는 기존 HP 감소를 기여로 사용한다. HP0 회오리 수류탄은 별도 credit를
  실제 창의 threshold와 HUD·성공 판정에 연결해야 한다. typed gauge에만 넣으면 쿠크 창은 완료되지 않는다.
  최대치 1/3은 ceil(max/3)으로 계산해 세 번에 완료하며, 남은 양의 1/3이나 33%로 대체하지 않는다.
- 시간 정지/성부를 기존 invulnerability tick에 합치면 직접 wipe 소비자의 사전 skip까지 바뀔 수 있다.
  별도 보호 deadline을 공간 hit/공포 판정에 연결하고 명시적인 encounter wipe를 유지한다.
- 지속 buff Effect는 skill action tick과 수명을 공유하지 않는다. recipient+endTick occurrence로
  늦은 입장·교체의 경과 시간을 복원하고 만료/사망/이탈 시 pending와 active를 함께 정리한다.
- 원작 Ribbon `sheetspertrail`은 실제 값과 buffer capacity를 함께 소비한다. 회수 비행 원본의
  5장을 1로 줄이거나 해당 emitter를 삭제하지 말고 기존 trail renderer의 장별 geometry/draw를 사용한다.
- 배포 dependency는 `validate_effect_sources._collect_runtime_resource_ids`로 수집한다. `assetId`
  키만 훑으면 source native material의 별도 texture 참조와 공통 Character 리소스를 빠뜨린다.

### Movie 오디오의 시각 clock과 FMOD media position

Movie Update의 긴 frame clamp나 시작 defer는 FMOD 채널의 진행을 멈추지 않는다.
source delta250ms 이하라는 이유로 audio seek를 생략하면 slomo 구간에서 초 단위 drift가
남을 수 있다. 명시적 external Movie clock에서는 sourceStartMs+age와 실제 media cursor를
비교하고100ms 초과만 교정하며, 아직 sound box 안인데 채널이 끝났으면 현재 위치에서
복구한다. 기본 World/SFX의 독립 재생에는 이 정책을 전파하지 않는다.

Sound startMs는 timeline 배치, sourceStartMs는 WAV 내부 시작 위치다. 왼쪽 edge trim은
둘을 함께 바꾸고 body 이동은 sourceStartMs를 보존해야 한다. 음원을 자르기 전에 원본
AkEvent/Play delay와 PCM 선두 무음, animation finite clip 끝, Movie time dilation을
분리해 확인한다. 사용자가 관찰한 특정 시점을 근거 없이 고정 offset으로 저장하지 않는다.


### Movie ground foliage와 음원 합산 복원

일반 StaticMeshComponent 배치가 맞아도 InstancedFoliageActor의 native instance layer가 별도로
누락될 수 있다. actor/CDO visibility와 원본80-byte matrix/instance RNM bias를 대조하고 일반배치
개수만으로 전체지면 복원을 판정하지 않는다. 같은geometry라도 Area별MIC/staticset/lightmap은
원본에서 확인한다. foliageWind의 기존runtime과publisher 허용필드를 함께 검사한다.

WEM을 PCM16으로 변환하면 float peak가1을 넘는 원본파형이 합산 전에 잘릴 수 있다. WAV를
나중에float로바꿔도 복구되지 않으므로 원본media를float로 다시decode한다. Layer의 두음원은
중복오류로삭제하지 말고 원본bus gain/limiter까지 조사한다. offline dynamics bake는 해당동시
mix/time/gain에 한정되고 native wall-clock DSP와 동일하지 않다. 타이밍동기화와음질복원은
별개검증이며 현재박스 시작시점을임의보정하지 않는다.


### Visual Studio 프로젝트의 중복 항목은 MSBuild 성공과 별도로 검사한다

같은 파일을 같은 ItemType의 ClCompile/ClInclude에 두 번 등록하면 XML parse와 명령줄 빌드는
성공해도 Visual Studio의 프로젝트 다시 로드는 거부될 수 있다. 프로젝트 등록을 추가할 때
기존 Include 경로를 대소문자와 경로 구분자를 정규화해 비교하고, 동일 항목을 다시 추가하지
않는다. metadata가 같은 중복만 제거하며 원래 bigobj 설정과 실제 소스 파일은 보존한다.
프로젝트가 언로드된 상태에서 시작 설정을 저장하면 Server + Client 프로필에서도 Server가
빠져 있을 수 있으므로 두 상태를 별도로 확인한다. 실제 복구 증거는09-29 RAID_MOVIE_INTEGRATION_RESULT G13이다.

### Map catalog 게시 성공과 실제 Client admission을 함께 확인한다

mapassets의 위치 기반 토큰을 수정할 때 필드 이름과 CMapAssetCatalog parser의 순서를 대조한다.
SL03 foliage 생성기의 token13은 그림자 설정이 아니라 uvScale.x였다. 0을 저장하면 publisher가
배치·재질 검사를 통과해도 Client의 양수 UV 검증에서 전체 Area 로드를 거부한다. donor의 UV와
castsShadow 값을 보존하고, 변환 이후 양축 UV와 render profile 범위를 publisher에서도 검사한다.
source는 Load_Source, 게시본은 Load_Area로 실제 재질까지 읽어야 한다. 게시본에 Load_Source를
호출하면 authoring 경로 계약 오류가 나므로 검증 fixture의 호출 오류와 제품 오류를 구분한다.
수치·교체·게시 증거는09-25 FOUR_CLASS_SELECTION_MOVIES_IMPLEMENTATION_RESULT G13을 따른다.

### Movie 종료는 사용자가 지목한 Camera box 경계로 확인한다

Class Selection의 intro에는 첫 주요 연출 뒤 대기용 Camera box도 포함된다. repeatMovie=false로
전체 intro의 마지막 frame만 고정하면 첫 연출 뒤의 카메라 이동은 계속된다. 종료 기준 camera를
stable ID로 지정하고 그 source 끝을 기존 clock의 Movie time으로 변환한다. 다음 cut 진입을 막을
때는 double epsilon만 빼지 말고 실제 소비하는 float source의 직전 값을 역변환해야 한다.
일시정지된 수동 Seek는 자동 종료에서 제외해 뒤쪽 box 편집을 보존한다.

### Foliage 로드 성공과 material bind 성공을 분리한다

C++의 필수 Bind_RawValue 이름이 compiled FX의 global에 없으면 material 준비가 E_FAIL로 끝나
draw가 제출되지 않는다. catalog와 resource admission만으로 이 경계를 검증했다고 기록하지 않는다.
SL03의 g_SourceFoliageWindProgram 누락은 설치된 FX에서 실제 실패를 재현한 뒤 공용 HLSL에
동일 uint 계약을 연결했다. Binary base에 전역 변수를 추가할 때에는 모든 SourceGroup 변형도
재컴파일해야 CShader의 변수 타입·크기 parity를 유지한다. 조용한 optional bind로 우회하지 않는다.

- 이펙트 표식은 같은 자리를 그리는 프롭이 여러 개다. 항구 입항 구역(ITR_10297, DockingVolume 25 m 사각 테두리), 안전지대(ITR_10118, SafeZone), 섬 입구 닻(ITR_10066, Par_G_Symbol_Anchor_01)은 서로 다른 ParticleSystem이다. 복원 전에 스크린샷의 자리에 실제로 놓인 프롭 ID를 배치 데이터(DeployData의 Prop ID)에서 확인하고 LookInfo의 ParticleSystem 이름으로 대조한다. 트리거 위치만 옮기고 표현은 다른 프롭 것을 쓰면 화면이 달라진다. LookInfo에 CEFParticleData 블록이 여러 개 있으면 블록별 플래그를 확인하고 사용한 블록과 사용하지 않은 블록을 RESULT에 남긴다.
- 이펙트 문서의 요약 필드(detail.particle.startSize, initialPosition)는 음수 크기와 위치 모듈 여러 개를 반영하지 않는다. 대조는 sourceRecipe.modules에 보존된 원본 lookup table로 하고, 재생은 모듈을 따른다.
- 전투 HUD의 레벨 조건은 한 곳이 아니다. Update_CombatHUD의 isSupportedLevel, RenderCombatHUDText, RenderSkillCooldownText, RenderQuickSlotKeyLabels(그리고 RenderDamageNumbers)가 각자 같은 레벨 목록을 복제해 갖는다. 마하라카처럼 HUD가 없던 레벨에 HUD를 켤 때는 이 목록을 전부 확인한다. 한 곳만 열면 슬롯은 보이는데 쿨다운 초·키 글자가 빠진다. 물총 HUD는 `Is_WaterGunHudLevel`(마하라카+무장+도보)로 앞의 네 곳만 열었고 RenderDamageNumbers는 열지 않았다.
- 슬롯을 숨기는 HUD 모드는 C++이 고정 위치에 그리는 글자도 같이 막는다. HP/마나 숫자(RenderCombatHUDText)와 아이템·특수 슬롯 키 글자(RenderQuickSlotKeyLabels)는 슬롯 가시성과 무관하게 rect만 읽어 그린다. 배 HUD는 `m_bShipHudActive`로 두 글자를 함께 막는다.
- 원본 UI 무비(EFSwfMovie)는 UModel이 내보내지 못하고 이 PC에는 ffdec/Java가 없다. `Tools/LpkPipeline/dump_upk_movie.py`(패키지 export에서 GFX 바이트 추출) → `gfx_native_parse.py`(태그 파서) → `gfx_native_tree.py`(배치 트리)로 심볼 위치·이미지 영역을 읽는다. UModel 조회 이름은 난독화된 파일명이 아니라 논리 패키지명(`EFUI_OCEANHUD`, `EFUI_ICONATLAS_V`)이다. 도형에는 id 65535 플레이스홀더 bitmap fill이 실제 sub-image fill 옆에 함께 나온다.
- 표시 문구의 값 공식이 무비에 없을 수 있다. 배 HUD의 `{0} 노트`(`sys.voyage.hud_speed_indicator`)는 어떤 무비에도 키가 없고 네이티브 코드가 채운다. 공식을 확인하지 못한 값은 환산해 표시하지 말고 연결 불가로 남긴다.
- 세부 navregion이 걸을 수 있는 범위보다 작으면 함정이 된다. 서버는 질의의 첫 점을 담는 영역의 격자로만 판정하므로(`CServerNavigation::Select_Region`), 22m 창 하나만 굽고 나머지는 기본 격자에 맡기면 창 안에서 시작한 이동은 창 밖 목적지(출구 트리거·스폰·모래)를 전부 못 가고, 밖에서 안으로는 들어와지는 한 방향 함정이 된다(마하라카 WaterpangEntry). 영역을 만들 때는 그 영역 안에서 걸어 나가야 하는 모든 곳(출구·스폰·NPC·인접 지형)이 같은 영역 격자 안에 있는지 게시본으로 검산한다(`Tools/MapPipeline/test_maharaka_walkable_outside.py`의 규칙 재현 방식). 영역을 넓히면 "영역 안 walkable"을 다른 판정(아레나 위험·무장)의 근거로 쓰던 코드가 섬 전체로 번지므로 그 호출부에 별도 범위 검사를 붙인다.
- 원본 HUD(Scaleform 무비)를 복원할 때 frame 1의 좌표를 그대로 믿지 않는다. oceanhud의 skillSlotList는 무비상 간격이 47px인데 원본 클라이언트 스크린샷에서는 44.3px(Q x=699.0, A x=721.3)로 실행 시 다시 배치된다. 조각의 크기와 버튼 위치는 무비가 맞았지만 스크립트가 재배치하는 목록은 사용자 사진을 격자로 확대해 잰 값이 정답이다. 무비 좌표로 만든 결과와 사진을 나란히 합성해 비교한 뒤 확정한다(2026-09-30 배 HUD).
- 배 HUD 돔의 정체는 내구도가 아니라 보급(OceanSupplieGauge)이다. 분모는 EFTable_VoyageShip.MaxSupply(레벨1: 8200=3500, 8203=3900)다. 이름을 추측하기 전에 무비 sprite 이름과 테이블 열 이름으로 확정한다. 프로젝트에는 보급 소모가 없어 가득으로 표시한다.
- GFX 도형 배치를 계산할 때 음수 스케일(거울 반전)이 걸린 조각은 x0 = tx + min(b0*sx, b1*sx)로 왼쪽 위를 구한다(b0*sx만 쓰면 반전된 조각이 반대편에 놓여 링이 X자로 겹친다). 회전 성분(r0, r1)이 있는 조각은 게이지 채움용이므로 정지 그림에서는 반쪽 두 장을 거울로 합쳐 쓰고 파이 마스크로 채운다.


### Release 필수 Effect와 Movie 오디오 시계 재발 방지

- Effect element displayName은 UTF-8 1~64바이트다. 생성기와 ZIP preflight에서 같은
  계약을 검사하고, JSON 구조 검사와 실제 필수 문서의 CEffectDocumentCodec Load를 구분한다.
  긴 asset ID를 표시 이름에 반복해 입장을 막지 않는다.
- Character mesh particle이 공유하는 원본 Character/SourceMaterials DDS는 texture로만
  admission한다. 동명 Effect DDS의 다른 해시를 무시하고 교체하지 않는다. 경로 탈출과
  모델·미존재 파일 거부를 유지한다. 동일 native program을 쓰는 두 MIC도 sourceMaterialPath
  정확 일치가 입장을 막을 수 있다. parent/base/static ShaderMap key와 입력 ABI를 대조한
  명시 variant만 허용하고 텍스처·동적 수치·다른 carrier 검증은 유지한다.
- Movie 카메라 source clock의 time dilation을 WAV pitch·cursor·drift 보정에 중복 적용하지
  않는다. cue 시작만 source→Movie 시간으로 매핑하고 WAV 진행·길이·Sound trim은 감속 전
  Movie 시간, pitch는 사용자 수동 배속으로 계산한다. Pause/Seek/끝/tail과 일반 World를
  함께 확인한다. 화면·실청 결과는 FMOD NOSOUND 검증과 구분한다.
- Lobby 저장 카드도 animated shader·Character/part/collider 공통 prototype이 필요하다.
  Begin_LevelLoad로 이전 레벨별 준비 상태를 초기화하고 실제 class 모델의 지연 로드는 유지한다.

구체적인 실패 로그와 실행 검증은09-29 RELEASE_ENTRY_AUDIO_REPAIR_RESULT에 둔다.


### Movie0에 배치한 Sound의 원본 앞부분 복구

Sound startMs가0인데 sourceStartMs가양수이면 왼쪽 edge는 Movie0 경계때문에 더 늘릴 수
없다. 파일 손실이나 감속 clock 회귀로 단정하지 않는다. 원본 앞부분 복구는 source-in0과
실제 WAV frame 수에서 얻은 전체 duration으로 한다. 기존 배치와 volume·mix bus 복원본은
보존하며 음수 timeline이나 pitch 보정으로 대신하지 않는다. 해당 수정은 WorldSequences
scope로 publish하고 실행 중 authoring draft와 저장본을 구분한다. G20 결과를 따른다.

### 신규 packet 실패는 실제 링크된 Shared도 확인한다

전투 연결은 정상인데 새 도구 명령만 실패하면 현재 소스 codec 검사와 설치된 Debug/Release
Shared.lib 링크 검사를 구분한다. payload 생성 성공 후 Build_Packet_Frame 또는 header 읽기가
거부되면 PacketType 정의와 실제 provider 객체를 대조한다. CL.read의 header 의존성이 빠진
객체는 Build가 최신 상태라고 보고해도 오래된 packet 범위를 유지할 수 있다. 진단 로그와
실패 archive를 보존한 뒤 표준 경로의 필요한 C++ 컴파일로 복구하고 compiler가 의존성을
다시 기록했는지 확인한다. tlog 삭제·수동 편집이나 timestamp 조작으로 우회하지 않는다.
실행 중 Client/Server는 예전 static library가 연결된 EXE이므로 새 archive 생성과 EXE 재연결·
재실행을 구분한다. 09-30 Balance Test의 증거는 RELEASE_ENTRY_AUDIO_REPAIR_RESULT G08에 둔다.


### 관전 대상과 local-only 카메라·효과

관전은 follow target 교체만으로 완성되지 않는다. ALT_V camera, private element mask, afterimage,
skill shake와 Kouku area camera·lighting·timer가 실제 camera subject를 소비하는지 함께 확인한다.
이미 재생 중인 private effect도 대상을 바꾸면 mask를 갱신해야 한다. 대상의 사망 때 live transform만
추적하면 후속 정리/텔레포트가 죽은 시점을 이동시키므로 stable ID와 frozen pose를 따로 유지한다.
Debug timer preview의 성공은 Release 제품 HUD 표시 증거가 아니다. Server deadline 복제와
일반 HUD 숨김 뒤의 타이머 text gate를 함께 검사한다. 배틀 아이템은 inventory 소비와 cooldown을
분리해 확인하며, 서버 30초 거절이 Client에 표시되지 않는 것을 슬롯 소진으로 단정하지 않는다.

### 발탄 반복 그래프와 root portal 위치의 소비자 경계

카운터까지 반복하는 Trash는 Python/PowerShell/native graph admission뿐 아니라 Workbench 그래프와
타임라인 미리보기까지 같은 exact retry edge 계약을 소비해야 한다. 그래프의 실제 target을 terminal로
바꾸지 않고 미리보기만 첫 반복 경계에서 멈춘다. 다른 cycle과 dangling target은 계속 거절한다.

root snapshot 이펙트의 offset은 `Local * SnapshotRootSourceBasis * AttachmentRoot`에서 확인한다.
body의 전방 보정과 snapshot source basis가 각각 -90도인 발탄 portal에서 local +X는 owner 왼쪽이다.
position만 owner 전방으로 옮길 때 정상 geometry 회전이나 공용 shader 축을 제거하지 않는다.
authoring motion을 FORWARD로 바꿨으면 Product→rootmotion→Gameplay 게시 순서를 지켜 옛 curve를 제거한다.

### 레이드 즉사·독립 무력화·저작 Duration 연결

최대 HP 피해를 rawDamage로만 전달하면 방어·보호막·시간 정지·death-deny에 막힐 수 있다.
명시적 즉사와 전멸은 공통 lethal 플래그 및 그 이전 대상 선정까지 함께 검증한다.
무력화는 HP snapshot 차이가 아니라 별도 채널이며, 동일 cast의 DAMAGE/STAGGER/COUNTER
저작 행에 기여를 중복 배분하지 않는다. STAGGER-only 행도 감소 전 피해 basis를 보존해야 한다.
Duration 이름만 저장하고 judgementKind를 지정하지 않으면 Product 동작이 생기지 않는다.
새 kind는 authoring 목록·projector·bootstrap parser뿐 아니라 Brain의 runtime admission도 연결한다.

### 무력화 damage 기준과 typed Result 채널

DAMAGE/COUNTER/STAGGER가 별도 row인 skill에서 incoming HP damage만 /1000하면 STAGGER row의
호출자가 이미0으로 지운 값을 사용하게 된다. caster와 projectile timed/contact의 독립 stagger
subhit ordinal로 같은 cast damage share를 전달하고, HP차감·무력화지급 채널은 따로 유지한다.
DAMAGE row와 STAGGER row 양쪽에서 기여를 지급하지 않는다. shared raid maximum 편집은 현재
진행률을 ratio로 보존해야 하며 min clamp로 성공 outcome 없는 가득 찬 gauge를 만들지 않는다.

### 같은 용 모델의 스킬별 descriptor와 shader 수정 구분

스킬 shader 수정이 효과가 없다는 보고에서는 현재 binding·animevent·effect stable ID부터
실제 ModelCue 또는 particle carrier까지 확인한다. BRDF의 NaN 방어가 적용돼 있어도 MIC의
파란 rim·발광 곡선이 남아 있으면 색은 계속 파랗게 나온다. 다른 스킬의 재질로 맞출 때는
family뿐 아니라 texture·parameter track을 함께 비교하되 실제 mesh의 materialName은 보존한다.
시간은 해당 occurrence 수명에 맞추고 visibility/dead·geometry·transform은 별도로 보존한다.
JSON의 명시 texture만 복사하면 CModel override 이전 기본 material DDS가 빠질 수 있으므로
기본 모델 dependency와 Resources 전달본 자체의 실제 CModel 입장을 같이 검증한다.

### UI rect가 0 크기를 거친 뒤 다시 커지는 경우

보호막·게이지가 0일 때 quad 크기를 0으로 바꾸면 CTransform::Scale의 기존 basis normalize로는
다음 양수 크기를 복원할 수 없다. CUI_Sprite는 이전 transform이 아니라 현재 rect와 viewport에서
축을 재구성한 뒤 authored rotation을 적용한다. 흰 texture와 tint 확인만으로 fill 표시 성공을
판정하지 말고, 0→양수 및 소진→재부여 순서에서 실제 transform 폭이 복원되는지 확인한다.

### 인형·공의 독립 광기를 HP·shield 변화로 판정하지 않는다

특수 광기의 유효 접촉을 HP 감소 또는 shield 감소로 추론하면 피해가 미리 차단된 경우 함께
누락된다. 실제 내부 spatial contact와 stable WORLD source를 기준으로 광기를 계산하고,
일반 피해 기반 광기 gate와 분리한다. AREA_OVERLAP은 범위 밖 timeout에도 spatialContact를
전달하므로 resolved contact center까지 확인한다. 새로 발생한 특수 흡수 텍스트만 제거하며
실제 HP 손실과 일반 공격·이전 대상 event는 보존한다. 회귀에 shield 감소뿐 아니라 둘 다
변하지 않는 무적/피해0 접촉과 범위 밖/시간정지/변신 상태를 포함한다.

### 반복 패턴 snapshot과 입장 공격 중복

Server stage graph에 뒤쪽→앞쪽 retry edge를 추가하면 Client의 stageIndex 단조 증가 검사도
같이 검토한다. 동일 patternSequence에서 stageIndex가 작아졌다는 이유만으로 거절하면 Server는
반복하지만 Client는 마지막 자세에 멈춘다. 승인된 반복 경로에서는 새 actionStartTick으로 회차를
구분하고 serverTick·sequence·과거 actionStartTick 거절은 유지한다. snapshot이 합쳐져 같은
stage의 다음 회차만 도착하는 경우와 반복 뒤 실제 counter→GROGGY 종료도 포함해 검증한다.

자동 입장 컷씬 뒤 별도 legacy intro를 삽입할 때 저장된 rotation의 첫 occurrence와 중복되는지
확인한다. 컷씬→등장 휠윈드→일반 휠윈드를 테스트 기대값에 그대로 넣으면 실제 중복도 통과한다.
G 입장 검증은 사용자가 보는 순서인 컷씬→저장된 휠윈드 한 번→다음 패턴을 확인하고 명시적인
Play All과 legacy audition을 별도로 보존한다.


### 부위 파괴 효과와 Combat Object 소환 연결

- 갑옷 파편과 바닥 돌은 같은 연출로 취급하지 않는다. PART_BROKEN의 실제 제거 mask가
  파편을 소유하고, RECOVERY ENTER의 SPAWN_COMBAT_OBJECT는 별도의 피해 오브젝트다.
  잘못된 연출 제거는 해당 spawn과 전용 companion/visual/sound까지 함께 교정한다.
- managed spawn을 제거한 publisher는 이전 Product의 같은 managed owner object도 제거해야 한다.
  replace-or-append만 쓰면 사라진 오브젝트가 게시본에 남는다. 다른 legacy owner는 보존한다.
- 파괴 폭탄 전용 갑옷은 서버가 생성한 폭탄 provenance로 판정한다. skillId 숫자만 검사하거나
  일반 HP 피해를 legacy armor durability로 다시 빼면 평타·스킬로도 갑옷이 파괴된다.
- 부위 파괴 준비 PNG는 살아 있는 갑옷과 열린 파괴 창을 함께 확인한다. 실제 파괴 파편/성공 문구와
  준비 PNG의 발생 조건을 합치지 않는다. 반복 돌진·추가 폭탄은 이미 제거된 mask를 복구하지 않는다.


### 공통 무력화 정책과 기존 HP 누적 response

공통 피해/1000 계산을 추가해도 authored stage가 ACCUMULATED_HEALTH_DAMAGE를 계속 사용하면
그 패턴은 HP 누적 경로에 남는다. 실제 자동 선택 패턴의 ENTER/EXIT gauge, 성공 outcome,
후속 패턴과 HUD snapshot을 함께 확인한다. 발탄 마력구는 기존 SET_STAGGER_GAUGE와
STAGGER_BROKEN을 사용하며 HP0·흡수·감소·회오리 및 성공 한 번을 실제 hit 경계에서 검사한다.
무력화 창의 stage 높이를 허용할 때 source/publisher/Server/Client의 admission을 같이 맞추고,
닫히지 않은 ENTER/EXIT와 malformed branch는 거부한다. F1으로 바꾼 현재 공통값을 테스트가
40000 같은 과거 고정값으로 덮어쓰거나 실패로 간주하지 않는다.


### 공유 Effect pivot와 맵 ParticleSystem의 실제 입력

- 공유 Effect 원점 재기준화는 그 asset을 사용하는 모든 occurrence를 찾아 rotation·scale을 반영한
  inverse translation으로 현재 위치를 보존한다. 선택한 피자만 보정하고 지형 파괴 같은 다른 소비자를
  놓치면 같은 source 수정으로 다른 패턴의 연출이 밀린다. 원본 입자 world matrix를 시간별 비교한다.
- 고정 emission 중심을 동적인 렌더링 AABB의 중심으로 설명하지 않는다. 원본 속도와 발사 방향은
  보존하고, 일부 occurrence의90도 방향 요청은 그 occurrence에만 적용한다.
- 맵 상공 FX는 base ParticleSystem만 추출하면 instance 색·밝기·비균일 scale·warmup을 빠뜨릴 수 있다.
  원본 actor/component의 입력을 확인하고 같은 asset 이름의 proxy와 source-native 복원을 구분한다.
- 모든 문서 Element가 입자를 내는 것은 아니다. 원본 null Rate/RateScale·빈 BurstList emitter는
  비활성을 보존한다. finite/count 검증에서 제외할 때 정확한 원본 입력을 근거로 남기고 임의 spawn을
  추가하지 않는다. 현재 패턴 표시 시각은 원작 활성화/비행 시각을 복구했다는 근거가 아니다.

### Combat-object 폭발 준비와 피해 시계

원본 whole-sequence Effect의 준비 구간을 건너뛰어 기존 피해 시계에 맞추지 않는다. 실제 파편 시작
source age를 측정하고 준비 시작 event와 피해 hit를 분리한다. owner-hit chain은 direct/indirect
분류 지연에 준비 duration을 더한 시점에 피해·넉백·Sound를 발생시킨다. direct도 준비를 생략하지
않는다. 같은 armed Effect가 terminal까지 소유하면 hit에서 이펙트를 다시 시작하지 않는다.
Composition의 fixed hit 시점을 편집할 때 대응 준비 event도 동일 delta로 이동해 원본 준비 길이를
유지한다. local Preview의 입자·wire·Sound도 동일 준비/폭발 시계를 사용한다.

Valtan local Pattern preview는 LEAP 유무와 무관하게 staged Effect/volley/aim이 요구하는
ARENA_CENTER를 canonical boss placement에서 먼저 admit해야 한다. Object session 라우팅 변경은
pane 선택뿐 아니라 MainApp Resources, transport, viewport input owner까지 함께 연결한다.


### 공중 넉백 대상의 속박 복귀 위치는 현재 높이와 분리한다

Valtan FOUR_SLASH의 연속 forcePush는 최초 supportY를 유지한 채 공중에서 다시 발사될 수 있다.
속박 대상 선정은 살아 있는 KNOCKDOWN을 허용하므로, Stage admission에서 현재 Y와 nav ground의
높이 차이만 검사하면 정상 넉백 대상을 거절하고 room을 중단할 수 있다. Stage와 Commit은 같은
지면 resolver를 사용하고 유효한 bounded ballistic 상태에서만 supportY를 navigation hint로
사용한다. walkability·finite·기존1.5m support 검사는 유지하고 복원 XYZ는 검증된 실제 지면으로
저장한다. 넉백 정리는 기존 Cancel_PlayerActionForPatternStatus/Clear_Attachment를 재사용한다.
거절 진단은 current XYZ/ground/supportY/비행 상태를 함께 남겨 연결 종료 문구만으로 원인을
추정하지 않는다. 09-30 COUNTER_LOOP_AND_STRUGGLING RESULT G14에 구코드 실패/수정본 통과
native 재현과 실제 Debug/Release session 로그의 확인 범위를 구분해 기록했다.

### 겹친 source sprite의 회전과 완료 상태를 함께 확인한다

같은 문양이 여러 emitter에 있으면 선택한 한 겹만 Element 회전을 따르는지 확인한다.
fixed-axis sprite는 `followEmitterAxisRotation` opt-in과 실제 SourceEmitterWorld를 함께
소비한다. 다른 겹이 기존 방향에 남은 현상을 공용 renderer의 회전 실패로 단정하지 않는다.
UV 완성 시점은 native 재질이 실제 소비하는 Color/Dynamic 채널의 원본 곡선에서 산출하며
완성 겹의 생성과 최초 alpha를 함께 확인한다. import scale이 있는 캐릭터의 본 검증은
실제 제품의 bone scale normalization까지 포함한다. 양의 측 companion을 음의 측에서
복제한 배치에 옮길 때는 원본 local offset 차이를 회전·scale한 위치 보정도 필요하다.

문양 자체를 중심으로 돌릴 때는 Element 원점과 실제 quad 중심을 구분한다. source
StartLocation과 중앙이 아닌 sprite pivot의 두 offset이 모두 회전하므로, Element position만
고정하면 문양이 이동한다. 현재 회전에 delta를 합성하고 두 offset의 회전 전후 차이를 위치에
보상한다. 실제 본 basis의 최종 중심·right/up·법선·정점으로 확인하며 다른 겹의 사용자 위치를
공통 평균으로 덮어쓰지 않는다.

### 손 부착 해제는 포획 시점의 상대 위치를 착지 위치로 쓰지 않는다

root translation을 억제해도 원본 본의 회전은 남는다. 잡기와 내려찍기 사이에 몸이
회전하면 포획 시점 owner-local offset은 실제 손과 반대편이 될 수 있다. 설치 모델의
impact source time, preScale, visual yaw, presentation scale과 실제 손 본을 함께 측정하고,
해제 지점은 그 손 부근의 같은 층 navigation·collision을 검증한 뒤 Server에서 확정한다.
이미 적용한 visual yaw를 서버에 다시 더하지 않는다. 지면 검사에는 착지 후 실제 이동
명령도 포함하며, 다중 대상은 HP·attachment·피해 event를 모두 stage한 뒤 한 번에 commit한다.

### 검격의 표시 회전과 발사 방향을 분리한다

Particle System yaw는 모양뿐 아니라 입자의 이동 벡터도 회전한다. 모양만 회전하고
owner 정면 발사를 유지하려면 기존 directionYaw를 반대 방향으로 보정하고 실제
Playback의 초기 속도·수명별 velocity module·부모 basis 합성 순서를 확인한다.
고정 축 sprite는 최종 quad의 emitter basis 소비도 따로 검사하며 필요한 요소에만
followEmitterAxisRotation을 적용한다. 이동 중심만 정상이라고 최종 면 회전도 정상으로
판정하지 않는다. 여러 패턴이 공유하는 effect 자체의 수정은 모든 소비자에 적용되므로
사용자가 지정한 공유 범위를 확인하고 occurrence의 위치·시각과 삭제한 요소를 보존한다.
발탄 Atk_08_04의 모양 반시계90도·정면 이동 검증은09-30
VALTAN_EFFECT_CENTER_AND_SKY_RESULT G05에 기록한다.

### 본 부착 Element 위치축을 화면 높이축으로 가정하지 않는다

사용자가 높이를 내리려는 경우 Element Y 숫자 감소만으로 완료했다고 판단하지 않는다.
실제 CModel preScale·preRotation, 정규화 본, owner basis를 포함해 world 변위를 측정하고
원하는 world-Y 이동을 해당 local 축으로 환산한다. Guardian b_effectworldzero는 localY가
수평이고 localZ가 world-Y다. fixture의 모델 생성 인수도 제품과 같아야 하며 identity
anchor의 Y 감소 성공을 실제 모델 높이 성공으로 대체하지 않는다. 복제한 주변 요소는
sourceNode 문자열만으로 누락시키지 말고 stable ID와 실제 소비자를 함께 확인한다.


### 맵 환경 profile의 전역·지역·셀피와 구운 조명

- 환경 component·WorldInfo·Volume의 값은 CDO와 실제 override flag를 합성한다.
  SelfCamera의 두 번째 환경 세트는 owner 참조가 셀피 전용인지 먼저 확인한다.
  큰 native shadow grid 뒤의 tagged property를 앞4KiB에서 찾지 못했다는 이유로
  미직렬화 기본값으로 처리하지 않는다.
- RNM의 bakedLightGuids와 local light의 lightmapGuid를 대조해 중복 직접광을 막는다.
  dominant directional의 ShadowMap2D는 별도 lightGuid이며 두 GUID를 혼동하지 않는다.
  같은 모델도 shadow atlas가 다르면 material variant를 나누고 placement UV를 보존한다.
- 환경광으로 쓰는 Lightmass 색의 scene adapter는 원본 runtime SH 복원이 아니다.
  Resources 전달은 조명·LUT·실제 재질 반사 참조를 모두 포함하고 기존 파일을 보존한다.
  원본 재조사와 기존 G05 오류 교정은09-30 마하라카 SOURCE_LIGHTING RESULT를 따른다.


### 2026-09-30 scene 안개 스위치와 탈것 방향광의 범위

Height Fog Enabled는 base뿐 아니라 같은 profile의 environment region 안개를 함께 끄는 master다. 지역 선택 뒤 profile.Fog.bEnabled를 최종 enabled에 적용하며, OFF/ON을 밀도 0 또는 지역 값 덮어쓰기로 구현하지 않는다. 28개 현재 profile의 지역 사용 12개는 모두 기존 enabled=true여서 이 gate가 현재 저장된 화면값을 바꾸지 않는다. Rendering Workbench Save/Publish는 기존 fog.enabled를 저장한다.

고대의 바다 9523의 원본 vehicle cue Brightness 0.0001을 MainApp에서 scene directional 배율로 사용하면 맵 diffuse/specular가 거의 사라진다. 이는 원본 retail scheduler로 확인되지 않은 project adapter였으며 전역 조명 입력에서 제외했다. 일반 character presentation directional control, 지역 조명, 원본 vehicle cue·재질·effect는 유지한다. 소스 검증과 제품 빌드·화면 확인은 대응 RESULT에서 구분한다.

### 맵 가시성 복원과 원본 초기 시퀀스

actor/component/CDO가 visible이고 LevelStreaming이 AlwaysLoaded여도 최종 초기 가시성이 같다는 뜻은 아니다. `LevelLoaded`/`LevelStartup`에서 `ToggleHidden.Hide`로 이어지는 실제 target actor와 외부 level 참조까지 대조한다. 베른 SCENE03E의 45개는 원본 초기 시퀀스가 숨기므로 이름 차단을 제거하는 방식으로 복원하지 않는다. 원본 스트리밍에 연결되지 않은 이벤트 패키지도 현재 기본 맵에 일괄 표시하지 않는다.

placement ID와 TRS가 일치해도 오래된 Landscape WModel의 축이 반대일 수 있다. 원본 높이·normal·hole topology와 설치 정점을 함께 비교하고, 현행 추출기로 재변환한 뒤 tile anchor와 winding을 확인한다. 재질 bake/cliff 분리가 달라지는 전체 변환은 UV·재질 byte 불변으로 설명하지 않는다. 전체 mip 후보를 별도 overlay로 만든 경우 배포는 cook 폴더 전체 복사 대신 최종 manifest의 후보 경로를 소비한다.



### Landscape의 흐림은 해상도와 원본 UV 계약을 함께 확인한다

큰 타일을256px로 베이크한 결과는 원본 반복 텍스처를 복원한 것이 아니다. 베른은 원본 grid×0.1, 중심 회전의 source scalar×3.1400001049, layer tiling을 사용한다. component 폭으로 나누거나 rotation을 degree로 해석하면 무늬 크기부터 달라진다. 단순 upsample·mip bias로 보정하지 않는다.

actor의 Landscape material instance static key와 원본 ShaderCache PS/VF를 맞춘 뒤 layer별 paint/height blend, linear 색 공간, sample normal RG 및 Heightmap BA의 pixel basis를 함께 연결한다. diffuse와 normal의 height blend는 같다고 가정하지 않는다. source PS가 layercliff를 샘플하지 않으면 경사면에 별도 cliff layer를 만들지 않는다. 원본 weight/height의 subsection 중복 경계와 모든 mip, geometry hole을 보존한다. 수직면의 높이 0은 원본 height bytes·collision sample·paint·hole allocation과 선택 PS의 discard를 함께 확인한 뒤 판정한다. source 높이·hole과 설치 삼각형의 일치는 원작의 실제 draw/LOD나 사진 속 메시 식별을 대신하지 않는다. WARP의 constant-sample 수치 일치를 실제 공간 UV·화면 검증으로 확대하지 않는다.

### 본체 헤어를 숨기기 전에 기본 대체 파츠를 확인한다

새 hairstyle catalog 등록만으로 모든 캐릭터 생성 경로에 별도 헤어가 장착되지는 않는다.
Guardian은 기본 파츠에 helmet만 있고 preview·직접 audition·저장 외형 없는 spawn은
본체 hair submesh를 사용한다. iBodyHairMeshMask를 무조건 OR하면 파일에 정상 헤어가
남아 있어도 대머리와 기존 머리 장식만 표시될 수 있다. CHARACTER_SPEC의
isBodyHairFallback은 성공한 HEAD 교체가 있을 때만 본체 헤어를 숨기며 reset은 되살린다.
기존 별도 기본 hair 파츠를 가진 class는 이전 숨김 정책을 유지한다. WModel 파손·교체와
표시 mask 회귀를 구분하고, 실제 submesh/material 및 spawn·commit·실패 보존을 확인한다.
생성창의 기본은 별도 경로다. hairstyle defaultBodyHair=true는 기존 index를 재정렬하지 않고
-1로 본체 기본 머리를 유지한다. 원본 full outfit이 HEAD까지 점유하면 머리를 함께 제거하므로
생성창은 HEAD 없는 torso variant를 참조한다. 저장 복원도 같은 문서를 사용하고 의상+머리를
한 transaction으로 적용한다. 미선택 머리 색은 alpha -1이며 초기 zero 값을 검정 선택으로
저장하지 않는다. 장비 재생성 뒤에는 실제로 선택한 색과 투톤만 다시 적용한다.


### UE3 static mesh COLOR0와 배치별 static shadow

- UModel glTF에 COLOR_0가 있다는 사실만으로 원본 vertex color의 존재를 확정하지 않는다.
  native position/UV stream 다음 FStaticMeshColorStream의 count와 실제 bytes를 확인한다.
  KR868/16의 빈 stream에서 생긴 임의 색은 제거하고 CMesh의 기존 white fallback을 사용한다.
  실제 stream만 BGRA→RGBA로 연결하며 position/UV/basis/index/bounds/WMAT를 보존한다.
- static shadow는 같은 RNM atlas pair의 같은 모델이라도 배치마다 다를 수 있다.
  atlas뿐 아니라 light GUID/channel/penumbra/exponent까지 같은 경우에만 재질을 공유한다.
  source shadow가 없는 배치를 별도로 구분해 다른 배치의 shadow를 상속시키지 않는다.
- 재현 도구는 `restore_static_source_colors.py`, `build_map_static_shadow_variant_set.py`,
  측정값과 남은 경계는 `10-01/2026-10-01_COLOSSEUM_MATERIAL_RESTORE_RESULT.md`를 따른다.


### Local movement 보정의 총 이동량과 지면 높이 연속성

- 잔여 위치 오차만 이동 속도로 줄여도 snapshot projection이 같은 방향으로 전진하면
  합산 표시 속도는 두 배가 될 수 있다. 최종 XZ 이동량에 frame budget을 적용하고,
  같은 시각의 재호출이 예산을 다시 소비하지 않게 한다. 서버 이동 중 오차를 줄이려면
  작은 catch-up 여유가 필요하며1배 제한은 지연을 영구 유지할 수 있다.
- 이동 불연속의 수평 속도 한계와 navigation이 허용하는 계단 높이를 혼동하지 않는다.
  XZ 한계 안의 수직 변화만 실제 같은 grid/layer의 양끝 지면과 segment walkability로
  증명한다. 다른 층·공중·막힌 구간·수평 teleport는 완화하지 않는다. 전체3D threshold를
  높이거나 Y를 무조건 무시하는 방식으로 수정하지 않는다.
- ACK를 적용하는 wall 시각에서 이전 frame의 visual pose 보정을 다시 시작하면
  Object→Level 사이 처리 시간만큼 표시 시간이 사라져 가감속을 반복할 수 있다.
  freshness·수신 관측은 wall clock, projection·보정은 Engine delta를 한 번씩 누적하는 frame clock을
  사용한다. min(Engine delta, Object 호출 wall 간격)도 호출 위상 jitter에서 시간을 잃는다.
  ACK 즉시 조회와 같은 frame 재호출은 위치·presentation 시간을 추가 소비하지 않는다.
- MOVE ACK RTT를 매번 projection lead에 넣거나 재클릭마다 경로·보정 시간을 초기화하면
  입력 빈도가 표시 속도를 바꾼다. command ACK와 path 수명, Server tick/수신 clock,
  frame당 한 번 소비하는 이동·보정을 분리한다. 같은 직선의 다른 거리 목표와 수신 gap도 검사한다.
- 40/60FPS에서 최신 snapshot의 음수 age를 0으로 자르면 30Hz 수신 위상이 목표 위치를 흔든다.
  같은 Server 시점의 sample로 비교하고, 각 클릭·ACK 전후 frame과 steady 절대 속도를 함께
  검사한다. single과 연타가 같은 잘못된 가감속을 반복하는 경우를 동등성만으로 통과시키지 않는다.
- known Server corner의 sample만 polyline으로 만들어도 visual→최신 waypoint 기본 이동과
  residual이 코너 안쪽을 가를 수 있다. 실제 지난 이전 waypoint를 먼저 소비하고, 새 경로의
  fast ACK에는 cached corner를 폐기한다. 새 입력 후 local frame이 없는 ACK도 별도로 검사한다.
- 프레임 순서는 Character Update → Level replication → Controller 입력이다.
  평균 FPS만 같게 만든 fixture와 실제 입력 순간의 GPU readback stall을 구분한다.
  숫자 재현·컴파일과 사용자의 최종 화면 재현을 별도 기록한다.

### 워터팡 종료와 snapshot의 latest-cast 쌍

- 물총 `iWaterGunSkillId`와 `iWaterGunCastTick`은 둘 다0이거나 둘 다 유효해야 한다.
  종료 때 ID만 지우면 Shared writer가 방 전체 snapshot을 거부한다. Client 입력·클릭 효과와
  Server tick이 살아 있어도 위치가 마지막 snapshot에 멈출 수 있으므로 먼저 encode 실패와
  cast 쌍을 대조한다. navigation 데이터를 지우거나 Client transform으로 우회하지 않는다.
- 종료는 최초 admission spawn의 전원 착지를 먼저 준비하고 participant 상태·물총·낙사·
  발사대를 함께 정리한다. deck 밖으로 떨어진 사람도 참가자로 기억하며 일반 방문자는 제외한다.
  intro 저장값뿐 아니라 trigger의 sequence 활성화도 해제해야 같은 방에서 재입장할 수 있다.
- 실제 인간 발사 → 종료 tick → 모든 session의 snapshot decode → 이동 → G 재입장과
  다음 intro/AI 생성까지 확인한다. 단순 AI 제거 개수만으로 종료 성공을 판정하지 않는다.

### GPU 최적화에서 실제 texture fetch와 scene-color 소비 순서

- HLSL 삼항식은 OFF 재질에서도 sample 후 선택으로 컴파일될 수 있다. 재질 공통 flag에만
  명시 분기를 적용하고 실제 FXC DXBC를 검사한다. ON 수식·OFF fallback·sampler를 보존해도
  GPU 시간 개선과 최종 화면 확인은 사용자 캡처 전 확정하지 않는다.
- Effect의 초기 scene snapshot을 없앨 때 모든 live 소비 전에 occurrence refresh가 있는지
  확인한다. Map/World 요청과 HDR/Bloom pair는 별도이며 frozen capture는 실제 source target을
  사용한다. Bloom OFF만 보고 native PS의 추가 평가를 지우면 scene 입력에 따른 clip/coverage가
  달라질 수 있다. 맵 draw 수가 줄었는데 Blend GPU 시간이 늘어난 캡처를 mesh 과다로 단정하지 않는다.

### WORLD 외부 시계의 연속 샘플과 Sound 재생성

쿠크처럼 Server 시각을 매 frame WORLD에 전달할 때는 Seek_AllToMs의 continuous
호출을 사용한다. 기본 discontinuous scrub으로 샘플하면 Apply_Sounds가 이미 재생 중인
soundTracks 채널을 frame마다 Stop/Play하여 정상 WAV도 끊겨 들린다. 최초 catch-up과
명시 scrub은 기존 seek를 유지하고, 실제 연속 재생·역방향·큰 시각 이동을 각각 검사한다.
Composition SOUND와 WORLD soundTracks는 소비자가 다르므로 한쪽이 정상이어도 다른
쪽의 채널 수명은 따로 추적한다. 원본 WAV 실청, 채널 재생 횟수, Client 최종 실청은
구분한다. 수정·수치 근거는 09-26 KOUKU_PLAYTEST_RECOVERY RESULT의 G15 후속에 둔다.
### WORLD 본 Collider와 부착 Effect의 부모 좌표계

본·모델·재생 시계가 같아도 Effect root의 회전이 다르면 화염과 Collider가 다른 방향을 본다.
생성 수명이나 본 회전 부호를 바꾸기 전에 실제 설치 WModel의 본 basis와 Element·socket·
Effect root·Object 변환을 함께 대조한다. Collider의 optional `worldEffectTrackId`로 같은
Effect 부모를 명시하고 빈 값의 기존 동작을 보존한다. 임의90도 상수를 Collider에 더하지 않는다.
회귀는 실제 앞뒤 본·여러 시점·반복 경계를 사용하며 불 방향 접촉과 종전 오방향 미접촉을
함께 검사한다. `WORLDKEY` 개수·finite 성공만으로 방향 정합을 판정하지 않는다.
[인형 Collider·칼날·카드 미로 결과](10-01/2026-10-01_KOUKU_DOLL_COLLIDER_AND_BLADE_RESULT.md).

### 저FPS 클릭 정지와 비동기 피킹 명령 순서

1픽셀만 복사해도 직후 flags0 Map은 GPU 완료를 main thread에서 기다려 클릭 프레임을 멈출 수 있다.
픽셀 수 최적화와 완료 대기 제거를 구분하며 일반 이동은 Request 후 DO_NOT_WAIT Poll을 사용한다.
지연 응답은 요청 당시 pixel·fallback·press와 action/move sequence를 보존해야 한다. 후속 스킬·상호작용·
외부 이동이 먼저 송신됐다면 이전 결과를 버리고, 새 press·UI/focus·사망·capture·재바인딩도 취소한다.
취소 결과에 fallback을 적용하면 옛 이동이 되살아난다. WARP의 실제 Copy/Map 검사는 GPU 비대기·
원본 샘플·취소를, 생산 Controller 추출 검사는 버튼 release·재입력·명령 순서를 따로 검증한다.
40/60FPS 예측 회귀 성공과 실제 화면의 끊김 해소 판정은 구분한다.

후속 CPU 전환에서 `MapPlacementRuntime`만 조회하면 별도 Deploy가 그리는 발탄 파괴 바닥·난간과
쿠크 종이 다리가 빠진다. GPU를 대체할 때 실제 바닥의 소유 runtime을 함께 조사하고 Map/Deploy의
최근접 표면을 합성한다. 정적 Deploy는 현재 intact/fractured 모델, animated 다리는 현재 pose를
사용하며 bind-pose bounds로 펼쳐진 다리를 배제하지 않는다. despawn·opacity·source/camera
suppression을 Render와 맞추고 debris를 걷는 바닥으로 승격하지 않는다. resolver 대역의 Controller
검사만으로 carrier 연결을 검증하지 않는다. 실제 설치 geometry와 Level 연결·파괴 상태를 따로 검사한다.
[수정 결과](10-01/2026-10-01_DEPLOY_CPU_MOVEMENT_PICKING_RESULT.md).


### 워터팡 AI 최초 등장 준비와 NPC 장비

- 입장 Loader가 선택 class만 준비하면 경기 예약 뒤 20명 AI 첫 spawn에서 다른 class·24개 의상 모델·NPC8종의 decode/material 준비가 gameplay thread에 몰린다. Server roster의 2 class와 Shared NPC8 정본 및 item→visual-set 해석을 로딩 단계에서 준비하고 기존 prototype을 재사용한다.
- NPC 외형의 hidden Character proxy에 총을 달거나 player 전용 prop3를 찾으면 실제 NPC는 맨손이다. NPC의 실제 right-hand 본과 최종 preScale basis를 확인한 뒤 기존 CPart_Equipment를 현재 NPC pose 뒤에 갱신한다. 무장·사망/숨김·해제는 snapshot을 따르고 total gun scale에 .01을 두 번 적용하지 않는다.


### SpriteFont에 없는 UI 문자로 준비 완료 직후 Client가 종료될 수 있다

- `.spritefont`에 글리프가 없고 `defaultCharacter=0`이면 `DrawString`뿐 아니라
  `MeasureString`도 `Character not in font` 예외를 던진다. HRESULT `E_FAIL` 검사로는
  잡히지 않으므로 종료 로그의 예외와 실제 EXE/DLL에 맞는 PDB 호출 스택을 먼저 확인한다.
- 콜로세움에서는 준비 뒤 RECRUITING으로 전환하며 처음 표시한 모집 문구의 `·`(U+00B7)
  두 개가 YoonGasiIIM 및 소형 파생 폰트에 없어 `Render_PartyInviteText -> Measure_Text`
  에서 종료됐다. 컷신·AI·loader가 직전에 동작했다는 이유만으로 그쪽 결함으로 단정하지 않는다.
- UI 문구에 특수문자를 추가할 때 실제 선택되는 원본·크기별 폰트의 글리프와 fallback을
  확인한다. 이번 수정은 지원되는 ASCII `|`로 교체한 한 줄이며, 공통 font fallback은
  제품에 반영하지 않았다. 다른 미지원 문자나 닉네임까지 보호됐다고 설명하지 않는다.
- 실제 설치 폰트와 DirectXTK로 이전 문구의 예외를 재현하고 수정 문구의 측정·그리기를
  확인한다. Server 인원 계약이나 컴파일 성공만으로 Client UI 실행까지 통과했다고
  판단하지 않는다. 실제 다인 Client 화면 확인은 별도 검증이다.
- 상세 호출 스택·수정·배포 증거는
  [콜로세움 결과 G15](10-01/2026-10-01_PR494_496_FLEXIBLE_COLOSSEUM_RESULT.md#g15-사용자-실제4인-종료-모집-문구의-누락-glyph-예외)를 따른다.


### 콜로세움 연출의 별도 텍스트 경로와 초상 lookup

- HUD sprite 숨김만으로 Level의 월드 이름/HP·말풍선이나 Intro 자체의 직업/닉네임 텍스트는 숨겨지지 않는다. 연출 구간에는 별도 Draw_Text 소비자도 함께 차단한다. FINISHED 전체에서 HUD를 숨길 때 복귀 버튼의 입력까지 막히지 않도록 cinematic-owned pointer scope만 허용하고 일반 창·gameplay 차단은 유지한다.
- 초상과 MVP 대표/파티 카드·캐릭터 정보창·아바타창은 Movie 문서를 복사해 그리는 화면이 아니라 실제 CharacterCatalog 기본 직업·lazy 장비 재질을 사용한다. Movie의 TGA mip 수정 후에도 DDS가 남으면 초상만 이전 선명한 반사를 유지할 수 있다. 같은 mip0·색공간을 확인한 뒤 실제 소비 assetId를 맞추고 공통 조명·거칠기 옵션은 바꾸지 않는다.
- 컴파일·입력 수치 검증과 실제 Client 컷씬/초상 품질 확인은 구분한다. [콜로세움 결과](10-01/2026-10-01_COLOSSEUM_MATCH_FLOW_RESULT.md#g11-초기-초상월드-텍스트와-movie-lookup-일치)를 따른다.

### 생성 JSON의 CRLF/LF 차이를 게임 데이터 변경으로 오인하지 않는다

- Windows Git checkout의 CRLF와 publisher의 LF가 달라도 JSON 항목·값은 같을 수 있다. `projected Product is stale`만으로 레이드 진입 실패나 컷씬 설정 누락을 단정하지 않고 실제 구조와 줄바꿈을 구분한다.
- Kouku 생성본 검사는 CRLF를 LF로 정규화한 bytes를 비교한다. 실제 값 변경과 다른 형식 차이는 계속 거부하고, 기존 duplicate-key 검증도 유지한다. 이 판정을 고치기 위해 사용자의 원본이나 생성본을 다시 저장하지 않는다.

### Profiler 저장 범위와 누락된 CPU 자식을 먼저 확인한다

- 기본 JSON 저장은 현재 보유 전체(최대 1200프레임)이며 분석·표시 범위와 독립이다. `JSON 저장 범위 제한`을 켰을 때만 최근 프레임 수로 줄인다. 사용자가 관찰한 최저 FPS가 JSON에 실제로 있는지 frame interval과 `captureWindow`를 먼저 대조한다. 저장 범위 제외와 history 퇴출은 다르며 GPU pending/drop은 유효한 0ms가 아니다. export metadata는 저장 순간의 조건이다.
- Detailed scope가 frame cap을 넘으면 빠진 자식 시간이 부모 Self로 남을 수 있다. 누락이 있는 선택 구간의 Self를 병목 확정에 쓰지 않는다. 해당 UI는 Self를 `--`로 표시하며, 다음 비교는 Detailed OFF로 수집하고 raw 범위와 고정 cpuWork를 함께 본다.
- frame N interval은 Begin(N-1)→Begin(N), frame N CPU는 Begin(N) 이후 작업이다. 순간 지연은 CPU 원인 행과 interval 행 사이의 차이를 고려한다. GPU pending은 0ms가 아니고 전체 GPU timestamp 경과도 utilization이 아니다.
- 같은 actor의 애니메이션 보간 재사용, 재질별 draw 병합, GPU LOD는 별도 비용을 줄인다. 한 kernel의 절감률을 전체 게임 FPS로 환산하지 않는다. 구조·검증·남은 실측은 [프레임 통합 계획](10-02/2026-10-02_FRAME_PIPELINE_OPTIMIZATION_IMPLEMENTATION_PLAN.md)을 따른다.


### 구운 조명이 나눈 draw와 실패 경로 탐색의 비용

- CPU Map.Batch.Draw·Material·Pass가 호출 수에 비례하면 geometry 이름만으로 병합 가능하다고
  판단하지 않는다. 원래 RNM average/directional·static-shadow texture identity가 배치를 나누므로
  전체 비조명 입력과 실제 공유 geometry를 비교한다. 인접 배치의 원래 SRV를 제한된 bank로
  전달하고 각 instance의 UV·RGB scale·표시 순서를 보존한다. LOD·다중 mesh·override와 진단 중
  대상은 원래 draw로 남긴다. authored 배치 수의 감소 가능성을 실제 카메라 FPS 개선으로 쓰지 않는다.
- GPU timestamp에는 CPU의 제출 지연도 포함된다. PS 호출·VS 호출·draw 수와 CPU 구간을 함께
  비교하고 긴 GPU 경과 시간만으로 픽셀 과부하·VRAM 부족을 단정하지 않는다.
- walkable 목표의 작은 고립 영역 때문에 A*가 출발 쪽 넓은 지형을 상한까지 탐색할 수 있다.
  목표의 incoming Can_Step 간선을 제한된 수만 역탐색하여 연결 성분이 완전히 소진됐을 때만
  조기 UNREACHABLE로 판정한다. 제한 초과는 불연결 증거가 아니므로 원래 A*를 유지한다.
  성공 경로·동적 blocker·높이·대각선 조건을 원본과 비교한다.
  [베른 제출 최적화 결과](10-04/2026-10-04_BERN_DRAW_SUBMISSION_OPTIMIZATION_RESULT.md).

### 같은 맵 순간이동의 Guide 착지와 실제 보행 연결

- Sample_Position으로 높이를 얻었다고 주변 보행 영역과 연결됐다고 판정하지 않는다.
  Bern 항구에서는 owner와 높이가 비슷한 ARCHENTRY 위 고립4셀도 후보가 됐다. 실제 blocker가
  0인 데이터의 결함을 runtime blocker 누락으로 설명하지 않는다.
- Guide local 이동은 exact walkability·높이·충돌·다른 player 겹침과 owner의 navigation LOS를
  함께 검증한다. 뒤쪽 선호 후보가 없으면 owner 주변을 탐색하며, 후보 확정 전에 이전 pose나
  이동·combo 상태를 초기화하지 않는다. Guide actor identity는 유지한다.
- 성·도서관의 authored MOVE_PLAYER는 hold 시작이 아니라 실제 이동 완료에서 같은 도착
  처리를 호출한다. 성공한 Server 계약 검사를 Client 화면 확인으로 대신 기록하지 않는다.
  [Guide 결과 G12](09-27/2026-09-27_GUIDE_AI_TOOL_IMPLEMENTATION_RESULT.md#g12-건물-출입-동행연결된-가이드-착지-2026-10-02).

### Movie 분할 모델의 반복 포즈 샘플링

- Debug compiler 최적화와 Debug/Release 공통 알고리즘 개선을 구분한다. /O2만으로 Release의
  같은 입력 반복 계산이 없어지지는 않는다. Profiler의 부분 CPU 시간이나 미세 측정을 전체 FPS로
  환산하지 않으며 CPU/GPU 구간·계측 overhead·실제 장면을 각각 확인한다.
- 같은 Movie 배우의 분할 모델은 mutable CModel을 공유하지 않고 정확히 같은 channel 입력과
  시각의 local 보간 결과만 재사용한다. 이름이나 skeleton/hash만으로 입력 동일성을 판정하지 않는다.
  clone clock·unkeyed bone·root/blend/preScale·combined palette·parts는 각 모델 경로에 남긴다.
- 실제 설치 WModel에서 local/combined/inverse-bind palette를 대조하고 역방향·loop·서로 다른
  시계·후처리 오염·hash 충돌·동시 접근도 검사한다. Product SDK 배포·링크 및 실제 화면 검증은
  독립 native probe 성공과 구분한다.
  [공통 샘플 재사용 결과](10-02/2026-10-02_MOVIE_ANIMATION_SAMPLE_REUSE_RESULT.md).


### Bern 도착점의 walkable과 구역 연결성은 별도다

- 동일 source/paint를 재게시한 bytes가 같다면 다운로드 누락으로 설명하지 않는다. 도착 한 셀이
  walkable이어도33셀 고립 영역이면 이동할 수 없다. 현재 실제 geometry와 주변 성분을 대조하고
  native Find_Path의 bool뿐 아니라 exact 목적지와 smoothing 이후 도착점까지 확인한다.
- 기존 paint 차단·높이는 보존하고 실제 ground·장애물 근접으로 확인한 누락만 복구한다. foliage
  이름 전체를 장애물 예외로 삼지 않고 실제 허용 geometry를 검토한다. 고립된 roof·별도 실내·바다와
  authored travel로 오가는 detail region은 전부 하나의 보행 영역으로 합치지 않는다.
- navsurface는 공식 publisher의 Server 산출물이다. Client에 임의로 추가하지 않으며 source paint와
  Client/Server grid·Server surface를 한 변경으로 전달한다. 디스크 교체와 실행 중 Server Reload는
  별도 상태다. [검증과 범위](10-02/2026-10-02_BERN_NAV_GROUND_RECOVERY_RESULT.md).

### Profiler 비교의 미계측과 세션 실험 복원

- GPU pending·없는 구버전 필드·누락 scope를 측정된0으로 비교하지 않는다. 같은 thread의 scope가
  중첩 규칙을 위반하거나 부모가 없으면 inclusive 관측은 보존해도 self를 완전하다고 표시하지 않는다.
- 세션 옵션은 선택 필드만 복원하되 renderer가 OFF 처리에서 정규화한 보조 필드도 추적한다.
  특히 CShadow의 OFF는 눈/목표·범위·bias를 기본 descriptor로 바꾸므로 마지막 적용값과 같은
  보조값만 되돌리고 다른 편집·새 owner는 보존한다. PBR routing 정규화도 같은 원칙을 따른다.
- sweep의 A와 모든 측정점은 동일한 독립 변수 mask로 조건을 비교한다. 측정 종료 후 Profiler
  제어 소유권을 해제하여 사용자가 새로 시작한 수집을 나중의 실험 종료가 끄지 않게 한다.
  [구현·검증 범위](10-03/2026-10-03_PROFILER_RENDERING_WORKBENCH_RESULT.md)를 따른다.

### 화면 공간 조명 실험은 원본 HDR과 중간 결과를 분리한다

- SSGI와 SSR이 같은 프레임의 원본 opaque radiance를 읽도록 유지한다. 앞 패스의 간접광을 다음
  패스의 추적 radiance로 다시 읽으면 옵션 조합이 숨은 bounce/feedback을 만들어 A/B 의미가 변한다.
- 기존 SceneHDR MRT를 Begin_MRT로 다시 열면 누적 화면이 clear된다. 별도 scratch에서 완료한 뒤
  scene/bloom만 복사하고 원래 모든 MRT·DSV·viewport를 복구한다. 복사 전 SRV와 RTV를 unbind한다.
- MapPBR marker3의 depth/normal/roughness/metallic ABI에만 적용한다. 다른 family를 같은 layout으로
  해석하지 않으며 Lumen·DXR·시간 누적 지원으로 표시하지 않는다. 기본 OFF와 팀장 저장값을 보존한다.
- Profiler의 1Hz 메모리 샘플을 매 frame의 새 측정으로 세지 않는다. Valid인 0과 API 실패/N/A,
  process lifetime peak와 캡처 peak, DXGI node budget과 물리 VRAM 용량을 구분한다.


### 새 Engine shader의 실행 배포 경계

Engine FxCompile 등록과 CSO 생성만으로 Client에서 로드할 수 있다고 판정하지 않는다.
공용 include가 Engine/Bin/ShaderFiles에도 있으면 Engine 정본부터 수정한다.
Client 복사본만 수정하면 PrepareEngineSdk가 덮어써 독립 FXC와 제품 빌드 결과가 달라진다.
독립 shader는 Client `DeployClientCompiledShaders`의 명시 목록과 BuildDomains product의
필수 outputs·deploymentPairs까지 연결한다. 정상 Product 뒤 실제 EXE 옆 CSO와 Engine CSO의
크기·hash를 확인한다. 추적하는 HLSL source 배포와 Git 제외 compiled CSO 배포를 구분한다.

### 렌더링 A/B는 실제 선택 필드와 지원 범위를 함께 표시한다

기법 사전의 설명·공식 자료와 현재 실행할 수 있는 recipe를 구분한다. SSAO/GTAO,
Height Fog/Volumetric Fog, SSR/Planar처럼 지원 상태가 다른 항목을 같은 구현 상태로 묶지 않는다.
현재 값을 초기화하거나 FXAA를 강제로 켜는 preset을 A/B 시작이라고 부르지 않는다.
원본 PBR 간접광 ON에서는 cube diffuse 근사가 억제되며, LUT가 없는 장면의 LUT 토글과
MapPBR 전용 실험을 캐릭터에서 비교하는 경우 화면 차이가 없을 수 있다.
source postprocess는 tone·grading 묶음이므로 선택 bit만 복원/측정 제외하고 curve·LUT 원본 입력은
보존한다. LUT ON 검증은 원래 profile의 enabled만 보지 않고 동시에 적용할 세션 후보의
postprocess enabled를 사용해야 A/B 왕복과 기준 채택이 일관된다.

복원 시연의 기본 재질은 현재 asset의 비교이지 최초 임포트 EXE의 재현이 아니다. material selector도
실험 transaction이 소유해야 픽셀 진단과 두 owner가 겹치지 않는다. Native forward/hair 또는
diffuse 없는 program을 무조건 legacy로 내리면 오히려 표면이 사라진다. 확인한 deferred program과
유효 diffuse/override만 기존 fallback에 연결한다. 단계→한 기법 비교는 dimmed 화면이 아닌
Original에서 재구성하고, rebased A를 바꾸면 experiment ID를 분리한다.

누적 단계의 비용은 직전 단계 A와 현재 단계 B를 같은 mask로 측정한다. Original 대비 여러 기능을
끈 결과를 한 기능의 비용으로 표시하지 않는다. 행·측정 묶음·반복·실제 A/B 값·공통 조건이 같은
완료 표본만 짝짓고 누락 GPU는 N/A다. BG RNM·정적 shadow는 재질 단계에 포함되며 별도
MapPBR 간접 diffuse 배율은 RNM·SH·hemisphere 합계를 제어한다. Source postprocess OFF에서
보유 중인 dormant LUT를 단계마다 지웠다가 ON으로 소유하면 admission이 실패하므로 그대로
보존한다. 날짜별 자산 복구와 현재 옵션의 누적 시연을 구분한다.

원본 UE3 자료의 DX11/PBR/SH 존재는 UE4 이식의 증거가 아니다. UModel의 `.mat` export와
원본 ShaderMap/DXBC·runtime binding은 별도다. 옛 SH/BRDF 미복구 기록은 이후 native 입력 복원
결과와 대조한 뒤 인용한다. [현재 원본 근거](10-04/2026-10-04_RENDERING_SOURCE_EVIDENCE_RESULT.md)를 따른다.

### Object 목록의 진입 전 실패와 창 열림 상태를 구분한다

도구의 m_Open=true는 source load 성공이 아니다. Lobby/Loading에서 Arena가 없어 최초 로드를
미뤘으면 실제 CurrentLevelID와 active instance를 함께 확인한 뒤 한 번 재시도한다. Arena 생성자에서
설정한 포인터만으로 아직 구성 중인 validation target을 읽지 않는다. 파일/검증 실패의 frame별
I/O 반복과 기존 dirty draft 자동 재로드는 금지한다. 통합 Object 목록에도 Reload를 노출하여
도킹된 Action Workbench 탭이 숨겨져도 오류를 복구할 수 있게 한다.

### 큰 Composition의 미리보기 탐색과 UI 행 배치를 분리한다

시퀀서가 열린 상태의 긴 프레임을 ImGui draw 비용으로 바로 귀속하지 않는다. 현재 CPU scope의
BundleSample/Sample과 Timeline Layout/Draw를 분리하고, frameInterval은 이전 프레임의 CPU와
gap이라는 계약을 지킨다. scope cap에 도달한 프레임의 Self는 완전한 수치로 사용하지 않는다.

presentation occurrence마다 logic occurrence와 전체 definition을 중첩 탐색하지 않는다.
Sample 호출 안에서 현재 문서의 연결을 한 번 resolve하고, disabled/누락 정의·첫 definition·
마지막 overlapping window의 의미를 보존한다. 다른 호출까지 임시 string_view를 보관하지 않는다.
UI display row cache는 draft뿐 아니라 외부 World inventory, camera tail, 최소 box 시간의 변경을
포함한다. 선택·drag·marquee hit test와 픽셀 위치는 계속 현재 프레임 값을 사용한다.
함수 단독 benchmark의 개선량과 제품 Client의 실제 FPS를 구분한다.

### 시퀀서의 DPI 대응은 글꼴과 그리기·입력 영역을 함께 계산한다

`ConfigDpiScaleFonts`로 확대된 글꼴을 고정 24px 행/19px 박스에 넣으면 150% 이상에서 글자가
잘린다. 현재 ImGui 글꼴 높이와 style padding으로 행·박스·ruler를 계산하고 모니터 DPI를 다시
곱하지 않는다. 라벨 열은 실제 문자열 폭도 반영한다. 그리기·culling·InvisibleButton은 같은
사각형을 사용하며 시간→픽셀 zoom과 편집 시간값은 그대로 유지한다.

### Map shard 확인과 기존 재질 연결 보완

활성 배치는 mapset이 명시한 shard로 판정한다. 남아 있는 통합 placement 파일과의 차이를
현재 런타임 불일치로 처리하지 않는다. 원본 component용 variant와 editor 기본 asset은 key가
다르므로, 기본 asset의 named material 연결 누락을 따로 확인한다. 기존 복원 기본 MIC와 geometry
채널·DDS 동치를 확인한 뒤 non-baked 입력만 재사용하고 다른 placement의 RNM은 복사하지 않는다.
재질 연결 보완과 비균일 placement scale에 의한 UV 확대를 별도 문제로 기록한다.

### 보스 몸체 컷신 숨김과 독립 맵 이펙트

보스가 수명을 소유해도 유한 map/snapshot 이펙트는 몸체 컷신의 대체 배우가 아니다.
world-root adapter에서 map 분류를 잃지 않게 하고 초기 spawn·pending→active·product/preview
숨김 경로에 같은 제한 조건을 적용한다. 모든 map 또는 모든 tail을 예외로 만들지 않는다.
위치·시계와 cue/owner 종료는 기존 경로를 유지하며 additive alpha0을 무조건1로 바꾸지 않는다.

### 차원술사 V의 사용자 지정 시전자 기준

V2050520은 사용자 지정으로 시전자 snapshot/Local Space OFF를 사용한다. 원본 camera_view를
자동 복구하지 않는다. localSpace만 false로 바꾸면 카메라 attachment가 남으므로 follow,
orientation, runtime anchor도 기존 caster root 경로로 전환한다. 원본 recipe/socket은 근거로
보존하고 미사용 camera socket의 회전을 시전자 보정으로 옮기지 않는다. RGBNoise/ZoomBlur는
별도 localOnlyElementIds 네 개로 관전 대상을 제한하며 월드 요소는 다른 플레이어도 본다.


## 원본 이펙트의 import 축척과 실제 GPU 표시

WModel bone combined basis가0.01인 source skeleton에는 이미 meter로 변환한 particle 크기를
그대로 곱하지 않는다. 같은 클래스의 이전 skill이 정규화됐더라도 새 exact asset/slot이
`Requires_SourceBoneImportScaleNormalization`에 연결됐는지 실제 소비자까지 확인한다.
raw bone의 회전·translation을 유지하고 기존 검증된 정규화 경로를 사용한다. 전체 effect size나
shader gain을100배 올리는 보정으로 대체하지 않는다. 다른 source/actor의 basis까지 전파하지 않는다.
실제 WModel·애니메이션·admission preScale의 bone sample과 최종 particle world 축을 대조하며,
Stage/particle count/finite 성공은 GPU 표시 성공을 대신하지 않는다. 합성 카메라의 실제 draw와
인게임 구도·가림·후처리 판정도 분리한다. 이번 사례는09-09 Warlord ASVF RESULT G19에 있다.

본 배율을 정규화해도 축 방향은 남는다. 같은 shader를 쓰는 V/Alt V도 notify·socket·pivot·분포가
다르므로 한쪽 위치를 전부 복사하지 않는다. 머리 위 원점이 옆으로 밀리면 실제 본 basis에서
notify translation을 먼저 계산한다. Alt V SDenergy는 Y1.55m가 Z-1.55m로 바뀌어 exact notify의
18개 위치만 현재 본 공간으로 재표현했다. 전체 socket을 돌려 내부 분포·속도·mesh 방향까지
바꾸지 않는다. 기존 방패 주변 낙뢰의 승인된 ring 위치와 원작 offset 의미의 미확정 경계는
[워로드 결과 G23](09-09/2026-09-09_WARLORD_ASVF_FULL_RESTORE_IMPLEMENTATION_RESULT.md#g23-10-05-alt-v-시작-효과의-머리-위-원점-교정)에서 구분한다.

## 원본 데이터 동일성과 추가 GI A/B의 경계

재설치 시 logical object/static shader key/serial hash를 먼저 맞추고 export index와 UE reference를
구분한다. UV가 이상해 보여도 해당 VS/PS·geometry·height/weight mip·주변 coverage가 같으면 임의
triplanar나 hidden 해제를 원본 복원으로 넣지 않는다. 실행 중 material branch와 동일 카메라는 별도다.
원본 CDO 기본값과 사용자 삭제/튜닝·owner 정책을 구분한다. raw float32 반올림이나 의도적 삭제를
추출 손실로 취급하지 않는다. 코드·원본 자료가 같다는 것만으로 전체 화면 정상도 선언하지 않는다.

SSGI/SSR 옵션은 actual pass와 field whitelist/Apply/Restore/fingerprint/capture까지 연결한다.
SSGI half는 marker3 전용 추가 screen-space GI다. 기존 full 경로와 원본 RNM/IBL을 보존하고
Lumen/DXR로 표시하지 않는다. marker5/14를 조건문에만 추가하면 서로 다른 G-buffer ABI를 잘못 읽는다.
half texture는 홀수·1픽셀·resize·depth/normal 경계를 검증하고 full/SSR 보존을 실제 픽셀로 대조한다.

### 환경설정 텍스처 품질과 sampler owner

`텍스처 품질`의 4등급은 원본 mip 체인의 최소 레벨0/1/2/3으로 연결한다. sampler state와
마지막 적용 단계는 FX11 Effect와 같은 공유 owner에 보관하고, 실제 SourceCharacter variant의
Begin에서도 적용한다. base FX의 state만 변경하면 별도 light/geometry variant가 이전 품질을 쓴다.
인스턴스 표면의 LinearSampler를 포함하되 같은 이름의 UI/Deferred sampler에는 적용하지 않는다.
lightmap/lookup sampler는 BRDF와 roughness cube도 공유하므로 일괄 변경하지 않는다.
AnimMesh의 native ModelCue도 LinearSampler를 공유한다. `EffectModelCueNative*` 네 pass만
원본으로 복원하고 다음 표면 draw에서 사용자 품질을 다시 적용한다. 모든 ModelCue pass를
제외하면 일반 cue의 color0과 shadow15가 서로 다른 mip을 쓰므로 native 범위만 분리한다.
최상은 원본 sampler 자체로 복귀하며 texture mip 제한이 바뀌면 masked static shadow cache도
무효화한다. mip chain 복구와 샘플링 품질 선택, VRAM residency 절감은 별도 작업이다.

낮은 mip을 고르더라도 draw 수·재질 바인딩·shader 연산 수·상주 texture allocation은 줄지 않는다.
이미 화면 footprint가 더 작은 mip을 선택하거나 원본 sampler가 단일 mip을 강제하면 설정 차이가
작거나 없을 수 있다. 성능 비교는 같은 카메라와 도구 상태의 A/B를 사용하고 GPU timestamp의
경과 시간을 GPU 사용률로 읽지 않는다. 조명 OFF만으로 불투명 재질·RNM·이펙트 비용이 사라지지 않는다.
LiveCompare의 Directional OFF는 diffuse/specular RGB를0으로 바꾸는 기여 비교이며 ambient와
light 제출은 유지한다. 이 결과를 조명 pass 제거 실험으로 부르지 말고 실제 light draw와 시간을 확인한다.

### 느린 frame의 배경 이펙트 따라잡기와 카메라 속도를 구분한다

ambient fixed-step60회는 효과60개 생성이나 카메라 속도의 직접 증거가 아니다. 실제 delta와
누적 잔량을 확인한다. 베른 입장 카메라는 이미 frame당0.1초 진행 제한이 있으므로 같은 넓은
구도의 draw 비용과 배경 simulation 비용을 나눠 본다. 기존 offscreen pause 승인을 통과한
독립 source-loop 배경만 rate 적용 후 visual delta를0.1초로 제한하며 object와 service elapsed에
같은 값을 전달한다. 초과 시간은 다음 frame에 보관하지 않는다. 이는 과부하 동안 시각 재생을
느리게 하는 정책이며 원본 wall-clock phase 보존이나 실제 FPS 개선 완료로 설명하지 않는다.
combat/history/authoring의 fixed-step·Seek는 변경하지 않는다. fixed-step clock은 잔량도 포함하므로
실행 횟수는 실제 simulation step 정수의 차이로 계측한다. 제외 시간은 effect별 합계로,
frame wall time·절약 CPU 시간과 다르다. 수치와 재측정 경계는
[베른 제출 비용 결과](10-04/2026-10-04_BERN_DRAW_SUBMISSION_OPTIMIZATION_RESULT.md)를 따른다.

### 원본 DDS mip 누락과 확대된 무늬를 구분한다

원본 mip0가 설치 DDS와 같아도 전체 texture 복원이 끝난 것은 아니다. 제품 DDS loader가
context 없는 CreateDDSTextureFromFileEx를 쓰면 단일 mip를 자동 보완하지 않는다.
원본 압축 mip chain을 회수하고 mip0·모든 하위 block 및 실제 GPU Texture/SRV mip 수를 대조한다.
최고 해상도와 UV가 같은 mip 복구는 축소·경사 샘플링 입력 복구이며 확대된 배치의 무늬 크기나
사용자 화면의 흐림 원인을 함께 해결했다고 기록하지 않는다. 실제 교체·화면 경계는
[발탄 대기 돌 결과 G05·G06](10-04/2026-10-04_VALTAN_WAITING_STONE_MATERIAL_RESULT.md)에 둔다.

원본 bulk의 압축 여부는 저장 길이와 해제 길이의 일치가 아니라 native flags로 판정한다. texture flags 0은 raw 길이를 검증하고 0x80은 길이가 같아도 LZ4 컨테이너를 해제한다. 알 수 없는 flags를 raw로 간주하지 않는다.
GPU readback이 DDS와 같다는 결과는 추출 오류까지 검출하지 못하므로 native block을 따로 대조한다.
source/output hash만 있는 예전 캐시는 같은 decoder 결함을 보존할 수 있다. Landscape G8 shadow는
캐시를 재사용하지 않고 원본 flags로 재해석하며, 이전 receipt의 원본 일치 주장은 교정 이력과 함께 읽는다.

### Landscape의 NoLightmapPolicy 대조를 전체 복원으로 판정하지 않는다

- 원본 LandscapeComponent의 native FLightMap2D·ShadowMap 참조가 있으면 같은 static material의 lightmapped policy도 조회한다. layer D/N·weight·height와 NoLightmapPolicy가 일치해도 실제 component 조명 입력이 누락될 수 있다.
- Landscape lightmap 좌표는 원본 CPU grid padding과 subsection 계산 뒤 atlas scale/bias를 적용한다. 현재 painted WModel의 UV0로 환산하여 기존 placementLighting에 합성하며 UV1의 존재나 raw UV0 자체를 원본 lightmap 좌표로 대신하지 않는다.
- pixel Heightmap BA에서 복원한 TBN으로 RNM의 view-dependent 입력을 계산한다. VS가 운반하는 inverse-transpose world axes는 그 지형의 pixel tangent basis가 아니다.
- StaticMesh와 foliage 배치 전수 대조에도 DecalComponent는 포함되지 않는다. 누락 데칼은 원본 receiver·투영 footprint·가시성을 따로 확인하며, 근처에 있다는 이유만으로 특정 픽셀 증상의 원인이라 하지 않는다.

### 같은 ParticleSystem의 일부 emitter 회전만 복구하지 않는다

원본 notify에 저장된 FRotator는 해당 발생의 ParticleSystem 전체에 적용되는 입력이다. 같은 source event 아래 sprite·mesh·파편·먼지를 모두 세고 각 carrier의 위치·방향 소비자를 대조한다. 바닥 띠만 방사형으로 고쳐도 같은 발생의 돌 메시가 rotation0이면 한 방향에 겹칠 수 있다. generic typed 결과가0이어도 raw payload에서 named anchor 뒤의 native FRotator를 지원하는지 확인하며, 원본의0이라고 단정하지 않는다. 이미 복구한 emitter에는 같은 보정을 다시 적용하지 않는다. 실제 설치 본의 scale·basis와 socket을 함께 확인하고 수명·크기·TypeData pre-rotation·사용자 localSpace 정책을 보존한다. 구체적인 대상과 검증은 Warlord 복원 RESULT의 해당 절에 남긴다.

### 복원 대상 메시 선택은 authoring AABB 추정과 분리한다

Bern/Character Select/Valtan/Kouku의 원본 렌더링 대조는 독립 F1 `World Scene Tool → Pick in scene`의 실제 삼각형 hit에서 placement/mesh/material ID를 함께 확보한다. GPU world-position을 가장 작은 포함 AABB로 해석한 결과는 exact mesh 증거가 아니다. 기존 CModel LOD0 CPU 질의와 현재 instance world/visibility/suppression을 소비하고, 이동 피커의 식생 제외 조건을 검사 피커에 전파하지 않는다. alpha coverage·shader wind/displacement·rendered LOD와 CPU hit는 별도다. 선택 자체는 데이터를 변경하지 않는다. 명시적 TRS/표시 편집은 기존 session을 사용하고 미로드 행·dirty draft·최신 source bytes를 보존한다. writer의 고유 temporary를 stage한 뒤 교체 직전 freshness를 다시 확인하며 stale 바이트는 덮어쓰지 않는다. self-motion/Deploy preview는 자기 Begin 성공과 host/runtime generation 소유권을 확인한 뒤에만 복원한다. Server 파괴는 Deploy preview를 정상 종료한 후 선점한다. rendering options는 변경하지 않는다. UI click과 취소 입력은 gameplay에 전달하지 않고 miss는 이전 선택을 보존한다.

live 맵의 placement 편집을 열 때 큰 material JSON과 재질 벡터를 다시 복제하지 않는다. source metadata·material bytes·typed water 입력을 이미 로드한 runtime과 대조한 뒤 immutable payload의 소유권을 공유한다. 검증 실패는 읽기 전용으로 남기고, 새 runtime 소유자로 바뀌면 기존 draft를 보존한 채 편집을 해제한다. prototype 재바인딩은 copy-on-write로 기존 material·lighting·wind 입력을 보존한다.

### 원본 wind phase와 공유 draw state를 보존한다

식생의 actor position·primitive bounds는 source component owner 입력이다. placement TRS로
대신하면 native wind phase가 달라진다. FBox extent/radius의 +1cm padding과 BoundsScale 순서,
ordinary/instanced surface·shadow의 동일 carrier를 확인한다. native E4FE/1C39/098C 연산과 원본
zero wind/noise를 각각 유지하며 DXBC replay 수치와 실제 화면 판정을 분리한다.
공유 Effect clone에 preview 전용 opacity를 bind했으면 draw 뒤 기본값으로 되돌린다. state만
검사하지 말고 valid destruction debris/suppression의 commit 직전에도 preview를 정상 종료해
Server의 동일-state 파괴 burst가 preview root를 유지하지 않게 한다.

### Mario World 생성 준비와 캐시 수명

모델 사전 로드와 실제 WorldObject clone 준비를 구분한다. 고유 motion ID 집합은 반복 occurrence의 수량을 잃으므로 발생 행과 EmissionCount를 함께 세며, NEXT/APPLY_TARGET를 새 spawn으로 합산하지 않는다. Hide는 pool 반환이 아니고 기존 owner 종료가 반환 시점이다. 인형·공은 이미 준비한 clone 수가 충분할 수 있으므로 부족을 추측하기 전에 실제 수량을 대조한다. 이미 검증한 object-only 재생 subset을 재사용할 때는 owner·level·device/context/catalog·Area/revision을 맞추고 동일 revision 문서 교체에서도 폐기한다. 재생 전 준비 개선을 GPU draw/FPS 성공으로 대신 기록하지 않는다. 근거는 `10-04/2026-10-04_MARIO_WORLD_PREWARM_IMPLEMENTATION_RESULT.md`다.

### 원본 복구 A/B와 추가 품질 실험의 기준을 섞지 않는다

원본 복구의 성분별 A/B는 시작 때 보관한 실효값을 기준으로 source material·PBR indirect·
RNM/SH·IBL·source tone/LUT 중 한 필드만 비교한다. 현재 연결된 profile/resource ID는
입력 근거이며 원작 화면 일치 인증이 아니다. 고정 복구된 UV/geometry/mip를 성분 OFF로
과거 결함 상태까지 복원했다고 설명하지 않는다. 추가 Horizon AO/SSR 같은 자체 확장과
통합 후보는 명시적인 별도 session 실험이며 기본ON·저장/publish로 전파하지 않는다.

패키지를 개별 A/B의 기준으로 사용할 때 original 복원값을 덮어쓰지 않는다. 서비스 ownership은
original 대비 A와 B 차이의 합집합, 비교 제외 mask는 A와 B의 실제 차이만 사용한다.
선택 기준별 row ID·실제 expected A/B·공통 조건을 대조하여 다른 기준의 비용을 재사용하지 않는다.
행별 ID가 없는 일반 캡처도 A 기준이 성공적으로 바뀔 때 새 experiment ID로 분리한다.
거부된 기준 변경은 이전 ID를 유지하고, 같은 A에서 B만 바꾸는 실험과 구분한다.

Horizon AO에서 depth는 실제 point sample의 texel-center UV로 재구성하고 표본의 실제
방향에서 tangent를 구한다. 비정사각 viewport의 texel 비율을 빠뜨리거나 unquantized UV를
point depth와 섞으면 평면·경사면이 스스로 어두워진다. OFF 보존과 평면1/접점차폐를 함께
수치 검증한다. SSR 세부 탐색은 연속 표면의 실제 교차만 복구하며 depth 절벽을 hit로 만들지
않는다. Roughness 필터도 depth/normal/family 경계를 검사한다. 실제 게임 화질·비용은 별도다.
근거는 [Workbench 결과](10-03/2026-10-03_PROFILER_RENDERING_WORKBENCH_RESULT.md)를 따른다.

### 카메라 행렬 오차와 인접 draw의 LOD 선택을 구분한다

inverse view의 `_44`를 정확히1과 비교하면 affine 표현의 float 오차 때문에 원거리 LOD가
건너뛰어질 수 있다. camera revision당 LOD 전용 view를 준비하고 같은 homogeneous scale로
center·radius·error를 일관되게 계산한다. 원본 GPU view와 culling plane은 바꾸지 않는다.
projective/비유한 행렬은 계속 거부하고 source LOD로 돌아간다. 실제 cue 재현과 화면 오차를
함께 확인하며 생성 하한을 일괄 낮추기 전에 실제 asset의 감소율과 준비 비용을 측정한다.

같은 mesh의 인접 lighting bank도 각 원래 batch가 선택한 LOD가 같을 때만 합친다. 서로 다른
선택은 순서 경계로 남긴다. 작은2~3개 묶음은 별도3slot shader로 SRV 바인딩 비용을 제한하며
기존4~8개 경로와 ordinary shader의 비용을 늘리지 않는다. native 수치 정합·실제 제출 비용과
사용자 컷신 FPS 검증은 구분한다.

텍스처 품질의 빌드별 기본값은 초기 로드 fallback과 UI seed·Reset에서 같은 함수를 쓴다.
Debug 하/Release 최상은 저장값이 없는 경우의 기본값이며 기존 명시 저장값을 강제하지 않는다.

### 실제 typed texture와 원본 mip 근거를 먼저 연결한다

WModel의 legacy texture 복사본이 단일 mip이어도 현재 mapmaterials의 typed override는 다른
DDS를 사용할 수 있다. 실제 asset/material/texture field에서 Resources ID를 따라간 뒤
원본 MIC 상속과 Texture2D를 연결한다. full mip count도 native 복원 증거가 아니며 생성된
하위 단계일 수 있다. 원본 mip0뿐 아니라 모든 단계의 압축 blocks를 비교한다.

원본 native height가64/32/16/8/4까지만 보관하면 없는2/1단계를 복원 명목으로 생성하지 않는다.
DX10 DDS의 format/sRGB와 기존 channel order를 유지하고, 원본 mip0 불일치·source object
모호성·지원하지 않는 carrier를 임의 재압축이나 이름 추정으로 통과시키지 않는다.
Resources 설치와 현재 GPU 메모리 갱신, sampler 품질 선택과 VRAM streaming은 구분한다.
원본에 `TMGS_NoMipmaps`가 명시되고 native1단만 있으면 생성된 하위 단계를 원본 복원으로
남기지 않는다. 실제 소비자·mip0·형식·색 공간을 확인하고 원본1단 정책을 별도로 복구한다.

### 원거리 캐릭터 무대는 지면과 조명 기준점을 함께 이동한다

미리보기 카메라만 이동하거나 지면을 복제해도 중앙의 POINT/SPOT·그림자 영역은 따라가지
않는다. 설치 geometry의 실제 상면과 기본 캐릭터 발 원점을 대조하고, loaded placement의
material override·RNM·sourceWind를 보존한 기존 Clone으로 지면을 준비한다. 조명 색·강도·
방향·품질은 보존하면서 POINT/SPOT 위치의 frustum 검사와 제출, 그림자 eye/at를 함께
평행이동한다. 그림자는 이전 자기 offset을 제거한 뒤 계산해 프레임 누적을 막는다. 현재
저작 재질을 과거 원본 brightness 값으로 덮지 않는다. Server entity를 이동하지 않는 표시
캐릭터는 정상 ObjectUpdate가 한 번 지난 뒤 보여주고 전환 시 실제 플레이어의 이전 숨김을
복원한다. 지면 수치·컴파일 성공과 사용자의 화면 확인은 별도다.

### Rendering Workbench의 옵션 연결과 촬영 적용을 구분한다

before-restoration은 저장 scene profile 비교이며 current shader/assets를 과거 EXE로 되돌리지
않는다. 이 profile 비교가 활성인 동안14단계 실험은 시작할 수 없으므로 현재 entry로 복귀한 뒤
별도 시작한다. 각 Technique B는 직전 B 위에 누적되지 않는다. Debug 전용 Workbench를
Release에서도 쓸 수 있다고 안내하지 않고, F1 숨김·Level/profile·region/Video 변경에 따른
실험 종료를 촬영 절차에 반영한다. profile 복귀 실패 때 entry ID가 지워지는 현재 결함과
일반 A/B fixture 통과는 별개이며, 복귀 성공 전 복원 정보 폐기를 재사용하지 않는다.

SSGI/SSR marker3 수신 조건과 BG RNM의 AO 우회, FXAA subpixel0 조기 반환까지 확인한다.
버튼 값이 바뀌거나 패스가 실행된다는 이유로 해당 바닥의 화면 변화까지 보장하지 않는다.
게시 material override 행 수는 실제 load scope/픽셀·상속 WModel·동적 배우의 전수 집계가 아니다.
촬영 적용 범위와 미연결 항목은 [Workbench 촬영 결과 G10](10-04/2026-10-04_RENDERING_WORKBENCH_PRESENTATION_RESULT.md)에 둔다.

### 선택창 크기 차이는 기본 체형과 대기 자세를 따로 측정한다

canonical mesh geometry, 설치 골격의 저장 rest pose, 실제 기본 idle의 skin 경계는 서로 다르다.
워로드처럼 원래 몸체가 더 커도 전투 idle에서 높이가 크게 낮아질 수 있다. rig root 공통 basis와
runtime preScale·presentation 배율을 함께 적용하고, 큰 무기를 포함한 전체 경계로 몸체 크기를
대신하지 않는다. 선택창의 구도 문제는 전투 world 배율을 바꾸기 전에 전용 카메라에서 보정한다.
거리·eye/look 높이를 같은 비율로 줄이면 pitch와 캐릭터 원점의 화면 위치를 보존하지만 깊이가
다른 실제 발끝은 조금 움직인다. 몸체 CPU 계산과 장비·cloth를 포함한 실제 화면을 구분한다.
근거는 [검은 무대 결과 G04 이후](10-05/2026-10-05_CHARACTER_SELECT_BLACK_STAGE_IMPLEMENTATION_RESULT.md)에 둔다.


### 최적화 A/B의 cache 수명과 증거

- 최적화 bool을 rendering quality의 매 프레임 base 복원/preview 재적용에 넣으면 ON cache가 계속 무효화된다. 구조 설정은 entry base·last applied·owned mask를 별도로 보관하고 실제 delta에만 setter/revision을 바꾼다. 종료는 최신 외부 변경을 보존하면서 남아 있는 소유 필드만 복원한다.
- 같은 geometry의 instance1 draw와 instanced draw 비교는 제출 경로의 비교다. 객체 생성·map partition·material 정렬·LOD 생성·asset bake 비용까지 꺼진 것으로 표현하지 않는다. shadow cache OFF는 shadow OFF와 다르다.
- 고정 카메라 warmup은 visibility cache를 재사용해 worker 준비 대상0이 될 수 있다. 대상0·cachehit·실제 worker 완료량을 보고하며 시간 차이만으로 병렬화 효과를 확정하지 않는다. Debug/Release와 D3D debug device는 각각의 실행 조건이며 재시작만으로 OS cache cold를 보장하지 않는다.
- ABBA의 각 단계 raw frame window는1200frame history에서 사라지기 전에 따로 보관한다. 다음 측정은 저장 완료 뒤 warmup하고 summary에 measurement ID·실제 범위·raw 저장 성공을 연결한다. 중단·저장 실패·도구 표시/카메라 변경·GPU 미유효 표본을 정상 비교 수치로 제시하지 않는다.


### 용병 AI의 후보·종료·회피 예측은 native 실행 계약과 대조한다

- 후보의 resource 필드를 모두 hard gate로 해석하지 않는다. Guardian emberCost는 보유량만큼 소비해
  피해를 강화하는 값이며 잔불0에서도 native 시전을 허용한다. AI가 더 엄격한 제한을 만들면 안 된다.
- top-level actionDurationMs는 native COMBO/HOLD의 실제 종료 시각이 아니다. 실제 action state와
  입력창으로 진행하고, nominal duration을 별도 공격 잠금으로 다시 적용하지 않는다.
- SPACE aim은 방향이며 aim까지 이동하는 거리 제한이 아니다. 원본 root-motion의 초기 offset,
  forward/lateral, scale, stage/release를 시간별로 읽어 실제 경로와 착지를 평가한다. 위협 cache의
  경과 시간도 경로 샘플에 반영하며 예측을 별도 이동·피해 실행기로 만들지 않는다.
- 거절된 Move가 이전 goal을 보존할 수 있으므로 hasMoveGoal만으로 새 회피 성공을 판정하지 않는다.
  기존 executor의 결과 goal과 요청한 안전 후보를 비교한다. STANDUP도 실제 published 정의가 있는
  직업만 검사하며 미정의 기상기를 합성하지 않는다.
근거는 [콜로세움 용병 결과 G10](10-01/2026-10-01_COLOSSEUM_PVP_MERCENARY_RESULT.md)에 둔다.
