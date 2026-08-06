# Effect Tool G07~G09 최종 코드 묶음

이 폴더는 `2026-08-05_EFFECT_TOOL_G07_REFERENCE_WORKSPACE_WORLD_PREVIEW_PLAN.md`부터
G09까지 실제 프로젝트에 반영된 코드와 데이터의 읽기용 정본 사본이다.

## 읽는 순서

1. `Client/Public/Effect_AuthoringDocument.h`: 저장되는 Effect와 Element 구조
2. `Client/Public/Effect_DocumentCodec.h` / `Client/Private/Effect_DocumentCodec.cpp`: JSON 저장·로드와 drawable 검증
3. `Client/Public/Effect_Tool.h` / `Client/Private/Effect_Tool.cpp`: 리소스·슬롯·Detail·Model View 저작 UI
4. `Client/Public/Effect_DocumentRenderer.h` / `Client/Private/Effect_DocumentRenderer.cpp`: 문서를 실제 GPU resource로 stage
5. `Client/Public/Effect_Object.h` / `Client/Private/Effect_Object.cpp`: 월드 Tick/LateTick/Render 객체
6. `CharacterPreviewPanel.*`, `MainApp.*`: Effect/Animation Tool이 같은 캐릭터·무기·피벗 session을 공유
7. `Client/Private/Loader.cpp`: 다섯 playable model은 공용 Prototype 경로로, Core/Summon은 preview-only 경로로 준비
8. `Client/Public/EffectAuthoringTransfer.h`, `Animation_Tool.*`: 선택한 Effect를 animation cue로 넘기는 저작 경계
9. `Effect_Catalog.*`, `Effect_PresentationService.*`: publish된 Effect를 런타임에서 찾아 재생하는 경계
10. `Engine/VIBuffer_*`, `Client/Bin/ShaderFiles/*`: Particle·Trail·Mesh·Sprite·Decal 실제 렌더링
11. `Data/Effects`, `Data/Animation`, `Data/Balance`: 차원술사 11개 스킬의 데이터 연결
12. `Tools/EffectPipeline`, `Tools/ProjectAudit`: publish·rollback·교차 문서 검증

## 적용 원칙

- 이 사본을 직접 빌드하지 않는다. 같은 상대 경로의 실제 `Engine`, `Client`, `Data`, `Tools` 파일이 빌드 대상이다.
- 새 C++ 파일은 `ProjectRegistration/*.xml` 항목대로 실제 `.vcxproj`와 `.vcxproj.filters`에 등록한다.
- Effect 원본은 `Data/Effects`, 런타임 생성물은 `Client/Bin/DataFiles/Effect`이다.
- 저장 가능한 초안은 미바인딩 Element를 허용하지만, publish·월드 preview·runtime은 `Validate_Drawable`을 통과해야 한다.
- Server는 action tick을 확정하고 Client presentation이 animation과 Effect 렌더링을 실행한다.

## 검증 명령

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Publish
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/EffectPipeline/Test-EffectPipeline.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-EffectToolFinal.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-EffectToolFinal.ps1 `
  -BundleRoot .md/GB/08-05/EffectTool_G07_G09_FinalCode `
  -ResourceRoot Client/Bin/Resources `
  -SourceRoot .
```

마지막 명령은 실제 소스와 이 묶음의 UTF-8 내용을 LF 기준 SHA-256으로 비교한다. CRLF/LF 차이만 무시하며 코드·데이터 내용 차이는 실패한다.
