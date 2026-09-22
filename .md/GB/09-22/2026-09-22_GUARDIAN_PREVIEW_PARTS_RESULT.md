# Guardian preview stance와 S/F 원본 파츠 결과

## 반영한 코드

EffectAuthoringSequencer는 Product binding의 skill ID를 유지하여 Begin_Model 때 실제 editor
CCharacter clone에 catalog requiredStance를 적용한다. 이전 stance는 weak owner와 함께 보관하고
Stop·대상 교체·파괴 때 복원한다. HUMAN 스킬을 선택하면 HUMAN 파츠로 돌아온다. Server와 scene
캐릭터의 stance는 변경하지 않는다. Wing WModel과 IDENTITY equipment는 이미 설치되어 있으므로
붉은 F49270 preview에서 HUMAN baseline 때문에 날개가 숨는 연결 누락을 수정했다.

V1 factory 호출 직전에 이전 성공 상태를 비우고, runtime sampling false가 오류를 주지 않으면
stable effect ID를 포함한 fallback을 기록한다. Load_Sequence는 이전 custom action의 skill ID를
초기화하여 관계없는 저장본으로 stance가 전파되는 일을 막는다. custom draft에 skill ID를 추가로
저장하는 schema 변경은 하지 않았으며, 새로 Character Actions에서 선택한 binding과 저장된
Product model sequence를 통해 stance를 resolve한다.

## S의 용머리 실측

현재 정본 S는 49220 스피닝 플레임(HUMAN), 49230 아바돈 플레임(DRAGON)이다.
원본 Action49220 Effect→SkillEffect492200→Projectile492200은
Par_W_DDK_Flamescale_00의 16 emitter와 fm_j_ddk_gauntlet_01, 검격 mesh를 사용한다.
Action49230의 Effect notify012는 Projectile492300과492384를 참조한다. 492300의
Abaddon trail/Fire_exp00/01은38 emitter,492384의Fire_S_2는20 emitter이다.
원본5개particle system에는 skeletal 용머리가 없다. source acquisition 및 emitter 증거는
out/GuardianParts20260922 아래에 있다. 이 조사 후보를 제품에 추가 설치하지 않았다.

DRAGONKNIGHT.loa 전체 PlaySkeletalMesh를 대조했을 때 SK_DDK_DRR_01 용머리와
Cast/Trail/Mouth/Eye particle 부착은 **49290 DragonResonance stage1/5/6**의 원본 계약이다.
49350 DragonRoar에도 별도 head/neck 모델이 있다.49290과49350은 조사 시점 PlayerSkills와
GuardianKnight.skillbindings에 미등록이었다. 사용자 최종 승인에 따라 아래와 같이 일반 S49220에
49290의 실제 source presentation을 연결했다. Server skill ID나 변신 S49230을 재지정하지 않는다.

## 일반 F 반영

49260의 선택된3개IdentityParts는 MakeParts=false,FailCompleteCancel=true,
bExecuteNotifyEnd=true이며 원본 구간은 .1~.5초,0~.5초,0~.4초다.
기존 projection의 구간전체 visibility=0을 이 세 stable ID에만1로 교정하고 sourceValues에
원본flags[0,1,1]를 보관했다. 끝에서 제거하는 원본 정책과 PROJECT_ADAPTER의 구간 표시를
구분한다. CCharacter owner token의 show overlay를 CPart_Equipment가 소비하며 원래 visibility는
수정하지 않는다. token종료/제거가 현재 stance baseline으로 복원하고 hide suppression이 우선한다.
일반 F 날개 C++/data반영은 완료했고 화면은 사용자 확인 대상이다.

## 일반 S 교체 후속 승인

사용자가 일반상태 S49220에 용머리와 원형화염을 표시하도록 원본49290 presentation 교체를
승인했다. 변신 S49230은 유지한다. 신규 effect IDs는
`effect.guardianknight.skill.49220.source.49290.clip.0.full.restore`와 `.1.full.restore`다.
body클립은설치WModel의ddk_sk_dragonicresonance_01/02(26/68ticks,30Hz)를사용한다.
49290 원본13bone DRR01모델과1개actorclip은cook 완료했고 source actor는(0,.4,0)m,
2.3scale,playRate1.2를사용한다. 원본 head/neck material profile은
상위 작업이 생성한 SOURCE_CHARACTER 110/111을 실제 ModelCue가 사용한다. 일반 S 입력·body binding과
per-clip 재생률은 같은 기능의 별도 수직 연결 작업에서 적용한다.

## 일반 S 설치 결과

두 target49220/source49290 문서와 EffectCatalog 등록을 완료했다. 첫 클립은 9 elements,
두 번째 클립은 76 elements와 head/neck ModelCue 2개다. 원본 81 emitter 중 본 배열 확장으로
85 runtime elements가 생성되며, 31 elements가 용머리의 실제 본에 연결된다. source actor는
기존 CModel 변환 경로의 assetPreRotation -90도를 사용하며 전체 이펙트에 추가 회전을 강제하지 않는다.
13개 source joint와 mesh root를 포함한 실제 CModel 14 bones를 확인했다.

model actor의 LoopCount=1/StartAnimTime=0/StartAnimTimeUseOnlyFirst=true는 원본의 허용된
조합이다. decoder는 first-only bool 0/1을 허용하며 원본 tail의 위치·회전·크기는 그대로 유지한다.
원본 nested Cast/Trail/Mouth/Eye는 B_Root, B_Neck_01/03, B_Tongue_01, B_Skull에 연결했다.
1.1초 material track와 actor playRate1.2를 보존한다. 기존 S 두 문서는 남겨두고 새 안정 ID를
추가했으며, 현재 재생 선택은 일반 S의 새 body binding과 animevent cue가 담당한다.

source particle material은 기존 34개를 재사용하고 새 color native 4532~4552의 21개를 추가했다.
원본 distortion companion 22개도 같은 program ID에 설치했다. strict projection의 deferred emitter와
source material failure는 0이다. DDS/WModel 163개 중 146개는 기존 bytes와 동일하고 신규 17개만
Resources에 설치했다. Data 및 project 등록은 최신 디스크 bytes 검증, 백업, 원자 교체로 반영했다.
`out/GuardianParts20260922/DragonResonance/installation`에 설치·최종 runtimeExtensions 교정 receipt가 있다.

## 검증

- 기존 Python source actor 4개와 owner/camera control 9개 검사: 13/13 통과.
- 새 color native: 원본 PS DXBC와 21 programs × 4 fixture = 84건 대조, 최대 오차 0, finite 통과.
- 원본 distortion companion: 22 × 4 = 88건, 최대 오차 4.57763671875e-05, finite 통과.
  원본 discard는 기존 GPU probe의 -99 clear sentinel로 확인하고 합성 MRT의 중립 0과 대조했다.
- 현재 헤더와 out 전용 object로 기존 codec probe를 재컴파일했다. Parse/Validate_Drawable/roundtrip,
  source material parameter 12시점, owner control 종료 검증: 2 documents/85 elements/2 ModelCues/1 ownerControl 통과.
  clip0의 v15 runtimeExtensions 기본값 누락을 이 검사로 찾아 보완했고 설치본과 검증 후보의 일치를 확인했다.
- 기존 model consumer probe로 실제 설치된 head/neck WModel을 CModel에 생성했다. native 110/111,
  14 bones 및 필수 5개 본 이름, source animation 선택, material variant 독립 변경/복원 2/2 통과.
  embedded DDS 6개 경로도 모두 존재한다. 원본 material source textures 8개는 두 profile이 소비한다.
- 변경 C++의 UTF-8 BOM 없음/CRLF를 보존했다. Product 통합 컴파일은 상위 작업에서 수행한다.

CPU와 WARP shader 대조는 실제 Client 화면 승인이 아니다. Client/UI를 실행하지 않았으며,
최종 날개·용머리·원형화염의 시각 판정과 실행 중 도구 Reload는 사용자 확인 대상이다.
