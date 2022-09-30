#pragma once

// stb
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

// C++ library
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <thread>
#include <cstring>
#include <set>

// Windows
#include <Windows.h>

// Vulkan
#define VK_USE_PLATFORM_WIN32_KHR

// Volk
#include <volk.h>

// Vma
#include <vk_mem_alloc.h>

// Glslang
#include <SPIRV/GlslangToSpv.h>

// Glad
#include <glad/glad.h>

// GLFW
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/closest_point.hpp>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Imgui
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_vulkan.h>



// Types
// base typedefs
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

// Math & GLM
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::mat4 mat4;
typedef glm::mat3 mat3;
typedef glm::ivec3 ivec3;

using String = std::string;