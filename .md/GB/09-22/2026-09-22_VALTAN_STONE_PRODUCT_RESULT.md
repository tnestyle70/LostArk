# 발탄 돌 Product 생성 연결 결과

## G00. 반영 상태

사용자가 편집 중이므로 최종 데이터 반영을 기다린다. 이 작업은 C++ 소비자 최소 변경과
out 후보 준비·자동 검증까지 끝냈다. Data/Effects/Authored와 게시 데이터는 교체하지 않았고
Client/Server 실행·UI 조작·link·commit/push를 하지 않았다. 통합 담당은 사용자 대기 요청
이전에 Client /t:ClCompile Debug x64를 완료했다고 보고했다. 이후 link와 publish는 보류한다.

최종 적용 후보는 두 manifest의 합계7문서다.

- out/ValtanStoneProduct20260922/manifest.json: 기존 십자 Product1, persistent active3.
- out/ValtanStoneMask20260922/source_candidates/manifest.json: source library3. 이 후보의
  재질 교정·원본 GPU 대조는 통합 담당이 소유하며 아래 codec 검증만 이 작업에서 수행했다.

기존 Product stable ID와 현 runtime 연결을 유지한다. ground-roar, six-pizza, struggling의
active는 .600000024→.7200000288, cross4개는 기존 .4/.4/.7에1.2를 곱한다.
native2391의09.gap_offset=.25와 일치하는 authoring scalar override를 사용한다.
일반 연기·텔레그래프·폭발 파편 및 Server damage, coverRadius1.5m, 위치·지연·수명은 보존한다.
part-break는 ground-roar active를 공유하므로 같은 교정이 적용된다.

## G01. 실제 코드 변경

Client/Private/Effect_Playback.cpp의 Step에 명시된 고정 간격을 가진 portable SourceRecipe를
기존 Spawn_FixedCenterSpacingParticles로 연결했다. 동일 Spawn_Particles가 native size,
lifetime, dynamic parameter를 평가한다. SourceRecipe를 추가하면 기존 source 분기가
고정 간격을 건너뛰던 문제가 원인이었다. source burst/rate와 중복 생성하지 않는다.

Client/Private/Effect_DocumentCodec_Validation.cpp는 direct authored, world-space mesh와
위치·운동 모듈이 없는6종(required/spawn/lifetime/TypeDataMesh/size/dynamic)만 고정 간격
source carrier로 허용한다. 기존 zero rate/burst, zero offset/velocity/acceleration과 범위
검사는 보존한다. public header와 project/filter의 변경은 없다. 기존 dirty 변경은 보존했다.

Tools/EffectPipeline/stage_valtan_product_stones.py는 최종 source를 직접 쓰지 않는 후보
생성기다. 네 stable cross element의 material/resources/recipe와 크기만 교체하고 active3의
대상 돌 element만 gap/scale을 고친다. baseline SHA·원문 backup·후보 SHA를 기록한다.
cross generic base/mask override는 교체된 native material에 더 이상 존재하지 않는 slot이므로
해당 material 교체와 함께 제거한다. 연기4개의 source byte는 유지한다.

Tools/EffectPipeline/test_valtan_cross_rock_wave_effect.py의 기존 Product 계약은 native2391,
새 material/mesh, gap.25와 돌만1.2배인 관계를 검사하도록 맞췄다. 아직 Data 설치 전이므로
이 테스트는 후보 경로를 지정하여 실행했으며 저장소 원문 대상으로는 설치 후 다시 확인한다.

## G02. 실제 자동 검증

- 변경한 두 CPP를 out 분리 object로 compile하여 exit0. 기존 Engine CP949 헤더의 /utf-8
  warning이 있었으며 이번 파일의 인코딩은 UTF-8/기존 CRLF를 보존했다.
- 실제 CEffectDocumentCodec Load/Validate_Drawable와 CEffectPlayback의60Hz 재생:
  cross rootScale1 peak56,6,776 particle samples; 실제 Product cue의 rootScale1.5 peak80,
  9,680 samples. 기준본과 매 tick count·stable ID·생성 중심이 같고 돌의3축 길이만1.2배다.
- persistent 후보3개는5초/19.5초/5초의 lifetime·native dynamic parameter·companion
  count를 유지했다. 실제 돌 표본은300/1171/300이며 저작 수명 이후 대상 돌은0개다.
- source 후보3개는 실제 codec load/drawable/stage 및 Serialize→Parse→Serialize의 canonical
  왕복이 동일했다. native2391 표본은221/191/242다.
- local-space 고정 간격과 SourceRecipe 위치 module의 오염을 validator가 거부했다.
- 위 C++ probe 합계436,179검사 통과:
  out/ValtanStoneProduct20260922/product_probe.cpp / product_probe.log.
- 기존 cross 계약6개는 후보 경로에서 통과. rock-pillar Server/publisher 계약8개도 통과했다.
- 변경 파일 git diff --check 통과. 실제 화면의 형태·색·소멸은 사용자 검증 대기다.

첫 probe 실패는 out EXE가 Resources 루트를 찾지 못한 실행 준비 오류였다. 명시적인
LOSTARK_RESOURCE_ROOT=Client/Bin/Resources로 해결했으며 runtime 검증을 제거하지 않았다.
GPU gap 결과는 통합 담당의 별도 결과를 따른다. 원본 Dynamic.X와 natural lifetime을 유지했고
terminal 잔여 fragment를 없애기 위한 임의 alpha/색 module은 추가하지 않았다.

## G03. Product 게시 경계

십자 effect.valtan.sequence.cross는 Valtan.patterneffectcues의 직접 cue라 presentation
generation의 SHA 대상이다. 조사 당시146artifacts의 generation은
d2aca7336ddc43a42a73b08a1a410e1f64392b5f2b5dd050714b06ef288acf80이었고 cross 후보만으로
7bd856b1673bfe1d75c891e58e0ad208ec3bcc51f9998118eafb2cc821e5853f로 바뀌었다.
실제 적용 때는 최신 저장본·다른 변경에 따라 generation이 달라지므로 이 값을 고정하지 않는다.

저장본 적용 후 최소 Publish-GameplayBalance.ps1 -Mode Publish로 Gameplay.bootstrap과
해당 generation manifest를 생성해야 한다. 현재 writer는 pattern cue closure를 수집하므로
BossCatalog의 persistent active/explode만 있는6문서는 기존 generation hash 대상이 아니다.
이 사실을 publisher 지원 확대나 이미 설치된 상태로 설명하지 않는다.

## G04. 버러지와 발악의 구분

VALTAN_STRUGGLING은 '3페이즈 전 발악패턴'이다. 사용자 표현의 버러지는 실제 stable pattern
VALTAN_TRASH와 TRASH_CATCH_SUCCESS/FAIL/IF이며 서로 다른 패턴이다.

현재 저장본에서 네 TRASH 패턴의 gameplay에는 rock combat-object spawn이0개다. 전체 Valtan
rock spawn은 ground-roar, six-pizza, struggling, part-break의4archetype뿐이다.
TRASH Product V2 closure는 hand_1~6와 counter pulse group이고 WModel은
Effect/DimensionMaster/Meshes/fm_b_ring_001.wmodel 하나다. V1 catch effect는 sphere/helix,
map source-preview.trash의8effect track도 cast/light/helix를 사용하며 돌기둥 mesh는 없다.
현재 authored full.restore 문서에도 fm_d_stoneparts_003 직접 요소가 없다.

Server CCombatObjectRuntime은 기존 combat object를 자기 lifetime 동안 유지하고 source 사망·
소멸을 따로 처리한다. 다른 패턴에서 생긴 돌을 버러지 동안 볼 가능성은 있으나 실행 로그나
사용자 occurrence 식별 없이 그것이라고 확정하지 않는다. 현재 확정된4archetype의 shared
visual은 후보에 포함됐고, 별도 버러지 돌 generator는 발견되지 않았다. 사용자 편집이 끝난
최신 저장본이나 실제 선택한 occurrence로 다음 반영 때 재확인한다. 버러지 전체 완료로
기록하지 않는다.
