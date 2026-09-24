# build.ps1 - bootstrap the self-hosted Strata compiler and build all artifacts into
# compiler\bin\ :
#     stratac.exe    front-end #1 (the compiler CLI: run, build, emit, check, ast, tokens)
#     console.exe    front-end #2 (the explorer console)
#     libstrata.dll  the compiler as a shared library (the exe/lib/LSP split in
#                    ARCHITECTURE.md sec 11)
#
# The compiler is written in Strata (selfhost/). A Strata compiler is needed to compile
# it, so the build bootstraps from the frozen D-- seed in src/:
#
#   stage0  dec builds src/stratac.dmm             (the D-- bootstrap compiler)
#   stage1  stage0 builds selfhost/stratac.strata  (Strata compiler, built by D--)
#   stage2  stage1 builds selfhost/stratac.strata  (Strata compiler, built by itself)
#
# Fixpoint check: stage1 and stage2 must emit byte-identical C for the compiler. If they
# don't, the build fails. stage2 is what ships. Run from anywhere:
#     powershell -ExecutionPolicy Bypass -File compiler\build.ps1

$ErrorActionPreference = "Stop"
$here     = Split-Path -Parent $MyInvocation.MyCommand.Path   # ...\compiler
$src      = Join-Path $here "src"
$selfhost = Join-Path $here "selfhost"
$lib      = Join-Path $here "lib"
$bin      = Join-Path $here "bin"
$boot     = Join-Path $here "build"      # stage compilers (a sibling of lib/, so they find it)

# --- locate the D-- toolchain (only needed for stage0) ----------------------
$dec = (Get-Command dec -ErrorAction SilentlyContinue).Source
if (-not $dec) { $dec = "C:\DMinusMinus\dec.exe" }
if (-not (Test-Path $dec)) { Write-Host "FAIL: dec not found" -ForegroundColor Red; exit 1 }

foreach ($d in $bin, $boot) { if (-not (Test-Path $d)) { New-Item -ItemType Directory -Path $d | Out-Null } }

$compilerSrc = Join-Path $selfhost "stratac.strata"
$builtExe    = Join-Path $selfhost "stratac.exe"   # `stratac build` writes next to the source

# Build selfhost/stratac.strata with compiler $with; copy the result to $to.
# (Each stage runs from its own copy, so it never overwrites the exe that is running.)
function Build-Stage([string]$name, [string]$with, [string]$to) {
    Write-Host "building $name ..." -ForegroundColor Cyan
    & $with build $compilerSrc | Out-Null
    if (-not $?) { throw "$name build failed" }
    Copy-Item $builtExe $to -Force
}

# --- stage0: the D-- bootstrap compiler --------------------------------------
Write-Host "building stage0 (D-- bootstrap) ..." -ForegroundColor Cyan
Push-Location $src
try {
    & $dec build stratac.dmm | Out-Null
    if (-not $?) { throw "stage0 build failed" }
} finally { Pop-Location }
$stage0 = Join-Path $boot "stage0.exe"
Copy-Item (Join-Path $src "stratac.exe") $stage0 -Force

# --- stage1, stage2: the Strata compiler, built by D-- and then by itself ----
$stage1 = Join-Path $boot "stage1.exe"
$stage2 = Join-Path $boot "stage2.exe"
Build-Stage "stage1 (Strata, built by stage0)" $stage0 $stage1
Build-Stage "stage2 (Strata, built by stage1)" $stage1 $stage2

# --- fixpoint: stage1 and stage2 must generate the same compiler -------------
$c1 = (& $stage1 emit $compilerSrc) -join "`n"
$c2 = (& $stage2 emit $compilerSrc) -join "`n"
if ($c1 -ne $c2) { throw "fixpoint check FAILED: stage1 and stage2 emit different C for the compiler" }
Write-Host "fixpoint ok: stage1 and stage2 emit identical C" -ForegroundColor Green

Copy-Item $stage2 (Join-Path $bin "stratac.exe") -Force
$stratac = Join-Path $bin "stratac.exe"

# --- console.exe: built by the shipped compiler ------------------------------
Write-Host "building console.exe ..." -ForegroundColor Cyan
& $stratac build (Join-Path $selfhost "console.strata") | Out-Null
if (-not $?) { throw "console.exe build failed" }
Copy-Item (Join-Path $selfhost "console.exe") (Join-Path $bin "console.exe") -Force

# --- libstrata.dll: the compiler's C, compiled as a shared library ---------
Write-Host "building libstrata.dll ..." -ForegroundColor Cyan
$cpath = Join-Path $bin "libstrata.c"
$cLines = & $stratac emit $compilerSrc
if (-not $?) { throw "emit failed" }
[IO.File]::WriteAllLines($cpath, $cLines)   # UTF-8 no BOM, so gcc reads it verbatim
$dll = Join-Path $bin "libstrata.dll"
& gcc -std=gnu11 -shared -O2 -o $dll $cpath -I $lib -lm
if (-not $?) { throw "gcc -shared failed" }

Write-Host ""
Write-Host "artifacts in compiler\bin\ :" -ForegroundColor Green
Get-ChildItem (Join-Path $bin '*') -Include *.exe,*.dll | ForEach-Object { "  {0,-16} {1,8:N0} bytes" -f $_.Name, $_.Length }
