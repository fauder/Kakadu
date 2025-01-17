import os
import subprocess
from collections import defaultdict # Provides dictionary of lists.

glslang_path = os.environ[ 'GLSLANG_PATH' ]
if glslang_path == None:
    print( 'Environment variable \"GLSLANG_PATH\" is not defined. Skipping post-build shader validation.' )
    exit()
    
glslang_validator_path = os.path.join( glslang_path, 'glslangValidator.exe' )
print( '\nPostBuild_ValidateShaders.bat: Validating all GLSL vertex and fragment shaders via glslangValidator.exe...' )

root_directory_path = os.path.dirname(os.path.realpath(__file__))
shaders_directory_path = os.path.join( root_directory_path, 'Asset\\Shader' )

allowed_shader_extensions = [ '.vert', '.frag' ]

def ValidateIndividualStages( shaders_stage_file_paths ):
    success = True;
    for shader_stage_file_path in shaders_stage_file_paths:
        result = subprocess.run( [ glslang_validator_path, shader_stage_file_path,  ], 
                                    capture_output = True, shell = True, text = True )
        error_code = result.returncode

        if error_code != 0:
            print( result.stdout )
            success = False
            
    return success
                
def ValidateCompletePrograms( shader_programs ):
    success = True;
    for shader_name, shader_stage_file_paths in shader_programs.items():
        result = subprocess.run( [ glslang_validator_path ] + shader_stage_file_paths + [ '-I' + shaders_directory_path, '-l' ], # -l = link; Consumes multiple stages of a program.
                                 capture_output = True, shell = True, text = True )
       
        # most_recently_processed_files = ''
        # for x in shader_stage_file_names:
        #     most_recently_processed_files += '\t' + x + '\n'
        
        error_code = result.returncode
        if error_code != 0:
            print( result.stdout )
            print( 'error: Shader "' + shader_name + '" could not be compiled as a whole.' )
            success = False
            
    return success

for subdir, dirs, files in os.walk(shaders_directory_path):
    shader_programs         = defaultdict( list )
    shader_stage_file_paths = []
    shader_names            = []
    
    # First collect all shader stages per shader name:
    for file in files:
        file_name_alone, file_extension = os.path.splitext( file )
        if any( file_extension in x for x in allowed_shader_extensions ):
            shader_programs[ file_name_alone ].append( os.path.join( shaders_directory_path, file ) )
            shader_stage_file_paths.append( os.path.join( shaders_directory_path, file ) )
            shader_names.append( file )
            
    # Then validate:
    
    # TODO: Enable this when Google's Shaderc is integrated, which has #include support. Glslang does not support #include for individual shader stages...
    # success = ValidateIndividualStages( shader_stage_file_paths )
    
    success = ValidateCompletePrograms( shader_programs )
    
    if success:
        print( 'PostBuild_ValidateShaders.bat: All shaders validated successfully.' )
    else:
        print( '\nError: PostBuild_ValidateShaders.bat: Some shaders have validation errors.' )