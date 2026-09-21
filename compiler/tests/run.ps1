# tests/run.ps1 - rebuild the Strata compiler and validate each stage against its
# golden files, byte-for-byte (ARCHITECTURE.md sec 6). Run from anywhere:
#     powershell -ExecutionPolicy Bypass -File compiler\tests\run.ps1
#
# A golden file  tests/<stage>/<name>.expected  is compared against the output of
#     strata <stage> examples/<name>.strata
# Exit code 0 = all passed, 1 = a mismatch or build failure.

$ErrorActionPreference = "Stop"
$here     = Split-Path -Parent $MyInvocation.MyCommand.Path   # ...\compiler\tests
$compiler = Split-Path -Parent $here                          # ...\compiler
$src      = Join-Path $compiler "src"
$examples = Join-Path $compiler "examples"

# --- locate the D-- compiler (dec) -------------------------------------------
$dec = (Get-Command dec -ErrorAction SilentlyContinue).Source
if (-not $dec) { $dec = "C:\DMinusMinus\dec.exe" }
if (-not (Test-Path $dec)) { Write-Host "FAIL: dec not found (PATH or C:\DMinusMinus\dec.exe)" -ForegroundColor Red; exit 1 }

# --- rebuild the compiler ----------------------------------------------------
Push-Location $src
& $dec build stratac.dmm | Out-Null
$ok = $?
Pop-Location
if (-not $ok) { Write-Host "FAIL: compiler build failed" -ForegroundColor Red; exit 1 }
$strata = Join-Path $src "stratac.exe"

# --- run each golden ---------------------------------------------------------
$pass = 0; $fail = 0
Get-ChildItem -Path $here -Directory | ForEach-Object {
    $stage = $_.Name                                          # e.g. "tokens"
    Get-ChildItem -Path $_.FullName -Filter *.expected | ForEach-Object {
        $name    = [IO.Path]::GetFileNameWithoutExtension($_.Name)
        $source  = Join-Path $examples "$name.strata"
        if (-not (Test-Path $source)) {
            Write-Host "SKIP  $stage/$name  (no examples\$name.strata)" -ForegroundColor Yellow
            return
        }
        # Normalize line endings (golden files may be CRLF; captured output is LF).
        $actual   = ((& $strata $stage $source) -join "`n") -replace "`r",""
        $expected = ((Get-Content $_.FullName -Raw) -replace "`r","").TrimEnd("`n")
        $actual   = $actual.TrimEnd("`n")
        if ($actual -eq $expected) {
            Write-Host "PASS  $stage/$name" -ForegroundColor Green
            $script:pass++
        } else {
            Write-Host "FAIL  $stage/$name" -ForegroundColor Red
            $script:fail++
        }
    }
}

Write-Host ""
Write-Host "$pass passed, $fail failed"
if ($fail -gt 0) { exit 1 }
exit 0
