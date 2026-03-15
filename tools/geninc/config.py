from __future__ import annotations

import configparser
from dataclasses import dataclass
from pathlib import Path
from typing import List


def parse_bool(s: str, default: bool = False) -> bool:
    v = s.strip().lower()
    if v in ("1", "true", "yes", "on"):
        return True
    if v in ("0", "false", "no", "off"):
        return False
    return default


@dataclass(frozen=True)
class BundleConfig:
    name: str
    folder: str
    output_ext: str
    output_name_template: str
    output_subdir: str
    scan_glob: str
    excludes_file: str | None
    auto_create: bool


@dataclass(frozen=True)
class GlobalConfig:
    search_base: str
    include_root_rel: List[str]
    include_root_regex: List[str]
    excludes_file: str | None
    log_file: str | None
    case_sensitive_keywords: bool
    default_output_ext: str
    default_output_name_template: str
    default_output_subdir: str
    default_scan_glob: str
    auto_create_aggregators: bool


@dataclass(frozen=True)
class AppConfig:
    global_cfg: GlobalConfig
    bundles: List[BundleConfig]


def read_ini_config(path: Path) -> AppConfig:
    if not path.exists():
        raise FileNotFoundError(f"Config file not found: {path}")

    cp = configparser.ConfigParser(
        inline_comment_prefixes=(";", "#"),
    )
    cp.read(path, encoding="utf-8")

    if "global" not in cp:
        raise ValueError("script_config must contain a [global] section")

    g = cp["global"]

    default_output_ext = g.get("default_output_ext", "hpp").strip().lstrip(".")
    default_output_name_template = g.get(
        "default_output_name_template", "{folder}.{ext}"
    ).strip()
    default_output_subdir = g.get("default_output_subdir", ".").strip()
    default_scan_glob = g.get("default_scan_glob", fallback="**/*.h*").strip()

    global_cfg = GlobalConfig(
        search_base=g.get("search_base", ".."),
        include_root_rel=[
            s.strip() for s in g.get("include_root_rel", "").splitlines() if s.strip()
        ],
        include_root_regex=[
            s.strip() for s in g.get("include_root_regex", "").splitlines() if s.strip()
        ],
        excludes_file=g.get("excludes_file", fallback=None),
        log_file=g.get("log_file", fallback=None),
        case_sensitive_keywords=parse_bool(
            g.get("case_sensitive_keywords", "false"), default=False
        ),
        default_output_ext=default_output_ext,
        default_output_name_template=default_output_name_template,
        default_output_subdir=default_output_subdir,
        default_scan_glob=default_scan_glob,
        auto_create_aggregators=parse_bool(
            g.get("auto_create_aggregators", "false"), default=False
        ),
    )

    bundles: List[BundleConfig] = []
    for section in cp.sections():
        if not section.lower().startswith("bundle:"):
            continue

        name = section.split(":", 1)[1].strip()
        b = cp[section]

        folder = b.get("folder", fallback=None)
        if not folder:
            raise ValueError(f"[{section}] must define folder=...")

        output_ext = (
            b.get("output_ext", fallback=global_cfg.default_output_ext)
            .strip()
            .lstrip(".")
        )
        output_name_template = b.get(
            "output_name_template", fallback=global_cfg.default_output_name_template
        ).strip()
        output_subdir = b.get(
            "output_subdir", fallback=global_cfg.default_output_subdir
        ).strip()

        scan_glob = b.get("scan_glob", fallback="").strip()
        if not scan_glob:
            scan_glob = global_cfg.default_scan_glob
        if not scan_glob:
            scan_glob = "**/*.h*"

        
        excludes_file = b.get("excludes_file", fallback=None)
        auto_create = parse_bool(
            b.get("auto_create", fallback=str(global_cfg.auto_create_aggregators)),
            default=global_cfg.auto_create_aggregators,
        )

        bundles.append(
            BundleConfig(
                name=name,
                folder=folder.strip(),
                output_ext=output_ext,
                output_name_template=output_name_template,
                output_subdir=output_subdir,
                scan_glob=scan_glob,
                excludes_file=excludes_file,
                auto_create=auto_create,
            )
        )

    if not bundles:
        raise ValueError("script_config must define at least one [bundle:...] section")

    return AppConfig(global_cfg=global_cfg, bundles=bundles)
