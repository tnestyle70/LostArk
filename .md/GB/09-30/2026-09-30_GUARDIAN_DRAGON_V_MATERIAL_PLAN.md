# Guardian 광포화 S 용의 V 재질 연결 계획

## G00. 현재 소비자와 원인

DRAGON S49230은 dragonicresonance_02 clip의 animevent를 통해 이름에 49220이 남은 source49290 clip1 문서를 사용한다. 실제 용은 guardian.49290.1.sk_ddk_drr_01_head_st_mi와 neck_st_mi 두 ModelCue이며 particle element가 아니다. 기존 110/111 shader의 engine BRDF 0분모 방어와 scene 환경 입력 수정은 현재 분할된 Group108에도 남아 있다. 이전 수정이 다른 element에 적용되거나 skill별 shader가 분리되지 않아 사라진 것이 아니다. 두 cue의 파란 rim 30배와 파란 transcolor 곡선이 유지되어 있는 것이 현재 V와 다른 원인이다.

## G01. 교체 범위

Data/Effects/Authored/effect.guardianknight.skill.49220.source.49290.clip.1.full.restore.effect.json의 두 stable cue에서 sourceMaterialProfile과 materialParameterTracks만 교체한다. V49400 clip1의 대응 head/neck descriptor를 복사해 SOURCE_CHARACTER107/108의 텍스처·색·발광·표면 계산을 사용한다. materialName은 S WModel의 원래 slot 이름을 유지한다. S 모델·animation·시간·local/asset transform·다른 particle과 runtimeExtensions를 보존한다.

S dead 곡선은 그대로 유지한다. V의 emissive_intensity와 transcolor 곡선은 같은 정규화 나이에 같은 V 값을 갖도록 1.533332944초에서 S1.100000024초로 시간만 대응한다. V 재질이 소비하지 않는 opacity_intensity 곡선은 제거한다. 별도 shader·renderer·C++ 파일은 필요 없고 project/filter 등록도 추가하지 않는다.

## G02. 검증과 반영

후보를 먼저 out/GuardianS20260930에 준비한다. 실제 SourceCharacterMaterial::Configure, CWorldSequenceDocument::Build_MaterialOverride, CModel의 material variant/상수 변경 소비를 headless probe로 확인한다. 참조된 모델·텍스처의 설치 및 GBResources 상대 경로와 SHA256을 검증한다. 승인된 현재 저장본을 최종 reread하여 hash 일치·backup·원자적 교체를 유지한다. 최종 publisher와 Product 빌드는 통합 담당자가 수행하며 Client/UI와 사용자 화면 승인을 대신하지 않는다.
