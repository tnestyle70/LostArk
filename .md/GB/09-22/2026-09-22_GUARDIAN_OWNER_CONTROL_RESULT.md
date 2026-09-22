# 가디언 나이트 owner control 편집·재생 결과

## 현재 완료 상태

원본 PawnMaterialParam 42개와 DominantDirectionalLight_Control 6개를 원본 payload에서 다시 읽고, 활성 재질 제어 23개와 조명 밝기 제어 5개를 기존 Effect 문서와 재생 경로에 연결했다. IdentityParts 9개와 HidePawn 3개도 같은 owner control 계약을 소비한다. 총 40개 제어가 24개 stage 문서에 들어갔으며, 기존 43개 Effect 문서에 제어 전용 문서 1개를 추가해 총 44개다.

Effect Editor에서 stable ID로 제어를 선택·추가·삭제하고 시작 시간, 키 시간·값, 대상과 종류를 편집해 기존 Save로 저장한다. All Effects와 Character Workbench의 선택 캐릭터 미리보기 및 실제 승인된 Character action은 같은 CEffectObject·CEffectPlayback clock을 사용한다. 파티클이 없는 제어 전용 문서도 정상 문서로 로드·재생하며 가짜 emitter를 넣지 않았다.

이 문서는 owner control의 원본 투영, 수신 객체, 편집 저장, 취소·원복과 실제 headless 검증 결과를 소유한다. 통합 빌드, 원본 작업 폴더 반영, Git 전달 및 GBResources 패키지는 [통합 결과](2026-09-22_ANCIENT_SEA_GUARDIAN_INTEGRATION_RESULT.md)를 따른다. Client와 아레나 UI는 실행하지 않았다.

## 저장·편집 계약

Effect 문서의 optional `ownerControls`는 `controlId`, `kind`, `parameter`, `mappingBasis`, `sourceTargetType`, `onlyLocalPlayer`, `startSeconds`, `keys[{seconds,value}]`, optional `sourceValues`를 저장한다. `mappingBasis`는 현재 `PROJECT_ADAPTER`다. 원본 수치는 read-only SourceValues로 보존하고 저작 키와 구분한다.

| kind | parameter·수신 대상 | 소비 |
|---|---|---|
| `MATERIAL_VECTOR` | `transcolor` 또는 `buffcolor`; 원본 mesh group 0~4 | 실제 body·equipment·weapon의 지원 native material 상수 |
| `DIRECTIONAL_BRIGHTNESS` | 빈 parameter | 기존 scene directional light의 일시 밝기 배율 |
| `DIRECTIONAL_COLOR` | 빈 parameter | 기존 scene directional diffuse/specular의 색과 0~1 가중치 |
| `IDENTITY_VISIBILITY` | `identity`, 원본 target 12 | stance 소유 장비의 일시 visibility overlay |
| `PAWN_VISIBILITY` | `visibility`, target 0 또는 9 | 전체 pawn 또는 실제 weapon part의 일시 visibility overlay |

한 문서는 최대 64개 제어를 가진다. ID 중복, 알 수 없는 kind·parameter·target, nonfinite 값, 정렬되지 않거나 중복된 키 시간, 범위 밖 시간·값은 parse/validate에서 거부한다. 키는 2~128개이며 첫 키는 0초, 뒤 키는 엄격히 증가한다. 조명 밝기는 0~16이고 조명 색 가중치는 0~1이다. 일반 authoring v13/v15가 optional 필드를 보존하며, v14의 엄격한 원본 element 계약을 제어 전용 문서의 우회로로 느슨하게 바꾸지 않았다.

문서 길이는 element/model cue뿐 아니라 `startSeconds + 마지막 key.seconds`를 포함한다. Component split/assembly도 제어 배열을 보존한다. 편집은 기존 `Try_CommitDocument`의 parse→validate→stage→commit과 dirty/Save 경로를 사용한다. 잘못된 교체가 현재 재생 문서와 owner 상태를 지우지 않는 것은 실제 CEffectObject로 확인했다.

추가 문서는 `effect.guardianknight.skill.49330.clip.1.full.restore`이며 실제 clip `ddk_sk_deepimpact_02`에 연결했다. EffectCatalog, All Effects 인덱스와 animevents startup cue를 함께 등록했다. 최신 문서의 다른 element·model cue·sound 변경은 유지했다.

## 원본 투영과 명시한 해석 범위

`Tools/EffectPipeline/project_owner_control_notifies.py`는 원본 payload의 base64, byteSize, SHA256, class signature, 공통 header, FString 및 typed field 범위를 검증한다. 끝 위치도 다음 notify length 직전과 일치해야 한다. 원본 field와 원본 SHA는 receipt에 남겼다.

원본 42개 Pawn 항목은 활성 23개, 비활성 13개, 이름과 곡선이 비어 있는 no-op 6개로 나뉜다. 6개 Directional 항목은 활성 밝기 5개와 비활성 1개다. 활성 원본 색 제어는 없으므로 `DIRECTIONAL_COLOR`의 편집·소비 지원을 원본 색 cue 복원 개수로 세지 않는다.

공통 header의 signature 이후 +12는 기존 PlayParticleEffect decoder의 enabled offset과 같고 EFAction_Notify reflection의 Enable 계약과 대조했다. bool 0/1만 허용한다. 원본 native serializer를 실행해 이 offset을 검증한 것은 아니며, 기존 decoder와 원본 reflection에 따른 해석이라는 근거를 유지한다.

PawnMaterialParam의 start curve→원본 life hold→end curve는 현재 엔진 clock에 맞춘 project adapter다. Directional의 음수 Brightness는 원본 값을 보존하면서 `1 + 원본 Brightness`를 unit baseline의 배율로 투영했다. 원본 CDO와 field를 확인했지만 retail 실행 코드의 additive/multiplicative 식까지 확보하지 못했으므로 이 수식을 native ABI와 동일하다고 기록하지 않는다. 조명 fade/hold/release와 visibility 시간도 원본 값을 사용하되 제품의 임시 owner 제어로 연결한 범위를 명시한다.

HidePawn 원본 2개에 있는 status-effect-FX 숨김 flag는 raw receipt에 보존한다. 현재 제품에는 buff/status FX를 별도로 식별하는 owner 분류가 없어 해당 flag만으로 status FX를 숨기는 소비자는 추가하지 않았다. 일반 스킬 효과 전체를 함께 숨기지 않는다.

## 실제 owner와 수명

CEffectObject는 문서를 stage하기 전에 명시적인 weak Character owner와 승인 action 시작 tick을 받는다. 일반 gameplay 효과는 EffectPresentationService가 실제 owner를 전달하고, NPC·boss 효과의 비어 있는 명시적 Character owner를 전역 미리보기 대상으로 바꾸지 않는다. 독립 Effect Editor 미리보기만 AnimationTargetService의 실제 선택 Character를 사용한다.

각 EffectObject는 고유한 transient token을 가진다. Character는 `[시작 키, 마지막 키)`의 현재 샘플을 token별로 보관하고 남은 제어를 다시 합성한다. 다른 효과 하나가 끝나도 살아 있는 제어가 사라지지 않는다. 다음 경우 해당 token을 해제하고 남은 제어 또는 기존 상태를 복원한다.

- 마지막 키 도달, Reset, EffectObject 소멸
- owner 선택 변경 또는 weak owner 소멸
- 효과 숨김 또는 재생 실패
- gameplay 승인 action tick 변경·취소

NATURAL particle의 잔여 수명은 유지하면서 취소된 action의 재질·조명·visibility만 해제한다. 외부 clock seek도 즉시 같은 샘플을 소비한다. 미리보기는 관련 없는 live action tick에 의해 취소되지 않는다.

Visibility는 사용자 visibility와 독립적인 suppression 상태다. weapon 여부는 socket 추정 대신 CharacterSpec과 장비 primary slot에서 정한 `Is_WeaponPart()`를 사용한다. Body와 equipment의 일반 render·translucent·shadow·afterimage 경로가 같은 suppression을 소비한다. identity 제어를 제거하면 기존 stance에 따른 visibility가 돌아온다.

## 재질과 scene light 원복

재질 대상은 실제 CPart_Body/CPart_Equipment의 CModel이다. mesh group 3은 body, 4는 전체, 1은 실제 weapon, 0은 non-weapon equipment, 2는 equipment와 weapon을 선택한다. 현재 숨긴 equipment는 건드리지 않는다. 툴이 part tag나 socket 이름으로 수신 대상을 추측하지 않는다.

`SourceCharacterMaterial::Patch_NamedVector`는 설치된 native program의 정확한 TransColor/BuffColor register binding만 수정한다. generator는 기존 packing식을 읽어 지원 binding을 생성하고 지원하지 않는 program·parameter는 false로 반환해 기존 상수를 유지한다. 현재 61개 family에서 116개 program/parameter 조합을 지원한다. program 80~83의 공통 후속 register도 포함한다. AUTO/program 0 및 map program 33~79에는 임의 매핑을 만들지 않는다.

처음 제어가 적용될 때 각 모델·재질의 실제 기존 상수 전체를 보관한다. 매 합성 전에 그 baseline을 복원하고 살아 있는 token을 다시 적용하므로 현재 customization·다른 상수·native texture binding을 유지한다. CModel의 material copy-on-write는 수정하는 owner의 CMaterial만 분리하고 geometry와 native texture는 공유한다. peer clone과 prototype의 상수는 변하지 않는다.

MainApp은 활성 미리보기 owner 또는 local Character의 조명 제어를 기존 RenderingProfileService에 전달한다. 서비스는 이전 presentation override를 원래 scene light로 돌린 뒤 camera region과 이번 샘플을 적용한다. diffuse/specular만 일시 색·배율을 받고 ambient와 저작 profile은 유지한다. Character의 겹침·해제 상태는 실제 검사했고, 전체 scene light transaction과 camera-region 복원 연결은 코드 소비 경로를 검토했다. Client/GameInstance 전체를 초기화한 실제 scene 복원·화면 판정까지 수행한 것으로 표현하지 않는다.

## 기본 스택에서 확인한 native packing 결함

첫 실제 cold ActorCatalog 검증에서 기본 1MB 스택이 `0xC00000FD`로 넘쳤다. stack trace는 `__chkstk → SourceCharacterMaterial::Configure → ParseModelMaterialOverrides → ParseCharacters → CActorCatalog::Initialize`였다. 생성된 Configure가 모든 61개 family의 debug 임시 변수를 한 함수 frame에 쌓는 구조였다.

각 family의 packing block을 즉시 호출 lambda로 분리하고 정본 generator도 같은 형태를 출력하게 했다. `/STACK`을 늘리지 않았다. 원본 packing 수식·값·parameter 이름은 바꾸지 않았다. 독립 비교에서 61개 family의 wrapper를 제거한 body bytes가 모두 같았으며 Guardian 101~108의 generator/installed 결과도 8개 모두 일치했다.

이어 실제 C++에서 61개 Configure와 116개 named vector 변경을 전체 재packing 결과와 비교했다. 기본 스택 1,048,576byte에서 734개 검사, 실패 0개다. generator의 direct/copy/common/negative 조건 6개도 통과했다. 이 결함은 probe만의 stack 설정으로 숨기지 않고 제품 호출 경로와 생성기에 함께 수정했다.

## 실제 실행한 검증

최종 headless owner probe는 기존 Product의 실제 object 332개와 최신 Character object를 링크했다. 실제 CCharacter·CEffectObject·CModel·CMaterial, 설치 ActorCatalog, D3D11 WARP를 사용하며 Client/UI와 full scene 초기화는 하지 않았다. 데이터는 통합 worktree의 최신 Data, 리소스는 기존 자산까지 완전하게 있는 Desktop Resources를 사용했다. 다음 45개 검사가 모두 통과했다.

- local-only와 선택 preview owner 분리, 조명 token 중첩·개별 해제·마지막 baseline 복원
- 실제 weapon/identity/body 구분, 사용자 숨김 상태 보존, 마지막 키에서 transient visibility 복원
- 실제 EffectObject 문서 stage, 공통 clock, owner 전환, hide/show, action 취소, 소멸 후 원복
- 실제 Guardian body native hair 99와 weapon 96의 TransColor 변경, 무관한 tuned light 상수 보존
- CModel prototype/다른 clone 불변, 정확한 pre-cue 상수 복원, 숨긴 weapon 보존
- renderer resource staging을 포함한 제어 전용 문서 재생, duplicate key가 있는 교체 실패 시 기존 stage·owner 상태 유지

실제 body는 hair 한 슬롯만 native 99이고 나머지 다섯 슬롯은 AUTO/program 0이었다. 이전 probe의 무관한 light 상수 비교 실패는 AUTO 슬롯에도 source override가 성공했다고 가정한 fixture 오류였다. 실제 commit된 baseline을 기록하도록 바로잡았고 최종 45개 검사는 그 baseline으로 비교했다.

Guardian 전체 44개 문서는 실제 C++ codec·playback canonical roundtrip을 통과했다. 1,608 elements, 23 model cues, 40 owner controls와 실제 source parameter sample 66개를 검증했다. 제어 전용 fake carrier는 0개이고 잘못된 owner mutation은 거부됐다. All Effects의 20스킬, 실제 model clip 165개, 연결된 animation effect 44개에서 unresolved 항목은 0개다. UI·clock·admission·assembly 7TU syntax, Component split/compile 30개 검증도 통과했다.

검증 자료는 Git 제외 중간 산출물에 있다.

| 위치 | 내용 |
|---|---|
| `out/SeaLifetime20260922/owner_consumer_probe.cpp`, `owner-consumer-result.log` | 실제 owner 경로 45개 PASS |
| `out/SeaLifetime20260922/owner_probe_compile.rsp`, `owner_probe_link.rsp`, `owner_probe_build.cmd` | 실제 compile/link 재현 입력 |
| `out/SeaLifetime20260922/guardian-owner-controls.patch.json`, `guardian-owner-controls.receipt.json`, `gk-controls.decoded.json` | 48개 원본 제어의 decode·투영·제외 근거 |
| `out/GuardianEffects20260922/owner-install/installed.receipt.json` | 24개 stage 제어 병합과 44개 실제 문서 검증 |
| `out/GuardianEffects20260922/stack-packing-audit/independent-packing-comparison.json` | 61개 family packing body 보존 |
| `out/GuardianEffects20260922/stack-packing-audit/named-vector-result.log`, `named-vector-preservation.receipt.json` | 116개 binding·734개 실제 C++ 검사 및 generator 6개 검사 |

최종 owner probe EXE SHA256은 `41c85a125c1d0a97bec46f961555333e1b3dec0c2e51972408ef43e5bbeab758`, 사용한 Engine.dll SHA256은 `63e034868a26b07a73c19dc65d325dbc81a00f529ffc8b4108f4922cf71bd136`이다. 이 파일은 headless 검증 산출물이며 제품 패키지로 넣지 않는다. 최종 제품 빌드 receipt는 통합 결과에서 구분한다.

## 남은 원본 재질 경계

Guardian body의 `pc_dl_av_018a_upper1_mi`, `pc_dl_av_018a_upper_mi`, `pc_dl_face_mi_high`, `pc_dl_eyeao_mi`, `pc_dl_eye_mi` 다섯 슬롯은 기존 AUTO 경로다. 이번 owner control은 이 슬롯에 임의 native shader를 지정하지 않았다. 원본 Pawn ALL의 색 변화가 이 다섯 슬롯까지 모두 재현됐다는 의미로 완료를 기록하지 않는다.

이 상태는 [#437 재질 복원 결과](../../JS/09-21/2026-09-21_GUARDIANKNIGHT_RESTORE_SYNC_RESULT.md)에 이미 기록되어 있다. 눈 program 5는 원본 shader가 일치하지만 기존 GK WModel 1.0에 필요한 UV1/UV2가 없어 실제 class admission 실패 뒤 override를 제거했다. EyeAO는 generator IndexError, face는 원본 MIC tail parser 미지원이다. 실제 visible skin·갑옷·무기에는 별도 native equipment 경로가 있고 이번 제어는 그 지원 binding과 실제 native hair·weapon을 소비한다. 이 다섯 AUTO 슬롯의 재쿠킹·원본 parser 복구와 최종 사용자 화면 확인은 이번 headless owner 소비 검증과 구분한다.
