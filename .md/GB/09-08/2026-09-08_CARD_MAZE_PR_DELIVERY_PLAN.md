# 카드미로 PR·리소스 인계 계획

## 이번 작업 경계

2026-09-08 10:28:29 KST의 pull/fast-forward `f92178f0` 이후 미커밋 변경을
`codex/kouku-card-maze-0908`에서 main 대상 PR로 전달한다. Resources와 개인 파일,
백업, 빌드 산출물은 Git에 추가하지 않는다. 기존 기능 PLAN/RESULT는 각 단계 기록으로 보존한다.

## 배포 호환 수정

`Tools/CompositionPipeline/composition_pipeline.py`의 `_validate_camera_shot_source`에서
카메라 owner가 지원하는 `displayName`을 선택 필드로 허용하고 기존 bounded text 검사로
1~128 UTF-8 byte 이름을 검사한다. 현재 카드미로 카메라의 이름 때문에 Composition Validate가
실패한 것을 실제 재현했다. 다른 알 수 없는 필드 거부는 유지한다.

shot `_require_exact_fields`의 선택 필드를 다음 블록으로 교체한다.

```python
            ("cameraTrack", "follow", "displayName"),
```

동일 검사 직후, shot ID 검사 전에 다음 블록을 추가한다.

```python
        if "displayName" in shot:
            _require_bounded_display_text(
                shot["displayName"], f"{shot_context}.displayName", 128
            )
```

기존 `test_composition_pipeline.py`에 현재 camera source의 한국어 이름 수용과
null/숫자/빈 이름/초과 이름 거부 검사를 추가한다. 새 C++와 프로젝트 등록은 없다.

## 검증과 전달

기존 Product Debug 성공 기록을 확인하고 Server 카드미로 계약 검사와 protocol 범위 검사를
재실행한다. 카메라 이름 회귀 검사와 Composition Validate/Publish, 변경 JSON/XML parse,
diff check 후 생성된 composition과 receipt도 같은 커밋에 포함한다.

카드 병사 4폴더는 Drive 신규 전달 대상으로 먼저 안내한다. 새로 참조한 기존 세토 폴더와
문양 DDS 4개는 팀원의 기존 수령 여부를 확인할 의존 리소스로 구분한다.
화면 검증, 실제 4인 접속, 일반 캐릭터 망치 모션과 망원경 상자 모델은 완료로 주장하지 않는다.
