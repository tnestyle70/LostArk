# 발탄 화면과 Server 패턴 연결 복구 구현 계획서

작성일: 2026-09-11. 기준 브랜치: `codex/kouku-full-material-lighting-restoration`, HEAD `e26cd2b282293bd4cb433338bdbc6a7315dd3f84`.

## G00. 현재 보이는 발탄과 패턴 재생 경로

`CClientReplication::Apply_WorldSnapshot`은 Server entity ID로 찾은 `CValtan` 하나에 animation, transform, Effect V2, sound, camera shake를 연결한다. `CValtan::Get_BodyModel()`은 실제 `Part_Body`의 `Com_Model`이며 별도 숨은 모델로 패턴을 재생하는 구조가 아니다. ghost pool은 Server가 승인한 별도 ghost occurrence만 사용한다.

현재 데이터에서 기존 `ValtanPatternAuditionServiceHarness.exe --presentation-generation-admission-contract`를 실행하면 `BossCatalog Effect V2 owner header is invalid.`로 실패한다. `ActorCatalog`와 Python presentation publisher는 BossCatalog의 optional `modelMaterialOverrides`를 지원하지만, C++ presentation reader만 root의 세 필드를 정확하게 요구한다. 재질 복구로 네 번째 필드가 추가되면서 모든 발탄 presentation admission이 실패하고, replication은 Server/HUD 상태만 진행하며 `Apply_NetworkState`를 건너뛴다. 화면의 발탄은 spawn 위치와 idle pose를 유지하는 반면 전투 판정은 Server 위치에서 진행할 수 있다.

네이티브 모델은 body와 donor 합계 173 clip을 갖고, authored presentation 222 occurrence의 native window 검증은 통과했다. Source에 없는 clip을 임의 추가하거나 정상 패턴을 변경하지 않는다.

## G01. C++ presentation reader의 현행 BossCatalog 계약 연결

수정 파일은 `Client/Private/ValtanPresentationGenerationAdmission.cpp`이다. `Build_EffectV2Closure`의 BossCatalog header 검사에서 기존 세 필드와 optional `modelMaterialOverrides` 배열을 허용한다. 배열 존재 시 ActorCatalog와 같은 최대 128개를 요구하고, unknown root field와 잘못된 type은 거부한다. 각 material row의 shader와 texture 검증은 기존 `CActorCatalog`가 소유한다. 전체 BossCatalog bytes는 계속 presentation receipt에 포함된다.

`CValtan`, body component, Server action authority와 기존 snapshot 적용 순서는 유지한다. 잘못된 source를 허용하거나 receipt failure를 무시하지 않고, 지원되는 material contract 때문에 정상 animation/Effect/sound/cache 로드가 막히는 원인을 수정한다.

새 public H 계약이나 C++ 파일은 없으며 프로젝트와 filter 등록도 필요 없다.

## G02. 기존 admission contract의 회귀 검증

`Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPresentationGenerationAdmissionContractTests.cpp`의 기존 fixture runner로 material override 존재/부재 모두 admission되고, unknown root field와 non-array override는 정확한 header 오류로 거부됨을 검증한다. 실패 시 결과 receipt를 덮어쓰지 않는 기존 parse-stage-commit 경계를 유지한다.

기존 harness만 Debug 빌드하고 `--presentation-generation-admission-contract`를 실행한다. Python publisher read-only validate, native animation windows, 변경 파일 UTF-8/CRLF와 `git diff --check`도 확인한다. Client/Server 제품 빌드와 최종 publisher는 통합 담당이 수행한다. Client/UI 실행과 화면 캡처는 하지 않는다.

## G03. 사용자 실행 확인

최종 제품 빌드 이후 사용자가 Visual Studio `Server + Client` profile을 `Ctrl+F5`로 시작하고 Lobby에서 Valtan에 진입한다. 실제 보이는 발탄의 이동/패턴 pose, 무기 socket, Effect와 Server hit 위치가 함께 움직이는지 직접 확인한다. Composition/Pattern별 재생과 시각 품질은 사용자 확인 전에는 완료로 기록하지 않는다.

## G04. V2 binding detail의 실제 transform 소비

통합 전 독립 코드 리뷰에서 새 V2 Detail의 full `LocalTransform` 입력은 저장되지만 `EffectV2_Runtime.cpp::Binding_Local`은 기존 translation/yaw mirror만 소비함을 확인했다. Composition 담당과 조정하여 이 파일의 runtime 소비를 함께 수정한다. full typed scale/rotation/translation으로 기존 binding local matrix를 만들고 child local 뒤에 한 번 합성한다. billboard와 particle은 pivot basis를 교체하므로 기존 `Set_OccurrenceScale` 경계에 binding scale을 별도로 전달한다. 이 값은 일반 mesh/decal의 matrix scale에 추가 곱하지 않으며 기존 child scale 소유는 유지한다.

새 파일이나 public API는 추가하지 않는다. runtime의 실제 helper를 추출한 CPU DirectXMath 검증으로 scale, X/Z rotation, child-parent 합성 및 legacy yaw 동등성을 확인하고, 최소 C++ 컴파일은 root의 통합 Product 빌드에 포함한다. Client 화면과 최종 시각 판정은 수행하지 않는다.
## G05. Server Play로 전환할 때 local clone 해제

추가 실패 소비자 조사에서 MainApp의 successful Server Play가 local preview clone을 제거하지 않고, 일반 Stop은 idle로만 바꿈을 확인했다. preview 생성이 live Server boss를 숨기는 코드는 없지만 별도 clone이 남아 사용자 관찰을 혼동시킬 수 있다. `Animation_Tool.h/.cpp`에 typed release 경계를 추가하고 MainApp의 command submit 성공 후만 호출한다. 기존 PreviewPanel `Release(true)`가 layer와 published target을 해제하며 authoring draft는 유지한다. Animation Tool arena auto-stage를 억제하여 다음 frame에 clone이 다시 생성되지 않게 하고, 명시적인 Local Stage/Retry 성공과 Level change에서 해제한다. submit 실패는 이전 clone/transport 상태를 유지한다. 새 파일·project 등록은 필요 없다.