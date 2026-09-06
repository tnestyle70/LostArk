# 쿠크세이튼 1스테이지 피날레(서커스 광장) 장면 구현 설명서

원본 영상: 쿠크 처음시작 스트림 클립 24.7초 (0:56~1:20 구간).
종이 편지 다리를 건넌 파티가 금색 액자 프레임을 통과해 서커스 광장까지 걸어가는,
1스테이지 마지막 연출 전체를 다룬다. 이 문서는 배경 이미지, 카메라, 지형 세 축을
현재 저장소 실측과 함께 단계(G)로 나눈 구현 설명서다.

## 1. 영상 실측 — 장면이 무엇으로 구성되는가

| 시점 | 관찰 내용 |
|---|---|
| 0:56 | 파티가 펼쳐진 편지(종이 다리) 위. 바닥엔 대형 트럼프 카드, 서커스 문양 원형 바닥. 카메라는 일반 쿼터뷰(높은 부감) |
| 1:02 | 거대한 펼친 책 위. 정면의 금색 장식 액자(문) 너머로 초원·산 원경이 보임. "서커스 광장으로 이동 완료" 메시지 |
| 1:04 | 액자 통과 직후 카메라가 **낮은 수평 앵글로 전환**. 좌우에 종이 공작(paper-cutout) 산, 청록 하늘, 중앙 원경에 빨간 서커스 천막, 우측에 종이 용. 화면 가장자리를 빨간 극장 커튼이 프레임처럼 감쌈 |
| 1:13~1:18 | 서커스 광장 파노라마: CIRCUS 대형 천막, 우측 **대관람차**(전구 장식), 줄무늬 파빌리온 천막들, 전구 스트링, **불꽃놀이**, 붉은 초승달, 밤하늘·구름, 우중앙 **회전목마**(원형 링 위 목마들), NPC 엘리자베스. 파티는 화면 중앙 하단에 작게. 카메라는 낮은 수평 와이드를 유지하며 파티를 따라감 |

핵심 관찰 세 가지.

1. 화면 가장자리의 극장 커튼은 UI가 아니라 **맵에 배치된 메시**다. 낮은 카메라가
   커튼 사이를 지나가도록 배치되어 화면 프레임처럼 보인다.
2. 카메라는 컷신이 아니라 **플레이어를 계속 따라가는 게임플레이 카메라**다. 이 구간에서만
   pitch가 수평에 가깝게 내려가고 거리가 멀어진다(지역 오버라이드).
3. 배경의 산·하늘·달은 원경 배경 메시이고, 대관람차·회전목마·불꽃만 움직인다.

## 2. 저장소 실측 — 이미 있는 것과 없는 것

`Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/` 기준.

이미 임포트되어 배치까지 있는 것 (mapplacements 2,951행):

```text
curtain(극장 커튼)  배치 114행 / mapassets 72종   <- 화면 프레임 커튼 이미 있음
tent(천막)          배치 44행
circus(서커스 구조) 배치 20행
rock 등 지형 데코    mapassets 6종+
```

임포트에서 빠져 source-only 인벤토리로만 남은 것
(`LV_LUT_MIDNIGHTC_ED.nonstatic.inventory.json`, runtimeAdmission: source-only):

```text
motionOrInterpActors 377   <- 대관람차, 회전목마 등 움직이는 액터가 여기 속함
particleEmitters      54   <- 불꽃놀이 등 (uniqueParticleSystems 9)
lights               116   <- 광장 전구/조명
decals                33, fog 1, waterComponents 7, foliageActors 2
```

- 원본 소스 manifest 물리 경로는 receipt에 `C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/`로
  기록되어 있으나 현재 이 PC의 해당 폴더는 비어 있다(추출본이 이동/정리됨). G2 진행 전에
  추출 폴더 위치를 다시 확보하거나 lpk에서 재추출한다. 재추출 절차는
  `.md/GB/07-29/2026-07-29_LOSTARK_MAP_ASSET_EXTRACTION_RUNTIME_RESULT.md`를 따른다.
- 스케일 3단 계약(UModel glTF meter x converter 100 x Loader 0.01)은
  `.md/GB/07-29/gotchas.md`가 정본이다. 스킨드 자산은 converter `--scale` 대신
  `Tools/ActorXAssetCooker/build_umodel_gltf_psa.py --scale 100`을 사용한다(메시·본·애니 키 동시 스케일).

이번 작업에서 재사용할 기존 도구·계약:

```text
World Sequence 툴      transform track(위치/회전/크기/표시, LINEAR/SMOOTH, loop)
                       + Deploy ANIM animation track. 저장은 <Area>.worldsequences.json
Deploy ANIM 파이프라인  Cook-KakulInteractionProps.ps1, build_umodel_gltf_psa.py,
                       retime_wmodel_ticks.py(30tps), mirror_wmodel_bend.py(굽힘 반전)
카메라 코드             CameraTool(컷/포즈 캡처), ValtanCinematicCameraController/Document
                       (레벨 진입 1회 카메라 큐 + ESC 스킵), Camera_Free(F6), CameraShakeService
내비게이션             <Area>.navsource/.navpaint(v3 HEIGHT)/.navblockers
                       -> Publish-ServerNavigation.ps1 -> 서버 .navgrid/.navpolicy
포인트라이트 표현       MapCatalog optional sourceLights/lights pair (Valtan에서 가동 중)
```

## 3. 요소 -> 시스템 매핑

| 화면 요소 | 담당 시스템 | 현재 상태 |
|---|---|---|
| 극장 커튼 프레임, 천막, 서커스 구조물, 종이 산 | 정적 mapplacements | 대부분 임포트됨. 확인·보정만 |
| 대관람차·회전목마 회전 | 정적 메시 + World Sequence 회전 loop, 스켈레탈이면 Deploy ANIM | 미임포트(377 motion 중) |
| 하늘·붉은 달·구름 | 원경 배경 메시(스카이 돔/플레인) | 미임포트. 확인 필요 |
| 전구 스트링·광장 조명 | maplights pair | 미선언(116 lights source-only) |
| 불꽃놀이 | Effect 시스템(독립 Effect) | 범위 분리. Effect 담당 계약 |
| 낮은 수평 카메라 | 지역 카메라 오버라이드(신규 소계약) | 신규 구현 필요 |
| 종이 바닥·흙길·광장 지형 | nav bake + height override + collisionBox | 기존 파이프라인 그대로 |

## 4. G1 — 정적 배경 확인·보정 (커튼·천막·산)

목표: 서커스 광장의 정적 배경이 원작 배치대로 보이는 상태. 종료 증거: 에디터에서
광장 방향 화면이 원작 스크린샷과 구조적으로 일치.

1. Map Editor -> `KoukuSaton / MidnightC ED` -> 서브레벨 점프 버튼(SL01~SL05)으로 광장 구역 이동.
   광장은 배치 수가 가장 많은 SL02(원본 1,357행) 일대다.
2. 원작 프레임(1:13)과 비교해 세 종류의 결함을 구분한다.
   - 회색 메시 = 텍스처 미해석. `.md/GB/07-29/gotchas.md`의 diffuse 복구 절차.
   - 없는 구조물 = 미임포트. mapassets에 에셋이 있으면 배치만, 없으면 추출부터.
   - 형광/보라 발광 = 머티리얼 채널 문제. 같은 gotchas 절차.
3. 보정은 전부 `Data/Maps/Authoring` 저작 + `Tools/MapPipeline/Publish-MapAuthoring.ps1`
   publish로만 반영한다. `Client/Bin/DataFiles`를 직접 수정하지 않는다.

## 5. G2 — 대관람차·회전목마 (움직이는 배경)

목표: 광장 우측 대관람차가 회전하고 회전목마가 도는 상태(툴 미리보기 기준).

1. 추출: UModel로 원본 UPK에서 해당 메시를 찾는다. 후보 검색어는 wheel, carousel,
   merry, horse, round. 07-29 RESULT의 `umodel_lostark_v7.exe -export -game=lostark -kr` 절차.
2. 형태 판정 후 두 경로 중 하나.
   - **정적 메시 + 회전만 필요**(대관람차 몸체가 한 덩어리): ModelAssetConverter로
     `.wmodel` 쿠킹 -> mapassets 등록 -> MapTool 배치 -> World Sequence `New Sequence`
     (transform)로 회전 loop 저작.
   - **스켈레탈/자체 애니 보유**(회전목마 말이 위아래로 움직이는 형태):
     paperstage와 동일한 Deploy ANIM 경로. `build_umodel_gltf_psa.py --scale 100`
     -> `retime_wmodel_ticks.py --ticks-per-second 30` -> deployassets 등록 -> 배치 ->
     animation track loop.
3. **회전 loop 저작 주의(쿼터니언 함정)**: 회전 키 2개(0도, 360도)는 같은 값이라 돌지 않는다.
   키 4개를 균등 배치한다.

```text
key 0ms      rotation   0도
key 1/3 t    rotation 120도
key 2/3 t    rotation 240도
key t(끝)    rotation 360도(=0도)   interpolation LINEAR, loop 켬
```

   대관람차 1회전 원작 감각은 20~40초. duration 30000ms 시작점 추천.
4. 검증: World Sequence Play로 회전 확인. 축이 어긋나면 배치 rotation이 아니라
   키의 회전축(Y=수평 회전목마, X 또는 Z=수직 대관람차)을 바꾼다.

## 6. G3 — 하늘·붉은 달·구름

목표: 광장 뒤 밤하늘, 붉은 초승달, 구름 원경.

1. 원본에서 sky/moon/cloud 이름의 배경 메시를 추출 검색한다. 이 Area는 실내
   무대(종이 세계)라 스카이 돔이 아니라 **대형 배경 플레인/곡면 메시 + 발광 텍스처**일
   가능성이 높다.
2. 찾으면: 정적 메시 경로(쿠킹 -> mapassets -> 배치). 원경이므로 광장 뒤 먼 위치,
   카메라 최대 거리에서도 잘리지 않는 크기를 확인한다.
3. 원본 메시를 못 찾으면: 추출된 하늘 텍스처를 입힌 대형 곡면 플레인을 직접 배치하는
   대체안. 이때도 새 런타임을 만들지 않고 일반 map asset으로 등록한다.

## 7. G4 — 광장 조명 (전구 스트링·야간 무드)

목표: 원작 116개 광원 중 광장 구간을 maplights 계약으로 재현.

1. `Data/Maps/MapCatalog.json`의 MIDNIGHTC 항목에 Valtan과 동일한
   `sourceLights`/`lights` pair를 선언한다(둘 중 하나만 선언 불가).
2. `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.maplights.json`을
   만들고 원본 인벤토리의 광원 중 광장 주변을 선별해 위치·색·반경·밝기를 옮긴다.
   Valtan 문서(SL04 22개)가 형식 참고본이다.
3. `Publish-MapAuthoring.ps1`이 visual placement와 한 트랜잭션으로 publish한다.
4. 경계: PointLight는 주변광만 만든다. 전구 알 자체의 발광(visible sprite)은 별도
   미복구 표현이며 이 G의 완료 조건이 아니다(Valtan과 같은 경계).

## 8. G5 — 카메라 (쿼터뷰 -> 낮은 수평 앵글 전환)

목표: 파티가 액자를 통과하면 카메라가 낮은 수평 와이드로 전환되고, 광장 구간 내내
유지되며, 구간을 벗어나면 쿼터뷰로 복귀.

현재 실측: CameraTool은 컷/포즈 캡처 저작 도구, Valtan/Bern 진입 시네마틱은
**1회성 재생 큐**다. "플레이어가 영역 안에 있는 동안 유지되는" 지역 오버라이드는 없다.
새 소계약이 필요하며, Client 표현 전용이라 Server 변경이 없다.

데이터 계약(신규): 저작 위치 `Data/Maps/Authoring/<AreaId>/<AreaId>.camerazones.json`

```text
zone 항목: stable zoneId, OBB(center/halfExtents/yaw),
          pitchDegrees, yawMode(FREE|LOCKED+yawDegrees), distance, fovDegrees,
          blendInMs, blendOutMs, priority
```

런타임 흐름(신규 구현 지점):

```text
gameplay follow 카메라 update
-> 내 플레이어 위치로 zone 판정(겹치면 priority 높은 것 하나)
-> zone 있으면 pitch/yaw/distance/fov를 blendInMs로 보간해 오버라이드
-> zone 이탈 시 blendOutMs로 기본 쿼터뷰 복귀
-> F6 free camera 중에는 zone을 적용하지 않음, 시네마틱 큐 재생 중에는 큐가 우선
```

영상 실측 기반 저작 시작값(눈 튜닝 전제):

```text
쿼터뷰(기본)      pitch 약 -55도
광장 zone        pitch -10 ~ -15도, distance 30~40m, fov 기본 유지,
                 yaw LOCKED(광장 정면 방향), blendIn 1500ms, blendOut 1000ms
액자 통과 복도    같은 zone을 복도 시작점까지 늘려 액자 통과 순간 전환 시작
```

로드는 parse -> validate -> stage -> commit, 실패 시 기본 카메라 유지. zoneId는
stable ID이며 포인터/인덱스 저장 금지. 저작 UI는 기존 CameraTool 화면에 zone 목록과
현재 카메라 캡처 -> zone 값 채우기 버튼을 추가하는 것이 가장 작다.

구현 파일(예상 범위): CameraTool.h/cpp(저작), 신규 CameraZoneDocument.h/cpp(문서),
gameplay 카메라 소유 코드(오버라이드 적용). 이 G를 시작할 때 해당 파일들을 실측한
뒤 전체 코드 PLAN을 별도로 확정한다.

## 9. G6 — 지형과 이동 (플레이어가 밟는 것)

원작 실측: 걷는 면은 세 구간 전부 평면이다. 책/편지 종이 바닥 -> 흙길(완만한 내리막)
-> 광장 평지. 절벽 같은 낙차는 없고 경계는 커튼·구조물이 막는다.

1. 걷는 면 자체는 이미 배치된 정적 메시다. 별도 지형 시스템이 없으므로 **서버
   내비게이션이 곧 지형**이다.
2. MapTool Navigation 모드에서 광장 구간 bake -> `.navsource` 갱신.
3. 이음매·다층 오선택은 `.navpaint` v3 HEIGHT override로 교정한다. 이 Area는 이미
   v3이고 override 2행(486,424 / 485,427)이 있다. 같은 방식으로 광장 셀을 추가한다.
4. 통행 금지 경계(커튼 뒤, 천막 안, 무대 밖)는 `.navblockers` + 필요 지점의
   `collisionBox`(Gameplay.world.json, 서버 swept 차단)로 막는다.
5. `Tools/Navigation/Publish-ServerNavigation.ps1` publish -> 서버 재시작 ->
   우클릭 이동으로 세 구간 연속 주파 확인.
6. 종이 다리 -> 광장 낙차가 생기면 기존 jump triggerBox 패턴(jump.1~3,
   movePlayer arc)을 재사용한다.

## 10. 구현 순서와 검증

순서는 의존성 기준이다. 이동이 먼저 뚫려야 나머지를 걸어다니며 확인할 수 있다.

```text
G1 정적 배경 확인   -> 에디터 화면 vs 원작 1:13 프레임 구조 일치
G6 지형/이동        -> 서버+클라 실행, 우클릭으로 책->길->광장 연속 이동
G2 대관람차/회전목마 -> World Sequence Play로 회전 확인
G3 하늘/달/구름     -> 광장에서 원경 프레임 비교
G4 조명            -> publish 후 야간 무드 확인 (PointLight 주변광 경계 유지)
G5 카메라          -> zone 진입/이탈 blend, F6/시네마틱 우선순위, 영상과 앵글 비교
```

공통 검증: 각 G 종료 시 `git diff --check`, 변경 domain publisher Validate,
Debug Product 빌드. 화면 판정은 전부 사용자가 실제 Client에서 수행한다(에이전트는
빌드·로그·수치 진단까지).

## 11. 범위 밖 (이 문서에서 다루지 않음)

- 불꽃놀이·파티클 54종 재현: Effect 담당의 unified Effect 저작 계약으로 별도 진행.
- motionOrInterp 377 전체 복원: 이 문서는 화면에 보이는 대관람차·회전목마만 다룬다.
- 서커스 광장의 Server 상호작용(문 개방, 인카운터 트리거): 별도 수직 슬라이스.
- World Sequence의 제품 런타임 재생(현재는 툴 미리보기 전용): 별도 수직 슬라이스.
- decal 33, fog 1, water 7: 대응 런타임 계약이 없어 admission 보류 상태 유지.