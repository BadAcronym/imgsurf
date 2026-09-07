if(Test-Path "./vendor/datasurf/clean.ps1")
{
    pushd "./vendor/datasurf/"
    &./clean.ps1
    popd
}

Write-Host "cleaning up imgsurf builds..." -Fore Yellow

if(Test-Path "./bin")
{
    rm "./bin/" -Recurse -Force
}

foreach($file in (gci *.o))
{
    if(Test-Path $file)
    {
        Remove-Item $file
    }
}

foreach($file in (gci *.exe))
{
    if(Test-Path $file)
    {
        Remove-Item $file
    }
}

foreach($file in (gci *.pdb))
{
    if(Test-Path $file)
    {
        Remove-Item $file
    }
}

Write-Host "cleaned imgsurf!`n" -Fore Green
