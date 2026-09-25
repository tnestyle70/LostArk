# 쿠크 체력 기믹 시험과 Effect·Sound 타임라인 편집

## G00. 실측과 구현 경계

창술사 ALT_V는 skill 34630 / damage.player.34630이다. Retail override의 공격 계수,
피해 편차와 치명타를 거친 뒤 여섯 HP 피해 타격으로 실행된다. 계수만 변경하면 보스별
최대 HP·최대 줄 수가 달라 정확한 35줄 시험값을 보장할 수 없다.

Effect occurrence에는 Sound의 source-in에 해당하는 필드가 없다. 타임라인 앞 edge는
시작·수명만 바꾸므로 asset 내부의 공백이 다시 재생된다. Sound 다중 선택은 가능하지만
drag 선택 보존·일괄 이동·영구 그룹 저장은 현재 Effect/Collider 위주로 제한돼 있다.

이번 변경은 기존 Server damage, Composition document, PresentationPlayer와 Workbench
경로를 확장한다. 별도 runtime manager나 Client 피해 판정을 만들지 않는다. 사용자가 편집
중인 Composition과 이미 변경된 게시 데이터, 앞선 Play 분리 소스를 보존한다.

## G01. 보스 체력 줄 기준 피해

Retail damage profile의 optional `bossHealthBarDamage`는 기본 0이며, 양수일 때 해당
스킬이 보스에 가하는 한 cast의 총 HP 피해를 대상의 최대 HP·최대 줄 수로 계산한다.
창술사 damage.player.34630의 시험값은 35다. 총량은 정수 HP로 올림하고 기존 다단
타격에 누적 차분으로 나눠 반올림 손실을 막는다. 적중·무적·실드·사망은 기존 판정을
따르며, 확정 보스 HP 피해에는 공격 배율·피해 편차·치명타·방어력을 재적용하지 않는다.
비보스와 기본값 0인 profile은 기존 계산을 유지한다.

`Publish-GameplayBalance.ps1`와 Server `GameplayCatalog`가 optional 행을 함께 읽고,
`PlayerSkillSystem`에서 기존 hit payload를 구성한다. `ServerCombatHitRuntime`은 이미
계산된 피해 여부를 기존 보스 피해 소비자에 전달한다. F1 Balance editor의 저장/표시
계약도 같은 optional 값으로 연결한다. Shared 네트워크 계약 변경은 없다.

## G02. Effect source-in

`KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE`에 기본 0인
`iEffectSourceStartMs`를 추가하고 JSON `effectSourceStartMs`로 왕복한다. Effect만
사용하며 source-in은 source duration보다 작아야 한다. Codec과 projector가 잘못된
kind·범위 입력을 거부하고 기존 문서를 보존한다.

Workbench의 앞 edge trim은 잘린 길이를 source-in에 더하고 수명을 줄인다. body 이동은
시작 시각만 이동한다. 상세 Source In은 박스 시작 시각을 유지하면서 원본 앞부분을
건너뛸 수 있다. asset의 공용 emitter delay는 변경하지 않는다.

PresentationPlayer는 `sourceIn + age * rate`로 재생한다. Fit은 source-in 뒤의 남은
구간을 박스 수명에 맞춘다. 유한 Loop는 남은 source 구간을 반복하고 원본 무한 emitter는
source-in을 포함한 emission 종료 시각을 사용한다. follow provider는 source 시간과
Pattern 시간을 역변환해 attachment가 잘린 길이만큼 미래 위치를 읽지 않게 한다.
local preview·게시 패턴·임시 Server draft 모두 기존 동일 표현 소비자를 사용한다.

## G03. Sound 그룹과 일괄 시간 이동

Workbench의 선택 보존과 `Prepare_PresentationTimelineMove`를 Sound 및 Effect+Sound
혼합 선택에 확장한다. 선택된 박스는 동일 delta로 이동하고 source-in·수명·fade·volume은
보존한다. Collider는 기존 판정 구간의 연결된 박스 이동 계약을 유지한다.

Sound 영구 그룹은 같은 kind의 박스 두 개 이상을 묶는다. C++ codec과 Python projector에
동일 허용 조건을 연결하고 기존 SHOWTIME의 Effect+Sound 특례는 유지한다. generation이
변한 drag와 유효하지 않은 입력은 전체 편집을 거절한다.

## G04. 사용자 공통 피해 Trigger 연결

사용자가 만든 Logic507 `트리거_대미지_넉백`과 Logic508 `트리거_대미지`는 현재 TRIGGER만
있고 triggerKind가 없어 Collider Apply가 거절된다. 두 정의를 ENTER_AREA로 완성하고
optional `colliderDamageContactRole`의 KNOCKBACK/DAMAGE 역할을 명시한다. Workbench는
표시 이름을 identity로 사용하지 않고 이 역할과 반복 설정이 일치하는 순수 접촉 정의를
재사용한다. 없는 경우 같은 역할의 정의를 생성한다.

일반 피해 window에 재사용된 잡기 Trigger는 같은 window ID·시간·geometry·피해 값을
유지하면서 해당 공통 역할로 다시 연결한다. hold, 비피해 Success, Fail/Timeout 등 실제
기믹 연결이 있는 window는 유지한다. 특히 P17의 실제 잡기 접촉과 ATTACHMENT_HOLD는
교체하지 않는다. 현재 확인한 일반 재사용 대상은 P47.logic.4와 P47.logic.7이다.

## G05. 데이터 적용값

사용자는 창술사 ALT_V만 35줄 시험값으로 확정했다. 트럼펫 카드 장판은 처음 언급한
3초 대신 첫 원형 장판 생성 시점까지 자르도록 최종 확정했다. asset의 최초 floor birth
설정은 4184ms지만 실제 native playback은 4200ms까지 비어 있고 4201ms에서 처음
floor 입자 26개와 양수 alpha 11개를 생성한다. P47의 두 트럼펫 장판 occurrence는
배치 시작 시각을 유지하고 effectSourceStartMs를 4201로 설정하며 제거한 source
길이만큼 수명을 줄인다. 이는 CPU 재생 표본 검증이며 최종 화면 확인은 사용자가 한다.
최종 반영 직전에 최신 저장본의 stable ID와 이 필드를 다시 읽어 병합한다.

## G06. 검증과 설치

기존 native Server fixture에서 35줄 총량과 다단 분배, 보스별 줄 수, 비보스 보존 및
적중·무적 경계를 확인한다. Composition native fixture와 Python projector에서 source-in
왕복·잘못된 값 거부·source 시간과 그룹 이동/저장 보존을 검증한다.

기존 제품 C++ 파일을 수정하므로 새 제품 파일의 프로젝트 등록은 없다. harness의 새
dispatch는 기존 main에 연결한다. UTF-8/CRLF를 유지하고 관련 컴파일·XML/JSON parse와
diff-check를 확인한다. 실행 중 Client/Server를 자동 종료하거나 UI를 조작하지 않는다.
최종 데이터 교체는 최신 저장본과 stable ID를 다시 확인하고 writer lock·백업·원자 교체로
적용한다. 빌드 산출물 교체, 게시, 실행 중 메모리 반영과 사용자 화면 확인을 구분해 기록한다.
