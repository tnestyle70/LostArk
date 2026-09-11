# 호랑이·말 애니메이션 교정과 창술사 ALT V 통합 구현 계획

## G00. 목표와 현재 기준점

도화가 D의 두 호랑이를 캐릭터 정면에 배치하고 이동 속도를 절반으로 낮추며 달리기를 반복한다. 창술사 ALT V는 현재 저장된 clip1~4의 내용을 보존하면서 실제 캐릭터 애니메이션 시간축을 가진 하나의 full restore로 재생한다. 말과 호랑이의 골격 애니메이션은 원본에서 재추출하고, 거대 창의 부착 편집을 실제 런타임 소비 경로에 연결한다.

사용자가 첨부한 두 화면에서 거대 창이 캐릭터에서 떨어져 있고 말의 몸과 앞다리가 서로 다른 방향으로 꺾여 있는 상태를 관찰했다. 스크린샷만으로 visual fidelity나 원인을 확정하지 않고 원본 PSA, 설치 WModel, 문서, 실행 경로를 대조한다. Character Select 모델 로딩과 Profiler의 선행 변경은 별도 `2026-09-11_LOADING_FREEZE_AND_NAMED_PROFILER_IMPLEMENTATION_RESULT.md`에 기록한다.

현재 branch는 `codex/kouku-gate1-sequence-playback`이며 혼합 dirty worktree다. clip1의 사용자 삭제 6개와 clip2의 사용자 회전 편집을 포함한 기존 문서는 보존한다. Resources 바이너리는 기존 경로에만 교체하고 Git에 추적하지 않는다.

## G01. 원본 애니메이션 변환과 자산 교체

`Tools/ActorXAssetCooker/build_umodel_gltf_psa.py`의 교정된 root/child quaternion, mesh hierarchy, FLOAT skin weights 변환을 사용한다. Tiger `SK_SDM_TIG_00`은 37 joints/7 clips, Horse `SK_FLM_HOR_00`은 151 joints/3 clips다. Horse의 PSA BONENAMES 순서와 glTF joint 순서는 다르지만 이름 집합은 같으므로 명시적인 `--allow-bone-order-remap`에서만 이름의 전단사 대응을 허용한다. 기본 strict order 검사와 skin JOINTS, inverse bind, mesh hierarchy 순서는 유지한다.

원본 재쿠킹 결과와 설치 파일의 WMSH/WSKL byte 비교, finite/weight 범위, clip duration/ticks와 여러 pose를 대조한다. 현재 관찰된 핵심 차이는 자식 본 animation quaternion이며, glTF와 WModel만의 일치는 잘못 변환된 PSA 입력의 정확성을 증명하지 못한다.

기존 `Effect/Artist/Models/SK_SDM_TIG_00/sk_sdm_tig_00_sk.wmodel`과 `Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section0~3.wmodel`을 같은 CModel 경로에서 소비한다. 말의 네 파일은 한 말의 네 재질 section이다. 기존 재질 section bytes를 보존하고 애니메이션 교정 자산을 검증한 뒤 설치한다. 새 C++ 모델 런타임이나 프로젝트 파일은 추가하지 않는다.

## G02. ModelCue 반복 계약과 Artist D 두 마리

`Client/Public/Effect_AuthoringDocument.h`의 ModelCue에 optional `loop=false`를 추가한다. `Effect_DocumentCodec.cpp`가 parse/serialize하고 `Effect_DocumentRenderer.cpp`가 loop와 holdLastFrame의 동시 사용을 거부한다. loop=true일 때만 clip 길이에 대해 local animation time을 감싼 뒤 기존 CModel seek를 호출한다. 생략한 기존 문서와 holdLastFrame=false의 의미는 바꾸지 않는다.

`Data/Effects/Authored/effect.artist.skill.31490.full.restore.effect.json`에서 기존 tiger cue ID를 유지하고 두 번째 stable cue ID를 추가한다. particle 26행과 재질·delay·전체 cue 길이는 보존한다. source Y-up/+X-forward를 owner Y-up/+Z-forward로 보내는 local rotation `[0,-90,0]`, 위치 X ±0.8/Y 1/Z 1.5, velocity `[0,0,3.25]`를 사용한다. 기존 6.5에서 이동 속도만 절반으로 낮추며 animation 속도는 유지하고 loop를 켠다. 기존 source yaw 오류에 화면상 30도를 중복 적용하지 않는다.

`Effect_Tool.cpp`의 기존 ModelCue detail에서 Loop/Hold의 배타 선택과 velocity 편집을 연결한다. 기존 authoring generator가 cue를 복사하는 경로도 확인해 재생성으로 수정이 사라지지 않게 한다. 파싱 실패·충돌하는 loop/hold 입력은 기존 문서와 Preview를 보존한다.

## G03. LanceMaster ALT V 한 문서와 시간축

정확한 join은 class LanceMaster, skill 34630, `super_squalllance_01~04`다. 설치 playable 모델의 duration은 `[67,66,30,41] / 30`초이고 Sequencer와 동일한 millisecond ceiling 기준 시작점은 `[0,2234,4434,5434]`다.

새 stable ID `effect.lancemaster.skill.34630.full.restore`는 현재 clip1~4의 378 Elements/12 ModelCues를 시간 이동해 담는다. 각 source emitter 내부 시간과 cue의 상대 시간은 원본 계약을 따라 필요한 외부 시작점에만 적용한다. 기존 sidecar의 camera 3 shots와 살아 있는 localOnlyElementIds를 동일 시간축에 합친다. 기존 four clip 문서는 보존하며 새 full의 네 horse sections는 교정된 한 골격의 동일 pose/transform을 사용한다.

EffectCatalog, EffectResourceTree, Animation의 기존 animevent를 실제 소비자로 연결한다. 첫 clip의 단일 event가 full effect의 NATURAL lifetime을 소유하며 뒤 세 clip의 중복 effect event를 제거한다. full effect와 camera sidecar는 Client.vcxproj의 96.DataFiles None 항목 및 해당 filters에 필요한 항목만 등록한다. 기존 필터를 재배치하지 않는다.

`Tools/EffectPipeline/combine_lancemaster_altv_full_restore.py`는 입력 네 문서를 보존하고 별도 출력에 합치는 재현 도구다. 단일 문서의 기존 ModelCue 상한 8개는 세 시점의 말 material sections 12개를 허용하도록 16개로 늘리고 ID/path/clip/timing의 기존 검증은 유지한다.

`Effect_PresentationService.cpp`의 기존 `Requires_SourceBoneImportScaleNormalization`에 새 full ID만 추가해 clip별 경로의 source bone 단위를 유지한다. 별도 spawn 경로나 effect runtime을 만들지 않는다.

## G04. 거대 창 부착과 편집 소비자

거대 창 후보는 ModelCue가 아닌 `fm_x_flm_gdr_01` MeshParticle이며 원본 Midcontrol에서 b_weapon_rhand와 socket offset을 사용한다. Play All과 Play Saved Effect는 현재 같은 recovery/Sequencer/document factory를 사용하므로, clip-local 문서를 전체 animation 시작점 0에 붙이던 시간을 우선 교정한다.

`Effect_Tool.cpp`의 기존 Follow Attachment draft/commit 흐름을 source-contract 아닌 SourceRecipe MeshParticle에도 열어 source socket position/rotation과 Bone/OwnerYaw를 편집할 수 있게 한다. 실제 source attachment provider가 값을 소비하는지 확인하고 개별 수정은 stable ElementId의 runtimeAnchorSlotId를 사용해 같은 Midcontrol의 다른 행과 분리한다. 단순 local rotation으로 뒤틀린 골격을 숨기지 않으며, 확정되지 않은 창의 화면상 오프셋을 임의로 만들어 원본에 덮어쓰지 않는다.

## G05. 종료 검증

변경한 converter의 기존 및 이름 remap 검사를 실행하고, 설치 모델의 chunk/clip/pose 수치와 기존 재질 보존을 확인한다. ModelCue parser/serializer/loop seek의 실제 소비 경로와 기존 no-loop 의미를 검증한다. full 문서의 ID 중복, element/modelcue 수, source dependency, animation/camera 시작점, catalog/tree/project 연결을 검사한다. 변경 C++는 Debug focused compile, JSON/XML parse와 git diff --check를 수행한다.

Client/Server는 사용자가 실행 중이므로 에이전트가 종료하거나 Client/UI를 실행·조작하지 않는다. 전체 제품 DLL/EXE 빌드와 실제 화면 확인은 실행 상태를 구분해 보고한다. 사용자가 F1 Effect Tool v1에서 Artist D full restore와 LanceMaster ALT V full restore를 열어 Play All로 확인하고, 캐릭터 ALT V의 실제 camera/창/말 동작을 판단한다. 수치 검사 결과를 visual PASS로 대신 기록하지 않는다.

## G06. 전체 창술사 이펙트 미출력의 이벤트 헤더 교정

후속 사용자 관찰은 캐릭터 스킬 애니메이션은 동작하지만 모든 창술사 이펙트가 보이지 않는 것이다. 실제 CAnimationEffectCueDocument의 prewarm 및 model clip catalog를 받는 Load 양쪽에서 `Animation event row count does not match the header.`를 재현했다. 기존 ALT V 설치 작업은 네 EFFECT 행을 한 행으로 합치면서 LanceMaster.animevents의 선언 3,139행을 남겼고 실제 행은 3,136개였다. 이 차이가 전체 문서를 거부해 CCharacter에 43개 Effect cue가 설치되지 않았다.

정본 animevents의 헤더를 실제 행 수로 교정하고, 추후 같은 통합 도구가 별도 출력에 최종 event 문서를 생성할 때 행 수를 다시 계산하도록 연결한다. 기존 네 clip 문서와 사용자가 편집 중인 World Effect/tree는 덮지 않는다. parser의 일치 검사나 실패한 문서의 transactional 격리를 완화하지 않는다. 새로운 C++ runtime/API/project 항목은 추가하지 않는다.

실제 C++ 로더 양쪽에서 43개 cue와 동일한 clip/effect/anchor/start/end 대응이 복구되는지 검증한다. 설치된 playable WModel과 추가 animation set에서 decode한 clip·bone 이름을 사용하고 catalog 누락과 anchor 누락이 없는지 확인한다. 생성 도구는 네 행→한 행, 이미 합친 입력의 재실행, 다른 event·comment 보존과 잘못된 입력의 기존 출력 보존을 필요한 범위에서 검사한다. 단순 파일 존재 검사를 cue admission 성공으로 대신하지 않는다.

이 데이터는 CProjectDataRoot가 저장소 Data에서 직접 읽고 준비된 cue 문서는 Client process 메모리에 남는다. C++와 바이너리를 바꾸지 않으므로 재빌드·publisher·Server 재시작은 필요 없고, 수정 후 사용자가 Client를 재시작해 다시 준비한다. PR #360은 이미 사용자에 의해 merge됐으므로 후속 수정은 별도 codex branch와 PR로 게시하고 merge하지 않는다.

## G07. ALT V 제품 문서 크기 계약과 V 손 부착 회귀

사용자가 Client를 재시작한 뒤 V는 출력되지만 손의 생성 창 위치가 달라졌고 ALT V는 캐릭터 애니메이션만 출력된다고 확인했다. 9월 12일 actual Catalog Load → Capture_ProductLoadStageRequest → Stage_LoadingProductTarget에서 통합 문서가 `exceeds 16 MiB`로 거부되는 것을 재현했다. 통합 정본은 20,049,144 bytes다. standalone Codec Parse는 64MiB를 허용하므로 앞선 Codec/Renderer Stage 검사와 실제 제품 준비 경로가 서로 다른 파일 계약을 사용했다.

`Client/Public/Effect_DocumentCodec.h`에 기존 Codec의 64MiB 최대 문서 크기를 공통 상수로 두고 `Effect_DocumentCodec.cpp`의 Parse/Load와 `Effect_Catalog.cpp`의 직접 authored 문서 로드가 이를 함께 소비한다. Load는 읽기 전에 길이를 확인하고 bounded read 이후 불완전 읽기나 크기 변화도 거부한다. catalog index의 기존 16MiB와 문서 depth/형식/ID/의미 검증은 유지한다. 단순 공백 압축은 F1 Save가 다시 큰 문서를 만들 수 있으므로 런타임과 저작 도구의 계약 불일치를 남기는 해결로 사용하지 않는다. 새 C++ 파일이나 프로젝트 등록은 없다.

최신 실제 C++ Catalog·Product Stage로 20MiB 통합 정본의 준비 완료를 확인하고, 공통 상한 초과 입력 거부와 실패 시 기존 출력 보존을 검사한다. Character의 실제 cue scheduling도 설치 모델의 시간과 현재 binding으로 확인한다. 최초 cue 실패를 이후 clip의 별도 spawn으로 숨기거나 통합 문서를 다시 네 제품 cue로 분리하지 않는다.

V의 세 창은 `fm_n_flm_ydr_00_sm`, ALT V의 창은 `fm_x_flm_gdr_01` 및 dragon이므로 같은 자산으로 가정하지 않는다. V 정본은 이전 커밋과 동일하며 9월 11일 추가된 source-bone 단위 보정의 실제 크기·위치·방향 효과를 CModel과 Playback으로 측정한다. 실제 손본 pose와 두 mesh의 축·단위를 대조한 뒤 필요한 부착 수정만 적용한다. 화면상 표현을 근거 없는 공통 90도 회전으로 덮지 않는다. 이 G의 C++ 수정은 새 Client 빌드가 필요하며 G06의 데이터만 변경한 재시작 안내와 구분한다.

실측한 V/ALT V 창은 model-local +Y가 긴 축이고 손의 장착 무기는 +X가 긴 축이다. SourceRecipe의 TypeData `pitch=-90`이 기존 detail에 투영되지 않아 0으로 실행되고 있었으며, 별도 MeshRotation quarter-turn만 적용하면 창의 긴 축이 손본 -Z로 향한다. 세 exact mesh의 기존 `detail.mesh.sourceTypeDataRotationDegrees`에 source `[roll=0,pitch=-90,yaw=0]`을 한 번 투영한다. 기존 `UE3_EulerDegreesToClientRotation`과 곱 순서를 사용하면 +X로 정렬된다. V 3행, 이전 ALT V 10행과 통합 ALT V 10행의 총 23행만 수정하고 offset/scale/local rotation·재질·시간축은 유지한다. 기존 Lance generator의 해당 투영 누락도 수정하며, 통합 generator는 교정된 원본 element를 그대로 복사한다. 실제 mesh vertex와 60Hz CModel/Playback pose로 수정 전후 손 기준 축과 거리를 대조한다.
