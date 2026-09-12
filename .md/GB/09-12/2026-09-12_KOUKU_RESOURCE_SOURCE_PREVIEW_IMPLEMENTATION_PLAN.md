# 쿠크 독립 Effect Resource의 원본 모델 시계 연결 구현 계획

## G00. 실제 오류와 소유자

사용자 첨부 오류는 `preview.resource.occurrence`의 외부 transform-history 재생 중
`Kouku source attachment has no animation at its requested time`으로 발생했다.
`MainApp.cpp`는 단일 Resource Preview에 Animation 없는 Stage를 만들지만,
`KoukuSaydonPresentationPlayer.cpp`의 source sampler는 그 Stage만 읽는다.
쇼타임 총구·총연기·탄피·서명·종료·바닥 Effect에는 이미 `sourceModelPreview`가 있으므로
이 오류를 texture나 shader 리소스 누락으로 처리하지 않는다.

## G01. 기존 CNpc/CModel과 단일 Resource Preview 연결

PresentationPlayer는 단일 V1 Resource의 원본 모델 metadata를 읽고, 기존
`Begin_BundlePreview`의 단일 member로 actor·Animation·Effect를 함께 준비한다.
추가 actor runtime이나 새 Level은 만들지 않으며 공개 preview ID와 0ms 원점은 유지한다.
원본 sourceModelPreview가 없던 기존 네 Effect에는 생성기가 사용하는 실제 Action/Stage의
clip 이름과 길이로 이 metadata만 추가한다. 기존 occurrence와 SourceRecipe는 보존한다.
G1 내려치기C·불뿜기와 G2 거미카운터 두 구간이 대상이며 G2 반복 호출도 같은 source Stage다.
그 밖의 metadata 없는 독립 bone Effect는 실제 선택 모델의 현재 pose를 한 번 캡처한다.
이 정지 pose 정책은 독립 Resource Preview에만 적용하며 Product의 과거 Animation 누락은 계속 오류다.

Effect lifetime은 기존 occurrence 창을 유지하고, 마지막 원본 pose는 입자 tail 동안 유지한다.
SourceModelPreview의 sourceStart·playMs·rate와 Pattern의 source range·blend는 같은 CModel
sampler가 소비한다. 모르는 actor·bone·clip을 root나 다른 클립으로 대체하지 않는다.
MainApp은 이 actor 소유 Preview에 중복 World Preview를 시작하지 않는다.

## G02. 검증과 기존 변경 보존

소유 파일은 PresentationPlayer CPP와 MainApp의 단일 Resource Preview 구간,
기존 G1 전체 복원·거미카운터 생성기와 이들이 소유하는 네 Effect JSON이다.
기존 Parent·clip range·terminal WORLD failure 변경을 보존한다. 파일별 UTF-8/CRLF를 유지하고
기존 bytes는 Git 제외 out에 보관했다. 새 C++ 파일과 project/filter 등록은 없다.

실제 Effect Codec, 설치 WModel과 CModel의 CPU bone sample로 총구를 포함한 source actor/clip
일치, 0ms·중간·마지막 tail·rewind 입력과 finite matrix를 확인한다. 수정 CPP 최소 컴파일과
git diff --check를 수행한다. Client/UI 실행·조작·캡처와 최종 시각 판정은 사용자가 담당한다.
