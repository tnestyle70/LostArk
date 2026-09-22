# Guardian preview stance와 S/F 원본 파츠 점검

## 실측

변신 F49270은 `requiredStance=GUARDIANKNIGHT_DRAGON`이다. `GuardianKnight_Wing.wmodel`과
`Logic_GuardianKnight`의 stance-owned IDENTITY 파츠는 이미 설치되어 있다. Tool의 실제
CCharacter preview는 생성 시 scene stance 또는 HUMAN fallback을 받지만 스킬 선택은
requiredStance를 반영하지 않는다. 따라서 HUMAN preview에서 붉은 F를 골라도 날개가 숨겨진다.

## 구현

기존 EffectAuthoringSequencer가 모델 clock을 소유하는 동안에만 선택한 Product skill의
requiredStance를 preview clone에 적용한다. saved model sequence는 저작 skill ID를 유지하고,
Stage_CharacterAction은 전달받은 binding skill ID를 보관한다. Begin_Model에서 기존 stance를
보관한 뒤 적용하고 Stop·대상 변경·파괴는 원래 stance를 복원한다. 스킬을 다시 선택하면 새로운
requiredStance를 사용하며 HUMAN 스킬은 HUMAN으로 돌아온다. Server/scene 캐릭터는 변경하지 않는다.

## 원본 조사 범위

S49220/49230은 source Effect notify와 SkillEffect→Projectile 경로를, 일반 F49260은
IdentityParts source payload와 실행 시점을 계속 대조한다. 근거가 확정되지 않은 파츠나 socket은
추측해서 생성하지 않는다. 기존 모델/셰이더/CModel 경로를 사용하며 새로운 runtime은 만들지 않는다.

## 검증

소유 C++ 파일의 기존 인코딩과 다른 작업 변경을 보존한다. 통합 컴파일은 상위 작업에서 수행한다.
변경 부분의 수명과 실패 복원을 코드 검토하고 diff 검사를 실행한다. Client/UI는 실행하지 않는다.

## 일반 F 세 구간 후속 구현

원본49260의MakeParts=false,bExecuteNotifyEnd=true는 종료시 제거 정책이다.
기존 ownerControls PROJECT_ADAPTER의 세 ID만 구간 visibility=1로 교정하고 원본3flags를
sourceValues에 보관한다. CPart_Equipment는 원래 visibility를 변경하지 않는 presentation
visible overlay를 소비하며 hide suppression이 우선한다. CCharacter는 active owner token을
합산하여 show/hide를 적용하고 제거 시 overlay만 해제한다. 기존 다른8개Guardian visibility와
stance,scene값은 보존한다. S49290의 연결 대상은 아래 사용자 최종 승인 계약을 따른다.

## 승인된 일반 S source49290 연결

사용자 최종 요청은 일반상태 S49220의 용머리/원형화염이다.49230 변신S는 유지한다.
49220의server수치와HUMAN requiredStance를보존하며source49290의두bodyclip와head원본을
기존ModelCue+particle경로에연결한다. source13bone DRR01 head/neck을material별WModel로
cook하고PSA clip시간을CAnimation30ticks로맞춘다. 원본relativeTransform(0,.4,0)m,scale2.3,
actorRate1.2를보존한다. Cast/Trail/Mouth/Eye는실제본B_Root/B_Neck_01/03/B_Tongue_01/B_Skull을
modelCueId에소속시킨다. 두문서는target49220/source49290 stable ID로등록한다.
상위작업은head/neck sourcecharacter재질110/111,별도에이전트는HumanSbinding과ground-target typed
입력계약을담당한다. 이작업은particle source native4532..4623 예약영역을사용한다.
