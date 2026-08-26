# 베른성 회색 가림 메시 수정 계획

## 목표

Lobby -> Character Select -> Bern 진입 후 이동할 때 화면 전체를 가리는 회색 메시를
원본 데이터와 런타임 산출물 기준으로 제거한다. 기존 내비게이션 높이 보정은 유지하되,
이번 결함을 카메라나 내비게이션 문제로 오인하지 않는다.

## 확인된 원인

- `MAP_AAB9BAA9D104_SQG_TR01_SM`, `MAP_C81AD3A280F5_DM_FAT_TREE_B04_SM`의
  원본 material instance에는 `a_texture_diffuse`, `r_texture_diffuse`,
  `r_texture_normal`이 있으나 Bern asset pipeline이 채널 접두사를 인식하지 못한다.
- 두 모델은 diffuse/emissive가 빈 WModel로 조리되어 `CMaterial`의 불투명 회색 fallback을
  사용한다.
- `MAP_5597D1E5C47F_LV_LUT_CRSEASHO_COMMON03_SM`은 18정점 특수 표현 메시이며,
  exact export에도 `dummy_material_0`만 있고 복원 가능한 texture parameter가 없다.
  이 asset의 14개 대형 배치가 시작 지점과 이동 경로에서 회색 가림막으로 보인다.

## 구현

1. Bern asset pipeline에 vertex-blend channel texture parameter의 명시적 우선순위를 추가한다.
2. 나무·절벽·원경 산·눈 장식 중 exact texture parameter가 확인된 asset pack을 다시
   추출·조리하고 Bern runtime Resources에 배포한다.
3. render profile authoring의 source placement visibility override가 shard 재생성에서 보존되도록
   builder를 수정한다.
4. exact export에도 material parameter가 없는 특수 해안/장식 메시 placement만
   `visible=false`로 저작한다.
5. unit test, shard build, Map publish/validation, Client Debug build를 실행한다.

## 완료 경계

자동 검증은 데이터 계약과 빌드까지다. 최종 화면에서 회색 면이 사라졌는지는 사용자가
Server + Client로 같은 이동 경로를 반복해 확인한다.
