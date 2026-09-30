# Guardian ALT V 브레스 타격 시점 결과

## G00. 원인과 실제 연결

skill49420 / GuardianKnight ALT_V / ddk_sk_super_sanctumofembereth의Server 타격은300ms였다. PlayerSkills뿐 아니라 실제 PlayerSkillSystem이우선 소비하는HitShapes의DAMAGE/COUNTER/STAGGER도모두300ms였다. reference skilltiming은hits가비어있으며이300ms는fallback project timing이다.

동일clip authored EFFECT는3790ms에projectile494200을시작한다. product sequence의guardian.altv.source.effect는0ms부터해당Effect를재생한다. actual authored fx-03 FireBreath01 12개와fx-04 LocalFireBreath01 2개는3.78999996초부터활성화하며zero-time burst가존재한다. camera.guardianknight.skillcam_dragonknight_02.zoomout은1500~3800ms이며CEffectRecoveryCamera::Sample은끝3800ms를제외한다. 이후기존follow camera로복귀한다.

## G01. 변경한 정본

- Data/Balance/PlayerSkills.json: skill49420.hitTimeMs를3800ms로변경했다.
- Data/Animation/HitShapes/GuardianKnight.hitshapes.json: skill49420.stage0.caster.hit1의damage/counter/stagger stable collider3개timeMs를3800ms로변경했다.
- Data/Animation/Authored/GuardianKnight/GuardianKnight.animevents: 동일clip HIT startms/endms만3800으로맞췄다.2240행header와다른notify는보존했다.
- Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json: 해당hitTimeMs행만PROJECT_TUNED source breath3790ms→camera return3800ms근거로변경했다.
- Server/Private/ServerGameplayContractTests_SkillStages.cpp: 기존Guardian 실제Try_Start→Update fixture에3800ms경계전후검증을추가했다.

피해량·범위·반복·action4833ms·Retail overrides·effect/camera는변경하지않았다.3790ms와3800ms는30Hz에서114번째tick에속한다. source 카메라정본을Server가로드하는새결합은추가하지않았으며, 현재사용자의연출에맞춘저장값조정이다.

최신byte재확인→대상필드만후보생성→JSON의미상변경범위검사→같은폴더임시파일→ReplaceFileW백업/원자교체를수행했다.기존변경·렌더링튜닝은보존했다.영수증은out/GuardianAltVTiming20260930/apply-receipt.json이다.

## G02. 검증과 게시 경계

JSON3개parse, hitshape세결과일치, 실제Effect/sequence timing연결, animevents2240행과HIT한행을확인했다.수치영수증은out/GuardianAltVTiming20260930/consistency-receipt.json이며unchanged effect/sequence SHA도기록했다.

python -m unittest discover -s Tools/EffectPipeline -p test_guardian_owner_control_projection.py:9PASS.
python -m unittest discover -s Tools/EffectPipeline -p test_player_skill_catalog_combo_timings.py:구조검사2PASS,기존자동combo class목록1FAIL와Client정본의g_Skills commit 문자열을찾는fixture1ERROR.현재source는기존expected와다르며이번단일ACTIVE timing필드와무관하다.무관test는수정하지않았다.

git diff --check는담당data/C++에서PASS다.새Server회귀는3.799초까지HP/event불변,3.801초HP차감과event1회일치,이후중복없음을검사한다.최종C++컴파일과--skill-stages-contract-test실행결과는root통합검증이소유한다.

Gameplay publisher는root최종통합단계에맡겼으며이작업자가별도로실행하지않았다.제품runtime C++변경은없고기존SkillStages test TU재컴파일이필요하다.새Resources가없으므로GBResources추가복사는없다.이펙트/shader를변경하지않았고Client/UI는실행하지않았다.실제화면에서카메라복귀후브레스와피해의최종체감은사용자가확인한다.

## G03. 최종 통합 검증

최종 Gameplay publish와 Debug/Release Product 빌드가 완료됐다. 양 구성의 SkillStages는 각각 95 PASS/실패 0이며 3.799초 무피해, 3.801초 첫 HP 타격·event 일치, 이후 중복 없음이 통과했다.

최종 빌드 영수증·ZIP·한계는 [통합 RESULT](2026-09-30_RAID_GAMEPLAY_REPAIR_RESULT.md)를 따른다.
