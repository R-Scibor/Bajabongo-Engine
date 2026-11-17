#!/usr/bin/env python3
"""Summarize gcov data for src/ after a BAJA_COVERAGE build and ctest run."""

from __future__ import annotations

import argparse
import gzip
import json
import subprocess
import sys
from pathlib import Path


def iter_gcda(build_dir: Path) -> list[Path]:
    return sorted(p for p in build_dir.rglob("*.gcda") if "vcpkg" not in p.parts and "vcpkg_installed" not in p.parts)


def load_json(payload: bytes) -> dict:
    if payload[:2] == b"\x1f\x8b":
        payload = gzip.decompress(payload)
    return json.loads(payload.decode())


def is_project_source(path: Path, src_root: Path) -> bool:
    try:
        path.resolve().relative_to(src_root.resolve())
        return True
    except ValueError:
        return False


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", required=True, type=Path, help="Repository root")
    parser.add_argument("--build", required=True, type=Path, help="Coverage build directory")
    parser.add_argument("--out", required=True, type=Path, help="Output directory for the report")
    args = parser.parse_args()

    src_root = (args.root / "src").resolve()
    build_dir = args.build.resolve()
    out_dir = args.out.resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    gcda_files = iter_gcda(build_dir)
    if not gcda_files:
        print("No .gcda files found. Build with BAJA_COVERAGE=ON and run the tests first.", file=sys.stderr)
        return 1

    files: dict[str, dict[int, int]] = {}
    functions: dict[str, dict[str, int]] = {}

    for gcda in gcda_files:
        result = subprocess.run(
            ["gcov", "-j", "-t", "-b", str(gcda)],
            cwd=gcda.parent,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        if result.returncode != 0 or not result.stdout:
            print(f"gcov failed for {gcda}: {result.stderr.decode(errors='replace')}", file=sys.stderr)
            continue
        data = load_json(result.stdout)
        for entry in data.get("files", []):
            raw = Path(entry.get("file", ""))
            if not raw.is_absolute():
                raw = (gcda.parent / raw)
            if not is_project_source(raw, src_root):
                continue
            rel = str(raw.resolve().relative_to(args.root.resolve()))
            line_map = files.setdefault(rel, {})
            for line in entry.get("lines", []):
                number = int(line["line_number"])
                count = int(line.get("count", 0))
                line_map[number] = line_map.get(number, 0) + count
            fn_map = functions.setdefault(rel, {})
            for fn in entry.get("functions", []):
                name = fn.get("name") or fn.get("demangled_name") or "?"
                fn_map[name] = fn_map.get(name, 0) + int(fn.get("execution_count", fn.get("count", 0)))

    if not files:
        print("gcov produced no records under src/.", file=sys.stderr)
        return 1

    total_lines = 0
    covered_lines = 0
    total_fns = 0
    covered_fns = 0
    per_file = []

    for rel in sorted(files):
        line_map = files[rel]
        file_total = len(line_map)
        file_covered = sum(1 for c in line_map.values() if c > 0)
        fn_map = functions.get(rel, {})
        fn_total = len(fn_map)
        fn_covered = sum(1 for c in fn_map.values() if c > 0)
        total_lines += file_total
        covered_lines += file_covered
        total_fns += fn_total
        covered_fns += fn_covered
        line_pct = (100.0 * file_covered / file_total) if file_total else 0.0
        fn_pct = (100.0 * fn_covered / fn_total) if fn_total else 0.0
        per_file.append((rel, file_covered, file_total, line_pct, fn_covered, fn_total, fn_pct))

    line_pct = (100.0 * covered_lines / total_lines) if total_lines else 0.0
    fn_pct = (100.0 * covered_fns / total_fns) if total_fns else 0.0

    summary = [
        f"Source root: src/",
        f"Build: {build_dir}",
        f"Lines: {covered_lines}/{total_lines} ({line_pct:.1f}%)",
        f"Functions: {covered_fns}/{total_fns} ({fn_pct:.1f}%)",
        "",
        f"{'Lines':>16}  {'Funcs':>16}  File",
    ]
    for rel, lc, lt, lp, fc, ft, fp in per_file:
        summary.append(f"{lc:>7}/{lt:<7} {lp:5.1f}%  {fc:>7}/{ft:<7} {fp:5.1f}%  {rel}")
    text = "\n".join(summary) + "\n"
    (out_dir / "summary.txt").write_text(text)
    (out_dir / "summary.json").write_text(
        json.dumps(
            {
                "lines_covered": covered_lines,
                "lines_total": total_lines,
                "line_percent": round(line_pct, 1),
                "functions_covered": covered_fns,
                "functions_total": total_fns,
                "function_percent": round(fn_pct, 1),
            },
            indent=2,
        )
        + "\n"
    )
    print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
