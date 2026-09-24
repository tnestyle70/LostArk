# 쿠크 체력 기믹 시험과 Effect·Sound 타임라인 편집 결과

## G00. 구현과 데이터 반영

기존 Workbench·Composition·PresentationPlayer·Server 피해 경로를 확장했다.
사용자 미커밋 저작 변경과 앞선 Play/Play Pattern 분리 코드를 보존했다.

- 창술사 ALT_V의 Retail `damage.player.34630`에 `bossHealthBarDamage:35`를 설정했다.
  ACTIVE 한 cast의 총 피해를 보스 최대 HP·최대 줄 수로 계산하고 여섯 HP 타격에
  누적 차분으로 분배한다. 전 타격 적중 시35줄이며 무적·실드·빗나감은 기존 판정이다.
  비보스와 필드 생략/0은 기존 계산이다. publisher·catalog·F1 저장 계약을 함께 연결했다.
- Effect `effectSourceStartMs`를 codec·projector·V1/V2 재생에 연결했다. 앞 edge는
  source-in과 시작·수명을, body 이동은 시작만 바꾼다. Fit/Loop·attachment·explicit fade의
  시간 기준을 분리해 source-in이 부착 위치나 박스 fade에 중복 적용되지 않게 했다.
- Sound 동종 그룹, 다중 선택 보존, Effect+Sound 혼합 선택의 동일 delta 이동을 연결했다.
  source-in·수명·fade·volume을 보존하며 stale drag는 전체 거절한다.
- Collider Apply는 표시 이름 대신 순수 ENTER_AREA의 `colliderDamageContactRole`과
  반복 정책을 확인한다. 피해는 DAMAGE, 수평 밀림·상승은 KNOCKBACK에 연결한다.
  실제 hold·비피해 Result·Fail/Timeout 등 기믹 연결은 자동 교체하지 않는다.

저장된 공통 Logic507 `트리거_대미지_넉백`과 Logic508 `트리거_대미지`에는 triggerKind가
없었다. ENTER_AREA와 KNOCKBACK/DAMAGE 역할을 채웠다. P47.logic.4와 .7은 같은 ID·시간·
Result509·Collider 링크를 유지하면서 잘못 재사용한 Logic32 대신507을 참조한다.
P17의 실제 잡기와 Logic32·33·34는 그대로다. 현재 두 피해 창은10% max HP와 수평1m,
250ms이며 상승값은0이다. 이 값을 임의로 상승 피해로 바꾸지 않았다.

Composition은 최신 revision2252에서2253으로 병합했다. P47.presentation.2/.3의 source-in을
4201ms로 설정했다. 시작0/4445ms는 유지했고 수명은9212→5011ms,8836→4635ms로 줄였다.
최신본을 다시 읽고 writer lock·전체 구조/표현 projection 검증·백업·byte 재확인·원자 교체를
수행했다. 해당 필드 이외의 JSON 의미가 보존됐음을 비교했다. 원본/대상 Trigger의 추가
mechanic과 서로 다른 반복 설정은 적용을 거부한다.

데이터 적용 영수증과 백업 위치는 `out/KoukuDamageEffectSound20260924/trumpet/applied.receipt.json`이다.

## G01. 자동 검증

| 검증 | 실제 결과 |
|---|---|
| Server native 피해 fixture |21 checks PASS. 실제6타, 서로 다른 보스 HP/줄 수, projectile·contact·fallback, 실드·무적·빗나감·비보스·정수 경계·ACTIVE 제한. |
| publisher DAMAGE projection |10 checks PASS. optional7열, 기존 행 호환, 잘못된 값/소유자 거부. |
| `--kouku-sound-timeline-contract` |PASS. Sound group/selection/mixed move, 경계·stale rollback, Collider closure, Effect/Sound trim, Fit clock, Save/reopen. |
| `--kouku-fixed-damage-contract` |PASS. fixed/vertical/shared Result COW, common role, legacy rewire, Hold/capture/Success 보존, invalid rollback, Save/reopen. |
| Python focused |source-in2, role1, selection group7, SHOWTIME2의12개 PASS. |
| 넓은 Python effect 검사 |5개 메서드의6FAIL+4ERROR. HEAD projector에서도 같은 실패를 재현해 기존 실패로 분리했다. 전체 suite 통과로 기록하지 않는다. |
| 최신 데이터 재연결 guard |원본/대상 Charge 및 반복 설정6변형 거부, 동일 정책 허용, P17 보존 PASS. |
| 데이터/스크립트/프로젝트 구조 |Composition 전체 검증·presentation projection, Retail JSON, 변경 PS parse, Client/Server XML PASS. |
| `git diff --check` |PASS. |

native 원본83개 element를 실제 codec·playback으로 Stage/Seek했다. authored earliest birth는
4184ms이나4184/4200ms에는 입자가 없고4201ms에서 처음 총34개, floor26개, 양수 alpha11개를
얻었다. source asset은 수정하지 않았다. CPU 표본 결과이며 GPU 화면 판정으로 기록하지 않는다.

증거는 `out/BossHealthBarDamage20260924/{probe,projection}.log`,
`out/KoukuDamageEffectSound20260924-{sound-timeline,fixed-damage}.log`,
`out/KoukuDamageEffectSound20260924/source-in-probe/sample.log`,
`out/KoukuDamageEffectSound20260924/trumpet/contact-guard-check.json`이다.

## G02. 컴파일과 실행 파일

VS18 Insiders amd64 MSBuild, v14314.44.35207 HostX64로 정상 x64 Debug/Release Build를 수행했다.
Shared·Engine 소스/ABI는 바꾸지 않았고 기존 정상 reference 산출물을 사용했다. 자동 Data publish는
끄고 명시적인 Kouku owner 게시를 별도로 수행한다. 새 제품 파일이나 project/filter 등록은 없다.

- Client Debug/Release 정상 Build·링크·shader/runtime DLL 배포 PASS, 오류0.
- Server Debug/Release 정상 Build·링크 PASS, 경고0·오류0.
- Client의 기존 C4819/C4828 인코딩 경고와 DirectXTK PDB 누락 LNK4099는 남아 있다.
- 최종 정상 Build에서 FxCompile은 최신 상태로 확인됐다. HLSL 입력은 변경하지 않았다.
- 최초 별도 OutDir Client Build가 불필요한 shader 재생성을 시작해 해당 에이전트의 MSBuild
  tree만 중단했다. 이어 C++ 컴파일을 마치고 정상 출력 경로의 Build로 최종 링크·배포했다.
  단독 ClCompile/Link invocation은 Client EXE 생성 증거로 사용하지 않았다.

갱신 경로는 `Client/Bin/Debug/Client.exe`, `Client/Bin/Release/Client.exe`,
`Server/Bin/Debug/Server.exe`, `Server/Bin/Release/Server.exe`다. 최종 링크 시 두 게임 프로세스가
종료된 것을 확인했다. Client/Server를 자동 실행·종료하거나 UI를 조작하지 않았다.
로그는 `out/KoukuDamageEffectSound20260924/*-{debug,release}-installed.log`에 있다.

## G03. 게시와 사용자 확인 경계

저장본 revision2253을 대상으로 아래 명시적 게시가 exit0으로 완료됐다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 2253
```

Kouku product·해당 world·gameplay balance는 PASS, map은 동일 fingerprint로 REUSED였다.
총119444ms이며 로그는 `out/KoukuDamageEffectSound20260924/publish-kouku-owner.log`다.
게시 후 원본 hash가 그대로인지, 실제 P17 잡기 보존, 두 표현 박스의 source-in4201과
시작/수명, 두 공통 Trigger 창과 Result509 연결, 생성 DAMAGE7열의 최종값35를 다시
확인했다. 원본 Effect asset hash도 동일하다. 모두 PASS이며 결과는
`out/KoukuDamageEffectSound20260924/final-data-verification.json`이다.

Client/Server는 현재 종료 상태다. 실행 중 메모리 갱신이나 사용자 최종 화면 검증은
수행하지 않았다. 생성 bootstrap과 실행 파일은 정상 publisher/Build 결과이며 수동 편집하지 않았다.

사용자 확인은 Server와 Client를 새 실행 파일로 시작한 뒤 공통 Trigger/Collider Apply,
Play의 로컬 timeline 조작과 Play Pattern의 Server 피해, 창술사 ALT_V 전 타격35줄,
P47 장판 시작과 Effect/Sound 다중 이동을 대상으로 한다. Release의 게시 패턴 테스트와
Debug의 저작 도구 범위는 기존 계약을 유지한다.
