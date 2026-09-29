# 발탄 갑옷 파괴와 배틀 아이템 연결 결과

## G00. 범위와 현재 상태

branch는 codex/kouku-release-sequence-ready, 시작 HEAD는3a55be17c다.
직전 발탄·쿠크·마리오 수정과 사용자의 Camera/UI 저장본을 보존했다.
사용자는 Debug 검토를 계속하고 Release만 먼저 검증하도록 요청했다.
Debug Client41820/Server52740의 종료·Reload와 Client/UI 자동 실행은 하지 않았다.

아이템 사용의 Data→Shared→Server→Client 연결, 원작 리소스 설치와 GBResources 전달을
완료했다. Server·protocol·원본 재질 수치와 ribbon 기하 검증은 아래 실제 실행 근거에
한정한다. 실제4클라의 피킹·UI 배치·GPU 재생·음향은 아직 사용자 화면 확인이 필요하다.
최종 Release Client/Server 빌드와 게시 데이터 검사를 완료했다. 상세 증거는 G06을 따른다.

## G01. 실제 인벤토리와 입력 경로

Data/Items/ItemCatalog.json에 네 종류의 battleUse와 시간 정지 물약 정의·원본 icon을
연결했다. 기존 베른 물약 상점 shop.bern.potion은 파괴 폭탄·회오리 수류탄·성스러운 부적·
시간 정지 물약을 각각161 SILVER에 판다. 가격은 프로젝트 설정이며 원작 가격 복원 주장이 아니다.
HUD의1~4는 현재 배치한 item ID와 마우스 목표를 PlayerController→IPlayerCommandSink로
제출한다. Server는 소지·쿨다운·생존·대상과 거리 검증 뒤에만 수량을 소비한다.

Debug/Release 공통 F1의 Battle Items에서 개별10개 또는 Give all four (10 each)를
요청한다. 기존 Server 인벤토리 지급 command를 사용한다. 지급 후 I로 인벤토리를 열고
네 슬롯에 drag한 뒤 F1을 닫아 숫자키로 사용한다. 이 버튼은 효과 직접 재생·쿨다운 해제가 아니다.
실제 상점 UI 클릭·드래그는 자동 실행하지 않았고 기존 인벤토리/슬롯 소비자 연결을 확인했다.

Shared protocol version122의 C2S_USE_ITEM은 requestSequence, itemId, groundTarget 여부,
XZ와 대상 아군 net entity ID를 보낸다. ground와 아군 ID를 동시에 보내거나 ground=false에서
좌표를 보내는 비정규 payload는 거절한다. Item bootstrap version6의 BATTLEITEM은15열이며
마지막 staggerMaximumDivisor가 회수에3, 나머지에0이다. 구 실행 파일과 새 schema를 섞지 않는다.

## G02. 폭탄과 무력화

두 폭탄은 기존 CombatObjectRuntime의 Server-owned 투사체다. ground 피킹 위치로 비행하고
보스 접촉 또는 목표 도착에서 실제 hit와 HIT_PULSE를 한 번 보내고 despawn한다. Client는
기존 projection의 spawn/snapshot/event/despawn으로 비행 effect를 이동하고 착탄 effect를
재생한다. 중복 착탄은 무시하고, 착탄 리소스 실패 시에도 종료된 비행 effect를 정리한다.

원본 Projectile의 속도1000cm/s, 범위700cm와 수명6초를 사용한다. 발사 높이100cm와
곡선 높이150cm는 프로젝트 표현값이다. 비행 effect의 원본 ResScale은 파괴1.1, 회수1.2로
각 document의 particleSystem.uniformScaleMultiplier에만 적용했다. Server와 Client root는
1이므로 중복 배율이 없다. evidence는 source/grenade_motion_evidence.json이다.

사용자의 최종 결정에 따라 회수의 HP 피해는0이다. 매번 ceil(최대 무력화/3)를 적용하므로
최대1000이면334씩, 세 번째에1000으로 clamp한다. 잔량 비율이 아니고 강한 HP 피해로 대신하지
않는다. 기존 일반 스킬의 HP/무력화 정책은 유지한다. 쿠크 STAGGER_WINDOW는 기존 HP 감소와
별도 아이템 기여량을 합산해 표시 잔량과 성공을 계산한다. 실제 HP에는 아이템 기여를 쓰지 않는다.
유지된 이전 occurrence에는 그 창의 기여량을 적용하고 새 창에는 새 상태를 사용한다.

파괴 폭탄은 기존 typed partDamage3과 공격력100% HP 피해를 사용한다. 이 값들은 프로젝트
정책이다. 기존 스킬의 partDamage100 같은 별도 저작 수치를 일괄 변경하지 않았다.

## G03. 성부와 시간 정지

성부는 현재 렌더되는 아군의 실제 CModel pose를 ray picking하여 stable net ID를 제출한다.
Server가 자신·GUIDE_AI·다른 파티·사망·범위5m 밖·마리오/구속/낙하 등 부적합 대상을 거절한다.
승인되면 해당 아군의 FEAR를 즉시 해제하고3초 동안 새 FEAR와 충돌 피해를 막는다.

시간 정지는 본인에게3초 동안 이동·스킬 제한과 공간 피격 제외를 적용한다. 벽·바닥·낙하 등
이동 물리는 유지하고, 직접 encounter failure wipe는 두 아이템 보호를 우회한다. 기존 무적 tick과
합치지 않고 각각의 종료 tick으로 처리하여 즉사 칼날 충돌과 전원 전멸 판정을 분리했다.

ActiveBuffs의 holy32282/time33500 종료 tick을 Client가 소비한다. 늦은 snapshot은 이미
경과한 만큼 effect를 seek하고, 같은 owner/endTick은 중복 생성하지 않는다. 스킬 교체와 독립인
Character occurrence를 사용하며 만료·사망·캐릭터 교체·despawn·reset에서 해당 effect만 정리한다.
시간 정지의 원본 buffcolor RGB(0.8,0.8,0.8)는 기존 ownerControls를 통해 연결했다.
3초 envelope는 사용자 요청을 위한 PROJECT_ADAPTER이며 원본 전체 material logic 복원 주장이 아니다.

## G04. 발탄 갑옷과 원작 표현

기존 MN_RPBF_01_Parts1/Parts2 모델과 Server part mask가 갑옷 표시를 소유한다.
primary BOSS_VALTAN의 phase3 이전 DASH_CHARGE recovery/GROGGY에서 살아 있는 갑옷이
남아 있을 때만 녹색 부위 파괴 marker를 기존 health bar의 world projection 경로로 표시한다.
모든 갑옷이 제거되면 marker를 숨긴다. PART_BROKEN sequence를 중복 제거하여 mask1은
420627, mask2는420628 파편을 발생시키고 발탄 위치에 파괴 텍스트를1.4초 표시한다.

녹색 PNG는 원본 FX_TEX_02.fx_d_symbol_029_cl의 ring/ticks/center channel과 원본색에서
추출한1024px 이미지다. stable slot Valtan_ArmorBreakReady는 reference72×72이며 head bar
아래8px다. 원본의 움직이는 내부 noise·bloom까지 정적 PNG에 포함했다고 설명하지 않는다.
상세는 out/ValtanArmorPresentation20260929/armor-break-implementation.md와
green-marker-extraction.receipt.json, out/ValtanArmor20260929/part02-restoration.receipt.json이다.

## G05. 이펙트 복원과 공유 리소스

World의 마우스 표시 다음에 네 item effect를 정렬했고 두 폭탄의 .flight도 별도 stable asset으로
등록했다. 아이템6개 document에는59 emitter,51 color·21 distortion native program이 있다.
신규 ID는5312~5362다. 단순 흰색으로 네 아이템을 대체하지 않고 원본 source/material을 사용한다.
신규6개와420628의7개 document에서125개 runtime dependency(10,370,560bytes)를 확인했다.

회수 비행 ribbon은 원본 sheetspertrail5를 읽는다. portable codec의 지원 범위1~8과 실제
renderer 기하 adapter를 연결하고 sheet마다 기존 GPU buffer에 독립 draw한다.1장 기존 경로와
최대500points×25tessellation의 기존 buffer 용량을 유지한다. UE CPU 원문 복구 주장은 아니다.

추가 asset과 공통 의존성을 C:/Users/user/Desktop/GBResources에 상대 경로 그대로 복사했다.
총163개15,431,600bytes이며 설치본과 대상의 SHA-256이 모두 같다. 두 갑옷 파편, 네 item icon,
녹색 marker와 native texture/model material까지 포함한다. 기존 무관 파일을 삭제하지 않았다.
out/BattleItemEffects20260929/GBResources.receipt.json이 파일별 증거다. Git에서 제외되는
Resources 실물 전달과 소스·effect JSON의 Git 전달은 구분하며 별도 원격 업로드는 하지 않았다.

## G06. 실행한 검증과 남은 경계

- Publish-ItemCatalog Publish 성공, CheckPublished unchanged 확인. Server는 생성된
  Server/Bin/DataFiles/Items/Items.bootstrap만 읽는다. effect와 UI JSON은 Data 정본 직접 소비다.
- Release Shared와 Server compile/link 성공. 마지막 GameRoom_KoukuAudition·hit 수정 뒤
  Server-Release-final.log의 재빌드도 성공했다. 기존 중복 project item 경고는 범위 밖으로 남겼다.
- 최종 Release Server --battle-items-contract-test:38개 PASS, failures0.
  실제 room·catalog·grant·consume·cooldown·duplicate, 비파티 거절, 성부 FEAR/피해와 만료,
  시정 lethal collision admission과 명시적 wipe, 두 보스의3회 회수·HP 불변·착탄/제거,
  쿠크 기존 스킬 기여와 합산을 실행했다. 로그는 battle-items-contract-final.log다.
- Release NetworkProtocolHarness의 --battle-items-only:14개 PASS. 네 payload roundtrip과
  비정규/범위 밖/비유한 값의 writer·reader 거절 및 reader 실패 시 기존 출력 보존을 확인했다.
- 실제 C++ ribbon settings/기하 함수 추출 실행:2,112개 PASS. 단장·5장·8장과 축/간격/단면,
  legacy index를 확인했다. GPU 장면 검사가 아니다. ribbon-sheets-test.log에 기록했다.
- 원본 decal PS의 DXBC와 translated raw HLSL WARP 비교는96instruction·6RT·5seed 모두
  worstRelativeDelta0이다. decal-native-parity.json에 기록했다. 제품 carrier GPU parity와 다르다.
- effect7개 구조/자산 검증과 프로젝트 XML parse·중복0 통과. 현재 소스로 실행 가능한 기존
  전용 codec runner가 없어 오래된 검증 EXE는 실행하지 않았다.
- Release Client의 shader/CPP compile·link와 runtime DLL/CSO 배포 성공. 마지막 소스 변경이
  포함된 첫 빌드 뒤 재실행한 Client-Release-final.log도 PASS이며 추가 compile이 필요하지 않았다.
  기존 X4000/X4008/X4717·C4819 등 경고는 남지만 오류는 없다. 새5312 mesh/particle CSO의
  실제 배포본과 EXE/Shared.lib/Items.bootstrap SHA-256은 final-release-receipt.json에 기록했다.
- Product -Configuration Release -SkipBuild 실행의 runtime 존재·Navigation 참조·Items와
  Valtan reward 게시 일치 검사는 PASS다. 영수증은
  out/BuildPipeline/runs/20260929T110525414Z-release-product.json이다. 이 검사는 배포 검증이고
  컴파일 근거는 위 MSBuild 로그다. 추가 publish나 Client 실행을 하지 않았다.
- 변경 JSON/XML29개 parse, GBResources163개 최종 해시 재대조와 git diff --check PASS.
  final-static-validation.json에 파일 목록을 남겼다. 현재 process/user/machine의 data-root
  override가 없고 Debug/Release 아래 별도 Data/DataFiles/Resources 사본이 없음을 확인했다.
  실행 중 다른 process의 환경·메모리를 확인한 것은 아니다.

로그·영수증의 기본 폴더는 out/BattleItemEffects20260929다. Debug와 Release의 Data 및
Resources는 공유하지만 기존 Debug EXE와 실행 중 Server 메모리는 갱신하지 않았다.
프로토콜과 item schema가 바뀌었으므로 실제 확인은 같은 새 빌드의 Client/Server로 진행해야 한다.
현재 테스트는 클라이언트4개를 실행한 네트워크/GPU 종합 검증을 대신하지 않는다.
