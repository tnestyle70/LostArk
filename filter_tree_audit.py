r"""filter_tree_audit.py — .vcxproj.filters의 논리 트리와 물리 폴더 배치를 대조 출력.

사용: python filter_tree_audit.py <path\to\Project.vcxproj.filters> [...]
출력: 필터 트리(파일 수 포함) + 물리 버킷 분포 + 이동 계획(filters 트리를 물리 정본으로 삼을 경우).
"""
import sys, os
sys.stdout.reconfigure(encoding="utf-8")
import xml.etree.ElementTree as ET
from collections import defaultdict

NS = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}
ITEM_TAGS = ["ClCompile", "ClInclude", "None", "FxCompile", "ResourceCompile", "Image", "Text"]


def audit(filters_path: str):
    proj_dir = os.path.dirname(os.path.abspath(filters_path))
    tree = ET.parse(filters_path)
    root = tree.getroot()

    declared = set()
    for f in root.iterfind(".//m:Filter[@Include]", NS):
        declared.add(f.get("Include"))

    by_filter = defaultdict(list)  # filter path -> [(filename, physical rel dir)]
    total = 0
    for tag in ITEM_TAGS:
        for item in root.iterfind(f".//m:{tag}[@Include]", NS):
            inc = item.get("Include")
            fnode = item.find("m:Filter", NS)
            fpath = fnode.text if fnode is not None and fnode.text else "(필터 없음/루트)"
            phys = os.path.normpath(os.path.join(proj_dir, inc))
            phys_dir = os.path.relpath(os.path.dirname(phys), os.path.dirname(proj_dir))
            by_filter[fpath].append((os.path.basename(inc), phys_dir))
            total += 1

    print(f"\n{'='*70}\n[{os.path.basename(filters_path)}]  항목 {total}개, 필터 폴더 {len(declared)}개 선언")
    print(f"{'='*70}")

    empty = declared - set(by_filter.keys())

    def depth(p):
        return p.count("\\")

    for fpath in sorted(by_filter.keys()):
        files = by_filter[fpath]
        indent = "  " * depth(fpath)
        label = fpath.split("\\")[-1]
        buckets = defaultdict(int)
        for _, d in files:
            buckets[d] += 1
        bucket_s = ", ".join(f"{k}:{v}" for k, v in sorted(buckets.items()))
        print(f"{indent}+ {label}  ({len(files)}개)  [물리: {bucket_s}]")

    if empty:
        print(f"\n  (빈 필터 선언 {len(empty)}개: {', '.join(sorted(empty))})")

    # 이동 계획: 필터 트리를 물리 정본으로 승격한다면
    plan_path = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                             os.path.basename(filters_path).replace(".vcxproj.filters", "_move_plan.txt"))
    with open(plan_path, "w", encoding="utf-8") as w:
        w.write("# 필터 트리 → 물리 폴더 승격 시 이동 계획 (현재물리 -> 제안물리/파일)\n")
        for fpath in sorted(by_filter.keys()):
            for name, phys_dir in sorted(by_filter[fpath]):
                w.write(f"{phys_dir}\\{name} -> {fpath}\\{name}\n")
    print(f"\n  이동 계획 저장: {plan_path}")


if __name__ == "__main__":
    for p in sys.argv[1:]:
        audit(p)
