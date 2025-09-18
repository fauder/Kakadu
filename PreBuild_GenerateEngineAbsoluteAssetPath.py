# PreBuild_GenerateEngineAbsoluteAssetPath.py
#
# Generates: <Engine/Engine>/Generated/EngineAssetAbsolutePath.h
# Contains:  absolute path to Engine/Asset as ENGINE_ASSET_PATH_ABSOLUTE

import argparse
import os
import pathlib
import sys

def main() -> int:
    ap = argparse.ArgumentParser(description="Generate EngineAssetAbsolutePath.h with absolute Engine asset path.")
    ap.add_argument("--engine", required=True, help="Path to Engine/Engine/Asset (VS macros are expanded by MSBuild).")
    args = ap.parse_args()

    engine_dir = pathlib.Path(os.path.expandvars(os.path.expanduser(args.engine))).resolve()
    if not engine_dir.exists():
        print(f"ERROR: Engine asset dir not found: {engine_dir}", file=sys.stderr)
        return 1

    # project_dir = .../Engine/Engine
    # engine_dir  = .../Engine/Engine/Asset  
    try:
        project_dir = engine_dir.parents[1]
    except IndexError:
        project_dir = engine_dir

    out_dir = project_dir / "Generated"
    out_dir.mkdir(parents=True, exist_ok=True)

    abs_posix = engine_dir.as_posix()

    header = f"""#pragma once

#define ENGINE_ASSET_ROOT_ABSOLUTE R"({abs_posix})"
"""
    (out_dir / "EngineAssetAbsolutePath.h").write_text(header, encoding="utf-8")

    print(f"Generated: {(out_dir / 'EngineAssetAbsolutePath.h')}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
