# 마리오 기본 망치 크기와 Q 중복 표현 교정 결과

## G01. 반영 상태

`Client/Private/KoukuSaydonPresentationAssetService.cpp`의 Mario 전용 망치 preScale을
1.313에서 `1.313 * (2.3730526 / 1.5)`, 약 2.07721204로 변경했다. 기존 정적 WModel,
오른손 socket, pitch 220도, CModel/CMaterial 및 CPart_Equipment 경로를 유지했다.
Character는 이 weapon을 Mario에서만 표시한다. 일반 광대·NPC·카드미로와 무관하다.

`Data/Effects/Authored/effect.kouku.mario.player.hammer.impact.effect.json`의
`source.1.particlespriteemitter_6`와 `source.1.particlespriteemitter_16`만 visible=false로
바꿨다. 별도 망치 본체와 그 본체 위의 fresnel overlay를 비활성화하며 원본 sourceRecipe는
삭제하지 않았다. 다른 23개 emitter와 문서의 모든 다른 필드는 변경 전후 완전히 같다.

남은 charge 7개는 272.664ms, swing 3개는 391.842ms, hit 13개는 449.122ms에 시작한다.
기존 Q contact 약 400ms 전후 공격 표현과 sound/clip을 유지했다. 새 상시 Effect나 별도
rendering 경로, Resources, rendering option, gameplay 데이터는 추가·변경하지 않았다.

## G02. 크기와 모델 검증

두 설치 망치의 정점 수는 각각 2718개다. UV로 유일하게 대응한 1002개 정점의 basis 변환
최대 오차는 0.000005509이며, 변환의 축 배율은 49.99991~49.99996이다. Q의 cm→m 0.01,
source StartSize 2, socket scale 1.313을 모두 포함했다. 기존 장착은 같은 1.313과 body
local 축소 0.63209724를 소비한다. 새 값은 body 축소만 상쇄한다.

실제 설치 `MN_RPCZ_00-1.wmodel`의 idle 0초와 Q 0.4초·0.8초의 오른손 bone chain을
읽고 CModel preTransform, socket, body/root 순서로 전체 망치 정점을 변환했다. 변환된
정점 사이 최대 거리는 기존 장착 약 1.151088m, 새 장착 약 1.821062m, Q 약 1.821061m다.
새 장착과 Q의 세 표본 최대 차이는 0.0000031m다. 이 값은 실제 bone/geometry에 대한 CPU
행렬 검증이며 움직이는 손의 grip과 최종 GPU 화면을 확인했다는 뜻은 아니다.

기존 장착과 Q의 D/N/S DDS 파일 바이트는 container/mip 차이로 서로 다르다. 같은 source
WP_MN_RHKP_07의 decoded top mip은 각각 1024×1024, 1024×1024, 512×512이고 모든
픽셀 차이가 0이다. 기존 장착 재질과 Q native shader 자체가 동일하다고 판정하지 않았다.

## G03. 저장과 검증

원본 SHA256 `d63a02f535a12336f27f6d55ffec193a4bcf849b8fd86804319b0c59c8d7c80e`를
교체 직전에 재확인하고 out 백업 및 같은 디렉터리의 임시 파일을 사용해 원자 교체했다.
반영 후 SHA256은 `79a8ed41088526d3df9a602be804211994bf8e6f6839220ee68a8c1e29b3e39d`다.
두 visible 필드 외 JSON 동일, UTF-8 BOM 없는 C++와 CRLF 보존, git diff --check는 PASS다.

공식 `Tools/EffectPipeline/Validate-EffectSources.ps1` 전체 검사는 변경하지 않은 기존
`effect.kouku.gate1.blade-dance.circle.impact.effect.json`의
`v15 Product document has no runtime carriers`에서 실패했다. 해당 파일은 현재 Git diff가
없으며 이번 작업에서 수정하지 않았다. 이를 전체 Effect validation PASS로 기록하지 않는다.

변경한 Q 문서에 대해 공식 validator의 material color space, native particle option,
module override, attachment orientation과 resource closure 함수를 그대로 실행해 PASS했다.
실제 dependency는 47개, 5,129,896bytes이며 읽기·내용·안전한 상대경로 검증을 통과했다.
Effect는 DIRECT_AUTHORED_DOCUMENT 계약으로 같은 Data source를 읽으므로 별도 publisher나
DataFiles 복사는 실행하지 않았다. 다음 Client 실행에서 수정 문서를 읽는다.

증거와 원본 백업은 `out/MarioHeldHammer20260929/receipt.json`,
`final-matrix-validation.json`, `effect-validator.log`, `effect.before.json`에 있다.
root가 최종 Client Debug 증분 컴파일·링크·배포를 완료했다. 해당 로그는 같은 out 폴더의
`client-debug-build.log`이며 KoukuSaydonPresentationAssetService.cpp 컴파일과 Debug
Client.exe 링크를 확인했다. 기존 C4819 경고는 남고 root가 보고한 종료 결과는 PASS다.
Client/UI는 실행하지 않았다. 실제 기본 망치 크기·손 부착·Q 이펙트 최종 화면은 사용자가
확인해야 한다.
