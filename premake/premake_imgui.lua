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
			"../libs/imgui/backends/imgui_impl_opengl3.h",
			"../libs/imgui/backends/imgui_impl_glfw.h",
			"../libs/imgui/backends/imgui_impl_vulkan.h",
		},

		["Source Files"] = {
			"../libs/imgui/*.cpp",
			"../libs/imgui/backends/imgui_impl_opengl3.cpp",
			"../libs/imgui/backends/imgui_impl_glfw.cpp",
			"../libs/imgui/backends/imgui_impl_vulkan.cpp",
		}
	}

	files
	{
		"../libs/imgui/*.cpp",
		"../libs/imgui/*.h",

		"../libs/imgui/backends/imgui_impl_opengl3.h",
		"../libs/imgui/backends/imgui_impl_glfw.h",
		"../libs/imgui/backends/imgui_impl_vulkan.h",
		"../libs/imgui/backends/imgui_impl_opengl3.cpp",
		"../libs/imgui/backends/imgui_impl_glfw.cpp",
		"../libs/imgui/backends/imgui_impl_vulkan.cpp"
	}

	defines {
		"WIN32",
		"_WINDOWS"
	}
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
