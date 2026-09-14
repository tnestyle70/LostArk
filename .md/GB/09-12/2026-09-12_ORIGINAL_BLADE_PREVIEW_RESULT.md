# 원본_칼날 6개 교차 미리보기 결과

## 구현

- 원본_칼날 template/instance 한 쌍 추가. 기존 리소스/기본 모션/갈고리 등 모든 기존 행 semantic equality 통과.
- 6개를 서로 다른 횡방향 경로에 놓고 홀수/짝수 반대 방향으로 이동. 간격2m, 이동28m/14초, 생성 지연0. 설치 원본 참조 본체 메시를 재사용하며 animationTracks는 비어 있다.
- source01 발사체와 source02 로컬 변형 자료를 구분한다. 이동/배치/회전은 저작값이며 원본 파티클/사운드/판정 전체 복원은 아니다.
- C++/리소스/제품 패턴은 변경하지 않음. Client/UI 자율 실행 및 화면 판정 없음.

## 자동 검증

{
  "vertexPositionsAndUnorientedTrianglesEqual": true,
  "indexBytesEqual": false,
  "vertexCount": 28228,
  "meshSha256": "c69b8861cf3d27db9ce8aa0bc85ee4ee92e961be6336760be0823ac38ceebffd",
  "uprightSpinRadiusM": 0.9799186750342795,
  "lateralThicknessM": 0.1609534609670804,
  "laneSeparationM": 2,
  "travelDistanceM": 28,
  "travelDurationMs": 14000
}

revision 1415 → 1416. 공식 publisher 결과는 실행 후 기록한다.

## 최종 검증 및 인계

- 공식 WorldSequences Validate, Publish, Check 성공. 게시 전 source1415/runtime1415가 같고, 추가 후의 차이는 칼날 template/instance 한 쌍과 revision뿐임을 검사했다. 과거 runtime675/authoring672의 불일치를 현재 상태로 오인하지 않는다. 첫 게시 안전 거절 후 위 비교 증거를 추가 확인하여 같은 공식 게시 요청이 승인됐다.
- 기존 모든 row 보존, 6개 교대 방향, 2m 경로 간격, 28m 이동과 14000ms 숨김 endpoint의 실행형 수치 검사 통과. WorldSequenceAuthoringContract의 version/rollback, invalid refs/enums/scales, loaded selection 3개 테스트 통과.
- 정점 위치 및 winding 제외 삼각형 연결은 원본 참조 모델과 일치한다. index/전체 WMSH byte는 다르며 전체 원본 이펙트 fidelity PASS가 아니다.
- 코드/리소스 변경이 없어 빌드를 하지 않았다. Client 화면과 실제 UI Save/Reload 왕복은 미검증이며 사용자 확인 대상이다. 기존 갈고리의 delay/Lifetime 문제를 이번 변경으로 고쳤다고 주장하지 않는다.
- 확인 경로: F1 → World Object Tool → 미저장 편집이 없는지 확인 → Reload Source → 월드오브젝트_칼날 → 원본_칼날. Preview at Character 해제 후 Play. 1/3/5와 2/4/6은 반대편에서 출발해 서로 다른 통로로 교차한다. 전체14초, Count6, 지연0. 변경 전 검사 시 Client/Server 프로세스는 발견되지 않았고 에이전트가 실행하지 않았다.
- git fetch는 FETCH_HEAD 쓰기 권한으로 실패했다. 다수 사용자 변경이 있는 브랜치를 자동 stage/commit/push하지 않았다.

## G02. 원본_칼날 삭제 완료

사용자 요청으로 sequence.kouku.cutting_blade.original_preview와 world.sequence.instance.kouku.cutting_blade.original_preview 한 쌍만 제거했다. revision1662→1663. 바닥_칼날의 6개 배치/11000ms/크기/속도, 공용 칼날 resource와 모델/텍스처, 갈고리 및 나머지 모든 JSON 값을 보존했다. 외부 Data 참조는 없었으며 제거 후 문서 내부 dangling reference도 없다. 삭제된 실제 template/instance는 C:/Users/USER/.codex/worktrees/7395/LostArk/.codex_tmp/original_blade_deleted_fragment.json에 보관해 복구 가능하다. 공식 WorldSequences Validate/Publish/Check와 scoped diff check를 실행했다. 새 비교 모션의 과거 추가 기록은 이 문서의 이력으로 보존한다. Client/UI를 실행하거나 화면을 판정하지 않았고, C++ 무변경이라 재빌드는 하지 않았다. 미저장 편집이 없다면 Reload Source로 목록을 갱신한다.
