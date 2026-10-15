#!/usr/bin/env python3
"""PowerShell JSON 변환 전에 중복 key·비표준 숫자를 검사한다. 입력 파일은 변경하지 않는다."""

from __future__ import annotations

# object_pairs_hook으로 dict 병합 전에 중첩 object의 중복 key를 확인한다.
import json
# 명령행으로 받은 JSON 경로를 UTF-8 또는 UTF-8 BOM으로 읽는다.
import pathlib
# 파일 경로 인자·오류 출력·종료 코드를 PowerShell 호출자와 주고받는다.
import sys


class DuplicateJsonKeyError(ValueError):
    pass


# 중복 key가 조용히 덮이지 않게 object를 만드는 시점에 예외를 발생시킨다.
def reject_duplicate_keys(pairs: list[tuple[str, object]]) -> dict[str, object]:
    result: dict[str, object] = {}
    for key, value in pairs:
        if key in result:
            raise DuplicateJsonKeyError(f"duplicate JSON object key: {key}")
        result[key] = value
    return result


# 표준 JSON 숫자가 아닌 NaN·Infinity를 거절한다.
def reject_nonstandard_constant(value: str) -> None:
    raise ValueError(f"non-standard JSON constant: {value}")


# 인자 오류는 2, 읽기·파싱 실패는 1, 정상은 0으로 호출자에게 알린다.
def main() -> int:
    if len(sys.argv) != 2:
        print("usage: assert_no_duplicate_json_keys.py <json-path>", file=sys.stderr)
        return 2

    source_path = pathlib.Path(sys.argv[1])
    try:
        source_text = source_path.read_text(encoding="utf-8-sig")
        json.loads(
            source_text,
            object_pairs_hook=reject_duplicate_keys,
            parse_constant=reject_nonstandard_constant,
        )
    except (OSError, UnicodeError, json.JSONDecodeError, ValueError) as error:
        print(f"strict JSON validation failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
