# Render Sample

This is a sample project for future projects to practice Graphics Programming in Vulkan and D3D12
Because the zero planning of a really simple engine is hard, I'm creating this sample to be later
used in a real Game Engine.

Considering that D3D12 and Vulkan are 2 of the 3 most complex graphics API, this repository will
help later on more complex features development.

# Build

This repository is built under cmake and is structured in following distribution (each `-` indicates
one level dir):

- `-cmake_helpers`: A submodule with a set of cmake helpers, going from dependencies tracing to installers
- `-installers`: A set of PowerShell, and futurely Bash, scripts to download the dependencies
- `-Render`: Default rendering library
- `-ShaderManager`: Library renponsible to compile shaders and generate metadata
- `-TargetView`: Executable, encapsulates all the application
- `-Utils`: Library with utilities, like the console or a file I/O

To build the repository, just select a preset, at this point, only the presets `x64-debug` and `x64-release`
are avaliable, and to build, just run `cmake --preset <preset_name>`, that all dependencies will be installed
and the project will be built.

Disclaimer: all the installers will be run at first time by cmake, so, the first build will take a several time,
and they will be build and install all programs. But, if the dependencies are installed in the prefix_path, the build
will be quick.

Disclaimer 2: you will need admin privileges on windows to run the vulkan version, it will record in HKEY_LOCAL_MACHINE,
the validation layer and the extensions, that requires the admin privilege, by default, this option will be disabled,
if you want to set the vulkan layers location in regedit, just run `cmake --preset <preset_name> -DREGISTER_VULKAN_LAYERS=ON`
