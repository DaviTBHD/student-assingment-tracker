$compiler = (Get-Command g++.exe -CommandType Application -ErrorAction SilentlyContinue |
    Select-Object -First 1).Source

if (-not $compiler) {
    $packages = Join-Path $env:LOCALAPPDATA 'Microsoft\WinGet\Packages'
    $compiler = Get-ChildItem -LiteralPath $packages -Directory -Filter 'BrechtSanders.WinLibs*' -ErrorAction SilentlyContinue |
        ForEach-Object { Join-Path $_.FullName 'mingw64\bin\g++.exe' } |
        Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } |
        Select-Object -First 1
}

if (-not $compiler) {
    Write-Error 'MinGW-w64 (g++.exe) was not found. Check that it is installed and available on PATH.'
    exit 1
}

$executable = Join-Path $PSScriptRoot 'tracker.exe'
$source = Join-Path $PSScriptRoot 'main.cpp'
& $compiler -std=c++17 -O2 -municode -mwindows -static -o $executable $source -lcomctl32 -lshell32 -lole32 -luuid -lgdi32
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Output "Build complete. Executable: $executable"
