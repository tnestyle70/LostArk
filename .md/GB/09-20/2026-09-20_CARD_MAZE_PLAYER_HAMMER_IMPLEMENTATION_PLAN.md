# 카드미로 플레이어 뿅망치와 두 입력

카드미로는 Server가 정상 class 몸체를 유지하지만 Client의 interaction binding 로더는 광대만 허용했다. 그래서 Q의 Server 판정은 존재해도 손의 기본 무기와 정상 class 애니메이션은 바뀌지 않았다.

1. 설치 게임의 각 class 기본 AnimSet에서 `pr_it_gstfp_00_att_1_01`(1초)과 `pr_it_gstfp_00_att_2_01`(2.5초)을 추출한다. 기존 WModel 골격과 armature scale을 보존한 별도 animation set으로 연결한다. 기존 body 파일은 수정하지 않는다.
2. 정상 class의 MAZE binding을 읽고 Server snapshot의 MAZE 진입/이탈에 따라 기존 WorldObject의 WhirlwindHammer 모델을 기존 CModel/Part_Equipment 경로로 본에 부착하고 기본 무기를 복구한다.
3. Q는 기존 interaction index 0, LMB는 기존 wire W의 index 1을 사용한다. 새 packet/로컬 damage 경로를 만들지 않는다. 두 action의 길이와 타격 시점을 Server가 소비한다.
4. All Effects에 MAZE LMB/Q 저작 문서를 열고 class clip과 함께 미리보기할 수 있는 행을 추가한다. 사용자가 이펙트를 붙일 빈 저작 문서를 제공하며 임의 이펙트는 만들지 않는다.
5. 현재 인코딩/미커밋 변경을 보존한다. 변경 TU 컴파일, JSON parse, 실제 native bone/clip 비교와 기존 Server 카드미로 계약 검증으로 확인한다. Client/UI/GPU 확인은 실행하지 않는다.

새 C++ 파일은 추가하지 않으므로 vcxproj/filter 등록은 필요하지 않다. 신규 Data 문서는 Client 96.DataFiles에 등록하고 Resources 파생 모델은 산출물로 분리한다.

플레이어 손의 별도 Pos(cm)/Rotation(deg)/축별 Size는 ArenaCameraProfile optional 필드로 저장한다. 기존 맵별 Save/Reload와 freshness 검증을 유지하고 기본값 0/0/1 위에 조절한다. 쿠크 보스 휠윈드의 사용자 screenshot 1.25/0/0,0/180/0,2배 수치는 이 필드에 복제하지 않는다.
