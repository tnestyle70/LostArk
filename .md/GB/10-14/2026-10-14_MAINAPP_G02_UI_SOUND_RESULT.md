# G02 UI 사운드 선택 분리 결과

작업 주제일: 2026-10-14. 실제 반영·빌드 확인: 2026-10-15 00시대 KST.
[G02 PLAN](2026-10-14_MAINAPP_G02_UI_SOUND_PLAN.md)의 소스 반영과 Debug Product 빌드·링크를 확인했다.
Client 실행·키 입력 재현·수동 청취는 수행하지 않았다.

## 실제 반영

`Client::UI_SOUND_CUE`와 `Client::Play_UISound`가 공통 UI의 네 WAV 선택을 맡는다.
재생은 기존 `CRuntimeAssetRoot::Resolve → CGameInstance::Play_Sound → CSound_Manager`를 사용한다.
Engine 장치·cache·channel과 UI 입력·창 소유권은 이동하지 않았다. 새 singleton, JSON, Loader 등록, 선적재 API는 없다.

- 신규 2파일: `Client/Public/UISoundPlayer.h`, `Client/Private/UISoundPlayer.cpp`.
- 기존 22파일: MainApp.h/.cpp와 PLAN 6절의 외부 20 CPP. 클릭 107곳, 팝업 5곳, 네 창 토글 4곳을 연결했다.
- MainApp의 옛 두 static 선언·정의·호출을 제거했다. 외부 17 CPP는 MainApp.h를 UISoundPlayer.h로 교체했고 세 Level은 기존 다른 MainApp API 때문에 둘 다 유지했다.
- CustomizingView는 기존 간접 의존 `DataJson.h`를 직접 포함한다. PartyInteraction의 별도 수락음은 그대로 유지했다.
- I/P/N/M은 각 창을 Toggle한 뒤 그 창의 Is_Open으로 open/close를 선택한다. 입력 조건·키 edge·상태 저장 순서와 volume 1.f를 유지했다.
- 사용자의 GetForegroundWindow·InputManager·사운드 관리 질문 주석을 보존했다.

프로젝트와 filters의 사용자 등록 상태를 확인했다. H/CPP 각각 한 번, 기존
`02.GameObjects\04. UI\00.LayoutInputAndDrawing` filter를 사용한다.
기존 위치를 재배치하거나 별도 PCH/charset override를 추가하지 않았다. 실제 등록 위치는 PLAN 5절에 기록했다.

## 실제 검증

| 검증 | 확인 결과 |
|---|---|
| 구 helper 전수 검색 | Client/Public·Private의 선언·정의·호출 0건 |
| 새 호출과 음원 선택 | BUTTON_CLICK 107, POPUP_REQUEST 5, 창 선택 4. 네 WAV literal은 UISoundPlayer.cpp의 4줄에만 존재 |
| include·등록 | 호출하는 모든 CPP가 UISoundPlayer.h를 직접 포함. 두 XML parse 성공, 각 H/CPP 중복 0 |
| 변경 검토 | root 4파일 + 외부 20파일의 SHA256이 after manifest와 24/24 일치한 시점에서 독립 재검토. 새 concern 없음 |
| 전체 코드 문서 | 신규 2 + 기존 22 H/CPP의 전문을 문서 갱신 시점 실제 저장본과 24/24 대조. BOM·CRLF/LF·fence에 필요한 최종 개행만 정규화 |
| Debug x64 Product | Engine·Shared·Server·Client 단계 PASS, Client 컴파일·링크·배포 PASS, SkipBuild=false |
| 런타임 데이터 검사 | Product 결과의 missingRuntimeInputs·invalidRuntimeInputs는 빈 배열. 데이터 publish 없음 |
| 실행·청취 | 미실행·미검증 |

실제 실행 명령은 저장소 루트에서 다음과 같다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File 'Tools/Build/Invoke-BuildAndRegression.ps1' -Configuration Debug -Profile Product -MaxCompilerProcesses 4 -BuildLogDirectory 'out/MainAppRefactor/g02-review/build-debug' *> 'out/MainAppRefactor/g02-review/product-debug.log'
```

빌드 시작은 2026-10-14 23:59:04 KST, 완료는 2026-10-15 00:01:07 KST다.
총 기록 시간은 122789ms, Client 단계는 113686ms였다. 기존 PCH를 사용하는 제품 경로로 실행했으며
Client 출력 기록은 OBJ=46, PCH=0, CSO=0, binaries=1이다. 이 수는 산출물 쓰기 수이며 G02 파일 수와 같다는 뜻이 아니다.

- [실행 로그](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g02-review/product-debug.log)
- [Product 결과 JSON](C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261014T150107069Z-debug-product.json)
- [root 변경 manifest](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g02-review/root-manifest.json)
- [외부 20파일 변경 manifest](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g02-review/external-after-manifest.json)
- [외부 변경 diff](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g02-review/external-g02.diff)

빌드 직전 Client·Server·MSBuild·cl·link 실행 프로세스가 없음을 확인했다. VS Insiders의 솔루션은 열려 있었다.
사용자 프로세스를 종료하지 않았고 Client나 Server를 실행하지 않았다.
제품 소스는 사용자·다른 세션의 미커밋 변경을 포함한다. 이번 Product 통과는 실행 당시 작업본에 대한 결과다.
PLAN 전문 갱신 때 보존한 무관한 후속 변경을 G02가 구현한 변경이나 별도 빌드 검증으로 기록하지 않는다.

## 남아 있는 경고와 미검증

MainApp.cpp의 기존 trailing whitespace는 보존했다. 저장소 전체 `git diff --check`가 깨끗하다고 기록하지 않는다.
이번 빌드 로그에는 기존 문자 집합 C4819/C4828과 DirectXTK.pdb 누락 LNK4099가 남아 있다.
ValtanBossTool.cpp:663의 C4244 변환 경고도 남아 있다. 이 경고를 제거하려고 무관한 소스·인코딩·빌드 설정을 바꾸지 않았다.

Release 빌드, 실제 I/P/N/M과 문자 입력·시네마틱 억제 재현, 버튼·초대·레이드 팝업 청취,
첫 재생 지연 측정은 하지 않았다. 소스 전환과 Debug 링크 성공을 실제 음원 재생 확인으로 대신하지 않는다.
사용자는 기존 같은 조건에서 창이 한 번 토글되고 소리가 한 번 나며 팝업 수신과 수락음이 구분되는지 확인한다.

## 다음 단계

[G00 구조 지도](2026-10-14_MAINAPP_G00_ARCHITECTURE_MAP.md)와
[전체 DESIGN](2026-10-14_MAINAPP_RESPONSIBILITY_REFACTOR_DESIGN.md)은 G02의 소스 반영·Debug 통과를 표시한다.
G03은 사용자 구현을 검토·보정했고 Debug Product 빌드·링크와 반환값 검사 9건을 통과했다.
후속 검증은 [G03 RESULT](../10-15/2026-10-15_MAINAPP_G03_SOUND_PREPARATION_RESULT.md)에 구분하며 G04 이후는 제품에 미적용이다. [G03 음원 준비 PLAN](../10-15/2026-10-15_MAINAPP_G03_SOUND_PREPARATION_PLAN.md)은
기존 Sound cache의 명시적 준비 계약을 다루며 이번 G02에서 첫 클릭 적재가 제거됐다고 주장하지 않는다.
