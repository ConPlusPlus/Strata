# build.ps1 - build all Strata compiler artifacts into compiler\bin\ :
#     stratac.exe    front-end #1 (the compiler CLI: run, build, emit, ast, tokens)
#     console.exe    front-end #2 (the explorer console)
#     libstrata.dll  the compiler CORE as a shared library (proof of the exe/lib/LSP
#                    split in ARCHITECTURE.md sec 11 - exports lex, parse_program, ...)
#
# The two exes and the DLL are all built from the SAME core.dmm; only the front-end
# (or lack of one) differs. Run from anywhere:
#     powershell -ExecutionPolicy Bypass -File compiler\build.ps1

$ErrorActionPreference = "Stop"
$here     = Split-Path -Parent $MyInvocation.MyCommand.Path   # ...\compiler
$src      = Join-Path $here "src"
$bin      = Join-Path $here "bin"

# --- locate the D-- toolchain ------------------------------------------------
$dec = (Get-Command dec -ErrorAction SilentlyContinue).Source
if (-not $dec) { $dec = "C:\DMinusMinus\dec.exe" }
if (-not (Test-Path $dec)) { Write-Host "FAIL: dec not found" -ForegroundColor Red; exit 1 }
$declib = Join-Path (Split-Path -Parent $dec) "lib"           # arena.h lives here

if (-not (Test-Path $bin)) { New-Item -ItemType Directory -Path $bin | Out-Null }

Push-Location $src
try {
    # --- front-end exes (dec emits <stem>.exe next to the source) ------------
    Write-Host "building stratac.exe ..." -ForegroundColor Cyan
    & $dec build stratac.dmm | Out-Null
    if (-not $?) { throw "stratac.exe build failed" }

    Write-Host "building console.exe ..." -ForegroundColor Cyan
    & $dec build console.dmm | Out-Null
    if (-not $?) { throw "console.exe build failed" }

    # --- the core as a DLL: emit the core's C, compile it with gcc -shared ---
    Write-Host "building libstrata.dll ..." -ForegroundColor Cyan
    $cpath = Join-Path $bin "libstrata.c"
    $cLines = & $dec emit stratac.dmm
    if (-not $?) { throw "emit failed" }
    [IO.File]::WriteAllLines($cpath, $cLines)   # UTF-8 no BOM, so gcc reads it verbatim
    $dll = Join-Path $bin "libstrata.dll"
    & gcc -shared -O2 -o $dll $cpath -I $declib
    if (-not $?) { throw "gcc -shared failed" }

    # --- collect the exes into bin\ -----------------------------------------
    Copy-Item (Join-Path $src "stratac.exe") (Join-Path $bin "stratac.exe") -Force
    Copy-Item (Join-Path $src "console.exe") (Join-Path $bin "console.exe") -Force
}
finally { Pop-Location }

Write-Host ""
Write-Host "artifacts in compiler\bin\ :" -ForegroundColor Green
Get-ChildItem $bin -Include *.exe,*.dll -Recurse | ForEach-Object { "  {0,-16} {1,8:N0} bytes" -f $_.Name, $_.Length }
