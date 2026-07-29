# LostArk 발탄 코어 자동 조립 결과

작성일: 2026-07-29

## 결과

- `Floor01`, `Floor01A`, `Floor01B`, `Statue01` 네 `.wmodel`은 source-space 공통 원점을 보존한다는 Blender 검증 결과를 런타임 계약에 반영했다.
- Catalog 전체 17개 중 코어 네 조각만 `Origin`으로 전환했다. 나머지 13개는 일반 월드 배치 편의를 위해 `BottomCenter`를 유지했다.
- MapTool 툴바에 `Assemble Valtan Core`를 추가했다.
- 자동 조립은 네 조각을 position `(0,0,0)`, rotation `(0,0,0)`, scale `(1,1,1)`로 생성한다.
- 기존 코어 배치가 하나라도 있으면 자동 조립을 거부해 중복 렌더링을 막는다.
- 네 clone 중 하나라도 실패하면 이번 명령에서 만든 객체를 모두 Layer에서 제거하고 placement 목록을 변경하지 않는다.
- 네 clone이 모두 성공한 경우에만 placement 목록, next ID, dirty 상태를 commit한다.
- 일반 `Arm placement`는 성공 후에도 armed 상태를 유지하므로 같은 바닥 에셋을 연속 클릭해 여러 인스턴스로 배치할 수 있다. `Esc`로 종료한다.
- Blender 메시 병합, 180도 복제, 내부 0.5배 복제, Navigation, StageData는 이번 코드에 섞지 않았다.

## 기존 사용자 배치 보존

`Client/Bin/DataFiles/Map/BG_RAD_VALTAN_A.mapplacements`는 수정하지 않았다. 검증 시점에는 수동으로 저장된 코어 placement 9개가 있으며 다른 asset ID는 없다. 따라서 최초 실행 순서는 다음과 같다.

1. F2로 AssetTest 진입
2. `Clear`
3. 확인 팝업의 `Clear all`
4. `Assemble Valtan Core`
5. 네 조각 결합 상태 확인
6. `Save`

기존 수동 배치를 비교용으로 남길 필요가 있으면 Save 전에 placement 파일을 별도로 복사해야 한다.

## 검증

- Catalog 실제 행: 17
- `Origin`: 코어 4행
- `BottomCenter`: 기타 13행
- 기존 수동 코어 placement: 9행, 보존
- `git diff --check` 대상 C++/catalog: 오류 없음
- `Framework.sln /t:Client /p:Configuration=Debug /p:Platform=x64`: 성공
- 결과 실행 파일: `Client/Bin/Client.exe`

빌드에는 기존 코드 페이지 `C4819`와 DirectXTK PDB `LNK4099` 경고가 있었지만 컴파일 및 링크는 성공했다. 실제 ImGui 클릭, 화면 결합, Save/Reload 확인은 실행 중 사용자 시각 검증이 남아 있다.

## 다음 작업

1. 자동 조립 결과를 저장한다.
2. 현재 네 조각을 기준으로 플레이 가능 영역과 충돌 범위를 확정한다.
3. Navigation을 베이크하고 이동 테스트를 먼저 마친다.
4. 이후 StageData 및 파괴 가능 외곽 구조물을 별도 근거로 조사·배치한다.
