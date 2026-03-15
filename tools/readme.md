# Tools

## Header Generator

### Overview

This tool automatically maintains aggregator headers for C/C++ projects. It scans for header files under Components/ and Systems/ and generates #include "..." lines. It only writes inside a marked auto-generated section so it’s safe and independent.

### What it updates

* For each resolved include root directory:

  * <include_root>/components.hpp
  * <include_root>/systems.hpp
* It scans:

  * <include_root>/Components/** (headers)
  * <include_root>/Systems/** (headers)

### Auto-generated section markers

* The tool will only replace text between these two lines:
  `// AUTO-GENERATED INCLUDES BEGIN (do not edit)`
  `// AUTO-GENERATED INCLUDES END (do not edit)`

Any content outside the markers is left untouched by the sciprt.

### How include paths are formed

* Generated includes are relative to the include root, for example:
  #include "Components/Sprite.h"
  #include "Systems/RenderSystem.h"
* Paths are normalized to forward slashes in the include string.

### Filters (include/exclude)

* Excludes are loaded from excludes.txt:

  * A plain text line is treated as a keyword exclusion (substring match).
  * A quoted line is treated as a regex exclusion (Python re).
  * Blank lines and lines starting with # are ignored.
* Optional include filters can be passed via CLI:
  --include-regex (repeatable)
  --include-keyword (repeatable)
* Rule behavior:

  * Excludes always win.
  * If any include filter exists, a file must match at least one include filter to be included.

### Configuration files

#### 1. script_config

* A key=value config file loaded by the tool (relative to the script directory unless absolute).
* Keys may be repeated to form lists.
* Values may be wrapped in double quotes; quotes are removed.

Common keys:

* search_base

  * Base directory used when resolving include_root_regex patterns.
  * Can be relative to the script directory or absolute.
* include_root_rel

  * One or more include roots given as a path.
  * Example: include_root_rel=../raylib_test/raylib_test/src/include
* include_root_regex

  * One or more regex patterns used to find include roots under search_base.
  * The regex matches normalized full directory paths using forward slashes.
  * Example: include_root_regex="raylib_test/raylib_test/src/include$"
* components_out / systems_out

  * Aggregator filenames (defaults: components.hpp / systems.hpp)
* components_dir / systems_dir

  * Directory names inside include root (defaults: Components / Systems)
* excludes_file

  * Path to excludes file (relative to script dir unless absolute)
* log_file

  * Path to append-only log file (relative to script dir unless absolute)
* case_sensitive_keywords

  * true/false: controls keyword matching case sensitivity

#### 2. excludes.txt

* One rule per line:

  * Plain text => keyword exclude
  * "quoted text" => regex exclude

**Example:**

```txt
Base
Impl
Detail
"^(Tests|test)/"
```

### Logging

* The tool logs to console and optionally to a file set by log_file in script_config.
* It writes a timestamped entry whenever it updates an aggregator file.
* In watch mode it typically stays quiet unless changes occur (other than initial startup messages).

### Watch mode

With --watch, the tool continuously monitors:

* script_config
* excludes file (if configured)
* scanned headers under Components/ and Systems/
* aggregator output files

On any change it regenerates and updates the aggregator headers if needed.

### Recommended usage

Run once:

> py gen_includes.py

Run continuously:

> py gen_includes.py --watch

Run with include filters:

> py tools\gen_includes.py --include-regex "Systems/.*System\.(h|hpp)$"

### Module overview

#### Entry point

* tools/gen_includes.py

  * Parses CLI args.
  * Resolves config path relative to the script directory.
  * Calls geninc.runner.run(...) and exits with its status code.

#### Core orchestration

* tools/geninc/runner.py

  * Loads script_config.
  * Sets up logging.
  * Loads excludes.txt (keywords + compiled regex).
  * Resolves include roots (possibly multiple).
  * For each include root:

    * scans headers
    * generates include lists
    * updates aggregator headers
  * Implements watch mode:

    * reloads config/excludes as needed
    * monitors file mtimes and triggers regeneration

#### Config parsing

* tools/geninc/config.py

  * read_kv_config(path): reads key=value lines, supports repeated keys, strips quotes.
  * parse_bool(value): parses common true/false representations.

#### Logging utility

* tools/geninc/logutil.py

  * setup_logger(log_file): configures timestamped console logging and optional file logging.

#### Excludes parsing

* tools/geninc/excludes.py

  * load_excludes_file(path): reads excludes.txt and returns:

    * exclude_keywords: list of plain text rules
    * exclude_regex: list of compiled regex patterns

#### Include root resolution

* tools/geninc/resolve.py

  * resolve_include_roots(script_dir, cfg): returns a list of resolved include roots from:

    * include_root_rel entries
    * include_root_regex entries searched under search_base
  * Normalizes paths for regex matching and deduplicates results.

#### Filesystem scanning + regex compile helper

* tools/geninc/scan.py

  * scan_headers(dir): recursive scan for .h/.hpp/.hh/.hxx
  * compile_regexes(patterns): compiles a list of regex strings.

#### Generation + update logic

* tools/geninc/update.py

  * generate_includes(...): creates sorted #include lines relative to include_root.
  * update_auto_section(file, lines): replaces only content between markers.
  * Contains filter logic (include/exclude) and path normalization.

#### \(watch.py\)

>**collect_mtimes(paths):**
> returns a stable snapshot of (path, mtime) pairs used to detect changes.

### Notes/gotchas

* Aggregator headers must contain the BEGIN/END markers or the tool will error for that file.
* include_root_regex should usually be anchored (e.g. .../src/include$) to avoid matching unintended directories.
* If multiple include roots match, the tool will update all of them.
