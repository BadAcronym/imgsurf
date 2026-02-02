$toDelete =
    "./build/",
    "./bin/imgsurftest_linux/",
    "./bin/imgsurftest_win64/",
    "./obj/",
    "./log/",
    "./compile_commands.json"

Write-Host "cleaning the build..."

foreach($folder in $toDelete)
{
    if(Test-Path $folder)
    {
        Remove-Item $folder -Recurse
    }
}

Write-Host "all clean!" -ForegroundColor Green
