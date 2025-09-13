# PostBuild_ValidateShaders.py (v2)
import os
import sys
import argparse
import subprocess
from collections import defaultdict

VERSION = "2"

def Main():
    parser = argparse.ArgumentParser( description = "Validate/link GLSL shaders with glslangValidator." )
    parser.add_argument(
        "--scan-dir",
        required = True,
        help = "Directory to scan for .vert/.frag shader stages (single directory)."
    )
    parser.add_argument(
        "--include-dir",
        action = "append",
        default = [],
        help = "Directory to search for #include .glsl files (repeatable). "
               "If omitted, the scan dir is used. Scan dir is always first in search order."
    )
    parser.add_argument(
        "--version",
        action = "store_true",
        help = "Print script version and exit."
    )
    args = parser.parse_args()

    if args.version:
        print( VERSION )
        sys.exit( 0 )

    glslang_path = os.environ.get( "GLSLANG_PATH" )
    if glslang_path == None:
        print( "Environment variable \"GLSLANG_PATH\" is not defined. Skipping post-build shader validation.\n" )
        sys.exit( 0 )
        
    scan_root = os.path.abspath( args.scan_dir )

    glslang_validator_path = os.path.join( glslang_path, "glslangValidator.exe" )
    print( "\nPostBuild_ValidateShaders.py (v" + VERSION + "): Validating GLSL shaders in \"" + scan_root + "\" via glslangValidator..." )

    if not os.path.isdir( scan_root ):
        print( "No .vert/.frag found under: " + scan_root + "\n" )
        sys.exit( 0 )

    allowed_shader_extensions = [ ".vert", ".frag" ]

    # Collect shaders (group by stem relative to scan_root)
    shader_programs    = defaultdict( list )
    shader_directories = set()

    for subdir, _, files in os.walk( scan_root ):
        for file in files:
            file_name_alone, file_extension = os.path.splitext( file )
            if file_extension in allowed_shader_extensions:
                abs_path = os.path.join( subdir, file )
                rel_stem = os.path.splitext( os.path.relpath( abs_path, scan_root ) )[ 0 ]
                shader_programs[ rel_stem ].append( abs_path )
                shader_directories.add( os.path.abspath( subdir ) )

    # If no shaders found, succeed gracefully
    if len( shader_programs ) == 0:
        print( "No .vert/.frag found under: " + scan_root + "\n" )
        sys.exit( 0 )

    # Build include dirs
    include_dirs = [ scan_root ]
    for d in sorted( shader_directories ):
        if d not in include_dirs:
            include_dirs.append( d )
    for d in args.include_dir:
        abs_d = os.path.abspath( d )
        if abs_d not in include_dirs:
            include_dirs.append( abs_d )

    def ValidateCompletePrograms( shader_programs, include_dirs_for_cmd ):
        success      = True
        include_args = [ "-I" + inc for inc in include_dirs_for_cmd ]

        for shader_key, shader_stage_file_paths in shader_programs.items():
            cmd    = [ glslang_validator_path ] + shader_stage_file_paths + include_args + [ "-l" ]
            result = subprocess.run( cmd, capture_output = True, shell = True, text = True )

            if result.returncode != 0:
                if result.stdout:
                    print( result.stdout )
                if result.stderr:
                    print( result.stderr )
                print( "error: Shader program \"" + shader_key + "\" failed to link/validate.\n" )
                success = False

        return success

    ok = ValidateCompletePrograms( shader_programs, include_dirs )
    if ok:
        print( "PostBuild_ValidateShaders.py: All shaders validated successfully.\n" )
        sys.exit( 0 )
    else:
        print( "\nError: PostBuild_ValidateShaders.py: Some shaders have validation errors.\n" )
        sys.exit( 1 )

if __name__ == "__main__":
    Main()
