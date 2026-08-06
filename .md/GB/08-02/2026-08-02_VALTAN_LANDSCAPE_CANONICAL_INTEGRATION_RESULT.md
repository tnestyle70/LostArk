# 발탄 Landscape 정본 통합 결과

## 결과

발탄 원본 Landscape 6개를 기존 `LV_LUT_HEARTRB_ED` 단일 MapTool 문서에 통합했다.

| 항목 | 통합 전 | 추가 | 통합 후 |
|---|---:|---:|---:|
| catalog asset | 269 | 6 | 275 |
| placement | 13,103 | 6 | 13,109 |

Landscape는 별도 `overlay`가 아니라 원본 `component` Transform과 imported ID를 유지한다.
따라서 `LV_LUT_HEARTRB_ED` 한 Area로 게임 레벨과 MapTool이 같은 데이터를 읽으며,
`.mapset` shard read-only 모드로 바뀌지 않아 기존 Save/Reload 경로도 유지된다.

## 변경 내용

1. `merge_maptool_landscape.py`를 추가했다.
   - catalog v4, placement v2 스키마와 행 필드 수 검사
   - Area, asset ID, Prototype tag, runtime/source placement ID 충돌 검사
   - placement-to-asset join과 `transformSource=component` 검사
   - Landscape WModel 경로와 WINT/WMOD 버전, 크기, 섹션 테이블 검사
   - 두 출력의 사전 staging, 교체 실패 시 catalog 복구
   - 같은 입력을 다시 실행하면 추가 0건이 되는 idempotent 병합
   - 동일 출력 재실행 시 최초 receipt를 보존하고, 다른 출력의 receipt면 쓰기 전 실패
2. `LV_LUT_HEARTRB_ED.mapassets`와 `.mapplacements`를 275/13,109로 생성했다.
3. 현재 DDS 디코더와 hole 규칙으로 원본 UPK에서 6개를 다시 검증하고,
   `baked_diffuse.png` 6개와 `baked_normal.png` 6개만 공유 런타임 리소스에 교체했다.
   WModel 6개는 기존 설치본과 해시가 같아서 교체하지 않았다.
4. UE3 원본 Landscape hole 계약을 확인해 추출기의 기준을 `__DataLayer__ > 170`으로 수정했다.
   - 렌더 메시 topology는 원본 UE3 PC 경로처럼 유지한다.
   - 화면 hole은 opacity mask로 처리한다.
   - 추후 collision/nav 전용 geometry를 만들 때만 해당 샘플이 소유한 quad의 두 삼각형을 제거한다.
5. 베이크 입력을 환경 영향을 받는 `tgaRoot`가 아니라 명시적 `-dds` 출력인
   `ddsRoot`로 고정했다. 따라서 팀원별 `umodel.cfg`와 무관하게 같은 픽셀을 생성한다.

최종 표시용 텍스처의 SHA-256은 다음과 같다.

| suffix | diffuse SHA-256 | normal SHA-256 |
|---|---|---|
| `00080` | `EE8270BF02935506E509FCD00832FE684CB323384AB15DCD39508EF098EA0240` | `0B2A5982C2F51A18E447E5204C8B30BC141E9F8318F7D195F89311589C08CF63` |
| `00081` | `6F2240BF1F756BB4A527110D0EF7D820C6F6717C03869993C935C9046AFB7277` | `73E9018D7F7A6A82F96C6DE238B9322FC2E3284C289E50A764C11EA29C49B4F3` |
| `00082` | `B9FCA276CCD6EC63988095A7EC8BDD1B15FCB0D461DA3C312E270F2B8D24A749` | `4001F9E6250B9F5D5B250D1913DAA916FAF8A909A1FAF1544DE015FC094BDCBE` |
| `00083` | `0020D7E700E6B4CFDBA842570B26298838D0EE023C56D0C0F5A4D8285846F3C0` | `951EF4C7B5BED33DCAAAA0D6DA080BA1636BB21220CB56F88C570C5CE7FCCD2C` |
| `00084` | `B4160CA55FEE41E44BDD2FEB560042411F79FFCC29B2E8BDBF2B19DA1821434E` | `6D23FCA949F247A0D5B8BE695378028F80C6433BA5413B5336DA90D054678B28` |
| `00085` | `C0483467424277BFE4541D8C12AEE26465D78E9BD8CF1C4D2A494D84937C3981` | `226A779E908F5C83BA0CA1A067E4150315C987A7D8C9FAB46037BAA7FCEB7C76` |

재생성 증빙은
`C:\LostArkExtract\valtan\landscape_hole170_dds_final_2026-08-02`에 보존했고,
manifest SHA-256은
`18E32AA3BA5E24CD5F188B13D05CD012EC6CB67B53AC9E11EBC75DFD6295CAD6`다.

## 검증

- Landscape 병합 도구 11 tests 통과(출력 경로 alias, 가짜/잘린 WModel,
  receipt 보존·실행 전 및 check-only 충돌 검사, 두 번째 출력 교체 실패 rollback 포함)
- 기존 MapTool scene 생성기 15 tests 통과
- 기존 Bern shard 생성기 6 tests 통과(LevelPlacementExtractor 합계 32 tests)
- Landscape 추출기 16 tests 통과(명시적 DDS 입력 선택 테스트 포함)
- Python compile 통과
- 실제 입력 사전 검사: 충돌 0, 출력 275/13,109
- 병합 재실행 검사: 추가 asset 0, placement 0
- catalog SHA-256: `523D12B471141447C01225027467639D1980EDC2B91A403AC4D0EDA47C9CFAE4`
- placements SHA-256: `FF344A82E7CB9DC514866F2BED158A16E92F7955BA5DA6AC1547D83867641B23`
- Client x64 Debug/Release 빌드 성공
- 최종 DDS 텍스처 설치 후 `Client/Bin`을 작업 디렉터리로 사용해 Debug
  `Client.exe --map-level=valtan`을 다시 실행했다. 약 4초 안에
  `Valtan Arena Map` 진입, 응답 정상, 오류 대화상자 없음까지 확인했다.
- 실행 작업 디렉터리를 `Client/Bin/Debug`로 두면 `../Bin/ShaderFiles` 상대 경로가
  깨져 `Failed to Created : CShader`가 발생한다. 실행 파일 위치와 관계없이 런타임
  작업 디렉터리는 기존 계약대로 `Client/Bin`이어야 한다.
- Landscape WModel 6개는 모두 246,492바이트의 완전한 WINT/WMOD 컨테이너이며,
  내장 WMAT v2의 material 0이 각 모델 옆 `textures/baked_diffuse.png`와
  `textures/baked_normal.png`를 정상적으로 참조한다. 런타임 `CModel -> CMaterial`도
  이 상대 경로를 WModel 부모 디렉터리 기준으로 해석하고, 파일 로드 실패 시 모델 생성을
  실패시키는 경로임을 확인했다.
- SourceRaw Weightmap을 독립적으로 다시 샘플링한 6개 393,216픽셀 검사에서
  strict `__DataLayer__ > 170`과 설치 diffuse 알파의 불일치는 0픽셀이었다.
- 최종 DDS normal 픽셀은 앞서 교차검증한 fixed normal과 6개 모두 동일하다.

기존 C4819 코드페이지 경고와 DirectXTK PDB 경고는 남지만 빌드 오류는 아니다.

## 남은 작업

- 현재 통합은 발탄 화면 지형 복원이다. 내비게이션 충돌 hole은 별도 nav/collision 입력에
  `__DataLayer__ > 170` top-left-owned quad 규칙을 적용해야 한다.
- 현재 glTF 지형의 quad 대각선은 UE3 원본 index buffer와 반대다. 이번 hole/정본 병합과
  섞지 않았으며, 원본 표면 정밀도를 맞추는 별도 검증 작업으로 남긴다.
- MapTool Save 버튼의 실제 수동 클릭과 재실행 확인은 Debug 창에서 한 번 더 수행한다.
- `Client/Bin/Resources`는 Git 미추적 공유 팩이므로 팀원 PC에도 최종 diffuse/normal
  12개를 동기화해야 한다.
