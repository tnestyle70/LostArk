# 발탄 화면과 Server 패턴 연결 복구 결과

작성일: 2026-09-11. 상태: **원인 재현과 C++ 수정, focused 자동 검증 완료 / 제품 빌드·사용자 화면 확인은 통합 단계**.

## G00. 재현된 원인

`Client/Private/ValtanPresentationGenerationAdmission.cpp`의 `Build_EffectV2Closure`가 BossCatalog root를 `schema/formatVersion/bosses` 세 필드로만 제한했다. 현재 재질 저작 데이터의 정상적인 optional `modelMaterialOverrides`를 `ActorCatalog`와 Python publisher가 허용해도 C++ 발탄 presentation reader는 거부했다.

수정 전 기존 Debug executable로 다음을 실행했고 실제로 실패했다.

```powershell
Tools/ValtanPatternAuditionServiceHarness/Bin/Debug/ValtanPatternAuditionServiceHarness.exe --presentation-generation-admission-contract
```

실제 진단은 `BossCatalog Effect V2 owner header is invalid.`였고 current typed presentation source admission부터 실패했다. 이 실패를 받는 `CClientReplication::Apply_WorldSnapshot`은 HUD/Server 상태는 반영하면서 `CValtan::Apply_NetworkState`는 건너뛴다. 결과적으로 화면의 발탄은 spawn 위치/idle에 남고, Server 전투 위치와 패턴 판정은 계속 진행할 수 있다. 이는 사용자 증상과 일치하는 코드 경로이며 실제 Client 화면 재생은 아직 검증하지 않았다.

`CValtan`이 animation을 적용하는 body model은 실제 `Part_Body/Com_Model`과 동일하다. 따라서 이번 원인은 별도 invisible boss를 잘못 선택했다는 구현이 아니라 presentation admission이 전체 차단된 문제다. Server ghost pool과 map-world combat object의 독립 occurrence는 정상적인 기존 계약으로 보존했다.

## G01. 실제 수정

- `Client/Private/ValtanPresentationGenerationAdmission.cpp`: optional `modelMaterialOverrides` array를 현행 BossCatalog 계약으로 인정한다. 배열 최대 128개, unknown root field/type 거부, full BossCatalog hash 포함, currentness 검증과 실패 시 기존 receipt 보존을 유지한다. 각 material row 검증은 `CActorCatalog`가 계속 소유한다.
- `Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPresentationGenerationAdmissionContractTests.cpp`: override 없음과 빈 array admission, null/object/unknown field 거부, 거부 뒤 이전 receipt 보존을 기존 fixture runner에 추가했다. 실제 원본의 populated override array는 기존 baseline admission이 검증한다.

admission 수정에는 새 C++ 파일·public API·JSON schema·project/filter 등록이 없다. CValtan animation/sound runtime과 Server 패턴 상태·데이터 정본을 유지했다. 통합 리뷰 중 추가한 V2 transform 소비 수정은 G04에 분리한다.

## G02. 수행한 검증

```powershell
& 'C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe' Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /p:BuildProjectReferences=false /m:2 /v:minimal /nologo
Tools/ValtanPatternAuditionServiceHarness/Bin/Debug/ValtanPatternAuditionServiceHarness.exe --presentation-generation-admission-contract
python Tools/GameplayPipeline/valtan_presentation_generation.py --repository-root . --mode Validate
git diff --check -- Client/Private/ValtanPresentationGenerationAdmission.cpp Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPresentationGenerationAdmissionContractTests.cpp
```

- 기존 harness Debug compile/link: exit 0. 기존 Engine CP949 header에 대한 `/utf-8` C4828 warning은 있으며 이 작업에서 인코딩을 변경하지 않았다.
- 수정 후 `ValtanPresentationGenerationAdmissionContractTests: PASS (Pattern Sound remains an independent typed receipt)`; exit 0. 기존 current/exact/stale receipt, V2 reachable group/leaf, malformed identity 및 rollback 검증도 이 flag에 포함된다.
- Python read-only presentation generation: artifact 144, PASS. 수정 전 generation은 `579f9b765a5a8ca937dd2b2e37139cb3e36de5dbdc77eee123d57f8c469cc827`였고, 병행 재질 변경 이후 값은 달라질 수 있다.
- 기존 native animation inventory: 합성 clip 173개, authored occurrence 222개/사용 clip 79개, source-window 검증 PASS. Product binding의 누락 clip 이름 0개.
- 변경 C++ 두 파일 UTF-8, CRLF 유지 확인. PLAN/RESULT UTF-8 확인. 기존 harness project XML parse와 변경 파일 `git diff --check` PASS.

Server bootstrap이 가리키던 `ecdc42285914f2303cdcf26bca1950e797bf97c95598026c2eaad935f08397ce`와 현재 presentation manifest를 비교했을 때 차이 artifact는 `Data/Actors/BossCatalog.json` 하나였고 누락된 artifact는 없었다. 기존 `Acquire_Receipt`는 ordinary typed source를 현재 검증하는 경로이며 stale entry receipt 자체를 실패 원인으로 단정하지 않았다. 최종 패키지 publisher 동기화는 통합 담당이 수행한다.

## G03. 미실행 검증과 인계

Client/Server 제품 빌드와 publisher는 이 하위 작업에서 실행하지 않았다. 실제 Client/UI 실행·조작·화면 캡처, 사용자가 직접 Pattern Play를 누른 결과, 시각 품질과 도넛/카메라 타이밍 판정은 미검증이다.

LAN 자동 설정은 `server-host`, TCP 7777 LocalSubnet ready, endpoint `not-listening`으로 완료됐다. 통합 빌드 뒤 사용자는 Visual Studio `Server + Client` profile을 `Ctrl+F5`로 실행해 Lobby → Valtan으로 진입한다. 보이는 발탄의 실제 이동·패턴 pose와 Effect/weapon anchor가 함께 움직이는지 확인한다. 화면 확인 전에는 발탄 전체 복원이나 visual PASS로 기록하지 않는다.

## G04. 독립 리뷰에서 확인한 V2 binding transform 소비 보완

Composition 담당의 새 V2 binding Detail은 `LocalTransform` 전체를 stage/save하지만, 실제 `EffectV2_Runtime.cpp::Binding_Local`은 이전 `fYawDegrees/vOffset` mirror만 읽었다. 따라서 scale `(2,2,2)` 또는 X/Z rotation을 입력해도 저장값만 바뀌고 runtime local matrix는 동일했다. API의 typed parse/validate/snapshot/CAS 경로에는 이 누락이 없었고 최종 렌더 소비 경계가 원인이었다.

조정 후 `Client/Private/EffectV2_Runtime.cpp` 한 파일을 수정했다. Binding local은 typed scale × roll/pitch/yaw rotation × translation을 만들며, group child local 뒤에 합성된다. billboard/particle draw는 pivot basis를 교체하므로 기존 `Set_OccurrenceScale`에 binding scale을 전달한다. free-group playback은 여기에 기존 parent pivot scale을 한 번 곱한다. 기존 child size의 Params/local matrix 경로는 이 변경에서 유지했고, binding scale을 child Params에 추가로 곱하지 않는다. V1 binding parser도 typed `LocalTransform`을 채우므로 기존 yaw/translation 데이터는 같은 matrix를 만든다.

실제 runtime의 `Binding_Local` 함수 본문을 TEMP의 독립 DirectXMath 프로그램으로 추출해 `cl /EHsc /std:c++20 /W4` compile/run했다. exit 0, `Actual Binding_Local DirectXMath checks: PASS`이다. default identity, `(2,3,4)` scale 축 길이, translation 보존, child X=10에 binding scaleX=2/translationX=3을 적용한 X=23, pitch와 roll 각 90°의 축 변환, 기존 yaw37°+translation과 행렬 동등성을 확인했다. 임시 프로그램은 저장소 source/project에 추가하지 않았다.

`git diff --check`와 UTF-8/CRLF 유지도 확인했다. root가 이 소스를 포함한 통합 Product Debug 빌드를 시작했으며 이 문서 작성 시 그 결과는 아직 미확인이다. Client/UI 실행과 실제 billboard/particle/mesh 시각 결과는 미검증이다.
## G05. Server Play 성공 뒤 local preview clone 해제

추가 조사에서 일반 Composition `Stop`은 clone의 `mesh_idle_battle_1`을 복원하며 객체 자체를 해제하지 않음을 확인했다. `CCharacterPreviewPanel`은 별도 `Layer_AnimationPreview`에 collision 없는 non-authoritative Valtan을 배치하고 live Server boss의 visibility flags를 변경하지 않는다. 그러나 기존 `MainApp::Debug_CompletePlaySelected`는 Server command를 제출하고도 clone을 남겼다. 단순 `Release(true)`만 추가해도 Animation Tool의 arena auto-stage가 다음 frame에 clone을 다시 만든다.

`Animation_Tool.h/.cpp`에 `Release_ValtanCompositionPreviewForServerPlayback`을 추가했다. MainApp의 Server submit 성공 block에서만 이를 호출하고 Composition preview owner를 NONE으로 전환한다. helper는 정확한 Valtan preview asset/boss를 확인한 뒤 master/raw clip transport를 멈추고 기존 PreviewPanel `Release(true)`로 layer 객체와 target publication을 해제한다. unrelated preview와 authoring draft는 유지한다. auto-stage 억제 상태는 명시적인 Local Stage/Retry 성공 또는 On_LevelChanged에서 해제된다. 일반 Local Stop의 idle 유지 동작은 그대로다.

실제 수정 함수 `Debug_CompletePlaySelected`와 `Release_ValtanCompositionPreviewForServerPlayback` 본문을 TEMP 독립 C++ 프로그램에 추출하고 collaborator만 stub으로 대체해 `cl /EHsc /std:c++20 /W4` compile/run했다. exit 0, `Actual Server-submit/preview-release failure-consumer checks: PASS`이다. 다음을 확인했다.

- 제출 실패: clone, master/raw clip transport, preview owner, suppression state 유지.
- 제출 성공: Valtan clone 한 번 해제, 두 transport 중지, owner NONE, Server status tracking 유지.
- dirty authoring draft와 unrelated asset preview 유지.
- inventory에 없는 Pattern: command를 호출하지 않고 preview 유지.

기존 source tests `Tools.EffectPipeline.test_valtan_model_view_composition`와 `Tools.ValtanPipeline.test_valtan_f1_arena_preservation_contract`도 실행했다. 총 30개 중 29 PASS, 1 FAIL이다. 실패는 수정하지 않은 `Effect_Tool.cpp` source-anchor history가 `PoseSample.BoneCombinedMatrices[iBoneSample]`를 쓰는데 기존 테스트 747행이 `[iRequest]` literal을 요구하는 불일치다. 관련 없는 소스/테스트는 변경하지 않았다. 변경한 세 C++ 파일 UTF-8/CRLF 유지와 `git diff --check`는 PASS다.

Product compile은 root가 이 소스를 포함해 진행하며 이 하위 문서 작성 시 결과는 미확인이다. UI 입력을 통한 clone 제거·재생성·Server 거부 상태와 실제 Valtan 화면 판정은 사용자 확인 전까지 미검증이다.