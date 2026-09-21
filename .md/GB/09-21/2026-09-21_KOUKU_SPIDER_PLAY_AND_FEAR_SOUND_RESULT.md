# 쿠크 거미 Play Pattern 상태와 공포 사운드 결과

## G00. 확인한 결함과 실행 한계

사용자가 본 `Requested the saved Pattern on the Server. Collider and Logic execution follows Server admission.`은 Server 거절 메시지가 아니라 Workbench가 로컬 요청을 예약한 직후의 고정 안내였다. 실제 Gate·리소스 준비는 MainApp/BossTool에서 나중에 실행된다. P15는 completion-chain/Mario 조건을 충족하지 않아 Workbench가 audition 상태를 추적하지 않았고, 준비 실패도 별도 MainApp 상태에만 남았다.

실행 중 Client PID13240의 로그에는 P15 local bundle-preview의 external-history 실패가 있으나, 해당 Play Pattern 클릭의 Gate·리소스 준비 결과나 Server 거절 이유는 기록되지 않았다. 이 오류는 active Effect instance만 정리하고 Server Play 준비 queue의 실패 receipt를 오염시키지 않는다. prewarm 자체도 고정한 V1 catalog revision/V2 cache generation을 변경하지 않는다. 따라서 사용자의 실제 재생 미시작 원인을 이 두 경로로 단정하거나 해당 검사를 제거하지 않았다.

현재 게시 source1957의 실제 Server GameRoom은 P15 단독과 Bundle6 모두 관문 보스 생성→QUEUED→ACTIVE를 통과했다. Update_WorldEntities의 실제 collider 접촉에서 tick155에 FEAR가 적용되고 종료 tick246, presentation `kakulsaydon.g1.logic.35`를 확인했다. 범위 밖 플레이어에게는 FEAR가 적용되지 않았다. 이는 격리된 실제 Server 소비자 검사이며 사용자가 실행 중인 Client 화면 검증은 아니다.

## G01. 반영한 코드

- `KoukuSaydonActionWorkbench.h/.cpp`: 명시적 Play Pattern은 P15를 포함해 모든 패턴의 admission 상태를 추적한다. 준비 상태 전달 함수가 실제 진행·실패 문자열도 받는다. 최초 안내를 서버 송신 완료로 오해하지 않도록 준비 대기로 바꿨다. 일반 local Preview의 completion-chain/Mario 정책은 유지했다.
- `MainApp.cpp`: 클릭 직후 준비 시작/실패, 비동기 준비 진행·종료 결과를 같은 Workbench에 전달한다.
- `KoukuSaydonCompositionDocument.h/.cpp`: FEAR의 optional `soundResourceId` 읽기·쓰기·stable ID/SOUND kind 참조 검증을 추가했다. 다른 result kind에는 허용하지 않고, 기존 무음 문서와 저장 실패 시 LastGood 보존을 유지한다.
- `KoukuSaydonActionWorkbench.cpp`: Fear Sound 선택과 clipboard 리소스 수집을 연결했다.
- `project_kouku_saydon_composition.py`: Client FEAR projection에 optional `soundResource`를 전달한다. Server에는 기존 stable presentation ID만 전송한다.
- `KoukuSaydonPresentationPlayer.cpp`: 로컬 FEAR session에 SOUND occurrence 하나를 만들어 얼굴과 같은 `effectDelayMs`에 재생한다. 기존 snapshot age seek·SoundCue handle·FEAR 종료 cleanup을 사용한다. 화면 particle의 반복이 별도 보이스를 중첩시키지 않는다.

새 제품 C++ 파일과 project/filter 등록은 없다. 기존 C++ UTF-8 무BOM/CRLF를 유지했다. 헤더를 잠근 프로세스는 Windows Restart Manager로 확인한 VS C++ 언어 서비스 PID32184였고 해당 보조 프로세스만 종료한 뒤 검증한 헤더를 반영했다. VS 편집 창과 Client/Server는 종료하지 않았다.

## G02. 사운드 후보와 데이터 적용 상태

기존 설치 리소스 `sound.kouku.018b3ad1ae968ba781cc`, event `s_mob_g_kouku1.g_kouku1_attack06_shotvox1`를 사용한다. 네 WAV는 각 2.233333초이고 Resources 상대 위치는 다음과 같다.

```text
Sound/KoukuSaton/S_MOB_G_KOUKU1/g_kouku1_attack06_shotvox1__21951570.wav
Sound/KoukuSaton/S_MOB_G_KOUKU1/g_kouku1_attack06_shotvox1__945224082.wav
Sound/KoukuSaton/S_MOB_G_KOUKU1/g_kouku1_attack06_shotvox1__357862284.wav
Sound/KoukuSaton/S_MOB_G_KOUKU1/g_kouku1_attack06_shotvox1__468344697.wav
```

물리 파일은 `Client/Bin/Resources/` 아래에 이미 있고 새 Resources를 설치하지 않았다. 원본 action4219776 stage002 notify005의 0.001초 돌진 보이스다. FEAR buff의 원본 참조는 Darkness 재질과 ScreenPP particle이며 직접 AkEvent가 없으므로 얼굴 피격에 재사용하는 연결은 PROJECT_TUNED다. 기존 돌진 보이스와 일부 겹칠 수 있다. 이번 정책은 얼굴 시작 1000ms에 한 번 재생하고 FEAR 종료 3000ms에 함께 정리한다.

`out/SpiderPlayFearSound20260921/KoukuSaydonComposition.candidate.json`과 `fear-sound-projection.json`으로 후보를 준비·검증했다. 이후 사용자가 모든 편집 저장 완료를 확인해 안전존·휠윈드와 함께 최신 Composition1985→1986에 logic35의 사운드 필드를 병합했다. 무관한 사용자 변경을 보존하고 hash 재확인·백업·원자 교체를 사용했다. 설치 증거는 `out/KoukuSafeZoneRepair20260921/applied.receipt.json`이다. **Publish는 사용자가 직접 수행하며 에이전트는 실행하지 않았다.**

## G03. 실행한 검증

| 검증 | 결과와 근거 |
|---|---|
| 실제 Server P15/Bundle6 admission·collider FEAR | 16 checks / 0 failures. `out/SpiderAdmission20260921/result.json`, `run.log` |
| 실제 Client resource closure 수집 | P15/P22, V1 4종, V2 11종, WORLD 0. `out/SpiderAdmission20260921/collector.log` |
| FEAR Python focused tests | 2 tests PASS. `out/SpiderCounterFearSound/focused-tests.log` |
| 실제 C++ codec·원자 저장·재로드 | 59 checks / 0 failures. optional 생략, bad kind/type/ID, missing/deleted/non-FEAR 거절, 실패 시 디스크/LastGood 보존. `out/SpiderCounterFearSound/codec/result.json` |
| MainApp/Workbench/PresentationPlayer | 각각 Debug/Release 실제 TU 컴파일 6개 PASS. `out/SpiderPlayFearSound20260921/compile/` |
| codec 의존 TU | CompositionDocument/AnimationActionDocument/DataJson/ProjectDataRoot/probe 5개 fresh compile+link PASS. 기존 struct ABI의 OBJ 혼합 없음 |
| 후보 JSON·기존 project/filter XML parse | PASS. `out/SpiderPlayFearSound20260921/validation.json` |
| `git diff --check` | PASS |

서버 검사는 병합 뒤 오래된 TU 33개를 out에 새로 컴파일하고 기존 제품 artifact56개와 데이터 snapshot63개의 불변을 확인했다. Client 컴파일도 산출물을 out에 격리했고 헤더 잠금 중 사용한 후보는 이후 설치한 소스 헤더와 byte 동일함을 확인했다.

정식 Debug Product runner는 `out/SpiderPlayFearSound20260921/product-build.log`와 `out/BuildPipeline/runs/20260920T200023525Z-debug-product.json`에 **실패**로 기록됐다. 실행 중인 `Client/Bin/Debug/Client.exe` PID13240과 `Server/Bin/Debug/Server.exe` PID14988의 output guard가 빌드 전 종료시켰다. Product 링크·EXE 교체 성공으로 기록하지 않는다. 이 guard의 메시지를 JSON 데이터 반영에 Client 종료가 필수라는 규칙으로 확대하지 않는다.

## G04. 남은 최종 반영과 사용자 확인

승인된 logic35 사운드 참조의 디스크 병합은 완료했다. 사용자 직접 Publish와 새 schema를 소비하는 Client/Server의 최종 Debug Product 빌드·재시작은 남아 있다. 제품 링크를 위해 실제 실행 파일 점유를 사용자가 해제해야 한다. 자동 종료·Reload는 하지 않았다.

새 빌드에서 Lobby→KoukuSaydon→F1 Sequencer Boss→P15 거미 카운터→Play Pattern으로 실제 준비/거절 메시지를 확인한다. 피격·회피·카운터 성공을 구분해 얼굴 시작과 보이스, FEAR 종료 정리를 청취한다. 현재 실행의 정확한 미시작 원인과 화면·음성 판정은 아직 미확인이다. 코드 변경, 후보 검증, 게시, 실행 중 메모리 적용을 구분한다.
