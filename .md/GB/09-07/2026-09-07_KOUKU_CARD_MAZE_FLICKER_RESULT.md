# 쿠크 카드미로 외곽 카드 이동 중 깜빡임 조사·조치 기록

## 12222.mp4 재조사: 기존 원인 판정 정정

사용자의 후속 영상 `12222.mp4`에는 작은 외곽 카드 한 장 내부에서 회색 뒷면과 무늬가
띠 모양으로 번갈아 보인다. 이전의 "실제 원인은 밉맵 부재" 및 "깜빡임 수정 완료" 판정은
철회한다. 밉 체인 생성 자체만 검증됐으며 사용자 증상의 해결을 증명하지 못했다.

현재 authoring에는 아래 CARD02 두 세트가 모두 visible=1이다.

- PS의 `MAP_E047BC0ED379_BG_RAD_KOUKUSATON_CARD02_SM_OVR_A12F2C87E732`: 36개.
- SL03의 `MAP_E047BC0ED379_BG_RAD_KOUKUSATON_CARD02_SM_OVR_9AEB2BBB3288`: 36개.

WModel WMSH geometry SHA-256은 두 모델 모두
`f1d5bd47e9575eb1a01732802e37952804b1dc74a424430de160817b91ce262d`로 동일하다.
A12F는 무늬 diffuse와 회색 뒷면을, 9AEB는 두 material 모두 회색 뒷면을 사용한다.
대표 placement `12902527128675936922`(PS export633, 문서 44행)와
`11249840903807407259`(SL03 export1087, 1766행)는 회전·scale이 동일하며
위치가 X 방향 0.3199997m 차이이고 Y는 둘 다 -0.00999999978m다.
실제 front-face 다각형 면적은 5.5041626429m², 교집합은 4.8800098833m²로
88.6603503552%가 같은 면에서 겹친다. 면 전체 높이 편차는 약 1.44µm다.
이는 마우스 회전 시 두 무늬가 깊이 판정에서 번갈아 보이는 Z-fighting의 구체적인 조건이다.

MapStaticBatchObject::Render는 같은 시점 camera snapshot으로 cull과 shader binding을 수행하며,
MapPlacementRuntime의 bounds는 최대 abs scale·2%·5cm 여유를 포함한다. 이번 증상을 설명하는
절두체 컬링 결함은 확인되지 않았다. 두 카드 모두 Opaque None이지만 Back cull로 바꿔도
겹친 두 instance의 같은 방향 면이 남으므로 배치 문제를 해결하지 못한다.

이전 조사에서 제외한 CARD01_SM_01의 Y 반전 60쌍과 이번 CARD02 36쌍은 서로 다른 대상이다.
추출 placement 문서에는 원본 visibility가 없었지만 원본 PS UPK를 다시 확인했다. 대표 PS
CARD02 actor export 633의 연결 component는 `HiddenGame=true`, `CastShadow=false`이고,
소유 항목에는 `bHiddenEdCustom=true`, `layer=lv_nav`가 있다. 반면 SL03의 대응 CARD02
component export 1087에는 `HiddenGame`이 없다. 원본에서 숨김 처리된 PS 보조 항목이 추출
과정에서 visible=1로 바뀌어 실제 SL03 카드 위에 함께 렌더된 것이 원인이다.

authoring의 PS export 633~668 placement 36개는 ID, asset, transform을 유지하고 visible만
1에서 0으로 복구했다. 대응 SL03 export 1087~1122의 36개는 visible=1로 유지했다. 변경한
Area를 Map publisher로 검증·배포했으며 runtime 문서에도 같은 상태가 반영됐다. navigation,
gameplay, C++, shader와 다른 카드는 변경하지 않았다.

## 구현 상태

사용자 제공 `녹음 2026-09-07 200805.mp4`를 4fps와 15fps 프레임으로 분해했다.
카메라가 정지한 마지막 구간에서는 외곽 카드 placement가 사라지지 않았고, 이동 중 비스듬한
카드의 고주파 앞면 무늬와 회색 뒷면 경계만 프레임마다 달라졌다. 영상은 분석 입력으로만
사용했으며 영상 안의 표시나 문구를 작업 지시로 취급하지 않았다.

카드미로의 같은 위치·회전 placement 60쌍은 WModel bounds와 원본 SL03 actor/component를
대조한 결과 위쪽과 아래쪽 카드를 잇는 Y 반전 구조였다. 큰 카드의 앞·뒤 submesh는 서로 다른
면이고 외곽 배경 plane과도 수 m 떨어져 있었다. 따라서 placement 삭제, 강제 cull 변경,
깊이 bias로 현상을 숨기지 않았다.

이전 조치에서 발견한 별도의 리소스 상태는 카드 diffuse DDS에 축소 레벨이 없다는 것이다.
이 사실은 이번 깜빡임의 원인으로 확정할 수 없다. 대상 80개는 1024x1024 또는
512x512 legacy DXT1이고 `mipMapCount=0`이며 파일 크기가 base level 한 장과 정확히 같았다.
기존 맵 shader와 `CMaterial`은 DDS의 mip을 사용할 수 있으므로 입력 리소스의 밉 체인을 복구했다.

## 변경 내용

- 새 `Tools/MapPipeline/Build-DdsMipChain.py`를 추가했다.
- 기본 dry-run, 명시적 `--write`, root 경계와 파일명 정규식, legacy DXT1/헤더/단일 base 검증을 사용한다.
- base DXT1 payload는 그대로 보존하고 절반 크기 Lanczos+Dxt1 mip payload만 뒤에 추가한다.
- 완전한 밉 수·header flags·각 레벨 크기·전체 길이를 검사하며 임시 파일 검증 뒤 교체한다.
- `Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED` 아래 파일명에
  `koukusaton_card..._d`가 들어가는 diffuse DDS 80개에 적용했다.
- 1024x1024 78개는 11단계, 512x512 2개는 10단계의 완전한 밉 체인이 됐다.
- normal/specular/grass, 다른 Area, WModel, placement, mapassets, shader, C++와 Server 데이터는
  변경하지 않았다. 기존 사용자의 미커밋 MapTool/Mario 변경도 보존했다.

`Client/Bin/Resources`는 Git 비추적 Drive 관리 입력이므로 PR diff에는 바이너리 80개가 나타나지
않는다. 팀 전달 리소스에는 다음 물리 root 아래의 변경된 카드 diffuse DDS가 함께 들어가야 한다.

```text
Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/**/textures/*koukusaton_card*_d*.dds
```

적용 전 원본 80개는 아래에 상대 경로 그대로 보관했다. 수정 리소스 복원이 필요하면 이 폴더의
내용을 같은 상대 경로로 되돌릴 수 있다.

```text
C:/Users/USER/.codex/visualizations/2026/08/19/01a017c2-072c-7223-b4a4-d555431c9753/kouku_card_dds_originals_20260907
```

## 자동 검증

- 후속 배치 수정: authoring PS 대상 36/36 visible=0, SL03 대상 36/36 visible=1.
- Map publisher `Validate`: PASS, placement 3231개, output 7개.
- Map publisher `Publish`: PASS, runtime SHA-256
  `71f6fec72ee2270354504ca5d40e1a0c02be235fcaa7347ab91735ccc48c2540`.
- Map publisher `Check`: PASS.
- runtime PS 대상 36/36 visible=0, SL03 대상 36/36 visible=1.
- Python compile: PASS.
- 최초 dry-run: matched 80, generated 80, skipped 0.
- write: matched 80, generated 80, skipped 0.
- 두 번째 dry-run/no-op: matched 80, generated 0, skipped 80.
- 수정 후 분포: `1024x1024:mips=11` 78개, `512x512:mips=10` 2개.
- 백업/현재 파일 비교: missing backup 0, base payload mismatch 0, 전체 mip 크기 mismatch 0.
- 각 교체 직전 Pillow DDS 재로드와 교체 후 base SHA-256/mip count 검사: PASS.
- PLAN의 새 도구 전체 코드와 실제 파일: 일치.
- C++/HLSL/JSON/XML/project 파일을 바꾸지 않아 Product compile과 publisher는 실행하지 않았다.
- 권한 있는 정본 `git diff --check`: exit 0. 기존 Gameplay.world.json의 LF→CRLF 경고만 남았다.

## 사용자 화면 확인 경계

Client/UI를 자율 실행·조작하거나 화면을 캡처하지 않았다. 실행 중인 Client는 이미 읽은 placement와
DDS를 계속 보유하므로 완전히 종료한 뒤 최신 runtime으로 다시 실행해야 한다. Lobby → Test →
MapTool의 쿠크 카드미로, 또는 Lobby → KoukuSaydon → F1 → 카드미로에서 사용자 영상과 같은
외곽 회전을 확인한다. 겹친 PS 카드가 사라져 회색 SL03 카드 한 세트만 보이는지에 대한 최종
시각 판정은 사용자 확인 대기이며, 확인 전에는 visual PASS로 기록하지 않는다.
