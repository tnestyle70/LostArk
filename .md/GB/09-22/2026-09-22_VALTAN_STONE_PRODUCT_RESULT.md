# 발탄 돌 Product 생성 연결 결과

## G04. 2026-09-28 — 붉은 돌 폭발과 중앙 피자 원본 연결

### G04-1. 설치된 범위와 이전 저작본 보존

사용자가 EXE 종료와 전체 복구·빌드를 승인한 뒤 최신 디스크 저장본을 기준으로 원본 복원
Effect 문서 17개를 추가하고 catalog·resource tree·Pattern cue를 stable ID 기준으로 병합했다.
기존 tuned 돌·피자 문서는 삭제하지 않았다. 130개 Resources 참조를 닫았으며 이 중 신규 설치는
28개다. Effect native 프로그램은 5184..5214의 31개와 5220..5240의 21개, 합계 52개를 추가했다.
돌 모델은 별도 SourceCharacter 프로그램 1533과 원본 MIC 2개를 연결했다. 두 shader ID 공간은
서로 다르며 기존 native 재사용 항목을 신규 복원 수에 중복 계산하지 않는다.

신규 C++ 파일은 없다. 17개 authored JSON은 `Client.vcxproj`와 `.filters`의 `None`/
`96.DataFiles`에 등록했다. 설치·조합 입력과 한국어 검색명은
`out/ValtanStonePizzaRestore20260928/integration/manifest.json`에 있다.
아래 G00~G03과 2026-09-22 참고 항목은 이전 작업 결과이며, 이번 원본 복원 완료 범위와 구분한다.

### G04-2. 돌 원본, 모델·재질, 시각의 소유자

원본 Prop 375333/375339의 `EFDLProp_ITR_02326.ITR_02326` LookInfo가 stationary 돌의
실제 소유자다. 원본 `FX_ITR_02326`의 On01 8개, Off01 20개, Spawn01 15개, Off02 9개,
합계 52개 emitter의 첫 LOD·CDO·archetype·module 순서·원본 재질과 메시를 복원했다.
원본 package 경로·hash, 57개 texture와 8개 particle WModel 경로는
`lifecycle/FINAL-MANIFEST.json`과 `lifecycle/geometry-manifest.json`에 기록했다.

LookInfo parent event의 실제 해석은 다음과 같다. 모든 네 효과의 local TRS는 identity이고
instance parameter override가 없다. On/Spawn/Off02는 `B_Root`, Off01은 root snapshot이다.

| 원본 ParticleSystem | 시작 event | parent 지연 | 제거 event |
|---|---|---:|---|
| Par_L_ITR_02326_On_01 | PAIT_On | 0초 | PAIT_Off |
| Par_L_ITR_02326_Spawn_01 | PAIT_Spawn | 0초 | PAIT_On |
| Par_L_ITR_02326_Off_02 | PAIT_Off | 0초 | PAIT_MAX |
| Par_L_ITR_02326_Off_01 | PAIT_Off | 1.820000052초 | PAIT_MAX |

`lifecycle/lookinfo-particle-events.json`은 원본 byte offset과 EFGame enum/struct reflection을
보존한다. Off01의 1.82초는 parent lifecycle 지연이다. 독립 PS 편집 문서는 local time 0부터
시작하고, 전체 Off 조합만 이 지연을 한 번 적용한다. On의 원본 emitterLoops=0은 유지했으며
독립 편집기의 60초 emission cap은 무한 원본의 수명이 아닌 유한 Preview 정책이다.
`등장·청록 대기` 독립 조합의 모델은 기존 model-cue 최대 구간에 따라 31초에 끝나고,
On particle의 유한 Preview tail은 91초까지다. Product에서는 combat-object owner의
Armed/Hit 전환이 조합을 종료한다. 독립 편집기의 이 경계를 원본 무한 유지 수명으로 설명하지 않는다.

원본 `ITR_02326_SK` 모델의 두 material slot을 기존 WModel subset 경로로 분리했다.
WMA2, skeleton, 네 animation은 원본과 byte-identical하고 정점·index는 각 원래 slot에만 남는다.
`ao_spawn`, `ao_on`, `ao_off`와 원본 preScale 0.01을 사용한다. 두 MIC는 같은 GPU-skin
Base/Light bytecode·parameter pack을 쓰며 normal texture만 다르다. 전체 모델을 두 번 겹쳐
그리거나 원본 대신 튜닝된 particle 돌을 복사하지 않았다.

원본 LookInfo의 material action 세 개도 PAIT_Off를 기준으로 한다. emissive intensity는
0.6초 뒤부터 1.5초 동안 5→50, 색은 처음 0.5초 동안 청록→빨강, 이어 0.5초부터 3초 동안
짙은 빨강으로 변한다. 기존 material parameter track으로 원본 endpoint·duration·delay를
연결했다. reflection은 원본 실행 파일의 보간식을 증명하지 않으므로 기존 linear curve 투영과
원본 GPU 픽셀 동일성을 같은 결과로 기록하지 않는다. 근거는
`lifecycle/model-native/lookinfo-material-events.json`, `verification.json`, `result.md`다.

Off01의 `particlespriteemitter_4`는 TypeData의 Required override 대신 실제
`ParticleModuleMeshMaterial_0` 한 항목을 소유한다. native5233을 기존
`detail.mesh.sourceMaterialSlots[0]`에 연결하고 Required.Material은 inactive evidence로
보존했다. source module은 삭제하지 않았으며 단일 배열 reference literal만
`meshmaterials[0].objectpath`로 정규화했다. 원본 export395의 serial SHA와 대응은
`lifecycle/mesh-material-slot-evidence.json`에 있다. 또한 52개 element 표시명을 UTF-8
64-byte 제한 안의 짧은 한국어로 수정했다.

### G04-3. Server event, Preview, 현재 게임플레이와 원본 시각의 차이

`combatobject.valtan.six-pizza.rock-pillar`의 active는
`effect.valtan.source.itr-02326.spawn-and-idle`, armed와 hit는 같은
`effect.valtan.source.itr-02326.off.full`을 참조한다. 원본의 Spawn/On과 전체 Off를 기존
combat-object lifecycle에 연결하는 adapter이며 Server의 피해·위치·cover·연쇄 판정은 바꾸지 않았다.
현재 Server 연쇄 지연 1.5초와 원본 Off01 시각 지연 1.82초를 각각 보존했다. 따라서 전체 Off를
Armed 시각에 시작한 연쇄 돌은 Server hit보다 원형 폭발 표시가 약 0.32초 늦다. 이를 피해 시각과
완전히 일치한 원본 재현이라고 설명하지 않는다. 직접 hit만 받은 돌은 hit에서 전체 Off를 시작한다.

`BOSS_COMBAT_OBJECT_VISUAL_ENTRY`의 optional `stopActiveOnArmed`와
`armedEffectOwnsTerminal`은 기본 false다. 후자는 true일 때 paired V1 armed/hit asset의
동일성을 ActorCatalog와 두 publisher validator가 검사한다. `CValtan`은 Armed Effect 생성이
성공한 combat-object ID만 기록한다. 후속 Hit는 그 기록을 소비하여 Effect 중복만 막고 Sound는
계속 처리한다. 생성 실패는 소유 기록을 남기지 않아 직접 Hit fallback을 보존한다.
마지막 검토에서 Effect 생성 뒤 Sound만 실패하면 전체 반환값 때문에 기존 active 모델이
남는 경로를 발견했다. `Apply_CombatObjectPresentationEvent`의 optional visual commit 출력을
Sound 성공과 분리해 Replication이 성공한 시각 전환을 완료하도록 고쳤다. 소리 실패는 기존
오류 반환·진단을 유지하며 Effect 생성 자체 실패에서는 active를 보존한다.

Local Preview는 같은 Armed 시작 시계를 Exploded 이후에도 사용하고 전체 Off의 자연 수명을
샘플링한다. delay 0의 직접 cone hit도 하나의 Off를 time 0에서 시작한다. flag=false에서는
기존 warning/terminal 분리 재생을 유지한다. terminal이 소유 기록을 소비하며 boss occurrence
reset은 전체 기록을 비운다. 취소된 기록은 최대 저작 수명 600초에 1초를 더한 Server tick
경계를 넘으면 정리하고, 512개 cap에서는 기존 소유자를 버리는 대신 신규 Armed Effect 생성
실패를 명시한다. 해당 Hit fallback과 Sound는 계속 가능하다.

### G04-4. 중앙 피자 원본과 활성화한 바닥 경고

원본 Action420620의 stage001은 현재 STEP_07/12_07에 36개 element, stage005는
STEP_11/12_11에 11개를 연결했다. 원본 baked weapon trail과 Atk_08_01/02/04를 포함한다.
stage006/12_06 변형은 Atk_08_07의 활성 particle 9개를 독립 문서로 제공한다. 큰 붉은 slash의
원본 `fm_o_halfspear_01`, TypeData rotation·StartRotation·StartSize·notify TRS를 유지했고
전체 mesh에 임의 회전이나 배율을 적용하지 않았다.

노랑은 원본 SkillDecal2116/SkillEffect42061919의 300도·yaw180도, 빨강은
2006/42061920의 55도·yaw0도이며 반경 100m, 중앙 제외 반경 1.5m다. 중앙 빨간 원은
2002/42061921의 반경 1.5m다. 원본 시작 0.4/1.9/3.4초와 각 수명·fade·GroundEffect color를
기존 decal 경로에 연결했다. 현재 저작 warning 시작 11초/23초는 유지하고 STEP_01의 cue
참조만 새 조합으로 바꿨다. STEP_07/11에는 각각 원본 impact cue를 추가했다.

다만 이 GroundEffect notify 9개의 원본 serialized active flag는 0이다. 활성화한 노랑·빨강·
중앙 원 및 조합은 한국어 이름에 **원본 자료 기반 저작**을 명시했다. exact stage006 복원은
원본 disabled 상태를 유지한다. 노랑 native3601의 `EngineResources.DefaultTexture` UV warp
원본 payload 1개는 확보하지 못해 기존 white texture 중립화 adapter를 재사용한다. 원본 MIC의
inner 기본값을 유지하고 근거 없는 radial 시간 곡선은 만들지 않았다. 이 한계는
`pizza/candidate-manifest.json`, `pizza/native-resolution.json`, `pizza/README.md`에 기록했다.

All Effects의 `돌·피자 원본 복원` 분류에서 `발탄 돌`, `붉은 충전`, `낙하 폭발`, `모아치기`,
`노란 부채꼴`, `빨간 부채꼴`로 찾을 수 있는 저장 이름을 등록했다. 돌의 독립 PS 네 항목과
`발탄 돌 원본 복원 | 등장·청록 대기`, `발탄 돌 원본 복원 | 붉은 충전·폭발 전체` 두 조합을
각각 편집 대상으로 제공한다. UI 검색 결과와 최종 색·형태는 사용자 화면 확인 대상이다.

`ValtanFullRestoreAnimations.json`은 원본 action/stage/clip 복원 ID만 허용한다. 최초 설치에
포함한 파생 sector 3행은 이 계약에 맞지 않아 그 행만 원자적으로 제거했다. 원본 stage001/005/006
행은 보존하고 sector 3개는 EffectCatalog·direct authored index·ResourceTree로 검색/편집한다.
실제 source metadata와 새로 컴파일한 reader로 설치 17개 전부의 exact editable path를 확인했다.

추가 확인한 도끼 전기는 STEP_10의 원본 stage004/notify004 `Par_O_RPBF_Atk_08_05`
emitter26이다. `startcontrol → b_wp_r_01`에 0~5초 부착되고 원본 RGB는 (90,2.5,0.5)다.
6방향 후 전멸의 Atk_09_01 emitter26과 같은 전기 재질/native2995를 쓰며 후자의 RGB는
(5,50,30)이다. 붉은 버전은 이미 기존 stage004 복원과 STEP_10 cue에 연결되어 있었다.
충전 뒤 큰 붉은 검기는 STEP_11 stage005/notify009의 Atk_08_04이며 clip 시작 0.162793초에
발생한다. 새 stage005 조합에 원본 mesh6/sprite2, 속도1000cm/s·속도 배율3→0.2·수명0.8~1초를
연결했다. 첨부 화면과의 대응은 형태·시퀀스 근거이며 픽셀 동일성 검증은 아니다.

### G04-5. 실행한 검증과 후속 확인

- `lifecycle/runtime/RESULT.json`: 실제 Apply 메서드와 Preview 분기를 추출한 native probe의
  29개 확인 통과. Armed 생성 실패·소유 commit·Hit 중복 억제·Sound 유지·직접 Hit·optional false·
  delay 0·tail·reverse sample·용량 제한과 취소 기록 정리를 포함한다.
- 후속 `lifecycle/runtime/sound-visual/RESULT.json`: 실제 event 함수와 Replication 종료 분기
  45개 확인 통과. 이전 전체 성공 guard를 되돌린 비교본은 Sound 실패 후 cyan active 미종료를
  정확히 재현했다. 수정 후 Sound 실패·빈 catalog·terminal/직접 hit에서 visual commit을 유지하고
  Effect 자체 실패에서는 기존 active를 보존했다. 최종 빌드와 해당 3파일 SHA가 동일하다.
- 실제 `ActorCatalog.cpp`, `Valtan.cpp` 구문 컴파일 통과, 입력 hash 안정성 확인. 기존 SDK
  C4819 경고는 별도로 남았다. owner-hit Python 계약 6개, PowerShell publisher AST parse,
  변경 코드 `git diff --check` 통과.
- 모델 전용 actual codec 문서 3개와 headless WARP CModel cue 6개가 통과했다. 세 clip×두
  subset×7시각의 bone matrix 42세트는 원본과 byte-identical이며 두 재질×8시각의 native
  parameter pack 16개가 일치했다. 창 생성·GPU draw·제품 Client 실행은 하지 않았다.
- 최종 설치 17개 문서의 actual codec/CPU playback이 통과했다. native packet250개,
  finite particle5,213개, projector70개, trail point173개이며 원본 범위100m/중앙1.5m를 확인했다.
  최종 파일 hash와 실행 로그는 `pizza/native-validation-receipt.json`에 있다. root-only 재생은
  실제 본 부착의 화면 성공이나 GPU 픽셀 동일성을 증명하지 않는다.
- 새로 빌드한 `Engine/Bin/Debug/Engine.dll`을 직접 로드한 headless WARP probe에서 실제
  `CModel::Create_MaterialVariant` 6개와 SourceCharacter1533 선택이 통과했다.
  `lifecycle/model-native/probe/run-new-engine.log`의 마지막 값은 `nativeVariant=1`이다.
  정상 배포 뒤 `Client/Bin/Debug/Engine.dll`을 로드한 `probe/run-native.log`도 같은 검사에 통과했다.
- 현재 `gameplay-publish.log`와 `composition-publish.log`는 각각 Publish succeeded/passed를
  기록한다. 실행 중 Server 메모리와 Client 화면이 자동으로 갱신됐다는 뜻은 아니다.

정규 Debug Product 빌드는 14:48:09에 통과했다. 마지막 visual/Sound 분리 변경이 전부
실행 파일에 포함되도록 후속 증분 빌드를 수행했으며 14:49:47에 다시 통과했다. 최종 Client는
14:49:45에 링크됐다. Engine DLL은 14:36:51 빌드본이며 Client 배포본과 동일하다. Server 코드는
이번 변경이 없어 기존 EXE가 up-to-date로 통과했다. 두 빌드 receipt는
`out/BuildPipeline/runs/20260928T054809167Z-debug-product.json`과
`out/BuildPipeline/runs/20260928T054947539Z-debug-product.json`이다. 마지막 증분은 OBJ21,
CSO0, binary1이며 기존 C4819/C4828 및 DirectXTK PDB 경고는 남고 빌드 오류는 0이다.

| 최종 항목 | 확인 상태 |
|---|---|
| 설치 17개 문서의 통합 runtime codec/playback | PASS. `pizza/native-validation-receipt.json` |
| SourceCharacter1533 새 Engine registry의 native variant stage | PASS. `lifecycle/model-native/probe/run-native.log`, 새 Client 배포 DLL 사용 |
| 한국어 검색 및 17개 exact source 편집 경로 | PASS. `search/receipt.json`; 발탄 돌6·붉은 충전1·모아치기10·노랑1·빨강2 검색 결과 |
| Engine/Shared/Server/Client Debug Product 빌드 | PASS. 최종 `20260928T054947539Z-debug-product.json` |
| 실제 아레나·All Effects 화면, 색·방향·크기·부착 | 사용자 확인 대기. 이 작업에서 Client/UI를 실행하지 않음 |

사용자 요청으로 신규 Resources 28개만 `C:/Users/user/Desktop/GBResources/Effect/...`에
원래 Resources-relative 경로대로 복사했다. 10,541,999bytes이며 28개 모두 현재 설치 원본과
SHA-256이 같다. 그 폴더의 `README.md`와 `resources-manifest.json`은 안내·복사 확인용이다.
전체 리소스 팩·런타임 정본 manifest가 아니며 JSON/코드/CSO/EXE는 포함하지 않는다.
사용자 기존 Sound authored/dash Effect/gameplay와 렌더링 옵션 보존 결과는
`out/ValtanStonePizzaRestore20260928/preservation-check.json`에 있다.
최종 `final-verification.json`은 설치17개 body hash 유지, 변경 JSON/XML34개 parse,
배포 Engine DLL 동일성, 신규 리소스28개 전달 hash, 최종3개 C++ source hash와
`git diff --check` 통과를 기록한다.


## G00. 반영 상태

초기에는 편집 중 반영 대기 요청에 따라 후보만 준비했다. 이후 사용자가 발탄 돌까지
전부 반영하고 빌드하도록 승인하고 EXE 종료를 알렸다. 통합 담당이 최신 저장본을 다시
읽어 아래 7문서를 원자적으로 설치했다. 설치된 source 대상 cross 6개와 rock-pillar 8개
계약 테스트가 통과했다. GameplayBalance/Composition 게시와 통합 Debug 제품 빌드도 통과했다.
Client/Server 실행과 UI 조작은 하지 않았다.

최종 적용 후보는 두 manifest의 합계7문서다.

- out/ValtanStoneProduct20260922/manifest.json: 기존 십자 Product1, persistent active3.
- out/ValtanStoneMask20260922/source_candidates/manifest.json: source library3. 이 후보의
  재질 교정·원본 GPU 대조는 통합 담당이 소유하며 아래 codec 검증만 이 작업에서 수행했다.

기존 Product stable ID와 현 runtime 연결을 유지한다. ground-roar, six-pizza, struggling의
active는 .600000024→.7200000288, cross4개는 기존 .4/.4/.7에1.2를 곱한다.
native2391의09.gap_offset=.25와 일치하는 authoring scalar override를 사용한다.
일반 연기·텔레그래프·폭발 파편 및 Server damage, coverRadius1.5m, 위치·지연·수명은 보존한다.
part-break는 ground-roar active를 공유하므로 같은 교정이 적용된다.

## G01. 실제 코드 변경

Client/Private/Effect_Playback.cpp의 Step에 명시된 고정 간격을 가진 portable SourceRecipe를
기존 Spawn_FixedCenterSpacingParticles로 연결했다. 동일 Spawn_Particles가 native size,
lifetime, dynamic parameter를 평가한다. SourceRecipe를 추가하면 기존 source 분기가
고정 간격을 건너뛰던 문제가 원인이었다. source burst/rate와 중복 생성하지 않는다.

Client/Private/Effect_DocumentCodec_Validation.cpp는 direct authored, world-space mesh와
위치·운동 모듈이 없는6종(required/spawn/lifetime/TypeDataMesh/size/dynamic)만 고정 간격
source carrier로 허용한다. 기존 zero rate/burst, zero offset/velocity/acceleration과 범위
검사는 보존한다. public header와 project/filter의 변경은 없다. 기존 dirty 변경은 보존했다.

Tools/EffectPipeline/stage_valtan_product_stones.py는 최종 source를 직접 쓰지 않는 후보
생성기다. 네 stable cross element의 material/resources/recipe와 크기만 교체하고 active3의
대상 돌 element만 gap/scale을 고친다. baseline SHA·원문 backup·후보 SHA를 기록한다.
cross generic base/mask override는 교체된 native material에 더 이상 존재하지 않는 slot이므로
해당 material 교체와 함께 제거한다. 연기4개의 source byte는 유지한다.

Tools/EffectPipeline/test_valtan_cross_rock_wave_effect.py의 기존 Product 계약은 native2391,
새 material/mesh, gap.25와 돌만1.2배인 관계를 검사하도록 맞췄다. 최초 후보 검사 후
사용자 승인으로 설치한 저장소 원문을 대상으로 다시 실행해 6개 모두 통과했다.

## G02. 실제 자동 검증

- 변경한 두 CPP를 out 분리 object로 compile하여 exit0. 기존 Engine CP949 헤더의 /utf-8
  warning이 있었으며 이번 파일의 인코딩은 UTF-8/기존 CRLF를 보존했다.
- 실제 CEffectDocumentCodec Load/Validate_Drawable와 CEffectPlayback의60Hz 재생:
  cross rootScale1 peak56,6,776 particle samples; 실제 Product cue의 rootScale1.5 peak80,
  9,680 samples. 기준본과 매 tick count·stable ID·생성 중심이 같고 돌의3축 길이만1.2배다.
- persistent 후보3개는5초/19.5초/5초의 lifetime·native dynamic parameter·companion
  count를 유지했다. 실제 돌 표본은300/1171/300이며 저작 수명 이후 대상 돌은0개다.
- source 후보3개는 실제 codec load/drawable/stage 및 Serialize→Parse→Serialize의 canonical
  왕복이 동일했다. native2391 표본은221/191/242다.
- local-space 고정 간격과 SourceRecipe 위치 module의 오염을 validator가 거부했다.
- 위 C++ probe 합계436,179검사 통과:
  out/ValtanStoneProduct20260922/product_probe.cpp / product_probe.log.
- 기존 cross 계약6개는 후보 경로에서 통과. rock-pillar Server/publisher 계약8개도 통과했다.
- 변경 파일 git diff --check 통과. 실제 화면의 형태·색·소멸은 사용자 검증 대기다.

첫 probe 실패는 out EXE가 Resources 루트를 찾지 못한 실행 준비 오류였다. 명시적인
LOSTARK_RESOURCE_ROOT=Client/Bin/Resources로 해결했으며 runtime 검증을 제거하지 않았다.
GPU gap 결과는 통합 담당의 별도 결과를 따른다. 원본 Dynamic.X와 natural lifetime을 유지했고
terminal 잔여 fragment를 없애기 위한 임의 alpha/색 module은 추가하지 않았다.

## G03. Product 게시 경계

십자 effect.valtan.sequence.cross는 Valtan.patterneffectcues의 직접 cue라 presentation
generation의 SHA 대상이다. 조사 당시146artifacts의 generation은
d2aca7336ddc43a42a73b08a1a410e1f64392b5f2b5dd050714b06ef288acf80이었고 cross 후보만으로
7bd856b1673bfe1d75c891e58e0ad208ec3bcc51f9998118eafb2cc821e5853f로 바뀌었다.
실제 적용 때는 최신 저장본·다른 변경에 따라 generation이 달라지므로 이 값을 고정하지 않는다.

저장본 적용 후 Publish-GameplayBalance.ps1 -Mode Publish로 Gameplay.bootstrap과
위 `7bd856b1...` generation manifest를 생성했다. Publish-Compositions.ps1 -Mode Publish도
완료했다. 두 로그는 `out/KoukuUrgentFix20260922/`의 `gameplay-publish.log`와
`composition-publish.log`다. 현재 writer는 pattern cue closure를 수집하므로
BossCatalog의 persistent active/explode만 있는6문서는 기존 generation hash 대상이 아니다.
이 사실을 publisher 지원 확대로 설명하지 않는다. 대상 7문서의 디스크 설치는 완료했다.

Engine/Shared/Server/Client Debug Product 빌드와 배포는 exit 0이며
`out/BuildPipeline/runs/20260922T011037720Z-debug-product.json`의 result는 PASS다.
missing/invalid runtime input은 0이다. 실제 사용하는 Server의 새 bootstrap 로드와
Client 재실행 뒤 화면 검증은 사용자 확인으로 남긴다.

## 2026-09-22 참고. 버러지와 발악의 구분

VALTAN_STRUGGLING은 '3페이즈 전 발악패턴'이다. 사용자 표현의 버러지는 실제 stable pattern
VALTAN_TRASH와 TRASH_CATCH_SUCCESS/FAIL/IF이며 서로 다른 패턴이다.

현재 저장본에서 네 TRASH 패턴의 gameplay에는 rock combat-object spawn이0개다. 전체 Valtan
rock spawn은 ground-roar, six-pizza, struggling, part-break의4archetype뿐이다.
TRASH Product V2 closure는 hand_1~6와 counter pulse group이고 WModel은
Effect/DimensionMaster/Meshes/fm_b_ring_001.wmodel 하나다. V1 catch effect는 sphere/helix,
map source-preview.trash의8effect track도 cast/light/helix를 사용하며 돌기둥 mesh는 없다.
현재 authored full.restore 문서에도 fm_d_stoneparts_003 직접 요소가 없다.

Server CCombatObjectRuntime은 기존 combat object를 자기 lifetime 동안 유지하고 source 사망·
소멸을 따로 처리한다. 다른 패턴에서 생긴 돌을 버러지 동안 볼 가능성은 있으나 실행 로그나
사용자 occurrence 식별 없이 그것이라고 확정하지 않는다. 현재 확정된4archetype의 shared
visual은 설치됐고, 별도 버러지 돌 generator는 발견되지 않았다. 사용자 화면에서 지칭하는
돌의 실제 occurrence가 식별되기 전까지 별도 버러지 돌 교체를 완료로 기록하지 않는다.
