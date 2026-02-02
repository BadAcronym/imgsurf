param
(
    $build = "release",
    [switch]$compile_only = $false
)

if(-Not(Test-Path ".\obj\"))
{
    &mkdir .\obj\
}

if(-Not(Test-Path ".\build\"))
{
    &mkdir .\build\
}

if(-Not(Test-Path ".\bin\"))
{
    &mkdir .\bin\
}

function Get-Compileprep()
{
    Write-Host ""
    Write-Host "\033[36mcompiling imgsurf...\033[0m"
    Write-Host ""
    premake5 ecc
    premake5 vs2022
}

if($build -eq "debug")
{
    Get-Compileprep
    pushd ".\build\"
    &MSBuild imgsurf.sln -p:Configuration=$build -p:Platform=windows
}
elseif($build -eq "release")
{
    Get-Compileprep
    pushd ".\build\"
    &MSBuild imgsurf.sln -p:Configuration=$build -p:Platform=windows
}
elseif($build -eq "asan")
{
    Get-Compileprep
    pushd ".\build\"
    &MSBuild imgsurf.sln -p:Configuration=$build -p:Platform=windows
}
else
{
    Write-Host "\033[31m\nERROR: invalid make config: $build.\033[0m"
    exit -2;
}

if(0 -ne $LASTEXITCODE)
{
    Write-Host "\033[31m\nERROR: failed to compile.\n\033[0m"
    popd
    exit -1
}

Write-Host "\n"

if($compile_only)
{
    exit 0
}

popd

if(0 -eq $LASTEXITCODE -and -not $compile_only)
{
    $target = ".\bin\$build\imgsurftest.exe"
    Write-Host "`nrunning $target..."
    Invoke-Expression $target
}
