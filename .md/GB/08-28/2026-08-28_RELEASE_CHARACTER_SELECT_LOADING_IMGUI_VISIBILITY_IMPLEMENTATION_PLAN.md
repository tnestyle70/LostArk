# Release Lobby·Character Select·Loading ImGui 비노출 구현 계획

## 목표

Release Client에서 다음 진단용 ImGui 창을 제출하지 않는다.

- `LostArk Lobby`
- `Character Select`
- `Loading progress`
- `Loading recovery`

ImGui runtime 자체는 authored UI draw-list, screen-space hit test, nickname modal과 IME가
사용하므로 유지한다. Win32 `파일/도움말` 메뉴는 이번 사용자 요청 범위가 아니므로 바꾸지 않는다.

## 구현 경계

1. Lobby 진단 panel은 `_DEBUG`에만 남긴다. Release에는 authored layout의 stable slot으로
   `Test`, `Character Select`, `Valtan`, `Bern` 네 typed command를 제공한다. 요청 중 중복 click은
   현재 `CLevel_Lobby` entry/transition 상태로 차단한다.
2. Character Select 진단 panel은 `_DEBUG`에만 남긴다. 공통 hidden product input host가 authored와
   Debug Create Character 요청을 한 번 소비하고, 같은 ImGui ID stack에서 nickname modal을 연다.
3. Loading progress 진단창은 `_DEBUG` helper로 이동한다. Lobby load 실패의 Retry는 별도
   `LoadingRecovery.json` authored overlay와 screen-space hit test가 기존 typed retry flag를 제출한다.
4. 새 JSON은 Client project `96.DataFiles/UI`에 등록한다.
5. source/data contract를 정본 Debug/Release regression에 등록한다.

새 C++ 파일이나 두 번째 UI runtime은 만들지 않는다. Loading recovery는 기존
`CHUDRuntimeView`, Lobby는 기존 `CLobbyCommandService`, Character Select는 기존 modal 계약을
그대로 확장한다.

## 검증

```powershell
python -B Tools/Build/test_release_client_surface_contract.py
powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug `
  -ResourceRoot Client/Bin/Resources -AllowLocalEffectResources
powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release `
  -ResourceRoot Client/Bin/Resources -AllowLocalEffectResources
git diff --check
```

에이전트는 Client/UI를 실행하거나 visual PASS를 기록하지 않는다. 병합본 Release를 사용자가 직접
실행해 네 진단창 비노출, 네 Lobby 버튼, nickname modal, Loading retry overlay를 확인한다.
