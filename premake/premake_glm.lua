project "glm"
	filename "glm"
	kind "StaticLib"
	language "C++"
	targetdir "../libs/compiled_libs/glm"
	location "../project_files/"

	includedirs {"../libs/glm/"}

	files
	{
		"../libs/glm/glm/**.hpp",
		"../libs/glm/glm/**.cpp",
		"../libs/glm/glm/**.inl"
	}

	defines {
		"WIN32",
		"_WINDOWS"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
