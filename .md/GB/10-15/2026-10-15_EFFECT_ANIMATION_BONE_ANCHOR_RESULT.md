# Effect Animation Library와 Bone Anchor 구현 결과

기준 소스는 `git pull --ff-only`로 받은 `703294d542dedb932dc440863628ba037ffbafa2`이다.
작업 브랜치는 `codex/effect-animation-bone-anchor-20261015`이며 시작 작업 트리는 clean이었다.
계획은 [구현 계획](2026-10-15_EFFECT_ANIMATION_BONE_ANCHOR_IMPLEMENTATION_PLAN.md)을 따른다.

## G01. 구현한 편집 흐름

Effect Tool V1/V2의 `Model View → Animation Library`와 `Composition Resources → Animation`은
현재 모델이 없어도 설치된 캐릭터·보스·World 목록을 읽는다. Action Workbench도 같은 reader를 사용한다.
Character는 직업별, Boss는 보스별, World는 오브젝트 표시 이름과 원본 모델 ID로 분류한다.
목록 선택만으로 모델이나 저작 문서를 바꾸지 않으며 `Load Model`, `Add Animation`, `Add & Play`를
명시적으로 눌렀을 때 기존 CharacterPreviewPanel과 CModel preview를 사용한다.
현재 모델의 미등재 사용자 작성 clip과 Vehicle clip도 보충하고 generation 전환·명시적인 Refresh를
반영한다. 공유 Character Model Workbench의 기존 사용자 작성 clip 선택 경로를 유지한다.

괴기스러운 인형은 실제 native clip을 Effect animation lane에 추가한다. 공 분열은 원본이
Transform/Physics Motion이므로 `Open Object Motion`으로 기존 World Object Tool을 연다.
Motion 행을 skeletal clip으로 삽입하거나 다른 보스·캐릭터의 Animation으로 붙여넣지 않는다.
World 모델은 stable object ID, model asset ID, donor animation set과 modelPreScale를 사용한다.
실제 staged Clone에 요청한 clip이 있는지 확인한 뒤 이전 preview를 교체한다.
같은 object ID라도 model/donor/preScale가 바뀌면 실제 모델 교체로 판단하여 기존 Animation 및
named anchor 보호 조건을 먼저 적용한다. 표시 이름만 같다는 이유로 이전 target을 버리지 않는다.
prototype cache key도 preScale의 float bit를 보존한다. `.0100001f`와 `.0100002f`의 key가 각각
1008981878/1008981985로 달라 이전 prototype의 스케일을 잘못 재사용하지 않는다.

Effect occurrence의 `New occurrence bone/socket` 및 Box Detail의 `Bone / socket`은 로드한 모델의
실제 본을 검색·선택한다. V1에는 sequencer의 조기 반환 뒤에 가려져 있던 Element 부착 UI도 연결했다.
창술사의 `bip001-l-hand`, `bip001-r-hand`, `b_weapon_lhand`, `b_weapon_rhand`는 설치 모델에서 확인했다.
창 socket은 `b_weapon_rhand`다. 전체 occurrence는 sequence 저장, 지원 Element의 개별 부착은
Effect 문서 `Save Changes`가 담당한다. Attach/Clear는 현재 preview cursor를 유지한다.
원본 carrier를 가진 Element의 내부 부착을 강제 변경하지 않으며 전체 occurrence 부착을 사용한다.

Kouku Collider의 `Box Detail → Bone Anchor → Load anchor model`은 선택 Pattern의 BODY/WEAPON을
공용 preview에 준비한다. 아직 Pattern을 Play하지 않아도 실제 본 목록을 검색할 수 있다.
`Anchor bone`, `Rotate with this bone`을 기존 box 계약으로 저장하고 기존 publisher의 본 sampling과
Server 판정 경로를 사용한다. boneRotation 및 World occurrence/emission/effect frame 변경도
즉시 preview 갱신 조건에 포함했다. 대형세이튼 잡기의 실제 왼손은 `BODY / bip001-l-hand`다.
Valtan의 별도 hit-anchor schema와 gameplay 판정은 변경하지 않았다.

신규 C++ 파일은 없으며 `.vcxproj`/`.filters` 변경도 없다. 저작 JSON·게시 snapshot·Resources·
팀장 렌더링 옵션·LAN endpoint를 변경하지 않았다. 따라서 데이터 publisher는 실행하지 않았다.

## G02. 설치 데이터와 본 궤적 확인

설치 WModel과 현재 World 정본을 읽은 정적 inventory 결과는 다음과 같다.
실행 중 ImGui에서 목록을 조작한 결과로 확대하지 않는다.

| 항목 | 확인 결과 |
|---|---|
| 고정 Character/Boss/Monster target | 28개, native clip 3,104개 |
| World object | 470개 |
| World native clip 행 | 3,322개 |
| World Motion/오브젝트 열기 행 | 660개 |
| 전체 resource identity | 7,086개, 중복 없음 |
| 괴기스러운 인형 작은/큰 크기 | `MN_CDMD_00`, 각각 native 30개와 Motion 30개 |
| 월드오브젝트_공 | `world.object.kouku.ball`, 기본 Motion과 `ball_bounce` |
| 분열 공 | `world.object.kouku.saydon.circus.split[.model]`, `circus.shot[.model]`의 실제 g0~g5 Motion |

증거는 로컬 `out/animation-library-inventory.json`, 검사 코드는 `out/check_animation_inventory.py`다.
다음 다섯 World 모델은 현재 Resources에 없어 해당 native 목록만 상태 메시지로 격리한다.

- `Map/KoukuSaydon/Gate2Intro/Table/Table.wmodel`
- `Map/KoukuSaydon/Gate2Intro/Book/Book.wmodel`
- `Map/KoukuSaydon/Gate2Intro/HandBook/HandBook.wmodel`
- `Map/KoukuSaydon/Gate2Intro/CardEruption/CardEruption.wmodel`
- `Map/KoukuSaydon/SourceSequences/kouku.gate1.full/Book/Book.wmodel`

대형세이튼은 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`의
`KOUKUSAYDON_G1_PATTERN_17.presentation.1`을 메모리에만 복사해 검사했다.
`bone=bip001-l-hand`, `boneTarget=BODY`, `boneRotation=BONE`, `followBoss=true`를 적용하고
기존 `_project_collider_regions(document, pattern, window, logic, {}, Path.cwd())`를 호출했다.
기존 `.logic.1`의 시작 2029ms와 수명 788ms에서 42개 본 표본이 생성되었다.
시간은 0~788ms 단조 증가, 위치는 finite, 회전은 단위 quaternion이며 최대 변위는 6.777605m다.
출력은 `BOSS_CURRENT`, center 0, yaw 0으로 offset의 이중 적용이 없다.
`missing.left-hand`는 `CompositionError`로 거절했고 메모리 입력과 디스크 원본은 보존되었다.
원본 SHA-256은 `427575c883f3b56969b71c83cfc6f03e4dace0733faa8c6d034609b3ec2ef70d`다.
증거는 로컬 `out/EffectAnimationBoneAnchor20261015/p17-bone-sampling.json`이다.
이는 실제 설치 모델의 수치 검증이며 GPU 표시·Server 실전 피격·사용자 화면 승인이 아니다.

## G03. 빌드와 검증 경계

Product Debug 정본 명령으로 증분 빌드를 실행했다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -BuildLogDirectory out/EffectAnimationBoneAnchor20261015/build
```

결과는 PASS, process exit 0이다. Engine → Shared → Server → Client 네 프로젝트의 컴파일·링크와
정상 배포가 완료되었다. 최종 C++ 수정은 Client 컴파일 시작 전에 끝났고 이후 소스 변경은 없다.
증거는 `out/BuildPipeline/runs/20261015T063842290Z-debug-product.json`과
`out/EffectAnimationBoneAnchor20261015/product-build.log`다. 전체 소요 시간은 1,796,353ms이며
pull 이후 갱신 대상의 shader/의존 C++도 정상 Build가 처리했다. Clean/Rebuild나 캐시 삭제는 하지 않았다.

Product의 `missingRuntimeInputs=[]`, `invalidRuntimeInputs=[]`이고 Items/ClearRewards·Navigation
검사는 모두 PASS다. 이는 해당 runner의 검사 범위이며 전체 domain 내용·실제 플레이 검증을 뜻하지 않는다.
기존 shader 경고와 C4819/C4828 등의 인코딩 경고는 있었지만 컴파일 오류는 없다.
변경 C++ 19개 파일은 Git checkout 정책 기준 UTF-8/BOM·CRLF/LF를 보존했고 `git diff --check`도 통과했다.
변경 JSON/XML은 없다. Client/UI, 실행 중 Server의 활성화와 실제 저장·재열기 조작은 수행하지 않았다.

기존 Python 테스트 세 개도 실행했으나 현재 pull된 정본과 fixture의 불일치로 실패했다.
이번 변경으로 해당 pipeline/test/data 파일을 바꾸지 않았다.

```text
python -m unittest
  Tools.KoukuSaydonPipeline.test_project_kouku_saydon_composition.KoukuSaydonCompositionProjectionTests.test_bone_contact_bakes_real_hammer_and_body_with_target_yaw
  Tools.KoukuSaydonPipeline.test_project_kouku_saydon_composition.KoukuSaydonCompositionProjectionTests.test_bone_contact_uses_quantized_stage_origin_and_tick_brackets
  Tools.KoukuSaydonPipeline.test_project_kouku_saydon_composition.KoukuSaydonCompositionProjectionTests.test_bone_contact_rejects_missing_bone_clip_asset_and_sample_gap
  -v
```

결과는 failures=6, errors=1이다. `setUpClass`는 현재 정본을 읽지만 `first_product`가 고정 P1을
찾는다. 실제 P1은 DRAFT이고 PlayAll 목록에 없어 projection에서 제외되므로 정상 사례의 조회는
StopIteration이고 잘못된 본 사례는 bake까지 진입하지 않는다. quantized-stage 테스트는 1000ms에
두 번째 clip을 기대하지만 현재 정본의 STAGE_51 시작은 1001ms다. 위 P17 직접 검증은 이 오래된
fixture를 통과했다고 주장하지 않으며, 요청한 실제 grab의 sampling을 별도로 확인한 결과다.

## G04. 사용자가 확인할 화면

1. Effect V1/V2 `Model View → Animation Library → Character → LanceMaster`에서 clip을 선택하고
   `Add & Play`로 재생한다. 같은 timeline에 Effect를 추가한 뒤 occurrence `Bone / socket`을
   `b_weapon_rhand` 또는 `bip001-l-hand`로 선택한다. sequence 저장·재열기와 본 추종 화면을 확인한다.
2. `World`에서 괴기스러운 인형의 Native clips와 공 분열의 Object Motions가 각각 해당 이름·모델로
   분류되는지 확인한다. 인형은 animation lane, 분열 공은 기존 Object Tool의 Motion을 사용한다.
3. 대형세이튼 잡기 Collider Box Detail에서 BOSS anchor를 선택하고 `Load anchor model`을 누른다.
   `BODY → bip001-l-hand`와 필요한 bone rotation을 선택해 저장·게시하고 왼손 부착 화면을 확인한다.
4. 잘못된 clip/본, 미적용 Detail draft, 다른 골격의 sequence가 남은 상태에서는 기존 편집 내용이
   유지되는지 확인한다. 위 화면 검증과 실행 중 Server 활성화는 아직 수행하지 않았다.


<!-- LOCAL_MAIN_BUILD_AUDIT_20261015 -->
## main 통합 후 로컬 빌드·연결 검토

PR #542 (`6057739db` → main `92169ebe1`)를 로컬로 fast-forward했다. 변경25파일 중 기존
C++/헤더19개와 문서6개이며 새 프로젝트 등록/JSON/XML/Resources/schema 변경은 없다.
실제 모델 stage→clip 검증→preview 교체, occurrence와 Element의 저장 분리,
BODY/WEAPON 본 선택→기존 publisher sampling→Server worldTrack 소비를 대조했다.
독립 재현 가능한 새 결함은 확인하지 못했고 source diff whitespace 검사는 통과했다.

이 PC에서 Product Release·Debug를 다시 빌드했다. 공식 결과는 각각
`out/BuildPipeline/runs/20261015T094035032Z-release-product.json`,
`out/BuildPipeline/runs/20261015T094639831Z-debug-product.json`이며 두 구성 모두 PASS다.
빌드 전후 source fingerprint는 각 구성에서 동일했고 기존 warning은 남는다.
실제 UI 저장·재열기/본 추종/Server 활성화·피격을 이번 작업에서 실행하지 않았다.
앞 절의 upstream inventory/42표본 및 Python fixture 실패를 재실행하거나 결과를 덮어쓰지 않았다.
상세 검토는 `기술소개서/베른컷신최적화/2026-10-15_근거정리/근거데이터/effect-anchor-audit.json`,
최신 binary SHA는 같은 폴더의 `latest-build-verification.json`을 따른다.
