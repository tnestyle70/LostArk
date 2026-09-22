# 쿠크 2관문 흐름 반복과 원본 사운드 구현 계획

## G01. 흐름 저장과 반복 시작점

현재 Gate2 raid는 마지막 entry 후 정지한다. `patternFlows.loopStartEntryId`를 선택적 stable entry ID로 추가한다. Client 문서 parse/validate/save와 Boss Tool의 기존 Flow 편집·재생, Python projection, PowerShell gameplay publisher, Server bootstrap catalog와 완료 전이를 연결한다. 값이 있으면 그 행부터 마지막 행까지 반복하고 앞 구간은 최초 한 번 실행한다. 미지정 문서는 기존 관문별 정책을 유지한다. 삭제·잘못된 ID는 publish/admission에서 거절하며 별도 런타임은 만들지 않는다.

## G02. 저글링·나팔 사운드와 등장 자막 후보

`out/RaidAudio20260922/source/MN_RPCZ_00.action-effects.json` 원본 action 4219713/4219716의 AkEvent를 현재 animation source-in/rate/stage offset에 대응한다. 이미 설치된 SOUND resource와 catalog를 재사용한다. 사용자 문구 두 줄을 기존 SUBTITLE resource/occurrence로 등장 bundle에 연결하는 stable ID 후보를 `out/Gate2FlowAudio20260922`에 만든다. 공유 authoring JSON은 통합 담당이 최신 디스크 기준 병합한다.

## G03. 검증

새 C++ 파일은 없어 project/filter 등록이 필요하지 않다. optional boundary 저장·projection·잘못된 ID 거부, Server/Client 끝 전이와 기존 정책 보존을 검사하고 변경 TU를 컴파일한다. 후보는 멱등성과 무관한 필드 보존, 설치 음원 존재·catalog join을 확인한다. Client/UI·청취·최종 화면 확인은 수행하지 않는다.

## G04. 후속 source projectile 사운드 범위

기존 sound resource가 없는 projectile AKEvent는 설치 Wwise의 Play target·random playlist·미디어를 확인해 후보로 추출한다. 저글링 Attack04 launch/impact 두 이벤트·8 원본 variant만 통합하고, 등장 Attack07 두 이벤트·8 variant는 조사 후보로 둔다. 추가 targeted SOUND 런타임은 이번 범위에 넣지 않는다. source action SOUND는 pattern lifetime 안으로 제한하며 공 자체의 impact cue는 통합 담당의 실제 effect 착지 시계에 대응한다.
