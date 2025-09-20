# PreBuild_CopyPDBFiles.py
import argparse
from pathlib import Path
import shutil
import sys

def main():
    ap = argparse.ArgumentParser(description="Copy PDBs.")
    ap.add_argument("solution_dir")
    ap.add_argument("platform")
    ap.add_argument("out_dir")
    args = ap.parse_args()

    solution_dir = Path(args.solution_dir)
    out_dir      = Path(args.out_dir)       # Do NOT create if missing.

    src_glfw   = solution_dir / "Lib" / f"{args.platform}-Debug" / "glfw3.pdb"
    src_vendor = solution_dir / "Bin" / f"{args.platform}-Debug" / "Vendor" / "Vendor.pdb"

    # Fail fast if anything is missing:
    for p in (src_glfw, src_vendor, out_dir):
        if not p.exists():
            print(f"Missing: {p}", file=sys.stderr)
            return 1

    # Overwrite like xcopy /Y:
    try:
        shutil.copy2(src_glfw, out_dir)
        shutil.copy2(src_vendor, out_dir)
    except Exception as e:
        print(f"Copy failed: {e}", file=sys.stderr)
        return 1
        
        
    print( "Done." )
    return 0

if __name__ == "__main__":
    sys.exit(main())
