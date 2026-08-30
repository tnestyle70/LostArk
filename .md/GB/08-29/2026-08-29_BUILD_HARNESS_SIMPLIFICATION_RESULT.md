# Build / Harness 단순화 결과

## 1. 완료 상태

### 구현 완료

- `Framework.sln` 기본 Build를 제품 4개 전용으로 변경했다.
- 하네스 9개 project와 ActiveCfg는 보존했고 `.Build.0`을 제거했다. 고정 4인 제품 요구를 굳히던 `ValtanFourPlayerHarness` project/wrapper는 후속 감사에서 퇴역했다.
- 기본 solution compile entry는 329 -> 258, 71개 감소했다.
- 제품 CPP 중복 compile edge 58개가 기본 빌드 critical path에서 빠졌다.
- Engine/Shared/Server/Client x64 Debug/Release에 병렬 compile을 활성화했다.
- build runner를 `Product`, `Core`, `FullDiagnostic` profile로 분리했다.
- runner가 명시적으로 선행 프로젝트를 빌드한 뒤에는 `BuildProjectReferences=false`로 같은 Engine/Shared를 반복 평가하지 않게 했다.
- product CSO closure가 EffectRender/PointLight stale output을 요구하지 않도록 module scope를 분리했다.
- 삭제된 Artist candidate/oracle에 묶인 EffectRender broad 실행을 활성 profile에서 격리했다.
- `UpdateLib.bat`의 `Engine/Bin/<Config>/*.lib` wildcard를 `Engine.lib` 한 파일로 좁혔다.
- 삭제된 Effect Imported/Contracts 경로의 stale `.gitattributes` 규칙을 제거했다.
- DimensionMaster 2050500 targeting preview의 삭제된 receipt 의존을 runtime resource 권위로 교정했다.

### 이관 전 보존

- NetworkProtocol과 live Server Character Select `Core` private·shared world isolation assertion
- Action timeline sound/camera/encounter assertion
- Audition lifecycle/Flow/Tuning assertion
- Physics atomic document/PhysX assertion
- WModel frozen golden/corruption assertion
- Map numeric oracle와 PointLight CPU/WARP assertion

이 assertion들은 신규 공용 library/contract executable로 이관하기 전 소스 삭제하지 않았다.

## 2. 실측 결과

| 검증 | 결과 | 시간/수치 |
|---|---|---:|
| Client Debug x64 Rebuild | PASS, 오류 0 | 126.09초 |
| 이전 full solution에서 관측한 Client leg | 기준선 | 약 532초 |
| Framework.sln product Build | PASS, 경고 0/오류 0 | 21.87초 최종 재검사 |
| runner Product | PASS | 26.02초 |
| runner Core, SkipBuild, local Effect 허용, 기존 `Scenario=All` | PASS | 83.64초 |
| runner Core, SkipBuild, local Effect 허용, `Scenario=Core` 분리 후 | PASS | 52.45초 |
| Framework.sln product Build, FourPlayer project 제거 후 | PASS, 오류 0 | 34.28초 |
| Gameplay balance Validate | PASS | 6 profiles / 230 skills / 109 damage profiles |
| product CSO closure | PASS | 23 producers / 22 Client consumers |

Client Rebuild 비교는 top-level invocation이 달라 절대 benchmark로 사용하지 않지만, 같은 PC에서
직렬 compile 병목이 크게 줄었다는 실측 근거다.

## 3. 의도적으로 분리한 실패

- Git-only Effect validation은 local-only resource 3개 때문에 현재 FAIL이 맞다.
  `AllowLocalEffectResources` PASS는 이 PC의 local pack 검증이지 teammate clone 재현 증거가 아니다.
- `Server.exe --contract-test`는 현재 Valtan audition 5개와 fall revive 1개, 총 6개 기존 회귀가 있어
  `FullDiagnostic`에 남겼다. 실제 게임이 실행된다는 사실만으로 이 assertion을 자동 삭제하지 않는다.
- EffectRender broad executable은 삭제된 15,121,873-byte Artist candidate를 모든 mode 전에 요구해
  현 cleanup에서 exit 2다. candidate를 복원하지 않고 현재 Product fixture로 고유 assertion을 이관한다.
- visual/audio smoke는 실행하지 않았고 PASS로 기록하지 않는다.

## 4. 발탄 바닥 타격음 조사 결과

- gameplay에는 `VALTAN_TRASH`, `VALTAN_TRASH_CATCH_SUCCESS`, `VALTAN_TRASH_CATCH_IF`의
  `CATCH_SLAM -> DAMAGE_GRABBED_PLAYERS`가 존재한다.
- 현 sound cue 문서에는 `CATCH_SLAM` exact tuple이 0개다.
- effect/shake도 해당 exact tuple이 0개다.
- 기존 Action harness는 sound 511행을 active 497 + explicit NONE 14로 join할 뿐,
  필수 semantic role 자체의 누락을 표현하지 않는다.
- Client runtime은 catalog variant가 없으면 해당 sound만 건너뛰므로 Arena와 gameplay는 계속 실행된다.

따라서 이 누락은 하네스 실행 부족이 아니라 schema가 “필수 sound slot”을 소유하지 않은 구조 문제다.

## 5. Git / 물리 폴더 재현성 판정

- 감사 시점 cleanup은 대규모 working-tree 변경이며 아직 commit SHA에 없다. 같은 HEAD checkout은 정리 전
  Data/Effects 2,405개와 project 등록을 복원한다.
- Resources 물리 22,229개 중 tracked는 1,057개다.
- Effect V2 resource 48개 중 42개가 local-only다.
- SoundCatalog의 WAV 2,905개는 현 PC에 있지만 tracked 0이다.
- Character direct model 49개도 tracked 0이다.
- Resources 6-folder 문서 계약은 실제 `Sound/...` runtime 소비와 모순된다.
- root `Intermediate/` ignore는 `.git/info/exclude`의 이 PC 전용 규칙이다.

현재 판정은 `same commit != same runtime`이다. cleanup commit, Product dependency closure와 public Sound
domain 결정 전에는 다른 팀원의 full visual/audio 재현을 보장할 수 없다.

## 6. 남은 순서

1. cleanup 전체를 검토 가능한 단일 commit/PR 단위로 확정한다.
2. V2/Sound/Character/UI/Map dependency closure validator를 기존 Catalog에서 유도한다.
3. Sound domain public 계약을 결정한다.
4. `ClientPresentationCore`를 추출해 Action/Audition/Effect CPU tests와 Client가 같은 artifact를 링크한다.
5. `requiredPresentationRoles`를 Valtan presentation master에 추가해 CATCH_SLAM 같은 누락을 fail-closed한다.
6. assertions 이관 뒤 남은 broad standalone harness project와 stale modes를 삭제한다.
7. 사용자 Client에서 발탄 버러지 catch slam의 sound/effect/shake occurrence를 직접 판정한다.
