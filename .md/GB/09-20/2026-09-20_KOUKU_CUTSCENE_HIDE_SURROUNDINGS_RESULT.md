# 쿠크 컷신 중 주변 맵 렌더링 격리 RESULT (2026-09-20)

브랜치: `feature/maharaka-island-level` (PR #417이 열려 있음. 이 변경은 커밋하지 않고 작업 폴더에 남겼다)
요청: "쿠크 컷신 진행 중일 때 주변 맵이나 사물들이 렌더링이 안 돼야 하고 컷신이 끝나면 보여야 한다."

## 상태 요약

- **원인은 소스와 데이터로 확정했다.** 전체 맵을 한꺼번에 올리고 카메라 far가 2km 이상인데 컷신 중 다른 구역을 숨기는 장치가 없었다.
- **수정은 소스에 들어갔다.** Client 8개 파일, 구문 검사(Debug·Release)와 군집 계산 하네스 검증까지 했다.
- **빌드는 하지 않았다. 게임에서 실행하지도 않았다.** 화면에서 주변이 실제로 안 보이는지, 컷신이 끝나면 돌아오는지는 사용자만 판정한다.
- **원작에서 "컷신 중 주변 맵을 숨긴다"는 규칙은 찾지 못했다**(3절). 요구사항은 사용자 요청을 기준으로 삼았다.

## 0. 가정과 확정하지 못한 것

- "컷신"은 이 프로젝트가 이미 쓰는 판정 `CLevel_KakulSaydonArena::Is_CinematicPresentationActive()`로 정의했다(새 신호를 만들지 않았다). 카메라 트랙이 없고 `m_bSequenceCombatPending`도 아닌 시퀀스(카메라가 일반 follow 그대로인 트리거 컷신)는 이 판정이 false라서 이번 변경의 대상이 아니다.
- 사용자가 본 화면(스크린샷)이 없어서 "주변 다른 맵"이 정확히 어느 구역인지는 확정하지 못했다. 2관문 입장 컷신 후반은 구역에서 300m 이상 떨어진 빈 공간이라 가장 유력한 후보이고 이 경우를 규칙에 반영했다(5절). 이 추정은 화면으로 확인하지 못했다.
- 무대 구역의 경계는 원본 sub-level 이름이 아니라 배치 위치의 60m 군집으로 정했다(2절 근거). 원본의 실제 streaming 가시성 규칙은 확인하지 못했다.
- 컷신 카메라가 블렌드 아웃으로 follow 카메라에 돌아가는 동안은 판정이 true로 남는다(4절). 그 시간 동안은 주변이 계속 숨겨지고 블렌드가 끝난 뒤에 돌아온다.

## 1. 원인 (소스로 확정)

1. **전체 맵을 로드한다.** 쿠크의 `CLevelRegistry` descriptor는 `MakeFullMapScope()`이고 배치 3,369개가 전부 올라온다.
2. **맵이 서로 떨어진 무대 구역으로 이루어져 있다.** 배치 위치를 60m 연결 거리로 묶으면 17개 구역이 나온다. 큰 구역은 마리오 5곳, 1관문 팝업북 아레나(z 707~767), 2관문 아레나(z 912~972), 카드미로(z 1295~1408), 시작 광장, 빙고판(z 1108~1185)이다.
3. **카메라 far가 크다.** `Ready_Layer_Camera` 계열의 `cameraDesc.fFar = max(2000, span * 8)`라서 최소 2km이고 다른 구역이 시야에 들어오면 그대로 그려진다.
4. **컷신 중 주변을 숨기는 범용 장치가 없었다.**
   - `Set_RuntimeVisible`로 배치를 켜고 끄는 기존 코드는 모두 특정 연출 전용이다. 팝업북 컷신의 펼침 사본↔서 있는 아레나 교체(`KAKUL_ARENA_HIDDEN_PLACEMENT_IDS` 461개), 서커스 피날레 소품, 게이트 오브젝트 표시.
   - `Is_CinematicPresentationActive()`는 UI 입력 차단(`Sync_KoukuCinematicUI`)과 안내 문구 억제에만 쓰였고 맵 렌더링에는 연결돼 있지 않았다.
5. 원본 sub-level 이름은 배치 데이터(`.mapplacements` 셋째 문자열)에 남아 있다: `SL01` 211, `SL02` 1,357, `SL03` 348, `SL04` 429, `SL05` 497, `SCENE01A` 79, `PS` 109, `EDITOR` 339. 구역과 잘 대응하지만 제품 코드는 이 값을 쓰지 않았다.

## 2. 설계와 경계

- **구역**: `Build_CinematicStageAreas()`가 레벨 초기화 때 한 번, 배치 XZ 위치를 60m 단일 연결로 군집화해 구역(AABB)과 `placementId → 구역` 표를 만든다. 추가 저작 데이터가 없다.
- **그릴 구역**: 판정이 true인 동안 매 프레임 `Update_CinematicSurroundings()`가 세 종류의 기준점 중 하나라도 구역 AABB+80m 안에 있으면 그 구역을 유지한다: 카메라 위치, 카메라 전방 40m와 100m 표본점, 로컬 플레이어 위치.
- **숨김 방식**: 기준을 벗어난 구역의 배치를 `CMapPlacementRuntime::Set_RuntimeSuppressed`로 숨긴다. 이것은 논리 표시(`Set_RuntimeVisible`)와 별개인 오버레이 플래그(`CMapAssetObject::m_bStageSuppressed`, static batch는 인스턴스별 `Suppressed`)이며 렌더 판정만 `visible && !suppressed`가 된다. 팝업북 아레나 교체나 시퀀스가 직접 켜고 끄는 값과 충돌하지 않고, 복원은 오버레이를 끄는 것 하나다.
- **숨기지 않는 것**: 시퀀스가 `MAP_PLACEMENT`로 바인딩한 배치(814개, 시퀀스가 직접 다룸), 스케일 100 이상 배경물(5개), 구역 표에 없는 배치(에디터 추가 등).
- **기준점이 어느 구역에도 없으면**: 구역 배치를 전부 숨기고 시퀀스 소유 대상만 그린다(5절).
- **파라미터 근거**: 92개 카메라 트랙의 2,920개 키프레임으로 정했다. 여유 80m와 표본 40m·100m는 무대 구역이 100m 안에 있는 2,741개 키프레임에서 한 번도 무대를 놓치지 않았고, 60m 이하는 놓쳤다. 카메라 위치만 쓰면 팝업북 컷신 처음 2개 키프레임(0~300ms)에서 무대(1관문 아레나)를 놓친다. 그래서 전방 표본을 넣었다.
- **성능**: 컷신이 아닐 때는 판정 호출과 bool 검사뿐이다. 판정이 true일 때도 기준점 4개와 구역 17개 비교뿐이고 힙 할당이 없다(스크래치 벡터 재사용). 숨김 집합이 바뀔 때만 배치를 한 번 훑는다.

## 3. 원작 근거

- 앵콜 컷신 패키지(SCENE07A)의 Kismet에는 `SeqAct_ToggleHidden`이 있다. 입력 링크는 `[0]=Hide`, `[1]=UnHide`이고 배선은 "레벨 로드 시 → Hide, 컷신 시작(`37081_331`) → UnHide, 컷신 끝(Matinee `Completed`) → Hide"이다. `Target` 변수는 `efskeletalmeshactorlookinfomat_0`(스켈레탈 메시 액터, 컷신에 출연하는 3D 보스)를 가리킨다.
- 즉 **원본에서 확인한 숨김은 컷신 출연자를 컷신 동안만 보이게 하는 것이고, 주변 맵을 숨기는 규칙은 아니다.** 다른 컷신 패키지와 TriggerMapData의 level streaming 노드는 이번에 조사하지 않았다.
- 원본 배치 데이터에 sub-level이 나뉘어 있는 것은 UE3의 streaming 구조와 맞지만, 컷신 중 다른 sub-level을 어떻게 다루는지는 확인하지 못했다. **원본 근거 없음**으로 기록한다.

## 4. 컷신 진입점별 숨김·복원 경로 (코드로 따라간 결과)

`Update_CinematicSurroundings()`는 `Update()`에서 `Update_CameraShots()` 바로 뒤에 호출된다(그 시점에 카메라·컷신 상태가 이번 프레임 값이다). 판정이 false가 되면 같은 호출에서 `Restore_CinematicSurroundings()`가 모든 오버레이를 끈다.

- **구역 카메라 shot이 시퀀스에 묶인 컷신**(`sequenceInstanceId`가 있는 shot, 6개): 바인딩된 시퀀스가 끝나거나 STOP되어 `Is_Playing`이 false가 되면 판정이 false다. 서버 STOP은 `stopOwner`/`Consume_OwnedWorldCue` 경로로 시퀀스를 멈춘다. `Release_CameraShot()`도 shot 소유를 끊는다.
- **Composition 카메라 트랙**(92개 shot의 track, 관문 입장·클리어·패턴 컷신, Workbench 미리보기): `m_CompositionCamera.ownerKey`가 비면 false다. 복귀 블렌드가 끝나거나(`transition.ownerKey.clear()`), `Stop_CompositionCamera(true)`(F6 자유 카메라 전환, 카메라 소유권 상실, 샘플 실패)가 호출될 때다. 복귀 블렌드(`blendOutMs`) 동안은 true가 유지된다.
- **`m_bSequenceCombatPending`**: 시퀀스 종료 처리와 `Debug_SetSequenceCombatPending(false)`에서 해제된다.
- **연속 재생**: 판정이 계속 true이면 매 프레임 유지 구역을 다시 계산하고 집합이 바뀔 때만 다시 적용한다.
- **Level 전환·접속 끊김**: 레벨과 맵 런타임이 함께 정리되므로 복원할 대상이 남지 않는다(`~CLevel_KakulSaydonArena`가 `m_MapRuntime.Clear()`). 접속 끊김 복구 요청이 거절돼 레벨이 남으면 그 프레임의 `Update`가 계속 돌아 정상 경로로 복원된다.
- **Debug Map Tool 활성**: 툴이 모든 배치를 편집·미리보기해야 하므로 hook이 복원하고 아무것도 숨기지 않는다.
- **`Update()` 조기 return**: 월드 전환 승인 처리 중에는 hook이 실행되지 않는다. 그 프레임은 레벨이 교체되는 중이다.

## 4b. 바꾼 파일

- `Client/Public/MapAssetObject.h`: `Is_StageSuppressed`/`Set_StageSuppressed`, 렌더 판정 `Is_Rendered()` (+7)
- `Client/Private/MapAssetObject.cpp`: `m_bVisible` 5곳(Late_Update, Render_Group, Render_Shadow, 정적 그림자 리비전 2곳)을 `Is_Rendered()`로 교체 (+5 −5). 논리 표시 `Is_Visible()`는 그대로다.
- `Client/Public/MapStaticBatchObject.h`, `Client/Private/MapStaticBatchObject.cpp`: 인스턴스 `Suppressed`, `Set_InstanceSuppressed`, 업로드·그림자 루프 제외, `Update_Instance`가 플래그를 보존 (+33 −2). 배치 경계는 보수적으로 그대로 둔다.
- `Client/Public/MapPlacementRuntime.h`, `Client/Private/MapPlacementRuntime.cpp`: `Set_RuntimeSuppressed` (+22)
- `Client/Public/Level_KakulSaydonArena.h`, `Client/Private/Level_KakulSaydonArena.cpp`: `Build/Update/Apply/Restore_CinematicSurroundings`와 초기화·Update hook, 진단 로그 `stageAreas`·`stageSuppressed` (+255 −1)
- `.md/GB/gotchas.md`: 재발 방지 항목 1개
- Server, Shared, 데이터, 프로토콜은 바꾸지 않았다. 새 C++ 파일과 vcxproj 변경도 없다.
- 합계 코드 +322 −8. 8개 파일 모두 CRLF, BOM, 비ASCII 바이트를 바이트 단위로 보존했다(패치 전후 대조).

## 5. 검증 (실측)

**군집 계산**: 패치된 파일에서 `Build_CinematicStageAreas()` 본문을 그대로 잘라 임시 폴더의 독립 하네스로 컴파일·실행했다. 배치 3,369개가 모두 구역에 배정되고 17개 구역이 나온다. 개수와 좌표 범위가 파이썬 모델과 일치한다(716, 614, 472, 315, 294, 290, 269, 267, 92, 32 …).

**카메라 트랙 시뮬레이션**(최종 파라미터, 시퀀스 바인딩·거대 배경 면제 반영, 92개 shot, 2,920개 키프레임):

- 카메라가 바라보는 지점 25m 안의 배치가 숨겨지는 키프레임: **0건**
- 무대 중심 30m 안의 배치가 숨겨지는 키프레임: **0건**
- 키프레임당 숨겨지는 배치: 최소 1,910, 중앙값 2,458, 최대 2,550 (전체 3,369)
- 키프레임당 유지되는 구역: 중앙값 1개(무대), 최대 2개
- 팝업북 컷신(`2Stage.book`, 60키프레임): 처음 유지 구역 2개(무대와 이웃), 끝 1개. 이웃 아레나는 카메라가 무대 안으로 내려오면서 숨겨진다.
- **179개 키프레임(6.1%)은 유지 구역이 0개다.** 모두 `kouku.gate2.intro.camera.3~7`이고 카메라가 지하(y −56~−100)의 빈 공간에 있으며 가장 가까운 구역이 310~370m 밖이다. 반경 120m 안에 배치가 0개다. 이 프레임들에서 구역 배치를 전부 숨겨도 위 두 검사에 위반이 없다(시퀀스 소유 대상은 면제).

**컴파일**: 수정한 4개 cpp를 Debug·Release로, 바뀐 헤더의 구조체를 쓰는 8개 cpp(맵 툴 4, 캐릭터 선택, 마하라카 셸, 쿠크 월드 오브젝트, MainApp)를 Debug·Release로 `cl /Zs` 검사했다. 전부 exit 0. 링크는 하지 않았다.

## 6. 자기 검증 체크리스트

- (a) 원인이 코드 경로로 확정 — **소스로 직접 확인**(전체 scope, far 평면, 범용 장치 부재, 판정의 사용처)
- (b) 원작 근거 — **원본에서 직접 확인**: 주변 숨김 규칙 없음, `ToggleHidden`은 출연 액터 표시용. streaming 규칙은 **미확인**
- (c) 컷신 시작 시 숨김이 걸리는 경로가 diff에 들어감 — **소스로 직접 확인**(`Update` hook → `Update_CinematicSurroundings` → `Apply_` → `Set_RuntimeSuppressed` → 객체/배치 오버레이 → 렌더 판정 5곳)
- (d) 컷신 무대·소유 대상은 숨김에서 제외 — **데이터 실측**(위 두 검사 위반 0, 시퀀스 바인딩 814개 면제). 화면 확인은 안 함
- (e) 모든 종료 경로에서 복원 — **소스로 경로별 추적**(4절). 실제 실행은 안 함
- (f) 컷신이 아닐 때 렌더링·성능·서버 게임플레이 불변 — **소스로 확인**: 오버레이 기본값 false, 판정 false면 복원 호출 외에 작업 없음, Server·Shared 변경 0. 성능은 측정하지 않았다
- (g) 문서 일치 — 이 문서와 gotchas 항목
- **구문 검사만 통과 / 빌드 안 함 / 게시할 데이터 없음 / 게임 실행 안 함**

## 7. 하지 않은 것과 알려진 한계

- **화면 확인 못 함.** 주변이 안 보이는지, 컷신 뒤 복원되는지, 컷신 무대가 잘 보이는지는 사용자 판정이다.
- **카메라 트랙이 없는 시퀀스 컷신은 대상이 아니다**(판정 false). 사용자가 말한 "컷신"이 이 판정 밖에 있으면 여전히 주변이 보인다.
- **복귀 블렌드 동안 숨김이 유지된다.** 컷신 내용이 끝나도 카메라가 follow로 돌아올 때까지 주변이 안 보이다가 끝난 뒤 한 번에 나타난다.
- **Deploy 소품 7개는 숨기지 않는다**(광장 종이무대·레버 4개, 사이던·책 컷신 사본 2개, 망원경 1개). NPC·보스·몬스터·다른 플레이어도 대상이 아니다.
- **구역이 60m 이내로 붙어 있으면 한 구역으로 합쳐진다.** 새 무대를 만들 때 다른 무대와 떨어뜨려야 한다.
- **구역이 아닌 곳으로 카메라가 들어오면 구역 배치가 전부 사라진다**(2관문 입장 후반). 그 무대의 배경이 배치가 아니라 Deploy·오브젝트 리소스로만 이루어져 있다는 것을 데이터로 확인했지만(반경 120m 안 배치 0개) 화면으로는 확인하지 못했다.
- **카메라 앞 100m 이후에 있는 무대를 향해 날아갈 때** 그 구역이 100m 안에 들어오는 순간 나타난다(미리보기 거리 한계).
- **Debug 미리보기(F1 Action Workbench)의 Composition 카메라 컷신에도 같은 규칙이 적용된다.** Map Tool이 켜져 있으면 적용하지 않는다.

## 8. 반영에 필요한 것

- 게시할 데이터는 없다. **Client 재빌드가 필요하다**(Engine 공개 헤더 변경 없음, Engine 빌드 불필요). Server 재시작은 필요 없다.
- 확인 방법: 쿠크에서 컷신을 재생하고 클라이언트 세션 로그(실행 파일 옆 process별 JSONL)의 `kouku.cinematic.transition` 이벤트를 본다. `stageAreas=17`이고 컷신 중 `stageSuppressed`가 0보다 크면 격리가 걸린 것이다. 컷신인데 0이면 그 컷신은 판정 밖이다.

## 9. 사용자가 결정할 것

1. 사용자가 본 "주변 다른 맵"이 어느 컷신인지 알려 주면 그 컷신의 판정 여부와 `stageSuppressed` 값을 먼저 확인한다.
2. 복귀 블렌드 동안도 숨길지(지금), 컷신 내용이 끝나는 즉시 복원할지.
3. Deploy 소품(7개)까지 숨길지.

## 10. 2차 수정: 컷신 중 오른쪽 위에 남던 무대 조각 (2026-09-20 낮)

사용자 관찰(스크린샷 12:08:06): 1관문 팝업북 컷신에서 주변 맵은 사라졌지만 검은 배경 오른쪽 위(약 x 950~1050, y 70~150)에 기둥·발판이 있는 작은 무대 조각이 남아 있었다.

### 10.1 원인 (증거)

- **로그(클라이언트 세션 로그 `client-session-18204.jsonl`, `kouku.cinematic.transition` 12:08:01.074)**: `cinematic=1`, `scene=scene.kakulsaydon.g1.popup.v1`, `cameraShot=kouku.gate1.authored.book`, `cameraOwner=bundle-preview:KAKULSAYDON_G1_PATTERN_4:...presentation.29`, `sequence=`(비어 있음), `stageAreas=17`, `stageSuppressed=2402`. 즉 WorldSequence가 아니라 F1 Action Workbench의 1관문 패턴 4 미리보기였고, 이후 12:08:06까지 새 전환 이벤트가 없어 스크린샷도 같은 상태다.
- **로그 숫자를 코드 규칙으로 재현**: 전체 3,369개 중 면제 819개(모든 시퀀스 바인딩 814 + 배경물 5)라서 비면제는 2,550개이고 `그려지는 비면제 = 2,550 - stageSuppressed`다. 2,402이면 148개 = 구역 #1(팝업북 아레나 SL04)의 56개 + **구역 #8(SL03, x -7~45, z 292~335)의 92개**이며 20개 미만 구역 조합 중 이 조합뿐이다. 세션의 다른 억제 값 9종 중 8종도 구역 조합으로 정확히 분해된다(예: 2,078 = 구역 #2만, 2,213 = #6 + #8, 2,458 = #8만, 1,930 = #1 + #2 + #8).
- **구역 #8의 정체**: 2관문 보스 배치(`boss.kakulsaydon.g2.big-saydon`, `g2.kouku`)와 플레이어 스폰 `stage.kakul.sl03`(2, 3.5, 330)이 있는 SL03 무대. 팝업북 아레나(z 707~767)에서 400m 이상 떨어져 있다.
- **왜 유지됐나**: 이 shot의 카메라 기준점(카메라 위치, 전방 40m, 100m)은 50ms 간격 보간 799프레임 모두에서 구역 #1의 유지 범위에는 들어가고 구역 #8의 유지 범위(AABB+80m)에는 **0프레임**이다. 코드의 유지 기준점은 넷뿐(카메라, 표본 2개, 로컬 플레이어)이므로 #8을 유지시킨 것은 로컬 플레이어 위치다(플레이어 좌표는 로그에 없어 코드 경로에서 추론한 것이다. 플레이어가 2관문 무대에 서 있었다는 사용자의 실제 위치는 확인하지 못했다).
- **조각과의 일치**: shot의 처음 키프레임(0~1.5초)에서 구역 #8의 91개 배치를 화면(1283x713)에 투영하면 820ms에서 핵심부가 x 946~977, y 94~125, 1,460ms에서 x 1082~1122, y 71~103이다. 스크린샷 조각(x 950~1050, y 70~150)과 위치·크기가 겹친다. 광장 Deploy 소품(종이무대 다리, 레버)도 같은 방향에 겹쳐 보이지만 766~784m 거리라 10~15px 이하여서 100x80px 조각이 될 수 없다.
- **확정도**: 원인 경로(플레이어 기준점이 다른 무대를 유지)는 로그 분해와 기준점 계산으로 **강하게 뒷받침**되지만, 스크린샷 자체에서 그 물체를 직접 식별한 것은 아니다(투영 위치·크기 일치까지).

### 10.2 이전 수정이 놓친 이유

- 이전 수정은 유지 기준점에 로컬 플레이어 위치를 넣었고, 검증 하네스는 "플레이어가 컷신 무대 중심에 있다"고 가정했다(그 가정에서는 누수 0). 실제로는 미리보기·게이트 이동 중처럼 플레이어가 다른 무대에 서 있을 수 있어서 그 무대 전체가 먼 조각으로 남았다.
- 부수적으로 시퀀스 면제가 문서의 **모든 시퀀스(326개)** 의 `MAP_PLACEMENT` 바인딩(814개) 전부였다. 재생 중이 아닌 시퀀스의 배치까지 면제해서 유지 구역 밖의 다른 시퀀스 소품이 남았다(구역 #8은 면제가 0이라 이번 증상의 원인은 아니다).

### 10.3 변경 (Client만, 4개 파일)

- `Client/Private/Level_KakulSaydonArena.cpp`: 유지 기준점에서 로컬 플레이어 제거(카메라, 전방 40m, 100m만), 면제를 `m_SequencePlayer.Collect_OwnedPlacements()`가 돌려주는 "재생 중이거나 자세를 유지 중인 시퀀스가 소유한 배치"로 축소, 소유 집합이 바뀌면(시퀀스 시작·유지·종료) 다시 적용하도록 서명 비교 추가, 진단 로그에 `stageKept=x,z|x,z`(유지 구역 중심) 추가.
- `Client/Public/Level_KakulSaydonArena.h`: 서명 멤버 `m_iCinematicOwnedSignature`.
- `Client/Private/WorldSequencePlayer.cpp`, `Client/Public/WorldSequencePlayer.h`: `Collect_OwnedPlacements(std::unordered_set<uint64_t>*) const`(활성·유지 인스턴스의 `placementBaselines`를 모으고 순서 무관 서명을 반환, 포인터가 null이면 할당 없음).
- Server, Shared, 데이터, 프로토콜, Engine 공개 헤더, 새 C++ 파일, vcxproj는 바꾸지 않았다. 4개 파일 모두 CRLF, BOM, 비ASCII 바이트를 패치 전후 바이트 단위로 대조했다.

### 10.4 수치 검증 (수정 전후)

- **무대 손실**: 카메라 트랙 92개의 키프레임 2,741개(자기 무대가 100m 안인 것)에서 플레이어 기준점을 뺀 규칙의 무대 손실 0건, 정적 shot 22개도 전부 카메라 기준점만으로 자기 무대를 유지한다.
- **플레이어가 다른 무대에 서 있을 때(재현 모델)**: 기준점에 플레이어를 넣으면 2,741개 키프레임 중 517개(19%)에서 그 무대 배치가 화면에 남고(프레임당 중앙값 34, 최대 92), 뺀 규칙은 0개다.
- **카메라가 유지한 구역 밖에서 화면 안에 그려지는 배치 수**: 팝업북 shot(64 키프레임)은 수정 전 25개 키프레임에서 남고 최대 116개, 수정 후 0개. 92개 트랙 전체(2,920 키프레임)는 수정 전 1,128개 키프레임에서 남고 최대 911개, 수정 후 96개 키프레임에서 최대 5개다. 남는 5개는 스케일 100 이상 배경물이며 의도한 면제다. 수정 후 수치는 재생 중인 시퀀스가 없는 미리보기 기준이고, 재생 중인 시퀀스가 소유한 배치는 정의상 그려진다.
- 스크립트: `$CLAUDE_JOB_DIR/tmp`의 `kouku_remaining_model.py`(로그 숫자 재현), `kouku_anchor_check.py`, `kouku_project_area8.py`, `kouku_leak_eval2.py`, `kouku_static_shots.py`.

### 10.5 종료 경로별 복원

이전 절(4절)의 경로는 그대로다. 새로 생긴 상태는 서명 하나(`m_iCinematicOwnedSignature`)이고 `Build_CinematicStageAreas()`와 `Restore_CinematicSurroundings()`가 0으로 되돌린다. 숨김의 적용·복원은 여전히 오버레이 플래그 하나이고 컷신 판정이 false면 복원 호출 외에 하는 일이 없다.

### 10.6 검증 티어와 남은 한계

- **구문 검사**: 바뀐 헤더를 (간접 포함해서) 쓰는 35개 cpp를 Debug·Release로 `cl /Zs`, 전부 exit 0이고 오류 0. 오타를 넣은 사본은 `C2039`로 실패해 검사가 동작함을 확인했다.
- **하지 않음**: 빌드, 게임 실행, 화면 확인. 조각이 사라지는지, 책이 온전한지는 사용자만 판정한다.
- **이 규칙이 다루지 않는 것**: 2관문 보스·몬스터·NPC·다른 플레이어 같은 엔티티와 Deploy 소품 7개. 스크린샷의 조각 위 작은 물체는 구역 #8에 있는 2관문 보스 엔티티(z 약 318)일 가능성이 있고, 그 경우 조각이 사라져도 그 물체가 남을 수 있다(미확인).
- 지금 상태에서 로그의 `stageKept`가 팝업북 컷신 중 팝업북 아레나 중심(약 0,737) 하나뿐이면 플레이어 기준점 제거가 적용된 것이다.
- 반영: Client 재빌드만 필요하다(Engine 공개 헤더 변경 없음, Server 재시작 불필요).
