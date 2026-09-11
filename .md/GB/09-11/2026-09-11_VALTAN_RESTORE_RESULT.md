# 발탄 재질·애니메이션·Composition 연결 복구 결과

작성일: 2026-09-11. 구현과 Gameplay·바닥 배포, 유령 program84 수정을 포함한 최종 Debug Product 재빌드·런타임 배포 완료(exit0). 사용자의 마지막 요청에 따라 최종 재빌드 결과 확인 후 추가 검사를 실행하지 않고 종료했다. Client 화면·음향·패턴 실행은 사용자 확인 전이다.

## G00. 범위와 작업 상태

시작 브랜치는 `codex/kouku-full-material-lighting-restoration`, 시작 HEAD는 `e26cd2b282293bd4cb433338bdbc6a7315dd3f84`다. 이전 쿠크·베른·캐릭터 이펙트의 대규모 미커밋 변경을 유지했다. 이 상태에서는 자동 stage/commit하지 않았다. Resources는 Git에 추가하지 않는다.

첨부 화면의 중앙 원형 바닥과 바깥 청록 균열 석판이 서로 다른 표면임을 관찰했다. 정지 화면만으로 실제 animation이나 material fidelity를 PASS로 판단하지 않았다. Client/UI를 실행·조작하거나 화면을 캡처하지 않았다.

## G01. 발탄 본체·장비·유령의 재질

본체·갑옷·도끼·유령의 5개 모델 11개 slot을 BossCatalog override와 실제 `CValtanPresentationAssetService`의 `MODEL_LOAD_DESC`로 연결했다. 정상 발탄의 8개 slot은 기존 source program21, 유령 3개 slot은 program84를 사용한다. 원본 MIC 8개에 필요한 texture 26개를 `Resources/Character/SourceMaterials/Valtan`에 배치했다. 기존 source program은 보존했다.

첫 실제 모델 bind 검사에서 초기 유령 program38이 기존 map material 예약 범위와 충돌하는 것을 확인했다. 비충돌 84로 옮기고 CModel admission과 양쪽 shader dispatcher를 함께 수정했다. shader 식 비교만으로 실제 CMaterial의 분기·deferred row 등록까지 검증했다고 처리하지 않았다.

유령 cloud/rim/opacity 식을 연결했으나 원작 sorted translucency와 scene hemisphere 전체를 복원한 것은 아니다. 현재 deferred의 ordered coverage 경계를 유지한다. 정확한 MIC·shader·slot·물리 경로와 수치 검증은 같은 날짜의 material 전용 RESULT가 소유한다.

중앙 circlefloor는 다른 장소의 rain MIC가 남아 있던 2개 slot을 해당 level의 원본 MIC로 수정했다. 원본 1185개 정점과 2346개 index를 재추출해 UV1·tangent W·80종 component COLOR0를 기존 geometry 경로에 연결했다. 기존 overlay shader family와 RNM을 사용하며, placement `15561800956777256508` 하나만 원본 조명 variant로 교체했다. 원본 두 slot의 서로 다른 밝기·normal 값은 유지했다. Valtan Area publisher는 13184개 placement·7개 출력 검증과 publish를 통과했다.

## G02. 중앙 idle 고정의 원인과 수정

기존 native admission에서 `BossCatalog Effect V2 owner header is invalid`를 재현했다. 발탄 전용 C++ reader가 재질용 optional `modelMaterialOverrides` root를 거부해 `CClientReplication`이 `CValtan::Apply_NetworkState`를 건너뛰었다. Server 전투 상태와 HUD는 진행하지만 보이는 발탄의 transform·animation은 spawn 상태에 남는 경로였다.

정상 optional array를 허용하고 unknown field·잘못된 자료형·128개 초과를 거부한다. 기존 receipt rollback과 hash 검증은 유지한다. 실제 body animation 대상은 렌더링하는 `Part_Body/Com_Model`과 같다. 보스를 하나 더 만들거나 Client local AI로 우회하지 않았다.

수정 전 FAIL 및 수정 후 native admission PASS, 173개 native animation과 222개 authored occurrence의 이름·source 구간 대조는 [animation RESULT](2026-09-11_VALTAN_PRESENTATION_ADMISSION_ANIMATION_RESULT.md)에 기록했다.

별도로 local Composition Preview 뒤 `Play on Server`를 누르면 기존 idle/재생 clone이 화면에 남는 경로도 확인했다. Server submit 성공 때만 해당 발탄 clone의 transport를 중지하고 기존 PreviewPanel 경로로 해제한다. Animation Tool의 다음 프레임 auto-stage도 억제하며, 명시적 local Stage/Retry 성공 또는 Level 변경에서 다시 허용한다. Server submit 실패와 다른 asset preview는 유지한다. Server 발탄의 visibility flag나 snapshot 권위를 변경하지 않는다.

## G03. Composition에서 편집하고 실행하는 것

현재 Product 목록은 65개이며 관리 패턴 42개와 compatibility 참조 23개다. legacy source 자체의 25개와 Product의 중복 제거 후 개수를 구분한다. 관리 패턴은 기존 split writer와 Server Play를 사용하고 compatibility는 기존 트랙·local Preview 참조로 남긴다.

- V2 box를 합성 이름 대신 stable `bindingId`로 선택한다. animation occurrence clock을 실제 Stage 위치로 환산하며 `VALTAN_BIND_SLOT` shout 세 개는 1400/2300/3200ms로 분리된다.
- V2 상세에서 clock/occurrence, anchor, follow, rotation basis, local TRS, repeat와 stop을 편집한다. `Apply V2 Binding`은 draft에 반영하고 `Save`가 기존 owner transaction으로 저장한다. 잘못된 payload나 stale 원본은 기존 snapshot·파일을 유지한다.
- runtime이 전체 local S*R*T를 소비하도록 수정했다. billboard/particle에도 binding 크기를 전달하고 기존 group child의 크기를 중복 적용하지 않는다.
- 독립 combat object 9개를 `World Objects` 탭과 별도 타임라인에서 소유 Pattern/Stage로 연결했다. 도넛 반지름과 도끼 개수는 기존 typed writer로 편집한다. 이는 임의 모델·archetype을 추가하는 범용 World 배치 writer가 아니다.

기존 V2 102개 binding, Pattern Sound 726개, combat-object Sound 9개, Shake 128개, Camera 11개를 보존했다. 실제 V2 dependency closure는 leaf 59개·group 12개·물리 리소스 66개이며 누락은 없었다. 도넛·돌의 기존 V1 표현을 이름이 비슷한 V2로 교체하지 않았다. 특히 돌 V2 해제는 기존 사용자 저작 V1을 보존한 09-04 결정이다.

Sound 133개 event 중 132개가 연결하는 WAV 251개와 BGM 5개는 파일·헤더가 정상이다. `G_Voltan1_Attack13_Loop1`은 이전부터 빈 항목이며 현재 추출 원본에서도 대응 소리를 찾지 못했다. 다른 WAV를 임의로 배정하지 않았다.

상세 구현과 편집 범위는 [Composition RESULT](2026-09-11_VALTAN_COMPOSITION_RESTORATION_RESULT.md)에 기록했다.

## G04. Gameplay 배포와 검증

다음 정본 publisher를 실행해 성공했다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
```

첫 실행은 기존 쿠크 map light 512개 계약에 남아 있던 Composition의 중복 64개 제한으로 실패했다. `_join_light_resources`가 이미 실행한 map owner 검증을 소비하도록 수정했고, 원본 조명이나 다른 작업의 변경을 삭제하지 않았다. 기존 테스트에 512개 성공·513개 거부를 추가해 수정 전 실패를 재현하고, 수정 후 관련 12개 테스트가 통과했다.

최종 발탄 presentation generation은 `7f441540a11ca800a871a6088bb1eae1ff4c0cfbabf6be9f9487adba59c89462`다. source artifact 144개와 `Gameplay.bootstrap`의 `PATTERNPRESENTATIONGENERATION` 값이 일치한다. 기존 player hit-shape coverage 경고는 남아 있으며 이번 발탄 복구의 성공으로 그 항목을 완료 처리하지 않았다.

기존 Valtan Sound/Camera/ring 관련 12개 검사, Composition 관련 56개 source/data 회귀, 실제 runtime `Binding_Local` 본문을 사용하는 독립 DirectXMath 수치 검증이 통과했다. clone 해제도 실제 함수 본문의 CPU fixture에서 submit 성공/실패·dirty 보존·다른 asset 보존을 확인했다. 별도 기존 source 검사 30개 중 29개는 통과했고, 1개는 이번에 수정하지 않은 Effect Tool의 camera-only bone index와 오래된 문자열 기대값의 불일치로 실패했다.

첫 전체 Debug Product 빌드와 배포는 exit0으로 통과했다. `out/BuildPipeline/runs/20260910T202254944Z-debug-product.json`의 실행 시간은 906871ms이며 missing runtime input은 없다. 마지막 Animation Tool·MainApp·Workbench·V2 Runtime 수정 이후 해당 object가 컴파일된 것도 확인했다. 이후 실제 CModel 검사에서 정상 발탄 8개 slot의 create/clone/prototype 해제 뒤 bind는 통과했고 유령38의 충돌을 발견해84로 수정했다. **사용자의 종료 요청으로 program84 이후 CModel 재검사와 circlefloor 실제 bind 검사는 실행하지 않는다.**

확장한 기존 native harness의 Debug compile은 성공했다. API fixture의 첫 실행은 harness 옆 Resources를 찾으면서 실패했고, 실제 `Client/Bin/Resources`를 명시한 같은 executable의 전체 admission/API 실행은 exit0 PASS였다. 이후 test에 scoped ResourceRoot 설정·복원을 추가했으며 그 소스의 재컴파일·기본 환경 재실행은 종료 요청에 따라 생략한다. 실제 파일 저장 성공이나 Client UI 입력을 검증한 것으로 표현하지 않는다.

최종 바닥 배포 이후 변경·신규 파일의 JSON 42개와 project/filter XML 6개가 parse를 통과했고, 그 시점의 전체 `git diff --check`도 exit0이었다. 이후 문서·program84 및 harness root 보완은 각각 담당자가 scoped diff 검사를 통과했다. 빌드에는 기존 C4828/FXC X4000/DirectXTK PDB 경고가 있으므로 warning-free 빌드라고 하지 않는다.

최종 재빌드는 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`로 실행해 exit0이었다. Engine → Shared → Server → Client와 Engine shader/DLL 및 외부 runtime 배포가 완료됐다. 최종 기록은 `out/BuildPipeline/runs/20260910T204023737Z-debug-product.json`, 로그는 `%TEMP%/LostArkValtanRestore20260911/product-build-final.log`다. C4819 등 기존 인코딩 경고는 남아 있으나 빌드 오류는 없었다. 이 결과 확인 이후 사용자 지시에 따라 새 검사·실행·코드 변경 없이 종료했다.

작업 중 외부 `out` 정리로 이전 원본 조사 helper와 검증 로그 일부가 삭제됐다. 소스와 Resources는 유지됐다. 현재 빌드·배포 로그는 `%TEMP%/LostArkValtanRestore20260911`에 보존하며, 없어진 로그를 재생성된 증거로 가장하지 않는다.

## G05. 사용자 실행과 남은 확인

LAN 설정은 `server-host`, TCP 7777 LocalSubnet 준비로 완료했다. Client 접속 주소는 `192.168.0.14:7777`이다.

1. Visual Studio `Server + Client` profile을 `Ctrl+F5`로 실행하고 Lobby → Valtan으로 입장한다.
2. F1 → `Open Action Workbench` → Boss `Valtan` → `Composition Patterns`에서 관리 패턴을 선택한다. `Server Playback`의 **`Play on Server`**로 화면 발탄의 위치·pose·이펙트·사운드·카메라를 확인한다.
3. `Composition Resources → World Objects`에서 `donut`을 검색하고 도넛 또는 큰 도넛 항목을 선택한 뒤 `Play on Server`를 누른다. 발탄과 독립된 생성 위치와 지연 판정을 확인한다.
4. V2 box → Box Detail → `Apply V2 Binding` → Composition Sequencer `Save`로 편집한다. 제품 변경은 기존 `Publish after Save` 완료 뒤 Server 재시작·Valtan 재입장으로 확인한다.

Sequencer의 일반 `Play`는 local clone의 지원 Animation/Effect 미리보기다. 도넛·Sound·Camera를 포함한 전체 Server 패턴 실행으로 사용하지 않는다. 재질 외형, 유령 투명도, 바닥 경계, 실제 소리·카메라 timing과 사용자 저장/실행 결과는 아직 수동 확인 전이다.
