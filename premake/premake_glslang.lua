externalproject "glslang"
  location "../libs/build_glslang/glslang"
  filename "glslang"
  kind "StaticLib"
  language "C++"

  targetdir "../libs/compiled_libs/glslang"

  if _ACTION ~= 'clean_up' and _ACTION ~= 'clean_up_full'  then
	  --os.executef('cmake --log-level NOTICE -G %q -B%s -S%s -D %s -D %s -D %s -D %s -D %s',
	--	  "Visual Studio 17 2022",
	--	  "../libs/build_assimp/",
	--	  "../libs/assimp/",
	--	  "BUILD_SHARED_LIBS:BOOL=ON",
	--	  "ASSIMP_BUILD_ZLIB=ON",
	--	  'ASSIMP_ARCHIVE_OUTPUT_DIRECTORY:STRING=../../compiled_libs/assimp',
	--	  'ASSIMP_LIBRARY_OUTPUT_DIRECTORY:STRING=../../compiled_libs/assimp',
	--	  'ASSIMP_RUNTIME_OUTPUT_DIRECTORY:STRING=../../compiled_libs/assimp'
	  --)

	  os.executef('cmake ../libs/glslang/ -B%s -DCMAKE_INSTALL_PREFIX="$(pwd)/install"',
	  	"../libs/build_glslang/")
  end

  filter "configurations:Debug"
	  runtime "Debug"
	  symbols "on"
