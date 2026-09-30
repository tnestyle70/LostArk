# 베른 최적화·복원 및 PR490·491·492 통합 결과

## G00. 범위와 현재 상태

현재 브랜치 `codex/valtan-authoring-and-entry-20260930`을 유지하고 현재 작업을 기능별
checkpoint 14개로 보존한 뒤 origin/main과 PR490·491·492를 merge했다. checkout,
stash push/pop, Clean/Rebuild, OBJ/PCH/tlog 삭제와 shader timestamp 보정은 하지 않았다.
최종 Release 제품 빌드와 실행 ZIP은 아래 완료 기록을 기준으로 판단한다.

사용자가 우선 전달을 요청하여 Release 통합 실행본을 먼저 완성한다. Debug의 이전 빌드
실패와 미반영 shader는 Release 성공으로 대신 승인하지 않는다. Client/UI 실행·화면 판정은
사용자가 수행한다. 콜로세움 신규 원본 재질 복원은 사용자 범위 조정으로 조사만 남겼다.

## G01. 보존한 작업과 병합

- 베른 NPC의 화면 밖 clock-only update, 재진입 pose 복구, conservative animation envelope,
  Layer phase 생략, final-camera map 제출·shadow 독립 판정과 profiler 13분류를 유지했다.
- 베른·캐릭터 선택 원본 지형 42개, 마하라카 원본 환경, 워터팡 효과 23개·AI·3분 경기·
  AI Tool·주민 걷기/달리기, 같은 작업 트리의 발탄·쿠크·Guardian·World Movie 보완을 보존했다.
- PR490은 `aa5a78170`, PR491은 `965c565c0`, PR492는 `69981324d`로 로컬 병합했다.
  PR492의 베른 임시 FLOORFILL 434개는 원본 지형 복원과 중복하므로 12개 관련 파일에서
  제외했다. Colosseum 등록과 기존 입장·로딩·컷신·카운트다운·관중은 포함했다.
- PR491의 class 정렬은 현재 선택 슬롯 유지 계약과 충돌하므로 채택하지 않았다.
  Guardian의 미지원 헤어 index1은 인덱스를 압축하지 않고 catalog 기준으로 선택을 막고,
  기존 잘못된 저장 선택은 지원 기본값으로 회복한다.
- 새 Data 15개를 Client 프로젝트의 `None`/`96.DataFiles`에 등록했다. FxCompile 설정은
  변경하지 않았다. Resources와 빌드 산출물은 Git에 포함하지 않는다.

## G02. 통합 네트워크와 상태 보존

protocol129에서 기존 packet1~112는 보존하고 Colosseum113~116, Repair117,
Waterpang tuning118/119로 충돌을 해소했다. Spawn suffix는 양쪽 모두 voice U8 다음
Waterpang NPC ID 문자열이다. 자동 merge의 writer/reader 순서 비대칭을 교정했다.
inventory snapshot 끝에는 durability6개가 있으며 잘못된 값·잘린 payload는 출력 상태를
보존하며 거절한다. Waterpang의 범위 오류는 Server typed 응답을 사용하는 계약을 유지한다.

Colosseum matched seat에 voice를 전달하고 같은 session의 solo·party·NPC 귀환·trigger·
Debug world transfer에 durability6개와 wear cursor를 전달했다. 새 session은 원래100%로
시작한다. Server stage에서 invalid carried state를 검증한 뒤 commit하므로 실패 시 기존
player/session을 보존한다.

실제 Release 검증은 NetworkProtocolHarness1421, NPC 귀환54, CharacterAdmission71,
합계1546 PASS/0 FAIL이며 각 프로세스 exit0이다. 구체적 로그와 바이너리 SHA는
`out/PR490-492Integration20260930/final-release-contract-verification.json`에 있다.

## G03. 셰이더·최적화·데이터 검증

세 PR 병합 전후 shader input664개와 CSO574개의 SHA256·mtime는 전부 동일했다.
근거는 `three-pr-shader-preservation.json`이다. 최종 Release에서 이전 빌드 뒤23:53:41에
변경된 Waterpang native include를 소비하는 이펙트7개만 추가 FXC 대상이 됐다. Mesh·AnimMesh
본체를 PR 병합 때문에 다시 컴파일하지 않았다. final build의 실제 출력 개수는 완료 기록에 남긴다.

Release 읽기 검증은 producer254, dynamic SourceGroup84, Engine→Client 배포30 모두 PASS,
누락 CSO0·오래된 source/include0·배포 hash 불일치0이다. 개별 runtime shader consumer 검증은
`Test-CompiledShaderClosure.ps1 -Configuration Release -Modules Product` 결과로 구분한다.

정적 검증은 최적화26파일 중25개 동일, ClientReplication만 voice 전달6줄 추가이며 기존
최적화 opt-in을 유지했다. JSON103개·XML4개 parse PASS. 베른 authoring/runtime24shards는
각각50,021 stable sourceID가 같고 Landscape42 visible1, FLOORFILL0이다. Colosseum은
103assets/1,294배치, world6spawn+40NPC, loader/runtime 동일 scope다. Waterpang31개
관련 source/Data의 프로젝트·filter 등록이 각각 하나임을 확인했다.

## G04. Resources 충돌과 Drive 전달 범위

사용자가 지정한 `Downloads/TJ_Resources (5).zip`, `CY_Resources (3).zip`은 이미 공용
Drive에서 받은 기준본이다. 같은 파일을 GBResources에 중복 업로드하지 않는다. 초기 의존성
통합 모음에서 이 두 ZIP과 byte 동일한 항목 및 새 파일이 아닌 기존 voice WAV 의존성을
제외하고 이번 복원·충돌 해결분만 남긴다. 제외 파일은 out 보존 폴더로 이동하며 설치본은 유지한다.

충돌이 확인된 주민4모델은 GBResources가 기존 모든 section을 보존하고 run/walk만 추가한
복원본이었다. 설치본에 복구했다. Guardian CustomizingAnimSet은 TJ 설치본이 기존 idle을
보존하고 pose4개를 추가한5clip 모델이므로 그 모델을 유지했다. SHA와 전체 WModel 파싱,
기존 mesh/material/skeleton/clip section byte 동일성까지 확인했다.

누락 Money_Silver/Gold 아이콘은 기존 `build_shop_ui.py`의 원본 atlas·crop 좌표로 두 파일만
복구했다. 다른 UI를 재생성하지 않았다. 해당 PNG의 source crop RGBA 일치와 설치 hash를 확인했다.
콜로세움 리소스는 기존 PR 전달분만 사용하며 원본 재질 복구 완료로 부르지 않는다.

최종 GBResources는 1,299개/1,102,971,516 bytes다. TJ·CY ZIP과 동일한 1,201개와 기존
voice WAV 2,652개, 총3,853개/983,507,982 bytes를 out 보존 폴더로 이동했다. 삭제0,
미확인 충돌0이며 설치 Resources는 유지했다. ZIP과 다른 주민4종 run/walk 복원본은 남겼다.
`already-shared-resources-result.json`과 파일별 이동 journal에 SHA CAS 검증을 기록했다.

## G05. 빌드·실행 ZIP·남은 사용자 확인

Release 최종 Product는 4개 프로젝트 모두 PASS다. 정본 결과는
`out/BuildPipeline/runs/20260930T153129533Z-release-product.json`이며 runtime input 누락·오류0이다.
Engine은 OBJ0/CSO0/binary0, Shared는 OBJ3/binary1, Server는 OBJ90/binary1,
Client는 OBJ350/CSO7/binary1을 기록했다. PCH 재작성0, tracking identity 변경0이다.
MeshBinary와 AnimMeshBinary의 재컴파일은0이다. compiled shader closure도 PASS이며
격리 WARP V1/V2 각1352 pixels·resource-root8cases·Effect148program을 확인했다.

실행 ZIP `C:/Users/user/Desktop/LostArk-Verification-20261001-Integrated.zip`을 생성하고
검증했다. 168,818,949 bytes이며 SHA256은
`affa6e605191606af48116cd68a5a7690ed241996c933bd4ef3e5f3e1b7a9472`다. 기존
`LostArk-Verification-20260930-GuardianFixed.zip`은 보존했다.
Client/Server/Engine/CSO/Data/DataFiles와 launcher를 포함하고 Resources는 외부 폴더를 사용한다.
payload2810개·directData2167개·CSO256개, protocol129다. 새 Waterpang AI/voice/Guardian/
Colosseum source와 published inputs가 포함됐다. ZIP CRC·내부 SHA·launcher `--check`가
PASS이며 Client/Server는 실행하지 않았다. 패키지 도구 기존20개 검사도 PASS했다.
생성 증거는 `out/ReleasePackaging/portable-delivery.receipt.json`,
`preflight-20261001-integrated.json`이다.

이번 변경의 실제 FPS와 화면, NPC 재진입, Colosseum 팀 입장, Waterpang 경기·3분 종료와 AI Tool
UI는 사용자 확인이 남는다. 60/100fps를 보장하거나 구조·컴파일 검증을 플레이 성공으로 바꾸지 않는다.
