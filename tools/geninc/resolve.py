from __future__ import annotations

import re
from pathlib import Path
from typing import List

from .scan import compile_regexes
from .config import GlobalConfig


def norm_slashes(p: str) -> str:
    return p.replace("\\", "/")


def matches_any_regex(s: str, regs: List[re.Pattern]) -> bool:
    return any(r.search(s) for r in regs)


def resolve_include_roots(script_dir: Path, g: GlobalConfig) -> List[Path]:
    roots: List[Path] = []

    for rel in g.include_root_rel:
        p = Path(rel)
        abs_p = p if p.is_absolute() else (script_dir / p).resolve()
        roots.append(abs_p)

    if g.include_root_regex:
        base = Path(g.search_base)
        base = base if base.is_absolute() else (script_dir / base)
        base = base.resolve()

        compiled = compile_regexes(g.include_root_regex)
        for d in base.rglob("*"):
            if not d.is_dir():
                continue
            s = norm_slashes(str(d))
            if matches_any_regex(s, compiled):
                roots.append(d.resolve())

    # dedupe preserve order
    seen = set()
    uniq: List[Path] = []
    for r in roots:
        if r not in seen:
            seen.add(r)
            uniq.append(r)
    return uniq
