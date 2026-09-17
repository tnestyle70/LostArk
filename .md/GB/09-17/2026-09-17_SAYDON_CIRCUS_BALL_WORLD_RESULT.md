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
