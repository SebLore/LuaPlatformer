from __future__ import annotations

import re
import time
from pathlib import Path
from typing import List

from .config import read_ini_config, AppConfig, BundleConfig
from .logutil import setup_logger
from .excludes import load_excludes_file
from .resolve import resolve_include_roots
from .scan import compile_regexes, scan_headers
from .update import generate_includes, update_auto_section, BEGIN, END
from .watch import collect_mtimes


def create_aggregator(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        "#pragma once\n\n"
        "// AUTO-GENERATED INCLUDES BEGIN (do not edit)\n"
        "// AUTO-GENERATED INCLUDES END (do not edit)\n",
        encoding="utf-8",
        newline="\n",
    )


def run(
    script_dir: Path,
    cfg_path: Path,
    watch: bool,
    interval: float,
    include_regex_patterns: List[str],
    include_keywords: List[str],
) -> int:
    app: AppConfig = read_ini_config(cfg_path)
    g = app.global_cfg

    # logging
    log_file = None
    if g.log_file:
        p = Path(g.log_file)
        log_file = p if p.is_absolute() else (script_dir / p)
    logger = setup_logger(log_file)

    # excludes (global)
    excludes_path: Path | None = None
    exclude_keywords: list[str] = []
    exclude_regex: list[re.Pattern] = []

    if g.excludes_file:
        p = Path(g.excludes_file)
        excludes_path = p if p.is_absolute() else (script_dir / p)
        excludes_path = excludes_path.resolve()
        exclude_keywords, exclude_regex = load_excludes_file(excludes_path)
        logger.info(
            f"Loaded excludes: {len(exclude_keywords)} keywords, {len(exclude_regex)} regex from {excludes_path}"
        )

    include_regex = compile_regexes(include_regex_patterns)

    # cache per-bundle excludes by resolved path
    bundle_excludes_cache: dict[Path, tuple[list[str], list[re.Pattern]]] = {}

    def bundle_output_path(include_root: Path, folder: str, b: BundleConfig) -> Path:
        name = b.output_name_template.format(folder=folder, ext=b.output_ext)
        return (include_root / b.output_subdir / name).resolve()

    def get_bundle_excludes(b: BundleConfig) -> tuple[list[str], list[re.Pattern]]:
        # default to global excludes
        if not b.excludes_file:
            return exclude_keywords, exclude_regex

        p = Path(b.excludes_file)
        bf = p if p.is_absolute() else (script_dir / p)
        bf = bf.resolve()

        if bf not in bundle_excludes_cache:
            bundle_excludes_cache[bf] = load_excludes_file(bf)

        return bundle_excludes_cache[bf]

    def run_once(appcfg: AppConfig) -> None:
        roots = resolve_include_roots(script_dir, appcfg.global_cfg)
        if not roots:
            raise RuntimeError(
                "No include roots resolved.\n"
                "Set include_root_rel=... and/or include_root_regex=... in [global]."
            )

        for include_root in roots:
            for b in appcfg.bundles:
                scan_dir = (include_root / b.folder).resolve()
                out_path = bundle_output_path(include_root, b.folder, b)

                bundle_exclude_keywords, bundle_exclude_regex = get_bundle_excludes(b)

                if not out_path.exists():
                    if b.auto_create:
                        create_aggregator(out_path)
                        logger.info(f"Created aggregator {out_path}")
                    else:
                        logger.warning(f"Missing aggregator: {out_path}")
                        continue

                includes = generate_includes(
                    include_root=include_root,
                    scan_dir=scan_dir,
                    scan_glob=b.scan_glob,
                    out_header_abs=out_path,
                    include_regex=include_regex,
                    exclude_regex=bundle_exclude_regex,
                    include_keywords=include_keywords,
                    exclude_keywords=bundle_exclude_keywords,
                    case_sensitive_keywords=appcfg.global_cfg.case_sensitive_keywords,
                )

                if update_auto_section(out_path, includes):
                    logger.info(f"Updated {out_path}")
                elif not watch:
                    logger.info(f"No changes for {out_path}")

    if not watch:
        run_once(app)
        return 0

    logger.info("Watching for changes. Ctrl+C to stop.")
    prev = None

    try:
        while True:
            # reload config so edits take effect live
            app = read_ini_config(cfg_path)
            g = app.global_cfg

            # recompute global excludes_path in case script_config changed it
            new_excludes_path: Path | None = None
            if g.excludes_file:
                p = Path(g.excludes_file)
                new_excludes_path = p if p.is_absolute() else (script_dir / p)
                new_excludes_path = new_excludes_path.resolve()

            excludes_path = new_excludes_path

            # reload global excludes live too
            if excludes_path:
                exclude_keywords[:], exclude_regex[:] = load_excludes_file(
                    excludes_path
                )
            else:
                exclude_keywords[:] = []
                exclude_regex[:] = []

            roots = resolve_include_roots(script_dir, g)

            watch_paths: list[Path] = [cfg_path.resolve()]
            if excludes_path:
                watch_paths.append(excludes_path)

            for include_root in roots:
                for b in app.bundles:
                    out_path = bundle_output_path(include_root, b.folder, b)
                    watch_paths.append(out_path)
                    watch_paths += scan_headers(include_root / b.folder)

                    if b.excludes_file:
                        p = Path(b.excludes_file)
                        bf = p if p.is_absolute() else (script_dir / p)
                        watch_paths.append(bf.resolve())

            cur = collect_mtimes(watch_paths)
            if cur != prev:
                prev = cur
                bundle_excludes_cache.clear()
                run_once(app)

            time.sleep(interval)
    except KeyboardInterrupt:
        return 0

    return 0
