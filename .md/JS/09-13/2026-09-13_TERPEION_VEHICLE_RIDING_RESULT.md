# 황금 테르페이온 탈것 탑승 결과

작성일: 2026-09-13. 브랜치: `feature/terpeion-vehicle-riding`. 계획: [PLAN](2026-09-13_TERPEION_VEHICLE_RIDING_PLAN.md).
로컬 커밋 기준 상태다.

## 구현 상태

| G | 내용 | 상태 |
|---|---|---|
| G01 | 몸체 `mn_pmstg_01_mi` SourceCharacter program 85 생성·설치, `Model.cpp` 상한 85, 원본 TGA 11개 설치 | 완료 |
| G02 | 창술사·워로드·도화가·차원술사 `<Class>_RideHorseAnimSet.wmodel` 쿠킹, `CharacterCatalog.json` 등록 | 완료 |
| G03 | Shared protocol 82: `PLAYER_SNAPSHOT.iVehicleId`, `C2S_SET_VEHICLE_RIDING`/`S2C_SET_VEHICLE_RIDING_RESULT`, 하네스 | 완료 |
| G04 | Server `CVehicleCatalog`, 탑승 판정·강제 하차·탈것 속도, 스킬/에스더 거부, 계약 테스트 | 완료 |
| G05 | `Data/Vehicles/VehicleProfiles.json`, `Publish-VehicleProfiles.ps1`, BuildDomains `vehicles.profiles` | 완료 |
| G06 | `Data/Actors/VehicleCatalog.json`, NpcCatalog의 `VEHICLE_TERPEION_GOLD` 행 삭제, `CActorCatalog` 탈것 파서 | 완료 |
| G07 | `CPart_Vehicle`, `CVehiclePresentationAssetService`, `CCharacter` 좌석 합성·탑승자 클립, Replication, Loader | 완료 |
| G08 | H 키 명령 경로, 탑승 중 Client 스킬·에스더 입력 차단 | 완료 |
| G09 | Effect Tool V2 Attach 대상에 탈것 추가, EffectV2 runtime 태그 조회 | 완료 |
| G10 | 문서 갱신, 빌드·하네스·계약 테스트 | 완료 |

## 계획과 달라진 점

- **G02 차원술사:** 쿠킹 결과가 원본 225 bone이라 기존 236 bone(09-09 hair55 11개 추가) 애니셋과 attach가 맞지 않았다.
  `out/VehicleTerpeion20260913/splice_skeleton.py`로 Esther 애니셋의 WSKL·carrier mesh에 탑승 클립 2개를 옮기고
  WANM trailer를 236 bone skeletonHash로 바꿨다. 225 bone이 236의 앞부분과 순서까지 같은지 먼저 검사한다.
- **G03 하네스:** 기존 World snapshot 크기 검사에 `playerVehicleBytes = 4`를 추가했다.
- **G06:** `Client.vcxproj/.filters`의 `96.DataFiles\Actors`에 `VehicleCatalog.json` None 항목을 추가했다.
- **G08:** 소비자가 없는 `Get_VehicleRidingStatus()` 대신 같은 문구를 `[Client][VehicleRiding]` 디버그 출력으로 남긴다.
- **G04 계약 테스트:** 워로드 테스트 플레이어의 `eStance`를 `WARLORD_NORMAL`로 두었다. `NONE`이면 서버가 방어 태세 유지로 보고 속도 배율을 걸어 하차 후 속도 검사가 실패했다.

- **탈것 모델 UV1 (사용자 첫 테스트 후):** Character Select에서 H 탑승은 Server에 적용됐으나(속도 증가·스킬 차단) 말과 탑승 자세가 나오지 않았다.
  갈기 `mn_pmstg_01-3_mi`의 program 18은 native UV1을 요구하는데 NPC 파이프라인 쿠킹본 `Terpeion.wmodel`은 1.0(UV1 없음)이라
  `CModel`이 "source character requires native extra UV channels"로 모델 전체를 거부했고, 탈것 원형이 없어 Client 표현이 도보로 격리됐다.
  원본 PSK는 UV set 1개(EXTRAUVS 없음)이고 UE3는 없는 TexCoord[1]을 마지막 set으로 clamp하므로 갈기 submesh(8537 vertex)의 UV1을 UV0로 채웠다.
  도구: `Tools/VehiclePipeline/cook_single_set_uv1.py`(`cook_skinned_uv_contract`로 1.3 변환, legacy stream·비메시 section 보존 증명).
  원본 1.0 파일은 `out/VehicleTerpeion20260913/Terpeion.v1_0.wmodel`에 보존했다.
- **하차 표현 (사용자 두 번째 테스트 후):** UV1 수정 뒤 탑승은 표시됐지만 H로 하차해도 말이 남았다. Server는 하차를 처리했고
  Client 디버그 출력에도 `[Client][VehicleRiding] Dismounted.`가 기록됐다. `CCharacter::Apply_NetworkVehicle`의 재시도 억제 조건이
  초기값 0인 `m_iRejectedVehicleId`와 하차 id 0을 같은 값으로 보고 return했다. 억제는 0이 아닌 id에만 적용하도록 고치고 Client를 다시 빌드했다.

- **탑승 중 무기 숨김 (사용자 요청):** 무기가 탈것을 관통해 `CCharacter`의 무기 가시성 세 경로(태세, 장비 미리보기 태세, 기본 장비 복원)에
  `0u == m_iVehicleId` 조건을 더하고 탑승·하차 commit 뒤 `Apply_NetworkStance(m_eStance)`로 다시 계산한다.
- **갈기 dither 점 (사용자 요청, 범위: 탈것 갈기만):** 갈기 부모 `pc_hair_trn`은 `BLEND_Translucent`·`TwoSided`인데 기존 deferred는
  program 6/7/18/20/29/84를 4×4 ordered dither로 그린다. 갈기 diffuse 알파는 0 31%·중간 50%·255 19%(워로드 헤어 255가 92%)라 점이 드러났다.
  `CPart_Vehicle`이 program 18 mesh를 NONBLEND에서 빼고 BLEND 그룹 `Render_Group`에서 `Shader_VtxAnimMeshBinary` pass 9
  `SourceCharacterTranslucentTwoSided`(Cull None, depth read-only, alpha blend)로 그린다. PS는 native base program으로 diffuse·indirect·opacity를,
  forward 광원마다 `SourceCharacterLight18`로 직접광을 계산하고 광원별 diffuse×ambient를 더한 뒤 scene fog를 적용한다(deferred marker-5 합성식과 같다).
  광원·ambient·fog는 `CMapAssetRenderUtils::Bind_SourceCharacterForwardLights`(기존 map forward binder에 ambient 배열 옵션 추가),
  light constants는 새 Engine API `CModel/CMaterial::Bind_SourceCharacterForwardLight`로 바인딩한다.
  deferred와 달리 동적 그림자·SSAO는 적용하지 않는다. `fxc /T fx_5_0 Shader_VtxAnimMeshBinary.hlsl` 컴파일 성공(신규 코드 경고 없음).
  Product Debug 빌드 성공(`out/BuildPipeline/runs/20260913T132536489Z-debug-product.json`) 후 로컬 Server·Client 재시작.
  사용자 관찰(무기 숨김·반투명 갈기 빌드): "개이뿌덩".

## 자동 검증

| 검사 | 결과 |
|---|---|
| `build_vehicle_source_material.py verify` program 24(`mn_ppch_00-1_mi`) | base/light/configure EXACT |
| 같은 verify program 85(`mn_pmstg_01_mi`) | base/light/configure EXACT |
| Engine·Client `Shader_SourceCharacter{Base,Light}Programs.hlsli` | SHA-256 동일 |
| `VehicleCatalog.json` override | `rows.json`과 float32 값 동일(3행, 파라미터 115, 텍스처 18) |
| JSON parse: VehicleCatalog, VehicleProfiles, NpcCatalog, CharacterCatalog, BuildDomains | 성공 |
| `Publish-VehicleProfiles.ps1` Validate → Publish | `VEHICLE 6705 5` 1행. 속도 6.0 변조 복사본은 거부 |
| 4직업 애니셋 `validate_wmodel` + `compare_attach.py` | OK, bone table·skeletonHash 일치(224/218/239/236) |
| Product Debug 빌드 `Invoke-BuildAndRegression.ps1 -Configuration Debug` | Engine/Shared/Server/Client PASS, OBJ 219·CSO 7 갱신. `out/BuildPipeline/runs/20260913T120829701Z-debug-product.json` |
| Server 재빌드(테스트 수정 후) | 성공 |
| Client 재빌드(하차 수정 후, `Client.vcxproj` Debug) | 성공, 오류 0 |
| `Terpeion.wmodel` UV1 추가 후 `validate_wmodel` | OK |
| `NetworkProtocolHarness` 빌드·실행 | `failures : 0` |
| `Server.exe --vehicle-riding-contract-test` | `vehicle riding failures: 0` |
| `Server.exe --contract-test` | `failures : 13`, 아래 참고 |
| `git diff --check` | 경고 0 |

`--contract-test` 실패 13개는 쿠크 Gate 3/Play All/Pattern 5 lifecycle 5개와 world bootstrap v7 trigger·NPC 파싱 8개다.
v7 항목은 `CWorldBootstrap`만 합성 파일로 검사하는데 현재 파서는 v8~v11만 받으므로 이번 변경과 무관한 기존 실패다.
쿠크 5개는 탈것 없는 플레이어에서 바뀐 경로가 없지만 main 기준 실행 결과는 없다. 로그: `out/VehicleTerpeion20260913/server-contract-test.log`.

## 문서 갱신

- `CLAUDE.md`: 현재 protocol 표기 v82(3곳), 탈것 계약 문단.
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`: 현재 protocol 표기 82(4곳), 입력 표 H 행.
- `.md/GB/렌더링이펙트복원V2.md`: 연결 범위 표에 황금 테르페이온 행.

## 실행 준비와 사용자 확인

로컬 Server(`--bind-address 127.0.0.1`, 작업 디렉터리 `Server/Default`)와 Client(`LOSTARK_SERVER_HOST=127.0.0.1`,
작업 디렉터리 `Client/Default`)를 새 빌드로 띄웠다. 화면 조작·캡처는 하지 않았다.

사용자 관찰(Character Select, UV1 수정 후 Client): "잘탄다". 같은 테스트에서 "탑승 상태에서 h 눌러도 말 안사라져" → 위 하차 수정.
이후 사용자 관찰(최종 빌드): "베른성에서 탑승/하차 잘 작동함, 탑승 상태로 발탄 입장 시 도보 시작함, 직업 변경 시 하차됨".
처음 계획한 확인 목록:

1. Character Select에서 H 하차 시 말이 사라지고 도보 idle/run으로 돌아오는지.
1. Bern 입장 → H: 말이 발밑에 나타나고 캐릭터가 등에 앉는지.
2. 우클릭 이동: 말 run + 탑승자 run 클립, 도보보다 빠른지(5 m/s).
3. 탑승 중 Q/LMB 무반응, H 다시 하차.
4. Character Select에서 같은 동작, 직업 변경 시 하차.
5. 탑승 상태로 발탄 입장 후 도보.
6. 말 재질(금색 장식, 몸체 IBL, 발광, 갈기).

## 다른 PC 준비

- Server PC는 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Server` 또는 `Publish-VehicleProfiles.ps1 -Mode Publish`를 실행해야 방이 준비된다.
- Resources(Git 비추적, Drive 전달 필요):
  - UV1을 추가한 1.3 `Character/Vehicle/Terpeion/Terpeion.wmodel`과 `Character/Vehicle/Terpeion/SourceMaterials/*.tga` 11개
  - `Character/{LanceMaster,Warlord,Artist,DimensionMaster}/AnimSets/<Class>_RideHorseAnimSet.wmodel`
- Server·Client·Shared는 protocol 82로 함께 빌드·재시작한다.

## 남은 범위

- 탈것 고유 스킬(96000/96010/96020/96030), 탑승·하차 클립, 소환 파티클, LookInfo 파티클, 발굽 사운드.
- Gunslinger/Slayer 탑승자 애니셋.
- 여러 탈것 선택 UI(현재 catalog 첫 탈것).
