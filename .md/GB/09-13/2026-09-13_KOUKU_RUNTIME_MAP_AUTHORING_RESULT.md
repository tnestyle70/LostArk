# 쿠크 런타임 F1 맵 편집 연결 결과

## 구현 상태

2026-09-13 사용자 요청에 따라 실제 저장소 `C:/Users/USER/source/졸업팀폴/LostArk`에 반영했다. 브랜치는 `codex/kouku-g1-fade-visible`이며 기존 dirty 컷신·효과·솔루션 변경은 보존했다. 자동 commit/push는 하지 않았다.

- Debug Lobby → KoukuSaydon → F1 → Map Tool에서 현재 Area를 기존 편집기에 연결한다. Test의 기존 Area 선택과 소유 컨테이너는 유지한다.
- 쿠크 arena 소유 map placement/batch/Deploy를 차용한다. 초기 연결에서 맵을 중복 생성하지 않고 source/runtime 전체 ID·asset 집합을 검증한다. 불일치 시 상태 메시지와 Retry를 제공한다.
- Inspector와 Save는 원본 placement draft를 사용한다. 현재 애니메이션의 sampled transform을 저작값으로 저장하지 않는다.
- World/Composition 재생 중 배치 변경·재로드를 거절한다. 현재 Area는 고정하며 Server 권위 collision/navigation/전투는 변경하지 않는다.
- 자체 맵 움직임은 편집 입력 소유 중 잠시 정지한다. stable ID로 base/index를 갱신하며, 삭제한 motion 정의는 범위 밖 index로 비활성화해 Reload로 ID가 돌아왔을 때 복구한다.
- Reload에서 사용자 미저장 Visible 편집은 source로 되돌리고, authored Visible과 다른 runtime 표시 override만 보존한다. Deploy state/surface presentation은 staged 객체에 보존한 후 commit한다. 보존 실패 시 기존 runtime을 유지한다.
- C++ 신규 파일·프로젝트 등록·리소스·JSON 변경은 없다. 기존 15개 C++ 파일, 기존 계약 검사와 팀 사용서·계획/결과를 갱신했다.

## 자동 검증

| 검사 | 결과 |
|---|---|
| 기존 Debug Client 증분 Build, 기존 Engine/Shared 입력 재사용 | 성공, 47.35초, 오류 0 |
| 마지막 재로드 보완 2개 CPP 증분 Build/링크 | 성공, 13.07초, 오류 0, 기존 포함 경고 270 |
| `python -X utf8 -m unittest Tools.MapPipeline.test_world_sequence_authoring_contract` | 35개 성공, 27.888초 |
| `git diff --check` | 공백 오류 없음; Git LF/CRLF 안내만 있음 |
| Client vcxproj/filters XML parse | 성공 |
| 독립 코드 검토 | self-motion 삭제/복구 및 미저장 Visible 재로드 문제를 재현·수정하고 회귀 source guard 추가 |

초기 검사 실행은 이전 멤버명을 찾는 source assertion 3건과 cp949 subprocess 출력 디코딩 문제로 실패했다. assertion을 실제 새 접근 함수로 갱신하고 UTF-8 모드로 다시 실행한 위 35건이 최종 결과다. source guard는 실행 중 렌더링 검증을 대신하지 않는다.

Clean/Rebuild, 셰이더 재컴파일, Engine/Server 재빌드, publisher 실행은 하지 않았다. 빌드의 기존 compiled shader 복사 단계만 실행됐다. Engine/Shared public 헤더나 Data 계약 변경이 없어 Client 범위로 검증했다. Release 제품에는 개발자 도구를 공개하지 않으며 이번 Release 빌드는 수행하지 않았다.

빌드 로그는 `out/RuntimeMapTool20260913-build.log`, `out/RuntimeMapTool20260913-final-build.log`다. 산출물은 `Client/Bin/Debug/Client.exe`다. C++ 파일은 기존 UTF-8 및 BOM 여부를 유지했다.

## 수동 확인 / 남은 경계

Client와 UI는 에이전트가 실행·조작·캡처하지 않았다. 최종 화면과 저장/재로드 체감 동작은 사용자 확인 대기다. 빌드와 source tests만으로 visual PASS를 기록하지 않는다.

1. 시작 위치가 `Client/Default`인 새 Debug Client를 실행한다. Lobby → KoukuSaydon → F1 → Map Tool을 연다.
2. 현재 쿠크 Area 자동 연결과 기존 맵 위 선택·Position/Rotation/Scale/Visible 편집을 확인한다. 런타임에서 다른 Area 선택은 잠근다.
3. World/Composition 재생 중에는 Stop/Restore 안내를 확인하고, 정지 후 편집한다. 미저장 Visible 변경 후 Reload가 원본으로 돌아오는지 확인한다.
4. 배치 하나 Save → Reload, 움직이는 배치 삭제 → Reload, MapTool 닫기/재열기 및 Lobby 왕복을 확인한다. 실제 source 삭제를 보존할 의도가 없다면 삭제 상태에서 Save하지 않는다.
5. Test의 기존 Area 선택·편집도 비교한다. Save는 Data authoring만 저장하므로 별도 runtime DataFiles/Server 반영에는 기존 publish 절차를 사용한다.

이번 작업은 컷신 새 제작·포탈·암전·전등·재질 복구 작업을 포함하지 않는다.


## G02. World Sequence 공통 재생·원상복구 수정 결과

### 사용자 재현과 원인

G01 화면 확인에서 사용자는 World Sequence 전체 재생 실패를 보고했다. 제공된 화면은 `Stage1_wall → Map #3`, Time 32ms, `Preview restore failed; targets remain owned for retry`였다. G01의 자동 검사 성공은 이 실제 재생 성공을 뜻하지 않았다.

공통 실패 원인은 런타임 소유 맵을 차용하면서 Load_Source가 붙인 `MapEditorArea:<AreaId>:` 모델 태그를 그대로 소비한 것이다. 실제 arena가 등록한 태그는 달라 batch transform용 Clone_Prototype이 실패했다. Apply와 Restore가 같은 조회를 사용하므로 복구도 실패하고 기존 target ownership이 남아 다음 항목 재생을 막았다. 개별 Stage1_wall 키프레임 문제가 아니다.

### 반영한 코드

- source catalog의 stable asset ID·resolved model path와 runtime catalog를 전체 검증한 뒤 prototype tag만 runtime 값으로 연결한다. source 경로와 metadata는 유지하며 Test namespace는 바꾸지 않는다.
- 수동 Map preview는 선택한 track의 Visible key를 적용하고 Stop에서 캡처한 기존 runtimeVisible을 복원한다.
- Deploy preview의 Sample에 기본 false인 revealHidden을 추가했다. World Sequence panel에서 Start Delay 이후만 opt-in하고, 지연 이전으로 스크럽하거나 End할 때 해제한다. 실제 Deploy state와 원본 clip capture는 바꾸지 않는다. 기존 다른 caller는 기본 false를 유지한다.
- CardMiro_Play는 runtime attach에서 arena의 WorldSequenceObject prototype을 재등록하지 않는다.
- Restore 실패 메시지에 Map/Deploy ID와 모델 조회/transform 실패 이유를 남긴다. 오류를 숨기기 위한 강제 ownership 초기화는 하지 않는다.

실제 변경은 기존 C++ 7개와 기존 source 검사 파일이다. 새 C++/프로젝트 등록, Engine/Server 소스 변경은 없다. 이번 G02에서 사용자 배치·애니메이션·Composition·카메라·Resources·runtime 생성 문서는 수정하지 않았다. 다른 작업에서 이미 변경된 Data와 Bin/DataFiles도 유지했다.

### 검증

- Debug/x64 Client 정상 증분 Build 성공: **1분 16.39초, 오류 0**. 기존 헤더 포함 경고 4,787건이 출력됐으며 관련 없는 인코딩/경고 일괄 정리는 하지 않았다.
- `python -X utf8 -m unittest Tools.MapPipeline.test_world_sequence_authoring_contract`: **39개 성공, 43.080초**.
- `git diff --check`: 성공.
- 파일 설치 전 현재 원본 hash 대조, UTF-8 유효성과 기존 BOM 유지 확인. source mismatch가 있으면 설치를 거절하도록 했다.
- 독립 검토에서 Deploy가 Start Delay 전에 표시되는 문제를 찾아 재현 후 Begin 즉시 표시 대신 Sample별 delayedMs 기준으로 수정했다. 기존 테스트에 opt-in/해제/지연 경로 검사를 반영했다.
- Clean/Rebuild나 Engine/Server 재빌드, HLSL 변경, publisher 실행은 하지 않았다. 기존 Client build의 SDK shader 배포 복사만 수행했다.

빌드 로그: `C:/Users/USER/source/졸업팀폴/LostArk/out/RuntimeSequencePreview20260913-build.log`.
실행 파일: `C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Debug/Client.exe`.

### 사용자 실행 확인 대기

에이전트는 Client/UI를 실행하거나 캡처하지 않았다. 실제 화면 재생 성공은 아직 판정하지 않는다.

VS의 기존 **Debug/x64 F5** 경로로 새 Client를 실행한다. Lobby → KoukuSaydon → F1 → Map Tool → World Sequence에서 기존 Placed Instances를 선택하고 아래 Preview의 Play, Pause, Time 왕복, Stop/Restore를 확인한다. Stage1_wall 외에도 다른 Map 배치와 Paper stage bridge 등 Deploy를 확인한다. 상태줄이 다시 실패하면 이제 구체적인 target ID와 단계가 함께 표시된다.

Stop 이후 맵/소품의 이전 표시 상태가 돌아오는지, 다른 인스턴스로 선택·재생이 가능한지 확인한다. 애니메이션 데이터 재작성이나 Runtime 설치기 재실행은 필요하지 않다. 일반 아래쪽 Preview와 여러 시퀀스를 묶어 실행하는 전용 Cutscene 버튼은 동일 UI 기능이 아니며, 이번 변경은 그 차이를 숨기거나 새 통합 플레이어를 만드는 작업이 아니다.
