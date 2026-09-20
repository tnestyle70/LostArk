# 2026-09-20 origin/main 병합 결과

> 범위: `origin/main`(`7947f8fe`)을 작업 브랜치에 병합해 두는 데까지다. **push와 PR 생성은 하지 않았다.**
> 빌드, publisher, 계약 테스트, Client·Server 실행은 하지 않았다(사용자가 VS·Client·Server를 켜 두고 시험 중이었다).

## 1. 브랜치와 백업

| 항목 | 값 |
|---|---|
| 작업 브랜치 | `feature/bern-travel-kouku-encore-0920` (`42ce3c1d`에서 분기, 옛 `feature/maharaka-island-level`은 PR #417로 이미 main에 병합됨) |
| 병합 커밋 | `4bd3366b` (부모: 우리 `50914671`, main `7947f8fe`) |
| origin/main 대비 | 앞선 커밋 5개, 뒤처진 커밋 0개 |
| 병합 전 브랜치 백업 | `backup/pre-main-merge-20260920` = `42ce3c1d` |
| 병합 전 파일 백업 | `out/PrePullBackup20260920/` (파일 992개 92.7 MiB, SHA-256 대조, `MANIFEST.tsv`와 `tracked_changes_vs_HEAD.patch` 포함) |

## 2. 로컬 커밋

| 해시 | 내용 |
|---|---|
| `b83d84bc` | feat(bern): castle/library 왕복 이동 트리거 암전과 도착 직후 재발동 방지, Bern2 네비 영역 |
| `d750014b` | feat(kouku): 컷신 중 주변 맵 숨김과 마리오4 진행선 방향 수정 |
| `57425003` | feat(kouku): 앵콜 컷신 Boss/Sequence 항목과 세이튼 배우 시퀀스, 카메라 크기 보정 |
| `50914671` | docs: 09-20 결과 문서와 gotchas |
| `4bd3366b` | Merge origin/main |

### 커밋에서 제외한 것

- `.claude/`, `.codex/`, `Client/Default/Client.vcxproj.user.bak-20260913`
- 저장소 루트의 `Copy_ResourceDistribution*.ps1`, `Resource_Distribution*.txt`(기존에도 미추적으로 둔 관례)
- `out/`, 빌드 산출물, `Client/Bin/Resources`
- 무관한 잡파일: `temp_effect_action_out/`(903개), `dev/null/`(4개), `glaivier_skill_phase_1_3_reextract.*`, `mario_meshes.json`, `mesh_cook_list.txt`, `Data/Navigation/LV_LUT_MIDNIGHTC_ED.navpaint.before-prune`
- `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.{deployassets,deployplacements,maplights.json,mapmaterials.json}` 4개: 내용이 HEAD와 바이트까지 같고 git이 줄끝 변환 때문에 수정으로 표시하던 것이라 `git checkout --`으로 정리했다.

## 3. 병합 결과와 충돌 해결

main에서 들어온 변경은 391개 파일이고, 양쪽이 모두 바꾼 파일은 12개였다. 충돌은 7개였고 나머지 5개(`MainApp.cpp`, `WorldSequencePlayer.cpp/.h`, `Level_KakulSaydonArena.h`, `PacketMessages.h`)는 자동 병합됐다.

| 파일 | 해결 방법 |
|---|---|
| `.md/GB/gotchas.md` | main 쪽 줄끝이 `CR CR LF`(2634줄)라 main 원본 바이트를 그대로 두고, 3-way 병합으로 확인한 우리 6항목을 main 추가분 뒤에 같은 줄끝으로 붙였다. 양쪽 줄 손실 0. |
| `Client/Private/Level_KakulSaydonArena.cpp` | 우리 `Build_CinematicStageAreas();` 호출 유지 + main이 바꾼 새 주석 채택(main 코드는 시퀀스 로드 실패 시 `E_FAIL`을 반환해 옛 주석이 맞지 않음). BOM과 줄끝 보존. |
| `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | **ID 충돌 2건**을 해소했다. 아래 4절 참조. |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` 및 `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json` | 한 줄짜리 압축 JSON이라 항목 ID 기준으로 병합했다. main 기준에 우리 앵콜 배우 항목 3개(objectResource, template, instance)를 얹었다(revision 2142). 두 파일은 바이트 동일하다(세 버전 모두에서 Data 원본과 Client 복사본이 같았음을 확인). |
| `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`, `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json` | Composition에서 생성되는 출력물이라 손으로 합치지 않고 **main 버전을 임시로 뒀다**. 5절 참조. |

## 4. ID 충돌과 번호 변경 (중요)

두 브랜치가 같은 기준에서 갈라져 같은 다음 번호를 썼다.

| 대상 | main | 우리(옛 번호) | 병합 결과 |
|---|---|---|---|
| 패턴 `KAKULSAYDON_G1_PATTERN_94` | 메두사공포 | 앵콜컷신 | main 유지, **우리는 `PATTERN_96`으로 변경** |
| 패턴 `PATTERN_95` | 즉사 칼날 / Object 판정 테스트 | 없음 | main 유지 |
| 월드 `kakulsaydon.g1.world.40` | 카드미로 연출 쿠크 뿅망치 | 앵콜컷신 / Saydon | main 유지, **우리는 `world.41`로 변경** |

- 카운터: `nextPatternOrdinal` 97, `nextWorldOrdinal` 42, `revision` 1913.
- 우리 쪽 옛 ID를 참조하던 곳은 Composition 안뿐이었고(생성 출력물 2개 포함 3개 파일), 새 번호로 함께 바꿨다.
- 이 세션의 이전 문서(`2026-09-20_KOUKU_ENCORE_*`, `2026-09-20_ENCORE_CAMERA_FIT_RESULT.md`)에 적힌 앵콜 `PATTERN_94`는 이제 **`PATTERN_96`** 이다. 옛 문서는 고치지 않았다.

## 5. 아직 남은 것: 생성 출력물 재생성 (필수)

병합된 Composition(revision 1913) 기준으로 아래를 다시 만들어야 한다. 지금은 main 버전이라 `sourceRevision`이 1912이고 **우리 앵콜 패턴(PATTERN_96)이 들어 있지 않다**.

- `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`
- `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json`

**재생성 후 확인할 위험:** main의 Encounter에는 `raidGates`가 GATE2, GATE3뿐이다(공통 조상과 우리 쪽은 GATE1~3). Sequence 원본(`KoukuSaydonSequenceComposition.json`)은 main이 바꾸지 않았는데도 그렇다. 재생성 뒤에 GATE1이 살아 있는지, 앵콜 Sequence와 함께 확인해야 한다.

## 6. 실제로 검증한 것

| 확인 | 결과 |
|---|---|
| 충돌 마커 | HEAD 트리에서 `<<<<<<<`/`>>>>>>>` 0개 (`=======` 단독 1줄은 아래 참조) |
| 병합으로 바뀐 JSON 139개 파싱 | 실패 0개 |
| 집합 검증 | 병합 결과가 main과 다른 파일 46개는 모두 우리가 바꾼 파일, 우리 HEAD와 다른 파일 391개는 모두 main이 바꾼 파일 |
| 자동 병합 5개 + 충돌 C++ 1개 | 양쪽 변경 줄이 하나도 부족하지 않음(줄 개수 비교) |
| 양쪽 기능 존재 확인 14항목 | 실패 0개 (베른 상수·트리거 이벤트 4개·컷신 숨김 심볼·앵콜 카메라 shot 키 26개·Composition의 PATTERN_96/94/95와 world.41/40·worldsequences 앵콜/뿅망치/해골·main #418/#419/#420 심볼) |
| C++ 구문 검사(`cl /Zs`, 산출물 없음) | Client 7개(충돌·변경 소스) + Server 90개 전부 `exit=0`. 병합된 `Engine/Public`을 `EngineSDK\inc`보다 앞에 뒀다(main이 `Engine_RenderTypes.h`를 바꿨고 SDK 복사본은 오래됨). |
| 프로젝트 등록 | main이 추가한 소스 13개(셰이더 11개, `UITextOcclusion.cpp/.h`) 모두 `Client.vcxproj`에 등록됨. 병합 결과 vcxproj/filters는 main과 동일(우리는 건드리지 않음). |
| 프로토콜 | `NETWORK_PROTOCOL_VERSION` 공통 조상·main·병합 결과 모두 94 |

## 7. 하지 못한 것과 남은 위험

- **빌드, 게시, 계약 테스트, 화면 확인은 하지 않았다.** 구문 검사는 링크와 실행을 대신하지 않는다. Client는 `_DEBUG`만 검사했다.
- main이 Engine 공개 헤더(`Engine_RenderTypes.h`)와 Client·Server 소스를 많이 바꿨으므로 **Debug 제품 빌드는 오래 걸릴 수 있다**(Engine 재컴파일).
- **작업 폴더의 데이터가 새 코드에 맞춰 병합됐지만 실행 파일은 병합 전 빌드다.** 재빌드 전에 지금의 Client/Server를 다시 켜면 main의 데이터(예: 스킬 MP 비용)와 어긋날 수 있다.
- main의 `.md/GB/gotchas.md`에는 병합 전부터 `=======` 단독 줄이 하나 있다(공통 조상에는 없음, main 커밋의 잔재로 보임). 우리 변경이 아니라 그대로 뒀다. `git diff --check`가 이 파일의 `CR CR LF` 줄끝을 후행 공백으로 표시하는 것도 main 원본 때문이다.
- 이 병합 도중 충돌 해결 스크립트가 실패했는데 뒤이은 `git add`가 그대로 실행돼 `Level_KakulSaydonArena.cpp`가 마커가 남은 채 스테이징된 적이 있다. 바로 발견해 줄끝(CRLF) 처리를 고친 스크립트로 다시 해결·재스테이징했고, 병합 커밋 전에 스테이징 내용에서 마커 0개를 확인했다.

## 8. 사용자가 VS·Client·Server를 끈 뒤 할 일 (순서)

1. 생성 출력물 재생성과 게시: `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`(Composition 프로젝션 포함), 필요 시 `-Owner Server`, `-Owner Client`. main이 Data(밸런스 등)를 125개 파일 바꿨으므로 Server 쪽 게시도 필요하다.
2. `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug` (같은 설치의 VS와 겹치지 않게).
3. `Server\Bin\Debug\Server.exe --contract-test` 결과를 보고, 실패가 이번 병합 이전에도 있었는지 구분한다(앞선 실행에서 쿠크 마리오·패턴 체인·관문 보스 배치 실패가 있었고 원인은 미확정).
4. 화면 확인은 사용자: 베른 castle/library 왕복, 앵콜 컷신(Boss 탭과 Sequence 탭에서 `PATTERN_96`으로 보이는지), 쿠크 컷신 주변 숨김, 마리오4 방향.
5. push 전 확인: 앵콜 배우 리소스 18개(53 MB)는 Git 밖이라 Drive로 전달해야 한다.
