# Effect Tool 완전 재구축 G0 결과

2026-08-04 기준 구 Effect 구현을 삭제하고, Effect Tool을 다섯 타입 중 하나만 고르는 ImGui G0로
처음부터 다시 만들었다. 현재 Effect authoring 파일, catalog, preview/runtime, 저장 포맷은 존재하지 않는다.

## 1. 실제 완료 상태

### 참고 이미지 보존

사용자가 제공한 PNG를 다음 경로에 원본 byte 그대로 저장했다.

```text
C:/Users/user/Desktop/툴/10_EffectTool_Integrated_Workspace.png
SHA-256 1403d9b11189a09220d0f0d308be33b7a70a24e3ad623008d336afa439bcc885
```

원본 임시 파일과 저장 파일의 SHA-256은 일치한다.

### 구 Effect 계약 삭제

다음을 삭제했다.

- `Effect_Types`, `Effect_AssetIO`, `Effect_ParticleSimulator`, `Effect_Runtime`, `Effect_ResourceCatalog`
- `Shader_Effect*` HLSL/HLSLI 4개와 project/filter 등록
- `Tools/EffectResourceIntake` Git 관리 파일 14개
- `Data/Effects/Authored`의 구 `.effect` 459개와 project/filter 등록
- Client의 `--effect-sim`, `--effect-roundtrip`, `--effect-phase2-test`와 helper code
- Engine의 `--effect-reset-layout` 예외
- 삭제 구현만 검사하던 ProjectAudit 항목
- 실제 Client GameObject 목록에서 삭제된 `CEffect_Runtime`을 제거한 `CLAUDE.md` 구조 설명

물리 파일 실측은 다음과 같다.

```text
Data/Effects/Authored files       0
Tools/EffectResourceIntake files  0
Shader_Effect*.cso                0
active Effect source files        Effect_Tool.h, Effect_Tool.cpp 두 개
```

Git ignore 대상이던 Python `__pycache__` 7개와 구 shader `.cso` 6개는 삭제 정책이 직접 제거를 막아
`C:/Users/user/AppData/Local/Temp/LostArk_EffectTool_Reboot_20260804_1309`로 격리했다. 워크스페이스에는
남아 있지 않으며 필요하면 이 임시 폴더에서 복구할 수 있다.

### 새 G0 UI

`CEffect_Tool`은 다음 session state만 소유한다.

```text
MESH / TEXTURE / PARTICLE / DECAL / TRAIL
default = MESH
```

ImGui radio 선택은 한 값만 유지한다. 파일 scan, Load/Save, Effect 생성, D3D resource 생성, preview,
Animation binding을 하지 않는다. `CMainApp`은 Effect debug tool 선택 시 device/context 없이 이 객체를
생성하고 종료 때 `unique_ptr`을 reset한다.

구 `imgui.ini`의 `[Window][LostArk Effect Tool]` 도킹·크기 상태가 G0에 재적용되지 않도록 보이는 제목과
별개인 내부 ID `LostArkEffectToolG0`를 사용한다.

## 2. 보존한 경계

다음은 수정하거나 삭제하지 않았다.

- Animation Tool과 Animation preview/model 경로
- Git 비관리 `Data/Effects/SourceCatalog`, `Data/Effects/SourceExtracted` 원본 추출 증거
- 외부 resource pack payload `Client/Bin/Resources/Effect`
- 기존 날짜별 RESULT/review 문서
- 사용자 선행 변경 `Data/AssetPacks.lock.json`과 삭제 상태의
  `Data/AssetManifests/lostark-resources-2026.08.03.4.manifest.json`

Animation cue가 clip time과 attachment binding을 소유하고, 향후 Effect asset은 effect-local 모양과
수명을 소유하며, Preview owner가 현재 pose/anchor transform을 공급한다는 경계도 팀 인계 문서와 bridge
계획에 동기화했다.

## 3. 자동 검증

### PASS

```text
Client.vcxproj XML parse                         PASS
Client.vcxproj.filters XML parse                 PASS
구 Effect active source/project direct consumer  PASS, 0건
effect.g0-selection-boundary                      PASS
git diff --check                                 PASS
Engine x64 Debug                                 PASS
Engine x64 Release                               PASS
UpdateLib.bat Debug                              PASS
UpdateLib.bat Release                            PASS
Client x64 Debug                                 PASS
Client x64 Release                               PASS
```

Release의 DirectXTK/Effects11 PDB 경고와 Debug의 기존 C4819 코드페이지 경고는 있었지만 새 G0의
컴파일·링크 오류는 없었다.

### ProjectAudit

ProjectAudit는 전체 PASS가 아니며 다음 4개에서 실패했다.

```text
asset-lock.inventory: files=10179 bytes=5405091440
ui.json-contract: documents=2 slots=111
projects.registered-files-exist: missing=1
projects.data-source-visibility: expected=92 project=93 filters=93
```

`missing=1`은 사용자 선행 삭제 상태인
`Data/AssetManifests/lostark-resources-2026.08.03.4.manifest.json`이고 data visibility 차이 1건도 같은
project item이다. asset-lock은 사용자의 resource pack lock/inventory 상태이며, UI JSON failure도 이번
Effect 삭제 범위 밖이다. 이 파일과 pack lock을 되돌리거나 project에서 임의로 제거하지 않았다.

## 4. 수동 검증과 미완료

이 환경에서는 실제 Client 창에서 F1을 누르고 Effect Tool radio를 클릭하는 수동 UI 검증을 수행하지
못했다. 따라서 다음은 수동 확인으로 남긴다.

1. Debug Client 작업 디렉터리를 `Client/Default`로 실행한다.
2. F1 Developer Tools에서 Effect를 선택한다.
3. Mesh, Texture, Particle, Decal, Trail을 차례로 눌러 항상 하나만 선택되는지 확인한다.
4. 선택 전후 Data, Resources, Animation 문서가 바뀌지 않는지 확인한다.

Reset/CreateEffect, resource palette, Model View, Effect Detail, All Effects, Data Files는 아직 구현하지
않았다. 다음 G는 G0의 enum과 UI 흐름을 확인한 뒤 한 계약씩 추가해야 한다.

## 5. Git 상태

작업 브랜치는 `codex/effect-tool-reboot`다. 사용자 소유 dirty resource pack 변경이 함께 있는 대규모
working tree이므로 자동 stage, commit, push는 하지 않았다.

## 6. G1 Effect Document boundary

2026-08-04에 G0 selector 위에 첫 메모리 문서 수명을 추가했다.

- `EFFECT_DOCUMENT_DESC`는 Effect asset 하나의 authoring draft다.
- `m_ActiveDocument`가 비어 있으면 ID/표시 이름/Create UI를 표시한다.
- Create는 입력 검증 후 local `StagedDocument`를 완성하고 optional에 한 번 commit한다.
- active document가 있으면 version, ID, 표시 이름, Element 수를 읽기 전용으로 표시한다.
- radio 선택은 G2 전까지 session state일 뿐 document를 변경하지 않는다.
- Discard는 메모리 문서만 제거하며 파일이나 GPU resource를 변경하지 않는다.

실제 반영 파일은 다음과 같다.

```text
Client/Public/Effect_AuthoringDocument.h
Client/Public/Effect_Tool.h
Client/Private/Effect_Tool.cpp
Client/Default/Client.vcxproj
Client/Default/Client.vcxproj.filters
Tools/ProjectAudit/Invoke-ProjectAudit.ps1
```

검증 결과:

```text
Client.vcxproj XML parse                         PASS
Client.vcxproj.filters XML parse                 PASS
새 C++ 3개 UTF-8 BOM 없음                       PASS
git diff --check                                 PASS
Client x64 Debug compile/link                    PASS
effect.g1-document-boundary                      PASS
독립된 줄의 G1 한국어 주석 복구 + rebuild        PASS
Debug Client G1 UI                               PASS, user screenshot
```

전체 ProjectAudit는 기존 resource lock/UI 상태와 원격의 Dimensionist→DimensionMaster 전환이 아직
현재 dirty worktree에 병합되지 않아 6개 항목에서 실패했다. G1 check 자체는 다음 detail로 통과했다.

```text
paths=0 authored=0 intake=0 shaders=0 symbols=0 project=0 entry=False document=True
```

사용자가 실제 Debug Client에서 F1 → Effect Tool 진입과 G1 화면 표시를 검증했다. 제공한 화면에서
빈 Effect Asset ID가 거부되고, New Document 입력창, Create 버튼, Mesh 기본 선택, G2 전까지
session-only라는 경계가 함께 확인됐다. 자동 입력이나 파일 저장은 G1 범위에 없다.
