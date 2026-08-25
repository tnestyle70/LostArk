# 베른성 회색 가림 메시 수정 결과

## 구현 상태

- Bern exact asset pipeline이 vertex-blend channel과 legacy `layer01_diffuse` /
  `normalmap` parameter를 runtime texture lane으로 결정적으로 변환한다.
- 나무·절벽·산·눈 asset 7종을 exact source에서 다시 추출·조리해
  `Client/Bin/Resources/Map/LV_BER_BERNCASTLE`에 배포했다.
- exact export에도 material parameter가 없는 해안/장식 특수 메시 22개 placement는
  authoring과 imported shard에서 `visible=false`로 격리했다.
- Bern shard builder가 stable source placement visibility override를 shard 재생성에서도
  보존하고, unknown/duplicate/invalid override는 commit 전에 거부한다.
- 원경 산 2종과 눈 장식은 source blend mode에 맞춰 Alpha render profile을 적용했다.

## 자동 검증

- Bern asset pipeline unit test: 6 PASS
- Bern shard builder unit test: 11 PASS
- Bern shard 재생성: 950 static assets / 50,017 placements / 13 shards PASS
- `Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE`: PASS
- visible opaque-gray fallback audit: 0 assets
- Server x64 Debug build: PASS
- Server `--navigation-contract-test`: 4 PASS / 0 failures
- Client x64 Debug build and full relink: PASS
- `git diff --check`: PASS

## 남은 검증

에이전트는 Client를 직접 실행하거나 화면 판정을 하지 않는다. 사용자가 Lobby -> Character
Select -> Bern으로 진입해 기존 이동 경로를 반복하고, 회색 면이 화면을 가리지 않는지 최종
확인해야 한다.
