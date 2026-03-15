#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

from geninc.runner import run

def main() -> int:
    ap = argparse.ArgumentParser(
        description="Generate mass-include headers for an ECS project."
    )
    ap.add_argument(
        "--config",
        default="script_config",
        help="Config file (relative to script dir unless absolute)",
    )
    ap.add_argument(
        "--watch", action="store_true", help="Watch for changes and regenerate"
    )
    ap.add_argument(
        "--interval",
        type=float,
        default=0.5,
        help="Watch poll interval seconds (default: 0.5)",
    )
    ap.add_argument(
        "--include-regex",
        action="append",
        default=[],
        help="Include only paths matching regex (repeatable)",
    )
    ap.add_argument(
        "--include-keyword",
        action="append",
        default=[],
        help="Include only paths containing keyword (repeatable)",
    )
    args = ap.parse_args()

    script_dir = Path(__file__).resolve().parent
    cfg_path = Path(args.config)
    cfg_path = cfg_path if cfg_path.is_absolute() else (script_dir / cfg_path)

    print("CONFIG PATH:", cfg_path)
    return run(
        script_dir=script_dir,
        cfg_path=cfg_path,
        watch=args.watch,
        interval=args.interval,
        include_regex_patterns=args.include_regex,
        include_keywords=args.include_keyword,
    )


if __name__ == "__main__":
    raise SystemExit(main())
