# 쿠크 카드 추적과 접촉 재생 Server 결과

## G00. 실제 반영

`PURSUIT_PROJECTILES` Logic의 Composition → targetedCombatVisuals/Encounter projection → Gameplay publisher → 18-field `PATTERNPURSUITPROJECTILES` bootstrap → Server Pattern duration clock → 기존 `CCombatObjectRuntime` 연결을 구현했다. 새 Shared packet과 제품 C++ 파일은 없다. 기존 Showtime과 일반 전투 객체의 계약은 유지한다.

Logic은 1~4개 Effect resource ID, 접촉 Effect resource ID, 속도(m/s), 접촉·생성 반경(m), 수명(ms), cadence(ms), wave당 수, homing을 갖는다. 수명0은 한 번 생성하는 homing에만 허용한다. 카드4개는 현재 Server의 pattern aggro target을 고정해서 매 tick 실제 위치를 향해 이동한다. 무한 수명은 임의의 큰 유한값으로 종료하지 않으며 자연 animation 종료 뒤에도 유지된다. target 사망·퇴장, owner 소멸과 명시 Stop은 기존 수명·source 취소 경로로 정리한다.

현재 row 경계는 visual pool1~4, countPerWave1~16, speedMps .01~100, contactRadiusM .01~10, spawnRadiusM0~100, lifetimeMs0~600000, spawnIntervalMs0~600000이다. duration은1~600000ms이며 Pattern 전체 시간 안에 있어야 한다. Lifetime0은 homing=true와 spawnInterval0 조합만 허용한다. cadence0은1회 생성이다. 생략한 countPerWave는 visual pool 수로 resolve한다.

Server는 Shared XZ swept circle로 접촉을 판정한다. 피해량은 추가하지 않았다. 빠른 이동에서 한 tick 안에 플레이어를 통과해도 접촉을 검출하고 해당 플레이어의 Server XYZ에 reliable burst event를 남긴 뒤 despawn한다. 유한 회전 카드는 접촉 또는 수명 종료에서 같은 burst를 한 번 재생한다. 생존하지 않는 target이나 명시 취소는 burst를 만들지 않는다. event의 `strHitId`는 정확히 `combatobject.kouku.pursuit` archetype에서만 pinned contact visual ID를 나타내며 Client 소비는 통합 작업이 담당한다.

## G01. 검증

실제 Server·Shared 전체 구현 TU를 Debug 설정으로 `out/KoukuPursuit20260917/server`에 격리 컴파일하고 link했다. 후속 수정은 해당 TU를 다시 컴파일했다. 최신 `server_probe.exe`의 기존 support-surface 계약과 신규 카드 검사는 **138 checks, failures 0**이다. 결과는 `out/KoukuPursuit20260917/server_result.log`, 빌드는 `server_compile.log`와 `server_link.log`에 남겼다.

신규 검사는 시작 시각 이전 무생성,4개 1회 생성,600초 초과 무한 수명, 움직인 target으로 방향 수정, 자연 Pattern 완료 후 생존, target 사망 정리,300ms 간격3개×4wave=12개, 유한 수명과 burst1회, swept contact의 burst 위치·despawn, 명시 Stop과 bootstrap admission rollback을 확인했다. 실제 Client·UI·GPU 재생 검사가 아니다.

Python projector/실제 PowerShell 함수의 신규3개 검사와 기존 Showtime 회귀3개가 통과했다.4개 looping template과 contact asset pin, 잘못된 resource·수치·무한 반복 거절,18-field bootstrap exact join, 기존 Showtime template·11-field row·World publisher optional lane을 확인했다. 변경 파일의 `git diff --check`가 통과했다.

## G02. 실제 데이터와 남은 경계

effect_data 작업이 반영한 Composition revision1185에서 P48과 P78의 pursuit2행과8개 Client template을 실제 projection했다. 근거는 `out/KoukuPursuit20260917/actual-pursuit-projection.json`이다. P78은6114ms부터4개,30m/s,접촉반경.5m,생성반경3m,수명0이고 P48은3533ms부터901ms 동안300ms 간격3개씩,15m/s,접촉반경.4m,생성반경.2m,수명1000ms다. 원작 수치와 사용자의 무한 수명 override 근거는 effect_data 통합 RESULT가 소유한다.

raw `validate_document`는 이번 대상 밖 기존 P32의0ms Pattern에 남아 있는20개 presentation occurrence 때문에 실패했다. 정식 `run → prepare_publication`은 saved inventory를 보존하면서 각 Pattern의 의존 closure를 별도로 검증하고 ready만 Product에 포함하므로 이 기존 불완전 Pattern은 정식 publish를 막지 않는다. 해당 문서를 삭제·축소하거나 validation을 우회하지 않았다.

World 저장이 끝난 Composition1186 / WorldSequences2034 checkpoint에서 정식 prepare가 사용하는 `_publication_candidate → validate_document → validate_publishable → projected_outputs` 단계를 P47/P48/P78/P79/P81에 각각 read-only로 실행해 **5개 모두 ready**와 각2개 출력을 확인했다. P32는 `PRODUCT pattern has no stages`로 별도 unavailable이며 source bytes는 변하지 않았다. 근거는 `out/KoukuPursuit20260917/selected-publication-readiness.json`이다. 앞선 전체 audit는 다른 agent의 World 저장을 freshness guard가 감지해 정상 거절했으며 guard를 제거하지 않았다.

이후 실제 Product publish·제품 빌드·Client 소비 통합 상태는 공통 작업 RESULT에서 별도로 기록한다. Client 실행·조작·화면 캡처·시각 PASS는 수행하지 않았다.

## G03. 실제 publish의 64개 상한 수정

정식 `prepare_publication`이 저장된 유효 Pattern을 ready Product로 모을 때 기존 64개 제한에 걸리는 별도 결함을 확인했다. 이 값은 wire 제한이 아니다. Server `GameplayCatalog.cpp`의 `PATTERNSEQUENCE`는 이미 Shared `MAX_VALTAN_PATTERN_FLOW_SLOTS=255`를 사용하고, Pattern ID는 string으로 전달하며 sequence count만 기존 uint8 슬롯 계약을 따른다. Server Pattern catalog는 vector이며 개별 Pattern의 64 Stage 제한은 전체 Pattern 수와 별개다. Client Composition과 Presentation은 각각4096개를 허용한다.

따라서 Python `MAX_PRODUCT_PATTERNS`와 Gameplay publisher의 header count를255로 맞추고, Client `KoukuSaydonBossTool.cpp`의 header 검증은 같은 Shared 상수를 직접 참조하도록 한 줄 수정했다. Server·Shared와64 Stage 제한은 바꾸지 않았다. Client 파일의 기존 BOM 없음·CRLF를 보존했다. 기존 Pattern 삭제나 격리 조작은 없다.

Python의 실제 validate/project 경로에서64·65·255개 수용 및256개 거절, PowerShell의 실제 header validator에서 같은 네 경계를 검증한2개 테스트가 통과했다. Client 검증식의 Shared 상수 참조와 Shared255 정의도 검사했다. Client 변경 TU의 제품 컴파일은 통합 빌드가 담당한다. 변경 파일 `git diff --check`는 통과했다.

수정 후 실제 `prepare_publication` 전체 경로도 통과했다. Composition1186은 saved83개 중 ready69개를 Product로 만들고 P47/P48/P78/P79/P81을 모두 포함했다. P32의 기존 no-stages unavailable은 유지되며 source bytes도 그대로다. 근거는 `out/KoukuPursuit20260917/publication-readiness.json` 및 `product-count-boundaries.log`다. 실제 publish 호출은 effect_data가 담당했다.

## G04. 원작 속도 해석 정정과 독립 이동 거리 제한

원본 `NU1V7NCQ4YAE9ZPJVNOQS.u`의 실제 class export/property를 읽어 EFSequenceSummonsProjectile의 ResScale/CollisionSize/Height/Speed/MaxSpeed/Lifetime/MaxDistance 형식과 순서를 확인했다. Missile421981901의1197-byte block은 `(1,40,30,800,1500,5,1500)`이며 초기8m/s, 최대15m/s, 수명5초, 최대거리15m다. G02의 당시15m/s·수명1000ms 후보와 기존 builder의 최소/최대 거리 해석은 정정 대상이다. 원작 가속식은 확인되지 않아 일정 초기 속도로 재생하고15m 또는5초 중 먼저 도달한 경계에서 종료한다. 기존 source builder와 timing builder의 필드명·수치 근거도 교정했다. 매칭 Trace421980901~904는 같은 구조의 Speed/MaxSpeed300cm/s와 MaxDistance3000cm이므로3m/s/30m다. 사용자 무한 추격과3m 생성 반경은 별도 저작 override로 보존한다.

`maxDistanceM`은 선택적인0~1000m 필드다. Client typed parser·validator·serializer·Logic 편집기, Python projection, PowerShell publisher, Server bootstrap과 기존 이동 거리 제한에 연결했다. 생략/0은 기존 정책, 양수는 누적 이동 거리 제한이다. 무한 lifetime0과 양수 거리 제한은 거절한다. 기존18열 row는 그대로 읽고 양수 거리만19번째 열에 추가한다. Client의 기존 GAZE_REAL_BOSS `maxDistanceM`은 별도 typed member로 보존한다. 새 packet이나 projectile runtime은 없다. 거리 종료에서 폭발이 빠지는 실제 실패를 확인해, contact presentation의 자연 종료 조건을 수명 종료에서 기존 `expired`(시간 또는 거리)로 맞췄다.

실제 Server·Shared 전체 TU를 격리 재컴파일하고 기존 support-surface 테스트를 확장해 **147 checks / failures0**을 확인했다. 기존 무한 추적·12발·접촉·Stop 검사를 유지하면서14wave42발,1초8m 이동과 잔여7m,15m 정확 정지·폭발42회,18/19열 admission과 잘못된 거리의 catalog rollback을 검증했다. Client 실제 Composition codec은 **23 checks / failures0**으로 양수 거리 roundtrip·상하한·무한수명 충돌·GAZE 기존 필드 분리·Save_Atomic/reload·freshness를 통과했다. 변경 Workbench TU도 격리 Debug 컴파일했다. Python projector와 실제 PowerShell 함수의 focused3 tests도 통과했다. 근거는 `out/KoukuPursuit20260917/server_result.log`, `source-projectile-field-proof.json`, `out/sayton-pattern-20260917/composition-distance.log`, `distance-workbench.compile.log`다.

사용자의 미저장 편집이 있어 실제 Data와 bootstrap을 교체하거나 publish하지 않았다.1196을 읽은 뒤 외부 저장으로 SHA가 달라진 것을 guard가 발견했고 최신1199를 새로 읽어 후보만 만들었다. `out/KoukuPursuit20260917/card-motion.patch.json`은 source SHA/revision, 기존·제안 Logic73/74와 P48 occurrence를 함께 기록한다. 같은 폴더의 `card-motion.candidate.json`은 검토용 전체 후보다. P48 연속 spin2833~7633ms 안에서3533~7433ms까지300ms 간격3개씩42발이며, 원작의4wave 이후 연장은 사용자 현재 clip 길이에 따른 저작 정책이다. 후보의 실제 projector는2행/8visual template 연결을 확인했다. 적용 전에는 최신 saved document와 미저장 draft를 다시 확인해야 한다. 제품 빌드·배포·Client/UI 실행·GPU/시각 판정은 수행하지 않았다.
