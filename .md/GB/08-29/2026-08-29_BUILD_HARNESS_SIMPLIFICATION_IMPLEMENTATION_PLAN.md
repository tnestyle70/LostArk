# Build / Harness 단순화 구현 계획

## 1. 목표

- Visual Studio 기본 빌드에서 제품과 하네스를 분리한다.
- 하네스가 제품 CPP를 직접 다시 컴파일하는 비용을 일상 빌드에서 제거한다.
- 빌드, headless contract, 실제 Client 수동 판정이 각각 무엇을 증명하는지 분리한다.
- 삭제된 Effect Imported/receipt/candidate를 회귀 통과만을 위해 복원하지 않는다.
- 고유 assertion은 이관 전 삭제하지 않고, 의미 없는 프로젝트 shell과 stale corpus만 단계적으로 퇴역한다.

## 2. 실측 기준선

| 항목 | 변경 전 |
|---|---:|
| Framework.sln 전체 ClCompile entry | 329 |
| unique CPP | 271 |
| 중복 compile edge | 58 |
| 제품 CPP를 직접 컴파일하는 하네스 수 | 6 |
| 하네스가 직접 다시 컴파일하는 제품 LOC | 125,948 |
| EffectRender 직접 제품 CPP | 28 / 86,285 LOC |
| ActionTimeline 직접 제품 CPP | 15 / 30,288 LOC |

기존 full Debug x64 Rebuild 관측은 약 10분 25초였고 EffectRender가 critical path였다.
사용자가 경험한 3시간/16시간은 단일 필수 빌드가 아니라 직렬 MSBuild, broad harness,
Debug/Release 반복, late gate 실패와 재실행이 누적된 결과로 본다.

## 3. 증명 경계

| 검증 | 증명하는 것 | 증명하지 않는 것 |
|---|---|---|
| Product build | 현재 TU의 compile/link, build hook, CSO 생성/배포 | 발탄 Arena의 화면·소리·타이밍 |
| JSON/publisher | schema, ID, join, resource closure처럼 명시된 invariant | 필요한 cue가 애초에 모델에 선언되지 않은 경우 |
| headless C++ contract | executable에 적힌 assertion과 fixture | 실제 Client Level, GPU hardware, FMOD 청취 결과 |
| live Server probe | 실제 Server process의 protocol/room/isolation | Client presentation/UI |
| 사용자 smoke | 실제 화면, 음향, 조작, occurrence 체감 | 자동 재현 가능한 회귀 원인 전체 |

Codex는 Client를 자율 실행·조작하거나 화면/음향을 대신 PASS하지 않는다. 자동화 결과는 위 경계까지만 보고한다.

## 4. 구현 G

### G1. 솔루션 기본 Build를 제품 전용으로 분리

- `Framework.sln`의 harness `.Build.0`만 제거한다.
- project와 `.ActiveCfg`는 유지해 개별 열람/빌드를 보존한다.
- 결과 목표는 Client 148 + Engine 74 + Shared 7 + Server 29 = 258 TU다.

### G2. x64 제품 compile 병렬화

- Engine, Shared, Server, Client의 Debug/Release x64에 `MultiProcessorCompilation=true`를 둔다.
- Engine -> UpdateLib -> Client dependency 순서는 유지한다.

### G3. 정본 runner profile 분리

- `Product`: 제품 4개와 product CSO closure.
- `Core`: Product + publisher + NetworkProtocol + 실제 Server Character Select `Core` scenario 1회(private·shared world 격리).
- `FullDiagnostic`: Core + Character Select `Party2`/`Party4` transfer + presentation/map/light/physics/model/Server 광역 진단.
- 삭제된 Artist Imported candidate를 무조건 요구하는 EffectRender broad 실행과 Artist oracle은 활성 profile에서 격리한다.
- `SkipBuild`와 local resource 허용은 Git-only 재현 PASS로 사용하지 않는다.

### G4. stale Effect build metadata 제거

- 삭제된 Imported/Contracts/receipt 전용 `.gitattributes` 규칙을 제거한다.
- 현재 Authored 문서와 전역 LFS 규칙은 유지한다.
- 삭제된 DimensionMaster source receipt를 계속 요구한 targeting preview는 체크인된 runtime resource 권위로 교정한다.

### G5. assertion 이관 후 프로젝트 퇴역

1. NetworkProtocol은 존치하되 5,473줄 단일 TU를 domain별로 분리한다.
2. `ValtanFourPlayerHarness`는 고정 4인·5번째 거부를 제품 요구로 굳히므로 퇴역한다. 일반 `ROOM_FULL` codec과 Server의 atomic admission/reset 계약은 기존 제품 contract에 남기고, CharacterSelectIsolation은 private/shared world 경계 때문에 유지한다.
3. Physics + WModel core는 `EngineContractHarness`로 이관한다.
4. PointLight + Map numeric/WARP는 `RenderingContractTests`로 이관한다.
5. ActionTimeline + Audition + Effect CPU contract는 `ClientPresentationCore` static library를 Client와 테스트가 함께 링크하도록 바꾼다.
6. assertions 이관이 완료된 뒤 기존 standalone project shell을 삭제한다.

## 5. 발탄 sound 누락의 구조적 후속 작업

현재 `VALTAN_TRASH* / CATCH_SLAM / DAMAGE_GRABBED_PLAYERS`는 gameplay에 있으나 정확한
`CATCH_SLAM` sound/effect/shake cue가 0개다. 기존 sound harness는 존재하는 511행의 join만
검증하므로 누락 자체를 PASS할 수 있다.

- 기존 `Data/Valtan/Valtan.presentation.json`을 두 번째 정본 없이 확장한다.
- stage/action마다 `requiredPresentationRoles`를 선언한다.
- 역할은 `ANIMATION`, `EFFECT`, `SOUND`, `SHAKE`, `CAMERA`, `WORLD_EVENT`다.
- 필수 역할은 정확히 한 cue 또는 `NONE + reason`을 요구한다.
- projector가 현행 runtime sound/shake/effect 문서를 생성하거나 검사한다.
- validator는 event -> SoundCatalog variant -> 실제 WAV까지 closure를 검사한다.
- runtime presentation은 계속 fail-open하되 authoring/publish/CI는 필수 slot에 fail-closed한다.

## 6. Git 재현성 후속 작업

- cleanup 변경을 하나의 commit SHA로 만든 뒤 clean checkout에서 검사한다.
- V2 Authored 48개 resource 중 local-only 42개와 Sound WAV 2,905개 tracked 0 문제를 닫는다.
- 전체 pack manifest를 중복 정본으로 만들지 않고 기존 Catalog/Bindings에서 dependency closure를 유도한다.
- `Sound`를 Resources 공식 7번째 domain으로 인정할지, 기존 6-domain 아래로 이관할지 public 계약을 한 번 결정한다.
- build 전 LFS hydration, required Fonts, V1/V2/Character/UI/Map/Sound closure를 빠르게 검사한다.

## 7. 검증

- solution `.Build.0` 수와 product TU 수 검사
- 모든 변경 XML/PowerShell/JSON parse
- `git diff --check`
- Client Debug x64 Rebuild 시간 측정
- `Framework.sln` Debug x64 제품 Build
- `Publish-GameplayBalance.ps1 -Mode Validate`
- runner `Product` 실행
- runner `Core -AllowLocalEffectResources -SkipBuild` 실행
- visual/audio PASS는 사용자 smoke로 남김
