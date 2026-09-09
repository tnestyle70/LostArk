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

이 변경은 서버 전체 Hot Reload 구현을 포함하지 않는다. 이번 게시 revision229는 새 Server
시작으로 소비된다. 이후 Save/Publish만으로 실행 중 Server의 revision이 바뀌지는 않으며
현재는 Server 재시작과 Client 재접속이 필요하다. 한 버튼 무중단 교체는 별도 검토 범위다.
