# 2026-09-29 섬 입구 닻 표식 — 원본 ITR_10066 (Par_G_Symbol_Anchor_01) 복원 RESULT

빌드와 Client/Server 실행은 하지 않았다. 화면 판정은 사용자 몫이다.
이 변경은 새 native 셰이더 program 7개(4973~4979)를 설치하므로 **재빌드가 필요하다. 재시작만으로는 반영되지 않는다.**

## 0. 결론

- 이전 마커(effect.bern.anchor.marker.marker.full.restore)는 항구 입항 구역 프롭 ITR_10297(DockingVolume, 25 m 사각 테두리)의 표현이었다. 섬 입구의 닻은 다른 프롭이다.
- 섬 입구 닻 = Prop 1040158 = **EFDLProp_ITR_10066** = ParticleSystem FX_BS_03.mark.Par_G_Symbol_Anchor_01. 이 시스템을 이번에 복원해 같은 asset ID의 문서 내용만 교체했다.
- ITR_10297 표현은 삭제하지 않고 **effect.bern.harbor.dockingvolume.marker.full.restore** 로 분리해 보존했다. 현재 어떤 코드도 이 ID를 호출하지 않는다.
- Level_Bern.cpp는 바꾸지 않았다(같은 asset ID 유지). 게시(Publish)는 필요 없다.

## 1. 원본 체인

| 구분 | 내용 | 근거 |
|---|---|---|
| 사실 | 섬 입구 닻 소품은 Prop 1040158, Model EFDLProp_ITR_10066 | 섬 fork 조사 (SEA_ISLAND_ORIGINAL_REDO_RESULT 6.4)와 EFTable_Prop |
| 사실 | LookInfo(data4.lpk의 LookInfo/Prop/EFDLProp_ITR_10066.ITR_10066.loa, 1355 바이트)는 SkeletalMesh ITR_00.Mesh.ITR_Box128_SK (보이지 않는 128 cm 상자)와 CEFParticleData 블록 3개를 가진다 | 이번에 추출해 바이트 확인 |
| 사실 | 블록 3개가 모두 ParticleSystem FX_BS_03.mark.Par_G_Symbol_Anchor_01 을 가리키며 본(B_...) 지정이 없다 | 바이트 확인 (오프셋 316, 605, 890) |
| 사실 | 시스템은 이미터 10개 (스프라이트 8, 메시 2), 재질 7종 | UModel 획득 결과 |
| 사실 | 항구용 ITR_10297은 FX_ITR_10297.Par_D_ITR_10297_DockingVolume, 안전지대용 ITR_10118은 FX_BS_03.mark.Par_D_SafeZone_01/04 | LookInfo 추출 (ITR_10297과 10118 모두 다시 확인) |
| 추론 | 스크린샷의 금빛 닻 + 원형 광채가 이 시스템이다: 이름이 Symbol_Anchor이고, 닻 심볼 스프라이트 재질(master_01_072_tr + fx_d_symbol_035), 반달 메시 광채 재질(oceanmark_02_tr), 링 재질이 구성에 있다 | 이름과 재질 근거. 화면 1:1 대조는 하지 않았다 |
| 미확인 | 3개 블록의 역할: 같은 시스템에 플래그만 다르다. 블록 0(기본)만 사용했다. 세 블록이 동시에 세 인스턴스를 만드는 것인지 상태별 변형인지 확인하지 못했다 | 블록 2에는 2.0 값 두 개가 더 있으나 의미를 확정하지 못함 |

항구용과 섬 입구용의 차이: ITR_10297은 항구 입항 구역의 25 m 사각 테두리와 바닥 파문이고, ITR_10066은 섬 입구의 닻 심볼 + 반달 광채 + 링이다.

## 2. 복원 내용

- 빌더: Tools/EffectPipeline/build_island_anchor_symbol_source_effects.py (기존 build_anchor_marker_source_effects.py를 복사해 ITR_10066용 상수와 3블록·본 없음 처리만 변경).
- 문서: Data/Effects/Authored/effect.bern.anchor.marker.marker.full.restore.effect.json (요소 10개, 지속 14000 ms), 세계 root로 재바인딩 10/10.
- native program 7개 (4973~4979), 재질 실패 0, 보류 0. 프로그램 구간 4973~4991 중 사용, 다른 그룹과 충돌 없음(4961~4979 표에 모두 존재, 중복 없음).
- 새 리소스: Effect/Bern/AnchorMarker/Meshes/fm_b_halfsphere_003.wmodel (Client/Bin/Resources에 설치, CY_Resources에 같은 경로로 복사, 바이트 동일). 텍스처는 fx_d_symbol_035.dds(기존 설치)와 Kouku 텍스처(기존 팀 리소스) 재사용.
- 항구 표현 보존: effect.bern.harbor.dockingvolume.marker.full.restore (요소 7개, 기존 내용을 asset ID만 바꿔 복사). 기존 빌더의 ASSET_PREFIX를 effect.bern.harbor.dockingvolume. 으로 바꿔 재실행해도 섬 문서를 덮지 않게 했다.

## 3. 요소 전수 대조 (10/10)

크기·위치는 원본 cm를 100으로 나눈 m 값이다. 수명·지속은 초. 대조는 원본 모듈(source_module_inputs)의 lookup table과 문서 값을 직접 비교했다.

| 이미터 | 형태 | 재질 (프로그램) | 지속/수명 | 크기 원본 → 문서 | 위치 y 원본 → 문서 | 방향/비고 | 대조 |
|---|---|---|---|---|---|---|---|
| 24 | 스프라이트 | dark_05_01_tr (4973) | 4 / 4 | 140 cm → 1.4 | 5 cm → 0.05 | 축 고정 Z(바닥) | 일치 |
| 30 | 스프라이트 | circ_02_01_ad (4974) | 4 / 4 | 190 → 1.9 | 5 → 0.05 | 축 고정 Z | 일치 |
| 1 | 스프라이트 | ring_08_01_ad (4975) | 4 / 4 | 120 → 1.2 | 60 → 0.6 | 축 고정 Z, 속도 30 cm/s | 일치 |
| 8 | 스프라이트 | ring_08_01_ad (4975) | 4 / 4 | -120 → 1.2 (원본 모듈에 음수 그대로 보존) | 60 → 0.6 | 축 고정 rotate Z | 요약 필드는 절댓값, 모듈은 원본 |
| 25 | 스프라이트 | dark_05_01_tr (4973) | 4 / 4 | 110 → 1.1 | 75 → 0.75 | 빌보드 | 일치 |
| 12 | 메시(반달) | oceanmark_02_tr (4976) | 4 / 4 | 1.7 → 0.017 (프리스케일 0.01) | 없음 | 모델 fm_b_halfsphere_003 | 일치 |
| 14 | 메시(반달) | oceanmark_02_tr (4976) | 2 / 2 | -1.5 → 0.015 (음수 그대로 모듈에 보존) | 없음 | 위와 같은 모델 | 요약 필드는 절댓값, 모듈은 원본 |
| 2 | 스프라이트 | circ_02_ad (4977) | 4 / 4 | 250 → 2.5 | 40 + 35 (위치 모듈 2개) → 요약 0.4 | 속도 10 cm/s, 카메라 오프셋 15 | 모듈 2개 모두 보존 |
| 0 | 스프라이트 | master_01_072_tr (4978) | 4 / 4 | 100 → 1.0 | 60 → 0.6 | 빌보드. 닻 심볼(fx_d_symbol_035) | 일치 |
| 11 | 스프라이트 | ri_01_3_ad (4979) | 1.35 / 1.2~1.5 | 최대 30 → 0.1 | 0 | 파티클 42개, 위로 40 cm/s | 일치 |

- 부착: 10개 모두 follow=false, 슬롯 root, 소켓 항등, 기준 yaw 0. 마커는 Level 소유 세계 root로 그려진다.
- 위치: Level_Bern이 서버 입항 트리거 island.dock.to.maharaka (430.1, 10.95, -476.64) 중심에 그린다. 이 좌표는 원본 닻 소품 위치이다: 섬 원점 (440, -480)에서 원본 오프셋(서쪽 4.95 m, y 1.68 m)에 섬 스케일 2.0을 곱한 값과 일치한다(440 - 9.9 = 430.1, -480 + 3.36 = -476.64).
- 요약 필드(startSize, initialPosition)가 음수 크기와 위치 모듈 2개를 반영하지 않는 것은 문서의 요약 표시일 뿐이다. 재생은 문서에 보존된 원본 모듈을 따른다.

## 4. 실행한 검증

- LookInfo 바이트 추출, UModel 획득 (이미터 10, 재질 7), native 재질 복구 (프로그램 7, 실패 0), native 설치, 투영 (요소 10, 보류 0).
- 문서별 검증 함수 4종(재질 색공간, native 스프라이트 옵션, 모듈 override, 부착 방향): 섬 문서와 항구 문서 모두 실패 0. 문서 버전 13이라 v15 확장 검사는 해당 없음.
- 리소스 폐쇄: 섬 문서 8개, 항구 문서 3개 리소스 모두 존재, DDS와 WModel 시그니처 정상.
- JSON parse (카탈로그, 두 문서), 카탈로그 중복 ID 0, git diff --check 통과.
- 실행하지 않은 것: 전체 Validate-EffectSources.ps1 (쿠크 기존 파일 때문에 전체가 실패하는 상태), 빌드, Client 실행, 화면 확인.

## 5. 바꾼 파일

- Data/Effects/Authored/effect.bern.anchor.marker.marker.full.restore.effect.json (내용 교체)
- Data/Effects/Authored/effect.bern.harbor.dockingvolume.marker.full.restore.effect.json (신규, 항구 표현 보존)
- Data/Effects/EffectCatalog.json (항구 항목 추가)
- Tools/EffectPipeline/build_island_anchor_symbol_source_effects.py (신규), build_anchor_marker_source_effects.py (접두사와 표시 이름만)
- Client/Bin/ShaderFiles/Shader_EffectKoukuNativeGroup4928.hlsli, Shader_EffectArtistNativeSelectedGroup4928.hlsli, Shader_EffectArtistNativeDispatchKoukuNativeCases4928.hlsli, Client/Private/Effect_ArtistMaterial_Tables.inl (native program 4973~4979 추가)
- Client/Bin/Resources/Effect/Bern/AnchorMarker/Meshes/fm_b_halfsphere_003.wmodel 및 CY_Resources 복사본 (Git 비추적)

## 6. 남은 경계와 사용자 확인

- 재빌드 필요: native program 7개가 HLSL/표에 들어갔다. 이미 빌드된 Client.exe에는 이 program이 없어서 닻 표식은 재빌드 전에는 나오지 않는다(Client/Default/EffectFailure.user.log의 AnchorMarker.Bern 채널에 not prepared로 남는다). 이전 마커(4969~4972)는 항구용 문서에 그대로 남아 있다.
- 화면 확인: 금빛 닻 심볼, 반달 광채, 링, 바닥 원의 모양·밝기·높이. 특히 E14의 음수 메시 스케일, E2의 위치 모듈 합산, E8의 음수 크기가 화면에서 어떻게 보이는지는 미확인이다.
- 블록 1, 2를 반영하지 않았다. 원본 화면에 닻이 더 진하거나 크게 보이면 블록 2의 2.0 값과 함께 이 부분이 첫 후보이다.
- 마커 크기는 섬 스케일 2.0을 곱하지 않는다(원본 100%). 섬에 비해 작게 보이면 섬 스케일 기준의 조정이 필요하다.
- 항구용 표현은 어디에도 연결하지 않았다. 항구 쪽 입항 구역에 쓸지는 원본에서 그 자리에 무엇이 그려지는지 확인한 뒤 판단한다.
