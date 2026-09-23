# 레이드 자막·낙사·복귀와 앵콜 연출 통합 전달

## 포함 범위

1. 보스 자막: 쿠크2관문 클리어+3관문 입장 기준의 scale2/offsetY90을 쿠크38개 자막과
   발탄을 포함한 WorldSequence 화면 자막에 적용. 월드 말풍선과 개별 occurrence 편집은 유지.
2. 도박장: 실제 지면을 벗어나면 서버 낙사, 사망 후 부활 명령은 도박장 중앙으로 복귀.
3. 마리오1~4: 발판 사이 구멍에 걸어서 진입하면 낙사. 사망자는 HP0/DEAD로3관문에 복귀.
   피해 사망도 동일하며 정상 점프, 살아서 완료, 서버 이동 권위와 충돌 검사는 유지.
4. 레이드 종료: 베른에서 입장한 실제 NPC ID를 개인/파티 이동에 보존하여 그 NPC 앞으로 복귀.
   베다 고정 좌표는 제거하고 현재 배치 앞2.5m를 높이 포함 네비게이션에 투영.
5. 베른 입장 컷신: 한 Client 실행 중 최초 재생/ESC 이후 레이드 복귀 때 재생하지 않음.
   재실행 시 다시 최초 진입이며 영구 시청 기록은 추가하지 않음.
6. 앵콜: 가짜 클리어 UI·문구를 Sequence/Boss 공통 재생 시간에 연결하고 유리 후처리에 포함.
   앞5초를 배우/카메라/이펙트/음성/BGM/자막과 함께 단축. 새 총 길이21322ms.

각 기능의 상세한 원인과 검사는 같은 날짜의 BOSS_SUBTITLE_STYLE, KOUKU_FALL_MARIO_DEAD_RETURN,
RAID_RETURN_ENTRY_NPC, BERN_ENTRANCE_ONCE_PER_SESSION, KOUKU_ENCORE_CLEAR_UI_TRIM RESULT를 따른다.

## Git 및 데이터 전달

- PR452까지 포함한 main7e818914를 fast-forward 반영했다. 이번 PR은 그 위의 새 브랜치다.
- 통합 전 이전 Gameplay 생성물은 명명한 stash에 백업하고, 수동 병합 대신 정본 publisher로
  다시 생성하여 팀원의 최신 플레이어 hit shape 행과 이번 쿠크 revision을 함께 보존한다.
- 변경된 Data 원본, 쿠크 projected Encounter/patternbindings, Client Map 카메라/WorldSequence,
  Server Gameplay.bootstrap 및 변경된 Composition 게시본/receipt를 함께 전달한다.
- 개인 endpoint, .claude/.codex, 배포 임시파일, out, Resources, exe/dll/lib/pdb/CSO는 제외한다.
  무관한 기존 로컬 파일은 삭제하거나 되돌리지 않는다.

## 리소스 배포 조사

이번 변경 JSON14개에서 main 대비 추가된 바이너리 의존성0개. 기존 RaidClear 키프레임이
참조하는306개 리소스 모두 존재. 새 모델/애니메이션/텍스처/음원을 생성하거나 수정하지 않았다.
따라서 이번 PR 때문에 Drive에 추가 배포할 Resources 파일은0개다. 기존 팀 Resources 전체는
여전히 필요하며, 이 결론은 사용자가 별도로 수정한 무관한 ignore 리소스의 전수 감사가 아니다.
자동 조사 기록: out/raid-pr-resource-audit-20260923.json.

## 통합 검증 상태

- Debug Engine/Shared/Server/Client 빌드 성공. 최신 main 추가분은 코드가 아닌 판정 데이터다.
- 앵콜 집중 검사7개와 베른 컷신 검사4개 PASS. git diff --check PASS.
- 최신 main 통합 후 Gameplay Publish PASS: 쿠크revision2224와 플레이어 hit shape92/92 검증.
- 최종 Composition Publish PASS. Client Composition snapshot/receipt와 Server 게시본을 함께 포함.
- 게시 후 NPC 복귀 검사18개 PASS / exit0: out/raid-pr-npc-return-test-20260923.log.
- 게시 후 낙사·마리오·레이드 통합 검사8021개 PASS / failures0 / exit0:
  out/raid-pr-fall-test-final-20260923.log.
- 최초 통합 검사에서 앵콜이23333ms 이상이어야 한다는 이전 기대값만1~4인에서4회 실패했다.
  사용자가 요청한5초 단축 후 저장/게시 길이21322ms로 해당 기대값을 수정하고 전체 재검사했다.
  실행 중 Server.exe를 건드리지 않고 동일 소스의 ServerRaidPRContract.exe를 별도 링크했다.
  빌드 기록: out/raid-pr-server-contract-build-final-20260923.log.
- 사용자가 앵콜 표시를 확인했으며 전체 연출·낙사·복귀의 최종 화면 판정과 Release 빌드는 별도다.
- 실행 중 사용자 Server/Client는 종료하거나 조작하지 않는다. 디스크 게시를 실행 중 메모리의
  자동 갱신으로 설명하지 않는다.
