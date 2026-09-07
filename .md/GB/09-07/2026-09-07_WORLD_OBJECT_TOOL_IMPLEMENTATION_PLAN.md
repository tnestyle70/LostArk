# 2026-09-07 World Object Tool 구현 계획서

## G00. 목표와 현재 경로

사용자가 요청한 카드·조커카드·공·세토·칼날·갈고리·빙고폭탄·빙고·커튼·룰렛을
World Object Resources로 정리하고, 물리 모델과 texture 선택, 동작 상태 저작, 패턴 재사용을 연결한다.
기존 Effect resource 파일은 모델·texture의 입력으로 재사용한다.

현재 World는 Composition의 world definition/occurrence가 Area WorldSequence instance를 참조한다.
제품 재생은 Server의 sequence 시작 → ClientReplication → Level → CWorldSequencePlayer를 사용한다.
커튼은 기존 11개 Map placement, 룰렛은 placement 40에 연결되어 있다. 두 대상의 stable ID와 키는 보존한다.

새 도구는 기존 World Sequence source와 sampler를 확장한다. 카드의 들썩임·뒤집힘은 이름 있는
template/state로 저장하고 instance가 대상 object resource를 binding한다. 물리 파일을 상태마다 복제하지 않는다.

## G01. 문서와 데이터 정본

정본은 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`이다.
새 formatVersion 3은 `objectResources`, `OBJECT_RESOURCE` binding, template motion과 instance anchor를 추가한다.
기존 v1/v2 reader와 Map/Deploy bindings를 유지한다.

Object resource는 stable objectId, displayName, Resources-relative model/diffuse texture, model 단위 보정,
animated 여부와 기본 scale을 소유한다. 기존 커튼·룰렛은 원래 sequence instance의 alias로 목록에 등록한다.
표시 이름은 `월드오브젝트_카드`, `월드오브젝트_조커카드`, `월드오브젝트_공`, `월드오브젝트_세토`,
`월드오브젝트_칼날`, `월드오브젝트_갈고리`, `월드오브젝트_빙고폭탄`, `월드오브젝트_빙고`,
`월드오브젝트_커튼`, `월드오브젝트_룰렛`을 사용한다.

State는 기존 Transform key/animation track과 duration을 사용한다. motion은 속도·가속도·자전·공전,
생성 개수·간격·방향 분산·seed를 소유한다. 기본 생성 개수는 1이며 seed와 시각으로 샘플링해 seek와 재생을 맞춘다.
Instance는 WORLD/PLAYER anchor와 시작 위치를 소유한다. PLAYER transform은 ClientReplication의 읽기 전용 view다.

Load/Save/Publish는 기존 문서 검증과 원자적 저장을 확장한다. 외부 source 변경, 잘못된 경로·숫자·ID·binding은
기존 정상 문서를 보존한다. 물리 자산은 실제 패키지·material 근거로 확인하고 필요한 기존 cooker로 준비한다.
Resources binary는 Git에 추가하지 않는다.

## G02. World Object Tool

F1의 World 범주에서 `World Object Tool`을 연다. 화면 상단은 저장 Object Resources와 상태 목록,
가운데는 Object Detail·부품 slot·동작 timeline, 하단은 실제 Physical Resources 폴더 목록이다.
등록된 Effect 항목으로만 제한하지 않고 실제 상대 경로별 모델·texture를 표시한다.

타임라인은 `CompositionTimeline.h`의 ruler/box/gesture를 재사용한다. 키 이동·Transform·rotation·scale·visibility,
상태 duration, 속도·공전·복수 생성과 Play/Pause/Stop/Seek를 동일 source에 연결한다.
커튼·룰렛을 선택하면 원래 template의 tracks를 편집하므로 또 다른 애니메이션 사본을 만들지 않는다.

## G03. 실행과 패턴 연결

CWorldSequencePlayer가 OBJECT_RESOURCE binding을 기존 GameObject Prototype/Clone 경로로 생성한다.
새 CWorldSequenceObject는 CModel/CMaterial을 렌더링하는 표현 객체이며 시간·배치·동작 권한은 Player에 둔다.
생성/seek/종료/Level 퇴장과 실패 시 정리는 같은 Player가 수행한다. 모델 prototype은 공유하고 움직이는 clone만 둔다.

World Object Tool preview도 Level의 같은 Player 경로를 사용한다. 제품 sequence와 충돌하는 preview는 거부한다.
Action Workbench World 목록에 실제 sequence/state 목록을 공급하고 Append한다.
World box의 lifetime을 제품 호출과 preview 종료까지 연결한다. Effect는 기존 World anchor 조회 경계로 연결한다.
게임플레이 충돌·피해 수치를 object 렌더러에서 판정하지 않는다.

## G04. 파일과 등록

| 구분 | 경로 | 책임 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/WorldSequenceDocument.h` 및 Private CPP | v3 resource/motion/anchor, 검증·저장 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/WorldSequencePlayer.h` 및 Private CPP | dynamic object 생성·샘플·수명·pivot |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/WorldSequenceObject.h` 및 Private CPP | 기존 CModel의 물리 모델·texture 표현 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/WorldObjectTool.h` 및 Private CPP | Object/상태/slot/timeline/폴더 저작 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/PhysicalResourceCatalog.h` 및 Private CPP | Effect/World Object 공용 물리 폴더 검색 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/WorldSequencePlayer_Objects.cpp` | 기존 Player의 모델 준비·복수 생성·샘플·정리 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_KakulSaydonArena_WorldObjects.cpp` | Level의 World Object preview와 typed target view |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MainApp.h` 및 Private CPP | 도구 ownership과 F1 연결 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_KakulSaydonArena.h` 및 Private CPP | typed target view, preview·runtime reload |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp` 및 대응 H | 저장 Object/state 목록·Append |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/MapPipeline/Publish-MapAuthoring.ps1` 및 관련 validator | v3 publish |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | World box 수명·정본 join |

World box lifetime의 기존 Shared/Server/Client typed 전송도 같은 변경에서 연결한다.
`iDurationMs`를 WORLD cue에 추가하며 protocol 63을 사용한다. 0인 기존 직접 호출은 authored duration을 유지한다.
diffuse override는 DDS 로드 성공을 검사한 SRV를 기존 공용 CModel material 바인딩에 전달한다.
OBJECT_RESOURCE는 단일 상태에서 최대 128개, Player 전체에서 최대 1,024개로 제한하고 초과는 실패로 정리한다.
기존 Effect WORLD anchor는 살아 있는 단일 object pose를 소비하며 복수 입자별 effect 이벤트 저작은 이번 범위가 아니다.
새 H/CPP는 Client.vcxproj와 기존 물리 폴더에 대응하는 filters에 등록한다. 기존 sequence JSON의 None 등록을 재사용한다.
public 계약 변경은 CLAUDE와 팀 Animation/Area 사용서에 반영하고 검증 이력은 대응 RESULT에 기록한다.

## G05. 검증과 종료 기준

변경한 sequence/Composition publisher 검사, 정상 저장 왕복과 실패 보존, 실제 resource 경로 확인,
관련 기존 Python 검사, 필요한 native 검사와 Debug Product 빌드를 수행한다. JSON/XML parse와 diff check를 확인한다.
무관한 광역 진단을 기본 조건으로 추가하지 않는다.

사용자는 Server + Client profile의 Ctrl+F5로 시작하고 Lobby → KoukuSaydon → F1 → World Object Tool에서
목록·slot·키·저장·재로드와 아레나 재생을 확인한다. 에이전트는 Client/UI 실행·캡처와 최종 화면 판정을 하지 않는다.
