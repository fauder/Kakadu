#!/usr/bin/env python3
# PreBuild.py
import argparse, os, shlex, subprocess, sys
from pathlib import Path

step_index = 0
step_count = 2

def run(description, argv):
    global step_index, step_count
    step_index += 1
    print(f"[Pre-Build {step_index}/{step_count}]: {description}",
          flush=True)  # <- ensure header appears before child output)
    try:
        subprocess.check_call(argv)
    except subprocess.CalledProcessError as e:
        print(f"[Pre-Build {step_index}/{step_count}] FAILED: {description} (exit {e.returncode})", file=sys.stderr,
              flush=True)  # <- ensure header appears before child output
        sys.exit(e.returncode)

def env_or_default(name, default=None):
    v = os.environ.get(name)
    if v is None:
        return default
    v = v.strip()
    return v if v else default

def main():
    parser = argparse.ArgumentParser(description="Kakadu pre-build driver")
    parser.add_argument("--solution-dir", default=env_or_default("SolutionDir"))
    parser.add_argument("--outdir", default=env_or_default("OutDir"))
    parser.add_argument("--configuration", default=env_or_default("Configuration"))
    parser.add_argument("--python", default=sys.executable)
    args = parser.parse_args()

    # Fallback: if no solution-dir provided, assume script's directory
    if args.solution_dir:
        sol = Path(args.solution_dir).resolve()
    else:
        sol = Path(__file__).resolve().parent

    outd = Path(args.outdir).resolve() if args.outdir else None
    
    # 1) Always execute: GenerateEngineAbsoluteAssetPath
    gen_script       = sol / "PreBuild_GenerateEngineAbsoluteAssetPath.py"
    engine_asset_dir = sol / "Engine" / "Engine" / "Asset"
    
    print()
    
    run(
        "Generating engine absolute asset path.",
        [args.python, str(gen_script), "--engine", str(engine_asset_dir)]
    )

    # 2) Conditionally execute: Copy PDB files
    cfg = (args.configuration or "").lower()
    if cfg == "debug" or "asan" in cfg:
        pdb_script = sol / "PreBuild_CopyPDBFiles.py"
        run(
            "Copying 3rd party lib. PDB files.",
            [args.python, str(pdb_script), str(sol) + os.sep, "x64", str(outd) + os.sep if outd else ""]
        )
        
    print()

if __name__ == "__main__":
    main()
