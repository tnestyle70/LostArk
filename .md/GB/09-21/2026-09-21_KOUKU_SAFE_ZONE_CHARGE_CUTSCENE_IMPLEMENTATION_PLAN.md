# 쿠크 안전존·휠윈드·컷씬 모델 구현 계획

## G00. 현재 저장본과 작업 경계

사용자 요청은 파1빨2에서 즉사를 막는 안전존, 입장 즉시 및 2초 간격의 파란 무적 문구,
안전존에서 즉사·공포·최대 체력 50% 피해를 모두 막는 동작이다(사용자의 후속 답변 기준).
안전존 밖에서 시선 실패한 플레이어는 공포와 50% 피해를 함께 받는다.
현재 rev1974의 P11 gaze36에는 Result6(50% 피해)과 Result37(공포)가 연결되어 있다.
Logic101은 아직 이름만 있는 DURATION이고 연결 Collider가 없다. 사용자 Publish와 편집은 유지한다.
기존 거미 Play 상태 전달·공포 사운드 변경은 같은 브랜치에서 보존한다.

## G01. 서버 판정과 무적 표시

`INVULNERABILITY_ZONE` DURATION은 기존 linked Collider region과 시간 창을 소비한다.
같은 Pattern의 즉사·최대 체력 비례 피해·FEAR Result를 보호 영역 안에서 차단한다.
Server가 입장과 2초 경과를 판단해 `SNAPSHOT_PLAYER.iInvulnerabilityZonePulseTick`을 전달하고,
퇴장·종료 시 0으로 지운다. Shared writer/reader와 protocol, Server snapshot을 함께 변경한다.

Composition codec·Workbench·Python projector·Gameplay publisher는 같은 kind를 검증한다.
이 Duration에는 Result 슬롯이 없다. 공포와 피해는 기존 GAZE_REAL_BOSS의 같은 Fail 슬롯을 소비한다.
Client Level은 Server pulse를 `CStatusEffectTextView`의 파란 `무적`으로 표현한다.
텍스트 중복 제거는 owner와 단어별로 구분해 FEAR와 보호 문구가 서로 재생을 반복시키지 않는다.

## G02. 플레이어 방향 휠윈드

P24는 수평 root 배율 0에 더해 Logic38의 기존 10m charge가 제거돼 이동 경로가 없다.
기존 Server target capture와 navigation/body collision 제한을 유지하며 charge 데이터를 복구한다.
실제 모델 전방과 charge yaw를 대조한다. 레이저와 팡파레 이동 설정은 이 회귀 수정에 포함하지 않는다.

## G03. 컷씬의 별도 파생 모델

2관문 입장 Kouku/Saydon, 2관문 미로 Kouku, 3관문 입장 SaydonArrival, 빙고 앵콜 Saydon은
복구한 Character 모델과 별개의 WModel을 소비한다. 원본 indexed corner 기준으로 노멀·탄젠트와
handedness를 복구하되 파생 모델의 위치·UV·skin·index·애니메이션은 보존한다.
후보와 원본 hash 및 section별 비교 결과를 out에 준비하고 최종 반영 때 재확인한다.

## G04. 검증과 최종 반영

실제 Server 시뮬레이션으로 보호 범위·입출입·2초 주기·공포와 50% 피해의 두 Result 순서를 검증한다.
새 kind의 codec roundtrip/실패 보존, projector geometry 및 outcome 제한, Shared wire roundtrip을 확인한다.
변경 C++는 실제 컴파일 옵션으로 검증하고 점유된 EXE를 임의 종료하지 않는다.
새 C++ 파일은 없으므로 프로젝트/filter 추가 등록은 필요 없다. JSON/XML parse와 diff check를 수행한다.
후보 완성 뒤 저장본 기준 반영을 한 번 확인하고 stable ID·필드 단위로 최신 데이터에 병합한다.
파일 반영, Publish, 제품 링크, 실행 중 메모리와 사용자의 화면·청취 확인은 RESULT에서 구분한다.

## G05. 저장 완료 승인 뒤 중앙 이동과 낙사 밀림

사용자가 모든 편집 저장 완료를 확인해 이전 후보를 최신1985→1986으로 반영했다. Publish는 사용자가 직접 수행한다.
추가 요청의 휠윈드 기준은 실제 Result39의2m/242ms다. 레이저 P21·슈퍼바주카 P85는6m,
대형 세이튼의 기존 damage contact12개는16m로 설정한다. 공용 Result42와98은 다른 패턴이
공유하므로 요청 범위의 Result를 복제해 연결한다. 기존 피해량·접촉 시각을 보존한다.

Logic103 중앙 이동은 `BOSS_TELEPORT_GROUNDED`로(4.66,10.56,322.94)를 연결한다.
현재 navigation의 실제 높이는10.5599994659이며 XZ를 cell center로 반올림하지 않는다.

MAX_HP_PERCENT_DAMAGE Result에 optional `forcePush`, `pushCanLeaveArena`, `pushYawOffsetDegrees`를
같은 codec→projector→publisher→Server 계약으로 추가한다. 기본 false/false/0은 기존 동작을 보존한다.
두 flag의 true는 양수 push를 요구하며 yaw는 BOSS_FORWARD 양수 push에서만 허용한다.
레이저·바주카 collider local+X와 기존 body+Z의 차이에 따라 중앙 발사는 yaw+90도를 적용한다.
P21의 비스듬한 두 발사는 실제 BOX 축에 맞춰+135.9000015도와+45도의 독립 Result를 사용한다.
대형 contact의 forcePush는 공포·다운·기존밀림·기상 grace를 중단하고 새 밀림을 적용한다.
사망·낙하·잡힘·맵이동 상태는 제외하고 기존 안전존이 차단한 Result는 밀림도 적용하지 않는다.
pushCanLeaveArena는 요청한 공격에만 켜고 서버가 바깥 경계 통과를 확인한 때 기존 FALLING→DEAD를 소비한다.
거리가 끝나기 전에 경계를 넘지 않거나 실제 collision에 막히면 낙사로 위장하지 않는다.
