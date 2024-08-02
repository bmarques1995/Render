param (
    [string]$buildMode = "Debug",
    [string]$installPrefix,
    [string]$moduleDestination
)

if (($buildMode -eq "Debug" -or $buildMode -eq "Release") -and ($installPrefix -ne "") -and ($moduleDestination -ne ""))
{
    git clone --recursive https://gitlab.com/libeigen/eigen.git "$moduleDestination/modules/eigen"
    cmake -S "$moduleDestination/modules/eigen" -B "$moduleDestination/dependencies/windows/eigen" -DCMAKE_INSTALL_PREFIX="$installPrefix"
    cmake --build "$moduleDestination/dependencies/windows/eigen" --config "$buildMode" --target install
}
else
{
    Write-Output "Invalid build type or install path. Please provide either 'Debug' or 'Release', a valid prefix path and a valid Module Destination"
}
