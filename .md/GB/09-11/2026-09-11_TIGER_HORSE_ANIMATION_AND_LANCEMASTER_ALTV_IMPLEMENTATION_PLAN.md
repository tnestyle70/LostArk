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
