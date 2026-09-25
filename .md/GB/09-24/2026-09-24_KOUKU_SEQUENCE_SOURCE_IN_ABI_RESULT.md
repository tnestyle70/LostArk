# 쿠크 Sequence Source In 준비 실패 조사와 OBJ 복구 결과

## G00. 첨부 오류의 본질

오류는 entry Collider의 별도 API나 잘못된 Effect trim 데이터가 아니다. Debug Client의
이전 C++ 복사 구현이 새 optional 필드를 초기화하지 않은 빌드 산출물 혼합이다.

PID36716·49220의18:49:18와19:01:10 로그는 Action2254, Sequence171, composition ID와
Server gameplay pin이 모두 일치한다. MainApp의 sequence.expand 단계가 첫 패턴에서
실패하고 FAILED acknowledgement를 보내므로 Server가 준비를 거절한 것이다.

Sequence171의 실제10개 패턴에는 effectSourceStartMs가 한 개도 없으며 기본값0이
정상이다. 현재 CompositionDocument OBJ를 연결한 native probe는 동일 파일의10개
패턴 모두 Reload와 Try_ExpandPatternDocument를 통과했다.

Debug Animation_Tool.obj는07:29:29 산출물이었다. 변경 header는15:37:06인데 해당
CL.read 기록에는 Animation_Tool.cpp와 Client.pch만 있다. 다른 최신 TU의 occurrence
복사 생성자 hash는 dc30c1919126994e인 반면 이 OBJ만 d87dd105226a623b다. 대입 연산자도
이전 구현이었다. Source In은 이전 alignment padding인offset0x144에 들어갔으므로
구조체 크기는 유지되지만 이전 memberwise copy는0x140 다음0x148만 복사한다. 새 필드는
미초기화 값으로 남아 정상적인 수명 검사를 실패시킨다. 설치 Debug EXE에도 이전 복사자
machine code가 남아 있는 것을 relocation-aware byte 비교로 확인했다.

Complete Play 요청과 entry Collider는 기존 Begin_KoukuRaidPreparation에서 같은 준비
절차로 합쳐진다. API를 복제하거나 validation을 풀 이유가 없다.

## G01. 수행한 복구

정확한 이전 Animation_Tool.obj와 read/write/compiler/link tracking을
out/KoukuSequenceLifetime20260924/previous-debug-objects에 보존했다. 원본과 보존본
SHA256이69E1733546360BB22C3D18AED226CCAB91E80A410E50CE33555E4C5929874C32로
일치함을 확인한 뒤 workspace 내부 절대 경로를 검증하고 해당 OBJ1개만 격리했다.

소스·PCH timestamp와 tracking 파일을 지우거나 전체 Clean/Rebuild하지 않았다.
제품 C++/JSON/schema와 Source In validation은 바꾸지 않았다. 이 작업의 Git 변경은
PLAN/RESULT뿐이며 신규 project/filter 등록은 없다. root가 정상 VS18/v143 Debug
Client ClCompile을 실행해exit0으로 완료했다. 실행 중 Client 파일은 링크·교체하지 않았다.

## G02. 자동 검증과 남은 경계

- 현재 Document OBJ와 기존 native harness를 연결한 focused 실제10 Sequence Load/Expand: PASS.
- Sequence171의 Source In 부재와 Action2254의 P47 두 source-in4201 < resource10637: PASS.
- 구형/최신 COMDAT 복사 생성자·대입 연산자 binary 비교: 이전1개만 서로 다른 구현 확인.
- 전체 --kouku-sequence-document-contract: 기존 'three combat entries' count 단언 실패.
  현재 Sequence는 Encore를 포함해4개 entry다. 이번 조사에서 그 기존 단언을 완화하거나
  전체 suite 통과로 기록하지 않았다.
- Debug Client ClCompile: PASS, exit0. out/KoukuMadnessRaid20260924-client-debug-compile.log.
- Animation_Tool 복사자 재검증: PASS. 신규hash dc30c1919126994e가 최신 Document OBJ와
  같고 이전hash와 다르다. CL.read가 Composition header를 추적하며 OBJ가 header보다 최신이다.
  증거는out/KoukuSequenceLifetime20260924/abi-after-build.json이다.
- Debug 링크·EXE 교체·사용자 재시작: 미완료. 현재 실행 중 Client에는 복구 OBJ가 적용되지 않았다.
- Client/Server 실행·종료·UI 조작: 수행하지 않았다. 실제 Complete Play와 entry trigger의
  최종 화면은 사용자 확인 대상이다.

진단과 재검증 파일은 out/KoukuSequenceLifetime20260924의evidence.json,
memberwise-obj-inventory.json, installed-copy-constructor-match.json, client-obj-run.log,
source-hashes.json이다. 정상 Debug 컴파일 뒤verify_after_build.py를 실행해 새
Animation_Tool 복사자의 최신 Document OBJ 일치와 header 추적을 함께 확인했다.


## G03. 후속 첨부의 Unknown model과 빈 타임라인

후속 이미지의 `Unknown model`은 별도 모델/asset catalog 누락이 아니라 동일 Source In
실패로 격리된 패턴의 표시다. Workbench의 `Actor_Label`은 `actorProfileId`의 세 문자열만
비교하며 리소스 파일을 로드하지 않는다. Composition parser는 패턴 검증 실패 시 원본
JSON을 `strPreservedJson`에 보존하고 `stagedPattern`을 초기화한 뒤 ID/name/gate/folder만
복구한다. 따라서 `actorProfileId`와 stages/lanes가 빈 상태로 격리된다. 선택된 패턴의
`actorProfileId`를 Create Pattern의 Target Boss에도 쓰므로 그곳에도 Unknown model이
표시된다. 원본 저장 데이터를 삭제한 것이 아니다.

이미지에서 선택된 십자화염폭발 P40은 실제 저장본에서 `MN_RPCT_05`,
`boss.kakulsaydon.g3.saydon`, stages 2, presentation 3이다. 유일하게 모델명이 남은
분신소환 십자화염폭발 P45는 stages 0 / presentation 0인 빈 부모여서 문제의 occurrence
복사자를 거치지 않는 것과 일치한다.

같은 저장본을 정상 native codec으로 다시 읽은 결과 Action 119개와 Sequence 10개 모두
known model이며 unknown 0 / unavailable 0이다. P40의 actor·2 stages·3 boxes와 P45의
actor·0 stages·0 boxes를 확인했다. 로그는
`out/KoukuSequenceLifetime20260924/unknown-model-native-parse.log`다.
추가 제품 코드나 데이터 수정은 필요하지 않다. PID 36716/49220은 기존 18:45/18:44 실행
프로세스이므로 후속 컴파일만으로 메모리의 이전 복사자나 격리 상태가 갱신되지 않는다.
정상 링크된 EXE로 사용자가 재시작한 뒤 화면을 확인해야 한다.
