

externalproject "zlib"
	location "../libs/build_assimp/contrib/zlib/"
	filename "zlibstatic"
	kind "StaticLib"
	language "C++"
	targetdir "../libs/compiled_libs/assimp/Debug/"



externalproject "Assimp"
  location "../libs/build_assimp/code/"
  filename "assimp"
  kind "StaticLib"
  language "C++"


  config_file = io.readfile("../libs/assimp/include/assimp/config.h.in"):gsub("#cmakedefine .", "// fuck you ")
  io.writefile("../libs/assimp/include/assimp/config.h", config_file)

  targetdir "../libs/compiled_libs/assimp"

  if _ACTION ~= 'clean_up' and _ACTION ~= 'clean_up_full'  then
	  os.executef('cmake --log-level NOTICE -G %q -B%s -S%s -D %s -D %s -D %s -D %s -D %s',
		  "Visual Studio 17 2022",
		  "../libs/build_assimp/",
		  "../libs/assimp/",
		  "BUILD_SHARED_LIBS:BOOL=ON",
		  "ASSIMP_BUILD_ZLIB=ON",
		  'ASSIMP_ARCHIVE_OUTPUT_DIRECTORY:STRING=../../compiled_libs/assimp',
		  'ASSIMP_LIBRARY_OUTPUT_DIRECTORY:STRING=../../compiled_libs/assimp',
		  'ASSIMP_RUNTIME_OUTPUT_DIRECTORY:STRING=../../compiled_libs/assimp'
	  )
  end

  dependson {
	  "zlib"
  }

  filter "configurations:Debug"
	  runtime "Debug"
	  symbols "on"



