# tests/run.ps1 - bootstrap the Strata compiler (build.ps1) and validate it:
#   1. goldens: each stage's output vs its golden file, byte-for-byte (ARCHITECTURE.md sec 6)
#   2. bootstrap parity: the self-hosted compiler vs the frozen D-- seed, every stage
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
$selfhost = Join-Path $compiler "selfhost"
$examples = Join-Path $compiler "examples"

# --- bootstrap (stage0 -> stage1 -> stage2, with the fixpoint check) ---------
& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $compiler "build.ps1") | Out-Null
if ($LASTEXITCODE -ne 0) { Write-Host "FAIL: bootstrap build failed (run compiler\build.ps1 to see why)" -ForegroundColor Red; exit 1 }
$strata = Join-Path $compiler "bin\stratac.exe"
$stage0 = Join-Path $compiler "build\stage0.exe"
Write-Host "PASS  bootstrap (stage0 -> stage1 -> stage2, fixpoint)" -ForegroundColor Green
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

# --- bootstrap parity: self-hosted compiler vs the frozen D-- seed -----------
# While selfhost/ is still a faithful translation of src/, both compilers must agree
# byte-for-byte on every stage, over every example and compiler source. Once the
# Strata compiler deliberately gains something the seed lacks, drop the affected stage
# here (the fixpoint check in build.ps1 is what must always hold).
$inputs = @(Get-ChildItem $examples -Filter *.strata) + @(Get-ChildItem $selfhost -Filter *.strata) + @(Get-ChildItem $src -Include *.dmm,*.hmm -Recurse)
foreach ($stage in @("tokens", "ast", "check", "emit")) {
    $bad = @()
    foreach ($f in $inputs) {
        $want = (& $stage0 $stage $f.FullName) -join "`n"
        $got  = (& $strata $stage $f.FullName) -join "`n"
        if ($want -ne $got) { $bad += $f.Name }
    }
    if ($bad.Count -eq 0) {
        Write-Host "PASS  parity/$stage  ($($inputs.Count) files identical to the D-- seed)" -ForegroundColor Green
        $pass++
    } else {
        Write-Host "FAIL  parity/$stage  differs on: $($bad -join ', ')" -ForegroundColor Red
        $fail++
    }
}

Write-Host ""
Write-Host "$pass passed, $fail failed"
if ($fail -gt 0) { exit 1 }
exit 0
