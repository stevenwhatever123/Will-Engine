includedirs 
{
	"../libs/volk/",
	"../libs/vulkan/include/",
}

project "volk"
	filename "volk"
	kind "StaticLib"
	language "C++"
	targetdir "../libs/compiled_libs/volk"
	location "../project_files/"

	includedirs 
	{
		"../libs/volk/",
		"../libs/vulkan/include/",
	}

	files( "../libs/volk/*.c" )
	files( "../libs/volk/*.h" )

	defines {
		"WIN32",
		"_WINDOWS"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

project "vulkan"
	filename "vulkan"
	kind "Utility"

	location "../project_files/"

	files{ 
		"../libs/vulkan/include/vulkan/**.h*" ,
		"../libs/vulkan/include/vk_video/**.h*",
	}