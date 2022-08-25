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
		"libs/glfw/include",
		"libs/assimp/include",
		"libs/glm/",
		"libs/glad/include",
		"libs/stb/",
		"libs/imgui/",
	}

	dependson
	{
		"glfw",
		"assimp",
		"glm",
		"glad",
		"imgui",
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
	}

	vpaths
	{
		["Precompiled Headers"]		= {"*.h", "*.cpp"},

		["Headers"]					= {"headers/*.h"},
		["Headers/Managers"]		= {"headers/Managers/*.h"},
		["Headers/Utils"]			= {"headers/Utils/*.h"},
		["Headers/Core"]			= {"headers/Core/*.h"},

		["Source Files"]			= {"src/*.cpp"},
		["Source Files/Managers"]	= {"src/Managers/*.cpp"},
		["Source Files/Utils"]		= {"src/Utils/*.cpp"},
		["Source Files/Core"]		= {"src/Core/*.cpp"},

		["Source Files/imgui"] = {
				"libs/imgui/*.cpp"
			}

	}

	files
	{
		"headers/**.h",
		"src/**.cpp",
		"*.h",
		"*.cpp",
	}

	pchheader "pch.h"
	pchsource "pch.cpp"

			postbuildcommands { '{COPYFILE} "%{wks.location}/libs/compiled_libs/assimp/Debug/assimp-vc143-mtd.dll" %{cfg.targetdir}'  }