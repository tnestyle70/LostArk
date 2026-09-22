# 쿠크 1관문 패턴·카드비·오디오 통합 결과

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
