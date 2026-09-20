# 마리오 NPC 광대 망치 마무리

대상은 플레이어 광대가 아닌 Mario 원본 MN_REUP_05 NPC이다. native action4221701의 att_battle_1_01은1.366667초이며 source 마지막 swipe cue가 .724025초다. 사용자가 요청한 clip끝으로 시작시점만 옮기고 원본 두mesh emitter, notify의TRS/parameter와 이미 설치된 native2936 shader를 보존한다.

기존 WorldObject에 마리오_광대 parent와 idle/attack두motion을 만들고 attack의MOTION_END에 같은Effect를 연결한다. 실제 Server-spawn NPC는 MonsterCatalog attack의 optional endEffectAssetId를 CNpc에게 전달하여 실제 admission clip길이에서1회 생성한다. 기존 action전환은예약을해제하고 level-owned 자연수명Effect를 사용한다. 기존Kouku Loader target queue에입장전준비한다.

새C++파일없음. 새DataEffect문서는96.DataFiles등록. 최신savedWorldSeq/EffectCatalog/MonsterCatalog의독립행/필드를hash확인·backup·atomicmerge한다. Client/UI실행하지않고CModel/Codec/변경TU컴파일및WorldSequence실제parser로검증한다.
