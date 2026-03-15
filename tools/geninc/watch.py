from __future__ import annotations

from pathlib import Path
from typing import Iterable, List, Tuple


def collect_mtimes(paths: Iterable[Path]) -> Tuple[Tuple[str, float], ...]:
    items: List[Tuple[str, float]] = []
    for p in paths:
        try:
            items.append((str(p), p.stat().st_mtime))
        except FileNotFoundError:
            items.append((str(p), -1.0))
    return tuple(sorted(items))
