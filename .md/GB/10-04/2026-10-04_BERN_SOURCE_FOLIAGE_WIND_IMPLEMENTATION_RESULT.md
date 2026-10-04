# Bern source foliage wind 구현 결과

Bern 원본 foliage wind 복원 결과

원본 127 MIC의 ShaderMap/static key/부모 chain/numeric override를 조사한 뒤, 시간 바람·noise amplitude 중 하나 이상이 살아 있는 120 MIC / 5,901 material 행만 복원했다. wind/noise amplitude가 둘 다 0인 7 MIC / 1,427행에는 시간 바람을 추가하지 않았다. 원본 static wind switch true라는 사실과 화면상 실제 움직임을 구분한다.

설치된 프로그램은 E4FE 4,789행, grass 1C39 1,047행, 별도 basic098C 65행이다. 별도098C를 비슷한 A1C6에 alias하지 않고 원본 DXBC identity/instruction SHA로 별도 admission 했다. 원본 arithmetic은 LocalVF 3개를 literal HLSL로 옮겼고, 대응 InstancedVF 3개도 직접 원본 DXBC GPU 실행으로 대조했다. 기존 foliage surface뿐 아니라 source.character native1100..1166 map surface도 이 wind를 소비한다.

원본 mesh 144개의 native FBoxSphereBounds 7float와 physical/serial SHA를 새로 해독했다. 원본 static primitive 6,624개 및 instanced foliage primitive 1,697개, 총 8,321개를 native actor/component/archetype/CDO까지 읽어 현재 배치 24,275개에 source owner 위치와 primitive ObjectDimension/radius를 별도로 공급한다. static 6,624배치와 foliage 17,651배치다. 기존 mixed/shared owner 161 material 행을 포함하고 historical census에 없던 EVENT01 static188/foliage6 join도 실제 해당 물리 원본에서 해독하여 오류0으로 닫았다. 24,275개를 '서로 다른 원본 actor 수'라고 부르지 않는다.

Native EFEngine.dll SHA 6d107bf77f7dee8111882bcd3309f05dd2217d4bf8c7aa730967af04ce6d2c47의 archived loaded-memory와 PE/RVA를 읽었다. 원작 실행은 하지 않았다. CreateSceneProxy0x625960은 owner LocalToWorld X축을 정규화한다. 방향 proxy0x657974는 XYZ=normalizedDir*Strength / W=Speed를 전달하고 FScene0xaba270(vtable0x128)은 source count 평균으로 소비한다. Bern PS actor1555 rotation(-10856,84404,-103716), component1556 Strength2, Engine.u WindDirectionalSourceComponent CDO30941 Speed1을 연결했다. 현재 source wind float4는 [-0.23851251602172852, 0.9825141429901123, -1.7256238460540771, 1.0]다. 원본이 없는 경우에만 쓰는 [0,0,1,0] fallback으로 현재 Bern을 대체하지 않는다.

Native SetMesh0x9db950은 source ActorWorldPos와 primitive bounds를 읽는다. ObjectDimension consumer0x9dbf71, FBox TransformBy0x2ef010, static UpdateBounds0x9d1aa0 및 instanced UpdateBounds0x505d50/union0x1c6420을 추적했다. UE3 row-vector matrix의 columnSq 최대값 sqrt로 sphere를 스케일한다. static/instanced 모두 extentXYZ와 radius에 +1cm를 적용한다(0x9d1cb5, constant0x13f6dd0=[1,1,1,1]). Static은 그 뒤 original BoundsScale을 곱한다. 현재 대상 primitive의 full source chain BoundsScale은 모두1이다. Instanced는 해당 원본 component의 모든 native instance matrix로 각 bounds를 변환하고 원본 순서대로 union한 aggregate bounds를 쓴다. 설치된 모델 삼각형의 파생 AABB를 native bounds 근거로 쓰지 않았다.

Per-placement carrier는 material 공유 때 원본 actor가 달라지는 문제를 해결한다. 엄격 placementWind sidecar에 sourcePlacementId/assetId/actorPositionSourceCm[3]/objectDimensionsAndRadiusSourceCm[4]를 저장한다. Source owner W=1 유효 표식을 standalone draw와 instanced vertex input 모두 전달한다. Instanced stream은 기존192에서224byte, input19에서21개로 늘고 기존 offset은 유지한다. standalone shadow에도 동일 source owner/bounds를 넘긴다. material time0 scalar lanes와 native time index를 admission하며 runtime elapsed seconds가 해당 원본 lane을 대체한다. grass LocalPlayerWorldPos는 현재 scene character root를 [x,-z,y]*100cm로 변환하고, 없으면 original MIC99999 sentinel을 유지한다.

수치 검증: headless D3D11 WARP에서 설치된 92 physical WModel의 실제 carrier vertex/color를 사용하고, source MIC120 각각에서 12~14 vertex와 time0/.25/3/7을 시험했다. 원본 native DXBC VS가 반환한 SV_Position을 GS stream output으로 읽어 복원 HLSL과 직접 비교했다. Local3 + Instanced3, 실제 source carrier11,584 + controlled mirror/nonuniform adapter96, 전체 11,680cases 유한값/PASS. controlled adapter96은 원본 game draw라고 부르지 않는다. 최대 좌표차 0.01123046875cm (0.0001123046875m). tolerance0.1cm이며 bitexact라는 선언이 아니다. 실제 원본 DXBC의 immediate 상수는 완전 정밀도이고 HLSL disassembly literal은6decimal이다. 이 fixture는 한 source shader가 실제 game draw에서 선택되었다는 증거와 모든 vertex의 일치를 뜻하지 않는다.

저작/게시 mapmaterials 최신 양쪽 SHA: 4116cc4b44a0a1168b72f0d6fc37c81b5708c8f0eb34b325c080fd8864ada5c5
5,901 foliageWind 행과 24,275 placementWind 행만 freshmerge했다. 이전42 landscape lighting/static shadow, 모든 non-wind material/lighting/draw policy를 제거 역대조로 그대로 보존했다. 교체 전 freshness 재확인/백업/atomic replace를 수행하고, publisher LF 규약 정정은 JSON field 변화0으로 별도 기록했다. Bloom/Fog/FXAA 등 사용자 렌더링 옵션은 변경하지 않았다. 최초7 관련 TU focused syntax PASS는 root out/F1WorldMeshInspection/wind-syntax.json에 있다. 원작/Client/UI/자동 screenshot을 실행하지 않았다.

게시 검사: Area Validate는 50,021placements/24shards/53files PASS. wind mapmaterials 저작/게시 byte equality PASS. 전체 Area Check는 기존 mapeffects CRLF byte 규약차이 때문에 FAIL이다. 해당 effects 저작/게시 파일은 SHA11dd93cda40f38d64537c9de04833b86995bd53fd5592e1a8cff2eba5b3cc77d로 정확히 같으며 JSONfield91presentations 동일, mtime9월19일 및 초기감사 snapshot SHA와 동일하다. Publisher4089는 항상LF expected byte를 만들어 runtime CRLF와 비교하므로 실제 effects내용변화 없이 실패한다. root 지시에 따라 effects 파일은 수정하지 않았다. 그러므로 전체Area Check PASS라고 요약하지 않는다. 최종 Product Debug/Release build 결과는 root 합본에 별도로 기록한다.

남는 경계:
- native SetMesh의 meshbatch flag0x8000 + userdata radius override가 실제 이 원본 draw들에서 활성인지 아직 판정하지 못했다. StaticMesh BodySetup collision bounds union이 원본 visual bounds보다 큰지 전수 판정하지 않았다. source 일반primitive bounds를 복원했으며 이 두 branch까지 닫혔다고 주장하지 않는다.
- 원본 rotator/CPU SSE normalization은 native 소비자 산술/단위를 추적했지만 수치 구현은 double trig 후f32다. native trig table/SSE rsqrt bitexact 재생은 아니다.
- shader replay에 공급한 source uniform/owner/bounds는 실제 source 문서에서 생성했지만 모든 source vertex/topology/tangent/color 일치를 검증한 것은 아니다. current COLOR0 mask가 없을 때 installed carrier white default를 사용한다. 원본 visible frame 동일성 검증은 하지 않았다.
- 원본 restpose의 owner/bounds sidecar를 individual World Scene Tool placement 편집에서 원본 Actor 이동으로 자동 오인하지 않는다. source Actor 전체 이동, instance 하나 이동, aggregate bounds 재계산은 구분해야 한다. 현재 임의 tool edit 뒤 source phase/bounds 재해석은 별도 범위다.
- CPU triangle picker는 shader wind로 변형된 vertex를 재생하지 않으므로 흔들리는 잎/잔디 가장자리의 화면상 삼각형과 picking triangle이 완전히 동일하다는 판정을 하지 않는다.
- 시간 amplitude0인1,427행의 interactive character bending은 별개이며 이 작업에서 시간 wind를 켜지 않았다.

세부 receipt: outputs/bern-wind-restoration.json
원본 DXBC replay: outputs/bern-wind-native-shader-replay.json
원본127 MIC 전수 조사시점 기록: outputs/bern-foliage-source-wind-census.json
