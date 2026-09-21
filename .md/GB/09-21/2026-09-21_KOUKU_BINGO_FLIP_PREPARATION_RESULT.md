# 쿠크 빙고 flip 문서 준비 비용 제거 결과

## G01. 반영한 두 CPP

`Client/Private/WorldSequencePlayer_Objects.cpp`의 `Set_DocumentBatch`가 Object resource 전용 문서에는 무관한 전체 맵 placement read 및 Deploy clip 목록 수집을 수행하지 않는다. 실제 `CWorldSequenceDocument::Validate`와 batch stage/commit은 유지한다. 하나라도 Map/Deploy 등 Object 외 binding이 있으면 기존 수집을 수행한다. validator에서 외부 두 target table을 조회하는 곳은 해당 binding 분기임을 대조했다.

`Client/Private/KoukuSaydonPresentationPlayer.cpp`의 `Update_BingoMarks`는 같은 revision의 색 변경에서 기존 player, document, 준비한 model을 유지한다. 새 instance의 `Play → Seek_InstanceToMs(0)`가 성공한 뒤 이전 instance를 정리한다. 실패하면 새 instance만 정리하여 이전 색·모션·player를 유지한다. 재시도 대기 중에도 이전 칸을 Update한다. 최초 생성도 실제 첫 샘플 성공 뒤 commit한다. source revision이 바뀌면 새 문서 admission을 거친 기존 replacement 경로를 사용한다.

기존 `Apply_ObjectMotion`은 이전 모션의 effect tail을 보존한다. 현재 빙고 유지 효과는 300,000ms이므로 이를 바로 재사용하면 이전 색 유지 효과가 겹친다. 기존 `Play/Seek/Stop_Instance`로 모션 수명을 분리했으며 새 API나 별도 모델 경로를 만들지 않았다. 셀 위치·원본 flip/NEXT 연결·Server mask 권위·재시도 예산·셀 제거/Reset 정리는 유지한다. Header와 Data/Resources는 수정하지 않았다.

## G02. 실행한 검증

- 두 변경 CPP 전체 TU의 Debug scratch compile PASS. `out/KoukuBingoFlipPreparation20260921/compile.log`.
- 기준본과 변경본의 실제 `Update_BingoMarks`, `Set_Document`, `Set_DocumentBatch` 원문을 추출하여 native adapter를 각각 컴파일했다. 문서 Load/Validate는 현재 `WorldSequenceDocument.cpp`와 `DataJson.cpp`를 새로 컴파일한 실제 consumer다.
- 기준본은 25칸 생성 + 25칸 red 전환에서 target 수집 50회, 문서 admission 50회, player 생성 50회였다. 변경본은 25칸 생성 + 275회 색 전환에서 target 수집 0회, 문서 admission 25회, player 생성 25회였다. `baseline.log`, `current.log`.
- 변경본 1,059 checks PASS: 25개 실제 셀 좌표, native white flip/red flip 선택, 반복 전환 후 이전 instance 잔존 없음, 준비·첫 샘플 실패 시 이전 색 유지, 실패 새 instance 정리, retry 대기 중 이전 Update, generation retry, revision 교체, 제거/level teardown, 최초 sample 실패 시 미commit, Object batch 수집 생략, Map/Deploy 실제 문서 검증 및 수집 유지, 없는 target·잘못된 Object·중복 player·빈 batch rollback.
- `git diff --check` PASS. 기존 UTF-8 BOM 없음·CRLF 보존. 기준본은 `out/KoukuBingoFlipPreparation20260921/backup`, 이번 변경 SHA는 `edit-receipt.json`, native 추출 source SHA는 `source-receipt.json`에 있다. 다른 미커밋 변경은 기준본에 포함하여 보존했다.

## G03. 남은 실행 경계

native adapter의 GPU/effect/model 생성은 의도적으로 대체하였으므로 위 숫자는 실제 변경 함수의 호출 횟수이며 실제 GPU 표시·프레임 시간 측정 결과가 아니다. 실제 기존 `Play`, `Seek_InstanceToMs`, `Stop_Instance`, `Apply_ObjectEffects`의 수명 및 실패 경로는 소스로 대조했고 전체 TU 컴파일을 마쳤다. Client/Server/UI 실행, 제품 링크, 게시와 최종 화면 검증은 이 하위 작업에서 수행하지 않았다. 부모 작업자가 전체 제품 빌드와 관련 원본 이펙트 변경을 통합한다.
