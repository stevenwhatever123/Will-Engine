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
	}

	links
	{
		"glfw",
	}

	vpaths {

		['Header Files'] = {
			"../libs/imgui/*.h"
		},

		["Source Files"] = {
			"../libs/imgui/*.cpp",
			"../libs/imgui/backends/imgui_impl_opengl3.cpp",
			"../libs/imgui/backends/imgui_impl_glfw.cpp"
		}
	}

	files
	{
		"../libs/imgui/*.cpp",
		"../libs/imgui/*.h",
		"../libs/imgui/backends/imgui_impl_opengl3.cpp",
		"../libs/imgui/backends/imgui_impl_glfw.cpp"
	}

	defines {
		"WIN32",
		"_WINDOWS"
	}
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
