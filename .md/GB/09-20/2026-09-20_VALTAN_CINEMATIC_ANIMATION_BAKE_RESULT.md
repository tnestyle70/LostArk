# 발탄 원본 연출 애니메이션 병합 결과

`Tools/ValtanPipeline/bake_valtan_original_cinematic_actors.py`로 일반·유령 발탄 각각의 기존 AnimSet에 `valtan.cinematic.entrance`, `valtan.cinematic.trash`, `valtan.cinematic.finale`을 추가했다. 각 원본의 146개 애니메이션과 총 149개 기존 섹션은 바이트 단위로 보존했다. 후보와 hash·원본 참조는 `out/ValtanCinematicBake20260920/candidates/receipt.json`에 있다. 설치와 BossCatalog·WorldSequence 연결은 같은 작업의 발탄 담당자에게 전달했다.

원본 EFGame 패키지의 EFSkeletalMeshActorLookInfoMAT CDO 12363 → SkeletalMeshComponent 12365 → AnimTree 13838을 실제 파싱해 `AnimBlendingMAT.AnimBlending_mix` 상속을 확인했다. A-over-B 시간별 곡선, clip trim/rate/loop/reverse를 적용했고, 사망 UP_A는 원본 spine1 하위 58개 본만 섞는다. H_Up은 원본 head/neck 부모 본 공간의 가산 회전과 0→0.5→0 강도 곡선을 적용했다. UP_B 강도는 0이며 기존 하체 pose가 보존되는 것을 확인했다. 입장 actor 65와 무채색 actor 66의 clip 및 blend curve가 동일하므로 입장 clip을 공유한다.

실제 WARP CModel에 각 발탄 본체와 후보 AnimSet을 생성·Attach하고 여섯 연출의 모든 30fps 샘플에서 root/spine/head/weapon 행렬을 검증했다. `model_probe.log`: 208,256개 유한 수치 검사 PASS, 창 0개, Draw 0회. 길이는 입장 24.707817초, 버러지들 약 6.37448초, 사망 23초다. 원본 프레임 사이에는 기존 CModel 보간을 사용한다. 화면상 최종 동작·카메라와의 합성은 사용자 검증 범위이며 실행하지 않았다.

연출 actor의 월드 이동과 가시성·이펙트·카메라·재질 트랙은 별도 기존 WorldSequence 데이터가 소유한다. 이 도구의 검증은 본 애니메이션과 기존 clip 보존이며 해당 트랙이나 화면 검증을 대신하지 않는다.
