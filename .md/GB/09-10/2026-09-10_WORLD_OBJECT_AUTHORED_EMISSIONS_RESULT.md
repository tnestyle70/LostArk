# 2026-09-10 World Object 저작 Emission 결과

대응 계획: [World Object 저작 Emission 계획서](2026-09-10_WORLD_OBJECT_AUTHORED_EMISSIONS_PLAN.md).
브랜치: `codex/kouku-arena-fire-0910`.

구현과 자동 검증은 끝났다. **publish, Client 빌드, 화면 확인은 실행하지 않았다.** 이 문서의 어떤
항목도 visual PASS가 아니다.

## 1. 요청과 실제로 바뀐 것

팀장님 요청은 "World Object Tool에서 갈고리를 눌렀을 때 지금 패턴으로 만들어 놓은 것을 위치·개수·
속도·애니메이션을 합쳐 프리뷰로 보고, 그 데이터도 거기서 관리"였다.

바뀐 것은 배치의 **정본 위치**다. 이전에는 개수와 위치가 Composition의 World 박스 78개에 있어 도구가
볼 수 없었다. 이제 각 Motion이 자기 `emissions` 행을 소유하고 Composition에는 박스가 하나만 남는다.

| 대상 | 이전 | 이후 |
|---|---|---|
| `PATTERN_18` World 박스 | 78 (불 60 + 갈고리 18) | 7 (불 6 + 갈고리 1) |
| `PATTERN_19` World 박스 | 18 | 1 |
| 불 D/E/F × CW/CCW 모션 | 행 없음 | 각 10행, 반경 12.6 / 11.7 / 10.8 |
| 갈고리 모션 | 행 없음, Lifetime 8000 | 18행, Lifetime 26875 |
| Collider 18개 | 박스 18개에 각각 연결 | 한 박스 + `worldEmissionIndex` 0..17 |

`worldsequences` revision 429 → 430, Composition revision 234 → 235.

## 2. 팀장님 도구 관례를 따른 지점

- 새 개념·새 창·새 파일을 만들지 않고 기존 `Physics / Motion / Emission` 섹션 안에 `Authored Emissions`
  표를 넣었다. 새 C++ 파일이 없으므로 `.vcxproj` / `.filters` 등록도 없다.
- `formatVersion`은 3 그대로다. `anchorKind`, `defaultMotionInstanceId`, `motionEnd`, `walkableSurface`,
  `displayName`과 같은 optional 필드 방식이며 행이 없으면 기존 동작과 같다.
- binding은 계속 1개다. `Is_SingleObjectMotion`, Workbench Append, NEXT 체인 규칙을 건드리지 않았다.
- 한 기능에 문서 parser/Validate/writer/equality → 런타임 → Tool → MainApp → Workbench → publisher →
  test → 팀 문서를 같은 변경 단위로 묶었다(`bf43ea52`가 `anchorKind`를 넣을 때와 같은 묶음).
- UI 라벨은 영어, 도움말은 한글이다(계약 테스트가 강제).
- Server C++, Shared packet, protocol 번호는 바꾸지 않았다.

## 3. 이전이 정확한 이유

한 박스 = 한 사본일 때 Client는 `world = S x R x T(local) x RotY(boxYaw) x T(boxPos)`를 만들고,
한 박스 + 행일 때는 `world = S x R x T(local) x RotY(rowYaw) x T(rowOffset + basePos)`를 만든다.
따라서 `rowYaw = boxYaw`, `rowOffset = boxPos - basePos`가 정확한 해다.

불의 base는 모든 박스가 공유하던 공전 중심으로 잡았다. 행 offset이 `R(yaw) x revolutionOffset`이 되어
궤적이 `R(yaw + w*t) x revolutionOffset + boxPos`가 되고, 모든 행이 box 위치를 중심으로 같은 원을 돈다.
`Distribute on Ring` preset이 만드는 형태와 같으므로 이후 도구 편집과 저장 데이터가 같은 모양이다.

갈고리는 행 지연(최대 18875ms)이 모션 Lifetime 8000ms를 넘는다. 기존 seed emitter 계약도 "Lifetime은
전체 생성 창"이므로, 저작 키를 그대로 둔 채 `visible=false` 꼬리 키를 붙이고 Lifetime을 26875로
늘렸다. `Sample_Track`이 마지막 키로 clamp하므로 각 행은 자기 8초를 그대로 재생한 뒤 사라진다.

## 4. 실행한 검증

| 검증 | 결과 |
|---|---|
| 이전 재현 검사 | 8그룹 78행 × 241시점, 위치 오차 ≤1e-4, 가시성 동일, 불일치 **0** |
| `cl /Zs` 구문 검사 (변경 9 TU) | exit 0, error 0, C4819 외 경고 0 |
| WorldSequence 계약 테스트 | 30개 중 29 PASS. 새 emission 케이스 9개(유효 1 + 거부 8) 포함 |
| Composition projector 테스트 | 117개, 실패 13개가 **변경 전후 완전히 동일**(아래 5절) |
| projector `--mode validate` | parse·validate·projection 통과, `projected Product is stale`까지 도달 |
| 실제 publisher parser | `Read-WorldSequenceDocument`가 이전된 authoring 원본을 수락 |
| 변경 JSON parse | 5개 전부 통과 |
| `git diff --check` | 오류 0 |
| 인코딩·줄바꿈 | 편집한 모든 파일이 UTF-8(BOM 없음)과 원래 CRLF/LF 유지 |

거부를 확인한 8가지: `count`가 행 수와 불일치, 행과 함께 `intervalMs` 지정, 행과 함께 `spreadDegrees`
지정, 지연이 Lifetime 초과, 행에 모르는 필드, 행에 필드 누락, 지연이 소수, 빈 행 배열.

## 5. 기존 실패로 확인한 항목 — 내 변경이 아니다

- `test_korean_help_is_escaped_utf8_because_map_tool_has_no_utf8_flag` 실패. `Client.vcxproj`의
  `MapTool.cpp` 항목에 `AdditionalOptions /utf-8`이 들어 있어서다. 그 항목은 `7eaa7b69`
  (tnestyle70, 2026-07-29)로 이미 커밋돼 있고 내 변경에는 `.vcxproj`가 없다. 프로젝트 파일은
  팀장님 정본이므로 건드리지 않았다.
- Composition projector 테스트의 실패 13개는 변경 전 원본(HEAD projector + 이전 전 데이터)으로
  되돌려 측정한 기준선과 **이름까지 동일**했다. 측정에 쓴 목록은
  `out/Gate3EmissionMigration/`이 아니라 세션 임시 폴더에 있으며 커밋 대상이 아니다.

## 6. 실행하지 않은 것

- **publish 미실행.** Visual Studio(`devenv.exe`)가 열려 있어 같은 워킹 트리에서 publisher를 겹쳐
  돌리지 않았다. `Client/Bin/DataFiles/Map/...worldsequences.json`은 revision 429 그대로이고
  `Data/Encounters` / `Data/Animation/Authored`의 생성물도 stale이다.
- **Client 빌드 미실행.** 구문 검사만 했다. 링크와 실제 실행은 확인하지 않았다.
- **화면 확인 없음.** 도구에서 갈고리 18개가 웨이브대로 보이는지, 불 세 줄이 도는지, 잡기 판정이
  이전과 같은 자리에서 걸리는지는 전부 사용자 확인 대기다.
- Server C++ / Shared / protocol / `.vcxproj` / Resources 파일은 변경하지 않았다. 새 리소스도 없다.

## 7. 사용자 확인 절차

Visual Studio를 닫은 뒤 순서대로 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish
python Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode publish
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon
```

그다음 Client(x64 Debug)를 빌드하고 Server와 Client를 재시작한다.

1. Lobby → KoukuSaydon → F1 Tools → **World Object Tool**
2. Object Resources에서 `월드오브젝트_갈고리` 선택 → 연결 Motion `3관문_갈고리_바닥끌기_내려치기_회수`
   선택 → Object Sequencer **Play**. 갈고리 18개가 1초 / 10초 / 19초 웨이브로 두 방향에서 나온다.
   Object Detail의 `Authored Emissions` 표에서 행의 Offset / Yaw / Start Delay를 바꾸고 다시 Play한다.
3. `3관문_외곽불_D` → `3관문_추가불_D_CW_외곽공전` Play. 불 10개가 반경 12.6m 원을 돈다.
   E는 11.7m, F는 10.8m이며 CCW 모션이 반대로 돈다. 세 줄 전체는 각각 눌러 확인한다.
4. Save를 누르면 `worldsequences.json` 원자 저장과 `-Scope WorldSequences` publish가 자동 실행된다.
   Reload로 왕복을 확인한다.
5. F1 Tools → Action Workbench → Boss KoukuSaydon → GATE3 →
   `3관문_외곽불회전_갈고리대각선_시각테스트` 재생. 불과 갈고리가 이전과 같은 자리·같은 시각에 나오는지,
   갈고리에 맞았을 때 끌려가는지 확인한다.
6. `3관문_갈고리만_확인용_불없음`은 불 없이 갈고리만 확인하는 용도다.

2·3번이 이번 요청의 핵심이다. 5번은 이전이 기존 재생을 깨지 않았는지 보는 대조군이다.

## 8. 남은 경계

- 한 번에 한 오브젝트의 모션만 재생하므로 `외곽불_D`를 누르면 D줄 20개(CW 10 + CCW 10)를 두 모션으로
  나눠 본다. 불 60개 전체와 갈고리를 한 화면에서 보는 것은 Workbench Box Detail의 `Preview placements`나
  실제 패턴 재생이다. 이는 불이 D/E/F 세 모델이라는 저작 선택의 결과이지 이번 구조의 제약이 아니다.
- 도구 프리뷰는 박스 placement 없이 `instance.position`을 쓰므로 링 중심이 프리뷰 원점이다. 아레나
  재생은 박스 placement를 써서 저장 좌표 그대로다.
- `worldEmissionIndex`는 WORLD anchor Collider에만 허용한다. Effect companion은 기존처럼 박스 단위다.
- 행이 있는 모션은 `NEXT` 체인에 쓸 수 없다. 기존 "NEXT는 단일 생성" 규칙을 그대로 따른다.
