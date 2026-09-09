# 마리오 폭탄·문양 공·삐에로 배치 설계

작성일: 2026-09-09

> 후속 조사 정정: [노란·파란 공 원본 추적 결과](2026-09-09_MARIO_BALL_RESOURCE_TRACE_RESULT.md)에서 공통 `MN_PPCC_00_SK`와 노란/파란 전용 재질·DDS를 확인했다. 아래 G01의 미확정 표는 최초 조사 시점 기록이다. 이 두 공의 구현은 Effect-only 대체 외형이 아니라 확인된 메시+재질 variant를 우선한다. 기존 `fm_k_ppct_ball_01.wmodel`과 위치/UV/삼각형 구성도 좌표 정규화 후 일치하지만 skeletal animation은 보존되어 있지 않다. 다른 대상의 미확정 상태나 stage 배치/서버 연결 미구현 상태를 완료로 바꾸는 것은 아니다.

## 현재 실제 반영 상태와 설계 경계

사용자 요청은 스크린샷의 네 대상을 MapTool에서 배치하고 마리오 입장 직후 보이게 만드는 **설계**다. 이번 문서는 현재 코드 조사에 기반한 기능·데이터 소유권 설계이며, 그대로 복사할 C++ 구현 PLAN이나 구현 완료 보고가 아니다. 실제 소스·게임 데이터·Resources는 변경하지 않았다.

조사 기준: `ffca5286`, 작업 브랜치 `codex/kouku-card-maze-0908`, WorldSequence v3/revision 412. 시작 시 존재한 사용자 미커밋 시퀀스 뷰어·쿠크 패턴 변경은 보존했다. 최신 World Object의 공 Motion/Effect 연결을 기준으로 한다. 예전 결과서의 공 모델이나 protocol 번호를 현재 값으로 간주하지 않는다.

목표는 **외형을 한 번 등록하고, MapTool에서 종류와 위치를 저장하면, 해당 마리오 진행에 맞춰 서버가 생성·파괴를 관리하는 배치형 게임 오브젝트**다. 사용자가 매번 Effect Tool에서 Play를 누르거나 입장 때마다 직접 시퀀스를 재생하지 않는다.

## G01. 리소스: 무엇을 확인했고 무엇을 확정하지 못했는가

스크린샷에서 보이는 것은 해골 폭탄, 노란 역삼각형 표적, 작은 삐에로, 파란 다이아 표적이다. 이것만으로 원본이 StaticMesh인지 particle mesh인지 skeletal actor인지는 확정할 수 없다.

| 대상 | 현재 로컬에서 확인한 근거 | 판정 |
|---|---|---|
| 해골 폭탄 | `Effect/KoukuSaydon/Textures/FX_TEX_06/fx_x_symbol_007_1_cl_loc_int.dds`를 열어 해골·파란 테두리·붉은 몸체·도화선 이미지 확인 | 강한 외형 후보. 원본 마리오 actor→particle/material 연결은 아직 미확정 |
| 다른 폭탄 모델 | World Object의 `world.object.kouku.bingo_bomb`, `Character/KoukuSaton/MN_RHCN_01/MN_RHCN_01.wmodel` | 기존 빙고 폭탄이다. 마리오 폭탄 정답으로 대체하지 않음 |
| 노란·파란 표적 | 삼각형 마스크 `FX_TEX_06/fx_x_symbol_017_1_cl.dds`, 흰 다이아 `FX_TEX_05/fx_l_symbol_41.dds` 확인 | 문양 구성에 쓸 수 있는 후보일 뿐 완성된 공의 원본 재질이라고 확정하지 않음 |
| 기존 월드오브젝트 공 | `world.object.kouku.ball`은 `Effect/KoukuSaydon/Meshes/wp_mn_rhcn_00/mesh/fm_d_rhcn_00.wmodel`와 `Textures/MN_RHCN_00/tex/mn_rhcn_00_d.dds` 사용 | 얼굴 무늬 공이다. 스크린샷의 두 문양 표적과 동일하다고 주장하지 않음 |
| 작은 삐에로 | 현재 `MonsterCatalog.json` 9종은 발탄/Lugaru 5종과 카드미로 문양 병사 4종 | 마리오 적으로 식별·등록된 행은 없음. 플레이어 변신 모델 또는 세토를 정답으로 지정하지 않음 |
| 노란 연기 | `boss.kouku.ball.smoke.yellow_1.effectv2.json`의 texture는 `fx_a_cloud_020_cl.dds` | 노란 공 본체가 아니라 연기. 이름이 yellow라는 이유로 재사용하지 않음 |

표의 `Effect/...`, `Character/...`는 모두 `Client/Bin/Resources` 상대 경로다. 문양 두 마스크의 전체 폴더는 `Effect/KoukuSaydon/Textures/` 아래다. DDS가 있다는 사실과 제품 Effect 문서에 등록되어 있다는 사실을 구분한다. 확인한 세 문양/폭탄 파일명은 조사한 `Data`, `Tools`, `.md/GB` 텍스트 검색에서 제품 연결을 찾지 못했다. 이는 모든 원본 패키지에 연결이 없다는 증거는 아니다.

48개의 Kouku `*symbol*.dds`를 오프라인으로 열람했다. 과거 Resource Ledger가 가리키는 `C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/WorldObjectExtraction-20260907`은 현재 PC에 없어 원본 DB 연결을 재검증하지 못했다. 바탕화면의 추가 원본 폴더는 존재하지만 이번 조사에서 네 대상의 Npc→LookInfo/Action→Particle→Material 추적 사슬을 완성하지 않았다. 따라서 **네 개 모두 찾았다/전부 이펙트다/텍스처가 없다** 중 어느 것도 결론으로 삼지 않는다.

원본 식별 작업은 정확한 NPC 또는 Prop 행에서 LookInfo, Action의 지속/피격/사망 effect, particle의 Required material 및 TypeDataMesh를 따라간다. 검색 archive에 있는 기존 추출물을 먼저 재사용한다. 정확히 확인한 소수 메시·재질·텍스처·clip만 가져오며 전체 패키지를 다시 추출하거나 Resources에 DB를 복사하지 않는다.

원본 연결이 확보되면 그 표현을 등록한다. 연결이 확보되지 않았는데 빨리 배치 테스트가 필요하면 후보 텍스처/마스크를 이용한 프로젝트 제작 외형을 **별도 승인 후** 사용한다. 원본 복원으로 표시하지 않으며, 나중에 외형 참조만 교체해 배치 ID·좌표·판정은 보존한다.

## G02. 현재 프레임워크에서 재사용할 것

| 실제 파일·진입점 | 현재 책임 | 이번 기능에서 필요한 확장 |
|---|---|---|
| `Client/Public/WorldSequenceDocument.h`, `Private/WorldSequenceDocument.cpp::Validate` | `objectResources`, 기본 Motion, animation/effectTracks, stable instance ID | 모델 없는 V2 표현도 명시적으로 정의할 수 있는 구분. 현재 모델 리소스의 빈 model 경로를 그냥 허용하지 않음 |
| `Client/Private/WorldSequencePlayer.cpp::Load_Area` | 문서 로드·검증·캐시 정리. 모든 enabled instance를 자동 재생하지 않음 | 게임 배치 상태가 요구하는 정확한 occurrence만 재생 |
| `Client/Private/WorldSequencePlayer_Objects.cpp` | CWorldSequenceObject/CModel 생성, 기존 V2 Group/Leaf 재생 | 같은 시각 재생 경로에서 외형 시작·상태 변경·정리 연결 |
| `Client/Public/MapEffectDocument.h`, `Private/MapEffectPresentationRuntime.cpp` | V1 EffectCatalog 기반 LEVEL_ACTIVE/LOCAL_LOOP 월드 효과 | 상시 이펙트라는 개념은 이미 있음. 그러나 쿠크의 네 gameplay 객체를 이 V1 레이어에 중복 배치하지 않음 |
| `Data/Maps/MapCatalog.json` | Area optional layer 선언 | 쿠크에는 현재 sourceEffects/effects pair가 없음. 발탄 mapeffects 파일만 있는 상태를 쿠크 지원 완료로 오인하지 않음 |
| `Client/Private/MapTool.cpp::Render_WorldGameplayPanel` | Area gameplay 배치 편집 | 같은 패널 안에 Mario Objects 목록·배치·상태 미리보기 추가 |
| `Client/Public/WorldGameplayDocument.h` | Gameplay placement kind와 저장 계약 | 현재 enum에 마리오 오브젝트 kind가 없으므로 명시적인 확장 필요 |
| `Server/Private/GameRoom.cpp::Update_MarioControlState` | 서버가 Mario1..4 Intro, stage, 레일 이동 상태 확정 | 동일한 stage 진입·퇴장에 배치 객체 수명 연결 |
| `Client/Private/PlayerController.cpp::Update_MarioControls` 및 입력 early return | 마리오 좌우·Debug 위키 점프, 일반 입력 차단 | 마리오 공격 입력을 별도 허용. 일반 마우스 이동을 다시 열지 않음 |
| `Server/Private/GameRoom.cpp` skill/interaction 처리 | 일반 class skill은 Mario에서 거부, 별도 interaction slot 경로 존재 | 기존 typed interaction command를 재사용해 마리오 공격 상태·유효 타격을 서버에서 처리 |
| `Data/Actors/MonsterCatalog.json`, `Data/Balance/MonsterProfiles.json`, `CSpawnGroupRuntime` | 실제 몬스터 정의·수치·Server spawn | 적 삐에로는 기존 몬스터 entity 경로를 사용. 보이는 장식/일반 NPC로 위장하지 않음 |

파일 경로의 기준 저장소는 `C:/Users/USER/source/졸업팀폴/LostArk`다. 위 표는 수정 책임을 정하는 설계이며, 아직 추가하지 않은 필드/함수가 현재 존재한다는 뜻이 아니다.

### 외형과 게임 판정 분리

추가 비교: `Server/Public/EncounterPropRuntime.h`의 발탄 반복 기둥은 이미 epoch/occurrence·복사 후 commit·개별 slot 파괴를 제공한다. 그러나 현재 publisher는 Deploy placement의 XZ와 cover radius를 읽고, runtime은 HIDDEN/파괴 기둥 수명을 다룬다. 이를 이름만 바꿔 마리오 높이·문양 목표·경고 폭발까지 지원한다고 간주하지 않는다. transaction/중복 방지 원리는 재사용하되 발탄의 Deploy 연결을 마리오 Effect-only 대상에 강제하지 않는다.

폭탄 불꽃이 particle여도 게임 객체는 위치와 상태를 계속 가진다. 외형은 기존 EffectV2 runtime으로 재생하고, 서버에는 반경·높이·활성 여부가 있는 판정 대상이 존재한다. 애니메이션이 끝났다는 Client 통보로 서버 폭발이나 파괴를 확정하지 않는다.

적 삐에로가 skeletal model로 확인되면 기존 Monster presentation으로 생성한다. 그 몸체를 동시에 WorldSequenceObject로 생성하지 않는다. WorldSequence는 필요한 동반 효과나 모션 표현만 맡는다. Effect-only 표적 역시 보이지 않는 가짜 모델을 필수로 넣지 않는다.

## G03. 저장: 재사용 정의와 맵 배치를 분리

### 외형 정의의 소유권

World Object 라이브러리의 기존 `objectResources`를 재사용한다. 모델 외형은 현재 Model/Material과 Motion 참조, 순수 이펙트 외형은 기존 V2 Group/Leaf stable ID를 참조하도록 명시적으로 확장한다. 모델+동반 효과는 기존 animation/effectTracks를 재사용한다. 새로운 particle renderer나 두 번째 애니메이션 재생기를 만들지 않는다.

예상 UI 이름은 `Mario Bomb`, `Mario Yellow Target`, `Mario Blue Target`, `Mario Clown`이다. 이는 제안 표시 이름이며 이미 존재하는 asset ID가 아니다. 각각 한국어 별칭을 함께 저장한다. 정확한 native clip이나 원본 particle ID는 식별 전 임의 지정하지 않는다.

순수 효과는 Idle, Hit, Destroy/Explosion 표현을 구분하고, Idle은 owner가 살아 있는 동안 유지한다. finite leaf를 무조건 길게 늘리거나 매 프레임 Play해서 particle을 누적시키지 않는다. 정의된 loop/envelope와 handle 수명을 기존 V2 방식으로 소비한다. 모델이 필요 없는 분기는 모델 캐시 접근도 생략해야 한다.

### 게임 배치의 소유권

폭탄·문양 표적의 게임 배치는 `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`을 확장해 저장한다. 적 삐에로의 위치·archetype·생성 개수는 **기존 `Data/Worlds/LV_LUT_MIDNIGHTC_ED/SpawnGroups.world.json`의 anchor/wave/entry가 정본**이며 Gameplay에는 stage와 spawnGroupId 연결만 저장한다. 현재 World publisher도 이 SpawnGroups 문서를 몬스터 bootstrap으로 변환한다. MapTool은 네 종류를 한 목록에서 보여 주되 저장은 각 기존 소유자에게 전달한다. 같은 적의 좌표를 Gameplay와 SpawnGroups 양쪽에 쓰거나 `.mapplacements`·WorldSequence instance에 이중 저장하지 않는다.

추가할 배치 정보의 의미:

- stable placement ID, 영어 이름 및 한국어 설명, enabled.
- Mario stage 1..4와 구간/레일 식별자. 같은 Area 안의 구간이지 새 Level/Area가 아님.
- UI 대상 종류: 폭탄 / 문양 표적 / 적 삐에로. 적 행은 SpawnGroups의 stable entry/anchor를 보여 주는 통합 편집 행이며 새로운 Gameplay monster kind가 아니다.
- 위치·회전·외형 크기. 판정 반경·높이는 별도 값으로 표시.
- 폭탄·표적은 모델/Effect 외형 정의의 stable 참조. 적의 monster archetype 참조와 위치는 SpawnGroups entry/anchor에서만 편집.
- 초기 상태와 stage 진입 활성 정책, 재시작 초기화 정책.
- 표적의 색·문양 의미 ID, 파괴 조건. 색 텍스처 이름을 게임 규칙 ID로 쓰지 않음.
- 폭탄의 경고·폭발 시간 및 피해 profile 참조, 적의 행동/profile 참조.

이 항목들은 **추가할 계약**이다. 현재 JSON에 임의 키를 손으로 넣어도 동작하지 않는다. Client parser/save, publisher, Server bootstrap parser, Shared state, Client consumer를 같은 구현 단위로 연결해야 한다. 현행 파일을 읽는 호환성은 유지하고, 미지원 새 데이터는 명확히 거부한다.

Server 생성물에는 gameplay ID·좌표·수치·stage만 내보내고 model/texture/clip 경로를 보내지 않는다. 폭탄·표적의 Client용 시각 매핑은 같은 source에서 분리하여 publish하고, 적은 기존 MonsterCatalog presentation 참조를 사용한다. Server entity state의 stable identity로 정확한 외형을 찾는다. 저장 ID는 배열 index나 Prototype tag가 아니다. Save All은 Gameplay의 group 참조와 SpawnGroups의 행을 함께 검증하고 저장·publish 실패 시 기존 유효 runtime 묶음을 유지한다.

## G04. 입장하자마자 보이는 수명 계약

제안 정책: 방 안에서 stage별 진행 인스턴스(stage run) 하나를 두고, 동일 stage에 추가로 들어온 플레이어는 진행 중 상태를 공유한다. 아무도 없는 stage의 첫 입장 때만 초기화한다. 이는 원작 규칙을 추출한 결과가 아니라 재입장·다인 테스트를 위한 프로젝트 설계다.

1. 쿠크 Area 로드 때 필요한 외형 정의와 종속 리소스를 준비한다. 다른 모든 이펙트를 전수 로드하지 않는다.
2. 서버가 실제 마리오 stage 입장을 확정하면 그 stage의 배치 목록을 한 번 생성한다. F1 디버그 이동과 일반 Intro 진입 모두 이 경계를 사용한다.
3. 서버가 stage run ID, placement/entity ID, 현재 상태, 상태 시작 tick과 transform을 복제한다. 컷신 카메라가 맵을 보여주기 전에 필요한 표현을 준비·반영하는 순서를 보장한다. 실행 중 디스크 읽기를 최소화한다.
4. Client는 snapshot의 상태를 따라 Idle을 시작한다. Effect Tool/시퀀스 뷰어 Play를 누를 필요가 없다.
5. 파괴된 대상은 카메라를 멀리 옮겼다가 돌아와도 살아나지 않는다. 늦게 접속한 Client 역시 최초 Idle 이벤트가 아니라 현재 상태를 받는다.
6. 개인의 완료·실패·퇴장은 그 참가자만 run에서 해제한다. 마지막 참가자 이탈 또는 명시적인 run 전체 종료 때만 해당 run 소유 객체를 정리한다. 다른 참가자·stage, 보스 패턴, MapTool preview handle은 건드리지 않는다. 현재 player별 `Clear_MarioControl`에서 run 전체를 무조건 정리하지 않는다.
7. 마지막 참가자 퇴장 뒤 다음 첫 진입은 새 run으로 초기화한다. 다른 참가자가 남아 있는데 F1 재진입했다고 공을 전부 되살리지 않는다. Reset은 별도의 명시적 서버 debug 명령으로 취급한다.

같은 객체를 `LEVEL_ACTIVE` map effect와 Server spawn 두 경로에서 동시에 생성하지 않는다. 화면 밖 컬링은 그리기만 생략하고 gameplay 상태와 owner 수명을 삭제하지 않는다. Effect 재가시화 시 현재 나이에 맞춰 복구한다.

입장 좌표만 먼저 적용한 뒤 별도 지연 이벤트로 객체를 만드는 방식은 피한다. stage run 초기 상태와 이동/컷신 시작을 같은 진행 세대에 묶고, Client가 해당 세대의 필수 표현을 stage한 뒤 화면 전환을 commit하도록 연결한다. 필수 외형 준비 실패는 빈 맵에서 진행시키지 말고 누락 ID와 이유를 표시하고 진입 실패/재시도 경로로 반환한다. 연결 지연 중 이전 세대의 늦은 spawn 메시지가 새 진행에 섞이지 않도록 run ID와 상태 revision을 검사한다. 이는 추가할 입장 동기화 계약이지 현재 구현 완료된 barrier가 아니다.

기존 `CSpawnGroupRuntime`의 Activate/Is_ActiveOrCompleted는 있지만 개별 stage 재시작 API는 없다. 몬스터 재사용 구현에는 stage 소유 entity 추적과 제한된 reset/cleanup 연결이 필요하다. 방 전체 spawn group을 재초기화하는 우회는 사용하지 않는다.

## G05. 배치 이후 실제 상호작용

### 문양 표적

Idle → 서버 유효 타격 → Hit 또는 Destroyed → 몸체 정리/파괴 효과 순서다. 필요한 타격 횟수, 정답 문양, 목표 집계는 데이터화하되 이번 스크린샷만으로 원작 HP나 클리어 조건을 지정하지 않는다. 동일 공격의 한 판정 구간에서는 같은 entity를 중복 집계하지 않는다.

### 폭탄

Idle → Warning → Exploded → Inactive를 기본 편집 가능한 상태로 제안한다. 타이머형/접근형/피격형 중 실제 사용할 발동 정책은 데이터로 명시한다. 화면만 보고 몇 초 후 폭발하는지 확정하지 않는다. 도화선 불꽃, 경고, 폭발 V2 Group은 Client 표현이고 발동 tick·피해 대상·한 번 판정은 Server가 소유한다.

### 적 삐에로

정확한 모델·clip이 식별된 뒤 MonsterCatalog/MonsterProfiles에 등록한다. 서버 몬스터 entity의 Idle/이동/공격/피격/죽음 상태로 표현한다. 마리오에서 쓸 stationary 또는 지정 구간 왕복 행동을 저작하고, 일반 자유 XZ chase를 그대로 적용해 무대 뒤로 빠지게 하지 않는다. 플레이어 변신 모델을 적의 기본값으로 자동 지정하지 않는다.

### 공격과 층 구분

현재 Mario 입력 분기는 일반 공격까지 소비한다. 따라서 배치 구현의 종료 조건에는 기존 `IPlayerCommandSink`의 interaction slot 경로를 통한 마리오 공격 제출과 Server 소비가 포함된다. 물리 키와 semantic action을 분리하고 일반 class skill ID를 하드코딩하지 않는다. 컷신·UI 포커스·사망 중 입력 차단은 보존한다.

접촉/타격은 stage run·참가자·대상 활성 상태를 먼저 확인한 뒤, Shared XZ overlap에 **높이 범위와 구간 조건**을 함께 적용한다. 마리오는 위아래 층이 겹치므로 XZ 반경만으로 판정하면 다른 층의 공이 맞을 수 있다. 이펙트 billboard 크기와 공격 판정 크기를 동일시하지 않는다. Scene depth/particle 충돌을 서버 정답으로 사용하지 않는다.

이 네 객체를 추가한다는 이유로 기존 Navigation을 재베이크하지 않는다. 통행을 실제로 막아야 하는 개별 대상만 명시적인 서버 collision 계약으로 검토한다. 장식 불꽃이나 문양의 투명 영역이 이동을 막으면 안 된다.

## G06. MapTool에서 사용할 화면과 순서

위치: `F1 → Map Tool → World Gameplay → Mario Objects`를 제안한다. 별도의 독립 에셋 브라우저를 다시 만들지 않고 기존 Object/Effect/Monster 목록을 타입에 맞게 참조한다.

| UI | 기능 / 한국어 도움말 |
|---|---|
| Stage: Mario 1 / 2 / 3 / 4 | 편집할 마리오 선택. 다른 stage 배치를 필터링하며 삭제하지 않음 |
| Object Library | 폭탄·노란 표적·파란 표적·삐에로 정의. 영어 이름과 한국어 별칭, MODEL/EFFECT/MONSTER 표시 |
| Place / Duplicate | 선택 정의로 새 배치. 복제 시 ID는 새로 발급하고 외형 정의는 공유 |
| Placed Objects | 한국어 이름, stage/구간, 배치 ID, Enabled, 외형 연결 상태 검색 |
| Focus | 선택 배치로 편집 카메라 이동 |
| Inspector | 위치·회전·크기·초기 상태·자동 활성·판정 범위·목표 문양 편집 |
| Show Hit Bounds | 현재 선택 층의 타격/접촉 범위를 wire로 표시 |
| Preview Idle / Hit / Destroy | 선택 객체의 시각 상태만 미리보기. 제품 상태를 변경하지 않음 |
| Preview Stage | 선택 stage 배치를 한꺼번에 표시. missing 항목은 오류 표시하고 정상 항목은 유지 |
| Save / Publish Status | 기존 source 저장과 publisher 상태를 구분. 저장만 성공한 것을 Server 반영 완료로 표시하지 않음 |

버튼과 항목명은 영어, 설명·tooltip 및 사용자가 지정한 목록 이름은 한국어를 허용한다. UTF-8 표시와 기존 한글 폰트 경로를 사용하고 stable ID를 번역하지 않는다.

사용 순서: stage 선택 → 정의 선택 → 바닥/설치면 피킹 → 필요하면 중심 높이 보정 → 배치 이름 지정 → 복제·위치 조정 → Preview Stage → Save 및 검증/publish → Server 재시작 → 정상 진입 또는 F1 마리오 진입으로 확인. 원본이 billboard이면 월드 중심은 고정하고 면 방향만 presentation 정책으로 처리한다. 무조건 ground decal로 눕히지 않는다.

## G07. 구현 단위와 검증 종료 조건

구현 순서는 외형 식별 → 정의 admission/preview → gameplay 배치 저장 → stage 수명/복제 → 공격·파괴 → 도구 조정·회귀다. 폭탄·노란/파란 표적·적을 한꺼번에 잘못된 임시 모델로 등록하지 않는다. 공통 배치 계약을 먼저 검증한 뒤 식별된 종류별로 연결한다.

수정 책임은 기존 WorldSequenceDocument/Player/WorldObjectTool, MapTool/WorldGameplayDocument, Map 및 World publisher, GameRoom/Shared 메시지, ClientReplication/Level_KakulSaydonArena/PlayerController에 연결한다. 적 정의와 수치는 MonsterCatalog/MonsterProfiles가 소유한다. 새 C++ 분리가 실제 구현에서 필요하면 해당 프로젝트 `.vcxproj`/`.filters`에 함께 등록하고, Data는 Client `96.DataFiles`의 None 항목으로 등록한다. 현재 문서는 미래용 빈 C++ 파일이나 등록 항목을 만들지 않는다.

검증 항목:

- 저장→재로드 후 stable ID·좌표·stage·외형 참조 동일. 같은 정의를 쓰는 두 배치가 독립적으로 이동·파괴됨.
- invalid V2 ID, 없는 모델/clip, 중복 ID, 음수 scale, 지원하지 않는 kind는 사유를 보존하고 기존 정상 문서·runtime을 교체하지 않음.
- 지연 접속, snapshot 재수신, 중복 spawn/despawn에도 객체와 Effect handle이 하나씩만 존재.
- 첫 stage 진입 및 F1 진입 때 수동 Play 없이 표시. 컷신 종료 후 사라지지 않음.
- 타격한 대상만 서버에서 집계·파괴. 다른 높이의 대상/다른 stage에는 영향 없음.
- 이동 레일·점프·컷신 입력 제한·기존 카드미로와 보스 패턴 회귀.
- 두 플레이어 동시 입장/퇴장, 실패 재시도, 같은 정의의 다중 배치, 카메라 왕복 컬링에서 수명 보존.
- 변경 기능 최소 컴파일, 변경 JSON/XML parse, 해당 publisher 구조 검사, `git diff --check`.
- 실제 위치·크기·재질·애니메이션과 Save/Reload UI는 사용자가 Client에서 직접 확인. 자동 visual PASS 금지.

이번 조사에서는 빌드·publisher 실행·Client/UI 실행·새로운 gameplay 동작 검증을 하지 않았다. 문서·코드·DDS 열람은 구현 완료 증거가 아니다.

## 배포와 인계

Source/JSON/필요한 generated Data는 기능 PR에 넣고, 실제 사용하는 `.wmodel`과 참조 DDS 등은 기존 Resources 상대 폴더를 보존해 Drive로 전달한다. 텍스처가 이미 설치되어 있고 Effect JSON만 새로 만들었다면 새 바이너리 팩이 반드시 필요한 것은 아니다. 원본 DB·추출 전체 팩·미참조 후보는 배포하지 않는다. 이번 설계에서 새로 추가한 제품 리소스나 Drive 업로드는 없다.
