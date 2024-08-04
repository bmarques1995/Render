param (
    [string]$buildMode = "Debug",
    [string]$installPrefix,
    [string]$moduleDestination
)

if (($buildMode -eq "Debug" -or $buildMode -eq "Release") -and ($installPrefix -ne "") -and ($moduleDestination -ne ""))
{
    git clone --recursive -b vulkan-sdk-1.3.290 https://github.com/KhronosGroup/Vulkan-Headers.git "$moduleDestination/modules/vulkan_headers"
    cmake -S "$moduleDestination/modules/vulkan_headers" -B "$moduleDestination/dependencies/windows/vulkan_headers" -DCMAKE_INSTALL_PREFIX="$installPrefix" -DUPDATE_DEPS=ON
    cmake --build "$moduleDestination/dependencies/windows/vulkan_headers" --config "$buildMode" --target install
}
else
{
    Write-Output "Invalid build type or install path. Please provide either 'Debug' or 'Release', a valid prefix path and a valid Module Destination"
}
