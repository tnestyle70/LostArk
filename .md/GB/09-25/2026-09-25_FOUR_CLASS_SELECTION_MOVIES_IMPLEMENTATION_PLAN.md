# 네 클래스 선택 무비 복구 구현 계획

## G00. 기존 Play 수정과 작업 경계

사용자는 중단된 `Fix play class selection`의 재생 실패를 전부 고친 뒤 창술사·워로드·도화가·
차원술사 무비를 복구하고 추가 Resources를 `C:/Users/user/Desktop/GBResources`에 모으도록 요청했다.
현재 branch는 `codex/kouku-timeline-local-preview`, HEAD는
`1ddbcffc016144edb8c8120c4164f68668fdec40`이며 무관한 미커밋 변경을 보존한다.

기존 Play 작업은 선택한 category의 class ID를 F1/WORLD 공통 entry로 전달하고 실제 배경·배우·
FX·camera Initialize/Play/Update/Stop/rePlay 소비자를 검증한다. 기존 C++ 변경은 현재 저장본을
기준으로 통합한다. 네 클래스의 원본 조사·도구·데이터·리소스는
이 계획에서 소유한다. Client/UI 자동 실행이나 화면 판정은 하지 않는다.

## G01. 원본 클래스와 무대

정본은 현재 retail `LV_LOBBY_CLASSSELECT_SCENE01`의 ChangeClass와 Matinee 연결이다.
공통 원본 package SHA256은
`69538e81c4a1566795799fc8a5348d1bb0561217db935f0ac76b1d4bab0cc748`이다.
아래 Matinee 값은 zero-based export index다.

| 선택 class | 원본 category | intro / loop | 배경 Area |
|---|---|---|---|
| WARLORD | Warrior |689 / 688|LV_LOBBY_CLASSSELECT_SL01|
| LANCE_MASTER | Fighter |690 / 691|LV_LOBBY_CLASSSELECT_SL03|
| ARTIST | Specialist |699 / 692|LV_LOBBY_CLASSSELECT_SL08|
| DIMENSIONMASTER | Magician Male |700 / 698|LV_LOBBY_CLASSSELECT_SL12|

현재 플레이 캐릭터 외형으로 원본 컷신 배우를 대체하지 않는다. 원본 actor/component의 mesh,
MIC, AnimTree/AnimControl, attachment와 world transform을 사용한다. SL00의 사용자가 옮긴
11개 바닥과 Server player는 그대로 두고 camera가 해당 원본 무대를 관람한다.
배경 선택 근거는 실제 source actor·mesh WORLD 위치이며, 차원술사는 explicit streaming
참조도 함께 대조한다. 공간 대조와 명시적인 graph 참조의 증거 수준을 구분한다.

## G02. 기존 투영기와 bone bake의 일반화

`Tools/CharacterSelectPipeline/project_guardian_selection.py`와
`bake_guardian_selection.py`의 기존 Matinee 평가·camera·clock·WorldSequence·WANM 경로를
재사용한다. Guardian 전용 actor 번호와 clip prefix, intro 판별을 검증된 class profile로
분리하며 기존 Guardian 기본 동작과 원본 보간·정지 포즈를 보존한다.

실제 첫 source 검사에서 확인한 Warrior/Fighter의 빈 FOV placeholder와 실제 FOV track 공존,
Artist의 구간 끝 중복 Slomo 시각을 원본 값대로 처리한다. 없는 animation clip을 다른 class clip으로 대체하거나
소스의0배속·역재생·blend·SkelControl을 전체 고정 속도로 바꾸지 않는다. 파생 donor는 원본
geometry/material/skeleton을 보존하며 같은 packed-float run 내부만 줄여 WANM 상한을 지킨다.

## G03. 배우·배경·이펙트 리소스

클래스별 actor closure와 재질 slot을 기준으로 기존 설치 asset을 재사용하고 누락된 모델·DDS·
애니메이션·소리만 원본에서 추출한다. 독립 particle은 실제 template/redirector·emitter/module·
native material/carrier를 사용하며 해당 원본의 활성/숨김·별도 particle 시계를 유지한다.
부착된 소품과 움직이는 PSC는 실제 owner 변환을 소비하며 정적 root로 위장하지 않는다.

배경은 기존 LevelPlacementExtractor/ModelAssetConverter/source material compiler의
actor/component 가시성과 UV·정점색·RNM·환경 절차로 준비한다. 미지원 재질을 검정 fallback으로
숨기거나 resource 존재만으로 준비 완료를 기록하지 않는다. 기존 CModel→CMaterial 및
WorldSequence/Effect presentation 소비자를 사용한다.

## G04. scene별 배경 계약

기존 Play 소유 작업이 `scenes[].backgroundAreaId` optional Area ID를 연결한다.
기존 Guardian의 생략값은 registry SL10을 사용한다. Loader는 manifest의 고유 background를
기존 scope로 준비하고 Level은 Area별 CMapPlacementRuntime과 실패 상태를 소유한다.
현재 활성 class의 배경만 표시하고 Stop·교체·Level 이탈 시 자기 상태를 정리한다.
한 배경 실패 때문에 다른 class의 Play까지 막지 않는다. 별도 map/runtime 경로를 만들지 않는다.

## G05. 설치와 게시

후보는 out에서 준비·검증한다. 추가 Resource 전부를 GBResources의 기존 Resources-relative
하위 경로에 제공하고 실제 재생 경로에도 같은 파일을 기존 dual-root 설치 절차로 동기화한다.
기존 다른 내용의 파일을 자동 교체하지 않고 충돌을 보고한다. 원본 UPK/LPK와 조사 캐시는
Resources에 넣지 않는다. 바이너리는 Git index에 추가하지 않는다.

Data/Camera manifest, SL00 WorldSequences, EffectCatalog, 필요한 MapCatalog와 Area 정본은
최신 디스크의 stable ID 기준으로 병합한다. writer lock·hash 재확인·백업·원자 교체와 자기 변경
rollback을 유지한다. WorldSequences와 각 배경 Area는 기존 Map publisher로 게시하고 Check한다.
추가 Data는 Client96.DataFiles None 항목으로만 등록한다.

## G06. 완료 증거

각 class의 원본 Matinee·actor/clip/material/FX/camera 연결, 전체 intro→loop의 finite 변환과
시간 경계, 실제 CModel Create/Attach/Clone, WorldSequence/Effect 준비 및 camera 소유권을
검사한다. Play/Stop/rePlay와 class 교체·실패 시 기존 정상 상태 보존을 현재 소비자로 검증한다.
필요한 최소 C++/shader 컴파일과 정상 증분 Product Build, JSON/XML parse, publisher Check 및
git diff --check를 수행한다. 사용자 화면과 원작 시각 동등성은 자동 검증으로 대신하지 않는다.
RESULT에는 실제 설치·게시·빌드·검증과 남은 사용자 확인을 분리한다.

## G07. 다섯 클래스 공통 재생과 Movie Timeline

Guardian을 포함해 다섯 category만 F1/WORLD 공통 option으로 구성한다. 원래 SL00 placement
목록과 별개로 category를 필터링하며, 새 Engine timer 대신 presentation의 elapsed movie 시간을
owner로 유지한다. source clock은 원본 slomo mapping, effect particle age는 그 source 시각을
입력으로 받는 원본 PSC mapping이다. 0.05..2배속은 하나의 movie 진행량에만 적용한다.

`ClassSelectionTimeline.h`의 row/box는 loaded scene과 실제 WorldSequence document를 투영한다.
id/label/resource는 stable 식별·표시·원본 연결이며, movie/source 시작·끝과 keyMovieTimes의 단위는
ms다. clip rate/offset과 optional camera row를 함께 보유한다. camera sample은 실제 제출 성공
시각, 보간 pose, up, aspect를 보존한다. cache는 presentation Clear에서 정리한다.

`ClassSelectionPresentation::Get_Timeline`→Workbench callback→row 선택→Box Detail 흐름으로
읽으며 `Set_PlaybackRate`/Seek/Pause 명령만 같은 owner에 제출한다. shader·Server timer를
Workbench가 직접 조작하지 않는다. Camera Detail은 저장 key와 실제 보간·FOV 변환된 camera를
구분하고 선택 row가 현재 active camera일 때만 비교 sample을 표시한다.

추가 헤더는 Client.vcxproj와 Client.vcxproj.filters에 등록한다. 별도 CPP나 두 번째 재생기는
추가하지 않는다. 신규 헤더 전문은 다음과 같다.

```cpp
#pragma once

#include "EffectRecoveryCamera.h"
#include <memory>
#include <optional>

namespace Client
{
// Read-only projection of the admitted movie. Movie milliseconds include the
// source time-dilation curve; source milliseconds address the original tracks.
struct CLASS_MOVIE_TIMELINE_BOX final
{
    std::string id, label, resource;
    double movieStartMs = 0., movieEndMs = 0., sourceStartMs = 0., sourceEndMs = 0.;
    double playbackRate = 1., sourceOffsetMs = 0.;
    std::vector<double> keyMovieTimes;
    std::optional<EFFECT_CAMERA_ROW> camera;
};
struct CLASS_MOVIE_TIMELINE_ROW final
{
    std::string kind, id, label;
    std::vector<CLASS_MOVIE_TIMELINE_BOX> boxes;
};
struct CLASS_MOVIE_TIMELINE final
{
    std::string classId;
    bool loop = false;
    double movieDurationMs = 0., sourceDurationMs = 0.;
    std::vector<CLASS_MOVIE_TIMELINE_ROW> rows;
};
struct CLASS_MOVIE_CAMERA_SAMPLE final
{
    bool valid = false;
    std::string rowId;
    double movieMs = 0., sourceMs = 0.;
    float aspect = 1.f;
    VALTAN_CINEMATIC_CAMERA_POSE pose;
    float3_t up{0.f, 1.f, 0.f};
};
}
```

## G08. 실제 admission과 원본 reflection

미완전 전방 선언 class의 멤버 함수 포인터를 public registry layout에 넣지 않는다.
LOAD_FUNCTION은 HRESULT(*)(CLoader&)와 captureless wrapper로 고정한다.
배경 native RNM admission은 CModel/MapCatalog/publisher의 동일 program 범위를 유지한다.

WorldSequence key는 원본 reflection을 위해 음수 nonzero scale을 허용하되 각 축의 부호가
track에서 바뀌면 거부한다. 음수 multiplier는 Object Resource binding에 한정한다. 모델 draw는
음수 world determinant에서 기존 cull winding만 반전하고 원래 rasterizer를 복원한다.
원본 LookInfo override는 실제 native packer의 parameter 집합에만 적용한다. publisher는
C++ octal UTF-8 이름을 decode한 뒤 정확한 집합과 texture mask를 검사한다.

## G09. 원본 파티클의 선택 타일과 충돌 완료 동작

기존 portable carrier에 SubUVSelect의 상대 수명 기반 X/Y 정수 타일 선택을 연결한다.
분포는 3성분을 요구하고 Required의 atlas 크기를 검증한다. FreezeRotation은 충돌 완료 후
추가 충돌과 회전만 멈추며 이동, 색, 크기와 수명은 계속 평가한다. 기존 FreezeMovement와
Kill 동작은 보존한다. 실제 무비 60개 Effect 준비 및 다섯 Play 경로로 검증한다.

원본 Cascade의 수신자 없는 event generator는 bounded queue에 남는 무효 이벤트로 보존한다.
연결된 route만 순환 검사를 하고 queue 상한과 event identity 검증은 유지한다. codec와 runtime은
mesh sourceMaterialSlots를 포함한 동일 element 실행 판정을 사용한다.

## G10. 실제 재질 draw 실패와 끝까지 재생

사용자 화면에서 Guardian의 `guardianknight.intro.a12265.p0`는 시작 직후,
DimensionMaster의 `dimensionmaster.intro.a740.p0`는 약4.67초에 mesh0 material binding으로
중단됐다. 실제 target은 skinned SOURCE_CHARACTER의 Guardian 얼굴 native200과
DimensionMaster 무기 native902다. WorldSequenceObject의 재질 binder에서 legacy 입력과
native 입력을 분리해 실패 지점을 확인하고 기존 CModel/CMaterial 경로를 수정한다.
실패를 무시하거나 actor를 숨겨 무비 완료로 처리하지 않는다.

설치 Engine의 실제 native clone은 256개까지 성공하고 257번째가 E_BOUNDS로 실패했다.
CMaterial frame registry의 임의 256 상한을 Target_Depth.z의 R32 float 정확정수 계약에
맞추고 pointer→row 조회를 같은 frame owner에 유지한다. frame reset은 vector와 lookup을
함께 비운다. WorldSequenceObject는 실패 HRESULT·실제 material/program을 보존해 원인을
표시한다. 셰이더의 재질식과 사용자 조명·후처리 설정은 바꾸지 않는다.

기존 headless fixture의 pose/particle sampling은 실제 World Object draw의 성공 증거가
아니다. 같은 실제 player·모델·설치 shader로 활성 WORLD 객체 Render와 render status를
검사한다. 시작·4.67초뿐 아니라 intro 전체와 두 loop, Stop/rePlay와 seek 경계까지 확인한다.
정상인 Artist/Lance/Warlord 경로도 같은 binder를 사용하므로 재질 admission 회귀를 확인한다.
Client/UI 실행·화면 촬영은 하지 않고 최종 육안 확인은 사용자가 수행한다.

## G11. All Effects World와 무비 타임라인 편집

현재 ClassSelectionTimeline은 read-only 투영이며 camera/Effect/material/light/clock의 정본은
`Data/Camera/ClassSelection.cinematics.json`, 모델 transform/animation/sound의 정본은 SL00
WorldSequences다. 두 문서의 stable ID를 유지하며 기존 presentation owner에 검증된 draft
적용과 Save/Reload를 연결한다. 선택 row/key 편집을 기존 SequencerTool Box Detail에 노출하고
재생·seek는 같은 CClassSelectionPresentation과 CWorldSequencePlayer가 소비한다.

All Effects의 World에는 현재 Level이 승인한 다섯 movie option을 그대로 표시한다.
Open Editor는 class ID 요청을 MainApp에 전달하고 기존 WORLD Sequencer를 연다. 별도
movie catalog·Effect 사본·재생기를 만들지 않는다. 현재 class의 실제 길이와 intro/loop
clock mapping을 사용하여 긴 컷신 전체를 편집·탐색할 수 있게 한다.

저장은 최신 디스크와 stable row/field 기준으로 비교해 무관한 수정은 보존하고 같은 필드의
실제 충돌만 거절한다. parse→validate→stage→commit과 실패 시 기존 draft/재생 보존을
검사한다. 기존 파일 확장으로 구현하되 새 C++ 파일이 필요하면 project/filter도 등록한다.
이 작업 중 사용자의 정본 JSON을 자동 교체하거나 Reload하지 않는다. 구현된 UI의 명시 Save가
저작·게시 경로를 사용하도록 연결하고, 임시 복사본에서 저장/재로드를 검증한다.
