# 마리오 기본 망치 크기와 Q 중복 표현 교정

## G00. 현재 실측과 변경 경계

사용자는 Q 때 나타나는 큰 망치를 평소에도 들고, Q에서는 공격 이펙트가 나오도록 요청했다.
기본 장착은 `CharacterCatalog -> CPart_Equipment`의 기존 정적 망치이고, Q는
`Clown.interactionbindings.json -> effect.kouku.mario.player.hammer.impact`의 별도 망치
mesh particle 두 개를 겹쳐 그린다. body와 실제 hand bone, 기존 clip·서버 판정은 유지한다.

두 설치 WModel은 각각 2718개 정점이며 UV로 일의적으로 대응한 1002개 정점의 affine
변환 최대 오차는 0.000005509다. Q carrier의 cm→m 0.01과 StartSize 2를 합치면 기존
hand-frame cook과 같은 형상·치수다. 그러나 Q source bone은 Character root를, 장착 망치는
Mario body local 축소 `1.5 / 2.3730526`까지 소비하므로 Q 쪽이 약 1.582배 크다.
Q의 `actionCueAttachment.socketLocalTransform.scale`에도 `[1.313, 1.313, 1.313]`이
있으므로 두 경로에 공통인 1.313을 크기비에서 빼거나 다시 곱하지 않는다.
기본 망치 D/N/S와 Q 본체는 같은 WP_MN_RHKP_07 source texture를 사용한다.

## G01. 기존 장착 모델의 크기

`Client/Private/KoukuSaydonPresentationAssetService.cpp`의 `Ensure_ClownBodyPrototype`은
기존 WModel, static shader, pitch 220도, hand socket과 Prototype staging을 유지한다.
`CLOWN_HAMMER_PRE_SCALE`만 `1.313f * (2.3730526f / 1.5f)`로 바꿔 Q에서 보던
치수의 망치를 실제 작은 body의 손에 장착한다. 값은 약 2.07721204다.
새 H 계약·C++ 파일·project/filter 등록은 없다. 새 Resources도 필요하지 않다.
Character의 `Apply_MarioPresentation`과 `Set_PartVisible`이 일반 광대에서는 이 무기를
숨기고 Mario에서만 보이게 하므로 일반 광대·NPC·카드미로 망치 크기는 변경하지 않는다.

## G02. Q는 기존 타격 이펙트만 재생

`Data/Effects/Authored/effect.kouku.mario.player.hammer.impact.effect.json`의 stable ID
`effect.kouku.mario.player.hammer.impact.source.1.particlespriteemitter_6`와
`effect.kouku.mario.player.hammer.impact.source.1.particlespriteemitter_16`의 `visible`만
false로 바꾼다. 앞 항목은 별도 망치 본체이며 뒤 항목은 같은 망치에 붙는 fresnel overlay다.
기존 위치가 실제 축소된 손과 다르므로 두 망치 geometry를 함께 격리한다. 나머지 23개
charge·swing·hit particle의 시간·본 부착·크기·재질과 Q sound/clip은 그대로 유지한다.

최신 파일을 읽고 stable ID 두 개가 정확히 한 번씩 존재하는지 검사한다. 후보를 먼저 검증하고
직전 SHA256을 재확인한 뒤 원본 백업과 같은 폴더의 임시 파일을 사용해 원자 교체한다.
다른 필드의 동시 변경은 덮어쓰지 않는다. direct-authored Effect는 CLAUDE의 계약대로
제품이 같은 파일을 읽으므로 별도 publisher나 DataFiles 복사는 하지 않는다.

## G03. 검증과 전달

설치 WModel 정점·UV 대응, 실제 source StartSize/mesh preScale와 Mario body 축소를
같이 계산한다. 변경 전후 JSON을 비교해 두 visible 필드 외에는 동일함을 검사하고,
남은 23개 emitter가 Q contact 약 400ms 전후에 재생되는지 authored timing을 확인한다.
공식 Effect source validator와 git diff --check, 기존 C++ UTF-8/CRLF 보존을 확인한다.
컴파일은 root의 통합 Product 빌드가 담당한다. Client/UI 실행과 실제 크기·이펙트의 화면
판정은 사용자가 수행한다. 자동 수치 검사와 사용자 화면 확인을 RESULT에서 구분한다.
