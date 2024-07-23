param (
    [string]$buildMode = "Debug",
    [string]$installPrefix,
    [string]$moduleDestination
)

if (($buildMode -eq "Debug" -or $buildMode -eq "Release") -and ($installPrefix -ne "") -and ($moduleDestination -ne ""))
{
    vcvars64
    git clone --recursive https://github.com/KhronosGroup/Vulkan-ValidationLayers.git "$moduleDestination/modules/vulkan_validation"
    cmake -S "$moduleDestination/modules/vulkan_validation" -B "$moduleDestination/dependencies/windows/vulkan_validation" -G "Ninja" -DCMAKE_INSTALL_PREFIX="$installPrefix" -DUPDATE_DEPS=ON -DCMAKE_BUILD_TYPE="$buildMode"
    cmake --build "$moduleDestination/dependencies/windows/vulkan_validation" --config "$buildMode" --target install
}
else
{
    Write-Output "Invalid build type or install path. Please provide either 'Debug' or 'Release', a valid prefix path and a valid Module Destination"
}
