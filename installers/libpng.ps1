param (
    [string]$buildMode = "Debug",
    [string]$installPrefix,
    [string]$moduleDestination
)
if (($buildMode -eq "Debug" -or $buildMode -eq "Release") -and ($installPrefix -ne "") -and ($moduleDestination -ne ""))
{
    git clone --recursive https://git.code.sf.net/p/libpng/code "$moduleDestination/modules/libpng"
    Set-Location "$moduleDestination/modules/libpng"
    git reset --hard ed217e3e601d8e462f7fd1e04bed43ac42212429
    Set-Location "$moduleDestination"
    cmake -S "$moduleDestination/modules/libpng" -B "$moduleDestination/dependencies/windows/libpng" -DCMAKE_INSTALL_PREFIX="$installPrefix"
    cmake --build "$moduleDestination/dependencies/windows/libpng" --config "$buildMode" --target install
}
else
{
    Write-Output "Invalid build type or install path. Please provide either 'Debug' or 'Release', a valid prefix path and a valid Module Destination"
}
