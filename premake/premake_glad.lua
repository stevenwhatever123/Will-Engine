project "glad"
	kind "StaticLib"
	language "C"
	architecture "x64"

	targetdir "../libs/compiled_libs/glad"
    
  	includedirs { "../libs/glad/include/" }

	files { "../libs/glad/src/glad.c" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"