# Will-Engine

This is a hobby game engine created as a personal project with the WILL to make a change (heavily inspired by Xenoblade Chronicles btw). It includes features such as :
- A Vulkan Physically Based Renderer
- A Physically Based Rendering (PBR) Material System
- Unreal Engine inspired BRDF Shading Model
- Deferred Rendering
- Shadow Mapping (Only point light at the moment)
- Call of Duty's Compute Shader Bloom Post-Processing
- Entity Component System (ECS)
- Skeletal Mesh/Animation
- Normal Mapping
- Asset Loading

# Getting Started
Visual Studio 2019 or 2022 is recommended. This build works with Windows ONLY and is not fully tested.

<ins>1. Download the repository</ins>

Start by cloning the repository with `git clone --recursive https://github.com/stevenwhatever123/Will-Engine`.

<ins>2.Setting up the project</ins>

This project requires `premake5` for setting up visual studio project and its submodules. If you do not have premake5 installed you could download here: `https://premake.github.io/`.

`glslc` is also required as all shaders code included is compiled targed to my current working machine. You can download it from `https://github.com/google/shaderc` or from the `Vulkan SDK`

1. Run `git submodule update --init --recursive` to download and update all submodules required.
2. Modify the `imconfig.h` file in `libs/imgui` by adding the line `#define VK_NO_PROTOTYPES` in the end of the file.
3. Compile all shader code in `/shaders` folder using the command `glslc.exe [file_name].vert -o [file_name].vert.spv`
4. All file directory is currently hard-coded in the source file, changes on the file path have to be made.
5. Run premake5 with the command `./premake5.exe vs2022` to generate a Visual Studio solution and you should be good to go.

# Project Status

This project is still work in progress in a slow pace. Planning to work on sky light(skybox) and skeletal animation next.
  
Since I recently got a job, any progress in this project will stop and halt.
# References

These are some of the articles/paper that I have been referenced throughout the engine's development if you are curious:  

PBR and BRDF shading:
- [Real Shading in Unreal Engine 4](https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf) by Brian Karis, Epic Game
- [Specular BRDF Reference](http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html) by Brian Karis
- [Moving Frostbite to PBR](https://www.ea.com/frostbite/news/moving-frostbite-to-pb)

Game Engine Layout and Rendering Pipeline:
- [Behind the Pretty Frames: Resident Evil](https://mamoniem.com/behind-the-pretty-frames-resident-evil/) by Muhammad
- [DOOM Eternal - Graphics Study](https://simoncoenen.com/blog/programming/graphics/DoomEternalStudy.html) by Simon Coenen
- [DOOM (2016) - Graphics Study](https://www.adriancourreges.com/blog/2016/09/09/doom-2016-graphics-study/) by Adrian Courrèges
- [Metal Gear Solid V - Graphics Study](https://www.adriancourreges.com/blog/2017/12/15/mgs-v-graphics-study/) by Adrian Courrèges

Animation:
- [Fix Your Timestep!](https://gafferongames.com/post/fix_your_timestep/)

Post Processing:
- [Next Generation Post Processing In Call Of Duty: Advanced Warfare](http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare)

Others:
- [Stop Using Normal Matrix](https://lxjk.github.io/2017/10/01/Stop-Using-Normal-Matrix.html) by Eric Zhang
- [https://learnopengl.com/](https://learnopengl.com/)
- [https://ogldev.org/](https://ogldev.org/)
- Reddit and StackOverFlow of course


# Screenshots

UI Editor and BRDF Shading
![](./screenshots/ui_editor.PNG)

Normal Mapping
![](./screenshots/normal_mapping.gif)

Shadow Mapping
![](./screenshots/shadow_mapping.PNG)

Bloom
![](./screenshots/bloom.gif)

Skeletal Mesh/Animation and Node Hierarchy
![](./screenshots/Skeletal.gif)

Entity Component System
![](./screenshots/ecs.gif)
  
Deferred Rendering  
![](./screenshots/GBuffers.PNG)
  
PBR Material System  
![](./screenshots/materials.PNG)
