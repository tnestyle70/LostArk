# BossToolTests — 보스 도구 자동 검사

발탄·쿠크 보스 도구를 수정했을 때 **기존 기능이 깨졌는지 검사하는 콘솔 프로그램**이다.
이전 프로젝트 이름은 `ValtanPatternAuditionServiceHarness`였다. 검사 범위가 쿠크와 공용 편집 코드까지 넓어져 `BossToolTests`로 바꿨다.

## 무엇을 검사하나

- **패턴 재생 요청:** Play, Next 예약, Restart, Clear 요청에 서버 응답을 흉내 낸 입력을 주어 상태가 올바르게 바뀌는지 검사한다. 늦은 응답, 거절, 타임아웃, 재시도 때 기존 예약과 요청 ID를 보존하는지도 확인한다.
- **편집 문서:** 패턴·애니메이션·이펙트·사운드 데이터의 읽기, 검증, 저장, 재로드와 실패 시 기존 상태 보존을 검사한다.
- **그래프·연출 계산:** 패턴 연결, 선택한 분기, 타임라인, 카메라·본 부착 계산 등을 검사한다.

실제 Client 서비스·문서 코드를 이 테스트 EXE에 함께 컴파일한다. 테스트 입력을 넣고 결과를 기대값과 비교한다. 게임 화면을 띄우거나 실제 서버의 패킷 처리량을 측정하지 않는다. IOCP·Job System 성능 비교와는 별도 프로젝트다.

## Visual Studio에서 읽는 순서

`Framework.sln`의 **BossToolTests** 프로젝트를 펼치고 `00.Start/README.md`부터 읽는다.
기본 솔루션 빌드에서는 제외되어 있으므로 필요한 때 이 프로젝트를 우클릭해 **빌드**한다.

| 필터 | 내용 |
|---|---|
| `00.Start` | 이 설명서와 `Main.cpp`. `main()`에서 실행 옵션과 테스트 호출 순서를 확인한다. |
| `01.Tests.Valtan` | 발탄의 흐름·튜닝·문서·연출 검증 사례. |
| `02.Tests.SharedAndKouku` | 공용 그래프·편집기·디버그·본 부착 검사. 쿠크 편집 검사는 `BossCompositionDocumentContractTests.cpp`에 있다. |
| `03.Client.Services` | 검사하는 실제 Client의 재생 요청·흐름·튜닝 서비스. |
| `04.Client.Documents` | 검사하는 실제 Client의 편집 문서·그래프·워크벤치 코드. |
| `05.Client.Presentation` | 검사하는 실제 Client의 연출·카메라·변환 코드. |
| `06.Client.Support` | 문서 파싱·경로·리소스 조회 등 보조 코드. |

`03`~`06`의 파일은 `Client/Private`, `Client/Public`에 있는 원본을 참조한다. 여기서 편집하면 Client 원본이 바뀐다. 필터는 VS 표시용 분류이며 실제 복사 폴더가 아니다.

## 실행과 결과

빌드 결과는 `Tools/BossToolTests/Bin/<Debug 또는 Release>/BossToolTests.exe`이다.
VS에서 이 프로젝트를 시작 프로젝트로 선택하면 콘솔 검사 프로그램을 실행할 수 있다.
디버거 작업 디렉터리는 저장소 루트로 설정되어 있다. 명령행에서도 저장소 루트를 작업 디렉터리로 사용한다.

```powershell
& '.\Tools\BossToolTests\Bin\Debug\BossToolTests.exe'
& '.\Tools\BossToolTests\Bin\Debug\BossToolTests.exe' --action-composition-graph-contract
```

인자가 없으면 audition 서비스 30개 사례와 추가 14개 그룹의 **기본 회귀 검사**를 실행한다. 처음 출력되는 `30/30 passed`는 audition 서비스만의 결과다. 기본 실행이 아래 모든 개별 옵션을 포함하지는 않는다.

| 개별 옵션 | 검사 범위 |
|---|---|
| `--action-composition-graph-contract` | 패턴 그래프와 분기·시간 계산 |
| `--cinematic-view-rebase-contract` | 시네마틱 카메라 view 재계산 |
| `--valtan-presentation-contract` | 발탄 연출 계약 |
| `--presentation-generation-admission-contract` | 연출 세대·리소스 승인 |
| `--kouku-pattern-delete-contract` | 쿠크 패턴 삭제 |
| `--kouku-collider-duplicate-contract` | 쿠크 collider 복제 |
| `--kouku-fixed-damage-contract` | 쿠크 고정 피해 편집 |
| `--kouku-collider-group-contract` | 쿠크 collider 그룹 |
| `--kouku-sound-timeline-contract` | 쿠크 사운드 타임라인 |
| `--kouku-preview-transport-contract` | 쿠크 preview/publisher 호출 경계 |
| `--kouku-sequence-document-contract` | 쿠크 sequence 문서 |
| `--kouku-parent-timing-contract` | 쿠크 순차 부모 시간 |
| `--kouku-independent-row-clock-contract` | 쿠크 독립 row 시간 |
| `--kouku-world-effect-frame-contract` | 쿠크 world 이펙트 좌표 |
| `--kouku-composition-editor-contract` | 쿠크 composition 편집 |

옵션은 한 번에 하나를 지정한다. 종료 코드는 성공 `0`, 검사 실패 `1`, 잘못된 인자 `2`이다. 실패 시 `FAIL` 행과 마지막 종료 코드를 확인한다.

## 실제 데이터와 검사 한계

일부 검사는 현재 `Data` 문서와 `Client/Bin/Resources` 참조를 읽는다. 저장·충돌 검사는 주로 임시 복사본에 수행한다. 생성 문서 저장 거부 검사는 실제 원본 경로에 저장을 시도하되 거부와 원본 바이트 보존을 확인한다. 쿠크 preview transport 검사는 임시 PowerShell publisher stub을 자식 프로세스로 실행한다.

검사 통과는 게임의 최종 화면, GPU 표시, 실제 서버 연결, 패킷 처리 속도까지 확인했다는 뜻이 아니다.
내부 매크로 `LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS`는 기존 Client 테스트 연결과 호환되도록 유지한다.
