# 괴기스러운 인형·Object/Sequence Effect·포탈·커튼 결과

## 현재 완료 경계

사용자가 저장 후 EXE 종료와 전체 반영·빌드를 요청했고 Client/Server 부재를 확인했다.
최신 저장본에 기반한 최초 13개 원본 변경과 후속 Boss 좌표 보정 1개를 hash 재확인과 백업 후 적용했다.
맵 전체 Publish, RenderingProfiles Publish, KAKULSAYDON_ARENA World Publish가 통과했다.
새 칼날 Travel/Parent 작업을 포함한 Engine·Shared·Server·Client Debug Product 빌드가 통과했다.
사용자 Client 화면 검증은 수행하지 않았다. 아래 이전 후보 준비/출력 점유 기록은 당시 상태이며,
최신 적용 내역은 이 문서 끝의 적용 항목과 `out/DollEffects20260913/activation-manifest.json`을 따른다.

## G01. 두 인형과 모델·불꽃

- 작은 인형은 기존 stable ID `world.object.kouku.odd_doll`과 배율 1을 유지한다.
  큰 인형은 `world.object.kouku.odd_doll.large`, 배율 1.6과 독립 Motion 30개를 가진다.
  적용한 표시명은 각각 `괴기스러운 인형 - 작은 사이즈`, `괴기스러운 인형 - 큰 사이즈`다.
- 큰 크기 근거는 원본 raid NPC 480641/480742의 ModelSize 160 대 480746/480747의 100이다.
  installed WModel의 modelPreScale 0.01은 유지한다. idle 정점 높이는 1.13006m / 1.80810m다.
- Object 목록은 기존 V2 GROUP/LEAF와 V1 Effect를 제공한다. `Play Effect`는 임시 문서를
  재생하고 `Append Effect`는 선택 Motion에 TIME 0ms, followObject=true로 추가한다.
  같은 Effect가 이미 붙은 Motion의 Play에서는 기존 행을 사용해 중복 생성하지 않는다.
- V1 Effect 준비가 끝난 뒤 모델·Effect의 같은 시계가 출발한다. Play/Pause/Seek/Stop을
  기존 WorldSequencePlayer가 소비한다. 실패하면 저장 문서와 기존 preview를 보존한다.
- `V1_EFFECT`, `followObject`, optional `bone`은 C++ codec와 publisher가 함께 검증한다.
  matching ModelCue는 실제 Object CModel/pose를 사용하며 내장 인형을 중복 렌더하지 않는다.
- 두 기본 불 뿜기 Motion에는 `effect.kouku.gate3.doll.flame.full.restore`를 0ms부터
  11334ms 동안 연결한 데이터를 적용하고 맵 runtime으로 게시했다. 원본 첫 불꽃은 약 1.55초다.
  작은 원본의 한 입과 큰 원본의 두 입 연출 차이를 소스 복원 완료로 동일시하지 않는다.

## G02. Sequence Effect

Effect를 선택된 Animation/World 행의 시작, Sequence 시작 또는 cursor에 연결한다.
Play와 Append는 같은 배치 함수를 사용한다. Play는 모델과 Effect를 임시 snapshot으로
같이 재생하며 Append는 검증 후 draft에 반영한다. WORLD Effect는 실제 world occurrence
ID와 같은 앵커를 저장하며 기존 Boss companion 형식도 계속 검증한다. V2 LEAF 목록도 표시한다.

## G03. 포탈 World 위치와 캡처 수축

독립 Effect의 Play All / Solo / Append가 같은 World position과 yaw를 소비한다.
`Use Player Pos`는 현재 플레이어 위치·방향을 고정 복사하며 `Use Mouse Pos`는 UI 밖 맵의
한 번 피킹을 받는다. 실패·취소는 기존 값을 유지하고 피킹 클릭은 gameplay 입력에 전달하지 않는다.
Player-at-play 모드도 유지하며 모드 변경 직후 이전 World root가 남는 경계를 수정했다.

수축은 캡처 RT의 해상도를 변경하지 않는다. native screen-post shader가 검은 바깥을
유지하고 캡처 SceneHDR와 Bloom의 표시 영역·중심을 줄인다. 목표 UV는 actual source
transform을 화면에 투영한다. 이 scene replacement는 NonLight 뒤, Blend 앞에서 합성하고
기존 MRT/depth를 복원한다. 이후 포탈과 파티클이 그려지므로 불투명 검정이 포탈을 덮지 않는다.
일반 ScreenPost의 순서와 ping-pong 카운트에서는 scene replacement를 제외한다.

centered portal의 native 30개 emitter를 보존하고 수축 1개를 더한 새 Effect를 적용했다.
수축 구간은 실제 흡입 emitter의 약 2.009663~6.139774초다. Sequence resource
`kakulsaydon.g1.presentation.54`만 이 Effect를 참조하도록 바꾸었으며 사용자 pattern의 시각·위치 행은
변경하지 않았다. EffectCatalog, ResourceTree와 프로젝트 None 등록도 적용했다.
Sequence 원본 revision은 25에서 26으로 변경됐다. 실제 화면 판정은 사용자에게 남는다.

검은 바깥 화면 유지는 1관문 도입 포탈 수축에만 해당한다. 차원술사 Alt+V에는 이 검은 배경
처리를 추가하거나 연결하지 않았다. Alt+V의 full/tuning Effect 데이터는 해당 검증 시점과 hash가
같으며, 수정 범위는 기존 캡처 화면을 실제 큐브 위치·크기·애니메이션에 맞추는 코드다.
Alt+V의 기존 배경과 연출은 유지한다. 검은 배경 추가는 Alt+V의 미완료 작업에 포함하지 않는다.

## G04. 커튼 원리와 보이지 않은 원인

`world.object.kouku.curtain`은 모델을 소유하지 않는 alias다. `curtain_drop`이 맵 placement
11개의 Y offset을 +18→0으로 1050ms 동안 이동하고 2450ms까지 유지한 뒤 3500ms에 +18로 숨긴다.
화면을 덮는 천은 별도 V2 ScreenPost `boss.kouku.curtain_1`이며 normalized screen UV에서
Y -0.54→0.52→-0.54, enter 30% / exit 70%, depth test 없이 합성된다.

현재 화면 커튼은 Presentation Manager의 ScreenOverlay와 Deferred shader를 이용한다.
UI처럼 화면 좌표로 렌더링하므로 별도 직교 투영 CUIObject를 추가할 필요는 없다.
World alias만 Preview하면 화면 companion을 호출하지 않았고, 해당 맵 placement가 현재
scope에 없으면 월드 재생도 실패했다. Sequence의 alias Append 역시 기존 companion을 잃을 수 있었다.

소스는 저장된 Boss/Sequence의 같은 alias association에서 명시적인 screen companion을
찾아 기존 V2 Play/외부 clock Sample/Stop을 함께 호출한다. 단일 MAP alias의 placement가
scope 밖이면 명시된 screen companion만 Preview하고 상태에 이를 표시한다.
미등록 리소스를 커튼 ID 하드코딩이나 임의 UI로 대체하지 않는다.

이전 P37 저장본을 대상으로 화면 companion 후보를 준비했으나, 사용자가 마지막에 저장한
Boss revision 511에서는 해당 월드 커튼 행 자체가 없어졌다. 따라서 P37 후보와 Boss 문서는
적용 대상에서 완전히 제외했다. 최신 P37을 보존했고 커튼 월드 행이나 화면 행을 다시 넣지 않았다.
기존 `V2_커튼_전체화면` 리소스와 다른 저장된 연결도 제거하지 않았다. 독립 Sequence에 임의의
커튼 occurrence를 추가하지 않으며, 이후 사용자의 Append는 기존 명시 companion을 보존한다.

## 개별 검증 증거와 이전 빌드 대기 기록

- 변경 WorldObjectTool, MainApp, Kouku Workbench/Composition/PresentationPlayer,
  Level WorldObjects, Effect World placement 4개 TU, Effect runtime 관련 TU,
  Engine Renderer, NativeScreenPostMaterial, DocumentRenderer Rendering의 격리 MSVC 컴파일 통과.
- 원본 설치 인형 크기·골격·애니메이션 CPU 측정 완료. canonical VS2022 14.44와 격리 Engine을
  사용해 실제 `MN_CDMD_00.wmodel`의 입 본 2개·0~11.333초 1334 samples를 비교했다.
  source ModelCue preScale 1 × 0.01과 Object preScale 0.01의 최대 행렬 오차 2.38419e-07,
  large 1.6 비교 오차 0, live palette 보존과 missing bone 거부 통과.
  DLL 실제 로드 경로도 검증했다. 증거 `out/WorldObjectEffect20260913/anchor.isolated.run.log`.
  격리 Engine은 최신 Model.obj와 기존 Engine object를 연결한 수치 검증용이며 Product 빌드가 아니다.
- WorldSequence native codec 저장/roundtrip/실패 보존 9건, publisher 새 계약 7건 통과.
  기존 Python 계약 31건 중 30건 통과, 1건은 무관한 MapTool inline weak_ptr.lock 표현식의
  정규식 기대 불일치다. Sequence timing helper 7건과 신규 projector 테스트 통과.
- World placement 실제 함수 CPU 63건 통과.
- 전체 WorldSequence 후보는 현재 publisher 함수의 구조·재질과 MAP/DEPLOY 배치 join 검증 통과.
  Sequence 후보는 최신 C++ Reload/Validate/canonical roundtrip 및 7개 Pattern expansion 통과.
  Boss P37 최소 커튼 후보도 같은 C++ 로드·저장·P37 확장을 통과했다. root 후보는 native로
  검증한 후보와 JSON 값이 같으며 기존 파일의 줄 형식까지 보존한다.
  증거 `out/SequenceEffects20260913/activation-validation.json`.
- 실제 `PS_SCENE_COLLAPSE`를 WARP에서 중심/비중심 목표 × progress 0/0.5/1로 실행한
  6건 통과. HDR/Bloom RGB, 검은 바깥, 축소 면적과 중심 이동 최대 오차 0, D3D11 오류 0.
  `fxc fx_5_0` 통과. 로그 `out/DollEffects20260913/collapse-warp-result.log`.
- 정상 Debug Product 빌드는 실행 중 출력 점유로 차단. receipt:
  `out/BuildPipeline/runs/20260913T122520989Z-debug-product.json`.
- 후보 JSON/XML parse와 `git diff --check` 통과. 원본 freshness 재확인→후보 적용→
  WorldSequences scoped publish/check→정상 증분 Product build가 남았다.

이후 사용자가 저장·종료를 완료했다고 알려 아래 원본·런타임 적용을 진행했다.
새 실행 파일 적용 뒤 F1 → Action Workbench → Object에서 두 인형의 Play Effect/Append와
커튼 Preview Default/Play/Pause/Seek/Stop을 확인하고 Sequence의 같은 항목을 확인한다.
Effect Resources의 World anchor에서 Use Mouse Pos/Use Player Pos 후 Play All/Append를 확인한다.
포탈 Sequence와 DimensionMaster Alt+V의 최종 화면 판정은 사용자에게 남긴다.

## Composition Resources Leaf 노출과 전체 빌드 요청

사용자가 Composition Resources의 커튼 Leaf 표시 수정 반영과 전체 빌드를 요청했다.
현재 소스는 MainApp의 V2 inventory에서 LEAF/GROUP을 전달하고 Workbench의 V2 필터가
두 종류를 모두 표시한다. 검색·선택에서 기존 Play_Leaf와 Append 저장까지 연결됨을 대조했다.
상단 원본 목록은 `boss.kouku.curtain_1` 또는 `curtain`으로 검색하며, 하단 Created Resources의
기존 표시명은 `V2_커튼_전체화면`이다. 이 요청에서 원본 Effect·Composition 데이터나 후보를
추가 적용하지 않았다. Boss revision 510과 Sequence revision 25의 기존 커튼 LEAF 참조를
JSON parse로 확인했고 관련 소스의 `git diff --check`가 통과했다.

정상 Debug Product 명령을 실행했지만 Client PID 3000 / Server PID 29804의 출력 점유로
컴파일 전 차단됐다. 증거는 `out/BuildPipeline/runs/20260913T132610914Z-debug-product.json`이다.
사용자가 아직 편집 중이라고 답했으므로 프로세스 종료·제품 출력 교체 없이 유지했다.
저장·종료 확인 뒤 Engine → Shared → Server → Client의 정상 Product Build를 수행해야 한다.
이 시점에는 새 실행 파일 반영이나 전체 빌드 성공으로 기록하지 않는다.

## 최신 원본·런타임 적용

사용자의 EXE 종료 통보 후 Client/Server 부재를 확인하고 최신 저장본 기준 13개 파일을
hash 재확인, 백업과 교체 검증을 거쳐 적용했다. 기록은
`out/DollEffects20260913/activation-manifest.json`이며 과거 P37 후보는 포함하지 않는다.

- WorldSequences revision 1738: 인형 두 크기와 불꽃 연결, `3관문_외곽불`을 적용했다.
  새 그룹의 D_CCW와 E_CW만 각각 1m 바깥으로 옮겼고 기존 D/E/F 원본 Motion은 유지했다.
  14460개 좌표 비교에서 반경 변화 오차는 3e-8m 미만이다.
- 1관문 기본 진입은 펼쳐진 책 아레나와 열린 책의 마지막 자세를 사용한다.
  책을 펼치는 Sequence가 이를 잠시 빌려 재생하고 종료/Stop 시 기본 상태를 복구한다.
  보스 placement Z만 942.330017에서 737.530017로 옮겼으며 기존 navigation은 유지했다.
- 1관문은 책 아레나의 기존 조명과 원본 책 재질, 공통 렌더링 품질을 사용한다.
  3관문은 어두운 전용 profile과 청색 스포트라이트를 사용한다. 카메라 이동 중 frustum 정밀도에
  따른 잘못된 light culling 수정도 새 Engine.dll과 Client 제품 빌드에 반영됐다.
- 커튼 8개 placement는 원본 SOURCE_BG 재질의 별도 alias 2개로 연결했다.
  기존 원본 WModel과 texture를 재사용한다.
- `Publish-MapAuthoring -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish` 및 `-Mode Check`,
  `Publish-RenderingProfiles -Mode Publish`,
  `Publish-WorldGameplay -WorldId KAKULSAYDON_ARENA -Mode Publish`가 통과했다.
  맵은 3368 placement, 8개 runtime 파일을 게시했다.

새 칼날 Travel/Parent 확장과 최종 Product 빌드 증거는 별도
`2026-09-13_WORLD_OBJECT_TRAVEL_PARENT_LOOP_IMPLEMENTATION_RESULT.md`에서 이어 기록한다.
Client와 아레나를 실행하거나 화면 결과를 대신 판정하지 않았다.

## 빌드 뒤 발견한 1관문 Gaze 절대좌표

새 Server의 광역 계약 검사에서 Gaze 분신 소환 실패를 확인했다. 1관문 Saydon spawn은
열린 책 아레나 Z 737.530017로 이동했지만 저장된 `kakulsaydon.g1.logic.9`의
REAL_GAZE_TELEPORT Z는 이전 아레나의 934.67770021로 남아 있다. Server는 보스 spawn을
중심으로 이 target까지 반경을 구하므로 약 197m의 clone ring을 만들어 navigation을 벗어난다.
같은 상대 연출을 유지하는 최소 수정은 teleport Z를 -204.8 옮겨 729.87770021로 맞추는 것이다.

사용자가 Client/Server를 다시 실행한 뒤 발견하여 공유 원본과 runtime은 추가 수정하지 않았다.
격리 후보와 실제 Server navigation 수치 검증을 완료한 뒤, 사용자 Client/Server가 종료된 상태를
확인해 적용했다. Boss revision은 512이며 P37을 포함한 모든 Pattern 행은 그대로 보존했다.

추가 대조에서 P6만 참조하는 기존 `kakulsaydon.g1.world.2` 커튼의 NONE anchor offset도
Z 204.799로 남아 있었다. source MAP 11개 placement는 펼친 책 아레나에 이미 있으므로
이 offset만 -0.001로 교정해야 한다. 기존 WORLD/Effect/Pattern 행의 추가·삭제는 없다.
BOSS_SPAWN으로 새 보스를 따르는 룰렛에는 같은 보정을 중복 적용하지 않는다.

검토 후보는 revision 511→512, Gaze Z, 커튼 offset Z의 세 값만 변경한다.
`out/KoukuGate1GazeMigration20260913/candidate.diff`, `receipt.json`, `preapply.ps1`을 준비했고
기존 Gaze/Clone/P6 projector, source freshness, 커튼 11개 baseline join·상대 위치 보존이 통과했다.
실제 ServerNavigation으로 반경 7.681984m의 순간이동과 4/7/10시 분신 네 지점 모두 정확한
walkable 판정과 중심 경로 연결이 통과했다. 증거는
`out/KoukuGateNavigation20260913/gaze_navigation.receipt.json`이다.
source 적용 후 정상 Composition publish와 GameplayBalance publish가 통과했다.
적용 기록은 `out/KoukuGate1GazeMigration20260913/preapply-backup/20260913T1421108498259Z/apply.receipt.json`이다.

## Effect 목록 표시 추가 요청

사용자 예시 `Skill | Input Q ...`는 All Effects 창의 캐릭터 목록이다.
같은 창의 KoukuSaydon 목록은 `Render_SavedAuthoredEffectSection`이 표시하므로 이 경로의
중간 분류 트리를 한 줄 목록으로 바꾸는 소스를 반영했다. `1관문 | 연출 | ... | 이름`,
`1관문 | 패턴 | ... | 이름` 순서이며 관문 1→2→3→공통으로 정렬한다.
별도 Composition Resources renderer와 저장된 EffectResourceTree JSON은 변경하지 않는다.
개별 Effect의 기존 열기·Play All·Append Group과 편집 보존은 유지한다.

실제 metadata 652개를 1관문 256 / 2관문 201 / 3관문 180 / 공통 15 순서로 표시한다.
예시는 `3관문 | 패턴 | 세이튼 / 쇼타임 / 기관총 | 기관총 생성`이다. 반복 접두사는 화면에서만
생략하며 원래 표시명·전체 경로·asset ID를 tooltip과 검색에 유지한다. 같은 이름 1쌍은
기존 PushID(assetId)로 구분한다. Open/Play/Append/pending load callback 본문은 변경 전과 같다.
증거: `out/KoukuEffectFlatList20260913/receipt.json`, `label_inventory.json`.

최초 목록 TU 검증과 제품 설정의 문자 집합이 달라 Product compile이 한 번 실패했다.
파일 인코딩과 프로젝트 옵션은 유지하고 새 한글 literal 20개만 같은 UTF-8 byte escape로
바꾼 뒤 실제 제품 PCH/include/CP949 옵션 검사와 최종 전체 Debug Product 빌드가 통과했다.
최종 빌드 증거는 `out/BuildPipeline/runs/20260913T142636354Z-debug-product.json`이다.
Engine·Shared·Server·Client 모두 PASS, 기존 경고는 있으며 compile/link 오류는 없다.
최종 14개 적용 문서 hash, P37/다른 Pattern 보존, Engine 배포 hash와 diff --check가 통과했다.

## 최종 서버 생성 검증의 범위

최종 Product의 Server object 80개와 Shared.lib를 그대로 사용한 격리 runner에서 기존
Run_KoukuProduct 그룹만 4.282초 동안 실행했다. source/생성물/제품 입력 hash는 전후 같았다.
수정된 Gaze는 실제 분신 3개를 commit했고 다음 tick 중복 생성 방지와 Pattern 종료 시 제거가
통과했다. 이전 navigation 초과에 따른 clone admission 실패 분기는 더 이상 발생하지 않았다.

그룹 전체는 7개 assertion이 실패했다. 6개는 앞서 확인한 기존 관문 재생·댄스·lifecycle 검사이며,
Gaze의 나머지 1개는 예전 X=-6.36/Z=937.92를 기대하는 fixture와 현재 저장 좌표의 불일치다.
이 결합 assertion 뒤에 있는 inward yaw/spawn owner 검사를 독립적으로 통과했다고 기록하지 않는다.
그룹/광역 전체 PASS가 아니라 실제 생성·중복 방지·제거와 별도 수치 검사 PASS를 구분한다.
증거: `out/KoukuGate1GazeMigration20260913/ProductGroup/gaze-product-analysis.receipt.json`.
추가 Client/UI 실행·화면 판정은 수행하지 않았으며 동적 맵 끝/아이언메이든 충돌 소멸은 여전히
이번 칼날 끝점·Lifetime 소멸과 별도의 미구현 범위다.

위 7건과 광역 WorldTriggers 8건은 후속 조사에서 오래된 검사 입력·호출 순서·기대값으로 확인해
두 test TU를 수정했다. 집중 두 그룹은 81 PASS / 0 FAIL이며 제품 런타임 변경은 없다.
사용자의 후속 지시에 따라 동적 접촉 소멸 추가는 보류하고 현재 빌드의 직접 검증을 우선한다.
원인별 내역과 이후 최종 빌드·전체 서버 검사 증거는 같은 날짜
`2026-09-13_WORLD_OBJECT_TRAVEL_PARENT_LOOP_IMPLEMENTATION_RESULT.md`의 G07을 따른다.

## G11. 포탈을 시퀀스 폭죽 박스 위치로 이동

사용자가 포탈의 기준점을 `시퀀스 폭죽 박스의 저장 위치`로 확정했다. Sequence의 P4 `.presentation.30`에 저장된 폭죽 MAP 위치 `[59.75699996948242, 0.49000000953674316, -94.59600067138672]m`를 포탈 `.presentation.32`의 `positionOffset`에 반영했다. 포탈의 이전 위치는 `[72.857119140625, 1.7616273498535158, -99.769365234375]m`였으며 revision은 26→27이다. G03의 위치 보존은 당시 설치 범위이고, 이 후속 요청에서 위치를 변경했다.

변경 필드는 이 위치와 root revision뿐이다. 포탈의 start 0ms, duration 36541ms, native30+수축1 resource, 회전·크기와 모든 다른 occurrence/resource는 그대로다. 내부 centered source origin은 바꾸지 않아 MAP 배치가 한 번 적용된다. 최신 source bytes를 다시 확인한 뒤 교체했고 백업·필드 보존 증거는 `out/KoukuWarningPortalFollowup20260913/portal_placement_receipt.json`과 같은 폴더의 `KoukuSaydonSequenceComposition.before.json`이다.

`Box Detail → Use Mouse Position → UI 밖 맵 표면 클릭`은 현재 제품 코드에 있다. 성공하면 선택한 MAP Effect 박스 XYZ와 임시 미리보기 배치가 즉시 이동한다. 현재 cursor/paused 상태는 유지하므로 수명 밖에 있다면 박스 `Preview`로 시작부터 재생한다. 위치만 바꿨을 때는 `Save`가 staged placement를 검증·병합해 저장하며 `Apply`는 시간·수명·fade 등 나머지 Detail 변경을 적용할 때 사용한다. 빈 공간은 이전 위치를 유지하고 재클릭을 기다리며 Esc/우클릭과 선택 변경은 취소한다. Effect Resources의 별도 `Use Mouse Pos`는 다음 Play All/Append 원점 선택이므로 기존 Box 이동과 구분한다.

현재 코드의 `Complete_MapEffectPlacementRequest`, `Request_PresentationGeometryPreview`, `Save`, MainApp 표면 picking 호출과 기존 Product 반영 기록을 대조했다. 이 변경은 JSON뿐이므로 EXE/CSO를 재빌드하지 않았다. 기존 네이티브 Workbench probe로 변경된 Sequence를 읽어 Box Preview 및 MAP placement 67검사와 source 불변을 확인했다. 로그는 `out/KoukuWarningPortalFollowup20260913/portal_native_read.log`다. JSON parse·대상 외 보존은 통과했으며 실제 Client/UI 실행·피킹 조작·화면 판정은 수행하지 않았다.
