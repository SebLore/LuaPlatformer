from __future__ import annotations

import re
from pathlib import Path
from typing import List, Tuple


def compile_regexes(patterns: List[str]) -> List[re.Pattern]:
    return [re.compile(p) for p in patterns]


def load_excludes_file(excludes_path: Path) -> Tuple[List[str], List[re.Pattern]]:
    """
    Each non-empty, non-comment line:
      - if wrapped in double-quotes => regex (quotes removed)
      - else => keyword
    """
    if not excludes_path.exists():
        raise FileNotFoundError(f"Excludes file not found: {excludes_path}")

    keywords: List[str] = []
    regexes_raw: List[str] = []

    for raw in excludes_path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue

        if len(line) >= 2 and line[0] == '"' and line[-1] == '"':
            regexes_raw.append(line[1:-1])
        else:
            keywords.append(line)

    return keywords, compile_regexes(regexes_raw)
