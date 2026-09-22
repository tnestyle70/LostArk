# 쿠크 1관문 패턴·카드비·오디오 통합 결과

현재 설치 상태는 G06 이후의 revision2195이며, G00~G05는 최초 설치와 그 뒤의 검증 이력이다. 복구 보완까지 포함한 공식 Debug Product 빌드는 `20260922T045116788Z-debug-product.json`에서 PASS했다(OBJ105, CSO5, binary1). 별도 Effect Tool 회전 수정과 최종 화면 확인은 이 빌드 성공에 포함하지 않는다.

## G00. 실제 설치

사용자의 실제 반영 승인을 기준으로 최신 Composition revision2193을 다시 읽어 stable ID/field로 병합하고 revision2194로 저장했다. `out/Gate1Restored20260922/applied-manifest.json`의33파일(Effect14, Composition/Catalog/Tree/Independent, Resources13, project/filter2)을 기존값·직전hash 확인, 백업, 원자 교체와 자기 변경 rollback 경로로 설치했다. 최초 Clown·MAZE·핑·14그룹 화염파동23파일도 앞서 별도로 설치됐다. 실행 중 메모리 draft나 Server 상태가 자동 갱신된 것은 아니다.

Gate2/Gate3/Bingo flow, P58 전체, P100 기존 visual3개, 사용자 back_shot 문서는 보존했다. 통합 독립 검토에서 변경 패턴·Effect의 Resources 참조198개가 모두 존재하며 설치33파일이 후보hash와 같음을 확인했다. 신규 Data는96.DataFiles None에 등록했다. 기존 C++ 인코딩과 CRLF를 유지했다.

## G01. 패턴 흐름·판정

1관문 flow28항목을 요청 순서로 교체했다. P101은 기존2134ms clip 두 개를 유지하면서 Server BOSS_TRACK_TARGET 이동을 사용한다. P104 `세이튼_1초추적`은1000ms이며 체력 패턴 사이에 별도 항목으로 들어간다. 다른 관문 흐름은 바꾸지 않았다.

P100은 원본 Action4219873/SkillEffect421992501의2000..2500ms, BOX halfExtents[1,.5,4]와 기존 좌표 변환에 따른 local center[-4.3,0,0]/yaw90을 사용한다. 원본 Key2의 Area primitive만 재사용하고 피해량10%는 프로젝트 조정값이다. 원본 거리500~520cm와시간2151~2171ms의 중간값5.1m/2161ms를 기존 authoritative ballistic knockdown에 연결했다. fixed g9.8 결과 peak5.720653m는 원본 HitTypeHeight300cm와 다르므로 원본 높이 복원 완료로 기록하지 않는다. 일반7개 클래스의 KNOCKDOWN→LAND→DOWN→STANDUP 소비는 연결되어 있으나 LAND 전환은 기존 clip 완료 기준이며 Clown 변신의 특수 피격 clip은 기존 미지원 상태다.

P103의 기존 카드낙하 duration[3243,6995)에서500ms마다 현재 Saydon 주변12m/유효nav/높이허용1m에 Effect group을 투하한다. scale1~2를 Server가 결정해 최초 CombatObject spawn, late join, retry와 타격 radius/offset에 동일하게 적용한다. Shared protocol103으로 함께 갱신했다. 원본 카드7m→.35m 곡선과 착지1650ms, 원본 재질·emissive를 소비하며 노란 경고·낙하·폭발이 하나의 Effect이다. 위치 반경·간격·scale·피해10%는 프로젝트 저작값이다. 이미 생성된7200ms tail과 투하 duration을 분리한다.

CARD_RAIN_SOLDIERS trigger는 원본NPC480726/480727/480728과 일치하는 기존 CLUB/HEART/DIAMOND 모델·MonsterProfiles로 각1마리를 Server Spawn_Monster에서 생성한다. maze 상태 등록은 하지 않는다. 수량·3~6m 범위·30초상한/owner패턴 종료 정리는 프로젝트 조정값이다. 기존 카드미로 profile의 일반 타격 대상·표현을 재사용하며 원작 병정 공격AI 추가로 기록하지 않는다.

## G02. 이펙트·사운드·BGM

P48은 원본 준비/머리카드1회와 기존6개 저작 투척의208요소 파생을 분리해 중복 준비카드를 제거했다. 기존6행 start/duration/position은 유지했다. DJ 쿠크는 원본 portrait231·GameNoteFrame와 원본YG760 폰트로 만든 한국어 배너2개를 기존 V2 ScreenPost LEAF로 재생한다. 원본 art와 새로 rasterized한 문구를 구분한다.

Dice floor/release는 Server isPatternBound와 기존 presentation lifecycle에 연결했다. 원본 준비·카드 출력·충돌 폭발, 무력화 방패·별 선·무지개 폭발을 연결하고 임시V2중복18개를 제거했다. P102 화염링7개의 xyz에 실제G1−G3 delta[약0,약0,-204.800017]를 더했다. Backstep 잔상은 실제 설치MN_RPCT_05 model pose/source diffuse/5ms/.5s를 사용하며 흰색 반투명 스타일은 PROJECT_AUTHORED이다. 자세한 근거는 Dice/Stagger/Backstep RESULT를 따른다.

베른과발탄 지정 WAV는 이미 올바른 진입/전투 상태에서 소비하므로 설치원본일치와 코드연결을 대조했다. 쿠크 관문1~3·Maze·Bingo·Mario1~4의 원본 Wwise Resume target을 복원하고 loop metadata를 보존했다. 기존 terrace2nd와새 전투 selector의 한 owner가 시퀀스·컷씬·카메라 재생 중 BGM을 억제하며 반복 snapshot으로 재시작하지 않는다. P100/P103 원본clip notify, 카드비/배송voice, Dice·도넛·팡파레 폭발의 사운드30개 변경을 연결했다. 짧은 ‘빠밤’ sting은 독립 원본 이벤트를 아직 특정하지 못해 임의 파일을 연결하지 않았다.

## G03. 검증과 게시·빌드 상태

수정 Client9TU/Server9TU의 MSVC Debug scratch compile PASS. SHOWTIME/카드비 projector13테스트 PASS. 실제 packet read/write·invalid·retry·late join60검사, 실제 navigation/random-only/scale176검사, BGM selector31검사, FMOD NOSOUND12WAV load/loop 검사 PASS. 이 소형 검증은 Product 링크나 GPU·실제 청취를 대신하지 않는다.

Dice/Stagger9후보 native66,299 samples·Beam332점·실제model4200palette PASS. Clown/MAZE11,254검사·핑303·입력35·delete15·flame219,335검사의 이전 결과는 각 RESULT를 따른다. source emitter lifetime은 source recipe만finite120초, 수동30초와2048capacity를 유지한다.

최종 변경 JSON 33개와 프로젝트 XML 2개 parse, 설치 receipt 33개 및 전달 리소스 24개 hash/size 검사 PASS. 근거는 `out/Gate1Restored20260922/final-check.json`이다. `git diff --check`도 PASS했다.

Kouku owner publisher는 source revision 2194로 PASS했다. koukusaydon.product, map.kakulsaydon, world.gameplay, gameplay.balance 네 domain을 게시했고, product 94개 패턴·500개 stage·9개 bundle을 생성했다. 근거는 `out/Gate1Restored20260922/publish.log`이며 총 315,575ms다. 마지막 CARD_RAIN_SOLDIERS 툴 선택항목 추가 후 Workbench TU도 다시 컴파일해 PASS했다.

공식 Product Debug 빌드 명령을 실행했으나 ProductOutputGuard가 실행 중인 Client PID 56948과 Server PID 55976의 표준 출력 점유를 확인해 컴파일 전에 중단했다. 근거는 `out/Gate1Restored20260922/product-build.log`와 `out/BuildPipeline/runs/20260922T034122434Z-debug-product.json`이다. 프로세스를 자동 종료하거나 출력 경로를 우회하지 않았다. 따라서 최종 EXE/DLL/CSO 교체와 새 Server의 `--card-maze-contract-test`, `--kouku-support-surface-contract-test` 실행은 미완료다. 데이터 설치·게시와 실행 중 메모리 갱신을 구분하며, 사용자 종료 상태 응답 후 동일 공식 명령으로 이어간다.

## G04. 리소스 전달

`C:/Users/user/Desktop/GBResources`에 Character/Effect/Sound/UI 상대 경로로24파일351,616,235바이트를 추가했다. 기존373개 payload는 보존했고 새24개 SHA/size 일치를 확인했다. `manifest-2026-09-22-kouku.json`과README를 제공하며 Resources wrapper는 없다. source코드·Data JSON·shader는Git 변경으로, binary assets는이 전달폴더로 구분한다. 사용자 화면·실제 청취와 실행 중 도구 Reload는 수행하지 않았다.

## G05. 후속 질문에서 발견한 반복 누락 보완

사용자의 리소스·Effect·추적 질문에 답하기 위해 소비자를 다시 확인했다. revision 2194의 28개 Flow entry는 정확했으나 Server의 마지막 index 순환이 BINGO에만 있었고 Client Play_Flow는 마지막 entry 완료 후 종료했다. 앞서 대화에서 Gate 1 반복이 구현됐다고 설명한 것은 이 시점의 실제 소비자와 달랐다.

기존 `GameRoom_KoukuRaidFlow.cpp`의 관문별 순환 조건과 continueRaid 조건에 GATE1을 추가했다. index 0으로 돌아갈 때도 이미 승인된 epoch와 catalog를 유지하여 잔여 Effect/Logic의 소유권이 새 수동 실행처럼 초기화되지 않게 한다. 최초 입장의 epoch 0 조건은 유지한다. `KoukuSaydonPatternAuditionService.cpp`는 GATE1만 마지막 완료 후 저장된 wait를 거쳐 index 0을 다시 제출하며 기존 Stop/거절/중단/world/revision 검사를 보존했다.

P101 `세이튼_플레이어추적`은 2,134ms 독립 패턴으로 8곳, P104 `세이튼_1초추적`은 1,000ms 독립 패턴으로 5곳에 있다. 둘 다 `logic.132 → DURATION/BOSS_TRACK_TARGET`을 전체 구간 소비한다. 28개 entry의 waitAfterMs는 모두 0이다. 이 보완에서 Data/Resources/packet을 다시 수정하지 않아 revision 2194 및 게시본은 그대로다.

두 CPP의 수정 후 Client/Server 종료를 확인하여 공식 Product Debug 빌드를 다시 시작했다. 결과는 완료 후 아래에 기록한다.

독립 검토에서 레이드 재시작은 실제 audition을 지워도 raid의 이전 epoch가 남는다는 점을 확인했다. index 0의 GATE1 continuation은 실제 audition의 nonzero epoch와 관문도 일치할 때만 허용하도록 한정했다. 처음 시작·재시작은 fresh admission을 사용하고 정상 순환만 기존 실행을 이어간다.

최종 두 CPP에서 Client Cancel_Flow/Update_Flow/Stop 함수, 실제 continuation 조건, Server 완료 receipt→마지막 wait→index 순환→다음 entry 분기를 그대로 추출한 native probe 43개가 PASS했다. 두 주기, 마지막 wait, 진행·대기 중 Stop, abort/reject/world/revision 변경, 다른 관문 보존, 최초·재시작·정상 순환의 epoch와 관문 조건을 확인했다. `out/Gate1FlowLoop20260922/probe.log`와 source-receipt의 SHA가 최종 CPP와 일치한다. 이 검증은 full room이나 사용자 화면 실행을 대신하지 않는다.

첫 제품 Server의 카드미로 검사는 78개 PASS, failures 0이었다. SupportSurface는 최초 환경 변수 미설정으로 끝의 catalog fixture를 실행하지 못해 실패했고, 104개/23,274,851바이트의 별도 DataFiles 복사본을 LOSTARK_SERVER_DATA_ROOT에 지정해 재실행했다. 이 실행에서 잘못된 Showtime window가 최종 catalog 검증을 건너뛰는 실제 결함 1건을 발견했다. fixed/tracking이 모두 없더라도 후속 random 행이 붙을 수 있어 parse 단계의 허용은 유지하고, 전체 행을 읽은 뒤 SHOWTIME_PLAYER_TARGETS도 기존 Brain validator를 반드시 호출하게 연결했다. 테스트를 완화하지 않았으며, Server 증분 빌드 후 재검증한다.

수정된 제품 Server로 같은 격리 DataFiles와 기존 `--kouku-support-surface-contract-test`를 다시 실행하여 exit 0, failures 0을 확인했다. 잘못된 Showtime identity/lifetime/interval/speed에서 이전 catalog를 보존하는 검사가 이번에는 PASS했다. 근거는 `out/Gate1Restored20260922/support-surface-final-product-test.log`다.

Client/Server 종료 후 첫 공식 Product Debug 빌드는 PASS했다(`20260922T041132722Z-debug-product.json`). 후속 증분 빌드는 다른 동시 작업의 Client project reference 메타데이터 손상으로 MSB3107 실패했으며 해당 작업에서 메타데이터가 정상 복구된 것을 확인했다. 이펙트 재검토 수정까지 포함하는 최종 Product 빌드는 별도로 기록한다. G03의 최초 출력 점유 실패는 당시 이력이며 현재 최종 상태로 사용하지 않는다.

## G06. 이펙트 재검토 후 실제 데이터 보완

재검토에서 발견한 P48 중복 투사체, 카드비 개별 사운드 누락, G1 화염파동 간격·tail을 최신 디스크에 병합했다. `out/Gate1EffectReaudit20260922/applied-manifest.json`의 14파일을 hash 재확인·백업·원자 교체·자기 변경 rollback 절차로 반영했으며 Composition revision은2195다. 아래 설치 상태는 이전 G02의 첫 구현보다 나중 상태다.

P48의 기존6개 occurrence는 사용자 저작 시작시간·길이·TRS를 모두 유지하고 resourceId만 `effect.kouku.common.spinning.card.casting`으로 바꿨다. 이 파생은 링·먼지16요소를 보존하고 baked projectile192요소를 제외한다. 실제 날아가는 카드는 기존 Server PURSUIT_PROJECTILES의 4문양 표현이 담당한다. 기존 6발/300ms·8.2초 window와 사용자 1.5배 카드 설정은 보존했다. 머리 위 준비카드는 `.overhead`4요소로 분리돼 있다.

P103 random set은 기존 노란 장판·카드 메시·폭발 Effect와 원본 Projectile421980301의 개별 SOUND를 함께 소유한다. 원본 AkEvent timer1350ms를 각 투하 birth 기준으로 적용했고6개 원본 변형을 SoundCueCatalog에 연결했다. 카드 시작1500ms와 충돌1650ms는 그대로다. 크기1~2·현재 Saydon12m·walkable/지면 높이 검사는 유지된다. 병정은 별도 trigger에서 현재 Saydon3~6m의 유효 nav 위치에 각1마리 생성하며 카드 낙하 위치를 공유하거나1관문 전체 cell에서 선택하는 구현은 아니다.

G1 화염파동은14개 그룹의 전체 중심과 기존 방향을 유지한 채 모든 행 가로3.5m, 행간3.031m로 배치했다. 그룹당28요소의 위치만 이동했으며 각 그룹의 개별 이동·저장 ID는 보존했다. 최초 설치 이후 사용자 위치 수정이 없음을 SHA로 확인했다. 실제 CPU 재평가에서 FireWave 바닥의 마지막 visible sample은7.283초였으나 기존5352ms occurrence에서 잘렸으므로 두 occurrence를9100ms로 늘렸다. Pattern visual duration15247ms와 animation stages11534ms를 분리해 기존 tail 소비자가 바닥을 마무리하며, 보스 애니메이션·Logic·다음 패턴 시계를 늘리지 않는다.

새 Sound admission은 실제 C++ Composition parser/validator/save roundtrip10검사와 projector/기존Showtime15테스트에서 PASS했다. 잘못된 SOUND 부착·SOUND 단독 세트를 거부하고 기존 문서를 보존했다. 실제 GPU 검사에서 카드비 native2864/2865, 머리카드 및 서버4문양 카드13조건×8샘플=104 draw가 모두 nonzero였고 D3D11 error0이었다. 2865의 emissive와 scene-depth 소비자도 확인했다. 이 검사는 재질 경로이며 실제 전투의 WModel 배치·외형 판정은 아니다.

1관문 BGM은 사용자 후속 지정에 따라 `Sound/KoukuSaton/S_BGM_COMMANDERRAID/midnightc_ed__398225682.wav`로 바꿨다. 해당 파일에는 smpl 구간이 없어 전체120.177초를 반복한다. 이전 `Raid/gate1.wav` 선택을 현재 연결로 설명하지 않는다.

전달 폴더에는 기존24개 중 이번 작업이 추가했던 미사용 `Raid/gate1.wav`만 SHA와 참조 부재를 재확인해 빼고, 지정BGM1개와 카드비 원본음6개를 추가했다. 현재 요청의 전달 리소스는 **30파일339,604,137바이트**이며 설치본과 SHA가 모두 같다. 다른 기존 파일은 보존했다. 외부에서 없어진 폴더 루트 README/manifest는 재생성하지 않고 현재 Character/Effect/Sound/UI 양식을 유지했다. 최종 목록은 `out/Gate1EffectReaudit20260922/gbresources-final-manifest.json`이다.

## G07. 설치 이펙트 이름과 복원 방식

아래 `effect.kouku.` 접두사는 생략해 표시한다. 툴 표시명은 따옴표 안의 이름이다.

| 요청 | stable asset ID / 표시명 | 연결·복원 방식 |
|---|---|---|
| Clown Q | `clown.polymorph.q` / Clown / POLYMORPH / Q / 폭탄 던지기 | 기존 폭탄 메시와 원본 폭발10요소, Q animation binding. 투척 궤적은 프로젝트 저작 |
| Clown W | `clown.polymorph.w` / Clown / POLYMORPH / W / 공 | 기존 공 리소스와 원본 sk02/sk02_1, W binding |
| Clown E | `clown.polymorph.e` / Clown / POLYMORPH / E / 선물상자와 하얀 폭발 | MN_PPPP_00 선물 respawn/idle 모델 추가, 원본 하얀 폭발12요소와 E binding |
| MAZE LMB | `cardmaze.lmb` / 카드미로 \| LMB \| 이펙트 저작 | 요청한 뿅망치_휘두르기_끝2요소,7개 class+Clown binding |
| MAZE Q | `cardmaze.q` / 카드미로 \| Q \| 이펙트 저작 | 요청한 바주카포 jump10요소,7개 class+Clown binding |
| 핑·머리 과녁 | `effect.world.ping`, `effect.world.target_reticle` | Ctrl pending 머리 표시와 다음 LMB의 nav 지면 핑. UI 소비·취소 경계 포함 |
| 화염파동 | `common.flame.wave.decal`, `.full`, `.full.koukusaydon` | 원본 바닥과 링,14개 stable 수동 그룹, G1/G3 간격 축소 |
| 카드비 | `gate1.cardrain.drop` / 세이튼_카드비_노란장판과낙하폭발 | 노란 경고1+원본 카드15+폭발11. CDO LocationDirect 배율과 중복7m 원점 수정, 서버 random set 연결 |
| 회전카드 | `common.spinning.card.overhead`, `.casting` | 머리카드4요소·링/먼지16요소 분리. 실제 발사체는 `card.spinning.heart/clover/diamond/spade` |
| DJ 안내 | `boss.kouku.dj.cardrain`, `boss.kouku.dj.delivery` | 원본 portrait/frame/font를 조합한 새PNG2개, 기존 ScreenPost 경로 |
| 주사위 속박 | `card.match.bind.floor`, `.bind.release` | 바닥 문양·버블11요소/해제9요소를 Server bound lifecycle에 연결 |
| 주사위·카드 출력 | `card.match.dice.full.restore`, `.prepare.full.restore`, `.emit.full.restore` | 기존 주사위53요소 보존, 준비14·출력19요소 복원, 카드 충돌과 원본 사운드 연결 |
| 무력화 | `gate1.stagger.shield.full.restore`, `.star.draw.full.restore`, `.explosion.full.restore` | 원본 방패10·별 선28·무지개 폭발38요소. 임시 폭탄 표현 교체 |
| 백스텝 | `gate1.backstep.afterimage` | 실제 Saydon 모델 pose 잔상, 흰 반투명 스타일은 프로젝트 저작. P102 화염링7개에 G1−G3 보스 스폰 차이 적용 |

고대의 바다 SPACE의 Trail/Ribbon은 툴의 inactive CPU 삭제 경로를 수정했다. 사용자 문서의 Trail/Ribbon을 임의로 일괄 삭제하지는 않았다. 모든 이펙트의 최종 게임 화면·청취 확인은 수행하지 않았다.

## G08. Interaction 준비·취소 후속 검증

후속 소규모 오디오 요청: `Level_CharacterSelect.cpp`의 Initialize 성공 직전에 로비와 동일한 `Sound/BGM/Lobby/bgm_wallpaperin.wav`를 `CRuntimeAssetRoot::Resolve → CGameInstance::Play_Music(...,1.f)`로 연결했다. 기존 WAV를 재사용하므로 추가 Resources는 없다. 해당 TU의 Debug scratch compile과 diff check PASS(`out/CharacterSelectBgm20260922/compile.log`); 이 후속 변경의 Product 재링크·청취는 수행하지 않았다.

Clown/MAZE 첫 action의 비동기 리소스 준비 지연을 실제 Server action age 안에서 재시도하도록 보완했다. queue가 수락한 action은 중복 제출하지 않고 clip/playRate, Server lock, Effect duration을 지난 요청은 버린다. 선택적 typed weak `PendingAdmission`을 Character가 action 동안 소유하며 새 action·취소·사망·class/form 교체 시 해제한다. service의 두 pending commit 경로도 만료 여부를 확인하므로 같은 프레임에 취소된 이전 action이 뒤늦게 생성되지 않는다. active E의 자연 꼬리와 기존 nullopt 호출자는 유지한다.

현재 실제 branch·age/lock 함수·commit guard를 추출한 C++ probe는87검사 실패0이다. 모델과 queue 주변은 test double이며 실제 GPU 검증과 구분한다. source SHA 및 로그는 `out/Gate1EffectReaudit20260922/interaction-retry-source-receipt.json`과 `clown-maze-ping-review.md`를 따른다. 이5개 CPP/H를 포함한 공식 Debug Product 빌드는 위 receipt에서 통과했다.

## G09. 재질 GPU 경로와 원본 상수 보완

무력화 별 선의 native3008은 CPU가 허용한 sprite를 생성된 shader의 ribbon 전용 guard가 제외하고 있었다. 해당 원본 material만 particle/ribbon 양쪽에 연결했다. 실제 DDS를 사용한 D3D11 WARP 검사에서 수정 전0픽셀→수정 후2,212픽셀, alpha0 control0, D3D error0을 확인했다. 원본 ribbon 경로와 PS 식은 유지했다.

불바닥 native2873은 원본 MacroUV/Opacity 입력 위치가 shader prefix에서 어긋나 실제 DDS 결과가0픽셀이었다. 원본 상수 배열과 renderer MacroUV 연결,6문서36개 요소의 원본 CDO radius200cm를 복원했다. 수정 뒤 실제 DDS1,844/1,757픽셀, alpha0 control0이며 인접 native2874 결과는 유지됐다. 설치6문서의 실제 Codec/Playback과 원본 source emitter 중심에서390 MacroUV 샘플을 검사해 실패0이었다. 전체 화면의 밝기와 원작 동일성 판정은 별개다.

백스텝 Preview도 기존 실제 NPC 모델 잔상 경로와 preview clock을 사용한다. pause60frame·advance·tail·rewind·stop·mode exit를 추출한 실제 시계 로직으로 검사했다. Product의 기존 실제 시간 경로는 유지했다. 위 소스는 `20260922T045116788Z-debug-product.json` 빌드에 포함됐다.

상세 자산·timing·제한은 `out/Gate1EffectReaudit20260922/flame-dice-stagger-backstep-review.md`에 기록했다. P78 마지막 카드 emit의 사용자1255ms window와 P1 별 선의10,694ms window는 보존했다. 원본의 더 긴 잔여 표현 전체가 보인다는 판정은 미확정이며, 사용자 재생 시간을 일괄 확장하지 않았다.
