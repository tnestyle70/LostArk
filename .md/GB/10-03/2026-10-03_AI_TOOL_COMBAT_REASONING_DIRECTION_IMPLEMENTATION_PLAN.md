# AI Tool 전투 판단·상대 스킬 인지 확장 방향 구현 계획

작성일: 2026-10-03

## G00. 목표와 이번 작업의 종료 범위

AI가 어떤 상대 행동을 관찰했고, 어떤 행동을 후보로 삼았으며, 왜 이동·회피·공격을
선택했는지 확인할 수 있는 도구로 발전시킨다. 먼저 기존 규칙의 판단 근거와 실제 Server
승인을 연결하고, 그 기록으로 규칙 개선과 향후 학습 정책을 같은 조건에서 평가한다.

이번 요청의 산출물은 **방향성 구현 계획서**다. AI 코드, 저작 데이터, packet, 학습기,
모델 파일과 런타임 정책은 변경하지 않는다. 아래 G02~G08은 후속 구현 범위이며 현재 완료된
기능으로 표시하지 않는다. H/CPP 전체 코드를 작성하는 디테일 계획서는 이번 범위가 아니다.

첫 구현 대상으로는 실제 상대 플레이어 스킬을 소비하는 콜로세움 용병의 **관찰·판단 기록**을
권한다. 기존 판단을 바꾸기 전에 현재 선택을 재현할 수 있어야 개선 효과를 구분할 수 있다.
워터팡은 별도 게임 규칙과 tuning 계약을 유지하며 후속 소비자로 확장한다.

## G01. 현재 코드와 도구의 실제 연결

### 현재 구현 범위

| 대상 | 현재 입력과 선택 | 실제 소비자·저장 경계 | 아직 없는 연결 |
|---|---|---|---|
| Waterpang AI Tool | AI 수, 판단·이동·스킬 간격, 대상 거리, 이동·공격 확률, 밀침 수치 편집 | `Client/Private/MaharakaAITool.cpp` → typed command sink → `CGameRoom::Handle_MaharakaAITuning`; Server만 `Data/AI/MaharakaWaterpangAI.json` 저장 | 개별 AI가 왜 특정 목표·스킬을 골랐는지 보여 주는 decision trace, 학습기 |
| 워터팡 실행 | 가까운 유효 대상, tick/actor 기반 난수, 이동·공격 확률, 물총 슬롯 순회 | `Server/Private/GameRoom_MaharakaAI.cpp`의 update → `Execute_PlayerMove` / `Try_StartMaharakaWaterGunSkill` | 상대 스킬의 향후 적중 구간을 이용한 일반 전투 판단, 학습 정책 |
| 콜로세움 용병 | 근접 적, 현재 실행 중 적 스킬·콤보 단계·경과 시간과 hit shape, 직업별 스킬 순서 | `Server/Private/GameRoom_Colosseum.cpp`의 `Update_Colosseum` → 기존 이동·스킬 실행기 | 후보별 관측·점수·거부 이유를 Client 도구에 전달하는 계약, 학습 정책 |
| Guide AI Combat Detail | Follow/Evade/Combat 가중치와 생존 우선, Server 점수·위험·HP·판단 이유 표시 | `Client/Private/GuideAITool_Combat.cpp`, `Server/Private/GameRoom_Guide.cpp`; Guide 정본은 `Data/Guide`와 독립 publisher | 현재 콜로세움/워터팡 판단을 이 화면에 전달하는 공통 계약 |

워터팡 설정은 GET/APPLY/SAVE와 expected revision 비교를 사용한다. Apply는 현재 Server 설정,
Save + Apply는 Server 원본 저장까지 처리한다. 충돌이나 실패 때 draft와 이전 적용 상태를
보존하는 현재 계약을 유지한다. 저장값의 기본 `decisionTicks=9`는 30Hz에서 0.3초이며,
`moveProbability=0.8`, `aggression=0.8`은 작성된 확률값이다. 이 값이 실행 중 자동 학습된다는
근거는 없다. 기준은 `MaharakaAITool.cpp:79`, `GameRoom_MaharakaAI.cpp:92`, 같은 파일 `:284`다.

콜로세움의 일반 행동 결정은 0.2초 간격이며 COMBO 입력은 별도 fixed tick 경로다.
위험 평가는 적의 현재 `iCurrentSkillId`, `iComboStage`, `fActionElapsedSeconds`로 선택한
hit 목록을 사용한다. 시작이 앞으로 400ms 이내이고 종료 후 100ms 허용 구간 안인 shape를
현재 후보 위치와 대조한다. 위험이 있으면 반경 3m의 12방향 후보에서 navigation·충돌·LOS와
더 낮은 위험을 확인한다. 근거는 `GameRoom_Colosseum.cpp:622`, `:667`, `:686`이다.
이는 이미 실행 중인 스킬의 알려진 hit schedule을 읽는 규칙이다. 상대의 다음 입력이나
숨은 쿨다운을 추론하는 모델, 발사체 전체 미래 궤적의 완전한 예측으로 설명하지 않는다.

같은 파일 `:729` 이후의 공격은 승인 가능한 스킬 순회이며 차원술사는 게시 Guide 콤보의
고정 순서를 사용한다. `pReason`은 현재 Server 상태와 계약 검사에서 쓰이고 콜로세움
Client 판단 화면으로 전달되지 않는다. Guide의 가중치 선택기를 콜로세움이 공통 사용한다고
해석하면 안 된다.

### Guide의 현재 제품 범위

현재 Guide는 Bern 안내 NPC다. `GameRoom_Guide.cpp:325`의 `Update_Guides`는 Bern 이외의
월드에서 반환한다. 전투 가중치와 도움 콤보 코드는 남아 있고 synthetic monster를 넣는
계약 검사도 있지만, 현재 Bern 저작 placement에는 MONSTER/BOSS가 없다. 그러므로 Combat
Detail의 존재를 레이드 전투 동행 완료 증거로 사용하지 않는다. 레이드 동행을 부활시키는
일도 이 계획에 포함하지 않는다.

기존 설계와 비교할 때는 다음 문서의 현재 정정 구간을 함께 읽는다.

- `.md/GB/09-27/2026-09-27_GUIDE_AI_TOOL_IMPLEMENTATION_PLAN.md`
- `.md/GB/09-27/2026-09-27_GUIDE_AI_TOOL_IMPLEMENTATION_RESULT.md`의 G06 이후. 앞선 동행 기획은 폐기 이력이다.
- `.md/GB/10-01/2026-10-01_COLOSSEUM_PVP_MERCENARY_IMPLEMENTATION_PLAN.md`
- `.md/GB/10-01/2026-10-01_COLOSSEUM_PVP_MERCENARY_RESULT.md`의 직업별 최종 선택 계약과 후속 정정.
- `AGENTS.md`의 Bern Guide, Server 권위, 워터팡 AI 계약과 `CLAUDE.md`의 해당 도구 사용법.

## G02. 상대 스킬 인지의 입력 계약

### 관찰 상태와 정답 상태를 구분한다

Server가 모든 actor 상태를 소유해도 AI가 그 전부를 사용할 필요는 없다. 후속 정책의 관찰은
상대가 현재 드러낸 행동과 자기 상태를 중심으로 제한한다. 현재 규칙과 비교할 때도 두 정책이
같은 관찰 집합과 반응 지연을 쓰도록 한다. 제한을 추가하면 행동이 달라지므로 관찰 기록 G04와
정책 변경 G05를 별도 변경 단위로 검증한다.

| 입력 | 단위·정본 | 정책이 읽는 범위 |
|---|---|---|
| 자기 상태 | Server tick, HP 비율, 위치 m, action/CC, 자신의 스킬 쿨다운·자원 | 실제 실행기의 현재 상태. Client 애니메이션으로 역산하지 않는다. |
| 상대 현재 행동 | entity ID, skill ID, combo stage, action 시작 tick/경과 ms, 위치·방향 | 인지 시점에 공개된 행동. 취소·종료·대상 사라짐을 다음 판단에서 반영한다. |
| 스킬 위험 | active gameplay catalog revision, hit shape·시작/종료 ms, 후보 위치의 겹침 | 현재 콜로세움에서 확인한 소비 범위부터 시작한다. 미지원 projectile 예측은 지원된 것처럼 채우지 않는다. |
| 지형과 이동 | 현재 navigation/collision revision, 경로·LOS·후보 위치 | 실제 Server 질의 결과를 사용한다. 화면상의 빈 공간을 이동 가능 판정으로 쓰지 않는다. |
| 기억 | 마지막으로 관찰한 스킬, 관찰 tick, 추정 잔여 시간, 근거의 종류 | 상대 쿨다운을 추정한다면 추정으로 명시한다. Server의 실제 숨은 잔여 쿨다운을 정답 입력으로 몰래 넣지 않는다. |

저장용 identity는 world/match, actor의 stable identity, skill ID와 revision이다. 포인터와
vector index는 런타임 참조일 뿐 기록을 다시 읽는 식별자로 쓰지 않는다. 숫자 입력에는 단위와
유효 여부를 둔다. 관찰이 오래됐거나 데이터가 없으면 unknown 상태로 유지한다.

상대 입력을 받기 전의 미래 command, 아직 시작하지 않은 패턴 내부 정답, 학습용 reward와
평가용 정답 상태는 행동 선택 입력에 섞지 않는다. Debug 관찰창은 정책이 본 값과 검사기가
알고 있는 실제 값을 구별해 표시한다. 이 분리는 추후 반응 속도·난이도를 공정하게 조절하는
근거가 된다.

## G03. 행동 후보와 선택·승인 흐름

다음 흐름은 새 전투 실행기를 만드는 제안이 아니다. 기존 `CGameRoom`의 actor 상태와
이동·스킬·피격 실행 경계 앞에 관찰 및 후보 선택을 정리하는 후속 구조다.

```text
Server 판단 tick의 관찰
→ 현재 action/CC·쿨다운·자원·거리·navigation에 따른 후보 검사
→ 대기 / 접근 / 위치 유지 / 회피 / 대상·스킬 선택
→ 규칙별 점수와 선택 이유 기록
→ 기존 Execute_PlayerMove / Execute_PlayerSkill 등 해당 domain 실행기
→ 승인·buffer·거부 및 실제 상태 전이 기록
→ 기존 snapshot 표현 + 별도 Debug 진단 응답
```

후보 선택의 score가 높다는 이유로 이미 실행 중인 스킬을 강제로 끝내거나 쿨다운을 초기화하지
않는다. 후보의 사전 검사와 최종 승인은 별도다. 관찰 이후 상태가 달라질 수 있으므로 마지막
판정은 기존 실행기가 수행한다. COMBO buffer는 즉시 스킬 승인과 구분한다.

규칙 단계에서는 공격 이득, 적중 가능성, 예상 노출 시간, 위험 감소, 접근 거리 같은 작은
점수 항목을 기록한다. 처음부터 모든 수치를 하나의 불투명한 점수로 합치지 않고 각 항목의
실제 입력과 가중치를 보여 준다. 점수는 예측이며 실제 damage/CC 결과와 분리한다.
상황 변화가 작은데 이동·공격을 매번 뒤집는 현상은 minimum hold와 switch margin으로
다룰 수 있다. Guide의 기존 구조를 참고하되, 콜로세움 적용값과 동작은 별도 검증한다.

판단 이유는 실제 통과·실패한 조건에서 만든다. 예를 들어 “상대 38000의 다음 hit까지
180ms, 현재 위치 위험 1, 선택 위치 위험 0, 회피 승인”처럼 기록한다. 이 숫자는 설명
형식의 예시이며 현재 제품에서 측정한 값이 아니다. 자연어가 실제 실행 결과보다 앞서
공격 성공이나 회피 성공을 단정하면 안 된다.

## G04. 도구에서 먼저 보여 줄 정보

첫 변경은 현재 행동을 바꾸지 않고 콜로세움 한 AI의 결정 기록을 관찰하는 기능이다.
Guide Combat Detail의 읽기 전용 Server 영역을 참고하되 Bern Guide 전용 상태를
콜로세움 상태처럼 재사용하지 않는다.

| 화면 영역 | 표시할 내용 | 수명과 실패 처리 |
|---|---|---|
| 대상 선택 | 현재 world/match와 AI entity, class, 적용 규칙 revision | 퇴장·재생성 시 오래된 actor 선택 해제 |
| 관찰 | 정책이 본 상대 스킬·경과 시간·위험 shape·관찰 tick | unknown/stale를 표시하고 이전 기록을 현재 값으로 재사용하지 않음 |
| 행동 비교 | 후보별 점수 항목, 제외 조건, 선택 항목 | 해당 decision ID에 고정된 읽기 전용 기록 |
| 실행 결과 | command sequence, 승인/buffer/거부, 실제 action 변경 | 판단과 승인 결과의 identity를 대조 |
| 설정 | Server 적용값, local draft, 기준 revision | 기존 Waterpang CAS와 같은 실패 시 보존 원칙. 새 domain 저장 계약은 별도로 연결 |
| 평가 | 정책 종류, 버전, 데이터·평가 보고서 식별자 | 규칙, 학습 중 후보, 검증 완료 모델을 구분 |

진단은 선택한 actor의 bounded ring buffer와 명시적 구독/해제로 제한한다. 화면을 닫아도
전투는 계속되며, ring overflow는 누락 개수로 표시한다. 전체 actor 상태를 매 프레임 JSON으로
직렬화하거나 Client가 Server 판단을 다시 계산하지 않는다. 기록 보존량, 메시지 최대 크기와
요청 빈도 상한을 구현 전에 packet 크기·tick 비용과 함께 정한다.

도구 명령은 `Client/Public/PlayerCommandSink.h`의 typed 경계와 기존 network sink를 통과한다.
ImGui에서 socket이나 Server 파일을 직접 조작하지 않는다. GET 응답·구독 데이터는 현재
session/world의 권한과 actor identity를 검증하고, 연결 종료 시 구독과 pending 상태를 정리한다.

## G05. 규칙 개선을 먼저 검증하는 구현 순서

| 변경 단위 | 실제 작업 | 종료 증거 |
|---|---|---|
| 관찰 | G02~G04의 최소 decision 기록과 한 actor 조회 UI | 기록을 켜기 전후 동일 입력의 command/action 결과가 일치하고, 기록 비용과 누락이 측정됨 |
| 상대 스킬 판독 | 현재 hit schedule 관찰을 명시적인 입력으로 정리 | 취소·combo stage·반복 hit·경계 시각·stale actor fixture에서 입력이 일치 |
| 후보 선택 | 기존 순회 정책을 baseline으로 보존하고 제한된 회피/공격 점수를 비교 | 고정 seed·동일 class matchup에서 baseline과 후보의 차이를 설명 가능 |
| 데이터와 적용 | 필요한 tuning만 domain 정본과 Server 적용 경계에 연결 | invalid 값·revision 충돌·저장 실패 시 기존 설정과 draft 보존 |
| 학습 확장 준비 | 동일 관찰·행동·결과를 재사용하는 별도 평가 실행기 | 제품 실행기와 동일 action admission·damage·CC·navigation 결과, 학습 전 baseline 재현 |

첫 실험은 제한된 class 조합의 1대1 평가 fixture부터 시작한다. 이는 제품 매칭 인원이나
입장 흐름을 바꾸는 제안이 아니다. 이후 class 조합과 다인전을 넓히되, 전투 판정은
기존 private room과 실행기에서 수행한다. 워터팡의 경기 규칙과 확률 tuning을 콜로세움용
가중치 JSON에 합치지 않는다.

## G06. 학습으로 확장할 때의 구분

현재 조사한 AI 경로에는 optimizer, reward 기반 parameter update, 학습 checkpoint를
읽는 policy inference 연결이 없다. 규칙을 조정하거나 JSON의 확률을 바꾸는 작업은
**규칙 조정**으로 부른다. 경기 결과를 저장하는 것만으로 학습이 일어나지는 않는다.

| 방식 | 데이터와 변경 대상 | 도구에서 사용할 이름 |
|---|---|---|
| 현재 규칙·점수 선택 | 사람이 작성한 순서·조건·가중치 | 규칙 정책 / 수동 조정 |
| 별도 시뮬레이션에서 반복 학습 | 학습 중 Server 환경과 새로 상호작용하며 parameter 갱신 | 시뮬레이션 RL 학습 |
| 고정 로그로 학습 | 이미 수집한 dataset만 사용하고 학습 중 새 환경 경험을 수집하지 않음 | Offline RL 학습 |
| 학습 산출물 실행 | 고정된 검증 완료 parameter로 행동 추론, 제품에서 갱신하지 않음 | 학습 정책 추론 |

Offline RL은 고정된 기존 데이터로 학습하는 방법이다. 별도 PC에서 게임 화면 없이
PPO를 돌려도 새 episode를 수집하면서 학습하면 방법론상 online RL에 해당한다.
이 용어 구분은 [Offline RL 원 논문·개관](https://arxiv.org/abs/2005.01643)과
[PPO 원 논문](https://arxiv.org/abs/1707.06347)의 학습 절차에 따른다.

현재 저장소에 제안하는 순서는 규칙 baseline과 재현 평가를 먼저 만들고, 그다음 별도
Server simulation을 사용하는 작은 학습 실험이다. PPO는 환경 상호작용 기반 baseline
후보로 검토할 수 있으나 이 프로젝트에서 성능이 확인된 선택은 아니다. 먼저 관찰·행동
공간과 샘플 수집 비용을 측정하고 알고리즘을 선택한다. 충분한 로그가 없는 상태에서
Offline RL부터 채택하는 것은 권하지 않는다. 이 순서는 현재 코드 구조에 근거한 설계 판단이다.

학습 환경은 실제 `CGameRoom`과 skill executor를 호출한다. Python에 damage·CC·쿨다운·
navigation을 다시 구현해 제품과 다른 전투를 학습시키지 않는다. 외부 trainer를 붙이더라도
reset/step adapter가 simulation을 구동하고, 모델이 내는 결과는 같은 유효 행동 계약을
따른다. `reset(seed)`와 observation/action space, step의 reward·terminated·truncated
구분은 [Gymnasium 공식 Env 계약](https://gymnasium.farama.org/api/env/)을 참고한다.
경기 규칙에 따른 실제 종료와 실험 시간 제한·인프라 중단은 로그에서도 분리한다.

Offline RL을 검토할 때의 최소 episode 기록은 관찰, valid action mask, 선택 행동, 승인 결과,
실제 다음 관찰, reward 항목, 종료 이유, seed, simulation/data/policy revision이다.
한 순회 정책이 방문한 상태만으로 다른 행동의 품질까지 확인됐다고 주장하지 않는다.
dataset의 class·거리·HP·CC·지형·승패 분포와 행동 분포를 먼저 보고한다.

reward는 실제 피해·생존·승리 등 목적을 분리해서 기록한다. 대기·도주 반복, 무효 command
반복, 특정 상대만 이기는 정책으로 점수만 높아지는지 검사한다. 추후 자기 대전도 고정 baseline과
분리된 평가 상대를 유지한다. 제품 Server 안에서 사용자와 경기하며 모델 parameter를 자동
업데이트하는 기능은 이 계획의 범위에 넣지 않는다.

## G07. 평가와 제품 반영 판정

평가는 학습에 사용하지 않은 고정 시나리오와 여러 seed에서 진행한다. 별도 평가 환경과
여러 실행 결과를 쓰라는 원칙은 [Stable-Baselines3 공식 평가 지침](https://stable-baselines3.readthedocs.io/en/master/guide/rl_tips.html)을 참고한다.
아래 항목과 통과 조건은 이 프로젝트의 후속 검증 설계이며 현재 측정 결과가 아니다.

| 평가 축 | 기록할 증거 | 통과 판단 |
|---|---|---|
| 행동 적법성 | 승인·buffer·거부 수, action/CC·쿨다운·resource 위반 fixture | 기존 실행기를 우회하는 상태 변경 0, 거부를 성공으로 표시한 사례 0 |
| 판단 설명 | 관찰/decision/command identity와 실제 결과 대응 | 동일 기록의 이유·수치가 재현되고 누락은 명시됨 |
| 전투 성능 | matchup별 승률, damage, 피격, 생존, 유효 공격·회피, idle 비율 | baseline과 같은 입력 분포로 비교. 평균만으로 특정 class 퇴화를 숨기지 않음 |
| 공정성 | 반응 지연, 숨은 상태 접근, 관측되지 않은 스킬에 대한 반응 | 같은 관찰 정책과 난이도 조건. 정답 누출을 성능으로 계산하지 않음 |
| 안정성 | 취소·사망·퇴장·재생성·room 종료, 실패한 설정/model 로드 | 이전 유효 정책 유지 또는 명시적인 기존 규칙 fallback. actor/state 잔류 없음 |
| 비용 | 동일 빌드·장비에서 actor 수별 판단 CPU p50/p95/max, tick 전체 시간, allocation·기록 크기 | 먼저 baseline과 tick 잔여 여유를 측정해 수치 budget 확정. FPS 상승이나 비용 0을 선약하지 않음 |
| 일반화 | 학습 제외 seed·class·거리·지형, 고정 규칙과 별도 상대 | 반복 실행 분산과 비교 불확실성을 함께 보고하고, 특정 학습 상대의 승률만 제출하지 않음 |

학습 완료 표시는 training run ID, 실제 parameter update 기록, dataset/환경 버전,
artifact hash가 있는 경우에만 사용한다. 검증 완료 표시는 별도 평가 보고서와 통과 범위가
연결된 경우에만 사용한다. 높은 reward, checkpoint 파일 존재, 규칙 변경만으로 두 상태를
대신하지 않는다.

제품에 모델을 적용하는 후속 단계에서는 모델 입력 schema, 허용 action, feature normalization,
모델 hash와 적용 policy revision을 함께 검증한다. candidate 로드와 검증이 끝난 뒤 경계 tick에
교체하고 실패하면 이전 정책을 보존한다. 모델 학습, 파일 게시, Server 적용, Client 진단 표시를
각각 기록한다. 사용자 체감 난이도와 실제 화면 확인은 수치 평가와 별도의 사용자 판정이다.

## G08. 후속 파일 범위와 검증

### 기존 파일의 책임

| 파일 | 후속 변경의 시작점 |
|---|---|
| `Server/Private/GameRoom_Colosseum.cpp`, `Server/Public/GameRoom.h` | 현재 선택 지점의 bounded decision 기록과 해당 room 수명 정리 |
| `Server/Private/GameRoom_MaharakaAI.cpp` | 후속 워터팡 관찰 adapter. 기존 GET/APPLY/SAVE와 경기 상태 유지 |
| `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp` | 이미 승인된 skill/hit 정본의 읽기 경계. AI만의 별도 skill catalog를 만들지 않음 |
| `Server/Private/PlayerSkillSystem.cpp`, `Server/Public/ColosseumCombatPolicy.h` | 실제 스킬·적중 승인 경계. 관찰 기능 때문에 피해 규칙을 바꾸지 않음 |
| `Shared/Public/Network/PacketMessages.h`, `Shared/Private/Network/PacketMessages.cpp` | Client 진단이 필요할 때만 bounded typed request/response 및 codec 확장 |
| `Client/Public/PlayerCommandSink.h`, `Client/Private/NetworkPlayerCommandSink.cpp`, `Client/Private/ClientReplication.cpp` | 진단 요청과 응답의 typed 연결, session/world 수명과 pending 정리 |
| `Client/Private/MaharakaAITool.cpp`, `Client/Private/GuideAITool_Combat.cpp` | 기존 Server/draft 구분과 관찰 표현 참고. 서로 다른 domain 상태를 임의 혼용하지 않음 |
| `Server/Private/ServerGameplayContractTests_ColosseumMatch.cpp`, `ServerGameplayContractTests_ColosseumCombat.cpp`, `ServerGameplayContractTests_MaharakaAI.cpp` | 실제 room/executor fixture에서 기존 동작과 관찰 parity·실패 격리 확인 |
| `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` | wire 변경이 있을 때 codec·상한·invalid 입력·구버전 거절 검사 |

이번 문서는 새 C++ 파일을 추가하지 않는다. 후속 구현에서 공통 관찰 타입이나 별도 도구 파일을
분리한다면 실제 owner를 결정한 뒤 해당 Client/Server/Shared의 `.vcxproj`와 `.vcxproj.filters`에
필요한 파일만 등록하고 물리 폴더 구조를 유지한다. trainer 의존성이나 모델 runtime 설치는
G06의 구체 구현을 확정하기 전에는 수행하지 않는다.

### 후속 검증 순서

관찰 기능은 변경 TU와 연결 소비자의 최소 컴파일, 필요한 JSON/XML parse, scoped
`git diff --check`부터 확인한다. Server 변경이 있으면 기존 아래 entry를 변경한 소비 범위에
맞게 실행한다. 이 문서 작성에서 아래 계약 검사를 새로 실행한 것으로 기록하지 않는다.

```powershell
& .\Server\Bin\Debug\Server.exe --colosseum-match-contract-test
& .\Server\Bin\Debug\Server.exe --colosseum-combat-contract-test
& .\Server\Bin\Debug\Server.exe --maharaka-ai-contract-test
```

wire를 확장한 경우에는 protocol 정의와 Writer/Reader, NetworkProtocolHarness를 같은 변경
단위로 검증하고 Client/Server를 같은 계약으로 빌드한다. 실제 executable 경로와 build receipt를
확인해 이전 바이너리의 PASS를 새 소스의 증거로 사용하지 않는다. Guide를 변경하지 않았다면
Guide 전체 하네스를 이 문서 작업의 완료 조건으로 끌어오지 않는다.

문서 단계에서 완료한 일은 현재 코드·데이터·기존 PLAN/RESULT 대조, 공식 자료 확인과
후속 범위 정의다. AI 정책 구현·학습·새 모델 평가·Client/UI 실행은 수행하지 않았다.
이 방향 문서만으로 기존 public 계약이나 팀 사용서의 현재 동작을 바꾸지 않는다.
