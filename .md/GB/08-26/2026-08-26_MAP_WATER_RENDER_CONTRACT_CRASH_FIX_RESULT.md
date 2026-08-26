# Bern/Valtan map water render-contract crash fix result

작업일: 2026-08-26

## 1. 결론

로비에서 베른·발탄으로 직접 진입하거나 Character Create 뒤 베른으로 진입할 때 보이던 종료는
로더 실패나 네트워크 실패가 아니었다. 두 Level은 로드를 마친 뒤 첫 화면의 BLEND render group에서
`CMapAssetObject::Render()`가 `E_FAIL`을 반환했고, `CMainApp`이 설계된 render-failure 경로로
프로세스를 종료했다.

공통 원인은 물 렌더 기능의 consumer/shader 불일치였다. `CMapAssetObject`는
`Shader_VtxMeshBinary.hlsl`을 사용하지만 물 변수와 물 pass는 정적 인스턴스 셰이더에만 있었다.
더구나 object shader의 pass 15는 Valtan deferred emissive overlay가 이미 사용하고 있었다.

## 2. 확인한 증거

`Client/Default/ClientExit.user.log`:

```text
2026-08-26 09:05:21 Render failed hr=0x80004005 level=5
2026-08-26 09:20:10 Render failed hr=0x80004005 level=4
2026-08-26 09:42:20 Render failed hr=0x80004005 level=5
2026-08-26 09:45:03 Render failed hr=0x80004005 level=4
```

`Client/Default/RendererExit.user.log`:

```text
stage=Render_Blend_Object hr=0x80004005 object=class Client::CMapAssetObject
stage=Render_Scene hr=0x80004005
```

Level 4와 5가 같은 object와 같은 render stage에서 종료됐으므로 세 가지 진입 경로는 하나의
렌더 계약 결함으로 수렴한다.

## 3. 반영한 수정

- `Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl`
  - `Bind_WaterShaderResources`가 쓰는 모든 water scalar/vector/flag와 camera/time 변수를 추가했다.
  - RT0 scene color와 RT1 distortion을 출력하는 `PS_MAIN_WATER`를 추가했다.
  - shadow pass 12-14 다음에 water pass 15-17을 추가했다.
  - 기존 Valtan deferred emissive overlay를 water pass 뒤 pass 18로 이동했다.
- `Client/Private/DeployPropObject.cpp`
  - Valtan floor emissive overlay의 shader-specific pass를 18로 맞췄다.
- `Tools/LevelPlacementExtractor/test_valtan_floor_emissive_contract.py`
  - overlay pass의 새 위치와 기존 emissive-only write contract를 함께 검사한다.
- `Tools/ProjectAudit/Test-MapWaterRenderContract.ps1`
  - 실제 object shader prototype, 모든 water binding/선언, pass 순서, static-batch DEFERRED 전용
    경계를 한 번에 검사하는 새 실행형 회귀 테스트다.

## 4. 자동 검증

```text
Test-MapWaterRenderContract.ps1                                  PASS
test_valtan_floor_emissive_contract.py                           6/6 PASS
fxc /T fx_5_0 Shader_VtxMeshBinary.hlsl                          PASS
Client.vcxproj x64 Debug                                         PASS
  Shader_VtxMeshBinary.cso 생성                                  PASS
  Client/Bin/Debug/Client.exe 링크                               PASS
git diff --check (이 변경 파일)                                  PASS
```

Client 빌드의 FXC X4717과 기존 C4819/LNK4099는 경고이며 오류는 0개다.

## 5. 수동 검증 경계

에이전트는 Client를 실행·조작하지 않았다. 사용자가 다음 세 경로를 새 Debug 실행 파일로 확인해야 한다.

1. Lobby -> Bern
2. Lobby -> Valtan
3. Lobby -> Character Create -> Bern

세 경로 모두 로딩 완료 뒤 첫 화면에서 종료되지 않아야 한다. 베른 물 표현과 발탄 바닥 발광의
최종 시각 품질 판정도 사용자 소유이며 자동 PASS로 기록하지 않는다.

