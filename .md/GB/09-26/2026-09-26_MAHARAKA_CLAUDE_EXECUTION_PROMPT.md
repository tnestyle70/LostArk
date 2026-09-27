# Claude에 전달할 마하라카 복원 실행 지시문

아래 `복사할 지시문` 전체를 새 작업 또는 이어서 진행할 Claude 작업에 전달한다. 게임 수정은 아직 이 문서에서 실행하지 않았다.

## 복사할 지시문

```text
마하라카 원본 복원을 이어서 실제 코드·데이터·리소스·게시까지 구현해라.

작업 저장소:
C:\Users\USER\source\졸업팀폴\LostArk

반드시 처음부터 끝까지 읽을 실행 설계:
.md/GB/09-26/2026-09-26_MAHARAKA_SOURCE_RESTORATION_HANDOFF.md

이 파일과 현재 AGENTS.md/CLAUDE.md/팀 정본을 읽고 G00부터 진행해라.
다른 오래된 worktree에서 빌드하지 마라. 문서의 조사 시점 이후 코드가 바뀌었으면
현재 코드·데이터와 대조하고 실제 차이를 보고한 뒤 현재 사용자 변경을 보존해라.

내 요구:
1. 이미 설치된 모코모코/워터캐논의 원본 물 발사, 얼굴, 지면, 종료 효과를 연결해라.
2. 원본 action의 애니메이션·material parameter·사운드를 같은 시간축으로 재생해라.
3. 워터팡 무대의 정확한 원본 모델·재질·조각을 찾아 연결하고 회전·붕괴·복구를 연결해라.
4. 지형/물/조명/상시 이펙트도 원본 입력으로 보완해라.
5. 베른에서 쓰는 검증된 렌더링 기능을 재사용하되 베른의 좌표·색·조명값을 복사하지 마라.
6. 팀장이 현재 저장한 전역/기존 scene/region quality 값은 보존해라.
   새 마하라카 전용 scene 후보를 분리하고 기존 조율값을 바꿀 때는 diff와 이유로 승인받아라.
7. 기존 정상 기능과 다른 작업자의 선박/카메라/UI 변경을 제거하거나 덮어쓰지 마라.

이전 작업을 오해하지 마라:
- Landscape 16개/32 PNG, NPC 두 모델/재질/idle, map self-motion 축 합성은 이미 설치되었다.
- 물 10행에 water42를 연결한 것과 원본 water 전체 동일성은 다르다.
- 모코모코 Action 원본은 있다. 36 actions/182 stages/20종 particle/111 sound notify가 있다.
- MN_ISTM_00는 MN_Empty_00_SK를 쓰는 controller다. 이것을 꽃 모양 무대 모델로 쓰지 마라.
- Prop는 Prop DB에서 조회한다. 57011 Prop ID 필드는 ints_0x30_0x68[13]이다.
- 테이블에서 못 찾은 것이 원본 삭제의 증거는 아니다.
- 베른은 mapmaterials bakedLighting 21321행/placementLighting 49047행을 가진다.
  현재 마하라카는 둘 다 0이고 공용 Development scene profile을 사용한다.
  이 수치를 목표로 채우지 말고 마하라카 component 원본 입력을 찾아라.

순서:
G00. 실행 checkout/branch/dirty diff/프로세스/설치본 확인 및 보호.
G01. action4225601 원본 연결표 작성. 각 notify를 source-bound/unresolved/gameplay-reference로 분류.
G02. 무대의 Prop/LookInfo/package/trigger/state 연결을 조사하여 모델 정체성 확정.
G03. 현재 Bern/Maharaka 렌더링 소비자와 부족한 입력을 항목별 비교.
G04. 원본 geometry/UV/mip/material/RNM을 대표 지형·메시부터 정확히 연결.
G05. 마하라카 전용 scene/ambient particle/water permutation 입력을 보완.
G06. 첫 행동4225601의 얼굴·물줄기·지면·종료 particle와 socket/material 곡선을 복구.
G07. 같은 행동의 조건별 cast/shot sound, gain/loop/stop 의미를 복구.
G08. 기존 CNpc/EffectPresentation 경로의 clock·socket·준비·cleanup을 고쳐 제품으로 연결.
G09. G02에서 확인된 무대만 기존 WorldSequence/서버 파괴 경로에 연결.
G10. MAHARAKA Loader/Level/replication/cleanup 경로를 닫아 실제 소비되게 연결.
G11. domain 검증→게시→Check→정본 증분 빌드→사용자 수동 확인 준비.
G12. 결과·증거·미완료를 다음 작업자가 그대로 이어받게 기록.

G02에서 특정 원본 연결이 막혀도 독립적인 G03~G08까지 전부 멈추지 마라.
막힌 무대 조각은 미해결로 보존하고 검색 근거를 남겨라. 임의 모델로 대신 완성하지 마라.
단순 단계 종료마다 진행 허락을 되묻지는 마라. 실제 새 권한·충돌·사용자 선택이
필요한 경우에만 구체적인 대상과 이유를 제시해라.

가장 중요한 구현 주의:
- 첫 행동4225601은 6초 Att_Battle_1_01이다. 다른 공격을 임의 반복시키지 마라.
- face=0.5915589929s, jet=2.2925870419s, ground=2.3902139664s,
  finish=5.0718688965s, cast=0s, shot=2.2000000477s.
  실제 저장에는 설계서가 가리키는 원본 JSON의 정확한 값을 사용해라.
- FX_01는 현재 skeleton bone이 아니다. 원본 socket→bone→local transform을 찾아라.
- duration0을 이벤트 삭제/임의1초로 바꾸지 마라.
- PawnMaterialParam/PlayDecalEffect/Color_1/Alpha_1을 빼고 전체 완료라고 하지 마라.
- Water1/Water2 사운드는 LookInfo 조건 선택이다. 둘을 동시에 틀지 마라.
- gameplay Effect notify를 particle로 만들거나 Client 피해 판정으로 구현하지 마라.
- 현재 NpcActionEffectCueDocument와 CNpc에 bone/followBone/duration/clock/preparation
  연결의 보완점이 있다. JSON 작성만으로 해결됐다고 하지 마라.
- Spawn_LevelPlacement의 반환 handle을 occurrence가 보유하고 Seek/Stop_WorldRoot로 정리해라.
  NPC 재질은 Get_Model→Override_SourceCharacterConstants의 clone 경로를 재사용해라.
- npcactioncues version1에 임의 필드를 쓰지 마라. 필요하면 reader/validator/consumer와
  버전·기존 호환 회귀를 같은 변경으로 구현해라.
- 동일 NPC를 기존 server CNpc와 새 WorldObject가 동시에 그리지 않게 해라.
- 얼굴/물/sound는 action occurrence에 속한다. 상시 LEVEL_ACTIVE 환경 효과로 넣지 마라.
- 무대가 발판이면 visual hide와 Server support/collision/navigation이 함께 바뀌어야 한다.
- server에는 asset path/clip name/Client 객체를 보내지 마라. stable typed ID/state를 써라.

작업 방식:
1. 해당 G의 기존 실제 호출자/소유자/정본/실패 소비자를 읽어라.
2. 원본 object/export/offset/ID와 현재 runtime 연결을 확인해라.
3. 기존 도구를 읽고 --help로 CLI를 확인해라. 설계서 명령은 용도별로 나뉘어 있다.
   전체 code block을 한꺼번에 실행하지 마라. 확정되지 않은 package 인자를 추측하지 마라.
4. 후보는 매 작업의 새 out 폴더로 생성해라. 원본/기존 백업/사용자 편집을 덮지 마라.
5. 최소 변경을 구현하고 기존 성공 경로와 실패 rollback을 검사해라.
6. 승인된 candidate만 정본 authoring/Resources에 적용하고 해당 domain publisher를 실행해라.
7. 생성 runtime을 손으로 편집하지 마라. 추가 runtime 소비자가 없으면 끝난 것이 아니다.
8. C++/shader 변경은 현재 정본 Product 빌드로 검증해라. Clean/Rebuild는 기본으로 쓰지 마라.
9. G별 RESULT에 실제 명령/exit code/변경파일/테스트/미검증을 기록해라.

지켜야 할 실행 경계:
- Client/UI를 네가 실행·조작·캡처하거나 visual PASS 판정하지 마라.
- 이번 링크/교체 대상 EXE/DLL의 실제 점유가 막힐 때만 저장·종료를 요청해라.
  데이터 검사/게시에도 무조건 종료를 요구하거나 다른 checkout/팀 서버를 죽이지 마라.
- 원본 추출·headless 검사·빌드·준비까지 하고 정확한 사용자 실행/검사 경로를 안내해라.
- 실제 endpoint/override와 server-host/client 역할을 읽어 실행 profile을 안내해라.
- 존재하지 않는 F1 메뉴가 있다고 말하지 마라. 구현 후 실제 label을 확인해 알려줘라.
- endpoint 변경, git reset/clean, 전체 stage, commit/push/PR/Drive 업로드를 자동으로 하지 마라.
- 서버/클라이언트 자동 검증 계약과 네트워크 프로토콜 테스트는 UI 없이 검증 가능한 범위에서 해라.
- 기본 Product는 Server/protocol harness를 실행하지 않는다. 변경 계약의 focused 검사를 별도로 해라.

금지된 완료 판단:
- 사진이 예뻐 보일 것이라는 추측, texture의 clipped pixel0%, 단순 shader번호 변경.
- 파일 생성, particle graph 추출, JSON parse, build PASS만으로 게임 적용 완료.
- sound extractor exit0만으로 event resolve 성공. EVENT NOT FOUND도 exit0일 수 있다.
- 검색 안 된 한 테이블만 보고 원본 없음/삭제라고 단정.
- 원본 숨김 조각을 전부 visible로 바꾸거나 null texture를 다른 것에 자동 연결.
- 원본 지형 tint를 흰색/저채도로 바꿔 source 동일성이라고 주장.
- 무대 구현이 막혔다고 원통/임의 회전/Valtan 격자 파편으로 source 복원 완료 처리.

각 중간/최종 보고는 다음을 분리해라:
[확인한 원본] [실제 구현] [live 설치] [게시] [빌드/자동검사]
[사용자 수동확인] [미완료와 다음 조사 위치]

처음 답변에서는 G00 실측과 4225601/무대/렌더링 세 흐름의 현재 상태를 짧게 요약하고
바로 순차 작업을 시작해라. 이미 설치된 작업을 처음부터 다시 만들지 마라.
```

## 작업이 끊겼을 때 다시 줄 지시문

```text
마하라카 복원 작업을 이어서 진행해라.
.md/GB/09-26/2026-09-26_MAHARAKA_SOURCE_RESTORATION_HANDOFF.md와
네가 기록한 최신 G별 RESULT를 먼저 읽고 현재 코드/dirty diff와 대조해라.
완료 G를 다시 실행하거나 일회성 install_terrain.py를 재실행하지 마라.
이전 후보 파일 존재만으로 완료를 판단하지 말고 actual runtime 소비자를 확인해라.
가장 앞의 미완료 G부터 진행하되 외부 근거 때문에 막힌 항목은 분리하고
독립적으로 할 수 있는 다음 G는 진행해라. 원본 미확정을 임의 값으로 메우지 마라.
```

## 사용자가 결과를 받아볼 때 물을 항목

1. `4225601에서 clip·얼굴·물줄기·지면·finish·cast/shot·material parameter가 각각 어떤 source/consumer로 연결됐어?`
2. `FX_01는 실제 어느 socket과 bone에 연결됐고 회전 중에도 같은 원점을 따라가?`
3. `워터팡 무대의 실제 원본 object path와 각 조각의 trigger ID는 뭐야? MN_Empty와 구분한 증거는?`
4. `베른의 어떤 기능을 재사용했고, 마하라카 원본 RNM·조명·물 입력 중 아직 미완료는 뭐야? 기존 팀장 옵션은 보존됐어?`
5. `어느 저장소의 어느 exe를 빌드했고, 게시된 runtime과 Resources까지 실제로 교체된 거야? 내가 정확히 어디에서 무엇을 눌러 확인하면 돼?`

이 질문에 source와 함수·데이터·검증 결과를 제시하지 못하는 항목은 완료로 간주하지 않는다.
