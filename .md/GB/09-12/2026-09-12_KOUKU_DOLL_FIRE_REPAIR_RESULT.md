# 괴기스러운 인형·외곽불 교정 결과

## G00. 현재 상태

인형과 외곽불 D/E/F의 교정 후보 및 CPU 검증을 완료했다. 실행 중인 기존 Client가 읽는 Resources에는 아직 설치하지 않았다. 화면 복원 완료나 GPU 표시 성공을 의미하지 않는다.

## G01. 인형 원본 회전

`world.object.kouku.odd_doll`의 `Character/KoukuSaton/MN_CDMD_00/MN_CDMD_00.wmodel`은 기존 FLOAT weight 교정과 30Hz 시간 교정이 이미 들어 있었다. 문제는 09-11 말·호랑이에서 확인한 자식 본 quaternion conjugate 누락과 같은 종류였다.

실제 원본 UPK의 `mn_cdmd_00_ani` PSA와 `mn_cdmd_00_sk_loc_int` glTF를 UModel의 파일 export로 추출했다. 기존 `build_umodel_gltf_psa.py`와 ModelAssetConverter로 다시 cook하고 시간 단위를 30Hz로 맞췄다. 새 cook 전체를 설치하지 않고 기존 WModel의 WANM 회전 결과만 교체해 geometry·skeleton·material을 byte 단위로 보존했다.

| 검사 | 결과 |
|---|---|
| 모델 | 9,301 정점 / 58 노드 / 30 clip |
| 원본 PSA 회전 대조 | 103,056 표본, 최대 성분 오차 약 1.414 → 0.000005642 이하 |
| 기존 계약 보존 | 모든 clip 이름·길이·30Hz·position/scale/key time 동일 |
| CPU skinning | clip별 9개 시점, 전체 정점 finite |
| geometry / skeleton / material | 설치본과 byte 동일 |

후보는 `out/KoukuDollFire20260912/candidates/MN_CDMD_00.wmodel`, 원본 백업은 같은 작업 폴더의 `backup/MN_CDMD_00.wmodel`이다. 검사 근거는 `doll_validation.json`, `doll_validation.log`, `validate_doll.py`에 있다. 기존 body preScale 0.01과 사용자가 편집한 Object transform은 바꾸지 않았다.

## G02. 외곽불 D/E/F

설치된 정적 WModel은 WINT 1.2이며 WMA2 slot 0의 발광 경로에 반사광 텍스처 `textures/b2378a8f80d6_t_tds_specular04.dds`가 들어 있었다. 기존 `repair_gate3_fire_materials.py`가 WINT 1.0만 받아 이 설치본을 거부했다.

도구가 구조가 동일한 WINT 1.1/1.2를 받도록 확장하고, WMOD·WMA2 및 정확한 material/diffuse/emissive identity 검사를 유지했다. apply는 임시 파일·입력 재확인·원자 교체를 사용하며 부분 실패 때 원본을 복구한다. 사용자가 바꾼 geometry·UV·배치·크기는 변경하지 않는다.

실제 도구를 `out/KoukuDollFire20260912/fire_fixture` 복사본에 적용해 세 파일 교정, 재실행 시 0개 변경, 미지원 WINT 1.99 거부를 확인했다. 각 파일은 지정된 520-byte emissive 필드 안에서만 달라졌다. `fire_validation.json`에 원본/후보 SHA와 미설치 상태를 기록했다. 이 교정은 잘못된 발광 입력을 제거한 것이며 원본 불 재질의 전체 복원으로 기록하지 않는다.

## G03. 다음 설치와 사용자 확인

Client 종료 후 원본 SHA가 검사 당시와 같은지 다시 확인하고 네 후보를 설치해야 한다. 현재 사용자 실행 파일과 Resources는 유지한다. 이후 인형 preview의 머리·다리, 외곽불 색은 사용자가 직접 확인한다. Object Tool의 개수·간격과 Box Detail의 정확한 Object/Motion 열기는 별도 `WORLD_OBJECT_GROUP_EDIT_AND_PREWARM_RESULT.md`에 기록했다.
