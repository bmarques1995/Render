param (
    [string]$moduleDestination
)

if ($moduleDestination -ne "")
{
    $url = "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.614.0"

    # Extract the middle version number using a regular expression
    if ($url -match "/(\d+)\.(\d+)\.0") {
        $middleVersion = $matches[2]
        $middleVersion | Out-File -FilePath "$moduleDestination/d3d12sdk_version.txt" -encoding UTF8
    } else {
        Write-Output "No match found"
    }
    Invoke-WebRequest "$url" -OutFile "$moduleDestination/d3d12core.zip"
    Expand-Archive -Path "$moduleDestination/d3d12core.zip" -DestinationPath "$moduleDestination/D3D12Core"
}
else
{
    Write-Output "Invalid module destination"
}