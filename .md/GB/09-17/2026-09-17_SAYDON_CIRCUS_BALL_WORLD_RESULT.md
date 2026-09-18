# 세이튼 서커스 공 World / V1 통합 결과

## G01. 구현과 원작 경계

원작 Action 4219806, Missile 421980602~421980607의 end-child callback을 읽어 5회
분열, 6세대 1→2→4→8→16→32, 총 63공을 생성했다. 사용자가 정정한 5회를 반영했다.
각 세대 source scale 2/1.7/1.4/1.1/.8/.5, range 2.5/2.75/3/3.25/3.5/4m와
maxLife 1.5초를 사용한다. 원작 random360 대신 부모 +90° 축에서 ±45°는
USER_AUTHORED다. 첫 bounce 구간을 선택하여 1.5초에 재배치한 이동과 source range를
1.5초에 걸쳐 이동하는 속도는 PROJECT_AUTHORED다. 원작 1500cm/s를 정확 재현했다고
기록하지 않는다.

`Tools/EffectPipeline/build_saydon_circus_world.py`는 기존 Mario StripedBall의 CModel/
CMaterial 정의를 재사용한다. native circus-ball 모델과 290 정점 및 축별 extent가
일치함을 확인했다. 실제 모델 pivot을 상쇄하여 ground contact와 FX 지면 원점을
일치시켰다. 새 damage/collider/Server projectile simulation은 추가하지 않았다.

기존 World model-less group에 6 STOP motion을 넣고 per-emission delay로 세대를
예약한다. NEXT의 count=1 제한은 유지했다. 부모 종점이 두 자식 원점이고 각 공은
한 bounce 후 사라진다. 각 그룹의 전체 재생은 11.5초이며 마지막 충격 2.5초를 포함한다.

## G02. 등록 후보

정본 후보 위치는 `out/SaydonCircusWorld20260917/candidates/`다.

| 종류 | ID / 이름 |
|---|---|
| V1 | `effect.kouku.gate1.circus.rainbow.drop` — 세이튼 / 쓰리투원투하 / 공 낙하·상단 무지개·도넛 폭발 |
| V1 | `effect.kouku.gate1.circus.rainbow.impact` — 무지개 도넛 폭발 |
| V1 | `effect.kouku.gate1.circus.ball.upper` — 공 상단 무지개 |
| V1 | `effect.kouku.gate1.circus.gun.muzzle` — 세이튼 / 서커스 공 발사 / 쇼타임 총구 발사 |
| World group | `world.object.kouku.saydon.circus.split` — 1회 튕김·5회 분열 |
| World group | `world.object.kouku.saydon.circus.shot` — 발사·5회 분열 |

World group 둘과 실제 model 정의 둘, template 12개, instance 12개를 생성했다.
`WorldSequence.entries.json`, `EffectCatalog.entries.json`, `EffectResourceTree.entries.json`,
`Composition.worlds.proposal.json`과 상위 `pending-registration.json`이 semantic merge
입력이다. Composition 정의는 objectResourceId와 sequenceInstanceId에 같은 group ID를
저장한다. P83의 비어 있는 stage/timeline은 변경하지 않았다.

Projectile 421980613이 실제 참조하는 VividFracture_Ball_04와 Exp_02/03/04의
native recipe/material을 보존했다. 누락된 nested CDO ScaleFactor/StartSize만 복구한다.
World 상단광은 원작 sprite30/35 위치에서 원작 공 mesh5 위치를 빼서 같은 공의
transform을 한 번만 적용한다. V1 낙하 통합은 3.5초, 충격은 2.5초, 상단광은 1.8초,
Showtime 총구는 11초다. source 시스템의 원래 geometry와 색을 새 texture로 대체하지 않았다.

발사 World는 원작 Showtime muzzle FX를 공과 함께 재생한다. 기존 WORLD 총 좌/우
`world.object.kouku.saydon_showtime_gun_left/right`는 BOSS 손 부착 계약을 유지하는
별도 배치 자원이다. WORLD-only 시각 그룹에 임의의 G1 bone/pose로 복제하지 않았다.

## G03. 실행한 검증

- 실제 CWorldSequenceDocument Load → Save → Load semantic equality 통과.
- production Sample_Track / Sample_ObjectWorld / Resolve_ObjectMotion /
  Get_InstanceElapsedSpanMs 본문을 추출하여 CPU에서 실행했다. 두 그룹 각각 63공,
  부모 종점/자식 원점, +45/+135 방향, 모델 pivot, 지면 FX, 세대 scale, 2배속,
  seek/STOP/tail을 확인했다.
- 실제 Level 그룹 helper 본문을 failure-injection 경계와 함께 실행했다. member prepare
  실패 시 play 이전 중단, 중간 play 실패 시 전체 rollback, 이미 재생한 member의
  중복 생성 방지, 한 번 성공한 birth matrix 공유, pending retry를 확인했다.
- 최종 `world_probe.log`: **18,368 checks PASS**. production 함수 source hash는
  `probe.source-hashes.json`에 보존했다. GPU/window/Client 실행은 하지 않았다.
- 최신 Effect ABI로 V1 4개 Load/Drawable/Serialize-Parse/Stage/15초 Update 통과.
  모두 particles=0, finished=1. 로그 `out/EffectV1DovePizza20260917/cpu-probe.log`.
- 최신 `Level_KakulSaydonArena.cpp` isolated Debug compile exit 0:
  `out/SaydonCircusWorld20260917/level_compile.log`.
- publisher 3개 focused tests(14종 invalid 변형 포함)와 실제 두 group의 단일 occurrence
  projection 통과. `publisher-group-probe.json`은 6 motions, 63 emissions, 11500ms를 기록한다.
  실제 Server focused probe는 기존 151 + group bootstrap/Logic/once 3 = 154 checks, 0 failures.
- `git diff --check` 통과. source encoding UTF-8 BOM/CRLF 보존.

## G04. 단일 그룹 소유권

Level은 group ID를 기존 CWorldSequencePlayer의 여러 member로 확장한다. resource
prepare와 placement 검증을 전부 마친 뒤 한 cue의 player로 commit한다. Debug preview와
Server 승인 재생이 동일한 그룹 helper를 사용하고 seek/Stop은 같은 clock/owner를 공유한다.
최초 성공한 birth matrix를 모든 member가 공유한다. pending anchor는 값을 확정하지 않는다.
복수 visible member의 모호한 단일 effect pivot은 실패하고, combatBody/collider/walkable을
가진 그룹은 Client/publisher에서 거절한다. Server packet 형식과 판정 경로는 추가하지 않았다.

MainApp 목록/Workbench Append-duration은 root, publisher와 Server focused 검증은
server_patterns가 구현·확인했다. Level 변경은 read-only 독립 검토에서 P0/P1 발견 없음.

## 남은 경계

Live Data/설치 DataFiles 자동 덮어쓰기, 제품 빌드, Client/UI 실행과 화면 판정은 하지 않았다.
현재는 후보와 소스 연결 완료 단계다. 미저장 사용자 draft 보존 후 root의 semantic 등록과
publisher 실행, 사용자 제품 빌드·실제 화면 확인이 필요하다. 시각적 원작 일치와 실제
아레나 GPU rendering을 CPU 검사로 PASS 처리하지 않았다.

## G05. 최종 병합과 World 모션 수용량

최종 병합 전 원본은 revision 2034, object 452 / template 256 / instance 312였다.
공 후보의 object 4 / template 12 / instance 12와 revision 변경만 추가한 2035 문서에서
기존 모든 배열 행과 다른 root 값이 유지됨을 확인했다. 최초 실제 codec은 template
256개 상한으로 268개 문서를 거절했으며, 이 실패를 숨기지 않고 수용량 확장 후 재검사했다.

C++ 저장은 std::vector와 JSON array이고 template ID는 문자열이다. 고정 256칸 배열이나
8-bit count는 없었다. 공용 MAX_TEMPLATE_COUNT, Map publisher, Composition validator,
컷신 후보 생성기의 제한을 512로 일치시켰다. 16 MiB, instance 2048, track 32, key 256과
모든 참조·자료형 검사는 변경하지 않았다. 과거 날짜 문서의 256 기록은 당시 결과이며 현재
정본은 AREA_DATA_LAYER_GUIDE의 512 계약이다.

새 header로 실제 WorldSequenceDocument.cpp와 격리 probe를 컴파일·링크하여 전체 병합
문서 Load / Validate / Save / 재Load / Is_Equivalent가 통과했다. 결과는 template 268,
object 456, instance 324다. 별도로 512개 Validate 허용, 513개 Validate와 Load 거절 및
실패 후 이전 문서 보존을 확인했다. 기존 probe의 실제 Map/Deploy context를 최신 원본에서
다시 읽고 Deploy의 설치 WModel animation catalog를 사용했다.

`WorldSequenceAnimationSourceStartContractTests.test_bounded_template_capacity_matches_map_and_client`
한 개의 집중 test에서 Composition validator와 실제 PowerShell Read-WorldSequenceDocument
함수의 268/512 허용 및 513 거절을 확인했다. isolated compile/link exit 0, focused test PASS,
git diff --check PASS다. 로그와 왕복 출력은 `out/EffectV1Final20260917/world-codec/`에 있다.
Live source/설치 문서를 이 probe로 쓰거나 Client/UI를 실행하지 않았다.

## G06. 실제 선택한 g0와 낙하 상단광 정렬

사용자가 저장한 Composition1232의 P83.world.7은 model-less group(world30)이 아니라 동일
표시명의 내부 donor(world32, split.model/default g0)를 참조했다. 따라서 기존 group 확장
소비자까지 도달하지 않아 한 공만 재생됐다. 여섯 motion과63공 정의는 모두 존재했다.
MainApp의 일시 World 목록에 고유한 owning group의 Append alias를 기록하고 donor 행은
Objects 목록에서 제외했다. 개별 motion은 Logic/편집에 남긴다. Workbench에서 이전 donor
선택으로 Append해도 동일 group 경로로 들어가며11500ms 전체 수명을 사용한다. 두 group이
같은 default를 소유하면 임의로 하나를 고르지 않는다. 기존 Level/Server 그룹 재생 경로는 유지한다.

현재 P83의 start0, position[1.9827895164,1.3078061342,727.0396728516], rotation/scale/anchor를
보존하고 world32→world30, duration7479→11500만 바꾼 guarded 후보를 작성했다. Pattern의
16045ms 수명과 모든 clip/다른 lane은 충분하므로 유지했다. Source/Server 데이터 projection은
한 WORLD cue의 stable group ID와11500ms를 소비하는 것을 실제 projector로 확인했다.

V1 rainbow.drop의 사용자 저장 상단광30은 offset[-.41,5.18,.66]과 독립0.1m/s 속도를 가졌고,
공 mesh5는0.8초 동안10→-.5m의 DirectLocation을 소비했다. native FX 모델은 중심0 pivot이고,
동일한 크기의 Mario World 모델은 중심Y.46933609m pivot이므로 두 geometry를 별도로 측정했다.
크기.7 native 공의 바닥이 지면에 닿도록 model root에+.72956332m를 적용했다. 상단광은 기존
EmitterDirectLoc으로 mesh5의 실제 live position을 받아 고정 top offset.32956332m만 더한다.
source 공급자를 먼저 평가하도록 두 element 순서를 정렬했다. 기존 material/색/size 및 사용자가
삭제한35는 그대로다. 독립 velocity/LocationDirect만 끄고 새로운 이동 runtime을 만들지 않았다.

도넛은 실제 fixed-step birth를 포함한0.816667초 착지 시각과 공 XZ에 맞췄다. 원작0.8초
particle lifetime을 바꾸지 않았으며 emitter delay 보정은 PROJECT_AUTHORED다. World upper는
1.5초 motion 전체에서 살아 있고 자체 위치/속도 없이 existing followObject provider만 따른다.
12개 upper track은 실제 Mario 모델 상단을 anchor로 사용하고 fitEffectToDuration=false로
자연 emitter tail에 의한 조기 수명 축소를 제거했다. upper resource metadata는2500ms이다.

실제 Effect Codec/Playback891검사에서 upper 최대 위치편차2.98e-7m, 공과 속도차2.86e-5m/s,
공 바닥 오차1.73e-6m, 착지·폭발 동시0.816667초를 확인했다. 실제 World codec과 production
sampling 본문18,494검사에서 두 group각63공, 부모/자식 원점, 실제 model top, 배속/seek/cleanup을
검증했다. MainApp actual collector 추출 검사는 unique alias, 목록 두 행, disabled member와
ambiguous group 보존을 통과했다. MainApp/Workbench/MainApp_WorldLevel 세 TU isolated compile도 통과했다.

전체 World268template/456object/324instance 후보의 실제 Load/Validate/Save/재Load 동등성,
Composition1233 후보의 실제 Codec3검사와 P83 publisher projection이 통과했다. World full 후보는
compact JSON8.03MB로 기존16MiB 경계를 유지한다. 정본 설치 입력은 전체 후보 덮어쓰기가 아니라
`out/SaydonCircusWorld20260917/follow/candidate/repair.patch.json`의 guarded record 교체다.
source1232/World2035 hash와 Effect2개별 hash를 포함하며 Composition operation2개, World template
operation12개다. 재생성은 `python Tools/EffectPipeline/build_saydon_circus_world.py --repair-follow`이다.
검증 manifest와 로그는 같은 follow 폴더의 verified.json/result.log/world_result.log/full_result.log에 있다.
live Data 설치·제품 빌드·Client/UI 실행·화면 판정은 이 작업에서 하지 않았다. root가 최종 등록을 소유한다.

## G07. 상단 무지개의 실제 quad 부착과 분열 공 반복 제거 후보

최신 저장본 rainbow.drop의 mesh5→sprite30 emitter-direct 연결과0.8초 입자 시계는 이미
같았다. 사용자 position[1.87,6.05999994,1.20000005]는 provider world→target inverse에서
상쇄되어 실제 중심 위치를 바꾸지 않는다. root가 허용한 보호된1회 actual Codec/Playback
검사891 checks/0 failures에서 공 상단 거리.329563m, 위치편차4.17e-7m, 속도차2.86e-5m/s,
같은0.816667초 착지/폭발을 확인했다. 중심 검사만으로 화면의 긴 무지개를 올바르게 붙였다고
판단하면 안 되는 경우였다.

sprite30은 원작 boffsetcenter=true/offsetcentery0, StartSizeY=-300cm, SizeLifeY.2→2를
사용한다. 현재 GeometryHelpers의 abs 크기+UV flip 경로에서0 pivot은 전체 quad가 anchor
아래로 확장되므로 길이가.6→6m로 커질 때 표시 중심이 추가로 아래로 이동한다. 이전 G06은
source velocity(+10cm/s, 위쪽)를 꺼서 PSA_Velocity의 표시축도 world-up 대신 zero-motion
camera-plane fallback이 되게 했다. source2843 PS와 native VS의 독립 읽기 검토에는 별도
시간 낙하나 WPO가 없었다. 원본 VS의 signed-size 입력과 UE CPU packing 전체를 확정한
것은 아니므로 공용 signed-pivot renderer를 바꾸지는 않았다.

최소 후보는 현재 sprite30 source velocity의 benabled를false→true로 복구하고, 파생 Effect의
상단 배치 저작값 offsetcentery를0→1로 변경한다. 원본 module·binary의 pivot 복원이라고
주장하지 않는다. 기존 emitter-direct가 매 tick 최종 위치를 계속 소유하므로 별도 이동을
더하지 않는다. 사용자 element position, 모든 source curve·크기·색·재질·시각과 삭제한35는
그대로 유지했다. 공·무지개 중심을98개 paired row에서 비교한 CSV는 수정 전후 정확히 같다.

후보 actual Codec/Drawable/roundtrip/Stage/Playback과 실제 Make_ParticleSpriteWorld는
root가 허용한 추가1회에서1,283 checks/0 failures, exit0을 기록했다. 35도 기울인 camera
조건에서도 표시 up축은 world+Y(오차0)이며 quad 하단은 공 상단+보존한 원작20cm camera
offset에 붙는다(최대 오차9.54e-7m). 중심·속도·착지·폭발 시각은 전후 동일하다. 실제 GPU
표시나 사용자 눈 판정은 하지 않았다. stderr의 static teardown CRT dump를 보관했으며
이 검사는 메모리 누수 검사가 아니다. 모든 검사에 Windows/CRT/Engine 오류 창 차단과
45초 timeout, 첫 실패 중단을 적용했고 자동 재실행은 하지 않았다.

별도 사용자 요청으로 World2036의 split.g1..g5 upper effectTrack 다섯 개만 제거한 후보를
만들었다. 상단 무지개 총 방출은63→1로 줄고, 자식62공에서는 제거하며 g0 첫 공의 upper는 남는다.
shot mode의6개 upper,63공과 모든 world model·motion key·emission·impact·first bounce,
전체11500ms 수명 및 나머지263개 template 내용은 그대로다. 전체 문서의 다른 행은 exact
비교로 보존을 확인했다. World 전체 실제 codec도 아래 최종 병합 검사에서 통과했다.

기존 build_saydon_circus_world.py에 같은 facing/명시적 pivot과 split child upper 제외를
반영했다. out 재생성에서도 split63공/upper1track, shot63공/upper6tracks를 확인했다.
이전에 검증한 손상 없는 라이브 데이터는 덮어쓰지 않았다. 입력 bytes/hash, 의미 Effect2값,
World5track-list guarded patch와 후보 및 로그는 out/CardDiceScale20260917/ball에 있으며
정리 파일은 verified.json, 설치 입력은 semantic-patch.json이다. 최신 Effect hash는
19baa96cb44c4c33f43d552fbe17b393fdb16da1f041d4c8d2b6dd674cb0d8c7이고 World baseline은2036이다.
실제 등록·publish는 root의 통합 단계가 담당한다.

G07 최종 병합 검증: out/CardDiceScale20260917/stage.py는 11파일/103개 의미 변경만
staged하고 readyForCommit=false를 유지한다. World revision2036→2037와 split.g1..g5의
upper 삭제 다섯 배열만 허용하는 exact expected/proposed 제한 및 오허용 거부9검사를 통과했다.
Composition1246의 P48/P78/P83은 실제 publisher의 저장 inventory/의존 closure/검증/투영
경로에서 모두 ready이고, 기존 P32 미완성은 같은 원인으로 격리된다. 실제 PowerShell
Read-WorldSequenceDocument의 전체 staged 문서 읽기와 Python의 6 motion group/placement
검사도 통과했다. 최초 추출 검사의 누락된 전역 재질 경로는 out 검사 설정만 보완했다.
승인된 전체 C++ codec1회는 exit0,268templates/456objects/324instances의 Load/Validate/
Save(out)/reload/Is_Equivalent를 통과했다. context 원본 mapplacements/deployassets/
deployplacements3개 SHA는 생성 당시와 동일하고, map scale3369행도 일치했다.
증거는 ball/world-validation/{verified.json,context-and-codec.json,full-codec-result.log,
publisher-result.log,selected-publication-readiness.json}이다. Data/제품 배포와 UI 실행은 없다.


### G07 실제 저장본 반영

사용자의 최종 교체 승인으로 공 상단 무지개와 분열 자식 upper 제외 후보를 실제 설치했다.
WorldSequence는2036→2037이며 상단 무지개 문서와 나머지 사용자 편집을 최신 기준으로
병합했다. Client 프로세스 실행 여부를 설치 선행 조건으로 두던 out gate는 제거했다.
명시적 승인·최신 hash·필드 충돌 검사와 백업·원자적 교체는 유지한다. 통합11파일 설치와
후속 publisher 증거의 정본은 같은 날짜 세이튼 카드 RESULT G28 및
`out/CardDiceScale20260917/installed-registration.json`이다. 이전 절의 후보/미설치 문구는
검증 당시 상태이며 현재 상태를 대체하지 않는다. 도구 Reload와 화면 확인은 사용자가 한다.
