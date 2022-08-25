project "glfw"
	filename "glfw"
	kind "StaticLib"
	language "C++"
	targetdir "../libs/compiled_libs/glfw"
	location "../project_files/"

	includedirs {"../libs/glfw/include"}

	files
	{
		"../libs/glfw/include/GLFW/glfw3.h",
		"../libs/glfw/include/GLFW/glfw3native.h",
		"../libs/glfw/src/internal.h",
		"../libs/glfw/src/mappings.h",
		"../libs/glfw/src/null_joystick.h",
		"../libs/glfw/src/null_platform.h",
		"../libs/glfw/src/win32_platform.h",
		"../libs/glfw/src/win32_joystick.h",
		"../libs/glfw/src/win32_thread.h",
		"../libs/glfw/src/win32_time.h",

		"../libs/glfw/src/context.c",
		"../libs/glfw/src/egl_context.c",
		"../libs/glfw/src/init.c",
		"../libs/glfw/src/input.c",
		"../libs/glfw/src/monitor.c",
		"../libs/glfw/src/null_init.c",
		"../libs/glfw/src/null_joystick.c",
		"../libs/glfw/src/null_monitor.c",
		"../libs/glfw/src/null_window.c",
		"../libs/glfw/src/osmesa_context.c",
		"../libs/glfw/src/platform.c",
		"../libs/glfw/src/vulkan.c",
		"../libs/glfw/src/wgl_context.c",
		"../libs/glfw/src/window.c",

		"../libs/glfw/src/win32_init.c",
		"../libs/glfw/src/win32_joystick.c",
		"../libs/glfw/src/win32_module.c",
		"../libs/glfw/src/win32_monitor.c",
		"../libs/glfw/src/win32_thread.c",
		"../libs/glfw/src/win32_time.c",
		"../libs/glfw/src/win32_window.c",
	}

	defines 
	{
		"_GLFW_WIN32",
		"_CRT_SECURE_NO_WARNINGS",
	}


	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		symbols "on"
