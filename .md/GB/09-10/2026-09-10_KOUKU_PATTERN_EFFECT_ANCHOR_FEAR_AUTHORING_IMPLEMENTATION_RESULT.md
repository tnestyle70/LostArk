# Kouku Pattern 앵커·공포·저작 도구 구현 결과

## 현재 상태

사용자의 최종 Save와 Client/Server 종료 확인 후 원본 revision228을 보존하고 요청 항목을
revision229로 저장했다. KoukuSaydon 정본 domain owner로 Client Product와 Server Gameplay를
게시했고 Engine → Shared → Server → Client의 정규 Debug Product 빌드·배포를 완료했다.
새 Server의 실제 부착/공포/돌진/카운터 검사, bundle 검사와 게시 데이터 시작 검증이 통과했다.
Client 화면의 입력·재저장·아레나 재생 및 시각 판정은 사용자가 직접 확인할 단계다.

## 코드에 반영한 기능

- Pattern/Parent/Bundle/Logic/Resource Rename: 기존 이름 prefill → Apply name → 기존 Save.
- Effect V2 GROUP 기본 목록과 V1 Effect/Element lazy 목록, 넓어진 목록과 작은 Created scroll 영역.
- Collider와 Effect의 BODY/WEAPON bone·BOSS/WORLD anchor, Collider anchor 복사.
- V1 Element 선택 재생, 별도 ModelCue 렌더 제외, occurrence 소유 수명·실패 정리·기록된 transform 기반 seek.
- named bone/WORLD의 실제 사전 관측 기록을 V1/V2에 연결. 미기록 과거·teleport 구간은 격리한다.
- 명시 blendInMs를 Preview/Product body+weapon과 Server bone bake에 연결.
- 짧은 EXACT playMs 종료 자세 유지와 LOOP_TO_WINDOW Product 재생 지원.
- BossMotion 안의 Stage Delete/Duplicate 시간 splice와 Details Delete 경로 통합.
- FEAR의 Server 3초 이동/스킬 차단·종료 복원, protocol73 snapshot, 여섯 class 공포 animation.
- 로컬 FEAR Scene/캐릭터 Light/지연 Effect 재생과 종료·실패 복구. 같은 snapshot 종료 후 재시작 방지.
- COUNTER_WINDOW의 실제 counter hit 성공과 그로기 FOLLOWUP, 이전 패턴 종료.
- ENTER_AREA 시작 방향을 한 번 잡는 bossChargeDistanceM, 같은 시간 동안 이동 및 navigation/collision 검사.
- CAPTURE_PLAYER와 ATTACHMENT_HOLD, stable holdLogicOccurrenceId의 저작·복제·삭제·저장·게시.
- 실제 Server Capture 성공 뒤에만 Success 확정. 짧은 Collider 종료와 부착 종료를 분리하며
  중복 획득·기간 연장·FEAR 덮어쓰기를 차단한다. Hold 종료·Stop/Restart·사망·owner 변경 때 해제한다.
- Client CNpc의 실제 BODY bip001-l-hand socket과 기존 IPlayerHandGripSocketSource 연결.
  같은 snapshot의 owner pattern/revision으로 attachmentGrips를 선택하고 재로드 실패 시 기존 캐시를 보존한다.

## 원본과 제품 데이터 적용

최종 사용자 Save bytes는 `out/KoukuPattern20260910/final-user-save.json`과
`composition-before-228.json`에 보존했다. 쓰기 전에 원본 bytes가 같은지 재확인했다.
수정 대상 밖의 Pattern, 기존 Resource와 root 필드가 사용자 저장본과 같음을 확인했다.

| 대상 | 적용한 값 |
|---|---|
| P11 파1빨2 | 레이저 세 occurrence의 BODY bip001-head anchor, 기존 root 위치 offset 제거. 세 GAZE fail → FEAR3000, 암전 profile와 캐릭터 Light 연결 |
| P13 조커 찾기 | 망치 네 occurrence의 WEAPON b_rpct_01 anchor. STAGE5 → 13 → 6의 두 진입에 blendInMs=100 |
| P15 거미카운터 | 중복 STAGE27/32/38 제거. 반복당 돌진 clip 한 개, 총3개. 전체21501ms. 카운터 성공 → P22 그로기 |
| 거미 돌진 | 4500~5667, 11667~12834, 18834~20001ms. 각 Trigger 시작 때 player 방향 확정, 7m charge와 BODY-follow Collider 접촉 → FEAR3000 |
| 거미 얼굴/링 | 공포 시작1000ms 뒤 얼굴2초. 기존 V1 counter의 cyan-roar-ring Element를 세 counter window에 배치, 원본 V1 Effect 문서는 보존 |
| P17 대형 세이튼 잡기 | 부채꼴 판정2029~2817ms, Hold2029~7484ms. Success의 CAPTURE_PLAYER가 왼손에 부착하고 늦게 잡혀도7484ms에 해제. grip offset 각 축0 |
| 이름 | 실제 Large Saydon actor인 P20을 대형세이튼_레이저로 변경. Kouku actor인 P21 이름은 보존 |

새 V2 `boss.kouku.fear.face` GROUP과 `boss.kouku.fear.face_1` ScreenPost를 만들고 Data 프로젝트와
Independent 목록에 등록했다. 물리 입력은 기존
`Client/Bin/Resources/Effect/KoukuSaydon/Textures/FX_TEX_06/fx_g_rpcz_01.dds`다.
Resources binary를 생성·수정하거나 Git에 추가하지 않았다. 새 Drive 전달은 필요하지 않다.

정본 publisher 명령은 다음과 같다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 229
```

원본, Encounter, Client patternbindings와 Server bootstrap의 revision229 일치를 확인했다.
저장 Pattern24개 중20개, Bundle10개 중6개가 실행 가능하다. P3/P16/P19/P20은 사용자의 저장본에
Stage가 없으므로 목록과 unavailable 이유를 보존했다. 별도 대형세이튼_레이저 P20은 이름만
변경했으며 빈 animation을 임의 생성하지 않았다. 파1빨2 P11 안의 머리 레이저 연결과는 별개다.
상세 적용값은 `out/KoukuPattern20260910/final-source-and-products.json`에 기록했다.

## 실행한 자동 검증

로그 기준 경로는 `out/KoukuPattern20260910/`이다.

| 검증 | 결과와 증거 |
|---|---|
| 정본 KoukuSaydon + Gameplay 게시 | PASS, source229, Product20 patterns/6 bundles; publish-final.log |
| 최종 정규 Debug Product | Engine/Shared/Server/Client Build 및 shader/DLL 배포 성공, SkipBuild=False; product-final-retry.log |
| 새 정규 Server focused 검사 | 공포·counter·7m charge·bone contact·잡기 실제 Room 검사 failures0; server-grab-final-test.log |
| 새 정규 Server bundle 검사 | revision/actor admission·지연 시작·Stop/Restart·부분 시작 거부·사망 정리 failures0; server-final-bundle-test.log |
| 게시 데이터 Server 시작 | 별도 loopback17877, headless1000ms 정상 시작/종료 exit0; server-final-startup.log |
| NetworkProtocolHarness | 전체 failures0; FEAR 왕복·잘못된 조합·부분 payload 기존 상태 보존 포함; protocol-test.log |
| Grab projector | 신규3건 PASS: 원래 Hold deadline/grip, 잘못된 참조·outcome·offset, DRAFT 미완성/PRODUCT 충돌 거부 |
| FEAR/COUNTER/charge 및 기존 접촉 projector | 기존 focused6건과 Grab 이후 회귀3건 PASS, Python compile/PowerShell parse PASS |
| blend/짧은 window/loop projector | focused5건 PASS, 실제 P13 100ms 후 원래 pose 수렴과 bone contact bake 성공 |
| Client product root 계약 | 실제 producer 필드와 reader 허용 필드1건 PASS; fearPresentations/attachmentGrips 소비 연결 |
| 새 얼굴 Effect 구조 | 공식 Effect V2 validator leaf/group 구조·typed reference·DDS 물리경로 PASS |
| JSON/XML | 원본/Encounter/patternbindings/얼굴 leaf/group/Independent JSON6개, Client project/filter XML2개 parse PASS |
| git diff --check | 최종 전체 worktree 검사 exit0. 줄 끝 정규화 안내만 있으며 whitespace 오류 없음; final-diff-check.log |
| 전체 Effect V2 binding 검사 | 기존 미연결 curtain/dance leaf3개 때문에 실패. 새 얼굴 구조는 유효하며 전체 PASS로 기록하지 않음 |
| 공 CModel WARP probe | load true, mesh1/material0, Diffuse/Normal/Specular/Emissive4slot 로드 성공; kouku-ball-textures.json |

최초 Product 링크는 Client.ilk 열기 오류로 실패했다. Client 비증분 링크 성공 후 정규 Product
명령 전체를 다시 실행해 성공했다. 새 잡기 fixture의 최초5실패는 GameRoom 생성 때 로드된 기존
아레나 보스를 테스트 보스로 잘못 선택한 원인이었다. fixture의 synthetic 보스 목록을 명시적으로
구성한 뒤 실제 Room 검사 전체가 통과했다. 실패한 검사를 그대로 완료 증거로 쓰지 않았다.
기존 header codepage 경고와 외부 DirectXTK PDB 경고는 남아 있으며 최종 compiler/linker error는 없다.

## 공 재질 조사와 화면 확인 경계

현재 공의 WModel D/N/S/E 연결, DDS와 UV가 유효했고 실제 CModel에서도 네 texture slot이 로드됐다.
회색 화면의 원인을 재질 경로 누락으로 재현하지 못했으므로 binary나 shader를 임의 변경하지 않았다.
Client/UI 실행·조작·캡처는 수행하지 않았다. 블렌딩/레이저/얼굴/공/링/왼손 위치의 시각 결과와
사용자의 실제 Workbench 입력·Save 왕복을 PASS로 판정하지 않았다.

## 사용자 실행 순서와 남은 경계

LAN 설정은 server-host, firewall ready이며 지속 Server/Client는 종료 상태로 둔다.
Visual Studio의 `Server + Client` profile을 Ctrl+F5로 실행한다.
Lobby → KoukuSaydon → F1 → `KoukuSaydon Complete Play (Server Boss Replay)` →
`Load KoukuSaydon Inventory` → `Gate: 2관문` → Pattern 선택 →
`Complete Play - Selected Pattern`에서 확인한다. Bundle은 `Complete Play - N actors`다.

편집은 `Open Action Workbench` → `Boss: Large Saydon, Kouku`에서 한다.
잡기 Trigger의 `Capture Hold window`와 Success의 CAPTURE_PLAYER 연결을 볼 수 있다.
손 위치는 Result의 `Grip forward / up / right (m)`에서 조정한다.

초기 게시 revision229는 새 Server 시작으로 소비했다. 이후 G08/G10에서 쿠크 encounter 전용
게시·재승인을 구현하고 새 Product에 반영했다. 실행 중 run의 revision은 유지하며 새 Complete Play가
게시된 쿠크 revision을 승인한다. 쿠크 밖의 실제 balance와 world placement 변경은 계속 Server 재시작 대상이다.

## G08. Publish 뒤 다른 밸런스 때문에 거절되던 재승인 경로

사용자 첨부 화면은 Server REJECTED였고, 이전 Waiting 문자열이 별도로 계속 표시됐다. Load_PublishedKoukuProduct가 전체 게시물을 현재 Server와 비교하여 쿠크 밖의 별도 balance 변경까지 거부하던 경로를 수정했다. 새 게시물은 잠금 아래 기존 loader로 검증하고, 그 쿠크 소유 행과 현재 활성 catalog의 검증된 non-Kouku 행을 합쳐 동일한 Load_BootstrapBytes parser로 재검사한다. 별도 파일 런타임이나 미검증 map 복사 경로는 추가하지 않았다. 실행 중 generation·Restart·STOP의 pin과 fixed-tick admission은 유지한다. MainApp은 Server status와 별개로 남아 있던 초기 Waiting 문자열을 다시 표시하지 않는다.

별도 out 경로로 Server 전체 C++ 컴파일·링크 후 기존 --kouku-bundle-contract-test가 failures0이었다. publisher 잠금 중 거부, 기존 run Restart pin, 새700ms stage 실행, stale STOP 거부, 손상 문서 실패 보존, 미게시 source 거부, 외부 damage99999 변경을 제외하면서 새 쿠크 source만 승인, 과거 source rollback 거부를 실제 CGameRoom으로 확인했다. 현 Client/Server 프로세스나 제품 EXE를 교체하지 않았다. 근거는 out/KoukuLivePublishFix20260910/{build.log,contract.log}다. 최종 Product 빌드·사용자 아레나 검증은 후속 단계다.

## G09. 최종 저장·종료 대기 checkpoint

사용자는 편집 중이며 끝나면 Save/종료 완료를 알리겠다고 답했다. 따라서 source 데이터 통합, runtime publisher와 Product EXE 교체는 보류한다. task-local out/KoukuFollowup20260910/stage_changes.py는 최신 원본을 다시 읽고 기존 decimal/서식을 보존해 P13 내부20전환150ms, 거미 전용 blackout/얼굴300ms확대, P24손망치WORLD, logic31몸yaw90, 공 materialProfile을 준비한다. 현재 검증 후보는 source259→260이며 실제 저장 때는 마지막 사용자 revision을 기준으로 다시 만든다. 각 source의 기존 bytes를 모두 비교한 뒤 원자 교체한다. 실행 중인 도구의 메모리 draft를 대신 Save하지 않는다.

실행 준비 확인: 최종 MainApp/WorldObjectTool 개별 컴파일 성공. 새 charge 필드를 포함한 별도 Server 전체 빌드·링크와 기존 --kouku-object-overlap-contract-test/--kouku-bundle-contract-test 모두 failures0. 대포·트럼펫180도 및 망치 object/template/instance의5개 변경은 현재 worldsequences와 준비했던 값이 정확히 일치한다. 소품 Transform 공개 코드와 원본 Resources 모델도 보존됐다. 정식 Product는 아직 빌드하지 않았다.

P25 aura 읽기 검토: source259/published256의 anchor BOSS, follow, offsetY1.4500000477와현재start5465/duration7647이 같다. 실제 Snapshot→KoukuPresentationPlayer→Make_Pivot→V2 GROUP에서 offset을 한 번 합성한다. leaf 자체Y1이 더해져 최종중심은bossY+2.4500000477이며 미리보기와 제품이 동일하다. 사용자 손조정은 바꾸지 않는다. P13/P17은 reset=false이며 preview가live Snapshot position/yaw를 쓰도록 한 수정으로 Server 시작 기준에 맞는다. 무본collider localTRS와weaponBone*socket boss-local bake에 live yaw는 각각 한 번 적용된다. 여러 플레이어에서 서로 다른 retarget 대상을 택하면 방향 차이는 의도된 상태이며 사용자 최종 화면은 미검증이다.

도화가 기존Product/AltV는 Artist 결과G10, 차원술사 mapping/A/BA는 Round2 결과G38, 워로드Q는 Warlord 결과G13, 공과거미몸방향은09-10 개별결과에 검증 근거를 둔다. 해당 source 구현과 최소 검증은 완료했고, 최종 사용자 저장본 통합→WorldSequences Publish/Check→Kouku domain publish→Product Debug 빌드가 남는다.


## G10. 마지막 저장본 통합·게시·정규 Debug 실행 준비 완료

사용자가 Save/종료를 알린 뒤 마지막 Composition262를 다시 읽어263으로 통합했다. 중간에 사용자가
이전 EXE를 실행했으나 다시 종료를 확인한 뒤 정규 Product 빌드를 시작했다. 광역 원본 재저장 대신
대상 JSON 값만 교체했고, 사용자 R tuning과 기존 WorldSequence 소품 설정을 보존했다.

- P13 내부20개 animation 전환을150ms blend로 연결했다. 거미 logic35만 전용 완전암전 profile을
  사용하고 local light를 제거했다. 얼굴의 기존1000ms delay는 유지하며300ms 점확대를 연결했다.
- 거미 logic31의 몸 yaw는+90이며7m charge 이동 자체는 보존한다. Source/Encounter/Server의
  charge3개 구간이 같은 값을 소비한다. P24는 손망치 World18을 전체6502ms occurrence로 연결한다.
- 대포·트럼펫180도와 망치 actor/bone·Transform5개 설정은 마지막 저장본에서도 준비값과 동일했다.
  Object Tool/Action Workbench Pos·Rot·Scale 및 앵커 기준 보정 코드가 새 EXE에 들어갔다.
- 공 materialProfile41값/8texture를 worldsequences426에 통합했다. 기존4개 map light는 이미 켜져
  있었으며 공에 원본 PBR mask·IBL·BRDF를 연결했다. 전체 맵 모든 material family 복원은 아니다.
- Artist 지정7cue(Q/W/R2/A/S/F)와18개 전체 cue, DM 요청14개 연결과 A36개 Y·34개 sprite delay,
  BA4개 full/기존 All Effects3개 donor freshness를 마지막 저장본으로 다시 확인했다.
  사용자 R tuning SHA256는419219079911aaa39fa366446c2a4387ee3893bc1bb0e8189b840d9677c25d4d로 보존됐다.
- Warlord Q의 source bone scale 중복 적용과 Artist Alt V의 simulation provider GPU 집계 차이 수정이
  새 Client에 반영됐다. Q 원인을 확인했으므로 이전 효과로 우회하지 않고 full.restore 연결을 유지했다.

| 최종 검사 | 실제 결과 |
|---|---|
| WorldSequences Validate/Publish/Check | PASS, source/runtime426 전체 동등, material 입력8개 물리 파일 존재 |
| Kouku domain 게시 | PASS, Composition/Encounter/patternbindings/Server 모두263, Product22patterns/6bundles |
| RenderingProfiles Publish | PASS, source/runtime26 전체 동등, 거미 blackout 참조 닫힘 |
| 정규 Debug Product | Engine/Shared/Server/Client 컴파일·링크·배포 PASS, SkipBuild=False, missingRuntimeInputs0 |
| 새 제품 Server bundle 검사 | --kouku-bundle-contract-test exit0/failures0, 외부 balance 보존·게시 revision 재승인·실패 rollback 확인 |
| 새 제품 Server overlap 검사 | --kouku-object-overlap-contract-test exit0/failures0, 거미 charge/공포와 휠윈드 방향·거리·반복 hit·navigation 확인 |
| 변경 JSON/XML | 변경 JSON12개 및 project/filter XML4개 parse PASS |
| DLL 배포 | Engine/Bin/Debug와 Client/Bin/Debug의 Engine.dll SHA256 일치 |
| diff 검사 | git diff --check exit0; 줄끝 정규화 안내만 있음 |
| 사용자 화면 | 미실행. Client/UI 실행·조작·캡처 및 visual PASS를 수행하지 않음 |

Rendering publish 최초 검사는 exposure의 float32 최소값과 fog heightFalloff 하한을 거부했다.
새 profile만 각각0.10000000149011612와0.0001로 바로잡은 후 실제 publisher를 통과했다.
정규 빌드는 약288초였고 C4819/C4828·기존 FXC 경고와 외부 DirectXTK PDB 경고가 남지만
compiler/linker error는 없다. build receipt는 `out/BuildPipeline/runs/20260910T091016153Z-debug-product.json`다.
통합 증거는 `out/KoukuFollowup20260910/`의 publish.log, rendering-publish.log,
worldsequences-publish.log, worldsequences-check.log, product-build.log,
server-bundle-contract.log, server-object-contract.log, parse-check.json, final-source-check.json이다.

18:10 KST 새 Client.exe 생성·배포가 완료됐고, 확인 시 지속 Client/Server는 종료 상태였다.
사용자는 Visual Studio의 Server + Client profile을 Ctrl+F5로 실행한다. 쿠크는 Lobby → KoukuSaydon →
F1 → KoukuSaydon Complete Play에서 최신 inventory를 불러와 검증한다. Publish 뒤 기존 실행은 Stop하고
새 Complete Play로 게시 revision을 승인한다. 일반 승인 제한5초/예약 시작15초는 정상 대기 소요시간이
아니라 timeout이며 REJECTED 뒤 기다린다고 승인되지 않는다. 별도 Waiting 문구 잔류도 제거했다.

Bundle9/10은 개별 P24/P23를 유지하므로 개별 재생을 위해 분리할 필요가 없다. bundle은 동시 actor와
start offset의 묶음이며 같은 boss의 순차 진행 자체는 패턴 순서로 관리한다. 사용자가 지정하지 않은
전체 Gate2 진행 순서는 새로 만들지 않았다. P25 aura의 사용자 BOSS/offset은 그대로 유지하며
Server snapshot→presentation에서 offset1회 적용을 검토했다. 손 위치·몸 방향·공 반사·암전과 확대·
블렌딩 및 스킬의 최종 색/모양은 사용자의 인게임 확인 대상이다. 대규모 공동 dirty 상태이므로
자동 stage/commit/push는 하지 않았다.
