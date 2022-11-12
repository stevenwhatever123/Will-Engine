newaction {
	trigger = "clean_up",
	description = "Clean up files gen",
	execute = function()
		os.rmdir('build')
		os.rmdir('obj')
		os.rmdir('project_files')
		os.rmdir('x64')
		os.rmdir('.vs')
		os.remove('*.sln')
		os.remove('*.vcxproj*')
	end
}


workspace "WillEngine"
	configurations {"Debug", "Release"}
	architecture "x64"
	startproject "WillEngine"

include "premake/premake_glfw.lua"
include "premake/premake_assimp.lua"
include "premake/premake_glm.lua"
include "premake/premake_glad.lua"

include "premake/premake_volk.lua"
include "premake/premake_vma.lua"
include "premake/premake_glslang.lua"

include "premake/premake_imgui.lua"

project "WillEngine"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	targetdir "bin/%{cfg.buildcfg}"

	defines {"NOMINMAX"}
	disablewarnings {"4005"}

	filter "configurations:Debug"
	defines {"DEBUG"}
	symbols "on"

	includedirs
	{
		"",
		"headers",
		"libs/glfw/include/",
		"libs/assimp/include/",
		"libs/glm/",
		"libs/glad/include/",
		"libs/stb/",
		"libs/imgui/",
		"libs/volk/",
		"libs/vulkan/include/",
		"libs/vma/include/",
		"libs/glslang/"
	}

	dependson
	{
		"glfw",
		"assimp",
		"glm",
		"glad",
		"imgui",
		"volk",
		"vulkan",
		"vma",
		"glslang",
	}

	links
	{
		"user32",
		"opengl32",
		"glfw",
		"assimp",
		"glm",
		"glad",
		"imgui",
		"volk",
		"vulkan",
		"vma",
		"glslang",
	}

	vpaths
	{
		["Precompiled Headers"]				= {"*.h", "*.cpp"},

		["Headers"]							= {"headers/*.h"},
		["Headers/Managers"]				= {"headers/Managers/*.h"},
		["Headers/Utils"]					= {"headers/Utils/*.h"},
		["Headers/Core"]					= {"headers/Core/*.h"},
		["Headers/Core/OpenGL"]				= {"headers/Core/OpenGL/*.h"},
		["Headers/Core/Vulkan"]				= {"headers/Core/Vulkan/*.h"},
		["Headers/Core/EngineGui"]			= {"headers/Core/EngineGui/*.h"},

		["Source Files"]					= {"src/*.cpp"},
		["Source Files/Managers"]			= {"src/Managers/*.cpp"},
		["Source Files/Utils"]				= {"src/Utils/*.cpp"},
		["Source Files/Core"]				= {"src/Core/*.cpp"},
		["Source Files/Core/OpenGL"]		= {"src/Core/OpenGL/*.cpp"},
		["Source Files/Core/Vulkan"]		= {"src/Core/Vulkan/*.cpp"},
		["Source Files/Core/EngineGui"]		= {"src/Core/EngineGui/*.cpp"},

		["Shaders"]							= {"shaders/**.vert"},
		["Shaders"]							= {"shaders/**.frag"},
		["Shaders"]							= {"shaders/**.geom"},

	}

	files
	{
		"headers/**.h",
		"src/**.cpp",
		"*.h",
		"*.cpp",
		"shaders/**.vert",
		"shaders/**.frag",
		"shaders/**.geom",
	}

	pchheader "pch.h"
	pchsource "pch.cpp"

	postbuildcommands { '{COPYFILE} "%{wks.location}/libs/compiled_libs/assimp/Debug/assimp-vc143-mtd.dll" %{cfg.targetdir}'  }