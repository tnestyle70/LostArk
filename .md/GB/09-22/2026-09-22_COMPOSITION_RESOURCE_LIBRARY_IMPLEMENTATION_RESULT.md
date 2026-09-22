# Composition Resource Library 구현 결과

## G00. 완료 범위

공용 Animation Library를 기본 표시하고 Character owner에서도 연다. 기존 21개 대상에 실제 설치된
LanceMaster, GunSlinger, Slayer, Artist, DimensionMaster, Warlord, GuardianKnight를 추가했다.
캐릭터·보스·Mario/Clown·기타 모델별 트리에서 clipmap, 현재 skillbinding/PlayerSkill, 동일 body의
Kouku/Saydon actionreference 기획 이름과 원본 clip ID를 함께 검색·선택한다. 이름 자료가 없는
clip도 원본 ID로 전부 남긴다. 단일 클릭은 선택만 하고 명시적 Play Preview/더블클릭으로 실제
CModel preview를 요청한다. Append는 현재 workbench의 target/model guard를 통과해야 한다.
Character의 버튼은 Replace selected Animation / Append Animation이다.

V1 이펙트 inventory는 기존 EffectResourceTree 조직을 stable asset ID로 결합한다. 저장 이름과
관문별 하위 트리를 유지하고 미조직 파일은 실제 asset family의 Character/Boss/World 범주에 둔다.
V2 leaf/group도 같은 대분류 아래 기존 category를 유지하며 ScreenPost track을 보존한다.
빈 Gunslinger/Slayer를 포함하여 7개 클래스 범주를 표시하고 빈 폴더에 No saved effects를 표시한다.
선택된 효과는 기존 Preview/Append 경로를 사용한다. 임의의 다른 클래스 효과로 대체하지 않는다.

7개 playable target이 raid preview 대상으로 들어가므로 기존 동일-target 재선택 경로에
CCharacter 분기를 추가했다. 실제 CCharacter transform과 기존 CAnimationTargetService를 사용한다.

## G01. 설치 데이터 검증

기존 retime_wmodel_from_psa.py의 read_wmodel_animation_sections와 CWModelDecoder의 중복 이름
suffix 규칙으로 28개 대상, 3,077개 runtime clip을 조사했다. 설치된 모든 source 파일을 읽었고
runtime target/source/clip identity 중복은 없다. receipt는 Git 제외
out/CompositionResources20260922/inventory.json이다.

| 대상 | 실제 clip 수 |
|---|---:|
| LanceMaster | 224 |
| GunSlinger | 187 |
| Slayer | 208 |
| Artist | 100 |
| DimensionMaster | 155 |
| Warlord | 194 |
| GuardianKnight | 165 |
| Valtan / Ghost Valtan | 177 / 290 |
| Saydon 00 / 03 / 05 / Large Saydon 06 | 249 / 249 / 258 / 35 |
| Kouku / Mario·Clown | 96 / 28 |
| 나머지 13개 일반 몬스터 | 462 |

7개 클래스 합계는 1,233개다. 기본 clipmap과 runtime 이름이 일치하는 label 수는 순서대로
144/122/148/67/88/114/127개이며 skillbinding과 actionreference 이름은 추가 표시 자료다.
V1 실제 authored 파일은 1,324개, organization 참조는 860개(실제 파일과 일치 857개)다.
조직에 없는 나머지 파일도 노출한다. 현재 GunSlinger/Slayer V1 authored effect는 각각 0개다.
V2 inventory의 258개 leaf와 60개 group을 제외 없이 유지한다. 누락한 원본 particle corpus를
새로 복원한 결과로 보고하지 않는다.

## G02. 소비자·컴파일 경계

Valtan과 Kouku Append는 기존 현재 actor/model/source/revision 검사를 유지한다.
Character Append는 CEffectAuthoringSequencer의 정확한 현재 asset과 CModel clip 검사를 사용한다.
다른 폴더를 탐색하는 것만으로 preview target을 바꾸지 않으며 실제 preview 전환은 기존 dirty lock을 거친다.
scoped git diff --check를 통과했다. 통합 owner가 resources-syntax.cmd로 아래 5개 TU의 실제 Debug /Zs
컴파일을 수행하여 PASS를 확인했다. 로그는 Git 제외 out/CharacterSizeSave20260922/resources-syntax.log다.
그 뒤 V1 effect.esther. 문자열 category 항목 하나를 World/Esther로 추가했다.
Client 또는 UI는 실행하지 않았다. 실제 창 크기·트리 조작·GPU preview 화면은 사용자 확인 대상이다.
신규 C++ 파일은 없고 project/filter 등록 변경은 없다.

변경 TU는 Animation_Tool_DocumentIo.cpp, CharacterPreviewPanel.cpp,
CompositionResourceTree.cpp, EffectAuthoringSequencer_Resources.cpp, SequencerTool.cpp이다.
공용 header는 CompositionAnimationResource.h, CompositionResourceTree.h이며 SequencerTool.h의
cache/default 표시 멤버는 Character Sequencer 작업과 조정하여 반영했다.
