# tests/run.ps1 - bootstrap the Strata compiler (build.ps1) and validate it:
#   1. the bootstrap: stage0 (pinned release) -> stage1 -> stage2, fixpoint-checked
#   2. goldens: each stage's output vs its golden file, byte-for-byte (ARCHITECTURE.md sec 6)
# Run from anywhere:
#     powershell -ExecutionPolicy Bypass -File compiler\tests\run.ps1
#
# A golden file  tests/<stage>/<name>.expected  is compared against the output of
#     stratac <stage> examples/<name>.strata
# using the SHIPPED compiler (bin\stratac.exe, the self-hosted stage2).
# Exit code 0 = all passed, 1 = a mismatch or build failure.

$ErrorActionPreference = "Stop"
$here     = Split-Path -Parent $MyInvocation.MyCommand.Path   # ...\compiler\tests
$compiler = Split-Path -Parent $here                          # ...\compiler
$src      = Join-Path $compiler "src"
$examples = Join-Path $compiler "examples"

# --- bootstrap (stage0 -> stage1 -> stage2, with the fixpoint check) ---------
& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $compiler "build.ps1") | Out-Null
if ($LASTEXITCODE -ne 0) { Write-Host "FAIL: bootstrap build failed (run compiler\build.ps1 to see why)" -ForegroundColor Red; exit 1 }
$strata = Join-Path $compiler "bin\stratac.exe"
Write-Host "PASS  bootstrap (pinned release -> stage1 -> stage2, fixpoint)" -ForegroundColor Green
$pass = 1; $fail = 0

# --- run each golden ---------------------------------------------------------
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

# --- projects (strata.toml): the build system ---------------------------------
# tests/projects/<name>/ is a project. If it has expected.txt, `stratac run <dir> --force`
# must print exactly that (a failing project's expected.txt holds its errors); otherwise
# `stratac build <dir> --force` must succeed (e.g. a dll). Then, for projects that built,
# a second `stratac build` must hit the cache ("up to date").
Get-ChildItem -Path (Join-Path $here "projects") -Directory | ForEach-Object {
    $dir = $_.FullName
    $name = $_.Name
    $expFile = Join-Path $dir "expected.txt"
    $ok = $true
    if (Test-Path $expFile) {
        $actual   = ((& $strata run $dir --force) -join "`n") -replace "`r",""
        $expected = ((Get-Content $expFile -Raw) -replace "`r","").TrimEnd("`n")
        if ($actual.TrimEnd("`n") -ne $expected) { $ok = $false }
        $built = ($LASTEXITCODE -eq 0)
    } else {
        & $strata build $dir --force | Out-Null
        $built = ($LASTEXITCODE -eq 0)
        if (-not $built) { $ok = $false }
    }
    if ($ok -and $built) {
        $again = (& $strata build $dir) -join "`n"
        if ($again -notlike "up to date*") { $ok = $false; Write-Host "      (second build wasn't cached: $again)" -ForegroundColor Yellow }
    }
    if ($ok) { Write-Host "PASS  project/$name" -ForegroundColor Green; $script:pass++ }
    else     { Write-Host "FAIL  project/$name" -ForegroundColor Red;   $script:fail++ }
}

Write-Host ""
Write-Host "$pass passed, $fail failed"
if ($fail -gt 0) { exit 1 }
exit 0
