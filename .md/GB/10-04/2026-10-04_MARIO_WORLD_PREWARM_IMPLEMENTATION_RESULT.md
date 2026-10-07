# Mario World Object 생성 준비 개선 결과

## G00. 범위와 관찰 근거

2026-10-04 사용자가 Debug에서 Mario 1~4의 1페이즈와 2페이즈 영상을 촬영할 때 인형·공·칼날·갈고리 생성 순간의 병목을 구조적으로 줄이도록 요청했다. 이 변경은 `codex/debug-effect-loading`의 DataJson 최적화 후속이다. 실제 프레임 캡처는 없으므로 특정 프레임 드랍의 원인 비율이나 FPS 개선 수치를 단정하지 않는다.

현재 저장된 Composition revision 2498의 P88/P91/P92/P93은 큰 인형 2개와 공 2개를 발생시킨다. 기존 준비 단계는 해당 두 모션에 각각 clone 4개를 만들므로, 이 선택만으로 인형·공 pool 부족이라고 진단하지 않는다. 반면 칼날·갈고리는 모델 준비만 수행하고 clone 준비 대상에는 없었다. P91/P92/P93에는 갈고리 18 emissions가 있으며 P33에는 갈고리 15개·일반 칼날 8개·즉사 칼날 1개가 있다. 별도 P31의 같은 일반 칼날 모션 5 occurrence는 40개를 요구한다. 고유 motion ID 집합만 읽으면 이 중복 발생 수가 사라진다.

인형 cue 생성 경로는 `Consume_OwnedWorldCue → Build_PlaybackSubset → Set_Document → PrepareCompositionWorld → Play → Seek`다. `Build_PlaybackSubset`은 같은 object를 사용하는 모션도 가져오므로 큰 인형 하나에도 현재 30 motion/template을 복사한다. 이어 `Set_DocumentBatch`는 subset 검증과 추가 복사를 수행한다. 실제 V1 Effect의 해당 spawn 경로는 이미 준비된 renderer 리소스를 사용하므로, 이 현상을 매번 Effect DDS 파일을 읽는 문제라고 설명하지 않는다.

기준 소스와 입력 SHA-256는 `out/MarioWorldPrewarm20261004/baseline-manifest.json`에 보존했다. 기존 5개 설명 문서는 이 변경에 포함하지 않는다.

## G01. 실제 반영

- `KoukuSaydonPresentationAssetService`는 게시 Encounter의 `worldSequences` 발생 행을 중복 보존한다. 기존의 고유 `WorldInstanceIds`는 모델·Effect 의존 종류를 준비하는 역할을 유지한다. 발생 예약은 root의 도달 가능한 pattern을 보수적으로 합산하고, 전체 raid의 순차 root 사이에서는 최대를 취한다. Bundle member는 각각 합산한다. `motionInstanceId` 등 기존 객체의 상태 전환은 새 생성 행으로 세지 않는다.
- `Level_KakulSaydonArena`는 인형·공·칼날·갈고리의 실제 `EmissionCount()`로 objectId별 복제본 예약을 만든다. 카드6/조커1 및 인형·공4의 기존 최소값을 유지한다. 모델 준비 뒤 기존 owner pool에 한 update당 최대 한 clone을 준비하고, 실제 생성 root의 재생 문서를 검증·보관한 뒤 재생 준비 완료를 반환한다. Play Pattern의 BossTool, Complete Play의 MainApp, Release 진입 준비가 같은 소비자다.
- `Prewarm_ObjectInstancesStep`은 성공과 준비 완료를 구분한다. 새 객체는 기존 Prototype/Clone/Layer 경로에서 숨겨 준비하며 요청 전체의 object별128/owner전체1024 제한을 확인한다. 한도를 잘라 맞추지 않는다. 실패하면 해당 step에서 만든 객체만 제거하고 기존 pool을 유지한다. 기존 즉시 `Prewarm_ObjectInstances`의 호출 계약도 유지한다.
- `Prepare_PlaybackSubset`은 object-only WORLD subset을 실제 기존 validator로 먼저 검증한다. `Set_PlaybackSubset`은 같은 owner와 level/device/context/catalog/Area/revision에서 그 결과를 한 번 복사해 commit한다. 캐시가 맞지 않거나 map/deploy/live anchor 대상이면 원래 Build→Set_Document 경로를 사용한다. 모든 문서 교체와 Clear는 캐시를 비우며 같은 revision으로 다시 저장한 문서도 예외가 아니다. 인형 생성 시 문서 복사가 완전히 사라진 것은 아니며, 반복 Build·Validate와 두 번 하던 복사 중 한 번을 제거했다.

전체 반영 코드는 [PLAN](2026-10-04_MARIO_WORLD_PREWARM_IMPLEMENTATION_PLAN.md)에 있다. 기존 C++ 6파일만 수정했으며 새 project/filter 등록은 없다. 각 파일의 기존 BOM·CRLF를 유지했다. Client/UI와 Server의 자율 실행, 화면 캡처, authored/runtime 데이터 교체 또는 raid publish는 수행하지 않았다.

## G02. 현재 데이터의 준비 수량과 검증

| 선택 | 갈고리 | 칼날 | 인형 | 공 | 전체 예약 |
|---|---:|---:|---:|---:|---:|
| Mario1 P88 및 후속 | 15 | 9 | 4 | 4 | 32 |
| Mario2~4 P91/P92/P93 및 후속 | 33 | 9 | 4 | 4 | 50 |
| P33 2페이즈 단독 | 15 | 9 | 0 | 0 | 24 |
| P31 칼날 시험 | 0 | 40 | 0 | 0 | 40 |
| Whole Raid | 33 | 40 | 4 | 4 | 88 |

Whole Raid의88개에는 카드6·조커1이 포함된다. 갈고리33개는 1페이즈18개와 후속15개를 함께 준비한 보수적인 예약이다. 정확한 최대 동시 생존 개수나 프레임 시간을 측정한 표가 아니다. 별도 독립 실행이 기존 준비 범위와 겹치면 pool 소진 후 원래 clone fallback이 남는다. 실제 World loop는 같은 emitter 객체를 재사용하며, Hide만으로 pool에 반환되는 것으로 계산하지 않는다.

현재114개 패턴·9개 Bundle·Whole Raid는 모두128/1024 한도 안이다. Whole Raid의 준비 subset root는7개다. `out/MarioWorldPrewarm20261004/check_pool_reservations.py`와 `pool-reservations.json`은 실제 게시 JSON 및 hash를 이용한 Python 참조 계산이다. C++ collector를 실행한 검사로 기록하지 않는다.

실제 캐시 메서드 두 개를 추출한 Debug C++ CPU fixture의48개 검사가 통과했다. hit에서 문서 복사1회/Build0회/Validate0회, identity 불일치 fallback, revision/Area 및 동일 revision 문서 교체, 자기 자신을 source로 사용하는 경우, 실패 시 receiver 보존을 확인했다. 기존5개 문서 교체 경로의 cache clear도 소스로 확인했다. `subset-cache-receipt.json`에 source SHA와 범위가 있다. fixture의 문서·target은 통제된 adapter이므로 실제 World validator 의미나 scene/GPU 검증을 대신하지 않는다.

실제 incremental 준비 함수3개를 추출한 Debug C++ CPU fixture83개 검사도 통과했다. 1call당 최대1개 생성, pending/ready, 기존 일괄 준비 의미,128/1024 전체 요청 한도, clone 실패·잘못된 type·owner map 삽입 bad_alloc·예외 rollback 및 기존 pool 보존을 확인했다. `step-receipt.json`에 source hash와 범위를 기록했다. 문서·Layer·모델은 통제된 CPU 대역이며 실제 GPU clone 성능을 검사한 결과는 아니다.

실제 Client 프로젝트의 Debug C++/리소스/링크 target은 exit0이다. 변경한3개 헤더에 의존하는 기존72개 TU의 OBJ가 모두 이번 빌드에서 갱신됐고 Debug Client.exe 링크와 의존 DLL 배포가 완료됐다. `product-cpp-verification.json`, `client-cpp-diagnostic.log`, `client-cpp.binlog`에 근거가 있다. 명령은 다음과 같다.

```powershell
& 'C:/Program Files/Microsoft Visual Studio/18/Insiders/MSBuild/Current/Bin/amd64/MSBuild.exe' Client/Default/Client.vcxproj '/t:PrepareForBuild;ResolveReferences;_ClCompile;_ResourceCompile;_Link;_Manifest;DeployClientRuntimeDependencies' /m:1 /nodeReuse:false /p:Configuration=Debug /p:Platform=x64 /p:PreferredToolArchitecture=x64 /p:WindowsSDKToolArchitecture=Native64Bit /p:BuildProjectReferences=false /p:CL_MPCount=3 /v:minimal
```

기존 헤더의 C4819/C4828 및 vendor PDB 경고는 남아 있다. 앞선 DataJson 작업의 전체 Product 빌드는 대규모 Shader 단계에서 명시 중단했으며 이 C++ 성공으로 전체 Shader/CSO 최신성을 인증하지 않는다. 별도 Release 제품 빌드나 Client/Server 실행은 하지 않았다. source/frame 설정·판정·게시 데이터 변경이 없어 이 변경만을 위한 raid Publish는 필요하지 않다.

독립 agent가 Step의 commit/rollback과 cache의 owner/identity/동일 revision 무효화를 검토했고 추가 결함을 발견하지 못했다. 6개 C++의 기존 BOM·CRLF 유지, `git diff --check`, 기존 미커밋 문서5개의 SHA-256 보존을 확인했다.

## G03. 남은 비용과 사용자 화면 확인 경계

V1 spawn은 GPU 리소스를 재사용해도 `CEffectPlayback::Stage_PrevalidatedDocumentInternal`에서 source dispatch와 인스턴스 상태를 준비한다. 처음 시점까지의 particle seek, bone/animation 평가, GPU draw 비용도 남는다. 이번 변경은 Effect runtime을 별도로 만들거나 그 검증을 제거하지 않는다. 준비 단계의 cold model `CModel::Create`도 동기이므로 한 update에 clone 하나라는 규칙을 프레임 시간 상한으로 설명하지 않는다.

사용자가 Debug 쿠크 아레나에서 저장·게시된 Mario 1~4의 1페이즈와 2페이즈를 Play Pattern 또는 Complete Play로 재생해 최초 등장과 반복 재생을 확인해야 한다. CPU 계약 검사와 C++ 링크 성공은 GPU 표시·프레임 시간·녹화 결과를 대신하지 않는다. 기존 준비 장벽을 거치지 않는 도구의 독립 Object Preview나 캐시 소진 이후 임의 동시 재생까지 프레임 지연이 없다고 보장하지 않는다.
