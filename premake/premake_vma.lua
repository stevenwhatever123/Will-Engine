project "vma"
  filename "vma"
  kind "StaticLib"
  targetdir "../libs/compiled_libs/vma"
  location "../project_files/"
  language "C++"

  includedirs 
  {
    "../libs/vulkan/include/",
  }

  filter "toolset:gcc or toolset:clang"
    buildoptions { 
      "-Wno-unused-variable",
      "-Wno-reorder"
    }
  filter {}

  files( "../libs/vma/src/*.cpp" )

  filter "configurations:Debug"
  runtime "Debug"
  symbols "on"