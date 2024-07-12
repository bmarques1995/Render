param (
    [string]$moduleDestination
)

if ($moduleDestination -ne "")
{
    Invoke-WebRequest "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.614.0" -OutFile "$moduleDestination/d3d12core.zip"
    Expand-Archive -Path "$moduleDestination/d3d12core.zip" -DestinationPath "$moduleDestination/D3D12Core"
}
else
{
    Write-Output "Invalid module destination"
}