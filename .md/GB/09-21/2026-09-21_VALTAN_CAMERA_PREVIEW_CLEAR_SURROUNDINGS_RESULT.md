# Valtan Camera Preview Clear Surroundings Result

## Root Cause and Fix

- 첫 수정은 `destroyable.group.valtan.outerwall109.*` 27개 그룹의 Deploy 54개만
  골랐다. 이들은 Arena 가장 바깥 ring이고, 사용자가 말한 "발탄이 서 있는 지면 위"의
  벽 구조물은 `wall.*`, `wall159.*`, entrance frontwall 및 group이 없는 구조물로 따로
  배치돼 있어 남았다.
- Valtan Deploy runtime은 145개 placement다. `destroyable.group.valtan.floor*`가
  소유한 floor/rail 6개만 제외하고, 나머지 139개 구조물을 선택한다. 139개는 기존
  outerwall109 54개, 안쪽/주변 벽 group 77개, group 없는 구조물 8개를 모두 포함한다.
- Camera Shots의 `Reload Shots` 오른쪽 `Hide Arena Outer Walls` 버튼은 이제
  이 139개 Deploy prop에만 camera-preview suppression을 적용한다. 성공하지 못하면
  버튼은 숨김 상태로 유지되지 않고 실패 원인을 MapTool status에 표시한다.
- suppression은 Deploy prop 자체의 별도 overlay다. authoring 문서와 저장 데이터,
  바닥·물·무대·발탄 배우, Server의 destruction 상태를 바꾸지 않으며, 컷신이 Deploy
  상태를 갱신해도 매 frame 다시 적용된다.
- MapTool을 닫거나 authoring Level이 바뀌면 overlay를 해제한다.

## Validation

- destruction 정본의 floor/rail group unique `memberPlacementIds` 6개와
  `LV_LUT_HEARTRB_ED.deployplacements`의 전체 placement 145개를 대조해 139개 선택을
  확인했다.
- `Client/Default/Client.vcxproj /t:ClCompile /p:Configuration=Debug /p:Platform=x64`
  성공. 오류 0개. 기존 EngineSDK 문자 인코딩 경고 4,961개는 이 변경과 무관하게 남는다.
- Client.exe가 실행 중이어서 링크와 실행 파일 교체는 수행하지 않았다.
- publisher는 실행하지 않았다. 이 변경은 runtime data가 아닌 Client 코드만 바꾼다.

## Remaining Screen Check

새 Debug Client를 실행한 뒤 발탄 최후 또는 발탄 스킬 연출(버러지)에서 버튼을 누른다.
MapTool status가 `Hidden 139 Valtan Arena structural Deploy props; the 6 floor props remain.`으로
표시될 때만 숨김이 적용된 것이다. 그 상태에서 컷신을 재생해도 Arena 바닥 위의 벽
구조물이 보이지 않고, 바닥·발탄 무대가 유지되는지 화면 확인이 남아 있다. `Restore Arena Outer Walls`나
MapTool 닫기로 원래 화면으로 돌아온다.
