project "imgui"
	filename "imgui"
	kind "StaticLib"
	language "C++"
	targetdir "../libs/compiled_libs/imgui"
	location "../project_files/"

	includedirs
	{
		"",
		"../libs/glfw/include",
		"../libs/imgui/",
		"../libs/vulkan/include/",
		"../libs/volk/",
	}

	dependson
	{
		"vulkan",
		"volk",
	}

	links
	{
		"glfw",
		"vulkan",
		"volk",
	}

	vpaths {

		['Header Files'] = {
			"../libs/imgui/*.h",
		},

		["Source Files"] = {
			"../libs/imgui/*.cpp",
		}
	}

	files
	{
		"../libs/imgui/*.cpp",
		"../libs/imgui/*.h",
	}

	defines {
		"WIN32",
		"_WINDOWS"
	}
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
