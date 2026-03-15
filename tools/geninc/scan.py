from __future__ import annotations

import re
from pathlib import Path
from typing import List

HEADER_EXTS = {".h", ".hpp", ".hh", ".hxx"}

def scan_headers(root: Path) -> List[Path]: 
    files: List[Path] = []
    if not root.exists():
        return files
    for p in root.rglob("*"):
        if p.is_file() and p.suffix.lower() in HEADER_EXTS:
            files.append(p)
    return files


def scan_headers_with_glob(root: Path, pattern: str) -> List[Path]:
    if not root.exists():
        return []
    pat = (pattern or "").strip()
    if not pat:
        pat = "**/*.h*"  # safe default: .h, .hpp, .hh, .hxx

    return [p for p in root.glob(pat) if p.is_file()]


def compile_regexes(patterns: List[str]) -> List[re.Pattern]:
    return [re.compile(p) for p in patterns]
