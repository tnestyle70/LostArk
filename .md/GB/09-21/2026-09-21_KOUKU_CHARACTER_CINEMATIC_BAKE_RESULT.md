# 쿠크 연출 애니메이션 Character 본체 통합 결과

## G00. 실제 모델 조사

기존 Character 본체는 쿠크 `MN_RPCZ_00` 91개, 세이튼 `MN_RPCT_05` 249개, 큰 세이튼 `MN_RPCT_06` 34개 클립이었다. 세이튼의 `rpct00_evt2_rpct_showtime_01`은 이미 본체에 있었다. 첫 진입·1관문 연출도 본체에 합쳐져 있던 것이 아니라 별도 Map 파생 모델에 구워져 있었다.

Map 보스 파생 모델 11개 모두 대응 본체의 skeleton·material section과 바이트가 동일했다. 일부 1관문·2관문 클리어 파생 모델은 이전 mesh stride/geometry 복구 상태였다. 후보는 현재 Character 본체의 최신 geometry를 유지하고 WANM section만 추가하므로 기존 재질·mesh·rest·기존 애니메이션을 되돌리지 않는다.

## G01. 후보 생성

`Tools/KoukuSaydonPipeline/bake_character_cinematic_clips.py`로 다음 후보를 만들었다. 새 좌표계 변환·retarget·배율 조정은 하지 않았으며 이미 설치돼 있던 source-baked clip bytes를 그대로 넣었다.

| Character 본체 | 추가 연출 | 기존 → 후보 클립 수 |
|---|---|---|
| `MN_RPCZ_00` | 2관문 진입, 2관문 클리어, 카드미로 | 91 → 94 |
| `MN_RPCT_05` | 2관문 진입, 1관문 책/무대/피날레, 2관문 클리어, 3관문 진입, 빙고 앵콜 | 249 → 256 |
| `MN_RPCT_06` | 2관문 클리어 큰 세이튼 | 34 → 35 |

쇼타임은 기존 클립을 그대로 보존했다. 후보 경로는 `out/KoukuCharacterCinematicBake20260921/candidates/Character/...`이며 같은 폴더의 `receipt.json`에 원본·후보 SHA-256, donor ID/hash, stable object mapping 11개, 정확한 duration과 skeleton/키 검사가 기록돼 있다. 기존 source donor의 30fps 양자화 길이를 보존했다. 예를 들어 카드미로 클립은 359ticks/30Hz = 11.9667초이며 기존 Pattern duration 11.95초를 도구가 바꾸지 않는다.

## G02. 자동 검증

- 원본 모델의 각 기존 section payload·name·index를 그대로 보존하고 새 section/table만 추가했다. 각각 94/252/37개 기존 section을 대조했다.
- donor와 target의 skeleton/rest section 11개 byte-identical. 추가한 WANM도 donor와 byte-identical.
- 모든 donor 키의 유한성, 시간 범위·순서, quaternion 길이 검사를 통과했다.
- 후보에 같은 11개 클립을 다시 추가하면 addedCount=0이며 전체 파일이 byte-identical이다. 이름이 같고 내용이 다른 경우 명시적 오류로 거부한다.
- `test_bake_character_cinematic_clips.py` 4개 검사 PASS: 기존 보존·idempotence, 기존 이름 충돌, 입력끼리의 이름 충돌, 40byte truncation 충돌 차단.
- 실제 WARP CModel로 후보 본체와 donor를 로드해 11개 연출의 전체 30fps 샘플 10,171개에서 모든 본의 combined matrix를 비교했다. 23,587,616개 float 모두 유한, 최대 오차 0. 창 0개·Draw 0회. `out/KoukuCharacterCinematicBake20260921/model_probe.log`.
- 영속 source generator용 `install_baked_clip`은 원본 백업·재확인·동일 폴더 임시 파일·원자적 교체를 사용한다. 실제 원본 쿠크 모델을 candidate로 hardlink한 뒤 추가 설치해서 원본 bytes가 그대로이고 candidate만 별도 inode로 교체되는 것을 확인했다. 같은 이름의 변경된 내용은 교체하지 않는다.
- 수정 파일 Python 실행·diff check PASS. 새 제품 C++ 파일이나 project/filter 항목은 없다.

## G03. 통합과 화면 경계

후보와 native 검사 결과를 최종 통합 담당자에게 전달했다. Resources 실물 교체·Data 참조 변경·publisher·Debug build 결과는 통합 RESULT가 소유한다. 이 결과 문서는 그 작업을 선행 완료로 기록하지 않는다. candidate의 textures hardlink는 native 검사용이며 설치 대상은 WModel 3개다.

카메라·배경·월드 이동·가시성·게임플레이·원본 Effect는 이 도구의 소유가 아니다. WorldSequence의 stable ID와 시간·transform을 유지한 상태에서 modelAssetId가 본체를 참조하도록 연결해야 한다. 사용자 Client/아레나 실행과 실제 연출 합성 화면 판정은 수행하지 않았다.

## G04. 게시 전 경로 계약 재발 방지

최종 통합 담당자의 전체 domain publish가 성공적으로 끝난 뒤 `project_kouku_saydon_composition.py::_load_bone_bake_actor`의 공통 model loader를 수정했다. body·weapon·animationSet 모두 runtime `CActorCatalog::IsResourceId`와 같은 대소문자 구분 `Character/` prefix와 정확한 `.wmodel` extension을 요구한다. Resources 밖 경로와 기존 slash/traversal 검증도 유지했다. 별도 Character donor의 기존 skeleton 동일·clip 중복 거부·기존 clip 보존 계약은 그대로다.

이전 `test_card_maze_staging.py`의 Map donor 성공 fixture를 Character donor로 바꾸고, Map donor 파일을 읽기 전에 명시적으로 거부하는 회귀 검사를 추가했다. 세 모델 필드 각각에 Map prefix, 잘못된 Character 대소문자, 대문자 extension, 추가 extension의 12개 잘못된 경로를 대조했다. 해당 파일의 9개 테스트 PASS, 두 파일 diff check PASS. 수정 전 dirty 원문과 hash는 `out/KoukuCharacterCinematicBake20260921/projector-path-contract`에 보존하고 최신 원문 일치 확인 후 적용했다. 이 단계 뒤의 증분 재게시는 최종 통합 담당자가 수행한다.
