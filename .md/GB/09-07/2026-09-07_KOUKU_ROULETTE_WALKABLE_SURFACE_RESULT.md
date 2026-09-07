# 2026-09-07 쿠크 룰렛 임시 보행면 결과

## G00. 구현 상태와 실행 경계

[구현 계획](2026-09-07_KOUKU_ROULETTE_WALKABLE_SURFACE_IMPLEMENTATION_PLAN.md)의 원형 보행면 수직 슬라이스를 구현했다. 기존 Navigation의 높이 질의를 확장하고 쿠크 WORLD cue가 활성인 동안 같은 서버 room의 정지·이동 플레이어에 유효 지면 높이를 적용한다. 기본 navgrid 원본이나 Client player Transform을 변경하지 않는다.

자동 저장·투영 검증과 해당 도메인 배포는 완료했다. 변경 C++ Product 컴파일 및 `Server --kouku-support-surface-contract-test` 실행은 root의 묶음 빌드 뒤 확인할 항목이다. Client/UI 실행·조작·캡처를 하지 않았다. 사용자의 룰렛 위 걷기 확인은 아직 수행되지 않았다.

최신 Composition revision95에서 `KAKULSAYDON_G1_PATTERN_7` 룰렛은 **DRAFT**다. 따라서 현재 제품 투영에는 룰렛이 포함되지 않고 `PATTERNWORLDSUPPORT` 행도 없다. 원본의 DRAFT 상태를 임의로 PRODUCT로 바꾸지 않았다. 사용자가 룰렛을 PRODUCT로 저장하고 해당 domain을 배포한 뒤 Complete Play에서 실제 보행면을 소비한다. 좁은 검증은 원본을 변경하지 않는 임시 입력으로 실제 support bootstrap 행 생성까지 확인했다.

## G01. 저장본 보존과 저작

사용자가 저장하고 Client/Server를 종료한 것을 root가 확인한 뒤 CAS 병합했다. `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`은 revision396에서397로 증가했고 `world.sequence.instance.8`에 아래 optional 필드만 추가했다.

```json
"walkableSurface": { "radiusM": 2.5, "localHeightM": 0.026313 }
```

추가분은71bytes이며 JSON 전체 비교에서 revision과 위 필드를 제외한 값이 모두 동일했다. 원본의 줄바꿈·배치·기존 float 표기는 유지했다. 별도 backup과 CAS 증거는 `out/KoukuWalkableSurface/saved-before-publish/`, `out/KoukuWalkableSurface/source-cas-evidence.json`에 있다.

| 문서 | revision | SHA256 |
|---|---:|---|
| WorldSequence 변경 전 |396|`7106a79f0e99f8c5f922666116b6c2e78d60004a0c16ab6baffe3bc185a661fc`|
| WorldSequence 변경 후 |397|`f68492d68586bae5264e2ebc2007611f71302d9d1b5b49fc1c2abe00c5f4ef8b`|
| Composition 보존 |95|`8856242fed4c524bac4911de73207663e50f8002a0904b47f88938cf0bcc1bbc`|

`WorldSequenceDocument`가 optional 필드 parse/save/equivalence/validation을 소유한다. `WorldObjectTool` Motion Detail의 `Walkable Surface → Enable Circular Walking Surface`에서 반지름과 로컬 높이를 수정하고 기존 Save로 저장한다. 단일 MAP_PLACEMENT, WORLD anchor, STOP, 고정 위치·크기, Y축 회전만 허용한다. native animation이나 기울어진 면은 지원한 것처럼 근사하지 않는다.

## G02. Geometry와 투영

대상 연결은 `world.object.kouku.roulette → world.sequence.instance.8 → sequence.LV_LUT_MIDNIGHTC_ED.5 → MAP_PLACEMENT 40`이다. 설치된636vertex wmodel의 주평면 raw Y는 약2.6313이고, import pre-scale0.01을 적용한 높이는0.026313m다. placement scale4를 적용하면 평면이 placement origin보다0.105252m 높고 반지름은10m다. 장식의 최대 Y를 보행 높이로 사용하지 않았다.

projector는 기존 placement 위치·Y축 회전·크기에 고정 Transform key와 local height를 적용한다. Room은 WORLD offset `(0,0.58,0)`과 기존 Boss Spawn anchor 변환을 더한다. instance delay와 instance/cue speed를 적용한 visibility 구간은30Hz tick 올림으로 변환한다. 마지막 숨김 키 또는 WORLD cue 종료에서 면이 제거된다.

`Publish-MapAuthoring.ps1`은 optional 필드를 검증하고 그대로 runtime에 전달한다. Kouku projector는 지원 면을 기존 WORLD cue의 부가 정의로 출력한다. `Publish-GameplayBalance.ps1 → GameplayCatalog`의10필드 `PATTERNWORLDSUPPORT` 행은 encounter/pattern/occurrence stable ID와 상대 start/end tick, center X/Z, height Y, radius만 전달한다. Shared network protocol과 기존 WORLD 행은 변경하지 않았다. `BuildDomains.json`의 Kouku product 입력에는 실제 소비하는 WorldSequence와 Map placement 원본을 추가했다.

## G03. Navigation과 Room 소비

`CServerNavigation::Set_RuntimeSupportSurfaces`는 원형 면 목록을 검증·stage한 뒤 root와 detail region에 함께 commit한다. 잘못된 교체는 이전 목록과 revision을 유지한다. 유효 높이는 기존 walkable 지면 위에서만 `max(원래 바닥, 활성 원판)`으로 선택한다. 기본 막힌 셀, NO_SURFACE 및 runtime blocker는 열리지 않는다.

`Sample_Position`, `Cell_ToPoint`, A* cell traversal, LOS, 실제 movement step은 같은 effective height를 사용한다. 원의 XZ 경계는4m 격자 중심으로 확장하지 않는다. 같은 셀 안의 좁은 높은 원판도 실제 교차점 앞뒤 높이를 검사하므로 기존1m step 정책을 우회하지 않는다. 목록 변경은 기존 Nav revision과 경로 무효화를 사용한다.

`GameRoom`은 이동 전에 WORLD 소유권과 시간표를 갱신한다. run/member/pattern sequence/occurrence ID가 면을 소유한다. 다른 member가 살아 있는 면을 한 member STOP이 제거하지 않는다. cue 만료, owner STOP, abort, run reset과 마지막 플레이어 퇴장에서 면을 정리한다. 룸 재초기화 때 보행면을 먼저 제거하므로 새 boss spawn 높이에 임시 면이 섞이지 않는다.

정지 플레이어도 활성·제거 tick에 지면 높이를 다시 읽는다. 낙하, 붙잡힘, pattern bind, arena ejection, trigger move, knockback은 기존 별도 Y 권위를 유지한다. Server snapshot이 결과 Y를 전달한다.

## G04. 수행한 자동 검증과 배포

기존 테스트 파일에 추가한 좁은5개 메서드만 실행했고 모두 PASS다.

- placement scale과 주평면 높이, 마지막 숨김 시각 투영.
- start delay·speed와30Hz 올림을 포함한 두 visibility 구간.
- 움직이는 위치, 기울어진 회전, 잘못된 원 반지름 거부.
- 실제 Gameplay publisher 함수의10필드 owner 연결 행 생성 및 WORLD 수명 초과 거부.
- 실제 Map publisher 함수의 정상 저장 및 음수 반지름·boolean 높이·알 수 없는 필드·이동·기울기·비균일 크기 거부.

Python AST3파일, PowerShell AST2파일, BuildDomains JSON parse, 변경한12개 C++의 UTF-8 BOM 없음/CRLF 검사, 변경 범위 `git diff --check`를 통과했다. 새 C++ 파일이 없으므로 project/filter 신규 등록은 없다.

프로세스 종료를 확인한 뒤 아래 두 명령을 실행했다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 95
```

Map Area publish는3231placement/7파일 PASS, Kouku product와 gameplay.balance도 PASS다. Kouku product는 현재 저장된 PRODUCT5개/57stage를 출력했다. 출판 뒤 WorldSequence authoring/runtime JSON revision397 동일, Composition95 원본 해시 불변을 다시 확인했다. 기존 player hit-shape partial coverage 경고는 이번 기능의 오류가 아니며 기존 fallback 정보를 그대로 출력했다.

## G05. 남은 확인

기존 `ServerGameplayContractTests.cpp`에 `Run_ServerKoukuSupportSurfaceContractTests`를 추가했다. flag는 `--kouku-support-surface-contract-test`다. 격리4m nav fixture에서 실제 원 경계, A*/LOS/이동, 높은 subcell 원판, blocked/NO_SURFACE 보존, 잘못된 교체의 기존 상태 보존, 정지 플레이어의 시작·종료 높이 및 member별 STOP을 검사한다. 별도 harness 파일이나 광역 진단 실행은 추가하지 않았다. Product 빌드 후 이 명령의 실제 결과를 본 문서에 갱신한다.

수동 확인은 사용자가 룰렛을 PRODUCT로 저장·배포한 뒤 `F1 → Saved Patterns → 룰렛 → Complete Play`에서 수행한다. 원판 안팎 걷기, 원판 아래 정지 시 생성 순간의 높이, 종료 후 원래 지면 복귀가 대상이다. 에이전트의 visual PASS는 기록하지 않았다.

## G06. 통합 후 native 검증

Debug Product 빌드·링크·SDK/셰이더/runtime 배포 성공. receipt는 `out/BuildPipeline/runs/20260907T113006867Z-debug-product.json`이다. `Server/Bin/Debug/Server.exe --kouku-support-surface-contract-test` 종료0, 11개 사례 PASS(failures0). 원판 내부/정확한 경계, A*/LOS/이동, 한 셀 내 최대 step 초과 거부, blocked/NO_SURFACE 보존, 잘못된 면의 transactional 거부, WORLD 시작/종료와 정지 player 재지지, owner별 정리 및 전체 종료를 확인했다. 로그는 `out/KoukuWalkableSurface/server-support-contract.log`. 이것은 GUI/아레나 실행 또는 visual PASS가 아니다. 현재 룰렛 Pattern7 DRAFT 상태는 사용자 저장본 그대로 보존했다.
