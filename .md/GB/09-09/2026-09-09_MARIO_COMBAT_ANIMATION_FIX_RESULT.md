# 마리오 공격 입력·모션·망치 연결 교정 결과

## 구현 상태

- PlayerController 마리오 조기 return이 Q/W까지 소비하던 오류 수정. InteractionSlot 제출,
  Server INTERACTION snapshot, Character의 기존 MARIO Q/W clip 연결을 사용한다.
- 마리오 Q 승인 후 12틱(30Hz 기준 0.4초)에 한 번 전방 타격한다. 같은 stage, 높이차
  0.8m 이내, 전방 120도, 2.4m+몬스터 반경. 기존 HP 1 몬스터는 한 번의 유효 타격에 죽는다.
- W는 기존 저작 동작이며 Q 피해를 빌려 쓰지 않는다. 일반 class 스킬/마우스 이동 차단 유지.
- 원본 PSA의 자식 bone quaternion conjugate 누락 수정. root는 유지한다.
  REUP PSK bind→glTF 실측 최대 오차: 기존 약 1.0 → 교정 후 5.93e-7.
- normalized UINT8 glTF skin weights를 FLOAT VEC4로 확장한 후 Assimp에 전달한다.
  기존 일부 RHKP/CDMD 가중치의 garbage/Inf 문제를 함께 교정했다.
- REUP/RHKP/CDMD 재쿠킹. 망치 원본 소켓 REUP `bip001-prop1`, RHKP `b_wp_1` 유지.
  각 망치 2,718개 최종 vertex가 해당 뼈에 weight 1로 연결됨을 확인했다.
- 마리오 6종 순찰을 원본 `walk_normal_1` 계열로 변경. 카드미로 병사 설정은 변경하지 않았다.

## 실행한 자동 검증

- 기존 cooker unittest: 10개 통과. root/child 순서 역전, normalized byte 가중치 확장 포함.
- 모델 3종 geometry 30,470 vertices의 position/normal/UV/tangent 바이트 불변.
  새 모델 전체 가중치 finite/nonnegative, bone index 범위, 합 1 검사 통과(최대 합 오차 4.85e-8).
- 실제 WModel parser로 뼈대/clip을 읽음. 사용되는 bone inverseBind×restCombined 최대 오차
  REUP 0.000231, RHKP 0.000167, CDMD 0.000233(모델 cm 단위).
  idle/walk/attack 각 5시점, 3종 총 45 pose 수치 샘플 검사. 시각 PASS 의미가 아니다.
- Server Debug Build 성공. 기존 `--debug-teleport-contract-test`: failures 0.
  1~4 stage에서 Q InteractionSlot → Update_Players → 접촉 전 무피해 → 접촉 시 사망,
  뒤/다른 층/다른 stage/거리 밖 제외, 중복 피해 없음, W에 Q 피해 없음 검사 통과.
- Client Debug Build 성공. 기존 shader 경고 및 DirectXTK PDB 누락 경고는 남아 있다.
- WorldGameplay publisher Validate 성공. MonsterCatalog JSON, Client/Server project XML parse 통과.
- 변경 경로만 지정한 `git diff --check` 통과. 저장소 전체 검사는 이번 변경과 무관한
  LFS mapplacements clean filter의 `.git/lfs/tmp` 권한 오류로 중단되어 전체 PASS로 기록하지 않는다.
- 독립 read-only 비평 2회(공격 경로, 후속 가중치 변환)에서 추가 결함 지적 없음.
  물리 키 입력과 화면 모션은 자동 실행하지 않았으며 실제 Client 확인은 아래 미완료로 구분한다.
- 실행 로그: `out/MarioOriginalReplacement/server-combat-animation-fix-build.log`,
  `client-combat-animation-fix-build.log`, `server-combat-animation-fix-contract.log`.
  첫 검사 시 로그 상대 경로 오류로 실행되지 않았으며, 경로를 교정한 실제 실행 결과만 위에 기록했다.

## 리소스 및 실행 준비

교체한 Resources-relative asset ID(기존 재질/텍스처 변경 없음):

- `Character/Monster/MarioOriginal/REUP/REUP.wmodel`
- `Character/Monster/MarioOriginal/RHKP/RHKP.wmodel`
- `Character/Monster/MarioOriginal/CDMD/CDMD.wmodel`

물리 위치는 `Client/Bin/Resources/` 아래이며 이 PC에는 복사 및 바이트 일치 확인을 마쳤다.
수정 전 모델 3개는 `out/MarioOriginalReplacement/backup-before-animation-fix/`에 보존했다.
팀 배포 시 위 파일 3개를 Drive로 전달해야 한다. Drive 업로드나 Git commit/push는 하지 않았다.
새 packet/프로토콜 변경 없음(76 유지). 실제 서버에는 갱신된 Server 실행 파일이 필요하다.

## 사용자 화면 확인 — 미완료

에이전트는 Client/UI를 실행하거나 조작하지 않았다. Client/Server 종료는 사용자가 확인했다.
공유 Server PC에서 갱신본을 시작한 뒤 이 PC의 Client project Debug x64를 Ctrl+F5로 실행한다.
Lobby → KoukuSaydon → F1 마리오 1~4 진입 → 컷신 종료 → F1 닫기.
←/→ 이동, ↑ 점프 유지. 몬스터 정면 가까이에서 Q를 한 번 눌러 공격 모션과 한 방 처치를 확인한다.
Q는 누르고 놓아야 다음 press가 생기며 cooldown 동안에는 새 공격이 거절된다.
걷기/선회/공격 시 몸체와 망치의 실제 시각 결과는 사용자 확인을 기다린다.
