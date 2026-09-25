# 전투 대상의 노란 피격 강조와 빨간 호버 외곽선 구현 계획

## G00. 현재 기준과 원본 조사

현재 브랜치 `codex/kouku-timeline-local-preview`의 기존 쿠크 저작·게시·코드 변경을 보존한다.
사진에는 몸체 윤곽을 따라 밝아지는 노란 피격 표현이 보인다. 원본 재질의 `hit_color`,
`selectioncolor`, `constantoutline` 입력과 현재 `CNpc`·`CValtan`의 흰색 Fresnel 피격
표시를 대조한다. 개체별 particle 복원을 공통 재질 강조와 혼동하지 않는다.

현재 `CClientReplication::Apply_WorldSnapshot`은 MONSTER의 CNpc와 BOSS의 CValtan만
피격 표시를 호출한다. CNpc로 표시하는 쿠크 계열 BOSS는 빠져 있다. Server의 일반
피격 필터는 2관문 대형 세이튼도 대상으로 수집한다. 기존 렌더링 옵션과 저작 데이터는
변경하지 않으며 Client 실행·화면 확인은 사용자가 수행한다.

## G01. Server의 대형 세이튼 제외

기존 Shared PacketMessages의 archetype predicate와 ServerCombatHitRuntime의 공통
player-damageable 필터에서 `BOSS_KAKULSAYDON_G2_BIG_SAYDON`만 제외한다.
직접 피해, 스킬·투사체·전투 오브젝트 후보 수집이 같은 필터를 사용하여 HP·무력화·
카운터·부위파괴·피해 이벤트와 maximumTargets 소모를 차단한다. 보스 spawn·패턴·
몸체 충돌은 유지한다. 기존 SkillStages contract에서 제외와 일반 쿠크 허용을 검사한다.

## G02. 서버 피해 이벤트와 공통 모델 표현

`ClientReplication`은 양수의 outgoing Server DAMAGE_EVENT에만 노란 피격 표시를
요청한다. CNpc와 CValtan의 실제 모델 타입으로 전달하며, 몬스터 hit animation은 기존
IDLE/CHASE 조건을 유지한다. 쿠크·세이튼·앵콜·카드미로 중앙 파괴체와 발탄 및 발탄
스폰 몬스터는 기존 WorldEntity 표현을, 인형·공은 G05의 기존 owned WORLD cue를 사용한다.

`CNpc`, `CValtan`의 기존 피격 시계와 `DEFERRED_EMISSIVE_OVERRIDE`를 확장한다.
노란색 Fresnel 강조를 몸체·무기·모자·갑옷에 같은 수명으로 전달한다. 호버는 별도의
빨간 외곽선 상태이며 피격 시계가 끝나도 커서가 남아 있으면 유지한다. 모델 셰이더의
기존 pass 번호와 재질 coverage를 보존하고 필요한 outline pass만 끝에 추가한다.
공유 prototype 재질이나 원본 descriptor를 매 프레임 변경하지 않는다.

## G03. 공통 호버 선택과 수명

`CClientReplication::Update_CombatHover`는 최신 Server HP/action, kind, owner와 공통
archetype 제외 조건으로 후보를 제한한다. 현재 모델의 실제 presentation root와 모델
현재 골격 포즈 경계에 cursor ray를 교차시켜 가장 가까운 대상 하나를 강조한다. 단순 radius만으로
큰 세이튼을 선택하거나 벡터 index를 저장 ID로 쓰지 않는다.

Valtan/Kouku/CharacterSelect/Bern/Development의 기존 입력 처리 위치에서 호출한다.
UI가 마우스를 소비하거나 free camera·연출·모달·연결 종료 상태이면 강조를 해제한다.
매 프레임 이전 상태를 정리하고 despawn·world reset·presentation suppression에도
이전 대상의 빨간 상태를 남기지 않는다. 호버는 읽기 전용 presentation이며 packet이나
Client 피해 판정을 만들지 않는다.

## G04. 검증과 전달

새 C++ 파일 없이 기존 프로젝트 항목을 수정한다. Shared/Server의 관련 contract와
정상 Debug Product 증분 Build로 SDK·Client·model CSO를 함께 확인한다. 변경 범위의
`git diff --check`를 실행한다. JSON·프로젝트 XML을 변경하면 해당 parse도 확인한다.
원본 확인 범위, 실제 구현, 컴파일·실행 검사와 사용자 화면 미확인은 RESULT에서 구분한다.
대규모 기존 dirty worktree는 자동 stage/commit하지 않는다.

## G05. 공·인형의 WORLD cue와 실제 골격 경계

Server WORLD_OBJECT는 일반 WorldEntity spawn/snapshot에서 제외되고 owned WORLD cue로
표시된다. 기존 `S2C_WORLD_SEQUENCE_PLAY`에 Server가 확정한 combat body NetEntityId를
전달하여 피해 이벤트와 해당 cue의 모델을 연결한다. 공격할 수 없는 배경 소품은 invalid ID를
유지한다. 기존 PLAY/STOP_CUE/STOP_OWNER와 late join 수명을 사용하며 별도 모델을 생성하지
않는다. 공통 호버 후보 비교와 노란 피해 이벤트가 이 경로까지 연결된다. wire 변경에 맞춰
protocol과 Writer/Reader, 기존 network contract를 함께 갱신한다.

설치 원본 Valtan/Saydon의 raw vertex에 preScale만 적용한 bind bounds는 실제 골격 scale을
빠뜨려 약0.03m에 불과하다. Engine CModel/CMesh는 로드 때 bone 영향 정점의 경계를 보존하고
현재 skin palette로 변환한 경계를 조회하는 API를 제공한다. GPU skinning과 같은 inverse-bind,
combined pose, preTransform 순서를 한 번 적용한다. 매 프레임 전 정점 readback/스키닝을
추가하지 않으며 기존 bind bounds의 의미는 유지한다. Engine public header 변경은 정상
Product Build에서 SDK 배포와 Client 재컴파일을 함께 확인한다.

## G06. 현재 기준 잔상 알파 2배

사용자 추가 요청에 따라 기존 발탄 대시와 쿠크 계열 대시·카운터의 모델 잔상 알파를 현재값의
2배로 조정한다. 발탄 대시는0.38→0.76, 쿠크 대시는0.19→0.38, 쿠크 카운터는0.35→0.70이다.
끝점0과 기존 페이드·샘플 주기·수명·색은 유지한다. 별도 backstep·카드 particle와 공용
SkeletalAfterimage 기본값에 이 변경을 전파하지 않는다. 현재 발탄에는 별도 counter 모델
잔상 경로가 없어 새 카운터 연출을 만들어 알파 조정으로 보고하지 않는다.


## G07. 원본 TrailGhost 발생창과 별도 알파 튜닝

현재 설치 data3.lpk의 발탄·쿠크 계열 Action LOA 10개와 EFGame.u의 TrailGhost reflection을
기존 decoder로 읽고 원본 payload/hash·enabled·시각·22필드를 out/CombatSourceAudit20260925/
afterimages에 보존한다. source-enabled notify와 현재 저작 sourceActionId/sourceStageId를
대조한다. 원본에 없는 카운터 잔상 또는 피격/호버 particle을 추측하여 추가하지 않는다.

기존 발탄420604 돌진과 세이튼4219863 warmup/dash,4219951 backstep의 발생 종료·주기·자식
수명·source diffuse·시작/끝 rim을 기존 CSkeletalAfterimage로 연결한다. child마다 설정을
복사하여 다음 발생의 설정이 살아 있는 이전 tail을 바꾸지 않도록 한다. 발생창보다 sample
period가 긴 notify도 처음 제시된 실제 pose 하나를 보존하고 미관측 subframe pose는 만들지
않는다. pause·seek·숨김·모델 교체·실패 격리 경계는 유지한다.

SETTINGS의 source initial alpha와 presentation multiplier를 나눈다. 사용자 현재 peak인
발탄0.76, 세이튼 돌진0.38, 카운터0.70과 별도 backstep0.19는 그대로 보존한다. 원본
InitialAlphaDuration을 plateau로 해석하고 남은 수명에 기존 squared fade를 적용하는 정책,
색 endpoint의 선형 보간과 시선 rim은 PROJECT_RECONSTRUCTED adapter로 기록한다. 이 함수는
원본 native shader/fade ABI가 확인됐다는 뜻이 아니다. animated afterimage PS에서 diffuse,
ambient, rim을 별도 입력으로 처리하고 기존 PROJECT_AUTHORED 카운터와 다른 모델 cue의 기본
모드는 유지한다. SourcePartType NONE의 몸체 적용도 확인된 enum과 프로젝트 해석을 구분한다.

수정 파일은 SkeletalAfterimage h/cpp, Npc h/cpp의 잔상 경계, Body_Valtan의 잔상 설정,
Valtan의 발생창, KoukuSaydonPresentationPlayer의 해당 함수·호출부와 animated shader의 기존
afterimage 입력/PS다. 신규 C++/Data schema/별도 runtime은 만들지 않는다. 최신 바이트로
부분 수정하며 기존 사용자 편집과 다른 담당의 같은 shader 변경을 보존한다. 관련 최소
C++ 컴파일과 실제 helper의 수명/색/alpha 수치 검증을 수행하고 shader/Product 통합 빌드는
root가 합친다. 실제 Client/UI 실행과 원본 화면 동등성은 자동 검증 결과로 대신하지 않는다.


## G08. 원본 Hit_Color draw 입력과 호버 색 복원

현재 SourceCharacterMaterialParameters의 실제 monster program별 Hit_Color packing과
원본 cooked base 연산을 사용한다. 21~32/84/92/93/238의 해당 입력은 base 전용이고
deferred light constant에는 없다. DeferredMaterialRenderUtils는 모델 재질 bind 다음
그 draw의 base constant 사본만 수정한다. 원본 normal/view 및 source permutation의
rim 연산을 그대로 실행하고 같은 draw의 일반 Fresnel 중복 가산은 해제한다.
몸체·장비·모자·WORLD 오브젝트가 같은 함수를 소비하며 공유 CMaterial은 변경하지 않는다.
다음 재질 bind는 authored constant를 복구한다. native 입력 없는 legacy 모델은 기존
프로젝트 피격 표시를 유지하고 원본 native 복원으로 분류하지 않는다.

WorldSequenceObject의 static/animated/forward 공통 override 호출도 실제 Model/mesh를
받아 동일 경로를 사용한다. 원본에서 확인되지 않은 hit CPU 색·시간은 별도의 프로젝트
값으로 보존한다. ColorOption.loa의 OUTLINE_MONSTER_ENEMY=FE0000을 호버 색 근거로
사용한다. 원본 PPOutline API와 현재 외곽선 adapter의 구조 차이는 RESULT에 구분한다.
변경 H/CPP 최소 컴파일, native constant/reset 검사, 현재 설치 모델의 실제 draw 및
정상 Debug Product Build로 검증한다. 새 C++ 파일/프로젝트 등록은 필요하지 않다.


## G09. 쿠크 피격 반응의 원본 AKEvent 연결

원본 MN_RPCZ_00 Action12 BEHIT의 Dmg_Idle_1/2는 0.001초에
S_Mob_G_KouKu1.G_KouKu1_Damage1을 발생시킨다. 현재 actionreference의 실제 runtime clip은
rpcz00_dmg_idle_1/2이고, CharacterSoundCatalog의 KoukuSaydon 클래스에 동일 이벤트의
원본 Wwise media 5개가 설치되어 있다. source-occurrences.json과
event-media-installed-audit.json(out/CombatSourceAudit20260925/audio-fx)이 원본 단계·offset·
bank·media·설치 asset ID 근거를 소유한다.

Client/Public/Npc.h와 Client/Private/Npc.cpp의 기존 성공한 Play_NetworkAction 경계에
한 번만 소비하는 피격 음성 pending clip을 둔다. 실제 spawn이 쓰는 typed 서비스의
ModelPrototypeTag와 두 정확한 runtime clip을 대조한다. EffectV2 binding owner는
model stem이므로 boss archetype 문자열을 owner로 추측하지 않는다. Update에서 실제 모델 clip 시간 0.001초를 지나면
CSoundCueCatalog→CRuntimeAssetRoot→GameInstance::Play_Sound 기존 서비스를 사용한다.
clip 변경·재생 실패·숨김은 pending을 취소하고, 시도 전에 pending을 비워 반복 frame이나
음원 실패가 재시도로 중첩되지 않게 한다. 다음 semantic action의 명시적 재시작은 새
occurrence다. 공유 shader·afterimage 구간·플레이어·다른 몬스터·generic damage flash에는
이 연결을 전파하지 않는다. 현재 boss damage는 rim만 발생하므로 서버 action이 실제
피격 clip에 진입하지 않으면 음성도 재생하지 않는다.

새 C++ 파일과 project/filter 등록은 없다. Npc TU 최소 compile, exact owner/clip와
1ms 단발 소비·실패 격리의 좁은 검증, 기존 설치 5 WAV의 존재·형식 검사를 수행한다.
원본의 다른 대상 피해 음원은 out 후보만 준비하며 미확인 SoundSet slot을 새 damage
분기로 추정하지 않는다. Product 통합 빌드와 사용자 청취 결과는 별도로 기록한다.


### G07 보충. 전수 join에서 확인한 직접 관련 25 occurrence

현재 Kouku composition과 native TrailGhost를 profile/action/stage로 대조한 81개 중
source disabled 20, 앞선 대시/백스텝 adapter 20, 다른 enabled 41개를 구분한다.
41개 중 원본 `쿠크_돌진` RPCZ00 action4219776의 현재 거미카운터 15개,
RPCT07 action4219920/4219921 고속이동 전방·후방 10개를 이번 범위에 포함한다.
나머지 16개는 융단폭격·안전지대·피자·무기뇌격 등 별개 공격이므로 확대하지 않는다.
관련 25개는 removal flags false, part NONE, custom material/reference 0, pivot None,
scale 1, viewOffset 0이며 기존 carrier의 수치 입력에 맞는다. 출처와 occurrence별
전체 입력은 `out/CombatSourceAudit20260925/afterimages/remaining-scope-classification.json`에 둔다.

새 carrier/shader 없이 같은 source history가 현재 notify와 마지막 child 수명이 끝날
때까지 authored counter보다 먼저 표시된다. Counter flag의 변경은 source child를
지우지 않으며, source가 끝나면 기존 240ms/160ms·peak .70 pulse로 돌아온다.
RPCZ 원본 initialAlpha .6과 신규 기본 userAlphaScale 1을 보존하되, counter window에서
기존 사용자 peak .70을 대체하는 동안에는 userAlphaScale=.70/.6을 사용한다.
기존 Saydon dash .38, backstep 계열 .19는 그대로다. Source child마다 자기 설정을
보관하므로 counter flag 변경이 이미 만들어진 child의 원본 수명/색/알파를 다시 쓰지 않는다.

LOA 색은 packed FColor 4byte가 아니라 little-endian uint32 4개로 직렬화된 16byte다.
기존 decoder는 이를 RGBA로 해석한다. reflection의 Core.Color 타입과 raw component
sequence는 확인했지만 native custom serializer의 R/B 의미·gamma 변환은 미회수다.
따라서 /255·RGB 순서·색 interpolation·rim 계산은 PROJECT_RECONSTRUCTED 해석으로
구분하고 근거 없는 BGRA swap/sRGB 보정을 추가하지 않는다. raw hex와 field offset은
`color-packing-receipt.json`에 보존한다.

제품 build 중에는 out 후보만 검증하며, 완료 후 현재 소유 block의 exact-match와
교체 직전 hash를 확인한 뒤 최신 Npc audio 변경을 보존해 적용한다. Npc·presentation
TU 격리 compile과 25개 실제 occurrence/원본 수치 및 counter↔source tail native 검사를
먼저 수행한다. 최종 source 적용과 정상 증분 Product는 RESULT에서 별도 기록한다.


G09 추가 범위: 현재 실제 피격 animation 소비자가 있는 PADD·SJFC/ELITE·0019_05·ClownBox는
MonsterPresentationAssetService의 정확한 model tag와 MonsterCatalog의 현재 hit clip만
대조해 각각 원본10/10/1/1ms notify를 연결한다. 기존 Valtan/KoukuSaydon SoundCueCatalog
클래스에 원본 event4개를 추가하고 Sound 경계의 원본 WAV18개를 설치한다. 기존 RPCZ까지
다섯 이벤트별 전역 이전 asset ID를 제외하고 균등선택한다. 카탈로그 순서/크기 변경에도
index를 저장하지 않으며 선택 가능한 자산이 하나뿐이면 그 자산을 허용한다. 이 선택은
원본 Random·equal weight·global·avoid-repeat1 근거를 사용한다. 원본 Stop-other-voice,
voice-limit, bus/RTPC 제어를 기존 Play_Sound가 모두 재현한다고 주장하지 않는다.

최종 후보는 out/CombatSourceAudit20260925/audio-fx/consumer-candidate.json의 소유 block과
4개 event field 단위로 최신 디스크에 병합한다. 교체 직전 bytes/hash 재확인, backup,
원자 교체와 자기 변경 rollback을 적용하며 첫 Product Build가 끝나기 전에는 설치하지
않는다. WORLD 인형의 Dmg_Idle sequence는 현재 gameplay damage가 시작하지 않으므로
후보 WAV5개는 out에 남긴다. 카탈로그는 CProjectDataRoot로 직접 소비하며 새 runtime,
schema, publisher를 추가하지 않는다. 후보22 case는 실제 typed tag 함수와 현재 catalog,
설치 WModel의 clip bytes를 대조하고 model clock/audio backend만 좁게 대체한다.
