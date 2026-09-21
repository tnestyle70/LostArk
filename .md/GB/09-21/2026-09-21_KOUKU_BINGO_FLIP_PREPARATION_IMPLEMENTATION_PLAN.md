# 쿠크 빙고 flip 문서 준비 비용 제거

## G01. 실제 호출과 변경 범위

`CKoukuSaydonPresentationPlayer::Update_BingoMarks`는 새 칸과 색 변경마다 새 `CWorldSequencePlayer`에 작은 해골 문서를 설정한다. 하지만 `Set_DocumentBatch`가 매번 전체 맵 placement를 읽고 Deploy의 animation clip 목록을 복사한다. Object resource만 참조하는 문서의 실제 validator는 이 두 목록을 소비하지 않는다.

`WorldSequencePlayer_Objects.cpp`의 batch admission은 어떤 binding이라도 Object resource 이외의 target을 참조할 때만 기존 target 수집을 수행한다. Object 전용 문서도 실제 `Validate`와 stage/commit을 거치며 잘못된 문서를 허용하지 않는다. 일반 Map/Deploy의 admission은 유지한다.

`KoukuSaydonPresentationPlayer.cpp`의 색 변경은 revision이 같은 기존 player에서 새 모션을 `Play`하고 `Seek_InstanceToMs(0)`의 실제 첫 샘플 성공을 확인한 뒤 이전 instance를 정리한다. 실패하면 새 instance만 정리하고 이전 칸과 재시도 정책을 유지한다. 문서와 준비한 모델을 다시 만들지 않는다. revision 변경은 기존 replacement 경로로 최신 문서를 받는다. 실패 재시도를 기다리는 동안 이전 칸의 Update를 계속한다.

`Apply_ObjectMotion`은 이전 effect tail을 보존하므로 이 경로에서는 사용하지 않는다. 빙고 유지 효과의 duration은 300초여서 이 정책을 그대로 적용하면 이전 색의 효과가 함께 유지된다. 현재 cell pivot, Server mask 권위, 새 white의 flip→white와 white→red의 red_flip→red 모션은 보존한다.

## G02. 반영 및 검증

기준 파일과 SHA를 `out/KoukuBingoFlipPreparation20260921`에 보관하고 기존 UTF-8 BOM 없음·CRLF 및 다른 변경을 보존한다. 기존 두 CPP만 변경하며 새 제품 파일·프로젝트 등록은 없다.

두 전체 TU의 scratch 컴파일과 실제 변경 함수 native adapter를 통해 Object-only admission의 target 수집 생략, Map/Deploy 수집 유지, 잘못된 문서 rollback, 25칸 색 전환 시 player/document 재사용, 첫 샘플 실패 시 기존 칸 유지, 제거/Reset 수명과 retry 경계를 검사한다. 문서 Validate는 실제 C++ codec으로 수행한다. GPU 화면과 실제 frame time은 이 headless 검증의 완료 주장이 아니며 사용자 화면 검증을 남긴다. Product build/publish와 Data는 부모 작업자가 소유한다.
