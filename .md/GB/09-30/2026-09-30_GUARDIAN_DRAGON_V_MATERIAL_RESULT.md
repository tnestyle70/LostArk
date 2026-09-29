# Guardian 광포화 S 용의 V 재질 연결 결과

## G00. 이전 수정과 현재 원인

광포화 S49230은 DRAGON binding의 ddk_sk_dragonicresonance_02와 animevent를 거쳐 역사적 이름인 effect.guardianknight.skill.49220.source.49290.clip.1.full.restore를 사용한다. 실제 용은 guardian.49290.1.sk_ddk_drr_01_head_st_mi와 neck_st_mi 두 ModelCue다. 76개 particle element 또는 별도 runtimeCarriers를 수정하는 경로가 아니다.

09-22 수정의 shader110/111은 이 두 용 재질에 정확히 연결돼 있었다. engine BRDF lookup의 0분모 방어와 environment CB24/25 연결은 현재 분할된 Shader_SourceCharacterBaseGroup108.hlsli 및 generator에도 남아 있다. 이전 수정이 배포되지 않았거나 다른 element에 적용된 것이 아니다. 기존 두 descriptor가 파란 rim [0.1,0.2,2,1]·intensity30과 파란 transcolor [0.2,1,7,1]을 계속 사용해 V와 다른 색을 내도록 저작돼 있었다.

## G01. 실제 변경

현재 저장본의 해당 두 stable cue에서 sourceMaterialProfile과 materialParameterTracks만 변경했다. V49400 clip1 head/neck descriptor의 SOURCE_CHARACTER107/108, texture, 색·발광·표면 계산을 사용한다. materialName은 실제 S WModel의 sk_ddk_drr_01_head_st_mi와 neck_st_mi slot을 유지했다. 따라서 원래 S geometry·14개 bone·animation·play rate·delay·duration·transform과 다른 particle76개는 보존됐다.

S dead 곡선은 그대로 두었다. V의 transcolor/emissive_intensity 곡선은 V1.533332944초를 S1.100000024초에 대응해 같은 정규화 나이의 값을 소비한다. V family가 선언하지 않는 기존 opacity_intensity 곡선은 제거했다. 공용 shader, 전체 asset 회전·색, C++ 제품 코드는 바꾸지 않았다.

Effect_DocumentCodec의 SourceCharacter::Read와 material profile 검증을 거쳐 ResourceStaging의 Build_MaterialOverride가 실제 CModel에 전달된다. Catalog는 cue별 Create_MaterialVariant를 만들고 Rendering은 매 프레임 materialParameterTracks를 sample하여 named slot의 Override_SourceCharacterConstants로 반영한다. family 변경만 하고 slot 이름을 V로 바꾸면 S mesh가 소비하지 못하므로 이 경계를 실제 모델로 검증했다.

## G02. 저장과 Resources 전달

승인된 현재 디스크 원본과 V reference hash를 교체 직전 확인하고 backup 및 원자적 교체로 반영했다. S source SHA256은 05dbcb0fc6e4890c0c0c1dfd34cedbc41fa525bdf8ffd9ca8162700180aae020에서 ab338be199aedaeced5946f69f2e5bcece559268a5decfb05d3df11f8da881e7로 바뀌었다. V reference는 cc7deffda5cc109ba8783e5aa798163a1ed8fdb91b7a9e5dc2c8653e179e052d를 보존했다.

Client Resources의 기존 S 모델2개, V texture12개와 S WModel의 기본 material texture6개, 총20개27,229,408바이트를 GBResources의 같은 상대 경로로 복사했다. SHA256은 전부 일치했다. Client Resources에 새 파일을 생성한 것은 아니다. packed WModel에서 문자열 참조를 회수했다고 주장하지 않으며 기본 texture 디렉터리의 closure를 포함하고 실제 GBResources만 사용한 CModel 입장으로 검증했다. 전체 경로·크기·hash는 out/GuardianS20260930/installation-receipt.json에 있다. 사전 inventory가 있는14개는 GBResources에 없었으므로 신규 복사가 확정된다. 나중에 보완한 기본 texture6개는 복사·동일 hash를 확인했지만 사전 존재 상태를 기록하지 않았다. 파일 생성 시각은 이번 복사 시점과 일치하나 이를 사전 inventory 대신 사용해20개 모두 신규였다고 단정하지 않는다.

Data/Effects/EffectCatalog 및 Authored는 직접 제품 입력이므로 별도 Effect publish나 runtime 복사본 생성은 필요 없다. 최종 제품 패키지에는 이 JSON과 Resources closure가 필요하다.

## G03. 실행 검증과 한계

실제 제품의 SourceCharacterMaterialParameters, WorldSequenceDocument, DataJson과 Engine CModel을 링크한 console probe를 실행했다. D3D11 WARP 소프트웨어 장치를 사용했으며 Client 또는 UI 창을 실행하지 않았다. installed S WModel 두 개와 각각14개 bone·한 개 material slot을 로드했다. program110/111에서107/108로 전환되는 material override, 실제 variant 격리, 상수 적용과 원복을 확인했다. 23개 시간 표본×2개 모델의 상수 packet은 보존한 S dead 값을 제외한 V 재질의 정규화 sample과 일치했다. Client Resources와 GBResources 경로에서 각각 models2/slots2/samples46 PASS다.

로그는 out/GuardianS20260930/probe.log와 probe-gbresources.log다. build-probe.log 및 build-probe-relink.log는 exit0이며 기존 Engine header C4828 경고가 남는다. JSON parse·공식 validator의 해당 Guardian 문서6개 scoped 검사·candidate 필드 일치·git diff --check는 PASS다. scoped-validation.json에 증거를 기록했다.

전체 Validate-EffectSources.ps1은 기존 HEAD와 의미상 동일한 effect.valtan.action.420602.stage001.full.restore의 valtan.trail.7fcde5bbca0fbc103367e216에서 baked history clamp/sample closure 오류로 중단됐다. 같은 HEAD 데이터에도 동일 오류가 발생함을 별도로 확인했다. 이 실패를 Guardian 성공으로 숨기거나 무관한 발탄 데이터를 변경하지 않았다.

이 검증은 실제 CModel 및 CPU 상수 소비·Resources 입장까지 확인한다. 실제 GPU shader draw의 색·용 모양·사용자 화면 판정을 대신하지 않는다. 최종 광포화 S와 V의 화면 비교는 사용자가 확인한다.
