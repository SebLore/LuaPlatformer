from __future__ import annotations

import re
from pathlib import Path
from typing import List

from .scan import scan_headers_with_glob

BEGIN = "// AUTO-GENERATED INCLUDES BEGIN (do not edit)"
END = "// AUTO-GENERATED INCLUDES END (do not edit)"


def norm_slashes(p: str) -> str:
    """Normalize slashes between Linux and Windows"""
    return p.replace("\\", "/")


def matches_any_regex(s: str, regs: List[re.Pattern]) -> bool:
    return any(r.search(s) for r in regs)


def matches_any_keyword(s: str, keywords: List[str], case_sensitive: bool) -> bool:
    if not keywords:
        return False
    hay = s if case_sensitive else s.lower()
    for kw in keywords:
        needle = kw if case_sensitive else kw.lower()
        if needle in hay:
            return True
    return False


def should_include(
    rel_path: str,
    include_regex: List[re.Pattern],
    exclude_regex: List[re.Pattern],
    include_keywords: List[str],
    exclude_keywords: List[str],
    case_sensitive_keywords: bool,
) -> bool:
    if exclude_regex and matches_any_regex(rel_path, exclude_regex):
        return False
    if exclude_keywords and matches_any_keyword(
        rel_path, exclude_keywords, case_sensitive_keywords
    ):
        return False

    if include_regex or include_keywords:
        if include_regex and matches_any_regex(rel_path, include_regex):
            return True
        if include_keywords and matches_any_keyword(
            rel_path, include_keywords, case_sensitive_keywords
        ):
            return True
        return False

    return True


def generate_includes(
    include_root: Path,
    scan_dir: Path,
    scan_glob: str,
    out_header_abs: Path,
    include_regex: List[re.Pattern],
    exclude_regex: List[re.Pattern],
    include_keywords: List[str],
    exclude_keywords: List[str],
    case_sensitive_keywords: bool,
) -> List[str]:

    # Only scan headers matching scan_glob under scan_dir
    all_headers = scan_headers_with_glob(scan_dir, scan_glob)
    includes: List[str] = []

    for hp in all_headers:
        if hp.resolve() == out_header_abs.resolve():
            continue

        rel = hp.relative_to(include_root)
        rel_str = norm_slashes(str(rel))

        if not should_include(
            rel_str,
            include_regex,
            exclude_regex,
            include_keywords,
            exclude_keywords,
            case_sensitive_keywords,
        ):
            continue

        includes.append(f'#include "{rel_str}"')

    includes.sort()
    return includes


def update_auto_section(file_path: Path, include_lines: List[str]) -> bool:
    import re

    text = file_path.read_text(encoding="utf-8", errors="replace")

    # --- Ensure markers exist (auto-insert if missing) ---
    if BEGIN not in text or END not in text:
        lines = text.splitlines(keepends=True)

        insert_at = 0
        for i, line in enumerate(lines):
            if line.strip() == "#pragma once":
                insert_at = i + 1
                break

        marker_block = "\n" + f"{BEGIN}\n" + f"{END}\n"
        lines.insert(insert_at, marker_block)
        text = "".join(lines)

    # --- Collect includes OUTSIDE the managed section (manual includes) ---
    pre, rest = text.split(BEGIN, 1)
    managed_and_post = rest
    managed, post = managed_and_post.split(END, 1)

    include_re = re.compile(
        r'^\s*#\s*include\s+"([^"]+)"\s*(?://.*)?\s*$', re.MULTILINE
    )

    # Anything included in pre or post is considered "manual / already present"
    manual_targets = set(include_re.findall(pre)) | set(include_re.findall(post))

    # --- Filter generated include_lines: if already present manually, don't add it ---
    filtered_include_lines: List[str] = []
    for line in include_lines:
        s = line.strip()
        if s.startswith('#include "') and s.endswith('"'):
            target = s[len('#include "') : -1]
            if target in manual_targets:
                continue
        filtered_include_lines.append(line)

    # --- Write the managed section ---
    new_middle = "\n" + (
        "\n".join(filtered_include_lines) + ("\n" if filtered_include_lines else "")
    )
    new_text = pre + BEGIN + new_middle + END + post

    if new_text != text:
        file_path.write_text(new_text, encoding="utf-8", newline="\n")
        return True

    return False
